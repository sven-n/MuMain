///////////////////////////////////////////////////////////////////////////////
// Scene들을 처리하는 함수
// 로딩화면, 접속화면, 케릭터 선택 화면, 게임 화면 등등
//
// *** 함수 레벨: 4
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UIManager.h"
#include "GuildCache.h"
#include "ZzzOpenglUtil.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzLodTerrain.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"
#include "ZzzTexture.h"
#include "ZzzOpenData.h"
#include "ZzzScene.h"
#include "ZzzEffect.h"
#include "ZzzAI.h"
#include "DSPlaySound.h"
#include "DIMouse.h"
#include "wsclientinline.h"
#include "SMD.h"
#include "Local.h"
#include "MatchEvent.h"
#include "PhysicsManager.h"
#include "./Utilities/Log/ErrorReport.h"
#include "CSQuest.h"
#include "PersonalShopTitleImp.h"
#include "uicontrols.h"
#include "GOBoid.h"
#include "GMHellas.h"
#ifdef NP_GAME_GUARD
	#include "ExternalObject/Nprotect/npGameGuard.h"
#endif // NP_GAME_GUARD
#include "CSItemOption.h"
#include "PvPSystem.h"
#include "GMBattleCastle.h"
#include "GMHuntingGround.h"
#include "GMAida.h"
#include "GMCrywolf1st.h"
#include "npcBreeder.h"
#ifdef PET_SYSTEM
	#include "CSPetSystem.h"
	#include "GIPetManager.h"
#endif// PET_SYSTEM
#include "CComGem.h"
#include "UIMapName.h"	// rozy
#include "./Time/Timer.h"
#include "Input.h"
#include "UIMng.h"
#include "LoadingScene.h"
#include "CDirection.h"
#include "GM_Kanturu_3rd.h"
#ifdef MOVIE_DIRECTSHOW
	#include <dshow.h>
	#include "MovieScene.h"
#endif // MOVIE_DIRECTSHOW
#ifdef ADD_MU_HOMEPAGE
	#include "iexplorer.h"
#endif //ADD_MU_HOMEPAGE
#include "Event.h"
#ifdef CSK_LH_DEBUG_CONSOLE
	#include "./Utilities/Log/muConsoleDebug.h"
#endif // CSK_LH_DEBUG_CONSOLE
#include "MixMgr.h"
#include "GameCensorship.h"
#include "GM3rdChangeUp.h"
#include "NewUISystem.h"
#include "NewUICommonMessageBox.h"
#include "PartyManager.h"
#include "w_CursedTemple.h"
#include "CameraMove.h"
// 맵 관련 include
#ifdef PSW_ADD_MAPSYSTEM
#include "w_MapHeaders.h"
#endif // PSW_ADD_MAPSYSTEM
#ifdef LDK_ADD_NEW_PETPROCESS
#include "w_PetProcess.h"
#endif //LDK_ADD_NEW_PETPROCESS
#ifdef YDG_ADD_CS5_PORTAL_CHARM
#include "PortalMgr.h"
#endif	// YDG_ADD_CS5_PORTAL_CHARM
#ifdef CSK_HACK_TEST
#include "HackTest.h"
#endif // CSK_HACK_TEST
#ifdef CSK_MOD_PROTECT_AUTO_V1
#include "ProtectAuto.h"
#endif // CSK_MOD_PROTECT_AUTO_V1
#ifdef LDK_ADD_GLOBAL_PORTAL_SYSTEM
#include "GlobalPortalSystem.h"
#endif //LDK_ADD_GLOBAL_PORTAL_SYSTEM
#ifdef PBG_ADD_CHARACTERSLOT
#include "Slotlimit.h"
#endif //PBG_ADD_CHARACTERSLOT
#ifdef KJH_ADD_SERVER_LIST_SYSTEM
#include "ServerListManager.h"
#endif // KJH_ADD_SERVER_LIST_SYSTEM
#ifdef LDK_ADD_SCALEFORM
#include "CGFxProcess.h"
#endif //LDK_ADD_SCALEFORM

#ifdef KJH_ADD_CHECK_RESOURCE_GUARD_BEFORE_LOADING
	#ifdef RESOURCE_GUARD
		#include "./ExternalObject/ResourceGuard/MuRGReport.h"
		#include "./ExternalObject/ResourceGuard/ResourceGuard.h"
	#endif // RESOURCE_GUARD
#endif // KJH_ADD_CHECK_RESOURCE_GUARD_BEFORE_LOADING

//-----------------------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
//  EXTERN.
//////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////
    extern CUITextInputBox * g_pSingleTextInputBox;
    extern CUITextInputBox * g_pSinglePasswdInputBox;
    extern int g_iChatInputType;
    extern BOOL g_bUseChatListBox;
    extern DWORD g_dwMouseUseUIID;  // 마우스가 윈도우 위에 있는가 (있으면 윈도우 ID)
    extern DWORD g_dwActiveUIID;	// 현재 작동중인 UI ID
    extern DWORD g_dwKeyFocusUIID;	
	extern CUIMapName* g_pUIMapName;	// rozy
	extern bool HighLight;
	extern CTimer*	g_pTimer;

#ifdef ADD_MU_HOMEPAGE
extern BOOL g_bUseWindowMode;
#endif //ADD_MU_HOMEPAGE
#ifdef MOVIE_DIRECTSHOW
	extern CMovieScene* g_pMovieScene;
#endif // MOVIE_DIRECTSHOW
	
// 펜릴 변수 관련
#ifndef CSK_FIX_FENRIR_RUN		// 정리할 때 지워야 하는 소스	
BOOL	g_bFenrir_Run = FALSE;
int		g_iFenrir_Run = 0;
#endif //! CSK_FIX_FENRIR_RUN	// 정리할 때 지워야 하는 소스

bool	g_bTimeCheck = false;
int 	g_iBackupTime = 0;

float	g_fMULogoAlpha = 0;		// 뮤 로고 알파값

///////////////////////////////////////////

#ifdef _PVP_ADD_MOVE_SCROLL
    extern CMurdererMove g_MurdererMove;
#endif	// _PVP_ADD_MOVE_SCROLL
    extern CGuildCache g_GuildCache;

extern float g_fSpecialHeight;

//////////////////////////////////////////////////////////////////////////
//  Global Variable.
//////////////////////////////////////////////////////////////////////////
short   g_shCameraLevel = 0;

#ifdef LDS_MR0_MOD_FIXEDFRAME_DELTA
typedef std::list<unsigned int>	LISTACCUMULATETIME;

LISTACCUMULATETIME	listAccumulateTime;
#endif // LDS_MR0_MOD_FIXEDFRAME_DELTA

//////////////////////////////////////////////////////////////////////////
//  Function.
//////////////////////////////////////////////////////////////////////////

#ifdef USE_SHADOWVOLUME
#include "ShadowVolume.h"
int g_iShadowMode = 0;
DWORD g_dwShadowModeLatest = 0;
void ShadowVolumeDoIt( void)
{
	if ( PressKey( '~'))
	{
		DWORD dwTick = GetTickCount();
		if ( dwTick - g_dwShadowModeLatest > 300)
		{
			g_iShadowMode = ( g_iShadowMode + 1) % 3;
			g_dwShadowModeLatest = dwTick;
		}
	}
	switch ( g_iShadowMode)
	{
	case 0:
		ShadeWithShadowVolumes();
		break;
	case 1:
		RenderShadowVolumesAsFrame();
		break;
	}
}
#endif //USE_SHADOWVOLUME


/*#ifdef _DEBUG
bool EnableEdit    = true;
#else
bool EnableEdit    = false;
#endif*/

int g_iLengthAuthorityCode = iLengthAuthorityCode[SELECTED_LANGUAGE];

char *szServerIpAddress = lpszServerIPAddresses[SELECTED_LANGUAGE];
//char *szServerIpAddress = "210.181.89.215";
WORD g_ServerPort = 44405;

#ifdef MOVIE_DIRECTSHOW
int  SceneFlag = MOVIE_SCENE;
#else // MOVIE_DIRECTSHOW
int  SceneFlag = WEBZEN_SCENE;
#endif // MOVIE_DIRECTSHOW

int  MoveSceneFrame = 0;

#ifdef ANTIHACKING_ENABLE
int g_iPrevFrameCount = 0;
BOOL g_bNewFrame = FALSE;
#endif //ANTIHACKING_ENABLE

#ifdef _PVP_DYNAMIC_SERVER_TYPE
BOOL g_bIsCurrentServerPvP = FALSE;
#endif	// _PVP_DYNAMIC_SERVER_TYPE

extern int g_iKeyPadEnable;


CPhysicsManager g_PhysicsManager;


char *g_lpszMp3[NUM_MUSIC] =
{
#ifdef KOREAN_WAV_USE
	"data\\music\\주점.mp3",
	"data\\music\\뮤테마.mp3",
	"data\\music\\성당.mp3",
	"data\\music\\데비아스.mp3",
	"data\\music\\노리아.mp3",
	"data\\music\\던젼.mp3",
#else
	"data\\music\\Pub.mp3",
	"data\\music\\Mutheme.mp3",
	"data\\music\\Church.mp3",
	"data\\music\\Devias.mp3",
	"data\\music\\Noria.mp3",
	"data\\music\\Dungeon.mp3",
#endif
	"data\\music\\atlans.mp3",
	"data\\music\\icarus.mp3",
	"data\\music\\tarkan.mp3",
	"data\\music\\lost_tower_a.mp3",
	"data\\music\\lost_tower_b.mp3",
	"data\\music\\kalima.mp3",
    "data\\music\\castle.mp3",
    "data\\music\\charge.mp3",
    "data\\music\\lastend.mp3",
	"data\\music\\huntingground.mp3",
	"data\\music\\Aida.mp3",
	"data\\music\\crywolf1st.mp3",
	"data\\music\\crywolf_ready-02.ogg",
	"data\\music\\crywolf_before-01.ogg",
	"data\\music\\crywolf_back-03.ogg",
	"data\\music\\main_theme.mp3",
	"data\\music\\kanturu_1st.mp3",
	"data\\music\\kanturu_2nd.mp3",
	"data\\music\\KanturuMayaBattle.mp3",
	"data\\music\\KanturuNightmareBattle.mp3",
	"data\\music\\KanturuTower.mp3",
	"data\\music\\BalgasBarrack.mp3",
	"data\\music\\BalgasRefuge.mp3",
	"data\\music\\cursedtemplewait.mp3",
	"data\\music\\cursedtempleplay.mp3",
	"data\\music\\elbeland.mp3",
	"data\\music\\login_theme.mp3",
	"data\\music\\SwampOfCalmness.mp3",
#ifdef CSK_ADD_MAP_ICECITY	
	"data\\music\\Raklion.mp3",
	"data\\music\\Raklion_Hatchery.mp3",
#endif // CSK_ADD_MAP_ICECITY
#ifdef YDG_ADD_MAP_SANTA_TOWN
	"data\\music\\Santa_Village.mp3",
#endif	// YDG_ADD_MAP_SANTA_TOWN
#ifdef YDG_ADD_MAP_DUEL_ARENA
	"data\\music\\DuelArena.mp3",
#endif	// YDG_ADD_MAP_DUEL_ARENA
#ifdef PBG_ADD_PKFIELD			//BGM음악
	"data\\music\\PK_Field.mp3",
#endif //PBG_ADD_PKFIELD
#ifdef LDK_ADD_MAP_EMPIREGUARDIAN1
	"data\\music\\ImperialGuardianFort.mp3",
#endif //LDK_ADD_MAP_EMPIREGUARDIAN1
#ifdef LDS_ADD_MAP_EMPIREGUARDIAN2
	"data\\music\\ImperialGuardianFort.mp3",
#endif //LDS_ADD_MAP_EMPIREGUARDIAN2
#ifdef LDK_ADD_MAP_EMPIREGUARDIAN3
	"data\\music\\ImperialGuardianFort.mp3",
#endif //LDK_ADD_MAP_EMPIREGUARDIAN3
#ifdef LDS_ADD_MAP_EMPIREGUARDIAN4
	"data\\music\\ImperialGuardianFort.mp3",
#endif //LDS_ADD_MAP_EMPIREGUARDIAN4
#ifdef YDG_ADD_DOPPELGANGER_SOUND
	"data\\music\\iDoppelganger.mp3",
#endif	// YDG_ADD_DOPPELGANGER_SOUND
#ifdef LDS_ADD_MAP_UNITEDMARKETPLACE
	"data\\music\\iDoppelganger.mp3",		// 우선 임시.
#endif // LDS_ADD_MAP_UNITEDMARKETPLACE
#ifdef ASG_ADD_MAP_KARUTAN
	"data\\music\\Karutan_A.mp3",
	"data\\music\\Karutan_B.mp3",
#endif	// ASG_ADD_MAP_KARUTAN
};
extern char Mp3FileName[256];

#define MAX_LENGTH_CMB	( 38)

DWORD   g_dwWaitingStartTick;
int     g_iRequestCount;

int     g_iMessageTextStart     = 0;
char    g_cMessageTextCurrNum   = 0;
char    g_cMessageTextNum       = 0;
int     g_iNumLineMessageBoxCustom;
char    g_lpszMessageBoxCustom[NUM_LINE_CMB][MAX_LENGTH_CMB];
int     g_iCustomMessageBoxButton[NUM_BUTTON_CMB][NUM_PAR_BUTTON_CMB];// ok, cancel // 사용여부, x, y, width, height

int     g_iCustomMessageBoxButton_Cancel[NUM_PAR_BUTTON_CMB];	// 마이너스 스탯 생성, 감소, 취소버튼에서 취소버튼으로 사용

int		g_iCancelSkillTarget	= 0;

#define NUM_LINE_DA		( 1)
int g_iCurrentDialogScript = -1;
int g_iNumAnswer = 0;
char g_lpszDialogAnswer[MAX_ANSWER_FOR_DIALOG][NUM_LINE_DA][MAX_LENGTH_CMB];

DWORD GenerateCheckSum2( BYTE *pbyBuffer, DWORD dwSize, WORD wKey);


///////////////////////////////////////////////////////////////////////////////
// mp3 배경음악 모두 끄는 함수
///////////////////////////////////////////////////////////////////////////////

void StopMusic()
{
	for ( int i = 0; i < NUM_MUSIC; ++i)
	{
		StopMp3( g_lpszMp3[i]);
	}
}


//////////////////////////////////////////////////////////////////////////
//  생성할수 있는 클래스수를 설정한다.
//////////////////////////////////////////////////////////////////////////
static BYTE g_byMaxClass = 0;
void SetCreateMaxClass ( BYTE MaxClass )
{
    g_byMaxClass = MaxClass;
}


//////////////////////////////////////////////////////////////////////////
//  
//////////////////////////////////////////////////////////////////////////
BYTE GetCreateMaxClass ( void ) 
{ 
    return g_byMaxClass;
}

///////////////////////////////////////////////////////////////////////////////
// 계정 또는 케릭터이름 입력시 욕 필터링하는 함수
///////////////////////////////////////////////////////////////////////////////

bool CheckAbuseFilter(char *Text, bool bCheckSlash)
{
	if (bCheckSlash == true)
	{
		if ( Text[0] == '/')
		{
			return false;
		}
	}

	int icntText = 0;
	char TmpText[2048];
	for( int i=0; i<(int)strlen(Text); ++i )
	{
		if ( Text[i]!=32 )
		{
			TmpText[icntText] = Text[i];
			icntText++;
		}
	}
	TmpText[icntText] = 0;

	for(int i=0;i<AbuseFilterNumber;i++)
	{
        if(FindText(TmpText,AbuseFilter[i]))
		{
			return true;
		}
	}
	return false;
}

bool CheckAbuseNameFilter(char *Text)
{
	int icntText = 0;
	char TmpText[256];
	for( int i=0; i<(int)strlen(Text); ++i )
	{
		if ( Text[i]!=32 )
		{
			TmpText[icntText] = Text[i];
			icntText++;
		}
	}
	TmpText[icntText] = 0;

	for(int i=0;i<AbuseNameFilterNumber;i++)
	{
		if(FindText(TmpText,AbuseNameFilter[i]))
		{
			return true;
		}
	}
	return false;
}

bool CheckSpecialText(char *Text);	// Local.cpp 에 있음

bool CheckName()
{
    if( CheckAbuseNameFilter(InputText[0]) || CheckAbuseFilter(InputText[0]) ||
		FindText(InputText[0]," ") || FindText(InputText[0],"　") ||
		FindText(InputText[0],".") || FindText(InputText[0],"·") || FindText(InputText[0],"∼") ||
		FindText(InputText[0],"Webzen") || FindText(InputText[0],"WebZen") || FindText(InputText[0],"webzen") ||  FindText(InputText[0],"WEBZEN") ||
		FindText(InputText[0],GlobalText[457]) || FindText(InputText[0],GlobalText[458]))
		return true;
	return false;
}

#ifdef MOVIE_DIRECTSHOW
void MovieScene(HDC hDC)
{
	if(g_pMovieScene->GetPlayNum() == 0)
	{
		g_pMovieScene->InitOpenGLClear(hDC);
		
		g_pMovieScene->Initialize_DirectShow(g_hWnd, MOVIE_FILE_WMV);
		
		if(g_pMovieScene->IsFile() == FALSE || g_pMovieScene->IsFailDirectShow() == TRUE)
		{
			g_pMovieScene->Destroy();
			SAFE_DELETE(g_pMovieScene);
			SceneFlag = WEBZEN_SCENE;
			return;	
		}

		g_pMovieScene->PlayMovie();

		if(g_pMovieScene->IsEndMovie())
		{
			g_pMovieScene->Destroy();
			SAFE_DELETE(g_pMovieScene);
			SceneFlag = WEBZEN_SCENE;
			return;
		}
		else
		{
#ifdef KWAK_FIX_KEY_STATE_RUNTIME_ERR
			if(SEASON3B::IsPress(VK_ESCAPE) == TRUE || SEASON3B::IsPress(VK_RETURN) == TRUE)
#else // KWAK_FIX_KEY_STATE_RUNTIME_ERR
			if(HIBYTE(GetAsyncKeyState(VK_ESCAPE))==128 || HIBYTE(GetAsyncKeyState(VK_RETURN))==128)
#endif // KWAK_FIX_KEY_STATE_RUNTIME_ERR
			{
				g_pMovieScene->Destroy();
				SAFE_DELETE(g_pMovieScene);
				SceneFlag = WEBZEN_SCENE;
				return;
			}
		}
	}
	else
	{
		g_pMovieScene->Destroy();
		SAFE_DELETE(g_pMovieScene);
		SceneFlag = WEBZEN_SCENE;
		return;
	}
}
#endif // MOVIE_DIRECTSHOW

///////////////////////////////////////////////////////////////////////////////
// 웹젠 로고 나오는 화면
///////////////////////////////////////////////////////////////////////////////

bool EnableMainRender = false;
extern int HeroKey;

