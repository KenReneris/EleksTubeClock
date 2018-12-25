/*
 * WebServer
 *  Local WebServer for EleksTubeClock
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */

#include "platform.h"
#include "WebServer.h"

#include "NtpClient.h"
#include "TimeZone.h"
#include "Console.h"


const char    WebServer::k_daysOfWeek3[] = "Sun\0Mon\0Tue\0Wed\0Thu\0Fri\0Sat";


WebServer::WebServer()
    : _responses            ( 0 )
    , _snappedOptions       ( nullptr )
    , _buffer               ( [this](char const *txt){ _content += txt; } )
{
}


uint32
WebServer::ResponsesSent() const
{
    return _responses;
}


void
WebServer::on( char const *uri, THandlerFunction handler )
{
    on( uri, true, handler );
}


void
WebServer::on( char const *uri, bool sta, THandlerFunction handler )
{
    WiFi.scanNetworksAsync( nullptr );

    ESP8266WebServer::on( 
        PrintF( "/%s", uri ),
        [this, sta, handler]()
        {           
            const bool    isLocal = ( this->client().localIP() == WiFi.localIP() );

            Out( "WebServer: '%s'\n", this->uri().c_str() );
            ResetPageState();

            if ( sta != isLocal )
            {
                OnNotFound();
            }
           
            handler();
            SendPage();
        } );
}


void
WebServer::begin()
{
    // todo - could save some ram here 

    on( "",             std::bind( &WebServer::OnRoot, this ) );
    on( "r2",           std::bind( &WebServer::OnRoot2, this ) );
    on( "p",            std::bind( &WebServer::OnGetPassword, this ) );
    on( "Sp",           std::bind( &WebServer::OnSetPassword, this ) );

    // forms
    on( "Settings",     std::bind( &WebServer::OnSettings, this ) );
    on( "SSettings",    std::bind( &WebServer::OnSetSettings, this ) );
    on( "TOnOff",       std::bind( &WebServer::OnTimeOnOff, this ) );
    on( "STOnOff",      std::bind( &WebServer::OnSetTimeOnOff, this ) );
    on( "DOnOff",       std::bind( &WebServer::OnDimOnOff, this ) );
    on( "SDOnOff",      std::bind( &WebServer::OnSetDimOnOff, this ) );

    on( "TopHr",        std::bind( &WebServer::OnTopOfHour, this ) );
    on( "STopHr",       std::bind( &WebServer::OnSetTopOfHour, this ) );
    on( "QHr",          std::bind( &WebServer::OnQuarterOfHour, this ) );
    on( "SQHr",         std::bind( &WebServer::OnSetQuarterOfHour, this ) );
    on( "Date",         std::bind( &WebServer::OnShowDate, this ) );
    on( "SDate",        std::bind( &WebServer::OnSetShowDate, this ) );

    on( "NTP",          std::bind( &WebServer::OnNtp, this ) );
    on( "SNTP",         std::bind( &WebServer::OnSetNtp, this ) );
    on( "TimeZone",     std::bind( &WebServer::OnTimeZone, this ) );
    on( "STimeZone",    std::bind( &WebServer::OnSetTimeZone, this ) );
    on( "MicroAdjust",  std::bind( &WebServer::OnMicroAdjust, this ) );
    on( "SMicroAdjust", std::bind( &WebServer::OnSetMicroAdjust, this ) );
    on( "WebReq",       std::bind( &WebServer::OnWebReq, this ) );
    on( "SWebReq",      std::bind( &WebServer::OnSetWebReq, this ) );
    
    // buttons
    on( "BNtp",         std::bind( &WebServer::OnSyncNtp, this ) );
    on( "TestPopup",    std::bind( &WebServer::OnCreatePopup, this ) );
    on( "SPopup",       std::bind( &WebServer::OnSetPopup, this ) );
    
    on( "cp.css",       std::bind( &WebServer::OnColorPickerMinCss, this ) );
    on( "cp.js",        std::bind( &WebServer::OnColorPickerMinJs, this ) );
    
    // actions
    on( "popup",        std::bind( &WebServer::OnPopup, this ) );
    on( "forcedim",     std::bind( &WebServer::OnForceDim, this ) );
    on( "forceon",      std::bind( &WebServer::OnForceOn, this ) );
    on( "time",         std::bind( &WebServer::OnTime, this ) );

    // misc
    on( "stats",        std::bind( &WebServer::OnStats, this ) );
    on( "about",        std::bind( &WebServer::OnAbout, this ) );
    
    ESP8266WebServer::onNotFound ( std::bind( &WebServer::OnNotFound, this ) );

    static const char * headerkeys[] = { "Cookie" };
    ESP8266WebServer::collectHeaders( headerkeys, countof(headerkeys) );
    ESP8266WebServer::begin();
}


String
WebServer::arg( char name )
{
    char        str[ 2 ];

    str[ 0 ] = name;
    str[ 1 ] = 0;
    return ESP8266WebServer::arg( str );
}


