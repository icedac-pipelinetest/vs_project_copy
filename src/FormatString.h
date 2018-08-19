#pragma once
/****************************************************************************
 * 	
 *		FormatString.h
 *      ($\Olive\tools\vs_project_copy\src)
 *
 *		by icedac@gmail.com (2014/04/24)
 *
 */
#ifndef _VS__FORMATSTRING_H_
#define _VS__FORMATSTRING_H_

#include <string>

/****************************************************************************
*
*	TStringPrintf, TVStringPrintf
*
*/
#ifdef _MSC_VER
template < typename... Args >
inline int TStringPrintf( char* s, size_t count, const char* format, Args... args )
{
    return _snprintf_s( s, count, _TRUNCATE, format, args... );
}
template < typename... Args >
inline int TStringPrintf( wchar_t* s, size_t count, const wchar_t* format, Args... args )
{
    return _snwprintf_s( s, count, _TRUNCATE, format, args... );
}
inline int VStringPrintf( char* s, size_t count, const char* format, va_list args )
{
    return _vsnprintf_s( s, count, _TRUNCATE, format, args );
}
inline int VStringPrintf( wchar_t* s, size_t count, const wchar_t* format, va_list args )
{
    return _vsnwprintf_s( s, count, _TRUNCATE, format, args );
}
#else
#error Need to define TStringPrintf for other platform.
#endif

template < typename CharType, size_t Count, typename... Args >
inline int TStringPrintf( CharType( &s )[Count], const CharType* format, Args... args )
{
    return TStringPrintf( (CharType*) s, Count, format, args... );
}
template < typename CharType, size_t Count >
inline int TVStringPrintf( CharType( &s )[Count], const CharType* format, va_list args )
{
    return VStringPrintf( (CharType*) s, Count, format, args );
}

/****************************************************************************
*
*	TFormatString
*
*/
template < typename CharType, int STRING_BUFFER_SIZE = 8192 >
class TFormatString
{
public:
    typedef std::basic_string< CharType > String;

    inline TFormatString( const CharType* format, ... )
    {
        formatted_[STRING_BUFFER_SIZE - 1] = 0; // ensure trailing '\0'
        va_list args;
        va_start( args, format );
        TVStringPrintf( formatted_, format, args );
        va_end( args );
    }

    inline operator String() const
    {
        return AsString();
    }

    inline String AsString() const
    {
        return String( formatted_ );
    }
    inline const CharType* AsStr() const
    {
        return reinterpret_cast<const CharType*>(formatted_);
    }

protected:
    CharType formatted_[STRING_BUFFER_SIZE];
};

typedef TFormatString<char, 8192>	    FormatStringA;
typedef TFormatString<wchar_t, 8192>	FormatStringW;

#ifdef _UNICODE
typedef FormatStringW                   FormatString;
#else
typedef FormatStringA                   FormatString;
#endif

#endif // _VS__FORMATSTRING_H_

