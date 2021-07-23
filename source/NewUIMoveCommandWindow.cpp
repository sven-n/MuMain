// NewUIMoveCommandWindow.cpp: implementation of the CNewUIMoveCommandWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "NewUIMoveCommandWindow.h"
#include "NewUISystem.h"
#include "ZzzInterface.h"
#include "wsclientinline.h"
#include "ChangeRingManager.h"
#include "PCRoomPoint.h"
#ifdef CSK_MOD_PROTECT_AUTO_V1
#include "ProtectAuto.h"
#endif // CSK_MOD_PROTECT_AUTO_V1
#ifdef CSK_FIX_BLUELUCKYBAG_MOVECOMMAND
#include "Event.h"
#endif // CSK_FIX_BLUELUCKYBAG_MOVECOMMAND
#ifdef YDG_ADD_MOVE_COMMAND_PROTOCOL
#include "KeyGenerater.h"
#endif	// YDG_ADD_MOVE_COMMAND_PROTOCOL
#ifdef PBG_ADD_PKSYSTEM_INGAMESHOP
#include "PKSystem.h"
#endif //PBG_ADD_PKSYSTEM_INGAMESHOP
#ifdef ASG_ADD_GENS_SYSTEM
#include "Local.h"
#endif	// ASG_ADD_GENS_SYSTEM
#ifdef KJH_ADD_SERVER_LIST_SYSTEM
#include "ServerListManager.h"
#endif // KJH_ADD_SERVER_LIST_SYSTEM
#ifdef LJH_ADD_SAVEOPTION_WHILE_MOVING_FROM_OR_TO_DIFF_SERVER
#include "ZzzOpenData.h"
#endif //LJH_ADD_SAVEOPTION_WHILE_MOVING_FROM_OR_TO_DIFF_SERVER

using namespace SEASON3B;

#ifdef CSK_LUCKY_SEAL

namespace 
{
	//로드 할때 처리 하자...
#ifdef LDK_ADD_INGAMESHOP_LIMIT_MOVE_WINDOW
#ifdef ASG_ADD_STRIFE_KANTURU_RUIN_ISLAND
	const int MapNameCount = 30;
#else	// ASG_ADD_STRIFE_KANTURU_RUIN_ISLAND
#ifdef LDK_FIX_BLUESERVER_UNLIMIT_AREA
	//지역 추가
#ifdef LDS_ADD_MOVEMAP_UNITEDMARKETPLACE
	const int MapNameCount = 29;
#else // LDS_ADD_MOVEMAP_UNITEDMARKETPLACE
	const int MapNameCount = 28;
#endif // LDS_ADD_MOVEMAP_UNITEDMARKETPLACE
#else
	const int MapNameCount = 25;
#endif //LDK_FIX_BLUESERVER_UNLIMIT_AREA
#endif	// ASG_ADD_STRIFE_KANTURU_RUIN_ISLAND
	const string MapName[MapNameCount] = {
		"Arena",
		"Lorencia",
		"Noria",
		"Elbeland",
		"Devias",
		"Dungeon",
		"Atlans",
		"LostTower",
		"Tarkan",
		"Aida1",
		"Icarus",
		"Crywolf",
		"Kanturu_ruin1",
		"Kanturu_Remain",
		"Raklion",
		"Swamp_of_Calmness",
		"Vulcanus",
		"Kalima1",
		"Kalima2",
		"Kalima3",
		"Kalima4",
		"Kalima5",
		"Kalima6",
		"Kalima7",
		"LorenDeep",
#ifdef LDK_FIX_BLUESERVER_UNLIMIT_AREA
		//지역 추가
		"Elbeland2",
		"Devias2",
		"Dungeon2",
#endif //LDK_FIX_BLUESERVER_UNLIMIT_AREA
#ifdef LDS_ADD_MOVEMAP_UNITEDMARKETPLACE
		"Loren_Market",
#endif // LDS_ADD_MOVEMAP_UNITEDMARKETPLACE
#ifdef ASG_ADD_STRIFE_KANTURU_RUIN_ISLAND
		"Kanturu_ruin_island",
#endif	// ASG_ADD_STRIFE_KANTURU_RUIN_ISLAND
	};
#else //LDK_ADD_INGAMESHOP_LIMIT_MOVE_WINDOW
#ifdef LDS_ADD_MOVEMAP_UNITEDMARKETPLACE
	const int MapNameCount = 7;
#else  //LDS_ADD_MOVEMAP_UNITEDMARKETPLACE
	const int MapNameCount = 6;
#endif // LDS_ADD_MOVEMAP_UNITEDMARKETPLACE

	const string MapName[MapNameCount] = {
		"Lorencia",
		"Noria",
		"Elbeland",
		"Dungeon",
		"Devias",
		"LostTower",
#ifdef LDS_ADD_MOVEMAP_UNITEDMARKETPLACE
		"Loren_Market",
#endif // LDS_ADD_MOVEMAP_UNITEDMARKETPLACE
	};
#endif //LDK_ADD_INGAMESHOP_LIMIT_MOVE_WINDOW

	const bool IsLuckySeal( const string& name ) 
	{
		if( name.size() != 0 ) {
			for( int i = 0; i < MapNameCount; ++i)  {
#ifdef PBG_WOPS_MOVE_MAPNAME_JAPAN
				if(strcmp(name.c_str(), MapName[i].c_str()) ==0)
#else //PBG_WOPS_MOVE_MAPNAME_JAPAN
				if( name == MapName[i]) 
#endif //PBG_WOPS_MOVE_MAPNAME_JAPAN
				{
					return true;
				}
			}
		}
		return false;
	}
};

#endif //CSK_LUCKY_SEAL

#ifdef YDG_MOD_PROTECT_AUTO_V4_R3
BYTE g_btKeyAddressBlock1[4] = { 0, 0, 0, 0 };
#endif	// YDG_MOD_PROTECT_AUTO_V4_R3

#ifdef YDG_ADD_ENC_MOVE_COMMAND_WINDOW

#ifdef YDG_MOD_PROTECT_AUTO_V4_R5
int * g_piEncValue = NULL;	// 리턴값 암호화
#endif	// YDG_MOD_PROTECT_AUTO_V4_R5

#ifdef YDG_MOD_PROTECT_AUTO_V4_R2
#ifdef YDG_MOD_PROTECT_AUTO_V4_R5
#define DECREAD(VAR) (CMoveCommandWindowEncrypt::Read(&VAR) ^ *g_piEncValue)
#define DECWRITE(VAR,VALUE) CMoveCommandWindowEncrypt::Write(&VAR,VALUE)
#else	// YDG_MOD_PROTECT_AUTO_V4_R5
#define DECREAD(VAR) CMoveCommandWindowEncrypt::Read(&VAR)
#define DECWRITE(VAR,VALUE) CMoveCommandWindowEncrypt::Write(&VAR,VALUE)
#endif	// YDG_MOD_PROTECT_AUTO_V4_R5
#endif	// YDG_MOD_PROTECT_AUTO_V4_R2

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNewUIMoveCommandWindow::CNewUIMoveCommandWindow()
{
	m_pNewUIMng = NULL;
	m_Pos.x = m_Pos.y = 0;

	memset( &m_StartUISubjectName, 0, sizeof(POINT) );
	memset( &m_StartMapNamePos, 0, sizeof(POINT) );
	memset( &m_MapNameUISize, 0, sizeof(POINT) );
#ifdef ASG_ADD_GENS_SYSTEM
	memset(&m_StrifePos, 0, sizeof(POINT));
#endif	// ASG_ADD_GENS_SYSTEM
	memset( &m_MapNamePos, 0, sizeof(POINT) );
	memset( &m_ReqLevelPos, 0, sizeof(POINT) );
	memset( &m_ReqZenPos, 0, sizeof(POINT) );
	m_iSelectedMapName = -1;

	memset( &m_ScrollBarPos, 0, sizeof(POINT) );
	memset( &m_ScrollBtnStartPos, 0, sizeof(POINT) );
	memset( &m_ScrollBtnPos, 0, sizeof(POINT) );
	m_iScrollBarHeightPixel = 0;
	m_iRenderStartTextIndex = 0;
	m_iSelectedTextIndex = -1;
	m_iScrollBtnInterval = 0;
	m_iScrollBarMiddleNum = 0;
	m_iScrollBarMiddleRemainderPixel = 0;
	m_iNumPage = 0;
	m_iCurPage = 0;
	m_iMousePosY = 0;
	m_bScrollBtnActive = false;
	m_iScrollBtnMouseEvent = MOVECOMMAND_MOUSEBTN_NORMAL;

	m_iPosY_Random = 0;
	m_iPosY_Random_Range = 50;
	m_iPosY_Random_Min = 15;
	m_Pos_Start.x = 0;
	m_Pos_Start.y = 0;
	m_iWindowOpenCount = 0;
	m_iWorldIndex = NUM_WD;
	m_bCastleOwner = false;

	m_iTextLine = MOVECOMMAND_MAX_RENDER_TEXTLINE;
	m_iTextAlpha = 255;
	m_fBackgroundAlpha = 0.8f;
	m_iTopSpace = 0;
	m_iWheelCounter = 0;
	m_iRealFontHeight = 0;

#ifdef YDG_MOD_PROTECT_AUTO_V4_R3
	m_pbtKeyAddressBlock3 = new BYTE[4];
	for (int i = 0; i < 4; ++i)
		m_pbtKeyAddressBlock3[i] = 0;
#endif	// YDG_MOD_PROTECT_AUTO_V4_R3

#ifdef YDG_ADD_MOVE_COMMAND_PROTOCOL
	m_dwMoveCommandKey = 0;
#endif	// YDG_ADD_MOVE_COMMAND_PROTOCOL

#ifdef YDG_MOD_PROTECT_AUTO_V4_R5
	g_piEncValue = new int;
	*g_piEncValue = rand() * rand();	// 리턴값 암호화
#endif	// YDG_MOD_PROTECT_AUTO_V4_R5
}

CNewUIMoveCommandWindow::~CNewUIMoveCommandWindow()
{
#ifdef YDG_MOD_PROTECT_AUTO_V4_R3
	delete [] m_pbtKeyAddressBlock3;
#endif	// YDG_MOD_PROTECT_AUTO_V4_R3
#ifdef YDG_MOD_PROTECT_AUTO_V4_R5
	delete g_piEncValue;
#endif	// YDG_MOD_PROTECT_AUTO_V4_R5
	Release();
}

//---------------------------------------------------------------------------------------------
// Create
bool SEASON3B::CNewUIMoveCommandWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if( NULL == pNewUIMng )
		return false;

#ifdef YDG_MOD_PROTECT_AUTO_V4
// 	CMoveCommandWindowEncrypt enc;
#endif	// YDG_MOD_PROTECT_AUTO_V4
	DECWRITE(m_Pos_Start.x, x);
	DECWRITE(m_Pos_Start.y, y);
	
	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj( SEASON3B::INTERFACE_MOVEMAP, this );		// 인터페이스 오브젝트 등록

	SetPos(x, y);
	
	LoadImages();
	
	Show( false );
	
	return true;
}

//---------------------------------------------------------------------------------------------
// Release
void SEASON3B::CNewUIMoveCommandWindow::Release()
{
	UnloadImages();
	
	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj( this );
		m_pNewUIMng = NULL;
	}
}

//---------------------------------------------------------------------------------------------
// SetPos
void SEASON3B::CNewUIMoveCommandWindow::SetPos(int x, int y)
{
#ifdef YDG_MOD_PROTECT_AUTO_V4
// 	CMoveCommandWindowEncrypt enc;
#endif	// YDG_MOD_PROTECT_AUTO_V4
	DECWRITE(m_Pos.x, x);
	DECWRITE(m_Pos.y, y);

	// 윈도우 해상도에 따라 UI크기를 변경시킨다.
#ifdef ASG_ADD_GENS_SYSTEM
	DECWRITE(m_StrifePos.x, DECREAD(m_Pos.x) + 20);
	switch( WindowWidth )
	{
	case 800:
		DECWRITE(m_MapNameUISize.x, 200); DECWRITE(m_MapNamePos.x, DECREAD(m_Pos.x) + 69); DECWRITE(m_ReqLevelPos.x, DECREAD(m_Pos.x) + 129); DECWRITE(m_ReqZenPos.x, DECREAD(m_Pos.x) + 174);
		break;
	case 1024:
		DECWRITE(m_MapNameUISize.x, 180); DECWRITE(m_MapNamePos.x, DECREAD(m_Pos.x) + 64); DECWRITE(m_ReqLevelPos.x, DECREAD(m_Pos.x) + 119); DECWRITE(m_ReqZenPos.x, DECREAD(m_Pos.x) + 159);
		break;
	case 1280:
		DECWRITE(m_MapNameUISize.x, 160); DECWRITE(m_MapNamePos.x, DECREAD(m_Pos.x) + 59); DECWRITE(m_ReqLevelPos.x, DECREAD(m_Pos.x) + 104); DECWRITE(m_ReqZenPos.x, DECREAD(m_Pos.x) + 139);
		break;
	}
#else	// ASG_ADD_GENS_SYSTEM
	switch( WindowWidth )
	{
	case 640:
		DECWRITE(m_MapNameUISize.x, 190); DECWRITE(m_MapNamePos.x, DECREAD(m_Pos.x) + 44); DECWRITE(m_ReqLevelPos.x, DECREAD(m_Pos.x) + 104); DECWRITE(m_ReqZenPos.x, DECREAD(m_Pos.x) + 154);
		break;
	case 800:
		DECWRITE(m_MapNameUISize.x, 170); DECWRITE(m_MapNamePos.x, DECREAD(m_Pos.x) + 39); DECWRITE(m_ReqLevelPos.x, DECREAD(m_Pos.x) + 99); DECWRITE(m_ReqZenPos.x, DECREAD(m_Pos.x) + 144);
		break;
	case 1024:
		DECWRITE(m_MapNameUISize.x, 150); DECWRITE(m_MapNamePos.x, DECREAD(m_Pos.x) + 34); DECWRITE(m_ReqLevelPos.x, DECREAD(m_Pos.x) + 89); DECWRITE(m_ReqZenPos.x, DECREAD(m_Pos.x) + 129);
		break;
	case 1280:
		DECWRITE(m_MapNameUISize.x, 130); DECWRITE(m_MapNamePos.x, DECREAD(m_Pos.x) + 29); DECWRITE(m_ReqLevelPos.x, DECREAD(m_Pos.x) + 74); DECWRITE(m_ReqZenPos.x, DECREAD(m_Pos.x) + 109);
		break;
	}
#endif	// ASG_ADD_GENS_SYSTEM

	DECWRITE(m_MapNameUISize.x, DECREAD(m_MapNameUISize.x) + 10);			// 스크롤바를 위해서 x의 길이를 10늘린다.

	m_listMoveInfoData = CMoveCommandData::GetInstance()->GetMoveCommandDatalist();
	DECWRITE(m_iRealFontHeight, FontHeight * 640 / WindowWidth + 2);

	DECWRITE(m_MapNameUISize.y, 60 + (DECREAD(m_iRealFontHeight) * DECREAD(m_iTextLine)) + DECREAD(m_iTopSpace));
	
	// UI제목 시작위치
	DECWRITE(m_StartUISubjectName.x, DECREAD(m_Pos.x) + DECREAD(m_MapNameUISize.x) / 2);
	DECWRITE(m_StartUISubjectName.y, DECREAD(m_Pos.y) + 4);

	// 맵 이름 시작위치
	DECWRITE(m_StartMapNamePos.x, DECREAD(m_Pos.x) + 2);
	DECWRITE(m_StartMapNamePos.y, DECREAD(m_Pos.y) + 38 + DECREAD(m_iTopSpace));
	
#ifdef ASG_ADD_GENS_SYSTEM
	// 분쟁지역
	DECWRITE(m_StrifePos.y, DECREAD(m_StartMapNamePos.y));
#endif	// ASG_ADD_GENS_SYSTEM

	// 맵이름 위치
	DECWRITE(m_MapNamePos.y, DECREAD(m_StartMapNamePos.y));

	// 요구레벨
	DECWRITE(m_ReqLevelPos.y, DECREAD(m_StartMapNamePos.y));

	// 요구젠
	DECWRITE(m_ReqZenPos.y, DECREAD(m_StartMapNamePos.y));

	DECWRITE(m_ScrollBarPos.x, DECREAD(m_Pos.x) + DECREAD(m_MapNameUISize.x) - 14);
	DECWRITE(m_ScrollBarPos.y, DECREAD(m_StartMapNamePos.y) - MOVECOMMAND_SCROLLBAR_TOP_HEIGHT);
	
	DECWRITE(m_ScrollBtnStartPos.x, DECREAD(m_ScrollBarPos.x) - (MOVECOMMAND_SCROLLBTN_WIDTH/2 - MOVECOMMAND_SCROLLBAR_TOP_WIDTH/2));
	DECWRITE(m_ScrollBtnStartPos.y, DECREAD(m_ScrollBarPos.y));
	DECWRITE(m_ScrollBtnPos.x, DECREAD(m_ScrollBtnStartPos.x));
	DECWRITE(m_ScrollBtnPos.y, DECREAD(m_ScrollBtnStartPos.y));

	DECWRITE(m_iScrollBarHeightPixel, DECREAD(m_iTextLine) * DECREAD(m_iRealFontHeight));
	
	DECWRITE(m_iScrollBarMiddleNum, (DECREAD(m_iScrollBarHeightPixel)-(MOVECOMMAND_SCROLLBAR_TOP_HEIGHT*2))/MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT);
	DECWRITE(m_iScrollBarMiddleRemainderPixel, (DECREAD(m_iScrollBarHeightPixel)-(MOVECOMMAND_SCROLLBAR_TOP_HEIGHT*2))%MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT);

	DECWRITE(m_iNumPage, 1 + (m_listMoveInfoData.size() / DECREAD(m_iTextLine)));
	
	DECWRITE(m_iCurPage, 1);
	
	// 스크롤바가 이동해야할 칸의 전체 갯수
	DECWRITE(m_iTotalMoveScrBtnperStep, m_listMoveInfoData.size() - DECREAD(m_iTextLine));
	// 스크롤바가 이동해야할 칸의 나머지 갯수
	DECWRITE(m_iRemainMoveScrBtnperStep, DECREAD(m_iTotalMoveScrBtnperStep));
	// 스크롤바가 이동해야할 전체 픽셀값
	DECWRITE(m_iTotalMoveScrBtnPixel, DECREAD(m_iScrollBarHeightPixel)-MOVECOMMAND_SCROLLBTN_HEIGHT);	
	// 스크롤바가 이동해야할 나머지 픽셀값
	DECWRITE(m_iRemainMoveScrBtnPixel, DECREAD(m_iTotalMoveScrBtnPixel));								
	// 스크롤바가 이동하는 한칸당 최소 픽셀
	DECWRITE(m_iMinMoveScrBtnPixelperStep, DECREAD(m_iTotalMoveScrBtnPixel)/DECREAD(m_iTotalMoveScrBtnperStep));	
	// 스크롤바가 이동하는 한칸당 최대 픽셀
	DECWRITE(m_iMaxMoveScrBtnPixelperStep, DECREAD(m_iMinMoveScrBtnPixelperStep)+1);						
	// 스크롤바가 이동해야하는 한칸당 최대픽셀의 갯수
	DECWRITE(m_iTotalNumMaxMoveScrBtnperStep, DECREAD(m_iTotalMoveScrBtnPixel)-(DECREAD(m_iTotalMoveScrBtnperStep)*DECREAD(m_iMinMoveScrBtnPixelperStep)));		
	// 스크롤바가 이동해야하는 한칸당 최대픽셀의 갯수
	DECWRITE(m_iTotalNumMinMoveScrBtnperStep, DECREAD(m_iTotalMoveScrBtnperStep) - DECREAD(m_iTotalNumMaxMoveScrBtnperStep));
	DECWRITE(m_icurMoveScrBtnPixelperStep, DECREAD(m_iMaxMoveScrBtnPixelperStep));
	DECWRITE(m_iAcumMoveMouseScrollPixel, 0);

	if( DECREAD(m_iNumPage) > 1 )
	{
		m_bScrollBtnActive = true;
	}

	DECWRITE(m_iRenderStartTextIndex, 0);
	DECWRITE(m_iRenderEndTextIndex, DECREAD(m_iRenderStartTextIndex) + DECREAD(m_iTextLine));

#ifdef _VS2008PORTING
	if( DECREAD(m_iRenderEndTextIndex) > (int)m_listMoveInfoData.size() )
#else // _VS2008PORTING
	if( DECREAD(m_iRenderEndTextIndex) > m_listMoveInfoData.size() )
#endif // _VS2008PORTING
	{
		DECWRITE(m_iRenderEndTextIndex, DECREAD(m_iRenderEndTextIndex) - (DECREAD(m_iRenderEndTextIndex)-m_listMoveInfoData.size()));
	}
}

#ifdef CSK_LUCKY_SEAL

bool SEASON3B::CNewUIMoveCommandWindow::IsLuckySealBuff()
{
	if( g_isCharacterBuff((&Hero->Object), eBuff_Seal1)  
		|| g_isCharacterBuff((&Hero->Object), eBuff_Seal2)   
		|| g_isCharacterBuff((&Hero->Object), eBuff_Seal3) 
#ifdef PSW_SEAL_ITEM
		|| g_isCharacterBuff((&Hero->Object), eBuff_Seal4) 
#endif //PSW_SEAL_ITEM
#ifdef PSW_ADD_PC4_SEALITEM
		|| g_isCharacterBuff((&Hero->Object), eBuff_Seal_HpRecovery)
		|| g_isCharacterBuff((&Hero->Object), eBuff_Seal_MpRecovery)
#endif //PSW_ADD_PC4_SEALITEM
#ifdef ASG_ADD_CS6_ASCENSION_SEAL_MASTER
		|| g_isCharacterBuff((&Hero->Object), eBuff_AscensionSealMaster)
#endif	// ASG_ADD_CS6_ASCENSION_SEAL_MASTER
#ifdef ASG_ADD_CS6_WEALTH_SEAL_MASTER
		|| g_isCharacterBuff((&Hero->Object), eBuff_WealthSealMaster)
#endif	// ASG_ADD_CS6_WEALTH_SEAL_MASTER
#if !defined LDK_FIX_NEWWEALTHSEAL_INFOTIMER_AND_MOVEWINDOW
		// eBuff_NewWealthSeal버프 사용중 이동창 비활성화 되야됨
	#ifdef LDK_ADD_INGAMESHOP_NEW_WEALTH_SEAL
 		|| g_isCharacterBuff((&Hero->Object), eBuff_NewWealthSeal)
	#endif //LDK_ADD_INGAMESHOP_NEW_WEALTH_SEAL
#endif //LDK_FIX_NEWWEALTHSEAL_INFOTIMER_AND_MOVEWINDOW
#ifdef YDG_ADD_CS7_PARTY_EXP_BONUS_ITEM
		|| g_isCharacterBuff((&Hero->Object), eBuff_PartyExpBonus)
#endif	// YDG_ADD_CS7_PARTY_EXP_BONUS_ITEM
		)
	{
		return true;
	}
	return false;
}

bool SEASON3B::CNewUIMoveCommandWindow::IsMapMove( const string& src )
{
	if( Hero->Object.Kind == KIND_PLAYER 
		&& Hero->Object.Type == MODEL_PLAYER 
		&& Hero->Object.SubType == MODEL_GM_CHARACTER )
	{
		return true;
	}

	if( g_isCharacterBuff((&Hero->Object), eBuff_GMEffect) )
	{
		return true;
	}

#ifdef KJH_ADD_PCROOM_MOVECOMMAND
	if( CPCRoomPtSys::Instance().IsPCRoom() == true )
	{
		return true;
	}
#endif // KJH_ADD_PCROOM_MOVECOMMAND

#ifdef YDG_MOD_PROTECT_AUTO_V4
//	CMoveCommandWindowEncrypt enc;
#endif	// YDG_MOD_PROTECT_AUTO_V4
	if( IsLuckySealBuff() == false ) 
	{
		char lpszStr1[1024]; char* lpszStr2 = NULL;
		if(src.find( GlobalText[260] ) != string::npos) {
			string temp = GlobalText[260];
			temp += ' ';
			wsprintf( lpszStr1, src.c_str() );	
			lpszStr2 = strtok( lpszStr1, temp.c_str() );
			if( lpszStr2 == NULL ) return false;

			SettingCanMoveMap();
			std::list<CMoveCommandData::MOVEINFODATA*>::iterator li = m_listMoveInfoData.begin();
			for( int i=0; i<DECREAD(m_iRenderEndTextIndex) ; i++, li++ ) {
				if(!strcmp( lpszStr2, (*li)->_ReqInfo.szMainMapName )) {
					if((*li)->_bCanMove == true ){
						return IsLuckySeal( (*li)->_ReqInfo.szSubMapName );
					}
				}
			}
			return false;
		}
		else if(src.find( "/move" ) != string::npos) {
			string temp = "/move";
			temp += ' ';
			wsprintf( lpszStr1, src.c_str() );	
#ifdef PBG_WOPS_MOVE_MAPNAME_JAPAN
			string lpszStr2;
			int iSize = src.size();
			lpszStr2 = src.substr(6, iSize);
#else //PBG_WOPS_MOVE_MAPNAME_JAPAN
			lpszStr2 = strtok( lpszStr1, temp.c_str() );
			if( lpszStr2 == NULL ) return false;
#endif //PBG_WOPS_MOVE_MAPNAME_JAPAN
			
			SettingCanMoveMap();
			std::list<CMoveCommandData::MOVEINFODATA*>::iterator li = m_listMoveInfoData.begin();
			for( int i=0; i<DECREAD(m_iRenderEndTextIndex) ; i++, li++ ) {
#ifdef PBG_WOPS_MOVE_MAPNAME_JAPAN
				if(stricmp(lpszStr2.c_str(), (*li)->_ReqInfo.szSubMapName) == 0)
				{
					if((*li)->_bCanMove == true)
					{
						bool bResult = IsLuckySeal((*li)->_ReqInfo.szSubMapName);
						
						return bResult;
					}
				}
#else //PBG_WOPS_MOVE_MAPNAME_JAPAN
				if(!stricmp( lpszStr2, (*li)->_ReqInfo.szMainMapName )) {
					if((*li)->_bCanMove == true ){
						return IsLuckySeal( (*li)->_ReqInfo.szSubMapName );
					}
				}
#endif //PBG_WOPS_MOVE_MAPNAME_JAPAN
			}
			return false;
		}
		else {
			return IsLuckySeal( src );
		}
	}
	return true;
}

