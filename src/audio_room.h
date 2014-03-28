#ifndef AUDIO_ROOM_H
#define AUDIO_ROOM_H
#include "common.h"
#include "message_center.h"
#include "stdafx.h"
#include "audio_player.h"
class audio_room:public std::enable_shared_from_this<audio_room>
{

public:
    typedef std::shared_ptr<audio_room> PTR;
    audio_room();
    void join_room(audio_player::PTR player);

    void send_data_room(audio_player::PTR player,char* data,uint len);


    void leave_room(audio_player::PTR player);

    uint room_id() const;
    void setRoom_id(const uint &room_id);

    void get_players( vector<audio_player::PTR>& players);
    uint get_player_count();

	bool is_idle() const {
		return idle_;
	}

private:
    uint room_id_;
    vector<audio_player::PTR> players_;
    std::mutex mutex_;
    bool idle_=true;
};

#endif // AUDIO_ROOM_H
