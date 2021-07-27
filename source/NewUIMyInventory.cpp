// NewUIMyInventory.cpp: implementation of the CNewUIMyInventory class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIMyInventory.h"
#include "NewUISystem.h"
#include "NewUICustomMessageBox.h"
#include "CSChaosCastle.h"
#include "GOBoid.h"
#include "ZzzEffect.h"
#include "GIPetManager.h"
#include "CSParts.h"
#include "UIJewelHarmony.h"
#include "CDirection.h"
#include "GMCryWolf1st.h"
#include "GMCryingWolf2nd.h"
#include "ZzzInventory.h"
#include "wsclientinline.h"
#include "PCRoomPoint.h"
#include "MixMgr.h"
#include "ZzzLodTerrain.h"
#include "CSQuest.h"
#include "UIGuildInfo.h"
#include "UIManager.h"
#include "CSItemOption.h"
#include "GMHellas.h"
#ifdef LDK_ADD_NEW_PETPROCESS
#include "w_PetProcess.h"
#endif //LDK_ADD_NEW_PETPROCESS
#ifdef SOCKET_SYSTEM
#include "SocketSystem.h"
#endif	// SOCKET_SYSTEM
#ifdef CSK_FIX_WOPS_K27964_LOSTMAP_POP
#include "w_CursedTemple.h"
#endif // CSK_FIX_WOPS_K27964_LOSTMAP_POP
#ifdef YDG_ADD_CS5_PORTAL_CHARM
#include "PortalMgr.h"
#endif	// YDG_ADD_CS5_PORTAL_CHARM
#ifdef CSK_FIX_BLUELUCKYBAG_MOVECOMMAND
#include "Event.h"
#endif // CSK_FIX_BLUELUCKYBAG_MOVECOMMAND
#ifdef LDS_ADD_MAP_UNITEDMARKETPLACE
#include "GMUnitedMarketPlace.h"
#endif // LDS_ADD_MAP_UNITEDMARKETPLACE
#ifdef YDG_MOD_CHANGE_RING_EQUIPMENT_LIMIT
#include "ChangeRingManager.h"
#endif	// YDG_MOD_CHANGE_RING_EQUIPMENT_LIMIT
#ifdef PBG_ADD_NEWCHAR_MONK
#include "MonkSystem.h"
#endif //PBG_ADD_NEWCHAR_MONK
#ifdef LDK_ADD_SCALEFORM
#include "CGFxProcess.h"
#endif //LDK_ADD_SCALEFORM

using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SEASON3B::CNewUIMyInventory::CNewUIMyInventory()
{ 
	m_pNewUIMng = NULL;
	m_pNewUI3DRenderMng = NULL;
	m_pNewInventoryCtrl = NULL;
	m_Pos.x = m_Pos.y = 0;

	memset(&m_EquipmentSlots, 0, sizeof(EQUIPMENT_ITEM)*MAX_EQUIPMENT_INDEX);
	m_iPointedSlot = -1;

	m_MyShopMode = MYSHOP_MODE_OPEN;
	m_RepairMode = REPAIR_MODE_OFF;
	m_dwStandbyItemKey = 0;

	m_bRepairEnableLevel = false;
	m_bMyShopOpen = false;
}

SEASON3B::CNewUIMyInventory::~CNewUIMyInventory() 
{ 
	Release(); 
}

bool SEASON3B::CNewUIMyInventory::Create(CNewUIManager* pNewUIMng, CNewUI3DRenderMng* pNewUI3DRenderMng, int x, int y)
{
	if(NULL == pNewUIMng || NULL == pNewUI3DRenderMng || NULL == g_pNewItemMng)
		return false;

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_INVENTORY, this);

	m_pNewUI3DRenderMng = pNewUI3DRenderMng;
	m_pNewUI3DRenderMng->Add3DRenderObj(this, INVENTORY_CAMERA_Z_ORDER);

	m_pNewInventoryCtrl = new CNewUIInventoryCtrl;
	if(false == m_pNewInventoryCtrl->Create(m_pNewUI3DRenderMng, g_pNewItemMng, this, x+15, y+200, 8, 8))
	{
		SAFE_DELETE(m_pNewInventoryCtrl);
		return false;
	}

	SetPos(x, y);

	LoadImages();

	//. 장비창 슬롯들 영역 셋팅
	SetEquipmentSlotInfo();
	SetButtonInfo();

	Show(false);

	return true;
}

void SEASON3B::CNewUIMyInventory::Release()
{
	if(m_pNewUI3DRenderMng)
		m_pNewUI3DRenderMng->DeleteUI2DEffectObject(UI2DEffectCallback);

	UnequipAllItems();
	DeleteAllItems();

	UnloadImages();

	SAFE_DELETE(m_pNewInventoryCtrl);

	if(m_pNewUI3DRenderMng)
	{
		m_pNewUI3DRenderMng->Remove3DRenderObj(this);
		m_pNewUI3DRenderMng = NULL;
	}
	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}
}

bool SEASON3B::CNewUIMyInventory::EquipItem(int iIndex, BYTE* pbyItemPacket)
{
	if(iIndex >= 0 && iIndex < MAX_EQUIPMENT_INDEX && g_pNewItemMng && CharacterMachine)
	{
		ITEM* pTargetItemSlot = &CharacterMachine->Equipment[iIndex];
		if(pTargetItemSlot->Type > 0)	//. 이미 차고있다면
		{
			UnequipItem(iIndex);	//. 장비 해제
		}

		//. 임시 아이템 생성
		ITEM* pTempItem = g_pNewItemMng->CreateItem(pbyItemPacket);		
		//. 장착 할 수 있는가??
		if(NULL == pTempItem)	
		//if(NULL == pTempItem || false == IsEquipable(iIndex, pTempItem))	
		{
			return false;
		}

#ifdef KJH_FIX_DARKLOAD_PET_SYSTEM
		// 장착 아이템이 다크호스나 다크스피릿이면, 펫 정보를 서버에 요청
		// 다크호스
		if( pTempItem->Type == ITEM_HELPER+4 )
			SendRequestPetInfo( PET_TYPE_DARK_HORSE, 0, iIndex );
		// 다크스피릿
		if( pTempItem->Type == ITEM_HELPER+5 )
		{
			giPetManager::CreatePetDarkSpirit( Hero );
			SendRequestPetInfo( PET_TYPE_DARK_SPIRIT, 0, iIndex );
		}
#endif // KJH_FIX_DARKLOAD_PET_SYSTEM
		
		pTempItem->lineal_pos = iIndex;
		pTempItem->ex_src_type = ITEM_EX_SRC_EQUIPMENT;
		memcpy(pTargetItemSlot, pTempItem, sizeof(ITEM));		//. 캐릭터 버퍼로 복사
		g_pNewItemMng->DeleteItem(pTempItem);	//. 임시 아이템 제거 (메니져 관리 밖이므로)

		//. 장착효과 생성
		CreateEquippingEffect(pTargetItemSlot);
	}
	return false;
}
void SEASON3B::CNewUIMyInventory::UnequipItem(int iIndex)
{
	if(iIndex >= 0 && iIndex < MAX_EQUIPMENT_INDEX && g_pNewItemMng && CharacterMachine)
	{
		ITEM* pEquippedItem = &CharacterMachine->Equipment[iIndex];

		//. 장착 효과 제거
		if(pEquippedItem && pEquippedItem->Type != -1)
		{
#ifdef PBG_FIX_SKILLHOTKEY
			ResetSkillofItem(pEquippedItem);
#endif //PBG_FIX_SKILLHOTKEY

#ifdef PBG_FIX_DARKPET_RENDER
			// 다크호스
			if( pEquippedItem->Type == ITEM_HELPER+4 )
			{
				Hero->InitPetInfo(PET_TYPE_DARK_HORSE);
			}
			// 다크스피릿
			else if( pEquippedItem->Type == ITEM_HELPER+5 )
			{
				giPetManager::DeletePet(Hero);
				Hero->InitPetInfo(PET_TYPE_DARK_SPIRIT);
			}
#endif //PBG_FIX_DARKPET_RENDER

#ifdef PBG_FIX_DARKPET_RENDER
			if(pEquippedItem->Type != ITEM_HELPER+5)
#endif //PBG_FIX_DARKPET_RENDER
			DeleteEquippingEffectBug(pEquippedItem);

#ifdef LJW_FIX_MANY_FLAG_DISAPPEARED_PROBREM
			if( pEquippedItem->Type == ITEM_HELPER+20 && pEquippedItem->Level>>3 == 3)
				Hero->EtcPart = PARTS_NONE;
#endif //LJW_FIX_MANY_FLAG_DISAPPEARED_PROBREM

			pEquippedItem->Type = -1;
			pEquippedItem->Level = 0;
			pEquippedItem->Number = -1;
			pEquippedItem->Option1 = 0;
			pEquippedItem->ExtOption = 0;
#ifdef SOCKET_SYSTEM
			pEquippedItem->SocketCount = 0;
			for(int i = 0; i < MAX_SOCKETS; ++i)
			{
				pEquippedItem->SocketSeedID[i] = SOCKET_EMPTY;
				pEquippedItem->SocketSphereLv[i] = 0;
			}
			pEquippedItem->SocketSeedSetOption = 0;
#endif	// SOCKET_SYSTEM

#ifndef PBG_FIX_DARKPET_RENDER
#ifdef KJH_FIX_DARKLOAD_PET_SYSTEM
			// 다크호스
			if( pEquippedItem->Type == ITEM_HELPER+4 )
			{
				Hero->InitPetInfo(PET_TYPE_DARK_HORSE);
			}
			// 다크스피릿
			else if( pEquippedItem->Type == ITEM_HELPER+5 )
			{
				giPetManager::DeletePet(Hero);
				Hero->InitPetInfo(PET_TYPE_DARK_SPIRIT);
			}
#endif // KJH_FIX_DARKLOAD_PET_SYSTEM
#endif //PBG_FIX_DARKPET_RENDER
			DeleteEquippingEffect();
		}
	}
}
void SEASON3B::CNewUIMyInventory::UnequipAllItems()
{
	if(CharacterMachine)
	{
		for(int i=0; i<MAX_EQUIPMENT_INDEX; i++)
		{
#ifdef KJH_FIX_DARKLOAD_PET_SYSTEM
			UnequipItem( i );
#else // KJH_FIX_DARKLOAD_PET_SYSTEM											//## 소스정리 대상임.
			ITEM* pEquippedItem = &CharacterMachine->Equipment[i];
			//. 장착 효과 제거		
			if(pEquippedItem && pEquippedItem->Type != -1)
			{
				DeleteEquippingEffectBug(pEquippedItem);

				pEquippedItem->Type = -1;
				pEquippedItem->Number = -1;
				pEquippedItem->Option1 = 0;
				pEquippedItem->ExtOption = 0;

				DeleteEquippingEffect();
			}
#endif // KJH_FIX_DARKLOAD_PET_SYSTEM											//## 소스정리 대상임.
		}
	}
}

bool SEASON3B::CNewUIMyInventory::IsEquipable(int iIndex, ITEM* pItem)
{
	if(pItem == NULL)
		return false;
	
	//////////////////////////////////////////////////////////////////////////
	//. 장착 가능한 클래스 체크
	ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
	bool bEquipable = false;
	if( pItemAttr->RequireClass[GetBaseClass(Hero->Class)] )
		bEquipable = true; //. 착용 可
	/* 예외 */
	else if (GetBaseClass(Hero->Class) == CLASS_DARK && pItemAttr->RequireClass[CLASS_WIZARD] 
		&& pItemAttr->RequireClass[CLASS_KNIGHT])
		bEquipable = true; //. 마검일 경우 법사, 기사것을 착용 可

	// 전직단계를 검사해서 
	BYTE byFirstClass = GetBaseClass(Hero->Class);
	BYTE byStepClass = GetStepClass(Hero->Class);
	if( pItemAttr->RequireClass[byFirstClass] > byStepClass)
	{
		return false;
	}

	if(bEquipable == false)
		return false;

	//////////////////////////////////////////////////////////////////////////
	//. 장착 가능한 슬롯 체크
	bEquipable = false;
	if(pItemAttr->m_byItemSlot == iIndex)
		bEquipable = true; //. 착용 可
	/* 예외 */
	else if(pItemAttr->m_byItemSlot == EQUIPMENT_WEAPON_RIGHT && iIndex == EQUIPMENT_WEAPON_LEFT)
	{
		if (GetBaseClass(Hero->Class) == CLASS_KNIGHT || GetBaseClass(Hero->Class) == CLASS_DARK
#ifdef PBG_ADD_NEWCHAR_MONK
			|| GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER
#endif //PBG_ADD_NEWCHAR_MONK
			)	// 기사 마검은 그냥 손 위치 무시하고 찰수있다.
		{
			if (!pItemAttr->TwoHand)	// 단, 양손 무기는 오른손에만 장비 가능
				bEquipable = true;
#ifdef PBG_FIX_EQUIP_TWOHANDSWORD
			else
			{
				bEquipable = false;
				return false;
			}
#endif //PBG_FIX_EQUIP_TWOHANDSWORD
		}
		else if (GetBaseClass(Hero->Class) == CLASS_SUMMONER &&
			!(pItem->Type >= ITEM_STAFF && pItem->Type <= ITEM_STAFF+MAX_ITEM_INDEX))	// 소환술사는 스틱과 서외에는 그냥 손 위치 무시하고 찰수있다.
			bEquipable = true;
	}
	else if(pItemAttr->m_byItemSlot == EQUIPMENT_RING_RIGHT && iIndex == EQUIPMENT_RING_LEFT)
		bEquipable = true;

	if (GetBaseClass(Hero->Class) == CLASS_ELF)
	{
		//ITEM *r = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT];
		ITEM *l = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT ];
		if (iIndex == EQUIPMENT_WEAPON_RIGHT && l->Type != ITEM_BOW+7
			&& (l->Type >= ITEM_BOW && l->Type < ITEM_BOW+MAX_ITEM_INDEX))	// 왼손에 활(석궁용화살 제외)이면
		{
			if (pItem->Type != ITEM_BOW+15)	// 오른손에 화살만 장비 가능
				bEquipable = false;
		}
	}
#ifdef PBG_ADD_NEWCHAR_MONK
	if(GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER)
	{
		if(iIndex == EQUIPMENT_GLOVES)
			bEquipable = false;	//레이지 파이터는 장갑을 착용할수 없다
		else if(pItemAttr->m_byItemSlot == EQUIPMENT_WEAPON_RIGHT)
			bEquipable = g_CMonkSystem.RageEquipmentWeapon(iIndex, pItem->Type);
	}
#endif //PBG_ADD_NEWCHAR_MONK

	if(bEquipable == false)
		return false;

	//////////////////////////////////////////////////////////////////////////
	//. 요구 능력치 체크
	WORD wStrength = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
	WORD wDexterity = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
	WORD wEnergy = CharacterAttribute->Energy + CharacterAttribute->AddEnergy;
	WORD wVitality = CharacterAttribute->Vitality + CharacterAttribute->AddVitality;
	WORD wCharisma = CharacterAttribute->Charisma + CharacterAttribute->AddCharisma;
	WORD wLevel = CharacterAttribute->Level;
	
	int iItemLevel = (pItem->Level>>3)&15;
	
	int iDecNeedStrength = 0, iDecNeedDex = 0;

	extern JewelHarmonyInfo*   g_pUIJewelHarmonyinfo;
	if( iItemLevel >= pItem->Jewel_Of_Harmony_OptionLevel )
	{
		StrengthenCapability SC;
		g_pUIJewelHarmonyinfo->GetStrengthenCapability( &SC, pItem, 0 );
		
		if( SC.SI_isNB ) 
		{
			iDecNeedStrength = SC.SI_NB.SI_force;
			iDecNeedDex      = SC.SI_NB.SI_activity;
		}
	}
#ifdef ADD_SOCKET_STATUS_BONUS
	if (pItem->SocketCount > 0)
	{
		for (int i = 0; i < pItem->SocketCount; ++i)
		{
			if (pItem->SocketSeedID[i] == 38)	// 필요힘 감소
			{
				int iReqStrengthDown = g_SocketItemMgr.GetSocketOptionValue(pItem, i);
				iDecNeedStrength += iReqStrengthDown;
			}
			else if (pItem->SocketSeedID[i] == 39)	// 필요민 감소
			{
				int iReqDexterityDown = g_SocketItemMgr.GetSocketOptionValue(pItem, i);
				iDecNeedDex += iReqDexterityDown;
			}
		}
	}
#endif	// ADD_SOCKET_STATUS_BONUS

	if ( pItem->RequireStrength - iDecNeedStrength > wStrength )
		return false;    //  요구힘보다 작으면 실패.
	if ( pItem->RequireDexterity - iDecNeedDex > wDexterity )
		return false;    //  요구민첩보다 작으면 실패.
	if ( pItem->RequireEnergy > wEnergy )
		return false;    //  요구에너지보다 작으면 실패.
	if ( pItem->RequireVitality > wVitality )
		return false;    //  요구체력보다 작으면 실패.
	if( pItem->RequireCharisma > wCharisma )
		return false;    //  요구통솔보다 작으면 실패.
	if( pItem->RequireLevel > wLevel )
		return false;    //  요구레벨보다 작으면 실패.

#ifdef KJH_FIX_DARKLOAD_PET_SYSTEM
	if(pItem->Type == ITEM_HELPER+5 )		// 다크스피릿
	{
		PET_INFO* pPetInfo = giPetManager::GetPetInfo( pItem );
		WORD wRequireCharisma = (185+(pPetInfo->m_wLevel*15));
		if( wRequireCharisma > wCharisma ) 
			return false;	//  요구통솔보다 작으면 실패.
	}
