#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H
#include "stdafx.h"
#include "common.h"
class audio_player:public boost::enable_shared_from_this<audio_player>
{
public:
      typedef std::shared_ptr<audio_player> PTR;
      audio_player();
      uint player_id() const;
      void setPlayer_id(const uint &player_id);

      PLAYER_STATUS player_status() const;
      void setPlayer_status(const PLAYER_STATUS &player_status);

      uint client_id() const;
      void setClient_id(const uint &client_id);

      uint room_id() const;
      void setRoom_id(const uint &room_id);
      short udp_port() const;
      void setUdp_port(short udp_port);


public:
      map<uint,bool> recv_audio_list_;
private:
      uint player_id_;
      uint client_id_;
      uint room_id_;
      short udp_port_;

      PLAYER_STATUS player_status_;
};

#endif // AUDIO_PLAYER_H
