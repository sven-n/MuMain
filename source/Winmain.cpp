///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN

#include <locale.h>
#include <zmouse.h>
#include "UIWindows.h"
#include "UIManager.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "ZzzOpenData.h"
#include "ZzzScene.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"
#include "zzzLodTerrain.h"
#include "DSPlaySound.h"
#include "wsclientinline.h"
#include "Resource.h"
#include <imm.h>
#include "zzzpath.h"
#include "Nprotect.h"
#include "./Utilities/Log/DebugAngel.h"
#include "Local.h"
#include "PersonalShopTitleImp.h"
#include "./Utilities/Log/ErrorReport.h"
#include "UIMapName.h"		// rozy
#include "CSMapInterface.h"
#include "./ExternalObject/leaf/ExceptionHandler.h"
#include "./Utilities/Dump/CrashReporter.h"
#ifdef PROTECT_SYSTEMKEY
	#include "ProtectSysKey.h"
#endif // PROTECT_SYSTEMKEY
#include "./Utilities/Log/muConsoleDebug.h"
#include "../ProtocolSend.h"

#ifdef BAN_USE_CMDLINE
	#include "./LauncherHelper/LauncherHelper.h"
#endif // BAN_USE_CMDLINE
#include "CBTMessageBox.h"
#include "./ExternalObject/leaf/regkey.h"

#include "CSChaosCastle.h"
#include "GMHellas.h"
#include <io.h>
#include "Input.h"
#include "./Time/Timer.h"
#include "UIMng.h"

#ifdef MOVIE_DIRECTSHOW
#include <dshow.h>
#include "MovieScene.h"
#endif // MOVIE_DIRECTSHOW
#include "GameCensorship.h"

#ifdef PSW_ADD_MAPSYSTEM
	#include "w_MapHeaders.h"
#endif // PSW_ADD_MAPSYSTEM

#ifdef LDK_ADD_NEW_PETPROCESS
	#include "w_PetProcess.h"
#endif //LDK_ADD_NEW_PETPROCESS

#ifdef CSK_MOD_PROTECT_AUTO_V1
	#include "ProtectAuto.h"
#endif // CSK_MOD_PROTECT_AUTO_V1

#include <ThemidaInclude.h>

#if defined LDK_ADD_SCALEFORM
#include "CGFxProcess.h"
#endif //LDK_ADD_SCALEFORM

#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
#include "MultiLanguage.h"
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
///////////////////////////////////////////
CUIMercenaryInputBox * g_pMercenaryInputBox = NULL;
CUITextInputBox * g_pSingleTextInputBox = NULL;
CUITextInputBox * g_pSinglePasswdInputBox = NULL;
int g_iChatInputType = 1;
extern BOOL g_bIMEBlock;

CChatRoomSocketList * g_pChatRoomSocketList = NULL;

#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
CMultiLanguage *pMultiLanguage = NULL;
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE

extern DWORD g_dwTopWindow;

#ifdef MOVIE_DIRECTSHOW
CMovieScene*	g_pMovieScene = NULL;
#endif // MOVIE_DIRECTSHOW

CUIManager* g_pUIManager = NULL;
CUIMapName* g_pUIMapName = NULL;		// rozy

int Time_Effect = 0;
bool ashies = false;
int weather = rand()%3;

HWND      g_hWnd  = NULL;
HINSTANCE g_hInst = NULL;
HDC       g_hDC   = NULL;
HGLRC     g_hRC   = NULL;
HFONT     g_hFont = NULL;
HFONT     g_hFontBold = NULL;
HFONT     g_hFontBig = NULL;
HFONT     g_hFixFont = NULL;

CTimer*		g_pTimer = NULL;	// performance counter.
bool      Destroy = false;
bool      ActiveIME = false;

BYTE*				RendomMemoryDump;
ITEM_ATTRIBUTE*		ItemAttRibuteMemoryDump;
CHARACTER*			CharacterMemoryDump;

int       RandomTable[100];

char TextMu[]       = "mu.exe";

CErrorReport g_ErrorReport;

BOOL g_bMinimizedEnabled = FALSE;
int g_iScreenSaverOldValue = 60*15;

// For Debuging - DebugCamera
#ifdef DO_PROCESS_DEBUGCAMERA
CDebugCameraManager *g_pDebugCameraManager = 0;
#endif // DO_PROCESS_DEBUGCAMERA

// For Debuging - Unfix FixedFrame
#if defined(LDS_FOR_DEVELOPMENT_TESTMODE) || defined(LDS_UNFIXED_FIXEDFRAME_FORDEBUG)
bool g_bUnfixedFixedFrame = false;
#endif // defined(LDS_FOR_DEVELOPMENT_TESTMODE) || defined(LDS_UNFIXED_FIXEDFRAME_FORDEBUG)

extern float g_fScreenRate_x;	// ※
extern float g_fScreenRate_y;

#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
BOOL g_bUseWindowMode = TRUE;
#endif

char Mp3FileName[256];

#pragma comment(lib, "wzAudio.lib")
#include <wzAudio.h>

void StopMp3(char *Name, BOOL bEnforce)
{
    if(!m_MusicOnOff && !bEnforce) return;

	if(Mp3FileName[0] != NULL)
	{
		if(strcmp(Name, Mp3FileName) == 0) {
			wzAudioStop();
			Mp3FileName[0] = NULL;
		}
	}
}

void PlayMp3(char *Name, BOOL bEnforce )
{
	if(Destroy) return;
    if(!m_MusicOnOff && !bEnforce) return;

	if(strcmp(Name,Mp3FileName) == 0) 
	{
		return;
	}
	else 
	{
		wzAudioPlay(Name, 1);
		strcpy(Mp3FileName,Name);
	}
}

bool IsEndMp3()
{
	if (100 == wzAudioGetStreamOffsetRange())
		return true;
	return false;
}

int GetMp3PlayPosition()
{
	return wzAudioGetStreamOffsetRange();
}

extern int  LogIn;
extern char LogInID[];

void CheckHack( void)
{
	gProtocolSend.SendCheckOnline();
}

