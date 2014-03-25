#include "audio_hall.h"

audio_hall::audio_hall()
{
	message_center::add_event("process_player_command",std::bind(&audio_hall::process_player_command,this,std::placeholders::_1));
	message_center::add_event("reset_player",std::bind(&audio_hall::reset_player,this,std::placeholders::_1));
}


void audio_hall::process_player_command(const vector<boost::any>& param)
{
	uint client_id;
	try{
		PLAYER_EVENT event = any_cast<PLAYER_EVENT>(param.at(0));
		client_id = any_cast<uint>(param.at(1));
		switch(event)
		{
		case CREATE_ROOM:
		{

			uint room_id = any_cast<uint>(param.at(2));
			this->on_create_room(client_id,room_id);
		}
		break;
		case RECV_AUDIO:
		{
			uint player_id = any_cast<uint>(param.at(2));
			bool recv = any_cast<bool>(param.at(3));
			this->on_set_recv_audio(client_id,player_id,recv);
		}
			break;
		case LOGIN:
		{
			uint player_id = any_cast<uint>(param.at(2));
			short port = any_cast<short>(param.at(3));
			on_login_event(client_id,player_id,port);
		}
		break;
		case LOGOUT:
		{

			on_logout_event(client_id);
		}
		break;
		case GET_ROOM_MEMBER:
		{

			on_get_room_member(client_id);
		}
		break;
		case GET_ROOM_LIST:
		{
			on_get_room_list(client_id);
		}
		break;
		case UDP_HOLE:
		{
			//uint player_id = any_cast<uint>(params.at(1));
			//on_get_udp_hole(client_id,player_id);
		}
		break;
		case JOIN_ROOM:
		{
			uint room_id = any_cast<uint>(param.at(2));
			on_join_room_event(client_id,room_id);
		}
		break;
		case LEAVE_ROOM:
		{
			uint room_id = any_cast<uint>(param.at(2));
			on_leave_room_event(client_id,room_id);
		}
		break;
		case AUDIO_DATA:
		{
			char* data = any_cast<char*>(param.at(2));
			uint len = any_cast<uint>(param.at(3));
			on_audio_data(client_id,data,len);
		}
		break;


		}
	}catch(const boost::exception& e)
	{
		//
		message_center::functions f = message_center::get_event("send_data");
		vector<boost::any> echo;
		echo.push_back(SERVER_ERROR_ECHO);
		echo.push_back(client_id);
		echo.push_back(*get_error_info<err_no>(e));
		f(echo);
		BOOST_LOG_TRIVIAL(error)<<"error_no:"<<*get_error_info<err_no>(e) \
				<<" "<<*get_error_info<errinfo_api_function>(e) \
				<<":"<<*get_error_info<errinfo_errno>(e);
		//

	}
}
/*
void audio_hall::on_get_udp_hole(uint client_id,uint player_id)
{
	auto it = find_if(players_.begin(),players_.end(),[&](const pair<uint,audio_player::PTR>& p](){
		if(p->second->player_id() == player_id)
			return true;
		else 
			return false;
	}
	);


}*/

void audio_hall::check_login_in(uint client_id)
{
	auto it =players_.find(client_id);
	if(it == players_.end())
	{
		MY_THROW(NOT_LOGIN);
	}
}
void audio_hall::check_not_login_in(uint client_id)
{
	auto it =players_.find(client_id);
	if(it != players_.end())
	{
		MY_THROW(HAVE_LOGIN);
	}
}
void audio_hall::check_have_room(uint room_id)
{
	auto it1=rooms_.find(room_id);
	if(it1 == rooms_.end())
	{
		MY_THROW(NO_ROOM);
	}
}
void audio_hall::check_not_have_room(uint room_id)
{
	auto it1=rooms_.find(room_id);
	if(it1 != rooms_.end())
	{
		MY_THROW(ROOM_HAVE_EXIST);
	}
}
void audio_hall::check_room_have_player(uint room_id)
{
	auto it1 = rooms_.find(room_id);
	uint size = it1->second->get_player_count();
	if(size >0)
	{
		//
		MY_THROW(ROOM_HAVE_PLAYER);
		//
	}
}
void audio_hall::on_destroy_room(uint client_id,uint room_id)
{
	//
	std::lock_guard<std::mutex> lck(mutex_);
	check_have_room(room_id);
	check_room_have_player(room_id);
	rooms_.erase(room_id);
	message_center::functions f = message_center::get_event("send_data");
	vector<boost::any> echo;
	echo.push_back(DESTROY_ROOM_ECHO);
	echo.push_back(client_id);
	f(echo);
	//
}
void audio_hall::reset_player(const vector<boost::any>& param)
{
	std::lock_guard<std::mutex> lck(mutex_);
	uint client_id = any_cast<uint>(param.at(0));

	auto it =players_.find(client_id);
	if(it != players_.end())
	{
		if(it->second->player_status()==JOINED_ROOM)
		{
			//
			auto v = rooms_.find(it->second->room_id());
			if(v != rooms_.end())
				v->second->leave_room(it->second);
			//
		}

		players_.erase(client_id);
	}
	//
}