#else // KJH_FIX_DARKLOAD_PET_SYSTEM								//## 소스정리 대상임.
	PET_INFO petInfo;
	giPetManager::GetPetInfo(petInfo, pItem->Type);
	WORD wRequireCharisma = (185+(petInfo.m_wLevel*15));		// 이혁재 - 다크호스, 스피릿 요구 통솔적용

	if(pItem->Type == 421 )		   // 이혁재 - 다크 스피릿 일때 요구통솔이 작으면 실패
	{
		if( wRequireCharisma > wCharisma ) 
			return false;	//  요구통솔보다 작으면 실패.
	}
#endif // KJH_FIX_DARKLOAD_PET_SYSTEM								//## 소스정리 대상임.
	//////////////////////////////////////////////////////////////////////////
	//  특정 행동이나, 특정 맵에서 장착할수 없는 아이템 체크
	if (World==WD_7ATLANSE && (pItem->Type >= ITEM_HELPER+2 && pItem->Type <= ITEM_HELPER+3))
	{ 
		//. 아틀란스에서 유니리아, 디노란트, 다크호스 불가
		return false;
	}
	else if(pItem->Type==ITEM_HELPER+2 && World==WD_10HEAVEN)
	{
		//. 천공에서 유니리아 불가
		return false;
	}
	else if(pItem->Type==ITEM_HELPER+2 && g_Direction.m_CKanturu.IsMayaScene())
	{
		//. 칸투르 보스전에서 유니리아 불가
		return false;
	}
	// 카오스캐슬에서 장착할 수 없는 아이템
	else if ( InChaosCastle() || (M34CryWolf1st::Get_State_Only_Elf()
		&& g_isCharacterBuff((&Hero->Object), eBuff_CrywolfHeroContracted) ) )  // 크라이울프 MVP에서 요정이 제단에 계약 된 상태 일때 착용 할수 없는것
	{ 
		//. 카캐에서 유니리아, 디노란트, 다크호스, 다크스피릿, 펜릴 불가
		if( (pItem->Type>=ITEM_HELPER+2 && pItem->Type<=ITEM_HELPER+5) || pItem->Type==ITEM_HELPER+37)
			return false;
	}
	else if( ( pItem->Type>=ITEM_HELPER+2 && pItem->Type<=ITEM_HELPER+4 || pItem->Type == ITEM_HELPER+37 )
		&& Hero->Object.CurrentAction>=PLAYER_SIT1 && Hero->Object.CurrentAction<=PLAYER_SIT_FEMALE2 )
	{
		// 캐릭터가 앉아있는 동안은 유니리아, 디노란트, 다크호스, 펜릴 불가
		return false;
	}

	return bEquipable;
}

bool SEASON3B::CNewUIMyInventory::InsertItem(int iIndex, BYTE* pbyItemPacket)
{
	if(m_pNewInventoryCtrl)
	{
		return m_pNewInventoryCtrl->AddItem(iIndex, pbyItemPacket);
	}

	return false;
}
void SEASON3B::CNewUIMyInventory::DeleteItem(int iIndex)
{
	if(m_pNewInventoryCtrl)
	{
		ITEM* pItem = m_pNewInventoryCtrl->FindItem(iIndex);
		if(pItem != NULL)
		{
			m_pNewInventoryCtrl->RemoveItem(pItem);
		}
#ifdef CSK_FIX_WOPS_K30813_HOLLYITEM_SAVE
		else
		{
			CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
			if(pPickedItem)
			{
				if(pPickedItem->GetOwnerInventory() == m_pNewInventoryCtrl)
				{
					if(pPickedItem->GetSourceLinealPos() == iIndex)
					{
						CNewUIInventoryCtrl::DeletePickedItem();
					}
				}
			}
		}
#endif // CSK_FIX_WOPS_K30813_HOLLYITEM_SAVE
	}
}
void SEASON3B::CNewUIMyInventory::DeleteAllItems()
{
	if(m_pNewInventoryCtrl)
		m_pNewInventoryCtrl->RemoveAllItems();
}

void SEASON3B::CNewUIMyInventory::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
	
	//. 다시 셋팅
	SetEquipmentSlotInfo();
	
	m_pNewInventoryCtrl->SetPos(x+15, y+200);
	m_BtnExit.SetPos(m_Pos.x+13, m_Pos.y+391);
	m_BtnRepair.SetPos(m_Pos.x+50, m_Pos.y+391);
	m_BtnMyShop.SetPos(m_Pos.x+87, m_Pos.y+391);
}

const POINT& SEASON3B::CNewUIMyInventory::GetPos() const
{ 
	return m_Pos; 
}

CNewUIMyInventory::REPAIR_MODE SEASON3B::CNewUIMyInventory::GetRepairMode() const
{ 
	return m_RepairMode; 
}

void SEASON3B::CNewUIMyInventory::SetRepairMode(bool bRepair)
{
	if(bRepair)
	{
		m_RepairMode = REPAIR_MODE_ON;
		if(m_pNewInventoryCtrl)
		{
			m_pNewInventoryCtrl->SetRepairMode(true);
		}
	}
	else
	{
		m_RepairMode = REPAIR_MODE_OFF;
		if(m_pNewInventoryCtrl)
		{
			m_pNewInventoryCtrl->SetRepairMode(false);
		}
	}
}

bool SEASON3B::CNewUIMyInventory::UpdateMouseEvent()
{
	if(m_pNewInventoryCtrl && false == m_pNewInventoryCtrl->UpdateMouseEvent())
		return false;

	//. PickedItem 처리
	if(true == EquipmentWindowProcess())		//. 처리가 완료 되었다면
		return false;
	if(true == InventoryProcess())
		return false;

	//. 버튼 처리
	if(true == BtnProcess())	//. 처리가 완료 되었다면
		return false;
	
	//. 버리기
	CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
	if(pPickedItem && SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(0, 0, GetScreenWidth(), 429))
	{
		if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP) == true
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_TRADE) == true 
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_DEVILSQUARE) == true
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_BLOODCASTLE) == true
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MIXINVENTORY) == true
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE) ==true
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MYSHOP_INVENTORY) == true
#ifdef LEM_ADD_LUCKYITEM
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND) ==true
#endif // LEM_ADD_LUCKYITEM
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY) == true )
		{
			ResetMouseLButton();
			return false;
		}

		ITEM* pItemObj = pPickedItem->GetItem();
		if( pItemObj && pItemObj->Jewel_Of_Harmony_Option != 0 )
		{
			// 2217 "강화된 아이템은 버릴 수 없습니다."			
			g_pChatListBox->AddText("", GlobalText[2217], SEASON3B::TYPE_ERROR_MESSAGE);
			
			ResetMouseLButton();
			return false;
		}
		else if(pItemObj && IsHighValueItem(pItemObj) == true)
		{
			// 269 "고가의 아이템은 버릴 수 없습니다."
			g_pChatListBox->AddText("", GlobalText[269], SEASON3B::TYPE_ERROR_MESSAGE);
			CNewUIInventoryCtrl::BackupPickedItem();
			
			ResetMouseLButton();
			return false;
		}
		else if(pItemObj && IsDropBan(pItemObj))
		{
			// "버릴 수 없는 아이템입니다."
			g_pChatListBox->AddText("", GlobalText[1915], SEASON3B::TYPE_ERROR_MESSAGE);
			CNewUIInventoryCtrl::BackupPickedItem();
			
			ResetMouseLButton();
			return false;
		}
#ifdef CSK_FIX_WOPS_K27964_LOSTMAP_POP
		// 잃어버린지도 환영의사원에서는 못버리게 수정
		else if(pItemObj && pItemObj->Type == ITEM_POTION+28 && g_CursedTemple->IsCursedTemple() == true)
		{
			ResetMouseLButton();
			return false;
		}
#endif // CSK_FIX_WOPS_K27964_LOSTMAP_POP
		
		// 칼리마맵에서는 잃어버린 지도를 못버립니다.
		// 던져도 상관없다고 해서 일단 주석으로 처리
// 		else if(pItemObj && pItemObj->Type == ITEM_POTION+28 && InHellas() == true)
// 		{
// 			// 메세지 추가해줘야 한다.
// 
// 			ResetMouseLButton();
// 			return false;
// 		}
		else
		{
			//떨어트리다
			RenderTerrain(true);
			if(RenderTerrainTile(SelectXF,SelectYF,(int)SelectXF,(int)SelectYF,1.f,1,true))
			{
				int iSourceIndex = pPickedItem->GetSourceLinealPos();
				int tx = (int)(CollisionPosition[0]/TERRAIN_SCALE);
				int ty = (int)(CollisionPosition[1]/TERRAIN_SCALE);
				if(pPickedItem->GetOwnerInventory() == m_pNewInventoryCtrl)
				{
					// 사망중에는 던질 수 없다
					if( Hero->Dead == false )
					{
						SendRequestDropItem(MAX_EQUIPMENT_INDEX+iSourceIndex,tx,ty);
					}
				}
				else if(pItemObj && pItemObj->ex_src_type == ITEM_EX_SRC_EQUIPMENT)
				{
					SendRequestDropItem(iSourceIndex,tx,ty);
				}
				MouseUpdateTime = 0;
				MouseUpdateTimeMax = 6;

				ResetMouseLButton();
				return false;
			}
		}
	}

	// 세트 옵션 설명 리스트
	g_csItemOption.SetViewOptionList(false);
#ifdef SOCKET_SYSTEM
	if(CheckMouseIn(m_Pos.x, m_Pos.y+20, INVENTORY_WIDTH * 0.5f, 15) == true)
#else	// SOCKET_SYSTEM
	if(CheckMouseIn(m_Pos.x, m_Pos.y+20, INVENTORY_WIDTH, 15) == true)
#endif	// SOCKET_SYSTEM
	{
		g_csItemOption.SetViewOptionList(true);	
	}

	//. 인벤토리 내의 영역 클릭시 하위 UI처리 및 이동 불가
	if(CheckMouseIn(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT))
	{
		if(SEASON3B::IsPress(VK_RBUTTON))
		{
			ResetMouseRButton();
			return false;
		}

		if(SEASON3B::IsNone(VK_LBUTTON) == false)
		{
			return false;
		}
	}

	return true;
}

bool SEASON3B::CNewUIMyInventory::UpdateKeyEvent()
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY))
	{
		if(SEASON3B::IsPress(VK_ESCAPE) == true)
		{
#ifdef YDG_FIX_NPCSHOP_SELLING_LOCK
			if (g_pNPCShop->IsSellingItem() == false)
#endif	// YDG_FIX_NPCSHOP_SELLING_LOCK
			{
				g_pNewUISystem->Hide(SEASON3B::INTERFACE_INVENTORY);
				PlayBuffer(SOUND_CLICK01);
			}
			return false;
		}
		else if(SEASON3B::IsPress('L') == true)
		{
			if(m_bRepairEnableLevel == true && g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP) == false
#ifdef PBG_WOPS_REPAIRKEY
				//조합창사용시 수리는 안되게 된다
				&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MIXINVENTORY) == false
#endif //PBG_WOPS_REPAIRKEY
#ifdef LEM_ADD_LUCKYITEM
				&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND) == false
#endif // LEM_ADD_LUCKYITEM
				)
			{
				ToggleRepairMode();
				
				return false;
			}
		}

		// 개인상점 인벤토리 단축키 
		if(CanOpenMyShopInterface() == true && SEASON3B::IsPress('S') )	
		{
#ifdef DO_PROCESS_DEBUGCAMERA
			if( EDEBUGCAMERA_NONE == g_pDebugCameraManager->GetActiveCameraMode() )
			{
				if( m_bMyShopOpen )
				{
					// 개인상점 토글 모드
					if(m_MyShopMode == MYSHOP_MODE_OPEN)
					{	
						ChangeMyShopButtonStateClose();
					}
					else if(m_MyShopMode == MYSHOP_MODE_CLOSE)
					{
						ChangeMyShopButtonStateOpen();
					}
					g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MYSHOP_INVENTORY);
					PlayBuffer(SOUND_CLICK01);
				}
			}
#else // DO_PROCESS_DEBUGCAMERA
			if( m_bMyShopOpen )
			{
				// 개인상점 토글 모드
				if(m_MyShopMode == MYSHOP_MODE_OPEN)
				{	
					ChangeMyShopButtonStateClose();
				}
				else if(m_MyShopMode == MYSHOP_MODE_CLOSE)
				{
					ChangeMyShopButtonStateOpen();
				}
				g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MYSHOP_INVENTORY);
				PlayBuffer(SOUND_CLICK01);
			}
#endif // DO_PROCESS_DEBUGCAMERA
			return false;
		}

		if(CheckMouseIn(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT) == false)
		{
			return true;
		}

		if(SEASON3B::IsRepeat(VK_CONTROL))
		{
			int iHotKey = -1;
			if(SEASON3B::IsPress('Q'))
			{
				iHotKey = HOTKEY_Q;
			}
			else if(SEASON3B::IsPress('W'))
			{
				iHotKey = HOTKEY_W;
			}
			else if(SEASON3B::IsPress('E'))
			{
				iHotKey = HOTKEY_E;
			}
			else if(SEASON3B::IsPress('R'))
			{
				iHotKey = HOTKEY_R;
			}
			
			if(iHotKey != -1)
			{
				ITEM* pItem = m_pNewInventoryCtrl->FindItemAtPt(MouseX, MouseY);
				if(pItem == NULL)
				{
					return false;
				}

				if(CanRegisterItemHotKey(pItem->Type) == true)
				{
					int iItemLevel = (pItem->Level >> 3) & 15;

#ifdef LDK_ADD_SCALEFORM
					if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
					{
						g_pMainFrame->SetItemHotKey(iHotKey, pItem->Type, iItemLevel);
					}
					else
					{
						GFxProcess::GetInstancePtr()->SetItemHotKey(iHotKey, pItem->Type, iItemLevel);
					}
#else //LDK_ADD_SCALEFORM
					g_pMainFrame->SetItemHotKey(iHotKey, pItem->Type, iItemLevel);
#endif //LDK_ADD_SCALEFORM

					return false;
				}

			}
		}
	}
	
	return true;
}

bool SEASON3B::CNewUIMyInventory::Update()
{
#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_UPDATE_NEWUIINVENTORY, PROFILING_UPDATE_NEWUIINVENTORY );
#endif // DO_PROFILING

	if(m_pNewInventoryCtrl && false == m_pNewInventoryCtrl->Update())
	{
#ifdef DO_PROFILING
		g_pProfiler->EndUnit( EPROFILING_UPDATE_NEWUIINVENTORY );
#endif // DO_PROFILING
		return false;
	}

	if(IsVisible())
	{
		m_iPointedSlot = -1;
		for(int i=0; i<MAX_EQUIPMENT_INDEX; i++)
		{
			if(CheckMouseIn(m_EquipmentSlots[i].x+1, m_EquipmentSlots[i].y, 
				m_EquipmentSlots[i].width-4, m_EquipmentSlots[i].height-4))
			{
				m_iPointedSlot = i;
				break;
			}
		}
	}
#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_UPDATE_NEWUIINVENTORY );
#endif // DO_PROFILING
	return true;
}

bool SEASON3B::CNewUIMyInventory::Render()
{
#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_RENDER_NEWUIINVENTORY, PROFILING_RENDER_NEWUIINVENTORY );
#endif // DO_PROFILING
	EnableAlphaTest();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	RenderFrame();
	
	RenderInventoryDetails();

	RenderSetOption();

#ifdef SOCKET_SYSTEM
	RenderSocketOption();
#endif	// SOCKET_SYSTEM

	RenderButtons();

	if(m_pNewInventoryCtrl)
		m_pNewInventoryCtrl->Render();

	RenderEquippedItem();

	DisableAlphaBlend();

#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_RENDER_NEWUIINVENTORY );
#endif // DO_PROFILING
	
	return true;
}

void SEASON3B::CNewUIMyInventory::RenderSetOption()
{
#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_RENDER_NEWUIINVENTORY_RENDERSETOPTION, PROFILING_RENDER_NEWUIINVENTORY_RENDERSETOPTION );
#endif // DO_PROFILING
	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetBgColor(0, 0, 0, 0);
	if(g_csItemOption.GetSetOptionANum() > 0 || g_csItemOption.GetSetOptionBNum() > 0)
	{
		g_pRenderText->SetTextColor(255, 204, 25, 255);
	}
	else 
	{
		g_pRenderText->SetTextColor(100, 100, 100, 255);
	}

	// 989 "세트 옵션"
	unicode::t_char strText[128];
	unicode::_sprintf(strText, "[%s]", GlobalText[989]);
#ifdef SOCKET_SYSTEM
	g_pRenderText->RenderText(m_Pos.x + INVENTORY_WIDTH * 0.2f, m_Pos.y+25, strText, INVENTORY_WIDTH * 0.3f, 0, RT3_SORT_CENTER);
#else	// SOCKET_SYSTEM]
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y+25, strText, INVENTORY_WIDTH, 0, RT3_SORT_CENTER);
#endif	// SOCKET_SYSTEM

	if(g_csItemOption.IsViewOptionList() == true)
	{
		m_pNewUI3DRenderMng->RenderUI2DEffect(INVENTORY_CAMERA_Z_ORDER, UI2DEffectCallback, this, -1, ITEM_SET_OPTION);
	}
#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_RENDER_NEWUIINVENTORY_RENDERSETOPTION );
#endif // DO_PROFILING
}

