/*
 * TimeOnOff
 *  Schedule for whenn the time should be displayed or not.
 *  All the functionality is in OnOff.cpp
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */

#include "platform.h"

void
TimeOnOff::Loop()
{
    OnOff::DigitalRead( GPIO_FORCE_TIMEON, GPIO_FORCE_TIMEOFF );
    g_globalColor.EnableState( GlobalColor::TimeOff, !OnOff::Loop() );
}


