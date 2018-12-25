/*
 * ZString
 *  Misc string functions & classes
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */

#include "platform.h"
#include <assert.h>


static StringBuffer     * g_strAppend;

void
Assert( bool f )
{
    assert( f );
}


void     
VAppendF( String &str, char const *format, va_list args )
{
    StringBuffer        
        buffer( 
            [&str]( char const *add )
            {
                str += add;
            } );

    Assert( !g_strAppend );
    g_strAppend = &buffer;
    ets_vprintf(
        []( int c )
        {
            g_strAppend->Add( c );
            return 1;
        },
        format,
        args
    );

    g_strAppend = nullptr;
}


///////////////////////////////////////////////////////////////////////////////////////////
//
//
//

StringBuffer::StringBuffer( std::function<void( char const * )> const & flush )
    : _pos      ( 0 )
    , _flush    ( flush )
{
}

StringBuffer::~StringBuffer()
{
    Flush();
}


void
StringBuffer::Reset()
{
    _pos = 0;
}


void
StringBuffer::Flush()
{
    if ( _pos )
    {
        _buffer[ _pos ] = 0;
        _flush( _buffer );
        _pos = 0;
    }
}


void
StringBuffer::Add( char c )
{
    if ( _pos >= sizeof(_buffer) - 2 )
    {
        Flush();
    }

    _buffer[ _pos ] = c;
    _pos += 1;
}


void 
StringBuffer::Add( const char *c )
{
    Add( c, strlen(c) );
}


void 
StringBuffer::Add( const String &s )
{
    Add( s.c_str(), s.length() );
}


void
StringBuffer::Add( const char *c, uint len )
{
    if ( _pos+len > sizeof(_buffer)-1 )
    {
        Flush();
        if ( len > sizeof(_buffer)-1 )
        {
            _flush( c );
            len = 0;
        }
    }

    memcpy( _buffer + _pos, c, len );
    _pos += len;
}


void 
StringBuffer::AddF( const char *format, ... )
{
    va_list         args;

    va_start( args, format );
    VAddF( format, args );
    va_end( args );
}


void 
StringBuffer::VAddF( const char *format, va_list args )
{
    Assert( !g_strAppend );
    g_strAppend = this;

    ets_vprintf(
        []( int c )
        {
            g_strAppend->Add( c );
            return 1;
        },
        format,
        args
        );

    g_strAppend = nullptr;
}


void
StringBuffer::Fill( const char c, int len )
{
    for (; len > 0; --len)
    {
        Add( c );
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//
//


String
PrintF( char const *format, ... )
{
    va_list     args;

    va_start ( args, format );
    return VPrintF( format, args );
    // skipped va_end
}


String
VPrintF( char const *format, va_list args )
{
    String      str;

    VAppendF( str, format, args );
    return str;
}


void 
AppendF( String &str, char const *format, ... )
{
    va_list args;
    va_start ( args, format );

    VAppendF( str, format, args );

    va_end ( args );
}



//////////////////////////////////////////////////////////////////////////////////
//
//
//

String
Duration( uint32 x )
{
    int         seconds;
    int         hours;
    int         minutes;
    int         days;
    String      str;

    auto addSep = 
        [&]()
        {
            if ( str.length() )
            {
                str += " ";
            }
        };

    auto addNum =
        [&]( char const *text, int v )
        {
            if ( v )
            {
                addSep();
                AppendF( str, "%d %s%s", v, text, (v > 1 ? "s" : "") );
            }
        };

    seconds = x % 60;
    x /= 60;

    minutes = x % 60;
    x /= 60;

    hours = x % 24;
    x /= 24;

    days = x;

    addNum( "day", days );
    addNum( "hour", hours );
    addNum( "minute", minutes );
    addNum( "second", seconds );
    if ( !str.length() )
    {
        str += " ";
    }

    return str;
}


String 
UrlEncode( const String &str )
{
    String      r;

    for ( char c : str )
    {
        if ( c == ' ' )
        {
            r += '+';
 
        }
        else if ( isalnum(c) )
        {
            r += c;
        }
        else
        {
            AppendF( r, "%%%02X", c );
        }
    }

    return r;
}


String
u64str( int64 v )
{
    char    buffer[ 32 ];
    char  * p = buffer + sizeof( buffer ) - 1;
    bool    neg = false;

    if (v < 0)
    {
        v = -v;
        neg = true;
    }

    *p = 0;
    do
    {
        p -= 1;
        *p = '0' + (v % 10);
        v /= 10;
    }
    while (v);

    if (neg)
    {
        p -= 1;
        *p = '-';
    }

    return String( p );
}