#ifdef SOCKET_SYSTEM
void SEASON3B::CNewUIMyInventory::RenderSocketOption()
{
#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_RENDER_NEWUIINVENTORY_RENDERSOCKETOPTION, PROFILING_RENDER_NEWUIINVENTORY_RENDERSOCKETOPTION );
#endif // DO_PROFILING
	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetBgColor(0, 0, 0, 0);

	if(g_SocketItemMgr.IsSocketSetOptionEnabled())
	{
		g_pRenderText->SetTextColor(255, 204, 25, 255);
	}
	else 
	{
		g_pRenderText->SetTextColor(100, 100, 100, 255);
	}

	// "소켓 옵션"
	unicode::t_char strText[128];
	unicode::_sprintf(strText, "[%s]", GlobalText[2651]);	// "소켓 옵션"
	g_pRenderText->RenderText(m_Pos.x + INVENTORY_WIDTH * 0.5f, m_Pos.y+25, strText, INVENTORY_WIDTH * 0.3f, 0, RT3_SORT_CENTER);

	if(CheckMouseIn(m_Pos.x + INVENTORY_WIDTH * 0.5f, m_Pos.y+20, INVENTORY_WIDTH * 0.5f, 15) == true)
	{
		m_pNewUI3DRenderMng->RenderUI2DEffect(INVENTORY_CAMERA_Z_ORDER, UI2DEffectCallback, this, -1, ITEM_SOCKET_SET_OPTION);
	}
#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_RENDER_NEWUIINVENTORY_RENDERSOCKETOPTION );
#endif // DO_PROFILING
}
#endif	// SOCKET_SYSTEM

void SEASON3B::CNewUIMyInventory::Render3D()
{
	//. 장착된 무기 렌더링

	for(int i=0; i<MAX_EQUIPMENT_INDEX; i++)
	{
		ITEM* pEquippedItem = &CharacterMachine->Equipment[i];
		if(pEquippedItem->Type >= 0)
		{
			float y = 0.f;
			if(i == EQUIPMENT_ARMOR)
			{
				y = m_EquipmentSlots[i].y - 10.f;
			}
			else
			{
				y = m_EquipmentSlots[i].y;
			}

			glColor4f(1.f, 1.f, 1.f, 1.f);
			RenderItem3D(
				m_EquipmentSlots[i].x+1, 
				y, 
				m_EquipmentSlots[i].width-4, 
				m_EquipmentSlots[i].height-4, 
				pEquippedItem->Type, 
				pEquippedItem->Level, 
				pEquippedItem->Option1, 
				pEquippedItem->ExtOption, 
				false);
		}
	}

}

bool SEASON3B::CNewUIMyInventory::IsVisible() const
{ 
	return CNewUIObj::IsVisible(); 
}

void SEASON3B::CNewUIMyInventory::OpenningProcess()
{
#ifdef LDK_ADD_SCALEFORM
	if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
	{
#ifdef KJH_MODIFY_MYINVENTORY_BTN
		g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);
#endif // KJH_MODIFY_MYINVENTORY_BTN
	}
#else //LDK_ADD_SCALEFORM
#ifdef KJH_MODIFY_MYINVENTORY_BTN
	g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);
#endif // KJH_MODIFY_MYINVENTORY_BTN
#endif //LDK_ADD_SCALEFORM

	SetRepairMode(false);

	m_MyShopMode = MYSHOP_MODE_OPEN;
	ChangeMyShopButtonStateOpen();

	WORD wLevel = CharacterAttribute->Level;

	// 수리가능한 레벨인가? 50부터 수리 가능
	if(wLevel >= 50)
	{
		m_bRepairEnableLevel = true;
	}
	else
	{
		m_bRepairEnableLevel = false;	
	}
	
	// 개인상점 레벨 6부터 가능
	if(wLevel >= 6
#ifdef LJH_MOD_CANT_OPENNING_PERSONALSHOP_WIN_IN_CURSED_TEMPLE
		// 개인상점 안되는 조건에 "환영사원에 있을 때" 추가
		&& !g_CursedTemple->IsCursedTemple()
#endif //LJH_MOD_CANT_OPENNING_PERSONALSHOP_WIN_IN_CURSED_TEMPLE
		)
	{
		m_bMyShopOpen = true;
		//UnlockMyShopButtonOpen();
	}
	else
	{
		m_bMyShopOpen = false;
		//LockMyShopButtonOpen();
	}

#ifdef ASG_ADD_NEW_QUEST_SYSTEM
// 장비창을 여는 퀘스트일 때 서버로 한 번만 알림.
	if (g_QuestMng.IsIndexInCurQuestIndexList(0x1000F))	// 0x1000F가 장비창을 여는 퀘스트 인덱스.
	{
		if (g_QuestMng.IsEPRequestRewardState(0x1000F))
		{
			SendSatisfyQuestRequestFromClient(0x1000F);		// 서버로 장비창이 열렸음을 알림.
			g_QuestMng.SetEPRequestRewardState(0x1000F, false);	// 서버로 한 번만 알리기 위해.
		}
	}
#endif	// ASG_ADD_NEW_QUEST_SYSTEM

#ifndef KJH_FIX_DARKLOAD_PET_SYSTEM							//## 소스정리 대상임.
	//인벤토리를 열 때마다 펫 정보 리셋을 요구한다.
	if(GetBaseClass(Hero->Class)==CLASS_DARK_LORD)
	{
		SendRequestPetInfo ( 0, 0, EQUIPMENT_WEAPON_LEFT );
		SendRequestPetInfo ( 1, 0, EQUIPMENT_HELPER );
	}
#endif // KJH_FIX_DARKLOAD_PET_SYSTEM						//##소스정리 대상임.
}

void SEASON3B::CNewUIMyInventory::ClosingProcess()
{
#ifdef LDK_ADD_SCALEFORM
	if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
	{
#ifdef KJH_MODIFY_MYINVENTORY_BTN
		g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, false);
#endif // KJH_MODIFY_MYINVENTORY_BTN
	}
#else //LDK_ADD_SCALEFORM
#ifdef KJH_MODIFY_MYINVENTORY_BTN
	g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, false);
#endif // KJH_MODIFY_MYINVENTORY_BTN
#endif //LDK_ADD_SCALEFORM

#ifdef KJH_FIX_WOPS_K29690_PICKED_ITEM_BACKUP
	// 창이 닫힐때 아이템이 Picked되어있으면 되돌린다.
	m_pNewInventoryCtrl->BackupPickedItem();
#endif // KJH_FIX_WOPS_K29690_PICKED_ITEM_BACKUP
	
#ifdef LDK_FIX_REPAIR_CURSOR_FREEZE
	// 창 닫힐때 수리 모드를 완전 해제 한다.
	RepairEnable = 0;
#endif //LDK_FIX_REPAIR_CURSOR_FREEZE

	SetRepairMode(false);
}

float SEASON3B::CNewUIMyInventory::GetLayerDepth() 
{ 
	return 4.2f; 
}

CNewUIInventoryCtrl* SEASON3B::CNewUIMyInventory::GetInventoryCtrl() const
{ 
	return m_pNewInventoryCtrl; 
}

ITEM* SEASON3B::CNewUIMyInventory::FindItem(int iLinealPos)
{
	if(m_pNewInventoryCtrl)
		return m_pNewInventoryCtrl->FindItem(iLinealPos);
	return NULL;
}

ITEM* SEASON3B::CNewUIMyInventory::FindItemByKey(DWORD dwKey)
{
	if(m_pNewInventoryCtrl)
		return m_pNewInventoryCtrl->FindItemByKey(dwKey);
	return NULL;
}

int SEASON3B::CNewUIMyInventory::FindItemIndex( short int siType, int iLevel )
{
	if(m_pNewInventoryCtrl)
		return m_pNewInventoryCtrl->FindItemIndex( siType, iLevel );
	return -1;
}

int SEASON3B::CNewUIMyInventory::FindItemReverseIndex(short sType, int iLevel)
{
	if(m_pNewInventoryCtrl)
	{
		return m_pNewInventoryCtrl->FindItemReverseIndex( sType, iLevel );
	}

	return -1;
}

int SEASON3B::CNewUIMyInventory::FindEmptySlot(IN int cx, IN int cy)
{
	if(m_pNewInventoryCtrl)
		return m_pNewInventoryCtrl->FindEmptySlot(cx, cy);
	return -1;
}

int SEASON3B::CNewUIMyInventory::FindEmptySlot(ITEM* pItem)
{
	ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
	if(m_pNewInventoryCtrl)
	{
		return m_pNewInventoryCtrl->FindEmptySlot(pItemAttr->Width, pItemAttr->Height);
	}

	return -1;
}

void SEASON3B::CNewUIMyInventory::UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB)
{
	if(pClass)
	{
		CNewUIMyInventory* pMyInventory = (CNewUIMyInventory*)(pClass);

		if(dwParamB == ITEM_SET_OPTION)
		{
			g_csItemOption.RenderSetOptionList(pMyInventory->GetPos().x, pMyInventory->GetPos().y);
		}
#ifdef SOCKET_SYSTEM
		else if (dwParamB == ITEM_SOCKET_SET_OPTION)
		{
			g_SocketItemMgr.RenderToolTipForSocketSetOption(pMyInventory->GetPos().x, pMyInventory->GetPos().y);
		}
#endif	// SOCKET_SYSTEM
		else
		{
			pMyInventory->RenderItemToolTip(dwParamA);
		}
	}
}

void SEASON3B::CNewUIMyInventory::SetStandbyItemKey(DWORD dwItemKey)
{ 
	m_dwStandbyItemKey = dwItemKey; 
}

DWORD SEASON3B::CNewUIMyInventory::GetStandbyItemKey() const
{ 
	return m_dwStandbyItemKey; 
}

int SEASON3B::CNewUIMyInventory::GetStandbyItemIndex()
{
	ITEM* pItem = GetStandbyItem();
	if(pItem)
		return pItem->y*m_pNewInventoryCtrl->GetNumberOfColumn()+pItem->x;
	return -1;
}

ITEM* SEASON3B::CNewUIMyInventory::GetStandbyItem()
{
	if(m_pNewInventoryCtrl)
		return m_pNewInventoryCtrl->FindItemByKey(m_dwStandbyItemKey);
	return NULL;
}

void SEASON3B::CNewUIMyInventory::CreateEquippingEffect(ITEM* pItem)
{
	//////////////////////////////////////////////////////////////////////////
	//. 코드 그대로 옮김
	SetCharacterClass(Hero);
	OBJECT* pHeroObject = &Hero->Object;
	if(false == InChaosCastle())
	{
		switch(pItem->Type)
		{
		case ITEM_HELPER:
			CreateBug(MODEL_HELPER, pHeroObject->Position, pHeroObject);
			break;
		case ITEM_HELPER+2:
			CreateBug(MODEL_UNICON, pHeroObject->Position, pHeroObject);
			if(!Hero->SafeZone)
				CreateEffect(BITMAP_MAGIC+1, pHeroObject->Position, pHeroObject->Angle, pHeroObject->Light, 1, pHeroObject);
			break;
		case ITEM_HELPER+3:
			CreateBug(MODEL_PEGASUS, pHeroObject->Position, pHeroObject);
			if(!Hero->SafeZone)
				CreateEffect(BITMAP_MAGIC+1, pHeroObject->Position, pHeroObject->Angle, pHeroObject->Light, 1, pHeroObject);
			break;
		case ITEM_HELPER+4:
			CreateBug(MODEL_DARK_HORSE, pHeroObject->Position, pHeroObject);
			if(!Hero->SafeZone)
				CreateEffect(BITMAP_MAGIC+1, pHeroObject->Position, pHeroObject->Angle, pHeroObject->Light, 1, pHeroObject);
			break;
		case ITEM_HELPER+37:	//^ 펜릴 인벤토리에 넣을때 생성및 효과
			Hero->Helper.Option1 = pItem->Option1;
			if(pItem->Option1 == 0x01)
			{
				CreateBug(MODEL_FENRIR_BLACK, pHeroObject->Position, pHeroObject);
			}
			else if(pItem->Option1 == 0x02)
			{
				CreateBug(MODEL_FENRIR_BLUE, pHeroObject->Position, pHeroObject);
			}
			else if(pItem->Option1 == 0x04)
			{
				CreateBug(MODEL_FENRIR_GOLD, pHeroObject->Position, pHeroObject);
			}
			else
			{
				CreateBug(MODEL_FENRIR_RED, pHeroObject->Position, pHeroObject);
			}

			if(!Hero->SafeZone)
			{
				CreateEffect(BITMAP_MAGIC+1,pHeroObject->Position, pHeroObject->Angle, pHeroObject->Light, 1, pHeroObject);
			}
			break;
#if defined( LDK_ADD_NEW_PETPROCESS ) && defined( LDK_ADD_PC4_GUARDIAN )
		case ITEM_HELPER+64:
			ThePetProcess().CreatePet(pItem->Type, MODEL_HELPER+64, pHeroObject->Position, Hero);
			break;
		case ITEM_HELPER+65:
			ThePetProcess().CreatePet(pItem->Type, MODEL_HELPER+65, pHeroObject->Position, Hero);
			break;
#endif //defined( LDK_ADD_NEW_PETPROCESS ) && defined( LDK_ADD_PC4_GUARDIAN )

#if defined( LDK_ADD_NEW_PETPROCESS ) && defined( LDK_ADD_RUDOLPH_PET )
		case ITEM_HELPER+67: //루돌프 펫 이름 출력
			ThePetProcess().CreatePet(pItem->Type, MODEL_HELPER+67, pHeroObject->Position, Hero);
			break;
#endif //defined( LDK_ADD_NEW_PETPROCESS ) && defined( LDK_ADD_RUDOLPH_PET )

#if defined( LDK_ADD_NEW_PETPROCESS ) && defined( PJH_ADD_PANDA_PET )
		case ITEM_HELPER+80: //팬더 펫 이름 출력
			ThePetProcess().CreatePet(pItem->Type, MODEL_HELPER+80, pHeroObject->Position, Hero);
			break;
#endif //defined( LDK_ADD_NEW_PETPROCESS ) && defined( PJH_ADD_PANDA_PET )
			
#if defined( LDK_ADD_NEW_PETPROCESS ) && defined( LDK_ADD_CS7_UNICORN_PET )
		case ITEM_HELPER+106: //유니콘 펫 이름 출력
			ThePetProcess().CreatePet(pItem->Type, MODEL_HELPER+106, pHeroObject->Position, Hero);
			break;
#endif //defined( LDK_ADD_NEW_PETPROCESS ) && defined( LDK_ADD_CS7_UNICORN_PET )

#if defined( LDK_ADD_NEW_PETPROCESS ) && defined( YDG_ADD_SKELETON_PET )
		case ITEM_HELPER+123: // 스켈레톤 펫 이름 출력
			ThePetProcess().CreatePet(pItem->Type, MODEL_HELPER+123, pHeroObject->Position, Hero);
			break;
#endif //defined( LDK_ADD_NEW_PETPROCESS ) && defined( YDG_ADD_SKELETON_PET )
		}
	}
	
#ifndef KJH_FIX_DARKLOAD_PET_SYSTEM						//## 소스정리 대상임.
#ifdef PET_SYSTEM
	switch (pItem->Type)
	{
	case ITEM_HELPER+5:
		giPetManager::CreatePetDarkSpirit ( Hero );            
		break;
	}
#endif// PET_SYSTEM										
#endif // KJH_FIX_DARKLOAD_PET_SYSTEM					//## 소스정리 대상임.
#ifdef LJW_FIX_PARTS_ENUM
	if(Hero->EtcPart<=PARTS_NONE || Hero->EtcPart>=PARTS_LION)
#else
	if(Hero->EtcPart<=0 || Hero->EtcPart>3)
#endif //LJW_FIX_PARTS_ENUM
	{
#ifdef LJW_FIX_MANY_FLAG_DISAPPEARED_PROBREM
		// 영예의 반지 착용시 중국 경극 깃발 생성. 단, 내구도가 0일때는 경극 깃발 생성 안함
		if ( pItem->Type==ITEM_HELPER+20 && (pItem->Level>>3)==3 && pItem->Durability > 0)
#else
		if ( pItem->Type==ITEM_HELPER+20 && (pItem->Level>>3)==3 )
#endif //LJW_FIX_MANY_FLAG_DISAPPEARED_PROBREM
		{
			DeleteParts ( Hero );
			Hero->EtcPart = PARTS_LION;
		}
	}
	if (pItem->Type==ITEM_WING+39 || pItem->Type==ITEM_HELPER+30 || 
#ifdef LDK_ADD_INGAMESHOP_SMALL_WING
		pItem->Type == ITEM_WING+130 ||
#endif //LDK_ADD_INGAMESHOP_SMALL_WING
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		(pItem->Type >= ITEM_WING+49 && pItem->Type <= ITEM_WING+50) ||
		(pItem->Type == ITEM_WING+135) ||
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		pItem->Type==ITEM_WING+40)	// 날개 장착시 망토 모양 변경
	{
		DeleteCloth(Hero, &Hero->Object);
	}
	
	//////////////////////////////////////////////////////////////////////////
}
#ifdef PBG_FIX_SKILLHOTKEY
void SEASON3B::CNewUIMyInventory::ResetSkillofItem(ITEM* pItem)
{
	ITEM_ATTRIBUTE* ipattribute = &ItemAttribute[pItem->Type];
	int SkillIndex = ipattribute->m_bySkillIndex;
#ifdef LDK_ADD_SCALEFORM
	if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
	{
		SkillIndex = g_pMainFrame->GetSkillHotKeyIndex(SkillIndex);
		g_pMainFrame->SetSkillHotKeyClear(SkillIndex);	
	}
#else //LDK_ADD_SCALEFORM
	SkillIndex = g_pMainFrame->GetSkillHotKeyIndex(SkillIndex);
	g_pMainFrame->SetSkillHotKeyClear(SkillIndex);	
#endif //LDK_ADD_SCALEFORM
}
#endif //PBG_FIX_SKILLHOTKEY
void SEASON3B::CNewUIMyInventory::DeleteEquippingEffectBug(ITEM* pItem)
{
#ifdef LDK_ADD_NEW_PETPROCESS
	if( ThePetProcess().IsPet(pItem->Type) == true )
	{
		ThePetProcess().DeletePet( Hero, pItem->Type );
	}
#endif //LDK_ADD_NEW_PETPROCESS
	
	switch(pItem->Type)
	{
#ifndef KJH_FIX_DARKLOAD_PET_SYSTEM				//## 소스정리 대상임.
	case ITEM_HELPER+5:	// 다크스피릿
		giPetManager::DeletePet(Hero);
		return;
#endif // KJH_FIX_DARKLOAD_PET_SYSTEM			//## 소스정리 대상임.

	case ITEM_HELPER+30:	// 군주의망토
	case ITEM_WING+39:		// 파멸의날개	
	case ITEM_WING+40:		// 제왕의망토
#ifdef LDK_ADD_INGAMESHOP_SMALL_WING
	case ITEM_WING+130:		// 작은 군주의망토
#endif //LDK_ADD_INGAMESHOP_SMALL_WING
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
	case ITEM_WING+49:		// 무인의망토
	case ITEM_WING+50:		// 군림의망토
	case ITEM_WING+135:		// 작은무인의 망토
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		DeleteCloth(Hero, &Hero->Object);
		return;
	}

	if(IsBug(pItem) == true)
	{
		DeleteBug(&Hero->Object);
	}
}

