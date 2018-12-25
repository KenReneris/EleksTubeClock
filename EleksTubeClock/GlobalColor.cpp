/*
 * GlobalColor
 *  Tracks set/queued states and determines if the default (rainbow color) should be overriden.
 *  When a state is activated it triggers a corresponding effect to transistion to (and then
 *  from) that state.  If no state is activated, the current time is displayed.
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */

#include "platform.h"

extern ARGB   g_popupColor;

const ARGB    GlobalColor::k_black  = { (ARGB::BlendInOut | 0), 0x00, 0x00, 0x00 };
const ARGB    GlobalColor::k_error  = { (ARGB::BlendOut   | 5), 0xFF, 0x00, 0x00 };         


GlobalColor::GlobalColor()
    : _activeStates     ( 0 )
    , _displayStates    ( 0 )
    , _state            ( MaxState )
    , _changingState    ( StateChange::None )
    , _holdEnd          ( 0 )
{
}


ARGB const *
GlobalColor::ColorOptions( State state )
{
    ARGB const    * argb = nullptr;

    switch ( state )
    {
        case Popup:             argb = & g_popupColor;              break;
        case TimeOff:           argb = & k_black;                   break;
        case TopOfHour:         argb = & g_options._topOfHour;      break;
        case QuarterOfHour:     argb = & g_options._quarterOfHour;  break;
        case SyncingNtpTime:    argb = & g_options._ntpColor;       break;
        case SyncingTimeZone:   argb = & g_options._tzColor;        break;                              
        case ClientRequest:     argb = & g_options._httpClient;     break;
        case Date:              argb = & g_options._dateColor;      break;
        case TimeError:         argb = & k_error;                   break;
        default:
            Out( "GlobalColor: FIX NEEDED (state: %d)\n", state );
            break;
    }

    return argb;
}


bool 
GlobalColor::UseGlobalColor() const
{
    return ( _displayStates != 0 );
}


bool
GlobalColor::IsWifiClientActive() const
{
    return !!( _displayStates & (1 << ClientRequest) );
}


bool
GlobalColor::IsTurnedOff() const
{
    return (_state == TimeOff) && (_changingState == StateChange::Holding);
}



GlobalColor::State
GlobalColor::IsDisplaying() const
{
    static const uint32     k_stateForeground =
        (1 << uint(StateChange::Holding))      |
        (1 << uint(StateChange::FadingIn1))    |
        (1 << uint(StateChange::FadingOut2))   |
        (1 << uint(StateChange::BlendingIn))   |
        (1 << uint(StateChange::BlendingOut))  |
        (1 << uint(StateChange::FlashingOff))  |
        (1 << uint(StateChange::FlashingOn));

    if ( k_stateForeground & (1 << uint(_changingState)) )
    {
        return _state;
    }
    
    return MaxState;
}


char const * 
GlobalColor::IsDisplayingStr() const 
{
    switch( IsDisplaying() )
    {
    case Popup:                 return "Popup";
    case TimeOff:               return "TimeOff";
    case TimeError:             return "TimeError";
    case ClientRequest:         return "ClientRequest";
    case SyncingTimeZone:       return "SyncingTimeZone";
    case SyncingNtpTime:        return "SyncingNtpTime";
    case Date:                  return "Date";
    case TopOfHour:             return "TopOfHour";
    case QuarterOfHour:         return "QuarterOfHour";
    case MaxState:              return "Time";
    }

    return nullptr;
}


bool
GlobalColor::IsBlend() const
{
    bool        blend = false;

    switch ( _changingState )
    {
    case StateChange::BlendingIn:
    case StateChange::BlendingOut:
        blend = true;
        break;
    }

    return blend;
}


uint32
GlobalColor::IsPending( State state ) const
{
    return ( _displayStates & (1 << uint(state)) );
}


ARGB
GlobalColor::GetColor() const
{
    ARGB    argb = _stateColor;

    switch( _changingState )
    {
    case StateChange::FadingOut1:
    case StateChange::FadingIn2:
    case StateChange::FlashingOff:
        argb = { 0, 0, 0, 0 };
        argb.alpha = _stateColor.alpha;
        break;

    case StateChange::FadingIn1:
    case StateChange::FadingOut2:
        nscale8x3( argb.red, argb.green, argb.blue, argb.alpha );
        argb.alpha = 0xFF;
        break;
    }

    return argb;
}


void
GlobalColor::PingState( State state )
{
    EnableState( state );
    ClearState( state );
}


void
GlobalColor::EnableState( State state, bool enabled )
{
    if ( enabled )
    {
        EnableState( state );
    }
    else
    {
        ClearState( state );
    }
}

