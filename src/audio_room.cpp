#include "audio_room.h"

audio_room::audio_room()
{

}
uint audio_room::room_id() const
{
    return room_id_;
}
void audio_room::join_room(audio_player::PTR player)
{
    std::lock_guard<std::mutex> lck(mutex_);
    BOOST_LOG_TRIVIAL(trace)<<"join_room,player is "<<player->player_id();

    auto it =std::find(players_.begin(),players_.end(),player);
    if(it != players_.end())
    {
        MY_THROW(HAVE_JOIN_ROOM);
    }
    players_.push_back(player);
    player->setPlayer_status(JOINED_ROOM);
    player->setRoom_id(room_id_);
    //
    for_each(players_.begin(),players_.end(),[&](const audio_player::PTR& v){
        if(v->player_id() != player->player_id())
        {
            vector<boost::any> echo;
            message_center::functions f = message_center::get_event("send_data");
            echo.push_back(MEMBER_JOIN_ROOM_ECHO);
            echo.push_back(v->client_id());
            echo.push_back(player->player_id());
            echo.push_back(room_id_);
            f(echo);
            //

            //

        }
    });
    //


}
void audio_room::send_data_room(audio_player::PTR player,char* data,uint len)
{
    //
     std::lock_guard<std::mutex> lck(mutex_);
    BOOST_LOG_TRIVIAL(trace)<<"send_data_room,player is "<<player->player_id()<<"data len is "<<len;

    for_each(players_.begin(),players_.end(),[&](const audio_player::PTR& v){
         if(v->player_id() != player->player_id())
        {
            message_center::functions f = message_center::get_event("send_data");
              vector<boost::any> echo;
            echo.push_back(AUDIO_DATA_ECHO);
            echo.push_back(v->client_id());
            echo.push_back(data);
            echo.push_back(len);
            f(echo);
            //
        }
    });
    //
}

void audio_room::leave_room(audio_player::PTR player)
{
     std::lock_guard<std::mutex> lck(mutex_);
    BOOST_LOG_TRIVIAL(trace)<<"leave_room,player is "<<player->player_id();

    auto it =std::find(players_.begin(),players_.end(),player);
    if(it == players_.end())
    {
        MY_THROW(NO_JOIN_ROOM);
    }
    players_.erase(it);
    player->setPlayer_status(LOGIN_HALL);
    player->setRoom_id(0);
    //
    for_each(players_.begin(),players_.end(),[&](const audio_player::PTR& v){
         if(v->player_id() != player->player_id())
        {
            message_center::functions f = message_center::get_event("send_data");
              vector<boost::any> echo;
            echo.push_back(MEMBER_LEAVE_ROOM_ECHO);
            echo.push_back(v->client_id());
            echo.push_back(player->player_id());
            echo.push_back(room_id_);

            f(echo);
        }
    });

}
void audio_room::setRoom_id(const uint &room_id)
{
    room_id_ = room_id;
}
vector<audio_player::PTR> audio_room::players() const
{
    return players_;
}

void audio_room::setPlayers(const vector<audio_player::PTR> &players)
{
    players_ = players;
}



