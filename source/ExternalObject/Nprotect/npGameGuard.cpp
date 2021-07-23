//////////////////////////////////////////////////////////////////////////
//  
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <process.h>
#include "zzzscene.h"


#ifdef NP_GAME_GUARD

#include "ZzzOpenglUtil.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "wsclientinline.h"

#include "npGameGuard.h"

#ifdef PBG_MOD_GAMEGUARD_HANDLE
#include "NPGameGuardHandle.h"
#endif //PBG_MOD_GAMEGUARD_HANDLE
///////////////////////////////////////
//  Extern.
///////////////////////////////////////
extern  CNPGameLib*  g_pnpGL;     //  nProtect Game Guard Lib.
extern  HWND        g_hWnd;     //  Main Window Handle.

extern  void CreateWhisper(char *ID,char *Text,int Type);



///////////////////////////////////////
//  npGameGuard.
///////////////////////////////////////
namespace npGameGuard
{
///////////////////////////////////////
//  Function.
///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//  Func    : init 
//
//  파라미터: HWND hWnd             -   메인 윈도우 핸들.
//            char* strGameName     -   게임 이름.
//
//  설명    : 메인 윈도우 핸들 생성 후 nProtect Game Guard를 초기화.
//////////////////////////////////////////////////////////////////////////
bool    init ( void )
{
	if(g_pnpGL == NULL)
		return false;

	DWORD	dwResult = g_pnpGL->Init();

    if ( dwResult!=NPGAMEMON_SUCCESS )
    {
        showErrorMessage ( NULL, dwResult );

        return false;
    }
#ifdef GAMEGUARD_AUTH25
	SetCallbackToGameMon(NPGameMonCallback);
#endif //GAMEGUARD_AUTH25
    return true;
}


//////////////////////////////////////////////////////////////////////////
//  게임 윈도우 핸들을 게임가드에 연결한다.
//////////////////////////////////////////////////////////////////////////
void    SetHwnd ( HWND hWnd )
{
	if(g_pnpGL != NULL)
		g_pnpGL->SetHwnd( hWnd );
}


//////////////////////////////////////////////////////////////////////////
//  Func    : loginID
//
//  파라미터: char* strID           - 사용자 아이디.
//
//  설명    : 로그인 하는 사용자의 아이디를 GameMon에 통보한다.
//////////////////////////////////////////////////////////////////////////
void    loginID ( char* strID )
{
	if(g_pnpGL == NULL)
		return;
	
    if ( !SendUserIDToGameMon( strID ) )
    {
        //  강제 종료.
        CloseHack ( g_hWnd, true );
    }

#ifdef _DEBUG
    TCHAR msg[128];
    wsprintf ( msg, "ID:%s를 GameMon에게 알린다.", strID );
    MessageBox ( g_hWnd, msg, "알림", MB_OK );
#endif
}


//////////////////////////////////////////////////////////////////////////
//  Func    : CheckGameMon
//  
//  설명    : nProtect의 GameMon과의 통신 상태를 검사하여 통신 두절 상태
//            라면 게임을 종료 
//////////////////////////////////////////////////////////////////////////
void    CheckGameMon ( void )
{
	if(g_pnpGL == NULL)
	{
#ifdef CSK_FIX_GAMEGUARD_CHECK
		CloseHack ( g_hWnd, true );
#endif // CSK_FIX_GAMEGUARD_CHECK

		return;
	}
	
#ifdef _DEBUG
	g_pChatListBox->AddText("nProtect", "GameMon과의 통신 상태 검사", SEASON3B::TYPE_SYSTEM_MESSAGE);	
#endif

	if ( g_pnpGL->Check()!=NPGAMEMON_SUCCESS )
    {
        //  GameMon과의 통신 두절.
        //MessageBox ( g_hWnd, GlobalText[791], "Error", MB_OK );

        //  강제 종료.
        CloseHack ( g_hWnd, true );
    }
}


//////////////////////////////////////////////////////////////////////////
//  Func    : showErrorMessage
//
//  파라미터: HWND  hWnd            - 메인 윈도우 핸들.
//            DWORD dwResult        - 에러 메시지 값.
//
//  설명    : 결과값에 해당되는 에러 메시지를 보여준다.
//////////////////////////////////////////////////////////////////////////
void    showErrorMessage ( HWND hWnd, DWORD dwResult )
{
    bool    bShow = true;
    TCHAR   msg[256];

    switch ( dwResult )
    {
	case NPGMUP_ERROR_DOWNCFG:	// 업데이트 에러
	case NPGMUP_ERROR_ABORT:
	case NPGMUP_ERROR_CONNECT:
		wsprintf ( msg, GlobalText[800], dwResult );
		break;
	case NPGAMEMON_ERROR_GAME_EXIST:	//- "게임이 중복 실행되었거나 게임가드가 이미 실행 중 입니다. 게임 종료 후 다시 실행해보시기 바랍니다."
		wsprintf ( msg, GlobalText[795], dwResult );
		break;
	case NPGAMEMON_ERROR_ILLEGAL_PRG:	//- "불법 프로그램이 발견되었습니다. 불필요한 프로그램을 종료한 후 다시 실행해보시기 바랍니다."
		wsprintf ( msg, GlobalText[796], dwResult );
		break;
	case NPGAMEMON_ERROR_CRYPTOAPI:	//- "윈도우의 일부 시스템 파일이 손상되었습니다. 인터넷 익스플로러(IE)를 다시 설치해보시기 바랍니다."
		wsprintf ( msg, GlobalText[797], dwResult );
		break;
	case NPGAMEMON_ERROR_EXECUTE:	//- "게임가드 실행에 실패했습니다. 게임가드 셋업 파일을 다시 설치해보시기 바랍니다."
		wsprintf ( msg, GlobalText[798], dwResult );
		break;

    case    NPGAMEMON_ERROR_EXIST :         // GameMon Already Exist
        wsprintf ( msg, GlobalText[792], dwResult );
        break;

    case    NPGAMEMON_ERROR_CREATE :        // GameGuard Directory Create Error
        wsprintf ( msg, GlobalText[792], dwResult );
        break;

    case    NPGAMEMON_ERROR_NPSCAN :        // npscan.dll Error
        wsprintf ( msg, GlobalText[792], dwResult );
        break;

    case    NPGAMEMON_ERROR_AUTH_INI :      // .ini Authentication Fail
        wsprintf ( msg, GlobalText[792], dwResult );
        break;

    case    NPGAMEMON_ERROR_AUTH_NPGMUP :   // npgmup.dll Authentication Fail
        wsprintf ( msg, GlobalText[792], dwResult );
        break;

    case    NPGAMEMON_ERROR_AUTH_GAMEMON :  // GameMon.exe Authentication Fail
        wsprintf ( msg, GlobalText[792], dwResult );
        break;

    case    NPGAMEMON_ERROR_AUTH_NEWUP :    // npgmup.dll.new Auth Fail
        wsprintf ( msg, GlobalText[792], dwResult );
        break;

    case    NPGAMEMON_ERROR_DECRYPT :       // .ini File Decryption Fail
        wsprintf ( msg, GlobalText[792], dwResult );
        break;

    case    NPGAMEMON_ERROR_CORRUPT_INI :   // Corrupt ini file Error
        wsprintf ( msg, GlobalText[792], dwResult );
        break;

    case    NPGAMEMON_ERROR_CORRUPT_INI2 :  // Not match GameName in ini file Error
        wsprintf ( msg, GlobalText[792], dwResult );
        break;

    case    NPGAMEMON_ERROR_NFOUND_INI :    // ini File not Found
        wsprintf ( msg, GlobalText[792], dwResult );
        break;

    case    NPGAMEMON_ERROR_NFOUND_NPGMUP : // npgmup.dll not found
        wsprintf ( msg, GlobalText[792], dwResult );
        break;

    case    NPGAMEMON_ERROR_NFOUND_NEWUP :  // npgmup.dll.new not found
        wsprintf ( msg, GlobalText[792], dwResult );
        break;

    case    NPGAMEMON_ERROR_COMM :          // Communication Init Error
        wsprintf ( msg, GlobalText[792], dwResult );
        break;

    /*case    NPGAMEMON_ERROR_EXECUTE :       // GameMon Execute Error
        wsprintf ( msg, GlobalText[792], dwResult );
        break;*/

    case    NPGAMEMON_ERROR_NPGMUP :        // npgmup.dll Error
        wsprintf ( msg, GlobalText[792], dwResult );
        break;

    case    NPGAMEMON_ERROR_MOVE_INI :      // Move ini Error
        wsprintf ( msg, GlobalText[792], dwResult );
        break;

    case    NPGAMEMON_ERROR_MOVE_NEWUP :    // Move npgmup.dll.new Error
        wsprintf ( msg, GlobalText[792], dwResult );
        break;

    case    NPGAMEMON_ERROR_GAMEMON :       // GameMon Init Error
        wsprintf ( msg, GlobalText[792], dwResult );
        break;

    case    NPGAMEMON_ERROR_SPEEDCHECK :    // SpeedCheck Init Error
        wsprintf ( msg, GlobalText[792], dwResult );
        break;

    case    NPGAMEMON_ERROR_GAMEGUARD :     // GameGuard Init Error
        wsprintf ( msg, GlobalText[792], dwResult );
        break;

    default :
        //bShow = false;
        wsprintf ( msg, GlobalText[792], dwResult );
        break;
    }

    if ( bShow )
    {
        TCHAR   msg2[256];
        wsprintf ( msg2, "GameGuard Error : %lu", dwResult );

        MessageBox ( NULL, msg, msg2, MB_OK );
    }
}

}


