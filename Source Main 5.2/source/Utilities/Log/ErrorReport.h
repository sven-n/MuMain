// ErrorReport.h: interface for the CErrorReport class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ERROR_REPORT_H__8C7DEDBA_0557_4B98_AD53_900F41EAC8AD__INCLUDED_)
#define AFX_ERROR_REPORT_H__8C7DEDBA_0557_4B98_AD53_900F41EAC8AD__INCLUDED_

#pragma once

#define MAX_LENGTH_CPUNAME	( 128)
#define MAX_LENGTH_OSINFO	( 128)
#define MAX_DXVERSION		( 128)


typedef struct
{
	char m_lpszCPU[MAX_LENGTH_CPUNAME];
	char m_lpszOS[MAX_LENGTH_OSINFO];
	int m_iMemorySize;

	char m_lpszDxVersion[MAX_DXVERSION];
} ER_SystemInfo;


class CErrorReport
{
public:
	CErrorReport();
	virtual ~CErrorReport();

	void Clear( void);

protected:
	HANDLE m_hFile;
	char m_lpszFileName[MAX_PATH];
	int m_iKey;
#ifdef ASG_ADD_MULTI_CLIENT
	int m_nFileCount;		// 로그 파일 개수.
#endif	// ASG_ADD_MULTI_CLIENT
	
public:
	void Create( char *lpszFileName);
	void Destroy( void);
protected:
	void CutHead( void);
	char* CheckHeadToCut( char *lpszBuffer, DWORD dwNumber);

protected:
	BOOL WriteFile( HANDLE hFile, void* lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
public:
	void WriteDebugInfoStr( char *lpszToWrite);
	void Write( const char* lpszFormat, ...);
	void HexWrite( void *pBuffer, int iSize);

	void AddSeparator( void);
	void WriteLogBegin( void);
	void WriteCurrentTime( BOOL bLineShift = TRUE);

	void WriteSystemInfo( ER_SystemInfo *si);
	void WriteOpenGLInfo( void);
	void WriteImeInfo( HWND hWnd);
	void WriteSoundCardInfo( void);
};


extern CErrorReport g_ErrorReport;


//////////////////////////////////////////////////////////////////////
// System Setting check
//////////////////////////////////////////////////////////////////////

void GetSystemInfo( ER_SystemInfo *si);

#endif // !defined(AFX_ERROR_REPORT_H__8C7DEDBA_0557_4B98_AD53_900F41EAC8AD__INCLUDED_)
