#include "stdafx.h"

#include "CrashHandler.h"

extern CCrashHandler* g_hCrash;

DWORD WINAPI InternetOpenUrlThread(LPVOID vThreadParm)
{
	HTTP_PARM* pThreadParm;
	// Initialize local pointer to void pointer passed to thread
	pThreadParm = (HTTP_PARM*)vThreadParm;

	*(pThreadParm->pHttpConnection) = ::InternetOpenUrl(
		*(pThreadParm->pInternetSession),
		pThreadParm->szUrl,
		NULL,
		0,
		INTERNET_FLAG_NO_CACHE_WRITE,
		NULL);

	if (*(pThreadParm->pHttpConnection) == NULL)
		return 1;
	return 0;
}

// Unhandled exception callback set with SetUnhandledExceptionFilter()
// EXCEPTION_EXECUTE_HANDLER		Return from UnhandledExceptionFilter and execute the associated exception handler.
// EXCEPTION_CONTINUE_EXECUTION		Return from UnhandledExceptionFilter and continue execution from the point of the exception.
// EXCEPTION_CONTINUE_SEARCH		Proceed with normal execution of UnhandledExceptionFilter.
LONG WINAPI CustomUnhandledExceptionHandler(PEXCEPTION_POINTERS pExInfo)
{
	return g_hCrash->HandleException(pExInfo, 0, UNEXPECTED_ERROR);
}

// Invalid parameter handler set with _set_invalid_parameter_handler()
void CustomInvalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved)
{
}

// Unexpected error handler set with set_unexpected(), set_terminate()
void CustomUnexpectedErrorHandler()
{
	g_hCrash->HandleException(NULL, 0, UNEXPECTED_ERROR);
	exit(-1);
}

// Client-defined reporting function set with _CrtSetReportHook
int __cdecl CustomHook(int nReportType, char* szMsg, int* pnRet)
{
	// Write errMsg.txt
	FILE *pFile;
	fopen_s(&pFile, "ErrMsg.txt", "w");

	if (pFile != NULL)
	{
		fprintf(pFile, "%s", szMsg);
		fclose(pFile);
	}

	g_hCrash->AddFile(_T("ErrMsg.txt"));

	// Handle exception
	switch (nReportType)
	{
	case _CRT_WARN:
		break;
	case _CRT_ERROR:
		g_hCrash->HandleException(NULL, 0, C_RUNTIME_ERROR);
		exit(1);
		break;
	case _CRT_ASSERT: 
		break;
	default:
		break;
	}

	// A retVal value of zero continues execution, a value of 1 starts the debugger.
	if (pnRet)
		*pnRet = 1;

	// If the hook handles the message in question completely, so that no further reporting is required, it should return TRUE.
	// If it returns FALSE, _CrtDbgReport will report the message normally.
	return TRUE;
}

