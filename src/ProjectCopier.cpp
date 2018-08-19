/****************************************************************************
 *
 *  ProjectCopier.cpp
 *      ($\Olive\tools\vs_project_copy\src)
 *
 *		by icedac@gmail.com (2014/04/24)
 *     (2014/8/19)
 *
 ***/
#include "stdafx.h"
#include "FormatString.h"

#pragma comment(lib, "rpcrt4.lib")  // UuidCreate - Minimum supported OS Win 2000

/*
rules for project copy

%old%: AppEchoServer
%new%: AppSessionServer
%OLD%: APPECHOSERVER
%NEW%: APPSESSIONSERVER

1. copy all folder from %old% to %new%
2. rename all file names containing %old% to %new%
3. replace texts all files from '*.h; *.inl; *.hpp; *.cpp; *.c; *.filters; *.vcxproj'
%old% -> %new% (case sensitive)
C%old% -> C%new% (case sensitive)
_%OLD%_ -> _%NEW%_ (uppercase)
4. give a new UUID

*/

/****************************************************************************
 * 	
 *	
 */
#define MAX_PATH 260
#define NULL_STMT ((void)0)

template<class chartype, class findchartype>
chartype* IStrChrR( chartype* str, findchartype ch )
{
    chartype* start = str;
    for ( ; *str; ++str )
        NULL_STMT;
    while ( str-- > start )
        if ( *str == ch )
            return str;
    return nullptr;
}
char* StrChrR( char* str, char ch )
{
    return IStrChrR( str, ch );
}

wchar_t* StrChrR( wchar_t* str, wchar_t ch )
{
    return IStrChrR( str, ch );
}

template<class chartype, class findchartype>
static chartype* IStrChr( chartype* str, findchartype ch, size_t chars )
{
    for ( ; chars--; ++str )
        if ( *str == ch )
            return str;
        else if ( !*str )
            break;
    return nullptr;
}

char* StrChr( char* str, char ch, size_t chars )
{
    return IStrChr( str, ch, chars );
}

wchar_t* StrChr( wchar_t* str, wchar_t ch, size_t chars )
{
    return IStrChr( str, ch, chars );
}

template<class chartype>
chartype* IStrStr( chartype source[], const chartype match[] )
{
    if ( !*match )
        return source;

    for ( chartype* curr = source; *curr; ++curr )
    {
        chartype* s1 = curr;
        const chartype* s2 = match;
        while ( *s1 && *s2 && *s1 == *s2 )
            s1++, s2++;
        if ( !*s2 )
            return curr;
    }

    return nullptr;
}
char* StrStr( char* source, const char match[] )
{
    return IStrStr( source, match );
}

const char* StrStr( const char source[], const char match[] )
{
    return IStrStr<const char>( source, match );
}

wchar_t* StrStr( wchar_t* source, const wchar_t match[] )
{
    return IStrStr( source, match );
}

const wchar_t* StrStr( const wchar_t source[], const wchar_t match[] )
{
    return IStrStr<const wchar_t>( source, match );
}

template<class chartype>
size_t IStrLen( const chartype str[] )
{
    size_t chars = 0;
    for ( ; *str++; ++chars )
        NULL_STMT;
    return chars;
}
unsigned StrLen( const char str[] )
{
    size_t length = IStrLen( str );
    assert( length <= (size_t) (unsigned) -1 );
    return (unsigned) length;
}

unsigned StrLen( const wchar_t str[] )
{
    size_t length = IStrLen( str );
    assert( length <= (size_t) (unsigned) -1 );
    return (unsigned) length;
}

class FindHandle {
public:
	FindHandle(HANDLE h) : handle_(h) {}
	FindHandle(FindHandle&& r) {
		*this = std::move(r);
	}
	FindHandle& operator = (FindHandle&& r) {
		handle_ = r.handle_;
		r.handle_ = INVALID_HANDLE_VALUE;
		return *this;
	}
	FindHandle(const FindHandle&) = delete;
	FindHandle& operator = (const FindHandle&&) = delete;
	FindHandle& operator = (HANDLE h) {
		if (handle_ != INVALID_HANDLE_VALUE)
			FindClose(handle_);
		handle_ = h;
		return *this;
	}

	~FindHandle() {
		if (handle_ != INVALID_HANDLE_VALUE)
			::FindClose(handle_);
		handle_ = INVALID_HANDLE_VALUE;
	}

	operator HANDLE () const {
		return handle_;
	}

private:
	HANDLE handle_;
};

std::wstring GenerateGUID() {
	UUID uuid;
	UuidCreate(&uuid);
	wchar_t *str = nullptr;
	UuidToStringW(&uuid, (RPC_WSTR*)&str);
	std::wstring s = str;
	RpcStringFreeW((RPC_WSTR*)&str);
	return s;
}