bool
WebServer::IsIp( String str )
{
    for ( char c : str )
    {
        if (c != '.' && (c < '0' || c > '9'))
        {
            return false;
        }
    }

    return true;
}


void
WebServer::Redirect( String const &uri, bool sendSessionId )
{
    Redirect( uri.c_str(), sendSessionId );
}


void
WebServer::Redirect( char const *uri )
{
    Redirect( uri, false );
}


void
WebServer::Redirect( char const *uri, bool sendSessionId )
{
    if ( !_responseSent )
    {
        String  fullUri = PrintF( "http://%s%s", client().localIP().toString().c_str(), uri );

        Out( "WebServer: Redirect '%s' -> '%s'\n", hostHeader().c_str(), fullUri.c_str() );
        sendHeader( "Location", fullUri, true );
        if ( sendSessionId )
        {
            sendHeader( "Set-Cookie", PrintF("ESPID=%d", _clientId) );
        }

        Send( 302, "text/plain", "" );
        client().stop();
     }
}


void
WebServer::OnNotFound()
{
    ResetPageState();
    if ( !IsIp(hostHeader()) )
    {
        Redirect( "/c" );
    }
    else
    {
        _content = PrintF( "File Not Found\n\nURI: %s", uri().c_str() );  

        SendHeader( "Cache-Control", "no-cache, no-store, must-revalidate" );
        SendHeader( "Pragma", "no-cache" );
        SendHeader( "Expires", "-1" );
        SendHeader( "Content-Length", String( _content.length()) );
        Send ( 404, "text/plain", _content );
    }

    g_globalColor.PingState( GlobalColor::ClientRequest );
}


void
WebServer::ResetPageState()
{
    _responseSent   = false;
    _content        = "";
    _titleScale     = 0;
    _formStarted    = false;
    _divStarted     = false;
    _isText         = false;
    _pingColorState = true;
    _buffer.Reset();
}


void
WebServer::SetTitle( char const *title )
{
    SetTitle( title, 1 );
}


void
WebServer::SetTitle( char const *title, uint8 scale )
{
    SetTitle( title, title, scale );
}


void
WebServer::SetTitle( char const *title, char const *pageName, uint8 scale )
{
    _titleScale = scale;

    AddF(
        "<!DOCTYPE html><html lang=\"en\">"
        "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=%d,\"/>"
        "<title>%s%s%s</title></head>"
        "<body onload='LX();LX2();'>"
            "<script>function LX(){}function LX2(){}</script>\n"
            "<h2>"
                "<font color='#2E9AFE'>%s%s%s%s</font>"
            "</h2>\n",
        scale,
        g_options._ssid, (title[0] ? ": " : ""), title,
        g_options._prefixName, g_options._ssid, (pageName[0] ? ": " : ""), pageName
    );
}


void
WebServer::AddBr()
{
    Add( "<br/>" );
}


void
WebServer::Add( String const &html )
{
    _buffer.Add( html.c_str() );
}


void 
WebServer::Add( char const *html )
{
    _buffer.Add( html );
}


void
WebServer::AddF( char const *format, ... )
{
    va_list     args;
    
    va_start ( args, format );
    _buffer.VAddF( format, args );
    va_end( args );
}


void
WebServer::AddF_br( char const *format, ... )
{
    va_list     args;

    va_start ( args, format );
    _buffer.VAddF( format, args );
    va_end( args );
    AddBr();
}


void
WebServer::AddClientForm( char const *action )
{
    CheckClient();
    AddForm( action );
}


void
WebServer::AddForm( char const *action )
{
    if ( !_titleScale )
    {
        SetTitle( action );
    }

    AddF( "<form method='get' action='S%s'><p style='line-height:%d%%'>", action, (_titleScale > 1 ? 100 : 200) );
    _formStarted = true;
}


void
WebServer::AddInput2_br( char id, int8 length, char const *placeholder )
{
    AddF_br( "<input id='%c' name='%c' length=%d placeholder='%s'/>", id, id, length, placeholder );
}


void
WebServer::AddInputPassword_br( char id, int8 length, char const *placeholder )
{
    AddF_br( "<input type='password' id='%c' name='%c' length=%d placeholder='%s'/>", id, id, length, placeholder );
}


void
WebServer::AddInput_br( char id, int8 length, char const *value, char const *desc )
{
    AddF( desc );
    AddInput_br( id, length, value );
}


void
WebServer::AddInput_br( char id, int8 length, char const *value )
{
    AddInput( id, length, value );
    AddBr();
}


void
WebServer::AddInput( char id, int8 length, char const *value )
{
    AddF( "<input id='%c' name='%c' length=%d value='%s'/>", id, id, length, value );
}


void 
WebServer::AddRadio( char id, char const *value, char const *desc, bool selected )
{
    AddF( "<input name='%c' type='radio' value='%s' %s/>%s", id, value, (selected ? "checked" : ""), desc );
}


