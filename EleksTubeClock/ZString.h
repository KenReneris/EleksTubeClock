/*
 * ZString.h
 *  Misc string functions
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */



extern String   PrintF( char const *format, ... );
extern String   VPrintF( char const *format, va_list args );
extern void     AppendF( String &str, char const *format, ... );
extern void     VAppendF( String &str, char const *format, va_list args );

String      Duration( uint32 seconds );
String      UrlEncode( const String &str );
String      u64str( int64 v );


// like a string builder, but invokes the "fluch" callback function when full
class StringBuffer
{
public:
    StringBuffer( std::function<void(char const*)> const & flush );
    ~StringBuffer();

    void            Reset();

    void            Add( const char c );
    void            Add( const char *c );
    void            Add( const String &c );
    void            Add( const char *c, uint len );
    void            AddF( const char *format, ... );
    void            VAddF( const char *format, va_list args );
    void            Fill( const char c, int len );
    void            Flush();

protected:
    uint8           _pos;
    char            _buffer[ 256-1-16 ];
    std::function<void( char const* )> _flush;
};


