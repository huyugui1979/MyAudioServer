#ifndef AUDIOCLIENT_H
#define AUDIOCLIENT_H
#include "stdafx.h"
#include "message_center.h"
using namespace boost::asio::ip;
class audio_client : public std::enable_shared_from_this<audio_client>
{
public:

    typedef std::shared_ptr<audio_client> PTR;
    boost::signals2::signal<void(uint)> reset_client_event;

    explicit audio_client(tcp::socket socket);
    void send_data(char* data,size_t len);
    void read_head(const boost::system::error_code ec, std::size_t length);
    void read_data(const boost::system::error_code ec, std::size_t length);
    void do_read_head();

    void reset();

    uint client_id() const;
    void setClient_id(const uint &client_id);
    bool online(){return online_;}
    void setOnline(const bool& online){online_=online;}
    int client_ip() const;
    void setClient_ip(int client_ip);


    boost::asio::ip::tcp::socket socket_;
    uint client_id_;
    int online_;
    ushort len_;
    char data_[1024];

    int   client_ip_;


private:


    boost::asio::io_service::strand strand_;

};
#endif // AUDIOCLIENT_H
