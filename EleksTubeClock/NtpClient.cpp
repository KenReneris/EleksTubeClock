/*
 * Ported from https://github.com/arduino-libraries/NTPClient with changes to make it
 * async for periodic updating while updating the clock.  And to integrate it into the 
 * desired clock behaviour (set syncing/error colors, etc..)
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * ----------------------------------------------------------
 */


#include "platform.h"
#include <WiFiUdp.h>              
#include "NtpClient.h"

#define SEVENZYYEARS                2208988800UL
#define NTP_PACKET_SIZE             48
#define NTP_DEFAULT_LOCAL_PORT      1337
#define MAX_NTP_RATE                10              // 10 minutes


static WiFiUDP                      g_ntpUdp;


void
NtpClient::Setup()
{
    _state = WaitingForWifi;
    g_ntpUdp.begin( NTP_DEFAULT_LOCAL_PORT );
}


void
NtpClient::ForceSync()
{
    if ( (_state == WaitingForSyncTime) || (_state == WaitingForRetry) )
    {
        _state = WaitingForSyncTime;
        _nextTime = g_poweredOnTime;
    }
}


NtpClient::State
NtpClient::GetState() const
{
    return _state;
}


char const * 
NtpClient::GetStateStr() const
{
    switch( _state )
    {
    case WaitingForWifi:            return "WaitingForWifi";
    case WaitingForSyncTime:        return "WaitingForSyncTime";
    case WaitingForResponse:        return "WaitingForResponse";
    case WaitingForRetry:           return "WaitingForRetry";
    }

    return nullptr;
}


String
NtpClient::LastSync() const
{
    if (_lastSync)
    {
        return Duration( g_poweredOnTime - _lastSync );
    }
    else
    {
        return "never";
    }
}


void
NtpClient::SendNtpPacket()
{
    byte    packetBuffer[ NTP_PACKET_SIZE ];
    int     result;

    Out( "Ntp: Send Requesst\n" );

    // if there was a previous timeout there could be an old packet waiting
    int cb = g_ntpUdp.parsePacket();
    while ( cb > 0 )
    {
        cb = g_ntpUdp.parsePacket();
    }

    memset( packetBuffer, 0, NTP_PACKET_SIZE );

    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    packetBuffer[ 0 ] = 0b11100011;   // LI, Version, Mode
    packetBuffer[ 1 ] = 0;     // Stratum, or type of clock
    packetBuffer[ 2 ] = 6;     // Polling Interval
    packetBuffer[ 3 ] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[ 12 ] = 49;
    packetBuffer[ 13 ] = 0x4E;
    packetBuffer[ 14 ] = 49;
    packetBuffer[ 15 ] = 52;

    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    g_ntpUdp.beginPacket( g_options._ntpServer, 123 );          // NTP requests are to port 123
    g_ntpUdp.write( packetBuffer, NTP_PACKET_SIZE );
    result = g_ntpUdp.endPacket();
    if ( result )
    {
        SetState( WaitingForResponse );
    }
    else
    {
        SetState( WaitingForRetry );
    }
}


void
NtpClient::Loop()
{
    switch ( _state )
    {
    case WaitingForWifi:
        if ( g_wifiIsConnected )
        {
            SendNtpPacket();
        }
        break;

    case WaitingForSyncTime:
    case WaitingForRetry:
        if ( g_poweredOnTime >= _nextTime )
        {
            if ( g_wifiIsConnected )
            {
                SendNtpPacket();
            }
            else
            {
                SetState( WaitingForWifi );
            }
        }
        break;

    case WaitingForResponse:
        CheckForResponse();
        break;
    }
}


void
NtpClient::CheckForResponse()
{
    int         cb;

    cb = g_ntpUdp.parsePacket();

    if ( cb != NTP_PACKET_SIZE ) 
    {
        if ( g_poweredOnTime > _nextTime )
        {
            Out( "Ntp: response timeout\n" );
            SetState( WaitingForRetry );
        }
    }
    else
    {
        byte    packetBuffer[ NTP_PACKET_SIZE ];

        g_ntpUdp.read( packetBuffer, NTP_PACKET_SIZE );

        auto read32 =
            []( byte *buffer ) -> uint32
            {
                uint32  highWord = word( buffer[0], buffer[1] );
                uint32  lowWord = word( buffer[2], buffer[3] );

                return ( (highWord << 16) | lowWord );
            };

        // 40-43 is transmit time
        uint32  secsSince1900 = read32( packetBuffer + 40 );
        uint32  ntpTime = secsSince1900 - SEVENZYYEARS;

        // 44-47 is fraction of time. e.g., 1/2^32 of a second.
        uint32  fraction = read32( packetBuffer + 44 );
        uint32  ms = fraction / 4294968;        // 0..999

        //
        _lastSync = g_poweredOnTime;
        _lastSyncHadDiff = SetNtpTime( ntpTime, ms );
        SetState( WaitingForSyncTime );
    }
}


void
NtpClient::SetState( State state )
{
    // handle old state
    switch( _state )
    {
    case WaitingForSyncTime:
        break;

    case WaitingForResponse:
        g_globalColor.ClearState( GlobalColor::SyncingNtpTime );
        break;

    case WaitingForRetry:
        break;
    }

    // new state
    _state = state;
    Out( "Ntp: %s\n", GetStateStr() );
    switch ( state )
    {
    case WaitingForWifi:
        break;

    case WaitingForSyncTime:
        UpdateNextTime();
        break;

    case WaitingForResponse:
        g_globalColor.EnableState( GlobalColor::SyncingNtpTime );
        _nextTime = g_poweredOnTime + 2;
        break;

    case WaitingForRetry:
        g_globalColor.PingState( GlobalColor::TimeError );
        _nextTime = g_poweredOnTime + MAX_NTP_RATE * 60;       // don't spam the server.  max rate is every 10 minutes
        break;
    }
}


void
NtpClient::UpdateNextTime()
{
    if ( _state == WaitingForSyncTime )
    {
        uint8   frequency = g_options._ntpFrequency;

        // if there was a diff, drop to 10 minute check.  at some point we should get a 0 diff with that frequency
        if ( _lastSyncHadDiff )
        {
            frequency = 1;
        }

        const time_t    curTime     = g_now;
        const time_t    nextTime    = g_options._ntpSync.NextTime( curTime );
        const uint      diff        = nextTime - curTime;
        uint            wait        = diff % ( frequency * (MAX_NTP_RATE*60) );

        // todo.. if the ntp syncs the time is set backwards a couple of seconds this can wake up quickly
        if ( wait == 0 )
        {
            wait = MAX_NTP_RATE * 60;
        }

        Out( "Ntp: waiting %d seconds\n", wait );
        _nextTime = g_poweredOnTime + wait;
    }
}

