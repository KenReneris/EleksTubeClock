/*
 * GlobalColor.h
 *  Tracks set/queued states and determines if the default (rainbow color) should be overriden.
 *  When a state is activated it triggers a corresponding effect to transistion to (and then
 *  from) that state.  If no state is activated, the current time is displayed.
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */


class GlobalColor
{
public:
    // stats are in order of priority.  e.g., Date will display before TopOfHour effect
    enum State : uint8
    {
        Popup               = 0,        // on top of everything.  note this will display even if the time is off.
        TimeNotSet          = 1,
        TimeOff             = 2,        // if in this state, displaying the time (& date) is surpressed.
        TimeError           = 3,        // some sort of time sync error.  ntp or timezone.
        ClientRequest       = 4,        // webserver responsing to something.
        SyncingTimeZone     = 5,        
        SyncingNtpTime      = 6,
        Date                = 7,
        TopOfHour           = 8,
        QuarterOfHour       = 9,
        MaxState
    };

public:
    GlobalColor();

    bool            UseGlobalColor() const;         // there's a global color/effect occuring 
    ARGB            GetColor() const;               // the current global color to use.  alpha is for color-blending to the current (time) color
    void            NextFrame();                    // only called if UseGlobalColor() is true

    void            EnableState( State state, bool enabled );   // calls either Enable or Clear depending on "enabled"
    void            EnableState( State state );
    void            ClearState( State state );
    void            PingState( State state );       // calls Enable followed by Clear

    State           IsDisplaying() const;           // which state is active
    char const    * IsDisplayingStr() const;
    bool            IsBlend() const;                // If the values are blending.  
    uint32          IsPending( State state ) const; // non-zero if the "state" is set.  Set but might not be displaying if a higher priority state is also set.
    bool            IsWifiClientActive() const;     // if the ClientRequest is set (used to poll the web requests for frequently)
    bool            IsTurnedOff() const;            // if the TimeOff is set.

private:
    enum class StateChange : uint8
    {
        None,

        // Cross fade sequence
        FadingOut1,         // foreground to black
        FadingIn1,          // black to global color
        Holding,            // global color
        FadingOut2,         // global color to black
        FadingIn2,          // black to foreground

        BlendingIn,         // foreground to global color
        BlendingOut,        // global color to foreground

        FlashingOff,
        FlashingOn,
    };

private:
    static const ARGB   k_black;
    static const ARGB   k_notSet;
    static const ARGB   k_error;

private:
    static ARGB const * ColorOptions( State state );

private:
    uint32              _activeStates;          // states being held on
    uint32              _displayStates;         // states wanting to be displayed. only the highest priroity is displayed

    State               _state;                 // current state
    uint8               _stateFlags;            // current flags from the states effect.  Form: g_options.{_effect}.alpha
    ARGB                _stateColor;            // current color from the states effect.  Alpha is not the alpha for the effect to use.  Might be ramping on/off, etc..

    StateChange         _changingState;         // current step of the effect being applied (for entering/leaving _state, ..)
    uint8               _flashCount;            // if the effect is to flash.. the count of flashes
    uint32              _holdEnd;               // if the effect has a "hold on" time in it's squence, this is when the hold end and the next step of the effect can start
};


extern GlobalColor      g_globalColor;

