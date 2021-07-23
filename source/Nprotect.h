// Nprotect.h: interface for the CNprotect class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NPROTECT_H__C46E8B63_57C1_4E4B_B35A_63ABE5C29D67__INCLUDED_)
#define AFX_NPROTECT_H__C46E8B63_57C1_4E4B_B35A_63ABE5C29D67__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define HACK_TIMER	1000
#define WINDOWMINIMIZED_TIMER	1001


typedef BOOL(NPROTECTCHECKCRC)(char *);
extern NPROTECTCHECKCRC *g_pNprotectCheckCRC;


BOOL LaunchNprotect( void);
void KillProtect();
bool CheckTotalNprotect( void);
void CheckNpmonCrc( HWND hWnd);
BOOL FindNprotectWindow( void);
void CloseNprotect( void);


#endif // !defined(AFX_NPROTECT_H__C46E8B63_57C1_4E4B_B35A_63ABE5C29D67__INCLUDED_)