void WebzenScene(HDC hDC)
{
#ifdef KJH_ADD_CHECK_RESOURCE_GUARD_BEFORE_LOADING
	#if defined RESOURCE_GUARD && !defined FOR_WORK
		
		char szModuleFileName[256];
		ResourceGuard::CResourceGuard RG101;
		CMuRGReport MuRGReport;
		RG101.AddReportObj(&MuRGReport);
	#ifdef _TEST_SERVER
		sprintf(szModuleFileName, "data\\local\\Gameguardtest.csr");
	#else  // _TEST_SERVER
		sprintf(szModuleFileName, "data\\local\\Gameguard.csr");
	#endif // _TEST_SERVER
		if(!RG101.CheckIntegrityResourceChecksumFile(szModuleFileName))
		{
			g_ErrorReport.Write("> ResourceGuard Error!!\r\n");
			SendMessage(g_hWnd,WM_DESTROY,0,0);
		}
	#endif // defined RESOURCE_GUARD && !defined FOR_WORK
#endif // KJH_ADD_CHECK_RESOURCE_GUARD_BEFORE_LOADING


#ifdef DO_PROFILING_FOR_LOADING
	g_pProfilerForLoading = new CProfiler();
	g_pProfilerForLoading->Initialize( TRUE, 
										"PROFILING_FOR_LOADING_RESULT.txt",
										EPROFILESORTING_WEIGHT, 
										EPROFILESORTING_DIRECTION_DESCENDING );
	
	g_pProfilerForLoading->RequestLiveProfiling();
	g_pProfilerForLoading->BeginTotal( FALSE );
	g_pProfilerForLoading->BeginUnit( EPROFILING_LOADING_TOTAL, PROFILING_LOADING_TOTAL );
#endif // DO_PROFILING_FOR_LOADING

	CUIMng& rUIMng = CUIMng::Instance();	// UI매니저의 인스턴스 얻음.

	OpenFont();
	ClearInput();

	// 타이틀 씬에 쓰일 텍스처 로드.
#ifdef PBG_ADD_NEWLOGO_IMAGECHANGE
// 테섭과 본섭의 데이터를 달리 두기위해 변경
	LoadBitmap("Interface\\New_lo_back_01.jpg", BITMAP_TITLE, GL_LINEAR);
	LoadBitmap("Interface\\New_lo_back_02.jpg", BITMAP_TITLE+1, GL_LINEAR);
#ifdef PBG_ADD_MU_LOGO
#ifdef PBG_ADD_MUBLUE_LOGO
	// 뮤 블루
	LoadBitmap("Interface\\MU_TITLE_Blue.tga", BITMAP_TITLE+2, GL_LINEAR);
#else //PBG_ADD_MUBLUE_LOGO
	LoadBitmap("Interface\\MU_TITLE.tga", BITMAP_TITLE+2, GL_LINEAR);
#endif //PBG_ADD_MUBLUE_LOGO
#else //PBG_ADD_MU_LOGO
	LoadBitmap("Interface\\New_lo_mu_logo.tga", BITMAP_TITLE+2, GL_LINEAR);
#endif //PBG_ADD_MU_LOGO
	LoadBitmap("Interface\\lo_121518.tga", BITMAP_TITLE+3, GL_LINEAR);
	LoadBitmap("Interface\\New_lo_webzen_logo.tga", BITMAP_TITLE+4, GL_LINEAR);
	LoadBitmap("Interface\\lo_lo.jpg", BITMAP_TITLE+5, GL_LINEAR, GL_REPEAT);
#else //PBG_ADD_NEWLOGO_IMAGECHANGE
	LoadBitmap("Interface\\lo_back_01.jpg", BITMAP_TITLE, GL_LINEAR);
	LoadBitmap("Interface\\lo_back_02.jpg", BITMAP_TITLE+1, GL_LINEAR);
	LoadBitmap("Interface\\lo_mu_logo.tga", BITMAP_TITLE+2, GL_LINEAR);
	LoadBitmap("Interface\\lo_121518.tga", BITMAP_TITLE+3, GL_LINEAR);
	LoadBitmap("Interface\\lo_webzen_logo.tga", BITMAP_TITLE+4, GL_LINEAR);
	LoadBitmap("Interface\\lo_lo.jpg", BITMAP_TITLE+5, GL_LINEAR, GL_REPEAT);
#endif //PBG_ADD_NEWLOGO_IMAGECHANGE
#ifdef PBG_ENGLISHLOGO_CHANGE
	LoadBitmap("Interface\\lo_webzen_logo_Eng.tga", BITMAP_TITLE+6, GL_LINEAR);
#endif //PBG_ENGLISHLOGO_CHANGE
#ifdef PBG_ADD_NEWLOGO_IMAGECHANGE
	LoadBitmap("Interface\\lo_back_s5_03.jpg", BITMAP_TITLE+6, GL_LINEAR);
	LoadBitmap("Interface\\lo_back_s5_04.jpg", BITMAP_TITLE+7, GL_LINEAR);
#ifndef KJH_ADD_EVENT_LOADING_SCENE				// #ifndef
#ifdef PBG_ADD_MU_LOGO
#ifdef PBG_ADD_MUBLUE_LOGO
	if(rand()%100 <= 70)
	{
		//새로운 블루 뮤
		LoadBitmap("Interface\\lo_back_im01_Blue.jpg", BITMAP_TITLE+8, GL_LINEAR);
		LoadBitmap("Interface\\lo_back_im02_Blue.jpg", BITMAP_TITLE+9, GL_LINEAR);	
		LoadBitmap("Interface\\lo_back_im03_Blue.jpg", BITMAP_TITLE+10, GL_LINEAR);
		LoadBitmap("Interface\\lo_back_im04_Blue.jpg", BITMAP_TITLE+11, GL_LINEAR);
		LoadBitmap("Interface\\lo_back_im05_Blue.jpg", BITMAP_TITLE+12, GL_LINEAR);
		LoadBitmap("Interface\\lo_back_im06_Blue.jpg", BITMAP_TITLE+13, GL_LINEAR);
	}
#else //PBG_ADD_MUBLUE_LOGO
	if(rand()%100 <= 70)
	{
		//새로운 뮤
		LoadBitmap("Interface\\lo_back_im01.jpg", BITMAP_TITLE+8, GL_LINEAR);
		LoadBitmap("Interface\\lo_back_im02.jpg", BITMAP_TITLE+9, GL_LINEAR);	
		LoadBitmap("Interface\\lo_back_im03.jpg", BITMAP_TITLE+10, GL_LINEAR);
		LoadBitmap("Interface\\lo_back_im04.jpg", BITMAP_TITLE+11, GL_LINEAR);
		LoadBitmap("Interface\\lo_back_im05.jpg", BITMAP_TITLE+12, GL_LINEAR);
		LoadBitmap("Interface\\lo_back_im06.jpg", BITMAP_TITLE+13, GL_LINEAR);
	}
#endif //PBG_ADD_MUBLUE_LOGO
	else
#endif //PBG_ADD_MU_LOGO
#endif // KJH_ADD_EVENT_LOADING_SCENE
	{
		//기존 이미지
		LoadBitmap("Interface\\lo_back_s5_im01.jpg", BITMAP_TITLE+8, GL_LINEAR);
		LoadBitmap("Interface\\lo_back_s5_im02.jpg", BITMAP_TITLE+9, GL_LINEAR);	
		LoadBitmap("Interface\\lo_back_s5_im03.jpg", BITMAP_TITLE+10, GL_LINEAR);
		LoadBitmap("Interface\\lo_back_s5_im04.jpg", BITMAP_TITLE+11, GL_LINEAR);
		LoadBitmap("Interface\\lo_back_s5_im05.jpg", BITMAP_TITLE+12, GL_LINEAR);
		LoadBitmap("Interface\\lo_back_s5_im06.jpg", BITMAP_TITLE+13, GL_LINEAR);
	}
#endif //PBG_ADD_NEWLOGO_IMAGECHANGE

	rUIMng.CreateTitleSceneUI();	// 타이틀 씬의 UI 생성.
	
	FogEnable = false;				// ?
	
	// 타이틀 씬 랜더 부분.
	::EnableAlphaTest();
	OpenBasicData(hDC);		// 기본 데이터 로딩 및 로딩게이지바 랜더.


	
	// 메인씬에서 사용할 UI 클래스들 생성
	g_pNewUISystem->LoadMainSceneInterface();

	CUIMng::Instance().RenderTitleSceneUI(hDC, 11, 11);

	// 타이틀 씬이 끝날때 부분.
	rUIMng.ReleaseTitleSceneUI();	// 타이틀 씬 UI Release.
	
	// 타이틀 씬에 쓰인 텍스처 삭제.
	DeleteBitmap(BITMAP_TITLE);
   	DeleteBitmap(BITMAP_TITLE+1);
	DeleteBitmap(BITMAP_TITLE+2);
   	DeleteBitmap(BITMAP_TITLE+3);
	DeleteBitmap(BITMAP_TITLE+4);
   	DeleteBitmap(BITMAP_TITLE+5);
#ifdef PBG_ENGLISHLOGO_CHANGE
	DeleteBitmap(BITMAP_TITLE+6);
#endif //PBG_ENGLISHLOGO_CHANGE
#ifdef PBG_ADD_NEWLOGO_IMAGECHANGE
	for(int i=6; i<14; ++i)
		DeleteBitmap(BITMAP_TITLE+i);
#endif //PBG_ADD_NEWLOGO_IMAGECHANGE

	g_ErrorReport.Write( "> Loading ok.\r\n");

	SceneFlag = LOG_IN_SCENE;	// 서버 선택 씬으로.

#ifdef DO_PROFILING_FOR_LOADING
	g_pProfilerForLoading->EndUnit( EPROFILING_LOADING_TOTAL );
	
	g_pProfilerForLoading->RequestKillProfiling();
	g_pProfilerForLoading->RequestGenerateResult();
	
	g_pProfilerForLoading->EndTotal();
	
	g_pProfilerForLoading->Release();
#endif // DO_PROFILING_FOR_LOADING
}

///////////////////////////////////////////////////////////////////////////////
// 게임내의 메세지 박스 처리 함수들
///////////////////////////////////////////////////////////////////////////////

int MenuStateCurrent = MENU_SERVER_LIST;
int MenuStateNext    = MENU_SERVER_LIST;
int DeleteGuildIndex = -1;

void DeleteCharacter()
{
	SelectedHero = -1;
	if (g_iChatInputType == 1)
	{
		g_pSinglePasswdInputBox->GetText(InputText[0]);
		g_pSinglePasswdInputBox->SetText(NULL);
		g_pSinglePasswdInputBox->SetState(UISTATE_HIDE);
	}
	SendRequestDeleteCharacter(CharactersClient[SelectedHero].ID,InputText[0]);

	MenuStateCurrent = MENU_DELETE_LEFT;
	MenuStateNext    = MENU_NEW_DOWN;
	PlayBuffer(SOUND_MENU01);
	
	ClearInput();
	InputEnable = false;
}
int  ErrorMessage = NULL;
int	 ErrorMessageNext = NULL;
extern bool g_bEnterPressed;

#ifdef PSW_BUGFIX_IME

bool IsEnterPressed() {
	return g_bEnterPressed;
}

void SetEnterPressed( bool enterpressed ) {
	g_bEnterPressed = enterpressed;
}

#endif //PSW_BUGFIX_IME

BOOL CheckOptionMouseClick(int iOptionPos_y, BOOL bPlayClickSound = TRUE)
{
	if (CheckMouseIn((640-120)/2, 30+iOptionPos_y, 120, 22) && MouseLButtonPush)
	{
		MouseLButtonPush = false;
		MouseUpdateTime = 0;
		MouseUpdateTimeMax = 6;
		if (bPlayClickSound == TRUE) PlayBuffer(SOUND_CLICK01);
		return TRUE;
	}
	return FALSE;
}

int SeparateTextIntoLines( const char *lpszText, char *lpszSeparated, int iMaxLine, int iLineSize)
{
	int iLine = 0;
	const char *lpLineStart = lpszText;
	char *lpDst = lpszSeparated;
	const char *lpSpace = NULL;
	int iMbclen = 0;
	for ( const char* lpSeek = lpszText; *lpSeek; lpSeek += iMbclen, lpDst += iMbclen)
	{
		iMbclen = _mbclen( ( unsigned char*)lpSeek);
		if ( iMbclen + ( int)( lpSeek - lpLineStart) >= iLineSize)
		{	// 꽉 찼다. 다음 줄로
			// 최근 공백이 가까우면 공백까지
			if ( lpSpace && ( int)( lpSeek - lpSpace) < min( 10, iLineSize / 2))
			{
				lpDst -= ( lpSeek - lpSpace - 1);
				lpSeek = lpSpace + 1;
			}

			lpLineStart = lpSeek;
			*lpDst = '\0';
			if ( iLine >= iMaxLine - 1)
			{	// 모든 줄을 다 썼다.
				break;
			}
			++iLine;
			lpDst = lpszSeparated + iLine * iLineSize;
			lpSpace = NULL;
		}

		// 한 글자 복사
		memcpy( lpDst, lpSeek, iMbclen);
		if ( *lpSeek == ' ')
		{
			lpSpace = lpSeek;
		}
	}
	// 끝
	*lpDst = '\0';

	return ( iLine + 1);
}

void SetEffectVolumeLevel ( int level )
{
	if(level > 9)
		level = 9;
	if(level < 0)
		level = 0;

	if(level == 0)
	{
		SetMasterVolume(-10000);
	}
	else
	{
		long vol = -2000*log10(10.f/float(level));
		SetMasterVolume(vol);
	}
}

