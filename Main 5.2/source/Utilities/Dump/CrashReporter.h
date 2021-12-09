#ifndef _CRASHREPORT_H_
#define _CRASHREPORT_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <DbgHelp.h>

typedef bool (*LPCALLBACK) (_EXCEPTION_POINTERS*);

namespace Crash
{
	BOOL Install(
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
		LPCTSTR szDumpfile = NULL,
		bool bWriteCrashLog = false);
	void Uninstall();
	BOOL AddFile(LPCTSTR szFilename);
	LONG HandleException(PVOID pExceptionAddress, DWORD dwExceptionCode);
	LONG HandleExceptionWithInfo(PEXCEPTION_POINTERS pExInfo);
}
#endif;