#endif //CSK_LUCKY_SEAL

#ifdef YDG_ADD_MOVE_COMMAND_PROTOCOL
void SEASON3B::CNewUIMoveCommandWindow::SetMoveCommandKey(DWORD dwKey)
{
	m_dwMoveCommandKey = dwKey;
}

DWORD SEASON3B::CNewUIMoveCommandWindow::GetMoveCommandKey()
{
	m_dwMoveCommandKey = g_KeyGenerater.GenerateKeyValue(m_dwMoveCommandKey);

	return m_dwMoveCommandKey;
}
#endif	// YDG_ADD_MOVE_COMMAND_PROTOCOL

#ifdef ASG_ADD_GENS_SYSTEM
#ifndef KJH_ADD_SERVER_LIST_SYSTEM			// #ifndef
extern  int  ServerSelectHi;
extern  int  ServerLocalSelect;
#endif // KJH_ADD_SERVER_LIST_SYSTEM

// 분쟁 맵 설정.
void SEASON3B::CNewUIMoveCommandWindow::SetStrifeMap()
{
	std::list<CMoveCommandData::MOVEINFODATA*>::iterator li;

	if (BLUE_MU::IsBlueMuServer())	// 부분 유료화 서버인가?
	{
#ifdef ASG_ADD_STRIFE_KANTURU_RUIN_ISLAND
		const int c_nStrife = 10;
		int anStrifeIndex[c_nStrife] = { 11, 12, 13, 25, 27, 28, 29, 33, 42, 45 };	// MoveReq.txt의 맵 인덱스.
#else	// ASG_ADD_STRIFE_KANTURU_RUIN_ISLAND
		int anStrifeIndex[9] = { 11, 12, 13, 25, 27, 28, 29, 33, 42 };	// MoveReq.txt의 맵 인덱스.
#endif	// ASG_ADD_STRIFE_KANTURU_RUIN_ISLAND
		int i;
		for (li = m_listMoveInfoData.begin(); li != m_listMoveInfoData.end(); advance(li, 1))
		{
			(*li)->_bStrife = false;
#ifdef ASG_ADD_STRIFE_KANTURU_RUIN_ISLAND
			for (i = 0; i < c_nStrife; ++i)
#else	// ASG_ADD_STRIFE_KANTURU_RUIN_ISLAND
			for (i = 0; i < 9; ++i)
#endif	// ASG_ADD_STRIFE_KANTURU_RUIN_ISLAND
			{
				if ((*li)->_ReqInfo.index == anStrifeIndex[i])
				{
					(*li)->_bStrife = true;
					break;
				}
			}
		}
	}
#ifdef KJH_ADD_SERVER_LIST_SYSTEM
	else if (!g_ServerListManager->IsNonPvP())	// PVP 서버인가?
#else // KJH_ADD_SERVER_LIST_SYSTEM
	else if (!::IsNonPvpServer(ServerSelectHi, ServerLocalSelect))	// PVP 서버인가?
#endif // KJH_ADD_SERVER_LIST_SYSTEM
	{
		int anStrifeIndex[1] = { 42 };							// MoveReq.txt의 맵 인덱스.
		int i;
		for (li = m_listMoveInfoData.begin(); li != m_listMoveInfoData.end(); advance(li, 1))
		{
			(*li)->_bStrife = false;
			for (i = 0; i < 1; ++i)
			{
				if ((*li)->_ReqInfo.index == anStrifeIndex[i])
				{
					(*li)->_bStrife = true;
					break;
				}
			}
		}
	}
	else	// NonPVP 서버에서는 분쟁 지역 없음.
	{
		for (li = m_listMoveInfoData.begin(); li != m_listMoveInfoData.end(); advance(li, 1))
			(*li)->_bStrife = false;
	}
}
#endif	// ASG_ADD_GENS_SYSTEM