/***************************************************************************
*
*   class ProjectCopier
*
***/
class ProjectCopier
{
public:
    ProjectCopier( _TCHAR* srcName, _TCHAR* dstName ) :
        m_srcName( srcName ), m_fileFilter( nullptr ), m_dstName( dstName )
    {
        m_flagBackup = false;
        m_flagPrompt = false;
		m_fileFilter = _T(";*.h;*.inl;*.hpp;*.cpp;*.c;*.filters;*.vcxproj");
    }

public:
    void Run()
    {
        _tprintf( _T( "try to ProjectCopy from [srcDir:%s] to [%s] (filter: %s)\n" ),
                  m_srcName, m_dstName, m_fileFilter );

        // make a filter list
        _TCHAR f[8192] = { 0, };
        TStringPrintf( f, m_fileFilter );
        std::vector< std::wstring > filters;
        wchar_t* p = nullptr;
        while ( (p = StrChrR( f, L';' )) )
        {
            *p = 0;
            filters.push_back( std::wstring(p + 1) );
        }

        
        CopyDir( m_srcName, m_dstName );
		
        RenameFiles( m_dstName, m_srcName, m_dstName );

        // do replaces in files
        for ( auto& filter : filters )
        {
            using namespace std::placeholders;

            FindFile( m_dstName, filter.c_str(), std::bind( &ProjectCopier::DoFile, this, _1 ) );
        }
    }

    static void RenameFile( const _TCHAR* srcPath, const _TCHAR* dstPath )
    {
        //_tprintf( _T( "try to rename from [%s] to [%s]\n" ),
        //          srcPath, dstPath );
        MoveFile( srcPath, dstPath );
    }

    void RenameFiles( const _TCHAR* srcDir, const _TCHAR* src, const _TCHAR* dst )
    {
        FindFile( srcDir, FormatStringW( L"%s.*", src ).AsStr(), [ = ]( const _TCHAR* path ) {
            unsigned path_len = StrLen( path );
            const wchar_t* ext = StrChr( const_cast<wchar_t*>(path), L'.', path_len ) + 1;
            RenameFile( path, FormatStringW( L"%s\\%s.%s", srcDir, dst, ext ).AsStr() );
        } );
    }

    void CopyDir( const _TCHAR* src, const _TCHAR* dst )
    {
        std::wstring src_str = std::wstring( src ) + L"\\*";
        std::wstring dst_str = std::wstring( dst );
        WCHAR sf[MAX_PATH + 1] = { 0, };
        WCHAR df[MAX_PATH + 1] = { 0, };
        wcscpy_s( sf, MAX_PATH, src_str.c_str() );
        wcscpy_s( df, MAX_PATH, dst_str.c_str() );
        sf[wcslen( sf )+1] = 0;
        df[wcslen( df )+1] = 0;
        // the string should ended with double terminated zero. "a\0";

        SHFILEOPSTRUCTW s = { 0 };
        s.wFunc = FO_COPY;
        s.fFlags = FOF_SILENT | FOF_NOCONFIRMMKDIR | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NO_UI;
        //s.pFrom = "C:\\source folder\\*\0";
        //s.pTo = "C:\\target folder\0";
        s.pFrom = sf;
        s.pTo = df;
        int res = SHFileOperationW( &s );
    }

