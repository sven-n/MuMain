// NewUIPersonalInventory.h: interface for the CNewUIMyInventory class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIMYINVENTORY_H__74DA6D7A_CF5A_46E9_8C72_9D38F0DC95EC__INCLUDED_)
#define AFX_NEWUIMYINVENTORY_H__74DA6D7A_CF5A_46E9_8C72_9D38F0DC95EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NewUIBase.h"
#include "NewUIInventoryCtrl.h"
#include "NewUIMessageBox.h"
#include "NewUI3DRenderMng.h"
#include "NewUIButton.h"

namespace SEASON3B
{
	class CNewUIMyInventory : public CNewUIObj, public INewUI3DRenderObj
	{
	public:
		enum IMAGE_LIST
		{
			IMAGE_INVENTORY_MYSHOP_OPEN_BTN = BITMAP_MYSHOPINTERFACE_NEW_PERSONALINVENTORY_BEGIN+1,
			IMAGE_INVENTORY_MYSHOP_CLOSE_BTN = BITMAP_MYSHOPINTERFACE_NEW_PERSONALINVENTORY_BEGIN+2,
			IMAGE_INVENTORY_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
			IMAGE_INVENTORY_BACK_TOP = BITMAP_INTERFACE_NEW_PERSONALINVENTORY_BEGIN,	//"newui_item_back01.tga"
			IMAGE_INVENTORY_BACK_TOP2,	//"newui_item_back04.tga"
			IMAGE_INVENTORY_BACK_LEFT,	//"newui_item_back02-L.tga"
			IMAGE_INVENTORY_BACK_RIGHT,	//"newui_item_back02-R.tga"
			IMAGE_INVENTORY_BACK_BOTTOM,	//"newui_item_back03.tga"
			IMAGE_INVENTORY_ITEM_BOOT,	//"newui_item_boots.tga"
			IMAGE_INVENTORY_ITEM_HELM,	//"newui_item_cap.tga"
			IMAGE_INVENTORY_ITEM_FAIRY,	//"newui_item_fairy.tga"
			IMAGE_INVENTORY_ITEM_WING,	//"newui_item_wing.tga"
			IMAGE_INVENTORY_ITEM_RIGHT,	//"newui_item_weapon(L).tga"
			IMAGE_INVENTORY_ITEM_LEFT,	//"newui_item_weapon(R).tga"
			IMAGE_INVENTORY_ITEM_ARMOR,	//"newui_item_upper.tga"
			IMAGE_INVENTORY_ITEM_GLOVES,	//"newui_item_gloves.tga"
			IMAGE_INVENTORY_ITEM_PANTS,	//"newui_item_lower.tga"
			IMAGE_INVENTORY_ITEM_RING,	//"newui_item_ring.tga"
			IMAGE_INVENTORY_ITEM_NECKLACE,	//"newui_item_necklace.tga"
			IMAGE_INVENTORY_MONEY,	//"newui_item_money.tga"
			IMAGE_INVENTORY_EXIT_BTN, //"newui_exit_00.tga"
			IMAGE_INVENTORY_REPAIR_BTN, //"newui_repair_00.tga"
			
		};

		enum REPAIR_MODE
		{
			REPAIR_MODE_OFF = 0,
			REPAIR_MODE_ON,
		};

		enum MYSHOP_MODE
		{
			MYSHOP_MODE_OPEN = 0,
			MYSHOP_MODE_CLOSE,	
		};

	private:
		enum ITEM_OPTION
		{
			ITEM_SET_OPTION = 1,
#ifdef SOCKET_SYSTEM
			ITEM_SOCKET_SET_OPTION = 2,
#endif	// SOCKET_SYSTEM
		};

		enum
		{
			INVENTORY_WIDTH = 190,
			INVENTORY_HEIGHT = 429,
		};
		typedef struct tagEQUIPMENT_ITEM
		{
			int x, y;
			int width, height;
			DWORD dwBgImage;
		} EQUIPMENT_ITEM;

		CNewUIManager*			m_pNewUIMng;
		CNewUI3DRenderMng*		m_pNewUI3DRenderMng;
		CNewUIInventoryCtrl*	m_pNewInventoryCtrl;
		POINT m_Pos;

		EQUIPMENT_ITEM m_EquipmentSlots[MAX_EQUIPMENT_INDEX];
		int	m_iPointedSlot;
		
		//. 버튼
		CNewUIButton m_BtnRepair;		// 수리 버튼
		CNewUIButton m_BtnExit;			// 닫기 버튼
		CNewUIButton m_BtnMyShop;		// 개인 상점 버튼

		MYSHOP_MODE m_MyShopMode;
		REPAIR_MODE m_RepairMode;
		DWORD m_dwStandbyItemKey;
		