CCrashHandler::CCrashHandler(
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
	: m_lpfnCallbackBefore(NULL), m_lpfnCallbackAfter(NULL), m_nProjectId(nProjectID), m_dumpType(dumpType), m_bCumulative(bCumulative), m_bStackTrace(bStackTrace),
	m_bDeleteDumpfile(bDeleteDumpfile), m_bCatchException(true), m_bUpload(false), m_bReportSuccess(true), m_nFileCount(0)
{
	SYSTEMTIME t;

	// Initialize
	if (bWriteCrashLog)
		m_pLogger = new CLogger();
	else
		m_pLogger = NULL;

	_stprintf_s(m_szClientVersion, BUFFER_SIZE, _T("%s"), szClientVersion);
	_stprintf_s(m_szHttpUrl, BUFFER_SIZE, _T("%s"), szHttpUrl);
	STACKTRACE_FILENAME = _T("Stacktrace.txt");

	if (szDumpfile!=NULL)
	{
		_stprintf_s(m_szDumpfile, BUFFER_SIZE, _T("%s"), szDumpfile);
		m_bCatchException = false;
	}
	else
	{
		::GetLocalTime(&t);

		_stprintf_s(m_szDumpfile, BUFFER_SIZE, _T("crash_%04d%02d%02d_%02d%02d.dmp"), t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute);
	}

	// Save user supplied callback
	if (lpfnCallbackBefore)
		m_lpfnCallbackBefore = lpfnCallbackBefore;
	if (lpfnCallbackAfter)
		m_lpfnCallbackAfter = lpfnCallbackAfter;

	// Set filters
	if (m_bCatchException)
	{
		// Add this handler in the exception callback chain
		m_oldFilter = SetUnhandledExceptionFilter(CustomUnhandledExceptionHandler);

		// Establish a new terminate handler and a new unexpected handler
		m_oldUnexpectedHandler = set_unexpected(CustomUnexpectedErrorHandler);
		m_oldTerminateHandler = set_terminate(CustomUnexpectedErrorHandler);
	
		WriteCrashLog(_T("Install"));
	}
	else
	{
		WriteCrashLog(_T("Install(without catching exception)"));
	}
}

CCrashHandler::~CCrashHandler()
{
	// Reset exception callback
	if (m_bCatchException)
	{
		if (m_oldFilter)
			SetUnhandledExceptionFilter(m_oldFilter);
		if (m_oldHandler)
			_set_invalid_parameter_handler(m_oldHandler);
		if (m_oldUnexpectedHandler)
			set_unexpected(m_oldUnexpectedHandler);
		if (m_oldTerminateHandler)
			set_terminate(m_oldTerminateHandler);
		if (m_oldReportFunction)
			_CrtSetReportHook(m_oldReportFunction);
	}

	WriteCrashLog(_T("Uninstall"));

	delete m_pLogger;
}

void CCrashHandler::Destroyer()
{
	WriteCrashLog(_T("Terminate"));

	delete m_pLogger;
	::DeleteFile(_T("errMsg.txt"));
}

LONG CCrashHandler::HandleException(PEXCEPTION_POINTERS pExInfo, PVOID pExceptionAddress, DWORD dwExceptionCode)
{
	TCHAR szUrl[2048] = {0, };
	TCHAR szParameter[BUFFER_SIZE] = {0, };
	TCHAR szParameters[2048] = {0, };

	// Call user callback function(before handle exception)
	if (m_lpfnCallbackBefore)
	{
		WriteCrashLog(_T("User callback funtion(before) is called"));

		if (!m_lpfnCallbackBefore(pExInfo))
		{
			WriteCrashLog(_T("User callback funtion(before) returns false"));

			Destroyer();

			return EXCEPTION_EXECUTE_HANDLER;
		}
	}

	WriteCrashLog(_T("Set the exception information"));

	// Set parameters
	_stprintf_s(szParameter, BUFFER_SIZE, _T("%p"), pExceptionAddress);
	SetParameter(szParameters, _T("address"), szParameter);

	GetExceptionCode(dwExceptionCode, szParameter);
	SetParameter(szParameters, _T("code"), szParameter);

	SetParameter(szParameters, _T("dump"), m_szDumpfile);

	SetParameter(szParameters, _T("mac"), m_szMacAddress);

	_stprintf_s(szParameter, BUFFER_SIZE, _T("%d"), m_nProjectId);
	SetParameter(szParameters, _T("id"), szParameter);

	_stprintf_s(szParameter, BUFFER_SIZE, _T("%d"), (m_bCumulative ? 1 : 0));
	SetParameter(szParameters, _T("isCumulative"), szParameter);

	_stprintf_s(szParameter, BUFFER_SIZE, _T("%s"), m_szClientVersion);
	SetParameter(szParameters, _T("clientVersion"), szParameter);

	SetParameter(szParameters, _T("stacktrace"), pExInfo != NULL && m_bStackTrace ? _T("1") : _T("0"));

	LPCTSTR szFilename;
	std::list<LPCTSTR>::iterator itor;
	int i = 0;
	for (itor = m_lstFile.begin(); itor != m_lstFile.end(); itor++)
	{
		_stprintf_s(szParameter, BUFFER_SIZE, _T("file%d"), (i + 1));

		szFilename = _tcsrchr(*itor, '/');
		if (szFilename == NULL)
			szFilename = *itor;
		else
			szFilename++;

		SetParameter(szParameters, szParameter, szFilename);
		i++;
	}

	// Set url
	_stprintf_s(szUrl, 2048, _T("http://%s/Insert.aspx?1%s"), m_szHttpUrl, szParameters);

	// Report exception to DB
	LONG lReturn = ReportException(szUrl);
	if (lReturn > 0)
	{
		WriteCrashLog(_T("Report failed"));
		m_bReportSuccess = false;

		if (lReturn == 2)
		{
			SYSTEMTIME t;
			::GetLocalTime(&t);

			_stprintf_s(m_szDate, BUFFER_SIZE, _T("%04d-%02d-%02d"), t.wYear, t.wMonth, t.wDay);
			_stprintf_s(m_szTime, BUFFER_SIZE, _T("%02d-%02d-%02d"), t.wHour, t.wMinute, t.wSecond);
		}
	}

	return GenerateErrorReport(pExInfo, pExceptionAddress);
}

void CCrashHandler::GetExceptionCode(DWORD dwExceptionCode, TCHAR strExceptionCode[])
{
	switch (dwExceptionCode)
	{
	case STATUS_WAIT_0:						_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_WAIT_0")); break;
	case STATUS_ABANDONED_WAIT_0:			_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_ABANDONED_WAIT_0")); break;
	case STATUS_USER_APC:					_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_USER_APC")); break;
	case STATUS_TIMEOUT:					_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_TIMEOUT")); break;
	case STATUS_PENDING:					_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_PENDING")); break;
	case DBG_EXCEPTION_HANDLED:				_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("DBG_EXCEPTION_HANDLED")); break;
	case DBG_CONTINUE:						_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("DBG_CONTINUE")); break;
	case STATUS_SEGMENT_NOTIFICATION:		_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_SEGMENT_NOTIFICATION")); break;
	case DBG_TERMINATE_THREAD:				_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("DBG_TERMINATE_THREAD")); break;
	case DBG_TERMINATE_PROCESS:				_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("DBG_TERMINATE_PROCESS")); break;
	case DBG_CONTROL_C:						_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("DBG_CONTROL_C")); break;
	case DBG_CONTROL_BREAK:					_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("DBG_CONTROL_BREAK")); break;
	case DBG_COMMAND_EXCEPTION:				_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("DBG_COMMAND_EXCEPTION")); break;
	case STATUS_GUARD_PAGE_VIOLATION:		_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_GUARD_PAGE_VIOLATION")); break;
	case STATUS_DATATYPE_MISALIGNMENT:		_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_DATATYPE_MISALIGNMENT")); break;
	case STATUS_BREAKPOINT:					_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_BREAKPOINT")); break;
	case STATUS_SINGLE_STEP:				_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_SINGLE_STEP")); break;
	case DBG_EXCEPTION_NOT_HANDLED:			_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("DBG_EXCEPTION_NOT_HANDLED")); break;
	case STATUS_ACCESS_VIOLATION:			_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_ACCESS_VIOLATION")); break;
	case STATUS_IN_PAGE_ERROR:				_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_IN_PAGE_ERROR")); break;
	case STATUS_INVALID_HANDLE:				_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_INVALID_HANDLE")); break;
	case STATUS_NO_MEMORY:					_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_NO_MEMORY")); break;
	case STATUS_ILLEGAL_INSTRUCTION:		_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_ILLEGAL_INSTRUCTION")); break;
	case STATUS_NONCONTINUABLE_EXCEPTION:	_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_NONCONTINUABLE_EXCEPTION")); break;
	case STATUS_INVALID_DISPOSITION:		_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_INVALID_DISPOSITION")); break;
	case STATUS_ARRAY_BOUNDS_EXCEEDED:		_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_ARRAY_BOUNDS_EXCEEDED")); break;
	case STATUS_FLOAT_DENORMAL_OPERAND:		_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_FLOAT_DENORMAL_OPERAND")); break;
	case STATUS_FLOAT_DIVIDE_BY_ZERO:		_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_FLOAT_DIVIDE_BY_ZERO")); break;
	case STATUS_FLOAT_INEXACT_RESULT:		_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_FLOAT_INEXACT_RESULT")); break;
	case STATUS_FLOAT_INVALID_OPERATION:	_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_FLOAT_INVALID_OPERATION")); break;
	case STATUS_FLOAT_OVERFLOW:				_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_FLOAT_OVERFLOW")); break;
	case STATUS_FLOAT_STACK_CHECK:			_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_FLOAT_STACK_CHECK")); break;
	case STATUS_FLOAT_UNDERFLOW:			_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_FLOAT_UNDERFLOW")); break;
	case STATUS_INTEGER_DIVIDE_BY_ZERO:		_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_INTEGER_DIVIDE_BY_ZERO")); break;
	case STATUS_INTEGER_OVERFLOW:			_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_INTEGER_OVERFLOW")); break;
	case STATUS_PRIVILEGED_INSTRUCTION:		_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_PRIVILEGED_INSTRUCTION")); break;
	case STATUS_STACK_OVERFLOW:				_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_STACK_OVERFLOW")); break;
	case STATUS_CONTROL_C_EXIT:				_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_CONTROL_C_EXIT")); break;
	case STATUS_FLOAT_MULTIPLE_FAULTS:		_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_FLOAT_MULTIPLE_FAULTS")); break;
	case STATUS_FLOAT_MULTIPLE_TRAPS:		_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_FLOAT_MULTIPLE_TRAPS")); break;
	case STATUS_REG_NAT_CONSUMPTION:		_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_REG_NAT_CONSUMPTION")); break;
	case STATUS_SXS_EARLY_DEACTIVATION:		_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_SXS_EARLY_DEACTIVATION")); break;
	case STATUS_SXS_INVALID_DEACTIVATION:	_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("STATUS_SXS_INVALID_DEACTIVATION")); break;

	case 0xE06D7363:						_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("Microsoft C++ Exception")); break;

	case C_RUNTIME_WARNING:					_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("C_RUNTIME_WARNING")); break;
	case C_RUNTIME_ERROR:					_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("C_RUNTIME_ERROR")); break;
	case C_RUNTIME_ASSERTION:				_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("C_RUNTIME_ASSERTION")); break;
	case UNEXPECTED_ERROR:					_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("%s"), _T("UNEXPECTED_ERROR")); break;

	default:								_stprintf_s(strExceptionCode, BUFFER_SIZE, _T("0x%08X"), dwExceptionCode);
	}
}

