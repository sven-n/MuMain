#include "stdafx.h"
#include "Logger.h"
#include <stdio.h>

CLogger::CLogger()
: m_bOpen(false), m_pFile(NULL)
{
	::InitializeCriticalSection(&m_csLog);

	Open();
}

CLogger::~CLogger()
{
	Close();

	::DeleteCriticalSection(&m_csLog);
}

bool CLogger::Open()
{
	if (m_bOpen)
		return false;

	::EnterCriticalSection(&m_csLog);

	_tfopen_s(&m_pFile, _T("CrashLog.txt"), _T("a"));
	if (m_pFile != NULL)
		m_bOpen = true;

	int nTry = 1;
	TCHAR szLogFilename[256];
	while (!m_bOpen && ++nTry <= MAXIMUM_TRY)
	{
		_stprintf_s(szLogFilename, 256, _T("CrashLog_%d.txt"), nTry);
		_tfopen_s(&m_pFile, szLogFilename, _T("a"));
		if (m_pFile != NULL)
			m_bOpen = true;
	}

	::LeaveCriticalSection(&m_csLog);

	return true;
}

void CLogger::Close()
{
	fclose(m_pFile);
	m_pFile = NULL;

	m_bOpen = false;
}

bool CLogger::Write(LPCTSTR szLine)
{
	if (m_bOpen)
	{
		SYSTEMTIME t;
		::GetLocalTime(&t);
		_ftprintf(m_pFile, _T("%04d.%02d.%02d %02d:%02d:%02d:%02d\t%s\n"), t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond, t.wMilliseconds, szLine);
	}
	else
		return false;

	return true;
}

bool CLogger::WriteWithFormat(LPCTSTR szFormat, ...)
{
	if (m_bOpen)
	{
		TCHAR szLine[256];
		va_list args;
		va_start(args, szFormat);
		_vstprintf_s(szLine, szFormat, args);

		SYSTEMTIME t;
		::GetLocalTime(&t);
		_ftprintf(m_pFile, _T("%04d.%02d.%02d %02d:%02d:%02d:%02d\t%s\n"), t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond, t.wMilliseconds, szLine);
	}
	else
		return false;

	return true;
}