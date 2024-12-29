//////////////////////////////////////////////////////////////////////
// NewUIInventoryCtrl.cpp: implementation of the CNewUIInventoryCtrl class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIInventoryCtrl.h"
#include "NewUIItemMng.h"
#include "NewUISystem.h"
#include "ZzzInventory.h"
#include "CComGem.h"
#include "GIPetManager.h"
#include "CSItemOption.h"
#include "SocketSystem.h"
#include "MapManager.h"
#include "MixMgr.h"
using namespace SEASON3B;

SEASON3B::CNewUIPickedItem::CNewUIPickedItem()
{
    m_pNewItemMng = nullptr;
    m_pSrcInventory = nullptr;
    m_pPickedItem = nullptr;
    m_bShow = true;
    m_Pos.x = m_Pos.y = 0;
    m_Size.cx = m_Size.cy = 0;
}

SEASON3B::CNewUIPickedItem::~CNewUIPickedItem()
{
    Release();
}

bool SEASON3B::CNewUIPickedItem::Create(CNewUIItemMng* pNewItemMng, CNewUIInventoryCtrl* pSrc, ITEM* pItem)
{
    if (g_pNewUI3DRenderMng == nullptr || pNewItemMng == nullptr || pItem == nullptr)
        return false;

    m_pNewItemMng = pNewItemMng;
    m_pSrcInventory = pSrc;
    if (nullptr == (m_pPickedItem = m_pNewItemMng->DuplicateItem(pItem)))
    {
        return false;
    }

    g_pNewUI3DRenderMng->Add3DRenderObj(this, INFORMATION_CAMERA_Z_ORDER);

    const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[m_pPickedItem->Type];
    m_Size.cx = pItemAttr->Width * INVENTORY_SQUARE_WIDTH;
    m_Size.cy = pItemAttr->Height * INVENTORY_SQUARE_HEIGHT;
    m_Pos.x = MouseX - m_Size.cx / 2;
    m_Pos.y = MouseY - m_Size.cy / 2;

    return true;
}

void SEASON3B::CNewUIPickedItem::Release()
{
    g_pNewUI3DRenderMng->Remove3DRenderObj(this);
    m_pNewItemMng->DeleteDuplicatedItem(m_pPickedItem);
    m_pPickedItem = nullptr;
    m_pNewItemMng = nullptr;
    m_pSrcInventory = nullptr;
    m_bShow = true;
}

CNewUIInventoryCtrl* SEASON3B::CNewUIPickedItem::GetOwnerInventory() const
{
    return m_pSrcInventory;
}

STORAGE_TYPE CNewUIPickedItem::GetSourceStorageType() const
{
    if (m_pSrcInventory)
    {
        const auto storageType = m_pSrcInventory->GetStorageType();
        if (storageType != STORAGE_TYPE::CHAOS_MIX)
        {
            return storageType;
        }

        return g_MixRecipeMgr.GetMixInventoryEquipmentIndex();
    }

    if (m_pPickedItem && m_pPickedItem->ex_src_type == ITEM_EX_SRC_EQUIPMENT)
    {
        return STORAGE_TYPE::INVENTORY;
    }

    return STORAGE_TYPE::UNDEFINED;
}

ITEM* SEASON3B::CNewUIPickedItem::GetItem() const
{
    return m_pPickedItem;
}

const POINT& SEASON3B::CNewUIPickedItem::GetPos() const
{
    return m_Pos;
}

const SIZE& SEASON3B::CNewUIPickedItem::GetSize() const
{
    return m_Size;
}

void SEASON3B::CNewUIPickedItem::GetRect(RECT& rcBox)
{
    rcBox.left = m_Pos.x;
    rcBox.top = m_Pos.y;
    rcBox.right = rcBox.left + m_Size.cx;
    rcBox.bottom = rcBox.top + m_Size.cy;
}

int SEASON3B::CNewUIPickedItem::GetSourceLinealPos()
{
    if (m_pSrcInventory)
    {
        return m_pSrcInventory->GetIndex(m_pPickedItem->x, m_pPickedItem->y);
    }

    if (m_pPickedItem && m_pPickedItem->ex_src_type > 0)
    {
        return m_pPickedItem->lineal_pos;
    }
    return -1;
}

bool SEASON3B::CNewUIPickedItem::GetTargetPos(CNewUIInventoryCtrl* pDest, int& iTargetColumnX, int& iTargetRowY)
{
    if (pDest != nullptr)
    {
        RECT rcInventory;
        pDest->GetRect(rcInventory);

        const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[m_pPickedItem->Type];
        const int iPickedItemX = MouseX - ((pItemAttr->Width - 1) * INVENTORY_SQUARE_WIDTH / 2);
        const int iPickedItemY = MouseY - ((pItemAttr->Height - 1) * INVENTORY_SQUARE_HEIGHT / 2);

        return pDest->GetSquarePosAtPt(iPickedItemX, iPickedItemY, iTargetColumnX, iTargetRowY);
    }
    return false;
}

int SEASON3B::CNewUIPickedItem::GetTargetLinealPos(CNewUIInventoryCtrl* pDest)
{
    int iTargetColumnX, iTargetRowY;
    if (GetTargetPos(pDest, iTargetColumnX, iTargetRowY))
    {
        return pDest->GetIndex(iTargetColumnX, iTargetRowY);
    }
    return -1;
}

bool SEASON3B::CNewUIPickedItem::IsVisible() const
{
    return m_bShow;
}

void SEASON3B::CNewUIPickedItem::ShowPickedItem()
{
    m_bShow = true;
}

void SEASON3B::CNewUIPickedItem::HidePickedItem()
{
    m_bShow = false;
}

void SEASON3B::CNewUIPickedItem::Render3D()
{
    if (m_pPickedItem && m_pPickedItem->Type >= 0)
    {
        m_Pos.x = MouseX - m_Size.cx / 2;
        m_Pos.y = MouseY - m_Size.cy / 2;
        RenderItem3D(m_Pos.x, m_Pos.y, m_Size.cx, m_Size.cy, m_pPickedItem->Type, m_pPickedItem->Level, m_pPickedItem->ExcellentFlags, m_pPickedItem->AncientDiscriminator, true);
    }
}

CNewUIPickedItem* SEASON3B::CNewUIInventoryCtrl::ms_pPickedItem = nullptr;

SEASON3B::CNewUIInventoryCtrl::CNewUIInventoryCtrl()
{
    Init();
}

SEASON3B::CNewUIInventoryCtrl::~CNewUIInventoryCtrl()
{
    Release();
}

void SEASON3B::CNewUIInventoryCtrl::Init()
{
    m_pNew3DRenderMng = nullptr;
    m_pNewItemMng = nullptr;
    m_pOwner = nullptr;
    m_Pos.x = m_Pos.y = 0;
    m_Size.cx = m_Size.cy = 0;
    m_nColumn = m_nRow = 0;
    m_pdwItemCheckBox = nullptr;
    m_EventState = EVENT_NONE;
    m_iPointedSquareIndex = -1;
    m_bShow = true;
    m_bLock = false;
    m_ToolTipType = TOOLTIP_TYPE_INVENTORY;
    m_pToolTipItem = nullptr;
    m_bRepairMode = false;
    m_bCanPushItem = true;
    Vector(0.1f, 0.4f, 0.8f, m_afColorStateNormal);
    Vector(1.f, 0.2f, 0.2f, m_afColorStateWarning);
}

void SEASON3B::CNewUIInventoryCtrl::LoadImages()
{
    LoadBitmap(L"Interface\\newui_item_box.tga", IMAGE_ITEM_SQUARE);
    LoadBitmap(L"Interface\\newui_item_table01(L).tga", IMAGE_ITEM_TABLE_TOP_LEFT);
    LoadBitmap(L"Interface\\newui_item_table01(R).tga", IMAGE_ITEM_TABLE_TOP_RIGHT);
    LoadBitmap(L"Interface\\newui_item_table02(L).tga", IMAGE_ITEM_TABLE_BOTTOM_LEFT);
    LoadBitmap(L"Interface\\newui_item_table02(R).tga", IMAGE_ITEM_TABLE_BOTTOM_RIGHT);
    LoadBitmap(L"Interface\\newui_item_table03(Up).tga", IMAGE_ITEM_TABLE_TOP_PIXEL);
    LoadBitmap(L"Interface\\newui_item_table03(Dw).tga", IMAGE_ITEM_TABLE_BOTTOM_PIXEL);
    LoadBitmap(L"Interface\\newui_item_table03(L).tga", IMAGE_ITEM_TABLE_LEFT_PIXEL);
    LoadBitmap(L"Interface\\newui_item_table03(R).tga", IMAGE_ITEM_TABLE_RIGHT_PIXEL);

#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
    LoadBitmap(L"Interface\\newui_inven_usebox_01.tga", IMAGE_ITEM_SQUARE_FOR_1_BY_1);
    LoadBitmap(L"Interface\\newui_inven_usebox_02.tga", IMAGE_ITEM_SQUARE_TOP_RECT);
    LoadBitmap(L"Interface\\newui_inven_usebox_03.tga", IMAGE_ITEM_SQUARE_BOTTOM_RECT);
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
}

