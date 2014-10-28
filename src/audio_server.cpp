#include "audio_server.h"
audio_server::audio_server(boost::asio::io_service& io_service, short port)
: acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
  socket_(io_service),
  scan_player_timer_(io_service,boost::posix_time::seconds(0))

{
	//
	acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	message_center::add_event("send_data",std::bind(&audio_server::send_data,this,std::placeholders::_1));
	message_center::add_event("get_address",std::bind(&audio_server::get_address,this,std::placeholders::_1));

	wait_scan_player_timer();

	do_accept();

	//
}
void audio_server::get_address(const vector<boost::any>& params)
{
	//
	uint client_id=boost::any_cast<uint>(params.at(0));
	int* ip = boost::any_cast<int*>(params.at(1));
	auto it =this->clients_.at(client_id);
	*ip = it->client_ip();

	//
}

void audio_server::wait_scan_player_timer()
{
	scan_player_timer_.async_wait([&](const boost::system::error_code& c){
		vector<audio_client::PTR> temp;
		{
			std::lock_guard<std::mutex> lck(mutex_);
			//copy unremoved values from aMap to aTempMap

			for(auto it = clients_.begin(), ite = clients_.end(); it != ite;)
			{
				if(it->second->online_count_ >180)
				{
					temp.push_back(it->second);
					it = clients_.erase(it);
				}
				else
				{
					it->second->online_count_++;
					++it;
				}
			}
		}
		std::for_each(temp.begin(), temp.end(),[&](const audio_client::PTR& v){
			BOOST_LOG_TRIVIAL(trace)<<"player time out,reset a player,client_id is "<<v->client_id();

			v->reset();

		});
		//Swap the contents of aMap and aTempMap

		scan_player_timer_.expires_at(scan_player_timer_.expires_at() + boost::posix_time::seconds(1));
		wait_scan_player_timer();
	});
}
void audio_server::on_client_reset(uint client_id)
{
	//
	std::lock_guard<std::mutex> lck(mutex_);
	BOOST_LOG_TRIVIAL(trace)<<"on_client_reset,client_id is "<<client_id;

	vector<boost::any> temp;
	temp.push_back(client_id);
	message_center::functions f = message_center::get_event("reset_player");
	f(temp);
	//

	clients_.erase(client_id);
	//
}