//////////////////////////////////////////////////////////////////////////
//  Func    : NPGameMonCallBack
//////////////////////////////////////////////////////////////////////////
#ifdef GAMEGUARD_AUTH25
BOOL    CALLBACK NPGameMonCallback ( DWORD dwMsg, DWORD dwArg )
#else
bool    CALLBACK NPGameMonCallback ( DWORD dwMsg, DWORD dwArg )
#endif //GAMEGUARD_AUTH25
{
    TCHAR msg[128];
	int   x = 640/2; 
    int   y = 50;

    switch ( dwMsg )
    {
	case NPGAMEMON_GAMEHACK_DOUBT:	//"게임이나 게임가드가 변조되었습니다.";
		MessageBox ( g_hWnd, GlobalText[799], "Error", MB_OK );

        //  강제 종료.
        CloseHack ( g_hWnd, true );
        return false;

#ifdef GG_SERVER_AUTH
	case NPGAMEMON_CHECK_CSAUTH:
#ifdef _DEBUG
		char str[256];
		wsprintf( str, "NPGAMEMON_CHECK_CSAUTH - 결과 보내기 %x", dwArg);
		g_pChatListBox->AddText("", str, SEASON3B::TYPE_SYSTEM_MESSAGE);
#endif	// _DEBUG
#ifdef NP_LOG
		g_ErrorReport.Write("NPGameMonCallback() NPGAMEMON_CHECK_CSAUTH: SendAuth( %x)\r\n", dwArg);	
#endif	// NP_LOG
		SendAuth( dwArg);
		return true;
#endif	// GG_SERVER_AUTH

#ifdef NPROTECT_AUTH2
	case NPGAMEMON_CHECK_CSAUTH2:
		GG_AUTH_DATA AuthData;
		memcpy(&AuthData, (const void*)dwArg, sizeof(GG_AUTH_DATA));
#ifdef _DEBUG
		wsprintf( str, "NPGAMEMON_CHECK_CSAUTH2 - Index = %X, Value1=%X, Value2=%X, Value3 = %X", AuthData.dwIndex,
			AuthData.dwValue1, AuthData.dwValue2, AuthData.dwValue3);
		g_pChatListBox->AddText("", str, SEASON3B::TYPE_SYSTEM_MESSAGE);
#endif	// _DEBUG
#ifdef NP_LOG
		g_ErrorReport.Write("NPGameMonCallback() NPGAMEMON_CHECK_CSAUTH2: SendAuth() - Index=%X, Value1=%X, Value2=%X, Value3=%X\r\n",
			AuthData.dwIndex, AuthData.dwValue1, AuthData.dwValue2, AuthData.dwValue3);
#endif	// NP_LOG
		SendAuth( AuthData);
		return true;
#endif // NPROTECT_AUTH2
		
    case NPGAMEMON_COMM_ERROR :
    case NPGAMEMON_COMM_CLOSE :
        wsprintf ( msg, GlobalText[792], dwMsg );
        MessageBox ( g_hWnd, msg, "Error", MB_OK );

        //  강제 종료.
        CloseHack ( g_hWnd, true );
        return false;

    case NPGAMEMON_INIT_ERROR :
        wsprintf ( msg, GlobalText[792], dwArg );
        MessageBox ( g_hWnd, msg, "Error", MB_OK );

#ifdef FOR_WORK
        npGameGuard::showErrorMessage ( g_hWnd, dwArg );
#endif
        //  강제 종료.
        CloseHack ( g_hWnd, true );
        return false;

    case NPGAMEMON_SPEEDHACK :
        MessageBox ( g_hWnd, GlobalText[793], "Error", MB_OK );

#ifdef FOR_WORK
        npGameGuard::showErrorMessage ( g_hWnd, dwArg );
#endif
        //  강제 종료.
        CloseHack ( g_hWnd, true );
        return false;

    case NPGAMEMON_GAMEHACK_KILLED :
        wsprintf ( msg, GlobalText[794], dwMsg );
        MessageBox ( g_hWnd, msg, "Error", MB_OK );

		
#ifdef FOR_WORK
        npGameGuard::showErrorMessage ( g_hWnd, dwArg );
#endif
        //  강제 종료.
        CloseHack ( g_hWnd, true );
        return true;

    case NPGAMEMON_GAMEHACK_DETECT :
#ifdef PBG_MOD_GAMEGUARD_HANDLE
		// 핵프로그램 발생시 클라이언트 바로 종료하지 않는다
		if(!g_NPGameGuardHandle->IsNPHack())
		{
			g_NPGameGuardHandle->SetNPHack(true);
			g_NPGameGuardHandle->SetErrorMsgIndex(dwArg);
		}
#else //PBG_MOD_GAMEGUARD_HANDLE
        wsprintf ( msg, GlobalText[794], dwMsg );
        MessageBox ( g_hWnd, msg, "Error", MB_OK );

#ifdef FOR_WORK
        npGameGuard::showErrorMessage ( g_hWnd, dwArg );
#endif
#endif //PBG_MOD_GAMEGUARD_HANDLE
        //  강제 종료.
        CloseHack ( g_hWnd, true );
        return false;
    }

    return true;
}

#endif