void SEASON3B::CNewUIInventoryCtrl::UnloadImages()
{
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
    DeleteBitmap(IMAGE_ITEM_SQUARE_BOTTOM_RECT);
    DeleteBitmap(IMAGE_ITEM_SQUARE_TOP_RECT);
    DeleteBitmap(IMAGE_ITEM_SQUARE_FOR_1_BY_1);
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY

    DeleteBitmap(IMAGE_ITEM_TABLE_RIGHT_PIXEL);
    DeleteBitmap(IMAGE_ITEM_TABLE_LEFT_PIXEL);
    DeleteBitmap(IMAGE_ITEM_TABLE_BOTTOM_PIXEL);
    DeleteBitmap(IMAGE_ITEM_TABLE_TOP_PIXEL);
    DeleteBitmap(IMAGE_ITEM_TABLE_BOTTOM_RIGHT);
    DeleteBitmap(IMAGE_ITEM_TABLE_BOTTOM_LEFT);
    DeleteBitmap(IMAGE_ITEM_TABLE_TOP_RIGHT);
    DeleteBitmap(IMAGE_ITEM_TABLE_TOP_LEFT);
    DeleteBitmap(IMAGE_ITEM_SQUARE);
}

void SEASON3B::CNewUIInventoryCtrl::SetItemColorState(ITEM* pItem)
{
    if (pItem == nullptr)
    {
        return;
    }

    if (pItem->byColorState == ITEM_COLOR_TRADE_WARNING)
    {
        return;
    }

    ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
    const int iLevel = pItem->Level;
    const int iMaxDurability = CalcMaxDurability(pItem, pItemAttr, iLevel);

    if (pItem->Durability <= 0)
    {
        pItem->byColorState = ITEM_COLOR_DURABILITY_100;
    }
    else if (pItem->Durability <= (iMaxDurability * 0.2f))
    {
        pItem->byColorState = ITEM_COLOR_DURABILITY_80;
    }
    else if (pItem->Durability <= (iMaxDurability * 0.3f))
    {
        pItem->byColorState = ITEM_COLOR_DURABILITY_70;
    }
    else if (pItem->Durability <= (iMaxDurability * 0.5f))
    {
        pItem->byColorState = ITEM_COLOR_DURABILITY_50;
    }
    else
    {
        pItem->byColorState = ITEM_COLOR_NORMAL;
    }
}

bool SEASON3B::CNewUIInventoryCtrl::CanChangeItemColorState(ITEM* pItem)
{
    if (pItem == nullptr)
    {
        return false;
    }

    if (pItem->Type < ITEM_WING)
    {
        return true;
    }

    if (pItem->Type == ITEM_BOLT || pItem->Type == ITEM_ARROWS)
    {
        return false;
    }

    if (pItem->Type == ITEM_WIZARDS_RING && (pItem->Level == 1 || pItem->Level  == 2))
    {
        return false;
    }

    if (pItem->Type >= ITEM_RING_OF_ICE && pItem->Type <= ITEM_RING_OF_POISON
        || pItem->Type == ITEM_TRANSFORMATION_RING
        || pItem->Type >= ITEM_PENDANT_OF_LIGHTING && pItem->Type <= ITEM_PENDANT_OF_FIRE
        || pItem->Type == ITEM_WIZARDS_RING
        || pItem->Type >= ITEM_RING_OF_FIRE && pItem->Type <= ITEM_PENDANT_OF_ABILITY
        || pItem->Type >= ITEM_MOONSTONE_PENDANT && pItem->Type <= ITEM_GAME_MASTER_TRANSFORMATION_RING
#ifdef PJH_ADD_PANDA_CHANGERING
        || pItem->Type == ITEM_PANDA_TRANSFORMATION_RING
#endif //PJH_ADD_PANDA_CHANGERING
        || pItem->Type == ITEM_SKELETON_TRANSFORMATION_RING
        || pItem->Type == ITEM_PET_PANDA
        || pItem->Type == ITEM_DEMON
        || pItem->Type == ITEM_SPIRIT_OF_GUARDIAN
        || pItem->Type == ITEM_PET_SKELETON
        || pItem->Type == ITEM_HELPER + 107
        || pItem->Type == ITEM_HELPER + 109
        || pItem->Type == ITEM_HELPER + 110
        || pItem->Type == ITEM_HELPER + 111
        || pItem->Type == ITEM_HELPER + 112
        || pItem->Type == ITEM_HELPER + 113
        || pItem->Type == ITEM_HELPER + 114
        || pItem->Type == ITEM_HELPER + 115
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
        || g_pMyInventory->IsInvenItem(pItem->Type)
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
        )
    {
        return true;
    }

    if (pItem->Type >= ITEM_HELPER && pItem->Type <= ITEM_DARK_RAVEN_ITEM || pItem->Type == ITEM_HORN_OF_FENRIR || pItem->Type == ITEM_PET_UNICORN)
    {
        return true;
    }

    if (IsWingItem(pItem) == true)
    {
        return true;
    }

    return false;
}

bool SEASON3B::CNewUIInventoryCtrl::Create(STORAGE_TYPE storageType, CNewUI3DRenderMng* pNew3DRenderMng, CNewUIItemMng* pNewItemMng,
    CNewUIObj* pOwner, int x, int y, int nColumn, int nRow, int nIndexOffset)
{
    m_StorageType = storageType;
    m_nIndexOffset = nIndexOffset;
    if (m_pdwItemCheckBox || false == m_vecItem.empty())
        return false;
    if (pNew3DRenderMng == nullptr || pNewItemMng == nullptr)
        return false;

    m_pNew3DRenderMng = pNew3DRenderMng;
    m_pNew3DRenderMng->Add3DRenderObj(this, INVENTORY_CAMERA_Z_ORDER);

    m_pNewItemMng = pNewItemMng;
    m_pOwner = pOwner;
    m_Pos.x = x;
    m_Pos.y = y;
    m_Size.cx = nColumn * INVENTORY_SQUARE_WIDTH;
    m_Size.cy = nRow * INVENTORY_SQUARE_HEIGHT;
    m_nColumn = nColumn;
    m_nRow = nRow;
    m_pdwItemCheckBox = new DWORD[nColumn * nRow];
    memset(m_pdwItemCheckBox, 0, sizeof(DWORD) * m_nColumn * m_nRow);

    LoadImages();

    if (m_StorageType == STORAGE_TYPE::UNDEFINED)
    {
        LockInventory();
    }

    return true;
}
void SEASON3B::CNewUIInventoryCtrl::Release()
{
    if (m_pNew3DRenderMng)
        m_pNew3DRenderMng->DeleteUI2DEffectObject(UI2DEffectCallback);

    RemoveAllItems();
    UnloadImages();

    SAFE_DELETE(m_pdwItemCheckBox);

    if (m_pNew3DRenderMng)
        m_pNew3DRenderMng->Remove3DRenderObj(this);

    Init();
}

bool SEASON3B::CNewUIInventoryCtrl::AddItem(int iLinealPos, std::span<const BYTE> itemData)
{
    iLinealPos -= m_nIndexOffset;
    if (iLinealPos < 0 || iLinealPos >= m_nColumn * m_nRow)
        return false;

    const int iColumnX = iLinealPos % m_nColumn;
    const int iRowY = iLinealPos / m_nColumn;

    return AddItem(iColumnX, iRowY, itemData);
}

