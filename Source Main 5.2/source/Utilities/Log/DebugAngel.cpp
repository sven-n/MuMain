// DebugAngel.cpp: implementation of the DebugAngel class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include <dxerr8.h>
#include "DebugAngel.h"


void WriteDebugInfoStr( char *lpszFileName, char *lpszToWrite)
{
	HANDLE hFile = CreateFile( lpszFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS,
								FILE_ATTRIBUTE_NORMAL, NULL);
	SetFilePointer( hFile, 0, NULL, FILE_END);
	DWORD dwNumber;
	WriteFile( hFile, lpszToWrite, strlen( lpszToWrite), &dwNumber, NULL);
	CloseHandle( hFile);
}

void DebugAngel_Write( char *lpszFileName, ...)
{
	char lpszBuffer[1024];
	va_list va;
	va_start( va, lpszFileName);
	char *lpszFormat = va_arg( va, char*);
	wvsprintf( lpszBuffer, lpszFormat, va);
	WriteDebugInfoStr( lpszFileName, lpszBuffer);
	va_end( va);
}

void DebugAngel_HexWrite( char *lpszFileName, void *pBuffer, int iSize)
{
	HANDLE hFile = CreateFile( lpszFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS,
								FILE_ATTRIBUTE_NORMAL, NULL);
	SetFilePointer( hFile, 0, NULL, FILE_END);
	DWORD dwNumber;

	BYTE *pbySeek = ( BYTE*)pBuffer;
	char lpszStr[1024];
	for ( int i = 0; i < iSize; i += 16)
	{
		lpszStr[0] = '\0';
		int iShow = min( iSize - i, 16);

		// Hex Ãâ·Â
		for ( int j = 0; j < iShow; j++, pbySeek++)
		{
			char lpszTemp[16];
			wsprintf( lpszTemp, "0x%02X ", *pbySeek);
			strcat( lpszStr, lpszTemp);
		}
		strcat( lpszStr, "\r\n");
		WriteFile( hFile, lpszStr, strlen( lpszStr), &dwNumber, NULL);
	}

	CloseHandle( hFile);
}