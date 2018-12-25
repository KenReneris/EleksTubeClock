/*
 * EleksDigit.h
 *  1 per digit on the ElkesTube clock
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */


class EleksDigit
{
public:
    enum ValueType : uint8
    {
        Time,
        Effect,
        Blend,
        ValueTypeMax
    };

public:
    void        Initialize( CRGB *leds, uint8 hue );

    void        NextFrame( ARGB color, ValueType show );

    void        ClearRGB();
    bool        SetValue( ValueType index, uint value );
    uint8       GetTimeValue() const;
    ARGB        GetTimeColor() const;
    void        AppendDigit( StringBuffer *sb );

private:
    void        _SetColorWithBrightness( uint8 value, CRGB color );
    void        _SetColor( uint8 value, CRGB color );

    friend class SplashScreen;

private:
    static const uint8  k_posMap[];

private:
    CRGB      * _leds;                  // our 0 to 9 leds.  Note there are 2 leds per digit.  So this is 20 leds
    uint8       _value[ Effect+1 ];     // two values (for cross fade). [0]=time value. [1]=effect value. if the value > 9, then the digit is not displayed
    uint8       _values;                // bitmask of which _values have been set this frame.  
    ValueType   _lastEffect;            // debugging

    // rainbow effect
    uint8       _hue;                   // udpated every frame to given rainbow effect
    CRGB        _hueColor;              // color for _hue
};
