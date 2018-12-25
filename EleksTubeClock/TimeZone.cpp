/*
 * TimeZone
 *  Note timezone updates are synchronous (the clock will stop when updating)
 *  Thanks to https://github.com/ib134866/EleksTube for the basic usage & idea*
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * ----------------------------------------------------------
 */


#include "platform.h"
#include <ArduinoJson.h>                // json library for parsing http results - https://arduinojson.org/?utm_source=meta&utm_medium=library.properties
#include <ESP8266HTTPClient.h>     
#include "TimeZone.h"


void
TimeZone::Setup()
{
    _state = ( g_options._tzKey[0] ? WaitingForWifi : Disabled );
    _isValid = false;
    _isDst = false;
    _dstStart = 0;
    _dstEnd = 0;
}


void
TimeZone::UserSyncNow()
{
    if ( _state == WaitingForSyncTime )
    {
        _nextTime = g_poweredOnTime;
    }
}


TimeZone::State
TimeZone::GetState() const
{
    return _state;
}


char const *
TimeZone::GetStateStr() const
{
    switch ( _state )
    {
    case Disabled:            return "Disabled";
    case WaitingForWifi:      return "WaitingForWifi";
    case WaitingForSyncTime:  return "WaitingForSyncTime";
    case SyncingTimeZone:     return "SyncingTimeZone";
    case WaitingForRetry:     return "WaitingForRetry";
    }

    return nullptr;
}


void
TimeZone::Loop()
{
    switch ( _state )
    {

    case Disabled:
    case SyncingTimeZone:
    case WaitingForRetry:
        break;

    case WaitingForWifi:
        if ( g_wifiIsConnected )
        {
            UpdateTimeZone();
        }
        break;

    case WaitingForSyncTime:
        if ( g_poweredOnTime >= _nextTime )
        {
            if ( g_wifiIsConnected )
            {
                UpdateTimeZone();
            }
            else
            {
                SetState( WaitingForWifi );
            }
        }
        break;
    }
}


void
TimeZone::SetState( State state )
{
    if ( _state != state )
    {
        // handle old state
        switch( _state )
        {
        case WaitingForSyncTime:
            break;

        case SyncingTimeZone:
            g_globalColor.ClearState( GlobalColor::SyncingTimeZone );
            break;

        case WaitingForRetry:
            break;
        }

        // new state
        _state = state;
        Out( "TimeZone: %s\n", GetStateStr() );
        switch ( state )
        {
        case WaitingForSyncTime:
            UpdateNextTime();
            break;

        case SyncingTimeZone:
            g_globalColor.EnableState( GlobalColor::SyncingTimeZone );
            break;

        case WaitingForRetry:
            g_globalColor.PingState( GlobalColor::TimeError );
            _nextTime = g_poweredOnTime + 4 * 60 * 60;        // retry in 4 hours
            break;
        }
    }
}


JsonVariant
GetJsonField( TimeZone *client, const JsonObject &root, char const *name )
{
    if ( !root.containsKey(name) )
    {
        client->JsonParseFailure( name );
    }

    return root[ name ];
}


JsonObject 
GetJsonResponse( TimeZone *client, String url, DynamicJsonDocument &jsonDoc )
{
    HTTPClient              http;
    String                  str;

    http.begin( url );
    http.GET();
    str = http.getString();
    http.end();

    deserializeJson( jsonDoc, str );
    return jsonDoc.as<JsonObject>();
}


void
TimeZone::UpdateTimeZone()
{
    String                  url;
    int32                   gmtOffset;

    SetState( SyncingTimeZone );
    Out( "TimeZone: Update\n" );
    _isValid = true;

    {
        DynamicJsonDocument     jsonDoc;
        JsonObject              root        = GetJsonResponse( this, "http://ip-api.com/json/?fields=timezone", jsonDoc );
        String                  timeZone    = GetJsonField( this, root, "timezone" );

        url = PrintF( "http://api.timezonedb.com/v2.1/get-time-zone?key=%s&format=json&by=zone&zone=%s", g_options._tzKey, timeZone.c_str() );
    }

    {
        DynamicJsonDocument     jsonDoc;
        JsonObject              root = GetJsonResponse( this, url, jsonDoc );

        gmtOffset       = GetJsonField( this, root, "gmtOffset" );
        _isDst          = GetJsonField( this, root, "dst" );
        _dstStart       = GetJsonField( this, root, "zoneStart" );
        _dstEnd         = GetJsonField( this, root, "zoneEnd" );
    }

    if ( _isValid )
    {
        if ( gmtOffset != g_options._gmtOffset )
        {
            Out( "TimeZone: gmtOffset %d\n", gmtOffset );
            g_options._gmtOffset = gmtOffset;
            g_options.Save();
        }
        else
        {
            Out( "TimeZone: checked.  no change\n" );
        }
        SetState( WaitingForSyncTime );
    }
    else
    {
        SetState( WaitingForRetry );
    }
}


void
TimeZone::JsonParseFailure( char const * fieldName )
{
    if ( _isValid )
    {
        Out( "TimeZone: update failure on field '%s'\n", fieldName );
        _isValid = false;
    }
}


void
TimeZone::UpdateNextTime()
{
    if( g_options._tzKey[0] )
    {
        if ( _state == Disabled )
        {
            SetState( WaitingForWifi );
        }
    }
    else
    {
        SetState( Disabled );
    }

    if ( _state == WaitingForSyncTime )
    {
        // this "mostly" works
        // (it's unclear what time the server precisley used for the next start & end as compared to our time)

        uint32 const  dstStart  = ( _dstStart - g_now );
        uint32 const  dstEnd    = ( _dstEnd - g_now );
        uint32  wait            = MIN( dstStart, dstEnd );

        if ( !_dstEnd )
        {
            wait = 365 * 24 * 60 * 60;
        }
    
        // give some slack
        wait += 10;
        Out( "Next Tz sync %d seconds\n", wait );

        _nextTime = g_poweredOnTime + wait;
    }
}
