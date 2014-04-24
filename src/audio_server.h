#ifndef AUDIOSERVER_H
#define AUDIOSERVER_H
#include "stdafx.h"
#include "common.h"
#include "audio_client.h"
#include "audio_player.h"
#include "message_center.h"
using namespace boost::asio::ip;
using namespace boost::asio;
class audio_server
{
public:
    audio_server(boost::asio::io_service& io_service, short port);

protected:
    void wait_scan_player_timer();
    void wait_scan_room_timer();
    void on_client_reset(uint client_id);


    void send_data(const vector<boost::any>& params);
    void get_address(const vector<boost::any>& params);

    void do_accept();
private:
    map<uint,audio_client::PTR > clients_;
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    boost::asio::deadline_timer scan_player_timer_;
    uint client_index_=0;
    std::mutex mutex_;
};


#endif // AUDIOSERVER_H
