//////////////////////////////////////////////////////////////////////////
//  
//  UIManager.cpp
//  
//  내  용 : UI 들을 관리 해주는 클래스
//  
//  날  짜 : 2004년 11월 09일
//  
//  작성자 : 강 병 국
//  
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//						 												//
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "ZzzInfomation.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "ZzzLodTerrain.h"
#include "ZzzInventory.h"
#include "wsclientinline.h"
#include "CSQuest.h"
#include "npcBreeder.h"
#include "CSItemOption.h"
#include "CSChaosCastle.h"
#include "GIPetManager.h"
#include "PersonalShopTitleImp.h"
#include "npcGateSwitch.h"
#include "CComGem.h"
#include "GM_Kanturu_2nd.h"
#ifndef KJH_DEL_PC_ROOM_SYSTEM		// #ifndef
#ifdef ADD_PCROOM_POINT_SYSTEM
#include "PCRoomPoint.h"
#endif	// ADD_PCROOM_POINT_SYSTEM
#endif // KJH_DEL_PC_ROOM_SYSTEM
#include "NewUICustomMessageBox.h"
#include "NewUINPCShop.h"
#include "MixMgr.h"

//////////////////////////////////////////////////////////////////////////
//						 												//
//////////////////////////////////////////////////////////////////////////

extern int g_iKeyPadEnable;
extern int g_iChatInputType;
extern DWORD g_dwMouseUseUIID;

CUIGateKeeper*				g_pUIGateKeeper = NULL;
JewelHarmonyInfo*			g_pUIJewelHarmonyinfo = NULL;
ItemAddOptioninfo*			g_pItemAddOptioninfo = NULL;
CUIPopup*					g_pUIPopup = NULL;

extern float g_fScreenRate_x;
extern float g_fScreenRate_y;

extern int   ShopInventoryStartX;
extern int   ShopInventoryStartY;

extern bool g_bTradeEndByOppenent;

bool HeroInventoryEnable       = false; //  캐릭터 장비창.
bool GuildListEnable           = false; //  길드 리스트.
bool EnableGuildInterface      = false; //  
bool StorageInventoryEnable    = false; //  창고.
bool MixInventoryEnable        = false; //  조합기.
bool g_bPersonalShopWnd		   = false;
bool g_bServerDivisionEnable   = false; //  서버분할창.

//////////////////////////////////////////////////////////////////////////
//						 												//
//////////////////////////////////////////////////////////////////////////

CUIManager::CUIManager()
{
	g_pUIGateKeeper				= new CUIGateKeeper;
	g_pUIPopup					= new CUIPopup;
	g_pUIJewelHarmonyinfo       = JewelHarmonyInfo::MakeInfo();
	g_pItemAddOptioninfo		= ItemAddOptioninfo::MakeInfo();

	Init();
}

CUIManager::~CUIManager()
{
	SAFE_DELETE( g_pItemAddOptioninfo );
	SAFE_DELETE( g_pUIJewelHarmonyinfo );
	SAFE_DELETE( g_pUIGateKeeper );
	SAFE_DELETE( g_pUIPopup );
}

void CUIManager::Init()
{
	g_pUIPopup->Init();
#ifdef PET_SYSTEM
    giPetManager::InitPetManager ();
#endif// PET_SYSTEM
	ClearPersonalShop();
}

