#ifndef TIME_H
#define TIME_H

#include <string>
#include <iomanip>
#include <sstream>

static long add_hours(long update, long time)
{
    return time + update*3600000;
}

static long add_minutes(long update, long time)
{
    return time + update*60000;
}

static long add_seconds(long update, long time)
{
    return time + update*1000;
}

static long add_millis(long update, long time)
{
    return time + update;
}

static long string_to_time(std::string s)
{
    int hour = stoi(s.substr(0, 2)),
        min = stoi(s.substr(3, 2)),
        sec = stoi(s.substr(6, 2)),
        mil = stoi(s.substr(9, 3));

    return add_hours(hour,
                     add_minutes(min,
                                 add_seconds(sec,
                                             add_millis(mil, 0))));
}

static std::string time_to_string(long t)
{
    int mil, sec, min;

    mil = t % 1000;
    t /= 1000;

    sec = t % 60;
    t /= 60;

    min = t % 60;
    t /= 60;

    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2);
    ss << t << ":";
    ss << std::setfill('0') << std::setw(2);
    ss << min << ":";
    ss << std::setfill('0') << std::setw(2);
    ss << sec << ".";
    ss << mil;

    std::string s;
    ss >> s;

    return s;
}

#endif