    void FindFile( const _TCHAR* dir, const _TCHAR* fileFilter, std::function< void( const _TCHAR* ) > func )
    {
        _TCHAR path[MAX_PATH] = { 0, };
        TStringPrintf( path, _T( "%s\\%s" ), dir, fileFilter );
		_tprintf(_T("processing... [%s]\n"), path);
                  // m_srcName, m_dstName, m_fileFilter );

        WIN32_FIND_DATA fileData;
		FindHandle hSearch = FindFirstFile( path, &fileData );
        while ( hSearch != INVALID_HANDLE_VALUE )
        {
            _tprintf( _T( "\t[%c%c%c] %s\\%s ..." ),
                      (fileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ? _T( 'R' ) : _T( '.' ),
                      (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? _T( 'D' ) : _T( '.' ),
                      (fileData.dwFileAttributes & FILE_ATTRIBUTE_NORMAL) ? _T( 'F' ) : _T( '.' ),
                      dir, fileData.cFileName );
            TStringPrintf( path, _T( "%s\\%s" ), dir, fileData.cFileName );
            FILE* f = nullptr;
            _tfopen_s( &f, path, _T( "rt" ) );
            if ( f )
            {
                _tprintf( _T( "ok\n" ) );
                fclose( f );

                func( path );
            }
            else
            {
                _tprintf( _T( "fail\n" ) );
            }
            if ( FindNextFile( hSearch, &fileData ) == 0 )
                break;
        }

        // do that in sub-directory
        TStringPrintf( path, _T( "%s\\*.*" ), dir, fileFilter );
        hSearch = FindFirstFile( path, &fileData );
        while ( hSearch != INVALID_HANDLE_VALUE )
        {
            if ( fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
                 && fileData.cFileName[0] != _T( '.' ) )
            {
                // _tprintf( _T("\t[ D] %s\\%s ...\n"), dir, fileData.cFileName );

                TStringPrintf( path, _T( "%s\\%s" ), dir, fileData.cFileName );
                FindFile( path, fileFilter, func );
            }

            if ( FindNextFile( hSearch, &fileData ) == 0 )
                break;
        }
	}

    const wchar_t* ReplaceName( const wchar_t* line, const wchar_t* from, const wchar_t* to )
    {
        wchar_t* s = nullptr;
        if ( (s = StrStr( const_cast<wchar_t*>(line), from )) != nullptr )
        {
            wchar_t temp = *s;
            *s = 0;
            static wchar_t buf[8192];
            TStringPrintf( buf, _T( "%s%s%s" ), line, to, s + StrLen( from ) );
            *s = temp;
            return buf;
        }

        // return new string if line changed.
        return nullptr;
    }

    const wchar_t* DoLine( const wchar_t* line )
    {
        bool changed = false;
        static wchar_t buf[8192];
        TStringPrintf( buf, L"%s", line );

		// #0 
		if (StrStr(const_cast<wchar_t*>(line), L"<ProjectGuid>") != nullptr) 
		{
			// maybe whole line is ProjectGuid
			// maybe it can be fail someday then fix it at that time :-)
			auto wguid = GenerateGUID();

			for (auto& c : wguid) c = towupper(c);

			wprintf((L" - give a new GUID: %s\n"), wguid.c_str() );

			TStringPrintf(buf, L"    <ProjectGuid>{%s}</ProjectGuid>", wguid.c_str());
			return buf;
		}

		// #1 replace names
        while ( const wchar_t* re = ReplaceName( buf, m_srcName, m_dstName ) )
        {
            changed = true;
            TStringPrintf( buf, L"%s", re );
        }

        wchar_t srcU[8192] = { 0, };
        wchar_t dstU[8192] = { 0, };
        for ( int i = 0; m_srcName[i]; ++i ) srcU[i] = towupper( m_srcName[i] );
        for ( int i = 0; m_dstName[i]; ++i ) dstU[i] = towupper( m_dstName[i] );

		// #2 replace _UPPER_NAME_
        while ( const wchar_t* re = ReplaceName( buf, FormatStringW( L"_%s_", srcU ).AsStr(), FormatStringW( L"_%s_", dstU ).AsStr() ) )
        {
            changed = true;
            TStringPrintf( buf, L"%s", re );
        }

		// #3 gen new GUID


        if ( !changed ) return nullptr;

        return buf;
    }

    void DoFile( const _TCHAR* path )
    {
        std::vector< std::wstring > lines;
        std::vector< std::wstring > removeLines;

        std::wstring line;
        std::wifstream in( path );
        unsigned lineNo = 1;
        while ( !in.bad() && std::getline( in, line ) )
        {
            // wprintf( _T("%s\n"), line.c_str() );

            const wchar_t* new_line = DoLine( line.c_str() );
            if ( new_line )
            {
                _TCHAR buf[8192] = { 0, };
                TStringPrintf( buf, _T( "[%4d] %s" ), lineNo, line.c_str() );
                removeLines.push_back( buf );
                lines.push_back( new_line );
            }
            else
            {
                lines.push_back( line );
            }
            ++lineNo;
        }
        in.close();
        if ( removeLines.empty() )
            return;

        if ( m_flagPrompt && !DoTextDialogYesOrNo( _T( "The file has read-only flag, do you want to modify that?\n" ) ) )
            return;

        SetFileAttributes( path, FILE_ATTRIBUTE_NORMAL );

        if ( m_flagBackup )
        {
            _TCHAR removedPath[MAX_PATH] = { 0, };
            TStringPrintf( removedPath, _T( "%s.removed_line" ), path );
            std::wofstream removedOut( removedPath );
            if ( removedOut.bad() )
            {
                wprintf( _T( "file open failed! %s\n" ), removedPath );
                return;
            }
            auto itr = removeLines.begin();
            auto itrEnd = removeLines.end();
            while ( !removedOut.bad() && itr != itrEnd )
            {
                removedOut << *itr << std::endl;
                ++itr;
            }
            removedOut.close();

            wprintf( ( L" - removed lines from %s saved to %s.\n" ), path, removedPath );
        }

        std::wofstream out( path );
        auto itr = lines.begin();
        auto itrEnd = lines.end();
        while ( !out.bad() && itr != itrEnd )
        {
            out << *itr << std::endl;
            ++itr;
        }
        out.close();

        wprintf( ( L" - %s file modified.\n" ), path );

        return;
    }

    bool DoTextDialogYesOrNo( const _TCHAR* msg )
    {
        _tprintf( _T( "\t===> %s Answer 'y' or 'n'\n" ), msg );
        int inputChar = 0;
        while ( (inputChar = GetInput()) != 'y' && inputChar != 'n' )
        {
            Sleep( 1 );
        }

        if ( inputChar == 'y' )
            return true;
        return false;
    }

    int GetInput()
    {
        int ch = _getch();
        return tolower( ch );
    }

private:
    _TCHAR*     m_srcName;
    _TCHAR*     m_dstName;

    _TCHAR*     m_fileFilter;

    bool        m_flagBackup;
    bool        m_flagPrompt;
};

int _tmain(int argc, _TCHAR* argv[])
{
	if ( argc != 3 )
    {
        _tprintf( _T( "usage: %s src_url dst_url" ), argv[0] );
        return -1;
    }

    ProjectCopier copier( argv[1], argv[2] );

    copier.Run();

    return 0;
}

