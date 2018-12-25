/*
 * WebServer.h
 *  Small web page server
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */

class WebServer : public ESP8266WebServer
{
public:
    WebServer();

    uint32              ResponsesSent() const;

    void                on( char const *uri, THandlerFunction handler );
    void                on( char const *uri, bool ap, THandlerFunction handler );
    void                begin();
    String              arg( char name );

    // Construct html response
    void                ResetPageState();
    void                CheckClient();
    void                SetTitle( char const *title );
    void                SetTitle( char const *title, uint8 scale );
    void                SetTitle( char const *title, char const *pageName, uint8 scale );
    void                AddBr();
    void                Add( char const *html );
    void                Add( String const &html );
    void                AddF( char const *format, ... );
    void                AddF_br( char const *format, ... );
    void                AddForm( char const *action );
    void                EndForm();
    void                AddClientForm( char const *action );
    void                AddInput( char id, int8 length, char const *value );
    void                AddInput_br( char id, int8 length, char const *value );
    void                AddInput_br( char id, int8 length, char const *value, char const *desc );
    void                AddInput2_br( char id, int8 length, char const *placeholder );
    void                AddInputPassword_br( char id, int8 length, char const *placeholder );
    void                AddRadio( char id, char const *value, char const *desc, bool selected );
    void                AddCheckbox( char id, char const *desc, bool selected );
    void                AddCheckbox_br( char id, char const *desc, bool selected );
    void                AddOption( uint value );
    void                AddOption( uint value, char const *name );
    void                AddOption( uint value, bool selected, char const *name );
    void                AddTime( char id, char const *desc, HourMinute value );
    void                AddDateTime( char const *desc, time_t value );
    void                AddLinkButton( char const *uri );
    void                AddLinkButton( char const *uri, char const *desc );
    void                AddOnOff( const OnOff & onOff );
    void                AddOnOffEdit( char os, char const *desc, const OnOff &onOff );
    void                ParseOnOff( const char os, OnOff *onOff );
    void                AddColorEffect( ARGB argb );
    void                AddColorEffectEdit( ARGB argb );
    ARGB                GetColorEffect();
    void                AddLinkDiv( char const *str );
    void                AddLinkDiv( char const *uri, char const *text );

    void                AddCachingAllowed();
    void                SendPage();

private:
    static bool         IsIp( String str );
    static String       ColoredText( ARGB rgb, char const *text );

private:
    void                SendHeader( char const *name, const String &value );
    void                SendHeader( char const *name, const char *value ); 
    void                Send( int code, char const * contentType, const String& content );
    void                Send( int code, char const * contentType, char const *content );
    void                Redirect( String const &uri, bool sendSessionId );
    void                Redirect( char const *uri );
    void                Redirect( char const *uri, bool sendSessionId );

    void                OnNotFound();
    void                EndDiv();

private:
    void                SnapOptions( char const *redir );

    // misc page handlers
    void                OnRoot();
    void                OnRoot2();
    void                OnGetPassword();
    void                OnSetPassword();
    void                OnSettings();
    void                OnSetSettings();
    void                OnTimeOnOff();
    void                OnSetTimeOnOff();
    void                OnDimOnOff();
    void                OnSetDimOnOff();
    void                OnTopOfHour();
    void                OnSetTopOfHour();
    void                OnQuarterOfHour();
    void                OnSetQuarterOfHour();
    void                OnShowDate();
    void                OnSetShowDate();
    void                OnNtp();
    void                OnSetNtp();
    void                OnTimeZone();
    void                OnSetTimeZone();
    void                OnMicroAdjust();
    void                OnSetMicroAdjust();
    void                OnWebReq();
    void                OnSetWebReq();
    void                OnColorPickerMinJs();
    void                OnColorPickerMinCss();
    void                OnPopup();
    void                OnForceDim();
    void                OnForceOn();
    void                OnTime();
    void                OnStats();
    void                OnAbout();
    void                OnSyncNtp();
    void                OnCreatePopup();
    void                OnSetPopup();

private:
    static const char   k_daysOfWeek3[];

private:
    uint                _responses;                 // misc stat
    uint32              _clientId;                  // if login required
                     
    // per-request
    StringBuffer        _buffer;                    // place to build some output before reallocating the content string
    bool                _responseSent;              // only send 1 response per request
    bool                _pingColorState;            // ping g_globalColor that a web request arrived
    uint8               _titleScale;                // non-zero if title has been set
    bool                _formStarted;               // true if a form was started (and </form> is needed)
    bool                _divStarted;                
    bool                _isText;                    // _content is a text response
    String              _content;                   // the response (right now is 1 big string?)
    Options           * _snappedOptions;            // if processing a form, this is the before state of the options
    char const        * _snapRedir;                 // if processing a form, this is the page to redirect the client too
    uint                _selectionValue;            // used for making selection values easier
};



