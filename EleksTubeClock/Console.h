/*
 * Console.h
 *  Serial console
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */


class Console
{
public:
    void                Loop();

public:
    using HtmlOut   = std::function<void( char const * )>;

    static void         MiscStats( HtmlOut const & htmlOut );
    static void         MiscAbout( HtmlOut const & htmlOut );

public:
    struct Cmd
    {
        char const          _cmd[ 8 ];
        char const * const  _desc;
        void                ( Console::* const _handler )();
    };

private:
    static void         TextOut( HtmlOut const &out, char const *txt, ... );

private:
    void                OnHelp();
    void                OnStats();
    void                OnTime();
    void                OnSetName();
    void                OnSetSsid();
    void                OnAccessPointOn();
    void                OnAccessPointOff();
    void                OnWiFiDisconnect();
    void                OnWiFiScan();
    void                OnSyncNtp();
    void                OnWiFiBegin();
    void                OnTestForFlicker();
    void                OnReboot();
    void                OnPasswordReset();
    void                OnFactoryReset();
    void                OnAbout();

private:
    static Cmd const    g_cmds[];

private:
    uint8               _bufferPos;
    char                _buffer[ 25 ];
};

