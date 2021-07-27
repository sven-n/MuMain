#include "stdafx.h"
#include "UIManager.h"
#include "GuildCache.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"
#include "ZzzLodTerrain.h"
#include "ZzzAI.h"
#include "ZzzTexture.h"
#include "ZzzEffect.h"
#include "ZzzOpenglUtil.h"
#include "ZzzOpenData.h"
#include "ZzzScene.h"
#include "wsclientinline.h"
#include "DSPlaySound.h"
#include "./Utilities/Log/DebugAngel.h"
#include "./Utilities/Log/ErrorReport.h"
#include "./Utilities/Memory/MemoryLock.h"
#include "MatchEvent.h"
#include "GOBoid.h"
#ifdef NP_GAME_GUARD
	#include "ExternalObject/Nprotect/npGameGuard.h"
#endif // NP_GAME_GUARD
#ifdef SAVE_PACKET
#include "./ExternalObject/leaf/stdleaf.h"
#endif // SAVE_PACKET
#include "CSQuest.h"
#include "PersonalShopTitleImp.h"
#include "PvPSystem.h"
#include "GMHellas.h"
#include "npcBreeder.h"
#ifdef PET_SYSTEM
#include "GIPetManager.h"
#endif// PET_SYSTEM
#include "CSMapServer.h"
#include "GMBattleCastle.h"
#include "npcGateSwitch.h"
#include "CSMapInterface.h"
#include "CComGem.h"
#include "UIMapName.h" // rozy
#include "UIMng.h"
#include "GMCrywolf1st.h"
#include "CDirection.h"
#include "GM_Kanturu_3rd.h"
#include "CSParts.h"
#include "PhysicsManager.h"
#include "Event.h"
#ifdef ADD_PCROOM_POINT_SYSTEM
#include "PCRoomPoint.h"
#endif	// ADD_PCROOM_POINT_SYSTEM
#include "MixMgr.h"

#ifdef RESOURCE_GUARD
#include "./ExternalObject/ResourceGuard/MuRGReport.h"
#include "./ExternalObject/ResourceGuard/ResourceGuard.h"
#endif // RESOURCE_GUARD
#include "UIGuardsMan.h"
#include "NewUISystem.h"
#include "NewUICommonMessageBox.h"
#include "NewUICustomMessageBox.h"
#include "NewUIInventoryCtrl.h"
#include "w_CursedTemple.h"
#include "SummonSystem.h"

#ifdef PSW_ADD_MAPSYSTEM
#include "w_MapHeaders.h"
#endif // PSW_ADD_MAPSYSTEM

#ifdef LDK_ADD_NEW_PETPROCESS
#include "w_PetProcess.h"
#endif //LDK_ADD_NEW_PETPROCESS
#ifdef YDG_ADD_CS5_REVIVAL_CHARM
#include "PortalMgr.h"
#endif	// YDG_ADD_CS5_REVIVAL_CHARM
#ifdef CSK_HACK_TEST
#include "HackTest.h"
#endif // CSK_HACK_TEST
#ifdef YDG_ADD_NEW_DUEL_SYSTEM
#include "DuelMgr.h"
#endif	// YDG_ADD_NEW_DUEL_SYSTEM

#ifdef LDK_ADD_GAMBLE_SYSTEM
#include "GambleSystem.h"
#endif //LDK_ADD_GAMBLE_SYSTEM
#ifdef YDG_MOD_PROTECT_AUTO_V5
#include "ProtectAuto.h"
#endif	// YDG_MOD_PROTECT_AUTO_V5
#ifdef ASG_ADD_NEW_QUEST_SYSTEM
#include "QuestMng.h"
#endif	// ASG_ADD_NEW_QUEST_SYSTEM
#ifdef PBG_ADD_SECRETBUFF
#include "FatigueTimeSystem.h"
#endif //PBG_ADD_SECRETBUFF
#ifdef PBG_ADD_PKSYSTEM_INGAMESHOP
#include "PKSystem.h"
#endif //PBG_ADD_PKSYSTEM_INGAMESHOP
#ifdef PBG_ADD_CHARACTERSLOT
#include "Slotlimit.h"
#endif //PBG_ADD_CHARACTERSLOT
#ifdef KJH_ADD_SERVER_LIST_SYSTEM
#include "ServerListManager.h"
#endif // KJH_ADD_SERVER_LIST_SYSTEM
#ifdef KJH_ADD_DUMMY_SKILL_PROTOCOL
#include "DummyAttackProtocol.h"
#endif // KJH_ADD_DUMMY_SKILL_PROTOCOL
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
#include "MonkSystem.h"
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
#ifdef LDK_ADD_SCALEFORM
#include "CGFxProcess.h"
#endif //LDK_ADD_SCALEFORM
#ifdef LEM_ADD_GAMECHU
#include "GCCertification.h"
#endif // LEM_ADD_GAMECHU

//////////////////////////////////////////////////////////////////////////
//  DEFINE.
//////////////////////////////////////////////////////////////////////////
#define MAX_DEBUG_MAX 10


//////////////////////////////////////////////////////////////////////////
//  EXTERN.
//////////////////////////////////////////////////////////////////////////

extern BYTE m_AltarState[];

///////////////////////////////////////////
extern int g_iChatInputType;
extern BOOL g_bUseChatListBox;

#ifdef WINDOWMODE
extern BOOL g_bUseWindowMode;
#endif //WINDOWMODE
#ifdef SCRATCH_TICKET
extern CUITextInputBox * g_pSingleTextInputBox;
#endif // SCRATCH_TICKET

extern CChatRoomSocketList * g_pChatRoomSocketList;
extern CGuildCache g_GuildCache;
///////////////////////////////////////////

#ifdef _PVP_ADD_MOVE_SCROLL
extern CMurdererMove g_MurdererMove;
#endif	// _PVP_ADD_MOVE_SCROLL

#ifdef _PVP_DYNAMIC_SERVER_TYPE
extern BOOL g_bIsCurrentServerPvP;
#endif	// _PVP_DYNAMIC_SERVER_TYPE

extern  short   g_shCameraLevel;

extern BYTE DebugText[MAX_DEBUG_MAX][256];
extern int  DebugTextLength[MAX_DEBUG_MAX];
extern char DebugTextCount;
extern int  TotalPacketSize;
extern int g_iKeyPadEnable;

extern CUIMapName* g_pUIMapName; // rozy

#ifndef YDG_ADD_NEW_DUEL_SYSTEM		// 정리할때 삭제해야 함
extern bool g_PetEnableDuel;	// LHJ - 결투중에 다크로드의 다크스피릿이 공격중 인지 판단하는 변수
#endif	// YDG_ADD_NEW_DUEL_SYSTEM	// 정리할때 삭제해야 함

MASTER_LEVEL_VALUE	Master_Level_Data;
//////////////////////////////////////////////////////////////////////////
//!! 버전과 시리얼
//////////////////////////////////////////////////////////////////////////

#if SELECTED_LANGUAGE == LANGUAGE_KOREAN		// 한국
#ifdef _ORIGINAL_SERVER
//********************블루 섭이 아닌 본섭의 경우 해당 한다********************//

//-----------------------------------> 사내 서버 버젼
//2007.12.07 테섭 (안상규)
BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '4'+3, '1'+4, '2'+5};
BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"wpqkf3rcsgkma2vo"};
//-----------------------------------------------------------------------------------
//2010.11.04 본섭 (김재희)
// BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '8'+3, '0'+4, '4'+5};
// BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"ddf09j3ohDgLr7lS"};
//-----------------------------------------------------------------------------------
//2010.11.03 테섭 (김재희)
// BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '8'+3, '0'+4, '3'+5};
// BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"llRkkdc0mFw2V79l"};
//-----------------------------------------------------------------------------------
//2010.11.02 테섭 (김재희)
// BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '8'+3, '0'+4, '2'+5};
// BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"ms8Tglat7ddsFkH2"};
//-----------------------------------------------------------------------------------
//2010.10.29 본섭 (김재희)
// BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '8'+3, '0'+4, '1'+5};
// BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"l1SwdpfTH9rWh2pn"};
//-----------------------------------------------------------------------------------
//2010.10.28 본섭 (김재희)
// BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '8'+3, '0'+4, '0'+5};
// BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"jklf3qbw5flfQjs6"};
//-----------------------------------------------------------------------------------
//2010.10.27 테섭 (김재희)
// BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '7'+3, '5'+4, '2'+5};
// BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"1dfclvlSo2aOJ7ik"};
//-----------------------------------------------------------------------------------
//2010.10.21 본섭 (김재희)
// BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '7'+3, '5'+4, '1'+5};
// BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"0sa6f5sJkshflEAn"};
//-----------------------------------------------------------------------------------
//2010.10.15 테섭 (김재희)
// BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '7'+3, '5'+4, '0'+5};
// BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"a5sc0dTmtuk1tslq"};
//-----------------------------------------------------------------------------------
//2010.10.05 본섭 (김재희)
// BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '7'+3, '4'+4, '8'+5};
// BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"sI8rk9srd4Klakgh"};
//-----------------------------------------------------------------------------------
//2010.10.05 테섭 (김재희)
// BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '7'+3, '4'+4, '7'+5};
// BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"7C5fS3mklm8eKtvF"};
//-----------------------------------------------------------------------------------
#else //_ORIGINAL_SERVER
//********************블루 섭의 경우 해당 한다********************//
//본섭 패치와 같은 형태의 버젼 관리로 패치한다
//국가는 LANGUAGE_KOREAN 이 된다
//===================================> 블루 사내 서버 버젼
BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '4'+3, '1'+4, '2'+5};
BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"wpqkf3rcsgkma2vo"};
//-----------------------------------------------------------------------------------
//2010.11.04 블루 (김재희)
// BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '0'+3, '4'+4, '3'+5};
// BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"skfswk7Lflk12QVE"};
//-----------------------------------------------------------------------------------
//2010.10.28 블루 (김재희)
// BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '0'+3, '4'+4, '2'+5};
// BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"ul7AA3gakAkkAak7"};
//-----------------------------------------------------------------------------------
//2010.10.28 블루 (김재희)
// BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '0'+3, '4'+4, '1'+5};
// BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"gshfkHs29kds4KeA"};
//-----------------------------------------------------------------------------------
//2010.10.21 블루 (김재희)
// BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '0'+3, '4'+4, '0'+5};
// BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"8Rs9Kfqlfseso2kL"};
//-----------------------------------------------------------------------------------
//2010.10.07 블루 (김재희)
// BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '0'+3, '3'+4, '8'+5};
// BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"w0sfeH3okd5ahgr6"};
//-----------------------------------------------------------------------------------
#endif //_ORIGINAL_SERVER
//-------------------------------------------------------------------------------------------------------------
#elif SELECTED_LANGUAGE == LANGUAGE_TAIWANESE	// 대만

//-----------------------------------> 사내 서버 버젼
BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1,'0'+2,'2'+3, '1'+4, '8'+5};
BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"pmdprdwjs0Pwh3wl"};
//-----------------------------------------------------------------------------------
//2009.07.22 본섭 (이동근)
//BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '3'+3, '0'+4, '5'+5};
//BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"26NwYW4BIQvIZUEr"};
 //-----------------------------------------------------------------------------------
//2010.10.11 본섭 (김재우)
//BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '3'+3, '1'+4, '0'+5};
//BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"26NwYwklIQvI23Er"};
//-------------------------------------------------------------------------------------------------------------
#elif SELECTED_LANGUAGE == LANGUAGE_JAPANESE	// 일본

//-----------------------------------> 사내 서버 버젼
//2008.09.09 본섭 (이동근)
BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '2'+3, '2'+4, '6'+5};
BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"DeL7QLE08KXTySe7"};
//-----------------------------------------------------------------------------------
//2009.07.08 본섭 (김재희)
// BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '3'+3, '0'+4, '9'+5};
// BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"jDivfh58dhQi2Vo8"};
//-----------------------------------------------------------------------------------
//2010.08.24 본섭 (이은미)
//BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '3'+3, '2'+4, '8'+5};
//BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"efElef2ERa2eioek"};
//-----------------------------------------------------------------------------------
//2010.09.09 본섭 (이은미)
// BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '3'+3, '2'+4, '9'+5};
// BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"efieef3Eaefeioek"};
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------
#elif SELECTED_LANGUAGE == LANGUAGE_PHILIPPINES	// 필리핀

//-----------------------------------> 사내 서버 버젼
BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1,'0'+2,'2'+3, '0'+4, '8'+5};
BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"CCoosSCKNyaalvMD"};
//-----------------------------------------------------------------------------------
// BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1,'0'+2,'3'+3, '0'+4, '2'+5};
// BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"Dhkswjstofh1rka4"};
//-----------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------
#elif SELECTED_LANGUAGE == LANGUAGE_CHINESE		// 중국

//-----------------------------------> 사내 서버 버젼
BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '3'+3, '0'+4, '6'+5};
BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"hFUPBTbYehR2KgZj"};
//-----------------------------------------------------------------------------------
//2010.11.23 본섭 (이종원)
//BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1,'0'+2,'3'+3, '3'+4, '4'+5};
//BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"fdUiqE28nKN324vH"};
//-----------------------------------------------------------------------------------
//2010.09.30 본섭 (깁재우)
//BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1,'0'+2,'3'+3, '1'+4, '6'+5};
//BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"sDjh2908ds2jnvsL"};
//-----------------------------------------------------------------------------------
//BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1,'0'+2,'2'+3, '3'+4, '0'+5};
//BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"k5lEopal83udns8h"};
//-----------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
#elif SELECTED_LANGUAGE == LANGUAGE_ENGLISH		// 글로벌
//-----------------------------------> 사내 서버 버젼
//2010.01.20 본섭 (안상규)
BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '4'+3, '0'+4, '5'+5};
BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"TbYehR2hFUPBKgZj"};
//-----------------------------------------------------------------------------------
//2010.10.28 본섭 (김재우)
//BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '3'+3, '3'+4, '4'+5};
//BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"55Hhqervsybr23fM"};
//-----------------------------------------------------------------------------------
//2010.10.13 본섭 (김재우)
//BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '3'+3, '3'+4, '2'+5};
//BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"QDjhqwrgd5b7n6s5"};
//-----------------------------------------------------------------------------------
//2010.09.30 본섭 (김재우)
//BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '3'+3, '2'+4, '9'+5};
//BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"sDjhqweklr2jnvsL"};
//-----------------------------------------------------------------------------------
//2010.09.20 본섭 (이주흥)
//BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '3'+3, '2'+4, '8'+5};
//BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"sDjh2908ds2jnvsL"};
//-----------------------------------------------------------------------------------
//2010.06.23 본섭 (이주흥)
//BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '3'+3, '2'+4, '7'+5};
//BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"shF904usjfa9saCS"};
//-----------------------------------------------------------------------------------
//2010.06.10 본섭 (이주흥)
//BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '3'+3, '2'+4, '6'+5};
//BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"0skjSAD94sdfjklC"};
//-----------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
#elif SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE	// 베트남

//-----------------------------------> 사내 서버 버젼
//2009.06.05 본섭 (김재희)
//BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '3'+3, '0'+4, '2'+5};
//BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"uv5VsdivR29GVskQ"};
//-----------------------------------------------------------------------------------
//2010.10.15 본섭 (김재우)
BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '3'+3, '1'+4, '3'+5};
BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"zoWm4DFgtl3q3th9"};
//-----------------------------------------------------------------------------------
//2010.08.31 본섭 (박보근)
//BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '3'+3, '1'+4, '2'+5};
//BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"tlWms5ektl3qhstj"};
//-----------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
#endif //SELECTED_LANGUAGE


//////////////////////////////////////////////////////////////////////////
//  Global Variable.
//////////////////////////////////////////////////////////////////////////
CWsctlc     SocketClient;
CWsctlc*    g_pSocketClient = &SocketClient;
CSimpleModulus g_SimpleModulusCS;
CSimpleModulus g_SimpleModulusSC;

#ifdef PKD_ADD_ENHANCED_ENCRYPTION
CSessionCryptor g_SessionCryptorCS;
CSessionCryptor g_SessionCryptorSC;
#endif // PKD_ADD_ENHANCED_ENCRYPTION

BYTE    g_byPacketSerialSend = 0;
BYTE    g_byPacketSerialRecv = 0;

BOOL    g_bGameServerConnected = FALSE;
DWORD   g_dwLatestMagicTick = 0;

#ifdef USE_SELFCHECKCODE
BYTE g_byNextFuncCrcCheck = 1;
DWORD g_dwLatestFuncCrcCheck = 0;
#endif

#ifdef ACC_PACKETSIZE
int g_iTotalPacketRecv = 0;
int g_iTotalPacketSend = 0;
DWORD g_dwPacketInitialTick;
#endif //ACC_PACKETSIZE

PMSG_MATCH_RESULT	g_wtMatchResult;
PMSG_MATCH_TIMEVIEW	g_wtMatchTimeLeft;
int g_iGoalEffect = 0;

CROWN_SWITCH_INFO *Switch_Info = NULL;

int     HeroKey;
int     CurrentProtocolState;

int DirTable[16] = {-1,-1,  0,-1,  1,-1,  1,0,  1,1,  0,1,  -1,1,  -1,0};

#ifdef LDS_MODIFY_CHAR_LENGTH_USERPASSWORD // 비밀번호 자릿수 10->12로 변경 사항
char    Password[MAX_PASSWORD_SIZE+1];
#else // LDS_MODIFY_CHAR_LENGTH_USERPASSWORD 
char    Password[MAX_ID_SIZE+1];
#endif // LDS_MODIFY_CHAR_LENGTH_USERPASSWORD 

char    QuestionID[MAX_ID_SIZE+1];
char    Question[31];

//////////////////////////////////////////////////////////////////////////
//  Function
//////////////////////////////////////////////////////////////////////////

#define FIRST_CROWN_SWITCH_NUMBER	322


void AddDebugText(unsigned char *Buffer,int Size)
{
	if(DebugTextCount > MAX_DEBUG_MAX-1)
	{
		DebugTextCount = MAX_DEBUG_MAX-1;
		for(int i=1;i<MAX_DEBUG_MAX;i++)
		{
			memcpy(DebugText[i-1],DebugText[i],DebugTextLength[i]);
			DebugTextLength[i-1] = DebugTextLength[i];
		}
	}
	if(Size <= 256)
	{
		memcpy(DebugText[DebugTextCount],Buffer,Size);
		DebugTextLength[DebugTextCount] = Size;
		DebugTextCount++;
	}
}

BOOL CreateSocket(char *IpAddr, unsigned short Port)
{
	BOOL bResult = TRUE;
	
	static bool First;
	if(!First)
	{
		First = true;
		SocketClient.Startup();
#ifdef _DEBUG
		SocketClient.LogPrintOn();
#endif
	}
	g_ErrorReport.Write("[Connect to Server] ip address = %s, port = %d\r\n", IpAddr, Port);
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_NORMAL, "[Connect to Server] ip address = %s, port = %d", IpAddr, Port);
#endif // CONSOLE_DEBUG
	
	SocketClient.Create(g_hWnd, TRUE);
	if( SocketClient.Connect(IpAddr,Port,WM_ASYNCSELECTMSG) == FALSE )
	{
		g_ErrorReport.Write( "Failed to connect. ");
		g_ErrorReport.WriteCurrentTime();
		
		CUIMng::Instance().PopUpMsgWin(MESSAGE_SERVER_LOST);
		
		bResult = FALSE;
	}
	g_byPacketSerialSend = 0;
	g_byPacketSerialRecv = 0;
	
	return ( bResult);
}

void DeleteSocket()
{
	SocketClient.Close();
}

static BYTE bBuxCode[3] = {0xfc,0xcf,0xab};

void BuxConvert(BYTE *Buffer,int Size)
{
	for(int i=0;i<Size;i++)
		Buffer[i] ^= bBuxCode[i%3];
}


int  LogIn = 0;
char LogInID[MAX_ID_SIZE+1] = {0, };

bool First = false;
int FirstTime = 0;

bool LogOut = false;

int  ChatTime = 0;
char ChatText[256];

char ChatWhisperID[MAX_ID_SIZE+1];

int MoveCount = 0;

int CurrentSkill = 0;

bool Teleport = false;

int BuyCost = 0;

int  EnableUse = 0;

int SendGetItem = -1;
int SendDropItem = -1;

///////////////////////////////////////////////////////////////////////////////
// 길드
///////////////////////////////////////////////////////////////////////////////

int FindGuildName(char *Name)
{
	for(int i=0;i<MARK_EDIT;i++)
	{
		MARK_t *p = &GuildMark[i];
		if(strcmp(p->GuildName,Name)==NULL)
		{
			return i;
		}
	}
	return -1;
}

void GuildTeam( CHARACTER *c )
{
	OBJECT *o = &c->Object;
	if(o->Live)
	{
		c->GuildTeam = 0;
		
		// 같은길드일때 초록색으로 세팅
		if( Hero->GuildMarkIndex!=-1 && c->GuildMarkIndex==Hero->GuildMarkIndex )
			c->GuildTeam = 1;
		
		// 길드워중이라면..
		if( EnableGuildWar )
		{
			if( GuildWarIndex==-1 && GuildWarName[0] )
			{
				GuildWarIndex = FindGuildName( GuildWarName );
			}
			if( GuildWarIndex >= 0 && c->GuildMarkIndex == GuildWarIndex )
            {
				c->GuildTeam = 2;
            }
		}
	}
}

void InitGuildWar()
{
	EnableGuildWar = false;
	GuildWarIndex = -1;
	GuildWarName[0] = NULL;
	for(int i=0;i<MAX_CHARACTERS_CLIENT;i++)
	{
		CHARACTER *c = &CharactersClient[i];
		GuildTeam(c);
	}
}


BOOL Util_CheckOption( char *lpszCommandLine, unsigned char cOption, char *lpszString);


#ifndef KJH_ADD_SERVER_LIST_SYSTEM			// #ifndef
extern int  ServerLocalSelect;
#endif // KJH_ADD_SERVER_LIST_SYSTEM

void ReceiveServerList( BYTE *ReceiveBuffer )
{
	LPPHEADER_DEFAULT_SUBCODE_WORD Data = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer;
	int Offset = sizeof(PHEADER_DEFAULT_SUBCODE_WORD);
	
	BYTE Value2 = *(ReceiveBuffer+Offset++);

#ifdef KJH_ADD_SERVER_LIST_SYSTEM
	g_ServerListManager->Release();

	g_ServerListManager->SetTotalServer(MAKEWORD(Value2, Data->Value));		// 전체 서버 개수.(서버군 개수가 아님)
	
	for(int i=0 ; i<g_ServerListManager->GetTotalServer() ; i++)
	{
		LPPRECEIVE_SERVER_LIST Data2 = (LPPRECEIVE_SERVER_LIST)(ReceiveBuffer+Offset);
		
		g_ServerListManager->InsertServerGroup(Data2->Index, Data2->Percent);
#ifdef CONSOLE_DEBUG
		g_ConsoleDebug->Write(MCD_RECEIVE, "0xF4 [ReceiveServerList(%d %d %d)]", i, Data2->Index, Data2->Percent);
#endif // CONSOLE_DEBUG		
		Offset += sizeof(PRECEIVE_SERVER_LIST);
	}
#else // KJH_ADD_SERVER_LIST_SYSTEM

	ServerNumber = MAKEWORD(Value2, Data->Value);	// 서버 개수.(서버군 개수가 아님)
	
	for(int i=0;i<MAX_SERVER_HI;i++)
	{
        ServerList[i].Number = 0;
	}
	
#ifdef PJH_USER_VERSION_SERVER_LIST
//	ServerNumber = 2;
#endif //#ifdef PJH_USER_VERSION_SERVER_LIST//테스트 지워야함

#ifdef PJH_NEW_SERVER_ADD
//#ifdef PJH_FINAL_VIEW_SERVER_LIST2	
//		ServerNumber = 26;//박종훈
#endif //#ifdef PJH_NEW_SERVER_ADD	


//$$$$$$$$$$$$$$$$$$$$$$4
//	DebugAngel_Write( PACKET_SAVE_FILE, "서버갯수 : %d\r\n", ServerNumber);//박종훈
//$$$$$$$$$$$$$$$$$$$$$$4

	for(int i=0;i<ServerNumber;i++)
	{
		LPPRECEIVE_SERVER_LIST Data2 = (LPPRECEIVE_SERVER_LIST)(ReceiveBuffer+Offset);

		int Server = Data2->Index/20;	// 서버군 번호. 서버군은 20개의 서버를 가질 수 있음.

/*
		if(i%5 == 0)
			DebugAngel_Write( PACKET_SAVE_FILE, "          번호 : %d\r\n", Data2->Index);//박종훈
		else
			DebugAngel_Write( PACKET_SAVE_FILE, "          번호 : %d", Data2->Index);//박종훈
*/			
#ifdef PJH_NEW_SERVER_ADD
//		Server = i;//박종훈
#endif //#ifdef PJH_NEW_SERVER_ADD	
#ifdef PJH_USER_VERSION_SERVER_LIST
//		Server = 30 + i;
#endif //#ifdef PJH_USER_VERSION_SERVER_LIST//테스트 지워야함
#ifdef CONNECT_LIST_FIX
#ifndef PJH_FINAL_VIEW_SERVER_LIST
//#ifdef PJH_NEW_SERVER_ADD
//			if(Server == 13)
//#else
			if(Server == 12)
//#endif //PJH_NEW_SERVER_ADD
			{
#ifdef PJH_NEW_SERVER_ADD
				Server = 29;
#else
				Server = 24;
#endif //#ifdef PJH_NEW_SERVER_ADD
			strcpy(ServerList[Server].Name,GlobalText[559]);
			
			ServerList[Server].Server[ServerList[Server].Number].Index   = Data2->Index;
			ServerList[Server].Server[ServerList[Server].Number].Percent = Data2->Percent;
#ifdef _PVP_DYNAMIC_SERVER_TYPE
			ServerList[Server].Server[ServerList[Server].Number].IsPvPServer = Data2->IsPvPServer;
#endif	// _PVP_DYNAMIC_SERVER_TYPE
			ServerList[Server].Number++;
		}
		else
#endif //PJH_FINAL_VIEW_SERVER_LIST
		{
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//서버리스트는 서버로 부터 서버 번호를 받고 번호는 바로 서버의 고유넘버가 된다.
			//예를 들어 0 을 받으면 마야를 가르키게 돼고 1 을 받으면 위글을 받게됀다.
			//위의 iServerListMap 에표시를 하고싶다면 (23 - 놓고 싶은 위치의 값) 을 고유서버넘버 번호에 넣어주면됀다.
			//서버의 고유번호는 enum으로 0부터 시작한다.(@)는 현재는 안쓰이고 있는 서버이다.
			//마야	  = 0		
			//위글	  = 1		
			//바르	  = 2		
			//카라	  = 3	
			//라무	  = 4
			//나칼	  = 5
			//라사	  = 6
			//란스	  = 7
			//타르	  = 8
			//유즈(@) = 9
			//모아(@) = 10
			//루네    = 11
			//테스트  = 12
			//이온	  = 13
			//밀론	  = 14
			//뮤렌(@) = 15
			//루가(@) = 16
			//티탄(@) = 17
			//엘카(@) = 18
			//시렌(@) = 19
			//레알	  = 20
			//신규1   = 21
			//신규2   = 22
			//신규3(@)  = 23
			//Server_Position = 23 - 놓고 싶은 위치의값;
			//switch(Server_Position)
			//case 4 ~ 10 == 왼쪽위에서1번째 ~ 왼쪽위에서7번째.
			//case 14 ~ 21 == 오른쪽위에서1번째 ~ 오른쪽위에서8번째
			//text.txt 파일에 540번부터 559번까지가 서버 이름 이다. 
			//레알서버는 560번을 쓰고있기때문에 1829번에서 가져다 쓰고있다.
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if SELECTED_LANGUAGE == LANGUAGE_TAIWANESE
			int iServerListMap[] = { 1, 3, 4, 2, 0, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24};
#elif SELECTED_LANGUAGE == LANGUAGE_JAPANESE
			//이혁재 - 일본 요청으로 인한 서버 순서 변경 2005.2.4
			//int iServerListMap[] = { 1, 2, 3, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24};
			//			int iServerListMap[] = { 1, 3, 2, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24};
			// 안상규 - 신서버 추가 및 위치 변경. 2007.2.14
			int iServerListMap[] = { 2, 4, 3, 1, 0, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24};
#elif defined PJH_FINAL_VIEW_SERVER_LIST
			int iServerListMap[] = { 2, 3, 4, 5, 6, 7, 8, 9, 14, 0, 0, 15, 1, 16, 17, 0, 0, 0, 0, 0, 20, 18, 19, 0, 24};
#elif defined PJH_SEMI_FINAL_VIEW_SERVER_LIST	
			int iServerListMap[] = {  4 ,  5 ,  6 ,  7 ,  8 ,  9 ,  16 , 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 20, 18, 19, 0, 24};
#elif defined PJH_USER_VERSION_SERVER_LIST
			int iServerListMap[] = { 2, 3, 4, 5, 6, 7, 8, 9, 14, 0, 0, 15, 1, 16, 17, 0, 0, 0, 0, 0, 20, 18, 19, 0, 24, 30, 31};
#elif defined PJH_FINAL_VIEW_SERVER_LIST2
			int iServerListMap[] = {  4 ,  5 ,  6 ,  7 ,  8 ,  9 ,  15 , 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 20, 18, 19, 17, 16};
//			int iServerListMap[] = { 2, 3, 4, 5, 6, 7, 8, 9, 12, 0, 0, 13, 1, 14, 15, 0, 0, 0, 0, 0, 20, 18, 19, 0, 24, 17, 16};
#elif defined PJH_NEW_SERVER_ADD
			int iServerListMap[] = {  4 ,  5 ,  6 ,  7 ,  8 ,  9 ,  15 , 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 20, 18, 19, 17, 16,25};
#else
			int iServerListMap[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 14, 20, 15, 16, 17, 18, 11, 12, 13, 19, 21, 22, 23, 24};
#endif //SELECTED_LANGUAGE == LANGUAGE_JAPANESE
			
			int iServerPos = iServerListMap[Server];
#ifdef PJH_NEW_SERVER_ADD
			int iServerIndex = 28-iServerPos;
#elif SELECTED_LANGUAGE == LANGUAGE_ENGLISH
			int iServerIndex = iServerPos;
#else
			int iServerIndex = 23-iServerPos;
#endif //PJH_NEW_SERVER_ADD
			
#if defined PJH_FINAL_VIEW_SERVER_LIST || defined PJH_SEMI_FINAL_VIEW_SERVER_LIST || defined PJH_FINAL_VIEW_SERVER_LIST2 || defined PJH_NEW_SERVER_ADD
			if(iServerPos == 0)
			{
				Offset += sizeof(PRECEIVE_SERVER_LIST);
				continue;
			}
			
//#ifdef PJH_NEW_SERVER_ADD
//			if(Server == 13)
//#else
			if(Server == 12)
//#endif //PJH_NEW_SERVER_ADD
			{
#ifdef PJH_NEW_SERVER_ADD
				iServerIndex = 29;
#else
				iServerIndex = 24;
#endif //#ifdef PJH_NEW_SERVER_ADD
				strcpy(ServerList[iServerIndex].Name,GlobalText[559]);
			}
			else
				if(Server == 21)
				{
					strcpy ( ServerList[iServerIndex].Name, GlobalText[1873] );	    //  새로운 서버 생성. ( 로크 ).
				}
				else
					if(Server == 22)
					{
						strcpy ( ServerList[iServerIndex].Name, GlobalText[1874] );	    //  새로운 서버 생성. ( 레인 ).
					}
					else
						if ( Server == 20 )
							strcpy ( ServerList[iServerIndex].Name, GlobalText[1829] );	    //  새로운 서버 생성. ( 레알 ).
#else	//PJH_FINAL_VIEW_SERVER_LIST
						if ( Server==19 )
							strcpy ( ServerList[iServerIndex].Name, GlobalText[552] );	    //  새로운 서버 생성. ( 시렌 ).
						else if ( Server==20 )
							strcpy ( ServerList[iServerIndex].Name, GlobalText[1829] );	    //  새로운 서버 생성. ( 레알 ).
#endif	//PJH_FINAL_VIEW_SERVER_LIST
#if defined PJH_FINAL_VIEW_SERVER_LIST2 || defined PJH_NEW_SERVER_ADD
						else if ( Server==23 )
							strcpy ( ServerList[iServerIndex].Name, GlobalText[1272] );	    //  새로운 서버 생성. ( 실버 ).
						else if ( Server==24 )
							strcpy ( ServerList[iServerIndex].Name, GlobalText[1273] );	    //  새로운 서버 생성. ( 스톰 ).
#endif //PJH_FINAL_VIEW_SERVER_LIST2
#ifdef PJH_NEW_SERVER_ADD
						else if ( Server==25 )
							strcpy ( ServerList[iServerIndex].Name, GlobalText[728] );	    //  새로운 서버 생성. ( 로랜 ).
#endif //PJH_NEW_SERVER_ADD
#ifdef PJH_USER_VERSION_SERVER_LIST
						else
						if( Server == 30)
						{
							iServerIndex = 21;
							strcpy ( ServerList[iServerIndex].Name, GlobalText[1879] );	    //  시즌 4체험 서버
						}
						else
						if( Server == 31)
						{
							iServerIndex = 20;
							strcpy ( ServerList[iServerIndex].Name, GlobalText[1881] );	    //  사내테스트 서버
						}
#endif //PJH_USER_VERSION_SERVER_LIST
						else
							strcpy(ServerList[iServerIndex].Name,GlobalText[540+Server]);	// 서버 개수 늘어나면서 바뀜
						
						// ServerList[Server].Number는 서버 count.
						ServerList[iServerIndex].Server[ServerList[iServerIndex].Number].Index   = Data2->Index;
						ServerList[iServerIndex].Server[ServerList[iServerIndex].Number].Percent = Data2->Percent;
#ifdef _PVP_DYNAMIC_SERVER_TYPE
						ServerList[iServerIndex].Server[ServerList[iServerIndex].Number].IsPvPServer = Data2->IsPvPServer;
#endif	// _PVP_DYNAMIC_SERVER_TYPE
						ServerList[iServerIndex].Number++;
		}
#else	// CONNECT_LIST_FIX
		if ( Server==12 )
		{
			Server = 24;	// 서버 개수 늘어나면서 바뀜
			strcpy(ServerList[Server].Name,GlobalText[559]);
			
			ServerList[Server].Server[ServerList[Server].Number].Index   = Data2->Index;
			ServerList[Server].Server[ServerList[Server].Number].Percent = Data2->Percent;
#ifdef _PVP_DYNAMIC_SERVER_TYPE
			ServerList[Server].Server[ServerList[Server].Number].IsPvPServer = Data2->IsPvPServer;
#endif	// _PVP_DYNAMIC_SERVER_TYPE
			ServerList[Server].Number++;
		}
		/*
		else if(Server == 12)
		{
		Server = 24;	// 서버 개수 늘어나면서 바뀜
		strcpy(ServerList[Server].Name,GlobalText[559]);
		}
		*/
		else
		{
#if SELECTED_LANGUAGE == LANGUAGE_TAIWANESE
			int iServerListMap[] = { 1, 3, 4, 2, 0, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24};
			if ( Server < sizeof ( iServerListMap) / sizeof ( int))
			{
				Server = iServerListMap[Server];
			}
#endif //SELECTED_LANGUAGE == LANGUAGE_TAIWANESE
#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE
			int iServerListMap[] = { 2, 3, 0, 1, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24};
#endif //SELECTED_LANGUAGE == LANGUAGE_JAPANESE
			
            if ( Server==19 )
            {
				strcpy ( ServerList[23-Server].Name, GlobalText[552] );	    //  새로운 서버 생성. ( 시렌 ).
            }
            else
            {
                strcpy(ServerList[23-Server].Name,GlobalText[540+Server] );	//  서버 개수 늘어나면서 바뀜
            }
			Server = 23-Server;	// 서버 개수 늘어나면서 바뀜
		}
		ServerList[Server].Server[ServerList[Server].Number].Index   = Data2->Index;
		ServerList[Server].Server[ServerList[Server].Number].Percent = Data2->Percent;
#ifdef _PVP_DYNAMIC_SERVER_TYPE
		ServerList[Server].Server[ServerList[Server].Number].IsPvPServer = Data2->IsPvPServer;
#endif	// _PVP_DYNAMIC_SERVER_TYPE
        ServerList[Server].Number++;
#endif	// CONNECT_LIST_FIX
		
#ifdef CONSOLE_DEBUG
		g_ConsoleDebug->Write(MCD_RECEIVE, "0xF4 [ReceiveServerList(%d %d %d)]", i, Data2->Index, Data2->Percent);
#endif // CONSOLE_DEBUG
		
		Offset += sizeof(PRECEIVE_SERVER_LIST);
	}
/*
//$$$$$$$$$$$$$$$$$$$$$$4	
fclose(fp);
//$$$$$$$$$$$$$$$$$$$$$$4	
*/	
#ifdef PJH_DEBUG_SERVER_ADD
#ifdef PJH_VIEW_MAIN_SERVER_LIST
	//	int iServerListMap[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 14, 20, 15, 16, 17, 18, 11, 12, 13, 19, 21, 22, 23, 24};
	int iServerListMap[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24};
	
	for(i=0;i<MAX_SERVER_HI;i++)
	{
        ServerList[i].Number = 0;
	}
	
	for(i = 0; i < 21; i++)
	{
		int Server_Num = 23 - iServerListMap[i];
		
		
		if ( i==12 )
		{
			Server_Num = 24;	// 서버 개수 늘어나면서 바뀜
			strcpy(ServerList[Server_Num].Name,GlobalText[559]);
			
			ServerList[Server_Num].Server[ServerList[Server_Num].Number].Index   = i;
			ServerList[Server_Num].Server[ServerList[Server_Num].Number].Percent = 1;
			ServerList[Server_Num].Number++;
		}
		else
		{
			if ( i==19 )
			{
				strcpy ( ServerList[Server_Num].Name, GlobalText[552] );	    //  새로운 서버 생성. ( 시렌 ).
			}
			else if ( i==20 )
				strcpy ( ServerList[Server_Num].Name, GlobalText[1829] );	    //  새로운 서버 생성. ( 레알 ).
			else
			{
				strcpy(ServerList[Server_Num].Name,GlobalText[540+i] );	//  서버 개수 늘어나면서 바뀜
			}
			ServerList[Server_Num].Server[ServerList[Server_Num].Number].Index   = i;
			ServerList[Server_Num].Server[ServerList[Server_Num].Number].Percent = 1;
			ServerList[Server_Num].Number++;
		}
	}
#endif //PJH_VIEW_MAIN_SERVER_LIST
#endif //PJH_DEBUG_SERVER_ADD
#endif // KJH_ADD_SERVER_LIST_SYSTEM
	
	
	CUIMng& rUIMng = CUIMng::Instance();
	if (!rUIMng.m_CreditWin.IsShow())	// 크레딧이 안보일때만 다시 보여줌.
	{
		rUIMng.ShowWin(&rUIMng.m_ServerSelWin);
		rUIMng.m_ServerSelWin.UpdateDisplay();
		rUIMng.ShowWin(&rUIMng.m_LoginMainWin);
	}
	
	g_ErrorReport.Write ( "Success Receive Server List.\r\n");
	
#ifndef KJH_ADD_SERVER_LIST_SYSTEM			// #ifndef
#if defined _DEBUG || defined FOR_WORK
	char lpszTemp[256];
	if ( Util_CheckOption( GetCommandLine(), 'a', lpszTemp))
	{
		int iServer = atoi( lpszTemp);
		for ( int i = 0; i < ServerList[24].Number; ++i)
		{
			if ( ( ServerList[24].Server[i].Index % 20) + 1 == iServer)
			{
#ifdef _PVP_DYNAMIC_SERVER_TYPE
				g_bIsCurrentServerPvP = ServerList[24].Server[i].IsPvPServer;
#endif	// _PVP_DYNAMIC_SERVER_TYPE

#ifdef PKD_ADD_ENHANCED_ENCRYPTION
				SendRequestServerAddress2( ServerList[24].Server[i].Index);
#else
				SendRequestServerAddress( ServerList[24].Server[i].Index);
#endif // PKD_ADD_ENHANCED_ENCRYPTION
				ServerLocalSelect = ( ServerList[24].Server[i].Index) % 20 + 1;
			}
		}
	}
#endif
#endif // KJH_ADD_SERVER_LIST_SYSTEM
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0xF4 [ReceiveServerList]");
#endif // CONSOLE_DEBUG
}
void ReceiveServerConnect( BYTE *ReceiveBuffer )
{
	LPPRECEIVE_SERVER_ADDRESS Data = (LPPRECEIVE_SERVER_ADDRESS)ReceiveBuffer;
	char IP[16];
	memset(IP,0,16);
	memcpy(IP,(char*)Data->IP,15);
	g_ErrorReport.Write("[ReceiveServerConnect]");
	SocketClient.Close();
	// 종량업체 처리 여기서
	if ( CreateSocket(IP,Data->Port))	// 서버에서 응답 시 ReceiveJoinServer()로.
	{	// 게임서버에 연결중인지 알 수 있게 한다.
		g_bGameServerConnected = TRUE;
#ifdef USE_SELFCHECKCODE
		g_byNextFuncCrcCheck = 1;
#endif
	}
	
	char Text[100];
	sprintf(Text,GlobalText[481],IP,Data->Port);
	g_pChatListBox->AddText("", Text, SEASON3B::TYPE_SYSTEM_MESSAGE);	
}

void ReceiveServerConnectBusy( BYTE *ReceiveBuffer )
{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	LPPRECEIVE_SERVER_BUSY Data = ( LPPRECEIVE_SERVER_BUSY)ReceiveBuffer;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	
	// 사라졌던 서버선택 윈도우를 띄우고 메시지 윈도우를 최상단에 띄움.
	//	CUIMng::Instance().PopUpMsgWin(RECEIVE_JOIN_SERVER_WAITING);
	
	// 그냥 다시 서버리스트를 요구함.
#ifdef PKD_ADD_ENHANCED_ENCRYPTION
		SendRequestServerList2();
#else
		SendRequestServerList();
#endif // PKD_ADD_ENHANCED_ENCRYPTION
}


///////////////////////////////////////////////////////////////////////////////
// 접속, 계정
///////////////////////////////////////////////////////////////////////////////

void ReceiveJoinServer( BYTE *ReceiveBuffer )
{
	LPPRECEIVE_JOIN_SERVER Data2 = (LPPRECEIVE_JOIN_SERVER)ReceiveBuffer;
	
    if ( LogIn!=0 )
    {
        HeroKey = ((int)(Data2->NumberH)<<8) + Data2->NumberL;
        g_csMapServer.SendChangeMapServer ();
    }
    else
    {
		CUIMng& rUIMng = CUIMng::Instance();
		
        switch(Data2->Result)
        {
        case 0x01:	// 게임 서버 접속 성공.
			rUIMng.ShowWin(&rUIMng.m_LoginWin);		// 로그인 창을 보여줌.
            HeroKey = ((int)(Data2->NumberH)<<8) + Data2->NumberL;
            CurrentProtocolState = RECEIVE_JOIN_SERVER_SUCCESS;
            break;
			
        default:		// 게임 서버 접속 실패.
            g_ErrorReport.Write( "Connectting error. ");
            g_ErrorReport.WriteCurrentTime();
			rUIMng.PopUpMsgWin(MESSAGE_SERVER_LOST);
            break;
        }
        for(int i=0;i<SIZE_PROTOCOLVERSION;i++)
        {
            if(Version[i]-(i+1) != Data2->Version[i])
            {
				rUIMng.HideWin(&rUIMng.m_LoginWin);		// 로그인 창을 감춤.
				rUIMng.PopUpMsgWin(MESSAGE_VERSION);
                g_ErrorReport.Write( "Version dismatch - Join server.\r\n");
			}
		}
	}
	
	g_GuildCache.Reset();
	
#if defined _DEBUG || defined FOR_WORK
	if ( Data2->Result == 0x01)
	{
		char lpszTemp[256];
		if ( Util_CheckOption( GetCommandLine(), 'i', lpszTemp))
		{
			g_ErrorReport.Write( "> Try to Login \"%s\"\r\n", m_ID);
			SendRequestLogIn( m_ID, lpszTemp);			
		}
	}
#endif
}

void ReceiveConfirmPassword( BYTE *ReceiveBuffer )
{
	LPPRECEIVE_CONFIRM_PASSWORD Data = (LPPRECEIVE_CONFIRM_PASSWORD)ReceiveBuffer;
	switch(Data->Result)
	{
    case 0:
		CurrentProtocolState = RECEIVE_CONFIRM_PASSWORD_FAIL_ID;
		break;
    case 1:
		BuxConvert(Data->Question,sizeof(Data->Question));
		strcpy(Question,(char *)Data->Question);
		CurrentProtocolState = RECEIVE_CONFIRM_PASSWORD_SUCCESS;
		break;
	}
}

void ReceiveConfirmPassword2( BYTE *ReceiveBuffer )
{
	LPPRECEIVE_CONFIRM_PASSWORD2 Data = (LPPRECEIVE_CONFIRM_PASSWORD2)ReceiveBuffer;
	switch(Data->Result)
	{
    case 1:
		BuxConvert(Data->Password,sizeof(Data->Password));
		strcpy(Password,(char *)Data->Password);
		CurrentProtocolState = RECEIVE_CONFIRM_PASSWORD2_SUCCESS;
		break;
    case 0:
		CurrentProtocolState = RECEIVE_CONFIRM_PASSWORD2_FAIL_ID;
		break;
    case 2:
		CurrentProtocolState = RECEIVE_CONFIRM_PASSWORD2_FAIL_ANSWER;
		break;
    case 3:
		CurrentProtocolState = RECEIVE_CONFIRM_PASSWORD2_FAIL_RESIDENT;
		break;
	}
}

void ReceiveChangePassword( BYTE *ReceiveBuffer )
{
	LPPHEADER_DEFAULT_SUBCODE Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
	switch(Data->Value)
	{
    case 1:
		CurrentProtocolState = RECEIVE_CHANGE_PASSWORD_SUCCESS;
		break;
    case 0:
		CurrentProtocolState = RECEIVE_CHANGE_PASSWORD_FAIL_ID;
		break;
    case 2:
		CurrentProtocolState = RECEIVE_CHANGE_PASSWORD_FAIL_RESIDENT;
		break;
    case 3:
		CurrentProtocolState = RECEIVE_CHANGE_PASSWORD_FAIL_PASSWORD;
		break;
	}
}
// 캐릭터 씬에서 선택 가능한 캐릭터들 정보를 받는다.(우히히)
void ReceiveCharacterList( BYTE *ReceiveBuffer )
{
	InitGuildWar();
	
	LPPHEADER_DEFAULT_CHARACTER_LIST Data = (LPPHEADER_DEFAULT_CHARACTER_LIST)ReceiveBuffer;
    
    SetCreateMaxClass ( Data->MaxClass );
#ifdef PBG_FIX_CHAOSMAPMOVE
	// ReceiveTeleport를 받지 않는 맵이동은 맵인덱스 업뎃이 안됨
	if(SceneFlag == CHARACTER_SCENE)
	{
		World = WD_74NEW_CHARACTER_SCENE;
	}
#endif //PBG_FIX_CHAOSMAPMOVE
    int Offset = sizeof(PHEADER_DEFAULT_CHARACTER_LIST);
	
	for(int i=0;i<Data->Value;i++)
	{
		LPPRECEIVE_CHARACTER_LIST Data2 = (LPPRECEIVE_CHARACTER_LIST)(ReceiveBuffer+Offset);
		
		int iClass = ChangeServerClassTypeToClientClassType(Data2->Class);
		
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		float fPos[2], fAngle = 0.0f;
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		float fPos[2], fAngle;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		
		switch(Data2->Index)
		{
#ifdef PJH_NEW_SERVER_SELECT_MAP
 		case 0:	fPos[0] = 8008.0f;	fPos[1] = 18885.0f;	fAngle = 115.0f; break;
 		case 1:	fPos[0] = 7986.0f;	fPos[1] = 19145.0f;	fAngle = 90.0f; break;
 		case 2:	fPos[0] = 8046.0f;	fPos[1] = 19400.0f;	fAngle = 75.0f; break;
 		case 3:	fPos[0] = 8133.0f;	fPos[1] = 19645.0f;	fAngle = 60.0f; break;
 		case 4:	fPos[0] = 8282.0f;	fPos[1] = 19845.0f;	fAngle = 35.0f; break;
#else //PJH_NEW_SERVER_SELECT_MAP
		case 0:	fPos[0] = 22628.0f;	fPos[1] = 15012.0f;	fAngle = 100.0f; break;
		case 1:	fPos[0] = 22700.0f;	fPos[1] = 15201.0f;	fAngle = 75.0f; break;
		case 2:	fPos[0] = 22840.0f;	fPos[1] = 15355.0f;	fAngle = 50.0f; break;
		case 3:	fPos[0] = 23019.0f;	fPos[1] = 15443.0f;	fAngle = 25.0f; break;
		case 4:	fPos[0] = 23211.6f;	fPos[1] = 15467.0f;	fAngle = 0.0f; break;
#endif //PJH_NEW_SERVER_SELECT_MAP
		}
		
		CHARACTER *c = CreateHero(Data2->Index,iClass,0,fPos[0],fPos[1],fAngle);
		
		c->Level = Data2->Level;
		c->CtlCode = Data2->CtlCode;
		
		memcpy(c->ID,(char *)Data2->ID,MAX_ID_SIZE);
		c->ID[MAX_ID_SIZE] = NULL;
		
		ChangeCharacterExt(Data2->Index,Data2->Equipment);
		
#ifdef _PVP_BLOCK_PVP_CHAR
		c->PK = Data2->PvPState;
#endif	// _PVP_BLOCK_PVP_CHAR
		c->GuildStatus = Data2->byGuildStatus;
		Offset += sizeof(PRECEIVE_CHARACTER_LIST);
	}
#ifdef PBG_ADD_CHARACTERSLOT
	// 블루뮤 서버일 경우만 CharacterSlotCount값을 사용한다
	if(BLUE_MU::IsBlueMuServer())
	{
		//캐릭터 자리는 생성하지 않는다
		g_SlotLimit->Init();
		g_SlotLimit->SetCharacterCnt(Data->CharacterSlotCount);
		g_SlotLimit->SetSlotPosition();
	}
#endif //PBG_ADD_CHARACTERSLOT
	CurrentProtocolState = RECEIVE_CHARACTERS_LIST;
}
#ifdef PBG_ADD_CHARACTERCARD
CHARACTER_ENABLE g_CharCardEnable;
// 국내 전용 캐릭터 카드(소환술사01/다크02/마검04)
void ReceiveCharacterCard_New(BYTE* ReceiveBuffer)
{
	LPPHEADER_CHARACTERCARD Data = (LPPHEADER_CHARACTERCARD)ReceiveBuffer;
	g_CharCardEnable.bCharacterEnable[0] = false;	//마검사
	g_CharCardEnable.bCharacterEnable[1] = false;	//다크로드
#ifdef PBG_MOD_BLUE_SUMMONER_ENABLE
	// 소환술사 캐릭터 카드를 블루에서는 사용하지 않는것으로 기획변경
	g_CharCardEnable.bCharacterEnable[2] = true;	//소환술사
#else //PBG_MOD_BLUE_SUMMONER_ENABLE
	g_CharCardEnable.bCharacterEnable[2] = false;	//소환술사
#endif //PBG_MOD_BLUE_SUMMONER_ENABLE

	// 마검사 카드가 활성화 되어있다면
	if((Data->CharacterCard & CLASS_DARK_CARD) == CLASS_DARK_CARD)
		g_CharCardEnable.bCharacterEnable[0] = true;
	// 다크로드 카드가 활성화되어있다면
	if((Data->CharacterCard & CLASS_DARK_LORD_CARD) == CLASS_DARK_LORD_CARD)
		g_CharCardEnable.bCharacterEnable[1] = true;
	// 소환술사 카드가 활성화 되어있다면
	if((Data->CharacterCard & CLASS_SUMMONER_CARD) == CLASS_SUMMONER_CARD)
		g_CharCardEnable.bCharacterEnable[2] = true;
}
#endif //PBG_ADD_CHARACTERCARD
// 캐릭터씬에서 캐릭터 생성 요청에 대한 응답.
void ReceiveCreateCharacter( BYTE *ReceiveBuffer )
{
	LPPRECEIVE_CREATE_CHARACTER Data = (LPPRECEIVE_CREATE_CHARACTER)ReceiveBuffer;
	if(Data->Result==1)
	{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		float fPos[2], fAngle = 0.0f;
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		float fPos[2], fAngle;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		
		switch(Data->Index)
		{
#ifdef PJH_NEW_SERVER_SELECT_MAP
	 	case 0:	fPos[0] = 8008.0f;	fPos[1] = 18885.0f;	fAngle = 115.0f; break;
 		case 1:	fPos[0] = 7986.0f;	fPos[1] = 19145.0f;	fAngle = 90.0f; break;
 		case 2:	fPos[0] = 8046.0f;	fPos[1] = 19400.0f;	fAngle = 75.0f; break;
 		case 3:	fPos[0] = 8133.0f;	fPos[1] = 19645.0f;	fAngle = 60.0f; break;
 		case 4:	fPos[0] = 8282.0f;	fPos[1] = 19845.0f;	fAngle = 35.0f; break;
#else //PJH_NEW_SERVER_SELECT_MAP
		case 0:	fPos[0] = 22628.0f;	fPos[1] = 15012.0f;	fAngle = 100.0f; break;
		case 1:	fPos[0] = 22700.0f;	fPos[1] = 15201.0f;	fAngle = 75.0f; break;
		case 2:	fPos[0] = 22840.0f;	fPos[1] = 15355.0f;	fAngle = 50.0f; break;
		case 3:	fPos[0] = 23019.0f;	fPos[1] = 15443.0f;	fAngle = 25.0f; break;
		case 4:	fPos[0] = 23211.6f;	fPos[1] = 15467.0f;	fAngle = 0.0f; break;
#endif //PJH_NEW_SERVER_SELECT_MAP
		}
		
#ifdef LDS_FIX_RESET_CHARACTERDELETED
		INT		iCharacterKey;
		iCharacterKey = Data->Index;
		DeleteCharacter( iCharacterKey );
#endif
		
		CreateHero(Data->Index,CharacterView.Class,CharacterView.Skin,fPos[0],fPos[1],fAngle);
		CharactersClient[Data->Index].Level = Data->Level;
		
		int iClass = ChangeServerClassTypeToClientClassType(Data->Class);
		
		CharactersClient[Data->Index].Class = iClass;
		memcpy(CharactersClient[Data->Index].ID,Data->ID,MAX_ID_SIZE);
		CharactersClient[Data->Index].ID[MAX_ID_SIZE] = NULL;
		CurrentProtocolState = RECEIVE_CREATE_CHARACTER_SUCCESS;
		CUIMng& rUIMng = CUIMng::Instance();
		rUIMng.CloseMsgWin();
		rUIMng.m_CharSelMainWin.UpdateDisplay();
		rUIMng.m_CharInfoBalloonMng.UpdateDisplay();
	}
	else if(Data->Result==0)	
		CUIMng::Instance().PopUpMsgWin(RECEIVE_CREATE_CHARACTER_FAIL);
	else if(Data->Result==2)
		CUIMng::Instance().PopUpMsgWin(RECEIVE_CREATE_CHARACTER_FAIL2);
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x01 [ReceiveCreateCharacter]");
#endif // CONSOLE_DEBUG
}

// 캐릭터씬에서 캐릭터 삭제 요청에 대한 응답.
void ReceiveDeleteCharacter( BYTE *ReceiveBuffer )
{
	LPPHEADER_DEFAULT_SUBCODE Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
	switch (Data->Value)
	{
	case 1:	// 삭제 성공.
#ifdef LDS_FIX_RESET_CHARACTERDELETED		// 선택 케릭터 삭제시에 실제 클라이언트 상 해당 케릭터 내용 초기화를 해줍니다.
		INT		iKey;
		iKey = CharactersClient[SelectedHero].Key;
		DeleteCharacter( iKey );
#endif
		CUIMng::Instance().PopUpMsgWin(MESSAGE_DELETE_CHARACTER_SUCCESS);
		break;
	case 0:	// 길드 삭제 불가에 의해
		CUIMng::Instance().PopUpMsgWin(MESSAGE_DELETE_CHARACTER_GUILDWARNING);
		break;
	case 3:	// 아이템 블럭된 캐릭터
		CUIMng::Instance().PopUpMsgWin(MESSAGE_DELETE_CHARACTER_ITEM_BLOCK);
		break;
	case 2:	// 주민번호 틀림
	default:
		CUIMng::Instance().PopUpMsgWin(MESSAGE_STORAGE_RESIDENTWRONG);
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
// 로그아웃
///////////////////////////////////////////////////////////////////////////////

int SummonLife = 0;

extern void StopMusic();

void InitGame()
{
	
	EnableUse = 0;
	SendGetItem = -1;
	SummonLife = 0;
	SoccerTime = 0;
	SoccerObserver = false;

	g_wtMatchResult.Clear();
	g_wtMatchTimeLeft.m_Time = 0;
	g_iGoalEffect = 0;
	
    g_shCameraLevel = 0;
	
	SelectedNpc = -1;
	SelectedOperate = -1;
	SelectedCharacter = -1;
	SelectedItem = -1;
	
	Attacking = -1;
	g_pOption->SetAutoAttack(true);
	g_pOption->SetWhisperSound(false);
	
	CheckInventory = NULL;
	
	//World = -1;
	
	SendExitInventory ();		// 왜 이걸 서버로 보내는지 확인할 것.
	
	g_iFollowCharacter = -1;
	
	LockInputStatus = false;
	
	//  수리.
	RepairEnable = 0;
	CheckSkill = -1;
	
	//  공지 초기화.
	ClearNotice ();
	
	//  술 효과 초기화.

	CharacterAttribute->Ability = 0;
	CharacterAttribute->AbilityTime[0] = 0;
	CharacterAttribute->AbilityTime[1] = 0;
	CharacterAttribute->AbilityTime[2] = 0;
	
#ifdef KJH_FIX_WOPS_K27340_INIT_STORAGE_GOLD
	// 창고/보유 금액 초기화
	CharacterMachine->StorageGold	= 0;
	CharacterMachine->Gold			= 0;
#endif // KJH_FIX_WOPS_K27340_INIT_STORAGE_GOLD

	
	g_shEventChipCount = 0;          //  제나개수.
	g_shMutoNumber[0]  = -1;
	g_shMutoNumber[1]  = -1;
	g_shMutoNumber[2]  = -1;
	
	ClearWhisperID ();
	
    matchEvent::ClearMatchInfo();
	
	//  파티 정보 초기화.
	InitPartyList();
	
	g_csQuest.clearQuest ();

#ifdef YDG_ADD_NEW_DUEL_SYSTEM
	g_DuelMgr.Reset();
#else	// YDG_ADD_NEW_DUEL_SYSTEM
	ClearDuelWindow();
#endif	// YDG_ADD_NEW_DUEL_SYSTEM
	g_pNewUISystem->Hide( SEASON3B::INTERFACE_DUEL_WINDOW );
	
	// UI 매니저 초기화
	if( g_pUIManager )
		g_pUIManager->Init();
	
	if( g_pSiegeWarfare )
		g_pSiegeWarfare->InitMiniMapUI();
	
	g_Direction.Init();
	g_Direction.DeleteMonster();
	
	RemoveAllPerosnalItemPrice(PSHOPWNDTYPE_SALE);
	RemoveAllPerosnalItemPrice(PSHOPWNDTYPE_PURCHASE);
	
	g_pNewUIHotKey->SetStateGameOver(false);	
	
	// 개인상점 제목 초기화
	g_pMyShopInventory->ResetSubject();
	g_pChatListBox->ResetFilter();

#ifdef CSK_FIX_WOPS_K30866_GUILDNOTICE
	g_pGuildInfoWindow->NoticeClear();
#endif // CSK_FIX_WOPS_K30866_GUILDNOTICE
}

BOOL ReceiveLogOut(BYTE *ReceiveBuffer, BOOL bEncrypted)
{
	LogOut = false;
	LPPHEADER_DEFAULT_SUBCODE Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
	switch(Data->Value)
	{
	case 0:
		SendMessage(g_hWnd, WM_DESTROY, 0, 0);
		break;
	case 1://케릭창
		if ( !bEncrypted)
		{
			GO_DEBUG;
			SendHackingChecked( 0x00, 0xF1);
			return ( FALSE);
		}
		StopMusic();
        AllStopSound();
		
		SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
		
		ReleaseMainData();
		CryWolfMVPInit();
		
		SceneFlag = CHARACTER_SCENE;
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		SendRequestCharactersList(g_pMultiLanguage->GetLanguage());
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		SendRequestCharactersList();
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
								
		InitCharacterScene = false;
        InitMainScene = false;
        EnableMainRender = false;
		CurrentProtocolState = REQUEST_JOIN_SERVER;
		InitGame();
		break;
	case 2://서버 선택창
		if(SceneFlag == MAIN_SCENE)
		{
			CryWolfMVPInit();
			StopMusic();
            AllStopSound();
			SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
			ReleaseMainData();
		}
		
		if ( !bEncrypted)
		{
			GO_DEBUG;
			SendHackingChecked( 0x00, 0xF1);
			return ( FALSE);
		}
		g_ErrorReport.Write("[ReceiveLogOut]");
		SocketClient.Close();
		ReleaseCharacterSceneData();
		SceneFlag = LOG_IN_SCENE;
								
		InitLogIn = false;
		InitCharacterScene = false;
        InitMainScene = false;
        EnableMainRender = false;
		CurrentProtocolState = REQUEST_JOIN_SERVER;
		
        LogIn = 0;
        g_csMapServer.Init ();
		InitGame();
		break;
	}
	
#ifdef PSW_BUGFIX_FRIEND_LIST_CLEAR
	g_pWindowMgr->Reset();
	g_pFriendList->ClearFriendList();
	g_pLetterList->ClearLetterList();
#endif //PSW_BUGFIX_FRIEND_LIST_CLEAR
	
#ifdef NEW_USER_INTERFACE_SHELL
	TheShopServerProxy().ShopReset( true );
#endif //NEW_USER_INTERFACE_SHELL
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x02 [ReceiveServerList(%d)]", Data->Value);
#endif // CONSOLE_DEBUG
	
	return ( TRUE);
}

///////////////////////////////////////////////////////////////////////////////
// 시작, 다시 살아날때
///////////////////////////////////////////////////////////////////////////////

int HeroIndex;

BOOL ReceiveJoinMapServer(BYTE *ReceiveBuffer, BOOL bEncrypted)
{
	if ( !bEncrypted)
	{
		GO_DEBUG;
		SendHackingChecked( 0x00, 0xF3);
		return ( FALSE);
	}
	MouseLButton = false;
	
#ifdef PJH_DEBUG
	char Text[300];
	wsprintf(Text,"맵이동 (join)[time : %d]", GetTickCount());
	g_pChatListBox->AddText("DEBUG",Text, SEASON3B::TYPE_GM_MESSAGE);
#endif

	LPPRECEIVE_JOIN_MAP_SERVER Data = (LPPRECEIVE_JOIN_MAP_SERVER)ReceiveBuffer;
	

	__int64 Data_Exp = 0x0000000000000000;
	Master_Level_Data.lMasterLevel_Experince = 0x0000000000000000;
	Data_Exp |= Data->btMExp1;	
	Data_Exp <<= 8;
	Data_Exp |= Data->btMExp2;	
	Data_Exp <<= 8;
	Data_Exp |= Data->btMExp3;	
	Data_Exp <<= 8;
	Data_Exp |= Data->btMExp4;	
	Data_Exp <<= 8;
	Data_Exp |= Data->btMExp5;	
	Data_Exp <<= 8;
	Data_Exp |= Data->btMExp6;	
	Data_Exp <<= 8;
	Data_Exp |= Data->btMExp7;	
	Data_Exp <<= 8;
	Data_Exp |= Data->btMExp8;
	
	if(IsMasterLevel(CharacterAttribute->Class) == true)
	{
		Master_Level_Data.lMasterLevel_Experince = Data_Exp;
	}
	else
	{
		CharacterAttribute->Experience = (int)Data_Exp;
	}
	
	// 다음 레벨 경험치
	Data_Exp = 0x0000000000000000;
	Master_Level_Data.lNext_MasterLevel_Experince = 0x0000000000000000;
	
	Data_Exp |= Data->btMNextExp1;	
	Data_Exp <<= 8;
	Data_Exp |= Data->btMNextExp2;	
	Data_Exp <<= 8;
	Data_Exp |= Data->btMNextExp3;	
	Data_Exp <<= 8;
	Data_Exp |= Data->btMNextExp4;	
	Data_Exp <<= 8;
	Data_Exp |= Data->btMNextExp5;	
	Data_Exp <<= 8;
	Data_Exp |= Data->btMNextExp6;	
	Data_Exp <<= 8;
	Data_Exp |= Data->btMNextExp7;	
	Data_Exp <<= 8;
	Data_Exp |= Data->btMNextExp8;
	
	if(IsMasterLevel(CharacterAttribute->Class) == true)
	{
		Master_Level_Data.lNext_MasterLevel_Experince = Data_Exp;
	}
	else
	{
		CharacterAttribute->NextExperince = (int)Data_Exp;
	}
	
	//	Master_Level_Data.wMaxLife			= Data->LifeMax;
	//	Master_Level_Data.wMaxMana			= Data->ManaMax;
	//	Master_Level_Data.wMaxShield		= Data->ShieldMax;

	CharacterAttribute->LevelUpPoint  = Data->LevelUpPoint;
	CharacterAttribute->Strength      = Data->Strength;
	CharacterAttribute->Dexterity     = Data->Dexterity;
	CharacterAttribute->Vitality      = Data->Vitality;
	CharacterAttribute->Energy        = Data->Energy;
    CharacterAttribute->Charisma      = Data->Charisma;
	CharacterAttribute->Life          = Data->Life;
	CharacterAttribute->LifeMax       = Data->LifeMax;
	CharacterAttribute->Mana          = Data->Mana;
	CharacterAttribute->ManaMax       = Data->ManaMax;
	CharacterAttribute->SkillMana     = Data->SkillMana;
	CharacterAttribute->SkillManaMax  = Data->SkillManaMax;
	CharacterAttribute->Ability		  = 0;
	CharacterAttribute->AbilityTime[0]= 0;
	CharacterAttribute->AbilityTime[1]= 0;
	CharacterAttribute->AbilityTime[2]= 0;
	CharacterAttribute->Shield		  = Data->Shield;
	CharacterAttribute->ShieldMax	  = Data->ShieldMax;
	
    //  추가되는 스텟 정보.
    CharacterAttribute->AddPoint		= Data->AddPoint;
    CharacterAttribute->MaxAddPoint		= Data->MaxAddPoint;
	// 마이너스 스탯 정보
	CharacterAttribute->wMinusPoint     = Data->wMinusPoint;
    CharacterAttribute->wMaxMinusPoint  = Data->wMaxMinusPoint;
	
	CharacterMachine->Gold            = Data->Gold;
	//CharacterAttribute->SkillMana     = CharacterAttribute->Energy*10+CharacterAttribute->ManaMax*10/6;

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	int iPreWorld = World;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	World = Data->Map;
	OpenWorld(World);

#ifdef LDS_FIX_DISABLE_INPUTJUNKKEY_WHEN_LORENMARKT_EX01
	g_bReponsedMoveMapFromServer = TRUE;		// 맵이동 응답 월드로딩OK
	LoadingWorld = 30;
#endif // LDS_FIX_DISABLE_INPUTJUNKKEY_WHEN_LORENMARKT_EX01
	
	if(World == WD_34CRYWOLF_1ST)
	{
		SendRequestCrywolfInfo();
	}
	
    matchEvent::CreateEventMatch ( World );
	
	HeroIndex = rand()%MAX_CHARACTERS_CLIENT;
	CHARACTER *c = &CharactersClient[HeroIndex];
	CreateCharacterPointer(c,MODEL_PLAYER,Data->PositionX,Data->PositionY,((float)Data->Angle-1.f)*45.f);
	c->Key = HeroKey;
	
    OBJECT *o = &c->Object;
	c->Class = CharacterAttribute->Class;
	c->Skin = 0;
	c->PK    = Data->PK;
	c->CtlCode	= Data->CtlCode;
	o->Kind  = KIND_PLAYER;
   	SetCharacterClass(c);
#ifdef PK_ATTACK_TESTSERVER_LOG
	PrintPKLog(c);
#endif // PK_ATTACK_TESTSERVER_LOG

#ifdef LEM_FIX_AUTOSKILLBAR_SAVE
	if( !g_bExit )	c->CurrentSkill = Hero->CurrentSkill;
	else	g_bExit = false;
#endif	//  [lem_2010.8.18]
	Hero = c;

#ifdef PBG_ADD_PKSYSTEM_INGAMESHOP
	g_PKSystem->SetHeroPKState(Hero->PK);
#endif //PBG_ADD_PKSYSTEM_INGAMESHOP
#ifdef LJH_ADD_MORE_ZEN_FOR_ONE_HAVING_A_PARTY_WITH_MURDERER
	g_PKSystem->SetHeroPartyPKState(Hero->PKPartyLevel);
#endif //LJH_ADD_MORE_ZEN_FOR_ONE_HAVING_A_PARTY_WITH_MURDERER

	memcpy(c->ID,(char *)CharacterAttribute->Name,MAX_ID_SIZE);
    //  초기화.
    for ( int i=0; i<MAX_EQUIPMENT; ++i )
    {
        CharacterMachine->Equipment[i].Type = -1;
        CharacterMachine->Equipment[i].Level = 0;
        CharacterMachine->Equipment[i].Option1 = 0;
    }
	c->ID[MAX_ID_SIZE] = NULL;
	CreateEffect(BITMAP_MAGIC+2,o->Position,o->Angle,o->Light,0,o);
	CurrentProtocolState = RECEIVE_JOIN_MAP_SERVER;
	
    LockInputStatus = false;
    CheckIME_Status(true,0);
	
#ifdef LDS_FIX_DISABLE_INPUTJUNKKEY_WHEN_LORENMARKT_EX01
	g_bReponsedMoveMapFromServer = TRUE;		// 맵이동 응답 월드로딩OK
#endif // LDS_FIX_DISABLE_INPUTJUNKKEY_WHEN_LORENMARKT_EX01
    LoadingWorld = 30;
    MouseUpdateTime = 0;
    MouseUpdateTimeMax = 6;
#ifdef PET_SYSTEM
    CreatePetDarkSpirit_Now ( Hero );
#endif// PET_SYSTEM
    CreateEffect(BITMAP_MAGIC+2,o->Position,o->Angle,o->Light,0,o);
    o->Alpha = 0.f;
	
	g_pNewUISystem->HideAll();
    
    //  초기화.
    SelectedItem      = -1;
    SelectedNpc       = -1;
    SelectedCharacter = -1;
    SelectedOperate   = -1;
    Attacking         = -1;
    
    //  수리.
    RepairEnable = 0;
   
    Hero->Movement = false;
    SetPlayerStop(Hero);
	
    //  블러드 캐슬의 음악을 제거한다.
    if ( InBloodCastle() == false )
    {
        StopBuffer ( SOUND_BLOODCASTLE, true );
    }
	
    if ( InChaosCastle() == false )
    {
        StopBuffer ( SOUND_CHAOSCASTLE, true );
        StopBuffer ( SOUND_CHAOS_ENVIR, true );
    }
	
#ifdef LDS_ADD_EMPIRE_GUARDIAN
	if( IsEmpireGuardian1() == false &&
		IsEmpireGuardian2() == false &&
		IsEmpireGuardian3() == false &&
		IsEmpireGuardian4() == false )
	{
		StopBuffer(SOUND_EMPIREGUARDIAN_WEATHER_RAIN, true);
		StopBuffer(SOUND_EMPIREGUARDIAN_WEATHER_FOG, true);
		StopBuffer(SOUND_EMPIREGUARDIAN_WEATHER_STORM, true);
		StopBuffer(SOUND_EMPIREGUARDIAN_INDOOR_SOUND, true);
	}
#endif //LDS_ADD_EMPIRE_GUARDIAN

#if defined RESOURCE_GUARD && !defined FOR_WORK

#ifdef KJH_MOD_RESOURCE_GUARD
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
		SocketClient.Close();
		g_ErrorReport.Write("> ResourceGuard Error!!\r\n");
		return FALSE;
	}
#else // KJH_MOD_RESOURCE_GUARD
	char szModuleFileName[256];
	GetModuleFileName(NULL, szModuleFileName, 256);
	ResourceGuard::CResourceGuard RG101;
	CMuRGReport MuRGReport;
	RG101.AddReportObj(&MuRGReport);
	if(!RG101.CheckIntegrityPEFile(szModuleFileName))
	//if(!RG101.CheckIntegrity(szModuleFileName))			// CheckIntegrityPEFile()로 변경
	{
		SocketClient.Close();
		g_ErrorReport.Write("> ResourceGuard Error!!(%s)\r\n", szModuleFileName);
		return FALSE;
	}
#endif // KJH_MOD_RESOURCE_GUARD
#endif // RESOURCE_GUARD && FOR_WORK

#ifdef ASG_ADD_NEW_QUEST_SYSTEM
#ifndef ASG_FIX_QUEST_PROTOCOL_ADD
	g_QuestMng.SetQuestIndexByEtcList(NULL, 0);	// 기타 상황에 의한 퀘스트 리스트 초기화.
	SendRequestProgressQuestList();	// 진행중인 퀘스트 리스트 요청.
#endif	// ASG_FIX_QUEST_PROTOCOL_ADD
#endif	// ASG_ADD_NEW_QUEST_SYSTEM

	g_pUIMapName->ShowMapName();

#ifdef ASG_ADD_INFLUENCE_GROUND_EFFECT
	CreateMyGensInfluenceGroundEffect();
#endif	// ASG_ADD_INFLUENCE_GROUND_EFFECT
		
#ifdef YDG_ADD_DOPPELGANGER_EVENT
		if (World >= WD_65DOPPLEGANGER1 && World <= WD_68DOPPLEGANGER4);
	else
#endif	// YDG_ADD_DOPPELGANGER_EVENT
	{
		//텍스트 표시부분 추가
		char Text[256];
		// 484 "에 오신것을 환영합니다."
		
#ifndef GRAMMAR_VERB
		sprintf(Text,"%s%s",GetMapName( World),GlobalText[484]);
#else //GRAMMAR_VERB
		sprintf(Text,"%s%s",GlobalText[484],GetMapName( World));
#endif //GRAMMAR_VERB
		
		g_pChatListBox->AddText("", Text, SEASON3B::TYPE_SYSTEM_MESSAGE);
	}
	
	if( World == WD_30BATTLECASTLE )
	{
		if( g_pSiegeWarfare )
			g_pSiegeWarfare->CreateMiniMapUI();
	}
	
#ifdef YDG_ADD_DOPPELGANGER_EVENT
	if( World < WD_65DOPPLEGANGER1 || World > WD_68DOPPLEGANGER4 )
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_DOPPELGANGER_FRAME);
	}
#endif	// YDG_ADD_DOPPELGANGER_EVENT

#ifdef LDK_ADD_EMPIREGUARDIAN_PROTOCOLS
	if( World < WD_69EMPIREGUARDIAN1 || WD_72EMPIREGUARDIAN4 < World)
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_EMPIREGUARDIAN_TIMER);
	}
#endif //LDK_ADD_EMPIREGUARDIAN_PROTOCOLS
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x03 [ReceiveJoinMapServer]");
#endif // CONSOLE_DEBUG
	
#ifdef PJH_DEBUG
	wsprintf(Text,"맵로딩완료(join)[time : %d]", GetTickCount());
	g_pChatListBox->AddText("DEBUG",Text, SEASON3B::TYPE_GM_MESSAGE);
#endif
	return ( TRUE);
}

void ReceiveRevival( BYTE *ReceiveBuffer )
{
	MouseLButton = false;
	Teleport = false;
	Hero->Object.Live = false;
	LPPRECEIVE_REVIVAL Data = (LPPRECEIVE_REVIVAL)ReceiveBuffer;
	
	CharacterAttribute->Life       = Data->Life;
	CharacterAttribute->Mana       = Data->Mana;
	CharacterAttribute->Shield	   = Data->Shield;
	CharacterAttribute->SkillMana  = Data->SkillMana;

	__int64 Data_Exp = 0x0000000000000000;
	Master_Level_Data.lMasterLevel_Experince = 0x0000000000000000;
	Data_Exp |= Data->btMExp1;	
	Data_Exp <<= 8;
	Data_Exp |= Data->btMExp2;	
	Data_Exp <<= 8;
	Data_Exp |= Data->btMExp3;	
	Data_Exp <<= 8;
	Data_Exp |= Data->btMExp4;	
	Data_Exp <<= 8;
	Data_Exp |= Data->btMExp5;	
	Data_Exp <<= 8;
	Data_Exp |= Data->btMExp6;	
	Data_Exp <<= 8;
	Data_Exp |= Data->btMExp7;	
	Data_Exp <<= 8;
	Data_Exp |= Data->btMExp8;
	
	
	if(IsMasterLevel(Hero->Class) == true)
	{
		Master_Level_Data.lMasterLevel_Experince = Data_Exp;
	}
	else
	{
		CharacterAttribute->Experience = (int)Data_Exp;
	}
	
	CharacterMachine->Gold         = Data->Gold;
	for(int i=0;i<MAX_CHARACTERS_CLIENT;i++)
	{
		CHARACTER *c = &CharactersClient[i];
		c->Object.Live = false;
	}
	
	BYTE Temp = Hero->PK;
	int TempGuild = Hero->GuildMarkIndex;
	
	CHARACTER *c = &CharactersClient[HeroIndex];
	// 길드관계는 백업
	BYTE BackUpGuildStatus = c->GuildStatus;
	BYTE BackUpGuildType = c->GuildType;
	BYTE BackUpGuildRelationShip = c->GuildRelationShip;
	BYTE byBackupEtcPart = c->EtcPart;
	
	CreateCharacterPointer(c,MODEL_PLAYER,Data->PositionX,Data->PositionY,((float)Data->Angle-1.f)*45.f);
	c->Key = HeroKey;
	c->GuildStatus = BackUpGuildStatus;
	c->GuildType = BackUpGuildType;
	c->GuildRelationShip = BackUpGuildRelationShip;
	c->EtcPart = byBackupEtcPart;
	
	OBJECT *o = &c->Object;
	c->Class    = CharacterAttribute->Class;
	c->Skin = 0;
	c->PK       = Temp;
	o->Kind     = KIND_PLAYER;
	c->GuildMarkIndex = TempGuild;
	c->SafeZone = true;
	SetCharacterClass(c);
	
#ifdef PK_ATTACK_TESTSERVER_LOG
	PrintPKLog(c);
#endif // PK_ATTACK_TESTSERVER_LOG
	
	SetPlayerStop(c);
	CreateEffect(BITMAP_MAGIC+2,o->Position,o->Angle,o->Light,0,o);
    ClearItems();
	ClearCharacters(HeroKey);
	RemoveAllShopTitleExceptHero();
	
	if( World >= WD_45CURSEDTEMPLE_LV1 && World <= WD_45CURSEDTEMPLE_LV6 )
	{
		if( !(Data->Map >= WD_45CURSEDTEMPLE_LV1 && Data->Map <= WD_45CURSEDTEMPLE_LV6) )
		{
			g_CursedTemple->ResetCursedTemple();
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_CURSEDTEMPLE_GAMESYSTEM);
		}
	}
	
    if(World != Data->Map)
	{
        int OldWorld = World;
		
		World = Data->Map;
		OpenWorld(World);

#ifdef LDS_FIX_DISABLE_INPUTJUNKKEY_WHEN_LORENMARKT_EX01
		g_bReponsedMoveMapFromServer = TRUE;		// 부활 한경우 월드로딩OK
		LoadingWorld = 30;
#endif // LDS_FIX_DISABLE_INPUTJUNKKEY_WHEN_LORENMARKT_EX01
		
        if ( ( InChaosCastle( OldWorld ) == true && OldWorld!=World ) 
			|| InChaosCastle() == true )
        {
            SetCharacterClass ( Hero );
        }
		
        if ( InChaosCastle() == false )
        {
            StopBuffer ( SOUND_CHAOSCASTLE, true );
            StopBuffer ( SOUND_CHAOS_ENVIR, true );
		}

#ifdef LDS_ADD_EMPIRE_GUARDIAN
		if( IsEmpireGuardian1() == false &&
			IsEmpireGuardian2() == false &&
			IsEmpireGuardian3() == false &&
			IsEmpireGuardian4() == false )
		{
			StopBuffer(SOUND_EMPIREGUARDIAN_WEATHER_RAIN, true);
			StopBuffer(SOUND_EMPIREGUARDIAN_WEATHER_FOG, true);
			StopBuffer(SOUND_EMPIREGUARDIAN_WEATHER_STORM, true);
			StopBuffer(SOUND_EMPIREGUARDIAN_INDOOR_SOUND, true);
		}
#endif //LDS_ADD_EMPIRE_GUARDIAN
		
        if ( World==-1 || c->Helper.Type!=MODEL_HELPER+3 || c->SafeZone )
        {
            o->Position[2] = RequestTerrainHeight(o->Position[0],o->Position[1]);
        }
        else
        {
            if ( World==WD_8TARKAN || World==WD_10HEAVEN )
                o->Position[2] = RequestTerrainHeight(o->Position[0],o->Position[1])+90.f;
            else
                o->Position[2] = RequestTerrainHeight(o->Position[0],o->Position[1])+30.f;
        }
	}
#ifdef PET_SYSTEM
    CreatePetDarkSpirit_Now ( c );
#endif// PET_SYSTEM
    SummonLife = 0;
	GuildTeam(c);
	
	g_pUIMapName->ShowMapName();	// rozy

#ifdef ASG_ADD_INFLUENCE_GROUND_EFFECT
	CreateMyGensInfluenceGroundEffect();
#endif	// ASG_ADD_INFLUENCE_GROUND_EFFECT
	
#ifdef YDG_ADD_DOPPELGANGER_EVENT
	if( World < WD_65DOPPLEGANGER1 || World > WD_68DOPPLEGANGER4 )
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_DOPPELGANGER_FRAME);
	}
#endif	// YDG_ADD_DOPPELGANGER_EVENT
#ifdef LDK_ADD_EMPIREGUARDIAN_PROTOCOLS
	if( World < WD_69EMPIREGUARDIAN1 || WD_72EMPIREGUARDIAN4 < World)
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_EMPIREGUARDIAN_TIMER);
	}
#endif //LDK_ADD_EMPIREGUARDIAN_PROTOCOLS

	g_pNewUISystem->HideAll();
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x04 [ReceiveRevival]");
#endif // CONSOLE_DEBUG
	
}

void ReceiveMagicList( BYTE *ReceiveBuffer )
{
#ifdef PJH_FIX_4_BUGFIX_33
	int Master_Skill_Bool = -1;
	int Skill_Bool = -1;
#endif //PJH_FIX_4_BUGFIX_33
	LPPHEADER_MAGIC_LIST_COUNT Data = (LPPHEADER_MAGIC_LIST_COUNT)ReceiveBuffer;
	int Offset = sizeof(PHEADER_MAGIC_LIST_COUNT);
	if(Data->Value == 0xFF)
	{	
		LPPRECEIVE_MAGIC_LIST Data2 = (LPPRECEIVE_MAGIC_LIST)(ReceiveBuffer+Offset);
		CharacterAttribute->Skill[Data2->Index] = 0;
	}
	else if(Data->Value == 0xFE) 
	{
		LPPRECEIVE_MAGIC_LIST Data2 = (LPPRECEIVE_MAGIC_LIST)(ReceiveBuffer+Offset);
		CharacterAttribute->Skill[Data2->Index] = Data2->Type;
#ifdef USE_SKILL_LEVEL
		CharacterAttribute->SkillLevel[Data2->Index] = Data2->Level;
#endif
	}
    else if ( Data->ListType==0x02 )
    {
        for ( int i=0; i<Data->Value; ++i )
        {
			LPPRECEIVE_MAGIC_LIST Data2 = (LPPRECEIVE_MAGIC_LIST)(ReceiveBuffer+Offset);
			CharacterAttribute->Skill[Data2->Index] = 0;
#ifdef USE_SKILL_LEVEL
			CharacterAttribute->SkillLevel[Data2->Index] = 0;
#endif
        }
    }
	else
	{
        if ( Data->ListType == 0x00 )
        {
			ZeroMemory( CharacterAttribute->Skill,  MAX_SKILLS * sizeof ( WORD));
        }
		for(int i=0; i<Data->Value; i++)
		{
			LPPRECEIVE_MAGIC_LIST Data2 = (LPPRECEIVE_MAGIC_LIST)(ReceiveBuffer+Offset);
			CharacterAttribute->Skill[Data2->Index] = Data2->Type;
#ifdef USE_SKILL_LEVEL
			CharacterAttribute->SkillLevel[Data2->Index] = Data2->Level;
#endif
			Offset += sizeof(PRECEIVE_MAGIC_LIST);
		}
        if ( GetBaseClass( Hero->Class )==CLASS_DARK_LORD )
        {
            for ( int i=0; i<PET_CMD_END; ++i )
            {
                CharacterAttribute->Skill[AT_PET_COMMAND_DEFAULT+i] = AT_PET_COMMAND_DEFAULT+i;
#ifdef USE_SKILL_LEVEL
                CharacterAttribute->SkillLevel[AT_PET_COMMAND_DEFAULT+i] = Data2->Level;
#endif
            }
        }
	}
	
	CharacterAttribute->SkillNumber = 0;
	CharacterAttribute->SkillMasterNumber = 0;
	
    int SkillType = 0;
	for(int i=0;i<MAX_SKILLS;i++)
	{
		SkillType = CharacterAttribute->Skill[i];
		if ( SkillType!=0 )
		{
			CharacterAttribute->SkillNumber++;
            BYTE SkillUseType = SkillAttribute[SkillType].SkillUseType;
            if ( SkillUseType==SKILL_USE_TYPE_MASTER )
            {
				CharacterAttribute->SkillMasterNumber++;
            }
		}
	}
	if(Hero->CurrentSkill >= CharacterAttribute->SkillNumber)
		Hero->CurrentSkill = 0;
    if(CharacterAttribute->SkillNumber == 1)
		Hero->CurrentSkill = 0;
    if(Hero->CurrentSkill>=0 && CharacterAttribute->Skill[Hero->CurrentSkill]==0)
		Hero->CurrentSkill = 0;
	int Skill = 0;

	for(int i = 0; i < MAX_SKILLS; i++)
	{
		Skill = CharacterAttribute->Skill[Hero->CurrentSkill];
		if ( Skill>= AT_SKILL_STUN && Skill<=AT_SKILL_REMOVAL_BUFF )
			Hero->CurrentSkill++;
		else
			break;
	}
#ifdef PJH_FIX_4_BUGFIX_33	
	for(int i = 0; i < MAX_SKILLS; i++)
	{
		Skill = CharacterAttribute->Skill[i];
		if((AT_SKILL_POWER_SLASH_UP <= Skill && AT_SKILL_POWER_SLASH_UP+4 >= Skill) || (AT_SKILL_MANY_ARROW_UP <= Skill && AT_SKILL_MANY_ARROW_UP+4 >= Skill))
		{
			Master_Skill_Bool = i;
		}
		if (AT_SKILL_ICE_BLADE == Skill || Skill == AT_SKILL_CROSSBOW)
		{
			Skill_Bool  = i;
		}
	}
	if(Master_Skill_Bool > -1 && Skill_Bool > -1)
		CharacterAttribute->Skill[Skill_Bool] = 0;
#endif //#ifdef PJH_FIX_4_BUGFIX_33
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x11 [ReceiveMagicList]");
#endif // CONSOLE_DEBUG
}

//-----------------------------------------------------------------------------------------------
// ReceiveInventory
BOOL ReceiveInventory(BYTE *ReceiveBuffer, BOOL bEncrypted)
{
	for(int i=0;i<MAX_EQUIPMENT;i++)
	{
		CharacterMachine->Equipment[i].Type = -1;
		CharacterMachine->Equipment[i].Number = 0;
        CharacterMachine->Equipment[i].Option1 = 0; //  추가 옵션. ( 액설런트 )
	}

	g_pMyInventory->UnequipAllItems();
	g_pMyInventory->DeleteAllItems();
	g_pMyShopInventory->DeleteAllItems();

	if ( !bEncrypted)
	{
		GO_DEBUG;
		SendHackingChecked( 0x00, 0xF3);
		return ( FALSE);
	}
	
	LPPHEADER_DEFAULT_SUBCODE_WORD Data = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer; //LPPHEADER_DEFAULT_SUBCODE_WORD 6byte
	int Offset = sizeof(PHEADER_DEFAULT_SUBCODE_WORD);
	DeleteBug(&Hero->Object);
#ifdef PET_SYSTEM
    giPetManager::DeletePet ( Hero );            
#endif// PET_SYSTEM
#ifdef LDK_ADD_NEW_PETPROCESS
	ThePetProcess().DeletePet( Hero );
#endif //LDK_ADD_NEW_PETPROCESS
	for(int i=0;i<Data->Value;i++)
	{
		LPPRECEIVE_INVENTORY Data2 = (LPPRECEIVE_INVENTORY)(ReceiveBuffer+Offset); //LPPRECEIVE_INVENTORY 8byte
		
		SEASON3B::CNewUIInventoryCtrl::DeletePickedItem();
		int itemindex = Data2->Index;
		if(itemindex >= 0 && itemindex < MAX_EQUIPMENT_INDEX)
		{
			g_pMyInventory->EquipItem(itemindex, Data2->Item);
		}
		else if(itemindex >= MAX_EQUIPMENT_INDEX && itemindex < MAX_MY_INVENTORY_INDEX)
		{
			itemindex = itemindex - MAX_EQUIPMENT_INDEX;
			g_pMyInventory->InsertItem(itemindex, Data2->Item);
		}
		else if(itemindex >= (MAX_EQUIPMENT_INDEX + MAX_INVENTORY) && itemindex < MAX_MY_SHOP_INVENTORY_INDEX)
		{
			itemindex = itemindex - (MAX_EQUIPMENT_INDEX + MAX_INVENTORY);
			g_pMyShopInventory->InsertItem(itemindex, Data2->Item);
		}

		Offset += sizeof(PRECEIVE_INVENTORY);
	}
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x10 [ReceiveInventory]");
#endif // CONSOLE_DEBUG
	
	return ( TRUE);
}

void ReceiveDeleteInventory( BYTE *ReceiveBuffer )
{
	LPPHEADER_DEFAULT_SUBCODE Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
	if(Data->SubCode != 0xff)
	{
		int itemindex = Data->SubCode;
		if(itemindex >= 0 && itemindex < MAX_EQUIPMENT_INDEX)
		{
#ifdef LJH_FIX_BUG_NOT_REMOVED_PET_WITH_1_DUR_WHEN_PLAYER_KILLED
			SEASON3B::CNewUIPickedItem* pPickedItem = SEASON3B::CNewUIInventoryCtrl::GetPickedItem();
			ITEM* pEquippedItem = &CharacterMachine->Equipment[itemindex];

			if (pPickedItem && pPickedItem->GetItem()->lineal_pos == pEquippedItem->lineal_pos)
				SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
#endif //LJH_FIX_BUG_NOT_REMOVED_PET_WITH_1_DUR_WHEN_PLAYER_KILLED
			

			g_pMyInventory->UnequipItem(itemindex);
		}
		else if(itemindex >= MAX_EQUIPMENT_INDEX && itemindex < MAX_MY_INVENTORY_INDEX)
		{
			itemindex = itemindex - MAX_EQUIPMENT_INDEX;
			g_pMyInventory->DeleteItem(itemindex);
		}
		else if(itemindex >= (MAX_EQUIPMENT_INDEX + MAX_INVENTORY) && itemindex < MAX_MY_SHOP_INVENTORY_INDEX)
		{
			itemindex = itemindex - (MAX_EQUIPMENT_INDEX + MAX_INVENTORY);
			g_pMyShopInventory->DeleteItem(itemindex);
		}
	}
	
	if(Data->Value)
	{
		EnableUse = 0;
	}
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x28 [ReceiveDeleteInventory(%d %d)]", Data->SubCode, Data->Value);
#endif // CONSOLE_DEBUG
}

void ReceiveTradeInventory( BYTE *ReceiveBuffer )
{
	LPPHEADER_DEFAULT_SUBCODE_WORD Data = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer;
	int Offset = sizeof(PHEADER_DEFAULT_SUBCODE_WORD);

	if(Data->SubCode == 3)//믹스창 조합 실패
	{
		g_pMixInventory->SetMixState(SEASON3B::CNewUIMixInventory::MIX_FINISHED);
		PlayBuffer(SOUND_MIX01);
		PlayBuffer(SOUND_BREAK01);
		g_pMixInventory->DeleteAllItems();
	}
    else if ( Data->SubCode==5 )    //  조련사 조합 실패
    {
		g_pChatListBox->AddText( "", GlobalText[1208], SEASON3B::TYPE_ERROR_MESSAGE);
		PlayBuffer ( SOUND_MIX01 );
		PlayBuffer ( SOUND_BREAK01 );
		g_pMixInventory->SetMixState(SEASON3B::CNewUIMixInventory::MIX_FINISHED);
		g_pMixInventory->DeleteAllItems();
    }
	else
	{
		for(int i=0;i<MAX_SHOP_INVENTORY;i++)
		{
			ShopInventory[i].Type = -1;
			ShopInventory[i].Number = 0;
		}
	}

	for(int i=0;i<Data->Value;i++)
	{
		LPPRECEIVE_INVENTORY Data2 = (LPPRECEIVE_INVENTORY)(ReceiveBuffer+Offset);
		
		if(Data->SubCode == 3)
		{
			g_pMixInventory->InsertItem(Data2->Index, Data2->Item);
		}
        else if ( Data->SubCode==5 )
        {
			g_pMixInventory->InsertItem(Data2->Index, Data2->Item);
        }
		else
		{
			if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP) == true)
			{
				g_pNPCShop->InsertItem(Data2->Index, Data2->Item);
			}
			else if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE))
			{
				g_pStorageInventory->InsertItem(Data2->Index, Data2->Item);
			}
		}
		
		Offset += sizeof(PRECEIVE_INVENTORY);
	}
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x31 [ReceiveTradeInventory]");
#endif // CONSOLE_DEBUG
}

void ReceiveChat( BYTE *ReceiveBuffer )
{
	if(SceneFlag == LOG_IN_SCENE)
	{
		g_ErrorReport.Write ( "Send Request Server List.\r\n");
#ifdef PKD_ADD_ENHANCED_ENCRYPTION
		SendRequestServerList2();
#else
		SendRequestServerList();
#endif // PKD_ADD_ENHANCED_ENCRYPTION
	}
	else
	{
		LPPCHATING Data = (LPPCHATING)ReceiveBuffer;
		
		char ID[MAX_ID_SIZE+1];
		memset(ID, 0, MAX_ID_SIZE+1);
		memcpy(ID,(char *)Data->ID,MAX_ID_SIZE);
		
		char Text[MAX_CHAT_SIZE+1];
		memset(Text, 0, MAX_CHAT_SIZE+1);
		
		if(Data->ChatText[0]=='~'
#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE
			|| Data->ChatText[0]==']'
#endif	// SELECTED_LANGUAGE == LANGUAGE_JAPANESE
			)
		{
			for(int i=0;i<MAX_CHAT_SIZE-1;i++)
				Text[i] = Data->ChatText[i+1];
			g_pChatListBox->AddText(ID, Text, SEASON3B::TYPE_PARTY_MESSAGE);
		}
		else if(Data->ChatText[0]=='@' && Data->ChatText[1]=='@')
		{
			for(int i=0;i<MAX_CHAT_SIZE-2;i++)
				Text[i] = Data->ChatText[i+2];
			g_pChatListBox->AddText(ID, Text, SEASON3B::TYPE_UNION_MESSAGE);
		}
		else if(Data->ChatText[0]=='@')
		{
			for(int i=0;i<MAX_CHAT_SIZE-1;i++)
				Text[i] = Data->ChatText[i+1];
			g_pChatListBox->AddText(ID, Text, SEASON3B::TYPE_GUILD_MESSAGE);	
		}
		else if(Data->ChatText[0]=='#')
		{
			for(int i=0;i<MAX_CHAT_SIZE-1;i++)
				Text[i] = Data->ChatText[i+1];
		
			CHARACTER* pFindGm = NULL;

			for(int i=0;i<MAX_CHARACTERS_CLIENT;i++)
			{
				CHARACTER *c = &CharactersClient[i];
				OBJECT *o = &c->Object;
				if( o->Live && o->Kind==KIND_PLAYER && ( g_isCharacterBuff( (&c->Object), eBuff_GMEffect)
					|| (c->CtlCode == CTLCODE_20OPERATOR) || (c->CtlCode == CTLCODE_08OPERATOR) ) )
				{
					if(strcmp(c->ID,ID) == NULL)
					{
						pFindGm = c;
						break;
					}	
				}
			}
			if(pFindGm)
			{
				AssignChat(ID,Text);
				g_pChatListBox->AddText(ID,Text, SEASON3B::TYPE_GM_MESSAGE);
			}
			else
			{
				AssignChat(ID,Text,1);
			}
		}
        else
		{
			memcpy(Text,(char *)Data->ChatText,MAX_CHAT_SIZE);
			CHARACTER* pFindGm = NULL;
			for(int i=0;i<MAX_CHARACTERS_CLIENT;i++)
			{
				CHARACTER *c = &CharactersClient[i];
				OBJECT *o = &c->Object;
				if( o->Live && o->Kind==KIND_PLAYER && g_isCharacterBuff((&c->Object), eBuff_GMEffect)
					|| (c->CtlCode == CTLCODE_20OPERATOR) || (c->CtlCode == CTLCODE_08OPERATOR) )
				{
					if(strcmp(c->ID,ID) == NULL)
					{
						pFindGm = c;
						break;
					}	
				}
			}
			if(pFindGm)
			{
				AssignChat(ID,Text);
				g_pChatListBox->AddText(ID, Text, SEASON3B::TYPE_GM_MESSAGE);
			}
			else
			{
				AssignChat(ID,Text);
				g_pChatListBox->AddText(ID, Text, SEASON3B::TYPE_CHAT_MESSAGE);	
			}
		}
	}
}

void ReceiveChatWhisper( BYTE *ReceiveBuffer )
{
	if(g_pChatInputBox->IsBlockWhisper() == true)
	{
		return;
	}
    
	LPPCHATING Data = (LPPCHATING)ReceiveBuffer;
	
	char ID[MAX_ID_SIZE+1];
	memset(ID, 0, MAX_ID_SIZE+1);
	memcpy(ID,(char *)Data->ID,MAX_ID_SIZE);
	
	char Text[MAX_CHAT_SIZE+1];
	memset(Text, 0, MAX_CHAT_SIZE+1);
	memcpy(Text,(char *)Data->ChatText,MAX_CHAT_SIZE);
	
    RegistWhisperID ( 10, ID );
	
	if(g_pOption->IsWhisperSound())
	{
		PlayBuffer(SOUND_WHISPER);
	}
	
	g_pChatListBox->AddText(ID,Text, SEASON3B::TYPE_WHISPER_MESSAGE);
}

void ReceiveChatWhisperResult( BYTE *ReceiveBuffer )
{
	LPPHEADER_DEFAULT Data = (LPPHEADER_DEFAULT)ReceiveBuffer;
	switch(Data->Value)
	{
	case 0:
        { 
#ifdef KJH_FIX_UI_CHAT_MESSAGE
			g_pChatListBox->AddText(ChatWhisperID,GlobalText[482], SEASON3B::TYPE_ERROR_MESSAGE, SEASON3B::TYPE_WHISPER_MESSAGE);
#else //KJH_FIX_UI_CHAT_MESSAGE	
			g_pChatListBox->AddText(ChatWhisperID,GlobalText[482], SEASON3B::TYPE_ERROR_MESSAGE);	 
#endif // KJH_FIX_UI_CHAT_MESSAGE
        }
	}
}

void ReceiveChatKey( BYTE *ReceiveBuffer )
{
	LPPCHATING_KEY Data = (LPPCHATING_KEY)ReceiveBuffer;
	int Key = ((int)(Data->KeyH)<<8) + Data->KeyL;
	int Index = FindCharacterIndex(Key);
	
	if( Hero->GuildStatus == G_MASTER && !strcmp( CharactersClient[Index].ID, "길드 마스터" ) )
	{
		g_pNewUISystem->Show(SEASON3B::INTERFACE_NPCGUILDMASTER);
		
		GuildInputEnable = true;
		InputEnable = false;
		ClearInput(FALSE);
		InputTextMax[0] = 8;
		InputNumber = 0;
		
		if( Hero->GuildMarkIndex == -1 )
			Hero->GuildMarkIndex = MARK_EDIT;
		return;
	}
	
	CreateChat(CharactersClient[Index].ID,(char *)Data->ChatText,&CharactersClient[Index]);
}

void ReceiveNotice( BYTE *ReceiveBuffer )
{
	LPPRECEIVE_NOTICE Data = (LPPRECEIVE_NOTICE)ReceiveBuffer;
	if(Data->Result==0)
	{
		CreateNotice((char *)Data->Notice,0);
	}
	else if(Data->Result==1)
	{
		if(CHARACTER_SCENE != SceneFlag)
		{
			g_pChatListBox->AddText("", (char*)Data->Notice, SEASON3B::TYPE_SYSTEM_MESSAGE);
#ifdef CSK_FREE_TICKET
			EnableUse = 0;
#endif // CSK_FREE_TICKET
		}
		else
		{
			CUIMng& rUIMng = CUIMng::Instance();
			rUIMng.AddServerMsg((char*)Data->Notice);
		}
	}
	else if(Data->Result==2)
	{
		char Text[100];
		sprintf(Text,GlobalText[483],(char *)Data->Notice);
		CreateNotice(Text,1);
		g_pGuildInfoWindow->AddGuildNotice((char*)Data->Notice);
	}
	else if (Data->Result >= 10 && Data->Result <= 15)
	{
		if (Data->Notice != NULL && Data->Notice[0] != '\0')
		{
			g_pSlideHelpMgr->AddSlide(Data->Count, Data->Delay, (char *)Data->Notice, Data->Result - 10, Data->Speed / 10.0f, Data->Color);
		}
	}
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x0D [ReceiveNotice(%s)]", Data->Notice);
#endif // CONSOLE_DEBUG
}

void ReceiveMoveCharacter(BYTE *ReceiveBuffer,int Size)
{
	LPPMOVE_CHARACTER Data = (LPPMOVE_CHARACTER)ReceiveBuffer;
	int  Key = ((int)(Data->KeyH)<<8) + Data->KeyL;
    CHARACTER *c = &CharactersClient[FindCharacterIndex(Key)];

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	OBJECT *o = &c->Object;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	if(c->Dead==0)
	{
		OBJECT *o = &c->Object;
		c->TargetAngle = Data->Path[0]>>4;
		if(Key == HeroKey)
		{
			if(!c->Movement)
			{
				c->PositionX = Data->PositionX;
				c->PositionY = Data->PositionY;
			}
		}
		else
		{
			c->TargetX = Data->PositionX;
			c->TargetY = Data->PositionY;
			if(o->Type == MODEL_MONSTER01+52)
			{
				c->PositionX = Data->PositionX;
				c->PositionY = Data->PositionY;
				o->Angle[2] = CreateAngle(o->Position[0],o->Position[1],c->PositionX*100.f,c->PositionY*100.f);
				SetPlayerWalk( c);
				c->JumpTime = 1;
			}
			else if(c->Appear == 0)
			{
#ifdef YDG_ADD_DOPPELGANGER_MONSTER
				int iDefaultWall = TW_CHARACTER;

				if (World >= WD_65DOPPLEGANGER1 && World <= WD_68DOPPLEGANGER4
					&& Key != HeroKey)
				{
					iDefaultWall = TW_NOMOVE;
				}

				if(PathFinding2(c->PositionX, c->PositionY, c->TargetX, c->TargetY, &c->Path, 0.0f, iDefaultWall))
#else	// YDG_ADD_DOPPELGANGER_MONSTER
				if(PathFinding2(c->PositionX, c->PositionY, c->TargetX, c->TargetY, &c->Path))
#endif	// YDG_ADD_DOPPELGANGER_MONSTER
				{
					c->Movement = true;
				}
				else
				{
					c->Movement = false;
					SetPlayerStop(c);
				}
			}

#ifdef CONSOLE_DEBUG
			g_ConsoleDebug->Write(MCD_RECEIVE, "ID : %s | sX : %d | sY : %d | tX : %d | tY : %d", c->ID, c->PositionX, c->PositionY, c->TargetX, c->TargetY);
#endif // CONSOLE_DEBUG
		}
	}
}

void ReceiveMovePosition( BYTE *ReceiveBuffer )
{
    LPPRECEIVE_MOVE_POSITION Data = (LPPRECEIVE_MOVE_POSITION)ReceiveBuffer;
	int Key = ((int)(Data->KeyH)<<8) + Data->KeyL;
	CHARACTER *c = &CharactersClient[FindCharacterIndex(Key)];
	
	OBJECT *o = &c->Object;
	if(o->Type == MODEL_BALL)
	{
		o->Gravity = 20.f;
		o->Direction[1] = (Data->PositionX-( c->PositionX))*400.f;
		o->Direction[0] = -(Data->PositionY-( c->PositionY))*400.f;
		PlayBuffer(SOUND_MENU01);
		CreateBomb(o->Position,false);
	}
	
	c->PositionX = Data->PositionX;
	c->PositionY = Data->PositionY;
	c->TargetX = Data->PositionX;
	c->TargetY = Data->PositionY;
	c->JumpTime = 1;
}

#ifdef USE_EVENT_ELDORADO
extern int EnableEvent;
#endif

BOOL ReceiveTeleport(BYTE *ReceiveBuffer, BOOL bEncrypted)
{
	if ( !bEncrypted)
	{
		GO_DEBUG;
		SendHackingChecked( 0x00, 0x1C);
		return ( FALSE);
	}
	
#ifdef PJH_DEBUG
	char Text[300];
	wsprintf(Text,"맵이동 (tel)[time : %d]", GetTickCount());
	g_pChatListBox->AddText("DEBUG",Text, SEASON3B::TYPE_GM_MESSAGE);
#endif

	SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
	
	LPPRECEIVE_TELEPORT_POSITION Data = (LPPRECEIVE_TELEPORT_POSITION)ReceiveBuffer;
	Hero->PositionX = Data->PositionX;
	Hero->PositionY = Data->PositionY;
	
	Hero->JumpTime = 0;
	
	OBJECT *o = &Hero->Object;
	o->Position[0] = ((float)(Hero->PositionX)+0.5f)*TERRAIN_SCALE;
	o->Position[1] = ((float)(Hero->PositionY)+0.5f)*TERRAIN_SCALE;
	
    if ( World==-1 || Hero->Helper.Type!=MODEL_HELPER+3 || Hero->SafeZone )
    {
        o->Position[2] = RequestTerrainHeight(o->Position[0],o->Position[1]);
    }
    else
    {
        if ( World==WD_8TARKAN || World==WD_10HEAVEN )
            o->Position[2] = RequestTerrainHeight(o->Position[0],o->Position[1])+90.f;
        else
            o->Position[2] = RequestTerrainHeight(o->Position[0],o->Position[1])+30.f;
    }
	
	if( World >= WD_45CURSEDTEMPLE_LV1 && World <= WD_45CURSEDTEMPLE_LV6 )
	{
		if( !(Data->Map >= WD_45CURSEDTEMPLE_LV1 && Data->Map <= WD_45CURSEDTEMPLE_LV6) )
		{
			g_CursedTemple->ResetCursedTemple();
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_CURSEDTEMPLE_GAMESYSTEM);
		}
	}
	
	int iEtcPart = Hero->EtcPart;
	
	o->Angle[2]    = ((float)(Data->Angle)-1.f)*45.f;
	if(Data->Flag == 0)
	{
        CreateTeleportEnd(o);
		Teleport = false;
	}
	else
	{
        ClearItems();
		ClearCharacters(HeroKey);
		RemoveAllShopTitleExceptHero();
		if(World != Data->Map)
		{
            int OldWorld = World;
			
			World = Data->Map;
			OpenWorld(World);
			
			if(World == WD_34CRYWOLF_1ST)
				SendRequestCrywolfInfo();
			
            if ( ( InChaosCastle(OldWorld) == true && OldWorld!=World ) || InChaosCastle()==true )
            {
                PlayBuffer( SOUND_CHAOS_ENVIR, NULL, true );
				
				g_pNewUISystem->Hide(SEASON3B::INTERFACE_FRIEND);
				
                SetCharacterClass ( Hero );
				//카오스 캐슬에서는 헬퍼 표시 안함
				DeleteBug(&Hero->Object);
            }
            if ( InChaosCastle()==false )
            {
                StopBuffer ( SOUND_CHAOSCASTLE, true );
                StopBuffer ( SOUND_CHAOS_ENVIR, true );
            }

#ifdef LDS_ADD_EMPIRE_GUARDIAN
			if( IsEmpireGuardian1() == false &&
				IsEmpireGuardian2() == false &&
				IsEmpireGuardian3() == false &&
				IsEmpireGuardian4() == false )
			{
				StopBuffer(SOUND_EMPIREGUARDIAN_WEATHER_RAIN, true);
				StopBuffer(SOUND_EMPIREGUARDIAN_WEATHER_FOG, true);
				StopBuffer(SOUND_EMPIREGUARDIAN_WEATHER_STORM, true);
				StopBuffer(SOUND_EMPIREGUARDIAN_INDOOR_SOUND, true);
			}
#endif //LDS_ADD_EMPIRE_GUARDIAN

            matchEvent::CreateEventMatch ( World );
			
            if ( World==-1 || Hero->Helper.Type!=MODEL_HELPER+3 || Hero->SafeZone )
            {
                o->Position[2] = RequestTerrainHeight(o->Position[0],o->Position[1]);
            }
            else
            {
                if ( World==WD_8TARKAN || World==WD_10HEAVEN )
                    o->Position[2] = RequestTerrainHeight(o->Position[0],o->Position[1])+90.f;
                else
                    o->Position[2] = RequestTerrainHeight(o->Position[0],o->Position[1])+30.f;
            }
			
#ifdef YDG_ADD_DOPPELGANGER_EVENT
			if (World >= WD_65DOPPLEGANGER1 && World <= WD_68DOPPLEGANGER4);
			else
#endif	// YDG_ADD_DOPPELGANGER_EVENT
			{
				char Text[256];
#ifndef GRAMMAR_VERB
				sprintf(Text,"%s%s",GetMapName( World),GlobalText[484]);
#else //GRAMMAR_VERB
				sprintf(Text,"%s%s",GlobalText[484],GetMapName( World));
#endif //GRAMMAR_VERB
			
				g_pChatListBox->AddText("", Text, SEASON3B::TYPE_SYSTEM_MESSAGE);
			}
		}
		SendRequestFinishLoading();

#ifdef LDS_FIX_DISABLE_INPUTJUNKKEY_WHEN_LORENMARKT_EX01
		g_bReponsedMoveMapFromServer = TRUE;			// 텔레포트로 맵이동 월드이동OK
#endif // LDS_FIX_DISABLE_INPUTJUNKKEY_WHEN_LORENMARKT_EX01
		LoadingWorld = 30;

		MouseUpdateTime = 0;
		MouseUpdateTimeMax = 6;
		
#ifdef YDG_ADD_DOPPELGANGER_EVENT
		if( World < WD_65DOPPLEGANGER1 || World > WD_68DOPPLEGANGER4 )
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_DOPPELGANGER_FRAME);
		}
#endif	// YDG_ADD_DOPPELGANGER_EVENT
#ifdef LDK_ADD_EMPIREGUARDIAN_PROTOCOLS
		if( World < WD_69EMPIREGUARDIAN1 || WD_72EMPIREGUARDIAN4 < World)
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_EMPIREGUARDIAN_TIMER);
		}
#endif //LDK_ADD_EMPIREGUARDIAN_PROTOCOLS

		g_pNewUISystem->HideAll();
		
#ifdef PET_SYSTEM
        CreatePetDarkSpirit_Now ( Hero );
#endif// PET_SYSTEM
		CreateEffect(BITMAP_MAGIC+2,o->Position,o->Angle,o->Light,0,o);
		o->Alpha = 0.f;
        EnableEvent = 0; //USE_EVENT_ELDORADO
		
        //  초기화.
        SelectedItem      = -1;
        SelectedNpc       = -1;
        SelectedCharacter = -1;
        SelectedOperate   = -1;
        Attacking         = -1;
		
        //  수리.
        RepairEnable = 0;
    }
	
    Hero->Movement = false;
    SetPlayerStop(Hero);
	
	if(Data->Flag) 
		g_pUIMapName->ShowMapName();	// rozy

#ifdef ASG_ADD_INFLUENCE_GROUND_EFFECT
	CreateMyGensInfluenceGroundEffect();
#endif	// ASG_ADD_INFLUENCE_GROUND_EFFECT
	
	if( battleCastle::InBattleCastle() && battleCastle::IsBattleCastleStart() )
	{
		Hero->EtcPart = iEtcPart;
	}
	
#ifdef LJH_FIX_NOT_INITIALIZING_BATTLECASTLE_UI	
	if( World == WD_30BATTLECASTLE )
	{
		if( g_pSiegeWarfare )
			g_pSiegeWarfare->CreateMiniMapUI();
	} else 
	{
		g_pSiegeWarfare->InitMiniMapUI();
	}
#endif //LJH_FIX_NOT_INITIALIZING_BATTLECASTLE_UI

#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x1C [ReceiveTeleport(%d)]", Data->Flag);
#endif // CONSOLE_DEBUG
	
#ifdef PJH_DEBUG
	wsprintf(Text,"맵로딩완료(tel)[time : %d]", GetTickCount());
	g_pChatListBox->AddText("DEBUG",Text, SEASON3B::TYPE_GM_MESSAGE);
#endif
	return ( TRUE);
}

///////////////////////////////////////////////////////////////////////////////
// 케릭터, 뷰포트
///////////////////////////////////////////////////////////////////////////////

void ReceiveEquipment( BYTE *ReceiveBuffer )
{
	LPPRECEIVE_EQUIPMENT Data = (LPPRECEIVE_EQUIPMENT)ReceiveBuffer;
	int Key = ((int)(Data->KeyH )<<8) + Data->KeyL;
	ChangeCharacterExt(FindCharacterIndex(Key),Data->Equipment);
}

//  화면상의 캐릭터가 장비를 교체했을때 모습이 바뀌는 처리.
void ReceiveChangePlayer( BYTE *ReceiveBuffer )
{	
	LPPCHANGE_CHARACTER Data = (LPPCHANGE_CHARACTER)ReceiveBuffer;
	int Key = ((int)(Data->KeyH )<<8) + Data->KeyL;
    CHARACTER *c = &CharactersClient[FindCharacterIndex(Key)];
	OBJECT *o = &c->Object;
	
	int Type = ConvertItemType(Data->Item); //  아이템 종류.
	BYTE Level = Data->Item[1]&0xf;         //  레벨.
    BYTE Option= Data->Item[3]&63;          //  액설런트 아이템.
    BYTE ExtOption = Data->Item[4];         //  세트 옵션.
	
    //  장비 부위.
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
    int maxClass = MAX_CLASS;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	
	switch(Data->Item[1]>>4)
	{
	case 0:
		if(Type==0x1FFF)
        {
            c->Weapon[0].Type = -1;
            c->Weapon[0].Option1 = 0;
        }
		else
		{
			c->Weapon[0].Type = MODEL_ITEM + Type;
			c->Weapon[0].Level = LevelConvert(Level);
            c->Weapon[0].Option1 = Option;
		}
		break;
	case 1:
		if(Type==0x1FFF)
        {
            c->Weapon[1].Type = -1;
            c->Weapon[1].Option1 = 0;
#ifdef PET_SYSTEM
#ifdef KJH_FIX_JP0467_RENDER_DARKLOAD_PET_OTHER_CHARATER
            DeletePet ( c );            
#else // KJH_FIX_JP0467_RENDER_DARKLOAD_PET_OTHER_CHARATER
            DeletePet ( Hero );            
#endif // KJH_FIX_JP0467_RENDER_DARKLOAD_PET_OTHER_CHARATER
#endif// PET_SYSTEM
        }
		else
		{
			c->Weapon[1].Type = MODEL_ITEM + Type;
			c->Weapon[1].Level = LevelConvert(Level);
            c->Weapon[1].Option1 = Option;
#ifdef PET_SYSTEM
            CreatePetDarkSpirit_Now ( c );
#endif// PET_SYSTEM
			g_SummonSystem.RemoveEquipEffects( c );
		}
		break;
	case 2:
		if(Type==0x1FFF)
		{
			c->BodyPart[BODYPART_HELM].Type = MODEL_BODY_HELM+GetSkinModelIndex(c->Class);
			c->BodyPart[BODYPART_HELM].Level = 0;
			c->BodyPart[BODYPART_HELM].Option1 = 0;
			c->BodyPart[BODYPART_HELM].ExtOption = 0;
		}
		else
		{
			c->BodyPart[BODYPART_HELM].Type = MODEL_ITEM+Type;
			c->BodyPart[BODYPART_HELM].Level = LevelConvert(Level);
			c->BodyPart[BODYPART_HELM].Option1 = Option;
			c->BodyPart[BODYPART_HELM].ExtOption = ExtOption;
		}
		break;
	case 3:
		if(Type==0x1FFF)
		{
			c->BodyPart[BODYPART_ARMOR].Type = MODEL_BODY_ARMOR+GetSkinModelIndex(c->Class);
			c->BodyPart[BODYPART_ARMOR].Level = 0;
			c->BodyPart[BODYPART_ARMOR].Option1 = 0;
			c->BodyPart[BODYPART_ARMOR].ExtOption= 0;
		}
		else
		{
			c->BodyPart[BODYPART_ARMOR].Type = MODEL_ITEM+Type;
			c->BodyPart[BODYPART_ARMOR].Level = LevelConvert(Level);
			c->BodyPart[BODYPART_ARMOR].Option1 = Option;
			c->BodyPart[BODYPART_ARMOR].ExtOption= ExtOption;
		}
		break;
	case 4:
		if(Type==0x1FFF)
		{
			c->BodyPart[BODYPART_PANTS].Type = MODEL_BODY_PANTS+GetSkinModelIndex(c->Class);
			c->BodyPart[BODYPART_PANTS].Level = 0;
			c->BodyPart[BODYPART_PANTS].Option1 = 0;
			c->BodyPart[BODYPART_PANTS].ExtOption = 0;
		}
		else
		{
			c->BodyPart[BODYPART_PANTS].Type = MODEL_ITEM+Type;
			c->BodyPart[BODYPART_PANTS].Level = LevelConvert(Level);
			c->BodyPart[BODYPART_PANTS].Option1 = Option;
			c->BodyPart[BODYPART_PANTS].ExtOption = ExtOption;
		}
		break;
	case 5:
		if(Type==0x1FFF)
		{
			c->BodyPart[BODYPART_GLOVES].Type = MODEL_BODY_GLOVES+GetSkinModelIndex(c->Class);
			c->BodyPart[BODYPART_GLOVES].Level = 0;
			c->BodyPart[BODYPART_GLOVES].Option1 = 0;
			c->BodyPart[BODYPART_GLOVES].ExtOption = 0;
		}
		else
		{
			c->BodyPart[BODYPART_GLOVES].Type = MODEL_ITEM+Type;
			c->BodyPart[BODYPART_GLOVES].Level = LevelConvert(Level);
			c->BodyPart[BODYPART_GLOVES].Option1 = Option;
			c->BodyPart[BODYPART_GLOVES].ExtOption = ExtOption;
		}
		break;
	case 6:
		if(Type==0x1FFF)
		{
			c->BodyPart[BODYPART_BOOTS].Type = MODEL_BODY_BOOTS+GetSkinModelIndex(c->Class);
			c->BodyPart[BODYPART_BOOTS].Level = 0;
			c->BodyPart[BODYPART_BOOTS].Option1 = 0;
			c->BodyPart[BODYPART_BOOTS].ExtOption = 0;
		}
		else
		{
			c->BodyPart[BODYPART_BOOTS].Type = MODEL_ITEM+Type;
			c->BodyPart[BODYPART_BOOTS].Level = LevelConvert(Level);
			c->BodyPart[BODYPART_BOOTS].Option1 = Option;
			c->BodyPart[BODYPART_BOOTS].ExtOption = ExtOption;
		}
		break;
	case 7:
		if(Type==0x1FFF)
		{
#ifdef LDK_ADD_INGAMESHOP_SMALL_WING
			if (c->Wing.Type == MODEL_WING+39 ||
				c->Wing.Type==MODEL_HELPER+30 ||
				c->Wing.Type==MODEL_WING+130 ||   // 추가
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
				c->Wing.Type == MODEL_WING+49 ||
				c->Wing.Type == MODEL_WING+50 ||
				c->Wing.Type == MODEL_WING+135||
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
				c->Wing.Type==MODEL_WING+40)
#else //LDK_ADD_INGAMESHOP_SMALL_WING
			if (c->Wing.Type == MODEL_WING+39 || c->Wing.Type==MODEL_HELPER+30 || c->Wing.Type==MODEL_WING+40)
#endif //LDK_ADD_INGAMESHOP_SMALL_WING
			{
				DeleteCloth(c, o);
			}
			c->Wing.Type = -1;
		}
		else
		{
			c->Wing.Type = MODEL_ITEM + Type;
			c->Wing.Level = 0;
#ifdef LDK_ADD_INGAMESHOP_SMALL_WING
			if (c->Wing.Type == MODEL_WING+39 ||
				c->Wing.Type==MODEL_HELPER+30 ||
				c->Wing.Type==MODEL_WING+130 ||   // 추가
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
				c->Wing.Type == MODEL_WING+49 ||
				c->Wing.Type == MODEL_WING+50 ||
				c->Wing.Type == MODEL_WING+135||
#endif //PBG_ADD_NEWCHAR_MONK_ITEM				
				c->Wing.Type==MODEL_WING+40)
#else //LDK_ADD_INGAMESHOP_SMALL_WING
			if (c->Wing.Type == MODEL_WING+39 || c->Wing.Type==MODEL_HELPER+30 || c->Wing.Type==MODEL_WING+40)
#endif //LDK_ADD_INGAMESHOP_SMALL_WING
			{
				DeleteCloth(c, o);
			}
		}
		break;
	case 8:
		if(Type==0x1FFF)
		{
			c->Helper.Type = -1;
			DeleteBug(o);
#ifdef LDK_ADD_NEW_PETPROCESS
			ThePetProcess().DeletePet(c, c->Helper.Type, true);
#endif //LDK_ADD_NEW_PETPROCESS

		}
		else
		{
			c->Helper.Type = MODEL_ITEM + Type;
			//c->Helper.Level = LevelConvert(Level);
			c->Helper.Level = 0;
			switch(Type)
			{
			case ITEM_HELPER  :CreateBug(MODEL_HELPER  ,o->Position,o);break;
			case ITEM_HELPER+2:CreateBug(MODEL_UNICON  ,o->Position,o);break;
			case ITEM_HELPER+3:CreateBug(MODEL_PEGASUS, o->Position,o);break;
			case ITEM_HELPER+4:CreateBug(MODEL_DARK_HORSE ,o->Position,o);break;
			case ITEM_HELPER+37:	// 펜릴 생성	//^ 펜릴 모델 생성
				c->Helper.Option1 = Option;
				if(Option == 0x01)
				{
					CreateBug(MODEL_FENRIR_BLACK, o->Position, o);
				}
				else if(Option == 0x02)
				{
					CreateBug(MODEL_FENRIR_BLUE, o->Position, o);
				}
				else if(Option == 0x04)
				{
					CreateBug(MODEL_FENRIR_GOLD, o->Position, o);
				}
				else
				{
					CreateBug(MODEL_FENRIR_RED, o->Position, o);
				}
				break;
#if defined(LDK_ADD_NEW_PETPROCESS) && defined(LDK_ADD_PC4_GUARDIAN)
			case ITEM_HELPER+64:
			case ITEM_HELPER+65:
				{
#if	SELECTED_LANGUAGE == LANGUAGE_JAPANESE
					if( CHARACTER_SCENE == SceneFlag || c == Hero )
#endif	//SELECTED_LANGUAGE == LANGUAGE_JAPANESE
					{
						ThePetProcess().CreatePet( Type, c->Helper.Type, o->Position, c );
					}
				}
				break;
#endif //defined(LDK_ADD_NEW_PETPROCESS) && defined(LDK_ADD_PC4_GUARDIAN) && (SELECTED_LANGUAGE != LANGUAGE_JAPANESE)
#if defined LDK_ADD_NEW_PETPROCESS && defined LDK_ADD_RUDOLPH_PET
			case ITEM_HELPER+67:ThePetProcess().CreatePet( Type, c->Helper.Type, o->Position, c );break;
#endif //defined LDK_ADD_NEW_PETPROCESS && defined LDK_ADD_RUDOLPH_PET
#if defined LDK_ADD_NEW_PETPROCESS && defined PJH_ADD_PANDA_PET
			case ITEM_HELPER+80:ThePetProcess().CreatePet( Type, c->Helper.Type, o->Position, c );break;
#endif //LDK_ADD_NEW_PETPROCESS && defined PJH_ADD_PANDA_PET
#if defined LDK_ADD_NEW_PETPROCESS && defined LDK_ADD_CS7_UNICORN_PET
			case ITEM_HELPER+106:ThePetProcess().CreatePet( Type, c->Helper.Type, o->Position, c );break;
#endif //defined LDK_ADD_NEW_PETPROCESS && defined LDK_ADD_CS7_UNICORN_PET
#if defined LDK_ADD_NEW_PETPROCESS && defined YDG_ADD_SKELETON_PET
			case ITEM_HELPER+123:ThePetProcess().CreatePet( Type, c->Helper.Type, o->Position, c );break;
#endif //defined LDK_ADD_NEW_PETPROCESS && defined YDG_ADD_SKELETON_PET
			}
		}
		break;
	}

    ChangeChaosCastleUnit ( c );
	
	SetCharacterScale(c);
}

void RegisterBuff( eBuffState buff, OBJECT* o, const int bufftime = 0 );

void UnRegisterBuff( eBuffState buff, OBJECT* o );

//  화면에 들어온 캐릭터를 생성한다.
void ReceiveCreatePlayerViewport(BYTE *ReceiveBuffer,int Size)
{
	LPPWHEADER_DEFAULT_WORD Data = (LPPWHEADER_DEFAULT_WORD)ReceiveBuffer;
	int Offset = sizeof(PWHEADER_DEFAULT_WORD);
	
	for(int i=0;i<Data->Value;i++)
	{
		LPPCREATE_CHARACTER Data2 = (LPPCREATE_CHARACTER)(ReceiveBuffer+Offset);
		WORD Key = ((WORD)(Data2->KeyH)<<8) + Data2->KeyL;
		int CreateFlag = (Key>>15);
		Key &= 0x7FFF;
		
		char Temp[MAX_ID_SIZE+1];
		::memcpy(Temp, (char*)Data2->ID, MAX_ID_SIZE);
		Temp[MAX_ID_SIZE] = NULL;
        if(FindText(Temp, "webzen") == false)
		{
			// 텔레포트로 인한 캐릭재생성일 경우 길드관련 변수들 백업
			int Index;
			short BackUpGuildMarkIndex = -1; 
			BYTE BackUpGuildStatus = 0;
			BYTE BackUpGuildType = 0;
			BYTE BackUpGuildRelationShip = 0;
			BYTE BackUpGuildMasterKillCount = 0;
            BYTE  EtcPart = 0;
			// ctlcode 백업
			BYTE BackupCtlcode = 0;
			
			if( (Data2->Class&0x07) == 1 )	// 공간이동이라면.
			{
				Index = FindCharacterIndex(Key);		// 인덱스를 찾아서 저장
				if( Index != MAX_CHARACTERS_CLIENT )	// 인덱스가 400이 아니라면
				{
					BackUpGuildMarkIndex = CharactersClient[Index].GuildMarkIndex;
					BackUpGuildStatus = CharactersClient[Index].GuildStatus;
					BackUpGuildType = CharactersClient[Index].GuildType;
					BackUpGuildRelationShip = CharactersClient[Index].GuildRelationShip;
					EtcPart = CharactersClient[Index].EtcPart;
					BackUpGuildMasterKillCount = CharactersClient[Index].GuildMasterKillCount;
				}

				if(&CharactersClient[Index] == Hero)	// 자기 자신이면
				{
					BackupCtlcode	= CharactersClient[Index].CtlCode;
				}
			}
			else	// 다른 클래스라면
			{
				Index = FindCharacterIndex(Key);
				if( Index != MAX_CHARACTERS_CLIENT )	// 인덱스가 400이 아니라면
				{
					BackUpGuildMarkIndex = CharactersClient[Index].GuildMarkIndex;
					BackUpGuildStatus = CharactersClient[Index].GuildStatus;
					BackUpGuildType = CharactersClient[Index].GuildType;
					BackUpGuildRelationShip = CharactersClient[Index].GuildRelationShip;
					BackUpGuildMasterKillCount = CharactersClient[Index].GuildMasterKillCount;
					EtcPart = CharactersClient[Index].EtcPart;
				}
					
				if(&CharactersClient[Index] == Hero)	// 자기 자신이면
				{
					BackupCtlcode	= CharactersClient[Index].CtlCode;
				}
			}
			
			// 캐릭터 새로 생성
			CHARACTER *c = CreateCharacter(Key,MODEL_PLAYER,Data2->PositionX,Data2->PositionY,0);
			DeleteCloth(c, &c->Object);	// 옷감 관련 제거
			
			OBJECT *o = &c->Object;
			c->Class = ChangeServerClassTypeToClientClassType(Data2->Class);
			c->Skin = 0;
			c->PK    = Data2->Path&0xf;		// Data2->Path뒤의 4바이트가 PK레벨
			o->Kind  = KIND_PLAYER;
				
#ifdef PK_ATTACK_TESTSERVER_LOG
			PrintPKLog(c);
#endif // PK_ATTACK_TESTSERVER_LOG	
				
			switch (Data2->Class&0x07)
			{
			case 1:
				CreateTeleportEnd(o);
				AddDebugText(ReceiveBuffer,Size);
				break;
			case 2:
				if(!IsFemale(c->Class))
					SetAction(o,PLAYER_SIT1);
				else
					SetAction(o,PLAYER_SIT_FEMALE1);
				break;
			case 3:
				if(!IsFemale(c->Class))
					SetAction(o,PLAYER_POSE1);
				else
					SetAction(o,PLAYER_POSE_FEMALE1);
				break;
			case 4:
				if(!IsFemale(c->Class))
					SetAction(o,PLAYER_HEALING1);
				else
					SetAction(o,PLAYER_HEALING_FEMALE1);
				break;
			}

			// 위치, 타켓, 각도 입력
			c->PositionX = Data2->PositionX;
			c->PositionY = Data2->PositionY;
#ifdef CSK_FIX_SYNCHRONIZATION
			c->TargetX = Data2->TargetX;
			c->TargetY = Data2->TargetY;
#else // CSK_FIX_SYNCHRONIZATION
			c->TargetX = Data2->PositionX;
			c->TargetY = Data2->PositionY;
#endif // CSK_FIX_SYNCHRONIZATION	
			
			c->Object.Angle[2] = ((float)(Data2->Path>>4)-1.f)*45.f;
			
#ifdef CONSOLE_DEBUG
			g_ConsoleDebug->Write(MCD_RECEIVE, "(RCPV)ID : %s | sX : %d | sY : %d | tX : %d | tY : %d", c->ID, c->PositionX, c->PositionY, c->TargetX, c->TargetY);
#endif // CONSOLE_DEBUG
				
			if(CreateFlag)
			{
				c->Object.Position[0] = (( c->PositionX)+0.5f)*TERRAIN_SCALE;
				c->Object.Position[1] = (( c->PositionY)+0.5f)*TERRAIN_SCALE;
				CreateEffect(BITMAP_MAGIC+2,o->Position,o->Angle,o->Light,0,o);
				c->Object.Alpha = 0.f;
			}
			else if(PathFinding2(c->PositionX, c->PositionY, Data2->TargetX, Data2->TargetY, &c->Path))
			{
				c->Movement = true;
			}
			
			if ( InHellas() )
			{
				CreateJoint ( BITMAP_FLARE+1, o->Position, o->Position, o->Angle, 8, o, 20.f );
			}
#ifdef LJW_FIX_MANY_FLAG_DISAPPEARED_PROBREM // 복원하는 순서를 바꿈
			// 텔레포트로 인한 캐릭재생성일 경우 길드관련 변수들 복원
			if( (Data2->Class&0x07) == 1 && Index != MAX_CHARACTERS_CLIENT )
			{	
				c->GuildMarkIndex = BackUpGuildMarkIndex;
				c->GuildStatus = BackUpGuildStatus;
				c->GuildType = BackUpGuildType;
				c->GuildRelationShip = BackUpGuildRelationShip;
				c->EtcPart = EtcPart;
				c->GuildMasterKillCount = BackUpGuildMasterKillCount;
				
				if(&CharactersClient[Index] == Hero)
				{
					c->CtlCode = BackupCtlcode;
				}
			}
			else
			{
				c->GuildMarkIndex = BackUpGuildMarkIndex;
				c->GuildStatus = BackUpGuildStatus;
				c->GuildType = BackUpGuildType;
				c->GuildRelationShip = BackUpGuildRelationShip;
				c->GuildMasterKillCount = BackUpGuildMasterKillCount;
				c->EtcPart = EtcPart;						
				if(&CharactersClient[Index] == Hero)
				{
					c->CtlCode = BackupCtlcode;
				}
			}

			ChangeCharacterExt(FindCharacterIndex(Key),Data2->Equipment);
#else
			ChangeCharacterExt(FindCharacterIndex(Key),Data2->Equipment);
				
			// 텔레포트로 인한 캐릭재생성일 경우 길드관련 변수들 복원
			if( (Data2->Class&0x07) == 1 && Index != MAX_CHARACTERS_CLIENT )
			{	
				c->GuildMarkIndex = BackUpGuildMarkIndex;
				c->GuildStatus = BackUpGuildStatus;
				c->GuildType = BackUpGuildType;
				c->GuildRelationShip = BackUpGuildRelationShip;
				c->EtcPart = EtcPart;
				c->GuildMasterKillCount = BackUpGuildMasterKillCount;
						
				if(&CharactersClient[Index] == Hero)
				{
					c->CtlCode = BackupCtlcode;
				}
			}
			else
			{
				c->GuildMarkIndex = BackUpGuildMarkIndex;
				c->GuildStatus = BackUpGuildStatus;
				c->GuildType = BackUpGuildType;
				c->GuildRelationShip = BackUpGuildRelationShip;
				c->GuildMasterKillCount = BackUpGuildMasterKillCount;
				c->EtcPart = EtcPart;						
				if(&CharactersClient[Index] == Hero)
				{
					c->CtlCode = BackupCtlcode;
				}
			}
#endif
			
			memcpy(c->ID,(char *)Data2->ID,MAX_ID_SIZE);
			c->ID[MAX_ID_SIZE] = NULL;
					
			if ((Data2->Class&0x07) == 1 && Index != MAX_CHARACTERS_CLIENT)
			{
				c->EtcPart = EtcPart;
			}
						
			// 마법 상태 세팅
			for( int j = 0; j < Data2->s_BuffCount; ++j )
			{
				RegisterBuff(static_cast<eBuffState>(Data2->s_BuffEffectState[j]), o);

				battleCastle::SettingBattleFormation ( c, static_cast<eBuffState>(Data2->s_BuffEffectState[j]) );
#ifdef CONSOLE_DEBUG
				g_ConsoleDebug->Write(MCD_RECEIVE, "ID : %s, Buff : %d", c->ID, static_cast<int>(Data2->s_BuffEffectState[j]));
#endif // CONSOLE_DEBUG				
			}

			if( battleCastle::InBattleCastle() && battleCastle::IsBattleCastleStart() )
			{
				//g_pSiegeWarfare->InitSkillUI();
			}
        }
		
		Offset += (sizeof(PCREATE_CHARACTER)-(sizeof(BYTE)*(MAX_BUFF_SLOT_INDEX-Data2->s_BuffCount)));
	}
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x12 [ReceiveCreatePlayerViewport(%d)]", Data->Value);
#endif // CONSOLE_DEBUG
}

// 변신 반지 착용하면 받는 정보
void ReceiveCreateTransformViewport( BYTE *ReceiveBuffer )
{
	LPPWHEADER_DEFAULT_WORD Data = (LPPWHEADER_DEFAULT_WORD)ReceiveBuffer;
	int Offset = sizeof(PWHEADER_DEFAULT_WORD);
	
	for(int i=0;i<Data->Value;i++)
	{
		LPPCREATE_TRANSFORM Data2 = (LPPCREATE_TRANSFORM)(ReceiveBuffer+Offset);
		WORD Key = ((WORD)(Data2->KeyH)<<8) + Data2->KeyL;
		int CreateFlag = (Key>>15);
		Key &= 0x7FFF;
		
		char Temp[MAX_ID_SIZE+1];
		memcpy(Temp,(char *)Data2->ID,MAX_ID_SIZE);
		Temp[MAX_ID_SIZE] = NULL;
		
		CHARACTER *pCha;
		int iIndex = FindCharacterIndex(Key);
		pCha = &CharactersClient[iIndex];
		
		// 공성전 관련 백업
		short sBackUpGuildMarkIndex = -1; 
		BYTE byBackUpGuildStatus = 0;
		BYTE byBackUpGuildType = 0;
		BYTE byBackUpGuildRelationShip = 0;
		BYTE byBackUpGuildMasterKillCount = 0;
		BYTE byEtcPart = 0;
		BYTE byBackupCtlcode = 0;
		
		if(iIndex != MAX_CHARACTERS_CLIENT)
		{
			sBackUpGuildMarkIndex = pCha->GuildMarkIndex;
			byBackUpGuildStatus = pCha->GuildStatus;
			byBackUpGuildType = pCha->GuildType;
			byBackUpGuildRelationShip = pCha->GuildRelationShip;
			byBackUpGuildMasterKillCount = pCha->GuildMasterKillCount;
			byEtcPart = pCha->EtcPart;
			byBackupCtlcode = pCha->CtlCode;	
		}
		
        if(FindText(Temp, "webzen") == false)
		{
			// 클래스 입력
			int Class = ChangeServerClassTypeToClientClassType(Data2->Class);
            
			// 변신할 타입
            WORD Type = ((WORD)(Data2->TypeH)<<8) + Data2->TypeL;
			
			// 몬스터 생성
			CHARACTER *c = CreateMonster(Type,Data2->PositionX,Data2->PositionY,Key);
			OBJECT *o = &c->Object;
			
			if(c->MonsterIndex == 7)	// 자이언트
			{
				// 원래 자이언트 몬스터는 스케일이 큰데 왜 자이언트로 변신하면 스케일을
				// 작게 만들었을까?
				o->Scale = 0.8f;
			}
			
			if(Type == 373)
			{
				DeleteCloth(c, o);
			}
			
			if( Type == 404 || Type == 405 )
			{
				DeleteCloth(c, o);
			}
			
			DeleteCloth(c, o);
			
			// 공성전 관련 저장
			c->GuildMarkIndex = sBackUpGuildMarkIndex;
			c->GuildStatus = byBackUpGuildStatus;
			c->GuildType = byBackUpGuildType;
			c->GuildRelationShip = byBackUpGuildRelationShip;
			c->GuildMasterKillCount = byBackUpGuildMasterKillCount;
			c->EtcPart = byEtcPart;
			c->CtlCode = byBackupCtlcode;
			c->Class  = Class;
			c->PK     = Data2->Path&0xf;
			o->Kind   = KIND_PLAYER;	// 종류를 플레이어로 설정
			c->Change = true;			// 변신상태 true 설정
			
#ifdef PK_ATTACK_TESTSERVER_LOG
			PrintPKLog(c);
#endif // PK_ATTACK_TESTSERVER_LOG	

			for( int j = 0; j < Data2->s_BuffCount; ++j )
			{
				RegisterBuff(static_cast<eBuffState>(Data2->s_BuffEffectState[j]), o);
				battleCastle::SettingBattleFormation ( c, static_cast<eBuffState>(Data2->s_BuffEffectState[j]) );
#ifdef CONSOLE_DEBUG
				g_ConsoleDebug->Write(MCD_RECEIVE, "ID : %s, Buff : %d", c->ID, static_cast<int>(Data2->s_BuffEffectState[j]));
#endif // CONSOLE_DEBUG	
			}
	
			c->PositionX = Data2->PositionX;
			c->PositionY = Data2->PositionY;
			c->TargetX = Data2->PositionX;
			c->TargetY = Data2->PositionY;
			c->Object.Angle[2] = ((float)(Data2->Path>>4)-1.f)*45.f;
			
			if(CreateFlag)
			{
				c->Object.Position[0] = (( c->PositionX)+0.5f)*TERRAIN_SCALE;
				c->Object.Position[1] = (( c->PositionY)+0.5f)*TERRAIN_SCALE;
				c->Object.Alpha = 0.f;
				CreateEffect(MODEL_MAGIC_CIRCLE1,o->Position,o->Angle,o->Light,0,o);
				CreateParticle(BITMAP_LIGHTNING+1,o->Position,o->Angle,o->Light,2,1.f,o);
			}
			else if(PathFinding2(( c->PositionX),( c->PositionY),Data2->TargetX,Data2->TargetY,&c->Path))
			{
				c->Movement = true;
			}
			
			// 아이디 입력
			memcpy(c->ID,(char *)Data2->ID,MAX_ID_SIZE);
			c->ID[MAX_ID_SIZE] = NULL;
			
			ChangeCharacterExt(FindCharacterIndex(Key), Data2->Equipment);
		}

		Offset += (sizeof(PCREATE_TRANSFORM)-(sizeof(BYTE)*(MAX_BUFF_SLOT_INDEX-Data2->s_BuffCount)));
	}
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x45 [ReceiveCreateTransformViewport(%d)]", Data->Value);
#endif // CONSOLE_DEBUG
}

void AppearMonster(CHARACTER *c)
{
	OBJECT *o = &c->Object;
	switch(c->MonsterIndex)
	{
	case 44://드래곤
		SetAction(o,MONSTER01_STOP2);
		o->PriorAction = MONSTER01_STOP2;
		c->Object.Alpha = 1.f;
		PlayBuffer(SOUND_MONSTER+124);
		break;
	case 21://암살자
		SetAction(o,MONSTER01_STOP2);
		o->PriorAction = MONSTER01_STOP2;
		c->Object.Alpha = 1.f;
		PlayBuffer(SOUND_ASSASSIN);
		break;
	case 53://지하괴물
	case 54://지하괴물
		c->Appear = 60;
		SetAction(o,MONSTER01_STOP2);
		o->PriorAction = MONSTER01_STOP2;
		c->Object.Alpha = 1.f;
		//PlayBuffer(SOUND_ASSASSIN);
		break;
	case 85 :	//. 오크궁수대장1
	case 91 :	//. 오크궁수대장2
	case 97 :	//. 오크궁수대장3
	case 114 :	//. 오크궁수대장4
	case 120 :	//. 오크궁수대장5
	case 126 :	//. 오크궁수대장6
		PlayBuffer(SOUND_MONSTER+161);
        break;
		// MVP 몹 소환 될때 효과
	case 440:
	case 340:	// 다크엘프
	case 341:	// 소람
	case 344:	// 발람
	case 345:	// 데스 스피릿
		{
			if(g_Direction.m_CMVP.m_iCryWolfState == CRYWOLF_STATE_READY)
				c->Object.Alpha = 1.0f;
			else
			{
				c->Object.Alpha = 0.f;
				
				vec3_t vPos;
				Vector(c->Object.Position[0]+20.0f, c->Object.Position[1]+20.0f, c->Object.Position[2], vPos);
				
				// 몬스터 소환 효과 - 하늘에서 번개 떨어지기
				CreateJoint ( BITMAP_JOINT_THUNDER+1, vPos, vPos, c->Object.Angle, 7, NULL, 60.f+rand()%10 );
				CreateJoint ( BITMAP_JOINT_THUNDER+1, vPos, vPos, c->Object.Angle, 7, NULL, 50.f+rand()%10 );
				CreateJoint ( BITMAP_JOINT_THUNDER+1, vPos, vPos, c->Object.Angle, 7, NULL, 50.f+rand()%10 );
				CreateJoint ( BITMAP_JOINT_THUNDER+1, vPos, vPos, c->Object.Angle, 7, NULL, 60.f+rand()%10 );
				
				// 몬스터 소환 효과 - 번개 떨어진 지점에 연기 나기
				CreateParticle ( BITMAP_SMOKE+4, c->Object.Position, c->Object.Angle, c->Object.Light, 1, 5.0f);
				CreateParticle ( BITMAP_SMOKE+4, c->Object.Position, c->Object.Angle, c->Object.Light, 1, 5.0f);
				CreateParticle ( BITMAP_SMOKE+4, c->Object.Position, c->Object.Angle, c->Object.Light, 1, 5.0f);
				
				// 몬스터 소환 효과 - 몹 나타 나면서 주위에 번개 효과
				Vector(c->Object.Position[0], c->Object.Position[1], c->Object.Position[2]+120.0f, vPos);
				CreateJoint(BITMAP_JOINT_THUNDER, c->Object.Position, vPos, c->Object.Angle, 17);
				CreateJoint(BITMAP_JOINT_THUNDER, c->Object.Position, vPos, c->Object.Angle, 17);
				CreateJoint(BITMAP_JOINT_THUNDER, c->Object.Position, vPos, c->Object.Angle, 17);
				CreateJoint(BITMAP_JOINT_THUNDER, c->Object.Position, vPos, c->Object.Angle, 17);
			}
		}
		break;
	case 362:
	case 363:
		{
			SetAction(o,MONSTER01_APEAR);
			c->Object.Alpha = 0.f;
		}
		break;
	default:
		if(o->Type == MODEL_PLAYER)
		{
			SetAction(o,PLAYER_COME_UP);
			o->PriorAction = PLAYER_COME_UP;
			c->Object.Alpha = 1.f;
		}
		else
		{
			c->Object.Alpha = 0.f;
		}
		break;
	}
}

// 화면에 들어온 몬스터를 생성한다.
void ReceiveCreateMonsterViewport( BYTE *ReceiveBuffer )
{
	LPPWHEADER_DEFAULT_WORD Data = (LPPWHEADER_DEFAULT_WORD)ReceiveBuffer;
	int Offset = sizeof(PWHEADER_DEFAULT_WORD);
	for(int i=0;i<Data->Value;i++)
	{
		LPPCREATE_MONSTER Data2 = (LPPCREATE_MONSTER)(ReceiveBuffer+Offset);
		WORD Key       = ((WORD)(Data2->KeyH)<<8) + Data2->KeyL;
		
		
        BYTE bMyMob     = (Data2->TypeH)&0x80;
        BYTE byBuildTime= (Data2->TypeH&0x70)>>4;
        WORD Type       = ((WORD)(Data2->TypeH&0x03)<<8) + Data2->TypeL;

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		if(Type != 106)
			int a= 0;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		
		int CreateFlag = (Key>>15);
		// 몬스터가 순간이동 후 나올때 효과를 있는지 없는지
		int TeleportFlag = (Data2->KeyH&0x40)>>6;
		
		Key &= 0x7FFF;
		CHARACTER *c = CreateMonster(Type,Data2->PositionX,Data2->PositionY,Key);
		
#ifdef CONSOLE_DEBUG
		g_ConsoleDebug->Write(MCD_RECEIVE, "0x13 [ReceiveCreateMonsterViewport(Type : %d | Key : %d)]", Type, Key);
#endif // CONSOLE_DEBUG	
		
		if(c == NULL) break;

		OBJECT *o = &c->Object;

		for( int j = 0; j < Data2->s_BuffCount; ++j )
		{
			RegisterBuff(static_cast<eBuffState>(Data2->s_BuffEffectState[j]), o);

#ifdef CONSOLE_DEBUG
			g_ConsoleDebug->Write(MCD_RECEIVE, "ID : %s, Buff : %d", c->ID, static_cast<int>(Data2->s_BuffEffectState[j]));
#endif // CONSOLE_DEBUG	
		}
		
		float fAngle = 45.0f;
		if(o->Type == MODEL_MONSTER01+99)
		{
			if(c->PositionY == 90)
				fAngle = 0.0f;
			if(c->PositionX == 62)
				fAngle = -135.0f;
			if(c->PositionX == 183)
				fAngle = 90.0f;
		}
		else if(o->Type == MODEL_KANTURU2ND_ENTER_NPC)
		{
			o->m_fEdgeScale = 1.0f;
			fAngle = 90.f;
		}

		c->Object.Angle[2] = ((float)(Data2->Path>>4)-1.f)*fAngle;
#ifdef PBG_ADD_LITTLESANTA_NPC
		// 리틀산타의 바라보는 방향을 Z축 90.
		if(o->Type >= MODEL_LITTLESANTA+1 && o->Type <= MODEL_LITTLESANTA+4)
		{
			c->Object.Angle[2] = 90.f;
		}
		else if(o->Type >= MODEL_LITTLESANTA+6 && o->Type <= MODEL_LITTLESANTA+7)
		{
			c->Object.Angle[2] = 90.f;
		}
#endif //PBG_ADD_LITTLESANTA_NPC
#ifdef LDK_ADD_SANTA_NPC
		else if(o->Type == MODEL_XMAS2008_SANTA_NPC)
		{
			c->Object.Angle[2] = 20.f;
		}
#endif //LDK_ADD_SANTA_NPC		
#ifdef LDS_ADD_NPC_UNITEDMARKETPLACE
		else if(o->Type == MODEL_UNITEDMARKETPLACE_JULIA)
		{
			//c->Object.Angle[2] = 44.0f;
			c->Object.Angle[2] = 49.0f;
			//c->Object.Angle[2] = 90.0f;
		}
#endif // LDS_ADD_NPC_UNITEDMARKETPLACE

		c->PositionX = Data2->PositionX;
		c->PositionY = Data2->PositionY;
		c->TargetX = Data2->PositionX;
		c->TargetY = Data2->PositionY;
		
        c->m_byFriend = bMyMob;
        o->m_byBuildTime = byBuildTime;
		
        if ( battleCastle::InBattleCastle() && c->MonsterIndex==277 )        
        {
            //  State값에 따라 행동을 결정한다.
			if( g_isCharacterBuff((&c->Object), eBuff_CastleGateIsOpen) )	//  성문이 열려있는 상태.
			{
				SetAction ( &c->Object, 1 );
				battleCastle::SetCastleGate_Attribute ( Data2->PositionX, Data2->PositionY, 0 );
			}
			else														 //  성문이 닫혀있는 상태.
			{
				SetAction ( &c->Object, 0 );
				battleCastle::SetCastleGate_Attribute ( Data2->PositionX, Data2->PositionY, 1 );
			}
        }
		if(CreateFlag)
		{
            AppearMonster(c);
		}
		// 칸투르3차 나이트메어전에서 나이트메어 순간이동 후 나올때 효과
		else if(World == WD_39KANTURU_3RD && o->Type == MODEL_MONSTER01+121 && TeleportFlag)
		{
			vec3_t Light;
			Vector ( 1.0f, 1.0f, 1.0f, Light );
			o->AlphaTarget = 1.f;
			CreateEffect(BITMAP_SPARK+1,o->Position,o->Angle,Light);
			CreateEffect(BITMAP_SPARK+1,o->Position,o->Angle,Light);
			PlayBuffer(SOUND_KANTURU_3RD_NIGHTMARE_TELE);
		}
#ifdef YDG_ADD_DOPPELGANGER_MONSTER
		int iDefaultWall = TW_CHARACTER;	//보통일때는 2이상이면 벽으로 체크

		if (World >= WD_65DOPPLEGANGER1 && World <= WD_68DOPPLEGANGER4
			&& Key != HeroKey)
		{
			iDefaultWall = TW_NOMOVE;	// 도플갱어 몬스터는 유저를 뚫고지나간다
		}

		else if(PathFinding2(c->PositionX, c->PositionY, Data2->TargetX, Data2->TargetY, &c->Path, 0.0f, iDefaultWall))
#else	// YDG_ADD_DOPPELGANGER_MONSTER
		else if(PathFinding2(c->PositionX, c->PositionY, Data2->TargetX, Data2->TargetY, &c->Path))
#endif	// YDG_ADD_DOPPELGANGER_MONSTER
		{
			c->Movement = true;
		}
#ifdef KJH_FIX_BTS167_MOVE_NPC_IN_VIEWPORT
		else
		{
			c->Movement = false;			// 초기값
		}
#endif // KJH_FIX_BTS167_MOVE_NPC_IN_VIEWPORT

		Offset += (sizeof(PCREATE_MONSTER)-(sizeof(BYTE)*(MAX_BUFF_SLOT_INDEX-Data2->s_BuffCount)));
	}
}

void ReceiveCreateSummonViewport( BYTE *ReceiveBuffer )
{
	LPPWHEADER_DEFAULT_WORD Data = (LPPWHEADER_DEFAULT_WORD)ReceiveBuffer;
	int Offset = sizeof(PWHEADER_DEFAULT_WORD);
	for(int i=0;i<Data->Value;i++)
	{
		LPPCREATE_SUMMON Data2 = (LPPCREATE_SUMMON)(ReceiveBuffer+Offset);
		WORD Key       = ((WORD)(Data2->KeyH)<<8) + Data2->KeyL;
        WORD Type      = ((WORD)(Data2->TypeH)<<8) + Data2->TypeL;
		int CreateFlag = (Key>>15);
		Key &= 0x7FFF;
		
        CHARACTER* c;
#ifdef ADD_ELF_SUMMON
		// for test
#endif // ADD_ELF_SUMMON
        //  칼리마로 통하는 마석의 경우.
		if ( Type>=152 && Type<=158 )
        {
            c = CreateHellGate ( &Data2->ID[0], Key, Type, Data2->PositionX,Data2->PositionY, CreateFlag );
        }
        else
        {
			c = CreateMonster(Type,Data2->PositionX,Data2->PositionY,Key);
        }

		if(c == NULL) break;

		OBJECT *o = &c->Object;
		
		for( int j = 0; j < Data2->s_BuffCount; ++j )
		{
			RegisterBuff(static_cast<eBuffState>(Data2->s_BuffEffectState[j]), o);

#ifdef CONSOLE_DEBUG
			g_ConsoleDebug->Write(MCD_RECEIVE, "ID : %s, Buff : %d", c->ID, static_cast<int>(Data2->s_BuffEffectState[j]));
#endif // CONSOLE_DEBUG	
		}
		
		c->Object.Angle[2] = ((float)(Data2->Path>>4)-1.f)*45.f;
		c->PositionX = Data2->PositionX;
		c->PositionY = Data2->PositionY;
		c->TargetX = Data2->PositionX;
		c->TargetY = Data2->PositionY;
		o->Kind = KIND_PLAYER;
		c->PK   = Data2->Path&0xf;
		
#ifdef PK_ATTACK_TESTSERVER_LOG
		PrintPKLog(c);
#endif // PK_ATTACK_TESTSERVER_LOG
		
		if(c->PK >= PVP_MURDERER2)
			c->Level = 1;
		
        if ( Type<152 || Type>158 )
        {
            char Temp[100];
#ifdef GRAMMAR_OF
			// 글로벌
            strcat(c->ID,GlobalText[485]);
            memcpy(Temp,Data2->ID,MAX_ID_SIZE);
            Temp[MAX_ID_SIZE] = NULL;
            strcat(c->ID,Temp);
#else // GRAMMAR_OF
			// 한국
            strcpy(Temp,c->ID);
            memcpy(c->ID,Data2->ID,MAX_ID_SIZE);
            c->ID[MAX_ID_SIZE] = NULL;
            strcat(c->ID,GlobalText[485]);
            strcat(c->ID,Temp);
#endif // GRAMMAR_OF
#ifdef YDG_ADD_NEW_DUEL_SYSTEM
			// 만든 요정 ID 저장
            memcpy(c->OwnerID,Data2->ID,MAX_ID_SIZE);
            c->OwnerID[MAX_ID_SIZE] = NULL;
#endif	// YDG_ADD_NEW_DUEL_SYSTEM
        }
		
		if(CreateFlag)
		{
			AppearMonster(c);
			CreateEffect(MODEL_MAGIC_CIRCLE1,o->Position,o->Angle,o->Light,0,o);
			CreateParticle(BITMAP_LIGHTNING+1,o->Position,o->Angle,o->Light,2,1.f,o);
			if ( Type>=152 && Type<=158 )
            {
                o->PriorAnimationFrame = 0.f;
                o->AnimationFrame = 0.f;
                PlayBuffer ( SOUND_CHAOS_FALLING_STONE );
            }
		}
		else if(PathFinding2(c->PositionX, c->PositionY, Data2->TargetX,Data2->TargetY,&c->Path))
		{
			c->Movement = true;
		}

		Offset += (sizeof(PCREATE_SUMMON)-(sizeof(BYTE)*(MAX_BUFF_SLOT_INDEX-Data2->s_BuffCount)));
	}
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x1F [ReceiveCreateSummonViewport(%d)]", Data->Value);
#endif // CONSOLE_DEBUG
}

void ReceiveDeleteCharacterViewport( BYTE *ReceiveBuffer )
{
	LPPHEADER_DEFAULT Data = (LPPHEADER_DEFAULT)ReceiveBuffer;
	int Offset = sizeof(PHEADER_DEFAULT);
	for(int i=0; i<Data->Value; i++)
	{
		LPPDELETE_CHARACTER Data2 = (LPPDELETE_CHARACTER)(ReceiveBuffer+Offset);
		
		if(Switch_Info != NULL)
		{
			WORD Key = ((WORD)(Data2->KeyH)<<8) + Data2->KeyL;
			if(Key == FIRST_CROWN_SWITCH_NUMBER)
				{
					Switch_Info[0].Reset();
				}
				else
				{
					Switch_Info[1].Reset();
				}
		}
		
		int Key = ((int)(Data2->KeyH)<<8) + Data2->KeyL;
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		int DeleteFlag = (Key>>15);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		Key &= 0x7FFF;
		
		// 상점 연 캐릭터가 다른 맵으로 이동하면 구매창 닫는 처리 
		int iIndex = g_pPurchaseShopInventory->GetShopCharacterIndex();
		if(iIndex >=0 && iIndex < MAX_CHARACTERS_CLIENT)
		{
			CHARACTER* pCha = &CharactersClient[iIndex];
			if(pCha && pCha->Key == Key)
			{
				g_pNewUISystem->Hide(SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY);
			}
		}
		
		iIndex = FindCharacterIndex(Key);
		CHARACTER* pCha = &CharactersClient[iIndex];
		
		int buffSize = g_CharacterBuffSize( (&pCha->Object) );
		
		for( int k = 0; k < buffSize; k++ )
		{
			UnRegisterBuff( g_CharacterBuff( (&pCha->Object), k ), &pCha->Object );
		}

		DeleteCharacter(Key);
		CHARACTER* pPlayer = FindCharacterTagShopTitle(Key);
		if(pPlayer) 
		{
			RemoveShopTitle(pPlayer);
		}
		
		Offset += sizeof(PDELETE_CHARACTER);
	}
}

///////////////////////////////////////////////////////////////////////////////
// 공격
///////////////////////////////////////////////////////////////////////////////

int AttackPlayer = 0;

void ReceiveDamage( BYTE *ReceiveBuffer )
{
	LPPRECEIVE_DAMAGE Data = (LPPRECEIVE_DAMAGE)ReceiveBuffer;
	int Damage = ((int)(Data->DamageH)<<8) + Data->DamageL;
	if(CharacterAttribute->Life >= Damage)
		CharacterAttribute->Life -= Damage;
	else
		CharacterAttribute->Life = 0;
	
	int ShieldDamage = ((int)(Data->ShieldDamageH)<<8) + Data->ShieldDamageL;
	if(CharacterAttribute->Shield >= ShieldDamage)
		CharacterAttribute->Shield = ShieldDamage;
	else
		CharacterAttribute->Shield = 0;
}

void ProcessDamageCastle( LPPRECEIVE_ATTACK Data)
{
	int Key    = ((int)(Data->KeyH   )<<8) + Data->KeyL;
	int Success = (Key>>15);
	Key &= 0x7FFF;
	
	int Index = FindCharacterIndex(Key);
	CHARACTER *c = &CharactersClient[Index];
	OBJECT *o = &c->Object;
	vec3_t Light;
	WORD Damage			= (((WORD)(Data->DamageH)<<8) + Data->DamageL);
	int	 DamageType		= (Data->DamageType)&0x3f;
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
    bool bDoubleEnable  = (Data->DamageType>>6)&0x01;
	bool bComboEnable	= (Data->DamageType>>7)&0x01;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	WORD ShieldDamage	= (((WORD)(Data->ShieldDamageH)<<8) + Data->ShieldDamageL);
	
	//데미지를 가지고 표현 형태를 결정
	int accumDamage = ShieldDamage + Damage;
	int	rstDamage = -1;
	float rstScale = 0.8f;
	Vector(0.5f, 0.5f, 0.5f, Light);
	if(accumDamage > 0)
	{
		rstDamage = -2;
		if(accumDamage > 0 && accumDamage < 1000)
		{
			Vector(1.f,0.f,0.f,Light);
		}
		else if(accumDamage >= 1000 && accumDamage < 3000)
		{
			Vector(0.95f, 0.7f, 0.15f, Light);
		}
		else if(accumDamage >= 3000)
		{
			Vector(0.95f, 0.7f, 0.15f, Light);
			rstScale = 1.5f;
		}
	}
	else
	{
       	if(Key == HeroKey)
		{
			Vector(1.f,1.f,1.f,Light);
		}
		else
		{
			Vector(0.5f,0.5f,0.5f,Light);
		}
	}
	
	if(Success)
	{
		SetPlayerShock(c,Damage);
		Vector(1.f,0.f,0.f,Light);
		if(Key == HeroKey)
		{
			if(Damage >= CharacterAttribute->Life)
				CharacterAttribute->Life = 0;
			else
				CharacterAttribute->Life -= Damage;
			
			if(ShieldDamage >= CharacterAttribute->Shield)
				CharacterAttribute->Shield = 0;
			else
				CharacterAttribute->Shield -= ShieldDamage;
			
		}
		
		CreatePoint(o->Position,rstDamage,Light,rstScale);
	}
	else
	{
		if(Key == HeroKey)
		{
			if(Damage >= CharacterAttribute->Life)
				CharacterAttribute->Life = 0;
			else
				CharacterAttribute->Life -= Damage;
			
			if(ShieldDamage >= CharacterAttribute->Shield)
				CharacterAttribute->Shield = 0;
			else
				CharacterAttribute->Shield -= ShieldDamage;
			
			
			if( g_isCharacterBuff(o, eBuff_PhysDefense) && o->Type==MODEL_PLAYER )			
			{
				CHARACTER *cm = &CharactersClient[AttackPlayer];
				OBJECT *om = &cm->Object;
				float fAngle = CreateAngle( om->Position[0], om->Position[1], o->Position[0], o->Position[1]);
				if ( fabs( fAngle - om->Angle[2]) < 10.f)
				{
					vec3_t Angle = { 0.0f, 0.0f, fAngle+180.f};
					CreateEffect( MODEL_MAGIC_CAPSULE2, o->Position, Angle, o->Light, 0, o);
				}
			}
#ifdef _PVP_ADD_MOVE_SCROLL
			if (Damage > 0)
				g_MurdererMove.CancelMove();
#endif	// _PVP_ADD_MOVE_SCROLL
		}
		else
		{
			if (c->MonsterIndex == 275);	// 쿤둔의 경우 피격 처리 안함
			else if(rand()%2==0)
				SetPlayerShock(c,Damage);
		}
		
		if(DamageType == 4)
		{
			Vector(1.f,0.f,1.f,Light);
		}
		
		CreatePoint(o->Position,rstDamage,Light,rstScale);
	}
	c->Hit = Damage;
}

//누군가 얻어맞음
void ReceiveAttackDamage( BYTE *ReceiveBuffer )
{
	LPPRECEIVE_ATTACK Data = (LPPRECEIVE_ATTACK)ReceiveBuffer;

	//카오스 캐슬에서는 데미지 체계 다르게 처리
	if(InChaosCastle())
	{
		ProcessDamageCastle(Data);
		return;
	}
	
	int Key    = ((int)(Data->KeyH   )<<8) + Data->KeyL;
	int Success = (Key>>15);
	Key &= 0x7FFF;
	
	int Index = FindCharacterIndex(Key);
	CHARACTER *c = &CharactersClient[Index];
	OBJECT *o = &c->Object;
	vec3_t Light;
	WORD Damage			= (((WORD)(Data->DamageH)<<8) + Data->DamageL);
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	// DamageType 에서 0~7까지만 사용으로 0x20 0x10비트 사용
	int	 DamageType		= (Data->DamageType)&0x0f;
	bool bRepeatedly = (Data->DamageType>>4)&0x01;
	bool bEndRepeatedly = (Data->DamageType>>5)&0x01;
#else //PBG_ADD_NEWCHAR_MONK_SKILL
	int	 DamageType		= (Data->DamageType)&0x3f;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
    bool bDoubleEnable  = (Data->DamageType>>6)&0x01;
	bool bComboEnable	= (Data->DamageType>>7)&0x01;
	WORD ShieldDamage	= (((WORD)(Data->ShieldDamageH)<<8) + Data->ShieldDamageL);

#ifdef CSK_HACK_TEST
	g_pHackTest->ReceiveDamage(c, o, Success, Damage, DamageType, bDoubleEnable, bComboEnable, ShieldDamage, Key);
#endif // CSK_HACK_TEST
	
	if(Success)
	{
		SetPlayerShock(c,Damage);
		Vector(1.f,0.f,0.f,Light);
		
		CreatePoint(o->Position,Damage,Light);
		
		if(ShieldDamage > 0)
		{
			vec3_t nPosShieldDamage;
			Vector(0.8f,1.f,0.f,Light);
			nPosShieldDamage[0] = o->Position[0]; nPosShieldDamage[1] = o->Position[1];
			nPosShieldDamage[2] = o->Position[2] + 25.f;
			CreatePoint(nPosShieldDamage, ShieldDamage, Light);
		}
		
		if(Key == HeroKey)
		{
			if(Damage >= CharacterAttribute->Life)
				CharacterAttribute->Life = 0;
			else
				CharacterAttribute->Life -= Damage;
			
			if(ShieldDamage >= CharacterAttribute->Shield)
				CharacterAttribute->Shield = 0;
			else
				CharacterAttribute->Shield -= ShieldDamage;
		}
	}
	else
	{
		if(Key == HeroKey)
		{
			if(Damage >= CharacterAttribute->Life)
				CharacterAttribute->Life = 0;
			else
				CharacterAttribute->Life -= Damage;
			
			if(ShieldDamage >= CharacterAttribute->Shield)
				CharacterAttribute->Shield = 0;
			else
				CharacterAttribute->Shield -= ShieldDamage;
			
			// 방어막 넣을 부분
			if( g_isCharacterBuff( o, eBuff_PhysDefense ) && o->Type==MODEL_PLAYER )			
			{
				CHARACTER *cm = &CharactersClient[AttackPlayer];
				OBJECT *om = &cm->Object;
				float fAngle = CreateAngle( om->Position[0], om->Position[1], o->Position[0], o->Position[1]);
				if ( fabs( fAngle - om->Angle[2]) < 10.f)
				{
					vec3_t Angle = { 0.0f, 0.0f, fAngle+180.f};
					CreateEffect( MODEL_MAGIC_CAPSULE2, o->Position, Angle, o->Light, 0, o);
				}
			}
			
#ifdef _PVP_ADD_MOVE_SCROLL
			if (Damage > 0)
				g_MurdererMove.CancelMove();
#endif	// _PVP_ADD_MOVE_SCROLL
		}
		else
		{
			if (c->MonsterIndex == 275)	// 쿤둔의 경우 피격 처리 안함
			{
				// 아무처리 안함
			}
			else if(rand()%2==0)
			{
				SetPlayerShock(c,Damage);
			}
		}
        float scale = 15.f;
		if(Damage == 0)
		{
			if(Key == HeroKey)
			{
				Vector(1.f,1.f,1.f,Light);
			}
			else
			{
				Vector(0.5f,0.5f,0.5f,Light);
			}
		}
		else
		{
			switch ( DamageType )
			{
			case 0:	//	DT_NONE
				if(Key == HeroKey)
				{
					Vector(1.f,0.f,0.f,Light);
				}
				else
				{
					Vector(1.f,0.6f,0.f,Light);
				}
				break;
			case 1:	//	DT_PERFECT
				scale = 50.f;
				Vector(0.0f,1.f,1.f,Light);
				break;
			case 2:	//	DT_EXCELLENT
				scale = 50.f;
				Vector(0.f,1.f,0.6f,Light);
				break;
			case 3:	//	DT_CRITICAL
				scale = 50.f;
				Vector(0.f,0.6f,1.f,Light);
				break;
			case 4:	//	DT_MIRROR
				Vector(1.f,0.f,1.f,Light);
				break;
			case 5: //	DT_POSION
				Vector(0.f,1.f,0.f,Light);
				break;
			case 6:	//	DT_DOT
				Vector(0.7f,0.4f,1.0f,Light);
				break;
			default :
				Vector ( 1.f, 1.f, 1.f, Light );
				break;
			}
			
			if ( bComboEnable )
			{
				scale = 50.f;
			}
		}
		
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
		if(bRepeatedly || bEndRepeatedly)
		{
			g_CMonkSystem.SetRepeatedly(Damage, DamageType, bDoubleEnable, bEndRepeatedly);
			if(bEndRepeatedly)
			{
				g_CMonkSystem.RenderRepeatedly(Key, o);
			}
		}
		else
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
		if(Damage == 0)
			CreatePoint(o->Position,-1,Light);
		else
        {
			if ( bComboEnable )
			{
				vec3_t Position, Light2;
				VectorCopy ( o->Position, Position);
				Vector(Light[0]-0.4f,Light[1]-0.4f,Light[2]-0.4f,Light2);
				CreatePoint( Position,Damage,Light2, scale);
				Position[2] += 10.f;
				Vector(Light[0]-0.2f,Light[1]-0.2f,Light[2]-0.2f,Light2);
				CreatePoint( Position,Damage,Light2, scale+5.f);
				Position[2] += 10.f;
				CreatePoint( Position,Damage,Light, scale+10.f);
			}
			else if ( bDoubleEnable  )    //  Double Damage
            {
				vec3_t Position, Light2;
				VectorCopy ( o->Position, Position);
				Vector(Light[0]-0.4f,Light[1]-0.4f,Light[2]-0.4f,Light2);
				CreatePoint( Position,Damage,Light2, scale);
				Position[2] += 10.f;
				Vector(Light[0]-0.2f,Light[1]-0.2f,Light[2]-0.2f,Light2);
				CreatePoint( Position,Damage,Light2, scale+5.f);
            }
			
			CreatePoint(o->Position,Damage,Light, scale);
        }
		
		if(ShieldDamage > 0)
		{
			vec3_t nPosShieldDamage;
			Vector(0.8f,1.f,0.f,Light);
			nPosShieldDamage[0] = o->Position[0]; nPosShieldDamage[1] = o->Position[1];
			nPosShieldDamage[2] = o->Position[2] + 25.f;
			CreatePoint(nPosShieldDamage, ShieldDamage, Light);
		}
	}
	c->Hit = Damage;
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x15 [ReceiveAttackDamage(%d %d)]", AttackPlayer, Damage);
#endif // CONSOLE_DEBUG
}

void ReceiveAction(BYTE *ReceiveBuffer,int Size)
{
	LPPRECEIVE_ACTION Data = (LPPRECEIVE_ACTION)ReceiveBuffer;
	int Key = ((int)(Data->KeyH )<<8) + Data->KeyL;
	
	int Index = FindCharacterIndex(Key);
	CHARACTER *c = &CharactersClient[Index];
	OBJECT *o = &c->Object;
	
	int iTargetKey, iTargetIndex;
	
	iTargetKey = ((int)(Data->TargetKeyH )<<8) + Data->TargetKeyL;	
	iTargetIndex = FindCharacterIndex(iTargetKey);
	
    if ( c->Helper.Type==MODEL_HELPER+4 ) return;
	//if(c->Helper.Type == MODEL_HELPER+37) return;	//^ 펜릴 캐릭터 에니메이션 관련(펜릴 타고 있을때는 안 앉아지게)
	
	c->Object.Angle[2] = ((float)(Data->Angle)-1.f)*45.f;
	c->Movement = false;
	
	c->Object.Position[0] = c->TargetX*TERRAIN_SCALE+TERRAIN_SCALE*0.5f;
	c->Object.Position[1] = c->TargetY*TERRAIN_SCALE+TERRAIN_SCALE*0.5f;
	switch(Data->Action)
	{
	case AT_STAND1:
	case AT_STAND2:
		SetPlayerStop(c);
		break;
	case AT_ATTACK1:
	case AT_ATTACK2:
		SetPlayerAttack(c);
		c->AttackTime = 1;
		c->Object.AnimationFrame = 0;
		
		c->TargetCharacter = HeroIndex;
		
		AttackPlayer = Index;
		break;
	case AT_SKILL_BLOCKING:
		SetAction(o,PLAYER_DEFENSE1);
		PlayBuffer(SOUND_SKILL_DEFENSE);
		break;
	case AT_SIT1:
		if(!IsFemale(c->Class))
			SetAction(&c->Object,PLAYER_SIT1);
		else
			SetAction(&c->Object,PLAYER_SIT_FEMALE1);
		break;
	case AT_POSE1:
		if(!IsFemale(c->Class))
			SetAction(&c->Object,PLAYER_POSE1);
		else
			SetAction(&c->Object,PLAYER_POSE_FEMALE1);
		break;
	case AT_HEALING1:
		if(!IsFemale(c->Class))
			SetAction(&c->Object,PLAYER_HEALING1);
		else
			SetAction(&c->Object,PLAYER_HEALING_FEMALE1);
		break;
	case AT_GREETING1:
		if(!IsFemale(c->Class))
			SetAction(o,PLAYER_GREETING1);
		else
			SetAction(o,PLAYER_GREETING_FEMALE1);
		break;
	case AT_GOODBYE1:
		if(!IsFemale(c->Class))
			SetAction(o,PLAYER_GOODBYE1);
		else
			SetAction(o,PLAYER_GOODBYE_FEMALE1);
		break;
	case AT_CLAP1:
		if(!IsFemale(c->Class))
			SetAction(o,PLAYER_CLAP1);
		else
			SetAction(o,PLAYER_CLAP_FEMALE1);
		break;
	case AT_GESTURE1:
		if(!IsFemale(c->Class))
			SetAction(o,PLAYER_GESTURE1);
		else
			SetAction(o,PLAYER_GESTURE_FEMALE1);
		break;
	case AT_DIRECTION1:
		if(!IsFemale(c->Class))
			SetAction(o,PLAYER_DIRECTION1);
		else
			SetAction(o,PLAYER_DIRECTION_FEMALE1);
		break;
	case AT_UNKNOWN1:
		if(!IsFemale(c->Class))
			SetAction(o,PLAYER_UNKNOWN1);
		else
			SetAction(o,PLAYER_UNKNOWN_FEMALE1);
		break;
	case AT_CRY1:
		if(!IsFemale(c->Class))
			SetAction(o,PLAYER_CRY1);
		else
			SetAction(o,PLAYER_CRY_FEMALE1);
		break;
	case AT_AWKWARD1:
		if(!IsFemale(c->Class))
			SetAction(o,PLAYER_AWKWARD1);
		else
			SetAction(o,PLAYER_AWKWARD_FEMALE1);
		break;
	case AT_SEE1:
		if(!IsFemale(c->Class))
			SetAction(o,PLAYER_SEE1);
		else
			SetAction(o,PLAYER_SEE_FEMALE1);
		break;
	case AT_CHEER1:
		if(!IsFemale(c->Class))
			SetAction(o,PLAYER_CHEER1);
		else
			SetAction(o,PLAYER_CHEER_FEMALE1);
		break;
	case AT_WIN1:
		if(!IsFemale(c->Class))
			SetAction(o,PLAYER_WIN1);
		else
			SetAction(o,PLAYER_WIN_FEMALE1);
		break;
	case AT_SMILE1:
		if(!IsFemale(c->Class))
			SetAction(o,PLAYER_SMILE1);
		else
			SetAction(o,PLAYER_SMILE_FEMALE1);
		break;
	case AT_SLEEP1:
		if(!IsFemale(c->Class))
			SetAction(o,PLAYER_SLEEP1);
		else
			SetAction(o,PLAYER_SLEEP_FEMALE1);
		break;
	case AT_COLD1:
		if(!IsFemale(c->Class))
			SetAction(o,PLAYER_COLD1);
		else
			SetAction(o,PLAYER_COLD_FEMALE1);
		break;
	case AT_AGAIN1:
		if(!IsFemale(c->Class))
			SetAction(o,PLAYER_AGAIN1);
		else
			SetAction(o,PLAYER_AGAIN_FEMALE1);
		break;
	case AT_RESPECT1:
		SetAction(o,PLAYER_RESPECT1);
		break;
	case AT_SALUTE1:
		SetAction(o,PLAYER_SALUTE1);
		break;
	case AT_RUSH1:
		SetAction(o,PLAYER_RUSH1);
		break;
	case AT_SCISSORS:	// 가위
		SetAction(o,PLAYER_SCISSORS);
		break;
	case AT_ROCK:		// 바위
		SetAction(o,PLAYER_ROCK);
		break;
	case AT_PAPER:		// 보
		SetAction(o,PLAYER_PAPER);
		break;
	case AT_HUSTLE:	// 일본 레슬러 동작 -_-
		SetAction(o,PLAYER_HUSTLE);
		break;
	case AT_DAMAGE1:	// 충격혹은 피격동작 (쿤둔포효등)
		SetAction(&c->Object,MONSTER01_SHOCK);
		break;
	case AT_PROVOCATION:	// 덤벼 동작 추가
		SetAction(o,PLAYER_PROVOCATION);
		break;
	case AT_LOOK_AROUND:	// 두리번 동작 추가
		SetAction(o,PLAYER_LOOK_AROUND);
		break;
	case AT_CHEERS:			// 만세 동작 추가
		SetAction(o,PLAYER_CHEERS);
		break;
	case AT_JACK1:
		SetAction(o, PLAYER_JACK_1);
		o->m_iAnimation = 0;
		break;
	case AT_JACK2:
		SetAction(o, PLAYER_JACK_2);
		o->m_iAnimation = 0;
		break;
	case AT_SANTA1_1:
	case AT_SANTA1_2:
	case AT_SANTA1_3:
		{
			SetAction(o, PLAYER_SANTA_1);
			int i = Data->Action % AT_SANTA1_1;
			g_XmasEvent->CreateXmasEventEffect(c, o, i);
			o->m_iAnimation = 0;
			PlayBuffer(SOUND_XMAS_JUMP_SANTA + i);
		}
		break;
	case AT_SANTA2_1:
	case AT_SANTA2_2:
	case AT_SANTA2_3:
		{
			SetAction(o, PLAYER_SANTA_2);
			int i = Data->Action % AT_SANTA2_1;
			g_XmasEvent->CreateXmasEventEffect(c, o, i);
			o->m_iAnimation = 0;
			PlayBuffer(SOUND_XMAS_TURN);
		}
		break;
		
#ifdef WORLDCUP_ADD
#ifndef NO_MORE_DANCING
	case AT_HANDCLAP:
		{
			int Rnd = (int)(rand()%6);
			SetAction(o,PLAYER_KOREA_HANDCLAP);
			PlayBuffer(SOUND_WORLDCUP_RND1 + Rnd);
		}
		break;
	case AT_POINTDANCE:
		SetAction(o,PLAYER_POINT_DANCE);
		break;
#endif
#endif
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	case AT_RAGEBUFF_1:
		{
			SetAction(o, PLAYER_SKILL_ATT_UP_OURFORCES);
			PlayBuffer(SOUND_RAGESKILL_BUFF_1);
		}
		break;
	case AT_RAGEBUFF_2:
		{
			SetAction(o, PLAYER_SKILL_HP_UP_OURFORCES);
			PlayBuffer(SOUND_RAGESKILL_BUFF_2);
		}
		break;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
	default:
		SetAction(&c->Object,Data->Action);
		break;
	}

#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x18 [ReceiveAction(%d)]", Data->Angle);
#endif // CONSOLE_DEBUG
}

///////////////////////////////////////////////////////////////////////////////
// 마법
///////////////////////////////////////////////////////////////////////////////

// 스킬을 사용 했을때 생성 되어야 할 버프
void ReceiveSkillStatus( BYTE *ReceiveBuffer )
{
	LPPMSG_VIEWSKILLSTATE Data = (LPPMSG_VIEWSKILLSTATE)ReceiveBuffer;
	int Key = ((int)(Data->KeyH )<<8) + Data->KeyL;
    CHARACTER* c = &CharactersClient[FindCharacterIndex(Key)];
	OBJECT *o = &c->Object;
	
	if ( Data->State == 1) // add
	{
		eBuffState bufftype = static_cast<eBuffState>(Data->BuffIndex);
		
		if( bufftype == eBuffNone || bufftype >= eBuff_Count ) return;
		
		if( g_isCharacterBuff( o, bufftype ) )
		{
			// 이건 먼지를 모르겠다.
			if((o->Type >= MODEL_CRYWOLF_ALTAR1 && o->Type <= MODEL_CRYWOLF_ALTAR5)
				|| MODEL_SMITH || MODEL_NPC_SERBIS || MODEL_MERCHANT_MAN
				|| MODEL_STORAGE || MODEL_NPC_BREEDER )
			{
				if( g_isCharacterBuff( o, eBuff_CrywolfHeroContracted ) )
				{
					g_TokenCharacterBuff( o, eBuff_CrywolfHeroContracted );
				}
				else
				{
					g_TokenCharacterBuff( o, eBuff_CrywolfHeroContracted );
					g_CharacterUnRegisterBuff( o, eBuff_CrywolfHeroContracted );
				}
			}
		}
		else
		{
			
			RegisterBuff( bufftype, o );
			if( bufftype == eBuff_CastleRegimentDefense || bufftype == eBuff_CastleRegimentAttack1
				|| bufftype == eBuff_CastleRegimentAttack2 || bufftype == eBuff_CastleRegimentAttack3 )
			{
				battleCastle::SettingBattleFormation ( c, bufftype );
			}
			else if( bufftype == eBuff_GMEffect )
			{
				if ( c->m_pParts != NULL )
				{
					DeleteParts ( c );
				}
				c->EtcPart = PARTS_WEBZEN;
			}
#ifdef CONSOLE_DEBUG
			g_ConsoleDebug->Write(MCD_RECEIVE, "RegisterBuff ID : %s, Buff : %d", c->ID, static_cast<int>(bufftype));
#endif // CONSOLE_DEBUG
		}
	}
	else // clear
	{
		eBuffState bufftype = static_cast<eBuffState>(Data->BuffIndex);
		
		if( bufftype == eBuffNone || bufftype >= eBuff_Count ) return;
		
		UnRegisterBuff( bufftype, o );
		if( bufftype == eBuff_CastleRegimentDefense || bufftype == eBuff_CastleRegimentAttack1
			|| bufftype == eBuff_CastleRegimentAttack2 || bufftype == eBuff_CastleRegimentAttack3 )
		{
			battleCastle::DeleteBattleFormation( c, bufftype );
#ifdef CONSOLE_DEBUG
			g_ConsoleDebug->Write(MCD_RECEIVE, "UnRegisterBuff ID : %s, Buff : %d", c->ID, static_cast<int>(bufftype));
#endif // CONSOLE_DEBUG	
		}
		else if( bufftype == eBuff_GMEffect )
		{
			DeleteParts ( c );
		}
	}
	
}

// 스킬을 종료 했을 경우 지워야 할 버프
void ReceiveMagicFinish( BYTE *ReceiveBuffer )
{
	LPPHEADER_DEFAULT_VALUE_KEY Data = (LPPHEADER_DEFAULT_VALUE_KEY)ReceiveBuffer;
	int Key = ((int)(Data->KeyH )<<8) + Data->KeyL;
	OBJECT *o = &CharactersClient[FindCharacterIndex(Key)].Object;
	
	switch(Data->Value)
	{
	case AT_SKILL_POISON:
		UnRegisterBuff( eDeBuff_Poison, o );
		break;
	case AT_SKILL_SLOW:
		UnRegisterBuff( eDeBuff_Freeze, o);
		break;
#ifdef CSK_ADD_SKILL_BLOWOFDESTRUCTION
	case AT_SKILL_BLOW_OF_DESTRUCTION:
		UnRegisterBuff( eDeBuff_BlowOfDestruction, o);
		break;
#endif // CSK_ADD_SKILL_BLOWOFDESTRUCTION

#ifdef PJH_SEASON4_MASTER_RANK4
	case AT_SKILL_ATT_POWER_UP:
	case AT_SKILL_ATT_POWER_UP+1:
	case AT_SKILL_ATT_POWER_UP+2:
	case AT_SKILL_ATT_POWER_UP+3:
	case AT_SKILL_ATT_POWER_UP+4:
#endif //PJH_SEASON4_MASTER_RANK4
	case AT_SKILL_ATTACK:
		UnRegisterBuff( eBuff_Attack, o);
		break;
		//#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
		//	case AT_SKILL_RECOVER:
		//#endif //PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
		
#ifdef PJH_SEASON4_MASTER_RANK4
	case AT_SKILL_DEF_POWER_UP:
	case AT_SKILL_DEF_POWER_UP+1:
	case AT_SKILL_DEF_POWER_UP+2:
	case AT_SKILL_DEF_POWER_UP+3:
	case AT_SKILL_DEF_POWER_UP+4:
#endif //PJH_SEASON4_MASTER_RANK4
	case AT_SKILL_DEFENSE:
		UnRegisterBuff( eBuff_Defense, o);
		break;
	case AT_SKILL_STUN:
		UnRegisterBuff( eDeBuff_Stun, o);
		break;
    case AT_SKILL_INVISIBLE:
		UnRegisterBuff( eBuff_Cloaking, o);
        break;
    case AT_SKILL_MANA:
        UnRegisterBuff( eBuff_AddMana, o );
        break;
    case AT_SKILL_BRAND_OF_SKILL: 
		UnRegisterBuff( eBuff_AddSkill, o );
        break;
    case AT_SKILL_IMPROVE_AG:
		UnRegisterBuff( eBuff_AddAG, o);
        break;
    case AT_SKILL_ADD_CRITICAL:
        UnRegisterBuff( eBuff_AddCriticalDamage, o );
        break;
	case AT_SKILL_LIFE_UP:
	case AT_SKILL_LIFE_UP+1:
	case AT_SKILL_LIFE_UP+2:
	case AT_SKILL_LIFE_UP+3:
	case AT_SKILL_LIFE_UP+4:
    case AT_SKILL_VITALITY:
		UnRegisterBuff( eBuff_HpRecovery, o); //eBuff_Life
        break;
    case AT_SKILL_PARALYZE:
		UnRegisterBuff( eDeBuff_Harden, o);
        break;
#ifdef PJH_SEASON4_MASTER_RANK4
	case AT_SKILL_BLOOD_ATT_UP:
	case AT_SKILL_BLOOD_ATT_UP+1:
	case AT_SKILL_BLOOD_ATT_UP+2:
	case AT_SKILL_BLOOD_ATT_UP+3:
	case AT_SKILL_BLOOD_ATT_UP+4:
#endif //PJH_SEASON4_MASTER_RANK4
	case AT_SKILL_REDUCEDEFENSE:
		UnRegisterBuff( eDeBuff_Defense, o);
		break;
	case AT_SKILL_SOUL_UP:
	case AT_SKILL_SOUL_UP+1:
	case AT_SKILL_SOUL_UP+2:
	case AT_SKILL_SOUL_UP+3:
	case AT_SKILL_SOUL_UP+4:
	case AT_SKILL_WIZARDDEFENSE:
		UnRegisterBuff( eBuff_PhysDefense, o);
		break;
	case AT_SKILL_BLAST_POISON:
		UnRegisterBuff( eDeBuff_Poison, o);
		break;
//AT_SKILL_ICE_UP
	case AT_SKILL_ICE_UP:
	case AT_SKILL_ICE_UP+1:
	case AT_SKILL_ICE_UP+2:
	case AT_SKILL_ICE_UP+3:
	case AT_SKILL_ICE_UP+4:
	case AT_SKILL_BLAST_FREEZE:
		UnRegisterBuff( eDeBuff_Freeze, o);
		break;
        //  몬스터.
    case AT_SKILL_MONSTER_MAGIC_DEF:
        SetActionDestroy_Def ( o );
		UnRegisterBuff( eBuff_Defense, o);
        break;
    case AT_SKILL_MONSTER_PHY_DEF:
        SetActionDestroy_Def ( o );
		UnRegisterBuff( eBuff_PhysDefense, o);
        break;
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	case AT_SKILL_ATT_UP_OURFORCES:
		UnRegisterBuff( eBuff_Att_up_Ourforces, o);
		break;
	case AT_SKILL_HP_UP_OURFORCES:
		UnRegisterBuff( eBuff_Hp_up_Ourforces, o);
		break;
	case AT_SKILL_DEF_UP_OURFORCES:
		UnRegisterBuff( eBuff_Def_up_Ourforces, o);
		break;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
	}
}

// 요정 플레이어의 애니메이션 설정 (요정만 사용)
void SetPlayerBow(CHARACTER *c)
{
#ifdef KJH_FIX_BOW_ANIMATION_ON_RIDE_PET
	OBJECT *o = &c->Object;

	// 플레이어, 요정, 안전지대가 아니면 return
	if(o->Type!=MODEL_PLAYER || GetBaseClass(c->Class) != CLASS_ELF || c->SafeZone)
		return;
	
	// 공속계산 셋팅
	SetAttackSpeed();
	
	switch( GetEquipedBowType( c ))
	{
	case BOWTYPE_BOW:		// 활
		{
			if( c->Helper.Type == MODEL_HELPER+37 )				
			{
				SetAction( &c->Object, PLAYER_FENRIR_ATTACK_BOW );		// 팬릴착용
			}
			else if((c->Helper.Type==MODEL_HELPER+2) || (c->Helper.Type==MODEL_HELPER+3))
			{
				SetAction( &c->Object, PLAYER_ATTACK_RIDE_BOW );		// 탈것착용
			}
			else if( c->Wing.Type!=-1 )										
			{
				SetAction( &c->Object, PLAYER_ATTACK_FLY_BOW );			// 날개착용
			}
			else
			{
				SetAction( &c->Object, PLAYER_ATTACK_BOW );				// 일반활공격
			}
		}break;
	case BOWTYPE_CROSSBOW:	// 석궁
		{
			if(c->Helper.Type == MODEL_HELPER+37 && !c->SafeZone)	// 팬릴착용
			{
				SetAction( &c->Object, PLAYER_FENRIR_ATTACK_CROSSBOW );
			}
			else if(( c->Helper.Type==MODEL_HELPER+2) || (c->Helper.Type==MODEL_HELPER+3))
			{
				SetAction( &c->Object, PLAYER_ATTACK_RIDE_CROSSBOW );	// 탈것착용
			}
			else if( c->Wing.Type!=-1 )										// 날개착용
			{	
				SetAction( &c->Object, PLAYER_ATTACK_FLY_CROSSBOW );
			}
			else
			{
				SetAction( &c->Object, PLAYER_ATTACK_CROSSBOW );		// 일반활공격
			}
		}break;
	}
#else // KJH_FIX_BOW_ANIMATION_ON_RIDE_PET		// 정리할 때 지워야 하는 소스
#ifdef ADD_SOCKET_ITEM
	// 활
	switch( GetEquipedBowType( c ))
	{
	case BOWTYPE_BOW:
		{
			if( (GetBaseClass(c->Class) == CLASS_ELF) && (c->Wing.Type!=-1) )
			{
				SetAction( &c->Object, PLAYER_ATTACK_FLY_BOW );
			}
			else
			{
				SetAction( &c->Object, PLAYER_ATTACK_BOW );
			}
		}break;
		// 석궁
	case BOWTYPE_CROSSBOW:
		{
			if( (GetBaseClass(c->Class) == CLASS_ELF) && (c->Wing.Type!=-1) )
			{
				SetAction( &c->Object, PLAYER_ATTACK_FLY_CROSSBOW );
			}
			else
			{
				SetAction( &c->Object, PLAYER_ATTACK_CROSSBOW );
			}
		}break;
	}
#else // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
    if((c->Weapon[1].Type>=MODEL_BOW && c->Weapon[1].Type<MODEL_BOW+7) 
		|| c->Weapon[1].Type==MODEL_BOW+17 
        || c->Weapon[1].Type==MODEL_BOW+20 
		|| c->Weapon[1].Type==MODEL_BOW+21
		|| c->Weapon[1].Type==MODEL_BOW+22
        )	//. 활
    {
        if(GetBaseClass(c->Class)==CLASS_ELF && c->Wing.Type!=-1)
            SetAction(&c->Object,PLAYER_ATTACK_FLY_BOW);
        else
            SetAction(&c->Object,PLAYER_ATTACK_BOW);
    }
    else if((c->Weapon[0].Type>=MODEL_BOW+8 && c->Weapon[0].Type<MODEL_BOW+15) || c->Weapon[0].Type==MODEL_BOW+16 || 
        (c->Weapon[0].Type>=MODEL_BOW+18 && c->Weapon[0].Type<MODEL_BOW+MAX_ITEM_INDEX) )	//. 석궁
    {
        if(GetBaseClass(c->Class)==CLASS_ELF && c->Wing.Type!=-1)
            SetAction(&c->Object,PLAYER_ATTACK_FLY_CROSSBOW);
        else
            SetAction(&c->Object,PLAYER_ATTACK_CROSSBOW);
    }
#endif // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
#endif // KJH_FIX_BOW_ANIMATION_ON_RIDE_PET	// 정리할 때 지워야 하는 소스
}

void SetPlayerHighBow ( CHARACTER* c )
{
#ifdef ADD_SOCKET_ITEM
#ifdef KJH_FIX_BOW_ANIMATION_ON_RIDE_PET
		switch( GetEquipedBowType( c ))
	{
	case BOWTYPE_BOW:		// 활
		{
			if( c->Helper.Type == MODEL_HELPER+37 )				
			{
				SetAction( &c->Object, PLAYER_ATTACK_RIDE_BOW_UP );		// 팬릴착용
			}
			else if((c->Helper.Type==MODEL_HELPER+2) || (c->Helper.Type==MODEL_HELPER+3))
			{
				SetAction( &c->Object, PLAYER_ATTACK_RIDE_BOW_UP );		// 탈것착용
			}
			else if( c->Wing.Type!=-1 )										
			{
				SetAction( &c->Object, PLAYER_ATTACK_FLY_BOW_UP );			// 날개착용
			}
			else
			{
				SetAction( &c->Object, PLAYER_ATTACK_BOW_UP );				// 일반활공격
			}
		}break;
	case BOWTYPE_CROSSBOW:	// 석궁
		{
			if(c->Helper.Type == MODEL_HELPER+37 && !c->SafeZone)	// 팬릴착용
			{
				SetAction( &c->Object, PLAYER_ATTACK_RIDE_CROSSBOW_UP );
			}
			else if(( c->Helper.Type==MODEL_HELPER+2) || (c->Helper.Type==MODEL_HELPER+3))
			{
				SetAction( &c->Object, PLAYER_ATTACK_RIDE_CROSSBOW_UP );	// 탈것착용
			}
			else if( c->Wing.Type!=-1 )										// 날개착용
			{	
				SetAction( &c->Object, PLAYER_ATTACK_FLY_CROSSBOW_UP );
			}
			else
			{
				SetAction( &c->Object, PLAYER_ATTACK_CROSSBOW_UP );		// 일반활공격
			}
		}break;
	}
#else KJH_FIX_BOW_ANIMATION_ON_RIDE_PET
	// 활
	switch( GetEquipedBowType( c ) )
	{
	case BOWTYPE_BOW:
		{
			if( (GetBaseClass(c->Class) == CLASS_ELF) && (c->Wing.Type!=-1) )
			{
				SetAction( &c->Object, PLAYER_ATTACK_FLY_BOW_UP );
			}
			else
			{
				SetAction( &c->Object, PLAYER_ATTACK_BOW_UP );
			}
		}break;
		// 석궁
	case BOWTYPE_CROSSBOW:
		{
			if( (GetBaseClass(c->Class) == CLASS_ELF) && (c->Wing.Type!=-1) )
			{
				SetAction( &c->Object, PLAYER_ATTACK_FLY_CROSSBOW_UP );
			}
			else
			{
				SetAction( &c->Object, PLAYER_ATTACK_CROSSBOW_UP );
			}
		}break;
	}
#endif // KJH_FIX_BOW_ANIMATION_ON_RIDE_PET
#else // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
    if((c->Weapon[1].Type>=MODEL_BOW && c->Weapon[1].Type<MODEL_BOW+7) || c->Weapon[1].Type==MODEL_BOW+17 
        || c->Weapon[1].Type==MODEL_BOW+20 
		|| c->Weapon[1].Type==MODEL_BOW+21
		|| c->Weapon[1].Type==MODEL_BOW+22
        )	//. 활
    {
        if(GetBaseClass(c->Class)==CLASS_ELF && c->Wing.Type!=-1)
            SetAction(&c->Object,PLAYER_ATTACK_FLY_BOW_UP);
        else
            SetAction(&c->Object,PLAYER_ATTACK_BOW_UP);
    }
    else if((c->Weapon[0].Type>=MODEL_BOW+8 && c->Weapon[0].Type<MODEL_BOW+15) || c->Weapon[0].Type==MODEL_BOW+16 || 
        (c->Weapon[0].Type>=MODEL_BOW+18 && c->Weapon[0].Type<MODEL_BOW+MAX_ITEM_INDEX) )	//. 석궁
    {
        if(GetBaseClass(c->Class)==CLASS_ELF && c->Wing.Type!=-1)
            SetAction(&c->Object,PLAYER_ATTACK_FLY_CROSSBOW_UP);
        else
            SetAction(&c->Object,PLAYER_ATTACK_CROSSBOW_UP);
    }
#endif // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
}

BOOL ReceiveMonsterSkill(BYTE *ReceiveBuffer,int Size, BOOL bEncrypted)
{
	LPPRECEIVE_MONSTERSKILL Data = (LPPRECEIVE_MONSTERSKILL)ReceiveBuffer;
	
	int SourceKey = Data->SourceKey;
	int TargetKey = Data->TargetKey;
	int Success = (TargetKey>>15);
	
	WORD SkillNumber = ((WORD)(Data->MagicH )<<8) + Data->MagicL;
	
	int Index       = FindCharacterIndex(SourceKey);
	int TargetIndex = FindCharacterIndex(TargetKey);
	if(TargetIndex == MAX_CHARACTERS_CLIENT)
		return ( TRUE);
	AttackPlayer = Index;
	
	CHARACTER *sc = &CharactersClient[Index];
	CHARACTER *tc = &CharactersClient[TargetIndex];
	OBJECT *so = &sc->Object;
	OBJECT *to = &tc->Object;
	

	sc->MonsterSkill = SkillNumber;

	sc->SkillSuccess = (Success != 0);
	sc->TargetCharacter = TargetIndex;
	
	if(SkillNumber > ATMON_SKILL_BIGIN && SkillNumber < ATMON_SKILL_END)
	{
		sc->Object.Angle[2] = CreateAngle(so->Position[0],so->Position[1],to->Position[0],to->Position[1]);
		SetPlayerAttack(sc);
		so->AnimationFrame = 0;
		sc->AttackTime = 1;
	}
		
#ifdef CONSOLE_DEBUG
		g_ConsoleDebug->Write(MCD_RECEIVE, "0x69 [ReceiveMonsterSkill(Skill : %d | SKey : %d |TKey : %d)]", SkillNumber, SourceKey, TargetKey);
#endif // CONSOLE_DEBUG
		
		return ( TRUE);
}

BOOL ReceiveMagic(BYTE *ReceiveBuffer,int Size, BOOL bEncrypted)
{
	if ( !bEncrypted)
	{
		GO_DEBUG;
		SendHackingChecked( 0x00, 0x19);
		return ( FALSE);
	}
	
	LPPRECEIVE_MAGIC Data = (LPPRECEIVE_MAGIC)ReceiveBuffer;
	int SourceKey = ((int)(Data->SourceKeyH )<<8) + Data->SourceKeyL;
	int TargetKey = ((int)(Data->TargetKeyH )<<8) + Data->TargetKeyL;
	int Success = (TargetKey>>15);
	
	WORD MagicNumber = ((WORD)(Data->MagicH )<<8) + Data->MagicL;
	
	if(MagicNumber == AT_SKILL_ATTACK || MagicNumber == AT_SKILL_DEFENSE
#ifdef PJH_SEASON4_MASTER_RANK4
		|| (AT_SKILL_DEF_POWER_UP <= MagicNumber && MagicNumber <= AT_SKILL_DEF_POWER_UP+4)
		|| (AT_SKILL_ATT_POWER_UP <= MagicNumber && MagicNumber <= AT_SKILL_ATT_POWER_UP+4)
#endif //PJH_SEASON4_MASTER_RANK4
		)
		{
			if(Success == false)
			{
				g_pChatListBox->AddText("", GlobalText[2249], SEASON3B::TYPE_SYSTEM_MESSAGE);
				return FALSE;
			}
		}
		
		TargetKey &= 0x7FFF;
		
		int Index       = FindCharacterIndex(SourceKey);
		int TargetIndex = FindCharacterIndex(TargetKey);
		if(TargetIndex == MAX_CHARACTERS_CLIENT)
			return ( TRUE);
		
		AttackPlayer = Index;
		CHARACTER *sc = &CharactersClient[Index];
		CHARACTER *tc = &CharactersClient[TargetIndex];
		OBJECT *so = &sc->Object;
		OBJECT *to = &tc->Object;

		if ( MagicNumber!=AT_SKILL_COMBO )
		{
			if(sc!=Hero && MagicNumber!=AT_SKILL_TELEPORT && MagicNumber!=AT_SKILL_TELEPORT_B && to->Visible)
					so->Angle[2] = CreateAngle(so->Position[0],so->Position[1],to->Position[0],to->Position[1]);
				sc->TargetCharacter = TargetIndex;
				
				// 펜릴 스킬 플라즈마 사용하면 타켓인덱스를 따로 저장한다.
			if(MagicNumber == AT_SKILL_PLASMA_STORM_FENRIR)
				sc->m_iFenrirSkillTarget = TargetIndex;
					
			sc->SkillSuccess = (Success != 0);
			sc->Skill = MagicNumber;
		}
		switch(MagicNumber)
		{
			case AT_SKILL_MONSTER_SUMMON:
				SetPlayerAttack ( sc );
				break;
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
			case AT_SKILL_RECOVER:
				{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
					OBJECT* oh = &Hero->Object;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
					OBJECT* o = so;
					vec3_t Light,Position,P,dp;
					vec3_t vFirePosition;
					
					float Matrix[3][4];
					
					if(sc != Hero)
					{
						Vector(0.f,-220.f,130.f,P);
						AngleMatrix(o->Angle,Matrix);
						VectorRotate(P,Matrix,dp);
						VectorAdd(dp,o->Position,Position);
						Vector(0.7f, 0.6f, 0.f, Light);
						CreateEffect(BITMAP_IMPACT, Position, o->Angle, Light, 0,o);
						SetAction(o,PLAYER_RECOVER_SKILL);
						sc->AttackTime = 1;
					}
					
					o = to;
					
					Vector(0.4f, 0.6f, 1.f, Light);
					Vector(0.f,0.f,0.f,P);
					AngleMatrix(o->Angle,Matrix);
					VectorRotate(P,Matrix,dp);
					VectorAdd(dp,o->Position,Position);
					Position[2] += 130;
					VectorCopy( o->Position, Position);
					for ( int i=0; i<19; ++i )
					{
						CreateJoint(BITMAP_FLARE,Position,Position,o->Angle,47,o,40,2);
					}
					Vector ( 0.3f, 0.2f, 0.1f, Light );
					CreateEffect(MODEL_SUMMON,Position,o->Angle,Light, 0);
					CreateEffect ( BITMAP_TWLIGHT, Position, o->Angle, Light, 0 );
					CreateEffect ( BITMAP_TWLIGHT, Position, o->Angle, Light, 1 );
					CreateEffect ( BITMAP_TWLIGHT, Position, o->Angle, Light, 2 );
					
					for (int i = 0; i < 2; ++i)
					{
						int iNumBones = Models[o->Type].NumBones;
						Models[o->Type].TransformByObjectBone(vFirePosition, o, rand()%iNumBones);	
						CreateEffect(BITMAP_FLARE, vFirePosition, o->Angle, Light,3);
					}
				}
				
				break;
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
			case AT_SKILL_MONSTER_MAGIC_DEF:
				sc->AttackTime = 1;
				g_CharacterRegisterBuff(so, eBuff_PhysDefense);
				SetPlayerAttack ( sc );
				break;
				
			case AT_SKILL_MONSTER_PHY_DEF:
				sc->AttackTime = 1;
				g_CharacterRegisterBuff(so, eBuff_Defense);
				SetPlayerAttack ( sc );
				break;
			case AT_SKILL_HEAL_UP:
			case AT_SKILL_HEAL_UP+1:
			case AT_SKILL_HEAL_UP+2:
			case AT_SKILL_HEAL_UP+3:
			case AT_SKILL_HEAL_UP+4:
			case AT_SKILL_HEALING:
#ifdef PJH_SEASON4_MASTER_RANK4
			case AT_SKILL_ATT_POWER_UP:
			case AT_SKILL_ATT_POWER_UP+1:
			case AT_SKILL_ATT_POWER_UP+2:
			case AT_SKILL_ATT_POWER_UP+3:
			case AT_SKILL_ATT_POWER_UP+4:
#endif //PJH_SEASON4_MASTER_RANK4
			case AT_SKILL_ATTACK:
#ifdef PJH_SEASON4_MASTER_RANK4
			case AT_SKILL_DEF_POWER_UP:
			case AT_SKILL_DEF_POWER_UP+1:
			case AT_SKILL_DEF_POWER_UP+2:
			case AT_SKILL_DEF_POWER_UP+3:
			case AT_SKILL_DEF_POWER_UP+4:
#endif //PJH_SEASON4_MASTER_RANK4
			case AT_SKILL_DEFENSE:
			case AT_SKILL_SUMMON:
			case AT_SKILL_SUMMON+1:
			case AT_SKILL_SUMMON+2:
			case AT_SKILL_SUMMON+3:
			case AT_SKILL_SUMMON+4:
			case AT_SKILL_SUMMON+5:
			case AT_SKILL_SUMMON+6:
#ifdef ADD_ELF_SUMMON
			case AT_SKILL_SUMMON+7:
#endif // ADD_ELF_SUMMON
				if ( sc->MonsterIndex != 77)
				{
					PlayBuffer(SOUND_SKILL_DEFENSE);
				}
			case AT_SKILL_SOUL_UP:
			case AT_SKILL_SOUL_UP+1:
			case AT_SKILL_SOUL_UP+2:
			case AT_SKILL_SOUL_UP+3:
			case AT_SKILL_SOUL_UP+4:
			case AT_SKILL_WIZARDDEFENSE:
				if(sc != Hero)
				{
					SetPlayerMagic(sc);
					so->AnimationFrame = 0;
				}
				sc->AttackTime = 1;
				break;
			case AT_SKILL_THUNDER:
				if(so->Type != MODEL_MONSTER01+83)
					PlayBuffer(SOUND_THUNDER01);
				if(to->CurrentAction==PLAYER_POSE1 || to->CurrentAction==PLAYER_POSE_FEMALE1 ||
					to->CurrentAction==PLAYER_SIT1 || to->CurrentAction==PLAYER_SIT_FEMALE1)
					SetPlayerStop(tc);
			case AT_SKILL_FIREBALL:
			case AT_SKILL_METEO:
			case AT_SKILL_SLOW:
			case AT_SKILL_ENERGYBALL:
			case AT_SKILL_POWERWAVE:
			case AT_SKILL_POISON:
			case AT_SKILL_FLAME:
				if(sc != Hero)
				{
					if(so->Type == MODEL_PLAYER)
					{
						SetPlayerMagic(sc);
						so->AnimationFrame = 0;
					}
					else
					{
						SetPlayerAttack(sc);
						so->AnimationFrame = 0;
					}
				}
				sc->AttackTime = 1;
				break;
				
			case AT_SKILL_TELEPORT_B :
				CreateTeleportBegin(to);
				
				CreateTeleportEnd(so);
				if ( sc==Hero )
					Teleport = false;
				
				PlayBuffer(SOUND_TELEKINESIS,so);
				break;
				
			case AT_SKILL_TELEPORT:
				// 칸투르3차 나이트메어전에서 나이트메어 순간이동 할 때 효과
				if(World == WD_39KANTURU_3RD && so->Type == MODEL_MONSTER01+121)
				{
					vec3_t Light;
					Vector ( 1.0f, 1.0f, 1.0f, Light );
					so->AlphaTarget = 0.f;
					CreateEffect(BITMAP_SPARK+1,so->Position,so->Angle,Light);
					CreateEffect(BITMAP_SPARK+1,so->Position,so->Angle,Light);
					PlayBuffer(SOUND_KANTURU_3RD_NIGHTMARE_TELE);
				}
				else
					CreateTeleportBegin(so);
				break;
				
			case AT_SKILL_STRONG_PIER:      //  스트롱피어.
				if ( (sc->Helper.Type>=MODEL_HELPER+2 && sc->Helper.Type<=MODEL_HELPER+4) && !sc->SafeZone )
				{
					SetAction ( so, PLAYER_ATTACK_RIDE_STRIKE );
				}
				else if(sc->Helper.Type == MODEL_HELPER+37 && !sc->SafeZone)
				{
					SetAction(so, PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE);	// 펜릴 타고 있으면 펜릴에 맞는 액션으로
				}
				else
				{
					SetAction ( so, PLAYER_ATTACK_STRIKE );
				}
				sc->AttackTime = 1;
				PlayBuffer(SOUND_SKILL_SWORD1);
				break;
			case AT_SKILL_FIRE_BUST_UP:
			case AT_SKILL_FIRE_BUST_UP+1:
			case AT_SKILL_FIRE_BUST_UP+2:
			case AT_SKILL_FIRE_BUST_UP+3:
			case AT_SKILL_FIRE_BUST_UP+4:
			case AT_SKILL_LONGPIER_ATTACK:  //  파이어버스트
				if ( (sc->Helper.Type>=MODEL_HELPER+2 && sc->Helper.Type<=MODEL_HELPER+4) && !sc->SafeZone )
				{
					SetAction ( so, PLAYER_ATTACK_RIDE_STRIKE );
				}
				else if(sc->Helper.Type == MODEL_HELPER+37 && !sc->SafeZone)
				{
					SetAction(so, PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE);	// 펜릴 타고 있으면 펜릴에 맞는 액션으로
				}
				else
				{
					SetAction ( so, PLAYER_ATTACK_STRIKE );
				}
				sc->AttackTime = 1;
				PlayBuffer ( SOUND_ATTACK_FIRE_BUST );
				break;
				
			case AT_SKILL_SWORD1:
				SetAction(so,PLAYER_ATTACK_SKILL_SWORD1+MagicNumber-AT_SKILL_SWORD1);
				sc->AttackTime = 1;
				PlayBuffer(SOUND_SKILL_SWORD1);
				break;
				
			case AT_SKILL_SWORD2:
				SetAction(so,PLAYER_ATTACK_SKILL_SWORD1+MagicNumber-AT_SKILL_SWORD1);
				sc->AttackTime = 1;
				PlayBuffer(SOUND_SKILL_SWORD2);
				break;
				
			case AT_SKILL_SWORD3:
				SetAction(so,PLAYER_ATTACK_SKILL_SWORD1+MagicNumber-AT_SKILL_SWORD1);
				sc->AttackTime = 1;
				PlayBuffer(SOUND_SKILL_SWORD3);
				break;
				
			case AT_SKILL_SWORD4:
				SetAction(so,PLAYER_ATTACK_SKILL_SWORD1+MagicNumber-AT_SKILL_SWORD1);
				sc->AttackTime = 1;
				PlayBuffer(SOUND_SKILL_SWORD4);
				break;
				
			case AT_SKILL_SWORD5://베기
				if(sc->SwordCount%2==0)
				{
					SetAction(so,PLAYER_ATTACK_SKILL_SWORD1+MagicNumber-AT_SKILL_SWORD1);
				}
				else
				{
					SetAction(so,PLAYER_ATTACK_TWO_HAND_SWORD1+2);
					//SetAction(so,PLAYER_ATTACK_TWO_HAND_SWORD1+sc->SwordCount%3);
				}
				sc->SwordCount++;
				sc->AttackTime = 1;
				PlayBuffer(SOUND_SKILL_SWORD4);
				break;
#ifdef PJH_SEASON4_MASTER_RANK4
			case AT_SKILL_POWER_SLASH_UP:
			case AT_SKILL_POWER_SLASH_UP+1:
			case AT_SKILL_POWER_SLASH_UP+2:
			case AT_SKILL_POWER_SLASH_UP+3:
			case AT_SKILL_POWER_SLASH_UP+4:
#endif //PJH_SEASON4_MASTER_RANK4
			case AT_SKILL_ICE_BLADE:
				SetAction ( so, PLAYER_ATTACK_TWO_HAND_SWORD_TWO );
				sc->AttackTime = 1;
				PlayBuffer( SOUND_SKILL_SWORD4 );
				break;
				
			case AT_SKILL_SPEAR:	// 창찌르기
				if(sc->Helper.Type == MODEL_HELPER+37)
					SetAction(so, PLAYER_FENRIR_ATTACK_SPEAR);	//^ 펜릴 캐릭터 에니메이션 설정 관련(스킬)
				else
					SetAction(so,PLAYER_ATTACK_SKILL_SPEAR);
				sc->AttackTime = 1;
				break;
#ifdef PJH_SEASON4_MASTER_RANK4
			case AT_SKILL_BLOW_UP:
			case AT_SKILL_BLOW_UP+1:
			case AT_SKILL_BLOW_UP+2:
			case AT_SKILL_BLOW_UP+3:
			case AT_SKILL_BLOW_UP+4:
#endif	//PJH_SEASON4_MASTER_RANK4
			case AT_SKILL_ONETOONE:	// 블로우
				SetAction(so,PLAYER_ATTACK_ONETOONE);
				if( sc != Hero && so->Type == MODEL_PLAYER)
				{
					so->AnimationFrame = 0;
				}
				sc->AttackTime = 1;
				//PlayBuffer(SOUND_FLASH);
				break;
			case AT_SKILL_MANY_ARROW_UP:
			case AT_SKILL_MANY_ARROW_UP+1:
			case AT_SKILL_MANY_ARROW_UP+2:
			case AT_SKILL_MANY_ARROW_UP+3:
			case AT_SKILL_MANY_ARROW_UP+4:
			case AT_SKILL_CROSSBOW:
				SetPlayerBow(sc);
				sc->AttackTime = 1;
				break;
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
			case AT_SKILL_MULTI_SHOT:
				SetPlayerBow(sc);
				if( sc != Hero && so->Type == MODEL_PLAYER)
				{
					so->AnimationFrame = 0;
				}
				sc->AttackTime = 1;
				break;
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
#ifdef PJH_SEASON4_DARK_NEW_SKILL_CAOTIC	
//			case AT_SKILL_GAOTIC:
//				SetPlayerMagic(sc);
//				break;
#endif //
			case AT_SKILL_BLAST_CROSSBOW4:
				SetPlayerBow(sc);
				sc->AttackTime = 1;
				break;
#ifdef PJH_SEASON4_MASTER_RANK4
			case AT_SKILL_BLOOD_ATT_UP:
			case AT_SKILL_BLOOD_ATT_UP+1:
			case AT_SKILL_BLOOD_ATT_UP+2:
			case AT_SKILL_BLOOD_ATT_UP+3:
			case AT_SKILL_BLOOD_ATT_UP+4:
#endif //PJH_SEASON4_MASTER_RANK4
			case AT_SKILL_REDUCEDEFENSE:
				if ( sc->SkillSuccess )
				{
					DeleteEffect(BITMAP_SKULL,to,0);
					vec3_t Light = { 1.0f, 1.f, 1.f};
					CreateEffect(BITMAP_SKULL,to->Position,to->Angle,Light,0,to);

					PlayBuffer(SOUND_BLOODATTACK,to);
					g_CharacterRegisterBuff(to, eDeBuff_Defense);
				}
				if ( so->Type != MODEL_PLAYER )
				{
					SetPlayerAttack(sc);
					so->AnimationFrame = 0;
					sc->AttackTime = 1;
				}
				break;
				
			case AT_SKILL_PIERCING:
				SetPlayerBow(sc);
				sc->AttackTime = 1;
				break;
				
			case AT_SKILL_PARALYZE:
				SetPlayerBow(sc);
				if( sc->SkillSuccess )
				{
					DeleteEffect(MODEL_ICE,to,1);
					
					vec3_t Angle;
					VectorCopy ( to->Angle, Angle );
					
					CreateEffect(MODEL_ICE,to->Position,Angle,to->Light,1,to);
					
					Angle[2] += 180.f;
					CreateEffect(MODEL_ICE,to->Position,Angle,to->Light,2,to);
					
					//  얼려진 캐릭터는 무조건 정지.
					tc->Movement = false;
					SetPlayerStop(tc);

					g_CharacterRegisterBuff(to, eDeBuff_Harden);
				}
				sc->AttackTime = 1;
				break;
				
			case AT_SKILL_RUSH:
				SetAction ( so, PLAYER_ATTACK_RUSH );
				sc->AttackTime = 1;
				PlayBuffer(SOUND_SKILL_SWORD2);
				break;
				
			case AT_SKILL_JAVELIN:
				if(sc != Hero)
				{
					if(so->Type == MODEL_PLAYER)
					{
						SetPlayerMagic(sc);
						so->AnimationFrame = 0;
					}
				}
				//        SetPlayerMagic(sc);
				so->AnimationFrame = 0;
				sc->AttackTime = 1;
				break;
				
			case AT_SKILL_DEATH_CANNON:
				//SetPlayerMagic(sc);
				SetAction ( so, PLAYER_ATTACK_DEATH_CANNON );
				so->AnimationFrame = 0;
				sc->AttackTime = 1;
				break;
				
			case AT_SKILL_DEEPIMPACT:
				SetPlayerHighBow ( sc );
				sc->AttackTime = 1;
				break;
				
			case AT_SKILL_SPACE_SPLIT :     //  공간 가르기.
#ifdef KJH_FIX_CHAOTIC_ANIMATION_ON_RIDE_PET
				if ( (sc->Helper.Type>=MODEL_HELPER+2 && sc->Helper.Type<=MODEL_HELPER+4) && !sc->SafeZone )
#else // KJH_FIX_CHAOTIC_ANIMATION_ON_RIDE_PET
				if ( sc->Helper.Type==MODEL_HELPER+4 && !sc->SafeZone )
#endif // KJH_FIX_CHAOTIC_ANIMATION_ON_RIDE_PET
				{
					SetAction ( so, PLAYER_ATTACK_RIDE_STRIKE );
				}
				else if(sc->Helper.Type == MODEL_HELPER+37 && !sc->SafeZone)
				{
					SetAction(so, PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE);	//^ 펜릴 캐릭터 에니메이션 관련
				}
				else
				{
					SetAction ( so, PLAYER_ATTACK_STRIKE );
				}
				sc->AttackTime = 1;
				PlayBuffer ( SOUND_ATTACK_FIRE_BUST );
				break;
				
			case AT_SKILL_BRAND_OF_SKILL:

				g_CharacterRegisterBuff(to, eBuff_AddSkill);	

				if ( sc->Helper.Type==MODEL_HELPER+4 && !sc->SafeZone )
				{
					SetAction ( so, PLAYER_ATTACK_RIDE_ATTACK_MAGIC );
				}
				else if(sc->Helper.Type == MODEL_HELPER+37 && !sc->SafeZone)
				{
					SetAction(so, PLAYER_FENRIR_ATTACK_MAGIC);	//^ 펜릴 캐릭터 에니메이션 관련
				}
				else
				{
					SetAction ( so, PLAYER_SKILL_HAND1 );
				}
				sc->AttackTime = 1;
				break;
				
			case AT_SKILL_ONEFLASH:
				SetAction ( so, PLAYER_ATTACK_ONE_FLASH );
				sc->AttackTime = 1;
				PlayBuffer(SOUND_SKILL_SWORD2);
				break;
				
			case AT_SKILL_STUN:
				if ( sc->Helper.Type==MODEL_HELPER+4 && !sc->SafeZone )
				{
					SetAction ( so, PLAYER_ATTACK_RIDE_ATTACK_MAGIC );
				} 
				else
					if( sc->Helper.Type==MODEL_HELPER+2 && !sc->SafeZone)         //  유니를 타고있음.
					{
						SetAction(so,PLAYER_SKILL_RIDER);
					}
					else
						if( sc->Helper.Type==MODEL_HELPER+3 && !sc->SafeZone )   //  페가시아를 타고있음.
						{
							SetAction(so,PLAYER_SKILL_RIDER_FLY);
						}
						else if(sc->Helper.Type == MODEL_HELPER+37 && !sc->SafeZone)	// 펜릴
						{
							SetAction(so, PLAYER_FENRIR_ATTACK_MAGIC);	//^ 펜릴 캐릭터 에니메이션 설정 관련
						}
						else
						{
							SetAction ( so, PLAYER_SKILL_VITALITY );
						}
						sc->AttackTime = 1;
						break;
						
			case AT_SKILL_REMOVAL_STUN:
				if ( sc->Helper.Type==MODEL_HELPER+4 && !sc->SafeZone )
				{
					SetAction ( so, PLAYER_ATTACK_RIDE_ATTACK_MAGIC );
				} 
				else
					if( sc->Helper.Type==MODEL_HELPER+2 && !sc->SafeZone)         //  유니를 타고있음.
					{
						SetAction(so,PLAYER_SKILL_RIDER);
					}
					else
						if( sc->Helper.Type==MODEL_HELPER+3 && !sc->SafeZone )   //  페가시아를 타고있음.
						{
							SetAction(so,PLAYER_SKILL_RIDER_FLY);
						}
						else if(sc->Helper.Type == MODEL_HELPER+37 && !sc->SafeZone)	// 펜릴
						{
							SetAction(so, PLAYER_FENRIR_ATTACK_MAGIC);	//^ 펜릴 캐릭터 에니메이션 설정 관련
						}
						else
						{
							SetAction ( so, PLAYER_ATTACK_REMOVAL );
						}
						
						if ( sc->SkillSuccess )
						{
							UnRegisterBuff( eDeBuff_Stun, to );
						}
						sc->AttackTime = 1;
						break;
						
			case AT_SKILL_INVISIBLE:
				if ( sc->Helper.Type==MODEL_HELPER+4 && !sc->SafeZone )
				{
					SetAction ( so, PLAYER_ATTACK_RIDE_ATTACK_MAGIC );
				} 
				else
					if( sc->Helper.Type==MODEL_HELPER+2 && !sc->SafeZone)         //  유니를 타고있음.
					{
						SetAction(so,PLAYER_SKILL_RIDER);
					}
					else
						if( sc->Helper.Type==MODEL_HELPER+3 && !sc->SafeZone )   //  페가시아를 타고있음.
						{
							SetAction(so,PLAYER_SKILL_RIDER_FLY);
						}
						else if(sc->Helper.Type == MODEL_HELPER+37 && !sc->SafeZone)	// 펜릴
						{
							SetAction(so, PLAYER_FENRIR_ATTACK_MAGIC);	//^ 펜릴 캐릭터 에니메이션 설정 관련
						}
						else
							SetAction ( so, PLAYER_SKILL_VITALITY );
						//		if ( sc->SkillSuccess )
						//		{
						//		    to->State |= STATE_INVISIBLE;
						//		}
						//		to->State |= STATE_INVISIBLE;
						if ( sc->SkillSuccess )
						{
							if( !g_isCharacterBuff(to, eBuff_Cloaking) )
								{
									if(so != to)
									{
										g_CharacterRegisterBuff(to, eBuff_Cloaking);
									}
								}
						}
						sc->AttackTime = 1;
						break;
						
			case AT_SKILL_REMOVAL_INVISIBLE:
				if ( sc->Helper.Type==MODEL_HELPER+4 && !sc->SafeZone )
				{
					SetAction ( so, PLAYER_ATTACK_RIDE_ATTACK_MAGIC );
				} 
				else
					if( sc->Helper.Type==MODEL_HELPER+2 && !sc->SafeZone)         //  유니를 타고있음.
					{
						SetAction(so,PLAYER_SKILL_RIDER);
					}
					else
						if( sc->Helper.Type==MODEL_HELPER+3 && !sc->SafeZone )   //  페가시아를 타고있음.
						{
							SetAction(so,PLAYER_SKILL_RIDER_FLY);
						}
						else if(sc->Helper.Type == MODEL_HELPER+37 && !sc->SafeZone)	// 펜릴
						{
							SetAction(so, PLAYER_FENRIR_ATTACK_MAGIC);	//^ 펜릴 캐릭터 에니메이션 설정 관련
						}
						else
							SetAction ( so, PLAYER_ATTACK_REMOVAL );
						if ( sc->SkillSuccess )
						{
							UnRegisterBuff( eBuff_Cloaking, to );
						}
						sc->AttackTime = 1;
						break;
						
			case AT_SKILL_MANA:
				if ( sc->Helper.Type==MODEL_HELPER+4 && !sc->SafeZone )
				{
					SetAction ( so, PLAYER_ATTACK_RIDE_ATTACK_MAGIC );
				} 
				else
					if( sc->Helper.Type==MODEL_HELPER+2 && !sc->SafeZone)         //  유니를 타고있음.
					{
						SetAction(so,PLAYER_SKILL_RIDER);
					}
					else
						if( sc->Helper.Type==MODEL_HELPER+3 && !sc->SafeZone )   //  페가시아를 타고있음.
						{
							SetAction(so,PLAYER_SKILL_RIDER_FLY);
						}
						else if(sc->Helper.Type == MODEL_HELPER+37 && !sc->SafeZone)	// 펜릴
						{
							SetAction(so, PLAYER_FENRIR_ATTACK_MAGIC);	//^ 펜릴 캐릭터 에니메이션 설정 관련
						}
						else
							SetAction ( so, PLAYER_SKILL_VITALITY );
						if ( sc->SkillSuccess )
						{
							g_CharacterRegisterBuff(so, eBuff_AddMana);

							CreateEffect ( MODEL_MANA_RUNE, so->Position, so->Angle, so->Light );
						}
						sc->AttackTime = 1;
						break;
						
			case AT_SKILL_REMOVAL_BUFF:
				if ( sc->SkillSuccess )
				{
					UnRegisterBuff( eBuff_Attack, to );
					UnRegisterBuff( eBuff_Defense, to );
					UnRegisterBuff( eBuff_HpRecovery, to ); //eBuff_Life
					UnRegisterBuff( eBuff_PhysDefense, to );
					UnRegisterBuff( eBuff_AddCriticalDamage, to );
					UnRegisterBuff( eBuff_AddMana, to );
				}
				if ( sc->Helper.Type==MODEL_HELPER+4 && !sc->SafeZone )
				{
					SetAction ( so, PLAYER_ATTACK_RIDE_ATTACK_MAGIC );
				} 
				else
					if( sc->Helper.Type==MODEL_HELPER+2 && !sc->SafeZone)         //  유니를 타고있음.
					{
						SetAction(so,PLAYER_SKILL_RIDER);
					}
					else
						if( sc->Helper.Type==MODEL_HELPER+3 && !sc->SafeZone )   //  페가시아를 타고있음.
						{
							SetAction(so,PLAYER_SKILL_RIDER_FLY);
						}
						else if(sc->Helper.Type == MODEL_HELPER+37 && !sc->SafeZone)	// 펜릴
						{
							SetAction(so, PLAYER_FENRIR_ATTACK_MAGIC);	//^ 펜릴 캐릭터 에니메이션 설정 관련
						}
						else
							SetAction ( so, PLAYER_SKILL_VITALITY );
						sc->AttackTime = 1;
						break;
			case AT_SKILL_IMPROVE_AG:
				if( !g_isCharacterBuff(to, eBuff_AddAG) )
					{
						DeleteEffect ( BITMAP_LIGHT, to, 2 );
						DeleteJoint ( BITMAP_JOINT_HEALING, to, 9 );
						
						CreateEffect ( BITMAP_LIGHT, to->Position, to->Angle, to->Light, 2, to );
					}

					g_CharacterRegisterBuff(to, eBuff_AddAG);		
					
					if(sc != Hero)
					{
						SetPlayerMagic(sc);
						so->AnimationFrame = 0;
					}
					sc->AttackTime = 1;
					break;
			case AT_SKILL_ADD_CRITICAL:
				SetAttackSpeed();

				g_CharacterRegisterBuff(to, eBuff_AddCriticalDamage);  
				
				if ( sc->Helper.Type==MODEL_HELPER+4 && !sc->SafeZone )
				{
					SetAction ( so, PLAYER_ATTACK_RIDE_ATTACK_MAGIC );
				}
				else if(sc->Helper.Type == MODEL_HELPER+37 && !sc->SafeZone)
				{
					SetAction( so, PLAYER_FENRIR_ATTACK_MAGIC);	//^ 펜릴 스킬 관련(캐릭터 에니메이션 설정)
				}
				else
				{
					SetAction ( so, PLAYER_SKILL_HAND1 );
				}
				sc->AttackTime = 1;
				break;
			case AT_SKILL_PARTY_TELEPORT:
				if ( (sc->Helper.Type>=MODEL_HELPER+2 && sc->Helper.Type<=MODEL_HELPER+4) && !sc->SafeZone )
				{
					SetAction ( so, PLAYER_ATTACK_RIDE_TELEPORT );
				}
				else if(sc->Helper.Type == MODEL_HELPER+37 && !sc->SafeZone)
				{
					SetAction(so, PLAYER_FENRIR_ATTACK_DARKLORD_TELEPORT);	//^ 펜릴 캐릭터 에니메이션
				}
				else
				{
					SetAction ( so, PLAYER_ATTACK_TELEPORT );
				}
				if ( so->Type!=MODEL_PLAYER )
				{
					SetPlayerAttack(sc);
					so->AnimationFrame = 0;
				}
				sc->AttackTime = 1;
				break;
			case AT_SKILL_LIFE_UP:
			case AT_SKILL_LIFE_UP+1:
			case AT_SKILL_LIFE_UP+2:
			case AT_SKILL_LIFE_UP+3:
			case AT_SKILL_LIFE_UP+4:
			case AT_SKILL_VITALITY:	// 스웰라이프
				if( !g_isCharacterBuff(to, eBuff_HpRecovery) ) //eBuff_Life
				{
					DeleteEffect ( BITMAP_LIGHT, to, 1 );
					
					CreateEffect ( BITMAP_LIGHT, to->Position, to->Angle, to->Light, 1, to );
				}
				
				g_CharacterRegisterBuff(to, eBuff_HpRecovery); //eBuff_Life
				
				SetAction(so,PLAYER_SKILL_VITALITY);
				sc->AttackTime = 1;
				break;
				
			case AT_SKILL_RIDER:
				if ( World==WD_8TARKAN || World==WD_10HEAVEN || g_Direction.m_CKanturu.IsMayaScene() )
					SetAction ( so, PLAYER_SKILL_RIDER_FLY );
				else 
					SetAction ( so, PLAYER_SKILL_RIDER );
				sc->AttackTime = 1;
				break;
				
			case AT_SKILL_BOSS:
				sc->AttackTime = 1;
				break;
				
			case AT_SKILL_COMBO :
				CreateEffect ( MODEL_COMBO, so->Position, so->Angle, so->Light );
				PlayBuffer(SOUND_COMBO);
				break;
			case AT_SKILL_BLAST_HELL_BEGIN:
				SetAction(so,PLAYER_SKILL_HELL_BEGIN);
				PlayBuffer(SOUND_NUKE1);
				so->m_bySkillCount = 0;
				break;
			case AT_SKILL_BLAST_HELL:
				SetAction(so,PLAYER_SKILL_HELL_START);
				if ( sc==Hero )
				{
					MouseRButtonPop = false;
					MouseRButtonPush= false;
					MouseRButton	= false;
					
					MouseRButtonPress = 0;
				}
				sc->AttackTime = 1;
				break;
#ifdef PJH_SEASON4_MASTER_RANK4
			case AT_SKILL_FIRE_SCREAM_UP:
			case AT_SKILL_FIRE_SCREAM_UP+1:
			case AT_SKILL_FIRE_SCREAM_UP+2:
			case AT_SKILL_FIRE_SCREAM_UP+3:
			case AT_SKILL_FIRE_SCREAM_UP+4:
#endif //PJH_SEASON4_MASTER_RANK4
			case AT_SKILL_DARK_SCREAM:		//  다크로드 새로운스킬
				{
#ifdef KJH_FIX_CHAOTIC_ANIMATION_ON_RIDE_PET
					if( sc->Helper.Type == MODEL_HELPER+37 )				
					{
						SetAction( so, PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE );		// 팬릴착용
					}
#ifdef PBG_FIX_CHAOTIC_ANIMATION
					else if((sc->Helper.Type>=MODEL_HELPER+2) && (sc->Helper.Type<=MODEL_HELPER+4))
#else //PBG_FIX_CHAOTIC_ANIMATION
					else if((sc->Helper.Type>=MODEL_HELPER+2) || (sc->Helper.Type<=MODEL_HELPER+4))
#endif //PBG_FIX_CHAOTIC_ANIMATION
					{
						SetAction( so, PLAYER_ATTACK_RIDE_STRIKE );		// 탈것착용 (다크호스포함)
					}
					else
					{
						SetAction( so, PLAYER_ATTACK_STRIKE );				// 일반공격
					}
#else // KJH_FIX_CHAOTIC_ANIMATION_ON_RIDE_PET
					if(sc->Helper.Type == MODEL_HELPER+37)
						SetAction(so,PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE);	//^ 펜릴
					else
						SetAction ( so, PLAYER_ATTACK_STRIKE );
#endif // KJH_FIX_CHAOTIC_ANIMATION_ON_RIDE_PET
					PlayBuffer(SOUND_FIRE_SCREAM);
				}
				break;
			case AT_SKILL_EXPLODE:
				{
					vec3_t Pos;
					float addx,addz;
					for(int iaa = 0; iaa < 3; iaa++)
					{
						addx = rand()%160;
						addz = rand()%160;
						Vector(to->Position[0]+addx,to->Position[1]+addz,to->Position[2], Pos);
						
						CreateBomb(Pos,true);
					}
				}
				break;
			case AT_SKILL_INFINITY_ARROW:
				{
					CharacterMachine->InfinityArrowAdditionalMana = 10;
					CreateEffect(MODEL_INFINITY_ARROW, so->Position, so->Angle, so->Light, 0, so);
					PlayBuffer(SOUND_INFINITYARROW);
				}	
				break;
#ifdef PJH_ADD_MASTERSKILL
			case AT_SKILL_ALICE_CHAINLIGHTNING_UP:
		case AT_SKILL_ALICE_CHAINLIGHTNING_UP+1:
		case AT_SKILL_ALICE_CHAINLIGHTNING_UP+2:
		case AT_SKILL_ALICE_CHAINLIGHTNING_UP+3:
		case AT_SKILL_ALICE_CHAINLIGHTNING_UP+4:
#endif
			case AT_SKILL_ALICE_CHAINLIGHTNING:
				{
					sc->AttackTime = 1;
					PlayBuffer(SOUND_SKILL_CHAIN_LIGHTNING);
				}
				break;
#ifdef PJH_ADD_MASTERSKILL
		case AT_SKILL_ALICE_SLEEP_UP:
		case AT_SKILL_ALICE_SLEEP_UP+1:
		case AT_SKILL_ALICE_SLEEP_UP+2:
		case AT_SKILL_ALICE_SLEEP_UP+3:
		case AT_SKILL_ALICE_SLEEP_UP+4:
#endif
			case AT_SKILL_ALICE_SLEEP:
			case AT_SKILL_ALICE_BLIND:
			case AT_SKILL_ALICE_THORNS:
#ifdef ASG_ADD_SKILL_BERSERKER
			case AT_SKILL_ALICE_BERSERKER:
#endif	// ASG_ADD_SKILL_BERSERKER
				{
					sc->AttackTime = 1;
					
					// 동작 설정
					switch(sc->Helper.Type)
					{
					case MODEL_HELPER+2:	// 유니리아
						SetAction(so, PLAYER_SKILL_SLEEP_UNI);
						break;
					case MODEL_HELPER+3:	// 디노란트
						SetAction(so, PLAYER_SKILL_SLEEP_DINO);
						break;
					case MODEL_HELPER+37:	// 펜릴	
						SetAction(so, PLAYER_SKILL_SLEEP_FENRIR);
						break;
					default:	// 기본
						SetAction(so, PLAYER_SKILL_SLEEP);
						break;
					}
					
					// 사운드
					if(MagicNumber == AT_SKILL_ALICE_SLEEP
#ifdef PJH_ADD_MASTERSKILL
						|| (AT_SKILL_ALICE_SLEEP_UP <= MagicNumber && MagicNumber <= AT_SKILL_ALICE_SLEEP_UP+4)
#endif
						) 
					{
							PlayBuffer(SOUND_SUMMON_SKILL_SLEEP);
					}
					else
					if(MagicNumber == AT_SKILL_ALICE_BLIND) 
					{
						PlayBuffer(SOUND_SUMMON_SKILL_BLIND);
					}
					else
					{
						PlayBuffer(SOUND_SUMMON_SKILL_THORNS);
					}
				}
				break;

#ifdef KJH_ADD_SKILL_SWELL_OF_MAGICPOWER		// 마력증대
			case AT_SKILL_SWELL_OF_MAGICPOWER:
				{
					SetAttackSpeed();
					SetAction( so, PLAYER_SKILL_SWELL_OF_MP );
								
					vec3_t vLight;
					Vector( 0.3f, 0.2f, 0.9f, vLight );
					CreateEffect( MODEL_SWELL_OF_MAGICPOWER, so->Position, so->Angle, vLight, 0, so );
#ifdef KJH_ADD_SKILL_SWELL_OF_MAGICPOWER_SOUND
					PlayBuffer(SOUND_SKILL_SWELL_OF_MAGICPOWER);
#endif // KJH_ADD_SKILL_SWELL_OF_MAGICPOWER_SOUND
				}break;
#endif // KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
#ifdef YDG_ADD_DOPPELGANGER_MONSTER
			case AT_SKILL_DOPPELGANGER_SELFDESTRUCTION:
				{
					SetAction(so, MONSTER01_APEAR);
					so->m_bActionStart = FALSE;
				}
				break;
#endif	// YDG_ADD_DOPPELGANGER_MONSTER
#ifdef LDK_MOD_EVERY_USE_SKILL_CAOTIC
			case AT_SKILL_GAOTIC:
				{
					if(so->Type == MODEL_PLAYER)
					{
						if( sc->Helper.Type == MODEL_HELPER+37 )				
						{
							SetAction( &sc->Object, PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE );		// 팬릴착용
						}
						else if((sc->Helper.Type>=MODEL_HELPER+2) && (sc->Helper.Type<=MODEL_HELPER+4))
						{
							SetAction( &sc->Object, PLAYER_ATTACK_RIDE_STRIKE );		// 탈것착용 (다크호스포함)
						}
						else
						{
							SetAction( &sc->Object, PLAYER_ATTACK_STRIKE );				// 일반공격
						}
					}
					else
					{
						SetPlayerAttack ( sc );
					}
					
					OBJECT* o = so;
					vec3_t Light,Position,P,dp;
					
					float Matrix[3][4];
					Vector(0.f,-20.f,0.f,P);
					Vector(0.f,0.f,0.f,P);
					AngleMatrix(o->Angle,Matrix);
					VectorRotate(P,Matrix,dp);
					VectorAdd(dp,o->Position,Position);
					
					Vector(0.5f, 0.5f, 0.5f, Light);
					for (int i = 0; i < 5; ++i)
					{
						CreateEffect(BITMAP_SHINY+6, Position, o->Angle, Light, 3, o, -1, 0, 0, 0, 0.5f);
					}
					
					VectorCopy(o->Position,Position);

					for(int i=0;i<8;i++)
					{
						Position[0] = (o->Position[0] - 119.f) + (float)(rand()%240); 
						Position[2] = (o->Position[2] + 49.f) + (float)(rand()%60); 
						CreateJoint(BITMAP_2LINE_GHOST,Position,o->Position,o->Angle,0,o,20.f,o->PKKey,0,o->m_bySkillSerialNum);//클라이언트마법처리
					}
					if(sc==Hero && SelectedCharacter!=-1)
					{
						vec3_t Pos;
						CHARACTER *st = &CharactersClient[SelectedCharacter];
						VectorCopy(st->Object.Position,Pos);
						CreateBomb(Pos,true);
					}
					PlayBuffer(SOUND_SKILL_CAOTIC);
				}
				break;
#endif //LDK_MOD_EVERY_USE_SKILL_CAOTIC
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	case AT_SKILL_THRUST:
		{
			g_CMonkSystem.SetRageSkillAni(MagicNumber, so);
			so->m_sTargetIndex = TargetIndex;
			sc->AttackTime = 1;

			if(sc != Hero)
			{
				g_CMonkSystem.RageCreateEffect(so, MagicNumber);
			}
		}
		break;
	case AT_SKILL_ATT_UP_OURFORCES:
	case AT_SKILL_HP_UP_OURFORCES:
	case AT_SKILL_DEF_UP_OURFORCES:
		{
			if(sc == Hero)
			{
				if(so->CurrentAction == PLAYER_SKILL_ATT_UP_OURFORCES)
				{
					SendRequestAction(AT_RAGEBUFF_1, ((BYTE)((so->Angle[2]+22.5f)/360.f*8.f+1.f)%8));
				}
				else
				{
					SendRequestAction(AT_RAGEBUFF_2, ((BYTE)((so->Angle[2]+22.5f)/360.f*8.f+1.f)%8));
				}
			}
			OBJECT *_pObj = to;
			if(sc != Hero)
			{
				_pObj = so;
				g_CMonkSystem.RageCreateEffect(_pObj, MagicNumber);
			}

			if(MagicNumber==AT_SKILL_HP_UP_OURFORCES)
			{
				DeleteEffect(BITMAP_LIGHT_RED, _pObj, 0);
				CreateEffect(BITMAP_LIGHT_RED, _pObj->Position, _pObj->Angle, _pObj->Light, 0, _pObj,-1,0,0,0,1.5f);
			}
			else if(MagicNumber==AT_SKILL_DEF_UP_OURFORCES)
			{
				DeleteEffect(BITMAP_LIGHT_RED, _pObj, 2);
				CreateEffect(BITMAP_LIGHT_RED, _pObj->Position, _pObj->Angle, _pObj->Light, 2, _pObj,-1,0,0,0,1.5f);

				vec3_t vLight;
				Vector(0.7f, 0.7f,1.0f,vLight);
				DeleteEffect(MODEL_WINDFOCE, _pObj, 1);
				CreateEffect(MODEL_WINDFOCE, _pObj->Position, _pObj->Angle, vLight, 1, _pObj, -1, 0, 0, 0, 1.0f);
			}
			else if(MagicNumber==AT_SKILL_ATT_UP_OURFORCES)
			{
				DeleteEffect(BITMAP_LIGHT_RED, _pObj, 1);
				CreateEffect(BITMAP_LIGHT_RED, _pObj->Position, _pObj->Angle, _pObj->Light, 1, _pObj,-1,0,0,0,1.5f);
			}

			switch(MagicNumber)
			{
			case AT_SKILL_ATT_UP_OURFORCES:
				g_CharacterRegisterBuff(_pObj, eBuff_Att_up_Ourforces);
				break;
			case AT_SKILL_HP_UP_OURFORCES:
				g_CharacterRegisterBuff(_pObj, eBuff_Hp_up_Ourforces);
				break;
			case AT_SKILL_DEF_UP_OURFORCES:
				g_CharacterRegisterBuff(_pObj, eBuff_Def_up_Ourforces);
				break;
			}
			sc->AttackTime = 1;
		}
		break;
	case AT_SKILL_STAMP:
		{
			sc->AttackTime = 1;
		}
		break;
	case AT_SKILL_GIANTSWING:
	case AT_SKILL_DRAGON_KICK:
		{
			sc->AttackTime = 1;
			if(sc != Hero)
				g_CMonkSystem.RageCreateEffect(so, MagicNumber);
		}
		break;
	case AT_SKILL_OCCUPY:
		{
			SetAction(so, PLAYER_ATTACK_RUSH);
			sc->AttackTime = 1;
			PlayBuffer(SOUND_SKILL_SWORD2);
		}
		break;
	case AT_SKILL_DRAGON_LOWER:
		{
			vec3_t vLight;
			Vector(1.0f, 1.0f, 1.0f, vLight);
			if(g_CMonkSystem.SetLowerEffEct())
			{
				CreateEffect(MODEL_VOLCANO_OF_MONK, to->Position, to->Angle, vLight, g_CMonkSystem.GetLowerEffCnt(), to, -1, 0, 0, 0, 0.8f);
			}

			CreateEffect(MODEL_TARGETMON_EFFECT, to->Position, to->Angle, vLight, 0, to, -1, 0, 0, 0, 1.0f);
#ifdef PBG_MOD_RAGEFIGHTERSOUND
			StopBuffer(SOUND_RAGESKILL_DRAGONLOWER_ATTACK, true);
#endif //PBG_MOD_RAGEFIGHTERSOUND
			PlayBuffer(SOUND_RAGESKILL_DRAGONLOWER_ATTACK);
		}
		break;
	case AT_SKILL_DARKSIDE:
		{
			if(sc != Hero)
				g_CMonkSystem.SetDarksideCnt();
		}
		break;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
	}
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x19 [ReceiveMagic(%d)]", MagicNumber);
#endif // CONSOLE_DEBUG
	
	return ( TRUE);
}

BOOL ReceiveMagicContinue(BYTE *ReceiveBuffer,int Size, BOOL bEncrypted)
{
	if ( !bEncrypted)
	{
		GO_DEBUG;
		SendHackingChecked( 0x00, 0x1E);
		return ( FALSE);
	}
	
	LPPRECEIVE_MAGIC_CONTINUE Data = (LPPRECEIVE_MAGIC_CONTINUE)ReceiveBuffer;
	int Key = ((int)(Data->KeyH )<<8) + Data->KeyL;
	WORD MagicNumber = ((WORD)(Data->MagicH )<<8) + Data->MagicL;
	
	CHARACTER *sc = &CharactersClient[FindCharacterIndex(Key)];
	OBJECT *so = &sc->Object;
	
	sc->Skill = MagicNumber;

	
	so->Angle[2] = (Data->Angle/255.f)*360.f;
	
	if(so->Type == MODEL_PLAYER)
	{
		if(sc != Hero)
		{
			switch(MagicNumber)
			{
			case AT_SKILL_MANY_ARROW_UP:
			case AT_SKILL_MANY_ARROW_UP+1:
			case AT_SKILL_MANY_ARROW_UP+2:
			case AT_SKILL_MANY_ARROW_UP+3:
			case AT_SKILL_MANY_ARROW_UP+4:
			case AT_SKILL_CROSSBOW:
				SetPlayerAttack(sc);
				break;
			case AT_SKILL_BLAST_CROSSBOW4:
				SetPlayerAttack(sc);
				break;
			case AT_SKILL_BLAST_POISON:
			case AT_SKILL_ICE_UP:
			case AT_SKILL_ICE_UP+1:
			case AT_SKILL_ICE_UP+2:
			case AT_SKILL_ICE_UP+3:
			case AT_SKILL_ICE_UP+4:
			case AT_SKILL_BLAST_FREEZE:
				SetPlayerMagic(sc);
				break;
				
            case AT_SKILL_PIERCING:
                SetPlayerAttack(sc);
                break;
				
			case AT_SKILL_FLASH:
				SetAction(so,PLAYER_SKILL_FLASH);
				break;
				
			case AT_SKILL_HELL_FIRE_UP:
			case AT_SKILL_HELL_FIRE_UP+1:
			case AT_SKILL_HELL_FIRE_UP+2:
			case AT_SKILL_HELL_FIRE_UP+3:
			case AT_SKILL_HELL_FIRE_UP+4:
			case AT_SKILL_HELL:
				SetAction(so,PLAYER_SKILL_HELL);
				break;
				
			case AT_SKILL_INFERNO:
				SetAction(so,PLAYER_SKILL_INFERNO);
				break;
			case AT_SKILL_TORNADO_SWORDA_UP:
			case AT_SKILL_TORNADO_SWORDA_UP+1:
			case AT_SKILL_TORNADO_SWORDA_UP+2:
			case AT_SKILL_TORNADO_SWORDA_UP+3:
			case AT_SKILL_TORNADO_SWORDA_UP+4:
				
			case AT_SKILL_TORNADO_SWORDB_UP:
			case AT_SKILL_TORNADO_SWORDB_UP+1:
			case AT_SKILL_TORNADO_SWORDB_UP+2:
			case AT_SKILL_TORNADO_SWORDB_UP+3:
			case AT_SKILL_TORNADO_SWORDB_UP+4:
			case AT_SKILL_WHEEL:
#ifdef YDG_ADD_SKILL_RIDING_ANIMATIONS
				switch(sc->Helper.Type)
				{
				case MODEL_HELPER+2:	// 유니리아
					SetAction(so, PLAYER_ATTACK_SKILL_WHEEL_UNI);
					break;
				case MODEL_HELPER+3:	// 디노란트
					SetAction(so, PLAYER_ATTACK_SKILL_WHEEL_DINO);
					break;
				case MODEL_HELPER+37:	// 펜릴	
					SetAction(so, PLAYER_ATTACK_SKILL_WHEEL_FENRIR);
					break;
				default:	// 기본
					SetAction(so, PLAYER_ATTACK_SKILL_WHEEL);
					break;
				}
#else	// YDG_ADD_SKILL_RIDING_ANIMATIONS
				SetAction(so,PLAYER_ATTACK_SKILL_WHEEL);
#endif	// YDG_ADD_SKILL_RIDING_ANIMATIONS
                break;
				
				// 다크로드 스킬 파이어 스크림
#ifdef PJH_SEASON4_MASTER_RANK4
			case AT_SKILL_FIRE_SCREAM_UP:
			case AT_SKILL_FIRE_SCREAM_UP+1:
			case AT_SKILL_FIRE_SCREAM_UP+2:
			case AT_SKILL_FIRE_SCREAM_UP+3:
			case AT_SKILL_FIRE_SCREAM_UP+4:
#endif //PJH_SEASON4_MASTER_RANK4
			case AT_SKILL_DARK_SCREAM:
				{
#ifdef KJH_FIX_CHAOTIC_ANIMATION_ON_RIDE_PET
					if( sc->Helper.Type == MODEL_HELPER+37 )				
					{
						SetAction( so, PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE );		// 팬릴착용
					}
#ifdef PBG_FIX_CHAOTIC_ANIMATION
					else if((sc->Helper.Type>=MODEL_HELPER+2) && (sc->Helper.Type<=MODEL_HELPER+4))
#else //PBG_FIX_CHAOTIC_ANIMATION
					else if((sc->Helper.Type>=MODEL_HELPER+2) || (sc->Helper.Type<=MODEL_HELPER+4))
#endif //PBG_FIX_CHAOTIC_ANIMATION
					{
						SetAction( so, PLAYER_ATTACK_RIDE_STRIKE );		// 탈것착용 (다크호스포함)
					}
					else
					{
						SetAction( so, PLAYER_ATTACK_STRIKE );				// 일반공격
					}
#else // KJH_FIX_CHAOTIC_ANIMATION_ON_RIDE_PET
					if(sc->Helper.Type == MODEL_HELPER+37)
						SetAction(so,PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE);	
					else
						SetAction ( so, PLAYER_ATTACK_STRIKE );
#endif // KJH_FIX_CHAOTIC_ANIMATION_ON_RIDE_PET
					// 사운드
					PlayBuffer(SOUND_FIRE_SCREAM);
				}
				break;
				
            case AT_SKILL_THUNDER_STRIKE:
				if(sc->Helper.Type == MODEL_HELPER+37)	//^ 펜릴
					SetAction(so,PLAYER_FENRIR_ATTACK_DARKLORD_FLASH);	
				else
					SetAction ( so, PLAYER_SKILL_FLASH );
                break;
			case AT_SKILL_ASHAKE_UP:
			case AT_SKILL_ASHAKE_UP+1:
			case AT_SKILL_ASHAKE_UP+2:
			case AT_SKILL_ASHAKE_UP+3:
			case AT_SKILL_ASHAKE_UP+4:
            case AT_SKILL_DARK_HORSE:       //  다크홀스 스킬.
                SetAction ( so, PLAYER_ATTACK_DARKHORSE );
                PlayBuffer ( SOUND_EARTH_QUAKE );
                break;
				
				//----------------------------------------------------------------------------------------------------
				
#ifdef PJH_SEASON4_MASTER_RANK4
			case AT_SKILL_ANGER_SWORD_UP:
			case AT_SKILL_ANGER_SWORD_UP+1:
			case AT_SKILL_ANGER_SWORD_UP+2:
			case AT_SKILL_ANGER_SWORD_UP+3:
			case AT_SKILL_ANGER_SWORD_UP+4:
#endif //PJH_SEASON4_MASTER_RANK4
            case AT_SKILL_FURY_STRIKE:	// 분노의 일격
				SetAction(so,PLAYER_ATTACK_SKILL_FURY_STRIKE);
                break;
				
				//----------------------------------------------------------------------------------------------------
				
#ifdef CSK_ADD_SKILL_BLOWOFDESTRUCTION
				// 흑기사 - 파괴의 일격
			case AT_SKILL_BLOW_OF_DESTRUCTION:
				SetAction(so,PLAYER_SKILL_BLOW_OF_DESTRUCTION);
				break;
#endif // CSK_ADD_SKILL_BLOWOFDESTRUCTION
				
				//----------------------------------------------------------------------------------------------------
				
				
				//----------------------------------------------------------------------------------------------------
				
			case AT_SKILL_SPEAR:	// 창찌르기
				if(sc->Helper.Type == MODEL_HELPER+37)
					SetAction(so, PLAYER_FENRIR_ATTACK_SPEAR);	//^ 펜릴 스킬 관련
				else
					SetAction(so,PLAYER_ATTACK_SKILL_SPEAR);
				break;
				
				//----------------------------------------------------------------------------------------------------
				
#ifdef PJH_SEASON4_MASTER_RANK4
			case AT_SKILL_BLOOD_ATT_UP:
			case AT_SKILL_BLOOD_ATT_UP+1:
			case AT_SKILL_BLOOD_ATT_UP+2:
			case AT_SKILL_BLOOD_ATT_UP+3:
			case AT_SKILL_BLOOD_ATT_UP+4:
#endif //PJH_SEASON4_MASTER_RANK4
			case AT_SKILL_REDUCEDEFENSE:
#ifdef YDG_ADD_SKILL_RIDING_ANIMATIONS
				switch(sc->Helper.Type)
				{
				case MODEL_HELPER+2:	// 유니리아
					SetAction(so, PLAYER_ATTACK_SKILL_WHEEL_UNI);
					break;
				case MODEL_HELPER+3:	// 디노란트
					SetAction(so, PLAYER_ATTACK_SKILL_WHEEL_DINO);
					break;
				case MODEL_HELPER+37:	// 펜릴	
					SetAction(so, PLAYER_ATTACK_SKILL_WHEEL_FENRIR);
					break;
				default:	// 기본
					SetAction(so, PLAYER_ATTACK_SKILL_WHEEL);
					break;
				}
#else	// YDG_ADD_SKILL_RIDING_ANIMATIONS
				SetAction(so,PLAYER_ATTACK_SKILL_WHEEL);
#endif	// YDG_ADD_SKILL_RIDING_ANIMATIONS
				break;
#ifdef PJH_SEASON4_MASTER_RANK4
			case AT_SKILL_POWER_SLASH_UP:
			case AT_SKILL_POWER_SLASH_UP+1:
			case AT_SKILL_POWER_SLASH_UP+2:
			case AT_SKILL_POWER_SLASH_UP+3:
			case AT_SKILL_POWER_SLASH_UP+4:
#endif //PJH_SEASON4_MASTER_RANK4
            case AT_SKILL_ICE_BLADE:
				SetAction(so,PLAYER_ATTACK_TWO_HAND_SWORD_TWO);
				break;
#ifdef PJH_SEASON4_MASTER_RANK4
			case AT_SKILL_BLOW_UP:
			case AT_SKILL_BLOW_UP+1:
			case AT_SKILL_BLOW_UP+2:
			case AT_SKILL_BLOW_UP+3:
			case AT_SKILL_BLOW_UP+4:
#endif	//PJH_SEASON4_MASTER_RANK4
            case AT_SKILL_ONETOONE:	// 블로우
				SetAction(so,PLAYER_ATTACK_ONETOONE);
                break;
				
            case AT_SKILL_STUN:
                SetAction ( so, PLAYER_SKILL_VITALITY );
                break;
                
            case AT_SKILL_INVISIBLE:
                SetAction ( so, PLAYER_SKILL_VITALITY );
                break;
				
			case AT_SKILL_PLASMA_STORM_FENRIR:
				SetAction_Fenrir_Skill(sc, so);	//^ 펜릴 스킬 관련
				break;
				
#ifdef PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
			case AT_SKILL_GAOTIC:
				{
#ifdef KJH_FIX_CHAOTIC_ANIMATION_ON_RIDE_PET
					if( sc->Helper.Type == MODEL_HELPER+37 )				
					{
						SetAction( &sc->Object, PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE );		// 팬릴착용
					}
#ifdef PBG_FIX_CHAOTIC_ANIMATION
					else if((sc->Helper.Type>=MODEL_HELPER+2) && (sc->Helper.Type<=MODEL_HELPER+4))
#else //PBG_FIX_CHAOTIC_ANIMATION
					else if((sc->Helper.Type>=MODEL_HELPER+2) || (sc->Helper.Type<=MODEL_HELPER+4))
#endif //PBG_FIX_CHAOTIC_ANIMATION
					{
						SetAction( &sc->Object, PLAYER_ATTACK_RIDE_STRIKE );		// 탈것착용 (다크호스포함)
					}
					else
					{
						SetAction( &sc->Object, PLAYER_ATTACK_STRIKE );				// 일반공격
					}
#else // KJH_FIX_CHAOTIC_ANIMATION_ON_RIDE_PET
					if(sc->Helper.Type == MODEL_HELPER+37)
						SetAction(so,PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE);	//^ 펜릴
					else
						SetAction ( so, PLAYER_ATTACK_STRIKE );
#endif // KJH_FIX_CHAOTIC_ANIMATION_ON_RIDE_PET

					OBJECT* o = so;
					vec3_t Light,Position,P,dp;
					
					float Matrix[3][4];
					Vector(0.f,-20.f,0.f,P);
					Vector(0.f,0.f,0.f,P);
					AngleMatrix(o->Angle,Matrix);
					VectorRotate(P,Matrix,dp);
					VectorAdd(dp,o->Position,Position);
					
					Vector(0.5f, 0.5f, 0.5f, Light);
					for (int i = 0; i < 5; ++i)
					{
						CreateEffect(BITMAP_SHINY+6, Position, o->Angle, Light, 3, o, -1, 0, 0, 0, 0.5f);
					}
					
					VectorCopy(o->Position,Position);

					for(int i=0;i<8;i++)
					{
						Position[0] = (o->Position[0] - 119.f) + (float)(rand()%240); 
						Position[2] = (o->Position[2] + 49.f) + (float)(rand()%60); 
						CreateJoint(BITMAP_2LINE_GHOST,Position,o->Position,o->Angle,0,o,20.f,o->PKKey,0,o->m_bySkillSerialNum);//클라이언트마법처리
					}
#ifdef PJH_FIX_CAOTIC
					if(sc==Hero && SelectedCharacter!=-1)
					{
						vec3_t Pos;
						CHARACTER *st = &CharactersClient[SelectedCharacter];
						VectorCopy(st->Object.Position,Pos);
						CreateBomb(Pos,true);
					}
#endif //PJH_FIX_CAOTIC
					PlayBuffer(SOUND_SKILL_CAOTIC);
				}
				break;
#endif //PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
			case AT_SKILL_MULTI_SHOT:
				{
					SetPlayerBow(sc);
					OBJECT* o = so;
					vec3_t Light,Position,P,dp;

					
#ifdef PJH_SEASON4_FIX_MULTI_SHOT
					float Matrix[3][4];
					Vector(0.f,20.f,0.f,P);
					AngleMatrix(o->Angle,Matrix);
					VectorRotate(P,Matrix,dp);
					VectorAdd(dp,o->Position,Position);
					Vector(0.8f, 0.9f, 1.6f, Light);
					CreateEffect ( MODEL_MULTI_SHOT3, Position, o->Angle, Light, 0);
					CreateEffect ( MODEL_MULTI_SHOT3, Position, o->Angle, Light, 0);
					
					Vector(0.f,-20.f,0.f,P);
					Vector(0.f,0.f,0.f,P);
					AngleMatrix(o->Angle,Matrix);
					VectorRotate(P,Matrix,dp);
					VectorAdd(dp,o->Position,Position);
					
					CreateEffect ( MODEL_MULTI_SHOT1, Position, o->Angle, Light, 0);
					CreateEffect ( MODEL_MULTI_SHOT1, Position, o->Angle, Light, 0);
					CreateEffect ( MODEL_MULTI_SHOT1, Position, o->Angle, Light, 0);
					
					Vector(0.f,20.f,0.f,P);
					AngleMatrix(o->Angle,Matrix);
					VectorRotate(P,Matrix,dp);
					VectorAdd(dp,o->Position,Position);
					CreateEffect ( MODEL_MULTI_SHOT2, Position, o->Angle, Light, 0);
					CreateEffect ( MODEL_MULTI_SHOT2, Position, o->Angle, Light, 0);
					
					Vector(0.f,-120.f,145.f,P);
					AngleMatrix(o->Angle,Matrix);
					VectorRotate(P,Matrix,dp);
					VectorAdd(dp,o->Position,Position);

					short Key = -1;
					for( int i=0;i<MAX_CHARACTERS_CLIENT;i++ )
					{
						CHARACTER *tc = &CharactersClient[i];
						if(tc == sc)
						{
							Key = i;
							break;
						}
					}

					CreateEffect(MODEL_BLADE_SKILL, Position, o->Angle, Light, 1, o, Key);
#else	//PJH_SEASON4_FIX_MULTI_SHOT

					float Matrix[3][4];
					Vector(0.f,20.f,0.f,P);
					AngleMatrix(o->Angle,Matrix);
					VectorRotate(P,Matrix,dp);
					VectorAdd(dp,o->Position,Position);
					Vector(0.4f, 0.6f, 1.f, Light);
					CreateEffect ( MODEL_MULTI_SHOT3, Position, o->Angle, Light, 0);
					CreateEffect ( MODEL_MULTI_SHOT3, Position, o->Angle, Light, 0);
					
					Vector(0.f,-20.f,0.f,P);
					Vector(0.f,0.f,0.f,P);
					AngleMatrix(o->Angle,Matrix);
					VectorRotate(P,Matrix,dp);
					VectorAdd(dp,o->Position,Position);
					
					CreateEffect ( MODEL_MULTI_SHOT1, Position, o->Angle, Light, 0);
					CreateEffect ( MODEL_MULTI_SHOT1, Position, o->Angle, Light, 0);
					CreateEffect ( MODEL_MULTI_SHOT1, Position, o->Angle, Light, 0);
					
					Vector(0.f,20.f,0.f,P);
					AngleMatrix(o->Angle,Matrix);
					VectorRotate(P,Matrix,dp);
					VectorAdd(dp,o->Position,Position);
					CreateEffect ( MODEL_MULTI_SHOT2, Position, o->Angle, Light, 0);
					CreateEffect ( MODEL_MULTI_SHOT2, Position, o->Angle, Light, 0);
					
					Vector(0.f,-120.f,130.f,P);
					AngleMatrix(o->Angle,Matrix);
					VectorRotate(P,Matrix,dp);
					VectorAdd(dp,o->Position,Position);
					CreateEffect(MODEL_BLADE_SKILL, Position, o->Angle, Light, 1);
					
					BYTE Skill = 0;
					//			CHARACTER *tc;
					OBJECT *to = NULL;
					vec3_t Angle;

					VectorCopy(o->Angle,Angle);
					CreateArrow(sc,o,to,FindHotKey(( o->Skill)),1,0);
					o->Angle[2] = Angle[2] + 8.f;
					CreateArrow(sc,o,to,FindHotKey(( o->Skill)),1,0);
					o->Angle[2] = Angle[2] + 16.f;
					CreateArrow(sc,o,to,FindHotKey(( o->Skill)),1,0);
					o->Angle[2] = Angle[2] - 8.f;
					CreateArrow(sc,o,to,FindHotKey(( o->Skill)),1,0);
					o->Angle[2] = Angle[2] - 16.f;
					CreateArrow(sc,o,to,FindHotKey(( o->Skill)),1,0);
					o->Angle[2] = Angle[2];
					PlayBuffer(SOUND_SKILL_MULTI_SHOT);
#endif //PJH_SEASON4_FIX_MULTI_SHOT
				}
				break;
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
			case AT_SKILL_RECOVER:
				{
					//			OBJECT* o = so;
					vec3_t Light,Position,P,dp;
					vec3_t vFirePosition;
					
					float Matrix[3][4];
					/*
					Vector(0.f,-220.f,130.f,P);
					AngleMatrix(o->Angle,Matrix);
					VectorRotate(P,Matrix,dp);
					VectorAdd(dp,o->Position,Position);
					Vector(0.7f, 0.6f, 0.f, Light);
					CreateEffect(BITMAP_IMPACT, Position, o->Angle, Light, 0,o);
					SetAction(o,PLAYER_RECOVER_SKILL);
					
					  
					*/
					OBJECT* o = so;
					
					Vector(0.4f, 0.6f, 1.f, Light);
					Vector(0.f,0.f,0.f,P);
					AngleMatrix(o->Angle,Matrix);
					VectorRotate(P,Matrix,dp);
					VectorAdd(dp,o->Position,Position);
					Position[2] += 130;
					VectorCopy( o->Position, Position);
					for ( int i=0; i<19; ++i )
					{
						CreateJoint(BITMAP_FLARE,Position,Position,o->Angle,47,o,40,2);
					}
					Vector ( 0.3f, 0.2f, 0.1f, Light );
					CreateEffect(MODEL_SUMMON,Position,o->Angle,Light, 0);
					CreateEffect ( BITMAP_TWLIGHT, Position, o->Angle, Light, 0 );
					CreateEffect ( BITMAP_TWLIGHT, Position, o->Angle, Light, 1 );
					CreateEffect ( BITMAP_TWLIGHT, Position, o->Angle, Light, 2 );
					
					for (int i = 0; i < 2; ++i)
					{
						int iNumBones = Models[o->Type].NumBones;
						Models[o->Type].TransformByObjectBone(vFirePosition, o, rand()%iNumBones);	
						CreateEffect(BITMAP_FLARE, vFirePosition, o->Angle, Light,3);
					}
					PlayBuffer(SOUND_SKILL_RECOVER);
				}
				break;
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
				
				
			case AT_SKILL_ALICE_LIGHTNINGORB:
				{
					switch(sc->Helper.Type)
					{
					case MODEL_HELPER+2:	// 유니리아
						SetAction(so, PLAYER_SKILL_LIGHTNING_ORB_UNI);
						break;
					case MODEL_HELPER+3:	// 디노란트
						SetAction(so, PLAYER_SKILL_LIGHTNING_ORB_DINO);
						break;
					case MODEL_HELPER+37:	// 펜릴	
						SetAction(so, PLAYER_SKILL_LIGHTNING_ORB_FENRIR);
						break;
					default:	// 기본
						SetAction(so, PLAYER_SKILL_LIGHTNING_ORB);
						break;
					}
				}
				break;
#ifdef PJH_ADD_MASTERSKILL
		case AT_SKILL_ALICE_DRAINLIFE_UP:
		case AT_SKILL_ALICE_DRAINLIFE_UP+1:
		case AT_SKILL_ALICE_DRAINLIFE_UP+2:
		case AT_SKILL_ALICE_DRAINLIFE_UP+3:
		case AT_SKILL_ALICE_DRAINLIFE_UP+4:
#endif				
			case AT_SKILL_ALICE_DRAINLIFE:		// Drain Life (드레인 라이프)
				{
					switch(sc->Helper.Type)
					{
					case MODEL_HELPER+2:	// 유니리아
						SetAction(so, PLAYER_SKILL_DRAIN_LIFE_UNI);
						break;
					case MODEL_HELPER+3:	// 디노란트
						SetAction(so, PLAYER_SKILL_DRAIN_LIFE_DINO);
						break;
					case MODEL_HELPER+37:	// 펜릴	
						SetAction(so, PLAYER_SKILL_DRAIN_LIFE_FENRIR);
						break;
					default:	// 기본
						SetAction(so, PLAYER_SKILL_DRAIN_LIFE);
						break;
					}
				}
				break;
				
				// ReceiveChainMagic() 에서 SetAction해주었으므로 안해주어도 됨.
				// 			case AT_SKILL_ALICE_CHAINLIGHTNING:
				// 				SetAction(so, PLAYER_SKILL_CHAIN_LIGHTNING);
				// 				break;
				
			case AT_SKILL_ALICE_WEAKNESS:
			case AT_SKILL_ALICE_ENERVATION:
				// 동작 설정
				switch(sc->Helper.Type)
				{
				case MODEL_HELPER+2:	// 유니리아
					SetAction(so, PLAYER_SKILL_SLEEP_UNI);
					break;
				case MODEL_HELPER+3:	// 디노란트
					SetAction(so, PLAYER_SKILL_SLEEP_DINO);
					break;
				case MODEL_HELPER+37:	// 펜릴	
					SetAction(so, PLAYER_SKILL_SLEEP_FENRIR);
					break;
				default:	// 기본
					SetAction(so, PLAYER_SKILL_SLEEP);
					break;
				}
				break;
					
			case AT_SKILL_SUMMON_EXPLOSION:
			case AT_SKILL_SUMMON_REQUIEM:
#ifdef ASG_ADD_SUMMON_RARGLE
			case AT_SKILL_SUMMON_POLLUTION:
#endif	// ASG_ADD_SUMMON_RARGLE
				{
					g_SummonSystem.CastSummonSkill(MagicNumber, sc, so, Data->PositionX, Data->PositionY);
				}
				break;
#ifdef YDG_ADD_SKILL_FLAME_STRIKE
				case AT_SKILL_FLAME_STRIKE:
					SetAction(so, PLAYER_SKILL_FLAMESTRIKE);
					break;
#endif	// YDG_ADD_SKILL_FLAME_STRIKE
#ifdef YDG_ADD_SKILL_GIGANTIC_STORM
				case AT_SKILL_GIGANTIC_STORM:
#ifdef YDG_ADD_SKILL_RIDING_ANIMATIONS
					switch(sc->Helper.Type)
					{
					case MODEL_HELPER+2:	// 유니리아
						SetAction(so, PLAYER_SKILL_GIGANTICSTORM_UNI);
						break;
					case MODEL_HELPER+3:	// 디노란트
						SetAction(so, PLAYER_SKILL_GIGANTICSTORM_DINO);
						break;
					case MODEL_HELPER+37:	// 펜릴	
						SetAction(so, PLAYER_SKILL_GIGANTICSTORM_FENRIR);
						break;
					default:	// 기본
						SetAction(so, PLAYER_SKILL_GIGANTICSTORM);
						break;
					}
#else	// YDG_ADD_SKILL_RIDING_ANIMATIONS
					SetAction(so, PLAYER_SKILL_GIGANTICSTORM);
#endif	// YDG_ADD_SKILL_RIDING_ANIMATIONS
					break;
#endif	// YDG_ADD_SKILL_GIGANTIC_STORM
#ifdef YDG_ADD_SKILL_LIGHTNING_SHOCK
#ifdef PJH_ADD_MASTERSKILL
		case AT_SKILL_LIGHTNING_SHOCK_UP:
		case AT_SKILL_LIGHTNING_SHOCK_UP+1:
		case AT_SKILL_LIGHTNING_SHOCK_UP+2:
		case AT_SKILL_LIGHTNING_SHOCK_UP+3:
		case AT_SKILL_LIGHTNING_SHOCK_UP+4:
#endif
				case AT_SKILL_LIGHTNING_SHOCK:
					SetAction(so, PLAYER_SKILL_LIGHTNING_SHOCK);
					break;
#endif	// YDG_ADD_SKILL_LIGHTNING_SHOCK
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
				case AT_SKILL_DRAGON_LOWER:
				{
					if(sc != Hero)
						g_CMonkSystem.RageCreateEffect(so, MagicNumber);

					g_CMonkSystem.SetRageSkillAni(MagicNumber, so);
				}
				break;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
				default:
					SetPlayerMagic(sc);
					break;
			}
			so->AnimationFrame = 0;
		}
	}
	else
	{
		SetPlayerAttack(sc);
		so->AnimationFrame = 0;
	}
	sc->AttackTime = 1;
	sc->TargetCharacter = -1;
	sc->AttackFlag = ATTACK_FAIL;
	sc->SkillX = Data->PositionX;
	sc->SkillY = Data->PositionY;
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x1E [ReceiveMagicContinue(%d)]", MagicNumber);
#endif // CONSOLE_DEBUG
	
	return ( TRUE);
}

// ChainLightning
void ReceiveChainMagic( BYTE *ReceiveBuffer )
{
	LPPRECEIVE_CHAIN_MAGIC pPacketData = (LPPRECEIVE_CHAIN_MAGIC)ReceiveBuffer;
	
	// 주인공 캐릭터
	CHARACTER* pSourceChar = &CharactersClient[FindCharacterIndex(pPacketData->wUserIndex)];
	OBJECT* pSourceObject = &pSourceChar->Object;
	OBJECT* pTempObject = NULL;
	
	//SetAction(pSourceObject, PLAYER_SKILL_CHAIN_LIGHTNING);
	
	switch(pSourceChar->Helper.Type)
	{
	case MODEL_HELPER+2:	// 유니리아
		SetAction(pSourceObject, PLAYER_SKILL_CHAIN_LIGHTNING_UNI);
		break;
	case MODEL_HELPER+3:	// 디노란트
		SetAction(pSourceObject, PLAYER_SKILL_CHAIN_LIGHTNING_DINO);
		break;
	case MODEL_HELPER+37:	// 펜릴	
		SetAction(pSourceObject, PLAYER_SKILL_CHAIN_LIGHTNING_FENRIR);
		break;
	default:	// 기본
		SetAction(pSourceObject, PLAYER_SKILL_CHAIN_LIGHTNING);
		break;
	}	
	
	pSourceChar->Skill = ((WORD)(pPacketData->MagicH)<<8) + pPacketData->MagicL;
	
	pTempObject = pSourceObject; 
	
	int iOffset = sizeof( PRECEIVE_CHAIN_MAGIC );
	for(int i=0 ; i<(int)(pPacketData->byCount) ; i++)
	{
		LPPRECEIVE_CHAIN_MAGIC_OBJECT pPacketData2 = (LPPRECEIVE_CHAIN_MAGIC_OBJECT)(ReceiveBuffer+iOffset);
		// 타겟 인덱스 저장
		CHARACTER *pTargetChar = &CharactersClient[FindCharacterIndex(pPacketData2->wTargetIndex)];
		OBJECT *pTargetObject = &pTargetChar->Object;
		
		if( pTempObject != pTargetObject && pTargetObject != NULL && pTargetObject->Live == true)
		{	
			// 이펙트 바로 생성
			// 라이트닝 (시작프레임 조절 필요)
			vec3_t vAngle;
			Vector( -60.f, 0.f, pSourceObject->Angle[2], vAngle);
			
			//SetPlayerStop(pTargetChar);
			
			// 라이트닝
			CreateEffect( MODEL_CHAIN_LIGHTNING, pSourceObject->Position, vAngle, pSourceObject->Light, i, 
				pTempObject, -1, 0, 0, 0, 0.f, pPacketData2->wTargetIndex );
		}
		
		pTempObject = pTargetObject;
		
		iOffset += sizeof( PRECEIVE_CHAIN_MAGIC_OBJECT );
	}
}

void ReceiveMagicPosition(BYTE *ReceiveBuffer,int Size)
{
	LPPRECEIVE_MAGIC_POSITIONS Data = (LPPRECEIVE_MAGIC_POSITIONS)ReceiveBuffer;
	int SourceKey = ((int)(Data->KeyH )<<8) + Data->KeyL;
	WORD MagicNumber = ((WORD)(Data->MagicH)<<8) + Data->MagicL;
	int Index = FindCharacterIndex(SourceKey);
	AttackPlayer = Index;
	
	CHARACTER *c = &CharactersClient[Index];
	OBJECT *o = &c->Object;
	CreateMagicShiny(c);
	PlayBuffer(SOUND_MAGIC);
	SetAction(o,PLAYER_SKILL_HELL);
	c->Skill = MagicNumber;
	c->AttackTime = 1;
	
	int Offset = sizeof(PRECEIVE_MAGIC_POSITIONS);
	for(int i=0;i<Data->Count;i++)
	{
		LPPRECEIVE_MAGIC_POSITION Data2 = (LPPRECEIVE_MAGIC_POSITION)(ReceiveBuffer+Offset);
		int TargetKey = ((int)(Data2->KeyH)<<8) + Data2->KeyL;
		CHARACTER *tc = &CharactersClient[FindCharacterIndex(TargetKey)];
		OBJECT *to = &tc->Object;
		if(rand()%2==0)
			SetPlayerShock(tc,tc->Hit);
		if(tc->Hit > 0)
		{
			vec3_t Light;
			Vector(1.f,0.6f,0.f,Light);
			CreatePoint(to->Position,tc->Hit,Light);
			if(TargetKey==HeroKey)
			{
				if(tc->Hit >= CharacterAttribute->Life)
					CharacterAttribute->Life = 0;
				else
					CharacterAttribute->Life -= tc->Hit;
			}
		}
		Offset += sizeof(PRECEIVE_MAGIC_POSITION);
	}
}


//////////////////////////////////////////////////////////////////////////
//	스킬 Count.
//////////////////////////////////////////////////////////////////////////
void ReceiveSkillCount ( BYTE* ReceiveBuffer )
{
	LPPRECEIVE_EX_SKILL_COUNT Data = (LPPRECEIVE_EX_SKILL_COUNT)ReceiveBuffer;
	int TargetKey = ((int)(Data->KeyH)<<8) + Data->KeyL;
	CHARACTER *tc = &CharactersClient[FindCharacterIndex(TargetKey)];
	OBJECT *to = &tc->Object;
	
	switch ( Data->m_byType )
	{
	case AT_SKILL_BLAST_HELL_BEGIN:
		break;
		
	case AT_SKILL_BLAST_HELL:
		to->m_bySkillCount = Data->m_byCount;
		break;
	}
}


///////////////////////////////////////////////////////////////////////////////
// 죽기
///////////////////////////////////////////////////////////////////////////////

// 내가 죽임
// 일반경험치
BOOL ReceiveDieExp(BYTE *ReceiveBuffer,BOOL bEncrypted)
{
	if ( !bEncrypted)
	{
		GO_DEBUG;
		SendHackingChecked( 0x00, 0x16);
		return ( FALSE);
	}
	
	LPPRECEIVE_DIE Data = (LPPRECEIVE_DIE)ReceiveBuffer;
	int     Key    = ((int)(Data->KeyH   )<<8) + Data->KeyL;
	DWORD   Exp    = ((DWORD)(Data->ExpH   )<<8) + Data->ExpL;
	int     Damage = ((int)(Data->DamageH)<<8) + Data->DamageL;
	int     Success = (Key>>15);
	Key &= 0x7FFF;
	
	int Index = FindCharacterIndex(Key);
	CHARACTER *c = &CharactersClient[Index];
	OBJECT *o = &c->Object;
	vec3_t Light;
	Vector(1.f,0.6f,0.f,Light);
	if(Success)
	{
		SetPlayerDie(c);
		CreatePoint(o->Position,Damage,Light);
	}
	else
	{
		Hero->AttackFlag = ATTACK_DIE;
		Hero->Damage = Damage;
		Hero->TargetCharacter = Index;
		CreatePoint(o->Position,Damage,Light);
	}
	c->Dead = true;
	c->Movement = false;
	

	if(IsMasterLevel(Hero->Class) == true)
	{
#ifdef LDK_ADD_SCALEFORM
		//gfxui 사용시 기존 ui 사용 안함
		if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
		{
			g_pMainFrame->SetPreExp_Wide(Master_Level_Data.lMasterLevel_Experince);
			g_pMainFrame->SetGetExp_Wide(Exp);	
		}
#else //LDK_ADD_SCALEFORM
		g_pMainFrame->SetPreExp_Wide(Master_Level_Data.lMasterLevel_Experince);
		g_pMainFrame->SetGetExp_Wide(Exp);	
#endif //LDK_ADD_SCALEFORM

		Master_Level_Data.lMasterLevel_Experince += Exp;
	}
	else
	{
#ifdef LDK_ADD_SCALEFORM
		//gfxui 사용시 기존 ui 사용 안함
		if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
		{
			g_pMainFrame->SetPreExp(CharacterAttribute->Experience);
			g_pMainFrame->SetGetExp(Exp);
		}
#else //LDK_ADD_SCALEFORM
		g_pMainFrame->SetPreExp(CharacterAttribute->Experience);
		g_pMainFrame->SetGetExp(Exp);
#endif //LDK_ADD_SCALEFORM

		CharacterAttribute->Experience += Exp;	
	}
	
	if(Exp > 0)
	{
		
		char Text[100];
		if(IsMasterLevel(Hero->Class) == true)
		{
			sprintf(Text,GlobalText[1750],Exp);
		}
		else
			sprintf(Text,GlobalText[486],Exp);
		g_pChatListBox->AddText("", Text, SEASON3B::TYPE_SYSTEM_MESSAGE);	
	}
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x16 [ReceiveDieExp : %d]", Exp);
#endif // CONSOLE_DEBUG	
	
	return ( TRUE);
}

// 마스터 레벨일때
BOOL ReceiveDieExpLarge(BYTE *ReceiveBuffer,BOOL bEncrypted)
{
	if ( !bEncrypted)
	{
		GO_DEBUG;
		SendHackingChecked( 0x00, 0x9C);
		return ( FALSE);
	}
	
	LPPRECEIVE_DIE2 Data = (LPPRECEIVE_DIE2)ReceiveBuffer;
	int     Key    = ((int)(Data->KeyH   )<<8) + Data->KeyL;
	DWORD   Exp    = ((DWORD)(Data->ExpH   )<<16) + Data->ExpL;
	int     Damage = ((int)(Data->DamageH)<<8) + Data->DamageL;
	int     Success = (Key>>15);
	Key &= 0x7FFF;
	
	int Index = FindCharacterIndex(Key);
	CHARACTER *c = &CharactersClient[Index];
	OBJECT *o = &c->Object;
	vec3_t Light;
	Vector(1.f,0.6f,0.f,Light);
	if(Success)
	{
		SetPlayerDie(c);
		CreatePoint(o->Position,Damage,Light);
	}
	else
	{
		Hero->AttackFlag = ATTACK_DIE;
		Hero->Damage = Damage;
		Hero->TargetCharacter = Index;
		CreatePoint(o->Position,Damage,Light);
	}
	c->Dead = true;
	c->Movement = false;

	if(IsMasterLevel(Hero->Class) == true)
	{
#ifdef LDK_ADD_SCALEFORM
		//gfxui 사용시 기존 ui 사용 안함
		if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
		{
			g_pMainFrame->SetPreExp_Wide(Master_Level_Data.lMasterLevel_Experince);
			g_pMainFrame->SetGetExp_Wide(Exp);	
		}
#else //LDK_ADD_SCALEFORM
		g_pMainFrame->SetPreExp_Wide(Master_Level_Data.lMasterLevel_Experince);
		g_pMainFrame->SetGetExp_Wide(Exp);	
#endif //LDK_ADD_SCALEFORM

		Master_Level_Data.lMasterLevel_Experince += Exp;
	}
	else
	{
#ifdef LDK_ADD_SCALEFORM
		//gfxui 사용시 기존 ui 사용 안함
		if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
		{
			g_pMainFrame->SetPreExp(CharacterAttribute->Experience);
			g_pMainFrame->SetGetExp(Exp);	
		}
#else //LDK_ADD_SCALEFORM
		g_pMainFrame->SetPreExp(CharacterAttribute->Experience);
		g_pMainFrame->SetGetExp(Exp);	
#endif //LDK_ADD_SCALEFORM

		CharacterAttribute->Experience += Exp;	
	}
	
	if(Exp > 0)
	{
		char Text[100];

		if(IsMasterLevel(Hero->Class) == true)
		{
			sprintf(Text,GlobalText[1750],Exp);
		}
		else
			sprintf(Text,GlobalText[486],Exp);
		g_pChatListBox->AddText("", Text, SEASON3B::TYPE_SYSTEM_MESSAGE);
	}
	
	return ( TRUE);
}


void FallingStartCharacter ( CHARACTER* c, OBJECT* o )
{
    //  튕겨서 떨어뜨린다.
    BYTE positionX = (BYTE)(o->Position[0]/TERRAIN_SCALE);
    BYTE positionY = (BYTE)(o->Position[1]/TERRAIN_SCALE);
    int WallIndex = TERRAIN_INDEX_REPEAT( positionX, positionY );
    int Wall = TerrainWall[WallIndex]&TW_ACTION;
	
    o->m_bActionStart = false;
	
    if ( InChaosCastle()==true && (TerrainWall[WallIndex]&TW_NOGROUND)==TW_NOGROUND )
    {
        c->StormTime = 0;
        o->m_bActionStart = true;
        o->Gravity  = rand()%10+10.f;
        o->Velocity-= rand()%3+3.f;
		
        Vector ( 0.f, 0.f, 1.f, o->Direction );
        VectorCopy ( o->Position, o->m_vDeadPosition );
		
        PlayBuffer ( SOUND_CHAOS_FALLING );
    }
    else if ( Wall==TW_ACTION )
    {
        c->StormTime = 0;
        o->m_bActionStart = true;
        o->Gravity = rand()%10+10.f;
        o->Velocity= rand()%20+20.f;
        float angle = rand()%10+85.f;
		
        if ( (TerrainWall[WallIndex+1]&TW_NOGROUND)==TW_NOGROUND )
        {
            Vector ( 0.f, 0.f, -angle, o->m_vDownAngle );
        }
        else if ( (TerrainWall[WallIndex-1]&TW_NOGROUND)==TW_NOGROUND )
        {
            Vector ( 0.f, 0.f, angle, o->m_vDownAngle );
        }
		
        o->Angle[2] = o->m_vDownAngle[2];
        Vector ( rand()%6+8.f, -rand()%2+13.f, 5.f, o->Direction );
        VectorCopy ( o->Position, o->m_vDeadPosition );
    }
	
    if ( c==Hero )
    {
        matchEvent::ClearMatchInfo();
    }
}

//다른플레이어가 공격해서 케릭터나 몬스터 죽음
void ReceiveDie(BYTE *ReceiveBuffer,int Size)
{
	LPPHEADER_DEFAULT_DIE Data = (LPPHEADER_DEFAULT_DIE)ReceiveBuffer;
	
    int Key = ((int)(Data->KeyH )<<8) + Data->KeyL;
	
	int Index = FindCharacterIndex(Key);
	
	CHARACTER *c = &CharactersClient[Index];
	OBJECT *o = &c->Object;
	c->Dead = true;
	c->Movement = false;

	WORD SkillType = ((int)(Data->MagicH )<<8) + Data->MagicL;

#ifdef CSK_HACK_TEST
	g_pHackTest->ReceiveDie(c, o, SkillType, Key);
#endif // CSK_HACK_TEST
	
	//	죽인 스킬.
	c->m_byDieType = SkillType;	//	( 0:기본, n:스킬번호 ).
	
    //  블러드 캐슬에서 죽는 몬스터들을 떨어뜨린다.
    if ( InBloodCastle() == true )
    {
        //  튕겨서 떨어뜨린다.
        FallingStartCharacter ( c, o );
    }
    else if ( InChaosCastle()==true )
    {
        //  튕겨서 떨어뜨린다.
        FallingStartCharacter ( c, o );
    }
    else
    {
#ifdef YDG_ADD_CS5_REVIVAL_CHARM
		if (c == Hero && g_PortalMgr.IsPortalUsable())
		{
			g_PortalMgr.SaveRevivePosition();	// 죽을때 위치 저장
		}
#endif	// YDG_ADD_CS5_REVIVAL_CHARM

        o->m_bActionStart = false;
		
		//	나를 죽인 스킬.
		switch ( SkillType )
		{
		case AT_SKILL_BLAST_HELL:
		case AT_SKILL_COMBO:
			o->m_bActionStart = true;
			c->StormTime= 0;
			o->Velocity	= (rand()%5+10.f)*0.1f;
			o->m_bySkillCount = 0;
			
			Vector ( 0.f, rand()%15+40.f, 0.f, o->Direction );
			VectorCopy ( o->Position, o->m_vDeadPosition );
			break;
		}
		
		if ( SkillType==AT_SKILL_BLAST_HELL || SkillType==AT_SKILL_COMBO )
			{
				int TKey   = ((int)(Data->TKeyH)<<8) + Data->TKeyL;
				int TIndex = FindCharacterIndex ( TKey );
				CHARACTER* tc = &CharactersClient[TIndex];
				OBJECT*    to = &tc->Object;
				
				o->Angle[2] = CreateAngle ( o->Position[0], o->Position[1], to->Position[0], to->Position[1] );
				
				VectorCopy ( o->Angle, o->HeadAngle );
			}
    }
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x17 [ReceiveDie(%d)]", Key);
#endif // CONSOLE_DEBUG
}

///////////////////////////////////////////////////////////////////////////////
// 아이템
///////////////////////////////////////////////////////////////////////////////

void ReceiveCreateItemViewport( BYTE *ReceiveBuffer )
{
	LPPWHEADER_DEFAULT_WORD Data = (LPPWHEADER_DEFAULT_WORD)ReceiveBuffer;
	int Offset = sizeof(PWHEADER_DEFAULT_WORD);
	for(int i=0;i<Data->Value;i++)
	{
		LPPCREATE_ITEM Data2 = (LPPCREATE_ITEM)(ReceiveBuffer+Offset);
		vec3_t Position;
		Position[0]  = (float)(Data2->PositionX+0.5f)*TERRAIN_SCALE;
		Position[1]  = (float)(Data2->PositionY+0.5f)*TERRAIN_SCALE;
		int Key      = ((int)(Data2->KeyH )<<8) + Data2->KeyL;
		int CreateFlag = (Key>>15);
		Key &= 0x7FFF;
		if(Key<0 || Key>=MAX_ITEMS)
			Key = 0;
		CreateItem(&Items[Key],Data2->Item,Position,CreateFlag);
		int Type = ConvertItemType(Data2->Item);
		if(Type==ITEM_POTION+15)//돈
		{
			Offset += sizeof(PCREATE_ITEM);
		}
		else//아이템
		{
			Offset += sizeof(PCREATE_ITEM);
		}
	}
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x20 [ReceiveCreateItemViewport]");
#endif // CONSOLE_DEBUG
}

void ReceiveDeleteItemViewport( BYTE *ReceiveBuffer )
{
	LPPWHEADER_DEFAULT_WORD Data = (LPPWHEADER_DEFAULT_WORD)ReceiveBuffer;
	int Offset = sizeof(PWHEADER_DEFAULT_WORD);
	for(int i=0;i<Data->Value;i++)
	{
		LPPDELETE_CHARACTER Data2 = (LPPDELETE_CHARACTER)(ReceiveBuffer+Offset);
		int Key = ((int)(Data2->KeyH)<<8) + Data2->KeyL;
		if(Key<0 || Key>=MAX_ITEMS)
			Key = 0;
		Items[Key].Object.Live = false;
		Offset += sizeof(PDELETE_CHARACTER);
	}
}

static  const   BYTE    NOT_GET_ITEM = 0xff;    //  줏을수 없다.
static  const   BYTE    GET_ITEM_ZEN = 0xfe;    //  젠을 줏는다.
static  const   BYTE    GET_ITEM_MULTI=0xfd;    //  중복되는 아이템을 줏는다.
extern int ItemKey;		// ★ 아이템 숨기기에서 사용할 변수
void ReceiveGetItem( BYTE *ReceiveBuffer )
{
	LPPRECEIVE_GET_ITEM Data = (LPPRECEIVE_GET_ITEM)ReceiveBuffer;
	if ( Data->Result==NOT_GET_ITEM )
	{	// 줏을 수 없다 메시지 표시 필요
	}
	else
	{
		if ( Data->Result==GET_ITEM_ZEN )
		{
			char szMessage[128];
			int backupGold = CharacterMachine->Gold;
			CharacterMachine->Gold = (Data->Item[0]<<24) + (Data->Item[1]<<16) + (Data->Item[2]<<8) + (Data->Item[3]);
#ifdef KJH_FIX_GET_ZEN_SYSTEM_TEXT
			int getGold = CharacterMachine->Gold - backupGold;
			
			if( getGold > 0 )
			{
#if SELECTED_LANGUAGE == LANGUAGE_CHINESE
				sprintf(szMessage, "%s %d %s", GlobalText[918], getGold, GlobalText[224]);
#else //SELECTED_LANGUAGE == LANGUAGE_CHINESE
				sprintf(szMessage, "%d %s %s", getGold, GlobalText[224], GlobalText[918]);
#endif // SELECTED_LANGUAGE == LANGUAGE_CHINESE
				g_pChatListBox->AddText("", szMessage, SEASON3B::TYPE_SYSTEM_MESSAGE);
			}
#else // KJH_FIX_GET_ZEN_SYSTEM_TEXT
            //  아이템 space로 자동 먹기
            if ( Items[ItemKey].Object.Live )
            {
#if SELECTED_LANGUAGE == LANGUAGE_CHINESE
				sprintf(szMessage, "%s %d %s", GlobalText[918], Items[ItemKey].Item.Level, GlobalText[224]);
#else //SELECTED_LANGUAGE == LANGUAGE_CHINESE
				sprintf(szMessage, "%d %s %s", Items[ItemKey].Item.Level, GlobalText[224], GlobalText[918]);
#endif // SELECTED_LANGUAGE == LANGUAGE_CHINESE
				g_pChatListBox->AddText("", szMessage, SEASON3B::TYPE_SYSTEM_MESSAGE);
			}
#ifdef CSK_EVENT_CHERRYBLOSSOM
			else
			{
				int getGold = CharacterMachine->Gold - backupGold;
				
				if( getGold > 0 )
				{
#if SELECTED_LANGUAGE == LANGUAGE_CHINESE
					sprintf(szMessage, "%s %d %s", GlobalText[918], getGold, GlobalText[224]);
#else //SELECTED_LANGUAGE == LANGUAGE_CHINESE
					sprintf(szMessage, "%d %s %s", getGold, GlobalText[224], GlobalText[918]);
#endif // SELECTED_LANGUAGE == LANGUAGE_CHINESE
					g_pChatListBox->AddText("", szMessage, SEASON3B::TYPE_SYSTEM_MESSAGE);
				}
			}
#endif //CSK_EVENT_CHERRYBLOSSOM
#endif // KJH_FIX_GET_ZEN_SYSTEM_TEXT
		}
		else
		{
            //  중복되는 아이템. 인벤에 아이템을 넣지 않고, 필드에 있는 아이템을 제거한다.
            if ( Data->Result!=GET_ITEM_MULTI )
            {
				if(Data->Result >= MAX_EQUIPMENT_INDEX && Data->Result < MAX_MY_INVENTORY_INDEX)
					g_pMyInventory->InsertItem(Data->Result-MAX_EQUIPMENT_INDEX, Data->Item);
            }
			
            //  아이템 space로 자동 먹기.
            char szItem[64] = {0, };
			int level = (Items[ItemKey].Item.Level>>3)&15;
			GetItemName(Items[ItemKey].Item.Type, level, szItem);
			
			char szMessage[128];
#if SELECTED_LANGUAGE == LANGUAGE_CHINESE
			sprintf(szMessage, "%s %s", GlobalText[918], szItem);
#else
			sprintf(szMessage, "%s %s", szItem, GlobalText[918]);
#endif // SELECTED_LANGUAGE == LANGUAGE_CHINESE
			g_pChatListBox->AddText("", szMessage, SEASON3B::TYPE_SYSTEM_MESSAGE);			
        }
#ifdef FOR_WORK
		// ★ 아이템 숨기기 -_-
		Items[ItemKey].Object.Live = false;
#endif
		// 아이템 주울때 나오는 소리
		int Type = ConvertItemType(Data->Item);
		if(Type==ITEM_POTION+13 || Type==ITEM_POTION+14 || Type==ITEM_POTION+16 || Type==ITEM_WING+15 || Type==ITEM_POTION+22 
			|| Type==INDEX_COMPILED_CELE || Type==INDEX_COMPILED_SOUL || Type==ITEM_POTION+31)
			PlayBuffer(SOUND_JEWEL01,&Hero->Object);
		else if(Type == ITEM_POTION+41)
			PlayBuffer(SOUND_JEWEL02,&Hero->Object);
		else
			PlayBuffer(SOUND_GET_ITEM01,&Hero->Object);
	}
    SendGetItem = -1;
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x22 [ReceiveGetItem(%d)]", Data->Result);
#endif // CONSOLE_DEBUG
}

void ReceiveDropItem( BYTE *ReceiveBuffer )
{
	LPPHEADER_DEFAULT_KEY Data = (LPPHEADER_DEFAULT_KEY)ReceiveBuffer;
	if(Data->KeyH)
	{
		if(Data->KeyL < 12)
		{
			g_pMyInventory->UnequipItem(Data->KeyL);
		}
		else
		{
			g_pMyInventory->DeleteItem( Data->KeyL-MAX_EQUIPMENT_INDEX );
		}

		SEASON3B::CNewUIInventoryCtrl::DeletePickedItem();
	}
	else
	{
		SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
	}

    SendDropItem = -1;
}


BOOL g_bPacketAfter_EquipmentItem = FALSE;
BYTE g_byPacketAfter_EquipmentItem[256];

void ReceiveTradeExit( BYTE *ReceiveBuffer );

BOOL ReceiveEquipmentItem(BYTE *ReceiveBuffer, BOOL bEncrypted)
{
 	if ( !bEncrypted)
	{
		GO_DEBUG;
		SendHackingChecked( 0x00, 0x16);
		return ( FALSE);
	}
	
	EquipmentItem = false;
	LPPHEADER_DEFAULT_SUBCODE_ITEM Data = (LPPHEADER_DEFAULT_SUBCODE_ITEM)ReceiveBuffer;
	if(Data->SubCode != 255)
	{
		SEASON3B::CNewUIPickedItem* pPickedItem = SEASON3B::CNewUIInventoryCtrl::GetPickedItem();
		int iSourceIndex = g_pMyShopInventory->GetSourceIndex();
		if(pPickedItem)
		{
			if(pPickedItem->GetOwnerInventory() == g_pMyInventory->GetInventoryCtrl())
			{	
				iSourceIndex += MAX_EQUIPMENT_INDEX;
			}
			else if(pPickedItem->GetOwnerInventory() == g_pMyShopInventory->GetInventoryCtrl())
			{
				iSourceIndex += MAX_MY_INVENTORY_INDEX;
			}
		}
		
		if(iSourceIndex >= MAX_MY_INVENTORY_INDEX)
		{
			int price = 0;
			if(GetPersonalItemPrice(iSourceIndex, price, g_IsPurchaseShop))
			{
				RemovePersonalItemPrice(iSourceIndex, g_IsPurchaseShop);
				if(Data->Index >= MAX_MY_INVENTORY_INDEX)
				{
					AddPersonalItemPrice(Data->Index, price, g_IsPurchaseShop);
				}
			}
		}
		
		if(Data->SubCode == 0)
		{
			SEASON3B::CNewUIInventoryCtrl::DeletePickedItem();
			
			int itemindex = Data->Index;
			
			if(itemindex >= 0 && itemindex < MAX_EQUIPMENT_INDEX)
			{
				g_pMyInventory->EquipItem(itemindex, Data->Item);
			}
			else if(itemindex >= MAX_EQUIPMENT_INDEX && itemindex < MAX_MY_INVENTORY_INDEX)
			{
				g_pStorageInventory->ProcessStorageItemAutoMoveSuccess();
				itemindex = itemindex - MAX_EQUIPMENT_INDEX;
				g_pMyInventory->InsertItem(itemindex, Data->Item);
			}
			else if(itemindex >= (MAX_EQUIPMENT_INDEX + MAX_INVENTORY) && itemindex < MAX_MY_SHOP_INVENTORY_INDEX)
			{
				itemindex = itemindex - (MAX_EQUIPMENT_INDEX + MAX_INVENTORY);
				g_pMyShopInventory->InsertItem(itemindex, Data->Item);
			}
		}
		else if (1 == Data->SubCode)
		{
			g_pTrade->ProcessToReceiveTradeItems(Data->Index, Data->Item);
		}
		else if (2 == Data->SubCode)
		{
			g_pStorageInventory->ProcessToReceiveStorageItems(Data->Index, Data->Item);
		}
		if(Data->SubCode == 3 || (Data->SubCode >= 5 && Data->SubCode <= 8)
#ifdef CSK_CHAOS_CARD
			|| (Data->SubCode == 9) 
#endif //CSK_CHAOS_CARD
#ifdef CSK_EVENT_CHERRYBLOSSOM
			|| (Data->SubCode == 10)
#endif //CSK_EVENT_CHERRYBLOSSOM
#ifdef ADD_SOCKET_MIX
			|| (Data->SubCode >= REQUEST_EQUIPMENT_EXTRACT_SEED_MIX && Data->SubCode <= REQUEST_EQUIPMENT_DETACH_SOCKET_MIX)
#endif	// ADD_SOCKET_MIX
			)	// 모든 조합
		{
			SEASON3B::CNewUIInventoryCtrl::DeletePickedItem();
			if(Data->Index >= 0 && Data->Index < MAX_MIX_INVENTORY)
				g_pMixInventory->InsertItem(Data->Index, Data->Item);
		}
#ifdef LEM_ADD_LUCKYITEM
		else if ( 15 == Data->SubCode || 16 == Data->SubCode )
		{
			g_pLuckyItemWnd->GetResult(1, Data->Index, Data->Item);
		}
#endif // LEM_ADD_LUCKYITEM
		
		PlayBuffer(SOUND_GET_ITEM01);
	}
	else
	{
		SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
		g_pStorageInventory->ProcessStorageItemAutoMoveFailure();
	}
	
	if ( g_bPacketAfter_EquipmentItem)
	{
		ReceiveTradeExit( g_byPacketAfter_EquipmentItem);
		g_bPacketAfter_EquipmentItem = FALSE;
	}
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x24 [ReceiveEquipmentItem(%d %d)]", Data->SubCode, Data->Index);
#endif // CONSOLE_DEBUG
	
	return ( TRUE);
}

void ReceiveModifyItem( BYTE *ReceiveBuffer )
{
	LPPHEADER_DEFAULT_SUBCODE_ITEM Data = (LPPHEADER_DEFAULT_SUBCODE_ITEM)ReceiveBuffer;
	
	if(SEASON3B::CNewUIInventoryCtrl::GetPickedItem())
		SEASON3B::CNewUIInventoryCtrl::DeletePickedItem();
	
	int itemindex = Data->Index - MAX_EQUIPMENT_INDEX;
	if(g_pMyInventory->FindItem(itemindex))
		g_pMyInventory->DeleteItem(itemindex);
	
	g_pMyInventory->InsertItem(itemindex, Data->Item);
	
	int iType = ConvertItemType(Data->Item);
	if(iType == ITEM_POTION+28
#ifdef YDG_ADD_DOPPELGANGER_ITEM
		|| iType == ITEM_POTION+111	// 차원의 마경
#endif	// YDG_ADD_DOPPELGANGER_ITEM
		)
	{
		PlayBuffer(SOUND_KUNDUN_ITEM_SOUND);
	}
#ifdef LDK_ADD_GAMBLE_SYSTEM
#ifdef LEM_FIX_JP0714_JEWELMIXSOUND_INGAMBLESYSTEM
	else if( iType == ITEM_POTION+11 )
	{
		PlayBuffer(SOUND_JEWEL01);
	}
#endif // LEM_FIX_JP0714_JEWELMIXSOUND_INGAMBLESYSTEM
	// 사운드 필요시 추가하자.
	else if(GambleSystem::Instance().IsGambleShop())
	{

	}
#endif //LDK_ADD_GAMBLE_SYSTEM
	else
	{
		PlayBuffer(SOUND_JEWEL01);
	}
}

///////////////////////////////////////////////////////////////////////////////
// NPC와 대화, 사기, 팔기
///////////////////////////////////////////////////////////////////////////////

//대화 시작
BOOL ReceiveTalk(BYTE *ReceiveBuffer, BOOL bEncrypted)
{
	if ( !bEncrypted)
	{
		GO_DEBUG;
		SendHackingChecked( 0x00, 0x30);
		return ( FALSE);
	}
	LPPHEADER_DEFAULT Data = (LPPHEADER_DEFAULT)ReceiveBuffer;
	
	g_pNewUISystem->HideAll();
	
	switch(Data->Value)
	{
	case 2: //  창고.
		g_pNewUISystem->Show(SEASON3B::INTERFACE_STORAGE);
		break;
		
	case 3: // 카오스?
		g_MixRecipeMgr.SetMixType(SEASON3A::MIXTYPE_GOBLIN_NORMAL);
		g_pNewUISystem->Show(SEASON3B::INTERFACE_MIXINVENTORY);
		//BYTE *pbyChaosRate = ( &Data->Value) + 1;
		//int iDummyRate[6];	// 광장표 확률을 서버에서 받으나 사용하지 않고 버림
		//for ( int i = 0; i < 6; ++i)
		//	iDummyRate[i] = ( int)pbyChaosRate[i];	// 광장표 확률을 서버에서 받으나 사용하지 않고 버림(스크립트사용)
		break;
		
	case 4: // 악마의 광장 가는 창
		g_pNewUISystem->Show( SEASON3B::INTERFACE_DEVILSQUARE );
		break;
		
    case 5: // 서버 분할창.
		g_pUIManager->Open( ::INTERFACE_SERVERDIVISION );
		break;
		
	case 6:	// 블러드 캐슬 가는 창	
		g_pNewUISystem->Show( SEASON3B::INTERFACE_BLOODCASTLE );
		break;
		
    case 7: //  조련사 NPC 조합.
		SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CTrainerMenuMsgBoxLayout));
		break;
		
	case INDEX_NPC_LAHAP:
		{
			if(COMGEM::isAble())
			{
				g_pNewUISystem->HideAll();
				SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CGemIntegrationMsgBoxLayout));
			}
		}
		break;
		
    case 0x0C : //  원로원.
		g_pNewUISystem->Show(SEASON3B::INTERFACE_SENATUS);
        break;
		
    case 0x0D : //  근위병.
		SendRequestBCStatus();	// 공성정보 요청
        break;
	case 0x11:	// 엘피스
		{
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CElpisMsgBoxLayout));
			// 확률을 서버에서 받으나 사용하지 않고 버림(스크립트사용)
			// 			BYTE *pbyChaosRate = ( &Data->Value) + 1;
			// 			g_pUIJewelHarmony->SetMixSuccessRate(pbyChaosRate);
		}
		break;
	case 0x12:	// 오스본
		{
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::COsbourneMsgBoxLayout));
			// 확률을 서버에서 받으나 사용하지 않고 버림(스크립트사용)
			// 			BYTE *pbyChaosRate = ( &Data->Value) + 1;
			// 			g_pUIJewelHarmony->SetMixSuccessRate(pbyChaosRate);
		}
		break;
	case 0x13:	// 제리든
		{
			g_MixRecipeMgr.SetMixType(SEASON3A::MIXTYPE_JERRIDON);
			g_pNewUISystem->Show(SEASON3B::INTERFACE_MIXINVENTORY);
			// 확률을 서버에서 받으나 사용하지 않고 버림(스크립트사용)
			// 			BYTE *pbyChaosRate = ( &Data->Value) + 1;
			// 			g_pUIJewelHarmony->SetMixSuccessRate(pbyChaosRate);
		}
		break;
	case 0x14:
		{
			g_pNewUISystem->Show(SEASON3B::INTERFACE_CURSEDTEMPLE_NPC);
			
			BYTE *cursedtempleenterinfo = ( &Data->Value) + 1;
			g_pCursedTempleEnterWindow->SetCursedTempleEnterInfo(cursedtempleenterinfo);
		}
		break;
#ifdef CSK_CHAOS_CARD
	case 0x15:	// 카오스 카드 믹스창
		{
			g_MixRecipeMgr.SetMixType(SEASON3A::MIXTYPE_CHAOS_CARD);
			g_pNewUISystem->Show(SEASON3B::INTERFACE_MIXINVENTORY);
		}
		break;
#endif //CSK_CHAOS_CARD
#ifdef CSK_EVENT_CHERRYBLOSSOM
	case 0x16:	// 벚꽃 이벤트 믹스창
		{
			g_MixRecipeMgr.SetMixType(SEASON3A::MIXTYPE_CHERRYBLOSSOM);
			g_pNewUISystem->Show(SEASON3B::INTERFACE_MIXINVENTORY);
		}
		break;
#endif //CSK_EVENT_CHERRYBLOSSOM
#ifdef ADD_SOCKET_MIX
	case 0x17:	// 시드마스터
		{
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CSeedMasterMenuMsgBoxLayout));
		}
		break;
	case 0x18:	// 시드연구가
		{
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CSeedInvestigatorMenuMsgBoxLayout));
		}
		break;
#endif	// ADD_SOCKET_MIX
#ifdef PSW_ADD_RESET_CHARACTER_POINT
	case 0x19:	// 스텟 초기화
		{
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CResetCharacterPointMsgBoxLayout));
		}
		break;
#endif //PSW_ADD_RESET_CHARACTER_POINT
#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
	case 0x20:	// 7주년이벤트 델가도NPC
		{
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CDelgardoMainMenuMsgBoxLayout));
		}
		break;
#endif //KJH_PBG_ADD_SEVEN_EVENT_2008
#ifdef YDG_ADD_NEW_DUEL_NPC
	case 0x21:	// 결투장 문지기 NPC 타이투스
		{
			g_pNewUISystem->Show(SEASON3B::INTERFACE_DUELWATCH);
		}
		break;
#endif	// YDG_ADD_NEW_DUEL_NPC
#ifdef LDK_ADD_GAMBLE_SYSTEM
	case 0x22:
		{
			GambleSystem::Instance().SetGambleShop();
			g_pNewUISystem->Show(SEASON3B::INTERFACE_NPCSHOP);
		}
		break;
#endif //LDK_ADD_GAMBLE_SYSTEM
#ifdef YDG_ADD_DOPPELGANGER_PROTOCOLS
	case 0x23:	// 도플갱어 NPC 루가드
		{
			g_pNewUISystem->Show(SEASON3B::INTERFACE_DOPPELGANGER_NPC);
			BYTE * pbtRemainTime = (&Data->Value) + 1;	// 남은 시간
			g_pDoppelGangerWindow->SetRemainTime(*pbtRemainTime);
		}
		break;
#endif	// YDG_ADD_DOPPELGANGER_PROTOCOLS
#ifdef LDK_ADD_EMPIREGUARDIAN_PROTOCOLS
	case 0x24:	// 제린트 npc - 임시 번호
		{
			g_pNewUISystem->Show(SEASON3B::INTERFACE_EMPIREGUARDIAN_NPC);
		}
		break;
#endif //LDK_ADD_EMPIREGUARDIAN_PROTOCOLS
#ifdef LDS_ADD_SERVERPROCESSING_UNITEDMARKETPLACE
	case 0x25:	// 통합시장 NPC 줄리아
		{
			g_pNewUISystem->Show(SEASON3B::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA);
		}
		break;
#endif // LDS_ADD_SERVERPROCESSING_UNITEDMARKETPLACE
#ifdef LEM_ADD_LUCKYITEM	// 럭키아이템 NPC 베리얼
	case 0x26:	
		{
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CLuckyTradeMenuMsgBoxLayout));
		}
		break;
#endif // LEM_ADD_LUCKYITEM
	default:
		{
			// Data->Value 값이 0이면 NPCSHOP 오픈
			g_pNewUISystem->Show(SEASON3B::INTERFACE_NPCSHOP);
		}
		break;
	}
    PlayBuffer(SOUND_CLICK01);
    PlayBuffer(SOUND_INTERFACE01);
#ifndef FOR_WORK		// 창모드에서 마우스 위치 바뀌는것 막았습니다.
#ifdef WINDOWMODE
	if (g_bUseWindowMode == FALSE)
	{
#endif	// WINDOWMODE
		int x = 260*MouseX/640;
		SetCursorPos((x)*WindowWidth/640,(MouseY)*WindowHeight/480);
#ifdef WINDOWMODE
	}
#endif	// WINDOWMODE
#endif	// FOR_WORK
	
	return ( TRUE);
}

void ReceiveBuy( BYTE *ReceiveBuffer )
{
	LPPHEADER_DEFAULT_ITEM Data = (LPPHEADER_DEFAULT_ITEM)ReceiveBuffer;
	if(Data->Index != 255)
	{
		if(Data->Index >= MAX_EQUIPMENT_INDEX && Data->Index < MAX_MY_INVENTORY_INDEX)
		{
			g_pMyInventory->InsertItem(Data->Index-MAX_EQUIPMENT_INDEX, Data->Item);
		}
		else
		{
			// 잘못된 인덱스가 넘어온 경우다. 서버팀에 문의하세요.!
#ifdef _DEBUG
			__asm { int 3 };
#endif // _DEBUG
		}
		
		PlayBuffer(SOUND_GET_ITEM01);
	}
#ifdef LDK_FIX_RECEIVEBUYSELL_RESULT_254
	//구매 횟수 이상 버그 .. receivebuy result값이 254면 hideAll(2008.08.11)
	if( Data->Index == 0xfe )
	{
		g_pNewUISystem->HideAll();

		g_pChatListBox->AddText(Hero->ID,GlobalText[732], SEASON3B::TYPE_ERROR_MESSAGE);
	}
#endif //LDK_FIX_RECEIVEBUYSELL_RESULT_254
	BuyCost = 0;
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x32 [ReceiveBuy(%d)]", Data->Index);
#endif // CONSOLE_DEBUG
}

void ReceiveSell( BYTE *ReceiveBuffer )
{
	LPPRECEIVE_GOLD Data = (LPPRECEIVE_GOLD)ReceiveBuffer;
	if(Data->Flag != 0)
	{
#ifdef PSW_FIX_EQUIREEQUIPITEMBUY
		if( Data->Flag == 0xff )
		{
			SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();

			g_pChatListBox->AddText(Hero->ID,GlobalText[733], SEASON3B::TYPE_ERROR_MESSAGE);
		}
#ifdef LDK_FIX_RECEIVEBUYSELL_RESULT_254
		//구매 횟수 이상 버그 .. receivebuy result값이 254면 hideAll(2008.08.11)
		else if( Data->Flag == 0xfe )
		{
			g_pNewUISystem->HideAll();

			g_pChatListBox->AddText(Hero->ID,GlobalText[733], SEASON3B::TYPE_ERROR_MESSAGE);
		}
#endif //LDK_FIX_RECEIVEBUYSELL_RESULT_254
		else 
#endif //PSW_FIX_EQUIREEQUIPITEMBUY
		{
			SEASON3B::CNewUIInventoryCtrl::DeletePickedItem();
			
			CharacterMachine->Gold = Data->Gold;
			
			PlayBuffer(SOUND_GET_ITEM01);

#ifdef CSK_FIX_HIGHVALUE_MESSAGEBOX
			g_pNPCShop->SetSellingItem(false);
#endif // CSK_FIX_HIGHVALUE_MESSAGEBOX
		}
	}
	else
	{
		SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
	}
}

void ReceiveRepair( BYTE *ReceiveBuffer )
{
	LPPRECEIVE_REPAIR_GOLD Data = (LPPRECEIVE_REPAIR_GOLD)ReceiveBuffer;
	
    if(Data->Gold != 0)
	{
		CharacterMachine->Gold = Data->Gold;
        CharacterMachine->CalculateAll();
        PlayBuffer(SOUND_REPAIR);
	}
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x34 [ReceiveRepair(%d)]", Data->Gold);
#endif // CONSOLE_DEBUG
}


///////////////////////////////////////////////////////////////////////////////
// 레벨업
///////////////////////////////////////////////////////////////////////////////

void ReceiveLevelUp( BYTE *ReceiveBuffer )
{
	LPPRECEIVE_LEVEL_UP Data = (LPPRECEIVE_LEVEL_UP)ReceiveBuffer;
	CharacterAttribute->Level = Data->Level;
	CharacterAttribute->LevelUpPoint = Data->LevelUpPoint;
	CharacterAttribute->LifeMax = Data->MaxLife;
	CharacterAttribute->ManaMax = Data->MaxMana;
	CharacterAttribute->Life    = Data->MaxLife;
	CharacterAttribute->Mana    = Data->MaxMana;
	CharacterAttribute->ShieldMax = Data->MaxShield;
	CharacterAttribute->SkillManaMax = Data->SkillManaMax;
    
    //  추가되는 스텟 정보.
    CharacterAttribute->AddPoint		= Data->AddPoint;
    CharacterAttribute->MaxAddPoint		= Data->MaxAddPoint;
	//	열매 감소 포인트 정보
	CharacterAttribute->wMinusPoint		= Data->wMinusPoint;		// 현재 감소 포인트
    CharacterAttribute->wMaxMinusPoint	= Data->wMaxMinusPoint;		// 최대 감소 포인트
	
	unicode::t_char szText[256] = {NULL, };
	WORD iExp = CharacterAttribute->NextExperince - CharacterAttribute->Experience;
	sprintf(szText,GlobalText[486], iExp);
	g_pChatListBox->AddText("", szText, SEASON3B::TYPE_SYSTEM_MESSAGE);

    CharacterMachine->CalculateNextExperince();
	
	OBJECT *o = &Hero->Object;

	if(IsMasterLevel(Hero->Class) == true)
	{
		//		OBJECT *o = &Hero->Object;
		
		CreateJoint(BITMAP_FLARE,o->Position,o->Position,o->Angle,45,o,80,2);
		for ( int i=0; i<19; ++i )
		{
			CreateJoint(BITMAP_FLARE,o->Position,o->Position,o->Angle,46,o,80,2);
		}
	}
	else
	{
		for ( int i=0; i<15; ++i )
		{
			CreateJoint(BITMAP_FLARE,o->Position,o->Position,o->Angle,0,o,40,2);
		}
		CreateEffect(BITMAP_MAGIC+1,o->Position,o->Angle,o->Light,0,o);
	}
    PlayBuffer(SOUND_LEVEL_UP);
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x05 [ReceiveLevelUp]");
#endif // CONSOLE_DEBUG
}

void ReceiveAddPoint( BYTE *ReceiveBuffer )
{
    LPPRECEIVE_ADD_POINT Data = (LPPRECEIVE_ADD_POINT)ReceiveBuffer;
	if(Data->Result>>4)
	{
		CharacterAttribute->LevelUpPoint --;
		switch(Data->Result&0xf)
		{
		case 0:
			CharacterAttribute->Strength ++;
			break;
		case 1:
			CharacterAttribute->Dexterity ++;
			break;
		case 2:
			CharacterAttribute->Vitality ++;
			CharacterAttribute->LifeMax = Data->Max;
			break;
		case 3:
			CharacterAttribute->Energy ++;
			CharacterAttribute->ManaMax = Data->Max;
			break;
        case 4:
			CharacterAttribute->Charisma ++;
            break;
		}
		CharacterAttribute->SkillManaMax = Data->SkillManaMax;
		CharacterAttribute->ShieldMax = Data->ShieldMax;
	}
    CharacterMachine->CalculateAll();
}

///////////////////////////////////////////////////////////////////////////////
// 생명, 마나
///////////////////////////////////////////////////////////////////////////////

void ReceiveLife( BYTE *ReceiveBuffer )
{
	LPPRECEIVE_LIFE Data = (LPPRECEIVE_LIFE)ReceiveBuffer;
	switch(Data->Index)
	{
	case 0xff:
		CharacterAttribute->Life = ((WORD)(Data->Life[0])<<8) + Data->Life[1];
		CharacterAttribute->Shield = ((WORD)(Data->Life[3])<<8) + Data->Life[4];
		break;
	case 0xfe:
		if(IsMasterLevel(Hero->Class) == true)
		{
			//	Master_Level_Data.wMaxLife			= Data->LifeMax;
			//	Master_Level_Data.wMaxMana			= Data->ManaMax;
			//	Master_Level_Data.wMaxShield		= Data->ShieldMax;
			Master_Level_Data.wMaxLife = ((WORD)(Data->Life[0])<<8) + Data->Life[1];
			Master_Level_Data.wMaxShield = ((WORD)(Data->Life[3])<<8) + Data->Life[4];
		}
		else
		{
			CharacterAttribute->LifeMax = ((WORD)(Data->Life[0])<<8) + Data->Life[1];
			CharacterAttribute->ShieldMax = ((WORD)(Data->Life[3])<<8) + Data->Life[4];
		}
		break;
	case 0xfd:
		EnableUse = 0;
		break;
	default:
		{
			int itemindex = Data->Index-MAX_EQUIPMENT_INDEX;
			ITEM* pItem = g_pMyInventory->FindItem(itemindex);
			if(pItem)
			{
				if(pItem->Durability > 0)
					pItem->Durability--;
				if(pItem->Durability <= 0)
					g_pMyInventory->DeleteItem(itemindex);
			}
		}
		break;
	}
	
#ifdef CONSOLE_DEBUG
	//g_ConsoleDebug->Write(MCD_RECEIVE, "0x26 [ReceiveLife]");
#endif // CONSOLE_DEBUG
}

void ReceiveMana( BYTE *ReceiveBuffer )
{
	LPPRECEIVE_LIFE Data = (LPPRECEIVE_LIFE)ReceiveBuffer;
	switch(Data->Index)
	{
	case 0xff:
		CharacterAttribute->Mana = ((WORD)(Data->Life[0])<<8) + Data->Life[1];
		CharacterAttribute->SkillMana = ((WORD)(Data->Life[2])<<8) + Data->Life[3];
		break;
	case 0xfe:
		if(IsMasterLevel(Hero->Class) == true)
		{
			//	Master_Level_Data.wMaxLife			= Data->LifeMax;
			//	Master_Level_Data.wMaxMana			= Data->ManaMax;
			//	Master_Level_Data.wMaxShield		= Data->ShieldMax;
			Master_Level_Data.wMaxMana = ((WORD)(Data->Life[0])<<8) + Data->Life[1];
			Master_Level_Data.wMaxBP = ((WORD)(Data->Life[2])<<8) + Data->Life[3];
		}
		else
		{
			CharacterAttribute->ManaMax = ((WORD)(Data->Life[0])<<8) + Data->Life[1];
			CharacterAttribute->SkillManaMax = ((WORD)(Data->Life[2])<<8) + Data->Life[3];
		}
		break;
	default:
		CharacterAttribute->Mana = ((WORD)(Data->Life[0])<<8) + Data->Life[1];
		if(Inventory[Data->Index-12].Durability > 0)
			Inventory[Data->Index-12].Durability --;
		if(Inventory[Data->Index-12].Durability <= 0)
		{
			Inventory[Data->Index-12].Type = -1;
			Inventory[Data->Index-12].Number = 0;
		}
		break;
	}
	
#ifdef CONSOLE_DEBUG
	//g_ConsoleDebug->Write(MCD_RECEIVE, "0x27 [ReceiveMana]");
#endif // CONSOLE_DEBUG
}

///////////////////////////////////////////////////////////////////////////////
// pk ,내구력, 술사용시, 날씨, 이밴트 발생, 소환에너지
///////////////////////////////////////////////////////////////////////////////

void ReceivePK( BYTE *ReceiveBuffer )
{
	LPPRECEIVE_PK Data = (LPPRECEIVE_PK)ReceiveBuffer;
	int Key = ((int)(Data->KeyH   )<<8) + Data->KeyL;
	CHARACTER *c = &CharactersClient[FindCharacterIndex(Key)];
    c->PK = Data->PK;

#ifdef PBG_ADD_PKSYSTEM_INGAMESHOP
	g_PKSystem->SetHeroPKState(Hero->PK);
#endif //PBG_ADD_PKSYSTEM_INGAMESHOP	
#ifdef PK_ATTACK_TESTSERVER_LOG
	PrintPKLog(c);
#endif // PK_ATTACK_TESTSERVER_LOG
	
	if(c->PK >= PVP_MURDERER2)
        c->Level = 1;
	else
        c->Level = 0;
	switch(Data->PK)
	{
	case 1: case 2:
		{
			g_pChatListBox->AddText(CharactersClient[FindCharacterIndex(Key)].ID,GlobalText[487], SEASON3B::TYPE_SYSTEM_MESSAGE);
		}
		break;
	case 3:
		{
			g_pChatListBox->AddText(CharactersClient[FindCharacterIndex(Key)].ID,GlobalText[488], SEASON3B::TYPE_ERROR_MESSAGE);			
		}
		break;
	case 4:
		{
			g_pChatListBox->AddText(CharactersClient[FindCharacterIndex(Key)].ID,GlobalText[489], SEASON3B::TYPE_SYSTEM_MESSAGE);
		}
		break;
	case 5:
		{
#if SELECTED_LANGUAGE == LANGUAGE_PHILIPPINES || SELECTED_LANGUAGE == LANGUAGE_ENGLISH || SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE
			char szTemp[100];
#else // SELECTED_LANGUAGE == LANGUAGE_PHILIPPINES || SELECTED_LANGUAGE == LANGUAGE_ENGLISH || SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE
			char szTemp[32];
#endif // SELECTED_LANGUAGE == LANGUAGE_PHILIPPINES || SELECTED_LANGUAGE == LANGUAGE_ENGLISH || SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE
			
			sprintf(szTemp, "%s %d%s", GlobalText[490], 1, GlobalText[491]);
			
			g_pChatListBox->AddText(CharactersClient[FindCharacterIndex(Key)].ID,szTemp, SEASON3B::TYPE_ERROR_MESSAGE);	
		}
		break;
	case 6:
		{
#if SELECTED_LANGUAGE == LANGUAGE_PHILIPPINES || SELECTED_LANGUAGE == LANGUAGE_ENGLISH || SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE
			char szTemp[100];
#else // SELECTED_LANGUAGE == LANGUAGE_PHILIPPINES || SELECTED_LANGUAGE == LANGUAGE_ENGLISH || SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE
			char szTemp[32];
#endif // SELECTED_LANGUAGE == LANGUAGE_PHILIPPINES || SELECTED_LANGUAGE == LANGUAGE_ENGLISH || SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE
			
			sprintf(szTemp, "%s %d%s", GlobalText[490], 2, GlobalText[491]);
			
			g_pChatListBox->AddText(CharactersClient[FindCharacterIndex(Key)].ID,szTemp, SEASON3B::TYPE_ERROR_MESSAGE);
		}
		break;
	}
}

// 내구도를 서버로 부터 받는 함수
void ReceiveDurability( BYTE *ReceiveBuffer )
{
    LPPHEADER_DEFAULT_VALUE_KEY Data = (LPPHEADER_DEFAULT_VALUE_KEY)ReceiveBuffer;
	
	if(Data->Value < 12)		//. 착용
	{
		CharacterMachine->Equipment[Data->Value].Durability = Data->KeyH;
	}
	else						//. 소지
	{
		int iItemIndex = Data->Value-MAX_EQUIPMENT_INDEX;
		ITEM* pItem = g_pMyInventory->FindItem(iItemIndex);
		
		if(pItem)
		{
			pItem->Durability = Data->KeyH;
		}
	}
	
	if(Data->KeyL)
	{
		EnableUse = 0;
	}
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x2A [ReceiveDurability(%d %d)]", Data->Value, Data->KeyL);
#endif // CONSOLE_DEBUG
}

//  술 사용시.
BOOL ReceiveHelperItem(BYTE *ReceiveBuffer, BOOL bEncrypted)
{
	if ( !bEncrypted)
	{
		GO_DEBUG;
		SendHackingChecked( 0x00, 0x29);
		return ( FALSE);
	}
	
    LPPRECEIVE_HELPER_ITEM Data = (LPPRECEIVE_HELPER_ITEM)ReceiveBuffer;
   	CharacterAttribute->AbilityTime[Data->Index] = Data->Time*24;
	switch (Data->Index)
	{
	case 0:	// 술
		CharacterAttribute->Ability |= ABILITY_FAST_ATTACK_SPEED;
		
        //  공격속도 계산.
        CharacterMachine->CalculateAttackSpeed();
		break;
	case 1: // 사랑의 묘약
		CharacterAttribute->Ability |= ABILITY_PLUS_DAMAGE;
		
        //  공격속도 계산.
        CharacterMachine->CalculateDamage();
		CharacterMachine->CalculateMagicDamage();
		break;
		
    case 2: //  영혼의 물약.
        CharacterAttribute->Ability |= ABILITY_FAST_ATTACK_SPEED2;
		
        //  공격속도 계산.
        CharacterMachine->CalculateAttackSpeed();
        break;
	}
	EnableUse = 0;
	
	return ( TRUE);
}

void ReceiveWeather( BYTE *ReceiveBuffer )
{
	LPPHEADER_DEFAULT Data = (LPPHEADER_DEFAULT)ReceiveBuffer;
	int Weather = (Data->Value>>4);
	if(Weather==0)
		RainTarget = 0;
	else if(Weather==1)
		RainTarget = (Data->Value&15)*6;
}

void ReceiveEvent( BYTE *ReceiveBuffer )
{
	LPPHEADER_EVENT Data = (LPPHEADER_EVENT)ReceiveBuffer;
	
    switch ( Data->m_byNumber )
    {
    case 1: //  환수 드래곤.
		if ( Data->m_byValue ) EnableEvent = 1;
        else               EnableEvent = 0; 
        break;
#ifdef USE_EVENT_ELDORADO
    case 3: //  황금 드래곤.
		if ( Data->m_byValue ) EnableEvent = 3;
        else               EnableEvent = 0; 
        break;
#endif
    case 2: //  타마짱.
#ifdef TAMAJJANG
#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE
        if ( Data->m_byValue )
        {
            DeleteTamaJjang (); //  타마짱 제거.
            CreateTamaJjang ();
        }
        else
        {
            HideTamaJjang();
        }
#endif
#endif
        break;
    }
    DeleteBoids();
}

void ReceiveSummonLife( BYTE *ReceiveBuffer )
{
	LPPHEADER_DEFAULT_SUBCODE Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
	SummonLife = Data->Value;
}

///////////////////////////////////////////////////////////////////////////////
// 교환
///////////////////////////////////////////////////////////////////////////////

BOOL ReceiveTrade(BYTE *ReceiveBuffer, BOOL bEncrypted)
{
	if ( !bEncrypted)
	{
		GO_DEBUG;
		SendHackingChecked( 0x00, 0x36);
		return ( FALSE);
	}
	
	LPPCHATING Data = (LPPCHATING)ReceiveBuffer;
	g_pTrade->ProcessToReceiveTradeRequest(Data->ID);
	
	return ( TRUE);
}

void ReceiveTradeResult( BYTE *ReceiveBuffer )
{
	LPPTRADE Data = (LPPTRADE)ReceiveBuffer;
	g_pTrade->ProcessToReceiveTradeResult(Data);
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x37 [ReceiveTradeResult(%d)]", Data->ID);
#endif // CONSOLE_DEBUG
}

void ReceiveTradeYourInventoryDelete( BYTE *ReceiveBuffer )
{
	LPPHEADER_DEFAULT Data = (LPPHEADER_DEFAULT)ReceiveBuffer;
	g_pTrade->ProcessToReceiveYourItemDelete(Data->Value);
}

void ReceiveTradeYourInventory( BYTE *ReceiveBuffer )
{
	LPPHEADER_DEFAULT_ITEM Data = (LPPHEADER_DEFAULT_ITEM)ReceiveBuffer;
	g_pTrade->ProcessToReceiveYourItemAdd(Data->Index, Data->Item);
}

void ReceiveTradeMyGold( BYTE *ReceiveBuffer )
{
	LPPHEADER_DEFAULT Data = (LPPHEADER_DEFAULT)ReceiveBuffer;
	g_pTrade->ProcessToReceiveMyTradeGold(Data->Value);
}

void ReceiveTradeYourGold( BYTE *ReceiveBuffer )
{
	LPPHEADER_DEFAULT_DWORD Data = (LPPHEADER_DEFAULT_DWORD)ReceiveBuffer;
	g_pTrade->SetYourTradeGold(int(Data->Value));
}

void ReceiveTradeYourResult( BYTE *ReceiveBuffer )
{
	LPPHEADER_DEFAULT Data = (LPPHEADER_DEFAULT)ReceiveBuffer;
	g_pTrade->ProcessToReceiveYourConfirm(Data->Value);
}


void ReceiveTradeExit( BYTE *ReceiveBuffer )
{
	if(EquipmentItem)
	{	// 장착 후 처리할 수 있게 한다.
		if ( !g_bPacketAfter_EquipmentItem)
		{
			memcpy( g_byPacketAfter_EquipmentItem, ReceiveBuffer, sizeof ( PHEADER_DEFAULT));
			g_bPacketAfter_EquipmentItem = TRUE;
		}
		return;
	}

	LPPHEADER_DEFAULT Data = (LPPHEADER_DEFAULT)ReceiveBuffer;
	g_pTrade->ProcessToReceiveTradeExit(Data->Value);
}

void ReceivePing( BYTE *ReceiveBuffer)
{
	SendPing();
}

#ifdef NP_GAME_GUARD
#ifdef GG_SERVER_AUTH
extern  CNPGameLib*  g_pnpGL;     //  nProtect Game Guard Lib.
BOOL ReceiveGGAuth( BYTE *ReceiveBuffer, BOOL bEncrypted)
{
	if ( !bEncrypted)
	{
		GO_DEBUG;
		SendHackingChecked( 0x00, 0xF1);
		return ( FALSE);
	}
	LPPHEADER_DEFAULT_DWORD Data = (LPPHEADER_DEFAULT_DWORD)ReceiveBuffer;
	if(g_pnpGL) 
	{
#ifdef NPROTECT_AUTH2
		GG_AUTH_DATA* pAuthData = (GG_AUTH_DATA*)(ReceiveBuffer+sizeof(PBMSG_HEADER)+1);
		g_pnpGL->Auth2(pAuthData);
#ifdef NP_LOG
		g_ErrorReport.Write("ReceiveGGAuth() g_pnpGL->Auth2(%x %x %x %x)\r\n",
			pAuthData->dwIndex, pAuthData->dwValue1, pAuthData->dwValue2, pAuthData->dwValue3);	
#endif	// NP_LOG

#else // NPROTECT_AUTH2
		
#ifdef NP_LOG
		g_ErrorReport.Write("ReceiveGGAuth() g_pnpGL->Auth(%x)\r\n", Data->Value);	
#endif	// NP_LOG
		g_pnpGL->Auth( Data->Value);
#endif // NPROTECT_AUTH2
	}
#ifdef CSK_FIX_GAMEGUARD_CHECK
	else
	{
#ifdef PBG_MOD_GAMEGUARD_HANDLE
		// 핵프로그램 발생시 클라이언트 바로 종료하지 않는다
		if(BLUE_MU::IsBlueMuServer())
		{
			if(!g_NPGameGuardHandle->IsNPHack())
				g_NPGameGuardHandle->SetNPHack(true);
			//g_NPGameGuardHandle->SetCharacterInfo(LogInID,);
		}
		else
#endif //PBG_MOD_GAMEGUARD_HANDLE
		CloseHack(g_hWnd, true);
	}
#endif // CSK_FIX_GAMEGUARD_CHECK
	
#ifdef _DEBUG
	char str[256];
#ifdef NPROTECT_AUTH2
	GG_AUTH_DATA* pAuthData = (GG_AUTH_DATA*)(ReceiveBuffer+sizeof(PBMSG_HEADER)+1);
	wsprintf( str, "ReceiveGGAuth - 요청   %X %X %X %X", pAuthData->dwIndex, pAuthData->dwValue1, pAuthData->dwValue2, pAuthData->dwValue3);
#else	// NPROTECT_AUTH2
	wsprintf( str, "ReceiveGGAuth - 요청   %X", Data->Value);
#endif // NPROTECT_AUTH2
	g_pChatListBox->AddText("", str, SEASON3B::TYPE_SYSTEM_MESSAGE);
#endif // _DEBUG
	
	return ( TRUE);
}
#endif	// GG_SERVER_AUTH
#endif	// NP_GAME_GUARD

///////////////////////////////////////////////////////////////////////////////
// 창고
///////////////////////////////////////////////////////////////////////////////

void ReceiveStorageGold( BYTE *ReceiveBuffer )
{
	LPPRECEIVE_STORAGE_GOLD Data = (LPPRECEIVE_STORAGE_GOLD)ReceiveBuffer;
	if(Data->Result)
	{
		CharacterMachine->StorageGold = Data->StorageGold;
		CharacterMachine->Gold        = Data->Gold;
	}
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x81 [ReceiveStorageGold(%d %d %d)]", Data->Result, Data->StorageGold, Data->Gold);
#endif // CONSOLE_DEBUG
}

void ReceiveStorageExit( BYTE *ReceiveBuffer )
{
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x82 [ReceiveStorageExit]");
#endif // CONSOLE_DEBUG
}

void ReceiveStorageStatus( BYTE *ReceiveBuffer )
{
	LPPHEADER_DEFAULT Data = ( LPPHEADER_DEFAULT)ReceiveBuffer;
	
	g_pStorageInventory->ProcessToReceiveStorageStatus(Data->Value);
}

///////////////////////////////////////////////////////////////////////////////
// 파티
///////////////////////////////////////////////////////////////////////////////

void ReceiveParty( BYTE *ReceiveBuffer )
{
	LPPHEADER_DEFAULT_KEY Data = (LPPHEADER_DEFAULT_KEY)ReceiveBuffer;
	PartyKey = ((int)(Data->KeyH )<<8) + Data->KeyL;
	
	SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CPartyMsgBoxLayout));
}

void ReceivePartyResult( BYTE *ReceiveBuffer )
{
	LPPHEADER_DEFAULT Data = (LPPHEADER_DEFAULT)ReceiveBuffer;
	switch(Data->Value)
	{
	case 0:g_pChatListBox->AddText("", GlobalText[497], SEASON3B::TYPE_ERROR_MESSAGE);break;
	case 1:g_pChatListBox->AddText("", GlobalText[498], SEASON3B::TYPE_ERROR_MESSAGE);break;
	case 2:g_pChatListBox->AddText("", GlobalText[499], SEASON3B::TYPE_ERROR_MESSAGE);break;
	case 3:g_pChatListBox->AddText("", GlobalText[500], SEASON3B::TYPE_ERROR_MESSAGE);break;
	case 4:g_pChatListBox->AddText("", GlobalText[501], SEASON3B::TYPE_ERROR_MESSAGE);break;
	case 5:g_pChatListBox->AddText("", GlobalText[502], SEASON3B::TYPE_ERROR_MESSAGE);break;
#ifdef ASG_ADD_GENS_SYSTEM
	case 6:g_pChatListBox->AddText("", GlobalText[2990], SEASON3B::TYPE_ERROR_MESSAGE);break;
#endif	// ASG_ADD_GENS_SYSTEM
#ifdef ASG_MOD_GENS_STRIFE_ADD_PARTY_MSG
	case 7:g_pChatListBox->AddText("", GlobalText[2997], SEASON3B::TYPE_ERROR_MESSAGE);break;
	case 8:g_pChatListBox->AddText("", GlobalText[2998], SEASON3B::TYPE_ERROR_MESSAGE);break;
#endif	// ASG_MOD_GENS_STRIFE_ADD_PARTY_MSG
	}
}

void ReceivePartyList( BYTE *ReceiveBuffer )
{
	LPPRECEIVE_PARTY_LISTS Data = (LPPRECEIVE_PARTY_LISTS)ReceiveBuffer;
	int Offset = sizeof(PRECEIVE_PARTY_LISTS);
	PartyNumber = Data->Count;
	for(int i=0;i<Data->Count;i++)
	{
		LPPRECEIVE_PARTY_LIST Data2 = (LPPRECEIVE_PARTY_LIST)(ReceiveBuffer+Offset);
		PARTY_t *p = &Party[i];
		memcpy(p->Name,Data2->ID,MAX_ID_SIZE);
		p->Name[MAX_ID_SIZE] = NULL;
		p->Number = Data2->Number;
		p->Map = Data2->Map;
		p->x = Data2->x;
		p->y = Data2->y;
        p->currHP = Data2->currHP;
        p->maxHP = Data2->maxHP;
#ifdef LJH_ADD_MORE_ZEN_FOR_ONE_HAVING_A_PARTY_WITH_MURDERER	
		g_PKSystem->SetHeroPartyPKState(Data2->PKPartyLevel);
#endif //LJH_ADD_MORE_ZEN_FOR_ONE_HAVING_A_PARTY_WITH_MURDERER	
		Offset += sizeof(PRECEIVE_PARTY_LIST);
	}
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x42 [ReceivePartyList(partynum : %d)]", Data->Count);
#endif // CONSOLE_DEBUG
}

void ReceivePartyInfo( BYTE *ReceiveBuffer )
{
    LPPRECEIVE_PARTY_INFOS Data = (LPPRECEIVE_PARTY_INFOS)ReceiveBuffer;
    int Offset = sizeof( PRECEIVE_PARTY_INFOS);
    for( int i=0; i<Data->Count; i++ )
    {
        LPPRECEIVE_PARTY_INFO Data2 = (LPPRECEIVE_PARTY_INFO)(ReceiveBuffer+Offset);
        char stepHP= Data2->value&0xf;
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		char number= (Data2->value>>4)&0xf;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		
#ifdef KJH_FIX_WOPS_K27187_STEPHP_INFO_IN_PARTY
		PARTY_t* p = &Party[i];
#else // KJH_FIX_WOPS_K27187_STEPHP_INFO_IN_PARTY
		PARTY_t* p = &Party[number];
#endif // KJH_FIX_WOPS_K27187_STEPHP_INFO_IN_PARTY
		
        p->stepHP = min ( 10, max ( 0, stepHP ) );
		
        Offset += sizeof( PRECEIVE_PARTY_INFO );
    }
}

void ReceivePartyLeave( BYTE *ReceiveBuffer )
{
	PartyNumber = 0;
	g_pChatListBox->AddText("", GlobalText[502], SEASON3B::TYPE_ERROR_MESSAGE);

#ifdef LJH_ADD_MORE_ZEN_FOR_ONE_HAVING_A_PARTY_WITH_MURDERER	
	// 파티를 해제했을때 플레이어의 파티 PK레벨 수준을 플레이어의 PK레벨 수준으로 맞춘다.
	g_PKSystem->SetHeroPartyPKState(Hero->PK);
#endif //LJH_ADD_MORE_ZEN_FOR_ONE_HAVING_A_PARTY_WITH_MURDERER	

    //  따라가고 있을때 서로 파티가 아니면은 X.
    if ( g_iFollowCharacter>=0 )
    {
        bool IsParty = false;
        CHARACTER* c = &CharactersClient[g_iFollowCharacter];
        for ( int i=0; i<PartyNumber; ++i )
        {
            if ( strcmp( Party[i].Name, c->ID )==NULL && strlen( Party[i].Name )==strlen( c->ID ) )
            {
                IsParty = true;
            }
        }
		
        if ( IsParty==false )
        {
            g_iFollowCharacter = -1;
        }
    }
}

void ReceivePartyGetItem ( BYTE* ReceiveBuffer )
{
	//9Bit(타입), 스킬, 행운, 옵션, 액설런트, 세트 (차례로)
    LPPRECEIVE_GETITEMINFO_FOR_PARTY Data = (LPPRECEIVE_GETITEMINFO_FOR_PARTY)ReceiveBuffer;
    int Key = ((int)(Data->KeyH )<<8) + Data->KeyL;
    int Index = FindCharacterIndex ( Key );
    CHARACTER* c = &CharactersClient[Index];
    if ( Hero==c ) return;
	
    int itemType = Data->ItemInfo&0x01fff;   //  아이템 타입.
    char itemName[100] = { 0, };
    char Text[100] = { 0, };
	
    if ( (Data->ItemInfo&0x10000) )      sprintf ( itemName, "%s ", GlobalText[620] );   //  액설런트.
    else if ( (Data->ItemInfo&0x20000) ) sprintf ( itemName, "%s ", GlobalText[1089] );  //  세트.
	
	int itemLevel = Data->ItemLevel;
    GetItemName ( itemType, itemLevel, Text );
    strcat ( itemName, Text );
    if ( (Data->ItemInfo&0x02000) ) strcat ( itemName, GlobalText[176] );    //  스킬.
    if ( (Data->ItemInfo&0x08000) ) strcat ( itemName, GlobalText[177] );    //  옵션.
    if ( (Data->ItemInfo&0x04000) ) strcat ( itemName, GlobalText[178] );    //  행운.
	
#if SELECTED_LANGUAGE == LANGUAGE_CHINESE
    sprintf ( Text, "%s %s", GlobalText[918], itemName );   //  획득.
#else
    sprintf ( Text, "%s %s", itemName, GlobalText[918] );   //  획득.
#endif // SELECTED_LANGUAGE == LANGUAGE_CHINESE
	
	g_pChatListBox->AddText(c->ID, Text, SEASON3B::TYPE_SYSTEM_MESSAGE);
}

///////////////////////////////////////////////////////////////////////////////
// 길드
///////////////////////////////////////////////////////////////////////////////

extern int ErrorMessage;

//  길드 가입.
void ReceiveGuild( BYTE *ReceiveBuffer )
{
	LPPHEADER_DEFAULT_KEY Data = (LPPHEADER_DEFAULT_KEY)ReceiveBuffer;
	GuildPlayerKey = ((int)(Data->KeyH )<<8) + Data->KeyL;
	
	SEASON3B::CNewUICommonMessageBox* pMsgBox;
	SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CGuildRequestMsgBoxLayout), &pMsgBox);
	pMsgBox->AddMsg(CharactersClient[FindCharacterIndex(GuildPlayerKey)].ID);
	// 429 "길드 가입 신청이 왔습니다."
	pMsgBox->AddMsg(GlobalText[429]);
}

void ReceiveGuildResult( BYTE *ReceiveBuffer )
{
	LPPHEADER_DEFAULT Data = (LPPHEADER_DEFAULT)ReceiveBuffer;
	switch(Data->Value)
	{
	case 0:g_pChatListBox->AddText("", GlobalText[503], SEASON3B::TYPE_ERROR_MESSAGE);break;
	case 1:g_pChatListBox->AddText("", GlobalText[504], SEASON3B::TYPE_ERROR_MESSAGE);break;
	case 2:g_pChatListBox->AddText("", GlobalText[505], SEASON3B::TYPE_ERROR_MESSAGE);break;
	case 3:g_pChatListBox->AddText("", GlobalText[506], SEASON3B::TYPE_ERROR_MESSAGE);break;
	case 4:g_pChatListBox->AddText("", GlobalText[507], SEASON3B::TYPE_ERROR_MESSAGE);break;
	case 5:g_pChatListBox->AddText("", GlobalText[508], SEASON3B::TYPE_ERROR_MESSAGE);break;
	case 6:g_pChatListBox->AddText("", GlobalText[509], SEASON3B::TYPE_ERROR_MESSAGE);break;
	case 7:g_pChatListBox->AddText("", GlobalText[510], SEASON3B::TYPE_ERROR_MESSAGE);break;
#ifdef ASG_ADD_GENS_SYSTEM
	case 0xA1:g_pChatListBox->AddText("", GlobalText[2992], SEASON3B::TYPE_ERROR_MESSAGE);break;// 2992	"길드 마스터가 겐스에 가입이 안되어 있습니다."
	case 0xA2:g_pChatListBox->AddText("", GlobalText[2995], SEASON3B::TYPE_ERROR_MESSAGE);break;// 2995 "길드 마스터가 다른 겐스에 가입되어 있습니다."
#ifdef ASG_MOD_GUILD_RESULT_GENS_MSG
	case 0xA3:g_pChatListBox->AddText("", GlobalText[2996], SEASON3B::TYPE_ERROR_MESSAGE);break;
#endif	// ASG_MOD_GUILD_RESULT_GENS_MSG
#endif	// ASG_ADD_GENS_SYSTEM
	}
}

void ReceiveGuildList( BYTE *ReceiveBuffer )
{
	LPPRECEIVE_GUILD_LISTS Data = (LPPRECEIVE_GUILD_LISTS)ReceiveBuffer;
	int Offset = sizeof(PRECEIVE_GUILD_LISTS);
	
	g_nGuildMemberCount = Data->Count;
	GuildTotalScore = Data->TotalScore;
    GuildTotalScore = max( 0, GuildTotalScore );
	
	g_pGuildInfoWindow->GuildClear();
	g_pGuildInfoWindow->UnionGuildClear();
	g_pGuildInfoWindow->SetRivalGuildName(Data->szRivalGuildName);
	for(int i=0;i<Data->Count;i++)
	{
		LPPRECEIVE_GUILD_LIST Data2 = (LPPRECEIVE_GUILD_LIST)(ReceiveBuffer+Offset);
		GUILD_LIST_t *p = &GuildList[i];
		memcpy(p->Name,Data2->ID,MAX_ID_SIZE);
		p->Name[MAX_ID_SIZE] = NULL;
		p->Number = Data2->Number;
		p->Server = ( 0x80 & Data2->CurrentServer) ? ( 0x7F & Data2->CurrentServer) : -1;	// 접속을 안했으면 -1
		p->GuildStatus = Data2->GuildStatus;
		Offset += sizeof(PRECEIVE_GUILD_LIST);
		g_pGuildInfoWindow->AddGuildMember(p);
	}
}

void ReceiveGuildLeave( BYTE *ReceiveBuffer )
{
	LPPHEADER_DEFAULT Data = (LPPHEADER_DEFAULT)ReceiveBuffer;
	switch(Data->Value)
	{
	case 0:g_pChatListBox->AddText("", GlobalText[511], SEASON3B::TYPE_ERROR_MESSAGE);break;
	case 1:g_pChatListBox->AddText("", GlobalText[512], SEASON3B::TYPE_ERROR_MESSAGE);break;
	case 2:g_pChatListBox->AddText("", GlobalText[513], SEASON3B::TYPE_ERROR_MESSAGE);break;
	case 3:g_pChatListBox->AddText("", GlobalText[514], SEASON3B::TYPE_ERROR_MESSAGE);break;
	case 4:g_pChatListBox->AddText("", GlobalText[515], SEASON3B::TYPE_ERROR_MESSAGE);break;
	case 5:g_pChatListBox->AddText("", GlobalText[568], SEASON3B::TYPE_ERROR_MESSAGE);break;
	}
    if(Data->Value==1 || Data->Value==4)
	{
		if(Data->Value==4 && Hero->GuildMarkIndex!=-1)
		{
			GuildMark[Hero->GuildMarkIndex].Key = -1;
			GuildMark[Hero->GuildMarkIndex].UnionName[0] = NULL;
			GuildMark[Hero->GuildMarkIndex].GuildName[0] = NULL;
			Hero->GuildMarkIndex = -1;
		}
		
		// 길드전중이였을 경우 대비..
		EnableGuildWar = false;
		
		g_nGuildMemberCount = -1;
		Hero->GuildStatus = G_NONE;
		Hero->GuildRelationShip = GR_NONE;
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_GUILDINFO);
		
#ifdef CSK_MOD_MOVE_COMMAND_WINDOW
		g_pMoveCommandWindow->SetCastleOwner(false);
#endif // CSK_MOD_MOVE_COMMAND_WINDOW
	}
	// 길드원 탈퇴시켰을때 목록 갱신 (주민번호묻는과정때문에 탈퇴누르고 바로 요청하면 안되므로 여기서 재갱신)
	else if( Data->Value == 5 )
	{
		SendRequestGuildList();
	}
}

void ReceiveCreateGuildInterface( BYTE *ReceiveBuffer )
{
	g_pNewUISystem->Show( SEASON3B::INTERFACE_NPCGUILDMASTER );
}

void ReceiveCreateGuildMasterInterface( BYTE *ReceiveBuffer )
{
}

void ReceiveDeleteGuildViewport( BYTE *ReceiveBuffer )
{
	LPPHEADER_DEFAULT_KEY Data = (LPPHEADER_DEFAULT_KEY)ReceiveBuffer;
    int Key = ((int)(Data->KeyH&0x7f)<<8) + Data->KeyL;
	int Index = FindCharacterIndex(Key);
	CHARACTER *c = &CharactersClient[Index];
	
	c->GuildStatus		= -1;		// 길드내에서의 직책
	c->GuildType		= 0;		// 길드타입
	c->ProtectGuildMarkWorldTime = 0.0f;
	c->GuildRelationShip	= 0;	// 길드관계
	c->GuildMarkIndex = -1;
	
	g_nGuildMemberCount = -1;
	
	g_pNewUISystem->Hide(SEASON3B::INTERFACE_GUILDINFO);
}

void ReceiveCreateGuildResult( BYTE *ReceiveBuffer )
{
	LPPMSG_GUILD_CREATE_RESULT Data = (LPPMSG_GUILD_CREATE_RESULT)ReceiveBuffer;
	switch(Data->Value)
	{
	case 0:g_pChatListBox->AddText("", GlobalText[516], SEASON3B::TYPE_ERROR_MESSAGE);break;
	case 2:g_pChatListBox->AddText("", GlobalText[517], SEASON3B::TYPE_ERROR_MESSAGE);break;
	case 3:g_pChatListBox->AddText("", GlobalText[518], SEASON3B::TYPE_ERROR_MESSAGE);break;
	case 4:g_pChatListBox->AddText("", GlobalText[940], SEASON3B::TYPE_ERROR_MESSAGE);break;
	case 5:g_pChatListBox->AddText("", GlobalText[941], SEASON3B::TYPE_ERROR_MESSAGE);break;
	case 6:g_pChatListBox->AddText("", GlobalText[942], SEASON3B::TYPE_ERROR_MESSAGE);break;
	case 1:
		memset(InputText[0],0,MAX_ID_SIZE);
		InputLength[0] = 0;
		InputTextMax[0] = MAX_ID_SIZE;
		
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_NPCGUILDMASTER);
		
		MouseUpdateTime = 0;
		MouseUpdateTimeMax = 6;
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
// 길드전
///////////////////////////////////////////////////////////////////////////////

bool EnableGuildWar = false;
int  GuildWarIndex = -1;
char GuildWarName[8+1];
int  GuildWarScore[2];

bool EnableSoccer = false;
BYTE HeroSoccerTeam = 0;
int  SoccerTime;
char SoccerTeamName[2][8+1];
bool SoccerObserver = false;

//전쟁 선포 메세지
void ReceiveDeclareWar( BYTE *ReceiveBuffer )
{
	LPPRECEIVE_WAR Data = (LPPRECEIVE_WAR)ReceiveBuffer;
	memset(GuildWarName,0,8);
	memcpy(GuildWarName,Data->Name,8);
	GuildWarName[8] = NULL;
	
	// 배틀사커전 신청
	if(Data->Type == 1)
	{
		SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CBattleSoccerMsgBoxLayout));
	}
	else	// 길드전 신청
	{
		SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CGuildWarMsgBoxLayout));
	}
}

//전쟁 선포 결과
void ReceiveDeclareWarResult( BYTE *ReceiveBuffer )
{
	LPPHEADER_DEFAULT Data = (LPPHEADER_DEFAULT)ReceiveBuffer;
	switch(Data->Value)
	{
	case 0:g_pChatListBox->AddText("", GlobalText[519], SEASON3B::TYPE_ERROR_MESSAGE);break;
	case 1:g_pChatListBox->AddText("", GlobalText[520], SEASON3B::TYPE_ERROR_MESSAGE);break;
	case 2:g_pChatListBox->AddText("", GlobalText[521], SEASON3B::TYPE_ERROR_MESSAGE);break;
	case 3:g_pChatListBox->AddText("", GlobalText[522], SEASON3B::TYPE_ERROR_MESSAGE);break;
	case 4:g_pChatListBox->AddText("", GlobalText[523], SEASON3B::TYPE_ERROR_MESSAGE);break;
	case 5:g_pChatListBox->AddText("", GlobalText[524], SEASON3B::TYPE_ERROR_MESSAGE);break;
	case 6:g_pChatListBox->AddText("", GlobalText[525], SEASON3B::TYPE_ERROR_MESSAGE);break;
	}
	if(Data->Value != 1 && !EnableGuildWar)
	{
        InitGuildWar();
	}
}

void ReceiveGuildBeginWar( BYTE *ReceiveBuffer )
{
	LPPRECEIVE_WAR Data = (LPPRECEIVE_WAR)ReceiveBuffer;
	EnableGuildWar = true;
	
	char Text[100];
	memset(GuildWarName,0,8);
	memcpy(GuildWarName,Data->Name,8);
	GuildWarName[8] = NULL;
	
	if(Data->Type==0)
	{
		sprintf(Text,GlobalText[526],GuildWarName);
	}
	else
	{
		sprintf(Text,GlobalText[533],GuildWarName);
		EnableSoccer = true;
	}
   	
	CreateNotice(Text,1);
	HeroSoccerTeam = Data->Team;
	
	for(int i=0;i<MARK_EDIT;i++)
	{
		MARK_t *p = &GuildMark[i];
		char Temp[8+1];
		memset(Temp,0,8);
		memcpy(Temp,(char *)Data->Name,8);
		Temp[8] = NULL;
		if(strcmp(p->GuildName,Temp)==NULL)
		{
			GuildWarIndex = i;
			break;
		}
	}
	if(GuildWarIndex >= 0)
	{
		for(int i=0;i<MAX_CHARACTERS_CLIENT;i++)
		{
			CHARACTER *c = &CharactersClient[i];
			GuildTeam(c);
		}
	}
	SetActionClass(Hero,&Hero->Object,PLAYER_RUSH1,AT_RUSH1);
   	SendRequestAction(AT_RUSH1,((BYTE)((Hero->Object.Angle[2]+22.5f)/360.f*8.f+1.f)%8));
	
	g_pNewUISystem->Show(SEASON3B::INTERFACE_BATTLE_SOCCER_SCORE);
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x62 [ReceiveGuildBeginWar(%d)]", Data->Team);
#endif // CONSOLE_DEBUG
}

void ReceiveGuildEndWar( BYTE *ReceiveBuffer )
{
	LPPHEADER_DEFAULT Data = (LPPHEADER_DEFAULT)ReceiveBuffer;
	char Text[100];
	int Win = 0;
	switch(Data->Value)
	{
	case 0:sprintf(Text,GlobalText[527]);break;
	case 1:sprintf(Text,GlobalText[528]);Win = 2;break;
	case 2:sprintf(Text,GlobalText[529]);Win = 2;break;
	case 3:sprintf(Text,GlobalText[530]);break;
	case 4:sprintf(Text,GlobalText[531]);Win = 2;break;
	case 5:sprintf(Text,GlobalText[532]);break;
	case 6:sprintf(Text,GlobalText[480]);Win = 1;break;
#ifdef LJH_FIX_NO_MSG_FOR_FORCED_MOVEOUT_FROM_BATTLESOCCER
	default:sprintf(Text,"");break;	
#endif	//LJH_FIX_NO_MSG_FOR_FORCED_MOVEOUT_FROM_BATTLESOCCER
	}
	
	//길드전 결과가 왔을 때, 초기화한다. 
	g_wtMatchTimeLeft.m_Time = 0;
	
#ifndef GUILD_WAR_EVENT
   	CreateNotice(Text,1);
#endif
	
	EnableGuildWar = false;
	EnableSoccer = false;
	
	GuildWarIndex = -1;
	GuildWarName[0] = NULL;
	for(int i=0;i<MAX_CHARACTERS_CLIENT;i++)
	{
		CHARACTER *c = &CharactersClient[i];
       	GuildTeam(c);
	}
	switch(Win)
	{
	case 2:
		SetActionClass(Hero,&Hero->Object,PLAYER_WIN1,AT_WIN1);
		SendRequestAction(AT_WIN1,((BYTE)((Hero->Object.Angle[2]+22.5f)/360.f*8.f+1.f)%8));
		break;
	case 0:
		SetActionClass(Hero,&Hero->Object,PLAYER_CRY1,AT_CRY1);
		SendRequestAction(AT_CRY1,((BYTE)((Hero->Object.Angle[2]+22.5f)/360.f*8.f+1.f)%8));
		break;
	}
	
	g_pNewUISystem->Hide(SEASON3B::INTERFACE_BATTLE_SOCCER_SCORE);
}

void ReceiveGuildWarScore( BYTE *ReceiveBuffer )
{
	LPPRECEIVE_WAR_SCORE Data = (LPPRECEIVE_WAR_SCORE)ReceiveBuffer;
	EnableGuildWar = true;
	
#ifdef GUILD_WAR_EVENT
	//나중에 꼭 고쳐야 함.
	int t1 = Data->Score1 - GuildWarScore[0];
	int t2 = Data->Score2 - GuildWarScore[1];
	if(t1 == 20 || t2 == 20)
	{
		//우리 팀원들에게 이펙트 출력
		if(t1 > t2) g_iGoalEffect = 1;
		//상대 팀원들에게 이펙트 출력
		else g_iGoalEffect = 2;
	}
#endif
	
	GuildWarScore[0] = Data->Score1;
	GuildWarScore[1] = Data->Score2;
}

void ReceiveGuildIDViewport( BYTE *ReceiveBuffer )
{
	LPPWHEADER_DEFAULT_WORD Data = (LPPWHEADER_DEFAULT_WORD)ReceiveBuffer;
	int Offset = sizeof(PWHEADER_DEFAULT_WORD);
	for( int i=0 ; i<Data->Value ; ++i )
	{
		LPPRECEIVE_GUILD_ID Data2 = (LPPRECEIVE_GUILD_ID)(ReceiveBuffer+Offset);
		int GuildKey = Data2->GuildKey;
		int Key = ((int)(Data2->KeyH&0x7f)<<8) + Data2->KeyL;
		int Index = FindCharacterIndex(Key);
		CHARACTER *c = &CharactersClient[Index];
		
		c->GuildStatus = Data2->GuildStatus;				// 길드직위
		c->GuildType = Data2->GuildType;					// 길드타입
		c->GuildRelationShip = Data2->GuildRelationShip;	// 길드관계

#ifdef CSK_MOD_MOVE_COMMAND_WINDOW
		g_pMoveCommandWindow->SetCastleOwner((bool)Data2->btCastleOwner);
#endif // CSK_MOD_MOVE_COMMAND_WINDOW
		
		// 길드마크 존재 확인
		if( g_GuildCache.IsExistGuildMark(GuildKey) )
			c->GuildMarkIndex = g_GuildCache.GetGuildMarkIndex( GuildKey );
		else
		{
			// 없으면 요청
			SendRequestGuildInfo(GuildKey);
			c->GuildMarkIndex = g_GuildCache.MakeGuildMarkIndex( GuildKey );
		}
        
		// 길드팀 세팅
		GuildTeam(c);
		
		// 길드장이나 길드원일때 미니맵을 다시 생성한다.
		if( World == WD_30BATTLECASTLE )
		{
			if( g_pSiegeWarfare )
			{
				if( g_pSiegeWarfare->IsCreated() == false )
				{
					g_pSiegeWarfare->InitMiniMapUI();
					g_pSiegeWarfare->SetGuildData( Hero );
					g_pSiegeWarfare->CreateMiniMapUI();
				}
			}
		}
		
		Offset += sizeof(PRECEIVE_GUILD_ID);
	}
}

void ReceiveGuildInfo( BYTE *ReceiveBuffer )
{
	LPPPRECEIVE_GUILDINFO Data = (LPPPRECEIVE_GUILDINFO)ReceiveBuffer;
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	g_GuildCache.SetGuildMark( Data->GuildKey, Data->UnionName, Data->GuildName, Data->Mark );
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	int Index = g_GuildCache.SetGuildMark( Data->GuildKey, Data->UnionName, Data->GuildName, Data->Mark );
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
}

// 길드직책을 임명/변경/해제 결과
void ReceiveGuildAssign( BYTE *ReceiveBuffer )
{
	char szTemp[MAX_GLOBAL_TEXT_STRING] = "Invalid GuildAssign";
	LPPRECEIVE_GUILD_ASSIGN pData = (LPPRECEIVE_GUILD_ASSIGN)ReceiveBuffer;
	if( pData->byResult == 0x01 )
	{
		switch( pData->byType )
		{
		case 0x01:		// 임명
			strcpy( szTemp, GlobalText[1374] );
			break;
		case 0x02:		// 변경
			strcpy( szTemp, GlobalText[1375] );
			break;
		case 0x03:		// 해제
			strcpy( szTemp, GlobalText[1376] );
			break;
		default:
			assert( !"Packet(0xE1)" );
			break;
		}
	}
	else
	{
		switch( pData->byResult )
		{
		case GUILD_ANS_NOTEXIST_GUILD:
			strcpy( szTemp, GlobalText[522] );
			break;
		case GUILD_ANS_NOTEXIST_PERMISSION:
			strcpy( szTemp, GlobalText[1386] );
			break;
		case GUILD_ANS_NOTEXIST_EXTRA_STATUS:
			strcpy( szTemp, GlobalText[1326] );
			break;
		case GUILD_ANS_NOTEXIST_EXTRA_TYPE:
			strcpy( szTemp, GlobalText[1327] );
			break;
		default:
			assert( !"Packet(0xE1)" );
			break;
		}
	}
	g_pChatListBox->AddText("", szTemp, SEASON3B::TYPE_SYSTEM_MESSAGE);
}

void ReceiveGuildRelationShip( BYTE *ReceiveBuffer )
{
	LPPMSG_GUILD_RELATIONSHIP pData = (LPPMSG_GUILD_RELATIONSHIP)ReceiveBuffer;

	g_pGuildInfoWindow->ReceiveGuildRelationShip( pData->byRelationShipType, pData->byRequestType, pData->byTargetUserIndexH, pData->byTargetUserIndexL );
}

void ReceiveGuildRelationShipResult( BYTE *ReceiveBuffer )
{
	char szTemp[MAX_GLOBAL_TEXT_STRING] = "Invalid GuildRelationShipResult";
	LPPMSG_GUILD_RELATIONSHIP_RESULT pData = (LPPMSG_GUILD_RELATIONSHIP_RESULT)ReceiveBuffer;
	if( pData->byResult == 0x01 )
	{
		if( pData->byRelationShipType == 0x01 )	// 길드연합
		{
			if ( pData->byRequestType == 0x01 )	
            {
                strcpy( szTemp, GlobalText[1381] );	// 가입
            }
			else if ( pData->byRequestType == 0x02 )
            {
                strcpy( szTemp, GlobalText[1382] );	// 탈퇴
            }
            else if ( pData->byRequestType == 0x10 )
            {
                strcpy( szTemp, GlobalText[1635] ); //  공성전시 연합(길드)가입,해체실패.
            }
		}
		else									// 적대길드
		{
			if( pData->byRequestType == 0x01 )	strcpy( szTemp, GlobalText[1383] );	// 맺기
			else								strcpy( szTemp, GlobalText[1384] );	// 끊기
		}
	}
	else if( pData->byResult == 0 )
	{
		// 1328 "실패했습니다."
		strcpy( szTemp, GlobalText[1328] );
	}
	else
	{
		switch( pData->byResult )
		{
		case GUILD_ANS_UNIONFAIL_BY_CASTLE:
			strcpy( szTemp, GlobalText[1637] );			// 1637 "공성전으로 인하여 연합기능이 제한됩니다."
			break;
		case GUILD_ANS_NOTEXIST_PERMISSION:
			strcpy( szTemp, GlobalText[1386] );			// 1386 "권한이 없습니다."
			break;
		case GUILD_ANS_EXIST_RELATIONSHIP_UNION:
			strcpy( szTemp, GlobalText[1250] );			// 1250 "연합 관계 입니다."
			break;
		case GUILD_ANS_EXIST_RELATIONSHIP_RIVAL:
			strcpy( szTemp, GlobalText[1251] );			// 1251 "적대 관계 입니다."
			break;
		case GUILD_ANS_EXIST_UNION:
			strcpy( szTemp, GlobalText[1252] );			// 1252 "길드연합이 존재 합니다."
			break;
		case GUILD_ANS_EXIST_RIVAL:
			strcpy( szTemp, GlobalText[1253] );			// 1253 "적대길드가 존재 합니다."
			break;
		case GUILD_ANS_NOTEXIST_UNION:
			strcpy( szTemp, GlobalText[1254] );			// 1254 "길드연합이 존재하지 않습니다."
			break;
		case GUILD_ANS_NOTEXIST_RIVAL:
			strcpy( szTemp, GlobalText[1255] );			// 1255 "적대길드가 존재하지 않습니다."
			break;
		case GUILD_ANS_NOT_UNION_MASTER:
			strcpy( szTemp, GlobalText[1333] );			// 1333 "길드연합 마스터가 아닙니다."
			break;
		case GUILD_ANS_NOT_GUILD_RIVAL:
			strcpy( szTemp, GlobalText[1329] );			// 1329 "적대길드가 아닙니다."
			break;
		case GUILD_ANS_CANNOT_BE_UNION_MASTER_GUILD:
			strcpy( szTemp, GlobalText[1331] );			// 1331 "연합길드생성조건미달"
			break;
		case GUILD_ANS_EXCEED_MAX_UNION_MEMBER:
			strcpy( szTemp, GlobalText[1287] );			// 1287 "최대 길드 연합 개수는 7개입니다."
			break;
		case GUILD_ANS_CANCEL_REQUEST:
			strcpy( szTemp, GlobalText[1268] );			// 1268 "신청이 취소되었습니다."
			break;
#ifdef ASG_ADD_GENS_SYSTEM
		case GUILD_ANS_UNION_MASTER_NOT_GENS:
			strcpy( szTemp, GlobalText[2991] );			// 2991	"연합 길드 마스터가 겐스 가입이 안되어 있습니다."
			break;
		case GUILD_ANS_GUILD_MASTER_NOT_GENS:
			strcpy( szTemp, GlobalText[2992] );			// 2992	"길드 마스터가 겐스에 가입이 안되어 있습니다."
			break;
		case GUILD_ANS_UNION_MASTER_DISAGREE_GENS:
			strcpy( szTemp, GlobalText[2993] );			// 2993	"연합 길드 마스터와 다른 겐스에 가입되어 있습니다."
			break;
#endif	// ASG_ADD_GENS_SYSTEM
		default:
			assert( !"Packet (0xE6)" );
			break;
		}
	}
	g_pChatListBox->AddText("", szTemp, SEASON3B::TYPE_SYSTEM_MESSAGE);	
	
	// 내가 길드연합에서 탈퇴성공했다면 유니온 정보 초기화
	int nCharKey = MAKEWORD( pData->byTargetUserIndexL, pData->byTargetUserIndexH );
	if( nCharKey == HeroKey && pData->byResult == 0x01 && pData->byRelationShipType == 0x01 && pData->byRequestType == 0x02 )
		GuildMark[Hero->GuildMarkIndex].UnionName[0] = NULL;
}

void ReceiveBanUnionGuildResult( BYTE *ReceiveBuffer )
{
	LPPMSG_BAN_UNIONGUILD pData = (LPPMSG_BAN_UNIONGUILD)ReceiveBuffer;
	if( pData->byResult == 0x01 )
	{
		if(g_pGuildInfoWindow->GetUnionCount() > 2)
		{
			Sleep(500);
			SendRequestUnionList();
		}
		// 바뀐 연합리스트 요청
		g_pGuildInfoWindow->UnionGuildClear();
	}
	else if( pData->byResult == 0 )
	{
		// 1328 "실패했습니다."
		g_pChatListBox->AddText("", GlobalText[1328], SEASON3B::TYPE_SYSTEM_MESSAGE);	
	}
}

void ReceiveUnionViewportNotify( BYTE *ReceiveBuffer )
{
	LPPMSG_UNION_VIEWPORT_NOTIFY_COUNT pData = (LPPMSG_UNION_VIEWPORT_NOTIFY_COUNT)ReceiveBuffer;
	int Offset = sizeof(PMSG_UNION_VIEWPORT_NOTIFY_COUNT);
	for( int i=0 ; i<pData->byCount ; ++i )
	{
		LPPMSG_UNION_VIEWPORT_NOTIFY pData2 = (LPPMSG_UNION_VIEWPORT_NOTIFY)(ReceiveBuffer+Offset);
		
		// 길드정보 갱신
		int nGuildMarkIndex = g_GuildCache.GetGuildMarkIndex( pData2->nGuildKey );
		memcpy( GuildMark[nGuildMarkIndex].UnionName, pData2->szUnionName, sizeof(char)*MAX_GUILDNAME );
		GuildMark[nGuildMarkIndex].UnionName[MAX_GUILDNAME] = NULL;
		
		// 캐릭정보 갱신
		int nCharKey = MAKEWORD( pData2->byKeyL, pData2->byKeyH );
		
		int nCharIndex = FindCharacterIndex(nCharKey);
		CharactersClient[nCharIndex].GuildRelationShip = pData2->byGuildRelationShip;
		
		Offset += sizeof(PMSG_UNION_VIEWPORT_NOTIFY);
	}
}

void ReceiveUnionList( BYTE *ReceiveBuffer )
{
	LPPMSG_UNIONLIST_COUNT pData = (LPPMSG_UNIONLIST_COUNT)ReceiveBuffer;
	g_pGuildInfoWindow->UnionGuildClear();
	if( pData->byResult == 1 )
	{
		int Offset = sizeof(PMSG_UNIONLIST_COUNT);
		for( int i=0 ; i<pData->byCount ; ++i )
		{
			LPPMSG_UNIONLIST pData2 = (LPPMSG_UNIONLIST)(ReceiveBuffer+Offset);
			
			BYTE tmp[64];
			for( int j=0 ; j<64 ; ++j )
			{
				if( j%2 == 0 )
					tmp[j] = ( pData2->GuildMark[j/2]>>4 )&0x0f;
				else
					tmp[j] = pData2->GuildMark[j/2]&0x0f;
			}
			
			g_pGuildInfoWindow->AddUnionList(tmp, pData2->szGuildName, pData2->byMemberCount);
			
			Offset += sizeof(PMSG_UNIONLIST);
		}
	}
}

void ReceiveSoccerTime( BYTE *ReceiveBuffer )
{
	LPPRECEIVE_SOCCER_TIME Data = (LPPRECEIVE_SOCCER_TIME)ReceiveBuffer;
	SoccerTime = Data->Time;
}

void ReceiveSoccerScore( BYTE *ReceiveBuffer )
{
	LPPRECEIVE_SOCCER_SCORE Data = (LPPRECEIVE_SOCCER_SCORE)ReceiveBuffer;
	memcpy(SoccerTeamName[0],Data->Name1,8);
	memcpy(SoccerTeamName[1],Data->Name2,8);
	SoccerTeamName[0][8] = NULL;
	SoccerTeamName[1][8] = NULL;
	GuildWarScore[0] = Data->Score1;
	GuildWarScore[1] = Data->Score2;

	if(GuildWarScore[0] != 255)
	{
		SoccerObserver = true;
		g_pNewUISystem->Show(SEASON3B::INTERFACE_BATTLE_SOCCER_SCORE);
	}
	else
	{
		SoccerObserver = false;
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_BATTLE_SOCCER_SCORE);
	}
	
	vec3_t Position, Angle, Light;
	Position[0]  = (float)(Hero->PositionX+0.5f)*TERRAIN_SCALE;
	Position[1]  = (float)(Hero->PositionY+0.5f)*TERRAIN_SCALE;
	Position[2] = RequestTerrainHeight(Position[0],Position[1]);
	ZeroMemory( Angle, sizeof ( Angle));
	Light[0] = Light[1] = Light[2] = 1.0f;
	CreateEffect(BITMAP_FIRECRACKERRISE,Position,Angle,Light);
}

void ReceiveSoccerGoal( BYTE *ReceiveBuffer )
{
	LPPHEADER_DEFAULT Data = (LPPHEADER_DEFAULT)ReceiveBuffer;
	char Text[100];
	if(Data->Value == HeroSoccerTeam)
		sprintf(Text,GlobalText[534],GuildMark[Hero->GuildMarkIndex].GuildName);
	else
		sprintf(Text,GlobalText[534],GuildWarName);
	g_pChatListBox->AddText("", Text, SEASON3B::TYPE_SYSTEM_MESSAGE);
}

void Receive_Master_LevelUp( BYTE *ReceiveBuffer )
{
	LPPMSG_MASTERLEVEL_UP Data = (LPPMSG_MASTERLEVEL_UP)ReceiveBuffer;
	Master_Level_Data.nMLevel			= Data->nMLevel;
	Master_Level_Data.nAddMPoint		= Data->nAddMPoint;
	Master_Level_Data.nMLevelUpMPoint	= Data->nMLevelUpMPoint;
	//	Master_Level_Data.nTotalMPoint		= Data->nTotalMPoint;
	Master_Level_Data.nMaxPoint			= Data->nMaxPoint;
	Master_Level_Data.wMaxLife			= Data->wMaxLife;
	Master_Level_Data.wMaxMana			= Data->wMaxMana;
	Master_Level_Data.wMaxShield		= Data->wMaxShield;
	Master_Level_Data.wMaxBP			= Data->wMaxBP;
	
	unicode::t_char szText[256] = {NULL, };
	WORD iExp = Master_Level_Data.lNext_MasterLevel_Experince - Master_Level_Data.lMasterLevel_Experince;
	sprintf(szText, GlobalText[1750], iExp);
	g_pChatListBox->AddText("", szText, SEASON3B::TYPE_SYSTEM_MESSAGE);
	
	CharacterMachine->CalulateMasterLevelNextExperience();
	
	OBJECT *o = &Hero->Object;
	
	if(IsMasterLevel(Hero->Class) == true)
	{
		CreateJoint(BITMAP_FLARE,o->Position,o->Position,o->Angle,45,o,80,2);
		for ( int i=0; i<19; ++i )
		{
			CreateJoint(BITMAP_FLARE,o->Position,o->Position,o->Angle,46,o,80,2);
		}
	}
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x51 [Receive_Master_LevelUp]");
#endif // CONSOLE_DEBUG
}

void Receive_Master_Level_Exp( BYTE *ReceiveBuffer )
{
	LPPMSG_MASTERLEVEL_INFO Data = (LPPMSG_MASTERLEVEL_INFO)ReceiveBuffer;
	Master_Level_Data.nMLevel = Data->nMLevel;
	Master_Level_Data.lMasterLevel_Experince = 0x0000000000000000;
	Master_Level_Data.lMasterLevel_Experince |= Data->btMExp1;	
	Master_Level_Data.lMasterLevel_Experince <<= 8;
	Master_Level_Data.lMasterLevel_Experince |= Data->btMExp2;	
	Master_Level_Data.lMasterLevel_Experince <<= 8;
	Master_Level_Data.lMasterLevel_Experince |= Data->btMExp3;	
	Master_Level_Data.lMasterLevel_Experince <<= 8;
	Master_Level_Data.lMasterLevel_Experince |= Data->btMExp4;	
	Master_Level_Data.lMasterLevel_Experince <<= 8;
	Master_Level_Data.lMasterLevel_Experince |= Data->btMExp5;	
	Master_Level_Data.lMasterLevel_Experince <<= 8;
	Master_Level_Data.lMasterLevel_Experince |= Data->btMExp6;	
	Master_Level_Data.lMasterLevel_Experince <<= 8;
	Master_Level_Data.lMasterLevel_Experince |= Data->btMExp7;	
	Master_Level_Data.lMasterLevel_Experince <<= 8;
	Master_Level_Data.lMasterLevel_Experince |= Data->btMExp8;
	Master_Level_Data.lNext_MasterLevel_Experince = 0x0000000000000000;
	Master_Level_Data.lNext_MasterLevel_Experince |= Data->btMNextExp1;	
	Master_Level_Data.lNext_MasterLevel_Experince <<= 8;
	Master_Level_Data.lNext_MasterLevel_Experince |= Data->btMNextExp2;	
	Master_Level_Data.lNext_MasterLevel_Experince <<= 8;
	Master_Level_Data.lNext_MasterLevel_Experince |= Data->btMNextExp3;	
	Master_Level_Data.lNext_MasterLevel_Experince <<= 8;
	Master_Level_Data.lNext_MasterLevel_Experince |= Data->btMNextExp4;	
	Master_Level_Data.lNext_MasterLevel_Experince <<= 8;
	Master_Level_Data.lNext_MasterLevel_Experince |= Data->btMNextExp5;	
	Master_Level_Data.lNext_MasterLevel_Experince <<= 8;
	Master_Level_Data.lNext_MasterLevel_Experince |= Data->btMNextExp6;	
	Master_Level_Data.lNext_MasterLevel_Experince <<= 8;
	Master_Level_Data.lNext_MasterLevel_Experince |= Data->btMNextExp7;	
	Master_Level_Data.lNext_MasterLevel_Experince <<= 8;
	Master_Level_Data.lNext_MasterLevel_Experince |= Data->btMNextExp8;
	Master_Level_Data.nMLevelUpMPoint			   = Data->nMLPoint;
	
	Master_Level_Data.wMaxLife			= Data->wMaxLife;
	Master_Level_Data.wMaxMana			= Data->wMaxMana;
	Master_Level_Data.wMaxShield		= Data->wMaxShield;
	Master_Level_Data.wMaxBP			= Data->wMaxSkillMana;
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x50 [Receive_Master_Level_Exp]");
#endif // CONSOLE_DEBUG
}

void Receive_Master_LevelGetSkill( BYTE *ReceiveBuffer )
{
	LPPMSG_ANS_MASTERLEVEL_SKILL Data = (LPPMSG_ANS_MASTERLEVEL_SKILL)ReceiveBuffer;
	
	if(Data->btResult == 1)
	{
		//습득했다.
		if(Data->nSkillNum > -1)
		{
			for(int i=0; i<MAX_MAGIC; ++i)
			{
				switch(Data->nSkillNum)
				{
				case AT_SKILL_ASHAKE_UP:
					if(AT_SKILL_DARK_HORSE == CharacterAttribute->Skill[i])
						CharacterAttribute->Skill[i] = 0;
					break;
				case AT_SKILL_BLAST_UP:
					if(AT_SKILL_BLAST == CharacterAttribute->Skill[i])
						CharacterAttribute->Skill[i] = 0;
					break;
				case AT_SKILL_MANY_ARROW_UP:
					if(AT_SKILL_CROSSBOW == CharacterAttribute->Skill[i])
						CharacterAttribute->Skill[i] = 0;
					break;
				case AT_SKILL_TORNADO_SWORDA_UP:
				case AT_SKILL_TORNADO_SWORDB_UP:
					if(AT_SKILL_WHEEL == CharacterAttribute->Skill[i])
						CharacterAttribute->Skill[i] = 0;
					break;
				case AT_SKILL_HEAL_UP:
					if(AT_SKILL_HEALING == CharacterAttribute->Skill[i])
						CharacterAttribute->Skill[i] = 0;
					break;
				case AT_SKILL_LIFE_UP:
					if(AT_SKILL_VITALITY == CharacterAttribute->Skill[i])
						CharacterAttribute->Skill[i] = 0;
					break;
				case AT_SKILL_SOUL_UP:
					if(AT_SKILL_WIZARDDEFENSE == CharacterAttribute->Skill[i])
						CharacterAttribute->Skill[i] = 0;
					break;
				case AT_SKILL_FIRE_BUST_UP:
					if(AT_SKILL_LONGPIER_ATTACK == CharacterAttribute->Skill[i])
						CharacterAttribute->Skill[i] = 0;
					break;
				case AT_SKILL_HELL_FIRE_UP:
					if(AT_SKILL_HELL == CharacterAttribute->Skill[i])
						CharacterAttribute->Skill[i] = 0;
					break;
#ifdef PJH_SEASON4_MASTER_RANK4
				case AT_SKILL_EVIL_SPIRIT_UP:
				case AT_SKILL_EVIL_SPIRIT_UP_M:
					if(AT_SKILL_EVIL == CharacterAttribute->Skill[i])
						CharacterAttribute->Skill[i] = 0;
					break;
				case AT_SKILL_BLOW_UP:
					if(AT_SKILL_ONETOONE == CharacterAttribute->Skill[i])
						CharacterAttribute->Skill[i] = 0;
					break;
				case AT_SKILL_ANGER_SWORD_UP:
					if(AT_SKILL_FURY_STRIKE == CharacterAttribute->Skill[i])
						CharacterAttribute->Skill[i] = 0;
					break;
				case AT_SKILL_BLOOD_ATT_UP:
					if(AT_SKILL_REDUCEDEFENSE == CharacterAttribute->Skill[i])
						CharacterAttribute->Skill[i] = 0;
					break;
				case AT_SKILL_POWER_SLASH_UP:
					if(AT_SKILL_ICE_BLADE == CharacterAttribute->Skill[i])
						CharacterAttribute->Skill[i] = 0;
					break;
				case AT_SKILL_FIRE_SCREAM_UP:
					if(AT_SKILL_DARK_SCREAM == CharacterAttribute->Skill[i])
						CharacterAttribute->Skill[i] = 0;
					break;
				case AT_SKILL_DEF_POWER_UP:
					if(AT_SKILL_DEFENSE == CharacterAttribute->Skill[i])
						CharacterAttribute->Skill[i] = 0;
					break;
				case AT_SKILL_ATT_POWER_UP:
					if(AT_SKILL_ATTACK == CharacterAttribute->Skill[i])
						CharacterAttribute->Skill[i] = 0;
					break;
				case AT_SKILL_ICE_UP:
					if(AT_SKILL_BLAST_FREEZE == CharacterAttribute->Skill[i])
						CharacterAttribute->Skill[i] = 0;
					break;
#endif //PJH_SEASON4_MASTER_RANK4
#ifdef PJH_ADD_MASTERSKILL
				case AT_SKILL_ALICE_SLEEP_UP:
					if(AT_SKILL_ALICE_SLEEP == CharacterAttribute->Skill[i])
						CharacterAttribute->Skill[i] = 0;
					break;
				case AT_SKILL_ALICE_CHAINLIGHTNING_UP:
					if(AT_SKILL_ALICE_CHAINLIGHTNING == CharacterAttribute->Skill[i])
						CharacterAttribute->Skill[i] = 0;
					break;
				case AT_SKILL_LIGHTNING_SHOCK_UP:
					if(AT_SKILL_LIGHTNING_SHOCK == CharacterAttribute->Skill[i])
						CharacterAttribute->Skill[i] = 0;
					break;
				case AT_SKILL_ALICE_DRAINLIFE_UP:
					if(AT_SKILL_ALICE_DRAINLIFE == CharacterAttribute->Skill[i])
						CharacterAttribute->Skill[i] = 0;
					break;
#endif
				}
			}
			bool Check_Add = false;

			for(int i=0; i<MAX_MAGIC; ++i)
			{
				if(Data->nSkillNum%5 != 0)
				{
					if(CharacterAttribute->Skill[i] == (Data->nSkillNum - 1))
					{
						CharacterAttribute->Skill[i] = Data->nSkillNum;
						Check_Add = true;
						break;
					}
				}
			}
			if(Check_Add == false)
			{
				for(int i=0; i<MAX_MAGIC; ++i)
				{
					if(CharacterAttribute->Skill[i] == 0)
					{
						CharacterAttribute->Skill[i] = Data->nSkillNum;
						break;
					}
				}
			}
			//////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////
		}
	}
	Master_Level_Data.nMLevelUpMPoint			   = Data->nMLPoint;
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x52 [Receive_Master_LevelGetSkill]");
#endif // CONSOLE_DEBUG
}

void ReceiveServerCommand( BYTE *ReceiveBuffer )
{
	LPPRECEIVE_SERVER_COMMAND Data = (LPPRECEIVE_SERVER_COMMAND)ReceiveBuffer;
	switch( Data->Cmd1)
	{
	case 2:	// 팡파레 + 폭죽
		PlayBuffer(SOUND_MEDAL);
		break;
	case 0:	// 폭죽
		{
			vec3_t Position, Angle, Light;
			Position[0]  = (float)(Data->Cmd2+0.5f)*TERRAIN_SCALE;
			Position[1]  = (float)(Data->Cmd3+0.5f)*TERRAIN_SCALE;
			Position[2] = RequestTerrainHeight(Position[0],Position[1]);
			ZeroMemory( Angle, sizeof ( Angle));
			Light[0] = Light[1] = Light[2] = 1.0f;
#ifdef YDG_ADD_FIRECRACKER_ITEM
			CreateEffect(BITMAP_FIRECRACKER0001,Position,Angle,Light,0);
#else	// YDG_ADD_FIRECRACKER_ITEM
			CreateEffect(BITMAP_FIRECRACKERRISE,Position,Angle,Light);
#endif	// YDG_ADD_FIRECRACKER_ITEM
		}
		break;
	case 1:	// 메시지 박스
        // Data->Cmd2 : 46번 까지 사용 (추가 상자는 그 뒤로 써주세요)
#ifndef LIMIT_CHAOS_MIX
		if(Data->Cmd2 >= 20)
		{
			SEASON3B::CreateOkMessageBox(GlobalText[830 + Data->Cmd2 - 20]);
		}
		else
		{
			SEASON3B::CreateOkMessageBox(GlobalText[650 + Data->Cmd2]);	
		}
#endif //LIMIT_CHAOS_MIX
		break;
		
	case 3:	// (레나)이벤트 메시지 박스
		SEASON3B::CreateOkMessageBox(GlobalText[710+Data->Cmd2]);
		break;
		
#ifdef LENA_EXCHANGE_ZEN
    case 4:	// (레나)이벤트 환전 메시지 박스
		SEASON3B::CreateOkMessageBox(GlobalText[725+Data->Cmd2]);
        break;
#endif // LENA_EXCHANGE_ZEN
		
	case 5:	// NPC 와의 대화
		{
			SEASON3B::CDialogMsgBox* pMsgBox = NULL;
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CDialogMsgBoxLayout), &pMsgBox);
			if(pMsgBox)
			{
				pMsgBox->AddMsg(g_DialogScript[Data->Cmd2].m_lpszText);
			}
		}
		break;
		
    case 6: //  서버 분할. ( 0: 길드 해제하고 와라. )
		SEASON3B::CreateOkMessageBox(GlobalText[449]);
        break;
		
#ifdef PCROOM_EVENT
	case 7:
#ifdef KJH_ADD_NPC_ROURKE_PCROOM_EVENT_MESSAGEBOX
		switch(Data->Cmd2) 
		{
		case 0:				// 0. 아이템을 지급받으시겠습니까?
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CPCRoomItemGiveLayout));
			break;
		case 1:				// 1. 이벤트 기간이 아님. 
			SEASON3B::CreateOkMessageBox(GlobalText[2021]);
			break;
		case 2:				// 2. 이미 아이템 지급  .
			SEASON3B::CreateOkMessageBox(GlobalText[2022]);
			break;
		case 3:				// 3. 실패 
			SEASON3B::CreateOkMessageBox(GlobalText[2023]);
			break;
		case 4:				// 4. 지급 자격 없음.
			SEASON3B::CreateOkMessageBox(GlobalText[2024]);
			break;
		}
#else // KJH_ADD_NPC_ROURKE_PCROOM_EVENT_MESSAGEBOX
		if (0 == Data->Cmd2)	// 0 아이템 지급 가능.
		{
			ShowCheckBox(1, 2020, MESSAGE_PCROOM_EVENT);
		}
		else					
		{
			// 1 이벤트 기간이 아님. 2  이미 아이템 지급  .
			// 3 실패. 4 지급 자격 없음.
			SEASON3B::CreateOkMessageBox(GlobalText[2020 + Data->Cmd2]);
		}
#endif // KJH_ADD_NPC_ROURKE_PCROOM_EVENT_MESSAGEBOX
		break;
		
#endif	// PCROOM_EVENT
	case 13:
		SEASON3B::CreateOkMessageBox(GlobalText[1826]);
		break;
		
	case 14:	// 화이트 앤젤 이벤트
		{
			switch(Data->Cmd2) 
			{
			case 0:	// 아이템 지급 대상자가 맞음
				SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CWhiteAngelEventLayout));
				break;
				
			case 1:	// 아이템 지급 대상자가 아님
				SEASON3B::CreateOkMessageBox(GlobalText[2024]);
				break;
				
			case 2:	// 아이템이 이미 지급되었다.
				SEASON3B::CreateOkMessageBox(GlobalText[2022]);
				break;
				
			case 3:	// 에러
				SEASON3B::CreateOkMessageBox(GlobalText[2023]);
				break;
				
			case 4:	// 아이템 지급 성공
				//ShowCustomMessageBox(GlobalText[858]);
				break;
				
			case 5:	// 아이템 지급 실패
				SEASON3B::CreateOkMessageBox(GlobalText[2023]);
				break;
			}
		}
		break;
		
#ifdef PRUARIN_EVENT07_3COLORHARVEST		// 07년도 3COLOR & 추석 이벤트
	case 15:
		{
			switch(Data->Cmd2) 
			{
			case 0:				// 이미 지급되었습니다.
				SEASON3B::CreateOkMessageBox(GlobalText[2022]);
				break;
			case 1:				// 아이템을 지급받으시겠습니까?
				SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CHarvestEventLayout));
				break;
			case 2:				// 아이템 지급실패
				SEASON3B::CreateOkMessageBox(GlobalText[2023]);
				break;
			}
		}
		break;
#endif // PRUARIN_EVENT07_3COLORHARVEST
		
#ifdef LDK_ADD_SANTA_NPC
	case 16:
		{
			SEASON3B::CNewUICommonMessageBox* pMsgBox = NULL;

			switch(Data->Cmd2)
			{
			case 0: //최초방문 2588
				SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CSantaTownSantaMsgBoxLayout), &pMsgBox);
				pMsgBox->AddMsg(GlobalText[2588]);	// 2588 "산타마을에 어서오세요, 구해주신 보답으로 선물을.... "
				break;
			case 1: //일반방문 2585
				SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CSantaTownSantaMsgBoxLayout), &pMsgBox);
				pMsgBox->AddMsg(GlobalText[2585]);	// 2585 "산타마을에 어서오세요. 선물을 받아가세요"
				break;
			case 2: //이미 아이템을 받아갔음
				SEASON3B::CreateOkMessageBox(GlobalText[2587]);
				break;
			case 3: //실패(내부오류)
				SEASON3B::CreateOkMessageBox(GlobalText[2023]);
				break;
			}
		}
		break;
#endif //LDK_ADD_SANTA_NPC
		
#ifdef LDK_ADD_SNOWMAN_NPC
	case 17: //데비아스로 이동 하시겠습니까?
		SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CSantaTownLeaveMsgBoxLayout));
		break;
#endif //LDK_ADD_SNOWMAN_NPC
		
		//블러드캐슬 텍스트 확장
	case 47:
	case 48:
	case 49:
		SEASON3B::CreateOkMessageBox(GlobalText[1823+Data->Cmd1-47]);
		break;
		
	case 55:
		{
			unicode::t_char strText[128];
			unicode::_sprintf(strText, GlobalText[2043], GlobalText[39]);
			SEASON3B::CreateOkMessageBox(strText);
		}
		break;
	case 56:
		{
			unicode::t_char strText[128];
			unicode::_sprintf(strText, GlobalText[2043], GlobalText[56]);
			SEASON3B::CreateOkMessageBox(strText);
		}
		break;
	case 57:
		{
			unicode::t_char strText[128];
			unicode::_sprintf(strText, GlobalText[2043], GlobalText[57]);
			SEASON3B::CreateOkMessageBox(strText);
		}
		break;
#ifdef CSK_EVENT_CHERRYBLOSSOM
	case 58: 
		{
			WORD Key = ((WORD)(Data->Cmd2)<<8)+Data->Cmd3;
			Key &= 0x7FFF;
			int Index = FindCharacterIndex(Key);					// 인덱스를 찾아서 저장
			if( Index >= 0 && Index != MAX_CHARACTERS_CLIENT ) {
				OBJECT* to = &CharactersClient[Index].Object;
				if( to != NULL ) {
					CreateEffect( MODEL_EFFECT_SKURA_ITEM, to->Position, to->Angle, to->Light, 0, to );
					PlayBuffer(SOUND_CHERRYBLOSSOM_EFFECT0, to);
				}
			}
		}
		break;
#endif //CSK_EVENT_CHERRYBLOSSOM
#ifdef YDG_ADD_FIRECRACKER_ITEM
	case 59:	// 크리스마스 폭죽
		{
			vec3_t Position, Angle, Light;
			Position[0]  = (float)(Data->Cmd2+0.5f)*TERRAIN_SCALE;
			Position[1]  = (float)(Data->Cmd3+0.5f)*TERRAIN_SCALE;
			Position[2] = RequestTerrainHeight(Position[0],Position[1]);
			ZeroMemory( Angle, sizeof ( Angle));
			Light[0] = Light[1] = Light[2] = 1.0f;
			CreateEffect(BITMAP_FIRECRACKER0001,Position,Angle,Light,1);
		}
#endif	// YDG_ADD_FIRECRACKER_ITEM
#ifdef YDG_ADD_CS5_REVIVAL_CHARM
	case 60:	// 부활의 부적 사용 성공
		g_PortalMgr.ResetRevivePosition();
		break;
#endif	// YDG_ADD_CS5_REVIVAL_CHARM
#ifdef YDG_ADD_CS5_PORTAL_CHARM
	case 61:	// 이동의 부적 사용 성공
		g_PortalMgr.ResetPortalPosition();
		break;
#endif	// YDG_ADD_CS5_PORTAL_CHARM
	}
}

///////////////////////////////////////////////////////////////////////////////
// 믹스 관련
///////////////////////////////////////////////////////////////////////////////

void ReceiveMix( BYTE *ReceiveBuffer )
{
	LPPHEADER_DEFAULT_ITEM Data = (LPPHEADER_DEFAULT_ITEM)ReceiveBuffer;
	
	switch(Data->Index)
	{
    case 0:	// 실패
		{
#ifdef LEM_ADD_LUCKYITEM
			if( g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND) && g_pLuckyItemWnd->GetAct() )
			{
				g_pLuckyItemWnd->GetResult( 0, Data->Index );
				break;
			}
#endif // LEM_ADD_LUCKYITEM
			g_pMixInventory->SetMixState(SEASON3B::CNewUIMixInventory::MIX_FINISHED);
			unicode::t_char szText[256] = { 0, };
			switch (g_MixRecipeMgr.GetMixInventoryType())
			{
			case SEASON3A::MIXTYPE_GOBLIN_NORMAL:
			case SEASON3A::MIXTYPE_GOBLIN_CHAOSITEM:
			case SEASON3A::MIXTYPE_GOBLIN_ADD380:
#ifdef ADD_SOCKET_MIX
			case SEASON3A::MIXTYPE_EXTRACT_SEED:
			case SEASON3A::MIXTYPE_SEED_SPHERE:
#endif	// ADD_SOCKET_MIX
				unicode::_sprintf(szText, GlobalText[594]);		// 조합 실패
				g_pChatListBox->AddText("", szText, SEASON3B::TYPE_ERROR_MESSAGE);
				break;
				// 			case SEASON3A::MIXTYPE_TRAINER:
				// 				unicode::_sprintf(szText, GlobalText[1208]);	// 부활 실패
				// 				g_pChatListBox->AddText("", szText, SEASON3B::TYPE_ERROR_MESSAGE);
				// 				break;
			case SEASON3A::MIXTYPE_OSBOURNE:
				unicode::_sprintf(szText, GlobalText[2105], GlobalText[2061]);	// 제련 실패
				g_pChatListBox->AddText("", szText, SEASON3B::TYPE_ERROR_MESSAGE);
				break;
			case SEASON3A::MIXTYPE_JERRIDON:
				unicode::_sprintf(szText, GlobalText[2105], GlobalText[2062]);	// 환원 실패
				g_pChatListBox->AddText("", szText, SEASON3B::TYPE_ERROR_MESSAGE);
				break;
			case SEASON3A::MIXTYPE_ELPIS:
				unicode::_sprintf(szText, GlobalText[2112], GlobalText[2063]);	// 정제 실패
				g_pChatListBox->AddText("", szText, SEASON3B::TYPE_ERROR_MESSAGE);
				break;
#ifdef PSW_NEW_CHAOS_CARD
			case SEASON3A::MIXTYPE_CHAOS_CARD:
				unicode::_sprintf(szText, GlobalText[2112], GlobalText[2265]);	// 카오스 카드 성공
				g_pChatListBox->AddText("", szText, SEASON3B::TYPE_ERROR_MESSAGE);
				break;
#endif	// PSW_NEW_CHAOS_CARD
#ifdef CSK_EVENT_CHERRYBLOSSOM
			case SEASON3A::MIXTYPE_CHERRYBLOSSOM:
				unicode::_sprintf(szText, GlobalText[2112], GlobalText[2560]);	// 벚꽃 조합 성공
				g_pChatListBox->AddText("", szText, SEASON3B::TYPE_ERROR_MESSAGE);
				break;
#endif	// CSK_EVENT_CHERRYBLOSSOM
			}
		}
		break;
    case 1:	// 성공
		{
#ifdef LEM_ADD_LUCKYITEM
			if( g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND) && g_pLuckyItemWnd->GetAct() )	
			{
				g_pLuckyItemWnd->GetResult(1, 0, Data->Item);
				break;
			}
#endif // LEM_ADD_LUCKYITEM
			g_pMixInventory->SetMixState(SEASON3B::CNewUIMixInventory::MIX_FINISHED);
			unicode::t_char szText[256] = { 0, };
			switch (g_MixRecipeMgr.GetMixInventoryType())
			{
			case SEASON3A::MIXTYPE_GOBLIN_NORMAL:
			case SEASON3A::MIXTYPE_GOBLIN_CHAOSITEM:
			case SEASON3A::MIXTYPE_GOBLIN_ADD380:
#ifdef ADD_SOCKET_MIX
			case SEASON3A::MIXTYPE_EXTRACT_SEED:
			case SEASON3A::MIXTYPE_SEED_SPHERE:
#endif	// ADD_SOCKET_MIX
				unicode::_sprintf(szText, GlobalText[595]);		// 조합 성공
				g_pChatListBox->AddText("", szText, SEASON3B::TYPE_SYSTEM_MESSAGE);
				break;
				// 			case SEASON3A::MIXTYPE_TRAINER:
				// 				unicode::_sprintf(szText, GlobalText[1209]);	// 부활 성공
				// 				g_pChatListBox->AddText("", szText, SEASON3B::TYPE_SYSTEM_MESSAGE);
				// 				break;
			case SEASON3A::MIXTYPE_OSBOURNE:
				unicode::_sprintf(szText, GlobalText[2106], GlobalText[2061]);	// 제련 성공
				g_pChatListBox->AddText("", szText, SEASON3B::TYPE_SYSTEM_MESSAGE);
				break;
			case SEASON3A::MIXTYPE_JERRIDON:
				unicode::_sprintf(szText, GlobalText[2106], GlobalText[2062]);	// 환원 성공
				g_pChatListBox->AddText("", szText, SEASON3B::TYPE_SYSTEM_MESSAGE);
				break;
			case SEASON3A::MIXTYPE_ELPIS:
				unicode::_sprintf(szText, GlobalText[2113], GlobalText[2063]);	// 정제 성공
				g_pChatListBox->AddText("", szText, SEASON3B::TYPE_SYSTEM_MESSAGE);
				break;
#ifdef PSW_NEW_CHAOS_CARD
			case SEASON3A::MIXTYPE_CHAOS_CARD:
				unicode::_sprintf(szText, GlobalText[2113], GlobalText[2265]);	// 카오스 카드 성공
				g_pChatListBox->AddText("", szText, SEASON3B::TYPE_SYSTEM_MESSAGE);
				break;
#endif	// PSW_NEW_CHAOS_CARD
#ifdef CSK_EVENT_CHERRYBLOSSOM
			case SEASON3A::MIXTYPE_CHERRYBLOSSOM:
				unicode::_sprintf(szText, GlobalText[2113], GlobalText[2560]);	// 벚꽃 조합 성공
				g_pChatListBox->AddText("", szText, SEASON3B::TYPE_SYSTEM_MESSAGE);
				break;
#endif	// CSK_EVENT_CHERRYBLOSSOM
			}
			
			g_pMixInventory->DeleteAllItems();
			g_pMixInventory->InsertItem(0, Data->Item);
			
			PlayBuffer(SOUND_MIX01);
			PlayBuffer(SOUND_JEWEL01);
		}
		break;
    case 2:	// 돈없음
	case 0x0B:
		{
			g_pMixInventory->SetMixState(SEASON3B::CNewUIMixInventory::MIX_READY);
			g_pChatListBox->AddText("", GlobalText[596], SEASON3B::TYPE_ERROR_MESSAGE);
		}
		break;
	case 4:	// 초대권은 레벨 10이상만
		SEASON3B::CreateOkMessageBox(GlobalText[649]);
		g_pMixInventory->SetMixState(SEASON3B::CNewUIMixInventory::MIX_FINISHED);
		break;
		
	case 9:	// 망토는 레벨 10 이상만
		SEASON3B::CreateOkMessageBox(GlobalText[689]);
		g_pMixInventory->SetMixState(SEASON3B::CNewUIMixInventory::MIX_FINISHED);
		break;
		
    case 100:    //  조련사 조합 성공.
		g_pMixInventory->SetMixState(SEASON3B::CNewUIMixInventory::MIX_FINISHED);
		g_pMixInventory->DeleteAllItems();
		g_pMixInventory->InsertItem(0, Data->Item);
        break;
	case 3:	// 악마의 열쇠,눈 개수 오류
	case 5:	// 경품
	case 7:
	case 8:
	case 0x0A:

#ifdef LEM_ADD_LUCKYITEM	// 럭키아이템 티켓 교환 실패시 메세지 [lem_2010.9.8]
	case 0x20:
		if( g_pLuckyItemWnd->GetAct() )	
		{
			g_pLuckyItemWnd->GetResult(0, Data->Index, Data->Item);
		}
	break;
#endif // LEM_ADD_LUCKYITEM
	default:
		g_pMixInventory->SetMixState(SEASON3B::CNewUIMixInventory::MIX_FINISHED);
		break;
	}
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x86 [ReceiveMix(%d)]", Data->Index);
#endif // CONSOLE_DEBUG
}

void ReceiveMixExit( BYTE *ReceiveBuffer )
{
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x87 [ReceiveMixExit]");
#endif // CONSOLE_DEBUG
}

//////////////////////////////////////////////////////////////////////////
//보석 조합 관련 함수
//////////////////////////////////////////////////////////////////////////
void ReceiveGemMixResult(BYTE* ReceiveBuffer)
{
	LPPMSG_ANS_JEWEL_MIX Data = (LPPMSG_ANS_JEWEL_MIX)ReceiveBuffer;
	char sBuf[256];
	memset(sBuf, 0, 256);
	switch(Data->m_iResult)
	{
	case 0:
	case 2:
	case 3:
		{
			sprintf(sBuf, "%s%s %s", GlobalText[1801], GlobalText[1816], GlobalText[868]);
			g_pChatListBox->AddText("", sBuf, SEASON3B::TYPE_SYSTEM_MESSAGE);
			COMGEM::GetBack();
		}
		break;
	case 1:
		{
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CGemIntegrationUnityResultMsgBoxLayout));
		}
		break;
	case 4:
		{
			g_pChatListBox->AddText("", GlobalText[1817], SEASON3B::TYPE_SYSTEM_MESSAGE);
			COMGEM::GetBack();
		}
		break;
	case 5:
		{
			g_pChatListBox->AddText("", GlobalText[1811], SEASON3B::TYPE_SYSTEM_MESSAGE);
			COMGEM::GetBack();
		}
		break;
	}
}

void ReceiveGemUnMixResult(BYTE* ReceiveBuffer)
{
	LPPMSG_ANS_JEWEL_UNMIX Data = (LPPMSG_ANS_JEWEL_UNMIX)ReceiveBuffer;
	char sBuf[256];
	memset(sBuf, 0, 256);
	
	switch(Data->m_iResult)
	{
	case 0:
	case 5:
		{
			sprintf(sBuf, "%s%s %s", GlobalText[1800], GlobalText[1816], GlobalText[868]);
			g_pChatListBox->AddText("", sBuf, SEASON3B::TYPE_SYSTEM_MESSAGE);
			COMGEM::GetBack();
		}
		break;
	case 1:
		{
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CGemIntegrationDisjointResultMsgBoxLayout));
		}
		break;
	case 2:
	case 3:
	case 4:
	case 6:
		{
			g_pChatListBox->AddText("", GlobalText[1812], SEASON3B::TYPE_SYSTEM_MESSAGE);	
			COMGEM::GetBack();
		}
		break;
	case 7:
		{
			g_pChatListBox->AddText("", GlobalText[1815], SEASON3B::TYPE_SYSTEM_MESSAGE);
			COMGEM::GetBack();
		}
		break;
	case 8:
		{
			g_pChatListBox->AddText("", GlobalText[1811], SEASON3B::TYPE_SYSTEM_MESSAGE);	
			COMGEM::GetBack();
		}
		break;
	}
}
///////////////////////////////////////////////////////////////////////////////
// 악마의 광장 관련 함수
///////////////////////////////////////////////////////////////////////////////

void ReceiveMoveToDevilSquareResult( BYTE *ReceiveBuffer )
{
	g_pNewUISystem->Hide( SEASON3B::INTERFACE_DEVILSQUARE );
	
	LPPHEADER_DEFAULT Data = ( LPPHEADER_DEFAULT)ReceiveBuffer;
	switch ( Data->Value)
	{
	case 0:	// 성공
		break;
	case 1: // 초대권이 없다
		SEASON3B::CreateOkMessageBox(GlobalText[677]);
		break;
		
	case 2: // 참여가능 시간이 지났다.
		SEASON3B::CreateOkMessageBox(GlobalText[678]);
		break;
		
	case 3: // 레벨 제한 때문에 못간다. ( 내 레벨이 높다. )
		SEASON3B::CreateOkMessageBox(GlobalText[686]);
		break;
		
	case 4: // 레벨 제한 때문에 못간다. ( 내 레벨이 낮다. )
		SEASON3B::CreateOkMessageBox(GlobalText[687]);
		break;
		
	case 5: // 인원이 꽉 차서 못간다.
		SEASON3B::CreateOkMessageBox(GlobalText[679]);
		break;
		
	case 6:	// "살인 상태에서는 악마의 광장에 입장이 불가능합니다."
		{
			unicode::t_char strText[128];
			unicode::_sprintf(strText, GlobalText[2043], GlobalText[39]);
			SEASON3B::CreateOkMessageBox(strText);
		}
		break;
	}
}

void ReceiveDevilSquareOpenTime( BYTE *ReceiveBuffer )
{
	LPPHEADER_DEFAULT Data = ( LPPHEADER_DEFAULT)ReceiveBuffer;
	if ( 0 == Data->Value)	// 오픈 시간(분)
	{
		SEASON3B::CreateOkMessageBox(GlobalText[643]);
	}
	else
	{
		unicode::t_char strText[128];
		unicode::_sprintf(strText, GlobalText[644], ( int)Data->Value);
		SEASON3B::CreateOkMessageBox(strText);
	}
}

void ReceiveDevilSquareCountDown( BYTE *ReceiveBuffer )
{
	LPPHEADER_DEFAULT Data = ( LPPHEADER_DEFAULT)ReceiveBuffer;
	
	if( g_CursedTemple->IsCursedTemple() )
	{
		if( Data->Value == TYPE_MATCH_CURSEDTEMPLE_ENTER_CLOSE
			|| Data->Value == TYPE_MATCH_CURSEDTEMPLE_GAME_START )
		{
			g_CursedTemple->SetInterfaceState(false, 0);
			matchEvent::StartMatchCountDown( Data->Value + 1);
		}
	}
#ifdef YDG_ADD_DOPPELGANGER_EVENT
	else if (World >= WD_65DOPPLEGANGER1 && World <= WD_68DOPPLEGANGER4)
	{
		if( ((BYTE)(Data->Value + 1) >= TYPE_MATCH_DOPPELGANGER_ENTER_CLOSE && (BYTE)(Data->Value + 1) <= TYPE_MATCH_DOPPELGANGER_CLOSE) || (BYTE)(Data->Value + 1) == TYPE_MATCH_NONE )
		{
			matchEvent::StartMatchCountDown((BYTE)(Data->Value + 1));
		}
	}
#endif	// YDG_ADD_DOPPELGANGER_EVENT
	else
	{
		matchEvent::StartMatchCountDown( Data->Value + 1);
	}
}

void ReceiveDevilSquareRank( BYTE *ReceiveBuffer )
{
	LPPDEVILRANK Data = ( LPPDEVILRANK)ReceiveBuffer;
    matchEvent::SetMatchResult ( Data->m_Count, Data->m_MyRank, ( MatchResult*)&Data->m_byRank, Data->m_MyRank );
}


///////////////////////////////////////////////////////////////////////////////
// 블러드 캐슬 관련 함수
///////////////////////////////////////////////////////////////////////////////

void ReceiveMoveToEventMatchResult( BYTE *ReceiveBuffer )
{
	g_pNewUISystem->HideAll();
	
	LPPHEADER_DEFAULT Data = ( LPPHEADER_DEFAULT)ReceiveBuffer;
	switch ( Data->Value)
	{
	case 0:	// 성공
		break;
	case 1: // 투명망토가 없다
		SEASON3B::CreateOkMessageBox(GlobalText[854]);
		break;
		
	case 2: // 참여가능 시간이 지났다.
        {
			unicode::t_char strText[128];
			unicode::_sprintf(strText, GlobalText[852], GlobalText[1146]);
			SEASON3B::CreateOkMessageBox(strText);
        }
		break;
		
	case 3: // 레벨 제한 때문에 못간다. ( 내 레벨이 높다. )
		SEASON3B::CreateOkMessageBox(GlobalText[686]);
		break;
		
	case 4: // 레벨 제한 때문에 못간다. ( 내 레벨이 낮다. )
		SEASON3B::CreateOkMessageBox(GlobalText[687]);
		break;
		
	case 5: // 인원이 꽉 차서 못간다.
        {
			unicode::t_char strText[128];
			unicode::_sprintf(strText, GlobalText[853], GlobalText[1146], MAX_BLOOD_CASTLE_MEN);
			SEASON3B::CreateOkMessageBox(strText);
        }
		break;
		
	case 6:	// 하루에 6번만 입장가능합니다
		{
			unicode::t_char strText[128];
			unicode::_sprintf(strText, GlobalText[867], 6);
			SEASON3B::CreateOkMessageBox(strText);
		}
		break;
    case 7: //  "살인 상태에서는 블러드캐슬에 입장이 불가능합니다."
		{
			unicode::t_char strText[128];
			unicode::_sprintf(strText, GlobalText[2043], GlobalText[56]);
			SEASON3B::CreateOkMessageBox(strText);	
		}
        break;
    case 8: //  참여가능 시간이 지났다.
        {
			unicode::t_char strText[128];
			unicode::_sprintf(strText, GlobalText[852], GlobalText[1147]);
			SEASON3B::CreateOkMessageBox(strText);
        }
        break;
		
    case 9: //  인원이 꽉 차서 못간다.
        {
			unicode::t_char strText[128];
			unicode::_sprintf(strText, GlobalText[853], GlobalText[1147], MAX_CHAOS_CASTLE_MEN);
			SEASON3B::CreateOkMessageBox(strText);
        }
        break;
	}
}

void ReceiveEventZoneOpenTime( BYTE *ReceiveBuffer )
{
	LPPHEADER_MATCH_OPEN_VALUE Data = (LPPHEADER_MATCH_OPEN_VALUE)ReceiveBuffer;
	if (Data->Value == 1)			// 악마의광장 남은시간
	{
		if ( 0 == Data->KeyH)	// 오픈 시간(분)
		{
			SEASON3B::CreateOkMessageBox(GlobalText[643]);
		}
		else
		{
			unicode::t_char strText[128];
			unicode::_sprintf(strText, GlobalText[644], (int)Data->KeyH);
			SEASON3B::CreateOkMessageBox(strText);
		}
	}
	else if (Data->Value == 2)	// 블러드캐슬 남은시간
	{
		unicode::t_char strText[256];
		if(0 == Data->KeyH)
		{
			unicode::_sprintf(strText, GlobalText[850], GlobalText[1146]);
		}
		else
		{
			unicode::_sprintf(strText, GlobalText[851], ( int)Data->KeyH, GlobalText[1146]);
		}
		SEASON3B::CreateOkMessageBox(strText);
	}
    else if ( Data->Value==4 )    //  카오스캐슬 남은 시간.
    {
#ifdef KJW_FIX_CHAOSCASTLE_MESSAGE
#endif // KJW_FIX_CHAOSCASTLE_MESSAGE
		WORD time = MAKEWORD ( Data->KeyL, Data->KeyH );
        
        if ( 0 == time )	// 오픈 시간(분)
		{
			char szOpenTime1[256] = {0, };
			char szOpenTime2[256] = {0, };
			
			sprintf( szOpenTime1, GlobalText[850], GlobalText[1147] );
			sprintf( szOpenTime2, GlobalText[1156], GlobalText[1147], Data->KeyM, 100 );
			
			GlobalText.Remove(1154);
			GlobalText.Remove(1155);
			GlobalText.Add(1154, szOpenTime1);
			GlobalText.Add(1155, szOpenTime2);
			
			SEASON3B::CNewUICommonMessageBox* pMsgBox = NULL;
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CChaosCastleTimeCheckMsgBoxLayout), &pMsgBox);
			if(pMsgBox)
			{
				pMsgBox->AddMsg(GlobalText[1154]);
				pMsgBox->AddMsg(GlobalText[1155]);
			}
		}
		else
		{
		/* 이혁재 - 확인 요망
		#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE
		wsprintf ( GlobalText[1154], GlobalText[851], time, GlobalText[1147] );
		
		  #else
			*/
            char Text[256];
            int Hour = (int)(time/60);
            int Mini = (int)(time)-(Hour*60);
			
			char szOpenTime[256] = {0, };
			
			/*
			#ifdef PHIL_LANGUAGE_PROBLEM
			if(Hour != 0) wsprintf ( Text, GlobalText[1167], GlobalText[1147], Hour, Mini );
			else wsprintf ( Text, GlobalText[851], Mini, GlobalText[1147] );
			strcpy( GlobalText[1154], Text);
			#else
			wsprintf ( GlobalText[1154], GlobalText[1164], Hour );
			wsprintf ( Text, GlobalText[851], Mini, GlobalText[1147] );
			strcat ( GlobalText[1154], Text );
			#endif //PHIL_LANGUAGE_PROBLEM
			*///??
			
			wsprintf ( szOpenTime, GlobalText[1164], Hour );
			wsprintf ( Text, GlobalText[851], Mini, GlobalText[1147] );
			strcat ( szOpenTime, Text );
			
			GlobalText.Remove(1154);
			GlobalText.Add(1154, szOpenTime);
			
			//#endif// SELECTED_LANGUAGE == LANGUAGE_JAPANESE
			
			SEASON3B::CNewUICommonMessageBox* pMsgBox = NULL;
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CChaosCastleTimeCheckMsgBoxLayout), &pMsgBox);
			if(pMsgBox)
			{
				// 1154 ""
				pMsgBox->AddMsg(GlobalText[1154]);
			}
		}
    }
	else if( Data->Value==5 )
	{
		unicode::t_char strText[256];
		if(0 == Data->KeyH)
		{
			unicode::_sprintf(strText, GlobalText[850], GlobalText[2369]);
		}
		else
		{
			unicode::_sprintf(strText, GlobalText[851], ( int)Data->KeyH, GlobalText[2369]);
		}
		SEASON3B::CreateOkMessageBox(strText);
	}
}

//////////////////////////////////////////////////////////////////////////
//  경기장 이동 요청 결과.
//////////////////////////////////////////////////////////////////////////
void ReceiveMoveToEventMatchResult2( BYTE *ReceiveBuffer )
{
	g_pNewUISystem->HideAll();
	
	LPPWHEADER_DEFAULT_WORD Data = ( LPPWHEADER_DEFAULT_WORD)ReceiveBuffer;
	switch ( Data->Value)
	{
	case 0:	// 성공
		break;
	case 1: // 알맞는 언데드 마스크 없음.
		SEASON3B::CreateOkMessageBox(GlobalText[854]);
		break;
		
	case 2: // 참여가능 시간이 지났다.
        {
			unicode::t_char strText[128];
			unicode::_sprintf(strText, GlobalText[852], GlobalText[1147]);
			SEASON3B::CreateOkMessageBox(strText);
        }
		break;
		
	case 3: // 레벨 제한 때문에 못간다. ( 내 레벨이 높다. )
		SEASON3B::CreateOkMessageBox(GlobalText[686]);
		break;
		
	case 4: // 레벨 제한 때문에 못간다. ( 내 레벨이 낮다. )
		SEASON3B::CreateOkMessageBox(GlobalText[687]);
		break;
		
	case 5: // 인원이 꽉 차서 못간다.
        {
			unicode::t_char strText[128];
			unicode::_sprintf(strText, GlobalText[853], GlobalText[1147], MAX_CHAOS_CASTLE_MEN);
			SEASON3B::CreateOkMessageBox(strText);
        }
		break;
		
	case 6:	// 하루에 6번만 입장가능합니다
		{
			unicode::t_char strText[128];
			unicode::_sprintf(strText, GlobalText[867], 6);
			SEASON3B::CreateOkMessageBox(strText);
		}
		break;
		
    case 7: //  Zen이 부족하여 입장할 수 없습니다.
		SEASON3B::CreateOkMessageBox(GlobalText[423]);
        break;
		
	case 8: //"살인 상태에서는 카오스 캐슬에 입장이 불가능합니다."
		{
			unicode::t_char strText[128];
			unicode::_sprintf(strText, GlobalText[2043], GlobalText[57]);
			SEASON3B::CreateOkMessageBox(strText);
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
//  변경되는 속성값을 받는다.
//////////////////////////////////////////////////////////////////////////
void ReceiveSetAttribute ( BYTE* ReceiveBuffer )
{
    LPPRECEIVE_SET_MAPATTRIBUTE Data = (LPPRECEIVE_SET_MAPATTRIBUTE)ReceiveBuffer;

	g_ErrorReport.Write("Type:%d \r\n", Data->m_byType);
	
    //  속성을 채운다.
    switch ( Data->m_byType )
    {
    case 0: //  범위 속성.
        {
            if ( InBloodCastle() && Data->m_byMapAttr==TW_NOGROUND )
            {
                SetActionObject ( World, 36, 0, 1.f );
            }
			
            for ( int k=0; k<Data->m_byCount; k++ )
            {
				int dx = Data->m_vAttribute[(k*2)+1].m_byX - Data->m_vAttribute[(k*2)].m_byX + 1;
                int dy = Data->m_vAttribute[(k*2)+1].m_byY - Data->m_vAttribute[(k*2)].m_byY + 1;
				
				g_ErrorReport.Write("count:%d, x:%d, y:%d \r\n", Data->m_byCount, dx, dy);
				
                AddTerrainAttributeRange ( Data->m_vAttribute[(k*2)].m_byX, Data->m_vAttribute[(k*2)].m_byY, dx, dy, Data->m_byMapAttr, 1-Data->m_byMapSetType );
            }
        }
        break;
		
    case 1: //  특정 위치 속성.
        for ( int i=0; i<Data->m_byCount; ++i )
        {
            if ( Data->m_byMapSetType )   //  제거.
            {
				g_ErrorReport.Write("SubTerrainAttribute - count:%d, x:%d, y:%d \r\n", Data->m_byCount, Data->m_vAttribute[i].m_byX, Data->m_vAttribute[i].m_byY);
				SubTerrainAttribute ( Data->m_vAttribute[i].m_byX, Data->m_vAttribute[i].m_byY, Data->m_byMapAttr );
            }
            else                    //  추가.
            {
				g_ErrorReport.Write("AddTerrainAttribute - count:%d, x:%d, y:%d \r\n", Data->m_byCount, Data->m_vAttribute[i].m_byX, Data->m_vAttribute[i].m_byY);
				AddTerrainAttribute ( Data->m_vAttribute[i].m_byX, Data->m_vAttribute[i].m_byY, Data->m_byMapAttr );
            }
        }
        break;
    }
}

//////////////////////////////////////////////////////////////////////////
//  게임 진행 메시지.
//////////////////////////////////////////////////////////////////////////
void ReceiveMatchGameCommand ( BYTE* ReceiveBuffer )
{
    LPPRECEIVE_MATCH_GAME_STATE Data = (LPPRECEIVE_MATCH_GAME_STATE)ReceiveBuffer;
	
    matchEvent::SetMatchGameCommand ( Data );
}

#ifdef YDG_ADD_NEW_DUEL_PROTOCOL
void ReceiveDuelRequest(BYTE* ReceiveBuffer)	// 결투요청
{
	if(g_MessageBox->IsEmpty() == false)
	{
		return;
	}

	LPPMSG_REQ_DUEL_ANSWER Data = (LPPMSG_REQ_DUEL_ANSWER)ReceiveBuffer;
	// 아무개님이 결투를 요청했습니다 응하겠습니까 창 띄움
	g_DuelMgr.SetDuelPlayer(DUEL_ENEMY, MAKEWORD(Data->bIndexL,Data->bIndexH), Data->szID);
	
	if(g_pNewUISystem->IsImpossibleDuelInterface() == true)
	{
		g_DuelMgr.SendDuelRequestAnswer(DUEL_ENEMY, FALSE);
		return;
	}
	
	SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CDuelMsgBoxLayout));
	PlayBuffer(SOUND_OPEN_DUELWINDOW);
}

void ReceiveDuelStart(BYTE* ReceiveBuffer)	// 결투요청 결과
{
	LPPMSG_ANS_DUEL_INVITE Data = (LPPMSG_ANS_DUEL_INVITE)ReceiveBuffer;
	char szMessage[256];
	if(Data->nResult == 0)
	{
		g_DuelMgr.EnableDuel(TRUE);
		g_DuelMgr.SetHeroAsDuelPlayer(DUEL_HERO);
		g_DuelMgr.SetDuelPlayer(DUEL_ENEMY, MAKEWORD(Data->bIndexL,Data->bIndexH), Data->szID);
		// 912 "%s 님이 결투에 응했습니다."
		sprintf(szMessage, GlobalText[912], g_DuelMgr.GetDuelPlayerID(DUEL_ENEMY));
		g_pChatListBox->AddText("", szMessage, SEASON3B::TYPE_ERROR_MESSAGE);
		
		g_pNewUISystem->Show(SEASON3B::INTERFACE_DUEL_WINDOW);
		PlayBuffer(SOUND_START_DUEL);
	}
	else if (Data->nResult == 15)	// 거부
	{
		g_DuelMgr.SetDuelPlayer(DUEL_ENEMY, MAKEWORD(Data->bIndexL,Data->bIndexH), Data->szID);
		// 913 "%s 님이 결투신청을 거부했습니다."
		sprintf(szMessage, GlobalText[913], g_DuelMgr.GetDuelPlayerID(DUEL_ENEMY));
		g_pChatListBox->AddText("", szMessage, SEASON3B::TYPE_ERROR_MESSAGE);
	}
	else if (Data->nResult == 16)	// 방 더이상 개설 못함
	{
		SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CDuelCreateErrorMsgBoxLayout));
	}
	else if (Data->nResult == 28)	// 상대방이 레벨 30이하
	{
		g_DuelMgr.SetDuelPlayer(DUEL_ENEMY, MAKEWORD(Data->bIndexL,Data->bIndexH), Data->szID);
		// 2704 "%d레벨 이상의 캐릭터만 결투가 가능합니다."
		sprintf(szMessage, GlobalText[2704], 30);
		g_pChatListBox->AddText("", szMessage, SEASON3B::TYPE_ERROR_MESSAGE);
	}
	else if (Data->nResult == 30)	// 젠부족
	{
		g_DuelMgr.SetDuelPlayer(DUEL_ENEMY, MAKEWORD(Data->bIndexL,Data->bIndexH), Data->szID);
		// 1811 "젠이 부족합니다."
		sprintf(szMessage, GlobalText[1811]);
		g_pChatListBox->AddText("", szMessage, SEASON3B::TYPE_ERROR_MESSAGE);
	}
}

void ReceiveDuelEnd(BYTE* ReceiveBuffer)	// 결투종료
{
	LPPMSG_ANS_DUEL_EXIT Data = (LPPMSG_ANS_DUEL_EXIT)ReceiveBuffer;

	if (Data->nResult == 0)
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_DUEL_WINDOW);
		g_DuelMgr.EnableDuel(FALSE);
		g_DuelMgr.SetDuelPlayer(DUEL_ENEMY, MAKEWORD(Data->bIndexL,Data->bIndexH), Data->szID);
		
		g_pChatListBox->AddText("", GlobalText[914], SEASON3B::TYPE_ERROR_MESSAGE);
		
		if(g_wtMatchTimeLeft.m_Type == 2)
			g_wtMatchTimeLeft.m_Time = 0;
		g_DuelMgr.EnablePetDuel(FALSE);	// LHJ - 결투 끝날때 다크 스피릿도 공격 못하게 하는 변수
	}
	else
	{
	}
}

void ReceiveDuelScore(BYTE* ReceiveBuffer)	// 결투자 점수 정보
{
	LPPMSG_DUEL_SCORE_BROADCAST Data = (LPPMSG_DUEL_SCORE_BROADCAST)ReceiveBuffer;

	if(g_DuelMgr.IsDuelPlayer(MAKEWORD(Data->bIndexL1, Data->bIndexH1), DUEL_HERO))
	{
		g_DuelMgr.SetScore(DUEL_HERO, Data->btDuelScore1);
		g_DuelMgr.SetScore(DUEL_ENEMY, Data->btDuelScore2);
	}
	else if(g_DuelMgr.IsDuelPlayer(MAKEWORD(Data->bIndexL2, Data->bIndexH2), DUEL_HERO))
	{
		g_DuelMgr.SetScore(DUEL_HERO, Data->btDuelScore2);
		g_DuelMgr.SetScore(DUEL_ENEMY, Data->btDuelScore1);
	}
	else
	{
		//. 먼가 데이타가 잘못온경우
	}
}

void ReceiveDuelHP(BYTE* ReceiveBuffer)	// 결투자 생명 정보
{
	LPPMSG_DUEL_HP_BROADCAST Data = (LPPMSG_DUEL_HP_BROADCAST)ReceiveBuffer;

	if(g_DuelMgr.IsDuelPlayer(MAKEWORD(Data->bIndexL1, Data->bIndexH1), DUEL_HERO))
	{
		g_DuelMgr.SetHP(DUEL_HERO, Data->btHP1);
		g_DuelMgr.SetHP(DUEL_ENEMY, Data->btHP2);
		g_DuelMgr.SetSD(DUEL_HERO, Data->btShield1);
		g_DuelMgr.SetSD(DUEL_ENEMY, Data->btShield2);
	}
	else if(g_DuelMgr.IsDuelPlayer(MAKEWORD(Data->bIndexL2, Data->bIndexH2), DUEL_HERO))
	{
		g_DuelMgr.SetHP(DUEL_HERO, Data->btHP2);
		g_DuelMgr.SetHP(DUEL_ENEMY, Data->btHP1);
		g_DuelMgr.SetSD(DUEL_HERO, Data->btShield2);
		g_DuelMgr.SetSD(DUEL_ENEMY, Data->btShield1);
	}
	else
	{
		//. 먼가 데이타가 잘못온경우
	}
}

void ReceiveDuelChannelList(BYTE* ReceiveBuffer)	// 결투 채널(관전) 리스트
{
	LPPMSG_ANS_DUEL_CHANNELLIST Data = (LPPMSG_ANS_DUEL_CHANNELLIST)ReceiveBuffer;
	for (int i = 0; i < 4; ++i)
	{
		g_DuelMgr.SetDuelChannel(i, Data->channel[i].bStart, Data->channel[i].bWatch, Data->channel[i].szID1, Data->channel[i].szID2);
	}
}

void ReceiveDuelWatchRequestReply(BYTE* ReceiveBuffer)	// 채널(관전) 요청
{
	LPPMSG_ANS_DUEL_JOINCNANNEL Data = (LPPMSG_ANS_DUEL_JOINCNANNEL)ReceiveBuffer;
	if (Data->nResult == 0)
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_DUELWATCH);

		g_DuelMgr.SetCurrentChannel(Data->nChannelId);
		g_DuelMgr.SetDuelPlayer(DUEL_HERO, MAKEWORD(Data->bIndexL1, Data->bIndexH1), Data->szID1);
		g_DuelMgr.SetDuelPlayer(DUEL_ENEMY, MAKEWORD(Data->bIndexL2, Data->bIndexH2), Data->szID2);
	}
	else if (Data->nResult == 16)	// 방 더이상 개설 못함
	{
		SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CDuelCreateErrorMsgBoxLayout));
	}
	else if (Data->nResult == 27)
	{
		// 방 꽉찼음
		SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CDuelWatchErrorMsgBoxLayout));
	}
	else
	{
	}
}

void ReceiveDuelWatcherJoin(BYTE* ReceiveBuffer)	// 채널(관전) 입장한 관전자
{
	LPPMSG_DUEL_JOINCNANNEL_BROADCAST Data = (LPPMSG_DUEL_JOINCNANNEL_BROADCAST)ReceiveBuffer;
	g_DuelMgr.AddDuelWatchUser(Data->szID);
}

void ReceiveDuelWatchEnd(BYTE* ReceiveBuffer)	// 채널(관전) 종료
{
	LPPMSG_ANS_DUEL_LEAVECNANNEL Data = (LPPMSG_ANS_DUEL_LEAVECNANNEL)ReceiveBuffer;
	if (Data->nResult == 0)
	{
		g_DuelMgr.RemoveAllDuelWatchUser();
	}
	else
	{
	}
}

void ReceiveDuelWatcherQuit(BYTE* ReceiveBuffer)	// 채널(관전) 떠난 관전자
{
	LPPMSG_DUEL_LEAVECNANNEL_BROADCAST Data = (LPPMSG_DUEL_LEAVECNANNEL_BROADCAST)ReceiveBuffer;
	g_DuelMgr.RemoveDuelWatchUser(Data->szID);
}

void ReceiveDuelWatcherList(BYTE* ReceiveBuffer)	// 관전자 리스트(All)
{
	g_DuelMgr.RemoveAllDuelWatchUser();
	
	LPPMSG_DUEL_OBSERVERLIST_BROADCAST Data = (LPPMSG_DUEL_OBSERVERLIST_BROADCAST)ReceiveBuffer;
	for (int i = 0; i < Data->nCount; ++i)
	{
		g_DuelMgr.AddDuelWatchUser(Data->user[i].szID);
	}
}

void ReceiveDuelResult(BYTE* ReceiveBuffer)	// 결투결과 정보
{
	LPPMSG_DUEL_RESULT_BROADCAST Data = (LPPMSG_DUEL_RESULT_BROADCAST)ReceiveBuffer;

	char szMessage[256];
	sprintf(szMessage, GlobalText[2689], 10);	// "결투가 종료되었습니다. %d초 후에 마을로 이동됩니다."
	g_pChatListBox->AddText("", szMessage, SEASON3B::TYPE_SYSTEM_MESSAGE);

	SEASON3B::CDuelResultMsgBox* lpMsgBox = NULL;
	SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CDuelResultMsgBoxLayout), &lpMsgBox);
	if(lpMsgBox)
	{
		lpMsgBox->SetIDs(Data->szWinner, Data->szLoser);
	}
	PlayBuffer(SOUND_OPEN_DUELWINDOW);
}

void ReceiveDuelRound(BYTE* ReceiveBuffer)	// 라운드 시작/끝 알림
{
	LPPMSG_DUEL_ROUNDSTART_BROADCAST Data = (LPPMSG_DUEL_ROUNDSTART_BROADCAST)ReceiveBuffer;

	if (Data->nFlag == 0)	// 라운드 시작
	{
		g_DuelMgr.SetHP(DUEL_HERO, 100);
		g_DuelMgr.SetHP(DUEL_ENEMY, 100);
		g_DuelMgr.SetSD(DUEL_HERO, 100);
		g_DuelMgr.SetSD(DUEL_ENEMY, 100);
		g_DuelMgr.SetFighterRegenerated(TRUE);
	}
	else if (Data->nFlag == 1)	// 라운드 종료
	{
	}
}

#else	// YDG_ADD_NEW_DUEL_PROTOCOL
#ifdef DUEL_SYSTEM

void ReplyDuelStart(BYTE* ReceiveBuffer)
{
	LPPRECEIVE_DUELSTART Data = (LPPRECEIVE_DUELSTART)ReceiveBuffer;
	
	if(Data->bStart)
	{
		g_bEnableDuel = true;
		g_iDuelPlayerIndex = MAKEWORD(Data->bIndexL,Data->bIndexH);
		strncpy(g_szDuelPlayerID, (const char*)Data->ID, MAX_ID_SIZE);
		g_szDuelPlayerID[10] = NULL;
		
		char szMessage[256];
		// 912 "%s 님이 결투에 응했습니다."
		sprintf(szMessage, GlobalText[912], g_szDuelPlayerID);
		g_pChatListBox->AddText("", szMessage, SEASON3B::TYPE_ERROR_MESSAGE);
		
		g_pNewUISystem->Show(SEASON3B::INTERFACE_DUEL_WINDOW);
		PlayBuffer(SOUND_START_DUEL);
	}
	else
	{
		char szMessage[256];
		strncpy(g_szDuelPlayerID, (const char*)Data->ID, MAX_ID_SIZE);
		g_szDuelPlayerID[10] = NULL;
		// 913 "%s 님이 결투신청을 거부했습니다."
		sprintf(szMessage, GlobalText[913], g_szDuelPlayerID);
		g_pChatListBox->AddText("", szMessage, SEASON3B::TYPE_ERROR_MESSAGE);
	}
}

void NotifyDuelStart(BYTE* ReceiveBuffer)
{
	if(g_MessageBox->IsEmpty() == false)
	{
		return;
	}
	
	LPPNOTIFY_DUELSTART Data = (LPPNOTIFY_DUELSTART)ReceiveBuffer;
	
	g_iDuelPlayerIndex = MAKEWORD(Data->bIndexL,Data->bIndexH);
	strncpy(g_szDuelPlayerID, (const char*)Data->ID, MAX_ID_SIZE);
	g_szDuelPlayerID[10] = NULL;
	
	if(g_pNewUISystem->IsImpossibleDuelInterface() == true)
	{
		SendRequestDuelOk(0, g_iDuelPlayerIndex, g_szDuelPlayerID);
		return;
	}
	
	SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CDuelMsgBoxLayout));
	PlayBuffer(SOUND_OPEN_DUELWINDOW);
}

void ReplyDuelEnd(BYTE* ReceiveBuffer)
{
	LPPRECEIVE_DUELEND Data = (LPPRECEIVE_DUELEND)ReceiveBuffer;
	
	g_pNewUISystem->Hide(SEASON3B::INTERFACE_DUEL_WINDOW);
	g_bEnableDuel = false;
	
	g_iDuelPlayerIndex = MAKEWORD(Data->bIndexL,Data->bIndexH);
	strncpy(g_szDuelPlayerID, (const char*)Data->ID, MAX_ID_SIZE);
	g_szDuelPlayerID[10] = NULL;
	
	g_pChatListBox->AddText("", GlobalText[914], SEASON3B::TYPE_ERROR_MESSAGE);
	
	if(g_wtMatchTimeLeft.m_Type == 2)
		g_wtMatchTimeLeft.m_Time = 0;
	g_PetEnableDuel = false;	// LHJ - 결투 끝날때 다크 스피릿도 공격 못하게 하는 변수
}

void NotifyDuelScore(BYTE* ReceiveBuffer)
{
	LPPNOTIFY_DUELSCORE Data = (LPPNOTIFY_DUELSCORE)ReceiveBuffer;
	
	if(g_iDuelPlayerIndex == MAKEWORD(Data->NumberL1, Data->NumberH1))
	{
		g_iDuelPlayerScore = Data->btDuelScore1;
		g_iMyPlayerScore = Data->btDuelScore2;
	}
	else if(g_iDuelPlayerIndex == MAKEWORD(Data->NumberL2, Data->NumberH2))
	{
		g_iDuelPlayerScore = Data->btDuelScore2;
		g_iMyPlayerScore = Data->btDuelScore1;
	}
	else
	{
		//. 먼가 데이타가 잘못온경우
	}
}

#endif // DUEL_SYSTEM
#endif	// YDG_ADD_NEW_DUEL_PROTOCOL

void ReceiveCreateShopTitleViewport(BYTE* ReceiveBuffer)
{
	LPPSHOPTITLE_HEADERINFO Header = (LPPSHOPTITLE_HEADERINFO)ReceiveBuffer;
	
	PSHOPTITLE_DATAINFO* pShopTitle = (PSHOPTITLE_DATAINFO*)(ReceiveBuffer+sizeof(PSHOPTITLE_HEADERINFO));
	for(int i=0; i<Header->byCount; i++, pShopTitle++){
		int key = MAKEWORD(pShopTitle->byIndexL, pShopTitle->byIndexH);
		int index = FindCharacterIndex(key);
		if(index >= 0 && index < MAX_CHARACTERS_CLIENT){
			CHARACTER* pPlayer = &CharactersClient[index];
			char szShopTitle[40];
			memcpy(szShopTitle, pShopTitle->szTitle, MAX_SHOPTITLE);
			szShopTitle[MAX_SHOPTITLE] = '\0';
			
			AddShopTitle(key, pPlayer, (const char*)szShopTitle);		//. 제거는 Update에서 자동으로 이루어 진다
		}
	}
}

void ReceiveShopTitleChange(BYTE* ReceiveBuffer)
{
	LPPSHOPTITLE_CHANGEINFO Header = (LPPSHOPTITLE_CHANGEINFO)ReceiveBuffer;
	
	int key = MAKEWORD(Header->byIndexL, Header->byIndexH);
	int index = FindCharacterIndex(key);
	if(index >= 0 && index < MAX_CHARACTERS_CLIENT) {
		CHARACTER* pPlayer = &CharactersClient[index];
		char szShopTitle[40];
		memcpy(szShopTitle, Header->szTitle, MAX_SHOPTITLE);
		szShopTitle[MAX_SHOPTITLE] = '\0';
		
		if(strncmp(pPlayer->ID, (const char*)Header->szId, MAX_ID_SIZE) == 0)
			AddShopTitle(key, pPlayer, (const char*)szShopTitle);		//. 제거는 Update에서 자동으로 이루어 진다
	}
}

void ReceiveSetPriceResult(BYTE* ReceiveBuffer)
{
	LPPSHOPSETPRICE_RESULTINFO Header = (LPPSHOPSETPRICE_RESULTINFO)ReceiveBuffer;
	
	if(Header->byResult != 0x01 && g_IsPurchaseShop == PSHOPWNDTYPE_SALE) 
	{
		// Header->byResult == 0x06 : 아이템 블럭계정
		if(SEASON3B::CNewUIInventoryCtrl::GetPickedItem())
		{
			SEASON3B::CNewUIInventoryCtrl::DeletePickedItem();
		}
		RemovePersonalItemPrice(MAX_MY_INVENTORY_INDEX+g_pMyShopInventory->GetTargetIndex(), PSHOPWNDTYPE_SALE);		//. 등록 취소
		
		SendRequestInventory();		//. 혹시 꼬였을지도 모르니 인벤을 새로 받아온다
		
		g_ErrorReport.Write("@ [Fault] ReceiveSetPriceResult (result : %d)\n", Header->byResult);
	}
}

void ReceiveCreatePersonalShop(BYTE* ReceiveBuffer)
{
	LPCREATEPSHOP_RESULSTINFO Header = (LPCREATEPSHOP_RESULSTINFO)ReceiveBuffer;
	if(Header->byResult == 0x01) 
	{
		g_pMyShopInventory->ChangePersonal( true );
		unicode::t_string title;
		g_pMyShopInventory->GetTitle( title );
		AddShopTitle(Hero->Key, Hero, title.c_str());
	}
	else 
	{
		// Header->btResult == 0x03 : 아이템 블럭계정
		g_ErrorReport.Write("@ [Fault] ReceiveCreatePersonalShop (result : %d)\n", Header->byResult);
	}
}
void ReceiveDestroyPersonalShop(BYTE* ReceiveBuffer)
{
	LPDESTROYPSHOP_RESULTINFO Header = (LPDESTROYPSHOP_RESULTINFO)ReceiveBuffer;
	if(Header->byResult == 0x01) 
	{
		int key = MAKEWORD(Header->byIndexL, Header->byIndexH);
		int index = FindCharacterIndex(key);
		if(index >= 0 && index < MAX_CHARACTERS_CLIENT) 
		{
			CHARACTER* pPlayer = &CharactersClient[index];
			if(pPlayer == Hero)
			{	//. 본인일 경우
				g_pMyShopInventory->ChangePersonal( false );
			}
			RemoveShopTitle(pPlayer);
		}
	}
	else 
	{
		g_ErrorReport.Write("@ [Fault] ReceiveDestroyPersonalShop (result : %d)\n", Header->byResult);
	}
}

void ReceivePersonalShopItemList(BYTE* ReceiveBuffer)
{
	LPGETPSHOPITEMLIST_HEADERINFO Header = (LPGETPSHOPITEMLIST_HEADERINFO)ReceiveBuffer;
	if(Header->byResult == 0x01) 
	{	
		if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE))
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_STORAGE);
		}

		if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY))
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_INVENTORY);
		}
		
		g_PersonalShopSeller.Initialize();

		unicode::t_string temp = (char*)Header->szShopTitle;
		g_pPurchaseShopInventory->ChangeTitleText( temp );
		g_pPurchaseShopInventory->GetInventoryCtrl()->RemoveAllItems();
		
		g_pNewUISystem->Show( SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY );		
		g_pNewUISystem->Show( SEASON3B::INTERFACE_INVENTORY );
		g_pMyInventory->ChangeMyShopButtonStateOpen();
		
		RemoveAllPerosnalItemPrice(PSHOPWNDTYPE_PURCHASE);	//. clear item price table
		LPGETPSHOPITEM_DATAINFO pShopItem = (LPGETPSHOPITEM_DATAINFO)(ReceiveBuffer+sizeof(GETPSHOPITEMLIST_HEADERINFO));

		for(int i=0; i<Header->byCount; i++, pShopItem++) 
		{
			if(pShopItem->iItemPrice > 0) 
			{
				int itemindex = pShopItem->byPos - (MAX_EQUIPMENT_INDEX + MAX_INVENTORY);
				g_pPurchaseShopInventory->InsertItem( itemindex, pShopItem->byItemInfo );
				AddPersonalItemPrice(pShopItem->byPos, pShopItem->iItemPrice, PSHOPWNDTYPE_PURCHASE);	//. 가격을 등록한다.
			}
			else 
			{	
#ifdef CONSOLE_DEBUG
				g_ConsoleDebug->Write(MCD_ERROR, "[ReceivePersonalShopItemList]Item Cound : %d, Item Index : %d, Item Price : %d", Header->byCount, i, pShopItem->iItemPrice);
#endif // CONSOLE_DEBUG	

				g_ErrorReport.Write("@ ReceivePersonalShopItemList - item price less than zero(%d)\n", pShopItem->iItemPrice);
				
				g_pNewUISystem->Hide(SEASON3B::INTERFACE_INVENTORY);
				g_pNewUISystem->Hide(SEASON3B::INTERFACE_MYSHOP_INVENTORY);		
				g_pNewUISystem->Hide(SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY);		
				
				return;
			}
		}
		
		int key = MAKEWORD(Header->byIndexL, Header->byIndexH);
		int index = FindCharacterIndex(key);
		
		g_pPurchaseShopInventory->ChangeShopCharacterIndex( index );
	}
	else 
	{
		switch(Header->byResult)
		{
		case 0x03:
			{
				g_pChatListBox->AddText("", GlobalText[1120], SEASON3B::TYPE_ERROR_MESSAGE);
			}	
			break;
		case 0x04:
		default:
			g_ErrorReport.Write("@ [Fault] ReceivePersonalShopItemList (result : %d)\n", Header->byResult);
		}
		
	}
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x05 [ReceivePersonalShopItemList]");
#endif // CONSOLE_DEBUG	
}

void ReceiveRefreshItemList(BYTE* ReceiveBuffer)
{
	LPGETPSHOPITEMLIST_HEADERINFO Header = (LPGETPSHOPITEMLIST_HEADERINFO)ReceiveBuffer;
	
	if (Header->byResult == 0x01 && g_IsPurchaseShop == PSHOPWNDTYPE_PURCHASE)
	{
		g_pPurchaseShopInventory->GetInventoryCtrl()->RemoveAllItems();
		
		LPGETPSHOPITEM_DATAINFO pShopItem = (LPGETPSHOPITEM_DATAINFO)(ReceiveBuffer+sizeof(GETPSHOPITEMLIST_HEADERINFO));
		for(int i=0; i<Header->byCount; i++, pShopItem++) 
		{
			int itemindex = pShopItem->byPos - (MAX_EQUIPMENT_INDEX + MAX_INVENTORY);
			g_pPurchaseShopInventory->InsertItem( itemindex, pShopItem->byItemInfo );
			AddPersonalItemPrice(pShopItem->byPos, pShopItem->iItemPrice, PSHOPWNDTYPE_PURCHASE);	//. 가격을 등록한다.
		}
	}
	else
	{
#ifdef LDS_ADD_OUTPUTERRORLOG_WHEN_RECEIVEREFRESHPERSONALSHOPITEM
		if( Header->byResult == 0x01 )
		{
			auto pCurrentInvenCtrl = g_pPurchaseShopInventory->GetInventoryCtrl();
			
			size_t uiCntInvenCtrl = pCurrentInvenCtrl->GetNumberOfItems();
			g_ErrorReport.Write("@ [Notice] ReceiveRefreshItemList (InventoryCtrl Count Items(%d))\n", uiCntInvenCtrl );
		}
		else
		{
			g_ErrorReport.Write("@ [Fault] ReceiveRefreshItemList (result : %d)\n", Header->byResult);
		}
#else // LDS_ADD_OUTPUTERRORLOG_WHEN_RECEIVEREFRESHPERSONALSHOPITEM
		g_ErrorReport.Write("@ [Fault] ReceiveRefreshItemList (result : %d)\n", Header->byResult);
#endif // LDS_ADD_OUTPUTERRORLOG_WHEN_RECEIVEREFRESHPERSONALSHOPITEM
	}
}


void ReceivePurchaseItem(BYTE* ReceiveBuffer)
{
	LPPURCHASEITEM_RESULTINFO Header = (LPPURCHASEITEM_RESULTINFO)ReceiveBuffer;
	
	if(Header->byResult == 0x01)
	{
		if( g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY) )
		{			
			RemovePersonalItemPrice(g_pPurchaseShopInventory->GetSourceIndex(), PSHOPWNDTYPE_PURCHASE);
			g_pPurchaseShopInventory->DeleteItem(g_pPurchaseShopInventory->GetSourceIndex());
			
			int myinvenitemindex = Header->byPos - MAX_EQUIPMENT_INDEX;
			g_pMyInventory->InsertItem( myinvenitemindex, Header->byItemInfo );
		}
		else 
		{	
			RemoveAllPerosnalItemPrice(PSHOPWNDTYPE_PURCHASE);
			int myinvenitemindex = Header->byPos - MAX_EQUIPMENT_INDEX;
			g_pMyInventory->InsertItem( myinvenitemindex, Header->byItemInfo );
		}
	}
	else if(Header->byResult == 0x06)
	{
		g_pChatListBox->AddText("", GlobalText[1166], SEASON3B::TYPE_ERROR_MESSAGE);
		g_pNewUISystem->Hide( SEASON3B::INTERFACE_MYSHOP_INVENTORY );		
		g_pNewUISystem->Hide( SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY );	
	}
	else
	{
		switch(Header->byResult)
		{
		case 0x07:
			{
				g_pChatListBox->AddText("", GlobalText[423], SEASON3B::TYPE_ERROR_MESSAGE);
			}
			break;
		case 0x08:
			{
				g_pChatListBox->AddText("", GlobalText[375], SEASON3B::TYPE_ERROR_MESSAGE);
				
			}
			break;
		case 0x09:
		default:
			g_ErrorReport.Write("@ [Fault] ReceivePurchaseItem (result : %d)\n", Header->byResult);
		}
		SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
	}
}

void NotifySoldItem(BYTE* ReceiveBuffer)
{
	LPSOLDITEM_RESULTINFO Header = (LPSOLDITEM_RESULTINFO)ReceiveBuffer;
	char szId[MAX_ID_SIZE+2]= {0,};
	strncpy(szId, (char*)Header->szId, MAX_ID_SIZE);
	szId[MAX_ID_SIZE]= '\0';
	
	char Text[100];
	sprintf(Text, GlobalText[1122], szId);
	g_pChatListBox->AddText("", Text, SEASON3B::TYPE_SYSTEM_MESSAGE);
}

void NotifyClosePersonalShop(BYTE* ReceiveBuffer)
{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	LPCLOSEPSHOP_RESULTINFO Header = (LPCLOSEPSHOP_RESULTINFO)ReceiveBuffer;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	if( g_IsPurchaseShop == PSHOPWNDTYPE_PURCHASE )
	{
		g_pNewUISystem->Hide( SEASON3B::INTERFACE_MYSHOP_INVENTORY );		
		g_pNewUISystem->Hide( SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY );	
		
		g_pChatListBox->AddText("", GlobalText[1126], SEASON3B::TYPE_ERROR_MESSAGE);
	}
}

void ReceiveDisplayEffectViewport(BYTE* ReceiveBuffer)
{
	LPDISPLAYEREFFECT_NOTIFYINFO Header = (LPDISPLAYEREFFECT_NOTIFYINFO)ReceiveBuffer;
	
	int key = MAKEWORD(Header->byIndexL, Header->byIndexH);
	int index = FindCharacterIndex(key);
	if(index >= 0 && index < MAX_CHARACTERS_CLIENT){
		CHARACTER* pPlayer = &CharactersClient[index];
		OBJECT* o = &pPlayer->Object;
		if(o->Kind == KIND_PLAYER) {
			switch(Header->byType) {
			case 0x01:	//. HP up
				{
#ifdef ENABLE_POTION_EFFECT
					CreateEffect(BITMAP_MAGIC+1,o->Position,o->Angle,o->Light,5,o);
#endif // ENABLE_POTION_EFFECT
				}
				break;
			case 0x02:	//. MP up
				{
				}
				break;
			case 0x10:	//. Level up
				{
					if(IsMasterLevel(pPlayer->Class) == true)
					{
						CreateJoint(BITMAP_FLARE,o->Position,o->Position,o->Angle,45,o,80,2);
						for ( int i=0; i<19; ++i )
						{
							CreateJoint(BITMAP_FLARE,o->Position,o->Position,o->Angle,46,o,80,2);
						}
					}
					else
					{
						for ( int i=0; i<15; ++i )
						{
							CreateJoint(BITMAP_FLARE,o->Position,o->Position,o->Angle,0,o,40,2);
						}
						CreateEffect(BITMAP_MAGIC+1,o->Position,o->Angle,o->Light,0,o);
					}
					PlayBuffer(SOUND_LEVEL_UP);
				}
				break;
			case 0x03:
				{
					CreateEffect(BITMAP_MAGIC+1,o->Position,o->Angle,o->Light,5,o);
				}
				break;
				
			case 0x11:
				{
					if(!InChaosCastle())
					{
						CreateEffect(MODEL_SHIELD_CRASH, o->Position, o->Angle, o->Light, 0, o);
						PlayBuffer(SOUND_SHIELDCLASH);
					}
				}
				break;
#ifdef _PVP_ADD_MOVE_SCROLL
			case 0x20:	// move
				{
					CreateEffect(BITMAP_MAGIC+1,o->Position,o->Angle,o->Light,6,o);
				}
				break;
#endif	// _PVP_ADD_MOVE_SCROLL
			}	//. end of switch
		}	//. end of (o->Kind == KIND_PLAYER)
	}
}

int g_iMaxLetterCount = 0;	// 최대 편지 수

// 캐릭터선택씬에서 캐릭터를 선택시에 호출
void ReceiveFriendList(BYTE* ReceiveBuffer)				// 친구 리스트 받기
{
#ifdef KJH_FIX_WOPS_K29708_SHARE_LETTER
	g_pWindowMgr->Reset();						// 친구창,창목록,편지리스트 초기화
#else KJH_FIX_WOPS_K29708_SHARE_LETTER
	g_pFriendList->ClearFriendList();
	
#ifdef KJH_FIX_WOPS_K22448_SHARED_CHARACTER_MEMOLIST
	g_pLetterList->ClearLetterList();			// 편지리스트 초기화
	g_pWindowMgr->RefreshMainWndLetterList();	// 편지리스트 갱신
#endif // KJH_FIX_WOPS_K22448_SHARED_CHARACTER_MEMOLIST
#endif // KJH_FIX_WOPS_K29708_SHARE_LETTER
	
	LPFS_FRIEND_LIST_HEADER Header = (LPFS_FRIEND_LIST_HEADER)ReceiveBuffer;
	int iMoveOffset = sizeof(FS_FRIEND_LIST_HEADER);
	char szName[MAX_ID_SIZE + 1] = {0};
	for (int i = 0; i < Header->Count; ++i)
	{
		LPFS_FRIEND_LIST_DATA Data = (LPFS_FRIEND_LIST_DATA)(ReceiveBuffer+iMoveOffset);
		strncpy(szName, (const char *)Data->Name, MAX_ID_SIZE);
		szName[MAX_ID_SIZE] = '\0';
		g_pFriendList->AddFriend(szName, 0, Data->Server);
		iMoveOffset += sizeof(FS_FRIEND_LIST_DATA);
	}
	g_pFriendList->Sort(0);
	g_pFriendList->Sort(1);
	g_pWindowMgr->RefreshMainWndPalList();
	
	// 채팅 서버 살아남
	g_pWindowMgr->SetServerEnable(TRUE);
	if (g_iChatInputType == 0) SendRequestChangeState(2);	// 구버전 채팅 사용 알림
	
	g_iMaxLetterCount = Header->MaxMemo;
	
	if (Header->MemoCount > 0)
	{
		char temp[MAX_TEXT_LENGTH + 1];
		sprintf(temp, GlobalText[1072], Header->MemoCount, Header->MaxMemo);
		g_pChatListBox->AddText("", temp, SEASON3B::TYPE_SYSTEM_MESSAGE);	// "편지함에 편지가 %d/%d개 있습니다."
	}
}

void ReceiveAddFriendResult(BYTE* ReceiveBuffer)		// 친구 등록 요청 결과
{
	LPFS_FRIEND_RESULT Data = (LPFS_FRIEND_RESULT)ReceiveBuffer;
	char szName[MAX_ID_SIZE + 1] = {0};
	strncpy(szName, (const char *)Data->Name, MAX_ID_SIZE);
	szName[MAX_ID_SIZE] = '\0';
	char szText[MAX_TEXT_LENGTH + 1] = {0};
	strncpy(szText, (const char *)Data->Name, MAX_ID_SIZE);
	szText[MAX_ID_SIZE] = '\0';
	switch(Data->Result)
	{
	case 0x00:	// 캐릭터가 없다
		strcat(szText, GlobalText[1047]);	// "은 존재하지 않는 ID입니다."
		g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, szText);
		break;
	case 0x01:	// 캐릭터 등록 성공
		{
			// "양쪽 모두 등록이 되어있지 않을 경우 [오프라인]으로 표시됩니다."
			g_pChatListBox->AddText("", GlobalText[1075], SEASON3B::TYPE_SYSTEM_MESSAGE);
			g_pFriendList->AddFriend(szName, 0, Data->Server);
			g_pFriendList->Sort();
			g_pWindowMgr->RefreshMainWndPalList();
			g_pFriendMenu->UpdateAllChatWindowInviteList();
		}
		break;
	case 0x03:	// 친구 최대수 초과
		strcpy(szText, GlobalText[1048]);	// "더이상 추가할 수 없습니다. 삭제하고 추가하세요."
		g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, szText);
		break;
	case 0x04:	// 이미 등록된 친구
		strcat(szText, GlobalText[1049]);	// "님이 이미 등록되어 있습니다."
		g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, szText);
		break;
	case 0x05:	// 자신을 등록할수 없다
		strcpy(szText, GlobalText[1050]);	// "자신의 아이디는 등록할 수 없습니다."
		g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, szText);
		break;
	case 0x06:	// 6레벨 이상만 등록 가능
		strcpy(szText, GlobalText[1068]);	// "상대방의 레벨이 6 이상이어야 가능합니다."
		g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, szText);
		break;
	default:
		break;
	};
}

void ReceiveRequestAcceptAddFriend(BYTE* ReceiveBuffer)	// 친구 등록 요청
{
	LPFS_ACCEPT_ADD_FRIEND_RESULT Data = (LPFS_ACCEPT_ADD_FRIEND_RESULT)ReceiveBuffer;
	char szName[MAX_ID_SIZE + 1] = {0};
	strncpy(szName, (const char *)Data->Name, MAX_ID_SIZE);
	szName[MAX_ID_SIZE] = '\0';
	char szText[MAX_TEXT_LENGTH + 1] = {0};
	strncpy(szText, (const char *)Data->Name, MAX_ID_SIZE);
	szText[MAX_ID_SIZE] = '\0';
	strcat(szText, GlobalText[1051]);	// "님으로 부터 친구 요청이 들어왔습니다."
	
#ifdef CSK_BUGFIX_NEWFRIEND_ADD
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_FRIEND) == false)
	{
		g_pNewUISystem->Show(SEASON3B::INTERFACE_FRIEND);
	}
#endif // CSK_BUGFIX_NEWFRIEND_ADD
	
	DWORD dwWindowID = g_pWindowMgr->AddWindow(UIWNDTYPE_QUESTION_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, szText, -1);
	if (dwWindowID != 0)
	{
		((CUIQuestionWindow *)g_pWindowMgr->GetWindow(dwWindowID))->SaveID(szName);
	}
	PlayBuffer(SOUND_FRIEND_LOGIN_ALERT);
}

void ReceiveDeleteFriendResult(BYTE* ReceiveBuffer)		// 친구 삭제 요청 결과
{
	LPFS_FRIEND_RESULT Data = (LPFS_FRIEND_RESULT)ReceiveBuffer;
	char szName[MAX_ID_SIZE + 1] = {0};
	strncpy(szName, (const char *)Data->Name, MAX_ID_SIZE);
	szName[MAX_ID_SIZE] = '\0';
	switch(Data->Result)
	{
	case 0x00:	// 삭제 실패
		g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, GlobalText[1052]);	// "삭제하지 못했습니다."
		break;
	case 0x01:	// 삭제 성공
		g_pFriendList->RemoveFriend(szName);
		g_pWindowMgr->RefreshMainWndPalList();
		break;
	default:
		break;
	};
}

void ReceiveFriendStateChange(BYTE* ReceiveBuffer)		// 친구 상태 변화
{
	LPFS_FRIEND_STATE_CHANGE Data = (LPFS_FRIEND_STATE_CHANGE)ReceiveBuffer;
	char szName[MAX_ID_SIZE + 1] = {0};
	strncpy(szName, (const char *)Data->Name, MAX_ID_SIZE);
	szName[MAX_ID_SIZE] = '\0';
	if (Data->Server == 0xFC)
	{
		// 채팅 서버 죽음
		g_pFriendList->UpdateAllFriendState(0, Data->Server);
		g_pFriendList->Sort();
		g_pWindowMgr->RefreshMainWndPalList();
		g_pFriendMenu->LockAllChatWindow();
		g_pWindowMgr->SetServerEnable(FALSE);
		return;
	}
	g_pFriendList->UpdateFriendState(szName, 0, Data->Server);
	g_pFriendList->Sort();
	g_pWindowMgr->RefreshMainWndPalList();
	g_pFriendMenu->UpdateAllChatWindowInviteList();
	
	// 채팅 중인것 처리
	DWORD dwChatRoomUIID = g_pFriendMenu->CheckChatRoomDuplication(szName);
	if (dwChatRoomUIID > 0)
	{
		CUIChatWindow * pWindow = (CUIChatWindow *)g_pWindowMgr->GetWindow(dwChatRoomUIID);
		if (pWindow == NULL);
		else if (Data->Server >= 0xFD/* || Data->Server == 0xFB*/)
		{
			// 채팅 방에 있으면 오프라인 표시
			pWindow->Lock(TRUE);
		}
		else
		{
			// 채팅방에 있으면 온라인으로 전환
			pWindow->Lock(FALSE);
		}
	}
}

void ReceiveLetterSendResult(BYTE* ReceiveBuffer)		// 편지 보낸 결과 받기
{
	LPFS_SEND_LETTER_RESULT Data = (LPFS_SEND_LETTER_RESULT)ReceiveBuffer;
	switch(Data->Result)
	{
	case 0x00:	// 보내기 실패
		if (Data->WindowGuid != 0)
			((CUILetterWriteWindow *)g_pWindowMgr->GetWindow(Data->WindowGuid))->SetSendState(FALSE);
		g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, GlobalText[1053]);	// "편지를 보내지 못했습니다. 다시 시도해 주십시오."
		break;
	case 0x01:	// 보내기 성공
		{
			if (Data->WindowGuid != 0)
				g_pWindowMgr->SendUIMessage(UI_MESSAGE_CLOSE, Data->WindowGuid, 0);
			char temp[MAX_TEXT_LENGTH + 1];
			sprintf(temp, GlobalText[1046], g_cdwLetterCost);
			g_pChatListBox->AddText("", temp, SEASON3B::TYPE_SYSTEM_MESSAGE);	// "편지를 보냈습니다."
		}
		break;
	case 0x02:	// 상대방 편지함 꽉참
		if (Data->WindowGuid != 0)
			((CUILetterWriteWindow *)g_pWindowMgr->GetWindow(Data->WindowGuid))->SetSendState(FALSE);
		g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, GlobalText[1061]);	// "받는분의 편지함이 꽉 차 편지를 보낼수 없습니다."
		break;
	case 0x03:	// 아이디 잘못 넣음
		if (Data->WindowGuid != 0)
			((CUILetterWriteWindow *)g_pWindowMgr->GetWindow(Data->WindowGuid))->SetSendState(FALSE);
		g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, GlobalText[1064]);	// "받을 상대가 없거나 편지함이 존재하지 않습니다."
		break;
	case 0x04:	// 자기 자신에게는 보낼수 없다
		if (Data->WindowGuid != 0)
			((CUILetterWriteWindow *)g_pWindowMgr->GetWindow(Data->WindowGuid))->SetSendState(FALSE);
		g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, GlobalText[1065]);	// "자기 자신에게는 편지를 보낼 수 없습니다."
		break;
	case 0x06:	// 6레벨 이상만 등록 가능
		if (Data->WindowGuid != 0)
			((CUILetterWriteWindow *)g_pWindowMgr->GetWindow(Data->WindowGuid))->SetSendState(FALSE);
		g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, GlobalText[1068]);	// "상대방의 레벨이 6 이상이어야 가능합니다."
		break;
	case 0x07:	// 젠 부족
		if (Data->WindowGuid != 0)
			((CUILetterWriteWindow *)g_pWindowMgr->GetWindow(Data->WindowGuid))->SetSendState(FALSE);
		g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, GlobalText[423]);	// "젠이 부족합니다."
		break;
	default:
		break;
	};
}

void ReceiveLetter(BYTE* ReceiveBuffer)					// 편지 받기
{
	//g_pLetterList->ClearLetterList();
	
	LPFS_LETTER_ALERT Data = (LPFS_LETTER_ALERT)ReceiveBuffer;
	char szDate[16] = {0};
	strncpy(szDate, (char *)Data->Date, 10);
	szDate[10] = '\0';
	char szTime[16] = {0};
	strncpy(szTime, (char *)Data->Date + 11, 8);
	szTime[8] = '\0';
	
	char szName[MAX_ID_SIZE + 1] = {0};
	strncpy(szName, (const char *)Data->Name, MAX_ID_SIZE);
	szName[MAX_ID_SIZE] = '\0';
	char szSubject[32 + 1] = {0};
	strncpy(szSubject, (const char *)Data->Subject, 32);
	szSubject[32] = '\0';
	
	switch (Data->Read)
	{
	case 0x02:	// 읽지않은 새 편지 도착
		PlayBuffer(SOUND_FRIEND_MAIL_ALERT);
		g_pFriendMenu->SetNewMailAlert(TRUE);
		g_pChatListBox->AddText("", GlobalText[1062], SEASON3B::TYPE_SYSTEM_MESSAGE);		// "새 편지가 도착했습니다."
		g_pLetterList->AddLetter(Data->Index, szName, szSubject, szDate, szTime, 0x00);
		g_pLetterList->Sort();
		break;
	case 0x00:	// 읽지 않은 편지
	case 0x01:	// 읽은 편지
		g_pLetterList->AddLetter(Data->Index, szName, szSubject, szDate, szTime, Data->Read);
		g_pLetterList->Sort(2);
		break;
	default:
		break;
	};
	
	g_pWindowMgr->RefreshMainWndLetterList();
	
	if (g_pLetterList->GetLetterCount() >= g_iMaxLetterCount)
	{
		g_pChatListBox->AddText("", GlobalText[1073], SEASON3B::TYPE_SYSTEM_MESSAGE);		// "편지함에 편지가 꽉 찼습니다. 정리해주세요."		
	}
}

extern int g_iLetterReadNextPos_x, g_iLetterReadNextPos_y;

void ReceiveLetterText(BYTE* ReceiveBuffer)				// 편지 내용 받기
{
	LPFS_LETTER_TEXT Data = (LPFS_LETTER_TEXT)ReceiveBuffer;
	Data->Memo[Data->MemoSize] = '\0';
	
	g_pLetterList->CacheLetterText(Data->Index, Data);
	
	LETTERLIST_TEXT * pLetter = g_pLetterList->GetLetter(Data->Index);
	if (pLetter == NULL) return;
	pLetter->m_bIsRead = TRUE;
	g_pWindowMgr->RefreshMainWndLetterList();
	
	char tempTxt[MAX_TEXT_LENGTH + 1];
	sprintf(tempTxt, GlobalText[1054], pLetter->m_szText);	// "편지읽기: %s"
	DWORD dwUIID = 0;
	if (g_iLetterReadNextPos_x == UIWND_DEFAULT)
	{
		dwUIID = g_pWindowMgr->AddWindow(UIWNDTYPE_READLETTER, 100, 100, tempTxt);
	}
	else
	{
		dwUIID = g_pWindowMgr->AddWindow(UIWNDTYPE_READLETTER, g_iLetterReadNextPos_x, g_iLetterReadNextPos_y, tempTxt, 0, UIADDWND_FORCEPOSITION);
		g_iLetterReadNextPos_x = UIWND_DEFAULT;
	}
	
	CUILetterReadWindow * pWindow = (CUILetterReadWindow *)g_pWindowMgr->GetWindow(dwUIID);
	char szText[1000 + 1] = {0};
	strncpy(szText, (const char *)Data->Memo, 1000);
	szText[1000] = '\0';
	pWindow->SetLetter(pLetter, szText);
	g_pWindowMgr->SetLetterReadWindow(pLetter->m_dwLetterID, dwUIID);
	
	if (strnicmp(pLetter->m_szID, "webzen", MAX_ID_SIZE) == 0)
	{
		pWindow->m_Photo.SetWebzenMail(TRUE);
	}
	else
	{
		pWindow->m_Photo.SetClass(ChangeServerClassTypeToClientClassType(Data->Class));
		pWindow->m_Photo.SetEquipmentPacket(Data->Equipment);
		pWindow->m_Photo.SetAnimation(Data->PhotoAction + AT_ATTACK1);
		int iAngle = Data->PhotoDir & 0x3F;
		int iZoom = (Data->PhotoDir & 0xC0) >> 6;
		pWindow->m_Photo.SetAngle(iAngle * 6);
		pWindow->m_Photo.SetZoom((iZoom * 10 + 80) / 100.0f);
	}
	pWindow->SendUIMessageDirect(UI_MESSAGE_LISTSCRLTOP, 0, 0);
}

void ReceiveLetterDeleteResult(BYTE* ReceiveBuffer)		// 편지 삭제 결과 받기
{
	LPFS_LETTER_RESULT Data = (LPFS_LETTER_RESULT)ReceiveBuffer;
	switch(Data->Result)
	{
	case 0x00:	// 삭제 실패
		g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, GlobalText[1055]);	// "편지를 삭제하지 못했습니다."
		break;
	case 0x01:	// 삭제 성공
		g_pLetterList->RemoveLetter(Data->Index);
		g_pLetterList->RemoveLetterTextCache(Data->Index);
		break;
	default:
		break;
	};
	
	g_pWindowMgr->RefreshMainWndLetterList();
}

void ReceiveCreateChatRoomResult(BYTE* ReceiveBuffer)		// 대화방 개설 요청 결과
{
	LPFS_CHAT_CREATE_RESULT Data = (LPFS_CHAT_CREATE_RESULT)ReceiveBuffer;
	char szName[MAX_ID_SIZE + 1] = {0};
	strncpy(szName, (const char *)Data->ID, MAX_ID_SIZE);
	szName[MAX_ID_SIZE] = '\0';
	char szIP[16];
	memset(szIP, 0, 16);
	memcpy(szIP, Data->IP, 15);
	switch(Data->Result)
	{
	case 0x00:	// 친구와 더이상 대화를 할 수 없습니다.
		g_pFriendMenu->RemoveRequestWindow(szName);
		g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, GlobalText[1069]);	// "더 이상 대화를 할 수 없습니다."
		break;
	case 0x01:	// 대화방 개설 성공
		if (Data->Type == 0)	// 방 만든 사람
		{
			g_pFriendMenu->RemoveRequestWindow(szName);
			DWORD dwUIID = g_pWindowMgr->AddWindow(UIWNDTYPE_CHAT, 100, 100, GlobalText[994]);	// "대화중: "
			((CUIChatWindow *)g_pWindowMgr->GetWindow(dwUIID))->ConnectToChatServer((char *)szIP, Data->RoomNumber, Data->Ticket);
		}
		else if (Data->Type == 1)	// 방에 들어올 대상 (1:1 대화  )
		{
			// 중복 채팅 체크
			DWORD dwUIID = g_pFriendMenu->CheckChatRoomDuplication(szName);
			if (dwUIID == 0)
			{
				dwUIID = g_pWindowMgr->AddWindow(UIWNDTYPE_CHAT_READY, 100, 100, GlobalText[994]);	// "대화중: "
				((CUIChatWindow *)g_pWindowMgr->GetWindow(dwUIID))->ConnectToChatServer((char *)szIP, Data->RoomNumber, Data->Ticket);
				g_pWindowMgr->GetWindow(dwUIID)->SetState(UISTATE_READY);
				g_pWindowMgr->SendUIMessage(UI_MESSAGE_BOTTOM, dwUIID, 0);
				if (g_pWindowMgr->GetFriendMainWindow() != NULL)
					g_pWindowMgr->GetFriendMainWindow()->RemoveWindow(dwUIID);
			}
			else if (dwUIID == -1);
			else
			{
				((CUIChatWindow *)g_pWindowMgr->GetWindow(dwUIID))->DisconnectToChatServer();
				((CUIChatWindow *)g_pWindowMgr->GetWindow(dwUIID))->ConnectToChatServer((char *)szIP, Data->RoomNumber, Data->Ticket);
				
				//				SendRequestCRDisconnectRoom(((CUIChatWindow *)g_pWindowMgr->GetWindow(dwUIID))->GetCurrentSocket());
				//				DWORD dwOldRoomNumber = ((CUIChatWindow *)g_pWindowMgr->GetWindow(dwUIID))->GetRoomNumber();
				//				((CUIChatWindow *)g_pWindowMgr->GetWindow(dwUIID))->ConnectToChatServer((char *)szIP, Data->RoomNumber, Data->Ticket);
				//				g_pChatRoomSocketList->RemoveChatRoomSocket(dwOldRoomNumber);
			}
		}
		else if (Data->Type == 2)	// 방에 들어올 대상 (초대  )
		{
			DWORD dwUIID = g_pWindowMgr->AddWindow(UIWNDTYPE_CHAT_READY, 100, 100, GlobalText[994]);	// "대화중: "
			((CUIChatWindow *)g_pWindowMgr->GetWindow(dwUIID))->ConnectToChatServer((char *)szIP, Data->RoomNumber, Data->Ticket);
			g_pWindowMgr->GetWindow(dwUIID)->SetState(UISTATE_READY);
			g_pWindowMgr->SendUIMessage(UI_MESSAGE_BOTTOM, dwUIID, 0);
			if (g_pWindowMgr->GetFriendMainWindow() != NULL)
				g_pWindowMgr->GetFriendMainWindow()->RemoveWindow(dwUIID);
		}
		break;
	case 0x02:	// 친구와 대화를 할 수 있는 서버가 종료되었습니다.
		g_pFriendMenu->RemoveRequestWindow(szName);
		g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, GlobalText[1070]);	// "대화 할 수 있는 서버가 종료되었습니다."
		break;
	default:
		break;
	};
}

void ReceiveChatRoomInviteResult(BYTE* ReceiveBuffer)		// 초대 결과 받기
{
	LPFS_CHAT_INVITE_RESULT Data = (LPFS_CHAT_INVITE_RESULT)ReceiveBuffer;
	CUIChatWindow * pChatWindow = (CUIChatWindow *)g_pWindowMgr->GetWindow(Data->WindowGuid);
	if (pChatWindow == NULL) return;
	
	switch(Data->Result)
	{
	case 0x00:	// 상대가 오프라인 상태
		//g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, "초대 대상이 오프라인 상태입니다.");
		pChatWindow->AddChatText(255, GlobalText[1056], 1, 0);	// "오프라인 상태입니다."
		break;
	case 0x01:	// 초대 성공
		if (pChatWindow->GetCurrentInvitePal() != NULL)
		{
			char szText[MAX_TEXT_LENGTH + 1] = {0};
			strncpy(szText, pChatWindow->GetCurrentInvitePal()->m_szID, MAX_ID_SIZE);
			szText[MAX_ID_SIZE] = '\0';
			strcat(szText, GlobalText[1057]);	// "님을 초대하였습니다."
			pChatWindow->AddChatText(255, szText, 1, 0);
		}
		else
		{
			assert(!"초대 ID가 없다");
		}
		break;
	case 0x03:	// 꽉차서 초대 불가
		pChatWindow->AddChatText(255, GlobalText[1074], 1, 0);	// "초대 가능 인원을 초과하였습니다."
		break;
	default:
		break;
	};
}

void ReceiveChatRoomConnectResult(DWORD dwWindowUIID, BYTE* ReceiveBuffer)		// 방 접속 결과 받기
{
	LPFS_CHAT_JOIN_RESULT Data = (LPFS_CHAT_JOIN_RESULT)ReceiveBuffer;
	switch(Data->Result)
	{
	case 0x00:	// 인원 꽉참
		g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, GlobalText[1058]);	// "대화방에 사람이 꽉 차서 들어갈 수 없습니다."
		break;
	case 0x01:	// 방 참여 성공
		//((CUIChatWindow *)g_pWindowMgr->GetWindow(dwWindowUIID))->AddChatText("", "대화방에 입장하였습니다.", 3, 0);
		break;
	default:
		break;
	};
}

void ReceiveChatRoomUserStateChange(DWORD dwWindowUIID, BYTE* ReceiveBuffer)	// 다른 사람 상태 변화
{
	LPFS_CHAT_CHANGE_STATE Data = (LPFS_CHAT_CHANGE_STATE)ReceiveBuffer;
	CUIChatWindow * pChatWindow = (CUIChatWindow *)g_pWindowMgr->GetWindow(dwWindowUIID);
	if (pChatWindow == NULL) return;
	char szName[MAX_ID_SIZE + 1] = {0};
	strncpy(szName, (const char *)Data->Name, MAX_ID_SIZE);
	szName[MAX_ID_SIZE] = '\0';
	char szText[MAX_TEXT_LENGTH + 1] = {0};
	strncpy(szText, (const char *)Data->Name, MAX_ID_SIZE);
	szText[MAX_ID_SIZE] = '\0';
	switch(Data->Type)
	{
	case 0x00:	// 다른 사용자가 접속했다
		if (pChatWindow->AddChatPal(szName, Data->Index, 0) >= 3)
		{
			strcat(szText, GlobalText[1059]);	// "님이 입장하셨습니다."
			pChatWindow->AddChatText(255, szText, 1, 0);
		}
		break;
	case 0x01:	// 다른 사용자가 방을 빠져 나갔다.
		if (pChatWindow->GetUserCount() >= 3)
		{
			strcat(szText, GlobalText[1060]);	// "님이 퇴장하셨습니다."
			pChatWindow->AddChatText(255, szText, 1, 0);
		}
		pChatWindow->RemoveChatPal(szName);
		break;
	default:
		return;
		break;
	};
	if (pChatWindow->GetShowType() == 2)
		pChatWindow->UpdateInvitePalList();
}

void ReceiveChatRoomUserList(DWORD dwWindowUIID, BYTE* ReceiveBuffer)			// 유저리스트 받기
{
	LPFS_CHAT_USERLIST_HEADER Header = (LPFS_CHAT_USERLIST_HEADER)ReceiveBuffer;
	int iMoveOffset = sizeof(FS_CHAT_USERLIST_HEADER);
	char szName[MAX_ID_SIZE + 1] = {0};
	for (int i = 0; i < Header->Count; ++i)
	{
		LPFS_CHAT_USERLIST_DATA Data = (LPFS_CHAT_USERLIST_DATA)(ReceiveBuffer+iMoveOffset);
		strncpy(szName, (const char *)Data->Name, MAX_ID_SIZE);
		szName[MAX_ID_SIZE] = '\0';
		((CUIChatWindow *)g_pWindowMgr->GetWindow(dwWindowUIID))->AddChatPal(szName, Data->Index, 0);
		iMoveOffset += sizeof(FS_CHAT_USERLIST_DATA);
	}
}

void ReceiveChatRoomChatText(DWORD dwWindowUIID, BYTE* ReceiveBuffer)			// 채팅 대화 글 받기
{
	LPFS_CHAT_TEXT Data = (LPFS_CHAT_TEXT)ReceiveBuffer;
	CUIChatWindow * pChatWindow = (CUIChatWindow *)g_pWindowMgr->GetWindow(dwWindowUIID);
	if (pChatWindow == NULL) return;
	
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	char ChatMsg[MAX_CHATROOM_TEXT_LENGTH] = {'\0'};
	if (Data->MsgSize >= MAX_CHATROOM_TEXT_LENGTH) return;
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	char ChatMsg[101] = {0};
	if (Data->MsgSize > 100) return;
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	memcpy(ChatMsg, Data->Msg, Data->MsgSize);
	BuxConvert((LPBYTE)ChatMsg, Data->MsgSize);
	
	if (pChatWindow->GetState() == UISTATE_READY)
	{
		g_pFriendMenu->SetNewChatAlert(dwWindowUIID);
		g_pChatListBox->AddText("", GlobalText[1063], SEASON3B::TYPE_SYSTEM_MESSAGE);	// "메시지가 왔습니다."
		pChatWindow->SetState(UISTATE_HIDE);
		if (g_pWindowMgr->GetFriendMainWindow() != NULL)
		{
			g_pWindowMgr->GetFriendMainWindow()->AddWindow(dwWindowUIID, g_pWindowMgr->GetWindow(dwWindowUIID)->GetTitle());
		}
	}
	else if (pChatWindow->GetState() == UISTATE_HIDE || g_pWindowMgr->GetTopWindowUIID() != dwWindowUIID)
	{
		g_pFriendMenu->SetNewChatAlert(dwWindowUIID);
	}
	pChatWindow->AddChatText(Data->Index, (char *)ChatMsg, 3, 0);
}

void ReceiveChatRoomNoticeText(DWORD dwWindowUIID, BYTE* ReceiveBuffer)			// 공지사항 글 받기
{
	LPFS_CHAT_TEXT Data = (LPFS_CHAT_TEXT)ReceiveBuffer;
	Data->Msg[99] = '\0';
	if (Data->Msg[0] == '\0')
	{
		return;
	}
	
	g_pChatListBox->AddText("", (char *)Data->Msg, SEASON3B::TYPE_SYSTEM_MESSAGE);
}

///////////////////////////////////////////////////////////////////////////////
// 옵션.
///////////////////////////////////////////////////////////////////////////////
void ReceiveOption(BYTE* ReceiveBuffer)
{
    LPPRECEIVE_OPTION Data = (LPPRECEIVE_OPTION)ReceiveBuffer;
	
#ifdef LDK_ADD_SCALEFORM
	//gfxui 사용시 기존 ui 사용 안함
	if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
	{
		g_pMainFrame->ResetSkillHotKey();
	}
	else
	{
		GFxProcess::GetInstancePtr()->SetSkillClearHotKey();
	}
#else //LDK_ADD_SCALEFORM
	g_pMainFrame->ResetSkillHotKey();
#endif //LDK_ADD_SCALEFORM

	int iHotKey;
	for(int i=0; i<10; ++i)
	{
#ifdef CSK_FIX_SKILLHOTKEY_PACKET
		int iIndex = i * 2;
		iHotKey = MAKEWORD(Data->HotKey[iIndex+1], Data->HotKey[iIndex]);
#else // CSK_FIX_SKILLHOTKEY_PACKET
		iHotKey = Data->HotKey[i];
#endif // CSK_FIX_SKILLHOTKEY_PACKET
				
#ifdef CSK_FIX_SKILLHOTKEY_PACKET
		if(iHotKey != 0xffff)
#else // CSK_FIX_SKILLHOTKEY_PACKET
		if(iHotKey != 0xff)
#endif // CSK_FIX_SKILLHOTKEY_PACKET
		{
			for(int j=0; j<MAX_SKILLS; ++j)
			{
				if(iHotKey == CharacterAttribute->Skill[j])
				{
#ifdef LDK_ADD_SCALEFORM
					//gfxui 사용시 기존 ui 사용 안함
					if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
					{
						g_pMainFrame->SetSkillHotKey(i, j);
					}
					else
					{
						GFxProcess::GetInstancePtr()->SetSkillHotKey(i == 0 ? 10 : i, j);
					}
#else //LDK_ADD_SCALEFORM
					g_pMainFrame->SetSkillHotKey(i, j);
#endif //LDK_ADD_SCALEFORM
					break;
				}
			}	
		}
	}
	
    //  자동공격 설정값.
    if ( (Data->GameOption&AUTOATTACK_ON)==AUTOATTACK_ON )
    {
		g_pOption->SetAutoAttack(true);
    }
    else
    {
		g_pOption->SetAutoAttack(false);
    }
	
    if ( (Data->GameOption&WHISPER_SOUND_ON)==WHISPER_SOUND_ON )
    {
		g_pOption->SetWhisperSound(true);
    }
    else
    {
		g_pOption->SetWhisperSound(false);
    }
	
    //  랜더링 옵션.
    if ( (Data->GameOption&SLIDE_HELP_OFF)==SLIDE_HELP_OFF )
    {
		g_pOption->SetSlideHelp(false);
    }
    else
    {
		g_pOption->SetSlideHelp(true);
    }
	
	BYTE byQLevel, byWLevel, byELevel, byRLevel;
	byQLevel = (Data->QWERLevel & 0xFF000000) >> 24;
	byWLevel = (Data->QWERLevel & 0x00FF0000) >> 16;
	byELevel = (Data->QWERLevel & 0x0000FF00) >> 8;
	byRLevel = Data->QWERLevel & 0x000000FF;


#ifdef LDK_ADD_SCALEFORM
	//gfxui 사용시 기존 ui 사용 안함
	if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
	{
#ifdef KJH_FIX_ITEMHOTKEYINFO_CASTING
		if( Data->KeyQWE[0] != 0xFF ) {
			g_pMainFrame->SetItemHotKey(SEASON3B::HOTKEY_Q, Data->KeyQWE[0]+ITEM_POTION, byQLevel);
		}

		if( Data->KeyQWE[1] != 0xFF ) {
			g_pMainFrame->SetItemHotKey(SEASON3B::HOTKEY_W, Data->KeyQWE[1]+ITEM_POTION, byWLevel);
		}

		if( Data->KeyQWE[2] != 0xFF ) {
			g_pMainFrame->SetItemHotKey(SEASON3B::HOTKEY_E, Data->KeyQWE[2]+ITEM_POTION, byELevel);
		}
#else // KJH_FIX_ITEMHOTKEYINFO_CASTING
		g_pMainFrame->SetItemHotKey(SEASON3B::HOTKEY_Q, Data->KeyQWE[0]+ITEM_POTION, byQLevel);
		g_pMainFrame->SetItemHotKey(SEASON3B::HOTKEY_W, Data->KeyQWE[1]+ITEM_POTION, byWLevel);
		g_pMainFrame->SetItemHotKey(SEASON3B::HOTKEY_E, Data->KeyQWE[2]+ITEM_POTION, byELevel);
#endif // KJH_FIX_ITEMHOTKEYINFO_CASTING
	}
	else
	{
		if( Data->KeyQWE[0] != 0xFF )	GFxProcess::GetInstancePtr()->SetItemHotKey(SEASON3B::HOTKEY_Q, Data->KeyQWE[0]+ITEM_POTION, byQLevel);
		if( Data->KeyQWE[1] != 0xFF )	GFxProcess::GetInstancePtr()->SetItemHotKey(SEASON3B::HOTKEY_W, Data->KeyQWE[1]+ITEM_POTION, byWLevel);
		if( Data->KeyQWE[2] != 0xFF )	GFxProcess::GetInstancePtr()->SetItemHotKey(SEASON3B::HOTKEY_E, Data->KeyQWE[2]+ITEM_POTION, byELevel);
	}
#else //LDK_ADD_SCALEFORM
#ifdef KJH_FIX_ITEMHOTKEYINFO_CASTING
	if( Data->KeyQWE[0] != 0xFF ) {
		g_pMainFrame->SetItemHotKey(SEASON3B::HOTKEY_Q, Data->KeyQWE[0]+ITEM_POTION, byQLevel);
	}

	if( Data->KeyQWE[1] != 0xFF ) {
		g_pMainFrame->SetItemHotKey(SEASON3B::HOTKEY_W, Data->KeyQWE[1]+ITEM_POTION, byWLevel);
	}

	if( Data->KeyQWE[2] != 0xFF ) {
		g_pMainFrame->SetItemHotKey(SEASON3B::HOTKEY_E, Data->KeyQWE[2]+ITEM_POTION, byELevel);
	}
#else // KJH_FIX_ITEMHOTKEYINFO_CASTING
	g_pMainFrame->SetItemHotKey(SEASON3B::HOTKEY_Q, Data->KeyQWE[0]+ITEM_POTION, byQLevel);
	g_pMainFrame->SetItemHotKey(SEASON3B::HOTKEY_W, Data->KeyQWE[1]+ITEM_POTION, byWLevel);
	g_pMainFrame->SetItemHotKey(SEASON3B::HOTKEY_E, Data->KeyQWE[2]+ITEM_POTION, byELevel);
#endif // KJH_FIX_ITEMHOTKEYINFO_CASTING
#endif //LDK_ADD_SCALEFORM
	
	//  채팅창 설정
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	BYTE wChatListBoxSize = (Data->ChatLogBox >> 4) * 3;
	BYTE wChatListBoxBackAlpha = Data->ChatLogBox & 0x0F;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	
#ifdef LDK_ADD_SCALEFORM
	//gfxui 사용시 기존 ui 사용 안함
	if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
	{
#ifdef KJH_FIX_ITEMHOTKEYINFO_CASTING
		if( Data->KeyR != 0xFF ) {
			g_pMainFrame->SetItemHotKey(SEASON3B::HOTKEY_R, Data->KeyR+ITEM_POTION, byRLevel);
		}
#else // KJH_FIX_ITEMHOTKEYINFO_CASTING
		g_pMainFrame->SetItemHotKey(SEASON3B::HOTKEY_R, Data->KeyR+ITEM_POTION, byRLevel);
#endif // KJH_FIX_ITEMHOTKEYINFO_CASTING
	}
	else
	{
		if( Data->KeyR != 0xFF ) GFxProcess::GetInstancePtr()->SetItemHotKey(SEASON3B::HOTKEY_R, Data->KeyR+ITEM_POTION, byRLevel);
	}
#else //LDK_ADD_SCALEFORM
#ifdef KJH_FIX_ITEMHOTKEYINFO_CASTING
	if( Data->KeyR != 0xFF ) {
		g_pMainFrame->SetItemHotKey(SEASON3B::HOTKEY_R, Data->KeyR+ITEM_POTION, byRLevel);
	}
#else // KJH_FIX_ITEMHOTKEYINFO_CASTING
	g_pMainFrame->SetItemHotKey(SEASON3B::HOTKEY_R, Data->KeyR+ITEM_POTION, byRLevel);
#endif // KJH_FIX_ITEMHOTKEYINFO_CASTING
#endif //LDK_ADD_SCALEFORM
}


///////////////////////////////////////////////////////////////////////////////
// 이벤트칩 관련 프로토콜( 제나칩 )
///////////////////////////////////////////////////////////////////////////////
void ReceiveEventChipInfomation( BYTE* ReceiveBuffer )
{
    LPPRECEIVE_EVENT_CHIP_INFO Data = (LPPRECEIVE_EVENT_CHIP_INFO)ReceiveBuffer;
	
	g_pNewUISystem->HideAll();

	g_bEventChipDialogEnable = Data->m_byType+1;
#ifdef ASG_FIX_LENA_REGISTRATION
	g_shEventChipCount       = Data->m_nChipCount;
#else	// ASG_FIX_LENA_REGISTRATION
	g_shEventChipCount       = Data->m_shChipCount;
#endif	// ASG_FIX_LENA_REGISTRATION
	
#ifdef PSW_GOLDBOWMAN		
	if( g_bEventChipDialogEnable == EVENT_SCRATCH_TICKET )
	{
		g_pNewUISystem->Show(SEASON3B::INTERFACE_GOLD_BOWMAN);
		g_bEventChipDialogEnable = 0;
	}	
#endif //PSW_GOLDBOWMAN
	
#ifdef PSW_EVENT_LENA		
	if( g_bEventChipDialogEnable == EVENT_LENA )
	{
		g_pNewUISystem->Show(SEASON3B::INTERFACE_GOLD_BOWMAN_LENA);
		g_bEventChipDialogEnable = 0;

		if( Data->m_shMutoNum[0]!=-1 && Data->m_shMutoNum[1]!=-1 && Data->m_shMutoNum[2]!=-1 ) {
			memcpy( g_shMutoNumber, Data->m_shMutoNum, sizeof( short )*3 );
		}
	}	
#endif //PSW_EVENT_LENA

	///////////////////////////////정리 필수///////////////////////////////////
	
#ifdef FRIEND_EVENT
    g_bEventChipDialogEnable = EVENT_FRIEND;
#endif// EVENT_FRIEND   임시 코드.
	
#ifdef BLOODCASTLE_2ND_PATCH
	if ( g_bEventChipDialogEnable==EVENT_SCRATCH_TICKET )
#endif
		
#ifdef SCRATCH_TICKET
		if ( g_bEventChipDialogEnable==EVENT_SCRATCH_TICKET )
		{
			ZeroMemory ( g_strGiftName, sizeof( char )*64 );
			
			ClearInput(FALSE);
			InputTextMax[0] = 12;
			InputNumber = 1;
			InputEnable		= false;
			GoldInputEnable = false;
			InputGold		= 0;
			StorageGoldFlag = 0;
			g_bScratchTicket= true;
		}
#endif

		///////////////////////////////정리 필수////////////////////////////////////
}

void ReceiveEventChip( BYTE* ReceiveBuffer)
{
    LPPRECEIVE_EVENT_CHIP Data = (LPPRECEIVE_EVENT_CHIP)ReceiveBuffer;
#ifdef ASG_FIX_LENA_REGISTRATION
	if( Data->m_unChipCount != 0xFFFFFFFF )
        g_shEventChipCount = Data->m_unChipCount;
#else	// ASG_FIX_LENA_REGISTRATION
    if( Data->m_shChipCount != 0xFFFF )
        g_shEventChipCount = Data->m_shChipCount;
#endif	// ASG_FIX_LENA_REGISTRATION
}

void ReceiveBuffState( BYTE* ReceiveBuffer )
{
	LPPMSG_ITEMEFFECTCANCEL Data = (LPPMSG_ITEMEFFECTCANCEL)ReceiveBuffer;
	
	eBuffState bufftype = static_cast<eBuffState>(Data->byBuffType);

	if( bufftype == eBuffNone || bufftype >= eBuff_Count ) return;
	
	// 토큰 버프류들은 서버에서 삭제 메세지를 받기 때문에 
	// 버프 추가 메세지에서 절대로 토큰 버프를 삭제 하지 않는다.
	
	if(Data->byEffectOption == 0) // 버프 시작
	{
		RegisterBuff( bufftype, (&Hero->Object), Data->wEffectTime );
		
		if( bufftype == eBuff_HelpNpc )
		{
			g_pChatListBox->AddText("", GlobalText[1828], SEASON3B::TYPE_SYSTEM_MESSAGE);
		}
	}
	else // 버프 종료
	{
		UnRegisterBuff( bufftype, (&Hero->Object) );
	}
}

void ReceiveMutoNumber( BYTE* ReceiveBuffer)
{
    LPPRECEIVE_MUTONUMBER Data = (LPPRECEIVE_MUTONUMBER)ReceiveBuffer;
	
    if( Data->m_shMutoNum[0]!=-1 && Data->m_shMutoNum[1]!=-1 && Data->m_shMutoNum[2]!=-1 )
    {
        memcpy ( g_shMutoNumber, Data->m_shMutoNum, sizeof( short )*3 );
    }
}


void ReceiveServerImmigration( BYTE *ReceiveBuffer)
{
	LPPHEADER_DEFAULT Data = (LPPHEADER_DEFAULT)ReceiveBuffer;
	
	switch ( Data->Value)
	{
	case 0:	// 주민번호 잘못됨
		SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CServerImmigrationErrorMsgBoxLayout));
		break;
	case 1:	// 서버 내부 오류
		SEASON3B::CreateOkMessageBox("서버 이전시 오류 발생 : 문의 바람");
		break;
	}
}


#ifdef SCRATCH_TICKET
void ReceiveScratchResult ( BYTE* ReceiveBuffer )
{
	LPPRECEIVE_SCRATCH_TICKET_EVENT Data = (LPPRECEIVE_SCRATCH_TICKET_EVENT)ReceiveBuffer;
	
	switch ( Data->m_byIsRegistered)
	{
	case 0: //	정상 등록.
	case 1: //	이미 등록된 시리얼번호.
	case 2: //	등록 횟수 초과.
	case 3: //	잘못된 시리얼 번호.	
	case 4:	//	알수 없는 에러. 
		SEASON3B::CreateOkMessageBox(GlobalText[886+Data->m_byIsRegistered]);
		break;
		
	case 5:	//	이미 등록을 했습니다.
		SEASON3B::CreateOkMessageBox(GlobalText[899]);
		break;
	}
	
	memcpy ( g_strGiftName, Data->m_strGiftName, sizeof( char )*64 );
}
#endif

void ReceivePlaySoundEffect(BYTE* ReceiveBuffer)	// 효과음 재생
{
	LPPRECEIVE_PLAY_SOUND_EFFECT Data = (LPPRECEIVE_PLAY_SOUND_EFFECT)ReceiveBuffer;
	
	switch(Data->wEffectNum)
	{
	case 0:
		PlayBuffer(SOUND_RING_EVENT_READY);
		break;
	case 1:
		PlayBuffer(SOUND_RING_EVENT_START);
		break;
	case 2:
		PlayBuffer(SOUND_RING_EVENT_END);
		break;
	default:
		break;
	}
}

void ReceiveEventCount ( BYTE* ReceiveBuffer )
{
    LPPRECEIVE_EVENT_COUNT Data = (LPPRECEIVE_EVENT_COUNT)ReceiveBuffer;
	
    g_csQuest.SetEventCount ( Data->m_wEventType, Data->m_wLeftEnterCount );
}


//////////////////////////////////////////////////////////////////////////
//  퀘스트 관련.
//////////////////////////////////////////////////////////////////////////
//  퀘스트 리스트를 받는다.
void ReceiveQuestHistory ( BYTE* ReceiveBuffer )
{
    LPPRECEIVE_QUEST_HISTORY Data = (LPPRECEIVE_QUEST_HISTORY)ReceiveBuffer;
	
    //  퀘스트 리스트를 저장한다 
    g_csQuest.setQuestLists ( Data->m_byQuest, Data->m_byCount, Hero->Class );
}

//  특정 퀘스트 상태 정보 전송.
void ReceiveQuestState ( BYTE* ReceiveBuffer )
{
    LPPRECEIVE_QUEST_STATE Data = (LPPRECEIVE_QUEST_STATE)ReceiveBuffer;
	
    //  퀘스트 정보 갱신.
    g_csQuest.setQuestList ( Data->m_byQuestIndex, Data->m_byState );
	g_pNewUISystem->HideAll();
	g_pNewUISystem->Show(SEASON3B::INTERFACE_NPCQUEST);
}

//  특정 퀘스트의 결과를 받는다.
void ReceiveQuestResult ( BYTE* ReceiveBuffer )
{
    LPPRECEIVE_QUEST_RESULT Data = (LPPRECEIVE_QUEST_RESULT)ReceiveBuffer;
	
    if ( Data->m_byResult==0 )
    {
        g_csQuest.setQuestList ( Data->m_byQuestIndex, Data->m_byState );
		g_pNewUISystem->HideAll();
		g_pNewUISystem->Show(SEASON3B::INTERFACE_NPCQUEST);
    }
}

//  퀘스트의 대한 보상을 받는다.
void ReceiveQuestPrize ( BYTE* ReceiveBuffer )
{
    LPPRECEIVE_QUEST_REPARATION Data = (LPPRECEIVE_QUEST_REPARATION)ReceiveBuffer;
	
	WORD Key = ((WORD)(Data->m_byKeyH)<<8) + Data->m_byKeyL;
	Key &= 0x7FFF;
    int Index = FindCharacterIndex ( Key );

    switch ( Data->m_byReparation )
    {
    case 200 :  //  스텟포인트.
        {
			CHARACTER *c = &CharactersClient[Index];
			
            //  자기 자신만..
            if ( c==Hero )
            {
                CharacterAttribute->LevelUpPoint += Data->m_byNumber;
            }
			
			OBJECT *o = &c->Object;
			
            vec3_t      Position;
            VectorCopy ( o->Position, Position );
            for ( int i=0; i<15; ++i )
            {
                CreateJoint(BITMAP_FLARE,o->Position,o->Position,o->Angle,0,o,40,2);
            }
            CreateEffect(BITMAP_MAGIC+1,o->Position,o->Angle,o->Light,0,o);
			PlayBuffer( SOUND_CHANGE_UP );
        }
        break;
		
    case 201 :  //  2차 클래스로 전직.
        {
			CHARACTER*  c = &CharactersClient[Index];
			OBJECT*     o = &c->Object;
            vec3_t      Position;
			
			BYTE byClass = ChangeServerClassTypeToClientClassType(Data->m_byNumber);
			if (2 != GetStepClass(byClass))	// 2차 클래스 전직이 아니면 처리하지 않음.
				break;
			
			c->Class = byClass;
			
            if ( Hero==c )
            {
                CharacterAttribute->Class = c->Class;
            }
			
			VectorCopy ( o->Position, Position );
			Position[2] += 200.f;
			for ( int i=0; i<15; ++i )
			{
				CreateJoint(BITMAP_FLARE,o->Position,o->Position,o->Angle,0,o,40,2);
				CreateJoint(BITMAP_FLARE,Position,Position,o->Angle,10,o,40,2);
			}
			CreateEffect(BITMAP_MAGIC+1,o->Position,o->Angle,o->Light,0,o);
			
			SetChangeClass( c );
			SetAction ( o, PLAYER_SALUTE1 );
			
			PlayBuffer ( SOUND_CHANGE_UP );
        }
        break;
		
	case 202 :	//	스텟  포인트 방식 5 --> 6.	스텟 추가.
		{
			CHARACTER*  c = &CharactersClient[Index];
			OBJECT*     o = &c->Object;
            vec3_t      Position;
			
            if ( Hero==c )
            {
                CharacterAttribute->LevelUpPoint += Data->m_byNumber;
            }
			
            VectorCopy ( o->Position, Position );
            Position[2] += 200.f;
            for ( int i=0; i<15; ++i )
            {
                CreateJoint(BITMAP_FLARE,o->Position,o->Position,o->Angle,0,o,40,2);
                CreateJoint(BITMAP_FLARE,Position,Position,o->Angle,10,o,40,2);
            }
            CreateEffect(BITMAP_MAGIC+1,o->Position,o->Angle,o->Light,0,o);
			
            SetChangeClass( c );
            SetAction ( o, PLAYER_SALUTE1 );
			PlayBuffer ( SOUND_CHANGE_UP );
		}
		break;
		
	case 203 :	//	콤보 스킬.
		{
			CHARACTER*  c = &CharactersClient[Index];
			OBJECT*     o = &c->Object;
            vec3_t      Position;
			
            VectorCopy ( o->Position, Position );
            Position[2] += 200.f;
            for ( int i=0; i<15; ++i )
            {
                CreateJoint(BITMAP_FLARE,o->Position,o->Position,o->Angle,0,o,40,2);
                CreateJoint(BITMAP_FLARE,Position,Position,o->Angle,10,o,40,2);
            }
            CreateEffect(BITMAP_MAGIC+1,o->Position,o->Angle,o->Light,0,o);
			
            SetChangeClass( c );
            SetAction ( o, PLAYER_SALUTE1 );
			
            PlayBuffer ( SOUND_CHANGE_UP );
		}
		break;
		
	case 204:	// 3차 클래스로 전직.
		{
			CHARACTER* c = &CharactersClient[Index];
			
			BYTE byClass = ChangeServerClassTypeToClientClassType(Data->m_byNumber);
			if (3 != GetStepClass(byClass))	// 3차 클래스가 아니면 처리하지 않음.
				break;
			
			c->Class = byClass;
			
            if (Hero == c)
            {
                CharacterAttribute->Class = c->Class;
            }
			
			//3차전직 효과 출력
			CreateEffect(MODEL_CHANGE_UP_EFF,c->Object.Position,c->Object.Angle,c->Object.Light,0,&c->Object);
			CreateEffect(MODEL_CHANGE_UP_NASA,c->Object.Position,c->Object.Angle,c->Object.Light,0,&c->Object);
			CreateEffect(MODEL_CHANGE_UP_CYLINDER,c->Object.Position,c->Object.Angle,c->Object.Light,0,&c->Object);
			CreateEffect(MODEL_INFINITY_ARROW3, c->Object.Position, c->Object.Angle, c->Object.Light, 1, &c->Object);
			
			SetChangeClass( c );
			SetAction(&c->Object,PLAYER_CHANGE_UP);
			PlayBuffer(SOUND_CHANGE_UP);
        }
        break;
		
    default :   //  기타.
        break;
    }
}

void ReceiveQuestMonKillInfo(BYTE* ReceiveBuffer)
{
	LPPMSG_ANS_QUEST_MONKILL_INFO pData = (LPPMSG_ANS_QUEST_MONKILL_INFO)ReceiveBuffer;
	
	/*BYTE	byResult;		// 결과 0x00:실패(에러)
	//		0x01:성공 
	//		0x02:이미 완료한 퀘스트
	//		0x03:진행중인 퀘스트 아님
	//		0x04:퀘스트가 없음(퀘스트 번호가 잘못왔음)
	//		0x05:완료조건이 킬카운트가 아님(클라이언트에서 요청하면 안됨)
	
	  BYTE	byQuestIndex;		// 퀘스트 인덱스
	  int		anKillCountInfo[10];// 몬스터 킬카운트 정보
	*/
	//	if (0x01 == pData->byResult || 0x02 == pData->byResult)
	//	{
	g_csQuest.SetKillMobInfo(pData->anKillCountInfo);
	//	}
}

#ifdef ASG_ADD_NEW_QUEST_SYSTEM
#ifdef ASG_ADD_TIME_LIMIT_QUEST
// 서버로 부터 퀘스트 제한 시 응답.(0xF6 0x00)
void ReceiveQuestLimitResult(BYTE* ReceiveBuffer)
{
	LPPMSG_ANS_QUESTEXP_RESULT pData = (LPPMSG_ANS_QUESTEXP_RESULT)ReceiveBuffer;

	g_pNewUISystem->HideAll();

	switch (pData->m_byResult)
	{
	case QUEST_RESULT_CNT_LIMIT:	// 퀘스트 개수 제한.
		SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CQuestCountLimitMsgBoxLayout));
		break;
	}
}

// 퀘스트 아이템 사용에 의한 EP(에피소드)를  .(0xF6 0x04)
void ReceiveQuestByItemUseEP(BYTE* ReceiveBuffer)
{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	LPPMSG_NPCTALK_QUESTLIST pData = (LPPMSG_NPCTALK_QUESTLIST)ReceiveBuffer;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX

	DWORD* pdwQuestIndex = (DWORD*)(ReceiveBuffer + sizeof(PMSG_NPCTALK_QUESTLIST));
	SendQuestSelection(*pdwQuestIndex, 0);
}
#endif	// ASG_ADD_TIME_LIMIT_QUEST

// 기타 상황의 퀘스트 EP(에피소드)리스트를  .(0xF6 0x03)
void ReceiveQuestByEtcEPList(BYTE* ReceiveBuffer)
{
	LPPMSG_NPCTALK_QUESTLIST pData = (LPPMSG_NPCTALK_QUESTLIST)ReceiveBuffer;
	g_QuestMng.SetQuestIndexByEtcList((DWORD*)(ReceiveBuffer + sizeof(PMSG_NPCTALK_QUESTLIST)),
		pData->m_wQuestCount);
}

// NPC에 의한 EP(에피소드)리스트를  .(0xF6 0x0A)
void ReceiveQuestByNPCEPList(BYTE* ReceiveBuffer)
{
	LPPMSG_NPCTALK_QUESTLIST pData = (LPPMSG_NPCTALK_QUESTLIST)ReceiveBuffer;
#ifdef ASG_ADD_UI_NPC_DIALOGUE
	if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPC_DIALOGUE))
		g_pNPCDialogue->ProcessQuestListReceive(
			(DWORD*)(ReceiveBuffer + sizeof(PMSG_NPCTALK_QUESTLIST)), pData->m_wQuestCount);
#else	// ASG_ADD_UI_NPC_DIALOGUE
	g_QuestMng.SetNPC(int(pData->m_wNPCIndex)); // NPC 이름 세팅.
#ifdef ASG_ADD_UI_NPC_MENU
	g_pNPCMenu->SetContents((DWORD*)(ReceiveBuffer + sizeof(PMSG_NPCTALK_QUESTLIST)),
		pData->m_wQuestCount);
#endif	// ASG_ADD_UI_NPC_MENU
#endif	// ASG_ADD_UI_NPC_DIALOGUE
}

// 선택문이 있는 QS(퀘스트 상태)를  .(0xF6 0x0B)
void ReceiveQuestQSSelSentence(BYTE* ReceiveBuffer)
{
	LPPMSG_NPC_QUESTEXP_INFO pData = (LPPMSG_NPC_QUESTEXP_INFO)ReceiveBuffer;

	g_QuestMng.SetCurQuestProgress(pData->m_dwQuestIndex);
}

// 요구, 보상이 있는 QS(퀘스트 상태)를  .(0xF6 0x0C)
void ReceiveQuestQSRequestReward(BYTE* ReceiveBuffer)
{
	LPPMSG_NPC_QUESTEXP_INFO pData = (LPPMSG_NPC_QUESTEXP_INFO)ReceiveBuffer;
	g_QuestMng.SetQuestRequestReward(ReceiveBuffer);
	g_QuestMng.SetCurQuestProgress(pData->m_dwQuestIndex);
	// 요구, 보상이 있는 QS(퀘스트 상태)이므로 true.
	g_QuestMng.SetEPRequestRewardState(pData->m_dwQuestIndex, true);
}

// 퀘스트 진행창에서 확인 버튼을 눌렀을 경우 서버에서 응답  .(0xF6 0x0D)
// 에피소드가 끝났을 경우(QS가 0xff)에 서버에서 (0xF6 0x0B)도 보냄.
void ReceiveQuestCompleteResult(BYTE* ReceiveBuffer)
{
	LPPMSG_ANS_QUESTEXP_COMPLETE pData = (LPPMSG_ANS_QUESTEXP_COMPLETE)ReceiveBuffer;

	switch (pData->m_byResult)
	{
	case 0:
		break;
	case 1:
#ifdef ASG_ADD_UI_QUEST_PROGRESS
		if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS))
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_QUEST_PROGRESS);
#endif	// ASG_ADD_UI_QUEST_PROGRESS
#ifdef ASG_ADD_UI_QUEST_PROGRESS_ETC
		if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC))
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC);
#endif	// ASG_ADD_UI_QUEST_PROGRESS_ETC

		// 퀘스트 진행이 일단락 되었으므로 EPProgress를 false로 초기화.
		g_QuestMng.SetEPRequestRewardState(pData->m_dwQuestIndex, false);
		// 진행중인 퀘스트 리스트에서 같은 에피소드인 퀘스트 인덱스 삭제.
		g_QuestMng.RemoveCurQuestIndexList(pData->m_dwQuestIndex);
		break;

	case 2:
#ifdef ASG_MOD_QUEST_OK_BTN_DISABLE
		// 퀘스트 진행창의 확인 버튼을 비활성화.
		if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS))
			g_pQuestProgress->EnableCompleteBtn(false);
		else if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC))
			g_pQuestProgressByEtc->EnableCompleteBtn(false);
#endif	// ASG_MOD_QUEST_OK_BTN_DISABLE
		// 2816	"소지금액이 꽉 찼습니다"
		g_pChatListBox->AddText("", GlobalText[2816], SEASON3B::TYPE_ERROR_MESSAGE);
		break;

	case 3:
#ifdef ASG_MOD_QUEST_OK_BTN_DISABLE
		// 퀘스트 진행창의 확인 버튼을 비활성화.
		if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS))
			g_pQuestProgress->EnableCompleteBtn(false);
		else if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC))
			g_pQuestProgressByEtc->EnableCompleteBtn(false);
#endif	// ASG_MOD_QUEST_OK_BTN_DISABLE
		// 375 "인벤토리가 꽉찼습니다."
		g_pChatListBox->AddText("", GlobalText[375], SEASON3B::TYPE_ERROR_MESSAGE);
		// 374 "확인해 주십시오!"
		g_pChatListBox->AddText("", GlobalText[374], SEASON3B::TYPE_ERROR_MESSAGE);
		break;
	}
}

// 퀘스트 포기했음을 응답  .(0xF6 0x0F)
void ReceiveQuestGiveUp(BYTE* ReceiveBuffer)
{
	LPPMSG_ANS_QUESTEXP_GIVEUP pData = (LPPMSG_ANS_QUESTEXP_GIVEUP)ReceiveBuffer;
#ifdef ASG_FIX_QUEST_GIVE_UP
	// 진행중인 퀘스트 리스트에서 해당 퀘스트 삭제.
	g_QuestMng.RemoveCurQuestIndexList(pData->m_dwQuestGiveUpIndex);
#else	// ASG_FIX_QUEST_GIVE_UP
	// 포기한 퀘스트의 인덱스(QS가 0xfe)를 추가하여 진행중인 퀘스트 리스트에서 해당 퀘스트 삭제.
	g_QuestMng.AddCurQuestIndexList(pData->m_dwQuestGiveUpIndex);
#endif	// ASG_FIX_QUEST_GIVE_UP
}

// 진행 중인 퀘스트 리스트를  .(0xF6 0x1A)
void ReceiveProgressQuestList(BYTE* ReceiveBuffer)
{
	LPPMSG_ANS_QUESTEXP_PROGRESS_LIST pData = (LPPMSG_ANS_QUESTEXP_PROGRESS_LIST)ReceiveBuffer;
	g_QuestMng.SetCurQuestIndexList((DWORD*)(ReceiveBuffer + sizeof(PMSG_ANS_QUESTEXP_PROGRESS_LIST)),
		int(pData->m_byQuestCount));
}

// 진행중인 퀘스트 리스트에서 특정 퀘스트의 요구, 보상 정보를  .(0xF6 0x1B)
void ReceiveProgressQuestRequestReward(BYTE* ReceiveBuffer)
{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	LPPMSG_NPC_QUESTEXP_INFO pData = (LPPMSG_NPC_QUESTEXP_INFO)ReceiveBuffer;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	g_QuestMng.SetQuestRequestReward(ReceiveBuffer);
#ifdef ASG_MOD_UI_QUEST_INFO
	g_pMyQuestInfoWindow->SetSelQuestRequestReward();
#endif	// ASG_MOD_UI_QUEST_INFO
}

#ifdef ASG_FIX_QUEST_PROTOCOL_ADD
// 진행 중인 퀘스트 리스트를 줄 준비가 됨.(0xF6 0x20)
void ReceiveProgressQuestListReady(BYTE* ReceiveBuffer)
{
	g_QuestMng.SetQuestIndexByEtcList(NULL, 0);	// 기타 상황에 의한 퀘스트 리스트 초기화.
	SendRequestProgressQuestList();	// 진행중인 퀘스트 리스트 요청.
	SendRequestQuestByEtcEPList();	// 기타 상황에 의한 퀘스트 리스트 요청.
}
#endif	// ASG_FIX_QUEST_PROTOCOL_ADD
#endif	// ASG_ADD_NEW_QUEST_SYSTEM

#ifdef ASG_ADD_GENS_SYSTEM
// 겐스 가입 요청 응답  .(0xF8 0x02)
void ReceiveGensJoining(BYTE* ReceiveBuffer)
{
	LPPMSG_ANS_REG_GENS_MEMBER pData = (LPPMSG_ANS_REG_GENS_MEMBER)ReceiveBuffer;
	if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPC_DIALOGUE))
		g_pNPCDialogue->ProcessGensJoiningReceive(pData->m_byResult, pData->m_byInfluence);
}

// 겐스 탈퇴 요청 응답  .(0xF8 0x04)
void ReceiveGensSecession(BYTE* ReceiveBuffer)
{
	LPPMSG_ANS_SECEDE_GENS_MEMBER pData = (LPPMSG_ANS_SECEDE_GENS_MEMBER)ReceiveBuffer;
	if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPC_DIALOGUE))
		g_pNPCDialogue->ProcessGensSecessionReceive(pData->m_byResult);
}

// 겐스 세력 정보  .(0xF8 0x07)
void ReceivePlayerGensInfluence(BYTE* ReceiveBuffer)
{
	LPPMSG_MSG_SEND_GENS_INFO pData = (LPPMSG_MSG_SEND_GENS_INFO)ReceiveBuffer;
	Hero->m_byGensInfluence = pData->m_byInfluence;
#ifdef PBG_ADD_GENSRANKING
	Hero->m_byRankIndex = pData->m_nGensClass;
	g_pNewUIGensRanking->SetContribution(pData->m_nContributionPoint);
	g_pNewUIGensRanking->SetRanking(pData->m_nRanking);
	g_pNewUIGensRanking->SetNextContribution(pData->m_nNextContributionPoint);
#endif //PBG_ADD_GENSRANKING
}

// 뷰포트 안의 다른 플래이어의 겐스 세력 정보  .(0xF8, 0x05)
void ReceiveOtherPlayerGensInfluenceViewport(BYTE* ReceiveBuffer)
{
	LPPMSG_SEND_GENS_MEMBER_VIEWPORT Data = (LPPMSG_SEND_GENS_MEMBER_VIEWPORT)ReceiveBuffer;
	int nOffset = sizeof(PMSG_SEND_GENS_MEMBER_VIEWPORT);
	int i;
	for (i = 0 ; i < Data->m_byCount ; ++i)
	{
		LPPMSG_GENS_MEMBER_VIEWPORT_INFO Data2 = (LPPMSG_GENS_MEMBER_VIEWPORT_INFO)(ReceiveBuffer+nOffset);
		int nKey = ((int)(Data2->m_byNumberH&0x7f)<<8) + Data2->m_byNumberL;
		int nIndex = FindCharacterIndex(nKey);
		CHARACTER *c = &CharactersClient[nIndex];
		
		c->m_byGensInfluence = Data2->m_byInfluence;
#ifdef PBG_ADD_GENSRANKING
		c->m_byRankIndex = Data2->m_nGensClass;
#endif //PBG_ADD_GENSRANKING
#ifdef PBG_MOD_STRIFE_GENSMARKRENDER
		// 상대방의 기여도 포인트를 필요로 하게됨
		c->m_nContributionPoint = Data2->m_nContributionPoint;
#endif //PBG_MOD_STRIFE_GENSMARKRENDER
#ifdef ASG_ADD_INFLUENCE_GROUND_EFFECT
		if (::IsStrifeMap(World))	// 분쟁지역인가?
		{
			vec3_t vTemp = {0.f, 0.f, 0.f};
			if (Hero->m_byGensInfluence == c->m_byGensInfluence)	// 나와 같은 세력인가?
				CreateEffect(BITMAP_OUR_INFLUENCE_GROUND, c->Object.Position, vTemp, vTemp, 0, &c->Object);
			else
				CreateEffect(BITMAP_ENEMY_INFLUENCE_GROUND, c->Object.Position, vTemp, vTemp, 0, &c->Object);
		}
#endif	// ASG_ADD_INFLUENCE_GROUND_EFFECT
		
		nOffset += sizeof(PMSG_GENS_MEMBER_VIEWPORT_INFO);
	}
}
#endif	// ASG_ADD_GENS_SYSTEM

#ifdef ASG_ADD_UI_NPC_DIALOGUE
// NPC UI가 대화로 진행되는 NPC 클릭 때 응답  .(0xF9 0x01)
void ReceiveNPCDlgUIStart(BYTE* ReceiveBuffer)
{
	LPPMSG_ANS_NPC_CLICK pData = (LPPMSG_ANS_NPC_CLICK)ReceiveBuffer;
	if (!g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPC_DIALOGUE))
	{
		g_QuestMng.SetNPC(pData->m_wNPCIndex); // NPC 이름 세팅.
		g_pNPCDialogue->SetContributePoint(pData->m_dwContributePoint);	// 기여도 세팅.
		g_pNewUISystem->Show(SEASON3B::INTERFACE_NPC_DIALOGUE);
	}
}
#endif	// ASG_ADD_UI_NPC_DIALOGUE

#ifdef PBG_ADD_GENSRANKING
// 보상받기 클릭시에 처리(0xF8 0x08)
void ReceiveReward(BYTE* ReceiveBuffer)
{
	LPPMSG_GENS_REWARD_CODE pData = (LPPMSG_GENS_REWARD_CODE)ReceiveBuffer;
	
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPC_DIALOGUE))
		g_pNPCDialogue->ProcessGensRewardReceive(pData->m_byRewardResult);
}
#endif //PBG_ADD_GENSRANKING
//////////////////////////////////////////////////////////////////////////
//  스테이트 열매.
//////////////////////////////////////////////////////////////////////////
void ReceiveUseStateItem ( BYTE* ReceiveBuffer )
{
	LPPMSG_USE_STAT_FRUIT Data = (LPPMSG_USE_STAT_FRUIT)ReceiveBuffer;
	
	BYTE result	= Data->result;			// 결과
	BYTE fruit	= Data->btFruitType;	// 열매종류
#ifdef PSW_FRUIT_ITEM
	WORD point	= Data->btStatValue;	// 스탯값
#else //PSW_FRUIT_ITEM
	BYTE point	= Data->btStatValue;	// 스탯값
#endif //PSW_FRUIT_ITEM
	
	unicode::t_char strText[MAX_GLOBAL_TEXT_STRING];
	
	switch(result) {
	case 0x00:	// 스탯생성 성공
		if(fruit >=0 && fruit <= 4)
		{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
			int index = 0;
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
            int  index;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
			
            switch ( fruit )
            {
            case 0: //  에너지.
				CharacterAttribute->Energy      += point;
                index = 168;
                break;
				
            case 1: //  체력.
				CharacterAttribute->Vitality    += point;
                index = 169;
                break;
				
            case 2: //  민첩.
				CharacterAttribute->Dexterity   += point;
                index = 167;
                break;
                
            case 3: //  힘.
				CharacterAttribute->Strength    += point;
                index = 166;
                break;
			case 4:	//: 통솔
				CharacterAttribute->Charisma    += point;
                index = 1900;
				break;
            }
			
            //  현재 레벨에 추가된 스텟 누적값.
            CharacterAttribute->AddPoint += point;
			
			unicode::_sprintf(strText, GlobalText[379], GlobalText[index], point, GlobalText[1412]);
			SEASON3B::CreateOkMessageBox(strText);
		}
		break;
		
	case 0x01:	// 스탯생성 실패
		SEASON3B::CreateOkMessageBox(GlobalText[378]);
		break;
		
	case 0x02:	// 열매를 Stat Plus에 사용 불가
		{
			unicode::t_char strText[128];
			unicode::_sprintf(strText, GlobalText[1904], GlobalText[1412]);
			SEASON3B::CreateOkMessageBox(strText);
		}
		break;
	case 0x03:	// 스탯감소 성공
		if(fruit >=0 && fruit <= 4)
		{
            int  index;
			
            switch ( fruit )
            {
            case 0: //  에너지.
				CharacterAttribute->Energy      -= point;
                index = 168;
                break;
				
            case 1: //  체력.
				CharacterAttribute->Vitality    -= point;
                index = 169;
                break;
				
            case 2: //  민첩.
				CharacterAttribute->Dexterity   -= point;
                index = 167;
                break;
                
            case 3: //  힘.
				CharacterAttribute->Strength    -= point;
                index = 166;
                break;
			case 4:	//: 통솔
				CharacterAttribute->Charisma    -= point;
                index = 1900;
				break;
            }
			
            CharacterAttribute->LevelUpPoint += point;
            CharacterAttribute->wMinusPoint += point;
			
			unicode::t_char strText[128];
			unicode::_sprintf(strText, GlobalText[379], GlobalText[index], point, GlobalText[1903]);
			SEASON3B::CreateOkMessageBox(strText);
		}
		break;
		
	case 0x04:	// 스탯감소 실패
		SEASON3B::CreateOkMessageBox(GlobalText[1906]);
		break;
		
	case 0x05:	// 열매를 Stat Minus에 사용 불가
		{
			unicode::t_char strText[128];
			unicode::_sprintf(strText, GlobalText[1904], GlobalText[1903]);
			SEASON3B::CreateOkMessageBox(strText);
		}
		break;
		
#ifdef PSW_FRUIT_ITEM
	case 0x06:
		if(fruit >=0 && fruit <= 4)
		{
			char Text[MAX_GLOBAL_TEXT_STRING];
            int  index;
			
            switch ( fruit )
            {
            case 0: //  에너지.
				CharacterAttribute->Energy      -= point;
                index = 168;
                break;
				
            case 1: //  체력.
				CharacterAttribute->Vitality    -= point;
                index = 169;
                break;
				
            case 2: //  민첩.
				CharacterAttribute->Dexterity   -= point;
                index = 167;
                break;
                
            case 3: //  힘.
				CharacterAttribute->Strength    -= point;
                index = 166;
                break;
			case 4:	//: 통솔
				CharacterAttribute->Charisma    -= point;
                index = 1900;
				break;
            }
			
            CharacterAttribute->LevelUpPoint += point;
			//여기는 포함 시키지 않는다.
			//마이너스 감소 열매와는 별도임.
            //CharacterAttribute->wMinusPoint += point;
			
			sprintf ( Text, GlobalText[379], GlobalText[index], point, GlobalText[1903] );
			SEASON3B::CreateOkMessageBox( Text );
		}
		break;
	case 0x07:	// 스탯감소 실패	
		SEASON3B::CreateOkMessageBox( GlobalText[1906] ); // "감소된 포인트가 없습니다."	
		break;
	case 0x08:	// 열매를 Stat Minus에 사용 불가
		char Text[MAX_GLOBAL_TEXT_STRING];
		sprintf ( Text, GlobalText[1904], GlobalText[1903]);
		SEASON3B::CreateOkMessageBox( Text ); // 1904 "해당 스테이트는 더 이상 %s 시킬 수 없습니다."
		break;
#endif //PSW_FRUIT_ITEM
		
	case 0x10:	// 아이템을 착용해서 사용 불가능
		{
			SEASON3B::CreateOkMessageBox(GlobalText[1909]);
		}
		break;
		
	case 0x21:	// 생성 최대치까지 사용 했음
		SEASON3B::CreateOkMessageBox(GlobalText[1911]);
		break;
		
	case 0x25:	// 감소 최대치까지 사용 했음
		SEASON3B::CreateOkMessageBox(GlobalText[1911]);
		break;
		
	case 0x26:	// 기본값 이하로는 감소 할 수 없음
		SEASON3B::CreateOkMessageBox(GlobalText[1912]);
		break;
	}
	
    EnableUse = 0;
}

#ifdef PET_SYSTEM
//////////////////////////////////////////////////////////////////////////
//  Pet 관련.
//////////////////////////////////////////////////////////////////////////
//  서버에서 팻 명령을 받는다.
//////////////////////////////////////////////////////////////////////////
void ReceivePetCommand ( BYTE* ReceiveBuffer )
{
	LPPRECEIVE_PET_COMMAND Data = ( LPPRECEIVE_PET_COMMAND )ReceiveBuffer;
	WORD Key = ((WORD)(Data->m_byKeyH)<<8) + Data->m_byKeyL;
	
    //  명령을 전달.
    giPetManager::SetPetCommand ( Hero, Key, Data->m_byCommand );
}


//  서버에서 공격 명령을 받는다.
//////////////////////////////////////////////////////////////////////////
void ReceivePetAttack ( BYTE* ReceiveBuffer )
{
    LPPRECEIVE_PET_ATTACK Data = ( LPPRECEIVE_PET_ATTACK )ReceiveBuffer;
    WORD Key = ((WORD)(Data->m_byKeyH)<<8) + Data->m_byKeyL;
    int  index = FindCharacterIndex ( Key );
    CHARACTER* sc = &CharactersClient[index];
	
    Key = ((WORD)(Data->m_byTKeyH)<<8) + Data->m_byTKeyL;
	
    //  공격.
    giPetManager::SetAttack ( sc, Key, Data->m_bySkillType );
}


//  팻의 정보를 받는다.
//////////////////////////////////////////////////////////////////////////
void ReceivePetInfo ( BYTE* ReceiveBuffer )
{
    LPPRECEIVE_PET_INFO Data = ( LPPRECEIVE_PET_INFO )ReceiveBuffer;
	
#ifdef KJH_FIX_DARKLOAD_PET_SYSTEM
	PET_INFO Petinfo;
	ZeroMemory( &Petinfo, sizeof(PET_INFO) );

	Petinfo.m_dwPetType = Data->m_byPetType;
	Petinfo.m_wLevel	= Data->m_byLevel;
	Petinfo.m_wLife		= Data->m_byLife;
	Petinfo.m_dwExp1	= Data->m_iExp;

	giPetManager::SetPetInfo( Data->m_byInvType, Data->m_byPos, &Petinfo );

#else // KJH_FIX_DARKLOAD_PET_SYSTEM										//## 소스정리 대상임.
#ifdef KJH_FIX_WOPS_K19787_PET_LIFE_ABNORMAL_RENDER

	PET_INFO Petinfo;
	ZeroMemory( &Petinfo, sizeof(PET_INFO) );

	Petinfo.m_dwPetType = Data->m_byPetType;
	Petinfo.m_wLevel	= Data->m_byLevel;
	Petinfo.m_wLife		= Data->m_byLife;
	Petinfo.m_dwExp1	= Data->m_iExp;
	
	giPetManager::SetPetInfo ( Data->m_byInvType, Data->m_byPos, Petinfo );

#else // KJH_FIX_WOPS_K19787_PET_LIFE_ABNORMAL_RENDER
    giPetManager::SetPetInfo ( Data->m_byPetType, Data->m_byInvType, Data->m_byPos, Data->m_byLevel, Data->m_iExp );
#endif // KJH_FIX_WOPS_K19787_PET_LIFE_ABNORMAL_RENDER

    CharacterMachine->CalculateAll();

#endif // KJH_FIX_DARKLOAD_PET_SYSTEM										//## 소스정리 대상임.
}

#endif// PET_SYSTEM

void ReceiveWTTimeLeft(BYTE* ReceiveBuffer) // 각종 경기 결과와 남은 시간 표시
{
	LPPMSG_MATCH_TIMEVIEW Data = (LPPMSG_MATCH_TIMEVIEW)ReceiveBuffer;
	g_wtMatchTimeLeft.m_Time = Data->m_Time;
	g_wtMatchTimeLeft.m_Type = Data->m_Type;
}

void ReceiveWTMatchResult(BYTE* ReceiveBuffer)
{
	LPPMSG_MATCH_RESULT	Data = (LPPMSG_MATCH_RESULT)ReceiveBuffer;
	if(Data->m_Type >= 0 && Data->m_Type < 3)
	{
		g_wtMatchResult.m_Type = Data->m_Type;
		memset(g_wtMatchResult.m_MatchTeamName1, 0, MAX_ID_SIZE);
		memcpy(g_wtMatchResult.m_MatchTeamName1, Data->m_MatchTeamName1, MAX_ID_SIZE);
		g_wtMatchResult.m_Score1 = Data->m_Score1;
		memset(g_wtMatchResult.m_MatchTeamName2, 0, MAX_ID_SIZE);
		memcpy(g_wtMatchResult.m_MatchTeamName2, Data->m_MatchTeamName2, MAX_ID_SIZE);
		g_wtMatchResult.m_Score2 = Data->m_Score2;
	}
}

void ReceiveWTBattleSoccerGoalIn(BYTE* ReceiveBuffer)
{
/*	LPPMSG_SOCCER_GOALIN Data = (LPPMSG_SOCCER_GOALIN)ReceiveBuffer;

  //정해진 위치에 이펙트 발생
  vec3_t Position, Angle, Light;
  Position[0]  = (float)(Data->m_x+0.5f)*TERRAIN_SCALE;
  Position[1]  = (float)(Data->m_y+0.5f)*TERRAIN_SCALE;
  Position[2] = RequestTerrainHeight(Position[0],Position[1]);
  ZeroMemory( Angle, sizeof ( Angle));
  Light[0] = Light[1] = Light[2] = 1.0f;
	CreateEffect(BITMAP_FIRECRACKERRISE,Position,Angle,Light);*/
} //월드 토너먼트용 결과 디스플레이

void ReceiveChangeMapServerInfo ( BYTE* ReceiveBuffer )
{
    LPPHEADER_MAP_CHANGESERVER_INFO Data = (LPPHEADER_MAP_CHANGESERVER_INFO)ReceiveBuffer;

#ifdef LDK_FIX_FULLSERVER_CONNECT_ERROR
	// 풀서버 맵이동시 클라이언트 락걸림.
	if( 0 == Data->m_vSvrInfo.m_wMapSvrPort	)
	{
#ifdef LDS_FIX_DISABLE_INPUTJUNKKEY_WHEN_LORENMARKT_EX01
		g_bReponsedMoveMapFromServer = TRUE;			// 맵이동으로서버정보받기 월드이동OK
#endif // LDS_FIX_DISABLE_INPUTJUNKKEY_WHEN_LORENMARKT_EX01
		LoadingWorld = 0;

		Teleport = false;
		return;
	}
#endif //LDK_FIX_FULLSERVER_CONNECT_ERROR

#ifdef YDG_ADD_CS5_PORTAL_CHARM
	g_PortalMgr.Reset();
#endif	// YDG_ADD_CS5_PORTAL_CHARM

    g_csMapServer.ConnectChangeMapServer( Data->m_vSvrInfo );
}


//////////////////////////////////////////////////////////////////////////
//  맵서버 이동 요청 결과.
void ReceiveChangeMapServerResult ( BYTE* ReceiveBuffer )
{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
    LPPHEADER_DEFAULT Data = (LPPHEADER_DEFAULT)ReceiveBuffer;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0xB1 [ReceiveChangeMapServerResult]");
#endif // CONSOLE_DEBUG	
}


void ReceiveBCStatus( BYTE* ReceiveBuffer )
{
	LPPMSG_ANS_CASTLESIEGESTATE Data = (LPPMSG_ANS_CASTLESIEGESTATE)ReceiveBuffer;
	
	switch( Data->btResult )
	{
	case 0x00:	// 실패
		g_pChatListBox->AddText("", GlobalText[1500], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1500 "성정보실패"
		break;
	case 0x01:	// 성공
	case 0x02:	// 성주인없음
		// 근위병 인터페이스 열기
		g_pNewUISystem->Show(SEASON3B::INTERFACE_GUARDSMAN);
		g_pGuardWindow->SetData( Data );
		break;
	case 0x03:	// 성정보비정상
		g_pChatListBox->AddText("", GlobalText[1501], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1501 "성정보비정상"	
		break;
	case 0x04:	// 성길드없어짐
		g_pChatListBox->AddText("", GlobalText[1502], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1502 "성길드없어짐"
		break;
	}
}

void ReceiveBCReg( BYTE* ReceiveBuffer )
{
	LPPMSG_ANS_REGCASTLESIEGE Data = (LPPMSG_ANS_REGCASTLESIEGE)ReceiveBuffer;
	
	switch( Data->btResult )
	{
	case 0x00:
		g_pChatListBox->AddText("", GlobalText[1503], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1503 "공성전 등록 실패"
		break;
	case 0x01:
		g_GuardsMan.SetRegStatus( 1 );
		g_pChatListBox->AddText("", GlobalText[1504], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1504 "공성전 등록 성공"
		break;
	case 0x02:
		g_pChatListBox->AddText("", GlobalText[1505], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1505 "공성전에 이미 등록되어 있습니다."
		break;
	case 0x03:
		g_pChatListBox->AddText("", GlobalText[1506], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1506 "당신은 수성측 길드입니다."
		break;
	case 0x04:
		g_pChatListBox->AddText("", GlobalText[1507], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1507 "잘못된 길드입니다."
		break;
	case 0x05:
		g_pChatListBox->AddText("", GlobalText[1508], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1508 "길드마스터의 레벨이 부족합니다."
		break;
	case 0x06:
		g_pChatListBox->AddText("", GlobalText[1509], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1509 "소속된 길드가 없습니다."
		break;
	case 0x07:
		g_pChatListBox->AddText("", GlobalText[1510], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1510 "공성전 등록기간이 아닙니다."
		break;
	case 0x08:
		g_pChatListBox->AddText("", GlobalText[1511], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1511 "길드원 수가 부족합니다."
		break;
	default:
		assert( !"잘못된 ReceiveBCReg(0xB2,0x01)" );
		break;
	}
}

void ReceiveBCGiveUp( BYTE* ReceiveBuffer )
{
	LPPMSG_ANS_GIVEUPCASTLESIEGE Data = (LPPMSG_ANS_GIVEUPCASTLESIEGE)ReceiveBuffer;
	
	switch( Data->btResult )
	{
	case 0x00:	// 실패
		g_pChatListBox->AddText("", GlobalText[1512], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1512 "공성전 포기가 실패했습니다."
		break;
	case 0x01:	// 성공
		SendRequestBCRegInfo();
		SendRequestBCDeclareGuildList();
		g_GuardsMan.SetRegStatus( 0 );
		g_pChatListBox->AddText("", GlobalText[1513], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1513 "공성전 포기가 성공했습니다."
		break;
	case 0x02:	// 등록된길드아님
		g_pChatListBox->AddText("", GlobalText[1514], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1514 "공성전에 등록된 길드가 아닙니다."
		break;
	case 0x03:	// 시간이지났음
		g_pChatListBox->AddText("", GlobalText[1515], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1515 "공성전 포기 기간이 아닙니다."
		break;
	default:
		assert( !"잘못된 ReceiveBCGiveUp(0xB2,0x02)" );
		break;
	}
}

void ReceiveBCRegInfo( BYTE* ReceiveBuffer )
{
	LPPMSG_ANS_GUILDREGINFO Data = (LPPMSG_ANS_GUILDREGINFO)ReceiveBuffer;
	
	switch( Data->btResult )
	{
	case 0x00:	// 실패
		g_GuardsMan.SetRegStatus( 0 );
		break;
	case 0x01:	// 성공
		{
			g_GuardsMan.SetRegStatus( !Data->btIsGiveUp );
			DWORD dwMarkCount;
			BYTE* pMarkCount = (BYTE*)&dwMarkCount;
			*pMarkCount++ = Data->btGuildMark4;
			*pMarkCount++ = Data->btGuildMark3;
			*pMarkCount++ = Data->btGuildMark2;
			*pMarkCount++ = Data->btGuildMark1;
			g_GuardsMan.SetMarkCount( dwMarkCount );
		}
		break;
	case 0x02:	// 등록된 길드 아님
		g_GuardsMan.SetRegStatus( 0 );
		break;
	}
}

void ReceiveBCRegMark( BYTE* ReceiveBuffer )
{
	LPPMSG_ANS_REGGUILDMARK Data = (LPPMSG_ANS_REGGUILDMARK)ReceiveBuffer;
	
	switch( Data->btResult )
	{
	case 0x00:	// 실패
		g_pChatListBox->AddText("", GlobalText[1516], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1516 "표식의 등록이 실패했습니다."
		break;
	case 0x01:	// 성공
		{
			DWORD dwMarkCount;
			BYTE* pMarkCount = (BYTE*)&dwMarkCount;
			*pMarkCount++ = Data->btGuildMark4;
			*pMarkCount++ = Data->btGuildMark3;
			*pMarkCount++ = Data->btGuildMark2;
			*pMarkCount++ = Data->btGuildMark1;
			g_GuardsMan.SetMarkCount( dwMarkCount );
		}
		break;
	case 0x02:	// 등록된길드아님
		g_pChatListBox->AddText("", GlobalText[1517], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1517 "공성전에 참여한 길드가 아닙니다."
		break;
	case 0x03:	// 잘못된아이템등록
		g_pChatListBox->AddText("", GlobalText[1518], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1518 "잘못된 아이템을 등록했습니다."
		break;
	}
}

void ReceiveBCNPCBuy( BYTE* ReceiveBuffer )
{
	LPPMSG_ANS_NPCBUY Data = (LPPMSG_ANS_NPCBUY)ReceiveBuffer;
	switch( Data->btResult )
	{
	case 0:
		g_pChatListBox->AddText("", GlobalText[1519], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1519 "구입이 실패했습니다."	
		break;
	case 1:
		g_SenatusInfo.BuyNewNPC(Data->iNpcNumber, Data->iNpcIndex);
		break;
	case 2:
		g_pChatListBox->AddText("", GlobalText[1386], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1386 "권한이 없습니다."
		break;
	case 3:
		g_pChatListBox->AddText("", GlobalText[1520], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1520 "구입비용이 모자랍니다."
		break;
	case 4:
		g_pChatListBox->AddText("", GlobalText[1616], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1616 "이미 존재합니다."
		break;
	}
}

void ReceiveBCNPCRepair( BYTE* ReceiveBuffer )
{
	LPPMSG_ANS_NPCREPAIR Data = (LPPMSG_ANS_NPCREPAIR)ReceiveBuffer;
	switch( Data->btResult )
	{
	case 0:
		{
			g_pChatListBox->AddText("", GlobalText[1519], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1519 "구입이 실패했습니다."
		}
		
		break;
	case 1:
		{
			LPPMSG_NPCDBLIST pNPCInfo = NULL;
			pNPCInfo = g_SenatusInfo.GetNPCInfo( Data->iNpcNumber, Data->iNpcIndex );
			pNPCInfo->iNpcHp = Data->iNpcHP;
			pNPCInfo->iNpcMaxHp = Data->iNpcMaxHP;
		}
		break;
	case 2:
		g_pChatListBox->AddText("", GlobalText[1386], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1386 "권한이 없습니다."
		break;
	case 3:
		g_pChatListBox->AddText("", GlobalText[1520], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1520 "구입비용이 모자랍니다."
		break;
	}
}

void ReceiveBCNPCUpgrade( BYTE* ReceiveBuffer )
{
	LPPMSG_ANS_NPCUPGRADE Data = (LPPMSG_ANS_NPCUPGRADE)ReceiveBuffer;
	switch( Data->btResult )
	{
	case 0:
		g_pChatListBox->AddText("", GlobalText[1519], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1519 "구입이 실패했습니다."	
		break;
	case 1:
		{
			LPPMSG_NPCDBLIST pNPCInfo = NULL;
			pNPCInfo = g_SenatusInfo.GetNPCInfo( Data->iNpcNumber, Data->iNpcIndex );
			if( Data->iNpcUpType == 1 )			// 방어력
				pNPCInfo->iNpcDfLevel = Data->iNpcUpValue;
			else if( Data->iNpcUpType == 2 )	// 회복력
				pNPCInfo->iNpcRgLevel = Data->iNpcUpValue;
			else if( Data->iNpcUpType == 3 )	// 내구도
				pNPCInfo->iNpcHp = pNPCInfo->iNpcMaxHp = Data->iNpcUpValue;
		}
		break;
	case 2:
		g_pChatListBox->AddText("", GlobalText[1386], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1386 "권한이 없습니다."
		break;
	case 3:
		g_pChatListBox->AddText("", GlobalText[1520], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1520 "구입비용이 모자랍니다."
		break;
	case 4:
		g_pChatListBox->AddText("", GlobalText[1521], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1521 "보석이 모자랍니다."
		break;
	case 5:
		g_pChatListBox->AddText("", GlobalText[1522], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1522 "타입이 잘못되었습니다."
		break;
	case 6:
		g_pChatListBox->AddText("", GlobalText[1523], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1523 "요청값이 잘못되었습니다."
		break;
	case 7:
		g_pChatListBox->AddText("", GlobalText[1524], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1524 "NPC가 존재하지 않습니다."
		break;
	}
}

void ReceiveBCGetTaxInfo( BYTE* ReceiveBuffer )
{
	LPPMSG_ANS_TAXMONEYINFO Data = (LPPMSG_ANS_TAXMONEYINFO)ReceiveBuffer;
	switch( Data->btResult )
	{
	case 0:
		g_pChatListBox->AddText("", GlobalText[1525], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1525 "세율정보얻기 실패"
		break;
	case 1:
		g_SenatusInfo.SetTaxInfo( Data );
		break;
	case 2:
		g_pChatListBox->AddText("", GlobalText[1386], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1386 "권한이 없습니다."
		break;
	}
}

void ReceiveBCChangeTaxRate( BYTE* ReceiveBuffer )
{
	LPPMSG_ANS_TAXRATECHANGE Data = (LPPMSG_ANS_TAXRATECHANGE)ReceiveBuffer;
	switch( Data->btResult )
	{
	case 0:
		g_pChatListBox->AddText("", GlobalText[1526], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1526 "세율정보변경 실패"		
		break;
	case 1:
        if ( Data->btTaxType==3 )   //  사냥터 입장료.
        {
			g_pUIGateKeeper->SetEntranceFee ( (Data->btTaxRate1<<24) | (Data->btTaxRate2<<16) | (Data->btTaxRate3<<8) | (Data->btTaxRate4) );
        }
        else
        {
			g_SenatusInfo.ChangeTaxInfo( Data );
        }
		break;
	case 2:
		g_pChatListBox->AddText("", GlobalText[1386], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1386 "권한이 없습니다."
		break;
	}
}

void ReceiveBCWithdraw( BYTE* ReceiveBuffer )
{
	LPPMSG_ANS_MONEYDRAWOUT Data = (LPPMSG_ANS_MONEYDRAWOUT)ReceiveBuffer;
	switch( Data->btResult )
	{
	case 0:
		g_pChatListBox->AddText("", GlobalText[1527], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1527 "출금 실패"
		break;
	case 1:
		g_SenatusInfo.ChangeCastleMoney( Data );
		break;
	case 2:
		g_pChatListBox->AddText("", GlobalText[1386], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1386 "권한이 없습니다."
		break;
	}
}

void ReceiveTaxInfo( BYTE* ReceiveBuffer )
{
	LPPMSG_ANS_MAPSVRTAXINFO Data = (LPPMSG_ANS_MAPSVRTAXINFO)ReceiveBuffer;
	// 카오스조합 세율 변경
	if( Data->btTaxType == 1 )
	{
		g_nChaosTaxRate = Data->btTaxRate;
	}
	// 상점 세율 변경
	else if( Data->btTaxType == 2 )
	{
		g_pNPCShop->SetTaxRate(Data->btTaxRate);
		g_nTaxRate = Data->btTaxRate;
	}
	else
	{
		assert( !"Packet (0xB2(0x1A))" );
	}
}

void ReceiveHuntZoneEnter ( BYTE* ReceiveBuffer )
{
    LPPMSG_CSHUNTZONEENTER pData = (LPPMSG_CSHUNTZONEENTER)ReceiveBuffer;
	
    switch ( pData->m_byResult )
    {
    case 0: //  실패.
        {
            g_pUIPopup->CancelPopup ();
            g_pUIPopup->SetPopup ( GlobalText[860], 1, 50, POPUP_OK, NULL );
        }
        break;
		
    case 1: //  성공.
        g_pUIGateKeeper->SetPublic ( pData->m_byHuntZoneEnter );
        break;
		
    case 2: //  권한이 없습니다.
        {
            g_pUIPopup->CancelPopup ();
            g_pUIPopup->SetPopup ( GlobalText[1386], 1, 50, POPUP_OK, NULL );
        }
        break;
    }
}

void ReceiveBCNPCList( BYTE* ReceiveBuffer )
{
	LPPMSG_ANS_NPCDBLIST Data = (LPPMSG_ANS_NPCDBLIST)ReceiveBuffer;
	int Offset = sizeof(PMSG_ANS_NPCDBLIST);
	
	switch( Data->btResult )
	{
	case 0:	// 실패
		g_pChatListBox->AddText("", GlobalText[860], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 860 "실패했습니다."
		break;
	case 1:	// 성공
		{
			for( int i=0 ; i<Data->iCount ; ++i )
			{
				LPPMSG_NPCDBLIST pNpcInfo = (LPPMSG_NPCDBLIST)(ReceiveBuffer+Offset);
				g_SenatusInfo.SetNPCInfo( pNpcInfo );
				Offset += sizeof(PMSG_NPCDBLIST);
			}
		}
		break;
	case 2:	// 권한이없음
		g_pChatListBox->AddText("", GlobalText[1386], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1386 "권한이 없습니다."
		break;
	}
}

void ReceiveBCDeclareGuildList( BYTE* ReceiveBuffer )
{
	LPPMSG_ANS_CSREGGUILDLIST Data = (LPPMSG_ANS_CSREGGUILDLIST)ReceiveBuffer;
	int Offset = sizeof(PMSG_ANS_CSREGGUILDLIST);
	
	switch( Data->btResult )
	{
	case 0:	// 실패
		g_pChatListBox->AddText("", GlobalText[860], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 860 "실패했습니다."
		break;
	case 1:	// 성공
		{
			g_pGuardWindow->ClearDeclareGuildList();
			for( int i=0 ; i<Data->iCount ; ++i )
			{
				LPPMSG_CSREGGUILDLIST pData2 = (LPPMSG_CSREGGUILDLIST)(ReceiveBuffer+Offset);
				
				DWORD dwMarkCount;
				BYTE* pMarkCount = (BYTE*)&dwMarkCount;
				*pMarkCount++ = pData2->btRegMarks4;
				*pMarkCount++ = pData2->btRegMarks3;
				*pMarkCount++ = pData2->btRegMarks2;
				*pMarkCount++ = pData2->btRegMarks1;
				
				g_pGuardWindow->AddDeclareGuildList( pData2->szGuildName, dwMarkCount, pData2->btIsGiveUp, pData2->btSeqNum );
				
				Offset += sizeof(PMSG_CSREGGUILDLIST);
			}
			g_pGuardWindow->SortDeclareGuildList();
		}
		break;
	}
}

void ReceiveBCGuildList( BYTE* ReceiveBuffer )
{
	LPPMSG_ANS_CSATTKGUILDLIST Data = (LPPMSG_ANS_CSATTKGUILDLIST)ReceiveBuffer;
	int Offset = sizeof(PMSG_ANS_CSATTKGUILDLIST);
	
	switch( Data->btResult )
	{
	case 0:	// 실패
		g_pChatListBox->AddText("", GlobalText[860], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 860 "실패했습니다."
		break;
	case 1:	// 성공
		{
			g_pGuardWindow->ClearGuildList();
			for( int i=0 ; i<Data->iCount ; ++i )
			{
				LPPMSG_CSATTKGUILDLIST pData2 = (LPPMSG_CSATTKGUILDLIST)(ReceiveBuffer+Offset);
				
				g_pGuardWindow->AddGuildList( pData2->szGuildName, pData2->btCsJoinSide, pData2->btGuildInvolved, pData2->iGuildScore);
				
				Offset += sizeof(PMSG_CSATTKGUILDLIST);
			}
		}
		break;
	case 2:	// 기간아님
		g_pChatListBox->AddText("", GlobalText[1609], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1609 "아직 확정되지 않았습니다."
		break;
	case 3:	// 데이터준비않됨
		g_pChatListBox->AddText("", GlobalText[1609], SEASON3B::TYPE_SYSTEM_MESSAGE);	// 1609 "아직 확정되지 않았습니다."
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//  성문 관련 프로토콜.
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//  성문 레버 선택시 사용가능 여부 및 기타 정보.
//////////////////////////////////////////////////////////////////////////
void ReceiveGateState ( BYTE* ReceiveBuffer )
{
    LPPRECEIVE_GATE_STATE Data = (LPPRECEIVE_GATE_STATE)ReceiveBuffer;
	int Key = ((int)(Data->m_byKeyH)<<8) + Data->m_byKeyL;
	
	switch( Data->m_byResult )
	{
	case 0: //  실패.
		break;
		
	case 1: //  성공.
		npcGateSwitch::DoInterfaceOpen ( Key );
		g_pNewUISystem->Show(SEASON3B::INTERFACE_GATESWITCH);
		break;
		
	case 2: //  성문이 없음.
		break;
		
	case 3: //  레버 없음.
		break;
		
	case 4: //  권한 없음.
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//  성문의 열고/닫음.
//////////////////////////////////////////////////////////////////////////
void ReceiveGateOperator ( BYTE* ReceiveBuffer )
{
    LPPRECEIVE_GATE_OPERATOR Data = (LPPRECEIVE_GATE_OPERATOR)ReceiveBuffer;
    int Key = ((int)(Data->m_byKeyH)<<8) + Data->m_byKeyL;
	
    //  성문의 상태 변경.
    npcGateSwitch::ProcessState ( Key, Data->m_byOperator, Data->m_byResult );
}


//////////////////////////////////////////////////////////////////////////
//  현재 성문의 상태가 변경되어 그 상태를 보여준다.
//////////////////////////////////////////////////////////////////////////
void ReceiveGateCurrentState ( BYTE* ReceiveBuffer )
{
    LPPRECEIVE_GATE_CURRENT_STATE Data = (LPPRECEIVE_GATE_CURRENT_STATE)ReceiveBuffer;
    int Key = ((int)(Data->m_byKeyH)<<8) + Data->m_byKeyL;
    int Index = FindCharacterIndex ( Key );
	
    CHARACTER* c = &CharactersClient[Index];
    OBJECT* o = &c->Object;
	
    if ( Data->m_byOperator )
    {
		// 성문 공격중이였다면 취소시킨다.
		if( Attacking != -1 && SelectedCharacter == Index )
		{
			Attacking = -1;
		}
        npcGateSwitch::GateOpen ( c, o );
    }
    else
    {
        npcGateSwitch::GateClose ( c, o );
    }
}


//////////////////////////////////////////////////////////////////////////
//  왕관 스위치 작동 여부.
//////////////////////////////////////////////////////////////////////////
void ReceiveCrownSwitchState ( BYTE* ReceiveBuffer )
{
    LPPRECEIVE_SWITCH_PROC pData = (LPPRECEIVE_SWITCH_PROC)ReceiveBuffer;
	
    int iKey = ((int)(pData->m_byIndexH)<<8) + pData->m_byIndexL;
    int iIndex = FindCharacterIndex ( iKey );
	
    CHARACTER* CrownSwitch = &CharactersClient[iIndex];
	
    if(CrownSwitch == NULL)
	{
		return;
	}
    if(CrownSwitch->ID == NULL)
	{
		return;
	}
	
    switch( pData->m_byState )
    {
    case 0: //  스위치 땜.
		{
			int iSwitchIndex = ((int)(pData->m_byIndexH)<<8) + pData->m_byIndexL;
			if(iSwitchIndex == FIRST_CROWN_SWITCH_NUMBER)
			{
				Switch_Info[0].Reset();
			}
			else
			{
				Switch_Info[1].Reset();
			}

			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCrownSwitchPopLayout));	
		}
        break;
		
    case 1: //  스위치 작동.
		SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCrownSwitchPushLayout));
        break;
		
    case 2: //  다른이가 스위치 작동중.
        {
			int iKey   = ((int)(pData->m_byKeyH)<<8) + pData->m_byKeyL;
            int iIndex = FindCharacterIndex ( iKey );
            CHARACTER* pCha = &CharactersClient[iIndex];
			unicode::t_char strText[256];
			
			SEASON3B::CProgressMsgBox* pMsgBox = NULL;
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCrownSwitchOtherPushLayout), &pMsgBox);
			if(pMsgBox)
			{
				if(pCha != NULL && pCha->ID != NULL)
				{
					// 1486 "%s님께서"
					unicode::_sprintf(strText, GlobalText[1486], pCha->ID);
				}
				else
				{
					// 1487 "아무개님께서"
					unicode::_sprintf(strText, GlobalText[1487]);	
				}
				pMsgBox->AddMsg(strText);
				
				// 1488 "이미 %s을 누르고 있습니다"
				unicode::_sprintf(strText, GlobalText[1488], CrownSwitch->ID);
				pMsgBox->AddMsg(strText);
			}
        }
        break;
    }
}

// 직인 등록중에는 ESC 에 의한 취소는 불가능
int DenyCrownRegistPopupClose( POPUP_RESULT Result )
{
	if( Result & POPUP_RESULT_ESC )
		return 0;
	return 1;
}

//////////////////////////////////////////////////////////////////////////
//  왕관 직인등록 여부.
//////////////////////////////////////////////////////////////////////////
void ReceiveCrownRegist ( BYTE* ReceiveBuffer )
{
    LPPRECEIVE_CROWN_STATE pData = (LPPRECEIVE_CROWN_STATE)ReceiveBuffer;
	
	g_MessageBox->PopAllMessageBoxes();
	
    switch ( pData->m_byCrownState )
    {
    case 0: //  직인 등록 시작.
		{
			//  시간 진행바를 생성시킨다.
			SEASON3B::CProgressMsgBox* pMsgBox = NULL;
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CSealRegisterStartLayout), &pMsgBox);
			if(pMsgBox)
			{
				unicode::t_char strText[256];
				// 1980 "%s (누적시간 : %d초)"
				// 1489 "직인 등록을 시작합니다"
				int iTime = (pData->m_dwCrownAccessTime/1000);
				if(iTime >= 59)
					iTime = 59;
				unicode::_sprintf(strText, GlobalText[1980], GlobalText[1489], iTime);
				pMsgBox->AddMsg(strText);
				pMsgBox->SetElapseTime(60000 - pData->m_dwCrownAccessTime);
			}
		}
        break;
		
    case 1: //  직인 등록 성공.
		SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CSealRegisterSuccessLayout));
        break;
		
    case 2: //  직인 등록 실패.
		{
			SEASON3B::CProgressMsgBox* pMsgBox = NULL;
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CSealRegisterFailLayout), &pMsgBox);
			if(pMsgBox)
			{
				unicode::t_char strText[256];
				// 1980 "%s (누적시간 : %d초)"
				// 1491 "직인 등록에 실패했습니다"
				int iTime = (pData->m_dwCrownAccessTime/1000);
				if(iTime >= 59)
					iTime = 59;
				unicode::_sprintf(strText, GlobalText[1980], GlobalText[1491], iTime);
				pMsgBox->AddMsg(strText);
			}
		}
        break;
		
    case 3: //  다른 사람 사용중.
		{
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CSealRegisterOtherLayout));
		}
        break;
	case 4:
		SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CSealRegisterOtherCampLayout));
        break;
		
    }
}


//////////////////////////////////////////////////////////////////////////
//  왕관 직인등록 여부.
//////////////////////////////////////////////////////////////////////////
void ReceiveCrownState ( BYTE* ReceiveBuffer )
{
    LPPRECEIVE_CROWN_STATE pData = (LPPRECEIVE_CROWN_STATE)ReceiveBuffer;
	
    //  이전에 띄어있던 시간바를 없앤다.
    g_pUIPopup->CancelPopup ();
    switch ( pData->m_byCrownState )
    {
    case 0: //  방어막 해제.
        {
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCrownDefenseRemoveLayout));	
			
            int Index = FindCharacterIndexByMonsterIndex ( 216 );

            OBJECT* o = &CharactersClient[Index].Object;

			g_CharacterRegisterBuff(o, eBuff_CastleCrown);
        }
        break;
		
    case 1: //  방어막 생성.
        {
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCrownDefenseCreateLayout));
			
            int Index = FindCharacterIndexByMonsterIndex ( 216 );

            OBJECT* o = &CharactersClient[Index].Object;

			g_CharacterClearBuff(o);
        }
        break;
		
    case 2: //  등록 성공.
		SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CSealRegisterSuccessLayout));
        
        break;
    }
}


//////////////////////////////////////////////////////////////////////////
//  자신의 공/수 참여 속성 알림.
//////////////////////////////////////////////////////////////////////////
void ReceiveBattleCastleRegiment ( BYTE* ReceiveBuffer )
{
    LPPRECEIVE_CROWN_STATE pData = (LPPRECEIVE_CROWN_STATE)ReceiveBuffer;
	
    switch ( pData->m_byCrownState )
    {
    case 0:
		battleCastle::SettingBattleFormation ( Hero, static_cast<eBuffState>(0) );
        return;
    case 1: //  수성.
		battleCastle::SettingBattleFormation(Hero, eBuff_CastleRegimentDefense);
		break;
    case 2: //  공성.
		battleCastle::SettingBattleFormation ( Hero, eBuff_CastleRegimentAttack1 );
        break;
	case 3: //  공성.
		battleCastle::SettingBattleFormation ( Hero, eBuff_CastleRegimentAttack2 );
		break;
	case 4: //  공성.
		battleCastle::SettingBattleFormation ( Hero, eBuff_CastleRegimentAttack3 );
		break;
    }
}


//////////////////////////////////////////////////////////////////////////
//현재 공성전 진행 상태를 알림.
//////////////////////////////////////////////////////////////////////////
void ReceiveBattleCasleSwitchInfo( BYTE* ReceiveBuffer )
{
	LPRECEIVE_CROWN_SWITCH_INFO pData = (LPRECEIVE_CROWN_SWITCH_INFO)ReceiveBuffer;
	Check_Switch (pData);
}

bool Check_Switch (PRECEIVE_CROWN_SWITCH_INFO *Data)
{
	if(Switch_Info == NULL)
	{
		Switch_Info = new CROWN_SWITCH_INFO[2];
		
		WORD Key = ((WORD)(Data->m_byIndex1)<<8) + Data->m_byIndex2;
		
		if(Key == FIRST_CROWN_SWITCH_NUMBER)
		{
			Switch_Info[0].m_bySwitchState = Data->m_bySwitchState;
			Switch_Info[0].m_JoinSide = Data->m_JoinSide;
		}
		else
		{
			Switch_Info[1].m_bySwitchState = Data->m_bySwitchState;
			Switch_Info[1].m_JoinSide = Data->m_JoinSide;
		}
	}
	else
	{
		WORD Key = ((WORD)(Data->m_byIndex1)<<8) + Data->m_byIndex2;
		
		if(Key == FIRST_CROWN_SWITCH_NUMBER)
		{
			Switch_Info[0].m_bySwitchState = Data->m_bySwitchState;
			Switch_Info[0].m_JoinSide = Data->m_JoinSide;
		}
		else
		{
			Switch_Info[1].m_bySwitchState = Data->m_bySwitchState;
			Switch_Info[1].m_JoinSide = Data->m_JoinSide;
		}
			
			char GName[8+1];
			memset(GName, 0, 8+1);
			memcpy(GName, (char*)Data->m_szGuildName, 8);
			
			GName[8] = NULL;
			
			if(Key == FIRST_CROWN_SWITCH_NUMBER)
				{
					memcpy(&Switch_Info[0].m_szGuildName, GName, 9);
				}
				else
				{
					memcpy(&Switch_Info[1].m_szGuildName, GName, 9);
				}
				
				char Name[MAX_ID_SIZE+1];
				memset(Name, 0, MAX_ID_SIZE+1);
				memcpy(Name, (char *)Data->m_szUserName, MAX_ID_SIZE);
				Name[MAX_ID_SIZE] = NULL;
				if(Key == FIRST_CROWN_SWITCH_NUMBER)
					{
						memcpy(&Switch_Info[0].m_szUserName,Name,MAX_ID_SIZE+1);
					}
					else
					{
						memcpy(&Switch_Info[1].m_szUserName,Name,MAX_ID_SIZE+1);
					}
					
	}
	return true;
}

bool Delete_Switch()
{
	if(Switch_Info != NULL)
	{
		delete [] Switch_Info;
		Switch_Info = NULL;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//  현재 공성전 시작 여부를 알림.
//////////////////////////////////////////////////////////////////////////
void ReceiveBattleCastleStart ( BYTE* ReceiveBuffer )
{
    LPPRECEIVE_CROWN_STATE pData = (LPPRECEIVE_CROWN_STATE)ReceiveBuffer;
	
	bool bStartBattleCastle = (bool)(pData->m_byCrownState);
	
    battleCastle::SetBattleCastleStart ( bStartBattleCastle );
	
	if( bStartBattleCastle )
	{
		g_pSiegeWarfare->InitSkillUI();
	}
	else
	{
		// 종료
		g_pSiegeWarfare->ReleaseSkillUI();
	}
}


//////////////////////////////////////////////////////////////////////////
//  공성전 진행 상태 알림.
//////////////////////////////////////////////////////////////////////////
void ReceiveBattleCastleProcess ( BYTE* ReceiveBuffer )
{
    LPPRECEIVE_BC_PROCESS pData = (LPPRECEIVE_BC_PROCESS)ReceiveBuffer;
	
    switch ( pData->m_byBasttleCastleState )
    {
    case 0: //  왕의 직인 등록 시도.
        {
            char GuildName[8+1];
			memset(GuildName, 0, 8+1);
            memcpy ( GuildName, (char *)pData->m_szGuildName, 8 );
            GuildName[8] = NULL;
			
            char Text[100];
            wsprintf ( Text, GlobalText[1496], GuildName );
			CreateNotice ( Text, 1 );
        }
        break;
		
    case 1: //  중간 승리.
        {
            char Text[8+1];
			memset(Text, 0, 8+1);
            memcpy ( Text, (char *)pData->m_szGuildName, 8 );
            Text[8] = NULL;
            ChangeBattleFormation ( Text, true );
			
            char Text2[100];
            wsprintf ( Text2, GlobalText[1497], Text );
            CreateNotice ( Text2, 1 );
        }
        break;
    }
}


//////////////////////////////////////////////////////////////////////////
//  마스터 스킬을 위한 KillCount값을 받는다.
//////////////////////////////////////////////////////////////////////////
void    ReceiveKillCount ( BYTE* ReceiveBuffer )
{
    LPPWHEADER_DEFAULT_WORD pData = (LPPWHEADER_DEFAULT_WORD)ReceiveBuffer;
    if ( pData->Value==0 )
    {
        if ( Hero->BackupCurrentSkill!=255 )
        {
            Hero->CurrentSkill = Hero->BackupCurrentSkill;
        }
        Hero->BackupCurrentSkill = 255;
        Hero->GuildMasterKillCount = 0;
    }
    else
    {
        Hero->GuildMasterKillCount = pData->Value;
    }
}


//////////////////////////////////////////////////////////////////////////
//  몬스터의 생성되는 시간을 받는다.
//////////////////////////////////////////////////////////////////////////
void    ReceiveBuildTime ( BYTE* ReceiveBuffer )
{
    LPPRECEIVE_MONSTER_BUILD_TIME pData = (LPPRECEIVE_MONSTER_BUILD_TIME)ReceiveBuffer;
	int Key = ((int)(pData->m_byKeyH)<<8) + pData->m_byKeyL;
	int Index = FindCharacterIndex ( Key );
	
    CHARACTER* c = &CharactersClient[Index];
    OBJECT* o = &c->Object;
	
    //  생성 시간을 얻는다.
    o->m_byBuildTime = pData->m_byBuildTime;
}


//////////////////////////////////////////////////////////////////////////
//  GC [0xB9][0x02] 성을 차지한 길드 마크.
//////////////////////////////////////////////////////////////////////////
void    ReceiveCastleGuildMark ( BYTE* ReceiveBuffer )
{
    LPPRECEIVE_CASTLE_FLAG pData = (LPPRECEIVE_CASTLE_FLAG)ReceiveBuffer;
	
	BYTE GuildMark[64];
	for(int i=0;i<64;i++)
	{
		if(i%2 == 0)
			GuildMark[i] = (pData->m_byGuildMark[i/2]>>4)&0x0f;
		else
			GuildMark[i] = pData->m_byGuildMark[i/2]&0x0f;
	}
    CreateCastleMark ( BITMAP_INTERFACE_MAP, GuildMark ); 
}


//////////////////////////////////////////////////////////////////////////
//  GC [0xB9][0x03] 성 사냥터 문지기가 그 사냥터의 여러 정보를 알려준다.
//////////////////////////////////////////////////////////////////////////
void    ReceiveCastleHuntZoneInfo ( BYTE* RecevieBuffer )
{
    LPPRECEIVE_CASTLE_HUNTZONE_INFO pData = (LPPRECEIVE_CASTLE_HUNTZONE_INFO)RecevieBuffer;
	
    if ( pData->m_byResult==0 )
    {
		SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CGatemanFailMsgBoxLayout));
    }
    else
    {
        g_pUIGateKeeper->SetInfo ( pData->m_byResult, (bool)pData->m_byEnable, pData->m_iCurrPrice, pData->m_iUnitPrice, pData->m_iMaxPrice );
		g_pNewUISystem->Show(SEASON3B::INTERFACE_GATEKEEPER);
    }
}


//////////////////////////////////////////////////////////////////////////
//  GC [0xB9][0x05] 성 사냥터 입장 결과.
//////////////////////////////////////////////////////////////////////////
void    ReceiveCastleHuntZoneResult ( BYTE* ReceiveBuffer )
{
    LPPRECEVIE_CASTLE_HUNTZONE_RESULT pData = (LPPRECEVIE_CASTLE_HUNTZONE_RESULT)ReceiveBuffer;
	
    if ( pData->m_byResult==0 )
    {
		SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CGatemanFailMsgBoxLayout));
    }
}


//////////////////////////////////////////////////////////////////////////
//  투석기 관련 프로토콜
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//  공성 무기 선택 결과.
//////////////////////////////////////////////////////////////////////////
void ReceiveCatapultState ( BYTE* ReceiveBuffer )
{
    LPPRECEIVE_CATAPULT_STATE pData = (LPPRECEIVE_CATAPULT_STATE)ReceiveBuffer;
	
    if ( pData->m_byResult==1 )
    {
        int Key = ((int)(pData->m_byKeyH)<<8) + pData->m_byKeyL;
		
		g_pNewUISystem->Show(SEASON3B::INTERFACE_CATAPULT);
		g_pCatapultWindow->Init(Key, pData->m_byWeaponType);
    }
    else if ( pData->m_byResult==0 )
    {
		g_pChatListBox->AddText( "", "공성 무기 선택 실패", SEASON3B::TYPE_SYSTEM_MESSAGE);
    }
}


//////////////////////////////////////////////////////////////////////////
//  공성무기를 발사하라는 명령을 받는다.
//////////////////////////////////////////////////////////////////////////
void    ReceiveCatapultFire ( BYTE* ReceiveBuffer )
{
    LPPRECEIVE_WEAPON_FIRE pData = (LPPRECEIVE_WEAPON_FIRE)ReceiveBuffer;
    
    if ( pData->m_byResult )
    {
        int Key = ((int)(pData->m_byKeyH)<<8) + pData->m_byKeyL;
		
        //  공격을 명령한다.
		g_pCatapultWindow->DoFire(Key, pData->m_byResult, pData->m_byWeaponType, pData->m_byTargetX, pData->m_byTargetY);
    }
    else if ( pData->m_byResult==0 )    //  실패.
    {
		g_pChatListBox->AddText( "", "공성무기 발사 실패했습니다.", SEASON3B::TYPE_SYSTEM_MESSAGE);
    }
}


//////////////////////////////////////////////////////////////////////////
//  공성무기가 자신들에게 발사되었음을 알림.
//////////////////////////////////////////////////////////////////////////
void    ReceiveCatapultFireToMe ( BYTE* ReceiveBuffer )
{
    LPPRECEIVE_BOMBING_ALERT pData = (LPPRECEIVE_BOMBING_ALERT)ReceiveBuffer;
	
	g_pCatapultWindow->DoFireFixStartPosition(pData->m_byWeaponType, pData->m_byTargetX, pData->m_byTargetY);
}


//////////////////////////////////////////////////////////////////////////
//  공성무기에 공격을 받는다. ( 공격 받는 캐릭터들만   ).
//////////////////////////////////////////////////////////////////////////
/*
void    ReceiveDamageFrmeCatapult ( BYTE* ReceiveBuffer )
{
LPPRECEIVE_BOMBING_TARGET pData = (LPPRECEIVE_BOMBING_TARGET)ReceiveBuffer;
int Key = (((int)pData->m_byKeyH)<<8) + pData->m_byKeyL;
int Index = FindCharacterIndex ( Key );
CHARACTER* tc = &CharactersClient[Index];
OBJECT* to = &tc->Object;

  switch ( pData->m_byWeaponType )
  {
  case 1:
  SetAction ( to, PLAYER_SKILL_VITALITY );// PLAYER_HIGH_SHOCK );
  break;
  }
  }
*/

//  임시적인 위치의 뷰포트 정보를 받는다. ( 공성 투척기 관련 )
void    ReceivePreviewPort ( BYTE* ReceiveBuffer )
{
	LPPWHEADER_DEFAULT_WORD pData = (LPPWHEADER_DEFAULT_WORD)ReceiveBuffer;
	int Offset = sizeof(PWHEADER_DEFAULT_WORD);
	for ( int i=0;i<pData->Value;i++ )
	{
		LPPRECEIVE_PREVIEW_PORT pData2 = (LPPRECEIVE_PREVIEW_PORT)(ReceiveBuffer+Offset);
		WORD Key       = ((WORD)(pData2->m_byKeyH)<<8) + pData2->m_byKeyL;
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		int CreateFlag = (Key>>15);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		Key &= 0x7FFF;
		
        switch ( pData2->m_byObjType )
        {
        case 1: //  캐릭터.
            {
				CHARACTER* c = CreateCharacter ( Key, MODEL_PLAYER, pData2->m_byPosX, pData2->m_byPosY, 0 );
				OBJECT* o = &c->Object;
				
				c->Class = ChangeServerClassTypeToClientClassType(pData2->m_byTypeH);
				c->Skin = 0;
				c->PK   = 0;
				o->Kind = KIND_TMP;
				
				for( int j = 0; j < pData2->s_BuffCount; ++j )
				{
					RegisterBuff(static_cast<eBuffState>(pData2->s_BuffEffectState[j]), o);
				}
                
                c->PositionX = pData2->m_byPosX;
                c->PositionY = pData2->m_byPosY;
                c->TargetX = pData2->m_byPosX;
                c->TargetY = pData2->m_byPosY;
                c->Object.Angle[2] = rand()%360;
				
                c->m_iDeleteTime = 150;
				
				ChangeCharacterExt ( FindCharacterIndex( Key ), pData2->m_byEquipment );
                
                strcpy ( c->ID, "   " );
                c->ID[MAX_ID_SIZE] = NULL;
            }
            break;
			
        case 2:    //   몬스터.
        case 3:    //   NPC
            {
                WORD Type = ((WORD)(pData2->m_byTypeH)<<8) + pData2->m_byTypeL;
				CHARACTER *c = CreateMonster ( Type, pData2->m_byPosX, pData2->m_byPosY, Key );
				if ( c==NULL ) break;
				OBJECT *o = &c->Object;
				
				for( int j = 0; j < pData2->s_BuffCount; ++j )
				{
					RegisterBuff(static_cast<eBuffState>(pData2->s_BuffEffectState[j]), o);
				}
				
                c->PositionX = pData2->m_byPosX;
                c->PositionY = pData2->m_byPosY;
                c->TargetX = pData2->m_byPosX;
                c->TargetY = pData2->m_byPosY;
                c->Object.Angle[2] = rand()%360;
				
                c->m_iDeleteTime = 150;
				
                if ( battleCastle::InBattleCastle() && c->MonsterIndex==277 )        
                {
                    //  State값에 따라 행동을 결정한다.
					
					if (g_isCharacterBuff((&c->Object), eBuff_CastleGateIsOpen))  //  성문이 열려있는 상태.
					{
						SetAction ( &c->Object, 1 );
						battleCastle::SetCastleGate_Attribute ( pData2->m_byPosX, pData2->m_byPosY, 0 );
					}
					else if(g_isNotCharacterBuff((&c->Object))) //  성문이 닫혀있는 상태. 
					{
						SetAction ( &c->Object, 0 );
						battleCastle::SetCastleGate_Attribute ( pData2->m_byPosX, pData2->m_byPosY, 1 );
					}
                }
            }
            break;
		}

		Offset += (sizeof(PRECEIVE_PREVIEW_PORT)-(sizeof(BYTE)*(MAX_BUFF_SLOT_INDEX-pData2->s_BuffCount)));
	}
}



//////////////////////////////////////////////////////////////////////////
//  맵 전략 인터페이스.
//////////////////////////////////////////////////////////////////////////
//  맵 데이터 주기적으로 보내달라는 요청 결과값.
void ReceiveMapInfoResult ( BYTE* ReceiveBuffer )
{
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
    LPPRECEIVE_MAP_INFO_RESULT pData = (LPPRECEIVE_MAP_INFO_RESULT)ReceiveBuffer;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
}

//  (연합)길드 마스터의 명령을 받는다.
void ReceiveGuildCommand ( BYTE* ReceiveBuffer )
{
    LPPRECEIVE_GUILD_COMMAND  pData = (LPPRECEIVE_GUILD_COMMAND)ReceiveBuffer;
    GuildCommander            GCmd = { pData->m_byTeam, pData->m_byX, pData->m_byY, pData->m_byCmd };
	
	if( g_pSiegeWarfare )
	{
		g_pSiegeWarfare->SetMapInfo( GCmd );
	}
}

//  (연합)길드원들의 위치를 받는다.
void ReceiveGuildMemberLocation ( BYTE* ReceiveBuffer )
{
	if( g_pSiegeWarfare->GetCurSiegeWarType() != TYPE_GUILD_COMMANDER )
		return;
	
	g_pSiegeWarfare->ClearGuildMemberLocation( );
	
    LPPWHEADER_DEFAULT_WORD2 pData = (LPPWHEADER_DEFAULT_WORD2)ReceiveBuffer;
	int Offset = sizeof(PWHEADER_DEFAULT_WORD2);
	
	for ( int i=0; i<pData->Value; i++ )
	{
		LPPRECEIVE_MEMBER_LOCATION pData2 = (LPPRECEIVE_MEMBER_LOCATION)(ReceiveBuffer+Offset);
		
		g_pSiegeWarfare->SetGuildMemberLocation( 0, pData2->m_byX, pData2->m_byY );
		
		Offset += sizeof(PRECEIVE_MEMBER_LOCATION);
	}
}


//  (연합)길드에 포함되는 NPC를 받는다.
void ReceiveGuildNpcLocation ( BYTE* ReceiveBuffer )
{
	if( g_pSiegeWarfare->GetCurSiegeWarType() != TYPE_GUILD_COMMANDER )
		return;
	
    LPPWHEADER_DEFAULT_WORD pData = (LPPWHEADER_DEFAULT_WORD)ReceiveBuffer;
	int Offset = sizeof(PWHEADER_DEFAULT_WORD);
	
	for ( int i=0; i<pData->Value; i++ )
	{
		LPPRECEIVE_NPC_LOCATION pData2 = (LPPRECEIVE_NPC_LOCATION)(ReceiveBuffer+Offset);
		g_pSiegeWarfare->SetGuildMemberLocation( pData2->m_byType+1, pData2->m_byX, pData2->m_byY );
		
		Offset += sizeof(PRECEIVE_NPC_LOCATION);
	}
}


//  공성전(기타)중 남은 시간을 사용자들에게 주기적으로 알림.
void ReceiveMatchTimer ( BYTE* ReceiveBuffer )
{
    LPPRECEIVE_MATCH_TIMER pData = (LPPRECEIVE_MATCH_TIMER)ReceiveBuffer;
	
	g_pSiegeWarfare->SetTime( pData->m_byHour, pData->m_byMinute );
}

//////////////////////////////////////////////////////////////////////////
//	크라이울프 1차 레이드 MVP
//////////////////////////////////////////////////////////////////////////
void ReceiveCrywolfInfo ( BYTE* ReceiveBuffer )
{
	LPPMSG_ANS_CRYWOLF_INFO pData = (LPPMSG_ANS_CRYWOLF_INFO)ReceiveBuffer;
	
	M34CryWolf1st::CheckCryWolf1stMVP(pData->btOccupationState, pData->btCrywolfState);	// 크라이울프 맵상태와 진행 상태 값
	g_Direction.m_CMVP.GetCryWolfState(pData->btCrywolfState);	// 연출부분 체크
	
}

void ReceiveCrywolStateAltarfInfo ( BYTE* ReceiveBuffer )
{
	LPPMSG_ANS_CRYWOLF_STATE_ALTAR_INFO pData = (LPPMSG_ANS_CRYWOLF_STATE_ALTAR_INFO)ReceiveBuffer;
	
	// 크라이울프 석상 HP와 크라이울프 제단 상태
	M34CryWolf1st::CheckCryWolf1stMVPAltarfInfo(pData->iCrywolfStatueHP,
		pData->btAltarState1,
		pData->btAltarState2,
		pData->btAltarState3,
		pData->btAltarState4,
		pData->btAltarState5);
}

//제단들의 계약시도시의 답변
void ReceiveCrywolfAltarContract ( BYTE* ReceiveBuffer )
{
	LPPPMSG_ANS_CRYWOLF_ALTAR_CONTRACT pData = (LPPPMSG_ANS_CRYWOLF_ALTAR_CONTRACT)ReceiveBuffer;
	
	int Key = ((int)(pData->btObjIndexH)<<8) + pData->btObjIndexL; // 제단 인덱스
	
	if(pData->bResult == 0)
	{
		g_CharacterUnRegisterBuff((&Hero->Object), eBuff_CrywolfHeroContracted );
		
        int level = CharacterAttribute->Level;
		if(level < 260)
		{
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCry_Wolf_Dont_Set_Temple1));
			//			M34CryWolf1st::Set_Message_Box(54,0,0);	//"계약 자격이 부족합니다."
			//			M34CryWolf1st::Set_Message_Box(55,1,0);	//"350레벨 이상만 계약할수 있습니다."
		}
		else
		{
			//			M34CryWolf1st::Set_Message_Box(58,0,0);	//"잠시후에 다시시도해주세요"
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCry_Wolf_Wat_Set_Temple1));
		}
	}
	else if(pData->bResult == 1)
	{
		//		M34CryWolf1st::Set_Message_Box(3,0,0);	//늑대의 신상을 보호하기 위한 사제로 등록되었습니다.
		//		M34CryWolf1st::Set_Message_Box(4,1,0);	//사제의 역할을 하는 동안 움직일수 없습니다.
		SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCry_Wolf_Set_Temple));
		
		M34CryWolf1st::Check_AltarState(Key-316,pData->btAltarState);
		
		g_CharacterRegisterBuff((&Hero->Object), eBuff_CrywolfHeroContracted);
		
		SetAction(&Hero->Object,PLAYER_HEALING_FEMALE1);
		
		SendRequestAction(AT_HEALING1,((BYTE)((Hero->Object.Angle[2]+22.5f)/360.f*8.f+1.f)%8));
	}
}

void ReceiveCrywolfLifeTime ( BYTE* ReceiveBuffer )
//크라이울프의 진행시간을 받아옴.
{
    LPPPMSG_ANS_CRYWOLF_LEFTTIME pData = (LPPPMSG_ANS_CRYWOLF_LEFTTIME)ReceiveBuffer;
	
	// 시간, 분 상태 처리
	M34CryWolf1st::SetTime ( pData->btHour, pData->btMinute );
	g_pCryWolfInterface->SetTime( (int)(pData->btHour), (int)(pData->btMinute) );
}

void ReceiveCrywolfTankerHit ( BYTE* ReceiveBuffer )
{
	//작업중 - 이혁재
    LPPMSG_NOTIFY_REGION_MONSTER_ATTACK pData = (LPPMSG_NOTIFY_REGION_MONSTER_ATTACK)ReceiveBuffer;
	
    M34CryWolf1st::DoTankerFireFixStartPosition ( pData->btSourceX, pData->btSourceY, pData->btPointX, pData->btPointY );
}

void ReceiveCrywolfBenefitPlusChaosRate( BYTE* ReceiveBuffer )
{
	LPPMSG_ANS_CRYWOLF_BENEFIT_PLUS_CHAOSRATE pData = (LPPMSG_ANS_CRYWOLF_BENEFIT_PLUS_CHAOSRATE)ReceiveBuffer;
	
	g_MixRecipeMgr.SetPlusChaosRate(pData->btPlusChaosRate);
}

void ReceiveCrywolfBossMonsterInfo( BYTE* ReceiveBuffer)
//크라이울프 보스 몬스터 출현시 세팅
{
	LPPMSG_ANS_CRYWOLF_BOSSMONSTER_INFO pData = (LPPMSG_ANS_CRYWOLF_BOSSMONSTER_INFO)ReceiveBuffer;
	
	M34CryWolf1st::Set_BossMonster(pData->btBossHP,pData->btMonster2);
}

void ReceiveCrywolfPersonalRank( BYTE* ReceiveBuffer)
{
	LPPMSG_ANS_CRYWOLF_PERSONAL_RANK pData = (LPPMSG_ANS_CRYWOLF_PERSONAL_RANK)ReceiveBuffer;
	
	M34CryWolf1st::Set_MyRank(pData->btRank,pData->iGettingExp);
}

void ReceiveCrywolfHeroList( BYTE* ReceiveBuffer)
{
	LPPMSG_ANS_CRYWOLF_HERO_LIST_INFO_COUNT pData = (LPPMSG_ANS_CRYWOLF_HERO_LIST_INFO_COUNT)ReceiveBuffer;
	int Offset = sizeof(PMSG_ANS_CRYWOLF_HERO_LIST_INFO_COUNT);
	for ( int i=0; i<pData->btCount; i++ )
	{
		LPPMSG_ANS_CRYWOLF_HERO_LIST_INFO pData2 = (LPPMSG_ANS_CRYWOLF_HERO_LIST_INFO)(ReceiveBuffer+Offset);
		Offset += sizeof(PMSG_ANS_CRYWOLF_HERO_LIST_INFO);
		M34CryWolf1st::Set_WorldRank(pData2->iRank,pData2->btHeroClass,pData2->iHeroScore,pData2->szHeroName);
	}
}

//////////////////////////////////////////////////////////////////////////
//	칸투르 관련
//////////////////////////////////////////////////////////////////////////
void ReceiveKanturu3rdStateInfo(BYTE* ReceiveBuffer)
{
	LPPMSG_ANS_KANTURU_STATE_INFO pData = (LPPMSG_ANS_KANTURU_STATE_INFO)ReceiveBuffer;
	
	g_pKanturu2ndEnterNpc->ReceiveKanturu3rdInfo(pData->btState, pData->btDetailState, pData->btEnter, pData->btUserCount, pData->iRemainTime);
}

void ReceiveKanturu3rdEnterBossMap(BYTE* ReceiveBuffer)
{
	LPPMSG_ANS_ENTER_KANTURU_BOSS_MAP pData = (LPPMSG_ANS_ENTER_KANTURU_BOSS_MAP)ReceiveBuffer;
	g_pKanturu2ndEnterNpc->ReceiveKanturu3rdEnter(pData->btResult);
}

void ReceiveKanturu3rdCurrentState( BYTE* ReceiveBuffer)
{
	// 칸투르 처음 맵 들어왔을 당시 상태 와 연출 상태
	LPPMSG_ANS_KANTURU_CURRENT_STATE pData = (LPPMSG_ANS_KANTURU_CURRENT_STATE)ReceiveBuffer;
	M39Kanturu3rd::Kanturu3rdState(pData->btCurrentState, pData->btCurrentDetailState);
	M39Kanturu3rd::CheckSuccessBattle(pData->btCurrentState, pData->btCurrentDetailState);
}

void ReceiveKanturu3rdState( BYTE* ReceiveBuffer)
{
	// 칸투르 맵 상태 와 연출 상태 갱신
	LPPMSG_ANS_KANTURU_STATE_CHANGE pData = (LPPMSG_ANS_KANTURU_STATE_CHANGE)ReceiveBuffer;
	
	if(M39Kanturu3rd::IsInKanturu3rd())
	{
		if((pData->btState == KANTURU_STATE_MAYA_BATTLE 
			&& (pData->btDetailState == KANTURU_MAYA_DIRECTION_MONSTER1 
			|| pData->btDetailState == KANTURU_MAYA_DIRECTION_MAYA1
			|| pData->btDetailState == KANTURU_MAYA_DIRECTION_MONSTER2
			|| pData->btDetailState == KANTURU_MAYA_DIRECTION_MAYA2
			|| pData->btDetailState == KANTURU_MAYA_DIRECTION_MONSTER3
			|| pData->btDetailState == KANTURU_MAYA_DIRECTION_MAYA3
			))
			|| (pData->btState == KANTURU_STATE_NIGHTMARE_BATTLE
			&& (pData->btDetailState == KANTURU_NIGHTMARE_DIRECTION_BATTLE
			))
			)
		{
			if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_KANTURU_INFO) == false)
			{
				g_pNewUISystem->Show(SEASON3B::INTERFACE_KANTURU_INFO);
			}
		}
		else
		{
			if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_KANTURU_INFO) == true)
			{
				g_pNewUISystem->Hide(SEASON3B::INTERFACE_KANTURU_INFO);
			}
		}
		M39Kanturu3rd::Kanturu3rdState(pData->btState, pData->btDetailState);
		M39Kanturu3rd::CheckSuccessBattle(pData->btState, pData->btDetailState);
	}
}

void ReceiveKanturu3rdResult( BYTE* ReceiveBuffer)
{
	// 칸투르 성공 실패 결과
	LPPMSG_ANS_KANTURU_BATTLE_RESULT pData = (LPPMSG_ANS_KANTURU_BATTLE_RESULT)ReceiveBuffer;
	M39Kanturu3rd::Kanturu3rdResult(pData->btResult);
}

void ReceiveKanturu3rdTimer ( BYTE* ReceiveBuffer )
{
	//  공성전(기타)중 남은 시간을 사용자들에게 주기적으로 알림.
    LPPMSG_ANS_KANTURU_BATTLE_SCENE_TIMELIMIT pData = (LPPMSG_ANS_KANTURU_BATTLE_SCENE_TIMELIMIT)ReceiveBuffer;
	g_pKanturuInfoWindow->SetTime(pData->btTimeLimit);
}

void RecevieKanturu3rdMayaSKill( BYTE* ReceiveBuffer)
{
	LPPMSG_NOTIFY_KANTURU_WIDE_AREA_ATTACK pData = (LPPMSG_NOTIFY_KANTURU_WIDE_AREA_ATTACK)ReceiveBuffer;
	M39Kanturu3rd::MayaSceneMayaAction(pData->btType);
}

void RecevieKanturu3rdLeftUserandMonsterCount( BYTE* ReceiveBuffer)
{
	LPPMSG_NOTIFY_KANTURU_USER_MONSTER_COUNT pData = (LPPMSG_NOTIFY_KANTURU_USER_MONSTER_COUNT)ReceiveBuffer;
	M39Kanturu3rd::Kanturu3rdUserandMonsterCount(pData->bMonsterCount, pData->btUserCount);
}

#ifdef ADD_PCROOM_POINT_SYSTEM
//----------------------------------------------------------------------------
// PC방 포인트 시스템
//----------------------------------------------------------------------------
// PC방 포인트 정보를  .
void ReceivePCRoomPointInfo(BYTE* ReceiveBuffer)
{
	LPPMSG_ANS_GC_PCROOM_POINT_INFO pData = (LPPMSG_ANS_GC_PCROOM_POINT_INFO)ReceiveBuffer;
	
	CPCRoomPtSys& rPCRoomPtSys = CPCRoomPtSys::Instance();

#ifndef KJH_DEL_PC_ROOM_SYSTEM				// #ifndef
#ifndef CSK_MOD_MOVE_COMMAND_WINDOW		// 정리할 때 지워야 하는 소스
	rPCRoomPtSys.SetPCRoom();
#endif // CSK_MOD_MOVE_COMMAND_WINDOW	// 정리할 때 지워야 하는 소스

	// PC방 포인트 정보 입력
	rPCRoomPtSys.SetPoint(pData->nPoint, pData->nMaxPoint);
#endif // KJH_DEL_PC_ROOM_SYSTEM

#ifdef CSK_MOD_MOVE_COMMAND_WINDOW
	rPCRoomPtSys.SetPCRoom(pData->bPCBang);
#endif // CSK_MOD_MOVE_COMMAND_WINDOW

#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0xD0 0x04 [ReceivePCRoomPointInfo(%d | %d)]", pData->nPoint, pData->nMaxPoint);
#endif // CONSOLE_DEBUG
}

#ifndef KJH_DEL_PC_ROOM_SYSTEM			// #ifndef
// PC방 포인트 상점에서 구입시 응답.
void ReceivePCRoomPointUse(BYTE* ReceiveBuffer)
{
	LPPMSG_ANS_GC_PCROOM_POINT_USE pData = (LPPMSG_ANS_GC_PCROOM_POINT_USE)ReceiveBuffer;
	
	// 결과 0x00:성공, 0x01:안전지대 아님, 0x02:해당위치에 아이템이 없음,
	//0x03:인벤토리 공간 부족, 0x04:기본 조건 포인트부족,
	//0x05:PC방 포인트 시스템 대상자가 아님, 0x06:에러,
	//0x07:상점 사용 가능 시간 초과, 0x08:PC방 포인트 전용 아이템이 아님
	//0x09:구입 가능한 포인트 부족
	switch (pData->byResult)
	{
	case 0x00:	// 성공.
		if (pData->byInvenPos != 0xff)	// 인장 구입이 아님.
		{
			// 아이템 구입.
			g_pMyInventory->InsertItem(pData->byInvenPos-MAX_EQUIPMENT_INDEX, pData->ItemInfo);
			
			PlayBuffer(SOUND_GET_ITEM01);
		}
		break;
		
	case 0x03:
		// 375 "인벤토리가 꽉찼습니다."
		g_pChatListBox->AddText("", GlobalText[375], SEASON3B::TYPE_ERROR_MESSAGE);		// "인벤토리가 꽉 찼습니다."
		break;
					
	case 0x04:
		g_pChatListBox->AddText("", GlobalText[2333], SEASON3B::TYPE_ERROR_MESSAGE);	// "구입 조건을 확인하세요."
		break;
					
	case 0x09:
		g_pChatListBox->AddText("", GlobalText[2322], SEASON3B::TYPE_ERROR_MESSAGE);		// "포인트가 부족합니다."
		break;
	}
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0xD0 0x05 [ReceivePCRoomPointUse(%d)]", pData->byInvenPos);
#endif // CONSOLE_DEBUG	
}

// PC방 포인트 상점 오픈 요구에 대한 응답.
void ReceivePCRoomPointShopOpen(BYTE* ReceiveBuffer)
{
	LPPMSG_REQ_PCROOM_POINT_SHOP_OPEN pData = (LPPMSG_REQ_PCROOM_POINT_SHOP_OPEN)ReceiveBuffer;
	
	// 결과	0:오픈, 1:안전지대 아님, 2:상점에 아이템 없음, 3:다른 창 사용 중, 4:에러
	if (0 == pData->byResult)
	{
		CPCRoomPtSys::Instance().SetPCRoomPointShopMode();	// PC방 포인트 상점 모드임.
		g_pNewUISystem->Show(SEASON3B::INTERFACE_NPCSHOP);
	}
}
#endif // KJH_DEL_PC_ROOM_SYSTEM

#endif	// ADD_PCROOM_POINT_SYSTEM

void ReceiveCursedTempleEnterInfo( BYTE* ReceiveBuffer )
{
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	LPPMSG_CURSED_TEMPLE_USER_COUNT data = (LPPMSG_CURSED_TEMPLE_USER_COUNT)ReceiveBuffer;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	
	g_pCursedTempleEnterWindow->ReceiveCursedTempleEnterInfo( ReceiveBuffer );
}

void ReceiveCursedTempleEnterResult( BYTE* ReceiveBuffer )
{
	LPPMSG_RESULT_ENTER_CURSED_TEMPLE data = (LPPMSG_RESULT_ENTER_CURSED_TEMPLE)ReceiveBuffer;
	//0 : 성공, 1 : 알맞은 피의 두루마리 없음, 2 : 들어갈수 있는 시간이 아님, 3 : 피의 두루마리는 맞으나 자신의 레벨과 맞지 않음
	//4 : 인원제한이 넘었음, 5 : 1일 입장제한 횟수를 넘었음, 6 : 입장료가 모자란다., 7 : 카오유저는 입장할 수 없다., 8 : 변신반지를 착용하고 입장할 수 없다.
	if( data->Result == 0 ) 
	{
		g_pNewUISystem->HideAll();
	}
#ifdef PBG_FIX_CURSEDTEMPLE_SYSTEMMSG
	g_CursedTemple->UpdateTempleSystemMsg(data->Result);
#endif //PBG_FIX_CURSEDTEMPLE_SYSTEMMSG
}

void ReceiveCursedTempleInfo( BYTE* ReceiveBuffer )
{
	g_pCursedTempleWindow->ReceiveCursedTempleInfo( ReceiveBuffer );
	g_CursedTemple->ReceiveCursedTempleInfo( ReceiveBuffer );
}

void ReceiveCursedTempMagicResult( BYTE* ReceiveBuffer )
{
	g_pCursedTempleWindow->ReceiveCursedTempRegisterSkill( ReceiveBuffer );
}

void ReceiveCursedTempSkillEnd( BYTE* ReceiveBuffer )
{
	g_pCursedTempleWindow->ReceiveCursedTempUnRegisterSkill( ReceiveBuffer );
}

void ReceiveCursedTempSkillPoint( BYTE* ReceiveBuffer )
{
	g_pCursedTempleWindow->ReceiveCursedTempSkillPoint( ReceiveBuffer );
}

void ReceiveCursedTempleHolyItemRelics( BYTE* ReceiveBuffer )
{
	g_pCursedTempleWindow->ReceiveCursedTempleHolyItemRelics( ReceiveBuffer );
}

void ReceiveCursedTempleGameResult( BYTE* ReceiveBuffer )
{
	g_pNewUISystem->HideAll();
	
	if( g_pNewUISystem->IsVisible( SEASON3B::INTERFACE_CURSEDTEMPLE_GAMESYSTEM ) )
	{
		g_pCursedTempleResultWindow->ResetGameResultInfo();
		g_pCursedTempleResultWindow->SetMyTeam( g_pCursedTempleWindow->GetMyTeam() );
		
		g_pNewUISystem->Hide( SEASON3B::INTERFACE_CURSEDTEMPLE_GAMESYSTEM );
	}
	
	PlayBuffer( SOUND_CURSEDTEMPLE_GAMESYSTEM5 );
	
	g_pNewUISystem->Show( SEASON3B::INTERFACE_CURSEDTEMPLE_RESULT );
	
	g_pCursedTempleResultWindow->ReceiveCursedTempleGameResult(ReceiveBuffer);
}

void ReceiveCursedTempleState( BYTE* ReceiveBuffer )
{
	LPPMSG_ILLUSION_TEMPLE_STATE data = (LPPMSG_ILLUSION_TEMPLE_STATE)ReceiveBuffer;
	
	// 0: wait, 1: wait->ready, 2: ready->play, 3: play->end
	SEASON3A::eCursedTempleState cursedtemple = static_cast<SEASON3A::eCursedTempleState>(data->btIllusionTempleState+1);
	
	if( cursedtemple == SEASON3A::eCursedTempleState_Ready )
	{
		g_pNewUISystem->HideAll();
		
		g_pCursedTempleWindow->ResetCursedTempleSystemInfo();
		g_pCursedTempleWindow->StartTutorialStep();
		
		PlayBuffer( SOUND_CURSEDTEMPLE_GAMESYSTEM1 );
		
		g_pNewUISystem->Show( SEASON3B::INTERFACE_CURSEDTEMPLE_GAMESYSTEM );		
	}
	
	g_CursedTemple->ReceiveCursedTempleState( cursedtemple );
}

#ifdef CSK_RAKLION_BOSS
void ReceiveRaklionStateInfo(BYTE* ReceiveBuffer)
{
	LPPMSG_ANS_RAKLION_STATE_INFO pData = (LPPMSG_ANS_RAKLION_STATE_INFO)ReceiveBuffer;
	g_Raklion.SetState(pData->btState, pData->btDetailState);
}

void ReceiveRaklionCurrentState(BYTE* ReceiveBuffer)
{
	LPPMSG_ANS_RAKLION_CURRENT_STATE pData = (LPPMSG_ANS_RAKLION_CURRENT_STATE)ReceiveBuffer;
	g_Raklion.SetState(pData->btCurrentState, pData->btCurrentDetailState);
}

void RecevieRaklionStateChange(BYTE* ReceiveBuffer)
{
	LPPMSG_ANS_RAKLION_STATE_CHANGE pData = (LPPMSG_ANS_RAKLION_STATE_CHANGE)ReceiveBuffer;
	g_Raklion.SetState(pData->btState, pData->btDetailState);
}

void RecevieRaklionBattleResult(BYTE* ReceiveBuffer)
{
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	LPPMSG_ANS_RAKLION_BATTLE_RESULT pData = (LPPMSG_ANS_RAKLION_BATTLE_RESULT)ReceiveBuffer;	
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
}

void RecevieRaklionWideAreaAttack(BYTE* ReceiveBuffer)
{
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	LPPMSG_NOTIFY_RAKLION_WIDE_AREA_ATTACK pData = (LPPMSG_NOTIFY_RAKLION_WIDE_AREA_ATTACK)ReceiveBuffer;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
}

void RecevieRaklionUserMonsterCount(BYTE* ReceiveBuffer)
{
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	LPPMSG_NOTIFY_RAKLION_USER_MONSTER_COUNT pData = (LPPMSG_NOTIFY_RAKLION_USER_MONSTER_COUNT)ReceiveBuffer;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
}
#endif // CSK_RAKLION_BOSS

///////////////////////////////////////////////////////////////////////////////
// 해킹 관련
///////////////////////////////////////////////////////////////////////////////

void ReceiveCheckSumRequest( BYTE *ReceiveBuffer)
{
	LPPHEADER_DEFAULT_WORD Data = (LPPHEADER_DEFAULT_WORD)ReceiveBuffer;
	DWORD dwCheckSum = GetCheckSum( Data->Value);
	SendCheckSum( dwCheckSum );	
	
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_RECEIVE, "0x03 [ReceiveCheckSumRequest]");
#endif // CONSOLE_DEBUG
}


#ifdef NEW_USER_INTERFACE_SERVERMESSAGE

//////////////////////////////////////////////////////////////////////////
// [캐쉬샵 오픈 요청 결과] 0xF5, 0x02
//////////////////////////////////////////////////////////////////////////
void RecevieCashShopInOut( BYTE* ReceiveBuffer )
{
	TheShopServerProxy().RecevieShopInOut( ReceiveBuffer );
}
//////////////////////////////////////////////////////////////////////////
// [캐쉬 포인트 잔량 요청 결과] 0xF5, 0x04
//////////////////////////////////////////////////////////////////////////
void RecevieCashShopCashPoint( BYTE* ReceiveBuffer )
{
	TheShopServerProxy().RecevieShopCashPoint( ReceiveBuffer );
}
//////////////////////////////////////////////////////////////////////////
// [캐쉬 아이템 리스트 요청 결과] 0xF5, 0x06
//////////////////////////////////////////////////////////////////////////
void RecevieCashShopItemlist( BYTE* ReceiveBuffer )
{
	TheShopServerProxy().RecevieShopItemlist( ReceiveBuffer );
}
//////////////////////////////////////////////////////////////////////////
// [캐쉬샵 구매 요청 결과] 0xF5, 0x08
//////////////////////////////////////////////////////////////////////////
void RecevieCashShopItemPurchase( BYTE* ReceiveBuffer )
{
	TheShopServerProxy().RecevieShopItemPurchase( ReceiveBuffer );
}

#ifdef PSW_CHARACTER_CARD
void ReceiveBuyCharacterCard( BYTE* ReceiveBuffer )
{
	TheGameServerProxy().ReceiveBuyCharacterCard( ReceiveBuffer );
}

void ReceiveCharacterCard( BYTE* ReceiveBuffer )
{
	TheGameServerProxy().ReceiveCharacterCard( ReceiveBuffer );
}
#endif //PSW_CHARACTER_CARD

#endif //NEW_USER_INTERFACE_SERVERMESSAGE

void Action(CHARACTER *c,OBJECT *o,bool Now);


extern int TimeRemain;

///////////////////////////////////////////////////////////////////////////////

// 프로토콜  
///////////////////////////////////////////////////////////////////////////////

BOOL TranslateProtocol( int HeadCode, BYTE *ReceiveBuffer, int Size, BOOL bEcrypted);
void TranslateChattingProtocol(DWORD dwWindowUIID, int HeadCode, BYTE *ReceiveBuffer, int Size, BOOL bEcrypted);

void ProtocolCompiler( CWsctlc *pSocketClient, int iTranslation, int iParam)
{
	//if(CurrentProtocolState >= RECEIVE_JOIN_MAP_SERVER)
	//	return;
	int HeadCode;
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	int Size = 0;
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	int Size;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	
	while(1)
	{
		BYTE *ReceiveBuffer = pSocketClient->GetReadMsg();
		if( ReceiveBuffer == NULL )
		{
			break;
		}
		else 
		{
#ifdef USE_SELFCHECKCODE
			SendCrcOfFunction( 13, 2, CheckClientArrow, 0xF2A8);
#endif
			BOOL bEncrypted = FALSE;
			BYTE byDec[MAX_SPE_BUFFERSIZE_];
			if( ReceiveBuffer[0] == 0xC1 )
			{
				LPPBMSG_HEADER lpHeader = (LPPBMSG_HEADER)ReceiveBuffer;
				HeadCode = lpHeader->HeadCode;
				Size     = lpHeader->Size;
			}
			else if( ReceiveBuffer[0] == 0xC2 )
			{
				LPPWMSG_HEADER lpHeader = (LPPWMSG_HEADER)ReceiveBuffer;
				HeadCode = lpHeader->HeadCode;
				Size     = ((int)(lpHeader->SizeH)<<8) + lpHeader->SizeL;
			}
			else if( ReceiveBuffer[0] == 0xC3 || ReceiveBuffer[0] == 0xC4)
			{
				int iSize;
				if ( ReceiveBuffer[0] == 0xC3)
				{
					LPPBMSG_ENCRYPTED lpHeader = (LPPBMSG_ENCRYPTED)ReceiveBuffer;
					Size     = lpHeader->Size;
					// SC 도 시리얼

#ifdef PKD_ADD_ENHANCED_ENCRYPTION
					iSize = g_SessionCryptorSC.Decrypt( (int)pSocketClient->GetSocket(), byDec + 2, ReceiveBuffer + 2, Size - 2);
#else // PKD_ADD_ENHANCED_ENCRYPTION
					iSize = g_SimpleModulusSC.Decrypt( byDec + 2, ReceiveBuffer + 2, Size - 2);
#endif // PKD_ADD_ENHANCED_ENCRYPTION				
				}
				else
				{
					LPWBMSG_ENCRYPTED lpHeader = (LPWBMSG_ENCRYPTED)ReceiveBuffer;
					Size     = ((int)(lpHeader->SizeH)<<8) + lpHeader->SizeL;
					// SC 도 시리얼
#ifdef PKD_ADD_ENHANCED_ENCRYPTION
					iSize = g_SessionCryptorSC.Decrypt( (int)pSocketClient->GetSocket(), byDec + 2, ReceiveBuffer + 3, Size - 3);
#else // PKD_ADD_ENHANCED_ENCRYPTION
					iSize = g_SimpleModulusSC.Decrypt( byDec + 2, ReceiveBuffer + 3, Size - 3);
#endif // PKD_ADD_ENHANCED_ENCRYPTION
				}
				bEncrypted = TRUE;
				
				if ( iSize < 0)
				{
					SendHackingChecked( 0x06, 0);
					g_byPacketSerialRecv++;
					continue;
				}
				
				if ( ( g_byPacketSerialRecv) != byDec[2])
				{
					bEncrypted = FALSE;
					g_byPacketSerialRecv = byDec[2];		//. * 문제 발생 의심코드
					
					g_ErrorReport.Write("Decrypt error : g_byPacketSerialRecv(0x%02X), byDec(0x%02X)\r\n", g_byPacketSerialRecv, byDec[2]);
					g_ErrorReport.Write("Dump : \r\n");
					g_ErrorReport.HexWrite(ReceiveBuffer, Size);
					g_ErrorReport.Write("\r\n");
					g_ErrorReport.HexWrite(byDec+2, iSize);
				}
				else
				{
					g_byPacketSerialRecv++;
				}

				if ( ReceiveBuffer[0] == 0xC3)
				{
					LPPBMSG_HEADER pHeader = (LPPBMSG_HEADER)&( byDec[1]);
					pHeader->Code = 0xC1;
					pHeader->Size = ( BYTE)iSize;
					HeadCode = pHeader->HeadCode;
					ReceiveBuffer = (BYTE*)pHeader;
				}
				else
				{
					LPPWMSG_HEADER pHeader = (LPPWMSG_HEADER)byDec;
					pHeader->Code = 0xC2;
					pHeader->SizeH = ( BYTE)( iSize / 256);
					pHeader->SizeL = ( BYTE)( iSize % 256);
					HeadCode = pHeader->HeadCode;
					ReceiveBuffer = (BYTE*)pHeader;
				}
				Size = iSize;
			}
			TotalPacketSize += Size;
#ifdef USE_SELFCHECKCODE
			if ( TimeRemain >= 40)
			{
				SendCrcOfFunction( 7, 17, Action, 0x823F);
			}
#endif
			
#ifdef ACC_PACKETSIZE
			g_iTotalPacketRecv += Size;
#endif
            
#ifdef SAVE_PACKET
			SOCKET socket = pSocketClient->GetSocket();
			
			SOCKADDR_IN sockaddr;
			int isockaddrlen	= sizeof(sockaddr);
			getpeername(socket, (SOCKADDR*)&sockaddr, &isockaddrlen);
			
			std::string timeString;
			leaf::GetTimeString(timeString);
			DebugAngel_Write( PACKET_SAVE_FILE, "%s [%d.%d.%d.%d] Receive ", timeString.c_str(),
				(UCHAR)sockaddr.sin_addr.S_un.S_un_b.s_b1, (UCHAR)sockaddr.sin_addr.S_un.S_un_b.s_b2, 
				(UCHAR)sockaddr.sin_addr.S_un.S_un_b.s_b3, (UCHAR)sockaddr.sin_addr.S_un.S_un_b.s_b4);
			for(int i=0; i<6; i++) {
				DebugAngel_Write( PACKET_SAVE_FILE, "%02X", ReceiveBuffer[i]);
			}
			DebugAngel_Write( PACKET_SAVE_FILE, "\r\n");
#endif
			
#ifdef CONSOLE_DEBUG
			//g_ConsoleDebug->Write(MCD_RECEIVE, "[Receive]HeadCode : %02X, Size : %d", HeadCode, Size);
#endif // CONSOLE_DEBUG
			
			switch ( iTranslation)
			{
			case 1:	// 채팅용
				TranslateChattingProtocol(iParam, HeadCode, ReceiveBuffer, Size, bEncrypted);
				break;
			default:
				if ( !TranslateProtocol( HeadCode, ReceiveBuffer, Size, bEncrypted))
				{	// 암호화 되어야 하는데 안된 것들
					g_ErrorReport.Write( "Strange packet\r\n");
					g_ErrorReport.HexWrite( ReceiveBuffer, Size);
				}
				break;
			}
		}
	}
#ifdef USE_SELFCHECKCODE
	END_OF_FUNCTION( Pos_SelfCheck01);
Pos_SelfCheck01:
	;
#endif
}

#ifdef PJH_CHARACTER_RENAME
void Receive_Change_Name_Result(BYTE *ReceiveBuffer)
{
	LPPMSG_CHANGE_NAME_RESULT Data = (LPPMSG_CHANGE_NAME_RESULT)ReceiveBuffer;
	
	if(Data->btResult == 0) //성공
	{
		::strcpy(CharactersClient[SelectedHero].ID, Data->NewName);
		::StartGame(true);
	}
	else
	{
		CUIMng::Instance().PopUpMsgWin(RECEIVE_CREATE_CHARACTER_FAIL);
	}
}

void Receive_Check_Change_Name(BYTE *ReceiveBuffer)
{
	LPPMSG_ANS_CHECK_CHANGE_NAME Data = (LPPMSG_ANS_CHECK_CHANGE_NAME)ReceiveBuffer;
	
	if(Data->btResult == 0)	//캐릭터 이름을 바꿀필요가 없다.
	{
		::StartGame(true);
	}
	else
		if(Data->btResult == 1)	//캐릭터 이름을 바꿔야 한다.
		{
			//캐릭터 이름바꾸는 ui를 띄워준다.
			CUIMng& rUIMng = CUIMng::Instance();
			rUIMng.ShowWin(&rUIMng.m_CharMakeWin);
			rUIMng.m_CharMakeWin.Set_State(true);
		}
}
#endif //PJH_CHARACTER_RENAME

#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
//////////////////////////////////////////////////////////////////////////
// [행운의 동전 관련] 0xBF
//////////////////////////////////////////////////////////////////////////

//등록된 동전 갯수 응답
bool ReceiveRegistedLuckyCoin(BYTE* ReceiveBuffer)
{
	LPPMSG_ANS_GET_COIN_COUNT _pData = (LPPMSG_ANS_GET_COIN_COUNT)ReceiveBuffer;
	
	if(_pData->nCoinCnt >= 0)
	{
		//0이상으로 응답이 왔다면, 등록된 동전 갯수
		g_pLuckyCoinRegistration->SetRegistCount(_pData->nCoinCnt);
		return true;
	}
	
	//이상한 값이 들어왔다면
	return false;
}

//동전 등록 응답
bool ReceiveRegistLuckyCoin(BYTE* ReceiveBuffer)
{
	LPPMSG_ANS_REGEIST_COIN _pData = (LPPMSG_ANS_REGEIST_COIN)ReceiveBuffer;

	g_pLuckyCoinRegistration->UnLockLuckyCoinRegBtn();
	
	switch(_pData->btResult)
	{
	case 0:		// 실패 (행운의 동전부족)
		{
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CUseRegistLuckyCoinMsgBoxLayout));
		}break;
	case 1:		// 성공
		{
			//현재 등록된 동전 갯수
			g_pLuckyCoinRegistration->SetRegistCount(_pData->nCurCoinCnt);
		}break;
#ifdef PBG_MOD_LUCKYCOINEVENT
	case 100:
		{
			//등록된 갯수가 255개가 초과할시
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CRegistOverLuckyCoinMsgBoxLayout));
		}break;
#endif //PBG_MOD_LUCKYCOINEVENT
	default:	// 잘못된 Result값
		return false;
	}

	return true;
}

//동전 교환 응답
bool ReceiveRequestExChangeLuckyCoin(BYTE* ReceiveBuffer)
{
	LPPMSG_ANS_TREADE_COIN _pData = (LPPMSG_ANS_TREADE_COIN)ReceiveBuffer;

	g_pExchangeLuckyCoinWindow->UnLockExchangeBtn();
	
	switch(_pData->btResult)
	{
	case 0:	// 실패(행운의동전교환 갯수부족)
		{
			// 실패 메세지 처리
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CExchangeLuckyCoinMsgBoxLayout));
		}
		break;
	case 1:	//성공
		{
			// 성공 후 인벤 닫을시
			//g_pNewUISystem->Hide(SEASON3B::INTERFACE_EXCHANGE_LUCKYCOIN);
			g_pChatListBox->AddText("", GlobalText[1888], SEASON3B::TYPE_SYSTEM_MESSAGE);
		}break;
	case 2: // 공간없음
		{
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CExchangeLuckyCoinInvenErrMsgBoxLayout));
		}break;
	default:
		return false;
	}

	return true;
}

#endif //KJH_PBG_ADD_SEVEN_EVENT_2008

#ifdef YDG_ADD_DOPPELGANGER_PROTOCOLS
bool ReceiveEnterDoppelGangerEvent(BYTE* ReceiveBuffer)
{
	LPPMSG_RESULT_ENTER_DOPPELGANGER Data = (LPPMSG_RESULT_ENTER_DOPPELGANGER)ReceiveBuffer;

	unicode::t_char szText[256] = { 0, };

	switch(Data->btResult)
	{
	case 0:		// 성공
		break;
	case 1:		// 차원의마경 없음
		g_pDoppelGangerWindow->LockEnterButton(TRUE);
		break;
	case 2:		// 이미 다른 파티가 사용중
		unicode::_sprintf(szText, GlobalText[2864]);
		g_pChatListBox->AddText("", szText, SEASON3B::TYPE_ERROR_MESSAGE);
		g_pDoppelGangerWindow->LockEnterButton(TRUE);
		break;
	case 3:		// 카오유저는 입장할 수 없다.
		unicode::_sprintf(szText, GlobalText[2865]);
		g_pChatListBox->AddText("", szText, SEASON3B::TYPE_ERROR_MESSAGE);
		g_pDoppelGangerWindow->LockEnterButton(TRUE);
		break;
	case 4:		// 변신반지를 착용하고 입장할 수 없다.
		g_pDoppelGangerWindow->LockEnterButton(FALSE);
		break;
#ifdef KJH_MOD_ENTER_EVENTMAP_ERROR_MSG
	case 5:		// 150레벨 이하
		{
			CMsgBoxIGSCommon* pMsgBox = NULL;
			char szText[256];
			sprintf(szText, GlobalText[1123], 150);			// "레벨 150이상부터 이용할 수 있습니다."
			CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
			pMsgBox->Initialize(GlobalText[3028], szText);
		}break;
#endif // KJH_MOD_ENTER_EVENTMAP_ERROR_MSG
	default:
		return false;
	}

	return true;
}

bool ReceiveDoppelGangerMonsterPosition(BYTE* ReceiveBuffer)
{
	LPPMSG_DOPPELGANGER_MONSTER_POSITION Data = (LPPMSG_DOPPELGANGER_MONSTER_POSITION)ReceiveBuffer;

	// 위치번호(0~22) 몬스터 리젠지점이 0
	g_pDoppelGangerFrame->SetMonsterGauge((float)Data->btPosIndex / 22.0f);

	return true;
}

bool ReceiveDoppelGangerState(BYTE* ReceiveBuffer)
{
	LPPMSG_DOPPELGANGER_STATE Data = (LPPMSG_DOPPELGANGER_STATE)ReceiveBuffer;

	switch(Data->btDoppelgangerState)
	{
	case 0:		// wait
		// 사용안함
		break;
	case 1:		// wait->ready
		// 사용안함
		break;
	case 2:		// ready->play
		{
			// UI 출력
			g_pNewUISystem->Show(SEASON3B::INTERFACE_DOPPELGANGER_FRAME);

			// 안내 메시지
			SEASON3B::CNewUICommonMessageBox* pMsgBox;
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CDoppelGangerMsgBoxLayout), &pMsgBox);
			pMsgBox->AddMsg(GlobalText[2763], RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
			pMsgBox->AddMsg(" ");
			pMsgBox->AddMsg(GlobalText[2764], RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
			pMsgBox->AddMsg(" ");
			pMsgBox->AddMsg(GlobalText[2765], RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
		}
		break;
	case 3:		// play->end
		// 사용안함
		break;
	}

	return true;
}

bool ReceiveDoppelGangerIcewalkerState(BYTE* ReceiveBuffer)
{
	LPPMSG_DOPPELGANGER_ICEWORKER_STATE Data = (LPPMSG_DOPPELGANGER_ICEWORKER_STATE)ReceiveBuffer;

	switch(Data->btIceworkerState)
	{
	case 0:		// 출현
		g_pDoppelGangerFrame->SetIceWalkerMap(TRUE, (float)(22 - Data->btPosIndex) / 22.0f);
		break;
	case 1:		// 사라짐
		g_pDoppelGangerFrame->SetIceWalkerMap(FALSE, 0);
		break;
	}

	return true;
}

bool ReceiveDoppelGangerTimePartyState(BYTE* ReceiveBuffer)
{
	LPPMSG_DOPPELGANGER_PLAY_INFO Data = (LPPMSG_DOPPELGANGER_PLAY_INFO)ReceiveBuffer;

	// 시간 설정
	g_pDoppelGangerFrame->SetRemainTime(Data->wRemainSec);
	g_pDoppelGangerFrame->SetPartyMemberRcvd();
	LPPMSG_DOPPELGANGER_USER_POS pUserPos = (LPPMSG_DOPPELGANGER_USER_POS)&Data->UserPosData;
	for (int i = 0; i < Data->btUserCount; ++i)
	{
		g_pDoppelGangerFrame->SetPartyMemberInfo(pUserPos[i].wUserIndex, (float)(22 - pUserPos[i].btPosIndex) / 22.0f);
	}

	return true;
}

bool ReceiveDoppelGangerResult(BYTE* ReceiveBuffer)
{
	LPPMSG_DOPPELGANGER_RESULT Data = (LPPMSG_DOPPELGANGER_RESULT)ReceiveBuffer;

#ifdef YDG_MOD_DOPPELGANGER_END_SOUND
	PlayBuffer(SOUND_CHAOS_END);
#endif	// YDG_MOD_DOPPELGANGER_END_SOUND

	g_pDoppelGangerFrame->StopTimer(TRUE);
	g_pDoppelGangerFrame->EnabledDoppelGangerEvent(FALSE);

	switch(Data->btResult)
	{
	case 0:		// 성공
		{
			// 남은 시간을 0으로 설정한다
			g_pDoppelGangerFrame->SetRemainTime(0);

			// 성공 메시지
			SEASON3B::CNewUICommonMessageBox* pMsgBox;
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CDoppelGangerMsgBoxLayout), &pMsgBox);
			pMsgBox->AddMsg(GlobalText[2769], RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
			pMsgBox->AddMsg(" ");
			pMsgBox->AddMsg(GlobalText[2770], RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
// 			pMsgBox->AddMsg(" ");
// 			pMsgBox->AddMsg(" ");
// 			char szText[256] = { 0, };
// 			sprintf(szText, GlobalText[2771], Data->dwRewardExp);
// 			pMsgBox->AddMsg(szText, RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_BOLD);
		}
		break;
	case 1:		// 실패(죽어서실패_일반)
		{
			// 실패 메시지1
			SEASON3B::CNewUICommonMessageBox* pMsgBox;
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CDoppelGangerMsgBoxLayout), &pMsgBox);
			pMsgBox->AddMsg(GlobalText[2766], RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
		}
		break;
	case 2:		// 실패(방어실패)
		{
			// 실패 메시지2
			SEASON3B::CNewUICommonMessageBox* pMsgBox;
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CDoppelGangerMsgBoxLayout), &pMsgBox);
			pMsgBox->AddMsg(GlobalText[2767], RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
			pMsgBox->AddMsg(" ");
			pMsgBox->AddMsg(GlobalText[2768], RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
		}
		break;
	}

	return true;
}

bool ReceiveDoppelGangerMonsterGoal(BYTE* ReceiveBuffer)
{
	LPPMSG_DOPPELGANGER_MONSTER_GOAL Data = (LPPMSG_DOPPELGANGER_MONSTER_GOAL)ReceiveBuffer;

	// 도플갱어 들어온 몬스터 마리수 설정
	g_pDoppelGangerFrame->SetMaxMonsters(Data->btMaxGoalCnt);
	g_pDoppelGangerFrame->SetEnteredMonsters(Data->btGoalCnt);

	return true;
}
#endif	// YDG_ADD_DOPPELGANGER_PROTOCOLS

#ifdef YDG_MOD_PROTECT_AUTO_V5
// 오토 툴 사용 여부 응답
bool ReceiveRequestCheckAutoTool(BYTE* ReceiveBuffer)
{
	LPPMSG_REQ_HACKTOOL_STATISTICS Data = (LPPMSG_REQ_HACKTOOL_STATISTICS)ReceiveBuffer;

	Data->Filename[MAX_HACKTOOL_FILENAME_LENGTH - 1] = '\0';
	BOOL bFindAutoTool = g_pProtectAuto->FindAutoTool(Data->Filename);
	memset(Data->Filename, 0, MAX_HACKTOOL_FILENAME_LENGTH);	// 메모리에 남는것을 막기위해 지워버린다

	SendRequestCheckAutoToolResult(Data->dwKey, (BYTE)bFindAutoTool);

	return true;
}
#endif	// YDG_MOD_PROTECT_AUTO_V5

#ifdef YDG_ADD_MOVE_COMMAND_PROTOCOL
// 맵 이동 체크섬 값
bool ReceiveMoveMapChecksum(BYTE* ReceiveBuffer)
{
	LPPMSG_MAPMOVE_CHECKSUM Data = (LPPMSG_MAPMOVE_CHECKSUM)ReceiveBuffer;

	g_pMoveCommandWindow->SetMoveCommandKey(Data->dwKeyValue);
	
	return true;
}

// 맵 이동 결과
bool ReceiveRequestMoveMap(BYTE* ReceiveBuffer)
{
	LPPMSG_ANS_MAPMOVE Data = (LPPMSG_ANS_MAPMOVE)ReceiveBuffer;

	if (Data->btResult == 0)
	{
		// 이동 성공
	}
	else
	{
		// 이동 실패
	}

//	에러번호
// 	MAPMOVE_SUCCESS				= 0,			// 이동 성공
// 	MAPMOVE_FAILED,								// 이동 실패 (이동 불가능한 다른 경우)
// 	MAPMOVE_FAILED_TELEPORT,					// 이동 실패 (텔레포트 중)
// 	MAPMOVE_FAILED_PSHOP_OPEN,					// 이동 실패 (개인상점이 열려있는 경우)
// 	MAPMOVE_FAILED_RECALLED,					// 이동 실패 (소환되는 도중)
// 	MAPMOVE_FAILED_NOT_ENOUGH_EQUIP,			// 이동 실패 (해당 맵으로 이동하기 위한 장비 부족)
// 	MAPMOVE_FAILED_WEARING_EQUIP,				// 이동 실패 (해당 맵에 입장시 장비하지 못하는 장비 착용)
// 	MAPMOVE_FAILED_MURDERER,					// 이동 실패 (살인마 일 경우 맵 이동 불가)
// 	MAPMOVE_FAILED_NOT_ENOUGH_LEVEL,			// 이동 실패 (레벨 부족)
// 	MAPMOVE_FAILED_NOT_ENOUGH_ZEN,				// 이동 실패 (소유 젠 부족)
// 	MAPMOVE_FORCED_EVENT_END		= 20,		// 강제 이동 (이벤트 종료에 의한 강제 이동)
// 	MAPMOVE_FORCED_GM							// 강제 이동 (GM에 의한 강제 이동)

	return true;
}
#endif	// YDG_ADD_MOVE_COMMAND_PROTOCOL

#ifdef LDK_ADD_EMPIREGUARDIAN_PROTOCOLS
bool ReceiveEnterEmpireGuardianEvent(BYTE* ReceiveBuffer)
{
	LPPMSG_RESULT_ENTER_EMPIREGUARDIAN Data = (LPPMSG_RESULT_ENTER_EMPIREGUARDIAN)ReceiveBuffer;
	
	switch(Data->Result)
	{
	case 0:		// 0 : 성공
		{
			g_pEmpireGuardianTimer->SetDay((int)Data->Day);
			g_pEmpireGuardianTimer->SetZone((int)Data->Zone);
			g_pEmpireGuardianTimer->SetRemainTime(Data->RemainTick);
			
			// 모든 날씨 생성은 g_EmpireGuardian1에서 처리함.
			g_EmpireGuardian1.SetWeather((int)Data->Wheather);
		}break;
	case 1:		// 1 : 이미 게임 진행중
		{
			SEASON3B::CNewUICommonMessageBox* pMsgBox;
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CEmpireGuardianMsgBoxLayout), &pMsgBox);
			pMsgBox->AddMsg(GlobalText[2798], RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
			pMsgBox->AddMsg(" ");
			unicode::t_char szText[256] = {NULL, };
			sprintf(szText,GlobalText[2799], (Data->RemainTick / 60000) );
			pMsgBox->AddMsg(szText, RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
		}break;
	case 2:		// 2 : 퀘스트 아이템이 부족
		{
			SEASON3B::CNewUICommonMessageBox* pMsgBox;
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CEmpireGuardianMsgBoxLayout), &pMsgBox);
			pMsgBox->AddMsg(GlobalText[2839], RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
		}break;
	case 3:		// 3 : 인원수 초과
		{
			SEASON3B::CNewUICommonMessageBox* pMsgBox;
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CEmpireGuardianMsgBoxLayout), &pMsgBox);
			pMsgBox->AddMsg(GlobalText[2841], RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
		}break;
	case 4:		// 4 : 시간이 남아있음
		{
			SEASON3B::CNewUICommonMessageBox* pMsgBox;
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CEmpireGuardianMsgBoxLayout), &pMsgBox);
			pMsgBox->AddMsg(GlobalText[2842], RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
		}break;
	case 5:		// 5 : 파티 상태일경우만 입장가능합니다.
		{
			SEASON3B::CNewUICommonMessageBox* pMsgBox;
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CEmpireGuardianMsgBoxLayout), &pMsgBox);
			pMsgBox->AddMsg(GlobalText[2843], RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
		}break;
#ifdef KJH_MOD_ENTER_EVENTMAP_ERROR_MSG
	case 6:		// 6: 150레벨 이상 이용가능
		{
			CMsgBoxIGSCommon* pMsgBox = NULL;
			char szText[256];
			sprintf(szText, GlobalText[1123], 150);			// "레벨 150이상부터 이용할 수 있습니다."
			CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
			pMsgBox->Initialize(GlobalText[3028], szText);
		}break;
#endif // KJH_MOD_ENTER_EVENTMAP_ERROR_MSG

	default:
		return false;
	}
	
	return true;
}

bool ReceiveRemainTickEmpireGuardian(BYTE* ReceiveBuffer)
{
	LPPMSG_REMAINTICK_EMPIREGUARDIAN Data =  (LPPMSG_REMAINTICK_EMPIREGUARDIAN)ReceiveBuffer;

	if( g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_EMPIREGUARDIAN_TIMER) == false )
	{
		g_pNewUISystem->Show(SEASON3B::INTERFACE_EMPIREGUARDIAN_TIMER);
	}

	g_pEmpireGuardianTimer->SetType((int)Data->Type);
	g_pEmpireGuardianTimer->SetRemainTime((int)Data->RemainTick);
	g_pEmpireGuardianTimer->SetMonsterCount((int)Data->MonsterCount);

	return true;
}

bool ReceiveResultEmpireGuardian(BYTE* ReceiveBuffer)
{
	LPPMSG_CLEAR_RESULT_EMPIREGUARDIAN Data =  (LPPMSG_CLEAR_RESULT_EMPIREGUARDIAN)ReceiveBuffer;
	
	switch(Data->Result)
	{
	case 0: //실패
		{
			//제국 수호군 요세 공략에
			//실패하였습니다.
			SEASON3B::CNewUICommonMessageBox* pMsgBox;
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CEmpireGuardianMsgBoxLayout), &pMsgBox);
			pMsgBox->AddMsg(GlobalText[2803], RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
			pMsgBox->AddMsg(GlobalText[2804], RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
		}break;
	case 1: //존 성공
		{
			//제국 수호군 요새 %d차
			//%d존 성공
			int day = g_pEmpireGuardianTimer->GetDay();
			int zone = g_pEmpireGuardianTimer->GetZone();
			SEASON3B::CNewUICommonMessageBox* pMsgBox;
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CEmpireGuardianMsgBoxLayout), &pMsgBox);
			unicode::t_char szText[256] = {NULL, };
			sprintf(szText, GlobalText[2801], day);
			pMsgBox->AddMsg(szText, RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
			sprintf(szText,"%d%s",zone, GlobalText[2840]);
			pMsgBox->AddMsg(szText, RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
		}break;
	case 2: //맵 성공
		{
			//제국 수호군 요새 %d차
			//완료 하였습니다.
			//보상 경험치 : %d
			int day = g_pEmpireGuardianTimer->GetDay();
			SEASON3B::CNewUICommonMessageBox* pMsgBox;
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CEmpireGuardianMsgBoxLayout), &pMsgBox);
			unicode::t_char szText[256] = {NULL, };
			sprintf(szText, GlobalText[2801], day);
			pMsgBox->AddMsg(szText, RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
			pMsgBox->AddMsg(GlobalText[2802], RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
 			sprintf(szText, GlobalText[861], Data->Exp);
 			pMsgBox->AddMsg(szText, RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
		}break;
	}

	if( g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_EMPIREGUARDIAN_TIMER) == true )
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_EMPIREGUARDIAN_TIMER);
	}
	
	return true;
}
#endif //LDK_ADD_EMPIREGUARDIAN_PROTOCOLS

//////////////////////////////////////////////////////////////////////////
// 클라이언트 - 서버간의 캐쉬샵 연동 프로토콜
//////////////////////////////////////////////////////////////////////////
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM

//----------------------------------------------------------------------------
// 사용자의 캐쉬 포인트 정보 전달 (0xD2)(0x01)
bool ReceiveIGS_CashPoint(BYTE* pReceiveBuffer)
{
	return true;
}

//----------------------------------------------------------------------------
// 캐쉬샵 Open 결과 (0xD2)(0x02)
bool ReceiveIGS_ShopOpenResult(BYTE* pReceiveBuffer)
{
	return true;
}

//----------------------------------------------------------------------------
// 아이템 구매 결과 (0xD2)(0x03)
bool ReceiveIGS_BuyItem(BYTE* pReceiveBuffer)
{
	return true;
}

//----------------------------------------------------------------------------
// 아이템 선물 결과 (0xD2)(0x04)
bool ReceiveIGS_SendItemGift(BYTE* pReceiveBuffer)
{
	
	return true;
}

//----------------------------------------------------------------------------
// 보관함 리스트 카운트 전달 (0xD2)(0x06)
bool ReceiveIGS_StorageItemListCount(BYTE* pReceiveBuffer)
{
	return true;
}

//----------------------------------------------------------------------------
// 보관함 리스트 전달 (0xD2)(0x0D)
bool ReceiveIGS_StorageItemList(BYTE* pReceiveBuffer)
{
	
	return true;
}

//----------------------------------------------------------------------------
// 선물함 리스트 전달 (0xD2)(0x0E)
bool ReceiveIGS_StorageGiftItemList(BYTE* pReceiveBuffer)
{
	
	return true;
}

//----------------------------------------------------------------------------
// 캐쉬선물 결과 (0xD2)(0x07)
bool ReceiveIGS_SendCashGift(BYTE* pReceiveBuffer)
{

	return true;
}

//----------------------------------------------------------------------------
// 해당 사용자가 상품의 구매/선물이 가능한지 확인 결과 (0xD2)(0x08)
bool ReceiveIGS_PossibleBuy(BYTE* pReceiveBuffer)
{
	return true;
}

//----------------------------------------------------------------------------
// 상품 잔여 개수 조회 결과 (0xD2)(0x09)
bool ReceiveIGS_LeftCountItem(BYTE* pReceiveBuffer)
{
	return true;
}

#ifndef KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT				// #ifndef
//----------------------------------------------------------------------------
// 보관함 아이템 버리기 결과 (0xD2)(0x0A)
bool ReceiveIGS_DeleteStorageItem(BYTE* pReceiveBuffer)
{
	LPPMSG_CASHSHOP_STORAGE_ITEM_THROW_ANS Data = (LPPMSG_CASHSHOP_STORAGE_ITEM_THROW_ANS)pReceiveBuffer;

	switch( (BYTE)Data->byResult)
	{
	case -2: // DB오류
		{
			CMsgBoxIGSCommon* pMsgBox = NULL;
			CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
			pMsgBox->Initialize(GlobalText[2935], GlobalText[2967]);
		}
		break;
	case -1: // 오류
		{
			CMsgBoxIGSCommon* pMsgBox = NULL;
			CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
			pMsgBox->Initialize(GlobalText[2935], GlobalText[2966]);
		}
		break;
	case 0: // 성공
		{
			CMsgBoxIGSCommon* pMsgBox = NULL;
			CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
			pMsgBox->Initialize(GlobalText[2933], GlobalText[2934]);

			g_pInGameShop->DelStorageSelectedItem();		// 현재 선택된 보관함의 Item을 지운다.
		}
		break;
	case 1: // 보관함 버리기 항목이 없음
		{
			CMsgBoxIGSCommon* pMsgBox = NULL;
			CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
			pMsgBox->Initialize(GlobalText[2935], GlobalText[2936]);
		}
		break;
	default:	// 알 수 없는 에러
		{
			CMsgBoxIGSCommon* pMsgBox = NULL;
			CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
			pMsgBox->Initialize(GlobalText[2945], GlobalText[890]);		
		}
		break;
	}
	
	return true;
}
#endif // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT

//----------------------------------------------------------------------------
// 보관함 아이템 사용 결과 (0xD2)(0x0B)
bool ReceiveIGS_UseStorageItem(BYTE* pReceiveBuffer)
{
	return true;
}

//----------------------------------------------------------------------------
// 스크립트 업데이트 (0xD2)(0x0C)
bool ReceiveIGS_UpdateScript(BYTE* pReceiveBuffer)
{

	return true;
}

//----------------------------------------------------------------------------
// 이벤트 아이템 리스트 카운트 (0xD2)(0x13)
bool ReceiveIGS_EventItemlistCnt(BYTE* pReceiveBuffer)
{

	return true;
}

//----------------------------------------------------------------------------
// 이벤트 아이템 리스트 (0xD2)(0x14)
bool ReceiveIGS_EventItemlist(BYTE* pReceiveBuffer)
{

	return true;
}

//----------------------------------------------------------------------------
// 배너 버젼 업데이트 (0xD2)(0x15)
bool ReceiveIGS_UpdateBanner(BYTE* pReceiveBuffer)
{
	return true;
}

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

#ifdef PBG_ADD_SECRETBUFF
bool ReceiveFatigueTime(BYTE* pReceiveBuffer)
{
	LPPMSG_FATIGUEPERCENTAGE Data = (LPPMSG_FATIGUEPERCENTAGE)pReceiveBuffer;

	// 피로도 퍼센테이지를 서버로 받는다 1단위로->100%
	if(g_FatigueTimeSystem->SetFatiguePercentage(Data->btFatiguePercentage))
	{
		// 피로도 시스템을 적용하는 서버임
		g_FatigueTimeSystem->SetIsFatigueSystem(true);
#ifdef PBG_MOD_STAMINA_UI
		g_pNewUIStamina->SetCaution(Data->btFatiguePercentage);
#endif //PBG_MOD_STAMINA_UI
		return true;
	}
	else
		return false;

	return false;
}
#endif //PBG_ADD_SECRETBUFF

#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
//----------------------------------------------------------------------------
// 인벤 아이템 장착 결과 수신 (0xBF)(0x20)
// Data->btResult: 254 - 장착, 255 - 해제
//----------------------------------------------------------------------------
bool ReceiveEquippingInventoryItem(BYTE* pReceiveBuffer)
{
	LPPMSG_ANS_INVENTORY_EQUIPMENT_ITEM Data = (LPPMSG_ANS_INVENTORY_EQUIPMENT_ITEM)pReceiveBuffer;

	int iResult	 = (int)Data->btResult;
	int iItemPos = (int)Data->btItemPos;
	iItemPos -= MAX_EQUIPMENT_INDEX;
	
	// 정상적인 값(254-장착, 255-해제)이 아니면 return false
	if (!(iResult == 254 || iResult == 255))
		return false;
	// 비정상적이 아이템의 위치 
	if (iItemPos < 0 || iItemPos >= MAX_INVENTORY)
		return false;
	
	// 사용중인 아이템의 내구도 254, 장착해제중인 아이템 255
	ITEM* pItem = g_pMyInventory->FindItem(iItemPos);
	pItem->Durability = iResult;
	
#ifdef CONSOLE_DEBUG
			g_ConsoleDebug->Write(MCD_RECEIVE, "[0xBF][0x20]  [ReceiveEquippingInventoryItem]");
#endif // CONSOLE_DEBUG

	return true;
}
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY


#ifdef KJH_ADD_PERIOD_ITEM_SYSTEM
//----------------------------------------------------------------------------
// 기간제 아이템 리스트 카운트 (0xD2)(0x11)
bool ReceivePeriodItemListCount(BYTE* pReceiveBuffer)
{
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	LPPMSG_PERIODITEMEX_ITEMCOUNT Data = (LPPMSG_PERIODITEMEX_ITEMCOUNT)pReceiveBuffer;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX

	return true;
}

//----------------------------------------------------------------------------
// 기간제 아이템 리스트 (0xD2)(0x12)
bool ReceivePeriodItemList(BYTE* pReceiveBuffer)
{
	LPPMSG_PERIODITEMEX_ITEMLIST Data = (LPPMSG_PERIODITEMEX_ITEMLIST)pReceiveBuffer;

	if(Data->wItemSlotIndex < MAX_EQUIPMENT_INDEX)		//. 착용
	{
		CharacterMachine->Equipment[Data->wItemSlotIndex].lExpireTime = (long)Data->lExpireDate;
	}
	else	// 인벤
	{
		ITEM* pItem = g_pMyInventory->FindItem(Data->wItemSlotIndex-MAX_EQUIPMENT_INDEX);

		if( pItem == NULL )
			return false;

		pItem->lExpireTime = (long)Data->lExpireDate;
	}

	return true;
}
#endif // KJH_ADD_PERIOD_ITEM_SYSTEM

#ifdef PBG_MOD_GAMEGUARD_HANDLE
bool ReceiveGameGuardCharInfo(BYTE* pReceiveBuffer)
{
	LPPMSG_GAMEGUARD_HADLE Data = (LPPMSG_GAMEGUARD_HADLE)pReceiveBuffer;

	return true;
}
#endif //PBG_MOD_GAMEGUARD_HANDLE

#ifdef KJH_ADD_DUMMY_SKILL_PROTOCOL
bool ReceiveSkillDummySeedValue(BYTE* pReceiveBuffer)
{
	LPPMSG_SET_ATTACK_PROTOCOL_DUMMY_VALUE Data = (LPPMSG_SET_ATTACK_PROTOCOL_DUMMY_VALUE)pReceiveBuffer;
	
#ifdef KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL
	g_DummyAttackChecker->Initialize((DWORD)Data->dwDummySeedValue, (DWORD)Data->dwAttackSerialSeedValue);
#else // KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL
	g_DummyAttackChecker->Initialize((DWORD)Data->dwDummySeedValue);
#endif // KJH_ADD_CREATE_SERIAL_NUM_AT_ATTACK_SKILL

	return true;
}
#endif // KJH_ADD_DUMMY_SKILL_PROTOCOL

#ifdef LJH_ADD_ONETIME_PASSWORD
void ShowOTPInputWin()
{
	CUIMng& rUIMng = CUIMng::Instance();
	
	rUIMng.ShowWin(&rUIMng.m_OneTimePasswordWin);		// OneTime Password 입력창을 보여줌
}
#endif //LJH_ADD_ONETIME_PASSWORD

#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
BOOL ReceiveStraightAttack(BYTE *ReceiveBuffer, int Size, BOOL bEncrypted)
{
	if(!bEncrypted)
	{
		GO_DEBUG;
		SendHackingChecked(0x00, 0x4A);
		return (FALSE);
	}
	
	LPPRECEIVE_STRAIGHTATTACK Data = (LPPRECEIVE_STRAIGHTATTACK)ReceiveBuffer;
	int SourceKey = ((int)(Data->SourceKeyH)<<8) + Data->SourceKeyL;
	int TargetKey = ((int)(Data->TargetKeyH)<<8) + Data->TargetKeyL;
	int Success = (TargetKey>>15);

	WORD AttackNumber = ((WORD)(Data->AttackH)<<8) + Data->AttackL;
	
	TargetKey &= 0x7FFF;
	int Index       = FindCharacterIndex(SourceKey);
	int TargetIndex = FindCharacterIndex(TargetKey);
	if(TargetIndex == MAX_CHARACTERS_CLIENT && AttackNumber != AT_SKILL_DARKSIDE)
		return (TRUE);
	
	AttackPlayer = Index;
	CHARACTER *sc = &CharactersClient[Index];
	CHARACTER *tc = &CharactersClient[TargetIndex];
	OBJECT *so = &sc->Object;
	OBJECT *to = &tc->Object;

	if(sc!=Hero && to->Visible)
		so->Angle[2] = CreateAngle(so->Position[0],so->Position[1],to->Position[0],to->Position[1]);

	sc->TargetCharacter = TargetIndex;
		
	sc->SkillSuccess = (Success != 0);
	sc->Skill = AttackNumber;

	so->m_sTargetIndex = TargetIndex;

	if(g_CMonkSystem.IsRageHalfwaySkillAni(AttackNumber))
	{
		if(sc!=Hero && AttackNumber == AT_SKILL_STAMP)
		{
			g_CMonkSystem.SetRageSkillAni(AttackNumber, so);
			g_CMonkSystem.RageCreateEffect(so, AttackNumber);
		}
		else if(AttackNumber != AT_SKILL_STAMP)
		{
			g_CMonkSystem.SetRageSkillAni(AttackNumber, so);
		}
		sc->AttackTime = 1;
	}

	switch(AttackNumber)
	{
	case AT_SKILL_DARKSIDE:
		{
			if(sc != Hero)
				g_CMonkSystem.RageCreateEffect(so, AttackNumber);

			g_CMonkSystem.SetRageSkillAni(AttackNumber, so);
			sc->AttackTime = 1;
		}
		break;
	}

	return (TRUE);
}

void ReceiveDarkside(BYTE* ReceiveBuffer)
{
	LPPRECEIVE_DARKSIDE_INDEX Data = (LPPRECEIVE_DARKSIDE_INDEX)ReceiveBuffer;
	
	if(Data->MagicNumber == AT_SKILL_DARKSIDE)
	{
		g_CMonkSystem.InitDarksideTarget();
		g_CMonkSystem.SetDarksideTargetIndex(Data->TargerIndex);
	}
}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL

//////////////////////////////////////////////////////////////////////////
// TranslateProtocol
//////////////////////////////////////////////////////////////////////////
BOOL TranslateProtocol( int HeadCode, BYTE *ReceiveBuffer, int Size, BOOL bEncrypted)
{
	switch( HeadCode )
	{
	case 0xF1:     			
		{
			LPPHEADER_DEFAULT_SUBCODE Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
			switch( Data->SubCode )
			{
			case 0x00: //receive join server
                ReceiveJoinServer(ReceiveBuffer);
				break;
			case 0x01: //receive log in
                //AddDebugText(ReceiveBuffer,Size);
#ifdef LEM_ADD_GAMECHU
		#ifdef FOR_WORK
				g_ErrorReport.Write("\n\n!!!---- RECEIVE PACKET LOGINT : %d -------\n\n", Data->Value);				
		#endif // FOR_WORK
#endif // LEM_ADD_GAMECHU
				
				switch(Data->Value)
				{
				case 0x20:
					CurrentProtocolState = RECEIVE_LOG_IN_SUCCESS;
					LogIn = 2;
					m_pc_wanted = true;
                    CheckHack();
					break;
				case 0x00:
					CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_PASSWORD);
					break;
				case 0x01:
					CurrentProtocolState = RECEIVE_LOG_IN_SUCCESS;
					LogIn = 2;
                    CheckHack();
					break;
				case 0x02:
					CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_ID);
					break;
				case 0x03:
					CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_ID_CONNECTED);
					break;
				case 0x04:
					CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_SERVER_BUSY);
					break;
				case 0x05:
					CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_ID_BLOCK);
					break;
				case 0x06:
					CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_VERSION);
					g_ErrorReport.Write( "Version dismatch. - Login\r\n");
					break;
				case 0x07:
				default:	// 성민
					CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_CONNECT);
					break;
				case 0x08:
					CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_ERROR);
					break;
				case 0x09:
					CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_NO_PAYMENT_INFO);
					break;
				case 0x0a:
					CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_USER_TIME1);
					break;
				case 0x0b:
					CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_USER_TIME2);
					break;
				case 0x0c:
					CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_PC_TIME1);
					break;
				case 0x0d:
					CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_PC_TIME2);
					break;
				case 0x11:
					CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_ONLY_OVER_15);
					break;
#ifdef LEM_ADD_GAMECHU	// 게임츄 로그인 시도시 잘못된 정보로 인한 접속 실패 오류.
				case 0x12:
					CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_CONNECT);
					break;
#endif // LEM_ADD_GAMECHU
#ifdef ASG_ADD_CHARGED_CHANNEL_TICKET
				case 0x40:	// 유료채널 입장 불가.(글로벌 전용)
					CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_CHARGED_CHANNEL);
					break;
#endif	// ASG_ADD_CHARGED_CHANNEL_TICKET
				case 0xc0:
				case 0xd0:	// 대만 전용
					CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_POINT_DATE);
					break;
				case 0xc1:
				case 0xd1:
					CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_POINT_HOUR);
					break;
				case 0xc2:
				case 0xd2:
					CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_INVALID_IP);
					break;
#ifdef ADD_EXPERIENCE_SERVER
				case 0xb0:	// PC방에서만 접속 가능.
					CUIMng::Instance().PopUpMsgWin(MESSAGE_PCROOM_POSSIBLE);
					break;
#endif	// ADD_EXPERIENCE_SERVER
#ifdef LJH_ADD_ONETIME_PASSWORD
				//(0xF1 / 0x01, 0xA0): OTP 등록 유저
				case 0xA0:
					ShowOTPInputWin();
					break;
				//(0xF1 / 0x01, 0xA1): OTP 미등록 유저
				case 0xA1:	
					g_ErrorReport.Write( "this user not register OPT yet.");
					g_ErrorReport.WriteCurrentTime();
					CUIMng::Instance().PopUpMsgWin(MESSAGE_OTP_NOT_REGISTERED);
					break;
				//(0xF1 / 0x01, 0xA2): OTP 불일치
				case 0xA2:	
					g_ErrorReport.Write( "Incorrect OneTime Password.");
					g_ErrorReport.WriteCurrentTime();
					CUIMng::Instance().PopUpMsgWin(MESSAGE_OTP_INCORRECT);
					break;
				//(0xF1 / 0x01, 0xA3): 비정상적인 byOTPBtnType값을 보냈다는 서버의 응답  
				//(0xF1 / 0x01, 0xA4): 유효기간이 지난 OneTime Password
				case 0xA3:
				case 0xA4:
					g_ErrorReport.Write( "Invalid information received.");
					g_ErrorReport.WriteCurrentTime();
					CUIMng::Instance().PopUpMsgWin(MESSAGE_OTP_INVALID_INFO);
#endif //LJH_ADD_ONETIME_PASSWORD
				}
				break;
				case 0x02:
					if ( !ReceiveLogOut(ReceiveBuffer, bEncrypted))
					{
						return ( FALSE);
					}
					break;
				case 0x12: //0x02 receive create account
					switch(Data->Value)
					{
					case 0x00:
						CurrentProtocolState = RECEIVE_CREATE_ACCOUNT_FAIL_ID;
						break;
					case 0x01:
						CurrentProtocolState = RECEIVE_CREATE_ACCOUNT_SUCCESS;
						break;
					case 0x02:
						CurrentProtocolState = RECEIVE_CREATE_ACCOUNT_FAIL_RESIDENT;
						break;
					}
					break;
					case 0x03: //receive confirm password
						//AddDebugText(ReceiveBuffer,Size);
						ReceiveConfirmPassword(ReceiveBuffer);
						break;
					case 0x04: //receive confirm password
						//AddDebugText(ReceiveBuffer,Size);
						ReceiveConfirmPassword2(ReceiveBuffer);
						break;
					case 0x05: //receive change password
						ReceiveChangePassword(ReceiveBuffer);
						break;
			}
			break;
		}
	case 0xF3:     			
		{
			int subcode;
			if( ReceiveBuffer[0] == 0xC1 )
			{
				LPPHEADER_DEFAULT_SUBCODE Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
				subcode = Data->SubCode;
			}
			else
			{
				LPPHEADER_DEFAULT_SUBCODE_WORD Data = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer;
				subcode = Data->SubCode;
			}
			
#ifdef CONSOLE_DEBUG
			g_ConsoleDebug->Write(MCD_RECEIVE, "[0xF3][0x%02x]", subcode);
#endif // CONSOLE_DEBUG
			
			switch( subcode )
			{
			case 0x00: //receive characters list
                ReceiveCharacterList(ReceiveBuffer);
				break;
			case 0x01: //receive create character
                ReceiveCreateCharacter(ReceiveBuffer);
				break;
			case 0x02: //receive delete character
                ReceiveDeleteCharacter(ReceiveBuffer);
				break;
			case 0x03: //receive join map server
                if ( !ReceiveJoinMapServer(ReceiveBuffer, bEncrypted))
				{
					return ( FALSE);
				}
				break;
			case 0x04: //receive revival
                ReceiveRevival(ReceiveBuffer);
				break;
			case 0x10: //receive inventory
                //AddDebugText(ReceiveBuffer,Size);
                if ( !ReceiveInventory(ReceiveBuffer, bEncrypted))
				{
					return ( FALSE);
				}
				break;
			case 0x05: //receive level up
                ReceiveLevelUp(ReceiveBuffer);
				break;
			case 0x06: //receive Add Point
                ReceiveAddPoint(ReceiveBuffer);
				break;
			case 0x07: //receive damage
                //AddDebugText(ReceiveBuffer,Size);
                ReceiveDamage(ReceiveBuffer);
				break;
			case 0x08:
                //AddDebugText(ReceiveBuffer,Size);
                ReceivePK(ReceiveBuffer);
				break;
			case 0x11:
                //AddDebugText(ReceiveBuffer,Size);
                ReceiveMagicList(ReceiveBuffer);
				break;
			case 0x13:
                //AddDebugText(ReceiveBuffer,Size);
                ReceiveEquipment(ReceiveBuffer);
				break;
			case 0x14:
                //AddDebugText(ReceiveBuffer,Size);
                ReceiveModifyItem(ReceiveBuffer);
				break;
			case 0x20:
                ReceiveSummonLife(ReceiveBuffer);
				break;
			case 0x22://남은시간
                ReceiveWTTimeLeft(ReceiveBuffer);
				break;
				
			case 0x24://매치 결과
                ReceiveWTMatchResult(ReceiveBuffer);
				break;
				
			case 0x25://배틀사커 골인
				ReceiveWTBattleSoccerGoalIn(ReceiveBuffer);
				break;
			case 0x23://점수
                ReceiveSoccerScore(ReceiveBuffer);
				break;
            case 0x30://마법 핫키
                ReceiveOption(ReceiveBuffer);
                break;
			case 0x40:// 서버에서 클라이언트로의 명령어
				ReceiveServerCommand(ReceiveBuffer);
				break;
			case 0x50:
				Receive_Master_Level_Exp(ReceiveBuffer);
				break;
			case 0x51:
				Receive_Master_LevelUp(ReceiveBuffer);
				break;
			case 0x52:
				Receive_Master_LevelGetSkill(ReceiveBuffer);
				break;
#ifdef PJH_CHARACTER_RENAME
			case 0x15:
				Receive_Check_Change_Name(ReceiveBuffer);
				break;
			case 0x16:
				Receive_Change_Name_Result(ReceiveBuffer);
				break;
#endif //PJH_CHARACTER_RENAME
			}
			break;
		}
	case 0xF4:     			
		{
			int subcode;
			if( ReceiveBuffer[0] == 0xC1 )
			{
				LPPHEADER_DEFAULT_SUBCODE Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
				subcode = Data->SubCode;
			}
			else
			{
				LPPHEADER_DEFAULT_SUBCODE_WORD Data = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer;
				subcode = Data->SubCode;
			}
			switch( subcode )
			{
			case 0x06:
				ReceiveServerList(ReceiveBuffer);
				break;
			case 0x03:
				ReceiveServerConnect(ReceiveBuffer);
				break;
			case 0x05:
				ReceiveServerConnectBusy(ReceiveBuffer);
				break;
			}
			break;
		}
	case 0x00://chat
        ReceiveChat(ReceiveBuffer);
		break;
	case 0x01://chat
        ReceiveChatKey(ReceiveBuffer);
		break;
	case 0x02://chat whisper
        ReceiveChatWhisper(ReceiveBuffer);
		break;
	case 0x03: // 체크섬 요청
		ReceiveCheckSumRequest( ReceiveBuffer);
		break;
	case 0x0B://
        AddDebugText(ReceiveBuffer,Size);
        ReceiveEvent(ReceiveBuffer);
		break;
	case 0x0C://
        //AddDebugText(ReceiveBuffer,Size);
        ReceiveChatWhisperResult(ReceiveBuffer);
		break;
	case 0x0D://notice
        ReceiveNotice(ReceiveBuffer);
		break;
	case 0x0f:
        ReceiveWeather(ReceiveBuffer);
		break;
	case PACKET_MOVE: //move character
        ReceiveMoveCharacter(ReceiveBuffer,Size);
		break;
	case PACKET_POSITION: //move position
        ReceiveMovePosition(ReceiveBuffer);
		break;
	case 0x12: //create characters
        AddDebugText(ReceiveBuffer,Size);
        ReceiveCreatePlayerViewport(ReceiveBuffer,Size);
		break;
	case 0x13: //create monsters
        //AddDebugText(ReceiveBuffer,Size);
        ReceiveCreateMonsterViewport(ReceiveBuffer);
		break;
	case 0x1F: //create monsters
        //AddDebugText(ReceiveBuffer,Size);
        ReceiveCreateSummonViewport(ReceiveBuffer);
		break;
	case 0x45: //create monsters
        //AddDebugText(ReceiveBuffer,Size);
        ReceiveCreateTransformViewport(ReceiveBuffer);
		break;
	case 0x14: //delete characters & monsters
        //AddDebugText(ReceiveBuffer,Size);
        ReceiveDeleteCharacterViewport(ReceiveBuffer);
		break;
	case 0x20: //create item
        ReceiveCreateItemViewport(ReceiveBuffer);
		break;
	case 0x21://delete item
        ReceiveDeleteItemViewport(ReceiveBuffer);
		break;
	case 0x22://get item
        //AddDebugText(ReceiveBuffer,Size);
        ReceiveGetItem(ReceiveBuffer);
		break;
	case 0x23://drop item
        ReceiveDropItem(ReceiveBuffer);
		break;
	case 0x24://equipment item
        AddDebugText(ReceiveBuffer,Size);
        if ( !ReceiveEquipmentItem(ReceiveBuffer, bEncrypted))
		{
			return ( FALSE);
		}
		break;
	case 0x25://change character
        ReceiveChangePlayer(ReceiveBuffer);
		break;
	case PACKET_ATTACK://attack character
        ReceiveAttackDamage(ReceiveBuffer);
		break;
	case 0x18://action character
        ReceiveAction(ReceiveBuffer,Size);
		break;
	case 0x19://magic
        if ( !ReceiveMagic(ReceiveBuffer,Size, bEncrypted))
		{
			return ( FALSE);
		}
		break;
	case 0x69:
        if ( !ReceiveMonsterSkill(ReceiveBuffer,Size, bEncrypted))
		{
			return ( FALSE);
		}
		break;
	case 0x1A://magic
        ReceiveMagicPosition(ReceiveBuffer,Size);
		break;
	case 0x1E://magic
        if ( !ReceiveMagicContinue(ReceiveBuffer,Size, bEncrypted))
		{
			return ( FALSE);
		}
		break;
	case 0x1B://magic
        ReceiveMagicFinish(ReceiveBuffer);
		break;
	case 0x07://setmagicstatus
		ReceiveSkillStatus(ReceiveBuffer);
		break;
	case 0x16://die character(exp)
        if ( !ReceiveDieExp(ReceiveBuffer,bEncrypted))
		{
			return ( FALSE);
		}
		break;
	case 0x9C://die character(exp)
        if ( !ReceiveDieExpLarge(ReceiveBuffer,bEncrypted))
		{
			return ( FALSE);
		}
		break;
	case 0x17://die character
        ReceiveDie(ReceiveBuffer,Size);
		break;
	case 0x2A:
        AddDebugText(ReceiveBuffer,Size);
        ReceiveDurability(ReceiveBuffer);
		break;
	case 0x26:
        ReceiveLife(ReceiveBuffer);
		break;
	case 0x27:
        ReceiveMana(ReceiveBuffer);
		break;
	case 0x28:
        ReceiveDeleteInventory(ReceiveBuffer);
		break;
	case 0x29:
        //AddDebugText(ReceiveBuffer,Size);
        if ( !ReceiveHelperItem(ReceiveBuffer,bEncrypted))
		{
			return ( FALSE);
		}
		break;
    case 0x2c:
        ReceiveUseStateItem ( ReceiveBuffer );
        break;
	case 0x30:
        if ( !ReceiveTalk(ReceiveBuffer, bEncrypted))
		{
			return ( FALSE);
		}
		break;
	case 0x31:
        //AddDebugText(ReceiveBuffer,Size);
        ReceiveTradeInventory(ReceiveBuffer);
		break;
	case 0x32:
        ReceiveBuy(ReceiveBuffer);
		break;
	case 0x33:
        ReceiveSell(ReceiveBuffer);
		break;
    case 0x34:
        ReceiveRepair(ReceiveBuffer);
        break;
	case 0x36:
        if ( !ReceiveTrade(ReceiveBuffer, bEncrypted))
		{
			return ( FALSE);
		}
		break;
	case 0x37:
        //AddDebugText(ReceiveBuffer,Size);
        ReceiveTradeResult(ReceiveBuffer);
		break;
	case 0x38:
        //AddDebugText(ReceiveBuffer,Size);
        ReceiveTradeYourInventoryDelete(ReceiveBuffer);
		break;
	case 0x39:
        //AddDebugText(ReceiveBuffer,Size);
        ReceiveTradeYourInventory(ReceiveBuffer);
		break;
	case 0x3A:
        //AddDebugText(ReceiveBuffer,Size);
        ReceiveTradeMyGold(ReceiveBuffer);
		break;
	case 0x3B:
        //AddDebugText(ReceiveBuffer,Size);
        ReceiveTradeYourGold(ReceiveBuffer);
		break;
	case 0x3C:
        //AddDebugText(ReceiveBuffer,Size);
        ReceiveTradeYourResult(ReceiveBuffer);
		break;
	case 0x3D:
        //AddDebugText(ReceiveBuffer,Size);
        ReceiveTradeExit(ReceiveBuffer);
		break;
	case 0x1C:
        AddDebugText(ReceiveBuffer,Size);
		if ( !ReceiveTeleport(ReceiveBuffer, bEncrypted))
		{
			return ( FALSE);
		}
		break;
	case 0x40:
        ReceiveParty(ReceiveBuffer);
		break;
	case 0x41:
        ReceivePartyResult(ReceiveBuffer);
		break;
	case 0x42:
        ReceivePartyList(ReceiveBuffer);
		break;
	case 0x43:
        ReceivePartyLeave(ReceiveBuffer);
		break;
    case 0x44:
        ReceivePartyInfo(ReceiveBuffer);
        break;
    case 0x46:  //  속성 변경.
        ReceiveSetAttribute(ReceiveBuffer);
        break;
    case 0x47:
        ReceivePartyGetItem( ReceiveBuffer );
        break;
	case 0x48:
		ReceiveDisplayEffectViewport( ReceiveBuffer );
		break;
	case 0x50:
        ReceiveGuild(ReceiveBuffer);
		break;
	case 0x51:
        ReceiveGuildResult(ReceiveBuffer);
		break;
	case 0x52:
        ReceiveGuildList(ReceiveBuffer);
		break;
	case 0x53:
        ReceiveGuildLeave(ReceiveBuffer);
		break;
	case 0x54:	// 길드마스터 NPC 클릭시
        ReceiveCreateGuildInterface(ReceiveBuffer);
		break;
	case 0x55:	// 길드마스터 NPC 클릭후 길드 생성 클릭시
        ReceiveCreateGuildMasterInterface(ReceiveBuffer);
		break;
	case 0x56:
        ReceiveCreateGuildResult(ReceiveBuffer);
		break;
	case 0x65:
		ReceiveGuildIDViewport(ReceiveBuffer);
		break;
	case 0x66:
		ReceiveGuildInfo(ReceiveBuffer);
		break;
	case 0x5D:
        ReceiveDeleteGuildViewport(ReceiveBuffer);
		break;
	case 0x60:
        ReceiveDeclareWarResult(ReceiveBuffer);
		break;
	case 0x61:
        ReceiveDeclareWar(ReceiveBuffer);
		break;
	case 0x62:
        ReceiveGuildBeginWar(ReceiveBuffer);
		break;
	case 0x63:
        ReceiveGuildEndWar(ReceiveBuffer);
		break;
	case 0x64:
        ReceiveGuildWarScore(ReceiveBuffer);
		break;
	case 0xE1:	// 길드직책을 임명/변경/해제 결과
		ReceiveGuildAssign(ReceiveBuffer);
		break;
	case 0xE5:	// 길드관계 요청  
		ReceiveGuildRelationShip(ReceiveBuffer);
		break;
	case 0xE6:	// 길드관계 요청결과  
		ReceiveGuildRelationShipResult(ReceiveBuffer);
		break;
	case 0xEB:
		{
			LPPHEADER_DEFAULT_SUBCODE Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
			switch(Data->SubCode)
			{
			case 0x01:	// 연합길드 방출
				ReceiveBanUnionGuildResult(ReceiveBuffer);
				break;
			}
		}
		break;
	case 0x67:	// 길드연합정보 변경  
		ReceiveUnionViewportNotify(ReceiveBuffer);
		break;
	case 0xE9:	// 연합리스트  
		ReceiveUnionList(ReceiveBuffer);
		break;
	case 0xBC:
		{
			LPPHEADER_DEFAULT_SUBCODE Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
			switch(Data->SubCode)
			{
			case 0x00:
				ReceiveGemMixResult(ReceiveBuffer);
				break;
			case 0x01:
				ReceiveGemUnMixResult(ReceiveBuffer);
				break;
			}
		}
		break;		
    case 0x68:  //  임시 뷰포트 처리.
        ReceivePreviewPort ( ReceiveBuffer );
        break;
		
	case 0x71:
		ReceivePing(ReceiveBuffer);
		break;
#ifdef NP_GAME_GUARD
#ifdef GG_SERVER_AUTH
	case 0x73:
		if ( !ReceiveGGAuth(ReceiveBuffer, bEncrypted))
		{
			return ( FALSE);
		}
		break;
#endif
#endif
	case 0x81:
        ReceiveStorageGold(ReceiveBuffer);
		break;
	case 0x82:
        ReceiveStorageExit(ReceiveBuffer);
		break;
	case 0x83:
		ReceiveStorageStatus(ReceiveBuffer);
		break;
	case 0x86:
        ReceiveMix(ReceiveBuffer);
		break;
	case 0x87:
        ReceiveMixExit(ReceiveBuffer);
		break;
#ifdef YDG_MOD_PROTECT_AUTO_V5
	case 0x8A:
		ReceiveRequestCheckAutoTool(ReceiveBuffer);
		break;
#endif	// YDG_MOD_PROTECT_AUTO_V5
#ifdef YDG_ADD_MOVE_COMMAND_PROTOCOL
	case 0x8E:
		{
			LPPHEADER_DEFAULT_SUBCODE Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
			switch(Data->SubCode)
			{
			case 0x01:
				ReceiveMoveMapChecksum(ReceiveBuffer);
				break;
			case 0x03:
				ReceiveRequestMoveMap(ReceiveBuffer);
				break;
			}
		}
		break;
#endif	// YDG_ADD_MOVE_COMMAND_PROTOCOL
#ifdef KJH_ADD_DUMMY_SKILL_PROTOCOL
	case 0x8D:
		{
			LPPHEADER_DEFAULT_SUBCODE Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
			switch(Data->SubCode)
			{
			case 0x05:
				ReceiveSkillDummySeedValue(ReceiveBuffer);
				break;
			}
		}
		break;
#endif // KJH_ADD_DUMMY_SKILL_PROTOCOL
	case 0x90:
		ReceiveMoveToDevilSquareResult(ReceiveBuffer);
		break;
	case 0x91:
		ReceiveEventZoneOpenTime(ReceiveBuffer);
		break;
	case 0x92:
		ReceiveDevilSquareCountDown(ReceiveBuffer);
		break;
	case 0x93:
		ReceiveDevilSquareRank(ReceiveBuffer);
		break;
	case 0x9A://    ...
		ReceiveMoveToEventMatchResult(ReceiveBuffer);
		break;
    case 0x9B://    이벤트 게임 명령을 받는다.
        ReceiveMatchGameCommand ( ReceiveBuffer );
        break;
    case 0x94://    이벤트칩 정보를 받는다.
        ReceiveEventChipInfomation(ReceiveBuffer);
        break;
    case 0x95://    이벤트칩수.
        ReceiveEventChip(ReceiveBuffer);
        break;
    case 0x96://    뮤토 번호.
        ReceiveMutoNumber(ReceiveBuffer);
        break;
	case 0x99:	// 서버 분할 결과
		ReceiveServerImmigration(ReceiveBuffer);
		break;
#ifdef SCRATCH_TICKET
	case 0x9D:	//	복권 당첨 상품.
		ReceiveScratchResult( ReceiveBuffer );
		break;
#endif
	case 0x9E:	//	서버 제공 사운드
		ReceivePlaySoundEffect( ReceiveBuffer );
		break;
    case 0x9F:  //  이벤트 사용횟수.
        ReceiveEventCount ( ReceiveBuffer );
        break;
    case 0xA0://    퀘스트 리스트를 받는다.
        ReceiveQuestHistory ( ReceiveBuffer );
        break;
    case 0xA1://    특정 퀘스트 상태 정보를 받는다.
        ReceiveQuestState ( ReceiveBuffer );
        break;
    case 0xA2://    특정 퀘스트의 결과를 받는다.    
        ReceiveQuestResult ( ReceiveBuffer );
        break;
    case 0xA3://    퀘스트의 보상를 받는다.
        ReceiveQuestPrize ( ReceiveBuffer );
        break;
	case 0xA4:
		ReceiveQuestMonKillInfo(ReceiveBuffer);
		break;
#ifdef ASG_ADD_NEW_QUEST_SYSTEM
	case 0xF6:
		{
			LPPHEADER_DEFAULT_SUBCODE Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
			switch (Data->SubCode)
			{
#ifdef ASG_ADD_TIME_LIMIT_QUEST
			case 0x00:
				ReceiveQuestLimitResult(ReceiveBuffer);		// 서버로 부터 퀘스트 제한 시 응답.
				break;
#endif	// ASG_ADD_TIME_LIMIT_QUEST
			case 0x03:
				ReceiveQuestByEtcEPList(ReceiveBuffer);		// 기타 상황의 퀘스트 EP(에피소드)리스트를  .
				break;
#ifdef ASG_ADD_TIME_LIMIT_QUEST
			case 0x04:
				ReceiveQuestByItemUseEP(ReceiveBuffer);		// 퀘스트 아이템 사용에 의한 EP(에피소드)를  .
#endif	// ASG_ADD_TIME_LIMIT_QUEST
			case 0x0A:
				ReceiveQuestByNPCEPList(ReceiveBuffer);		// NPC에 의한 EP(에피소드)리스트를  .
				break;
			case 0x0B:
				ReceiveQuestQSSelSentence(ReceiveBuffer);	// 선택문이 있는 QS(퀘스트 상태)를  
				break;
			case 0x0C:
				ReceiveQuestQSRequestReward(ReceiveBuffer);	// 요구, 보상이 있는 QS(퀘스트 상태)를  .
				break;
			case 0x0D:
				ReceiveQuestCompleteResult(ReceiveBuffer);	// 요구, 보상 결과를  .
				break;
			case 0x0F:
				ReceiveQuestGiveUp(ReceiveBuffer);			// 퀘스트 포기했음을 응답  .
				break;
			case 0x1A:
				ReceiveProgressQuestList(ReceiveBuffer);	// 진행 중인 퀘스트 리스트를  .
				break;
			case 0x1B:
				ReceiveProgressQuestRequestReward(ReceiveBuffer);// 진행중인 퀘스트 리스트에서 특정 퀘스트의 요구, 보상 정보를  .
				break;
#ifdef ASG_FIX_QUEST_PROTOCOL_ADD
			case 0x20:
				ReceiveProgressQuestListReady(ReceiveBuffer);	// 진행 중인 퀘스트 리스트를 줄 준비가 됨.
				break;
#endif	// ASG_FIX_QUEST_PROTOCOL_ADD
			}
		}
		break;
#endif	// ASG_ADD_NEW_QUEST_SYSTEM

#ifdef ASG_ADD_GENS_SYSTEM
	case 0xF8:
		{
			BYTE bySubcode;

			if (ReceiveBuffer[0] == 0xC1)
			{
				LPPHEADER_DEFAULT_SUBCODE Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
				bySubcode = Data->SubCode;
			}
			else
			{
				LPPHEADER_DEFAULT_SUBCODE_WORD Data = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer;
				bySubcode = Data->SubCode;
			}

			switch (bySubcode)
			{
			case 0x02:
				ReceiveGensJoining(ReceiveBuffer);			// 겐스 가입 요청 응답. 
				break;
			case 0x04:
				ReceiveGensSecession(ReceiveBuffer);		// 겐스 탈퇴 요청 응답. 
				break;
			case 0x07:
				ReceivePlayerGensInfluence(ReceiveBuffer);	// 플래이어의 겐스 세력 정보  .
				break;
			case 0x05:
				ReceiveOtherPlayerGensInfluenceViewport(ReceiveBuffer);	// 뷰포트 안의 다른 플래이어의 겐스 세력 정보  .
				break;
#ifdef PBG_ADD_GENSRANKING
			case 0x0A:
				ReceiveReward(ReceiveBuffer);				// 보상받기 요청 응답
				break;
#endif //PBG_ADD_GENSRANKING
			}
		}
		break;
#endif	// ASG_ADD_GENS_SYSTEM
#ifdef ASG_ADD_UI_NPC_DIALOGUE
	case 0xF9:
		{
			LPPHEADER_DEFAULT_SUBCODE Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
			switch (Data->SubCode)
			{
			case 0x01:
				ReceiveNPCDlgUIStart(ReceiveBuffer);		// NPC UI가 대화로 진행되는 NPC 클릭 때 응답  . 
				break;
			}
		}
		break;
#endif	// ASG_ADD_UI_NPC_DIALOGUE

#ifdef PET_SYSTEM
    case 0xA7:
        ReceivePetCommand ( ReceiveBuffer );
        break;
    case 0xA8:
        ReceivePetAttack ( ReceiveBuffer );
        break;
    case 0xA9:
        ReceivePetInfo ( ReceiveBuffer );
        break;
#endif// PET_SYSTEM
		
#ifdef YDG_ADD_NEW_DUEL_PROTOCOL
	case 0xAA:
		{
			LPPHEADER_DEFAULT_SUBCODE Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
			switch ( Data->SubCode )
			{
			case 0x01:	// 결투요청 결과
				ReceiveDuelStart( ReceiveBuffer );
				break;
			case 0x02:	// 결투요청
				ReceiveDuelRequest( ReceiveBuffer );
				break;
			case 0x03:	// 결투종료
				ReceiveDuelEnd( ReceiveBuffer );
				break;
			case 0x04:	// 결투자 점수 정보
				ReceiveDuelScore( ReceiveBuffer );
				break;
			case 0x05:	// 결투자 생명 정보
				ReceiveDuelHP( ReceiveBuffer );
				break;
			case 0x06:	// 결투 채널(관전) 리스트
				ReceiveDuelChannelList( ReceiveBuffer );
				break;
			case 0x07:	// 채널(관전) 요청
				ReceiveDuelWatchRequestReply( ReceiveBuffer );
				break;
			case 0x08:	// 채널(관전) 입장한 관전자
				ReceiveDuelWatcherJoin( ReceiveBuffer );
				break;
			case 0x09:	// 채널(관전) 종료
				ReceiveDuelWatchEnd( ReceiveBuffer );
				break;
			case 0x0A:	// 채널(관전) 떠난 관전자
				ReceiveDuelWatcherQuit( ReceiveBuffer );
				break;
			case 0x0B:	// 관전자 리스트(All)
				ReceiveDuelWatcherList( ReceiveBuffer );
				break;
			case 0x0C:	// 결투결과 정보
				ReceiveDuelResult( ReceiveBuffer );
				break;
			case 0x0D:	// 라운드 시작/끝 알림
				ReceiveDuelRound( ReceiveBuffer );
				break;
			}
		}
		break;
#else	// YDG_ADD_NEW_DUEL_PROTOCOL
#ifdef DUEL_SYSTEM
	case 0xAA:
		ReplyDuelStart( ReceiveBuffer );
		break;
	case 0xAB:
		ReplyDuelEnd( ReceiveBuffer );
		break;
	case 0xAC:
		NotifyDuelStart( ReceiveBuffer );
		break;
	case 0xAD:
		NotifyDuelScore( ReceiveBuffer );
		break;
#endif // DUEL_SYSTEM
#endif	// YDG_ADD_NEW_DUEL_PROTOCOL
#ifdef LDK_ADD_EMPIREGUARDIAN_PROTOCOLS
		//제국 수호군
	case 0xF7:
		{
			LPPHEADER_DEFAULT_SUBCODE Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
			switch ( Data->SubCode )
			{
			case 0x02:
				ReceiveEnterEmpireGuardianEvent( ReceiveBuffer );
				break;

			case 0x04:
				ReceiveRemainTickEmpireGuardian( ReceiveBuffer );
				break;

			case 0x06:
				ReceiveResultEmpireGuardian( ReceiveBuffer );
				break;
			}
		}
		break;
#endif //LDK_ADD_EMPIREGUARDIAN_PROTOCOLS
	case 0x3F:
		{
			int subcode;
			if( ReceiveBuffer[0] == 0xC1 )
			{
				LPPHEADER_DEFAULT_SUBCODE Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
				subcode = Data->SubCode;
			}
			else
			{
				LPPHEADER_DEFAULT_SUBCODE_WORD Data = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer;
				subcode = Data->SubCode;
			}
			switch(subcode)
			{
			case 0x00:
				ReceiveCreateShopTitleViewport( ReceiveBuffer );
				break;
			case 0x01:
				ReceiveSetPriceResult( ReceiveBuffer );
				break;
			case 0x02:
				ReceiveCreatePersonalShop( ReceiveBuffer );
				break;
			case 0x03:
				ReceiveDestroyPersonalShop( ReceiveBuffer );
				break;
			case 0x05:
				ReceivePersonalShopItemList( ReceiveBuffer );
				break;
			case 0x06:
				ReceivePurchaseItem( ReceiveBuffer );
				break;
			case 0x08:
				NotifySoldItem( ReceiveBuffer );
				break;
			case 0x10:
				ReceiveShopTitleChange( ReceiveBuffer );
				break;
			case 0x12:
				NotifyClosePersonalShop( ReceiveBuffer );
				break;
			case 0x13:
				ReceiveRefreshItemList( ReceiveBuffer );
				break;
			}
		}
		break;
    case 0xAF:
        {
            LPPHEADER_DEFAULT_SUBCODE Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
            switch ( Data->SubCode )
            {
            case 0x01:
                ReceiveMoveToEventMatchResult2 ( ReceiveBuffer );
                break;
            }
        }
        break;
    case 0xB1:
        {
			int subcode;
			if( ReceiveBuffer[0] == 0xC1 )
			{
				LPPHEADER_DEFAULT_SUBCODE Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
				subcode = Data->SubCode;
			}
			else
			{
				LPPHEADER_DEFAULT_SUBCODE_WORD Data = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer;
				subcode = Data->SubCode;
			}
			switch(subcode)
			{
            case 0x00:
                ReceiveChangeMapServerInfo ( ReceiveBuffer );
                break;
				
            case 0x01:
                ReceiveChangeMapServerResult ( ReceiveBuffer );
                break;
            }
        }
        break;
	case 0xB2:
		{
			int subcode;
			if( ReceiveBuffer[0] == 0xC1 )
			{
				LPPHEADER_DEFAULT_SUBCODE Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
				subcode = Data->SubCode;
			}
			else
			{
				LPPHEADER_DEFAULT_SUBCODE_WORD Data = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer;
				subcode = Data->SubCode;
			}
			switch( subcode )
			{
			case 0x00:	// 공성 정보
				ReceiveBCStatus( ReceiveBuffer );
				break;
			case 0x01:	// 공성 등록결과
				ReceiveBCReg( ReceiveBuffer );
				break;
			case 0x02:	// 공성 포기결과
				ReceiveBCGiveUp( ReceiveBuffer );
				break;
			case 0x03:	// 공성 등록정보
				ReceiveBCRegInfo( ReceiveBuffer );
				break;
			case 0x04:	// 표식 등록결과
				ReceiveBCRegMark( ReceiveBuffer );
				break;
			case 0x05:	// 수성측 NPC 구매
				ReceiveBCNPCBuy( ReceiveBuffer );
				break;
			case 0x06:	// 수성측 NPC 수리
				ReceiveBCNPCRepair( ReceiveBuffer );
				break;
			case 0x07:	// 수성측 NPC 강화
				ReceiveBCNPCUpgrade( ReceiveBuffer );
				break;
			case 0x08:	// 현재 성의 자금, 세금 정보 요청 응답 (0xC1)
				ReceiveBCGetTaxInfo( ReceiveBuffer );
				break;
			case 0x09:	// 현재 성의 세율 변경 요청 응답 (0xC1)
				ReceiveBCChangeTaxRate( ReceiveBuffer );
				break;
			case 0x10:	// 현재 성의 자금 출금 요청 응답 (0xC1)
				ReceiveBCWithdraw( ReceiveBuffer );
				break;
			case 0x1A:	// 세율 변경
				ReceiveTaxInfo( ReceiveBuffer );
				break;
            case 0x1F:  //  문지기 입장요금.
                ReceiveHuntZoneEnter( ReceiveBuffer );
                break;
				
            case 0x11:
                ReceiveGateState ( ReceiveBuffer );
                break;
				
            case 0x12:
                ReceiveGateOperator ( ReceiveBuffer );
                break;
				
            case 0x13:
                ReceiveGateCurrentState ( ReceiveBuffer );
                break;
				
            case 0x14:
                ReceiveCrownSwitchState ( ReceiveBuffer );
                break;
				
            case 0x15:
                ReceiveCrownRegist ( ReceiveBuffer );
                break;
				
            case 0x16:
                ReceiveCrownState ( ReceiveBuffer );
                break;
				
            case 0x17:
                ReceiveBattleCastleStart ( ReceiveBuffer );
                break;
				
            case 0x18:
                ReceiveBattleCastleProcess ( ReceiveBuffer );
                break;
				
            case 0x19:
                ReceiveBattleCastleRegiment ( ReceiveBuffer );
                break;
				
			case 0x20:
				ReceiveBattleCasleSwitchInfo (ReceiveBuffer);
				break;
				
            case 0x1B:
                ReceiveMapInfoResult ( ReceiveBuffer );
                break;
				
            case 0x1D:
                ReceiveGuildCommand ( ReceiveBuffer );
                break;
				
            case 0x1E:
                ReceiveMatchTimer ( ReceiveBuffer );
                break;
			}
		}
		break;
		
	case 0xB3:	// 수성 NPC 목록
		ReceiveBCNPCList( ReceiveBuffer );
		break;
	case 0xB4:	// 공성 등록 길드 목록
		ReceiveBCDeclareGuildList( ReceiveBuffer );
		break;
	case 0xB5:	// 공성 확정 길드 목록
		ReceiveBCGuildList( ReceiveBuffer );
		break;
    case 0xB6:  //  길드원들의 위치 정보를 알아낸다.
        ReceiveGuildMemberLocation ( ReceiveBuffer );
        break;
    case 0xBB:  //  맵에 존재하는 NPC정보를 알아낸다.
        ReceiveGuildNpcLocation ( ReceiveBuffer );
        break;
    case 0xB7:
        {
			int subcode;
			if( ReceiveBuffer[0] == 0xC1 )
			{
				LPPHEADER_DEFAULT_SUBCODE Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
				subcode = Data->SubCode;
			}
			else
			{
				LPPHEADER_DEFAULT_SUBCODE_WORD Data = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer;
				subcode = Data->SubCode;
			}
			switch( subcode )
			{
            case 0x00:
                ReceiveCatapultState ( ReceiveBuffer );
                break;
				
            case 0x01:
                ReceiveCatapultFire ( ReceiveBuffer );
                break;
				
            case 0x02:
                ReceiveCatapultFireToMe ( ReceiveBuffer );
                break;
				
            case 0x03:
				//                ReceiveDamageFrmeCatapult ( ReceiveBuffer );
                break;
            }
        }
        break;
		
    case 0xB8:  //  배틀마스터 스킬.
        {
			int subcode;
			if( ReceiveBuffer[0] == 0xC1 )
			{
				LPPHEADER_DEFAULT_SUBCODE Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
				subcode = Data->SubCode;
			}
			else
			{
				LPPHEADER_DEFAULT_SUBCODE_WORD Data = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer;
				subcode = Data->SubCode;
			}
			switch( subcode )
			{
            case 0x01:
                ReceiveKillCount ( ReceiveBuffer );
                break;
				
            case 0x02:
                break;
            }
        }
        break;
    case 0xB9:
        {
			int subcode;
			if( ReceiveBuffer[0] == 0xC1 )
			{
				LPPHEADER_DEFAULT_SUBCODE Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
				subcode = Data->SubCode;
			}
			else
			{
				LPPHEADER_DEFAULT_SUBCODE_WORD Data = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer;
				subcode = Data->SubCode;
			}
			switch( subcode )
			{
            case 0x01:
                ReceiveBuildTime ( ReceiveBuffer );
                break;
				
            case 0x02:
                ReceiveCastleGuildMark ( ReceiveBuffer );
                break;
				
            case 0x03:
                ReceiveCastleHuntZoneInfo ( ReceiveBuffer );
                break;
				
            case 0x05:
                ReceiveCastleHuntZoneResult ( ReceiveBuffer );
                break;
            }
        }
        break;
		
	case 0xBA:
		ReceiveSkillCount ( ReceiveBuffer );
		break;
	case 0xBD:
		{
			int subcode;
			if( ReceiveBuffer[0] == 0xC1 )
			{
				LPPHEADER_DEFAULT_SUBCODE Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
				subcode = Data->SubCode;
			}
			else
			{
				LPPHEADER_DEFAULT_SUBCODE_WORD Data = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer;
				subcode = Data->SubCode;
			}
			switch(subcode)
			{
			case 0x00:
				ReceiveCrywolfInfo(ReceiveBuffer);  // 크라이울프 맵 상태를 받는다.
				break;
			case 0x02:
				ReceiveCrywolStateAltarfInfo(ReceiveBuffer);  // 크라이울프 석상 방어막, 제단 상태를 받는다.
				break;
			case 0x03:
				ReceiveCrywolfAltarContract(ReceiveBuffer);  // 제단 등록 결과를 받는다.
				break;
			case 0x04:
				ReceiveCrywolfLifeTime(ReceiveBuffer);  // 크라이울프 진행 시간에 대한 정보를 받는다.
				break;
			case 0x0C:
				ReceiveCrywolfTankerHit(ReceiveBuffer);
				break;
			case 0x05:
				ReceiveCrywolfBossMonsterInfo(ReceiveBuffer);
				break;
			case 0x07:
				ReceiveCrywolfPersonalRank(ReceiveBuffer);
				break;
			case 0x09:
				ReceiveCrywolfBenefitPlusChaosRate(ReceiveBuffer);
				break;
			case 0x08:
				ReceiveCrywolfHeroList(ReceiveBuffer);
				break;
			}
			break;
		}
	case 0xC0:	// 친구 리스트를 받는다.
		ReceiveFriendList(ReceiveBuffer);
		break;
	case 0xC1:	// 친구 등록 요청 결과를 받는다. (등록한 사람)
		ReceiveAddFriendResult(ReceiveBuffer);
		break;
	case 0xC2:	// 친구 등록 요청을 받는다. (등록 받은 사람)
		ReceiveRequestAcceptAddFriend(ReceiveBuffer);
		break;
	case 0xC3:	// 친구 삭제 요청 결과를 받는다.
		ReceiveDeleteFriendResult(ReceiveBuffer);
		break;
	case 0xC4:	// 친구 친구 상태 변화를 받는다.
		ReceiveFriendStateChange(ReceiveBuffer);
		break;
	case 0xC5:	// 편지 보낸 결과 받기
		ReceiveLetterSendResult(ReceiveBuffer);
		break;
	case 0xC6:	// 편지 받기
		ReceiveLetter(ReceiveBuffer);
		break;
	case 0xC7:	// 편지 내용 받기
		ReceiveLetterText(ReceiveBuffer);
		break;
	case 0xC8:	// 편지 삭제 결과 받기
		ReceiveLetterDeleteResult(ReceiveBuffer);
		break;
	case 0xCA:	// 대화방 개설 요청 결과 받기
		ReceiveCreateChatRoomResult(ReceiveBuffer);
		break;
	case 0xCB:	// 초대 결과 받기
		ReceiveChatRoomInviteResult(ReceiveBuffer);
		break;
	case 0x2D:	// 모든 버프의 시작과 종료를 알려준다.( 히어로 캐릭터 )
		ReceiveBuffState(ReceiveBuffer);
		break;
	case 0xD1:
		{
			int subcode;
			if( ReceiveBuffer[0] == 0xC1 )
			{
				LPPHEADER_DEFAULT_SUBCODE Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
				subcode = Data->SubCode;
			}
			else
			{
				LPPHEADER_DEFAULT_SUBCODE_WORD Data = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer;
				subcode = Data->SubCode;
			}
			switch(subcode)
			{
			case 0x00:		// 칸투르 보스전의 현재 상태 정보 요청에 대한 응답
				ReceiveKanturu3rdStateInfo(ReceiveBuffer);
				break;
			case 0x01:		// 칸투르 보스전 전투맵 입장 요청에 대한 응답
				ReceiveKanturu3rdEnterBossMap(ReceiveBuffer);
				break;
			case 0x02:		// 칸투르 보스전 상태와 그에 따른 연출 관련 상태 정보(맵에 처음 들어갈때 만)
				ReceiveKanturu3rdCurrentState(ReceiveBuffer);
				break;
			case 0x03:		// 칸투르 보스전 상태와 그에 따른 연출 관련 상태 정보
				ReceiveKanturu3rdState(ReceiveBuffer);
				break;
			case 0x04:		// 칸투르 보스전 전투 성공,실패 결과 정보
				ReceiveKanturu3rdResult(ReceiveBuffer);
				break;
			case 0x05:		// 칸투르 전투 제한 시간
				ReceiveKanturu3rdTimer(ReceiveBuffer);
				break;
			case 0x06:		// 칸투르 마야 전체 스킬 정보
				RecevieKanturu3rdMayaSKill(ReceiveBuffer);
				break;
			case 0x07:		// 칸투르 남은 인원 과 몬스터 수 표시
				RecevieKanturu3rdLeftUserandMonsterCount(ReceiveBuffer);
				break;
#ifdef CSK_RAKLION_BOSS
			case 0x10:		// 유저의 라클리온 보스전의 현재 상태 정보 요청에 대한 응답
				ReceiveRaklionStateInfo(ReceiveBuffer);
				break;
			case 0x11:		// 유저가 라클리온 보스전 전투맵에 입장했을 때 현재 상태를 보낸다.
				ReceiveRaklionCurrentState(ReceiveBuffer);
				break;
			case 0x12:		// 라클리온 보스전 도중 상태가 변했을 때 상태값을 알린다.
				RecevieRaklionStateChange(ReceiveBuffer);
				break;
			case 0x13:		// 라클리온 보스전 전투 성공 / 실패
				RecevieRaklionBattleResult(ReceiveBuffer);
				break;
			case 0x14:		// 라클리온 보스전 광역 공격( 세루판 스킬 사용 )
				RecevieRaklionWideAreaAttack(ReceiveBuffer);
				break;
			case 0x15:		// 라클리온 입장 유저&몬스터 수 디스플레이
				RecevieRaklionUserMonsterCount(ReceiveBuffer);
				break;	
#endif // CSK_RAKLION_BOSS
			}
			break;
		}
#ifdef ADD_PCROOM_POINT_SYSTEM
	case 0xD0:
		{
			int nSubCode;
			if (ReceiveBuffer[0] == 0xC1)
			{
				LPPHEADER_DEFAULT_SUBCODE pData = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
				nSubCode = pData->SubCode;
			}
			else
			{
				LPPHEADER_DEFAULT_SUBCODE_WORD pData = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer;
				nSubCode = pData->SubCode;
			}
			switch(nSubCode)
			{
			case 0x04:
				ReceivePCRoomPointInfo(ReceiveBuffer);
				break;
#ifndef KJH_DEL_PC_ROOM_SYSTEM		// #ifndef
			case 0x05:
				ReceivePCRoomPointUse(ReceiveBuffer);
				break;
			case 0x06:
				ReceivePCRoomPointShopOpen(ReceiveBuffer);
				break;
#endif // KJH_DEL_PC_ROOM_SYSTEM
			}
		}
		break;
#endif	// ADD_PCROOM_POINT_SYSTEM
	case 0xBF:
		{
			int subcode;
			if( ReceiveBuffer[0] == 0xC1 )
			{
				LPPHEADER_DEFAULT_SUBCODE Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
				subcode = Data->SubCode;
			}
			else
			{
				LPPHEADER_DEFAULT_SUBCODE_WORD Data = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer;
				subcode = Data->SubCode;
			}
					
			switch(subcode)
			{
			case 0x00:
				ReceiveCursedTempleEnterResult( ReceiveBuffer );
				break;
			case 0x01:
				ReceiveCursedTempleInfo(ReceiveBuffer);
				break;
			case 0x02:
				ReceiveCursedTempMagicResult( ReceiveBuffer );
				break;
			case 0x03:
				ReceiveCursedTempleEnterInfo( ReceiveBuffer );
				break;
			case 0x04:
				ReceiveCursedTempleGameResult( ReceiveBuffer );
				break;
			case 0x06:
				ReceiveCursedTempSkillPoint( ReceiveBuffer );
				break;
			case 0x07:
				ReceiveCursedTempSkillEnd( ReceiveBuffer );
			break;
			case 0x08:
				ReceiveCursedTempleHolyItemRelics( ReceiveBuffer );
				break;
			case 0x09:
				ReceiveCursedTempleState( ReceiveBuffer );
				break;
			case 0x0a:
				ReceiveChainMagic( ReceiveBuffer );
				break;
#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
				//행운의 동전 프로토콜
			case 0x0B:
				//등록된 동전갯수
				ReceiveRegistedLuckyCoin(ReceiveBuffer);
				break;
			case 0x0C:
			//등록동전
				ReceiveRegistLuckyCoin(ReceiveBuffer);
				break;
			case 0x0D:
				//동전교환
				ReceiveRequestExChangeLuckyCoin(ReceiveBuffer);
				break;
#endif //KJH_PBG_ADD_SEVEN_EVENT_2008
#ifdef YDG_ADD_DOPPELGANGER_PROTOCOLS
			case 0x0E:	// 도플갱어맵 입장 요청 결과
				ReceiveEnterDoppelGangerEvent(ReceiveBuffer);
				break;
			case 0x0F:	// 도플갱어 선두 몬스터 위치
				ReceiveDoppelGangerMonsterPosition(ReceiveBuffer);
				break;
			case 0x10:	// 도플갱어의 상태 전송
				ReceiveDoppelGangerState(ReceiveBuffer);
				break;
			case 0x11:	// 도플갱어의 아이스워커의 상태 전송
				ReceiveDoppelGangerIcewalkerState(ReceiveBuffer);
				break;
			case 0x12:	// 도플갱어의 진행 상태 / 시간 정보 / 파티 위치정보
				ReceiveDoppelGangerTimePartyState(ReceiveBuffer);
				break;
			case 0x13:	// 도플갱어의 결과
				ReceiveDoppelGangerResult(ReceiveBuffer);
				break;
			case 0x14:	// 도플갱어의 결과
				ReceiveDoppelGangerMonsterGoal(ReceiveBuffer);
				break;
#endif	// YDG_ADD_DOPPELGANGER_PROTOCOLS
#ifdef PBG_ADD_SECRETBUFF
			case 0x15:	// 피로도 퍼센트
				ReceiveFatigueTime(ReceiveBuffer);
				break;
#endif //PBG_ADD_SECRETBUFF
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
			case 0x20:
				ReceiveEquippingInventoryItem(ReceiveBuffer);
				break;
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
				}
			}
			break;
				
#if defined PSW_CHARACTER_CARD || defined PBG_ADD_CHARACTERCARD
			case 0xDE:
				{
					int subcode = 0;
					
					if( ReceiveBuffer[0] == 0xC1 ) {
						LPPHEADER_DEFAULT_SUBCODE Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
						subcode = Data->SubCode;
					}
					else {
						LPPHEADER_DEFAULT_SUBCODE_WORD Data = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer;
						subcode = Data->SubCode;
					}
					
					switch( subcode )
					{
#ifdef PBG_ADD_CHARACTERCARD
					case 0x00: ReceiveCharacterCard_New( ReceiveBuffer );	//소환/마검/다크
						break;
#else //PBG_ADD_CHARACTERCARD												//이하 해외 전용
					case 0x00: ReceiveCharacterCard( ReceiveBuffer );			break;
					case 0x01: ReceiveBuyCharacterCard( ReceiveBuffer );		break;
#endif //PBG_ADD_CHARACTERCARD
					}
				}
				break;
#endif //defined PSW_CHARACTER_CARD || defined PBG_ADD_CHARACTERCARD
				
#ifdef NEW_USER_INTERFACE_SERVERMESSAGE
			case 0xF5:
				{
					int subcode = 0;
					
					if( ReceiveBuffer[0] == 0xC1 )
					{
						LPPHEADER_DEFAULT_SUBCODE Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
						subcode = Data->SubCode;
					}
					else
					{
						LPPHEADER_DEFAULT_SUBCODE_WORD Data = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer;
						subcode = Data->SubCode;
					}
					
					switch( subcode )
					{
//#if SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE
//					case 0xA1: RecevieCashShopInOut( ReceiveBuffer );			break;
//#else	// SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE
					case 0x02:
						{
							RecevieCashShopInOut( ReceiveBuffer );
						}
						break;
//#endif	// SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE
					case 0x04: 
						{
							RecevieCashShopCashPoint( ReceiveBuffer );
						}
						break;
					case 0x06:
						{
							RecevieCashShopItemlist( ReceiveBuffer );
						}
						break;
					case 0x08: RecevieCashShopItemPurchase( ReceiveBuffer );	break;
					}
				}
				break;
#endif //NEW_USER_INTERFACE_SERVERMESSAGE
#ifdef KJH_PBG_ADD_INGAMESHOP_SYSTEM
		case 0xD2:		// 인게임샵
			{
				PBMSG_HEADER2* Data = (PBMSG_HEADER2*)ReceiveBuffer;
				switch(Data->m_bySubCode)
				{
				case 0x01:
					ReceiveIGS_CashPoint(ReceiveBuffer);			// 캐시 포인트 결과
					break;
				case 0x02:
					ReceiveIGS_ShopOpenResult(ReceiveBuffer);		// 샵 Open/Close 결과
					break;
				case 0x03:
					ReceiveIGS_BuyItem(ReceiveBuffer);				// 아이템 구매결과	
					break;
				case 0x04:
					ReceiveIGS_SendItemGift(ReceiveBuffer);			// 아이템 선물결과
					break;
				case 0x06:
					ReceiveIGS_StorageItemListCount(ReceiveBuffer);		// 보관함 리스트 카운트 전달
					break;
				case 0x07:
					ReceiveIGS_SendCashGift(ReceiveBuffer);			// 캐시선물 결과
					break;
				case 0x08:
					ReceiveIGS_PossibleBuy(ReceiveBuffer);			// 아이템이 구매/가능한지 결과
					break;
				case 0x09:
					ReceiveIGS_LeftCountItem(ReceiveBuffer);		// 상품잔여 조회갯수 결과
					break;
#ifndef KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT					// #ifndef
				case 0x0A:
					ReceiveIGS_DeleteStorageItem(ReceiveBuffer);	// 아이템 버리기 결과
					break;
#endif // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
				case 0x0B:
					ReceiveIGS_UseStorageItem(ReceiveBuffer);		// 보관함 아이템 사용결과
					break;
				case 0x0C:
					ReceiveIGS_UpdateScript(ReceiveBuffer);			// 스크립트 업데이트
					break;
				case 0x0D:
					ReceiveIGS_StorageItemList(ReceiveBuffer);		// 보관함 리스트 전달
					break;
				case 0x0E:
					ReceiveIGS_StorageGiftItemList(ReceiveBuffer);	// 선물함 리스트 전달
					break;
				case 0x13:
					ReceiveIGS_EventItemlistCnt(ReceiveBuffer);		// 이벤트 아이템 리스트 카운트
					break;
				case 0x14:
					ReceiveIGS_EventItemlist(ReceiveBuffer);		// 이벤트 아이템 리스트
					break;
				case 0x15:
					ReceiveIGS_UpdateBanner(ReceiveBuffer);			// 배너 버젼 업데이트
					break;
#ifdef KJH_ADD_PERIOD_ITEM_SYSTEM
				case 0x11:
					ReceivePeriodItemListCount(ReceiveBuffer);		// 기간제 아이템 리스트 카운트
					break;	
				case 0x12:
					ReceivePeriodItemList(ReceiveBuffer);			// 기간제 아이템 리스트
					break;
#endif // KJH_ADD_PERIOD_ITEM_SYSTEM
				}
			}break;

#endif // KJH_PBG_ADD_INGAMESHOP_SYSTEM
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
		case 0x4A:
			ReceiveStraightAttack(ReceiveBuffer, Size, bEncrypted);
			break;
		case 0x4B:
			ReceiveDarkside(ReceiveBuffer);
			break;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
		default:
			{
				//AddDebugText(ReceiveBuffer,Size);
			}
			
			break;
	}
	
	return ( TRUE);
}

void TranslateChattingProtocol(DWORD dwWindowUIID, int HeadCode, BYTE *ReceiveBuffer, int Size, BOOL bEcrypted)
{
	switch( HeadCode )
	{
	case 0x00:	// 방 접속 결과 받기
		ReceiveChatRoomConnectResult(dwWindowUIID, ReceiveBuffer);
		break;
	case 0x01:	// 다른 사람 상태 변화
		ReceiveChatRoomUserStateChange(dwWindowUIID, ReceiveBuffer);
		break;
	case 0x02:	// 유저리스트 받기
		ReceiveChatRoomUserList(dwWindowUIID, ReceiveBuffer);
		break;
	case 0x04:	// 채팅 대화 글 받기
		ReceiveChatRoomChatText(dwWindowUIID, ReceiveBuffer);
		break;
	case 0x0D:	// 공지사항
		ReceiveChatRoomNoticeText(dwWindowUIID, ReceiveBuffer);
		break;
	default:
		break;
	}
}

bool CheckExceptionBuff( eBuffState buff, OBJECT* o, bool iserase )
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!! 경고 !!!!!!!!!!!!!!!!!!!!!!!!!!! 
	// !!!!!!!!!!!!!!!!!!!!!!!!!!! 경고 !!!!!!!!!!!!!!!!!!!!!!!!!!! 
	
	// return false를 할 경우 buff는 등록 시키지 않는다.
	// 근대 앞으로는 절대로 이렇게 만들지 말자.
	// 이 부분들은 예전에 버프를 아껴서 쓰기 위함이다...절대로 여기에 추가 시켜서 작업을 하지 말자.
	// 추가를 안 하더라도 충분히 다른 방법으로 가능하다.
	
	// !!!!!!!!!!!!!!!!!!!!!!!!!!! 경고 !!!!!!!!!!!!!!!!!!!!!!!!!!! 
	// !!!!!!!!!!!!!!!!!!!!!!!!!!! 경고 !!!!!!!!!!!!!!!!!!!!!!!!!!! 
	
	if( iserase )
	{
		switch(buff)
		{
		case eBuff_SoulPotion:
			{
				if(o->Type >= MODEL_CRYWOLF_ALTAR1 && o->Type <= MODEL_CRYWOLF_ALTAR5)
				{
					g_CharacterUnRegisterBuff(o, eBuff_CrywolfAltarEnable );
					return false;
				}
			}
			break;
			
		case eBuff_RemovalMagic:
			{
				std::list<eBuffState> bufflist;

				//debuff
				bufflist.push_back( eDeBuff_Poison ); 
				bufflist.push_back( eDeBuff_Freeze );
				bufflist.push_back( eDeBuff_Harden ); 
				bufflist.push_back( eDeBuff_Defense );
				bufflist.push_back( eDeBuff_Stun );
				bufflist.push_back( eDeBuff_Sleep );
#ifdef CSK_ADD_SKILL_BLOWOFDESTRUCTION
				bufflist.push_back( eDeBuff_BlowOfDestruction );
#endif // CSK_ADD_SKILL_BLOWOFDESTRUCTION

				//buff
				bufflist.push_back( eBuff_HpRecovery ); bufflist.push_back( eBuff_Attack );
				//bufflist.push_back( eBuff_Life ); bufflist.push_back( eBuff_Attack );
				bufflist.push_back( eBuff_Defense ); bufflist.push_back( eBuff_AddAG );
				bufflist.push_back( eBuff_Cloaking ); bufflist.push_back( eBuff_AddSkill );
				bufflist.push_back( eBuff_PhysDefense ); bufflist.push_back( eBuff_AddCriticalDamage );
				bufflist.push_back( eBuff_CrywolfAltarOccufied );
				
				g_CharacterUnRegisterBuffList(o, bufflist );
			}
			break;
		}
		
		return true;
	}
	else
	{
		switch(buff)
		{
		case eBuff_CastleRegimentAttack3:
			{
				g_CharacterUnRegisterBuff(o, eBuff_CastleRegimentDefense);
			}	
			break;
			
		case eBuff_SoulPotion:
			{
				// 제단일 경우만..
				if(o->Type >= MODEL_CRYWOLF_ALTAR1 && o->Type <= MODEL_CRYWOLF_ALTAR5)
				{
					// MVP일때  제단 계약 가능 상태
					g_CharacterRegisterBuff(o, eBuff_CrywolfAltarEnable);
					return false;
				}
			}
			break;
			
		case eBuff_CastleGateIsOpen:
			{
				// 제단일 경우만..
				if(o->Type >= MODEL_CRYWOLF_ALTAR1 && o->Type <= MODEL_CRYWOLF_ALTAR5)
				{
					// MVP일때  제단 계약 불능 상태
					g_CharacterRegisterBuff(o, eBuff_CrywolfAltarDisable);
					return false;
				}
			}
			break;
			
		case eBuff_CastleRegimentDefense:
			{
				// 제단일 경우만..
				if(o->Type >= MODEL_CRYWOLF_ALTAR1 && o->Type <= MODEL_CRYWOLF_ALTAR5)
				{
					// MVP일때  제단 계약 된 상태
					g_CharacterRegisterBuff(o, eBuff_CrywolfAltarContracted);
					return false;
				}
				else
				{
					g_TokenCharacterBuff(o, eBuff_CastleRegimentDefense);
					return false;
				}
			}
			break;
			
		case eBuff_CastleRegimentAttack1:
			{
				// 제단일 경우만..
				if(o->Type >= MODEL_CRYWOLF_ALTAR1 && o->Type <= MODEL_CRYWOLF_ALTAR5)
				{
					// MVP일때  제단 계약 시도 상태
					g_CharacterRegisterBuff(o, eBuff_CrywolfAltarAttempt );
					return false;
				}
				else if( g_isCharacterBuff( o, eBuff_CastleRegimentDefense ) )
				{
					g_CharacterUnRegisterBuff(o, eBuff_CastleRegimentDefense);
				}
			}
			break;
			
		case eBuff_RemovalMagic:
			{
				// 제단일 경우만..
				if(o->Type >= MODEL_CRYWOLF_ALTAR1 && o->Type <= MODEL_CRYWOLF_ALTAR5)
				{
					// MVP일때  제단 점령 당한 상태
					g_CharacterRegisterBuff(o, eBuff_CrywolfAltarOccufied);
					return false;
				}
			}
			break;
			
		case eBuff_CastleRegimentAttack2:
			{
				if( g_isCharacterBuff( o, eBuff_CastleRegimentDefense ) )
					g_CharacterUnRegisterBuff(o, eBuff_CastleRegimentDefense);
			}
			break;
		}
		
		return true;
	}
}

// 논리적인 이펙트 처리는 여기서 다 해준다. 버프가 시작할때
void InsertBuffLogicalEffect( eBuffState buff, OBJECT* o, const int bufftime )
{
	if(o && o == &Hero->Object)
	{
		switch( buff )
		{
		case eBuff_Hellowin1:
		case eBuff_Hellowin2:
		case eBuff_Hellowin3:
		case eBuff_Hellowin4:
		case eBuff_Hellowin5:
			{
				// 버프 시간 설정
				g_RegisterBuffTime(buff, bufftime);
				
				// 버프 정보 캐릭터에 갱신
				if( buff == eBuff_Hellowin1 ) 
				{
					CharacterMachine->CalculateAttackSpeed();
				}
				if( buff == eBuff_Hellowin2 ) 
				{
					CharacterMachine->CalculateDamage();
				}
				if( buff == eBuff_Hellowin3 ) 
				{
					CharacterMachine->CalculateDefense();
				}
			}
			break;
		case eBuff_PcRoomSeal1:
		case eBuff_PcRoomSeal2:
		case eBuff_PcRoomSeal3:
			{
				// 버프 시간 설정
				g_RegisterBuffTime(buff, bufftime);
			}
			break;
		case eBuff_Seal1:
		case eBuff_Seal2:
		case eBuff_Seal3:
		case eBuff_Seal4:
#ifdef PSW_ADD_PC4_SEALITEM
		case eBuff_Seal_HpRecovery:
		case eBuff_Seal_MpRecovery:
#endif //PSW_ADD_PC4_SEALITEM
#ifdef ASG_ADD_CS6_GUARD_CHARM
		case eBuff_GuardCharm:
#endif	// ASG_ADD_CS6_GUARD_CHARM
#ifdef ASG_ADD_CS6_ITEM_GUARD_CHARM
		case eBuff_ItemGuardCharm:
#endif	// ASG_ADD_CS6_ITEM_GUARD_CHARM
#ifdef ASG_ADD_CS6_ASCENSION_SEAL_MASTER
		case eBuff_AscensionSealMaster:
#endif	// ASG_ADD_CS6_ASCENSION_SEAL_MASTER
#ifdef ASG_ADD_CS6_WEALTH_SEAL_MASTER
		case eBuff_WealthSealMaster:
#endif	// ASG_ADD_CS6_WEALTH_SEAL_MASTER
#ifdef LDK_ADD_INGAMESHOP_NEW_WEALTH_SEAL
		case eBuff_NewWealthSeal:
#endif //LDK_ADD_INGAMESHOP_NEW_WEALTH_SEAL
#ifdef YDG_ADD_CS7_MAX_AG_AURA
		case eBuff_AG_Addition:
#endif	// YDG_ADD_CS7_MAX_AG_AURA
#ifdef YDG_ADD_CS7_MAX_SD_AURA
		case eBuff_SD_Addition:
#endif	// YDG_ADD_CS7_MAX_SD_AURA
#ifdef YDG_ADD_CS7_PARTY_EXP_BONUS_ITEM
		case eBuff_PartyExpBonus:
#endif	// YDG_ADD_CS7_PARTY_EXP_BONUS_ITEM
			{
				// 버프 시간 설정
				g_RegisterBuffTime(buff, bufftime);
			}
			break;
		case eBuff_EliteScroll1:
		case eBuff_EliteScroll2:
		case eBuff_EliteScroll3:
		case eBuff_EliteScroll4:
		case eBuff_EliteScroll5:
		case eBuff_EliteScroll6:
#ifdef PSW_ADD_PC4_SCROLLITEM
		case eBuff_Scroll_Battle:
		case eBuff_Scroll_Strengthen:
#endif //PSW_ADD_PC4_SCROLLITEM
#ifdef YDG_ADD_HEALING_SCROLL
		case eBuff_Scroll_Healing:	// 치유의 스크롤
#endif	// YDG_ADD_HEALING_SCROLL
			{
				// 버프 시간 설정
				g_RegisterBuffTime(buff, bufftime);
				
				if( buff == eBuff_EliteScroll1 ) { //  공격속도 계산.
					CharacterMachine->CalculateAttackSpeed();
				}
				else if( buff == eBuff_EliteScroll2 ) { // 방어력 계산
					CharacterMachine->CalculateDefense();
				}
				else if( buff == eBuff_EliteScroll3 ) { // 공격력 계산
					CharacterMachine->CalculateDamage();
				}
				else if( buff == eBuff_EliteScroll4 ) { // 마력 계산
					CharacterMachine->CalculateMagicDamage();
				}
			}
			break;
		case eBuff_SecretPotion1:
		case eBuff_SecretPotion2:
		case eBuff_SecretPotion3:
		case eBuff_SecretPotion4:
		case eBuff_SecretPotion5:
			{
				// 버프 시간 설정
				g_RegisterBuffTime(buff, bufftime);
				
				CharacterMachine->CalculateAll();
			}
			break;
			
#ifdef CSK_EVENT_CHERRYBLOSSOM
		case eBuff_CherryBlossom_Liguor:
		case eBuff_CherryBlossom_RiceCake:
		case eBuff_CherryBlossom_Petal:
			{
				// 버프 시간 설정
				g_RegisterBuffTime(buff, bufftime);
				
				if( buff == eBuff_CherryBlossom_Petal ) { // 공격력 계산
					CharacterMachine->CalculateDamage();
				}
			}
			break;
#endif //CSK_EVENT_CHERRYBLOSSOM
#ifdef KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
		case eBuff_SwellOfMagicPower:
			{
				// 버프 시간 설정
				g_RegisterBuffTime(buff, bufftime);
				
				if( buff == eBuff_SwellOfMagicPower ) {		// 마력 계산
					CharacterMachine->CalculateMagicDamage();
				}
			}break;
#endif // KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
#ifdef PBG_ADD_SANTABUFF
			//산타버프관련.(해외 크리스마스 이벤트)
		case eBuff_BlessingOfXmax:
		case eBuff_CureOfSanta:
		case eBuff_SafeGuardOfSanta:
		case eBuff_StrengthOfSanta:
		case eBuff_DefenseOfSanta:
		case eBuff_QuickOfSanta:
		case eBuff_LuckOfSanta:
			{
				//버프시간설정.(30분)
				g_RegisterBuffTime(buff, bufftime);

#ifdef KJH_FIX_TALK_SANTATOWN_NPC
				char _Temp[256] = {0,};
#else // KJH_FIX_TALK_SANTATOWN_NPC
				char _Temp[32] = {0,};
#endif // KJH_FIX_TALK_SANTATOWN_NPC
				//버프 공격력+방어력 향상
				if(buff == eBuff_BlessingOfXmax)
				{
					// 2591 "공격력과 방어력이 향상 되었습니다."
					g_pChatListBox->AddText("", GlobalText[2591], SEASON3B::TYPE_SYSTEM_MESSAGE);

					//공격력향상
					CharacterMachine->CalculateDamage();
					//방어력향상
					CharacterMachine->CalculateDefense();
				}
				else if(buff == eBuff_StrengthOfSanta)
				{
					// 2594 "공격력이 %d만큼 상승 되었습니다."
					sprintf(_Temp, GlobalText[2594], 30);
					g_pChatListBox->AddText("", _Temp, SEASON3B::TYPE_SYSTEM_MESSAGE);

					//공격력 향상+30
					CharacterMachine->CalculateDamage();
				}
				else if(buff == eBuff_DefenseOfSanta)
				{
					// 2595 "방어력이 %d만큼 상승 되었습니다."
					sprintf(_Temp, GlobalText[2595], 100);
					g_pChatListBox->AddText("", _Temp, SEASON3B::TYPE_SYSTEM_MESSAGE);

					//방어력 향상+100
					CharacterMachine->CalculateDefense();
				}
				else if(buff == eBuff_QuickOfSanta)
				{
					// 2598 "공격속도가 %d만큼 상승 되었습니다."
					sprintf(_Temp, GlobalText[2598], 15);
					g_pChatListBox->AddText("", _Temp, SEASON3B::TYPE_SYSTEM_MESSAGE);

					//공속증+15
					CharacterMachine->CalculateAttackSpeed();
				}
				else if(buff == eBuff_LuckOfSanta)
				{
					// 2599 "AG회복속도가 %d만큼 증가 되었습니다."
					sprintf(_Temp, GlobalText[2599], 10);
					g_pChatListBox->AddText("", _Temp, SEASON3B::TYPE_SYSTEM_MESSAGE);

					//AG회복속도증 서버에서 받아서 처리한다.
				}
				else if(buff == eBuff_CureOfSanta)
				{
					// 2592 "최대 생명이 %d만큼 향상 되었습니다."
					sprintf(_Temp, GlobalText[2592], 500);
					g_pChatListBox->AddText("", _Temp, SEASON3B::TYPE_SYSTEM_MESSAGE);
				}
				else if(buff == eBuff_SafeGuardOfSanta)
				{
					// 2593 "최대 마나가 %d 만큼 향상 되었습니다."
					sprintf(_Temp, GlobalText[2593], 500);
					g_pChatListBox->AddText("", _Temp, SEASON3B::TYPE_SYSTEM_MESSAGE);
				}
			}
			break;
#endif //PBG_ADD_SANTABUFF
#ifdef YDG_ADD_NEW_DUEL_WATCH_BUFF
		case eBuff_DuelWatch:	// 관전 버프
			{
				// UI를 모두 닫는다.
				g_pNewUISystem->HideAll();
				// 메인프레임을 교체한다.
				g_pNewUISystem->Hide(SEASON3B::INTERFACE_MAINFRAME);
				g_pNewUISystem->Hide(SEASON3B::INTERFACE_BUFF_WINDOW);
				g_pNewUISystem->Show(SEASON3B::INTERFACE_DUELWATCH_MAINFRAME);
				g_pNewUISystem->Show(SEASON3B::INTERFACE_DUELWATCH_USERLIST);
			}
			break;
#endif	// YDG_ADD_NEW_DUEL_WATCH_BUFF
#ifdef PBG_ADD_HONOROFGLADIATORBUFFTIME
		case eBuff_HonorOfGladiator:
			{
				g_RegisterBuffTime(buff, bufftime);
			}
			break;
#endif //PBG_ADD_HONOROFGLADIATORBUFFTIME
#ifdef LDS_ADD_PCROOM_ITEM_JPN_6TH
		case eBuff_StrongCharm:			// 강함의 인장 (PC방 아이템)
			{
				g_RegisterBuffTime(buff, bufftime);

				// 2591 "공격력과 방어력이 향상 되었습니다." // 2591, 2472
				g_pChatListBox->AddText("", GlobalText[2591], SEASON3B::TYPE_SYSTEM_MESSAGE);
				
				//공격력향상
				CharacterMachine->CalculateDamage();
				//방어력향상
				CharacterMachine->CalculateDefense();
			}
			break;
#endif // LDS_ADD_PCROOM_ITEM_JPN_6TH
#ifdef PBG_ADD_SECRETBUFF
		case eBuff_Vitality_Lowest:				// 활력의 비약(최하급/하급/중급/상급)
		case eBuff_Vitality_Low:
		case eBuff_Vitality_Middle:
		case eBuff_Vitality_High:
		case eDeBuff_Fatigue_FirstPenalty:		// 피로도 디버프1
		case eDeBuff_Fatigue_SecondPenalty:		// 피로도 디버프2
			{
				// 활력의 버프가 설정되면 피로도 시간을 멈춘다
				//if(buff != eDeBuff_Fatigue_FirstPenalty && buff != eDeBuff_Fatigue_SecondPenalty)
				//	g_FatigueTimeSystem->SetIsVitalityBuff(true);

				g_RegisterBuffTime(buff, bufftime);
			}
			break;
#endif //PBG_ADD_SECRETBUFF
#ifdef PBG_ADD_PKSYSTEM_INGAMESHOP
		case eDeBuff_MoveCommandWin:
			{
				// 혼합유료화 섭일 경우 무법자 1단계(5분은 불가)와 상관없이 모두 이동창 사용가능
				g_PKSystem->SetMoveBuffState(g_isCharacterBuff((&Hero->Object), eDeBuff_MoveCommandWin));
			}
			break;
#endif //PBG_ADD_PKSYSTEM_INGAMESHOP
		}
	}
	// Hero가 아닌 경우만가 있으면 else를 달아 준다. 있으면 안 된다..-=-;;;
}

// 논리적인 이펙트 처리는 여기서 다 해준다 버프가 종료 될때
void ClearBuffLogicalEffect( eBuffState buff, OBJECT* o )
{
	if(o && o == &Hero->Object)
	{
		switch( buff )
		{
		case eBuff_Hellowin1:
		case eBuff_Hellowin2:
		case eBuff_Hellowin3:
		case eBuff_Hellowin4:
		case eBuff_Hellowin5:
			{
				// 버프 시간 설정
				g_UnRegisterBuffTime( buff );
				
				// 버프 정보 캐릭터에 갱신
				if( buff == eBuff_Hellowin1 )
				{
					CharacterMachine->CalculateAttackSpeed();
				}
				else if( buff == eBuff_Hellowin2 )
				{
					int iBaseClass = GetBaseClass(Hero->Class);
					
					if (iBaseClass == CLASS_WIZARD || iBaseClass == CLASS_DARK || iBaseClass == CLASS_SUMMONER)
					{
						CharacterMachine->CalculateMagicDamage();
					}
					else
					{
						CharacterMachine->CalculateDamage();
					}
				}
				else if( buff == eBuff_Hellowin3 )
				{
					CharacterMachine->CalculateDefense();
				}
			}
			break;
		case eBuff_PcRoomSeal1:
		case eBuff_PcRoomSeal2:
		case eBuff_PcRoomSeal3:
			{
				// 버프 시간 설정
				g_UnRegisterBuffTime( buff );
			}
			break;
		case eBuff_Seal1:
		case eBuff_Seal2:
		case eBuff_Seal3:
		case eBuff_Seal4:
#ifdef PSW_ADD_PC4_SEALITEM
		case eBuff_Seal_HpRecovery:
		case eBuff_Seal_MpRecovery:
#endif //PSW_ADD_PC4_SEALITEM
#ifdef ASG_ADD_CS6_GUARD_CHARM
		case eBuff_GuardCharm:
#endif	// ASG_ADD_CS6_GUARD_CHARM
#ifdef ASG_ADD_CS6_ITEM_GUARD_CHARM
		case eBuff_ItemGuardCharm:
#endif	// ASG_ADD_CS6_ITEM_GUARD_CHARM
#ifdef ASG_ADD_CS6_ASCENSION_SEAL_MASTER
		case eBuff_AscensionSealMaster:
#endif	// ASG_ADD_CS6_ASCENSION_SEAL_MASTER
#ifdef ASG_ADD_CS6_WEALTH_SEAL_MASTER
		case eBuff_WealthSealMaster:
#endif	// ASG_ADD_CS6_WEALTH_SEAL_MASTER
#ifdef LDK_ADD_INGAMESHOP_NEW_WEALTH_SEAL
		case eBuff_NewWealthSeal:
#endif //LDK_ADD_INGAMESHOP_NEW_WEALTH_SEAL
#ifdef YDG_ADD_CS7_MAX_AG_AURA
		case eBuff_AG_Addition:
#endif	// YDG_ADD_CS7_MAX_AG_AURA
#ifdef YDG_ADD_CS7_MAX_SD_AURA
		case eBuff_SD_Addition:
#endif	// YDG_ADD_CS7_MAX_SD_AURA
#ifdef YDG_ADD_CS7_PARTY_EXP_BONUS_ITEM
		case eBuff_PartyExpBonus:
#endif	// YDG_ADD_CS7_PARTY_EXP_BONUS_ITEM
			{
				// 버프 시간 설정
				g_UnRegisterBuffTime( buff );
			}
			break;
		case eBuff_EliteScroll1:
		case eBuff_EliteScroll2:
		case eBuff_EliteScroll3:
		case eBuff_EliteScroll4:
		case eBuff_EliteScroll5:
		case eBuff_EliteScroll6:
#ifdef PSW_ADD_PC4_SCROLLITEM
		case eBuff_Scroll_Battle:
		case eBuff_Scroll_Strengthen:
#endif //PSW_ADD_PC4_SCROLLITEM
#ifdef YDG_ADD_HEALING_SCROLL
		case eBuff_Scroll_Healing:	// 치유의 스크롤
#endif	// YDG_ADD_HEALING_SCROLL
			{
				// 버프 시간 설정
				g_UnRegisterBuffTime( buff );
				
				if( buff == eBuff_EliteScroll1 ) { //  공격속도 계산.
					CharacterMachine->CalculateAttackSpeed();
				}
				else if( buff == eBuff_EliteScroll2 ) { // 방어력 계산
					CharacterMachine->CalculateDefense();
				}
				else if( buff == eBuff_EliteScroll3 ) { // 공격력 계산
					CharacterMachine->CalculateDamage();
				}
				else if( buff == eBuff_EliteScroll4 ) { // 마력 계산
					CharacterMachine->CalculateMagicDamage();
				}
			}
			break;
		case eBuff_SecretPotion1:
		case eBuff_SecretPotion2:
		case eBuff_SecretPotion3:
		case eBuff_SecretPotion4:
		case eBuff_SecretPotion5:
			{
				// 버프 시간 설정
				g_UnRegisterBuffTime( buff );
				
				CharacterMachine->CalculateAll();
			}
			break;
#ifdef CSK_EVENT_CHERRYBLOSSOM
		case eBuff_CherryBlossom_Liguor:
		case eBuff_CherryBlossom_RiceCake:
		case eBuff_CherryBlossom_Petal:
			{
				// 버프 시간 설정
				g_UnRegisterBuffTime( buff );
				
				if( buff == eBuff_CherryBlossom_Petal ) { // 공격력 계산
					CharacterMachine->CalculateDamage();
				}
			}
			break;
#endif //CSK_EVENT_CHERRYBLOSSOM
#ifdef KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
		case eBuff_SwellOfMagicPower:
			{
				// 버프 시간 설정
				g_UnRegisterBuffTime( buff );
				
				CharacterMachine->CalculateMagicDamage();
			}break;
#endif // KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
#ifdef PBG_ADD_SANTABUFF
		//산타버프관련.(해외 크리스마스 이벤트)
		case eBuff_BlessingOfXmax:
		case eBuff_CureOfSanta:
		case eBuff_SafeGuardOfSanta:
		case eBuff_StrengthOfSanta:
		case eBuff_DefenseOfSanta:
		case eBuff_QuickOfSanta:
		case eBuff_LuckOfSanta:
			{
				//버프시간설정제거.
				g_UnRegisterBuffTime( buff );
				
				//버프정보 캐릭터 갱신.
				if(buff == eBuff_BlessingOfXmax)
				{
					CharacterMachine->CalculateDamage();
					CharacterMachine->CalculateDefense();
				}
				else if(buff == eBuff_StrengthOfSanta)
				{
					CharacterMachine->CalculateDamage();
				}
				else if(buff == eBuff_DefenseOfSanta)
				{
					CharacterMachine->CalculateDefense();
				}
				else if(buff == eBuff_QuickOfSanta)
				{
					CharacterMachine->CalculateAttackSpeed();
				}
			}
			break;
#endif //PBG_ADD_SANTABUFF
#ifdef YDG_ADD_NEW_DUEL_WATCH_BUFF
		case eBuff_DuelWatch:	// 관전 버프
			{
				// 메인프레임을 교체한다.
				g_pNewUISystem->Hide(SEASON3B::INTERFACE_DUELWATCH_MAINFRAME);
				g_pNewUISystem->Hide(SEASON3B::INTERFACE_DUELWATCH_USERLIST);
				g_pNewUISystem->Show(SEASON3B::INTERFACE_MAINFRAME);
				g_pNewUISystem->Show(SEASON3B::INTERFACE_BUFF_WINDOW);
			}	
			break;
#endif	// YDG_ADD_NEW_DUEL_WATCH_BUFF
#ifdef PBG_ADD_HONOROFGLADIATORBUFFTIME
		case eBuff_HonorOfGladiator:
			{
				g_UnRegisterBuffTime( buff );
			}
			break;
#endif //PBG_ADD_HONOROFGLADIATORBUFFTIME
#ifdef LDS_ADD_PCROOM_ITEM_JPN_6TH
		case eBuff_StrongCharm:					// 강함의 인장 (PC방 아이템)
			{
				g_UnRegisterBuffTime( buff );

				// 공격력과 방어력을 계산하여준다. 
				CharacterMachine->CalculateDamage();
				CharacterMachine->CalculateDefense();
			}
			break;
#endif // LDS_ADD_PCROOM_ITEM_JPN_6TH
#ifdef PBG_ADD_SECRETBUFF
		case eBuff_Vitality_Lowest:				// 활력의 비약(최하급/하급/중급/상급)
		case eBuff_Vitality_Low:
		case eBuff_Vitality_Middle:
		case eBuff_Vitality_High:
		case eDeBuff_Fatigue_FirstPenalty:		// 피로도 디버프1
		case eDeBuff_Fatigue_SecondPenalty:		// 피로도 디버프2
			{
				// 활력의 버프가 끝나면 피로도 재설정
				//if(buff != eDeBuff_Fatigue_FirstPenalty && buff != eDeBuff_Fatigue_SecondPenalty)
				//	g_FatigueTimeSystem->SetIsVitalityBuff(false);

				g_UnRegisterBuffTime( buff );
			}
			break;
#endif //PBG_ADD_SECRETBUFF
#ifdef PBG_ADD_PKSYSTEM_INGAMESHOP
		case eDeBuff_MoveCommandWin:
			{
				// 혼합유료화 섭일 경우 무법자 1단계(5분은 불가)와 상관없이 모두 이동창 사용가능
				g_PKSystem->SetMoveBuffState(g_isCharacterBuff((&Hero->Object), eDeBuff_MoveCommandWin));
			}
			break;
#endif //PBG_ADD_PKSYSTEM_INGAMESHOP
		}
	}
	// Hero가 아닌 경우만가 있으면 else를 달아 준다. 있으면 안 된다..-=-;;;
}

void InsertBuffPhysicalEffect( eBuffState buff, OBJECT* o )
{
	switch(buff)
	{
	case eBuff_CursedTempleProdection:
		{
			// 플레이어 한테만 생기는 이펙트
			if( o->Type == MODEL_PLAYER )
			{
				DeleteEffect( MODEL_CURSEDTEMPLE_PRODECTION_SKILL, o );
				DeleteEffect( MODEL_SHIELD_CRASH, o );
				DeleteEffect( BITMAP_SHOCK_WAVE, o );
				vec3_t  Light;
				Vector(0.3f, 0.3f, 0.8f, Light);
				CreateEffect( MODEL_CURSEDTEMPLE_PRODECTION_SKILL, o->Position, o->Angle, Light, 0, o );
				CreateEffect( MODEL_SHIELD_CRASH, o->Position, o->Angle, Light, 1, o );				
				CreateEffect ( BITMAP_SHOCK_WAVE, o->Position, o->Angle, Light, 10, o );
			}	
		}
		break;
		
	case eBuff_PhysDefense:
		{
			// 플레이어 한테만 생기는 이펙트
			if( o->Type == MODEL_PLAYER )
			{
				PlayBuffer(SOUND_SOULBARRIER);
				DeleteJoint( MODEL_SPEARSKILL, o,0);
				for ( int j = 0; j < 5; ++j)
				{
					CreateJoint( MODEL_SPEARSKILL, o->Position, o->Position, o->Angle, 0, o, 50.0f);
				}
			}
		}
		break;
		
	case eBuff_AddAG:
		{
			DeleteEffect ( BITMAP_LIGHT, o, 2 );
			CreateEffect ( BITMAP_LIGHT, o->Position, o->Angle, o->Light, 2, o );
		}
		break;
		
	case eBuff_HpRecovery:
		//case eBuff_Life:
		{
			DeleteEffect ( BITMAP_LIGHT, o, 1 );
			CreateEffect ( BITMAP_LIGHT, o->Position, o->Angle, o->Light, 1, o );
		}
		break;
	case eDeBuff_Harden:
		{
			vec3_t Angle;
			VectorCopy ( o->Angle, Angle );
			
			DeleteEffect ( MODEL_ICE, o, 1 );
			CreateEffect(MODEL_ICE,o->Position,Angle,o->Light,1,o);
			
			Angle[2] += 180.f;
			CreateEffect(MODEL_ICE,o->Position,Angle,o->Light,2,o);
		}
		break;
		
	case eDeBuff_Defense:
		{
			vec3_t Light = { 1.0f, 1.f, 1.f};
			DeleteEffect(BITMAP_SKULL,o,0);
			CreateEffect(BITMAP_SKULL,o->Position,o->Angle,Light,0,o);
			
			PlayBuffer(SOUND_BLOODATTACK,o);
		}
		break;
		
	case eDeBuff_Stun:
		{
			DeleteEffect(BITMAP_SKULL,o,5);
			
			vec3_t Angle;
			vec3_t Position;
			VectorCopy(o->Position, Position);
			Vector ( 0.f, 0.f, 90.f, Angle );
			CreateJoint ( MODEL_SPEARSKILL, Position, Position, Angle, 8, o, 30.0f );
			Position[2] -= 10.f;
			Vector ( 0.f, 0.f, 180.f, Angle );
			CreateJoint ( MODEL_SPEARSKILL, Position, Position, Angle, 8, o, 30.0f );
			Position[2] -= 10.f;
			Vector ( 0.f, 0.f, 270.f, Angle );
			CreateJoint ( MODEL_SPEARSKILL, Position, Position, Angle, 8, o, 30.0f );
		}
		break;
	case eDeBuff_NeilDOT:
		{
			g_SummonSystem.CreateDamageOfTimeEffect(AT_SKILL_SUMMON_REQUIEM, o);
		}
		break;
	case eDeBuff_SahamuttDOT:
		{
			g_SummonSystem.CreateDamageOfTimeEffect(AT_SKILL_SUMMON_EXPLOSION, o);
		}
		break;
	case eBuff_Thorns:	
		{
			// 타입 43, 44는 버프 전용 이펙트이므로 절대 다른 사람이 사용하지 마세요!!!!!!
			vec3_t vLight;
			Vector(0.9f, 0.6f, 0.1f, vLight);
			CreateJoint(BITMAP_FLARE, o->Position, o->Position, o->Angle, 43, o, 50.f, 0, 0, 0, 0, vLight);
		}
		break;
		
	case eDeBuff_Blind:
		{
			// 3, 4 타입은 버프 전용 이펙트 다른사람은 절대 사용하지 마세요!!!!!
			vec3_t vLight;
			Vector(1.0f, 1.0f, 1.0f, vLight);
			CreateEffect(MODEL_ALICE_BUFFSKILL_EFFECT, o->Position, o->Angle, vLight, 3, o);
		}
		break;
		
	case eDeBuff_Sleep:
		{
			// 3, 4 타입은 버프 전용 이펙트 다른사람은 절대 사용하지 마세요!!!!!
			vec3_t vLight;
			Vector(0.7f, 0.1f, 0.9f, vLight);
			CreateEffect(MODEL_ALICE_BUFFSKILL_EFFECT, o->Position, o->Angle, vLight, 4, o);
		}
		break;

	case eDeBuff_AttackDown:
		{
			vec3_t vLight;
			Vector(1.4f, 0.2f, 0.2f, vLight);
			CreateEffect(BITMAP_SHINY+6, o->Position, o->Angle, vLight, 1, o, -1, 0, 0, 0, 0.5f);
			CreateEffect(BITMAP_PIN_LIGHT, o->Position, o->Angle, vLight, 1, o, -1, 0, 0, 0, 1.f);
		}
		break;
		
	case eDeBuff_DefenseDown:
		{
			vec3_t vLight;
			Vector(0.25f, 1.0f, 0.7f, vLight);
			CreateEffect(BITMAP_SHINY+6, o->Position, o->Angle, vLight, 2, o, -1, 0, 0, 0, 0.5f);
			CreateEffect(BITMAP_PIN_LIGHT, o->Position, o->Angle, vLight, 2, o, -1, 0, 0, 0, 1.f);
		}
		break;
		
#ifdef ASG_ADD_SKILL_BERSERKER
	case eBuff_Berserker:
		{
			vec3_t vLight[2];
			Vector(0.9f, 0.0f, 0.1f, vLight[0]);
			Vector(1.0f, 1.0f, 1.0f, vLight[1]);
			for (int i = 0; i < 4; ++i)
			{
				DeleteEffect(BITMAP_ORORA, o, i);
				CreateEffect(BITMAP_ORORA, o->Position, o->Angle, vLight[0], i, o);
				if (i == 2 || i == 3)
				{
					DeleteEffect(BITMAP_SPARK+2, o, i);
					CreateEffect(BITMAP_SPARK+2, o->Position, o->Angle, vLight[1], i, o);
				}
			}
			DeleteEffect(BITMAP_LIGHT_MARKS, o);
			CreateEffect(BITMAP_LIGHT_MARKS, o->Position, o->Angle, vLight[0], 0, o);
		}
		break;
#endif	// ASG_ADD_SKILL_BERSERKER
		
#ifdef YDG_ADD_SKILL_FLAME_STRIKE
	case eDeBuff_FlameStrikeDamage:
		{
			CreateEffect(BITMAP_FIRE_CURSEDLICH, o->Position, o->Angle, o->Light, 2, o);
		}
		break;
#endif	// YDG_ADD_SKILL_FLAME_STRIKE
#ifdef YDG_ADD_SKILL_GIGANTIC_STORM
	case eDeBuff_GiganticStormDamage:
		{
#ifdef YDG_FIX_GIGANTIC_STORM_OPTIMIZE
			DeleteEffect(BITMAP_JOINT_THUNDER, o, 0);
#endif	// YDG_FIX_GIGANTIC_STORM_OPTIMIZE
			CreateEffect(BITMAP_JOINT_THUNDER, o->Position, o->Angle, o->Light, 0, o);
		}
		break;
#endif	// YDG_ADD_SKILL_GIGANTIC_STORM
#ifdef YDG_ADD_SKILL_LIGHTNING_SHOCK
	case eDeBuff_LightningShockDamage:
		{
			CreateEffect(MODEL_LIGHTNING_SHOCK, o->Position, o->Angle, o->Light, 2, o);
		}
		break;
#endif	// YDG_ADD_SKILL_LIGHTNING_SHOCK
#ifdef CSK_ADD_SKILL_BLOWOFDESTRUCTION
	case eDeBuff_BlowOfDestruction:
		{
			CreateEffect(MODEL_ICE, o->Position, o->Angle, o->Light);
		}
		break;
#endif // CSK_ADD_SKILL_BLOWOFDESTRUCTION
#ifndef KJH_FIX_SWELLOFMAGIC_EFFECT				// #ifndef 정리할때 지워야하는 소스
#ifdef KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
	case eBuff_SwellOfMagicPower:
		{
			// 투명이 아닐때만 이팩트사용
			if( g_isCharacterBuff(o, eBuff_Cloaking) )
				break;

			vec3_t vLight;
			Vector(0.7f, 0.2f, 0.9f, vLight);
			CreateEffect( MODEL_SWELL_OF_MAGICPOWER_BUFF_EFF, o->Position, o->Angle, vLight, 0, o );
		}break;
#endif // KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
#endif // KJH_FIX_SWELLOFMAGIC_EFFECT			// 정리할때 지워야하는 소스
#ifdef YDG_ADD_DOPPELGANGER_MONSTER
	case eBuff_Doppelganger_Ascension:	// 도플갱어 승천
		{
			//3차전직 효과 출력
#ifdef YDG_MOD_DOPPELGANGER_END_SOUND
			PlayBuffer(SOUND_SKILL_BERSERKER);
#else	// YDG_MOD_DOPPELGANGER_END_SOUND
			PlayBuffer(SOUND_CHAOS_END);
#endif	// YDG_MOD_DOPPELGANGER_END_SOUND
			CreateEffect(MODEL_CHANGE_UP_EFF,o->Position,o->Angle,o->Light,2,o);
			CreateEffect(MODEL_CHANGE_UP_CYLINDER,o->Position,o->Angle,o->Light,2,o);
			CreateEffect(MODEL_INFINITY_ARROW3, o->Position, o->Angle, o->Light, 1, o);
		}
		break;
#endif	// YDG_ADD_DOPPELGANGER_MONSTER
#ifdef PBG_ADD_AURA_EFFECT
	case eBuff_SD_Addition:
		{
			DeleteEffect(MODEL_EFFECT_SD_AURA,o);
			CreateEffect(MODEL_EFFECT_SD_AURA, o->Position, o->Angle, o->Light, 0, o);
		}
		break;
	case eBuff_AG_Addition:
		{
			DeleteParticle(BITMAP_AG_ADDITION_EFFECT);

			for(int i=0; i<5; ++i)
			{
				CreateParticle(BITMAP_AG_ADDITION_EFFECT,o->Position,o->Angle,o->Light,0, 1.0f, o);
				CreateParticle(BITMAP_AG_ADDITION_EFFECT,o->Position,o->Angle,o->Light,1, 1.0f, o);
				CreateParticle(BITMAP_AG_ADDITION_EFFECT,o->Position,o->Angle,o->Light,2, 1.0f, o);
			}
		}
		break;
#endif //PBG_ADD_AURA_EFFECT
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	case eBuff_Att_up_Ourforces:
		{
			DeleteEffect(BITMAP_LIGHT_RED, o, 1);
			CreateEffect(BITMAP_LIGHT_RED, o->Position, o->Angle, o->Light, 1, o,-1,0,0,0,1.5f);
		}
		break;
	case eBuff_Hp_up_Ourforces:
		{
			DeleteEffect(BITMAP_LIGHT_RED, o, 0);
			CreateEffect(BITMAP_LIGHT_RED, o->Position, o->Angle, o->Light, 0, o,-1,0,0,0,1.5f);
		}
		break;
	case eBuff_Def_up_Ourforces:
		{
			DeleteEffect(BITMAP_LIGHT_RED, o, 2);
			CreateEffect(BITMAP_LIGHT_RED, o->Position, o->Angle, o->Light, 2, o,-1,0,0,0,1.5f);		
		}
		break;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
	}
}

void ClearBuffPhysicalEffect( eBuffState buff, OBJECT* o )
{
	switch(buff)
	{
	case eBuff_CursedTempleProdection:
		{
			if( g_isCharacterBuff(o, eBuff_CursedTempleProdection ) )
			{
				DeleteEffect( MODEL_CURSEDTEMPLE_PRODECTION_SKILL, o );
			}
		}
		break;
		
	case eBuff_Defense:
		{
			DeleteJoint(MODEL_SPEARSKILL,o,4);
			DeleteJoint(MODEL_SPEARSKILL, o, 9);	//^ 펜릴 버그
		}
		break;
		
	case eBuff_AddAG:
		{
			DeleteEffect(BITMAP_LIGHT,o, 2);
		}
		break;
		
	case eBuff_HpRecovery:
		//case eBuff_Life:
		{
			DeleteEffect(BITMAP_LIGHT,o, 1);
		}
		break;
		
	case eBuff_PhysDefense:
		{
			if ( o->Type==MODEL_PLAYER )
			{
				DeleteJoint( MODEL_SPEARSKILL, o,0);
			}
		}
		break;
		
	case eBuff_HelpNpc:
		{
			DeleteJoint(MODEL_SPEARSKILL, o, 4);
			DeleteJoint(MODEL_SPEARSKILL, o, 9);	// 조인트 익페트 제거
		}
		break;
		
	case eBuff_RemovalMagic:
		{
			// 이펙트 제거
			DeleteEffect( BITMAP_LIGHT, o, 2 );
			DeleteEffect( BITMAP_LIGHT, o, 1 );
			DeleteEffect( BITMAP_SKULL, o, 0 );
			// 조인트 이펙트 제거
			DeleteJoint ( MODEL_SPEARSKILL, o, 4 );
			DeleteJoint ( MODEL_SPEARSKILL, o, 9 );
			DeleteJoint ( MODEL_SPEARSKILL, o, 0 );
		}
		break;
	case eDeBuff_CursedTempleRestraint:
		{
			if( g_isCharacterBuff(o, eDeBuff_CursedTempleRestraint ) )
			{
				DeleteEffect( MODEL_CURSEDTEMPLE_RESTRAINT_SKILL, o );
			}
		}
		break;
		
	case eDeBuff_Defense:
		{
			DeleteEffect(BITMAP_SKULL,o,0);
		}
		break;
		
	case eDeBuff_Stun:
		{
			DeleteEffect(BITMAP_SKULL,o,5);
		}
		break;
		
	case eDeBuff_NeilDOT:
		{
			g_SummonSystem.RemoveDamageOfTimeEffect(AT_SKILL_SUMMON_REQUIEM, o);
		}
		break;

	case eDeBuff_SahamuttDOT:
		{
			g_SummonSystem.RemoveDamageOfTimeEffect(AT_SKILL_SUMMON_EXPLOSION, o);
		}
		break;
		
	case eBuff_Thorns:
		{
			DeleteJoint(BITMAP_FLARE, o, 43);
		}
		break;

	case eDeBuff_Sleep:
		{
			DeleteEffect(MODEL_ALICE_BUFFSKILL_EFFECT, o, 4);
		}
		break;

	case eDeBuff_Blind:
		{
			DeleteEffect(MODEL_ALICE_BUFFSKILL_EFFECT, o, 3);
		}
		break;

	case eDeBuff_AttackDown:
		DeleteEffect(BITMAP_SHINY+6, o, 1);
		DeleteEffect(BITMAP_PIN_LIGHT, o, 1);
		break;
		
	case eDeBuff_DefenseDown:
		DeleteEffect(BITMAP_SHINY+6, o, 2);
		DeleteEffect(BITMAP_PIN_LIGHT, o, 2);
		break;
		
#ifdef ASG_ADD_SKILL_BERSERKER
	case eBuff_Berserker:
		{
			for (int i = 0; i < 4; ++i)
			{
				DeleteEffect(BITMAP_ORORA, o, i);
				if (i == 2 || i == 3)
					DeleteEffect(BITMAP_SPARK+2, o, i);
			}
			DeleteEffect(BITMAP_LIGHT_MARKS, o);
		}
		break;
#endif	// ASG_ADD_SKILL_BERSERKER
		
#ifdef YDG_ADD_SKILL_FLAME_STRIKE
	case eDeBuff_FlameStrikeDamage:
		{
// 			DeleteEffect(BITMAP_FIRE_CURSEDLICH, o, 2);
		}
		break;
#endif	// YDG_ADD_SKILL_FLAME_STRIKE
#ifdef YDG_ADD_SKILL_GIGANTIC_STORM
	case eDeBuff_GiganticStormDamage:
		{
// 			DeleteEffect(BITMAP_JOINT_THUNDER, o, 0);
		}
		break;
#endif	// YDG_ADD_SKILL_GIGANTIC_STORM
#ifdef YDG_ADD_SKILL_LIGHTNING_SHOCK
	case eDeBuff_LightningShockDamage:
		{
// 			DeleteEffect(MODEL_LIGHTNING_SHOCK, o, 2);
		}
		break;
#endif	// YDG_ADD_SKILL_LIGHTNING_SHOCK
#ifdef CSK_ADD_SKILL_BLOWOFDESTRUCTION
	case eDeBuff_BlowOfDestruction:
		{

		}
		break;
#endif // CSK_ADD_SKILL_BLOWOFDESTRUCTION
#ifdef KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
	case eBuff_SwellOfMagicPower:
		{
			// 버프 이팩트를 끈다.
			DeleteEffect(MODEL_SWELL_OF_MAGICPOWER_BUFF_EFF, o, 0);
		}break;
#endif // KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
#ifdef YDG_ADD_DOPPELGANGER_MONSTER
	case eBuff_Doppelganger_Ascension:	// 도플갱어 승천
		{
// 			DeleteEffect(BITMAP_JOINT_THUNDER, o, 0);
		}
		break;
#endif	// YDG_ADD_DOPPELGANGER_MONSTER
#ifdef PBG_ADD_AURA_EFFECT
	case eBuff_SD_Addition:
		{
			DeleteEffect(MODEL_EFFECT_SD_AURA,o);
		}
		break;
	case eBuff_AG_Addition:
		{
			DeleteParticle(BITMAP_AG_ADDITION_EFFECT);
		}
		break;
#endif //PBG_ADD_AURA_EFFECT
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	case eBuff_Att_up_Ourforces:
		{
			DeleteEffect(BITMAP_LIGHT_RED, o, 1);
		}
		break;
	case eBuff_Hp_up_Ourforces:
		{
			DeleteEffect(BITMAP_LIGHT_RED, o, 0);
		}
		break;
	case eBuff_Def_up_Ourforces:
		{
			DeleteEffect(BITMAP_LIGHT_RED, o, 2);
		}
		break;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
	}	
}

void RegisterBuff( eBuffState buff, OBJECT* o, const int bufftime )
{
	eBuffClass buffclasstype = g_IsBuffClass( buff );
	
	if( buffclasstype == eBuffClass_Count ) return;
	
	// 카오스캐슬이면 자기자신만 Register 되야 한다.
	if(InChaosCastle() && (o && o != &Hero->Object)) 
	{
		return;
	}
	// 물리적인 이펙트 부분
	InsertBuffPhysicalEffect( buff, o );
	// 예외 적인 부분
	if( CheckExceptionBuff( buff, o, false ) )
	{
		g_CharacterRegisterBuff(o, buff);
		// 논리적인 이펙트 부분
		InsertBuffLogicalEffect( buff, o, bufftime );
	}
}

void UnRegisterBuff( eBuffState buff, OBJECT* o )
{
	eBuffClass buffclasstype = g_IsBuffClass( buff );
	
	if( buffclasstype == eBuffClass_Count ) return;

	// 물리적인 이펙트 부분
	ClearBuffPhysicalEffect( buff, o );
	// 예외 적인 부분
	if( CheckExceptionBuff( buff, o, true ) )
	{
		g_CharacterUnRegisterBuff(o, buff);
		// 논리적인 이펙트 부분
		ClearBuffLogicalEffect( buff, o );
	}
}

