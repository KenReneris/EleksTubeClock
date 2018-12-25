/*
 * NtpClient.h
 *  Async ntp client.
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */


class NtpClient
{
public:
    enum State : uint8
    {
        WaitingForWifi,                     // waiting for wifi status to be online
        WaitingForSyncTime,                 // normal / idle
        WaitingForResponse,                 // sent request for time
        WaitingForRetry                     // request failed..  waiting to retry
    };

public:
    void            Setup();
    void            Loop();
    void            UpdateNextTime();       // Time or Options changed

    void            ForceSync();
    String          LastSync() const;
    State           GetState() const;
    char const    * GetStateStr() const;

private:
    void            SetState( State state );
    void            SendNtpPacket();
    void            CheckForResponse();
    
private:
    State           _state;
    bool            _lastSyncHadDiff;       // true if the last sync updated the time.  this forces the next sync to be quick (in 10m)
    uint32          _lastSync;              // last time synced
    uint32          _nextTime;              // the next time to start a sync
};

extern NtpClient    g_ntp;

