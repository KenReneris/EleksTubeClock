/*
 * ElekTubeClock
 *  Replacement (ESP8266) controller and firmware for the ElkesTube clock as found on:
 *      www.kickstarter.com/projects/938509544/elekstube-a-time-machine
 *      www.banggood.com/EleksMaker-EleksTube-Bamboo-6-Bit-Kit-Time-Electronic-Glow-Tube-Clock-Time-Flies-Lapse-p-1297292.html
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */

#include "platform.h"
#include "ResetButton.h"
#include "WebServer.h"
#include "WiFiAp.h"
#include "SplashScreen.h"
#include "NtpClient.h"
#include "TimeZone.h"
#include "Console.h"


enum class UpdateType
{
    Start,
    Ntp,
    TimeZone,
    OnOff,
    Dim,
    End,
};


#define NUM_LEDS            ( NUM_DIGITS * 20 )
#define APPROX_FPS          60
#define MS_PER_FRAME        (1000 / APPROX_FPS)  

// misc globals
EleksDigit          g_digits[ NUM_DIGITS ];         // Digits of the elekstube display
static CRGB         g_leds[ NUM_LEDS ];             // Fastled array
static uint8        g_showLeds;                     // stop calling fastled.show() when off
static time_t       g_lastTime;                     // to notice when the time changes
static uint32       g_lastFrame;                    // to notice if there's been lag
bool                g_wifiIsConnected;              // global of last wifi connection status
uint8               g_brightness;                   // the current brightness to display at. 0..255
uint64              g_frameCount;                   // # of frame events
uint64              g_frameOff;                     // # of frame events where the leds where off and not updated
uint32              g_framesLag;                    // # of times there was some sort of lag
static UpdateType   g_update;                       // extra compute (in shadow of led update).  round-robin polling 
static Console      g_console;                      // global instance of console 
Options             g_options;                      // global instance of user settings
WiFiAp              g_wifiAp;                       // global instance of soft AP (and webserver)
NtpClient           g_ntp;                          // global instance of ntp client time sync
TimeZone            g_timeZone;                     // global instance of timezone time sync
GlobalColor         g_globalColor;                  // global instance of global color state
ResetButton         g_resetButton;                  // global instance of reset button handler

static char         g_popup[ NUM_DIGITS ];          // (1 set of) values for a popup 
ARGB                g_popupColor;                   // the requested color/effect of the popup


void
setup()
{
    Serial.begin( 115200 );
    ets_install_putc1( (void *) &OutC );
    Out( "\n\n\nEleksTube: Initialize (setup)\n" );

    pinMode( BUILTIN_LED, OUTPUT );
    pinMode( FLASH_BUTTON_PIN, INPUT );
    PinModePullUp( GPIO_FORCE_TIMEON );
    PinModePullUp( GPIO_FORCE_TIMEOFF );
    PinModePullUp( GPIO_FORCE_BRIGHT );
    PinModePullUp( GPIO_FORCE_DIM );

    FastLED.addLeds<NEOPIXEL, NEOLED_PIN>( g_leds, countof(g_leds) );

    for ( int pos = 0; pos < countof(g_digits); ++pos )
    {
        g_digits[ pos ].Initialize( &g_leds[pos * 20], (255/countof(g_digits)) * pos );
    }

    g_update = UpdateType::Start;
    g_options.Setup();
    g_options.Load();

    if ( g_options._splashScreen )
    {
        SplashScreen( 25 );
    }

    g_wifiAp.Setup();
    g_ntp.Setup();
    g_timeZone.Setup();
    g_lastFrame = millis();
    g_brightness = g_options._bright;
    Log( "\n" );
}


void
PinModePullUp( int gpio )
{
    if ( gpio )
    {
        pinMode( gpio, INPUT_PULLUP );
    }
}


void
OutC( char c )
{
    ets_putc( c );
}


void
OutStr( char const *str )
{
    Out( "%s", str );
}


void
loop()
{
    UpdateTime();

    if ( g_ms - g_lastFrame >= MS_PER_FRAME )
    {
        // advance smmoothly
        g_lastFrame += MS_PER_FRAME;
        
        // missed frames?
        if ( g_lastFrame < g_ms )
        {
            g_lastFrame = g_ms;
            if ( g_showLeds )
            {
                g_framesLag += 1;
            }
        }
        
        NextFrame();
        yield();
        
        g_update = UpdateType( int(g_update) + 1 );
        switch ( g_update )
        {
        case UpdateType::Ntp:
            g_ntp.Loop();
            break;
        
        case UpdateType::TimeZone:
            g_timeZone.Loop();
            break;
        
        case UpdateType::OnOff:
            g_options._timeOnOff.Loop();
            g_resetButton.Loop( !digitalRead(FLASH_BUTTON_PIN) );
            break;

        case UpdateType::Dim:
            g_options._dimOnOff.Loop();
            break;

        case UpdateType::End:
            g_update = UpdateType::Start;
            break;
        }

        g_console.Loop();
        g_wifiAp.Loop();
    }

    // scan faster when expect responses
    if ( g_ntp.GetState() == NtpClient::WaitingForResponse )
    {
        g_ntp.Loop();
    }
}


void
UpdateWaitTimes()
{
    // skip if not running yet
    if ( g_poweredOnTime )
    { 
        // Settings or Time changed
        g_ntp.UpdateNextTime();
        g_timeZone.UpdateNextTime();
        g_wifiAp.UpdateNextTime();

        if ( !g_options._madjEnable )
        {
            g_options._madjFreq = 0;
            g_options._madjSmooth.Reset();
        }
    }
}


