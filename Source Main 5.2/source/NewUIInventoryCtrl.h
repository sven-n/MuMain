// NewUIInventoryCtrl.h: interface for the CNewUIInventoryCtrl class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIINVENTORYCTRL_H__3A635AF1_3FFA_44B5_9BF6_3DFE0F061927__INCLUDED_)
#define AFX_NEWUIINVENTORYCTRL_H__3A635AF1_3FFA_44B5_9BF6_3DFE0F061927__INCLUDED_

#pragma once

#pragma warning(disable : 4786)
#include <vector>

#include "NewUI3DRenderMng.h"
#include "span.hpp"
#include "ZzzTexture.h"

namespace SEASON3B
{
    class CNewUIItemMng;
    class CNewUIInventoryCtrl;

    enum
    {
        INVENTORY_SQUARE_WIDTH = 20,
        INVENTORY_SQUARE_HEIGHT = 20,
    };
    enum TOOLTIP_TYPE
    {
        UNKNOWN_TOOLTIP_TYPE = 0,
        TOOLTIP_TYPE_INVENTORY,
        TOOLTIP_TYPE_REPAIR,
        TOOLTIP_TYPE_NPC_SHOP,
        TOOLTIP_TYPE_MY_SHOP,
        TOOLTIP_TYPE_PURCHASE_SHOP,
    };
    enum SQUARE_COLOR_STATE
    {
        UNKNOWN_COLOR_STATE = 0,
        COLOR_STATE_NORMAL,
        COLOR_STATE_WARNING,
    };

    class CNewUIPickedItem : public INewUI3DRenderObj
    {
        CNewUIItemMng* m_pNewItemMng;
        CNewUIInventoryCtrl* m_pSrcInventory;
        ITEM* m_pPickedItem;

        bool	m_bShow;
        POINT	m_Pos;
        SIZE	m_Size;

    public:
        CNewUIPickedItem();
        virtual ~CNewUIPickedItem();

        bool Create(CNewUIItemMng* pNewItemMng, CNewUIInventoryCtrl* pSrc, ITEM* pItem);
        void Release();

        CNewUIInventoryCtrl* GetOwnerInventory() const;
        STORAGE_TYPE GetSourceStorageType() const;
        ITEM* GetItem() const;

        const POINT& GetPos() const;
        const SIZE& GetSize() const;
        void GetRect(RECT& rcBox);

        int GetSourceLinealPos();
        bool GetTargetPos(CNewUIInventoryCtrl* pDest, int& iTargetColumnX, int& iTargetRowY);
        int GetTargetLinealPos(CNewUIInventoryCtrl* pDest);

        bool IsVisible() const;
        void ShowPickedItem();
        void HidePickedItem();

        void Render3D();
    };

    class CNewUIInventoryCtrl : public INewUI3DRenderObj
    {
    public:
        enum EVENT_STATE
        {
            EVENT_NONE = 0,
            EVENT_HOVER,
            EVENT_PICKING,
        };
        enum IMAGE_LIST
        {
            IMAGE_ITEM_SQUARE = BITMAP_INTERFACE_NEW_INVENTORY_BASE_BEGIN,	//. newui_item_box.tga
            IMAGE_ITEM_TABLE_TOP_LEFT,	//. newui_item_table01(L).tga
            IMAGE_ITEM_TABLE_TOP_RIGHT,	//. newui_item_table01(R).tga
            IMAGE_ITEM_TABLE_BOTTOM_LEFT,	//. newui_item_table02(L).tga
            IMAGE_ITEM_TABLE_BOTTOM_RIGHT,	//. newui_item_table02(R).tga
            IMAGE_ITEM_TABLE_TOP_PIXEL,		//. newui_item_table03(Up).tga
            IMAGE_ITEM_TABLE_BOTTOM_PIXEL,	//. newui_item_table03(Dw).tga
            IMAGE_ITEM_TABLE_LEFT_PIXEL,	//. newui_item_table03(L).tga
            IMAGE_ITEM_TABLE_RIGHT_PIXEL,	//. newui_item_table03(R).tga

#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
            IMAGE_ITEM_SQUARE_FOR_1_BY_1,	//. newui_inven_usebox_01.tga
            IMAGE_ITEM_SQUARE_TOP_RECT,		//. newui_inven_usebox_02.tga
            IMAGE_ITEM_SQUARE_BOTTOM_RECT,	//. newui_inven_usebox_03.tga
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
        };

    private:
        enum
        {
            ITEM_SQUARE_WITH = 20,
            ITEM_SQUARE_HEIGHT = 20,
            WND_TOP_EDGE = 3,
            WND_LEFT_EDGE = 4,
            WND_BOTTOM_EDGE = 8,
            WND_RIGHT_EDGE = 9,

            RENDER_NUMBER_OF_ITEM = 1,
            RENDER_ITEM_TOOLTIP = 2,
        };

        typedef std::vector<ITEM*>	type_vec_item;

        static CNewUIPickedItem* ms_pPickedItem;

        CNewUI3DRenderMng* m_pNew3DRenderMng;
        CNewUIItemMng* m_pNewItemMng;
        CNewUIObj* m_pOwner;

        type_vec_item	m_vecItem;
        POINT	m_Pos;
        SIZE	m_Size;
        STORAGE_TYPE m_StorageType;
        int	m_nColumn, m_nRow;
        /**
         * \brief The index of the first slot for this control.
         * For example, the box of an inventory starts at 12.
         */
        int m_nIndexOffset;
        DWORD* m_pdwItemCheckBox;
        EVENT_STATE	m_EventState;
        int	m_iPointedSquareIndex; // has m_nIndexOffset included
        bool m_bShow, m_bLock;

