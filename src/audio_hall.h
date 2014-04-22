#ifndef AUDIO_HALL_H
#define AUDIO_HALL_H
#include "stdafx.h"
#include "common.h"
#include "message_center.h"
#include "audio_server.h"
#include "audio_room.h"
class audio_hall:public std::enable_shared_from_this<audio_hall>
{
public:

    audio_hall(boost::asio::io_service& io);
protected:
    void process_player_command(const vector<boost::any>& param);
    void check_login_in(uint client_id);
    void reset_player(const vector<boost::any>& param);
    void scan_room();
    void on_destroy_room(uint client_id,uint room_id);
    void check_not_have_room(uint room_id);
    void check_have_room(uint room_id);
    void check_not_login_in(uint client_id);
    void check_room_have_player(uint room_id);
    void on_create_room(uint client_id,uint player_id);
    void on_set_talk_list(uint client_id,vector<uint> list);

    void on_logout_event(uint client_id);

    void on_login_event(uint client_id, uint player_id,short port);

    void on_audio_data(uint client_id,char* data,uint len);

    void on_set_recv_audio(uint client_id,uint player_id,bool b);
    void on_get_room_member(uint client_id);
    void on_begin_talk(uint client_id);
    void on_stop_talk(uint client_id);

    void on_get_room_list(uint client_id);

    void on_join_room_event(uint client_id,uint room_id);

    void on_leave_room_event(uint client_id,uint room_id);
    void wait_scan_room_timer();

private:
    map<uint,audio_player::PTR>  players_;
    map<uint,audio_room::PTR> rooms_;
    std::mutex mutex_;
    boost::asio::io_service& io_;
    boost::asio::deadline_timer scan_room_timer_;
};

#endif // AUDIO_HALL_H