void
NextFrame()
{    
    EleksDigit::ValueType   digitShow;
    ARGB                    globalColor;
    tmElements_t            tm;

    digitShow = EleksDigit::Time;
    globalColor.alpha = 0;

    breakTime( g_now, tm );

    for ( int index = 0; index < sizeof( g_popup ); ++index )
    {
        g_digits[ index ].ClearRGB();
    }

    if ( g_now != g_lastTime )
    {
        g_lastTime = g_now;
        digitalWrite( BUILTIN_LED, LOW );

        if ( !tm.Second )
        {
            if ( !tm.Minute )
            {
                g_globalColor.PingState( GlobalColor::TopOfHour );
            }
            else if ( (tm.Minute % 15) == 0 )
            {
                g_globalColor.PingState( GlobalColor::QuarterOfHour );
            }
        }
    
        if ( (tm.Second == g_options._dateSecond) && ((tm.Minute % g_options._dateMinutes) == 0) )
        {
            g_globalColor.PingState( GlobalColor::Date );
        }

        SetDigitsXXXXXX( EleksDigit::Time, HourMode(tm.Hour), tm.Minute, tm.Second );
    }

    // set the value for the effect
    switch( g_globalColor.IsDisplaying() )
    {
    case GlobalColor::Popup:
        for ( int index=0; index < sizeof(g_popup); ++index )
        {
            g_digits[ index ].SetValue( EleksDigit::Effect, g_popup[ index ] );
        }
        digitShow = EleksDigit::Effect;
        break;

    case GlobalColor::Date:
        tm.Year -= 30;
        if ( g_options._dateMmddyy )
        {
            SetDigitsXXXXXX( EleksDigit::Effect, tm.Month, tm.Day, tm.Year );
        }
        else
        { 
            SetDigitsXXXXXX( EleksDigit::Effect, tm.Year, tm.Month, tm.Day );
        }
        digitShow = EleksDigit::Effect;
        break;

    case GlobalColor::TimeNotSet:
        SetDigitsTimer( g_poweredOnTime );
        break;

    default:
        break;
    }

    if ( g_globalColor.UseGlobalColor() )
    {
        globalColor = g_globalColor.GetColor();
        if ( g_globalColor.IsBlend() )
        {
            digitShow = EleksDigit::Blend;
        }
        g_globalColor.NextFrame();
    }

    for ( int pos = 0; pos < countof( g_digits ); ++pos )
    {
        g_digits[ pos ].NextFrame( globalColor, digitShow );
    }

    g_options._dimOnOff.NextFrame();
    ShowLeds();
}


void
Popup1x6( char c, ARGB color )
{
    char    str[ countof(g_popup)+1 ];

    c += '0';
    for ( int index=0; index < countof(g_popup); ++index )
    {
        str[ index ] = c;
    }
    str[ countof(g_popup) ] = 0;
    Popup( str, color );
}


void
Popup( char const *str, ARGB color )
{
    int     len     = strlen( str );
    int     offset  = 0;
    
    Out( "Popup: %s, %s\n", str, color.toString().c_str() );
    if ( len > countof(g_popup) )
    {
        len = countof(g_popup);
    }

    memset( g_popup, 10, countof(g_popup) );
    offset = countof( g_popup ) - len;
    for ( int index=0; index < len; ++index )
    {
        g_popup[ index+offset ] = str[ index ] - '0';
    }
    g_popupColor = color;
    g_globalColor.PingState( GlobalColor::Popup );
}



void
OutNl()
{
  Out( "\n" );
}


String
TimeStr()
{
    tmElements_t            tm;

    breakTime( g_now, tm );
    tm.Year -= 30;
    return PrintF( "%02d:%02d:%02d %02d/%02d/%2d", tm.Hour, tm.Minute, tm.Second, tm.Year, tm.Month, tm.Day );
}


int
HourMode( int hour )
{
    if ( g_options._12Hour )
    {
        if ( hour > 12 )
        {
            hour -= 12;
        }
        else if ( hour == 0 )
        {
            hour = 12;
        }
    }   
    
    return hour;
}


void
SetDigitsTimer( uint32 time )
{
    int     hours       = time / 60 * 60;
    int     minutes     = ( time - hours*60*60 ) / 60;
    int     seconds     = ( time - hours*60*60 - minutes*60 );

    SetDigitsXX( EleksDigit::Time, &g_digits[0], hours );
    SetDigitsXX( EleksDigit::Time, &g_digits[2], minutes );
    SetDigitsXX( EleksDigit::Time, &g_digits[4], seconds );

    for ( int index = 0; (index < NUM_DIGITS) && (!g_digits[index].GetTimeValue()); ++index )
    {
        g_digits[ index ].SetValue( EleksDigit::Time, 10 );
    }
}


void
SetDigitsXX( EleksDigit::ValueType index, EleksDigit *digit, int value )
{
    int   x0 = value / 10;
    int   x1 = value % 10;

    digit[ 0 ].SetValue( index, x0 );
    digit[ 1 ].SetValue( index, x1 );
}


void
SetDigitsXXXXXX( EleksDigit::ValueType index, int x01, int x23, int x45 )
{
    SetDigitsXX( index, &g_digits[0], x01 );
    SetDigitsXX( index, &g_digits[2], x23 );
    SetDigitsXX( index, &g_digits[4], x45 );

    if ( (g_options._surpressLeadingZero) && (x01 < 10) )
    {
        g_digits[ 0 ].SetValue( index, 10 );
    }
}


void
ShowLeds()
{
    g_frameCount += 1;

    if ( !g_globalColor.IsTurnedOff() )
    { 
        g_showLeds = 4;
    }

    if ( g_showLeds )
    {
        g_showLeds -= 1;
        FastLED.show();
    }
    else
    {
        g_frameOff += 1;
    }

    digitalWrite( BUILTIN_LED, HIGH );
}