bool SEASON3B::CNewUIInventoryCtrl::AddItem(int iColumnX, int iRowY, std::span<const BYTE> itemData)
{
    if (iColumnX < 0 || iRowY < 0 ||
        iColumnX >= m_nColumn || iRowY >= m_nRow)
    {
        return false;
    }

    ITEM* pNewItem = m_pNewItemMng->CreateItem(itemData);
    if (nullptr == pNewItem)
        return false;

    if (!CanMove(iColumnX, iRowY, pNewItem))
    {
        m_pNewItemMng->DeleteItem(pNewItem);
        return false;
    }

    const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pNewItem->Type];
    pNewItem->x = iColumnX;
    pNewItem->y = iRowY;

    for (int y = 0; y < pItemAttr->Height; y++)
    {
        for (int x = 0; x < pItemAttr->Width; x++)
        {
            const int iCurIndex = (pNewItem->y + y) * m_nColumn + (pNewItem->x + x);
            m_pdwItemCheckBox[iCurIndex] = pNewItem->Key;
        }
    }
    m_vecItem.push_back(pNewItem);

    return true;
}

bool SEASON3B::CNewUIInventoryCtrl::AddItem(int iColumnX, int iRowY, ITEM* pItem)
{
    if (iColumnX < 0 || iRowY < 0 ||
        iColumnX >= m_nColumn || iRowY >= m_nRow) return false;

    ITEM* pNewItem = m_pNewItemMng->CreateItem(pItem);
    if (nullptr == pNewItem)
        return false;

    if (!CanMove(iColumnX, iRowY, pNewItem))
    {
        m_pNewItemMng->DeleteItem(pNewItem);
        return false;
    }

    const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pNewItem->Type];
    pNewItem->x = iColumnX;
    pNewItem->y = iRowY;

    for (int y = 0; y < pItemAttr->Height; y++)
    {
        for (int x = 0; x < pItemAttr->Width; x++)
        {
            const int iCurIndex = (pNewItem->y + y) * m_nColumn + (pNewItem->x + x);
            m_pdwItemCheckBox[iCurIndex] = pNewItem->Key;
        }
    }
    m_vecItem.push_back(pNewItem);
    return true;
}

bool SEASON3B::CNewUIInventoryCtrl::AddItem(int iColumnX, int iRowY, BYTE byType, BYTE bySubType, BYTE byLevel, BYTE byDurability, BYTE byOption1, BYTE byOptionEx, BYTE byOption380, BYTE byOptionHarmony)
{
    if (iColumnX < 0 || iRowY < 0 ||
        iColumnX >= m_nColumn || iRowY >= m_nRow) return false;

    ITEM* pNewItem = m_pNewItemMng->CreateItem(byType, bySubType, byLevel, byDurability,
        byOption1, byOptionEx, byOption380, byOptionHarmony);
    if (nullptr == pNewItem)
        return false;

    if (!CanMove(iColumnX, iRowY, pNewItem))
    {
        m_pNewItemMng->DeleteItem(pNewItem);
        return false;
    }

    const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pNewItem->Type];
    pNewItem->x = iColumnX;
    pNewItem->y = iRowY;

    for (int y = 0; y < pItemAttr->Height; y++)
    {
        for (int x = 0; x < pItemAttr->Width; x++)
        {
            const int iCurIndex = (pNewItem->y + y) * m_nColumn + (pNewItem->x + x);
            m_pdwItemCheckBox[iCurIndex] = pNewItem->Key;
        }
    }
    m_vecItem.push_back(pNewItem);
    return true;
}

void SEASON3B::CNewUIInventoryCtrl::RemoveItem(ITEM* pItem)
{
    auto li = m_vecItem.begin();
    for (; li != m_vecItem.end(); ++li)
    {
        if ((*li) == pItem)
        {
            m_vecItem.erase(li);

            const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
            for (int y = 0; y < pItemAttr->Height; y++)
            {
                for (int x = 0; x < pItemAttr->Width; x++)
                {
                    const int iCurIndex = (pItem->y + y) * m_nColumn + (pItem->x + x);
                    m_pdwItemCheckBox[iCurIndex] = 0;
                }
            }
            m_pNewItemMng->DeleteItem(pItem);
            break;
        }
    }
}

void SEASON3B::CNewUIInventoryCtrl::RemoveAllItems()
{
    memset(m_pdwItemCheckBox, 0, sizeof(DWORD) * m_nColumn * m_nRow);

    auto li = m_vecItem.begin();
    for (; li != m_vecItem.end(); ++li)
    {
        ITEM* pItem = (*li);
        m_pNewItemMng->DeleteItem(pItem);
    }

    m_vecItem.clear();
}

size_t SEASON3B::CNewUIInventoryCtrl::GetNumberOfItems()
{
    return m_vecItem.size();
}

ITEM* SEASON3B::CNewUIInventoryCtrl::GetItem(int iIndex)
{
    if (iIndex < 0 || iIndex >= static_cast<int>(m_vecItem.size()))
        return nullptr;
    return m_vecItem[iIndex];
}

void SEASON3B::CNewUIInventoryCtrl::SetSquareColorNormal(float fRed, float fGreen, float fBlue)
{
    Vector(fRed, fGreen, fBlue, m_afColorStateNormal);
}

void SEASON3B::CNewUIInventoryCtrl::GetSquareColorNormal(float* pfParams) const
{
    Vector(m_afColorStateNormal[0], m_afColorStateNormal[1], m_afColorStateNormal[2], pfParams);
}

void SEASON3B::CNewUIInventoryCtrl::SetSquareColorWarning(float fRed, float fGreen, float fBlue)
{
    Vector(fRed, fGreen, fBlue, m_afColorStateWarning);
}

void SEASON3B::CNewUIInventoryCtrl::GetSquareColorWarning(float* pfParams) const
{
    Vector(m_afColorStateWarning[0], m_afColorStateWarning[1], m_afColorStateWarning[2], pfParams);
}

ITEM* SEASON3B::CNewUIInventoryCtrl::FindItem(int iLinealPos)
{
    iLinealPos -= m_nIndexOffset;
    if (iLinealPos >= 0 && iLinealPos < m_nColumn * m_nRow)
    {
        const DWORD dwKey = m_pdwItemCheckBox[iLinealPos];
        return FindItemByKey(dwKey);
    }
    return nullptr;
}

ITEM* SEASON3B::CNewUIInventoryCtrl::FindItem(int iColumnX, int iRowY)
{
    return FindItem(iRowY * m_nColumn + iColumnX + m_nIndexOffset);
}

ITEM* SEASON3B::CNewUIInventoryCtrl::FindItemByKey(DWORD dwKey)
{
    auto li = m_vecItem.begin();
    for (; li != m_vecItem.end(); ++li)
        if ((*li)->Key == dwKey)
            return (*li);
    return nullptr;
}

ITEM* SEASON3B::CNewUIInventoryCtrl::FindTypeItem(short int siType)
{
    auto li = m_vecItem.begin();
    for (; li != m_vecItem.end(); ++li)
        if ((*li)->Type == siType)
            return (*li);
    return nullptr;
}

bool SEASON3B::CNewUIInventoryCtrl::IsItem(short int siType)
{
    auto li = m_vecItem.begin();
    for (; li != m_vecItem.end(); ++li)
        if ((*li)->Type == siType)
            return true;
    return false;
}

int SEASON3B::CNewUIInventoryCtrl::GetItemCount(short int siType, int iLevel)
{
    int count = 0;
    auto li = m_vecItem.begin();
    for (; li != m_vecItem.end(); ++li) {
        if ((*li)->Type == siType) {
            if (iLevel == -1 || (*li)->Level == iLevel)
            {
                count += ((*li)->Durability == 0) ? 1 : (*li)->Durability;
            }
        }
    }
    return count;
}

int SEASON3B::CNewUIInventoryCtrl::FindItemIndex(short int siType, int iLevel)
{
    auto li = m_vecItem.begin();
    for (; li != m_vecItem.end(); ++li)
    {
        if ((*li)->Type == siType)
        {
            if (iLevel == -1 || (*li)->Level == iLevel)
            {
                return (*li)->y * GetNumberOfColumn() + (*li)->x + m_nIndexOffset;
            }
        }
    }

    return -1;
}

