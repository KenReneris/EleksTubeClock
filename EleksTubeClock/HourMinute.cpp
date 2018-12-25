/*
 * HourMinute
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */

#include "platform.h"


HourMinute::HourMinute( const String & rh )
{
    _hour = 0;
    _minute = 0;
    if ( rh.length() == 5 )
    {
        _hour = atoi( rh.c_str() );
        _minute = atoi( rh.c_str()+3 );
    }
}


String 
HourMinute::toString() const
{
    char const  *amPm   = "";
    uint8       hh      = _hour;

    if ( g_options._12Hour )
    {
        amPm = "am";
        if ( hh > 12 )
        {
            hh -= 12;
            amPm = "pm";
        }

        if ( hh == 0 )
        {
            hh = 12;
        }
    }

    return PrintF( "%2d:%02d%s", hh, _minute, amPm );
}


bool 
HourMinute::operator != ( HourMinute const &rh ) const
{
    return (_hour != rh._hour) || (_minute != rh._minute);
}


time_t
HourMinute::NextTime( time_t curTime ) const
{ 
    tmElements_t        tm;
    
    breakTime( curTime, tm );
    
    tm.Hour     = _hour;
    tm.Minute   = _minute;
    tm.Second   = 0;

    // get aligned on users requested hh:min
    time_t    nextTime = makeTime( tm );
    if ( nextTime <= curTime )
    {
        nextTime += 24 * 60 * 60;
    }
    
    return nextTime;
}
    