void SetViewPortLevel ( int Wheel )
{
#ifdef KWAK_FIX_KEY_STATE_RUNTIME_ERR
	if(SEASON3B::IsPress(VK_CONTROL) == TRUE)
#else // KWAK_FIX_KEY_STATE_RUNTIME_ERR
    if ( (HIBYTE( GetAsyncKeyState(VK_CONTROL))==128) )
#endif // KWAK_FIX_KEY_STATE_RUNTIME_ERR
    {
        if ( Wheel>0 )
            g_shCameraLevel--;
        else if ( Wheel<0 )
            g_shCameraLevel++;

        MouseWheel = 0;

	    if ( g_shCameraLevel>4 )
		    g_shCameraLevel = 4;
	    if ( g_shCameraLevel<0 )
		    g_shCameraLevel = 0;
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void RenderInfomation3D()
{
	bool Success = false;

	if ( ( ( ErrorMessage==MESSAGE_TRADE_CHECK || ErrorMessage==MESSAGE_CHECK ) && AskYesOrNo==1 ) 
		|| ErrorMessage==MESSAGE_USE_STATE 
		|| ErrorMessage==MESSAGE_USE_STATE2) 
	{
		Success = true;
	}

    if ( ErrorMessage==MESSAGE_TRADE_CHECK && AskYesOrNo==5 )
	{
		Success = true;
	}
	if ( ErrorMessage==MESSAGE_PERSONALSHOP_WARNING ) 
	{
		Success = true;
	}

	if ( Success )
    {
#ifdef MR0
		VPManager::Enable();
#endif //MR0

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glViewport2(0,0,WindowWidth,WindowHeight);
        gluPerspective2(1.f,(float)(WindowWidth)/(float)(WindowHeight),CameraViewNear,CameraViewFar);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        GetOpenGLMatrix(CameraMatrix);
        EnableDepthTest();
        EnableDepthMask();

        float Width, Height;
        float x = (640-150)/2;
        float y;
        if ( ErrorMessage==MESSAGE_TRADE_CHECK )
        {
            y = 60+55;
        }
        else
        {
            y = 60+55;
        }
#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE
		if (AskYesOrNo==5)
		{
			y = 225;
		}
#endif

	    Width=40.f;Height=60.f;
		int iRenderType = ErrorMessage;
		if(AskYesOrNo == 5)
			iRenderType = MESSAGE_USE_STATE;
		switch( iRenderType )
		{
		case MESSAGE_USE_STATE :
		case MESSAGE_USE_STATE2 :
        case MESSAGE_PERSONALSHOP_WARNING :
            RenderItem3D(x,y,Width,Height,TargetItem.Type,TargetItem.Level,TargetItem.Option1,TargetItem.ExtOption,true);
			break;

		default :
            RenderItem3D(x,y,Width,Height,PickItem.Type,PickItem.Level,PickItem.Option1,PickItem.ExtOption,true);
			break;
		}

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		
		// 현재 카메라의 매트릭스를 가지고 MousePosition 업데이트
		UpdateMousePositionn();

#ifdef MR0
		VPManager::Disable();
#endif //MR0
    }
}

void RenderInfomation()
{
#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_RENDER_INFORMATION, PROFILING_RENDER_INFORMATION );
#endif // DO_PROFILING

	RenderNotices();
    
	CUIMng::Instance().Render();	// 윈도우들 렌더.

#ifdef LDK_ADD_SCALEFORM
	if(GFxProcess::GetInstancePtr()->GetUISelect() == 1)
	{
		GFxProcess::GetInstancePtr()->GFxRender();
	}
#endif //LDK_ADD_SCALEFORM

	if(SceneFlag == LOG_IN_SCENE || SceneFlag == CHARACTER_SCENE)
	{
		RenderCursor();
	}
	
    RenderInfomation3D();
#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_RENDER_INFORMATION );
#endif // DO_PROFILING
}

///////////////////////////////////////////////////////////////////////////////
// 거래 확인창.
///////////////////////////////////////////////////////////////////////////////
BOOL ShowCheckBox( int num, int index, int message )
{
	if ( message==MESSAGE_USE_STATE || message==MESSAGE_USE_STATE2)
    {
        char Name[50] = { 0, };
        if ( TargetItem.Type==ITEM_HELPER+15 )//  서클. ( 에너지/체력/민첩/힘/통솔 )
        {
            switch ( (TargetItem.Level>>3)&15 )
            {
            case 0:sprintf(Name,"%s", GlobalText[168] );break;
            case 1:sprintf(Name,"%s", GlobalText[169] );break;
            case 2:sprintf(Name,"%s", GlobalText[167] );break;
            case 3:sprintf(Name,"%s", GlobalText[166] );break;
			case 4:sprintf(Name,"%s", GlobalText[1900] );break;	// 통솔
            }
        }
#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE
		sprintf ( g_lpszMessageBoxCustom[0], "%s", Name );
#else	// SELECTED_LANGUAGE == LANGUAGE_JAPANESE

		if (message==MESSAGE_USE_STATE2)
			sprintf ( g_lpszMessageBoxCustom[0], "( %s%s )", Name, GlobalText[1901] );
		else
			sprintf ( g_lpszMessageBoxCustom[0], "( %s )", Name );
		
#endif	// SELECTED_LANGUAGE == LANGUAGE_JAPANESE

        num++;
        for ( int i=1; i<num; ++i )
        {
	        sprintf ( g_lpszMessageBoxCustom[i], GlobalText[index] );
        }
        g_iNumLineMessageBoxCustom = num;
    }
	else if ( message==MESSAGE_PERSONALSHOP_WARNING )
	{
		char szGold[256];
		ConvertGold(InputGold,szGold);
	    sprintf ( g_lpszMessageBoxCustom[0], GlobalText[index], szGold );

        for ( int i=1; i<num; ++i )
        {
	        sprintf ( g_lpszMessageBoxCustom[i], GlobalText[index+i] );
        }
        g_iNumLineMessageBoxCustom = num;
	}
    else if ( message==MESSAGE_CHAOS_CASTLE_CHECK )
    {
	    g_iNumLineMessageBoxCustom = 0;
        for ( int i=0; i<num; ++i )
        {
	        // 1. 텍스트 초기화
	        g_iNumLineMessageBoxCustom += SeparateTextIntoLines( GlobalText[index+i], g_lpszMessageBoxCustom[g_iNumLineMessageBoxCustom], NUM_LINE_CMB, MAX_LENGTH_CMB);
        }
    }
	else if ( message==MESSAGE_GEM_INTEGRATION3)
	{
		char tBuf[MAX_GLOBAL_TEXT_STRING];
		char tLines[2][30];
		for(int t = 0; t < 2; ++t) memset(tLines[t], 0, 20);
		g_iNumLineMessageBoxCustom = 0;
		if(COMGEM::isComMode())
		{
			if(COMGEM::m_cGemType == 0) sprintf(tBuf, GlobalText[1809], GlobalText[1806], COMGEM::m_cCount);
			else sprintf(tBuf, GlobalText[1809], GlobalText[1807], COMGEM::m_cCount);

			g_iNumLineMessageBoxCustom += SeparateTextIntoLines( tBuf, 
				tLines[g_iNumLineMessageBoxCustom], 2, 30);

			for(int t = 0; t < 2; ++t) strcpy(g_lpszMessageBoxCustom[t], tLines[t]);

			sprintf(g_lpszMessageBoxCustom[g_iNumLineMessageBoxCustom], GlobalText[1810], COMGEM::m_iValue);
			++g_iNumLineMessageBoxCustom;

		}
		else
		{
			int t_GemLevel = COMGEM::GetUnMixGemLevel()+1;
			if(COMGEM::m_cGemType == 0) sprintf(tBuf, GlobalText[1813], GlobalText[1806], t_GemLevel);
			else sprintf(tBuf, GlobalText[1813], GlobalText[1807], t_GemLevel);

			g_iNumLineMessageBoxCustom += SeparateTextIntoLines( tBuf, 
				tLines[g_iNumLineMessageBoxCustom], 2, 30);

			for(int t = 0; t < 2; ++t) strcpy(g_lpszMessageBoxCustom[t], tLines[t]);

			sprintf(g_lpszMessageBoxCustom[g_iNumLineMessageBoxCustom], GlobalText[1814], COMGEM::m_iValue);
			++g_iNumLineMessageBoxCustom;
		}
	}
	else if(message == MESSAGE_CANCEL_SKILL)
	{
		char tBuf[MAX_GLOBAL_TEXT_STRING];	
		sprintf(tBuf, "%s%s", SkillAttribute[index].Name, GlobalText[2046]);
		g_iNumLineMessageBoxCustom = SeparateTextIntoLines(tBuf, g_lpszMessageBoxCustom[0], 2, MAX_LENGTH_CMB);
		g_iCancelSkillTarget = index;
	}
    else
    {
        for ( int i=0; i<num; ++i )
        {
	        strcpy ( g_lpszMessageBoxCustom[i], GlobalText[index+i]);
        }
        g_iNumLineMessageBoxCustom = num;
    }

	ZeroMemory( g_iCustomMessageBoxButton, NUM_BUTTON_CMB * NUM_PAR_BUTTON_CMB * sizeof ( int) );

    int iOkButton[5]     = { 1,  21, 90, 70, 21};
    int iCancelButton[5] = { 3, 120, 90, 70, 21};

	if(message == MESSAGE_USE_STATE2)
	{
		iOkButton[1]	 = 22;	
		iOkButton[2]	 = 92;	// y
		iOkButton[3]     = 49;
		iOkButton[4]     = 16;

		iCancelButton[1] = 82;
		iCancelButton[2] = 92;	// y
		iCancelButton[3] = 49;
		iCancelButton[4] = 16;

		g_iCustomMessageBoxButton_Cancel[0] = 5;
		g_iCustomMessageBoxButton_Cancel[1] = 142;	// x
		g_iCustomMessageBoxButton_Cancel[2] = 92;	// y
		g_iCustomMessageBoxButton_Cancel[3] = 49;	// width
		g_iCustomMessageBoxButton_Cancel[4] = 16;	// height
	}

    if ( message==MESSAGE_CHAOS_CASTLE_CHECK )
    {
        iOkButton[2]     = 120;
        iCancelButton[2] = 120;
    }
	
    memcpy( g_iCustomMessageBoxButton[0], iOkButton, 5 * sizeof ( int));
    memcpy( g_iCustomMessageBoxButton[1], iCancelButton, 5 * sizeof ( int));

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// 계정 입력과 케릭터 선택 화면에서의 자동으로 카메라 움직여주는 함수
///////////////////////////////////////////////////////////////////////////////

int    CameraWalkCut;
int    CurrentCameraCount    = -1;
int    CurrentCameraWalkType = 0;
int    CurrentCameraNumber   = 0;
vec3_t CurrentCameraPosition;
vec3_t CurrentCameraAngle;
float  CurrentCameraWalkDelta[6]; 
float  CameraWalk[] = 
{
	 0.f,-1000.f,500.f,-80.f,0.f, 0.f,
	 0.f,-1100.f,500.f,-80.f,0.f, 0.f,
	 0.f,-1100.f,500.f,-80.f,0.f, 0.f,
     0.f,-1100.f,500.f,-80.f,0.f, 0.f,
	 0.f,-1100.f,500.f,-80.f,0.f, 0.f,
	200.f,-800.f,250.f,-87.f,0.f, -10.f,
};

void MoveCharacterCamera(vec3_t Origin,vec3_t Position,vec3_t Angle)
{
	vec3_t TransformPosition;
	CameraAngle[0] = 0.f;
	CameraAngle[1] = 0.f;
	CameraAngle[2] = Angle[2];
	float Matrix[3][4];
	AngleMatrix(CameraAngle,Matrix);
	VectorIRotate(Position,Matrix,TransformPosition);
	VectorAdd(Origin,TransformPosition,CameraPosition);
	CameraAngle[0] = Angle[0];
}

void MoveCamera()
{
#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_MOVE_CAMERA, PROFILING_MOVE_CAMERA );
#endif // DO_PROFILING

	if (CCameraMove::GetInstancePtr()->IsTourMode())
	{
#ifdef DO_PROFILING
		g_pProfiler->EndUnit( EPROFILING_MOVE_CAMERA );
#endif // DO_PROFILING
		return;
	}

    if(CurrentCameraCount == -1)
	{
		for(int i=0;i<3;i++)
		{
			CurrentCameraPosition[i] = CameraWalk[i  ];
			CurrentCameraAngle   [i] = CameraWalk[i+3];
		}
		CurrentCameraNumber = 1;
		CurrentCameraWalkType = 1;

		for(int i=0;i<3;i++)
		{
			CurrentCameraWalkDelta[i  ] = (CameraWalk[CurrentCameraNumber*6+i  ]-CurrentCameraPosition[i])/128;
			CurrentCameraWalkDelta[i+3] = (CameraWalk[CurrentCameraNumber*6+i+3]-CurrentCameraAngle   [i])/128;
		}
	}
	CurrentCameraCount++;
    if((CameraWalkCut==0 && CurrentCameraCount>=40) || (CameraWalkCut>0 && CurrentCameraCount>=128))
	{
        CurrentCameraCount = 0;
		if(CameraWalkCut==0)
		{
			CameraWalkCut = 1;
		}
		else
		{
            if(SceneFlag == LOG_IN_SCENE)
			{
				CurrentCameraNumber = rand()%4+1;
				CurrentCameraWalkType = rand()%2;
			}
			else
			{
      			CurrentCameraNumber = 5;
				CurrentCameraWalkType = 0;
			}
		}
		for(int i=0;i<3;i++)
		{
			CurrentCameraWalkDelta[i  ] = (CameraWalk[CurrentCameraNumber*6+i  ]-CurrentCameraPosition[i])/128;
			CurrentCameraWalkDelta[i+3] = (CameraWalk[CurrentCameraNumber*6+i+3]-CurrentCameraAngle   [i])/128;
		}
	}
	if(CurrentCameraWalkType==0)
	{
    	for(int i=0;i<3;i++)
		{
			CurrentCameraPosition[i] += (CameraWalk[CurrentCameraNumber*6+i  ]-CurrentCameraPosition[i])/6;
			CurrentCameraAngle   [i] += (CameraWalk[CurrentCameraNumber*6+i+3]-CurrentCameraAngle   [i])/6;
		}
	}
	else
	{
    	for(int i=0;i<2;i++)
		{
			CurrentCameraPosition[i] += CurrentCameraWalkDelta[i  ];
		}
	}
    CameraFOV = 45.f;
	vec3_t Position;
	Vector(0.f,0.f,0.f,Position);
    MoveCharacterCamera(Position,CurrentCameraPosition,CurrentCameraAngle);

#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_MOVE_CAMERA );
#endif // DO_PROFILING
}

///////////////////////////////////////////////////////////////////////////////
// 서버리스트 선택하는 화면
///////////////////////////////////////////////////////////////////////////////

#ifndef KJH_ADD_SERVER_LIST_SYSTEM			// #ifndef
SERVER_LIST_t ServerList[MAX_SERVER_HI];
WORD ServerNumber = 0;		// 서버 총 개수.
int  ServerSelectHi = -1;
int  ServerSelectLow = -1;
int  ServerLocalSelect = -1;

// 서버 그룹 이름으로 서버 그룹 버튼 번호를 리턴.(좌우측 구분) - 구버전 함수.
int SearchServer(char *Name, bool extServer)
{
	int Count = 0;
	for(int i=0;i<MAX_SERVER_HI;i++)
	{
		if(strcmp(ServerList[i].Name,Name)==NULL)
			return Count;
		if(ServerList[i].Number > 0 && ServerList[i].extServer==extServer )
			Count++;
	}
	return 0;
}

int ServerConnectCount = 0;
#endif // KJH_ADD_SERVER_LIST_SYSTEM

///////////////////////////////////////////////////////////////////////////////
// 계정 입력 받는 화면
///////////////////////////////////////////////////////////////////////////////

bool MenuCancel         = true;
bool EnableSocket       = false;
bool InitLogIn          = false;
bool InitLoading        = false;
bool InitCharacterScene = false;
bool InitMainScene      = false;
int  MenuY = 480;
int  MenuX = -200;

//  계정아이디.
extern char LogInID[MAX_ID_SIZE+1];
extern char m_ExeVersion[11];

BOOL Util_CheckOption( char *lpszCommandLine, unsigned char cOption, char *lpszString);

///////////////////////////////////////////////////////////////////////////////
// 케릭터 선택 화면
///////////////////////////////////////////////////////////////////////////////

extern DWORD g_dwBKConv;
extern DWORD g_dwBKSent;
extern BOOL g_bIMEBlock;

int SelectedHero = -1;
bool MoveMainCamera();

void StartGame(
#ifdef PJH_CHARACTER_RENAME
			   bool IsConnect
#endif //#ifdef PJH_CHARACTER_RENAME
			   )
{
#ifdef PJH_CHARACTER_RENAME
	if(IsConnect == false)
	{
		//CharactersClient[SelectedHero].Class
		SendRequestCheckChangeName(CharactersClient[SelectedHero].ID);
	}
	else
#endif //PJH_CHARACTER_RENAME
	{
		// 캐릭터 블럭이 아닐때만 접속 가능
		if (CTLCODE_01BLOCKCHAR & CharactersClient[SelectedHero].CtlCode)
			CUIMng::Instance().PopUpMsgWin(MESSAGE_BLOCKED_CHARACTER);
		else
		{
			CharacterAttribute->Level = CharactersClient[SelectedHero].Level;
			CharacterAttribute->Class = CharactersClient[SelectedHero].Class;
			CharacterAttribute->Skin  = CharactersClient[SelectedHero].Skin;
			::strcpy(CharacterAttribute->Name, CharactersClient[SelectedHero].ID);

			::ReleaseCharacterSceneData();
			InitLoading = false;
			SceneFlag = LOADING_SCENE;
		}
	}
}

void CreateCharacterScene()
{
	g_pNewUIMng->ResetActiveUIObj();

	EnableMainRender = true;
	MouseOnWindow = false;	// ESC 키 눌렀을때 종료 옵션 메뉴 초기화 
	ErrorMessage = NULL;	// ESC 키 눌렀을때 종료 옵션 메뉴 초기화 

#ifdef PJH_NEW_SERVER_SELECT_MAP
	World = WD_74NEW_CHARACTER_SCENE;
#else //PJH_NEW_SERVER_SELECT_MAP
	World = WD_78NEW_CHARACTER_SCENE;
#endif //PJH_NEW_SERVER_SELECT_MAP

	OpenWorld(World);
    OpenCharacterSceneData();

    CreateCharacterPointer(&CharacterView,MODEL_FACE+1,0,0);
	CharacterView.Class = 1;
	CharacterView.Object.Kind = 0;

	//초기화
	SelectedHero = -1;
	CUIMng::Instance().CreateCharacterScene();	// 캐릭터 선택 씬 UI 생성.

    ClearInventory();
    CharacterAttribute->SkillNumber = 0;
#ifdef KWAK_FIX_CHARACTER_SKILL_RUNTIME_ERR
	for(int i = 0; i < MAX_SKILLS; i++)
#else // KWAK_FIX_CHARACTER_SKILL_RUNTIME_ERR
	for(int i=0;i<MAX_MAGIC;i++)
#endif // KWAK_FIX_CHARACTER_SKILL_RUNTIME_ERR
		CharacterAttribute->Skill[i] = 0;

	for(int i=EQUIPMENT_WEAPON_RIGHT;i<EQUIPMENT_HELPER;i++)
		CharacterMachine->Equipment[i].Level = 0;

	g_pNewUISystem->HideAll();
#ifdef PBG_ADD_CHARACTERSLOT
	// 블루뮤 서버일 경우만 CharacterSlotCount값을 사용한다
	if(BLUE_MU::IsBlueMuServer())
		g_SlotLimit->CreateSlotLock();
#endif //PBG_ADD_CHARACTERSLOT
	g_iKeyPadEnable = 0;
	GuildInputEnable = false;
	TabInputEnable   = false;
	GoldInputEnable  = false;
	InputEnable      = true;
    ClearInput();
	InputIndex = 0;
    InputTextWidth = 90;
    InputNumber = 1;

	for(int i=0;i<MAX_WHISPER;i++)
	{
		g_pChatListBox->AddText("", "", SEASON3B::TYPE_WHISPER_MESSAGE);
	}

	HIMC hIMC = ImmGetContext(g_hWnd);
    DWORD Conversion, Sentence;

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	int iSelectedLanguage = SELECTED_LANGUAGE;
	if(iSelectedLanguage == LANGUAGE_JAPANESE)
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
    if (SELECTED_LANGUAGE == LANGUAGE_JAPANESE)
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
    {
        Conversion = 0x19;
        Sentence   = IME_SMODE_PHRASEPREDICT;
    }
	else
	{
		Conversion = IME_CMODE_NATIVE;
		Sentence = IME_SMODE_NONE;
	}

	// IME 상태를 영문으로
	g_bIMEBlock = FALSE;
	RestoreIMEStatus();
	ImmSetConversionStatus(hIMC, Conversion, Sentence);
	ImmGetConversionStatus(hIMC, &g_dwBKConv, &g_dwBKSent);
	SaveIMEStatus();
	ImmReleaseContext(g_hWnd, hIMC);
	g_bIMEBlock = TRUE;

    g_ErrorReport.Write( "> Character scene init success.\r\n");
}

void NewMoveCharacterScene()
{
#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_MOVE_CHARACTERSCENE_TOTAL, PROFILING_MOVE_CHARACTERSCENE_TOTAL );
#endif // DO_PROFILING
	if (CurrentProtocolState < RECEIVE_CHARACTERS_LIST)
	{
#ifdef DO_PROFILING
		g_pProfiler->EndUnit( EPROFILING_MOVE_CHARACTERSCENE_TOTAL );
#endif // DO_PROFILING	
		return;
	}

	if (!InitCharacterScene)
	{
		InitCharacterScene = true;
		CreateCharacterScene();
	}
    InitTerrainLight();

#ifdef DO_PROFILING
	MoveInterface_ForProfiling();
#endif // DO_PROFILING

    MoveObjects();
	MoveBugs();
    MoveCharactersClient();
    MoveCharacterClient(&CharacterView);

	MoveEffects();
    MoveJoints();
    MoveParticles();
	MoveBoids();

#ifdef LDK_ADD_NEW_PETPROCESS
	ThePetProcess().UpdatePets();
#endif //LDK_ADD_NEW_PETPROCESS

    MoveCamera();

#if defined _DEBUG || defined FOR_WORK
	char lpszTemp[256];
	if (::Util_CheckOption(::GetCommandLine(), 'c', lpszTemp))
	{
		SelectedHero = ::atoi(lpszTemp);
		::StartGame();
	}
#endif


#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_MOVE_CHARACTERSCENE_INPUTEVENTS, PROFILING_MOVE_CHARACTERSCENE_INPUTEVENTS );
#endif // DO_PROFILING	

	CInput& rInput = CInput::Instance();
	CUIMng& rUIMng = CUIMng::Instance();

	if (rInput.IsKeyDown(VK_RETURN))	// 엔터키를 눌렀는가?
	{
		// 메시지 창, 캐릭터 생성 창 등등 보이지 않으며 캐릭터가 선택되어 있다면.
		if (!(rUIMng.m_MsgWin.IsShow() || rUIMng.m_CharMakeWin.IsShow()
			|| rUIMng.m_SysMenuWin.IsShow() || rUIMng.m_OptionWin.IsShow())
			&& SelectedHero > -1 && SelectedHero < 5)
		{
			::PlayBuffer(SOUND_CLICK01);	// 클릭 사운드.
#ifdef PBG_FIX_CHARACTERSELECTINDEX
			if(SelectedCharacter >= 0)		// 캐릭터 선택이 아닐경우만
#endif //PBG_FIX_CHARACTERSELECTINDEX
#ifdef LJH_FIX_LOADING_INFO_OF_AN_OTHER_CHARACTER_IN_LOGIN
			SelectedHero = SelectedCharacter;
#endif //LJH_FIX_LOADING_INFO_OF_AN_OTHER_CHARACTER_IN_LOGIN
			::StartGame();
		}
	}
	else if (rInput.IsKeyDown(VK_ESCAPE))	// Esc키를 눌렀는가?
	{
		// 메시지 창, 캐릭터 생성 창 등등 보이지 않으면.
		if (!(rUIMng.m_MsgWin.IsShow() || rUIMng.m_CharMakeWin.IsShow()
			|| rUIMng.m_SysMenuWin.IsShow() || rUIMng.m_OptionWin.IsShow()
			)
			&& rUIMng.IsSysMenuWinShow() )
		{
			::PlayBuffer(SOUND_CLICK01);	// 클릭 사운드.
			rUIMng.ShowWin(&rUIMng.m_SysMenuWin);
		}
	}

	if (rUIMng.IsCursorOnUI())	// UI 뒤의 캐릭터가 클릭되면 안되므로.
	{
#ifdef DO_PROFILING
		g_pProfiler->EndUnit( EPROFILING_MOVE_CHARACTERSCENE_TOTAL );
#endif // DO_PROFILING	
		return;
	}

	// 마우스 왼쪽 버튼 더블 클릭인가?
	// 캐릭터 선택 씬 메인 창이 보이지 않는다는 것은 로그인씬으로 변경한다는 뜻.
	if (rInput.IsLBtnDbl() && rUIMng.m_CharSelMainWin.IsShow())
	{
		if (SelectedCharacter < 0 || SelectedCharacter > 4)
		{
#ifdef DO_PROFILING
			g_pProfiler->EndUnit( EPROFILING_MOVE_CHARACTERSCENE_TOTAL );
#endif // DO_PROFILING	
			return;
		}

		SelectedHero = SelectedCharacter;
		::StartGame();
	}
	else if(rInput.IsLBtnDn())	// 마우스 왼쪽 버튼 클릭인가?
	{
		if (SelectedCharacter < 0 || SelectedCharacter > 4)
			SelectedHero = -1;
		else
			SelectedHero = SelectedCharacter;
		rUIMng.m_CharSelMainWin.UpdateDisplay();
	}

#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_MOVE_CHARACTERSCENE_INPUTEVENTS );
#endif // DO_PROFILING	

#ifdef CSK_LH_DEBUG_CONSOLE
	g_ConsoleDebug->UpdateMainScene();
#endif // CSK_LH_DEBUG_CONSOLE

#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_MOVE_CHARACTERSCENE_TOTAL );
#endif // DO_PROFILING	
}

bool NewRenderCharacterScene(HDC hDC)
{
#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_RENDER_CHARACTERSCENE_TOTAL, PROFILING_RENDER_CHARACTERSCENE_TOTAL );
#endif // DO_PROFILING
	
	if(!InitCharacterScene) 
	{
#ifdef DO_PROFILING
		g_pProfiler->EndUnit( EPROFILING_RENDER_CHARACTERSCENE_TOTAL );
#endif // DO_PROFILING
		return false;
	}
	if(CurrentProtocolState < RECEIVE_CHARACTERS_LIST) 
	{
#ifdef DO_PROFILING
		g_pProfiler->EndUnit( EPROFILING_RENDER_CHARACTERSCENE_TOTAL );
#endif // DO_PROFILING
		return false;
	}

    FogEnable = false;
	vec3_t pos;
#ifdef PJH_NEW_SERVER_SELECT_MAP
	Vector(9758.0f, 18913.0f, 675.0f, pos);
#else //PJH_NEW_SERVER_SELECT_MAP
	Vector(23566.0f, 14085.0f, 395.0f, pos);
#endif //PJH_NEW_SERVER_SELECT_MAP

    MoveMainCamera();

	int Width,Height;

	glColor3f(1.f,1.f,1.f);
#ifndef PJH_NEW_SERVER_SELECT_MAP
	BeginBitmap();
		Width = 320;
		Height = 320;
		RenderBitmap(BITMAP_LOG_IN+9,  (float)0,(float)25,(float)Width,(float)Height,0.f,0.f);
		RenderBitmap(BITMAP_LOG_IN+10,(float)320,(float)25,(float)Width,(float)Height,0.f,0.f);
	EndBitmap();
#endif //PJH_NEW_SERVER_SELECT_MAP
	Height = 480;
	Width = GetScreenWidth();
    
	glClearColor(0.f,0.f,0.f,1.f);
	BeginOpengl(0,25,640,430);
	
	CreateFrustrum((float)Width/(float)640, pos);

	OBJECT *o = &CharactersClient[SelectedHero].Object;

	CreateScreenVector(MouseX,MouseY,MouseTarget);
	for(int i = 0; i < 5; i++)
	{
		CharactersClient[i].Object.Position[2] = 163.0f;
		Vector ( 0.0f, 0.0f, 0.0f, CharactersClient[i].Object.Light );
	}

	if(SelectedHero!=-1 && o->Live)	// 그리는 순서 때문에 ㅠㅠ
	{
		EnableAlphaBlend();
		vec3_t Light;
		Vector ( 1.0f, 1.0f, 1.0f, Light );
		Vector ( 1.0f, 1.0f, 1.0f, o->Light );
		AddTerrainLight(o->Position[0],o->Position[1],Light,1,PrimaryTerrainLight);
		DisableAlphaBlend();
	}

#ifdef MR0
	if(EngineGate::IsOn() && g_pMeshMachine->bEnabled())
		g_pMeshMachine->ProcessLight(HighLight);
#endif //MR0

	CHARACTER* pCha = NULL;
	OBJECT* pObj = NULL;

	for(int i=0; i<5; ++i)
	{
		pCha = &CharactersClient[i];
		pObj = &pCha->Object;
		if(pCha->Helper.Type == MODEL_HELPER+3)	// 디노란트
		{
#ifdef PJH_NEW_SERVER_SELECT_MAP
			pObj->Position[2] = 194.5f;
#else //PJH_NEW_SERVER_SELECT_MAP
			pObj->Position[2] = 55.0f;
#endif //PJH_NEW_SERVER_SELECT_MAP
		}
		else
		{
#ifdef PJH_NEW_SERVER_SELECT_MAP
			pObj->Position[2] = 169.5f;
#else //PJH_NEW_SERVER_SELECT_MAP
			pObj->Position[2] = 30.0f;
#endif //PJH_NEW_SERVER_SELECT_MAP
		}
	}

	RenderTerrain(false);
	RenderObjects();
	RenderCharactersClient();

	if(!CUIMng::Instance().IsCursorOnUI())
		SelectObjects();

	RenderBugs();
	RenderBlurs();
	RenderJoints();
	RenderEffects();
#ifdef LDK_ADD_NEW_PETPROCESS
	ThePetProcess().RenderPets();
#endif //LDK_ADD_NEW_PETPROCESS
	RenderBoids();
	RenderObjects_AfterCharacter();	// 캐릭터 다음에 알파 오브젝트들 묘사

	CheckSprites();

	if(SelectedHero!=-1 && o->Live)	// 그리는 순서 때문에
	{
		vec3_t vLight;
		// 오로라 효과
		//Vector ( 0.3f, 0.2f, 1.f, vLight );
		//RenderAurora ( BITMAP_MAGIC+1, RENDER_BRIGHT, o->Position[0], o->Position[1], 2.5f, 2.5f, vLight );
		
		Vector ( 1.0f, 1.0f, 1.f, vLight );
		float fLumi = sinf ( WorldTime*0.0015f )*0.3f+0.5f;
		Vector ( fLumi*vLight[0], fLumi*vLight[1], fLumi*vLight[2], vLight );
#ifdef PJH_NEW_SERVER_SELECT_MAP
		EnableAlphaBlend();
		RenderTerrainAlphaBitmap(BITMAP_GM_AURORA, o->Position[0], o->Position[1], 1.8f, 1.8f, vLight, WorldTime*0.01f);
		RenderTerrainAlphaBitmap(BITMAP_GM_AURORA, o->Position[0], o->Position[1], 1.2f, 1.2f, vLight, -WorldTime*0.01f);
		DisableAlphaBlend();
#else //PJH_NEW_SERVER_SELECT_MAP
		RenderTerrainAlphaBitmap ( BITMAP_GM_AURORA, o->Position[0], o->Position[1], 1.5f, 1.5f, vLight, WorldTime*0.01f );
		RenderTerrainAlphaBitmap ( BITMAP_GM_AURORA, o->Position[0], o->Position[1], 1.f, 1.f, vLight, -WorldTime*0.01f );
#endif //PJH_NEW_SERVER_SELECT_MAP

		//CreateParticle(BITMAP_FLARE+1,o->Position,o->Angle,Light,0,0.15f);

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
		float Rotation = (int)WorldTime%3600/(float)10.f;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
		Vector ( 0.15f, 0.15f, 0.15f, o->Light );
		CreateParticle(BITMAP_EFFECT, o->Position, o->Angle, o->Light, 4);
		CreateParticle(BITMAP_EFFECT, o->Position, o->Angle, o->Light, 5);

		g_csMapServer.SetHeroID ( (char *)CharactersClient[SelectedHero].ID );
	}

	BeginSprite();
	RenderSprites();
	RenderParticles();
	RenderPoints();
	EndSprite();

	BeginBitmap();

	RenderInfomation();

#ifdef ENABLE_EDIT
	RenderDebugWindow();
#endif //ENABLE_EDIT

	EndBitmap();

	EndOpengl();

#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_RENDER_CHARACTERSCENE_TOTAL );
#endif // DO_PROFILING

	return true;
}

// 로그인 씬 생성.
// 원래 NewMoveLogInScene()함수 안에 있던 내용 중 초기화 부분임.
void CreateLogInScene()
{
	EnableMainRender = true;
#ifdef PJH_NEW_SERVER_SELECT_MAP
	World = WD_73NEW_LOGIN_SCENE;
#else
	World = WD_77NEW_LOGIN_SCENE;
#endif //PJH_NEW_SERVER_SELECT_MAP
	OpenWorld(World);

	OpenLogoSceneData();

	// 로그인 씬 UI 생성.
	// 아래 CreateSocket() 에러 처리를 위해 먼저 호출해야함.
	CUIMng::Instance().CreateLoginScene();

	CurrentProtocolState = REQUEST_JOIN_SERVER;
    CreateSocket(szServerIpAddress,g_ServerPort);
    EnableSocket = true;

#ifndef KJH_ADD_SERVER_LIST_SYSTEM		// #ifndef
	ServerNumber = 0;
	
	//초기화
	ServerSelectHi = -1;
	ServerSelectLow = -1;
	ServerConnectCount = 0;
#endif // KJH_ADD_SERVER_LIST_SYSTEM
	
	GuildInputEnable = false;
	TabInputEnable   = false;
	GoldInputEnable  = false;
	InputEnable      = true;
	ClearInput();

	if (g_iChatInputType == 0)
	{
		strcpy(InputText[0],m_ID);
		InputLength[0] = strlen(InputText[0]);
		InputTextMax[0] = MAX_ID_SIZE;
		if(InputLength[0] == 0)	InputIndex = 0;
		else InputIndex = 1;
	}
	InputNumber = 2;
    InputTextHide[1] = 1;

	CCameraMove::GetInstancePtr()->PlayCameraWalk(Hero->Object.Position, 1000);
#ifdef PJH_NEW_SERVER_SELECT_MAP
	// 시작위치 랜덤이 아닌 고정으로
	CCameraMove::GetInstancePtr()->SetTourMode(TRUE, FALSE, 1);
#else //PJH_NEW_SERVER_SELECT_MAP
	CCameraMove::GetInstancePtr()->SetTourMode(TRUE, TRUE);
#endif //PJH_NEW_SERVER_SELECT_MAP
	
	MoveMainCamera();

	g_fMULogoAlpha = 0;
	
	::PlayMp3(g_lpszMp3[MUSIC_LOGIN_THEME]);

	g_ErrorReport.Write( "> Login Scene init success.\r\n");
}

void NewMoveLogInScene()
{
	if(!InitLogIn)
	{
		InitLogIn = true;
		CreateLogInScene();
	}

#ifdef MOVIE_DIRECTSHOW
	if(CUIMng::Instance().IsMoving() == true)
	{
		return;
	}
#endif // MOVIE_DIRECTSHOW
	// 크레딧이 보이면 연산하지 않음.
	if (!CUIMng::Instance().m_CreditWin.IsShow())
	{
		InitTerrainLight();

		MoveObjects();
		MoveBugs();
		MoveLeaves();
		MoveCharactersClient();

		MoveEffects();
		MoveJoints();
		MoveParticles();
		MoveBoids();

#ifdef LDK_ADD_NEW_PETPROCESS
	ThePetProcess().UpdatePets();
#endif //LDK_ADD_NEW_PETPROCESS

#ifdef DO_PROFILING
		MoveInterface_ForProfiling();
#endif // DO_PROFILING

		MoveCamera();
	}

	if (CInput::Instance().IsKeyDown(VK_ESCAPE))	// Esc키를 눌렀는가?
	{
		CUIMng& rUIMng = CUIMng::Instance();
		// 메시지 창, 로그인 창 등등 보이지 않고
		//로그인씬 메인 창, 서버 선택창이 보이면.
		if (!(rUIMng.m_MsgWin.IsShow() || rUIMng.m_LoginWin.IsShow()
			|| rUIMng.m_SysMenuWin.IsShow() || rUIMng.m_OptionWin.IsShow()
			|| rUIMng.m_CreditWin.IsShow()
			)
			&& rUIMng.m_LoginMainWin.IsShow() && rUIMng.m_ServerSelWin.IsShow()
			&& rUIMng.IsSysMenuWinShow())
		{
			::PlayBuffer(SOUND_CLICK01);	// 클릭 사운드.
			rUIMng.ShowWin(&rUIMng.m_SysMenuWin);
		}
	}
	if (RECEIVE_LOG_IN_SUCCESS == CurrentProtocolState)
	{
		g_ErrorReport.Write( "> Request Character list\r\n");

		CCameraMove::GetInstancePtr()->SetTourMode(FALSE);

		//로그인 성공
		SceneFlag = CHARACTER_SCENE;
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		SendRequestCharactersList(g_pMultiLanguage->GetLanguage());
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		SendRequestCharactersList();
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE

#ifdef NP_GAME_GUARD
        npGameGuard::loginID( LogInID );
#endif
        ReleaseLogoSceneData();

		ClearCharacters();
	}

#ifdef CSK_LH_DEBUG_CONSOLE
	g_ConsoleDebug->UpdateMainScene();
#endif // CSK_LH_DEBUG_CONSOLE
}

bool NewRenderLogInScene(HDC hDC)
{
	if(!InitLogIn) return false;

	FogEnable = false;
// 	extern GLfloat FogColor[4];
// 	FogColor[0] = 178.f/256.f; FogColor[1] = 178.f/256.f; FogColor[2] = 178.f/256.f; FogColor[3] = 0.f;
// 	glFogf(GL_FOG_START, 3700.0f);
// 	glFogf(GL_FOG_END, 4000.0f);

#ifdef MOVIE_DIRECTSHOW
	if(CUIMng::Instance().IsMoving() == true)
	{
		g_pMovieScene->PlayMovie();

		if(g_pMovieScene->IsEndMovie())
		{
			g_pMovieScene->Destroy();
			SAFE_DELETE(g_pMovieScene);
			CUIMng::Instance().SetMoving(false);
			::PlayMp3(g_lpszMp3[MUSIC_MAIN_THEME]);
		}
		else
		{
#ifdef KWAK_FIX_KEY_STATE_RUNTIME_ERR
			if(SEASON3B::IsPress(VK_ESCAPE) == TRUE || SEASON3B::IsPress(VK_RETURN) == TRUE)
#else // KWAK_FIX_KEY_STATE_RUNTIME_ERR
			if(HIBYTE(GetAsyncKeyState(VK_ESCAPE))==128 || HIBYTE(GetAsyncKeyState(VK_RETURN))==128)
#endif // KWAK_FIX_KEY_STATE_RUNTIME_ERR
			{
				g_pMovieScene->Destroy();
				SAFE_DELETE(g_pMovieScene);
				CUIMng::Instance().SetMoving(false);
				::PlayMp3(g_lpszMp3[MUSIC_MAIN_THEME]);
			}
		}
		return true;
	}
#endif // MOVIE_DIRECTSHOW

	vec3_t pos;
	if(CCameraMove::GetInstancePtr()->IsCameraMove()) 
	{
		VectorCopy(CameraPosition, pos);
	}

    MoveMainCamera();

#ifdef MR0
	if(EngineGate::IsOn() && g_pMeshMachine->bEnabled())
		g_pMeshMachine->ProcessLight(HighLight);
#endif //MR0	

	int Width,Height;

	glColor3f(1.f,1.f,1.f);
#ifndef PJH_NEW_SERVER_SELECT_MAP
	BeginBitmap();
	Width = 320;
	Height = 320;
	RenderBitmap(BITMAP_LOG_IN+9,  (float)0,(float)25,(float)Width,(float)Height,0.f,0.f);
	RenderBitmap(BITMAP_LOG_IN+10,(float)320,(float)25,(float)Width,(float)Height,0.f,0.f);
	EndBitmap();
#endif //PJH_NEW_SERVER_SELECT_MAP

	Height = 480;
    Width = GetScreenWidth();
	glClearColor(0.f,0.f,0.f,1.f);

	BeginOpengl(0,25,640,430);
	CreateFrustrum((float)Width/(float)640, pos);

	// 크레딧이 보이면 렌더하지 않음.
	if (!CUIMng::Instance().m_CreditWin.IsShow())
	{
		CameraViewFar = 330.f * CCameraMove::GetInstancePtr()->GetCurrentCameraDistanceLevel();
#ifndef PJH_NEW_SERVER_SELECT_MAP
		// 범위를 다시 설정해야 하므로 걍 제거
		BeginOpengl();
#endif //PJH_NEW_SERVER_SELECT_MAP
		RenderTerrain(false);
		CameraViewFar = 7000.f;

		// BeginOpengl내부 glPushMatrix의 갯수와 EndOpengl내부 glPopMatrix갯수는 정확히 일치하여야 하는데
		// 여기서 BeginOpengl이 한개가 남용되어 삭제 합니다. // OpenGL : StackOverFlow 발생
#ifdef LDS_FIX_OPENGL_STACKOVERFLOW_STACKUNDERFLOW
#else // LDS_FIX_OPENGL_STACKOVERFLOW_STACKUNDERFLOW
		BeginOpengl();
#endif // LDS_FIX_OPENGL_STACKOVERFLOW_STACKUNDERFLOW

		RenderCharactersClient();
		RenderBugs();
		RenderObjects();
		RenderJoints();
		RenderEffects();
		CheckSprites();
		RenderLeaves();
		RenderBoids();
		RenderObjects_AfterCharacter();	// 캐릭터 다음에 알파 오브젝트들 묘사
#ifdef LDK_ADD_NEW_PETPROCESS
	ThePetProcess().RenderPets();
#endif //LDK_ADD_NEW_PETPROCESS

	// BeginOpengl내부 glPushMatrix의 갯수와 EndOpengl내부 glPopMatrix갯수는 정확히 일치하여야 하는데
	// 여기서 BeginOpengl이 하나 닫혀지는곳이 없어 아래를 추가합니다. // OpenGL : StackOverFlow 발생
#ifdef LDS_FIX_OPENGL_STACKOVERFLOW_STACKUNDERFLOW
#ifndef PJH_NEW_SERVER_SELECT_MAP
		//여기서 호출하면 다음 begin이 호출되지 않는다
		EndOpengl();
#endif //PJH_NEW_SERVER_SELECT_MAP
#endif // LDS_FIX_OPENGL_STACKOVERFLOW_STACKUNDERFLOW
	}

	BeginSprite();
#ifdef MR0
#ifdef MR0_NEWRENDERING_EFFECTS_SPRITES
	if(EngineGate::IsOn())
	{
		SpriteManager::Toggle(true);
		RenderSprites();
		RenderParticles();
		SpriteManager::RenderAll();
		SpriteManager::Toggle(false);
	}
	else
#endif //MR0_NEWRENDERING_EFFECTS_SPRITES	
	{
		RenderSprites();
		RenderParticles();
	}
#else
	RenderSprites();
	RenderParticles();
#endif //MR0
	
	EndSprite();
	BeginBitmap();

	if (CCameraMove::GetInstancePtr()->IsTourMode())
	{
#ifndef PJH_NEW_SERVER_SELECT_MAP
		// 화면 흐리기
		EnableAlphaBlend4();
		glColor4f(0.7f,0.7f,0.7f,1.0f);
		float fScale = (sinf(WorldTime*0.0005f) + 1.f) * 0.00011f;
		//RenderBitmap(BITMAP_CHROME+3, 0.0f,0.0f, 640.0f,480.0f, 800.f,600.f, (800.f)/1024.f,(600.f)/1024.f);
		RenderBitmap(BITMAP_CHROME+3, 0.0f,0.0f, 640.0f,480.0f, 800.f*fScale,600.f*fScale, (800.f)/1024.f-800.f*fScale*2,(600.f)/1024.f-600.f*fScale*2);
		float fAngle = WorldTime * 0.00018f;
		float fLumi = 1.0f - (sinf(WorldTime*0.0015f) + 1.f) * 0.25f;
		glColor4f(fLumi*0.3f,fLumi*0.3f,fLumi*0.7f,fLumi);
		fScale = (sinf(WorldTime*0.0015f) + 1.f) * 0.00021f;
		RenderBitmapLocalRotate(BITMAP_CHROME+4,320.0f,240.0f, 1150.0f, 1150.0f, fAngle, fScale*512.f,fScale*512.f, (512.f)/512.f-fScale*2*512.f,(512.f)/512.f-fScale*2*512.f);

		// 위아래 자르기
		EnableAlphaTest();
		glColor4f(0.0f,0.0f,0.0f,1.0f);
		RenderColor(0, 0, 640, 25);
		RenderColor(0, 480-25, 640, 25);

		// 화면칠
		glColor4f(0.0f,0.0f,0.0f,0.2f);
		RenderColor(0, 25, 640, 430);
#endif //PJH_NEW_SERVER_SELECT_MAP
		// 뮤로고
		g_fMULogoAlpha += 0.02f;
		if (g_fMULogoAlpha > 10.0f) g_fMULogoAlpha = 10.0f;
		
		EnableAlphaBlend();
		glColor4f(g_fMULogoAlpha-0.3f,g_fMULogoAlpha-0.3f,g_fMULogoAlpha-0.3f,g_fMULogoAlpha-0.3f);
#ifdef PBG_ADD_MUBLUE_LOGO
		BITMAP_t *pImage =NULL;
		pImage = &Bitmaps[BITMAP_LOG_IN+17];
		RenderBitmap(BITMAP_LOG_IN+17, 320.0f-432*0.4f*0.5f,25.0f, 432*0.4f,384*0.4f,0,0,(432-0.5f)/pImage->Width,(384-0.5f)/pImage->Height);
#else //PBG_ADD_MUBLUE_LOGO
		RenderBitmap(BITMAP_LOG_IN+17, 320.0f-128.0f*0.8f,25.0f, 256.0f*0.8f,128.0f*0.8f);
#endif //PBG_ADD_MUBLUE_LOGO
		EnableAlphaTest();
		glColor4f(g_fMULogoAlpha,g_fMULogoAlpha,g_fMULogoAlpha,g_fMULogoAlpha);
#ifdef PBG_ADD_MUBLUE_LOGO
		pImage = &Bitmaps[BITMAP_LOG_IN+16];
		RenderBitmap(BITMAP_LOG_IN+16, 320.0f-432*0.4f*0.5f,25.0f, 432*0.4f,384*0.4f,0,0,432/pImage->Width,384/pImage->Height);
#else //PBG_ADD_MUBLUE_LOGO
		RenderBitmap(BITMAP_LOG_IN+16, 320.0f-128.0f*0.8f,25.0f, 256.0f*0.8f,128.0f*0.8f);
#endif //PBG_ADD_MUBLUE_LOGO
	}

	SIZE Size;
	char Text[100];
	
	g_pRenderText->SetFont(g_hFont);

	InputTextWidth = 256;
	glColor3f(0.8f,0.7f,0.6f);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->SetBgColor(0, 0, 0, 128);
	
	strcpy(Text,GlobalText[454]);
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), Text,lstrlen(Text),&Size);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	unicode::_GetTextExtentPoint(g_pRenderText->GetFontDC(), Text,lstrlen(Text),&Size);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	g_pRenderText->RenderText(335-Size.cx*640/WindowWidth,480-Size.cy*640/WindowWidth-1,Text);

	strcpy(Text,GlobalText[455]);
	
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), Text,lstrlen(Text),&Size);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	unicode::_GetTextExtentPoint(g_pRenderText->GetFontDC(), Text,lstrlen(Text),&Size);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	g_pRenderText->RenderText(335,480-Size.cy*640/WindowWidth-1,Text);