void audio_server::send_data(const vector<boost::any>& params)
{

	char buffer[1024];
	memset(buffer,0,1024);
	SEVER_EVENT event  = boost::any_cast<SEVER_EVENT>(params.at(0));
	uint client_id=boost::any_cast<uint>(params.at(1));
	int* main_type = (int*)buffer;
	int* sub_type=(int*)(buffer+1);

	size_t len=0;
	switch(event)
	{
	case SERVER_ERROR_ECHO:
		*main_type=1;
		*sub_type=100;
		*(char*)(buffer+2)=(char)boost::any_cast<int>(params.at(2));
		len =3;
		break;

	case CREATE_ROOM_ECHO:
		*main_type=1;
		*sub_type=8;
		len=2;
		BOOST_LOG_TRIVIAL(trace)<<"create room echo,client_id is "<<client_id;

		break;
	case DESTROY_ROOM_ECHO:
		*main_type=1;
		*sub_type=26;
		len=2;
		BOOST_LOG_TRIVIAL(trace)<<"destroy room echo,client_id is "<<client_id;

		break;
	case LOGIN_ECHO:
		*main_type=1;
		*sub_type=4;
		len=2;
		BOOST_LOG_TRIVIAL(trace)<<"login echo,client_id is "<<client_id;

		break;

	case LOGOUT_ECHO:
		*main_type=1;
		*sub_type=6;
		len=2;
		BOOST_LOG_TRIVIAL(trace)<<"logout echo,client_id is "<<client_id;

		break;
	case AUDIO_DATA_ECHO:
	{
		char* data=boost::any_cast<char*>(params.at(2));
		uint temp_len=boost::any_cast<uint>(params.at(3));
		memcpy(buffer,data,temp_len);
		len = temp_len;
		BOOST_LOG_TRIVIAL(trace)<<"audio data echo,client_id is "<<client_id;

	}
	break;
	case GET_ROOM_LIST_ECHO:
	{
		*main_type=1;
		*sub_type=16;
		char* data=boost::any_cast<char*>(params.at(2));
		int temp_len=boost::any_cast<int>(params.at(3));
		memcpy(buffer+2,data,temp_len);
		len = temp_len+2;
		BOOST_LOG_TRIVIAL(trace)<<"get room list  echo,client_id is "<<client_id;

	}
	break;
	case MEMBER_BEGIN_TALK_ECHO:
		{
			*main_type=1;
			*sub_type=30;

			len=2;
			BOOST_LOG_TRIVIAL(trace)<<"member begin talk echo,client_id is "<<client_id;
		}break;
		case MEMBER_STOP_TALK_ECHO:
		{
			*main_type=1;
			*sub_type=32;

			len=2;
			BOOST_LOG_TRIVIAL(trace)<<"member stop talk echo,client_id is "<<client_id;
		}
		break;
	case BEGIN_TALK_ECHO:
	{
		*main_type=2;
		*sub_type=30;
		uint player_id = boost::any_cast<uint>(params.at(2));
		*(uint*)(buffer+2)=player_id;
		len=6;
		BOOST_LOG_TRIVIAL(trace)<<"begin talk echo,client_id is "<<client_id;
	}break;
	case STOP_TALK_ECHO:
	{
		*main_type=2;
		*sub_type=32;
		uint player_id = boost::any_cast<uint>(params.at(2));
		*(uint*)(buffer+2)=player_id;
		len=6;
		BOOST_LOG_TRIVIAL(trace)<<"stop talk echo,client_id is "<<client_id;
	}
	break;
	case RECV_AUDIO_ECHO:
	{
		*main_type=1;
		*sub_type=28;
		len=2;
		BOOST_LOG_TRIVIAL(trace)<<"recv audio echo,client_id is "<<client_id;
	}
	break;
	case GET_ROOM_MEMBER_ECHO:
	{
		*main_type=1;
		*sub_type=14;
		char* data=boost::any_cast<char*>(params.at(2));
		int temp_len=boost::any_cast<int>(params.at(3));
		memcpy(buffer+2,data,temp_len);
		len = temp_len+2;
		BOOST_LOG_TRIVIAL(trace)<<"get room member echo,client_id is "<<client_id;

	}
	break;
	case MEMBER_JOIN_ROOM_ECHO:

	{
		*main_type=1;
		*sub_type=22;
		uint player_id = boost::any_cast<uint>(params.at(2));
		uint room_id = boost::any_cast<uint>(params.at(3));
		*(uint*)(buffer+2)=player_id;
		*(uint*)(buffer+6)=room_id;
		len=10;
		BOOST_LOG_TRIVIAL(trace)<<"member join room echo,client_id is "<<client_id;

	}
	break;
	case MEMBER_LEAVE_ROOM_ECHO:
	{
		*main_type=1;
		*sub_type=24;
		uint player_id = boost::any_cast<uint>(params.at(2));
		uint room_id = boost::any_cast<uint>(params.at(3));
		*(uint*)(buffer+2)=player_id;
		*(uint*)(buffer+6)=room_id;
		len=10;
		BOOST_LOG_TRIVIAL(trace)<<"member leave room  echo,client_id is "<<client_id;

	}
	break;

	case JOIN_ROOM_ECHO:
		*main_type=1;
		*sub_type=10;
		len=2;
		BOOST_LOG_TRIVIAL(trace)<<"join room echo,client_id is "<<client_id;

		break;
	case LEAVE_ROOM_ECHO:
		*main_type=1;
		*sub_type=12;
		len=2;
		BOOST_LOG_TRIVIAL(trace)<<"leave room echo,client_id is "<<client_id;

		break;
	}

	if(clients_.count(client_id)>0)
	clients_[client_id]->send_data(buffer,len);

}


void audio_server::do_accept()
{
	try{
		acceptor_.async_accept(socket_,
				[this](boost::system::error_code ec)
				{
			if (!ec)
			{
				std::lock_guard<std::mutex> lck(mutex_);
				audio_client::PTR c= std::make_shared<audio_client>(std::move(socket_));
				c->setClient_id(client_index_++);
				c->reset_client_event.connect(std::bind(&audio_server::on_client_reset,this,std::placeholders::_1));
				clients_[c->client_id()]=c;
				BOOST_LOG_TRIVIAL(trace)<<"client have accept ,client_id is "<<c->client_id();


			}

			do_accept();
				});
	}catch(const std::exception& e)
	{
		int j=2;
	}
}

