/*
 * WiFiAp.h
 *  WiFi Access Point
 *  (also hosts the WebServer object)
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */


class WiFiAp
{
public:
    static char const * WlMode2Str( uint8 status );
    static char const * WlStatus2Str( uint8 status );

public:
    void            Setup();
    void            Loop();
    void            UpdateNextTime();
    void            ConnectNow();

    WebServer     & Server();

private:
    void            ScanStart();
    void            ScanDelete();

    void            UpdateWiFiStatus();
    bool            WiFiConnected();
    void            OnRoot();
    void            OnConnect();
    void            OnScan();

private:
    DNSServer       _dnsServer;         // esp8266 dns server
    WebServer       _server;            // our webserver and features (subclass of esp8266 web server)
    bool            _isScanning;        // if async scan has been kicked off
    wl_status_t     _status;            // last WiFi.status()
    bool            _isOn;              // if the Access Point is on or off
    uint8           _wlSurpress;        // surpress duplicate console output of status changes
    uint32          _offTime;           // schedule time when to turn the AP off
    uint32          _reconnect;         // testing this.  time to call WiFi.begin().  the normal automated approach gets to be really long/slow
};

extern WiFiAp       g_wifiAp;