void SEASON3B::CNewUIMyInventory::DeleteEquippingEffect()
{
	//////////////////////////////////////////////////////////////////////////
	//. 코드 그대로 옮김
	
	if(Hero->EtcPart < PARTS_ATTACK_TEAM_MARK)
	{
#ifdef LJW_FIX_MANY_FLAG_DISAPPEARED_PROBREM
		if(Hero->EtcPart == PARTS_LION)	
		{
			DeleteParts(Hero);
			Hero->EtcPart = PARTS_LION;
		}
		else 
		{
			DeleteParts(Hero);
			if(Hero->EtcPart >= PARTS_WEBZEN)	
			{
				Hero->EtcPart = PARTS_NONE;
			}
		}
		
#else
		DeleteParts(Hero);
		if(Hero->EtcPart > 3)
		{
			Hero->EtcPart = 0;
		}
#endif //LJW_FIX_MANY_FLAG_DISAPPEARED_PROBREM
	}

	SetCharacterClass(Hero);
}

//////////////////////////////////////////////////////////////////////////

void SEASON3B::CNewUIMyInventory::SetEquipmentSlotInfo()
{
	//. 펫
	m_EquipmentSlots[EQUIPMENT_HELPER].x = m_Pos.x+15;
	m_EquipmentSlots[EQUIPMENT_HELPER].y = m_Pos.y+44;
	m_EquipmentSlots[EQUIPMENT_HELPER].width = 46;
	m_EquipmentSlots[EQUIPMENT_HELPER].height = 46;
	m_EquipmentSlots[EQUIPMENT_HELPER].dwBgImage = IMAGE_INVENTORY_ITEM_FAIRY;

	//. 투구
	m_EquipmentSlots[EQUIPMENT_HELM].x = m_Pos.x+75;
	m_EquipmentSlots[EQUIPMENT_HELM].y = m_Pos.y+44;
	m_EquipmentSlots[EQUIPMENT_HELM].width = 46;
	m_EquipmentSlots[EQUIPMENT_HELM].height = 46;
	m_EquipmentSlots[EQUIPMENT_HELM].dwBgImage = IMAGE_INVENTORY_ITEM_HELM;

	//. 날개
	m_EquipmentSlots[EQUIPMENT_WING].x = m_Pos.x+120;
	m_EquipmentSlots[EQUIPMENT_WING].y = m_Pos.y+44;
	m_EquipmentSlots[EQUIPMENT_WING].width = 61;
	m_EquipmentSlots[EQUIPMENT_WING].height = 46;
	m_EquipmentSlots[EQUIPMENT_WING].dwBgImage = IMAGE_INVENTORY_ITEM_WING;

	//. 왼손무기
	m_EquipmentSlots[EQUIPMENT_WEAPON_LEFT].x = m_Pos.x+135;
	m_EquipmentSlots[EQUIPMENT_WEAPON_LEFT].y = m_Pos.y+87;
	m_EquipmentSlots[EQUIPMENT_WEAPON_LEFT].width = 46;
	m_EquipmentSlots[EQUIPMENT_WEAPON_LEFT].height = 66;
	m_EquipmentSlots[EQUIPMENT_WEAPON_LEFT].dwBgImage = IMAGE_INVENTORY_ITEM_LEFT;

	//. 갑옷
	m_EquipmentSlots[EQUIPMENT_ARMOR].x = m_Pos.x+75;
	m_EquipmentSlots[EQUIPMENT_ARMOR].y = m_Pos.y+87;
	m_EquipmentSlots[EQUIPMENT_ARMOR].width = 46;
	m_EquipmentSlots[EQUIPMENT_ARMOR].height = 66;
	m_EquipmentSlots[EQUIPMENT_ARMOR].dwBgImage = IMAGE_INVENTORY_ITEM_ARMOR;

	//. 오른손 무기
	m_EquipmentSlots[EQUIPMENT_WEAPON_RIGHT].x = m_Pos.x+15;
	m_EquipmentSlots[EQUIPMENT_WEAPON_RIGHT].y = m_Pos.y+87;
	m_EquipmentSlots[EQUIPMENT_WEAPON_RIGHT].width = 46;
	m_EquipmentSlots[EQUIPMENT_WEAPON_RIGHT].height = 66;
	m_EquipmentSlots[EQUIPMENT_WEAPON_RIGHT].dwBgImage = IMAGE_INVENTORY_ITEM_RIGHT;

	//. 장갑
	m_EquipmentSlots[EQUIPMENT_GLOVES].x = m_Pos.x+15;
	m_EquipmentSlots[EQUIPMENT_GLOVES].y = m_Pos.y+150;
	m_EquipmentSlots[EQUIPMENT_GLOVES].width = 46;
	m_EquipmentSlots[EQUIPMENT_GLOVES].height = 46;
	m_EquipmentSlots[EQUIPMENT_GLOVES].dwBgImage = IMAGE_INVENTORY_ITEM_GLOVES;

	//. 바지
	m_EquipmentSlots[EQUIPMENT_PANTS].x = m_Pos.x+75;
	m_EquipmentSlots[EQUIPMENT_PANTS].y = m_Pos.y+150;
	m_EquipmentSlots[EQUIPMENT_PANTS].width = 46;
	m_EquipmentSlots[EQUIPMENT_PANTS].height = 46;
	m_EquipmentSlots[EQUIPMENT_PANTS].dwBgImage = IMAGE_INVENTORY_ITEM_PANTS;

	//. 신발
	m_EquipmentSlots[EQUIPMENT_BOOTS].x = m_Pos.x+135;
	m_EquipmentSlots[EQUIPMENT_BOOTS].y = m_Pos.y+150;
	m_EquipmentSlots[EQUIPMENT_BOOTS].width = 46;
	m_EquipmentSlots[EQUIPMENT_BOOTS].height = 46;
	m_EquipmentSlots[EQUIPMENT_BOOTS].dwBgImage = IMAGE_INVENTORY_ITEM_BOOT;

	//. 왼쪽 반지
	m_EquipmentSlots[EQUIPMENT_RING_LEFT].x = m_Pos.x+114;
	m_EquipmentSlots[EQUIPMENT_RING_LEFT].y = m_Pos.y+150;
	m_EquipmentSlots[EQUIPMENT_RING_LEFT].width = 28;
	m_EquipmentSlots[EQUIPMENT_RING_LEFT].height = 28;
	m_EquipmentSlots[EQUIPMENT_RING_LEFT].dwBgImage = IMAGE_INVENTORY_ITEM_RING;

	//. 목걸이
	m_EquipmentSlots[EQUIPMENT_AMULET].x = m_Pos.x+54;
	m_EquipmentSlots[EQUIPMENT_AMULET].y = m_Pos.y+87;
	m_EquipmentSlots[EQUIPMENT_AMULET].width = 28;
	m_EquipmentSlots[EQUIPMENT_AMULET].height = 28;
	m_EquipmentSlots[EQUIPMENT_AMULET].dwBgImage = IMAGE_INVENTORY_ITEM_NECKLACE;

	//. 오른쪽 반지
	m_EquipmentSlots[EQUIPMENT_RING_RIGHT].x = m_Pos.x+54;
	m_EquipmentSlots[EQUIPMENT_RING_RIGHT].y = m_Pos.y+150;
	m_EquipmentSlots[EQUIPMENT_RING_RIGHT].width = 28;
	m_EquipmentSlots[EQUIPMENT_RING_RIGHT].height = 28;
	m_EquipmentSlots[EQUIPMENT_RING_RIGHT].dwBgImage = IMAGE_INVENTORY_ITEM_RING;
}

void SEASON3B::CNewUIMyInventory::SetButtonInfo()
{
	//. 닫기 버튼
	m_BtnExit.ChangeButtonImgState(true, IMAGE_INVENTORY_EXIT_BTN, false);
	m_BtnExit.ChangeButtonInfo(m_Pos.x+13, m_Pos.y+391, 36, 29);
	m_BtnExit.ChangeToolTipText(GlobalText[225], true); // 225 "닫기(I,V)"

	//. 수리 버튼
	m_BtnRepair.ChangeButtonImgState(true, IMAGE_INVENTORY_REPAIR_BTN, false);
	m_BtnRepair.ChangeButtonInfo(m_Pos.x+50, m_Pos.y+391, 36, 29);
	m_BtnRepair.ChangeToolTipText(GlobalText[233], true); // 233 "수리(R)"

	// 개인 삼정 버튼
	m_BtnMyShop.ChangeButtonImgState(true, IMAGE_INVENTORY_MYSHOP_OPEN_BTN, false);
	m_BtnMyShop.ChangeButtonInfo(m_Pos.x+87, m_Pos.y+391, 36, 29);
	// 1125 "개인상점열기(S)"
	m_BtnMyShop.ChangeToolTipText(GlobalText[1125], true);
}

void SEASON3B::CNewUIMyInventory::LoadImages()
{
	LoadBitmap("Interface\\newui_msgbox_back.jpg", IMAGE_INVENTORY_BACK, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back01.tga", IMAGE_INVENTORY_BACK_TOP, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back04.tga", IMAGE_INVENTORY_BACK_TOP2, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-L.tga", IMAGE_INVENTORY_BACK_LEFT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-R.tga", IMAGE_INVENTORY_BACK_RIGHT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back03.tga", IMAGE_INVENTORY_BACK_BOTTOM, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_boots.tga", IMAGE_INVENTORY_ITEM_BOOT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_cap.tga", IMAGE_INVENTORY_ITEM_HELM, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_fairy.tga", IMAGE_INVENTORY_ITEM_FAIRY, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_wing.tga", IMAGE_INVENTORY_ITEM_WING, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_weapon(L).tga", IMAGE_INVENTORY_ITEM_RIGHT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_weapon(R).tga", IMAGE_INVENTORY_ITEM_LEFT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_upper.tga", IMAGE_INVENTORY_ITEM_ARMOR, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_gloves.tga", IMAGE_INVENTORY_ITEM_GLOVES, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_lower.tga", IMAGE_INVENTORY_ITEM_PANTS, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_ring.tga", IMAGE_INVENTORY_ITEM_RING, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_necklace.tga", IMAGE_INVENTORY_ITEM_NECKLACE, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_money.tga", IMAGE_INVENTORY_MONEY, GL_LINEAR);
	LoadBitmap("Interface\\newui_exit_00.tga", IMAGE_INVENTORY_EXIT_BTN, GL_LINEAR);
	LoadBitmap("Interface\\newui_repair_00.tga", IMAGE_INVENTORY_REPAIR_BTN, GL_LINEAR);
	LoadBitmap("Interface\\newui_Bt_openshop.tga", IMAGE_INVENTORY_MYSHOP_OPEN_BTN, GL_LINEAR);
	LoadBitmap("Interface\\newui_Bt_closeshop.tga", IMAGE_INVENTORY_MYSHOP_CLOSE_BTN, GL_LINEAR);
}

void SEASON3B::CNewUIMyInventory::UnloadImages()
{
	DeleteBitmap(IMAGE_INVENTORY_MYSHOP_CLOSE_BTN);
	DeleteBitmap(IMAGE_INVENTORY_MYSHOP_OPEN_BTN);
	DeleteBitmap(IMAGE_INVENTORY_REPAIR_BTN);
	DeleteBitmap(IMAGE_INVENTORY_EXIT_BTN);
	DeleteBitmap(IMAGE_INVENTORY_MONEY);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_NECKLACE);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_RING);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_PANTS);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_GLOVES);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_ARMOR);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_RIGHT);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_LEFT);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_WING);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_FAIRY);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_HELM);
	DeleteBitmap(IMAGE_INVENTORY_ITEM_BOOT);
	DeleteBitmap(IMAGE_INVENTORY_BACK_BOTTOM);
	DeleteBitmap(IMAGE_INVENTORY_BACK_RIGHT);
	DeleteBitmap(IMAGE_INVENTORY_BACK_LEFT);
	DeleteBitmap(IMAGE_INVENTORY_BACK_TOP2);
	DeleteBitmap(IMAGE_INVENTORY_BACK_TOP);
	DeleteBitmap(IMAGE_INVENTORY_BACK);
}

void SEASON3B::CNewUIMyInventory::RenderFrame()
{
#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_RENDER_NEWUIINVENTORY_RENDERFRAME, PROFILING_RENDER_NEWUIINVENTORY_RENDERFRAME );
#endif // DO_PROFILING
	RenderImage(IMAGE_INVENTORY_BACK, m_Pos.x, m_Pos.y, 190.f, 429.f);
	RenderImage(IMAGE_INVENTORY_BACK_TOP2, m_Pos.x, m_Pos.y, 190.f, 64.f);
	RenderImage(IMAGE_INVENTORY_BACK_LEFT, m_Pos.x, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_INVENTORY_BACK_RIGHT, m_Pos.x+INVENTORY_WIDTH-21, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_INVENTORY_BACK_BOTTOM, m_Pos.x, m_Pos.y+INVENTORY_HEIGHT-45, 190.f, 45.f);
#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_RENDER_NEWUIINVENTORY_RENDERFRAME );
#endif // DO_PROFILING
}

void SEASON3B::CNewUIMyInventory::RenderEquippedItem()
{
#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_RENDER_NEWUIINVENTORY_RENDEREQUIPEDITEM, PROFILING_RENDER_NEWUIINVENTORY_RENDEREQUIPEDITEM );
#endif // DO_PROFILING

	for(int i=0; i<MAX_EQUIPMENT_INDEX; i++)
	{
		if(i == EQUIPMENT_HELM)
		{
			if(GetBaseClass(Hero->Class) == CLASS_DARK)
			{
				// 마검사는 헬맷 공간 안그림
				continue;
			}
		}
#ifdef PBG_ADD_NEWCHAR_MONK
		// 몽크일 경우 장갑을 그리지 않는다
		if((i == EQUIPMENT_GLOVES) && (GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER))
			continue;	
#endif //PBG_ADD_NEWCHAR_MONK

		EnableAlphaTest();
		
		//. 스롯 배경 그리기
		RenderImage(m_EquipmentSlots[i].dwBgImage, m_EquipmentSlots[i].x, m_EquipmentSlots[i].y, 
			m_EquipmentSlots[i].width, m_EquipmentSlots[i].height);
		DisableAlphaBlend();

		ITEM* pEquipmentItemSlot = &CharacterMachine->Equipment[i];
		if(pEquipmentItemSlot->Type != -1)
		{
			ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pEquipmentItemSlot->Type];
			int iLevel = (pEquipmentItemSlot->Level>>3)&15;
			int iMaxDurability = calcMaxDurability(pEquipmentItemSlot, pItemAttr, iLevel);
			
			// 용사/전사의반지 예외처리
			if( i == EQUIPMENT_RING_LEFT || i == EQUIPMENT_RING_RIGHT)
			{
				if( pEquipmentItemSlot->Type == ITEM_HELPER+20 && iLevel == 1 
					|| iLevel == 2 )
				{
					continue;
				}
			}
			
#ifdef KJH_FIX_RENDER_PERIODITEM_DURABILITY
			// 기간제 아이템이고 기간이 끝나지 않았을때 색 변화가 없다.
			if( (pEquipmentItemSlot->bPeriodItem == true) && (pEquipmentItemSlot->bExpiredPeriod == false) )
				continue;
#endif // KJH_FIX_RENDER_PERIODITEM_DURABILITY
			
			//. 내구도에 따른 색의 변화
			if(pEquipmentItemSlot->Durability <= 0)
				glColor4f(1.f,0.f,0.f,0.25f);	//  내구력 경고. 100%
			else if(pEquipmentItemSlot->Durability<=(iMaxDurability*0.2f))
				glColor4f(1.f,0.15f,0.f,0.25f);	//  내구력 경고. 80%
			else if(pEquipmentItemSlot->Durability<=(iMaxDurability*0.3f))
				glColor4f(1.f,0.5f,0.f,0.25f);	//  내구력 경고. 70%
			else if(pEquipmentItemSlot->Durability<=(iMaxDurability*0.5f))
				glColor4f(1.f,1.f,0.f,0.25f);	//  내구력 경고. 50%
			else if(IsEquipable(i, pEquipmentItemSlot) == false)
				glColor4f(1.f,0.f,0.f,0.25f);
			else
			{
				continue;	//. 그리지 않는다.
			}
			
			EnableAlphaTest();
			RenderColor(m_EquipmentSlots[i].x+1, m_EquipmentSlots[i].y, m_EquipmentSlots[i].width-4, m_EquipmentSlots[i].height-4);
			EndRenderColor();
		}
	}

	if(CNewUIInventoryCtrl::GetPickedItem() && m_iPointedSlot != -1)
	{
		ITEM* pItemObj = CNewUIInventoryCtrl::GetPickedItem()->GetItem();
		ITEM* pEquipmentItemSlot = &CharacterMachine->Equipment[m_iPointedSlot];
		if( pItemObj && (pEquipmentItemSlot->Type != -1 || false == IsEquipable(m_iPointedSlot, pItemObj)) 
#ifdef PBG_ADD_NEWCHAR_MONK
			&& !((GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER) && (m_iPointedSlot == EQUIPMENT_GLOVES))			
#endif //PBG_ADD_NEWCHAR_MONK
			)
		{	//. 장착할 수 없는 아이템 빨간색 표시
			glColor4f(0.9f, 0.1f, 0.1f, 0.4f);
			EnableAlphaTest();
			RenderColor(m_EquipmentSlots[m_iPointedSlot].x+1, m_EquipmentSlots[m_iPointedSlot].y, 
				m_EquipmentSlots[m_iPointedSlot].width-4, m_EquipmentSlots[m_iPointedSlot].height-4);
			EndRenderColor();
		}
	}


	//. 툴팁 그리기
	if(m_iPointedSlot != -1 && m_pNewUI3DRenderMng)
	{	
		m_pNewUI3DRenderMng->RenderUI2DEffect(INVENTORY_CAMERA_Z_ORDER, UI2DEffectCallback, this, m_iPointedSlot, 0);
	}

