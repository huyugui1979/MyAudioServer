#include "audio_hall.h"

audio_hall::audio_hall()
{
    on_create_room(0,0);
    message_center::add_event("process_player_command",std::bind(&audio_hall::process_player_command,this,std::placeholders::_1));
    message_center::add_event("reset_player",std::bind(&audio_hall::reset_player,this,std::placeholders::_1));
}


void audio_hall::process_player_command(const vector<boost::any>& param)
{

    PLAYER_EVENT event = any_cast<PLAYER_EVENT>(param.at(0));
    switch(event)
    {
    case LOGIN:
    {
        uint client_id = any_cast<uint>(param.at(1));
        uint player_id = any_cast<uint>(param.at(2));
        short port = any_cast<short>(param.at(3));
        on_login_event(client_id,player_id,port);
    }
        break;
    case LOGOUT:
    {
        uint client_id = any_cast<uint>(param.at(1));

        on_logout_event(client_id);
    }
        break;
    case GET_ROOM_MEMBER:
    {
        uint client_id = any_cast<uint>(param.at(1));

        on_get_room_member(client_id);
    }
        break;
    case GET_ROOM_LIST:
    {
        uint client_id = any_cast<uint>(param.at(1));
        on_get_room_list(client_id);
    }
        break;
    case JOIN_ROOM:
    {
        uint client_id = any_cast<uint>(param.at(1));
        uint room_id = any_cast<uint>(param.at(2));
        on_join_room_event(client_id,room_id);
    }
        break;
    case LEAVE_ROOM:
    {
        uint client_id = any_cast<uint>(param.at(1));
        uint room_id = any_cast<uint>(param.at(2));
        on_leave_room_event(client_id,room_id);
    }
        break;
    case AUDIO_DATA:
    {
        uint client_id = any_cast<uint>(param.at(1));
        char* data = any_cast<char*>(param.at(2));
        uint len = any_cast<uint>(param.at(3));
        on_audio_data(client_id,data,len);
    }
        break;


    }
}

void audio_hall::reset_player(const vector<boost::any>& param)
{
    std::lock_guard<std::mutex> lck(mutex_);
    uint client_id = any_cast<uint>(param.at(0));
    auto it =players_.find(client_id);
    if(it ==players_.end())
    {
        return;
    }
    if(it->second->player_status()==JOINED_ROOM)
    {
        //
        auto v = rooms_.find(it->second->room_id());
        if(v == rooms_.end())
        {
            MY_THROW(NO_JOIN_ROOM);
        }else
            v->second->leave_room(it->second);
        //
    }
    players_.erase(client_id);
    //
}

void audio_hall::on_create_room(uint client_id,uint player_id)
{
    //
    std::lock_guard<std::mutex> lck(mutex_);
    BOOST_LOG_TRIVIAL(trace)<<"create room,client_id is"<<client_id;

    audio_room::PTR p = std::make_shared<audio_room>();
    rooms_[p->room_id()]=p;
    //
    //
}

void audio_hall::on_logout_event(uint client_id)
{
    std::lock_guard<std::mutex> lck(mutex_);
    BOOST_LOG_TRIVIAL(trace)<<"logout,client_id is"<<client_id;

    vector<boost::any> echo;

    auto it =players_.find(client_id);
    if(it ==players_.end())
    {
        MY_THROW(NOT_LOGIN);

    }
    if(it->second->player_status()==JOINED_ROOM)
    {
        //
        auto v = rooms_.find(it->second->room_id());
        if(v == rooms_.end())
        {
            MY_THROW(NO_JOIN_ROOM);
        }else
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
    BOOST_LOG_TRIVIAL(trace)<<"login,player_id is "<<player_id<<" client_id is"<<client_id;

    vector<boost::any> echo;
    auto it =players_.find(client_id);
    if(it != players_.end())
    {
        MY_THROW(HAVE_LOGIN);
    }
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
    //
}
void audio_hall::on_audio_data(uint client_id,char* data,uint len)
{
    //
    std::lock_guard<std::mutex> lck(mutex_);
    BOOST_LOG_TRIVIAL(trace)<<"audio data,client is"<<client_id<<" len is "<<len;

    auto it = players_.find(client_id);
    if(it == players_.end())
    {
        MY_THROW(NOT_LOGIN);
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
    auto it =players_.find(client_id);
    if(it ==players_.end())
    {
        MY_THROW(NOT_LOGIN);
    }
    auto it1=rooms_.find(it->second->room_id());
    if(it1 == rooms_.end())
    {
        MY_THROW(NO_ROOM);
    }
    char buffer[1024];
    bzero(buffer,1024);

    vector<audio_player::PTR> players=it1->second->players();
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
            int client_id = a->client_id();
            short port = a->udp_port();
            memcpy(buffer+total,&client_id,4);
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
    BOOST_LOG_TRIVIAL(trace)<<"get room list ,client_id is "<<client_id;

    vector<boost::any> echo;
    auto it =players_.find(client_id);
    if(it ==players_.end())
    {
        MY_THROW(NOT_LOGIN);
    }
    //
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
    //

}
void audio_hall::on_join_room_event(uint client_id,uint room_id)
{
    std::lock_guard<std::mutex> lck(mutex_);
    BOOST_LOG_TRIVIAL(trace)<<"join room,client_id is"<<client_id<<" room_id is "<<room_id;

    vector<boost::any> echo;
    auto it =players_.find(client_id);
    if(it ==players_.end())
    {
        MY_THROW(NOT_LOGIN);
    }
    auto it1=rooms_.find(room_id);
    if(it1 == rooms_.end())
    {
        MY_THROW(NO_ROOM);
    }
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
    auto it =players_.find(client_id);
    if(it ==players_.end())
    {
        MY_THROW(NOT_LOGIN);
    }
    auto it1=rooms_.find(room_id);
    if(it1 == rooms_.end())
    {
        MY_THROW(NO_ROOM);
    }
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
