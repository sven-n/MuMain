// DebugAngel.h: interface for the DebugAngel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DEBUGUTIL_H__61A6CA46_25FA_4ECB_8B5C_B554831D00B1__INCLUDED_)
#define AFX_DEBUGUTIL_H__61A6CA46_25FA_4ECB_8B5C_B554831D00B1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


void DebugAngel_Write( char *lpszFileName, ...);
void DebugAngel_HexWrite( char *lpszFileName, void *pBuffer, int iSize);
//void DebugAngel_MessageBox( HRESULT hResult, ...);


#endif // !defined(AFX_DEBUGUTIL_H__61A6CA46_25FA_4ECB_8B5C_B554831D00B1__INCLUDED_)
