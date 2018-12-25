/*
 * Argb
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */

#include "platform.h"


ARGB
ARGB::FromCRGB( const CRGB &rgb )
{
    ARGB    r;

    r.alpha = 0xFF;
    r.red   = rgb.red;
    r.green = rgb.green;
    r.blue  = rgb.blue;

    return r;
}


ARGB
ARGB::FromString( const String &str )
{
    char const    * p       = nullptr;
    bool            failed  = false;
    uint32          accum   = 0;
    ARGB            argb    = { 0, 0, 0, 0 };

    if ( str.length() == 6 )
    {
        p = str.c_str();
    }
    if ( (str.length() == 7) && (str[0] == '#') )
    {
        p = str.c_str() + 1;
    }

    if ( p )
    {
        for ( int index=0; index < 6; ++index )
        {
            accum <<= 4;

            const char c = p[ index ];
            if ( c >= '0' && c <= '9' )
            {
                accum += c - '0';
            }
            else if (c >= 'A' && c <= 'F')
            {
                accum += c - 'A' + 10;
            }
            else if (c >= 'a' && c <= 'f')
            {
                accum += c - 'a' + 10;
            }
            else
            {
                failed = true;
            }
        }

        if ( !failed )
        {
            argb.alpha  = 0xFF;
            argb.red    = ( accum >> 16 ) & 0xFF;
            argb.green  = ( accum >> 8 ) & 0xFF;
            argb.blue   = accum & 0xFF;
        }
    }

    return argb;
}


String
ARGB::toString() const
{
    return PrintF( "0x%02x%02x%02x%02x", this->alpha, this->red, this->green, this->blue );
}


String
ARGB::toRgbString() const
{
    return PrintF( "%02x%02x%02x", this->red, this->green, this->blue );
}

String
ARGB::toHtmlRgbString() const
{
    return PrintF( "#%02x%02x%02x", this->red, this->green, this->blue );
}