void
WebServer::AddCheckbox( char id, char const *desc, bool selected )
{
    AddF( "<input name='%c' type='checkbox' %s/>%s", id, (selected ? "checked" : ""), desc );
}


void
WebServer::AddCheckbox_br( char id, char const *desc, bool selected )
{
    AddCheckbox( id, desc, selected );
    AddBr();
}


void
WebServer::AddOption( uint value )
{
    char        buffer[ 32 ];

    sprintf( buffer, "%d", value );
    AddOption( value, buffer );
}


void
WebServer::AddOption( uint value, char const *name )
{
    AddOption( value, (value == _selectionValue), name );
}


void
WebServer::AddOption( uint value, bool selected, char const *name )
{
    AddF( "<option value='%d' %s>%s</option>", value, (selected ? "selected" : ""), name );
}


void
WebServer::AddTime( char id, char const *desc, HourMinute hhmm )
{
    AddF( "%s<input name='%c' type='time' value='%02d:%02d'/>", desc, id, hhmm._hour, hhmm._minute );
}


void
WebServer::AddDateTime( char const *desc, time_t value )
{
    tmElements_t            tm;

    breakTime( value, tm );
    AddF( "<font id='f'>%s<input name='d' type='date' value='%04d-%02d-%02d'/>", desc, tm.Year+1970, tm.Month, tm.Day );
    AddF( "Time: <input name='t' type='time' value='%02d:%02d'/></font>", tm.Hour, tm.Minute );
}


void
WebServer::AddLinkButton( char const *uri )
{
    AddLinkButton( uri, uri );
}


void
WebServer::AddLinkButton( char const *uri, char const *desc )
{
    AddF( "<button onclick='window.location.href=\"/%s\"'>%s</button> ", uri, desc );
}


void
WebServer::AddLinkDiv( char const *text )
{
    AddLinkDiv( text, text );
}


void
WebServer::AddLinkDiv( char const *uri, char const *text )
{
    EndDiv();
    AddF( "<a href='/%s'>%s:</a><div style='margin-left: 20px'>", uri, text );
    _divStarted = true;
}


void
WebServer::EndDiv()
{
    if ( _divStarted )
    {
        AddF_br( "</div>\n" );
        _divStarted = false;
    }
}


void
WebServer::EndForm()
{
    if ( _formStarted )
    {
        Add( "<br/><button type='submit'>submit</button></form>" );
        _formStarted = false;
    }
}
    

void
WebServer::AddOnOff( const OnOff & onOff )
{
    if ( onOff._onDays )
    {
        if ( onOff._onDays == 0x7F )
        {
            Add( "Everyday " );
        }
        else
        {
            char const  * sep = "";

            for ( int dayOfWeek=0; dayOfWeek < 7; ++dayOfWeek )
            {
                if ( onOff._onDays & (1 << dayOfWeek) )
                {
                    AddF( "%s%s", sep, &k_daysOfWeek3[dayOfWeek*4] );
                    sep = ", ";
                }
            }
        }

        if ( onOff._onTime != onOff._offTime )
        {
            AddF( " between %s - %s", onOff._onTime.toString().c_str(), onOff._offTime.toString().c_str() );
        }
    }
    else
    {
        Add( "Disabled" );
    }
    AddBr();
    AddF_br( "Currently is %s", onOff.Desc( onOff.IsOn() ) );
}


void
WebServer::AddOnOffEdit( char os, char const *desc, const OnOff &onOff )
{
    AddTime( os+0, desc, g_options._timeOnOff._onTime );
    AddTime( os+1, " and ", g_options._timeOnOff._offTime );
    AddBr();
    Add( "On: " );
    for (int index = 0; index < 7; ++index)
    {
        AddCheckbox( os+2+index, &k_daysOfWeek3[index*4], !!(onOff._onDays & (1 << index)) );
    }
    AddBr();
}


void
WebServer::ParseOnOff( const char os, OnOff *onOff )
{
    onOff->_onTime   = HourMinute( arg(os+0) );
    onOff->_offTime  = HourMinute( arg(os+1) );

    onOff->_onDays = 0;
    for ( int index = 0; index < 7; ++index )
    {
        onOff->_onDays |= arg( os+2+index ).length() ? (1 << index) : 0;
    }
}


String
WebServer::ColoredText( ARGB rgb, char const *text )
{
    const String    color = rgb.toRgbString();

    if ( !text )
    {
        text = color.c_str();
    }

    return PrintF( "<font id='az' style='background-color: #%s; margin: 1px; border: 1px solid black;'>%s</font>", color.c_str(), text );
}