LONG CCrashHandler::ReportException(LPCTSTR szUrl)
{
	WriteCrashLog(_T("Report the exception"));

	char szBuffer[BUFFER_SIZE] = {0, };
	LONG lReturn = 0;

	if (Submit(szUrl, szBuffer) == 1)
	{
		m_bUpload = true;
		return 2;
	}

	// Upload minidump file is needed
	if (szBuffer[0] == '1')
	{
		WriteCrashLog(_T("Uploading file is needed"));
		m_bUpload = true;
	}
	// Failed to insert log
	else if (szBuffer[0] == '2')
	{
		WriteCrashLog(_T("Database error: Failed to insert exception log"));
		return 2;
	}
	// Failed to insert log and upload minidump file is needed
	else if (szBuffer[0] == '3')
	{
		WriteCrashLog(_T("Database error: Failed to insert exception log2"));
		m_bUpload = true;
		lReturn = 1;
	}
	else if (szBuffer[0] != '0')
	{
		m_bUpload = true;
		return 2;
	}

	char *pDate, *pTime;
	pDate = strtok_s(&szBuffer[2], " ", (char **)&pTime);

#ifdef _UNICODE
	MultiByteToWideChar(CP_ACP, 0, pDate, -1, m_szDate, BUFFER_SIZE);
	MultiByteToWideChar(CP_ACP, 0, pTime, -1, m_szTime, BUFFER_SIZE);
#else
	sprintf_s(m_szDate, BUFFER_SIZE, "%s", pDate);
	sprintf_s(m_szTime, BUFFER_SIZE, "%s", pTime);
#endif

	return lReturn;
}

