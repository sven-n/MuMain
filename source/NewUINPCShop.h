// NewUINPCShop.h: interface for the CNewUINPCShop class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUINPCSHOP_H__EEE639A8_C89E_47B3_8DBA_22560F102D98__INCLUDED_)
#define AFX_NEWUINPCSHOP_H__EEE639A8_C89E_47B3_8DBA_22560F102D98__INCLUDED_

#pragma once

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
			SHOP_STATE_BUYNSELL = 1,
			SHOP_STATE_REPAIR = 2,
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

		DWORD m_dwShopState;

		int m_iTaxRate;

		bool m_bRepairShop;
		
#ifdef KJH_FIX_WOPS_K32595_DOUBLE_CLICK_PURCHASE_ITEM			
		bool m_bIsNPCShopOpen;
#else // KJH_FIX_WOPS_K32595_DOUBLE_CLICK_PURCHASE_ITEM
#ifdef KJH_FIX_WOPS_K22181_ITEM_PURCHASED_OPENNING_SHOP
		bool m_bFirstOpen;
#endif // KJH_FIX_WOPS_K22181_ITEM_PURCHASED_OPENNING_SHOP
#endif // KJH_FIX_WOPS_K32595_DOUBLE_CLICK_PURCHASE_ITEM

		// ¹öÆ°
		CNewUIButton m_BtnRepair;
		CNewUIButton m_BtnRepairAll;
		
		DWORD m_dwStandbyItemKey;
	
#ifdef CSK_FIX_HIGHVALUE_MESSAGEBOX
		bool m_bSellingItem;
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

		void SetTaxRate(int iTaxRate);
		int GetTaxRate();

		bool InsertItem(int iIndex, BYTE* pbyItemPacket);
		
#ifdef KJH_FIX_WOPS_K22181_ITEM_PURCHASED_OPENNING_SHOP
		void OpenningProcess();
#endif // KJH_FIX_WOPS_K22181_ITEM_PURCHASED_OPENNING_SHOP

		void ClosingProcess();
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
