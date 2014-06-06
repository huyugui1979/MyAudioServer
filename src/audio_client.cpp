#include "audio_client.h"

uint audio_client::client_id() const
{
	return client_id_;
}

void audio_client::setClient_id(const uint &player_id)
{
	client_id_ = player_id;
}
int audio_client::client_ip() const
{
	return client_ip_;
}

void audio_client::setClient_ip(int client_ip)
{
	client_ip_ = client_ip;
}

audio_client::audio_client(tcp::socket socket)
: socket_(std::move(socket)),
  strand_(socket_.get_io_service()),online_count_(0)
{
	try{
	string address = socket_.remote_endpoint().address().to_v4().to_string();
	BOOST_LOG_TRIVIAL(trace)<<"create audio_client address is  "<<address;
	client_ip_ = socket_.remote_endpoint().address().to_v4().to_ulong();
	BOOST_LOG_TRIVIAL(trace)<<"ip is   "<<client_ip_;

	do_read_head();
	}catch(std::exception& e)
	{
		BOOST_LOG_TRIVIAL(trace)<<"create audio have exception,error is "<<e.what()<<endl;
	}
}
void audio_client::send_data(char* data,size_t len)
{

    if(client_id_ ==-1)
    {
    	BOOST_LOG_TRIVIAL(trace)<<"have reset,give up"<<" client_id is "<<client_id_;;

    	return ;
    }
	ushort s = len;
	char buffer[1024];
	*(ushort*)(buffer)=s;
	memcpy(buffer+2,data,len);
	boost::asio::async_write(socket_, boost::asio::buffer(buffer, len+2),
			[&](boost::system::error_code ec, std::size_t /*length*/)
			{
		if(!ec)
		{

		}
			});
	BOOST_LOG_TRIVIAL(trace)<<"send_data len is "<<len<<" client_id is "<<client_id_;

	//
}

void audio_client::read_head(const boost::system::error_code ec, std::size_t length)
{


	if(!ec)
	{
		BOOST_LOG_TRIVIAL(trace)<<"read_head,len is "<<length<<" client_id is "<<client_id_;
			memset(data_,0,1024);
		boost::asio::async_read(socket_,boost::asio::buffer(data_, len_),
				std::bind(&audio_client::read_data,this,std::placeholders::_1,std::placeholders::_2));
	}else
	{
		BOOST_LOG_TRIVIAL(error)<<"socket error "<<ec.message()<<" client_id is "<<client_id_;;
		if(ec.value() !=boost::asio::error::operation_aborted )
		{

			reset();
		}
	}
}
void audio_client::read_data(const boost::system::error_code ec, std::size_t length)
{

	if (!ec)
	{
		BOOST_LOG_TRIVIAL(trace)<<"read_data,len is %d"<<length<<" client_id is "<<client_id_;;
			vector<boost::any> params;
		char* main_type = (char*) data_;
		char* sub_type = (char*) (data_ + 1);

		//

		if (*main_type == 1 && *sub_type == 1) { //check server
			//
			params.push_back(CHECK_SERVER);
			params.push_back(client_id_);

			//
		}
		else if (*main_type == 1 && *sub_type == 3) { //LOGIN

			uint* playerId = (uint*) (data_ + 2);
			short* port =(short*)(data_+6);
			params.push_back(LOGIN);
			params.push_back(client_id_);
			params.push_back((uint)*playerId);
			params.push_back(*port);

		}
		else if (*main_type == 1 && *sub_type == 5)	//logout
		{
			params.push_back(LOGOUT);
			params.push_back(client_id_);

		}

		else if (*main_type == 1 && *sub_type == 7)	//create room
		{
			params.push_back(CREATE_ROOM);
			params.push_back(client_id_);
			params.push_back(*(uint*)(data_+2));

		}
		else if (*main_type == 1 && *sub_type ==25)	//destroy room
		{
			params.push_back(DESTROY_ROOM);
			params.push_back(client_id_);
			params.push_back(*(uint*)(data_+2));

		}
		else if(*main_type == 1 && *sub_type ==29)
		{
			//
			params.push_back(BEGIN_TALK);
						params.push_back(client_id_);
			//
		}else if(*main_type == 1 && *sub_type ==31)
		{
			//
			params.push_back(STOP_TALK);
			params.push_back(client_id_);
		}
		else if (*main_type == 1 && *sub_type ==27)	//set whether recv audio
		{
			params.push_back(RECV_AUDIO);
			params.push_back(client_id_);
			params.push_back(*(uint*)(data_+2));
			params.push_back(*(bool*)(data_+6));

		}

		else if (*main_type == 1 && *sub_type == 9)	//join room
		{
			//
			int* roomId = (int*) (data_ + 2);
			params.push_back(JOIN_ROOM);
			params.push_back(client_id_);
			params.push_back((uint)*roomId);


		}
		else if (*main_type == 1 && *sub_type == 11)	//leave room
		{
			//
			int* roomId = (int*) (data_ + 2);
			//
			params.push_back(LEAVE_ROOM);
			params.push_back(client_id_);
			params.push_back((uint)*roomId);

			//
		}
		else if (*main_type == 1 && *sub_type == 13)            //get memeber
		{
			//
			int* roomId = (int*) (data_ + 2);
			//
			params.push_back(GET_ROOM_MEMBER);
			params.push_back(client_id_);
			params.push_back((uint)*roomId);
			//

		}
		else if (*main_type == 1 && *sub_type == 15)            //get room's  list
		{
			//
			params.push_back(GET_ROOM_LIST);

			params.push_back(client_id_);


			//
		}
		else if (*main_type == 1 && *sub_type == 17) {		//audio data
			//
			params.push_back(AUDIO_DATA);
			params.push_back(client_id_);

			params.push_back(data_);
			params.push_back((uint)length);

			//
		}
		else if (*main_type == 1 && *sub_type == 99) {			//timer
			//
			online_count_=0;
			params.push_back(TIMER);
			params.push_back(client_id_);
			BOOST_LOG_TRIVIAL(trace)<<"timer,client_id is "<<client_id_;

			//
		}else//
		{
			reset();
		    return ;
		}

		message_center::functions f = message_center::get_event("process_player_command");
		f(params);
		do_read_head();

	}
	else
	{
		BOOST_LOG_TRIVIAL(error)<<"socket error "<<ec.message()<<" client_id is "<<client_id_;;
		if(ec.value() != boost::asio::error::operation_aborted)
		{
			reset();
		}
	}
}
void audio_client::do_read_head()
{
	memset(&len_,0,2);
	boost::asio::async_read(socket_,boost::asio::buffer(&len_, 2),
			std::bind(&audio_client::read_head,this,std::placeholders::_1,std::placeholders::_2));


}
void audio_client::reset(){

	BOOST_LOG_TRIVIAL(trace)<<"reset,client_id is "<<client_id_;
    boost::system::error_code ec;
    socket_.cancel(ec);
	socket_.close(ec);

	reset_client_event(client_id_);
	client_id_=-1;

}
