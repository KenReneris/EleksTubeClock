/*
 * TimeZone.h
 *   If automatic timezone (and DST) beign used, this class performs the IOs and makes the timezone updates.
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */


class TimeZone
{
public:
    enum State
    {
        Disabled,               // No key for api.timezonedb.com
        WaitingForWifi,
        WaitingForSyncTime,     // normal / idle
        SyncingTimeZone,
        WaitingForRetry         // request failed..  waiting to retry
    };

public:
    void            Setup();
    void            Loop();
    void            UpdateNextTime();

    void            UserSyncNow();
    State           GetState() const;
    char const    * GetStateStr() const;

    // used by GetJsonField
    void            JsonParseFailure( char const * fieldName );

private:
    void            SetState( State state );
    void            UpdateTimeZone();

private:
    State           _state;
    uint32          _nextTime;      // next time to fetch a timezone update

    bool            _isValid;
    bool            _isDst;
    int32           _dstStart;
    int32           _dstEnd;
};

extern TimeZone     g_timeZone;