LONG CCrashHandler::Submit(LPCTSTR szUrl, char szBuffer[])
{
	HINTERNET hInternetSession, hHttpConnection;
	DWORD dwResultLength = 0;
	BOOL bResult = TRUE;

	hInternetSession = ::InternetOpen(
		NULL,
		INTERNET_OPEN_TYPE_PRECONFIG,
		NULL,
		NULL,
		0);

	if (!hInternetSession)
	{
		WriteCrashLog(_T("InternetOpen() failed"));
		return 1;
	}

	// Create a worker thread 
	HANDLE hThread; 
	DWORD dwThreadID;
	DWORD dwExitCode = 0;
	HTTP_PARM threadParm;
	threadParm.pInternetSession = &hInternetSession;
	threadParm.pHttpConnection = &hHttpConnection;
	threadParm.szUrl = szUrl;

	hThread = CreateThread(
		NULL,			// Pointer to thread security attributes 
		0,				// Initial thread stack size, in bytes 
		InternetOpenUrlThread,	// Pointer to thread function 
		&threadParm,	// The argument for the new thread
		0,				// Creation flags 
		&dwThreadID);	// Pointer to returned thread identifier 

	// Wait for the call to InternetConnect in worker function to complete
	if (WaitForSingleObject(hThread, TIME_OUT) == WAIT_TIMEOUT)
	{
		WriteCrashLog(_T("InternetOpenUrl() is timeout"));
		if (hInternetSession)
			InternetCloseHandle(hInternetSession);
		WaitForSingleObject(hThread, INFINITE);
		InternetCloseHandle(hInternetSession);
		return 1;
	}

	// The state of the specified object (thread) is signaled
	if (!GetExitCodeThread(hThread, &dwExitCode))
	{
		WriteCrashLog(_T("Error on GetExitCode of InternetOpenUrlThread"));
		if (hInternetSession)
			InternetCloseHandle(hInternetSession);
		return 1;
	}

	if (!hHttpConnection || dwExitCode)
	{
		WriteCrashLog(_T("InternetOpenUrl() failed"));
		if (hInternetSession)
			InternetCloseHandle(hInternetSession);
		return 1;
	}

	if (szBuffer != NULL)
	{
		bResult = ::InternetReadFile(
			hHttpConnection,
			szBuffer,
			BUFFER_SIZE - 1,
			&dwResultLength);
	}

	InternetCloseHandle(hHttpConnection);
	InternetCloseHandle(hInternetSession);

	if (bResult == FALSE)
	{
		WriteCrashLog(_T("InternetReadFile() failed"));
		return 1;
	}

	return 0;
}

