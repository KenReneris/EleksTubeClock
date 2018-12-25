/*
 * Time.h
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */


void    UpdateTime();                           // called at the start of loop()
bool    SetNtpTime( time_t time, uint32 ms );   // replacement for TimeLib::SetTime()
String  LastMicroAdjust();
String  MadjSecondsPerDay( int32 rate );

// read-only globals (updated when UpdateTime() is called)
extern time_t       g_gmtTime;              // in seconds
extern time_t       g_now;                  // local time
extern uint32       g_poweredOnTime;        // seconds. monotonically increasing
extern uint32       g_ms;