int SEASON3B::CNewUIInventoryCtrl::FindItemReverseIndex(short sType, int iLevel)
{
    for (int x = m_nColumn - 1; x >= 0; x--)
    {
        for (int y = m_nRow - 1; y >= 0; y--)
        {
            const ITEM* pItem = FindItem(x, y);

            if (pItem)
            {
                if (pItem->Type == sType)
                {
                    if (iLevel == -1 || pItem->Level == iLevel)
                    {
                        return (pItem->y * GetNumberOfColumn()) + pItem->x + m_nIndexOffset;
                    }
                }
            }
        }
    }

    return -1;
}

int SEASON3B::CNewUIInventoryCtrl::GetIndexByItem(ITEM* pItem)
{
    if (pItem == nullptr)
    {
        return -1;
    }

    return this->GetIndex(pItem->x, pItem->y);
}

ITEM* SEASON3B::CNewUIInventoryCtrl::FindItemPointedSquareIndex()
{
    if (m_iPointedSquareIndex != -1)
    {
        ITEM* pItem = nullptr;
        pItem = FindItemByKey(m_pdwItemCheckBox[m_iPointedSquareIndex - m_nIndexOffset]);
        return pItem;
    }

    return nullptr;
}

int SEASON3B::CNewUIInventoryCtrl::GetPointedSquareIndex()
{
    return m_iPointedSquareIndex;
}

ITEM* SEASON3B::CNewUIInventoryCtrl::FindItemAtPt(int x, int y)
{
    const int iIndex = GetIndexAtPt(x, y);
    return FindItem(iIndex);
}

int SEASON3B::CNewUIInventoryCtrl::FindEmptySlot(IN int cx, IN int cy)
{
    for (int i = 0; i < m_nColumn * m_nRow; i++)
    {
        if (CheckSlot(i, cx, cy) && (i % m_nColumn < (m_nColumn - (cx - 1))))
        {
            return i + m_nIndexOffset;
        }
    }

    return -1;
}
bool SEASON3B::CNewUIInventoryCtrl::FindEmptySlot(IN int cx, IN int cy, OUT int& iColumnX, OUT int& iColumnY)
{
    for (int y = 0; y < m_nRow; y++)
    {
        for (int x = 0; x < m_nColumn; x++)
        {
            if (CheckSlot(x, y, cx, cy))
            {
                iColumnX = x;
                iColumnY = y;
                return true;
            }
        }
    }
    return false;
}

int SEASON3B::CNewUIInventoryCtrl::GetNumItemByKey(DWORD dwItemKey)
{
    int iCntItem = 0;
    for (int y = 0; y < m_nRow; y++)
    {
        for (int x = 0; x < m_nColumn; x++)
        {
            const ITEM* pItem = nullptr;
            pItem = FindItem(x, y);
            if (pItem == nullptr)
                return 0;

            if (pItem->Key == dwItemKey)
            {
                iCntItem++;
            }
        }
    }

    return iCntItem;
}

int SEASON3B::CNewUIInventoryCtrl::GetNumItemByType(short sItemType)
{
    int iCntItem = 0;
    for (int y = 0; y < m_nRow; y++)
    {
        for (int x = 0; x < m_nColumn; x++)
        {
            const ITEM* pItem = nullptr;
            pItem = FindItem(x, y);
            if (pItem)
            {
                if (pItem->Type == sItemType)
                {
                    iCntItem++;
                }
            }
        }
    }

    return iCntItem;
}

int SEASON3B::CNewUIInventoryCtrl::GetEmptySlotCount()
{
    int iResult = 0;
    for (int y = 0; y < m_nRow; y++)
    {
        for (int x = 0; x < m_nColumn; x++)
        {
            const int iIndex = y * m_nColumn + x;
            if (m_pdwItemCheckBox[iIndex] == 0)
            {
                ++iResult;
            }
        }
    }
    return iResult;
}

bool SEASON3B::CNewUIInventoryCtrl::UpdateMouseEvent()
{
    if (m_EventState == EVENT_NONE
        && SEASON3B::IsNone(VK_LBUTTON)
        && m_iPointedSquareIndex != -1)
    {
        m_EventState = EVENT_HOVER;
    }
    else if (m_EventState == EVENT_HOVER
        && SEASON3B::IsRelease(VK_LBUTTON)
        && m_iPointedSquareIndex != -1
        && nullptr == GetPickedItem()
        && false == IsLocked()
        && m_bRepairMode == false
        )
    {
        m_EventState = EVENT_PICKING;
        ITEM* pItem = FindItemByKey(m_pdwItemCheckBox[m_iPointedSquareIndex - m_nIndexOffset]);
        if (pItem)
        {
            if (CreatePickedItem(this, pItem))
            {
                RemoveItem(pItem);
                return false;
            }
        }
    }
    else if (m_EventState == EVENT_HOVER
        && SEASON3B::IsNone(VK_LBUTTON)
        && m_iPointedSquareIndex != -1
        && nullptr == GetPickedItem()
        && (m_pdwItemCheckBox[m_iPointedSquareIndex - m_nIndexOffset] > 1) && g_pNewUIMng)
    {
        ITEM* pItem = FindItemByKey(m_pdwItemCheckBox[m_iPointedSquareIndex - m_nIndexOffset]);
        if (pItem != m_pToolTipItem)
        {
            CreateItemToolTip(pItem);

            if ((pItem->Type == ITEM_DARK_HORSE_ITEM) || (pItem->Type == ITEM_DARK_RAVEN_ITEM))
            {
                const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[m_pToolTipItem->Type];
                const int iTargetX = m_Pos.x + m_pToolTipItem->x * INVENTORY_SQUARE_WIDTH + pItemAttr->Width * INVENTORY_SQUARE_WIDTH / 2;
                const int iTargetY = m_Pos.y + m_pToolTipItem->y * INVENTORY_SQUARE_HEIGHT;
                giPetManager::RequestPetInfo(iTargetX, iTargetY, pItem);
            }
        }
    }
    return true;
}

bool SEASON3B::CNewUIInventoryCtrl::Update()
{
    if (IsVisible())
    {
        UpdateProcess();
    }
    return true;
}

void SEASON3B::CNewUIInventoryCtrl::UpdateProcess()
{
    const int iCurSquareIndex = GetIndexAtPt(MouseX, MouseY);
    if (iCurSquareIndex != m_iPointedSquareIndex)
    {
        if ((GetPickedItem() == nullptr) && (g_pMyShopInventory->IsEnableInputValueTextBox() == false))
            giPetManager::InitItemBackup();
        m_iPointedSquareIndex = iCurSquareIndex;
    }

    if (m_iPointedSquareIndex == -1
        || (m_iPointedSquareIndex != -1 && m_pdwItemCheckBox[m_iPointedSquareIndex - m_nIndexOffset] == 0))
    {
        m_EventState = EVENT_NONE;
        DeleteItemToolTip();
    }
}