POINT CUIManager::RenderWindowBase( int nHeight, int nOriginY )
{
	int nWidth = 213;

    EnableAlphaBlend3();
	glColor4f( 1.0f, 1.0f, 1.0f, 0.8f );

	float fPosX = 320 - nWidth/2;
	float fPosY;
	if( nOriginY == -1 )
	{
		fPosY = 130 - nHeight/2;
		if( fPosY < 40.0f )		fPosY = 40.0f;
	}
	else
		fPosY = nOriginY;
	POINT ptPos = { fPosX, fPosY };

	// 위쪽외곽 그리기
	RenderBitmap( BITMAP_INTERFACE+22, fPosX, fPosY, nWidth, 5, 0.f, 0.f, nWidth/256.f, 5.f/8.f );
	fPosY += 5;

	// 가운데부분 40 단위로 그리고 (비트맵크기때문)
	int nBodyHeight = nHeight - 10;		// 가로높이에서 위아래 외곽을 뺀 크기
	int nPatternCount = nBodyHeight / 40;
	for( int i=0; i<nPatternCount; ++i )
	{
		RenderBitmap( BITMAP_INTERFACE+21, fPosX, fPosY, nWidth, 40, 0.f, 0.f, nWidth/256.f, 40.f/64.f );
		fPosY += 40;
	}
	// 남은 부분 그리기 (나머지는 비트맵을 잘라서 그린다)
	if( nBodyHeight > nPatternCount*40 )
	{
		float fRate = (float)( nBodyHeight-nPatternCount*40 ) / 40.0f;
		RenderBitmap( BITMAP_INTERFACE+21, fPosX, fPosY, nWidth, 40*fRate, 0.f, 0.f, nWidth/256.f, (40.f/64.f)*fRate );
		fPosY += 40*fRate;
	}

	// 아래쪽외곽 그리기
	RenderBitmap( BITMAP_INTERFACE+22, fPosX, fPosY, nWidth, 5, 0.f, 0.f, nWidth/256.f, 5.f/8.f );

	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
	DisableAlphaBlend();

	return ptPos;
}

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // #ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
bool CUIManager::PressKey( int nKey )
{
	return false;
}
#endif // #ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING

bool CUIManager::IsInputEnable()
{
	// 개선된 입력창 미사용시 이것먼저 체크해서 결과 반환
	if( InputEnable || GuildInputEnable || (g_pUIPopup->GetPopupID() != 0 && g_pUIPopup->IsInputEnable()) )
		return true;
	if( GetFocus() == g_hWnd )
		return false;
	return true;
}

void CUIManager::UpdateInput()
{
}

void CUIManager::Render()
{

}

void CUIManager::CloseAll()
{
	// 모든 창 닫기
	for( DWORD dwInterface=INTERFACE_FRIEND ; dwInterface<INTERFACE_MAX_COUNT ; ++dwInterface )
	{
		if( g_pUIManager->IsOpen( dwInterface ) )
		{
			Close( dwInterface );
		}
	}

	g_pUIPopup->CancelPopup();
}

bool CUIManager::CloseInterface( list<DWORD>& dwInterfaceFlag, DWORD dwExtraData )
{
	return true;
}

bool CUIManager::IsOpen( DWORD dwInterface )
{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	bool bOpen = false;

	if( dwInterface == 0 )
	{
		for( DWORD dwInterface=INTERFACE_FRIEND ; dwInterface<INTERFACE_MAX_COUNT ; ++dwInterface )
		{
			if( IsOpen( dwInterface ) )
				bOpen = true;
		}
	}

	switch( dwInterface )
	{
	case INTERFACE_INVENTORY:				bOpen = HeroInventoryEnable;		break;
	case INTERFACE_STORAGE:					bOpen = StorageInventoryEnable;		break;
	case INTERFACE_PERSONALSHOPSALE:	
	case INTERFACE_PERSONALSHOPPURCHASE:	bOpen = g_bPersonalShopWnd;			break;
	case INTERFACE_SERVERDIVISION:			bOpen = g_bServerDivisionEnable;	break;
	default:								bOpen = false;						break;
	}
	return bOpen;
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	if( dwInterface == 0 )
	{
		for( DWORD dwInterface=INTERFACE_FRIEND ; dwInterface<INTERFACE_MAX_COUNT ; ++dwInterface )
		{
			if( IsOpen( dwInterface ) )
				return true;
		}
	}

	switch( dwInterface )
	{
	case INTERFACE_INVENTORY:
		return HeroInventoryEnable;
	case INTERFACE_STORAGE:
		return StorageInventoryEnable;
	case INTERFACE_PERSONALSHOPSALE:
	case INTERFACE_PERSONALSHOPPURCHASE:
		return g_bPersonalShopWnd;
	case INTERFACE_SERVERDIVISION:
		return g_bServerDivisionEnable;
	default:
		return false;
	}
	return false;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
}