#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_RENDER_NEWUIINVENTORY_RENDEREQUIPEDITEM );
#endif // DO_PROFILING
}

void SEASON3B::CNewUIMyInventory::RenderButtons()
{
#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_RENDER_NEWUIINVENTORY_RENDERBUTTONS, PROFILING_RENDER_NEWUIINVENTORY_RENDERBUTTONS );
#endif // DO_PROFILING
	EnableAlphaTest();

	if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP)==false
		&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_TRADE)==false 
		&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_DEVILSQUARE)==false
		&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_BLOODCASTLE)==false
		&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MIXINVENTORY)==false
#ifdef LEM_ADD_LUCKYITEM
		&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND)==false
#endif // LEM_ADD_LUCKYITEM
		&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE)==false)
	{
		if(m_bRepairEnableLevel == true)
		{
			m_BtnRepair.Render();
		}
		if(m_bMyShopOpen == true)
		{
			m_BtnMyShop.Render();
		}
	}
	m_BtnExit.Render();

	DisableAlphaBlend();
#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_RENDER_NEWUIINVENTORY_RENDERBUTTONS );
#endif // DO_PROFILING
}

void SEASON3B::CNewUIMyInventory::RenderInventoryDetails()
{
#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_RENDER_NEWUIINVENTORY_RENDERINVENTORYDETAILS, PROFILING_RENDER_NEWUIINVENTORY_RENDERINVENTORYDETAILS );
#endif // DO_PROFILING
	EnableAlphaTest();

	//. 캡션
	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetBgColor(0);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->RenderText(m_Pos.x,m_Pos.y+12,GlobalText[223], INVENTORY_WIDTH, 0, RT3_SORT_CENTER);

	//. 젠
	RenderImage(IMAGE_INVENTORY_MONEY, m_Pos.x+11, m_Pos.y+364, 170.f, 26.f);
#ifndef KJH_DEL_PC_ROOM_SYSTEM			// #ifndef
#ifdef ADD_PCROOM_POINT_SYSTEM
	CPCRoomPtSys& rPCRoomPtSys = CPCRoomPtSys::Instance();
	if(rPCRoomPtSys.IsPCRoomPointShopMode())
	{
		// PC방 포인트 상점일 경우 젠이 아니라 포인트로 표시 됨.
		g_pRenderText->SetBgColor(40, 40, 40, 255);
		g_pRenderText->SetTextColor(255, 220, 250, 255);
		g_pRenderText->RenderText((int)m_Pos.x+50, (int)m_Pos.y+371, GlobalText[2326]);
		
		unicode::t_char Text[256] = { 0, };
		unicode::_sprintf(Text, "%d", rPCRoomPtSys.GetNowPoint());
		g_pRenderText->RenderText((int)m_Pos.x+82, (int)m_Pos.y+371, Text);
	}
	else
#endif	// ADD_PCROOM_POINT_SYSTEM
#endif // KJH_DEL_PC_ROOM_SYSTEM
	{
		DWORD dwZen = CharacterMachine->Gold;

		unicode::t_char Text[256] = { 0, };
		ConvertGold(dwZen, Text);

		//  젠의 색 단위별 변화. ( 녹색 -> 주황색 -> 빨간색 ).
		g_pRenderText->SetTextColor(getGoldColor(dwZen));
		g_pRenderText->RenderText((int)m_Pos.x+50,(int)m_Pos.y+371, Text);
	}
	g_pRenderText->SetFont(g_hFont);
	
	DisableAlphaBlend();
#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_RENDER_NEWUIINVENTORY_RENDERINVENTORYDETAILS );
#endif // DO_PROFILING
}

bool SEASON3B::CNewUIMyInventory::EquipmentWindowProcess()
{
	//. 외부에서 장비창으로
#ifdef KJH_FIX_WOPS_K27261_DISAPPEAR_EQUIPED_ITEM
	if(m_iPointedSlot != -1 && IsRelease(VK_LBUTTON))
#else // KJH_FIX_WOPS_K27261_DISAPPEAR_EQUIPED_ITEM
	if(m_iPointedSlot != -1 && IsPress(VK_LBUTTON))
#endif // KJH_FIX_WOPS_K27261_DISAPPEAR_EQUIPED_ITEM
	{
		CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
		if(pPickedItem)
		{
			ITEM* pItemObj = pPickedItem->GetItem();

#ifdef LDK_FIX_EXPIREDPERIOD_ITEM_EQUIP_WARNING
			// 만료된 아이템 작용불가 및 경고 
			if( pItemObj->bPeriodItem && pItemObj->bExpiredPeriod)
			{
				// 2285 "착용할 수 없습니다."
				g_pChatListBox->AddText("", GlobalText[2285], SEASON3B::TYPE_ERROR_MESSAGE);
				CNewUIInventoryCtrl::BackupPickedItem();
				
				ResetMouseLButton();
				return false;
			}
#endif //LDK_FIX_EXPIREDPERIOD_ITEM_EQUIP_WARNING
			
			// 장비창에 장비아이템이 들어가 있으면 겹치기 처리 안되게, 서버에게 패킷 안보내는 처리
			ITEM* pEquipmentItemSlot = &CharacterMachine->Equipment[m_iPointedSlot];
			if(pEquipmentItemSlot && pEquipmentItemSlot->Type != -1)
			{
				return true;
			}

#ifdef YDG_MOD_CHANGE_RING_EQUIPMENT_LIMIT
			// 변신반지 중복착용 금지
			if (g_ChangeRingMgr->CheckChangeRing(pPickedItem->GetItem()->Type))
			{
				ITEM * pItemRingLeft = &CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];
				ITEM * pItemRingRight = &CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];

				if (g_ChangeRingMgr->CheckChangeRing(pItemRingLeft->Type) || g_ChangeRingMgr->CheckChangeRing(pItemRingRight->Type))
				{
					// 3089 "다른 변신반지와 동시에 착용할 수 없습니다."
					g_pChatListBox->AddText("", GlobalText[3089], SEASON3B::TYPE_ERROR_MESSAGE);
					CNewUIInventoryCtrl::BackupPickedItem();
					
					ResetMouseLButton();
					return false;
				}
			}
#endif	// YDG_MOD_CHANGE_RING_EQUIPMENT_LIMIT

			if(IsEquipable(m_iPointedSlot, pItemObj))
			{
				if(pPickedItem->GetOwnerInventory() == m_pNewInventoryCtrl)
				{	//. 인벤토리에서 장비창
					int iSourceIndex = pPickedItem->GetSourceLinealPos();
					int iTargetIndex = m_iPointedSlot;
					SendRequestEquipmentItem(REQUEST_EQUIPMENT_INVENTORY, MAX_EQUIPMENT_INDEX+iSourceIndex, 
						pItemObj, REQUEST_EQUIPMENT_INVENTORY, iTargetIndex);
					
					return true;
				}
				else if(pPickedItem->GetOwnerInventory() == g_pMixInventory->GetInventoryCtrl())
				{	// 조합창에서 장비창
					int iSourceIndex = pPickedItem->GetSourceLinealPos();
					int iTargetIndex = m_iPointedSlot;

					SendRequestEquipmentItem(g_MixRecipeMgr.GetMixInventoryEquipmentIndex(), iSourceIndex, 
						pItemObj, REQUEST_EQUIPMENT_INVENTORY, iTargetIndex);
					
					return true;
				}
				// 창고에서 장비창으로.
				else if(pPickedItem->GetOwnerInventory() == g_pStorageInventory->GetInventoryCtrl())
				{
					int iSourceIndex = pPickedItem->GetSourceLinealPos();
					int iTargetIndex = m_iPointedSlot;

					g_pStorageInventory->SendRequestItemToMyInven(
						pItemObj, iSourceIndex, iTargetIndex);

					return true;
				}
				// 거래창에서 장비창으로.
				else if(pPickedItem->GetOwnerInventory() == g_pTrade->GetMyInvenCtrl())
				{
					int iSourceIndex = pPickedItem->GetSourceLinealPos();
					int iTargetIndex = m_iPointedSlot;

					g_pTrade->SendRequestItemToMyInven(
						pItemObj, iSourceIndex, iTargetIndex);

					return true;
				}
				// 개인상점에서 장비창으로
				else if(pPickedItem->GetOwnerInventory() == g_pMyShopInventory->GetInventoryCtrl())
				{
					int iSourceIndex = pPickedItem->GetSourceLinealPos();
					int iTargetIndex = m_iPointedSlot;

					SendRequestEquipmentItem(REQUEST_EQUIPMENT_MYSHOP, MAX_MY_INVENTORY_INDEX+iSourceIndex, pItemObj, 
						REQUEST_EQUIPMENT_INVENTORY, iTargetIndex);

					return true;
				}
				/* CNewUIInventoryCtrl를 사용하지 않는 예외 */
				/* ITEM::ex_src_type 값으로 구분한다. (ITEM구조체 참조) */
 				else if(pItemObj->ex_src_type == ITEM_EX_SRC_EQUIPMENT && EquipmentItem == false)
				{	//. 장비창에서 장비창
					if(pPickedItem->GetSourceLinealPos() == m_iPointedSlot)
					{
						//. 제자리에 놓기
						CNewUIInventoryCtrl::BackupPickedItem();
// 주석 임시추가@@@@@
#if !defined(LDK_FIX_RECALL_CREATEEQUIPPINGEFFECT) && !defined(LDS_FIX_RECALL_CREATEEQUIPPINGEFFECT) 
						//. 장착효과 생성
						CreateEquippingEffect(pItemObj);
#endif //LDK_FIX_RECALL_CREATEEQUIPPINGEFFECT
					}
					else
					{
						int iSourceIndex = pItemObj->lineal_pos;
						int iTargetIndex = m_iPointedSlot;
						SendRequestEquipmentItem(REQUEST_EQUIPMENT_INVENTORY, iSourceIndex, 
							pItemObj, REQUEST_EQUIPMENT_INVENTORY, iTargetIndex);
						
						return true;
					}
				}
			}
		}
		else // pPickedItem == NULL
		{
			if(GetRepairMode() == REPAIR_MODE_ON)
			{
				ITEM* pEquippedItem = &CharacterMachine->Equipment[m_iPointedSlot];
				
				if(pEquippedItem == NULL)
				{
					return true;
				}

				// 수리금지 아이템
				if(IsRepairBan(pEquippedItem) == true)
				{
					return true;
				}


				if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP) && g_pNPCShop->IsRepairShop())
				{
					SendRequestRepair(m_iPointedSlot, 0);
				}
				else if(m_bRepairEnableLevel == true)
				{
					SendRequestRepair(m_iPointedSlot, 1);
				}
				
				return true;
			}
			else
			{
				//. 장비창 아이템을 집는다.
				ITEM* pEquippedItem = &CharacterMachine->Equipment[m_iPointedSlot];
				if(pEquippedItem->Type >= 0)	//. 슬롯에 아이템이 있다면
				{
#ifdef KJH_FIX_WOPS_K26606_TRADE_WING_IN_IKARUS
					// 이카루스일때 예외처리
					if( World == WD_10HEAVEN )		
					{
						ITEM* pEquippedPetItem = &CharacterMachine->Equipment[EQUIPMENT_HELPER];
						bool bPicked = true;

						if( m_iPointedSlot == EQUIPMENT_HELPER || m_iPointedSlot == EQUIPMENT_WING )
						{
							// 장비창에서 펫을 집었을때, 날개가 없으면, 못집는다.
							if( ((m_iPointedSlot == EQUIPMENT_HELPER) && !IsEquipedWing()) )
							{
								bPicked = false;
							}
							// 장비창에서 날개를 집었을때, 펫(디노란트,다크호스,펜릴)이 없으면, 못집는다.
							else if( ((m_iPointedSlot == EQUIPMENT_WING) &&  
								!((pEquippedPetItem->Type == ITEM_HELPER+3)				// 디노란트
								|| (pEquippedPetItem->Type == ITEM_HELPER+4)			// 다크호스
								|| (pEquippedPetItem->Type == ITEM_HELPER+37)))			// 펜릴
								)
							{
								bPicked = false;
							}
						}
						
						if( bPicked == true )
						{
							if(CNewUIInventoryCtrl::CreatePickedItem(NULL, pEquippedItem))
							{	
								UnequipItem(m_iPointedSlot);	//. 장착된 아이템 해제
							}
						}
					}
					else 
					{
						if(CNewUIInventoryCtrl::CreatePickedItem(NULL, pEquippedItem))
						{	
							UnequipItem(m_iPointedSlot);	//. 장착된 아이템 해제
						}
					}
#else // KJH_FIX_WOPS_K26606_TRADE_WING_IN_IKARUS
					if(CNewUIInventoryCtrl::CreatePickedItem(NULL, pEquippedItem))
					{	
						UnequipItem(m_iPointedSlot);	//. 장착된 아이템 해제
					}
#endif // KJH_FIX_WOPS_K26606_TRADE_WING_IN_IKARUS	
				}
			}

		}
	}

	return false;
}