void SEASON3B::CNewUIInventoryCtrl::Render()
{
    int x, y;
    for (y = 0; y < m_nRow; y++)
    {
        for (x = 0; x < m_nColumn; x++)
        {
            const int iCurSquareIndex = y * m_nColumn + x;

            if (m_pdwItemCheckBox[iCurSquareIndex] > 1)
            {
                EnableAlphaTest();

                ITEM* pItem = FindItemByKey(m_pdwItemCheckBox[iCurSquareIndex]);

                if (pItem)
                {
                    if (CanChangeItemColorState(pItem) == true)
                    {
                        SetItemColorState(pItem);
                    }

                    if (pItem->byColorState == ITEM_COLOR_NORMAL)
                    {
                        glColor4f(0.3f, 0.5f, 0.5f, 0.6f);
                    }
                    else if (pItem->byColorState == ITEM_COLOR_DURABILITY_50)
                    {
                        glColor4f(1.0f, 1.0f, 0.f, 0.4f);
                    }
                    else if (pItem->byColorState == ITEM_COLOR_DURABILITY_70)
                    {
                        glColor4f(1.0f, 0.66f, 0.f, 0.4f);
                    }
                    else if (pItem->byColorState == ITEM_COLOR_DURABILITY_80)
                    {
                        glColor4f(1.0f, 0.33f, 0.f, 0.4f);
                    }
                    else if (pItem->byColorState == ITEM_COLOR_DURABILITY_100)
                    {
                        glColor4f(1.0f, 0.f, 0.f, 0.4f);
                    }
                    else if (pItem->byColorState == ITEM_COLOR_TRADE_WARNING)
                    {
                        glColor4f(1.0f, 0.2f, 0.1f, 0.4f);
                    }
                }
                else
                {
                    glColor4f(0.3f, 0.5f, 0.5f, 0.6f);
                }

                RenderColor(m_Pos.x + (x * INVENTORY_SQUARE_WIDTH), m_Pos.y + (y * INVENTORY_SQUARE_HEIGHT), INVENTORY_SQUARE_WIDTH, INVENTORY_SQUARE_HEIGHT);
                EndRenderColor();
            }

            EnableAlphaTest();
            RenderImage(IMAGE_ITEM_SQUARE, m_Pos.x + (x * INVENTORY_SQUARE_WIDTH), m_Pos.y + (y * INVENTORY_SQUARE_HEIGHT), 21, 21);
        }
    }

    EnableAlphaTest();
    RenderImage(IMAGE_ITEM_TABLE_TOP_LEFT, m_Pos.x - WND_LEFT_EDGE, m_Pos.y - WND_TOP_EDGE, 14, 14);
    RenderImage(IMAGE_ITEM_TABLE_TOP_RIGHT, m_Pos.x + m_Size.cx - WND_RIGHT_EDGE, m_Pos.y - WND_TOP_EDGE, 14, 14);
    RenderImage(IMAGE_ITEM_TABLE_BOTTOM_LEFT, m_Pos.x - WND_LEFT_EDGE, m_Pos.y + m_Size.cy - WND_BOTTOM_EDGE, 14, 14);
    RenderImage(IMAGE_ITEM_TABLE_BOTTOM_RIGHT, m_Pos.x + m_Size.cx - WND_RIGHT_EDGE, m_Pos.y + m_Size.cy - WND_BOTTOM_EDGE, 14, 14);

    for (x = m_Pos.x - WND_LEFT_EDGE + 14; x < m_Pos.x + m_Size.cx - WND_RIGHT_EDGE; x++)
    {
        RenderImage(IMAGE_ITEM_TABLE_TOP_PIXEL, x, m_Pos.y - WND_TOP_EDGE, 1, 14);
        RenderImage(IMAGE_ITEM_TABLE_BOTTOM_PIXEL, x, m_Pos.y + m_Size.cy - WND_BOTTOM_EDGE, 1, 14);
    }
    for (y = m_Pos.y - WND_TOP_EDGE + 14; y < m_Pos.y + m_Size.cy - WND_BOTTOM_EDGE; y++)
    {
        RenderImage(IMAGE_ITEM_TABLE_LEFT_PIXEL, m_Pos.x - WND_LEFT_EDGE, y, 14, 1);
        RenderImage(IMAGE_ITEM_TABLE_RIGHT_PIXEL, m_Pos.x + m_Size.cx - WND_RIGHT_EDGE, y, 14, 1);
    }

    if (ms_pPickedItem)
    {
        const bool pickitemvisible = ms_pPickedItem->IsVisible();

        if (pickitemvisible)
        {
            RECT rcPickedItem, rcInventory, rcIntersect;
            ms_pPickedItem->GetRect(rcPickedItem);
            GetRect(rcInventory);

            if (IntersectRect(&rcIntersect, &rcPickedItem, &rcInventory))
            {
                ITEM* pPickItem = ms_pPickedItem->GetItem();
                const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pPickItem->Type];
                const int iPickedItemX = MouseX - ((pItemAttr->Width - 1) * INVENTORY_SQUARE_WIDTH / 2);
                const int iPickedItemY = MouseY - ((pItemAttr->Height - 1) * INVENTORY_SQUARE_HEIGHT / 2);

                int iColumnX = 0, iRowY = 0;
                int nItemColumn = pItemAttr->Width, nItemRow = pItemAttr->Height;
                if (false == GetSquarePosAtPt(iPickedItemX, iPickedItemY, iColumnX, iRowY))
                {
                    iColumnX = ((iPickedItemX - rcInventory.left) / INVENTORY_SQUARE_WIDTH);

                    if (iPickedItemX - rcInventory.left < 0)
                        iColumnX = ((iPickedItemX - rcInventory.left) / INVENTORY_SQUARE_WIDTH) - 1;
                    else
                        iColumnX = (iPickedItemX - rcInventory.left) / INVENTORY_SQUARE_WIDTH;

                    if (iPickedItemY - rcInventory.top < 0)
                        iRowY = ((iPickedItemY - rcInventory.top) / INVENTORY_SQUARE_HEIGHT) - 1;
                    else
                        iRowY = (iPickedItemY - rcInventory.top) / INVENTORY_SQUARE_HEIGHT;
                }

                bool bWarning = false;
                //. Clipping
                if (iColumnX < 0 && iColumnX >= -nItemColumn)
                {
                    nItemColumn = nItemColumn + iColumnX;
                    iColumnX = 0;
                    bWarning = true;
                }
                if (iColumnX + nItemColumn > m_nColumn && iColumnX < m_nColumn)
                {
                    nItemColumn = m_nColumn - iColumnX;
                    bWarning = true;
                }
                if (iRowY < 0 && iRowY >= -nItemRow)
                {
                    nItemRow = nItemRow + iRowY;
                    iRowY = 0;
                    bWarning = true;
                }
                if (iRowY + nItemRow > m_nRow && iRowY < m_nRow)
                {
                    nItemRow = m_nRow - iRowY;
                    bWarning = true;
                }

                const int iDestPosX = m_Pos.x + iColumnX * INVENTORY_SQUARE_WIDTH;
                const int iDestPosY = m_Pos.y + iRowY * INVENTORY_SQUARE_HEIGHT;
                const int iDestWidth = nItemColumn * INVENTORY_SQUARE_WIDTH;
                const int iDestHeight = nItemRow * INVENTORY_SQUARE_HEIGHT;

                m_bCanPushItem = bWarning;

                //. Rendering Routine
                if (bWarning)
                {
                    EnableAlphaTest();
                    glColor4f(1.f, 0.2f, 0.2f, 0.4f);
                    RenderColor(iDestPosX, iDestPosY, iDestWidth, iDestHeight);
                    EndRenderColor();
                }
                else
                {
                    if (iColumnX >= 0 && iColumnX < m_nColumn && iRowY >= 0 && iRowY < m_nRow)
                    {
                        EnableAlphaTest();
                        for (int y = 0; y < nItemRow; y++)
                        {
                            for (int x = 0; x < nItemColumn; x++)
                            {
                                const int iSquarePosX = iColumnX + x;
                                const int iSquarePosY = iRowY + y;
                                const int iCurSquareIndex = iSquarePosY * m_nColumn + iSquarePosX;
                                if (m_pdwItemCheckBox[iCurSquareIndex] > 1)
                                {
                                    bool bSuccess = false;

                                    ITEM* pTargetItem = FindItemByKey(m_pdwItemCheckBox[iCurSquareIndex]);
                                    if (pTargetItem)
                                    {
                                        const int	iType = pTargetItem->Type;
                                        const int	iDurability = pTargetItem->Durability;

                                        if ((pPickItem->Type == ITEM_JEWEL_OF_BLESS) || (pPickItem->Type == ITEM_JEWEL_OF_SOUL))
                                        {
                                            bSuccess = CanUpgradeItem(pPickItem, pTargetItem);
                                        }
                                        else if (pPickItem->Type == ITEM_JEWEL_OF_HARMONY)
                                        {
                                            if (pTargetItem->Jewel_Of_Harmony_Option == 0)
                                            {
                                                const StrengthenItem strengthitem = g_pUIJewelHarmonyinfo->GetItemType(static_cast<int>(pTargetItem->Type));

                                                if ((strengthitem != SI_None) && (!g_SocketItemMgr.IsSocketItem(pTargetItem))
                                                    && (pTargetItem->AncientDiscriminator > 0))
                                                {
                                                    bSuccess = true;
                                                }
                                            }
                                        }
                                        else if (pPickItem->Type == ITEM_LOWER_REFINE_STONE || pPickItem->Type == ITEM_HIGHER_REFINE_STONE)
                                        {
                                            if (pTargetItem->Jewel_Of_Harmony_Option != 0)
                                            {
                                                bSuccess = true;
                                            }
                                        }

                                        if (pPickItem->Type == ITEM_JEWEL_OF_BLESS && iType == ITEM_HORN_OF_FENRIR && iDurability != 255)
                                        {
                                            bSuccess = true;
                                        }

                                        if (bSuccess == false && m_pOwner == g_pMyInventory)
                                        {
                                            bSuccess = AreItemsStackable(pPickItem, pTargetItem);
                                        }
                                        if (Check_LuckyItem(pTargetItem->Type))
                                        {
                                            bSuccess = false;
                                            if (pPickItem->Type == ITEM_POTION + 161)
                                            {
                                                if (pTargetItem->Jewel_Of_Harmony_Option == 0)	bSuccess = true;
                                            }
                                            else if (pPickItem->Type == ITEM_POTION + 160)
                                            {
                                                if (pTargetItem->Durability > 0)				bSuccess = true;
                                            }
                                        }
                                    }

                                    if (bSuccess)
                                    {
                                        SetSquareColorWarning(0.2f, 0.4f, 0.2f);
                                    }
                                    else
                                    {
                                        SetSquareColorWarning(1.f, 0.2f, 0.2f);
                                    }

                                    glColor4f(m_afColorStateWarning[0], m_afColorStateWarning[1], m_afColorStateWarning[2], 0.4f);
                                }
                                else
                                {
                                    glColor4f(m_afColorStateNormal[0], m_afColorStateNormal[1], m_afColorStateNormal[2], 0.4f);
                                }
                                RenderColor(m_Pos.x + (iSquarePosX * INVENTORY_SQUARE_WIDTH), m_Pos.y + (iSquarePosY * INVENTORY_SQUARE_HEIGHT), INVENTORY_SQUARE_WIDTH, INVENTORY_SQUARE_HEIGHT);
                            }
                        }
                        EndRenderColor();
                    }
                }
            }
        }
    }

    const bool tooltipvisible = true;

    if (m_pNew3DRenderMng)
    {
        m_pNew3DRenderMng->RenderUI2DEffect(INVENTORY_CAMERA_Z_ORDER, UI2DEffectCallback, this, RENDER_NUMBER_OF_ITEM, 0);
        if (m_pToolTipItem && GetPickedItem() == nullptr)
        {
            if (tooltipvisible)
            {
                m_pNew3DRenderMng->RenderUI2DEffect(INVENTORY_CAMERA_Z_ORDER, UI2DEffectCallback, this, RENDER_ITEM_TOOLTIP, 0);
            }
        }
    }
}

