/*
 * ARGB.h
 *  RGB with Alpha.
 *  The alha byte is overloaded for effect types.
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */


struct ARGB
{
    uint8   alpha;
    uint8   red;
    uint8   green;
    uint8   blue;

public:
    static ARGB     FromCRGB( const CRGB &rgb );
    static ARGB     FromString( const String &str );

public:
    String  toString() const;
    String  toRgbString() const;
    String  toHtmlRgbString() const;

    // encoding of alpha in g_options:
    enum Flags : uint8
    {
        HoldTimeMask    = 0x0F,
        EffectMask      = 0xF0,         
        Appear          = 0x00,         // Effect values & colors, hold, time value and colors
        CrossFade       = 0x10,         // fade to black, fade effect values & color on, hold, fade to black, fade time values & colors on
        BlendInOut      = 0x20,         // blend time values to effect values & color, hold, blend effect values to time values & colors
        BlendOut        = 0x30,         // appear, blend out (used for sync code)
        Flash           = 0x40,         // HoldTime is number of flashes
    };
};