        TOOLTIP_TYPE	m_ToolTipType;
        ITEM* m_pToolTipItem;

        float m_afColorStateNormal[3], m_afColorStateWarning[3];

        bool m_bRepairMode;

        bool m_bCanPushItem;

        void Init();

        void LoadImages();
        void UnloadImages();

        void SetItemColorState(ITEM* pItem);
        bool CanChangeItemColorState(ITEM* pItem);

        void UpdateProcess();

        bool CheckSlot(int startIndex, int width, int height);
        bool CheckSlot(int iColumnX, int iRowY, int width, int height);
    public:
        CNewUIInventoryCtrl();
        virtual ~CNewUIInventoryCtrl();

        bool Create(STORAGE_TYPE storageType, CNewUI3DRenderMng* pNew3DRenderMng, CNewUIItemMng* pNewItemMng, CNewUIObj* pOwner, int x, int y, int nColumn, int nRow, int nIndexOffset = 0);
        void Release();

        bool AddItem(int iLinealPos, std::span<const BYTE> pbyItemPacket);
        bool AddItem(int iColumnX, int iRowY, std::span<const BYTE> pbyItemPacket);
        bool AddItem(int iColumnX, int iRowY, ITEM* pItem);
        bool AddItem(int iColumnX, int iRowY, BYTE byType, BYTE bySubType, BYTE byLevel = 0, BYTE byDurability = 255,
            BYTE byOption1 = 0, BYTE byOptionEx = 0, BYTE byOption380 = 0, BYTE byOptionHarmony = 0);
        void RemoveItem(ITEM* pItem);
        void RemoveAllItems();

        size_t GetNumberOfItems();

        bool IsItem(short int siType);
        int GetItemCount(short int siType, int iLevel = -1);

        ITEM* GetItem(int iIndex/* 0 <= iIndex < GetNumberOfItems() */);

        ITEM* FindItem(int iLinealPos);
        ITEM* FindItem(int iColumnX, int iRowY);
        ITEM* FindItemByKey(DWORD dwKey);
        ITEM* FindItemAtPt(int x, int y);
        ITEM* FindTypeItem(short int siType);
        int FindItemIndex(short int siType, int iLevel);
        int FindItemReverseIndex(short sType, int iLevel);
        int GetIndexByItem(ITEM* pItem);
        short int FindItemTypeByPos(int iColumnX, int iRowY);

        ITEM* FindItemPointedSquareIndex();
        int GetPointedSquareIndex();

        int	GetNumItemByKey(DWORD dwItemKey);
        int GetNumItemByType(short sItemType);

        void SetEventState(EVENT_STATE es);

        int FindEmptySlot(IN int cx, IN int cy);	//. return lineal position
        bool FindEmptySlot(IN int cx, IN int cy, OUT int& iColumnX, OUT int& iColumnY);
        int GetEmptySlotCount();

        bool UpdateMouseEvent();
        bool Update();

        void Render();

        void SetPos(int x, int y);
        const POINT& GetPos() const;
        int GetNumberOfColumn() const;
        int GetNumberOfRow() const;
        void GetRect(RECT& rcBox);

        STORAGE_TYPE GetStorageType() const { return m_StorageType; }

        void SetSquareColorNormal(float fRed, float fGreen, float fBlue);
        void GetSquareColorNormal(float* pfParams) const;
        void SetSquareColorWarning(float fRed, float fGreen, float fBlue);
        void GetSquareColorWarning(float* pfParams) const;

        EVENT_STATE GetEventState();

        CNewUIObj* GetOwner() const;
        bool IsVisible() const;
        void ShowInventory();
        void HideInventory();

        bool IsLocked() const;
        void LockInventory();
        void UnlockInventory();

        //. Check Functions
        /* Caution: It's square index, not list index */
        bool GetSquarePosAtPt(int x, int y, int& iColumnX, int& iRowY);

        bool CheckPtInRect(int x, int y);
        bool CheckRectInRect(const RECT& rcBox);
        int GetIndexAtPt(int x, int y);

        int GetIndex(int column, int row);
        bool CanMove(int iLinealPos, ITEM* pItem);
        bool CanMove(int iColumnX, int iRowY, ITEM* pItem);
        bool CanMoveToPt(int x, int y, ITEM* pItem);

        void SetToolTipType(TOOLTIP_TYPE ToolTipType);
        void CreateItemToolTip(ITEM* pItem);
        void DeleteItemToolTip();

        void SetRepairMode(bool bRepair);
        bool IsRepairMode();

        bool AreItemsStackable(ITEM* pSourceItem, ITEM* pTargetItem);
        bool CanPushItem();
        bool CanUpgradeItem(ITEM* pSourceItem, ITEM* pTargetItem);

        static void UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB);

        //. PickedItem Control Functions
        static CNewUIPickedItem* GetPickedItem();
        static bool CreatePickedItem(CNewUIInventoryCtrl* pSrc, ITEM* pItem);
        static void DeletePickedItem();
        static void BackupPickedItem();

        //protected:
        void Render3D();
        void RenderNumberOfItem();
        void RenderItemToolTip();
    };
}

#define g_pPickedItem SEASON3B::CNewUIInventoryCtrl::GetPickedItem()

#endif // !defined(AFX_NEWUIINVENTORYCTRL_H__3A635AF1_3FFA_44B5_9BF6_3DFE0F061927__INCLUDED_)