#ifdef ATTACH_HACKSHIELD
int __stdcall HackShieldCallback(long lCode, long lParamSize, void* pParam)
{
	const char* szErrorBoxTitle = "HackShield Error";
	switch(lCode)
	{
		//Engine Callback
		case AHNHS_ENGINE_DETECT_GAME_HACK:
		case AHNHS_ENGINE_DETECT_GENERAL_HACK:
		{
			leaf::CBTMessageBox(g_hWnd, GlobalText[1450], szErrorBoxTitle, MB_ICONERROR, true);
			g_ErrorReport.Write("[HACKSHIELD] 다음 위치에서 해킹툴이 발견되어 프로그램을 종료시켰습니다.(%s)\r\n", (char*)pParam);

			if(g_hWnd) {
				SendMessage(g_hWnd, WM_DESTROY, 0, 0);
			}
			break;
		}

		//이미 후킹이 이루어진 상태 
		//일부 API가 이미 후킹되어 있는 상태
		//그러나 실제로는 이를 차단하고 있기 때문에 다른 후킹시도 프로그램으 동작하지 않습니다.
		//이 Callback은 단지 경보내지는 정보제공 차원에서 사용할 수 있습니다.
		case AHNHS_ACTAPC_DETECT_ALREADYHOOKED:
		{
			PACTAPCPARAM_DETECT_HOOKFUNCTION pHookFunction = (PACTAPCPARAM_DETECT_HOOKFUNCTION)pParam;
			
			g_ErrorReport.Write("[HACKSHIELD] Already Hooked(-szFunctionName : %s -szModuleName : %s)\r\n", 
				pHookFunction->szFunctionName, pHookFunction->szModuleName);

			break;
		}

		//Speed 관련
		case AHNHS_ACTAPC_DETECT_SPEEDHACK:
		case AHNHS_ACTAPC_DETECT_SPEEDHACK_APP:
		{
			leaf::CBTMessageBox(g_hWnd, GlobalText[1451], szErrorBoxTitle, MB_ICONERROR, true);
			g_ErrorReport.Write("[HACKSHIELD] PC에서 스피드핵으로 의심되는 증상이 발견되었습니다.(Code : %d)\r\n", lCode);
			if(g_hWnd) {
				SendMessage(g_hWnd, WM_DESTROY, 0, 0);
			}
			break;
		}

		//디버깅 방지 
		case AHNHS_ACTAPC_DETECT_KDTRACE:	
		case AHNHS_ACTAPC_DETECT_KDTRACE_CHANGED:
		{
			leaf::CBTMessageBox(g_hWnd, GlobalText[1452], szErrorBoxTitle, MB_ICONERROR, true);
			g_ErrorReport.Write("[HACKSHIELD] 디버깅시도가 있었습니다.(Code : %d)\r\n", lCode);
			if(g_hWnd) {
				SendMessage(g_hWnd, WM_DESTROY, 0, 0);
			}
			break;
		}

		//해킹 방지 기능 이상 
		case AHNHS_ACTAPC_DETECT_AUTOMOUSE:
		{
			leaf::CBTMessageBox(g_hWnd, GlobalText[1453], szErrorBoxTitle, MB_ICONERROR, true);
			g_ErrorReport.Write("[HACKSHIELD] 해킹방지 기능이상.(Code : %d)\r\n", lCode);
			if(g_hWnd) {
				SendMessage(g_hWnd, WM_DESTROY, 0, 0);
			}
			break;
		}
		case AHNHS_ACTAPC_DETECT_DRIVERFAILED:
		{
			leaf::CBTMessageBox(g_hWnd, GlobalText[1454], szErrorBoxTitle, MB_ICONERROR, true);
			g_ErrorReport.Write("[HACKSHIELD] 해킹방지 기능이상.(Code : %d)\r\n", lCode);
			if(g_hWnd) {
				SendMessage(g_hWnd, WM_DESTROY, 0, 0);
			}
			break;
		}
		case AHNHS_ACTAPC_DETECT_HOOKFUNCTION:
		{
			leaf::CBTMessageBox(g_hWnd, GlobalText[1455], szErrorBoxTitle, MB_ICONERROR, true);
			g_ErrorReport.Write("[HACKSHIELD] 해킹방지 기능이상.(Code : %d)\r\n", lCode);
			if(g_hWnd) {
				SendMessage(g_hWnd, WM_DESTROY, 0, 0);
			}
			break;
		}
		case AHNHS_ACTAPC_DETECT_MESSAGEHOOK:
		{
			leaf::CBTMessageBox(NULL, GlobalText[1456], szErrorBoxTitle, MB_ICONERROR, true);
			g_ErrorReport.Write("[HACKSHIELD] 해킹방지 기능이상.(Code : 0x%d)\r\n", lCode);
			if(g_hWnd) {
				SendMessage(g_hWnd, WM_DESTROY, 0, 0);
			}
			break;
		}
	}
	return 1;
}
bool AttachHackShield(int nGameCode, const char* szLicenseKey) 
{
	char* szFileName;
	char szEhSvcFileFullPath[256] = {0,};
	GetFullPathName("HackShield/EhSvc.dll", 256, szEhSvcFileFullPath, &szFileName);

	int nRet;
	nRet = _AhnHS_Initialize(szEhSvcFileFullPath, HackShieldCallback, 
		nGameCode, szLicenseKey, 
		AHNHS_CHKOPT_SPEEDHACK | AHNHS_CHKOPT_AUTOMOUSE |
#ifndef _DEBUG
		AHNHS_CHKOPT_READWRITEPROCESSMEMORY | AHNHS_CHKOPT_KDTARCER | AHNHS_CHKOPT_OPENPROCESS |
#endif // _DEBUG
		AHNHS_CHKOPT_MESSAGEHOOK | AHNHS_CHKOPT_PROCESSSCAN | AHNHS_DONOT_TERMINATE_PROCESS |
		AHNHS_ALLOW_SVCHOST_OPENPROCESS | AHNHS_USE_LOG_FILE/* | AHNHS_CHECK_UPDATE_STATE*/);

	assert(nRet != HS_ERR_INVALID_PARAM);
	assert(nRet != HS_ERR_INVALID_LICENSE);
	assert(nRet != HS_ERR_ALREADY_INITIALIZED);

	const char* szErrorBoxTitle = "HackShield Initailize Error";
	if(nRet != HS_ERR_OK) 
	{
		switch(nRet)
		{
			case HS_ERR_ANOTHER_SERVICE_RUNNING:
			{
				leaf::CBTMessageBox(g_hWnd, "[HACKSHIELD] HS_ERR_ANOTHER_SERVICE_RUNNING", szErrorBoxTitle, MB_ICONERROR, true);
				g_ErrorReport.Write("[HACKSHIELD] 다른 게임이 실행중입니다.\n프로그램을 종료합니다.", szErrorBoxTitle, MB_ICONERROR);
				break;
			}
			case HS_ERR_INVALID_FILES:
			{
				leaf::CBTMessageBox(g_hWnd, "[HACKSHIELD] HS_ERR_INVALID_FILES", szErrorBoxTitle, MB_ICONERROR, true);
				g_ErrorReport.Write("[HACKSHIELD] 잘못된 파일 설치되었습니다.\n프로그램을 재설치하시기 바랍니다.", szErrorBoxTitle, MB_ICONERROR);
				break;
			}
			case HS_ERR_SOFTICE_DETECT:
			{
				leaf::CBTMessageBox(g_hWnd, "[HACKSHIELD] HS_ERR_SOFTICE_DETECT", szErrorBoxTitle, MB_ICONERROR, true);
				g_ErrorReport.Write("[HACKSHIELD] 컴퓨터에서 SoftICE 실행이 감지되었습니다.\nSoftICE 실행을 중지시킨 뒤에 다시 실행시켜주시기바랍니다.", szErrorBoxTitle, MB_ICONERROR);
				break;
			}
			case HS_ERR_INIT_DRV_FAILED:
			{
				leaf::CBTMessageBox(g_hWnd, "[HACKSHIELD] HS_ERR_INIT_DRV_FAILED", szErrorBoxTitle, MB_ICONERROR, true);
				g_ErrorReport.Write("[HACKSHIELD] 해킹방지 기능에 문제가 발생하였습니다.(Error Code = %x)\n프로그램을 종료합니다.", nRet);
				break;
			}
			case HS_ERR_NEED_UPDATE:
			{
				leaf::CBTMessageBox(g_hWnd, "[HACKSHIELD] HS_ERR_NEED_UPDATE", szErrorBoxTitle, MB_ICONERROR, true);
				g_ErrorReport.Write("[HACKSHIELD] 업데이트파일이 정상적이지 않습니다.(Error Code = %x)\n프로그램을 종료합니다.", nRet);
				break;
			}
		}
		return false;
	}

	//시작 함수 호출 
	nRet = _AhnHS_StartService();
	assert(nRet != HS_ERR_NOT_INITIALIZED);
	assert(nRet != HS_ERR_ALREADY_SERVICE_RUNNING);

	if(nRet != HS_ERR_OK)
	{
		// nRet ==  HS_ERR_START_ENGINE_FAILED || nRet == HS_ERR_DRV_FILE_CREATE_FAILED
		// || nRet == HS_ERR_REG_DRV_FILE_FAILED || nRet == HS_ERR_START_DRV_FAILED
		g_ErrorReport.Write("해킹방지 기능에 문제가 발생하였습니다.(Error Code = 0x%x)\n프로그램을 종료합니다.", nRet);
		leaf::CBTMessageBox(g_hWnd, "[HACKSHIELD] Start Error", szErrorBoxTitle, MB_ICONERROR, true);
		return false;
	}

	return true;
}
void DetachHackShield()
{
	//서비스 종료 함수 호출 
	_AhnHS_StopService();
	//완료 함수 호출 
	_AhnHS_Uninitialize();
}
#endif // ATTACH_HACKSHIELD

///////////////////////////////////////////////////////////////////////////////
// Window 관련함수
///////////////////////////////////////////////////////////////////////////////

