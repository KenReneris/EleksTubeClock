/*
 * OnOff
 *  A simple OnOff schedule for the EleksTubeClock
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */


#include "platform.h"


void
OnOff::DigitalRead( uint gpioForceOn, uint gpioForceOff )
{
    _DigitalRead( gpioForceOn, _gpioOnTime );
    _DigitalRead( gpioForceOff, -_gpioOffTime );
}


void
OnOff::_DigitalRead( uint gpio, int seconds )
{
    if ( (gpio) && (seconds) && (g_options._madjEnable) )
    {
        if ( !digitalRead(gpio) )
        {
            String  m;

            m = ForceOnOff( seconds, true );
            if ( !!m )
            {
                Out( "OnOff: Gpio: %d, %s", gpio, m.c_str() );
            }
        }
    }
}


bool
OnOff::IsOn() const
{
    return const_cast<OnOff *>(this)->Loop();
}


bool
OnOff::Loop() 
{
    const time_t    curTime = g_now;
    bool            on      = false;
    tmElements_t    tm;

    breakTime( curTime, tm );

    tm.Wday -= 1;
    if ( _onDays & (1<<tm.Wday) )
    {
        on = true;
        if ( _onTime != _offTime )
        {
            const time_t  onTime    = _onTime.NextTime( curTime );
            const time_t  offTime   = _offTime.NextTime( curTime );
            
            if ( (curTime >= offTime) && (curTime < onTime) )
            {
                on = false;
            }
        }
    }
    

    if ( (_forceEnd) && (g_poweredOnTime < _forceEnd) )
    {
        on = _forceOn;
    }

    return on;
}


char const *
OnOff::Desc( bool onOff ) const
{
    char const   * onOffStr = "";
    
    // hack
    if (this == &g_options._dimOnOff)
    {
        onOffStr = (onOff ? "Dim" : "!Dim");
    }
    else
    {
        onOffStr = (onOff ? "On" : "Off");
    }

    return onOffStr;
}


String 
OnOff::ForceOnOff( const String &arg )
{
    return ForceOnOff( atoi( arg.c_str() ), true );
}


String
OnOff::ForceOnOff( int seconds, bool surpressMsg )
{
    bool            on      = true;

    if ( seconds < 0 )
    {
        on = false;
        seconds = -seconds;
    }

    if ( _forceOn != on )
    {
        _forceOn = on;
        _forceEnd = 0;
    }

    uint32 forceEnd = MAX( _forceEnd, g_poweredOnTime + seconds );
    if ( forceEnd != _forceEnd )
    {
        _forceEnd = forceEnd;
        return PrintF( "Forced %s %d seconds.  now %d\n", Desc(on), seconds, _forceEnd - g_poweredOnTime );
    }
    else
    {
        return String();
    }
}



