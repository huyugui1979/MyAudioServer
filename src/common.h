#ifndef COMMON_H
#define COMMON_H
#include "stdafx.h"
enum  PLAYER_EVENT {CHECK_SERVER,UDP_HOLE,TIMER,LOGIN,LOGOUT,CREATE_ROOM,GET_ROOM_LIST,GET_ROOM_MEMBER,JOIN_ROOM,LEAVE_ROOM,AUDIO_DATA,RESET};
enum  PLAYER_STATUS{NOTLOGIN,LOGIN_HALL,JOINED_ROOM};
enum  SEVER_EVENT{CHECK_ECHO,LOGIN_ECHO,UDP_HOLE_ECHO,LOGOUT_ECHO,MEMBER_JOIN_ROOM_ECHO,MEMBER_LEAVE_ROOM_ECHO,CREATE_ROOM_ECHO,GET_ROOM_LIST_ECHO,GET_ROOM_MEMBER_ECHO,
                 JOIN_ROOM_ECHO,LEAVE_ROOM_ECHO,AUDIO_DATA_ECHO
                 };
enum  APP_ERROR{HAVE_LOGIN=100,NOT_LOGIN,HAVE_JOIN_ROOM,NO_JOIN_ROOM,NO_ROOM};
struct my_execption :
    virtual std::exception,
    virtual boost::exception
{
};
typedef boost::error_info<struct tag_err_no, int> err_no;
typedef boost::error_info<struct tag_err_str, wstring> err_str;

#define MY_THROW(exepction_no) throw my_execption()<<err_no(exepction_no)\
                                      <<errinfo_api_function(__FILE__)\
                                      <<errinfo_errno(__LINE__);


#endif // COMMON_H