void SEASON3B::CNewUIMoveCommandWindow::SettingCanMoveMap()
{
	//----------------------------------------------
	//----------------------------------------------
	DWORD iZen;
	int iLevel, iReqLevel, iReqZen;

	std::list<CMoveCommandData::MOVEINFODATA*>::iterator li = m_listMoveInfoData.begin();
	for( int i=0 ; i<DECREAD(m_iRenderEndTextIndex) ; i++, li++ )
	{
		if( li == m_listMoveInfoData.end() )
		{
			break;
		}
		if( i<DECREAD(m_iRenderStartTextIndex) )
		{
			continue;
		}

		(*li)->_bCanMove = false;
		(*li)->_bSelected = false;

		iLevel = CharacterAttribute->Level;
		iZen = CharacterMachine->Gold;
		iReqLevel = (*li)->_ReqInfo.iReqLevel;
		iReqZen = (*li)->_ReqInfo.iReqZen;
#ifdef PBG_ADD_PKSYSTEM_INGAMESHOP
		// 무법자 1단계 상태 이상이고 무료섭이라면
		if(g_PKSystem->IsPKState())
		{
			// 이동 명령어(창) 사용시 소비되는 젠량 10배 상승
			iReqZen = g_PKSystem->GetReqZen(iReqZen);
		}
#endif //PBG_ADD_PKSYSTEM_INGAMESHOP

		// 마검사/다크로드일때 필요레벨을 계산(400 레벨은 제외)
		if( ( GetBaseClass(CharacterAttribute->Class)==CLASS_DARK || GetBaseClass(CharacterAttribute->Class)==CLASS_DARK_LORD 
#ifdef PBG_ADD_NEWCHAR_MONK
				|| GetBaseClass(CharacterAttribute->Class)==CLASS_RAGEFIGHTER
#endif //PBG_ADD_NEWCHAR_MONK
			)
			&& (iReqLevel != 400) )
		{
#ifdef CSK_FIX_GM_MOVE
			char strNewMapName[64];
			bool bKalima = false;

			// 칼리마1부터 칼리마6까지는 필요레벨 - 20 이고
			// 칼리마7은 필요레벨 그대로
			{ // for 루프 포팅
				int i = 0;
				for(i=0; i<8; ++i)
				{
					// 58 "칼리마"
					sprintf(strNewMapName, "%s%d", GlobalText[58], i);
					
					if(strcmp((*li)->_ReqInfo.szMainMapName, strNewMapName) == 0)
					{
						bKalima = true;
						break;
					}
				}

				if(bKalima == true)
				{
					if(i >= 0 && i <= 6)
					{
						iReqLevel = iReqLevel - 20;
					}
				}
				else
	#endif // CSK_FIX_GM_MOVE
				{
					iReqLevel = int(float(iReqLevel)*2.f/3.f);
				}
			} // for 루프 포팅
		}

		// 필요레벨충족/필요젠충족/pk모드가 아닐때 이동가능
#ifdef _VS2008PORTING
#ifdef ASG_FIX_MOVE_WIN_MURDERER1_BUG
#ifdef PBG_ADD_PKSYSTEM_INGAMESHOP
		// 기획변경으로 인해 디버프 걸려있을시에만 사용불가능으로변경
		// 혼합유료화 섭일 경우 무법자 1단계(5분은 불가)와 상관없이 모두 이동창 사용가능
		// 혼합유료화일경우 버프와 PVP_MURDERER1이상일경우를 판단하고
		// 아닐결우는 (int)Hero->PK<PVP_MURDERER1) 결과값에 의해서만 결정을 한다	
		if(g_PKSystem->GetCanDoMoveCommand() && iLevel >= iReqLevel && iZen >= iReqZen
#ifdef ASG_ADD_MOVEREQ_TEXT_MAX_LEVEL
			&& iLevel <= (*li)->_ReqInfo.m_iReqMaxLevel
#endif	// ASG_ADD_MOVEREQ_TEXT_MAX_LEVEL
#else //PBG_ADD_PKSYSTEM_INGAMESHOP
		if( iLevel >= iReqLevel && (int)iZen >= iReqZen && (int)Hero->PK<PVP_MURDERER1
#endif //PBG_ADD_PKSYSTEM_INGAMESHOP
#else	// ASG_FIX_MOVE_WIN_MURDERER1_BUG
		if( iLevel >= iReqLevel && (int)iZen >= iReqZen && (int)Hero->PK<PVP_MURDERER2
#endif	// ASG_FIX_MOVE_WIN_MURDERER1_BUG
#else // _VS2008PORTING
#ifdef ASG_FIX_MOVE_WIN_MURDERER1_BUG
#ifdef PBG_ADD_PKSYSTEM_INGAMESHOP
		// 기획변경으로 인해 디버프 걸려있을시에만 사용불가능으로변경
		// 혼합유료화 섭일 경우 무법자 1단계(5분은 불가)와 상관없이 모두 이동창 사용가능
		// 혼합유료화일경우 버프와 PVP_MURDERER1이상일경우를 판단하고
		// 아닐결우는 (int)Hero->PK<PVP_MURDERER1) 결과값에 의해서만 결정을 한다	
		if(g_PKSystem->GetCanDoMoveCommand() && iLevel >= iReqLevel && iZen >= iReqZen
#ifdef ASG_ADD_MOVEREQ_TEXT_MAX_LEVEL
			&& iLevel <= (*li)->_ReqInfo.m_iReqMaxLevel
#endif	// ASG_ADD_MOVEREQ_TEXT_MAX_LEVEL
#else //PBG_ADD_PKSYSTEM_INGAMESHOP
		if( iLevel >= iReqLevel && iZen >= iReqZen && Hero->PK<PVP_MURDERER1
#endif //PBG_ADD_PKSYSTEM_INGAMESHOP
#else	// ASG_FIX_MOVE_WIN_MURDERER1_BUG
		if( iLevel >= iReqLevel && iZen >= iReqZen && Hero->PK<PVP_MURDERER2
#endif	// ASG_FIX_MOVE_WIN_MURDERER1_BUG
#endif // _VS2008PORTING
#ifdef CSK_LUCKY_SEAL////////////////////////////////////////////////////////////
#ifdef PBG_MOD_VIEMAPMOVE
	#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE || defined LDK_ADD_INGAMESHOP_LIMIT_MOVE_WINDOW
			&& IsMapMove( (*li)->_ReqInfo.szSubMapName )
	#endif //SELECTED_LANGUAGE == LANGUAGE_JAPANESE || defined LDK_ADD_INGAMESHOP_LIMIT_MOVE_WINDOW
#else //PBG_MOD_VIEMAPMOVE
	#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE || SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE || defined LDK_ADD_INGAMESHOP_LIMIT_MOVE_WINDOW
			&& IsMapMove( (*li)->_ReqInfo.szSubMapName )
	#endif //SELECTED_LANGUAGE == LANGUAGE_JAPANESE || SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE || defined LDK_ADD_INGAMESHOP_LIMIT_MOVE_WINDOW
#endif //PBG_MOD_VIEMAPMOVE
#endif //CSK_LUCKY_SEAL////////////////////////////////////////////////////////////
			)
		{
			ITEM* pEquipedRightRing = &CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];
			ITEM* pEquipedLeftRing = &CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];
			ITEM* pEquipedHelper = &CharacterMachine->Equipment[EQUIPMENT_HELPER];
			ITEM* pEquipedWing = &CharacterMachine->Equipment[EQUIPMENT_WING];
			
			// 이카루스 일때 예외처리
			if(strcmp((*li)->_ReqInfo.szMainMapName,GlobalText[55])==0)		// 55 "이카루스"
			{
				// 펜릴/디노란트/군주의망토/날개를 착용해야 이동가능
				// 변신반지 시리즈 착용시 이동불가
				if( 
					(
					pEquipedHelper->Type == ITEM_HELPER+37 			// 펜릴			
#ifndef PSW_BUGFIX_ICARUS_MOVE_UNIRIA    // 코드 정리 할것 !!!
					|| pEquipedHelper->Type == ITEM_HELPER+2		// 유니리아
#endif //PSW_BUGFIX_ICARUS_MOVE_UNIRIA   // 코드 정리 할것 !!!
					|| pEquipedHelper->Type == ITEM_HELPER+3		// 디노란트
#ifdef KJH_FIX_WOPS_K26606_TRADE_WING_IN_IKARUS
					|| pEquipedHelper->Type == ITEM_HELPER+4		// 다크호스
#endif // KJH_FIX_WOPS_K26606_TRADE_WING_IN_IKARUS
					|| pEquipedWing->Type == ITEM_HELPER+30			// 군주의 망토
#ifdef ADD_ALICE_WINGS_1		 
					|| (pEquipedWing->Type >= ITEM_WING+36 && pEquipedWing->Type <= ITEM_WING+43)			
#else	// ADD_ALICE_WINGS_1
					|| (pEquipedWing->Type >= ITEM_WING+36 && pEquipedWing->Type <= ITEM_WING+40) 
#endif	// ADD_ALICE_WINGS_1
					|| (pEquipedWing->Type >= ITEM_WING && pEquipedWing->Type <= ITEM_WING+6) 
#ifdef LDK_ADD_INGAMESHOP_SMALL_WING			// 기간제 날개 작은(군망, 재날, 요날, 천날, 사날)
					|| ( ITEM_WING+130 <= pEquipedWing->Type && pEquipedWing->Type <= ITEM_WING+134 )
#endif //LDK_ADD_INGAMESHOP_SMALL_WING
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
					|| (pEquipedWing->Type >= ITEM_WING+49 && pEquipedWing->Type <= ITEM_WING+50)
					|| (pEquipedWing->Type == ITEM_WING+135)
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
					)
#ifdef KJH_FIX_WOPS_26619_CANMOVE_IKARUS_BY_UNIRIA
					&& !( pEquipedHelper->Type == ITEM_HELPER+2 )	// 유니리아
#endif // KJH_FIX_WOPS_26619_CANMOVE_IKARUS_BY_UNIRIA
#ifdef KJH_FIX_MOVE_ICARUS_EQUIPED_PANDA_CHANGE_RING
					&& ( g_ChangeRingMgr->CheckBanMoveIcarusMap(pEquipedRightRing->Type, pEquipedLeftRing->Type) == false )
#else // KJH_FIX_MOVE_ICARUS_EQUIPED_PANDA_CHANGE_RING
					&& ( g_ChangeRingMgr->CheckMoveMap(pEquipedRightRing->Type, pEquipedLeftRing->Type) == false )
#endif // KJH_FIX_MOVE_ICARUS_EQUIPED_PANDA_CHANGE_RING
					)
				{
					(*li)->_bCanMove = true;
				}
				else
				{
					(*li)->_bCanMove = false;
				}
			}
			// 아틀란스 시리즈 일때 예외처리
			else if(strncmp((*li)->_ReqInfo.szMainMapName, GlobalText[37], 8)==0)	// 37 "아틀란스"
			{
				// 유니리아/디노란트를 탄 상태에서는 아틀란스 이동불가
				if(pEquipedHelper->Type == ITEM_HELPER+2 || pEquipedHelper->Type == ITEM_HELPER+3)
				{
					(*li)->_bCanMove = false;
				}
				else
				{
					(*li)->_bCanMove = true;
				}
			}
			// 669 "로랜협곡"
			else if(strcmp((*li)->_ReqInfo.szMainMapName,GlobalText[669]) == 0)
			{
				// 클라이언트에서 수성측길드, 수성측연합인가 판별할 길이 없다. [2/17/2009 nukun]
				// 추후 서버와의 연계를 통해 판별루틴을 넣어야 한다. [2/17/2009 nukun]
				if(m_bCastleOwner == true)
				{
					(*li)->_bCanMove = true;
				}
				else
				{
					(*li)->_bCanMove = false;
				}
			}
			else
			{
				(*li)->_bCanMove = true;
			}	

			// 칼리마검사
			for(int i=0; i<8; ++i)
			{
				char strNewMapName[64];
				// 58 "칼리마"
				sprintf(strNewMapName, "%s%d", GlobalText[58], i);
				
				if(strcmp((*li)->_ReqInfo.szMainMapName, strNewMapName) == 0)
				{
					// PC방인지 안닌지
					if(CPCRoomPtSys::Instance().IsPCRoom() == true 
#ifdef CSK_FIX_BLUELUCKYBAG_MOVECOMMAND
						// 파란복주머니 사용했을때 가능
						|| g_pBlueLuckyBagEvent->IsEnableBlueLuckyBag() == true
#endif // CSK_FIX_BLUELUCKYBAG_MOVECOMMAND
						)
					{
						(*li)->_bCanMove = true;
					}
					else
					{
						(*li)->_bCanMove = false;
					}

					break;
				}
			}
		}
#ifdef ASG_ADD_GENS_SYSTEM
	// 이동 가능한 지역이고 분쟁 지역이고 겐스에 가입되어 있지 않다면 이동 불가.
		if ((*li)->_bCanMove && (*li)->_bStrife && 0 == Hero->m_byGensInfluence)
			(*li)->_bCanMove = false;
#endif	// ASG_ADD_GENS_SYSTEM
	}
	//----------------------------------------------
	//----------------------------------------------
}

//---------------------------------------------------------------------------------------------
// BtnProcess
bool SEASON3B::CNewUIMoveCommandWindow::BtnProcess()
{
	int iX, iY;

#ifdef YDG_MOD_PROTECT_AUTO_V4
//	CMoveCommandWindowEncrypt enc;
#endif	// YDG_MOD_PROTECT_AUTO_V4
	if( CheckMouseIn( DECREAD(m_ScrollBtnPos.x), DECREAD(m_ScrollBtnPos.y), MOVECOMMAND_SCROLLBTN_WIDTH, MOVECOMMAND_SCROLLBTN_HEIGHT ))
	{	
		if(IsPress(VK_LBUTTON))
		{
			DECWRITE(m_iScrollBtnMouseEvent, MOVECOMMAND_MOUSEBTN_CLICKED);
			DECWRITE(m_iMousePosY, MouseY);
			DECWRITE(m_iAcumMoveMouseScrollPixel, 0);
		}	
	}

	if(IsRelease(VK_LBUTTON))
	{
		DECWRITE(m_iScrollBtnMouseEvent, MOVECOMMAND_MOUSEBTN_NORMAL);
		DECWRITE(m_iAcumMoveMouseScrollPixel, 0);
	}
	
	if( DECREAD(m_iScrollBtnMouseEvent) == MOVECOMMAND_MOUSEBTN_CLICKED && DECREAD(m_icurMoveScrBtnPixelperStep) > 0 )
	{
		//MoveScrollBtn(MouseY-m_iMousePosY);
		int iMoveValue = MouseY-DECREAD(m_iMousePosY);
			
		if( iMoveValue < 0 )
		{
			if( MouseY <= DECREAD(m_ScrollBtnPos.y)+(MOVECOMMAND_SCROLLBTN_HEIGHT/2))
			{
				if( -(iMoveValue) > (DECREAD(m_iTotalMoveScrBtnPixel)-DECREAD(m_iRemainMoveScrBtnPixel)) )
				{
					iMoveValue = -(DECREAD(m_iTotalMoveScrBtnPixel)-DECREAD(m_iRemainMoveScrBtnPixel));
				}
				ScrollUp(-iMoveValue);
			}
		}
		else if( iMoveValue > 0 )
		{
			if( MouseY >= DECREAD(m_ScrollBtnPos.y)+(MOVECOMMAND_SCROLLBTN_HEIGHT/2))
			{
				if( iMoveValue > DECREAD(m_iRemainMoveScrBtnPixel) )
				{
					iMoveValue = DECREAD(m_iRemainMoveScrBtnPixel);
				}
				ScrollDown(iMoveValue);
			}
		}
		DECWRITE(m_iMousePosY, MouseY);	
	}

	// 맵 정보 셋팅
	SettingCanMoveMap();

	// 맵 이동창에 마우스를 올렸을때 계산
	if( CheckMouseIn( DECREAD(m_Pos.x), DECREAD(m_Pos.y), DECREAD(m_MapNameUISize.x), DECREAD(m_MapNameUISize.y) ) )
	{
		if( DECREAD(m_icurMoveScrBtnPixelperStep) > 0 )
		{
#ifdef CSK_MOD_PROTECT_AUTO_V2
#ifdef YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V2
			if (!g_pProtectAuto->IsNewVersion());
			else
#endif	// YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V2
			// 오토마우스 작동중이면 가끔 휠기능 먹통되게 수정 
#ifdef YDG_FIX_NONAUTO_MOVE_DEVIAS2
			if(g_pProtectAuto->IsStartAuto() == true && g_pProtectAuto->IsAutoProcess() == true && rand()%10 == 0)
#else	// YDG_FIX_NONAUTO_MOVE_DEVIAS2
			if(g_pProtectAuto->IsStartAuto() == true && rand()%10 == 0)
#endif	// YDG_FIX_NONAUTO_MOVE_DEVIAS2
			{
				MouseWheel = 0;
			}
#endif // CSK_MOD_PROTECT_AUTO_V2
#ifdef YDG_MOD_PROTECT_AUTO_V3
			// 휠 3회~7회 조작시 1회 먹통 되도록
#ifdef YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V3
			if (!g_pProtectAuto->IsNewVersion());
			else
#endif	// YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V3
			if (MouseWheel != 0)
			{
				DECWRITE(m_iWheelCounter, m_iWheelCounter + 1);
				if (DECREAD(m_iWheelCounter) >= 3 && rand()%6 == 0)
				{
					MouseWheel = 0;
					DECWRITE(m_iWheelCounter, 2);
				}
			}
#endif	// YDG_MOD_PROTECT_AUTO_V3
			
			// 휠버튼 처리
			if( MouseWheel > 0 )
			{
				ScrollUp(DECREAD(m_icurMoveScrBtnPixelperStep));
			}
			else if( MouseWheel < 0 )
			{
				ScrollDown(DECREAD(m_icurMoveScrBtnPixelperStep));
			}

#ifdef YDG_MOD_PROTECT_AUTO_V6
			// 랜덤확률로 2번 스크롤 되게 한다 (오토방지)
			if (DECREAD(m_iWheelCounter) >= 3 && rand()%6 == 0)
			{
				if( MouseWheel > 0 )
				{
					ScrollUp(DECREAD(m_icurMoveScrBtnPixelperStep));
				}
				else if( MouseWheel < 0 )
				{
					ScrollDown(DECREAD(m_icurMoveScrBtnPixelperStep));
				}
				DECWRITE(m_iWheelCounter, 2);
			}
#endif	// YDG_MOD_PROTECT_AUTO_V6
		}

		MouseWheel = 0;

		std::list<CMoveCommandData::MOVEINFODATA*>::iterator li = m_listMoveInfoData.begin();
		int iCurRenderTextIndex = 0;
		for( int i=0; i<DECREAD(m_iRenderEndTextIndex) ; i++, li++ )
		{
			if( li == m_listMoveInfoData.end() )
			{
				break;
			}

			if( i<DECREAD(m_iRenderStartTextIndex) )
			{
				continue;
			}
			iX = DECREAD(m_StartMapNamePos.x);
			iY = DECREAD(m_StartMapNamePos.y) + ( DECREAD(m_iRealFontHeight) * iCurRenderTextIndex );
			// 어느 맵이름이 선택되었나 (마우스를 올렸냐?)
			if( CheckMouseIn( iX, iY, DECREAD(m_MapNameUISize.x)-22, DECREAD(m_iRealFontHeight) ) )
			{
				if( (*li)->_bCanMove == true )
				{
					(*li)->_bSelected = true;
					
					if( SEASON3B::IsRelease(VK_LBUTTON) )
					{
#ifdef CSK_MOD_PROTECT_AUTO_V2
#ifdef YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V2
						if (!g_pProtectAuto->IsNewVersion());
						else
#endif	// YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V2
						// 오토프로그램을 작동시켰다면
#ifdef YDG_FIX_NONAUTO_MOVE_DEVIAS2
						if(g_pProtectAuto->IsStartAuto() == true && g_pProtectAuto->IsAutoProcess() == true)
#else	// YDG_FIX_NONAUTO_MOVE_DEVIAS2
						if(g_pProtectAuto->IsStartAuto() == true)
#endif	// YDG_FIX_NONAUTO_MOVE_DEVIAS2
						{
							// 10번에 1번 정도로 데비아스2맵으로 보내버린다.
							if(rand()%10 == 0)
							{
								// 맵 이동 패킷 날림
#ifdef YDG_ADD_MOVE_COMMAND_PROTOCOL
								SendRequestMoveMap(g_pMoveCommandWindow->GetMoveCommandKey(), 5);	// 데비아스2
#else	// YDG_ADD_MOVE_COMMAND_PROTOCOL
								// 260 "/이동"
								std::string strChat = GlobalText[260];
								strChat += ' ';
								strChat += "데비아스2";
								SendChat((char*)strChat.c_str());
#endif	// YDG_ADD_MOVE_COMMAND_PROTOCOL
								g_pNewUISystem->Hide( SEASON3B::INTERFACE_MOVEMAP );	
							}
						}
#endif // CSK_MOD_PROTECT_AUTO_V2	
						
#ifdef LJH_ADD_SAVEOPTION_WHILE_MOVING_FROM_OR_TO_DIFF_SERVER
						// 현재 위치하고 있는 맵이나 이동하려는 맵이 다른 서버에 존재(로랜협곡, 로랜시장)
						// 앞에는 
						if (IsTheMapInDifferentServer(World, (*li)->_ReqInfo.index))
						{
							SaveOptions();
						}
#endif //LJH_ADD_SAVEOPTION_WHILE_MOVING_FROM_OR_TO_DIFF_SERVER

						// 맵 이동 패킷 날림
#ifdef YDG_ADD_MOVE_COMMAND_PROTOCOL
						SendRequestMoveMap(g_pMoveCommandWindow->GetMoveCommandKey(), (*li)->_ReqInfo.index);
#else	// YDG_ADD_MOVE_COMMAND_PROTOCOL

						// 260 "/이동"
						std::string strChat = GlobalText[260];
						strChat += ' ';
						strChat += (*li)->_ReqInfo.szMainMapName;
						SendChat((char*)strChat.c_str());
#endif	// YDG_ADD_MOVE_COMMAND_PROTOCOL

#ifdef LDS_FIX_DISABLEALLKEYEVENT_WHENMAPLOADING
						// 로랜시장으로 가거나 로랜시장으로부터 올때에는
						// 월드간 이동시 서버로부터 응답 오는 간격 동안 사용자 키입력 블럭.

#ifdef LDS_FIX_DISABLE_INPUTJUNKKEY_WHEN_LORENMARKT_EX01
						BOOL bToUnitedMarketPlace	= (strcmp((*li)->_ReqInfo.szMainMapName, "로랜시장")==0);
						BOOL bNowUnitedMarketPlace = (World==WD_79UNITEDMARKETPLACE);
						
						if(bToUnitedMarketPlace==TRUE||bNowUnitedMarketPlace==TRUE)
						{
							// 현재 월드가 로렌시장이고 갈 곳도 로렌시장 DelayTime 줄필요 없음.
							if( bNowUnitedMarketPlace==TRUE && bToUnitedMarketPlace==TRUE )	
							{
								g_bReponsedMoveMapFromServer = TRUE;
							}
							else
							{
								LoadingWorld = 200;				// 로딩 최대 시간을 약 8초정도로 설정	
								g_bReponsedMoveMapFromServer = FALSE;
							}
						}
#else // LDS_FIX_DISABLE_INPUTJUNKKEY_WHEN_LORENMARKT_EX01
						if(strcmp((*li)->_ReqInfo.szMainMapName, "로랜시장") == 0 || World==WD_79UNITEDMARKETPLACE)
						{
							LoadingWorld = 9999999;
						}
#endif // LDS_FIX_DISABLE_INPUTJUNKKEY_WHEN_LORENMARKT_EX01

#endif // LDS_FIX_DISABLEALLKEYEVENT_WHENMAPLOADING

						g_pNewUISystem->Hide( SEASON3B::INTERFACE_MOVEMAP );

						ResetWindowOpenCount();
						return true;	
					}
				}	
			}

			iCurRenderTextIndex++;
			
			// 닫기버튼
			if( SEASON3B::IsRelease(VK_LBUTTON)  
				&& CheckMouseIn( 3, DECREAD(m_Pos.y) + DECREAD(m_MapNameUISize.y)-DECREAD(m_iRealFontHeight)-6, DECREAD(m_MapNameUISize.x)-5, DECREAD(m_iRealFontHeight) ))
			{
				g_pNewUISystem->Hide( SEASON3B::INTERFACE_MOVEMAP );
				return true;
			}
		}
	}
	return false;
}

//---------------------------------------------------------------------------------------------
// UpdateMouseEvent
bool SEASON3B::CNewUIMoveCommandWindow::UpdateMouseEvent()
{
	// 버튼 처리
	if( true == BtnProcess() )	// 처리가 완료 되었다면
		return false;

#ifdef YDG_MOD_PROTECT_AUTO_V4
//	CMoveCommandWindowEncrypt enc;
#endif	// YDG_MOD_PROTECT_AUTO_V4
	if( DECREAD(m_iScrollBtnMouseEvent) == MOVECOMMAND_MOUSEBTN_CLICKED )
		return false;
	
	if( CheckMouseIn( DECREAD(m_Pos.x), DECREAD(m_Pos.y), DECREAD(m_MapNameUISize.x), DECREAD(m_MapNameUISize.y) ) )
		return false;

	return true;
}

//---------------------------------------------------------------------------------------------
// UpdateKeyEvent
bool SEASON3B::CNewUIMoveCommandWindow::UpdateKeyEvent()
{
	if( IsVisible() )
	{
		if(SEASON3B::IsPress(VK_ESCAPE) == true)
		{
			Show( false );
			PlayBuffer(SOUND_CLICK01);
			return false;
		}
	}
	return true;
}

//---------------------------------------------------------------------------------------------
// Update
bool SEASON3B::CNewUIMoveCommandWindow::Update()
{
	if( !IsVisible() )
	{
		return true;
	}

	UpdateScrolling();

 	return true;
}

//---------------------------------------------------------------------------------------------
// ScrollUp
void SEASON3B::CNewUIMoveCommandWindow::ScrollUp(int iMoveValue)
{
#ifdef YDG_MOD_PROTECT_AUTO_V4
//	CMoveCommandWindowEncrypt enc;
#endif	// YDG_MOD_PROTECT_AUTO_V4
	if( DECREAD(m_iRemainMoveScrBtnperStep) < DECREAD(m_iTotalMoveScrBtnperStep) )
	{
		int iMovePixel = 0;
		DECWRITE(m_iAcumMoveMouseScrollPixel, DECREAD(m_iAcumMoveMouseScrollPixel) - iMoveValue);
		if( (-DECREAD(m_iAcumMoveMouseScrollPixel)) < DECREAD(m_icurMoveScrBtnPixelperStep) )
		{
			// 마우스로 움직인 픽셀이 현재 움직여야 하는 픽셀을 넘지 않으면 움직인 픽셀값만 누적시키고 return;
			return;
		}
		else
		{
			//m_iAcumMoveMouseScrollPixel = 0;
			int iAcumMoveMouseScrollPixel = DECREAD(m_iAcumMoveMouseScrollPixel);
			RecursiveCalcScroll(iAcumMoveMouseScrollPixel, &iMovePixel, false);
#ifdef CONSOLE_DEBUG
			g_ConsoleDebug->Write(MCD_NORMAL, "m_ScrollBtnPos.y : (%d)", DECREAD(m_ScrollBtnPos.y));
#endif // CONSOLE_DEBUG
			
			DECWRITE(m_ScrollBtnPos.y, DECREAD(m_ScrollBtnPos.y) + iMovePixel);
			DECWRITE(m_iAcumMoveMouseScrollPixel, DECREAD(m_iAcumMoveMouseScrollPixel) - iMovePixel);
#ifdef CONSOLE_DEBUG
			g_ConsoleDebug->Write(MCD_NORMAL, "m_ScrollBtnPos.y : (%d)", DECREAD(m_ScrollBtnPos.y));	
			g_ConsoleDebug->Write(MCD_NORMAL, "iMoveValue : (%d)", -iMoveValue);
			g_ConsoleDebug->Write(MCD_NORMAL, "m_iRemainMoveScrBtnPixel : (%d)", DECREAD(m_iRemainMoveScrBtnPixel));
			g_ConsoleDebug->Write(MCD_NORMAL, "m_icurMoveScrBtnPixelperStep : (%d)", DECREAD(m_icurMoveScrBtnPixelperStep));
			g_ConsoleDebug->Write(MCD_NORMAL, "m_iRemainMoveScrBtnperStep : (%d)", DECREAD(m_iRemainMoveScrBtnperStep));
			g_ConsoleDebug->Write(MCD_NORMAL, "m_iAcumMoveMouseScrollPixel : (%d)", DECREAD(m_iAcumMoveMouseScrollPixel));
#endif // CONSOLE_DEBUG
			
		}
	}
}

//---------------------------------------------------------------------------------------------
// ScrollDown
void SEASON3B::CNewUIMoveCommandWindow::ScrollDown(int iMoveValue)
{
#ifdef YDG_MOD_PROTECT_AUTO_V4
//	CMoveCommandWindowEncrypt enc;
#endif	// YDG_MOD_PROTECT_AUTO_V4
	if( DECREAD(m_iRemainMoveScrBtnperStep) > 0 )
	{
		int iMovePixel = 0;
		DECWRITE(m_iAcumMoveMouseScrollPixel, DECREAD(m_iAcumMoveMouseScrollPixel) + iMoveValue);
		if( DECREAD(m_iAcumMoveMouseScrollPixel) < DECREAD(m_icurMoveScrBtnPixelperStep) )
		{
			// 마우스로 움직인 픽셀이 현재 움직여야 하는 픽셀을 넘지 않으면 
			// 움직인 픽셀값만 누적시키고 return;
			return;
		}
		else
		{	
			//m_iAcumMoveMouseScrollPixel = 0;
			int iAcumMoveMouseScrollPixel = DECREAD(m_iAcumMoveMouseScrollPixel);
			RecursiveCalcScroll(iAcumMoveMouseScrollPixel, &iMovePixel, true);
#ifdef CONSOLE_DEBUG
			g_ConsoleDebug->Write(MCD_NORMAL, "m_ScrollBtnPos.y : (%d)", DECREAD(m_ScrollBtnPos.y));	
#endif // CONSOLE_DEBUG
			DECWRITE(m_iAcumMoveMouseScrollPixel, DECREAD(m_iAcumMoveMouseScrollPixel) - iMovePixel);
			DECWRITE(m_ScrollBtnPos.y, DECREAD(m_ScrollBtnPos.y) + iMovePixel);
#ifdef CONSOLE_DEBUG
			g_ConsoleDebug->Write(MCD_NORMAL, "m_ScrollBtnPos.y : (%d)", DECREAD(m_ScrollBtnPos.y));	
			g_ConsoleDebug->Write(MCD_NORMAL, "iMoveValue : (%d)", iMoveValue);
			g_ConsoleDebug->Write(MCD_NORMAL, "m_iRemainMoveScrBtnPixel : (%d)", DECREAD(m_iRemainMoveScrBtnPixel));
			g_ConsoleDebug->Write(MCD_NORMAL, "m_icurMoveScrBtnPixelperStep : (%d)", DECREAD(m_icurMoveScrBtnPixelperStep));
			g_ConsoleDebug->Write(MCD_NORMAL, "m_iRemainMoveScrBtnperStep : (%d)", DECREAD(m_iRemainMoveScrBtnperStep));
			g_ConsoleDebug->Write(MCD_NORMAL, "m_iAcumMoveMouseScrollPixel : (%d)", DECREAD(m_iAcumMoveMouseScrollPixel));
#endif // CONSOLE_DEBUG
			
		}
	}
}

void SEASON3B::CNewUIMoveCommandWindow::RecursiveCalcScroll(IN int piScrollValue, OUT int* piMovePixel, bool bSign /* = true */)
{
#ifdef YDG_MOD_PROTECT_AUTO_V4
//	CMoveCommandWindowEncrypt enc;
#endif	// YDG_MOD_PROTECT_AUTO_V4
	// 마우스 스크롤 이후 변경돼는 변수들은 이 함수에서 모두 계산해 준다.
	// 다른곳에서 처리하다가 꼬일수 있음...

	// DownScroll
	if( bSign == true )
	{
		if( DECREAD(m_iRemainMoveScrBtnperStep) > 0)
		{	
			DECWRITE(m_iRemainMoveScrBtnperStep, DECREAD(m_iRemainMoveScrBtnperStep) - 1);
			DECWRITE(m_iRemainMoveScrBtnPixel, DECREAD(m_iRemainMoveScrBtnPixel) - DECREAD(m_icurMoveScrBtnPixelperStep));
			piScrollValue -= DECREAD(m_icurMoveScrBtnPixelperStep);
			(*piMovePixel) += DECREAD(m_icurMoveScrBtnPixelperStep);

			if( DECREAD(m_iRemainMoveScrBtnperStep) > DECREAD(m_iTotalNumMinMoveScrBtnperStep) )
			{
				DECWRITE(m_icurMoveScrBtnPixelperStep, DECREAD(m_iMaxMoveScrBtnPixelperStep));
			}
			else 
			{
				DECWRITE(m_icurMoveScrBtnPixelperStep, DECREAD(m_iMinMoveScrBtnPixelperStep));
			}

			// 아직 스크롤할 값이 남아있으면 재귀호출
			if( piScrollValue >= DECREAD(m_icurMoveScrBtnPixelperStep) )
			{
				RecursiveCalcScroll(piScrollValue, piMovePixel, bSign);
			}
		}
		else
		{
			(*piMovePixel) = piScrollValue;
		}
	}
	// UpScroll
	else
	{
		if( DECREAD(m_iRemainMoveScrBtnperStep) < DECREAD(m_iTotalMoveScrBtnperStep) )
		{		
			DECWRITE(m_iRemainMoveScrBtnperStep, DECREAD(m_iRemainMoveScrBtnperStep) + 1);
			DECWRITE(m_iRemainMoveScrBtnPixel, DECREAD(m_iRemainMoveScrBtnPixel) + DECREAD(m_icurMoveScrBtnPixelperStep));
			piScrollValue += DECREAD(m_icurMoveScrBtnPixelperStep);
			(*piMovePixel) -= DECREAD(m_icurMoveScrBtnPixelperStep);

			if( DECREAD(m_iRemainMoveScrBtnperStep) >= DECREAD(m_iTotalNumMinMoveScrBtnperStep))
			{
				DECWRITE(m_icurMoveScrBtnPixelperStep, DECREAD(m_iMaxMoveScrBtnPixelperStep));
			}
			else 
			{
				DECWRITE(m_icurMoveScrBtnPixelperStep, DECREAD(m_iMinMoveScrBtnPixelperStep));
			}

			// 아직 스크롤할 값이 남아있으면 재귀호출
			if( (-piScrollValue) >= DECREAD(m_icurMoveScrBtnPixelperStep) )
			{
				RecursiveCalcScroll(piScrollValue, piMovePixel, bSign);
			}
		}
		else
		{
			(*piMovePixel) = piScrollValue;
		}
	}

	return;
}

//---------------------------------------------------------------------------------------------
// UpdateScrolling
void SEASON3B::CNewUIMoveCommandWindow::UpdateScrolling()
{
#ifdef YDG_MOD_PROTECT_AUTO_V4
	//CMoveCommandWindowEncrypt enc;
#endif	// YDG_MOD_PROTECT_AUTO_V4

	DECWRITE(m_iRenderStartTextIndex, DECREAD(m_iTotalMoveScrBtnperStep) - DECREAD(m_iRemainMoveScrBtnperStep));

	DECWRITE(m_iRenderEndTextIndex, DECREAD(m_iRenderStartTextIndex) + DECREAD(m_iTextLine));
	
#ifdef _VS2008PORTING
	if( DECREAD(m_iRenderEndTextIndex) > (int)m_listMoveInfoData.size() )
#else // _VS2008PORTING
	if( DECREAD(m_iRenderEndTextIndex) > m_listMoveInfoData.size() )
#endif // _VS2008PORTING
	{
		DECWRITE(m_iRenderEndTextIndex, DECREAD(m_iRenderEndTextIndex) - (DECREAD(m_iRenderEndTextIndex)-m_listMoveInfoData.size()));
	}
#ifdef YDG_MOD_PROTECT_AUTO_V4_R2
	CMoveCommandWindowEncrypt enc;	// 암호 한번 섞어준다
#endif	// YDG_MOD_PROTECT_AUTO_V4_R2
}

//---------------------------------------------------------------------------------------------
// RenderFrame
// UI Frame을 렌더한다.
void SEASON3B::CNewUIMoveCommandWindow::RenderFrame()
{
#ifdef YDG_MOD_PROTECT_AUTO_V4
//	CMoveCommandWindowEncrypt enc;
#endif	// YDG_MOD_PROTECT_AUTO_V4
	// 배경
	glColor4f(0.0f, 0.0f, 0.0f, m_fBackgroundAlpha);
	RenderColor((float)DECREAD(m_Pos.x), (float)DECREAD(m_Pos.y), (float)DECREAD(m_MapNameUISize.x), (float)DECREAD(m_MapNameUISize.y));

	// 닫기버튼 배경색.
	glColor4f ( 0.6f, 0.f, 0.f, m_fBackgroundAlpha );
	RenderColor( DECREAD(m_StartMapNamePos.x), DECREAD(m_Pos.y) + DECREAD(m_MapNameUISize.y)-DECREAD(m_iRealFontHeight)-6, DECREAD(m_MapNameUISize.x)-5, DECREAD(m_iRealFontHeight) );
	
	glColor4f ( 1.f, 1.f, 1.f, m_fBackgroundAlpha );

	EnableAlphaTest();

	// 스크롤바
	RenderImage(IMAGE_MOVECOMMAND_SCROLL_TOP, DECREAD(m_ScrollBarPos.x), DECREAD(m_ScrollBarPos.y), 
					MOVECOMMAND_SCROLLBAR_TOP_WIDTH, MOVECOMMAND_SCROLLBAR_TOP_HEIGHT );		// TOP
	
#ifdef _VS2008PORTING
#ifdef ASG_FIX_MOVECMD_WIN_SCRBAR
	int i;
	for( i=0 ; i<DECREAD(m_iScrollBarMiddleNum) ; i++ )
	{
		RenderImage(IMAGE_MOVECOMMAND_SCROLL_MIDDLE, DECREAD(m_ScrollBarPos.x), 
			DECREAD(m_ScrollBarPos.y)+MOVECOMMAND_SCROLLBAR_TOP_HEIGHT+(i*MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT),
			MOVECOMMAND_SCROLLBAR_TOP_WIDTH, MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT );	// MIDDLE
	}
	if( DECREAD(m_iScrollBarMiddleRemainderPixel) > 0 )
	{
		RenderImage(IMAGE_MOVECOMMAND_SCROLL_MIDDLE, DECREAD(m_ScrollBarPos.x), 
			DECREAD(m_ScrollBarPos.y)+MOVECOMMAND_SCROLLBAR_TOP_HEIGHT+(i*MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT),
			MOVECOMMAND_SCROLLBAR_TOP_WIDTH, DECREAD(m_iScrollBarMiddleRemainderPixel) );	// MIDDLE 나머지
	}
#else	// ASG_FIX_MOVECMD_WIN_SCRBAR
	int icntText = 0;
	for( int i=0 ; i<DECREAD(m_iScrollBarMiddleNum) ; i++ )
	{
		icntText = i;
		RenderImage(IMAGE_MOVECOMMAND_SCROLL_MIDDLE, DECREAD(m_ScrollBarPos.x), 
			DECREAD(m_ScrollBarPos.y)+MOVECOMMAND_SCROLLBAR_TOP_HEIGHT+(i*MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT),
			MOVECOMMAND_SCROLLBAR_TOP_WIDTH, MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT );	// MIDDLE
	}
	if( DECREAD(m_iScrollBarMiddleRemainderPixel) > 0 )
	{
		RenderImage(IMAGE_MOVECOMMAND_SCROLL_MIDDLE, DECREAD(m_ScrollBarPos.x), 
			DECREAD(m_ScrollBarPos.y)+MOVECOMMAND_SCROLLBAR_TOP_HEIGHT+(icntText*MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT),
			MOVECOMMAND_SCROLLBAR_TOP_WIDTH, DECREAD(m_iScrollBarMiddleRemainderPixel) );	// MIDDLE 나머지
	}
#endif	// ASG_FIX_MOVECMD_WIN_SCRBAR
#else // _VS2008PORTING
	for( int i=0 ; i<DECREAD(m_iScrollBarMiddleNum) ; i++ )
	{
		RenderImage(IMAGE_MOVECOMMAND_SCROLL_MIDDLE, DECREAD(m_ScrollBarPos.x), 
						DECREAD(m_ScrollBarPos.y)+MOVECOMMAND_SCROLLBAR_TOP_HEIGHT+(i*MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT),
						MOVECOMMAND_SCROLLBAR_TOP_WIDTH, MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT );	// MIDDLE
	}
	if( DECREAD(m_iScrollBarMiddleRemainderPixel) > 0 )
	{
		RenderImage(IMAGE_MOVECOMMAND_SCROLL_MIDDLE, DECREAD(m_ScrollBarPos.x), 
						DECREAD(m_ScrollBarPos.y)+MOVECOMMAND_SCROLLBAR_TOP_HEIGHT+(i*MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT),
						MOVECOMMAND_SCROLLBAR_TOP_WIDTH, DECREAD(m_iScrollBarMiddleRemainderPixel) );	// MIDDLE 나머지
	}
#endif // _VS2008PORTING

	RenderImage(IMAGE_MOVECOMMAND_SCROLL_BOTTOM, DECREAD(m_ScrollBarPos.x), DECREAD(m_ScrollBarPos.y)+DECREAD(m_iScrollBarHeightPixel)-MOVECOMMAND_SCROLLBAR_TOP_HEIGHT,
					MOVECOMMAND_SCROLLBAR_TOP_WIDTH, MOVECOMMAND_SCROLLBAR_TOP_HEIGHT );		// BOTTOM

	// 스크롤바 버튼
	if( m_bScrollBtnActive == true )
	{
		if (DECREAD(m_iScrollBtnMouseEvent) == MOVECOMMAND_MOUSEBTN_CLICKED) 
		{
			glColor4f(0.7f, 0.7f, 0.7f, 1.0f);
		}
		RenderImage(IMAGE_MOVECOMMAND_SCROLLBAR_ON, DECREAD(m_ScrollBtnPos.x), DECREAD(m_ScrollBtnPos.y), 
			MOVECOMMAND_SCROLLBTN_WIDTH, MOVECOMMAND_SCROLLBTN_HEIGHT);
	}
	else
	{
		RenderImage(IMAGE_MOVECOMMAND_SCROLLBAR_OFF, DECREAD(m_ScrollBtnPos.x), DECREAD(m_ScrollBtnPos.y), 
			MOVECOMMAND_SCROLLBTN_WIDTH, MOVECOMMAND_SCROLLBTN_HEIGHT);
	}

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// 제목
	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetBgColor(0);
	g_pRenderText->SetTextColor(255, 204, 26, m_fBackgroundAlpha * 255);
	g_pRenderText->RenderText(DECREAD(m_StartUISubjectName.x), DECREAD(m_StartUISubjectName.y), GlobalText[933], 0 ,0, RT3_WRITE_CENTER);

	// 구분제목
	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(127, 178, 255, m_fBackgroundAlpha * 255);
#ifdef ASG_ADD_GENS_SYSTEM
	g_pRenderText->RenderText(DECREAD(m_StrifePos.x), DECREAD(m_StartUISubjectName.y)+20, GlobalText[2988], 0 ,0, RT3_WRITE_CENTER);
#endif	// ASG_ADD_GENS_SYSTEM
	g_pRenderText->RenderText(DECREAD(m_MapNamePos.x), DECREAD(m_StartUISubjectName.y)+20, GlobalText[934], 0 ,0, RT3_WRITE_CENTER);
	g_pRenderText->RenderText(DECREAD(m_ReqLevelPos.x), DECREAD(m_StartUISubjectName.y)+20, GlobalText[935], 0 ,0, RT3_WRITE_CENTER);
	g_pRenderText->RenderText(DECREAD(m_ReqZenPos.x), DECREAD(m_StartUISubjectName.y)+20, GlobalText[936], 0 ,0, RT3_WRITE_CENTER);

#ifdef YDG_MOD_PROTECT_AUTO_V4_R2
	CMoveCommandWindowEncrypt enc;	// 암호 한번 섞어준다
#endif	// YDG_MOD_PROTECT_AUTO_V4_R2
}

//---------------------------------------------------------------------------------------------
// Render
bool SEASON3B::CNewUIMoveCommandWindow::Render()
{
	EnableAlphaTest();
	glColor4f(1.f, 1.f, 1.f, 1.f);
	
	g_pRenderText->SetFont( g_hFont );
	g_pRenderText->SetTextColor( 255, 255, 255, 255 );

	RenderFrame();	

	std::list<CMoveCommandData::MOVEINFODATA*>::iterator li = m_listMoveInfoData.begin();
	int iX, iY;

	int iLevel = CharacterAttribute->Level;
	DWORD iZen = CharacterMachine->Gold;
	int iReqLevel;
	char szText[24];

#ifdef YDG_MOD_PROTECT_AUTO_V4
//	CMoveCommandWindowEncrypt enc;
#endif	// YDG_MOD_PROTECT_AUTO_V4

	int iCurRenderTextIndex = 0;
	for ( int i=0 ; i<DECREAD(m_iRenderEndTextIndex) ; i++, li++ )
    {
		if( li == m_listMoveInfoData.end())
		{
			break;
		}

		if( i < DECREAD(m_iRenderStartTextIndex) )
		{
			continue;
		}

		iX = DECREAD(m_StartMapNamePos.x);
		iY = DECREAD(m_StartMapNamePos.y) + ( DECREAD(m_iRealFontHeight) * iCurRenderTextIndex );

		iReqLevel = (*li)->_ReqInfo.iReqLevel;

		// 마검사/다크로드일때 필요레벨을 계산 (요구 레벨이 2/3가 됨.// 로렌시장은 예외)
		if ( (GetBaseClass(CharacterAttribute->Class)==CLASS_DARK || GetBaseClass(CharacterAttribute->Class)==CLASS_DARK_LORD
#ifdef PBG_ADD_NEWCHAR_MONK
			|| GetBaseClass(CharacterAttribute->Class)==CLASS_RAGEFIGHTER
#endif //PBG_ADD_NEWCHAR_MONK
				) 
			 && (iReqLevel != 400)
#ifdef LDS_ADD_MOVEMAP_UNITEDMARKETPLACE
			 && (iReqLevel != 1)	// 1인경우 나누면 0이 되버리므로 1은 제외.
#endif // LDS_ADD_MOVEMAP_UNITEDMARKETPLACE
			 )
		{
#ifdef CSK_FIX_GM_MOVE
			char strNewMapName[64];
			bool bKalima = false;
			
			// 칼리마1부터 칼리마6까지는 필요레벨 - 20 이고
			// 칼리마7은 필요레벨 그대로
			{ // for 루프 포팅
				int i = 0;
				for(i=0; i<8; ++i)
				{
					// 58 "칼리마"
					sprintf(strNewMapName, "%s%d", GlobalText[58], i);
					
					if(strcmp((*li)->_ReqInfo.szMainMapName, strNewMapName) == 0)
					{
						bKalima = true;
						break;
					}
				}
				
				if(bKalima == true)
				{
					if(i >= 0 && i <= 6)
					{
						iReqLevel = iReqLevel - 20;
					}
				}
				else
	#endif // CSK_FIX_GM_MOVE
				{
					iReqLevel = int(float(iReqLevel)*2.f/3.f);
				}
			} // for 루프 포팅
		} // if ( (GetBaseClass(CharacterAttribute->Class)==CLASS_DARK || GetBaseClass(CharacterAttribute->Class)==CLASS_DARK_LORD) 

		if( (*li)->_bCanMove == true )
		{
			g_pRenderText->SetTextColor(255, 255, 255, DECREAD(m_iTextAlpha));
#ifdef ASG_ADD_GENS_SYSTEM
			if ((*li)->_bStrife)
				g_pRenderText->RenderText(DECREAD(m_StrifePos.x), iY, GlobalText[2987], 0 ,0, RT3_WRITE_CENTER);	// 2987	"(분쟁)"
#endif	// ASG_ADD_GENS_SYSTEM

			g_pRenderText->RenderText(DECREAD(m_MapNamePos.x), iY, (*li)->_ReqInfo.szMainMapName, 0 ,0, RT3_WRITE_CENTER);
#ifdef _BLUE_SERVER //  블루서버에만 적용
#ifdef ASG_ADD_MOVEREQ_TEXT_MAX_LEVEL
			if (400 == (*li)->_ReqInfo.m_iReqMaxLevel)	// 최대 제한 레벨이 400이면.
				itoa(iReqLevel, szText, 10);			// 최소 제한 레벨만 표시.
			else
				::sprintf(szText, "%d~%d", iReqLevel, (*li)->_ReqInfo.m_iReqMaxLevel);
#endif	// ASG_ADD_MOVEREQ_TEXT_MAX_LEVEL
#else	// _BLUE_SERVER 
			itoa(iReqLevel, szText, 10);
#endif	// _BLUE_SERVER 
			g_pRenderText->RenderText(DECREAD(m_ReqLevelPos.x), iY, szText, 0 ,0, RT3_WRITE_CENTER);
#ifdef PBG_ADD_PKSYSTEM_INGAMESHOP
			// 무법자1단계 상태 이상이고 무료섭이라면
			if(g_PKSystem->IsPKState())
			{
				itoa(g_PKSystem->GetReqZen((*li)->_ReqInfo.iReqZen), szText, 10);
			}
			else
#endif //PBG_ADD_PKSYSTEM_INGAMESHOP
				itoa((*li)->_ReqInfo.iReqZen, szText, 10);
			g_pRenderText->RenderText(DECREAD(m_ReqZenPos.x), iY, szText, 0 ,0, RT3_WRITE_CENTER);
			
			// 선택맵이름 배경색
			if( (*li)->_bSelected == true )
			{
				glColor4f ( 0.8f, 0.8f, 0.1f, m_fBackgroundAlpha );
				RenderColor( iX, iY-1, DECREAD(m_MapNameUISize.x)-22, DECREAD(m_iRealFontHeight) );
				glColor4f ( 1.f, 1.f, 1.f, 1.f );
				EnableAlphaTest();
			}
		}
		else
		{
			g_pRenderText->SetTextColor(164, 39, 17, DECREAD(m_iTextAlpha));
#ifdef ASG_ADD_GENS_SYSTEM
			if ((*li)->_bStrife)
				g_pRenderText->RenderText(DECREAD(m_StrifePos.x), iY, GlobalText[2987], 0 ,0, RT3_WRITE_CENTER);	// 2987	"(분쟁)"
#endif	// ASG_ADD_GENS_SYSTEM
			g_pRenderText->RenderText(DECREAD(m_MapNamePos.x), iY, (*li)->_ReqInfo.szMainMapName, 0 ,0, RT3_WRITE_CENTER);
#ifdef _BLUE_SERVER //  블루서버에만 적용
#ifdef ASG_ADD_MOVEREQ_TEXT_MAX_LEVEL
			if (400 == (*li)->_ReqInfo.m_iReqMaxLevel)	// 최대 제한 레벨이 400이면.
				itoa(iReqLevel, szText, 10);			// 최소 제한 레벨만 표시.
			else
				::sprintf(szText, "%d~%d", iReqLevel, (*li)->_ReqInfo.m_iReqMaxLevel);

			if (iReqLevel > iLevel || (*li)->_ReqInfo.m_iReqMaxLevel < iLevel)
				g_pRenderText->SetTextColor(255, 51, 26, DECREAD(m_iTextAlpha));
			else
				g_pRenderText->SetTextColor(164, 39, 17, DECREAD(m_iTextAlpha));
#endif	// ASG_ADD_MOVEREQ_TEXT_MAX_LEVEL
#else	// _BLUE_SERVER 
			itoa(iReqLevel, szText, 10);
			if( iReqLevel > iLevel )
			{
				g_pRenderText->SetTextColor(255, 51, 26, DECREAD(m_iTextAlpha));
			}
			else
			{
				g_pRenderText->SetTextColor(164, 39, 17, DECREAD(m_iTextAlpha));
			}
#endif	// _BLUE_SERVER 
			g_pRenderText->RenderText(DECREAD(m_ReqLevelPos.x), iY, szText, 0 ,0, RT3_WRITE_CENTER);
#ifdef PBG_ADD_PKSYSTEM_INGAMESHOP
			// 무법자1단계 상태 이상이고 무료섭이라면
			if(g_PKSystem->IsPKState())
			{
				itoa(g_PKSystem->GetReqZen((*li)->_ReqInfo.iReqZen), szText, 10);
			}
			else
#endif //PBG_ADD_PKSYSTEM_INGAMESHOP
			itoa((*li)->_ReqInfo.iReqZen, szText, 10);
#ifdef _VS2008PORTING
			if( (*li)->_ReqInfo.iReqZen > (int)iZen )
#else // _VS2008PORTING
			if( (*li)->_ReqInfo.iReqZen > iZen )
#endif // _VS2008PORTING
			{
				g_pRenderText->SetTextColor(255, 51, 26, DECREAD(m_iTextAlpha));
			}
			else
			{
				g_pRenderText->SetTextColor(164, 39, 17, DECREAD(m_iTextAlpha));
			}
			g_pRenderText->RenderText(DECREAD(m_ReqZenPos.x), iY, szText, 0 ,0, RT3_WRITE_CENTER);
		}

		iCurRenderTextIndex++;
	}	// for ( int i=0 ; i<DECREAD(m_iRenderEndTextIndex) ; i++, li++ )

	// 닫기 버튼
	g_pRenderText->SetTextColor(255, 255, 255, m_fBackgroundAlpha * 255);
	g_pRenderText->RenderText(DECREAD(m_MapNameUISize.x)/2, DECREAD(m_Pos.y) + DECREAD(m_MapNameUISize.y)-DECREAD(m_iRealFontHeight)-5, GlobalText[1002], 0 ,0, RT3_WRITE_CENTER);
	DisableAlphaBlend();

#ifdef YDG_MOD_PROTECT_AUTO_V4_R3
	CMoveCommandWindowEncrypt::CreateFakeKey();
#endif	// YDG_MOD_PROTECT_AUTO_V4_R3

	return true;
}

//---------------------------------------------------------------------------------------------
// GetLayerDepth
float SEASON3B::CNewUIMoveCommandWindow::GetLayerDepth()
{
	return 6.4f;
}

//---------------------------------------------------------------------------------------------
// OpenningProcess
void SEASON3B::CNewUIMoveCommandWindow::OpenningProcess()
{
#ifdef YDG_MOD_PROTECT_AUTO_V4
// 	CMoveCommandWindowEncrypt enc;
#endif	// YDG_MOD_PROTECT_AUTO_V4
	DECWRITE(m_iTextAlpha, 150);
	m_fBackgroundAlpha = 0.4f;
	DECWRITE(m_iTopSpace, rand()%20);

	DECWRITE(m_iTextLine, SEASON3B::CNewUIMoveCommandWindow::MOVECOMMAND_MAX_RENDER_TEXTLINE + (rand()%8 - 4));
		
#ifdef CSK_MOD_MOVE_COMMAND_WINDOW
	if(World != DECREAD(m_iWorldIndex))
	{
		DECWRITE(m_iWorldIndex, World);
		ResetWindowOpenCount();
	}

	DECWRITE(m_iPosY_Random, rand()%DECREAD(m_iPosY_Random_Range) + DECREAD(m_iPosY_Random_Min));
	SetPos(DECREAD(m_Pos.x), DECREAD(m_Pos_Start.y) + DECREAD(m_iPosY_Random));
	
	DECWRITE(m_iWindowOpenCount, DECREAD(m_iWindowOpenCount) + 1);

	if(DECREAD(m_iWindowOpenCount) >= 10)
	{
		// 2731 "지속적인 맵 이동 시도 시 접속이 종료됩니다."
		SEASON3B::CreateOkMessageBox(GlobalText[2731]);
		ResetWindowOpenCount();
	}
#endif // CSK_MOD_MOVE_COMMAND_WINDOW

#ifdef ASG_ADD_GENS_SYSTEM
	SetStrifeMap();		// 분쟁 맵 설정.
#endif	// ASG_ADD_GENS_SYSTEM
	SettingCanMoveMap();

	DECWRITE(m_iScrollBtnMouseEvent, MOVECOMMAND_MOUSEBTN_NORMAL);
	DECWRITE(m_ScrollBtnPos.y, DECREAD(m_ScrollBtnStartPos.y));
	DECWRITE(m_iRenderStartTextIndex, 0);

	DECWRITE(m_iRenderEndTextIndex, DECREAD(m_iRenderStartTextIndex) + DECREAD(m_iTextLine));

	DECWRITE(m_iWheelCounter, 0);
	
#ifdef _VS2008PORTING
	if( DECREAD(m_iRenderEndTextIndex) > (int)m_listMoveInfoData.size() )
#else // _VS2008PORTING
	if( DECREAD(m_iRenderEndTextIndex) > m_listMoveInfoData.size() )
#endif // _VS2008PORTING
	{
		DECWRITE(m_iRenderEndTextIndex, DECREAD(m_iRenderEndTextIndex) - (DECREAD(m_iRenderEndTextIndex) - m_listMoveInfoData.size()));
	}
}

#ifdef CSK_MOD_MOVE_COMMAND_WINDOW
void SEASON3B::CNewUIMoveCommandWindow::ResetWindowOpenCount()
{
#ifdef YDG_MOD_PROTECT_AUTO_V4
// 	CMoveCommandWindowEncrypt enc;
#endif	// YDG_MOD_PROTECT_AUTO_V4
	DECWRITE(m_iWindowOpenCount, 0);	
}

void SEASON3B::CNewUIMoveCommandWindow::SetCastleOwner(bool bOwner)
{
	m_bCastleOwner = bOwner;
}

bool SEASON3B::CNewUIMoveCommandWindow::IsCastleOwner()
{
	return m_bCastleOwner;
}
#endif // CSK_MOD_MOVE_COMMAND_WINDOW

//---------------------------------------------------------------------------------------------
// ClosingProcess
void SEASON3B::CNewUIMoveCommandWindow::ClosingProcess()
{

}

//---------------------------------------------------------------------------------------------
// LoadImages
void SEASON3B::CNewUIMoveCommandWindow::LoadImages()
{
	LoadBitmap("Interface\\newui_scrollbar_up.tga", IMAGE_MOVECOMMAND_SCROLL_TOP);
	LoadBitmap("Interface\\newui_scrollbar_m.tga", IMAGE_MOVECOMMAND_SCROLL_MIDDLE, GL_LINEAR);
	LoadBitmap("Interface\\newui_scrollbar_down.tga", IMAGE_MOVECOMMAND_SCROLL_BOTTOM);
	LoadBitmap("Interface\\newui_scroll_on.tga", IMAGE_MOVECOMMAND_SCROLLBAR_ON, GL_LINEAR);
	LoadBitmap("Interface\\newui_scroll_off.tga", IMAGE_MOVECOMMAND_SCROLLBAR_OFF, GL_LINEAR);
}

//---------------------------------------------------------------------------------------------
// UnloadImages
void CNewUIMoveCommandWindow::UnloadImages()
{
	DeleteBitmap(IMAGE_MOVECOMMAND_SCROLL_TOP);
	DeleteBitmap(IMAGE_MOVECOMMAND_SCROLL_MIDDLE);
	DeleteBitmap(IMAGE_MOVECOMMAND_SCROLL_BOTTOM);
	DeleteBitmap(IMAGE_MOVECOMMAND_SCROLLBAR_ON);
	DeleteBitmap(IMAGE_MOVECOMMAND_SCROLLBAR_OFF);
}

//////////////////////////////////////////////////////////////////////////

#ifdef YDG_MOD_PROTECT_AUTO_V4_R3
#define CREATE_KEY_ADDRESS(s1, s2, s3, s4) ((DWORD *)((s1^0x12) << 24 | (s2^0x39) << 16 | (s3^0x1A) << 8 | (s4^0x68)))
#define DECOMPOSE_KEY_ADDRESS(key, s1, s2, s3, s4)\
	s1 = (((DWORD)key^0x12000000) & 0xFF000000) >> 24;\
	s2 = (((DWORD)key^0x00390000) & 0x00FF0000) >> 16;\
	s3 = (((DWORD)key^0x00001A00) & 0x0000FF00) >> 8;\
	s4 = (((DWORD)key^0x00000068) & 0x000000FF);
#endif	// YDG_MOD_PROTECT_AUTO_V4_R3

#ifdef YDG_MOD_PROTECT_AUTO_V4
DWORD CMoveCommandWindowEncrypt::m_dwEncryptSeed = 0;
int CMoveCommandWindowEncrypt::m_iEncryptCounter = 1;

CMoveCommandWindowEncrypt::CMoveCommandWindowEncrypt()
{
#ifdef YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V4
	if (g_pProtectAuto == NULL || !g_pProtectAuto->IsNewVersion()) return;
#endif	// YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V4
	if (g_pMoveCommandWindow == NULL) return;

	assert(m_iEncryptCounter >= 0);
	++m_iEncryptCounter;
	if (m_iEncryptCounter == 1)
		DecryptWindowPos();
}

CMoveCommandWindowEncrypt::~CMoveCommandWindowEncrypt()
{
#ifdef YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V4
	if (g_pProtectAuto == NULL || !g_pProtectAuto->IsNewVersion()) return;
#endif	// YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V4
	if (g_pMoveCommandWindow == NULL) return;

	assert(m_iEncryptCounter >= 1);
	--m_iEncryptCounter;
	if (m_iEncryptCounter == 0)
		EncryptWindowPos();
}

void CMoveCommandWindowEncrypt::Enable(BOOL bFlag)
{
	if (bFlag)
	{
		if (m_iEncryptCounter >= 1)	// 초기 or 암호 해제 상태
		{
#ifdef YDG_MOD_PROTECT_AUTO_V4_R3
			CreateKey(TRUE);
#endif	// YDG_MOD_PROTECT_AUTO_V4_R3
			EncryptWindowPos();
		}
		m_iEncryptCounter = 0;
	}
	else
	{
		if (m_iEncryptCounter == 0)	// 암호 상태
			DecryptWindowPos();
		m_iEncryptCounter = 1;
	}
}

#ifdef YDG_MOD_PROTECT_AUTO_V4_R2
int CMoveCommandWindowEncrypt::Read(int * piValue)
{
	int iValue = 0;
	Decrypt(piValue);
	iValue = *piValue;
	Encrypt(piValue);
#ifdef YDG_MOD_PROTECT_AUTO_V4_R5
	*g_piEncValue = rand() * rand();
	return iValue ^ *g_piEncValue;
#else	// YDG_MOD_PROTECT_AUTO_V4_R5
	return iValue;
#endif	// YDG_MOD_PROTECT_AUTO_V4_R5
}

long CMoveCommandWindowEncrypt::Read(LONG * plValue)
{
	LONG lValue = 0;
	Decrypt((int *)plValue);
	lValue = *plValue;
	Encrypt((int *)plValue);
#ifdef YDG_MOD_PROTECT_AUTO_V4_R5
	*g_piEncValue = rand() * rand();
	return lValue ^ *g_piEncValue;
#else	// YDG_MOD_PROTECT_AUTO_V4_R5
	return lValue;
#endif	// YDG_MOD_PROTECT_AUTO_V4_R5
}

void CMoveCommandWindowEncrypt::Write(int * piValue, int iValue)
{
	Decrypt(piValue);
	*piValue = iValue;
	Encrypt(piValue);
}

void CMoveCommandWindowEncrypt::Write(LONG * plValue, LONG lValue)
{
	Decrypt((int *)plValue); 
	*plValue = lValue;
	Encrypt((int *)plValue);
}
#endif	// YDG_MOD_PROTECT_AUTO_V4_R2

void CMoveCommandWindowEncrypt::Encrypt(int * piValue)
{
#ifdef YDG_MOD_PROTECT_AUTO_V4_R3
	DWORD * pKey = NULL;
	pKey = CREATE_KEY_ADDRESS(g_btKeyAddressBlock1[0], CMoveCommandData::GetInstance()->m_btKeyAddressBlock2[0], g_pMoveCommandWindow->m_pbtKeyAddressBlock3[0], g_pProtectAuto->m_pbtKeyAddressBlock4[0]);
	assert(pKey != NULL);
	*piValue ^= (0x8F1BD38C | (*pKey & 0x0000FFFF) << 16 | (*pKey & 0x0000FFFF));

	pKey = CREATE_KEY_ADDRESS(g_pMoveCommandWindow->m_pbtKeyAddressBlock3[1], g_pProtectAuto->m_pbtKeyAddressBlock4[1], g_btKeyAddressBlock1[1], CMoveCommandData::GetInstance()->m_btKeyAddressBlock2[1]);
	*piValue ^= (0x46F2F0A6 | (*pKey & 0x0000FFFF) << 10 | (*pKey & 0x0000FFFF));

	pKey = CREATE_KEY_ADDRESS(g_pProtectAuto->m_pbtKeyAddressBlock4[2], g_pMoveCommandWindow->m_pbtKeyAddressBlock3[2], CMoveCommandData::GetInstance()->m_btKeyAddressBlock2[2], g_btKeyAddressBlock1[2]);
	*piValue ^= (0xA2044FDC | (*pKey & 0x0000FFFF) << 13 | (*pKey & 0x0000FFFF));

	pKey = CREATE_KEY_ADDRESS(g_btKeyAddressBlock1[3], g_pProtectAuto->m_pbtKeyAddressBlock4[3], CMoveCommandData::GetInstance()->m_btKeyAddressBlock2[3], g_pMoveCommandWindow->m_pbtKeyAddressBlock3[3]);
	*piValue ^= (0x95F9341A | (*pKey & 0x0000FFFF) << 15 | (*pKey & 0x0000FFFF));
#else	// YDG_MOD_PROTECT_AUTO_V4_R3
	*piValue ^= (0x9AC0341A | (m_dwEncryptSeed & 0x000000FF));
	*piValue ^= (0x8BD67ED2 | (m_dwEncryptSeed & 0x0000FF00));
	*piValue ^= (0x463CF097 | (m_dwEncryptSeed & 0x000F00F0));
#endif	// YDG_MOD_PROTECT_AUTO_V4_R3
}

void CMoveCommandWindowEncrypt::Decrypt(int * piValue)
{
#ifdef YDG_MOD_PROTECT_AUTO_V4_R3
	DWORD * pKey = NULL;
	pKey = CREATE_KEY_ADDRESS(g_btKeyAddressBlock1[3], g_pProtectAuto->m_pbtKeyAddressBlock4[3], CMoveCommandData::GetInstance()->m_btKeyAddressBlock2[3], g_pMoveCommandWindow->m_pbtKeyAddressBlock3[3]);
	assert(pKey != NULL);
	*piValue ^= (0x95F9341A | (*pKey & 0x0000FFFF) << 15 | (*pKey & 0x0000FFFF));

	pKey = CREATE_KEY_ADDRESS(g_pProtectAuto->m_pbtKeyAddressBlock4[2], g_pMoveCommandWindow->m_pbtKeyAddressBlock3[2], CMoveCommandData::GetInstance()->m_btKeyAddressBlock2[2], g_btKeyAddressBlock1[2]);
	*piValue ^= (0xA2044FDC | (*pKey & 0x0000FFFF) << 13 | (*pKey & 0x0000FFFF));

	pKey = CREATE_KEY_ADDRESS(g_pMoveCommandWindow->m_pbtKeyAddressBlock3[1], g_pProtectAuto->m_pbtKeyAddressBlock4[1], g_btKeyAddressBlock1[1], CMoveCommandData::GetInstance()->m_btKeyAddressBlock2[1]);
	*piValue ^= (0x46F2F0A6 | (*pKey & 0x0000FFFF) << 10 | (*pKey & 0x0000FFFF));

	pKey = CREATE_KEY_ADDRESS(g_btKeyAddressBlock1[0], CMoveCommandData::GetInstance()->m_btKeyAddressBlock2[0], g_pMoveCommandWindow->m_pbtKeyAddressBlock3[0], g_pProtectAuto->m_pbtKeyAddressBlock4[0]);
	*piValue ^= (0x8F1BD38C | (*pKey & 0x0000FFFF) << 16 | (*pKey & 0x0000FFFF));
#else	// YDG_MOD_PROTECT_AUTO_V4_R3
	*piValue ^= (0x463CF097 | (m_dwEncryptSeed & 0x000F00F0));
	*piValue ^= (0x8BD67ED2 | (m_dwEncryptSeed & 0x0000FF00));
	*piValue ^= (0x9AC0341A | (m_dwEncryptSeed & 0x000000FF));
#endif	// YDG_MOD_PROTECT_AUTO_V4_R3
}

void CMoveCommandWindowEncrypt::Encrypt(POINT * pValue)
{
	Encrypt((int *)&pValue->x);
	Encrypt((int *)&pValue->y);
}

void CMoveCommandWindowEncrypt::Decrypt(POINT * pValue)
{
	Decrypt((int *)&pValue->x);
	Decrypt((int *)&pValue->y);
}

#ifdef YDG_MOD_PROTECT_AUTO_V4_R3
void CMoveCommandWindowEncrypt::CreateKey(BOOL bInit)
{
	if (!bInit)
		DeleteKey();	// 이미 지정된 키값을 지운다

	DWORD * pKey = NULL;

	int iRand = rand()%5+5;
	DWORD * pDummy[10];
	int i = 0;
	for (i = 0; i < iRand; ++i)
	{
		pDummy[i] = new DWORD;
		*pDummy[i] = rand() * rand();
	}
	
	pKey = new DWORD;
	*pKey = rand() * rand();
	DECOMPOSE_KEY_ADDRESS(pKey, g_btKeyAddressBlock1[0], CMoveCommandData::GetInstance()->m_btKeyAddressBlock2[0], g_pMoveCommandWindow->m_pbtKeyAddressBlock3[0], g_pProtectAuto->m_pbtKeyAddressBlock4[0]);

	pKey = new DWORD;
	*pKey = rand() * rand();
	DECOMPOSE_KEY_ADDRESS(pKey, g_pMoveCommandWindow->m_pbtKeyAddressBlock3[1], g_pProtectAuto->m_pbtKeyAddressBlock4[1], g_btKeyAddressBlock1[1], CMoveCommandData::GetInstance()->m_btKeyAddressBlock2[1]);

	pKey = new DWORD;
	*pKey = rand() * rand();
	DECOMPOSE_KEY_ADDRESS(pKey, g_pProtectAuto->m_pbtKeyAddressBlock4[2], g_pMoveCommandWindow->m_pbtKeyAddressBlock3[2], CMoveCommandData::GetInstance()->m_btKeyAddressBlock2[2], g_btKeyAddressBlock1[2]);

	pKey = new DWORD;
	*pKey = timeGetTime();
	DECOMPOSE_KEY_ADDRESS(pKey, g_btKeyAddressBlock1[3], g_pProtectAuto->m_pbtKeyAddressBlock4[3], CMoveCommandData::GetInstance()->m_btKeyAddressBlock2[3], g_pMoveCommandWindow->m_pbtKeyAddressBlock3[3]);

	for (i = 0; i < iRand; ++i)
	{
		delete pDummy[i];
	}
}

void CMoveCommandWindowEncrypt::DeleteKey()
{
	DWORD * pKey = NULL;
	pKey = CREATE_KEY_ADDRESS(g_btKeyAddressBlock1[0], CMoveCommandData::GetInstance()->m_btKeyAddressBlock2[0], g_pMoveCommandWindow->m_pbtKeyAddressBlock3[0], g_pProtectAuto->m_pbtKeyAddressBlock4[0]);
	if (pKey != NULL)
	{
		delete pKey;
		pKey = NULL;
	}

	pKey = CREATE_KEY_ADDRESS(g_pMoveCommandWindow->m_pbtKeyAddressBlock3[1], g_pProtectAuto->m_pbtKeyAddressBlock4[1], g_btKeyAddressBlock1[1], CMoveCommandData::GetInstance()->m_btKeyAddressBlock2[1]);
	if (pKey != NULL)
	{
		delete pKey;
		pKey = NULL;
	}

	pKey = CREATE_KEY_ADDRESS(g_pProtectAuto->m_pbtKeyAddressBlock4[2], g_pMoveCommandWindow->m_pbtKeyAddressBlock3[2], CMoveCommandData::GetInstance()->m_btKeyAddressBlock2[2], g_btKeyAddressBlock1[2]);
	if (pKey != NULL)
	{
		delete pKey;
		pKey = NULL;
	}

	pKey = CREATE_KEY_ADDRESS(g_btKeyAddressBlock1[3], g_pProtectAuto->m_pbtKeyAddressBlock4[3], CMoveCommandData::GetInstance()->m_btKeyAddressBlock2[3], g_pMoveCommandWindow->m_pbtKeyAddressBlock3[3]);
	if (pKey != NULL)
	{
		delete pKey;
		pKey = NULL;
	}
}

void CMoveCommandWindowEncrypt::CreateFakeKey()
{
	int iRand = rand()%5+5;
	DWORD * pDummy[10];
	int i = 0;
	for (i = 0; i < iRand; ++i)
	{
		pDummy[i] = new DWORD;
		*pDummy[i] = rand() * rand();
	}
#ifdef YDG_MOD_PROTECT_AUTO_V4_R5
	*g_piEncValue = rand() * rand();	// 리턴값 암호화
#endif	// YDG_MOD_PROTECT_AUTO_V4_R5
	
	for (i = 0; i < iRand; ++i)
	{
		delete pDummy[i];
	}
}
#endif	// YDG_MOD_PROTECT_AUTO_V4_R3

void CMoveCommandWindowEncrypt::EncryptWindowPos()
{
#ifdef YDG_MOD_PROTECT_AUTO_V4_R3
	CreateKey();
	CreateFakeKey();
#endif	// YDG_MOD_PROTECT_AUTO_V4_R3
	m_dwEncryptSeed = timeGetTime() * 7;

	Encrypt(&g_pMoveCommandWindow->m_Pos);
	Encrypt(&g_pMoveCommandWindow->m_iRealFontHeight);
	Encrypt(&g_pMoveCommandWindow->m_StartUISubjectName);
	Encrypt(&g_pMoveCommandWindow->m_StartMapNamePos);
	Encrypt(&g_pMoveCommandWindow->m_MapNameUISize);
#ifdef ASG_ADD_GENS_SYSTEM
	Encrypt(&g_pMoveCommandWindow->m_StrifePos);
#endif	// ASG_ADD_GENS_SYSTEM
	Encrypt(&g_pMoveCommandWindow->m_MapNamePos);
	Encrypt(&g_pMoveCommandWindow->m_ReqLevelPos);
	Encrypt(&g_pMoveCommandWindow->m_ReqZenPos);
	Encrypt(&g_pMoveCommandWindow->m_iSelectedMapName);
	Encrypt(&g_pMoveCommandWindow->m_ScrollBarPos);
	Encrypt(&g_pMoveCommandWindow->m_ScrollBtnStartPos);
	Encrypt(&g_pMoveCommandWindow->m_ScrollBtnPos);
	Encrypt(&g_pMoveCommandWindow->m_iScrollBarHeightPixel);	
	Encrypt(&g_pMoveCommandWindow->m_iRenderStartTextIndex);	
	Encrypt(&g_pMoveCommandWindow->m_iRenderEndTextIndex);		
	Encrypt(&g_pMoveCommandWindow->m_iSelectedTextIndex);		
	Encrypt(&g_pMoveCommandWindow->m_iScrollBtnInterval);		
	Encrypt(&g_pMoveCommandWindow->m_iScrollBarMiddleNum);		
	Encrypt(&g_pMoveCommandWindow->m_iScrollBarMiddleRemainderPixel);
	Encrypt(&g_pMoveCommandWindow->m_iNumPage);				
	Encrypt(&g_pMoveCommandWindow->m_iCurPage);				
	Encrypt(&g_pMoveCommandWindow->m_iTotalMoveScrBtnPixel);		
	Encrypt(&g_pMoveCommandWindow->m_iRemainMoveScrBtnPixel);		
	Encrypt(&g_pMoveCommandWindow->m_icurMoveScrBtnPixelperStep);	
	Encrypt(&g_pMoveCommandWindow->m_iMaxMoveScrBtnPixelperStep);	
	Encrypt(&g_pMoveCommandWindow->m_iMinMoveScrBtnPixelperStep);	
	Encrypt(&g_pMoveCommandWindow->m_iTotalMoveScrBtnperStep);		
	Encrypt(&g_pMoveCommandWindow->m_iRemainMoveScrBtnperStep);		
	Encrypt(&g_pMoveCommandWindow->m_iTotalNumMaxMoveScrBtnperStep);
	Encrypt(&g_pMoveCommandWindow->m_iTotalNumMinMoveScrBtnperStep);
	Encrypt(&g_pMoveCommandWindow->m_iAcumMoveMouseScrollPixel);	
	Encrypt(&g_pMoveCommandWindow->m_iMousePosY);
	Encrypt(&g_pMoveCommandWindow->m_iScrollBtnMouseEvent);
	Encrypt(&g_pMoveCommandWindow->m_iPosY_Random);		
	Encrypt(&g_pMoveCommandWindow->m_iPosY_Random_Min);	
	Encrypt(&g_pMoveCommandWindow->m_iPosY_Random_Range);
	Encrypt(&g_pMoveCommandWindow->m_Pos_Start);		
	Encrypt(&g_pMoveCommandWindow->m_iWindowOpenCount);	
	Encrypt(&g_pMoveCommandWindow->m_iWorldIndex);		
	Encrypt(&g_pMoveCommandWindow->m_iTextLine);		
	Encrypt(&g_pMoveCommandWindow->m_iTextAlpha);		
	Encrypt(&g_pMoveCommandWindow->m_iTopSpace);		
	Encrypt(&g_pMoveCommandWindow->m_iWheelCounter);	
}

void CMoveCommandWindowEncrypt::DecryptWindowPos()
{
	Decrypt(&g_pMoveCommandWindow->m_Pos);
	Decrypt(&g_pMoveCommandWindow->m_iRealFontHeight);
	Decrypt(&g_pMoveCommandWindow->m_StartUISubjectName);
	Decrypt(&g_pMoveCommandWindow->m_StartMapNamePos);
	Decrypt(&g_pMoveCommandWindow->m_MapNameUISize);
#ifdef ASG_ADD_GENS_SYSTEM
	Decrypt(&g_pMoveCommandWindow->m_StrifePos);
#endif	// ASG_ADD_GENS_SYSTEM
	Decrypt(&g_pMoveCommandWindow->m_MapNamePos);
	Decrypt(&g_pMoveCommandWindow->m_ReqLevelPos);
	Decrypt(&g_pMoveCommandWindow->m_ReqZenPos);
	Decrypt(&g_pMoveCommandWindow->m_iSelectedMapName);
	Decrypt(&g_pMoveCommandWindow->m_ScrollBarPos);
	Decrypt(&g_pMoveCommandWindow->m_ScrollBtnStartPos);
	Decrypt(&g_pMoveCommandWindow->m_ScrollBtnPos);
	Decrypt(&g_pMoveCommandWindow->m_iScrollBarHeightPixel);	
	Decrypt(&g_pMoveCommandWindow->m_iRenderStartTextIndex);	
	Decrypt(&g_pMoveCommandWindow->m_iRenderEndTextIndex);		
	Decrypt(&g_pMoveCommandWindow->m_iSelectedTextIndex);		
	Decrypt(&g_pMoveCommandWindow->m_iScrollBtnInterval);		
	Decrypt(&g_pMoveCommandWindow->m_iScrollBarMiddleNum);		
	Decrypt(&g_pMoveCommandWindow->m_iScrollBarMiddleRemainderPixel);
	Decrypt(&g_pMoveCommandWindow->m_iNumPage);				
	Decrypt(&g_pMoveCommandWindow->m_iCurPage);				
	Decrypt(&g_pMoveCommandWindow->m_iTotalMoveScrBtnPixel);		
	Decrypt(&g_pMoveCommandWindow->m_iRemainMoveScrBtnPixel);		
	Decrypt(&g_pMoveCommandWindow->m_icurMoveScrBtnPixelperStep);	
	Decrypt(&g_pMoveCommandWindow->m_iMaxMoveScrBtnPixelperStep);	
	Decrypt(&g_pMoveCommandWindow->m_iMinMoveScrBtnPixelperStep);	
	Decrypt(&g_pMoveCommandWindow->m_iTotalMoveScrBtnperStep);		
	Decrypt(&g_pMoveCommandWindow->m_iRemainMoveScrBtnperStep);		
	Decrypt(&g_pMoveCommandWindow->m_iTotalNumMaxMoveScrBtnperStep);
	Decrypt(&g_pMoveCommandWindow->m_iTotalNumMinMoveScrBtnperStep);
	Decrypt(&g_pMoveCommandWindow->m_iAcumMoveMouseScrollPixel);	
	Decrypt(&g_pMoveCommandWindow->m_iMousePosY);
	Decrypt(&g_pMoveCommandWindow->m_iScrollBtnMouseEvent);
	Decrypt(&g_pMoveCommandWindow->m_iPosY_Random);		
	Decrypt(&g_pMoveCommandWindow->m_iPosY_Random_Min);	
	Decrypt(&g_pMoveCommandWindow->m_iPosY_Random_Range);
	Decrypt(&g_pMoveCommandWindow->m_Pos_Start);		
	Decrypt(&g_pMoveCommandWindow->m_iWindowOpenCount);	
	Decrypt(&g_pMoveCommandWindow->m_iWorldIndex);		
	Decrypt(&g_pMoveCommandWindow->m_iTextLine);		
	Decrypt(&g_pMoveCommandWindow->m_iTextAlpha);		
	Decrypt(&g_pMoveCommandWindow->m_iTopSpace);		
	Decrypt(&g_pMoveCommandWindow->m_iWheelCounter);	
}
#endif	// YDG_MOD_PROTECT_AUTO_V4

#ifdef LJH_ADD_SAVEOPTION_WHILE_MOVING_FROM_OR_TO_DIFF_SERVER
// 로랜 협곡이나 로랜시장처럼 맵이 다른 서버에 존재 하는지 여부
BOOL CNewUIMoveCommandWindow::IsTheMapInDifferentServer(const int iFromMapIndex, const int iToMapIndex) const
{
	BOOL bInOtherServer = FALSE;

	// 출발 지역 (ENUM_WORLD index)
	switch(iFromMapIndex) 
	{
		case WD_30BATTLECASTLE:
		case WD_79UNITEDMARKETPLACE:
			bInOtherServer = TRUE;
			break;
		default:
			break;
	}

	// 도착지역 (movereq index)
	switch(iToMapIndex)
	{
		case 24:	// 로렌협곡
		case 44:	// 로렌시장
			bInOtherServer = TRUE;
			break;
		default:
			break;
	}

	return bInOtherServer;
}
#endif //LJH_ADD_SAVEOPTION_WHILE_MOVING_FROM_OR_TO_DIFF_SERVER
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
// movereq에서 맵의 이름(자국어 또는 영어)에 맞는 index를 반환한다.
// 만약, NULL값이 넘어오거나 맵의 이름을 찾지 못했을 경우 -1을 반환.
int CNewUIMoveCommandWindow::GetMapIndexFromMovereq(const char *pszMapName)
{
	if (pszMapName == NULL)
		return -1;
	
	int iMapIndex = -1;
	std::list<CMoveCommandData::MOVEINFODATA*>::iterator li;
	
	for (li = m_listMoveInfoData.begin(); li != m_listMoveInfoData.end(); li++)
	{
		if (stricmp((*li)->_ReqInfo.szMainMapName, pszMapName) == 0 
			|| stricmp((*li)->_ReqInfo.szSubMapName, pszMapName) == 0)
		{
			iMapIndex = (*li)->_ReqInfo.index;
			break;
		}
	}
	
	return iMapIndex;
}
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
#else	// YDG_ADD_ENC_MOVE_COMMAND_WINDOW






















































































































































//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNewUIMoveCommandWindow::CNewUIMoveCommandWindow()
{
	m_pNewUIMng = NULL;
	m_Pos.x = m_Pos.y = 0;

	memset( &m_StartUISubjectName, 0, sizeof(POINT) );
	memset( &m_StartMapNamePos, 0, sizeof(POINT) );
	memset( &m_MapNameUISize, 0, sizeof(POINT) );
#ifdef ASG_ADD_GENS_SYSTEM
	memset(&m_StrifePos, 0, sizeof(POINT));
#endif	// ASG_ADD_GENS_SYSTEM
	memset( &m_MapNamePos, 0, sizeof(POINT) );
	memset( &m_ReqLevelPos, 0, sizeof(POINT) );
	memset( &m_ReqZenPos, 0, sizeof(POINT) );
	m_iSelectedMapName = -1;

	memset( &m_ScrollBarPos, 0, sizeof(POINT) );
	memset( &m_ScrollBtnStartPos, 0, sizeof(POINT) );
	memset( &m_ScrollBtnPos, 0, sizeof(POINT) );
	m_iScrollBarHeightPixel = 0;
	m_iRenderStartTextIndex = 0;
	m_iSelectedTextIndex = -1;
	m_iScrollBtnInterval = 0;
	m_iScrollBarMiddleNum = 0;
	m_iScrollBarMiddleRemainderPixel = 0;
	m_iNumPage = 0;
	m_iCurPage = 0;
#ifndef KJH_FIX_MOVECOMMAND_WINDOW_SIZE			// 정리할때 지워야 하는 소스
	m_iMoveSCBtnPerPage = 0;
	m_iNumMoveStep = 0;
	m_iHeightByMoveStep = 0;
#endif // KJH_FIX_MOVECOMMAND_WINDOW_SIZE		// 정리할때 지워야 하는 소스
	m_iMousePosY = 0;
	m_bScrollBtnActive = false;
	m_iScrollBtnMouseEvent = MOVECOMMAND_MOUSEBTN_NORMAL;

#ifdef CSK_MOD_MOVE_COMMAND_WINDOW
	m_iPosY_Random = 0;
	m_iPosY_Random_Range = 50;
	m_iPosY_Random_Min = 15;
	m_Pos_Start.x = 0;
	m_Pos_Start.y = 0;
	m_iWindowOpenCount = 0;
	m_iWorldIndex = NUM_WD;
	m_bCastleOwner = false;
#endif // CSK_MOD_MOVE_COMMAND_WINDOW

#ifdef CSK_MOD_PROTECT_AUTO_V1
	m_iTextLine = MOVECOMMAND_MAX_RENDER_TEXTLINE;
#ifndef CSK_MOD_REMOVE_AUTO_V1_FLAG  // 정리할 때 지워야 하는 소스
	m_bNewVersion = false;
#endif //! CSK_MOD_REMOVE_AUTO_V1_FLAG // 정리할 때 지워야 하는 소스
	m_iTextAlpha = 255;
	m_fBackgroundAlpha = 0.8f;
	m_iTopSpace = 0;
#endif // CSK_MOD_PROTECT_AUTO_V1
#ifdef YDG_MOD_PROTECT_AUTO_V3
	m_iWheelCounter = 0;
#endif	// YDG_MOD_PROTECT_AUTO_V3
}

CNewUIMoveCommandWindow::~CNewUIMoveCommandWindow()
{
	Release();
}

//---------------------------------------------------------------------------------------------
// Create
bool SEASON3B::CNewUIMoveCommandWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if( NULL == pNewUIMng )
		return false;

#ifdef YDG_MOD_PROTECT_AUTO_V4
	CMoveCommandWindowEncrypt enc;
#endif	// YDG_MOD_PROTECT_AUTO_V4
#ifdef CSK_MOD_MOVE_COMMAND_WINDOW
	m_Pos_Start.x = x;
	m_Pos_Start.y = y;
#endif // CSK_MOD_MOVE_COMMAND_WINDOW
	
	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj( SEASON3B::INTERFACE_MOVEMAP, this );		// 인터페이스 오브젝트 등록

	SetPos(x, y);
	
	LoadImages();
	
	Show( false );
	
	return true;
}

//---------------------------------------------------------------------------------------------
// Release
void SEASON3B::CNewUIMoveCommandWindow::Release()
{
	UnloadImages();
	
	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj( this );
		m_pNewUIMng = NULL;
	}
}

//---------------------------------------------------------------------------------------------
// SetPos
void SEASON3B::CNewUIMoveCommandWindow::SetPos(int x, int y)
{
#ifdef YDG_MOD_PROTECT_AUTO_V4
	CMoveCommandWindowEncrypt enc;
#endif	// YDG_MOD_PROTECT_AUTO_V4
	m_Pos.x = x;
	m_Pos.y = y;

	// 윈도우 해상도에 따라 UI크기를 변경시킨다.
#ifdef ASG_ADD_GENS_SYSTEM
#ifdef YDG_MOD_PROTECT_AUTO_V4_R2
	DECWRITE(m_StrifePos.x, DECREAD(m_Pos.x) + 20);
	switch( WindowWidth )
	{
	case 800:
		DECWRITE(m_MapNameUISize.x, 200); DECWRITE(m_MapNamePos.x, DECREAD(m_Pos.x) + 69); DECWRITE(m_ReqLevelPos.x, DECREAD(m_Pos.x) + 129); DECWRITE(m_ReqZenPos.x, DECREAD(m_Pos.x) + 174);
		break;
	case 1024:
		DECWRITE(m_MapNameUISize.x, 180); DECWRITE(m_MapNamePos.x, DECREAD(m_Pos.x) + 64); DECWRITE(m_ReqLevelPos.x, DECREAD(m_Pos.x) + 119); DECWRITE(m_ReqZenPos.x, DECREAD(m_Pos.x) + 159);
		break;
	case 1280:
		DECWRITE(m_MapNameUISize.x, 160); DECWRITE(m_MapNamePos.x, DECREAD(m_Pos.x) + 59); DECWRITE(m_ReqLevelPos.x, DECREAD(m_Pos.x) + 104); DECWRITE(m_ReqZenPos.x, DECREAD(m_Pos.x) + 139);
		break;
	}
#else  //YDG_MOD_PROTECT_AUTO_V4_R2
	m_StrifePos.x = m_Pos.x + 20;
	switch( WindowWidth )
	{
	case 800:
		m_MapNameUISize.x = 200; m_MapNamePos.x = m_Pos.x + 69; m_ReqLevelPos.x = m_Pos.x + 129; m_ReqZenPos.x = m_Pos.x + 174;
		break;
	case 1024:
		m_MapNameUISize.x = 180; m_MapNamePos.x = m_Pos.x + 64; m_ReqLevelPos.x = m_Pos.x + 119; m_ReqZenPos.x = m_Pos.x + 159;
		break;
	case 1280:
		m_MapNameUISize.x = 160; m_MapNamePos.x = m_Pos.x + 59; m_ReqLevelPos.x = m_Pos.x + 104; m_ReqZenPos.x = m_Pos.x + 139;
		break;
	}
#endif //YDG_MOD_PROTECT_AUTO_V4_R2
#else	// ASG_ADD_GENS_SYSTEM
	switch( WindowWidth )
	{
	case 640:
		m_MapNameUISize.x = 190; m_MapNamePos.x = m_Pos.x + 44; m_ReqLevelPos.x = m_Pos.x + 104; m_ReqZenPos.x = m_Pos.x + 154;
		break;
	case 800:
		m_MapNameUISize.x = 170; m_MapNamePos.x = m_Pos.x + 39; m_ReqLevelPos.x = m_Pos.x + 99; m_ReqZenPos.x = m_Pos.x + 144;
		break;
	case 1024:
		m_MapNameUISize.x = 150; m_MapNamePos.x = m_Pos.x + 34; m_ReqLevelPos.x = m_Pos.x + 89; m_ReqZenPos.x = m_Pos.x + 129;
		break;
	case 1280:
		m_MapNameUISize.x = 130; m_MapNamePos.x = m_Pos.x + 29; m_ReqLevelPos.x = m_Pos.x + 74; m_ReqZenPos.x = m_Pos.x + 109;
		break;
	}
#endif	// ASG_ADD_GENS_SYSTEM
	m_MapNameUISize.x += 10;			// 스크롤바를 위해서 x의 길이를 10늘린다.

	m_listMoveInfoData = CMoveCommandData::GetInstance()->GetMoveCommandDatalist();
	m_iRealFontHeight = FontHeight * 640 / WindowWidth + 2;

#ifdef CSK_MOD_PROTECT_AUTO_V1
	m_MapNameUISize.y = 60 + (m_iRealFontHeight * m_iTextLine) + m_iTopSpace;
#else // CSK_MOD_PROTECT_AUTO_V1
	m_MapNameUISize.y = 60 + (m_iRealFontHeight * MOVECOMMAND_MAX_RENDER_TEXTLINE);
#endif // CSK_MOD_PROTECT_AUTO_V1
	
	// UI제목 시작위치
	m_StartUISubjectName.x = m_Pos.x + m_MapNameUISize.x / 2;
	m_StartUISubjectName.y = m_Pos.y + 4;

	// 맵 이름 시작위치
	m_StartMapNamePos.x = m_Pos.x + 2;
#ifdef CSK_MOD_PROTECT_AUTO_V1
	m_StartMapNamePos.y = m_Pos.y + 38 + m_iTopSpace;
#else // CSK_MOD_PROTECT_AUTO_V1
	m_StartMapNamePos.y = m_Pos.y + 38;
#endif // CSK_MOD_PROTECT_AUTO_V1
	
#ifdef ASG_ADD_GENS_SYSTEM
	// 분쟁지역
	m_StrifePos.y = m_StartMapNamePos.y;
#endif	// ASG_ADD_GENS_SYSTEM

	// 맵이름 위치
	m_MapNamePos.y = m_StartMapNamePos.y;

	// 요구레벨
	m_ReqLevelPos.y = m_StartMapNamePos.y;

	// 요구젠
	m_ReqZenPos.y = m_StartMapNamePos.y;

	m_ScrollBarPos.x = m_Pos.x + m_MapNameUISize.x - 14;
#ifdef CSK_MOD_MOVE_COMMAND_WINDOW
	m_ScrollBarPos.y = m_StartMapNamePos.y - MOVECOMMAND_SCROLLBAR_TOP_HEIGHT;
#else // CSK_MOD_MOVE_COMMAND_WINDOW
	m_ScrollBarPos.y = m_Pos.y + m_StartMapNamePos.y - MOVECOMMAND_SCROLLBAR_TOP_HEIGHT;
#endif // CSK_MOD_MOVE_COMMAND_WINDOW
	
	m_ScrollBtnStartPos.x = m_ScrollBarPos.x - (MOVECOMMAND_SCROLLBTN_WIDTH/2 - MOVECOMMAND_SCROLLBAR_TOP_WIDTH/2);
	m_ScrollBtnStartPos.y = m_ScrollBarPos.y;
	m_ScrollBtnPos.x = m_ScrollBtnStartPos.x;
	m_ScrollBtnPos.y = m_ScrollBtnStartPos.y;

#ifdef CSK_MOD_PROTECT_AUTO_V1
	m_iScrollBarHeightPixel = m_iTextLine * m_iRealFontHeight;
#else // CSK_MOD_PROTECT_AUTO_V1
	m_iScrollBarHeightPixel = MOVECOMMAND_MAX_RENDER_TEXTLINE * m_iRealFontHeight;
#endif // CSK_MOD_PROTECT_AUTO_V1
	
	m_iScrollBarMiddleNum = (m_iScrollBarHeightPixel-(MOVECOMMAND_SCROLLBAR_TOP_HEIGHT*2))/MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT;
	m_iScrollBarMiddleRemainderPixel = (m_iScrollBarHeightPixel-(MOVECOMMAND_SCROLLBAR_TOP_HEIGHT*2))%MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT;

#ifdef CSK_MOD_PROTECT_AUTO_V1
	m_iNumPage = 1 + (m_listMoveInfoData.size() / m_iTextLine);
#else // CSK_MOD_PROTECT_AUTO_V1 
	m_iNumPage = 1+(m_listMoveInfoData.size()/MOVECOMMAND_MAX_RENDER_TEXTLINE);
#endif // CSK_MOD_PROTECT_AUTO_V1
	
	m_iCurPage = 1;
	
#ifdef KJH_FIX_MOVECOMMAND_WINDOW_SIZE
	// 스크롤바가 이동해야할 칸의 전체 갯수
#ifdef CSK_MOD_PROTECT_AUTO_V1
	m_iTotalMoveScrBtnperStep = m_listMoveInfoData.size() - m_iTextLine;
#else // CSK_MOD_PROTECT_AUTO_V1
	m_iTotalMoveScrBtnperStep = m_listMoveInfoData.size() - MOVECOMMAND_MAX_RENDER_TEXTLINE;
#endif // CSK_MOD_PROTECT_AUTO_V1
	// 스크롤바가 이동해야할 칸의 나머지 갯수
	m_iRemainMoveScrBtnperStep		= m_iTotalMoveScrBtnperStep;
	// 스크롤바가 이동해야할 전체 픽셀값
	m_iTotalMoveScrBtnPixel			= m_iScrollBarHeightPixel-MOVECOMMAND_SCROLLBTN_HEIGHT;	
	// 스크롤바가 이동해야할 나머지 픽셀값
	m_iRemainMoveScrBtnPixel		= m_iTotalMoveScrBtnPixel;								
	// 스크롤바가 이동하는 한칸당 최소 픽셀
	m_iMinMoveScrBtnPixelperStep	= m_iTotalMoveScrBtnPixel/m_iTotalMoveScrBtnperStep;	
	// 스크롤바가 이동하는 한칸당 최대 픽셀
	m_iMaxMoveScrBtnPixelperStep	= m_iMinMoveScrBtnPixelperStep+1;						
	// 스크롤바가 이동해야하는 한칸당 최대픽셀의 갯수
	m_iTotalNumMaxMoveScrBtnperStep		= m_iTotalMoveScrBtnPixel-(m_iTotalMoveScrBtnperStep*m_iMinMoveScrBtnPixelperStep);		
	// 스크롤바가 이동해야하는 한칸당 최대픽셀의 갯수
	m_iTotalNumMinMoveScrBtnperStep		= m_iTotalMoveScrBtnperStep - m_iTotalNumMaxMoveScrBtnperStep;
	m_icurMoveScrBtnPixelperStep	= m_iMaxMoveScrBtnPixelperStep;
	m_iAcumMoveMouseScrollPixel = 0;
#else // KJH_FIX_MOVECOMMAND_WINDOW_SIZE			// 정리할때 지워야 하는 소스

#ifdef CSK_MOD_PROTECT_AUTO_V1
	m_iNumMoveStep = m_listMoveInfoData.size() - m_iTextLine;
#else // CSK_MOD_PROTECT_AUTO_V1
	m_iNumMoveStep = m_listMoveInfoData.size() - MOVECOMMAND_MAX_RENDER_TEXTLINE;
#endif // CSK_MOD_PROTECT_AUTO_V1

	if( m_iNumMoveStep <= 0 )
	{
		m_iHeightByMoveStep = (m_iScrollBarHeightPixel-MOVECOMMAND_SCROLLBTN_HEIGHT) / m_iNumMoveStep;
#ifdef CSK_MOD_PROTECT_AUTO_V1
		m_iMoveSCBtnPerPage = m_iHeightByMoveStep * m_iTextLine;
#else // CSK_MOD_PROTECT_AUTO_V1
		m_iMoveSCBtnPerPage = m_iHeightByMoveStep * MOVECOMMAND_MAX_RENDER_TEXTLINE;
#endif // CSK_MOD_PROTECT_AUTO_V1
	}
	else
	{
		m_iHeightByMoveStep = (m_iScrollBarHeightPixel-MOVECOMMAND_SCROLLBTN_HEIGHT) / m_iNumMoveStep;
#ifdef CSK_MOD_PROTECT_AUTO_V1
		m_iMoveSCBtnPerPage = m_iHeightByMoveStep * m_iTextLine;
#else // CSK_MOD_PROTECT_AUTO_V1
		m_iMoveSCBtnPerPage = m_iHeightByMoveStep * MOVECOMMAND_MAX_RENDER_TEXTLINE;
#endif // CSK_MOD_PROTECT_AUTO_V1
	}
#endif // KJH_FIX_MOVECOMMAND_WINDOW_SIZE			// 정리할때 지워야 하는 소스

	if( m_iNumPage > 1 )
	{
		m_bScrollBtnActive = true;
	}

	m_iRenderStartTextIndex = 0;
#ifdef CSK_MOD_PROTECT_AUTO_V1
	m_iRenderEndTextIndex = m_iRenderStartTextIndex + m_iTextLine;
#else // CSK_MOD_PROTECT_AUTO_V1
	m_iRenderEndTextIndex = m_iRenderStartTextIndex+MOVECOMMAND_MAX_RENDER_TEXTLINE;
#endif // CSK_MOD_PROTECT_AUTO_V1

#ifdef _VS2008PORTING
	if( m_iRenderEndTextIndex > (int)m_listMoveInfoData.size() )
#else // _VS2008PORTING
	if( m_iRenderEndTextIndex > m_listMoveInfoData.size() )
#endif // _VS2008PORTING
	{
		m_iRenderEndTextIndex -= (m_iRenderEndTextIndex-m_listMoveInfoData.size());
	}
}

#ifndef CSK_MOD_REMOVE_AUTO_V1_FLAG  // 정리할 때 지워야 하는 소스
#ifdef CSK_MOD_PROTECT_AUTO_V1
void SEASON3B::CNewUIMoveCommandWindow::SetNewVersion(bool bNew)
{
	m_bNewVersion = bNew;
}

bool SEASON3B::CNewUIMoveCommandWindow::IsNewVersion()
{
	return m_bNewVersion;
}
#endif // CSK_MOD_PROTECT_AUTO_V1
#endif //! CSK_MOD_REMOVE_AUTO_V1_FLAG // 정리할 때 지워야 하는 소스

#ifdef CSK_LUCKY_SEAL

bool SEASON3B::CNewUIMoveCommandWindow::IsLuckySealBuff()
{
	if( g_isCharacterBuff((&Hero->Object), eBuff_Seal1)  
		|| g_isCharacterBuff((&Hero->Object), eBuff_Seal2)   
		|| g_isCharacterBuff((&Hero->Object), eBuff_Seal3)
#ifdef LEM_FIX_SEAL_ITEM_MAPMOVE_VIEW	// 누락된 인장 아이템 추가 [lem.2010.7.28]
		|| g_isCharacterBuff((&Hero->Object), eBuff_PcRoomSeal1 ) 
		|| g_isCharacterBuff((&Hero->Object), eBuff_PcRoomSeal2 ) 
		|| g_isCharacterBuff((&Hero->Object), eBuff_PcRoomSeal3 ) 
#endif	// LEM_FIX_SEAL_ITEM_MAPMOVE_VIEW
#ifdef PSW_SEAL_ITEM
		|| g_isCharacterBuff((&Hero->Object), eBuff_Seal4) 
#endif //PSW_SEAL_ITEM
#ifdef PSW_ADD_PC4_SEALITEM
		|| g_isCharacterBuff((&Hero->Object), eBuff_Seal_HpRecovery)
		|| g_isCharacterBuff((&Hero->Object), eBuff_Seal_MpRecovery)
#endif //PSW_ADD_PC4_SEALITEM
#ifdef ASG_ADD_CS6_ASCENSION_SEAL_MASTER
		|| g_isCharacterBuff((&Hero->Object), eBuff_AscensionSealMaster)
#endif	// ASG_ADD_CS6_ASCENSION_SEAL_MASTER
#ifdef ASG_ADD_CS6_WEALTH_SEAL_MASTER
		|| g_isCharacterBuff((&Hero->Object), eBuff_WealthSealMaster)
#endif	// ASG_ADD_CS6_WEALTH_SEAL_MASTER
#ifdef LDK_ADD_INGAMESHOP_NEW_WEALTH_SEAL
		|| g_isCharacterBuff((&Hero->Object), eBuff_NewWealthSeal)
#endif //LDK_ADD_INGAMESHOP_NEW_WEALTH_SEAL
#ifdef YDG_ADD_CS7_PARTY_EXP_BONUS_ITEM
		|| g_isCharacterBuff((&Hero->Object), eBuff_PartyExpBonus)
#endif	// YDG_ADD_CS7_PARTY_EXP_BONUS_ITEM
		)
	{
		return true;
	}
	return false;
}

bool SEASON3B::CNewUIMoveCommandWindow::IsMapMove( const string& src )
{
	if( Hero->Object.Kind == KIND_PLAYER 
		&& Hero->Object.Type == MODEL_PLAYER 
		&& Hero->Object.SubType == MODEL_GM_CHARACTER )
	{
		return true;
	}

	if( g_isCharacterBuff((&Hero->Object), eBuff_GMEffect) )
	{
		return true;
	}

#ifdef KJH_ADD_PCROOM_MOVECOMMAND
	if( CPCRoomPtSys::Instance().IsPCRoom() == true )
	{
		return true;
	}
#endif // KJH_ADD_PCROOM_MOVECOMMAND

#ifdef YDG_MOD_PROTECT_AUTO_V4
	CMoveCommandWindowEncrypt enc;
#endif	// YDG_MOD_PROTECT_AUTO_V4
	if( IsLuckySealBuff() == false ) {
		char lpszStr1[1024]; char* lpszStr2 = NULL;
		if(src.find( GlobalText[260] ) != string::npos) {
			string temp = GlobalText[260];
			temp += ' ';
			wsprintf( lpszStr1, src.c_str() );	
			lpszStr2 = strtok( lpszStr1, temp.c_str() );
			if( lpszStr2 == NULL ) return false;

			SettingCanMoveMap();
			std::list<CMoveCommandData::MOVEINFODATA*>::iterator li = m_listMoveInfoData.begin();
			for( int i=0; i<m_iRenderEndTextIndex ; i++, li++ ) {
				if(!strcmp( lpszStr2, (*li)->_ReqInfo.szMainMapName )) {
					if((*li)->_bCanMove == true ){
						return IsLuckySeal( (*li)->_ReqInfo.szSubMapName );
					}
				}
			}
			return false;
		}
		else if(src.find( "/move" ) != string::npos) {
			string temp = "/move";
			temp += ' ';
			wsprintf( lpszStr1, src.c_str() );	
#ifdef PBG_WOPS_MOVE_MAPNAME_JAPAN
			string lpszStr2;
			int iSize = src.size();
			lpszStr2 = src.substr(6, iSize);
#else //PBG_WOPS_MOVE_MAPNAME_JAPAN
			lpszStr2 = strtok( lpszStr1, temp.c_str() );
			if( lpszStr2 == NULL ) return false;
#endif //PBG_WOPS_MOVE_MAPNAME_JAPAN
			
			SettingCanMoveMap();
			std::list<CMoveCommandData::MOVEINFODATA*>::iterator li = m_listMoveInfoData.begin();
			for( int i=0; i<m_iRenderEndTextIndex ; i++, li++ ) {
#ifdef PBG_WOPS_MOVE_MAPNAME_JAPAN
				if(stricmp(lpszStr2.c_str(), (*li)->_ReqInfo.szSubMapName) == 0)
				{
					if((*li)->_bCanMove == true)
					{
						bool bResult = IsLuckySeal((*li)->_ReqInfo.szSubMapName);
						
						return bResult;
					}
				}
#else //PBG_WOPS_MOVE_MAPNAME_JAPAN
				if(!stricmp( lpszStr2, (*li)->_ReqInfo.szMainMapName )) {
					if((*li)->_bCanMove == true ){
						return IsLuckySeal( (*li)->_ReqInfo.szSubMapName );
					}
				}
#endif //PBG_WOPS_MOVE_MAPNAME_JAPAN
			}
			return false;
		}
		else {
			return IsLuckySeal( src );
		}
	}
	return true;
}

#endif //CSK_LUCKY_SEAL

#ifdef YDG_ADD_MOVE_COMMAND_PROTOCOL
void SEASON3B::CNewUIMoveCommandWindow::SetMoveCommandKey(DWORD dwKey)
{
	m_dwMoveCommandKey = dwKey;
}

DWORD SEASON3B::CNewUIMoveCommandWindow::GetMoveCommandKey()
{
	m_dwMoveCommandKey = g_KeyGenerater.GenerateKeyValue(m_dwMoveCommandKey);
	
	return m_dwMoveCommandKey;
}
#endif	// YDG_ADD_MOVE_COMMAND_PROTOCOL

#ifdef ASG_ADD_GENS_SYSTEM
#ifndef KJH_ADD_SERVER_LIST_SYSTEM		// #ifndef
extern  int  ServerSelectHi;
extern  int  ServerLocalSelect;
#endif // KJH_ADD_SERVER_LIST_SYSTEM

// 분쟁 맵 설정.
void SEASON3B::CNewUIMoveCommandWindow::SetStrifeMap()
{
	std::list<CMoveCommandData::MOVEINFODATA*>::iterator li;

	if (BLUE_MU::IsBlueMuServer())	// 부분 유료화 서버인가?
	{
#ifdef ASG_ADD_STRIFE_KANTURU_RUIN_ISLAND
		const int c_nStrife = 10;
		int anStrifeIndex[c_nStrife] = { 11, 12, 13, 25, 27, 28, 29, 33, 42, 45 };	// MoveReq.txt의 맵 인덱스.
#else	// ASG_ADD_STRIFE_KANTURU_RUIN_ISLAND
		int anStrifeIndex[9] = { 11, 12, 13, 25, 27, 28, 29, 33, 42 };	// MoveReq.txt의 맵 인덱스.
#endif	// ASG_ADD_STRIFE_KANTURU_RUIN_ISLAND
		int i;
		for (li = m_listMoveInfoData.begin(); li != m_listMoveInfoData.end(); advance(li, 1))
		{
			(*li)->_bStrife = false;
#ifdef ASG_ADD_STRIFE_KANTURU_RUIN_ISLAND
			for (i = 0; i < c_nStrife; ++i)
#else	// ASG_ADD_STRIFE_KANTURU_RUIN_ISLAND
			for (i = 0; i < 9; ++i)
#endif	// ASG_ADD_STRIFE_KANTURU_RUIN_ISLAND
			{
				if ((*li)->_ReqInfo.index == anStrifeIndex[i])
				{
					(*li)->_bStrife = true;
					break;
				}
			}
		}
	}
#ifdef KJH_ADD_SERVER_LIST_SYSTEM
	else if (!g_ServerListManager->IsNonPvP())	// PVP 서버인가?
#else // KJH_ADD_SERVER_LIST_SYSTEM
	else if (!::IsNonPvpServer(ServerSelectHi, ServerLocalSelect))	// PVP 서버인가?
#endif // KJH_ADD_SERVER_LIST_SYSTEM
	{
		int anStrifeIndex[1] = { 42 };							// MoveReq.txt의 맵 인덱스.
		int i;
		for (li = m_listMoveInfoData.begin(); li != m_listMoveInfoData.end(); advance(li, 1))
		{
			(*li)->_bStrife = false;
			for (i = 0; i < 1; ++i)
			{
				if ((*li)->_ReqInfo.index == anStrifeIndex[i])
				{
					(*li)->_bStrife = true;
					break;
				}
			}
		}
	}
	else	// NonPVP 서버에서는 분쟁 지역 없음.
	{
		for (li = m_listMoveInfoData.begin(); li != m_listMoveInfoData.end(); advance(li, 1))
			(*li)->_bStrife = false;
	}
}
#endif	// ASG_ADD_GENS_SYSTEM

void SEASON3B::CNewUIMoveCommandWindow::SettingCanMoveMap()
{
	int a = World;
	//----------------------------------------------
	//----------------------------------------------
	DWORD iZen;
	int iLevel, iReqLevel, iReqZen;

	std::list<CMoveCommandData::MOVEINFODATA*>::iterator li = m_listMoveInfoData.begin();
	for( int i=0 ; i<m_iRenderEndTextIndex ; i++, li++ )
	{
		if( li == m_listMoveInfoData.end() )
		{
			break;
		}
		if( i<m_iRenderStartTextIndex )
		{
			continue;
		}

		(*li)->_bCanMove = false;
		(*li)->_bSelected = false;

	//	if( i < m_iRenderEndTextIndex-1 )	continue;

		iLevel = CharacterAttribute->Level;
		iZen = CharacterMachine->Gold;
		iReqLevel = (*li)->_ReqInfo.iReqLevel;
		iReqZen = (*li)->_ReqInfo.iReqZen;

		// 마검사/다크로드일때 필요레벨을 계산(400 레벨은 제외)
		if( ( GetBaseClass(CharacterAttribute->Class)==CLASS_DARK || GetBaseClass(CharacterAttribute->Class)==CLASS_DARK_LORD 
#ifdef PBG_ADD_NEWCHAR_MONK
				|| GetBaseClass(CharacterAttribute->Class)==CLASS_RAGEFIGHTER
#endif //PBG_ADD_NEWCHAR_MONK
			)
			&& (iReqLevel != 400) )
		{
#ifdef CSK_FIX_GM_MOVE
			char strNewMapName[64];
			bool bKalima = false;

			// 칼리마1부터 칼리마6까지는 필요레벨 - 20 이고
			// 칼리마7은 필요레벨 그대로
			for(int i=0; i<8; ++i)
			{
				// 58 "칼리마"
				sprintf(strNewMapName, "%s%d", GlobalText[58], i);
				
				if(strcmp((*li)->_ReqInfo.szMainMapName, strNewMapName) == 0)
				{
					bKalima = true;
					break;
				}
			}

			if(bKalima == true)
			{
				if(i >= 0 && i <= 6)
				{
					iReqLevel = iReqLevel - 20;
				}
			}
			else
#endif // CSK_FIX_GM_MOVE
			{
				iReqLevel = int(float(iReqLevel)*2.f/3.f);
			}
		}
		
		// 필요레벨충족/필요젠충족/pk모드가 아닐때 이동가능
#ifdef _VS2008PORTING
#ifdef ASG_FIX_MOVE_WIN_MURDERER1_BUG
#ifdef PBG_ADD_PKSYSTEM_INGAMESHOP
		// 기획변경으로 인해 디버프 걸려있을시에만 사용불가능으로변경
		// 혼합유료화 섭일 경우 무법자 1단계(5분은 불가)와 상관없이 모두 이동창 사용가능
		// 혼합유료화일경우 버프와 PVP_MURDERER1이상일경우를 판단하고
		// 아닐결우는 (int)Hero->PK<PVP_MURDERER1) 결과값에 의해서만 결정을 한다	
		if(g_PKSystem->GetCanDoMoveCommand() && iLevel >= iReqLevel && iZen >= iReqZen
#ifdef ASG_ADD_MOVEREQ_TEXT_MAX_LEVEL
			&& iLevel <= (*li)->_ReqInfo.m_iReqMaxLevel
#endif	// ASG_ADD_MOVEREQ_TEXT_MAX_LEVEL
#else //PBG_ADD_PKSYSTEM_INGAMESHOP
		if( iLevel >= iReqLevel && (int)iZen >= iReqZen && (int)Hero->PK<PVP_MURDERER1
#endif //PBG_ADD_PKSYSTEM_INGAMESHOP
#else	// ASG_FIX_MOVE_WIN_MURDERER1_BUG
		if( iLevel >= iReqLevel && (int)iZen >= iReqZen && (int)Hero->PK<PVP_MURDERER2
#endif	// ASG_FIX_MOVE_WIN_MURDERER1_BUG
#else // _VS2008PORTING
#ifdef ASG_FIX_MOVE_WIN_MURDERER1_BUG
#ifdef PBG_ADD_PKSYSTEM_INGAMESHOP
		// 기획변경으로 인해 디버프 걸려있을시에만 사용불가능으로변경
		// 혼합유료화 섭일 경우 무법자 1단계(5분은 불가)와 상관없이 모두 이동창 사용가능
		// 혼합유료화일경우 버프와 PVP_MURDERER1이상일경우를 판단하고
		// 아닐결우는 (int)Hero->PK<PVP_MURDERER1) 결과값에 의해서만 결정을 한다	
		if(g_PKSystem->GetCanDoMoveCommand() && iLevel >= iReqLevel && iZen >= iReqZen
#ifdef ASG_ADD_MOVEREQ_TEXT_MAX_LEVEL
			&& iLevel <= (*li)->_ReqInfo.m_iReqMaxLevel
#endif	// ASG_ADD_MOVEREQ_TEXT_MAX_LEVEL
#else //PBG_ADD_PKSYSTEM_INGAMESHOP
		if( iLevel >= iReqLevel && iZen >= iReqZen && Hero->PK<PVP_MURDERER1
#endif //PBG_ADD_PKSYSTEM_INGAMESHOP
#else	// ASG_FIX_MOVE_WIN_MURDERER1_BUG
		if( iLevel >= iReqLevel && iZen >= iReqZen && Hero->PK<PVP_MURDERER2
#endif	// ASG_FIX_MOVE_WIN_MURDERER1_BUG
#endif // _VS2008PORTING
#ifdef CSK_LUCKY_SEAL////////////////////////////////////////////////////////////
#ifdef PBG_MOD_VIEMAPMOVE
	#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE || defined LDK_ADD_INGAMESHOP_LIMIT_MOVE_WINDOW
			&& IsMapMove( (*li)->_ReqInfo.szSubMapName )
	#endif //SELECTED_LANGUAGE == LANGUAGE_JAPANESE || defined LDK_ADD_INGAMESHOP_LIMIT_MOVE_WINDOW
#else //PBG_MOD_VIEMAPMOVE
	#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE || SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE || defined LDK_ADD_INGAMESHOP_LIMIT_MOVE_WINDOW
			&& IsMapMove( (*li)->_ReqInfo.szSubMapName )
	#endif //SELECTED_LANGUAGE == LANGUAGE_JAPANESE || SELECTED_LANGUAGE == LANGUAGE_VIETNAMESE || defined LDK_ADD_INGAMESHOP_LIMIT_MOVE_WINDOW
#endif //PBG_MOD_VIEMAPMOVE
#endif //CSK_LUCKY_SEAL////////////////////////////////////////////////////////////
			)
		{
			ITEM* pEquipedRightRing = &CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];
			ITEM* pEquipedLeftRing = &CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];
			ITEM* pEquipedHelper = &CharacterMachine->Equipment[EQUIPMENT_HELPER];
			ITEM* pEquipedWing = &CharacterMachine->Equipment[EQUIPMENT_WING];
			
			// 이카루스 일때 예외처리
			if(strcmp((*li)->_ReqInfo.szMainMapName,GlobalText[55])==0)		// 55 "이카루스"
			{
				// 펜릴/디노란트/군주의망토/날개를 착용해야 이동가능
				// 변신반지 시리즈 착용시 이동불가
				if( 
					(
					pEquipedHelper->Type == ITEM_HELPER+37 			// 펜릴			
#ifndef PSW_BUGFIX_ICARUS_MOVE_UNIRIA    // 코드 정리 할것 !!!
					|| pEquipedHelper->Type == ITEM_HELPER+2		// 유니리아
#endif //PSW_BUGFIX_ICARUS_MOVE_UNIRIA   // 코드 정리 할것 !!!
					|| pEquipedHelper->Type == ITEM_HELPER+3		// 디노란트
#ifdef KJH_FIX_WOPS_K26606_TRADE_WING_IN_IKARUS
					|| pEquipedHelper->Type == ITEM_HELPER+4		// 다크호스
#endif // KJH_FIX_WOPS_K26606_TRADE_WING_IN_IKARUS
					|| pEquipedWing->Type == ITEM_HELPER+30			// 군주의 망토
#ifdef ADD_ALICE_WINGS_1		 
					|| (pEquipedWing->Type >= ITEM_WING+36 && pEquipedWing->Type <= ITEM_WING+43)			
#else	// ADD_ALICE_WINGS_1
					|| (pEquipedWing->Type >= ITEM_WING+36 && pEquipedWing->Type <= ITEM_WING+40) 
#endif	// ADD_ALICE_WINGS_1
					|| (pEquipedWing->Type >= ITEM_WING && pEquipedWing->Type <= ITEM_WING+6) 
#ifdef LDK_ADD_INGAMESHOP_SMALL_WING			// 기간제 날개 작은(군망, 재날, 요날, 천날, 사날)
					|| ( ITEM_WING+130 <= pEquipedWing->Type && pEquipedWing->Type <= ITEM_WING+134 )
#endif //LDK_ADD_INGAMESHOP_SMALL_WING
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
					|| (pEquipedWing->Type >= ITEM_WING+49 && pEquipedWing->Type <= ITEM_WING+50)
					|| (pEquipedWing->Type == ITEM_WING+135)
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
					)
#ifdef KJH_FIX_WOPS_26619_CANMOVE_IKARUS_BY_UNIRIA
					&& !( pEquipedHelper->Type == ITEM_HELPER+2 )	// 유니리아
#endif // KJH_FIX_WOPS_26619_CANMOVE_IKARUS_BY_UNIRIA
#ifdef KJH_FIX_MOVE_ICARUS_EQUIPED_PANDA_CHANGE_RING
					&& ( g_ChangeRingMgr->CheckBanMoveIcarusMap(pEquipedRightRing->Type, pEquipedLeftRing->Type) == false )
#else // KJH_FIX_MOVE_ICARUS_EQUIPED_PANDA_CHANGE_RING
					&& ( g_ChangeRingMgr->CheckMoveMap(pEquipedRightRing->Type, pEquipedLeftRing->Type) == false )
#endif // KJH_FIX_MOVE_ICARUS_EQUIPED_PANDA_CHANGE_RING
					)
				{
					(*li)->_bCanMove = true;
				}
				else
				{
					(*li)->_bCanMove = false;
				}
			}
			// 아틀란스 시리즈 일때 예외처리
			else if(strncmp((*li)->_ReqInfo.szMainMapName, GlobalText[37], 8)==0)	// 37 "아틀란스"
			{
				// 유니리아/디노란트를 탄 상태에서는 아틀란스 이동불가
				if(pEquipedHelper->Type == ITEM_HELPER+2 || pEquipedHelper->Type == ITEM_HELPER+3)
				{
					(*li)->_bCanMove = false;
				}
				else
				{
					(*li)->_bCanMove = true;
				}
			}
#ifdef CSK_MOD_MOVE_COMMAND_WINDOW
			// 669 "로랜협곡"
			else if(strcmp((*li)->_ReqInfo.szMainMapName,GlobalText[669]) == 0)
			{
				// 클라이언트에서 수성측길드, 수성측연합인가 판별할 길이 없다. [2/17/2009 nukun]
				// 추후 서버와의 연계를 통해 판별루틴을 넣어야 한다. [2/17/2009 nukun]
				if(m_bCastleOwner == true)
				{
					(*li)->_bCanMove = true;
				}
				else
				{
					(*li)->_bCanMove = false;
				}
			}
#endif // CSK_MOD_MOVE_COMMAND_WINDOW
#ifdef KJH_MOD_BTS208_CANNOT_MOVE_TO_VULCANUS_IN_NONPVP_SERVER
			else if( ( g_ServerListManager->IsNonPvP() == true ) && (strcmp((*li)->_ReqInfo.szMainMapName, GlobalText[2686]) == 0) )
			{
				// NonPVP 서버에서는 불카누스 이동불가
				(*li)->_bCanMove = false;
			}
#endif // KJH_MOD_BTS208_CANNOT_MOVE_TO_VULCANUS_IN_NONPVP_SERVER
			else
			{
				(*li)->_bCanMove = true;
			}	

#ifdef CSK_MOD_MOVE_COMMAND_WINDOW
			// 칼리마검사
			for(int i=0; i<8; ++i)
			{
				char strNewMapName[64];
				// 58 "칼리마"
				sprintf(strNewMapName, "%s%d", GlobalText[58], i);
				
				if(strcmp((*li)->_ReqInfo.szMainMapName, strNewMapName) == 0)
				{
					// PC방인지 안닌지
					if(CPCRoomPtSys::Instance().IsPCRoom() == true 
#ifdef CSK_FIX_BLUELUCKYBAG_MOVECOMMAND
						// 파란복주머니 사용했을때 가능
						|| g_pBlueLuckyBagEvent->IsEnableBlueLuckyBag() == true
#endif // CSK_FIX_BLUELUCKYBAG_MOVECOMMAND
						)
					{
						(*li)->_bCanMove = true;
					}
					else
					{
						(*li)->_bCanMove = false;
					}

					break;
				}
			}
#endif // CSK_MOD_MOVE_COMMAND_WINDOW
		}
#ifdef ASG_ADD_GENS_SYSTEM
		// 이동 가능한 지역이고 분쟁 지역이고 겐스에 가입되어 있지 않다면 이동 불가.
		if ((*li)->_bCanMove && (*li)->_bStrife && 0 == Hero->m_byGensInfluence)
			(*li)->_bCanMove = false;
#endif	// ASG_ADD_GENS_SYSTEM
	}
	//----------------------------------------------
	//----------------------------------------------
}

//---------------------------------------------------------------------------------------------
// BtnProcess
bool SEASON3B::CNewUIMoveCommandWindow::BtnProcess()
{
	int iX, iY;

#ifdef YDG_MOD_PROTECT_AUTO_V4
	CMoveCommandWindowEncrypt enc;
#endif	// YDG_MOD_PROTECT_AUTO_V4
	if( CheckMouseIn( m_ScrollBtnPos.x, m_ScrollBtnPos.y, MOVECOMMAND_SCROLLBTN_WIDTH, MOVECOMMAND_SCROLLBTN_HEIGHT ))
	{	
		if(IsPress(VK_LBUTTON))
		{
			m_iScrollBtnMouseEvent = MOVECOMMAND_MOUSEBTN_CLICKED;
			m_iMousePosY = MouseY;
#ifdef KJH_FIX_MOVECOMMAND_WINDOW_SIZE
			m_iAcumMoveMouseScrollPixel = 0;
#endif // KJH_FIX_MOVECOMMAND_WINDOW_SIZE
		}	
	}

	if(IsRelease(VK_LBUTTON))
	{
 		m_iScrollBtnMouseEvent = MOVECOMMAND_MOUSEBTN_NORMAL;
#ifdef KJH_FIX_MOVECOMMAND_WINDOW_SIZE
		m_iAcumMoveMouseScrollPixel = 0;
#endif // KJH_FIX_MOVECOMMAND_WINDOW_SIZE
	}
	
//	if( CheckMouseIn( 0, m_ScrollBarPos.y, 640, m_iTotalMoveScrBtnPixel) )
//	{
#ifdef KJH_FIX_MOVECOMMAND_WINDOW_SIZE
		if( m_iScrollBtnMouseEvent == MOVECOMMAND_MOUSEBTN_CLICKED && m_icurMoveScrBtnPixelperStep > 0 )
#else // KJH_FIX_MOVECOMMAND_WINDOW_SIZE			// 정리할때 지워야 하는 소스
		if( m_iScrollBtnMouseEvent == MOVECOMMAND_MOUSEBTN_CLICKED && m_iHeightByMoveStep > 0 )
#endif // KJH_FIX_MOVECOMMAND_WINDOW_SIZE			// 정리할때 지워야 하는 소스
		{
			//MoveScrollBtn(MouseY-m_iMousePosY);
			int iMoveValue = MouseY-m_iMousePosY;
				
			if( iMoveValue < 0 )
			{
#ifdef KJH_FIX_MOVECOMMAND_WINDOW_SIZE
				if( MouseY <= m_ScrollBtnPos.y+(MOVECOMMAND_SCROLLBTN_HEIGHT/2))
				{
					if( -(iMoveValue) > (m_iTotalMoveScrBtnPixel-m_iRemainMoveScrBtnPixel) )
					{
						iMoveValue = -(m_iTotalMoveScrBtnPixel-m_iRemainMoveScrBtnPixel);
					}
					ScrollUp(-iMoveValue);
				}
#else // KJH_FIX_MOVECOMMAND_WINDOW_SIZE
				ScrollUp(iMoveValue);
#endif // KJH_FIX_MOVECOMMAND_WINDOW_SIZE
			}
			else if( iMoveValue > 0 )
			{
#ifdef KJH_FIX_MOVECOMMAND_WINDOW_SIZE
				if( MouseY >= m_ScrollBtnPos.y+(MOVECOMMAND_SCROLLBTN_HEIGHT/2))
				{
					if( iMoveValue > m_iRemainMoveScrBtnPixel )
					{
						iMoveValue = m_iRemainMoveScrBtnPixel;
					}
					ScrollDown(iMoveValue);
				}
#else // KJH_FIX_MOVECOMMAND_WINDOW_SIZE
				ScrollDown(iMoveValue);
#endif // KJH_FIX_MOVECOMMAND_WINDOW_SIZE
			}
			m_iMousePosY = MouseY;	
			//UpdateScrolling();
		}
//	}
// 	else
// 	{
// 		m_iScrollBtnMouseEvent = MOVECOMMAND_MOUSEBTN_NORMAL;
// 		m_iAcumMoveMouseScrollPixel = 0;
// 	}
	if( CheckMouseIn( m_Pos.x, m_Pos.y, m_MapNameUISize.x, m_MapNameUISize.y ) && IsPress(VK_LBUTTON))
	{
		SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
	}

	// 맵 정보 셋팅
	SettingCanMoveMap();

	// 맵 이동창에 마우스를 올렸을때 계산
	if( CheckMouseIn( m_Pos.x, m_Pos.y, m_MapNameUISize.x, m_MapNameUISize.y ) )
	{
#ifdef KJH_FIX_MOVECOMMAND_WINDOW_SIZE
		if( m_icurMoveScrBtnPixelperStep > 0 )
		{
#ifdef CSK_MOD_PROTECT_AUTO_V2
#ifdef YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V2
			if (!g_pProtectAuto->IsNewVersion());
			else
#endif	// YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V2
			// 오토마우스 작동중이면 가끔 휠기능 먹통되게 수정 
			if(g_pProtectAuto->IsStartAuto() == true && rand()%10 == 0)
			{
				MouseWheel = 0;
			}
#endif // CSK_MOD_PROTECT_AUTO_V2
#ifdef YDG_MOD_PROTECT_AUTO_V3
			// 휠 3회~7회 조작시 1회 먹통 되도록
#ifdef YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V3
			if (!g_pProtectAuto->IsNewVersion());
			else
#endif	// YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V3
			if (MouseWheel != 0)
			{
				++m_iWheelCounter;
				if (m_iWheelCounter >= 3 && rand()%6 == 0)
				{
					MouseWheel = 0;
					m_iWheelCounter = 2;
				}
			}
#endif	// YDG_MOD_PROTECT_AUTO_V3
			
			// 휠버튼 처리
			if( MouseWheel > 0 )
			{
				ScrollUp(m_icurMoveScrBtnPixelperStep);
			}
			else if( MouseWheel < 0 )
			{
				ScrollDown(m_icurMoveScrBtnPixelperStep);
			}

#ifdef YDG_MOD_PROTECT_AUTO_V6
			// 랜덤확률로 2번 스크롤 되게 한다 (오토방지)
			if (m_iWheelCounter >= 3 && rand()%6 == 0)
			{
				if( MouseWheel > 0 )
				{
					ScrollUp(m_icurMoveScrBtnPixelperStep);
				}
				else if( MouseWheel < 0 )
				{
					ScrollDown(m_icurMoveScrBtnPixelperStep);
				}
				m_iWheelCounter = 2;
			}
#endif	// YDG_MOD_PROTECT_AUTO_V6
		}
#else // KJH_FIX_MOVECOMMAND_WINDOW_SIZE			// 정리할때 지워야 하는 소스
		if( m_iHeightByMoveStep > 0 )
		{
#ifdef CSK_MOD_PROTECT_AUTO_V1
			// 오토마우스 작동중이면 가끔 휠기능 먹통되게 수정 
// 			if(g_pProtectAuto->IsStartAuto() == true && rand()%10 == 0)
// 			{
// 				MouseWheel = 0;
// 			}
#endif // CSK_MOD_PROTECT_AUTO_V1
			
			// 휠버튼 처리
			if( MouseWheel > 0 )
			{
				ScrollUp(-m_iHeightByMoveStep);
			}
			else if( MouseWheel < 0 )
			{
				ScrollDown(m_iHeightByMoveStep);
			}
		}
#endif // KJH_FIX_MOVECOMMAND_WINDOW_SIZE			// 정리할때 지워야 하는 소스

		MouseWheel = 0;

		std::list<CMoveCommandData::MOVEINFODATA*>::iterator li = m_listMoveInfoData.begin();
		int iCurRenderTextIndex = 0;
		for( int i=0; i<m_iRenderEndTextIndex ; i++, li++ )
		{
			if( li == m_listMoveInfoData.end() )
			{
				break;
			}

			if( i<m_iRenderStartTextIndex )
			{
				continue;
			}
			iX = m_StartMapNamePos.x;
			iY = m_StartMapNamePos.y + ( m_iRealFontHeight * iCurRenderTextIndex );
			// 어느 맵이름이 선택되었나 (마우스를 올렸냐?)
			if( CheckMouseIn( iX, iY, m_MapNameUISize.x-22, m_iRealFontHeight ) )
			{
				if( (*li)->_bCanMove == true )
				{
					(*li)->_bSelected = true;
					
					if( SEASON3B::IsRelease(VK_LBUTTON) )
					{
#ifdef CSK_MOD_PROTECT_AUTO_V2
#ifdef YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V2
						if (!g_pProtectAuto->IsNewVersion());
						else
#endif	// YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V2
						// 오토프로그램을 작동시켰다면
						if(g_pProtectAuto->IsStartAuto() == true)
						{
							// 10번에 1번 정도로 데비아스2맵으로 보내버린다.
							if(rand()%10 == 0)
							{
								// 맵 이동 패킷 날림
#ifdef YDG_ADD_MOVE_COMMAND_PROTOCOL
								SendRequestMoveMap(g_pMoveCommandWindow->GetMoveCommandKey(), 5);	// 데비아스2
#else	// YDG_ADD_MOVE_COMMAND_PROTOCOL
								// 260 "/이동"
								std::string strChat = GlobalText[260];
								strChat += ' ';
								strChat += "데비아스2";
								SendChat((char*)strChat.c_str());
#endif	// YDG_ADD_MOVE_COMMAND_PROTOCOL
								g_pNewUISystem->Hide( SEASON3B::INTERFACE_MOVEMAP );	
							}
						}
#endif // CSK_MOD_PROTECT_AUTO_V2	

#ifdef LJH_ADD_SAVEOPTION_WHILE_MOVING_FROM_OR_TO_DIFF_SERVER
						// 현재 위치하고 있는 맵이나 이동하려는 맵이 다른 서버에 존재(로랜협곡, 로랜시장)
						if (IsTheMapInDifferentServer(World, (*li)->_ReqInfo.index))
						{
							SaveOptions();
						}
#endif //LJH_ADD_SAVEOPTION_WHILE_MOVING_FROM_OR_TO_DIFF_SERVER
						
						// 맵 이동 패킷 날림
#ifdef YDG_ADD_MOVE_COMMAND_PROTOCOL
						SendRequestMoveMap(g_pMoveCommandWindow->GetMoveCommandKey(), (*li)->_ReqInfo.index);
#else	// YDG_ADD_MOVE_COMMAND_PROTOCOL
						// 260 "/이동"
						std::string strChat = GlobalText[260];
						strChat += ' ';
						strChat += (*li)->_ReqInfo.szMainMapName;
						SendChat((char*)strChat.c_str());
#endif	// YDG_ADD_MOVE_COMMAND_PROTOCOL
						g_pNewUISystem->Hide( SEASON3B::INTERFACE_MOVEMAP );

#ifdef CSK_MOD_MOVE_COMMAND_WINDOW
						ResetWindowOpenCount();
#endif // CSK_MOD_MOVE_COMMAND_WINDOW
						return true;	
					}
				}	
			}

			iCurRenderTextIndex++;
			
			// 닫기버튼
			if( SEASON3B::IsRelease(VK_LBUTTON)  
#ifdef CSK_MOD_MOVE_COMMAND_WINDOW
				&& CheckMouseIn( 3, m_Pos.y + m_MapNameUISize.y-m_iRealFontHeight-6, m_MapNameUISize.x-5, m_iRealFontHeight ))
#else // CSK_MOD_MOVE_COMMAND_WINDOW
				&& CheckMouseIn( 3, m_MapNameUISize.y-m_iRealFontHeight-6, m_MapNameUISize.x-5, m_iRealFontHeight ))
#endif // CSK_MOD_MOVE_COMMAND_WINDOW
			{
				g_pNewUISystem->Hide( SEASON3B::INTERFACE_MOVEMAP );
				return true;
			}
		}
	}
	return false;
}

//---------------------------------------------------------------------------------------------
// UpdateMouseEvent
bool SEASON3B::CNewUIMoveCommandWindow::UpdateMouseEvent()
{
	// 버튼 처리
	if( true == BtnProcess() )	// 처리가 완료 되었다면
		return false;

#ifdef YDG_MOD_PROTECT_AUTO_V4
	CMoveCommandWindowEncrypt enc;
#endif	// YDG_MOD_PROTECT_AUTO_V4
	if( m_iScrollBtnMouseEvent == MOVECOMMAND_MOUSEBTN_CLICKED )
		return false;
	
	if( CheckMouseIn( m_Pos.x, m_Pos.y, m_MapNameUISize.x, m_MapNameUISize.y ) )
		return false;

	return true;
}

//---------------------------------------------------------------------------------------------
// UpdateKeyEvent
bool SEASON3B::CNewUIMoveCommandWindow::UpdateKeyEvent()
{
	if( IsVisible() )
	{
		if(SEASON3B::IsPress(VK_ESCAPE) == true)
		{
			Show( false );
			PlayBuffer(SOUND_CLICK01);
			return false;
		}
	}
	return true;
}

//---------------------------------------------------------------------------------------------
// Update
bool SEASON3B::CNewUIMoveCommandWindow::Update()
{
	if( !IsVisible() )
	{
		return true;
	}

	UpdateScrolling();

 	return true;
}

//---------------------------------------------------------------------------------------------
// ScrollUp
void SEASON3B::CNewUIMoveCommandWindow::ScrollUp(int iMoveValue)
{
#ifdef YDG_MOD_PROTECT_AUTO_V4
	CMoveCommandWindowEncrypt enc;
#endif	// YDG_MOD_PROTECT_AUTO_V4
#ifdef KJH_FIX_MOVECOMMAND_WINDOW_SIZE
	if( m_iRemainMoveScrBtnperStep < m_iTotalMoveScrBtnperStep )
	{
		int iMovePixel = 0;
		m_iAcumMoveMouseScrollPixel -= iMoveValue;
		if( (-m_iAcumMoveMouseScrollPixel) < m_icurMoveScrBtnPixelperStep )
		{
			// 마우스로 움직인 픽셀이 현재 움직여야 하는 픽셀을 넘지 않으면 움직인 픽셀값만 누적시키고 return;
			return;
		}
		else
		{
			//m_iAcumMoveMouseScrollPixel = 0;
			RecursiveCalcScroll(m_iAcumMoveMouseScrollPixel, &iMovePixel, false);
#ifdef CONSOLE_DEBUG
			g_ConsoleDebug->Write(MCD_NORMAL, "m_ScrollBtnPos.y : (%d)", m_ScrollBtnPos.y);
#endif // CONSOLE_DEBUG
			
			m_ScrollBtnPos.y += iMovePixel;
			m_iAcumMoveMouseScrollPixel -= iMovePixel;
#ifdef CONSOLE_DEBUG
			g_ConsoleDebug->Write(MCD_NORMAL, "m_ScrollBtnPos.y : (%d)", m_ScrollBtnPos.y);	
			g_ConsoleDebug->Write(MCD_NORMAL, "iMoveValue : (%d)", -iMoveValue);
			g_ConsoleDebug->Write(MCD_NORMAL, "m_iRemainMoveScrBtnPixel : (%d)", m_iRemainMoveScrBtnPixel);
			g_ConsoleDebug->Write(MCD_NORMAL, "m_icurMoveScrBtnPixelperStep : (%d)", m_icurMoveScrBtnPixelperStep);
			g_ConsoleDebug->Write(MCD_NORMAL, "m_iRemainMoveScrBtnperStep : (%d)", m_iRemainMoveScrBtnperStep);
			g_ConsoleDebug->Write(MCD_NORMAL, "m_iAcumMoveMouseScrollPixel : (%d)", m_iAcumMoveMouseScrollPixel);
#endif // CONSOLE_DEBUG
			
		}
	}
#else // KJH_FIX_MOVECOMMAND_WINDOW_SIZE
#ifdef LDK_FIX_MOVEWINDOW_SCROLL_BUG
	m_ScrollBtnPos.y += iMoveValue;
#else //LDK_FIX_MOVEWINDOW_SCROLL_BUG
 	m_ScrollBtnPos.y -= iMoveValue;
#endif //LDK_FIX_MOVEWINDOW_SCROLL_BUG

	// 예외처리
	if( m_ScrollBtnPos.y <= m_ScrollBarPos.y || MouseY < m_ScrollBarPos.y+(MOVECOMMAND_SCROLLBTN_HEIGHT/2))
	{
		m_ScrollBtnPos.y = m_ScrollBarPos.y;
	}
#endif // KJH_FIX_MOVECOMMAND_WINDOW_SIZE
}

//---------------------------------------------------------------------------------------------
// ScrollDown
void SEASON3B::CNewUIMoveCommandWindow::ScrollDown(int iMoveValue)
{
#ifdef YDG_MOD_PROTECT_AUTO_V4
	CMoveCommandWindowEncrypt enc;
#endif	// YDG_MOD_PROTECT_AUTO_V4
#ifdef KJH_FIX_MOVECOMMAND_WINDOW_SIZE
	if( m_iRemainMoveScrBtnperStep > 0 )
	{
		int iMovePixel = 0;
		m_iAcumMoveMouseScrollPixel += iMoveValue;
		if( m_iAcumMoveMouseScrollPixel < m_icurMoveScrBtnPixelperStep )
		{
			// 마우스로 움직인 픽셀이 현재 움직여야 하는 픽셀을 넘지 않으면 
			// 움직인 픽셀값만 누적시키고 return;
			return;
		}
		else
		{	
			//m_iAcumMoveMouseScrollPixel = 0;
			RecursiveCalcScroll(m_iAcumMoveMouseScrollPixel, &iMovePixel, true);
#ifdef CONSOLE_DEBUG
			g_ConsoleDebug->Write(MCD_NORMAL, "m_ScrollBtnPos.y : (%d)", m_ScrollBtnPos.y);	
#endif // CONSOLE_DEBUG
			m_iAcumMoveMouseScrollPixel -= iMovePixel;
			m_ScrollBtnPos.y += iMovePixel;
#ifdef CONSOLE_DEBUG
			g_ConsoleDebug->Write(MCD_NORMAL, "m_ScrollBtnPos.y : (%d)", m_ScrollBtnPos.y);	
			g_ConsoleDebug->Write(MCD_NORMAL, "iMoveValue : (%d)", iMoveValue);
			g_ConsoleDebug->Write(MCD_NORMAL, "m_iRemainMoveScrBtnPixel : (%d)", m_iRemainMoveScrBtnPixel);
			g_ConsoleDebug->Write(MCD_NORMAL, "m_icurMoveScrBtnPixelperStep : (%d)", m_icurMoveScrBtnPixelperStep);
			g_ConsoleDebug->Write(MCD_NORMAL, "m_iRemainMoveScrBtnperStep : (%d)", m_iRemainMoveScrBtnperStep);
			g_ConsoleDebug->Write(MCD_NORMAL, "m_iAcumMoveMouseScrollPixel : (%d)", m_iAcumMoveMouseScrollPixel);
#endif // CONSOLE_DEBUG
			
		}
	}
#else // KJH_FIX_MOVECOMMAND_WINDOW_SIZE
	m_ScrollBtnPos.y += iMoveValue;
	// 	// 예외처리
	if( m_ScrollBtnPos.y >= m_ScrollBarPos.y+m_iScrollBarHeightPixel-MOVECOMMAND_SCROLLBTN_HEIGHT
			 || MouseY > m_ScrollBarPos.y+m_iScrollBarHeightPixel-(MOVECOMMAND_SCROLLBTN_HEIGHT/2))
	{
		m_ScrollBtnPos.y = m_ScrollBarPos.y+m_iScrollBarHeightPixel-MOVECOMMAND_SCROLLBTN_HEIGHT;
	}
#endif // KJH_FIX_MOVECOMMAND_WINDOW_SIZE
}

#ifdef KJH_FIX_MOVECOMMAND_WINDOW_SIZE
void SEASON3B::CNewUIMoveCommandWindow::RecursiveCalcScroll(IN int piScrollValue, OUT int* piMovePixel, bool bSign /* = true */)
{
#ifdef YDG_MOD_PROTECT_AUTO_V4
	CMoveCommandWindowEncrypt enc;
#endif	// YDG_MOD_PROTECT_AUTO_V4
	// 마우스 스크롤 이후 변경돼는 변수들은 이 함수에서 모두 계산해 준다.
	// 다른곳에서 처리하다가 꼬일수 있음...

	// DownScroll
	if( bSign == true )
	{
		if( m_iRemainMoveScrBtnperStep > 0)
		{	

			m_iRemainMoveScrBtnperStep--;
			m_iRemainMoveScrBtnPixel -= m_icurMoveScrBtnPixelperStep;
			piScrollValue -= m_icurMoveScrBtnPixelperStep;
			(*piMovePixel) += m_icurMoveScrBtnPixelperStep;

			if( m_iRemainMoveScrBtnperStep > m_iTotalNumMinMoveScrBtnperStep )
			{
				m_icurMoveScrBtnPixelperStep = m_iMaxMoveScrBtnPixelperStep;
			}
			else 
			{
				m_icurMoveScrBtnPixelperStep = m_iMinMoveScrBtnPixelperStep;
			}


			
			// 아직 스크롤할 값이 남아있으면 재귀호출
			if( piScrollValue >= m_icurMoveScrBtnPixelperStep )
			{
				RecursiveCalcScroll(piScrollValue, piMovePixel, bSign);
			}
		}
		else
		{
			(*piMovePixel) = piScrollValue;
		}
	}
	// UpScroll
	else
	{
		if( m_iRemainMoveScrBtnperStep < m_iTotalMoveScrBtnperStep )
		{		
			m_iRemainMoveScrBtnperStep++;
			m_iRemainMoveScrBtnPixel += m_icurMoveScrBtnPixelperStep;
			piScrollValue += m_icurMoveScrBtnPixelperStep;
			(*piMovePixel) -= m_icurMoveScrBtnPixelperStep;

			if( m_iRemainMoveScrBtnperStep >= m_iTotalNumMinMoveScrBtnperStep)
			{
				m_icurMoveScrBtnPixelperStep = m_iMaxMoveScrBtnPixelperStep;
			}
			else 
			{
				m_icurMoveScrBtnPixelperStep = m_iMinMoveScrBtnPixelperStep;
			}

			// 아직 스크롤할 값이 남아있으면 재귀호출
			if( (-piScrollValue) >= m_icurMoveScrBtnPixelperStep )
			{
				RecursiveCalcScroll(piScrollValue, piMovePixel, bSign);
			}
		}
		else
		{
			(*piMovePixel) = piScrollValue;
		}
	}

	return;
}
#endif // KJH_FIX_MOVECOMMAND_WINDOW_SIZE

//---------------------------------------------------------------------------------------------
// UpdateScrolling
void SEASON3B::CNewUIMoveCommandWindow::UpdateScrolling()
{
#ifdef YDG_MOD_PROTECT_AUTO_V4
	CMoveCommandWindowEncrypt enc;
#endif	// YDG_MOD_PROTECT_AUTO_V4

#ifdef KJH_FIX_MOVECOMMAND_WINDOW_SIZE
	m_iRenderStartTextIndex = m_iTotalMoveScrBtnperStep - m_iRemainMoveScrBtnperStep;
#else // KJH_FIX_MOVECOMMAND_WINDOW_SIZE
	m_iRenderStartTextIndex = ((m_ScrollBtnPos.y-m_ScrollBarPos.y)/m_iHeightByMoveStep);
#endif // KJH_FIX_MOVECOMMAND_WINDOW_SIZE

#ifdef CSK_MOD_PROTECT_AUTO_V1
	m_iRenderEndTextIndex = m_iRenderStartTextIndex + m_iTextLine;
#else // CSK_MOD_PROTECT_AUTO_V1
	m_iRenderEndTextIndex = m_iRenderStartTextIndex + MOVECOMMAND_MAX_RENDER_TEXTLINE;
#endif // CSK_MOD_PROTECT_AUTO_V1
	
#ifdef _VS2008PORTING
	if( m_iRenderEndTextIndex > (int)m_listMoveInfoData.size() )
#else // _VS2008PORTING
	if( m_iRenderEndTextIndex > m_listMoveInfoData.size() )
#endif // _VS2008PORTING
	{
		m_iRenderEndTextIndex -= (m_iRenderEndTextIndex-m_listMoveInfoData.size());
	}
}

//---------------------------------------------------------------------------------------------
// RenderFrame
// UI Frame을 렌더한다.
void SEASON3B::CNewUIMoveCommandWindow::RenderFrame()
{
#ifdef YDG_MOD_PROTECT_AUTO_V4
	CMoveCommandWindowEncrypt enc;
#endif	// YDG_MOD_PROTECT_AUTO_V4
	// 배경
#ifdef CSK_MOD_PROTECT_AUTO_V1
	glColor4f(0.0f, 0.0f, 0.0f, m_fBackgroundAlpha);
#else // CSK_MOD_PROTECT_AUTO_V1
	glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
#endif // CSK_MOD_PROTECT_AUTO_V1
	
	RenderColor((float)m_Pos.x, (float)m_Pos.y, (float)m_MapNameUISize.x, (float)m_MapNameUISize.y);

	// 닫기버튼 배경색.
#ifdef YDG_MOD_PROTECT_AUTO_V3
#ifdef YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V3
	if (!g_pProtectAuto->IsNewVersion())
		glColor4f ( 0.6f, 0.f, 0.f, 1.f );
	else
#endif	// YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V3
		glColor4f ( 0.6f, 0.f, 0.f, m_fBackgroundAlpha );
#else	// YDG_MOD_PROTECT_AUTO_V3
	glColor4f ( 0.6f, 0.f, 0.f, 1.f );
#endif	// YDG_MOD_PROTECT_AUTO_V3
#ifdef CSK_MOD_MOVE_COMMAND_WINDOW
	RenderColor( m_StartMapNamePos.x, m_Pos.y + m_MapNameUISize.y-m_iRealFontHeight-6, m_MapNameUISize.x-5, m_iRealFontHeight );
#else // CSK_MOD_MOVE_COMMAND_WINDOW
	RenderColor( m_StartMapNamePos.x, m_MapNameUISize.y-m_iRealFontHeight-6, m_MapNameUISize.x-5, m_iRealFontHeight );
#endif // CSK_MOD_MOVE_COMMAND_WINDOW
	
#ifdef YDG_MOD_PROTECT_AUTO_V3
#ifdef YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V3
	if (!g_pProtectAuto->IsNewVersion())
		glColor4f ( 1.f, 1.f, 1.f, 1.f );
	else
#endif	// YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V3
		glColor4f ( 1.f, 1.f, 1.f, m_fBackgroundAlpha );
#else	// YDG_MOD_PROTECT_AUTO_V3
	glColor4f ( 1.f, 1.f, 1.f, 1.f );
#endif	// YDG_MOD_PROTECT_AUTO_V3

	EnableAlphaTest();

	// 스크롤바
	RenderImage(IMAGE_MOVECOMMAND_SCROLL_TOP, m_ScrollBarPos.x, m_ScrollBarPos.y, 
					MOVECOMMAND_SCROLLBAR_TOP_WIDTH, MOVECOMMAND_SCROLLBAR_TOP_HEIGHT );		// TOP
	
#ifdef _VS2008PORTING
#ifdef ASG_FIX_MOVECMD_WIN_SCRBAR
	int i;
	for( i=0 ; i<m_iScrollBarMiddleNum ; i++ )
	{
		RenderImage(IMAGE_MOVECOMMAND_SCROLL_MIDDLE, m_ScrollBarPos.x, 
			m_ScrollBarPos.y+MOVECOMMAND_SCROLLBAR_TOP_HEIGHT+(i*MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT),
			MOVECOMMAND_SCROLLBAR_TOP_WIDTH, MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT );	// MIDDLE
	}
	if( m_iScrollBarMiddleRemainderPixel > 0 )
	{
		RenderImage(IMAGE_MOVECOMMAND_SCROLL_MIDDLE, m_ScrollBarPos.x, 
			m_ScrollBarPos.y+MOVECOMMAND_SCROLLBAR_TOP_HEIGHT+(i*MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT),
			MOVECOMMAND_SCROLLBAR_TOP_WIDTH, m_iScrollBarMiddleRemainderPixel );	// MIDDLE 나머지
	}
#else	// ASG_FIX_MOVECMD_WIN_SCRBAR
	int icntText = 0;
	for( int i=0 ; i<m_iScrollBarMiddleNum ; i++ )
	{
		icntText = i;
		RenderImage(IMAGE_MOVECOMMAND_SCROLL_MIDDLE, m_ScrollBarPos.x, 
			m_ScrollBarPos.y+MOVECOMMAND_SCROLLBAR_TOP_HEIGHT+(i*MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT),
			MOVECOMMAND_SCROLLBAR_TOP_WIDTH, MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT );	// MIDDLE
	}
	if( m_iScrollBarMiddleRemainderPixel > 0 )
	{
		RenderImage(IMAGE_MOVECOMMAND_SCROLL_MIDDLE, m_ScrollBarPos.x, 
			m_ScrollBarPos.y+MOVECOMMAND_SCROLLBAR_TOP_HEIGHT+(icntText*MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT),
			MOVECOMMAND_SCROLLBAR_TOP_WIDTH, m_iScrollBarMiddleRemainderPixel );	// MIDDLE 나머지
	}
#endif	// ASG_FIX_MOVECMD_WIN_SCRBAR
#else // _VS2008PORTING
	for( int i=0 ; i<m_iScrollBarMiddleNum ; i++ )
	{
		RenderImage(IMAGE_MOVECOMMAND_SCROLL_MIDDLE, m_ScrollBarPos.x, 
						m_ScrollBarPos.y+MOVECOMMAND_SCROLLBAR_TOP_HEIGHT+(i*MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT),
						MOVECOMMAND_SCROLLBAR_TOP_WIDTH, MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT );	// MIDDLE
	}
	if( m_iScrollBarMiddleRemainderPixel > 0 )
	{
		RenderImage(IMAGE_MOVECOMMAND_SCROLL_MIDDLE, m_ScrollBarPos.x, 
						m_ScrollBarPos.y+MOVECOMMAND_SCROLLBAR_TOP_HEIGHT+(i*MOVECOMMAND_SCROLLBAR_MIDDLE_HEIGHT),
						MOVECOMMAND_SCROLLBAR_TOP_WIDTH, m_iScrollBarMiddleRemainderPixel );	// MIDDLE 나머지
	}
#endif // _VS2008PORTING

	RenderImage(IMAGE_MOVECOMMAND_SCROLL_BOTTOM, m_ScrollBarPos.x, m_ScrollBarPos.y+m_iScrollBarHeightPixel-MOVECOMMAND_SCROLLBAR_TOP_HEIGHT,
					MOVECOMMAND_SCROLLBAR_TOP_WIDTH, MOVECOMMAND_SCROLLBAR_TOP_HEIGHT );		// BOTTOM

	// 스크롤바 버튼
	if( m_bScrollBtnActive == true )
	{
		if (m_iScrollBtnMouseEvent == MOVECOMMAND_MOUSEBTN_CLICKED) 
		{
			glColor4f(0.7f, 0.7f, 0.7f, 1.0f);
		}
		RenderImage(IMAGE_MOVECOMMAND_SCROLLBAR_ON, m_ScrollBtnPos.x, m_ScrollBtnPos.y, 
			MOVECOMMAND_SCROLLBTN_WIDTH, MOVECOMMAND_SCROLLBTN_HEIGHT);
	}
	else
	{
		RenderImage(IMAGE_MOVECOMMAND_SCROLLBAR_OFF, m_ScrollBtnPos.x, m_ScrollBtnPos.y, 
			MOVECOMMAND_SCROLLBTN_WIDTH, MOVECOMMAND_SCROLLBTN_HEIGHT);
	}

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// 제목
	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetBgColor(0);
#ifdef YDG_MOD_PROTECT_AUTO_V3
#ifdef YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V3
	if (!g_pProtectAuto->IsNewVersion())
		g_pRenderText->SetTextColor(255, 204, 26, 255);
	else
#endif	// YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V3
		g_pRenderText->SetTextColor(255, 204, 26, m_fBackgroundAlpha * 255);
#else	// YDG_MOD_PROTECT_AUTO_V3
	g_pRenderText->SetTextColor(255, 204, 26, 255);
#endif	// YDG_MOD_PROTECT_AUTO_V3
	g_pRenderText->RenderText(m_StartUISubjectName.x, m_StartUISubjectName.y, GlobalText[933], 0 ,0, RT3_WRITE_CENTER);

	// 구분제목
	g_pRenderText->SetFont(g_hFont);
#ifdef YDG_MOD_PROTECT_AUTO_V3
#ifdef YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V3
	if (!g_pProtectAuto->IsNewVersion())
		g_pRenderText->SetTextColor(127, 178, 255, 255);
	else
#endif	// YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V3
		g_pRenderText->SetTextColor(127, 178, 255, m_fBackgroundAlpha * 255);
#else	// YDG_MOD_PROTECT_AUTO_V3
	g_pRenderText->SetTextColor(127, 178, 255, 255);
#endif	// YDG_MOD_PROTECT_AUTO_V3
#ifdef ASG_ADD_GENS_SYSTEM
	g_pRenderText->RenderText(m_StrifePos.x, m_StartUISubjectName.y+20, GlobalText[2988], 0 ,0, RT3_WRITE_CENTER);
#endif	// ASG_ADD_GENS_SYSTEM
	g_pRenderText->RenderText(m_MapNamePos.x, m_StartUISubjectName.y+20, GlobalText[934], 0 ,0, RT3_WRITE_CENTER);
	g_pRenderText->RenderText(m_ReqLevelPos.x, m_StartUISubjectName.y+20, GlobalText[935], 0 ,0, RT3_WRITE_CENTER);
	g_pRenderText->RenderText(m_ReqZenPos.x, m_StartUISubjectName.y+20, GlobalText[936], 0 ,0, RT3_WRITE_CENTER);
}

//---------------------------------------------------------------------------------------------
// Render
bool SEASON3B::CNewUIMoveCommandWindow::Render()
{
	EnableAlphaTest();
	glColor4f(1.f, 1.f, 1.f, 1.f);
	
	g_pRenderText->SetFont( g_hFont );
	g_pRenderText->SetTextColor( 255, 255, 255, 255 );

	RenderFrame();	

	std::list<CMoveCommandData::MOVEINFODATA*>::iterator li = m_listMoveInfoData.begin();
	int iX, iY;

	int iLevel = CharacterAttribute->Level;
	DWORD iZen = CharacterMachine->Gold;
	int iReqLevel;
	char szText[24];

#ifdef YDG_MOD_PROTECT_AUTO_V4
	CMoveCommandWindowEncrypt enc;
#endif	// YDG_MOD_PROTECT_AUTO_V4

	int iCurRenderTextIndex = 0;
	for ( int i=0 ; i<m_iRenderEndTextIndex ; i++, li++ )
    {
		if( li == m_listMoveInfoData.end())
		{
			break;
		}

		if( i < m_iRenderStartTextIndex )
		{
			continue;
		}

		iX = m_StartMapNamePos.x;
		iY = m_StartMapNamePos.y + ( m_iRealFontHeight * iCurRenderTextIndex );

		iReqLevel = (*li)->_ReqInfo.iReqLevel;
		// 마검사/다크로드일때 필요레벨을 계산
		if ( (GetBaseClass(CharacterAttribute->Class)==CLASS_DARK || GetBaseClass(CharacterAttribute->Class)==CLASS_DARK_LORD
#ifdef PBG_ADD_NEWCHAR_MONK
				|| GetBaseClass(CharacterAttribute->Class)==CLASS_RAGEFIGHTER
#endif //PBG_ADD_NEWCHAR_MONK
				) 
			&& (iReqLevel != 400)
#ifdef LDS_ADD_MOVEMAP_UNITEDMARKETPLACE
			&& (iReqLevel != 1)	// 1인경우 나누면 0이 되버리므로 1은 제외.
#endif // LDS_ADD_MOVEMAP_UNITEDMARKETPLACE
			 )
		{
#ifdef CSK_FIX_GM_MOVE
			char strNewMapName[64];
			bool bKalima = false;
			
			// 칼리마1부터 칼리마6까지는 필요레벨 - 20 이고
			// 칼리마7은 필요레벨 그대로
			for(int i=0; i<8; ++i)
			{
				// 58 "칼리마"
				sprintf(strNewMapName, "%s%d", GlobalText[58], i);
				
				if(strcmp((*li)->_ReqInfo.szMainMapName, strNewMapName) == 0)
				{
					bKalima = true;
					break;
				}
			}
			
			if(bKalima == true)
			{
				if(i >= 0 && i <= 6)
				{
					iReqLevel = iReqLevel - 20;
				}
			}
			else
#endif // CSK_FIX_GM_MOVE
			{
				iReqLevel = int(float(iReqLevel)*2.f/3.f);
			}
		}
		
		if( (*li)->_bCanMove == true )
		{
#ifdef CSK_MOD_PROTECT_AUTO_V1
			g_pRenderText->SetTextColor(255, 255, 255, m_iTextAlpha);
#else // CSK_MOD_PROTECT_AUTO_V1
			g_pRenderText->SetTextColor(255, 255, 255, 255);
#endif // CSK_MOD_PROTECT_AUTO_V1
#ifdef ASG_ADD_GENS_SYSTEM
			if ((*li)->_bStrife)
				g_pRenderText->RenderText(m_StrifePos.x, iY, GlobalText[2987], 0 ,0, RT3_WRITE_CENTER);	// 2987	"(분쟁)"
#endif	// ASG_ADD_GENS_SYSTEM
			g_pRenderText->RenderText(m_MapNamePos.x, iY, (*li)->_ReqInfo.szMainMapName, 0 ,0, RT3_WRITE_CENTER);
#ifdef _BLUE_SERVER		//  블루서버에만 적용
#ifdef ASG_ADD_MOVEREQ_TEXT_MAX_LEVEL
			if (400 == (*li)->_ReqInfo.m_iReqMaxLevel)	// 최대 제한 레벨이 400이면.
				itoa(iReqLevel, szText, 10);			// 최소 제한 레벨만 표시.
			else
				::sprintf(szText, "%d~%d", iReqLevel, (*li)->_ReqInfo.m_iReqMaxLevel);
#endif	// ASG_ADD_MOVEREQ_TEXT_MAX_LEVEL
#else	// _BLUE_SERVER
			itoa(iReqLevel, szText, 10);
#endif	// _BLUE_SERVER
			g_pRenderText->RenderText(m_ReqLevelPos.x, iY, szText, 0 ,0, RT3_WRITE_CENTER);
#ifdef PBG_ADD_PKSYSTEM_INGAMESHOP
			// 무법자1단계 상태 이상이고 무료섭이라면
			if(g_PKSystem->IsPKState())
			{
				itoa(g_PKSystem->GetReqZen((*li)->_ReqInfo.iReqZen), szText, 10);
			}
			else
#endif //PBG_ADD_PKSYSTEM_INGAMESHOP
				itoa((*li)->_ReqInfo.iReqZen, szText, 10);
			g_pRenderText->RenderText(m_ReqZenPos.x, iY, szText, 0 ,0, RT3_WRITE_CENTER);
			
			// 선택맵이름 배경색
			if( (*li)->_bSelected == true )
			{
#ifdef YDG_MOD_PROTECT_AUTO_V3
#ifdef YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V3
			if (!g_pProtectAuto->IsNewVersion())
				glColor4f ( 0.8f, 0.8f, 0.1f, 0.6f );
			else
#endif	// YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V3
				glColor4f ( 0.8f, 0.8f, 0.1f, m_fBackgroundAlpha );
#else	// YDG_MOD_PROTECT_AUTO_V3
				glColor4f ( 0.8f, 0.8f, 0.1f, 0.6f );
#endif	// YDG_MOD_PROTECT_AUTO_V3
				RenderColor( iX, iY-1, m_MapNameUISize.x-22, m_iRealFontHeight );
				glColor4f ( 1.f, 1.f, 1.f, 1.f );
				EnableAlphaTest();
			}
		}
		else
		{
#ifdef CSK_MOD_PROTECT_AUTO_V1
			g_pRenderText->SetTextColor(164, 39, 17, m_iTextAlpha);
#else // CSK_MOD_PROTECT_AUTO_V1
			g_pRenderText->SetTextColor(164, 39, 17, 255);
#endif // CSK_MOD_PROTECT_AUTO_V1
#ifdef ASG_ADD_GENS_SYSTEM
			if ((*li)->_bStrife)
				g_pRenderText->RenderText(m_StrifePos.x, iY, GlobalText[2987], 0 ,0, RT3_WRITE_CENTER);	// 2987	"(분쟁)"
#endif	// ASG_ADD_GENS_SYSTEM
			g_pRenderText->RenderText(m_MapNamePos.x, iY, (*li)->_ReqInfo.szMainMapName, 0 ,0, RT3_WRITE_CENTER);
#ifdef _BLUE_SERVER		//  블루서버에만 적용
#ifdef ASG_ADD_MOVEREQ_TEXT_MAX_LEVEL
			if (400 == (*li)->_ReqInfo.m_iReqMaxLevel)	// 최대 제한 레벨이 400이면.
				itoa(iReqLevel, szText, 10);			// 최소 제한 레벨만 표시.
			else
				::sprintf(szText, "%d~%d", iReqLevel, (*li)->_ReqInfo.m_iReqMaxLevel);
			
#ifdef CSK_MOD_PROTECT_AUTO_V1
			if (iReqLevel > iLevel || (*li)->_ReqInfo.m_iReqMaxLevel < iLevel)
				g_pRenderText->SetTextColor(255, 51, 26, m_iTextAlpha);
			else
				g_pRenderText->SetTextColor(164, 39, 17, m_iTextAlpha);
#else  //CSK_MOD_PROTECT_AUTO_V1
			if (iReqLevel > iLevel || (*li)->_ReqInfo.m_iReqMaxLevel < iLevel)
				g_pRenderText->SetTextColor(255, 51, 26, 255);
			else
				g_pRenderText->SetTextColor(164, 39, 17, 255);
#endif //CSK_MOD_PROTECT_AUTO_V1

#endif	// ASG_ADD_MOVEREQ_TEXT_MAX_LEVEL
#else	// _BLUE_SERVER
			itoa(iReqLevel, szText, 10);
			if( iReqLevel > iLevel )
			{
#ifdef CSK_MOD_PROTECT_AUTO_V1
				g_pRenderText->SetTextColor(255, 51, 26, m_iTextAlpha);
#else // CSK_MOD_PROTECT_AUTO_V1
				g_pRenderText->SetTextColor(255, 51, 26, 255);
#endif // CSK_MOD_PROTECT_AUTO_V1
			}
			else
			{
#ifdef CSK_MOD_PROTECT_AUTO_V1
				g_pRenderText->SetTextColor(164, 39, 17, m_iTextAlpha);
#else // CSK_MOD_PROTECT_AUTO_V1
				g_pRenderText->SetTextColor(164, 39, 17, 255);
#endif // CSK_MOD_PROTECT_AUTO_V1
			}
#endif	// _BLUE_SERVER
			g_pRenderText->RenderText(m_ReqLevelPos.x, iY, szText, 0 ,0, RT3_WRITE_CENTER);
#ifdef PBG_ADD_PKSYSTEM_INGAMESHOP
			// 무법자1단계 상태 이상이고 무료섭이라면
			if(g_PKSystem->IsPKState())
			{
				itoa(g_PKSystem->GetReqZen((*li)->_ReqInfo.iReqZen), szText, 10);
			}
			else
#endif //PBG_ADD_PKSYSTEM_INGAMESHOP
			itoa((*li)->_ReqInfo.iReqZen, szText, 10);
#ifdef _VS2008PORTING
			if( (*li)->_ReqInfo.iReqZen > (int)iZen )
#else // _VS2008PORTING
			if( (*li)->_ReqInfo.iReqZen > iZen )
#endif // _VS2008PORTING
			{
#ifdef CSK_MOD_PROTECT_AUTO_V1
				g_pRenderText->SetTextColor(255, 51, 26, m_iTextAlpha);
#else // CSK_MOD_PROTECT_AUTO_V1
				g_pRenderText->SetTextColor(255, 51, 26, 255);
#endif // CSK_MOD_PROTECT_AUTO_V1	
			}
			else
			{
#ifdef CSK_MOD_PROTECT_AUTO_V1
				g_pRenderText->SetTextColor(164, 39, 17, m_iTextAlpha);
#else // CSK_MOD_PROTECT_AUTO_V1
				g_pRenderText->SetTextColor(164, 39, 17, 255);
#endif // CSK_MOD_PROTECT_AUTO_V1	
			}
			g_pRenderText->RenderText(m_ReqZenPos.x, iY, szText, 0 ,0, RT3_WRITE_CENTER);
		}

		iCurRenderTextIndex++;
	}

	// 닫기 버튼
#ifdef YDG_MOD_PROTECT_AUTO_V3
#ifdef YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V3
	if (!g_pProtectAuto->IsNewVersion())
		g_pRenderText->SetTextColor(255, 255, 255, 255);
	else
#endif	// YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V3
		g_pRenderText->SetTextColor(255, 255, 255, m_fBackgroundAlpha * 255);
#else	// YDG_MOD_PROTECT_AUTO_V3
	g_pRenderText->SetTextColor(255, 255, 255, 255);
#endif	// YDG_MOD_PROTECT_AUTO_V3
#ifdef CSK_MOD_MOVE_COMMAND_WINDOW
	g_pRenderText->RenderText(m_MapNameUISize.x/2, m_Pos.y + m_MapNameUISize.y-m_iRealFontHeight-5, GlobalText[1002], 0 ,0, RT3_WRITE_CENTER);
#else // CSK_MOD_MOVE_COMMAND_WINDOW
	g_pRenderText->RenderText(m_MapNameUISize.x/2, m_MapNameUISize.y-m_iRealFontHeight-5, GlobalText[1002], 0 ,0, RT3_WRITE_CENTER);
#endif // CSK_MOD_MOVE_COMMAND_WINDOW
	DisableAlphaBlend();

	return true;
}

//---------------------------------------------------------------------------------------------
// GetLayerDepth
float SEASON3B::CNewUIMoveCommandWindow::GetLayerDepth()
{
	return 6.4f;
}

//---------------------------------------------------------------------------------------------
// OpenningProcess
void SEASON3B::CNewUIMoveCommandWindow::OpenningProcess()
{
#ifdef YDG_MOD_PROTECT_AUTO_V4
	CMoveCommandWindowEncrypt enc;
#endif	// YDG_MOD_PROTECT_AUTO_V4
#ifdef CSK_MOD_MOVE_COMMAND_WINDOW
	if(World != m_iWorldIndex)
	{
		m_iWorldIndex = World;
		ResetWindowOpenCount();
	}

	m_iPosY_Random = rand()%m_iPosY_Random_Range + m_iPosY_Random_Min;
	SetPos(m_Pos.x, m_Pos_Start.y + m_iPosY_Random);
	
	m_iWindowOpenCount++;

	if(m_iWindowOpenCount >= 10)
	{
		// 2731 "지속적인 맵 이동 시도 시 접속이 종료됩니다."
		SEASON3B::CreateOkMessageBox(GlobalText[2731]);
		ResetWindowOpenCount();
	}
#endif // CSK_MOD_MOVE_COMMAND_WINDOW
#ifdef PBG_FIX_MOVECOMMAND_WINDOW_SCROLL
	SetPos(m_Pos.x, m_Pos.y);
#endif //PBG_FIX_MOVECOMMAND_WINDOW_SCROLL
#ifdef ASG_ADD_GENS_SYSTEM
	SetStrifeMap();		// 분쟁 맵 설정.
#endif	// ASG_ADD_GENS_SYSTEM
	SettingCanMoveMap();

	m_iScrollBtnMouseEvent = MOVECOMMAND_MOUSEBTN_NORMAL;
	m_ScrollBtnPos.y = m_ScrollBtnStartPos.y;
	m_iRenderStartTextIndex = 0;

#ifdef CSK_MOD_PROTECT_AUTO_V1
	m_iRenderEndTextIndex = m_iRenderStartTextIndex + m_iTextLine;
#else // CSK_MOD_PROTECT_AUTO_V1
	m_iRenderEndTextIndex = m_iRenderStartTextIndex + MOVECOMMAND_MAX_RENDER_TEXTLINE;
#endif // CSK_MOD_PROTECT_AUTO_V1

#ifdef YDG_MOD_PROTECT_AUTO_V3
	m_iWheelCounter = 0;
#endif	// YDG_MOD_PROTECT_AUTO_V3
	
#ifdef _VS2008PORTING
	if( m_iRenderEndTextIndex > (int)m_listMoveInfoData.size() )
#else // _VS2008PORTING
	if( m_iRenderEndTextIndex > m_listMoveInfoData.size() )
#endif // _VS2008PORTING
	{
		m_iRenderEndTextIndex -= (m_iRenderEndTextIndex-m_listMoveInfoData.size());
	}
}

#ifdef CSK_MOD_MOVE_COMMAND_WINDOW
void SEASON3B::CNewUIMoveCommandWindow::ResetWindowOpenCount()
{
#ifdef YDG_MOD_PROTECT_AUTO_V4
	CMoveCommandWindowEncrypt enc;
#endif	// YDG_MOD_PROTECT_AUTO_V4
	m_iWindowOpenCount = 0;	
}

void SEASON3B::CNewUIMoveCommandWindow::SetCastleOwner(bool bOwner)
{
	m_bCastleOwner = bOwner;
}

bool SEASON3B::CNewUIMoveCommandWindow::IsCastleOwner()
{
	return m_bCastleOwner;
}
#endif // CSK_MOD_MOVE_COMMAND_WINDOW

//---------------------------------------------------------------------------------------------
// ClosingProcess
void SEASON3B::CNewUIMoveCommandWindow::ClosingProcess()
{

}

//---------------------------------------------------------------------------------------------
// LoadImages
void SEASON3B::CNewUIMoveCommandWindow::LoadImages()
{
	LoadBitmap("Interface\\newui_scrollbar_up.tga", IMAGE_MOVECOMMAND_SCROLL_TOP);
	LoadBitmap("Interface\\newui_scrollbar_m.tga", IMAGE_MOVECOMMAND_SCROLL_MIDDLE, GL_LINEAR);
	LoadBitmap("Interface\\newui_scrollbar_down.tga", IMAGE_MOVECOMMAND_SCROLL_BOTTOM);
	LoadBitmap("Interface\\newui_scroll_on.tga", IMAGE_MOVECOMMAND_SCROLLBAR_ON, GL_LINEAR);
	LoadBitmap("Interface\\newui_scroll_off.tga", IMAGE_MOVECOMMAND_SCROLLBAR_OFF, GL_LINEAR);
}

//---------------------------------------------------------------------------------------------
// UnloadImages
void CNewUIMoveCommandWindow::UnloadImages()
{
	DeleteBitmap(IMAGE_MOVECOMMAND_SCROLL_TOP);
	DeleteBitmap(IMAGE_MOVECOMMAND_SCROLL_MIDDLE);
	DeleteBitmap(IMAGE_MOVECOMMAND_SCROLL_BOTTOM);
	DeleteBitmap(IMAGE_MOVECOMMAND_SCROLLBAR_ON);
	DeleteBitmap(IMAGE_MOVECOMMAND_SCROLLBAR_OFF);
}

//////////////////////////////////////////////////////////////////////////

#ifdef CSK_MOD_PROTECT_AUTO_V1


CNewUIMoveCommandWindowNew::CNewUIMoveCommandWindowNew()
{
}

CNewUIMoveCommandWindowNew::~CNewUIMoveCommandWindowNew()
{

}

void CNewUIMoveCommandWindowNew::OpenningProcess()
{
#ifdef YDG_MOD_PROTECT_AUTO_V4
	CMoveCommandWindowEncrypt enc;
#endif	// YDG_MOD_PROTECT_AUTO_V4
	m_iTextAlpha = 150;
	m_fBackgroundAlpha = 0.4f;
	m_iTopSpace = rand()%20;

	m_iTextLine = SEASON3B::CNewUIMoveCommandWindow::MOVECOMMAND_MAX_RENDER_TEXTLINE + (rand()%8 - 4);
		
	CNewUIMoveCommandWindow::OpenningProcess();
}

#endif // CSK_MOD_PROTECT_AUTO_V1

#ifdef LJH_ADD_SAVEOPTION_WHILE_MOVING_FROM_OR_TO_DIFF_SERVER
// 로랜 협곡이나 로렌시장처럼 맵이 다른 서버에 존재 하는지 여부
BOOL CNewUIMoveCommandWindow::IsTheMapInDifferentServer(const int iFromMapIndex, const int iToMapIndex) const
{
	BOOL bInOtherServer = FALSE;
	
	// 출발 지역 (ENUM_WORLD index)
	switch(iFromMapIndex) 
	{
	case WD_30BATTLECASTLE:
	case WD_79UNITEDMARKETPLACE:
		bInOtherServer = TRUE;
		break;
	default:
		break;
	}
	
	// 도착지역 (movereq index)
	switch(iToMapIndex)
	{
	case 24:	// 로렌협곡
	case 44:	// 로렌시장
		bInOtherServer = TRUE;
		break;
	default:
		break;
	}
	
	return bInOtherServer;
}
#endif //LJH_ADD_SAVEOPTION_WHILE_MOVING_FROM_OR_TO_DIFF_SERVER

#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
// movereq에서 맵의 이름(자국어 또는 영어)에 맞는 index를 반환한다.
// 만약, NULL값이 넘어오거나 맵의 이름을 찾지 못했을 경우 -1을 반환.
int CNewUIMoveCommandWindow::GetMapIndexFromMovereq(const char *pszMapName)
{
	if (pszMapName == NULL)
		return -1;

	int iMapIndex = -1;
	std::list<CMoveCommandData::MOVEINFODATA*>::iterator li;
	for (li = m_listMoveInfoData.begin(); li != m_listMoveInfoData.end(); li++)
	{
		if (stricmp((*li)->_ReqInfo.szMainMapName, pszMapName) == 0 
			|| stricmp((*li)->_ReqInfo.szSubMapName, pszMapName) == 0)
		{
			iMapIndex = (*li)->_ReqInfo.index;
			break;
		}
	}
	
	return iMapIndex;
}
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
#endif	// YDG_ADD_ENC_MOVE_COMMAND_WINDOW
