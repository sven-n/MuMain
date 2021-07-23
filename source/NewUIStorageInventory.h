//*****************************************************************************
// File: NewUIStorageInventory.h
//
// Desc: interface for the CNewUIStorageInventory class.
//		 창고창 클래스.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#if !defined(AFX_NEWUISTORAGEINVENTORY_H__BD790479_EDDE_4981_9B03_A12163A58D5D__INCLUDED_)
#define AFX_NEWUISTORAGEINVENTORY_H__BD790479_EDDE_4981_9B03_A12163A58D5D__INCLUDED_

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
	class CNewUIStorageInventory : public CNewUIObj
	{
	public:
		enum IMAGE_LIST
		{
			IMAGE_STORAGE_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,	// Reference
			IMAGE_STORAGE_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP,
			IMAGE_STORAGE_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,
			IMAGE_STORAGE_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
			IMAGE_STORAGE_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,

			IMAGE_STORAGE_BTN_INSERT_ZEN = BITMAP_INTERFACE_NEW_STORAGE_BEGIN,
			IMAGE_STORAGE_BTN_TAKE_ZEN = BITMAP_INTERFACE_NEW_STORAGE_BEGIN + 1,
			IMAGE_STORAGE_BTN_UNLOCK = BITMAP_INTERFACE_NEW_STORAGE_BEGIN + 2,
			IMAGE_STORAGE_BTN_LOCK = BITMAP_INTERFACE_NEW_STORAGE_BEGIN + 3,

			IMAGE_STORAGE_MONEY = BITMAP_INTERFACE_NEW_STORAGE_BEGIN + 4,
		};
		
	private:
		enum
		{
			STORAGE_WIDTH = 190,
			STORAGE_HEIGHT = 429,
		};

		enum STORAGE_BUTTON
		{
			BTN_INSERT_ZEN = 0,		// 입금.
			BTN_TAKE_ZEN,			// 출금.
			BTN_LOCK,				// 잠금.
			MAX_BTN
		};
		
		CNewUIManager*			m_pNewUIMng;			// UI 매니저.
		POINT					m_Pos;					// 창의 위치.
		CNewUIButton			m_abtn[MAX_BTN];		// 버튼.
		CNewUIInventoryCtrl*	m_pNewInventoryCtrl;	// 인벤토리 컨트롤.

		bool					m_bLock;				// true면 창고가 잠겨있는 상태.
		bool					m_bCorrectPassword;		// true면 패스워드가 맞는 상태.

		bool					m_bItemAutoMove;		// 마우스 우버튼으로 아이템을 창고나 인벤토리로 이동시 true.
		int						m_nBackupMouseX;		// 아이템 자동 이동 시 마우스 X 좌표 백업.
		int						m_nBackupMouseY;		// 아이템 자동 이동 시 마우스 Y 좌표 백업.

	// 잠겨있을 때 서버로 비밀번호를 먼저 전송해야 되므로 상황에 따라
	//m_bTakeZen에 그 상황을 저장 후 출금액 또는 아이템의 옮길 인벤토리 위치 백업.
		bool					m_bTakeZen;				// true면 출금, false면 아이템.
		int						m_nBackupTakeZen;		// 출금될 젠.
		int						m_nBackupInvenIndex;	// 옮길 인벤토리 위치 백업.
		
	public:
		CNewUIStorageInventory();
		virtual ~CNewUIStorageInventory();
		
		bool Create(CNewUIManager* pNewUIMng, int x, int y);
		void Release();
		
		void SetPos(int x, int y);
		
		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		bool Update();
		bool Render();
		
		float GetLayerDepth();	//. 2.2f
		
		CNewUIInventoryCtrl* GetInventoryCtrl() const;

		bool ProcessClosing();
		bool InsertItem(int iIndex, BYTE* pbyItemPacket);
		int FindEmptySlot(ITEM* pItemObj);

		bool IsStorageLocked(){ return m_bLock; }				// 창고가 잠겨 있는가?
		bool IsCorrectPassword(){ return m_bCorrectPassword; }	// 패스워드가 통과된 상태인가?
		bool IsItemAutoMove(){ return m_bItemAutoMove; }		// 아이템 자동 이동인가?

		void SetBackupTakeZen(int nZen);

		void ProcessMyInvenItemAutoMove();

		void SendRequestItemToMyInven(ITEM* pItemObj, int nStorageIndex, int nInvenIndex);

		void ProcessToReceiveStorageStatus(BYTE byStatus);
		void ProcessToReceiveStorageItems(int nIndex, BYTE* pbyItemPacket);
		void ProcessStorageItemAutoMoveSuccess();
		void ProcessStorageItemAutoMoveFailure();
		
		int GetPointedItemIndex();

		void SetItemAutoMove(bool bItemAutoMove);
		
	private:
		void LoadImages();
		void UnloadImages();

		void RenderBackImage();
		void RenderText();

		void ChangeLockBtnImage();
		void DeleteAllItems();

		void LockStorage(bool bLock);
		// 올바른 패스워드이면 bCorrectPassword에 true.
		void SetCorrectPassword(bool bCorrectPassword)
		{ m_bCorrectPassword = bCorrectPassword; }

		void InitBackupItemInfo();
		int GetBackupTakeZen(){ return m_nBackupTakeZen; }			// 출금할 돈을 얻음.
		void SetBackupInvenIndex(int nInvenIndex);
		int GetBackupInvenIndex(){ return m_nBackupInvenIndex; }	// 백업된 인벤토리 인덱스를 얻음.

		void ProcessInventoryCtrl();
		bool ProcessBtns();
		void ProcessStorageItemAutoMove();

		void SendRequestItemToStorage(ITEM* pItemObj,
			int nInvenIndex, int nStorageIndex);
	};
}

#endif // !defined(AFX_NEWUISTORAGEINVENTORY_H__BD790479_EDDE_4981_9B03_A12163A58D5D__INCLUDED_)