void audio_hall::on_create_room(uint client_id,uint room_id)
{
	//
	std::lock_guard<std::mutex> lck(mutex_);
	BOOST_LOG_TRIVIAL(trace)<<"create room,client_id is"<<client_id<<",room_id is "<<room_id;
	check_login_in(client_id);
	check_not_have_room(room_id);
	audio_room::PTR p = std::make_shared<audio_room>();
	p->setRoom_id(room_id);
	rooms_[p->room_id()]=p;

	//
	message_center::functions f = message_center::get_event("send_data");
	vector<boost::any> echo;
	echo.push_back(CREATE_ROOM_ECHO);
	echo.push_back(client_id);
	//
	f(echo);
}

void audio_hall::on_logout_event(uint client_id)
{
	std::lock_guard<std::mutex> lck(mutex_);
	BOOST_LOG_TRIVIAL(trace)<<"logout,client_id is"<<client_id;
	vector<boost::any> echo;
	check_login_in(client_id);
	auto it =players_.find(client_id);

	if(it->second->player_status()==JOINED_ROOM)
	{
		//
		check_have_room(it->second->room_id());
		auto v = rooms_.find(it->second->room_id());

		v->second->leave_room(it->second);
		//
	}
	players_.erase(client_id);
	message_center::functions f = message_center::get_event("send_data");
	echo.push_back(LOGOUT_ECHO);
	echo.push_back(client_id);
	f(echo);


}

void audio_hall::on_login_event(uint client_id, uint player_id,short port)
{
	std::lock_guard<std::mutex> lck(mutex_);

	vector<boost::any> echo;
	check_not_login_in(client_id);
	audio_player::PTR p=std::make_shared<audio_player>();
	p->setClient_id(client_id);
	p->setPlayer_id(player_id);
	p->setUdp_port(port);
	p->setPlayer_status(LOGIN_HALL);
	players_[p->client_id()]=p;

	//
	message_center::functions f = message_center::get_event("send_data");
	echo.push_back(LOGIN_ECHO);
	echo.push_back(client_id);
	f(echo);
	BOOST_LOG_TRIVIAL(trace)<<"login,player_id is "<<player_id<<" client_id is"<<client_id;

	//
}
void audio_hall::on_set_recv_audio(uint client_id,uint player_id,bool b)
{
	std::lock_guard<std::mutex> lck(mutex_);
	BOOST_LOG_TRIVIAL(trace)<<"on set recv audio,client is"<<client_id;
	this->check_login_in(client_id);

	auto it = players_.find(client_id);
	//

	//
	if(player_id != 0)
	{
		it->second->recv_audio_list_[player_id] = b;
	}else if(player_id ==0)
	{
		for(auto& it1:it->second->recv_audio_list_)
		{
			it1.second=b;
		}
	}
	message_center::functions f = message_center::get_event("send_data");
	vector<boost::any> echo;
	echo.push_back(RECV_AUDIO_ECHO);
	echo.push_back(client_id);
	f(echo);
}
void audio_hall::on_audio_data(uint client_id,char* data,uint len)
{
	//
	std::lock_guard<std::mutex> lck(mutex_);
	BOOST_LOG_TRIVIAL(trace)<<"audio data,client is"<<client_id<<" len is "<<len;

	auto it = players_.find(client_id);
	if(it == players_.end())
	{
		return;
	}
	auto v = rooms_.find(it->second->room_id());

	if(v == rooms_.end())
	{
		return;
	}
	v->second->send_data_room(it->second,data,len);
	//
}