bool CUIManager::IsCanOpen( DWORD dwInterfaceFlag )
{
	return true;
}

void CUIManager::GetInterfaceAll( list<DWORD>& outflag )
{
	for ( DWORD flag = INTERFACE_FRIEND; flag < INTERFACE_MAX_COUNT; ++flag )
	{
		outflag.push_back( flag );
	}
}

void CUIManager::GetInsertInterface( list<DWORD>& outflag, DWORD insertflag )
{
	outflag.push_back( insertflag );
}

void CUIManager::GetDeleteInterface( list<DWORD>& outflag, DWORD deleteflag )
{
	for ( list<DWORD>::iterator iter = outflag.begin(); iter != outflag.end(); )
	{
		list<DWORD>::iterator Tempiter = iter;
		++iter;
		DWORD Tempflag = *Tempiter;

		if( Tempflag == deleteflag )
		{
			outflag.erase( Tempiter );
		}
	}
}

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
bool CUIManager::Open( DWORD dwInterface )
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
bool CUIManager::Open( DWORD dwInterface, DWORD dwExtraData )
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
{
	if( IsOpen(INTERFACE_REFINERYINFO) )
		return false;
	if( IsOpen( dwInterface ) )
		return false;
	if( IsCanOpen( dwInterface ) == false)
		return false;
	if( LogOut == true) 
		return false;
	
	list<DWORD> closeinterfaceflag;
	// 자기자신은 제외한 모든 인터페이스
	GetInterfaceAll( closeinterfaceflag );
	GetDeleteInterface( closeinterfaceflag, dwInterface );
	// 같이 열려도 상관없는걸 뺀다.
	GetDeleteInterface( closeinterfaceflag, INTERFACE_FRIEND );

	switch( dwInterface )
	{
	case INTERFACE_INVENTORY:							//## 사용하지않는 UI
		{
			bool bResult = CloseInterface( closeinterfaceflag );
			if( bResult )
			{
				HeroInventoryEnable = true;
#ifndef KJH_FIX_DARKLOAD_PET_SYSTEM												//## 소스정리 대상임.
				//인벤토리를 열 때마다 펫 정보 리셋을 요구한다.
				if(GetBaseClass(Hero->Class)==CLASS_DARK_LORD)
				{
					SendRequestPetInfo ( 0, 0, EQUIPMENT_WEAPON_LEFT );
					SendRequestPetInfo ( 1, 0, EQUIPMENT_HELPER );
				}
#endif // KJH_FIX_DARKLOAD_PET_SYSTEM											//## 소스정리 대상임.
			}
		}
		break;
	case INTERFACE_PERSONALSHOPSALE:
		{
			// 같이 열려도 상관없는걸 뺀다.
			GetDeleteInterface( closeinterfaceflag, INTERFACE_INVENTORY );

			bool bResult = CloseInterface( closeinterfaceflag );
			if( bResult )
			{
				Open( INTERFACE_INVENTORY );

				if(g_iPShopWndType != PSHOPWNDTYPE_NONE) {
					g_ErrorReport.Write("@ OpenPersonalShop : SendRequestInventory\n");
					SendRequestInventory();		//. 혹시 꼬였을지도 모르니 인벤을 새로 받아온다
				}

				//. 인스턴스가 없을때만 새로 생성한다
				//. 인스턴스가 이미 있다면 return false;
				CreatePersonalItemTable();

				g_bPersonalShopWnd = true;
				g_iPShopWndType = PSHOPWNDTYPE_SALE;
			}
		}
		break;
	case INTERFACE_PERSONALSHOPPURCHASE:
		{
			bool bResult = CloseInterface( closeinterfaceflag );
			if( bResult )
			{
				Open( INTERFACE_INVENTORY );

				if(g_iPShopWndType != PSHOPWNDTYPE_NONE) {
					g_ErrorReport.Write("@ OpenPersonalShop : SendRequestInventory\n");
					SendRequestInventory();		//. 혹시 꼬였을지도 모르니 인벤을 새로 받아온다
				}

				//. 인스턴스가 없을때만 새로 생성한다
				//. 인스턴스가 이미 있다면 return false;
				CreatePersonalItemTable();

				g_bPersonalShopWnd = true;
				g_iPShopWndType = PSHOPWNDTYPE_PURCHASE;
			}
		}
		break;
	case INTERFACE_SERVERDIVISION:
		{
			bool bResult = CloseInterface( closeinterfaceflag );
			if( bResult )
			{
				g_bServerDivisionEnable = true;
				g_bServerDivisionAccept = false;
			}
		}
		break;
	default:
		return false;
	}

	PlayBuffer(SOUND_CLICK01);
	PlayBuffer(SOUND_INTERFACE01);

	return true;
}

