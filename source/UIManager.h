//////////////////////////////////////////////////////////////////////////
//  
//  UIManager.h
//  
//  내  용 : UI 들을 관리 해주는 클래스
//  
//  날  짜 : 2004년 11월 09일
//  
//  작성자 : 강 병 국
//  
//////////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIMANAGER_H__2A2AD8F0_E731_4B40_AA6E_E86226026AF9__INCLUDED_)
#define AFX_UIMANAGER_H__2A2AD8F0_E731_4B40_AA6E_E86226026AF9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "UIGuildInfo.h"
#include "UIGuildMaster.h"
#include "UISenatus.h"
#include "UIGateKeeper.h"
#include "UIPopup.h"
#include "UIJewelHarmony.h"
#include "ItemAddOptioninfo.h"
#include "GM_Kanturu_2nd.h"

//////////////////////////////////////////////////////////////////////////
//  EXTERN.
//////////////////////////////////////////////////////////////////////////
extern CUIGateKeeper*		g_pUIGateKeeper;
extern CUIPopup*			g_pUIPopup;
extern JewelHarmonyInfo*    g_pUIJewelHarmonyinfo;
extern ItemAddOptioninfo*   g_pItemAddOptioninfo;
class CUIManager;
extern CUIManager*			g_pUIManager;

enum
{
	INTERFACE_NONE = 0,
	INTERFACE_FRIEND,				
	INTERFACE_MOVEMAP,				
	INTERFACE_PARTY,					
	INTERFACE_QUEST,					
	INTERFACE_GUILDINFO,				
	INTERFACE_TRADE,					
	INTERFACE_STORAGE,				
	INTERFACE_GUILDSTORAGE,			
	INTERFACE_MIXINVENTORY,			
	INTERFACE_COMMAND,				
	INTERFACE_PET,					
	INTERFACE_PERSONALSHOPSALE,		
	INTERFACE_DEVILSQUARE,			
	INTERFACE_SERVERDIVISION,		
	INTERFACE_BLOODCASTLE,			
	INTERFACE_NPCBREEDER,			
	INTERFACE_NPCSHOP,				
	INTERFACE_PERSONALSHOPPURCHASE,	
	INTERFACE_NPCGUILDMASTER,		
	INTERFACE_GUARDSMAN,				
	INTERFACE_SENATUS,				
	INTERFACE_GATEKEEPER,			
	INTERFACE_CATAPULTATTACK,		
	INTERFACE_CATAPULTDEFENSE,		
	INTERFACE_GATESWITCH,					// 성문 개폐 스위치
	INTERFACE_CHARACTER,					// 연관된것 -> INTERFACE_QUEST, INTERFACE_PET
	INTERFACE_INVENTORY,					// 연관된것 -> INTERFACE_CHARACTER, INTERFACE_TRADE, INTERFACE_STORAGE, INTERFACE_GUILDSTORAGE, INTERFACE_MIXINVENTORY, INTERFACE_GEMINVENTORY ,INTERFACE_PERSONALSHOPSALE, INTERFACE_NPCBREEDER
	INTERFACE_REFINERY,						// 연관된것 -> INTERFACE_CHARACTER, INTERFACE_TRADE, INTERFACE_STORAGE, INTERFACE_GUILDSTORAGE, INTERFACE_MIXINVENTORY, INTERFACE_GEMINVENTORY ,INTERFACE_PERSONALSHOPSALE, INTERFACE_NPCBREEDER
	INTERFACE_REFINERYINFO,	
	INTERFACE_KANTURU2ND_ENTERNPC,
	INTERFACE_MAP_ENTRANCE,
	INTERFACE_MAX_COUNT,
};

class CUIManager  
{
public:
	CUIManager();
	virtual ~CUIManager();

protected:
	bool IsCanOpen( DWORD dwInterfaceFlag );		// 열 수 있는 상태인지 판단
	// 설정된 플래그 의 창 모두 닫기
	// 리턴값 : true  : 플래그에 해당되는 모든 창이 닫혔다 (닫혀있던 창은 무시)
	//			false : 어떤 창이 닫히지 못하는 상황일때..
	bool CloseInterface( list<DWORD>& dwInterfaceFlag, DWORD dwExtraData=0 );
public:
	// 초기화
	void Init();
	// 윈도우배경을 그린다 (리턴값은 640x480 기준의 윈도우기준점)
	POINT RenderWindowBase( int nHeight, int nOriginY=-1 );
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // #ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	// 내부에서 처리하면 TRUE 리턴
	bool PressKey( int nKey );
#endif // #ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	// 키보드 입력중이면 TRUE 리턴 (채팅등)
	bool IsInputEnable();
	// 입력처리
	void UpdateInput();
	// 렌더링
	void Render();
	// 모든 유아이창 닫기
	void CloseAll();
	bool IsOpen( DWORD dwInterface );						// 열려 있는지 판단 (0이면 아무거나 열려있는지 판단)
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	bool Open( DWORD dwInterface);							// 조건을 체크하여 연다.
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	bool Open( DWORD dwInterface, DWORD dwExtraData=0 );	// 조건을 체크하여 연다.
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	bool Close( DWORD dwInterface, DWORD dwExtraData=0 );	// 조건을 체크하여 닫는다.
	void GetInterfaceAll( list<DWORD>& outflag );
	void GetInsertInterface( list<DWORD>& outflag, DWORD insertflag );
	void GetDeleteInterface( list<DWORD>& outflag, DWORD deleteflag );
};


#endif // !defined(AFX_UIMANAGER_H__2A2AD8F0_E731_4B40_AA6E_E86226026AF9__INCLUDED_)
