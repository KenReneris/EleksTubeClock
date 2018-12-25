/*
 * HourMinute.h
 *  Structure for holding HH:MM
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */


struct HourMinute
{
    uint8       _hour;
    uint8       _minute;

public:
    HourMinute() = default;
    HourMinute( const String &rh );                     // build form HH:MM

    String      toString() const;
    time_t      NextTime( time_t curTime ) const;       // next time HH::MM occurs
    bool        operator != ( HourMinute const &rh ) const;
};
