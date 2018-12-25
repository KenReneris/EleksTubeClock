/*
 * Columns
 *  Output column/table data
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */


#include "platform.h"
#include "Columns.h"


Columns::Columns( uint indent, char const *fill, char const sep, std::function<void( char const * )> const &htmlTable )
    : _fill         ( fill )
    , _sep          ( sep )
    , _pass         ( 0 )
    , _indent       ( indent )
    , _htmlRowEnd   ( "" )
    , _isHtml       ( !!htmlTable )
    , _row          ( 0 )
    , _buffer       ( htmlTable ? htmlTable : OutStr )
{
    memset( _width, 0, sizeof( _width ) );
    if ( _isHtml )
    {
        _buffer.Add( "<div style='margin-left: 10px'><table border='1'>" );
    }
}


Columns::~Columns()
{
    if ( _isHtml )
    {
        _buffer.Add( _htmlRowEnd );
        _buffer.Add( "</table></div><br><br>" );
    }
    else
    {
        NewLine();
        NewLine();
    }
}


void
Columns::SetPass( uint pass )
{
    _pass = pass;
}


void        
Columns::Row( char const *str )
{
    if ( _pass )
    {
        _buffer.Add( str );
    }
}


void       
Columns::Row2( char const *c0, Fill fill, char const *c1 )
{
    if ( *c0 == '<' )
    {
        fill = Fill::Right;
        c0 += 1;
    }

    Cell( 0, Fill::Right, c0 );
    Cell( 1, fill,        c1 );
}


void
Columns::Row2( char const *c0, char const *c1 )
{
    Row2( c0, Fill::Left, c1 );
}


void       
Columns::Row2( char const *c0, String const &c1 )
{
    Row2( c0, Fill::Left, c1.c_str() );
}


void
Columns::Row2( char const *c0, uint c1 )
{
    char    buffer[ 32 ];

    sprintf( buffer, "%d", c1 );
    Row2( c0, Fill::Left, buffer );
}


void
Columns::Row2Flag( char const *c0, bool c1 )
{
    Row2( c0, Fill::Left, c1 ? "true" : "false" );
}


void       
Columns::Row3a( char const *c0, char const *link )
{
    Row3a( c0, "", link );
}


void
Columns::Row3a( char const *c0, char const *c1, char const *link )
{
    Cell ( 0, Fill::Right, c0 );
    Cell ( 1, Fill::Right, c1 );
    Cella( 2, link );
}


void       
Columns::Cella( uint col, char const *link )
{
    char const  * desc = link;

    if ( _isHtml )
    {
        Cell( col, Fill::None, PrintF("<a href='https://%s'>%s</a>", link, link).c_str() );
    }
    else
    {
        Cell( col, Fill::None, link );
    }
}


void
Columns::Cell( uint col, Fill fill, char const *txt )
{
    const uint      width       = strlen( txt );
    char            fillChar    = _fill[ col ];
   
    if ( _pass )
    {
        _row += 1;       
        if ( col == 0 )
        {
            if ( _row != 1 )
            {
                NewLine();
            }

            if ( !_isHtml )
            {
                _buffer.Fill( ' ', _indent );
            }
        }

        if ( _isHtml )
        {
            _buffer.AddF( "<td>%s</td>", txt );
        }
        else
        {
            if ( fill == Fill::Left )
            {
                _buffer.Fill( fillChar, _width[col] - width );
            }

            _buffer.Add( txt );

            // if there's a next column
            if ( (_width[col+1]) && (fillChar) )
            {
                if ( fill == Fill::Right )
                {
                    _buffer.Fill( fillChar, _width[col] - width );
                }

                _buffer.AddF( "%c%c ", fillChar, _sep );
            }
        }
    }
    else
    {
        if ( width > _width[col] )
        {
            _width[col] = width;
        }
    }
}



void
Columns::NewLine()
{
    if ( _isHtml )
    {
        _buffer.Add( _htmlRowEnd );
        _buffer.Add( "<tr>" );
        _htmlRowEnd = "</tr>";
    }
    else
    {
        _buffer.Add( "\n" );
    }
}