#ifdef PBG_MOD_BLUEVERLOGO
	sprintf(Text,"Blue Ver %s",m_ExeVersion);
#else //PBG_MOD_BLUEVERLOGO
	sprintf(Text,GlobalText[456],m_ExeVersion);
#endif //PBG_MOD_BLUEVERLOGO
	
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), Text,lstrlen(Text),&Size);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	unicode::_GetTextExtentPoint(g_pRenderText->GetFontDC(), Text,lstrlen(Text),&Size);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	g_pRenderText->RenderText(0, 480-Size.cy*640/WindowWidth-1,Text);

    RenderInfomation();
	
#ifdef ENABLE_EDIT
	RenderDebugWindow();
#endif //ENABLE_EDIT

	EndBitmap();

	EndOpengl();

	return true;
}

void RenderInterfaceEdge()
{
	int Width,Height;
	int WindowX,WindowY;
   	EnableAlphaTest();
	glColor3f(1.f,1.f,1.f);
	//interface edge
	Width = 192;Height = 37;WindowX = 448;WindowY = 0;
	RenderBitmap(BITMAP_LOG_IN,(float)WindowX,(float)WindowY,(float)Width,(float)Height,0.f,0.f,Width/256.f,Height/64.f);
	Width = 192;Height = 37;WindowX = 0;WindowY = 0;
	RenderBitmap(BITMAP_LOG_IN,(float)WindowX,(float)WindowY,(float)Width,(float)Height,Width/256.f,0.f,-Width/256.f,Height/64.f);
	Width = 106;Height = 256;WindowX = 534;WindowY = 3;
	RenderBitmap(BITMAP_LOG_IN+1,(float)WindowX,(float)WindowY,(float)Width,(float)Height,0.f,0.f,Width/128.f,Height/256.f);
	Width = 106;Height = 256;WindowX = 0;WindowY = 3;
	RenderBitmap(BITMAP_LOG_IN+1,(float)WindowX,(float)WindowY,(float)Width,(float)Height,Width/128.f,0.f,-Width/128.f,Height/256.f);
	Width = 106;Height = 222;WindowX = 534;WindowY = 259;
	RenderBitmap(BITMAP_LOG_IN+2,(float)WindowX,(float)WindowY,(float)Width,(float)Height,0.f,0.f,Width/128.f,Height/256.f);
	Width = 106;Height = 222;WindowX = 0;WindowY = 259;
	RenderBitmap(BITMAP_LOG_IN+2,(float)WindowX,(float)WindowY,(float)Width,(float)Height,Width/128.f,0.f,-Width/128.f,Height/256.f);
	Width = 256;Height = 70;WindowX = 192;WindowY = 0;
	RenderBitmap(BITMAP_LOG_IN+3,(float)WindowX,(float)WindowY,(float)Width,(float)Height,0.f,0.f,Width/256.f,Height/128.f);
}


///////////////////////////////////////////////////////////////////////////////
// 로딩 화면
///////////////////////////////////////////////////////////////////////////////

void LoadingScene(HDC hDC)
{
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_NORMAL, "LoadingScene_Start");
#endif // CONSOLE_DEBUG

	CUIMng& rUIMng = CUIMng::Instance();
	if (!InitLoading)
	{
#ifdef LDS_FIX_DISABLE_INPUTJUNKKEY_WHEN_LORENMARKT_EX01
		g_bReponsedMoveMapFromServer = FALSE;
		LoadingWorld = 200;
#else // LDS_FIX_DISABLE_INPUTJUNKKEY_WHEN_LORENMARKT_EX01
		LoadingWorld = 9999999;
#endif // LDS_FIX_DISABLE_INPUTJUNKKEY_WHEN_LORENMARKT_EX01

		InitLoading = true;
		
		// 텍스처 로드.
		LoadBitmap("Interface\\LSBg01.JPG", BITMAP_TITLE, GL_LINEAR);
		LoadBitmap("Interface\\LSBg02.JPG", BITMAP_TITLE+1, GL_LINEAR);
		LoadBitmap("Interface\\LSBg03.JPG", BITMAP_TITLE+2, GL_LINEAR);
		LoadBitmap("Interface\\LSBg04.JPG", BITMAP_TITLE+3, GL_LINEAR);

		::StopMp3(g_lpszMp3[MUSIC_LOGIN_THEME]);

		rUIMng.m_pLoadingScene = new CLoadingScene;
		rUIMng.m_pLoadingScene->Create();
	}

    FogEnable = false;
	::BeginOpengl();
#ifdef USE_SHADOWVOLUME
	::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
#else
	::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
	::BeginBitmap();

	rUIMng.m_pLoadingScene->Render();

	::EndBitmap();
	::EndOpengl();
	::glFlush();
	::SwapBuffers(hDC);

	SAFE_DELETE(rUIMng.m_pLoadingScene);

	SceneFlag = MAIN_SCENE;
	for (int i = 0; i < 4; ++i)
		::DeleteBitmap(BITMAP_TITLE+i);

	::ClearInput();

#ifdef ACC_PACKETSIZE
	g_dwPacketInitialTick = ::GetTickCount();
#endif

#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_NORMAL, "LoadingScene_End");
#endif // CONSOLE_DEBUG
}

///////////////////////////////////////////////////////////////////////////////
// 메인 게임 화면
///////////////////////////////////////////////////////////////////////////////

float CameraDistanceTarget = 1000.f;
float CameraDistance = CameraDistanceTarget;