bool SEASON3B::CNewUIMyInventory::InventoryProcess()
{
	if(CheckMouseIn(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT) == false)
	{
		return false;
	}

	//. 외부에서 인벤토리로
	CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
	if(m_pNewInventoryCtrl && pPickedItem 
		&& SEASON3B::IsRelease(VK_LBUTTON)
		//&& m_pNewInventoryCtrl->CanPushItem() == true
		)
	{
		ITEM* pPickItem = pPickedItem->GetItem();
		if(pPickItem == NULL)
		{
			return false;
		}
		if(pPickedItem->GetOwnerInventory() == m_pNewInventoryCtrl)
		{	
			//. 인벤토리에서 인벤토리

			int iSourceIndex = pPickedItem->GetSourceLinealPos();
			int iTargetIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);

			// 보석사용
			if(pPickItem->Type == ITEM_POTION+13
				|| pPickItem->Type == ITEM_POTION+14
				|| pPickItem->Type == ITEM_POTION+16
				|| pPickItem->Type == ITEM_POTION+42
				|| pPickItem->Type == ITEM_POTION+43
				|| pPickItem->Type == ITEM_POTION+44
#ifdef LEM_ADD_LUCKYITEM
				|| pPickItem->Type == ITEM_POTION+160
				|| pPickItem->Type == ITEM_POTION+161
#endif // LEM_ADD_LUCKYITEM
				)
			{
				ITEM* pItem = m_pNewInventoryCtrl->FindItem(iTargetIndex);
				if(pItem)
				{
					int	iType = pItem->Type;
					int	iLevel = (pItem->Level >> 3) & 15;
					int	iDurability = pItem->Durability;
					
					bool bSuccess = true;
					
					// 날개 종류 아이템이 아니면 false
					if(iType > ITEM_WING+6 // 날개들
						&& iType != ITEM_HELPER+30	// 군주의 망토
#ifdef ADD_ALICE_WINGS_1
						&& !(iType >= ITEM_WING+36 && iType <= ITEM_WING+43)	// 폭풍의 날개 ~ 제왕의 망토, 소환술사 날개
#else	// ADD_ALICE_WINGS_1
						&& !( iType >= ITEM_WING+36 && iType <= ITEM_WING+40 )	// 폭풍의 날개 ~ 제왕의 망토
#endif	// ADD_ALICE_WINGS_1
#ifdef LDK_ADD_INGAMESHOP_SMALL_WING			// 기간제 날개 작은(군망, 재날, 요날, 천날, 사날)
						&& !( ITEM_WING+130 <= iType && iType <= ITEM_WING+134 )
#endif //LDK_ADD_INGAMESHOP_SMALL_WING
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
						&& !( iType >= ITEM_WING+49 && iType <= ITEM_WING+50 )	// 레이지파이터 날개
						&& (iType != ITEM_WING+135)	// 작은 무인의 망토
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
						)
					{
						bSuccess = false;
					}
					
					// 화살 종류이면 false
					if(iType==ITEM_BOW+7 
						|| iType==ITEM_BOW+15) //화살
					{
						bSuccess = false;
					}
					// 축석에 레벨 6이상인 경우. ( 0, 1, 2, 3, 4, 5 ) -> ( 1, 2, 3, 4, 5, 6 )
					// 영석에 레벨 9이상인 경우  ( 6, 7, 8 ) -> ( 7, 8, 9 )
					if((pPickItem->Type==ITEM_POTION+13 && iLevel >= 6)    
						|| (pPickItem->Type==ITEM_POTION+14 && iLevel >= 9)) 
					{	    
						bSuccess = false;
					}
					
					//^ 펜릴 수리 관련(메세지 박스)
					if(pPickItem->Type == ITEM_POTION+13 
						&& iType == ITEM_HELPER+37 && iDurability != 255)	// 펜릴의 뿔피리
					{
						SEASON3B::CFenrirRepairMsgBox* pMsgBox = NULL;
						SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CFenrirRepairMsgBoxLayout), &pMsgBox);
						pMsgBox->SetSourceIndex(iSourceIndex);

						int iIndex = pItem->x + pItem->y * m_pNewInventoryCtrl->GetNumberOfColumn();
						pMsgBox->SetTargetIndex(MAX_EQUIPMENT_INDEX + iIndex);
						
						pPickedItem->HidePickedItem();
						return true;
					}
					
					if(pPickItem->Type==ITEM_POTION+42 )
					{
#ifdef SOCKET_SYSTEM
						if (g_SocketItemMgr.IsSocketItem(pItem))
						{
							// 소켓아이템에는 조석을 바를수 없다
							bSuccess = false;
						}
						else
#endif	// SOCKET_SYSTEM
						if(pItem->Jewel_Of_Harmony_Option != 0  )
						{
							//강화된 아이템은 강화를 못한다..
							// 환원을 한 다음 다시 강화...
							bSuccess = false;
						}
						else
						{
							StrengthenItem strengthitem = g_pUIJewelHarmonyinfo->GetItemType( static_cast<int>(pItem->Type) );
							
							if( strengthitem == SI_None )
							{
								//검류, 도끼류, 둔기류, 창류, 활/석궁류, 지팡이, 방어구
								bSuccess = false;
							}
						}
					}
					
					if(pPickItem->Type == ITEM_POTION+43 
						|| pPickItem->Type == ITEM_POTION+44 )
					{
#ifdef SOCKET_SYSTEM
						if (g_SocketItemMgr.IsSocketItem(pItem))
						{
							// 소켓아이템에는 제련석을 바를수 없다
							bSuccess = false;
						}
						else
#endif	// SOCKET_SYSTEM
						if( pItem->Jewel_Of_Harmony_Option == 0 )
						{
							bSuccess = false;
						}
					}

#ifdef LEM_ADD_LUCKYITEM		// 럭키아이템 보석 조합
					if( Check_LuckyItem( pItem->Type ) )
					{
						bSuccess	= false;
						if( pPickItem->Type == ITEM_POTION+161 )
						{
							if(pItem->Jewel_Of_Harmony_Option == 0  )	bSuccess = true;
						}
						else if( pPickItem->Type == ITEM_POTION+160 )
						{
							if( pItem->Durability > 0)					bSuccess = true;
						}
					}
#endif // LEM_ADD_LUCKYITEM
					
					if(bSuccess)
					{		
						// TargetItem의 기준 인덱스를 구한다. (서버에 보내는 Index)
						int iTargetBaseIndex = m_pNewInventoryCtrl->FindBaseIndexByITEM(pItem);
						// 들고 있는 보석의 위치, 보석을 조합할 아이템의 위치
						SendRequestUse(iSourceIndex, MAX_EQUIPMENT_INDEX + iTargetBaseIndex);

						PlayBuffer(SOUND_GET_ITEM01);
						return true;
					}
				}
			}
			// 아이템 겹치기
			else if(iTargetIndex != -1)
			{
				ITEM* pItem = m_pNewInventoryCtrl->FindItem(iTargetIndex);
				if(pItem)
				{
					bool bOverlay = m_pNewInventoryCtrl->IsOverlayItem(pPickItem, pItem);

					if(bOverlay)
					{
						SendRequestEquipmentItem(REQUEST_EQUIPMENT_INVENTORY, MAX_EQUIPMENT_INDEX+iSourceIndex,
							pPickItem, REQUEST_EQUIPMENT_INVENTORY, MAX_EQUIPMENT_INDEX+iTargetIndex);
					}
				}
			}
			
			if(iTargetIndex != -1 && m_pNewInventoryCtrl->CanMove(iTargetIndex, pPickItem))
			{
				if(pPickedItem->GetSourceLinealPos() != iTargetIndex)
				{
					SendRequestEquipmentItem(REQUEST_EQUIPMENT_INVENTORY, MAX_EQUIPMENT_INDEX+iSourceIndex, 
						pPickItem, REQUEST_EQUIPMENT_INVENTORY, MAX_EQUIPMENT_INDEX+iTargetIndex);
				}
				else
				{
					SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
				}
				
				return true;
			}
		}
		// 창고에서 인벤토리로.
		else if(pPickedItem->GetOwnerInventory() == g_pStorageInventory->GetInventoryCtrl())
		{
			int iSourceIndex = pPickedItem->GetSourceLinealPos();
			int iTargetIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);
			if (iTargetIndex != -1 && m_pNewInventoryCtrl->CanMove(iTargetIndex, pPickItem))
			{
				g_pStorageInventory->SendRequestItemToMyInven(
					pPickItem, iSourceIndex, MAX_EQUIPMENT_INDEX+iTargetIndex);

				return true;
			}
		}

		// 거래창에서 인벤토리로.
		else if(pPickedItem->GetOwnerInventory() == g_pTrade->GetMyInvenCtrl())
		{
			int iSourceIndex = pPickedItem->GetSourceLinealPos();
			int iTargetIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);
			if (iTargetIndex != -1 && m_pNewInventoryCtrl->CanMove(iTargetIndex, pPickItem))
			{
				g_pTrade->SendRequestItemToMyInven(
					pPickItem, iSourceIndex, MAX_EQUIPMENT_INDEX+iTargetIndex);

				return true;
			}
		}
#ifdef LEM_ADD_LUCKYITEM
		else if(pPickedItem->GetOwnerInventory() == g_pLuckyItemWnd->GetInventoryCtrl())
		{	//. 럭키 조합창에서 인벤토리
			int iSourceIndex = pPickedItem->GetSourceLinealPos();
			int iTargetIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);
			if(iTargetIndex != -1 && m_pNewInventoryCtrl->CanMove(iTargetIndex, pPickItem))
			{
				SendRequestEquipmentItem( g_pLuckyItemWnd->SetWndAction( eLuckyItem_Move ), iSourceIndex, 
					pPickItem, REQUEST_EQUIPMENT_INVENTORY, MAX_EQUIPMENT_INDEX+iTargetIndex);
				return true;
			}
		}
#endif // LEM_ADD_LUCKYITEM
		else if(pPickedItem->GetOwnerInventory() == g_pMixInventory->GetInventoryCtrl())
		{	//. 조합창에서 인벤토리
			int iSourceIndex = pPickedItem->GetSourceLinealPos();
			int iTargetIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);
			if(iTargetIndex != -1 && m_pNewInventoryCtrl->CanMove(iTargetIndex, pPickItem))
			{
				SendRequestEquipmentItem(g_MixRecipeMgr.GetMixInventoryEquipmentIndex(), iSourceIndex, 
					pPickItem, REQUEST_EQUIPMENT_INVENTORY, MAX_EQUIPMENT_INDEX+iTargetIndex);
				return true;
			}
		}
		else if(pPickedItem->GetOwnerInventory() == g_pMyShopInventory->GetInventoryCtrl())
		{
			int iSourceIndex = pPickedItem->GetSourceLinealPos();
			int iTargetIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);

			if(iTargetIndex != -1 && m_pNewInventoryCtrl->CanMove(iTargetIndex, pPickItem))
			{
				SendRequestEquipmentItem(REQUEST_EQUIPMENT_MYSHOP, MAX_MY_INVENTORY_INDEX+iSourceIndex, 
					pPickItem, REQUEST_EQUIPMENT_INVENTORY, MAX_EQUIPMENT_INDEX+iTargetIndex);
				return true;
			}
		}
		/* CNewUIInventoryCtrl를 사용하지 않는 예외 */
		/* ITEM::ex_src_type 값으로 구분한다. (ITEM구조체 참조) */
		else if(pPickItem->ex_src_type == ITEM_EX_SRC_EQUIPMENT)
		{	//. 장비창에서 인벤토리	(장비창은 CNewUIInventoryCtrl를 사용 하지 않으므로)
			int iSourceIndex = pPickedItem->GetSourceLinealPos();	// return ITEM::lineal_pos;
			int iTargetIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);
			if(iTargetIndex != -1 && m_pNewInventoryCtrl->CanMove(iTargetIndex, pPickItem))
			{
				SendRequestEquipmentItem(REQUEST_EQUIPMENT_INVENTORY, iSourceIndex, 
					pPickItem, REQUEST_EQUIPMENT_INVENTORY, MAX_EQUIPMENT_INDEX+iTargetIndex);	
				return true;
			}
		}
	}
	else if(m_pNewInventoryCtrl && GetRepairMode() == REPAIR_MODE_OFF && 
			SEASON3B::IsPress(VK_RBUTTON)
		#ifdef LEM_FIX_USER_LOGOUT
			 && g_bExit == false 
		#endif	// LEM_FIX_USER_LOGOUT [lem_2010.8.18]
			)
	{
		ResetMouseRButton();

		//. 창고
		if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE))
		{
			g_pStorageInventory->ProcessMyInvenItemAutoMove();
		}
		//. 아이템 사용
		else if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY)
			&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP)==false
			&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_TRADE)==false
			&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_DEVILSQUARE)==false
			&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_BLOODCASTLE)==false
#ifdef LEM_ADD_LUCKYITEM
			&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND)==false
#endif // LEM_ADD_LUCKYITEM
			&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MIXINVENTORY)==false)
		{
			ITEM* pItem = m_pNewInventoryCtrl->FindItemAtPt(MouseX, MouseY);
			if(pItem == NULL)
			{
				return false;
			}

			int iIndex = pItem->x+pItem->y*m_pNewInventoryCtrl->GetNumberOfColumn();
			if(pItem->Type == ITEM_POTION+10)//마을귀환문서
			{
				if ( !Teleport)	// 순간이동 중에는 못 쓰게 한다.
				{	
					SendRequestUse(iIndex, 0);
					return true;
				}
			}

			//////////////////////////////////////////////////////////////////////////
			//. 일반 아이템 사용
			if( (pItem->Type >= ITEM_POTION+0 && pItem->Type <= ITEM_POTION+9)	//. 물약 ~ 술
			|| (pItem->Type==ITEM_POTION+20 && ((pItem->Level>>3)&15) == 0)	// 사랑의 묘약
			|| (pItem->Type>=ITEM_POTION+35 && pItem->Type <=ITEM_POTION+40)
			||( pItem->Type>=ITEM_POTION+46 && pItem->Type<=ITEM_POTION+50 )//할로윈 물약
#ifdef NEW_YEAR_BAG_EVENT
			|| ( pItem->Type==ITEM_POTION+11 && ((pItem->Level>>3)&15)==14 )
#endif// NEW_YEAR_BAG_EVENT
#ifdef PSW_ELITE_ITEM              // 엘리트 물약
			|| ( pItem->Type>=ITEM_POTION+70 &&pItem->Type<=ITEM_POTION+71 )
#endif //PSW_ELITE_ITEM
#ifdef PSW_SCROLL_ITEM             // 엘리트 스크롤
			|| ( pItem->Type>=ITEM_POTION+72 &&pItem->Type<=ITEM_POTION+77 )
#endif //PSW_SCROLL_ITEM
#ifdef PSW_INDULGENCE_ITEM         // 면죄부
			|| pItem->Type==ITEM_HELPER+60
#endif //PSW_INDULGENCE_ITEM
#ifdef PSW_NEW_ELITE_ITEM              // 엘리트 중간 치료 물약
			|| pItem->Type==ITEM_POTION+94
#endif //PSW_NEW_ELITE_ITEM
#ifdef CSK_EVENT_CHERRYBLOSSOM
			|| ( pItem->Type>=ITEM_POTION+85 && pItem->Type<=ITEM_POTION+87 )
#endif //CSK_EVENT_CHERRYBLOSSOM		
#ifdef PSW_ADD_PC4_SCROLLITEM
			|| ( pItem->Type>=ITEM_POTION+97 &&pItem->Type<=ITEM_POTION+98 )
#endif //PSW_ADD_PC4_SCROLLITEM
#ifdef ASG_ADD_CS6_GUARD_CHARM
			|| pItem->Type == ITEM_HELPER+81	// 수호의부적
#endif	// ASG_ADD_CS6_GUARD_CHARM
#ifdef ASG_ADD_CS6_ITEM_GUARD_CHARM
			|| pItem->Type == ITEM_HELPER+82	// 아이템보호부적
#endif	// ASG_ADD_CS6_ITEM_GUARD_CHARM
#ifdef YDG_ADD_CS7_ELITE_SD_POTION
			|| pItem->Type == ITEM_POTION+133	// 엘리트SD회복물약
#endif	// YDG_ADD_CS7_ELITE_SD_POTION
#ifdef PBG_MOD_SECRETITEM
			|| (pItem->Type >= ITEM_HELPER+117 && pItem->Type <= ITEM_HELPER+120)	// 활력의비약
#endif //PBG_MOD_SECRETITEM
#ifdef ASG_ADD_TIME_LIMIT_QUEST_ITEM
			|| pItem->Type == ITEM_POTION+151	// 1레벨 의뢰서
#endif	// ASG_ADD_TIME_LIMIT_QUEST_ITEM
			)
			{
#ifdef CSK_FIX_BLUELUCKYBAG_MOVECOMMAND
				// 파란복주머니
				if(pItem->Type == ITEM_POTION+11 && (pItem->Level >> 3) == 14)
				{
					g_pBlueLuckyBagEvent->StartBlueLuckyBag();
				}
#endif // CSK_FIX_BLUELUCKYBAG_MOVECOMMAND
#ifdef PBG_MOD_SECRETITEM
				if(pItem->Type >= ITEM_HELPER+117 && pItem->Type <= ITEM_HELPER+120)
				{
					BuffScriptLoader& pBuffInfo = TheBuffInfo();
					int iBuffType = pBuffInfo.GetBuffType(pItem->Type);
					unicode::t_char szBuffName[MAX_TEXT_LENGTH] = {0, };

					if(Hero->Object.m_BuffMap.IsEqualBuffType(iBuffType, szBuffName))
					{
						SEASON3B::CBuffUseMsgBox* pMsgBox = NULL;
						SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CBuffUseOverlapMsgBoxLayout), &pMsgBox);

						if(pMsgBox != NULL)
						{
							unicode::t_char strText[MAX_TEXT_LENGTH]={0,};
							unicode::t_string strText_1, strText_2;
							unicode::_sprintf(strText, GlobalText[3047], ItemAttribute[pItem->Type].Name, szBuffName, ItemAttribute[pItem->Type].Name);

							strText_1 = strText_2 = strText;
							strText_1 = strText_1.substr(0, strText_1.rfind('#')-1);
							strText_2 = strText_2.substr(strText_2.rfind('#')+1);
							strText_1 += strText_2;
							pMsgBox->AddMsg(strText_1);
							pMsgBox->SetInvenIndex(iIndex);
						}
						return true;
					}
				}
#endif //PBG_MOD_SECRETITEM
				SendRequestUse(iIndex, 0);
				return true;
			}
#ifdef PSW_SECRET_ITEM             // 강화의 비약
			else if( ( pItem->Type>=ITEM_POTION+78 && pItem->Type<=ITEM_POTION+82 ) )
			{
				std::list<eBuffState> secretPotionbufflist;
				secretPotionbufflist.push_back( eBuff_SecretPotion1 );
				secretPotionbufflist.push_back( eBuff_SecretPotion2 );
				secretPotionbufflist.push_back( eBuff_SecretPotion3 );
				secretPotionbufflist.push_back( eBuff_SecretPotion4 );
				secretPotionbufflist.push_back( eBuff_SecretPotion5 );

				if( g_isCharacterBufflist( (&Hero->Object), secretPotionbufflist ) == eBuffNone ) {
					SendRequestUse(iIndex, 0);
					return true;
				}
				else {
					SEASON3B::CreateOkMessageBox(GlobalText[2530], RGBA(255, 30, 0, 255) );
				}
			}
#endif //PSW_SECRET_ITEM
#ifdef PSW_FRUIT_ITEM
			else if( ( pItem->Type>=ITEM_HELPER+54 && pItem->Type<=ITEM_HELPER+57 )
				|| ( pItem->Type==ITEM_HELPER+58 && GetBaseClass( Hero->Class )==CLASS_DARK_LORD ) )
			{
				bool result = true;
				WORD point[5] = { 0, };

				point[0] = CharacterAttribute->Strength + CharacterAttribute->AddStrength;   // 힘
				point[1] = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity; // 민첩
				point[2] = CharacterAttribute->Vitality   + CharacterAttribute->AddVitality; // 체력
				point[3] = CharacterAttribute->Energy   + CharacterAttribute->AddEnergy;     // 에너지
				point[4] = CharacterAttribute->Charisma   + CharacterAttribute->AddCharisma; // 통찰
#ifdef PBG_FIX_RESETFRUIT_CAL
#ifdef PBG_ADD_NEWCHAR_MONK
				char nStat[MAX_CLASS][5] =
#else //PBG_ADD_NEWCHAR_MONK
				char nStat[6][5] =
#endif //PBG_ADD_NEWCHAR_MONK
				{
					18, 18, 15, 30,	0,	// 흑마법사.
					28, 20, 25, 10,	0,	// 흑기사.
					22, 25, 20, 15,	0,	// 요정.
					26, 26, 26, 26,	0,	// 마검사.
					26, 20, 20, 15, 25,	// 다크로드.
					21, 21, 18, 23,	0,	// 소환술사.
#ifdef PBG_ADD_NEWCHAR_MONK
					32, 27, 25, 20, 0,  // 레이지파이터
#endif //PBG_ADD_NEWCHAR_MONK
				};
				// 기본 스탯까지 계산하여 1000이상의 여유분이 있어야 한다
				point[pItem->Type-(ITEM_HELPER+54)] -= nStat[GetBaseClass(Hero->Class)][pItem->Type-(ITEM_HELPER+54)];
#endif //PBG_FIX_RESETFRUIT_CAL

				if(point[pItem->Type-(ITEM_HELPER+54)] < (pItem->Durability*10) ) 
				{
					result = false;
				}
				
				if( result == false ) {
					SetStandbyItemKey(pItem->Key);
					SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CUsePartChargeFruitMsgBoxLayout));
				}
				else {
					SendRequestUse(iIndex, 0);
					return true;
				}
			}
