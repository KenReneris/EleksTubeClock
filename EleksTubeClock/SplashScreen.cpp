/*
 * SplashScreen
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */


#include "platform.h"
#include "SplashScreen.h"


SplashScreen::SplashScreen( uint msDelay )
{
    CRGB    color{ 0xFF, 0xFF, 0xFF };
    
    for ( int rgb=7; rgb; --rgb )
    {
        color.red   = (rgb & 4) ? 0xFF : 0;
        color.green = (rgb & 2) ? 0xFF : 0;
        color.blue  = (rgb & 1) ? 0xFF : 0;

        for( uint value=0; value < 10; ++value )
        {
            for( uint digitIndex=0; digitIndex < NUM_DIGITS; ++digitIndex )
            {
                EleksDigit  & digit = g_digits[ digitIndex ];

                digit.ClearRGB();
                digit.SetValue( EleksDigit::Time, value );
                digit._SetColor( value, color );
            }
    
            // spin on ShowLeds while delaying to check for flickering
            const uint32    start   = millis();
            uint32          end     = start;

            while( (end-start) < msDelay )
            {
                FastLED.show();
                delay( 1 );
                end = millis();
            }
        }
    }
}