bool MoveMainCamera()
{
#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_RENDER_MOVEMAINCAMERA, PROFILING_RENDER_MOVEMAINCAMERA );
#endif // DO_PROFILING
    bool bLockCamera = false;

	
#ifdef DO_PROCESS_DEBUGCAMERA
	CameraAngle[0] = fmod( CameraAngle[0], 360.0f );
	CameraAngle[1] = fmod( CameraAngle[1], 360.0f );
	CameraAngle[2] = fmod( CameraAngle[2], 360.0f );
#endif // DO_PROCESS_DEBUGCAMERA

	if (
#ifdef PJH_NEW_SERVER_SELECT_MAP
		World == WD_73NEW_LOGIN_SCENE
#else
		World == WD_77NEW_LOGIN_SCENE
#endif //PJH_NEW_SERVER_SELECT_MAP
		&& CCameraMove::GetInstancePtr()->IsTourMode())
#ifdef PJH_NEW_SERVER_SELECT_MAP
		CameraFOV = 65.0f;
#else //PJH_NEW_SERVER_SELECT_MAP
		CameraFOV = 61.0f;
#endif //PJH_NEW_SERVER_SELECT_MAP
	else
		CameraFOV = 35.f;

#ifdef ENABLE_EDIT2
	{
		bool EditMove = false;
		if( !g_pUIManager->IsInputEnable() )
		{
#ifdef KWAK_FIX_KEY_STATE_RUNTIME_ERR
			if(SEASON3B::IsPress(VK_INSERT) == TRUE)
				CameraAngle[2] += 15;
			if(SEASON3B::IsPress(VK_DELETE) == TRUE)
				CameraAngle[2] -= 15;
#else // KWAK_FIX_KEY_STATE_RUNTIME_ERR
			if(HIBYTE(GetAsyncKeyState(VK_INSERT))==128)
				CameraAngle[2] += 15;
			if(HIBYTE(GetAsyncKeyState(VK_DELETE))==128)
				CameraAngle[2] -= 15;
#endif // KWAK_FIX_KEY_STATE_RUNTIME_ERR

			vec3_t p1,p2;
			Vector(0.f,0.f,0.f,p1);
			FLOAT Velocity = sqrtf(TERRAIN_SCALE*TERRAIN_SCALE)*1.25f;

#ifdef KWAK_FIX_KEY_STATE_RUNTIME_ERR
			if(SEASON3B::IsPress(VK_LEFT) == TRUE) // || (MouseX<=0 && MouseY>=100))
			{
				Vector(-Velocity, -Velocity, 0.f, p1);
				EditMove = true;
			}
			if(SEASON3B::IsPress(VK_RIGHT) == TRUE) // || (MouseX>=639 && MouseY>=100))
			{
				Vector(Velocity, Velocity, 0.f, p1);
				EditMove = true;
			}
			if(SEASON3B::IsPress(VK_UP) == TRUE) // || (MouseY<=0 && MouseX>=100 && MouseX<540))
			{
				Vector(-Velocity, Velocity, 0.f, p1);
				EditMove = true;
			}
			if(SEASON3B::IsPress(VK_DOWN) == TRUE) // || (MouseY>=479))
			{
				Vector(Velocity, -Velocity, 0.f, p1);
				EditMove = true;
			}
#else // KWAK_FIX_KEY_STATE_RUNTIME_ERR
			if(HIBYTE(GetAsyncKeyState(VK_LEFT ))==128)// || (MouseX<=0 && MouseY>=100))
			{
				Vector(-Velocity, -Velocity, 0.f, p1);
				EditMove = true;
			}
			if(HIBYTE(GetAsyncKeyState(VK_RIGHT))==128)// || (MouseX>=639 && MouseY>=100))
			{
				Vector(Velocity, Velocity, 0.f, p1);
				EditMove = true;
			}
			if(HIBYTE(GetAsyncKeyState(VK_UP   ))==128)// || (MouseY<=0 && MouseX>=100 && MouseX<540))
			{
				Vector(-Velocity, Velocity, 0.f, p1);
				EditMove = true;
			}
			if(HIBYTE(GetAsyncKeyState(VK_DOWN ))==128)// || (MouseY>=479))
			{
				Vector(Velocity, -Velocity, 0.f, p1);
				EditMove = true;
			}
#endif // KWAK_FIX_KEY_STATE_RUNTIME_ERR

			glPushMatrix();
			glLoadIdentity();
			glRotatef(-CameraAngle[2],0.f,0.f,1.f);
			float Matrix[3][4];
			GetOpenGLMatrix(Matrix);
			glPopMatrix();
			VectorRotate(p1,Matrix,p2);
			VectorAdd(Hero->Object.Position, p2, Hero->Object.Position);
		}

        if ( InChaosCastle()==false || !Hero->Object.m_bActionStart	)
        {
			if(World == WD_39KANTURU_3RD && Hero->Object.m_bActionStart)
			{}
			else
            if ( World==-1 || Hero->Helper.Type != MODEL_HELPER+3 || Hero->SafeZone )
            {
				Hero->Object.Position[2] = RequestTerrainHeight(Hero->Object.Position[0],Hero->Object.Position[1]);
            }
            else
            {
                if ( World==WD_8TARKAN || World==WD_10HEAVEN )
                    Hero->Object.Position[2] = RequestTerrainHeight(Hero->Object.Position[0],Hero->Object.Position[1])+90.f;
                else
                    Hero->Object.Position[2] = RequestTerrainHeight(Hero->Object.Position[0],Hero->Object.Position[1])+30.f;
            }
        }

		if(EditMove)
		{
			BYTE PathX[1];
			BYTE PathY[1];
			PathX[0] = (BYTE)(Hero->Object.Position[0]/TERRAIN_SCALE);
			PathY[0] = (BYTE)(Hero->Object.Position[1]/TERRAIN_SCALE);
			SendCharacterMove(Hero->Key,Hero->Object.Angle[2],1,PathX,PathY,PathX[0],PathY[0]);
            Hero->Path.PathNum = 0;
		}
	}
#endif //ENABLE_EDIT2

	CameraAngle[0] = 0.f;
	CameraAngle[1] = 0.f;

	if(CameraTopViewEnable)
	{
		CameraViewFar = 3200.f;
		//CameraViewFar = 60000.f;
		CameraPosition[0] = Hero->Object.Position[0];
		CameraPosition[1] = Hero->Object.Position[1];
		CameraPosition[2] = CameraViewFar;
	}
#ifdef BATTLE_SOCCER_EVENT
    else if ( World==WD_6STADIUM && ( FindText( Hero->ID, "webzen" ) || FindText( Hero->ID, "webzen2" ) )  )
    {
        vec3_t Position,TransformPosition, Pos;
		float Matrix[3][4];

        if ( FindText( Hero->ID, "webzen" ) )
        {
    		Pos[0] = 54*TERRAIN_SCALE;
    		Pos[1] = 160*TERRAIN_SCALE;
            CameraAngle[2] = -40.f;
        }
        else
        {
		    Pos[0] = 57*TERRAIN_SCALE;
		    Pos[1] = 163*TERRAIN_SCALE;
            CameraAngle[2] = -140.f;
        }
		Pos[2] = 200.f;
        CameraViewFar = 5000.f;

		Vector(0.f,-CameraDistance,0.f,Position);//-750
		AngleMatrix(CameraAngle,Matrix);
		VectorIRotate(Position,Matrix,TransformPosition);

        VectorAdd(Pos,TransformPosition,CameraPosition);

        CameraPosition[2] = Pos[2];
		CameraPosition[2] += 800.f;
		CameraAngle[0] = -70.f;
    }
#endif// BATTLE_SOCCER_EVENT
	else
	{
		int iIndex = TERRAIN_INDEX((Hero->PositionX),(Hero->PositionY));
		vec3_t Position,TransformPosition;
		float Matrix[3][4];

        if ( battleCastle::InBattleCastle2( Hero->Object.Position ) )
        {
            CameraViewFar = 3000.f;
        }
        else if ( battleCastle::InBattleCastle() && SceneFlag == MAIN_SCENE)
        {
            CameraViewFar = 2500.f;
        }
		else if (World == WD_51HOME_6TH_CHAR)
		{
			CameraViewFar = 2800.f * 1.15f;	// 절벽 끝까지 보이도록
		}
#ifdef PBG_ADD_PKFIELD
 		else if(IsPKField()
#ifdef YDG_ADD_MAP_DOPPELGANGER2
			|| IsDoppelGanger2()
#endif	// YDG_ADD_MAP_DOPPELGANGER2
			)
		{
			CameraViewFar = 3700.0f;
 		}
#endif //PBG_ADD_PKFIELD
        else
        {
        #ifdef CAMERA_TEST_FPS
            CameraViewFar = 2000.f;
            CameraDistance = 500.f;
        #else
            switch ( g_shCameraLevel )
            {
            case 0:
				if(SceneFlag == LOG_IN_SCENE)           // 이혁재 - 로그인 씬 일때 카메라 거리 세팅
				{
				}
				else if(SceneFlag == CHARACTER_SCENE) // 이혁재 - 케릭터 씬 일때 카메라 거리 세팅
				{
#ifdef PJH_NEW_SERVER_SELECT_MAP
					CameraViewFar = 3500.f;
#else //PJH_NEW_SERVER_SELECT_MAP
					CameraViewFar = 10000.f;
#endif //PJH_NEW_SERVER_SELECT_MAP
				}
				else if (g_Direction.m_CKanturu.IsMayaScene())
				{
					CameraViewFar = 2000.f * 10.0f * 0.115f;
				}
				else
				{
					CameraViewFar = 2000.f; 
				}
				break;
            case 1: CameraViewFar = 2500.f; break;
            case 2: CameraViewFar = 2600.f; break;
            case 3: CameraViewFar = 2950.f; break;
			case 5:
            case 4: CameraViewFar = 3200.f; break;
            }
        #endif
        }

		Vector(0.f,-CameraDistance,0.f,Position);//-750
		AngleMatrix(CameraAngle,Matrix);
		VectorIRotate(Position,Matrix,TransformPosition);

		if(SceneFlag == MAIN_SCENE)
		{
			g_pCatapultWindow->GetCameraPos(Position);
		}
		else if (CCameraMove::GetInstancePtr()->IsTourMode())
		{
			CCameraMove::GetInstancePtr()->UpdateTourWayPoint();
			CCameraMove::GetInstancePtr()->GetCurrentCameraPos(Position);
			CameraViewFar = 390.f * CCameraMove::GetInstancePtr()->GetCurrentCameraDistanceLevel();
		}

		if(g_Direction.IsDirection() && !g_Direction.m_bDownHero)
		{
			Hero->Object.Position[2] = 300.0f;
			g_shCameraLevel = g_Direction.GetCameraPosition(Position);
		}
#ifdef PBG_ADD_PKFIELD
 		else if(IsPKField()
#ifdef YDG_ADD_MAP_DOPPELGANGER2
			|| IsDoppelGanger2()
#endif	// YDG_ADD_MAP_DOPPELGANGER2
			)
		{
 			g_shCameraLevel =5;
 		}
#endif //PBG_ADD_PKFIELD
#ifdef YDG_ADD_MAP_DOPPELGANGER1
		else if (IsDoppelGanger1())
		{
 			g_shCameraLevel =5;
		}
#endif	// YDG_ADD_MAP_DOPPELGANGER1
		else g_shCameraLevel =0;

#ifdef PJH_NEW_SERVER_SELECT_MAP
		if(CCameraMove::GetInstancePtr()->IsTourMode())
		{
			vec3_t temp = {0.0f,0.0f,-100.0f};
			VectorAdd(TransformPosition, temp, TransformPosition);
		}
#endif //PJH_NEW_SERVER_SELECT_MAP

		VectorAdd ( Position,TransformPosition,CameraPosition);

#ifdef CAMERA_TEST_FPS
        CameraPosition[2] = Hero->Object.Position[2];//700
		CameraPosition[2] += 200;//700
		CameraAngle[0] = -80.f;
#else
        if ( battleCastle::InBattleCastle()==true )
        {
            CameraPosition[2] = 255.f;//700
        }
		else if (CCameraMove::GetInstancePtr()->IsTourMode());
        else
        {
            CameraPosition[2] = Hero->Object.Position[2];//700
        }
		
		if ( (TerrainWall[iIndex]&TW_HEIGHT)==TW_HEIGHT )
		{
			CameraPosition[2] = g_fSpecialHeight = 1200.f+1;
		}            
		CameraPosition[2] += CameraDistance-150.f;//700

		if (CCameraMove::GetInstancePtr()->IsTourMode())
		{
#ifdef PJH_NEW_SERVER_SELECT_MAP
			CCameraMove::GetInstancePtr()->SetAngleFrustum(-112.5f);
			CameraAngle[0] = CCameraMove::GetInstancePtr()->GetAngleFrustum();
#else	// PJH_NEW_SERVER_SELECT_MAP
			CameraAngle[0] = -78.5f;
#endif	// PJH_NEW_SERVER_SELECT_MAP
			CameraAngle[1] = 0.0f;
			CameraAngle[2] = CCameraMove::GetInstancePtr()->GetCameraAngle();
		}
		else if(SceneFlag == CHARACTER_SCENE) // 이혁재 - 케릭터 씬 일때 카메라 위치 세팅
		{
#ifdef PJH_NEW_SERVER_SELECT_MAP
			CameraAngle[0] = -84.5f;
			CameraAngle[1] = 0.0f;
			CameraAngle[2] = -75.0f;
 			CameraPosition[0] = 9758.93f;
 			CameraPosition[1] = 18913.11f;
 			CameraPosition[2] = 675.5f;
#else //PJH_NEW_SERVER_SELECT_MAP
			CameraAngle[0] = -84.5f;
			CameraAngle[1] = 0.0f;
			CameraAngle[2] = -30.0f;
			CameraPosition[0] = 23566.75f;
			CameraPosition[1] = 14085.51f;
			CameraPosition[2] = 395.0f;
#endif //PJH_NEW_SERVER_SELECT_MAP
		}
		else
			CameraAngle[0] = -48.5f;
#endif
		CameraAngle[0] += EarthQuake;

        //  카메라 속성.
        if ( ( TerrainWall[iIndex]&TW_CAMERA_UP )==TW_CAMERA_UP )
        {
            if ( g_fCameraCustomDistance<=CUSTOM_CAMERA_DISTANCE1 )
            {
                g_fCameraCustomDistance+=10;
            }
        }
        else
        {
            if ( g_fCameraCustomDistance>0 )
            {
                g_fCameraCustomDistance-=10;
            }
        }

        if ( g_fCameraCustomDistance>0 )
        {
            vec3_t angle = { 0.f, 0.f, -45.f };
		    Vector ( 0.f, g_fCameraCustomDistance, 0.f, Position );
		    AngleMatrix ( angle, Matrix );
		    VectorIRotate ( Position, Matrix, TransformPosition );
		    VectorAdd ( CameraPosition, TransformPosition, CameraPosition );
        }
        else if ( g_fCameraCustomDistance<0 )
        {
            vec3_t angle = { 0.f, 0.f, -45.f };
		    Vector ( 0.f, g_fCameraCustomDistance, 0.f, Position );
		    AngleMatrix ( angle, Matrix );
		    VectorIRotate ( Position, Matrix, TransformPosition );
		    VectorAdd ( CameraPosition, TransformPosition, CameraPosition );
        }
	}
	if(World==5)
	{
		CameraAngle[0] += sinf(WorldTime*0.0005f)*2.f;
		CameraAngle[1] += sinf(WorldTime*0.0008f)*2.5f;
	}
#ifdef BATTLE_SOCCER_EVENT
    else if ( World==WD_6STADIUM && ( FindText( Hero->ID, "webzen" ) || FindText( Hero->ID, "webzen2" ) )  )
    {
        CameraDistanceTarget = 3200.f;
        CameraDistance = CameraDistanceTarget;
    }
    else
#endif// BATTLE_SOCCER_EVENT
	if (CCameraMove::GetInstancePtr()->IsTourMode())
	{
		CameraDistanceTarget = 1100.f * CCameraMove::GetInstancePtr()->GetCurrentCameraDistanceLevel() * 0.1f;
		CameraDistance = CameraDistanceTarget;
	}
	else
    {
        if ( battleCastle::InBattleCastle() )
        {
            CameraDistanceTarget = 1100.f;
            CameraDistance = CameraDistanceTarget;
        }
        else
        {
            switch ( g_shCameraLevel )
            {
            case 0: CameraDistanceTarget = 1000.f; CameraDistance += (CameraDistanceTarget-CameraDistance)/3; break;
            case 1: CameraDistanceTarget = 1100.f; CameraDistance += (CameraDistanceTarget-CameraDistance)/3; break;
            case 2: CameraDistanceTarget = 1200.f; CameraDistance += (CameraDistanceTarget-CameraDistance)/3; break;
            case 3: CameraDistanceTarget = 1300.f; CameraDistance += (CameraDistanceTarget-CameraDistance)/3; break;
            case 4: CameraDistanceTarget = 1400.f; CameraDistance += (CameraDistanceTarget-CameraDistance)/3; break;
			case 5: CameraDistanceTarget = g_Direction.m_fCameraViewFar; CameraDistance += (CameraDistanceTarget-CameraDistance)/3; break;
            }
        }
    }

#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_RENDER_MOVEMAINCAMERA );
#endif // DO_PROFILING

    return bLockCamera;
}

void MoveMainScene()
{
#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_MOVE_MAINSCENE_TOTAL, PROFILING_MOVE_MAINSCENE_TOTAL );
#endif // DO_PROFILING

#ifdef CSK_MOD_PROTECT_AUTO_V2
	g_pProtectAuto->Check();
#endif // CSK_MOD_PROTECT_AUTO_V2
	
#ifdef CSK_HACK_TEST
	g_pHackTest->Update();
#endif // CSK_HACK_TEST

	if(!InitMainScene)
	{
#ifdef LDK_ADD_SCALEFORM
		//gfxui 사용시 기존 ui 사용 안함
		if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
		{
#ifdef LDS_FIX_NONINITPROGLEM_SKILLHOTKEY
			g_pMainFrame->ResetSkillHotKey();
#endif // LDS_FIX_NONINITPROGLEM_SKILLHOTKEY
		}
#else //LDK_ADD_SCALEFORM
#ifdef LDS_FIX_NONINITPROGLEM_SKILLHOTKEY
		g_pMainFrame->ResetSkillHotKey();
#endif // LDS_FIX_NONINITPROGLEM_SKILLHOTKEY
#endif //LDK_ADD_SCALEFORM
		
#ifdef CONSOLE_DEBUG
		g_ConsoleDebug->Write( MCD_NORMAL, "Join the game with the following character: %s", CharactersClient[SelectedHero].ID);
#endif // CONSOLE_DEBUG
		g_ErrorReport.Write( "> Character selected <%d> \"%s\"\r\n", SelectedHero+1, CharactersClient[SelectedHero].ID);

        InitMainScene = true;
		
#ifdef CONSOLE_DEBUG
		g_ConsoleDebug->Write( MCD_SEND, "SendRequestJoinMapServer");
#endif // CONSOLE_DEBUG
	    SendRequestJoinMapServer(CharactersClient[SelectedHero].ID);

		CUIMng::Instance().CreateMainScene();	// 메인 씬 UI 생성.

		CameraAngle[2] = -45.f;

		//초기화
		ClearInput();
		InputEnable     = false;
		TabInputEnable  = false;
		InputTextWidth  = 256;
		InputTextMax[0] = 42;
		InputTextMax[1] = 10;
		InputNumber     = 2;
		for(int i=0;i<MAX_WHISPER;i++)
		{
			g_pChatListBox->AddText("", "", SEASON3B::TYPE_WHISPER_MESSAGE);
		}

		g_GuildNotice[0][0] = '\0';
		g_GuildNotice[1][0] = '\0';
	
		g_pPartyManager->Create();			// 파티메니져

#ifdef KJH_FIX_UI_CHAT_MESSAGE
		g_pChatListBox->ClearAll();
#else // KJH_FIX_UI_CHAT_MESSAGE			// 정리할 때 지워야 하는 소스
		g_pChatListBox->Clear();
#endif // KJH_FIX_UI_CHAT_MESSAGE			// 정리할 때 지워야 하는 소스

		g_pSlideHelpMgr->Init();		
		g_pUIMapName->Init();		// rozy

		g_GuildCache.Reset();

#ifdef YDG_ADD_CS5_PORTAL_CHARM
		g_PortalMgr.Reset();
#endif	// YDG_ADD_CS5_PORTAL_CHARM

		ClearAllObjectBlurs();	// 이펙트 초기화
		
		SetFocus(g_hWnd);

		g_ErrorReport.Write( "> Main Scene init success. ");
		g_ErrorReport.WriteCurrentTime();

#ifdef CONSOLE_DEBUG
		g_ConsoleDebug->Write(MCD_NORMAL, "MainScene Init Success");
#endif // CONSOLE_DEBUG
	}
	
	if(CurrentProtocolState == RECEIVE_JOIN_MAP_SERVER)
	{
		EnableMainRender = true;
	}
	if(EnableMainRender == false)
	{
#ifdef DO_PROFILING
		g_pProfiler->EndUnit( EPROFILING_MOVE_MAINSCENE_TOTAL );
#endif // DO_PROFILING
		return;
	}
	//init
	EarthQuake *= 0.2f;

	InitTerrainLight();

#ifdef CSK_FIX_BLUELUCKYBAG_MOVECOMMAND
	g_pBlueLuckyBagEvent->CheckTime();
#endif // CSK_FIX_BLUELUCKYBAG_MOVECOMMAND

#ifdef DO_PROFILING
	MoveInterface_ForProfiling();
#endif // DO_PROFILING

#ifdef DO_PROCESS_DEBUGCAMERA
	MoveInterface_DebugCamera();	
#endif // DO_PROCESS_DEBUGCAMERA

	CheckInventory = NULL;
	CheckSkill = -1;
	MouseOnWindow = false;


	if(!CameraTopViewEnable
#ifdef LDS_FIX_DISABLE_INPUTJUNKKEY_WHEN_LORENMARKT
#ifdef LDS_FIX_DISABLE_INPUTJUNKKEY_WHEN_LORENMARKT_EX01	// 공성전, 통합시장은 서버 로딩시간이 길어 로딩중 키입력 블럭.
		&& ( g_bReponsedMoveMapFromServer == TRUE )		// 키입력 막기 : 서버로부터 로딩 월드 응답이 안온 경우만. 
#else // LDS_FIX_DISABLE_INPUTJUNKKEY_WHEN_LORENMARKT_EX01
		&& LoadingWorld < 30
#endif // LDS_FIX_DISABLE_INPUTJUNKKEY_WHEN_LORENMARKT_EX01
#endif // LDS_FIX_DISABLE_INPUTJUNKKEY_WHEN_LORENMARKT
		)
	{
#ifdef MOD_MOUSE_Y_CLICK_AREA
		if(GFxProcess::GetInstancePtr()->GetUISelect() == 1)
		{
			if(MouseY>=(int)(480))
				MouseOnWindow = true;
		}
		else
		{
			if(MouseY>=(int)(480-48))
				MouseOnWindow = true;
		}
#else //MOD_MOUSE_Y_CLICK_AREA
		if(MouseY>=(int)(480-48))
			MouseOnWindow = true;
#endif //MOD_MOUSE_Y_CLICK_AREA

		g_pPartyManager->Update();
		g_pNewUISystem->Update();
		
		// 윈도우 아닌곳 클릭시
		if (MouseLButton == true 
			&& false == g_pNewUISystem->CheckMouseUse() /* NewUI용 마우스 체크 */
			&& g_dwMouseUseUIID == 0 /* 기존의 UI 마우스 체크 전역 변수 */
			&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHATINPUTBOX) == false
			)
		{
			g_pWindowMgr->SetWindowsEnable(FALSE);
			g_pFriendMenu->HideMenu();
			g_dwKeyFocusUIID = 0;
			if(GetFocus() != g_hWnd)
			{
				SaveIMEStatus();
				SetFocus(g_hWnd);
			}
		}
#ifdef _PVP_ADD_MOVE_SCROLL
		g_MurdererMove.MurdererMoveCheck();
#endif	// _PVP_ADD_MOVE_SCROLL
		
		MoveInterface();
		MoveTournamentInterface();
		if( ErrorMessage != MESSAGE_LOG_OUT )
			g_pUIManager->UpdateInput();
	}

	if(ErrorMessage != NULL)
		MouseOnWindow = true;

    MoveObjects();
    if(!CameraTopViewEnable)
    	MoveItems();
	if ( ( World==WD_0LORENCIA && HeroTile!=4 ) || 
         ( World==WD_2DEVIAS && HeroTile!=3 && HeroTile<10 ) 
		 || World==WD_3NORIA 
		 || World==WD_7ATLANSE 
		 || InDevilSquare() == true
		 || World==WD_10HEAVEN 
         || InChaosCastle()==true 
         || battleCastle::InBattleCastle()==true
		 || M31HuntingGround::IsInHuntingGround()==true
		 || M33Aida::IsInAida()==true
		 || M34CryWolf1st::IsCyrWolf1st()==true
		|| World == WD_42CHANGEUP3RD_2ND
#ifdef CSK_ADD_MAP_ICECITY
		|| IsIceCity()
#endif // CSK_ADD_MAP_ICECITY
#ifdef YDG_ADD_MAP_SANTA_TOWN
		|| IsSantaTown()
#endif	// YDG_ADD_MAP_SANTA_TOWN
#ifdef PBG_ADD_PKFIELD
		|| IsPKField()
#endif //PBG_ADD_PKFIELD
#ifdef YDG_ADD_MAP_DOPPELGANGER2
		|| IsDoppelGanger2()
#endif	// YDG_ADD_MAP_DOPPELGANGER2
#ifdef LDS_ADD_EMPIRE_GUARDIAN
		|| IsEmpireGuardian1() 
		|| IsEmpireGuardian2()
		|| IsEmpireGuardian3()
		|| IsEmpireGuardian4()
#endif //LDS_ADD_EMPIRE_GUARDIAN
#ifdef LDS_ADD_MAP_UNITEDMARKETPLACE
		|| IsUnitedMarketPlace()
#endif	// LDS_ADD_MAP_UNITEDMARKETPLACE
	 )
	{
        MoveLeaves();
	}
	
	MoveBoids();
   	MoveFishs();
	MoveBugs();
	MoveChat();
	UpdatePersonalShopTitleImp();
	MoveHero();
    MoveCharactersClient();