#ifdef LDS_ADD_NOTICEBOX_STATECOMMAND_ONLYUSEDARKLORD	// 인게임샾 아이템 // 리셋열매5종 // 통솔리셋열매는 다크로드만 사용가능한 메세지 출력.
			else if( pItem->Type==ITEM_HELPER+58 && GetBaseClass( Hero->Class )!=CLASS_DARK_LORD )
			{
				// 1905 "다크로드만 사용할 수 있습니다."
				SEASON3B::CreateOkMessageBox(GlobalText[1905]);
				return true;				
			}
#endif // LDS_ADD_NOTICEBOX_STATECOMMAND_ONLYUSEDARKLORD
#endif //PSW_FRUIT_ITEM
			//////////////////////////////////////////////////////////////////////////
			//. 특수 아이템 사용
			else if ( pItem->Type==ITEM_HELPER+29 ) //  근위병의 갑옷세트.
			{
#ifdef LDS_ADD_UI_UNITEDMARKETPLACE
				if( true == IsUnitedMarketPlace() )
				{
					// 3014	"로랜시장에서는 카오스 캐슬에"
					// 3015	"입장 할 수 없습니다."
					char	szOutputText[512];
					sprintf( szOutputText, "%s %s", GlobalText[3014], GlobalText[3015] );

					SEASON3B::CreateOkMessageBox(szOutputText);
					return true;
				}
#endif // LDS_ADD_UI_UNITEDMARKETPLACE

				if(Hero->SafeZone == false)
				{
					// 2330 "안전지대에서만 사용 가능합니다."
					SEASON3B::CreateOkMessageBox(GlobalText[2330]);
				}
				else
				{
					SendRequestEventZoneOpenTime (4, ((pItem->Level>>3)&15));
					SetStandbyItemKey(pItem->Key);	//. 요청 대기상태 아이템 설정
					return true;
				}
			}
#ifdef CSK_FREE_TICKET
			else if(pItem->Type == ITEM_HELPER+46)	// 데빌스퀘어 자유입장권	
			{
				BYTE byPossibleLevel = CaculateFreeTicketLevel(FREETICKET_TYPE_DEVILSQUARE);
				SendRequestEventZoneOpenTime(1, byPossibleLevel);
			}
			else if(pItem->Type == ITEM_HELPER+47)	// 블러드캐슬 자유입장권
			{
				BYTE byPossibleLevel = CaculateFreeTicketLevel(FREETICKET_TYPE_BLOODCASTLE);
				SendRequestEventZoneOpenTime(2, byPossibleLevel);
			}
			else if(pItem->Type == ITEM_HELPER+48)	// 칼리마 자유입장권
			{
				if(Hero->SafeZone || InHellas()) 
				{
					// 1238 "안전지대에서 사용할 수 없습니다"
					g_pChatListBox->AddText("", GlobalText[1238], SEASON3B::TYPE_ERROR_MESSAGE);
				}
				else 
				{
					SendRequestUse(iIndex, 0);
				}
			}
#endif // CSK_FREE_TICKET
#ifdef PSW_CURSEDTEMPLE_FREE_TICKET
			else if(pItem->Type == ITEM_HELPER+61)	// 환영의사원 자유입장권	
			{
				BYTE byPossibleLevel = CaculateFreeTicketLevel(FREETICKET_TYPE_CURSEDTEMPLE);
				SendRequestEventZoneOpenTime(5, byPossibleLevel);
			}
#endif //PSW_CURSEDTEMPLE_FREE_TICKET			
#ifdef LDS_FIX_INGAMESHOPITEM_PASSCHAOSCASTLE_REQUEST	// 카오스케슬 자유입장권 // 근위병 갑옷 세트와 동일
			else if(pItem->Type == ITEM_HELPER+121)
			{
				if(Hero->SafeZone == false)
				{
					// 2330 "안전지대에서만 사용 가능합니다."
					SEASON3B::CreateOkMessageBox(GlobalText[2330]);
				}
				else
				{
					SendRequestEventZoneOpenTime (4, ((pItem->Level>>3)&15));
					SetStandbyItemKey(pItem->Key);	//. 요청 대기상태 아이템 설정
					return true;
				}
			}
#endif // LDS_FIX_INGAMESHOPITEM_PASSCHAOSCASTLE_REQUEST
			else if( pItem->Type==ITEM_HELPER+51 ) //환영의 사원
			{
				SendRequestEventZoneOpenTime(5, ((pItem->Level>>3)&15));
				return true;
			}
			else if(pItem->Type==ITEM_POTION+19)//악마의 광장 초대권
			{
				SendRequestEventZoneOpenTime(1, ((pItem->Level>>3)&15));
				return true;
			}
			else if(pItem->Type==ITEM_HELPER+18)//블러드캐슬 투명망또
			{
#ifdef LJH_FIX_REARRANGE_INVISIBLE_CLOAK_LEVEL_FOR_CHECKING_REMAINING_TIME
				// 투명망토는 1~8레벨의 아이템만 사용 가능 함
				if (pItem->Level == 0)
				{
					// 잘못된 아이템 입니다. 
					g_pChatListBox->AddText("", GlobalText[2089], SEASON3B::TYPE_ERROR_MESSAGE);
				}
				else
				{
					SendRequestEventZoneOpenTime(2, ((pItem->Level>>3)&15)-1);
				}
#else  //LJH_FIX_REARRANGE_INVISIBLE_CLOAK_LEVEL_FOR_CHECKING_REMAINING_TIME
				SendRequestEventZoneOpenTime(2, ((pItem->Level>>3)&15));
#endif //LJH_FIX_REARRANGE_INVISIBLE_CLOAK_LEVEL_FOR_CHECKING_REMAINING_TIME
				return true;
			}
			else if( (pItem->Type >= ITEM_ETC+0 && pItem->Type < ITEM_ETC+MAX_ITEM_INDEX)//마법책
				|| (pItem->Type >= ITEM_WING+7 && pItem->Type <= ITEM_WING+14) // 구슬
				|| (pItem->Type >= ITEM_WING+16 && pItem->Type <= ITEM_WING+19)	// 구슬 
				|| (pItem->Type == ITEM_WING+20)			//  구슬
				|| (pItem->Type >= ITEM_WING+21 && pItem->Type <= ITEM_WING+24)
				|| (pItem->Type == ITEM_WING+35)	//  다크로드 스크롤
#ifdef CSK_ADD_SKILL_BLOWOFDESTRUCTION
				|| (pItem->Type == ITEM_WING+44)	// 파괴의일격 구슬
#endif // CSK_ADD_SKILL_BLOWOFDESTRUCTION
#ifdef YDG_ADD_SKILL_FLAME_STRIKE
				|| (pItem->Type == ITEM_WING+47)	// 플레임스트라이크 구슬
#endif	// YDG_ADD_SKILL_FLAME_STRIKE
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
				|| (pItem->Type==ITEM_WING+46)
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
				|| (pItem->Type==ITEM_WING+45)
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
#ifdef PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
				|| (pItem->Type==ITEM_WING+48)
#endif //PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
#ifdef YDG_ADD_SKILL_GIGANTIC_STORM
				|| (pItem->Type == ITEM_ETC+29)		// 기간틱스톰 법서
#endif	// YDG_ADD_SKILL_GIGANTIC_STORM
#ifdef KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
				|| ( pItem->Type == ITEM_ETC+28)	// 마력증대 법서
#endif // KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
				)
			{
				bool bReadBookGem = true;
				//	3차 전직 퀘스트(체인지업 마스터리)를 수행한 마스터만 읽을수 있는 법서.
				if ( (pItem->Type==ITEM_ETC+18)				// 헬버스트 법서
#ifdef KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
					|| (pItem->Type == ITEM_ETC+28)			// 마력증대 법서
#endif // KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
					|| pItem->Type == ITEM_WING+45			//멀티샷
#endif //#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
					|| (pItem->Type == ITEM_WING+46)	//회복
#endif //#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
#ifdef CSK_FIX_SKILL_ITEM
					|| (pItem->Type == ITEM_WING+44)	// 파괴의일격 구슬
#endif //CSK_FIX_SKILL_ITEM
					)
				{
					if ( g_csQuest.getQuestState2( QUEST_CHANGE_UP_3 )!=QUEST_END )		// 3차전직(체인지업 마스터리)
					
						bReadBookGem = false;
				}
#ifdef PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
				if(pItem->Type == ITEM_WING+48)			// 카오틱 
				{
					int Level = CharacterAttribute->Level;
					if( Level<220 )	//  220레벨이하면 못쓴다.
						bReadBookGem = false;
				}
#endif //#ifdef PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
				if ( bReadBookGem )
				{
					
					WORD Strength, Energy;
					Strength = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
					Energy	 = CharacterAttribute->Energy   + CharacterAttribute->AddEnergy;  
					
					if(CharacterAttribute->Level >= ItemAttribute[pItem->Type].RequireLevel && //레벨제한
						Energy >= pItem->RequireEnergy &&
						Strength >= pItem->RequireStrength) //필요힘 제한
					{
						SendRequestUse(iIndex, 0);
					}

					return true;
				}
			}
			else if ( pItem->Type==ITEM_HELPER+15) //  스테이트 열매를 사용한다.
			{
				int Level = CharacterAttribute->Level;
				
				if( Level>=10 )	//  10레벨이상 캐릭터들만 사용 가능.
				{
					bool bEquipmentEmpty = true;	
					for(int i=0; i<MAX_EQUIPMENT; i++)
					{
						if(CharacterMachine->Equipment[i].Type != -1)
						{
							bEquipmentEmpty = false;
						}
					}
					int Class = CharacterAttribute->Class;
					
					if(bEquipmentEmpty == true)	// 장비를 전부 미착용했으면
					{
						if(pItem->Level == 32)	// 통솔열매인가?
						{
							if (GetBaseClass(Class) != CLASS_DARK_LORD)		// 캐릭터 클래스가 다크로드 계열이 아닌가?
							{
								// 1905 "다크로드만 사용할 수 있습니다."
								SEASON3B::CreateOkMessageBox(GlobalText[1905]);
								return true;
							}
						}
						SetStandbyItemKey(pItem->Key);
						SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CUseFruitMsgBoxLayout));
						return true;
					}
					else	// 장비를 하나라도 착용하고 있으면
					{
						SEASON3B::CreateOkMessageBox(GlobalText[1909]);
						return true;
					}
				}
				else	//10레벨이하일때 메세지 박스 출력
				{
					SEASON3B::CreateOkMessageBox(GlobalText[749]); // "캐릭터 레벨 10이상부터 열매를 사용하실 수 있습니다."
					return true;
				}
			}
			else if( pItem->Type==ITEM_HELPER+11)    //  주문서.
			{
				bool bUse = false;
				int  Level = (pItem->Level>>3)&15;
				switch ( Level )
				{
				case 0: //  가디언.
					bUse = true;
					break;
				case 1: //  라이프 스톤.
					if ( Hero->GuildStatus!=G_MASTER )
						bUse = true;
					break;
				}
				if (bUse)
				{
					SendRequestUse(iIndex, 0);
					return true;
				}
			}
#ifdef YDG_ADD_CS5_REVIVAL_CHARM
			else if( pItem->Type==ITEM_HELPER+69 )	// 부활의 부적
			{
				if (g_PortalMgr.IsRevivePositionSaved())
				{
					if (g_PortalMgr.IsPortalUsable())
					{
						SetStandbyItemKey(pItem->Key);
						SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CUseReviveCharmMsgBoxLayout));
					}
					else
					{
						SEASON3B::CreateOkMessageBox(GlobalText[2608]); // "해당 지역에서는 사용할 수 없습니다."
					}
				}
			}
#endif	// YDG_ADD_CS5_REVIVAL_CHARM
#ifdef YDG_ADD_CS5_PORTAL_CHARM
			else if( pItem->Type==ITEM_HELPER+70)	// 이동의 부적
			{
				if (g_PortalMgr.IsPortalUsable())
				{
					if (pItem->Durability == 2)		// 이동 위치 저장
					{
						if (g_PortalMgr.IsPortalPositionSaved())
						{
							SEASON3B::CreateOkMessageBox(GlobalText[2610]);	// "이미 활성화 중인 아이템이 있으므로 사용할 수 없습니다."
						}
						else
						{
							SetStandbyItemKey(pItem->Key);
							SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CUsePortalCharmMsgBoxLayout));
						}
					}
					else if (pItem->Durability == 1)	// 이동 위치로 가기
					{
						SetStandbyItemKey(pItem->Key);
						SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CReturnPortalCharmMsgBoxLayout));
					}
				}
				else
				{
					SEASON3B::CreateOkMessageBox(GlobalText[2608]); // "해당 지역에서는 사용할 수 없습니다."
				}
			}
#endif	// YDG_ADD_CS5_PORTAL_CHARM
#ifdef PBG_ADD_SANTAINVITATION
			else if( pItem->Type == ITEM_HELPER + 66 )	//산타마을의 초대장
			{
				SetStandbyItemKey(pItem->Key);
				SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CUseSantaInvitationMsgBoxLayout));
			}
#endif //PBG_ADD_SANTAINVITATION

#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
			else if(g_pMyInventory->IsInvenItem(pItem->Type))
			{
#ifdef LJH_FIX_APP_SHUTDOWN_WEQUIPPING_INVENITEM_WITH_CLICKING_MOUSELBTN
				if (MouseLButton || MouseLButtonPop || MouseLButtonPush)
					return false;
#endif //LJH_FIX_APP_SHUTDOWN_WEQUIPPING_INVENITEM_WITH_CLICKING_MOUSELBTN
				// 내구도가 0인 경우 아이템을 장착할 수 없음.
				// 이미 장착되어 있는 종류의 아이템을 장착하려고 할 때에 대한 처리는 서버.
				if (pItem->Durability == 0) 
					return false; 

				int iChangeInvenItemStatus = 0;
				// 장착되어 있으면 장착해제 요청, 장착되어 있지 않으면 장착 요청
				(pItem->Durability == 255) ? iChangeInvenItemStatus = 254 : iChangeInvenItemStatus = 255;

				SendRequestEquippingInventoryItem(MAX_EQUIPMENT_INDEX+iIndex, iChangeInvenItemStatus);
			}
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		}
	}
	else if(m_pNewInventoryCtrl && GetRepairMode() == REPAIR_MODE_ON && SEASON3B::IsPress(VK_LBUTTON))
	{
		ITEM* pItem = m_pNewInventoryCtrl->FindItemAtPt(MouseX, MouseY);
		if(pItem == NULL)
		{
			return false;
		}

		// 수리금지 아이템
		if(IsRepairBan(pItem) == true)
		{
			return false;
		}

		int iIndex = MAX_EQUIPMENT + pItem->x+pItem->y*m_pNewInventoryCtrl->GetNumberOfColumn();
		
		if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP) && g_pNPCShop->IsRepairShop())
		{
			SendRequestRepair(iIndex, 0);
		}
		else if(m_bRepairEnableLevel == true)
		{
			SendRequestRepair(iIndex, 1);
		}
		
		return true;
	}

	return false;
}

bool SEASON3B::CNewUIMyInventory::BtnProcess()
{
	POINT ptExitBtn1 = { m_Pos.x+169, m_Pos.y+7 };
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	POINT ptExitBtn2 = { m_Pos.x+13, m_Pos.y+391 };
	POINT ptRepairBtn = { m_Pos.x+49, m_Pos.y+391 };
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX

	//. Exit1 버튼 (기본처리)
	if(SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_INVENTORY);
		return true;
	}
	if(m_BtnExit.UpdateMouseEvent() == true)
	{
		if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MYSHOP_INVENTORY))
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_MYSHOP_INVENTORY);
		}
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_INVENTORY);
		return true;
	}

	if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP)==false
		&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_TRADE)==false 
		&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_DEVILSQUARE)==false
		&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_BLOODCASTLE)==false
#ifdef LEM_ADD_LUCKYITEM
		&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND)==false
#endif // LEM_ADD_LUCKYITEM
		&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MIXINVENTORY)==false
		&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE)==false)
	{
		if(m_bRepairEnableLevel == true && m_BtnRepair.UpdateMouseEvent() == true)
		{
			//. 토글 수리모드
			ToggleRepairMode();
			
			return true;
		}

		if( m_bMyShopOpen == true && m_BtnMyShop.UpdateMouseEvent() == true )
		{
			// 개인상점 토글 모드
			if(m_MyShopMode == MYSHOP_MODE_OPEN)
			{	
				ChangeMyShopButtonStateClose();
				g_pNewUISystem->Show(SEASON3B::INTERFACE_MYSHOP_INVENTORY);
			}
			else if(m_MyShopMode == MYSHOP_MODE_CLOSE)
			{
				ChangeMyShopButtonStateOpen();
				g_pNewUISystem->Hide(SEASON3B::INTERFACE_MYSHOP_INVENTORY);
				g_pNewUISystem->Hide(SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY);
			}

			return true;
		}
	}
	
	return false;
}