void SEASON3B::CNewUIInventoryCtrl::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

const POINT& SEASON3B::CNewUIInventoryCtrl::GetPos() const
{
    return m_Pos;
}

int SEASON3B::CNewUIInventoryCtrl::GetNumberOfColumn() const
{
    return m_nColumn;
}

int SEASON3B::CNewUIInventoryCtrl::GetNumberOfRow() const
{
    return m_nRow;
}

void SEASON3B::CNewUIInventoryCtrl::GetRect(RECT& rcBox)
{
    rcBox.left = m_Pos.x;
    rcBox.top = m_Pos.y;
    rcBox.right = rcBox.left + m_Size.cx;
    rcBox.bottom = rcBox.top + m_Size.cy;
}

CNewUIInventoryCtrl::EVENT_STATE SEASON3B::CNewUIInventoryCtrl::GetEventState()
{
    return m_EventState;
}

CNewUIObj* SEASON3B::CNewUIInventoryCtrl::GetOwner() const
{
    return m_pOwner;
}

bool SEASON3B::CNewUIInventoryCtrl::IsVisible() const
{
    if (m_pOwner)
        return (m_pOwner->IsVisible() && m_bShow);
    return m_bShow;
}

void SEASON3B::CNewUIInventoryCtrl::ShowInventory()
{
    m_bShow = true;
}

void SEASON3B::CNewUIInventoryCtrl::HideInventory()
{
    m_bShow = false;
}

bool SEASON3B::CNewUIInventoryCtrl::IsLocked() const
{
    return m_bLock;
}

void SEASON3B::CNewUIInventoryCtrl::LockInventory()
{
    m_bLock = true;
}

void SEASON3B::CNewUIInventoryCtrl::UnlockInventory()
{
    m_bLock = false;
}

int SEASON3B::CNewUIInventoryCtrl::GetIndexAtPt(int x, int y)
{
    int iColumnX, iRowY;
    if (GetSquarePosAtPt(x, y, iColumnX, iRowY))
        return iRowY * m_nColumn + iColumnX + m_nIndexOffset;
    return -1;
}

bool SEASON3B::CNewUIInventoryCtrl::GetSquarePosAtPt(int x, int y, int& iColumnX, int& iRowY)
{
    RECT rcBox;
    GetRect(rcBox);

    if (x < rcBox.left || x >= rcBox.right || y < rcBox.top || y >= rcBox.bottom)
        return false;

    iColumnX = (x - rcBox.left) / INVENTORY_SQUARE_WIDTH;
    iRowY = (y - rcBox.top) / INVENTORY_SQUARE_HEIGHT;

    return true;
}

bool SEASON3B::CNewUIInventoryCtrl::CheckSlot(int startIndex, int width, int height)
{
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            const int iIndex = startIndex + (y * m_nColumn) + x;

            if (iIndex >= (m_nColumn * m_nRow))
            {
                return false;
            }

            if (m_pdwItemCheckBox[iIndex] != 0)
            {
                return false;
            }
        }
    }

    return true;
}

bool SEASON3B::CNewUIInventoryCtrl::CheckSlot(int iColumnX, int iRowY, int width, int height)
{
    const int iIndex = iRowY * m_nColumn + iColumnX;
    return CheckSlot(iIndex, width, height);
}

int CNewUIInventoryCtrl::GetIndex(int column, int row)
{
    return column + row * m_nColumn + m_nIndexOffset;
}

bool SEASON3B::CNewUIInventoryCtrl::CheckPtInRect(int x, int y)
{
    RECT rcSquare;
    GetRect(rcSquare);

    if (x < rcSquare.left || x >= rcSquare.right || y < rcSquare.top || y >= rcSquare.bottom)
        return false;
    return true;
}

bool SEASON3B::CNewUIInventoryCtrl::CheckRectInRect(const RECT& rcBox)
{
    RECT rcSquare;
    GetRect(rcSquare);

    if (rcBox.left >= rcSquare.left && rcBox.right <= rcSquare.right
        && rcBox.top >= rcSquare.top && rcBox.bottom <= rcSquare.bottom)
        return true;
    return false;
}

bool SEASON3B::CNewUIInventoryCtrl::CanMove(int iLinealPos, ITEM* pItem)
{
    const auto startIndex = iLinealPos - m_nIndexOffset;
    if (startIndex < 0 || startIndex >= m_nColumn * m_nRow)
    {
        return false;
    }

    const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
    return CheckSlot(startIndex, pItemAttr->Width, pItemAttr->Height);
}

bool SEASON3B::CNewUIInventoryCtrl::CanMove(int iColumnX, int iRowY, ITEM* pItem)
{
    const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
    return CheckSlot(iColumnX, iRowY, pItemAttr->Width, pItemAttr->Height);
}

bool SEASON3B::CNewUIInventoryCtrl::CanMoveToPt(int x, int y, ITEM* pItem)
{
    int iColumnX, iRowY;
    if (GetSquarePosAtPt(x, y, iColumnX, iRowY))
        return CanMove(iColumnX, iRowY, pItem);
    return false;
}

void SEASON3B::CNewUIInventoryCtrl::SetToolTipType(TOOLTIP_TYPE ToolTipType)
{
    m_ToolTipType = ToolTipType;
}

void SEASON3B::CNewUIInventoryCtrl::CreateItemToolTip(ITEM* pItem)
{
    if (m_pToolTipItem)
        DeleteItemToolTip();

    if (g_pNewItemMng)
        m_pToolTipItem = g_pNewItemMng->CreateItem(pItem);
}

void SEASON3B::CNewUIInventoryCtrl::DeleteItemToolTip()
{
    if (m_pToolTipItem && g_pNewItemMng)
    {
        g_pNewItemMng->DeleteItem(m_pToolTipItem);
        m_pToolTipItem = nullptr;
    }
}

void SEASON3B::CNewUIInventoryCtrl::SetRepairMode(bool bRepair)
{
    m_bRepairMode = bRepair;

    if (m_bRepairMode == true)
    {
        SetToolTipType(TOOLTIP_TYPE_REPAIR);
    }
    else
    {
        SetToolTipType(TOOLTIP_TYPE_INVENTORY);
    }
}

