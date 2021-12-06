// DebugAngel.h: interface for the DebugAngel class.
//////////////////////////////////////////////////////////////////////

#pragma once

void DebugAngel_Write( char *lpszFileName, ...);
void DebugAngel_HexWrite( char *lpszFileName, void *pBuffer, int iSize);
