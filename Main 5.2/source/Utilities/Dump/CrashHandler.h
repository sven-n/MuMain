#ifndef _CRASHHANDLER_H_
#define _CRASHHANDLER_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "CrashReporter.h"
#include <list>

#include "Uploader.h"
#include "Logger.h"

#pragma comment(lib, "DbgHelp")

// For GetAdaptersInfo
#include <IPHlpApi.h>
#pragma comment(lib, "IPHlpApi")

// Define C runtime error
#define C_RUNTIME_WARNING		((DWORD   )0x00100000L)
#define C_RUNTIME_ERROR			((DWORD   )0x00100001L)
#define C_RUNTIME_ASSERTION		((DWORD   )0x00100002L)
#define UNEXPECTED_ERROR		((DWORD   )0x00100003L)

// for InternetOpen
typedef struct 
{
	HINTERNET* pInternetSession;
	HINTERNET* pHttpConnection;
	LPCTSTR szUrl;
} HTTP_PARM;

class CCrashHandler
{
public:
	const static int BUFFER_SIZE = 256;
	const static int MAX_FILE_COUNT = 5;
	const static DWORD TIME_OUT = 1000;
	const static int MAX_STACKTRACE = 10;
	const TCHAR* STACKTRACE_FILENAME;

	CCrashHandler(
		LPCALLBACK lpfnCallbackBefore,
		LPCALLBACK lpfnCallbackAfter,
		int nProjectID,
		LPCTSTR szClientVersion,
		MINIDUMP_TYPE dumpType,
		LPCTSTR szHttpUrl,
		LPCTSTR szFtpUrl,
		int nPort,
		LPCTSTR szId,
		LPCTSTR szPassword,
		int nUploadCount,
		bool bCumulative,
		bool bDeleteDumpfile,
		bool bStackTrace,
		LPCTSTR szDumpfile,
		bool bWriteCrashLog);
	~CCrashHandler();

	LONG							HandleException(PEXCEPTION_POINTERS pExInfo, PVOID pExceptionAddress, DWORD dwExceptionCode);
	BOOL							AddFile(LPCTSTR szFilename);
	void							WriteCrashLog(LPCTSTR szFormat, ...);

private:
	void							Destroyer();
	void							GetExceptionCode(DWORD dwExceptionCode, TCHAR strExceptionCode[]);
	LONG							ReportException(LPCTSTR szUrl);
	LONG							Submit(LPCTSTR szUrl, char szBuffer[]);
	LONG							GenerateErrorReport(PEXCEPTION_POINTERS pExInfo, PVOID pExceptionAddress);
	bool							IsFile(LPCTSTR szFilename);
	void							SetParameter(TCHAR szParameters[], LPCTSTR szKey, LPCTSTR szValue);
	void							GetMacAddress(TCHAR szParameters[], bool bHyphen=TRUE);
	bool							StackTrace(PEXCEPTION_POINTERS pExInfo);

	LPCALLBACK						m_lpfnCallbackBefore;	// user callback function pointer(before handle exception)
	LPCALLBACK						m_lpfnCallbackAfter;	// user callback function pointer(after handle exception)
	int								m_nProjectId;	// project id
	TCHAR							m_szClientVersion[BUFFER_SIZE];	// client version
	_MINIDUMP_TYPE					m_dumpType;	// level of dump file
	TCHAR							m_szHttpUrl[BUFFER_SIZE];	// crash reporter website url
	bool							m_bCumulative;	// upload overlapping dump file or not
	bool							m_bDeleteDumpfile;	// delete dumpfile after uploading
	bool							m_bStackTrace;	// whether stack trace or not

	LPTOP_LEVEL_EXCEPTION_FILTER	m_oldFilter;	// previous exception filter
	_invalid_parameter_handler		m_oldHandler;	// previous invalid parameter handler
	unexpected_handler				m_oldUnexpectedHandler;	// previous unexpected handler
	terminate_handler				m_oldTerminateHandler;	// previous terminate handler
	_CRT_REPORT_HOOK				m_oldReportFunction;	// previous reporting function

	CLogger*						m_pLogger;	// logger pointer
	std::list< LPCTSTR >			m_lstFile;	// added file list

	bool							m_bCatchException; // catch exception or not
	bool							m_bUpload;	// upload dump file or not
	bool							m_bReportSuccess;	// result of reporting

	TCHAR							m_szDumpfile[BUFFER_SIZE];	// minidump file name
	TCHAR							m_szMacAddress[BUFFER_SIZE]; // user mac address
	TCHAR							m_szDate[BUFFER_SIZE];	// date of exception occurrence
	TCHAR							m_szTime[BUFFER_SIZE];	// time of exception occurrence

	int								m_nFileCount;	// maximum file count
};

#endif	// #ifndef _CRASHHANDLER_H_