void  
GlobalColor::EnableState( State state )
{
    uint      bit = ( 1 << state );

    // if not in pending display states, set it
    if ( !(_displayStates & bit) )
    {
        uint    alpha = ColorOptions( state )->alpha;

        // skip if there's no hold or fade values
        if ( alpha )
        {
            _activeStates |= bit;
            _displayStates |= bit;
        }
    }
}


void 
GlobalColor::ClearState( State state )
{
    if ( _activeStates & (1 << state) )
    {
        _activeStates &= ~(1 << state);
        if ( (_state == state) && (_changingState == StateChange::Holding) )
        {
            _holdEnd = g_ms + (_stateFlags & ARGB::HoldTimeMask) * 1000;
        }
    }
}


// only called if UseGlobalColor() is true
void
GlobalColor::NextFrame()
{
    const StateChange   oldChangingState    = _changingState;
    const State         newState            = State( FirstSetBit16(_displayStates) );
    const uint          curMs               = millis();

    if ( newState != _state )
    {
        ARGB            stateArgb       = *ColorOptions( newState );
        bool            changeStates    = true;

        // Check if we should change states now
        if ( _state != MaxState )
        {
            // if changing, pitch old state
            if ( changeStates )
            {
                _displayStates &= ~(1 << _state);
            }
        }
        
        if ( changeStates )
        {
            _state = newState;
            _stateFlags = stateArgb.alpha;
            _stateColor = stateArgb;

            _holdEnd = 0;
            _changingState = StateChange::Holding;

            switch ( _stateFlags & ARGB::EffectMask )
            { 
            case ARGB::CrossFade:
                _changingState = StateChange::FadingOut1;
                _stateColor.alpha = 0;
                break;

            case ARGB::BlendInOut:
                _changingState = StateChange::BlendingIn;
                _stateColor.alpha = 0;
                break;

            case ARGB::Flash:
                _changingState = StateChange::FlashingOff;
                _holdEnd = curMs + 50;
                _flashCount = 0;
                break;
            }
        }
    }

    switch( _changingState )
    {
    case StateChange::Holding:
        _stateColor.alpha = 0xFF;
        if ( _activeStates & (1 << _state) )
        {
            // reset the end time
            _holdEnd = 0;
        }
        else
        {
            // if end time not set, do it now
            if ( !_holdEnd )
            {
                _holdEnd = curMs + (_stateFlags & ARGB::HoldTimeMask) * 1000;
            }
            else if ( curMs >= _holdEnd )
            {
                _changingState = StateChange::None;
                switch( _stateFlags & ARGB::EffectMask )
                {
                case ARGB::CrossFade:
                    _changingState = StateChange::FadingOut2;
                    break;

                case ARGB::BlendOut:
                case ARGB::BlendInOut:
                    _changingState = StateChange::BlendingOut;
                    break;

                case ARGB::Flash:
                    break;
                }
            }
        }
        break;
        
    case StateChange::FlashingOff:
        _stateColor.alpha = 0xFF;
        if ( curMs >= _holdEnd )
        {
            _changingState = StateChange::FlashingOn;
            _holdEnd = curMs + 100;

            if ( _flashCount > (_stateFlags & ARGB::HoldTimeMask) )
            {
                _changingState = StateChange::None;
            }
        }
        break;

    case StateChange::FlashingOn:
        if (curMs >= _holdEnd)
        {
            _changingState = StateChange::FlashingOff;
            _holdEnd = curMs + 50;
            _flashCount += 1;
        }
        break;

    case StateChange::FadingOut1:
        _stateColor.alpha += 4;
        if ( _stateColor.alpha > (255-4) )
        {
            _stateColor.alpha = 1; 
            _changingState = StateChange::FadingIn1;
        }
        break;

    case StateChange::FadingIn1:
        _stateColor.alpha += 4;
        if ( _stateColor.alpha > (255-4) )
        {
            _changingState = StateChange::Holding;
        }
        break;

    case StateChange::FadingOut2:
        _stateColor.alpha -= 4;
        if ( _stateColor.alpha < 4 )
        {
            _stateColor.alpha = 255;
            _changingState = StateChange::FadingIn2;
        }
        break;

    case StateChange::FadingIn2:
        _stateColor.alpha -= 4;
        if ( _stateColor.alpha < 4 )
        {
            _changingState = StateChange::None;
        }
        break;

    case StateChange::BlendingIn:
        _stateColor.alpha += 5;
        if ( _stateColor.alpha > (255-5) )
        {
            _changingState = StateChange::Holding;
        }
        break;

    case StateChange::BlendingOut:
        _stateColor.alpha -= 5;
        if ( !_stateColor.alpha )
        {
            _changingState = StateChange::None;
        }
        break;
    }
    
    if ( _changingState == StateChange::None )
    {
        _displayStates &= ~(1 << _state);
        _state = MaxState;
    }
}

