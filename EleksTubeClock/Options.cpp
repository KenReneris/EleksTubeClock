/*
 * Options
 *  User settings presisted to flash.
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */

#include "platform.h"
#include <EEPROM.h>

#define VERSION     1

void
Options::Setup()
{
    EEPROM.begin( 512 );        // Initialize EEPROM
}


void
Options::Load()
{
    uint8   * const start   = (uint8 *) this;

    for( int pos=0; pos < sizeof(*this); ++pos )
    {
        start[ pos ] = EEPROM.read( pos );
    }
    _tzKey[ countof(_tzKey)-1 ] = 0;

    if ( (_checksum != Checksum()) || (_version != VERSION) || (!_dateMinutes) || (!_ntpFrequency) )
    {
        Reset();
    }
}


void
Options::Save()
{
    uint8   * const start = (uint8 *) this;

    Out( "Options: Save\n" );
    _checksum = Checksum();
    _dirty = false;
    for (int pos = 0; pos < sizeof(*this); ++pos)
    {
        EEPROM.write( pos, start[pos] );
    }
    EEPROM.commit();
    UpdateWaitTimes();
}


void
Options::Reset()
{
    Out( "Options: Reset\n" );
    memset( this, 0, sizeof(*this) );

    _version                = VERSION;

    strcpy( _ssid,      "EleksTube" );
    strcpy( _ntpServer, "time.google.com" );

    _splashScreen           = true;

    _timeOnOff._onDays      = 0x7F;
    _timeOnOff._gpioOnTime  = 10;
    _timeOnOff._gpioOffTime = 8*60*60;

    _dimOnOff._gpioOnTime   = 8*60*60;
    _dimOnOff._gpioOffTime  = 10;
    _bright                 = 0xFF;
    _dim                    = 0x80;

    _topOfHour              = { (ARGB::BlendOut | 2),  0xFF, 0xFF, 0xFF };          // white
    _quarterOfHour          = { (ARGB::BlendOut | 1),  0x00, 0xA0, 0xA0 };          // cyan

    _ntpColor               = { (ARGB::BlendInOut | 0),  0xFF, 0xFF, 0x00 };        // yellow
    _ntpSync._hour          = 4;
    _ntpSync._minute        = 10;
    _ntpFrequency           = 144;                                                  // daily

    _madjEnable             = true;

    _dateColor              = { (ARGB::CrossFade | 3), 0x00, 0xFF, 0x00 };          // green
    _dateMinutes            = 1;
    _dateSecond             = 30;
    _dateMmddyy             = true;

    _tzColor                = { (ARGB::BlendInOut | 1),  0xFF, 0xFF, 0x00 };        // yellow
    _httpClient             = { (ARGB::BlendOut | 3),  0xFF, 0xA5, 0x00 };          // orange

    _accessPointLifespan    = 60;
    _allowPopupUrl          = true;
    _allowBrightnessUrl     = true;
    Save();
}


uint32
Options::Checksum()
{
    uint8   * const start           = (uint8 *) this;
    uint32          checksum        = 0;
    const uint32    oldChecksum     = _checksum;

    _checksum = 0;
    for ( int pos = 0; pos < sizeof(*this); ++pos )   
    {
        checksum = (checksum << 1) + (start[ pos ] ^ 0x5A);
    }
    _checksum = oldChecksum;

    checksum &= 0x00FFFFFF;
    checksum |= ( sizeof(*this) << 24 );

    return checksum;
}

