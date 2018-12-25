/*
 * DimOnOff
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */


#include "platform.h"


bool
DimOnOff::IsDim() const
{
    return ( (g_options._bright != g_options._dim) && (_target == g_options._dim) );
}


void
DimOnOff::Loop()
{
    OnOff::DigitalRead( GPIO_FORCE_BRIGHT, GPIO_FORCE_DIM );
    _target = OnOff::Loop() ? g_options._dim : g_options._bright;
}


void
DimOnOff::NextFrame()
{
    if ( _target != g_brightness )
    {
        int8 const  diff    = _target - g_brightness;
        int8 const  step    = 8;
        int8 const  delta   = ( diff < 0 ? -step : +step );

        if ( ABS(diff) < step )
        {
            g_brightness = _target;
        }
        else
        {
            g_brightness += delta;
        }
    }
}