LONG CCrashHandler::GenerateErrorReport(PEXCEPTION_POINTERS pExInfo, PVOID pExceptionAddress)
{
	TCHAR szServerFilePrefix[BUFFER_SIZE] = {0, };
	TCHAR szServerFilename[BUFFER_SIZE] = {0, };
	TCHAR szClientFilename[BUFFER_SIZE] = {0, };
	BOOL bCreateDumpSuccess = FALSE;

	// Set file path
	_stprintf_s(
		szServerFilePrefix,
		BUFFER_SIZE,
		_T("%d_%s_%p_%s"),
		m_nProjectId,
		m_szDate,
		pExceptionAddress,
		m_szTime);

	if (m_bCatchException)
	{
		// Create the file
		HANDLE hFile = CreateFile(
			m_szDumpfile,
			GENERIC_WRITE,
			FILE_SHARE_WRITE,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		// Write the minidump to the file
		if (hFile != INVALID_HANDLE_VALUE)
		{
			MINIDUMP_EXCEPTION_INFORMATION eInfo;
			eInfo.ThreadId = GetCurrentThreadId();
			eInfo.ExceptionPointers = pExInfo;
			eInfo.ClientPointers = FALSE;

			WriteCrashLog(_T("Write the minidump to the file"));

			bCreateDumpSuccess = MiniDumpWriteDump(
				GetCurrentProcess(),
				GetCurrentProcessId(),
				hFile,
				m_dumpType,
				pExInfo ? &eInfo : NULL,
				NULL,
				NULL);

			if (bCreateDumpSuccess)
				WriteCrashLog(_T("Succeeded to write the minidump file"));
			else
				WriteCrashLog(_T("Failed to write the minidump file"));
		}
		else
		{
			WriteCrashLog(_T("Failed to create the minidump file"));
		}

		// Close file
		CloseHandle(hFile);
	}

	bool bUploadSuccess = true;
	if (bCreateDumpSuccess || !m_bCatchException)
	{
		// Delete minidump file
		if (m_bDeleteDumpfile && m_bCatchException)
		{
			::DeleteFile(m_szDumpfile);
		}

		// Delete stacktrace file
		if (m_bDeleteDumpfile)
		{
			::DeleteFile(STACKTRACE_FILENAME);
		}
	}
	else
	{
		WriteCrashLog(_T("Failed to write the minidump file"));
	}

	// Call user callback function(after handle exception)
	if (m_lpfnCallbackAfter)
	{
		WriteCrashLog(_T("User callback funtion(after) is called"));
		if (!m_lpfnCallbackAfter(pExInfo))
			WriteCrashLog(_T("User callback funtion(after) returns FALSE"));
	}

	// Upload additional files
	LPCTSTR szFilename;
	std::list<LPCTSTR>::iterator itor;

	for (itor = m_lstFile.begin(); itor != m_lstFile.end(); itor++)
	{
		// Get filename from filepath
		szFilename = _tcsrchr(*itor, '/');
		if (szFilename == NULL)
			szFilename = *itor;
		else
			szFilename++;

		// File not exist
		if (!IsFile(*itor))
		{
			WriteCrashLog(_T("Upload %s:\t%s"), szFilename, _T("Failed(file not exist)"));
			continue;
		}
	}

	if (m_bReportSuccess && bCreateDumpSuccess && bUploadSuccess)
		WriteCrashLog(_T("Succeeded in handling the exception"));
	else
		WriteCrashLog(_T("Failed to handle the exception"));

	if (m_bCatchException)
		Destroyer();

	return EXCEPTION_EXECUTE_HANDLER;
}

BOOL CCrashHandler::AddFile(LPCTSTR szFilename)
{
	if (MAX_FILE_COUNT <= m_nFileCount || szFilename == NULL)
		return false;

	m_lstFile.push_back(szFilename);
	m_nFileCount++;

	return true;
}

bool CCrashHandler::IsFile(LPCTSTR szFilename)
{
	FILE* pFile;
	_tfopen_s(&pFile, szFilename, _T("rb"));

	if (pFile == NULL)
		return false;

	fclose(pFile);
	return true;
}

void CCrashHandler::SetParameter(TCHAR szParameters[], LPCTSTR szKey, LPCTSTR szValue)
{
	TCHAR szParameter[BUFFER_SIZE] = {0, };

	_stprintf_s(szParameter, BUFFER_SIZE, _T("&%s=%s"), szKey, szValue);
	_tcscat_s(szParameters, 2048, szParameter);
}

void CCrashHandler::GetMacAddress(TCHAR szParameters[], bool bHyphen)
{
	PIP_ADAPTER_INFO Info;
	DWORD size;

	ZeroMemory(&Info, sizeof(PIP_ADAPTER_INFO));
	size = sizeof(PIP_ADAPTER_INFO);

	if (GetAdaptersInfo(Info, &size) == ERROR_BUFFER_OVERFLOW)
	{
		Info = (PIP_ADAPTER_INFO)malloc(size);
		GetAdaptersInfo(Info, &size);
	}

	if ( szParameters != NULL )
	{
		if ( bHyphen )
		{
			_stprintf_s(szParameters, BUFFER_SIZE, _T("%0.2x-%0.2x-%0.2x-%0.2x-%0.2x-%0.2x"),
				Info->Address[0], Info->Address[1], Info->Address[2], Info->Address[3], Info->Address[4], Info->Address[5]);
		}
		else
		{
			_stprintf_s(szParameters, BUFFER_SIZE, _T("%0.2X%0.2X%0.2X%0.2X%0.2X%0.2X"),
				Info->Address[0], Info->Address[1], Info->Address[2], Info->Address[3], Info->Address[4], Info->Address[5]);
		}
	}

	delete Info;
}

extern char m_Version[];

bool CCrashHandler::StackTrace(PEXCEPTION_POINTERS pExInfo)
{
	// Open file
	FILE *pFile = NULL;
	_tfopen_s(&pFile, STACKTRACE_FILENAME, _T("w"));
	if (pFile == NULL)
		return false;

	fprintf(pFile, "Client Version %s\r\n\r\n", m_Version );

	HANDLE hProcess = GetCurrentProcess();
	HANDLE hThread = GetCurrentThread();
	BOOL ret;

	SymSetOptions(SYMOPT_LOAD_LINES);

	if (SymInitialize(hProcess, 0, TRUE) == TRUE)
	{
		CONTEXT &context = *pExInfo->ContextRecord;

		// Set stackFrame
		STACKFRAME64 stackFrame = { 0, };
		DWORD imageType;
		#ifdef _M_IX86
		  imageType = IMAGE_FILE_MACHINE_I386;
		  stackFrame.AddrPC.Offset = context.Eip;
		  stackFrame.AddrPC.Mode = AddrModeFlat;
		  stackFrame.AddrFrame.Offset = context.Ebp;
		  stackFrame.AddrFrame.Mode = AddrModeFlat;
		  stackFrame.AddrStack.Offset = context.Esp;
		  stackFrame.AddrStack.Mode = AddrModeFlat;
		#elif _M_X64
		  imageType = IMAGE_FILE_MACHINE_AMD64;
		  stackFrame.AddrPC.Offset = context.Rip;
		  stackFrame.AddrPC.Mode = AddrModeFlat;
		  stackFrame.AddrFrame.Offset = context.Rsp;
		  stackFrame.AddrFrame.Mode = AddrModeFlat;
		  stackFrame.AddrStack.Offset = context.Rsp;
		  stackFrame.AddrStack.Mode = AddrModeFlat;
		#elif _M_IA64
		  imageType = IMAGE_FILE_MACHINE_IA64;
		  stackFrame.AddrPC.Offset = context.StIIP;
		  stackFrame.AddrPC.Mode = AddrModeFlat;
		  stackFrame.AddrFrame.Offset = context.IntSp;
		  stackFrame.AddrFrame.Mode = AddrModeFlat;
		  stackFrame.AddrBStore.Offset = context.RsBSP;
		  stackFrame.AddrBStore.Mode = AddrModeFlat;
		  stackFrame.AddrStack.Offset = context.IntSp;
		  stackFrame.AddrStack.Mode = AddrModeFlat;
		#endif

		for (int i = 0; i < MAX_STACKTRACE && stackFrame.AddrPC.Offset; i++)
		{
			ret = StackWalk64(imageType, hProcess, hThread, &stackFrame, &context, 0, 0, 0, 0);

			if (ret != FALSE)
			{
				DWORD64 dwAddress;
				DWORD dwDisplacement;

				ULONG64 buffer[(sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR) + sizeof(ULONG64) - 1) / sizeof(ULONG64)];
				PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;

				pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
				pSymbol->MaxNameLen = MAX_SYM_NAME;

				// Get Information
				ret = SymFromAddr(hProcess, stackFrame.AddrPC.Offset, &dwAddress, pSymbol);
				if (ret == TRUE)
				{
					IMAGEHLP_LINE64	errorLine = { sizeof(IMAGEHLP_LINE64) };
					IMAGEHLP_LINE64	symbolLine = { sizeof(IMAGEHLP_LINE64) };

					dwDisplacement = 0;
					BOOL ret1, ret2;

					ret1 = SymGetLineFromAddr64(hProcess, stackFrame.AddrPC.Offset, &dwDisplacement, &errorLine);
					ret2 = SymGetLineFromAddr64(hProcess, stackFrame.AddrPC.Offset - (DWORD)dwAddress, &dwDisplacement, &symbolLine);
					if (ret1 == TRUE && ret2 == TRUE)
					{
						fprintf(pFile, "%s(): %d line (%#x)\r\n", pSymbol->Name, errorLine.LineNumber - symbolLine.LineNumber, pSymbol->Address );

						fprintf(pFile, "\t%s: %d line\r\n", errorLine.FileName, errorLine.LineNumber );
					}
					else if (ret1 == TRUE)
						fprintf(pFile, "\t%s: %d line\r\n", errorLine.FileName, errorLine.LineNumber );
					else if (ret2 == TRUE)
						fprintf(pFile, "%s(): %d line(%#x)\r\n", pSymbol->Name, symbolLine.LineNumber, pSymbol->Address);
					else
						fprintf(pFile, "%s() (%#x)\r\n", pSymbol->Name, pSymbol->Address);
				}
				else
				{
					fprintf(pFile, "StackWalk failed: %d\r\n", ::GetLastError());
					break;
				}
			}
		}
	}

	SymCleanup(hProcess);

	fclose(pFile);

	return true;
}

void CCrashHandler::WriteCrashLog(LPCTSTR szFormat, ...)
{
	if (m_pLogger != NULL)
	{
		TCHAR szLine[2048];
		va_list args;
		va_start(args, szFormat);
		_vstprintf_s(szLine, szFormat, args);

		m_pLogger->Write(szLine);
	}
}