//윈도우 죽이는 함수
GLvoid KillGLWindow(GLvoid)								
{
	if (g_hRC)											// 랜더링 컨텍스트가 있다면...
	{
		if (!wglMakeCurrent(NULL,NULL))					// DC와 RC 컨텍스트를 제거할 수 있는가?
		{
			g_ErrorReport.Write( "GL - Release Of DC And RC Failed\r\n");
			MessageBox(NULL,"Release Of DC And RC Failed.","Error",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(g_hRC))						// 랜더링 컨텍스트 제거
		{
			g_ErrorReport.Write( "GL - Release Rendering Context Failed\r\n");
			MessageBox(NULL,"Release Rendering Context Failed.","Error",MB_OK | MB_ICONINFORMATION);
		}

		g_hRC=NULL;										// RC <- NULL 대입
	}

	if (g_hDC && !ReleaseDC(g_hWnd,g_hDC))					// DC 제거
	{
		g_ErrorReport.Write( "GL - OpenGL Release Error\r\n");
		MessageBox(NULL,"OpenGL Release Error.","Error",MB_OK | MB_ICONINFORMATION);
		g_hDC=NULL;										// DC <- NULL 대입
	}

	/*if (g_hWnd && !DestroyWindow(g_hWnd))					// 윈도우즈를 파괴한다.
	{
		MessageBox(NULL,"OpenGL Destroy Error. 그래픽카드 최신 드라이버를 설치하십시요.","ERROR",MB_OK | MB_ICONINFORMATION);
		g_hWnd=NULL;										// hWnd <- NULL 대입
	}*/

	/*if (!UnregisterClass("OpenGL",g_hInst))			// 윈도우 클래스 제거
	{
		MessageBox(NULL,"Could Not Unregister Class.","ERROR",MB_OK | MB_ICONINFORMATION);
		g_hInst=NULL;									// hInstance <- NULL 대입
	}*/
#if (defined WINDOWMODE)
	if (g_bUseWindowMode == FALSE)
	{
		ChangeDisplaySettings(NULL,0);					// 데스크탑 설정으로 복귀
		ShowCursor(TRUE);								// 마우스 포인트를 보이도록
	}
#else
#ifdef ENABLE_FULLSCREEN
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
	if (g_bUseWindowMode == FALSE)
#endif	// USER_WINDOW_MODE
	{
		ChangeDisplaySettings(NULL,0);					// 데스크탑 설정으로 복귀
		ShowCursor(TRUE);								// 마우스 포인트를 보이도록
	}
#endif //ENABLE_FULLSCREEN
#endif	//WINDOWMODE(#else)
}


BOOL GetFileNameOfFilePath( char *lpszFile, char *lpszPath)
{
	int iFind = ( int)'\\';
	char *lpFound = lpszPath;
	char *lpOld = lpFound;
	while ( lpFound)
	{
		lpOld = lpFound;
		lpFound = strchr( lpFound + 1, iFind);
	}

	// 이름 복사
	if ( strchr( lpszPath, iFind))
	{
		strcpy( lpszFile, lpOld + 1);
	}
	else
	{
		strcpy( lpszFile, lpOld);
	}
	// 뒤에 옵션 붙은 것 없애기
	BOOL bCheck = TRUE;
	for ( char *lpTemp = lpszFile; bCheck; ++lpTemp)
	{
		switch ( *lpTemp)
		{
		case '\"':
		case '\\':
		case '/':
		case ' ':
			*lpTemp = '\0';
		case '\0':
			bCheck = FALSE;
			break;
		}
	}

	return ( TRUE);
}

HANDLE g_hMainExe = INVALID_HANDLE_VALUE;

BOOL OpenMainExe( void)
{
#ifdef _DEBUG
	return ( TRUE);
#endif
	char lpszFile[MAX_PATH];
	char *lpszCommandLine = GetCommandLine();
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	GetFileNameOfFilePath( lpszFile, lpszCommandLine);
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	BOOL bResult = GetFileNameOfFilePath( lpszFile, lpszCommandLine);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING


	// 열어놔서 고치지 못하게 한다.
	g_hMainExe = CreateFile( ( char*)lpszFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL, 0);
	
	return ( INVALID_HANDLE_VALUE != g_hMainExe);
}

void CloseMainExe( void)
{
	CloseHandle( g_hMainExe);
}

WORD DecryptCheckSumKey( WORD wSource)
{
	WORD wAcc = wSource ^ 0xB479;
	return ( ( wAcc >> 10) << 4) | ( wAcc & 0xF);
}

DWORD GenerateCheckSum( BYTE *pbyBuffer, DWORD dwSize, WORD wKey)
{
	DWORD dwKey = ( DWORD)wKey;
	DWORD dwResult = dwKey << 9;
	for ( DWORD dwChecked = 0; dwChecked <= dwSize - 4; dwChecked += 4)
	{
		DWORD dwTemp;
		memcpy( &dwTemp, pbyBuffer + dwChecked, sizeof ( DWORD));

		// 1. 단계
		switch ( ( dwChecked / 4 + wKey) % 3)
		{
		case 0:
			dwResult ^= dwTemp;
			break;
		case 1:
			dwResult += dwTemp;
			break;
		case 2:
			dwResult <<= ( dwTemp % 11);
			dwResult ^= dwTemp;
			break;
		}

		// 2. 단계
		if ( 0 == ( dwChecked % 4))
		{
			dwResult ^= ( ( dwKey + dwResult) >> ( ( dwChecked / 4) % 16 + 3));
		}
	}

	return ( dwResult);
}

DWORD GetCheckSum( WORD wKey)
{
	wKey = DecryptCheckSumKey( wKey);

#ifdef KJH_MOD_RESOURCE_GUARD

	char lpszFile[MAX_PATH];

#ifdef _TEST_SERVER
	strcpy( lpszFile, "data\\local\\Gameguardtest.csr");
#else  // _TEST_SERVER
	strcpy( lpszFile, "data\\local\\Gameguard.csr");
#endif // _TEST_SERVER

	HANDLE hFile = CreateFile( ( char*)lpszFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if ( INVALID_HANDLE_VALUE == hFile)
	{
		return ( 0);
	}
	
	DWORD dwSize = GetFileSize( hFile, NULL);
	BYTE *pbyBuffer = new BYTE [dwSize];
	DWORD dwNumber;
	ReadFile( hFile, pbyBuffer, dwSize, &dwNumber, 0);
	CloseHandle( hFile);
	
	// 테이블 생성
	DWORD dwCheckSum = GenerateCheckSum(pbyBuffer, dwSize, wKey);
	delete [] pbyBuffer;
	
	return (dwCheckSum);
#else // KJH_MOD_RESOURCE_GUARD

	// 파일 열기
	char lpszFile[MAX_PATH];
#ifdef _DEBUG
	strcpy( lpszFile, "main.exe");
#else // _DEBUG
	char *lpszCommandLine = GetCommandLine();
	BOOL bResult = GetFileNameOfFilePath( lpszFile, lpszCommandLine);
#endif // _DEBUG

	HANDLE hFile;
#ifdef NDEBUG
	FAKE_CODE(Pos_WritableCheck);
Pos_WritableCheck:
	// 쓰기 가능여부 체크 (main.exe 가 실행중이면 쓰기 가능하지 않다.)
	hFile = CreateFile( ( char*)lpszFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if ( INVALID_HANDLE_VALUE != hFile)
	{
		// 해킹시도
		SendHackingChecked( 0x03, 0);
		return ( 0);
	}
#endif // NDEBUG

	hFile = CreateFile( ( char*)lpszFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if ( INVALID_HANDLE_VALUE == hFile)
	{
		return ( 0);
	}

	DWORD dwSize = GetFileSize( hFile, NULL);
	BYTE *pbyBuffer = new BYTE [dwSize];
	DWORD dwNumber;
	ReadFile( hFile, pbyBuffer, dwSize, &dwNumber, 0);
	CloseHandle( hFile);

	// 테이블 생성
	DWORD dwCheckSum = GenerateCheckSum( pbyBuffer, dwSize, wKey);
	delete [] pbyBuffer;

	return ( dwCheckSum);
#ifdef USE_SELFCHECKCODE
	END_OF_FUNCTION( Pos_SelfCheck01);
Pos_SelfCheck01:
	;
#endif
#endif // KJH_MOD_RESOURCE_GUARD
}


BOOL GetFileVersion( char *lpszFileName, WORD *pwVersion)
{
	DWORD dwHandle;
	DWORD dwLen = GetFileVersionInfoSize( lpszFileName, &dwHandle);
	if ( dwLen <= 0)
	{
		return ( FALSE);
	}

	BYTE *pbyData = new BYTE [dwLen];
	if ( !GetFileVersionInfo( lpszFileName, dwHandle, dwLen, pbyData))
	{
		delete [] pbyData;
		return ( FALSE);
	}

	VS_FIXEDFILEINFO *pffi;
	UINT uLen;
	if ( !VerQueryValue( pbyData, "\\", ( LPVOID*)&pffi, &uLen))
	{
		delete [] pbyData;
		return ( FALSE);
	}

	pwVersion[0] = HIWORD( pffi->dwFileVersionMS);
	pwVersion[1] = LOWORD( pffi->dwFileVersionMS);
	pwVersion[2] = HIWORD( pffi->dwFileVersionLS);
	pwVersion[3] = LOWORD( pffi->dwFileVersionLS);

	delete [] pbyData;
	return ( TRUE);
}

extern PATH     *path;

void DestroyWindow()
{
	//. save volume level
	leaf::CRegKey regkey;
#ifdef PBG_ADD_LAUNCHER_BLUE
	regkey.SetKey(leaf::CRegKey::_HKEY_CURRENT_USER, "SOFTWARE\\Webzen\\Mu_Blue\\Config");
#else //PBG_ADD_LAUNCHER_BLUE
	regkey.SetKey(leaf::CRegKey::_HKEY_CURRENT_USER, "SOFTWARE\\Webzen\\Mu\\Config");
#endif //PBG_ADD_LAUNCHER_BLUE
	regkey.WriteDword("VolumeLevel", g_pOption->GetVolumeLevel());

	CUIMng::Instance().Release();

#ifdef LDK_ADD_SCALEFORM
	if(GFxProcess::GetInstancePtr()->GetUISelect() == 1)
	{
		GFxProcess::GetInstancePtr()->GFxDestroy();
		GFxProcess* temp = GFxProcess::GetInstancePtr();
 		delete temp;
 		temp = NULL;
	}
#endif //LDK_ADD_SCALEFORM

#ifdef MOVIE_DIRECTSHOW
	if(g_pMovieScene)
	{
		g_pMovieScene->Destroy();
	}
#endif // MOVIE_DIRECTSHOW

	//. release font handle
	if(g_hFont)
		DeleteObject((HGDIOBJ)g_hFont);

	if(g_hFontBold)
		DeleteObject((HGDIOBJ)g_hFontBold);

	if(g_hFontBig)
		DeleteObject((HGDIOBJ)g_hFontBig);

	if(g_hFixFont)
		::DeleteObject((HGDIOBJ)g_hFixFont);
#ifdef PBG_ADD_INGAMESHOP_FONT
	if(g_hInGameShopFont)
		::DeleteObject((HGDIOBJ)g_hInGameShopFont);
#endif //PBG_ADD_INGAMESHOP_FONT

#ifdef YDG_MOD_PROTECT_AUTO_V4_R3
	SEASON3B::CMoveCommandWindowEncrypt::DeleteKey();
#endif	// YDG_MOD_PROTECT_AUTO_V4_R3
#ifdef CSK_MOD_PROTECT_AUTO_V1
	CProtectAuto::Destroy();
#endif // CSK_MOD_PROTECT_AUTO_V1
	
	ReleaseCharacters();

    if ( path!=NULL )
    {
	    delete path;
    }
	SAFE_DELETE(GateAttribute);

	for ( int i = 0; i < MAX_SKILLS; ++i)
	{
	}
	SAFE_DELETE(SkillAttribute);

	SAFE_DELETE(CharacterMachine);

    DeleteWaterTerrain ();

#ifdef MOVIE_DIRECTSHOW
	if(SceneFlag != MOVIE_SCENE)
#endif // MOVIE_DIRECTSHOW
	{
		gMapManager.DeleteObjects();

		// Object.
		for(int i=MODEL_LOGO;i<MAX_MODELS;i++)
		{
			Models[i].Release();
		}

		// Bitmap
		Bitmaps.UnloadAllImages();
	}

	SAFE_DELETE_ARRAY( CharacterMemoryDump );
	SAFE_DELETE_ARRAY( ItemAttRibuteMemoryDump );
	SAFE_DELETE_ARRAY( RendomMemoryDump );

#ifdef LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
	SAFE_DELETE_ARRAY( ModelsDump );
#endif // LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
	
#ifdef DYNAMIC_FRUSTRUM
	DeleteAllFrustrum();
#endif //DYNAMIC_FRUSTRUM

///////////////////////////////////////////
	SAFE_DELETE(g_pMercenaryInputBox);
	SAFE_DELETE(g_pSingleTextInputBox);
	SAFE_DELETE(g_pSinglePasswdInputBox);

	SAFE_DELETE(g_pChatRoomSocketList);
	SAFE_DELETE(g_pUIMapName);	// rozy
	SAFE_DELETE( g_pTimer );
	SAFE_DELETE(g_pUIManager);
	 
#ifdef MOVIE_DIRECTSHOW
	SAFE_DELETE(g_pMovieScene);
#endif // MOVIE_DIRECTSHOW

#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	SAFE_DELETE(pMultiLanguage);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	BoostRest( g_BuffSystem );

#ifdef NEW_USER_INTERFACE_SHELL
	BoostRest( g_shell );
#endif //NEW_USER_INTERFACE_SHELL
	
#ifdef PSW_ADD_MAPSYSTEM
	BoostRest( g_MapProcess );
#endif //PSW_ADD_MAPSYSTEM

#ifdef LDK_ADD_NEW_PETPROCESS
	BoostRest( g_petProcess );
#endif //LDK_ADD_NEW_PETPROCESS

	g_ErrorReport.Write( "Destroy" );
	 
	HWND shWnd = FindWindow(NULL, "MuPlayer");
	if(shWnd)
		SendMessage(shWnd, WM_DESTROY, 0, 0);
#ifdef ATTACH_HACKSHIELD
	DetachHackShield();
#endif // ATTACH_HACKSHIELD
	
#ifdef NDEBUG
#ifndef FOR_WORK
	int nOldVal;
	SystemParametersInfo(SPI_SETSCREENSAVETIMEOUT, g_iScreenSaverOldValue, NULL, 0);
    SystemParametersInfo(SPI_SCREENSAVERRUNNING, 0, &nOldVal, 0);
#endif
#endif
}
void DestroySound()
{
	for(int i=0;i<MAX_BUFFER;i++)
		ReleaseBuffer(i);

	FreeDirectSound();
	wzAudioDestroy();
}

int g_iInactiveTime = 0;
int g_iNoMouseTime = 0;
int g_iInactiveWarning = 0;
bool g_bWndActive = false;
bool HangulDelete = false;
int Hangul = 0;
bool g_bEnterPressed = false;

int g_iMousePopPosition_x = 0;
int g_iMousePopPosition_y = 0;

extern int TimeRemain;
extern bool EnableFastInput;
void MainScene(HDC hDC);

LONG FAR PASCAL WndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch (msg)
    {
#ifdef KWAK_FIX_ALT_KEYDOWN_MENU_BLOCK
	case WM_SYSKEYDOWN:
		{
			return 0;
		}
		break;
#endif // KWAK_FIX_ALT_KEYDOWN_MENU_BLOCK

#if defined PROTECT_SYSTEMKEY && defined NDEBUG
#ifndef FOR_WORK
	case WM_SYSCOMMAND:
		{
			if(wParam == SC_KEYMENU || wParam == SC_SCREENSAVE)
			{
				return 0;
			}
		}
		break;
#endif // !FOR_WORK
#endif // PROTECT_SYSTEMKEY && NDEBUG
    case WM_ACTIVATE:
		if(LOWORD(wParam) == WA_INACTIVE)
		{
			FAKE_CODE( Pos_ActiveFalse);
Pos_ActiveFalse:
#ifdef ACTIVE_FOCUS_OUT
			if (g_bUseWindowMode == FALSE)
#endif	// ACTIVE_FOCUS_OUT
				g_bWndActive = false;
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
			if (g_bUseWindowMode == TRUE)
			{
				MouseLButton = false;
				MouseLButtonPop = false;
				//MouseLButtonPush = false;
				MouseRButton = false;
				MouseRButtonPop = false;
				MouseRButtonPush = false;
				MouseLButtonDBClick = false;
				MouseMButton = false;
				MouseMButtonPop = false;
				MouseMButtonPush = false;
				MouseWheel = 0;
			}
#ifdef ASG_FIX_ACTIVATE_APP_INPUT
			g_pNewKeyInput->Init();
#endif	// ASG_FIX_ACTIVATE_APP_INPUT
#endif
		}
		else
		{
			FAKE_CODE( Pos_ActiveTrue);
Pos_ActiveTrue:
			g_bWndActive = true;
		}
		break;
	case WM_TIMER :
		//MessageBox(NULL,GlobalText[16],"Error",MB_OK);
		switch( wParam )
		{
		case HACK_TIMER:
			// PKD_ADD_BINARY_PROTECTION
			VM_START
			CheckHack();
			VM_END
			break;
		case WINDOWMINIMIZED_TIMER:	// 윈팅 방지 코드
			PostMessage(g_hWnd, WM_CLOSE, 0, 0);
			break;
		case CHATCONNECT_TIMER:
			g_pFriendMenu->SendChatRoomConnectCheck();
			break;
		case SLIDEHELP_TIMER:
			if(g_bWndActive)
			{
				if(g_pSlideHelpMgr)
					g_pSlideHelpMgr->CreateSlideText();
			}
			break;
#ifdef TEENAGER_REGULATION
		case WARNING_TIMER:
			{
				char dummy_chr[300];
				Time_Regulation++;
				if(Time_Regulation > 3)
					wsprintf(dummy_chr,GlobalText[2035],Time_Regulation);
				else
					wsprintf(dummy_chr,GlobalText[2036],Time_Regulation);
		
				g_pChatListBox->AddText("", dummy_chr, SEASON3B::TYPE_UNION_MESSAGE);	
				
				if(m_pc_wanted == true)
				{
					wsprintf(dummy_chr,GlobalText[2227]);
					g_pChatListBox->AddText("", dummy_chr, SEASON3B::TYPE_UNION_MESSAGE);			
				}

				g_GameCensorship->SetVisible(true);
			}
			break;
#endif
		}
		break;
	case WM_USER_MEMORYHACK:
		//SetTimer( g_hWnd, WINDOWMINIMIZED_TIMER, 1*1000, NULL);
		FAKE_CODE( Pos_UserMemoryHack);
Pos_UserMemoryHack:
		KillGLWindow();
		break;
	case WM_NPROTECT_EXIT_TWO:
		SendHackingChecked( 0x04, 0);
		SetTimer( g_hWnd, WINDOWMINIMIZED_TIMER, 1*1000, NULL);
		MessageBox(NULL,GlobalText[16],"Error",MB_OK);
		break;
	case WM_ASYNCSELECTMSG :
		switch( WSAGETSELECTEVENT( lParam ) )
		{
		case FD_CONNECT : // 서버에 접속 되었다.
			break;
		case FD_READ :
			SocketClient.nRecv();
			break;
		case FD_WRITE :
			SocketClient.FDWriteSend();
			break;
		case FD_CLOSE : // 서버와의 접속이 종료되었다.
			// 3  "서버와의 연결이 끊어졌습니다."
			g_pChatListBox->AddText("", GlobalText[3], SEASON3B::TYPE_SYSTEM_MESSAGE);
			g_ErrorReport.Write("서버와의 접속종료.\r\n");
#ifdef CONSOLE_DEBUG
			switch(WSAGETSELECTERROR(lParam))
			{
			case WSAECONNRESET:
				g_ConsoleDebug->Write(MCD_ERROR, "The connection was reset by the remote side.");
				g_ErrorReport.Write("The connection was reset by the remote side.\r\n");
				g_ErrorReport.WriteCurrentTime();
				break;
			case WSAECONNABORTED:
				g_ConsoleDebug->Write(MCD_ERROR, "The connection was terminated due to a time-out or other failure.");
				g_ErrorReport.Write("The connection was terminated due to a time-out or other failure.\r\n");
				g_ErrorReport.WriteCurrentTime();
				break;
			}
#endif // CONSOLE_DEBUG
			SocketClient.Close();
			gProtocolSend.DisconnectServer();
			CUIMng::Instance().PopUpMsgWin(MESSAGE_SERVER_LOST);
			break;
		}
		break;
	case WM_CTLCOLOREDIT:
		SetBkColor((HDC)wParam, RGB(0, 0, 0));
		SetTextColor((HDC)wParam, RGB(255, 255, 255));
		return (LRESULT)GetStockObject(BLACK_BRUSH);
		break;
	case WM_ERASEBKGND:
		return TRUE;
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps ;
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
			BeginPaint(hwnd, &ps);
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
			HDC hDC = BeginPaint(hwnd, &ps);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
			EndPaint(hwnd, &ps) ;
		}
		return 0;
		break;
	case WM_DESTROY:
		{
			Destroy = true;
			SocketClient.Close();
			gProtocolSend.DisconnectServer();
			DestroySound();
			//DestroyWindow();
			KillGLWindow();	
			CloseMainExe();
			PostQuitMessage(0);
		}
		break;
    case WM_SETCURSOR:
        ShowCursor(false);
		break;
#if (defined WINDOWMODE)
	case WM_SIZE:
		if ( SIZE_MINIMIZED == wParam && g_bUseWindowMode == FALSE )
		{
			if ( !( g_bMinimizedEnabled))
			{
				SendHackingChecked( 0x05, 0);
				FAKE_CODE( Pos_Wm_Move_MinimizedNotEnabled);
Pos_Wm_Move_MinimizedNotEnabled:
				DWORD dwMess[SIZE_ENCRYPTION_KEY];
				for ( int i = 0; i < SIZE_ENCRYPTION_KEY; ++i)
				{
					dwMess[i] = GetTickCount();
				}
				g_SimpleModulusCS.LoadKeyFromBuffer( ( BYTE*)dwMess, FALSE, FALSE, FALSE, TRUE);
			}
		}
		break;
#else
#ifdef NDEBUG
#ifndef FOR_WORK
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
	case WM_SIZE:
		if ( SIZE_MINIMIZED == wParam
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
			&& g_bUseWindowMode == FALSE
#endif
			)
		{	// 외부에서 윈도우를 최소화 시켰다.
			if ( !( g_bMinimizedEnabled))
			{
				SendHackingChecked( 0x05, 0);
				FAKE_CODE( Pos_Wm_Move_MinimizedNotEnabled);
Pos_Wm_Move_MinimizedNotEnabled:
				DWORD dwMess[SIZE_ENCRYPTION_KEY];
				for ( int i = 0; i < SIZE_ENCRYPTION_KEY; ++i)
				{
					dwMess[i] = GetTickCount();
				}
				g_SimpleModulusCS.LoadKeyFromBuffer( ( BYTE*)dwMess, FALSE, FALSE, FALSE, TRUE);
			}
		}
		break;
#endif
#endif
#endif
#endif	//WINDOWMODE(#else)
//-----------------------------
	default:
		if (msg >= WM_CHATROOMMSG_BEGIN && msg < WM_CHATROOMMSG_END)
			g_pChatRoomSocketList->ProcessSocketMessage(msg - WM_CHATROOMMSG_BEGIN, WSAGETSELECTEVENT(lParam));
		break;
	}

#ifdef USE_SELFCHECKCODE
	if ( TimeRemain >= 40)
	{
		SendCrcOfFunction( 4, 9, MainScene, 0x173C);
	}
#endif
	
	// 윈도우가 활성화 상태가 아니면 메세지 리턴시켜 버린다.
// 	if(!g_bWndActive)
//         return DefWindowProc(hwnd,msg,wParam,lParam);

	MouseLButtonDBClick = false;
	if (MouseLButtonPop == true && (g_iMousePopPosition_x != MouseX || g_iMousePopPosition_y != MouseY)) 
		MouseLButtonPop = false;
    switch (msg)
	{
	case WM_MOUSEMOVE:
		{
			MouseX = (float)LOWORD(lParam) / g_fScreenRate_x;
			MouseY = (float)HIWORD(lParam) / g_fScreenRate_y;
			if(MouseX < 0) 
				MouseX = 0;
			if(MouseX > 640) 
				MouseX = 640;
			if(MouseY < 0) 
				MouseY = 0;
			if(MouseY > 480) 
				MouseY = 480;
		}
		break;

#ifdef LDK_ADD_SCALEFORM
	}
	// Pass events to GFx
	bool pbNoFurtherProcessing = false;
	if(GFxProcess::GetInstancePtr()->GetUISelect() == 1)
	{
		GFxProcess::GetInstancePtr()->GFxProcessEvent(hwnd, msg, wParam, lParam, &pbNoFurtherProcessing);
	}

	if( pbNoFurtherProcessing )
		return 0;

	switch(msg)
	{
#endif //LDK_ADD_SCALEFORM

	case WM_LBUTTONDOWN:
		FAKE_CODE( Pos_LButtonDown);
Pos_LButtonDown:
		g_iNoMouseTime = 0;
		MouseLButtonPop = false;
		if(!MouseLButton) 
			MouseLButtonPush = true;
		MouseLButton = true;
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
		SetCapture(g_hWnd);
#endif
		break;
	case WM_LBUTTONUP:
		FAKE_CODE( Pos_LButtonUp);
Pos_LButtonUp:
		g_iNoMouseTime = 0;
		MouseLButtonPush = false;
		//if(MouseLButton) MouseLButtonPop = true;
		MouseLButtonPop = true;
		MouseLButton = false;
		g_iMousePopPosition_x = MouseX;
		g_iMousePopPosition_y = MouseY;
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
		ReleaseCapture();
#endif
		break;
	case WM_RBUTTONDOWN:
		g_iNoMouseTime = 0;
		MouseRButtonPop = false;
		if(!MouseRButton) MouseRButtonPush = true;
		MouseRButton = true;
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
		SetCapture(g_hWnd);
#endif
		break;
	case WM_RBUTTONUP:
		g_iNoMouseTime = 0;
		MouseRButtonPush = false;
		if(MouseRButton) MouseRButtonPop = true;
		MouseRButton = false;
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
		ReleaseCapture();
#endif
		break;
    case WM_LBUTTONDBLCLK:
		g_iNoMouseTime = 0;
		MouseLButtonDBClick = true;
		break;
	case WM_MBUTTONDOWN:
		g_iNoMouseTime = 0;
		MouseMButtonPop = false;
		if(!MouseMButton) MouseMButtonPush = true;
		MouseMButton = true;
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
		SetCapture(g_hWnd);
#endif
		break;
	case WM_MBUTTONUP:
		g_iNoMouseTime = 0;
		MouseMButtonPush = false;
		if(MouseMButton) MouseMButtonPop = true;
		MouseRButton = false;
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
		ReleaseCapture();
#endif
		break;
	case WM_MOUSEWHEEL:
        {
            MouseWheel = (short)HIWORD( wParam)/WHEEL_DELTA;
        }
        break;
	case WM_IME_NOTIFY:
		{
			if (g_iChatInputType == 1)
			{
				switch (wParam)
				{
				case IMN_SETCONVERSIONMODE:
					if (GetFocus() == g_hWnd)
					{
						CheckTextInputBoxIME(IME_CONVERSIONMODE);
					}
					break;
				case IMN_SETSENTENCEMODE:
					if (GetFocus() == g_hWnd)
					{
						CheckTextInputBoxIME(IME_SENTENCEMODE);
					}
					break;
				default:
					break;
				}
			}
		}
		break;
#ifdef PSW_BUGFIX_IME
	case WM_CHAR:
		{
			switch(wParam)
			{
			case VK_RETURN:
				{
					SetEnterPressed( true );
				}
				break;
			}
		}
		break;
#endif //PSW_BUGFIX_IME
    }

	if( g_BuffSystem ) {
		LRESULT result;
		TheBuffStateSystem().HandleWindowMessage( msg, wParam, lParam, result );
	}

    return DefWindowProc(hwnd,msg,wParam,lParam);
}

BOOL g_bSavage = FALSE;
bool CreateOpenglWindow();

BOOL IsUsingSavage( HINSTANCE hCurrentInst)
{
    char *windowName = "MU launching";
    WNDCLASS wndClass;
	
    wndClass.style         = CS_OWNDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc   = DefWindowProc;
    wndClass.cbClsExtra    = 0;
    wndClass.cbWndExtra    = 0;
    wndClass.hInstance     = hCurrentInst;
    wndClass.hIcon         = LoadIcon(NULL, "IDI_ICON1");
    wndClass.hCursor	   = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndClass.lpszMenuName  = NULL;
    wndClass.lpszClassName = windowName;
    RegisterClass(&wndClass);

	g_hWnd = CreateWindow(
		windowName, windowName,
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		0,
		0,
		100,
		40,
		NULL, NULL, hCurrentInst, NULL);

	g_ErrorReport.Write("Create OpenGL Window For Savage Test.\n\r");
	if ( !CreateOpenglWindow())
	{
		return ( FALSE);
	}

	char *lpszRenderer = ( char*)glGetString( GL_VENDOR);
	char *lpszSavage = "s3";
	BOOL bResult = ( 0 == strnicmp( lpszRenderer, lpszSavage, 2));

	KillGLWindow();
	DestroyWindow( g_hWnd);

	return ( bResult);
}

bool CreateOpenglWindow()
{
    PIXELFORMATDESCRIPTOR pfd;

    memset(&pfd, 0, sizeof(pfd));
    pfd.nSize        = sizeof(pfd);
    pfd.nVersion     = 1;
    pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType   = PFD_TYPE_RGBA;
    pfd.cColorBits   = 16;
	pfd.cDepthBits   = 16;

	if (!(g_hDC=GetDC(g_hWnd)))
	{
		g_ErrorReport.Write( "OpenGL Get DC Error - ErrorCode : %d\r\n", GetLastError());
		KillGLWindow();
		MessageBox(NULL,GlobalText[4],"OpenGL Get DC Error.",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	GLuint PixelFormat;

	if (!(PixelFormat=ChoosePixelFormat(g_hDC,&pfd)))
	{
		g_ErrorReport.Write( "OpenGL Choose Pixel Format Error - ErrorCode : %d\r\n", GetLastError());
		KillGLWindow();
		MessageBox(NULL,GlobalText[4],"OpenGL Choose Pixel Format Error.",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	if(!SetPixelFormat(g_hDC,PixelFormat,&pfd))
	{
		g_ErrorReport.Write( "OpenGL Set Pixel Format Error - ErrorCode : %d\r\n", GetLastError());
		KillGLWindow();
		MessageBox(NULL,GlobalText[4],"OpenGL Set Pixel Format Error.",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	if (!(g_hRC=wglCreateContext(g_hDC)))
	{
		g_ErrorReport.Write( "OpenGL Create Context Error - ErrorCode : %d\r\n", GetLastError());
		KillGLWindow();
		MessageBox(NULL,GlobalText[4],"OpenGL Create Context Error.",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	if(!wglMakeCurrent(g_hDC,g_hRC))
	{
		g_ErrorReport.Write( "OpenGL Make Current Error - ErrorCode : %d\r\n", GetLastError());
		KillGLWindow();
		MessageBox(NULL,GlobalText[4],"OpenGL Make Current Error.",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	ShowWindow(g_hWnd,SW_SHOW);
	SetForegroundWindow(g_hWnd);
	SetFocus(g_hWnd);
	return true;
}

HWND StartWindow(HINSTANCE hCurrentInst,int nCmdShow)
{
    char *windowName = "MU";

    WNDCLASS wndClass;
    HWND hWnd;
	
    wndClass.style         = CS_OWNDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc   = WndProc;
    wndClass.cbClsExtra    = 0;
    wndClass.cbWndExtra    = 0;
    wndClass.hInstance     = hCurrentInst;
    wndClass.hIcon         = LoadIcon(NULL, "IDI_ICON1");
    wndClass.hCursor	   = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndClass.lpszMenuName  = NULL;
    wndClass.lpszClassName = windowName;
    RegisterClass(&wndClass);

#ifdef ENABLE_FULLSCREEN
	{
#ifndef FOR_WORK
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
		if (g_bUseWindowMode == TRUE)
		{
			// ★ 필요한 창 크기 계산
			RECT rc = { 0, 0, WindowWidth, WindowHeight };
#if defined WINDOWMODE
			if (g_bUseWindowMode == TRUE)
				AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER | WS_CLIPCHILDREN, NULL);
			else
#endif	//WINDOWMODE
			AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, NULL);
			rc.right -= rc.left;
			rc.bottom -= rc.top;

#if defined WINDOWMODE
			if (g_bUseWindowMode == TRUE)
			{
			hWnd = CreateWindow(
				windowName, windowName,
				WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER | WS_CLIPCHILDREN,
				(GetSystemMetrics(SM_CXSCREEN) - rc.right) / 2,
				(GetSystemMetrics(SM_CYSCREEN) - rc.bottom) / 2,
				rc.right,
				rc.bottom,
				NULL, NULL, hCurrentInst, NULL);
			}
			else
#endif//WINDOWMODE
			{
			hWnd = CreateWindow(
				windowName, windowName,
				WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
				(GetSystemMetrics(SM_CXSCREEN) - rc.right) / 2,
				(GetSystemMetrics(SM_CYSCREEN) - rc.bottom) / 2,
				rc.right,
				rc.bottom,
				NULL, NULL, hCurrentInst, NULL);
			}
		}
		else
#endif//WINDOWMODE
		{
			hWnd = CreateWindowEx( WS_EX_TOPMOST | WS_EX_APPWINDOW,
				windowName, windowName,
				WS_POPUP,
				0, 0,
				WindowWidth,
				WindowHeight,
				NULL, NULL, hCurrentInst, NULL);
		}
#else //FOR_WORK
		hWnd = CreateWindow(
			windowName, windowName,
			WS_POPUP,
			0, 0,
			WindowWidth,
			WindowHeight,
			NULL, NULL, hCurrentInst, NULL);
#endif
	}
#else //ENABLE_FULLSCREEN
	{
		// ★ 필요한 창 크기 계산
		RECT rc = { 0, 0, WindowWidth, WindowHeight };
#if defined WINDOWMODE
		if (g_bUseWindowMode == TRUE)
			AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER | WS_CLIPCHILDREN, NULL);
		else
#endif
		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, NULL);
		rc.right -= rc.left;
		rc.bottom -= rc.top;
#if defined WINDOWMODE
		if (g_bUseWindowMode == TRUE)
		{
		hWnd = CreateWindow(
			windowName, windowName,
			WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER | WS_CLIPCHILDREN,
			(GetSystemMetrics(SM_CXSCREEN) - rc.right) / 2,
			(GetSystemMetrics(SM_CYSCREEN) - rc.bottom) / 2,
			rc.right,
			rc.bottom,
			NULL, NULL, hCurrentInst, NULL);
		}
		else
#endif
		{
		hWnd = CreateWindow(
			windowName, windowName,
			WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
			(GetSystemMetrics(SM_CXSCREEN) - rc.right) / 2,
			(GetSystemMetrics(SM_CYSCREEN) - rc.bottom) / 2,
			rc.right,
			rc.bottom,
			NULL, NULL, hCurrentInst, NULL);
		}
	}
#endif //ENABLE_FULLSCREEN
	
    return hWnd;
}

char m_ID[11];
char m_Version[11];
char m_ExeVersion[11];
int  m_SoundOnOff;
int  m_MusicOnOff;
int  m_Resolution;
int	m_nColorDepth;
int	g_iRenderTextType = 0;

#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
char g_aszMLSelection[MAX_LANGUAGE_NAME_LENGTH] = {'\0'};
std::string g_strSelectedML = "";
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE

BOOL OpenInitFile()
{
	//char szTemp[50];
	char szIniFilePath[256+20]="";
	char szCurrentDir[256];

	GetCurrentDirectory(256, szCurrentDir);

	strcpy(szIniFilePath, szCurrentDir);
	if( szCurrentDir[strlen(szCurrentDir)-1] == '\\' ) 
		strcat(szIniFilePath, "config.ini");
	else strcat(szIniFilePath, "\\config.ini");

	GetPrivateProfileString ("LOGIN", "Version", "", m_Version, 11, szIniFilePath);

	char *lpszCommandLine = GetCommandLine();
	char lpszFile[MAX_PATH];
	if ( GetFileNameOfFilePath( lpszFile, lpszCommandLine))
	{
		WORD wVersion[4];
		if ( GetFileVersion( lpszFile, wVersion))
		{
			sprintf( m_ExeVersion, "%d.%02d", wVersion[0], wVersion[1]);
			if ( wVersion[2] > 0)
			{
                char lpszMinorVersion[3] = "a";
                if ( wVersion[2]>26 )
                {
                    lpszMinorVersion[0] = 'A';
				    lpszMinorVersion[0] += ( wVersion[2] - 27 );
                    lpszMinorVersion[1] = '+';
                }
                else
                {
				    lpszMinorVersion[0] += ( wVersion[2] - 1);
                }
				strcat( m_ExeVersion, lpszMinorVersion);
			}
		}
		else
		{
			strcpy( m_ExeVersion, m_Version);
		}
	}
	else
	{
		strcpy( m_ExeVersion, m_Version);
	}

//#ifdef _DEBUG

	m_ID[0] = '\0';
	m_SoundOnOff = 1;
	m_MusicOnOff = 1;
	m_Resolution = 0;
	m_nColorDepth = 0;

	HKEY hKey;
	DWORD dwDisp;
	DWORD dwSize;
	if ( ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\Webzen\\Mu\\Config", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, & hKey, &dwDisp))
	{
		dwSize = 11;
		if ( RegQueryValueEx (hKey, "ID", 0, NULL, (LPBYTE)m_ID, & dwSize) != ERROR_SUCCESS)
		{
		}
		dwSize = sizeof ( int);
		if ( RegQueryValueEx (hKey, "SoundOnOff", 0, NULL, (LPBYTE) & m_SoundOnOff, &dwSize) != ERROR_SUCCESS)
		{
			m_SoundOnOff = true;
		}
		dwSize = sizeof ( int);
		if ( RegQueryValueEx (hKey, "MusicOnOff", 0, NULL, (LPBYTE) & m_MusicOnOff, &dwSize) != ERROR_SUCCESS)
		{
			m_MusicOnOff = false;
		}
		dwSize = sizeof ( int);
		if ( RegQueryValueEx (hKey, "Resolution", 0, NULL, (LPBYTE) & m_Resolution, &dwSize) != ERROR_SUCCESS)
			m_Resolution = 1;

		if (0 == m_Resolution)
			m_Resolution = 1;

	    if ( RegQueryValueEx (hKey, "ColorDepth", 0, NULL, (LPBYTE) & m_nColorDepth, &dwSize) != ERROR_SUCCESS)
	    {
		    m_nColorDepth = 0;
	    }
		dwSize = sizeof ( int);
		if ( RegQueryValueEx (hKey, "TextOut", 0, NULL, (LPBYTE) & g_iRenderTextType, &dwSize) != ERROR_SUCCESS)
		{
			g_iRenderTextType = 0;
		}

		g_iChatInputType = 1;

#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
		dwSize = sizeof ( int);
		if ( RegQueryValueEx (hKey, "WindowMode", 0, NULL, (LPBYTE) & g_bUseWindowMode, &dwSize) != ERROR_SUCCESS)
		{
			g_bUseWindowMode = FALSE;
		}
#endif // USER_WINDOW_MODE

#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		dwSize = MAX_LANGUAGE_NAME_LENGTH;
		if ( RegQueryValueEx (hKey, "LangSelection", 0, NULL, (LPBYTE)g_aszMLSelection, &dwSize) != ERROR_SUCCESS)
		{
			strcpy(g_aszMLSelection, "Eng");
		}
		g_strSelectedML = g_aszMLSelection;
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE

#ifdef ADD_GFX_REG_OPTION
		//레지스트레에서 ui설정값 확인
		dwSize = sizeof (int);
		int _iUiSelection = 0;
		if ( RegQueryValueEx (hKey, "UiSelection", 0, NULL, (LPBYTE) & _iUiSelection, &dwSize) != ERROR_SUCCESS)
		{
			_iUiSelection = 0;
		}
		GFxProcess::GetInstancePtr()->SetUISelect(_iUiSelection);
#endif //ADD_GFX_REG_OPTION
	}
	RegCloseKey( hKey);

	switch(m_Resolution)
	{
	case 0:WindowWidth=640 ;WindowHeight=480 ;break;
	case 1:WindowWidth=800 ;WindowHeight=600 ;break;
	case 2:WindowWidth=1024;WindowHeight=768 ;break;
	case 3:WindowWidth=1280;WindowHeight=1024;break;
	//case 3:WindowWidth=1920;WindowHeight=1440;break;
	case 4:WindowWidth=1600;WindowHeight=1200;break;
	}
	
	g_fScreenRate_x = (float)WindowWidth / 640;		// ※
	g_fScreenRate_y = (float)WindowHeight / 480;

	return TRUE;
}

BOOL Util_CheckOption( char *lpszCommandLine, unsigned char cOption, char *lpszString)
{
	unsigned char cComp[2];
	cComp[0] = cOption; cComp[1] = cOption;
	if ( islower( ( int)cOption))
	{
		cComp[1] = toupper( ( int)cOption);
	}
	else if ( isupper( ( int)cOption))
	{
		cComp[1] = tolower( ( int)cOption);
	}

	int nFind = ( int)'/';
	unsigned char *lpFound = ( unsigned char*)lpszCommandLine;
	while ( lpFound)
	{
		lpFound = ( unsigned char*)strchr( ( char*)( lpFound + 1), nFind);
		if ( lpFound && ( *( lpFound + 1) == cComp[0] || *( lpFound + 1) == cComp[1]))
		{	// 발견
			if ( lpszString)
			{
				int nCount = 0;
				for ( unsigned char *lpSeek = lpFound + 2; *lpSeek != ' ' && *lpSeek != '\0'; lpSeek++)
				{
					nCount++;
				}

				memcpy( lpszString, lpFound + 2, nCount);
				lpszString[nCount] = '\0';
			}
			return ( TRUE);
		}
	}

	return ( FALSE);
}

BOOL UpdateFile( char *lpszOld, char *lpszNew)
{
	SetFileAttributes(lpszOld, FILE_ATTRIBUTE_NORMAL);
	SetFileAttributes(lpszNew, FILE_ATTRIBUTE_NORMAL);

	DWORD dwStartTickCount = ::GetTickCount();
	while(::GetTickCount() - dwStartTickCount < 5000) {
		if ( CopyFile( lpszOld, lpszNew, FALSE))
		{	// 성공
			DeleteFile( lpszOld);
			return ( TRUE);
		}
	}
	g_ErrorReport.Write("%s to %s CopyFile Error : %d\r\n", lpszNew, lpszOld, GetLastError());
	return ( FALSE);
}

#include <tlhelp32.h>

BOOL KillExeProcess( char *lpszExe)
{
	HANDLE hProcessSnap = NULL; 
    BOOL bRet = FALSE; 
    PROCESSENTRY32 pe32 = { 0 }; 
 
    //  Take a snapshot of all processes in the system. 

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 

    if (hProcessSnap == INVALID_HANDLE_VALUE) 
        return (FALSE); 
 
    //  Fill in the size of the structure before using it. 

    pe32.dwSize = sizeof(PROCESSENTRY32); 
 
    //  Walk the snapshot of the processes, and for each process, 
    //  display information. 

    if (Process32First(hProcessSnap, &pe32)) 
    {
        do 
        { 
			if(stricmp(pe32.szExeFile, lpszExe) == 0)
			{
				HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);

				if(process)
				{
					TerminateProcess(process, 0);
				}
			}
        } while (Process32Next(hProcessSnap, &pe32)); 
        bRet = TRUE; 
    } 
    else 
        bRet = FALSE;    // could not walk the list of processes 
 
    // Do not forget to clean up the snapshot object. 

    CloseHandle (hProcessSnap);

	return bRet;
}

char g_lpszCmdURL[50];
BOOL GetConnectServerInfo( PSTR szCmdLine, char *lpszURL, WORD *pwPort)
{
	char lpszTemp[256] = {0, };
	if( Util_CheckOption( szCmdLine, 'y', lpszTemp))
	{
		BYTE bySuffle[] = { 0x0C, 0x07, 0x03, 0x13 };

		for(int i=0; i<(int)strlen(lpszTemp); i++)
			lpszTemp[i] -= bySuffle[i%4];
		strcpy(lpszURL, lpszTemp);

		if( Util_CheckOption( szCmdLine, 'z', lpszTemp)) 
		{
			for(int j=0; j<(int)strlen(lpszTemp); j++)
				lpszTemp[j] -= bySuffle[j%4];
			*pwPort = atoi( lpszTemp);
		}
		g_ErrorReport.Write("[Virtual Connection] Connect IP : %s, Port : %d\r\n", lpszURL, *pwPort);
		return (TRUE);
	}
	if ( !Util_CheckOption( szCmdLine, 'u', lpszTemp))
	{
		return ( FALSE);
	}
	strcpy( lpszURL, lpszTemp);
	if ( !Util_CheckOption( szCmdLine, 'p', lpszTemp))
	{
		return ( FALSE);
	}
	*pwPort = atoi( lpszTemp);

	return ( TRUE);
}


extern int TimeRemain;
BOOL g_bInactiveTimeChecked = FALSE;
void MoveObject(OBJECT *o);

bool ExceptionCallback(_EXCEPTION_POINTERS* pExceptionInfo )
{
#ifdef KJH_LOG_ERROR_DUMP
	DebugAngel_Write(LOG_ERROR_DUMP_FILENAME, " ## Start Log\r\n");
#endif // KJH_LOG_ERROR_DUMP

#ifndef MSZ_ADD_CRASH_DUMP_UPLOAD	// 덤프를 두번 생성할 필요는 없으므로..
	if(leaf::SaveExceptionDumpFile("MuError.dmp", pExceptionInfo->ContextRecord, pExceptionInfo))
	{
		g_ErrorReport.Write("Save DumpFile complete - MuError.dmp\r\n");
	}
#endif

#ifdef KJH_LOG_ERROR_DUMP
	DebugAngel_Write(LOG_ERROR_DUMP_FILENAME, " ## End Log\r\n");
#endif // KJH_LOG_ERROR_DUMP

#ifdef ENABLE_FULLSCREEN
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
	if (g_bUseWindowMode == FALSE)
#endif	// USER_WINDOW_MODE
	{
		ChangeDisplaySettings(NULL,0);					// 데스크탑 설정으로 복귀
	}
#endif //ENABLE_FULLSCREEN

	return true;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int nCmdShow)
{
#ifdef LDK_ADD_SCALEFORM
	GFxSystem gfxInit;
#endif //LDK_ADD_SCALEFORM

	MSG msg;

#ifdef USE_CRT_DEBUG
	//_CrtSetBreakAlloc(67839);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	leaf::AttachExceptionHandler(ExceptionCallback);

#ifndef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	OpenTextData();		//. Text.bmd, Testtest.bmd를 읽는다.
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	{
		char lpszExeVersion[256] = "unknown";

		char *lpszCommandLine = GetCommandLine();
		char lpszFile[MAX_PATH];
		WORD wVersion[4] = { 0,};
		if ( GetFileNameOfFilePath( lpszFile, lpszCommandLine))
		{
			if ( GetFileVersion( lpszFile, wVersion))
			{
				sprintf( lpszExeVersion, "%d.%02d", wVersion[0], wVersion[1]);
				if ( wVersion[2] > 0)
				{
					char lpszMinorVersion[2] = "a";
					lpszMinorVersion[0] += ( wVersion[2] - 1);
					strcat( lpszExeVersion, lpszMinorVersion);
				}
			}
		}

		g_ErrorReport.Write( "\r\n");
		g_ErrorReport.WriteLogBegin();
		g_ErrorReport.AddSeparator();
		g_ErrorReport.Write( "Mu online %s (%s) executed. (%d.%d.%d.%d)\r\n", lpszExeVersion, "Eng", wVersion[0], wVersion[1], wVersion[2], wVersion[3]);

		g_ConsoleDebug->Write(MCD_NORMAL, "Mu Online (Version: %d.%d.%d.%d)", wVersion[0], wVersion[1], wVersion[2], wVersion[3]);

		g_ErrorReport.WriteCurrentTime();
		ER_SystemInfo si;
		ZeroMemory( &si, sizeof ( ER_SystemInfo));
		GetSystemInfo( &si);
		g_ErrorReport.AddSeparator();
		g_ErrorReport.WriteSystemInfo( &si);
		g_ErrorReport.AddSeparator();
	}

	// PKD_ADD_BINARY_PROTECTION
	VM_START
	WORD wPortNumber;	
	if ( GetConnectServerInfo( szCmdLine, g_lpszCmdURL, &wPortNumber))
	{
		szServerIpAddress = g_lpszCmdURL;
		g_ServerPort = wPortNumber;
	}
	VM_END

	if ( !OpenMainExe())
	{
		return false;
	}

	// PKD_ADD_BINARY_PROTECTION
	VM_START
	g_SimpleModulusCS.LoadEncryptionKey( "Data\\Enc1.dat");
	g_SimpleModulusSC.LoadDecryptionKey( "Data\\Dec2.dat");
	VM_END

	g_ErrorReport.Write( "> To read config.ini.\r\n");
	if( OpenInitFile() == FALSE )
	{
		g_ErrorReport.Write( "config.ini read error\r\n");
		return false;
	}

	pMultiLanguage = new CMultiLanguage(g_strSelectedML);

	if (g_iChatInputType == 1)
		ShowCursor(FALSE);

	g_ErrorReport.Write( "> Enum display settings.\r\n");
	DEVMODE DevMode;
	DEVMODE* pDevmodes;
	int nModes = 0;
	while (EnumDisplaySettings(NULL, nModes, &DevMode)) nModes++;
	pDevmodes = new DEVMODE[nModes+1];
	nModes = 0;
	while (EnumDisplaySettings(NULL, nModes, &pDevmodes[nModes])) nModes++;

	DWORD dwBitsPerPel = 16;
	for(int n1=0; n1<nModes; n1++)
	{
		if(pDevmodes[n1].dmBitsPerPel == 16 && m_nColorDepth == 0) {
			dwBitsPerPel = 16; break;
		}
		if(pDevmodes[n1].dmBitsPerPel == 24 && m_nColorDepth == 1) {
			dwBitsPerPel = 24; break;
		}
		if(pDevmodes[n1].dmBitsPerPel == 32 && m_nColorDepth == 1) {
			dwBitsPerPel = 32; break;
		}
	}

#ifdef ENABLE_FULLSCREEN
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
	if (g_bUseWindowMode == FALSE)
#endif	// USER_WINDOW_MODE
	{
    	//윈도우를 풀스크린으로 변환	
		for(int n2=0; n2<nModes; n2++)
		{
			if(pDevmodes[n2].dmPelsWidth==WindowWidth && pDevmodes[n2].dmPelsHeight==WindowHeight && pDevmodes[n2].dmBitsPerPel == dwBitsPerPel)
			{
				g_ErrorReport.Write( "> Change display setting %dx%d.\r\n", pDevmodes[n2].dmPelsWidth, pDevmodes[n2].dmPelsHeight);
				ChangeDisplaySettings(&pDevmodes[n2],0);
				break;
			}
		}
	}
#endif //ENABLE_FULLSCREEN

	delete [] pDevmodes;

	g_ErrorReport.Write( "> Screen size = %d x %d.\r\n", WindowWidth, WindowHeight);

    g_hInst = hInstance;
    g_hWnd = StartWindow(hInstance,nCmdShow);
	g_ErrorReport.Write( "> Start window success.\r\n");

    if ( !CreateOpenglWindow())
	{
		return FALSE;
	}

	g_ErrorReport.Write( "> OpenGL init success.\r\n");
	g_ErrorReport.AddSeparator();
	g_ErrorReport.WriteOpenGLInfo();
	g_ErrorReport.AddSeparator();
	g_ErrorReport.WriteSoundCardInfo();

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

	g_ErrorReport.WriteImeInfo( g_hWnd);
	g_ErrorReport.AddSeparator();
	
	switch(WindowWidth)
	{
	case 640 :FontHeight = 12;break;
	case 800 :FontHeight = 13;break;
	case 1024:FontHeight = 14;break;
	case 1280:FontHeight = 15;break;
	}
	
	int nFixFontHeight = 13;
	int nFixFontSize;
	int iFontSize;

	iFontSize = FontHeight - 1;
	nFixFontSize = nFixFontHeight - 1;

	g_hFont		= CreateFont(iFontSize,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,DEFAULT_PITCH|FF_DONTCARE,GlobalText[0][0] ? GlobalText[0] : NULL);
	g_hFontBold = CreateFont(iFontSize,0,0,0,FW_BOLD,0,0,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,DEFAULT_PITCH|FF_DONTCARE,GlobalText[0][0] ? GlobalText[0] : NULL);
	g_hFontBig	= CreateFont(iFontSize*2,0,0,0,FW_BOLD,0,0,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,DEFAULT_PITCH|FF_DONTCARE,GlobalText[0][0] ? GlobalText[0] : NULL);
	g_hFixFont	= CreateFont(nFixFontSize,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,DEFAULT_PITCH | FF_DONTCARE,GlobalText[18][0] ? GlobalText[18] : NULL);

#ifdef PBG_ADD_INGAMESHOP_FONT
	int _FontSize = 12;
	switch(WindowWidth)
	{
	case 800:
		_FontSize = 16;
		break;
	case 1024:
		_FontSize = 19;
		break;
	case 1280:
		_FontSize = 26;
		break;
	default:
		_FontSize = 12;
		break;
	}

	g_hInGameShopFont = CreateFont(_FontSize,0,0,0,FW_NORMAL,0,0,0,
		g_dwCharSet[SELECTED_LANGUAGE],OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
		NONANTIALIASED_QUALITY,DEFAULT_PITCH|FF_DONTCARE,
		GlobalText[0][0] ? GlobalText[0] : NULL);
#endif //PBG_ADD_INGAMESHOP_FONT
	// 텍스트 길이 계산을 위한 부분
	setlocale( LC_ALL, "english");

	CInput::Instance().Create(g_hWnd, WindowWidth, WindowHeight);

	// 새로운 UI시스템 생성
	g_pNewUISystem->Create();

#ifdef LDK_ADD_SCALEFORM
	if(GFxProcess::GetInstancePtr()->GetUISelect() == 1)
	{
		if(GFxProcess::GetInstancePtr()->GFxInit())
		{
			//error log
		}
		
		GFxProcess::GetInstancePtr()->OnCreateDevice(WindowWidth, WindowHeight, 0, 0, WindowWidth, WindowHeight);
	}
#endif //LDK_ADD_SCALEFORM

	if(m_MusicOnOff)
	{
		wzAudioCreate(g_hWnd);
		wzAudioOption(WZAOPT_STOPBEFOREPLAY, 1);
	}

	if(m_SoundOnOff)
	{
		InitDirectSound(g_hWnd);
		leaf::CRegKey regkey;
		regkey.SetKey(leaf::CRegKey::_HKEY_CURRENT_USER, "SOFTWARE\\Webzen\\Mu\\Config");
		DWORD value;
		if(!regkey.ReadDword("VolumeLevel", value))
		{
			value = 5;	//. default setting
			regkey.WriteDword("VolumeLevel", value);
		}
		if(value<0 || value>=10)
			value = 5;
		
		g_pOption->SetVolumeLevel(int(value));
		SetEffectVolumeLevel(g_pOption->GetVolumeLevel());
	}

	SetTimer(g_hWnd, HACK_TIMER, 20*1000, NULL);

	srand((unsigned)time(NULL));
	for(int i=0;i<100;i++)
		RandomTable[i] = rand()%360;

	//memorydump[0]
	RendomMemoryDump = new BYTE [rand()%100+1];


	GateAttribute				= new GATE_ATTRIBUTE [MAX_GATES];
	SkillAttribute				= new SKILL_ATTRIBUTE[MAX_SKILLS];

	//memorydump[1]
	ItemAttRibuteMemoryDump		= new ITEM_ATTRIBUTE [MAX_ITEM+1024];
	ItemAttribute				= ((ITEM_ATTRIBUTE*)ItemAttRibuteMemoryDump)+rand()%1024;

	//memorydump[2]
	CharacterMemoryDump			= new CHARACTER      [MAX_CHARACTERS_CLIENT+1+128];
	CharactersClient			= ((CHARACTER*)CharacterMemoryDump)+rand()%128;
	CharacterMachine			= new CHARACTER_MACHINE;

	memset(GateAttribute       ,0,sizeof(GATE_ATTRIBUTE   )*(MAX_GATES              ));
	memset(ItemAttribute       ,0,sizeof(ITEM_ATTRIBUTE   )*(MAX_ITEM               ));
	memset(SkillAttribute      ,0,sizeof(SKILL_ATTRIBUTE  )*(MAX_SKILLS             ));
	memset(CharacterMachine    ,0,sizeof(CHARACTER_MACHINE));

    CharacterAttribute   = &CharacterMachine->Character;
    CharacterMachine->Init();
	Hero = &CharactersClient[0];	

	if (g_iChatInputType == 1)
	{
		 g_pMercenaryInputBox = new CUIMercenaryInputBox;
		 g_pSingleTextInputBox = new CUITextInputBox;
		 g_pSinglePasswdInputBox = new CUITextInputBox;
	}
	
	g_pChatRoomSocketList = new CChatRoomSocketList;
	g_pUIManager = new CUIManager;
	g_pUIMapName = new CUIMapName;	// rozy
	g_pTimer = new CTimer();

#ifdef MOVIE_DIRECTSHOW
	g_pMovieScene = new CMovieScene;
#endif // MOVIE_DIRECTSHOW
	
#ifdef NEW_USER_INTERFACE_SHELL
	g_shell = Shell::Make();
	g_shell->Initialize( g_hWnd, WindowWidth, WindowHeight );
#endif //NEW_USER_INTERFACE_SHELL

	g_BuffSystem = BuffStateSystem::Make();

	g_MapProcess = MapProcess::Make();

	g_petProcess = PetProcess::Make();

	CUIMng::Instance().Create();

	if (g_iChatInputType == 1)
	{
		g_pMercenaryInputBox->Init(g_hWnd);
		g_pSingleTextInputBox->Init(g_hWnd, 200, 20);
		g_pSinglePasswdInputBox->Init(g_hWnd, 200, 20, 9, TRUE);
		g_pSingleTextInputBox->SetState(UISTATE_HIDE);
		g_pSinglePasswdInputBox->SetState(UISTATE_HIDE);

		g_pMercenaryInputBox->SetFont(g_hFont);
		g_pSingleTextInputBox->SetFont(g_hFont);
		g_pSinglePasswdInputBox->SetFont(g_hFont);

		g_bIMEBlock = FALSE;
		HIMC  hIMC = ImmGetContext(g_hWnd);
		ImmSetConversionStatus(hIMC, IME_CMODE_ALPHANUMERIC, IME_SMODE_NONE );
		ImmReleaseContext(g_hWnd, hIMC);
		SaveIMEStatus();
		g_bIMEBlock = TRUE;
	}
#if (defined WINDOWMODE)
	if (g_bUseWindowMode == FALSE)
	{
		int nOldVal;
		SystemParametersInfo(SPI_SCREENSAVERRUNNING, 1, &nOldVal, 0);
		SystemParametersInfo(SPI_GETSCREENSAVETIMEOUT, 0, &g_iScreenSaverOldValue, 0);
		SystemParametersInfo(SPI_SETSCREENSAVETIMEOUT, 300*60, NULL, 0);
	}
#else
#ifdef NDEBUG
#ifndef FOR_WORK
#ifdef ACTIVE_FOCUS_OUT
	if (g_bUseWindowMode == FALSE)
	{
#endif	// ACTIVE_FOCUS_OUT
		int nOldVal; // 값이 들어갈 필요가 없음
		SystemParametersInfo(SPI_SCREENSAVERRUNNING, 1, &nOldVal, 0);  // 단축키를 못쓰게 함
		SystemParametersInfo(SPI_GETSCREENSAVETIMEOUT, 0, &g_iScreenSaverOldValue, 0);  // 스크린세이버 차단
		SystemParametersInfo(SPI_SETSCREENSAVETIMEOUT, 300*60, NULL, 0);  // 스크린세이버 차단
#ifdef ACTIVE_FOCUS_OUT
	}
#endif	// ACTIVE_FOCUS_OUT
#endif
#endif
#endif	//WINDOWMODE(#else)

#ifdef SAVE_PACKET
	DeleteFile( PACKET_SAVE_FILE);
#endif

#if defined PROTECT_SYSTEMKEY && defined NDEBUG
#ifndef FOR_WORK
	ProtectSysKey::AttachProtectSysKey(g_hInst, g_hWnd);
#endif // !FOR_WORK
#endif // PROTECT_SYSTEMKEY && NDEBUG
	
#ifdef TEENAGER_REGULATION
	SetTimer( g_hWnd, WARNING_TIMER, 60*60*1000, NULL);
//	SetTimer( g_hWnd, WARNING_TIMER, 3*60*1000, NULL);
	//시간조정
#endif
	//메세지 루프
	//MSG msg;

	// Profiling Initialize 정의
#ifdef DO_PROFILING
	g_pProfiler = g_pProfiler->GetThis_();
	g_pProfiler->Initialize(	TRUE, 
								"PROFILING_RESULT.txt",
								EPROFILESORTING_WEIGHT, 
								EPROFILESORTING_DIRECTION_DESCENDING );	// m_pProfiler->Initialize( TRUE, "PROFILE_RESULT.txt" );
#endif // DO_PROFILING

#ifdef DO_PROCESS_DEBUGCAMERA
	g_pDebugCameraManager = g_pDebugCameraManager->GetThis_();
	
	g_pDebugCameraManager->Initialize();
	
	for( int i_ = 0; i_ < (int)EDEBUGCAMERA_END; ++i_ )
	{
		vec3_t	v3DefaultPosition, v3DefaultAngle;
		v3DefaultPosition[0] = 0.0f; v3DefaultPosition[1] = 0.0f; v3DefaultPosition[2] = 0.0f; 
		v3DefaultAngle[0] = 0.0f; v3DefaultAngle[1] = 0.0f; v3DefaultAngle[2] = 0.0f; 
		
		g_pDebugCameraManager->InsertNewCamera( (EDEBUGCAMERA_TYPE)i_,
												v3DefaultPosition,
												v3DefaultAngle, 
												_DEFAULT_CAMERADISTANCE_NEAR_, 
												_DEFAULT_CAMERADISTANCE_FAR_,
												_DEFAULT_VELOCITY_PERTICK * 10, 
												_DEFAULT_RATE_ANGULARVELOCITY * 0.25f, 
												FALSE );
	}
#endif // DO_PROCESS_DEBUGCAMERA

    while( 1 )
    {
#ifdef USE_SELFCHECKCODE
		if ( TimeRemain >= 40)
		{
			SendCrcOfFunction( 8, 20, MoveHero, 0x93B7);
		}
#endif
        if( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
        {
            if( !GetMessage( &msg, NULL, 0, 0 ) )
            {
				break;
            }
            else
			{
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
     	else 
		{
#ifdef USE_SELFCHECKCODE
			if ( TimeRemain >= 40)
			{
				SendCrcOfFunction( 15, 11, MoveObject, 0x7AC0);
			}
#endif

			//Scene
			FAKE_CODE( Pos_ActiveCheck);
Pos_ActiveCheck:
#if (defined WINDOWMODE)
			if (g_bUseWindowMode == TRUE)
			{
				Scene(g_hDC);
			}
			else if(g_bWndActive)
			{
		       	Scene(g_hDC);
			}
#ifndef FOR_WORK
			else if (g_bUseWindowMode == FALSE)
			{
				SetForegroundWindow( g_hWnd);
				SetFocus(g_hWnd);

				if ( g_iInactiveWarning > 1)
				{
					SetTimer( g_hWnd, WINDOWMINIMIZED_TIMER, 1*1000, NULL);
					PostMessage(g_hWnd, WM_CLOSE, 0, 0);
				}
				else
				{
					g_iInactiveWarning++;
					g_bMinimizedEnabled = TRUE;
					ShowWindow( g_hWnd, SW_MINIMIZE);
					g_bMinimizedEnabled = FALSE;
					ShowWindow( g_hWnd, SW_MAXIMIZE);
				}
			}
#endif//FOR_WORK
#else//WINDOWMODE
			if(g_bWndActive)
		       	Scene(g_hDC);

#endif	//WINDOWMODE(#else)
		}
        ProtocolCompiler();
		g_pChatRoomSocketList->ProtocolCompile();
    } // while( 1 )

#ifdef DO_PROCESS_DEBUGCAMERA	
	g_pDebugCameraManager->Release();
#endif // DO_PROCESS_DEBUGCAMERA

#ifdef DO_PROFILING
	g_pProfiler->Release();
#endif // DO_PROFILING

#if defined PROTECT_SYSTEMKEY && defined NDEBUG
#ifndef FOR_WORK
	ProtectSysKey::DetachProtectSysKey();
#endif // !FOR_WORK
#endif // PROTECT_SYSTEMKEY && NDEBUG

#ifdef MSZ_ADD_CRASH_DUMP_UPLOAD
	Crash::Uninstall();
#endif

	DestroyWindow();

    return msg.wParam;
#ifdef USE_SELFCHECKCODE
	END_OF_FUNCTION( Pos_SelfCheck01);
Pos_SelfCheck01:
	;
#endif
}

