/*
 * WiFiAp
 *  WiFi Access Point
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */


#include "platform.h"
#include "WebServer.h"
#include "WiFiAp.h"


void
WiFiAp::Setup()
{
    _status = wl_status_t(-1);
    WiFi.enableAP( false );
    WiFi.hostname( g_options._ssid );
    WiFi.softAP( g_options._ssid );
    Out( "SoftAp: %s\n", WiFi.softAPIP().toString().c_str() );

    _dnsServer.setErrorReplyCode( DNSReplyCode::NoError );

    _server.on( "c",        false, std::bind( &WiFiAp::OnRoot, this ) );
    _server.on( "SConnect", false, std::bind( &WiFiAp::OnConnect, this ) );
    _server.on( "Scan",     false, std::bind( &WiFiAp::OnScan, this ) );
    _server.begin(); 

    UpdateNextTime();

    // startup client
    if ( WiFi.SSID().length() )
    {
        Out( "WiFi: begin()\n" );
        WiFi.begin();
    }
}


WebServer & 
WiFiAp::Server()
{
    return _server;
}


void
WiFiAp::UpdateNextTime()
{
    if ( (WiFi.hostname() != g_options._ssid) && (_isOn) )
    {
        Out( "SoftAp: Disable\n" );
        WiFi.enableAP( false );
        _isOn = false;
    }
    
    _offTime = 0;
    if ( g_options._accessPointLifespan != 255 )
    {
        _offTime = g_options._accessPointLifespan * 60 + 1;
        Out( "SoftAp: off in %d seconds\n", _offTime - g_poweredOnTime );
    }
}


void
WiFiAp::Loop()
{
    const bool      isOn    = ( (!_offTime) || (g_poweredOnTime < _offTime)  );

    if ( isOn != _isOn )
    {
        _isOn = isOn;
        if ( isOn )
        {
            const ARGB      color = { ARGB::Flash | 3, 0xFF, 0xFF, 0xFF };
    
            WiFi.hostname( g_options._ssid );
            WiFi.softAP( g_options._ssid );
            WiFi.enableAP( true );
            _dnsServer.start( 53, "*", WiFi.softAPIP() );
            Out( "SoftAp: Start: %s, %s\n", g_options._ssid, WiFi.softAPIP().toString().c_str() );
            Popup1x6( 0, color );       // todo.. debounce this popup
        }
        else
        {
            // dns server stop?
            WiFi.enableAP( false );
            g_globalColor.PingState( GlobalColor::TimeError );
            Out( "SoftAp: Disable\n" );
        }
    }
   
    if ( (!g_wifiIsConnected) && (_reconnect) && (g_poweredOnTime > _reconnect) )
    {
        // retry at least every 15 minutes (autoreconnect seems to be slower)
        _reconnect = g_poweredOnTime + (15 * 60);
        WiFi.begin();
    }

    UpdateWiFiStatus();
    _dnsServer.processNextRequest();
    _server.handleClient();
}


void
WiFiAp::UpdateWiFiStatus()
{
    wl_status_t     status = WiFi.status();

    if ( _status != status )
    {
        ARGB        color   = { ARGB::Flash | 3, 0xFF, 0, 0 };
        char        popup   =   0;

        if ( !(_wlSurpress & (1<<status)) )
        {
            _wlSurpress |= (1 << status);
            Out( "WiFi: %s\n", WlStatus2Str(status) );
        }

        g_wifiIsConnected = false;
        switch ( status )
        {
        case WL_CONNECTED: 
            color.alpha = 5;        // no effect. display for 5 seconds
            color.green = 0xFF;
            color.blue  = 0xFF;
            if ( !WiFi.getAutoConnect() )
            {
                WiFi.setAutoConnect( true );
            }
            
            {
                IPAddress   ipAddr = WiFi.localIP();
                char        str[ 64 ];

                Out ( "WiFi: %s\n", ipAddr.toString().c_str() );
                sprintf( str, "%d.%d", ipAddr[ 2 ], ipAddr[ 3 ] );
                if ( strlen(str) > NUM_DIGITS )
                {
                    sprintf( str, "%d", ipAddr[ 3 ] );
                }
                Popup( str, color );
            }

            ScanDelete();
            g_wifiIsConnected = true;
            _wlSurpress = 0;
            _reconnect = 1;
            break;

        case WL_CONNECT_FAILED:     
            popup = 6;      
            break;

        case WL_CONNECTION_LOST:   
            popup = 2;
            break;

        case WL_DISCONNECTED:   
            if ( _status == WL_CONNECTED )
            {
                popup = 3;
            }
            break;
        }

        if ( popup )
        {
            Popup1x6( popup, color );
            if ( g_options._dirty )
            {
                g_options.Save();
            }
        }

        _status = status;
    }
}


