/*
 * Log
 *  In memory text log for debugging.
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */

#include "platform.h"



static uint16   g_logTail;
static char     g_log[ 512 ];


void
AppendToLog( char const *txt )
{
    for( ; *txt; ++txt )
    {
        g_log[ g_logTail ] = *txt;
        g_logTail += 1;
        if( g_logTail >= countof(g_log) )
        {
            g_logTail = 0;
        }
    }
}


void    
Log( char const *format, ... )
{
    va_list         args;
    StringBuffer    sb(
        []( char const *txt )
        {
            OutStr( txt );
            AppendToLog( txt );
        }
    );

    va_start( args, format );
    sb.VAddF( format, args );
    va_end( args );
}


void
LogToHtml( std::function<void( char const * )> htmlOut )
{
    StringBuffer    sb( htmlOut );
    uint            index   = g_logTail + 1;

    while( index != g_logTail )
    {
        char    c = g_log[ index ];
        
        index += 1;
        if ( index >= countof(g_log) )
        {
            index = 0;
        }

        if ( c == '\n' )
        {
            break;
        }
    }

    while( index != g_logTail )
    {
        char    c = g_log[ index ];

        if ( c != '\n' )
        {
            sb.Add( c );
        }
        else
        {
            sb.Add( "<br>" );
        }

        index += 1;
        if ( index >= countof(g_log) )
        {
            index = 0;
        }
    }
}