void audio_hall::on_get_room_member(uint client_id)
{
	std::lock_guard<std::mutex> lck(mutex_);
	vector<boost::any> echo;
	check_login_in(client_id);
	auto it =players_.find(client_id);
	check_have_room(it->second->room_id());
	auto it1=rooms_.find(it->second->room_id());
	char buffer[1024];
	bzero(buffer,1024);
	vector<audio_player::PTR> players;
	it1->second->get_players(players);
	ushort* c = (ushort*)buffer;

	int total=2;
	std::for_each(players.begin(), players.end(), [&](const audio_player::PTR& a) {
		if(a->client_id() != client_id)
		{
			message_center::functions f = message_center::get_event("get_address");
			vector<boost::any> params;
			params.push_back(a->client_id());
			int client_ip;
			params.push_back(&client_ip);
			f(params);
			//
			uint player_id= a->player_id();
			short port = a->udp_port();
			memcpy(buffer+total,&player_id,4);
			total=total+4;
			memcpy(buffer+total,&client_ip,4);
			total=total+4;
			memcpy(buffer+total,&port,2);
			total=total+2;
			*c = *c+1;
		}
	});
	message_center::functions f = message_center::get_event("send_data");
	echo.push_back(GET_ROOM_MEMBER_ECHO);
	echo.push_back(client_id);
	echo.push_back(buffer);
	echo.push_back(total);
	f(echo);
}

void audio_hall::on_get_room_list(uint client_id)
{
	std::lock_guard<std::mutex> lck(mutex_);

	vector<boost::any> echo;
	check_login_in(client_id);
	//
	char buffer[1024];
	bzero(buffer,1024);
	int total=0;
	*(ushort*)buffer=rooms_.size();

	total=total+2;
	std::for_each(rooms_.begin(), rooms_.end(),
			[&](const pair<uint,audio_room::PTR> r) {
		uint room_id = r.second->room_id();
		memcpy(buffer+total,&room_id,4);
		total=total+4;
	});

	message_center::functions f = message_center::get_event("send_data");

	echo.push_back(GET_ROOM_LIST_ECHO);
	echo.push_back(client_id);
	echo.push_back(buffer);
	echo.push_back(total);
	f(echo);
	//
	BOOST_LOG_TRIVIAL(trace)<<"get room list ,client_id is "<<client_id;
	//

}
void audio_hall::on_join_room_event(uint client_id,uint room_id)
{
	std::lock_guard<std::mutex> lck(mutex_);
	BOOST_LOG_TRIVIAL(trace)<<"join room,client_id is"<<client_id<<" room_id is "<<room_id;

	vector<boost::any> echo;
	check_login_in(client_id);
	check_have_room(room_id);
	//
	audio_player::PTR p = players_.at(client_id);
	audio_room::PTR  r = rooms_.at(room_id);
	r->join_room(p);

	//
	message_center::functions f = message_center::get_event("send_data");
	echo.push_back(JOIN_ROOM_ECHO);
	echo.push_back(client_id);
	f(echo);
	//

}
void audio_hall::on_leave_room_event(uint client_id,uint room_id)
{
	std::lock_guard<std::mutex> lck(mutex_);
	BOOST_LOG_TRIVIAL(trace)<<"leave room,client_id is"<<client_id<<" room id is "<<room_id;

	vector<boost::any> echo;
	check_login_in(client_id);
	check_have_room(room_id);

	//
	audio_player::PTR p = players_.at(client_id);
	audio_room::PTR  r = rooms_.at(room_id);
	r->leave_room(p);

	//
	message_center::functions f = message_center::get_event("send_data");
	echo.push_back(LEAVE_ROOM_ECHO);
	echo.push_back(client_id);
	f(echo);
	//
}
