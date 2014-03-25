#include "audio_player.h"

audio_player::audio_player()
{

}
uint audio_player::player_id() const
{
    return player_id_;

}
short audio_player::udp_port() const
{
    return udp_port_;
}

void audio_player::setUdp_port(short udp_port)
{
    udp_port_ = udp_port;
}
void audio_player::setPlayer_id(const uint &player_id)
{
    player_id_ = player_id;
}
PLAYER_STATUS audio_player::player_status() const
{
    return player_status_;
}

void audio_player::setPlayer_status(const PLAYER_STATUS &player_status)
{
    player_status_ = player_status;
}
uint audio_player::client_id() const
{
    return client_id_;
}

void audio_player::setClient_id(const uint &client_id)
{
    client_id_ = client_id;
}
uint audio_player::room_id() const
{
    return room_id_;
}

void audio_player::setRoom_id(const uint &room_id)
{
    room_id_ = room_id;
}