#ifdef LDK_ADD_NEW_PETPROCESS
	ThePetProcess().UpdatePets();
#endif //LDK_ADD_NEW_PETPROCESS
    MovePoints();
	MovePlanes();
	MoveEffects();
    MoveJoints();
    MoveParticles();
    MovePointers();

	g_Direction.CheckDirection();
    
#ifdef ENABLE_EDIT
    EditObjects();
#endif //ENABLE_EDIT

	g_GameCensorship->Update();

#ifdef CSK_LH_DEBUG_CONSOLE
	g_ConsoleDebug->UpdateMainScene();
#endif // CSK_LH_DEBUG_CONSOLE

#ifdef USE_SELFCHECKCODE
	END_OF_FUNCTION( Pos_SelfCheck02);
Pos_SelfCheck02:
	;
#endif

#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_MOVE_MAINSCENE_TOTAL );
#endif // DO_PROFILING
}

bool RenderMainScene()
{
#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_RENDER_MAINSCENE_TOTAL, PROFILING_RENDER_MAINSCENE_TOTAL );
#endif // DO_PROFILING
	if(EnableMainRender == false)  
	{
#ifdef DO_PROFILING
		g_pProfiler->EndUnit( EPROFILING_RENDER_MAINSCENE_TOTAL );
#endif // DO_PROFILING
		return false;
	}

#ifdef LDS_FIX_DISABLE_INPUTJUNKKEY_WHEN_LORENMARKT_EX01
	if(g_bReponsedMoveMapFromServer != TRUE)		// 서버로부터 월드이동에 대한 응답이 없는 경우.
#else // LDS_FIX_DISABLE_INPUTJUNKKEY_WHEN_LORENMARKT_EX01
    if(( LoadingWorld) > 30)
#endif // LDS_FIX_DISABLE_INPUTJUNKKEY_WHEN_LORENMARKT_EX01
	{
#ifdef DO_PROFILING
		g_pProfiler->EndUnit( EPROFILING_RENDER_MAINSCENE_TOTAL );
#endif // DO_PROFILING
		return false;
	}

    FogEnable = false;

    vec3_t pos;
    if(MoveMainCamera() == true)
    {
        VectorCopy ( Hero->Object.StartPosition, pos );
    }
#ifdef BATTLE_SOCCER_EVENT
    else if(World==WD_6STADIUM 
		&& (FindText(Hero->ID, "webzen") || FindText(Hero->ID, "webzen2")))
    {
        if(FindText(Hero->ID, "webzen"))
        {
            pos[0] = 64*TERRAIN_SCALE;
            pos[1] = 154*TERRAIN_SCALE;
        }
        else
        {
            pos[0] = 63*TERRAIN_SCALE;
            pos[1] = 168*TERRAIN_SCALE;
        }
        pos[2] = Hero->Object.Position[2];
    }
#endif// BATTLE_SOCCER_EVENT
    else
    {
		g_pCatapultWindow->GetCameraPos(pos);
		
		if(g_Direction.IsDirection() && g_Direction.m_bDownHero == false)
		{
			g_Direction.GetCameraPosition(pos);
		}
    }

	int Width,Height;

    BYTE byWaterMap = 0;

	if(CameraTopViewEnable == false)
	{
#ifdef MOD_MAINSCENE_HEIGHT
		Height = 480;
#else //MOD_MAINSCENE_HEIGHT
		Height = 480-48;
#endif //MOD_MAINSCENE_HEIGHT
	}
	else
	{
		Height = 480;
	}

    Width = GetScreenWidth();
    if(World == WD_0LORENCIA)      
	{
		glClearColor(10/256.f,20/256.f,14/256.f,1.f);
	}
    else if(World == WD_2DEVIAS)
	{
		glClearColor(0.f/256.f,0.f/256.f,10.f/256.f,1.f);
	}
    else if(World == WD_10HEAVEN)
	{
		glClearColor(3.f/256.f,25.f/256.f,44.f/256.f,1.f);
	}
    else if(InChaosCastle() == true)
	{
		glClearColor(0/256.f,0/256.f,0/256.f,1.f);
	}
	else if(World >= WD_45CURSEDTEMPLE_LV1 && World <= WD_45CURSEDTEMPLE_LV6) 
	{
		glClearColor(9.f/256.f,8.f/256.f,33.f/256.f,1.f);
	}
    else if(InHellas() == true)
    {
        byWaterMap = 1;
        glClearColor(0.f/256.f,0.f/256.f,0.f/256.f,1.f);
    }
    else    
	{
		glClearColor(0/256.f,0/256.f,0/256.f,1.f);
	}

	BeginOpengl(0,0,Width,Height);		///////////////// BeginOpengl //////////////////////////////////////////////

	CreateFrustrum((float)Width/(float)640, pos);
#ifdef DYNAMIC_FRUSTRUM
	ResetAllFrustrum();
#endif //DYNAMIC_FRUSTRUM

#ifdef DO_PROCESS_DEBUGCAMERA
	// 게임 카메라 Frustum 구성된 이후에 디버깅용 카메라로 Camera View Transform 구성하도록 한다.
	ProcessDebugCamera();	
#endif // DO_PROCESS_DEBUGCAMERA

    if ( battleCastle::InBattleCastle() )
    {
        if ( battleCastle::InBattleCastle2( Hero->Object.Position ) )
        {
            vec3_t Color = { 0.f, 0.f, 0.f };
            battleCastle::StartFog ( Color );
        }
        else
        {
            glDisable ( GL_FOG );
        }
    }

#ifdef MR0
	if(EngineGate::IsOn() && g_pMeshMachine->bEnabled())
		g_pMeshMachine->ProcessLight(HighLight, battleCastle::InBattleCastle());
#endif //MR0

	CreateScreenVector(MouseX,MouseY,MouseTarget);

    if ( IsWaterTerrain()==false )
    {
		if(World==WD_39KANTURU_3RD)
		{
			if(!g_Direction.m_CKanturu.IsMayaScene())
				RenderTerrain(false);
		}
		else
        //  천공맵에서는 지형을 보여주지 않는다.
        if(World!=WD_10HEAVEN && World != -1)
        {
#ifdef PBG_ADD_PKFIELD
			if(IsPKField()
#ifdef YDG_ADD_MAP_DOPPELGANGER2
				|| IsDoppelGanger2()
#endif	// YDG_ADD_MAP_DOPPELGANGER2
				)
			{
				RenderObjects();
			}
#endif //PBG_ADD_PKFIELD
#ifndef DONTRENDER_TERRAIN
            RenderTerrain(false);
#endif // DONTRENDER_TERRAIN
        }
    }
#ifdef PBG_ADD_PKFIELD
		if(!IsPKField()
#ifdef YDG_ADD_MAP_DOPPELGANGER2
			&& !IsDoppelGanger2()
#endif	// YDG_ADD_MAP_DOPPELGANGER2
			)
#endif //PBG_ADD_PKFIELD
#ifndef DONTRENDER_TERRAIN
		RenderObjects();            //  오브젝트 묘사.
#endif // DONTRENDER_TERRAIN

#ifndef DONTRENDER_TERRAIN
	RenderEffectShadows();
   	RenderBoids();              //  
#endif // DONTRENDER_TERRAIN

	RenderCharactersClient();   //  캐릭터 묘사.

	if(EditFlag!=EDIT_NONE)     //  에디터에서만 처리.
	{
		RenderTerrain(true);
    }
    if(!CameraTopViewEnable)    //  아이템 묘사.
     	RenderItems();

#ifndef DONTRENDER_TERRAIN	
   	RenderFishs();                  //  물고기.
   	RenderBugs();                   //  벌레.
    RenderLeaves();             //  잎.
#endif // DONTRENDER_TERRAIN

#ifdef LDK_ADD_NEW_PETPROCESS
#ifdef LJH_FIX_PET_SHOWN_IN_CHAOS_CASTLE_BUG
	if (!InChaosCastle())
#endif //LJH_FIX_PET_SHOWN_IN_CHAOS_CASTLE_BUG
		ThePetProcess().RenderPets();
#endif //LDK_ADD_NEW_PETPROCESS

	RenderBoids(true);              //
	RenderObjects_AfterCharacter();	// 캐릭터 다음에 알파 오브젝트들 묘사

    RenderJoints(byWaterMap );      //  연결되는 효과.
	RenderEffects();                //  이펙트 효과.
    RenderBlurs();                  //  칼잔상 효과.

#ifdef USE_SHADOWVOLUME
	ShadowVolumeDoIt();

	BeginBitmap();		///////////////// BeginBitmap //////////////////////////////////////////////
	RenderShadowToScreen();
	EndBitmap();		///////////////// EndBitmap //////////////////////////////////////////////

	BeginOpengl(0,0,Width,Height);	///////////////// BeginOpengl //////////////////////////////////////////////
#endif

    //  2D 처리 내용들.
    CheckSprites();
    BeginSprite();		///////////////// BeginSprite //////////////////////////////////////////////
#ifndef DONTRENDER_TERRAIN	
    if((World==WD_2DEVIAS && HeroTile!=3 && HeroTile<10)
#ifdef CSK_ADD_MAP_ICECITY
		|| IsIceCity()
#endif // CSK_ADD_MAP_ICECITY
#ifdef YDG_ADD_MAP_SANTA_TOWN
		|| IsSantaTown()
#endif	// YDG_ADD_MAP_SANTA_TOWN
#ifdef PBG_ADD_PKFIELD
		|| IsPKField()
#endif //PBG_ADD_PKFIELD
#ifdef YDG_ADD_MAP_DOPPELGANGER2
		|| IsDoppelGanger2()
#endif	// YDG_ADD_MAP_DOPPELGANGER2
#ifdef LDS_ADD_EMPIRE_GUARDIAN
		|| IsEmpireGuardian1()
		|| IsEmpireGuardian2()
		|| IsEmpireGuardian3()
		|| IsEmpireGuardian4()
#endif //LDS_ADD_EMPIRE_GUARDIAN
#ifdef LDS_ADD_MAP_UNITEDMARKETPLACE
		|| IsUnitedMarketPlace()
#endif	// LDS_ADD_MAP_UNITEDMARKETPLACE
		)
        RenderLeaves();
#endif // DONTRENDER_TERRAIN

#ifdef MR0
#ifdef MR0_NEWRENDERING_EFFECTS_SPRITES
	if(EngineGate::IsOn())
	{
		SpriteManager::Toggle(true);
		RenderSprites();
		RenderParticles();
		SpriteManager::RenderAll();
		SpriteManager::Toggle(false);
	}
	else
#endif //MR0_NEWRENDERING_EFFECTS_SPRITES
	{
		RenderSprites();
		RenderParticles();
	}
#else // MR0
	RenderSprites();
	RenderParticles();
#endif // MR0

    //  물 지형을 찍는다.
    if ( IsWaterTerrain()==false )
    {
        RenderPoints ( byWaterMap );    //  점수.
    }

    EndSprite();		///////////////// EndSprite //////////////////////////////////////////////

	RenderAfterEffects();

    //  물 지형을 찍는다.
    if(IsWaterTerrain() == true)
    {
        byWaterMap = 2;

#ifdef KJH_FIX_WOPS_K22929_DONOT_RENDER_NOTICE_IN_KALIMA
		EndOpengl();	///////////////// EndOpengl //////////////////////////////////////////////
#endif // KJH_FIX_WOPS_K22929_DONOT_RENDER_NOTICE_IN_KALIMA
		// 새로 시작해준다.
	    BeginOpengl( 0, 0, Width, Height );	///////////////// BeginOpengl //////////////////////////////////////////////
        RenderWaterTerrain ();      //  물지형 묘사.
        RenderJoints(byWaterMap );      //  연결되는 효과.
        RenderEffects ( true );     //  이펙트 효과.
        RenderBlurs ();             //  칼잔상 효과.

        //  2D 처리 내용들.
        CheckSprites();
        BeginSprite();		///////////////// BeginSprite //////////////////////////////////////////////
#ifndef DONTRENDER_TERRAIN	
        if(World==WD_2DEVIAS && HeroTile!=3 && HeroTile<10)
            RenderLeaves();
#endif // DONTRENDER_TERRAIN

#ifdef MR0
#ifdef MR0_NEWRENDERING_EFFECTS_SPRITES
		if(EngineGate::IsOn())
		{
			SpriteManager::Toggle(true);
			RenderSprites(byWaterMap);
			RenderParticles(byWaterMap);
			SpriteManager::RenderAll();
			SpriteManager::Toggle(false);
		}
		else
#endif //MR0_NEWRENDERING_EFFECTS_SPRITES	
		{
			RenderSprites(byWaterMap);
			RenderParticles(byWaterMap);			
		}
#else // MR0
		RenderSprites(byWaterMap);
		RenderParticles(byWaterMap);
#endif //MR0

        RenderPoints ( byWaterMap );    //  점수.

        EndSprite();	///////////////// EndSprite //////////////////////////////////////////////
		EndOpengl();	///////////////// EndOpengl //////////////////////////////////////////////

#ifdef KJH_FIX_WOPS_K22929_DONOT_RENDER_NOTICE_IN_KALIMA
		// 아래 렌더할것들이 남아있으므로 해줘야 한다.
		BeginOpengl( 0, 0, Width, Height );	///////////////// BeginOpengl //////////////////////////////////////////////
#endif // KJH_FIX_WOPS_K22929_DONOT_RENDER_NOTICE_IN_KALIMA
    }

    if(battleCastle::InBattleCastle())
    {
        if(battleCastle::InBattleCastle2(Hero->Object.Position))
        {
            battleCastle::EndFog();
        }
    }

    SelectObjects();

	BeginBitmap();		///////////////// BeginBitmap //////////////////////////////////////////////	

    RenderObjectDescription();
	

	if(CameraTopViewEnable == false)
	{
        RenderInterface(true);
	}
	RenderTournamentInterface();

	EndBitmap();						
	
	g_pPartyManager->Render();
	g_pNewUISystem->Render();	//. 2D, 3D 렌더링
	
	BeginBitmap();

	RenderInfomation();

#ifdef ENABLE_EDIT
	RenderDebugWindow();
#endif //ENABLE_EDIT

#ifdef FOR_DRAMA
	extern int g_iShowGoodLuck;
	if ( g_iShowGoodLuck > 0)
	{
		g_iShowGoodLuck--;
		if ( g_iShowGoodLuck % 10 < 5)
		{
			g_pRenderText->SetTextColor(128, 128, 0, 255);
		}
		else
		{
			g_pRenderText->SetTextColor(255, 255, 0, 255);
		}
		g_pRenderText->SetFont(g_hFontBig);
		g_pRenderText->SetBgColor(40, 40, 40, 128);
		
		RenderText(250, 200,"다 잘될거야!");
	}
#endif // FOR_DRAMA

#if SELECTED_LANGUAGE == LANGUAGE_KOREAN	
	g_GameCensorship->Render();
#endif //SELECTED_LANGUAGE == LANGUAGE_KOREAN

	// 칼리마에서 커서가 안보이는 문제때문에
	EndBitmap();	///////////////// EndBitmap //////////////////////////////////////////////	
	BeginBitmap();	///////////////// BeginBitmap //////////////////////////////////////////////

    RenderCursor();

	EndBitmap();	///////////////// EndBitmap //////////////////////////////////////////////	
    EndOpengl();	///////////////// EndOpengl //////////////////////////////////////////////	
	
#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_RENDER_MAINSCENE_TOTAL );
#endif // DO_PROFILING
	return true;
}

int TimeRemain = 40;
#ifdef _DEBUG
BOOL g_bTimeTurbo = FALSE;
#endif
extern int ChatTime;
extern int WaterTextureNumber;

int TestTime = 0;
extern int  GrabScreen;


#ifdef FOR_DRAMA
int g_iGemCount = 0;
int g_iItemIndex = 0;
int g_iShowGoodLuck = 0;
#endif

//#ifdef _DEBUG
#if SELECTED_LANGUAGE == LANGUAGE_CHINESE
// 파티션 표시
extern char g_lpszPartitionName[64];
#endif //SELECTED_LANGAUGE == LANGUAGE_CHINESE*/


void MoveCharacter(CHARACTER *c,OBJECT *o);

#ifdef DO_PROCESS_DEBUGCAMERA
void ProcessDebugCamera()
{
	g_pDebugCameraManager->ProcessCamera( MouseX, MouseY );
}
#endif DO_PROCESS_DEBUGCAMERA