void
WebServer::AddColorEffect( ARGB argb )
{
    const uint      effectValue     = ( argb.alpha & ARGB::EffectMask );
    const uint      count           = ( argb.alpha & ARGB::HoldTimeMask );
    String          colorHtml;
    char          * effect;

    // could use an array here...
    switch ( effectValue )
    {
    case 0:                     effect = "Appear %s";               break;
    case ARGB::CrossFade:       effect = "Cross Fade %s";           break;
    case ARGB::BlendInOut:      effect = "Blend In %s Blend Out";   break;
    case ARGB::BlendOut:        effect = "Appear %s Blend Out";     break;
    case ARGB::Flash:           effect = "Flash %s";                break;
    default:                    effect = "? %s";                    break;
    }

    if ( !argb.alpha )
    {
        effect = "Effect Disabled";
    }

    colorHtml = ColoredText( argb, nullptr );
    AddF( effect, colorHtml.c_str() );

    if ( effectValue == ARGB::Flash )
    { 
        AddF_br( " %d times", count );
    }
    else
    {
        AddF_br( ", hold for %d second%s", count, (count == 1 ? "" : "s") );
    }
}


void
WebServer::AddColorEffectEdit( ARGB argb )
{
    static char const * names[]     = { "Appear", "Cross Fade", "Blend In & Out", "Appear & Blend Out", "Flash", "Disabled" };
    const uint          effect      = (argb.alpha & ARGB::EffectMask) >> 4;     // now matches the table above
 
    // D() - return document element by id
    // S() - return innerHTML document element
    // LX() - onload
    Add( 
        "<link href='cp.css' rel='stylesheet'>"
        "<style>.color-box{display:inline-block; width:20px; height:20px; cursor:pointer;}</style>"
        "<script>"
        "function D(i){return document.getElementById(i);}"
        "function S(i,v){return D(i).innerHTML=v;}"
        "function V(i,v){D(i).style.visibility=v;}"
        "function LX(){"
            "h=D('e');"
            "if(h.value==4){a='flash';b='times';}else{a='hold for';b='seconds';}"
            "S('aa',a);S('ab',b);"
            "v=(h==5?'hidden':'visible');"
          //"V('aa',v);V('ab',v);V('z',v);"
            "}"
        "</script>"
    );

    // e - effect value
    // h - hold value
    // aa, ab, az - misc text
    // c - color
    Add( "Effect Type:<select id='e' name='e' onchange='LX()'>" );
    _selectionValue = ( argb.alpha ? effect : 5 );
    for( uint index=0; index < countof(names); ++index )
    {
        AddOption( index, names[ index ] );
    }
    Add( "</select> <font id='aa'>x</font> " );
    AddF( "<input id='h' name='h' type='number' min='0' max='15' size='2' value='%d'/> ", (argb.alpha & ARGB::HoldTimeMask) );
    AddF( "<font id='ab'>x</font><br>Color: " );
    AddInput( 'c', 6, argb.toHtmlRgbString().c_str() );
    Add( ColoredText( argb, " " ).c_str() );

    Add( "<script src='cp.js'></script>" );
    Add(
        "<script>"
            "input=D('c');"
            "pr=new CP(input);"
            "pr.on('change',function(c){S('az',c);c='#'+c;this.source.value=c;D('az').style.backgroundColor=c;});"
            "function up(){pr.set(this.value).enter();}"
            "ps=pr.source;ps.oncut=ps.onpaste=ps.onkeyup=ps.oninput=up;"
        "</script>"
    );
}


ARGB
WebServer::GetColorEffect()
{
    const uint8     effectValue     = arg( 'e' ).toInt() & 0xF;
    const uint8     hold            = arg( 'h' ).toInt() & 0xF;
    ARGB            argb            = ARGB::FromString( arg('c') );

    argb.alpha = ( effectValue << 4 ) | hold;
    if ( effectValue == 5 )
    {
        // disabled == appear for 0 time 
        argb.alpha = 0;
    }
    return argb;
}


void
WebServer::SendPage()
{
    if ( _snappedOptions )
    { 
        if ( memcmp(_snappedOptions, &g_options, sizeof(g_options)) )
        { 
            g_options.Save();
        }

        delete _snappedOptions;
        _snappedOptions = nullptr;

        Redirect( _snapRedir );
    }
    else if ( !_isText )
    {
        EndDiv();
        EndForm();
        
        if ( _titleScale )
        {
            Add( "</body></html>" );
        }

        _buffer.Flush();
        SendHeader( "Content-Length", String( _content.length() ) );
        Send( 200, "text/html", _content );
    }
    else
    {
        _buffer.Flush();
        SendHeader( "Content-Length", String( _content.length() ) );
        Send( 200, "text/plain", _content );
    }

    if ( _pingColorState )
    {
        g_globalColor.PingState( GlobalColor::ClientRequest );
    }
}


void 
WebServer::SendHeader( char const *name, const String &value )
{
    if ( !_responseSent )
    {
        ESP8266WebServer::sendHeader( name, value );
    }
}


void
WebServer::Send( int code, char const * contentType, const String& content )
{
    Send( code, contentType, content.c_str() );
}


void
WebServer::SendHeader( char const *name, const char *value )
{
    SendHeader( name, String(value) );
}


void 
WebServer::Send( int code, char const * contentType, char const *content )
{
    if ( !_responseSent )
    {
        ESP8266WebServer::send_P( code, contentType, content );
        _responseSent = true;
        _responses += 1;
    }
}


