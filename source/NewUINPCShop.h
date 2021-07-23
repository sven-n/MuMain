// NewUINPCShop.h: interface for the CNewUINPCShop class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUINPCSHOP_H__EEE639A8_C89E_47B3_8DBA_22560F102D98__INCLUDED_)
#define AFX_NEWUINPCSHOP_H__EEE639A8_C89E_47B3_8DBA_22560F102D98__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NewUIBase.h"
#include "NewUIInventoryCtrl.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#include "NewUIButton.h"

namespace SEASON3B
{
	class CNewUINPCShop : public CNewUIObj
	{
	public:
		enum IMAGE_LIST
		{
			IMAGE_NPCSHOP_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
			IMAGE_NPCSHOP_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP2,
			IMAGE_NPCSHOP_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,
			IMAGE_NPCSHOP_RIGHT= CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
			IMAGE_NPCSHOP_BOTTOM= CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
			IMAGE_NPCSHOP_BTN_REPAIR = CNewUIMyInventory::IMAGE_INVENTORY_REPAIR_BTN,

			IMAGE_NPCSHOP_REPAIR_MONEY = BITMAP_INTERFACE_NEW_NPCSHOP_BEGIN,
		};
		
		enum
		{
			NPCSHOP_POS_X = 260,
			NPCSHOP_POS_Y = 0,

			// 상점 상태(토글된다.)
			SHOP_STATE_BUYNSELL = 1,	// 사고 파는 상태
			SHOP_STATE_REPAIR = 2,		// 수리 상태
		};
		
	private:
		enum
		{
			NPCSHOP_WIDTH = 190,
			NPCSHOP_HEIGHT = 429,
		};

		CNewUIManager*			m_pNewUIMng;
		CNewUIInventoryCtrl*	m_pNewInventoryCtrl;
		POINT m_Pos;

		// 상점 정보
		DWORD m_dwShopState;	// 상점 현재 상태

		// 상점 세율 정보
		int m_iTaxRate;

		// 수리 정보
		bool m_bRepairShop;		// 수리가능한 상점인가
		
#ifdef KJH_FIX_WOPS_K32595_DOUBLE_CLICK_PURCHASE_ITEM			
		bool m_bIsNPCShopOpen;	// 상점이 열리고 난 상태
#else // KJH_FIX_WOPS_K32595_DOUBLE_CLICK_PURCHASE_ITEM			// 디파인 정리할때 지워야 하는 부분
#ifdef KJH_FIX_WOPS_K22181_ITEM_PURCHASED_OPENNING_SHOP
		// 상점 처음열림상태
		bool m_bFirstOpen;		// 상점이 처음 열렸나?
#endif // KJH_FIX_WOPS_K22181_ITEM_PURCHASED_OPENNING_SHOP
#endif // KJH_FIX_WOPS_K32595_DOUBLE_CLICK_PURCHASE_ITEM		// 디파인 정리할때 지워야 하는 부분

		// 버튼
		CNewUIButton m_BtnRepair;		// 수리 버튼
		CNewUIButton m_BtnRepairAll;	// 전체수리 버튼
		
		DWORD m_dwStandbyItemKey;
	
#ifdef CSK_FIX_HIGHVALUE_MESSAGEBOX
		bool m_bSellingItem;	// 고가 아이템 판매중인가(패킷을 날려서 패킷 받기를 기다리고 있는 중인가?)
#endif // CSK_FIX_HIGHVALUE_MESSAGEBOX
		
	public:
		CNewUINPCShop();
		virtual ~CNewUINPCShop();

		bool Create(CNewUIManager* pNewUIMng, int x, int y);
		void Release();

		void SetPos(int x, int y);

		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		bool Update();
		bool Render();

		float GetLayerDepth();	//. 2.5f

		// 세율 관련 함수
		void SetTaxRate(int iTaxRate);
		int GetTaxRate();

		// 상점 아이템 등록
		bool InsertItem(int iIndex, BYTE* pbyItemPacket);
		
#ifdef KJH_FIX_WOPS_K22181_ITEM_PURCHASED_OPENNING_SHOP
		// 상점 열 때 프로세스
		void OpenningProcess();
#endif // KJH_FIX_WOPS_K22181_ITEM_PURCHASED_OPENNING_SHOP

		// 상점 닫을 때 프로세스
		void ClosingProcess();

		// 수리 관련 함수
		void SetRepairShop(bool bRepair);
		bool IsRepairShop();
		void ToggleState();
		DWORD GetShopState();
		
		int GetPointedItemIndex();

		//. Exporting Functions
		void SetStandbyItemKey(DWORD dwItemKey);
		DWORD GetStandbyItemKey() const;
		int GetStandbyItemIndex();
		ITEM* GetStandbyItem();

#ifdef CSK_FIX_HIGHVALUE_MESSAGEBOX
		void SetSellingItem(bool bFlag);
		bool IsSellingItem();
#endif // CSK_FIX_HIGHVALUE_MESSAGEBOX

	private:
		void Init();
		void SetButtonInfo();
		
		void LoadImages();
		void UnloadImages();

		bool InventoryProcess();
		bool BtnProcess();

		void RenderFrame();
		void RenderTexts();
		void RenderButton();
		void RenderRepairMoney();
	};
}

#endif // !defined(AFX_NEWUINPCSHOP_H__EEE639A8_C89E_47B3_8DBA_22560F102D98__INCLUDED_)
