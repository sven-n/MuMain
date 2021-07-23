#ifndef __ZZZSCENE_H__
#define __ZZZSCENE_H__

#include "zzzinfomation.h"

#ifndef KJH_ADD_SERVER_LIST_SYSTEM		// #ifndef
extern int ServerSelectHi;
extern SERVER_LIST_t ServerList[MAX_SERVER_HI];
extern WORD ServerNumber;
#endif // KJH_ADD_SERVER_LIST_SYSTEM

extern int MenuStateCurrent;
extern int MenuStateNext;
extern int  SceneFlag;
#ifndef CSK_FIX_FENRIR_RUN		// 정리할 때 지워야 하는 소스	
extern BOOL g_bFenrir_Run;
extern int	g_iFenrir_Run;
#endif //! CSK_FIX_FENRIR_RUN	// 정리할 때 지워야 하는 소스
extern int  MoveSceneFrame;
//extern bool EnableEdit;
extern int  ErrorMessage;
extern bool InitServerList;
extern bool InitLogIn;
extern bool InitLoading;
extern bool InitCharacterScene;
extern bool InitMainScene;
extern bool EnableMainRender;
extern char *szServerIpAddress;
extern unsigned short g_ServerPort;
extern int g_iLengthAuthorityCode;

extern void LogInScene(HDC hDC);
extern void LoadingScene(HDC hDC);
extern void Scene(HDC Hdc);
extern bool CheckSpecialText(char *Text);
extern bool CheckName();
void    StartGame(
#ifdef PJH_CHARACTER_RENAME
			   bool IsConnect = false
#endif //#ifdef PJH_CHARACTER_RENAME
				  );

BOOL	ShowCheckBox( int num, int index, int message=MESSAGE_TRADE_CHECK );

int	SeparateTextIntoLines( const char* lpszText, char *lpszSeparated, int iMaxLine, int iLineSize);

#ifndef KJH_ADD_SERVER_LIST_SYSTEM			// #ifndef
int		SearchServer(char *Name, bool extServer=false);
#endif // KJH_ADD_SERVER_LIST_SYSTEM
bool	GetTimeCheck(int DelayTime);
void	SetEffectVolumeLevel(int level);

// Ctrl + 마우스 휠 조작으로 뷰포트 레벨을 조절하여서 
// 카메라의 CameraViewFar, CameraDistanceTarget, CameraDistance를 조절한다.
// 현재 아무곳에서도 사용하고 있지 않는 함수
void    SetViewPortLevel ( int level );

// 생성할 수 있는 클래스의 수를 설정하는 함수
// ReceiveCharacterList() 서버로 부터 정보를 받아서 g_byMaxClass 변수에 설정
void    SetCreateMaxClass ( BYTE MaxClass );

// 생성할 수 있는 클래스의 수를 얻는 함수
BYTE    GetCreateMaxClass ( void );

#ifdef PSW_BUGFIX_IME
bool IsEnterPressed();
void SetEnterPressed( bool enterpressed );
#endif //PSW_BUGFIX_IME

#ifdef DO_PROCESS_DEBUGCAMERA
void ProcessDebugCamera();
#endif // DO_PROCESS_DEBUGCAMERA

#endif //__ZZZSCENE_H__