void
WiFiAp::ConnectNow()
{ 
    Out( "WiFi: begin()\n" );
    _wlSurpress = 0;
    WiFi.begin();
}



char const *
WiFiAp::WlMode2Str( uint8 mode )
{
    switch ( mode )
    {
    case WIFI_OFF:      return "OFF";
    case WIFI_STA:      return "STA";
    case WIFI_AP:       return "AP";
    case WIFI_AP_STA:   return "AP+STA";
    }

    return nullptr;
}


char const * 
WiFiAp::WlStatus2Str( uint8 status )
{
    switch ( status )
    {
    case WL_IDLE_STATUS:        return "idle";
    case WL_NO_SSID_AVAIL:      return "No SSID";
    case WL_CONNECTED:          return "Connected";
    case WL_CONNECT_FAILED:     return "Connect Failed";
    case WL_CONNECTION_LOST:    return "Connection Lost";
    case WL_DISCONNECTED:       return "Disconnected";
    }

    return nullptr;
}


bool
WiFiAp::WiFiConnected()
{
    if ( g_wifiIsConnected )
    {
        _server.AddF( "Connected: %s<br/>", WiFi.SSID().c_str() );
        _server.AddF( "IP: %s<br/>", WiFi.localIP().toString().c_str() );
        return true;
    }

    return false;
}


void
WiFiAp::ScanStart()
{
    if ( !_isScanning )
    {
        ScanDelete();
        WiFi.scanNetworks( true );      // start scanning 
    }
}


void
WiFiAp::ScanDelete()
{
    _isScanning = false;
    WiFi.scanDelete();
}


void
WiFiAp::OnRoot()
{
    String          name = _server.arg( 'n' );

    _server.SetTitle( "", 2 );

    if ( !WiFiConnected() )
    {
        ScanStart();
        _server.Add( "Connect to WiFi<br>" );
        _server.AddForm( "Connect" );
        if ( name.length() )
        {
            _server.AddInput_br( 'n', 32, name.c_str() );
        }
        else
        {
            _server.AddInput2_br( 'n', 32, "SSID" );
        }
        _server.AddInputPassword_br( 'p', 32, "password" );
        _server.EndForm();
        _server.AddLinkButton( "Scan", "scan" );
    }
}


void
WiFiAp::OnConnect()
{
    _server.SetTitle( "Connect" );

    if ( !WiFiConnected() )
    {
        ARGB    color       = { ARGB::Flash | 3, 0xFF, 0xFF, 0xFF };
        String  name        = _server.arg( 'n' );
        String  password    = _server.arg( 'p' );

        Popup1x6( 1, color );

        _wlSurpress = 0;
        WiFi.begin( name.c_str(), password.c_str() );
        _server.AddF( "Attempting to connect to: %s<br/>Check Clock", name.c_str() );

        // sometimes the first connect doesn't work.. queue on a quick retry
        _reconnect = g_poweredOnTime + 10;
    }
}


void
WiFiAp::OnScan()
{
    auto addLink = 
        [this]( const String &ssid )
        {
            _server.AddF( "<a href='c?n=%s'>%s</a><br>", UrlEncode(ssid).c_str(), ssid.c_str() );
        };

    int         max;
    const int   start = millis();
    
    do
    {
        max = WiFi.scanComplete();
        if ( max >= 0 )
        {
            break;
        }
        delay( 1 );
    }
    while( (millis() - start) < 3*1000 );

    _server.SetTitle( "Scan" );
    if ( max < 0 )
    {
        _server.AddF( "still scanning" );
    } 
    else 
    {
        _server.AddF_br( "Found %d WiFis:", max );
        for ( int index=0; index < max; ++index )
        {
            String  ssid    = WiFi.SSID( index );
            bool    isDup   = false;

            for ( int j=0; j < index; ++j )
            {
                if ( ssid == WiFi.SSID(j) )
                {
                    isDup = true;
                    break;
                }
            }

            if ( !isDup )
            {
                addLink( ssid );
            }
        }
    }
}


    