		bool m_bRepairEnableLevel;		// 수리 가능 레벨이 넘었는가? 레벨50부터
		bool m_bMyShopOpen;				// 개인상점 열 수 있는가? 레벨6부터

	public:
		CNewUIMyInventory();
		virtual ~CNewUIMyInventory();
	
		bool Create(CNewUIManager* pNewUIMng, CNewUI3DRenderMng* pNewUI3DRenderMng, int x, int y);
		void Release();

		bool EquipItem(int iIndex, BYTE* pbyItemPacket);
		void UnequipItem(int iIndex);
		void UnequipAllItems();

		bool IsEquipable(int iIndex, ITEM* pItem);

		bool InsertItem(int iIndex, BYTE* pbyItemPacket);
		void DeleteItem(int iIndex);
		void DeleteAllItems();

		void SetPos(int x, int y);
		const POINT& GetPos() const;
		
		REPAIR_MODE GetRepairMode() const;
		void SetRepairMode(bool bRepair);
		
#ifdef NEW_USER_INTERFACE_UISYSTEM
		void UpdateShop( const Coord& pos );
		void RenderShop( const Coord& pos );
		void RenderShop3D( const Coord& pos );
#endif //NEW_USER_INTERFACE_UISYSTEM

#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		BOOL IsInvenItem( const short sType );
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY

		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		bool Update();
		bool Render();
		void Render3D();
		
		bool IsVisible() const;

		void OpenningProcess();
		void ClosingProcess();

		float GetLayerDepth();	//. 4.2f

		CNewUIInventoryCtrl* GetInventoryCtrl() const;
		
		ITEM* FindItem(int iLinealPos);
		ITEM* FindItemByKey(DWORD dwKey);
		int FindItemIndex( short int siType, int iLevel = -1 );
		int FindItemReverseIndex(short sType, int iLevel = -1);
		int FindEmptySlot(int cx, int cy);
		int FindEmptySlot(ITEM* pItem);
		bool IsItem( short int siType, bool bcheckPick = false );
		// 아이템 키값으로 인벤토리내에 있는 아이템 갯수 리턴
		int	GetNumItemByKey( DWORD dwItemKey );
		// 아이템 타입으로 인벤토리내에 있는 아이템 갯수 리턴
		int GetNumItemByType(short sItemType);
		
		// 마우스가 가르키고 있는 아이템의 내구도를 얻는 함수
		BYTE GetDurabilityPointedItem(); 
		// 마우스가 가르키고 있는 아이템의 인텍스를 얻는 함수
		int GetPointedItemIndex();
		
		// 마나아이템 찾아서 인덱스 리턴해주는 함수
		int FindManaItemIndex();

		static void UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB);
		
		//. Exporting Functions
		void SetStandbyItemKey(DWORD dwItemKey);
		DWORD GetStandbyItemKey() const;
		int GetStandbyItemIndex();
		ITEM* GetStandbyItem();

		bool IsRepairEnableLevel();
		void SetRepairEnableLevel(bool bOver);
		
		void ChangeMyShopButtonStateOpen();
		void ChangeMyShopButtonStateClose();
		void LockMyShopButtonOpen();
		void UnlockMyShopButtonOpen();

		void CreateEquippingEffect(ITEM* pItem);	//. 장착효과 생성: 유니리아, 펜릴, 사탄, 요정, 디노란트, 펫

		// QWER 아이템 핫키에 등록 가능한 아이템인가?
		static bool CanRegisterItemHotKey(int iType);

	protected:
		void DeleteEquippingEffect();				// 아이템 장착효과 제거
		void DeleteEquippingEffectBug(ITEM* pItem);	// 펫이나 탈 것 장착효과 제거

		void SetEquipmentSlotInfo();
		void SetButtonInfo();

#ifdef PBG_FIX_SKILLHOTKEY
		void ResetSkillofItem(ITEM* pItem);			//아이템의 스킬 핫키 기억 제거
#endif //PBG_FIX_SKILLHOTKEY
	private:
		void LoadImages();
		void UnloadImages();

		void RenderFrame();
		void RenderSetOption();
#ifdef SOCKET_SYSTEM
		void RenderSocketOption();
#endif	// SOCKET_SYSTEM
		void RenderEquippedItem();
		void RenderButtons();
		void RenderInventoryDetails();

		bool EquipmentWindowProcess();
		bool InventoryProcess();
		bool BtnProcess();

		void RenderItemToolTip(int iSlotIndex);
		bool CanOpenMyShopInterface();
		void ToggleRepairMode();
		
		void ResetMouseLButton();
		void ResetMouseRButton();
	};
}

#endif // !defined(AFX_NEWUIMYINVENTORY_H__74DA6D7A_CF5A_46E9_8C72_9D38F0DC95EC__INCLUDED_)
