/*
 * Platform.h
 *  Global includes
 *  Global macros
 *  Global globals...
 *
 */


#include <Arduino.h> 
#include <ESP8266WiFi.h> 
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <TimeLib.h>            // https://github.com/PaulStoffregen/Time

#define NEOLED_PIN              14      // D5
#define FLASH_BUTTON_PIN        0      
#define GPIO_FORCE_TIMEON       0       // 5       // D1    (0 == disabled)
#define GPIO_FORCE_TIMEOFF      0       // 4       // D2
#define GPIO_FORCE_BRIGHT       0       // 12      // D6 
#define GPIO_FORCE_DIM          0       // 13      // D7

#define FASTLED_INTERNAL        // remove various compile time debug output
#include <FastLED.h>            // https://github.com/FastLED/FastLED

#define NUM_DIGITS  6

// simple types
using   int8    = int8_t;
using   int16   = int16_t;
using   int64   = int64_t;

using   uint8   = uint8_t;
using   uint16  = uint16_t;

using   uint    = unsigned int;


// simple macros
#define countof(a)  ( (int) ( sizeof(a)/sizeof(a[0]) ) )
#define ABS(a)      ( ((a) < 0) ? (-a) : (a) )
#define MAX(a,b)    ( (a) > (b) ? (a) : (b) )
#define MIN(a,b)    ( (a) < (b) ? (a) : (b) )
#define Out         ets_printf


// our common headers
#include "Time.h"
#include "Bits.h"
#include "Argb.h"
#include "ZString.h"
#include "Log.h"
#include "HourMinute.h"
#include "OnOff.h"
#include "TimeOnOff.h"
#include "DimOnOff.h"
#include "Smooth.h"
#include "Options.h"
#include "GlobalColor.h"
#include "EleksDigit.h"


// globals
extern EleksDigit   g_digits[];
extern uint64       g_frameCount;           // number of frames (at 60fps)
extern uint64       g_frameOff;             // number of frames skipped due to being off
extern uint32       g_framesLag;            // number of times a frame was late / laggy
extern bool         g_wifiIsConnected;
extern uint8        g_brightness;

extern void         OutC( char c );
extern void         OutStr( char const *str );
extern void         ShowLeds();
extern void         PrintTime();
extern void         OutNl();
extern void         UpdateWaitTimes();
extern String       TimeStr();

extern void         Popup( String const &str, ARGB color );
extern void         Popup( char const *str, ARGB color );
extern void         Popup1x6( char c, ARGB color );
