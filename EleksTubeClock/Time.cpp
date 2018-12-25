/*
 * Time
 *  Most date math is lef tup to https://github.com/PaulStoffregen/Time.
 *  This module replaces how time passage is tracked and can monitor the drift
 *  between the deviecs time and the report ntp time in order to reduce the 
 *  drift effects.
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */


#include "platform.h"

// positions between counts
static uint32       g_lastMs;               // last raw ms
static int32        g_lastPosPowerMs;       // 0..1000 position
static int32        g_lastPosGmtMs;         // 0..1000 position
static int32        g_lastPosMadj;          // 0..g_options._madjFreq position

uint32              g_poweredOnTime;        // elasped seconds since poweron (non-adjusted)
uint32              g_ms;                   // current ms (adjusted)
time_t              g_gmtTime;              // current gmt time
time_t              g_now;                  // local time

static uint64       g_madjPowerOnToNtp;     // offset from PoweredOnTimeAsMs() to ntp time as ms
static uint64       g_madjSyncStart;        // when drift period has started


uint64
PoweredOnTimeAsMs()
{
    return ( uint64(g_poweredOnTime) * 1000 + g_lastPosPowerMs);
}


uint64
GmtTimeAsMs()
{
    return ( uint64(g_gmtTime)*1000 + g_lastPosGmtMs);
}


String
LastMicroAdjust()
{
    if ( g_madjPowerOnToNtp )
    {
        const uint64    poweredOnTimeMs = PoweredOnTimeAsMs();
        const uint64    sinceStart = (poweredOnTimeMs - g_madjSyncStart);

        return  Duration( sinceStart / 1000 );
    }
    else
    {
        return "never";
    }
}


String
MadjSecondsPerDay( int32 rate )
{
    if ( rate )
    {
        float   msPerDay = float( 24 * 60 * 60 * 1000 ) / float( rate );
        float   secondsPerDay = msPerDay / 1000.0;

        return String( secondsPerDay, 3 );
    }
    else
    {
        return "0";
    }
}


// Replacement for TimeLib::now()
void
UpdateTime()
{
    // bump g_ms by elasped ms (and capture rawMs)
    const uint32    rawMs   = millis();
    int32           deltaMs = ( rawMs - g_lastMs );

    if ( deltaMs )
    {
        g_lastMs = rawMs;
    
        // powered on time
        g_lastPosPowerMs += deltaMs;
        while ( g_lastPosPowerMs >= 1000 )
        {
            g_poweredOnTime += 1;
            g_lastPosPowerMs -= 1000;
        }
    
        // adjust deltaMs.  should only loop 0 or 1 time.  so deltaMs should not go negative
        if ( g_options._madjFreq )
        {
            g_lastPosMadj += deltaMs;
            while ( g_lastPosMadj >= g_options._madjFreq )
            {
                deltaMs += g_options._madjDir;
                g_lastPosMadj -= g_options._madjFreq;
            }
        }
    
        // ms
        g_ms += deltaMs;
    
        // gmt time
        g_lastPosGmtMs += deltaMs;
        while ( g_lastPosGmtMs >= 1000 )
        {
            g_gmtTime += 1;
            g_lastPosGmtMs -= 1000;
        }
    }

    // add offset from gmt for current local time
    g_now = g_gmtTime + g_options._gmtOffset;
}


void
MicroAdjust( uint64 ntpTimeMs )
{
    const uint64    poweredOnTimeMs = PoweredOnTimeAsMs();

    // sync time is close
    if ( g_madjPowerOnToNtp )
    {
        const uint64    ourNtpTimeMs    = poweredOnTimeMs + g_madjPowerOnToNtp;
        const int64     ntpDiffMs       = ourNtpTimeMs - ntpTimeMs;
        const uint64    sinceStart      = ( poweredOnTimeMs - g_madjSyncStart );

      //Log( "Adj: %s, %s, %d\n", u64str(ntpTimeMs).c_str(), u64str(ourNtpTimeMs).c_str(), u64str(ntpDiffMs).c_str() );
        Log( "Adj: sync %s, Drift %d\n", u64str(sinceStart).c_str(), int32(ntpDiffMs) );
        if ( ABS(ntpDiffMs) < 1000 )
        {
            Log( "Adj: Not enough drift\n" );
            return;
        }

        if ( sinceStart < (9 * 60 * 60 * 1000) )
        {
            Log( "Adj: Not enough time\n" );
            return;
        }

        const float     msDriftPerMs    = float( -ntpDiffMs ) / float( sinceStart );
        const float     msDriftPerDay   = msDriftPerMs * float( 24 * 60 * 60 * 1000 );
        const int32     msDesiredRate   = float( 24 * 60 * 60 * 1000 ) / msDriftPerDay;
        const int32     oldRate         = ( g_options._madjFreq * g_options._madjDir );

        g_options._madjSmooth.Push( msDesiredRate );

        const int       msNewRate       = g_options._madjSmooth.Value();

      //Log( "Adj: Old Rate %d, New Rate %d\n", oldRate, msNewRate );
        Log( "Adj: Old Rate %s, New Rate %s\n", MadjSecondsPerDay(oldRate).c_str(), MadjSecondsPerDay(msNewRate).c_str() );

        if ( g_options._madjEnable )
        {
            if ( msNewRate != oldRate )
            {
                g_options._madjDir = ( msNewRate < 0 ? -1 : 1 );
                g_options._madjFreq = ABS( msNewRate );
                g_options._dirty = true;
            }
        }
        else
        {
            Log( "Adj: disabled\n" );
        }
    }
    else
    {
        Log( "Adj: FirstTime\n" );
    }

    // reset for next drift
    g_madjPowerOnToNtp = ( ntpTimeMs - poweredOnTimeMs );
    g_madjSyncStart = poweredOnTimeMs;
}


// Replacement for TimeLib::setTime()
bool
SetNtpTime( time_t ntpTime, uint32 ntpMs )
{
    UpdateTime();

    const uint64    ntpTimeMs   = ( uint64(ntpTime) * 1000 ) + ntpMs;
    const uint64    gmtMs       = GmtTimeAsMs();
    const int64     diffMs      = ( ntpTimeMs - gmtMs );
    bool            resync      = false;

    Log( "Time: %s\n", TimeStr().c_str() );

    // skip if time diff is less then 1/4 second
    if ( ABS(diffMs) > 250 ) 
    {
        // udpate gtmTime
        g_gmtTime       = ntpTime;
        g_lastPosGmtMs  = ntpMs;    
        g_now           = g_gmtTime + g_options._gmtOffset;

        Log( "Time: adjust %ss\n", String( (float(diffMs) / 1000.0), 3).c_str() );
        UpdateWaitTimes();

        // do quick resyncs until we get close
        resync = true;
    }
    else
    {
        // diff is small.. check for micro adjustment
        MicroAdjust( ntpTimeMs );        
    }

    return resync;
}

