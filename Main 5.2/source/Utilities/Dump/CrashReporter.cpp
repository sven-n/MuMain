#include "stdafx.h"
#include "CrashHandler.h"

#ifdef _DEBUG
#define CRASH_ASSERT(pObj)          \
   if (!pObj || sizeof(*pObj) != sizeof(CCrashHandler))  \
      DebugBreak()                                       
#else
#define CRASH_ASSERT(pObj)
#endif // _DEBUG

CCrashHandler* g_hCrash;

BOOL Crash::Install(
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
	bool bWriteCrashLog)
{
	g_hCrash = new CCrashHandler(
		lpfnCallbackBefore,
		lpfnCallbackAfter,
		nProjectID,
		szClientVersion,
		dumpType,
		szHttpUrl,
		szFtpUrl,
		nPort,
		szId,
		szPassword,
		nUploadCount,
		bCumulative,
		bDeleteDumpfile,
		bStackTrace,
		szDumpfile,
		bWriteCrashLog);

	if (g_hCrash == NULL || sizeof(*g_hCrash) != sizeof(CCrashHandler))
		return FALSE;

	return TRUE;
}

void Crash::Uninstall()
{
	CRASH_ASSERT(g_hCrash);

	delete g_hCrash;
}

BOOL Crash::AddFile(LPCTSTR szFilename)
{
	return g_hCrash->AddFile(szFilename);
}

LONG Crash::HandleException(PVOID pExceptionAddress, DWORD dwExceptionCode)
{
	return g_hCrash->HandleException(NULL, pExceptionAddress, dwExceptionCode);
}

LONG Crash::HandleExceptionWithInfo(PEXCEPTION_POINTERS pExInfo)
{
	return g_hCrash->HandleException(pExInfo, 0, UNEXPECTED_ERROR);
}