void SEASON3B::CNewUIMyInventory::RenderItemToolTip(int iSlotIndex)
{
	if(m_iPointedSlot != -1)
	{
		ITEM* pEquipmentItemSlot = &CharacterMachine->Equipment[iSlotIndex];
		if(pEquipmentItemSlot->Type != -1)
		{
			int iTargetX = m_EquipmentSlots[iSlotIndex].x + m_EquipmentSlots[iSlotIndex].width / 2;
			int iTargetY = m_EquipmentSlots[iSlotIndex].y + m_EquipmentSlots[iSlotIndex].height / 2;

#ifdef LDS_FIX_OUTPUT_WRONG_COUNT_EQUIPPEDSETITEMOPTIONVALUE
			pEquipmentItemSlot->bySelectedSlotIndex = iSlotIndex;
#endif // LDS_FIX_OUTPUT_WRONG_COUNT_EQUIPPEDSETITEMOPTIONVALUE

			if(m_RepairMode == REPAIR_MODE_OFF)
			{
#ifndef CSK_FIX_ITEMTOOLTIP_POS			// 정리할 때 지워야 하는 소스	
				if(iSlotIndex == EQUIPMENT_WEAPON_LEFT)
				{
					iTargetY += 100.f;
				}
#endif //! CSK_FIX_ITEMTOOLTIP_POS		// 정리할 때 지워야 하는 소스
				RenderItemInfo(iTargetX, iTargetY, pEquipmentItemSlot, false);
			}
			else
			{
				RenderRepairInfo(iTargetX, iTargetY, pEquipmentItemSlot, false);
			}
		}
	}
}

bool SEASON3B::CNewUIMyInventory::CanRegisterItemHotKey(int iType)
{
	switch(iType)
	{
	case ITEM_POTION+0:		// 사과
	case ITEM_POTION+1:		// 작은치료물약	
	case ITEM_POTION+2:		// 중간치료물약	
	case ITEM_POTION+3:		// 큰치료물약	
	case ITEM_POTION+4:		// 작은마나물약	
	case ITEM_POTION+5:		// 중간마나물약	
	case ITEM_POTION+6:		// 큰마나물약		
	case ITEM_POTION+7:		// 공성물약(축복의물약, 영혼의물약)
	case ITEM_POTION+8:		// 해독물약
	case ITEM_POTION+9:		// 술
	case ITEM_POTION+10:	// 마을귀환문서
	case ITEM_POTION+20:	// 사랑의 묘약
	case ITEM_POTION+35:	// 작은SD회복물약
	case ITEM_POTION+36:	// 중간SD회복물약
	case ITEM_POTION+37:	// 큰SD회복물약
	case ITEM_POTION+38:	// 작은복합물약
	case ITEM_POTION+39:	// 중간복합물약
	case ITEM_POTION+40:	// 큰복합물약
	case ITEM_POTION+46:	// 잭오랜턴의축복
	case ITEM_POTION+47:	// 잭오랜턴의분노
	case ITEM_POTION+48:	// 잭오랜턴의외침
	case ITEM_POTION+49:	// 잭오랜턴의음식
	case ITEM_POTION+50:	// 잭오랜턴의음료
#ifdef PSW_ELITE_ITEM
	case ITEM_POTION+70:    // 부분유료화 엘리트 체력 물약
	case ITEM_POTION+71:    // 부분유료화 엘리트 마나 물약
#endif //PSW_ELITE_ITEM
#ifdef PSW_ELITE_ITEM
	case ITEM_POTION+78:    // 부분유료화 힘의 비약
	case ITEM_POTION+79:    // 부분유료화 민첩의 비약
	case ITEM_POTION+80:    // 부분유료화 체력의 비약
	case ITEM_POTION+81:    // 부분유료화 에너지의 비약
	case ITEM_POTION+82:    // 부분유료화 통솔의
#endif //PSW_ELITE_ITEM
#ifdef PSW_NEW_ELITE_ITEM
	case ITEM_POTION+94:    // 부분유료화 엘리트 중간 체력 물약
#endif //PSW_NEW_ELITE_ITEM
#ifdef CSK_EVENT_CHERRYBLOSSOM
	case ITEM_POTION+85:	// 벚꽃술
	case ITEM_POTION+86:	// 벚꽃경단
	case ITEM_POTION+87:	// 벚꽃잎
#endif //CSK_EVENT_CHERRYBLOSSOM
#ifdef YDG_ADD_CS7_ELITE_SD_POTION
	case ITEM_POTION+133:	// 엘리트SD회복물약
#endif	// YDG_ADD_CS7_ELITE_SD_POTION
		return true;
	}

	return false;
}

bool SEASON3B::CNewUIMyInventory::CanOpenMyShopInterface()
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP)	
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE)
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MIXINVENTORY)
#ifdef LEM_ADD_LUCKYITEM
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND)
#endif // LEM_ADD_LUCKYITEM
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_TRADE)
#ifdef LJH_MOD_CANT_OPENNING_PERSONALSHOP_WIN_IN_CURSED_TEMPLE 
		|| g_CursedTemple->IsCursedTemple() 
#endif //LJH_MOD_CANT_OPENNING_PERSONALSHOP_WIN_IN_CURSED_TEMPLE
		)
	{
		return false;
	}
	return true;
}

bool SEASON3B::CNewUIMyInventory::IsRepairEnableLevel()
{
	return m_bRepairEnableLevel;
}

void SEASON3B::CNewUIMyInventory::SetRepairEnableLevel(bool bOver)
{
	m_bRepairEnableLevel = bOver;
}

void SEASON3B::CNewUIMyInventory::ChangeMyShopButtonStateOpen()
{
	m_MyShopMode = MYSHOP_MODE_OPEN;	
	m_BtnMyShop.UnRegisterButtonState();
	m_BtnMyShop.RegisterButtonState(BUTTON_STATE_UP, IMAGE_INVENTORY_MYSHOP_OPEN_BTN, 0);
	m_BtnMyShop.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_INVENTORY_MYSHOP_OPEN_BTN, 1);
	m_BtnMyShop.ChangeImgIndex(IMAGE_INVENTORY_MYSHOP_OPEN_BTN, 0);
	// 1125 "개인상점열기(S)"
	m_BtnMyShop.ChangeToolTipText(GlobalText[1125], true);
}

void SEASON3B::CNewUIMyInventory::ChangeMyShopButtonStateClose()
{
	m_MyShopMode = MYSHOP_MODE_CLOSE;
	m_BtnMyShop.UnRegisterButtonState();
	m_BtnMyShop.RegisterButtonState(BUTTON_STATE_UP, IMAGE_INVENTORY_MYSHOP_CLOSE_BTN, 0);
	m_BtnMyShop.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_INVENTORY_MYSHOP_CLOSE_BTN, 1);
	m_BtnMyShop.ChangeImgIndex(IMAGE_INVENTORY_MYSHOP_CLOSE_BTN, 0);
	// 1127 "개인상점닫기(S)"
	m_BtnMyShop.ChangeToolTipText(GlobalText[1127], true);
}

void SEASON3B::CNewUIMyInventory::LockMyShopButtonOpen()
{
	m_BtnMyShop.ChangeImgColor(BUTTON_STATE_UP, RGBA(100, 100, 100, 255));
	m_BtnMyShop.ChangeTextColor(RGBA(100, 100, 100, 255));
	m_BtnMyShop.Lock();
	// 1125 "개인상점열기(S)"
	m_BtnMyShop.ChangeToolTipText(GlobalText[1125], true);	
}

void SEASON3B::CNewUIMyInventory::UnlockMyShopButtonOpen()
{
	m_BtnMyShop.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
	m_BtnMyShop.ChangeTextColor(RGBA(255, 255, 255, 255));
	m_BtnMyShop.UnLock();
	// 1125 "개인상점열기(S)"	
	m_BtnMyShop.ChangeToolTipText(GlobalText[1125], true);	
}

void SEASON3B::CNewUIMyInventory::ToggleRepairMode()
{
	//. 토글 수리모드
	if(m_RepairMode == REPAIR_MODE_OFF)
	{
		SetRepairMode(true);
	}
	else if(m_RepairMode == REPAIR_MODE_ON)
	{
		SetRepairMode(false);
	}
}

bool SEASON3B::CNewUIMyInventory::IsItem( short int siType, bool bcheckPick )
{
	if( bcheckPick == true )
	{
		CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();

		if( pPickedItem )
		{
			ITEM* pItemObj = pPickedItem->GetItem();

			if( pItemObj->Type == siType ) return true;	
		}
	}
		
	ITEM* pholyitemObj = m_pNewInventoryCtrl->FindTypeItem( siType );

	if( pholyitemObj ) return true;	

	return false;
}

int SEASON3B::CNewUIMyInventory::GetNumItemByKey( DWORD dwItemKey )
{
	return m_pNewInventoryCtrl->GetNumItemByKey( dwItemKey );
}

int SEASON3B::CNewUIMyInventory::GetNumItemByType(short sItemType)
{
	return m_pNewInventoryCtrl->GetNumItemByType(sItemType);
}

BYTE SEASON3B::CNewUIMyInventory::GetDurabilityPointedItem()
{
	ITEM* pItem = NULL;	
	
	// 먼저 장비창쪽 검사
	if(m_iPointedSlot != -1)
	{
		pItem = &CharacterMachine->Equipment[m_iPointedSlot];
		BYTE byDurability = pItem->Durability;

		return byDurability;
	}

	// 다음으로 인벤토리 검사
	pItem = m_pNewInventoryCtrl->FindItemPointedSquareIndex();
	if(pItem != NULL)
	{
		BYTE byDurability = pItem->Durability;
		return byDurability;
	}
	
	return 0;
}

int SEASON3B::CNewUIMyInventory::GetPointedItemIndex()
{
	// 먼저 장비창쪽 검사
	if(m_iPointedSlot != -1)
	{
		return m_iPointedSlot;
	}

	// 다음으로 인벤토리 검사
	int iPointedItemIndex = m_pNewInventoryCtrl->GetPointedSquareIndex();
	if(iPointedItemIndex != -1)
	{
		iPointedItemIndex += MAX_EQUIPMENT_INDEX;
		return iPointedItemIndex;
	}

	return -1;
}

int SEASON3B::CNewUIMyInventory::FindManaItemIndex()
{
	for(int i=ITEM_POTION+6; i>=ITEM_POTION+4; i--)
	{
		int iIndex = FindItemReverseIndex(i);
		if(iIndex != -1)
		{
			return iIndex;
		}
	}

	return -1;
}

void SEASON3B::CNewUIMyInventory::ResetMouseLButton()
{
	MouseLButton = false;
	MouseLButtonPop = false;
	MouseLButtonPush = false;	
}

void SEASON3B::CNewUIMyInventory::ResetMouseRButton()
{
	MouseRButton = false;
	MouseRButtonPop = false;
	MouseRButtonPush = false;
}


#ifdef NEW_USER_INTERFACE_UISYSTEM

extern float g_fScreenRate_x;
extern float g_fScreenRate_y;

void SEASON3B::CNewUIMyInventory::UpdateShop( const Coord& pos )
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_PARTCHARGE_SHOP) == false || !m_pNewInventoryCtrl ) 
	{
		return;
	}

	Coord OldWindowSize = Coord( WindowWidth, WindowHeight );
	
	WindowWidth = 640;
	WindowHeight = 480;

	Coord OldMousePos = Coord( MouseX, MouseY );
	
	MouseX = MouseX*g_fScreenRate_x;
	MouseY = MouseY*g_fScreenRate_y;
	
	Coord OldPos = Coord( m_pNewInventoryCtrl->GetPos().x, m_pNewInventoryCtrl->GetPos().y );
	m_pNewInventoryCtrl->SetPos( pos.x, pos.y );

	if(m_pNewInventoryCtrl->UpdateMouseEvent() == false)
	{
		m_pNewInventoryCtrl->UpdateProcess();

		m_pNewInventoryCtrl->SetPos( OldPos.x, OldPos.y );
		
		MouseX = OldMousePos.x;
		MouseY = OldMousePos.y;
		
		WindowWidth = OldWindowSize.x;
		WindowHeight = OldWindowSize.y;

		return;
	}
	
	m_pNewInventoryCtrl->UpdateProcess();

	CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
	if( pPickedItem && SEASON3B::IsRelease(VK_LBUTTON))
	{
		ITEM* pPickItem = pPickedItem->GetItem();

		if( pPickItem ) 
		{
			if(pPickedItem->GetOwnerInventory() == m_pNewInventoryCtrl) 
			{
				//. 인벤토리에서 인벤토리
				
				int iSourceIndex = pPickedItem->GetSourceLinealPos();
				int iTargetIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);
				
				if(iTargetIndex != -1) 
				{
					ITEM* pItem = m_pNewInventoryCtrl->FindItem(iTargetIndex);

					if(pItem) 
					{
						bool bOverlay = m_pNewInventoryCtrl->IsOverlayItem(pPickItem, pItem);
						
						if(bOverlay) 
						{
							SendRequestEquipmentItem(REQUEST_EQUIPMENT_INVENTORY, MAX_EQUIPMENT_INDEX+iSourceIndex,
								pPickItem, REQUEST_EQUIPMENT_INVENTORY, MAX_EQUIPMENT_INDEX+iTargetIndex);
						}
					}
				}
				
				if(iTargetIndex != -1 && m_pNewInventoryCtrl->CanMove(iTargetIndex, pPickItem)) 
				{
					if(pPickedItem->GetSourceLinealPos() != iTargetIndex)
					{
						SendRequestEquipmentItem(REQUEST_EQUIPMENT_INVENTORY, MAX_EQUIPMENT_INDEX+iSourceIndex, 
							pPickItem, REQUEST_EQUIPMENT_INVENTORY, MAX_EQUIPMENT_INDEX+iTargetIndex);
					}
					else 
					{
						SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
					}
				}
			}
		}
	}

	m_pNewInventoryCtrl->SetPos( OldPos.x, OldPos.y );
	
	MouseX = OldMousePos.x;
	MouseY = OldMousePos.y;
	
	WindowWidth = OldWindowSize.x;
	WindowHeight = OldWindowSize.y;
}

void SEASON3B::CNewUIMyInventory::RenderShop( const Coord& pos )
{
	EnableAlphaTest();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	if( !m_pNewInventoryCtrl ) 
	{
		return;
	}

	Coord OldWindowSize = Coord( WindowWidth, WindowHeight );
	
	WindowWidth = 640;
	WindowHeight = 480;

	Coord OldMousePos = Coord( MouseX, MouseY );
	
	MouseX = MouseX*g_fScreenRate_x;
	MouseY = MouseY*g_fScreenRate_y;
	
	Coord OldPos = Coord( m_pNewInventoryCtrl->GetPos().x, m_pNewInventoryCtrl->GetPos().y );
	m_pNewInventoryCtrl->SetPos( pos.x, pos.y );
	
	m_pNewInventoryCtrl->Render();
	
	m_pNewInventoryCtrl->SetPos( OldPos.x, OldPos.y );
	
	MouseX = OldMousePos.x;
	MouseY = OldMousePos.y;
	
	WindowWidth = OldWindowSize.x;
	WindowHeight = OldWindowSize.y;
}

void SEASON3B::CNewUIMyInventory::RenderShop3D( const Coord& pos )
{
	if( !m_pNewInventoryCtrl ) 
	{
		return;
	}

	Coord OldWindowSize = Coord( WindowWidth, WindowHeight );
	WindowWidth = 640; WindowHeight = 480;

	Coord OldMousePos = Coord( MouseX, MouseY );

	MouseX = MouseX*g_fScreenRate_x;
	MouseY = MouseY*g_fScreenRate_y;

	Coord OldPos = Coord( m_pNewInventoryCtrl->GetPos().x, m_pNewInventoryCtrl->GetPos().y );
	m_pNewInventoryCtrl->SetPos( pos.x, pos.y );

	m_pNewInventoryCtrl->Render3D();
	
	WindowWidth = OldWindowSize.x;
	WindowHeight = OldWindowSize.y;
	BeginBitmap();
	WindowWidth = 640; WindowHeight = 480;
	m_pNewInventoryCtrl->RenderNumberOfItem();
	EndBitmap();

	CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
	if( pPickedItem ) 
	{
		ITEM* pPickItem = pPickedItem->GetItem();
		ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pPickItem->Type];

		Coord pos = Coord( m_Pos.x = MouseX, MouseY);
		Dim size = Dim( pItemAttr->Width*INVENTORY_SQUARE_WIDTH, pItemAttr->Height*INVENTORY_SQUARE_HEIGHT);

		pos.x = pos.x - size.x/2;
		pos.y = pos.y - size.y/2;

		WindowWidth = OldWindowSize.x;
		WindowHeight = OldWindowSize.y;
		BeginDrawMesh();
		WindowWidth = 640; WindowHeight = 480;
		RenderItem3D(pos.x, pos.y, size.x, size.y, pPickItem->Type, pPickItem->Level, pPickItem->Option1, pPickItem->ExtOption, true);
	}

	m_pNewInventoryCtrl->SetPos( OldPos.x, OldPos.y );
	
	MouseX = OldMousePos.x;
	MouseY = OldMousePos.y;

	WindowWidth = OldWindowSize.x;
	WindowHeight = OldWindowSize.y;
}

#endif //NEW_USER_INTERFACE_UISYSTEM

#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
/////////////////////////////////////////////////////////////////////
// 함수: IsInvenItem( ITEM* pItem )
// 결과: 아이템이 인벤장착가능한 아이템이면 return TRUE
//		 otherwise, return FALS
/////////////////////////////////////////////////////////////////////
BOOL SEASON3B::CNewUIMyInventory::IsInvenItem( const short sType )
{
	BOOL bInvenItem = FALSE;

	if (FALSE 
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM			// 매조각상, 양조각상, 편자
		|| (sType == ITEM_HELPER+128 || sType == ITEM_HELPER+129 || sType == ITEM_HELPER+134)
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2	// 오크참, 메이플참, 골든오크참, 골든메이플참
		|| (sType >= ITEM_HELPER+130 && sType <= ITEM_HELPER+133 )
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
	)
		bInvenItem = TRUE;
	
	return bInvenItem;
}
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY