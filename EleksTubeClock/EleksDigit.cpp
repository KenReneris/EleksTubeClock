/*
 * EleksDigit
 *  1 per digit on the ElkesTube clock
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */


#include "platform.h"


const uint8     EleksDigit::k_posMap[] = { 5, 0, 6, 1, 7, 2, 8, 3, 9, 4 };


void
EleksDigit::Initialize( CRGB *leds, uint8 hue )
{
    _leds = leds;
    _hue = hue;
}


void
EleksDigit::ClearRGB()
{
    _SetColor( _value[ Time ], CRGB::Black );
    _SetColor( _value[ Effect ], CRGB::Black );
    _values = 0;
}


bool
EleksDigit::SetValue( ValueType index, uint value )
{
    if ( index <= Effect )
    {
        _values |= ( 1 << index );
        _value[ index ] = value;
    }
}


uint8
EleksDigit::GetTimeValue() const
{
    return _value[ Time ];
}


ARGB
EleksDigit::GetTimeColor() const
{
    return ARGB::FromCRGB( _hueColor );
}


void
EleksDigit::NextFrame( ARGB effectColor, ValueType show )
{
    _hue += 1;
    _hueColor.setHue( _hue );

    if ( !effectColor.alpha )
    {
        // no effect
        _SetColorWithBrightness( _value[Time], _hueColor );
        _lastEffect = Time;
    }
    else
    {
        CRGB       rgb;
        
        rgb.red = effectColor.red;
        rgb.blue = effectColor.blue;
        rgb.green = effectColor.green;

        if ( effectColor.alpha == 0xFF )
        {
            show = Effect;
        }

        if ( !(_values & (1 << Effect)) )
        { 
            show = Time;
        }

        if ( (show == Blend) && (_value[Time] == _value[Effect]) )
        {
            show = Effect;
        }

        _lastEffect = show;
        if ( show != Blend )
        {
            rgb = blend( _hueColor, rgb, effectColor.alpha );
            _SetColorWithBrightness( _value[ show ], rgb );
        }
        else 
        {
            nscale8x3( rgb.r, rgb.g, rgb.b, effectColor.alpha );
            _SetColorWithBrightness( _value[ Effect ], rgb );
        
            rgb = _hueColor;
            nscale8x3( rgb.r, rgb.g, rgb.b, 255-effectColor.alpha );
            _SetColorWithBrightness( _value[ Time ], rgb );
        }
    }
}


void
EleksDigit::AppendDigit( StringBuffer *sb )
{
    auto add =
        [sb]( uint8 v )
        {
            sb->Add( (v < 10 ? ('0'+v) : 'x') );
        };

    if ( _lastEffect != Blend )
    {
        add( _value[_lastEffect] );
    }
    else
    {
        sb->Add( '[' );
        add( _value[Time] );
        add( _value[Effect] );
        sb->Add( ']' );
    }
}


void
EleksDigit::_SetColorWithBrightness( uint8 value, CRGB color )
{
    nscale8x3( color.r, color.g, color.b, g_brightness );
    _SetColor( value, color );
}


void
EleksDigit::_SetColor( uint8 value, CRGB color )
{
    if ( value < 10 )
    {
        const int     index   = k_posMap[ value ];

        _leds[ index ] = color;
        _leds[ index+10 ] = color;
    }
}

