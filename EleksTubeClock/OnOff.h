/*
 * OnOff.h
 *  Base calls for TimeOnOff and DimOnOff.  Has a schedule on on & off, with a user supplied
 *  override (called ForceOnOff) and support for gpio switches.
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */

class OnOff
{

public:
    void            DigitalRead( uint gpioForceOn, uint gpioForceOff );
    String          ForceOnOff( const String &arg );
    String          ForceOnOff( int seconds, bool surpressMsg );
    char const    * Desc( bool onOff ) const;
    bool            IsOn() const;

protected:
    bool            Loop();

private:
    void            _DigitalRead( uint gpio, int seconds );

public:
    // scheduled time
    uint8           _onDays;            // bit mask of which days to be on
    HourMinute      _onTime;            // start time to be on
    HourMinute      _offTime;           // ending time for being on

    // forced override
    bool            _forceOn;           // if _forceEnd != 0 then current state is being overridden to this
    uint32          _forceEnd;          // time when current forced state ends

    // gpio (time is in seconds)
    uint32          _gpioOnTime;
    uint32          _gpioOffTime;
};


