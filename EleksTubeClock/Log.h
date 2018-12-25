/*
 * Log.h
 *  Debugging
 *
 * Author: Ken Reneris <https://github.com/KenReneris>
 * MIT License
 * ----------------------------------------------------------
 */


void    Log( char const *format, ... );
void    LogToHtml( std::function<void( char const * )> htmlOut );