void
WebServer::SnapOptions( const char *redir )
{
    CheckClient();
    _snapRedir = redir;
    if ( !_snappedOptions )
    {
        _snappedOptions = new Options();
    }

    memcpy( _snappedOptions, &g_options, sizeof(g_options) ); 
}


void
WebServer::CheckClient()
{
    if ( g_options._password[0] )
    {
        bool    redirect    = true;

        if ( _clientId )
        {
            String      cookie      = ESP8266WebServer::header( "Cookie" );
            int const   pos         = cookie.indexOf( "ESPID=" );

            if ( pos >= 0 )           
            {
                uint const  id      = atoi( cookie.c_str() + 6 );

                if ( id == _clientId )
                {
                    redirect = false;
                }
            }
        }

        if ( redirect )
        {
            Redirect( PrintF("/p?u=%s", UrlEncode(uri()).c_str()), false );
        }
    }
}


void
WebServer::OnGetPassword()
{
    _clientId = 0;

    SetTitle( "Login" );
    AddForm( "p" );
    AddInput_br( 'p', 8, "", "password: " );
    AddF( "<input name='u' value='%s' hidden/>", arg('u').c_str() );
}


void
WebServer::OnSetPassword()
{
    String      p = arg( 'p' );

    if ( p == g_options._password )
    {
        randomSeed( g_ms ^ random(0x7FFFFFFF) );
        _clientId = random( 0x7FFFFFFF );
        Redirect( arg('u'), true );
    }
    else
    {
        Redirect( "" );
    }
}


///////////////////////////////////////
//
//
//


void
WebServer::OnRoot()
{
    // time in color
    {
        String      digits( "<font face='Courier New' color='black'><b>" );
        char        digitStr[ 2 ];

        digitStr[ 1 ] = 0;
        for ( int index=0; index < NUM_DIGITS; ++index )
        {
            EleksDigit    & digit = g_digits[ index ];
            uint8           value = digit.GetTimeValue() + '0';

            if (value <'0' || value > '9')
            {
                value = 0;
            }

            digitStr[ 0 ] = (char) value;
            if ( (!(index % 2)) && (index) )
            {
                digits += "&nbsp;";
            }
            digits += ColoredText( digit.GetTimeColor(), digitStr );
        }

        digits += "</b></font>";
        SetTitle( "", digits.c_str(), 1 );
    }

    // any warnings
    {
        char   * addBr = "";

        auto add =
            [this, &addBr]( char const *text )
            {
                AddF_br( text );
                addBr = "<br/>";
            };

        Add( "<font color='red'><b>" );
        if ( g_ntp.GetState() != NtpClient::WaitingForSyncTime )
        {
            add( "NTP time not synced" );
        }
            
        if ( g_timeZone.GetState() != TimeZone::WaitingForSyncTime )
        {
            add( "TimeZone not synced" );
        }

        if ( g_globalColor.IsDisplaying() == GlobalColor::TimeOff )
        {
            add( "Time is off" );
        }

        AddF( "</b>%s</font>", addBr );
    }

    {
        AddLinkDiv( "Settings" );
        AddF_br( "%d hour mode, %s mode, %ssplash screen", 
                (g_options._12Hour ? 12 : 24),
                (g_options._surpressLeadingZero ? "surpress0" : "00" ),
                (g_options._splashScreen ? "" : "no " ) );
        AddF_br( "Bright: %d, Dim: %d", g_options._bright, g_options._dim );
    }

    {
        AddLinkDiv( "TOnOff", "Display Time" );
        AddOnOff( g_options._timeOnOff );
    }

    {
        AddLinkDiv( "DOnOff", "Dim display" );
        AddOnOff(  g_options._dimOnOff );
    }

    {
        AddLinkDiv( "TopHr", "Top Of Hour" );
        AddColorEffect( g_options._topOfHour );
    }

    {
        AddLinkDiv( "QHr", "Quarter Of Hour" );
        AddColorEffect( g_options._quarterOfHour );
    }

    {
        AddLinkDiv( "Date" );
        if ( g_options._dateSecond )
        {
            AddColorEffect( g_options._dateColor );
            AddF_br( "Show at %02d second mark every %d minutes", g_options._dateSecond, g_options._dateMinutes );
            AddF_br( "Format: %s", (g_options._dateMmddyy ? "MMDDYY" : "YYMMDD") );
    
        }
        else
        {
            AddF_br( "Disabled" );
        }
    }

    EndDiv();
    Add( "<a href='r2'>More Settings</a><br><br>" );

    if ( g_globalColor.IsTurnedOff() )
    {
        AddLinkButton( "forceon?s=3600", "On 8hrs" );
    }
    else
    {
        AddLinkButton( "forceon?s=-3600", "Off 8hrs" );
    }

    if ( g_options._bright != g_options._dim )
    {
        if ( g_options._dimOnOff.IsDim() )
        {
            AddLinkButton( "forcedim?s=-3600", "!Dim 8hrs" );
        }
        else
        {
            AddLinkButton( "forcedim?s=3600", "Dim 8hrs" );
        }
    }

    AddLinkButton( "stats" );
    AddBr();
    AddBr();
    AddLinkButton( "TestPopup", "Popup" );
}


