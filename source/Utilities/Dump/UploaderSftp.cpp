#include "stdafx.h"

#include "UploaderSftp.h"

#include "CrashHandler.h"

#include "./ExternalObject/Chilkat/CkSFtp.h"

#ifdef _DEBUG
#pragma comment(lib, "ChilkatDbg.lib")
#else
#pragma comment(lib, "ChilkatRel.lib")
#endif

#pragma comment(lib, "rpcrt4.lib")
#pragma comment(lib, "dnsapi.lib")
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "ole32.lib")

namespace {
	void ConvertToMbcs( TCHAR* src, char* dst, int len );
	void ConvertToWide( char* src, TCHAR* dst, int len ); 
	const TCHAR* GetLastErrorText( CkSFtp* sftp );
}

extern CCrashHandler* g_hCrash;

CUploaderSftp::CUploaderSftp(LPCTSTR szUrl, int nPort, LPCTSTR szId, LPCTSTR szPassword, int nUploadCount)
: CUploader( szUrl, nPort, szId, szPassword, nUploadCount )
{
}

CUploaderSftp::~CUploaderSftp()
{
}

int	CUploaderSftp::UploadFTP(LPCTSTR szLocalFile, LPCTSTR szServerFile)
{
	char aUrl[1024];
	char aId[1024];
	char aPw[1024];
	char aLocalFile[MAX_PATH];
	char aServerFile[MAX_PATH];

	::memset( aUrl,	0, 1024 );
	::memset( aId,  0, 1024 );
	::memset( aPw,  0, 1024 );
	::memset( aLocalFile,  0, MAX_PATH );
	::memset( aServerFile, 0, MAX_PATH );

	ConvertToMbcs( m_szUrl, aUrl, 1024 );
	ConvertToMbcs( m_szId,  aId,  1024 );
	ConvertToMbcs( m_szPassword, aPw, 1024 );
	ConvertToMbcs( (TCHAR*)szLocalFile, aLocalFile, MAX_PATH );
	ConvertToMbcs( (TCHAR*)szServerFile, aServerFile, MAX_PATH );

	CkSFtp sftp; 

	sftp.put_ConnectTimeoutMs( 5000 );
	sftp.put_IdleTimeoutMs( 10000 );

	bool rc = true; 

	// License goes here
	rc = sftp.UnlockComponent("WEBZENSSH_s9Pqn98a2InM");
	if ( !rc )
	{
		g_hCrash->WriteCrashLog( _T("SFTP unlock failed") );

		return -1;
	}

	rc = sftp.Connect( aUrl, m_nPort ); 
	if ( !rc )
	{
		g_hCrash->WriteCrashLog( _T("Connect to sftp server %s:%d failed with %s"), 
				                 aUrl, m_nPort, GetLastErrorText( &sftp ) );
		return -1; 
	}

	rc = sftp.AuthenticatePw( aId, aPw );
	if ( !rc )
	{
		g_hCrash->WriteCrashLog( _T("Authenticate to sftp server failed") );
		return -1;
	}

	rc = sftp.InitializeSftp();
	if ( !rc )
	{
		g_hCrash->WriteCrashLog( _T("Intialize sftp session failed") );
		return -1;
	}
	
	// removed server directory creation
	// instead we assume directories are already created

	const char* handle = sftp.openFile( aServerFile, "writeOnly", "createTruncate" );

	if ( handle == 0 )
	{
		g_hCrash->WriteCrashLog( _T("Open file on sftp server failed with %s"), GetLastErrorText( &sftp ) );
		return -1;
	}

	rc = sftp.UploadFile( handle, aLocalFile );

	if ( !rc )
	{
		g_hCrash->WriteCrashLog( _T("Upload file to sftp server failed with %s"), GetLastErrorText( &sftp ) );

		return -1;
	}

	sftp.CloseHandle( handle );

	return 0;
}

namespace {

void ConvertToMbcs( TCHAR* src, char* dst, int len )
{

#ifdef _UNICODE
	WideCharToMultiByte(CP_ACP, 0, src, -1, dst, len, NULL, NULL);
#else
	sprintf_s( dst, len, "%s", src );
#endif
	
}

void ConvertToWide( char* src, TCHAR* dst, int len )
{
	MultiByteToWideChar(CP_ACP, 0, src, -1, (LPWSTR)dst, len);
}

const TCHAR* GetLastErrorText( CkSFtp* sftp )
{
	static TCHAR buf[1024] = { 0, };

	const char* msg = sftp->lastErrorText();

#ifdef _UNICODE
	ConvertToWide( (char*)msg, buf, 1024 );

	return buf;
#else
	return msg;
#endif
}

}