void MainScene(HDC hDC)
{
#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_MAINSCENE_TOTAL, PROFILING_MAINSCENE_TOTAL );
#endif // DO_PROFILING

#ifdef LDS_ADD_MULTISAMPLEANTIALIASING
	SetEnableMultisample();
#endif // LDS_ADD_MULTISAMPLEANTIALIASING

   	CalcFPS();

#ifdef LEM_ADD_GAMECHU	// 게임츄 웹페이지 로그인 정보 없음- 종료.

#endif // LEM_ADD_GAMECHU
	
#ifdef LDK_ADD_GLOBAL_PORTAL_WEBLOGIN_CHECK
	if( !GlobalPortalSystem::Instance().IsAuthSet() )
	{
		//g_ErrorReport.Write( "-- 웹페이지 로그인 정보 없음 --> 종료합니다. -- \r\n" );
		CUIMng::Instance().PopUpMsgWin(MESSAGE_NOT_EXECUTION_WEBSTARTER);
	}
#endif //LDK_ADD_GLOBAL_PORTAL_WEBLOGIN_CHECK

	// 초당 20 고정 프레임을 Unfixed 합니다.
#if defined(LDS_FOR_DEVELOPMENT_TESTMODE) || defined(LDS_UNFIXED_FIXEDFRAME_FORDEBUG)
	const int TIMEREMAIN_LIMITFIXEDFRAME = 40;
	const int TIMEREMAIN_LIMITUNFIXEDFRAME = -99999;
	int iCurrentLimitFrame = TIMEREMAIN_LIMITFIXEDFRAME;
	
	if( g_bUnfixedFixedFrame)
	{
		iCurrentLimitFrame = TIMEREMAIN_LIMITUNFIXEDFRAME;
	}

	while( TimeRemain >= iCurrentLimitFrame)
#else // defined(LDS_FOR_DEVELOPMENT_TESTMODE) || defined(LDS_UNFIXED_FIXEDFRAME_FORDEBUG)
    while(TimeRemain >= 40)
#endif // defined(LDS_FOR_DEVELOPMENT_TESTMODE) || defined(LDS_UNFIXED_FIXEDFRAME_FORDEBUG)
	{
//		항상 다음과 같은 순서로 해야함.
//		1. g_pNewKeyInput->ScanAsyncKeyState();
//		2. CInput::Instance().Update();
//		3. CUIMng::Instance().Update(dDeltaTick);

		g_pNewKeyInput->ScanAsyncKeyState();

		// 로그인씬과 캐릭터 선택씬에서만 작동하는 전용 UI.
		if (LOG_IN_SCENE == SceneFlag || CHARACTER_SCENE == SceneFlag)
		{
		// g_pTimer로 이전 프레임과 현재 프레임 간의 시간 계산.
			double dDeltaTick = g_pTimer->GetTimeElapsed();
			// DeltaTick 보정
			// if deltaSeconds is greater than 200ms it's probably due to disk paging or whatever.
			// we clamp to 200ms, essentially subtracting out the time. 
			// for important timing stuff like race times and such we will also need to accumulate
			// this subtracted time and adjust the race time accordingly
			dDeltaTick = MIN(dDeltaTick, 200.0);
			g_pTimer->ResetTimer();

			CInput::Instance().Update();			// 인풋 업데이트.
			CUIMng::Instance().Update(dDeltaTick);	// UI 메니저 업데이트.
		}

		g_dwMouseUseUIID = 0;	//. 초기화

#ifdef LDK_ADD_SCALEFORM
		if(GFxProcess::GetInstancePtr()->GetUISelect() == 1)
		{
			GFxProcess::GetInstancePtr()->GFxUpdate(SceneFlag);
		}
#endif //LDK_ADD_SCALEFORM

		switch (SceneFlag)
		{
		case LOG_IN_SCENE:		// 로그인 씬.
			NewMoveLogInScene();
			break;

		case CHARACTER_SCENE:	// 캐릭터 선택, 생성 씬.
			NewMoveCharacterScene();
			break;

		case MAIN_SCENE:		// 게임 씬.
			MoveMainScene();
			break;
		}
		
#ifdef ADD_MU_HOMEPAGE
		// LHJ - 창 모드 일때 뮤 홈페이지 열기
		if(g_bUseWindowMode)
		{
#ifdef NDEBUG
			if(PressKey(VK_F11))	// LHJ - F11키를 누르면 뮤 홈페이지 뜬다
			{
#ifdef KJH_FIX_MU_HOMEPAGE_ADDRESS
#ifdef PBG_FIX_BLUEHOMEPAGE_LINK
				leaf::OpenExplorer("http://blue.muonline.co.kr/SiteMain.aspx");
#else //PBG_FIX_BLUEHOMEPAGE_LINK
				leaf::OpenExplorer("www.muonline.co.kr/sitemain.aspx");
#endif//PBG_FIX_BLUEHOMEPAGE_LINK
#else // KJH_FIX_MU_HOMEPAGE_ADDRESS
				leaf::OpenExplorer("www.muonline.co.kr/GameMain.asp");
#endif // KJH_FIX_MU_HOMEPAGE_ADDRESS
			}
#endif // NDEBUG	
		}
#endif //ADD_MU_HOMEPAGE
		
		for ( int iCount = 0; iCount < 5; ++iCount)
		{
			g_PhysicsManager.Move(0.005f);
		}

        MoveNotices();

		if(PressKey(VK_SNAPSHOT))
		{
			if(GrabEnable)
				GrabEnable = false;
			else
				GrabEnable = true;
		}
        if(ChatTime > 0) ChatTime--;
		if(MacroTime > 0) MacroTime--;
		WaterTextureNumber++;
		WaterTextureNumber%=32;
		MoveSceneFrame++;

		// 초당 20 Frame UnFix인경우 While을 한번만 실행 하도록 합니다.
#if defined(LDS_FOR_DEVELOPMENT_TESTMODE) || defined(LDS_UNFIXED_FIXEDFRAME_FORDEBUG)
		TimeRemain -= 40;

		char Text[100];
		sprintf(Text,"%d",TestTime++);
		//SendChat(Text);
		break;	

#else // defined(LDS_FOR_DEVELOPMENT_TESTMODE) || defined(LDS_UNFIXED_FIXEDFRAME_FORDEBUG)
		TimeRemain -= 40;

		char Text[100];
		sprintf(Text,"%d",TestTime++);
		//SendChat(Text);
#endif // defined(LDS_FOR_DEVELOPMENT_TESTMODE) || defined(LDS_UNFIXED_FIXEDFRAME_FORDEBUG)

#if defined(MSZ_ADD_EXCEPTION_TEST)
		if ( SEASON3B::IsRepeat(VK_CONTROL) && SEASON3B::IsPress(VK_NUMPAD1) )
		{
			RaiseException(WAIT_IO_COMPLETION, 0, 0, NULL);
		}
		else if ( SEASON3B::IsRepeat(VK_CONTROL) && SEASON3B::IsPress(VK_NUMPAD2) )
		{
			RaiseException(STILL_ACTIVE, 0, 0, NULL);
		}
		else if ( SEASON3B::IsRepeat(VK_CONTROL) && SEASON3B::IsPress(VK_NUMPAD3) )
		{
			RaiseException(EXCEPTION_DATATYPE_MISALIGNMENT, 0, 0, NULL);
		}
		else if ( SEASON3B::IsRepeat(VK_CONTROL) && SEASON3B::IsPress(VK_NUMPAD4) )
		{
			RaiseException(EXCEPTION_BREAKPOINT, 0, 0, NULL);
		}
		else if ( SEASON3B::IsRepeat(VK_CONTROL) && SEASON3B::IsPress(VK_NUMPAD5) )
		{
			RaiseException(EXCEPTION_SINGLE_STEP, 0, 0, NULL);
		}
		else if ( SEASON3B::IsRepeat(VK_CONTROL) && SEASON3B::IsPress(VK_NUMPAD6) )
		{
			RaiseException(EXCEPTION_FLT_DENORMAL_OPERAND, 0, 0, NULL);
		}
		else if ( SEASON3B::IsRepeat(VK_CONTROL) && SEASON3B::IsPress(VK_NUMPAD7) )
		{
			RaiseException(EXCEPTION_FLT_DIVIDE_BY_ZERO, 0, 0, NULL);
		}
		else if ( SEASON3B::IsRepeat(VK_CONTROL) && SEASON3B::IsPress(VK_NUMPAD8) )
		{
			RaiseException(EXCEPTION_FLT_INEXACT_RESULT, 0, 0, NULL);
		}
		else if ( SEASON3B::IsRepeat(VK_CONTROL) && SEASON3B::IsPress(VK_NUMPAD9) )
		{
			RaiseException(EXCEPTION_FLT_INVALID_OPERATION, 0, 0, NULL);
		}
		else if ( SEASON3B::IsRepeat(VK_CONTROL) && SEASON3B::IsPress(VK_NUMPAD0) )
		{
			RaiseException(EXCEPTION_FLT_OVERFLOW, 0, 0, NULL);
		}
#endif	// defined(MSZ_ADD_EXCEPTION_TEST)
	} // while(TimeRemain >= 40)
	if(Destroy) return;

	//. 텍스쳐 관리
	Bitmaps.Manage();

	Set3DSoundPosition();

	// 스크린 캡쳐 준비
	SYSTEMTIME st;
	GetLocalTime( &st);
	sprintf( GrabFileName, "Screen(%02d_%02d-%02d_%02d)-%04d.jpg", st.wMonth, st.wDay, st.wHour, st.wMinute, GrabScreen);
	char Text[256];
	sprintf(Text,GlobalText[459],GrabFileName);
	char lpszTemp[64];
#ifdef KJH_ADD_SERVER_LIST_SYSTEM
	wsprintf( lpszTemp, " [%s / %s]", g_ServerListManager->GetSelectServerName(), Hero->ID);
#else // KJH_ADD_SERVER_LIST_SYSTEM
	wsprintf( lpszTemp, " [%s / %s]", ServerList[max( 0, min( ServerSelectHi, 30))].Name, Hero->ID);
#endif // KJH_ADD_SERVER_LIST_SYSTEM
	strcat( Text, lpszTemp);

	// 캡쳐시 문장 표시 부분
	int iCaptureMode = 1;	// 0 은 문장 없이, 1 은 문장 있게
#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE
	iCaptureMode = 0;
#endif
#ifdef KWAK_FIX_KEY_STATE_RUNTIME_ERR
	if(SEASON3B::IsRepeat(VK_SHIFT) == TRUE)
#else // KWAK_FIX_KEY_STATE_RUNTIME_ERR
	if ( HIBYTE( GetAsyncKeyState( VK_SHIFT)))
#endif // KWAK_FIX_KEY_STATE_RUNTIME_ERR
	{
		iCaptureMode = 1 - iCaptureMode;
	}
	if(GrabEnable && iCaptureMode == 1)
	{
#ifndef CAMERA_TEST
#ifndef FOR_WORK
		g_pChatListBox->AddText("", Text, SEASON3B::TYPE_SYSTEM_MESSAGE);
#endif // FOR_WORK
#endif // CAMERA_TEST
	}

#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_RENDER_SCENE_TOTAL, PROFILING_RENDER_SCENE_TOTAL );
#endif // DO_PROFILING

	//////////////// 랜더링 시작 //////////////////////////////////////////////////////////
#ifdef CAMERA_TEST_FPS
    glClearColor(0.5f,0.5f,0.5f,1.f);
#else

    if( World==WD_10HEAVEN )
    {
        glClearColor(3.f/256.f,25.f/256.f,44.f/256.f,1.f);
    }
#ifdef PJH_NEW_SERVER_SELECT_MAP
	else if (World == WD_73NEW_LOGIN_SCENE
		|| World == WD_74NEW_CHARACTER_SCENE)
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f );
    }
#endif //PJH_NEW_SERVER_SELECT_MAP
    else if (InHellas(World))
    {
        glClearColor(30.f/256.f,40.f/256.f,40.f/256.f,1.f);
    }
    else if ( InChaosCastle()==true )
    {
        glClearColor ( 0.f, 0.f, 0.f, 1.f );
    }
    else if ( battleCastle::InBattleCastle() && battleCastle::InBattleCastle2( Hero->Object.Position ) )
    {
        glClearColor ( 0.f, 0.f, 0.f, 1.f );
    }
	else if ( World >= WD_45CURSEDTEMPLE_LV1 && World <= WD_45CURSEDTEMPLE_LV6) 
	{
		glClearColor(9.f/256.f,8.f/256.f,33.f/256.f,1.f);
	}
	else if ( World == WD_51HOME_6TH_CHAR 
#ifndef PJH_NEW_SERVER_SELECT_MAP
		|| World == WD_77NEW_LOGIN_SCENE
#endif //PJH_NEW_SERVER_SELECT_MAP
		)
	{
		glClearColor(178.f/256.f,178.f/256.f,178.f/256.f,1.f);	// 배경색 회색으로
	}
#ifndef PJH_NEW_SERVER_SELECT_MAP
	else if(World == WD_78NEW_CHARACTER_SCENE)
	{
		glClearColor(0.f,0.f,0.f,1.f);
	}
#endif //PJH_NEW_SERVER_SELECT_MAP
#ifdef YDG_ADD_MAP_DOPPELGANGER1
	else if(World == WD_65DOPPLEGANGER1)
	{
		glClearColor(148.f/256.f,179.f/256.f,223.f/256.f,1.f);	// 배경색 하늘색으로
	}
#endif	// YDG_ADD_MAP_DOPPELGANGER1
    else
    {
        glClearColor(0/256.f,0/256.f,0/256.f,1.f);
    }
#endif
		
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    int TimeRender = 0;
    int TimePrior = GetTickCount();
	bool Success = false;
	if(SceneFlag == LOG_IN_SCENE)
	{
		Success = NewRenderLogInScene(hDC);
	}
#ifdef USE_SELFCHECKCODE
	SendCrcOfFunction( 3, 15, MoveCharacter, 0x21DE);
#endif
	else if(SceneFlag == CHARACTER_SCENE)
	{
		Success = NewRenderCharacterScene(hDC);
	}
	else if(SceneFlag == MAIN_SCENE)
	{
		Success = RenderMainScene();
	}

	g_PhysicsManager.Render();
	if(GrabEnable)
	{
		SaveScreen();
	}

	// 캡쳐시 문장 표시 부분
	if(GrabEnable && iCaptureMode == 0)
	{
		g_pChatListBox->AddText("", Text, SEASON3B::TYPE_SYSTEM_MESSAGE);		
	}
	GrabEnable = false;
#ifndef PBG_ADD_PCROOM_NEWUI
#ifdef FOR_WORK
#ifdef KJH_DEL_PC_ROOM_SYSTEM
	if( CPCRoomPtSys::Instance().IsPCRoom() == true )
	{
		BeginBitmap();
		g_pRenderText->SetFont(g_hFont);
		g_pRenderText->SetTextColor(255, 255, 0, 255);
		g_pRenderText->RenderText(65, 27, "PCRoom");
		EndBitmap();
	}
#endif // KJH_DEL_PC_ROOM_SYSTEM
#endif // FOR_WORK
#endif //PBG_ADD_PCROOM_NEWUI

#if defined(_DEBUG) || defined(LDS_FOR_DEVELOPMENT_TESTMODE) || defined(LDS_UNFIXED_FIXEDFRAME_FORDEBUG)
	BeginBitmap();
	// 디버그 모드에서만 FPS 찍는 부분
	unicode::t_char szDebugText[128];
#if defined(LDS_FOR_DEVELOPMENT_TESTMODE) || defined(LDS_UNFIXED_FIXEDFRAME_FORDEBUG)
	unicode::_sprintf(szDebugText, "FPS : %.1f/%.1f", FPS, g_fFrameEstimate);
#else // defined(LDS_FOR_DEVELOPMENT_TESTMODE) || defined(LDS_UNFIXED_FIXEDFRAME_FORDEBUG)
	unicode::_sprintf(szDebugText, "FPS : %.1f", FPS);
	unicode::t_char szMousePos[128];
	unicode::_sprintf(szMousePos, "MousePos : %d %d %d", MouseX, MouseY, MouseLButtonPush);
#endif // defined(LDS_FOR_DEVELOPMENT_TESTMODE) || defined(LDS_UNFIXED_FIXEDFRAME_FORDEBUG)
	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetBgColor(0, 0, 0, 100);
	g_pRenderText->SetTextColor(255, 255, 255, 200);
	//g_pRenderText->RenderText(70, 8, szDebugText);
	//g_pRenderText->RenderText(10, 40, szDebugText);
	g_pRenderText->RenderText(10, 26, szDebugText);
	g_pRenderText->RenderText(10, 36, szMousePos);
	
#ifdef CSK_HACK_TEST
	unicode::_sprintf(szDebugText, "AttackSpeed : %d", g_pHackTest->m_iAttackSpeed);
	g_pRenderText->RenderText(250, 8, szDebugText);
#endif // CSK_HACK_TEST

	g_pRenderText->SetFont(g_hFont);
	EndBitmap();
#endif // defined(_DEBUG) || defined(LDS_FOR_DEVELOPMENT_TESTMODE) || defined(LDS_UNFIXED_FIXEDFRAME_FORDEBUG)

#if defined(MR0) && defined(LDS_FOR_DEVELOPMENT_TESTMODE)
	BeginBitmap();
	// 디버그 모드에서만 FPS 찍는 부분
	unicode::t_char szDebugText_MR0[128];
	unicode::t_char szShaderQuailtyType[32];
	
	switch( g_iUseDriverType )
	{
	case GPVER_LOW:
		{
			sprintf( szShaderQuailtyType, "GPVER_LOW" );
		}
		break;
	case GPVER_MIDDLE:
		{
			sprintf( szShaderQuailtyType, "GPVER_MIDDLE" );
		}
		break;
	case GPVER_HIGH:
		{
			sprintf( szShaderQuailtyType, "GPVER_HIGH" );
		}
		break;
	case GPVER_HIGHEST:
		{
			sprintf( szShaderQuailtyType, "GPVER_HIGHEST" );
		}
		break;
	default	:
		{

		}
		break;
	}

	unicode::_sprintf(szDebugText_MR0, "RenderMODE : %s", szShaderQuailtyType);
	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetBgColor(0, 0, 0, 100);
	g_pRenderText->SetTextColor(255, 255, 255, 200);
	//g_pRenderText->RenderText(70, 8, szDebugText);
	g_pRenderText->RenderText(10, 16, szDebugText_MR0);
	g_pRenderText->SetFont(g_hFont);
	EndBitmap();
#endif // #if defined(MR0) && defined(LDS_FOR_DEVELOPMENT_TESTMODE)

#if defined(LDS_FOR_DEVELOPMENT_TESTMODE) || defined(LDS_UNFIXED_FIXEDFRAME_FORDEBUG)
	{
		unicode::t_char szDebugText[128];
		
		if( true == g_bUnfixedFixedFrame )
		{
			unicode::_sprintf(szDebugText, "UnfixedFrame(ON)");
		}
		else
		{
			unicode::_sprintf(szDebugText, "UnfixedFrame(OFF)");
		}
		BeginBitmap();
		g_pRenderText->SetFont(g_hFontBold);
		g_pRenderText->SetBgColor(0, 0, 0, 100);
		g_pRenderText->SetTextColor(255, 255, 255, 200);
		//g_pRenderText->RenderText(70, 8, szDebugText);
		g_pRenderText->RenderText(140, 16, szDebugText);
		EndBitmap();
	}
#endif // #if defined(LDS_FOR_DEVELOPMENT_TESTMODE) || defined(LDS_UNFIXED_FIXEDFRAME_FORDEBUG)


#if defined(LDS_FOR_DEVELOPMENT_TESTMODE)
	{
		unicode::t_char szDebugText[128];

#if defined(_DEBUG)
		unicode::_sprintf(szDebugText, "MODE : DEBUG");
#else // defined(_DEBUG)
		unicode::_sprintf(szDebugText, "MODE : RELEASE");
#endif // defined(_DEBUG)
		
		BeginBitmap();
		g_pRenderText->SetFont(g_hFontBold);
		g_pRenderText->SetBgColor(0, 0, 0, 100);
		g_pRenderText->SetTextColor(255, 255, 255, 200);
		//g_pRenderText->RenderText(70, 8, szDebugText);
		g_pRenderText->RenderText(250, 16, szDebugText);
		EndBitmap();
	}
#endif // #if defined(LDS_FOR_DEVELOPMENT_TESTMODE)


#ifdef DO_PROFILING
	BeginBitmap();
	{
		unicode::t_char szDebugText[128];
		// PROFILING 여부 출력 
		unicode::_sprintf(szDebugText, "Profiler(f11) : %s", 
			g_pProfiler->IsProflingNow()==TRUE?TEXT("ON"):TEXT("OFF"));
		g_pRenderText->SetFont(g_hFontBold);
		g_pRenderText->SetBgColor(0, 0, 0, 100);
		g_pRenderText->SetTextColor(255, 255, 255, 200);
		//g_pRenderText->RenderText(70, 8, szDebugText);
		g_pRenderText->RenderText(220, 8, szDebugText);
		g_pRenderText->SetFont(g_hFont);	
	}
	EndBitmap();
#endif // DO_PROFILING


#ifdef DO_PROCESS_DEBUGCAMERA

	if( g_pDebugCameraManager )
	{
		char szDebugCameraMode[32];
		memset( szDebugCameraMode, 0, sizeof(char) * 32 );
		
		EDEBUGCAMERA_TYPE eDebugCameraMode = g_pDebugCameraManager->GetActiveCameraMode();
		switch( eDebugCameraMode )
		{
		case EDEBUGCAMERA_NONE:
			{
				sprintf( szDebugCameraMode, "GAME MODE" );
			}
			break;
		case EDEBUGCAMERA_FREECONTROL:
			{
				sprintf( szDebugCameraMode, "DEBUG DEFAULT" );
			}
			break;
		case EDEBUGCAMERA_TOPVIEW:
			{
				sprintf( szDebugCameraMode, "DEBUG TOPVIEW" );
			}
			break;
		default:
			{
				sprintf( szDebugCameraMode, "GAME MODE" );
			}	
			break;
		}
		
		BeginBitmap();
		{
			unicode::t_char szDebugText[128];
			// PROFILING 여부 출력 
			unicode::_sprintf(szDebugText, "DebugCamera(SHIFT+F10),Reset(SHIFT+F9)(SHIFT+w,a,s,d) : %s", szDebugCameraMode );
			g_pRenderText->SetFont(g_hFontBold);
			g_pRenderText->SetBgColor(0, 0, 0, 100);
			g_pRenderText->SetTextColor(255, 255, 255, 200);
			//g_pRenderText->RenderText(70, 8, szDebugText);
			g_pRenderText->RenderText(320, 8, szDebugText);
			g_pRenderText->SetFont(g_hFont);	
		}
		EndBitmap();

		/*
		if( (int)EDEBUGCAMERA_NONE < (int)g_pDebugCameraManager->GetCurrentActivity() && 
			(int)EDEBUGCAMERA_END > (int)g_pDebugCameraManager->GetCurrentActivity() )
		{
			// Debug Camera Position
			BeginBitmap();
			{
				unicode::t_char szDebugText[128];
				CDebugCameraInterface *pCurDebugCam = g_pDebugCameraManager->GetActiveDebugCamera();
				
				// PROFILING 여부 출력 
				unicode::_sprintf(szDebugText, "DebugCamera Position : %f,%f,%f", 
					pCurDebugCam->m_vCameraPos[0], 
					pCurDebugCam->m_vCameraPos[1], 
					pCurDebugCam->m_vCameraPos[2] );
				g_pRenderText->SetFont(g_hFontBold);
				g_pRenderText->SetBgColor(0, 0, 0, 100);
				g_pRenderText->SetTextColor(255, 255, 255, 200);
				g_pRenderText->RenderText(320, 28, szDebugText);
				g_pRenderText->SetFont(g_hFont);	
			}
			EndBitmap();
			
			BeginBitmap();
			{
				unicode::t_char szDebugText[128];
				CDebugCameraInterface *pCurDebugCam = g_pDebugCameraManager->GetActiveDebugCamera();
				
				// PROFILING 여부 출력 
				unicode::_sprintf(szDebugText, "DebugCamera Angle : %f,%f,%f", 
					pCurDebugCam->m_vAngle[0], pCurDebugCam->m_vAngle[1], pCurDebugCam->m_vAngle[2] );
				g_pRenderText->SetFont(g_hFontBold);
				g_pRenderText->SetBgColor(0, 0, 0, 100);
				g_pRenderText->SetTextColor(255, 255, 255, 200);
				g_pRenderText->RenderText(320, 48, szDebugText);
				g_pRenderText->SetFont(g_hFont);	
			}
			EndBitmap();
			
			BeginBitmap();
			{
				unicode::t_char szDebugText[128];
				CDebugCameraInterface *pCurDebugCam = g_pDebugCameraManager->GetActiveDebugCamera();
				
				// PROFILING 여부 출력 
				unicode::_sprintf(szDebugText, "DebugCamera LookAt : %f,%f,%f", 
					pCurDebugCam->m_vLookAt[0], pCurDebugCam->m_vLookAt[1], pCurDebugCam->m_vLookAt[2] );
				g_pRenderText->SetFont(g_hFontBold);
				g_pRenderText->SetBgColor(0, 0, 0, 100);
				g_pRenderText->SetTextColor(255, 255, 255, 200);
				g_pRenderText->RenderText(320, 68, szDebugText);
				g_pRenderText->SetFont(g_hFont);	
			}
			EndBitmap();
		}

		*/
	}
#endif // DO_PROCESS_DEBUGCAMERA

#ifdef USE_SELFCHECKCODE
	SendCrcOfFunction( 11, 1, MoveJoint, 0x5B02);
#endif
	if(Success)
	{
		glFlush();
		SwapBuffers(hDC);
	}
	//////////////// 랜더링 끝 //////////////////////////////////////////////////////////

#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_RENDER_SCENE_TOTAL );
#endif // DO_PROFILING
	
	// 사용자 의사 결정에 의한 고정 20프레임을 Unfixed 또는 Fixed 할 수 있도록 합니다.