void
WebServer::OnRoot2()
{
    SetTitle( "More" );
   
    {
        AddLinkDiv( "NTP" );
        AddColorEffect( g_options._ntpColor );
        AddF_br( "Sync at %s then every %s", g_options._ntpSync.toString().c_str(), Duration(g_options._ntpFrequency*10*60).c_str() );
        AddF_br( "Last sync: %s", g_ntp.LastSync().c_str() );
    
        AddLinkDiv( "TimeZone" );
        if ( g_options._tzKey[0] )
        {
            AddColorEffect( g_options._tzColor );
        }
        else
        {
            AddF_br( "Manual" );
        }
    }
    
    {
        AddLinkDiv( "MicroAdjust" );            // link.. enable/disable.  smoothing data.  history data
        if ( g_options._madjEnable )
        {
            int32   madj = g_options._madjFreq * g_options._madjDir;
    
            AddF_br( "%s seconds per day", MadjSecondsPerDay( madj ).c_str() );
            AddF_br( "Last adjust: %s", LastMicroAdjust().c_str() );
        }
        else
        {
            AddF_br( "disabled" );
        }
    }
    
    {
        AddLinkDiv( "WebReq", "Web Request" );
        AddColorEffect( g_options._httpClient );
        AddF( "AP %srunning", (WiFi.getMode() & WIFI_AP) ? "" : "not-" );
    
        if ( (g_options._accessPointLifespan != 0) && (g_options._accessPointLifespan != 255) )
        {
            AddF_br( ", turn off after %s", Duration(g_options._accessPointLifespan*60).c_str() );
        }
        if ( WiFi.getMode() & WIFI_AP )
        {
            AddF_br( "IP: %s", WiFi.softAPIP().toString().c_str() );
        }
    
        AddBr();
    }

    EndDiv();
    AddLinkButton( "BNtp", "Sync Ntp" );
    AddLinkButton( "about" );

}


void
WebServer::OnSyncNtp()
{
    SnapOptions( "/r2" );
    g_ntp.ForceSync();
}


void
WebServer::OnSettings()
{
    auto addSlider =
        [this]( char const *desc, char id, uint8 level )
        {
            AddF_br( "%s: <input type='range' min='0' max='255' value='%d' name='%c'/>", desc, level, id );
        };

    AddClientForm( "Settings" );

    AddRadio( 'm', "12 ", "12hr ", g_options._12Hour );
    AddRadio( 'm', "24", "24hr", !g_options._12Hour );
    AddBr();

    AddCheckbox_br  ( 'z', "Surpress leading zeros", g_options._surpressLeadingZero );
    AddCheckbox_br  ( 's', "Splash screen on boot", g_options._splashScreen );

    addSlider( "Bright", 't', g_options._bright );
    addSlider( "Dim", 'u', g_options._dim );
}


void
WebServer::OnSetSettings()
{
    SnapOptions( "" );
    g_options._12Hour   = ( arg('m').toInt() == 12 );
    g_options._bright   = arg('t').toInt();
    g_options._dim      = arg('u').toInt();
    g_options._surpressLeadingZero  = !!arg('z').length();
    g_options._splashScreen         = !!arg('s').length();

}


void
WebServer::OnTimeOnOff()
{
    SetTitle( "Display Time" );
    AddClientForm( "TOnOff" );

    // 0=on time, 1=off time, 2,3,4,5,6,7,8,9 = sun-sat
    AddOnOffEdit( '0', "On between", g_options._timeOnOff );
}


void
WebServer::OnSetTimeOnOff()
{
    SnapOptions( "" );
    ParseOnOff( '0', &g_options._timeOnOff );
}


void
WebServer::OnDimOnOff()
{
    SetTitle( "Dimming Times" );
    AddClientForm( "DOnOff" );

    // 0=on time, 1=off time, 2,3,4,5,6,7,8,9 = sun-sat
    AddOnOffEdit( '0', "Dim between", g_options._dimOnOff );
}


void
WebServer::OnSetDimOnOff()
{
    SnapOptions( "" );
    ParseOnOff( '0', &g_options._dimOnOff );
}


void
WebServer::OnTopOfHour()
{
    SetTitle( "Top Of Hour" );
    AddClientForm( "TopHr" );
    AddColorEffectEdit( g_options._topOfHour );
}


void       
WebServer::OnSetTopOfHour()
{
    SnapOptions( "" );
    g_options._topOfHour = GetColorEffect();
}


void
WebServer::OnQuarterOfHour()
{
    SetTitle( "Quarter Of Hour" );
    AddClientForm( "QHr" );
    AddColorEffectEdit( g_options._quarterOfHour );
}