bool CUIManager::Close( DWORD dwInterface, DWORD dwExtraData )
{
	if( !IsOpen( dwInterface ) )		return false;

	// 같이 닫혀야 할것들을 설정한다.
	switch( dwInterface )
	{
	case INTERFACE_INVENTORY:
		{
			bool bResult = true;
			if( bResult )
			{
				list<DWORD> closeinterfaceflag;

				GetInsertInterface( closeinterfaceflag, INTERFACE_TRADE );
				GetInsertInterface( closeinterfaceflag, INTERFACE_STORAGE );
				GetInsertInterface( closeinterfaceflag, INTERFACE_GUILDSTORAGE );
				GetInsertInterface( closeinterfaceflag, INTERFACE_MIXINVENTORY );
				GetInsertInterface( closeinterfaceflag, INTERFACE_PERSONALSHOPSALE );
				GetInsertInterface( closeinterfaceflag, INTERFACE_NPCBREEDER );
				GetInsertInterface( closeinterfaceflag, INTERFACE_NPCSHOP );
				GetInsertInterface( closeinterfaceflag, INTERFACE_SENATUS );
				GetInsertInterface( closeinterfaceflag, INTERFACE_REFINERY );	
				bResult = CloseInterface( closeinterfaceflag, dwExtraData );
				if( bResult )
				{
					list<DWORD> closeflag;
					GetInsertInterface( closeflag, dwInterface );
					CloseInterface( closeflag, dwExtraData );
				}
			}
		}
		break;
	case INTERFACE_STORAGE:
	case INTERFACE_GUILDSTORAGE:
	case INTERFACE_MIXINVENTORY:
	case INTERFACE_TRADE:
	case INTERFACE_NPCBREEDER:
	case INTERFACE_NPCSHOP:
	case INTERFACE_GUARDSMAN:
		{
			list<DWORD> closeinterfaceflag;
			GetInsertInterface( closeinterfaceflag, dwInterface );
			bool bResult = CloseInterface( closeinterfaceflag, dwExtraData );
			if( bResult )
			{
				list<DWORD> closeflag;
				GetInsertInterface( closeflag, INTERFACE_INVENTORY );
				CloseInterface( closeflag, dwExtraData );
			}
		}
		break;
	default:
		{
			list<DWORD> closeinterfaceflag;
			GetInsertInterface( closeinterfaceflag, dwInterface );	
			CloseInterface( closeinterfaceflag, dwExtraData );
		}
		break;
	}

	PlayBuffer(SOUND_CLICK01);
	PlayBuffer(SOUND_INTERFACE01);

	return true;
}
