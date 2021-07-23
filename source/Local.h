// Local.h: interface for the Local
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOCAL_H__542800D0_DC18_4483_9D91_C8989BA698F4__INCLUDED_)
#define AFX_LOCAL_H__542800D0_DC18_4483_9D91_C8989BA698F4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


extern char *lpszFindHackFiles[NUM_LANGUAGE];
extern char *LanguageName[NUM_LANGUAGE];
extern char *lpszLocaleName[NUM_LANGUAGE];
extern DWORD g_dwCharSet[NUM_LANGUAGE];
extern int iLengthAuthorityCode[NUM_LANGUAGE];
extern char lpszServerIPAddresses[NUM_LANGUAGE][50];

#ifndef KJH_ADD_SERVER_LIST_SYSTEM			// #ifndef
BOOL IsNonPvpServer( int iServerHigh, int iServerLow);
#endif // KJH_ADD_SERVER_LIST_SYSTEM


#endif // !defined(AFX_LOCAL_H__542800D0_DC18_4483_9D91_C8989BA698F4__INCLUDED_)