bool SEASON3B::CNewUIInventoryCtrl::IsRepairMode()
{
    return m_bRepairMode;
}

void SEASON3B::CNewUIInventoryCtrl::RenderNumberOfItem()
{
    EnableAlphaTest();
    glColor3f(1.f, 1.f, 1.f);
    auto li = m_vecItem.begin();
    for (; li != m_vecItem.end(); ++li)
    {
        const ITEM* pItem = (*li);
        const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
        const float x = m_Pos.x + (pItem->x * INVENTORY_SQUARE_WIDTH);
        const float y = m_Pos.y + (pItem->y * INVENTORY_SQUARE_HEIGHT);
        const float width = pItemAttr->Width * INVENTORY_SQUARE_WIDTH;
        float height = pItemAttr->Height * INVENTORY_SQUARE_HEIGHT;

        if (pItem->Type >= ITEM_POTION && pItem->Type <= ITEM_ANTIDOTE && pItem->Durability > 1)
        {
            glColor3f(1.f, 0.9f, 0.7f);
            SEASON3B::RenderNumber(x + width - 6, y + 1, pItem->Durability);
        }
        else if (pItem->Type >= ITEM_JACK_OLANTERN_BLESSINGS && pItem->Type <= ITEM_JACK_OLANTERN_DRINK && pItem->Durability > 1)
        {
            glColor3f(1.f, 0.9f, 0.7f);
            SEASON3B::RenderNumber(x + width - 6, y + 1, pItem->Durability);
        }
        else if (pItem->Type >= ITEM_SMALL_SHIELD_POTION && pItem->Type <= ITEM_LARGE_COMPLEX_POTION && pItem->Durability > 1)
        {
            glColor3f(1.f, 0.9f, 0.7f);
            SEASON3B::RenderNumber(x + width - 6, y + 1, pItem->Durability);
        }
        else if (pItem->Type >= ITEM_POTION + 70 && pItem->Type <= ITEM_POTION + 71 && pItem->Durability > 1)
        {
            glColor3f(1.f, 0.9f, 0.7f);
            SEASON3B::RenderNumber(x + width - 6, y + 1, pItem->Durability);
        }
        else if (pItem->Type == ITEM_POTION + 94 && pItem->Durability > 1)
        {
            glColor3f(1.f, 0.9f, 0.7f);
            SEASON3B::RenderNumber(x + width - 6, y + 1, pItem->Durability);
        }
        else if (pItem->Type >= ITEM_POTION + 78 && pItem->Type <= ITEM_POTION + 82 && pItem->Durability > 1)
        {
            glColor3f(1.f, 0.9f, 0.7f);
            SEASON3B::RenderNumber(x + width - 6, y + 1, pItem->Durability);
        }
        else if (pItem->Type >= ITEM_CHERRY_BLOSSOM_WINE && pItem->Type <= ITEM_GOLDEN_CHERRY_BLOSSOM_BRANCH && pItem->Durability > 1)
        {
            glColor3f(1.f, 0.9f, 0.7f);
            SEASON3B::RenderNumber(x + width - 6, y + 1, pItem->Durability);
        }
        else if (pItem->Type == ITEM_POTION + 133 && pItem->Durability > 1)
        {
            glColor3f(1.f, 0.9f, 0.7f);
            SEASON3B::RenderNumber(x + width - 6, y + 1, pItem->Durability);
        }
        else if (COMGEM::isCompiledGem(pItem))
        {
            const int Level = pItem->Level;
            glColor3f(1.f, 0.9f, 0.7f);
            SEASON3B::RenderNumber(x + width - 6, y + 1, (Level + 1) * COMGEM::FIRST);
        }
    }
    glColor3f(1.f, 1.f, 1.f);
    DisableAlphaBlend();
}

void SEASON3B::CNewUIInventoryCtrl::RenderItemToolTip()
{
    if (m_pToolTipItem)
    {
        const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[m_pToolTipItem->Type];
        const int iTargetX = m_Pos.x + m_pToolTipItem->x * INVENTORY_SQUARE_WIDTH + pItemAttr->Width * INVENTORY_SQUARE_WIDTH / 2;
        int iTargetY = m_Pos.y + m_pToolTipItem->y * INVENTORY_SQUARE_HEIGHT;

        if (pItemAttr->Height == 1)
        {
            iTargetY += INVENTORY_SQUARE_HEIGHT / 2;
        }

        if (m_ToolTipType == TOOLTIP_TYPE_INVENTORY)
        {
            RenderItemInfo(iTargetX, iTargetY, m_pToolTipItem, false);
        }
        else if (m_ToolTipType == TOOLTIP_TYPE_REPAIR)
        {
            RenderRepairInfo(iTargetX, iTargetY, m_pToolTipItem, false);
        }
        else if (m_ToolTipType == TOOLTIP_TYPE_NPC_SHOP)
        {
            RenderItemInfo(iTargetX, iTargetY, m_pToolTipItem, true);
        }
        else if (m_ToolTipType == TOOLTIP_TYPE_MY_SHOP)
        {
            RenderItemInfo(iTargetX, iTargetY, m_pToolTipItem, false, m_ToolTipType);
        }
        else if (m_ToolTipType == TOOLTIP_TYPE_PURCHASE_SHOP)
        {
            RenderItemInfo(iTargetX, iTargetY, m_pToolTipItem, false, m_ToolTipType);
        }
    }
}

void SEASON3B::CNewUIInventoryCtrl::UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB)
{
    if (pClass)
    {
        auto* pInventoryCtrl = static_cast<CNewUIInventoryCtrl*>(pClass);
        if (dwParamA == RENDER_NUMBER_OF_ITEM)
            pInventoryCtrl->RenderNumberOfItem();
        else if (dwParamA == RENDER_ITEM_TOOLTIP)
            pInventoryCtrl->RenderItemToolTip();
    }
}

CNewUIPickedItem* SEASON3B::CNewUIInventoryCtrl::GetPickedItem()
{
    return ms_pPickedItem;
}

bool SEASON3B::CNewUIInventoryCtrl::CreatePickedItem(CNewUIInventoryCtrl* pSrc, ITEM* pItem)
{
    if (g_pNewItemMng)
    {
        ms_pPickedItem = new CNewUIPickedItem;
        return ms_pPickedItem->Create(g_pNewItemMng, pSrc, pItem);
    }
    return false;
}

void SEASON3B::CNewUIInventoryCtrl::DeletePickedItem()
{
    if (ms_pPickedItem)
    {
        CNewUIInventoryCtrl* pOwner = ms_pPickedItem->GetOwnerInventory();
        if (pOwner)
        {
            pOwner->SetEventState(CNewUIInventoryCtrl::EVENT_NONE);
        }
    }

    SAFE_DELETE(ms_pPickedItem);
}

void SEASON3B::CNewUIInventoryCtrl::BackupPickedItem()
{
    if (ms_pPickedItem && EquipmentItem == false)
    {
        CNewUIInventoryCtrl* pOwner = ms_pPickedItem->GetOwnerInventory();
        ITEM* pItemObj = ms_pPickedItem->GetItem();
        if (pOwner)
        {
            pOwner->AddItem(pItemObj->x, pItemObj->y, pItemObj);

            DeletePickedItem();
        }
        else if (pItemObj->ex_src_type == ITEM_EX_SRC_EQUIPMENT)
        {
            ITEM* pEquipmentItemSlot = &CharacterMachine->Equipment[pItemObj->lineal_pos];
            memcpy(pEquipmentItemSlot, pItemObj, sizeof(ITEM));

            g_pMyInventory->CreateEquippingEffect(pEquipmentItemSlot);

            if (pEquipmentItemSlot->Type == ITEM_DARK_RAVEN_ITEM && !gMapManager.InChaosCastle())
            {
                PET_INFO* pPetInfo = giPetManager::GetPetInfo(pEquipmentItemSlot);
                giPetManager::CreatePetDarkSpirit_Now(Hero);
                static_cast<CSPetSystem*>(Hero->m_pPet)->SetPetInfo(pPetInfo);
            }
            DeletePickedItem();
        }
    }
}

void SEASON3B::CNewUIInventoryCtrl::SetEventState(EVENT_STATE es)
{
    m_EventState = es;
}