void
WebServer::OnSetQuarterOfHour()
{
    SnapOptions( "" );
    g_options._quarterOfHour = GetColorEffect();
}


void                
WebServer::OnShowDate()
{
    AddClientForm( "Date" );

    AddColorEffectEdit( g_options._dateColor );
    AddF( "<br>Show at the <input min='0' max='59' size='2' value='%d' name='s'/> second mark ", g_options._dateSecond );
    AddF( "every <select name='m'> " );
    _selectionValue = g_options._dateMinutes;
    AddOption( 1 );
    AddOption( 5 );
    AddOption( 15 );
    AddOption( 30 );
    AddOption( 60 );
    AddF_br( "</select> minutes" );

    Add( "Format:<select name='f'>" );
    AddOption( 0,  g_options._dateMmddyy, "MMDDYY" );
    AddOption( 1, !g_options._dateMmddyy, "YYMMDD" );
    AddF_br( "</select>" );
}


void       
WebServer::OnSetShowDate()
{
    SnapOptions( "" );
    g_options._dateColor    = GetColorEffect();
    g_options._dateMinutes  = arg('m').toInt();
    g_options._dateSecond   = arg('s').toInt();
    g_options._dateMmddyy   = ( arg('f').toInt() == 0 );
}


void       
WebServer::OnNtp()
{
    AddClientForm( "NTP" );

    // s - ntp server
    // t - sync time
    // f - frequency
    // e - highlight errors
    // (e,h,z - in AddColorEffectEdit)

    AddInput_br     ( 's', sizeof(g_options._ntpServer), g_options._ntpServer, "NTP Server: " );

    AddTime( 't', "Sync at ", g_options._ntpSync );
    AddF( " every <select name='f'>" );
    _selectionValue = g_options._ntpFrequency;
    AddOption(   1, "10 minutes" );
    AddOption(   3, "30 minutes" );
    AddOption(   6, "1 hour" );
    AddOption(  24, "4 hours" );
    AddOption(  72, "12 hours" );
    AddOption( 144, "day" );
    AddF_br( "</select>" );
    AddBr();

    Add( "<font color='#191970'>Ntp sync effect:</font><br>" );        // todo use css style here
    AddColorEffectEdit( g_options._ntpColor );
    AddBr();
}


void       
WebServer::OnSetNtp()
{
    SnapOptions( "/r2" );
    strncpy( g_options._ntpServer, arg( 's' ).c_str(), sizeof(g_options._ntpServer) );
    g_options._ntpSync = HourMinute( arg( 't' ) );
    g_options._ntpFrequency = arg( 'f' ).toInt();
    g_options._ntpColor = GetColorEffect();
}


void       
WebServer::OnTimeZone()
{
    AddClientForm( "TimeZone" );

    Add(    
        "<script>"
            "function LX2(){"
                "m=D('u');"
                "if(m.checked){a='visible';b='hidden';}else{a='hidden';b='visible';}"
                "V('ba',a);V('bb',b);"
            "}"
        "</script>" );

    AddF( "<input name='u' id='u' type='checkbox' onchange='LX2()' %s/>Enable (p-api.com &amp; api.timezonedb.com)", (g_options._tzKey[ 0 ] ? "checked" : "") );
    Add( "<div id='ba'>" );
        AddInput_br( 'k', sizeof(g_options._tzKey), g_options._tzKey, "timezonedb api key: " );
        Add( "<font color='#191970'>TimeZone sync effect:</font><br>" );
        AddColorEffectEdit( g_options._tzColor );

    Add( "</div><div id='bb'>" );
        AddDateTime( "Enter current date &amp; time for manual gmt offset<br>", g_now );     // adds 'f', 't' and 'd' (font, time, date)
    Add( "</div><br>" );

    AddBr();
}


void       
WebServer::OnSetTimeZone()
{
    SnapOptions( "/r2" );
    const bool      enabled     = !!arg('u').length();
    String          dateStr     = arg('d');
    char const    * date        = dateStr.c_str();
    HourMinute      hhmm ( HourMinute(arg('t')) );
    tmElements_t    tm;

    strncpy( g_options._tzKey, arg( 'k' ).c_str(), sizeof( g_options._tzKey ) );
    g_options._tzColor = GetColorEffect();

    if ( !enabled )
    {
        g_options._tzKey[ 0 ] = 0;
        g_options._gmtOffset = 0;

        // 1234-67-89
        if ( dateStr.length() == 10 )
        {
            tm.Year     = atoi( date ) - 1970;
            tm.Month    = atoi( date+5 );
            tm.Day      = atoi( date+8 );
            tm.Hour     = hhmm._hour;
            tm.Minute   = hhmm._minute;
            tm.Second   = 0;

            g_options._gmtOffset = ( makeTime(tm) - g_gmtTime );
            Out( "WebServer: TzManual: gmtOffset %d\n", g_options._gmtOffset );
        }
    }
}


