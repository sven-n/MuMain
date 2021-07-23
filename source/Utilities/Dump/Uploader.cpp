#include "stdafx.h"
#include "Uploader.h"

DWORD WINAPI InternetConnectThread(LPVOID vThreadParm)
{
	FTP_PARM* pThreadParm;
	// Initialize local pointer to void pointer passed to thread
	pThreadParm = (FTP_PARM*)vThreadParm;

	*(pThreadParm->pFtpConnection) = ::InternetConnect(
		*(pThreadParm->pInternetSession),
		pThreadParm->szUrl,
		pThreadParm->nPort,
		pThreadParm->szId,
		pThreadParm->szPassword,
		INTERNET_SERVICE_FTP,
		INTERNET_FLAG_PASSIVE,
		NULL);

	if (*(pThreadParm->pFtpConnection) == NULL)
		return 1;
	return 0;
}

CUploader::CUploader(LPCTSTR szUrl, int nPort, LPCTSTR szId, LPCTSTR szPassword, int nUploadCount)
: m_nPort(nPort)
{
	_stprintf_s(m_szUrl, BUFFER_SIZE, _T("%s"), szUrl);
	_stprintf_s(m_szId, BUFFER_SIZE, _T("%s"), szId);
	_stprintf_s(m_szPassword, BUFFER_SIZE, _T("%s"), szPassword);

	m_nUploadCount = (nUploadCount < 1) ? 1 : nUploadCount;
}

CUploader::~CUploader()
{
}

int CUploader::UploadFTP(LPCTSTR szLocalFile, LPCTSTR szServerFile)
{
	HINTERNET hInternetSession, hFtpConnection;

	// Connect
	hInternetSession = ::InternetOpen(
		NULL,
		INTERNET_OPEN_TYPE_PRECONFIG,
		NULL,
		NULL,
		0);

	if (!hInternetSession)
		return 1;

	// Create a worker thread 
	HANDLE hThread; 
	DWORD dwThreadID;
	DWORD dwExitCode = 0;
	FTP_PARM threadParm;
	threadParm.pInternetSession = &hInternetSession;
	threadParm.pFtpConnection = &hFtpConnection;
	threadParm.szUrl = m_szUrl;
	threadParm.nPort = m_nPort;
	threadParm.szId = m_szId;
	threadParm.szPassword = m_szPassword;

	hThread = CreateThread(
		NULL,			// Pointer to thread security attributes 
		0,				// Initial thread stack size, in bytes 
		InternetConnectThread,	// Pointer to thread function 
		&threadParm,	// The argument for the new thread
		0,				// Creation flags 
		&dwThreadID);	// Pointer to returned thread identifier 

	// Wait for the call to InternetConnect in worker function to complete
	if (WaitForSingleObject(hThread, TIME_OUT) == WAIT_TIMEOUT)
	{
		if (hInternetSession)
			InternetCloseHandle(hInternetSession);
		WaitForSingleObject(hThread, INFINITE);
		return 2;
	}

	// The state of the specified object (thread) is signaled
	if (!GetExitCodeThread(hThread, &dwExitCode))
	{
		if (hInternetSession)
			InternetCloseHandle(hInternetSession);
		return 2;
	}

	if (!hFtpConnection || dwExitCode)
	{
		if (hInternetSession)
			InternetCloseHandle(hInternetSession);
		return 2;
	}

	// Create folder
	LPCTSTR szTempStr = szServerFile;
	TCHAR szMakeDir[MAX_PATH];
	int nPos;

	while ((szTempStr = _tcsstr( szTempStr + 1, _T("/") )) != NULL)
	{
		nPos = static_cast< int >(szTempStr - szServerFile + 1);
		_tcsncpy_s (szMakeDir, szServerFile, nPos);
		szMakeDir[nPos] = 0;

		::FtpCreateDirectory(hFtpConnection, szMakeDir);
	}

	// Send file
	BOOL bSuccess = false;
	int nUploadCount = 0;

	while (!bSuccess)
	{
		bSuccess = ::FtpPutFile(
			hFtpConnection,
			szLocalFile,
			szServerFile,
			FTP_TRANSFER_TYPE_BINARY,
			NULL);

		if (++nUploadCount >= m_nUploadCount)
			break;
	}

	InternetCloseHandle(hFtpConnection);
	InternetCloseHandle(hInternetSession);

	if (!bSuccess)
		return GetLastError();
	return 0;
}

#pragma comment(lib, "wininet")