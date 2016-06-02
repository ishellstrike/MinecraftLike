#include "AutoVersion.h"
#include <stdio.h>
#include <time.h>

std::string AutoVersion::GetTitle()
{
    if(ready_){
        return title_;
    }
    char buff[100];
    std::string s1 = Time;
    time_t t = atol(s1.c_str());
    std::string s2 = Ver;
    struct tm ts;
    localtime_s(&ts, &t);
    char buf[80];
    strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S", &ts);
    sprintf_s(buff, "%s %s %s %s\0", "Alpha Mensae", s2.c_str(), "from", buf);
    title_ = buff;
    ready_ = true;
    return title_;
}

bool AutoVersion::ready_ = false;
std::string AutoVersion::title_;