void
WebServer::OnMicroAdjust()
{
    const int32  madj0 = g_options._madjFreq * g_options._madjDir;

    AddClientForm( "MicroAdjust" );

    // todo.. make manual allow setting of time
    AddCheckbox_br  ( 'e', "Enable", g_options._madjEnable );
    AddF_br( "Adjust %s seocnds per day (%dms per %d)", MadjSecondsPerDay(madj0).c_str(), g_options._madjDir, g_options._madjFreq );
    EndForm();
    AddF_br( "Log:" );
    LogToHtml( [this]( char const *txt ) { Add( txt ); } );
    AddBr();
}


void
WebServer::OnSetMicroAdjust()
{
    SnapOptions( "/r2" );
    g_options._madjEnable = !!arg( 'e' ).length();
}


void       
WebServer::OnWebReq()
{
    SetTitle( "Web Request" );
    AddClientForm( "WebReq" );

    // 
    Add( "<font color='#191970'>Web request effect:</font><br>" );
    AddColorEffectEdit( g_options._httpClient );
    AddBr();

    AddF( "Turn Access Point off in:  <select name='l'>" );
    _selectionValue = g_options._accessPointLifespan;
    AddOption(   0, "0 minutes" );
    AddOption(   1, "1 minute" );
    AddOption(   5, "5 minutes" );
    AddOption(  60, "60 minutes" );
    AddOption( 255, "Never" );
    AddF_br( "</select>" );
    AddCheckbox_br( 'p', "Allow popup url", g_options._allowPopupUrl );
    AddCheckbox_br( 'v', "Allow brightness url", g_options._allowBrightnessUrl );
    AddBr();

    AddInput_br( 'w', sizeof( g_options._password ), g_options._password, "Set new password for updating settings: " );
    AddBr();
}


void       
WebServer::OnSetWebReq()
{
    SnapOptions( "/r2" );
    g_options._httpClient           = GetColorEffect();
    g_options._accessPointLifespan  = arg( 'l' ).toInt();
    g_options._allowPopupUrl        = !!arg( 'p' ).length();
    g_options._allowBrightnessUrl   = !!arg( 'v' ).length();
    strncpy( g_options._password, arg('w').c_str(), sizeof( g_options._password ) );
}


void
WebServer::OnCreatePopup()
{
    ARGB        argb = { (ARGB::BlendInOut | 3), 0xFF, 0xA5, 0x00 };

    AddClientForm( "Popup" );
    AddInput_br( 'd', NUM_DIGITS, "", "Digits: " );
    AddColorEffectEdit( argb );
}


void
WebServer::OnSetPopup()
{
    const ARGB      argb    = GetColorEffect();
    int             pos     = 0;
    char            digits[ NUM_DIGITS + 1 ];
    String          url;

    SetTitle( "PopupUrl" );
    
    for ( char d : arg('d') )
    {
        if ( d < '0' || d > '9' )
        {
            d = 'x';
        }

        digits[ pos ] = d;
        pos += 1;
        if ( pos >= NUM_DIGITS )
        {
            break;
        }
    }
    digits[ pos ] = 0;

    url = PrintF(
        "http://%s/popup?d=%s&e=%s&h=%s&c=%s",
            client().localIP().toString().c_str(),
            digits,
            arg( 'e' ).c_str(),
            arg( 'h' ).c_str(),
            argb.toRgbString().c_str()
        );
        
    AddF( "URL: <a href='%s'>%s</a>", url.c_str(), url.c_str() );

    _pingColorState = false;
    Popup( digits, argb );
}


void
WebServer::OnPopup()
{
    String          digits  = arg( 'd' );
    const ARGB      argb    = GetColorEffect();

    _pingColorState = false;
    _isText = true;
    if ( g_options._allowPopupUrl )
    { 
        Popup( digits.c_str(), argb );
        AddF( "Popup( %s, %s )\n", digits.c_str(), argb.toString().c_str() );
    }
    else
    {
        Add( "disabled\n" );
    }
}


void
WebServer::OnForceOn()
{
    _pingColorState = false;
    _isText = true;
    if ( g_options._allowBrightnessUrl )
    {
        Add( g_options._timeOnOff.ForceOnOff( arg('s') ) );
    }
    else
    {
        Add( "disabled\n" );
    }
}


void
WebServer::OnForceDim()
{
    _pingColorState = false;
    _isText = true;
    if ( g_options._allowBrightnessUrl )
    {
        Add( g_options._dimOnOff.ForceOnOff( arg('s') ) );
    }
    else
    {
        Add( "disabled\n" );
    }
}

void
WebServer::OnTime()
{
    _pingColorState = false;
    _isText = true;
    Add( TimeStr().c_str() );
}


void
WebServer::OnStats()
{
    SetTitle( "Stats" );

    Console::MiscStats(
        [this]( char const *line )
        {
            Add( line );
        } );
}


void
WebServer::OnAbout()
{
    SetTitle( "About" );

    Console::MiscAbout(
        [this]( char const *line )
        {
            Add( line );
        } );
    AddBr();
    AddBr();
}