void SEASON3B::CNewUIInventoryCtrl::Render3D()
{
    auto li = m_vecItem.begin();
    for (; li != m_vecItem.end(); ++li)
    {
        const ITEM* pItem = (*li);
        const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];

        const float x = m_Pos.x + (pItem->x * INVENTORY_SQUARE_WIDTH);
        const float y = m_Pos.y + (pItem->y * INVENTORY_SQUARE_HEIGHT);
        const float width = pItemAttr->Width * INVENTORY_SQUARE_WIDTH;
        const float height = pItemAttr->Height * INVENTORY_SQUARE_HEIGHT;
        glColor4f(1.f, 1.f, 1.f, 1.f);

        RenderItem3D(x, y, width, height, pItem->Type, pItem->Level, pItem->ExcellentFlags, pItem->AncientDiscriminator, false);
    }
}

bool SEASON3B::CNewUIInventoryCtrl::AreItemsStackable(ITEM* pSourceItem, ITEM* pTargetItem)
{
    if (pSourceItem == nullptr || pTargetItem == nullptr)
    {
        return false;
    }

    const int iSrcType = pSourceItem->Type;
    const int iTarType = pTargetItem->Type;
    const int iSrcLevel = pSourceItem->Level;
    const int iTarLevel = pTargetItem->Level;
    const int iSrcDurability = pSourceItem->Durability;
    const int iTarDurability = pTargetItem->Durability;

    if (iSrcType != iTarType)
    {
        return false;
    }

    if (iSrcType == ITEM_SIEGE_POTION && iTarType == ITEM_SIEGE_POTION && (iSrcDurability < 250 && iTarDurability < 250))
    {
        return true;
    }

    if ((iSrcType >= ITEM_POTION && iSrcType <= ITEM_ANTIDOTE && iSrcType != ITEM_SIEGE_POTION) && (iTarType >= ITEM_POTION && iTarType <= ITEM_ANTIDOTE && iTarType != ITEM_SIEGE_POTION) && (iSrcDurability < 3 && iTarDurability < 3))
    {
        return true;
    }

    if ((iSrcType >= ITEM_SMALL_COMPLEX_POTION && iSrcType <= ITEM_LARGE_COMPLEX_POTION) && (iTarType >= ITEM_SMALL_COMPLEX_POTION && iTarType <= ITEM_LARGE_COMPLEX_POTION) && (iSrcDurability < 3 && iTarDurability < 3))
    {
        return true;
    }

    if ((iSrcType == ITEM_BOLT && iTarType == ITEM_BOLT) && (iSrcLevel == iTarLevel))
    {
        return true;
    }

    if ((iSrcType == ITEM_ARROWS && iTarType == ITEM_ARROWS) && (iSrcLevel == iTarLevel))
    {
        return true;
    }

    if (iSrcType == ITEM_SYMBOL_OF_KUNDUN && iTarType == ITEM_SYMBOL_OF_KUNDUN)
    {
        return true;
    }

    if ((iSrcType >= ITEM_SPLINTER_OF_ARMOR && iSrcType <= ITEM_CLAW_OF_BEAST) && (iTarType >= ITEM_SPLINTER_OF_ARMOR && iTarType <= ITEM_CLAW_OF_BEAST))
    {
        return true;
    }

    if ((iSrcType >= ITEM_JACK_OLANTERN_BLESSINGS && iSrcType <= ITEM_JACK_OLANTERN_DRINK) && (iTarType >= ITEM_JACK_OLANTERN_BLESSINGS && iTarType <= ITEM_JACK_OLANTERN_DRINK) && (iSrcDurability < 3 && iTarDurability < 3))
    {
        return true;
    }

    if (iSrcType == ITEM_POTION + 70 && iTarType == ITEM_POTION + 70 && (iSrcDurability < 50 && iTarDurability < 50))
    {
        return true;
    }

    if (iSrcType == ITEM_POTION + 71 && iTarType == ITEM_POTION + 71 && (iSrcDurability < 50 && iTarDurability < 50))
    {
        return true;
    }

    if (iSrcType == ITEM_POTION + 78 && iTarType == ITEM_POTION + 78 && (iSrcDurability < 3 && iTarDurability < 3))
    {
        return true;
    }

    if (iSrcType == ITEM_POTION + 79 && iTarType == ITEM_POTION + 79 && (iSrcDurability < 3 && iTarDurability < 3))
    {
        return true;
    }

    if (iSrcType == ITEM_POTION + 80 && iTarType == ITEM_POTION + 80 && (iSrcDurability < 3 && iTarDurability < 3))
    {
        return true;
    }

    if (iSrcType == ITEM_POTION + 81 && iTarType == ITEM_POTION + 81 && (iSrcDurability < 3 && iTarDurability < 3))
    {
        return true;
    }

    if (iSrcType == ITEM_POTION + 82 && iTarType == ITEM_POTION + 82 && (iSrcDurability < 3 && iTarDurability < 3))
    {
        return true;
    }

    if (iSrcType == ITEM_POTION + 94 && iTarType == ITEM_POTION + 94 && (iSrcDurability < 50 && iTarDurability < 50))
    {
        return true;
    }

    if (iSrcType == ITEM_CHERRY_BLOSSOM_WINE && iTarType == ITEM_CHERRY_BLOSSOM_WINE && (iSrcDurability < 3 && iTarDurability < 3))
    {
        return true;
    }

    if (iSrcType == ITEM_CHERRY_BLOSSOM_RICE_CAKE && iTarType == ITEM_CHERRY_BLOSSOM_RICE_CAKE && (iSrcDurability < 3 && iTarDurability < 3))
    {
        return true;
    }

    if (iSrcType == ITEM_CHERRY_BLOSSOM_FLOWER_PETAL && iTarType == ITEM_CHERRY_BLOSSOM_FLOWER_PETAL && (iSrcDurability < 3 && iTarDurability < 3))
    {
        return true;
    }

    if (iSrcType == ITEM_POTION + 88 && iTarType == ITEM_POTION + 88 && (iSrcDurability < 10 && iTarDurability < 10))
    {
        return true;
    }

    if (iSrcType == ITEM_POTION + 89 && iTarType == ITEM_POTION + 89 && (iSrcDurability < 30 && iTarDurability < 30))
    {
        return true;
    }

    if (iSrcType == ITEM_GOLDEN_CHERRY_BLOSSOM_BRANCH && iTarType == ITEM_GOLDEN_CHERRY_BLOSSOM_BRANCH && (iSrcDurability < 50 && iTarDurability < 50))
    {
        return true;
    }

    if (iSrcType == ITEM_POTION + 100 && (iSrcDurability < 255 && iTarDurability < 255))
    {
        return true;
    }

    if (iSrcType == ITEM_POTION + 110 && iTarType == ITEM_POTION + 110)
    {
        return true;
    }

    if (iSrcType == ITEM_SUSPICIOUS_SCRAP_OF_PAPER && iTarType == ITEM_SUSPICIOUS_SCRAP_OF_PAPER && (iSrcDurability < 5 && iTarDurability < 5))
    {
        return true;
    }

    if (iSrcType == ITEM_POTION + 133 && (iSrcDurability < 50 && iTarDurability < 50))
    {
        return true;
    }

    return false;
}

bool SEASON3B::CNewUIInventoryCtrl::CanPushItem()
{
    return m_bCanPushItem;
}

bool SEASON3B::CNewUIInventoryCtrl::CanUpgradeItem(ITEM* pSourceItem, ITEM* pTargetItem)
{
    const int	iTargetLevel = pTargetItem->Level;

    if (((pTargetItem->Type >= ITEM_SWORD && pTargetItem->Type < ITEM_WING)
        && (pTargetItem->Type != ITEM_BOLT)
        && (pTargetItem->Type != ITEM_ARROWS))
        || (pTargetItem->Type >= ITEM_WING && pTargetItem->Type <= ITEM_WINGS_OF_DARKNESS)
        || (pTargetItem->Type >= ITEM_WING_OF_STORM && pTargetItem->Type <= ITEM_WING_OF_DIMENSION)
        )
    {
        if ((pSourceItem->Type == ITEM_JEWEL_OF_BLESS)
            && (iTargetLevel >= 0 && iTargetLevel <= 5))
        {
            return true;
        }

        if ((pSourceItem->Type == ITEM_JEWEL_OF_SOUL)
            && (iTargetLevel >= 0 && iTargetLevel <= 8))
        {
            return true;
        }
    }

    return false;
}