#ifndef MESSAGE_CENTER_H
#define MESSAGE_CENTER_H
#include "stdafx.h"
#include "common.h"
using namespace boost;
class message_center
{
public:
    //

    typedef std::function<void(const vector<boost::any>& param)> functions;
    static void add_event(string s,functions f)
    {
        events_[s]=f;
    }
    static functions get_event(string s)
    {
        return events_.at(s);
    }
    message_center();
private:
    static map<string,functions> events_;

};

#endif // MESSAGE_CENTER_H
