/*
 * ResetButton
 *  Monitors holding down of the ESP8266 flash button and will reset all settings if held down.
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */

#include "platform.h"
#include "ResetButton.h"


void
ResetButton::Loop( bool pressed )
{
    if ( !pressed )
    {
        _pressed = 0;
    }
    else
    {
        if ( !_pressed )
        {
            _pressed = g_ms;
        }

        const uint32    diff        = g_ms - _pressed;
        const uint32    seconds     = diff / 1000;
        const int32     remaining   = 5 - seconds;

        if ( remaining >= 0 )
        {
            ARGB        color = { 2, 0xFF, 0, 0 };
            char        str[ 8 ];

            sprintf( str, "%d", remaining );
            Popup( str, color );
        }
        else
        {
            FullReset( true );
        }
    }
}


void
ResetButton::FullReset( bool preserveName )
{
    char    prefixName[ sizeof( g_options._prefixName ) ];

    Out( "Reset: RESET\n" );
    strcpy( prefixName, g_options._prefixName );
    g_options.Reset();          // clear our settings
    if ( preserveName )
    {
        strcpy( g_options._prefixName, prefixName );
    }

    WiFi.disconnect( true );    // clear wifi settings
    system_restart();
    Out( "Reset: Failed?\n" );
    for (; ; );
}
