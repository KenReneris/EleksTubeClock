/*
 * Options.h
 *  User options saved to EEPROM.
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */


struct Options
{
    uint32              _checksum;
    uint8               _version;               // version of this structure
    bool                _dirty;                 // for slow updates mark dirty.  only saved when wifi is lost

    char                _prefixName[ 10 ];      // webserver decroates page names with this
    char                _ssid[ 32 ];            // the AP name.  defaults to "EleksTube"
    char                _password[ 16 ];        // password to edit settings via the web interface
    bool                _splashScreen;          // if the splash screen should be shown on bootup or not
    bool                _surpressLeadingZero;   // if the first digit of the time/date should be " " instead of "0"

    TimeOnOff           _timeOnOff;             // schedule for when the time (& date) should be shown
    DimOnOff            _dimOnOff;              // schedule for when dimming the display should be in effect
    bool                _12Hour;                // time is shown in 12 or 24 hour format
    uint8               _bright;                // the users setting of !Dim
    uint8               _dim;                   // the users setting of Dim

    ARGB                _topOfHour;             // Top of hour effect (if any)
    ARGB                _quarterOfHour;         // Quarter of hour effect (in any)

    char                _ntpServer[ 32 ];       // ntp server to use. defauts to "time.google.com"
    ARGB                _ntpColor;              // effect to use when syncing the ntp time (if any)
    HourMinute          _ntpSync;               // hh:mm to align syncing too
    uint8               _ntpFrequency;          // ntp polling rate in 10 minute increments (aligned to _ntpSync)

    bool                _madjEnable;            // if computing micro adjustments of lcoal time-drift are enabled. 
    int8                _madjDir;               // micro adjust +1 or -1 a millisecond 
    uint32              _madjFreq;              // micro adjust _madjDir every n elasped ms's
    Smooth<4>           _madjSmooth;            // smooth out the adjustment rate with a little history

    ARGB                _dateColor;             // effect when showing the date (if any).  If no effect, showing the date is disabled.
    uint8               _dateMinutes;           // show the date every this many minutes. E.g., 1, 5, 15, ...
    uint8               _dateSecond;            // At what second mark to show the date
    bool                _dateMmddyy;            // use mmddyy or yymmdd format

    char                _tzKey[ 16 ];           // lead byte is 0 if disabled.  the users timezonedb.com key
    ARGB                _tzColor;               // effect when syncing timezone (if any).  Note the timezone sync will cause lag/glitches
    int32               _gmtOffset;             // delta from gmtTime.  "g_now = g_gmtTime + _gmtOffset"

    ARGB                _httpClient;            // effect when responding to an http request.  note serving web requests will likely cause lag/glitches.
    uint8               _accessPointLifespan;   // minutes http AP server is active after coming online. 0=off, 255=no-timeout.  Note the STA server always stays on line.
    bool                _allowPopupUrl;         // if /popup url request should be honored (no password required)
    bool                _allowBrightnessUrl;    // if /forcedim or /forceon url requests should be honored (no password required)

public:
    void                Setup();
    void                Load();
    void                Save();
    void                Reset();

private:
    uint32              Checksum();
};


extern Options          g_options;