#if defined(LDS_FOR_DEVELOPMENT_TESTMODE) || defined(LDS_UNFIXED_FIXEDFRAME_FORDEBUG)

	// * 초당 프레임 계산. 추정치.
	static unsigned int uiCounting = 0;
	static unsigned int uiStarttime = 0;
	static unsigned int uiSumOffsetTime = 0;

	const unsigned int uiCalculatePerMS = 1000;

	unsigned int uiCurrentTime = GetTickCount();
	unsigned int uiOffsetCurrent = uiCurrentTime-TimePrior;

	if( uiStarttime == 0 )
	{
		uiStarttime = uiCurrentTime;
		uiSumOffsetTime = 0;
		uiCounting = 0;
	}

	if( uiOffsetCurrent > 0 )
	{
		uiSumOffsetTime += uiOffsetCurrent;
		++uiCounting;
	}

	unsigned int uiOffsetSum = uiCurrentTime - uiStarttime;

	if( uiOffsetSum > uiCalculatePerMS && uiSumOffsetTime)
	{
		unsigned int uiSumOffsetTime__ = uiSumOffsetTime;
		unsigned int uiCounting__ = uiCounting;

		g_fFrameEstimate = 1000 / ( uiSumOffsetTime / uiCounting );

		uiStarttime = 0;
	}

	if( g_fFrameEstimate < 1 )
	{
		g_fFrameEstimate = 1;
	}


	// * 고정20프레임 을 사용할지 말지 여부.
	if(false==g_bUnfixedFixedFrame)	// 20 고정 프레임
	{
#ifdef LDS_MR0_MOD_FIXEDFRAME_DELTA
		const unsigned int MAXTIMESTANDBY = 40;
		const unsigned int MAXFIXEDFRAME = 20;
		unsigned int MAXACCUMULATIONSUM = MAXTIMESTANDBY * (MAXFIXEDFRAME - 1);

		if(listAccumulateTime.size() >= 19)
		{
			unsigned int uiTimeDeltaAccumulation = 0, uiCurrentTimeDelta = 0;
			BOOL bCalculateStandbyTime = FALSE;

			LISTACCUMULATETIME::iterator iterTimeDelta;

			for( iterTimeDelta = listAccumulateTime.begin(); iterTimeDelta != listAccumulateTime.end(); ++iterTimeDelta )
			{
				uiCurrentTimeDelta = (*iterTimeDelta);

				uiTimeDeltaAccumulation += uiCurrentTimeDelta;
			}

			unsigned int uiTimeStandBy__ = MAXTIMESTANDBY;
			int iOffsetTimeDelta = uiTimeDeltaAccumulation - MAXACCUMULATIONSUM;

			if( iOffsetTimeDelta > 0 )
			{
				bCalculateStandbyTime = FALSE;	// 들어오는지 재확인.
			}
			else
			{
				bCalculateStandbyTime = TRUE;
			}
			
			// 한 Tick을 적어도 0.04초로 만듭니다. 
			if( bCalculateStandbyTime == TRUE )
			{
				while(TimeRender < MAXTIMESTANDBY)
				{
					TimeRender = GetTickCount()-TimePrior;
				}
			}

			while( listAccumulateTime.size() > 19 )
			{
				listAccumulateTime.pop_front();
			}
		}
		else
		{
			// 한 Tick을 적어도 0.04초로 만듭니다.
			while(TimeRender < MAXTIMESTANDBY)
			{
				TimeRender = GetTickCount()-TimePrior;
			}
		}
		listAccumulateTime.push_back( uiOffsetCurrent );
		
#else // LDS_MR0_MOD_FIXEDFRAME_DELTA
		TimeRender = uiOffsetCurrent;
		
		while(TimeRender < 40)
		{
			TimeRender = GetTickCount()-TimePrior;
		}
 #endif // LDS_MR0_MOD_FIXEDFRAME_DELTA
	}
#else // defined(LDS_FOR_DEVELOPMENT_TESTMODE) || defined(LDS_UNFIXED_FIXEDFRAME_FORDEBUG)
    TimeRender = GetTickCount()-TimePrior;
    while(TimeRender < 40)
	{
		TimeRender = GetTickCount()-TimePrior;
	}
#endif // defined(LDS_FOR_DEVELOPMENT_TESTMODE) || defined(LDS_UNFIXED_FIXEDFRAME_FORDEBUG)

#ifdef _DEBUG
	TimeRemain += TimeRender * ( g_bTimeTurbo ? 4 : 1);
#else
    TimeRemain += TimeRender;
#endif

#ifdef _DEBUG
#ifdef KWAK_FIX_KEY_STATE_RUNTIME_ERR
	if(SEASON3B::IsRepeat(VK_CONTROL) == TRUE && SEASON3B::IsPress(VK_SPACE) == TRUE)
#else // KWAK_FIX_KEY_STATE_RUNTIME_ERR
	if(HIBYTE(GetAsyncKeyState(VK_CONTROL)) && 128==HIBYTE(GetAsyncKeyState(VK_SPACE)))
#endif // KWAK_FIX_KEY_STATE_RUNTIME_ERR
	{
		g_bTimeTurbo = !g_bTimeTurbo;
	}
#endif
	if(EnableSocket && SceneFlag==MAIN_SCENE)
	{
		if( SocketClient.GetSocket()==INVALID_SOCKET)
		{
			static BOOL s_bClosed = FALSE;
			if ( !s_bClosed)
			{
				s_bClosed = TRUE;
				g_ErrorReport.Write( "> Connection closed. ");
				g_ErrorReport.WriteCurrentTime();
				SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CServerLostMsgBoxLayout));
			}
		}
	}

	if(SceneFlag == MAIN_SCENE)
	{
		switch(World)
		{
		case WD_0LORENCIA:
			if(HeroTile==4)
			{
				StopBuffer(SOUND_WIND01,true);
				StopBuffer(SOUND_RAIN01,true);
			}
			else
			{
				PlayBuffer(SOUND_WIND01,NULL,true);
				if(RainCurrent>0)
					PlayBuffer(SOUND_RAIN01,NULL,true);
			}
			break;
		case WD_1DUNGEON:
			PlayBuffer(SOUND_DUNGEON01,NULL,true);
			break;
		case WD_2DEVIAS:
			if(HeroTile==3 || HeroTile>=10)
				StopBuffer(SOUND_WIND01,true);
			else
				PlayBuffer(SOUND_WIND01,NULL,true);
			break;
		case WD_3NORIA:
			PlayBuffer(SOUND_WIND01,NULL,true);
			if(rand()%512==0)
				PlayBuffer(SOUND_FOREST01);
			break;
		case WD_4LOSTTOWER:
			PlayBuffer(SOUND_TOWER01,NULL,true);
			break;
		case WD_5UNKNOWN:
			//PlayBuffer(SOUND_BOSS01,NULL,true);
			break;
		case WD_7ATLANSE:
			PlayBuffer(SOUND_WATER01,NULL,true);
			break;
		case WD_8TARKAN:
			PlayBuffer(SOUND_DESERT01,NULL,true);
			break;
        case WD_10HEAVEN:
            PlayBuffer(SOUND_HEAVEN01,NULL,true);
            if( (rand()%100)==0 )
            {
//                PlayBuffer(SOUND_HEAVEN01);
            }
            else if( (rand()%10)==0 )
            {
//                PlayBuffer(SOUND_THUNDERS02);
            }
            break;
#ifdef CSK_ADD_MAP_ICECITY	
		case WD_58ICECITY_BOSS:
			PlayBuffer(SOUND_WIND01, NULL, true);
			break;
#endif // CSK_ADD_MAP_ICECITY
#ifdef LDS_ADD_SOUND_UNITEDMARKETPLACE
		case WD_79UNITEDMARKETPLACE:
			{
				PlayBuffer(SOUND_WIND01,NULL,true);
				PlayBuffer(SOUND_RAIN01,NULL,true);
			}
			break;
#endif // LDS_ADD_SOUND_UNITEDMARKETPLACE
#ifdef ASG_ADD_MAP_KARUTAN
		case WD_80KARUTAN1:
			PlayBuffer(SOUND_KARUTAN_DESERT_ENV, NULL, true);
			break;
		case WD_81KARUTAN2:
			if (HeroTile == 12)	// 투명타일 위에서
			{
				StopBuffer(SOUND_KARUTAN_DESERT_ENV, true);
				PlayBuffer(SOUND_KARUTAN_KARDAMAHAL_ENV, NULL, true);
			}
			else
			{
				StopBuffer(SOUND_KARUTAN_KARDAMAHAL_ENV, true);
				PlayBuffer(SOUND_KARUTAN_DESERT_ENV, NULL, true);
			}
			break;
#endif	// ASG_ADD_MAP_KARUTAN
		}
		if(World != WD_0LORENCIA && World != WD_2DEVIAS && World != WD_3NORIA
#ifdef CSK_ADD_MAP_ICECITY	
			&& World != WD_58ICECITY_BOSS
#endif // CSK_ADD_MAP_ICECITY	
#ifdef LDS_ADD_SOUND_UNITEDMARKETPLACE
			&& World != WD_79UNITEDMARKETPLACE
#endif // LDS_ADD_SOUND_UNITEDMARKETPLACE
			)
		{
			StopBuffer(SOUND_WIND01,true);
		}
		if ( World != WD_0LORENCIA && InDevilSquare() == false
#ifdef LDS_ADD_SOUND_UNITEDMARKETPLACE
			 && World != WD_79UNITEDMARKETPLACE
#endif // LDS_ADD_SOUND_UNITEDMARKETPLACE
			)
		{
			StopBuffer(SOUND_RAIN01,true);
		}
		if(World != WD_1DUNGEON)
		{
			StopBuffer(SOUND_DUNGEON01,true);
		}
		if(World != WD_3NORIA)
		{
			StopBuffer(SOUND_FOREST01,true);
		}
		if(World != WD_4LOSTTOWER)
		{
			StopBuffer(SOUND_TOWER01,true);
		}
		if(World != WD_7ATLANSE)
		{
			StopBuffer(SOUND_WATER01,true);
		}
		if(World != WD_8TARKAN)
		{
			StopBuffer(SOUND_DESERT01,true);
		}
        if(World != WD_10HEAVEN)
		{
            StopBuffer(SOUND_HEAVEN01,true);
		}
		if(World != WD_51HOME_6TH_CHAR)
		{
			StopBuffer(SOUND_ELBELAND_VILLAGEPROTECTION01, true);
			StopBuffer(SOUND_ELBELAND_WATERFALLSMALL01, true);
			StopBuffer(SOUND_ELBELAND_WATERWAY01, true);
			StopBuffer(SOUND_ELBELAND_ENTERDEVIAS01, true);
			StopBuffer(SOUND_ELBELAND_WATERSMALL01, true);
			StopBuffer(SOUND_ELBELAND_RAVINE01, true);
			StopBuffer(SOUND_ELBELAND_ENTERATLANCE01, true);
		}
#ifdef ASG_ADD_MAP_KARUTAN
		if (!IsKarutanMap())
			StopBuffer(SOUND_KARUTAN_DESERT_ENV, true);
		if (World != WD_80KARUTAN1)
			StopBuffer(SOUND_KARUTAN_INSECT_ENV, true);
		if (World != WD_81KARUTAN2)
			StopBuffer(SOUND_KARUTAN_KARDAMAHAL_ENV, true);
#endif	// ASG_ADD_MAP_KARUTAN

		if(World==WD_0LORENCIA)
		{
			if(Hero->SafeZone)
			{
				if(HeroTile==4)
					PlayMp3( g_lpszMp3[MUSIC_PUB]);
				else
					PlayMp3( g_lpszMp3[MUSIC_MAIN_THEME]);
			}
		}
		else
		{	
			StopMp3( g_lpszMp3[MUSIC_PUB]);
			StopMp3( g_lpszMp3[MUSIC_MAIN_THEME]);
		}
		if(World==WD_2DEVIAS)	
		{
			if(Hero->SafeZone)
			{
				// 크리스마스 사운드 넣을 곳
				if(( Hero->PositionX)>=205 && ( Hero->PositionX)<=214 &&
					( Hero->PositionY)>=13 && ( Hero->PositionY)<=31)
                {
					PlayMp3( g_lpszMp3[MUSIC_CHURCH]);
                }
				else
                {
                    PlayMp3( g_lpszMp3[MUSIC_DEVIAS]);
                }
			}
		}
		else
		{
			StopMp3( g_lpszMp3[MUSIC_CHURCH]);
			StopMp3( g_lpszMp3[MUSIC_DEVIAS]);
		}
		if(World==WD_3NORIA)
		{
			if(Hero->SafeZone)
				PlayMp3( g_lpszMp3[MUSIC_NORIA]);
		}
		else
		{
			StopMp3( g_lpszMp3[MUSIC_NORIA]);
		}
		if(World==WD_1DUNGEON || World==WD_5UNKNOWN)
		{
			PlayMp3( g_lpszMp3[MUSIC_DUNGEON]);
		}
		else
		{
			StopMp3( g_lpszMp3[MUSIC_DUNGEON]);
		}

		if(World==WD_7ATLANSE) {
			PlayMp3(g_lpszMp3[MUSIC_ATLANS]);
		}
		else {
			StopMp3(g_lpszMp3[MUSIC_ATLANS]);
		}
		if(World==WD_10HEAVEN) {
			PlayMp3(g_lpszMp3[MUSIC_ICARUS]);
		}
		else {
			StopMp3(g_lpszMp3[MUSIC_ICARUS]);
		}
		if(World==WD_8TARKAN) {
			PlayMp3(g_lpszMp3[MUSIC_TARKAN]);
		}
		else {
			StopMp3(g_lpszMp3[MUSIC_TARKAN]);
		}
		if(World==WD_4LOSTTOWER) {
			PlayMp3(g_lpszMp3[MUSIC_LOSTTOWER_A]);
		}
		else {
			StopMp3(g_lpszMp3[MUSIC_LOSTTOWER_A]);
		}

		if(InHellas(World)) {
			PlayMp3(g_lpszMp3[MUSIC_KALIMA]);
		}
		else {
			StopMp3(g_lpszMp3[MUSIC_KALIMA]);
		}

		if(World==WD_31HUNTING_GROUND) {
			PlayMp3(g_lpszMp3[MUSIC_BC_HUNTINGGROUND]);
		}
		else {
			StopMp3(g_lpszMp3[MUSIC_BC_HUNTINGGROUND]);
		}

		if(World==WD_33AIDA) {
			PlayMp3(g_lpszMp3[MUSIC_BC_ADIA]);
		}
		else {
			StopMp3(g_lpszMp3[MUSIC_BC_ADIA]);
		}

		M34CryWolf1st::ChangeBackGroundMusic(World);
		M39Kanturu3rd::ChangeBackGroundMusic(World);

		if (World == WD_37KANTURU_1ST)
			PlayMp3(g_lpszMp3[MUSIC_KANTURU_1ST]);
		else
			StopMp3(g_lpszMp3[MUSIC_KANTURU_1ST]);
		M38Kanturu2nd::PlayBGM();
		SEASON3A::CGM3rdChangeUp::Instance().PlayBGM();
		if( g_CursedTemple->IsCursedTemple() )
		{
			g_CursedTemple->PlayBGM();
		}
		if(World==WD_51HOME_6TH_CHAR) {
			PlayMp3(g_lpszMp3[MUSIC_ELBELAND]);
		}
		else {
			StopMp3(g_lpszMp3[MUSIC_ELBELAND]);
		}

		if(World==WD_56MAP_SWAMP_OF_QUIET) {
			PlayMp3(g_lpszMp3[MUSIC_SWAMP_OF_QUIET]);
		}
		else {
			StopMp3(g_lpszMp3[MUSIC_SWAMP_OF_QUIET]);
		}

#ifdef CSK_ADD_MAP_ICECITY	
		g_Raklion.PlayBGM();
#endif // CSK_ADD_MAP_ICECITY
#ifdef YDG_ADD_MAP_SANTA_TOWN
		g_SantaTown.PlayBGM();
#endif	// YDG_ADD_MAP_SANTA_TOWN
#ifdef PBG_ADD_PKFIELD
		g_PKField.PlayBGM();
#endif //PBG_ADD_PKFIELD
#ifdef YDG_ADD_MAP_DOPPELGANGER1
		g_DoppelGanger1.PlayBGM();
#endif	// YDG_ADD_MAP_DOPPELGANGER1
#ifdef LDS_ADD_EMPIRE_GUARDIAN
		g_EmpireGuardian1.PlayBGM();
		g_EmpireGuardian2.PlayBGM();
		g_EmpireGuardian3.PlayBGM();
		g_EmpireGuardian4.PlayBGM();
#endif //LDS_ADD_EMPIRE_GUARDIAN
#ifdef LDS_ADD_MAP_UNITEDMARKETPLACE
		g_UnitedMarketPlace.PlayBGM();
#endif // LDS_ADD_MAP_UNITEDMARKETPLACE
#ifdef ASG_ADD_MAP_KARUTAN
		g_Karutan1.PlayBGM();
#endif	// ASG_ADD_MAP_KARUTAN
	}

#ifdef USE_SELFCHECKCODE
	SendCrcOfFunction( 21, 10, SkillElf, 0x818F);
#endif

#ifdef FOR_DRAMA
	extern int HeroIndex;
	if ( PressKey( VK_F3))
	{
		g_iGemCount += 10000;
	}
	if ( g_iGemCount > 0)
	{
		for ( int i = 0; i < 30 && g_iGemCount > 0; ++i, --g_iGemCount)
		{
			ITEM_t *ip = &Items[g_iItemIndex++];
			vec3_t Position;
			memcpy( &Position, &CharactersClient[HeroIndex].Object.Position, sizeof ( vec_t));
			Position[0]  = (float)(rand() % 21 - 10 + CharactersClient[HeroIndex].PositionX+0.5f)*TERRAIN_SCALE + ( float)( rand() % 201 - 100) * 0.3f;
			Position[1]  = (float)(rand() % 21 - 10 + CharactersClient[HeroIndex].PositionY+0.5f)*TERRAIN_SCALE + ( float)( rand() % 201 - 100) * 0.3f;
			CreateItem(ip,MODEL_ITEM+ITEM_POTION+13,0,0,Position,TRUE);

			g_iItemIndex %= MAX_ITEMS;
			if ( 0 == i)
			{
				PlayBuffer(SOUND_JEWEL01,&ip->Object);
			}
		}
		g_iShowGoodLuck += 1000;
	}
#endif
#ifdef USE_SELFCHECKCODE
	END_OF_FUNCTION( Pos_SelfCheck01);
Pos_SelfCheck01:
	;
#endif

#ifdef LDS_ADD_MULTISAMPLEANTIALIASING
	SetEnableMultisample();
#endif // LDS_ADD_MULTISAMPLEANTIALIASING

#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_MAINSCENE_TOTAL );
#endif // DO_PROFILING
}

///////////////////////////////////////////////////////////////////////////////
// 씬 관리 함수 (윈도우 메세지 루프에서 실행함)
///////////////////////////////////////////////////////////////////////////////

float g_Luminosity;

extern int g_iNoMouseTime;
extern GLvoid KillGLWindow(GLvoid);


//void StartDevilSquareCountDown( int iType);	// 임시

void Scene(HDC hDC)
{
#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_SCENE_TOTAL, PROFILING_SCENE_TOTAL );
#endif // DO_PROFILING
	g_Luminosity = sinf(WorldTime*0.004f)*0.15f+0.6f;
	switch(SceneFlag)
	{
#ifdef MOVIE_DIRECTSHOW
	case MOVIE_SCENE:
		MovieScene(hDC);
		break;
#endif // MOVIE_DIRECTSHOW
	case WEBZEN_SCENE://웹젠 로고 나오는 씬
        WebzenScene(hDC);
		break;
	case LOADING_SCENE://로딩 씬
      	LoadingScene(hDC);
		break;
	case LOG_IN_SCENE://계정 입력받는 씬
	case CHARACTER_SCENE://케릭터 선택하는 씬
	case MAIN_SCENE://메인 씬
		MainScene(hDC);
		break;
	}
#ifdef USE_SELFCHECKCODE
	SendCrcOfFunction( 6, 3, ProtocolCompiler, 0xDA3F);
#endif

#ifdef ANTIHACKING_ENABLE
	// 체크코드
	int iFrameCount = ( GetTickCount() / 40);
	if ( g_iPrevFrameCount != iFrameCount)
	{
		g_iPrevFrameCount = iFrameCount;
		g_bNewFrame = TRUE;
	}
	else
	{
		g_bNewFrame = FALSE;
	}
	if ( g_bNewFrame)
	{
		switch ( iFrameCount % 150)
		{
		case 21:
			hanguo_check7();
			break;
		case 23:
			hanguo_check8();
			break;
		case 57:
			hanguo_check9();
			break;
		case 89:
			hanguo_check10();
			break;
		case 113:
			hanguo_check11();
			break;
		case 137:
			hanguo_check12();
			break;
		}
	}
#endif //ANTIHACKING_ENABLE

#ifdef USE_SELFCHECKCODE
	SendCrcOfFunction( 12, 16, Attack, 0x7329);
#endif //USE_SELFCHECKCODE

	_asm { jmp Pos_NoMouseTimeCheck2 }
	_asm { __emit 0xFF }
	_asm { __emit 0x15 }
Pos_NoMouseTimeCheck2:
	if ( g_iNoMouseTime > 31)
	{	// g_iNoMouseTime 체크를 건너뛰게 조작한 경우
		SendHackingChecked( 0x02, 0);
		FAKE_CODE( Pos_NoMouse_KillWindow);
Pos_NoMouse_KillWindow:
		KillGLWindow();
	}
#ifdef USE_SELFCHECKCODE
	SendCrcOfFunction( 17, 6, SkillWarrior, 0xAA83);
#endif

#ifndef PSW_BUGFIX_IME
	g_bEnterPressed = false;
#endif //PSW_BUGFIX_IME

#ifdef USE_SELFCHECKCODE
	END_OF_FUNCTION( Pos_SelfCheck01);
Pos_SelfCheck01:
	;
#endif

#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_SCENE_TOTAL );
#endif // DO_PROFILING	
}

bool	GetTimeCheck(int DelayTime)
{
	int PresentTime = timeGetTime();
	
	if(g_bTimeCheck)
	{
		g_iBackupTime = PresentTime;
		g_bTimeCheck = false;
	}

	if(g_iBackupTime+DelayTime <= PresentTime)
	{
		g_bTimeCheck = true;
		return true;
	}
	return false;	
}