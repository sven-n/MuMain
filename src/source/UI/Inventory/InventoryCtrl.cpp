
#include "stdafx.h"
#include "UI/Core/UIManager.h"
#include "UI/Inventory/InventoryCtrl.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"
#include "UI/Inventory/ItemMng.h"
#include "UI/Core/WindowSystem.h"
#include "Engine/Object/ZzzInventory.h"
#include "GameLogic/Items/CComGem.h"
#include "GameLogic/Pets/GIPetManager.h"
#include "GameLogic/Items/CSItemOption.h"
#include "Network/Server/SocketSystem.h"
#include "UI/Scaling/UITransform.h"
#include "World/MapInfra/MapManager.h"
#include "GameLogic/Items/MixMgr.h"
#include "UI/RmlBridge/RmlTooltip.h"
#include "UI/Tooltip/LegacyTextListTooltip.h"

#include <RmlUi/Core/ElementDocument.h>
using namespace SEASON3B;
using namespace mu::ui::window;

POINT UI::Items::Drag::PickupOffset(int itemLeft, int itemTop, int itemWidth, int itemHeight,
                                    int pointerX, int pointerY, bool preserveAnchor)
{
    if (itemWidth <= 0 || itemHeight <= 0)
    {
        return {0, 0};
    }

    if (preserveAnchor)
    {
        return {
            std::clamp(pointerX - itemLeft, 0, itemWidth - 1),
            std::clamp(pointerY - itemTop, 0, itemHeight - 1),
        };
    }

    return {itemWidth / 2, itemHeight / 2};
}

POINT UI::Items::Drag::ItemTopLeft(int pointerX, int pointerY, const POINT& pickupOffset)
{
    return {pointerX - pickupOffset.x, pointerY - pickupOffset.y};
}

bool UI::Items::Drag::ShouldConsumePanelPress(bool hasPickedItem, bool leftButtonPressed)
{
    return hasPickedItem && leftButtonPressed;
}

bool UI::Items::Grid::Fits(int startIndex, int itemWidth, int itemHeight, int columnCount, int rowCount)
{
    if (startIndex < 0 || itemWidth <= 0 || itemHeight <= 0 || columnCount <= 0 || rowCount <= 0)
    {
        return false;
    }

    const int startColumn = startIndex % columnCount;
    const int startRow = startIndex / columnCount;
    return startColumn + itemWidth <= columnCount && startRow + itemHeight <= rowCount;
}

mu::ui::window::CPickedItem::CPickedItem()
{
    m_pNewItemMng = nullptr;
    m_pSrcInventory = nullptr;
    m_pPickedItem = nullptr;
    m_bShow = true;
    m_Pos.x = m_Pos.y = 0;
    m_Size.cx = m_Size.cy = 0;
}

mu::ui::window::CPickedItem::~CPickedItem()
{
    Release();
}

bool mu::ui::window::CPickedItem::Create(CItemMng* pNewItemMng, CInventoryCtrl* pSrc, ITEM* pItem,
                                       bool preservePickupAnchor)
{
    if (pNewItemMng == nullptr || pItem == nullptr)
        return false;

    m_pNewItemMng = pNewItemMng;
    m_pSrcInventory = pSrc;
    if (nullptr == (m_pPickedItem = m_pNewItemMng->DuplicateItem(pItem)))
    {
        return false;
    }

    // Sized by the grid it left, or the inventory's for an equipped item.
    const CInventoryCtrl* sizing = SizingGrid();
    m_Size = SizeIn(sizing);
    const bool hasGridAnchor = preservePickupAnchor && pSrc != nullptr;
    const UI::Items::GridRect itemBox =
        hasGridAnchor ? pSrc->Geometry().CellsRect(pItem->x, pItem->y, 1, 1) : UI::Items::GridRect{};
    const POINT offset = UI::Items::Drag::PickupOffset(static_cast<int>(std::lround(itemBox.x)),
                                                       static_cast<int>(std::lround(itemBox.y)), m_Size.cx,
                                                       m_Size.cy, MouseX, MouseY, hasGridAnchor);
    const UI::Items::GridGeometry geometry = sizing ? sizing->Geometry() : UI::Items::GridGeometry{};
    m_Anchor = {offset.x / geometry.PitchX(), offset.y / geometry.PitchY()};
    m_Pos = TopLeftIn(sizing, MouseX, MouseY);

    return true;
}

void mu::ui::window::CPickedItem::Release()
{
    m_pNewItemMng->DeleteDuplicatedItem(m_pPickedItem);
    m_pPickedItem = nullptr;
    m_pNewItemMng = nullptr;
    m_pSrcInventory = nullptr;
    m_bShow = true;
    m_Anchor = {};
}

CInventoryCtrl* mu::ui::window::CPickedItem::GetOwnerInventory() const
{
    return m_pSrcInventory;
}

STORAGE_TYPE CPickedItem::GetSourceStorageType() const
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

ITEM* mu::ui::window::CPickedItem::GetItem() const
{
    return m_pPickedItem;
}

const POINT& mu::ui::window::CPickedItem::GetPos() const
{
    return m_Pos;
}

const SIZE& mu::ui::window::CPickedItem::GetSize() const
{
    return m_Size;
}

const CInventoryCtrl* mu::ui::window::CPickedItem::SizingGrid() const
{
    if (m_pSrcInventory)
        return m_pSrcInventory;
    return g_pMyInventory ? g_pMyInventory->GetInventoryCtrl() : nullptr;
}

POINT mu::ui::window::CPickedItem::TopLeftIn(const CInventoryCtrl* grid, int pointerX, int pointerY) const
{
    const UI::Items::GridGeometry geometry = grid ? grid->Geometry() : UI::Items::GridGeometry{};
    int left = 0;
    int top = 0;
    UI::Items::AnchoredTopLeft(static_cast<float>(pointerX), static_cast<float>(pointerY), m_Anchor,
                               geometry.PitchX(), geometry.PitchY(), left, top);
    return {left, top};
}

SIZE mu::ui::window::CPickedItem::SizeIn(const CInventoryCtrl* grid) const
{
    if (m_pPickedItem == nullptr)
        return {0, 0};
    const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[m_pPickedItem->Type];
    const UI::Items::GridGeometry geometry = grid ? grid->Geometry() : UI::Items::GridGeometry{};
    const UI::Items::GridRect box = geometry.CellsRect(0, 0, pItemAttr->Width, pItemAttr->Height);
    return {static_cast<LONG>(std::lround(box.width)), static_cast<LONG>(std::lround(box.height))};
}

void mu::ui::window::CPickedItem::GetRect(RECT& rcBox)
{
    rcBox.left = m_Pos.x;
    rcBox.top = m_Pos.y;
    rcBox.right = rcBox.left + m_Size.cx;
    rcBox.bottom = rcBox.top + m_Size.cy;
}

int mu::ui::window::CPickedItem::GetSourceLinealPos()
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

bool mu::ui::window::CPickedItem::GetTargetPos(CInventoryCtrl* pDest, int& iTargetColumnX, int& iTargetRowY)
{
    if (pDest != nullptr)
    {
        const POINT itemTopLeft = TopLeftIn(pDest, MouseX, MouseY);

        return pDest->GetSquarePosAtPt(itemTopLeft.x, itemTopLeft.y, iTargetColumnX, iTargetRowY);
    }
    return false;
}

int mu::ui::window::CPickedItem::GetTargetLinealPos(CInventoryCtrl* pDest)
{
    int iTargetColumnX, iTargetRowY;
    if (GetTargetPos(pDest, iTargetColumnX, iTargetRowY))
    {
        return pDest->GetIndex(iTargetColumnX, iTargetRowY);
    }
    return -1;
}

bool mu::ui::window::CPickedItem::IsVisible() const
{
    return m_bShow;
}

CObject* mu::ui::window::CPickedItem::GetLayoutOwner() const
{
    return m_pSrcInventory ? m_pSrcInventory->GetOwner() : nullptr;
}

void mu::ui::window::CPickedItem::ShowPickedItem()
{
    m_bShow = true;
}

void mu::ui::window::CPickedItem::HidePickedItem()
{
    m_bShow = false;
}

void mu::ui::window::CPickedItem::Render3D()
{
    if (m_pPickedItem && m_pPickedItem->Type >= 0)
    {
        const auto transform = UI::Scaling::GetActiveTransform();
        const int pointerX = static_cast<int>(std::floor(UI::Scaling::LogicalX(transform, g_fWindowMouseX)));
        const int pointerY = static_cast<int>(std::floor(UI::Scaling::LogicalY(transform, g_fWindowMouseY)));
        m_Pos = TopLeftIn(SizingGrid(), pointerX, pointerY);
        RenderItem3D(m_Pos.x, m_Pos.y, m_Size.cx, m_Size.cy, m_pPickedItem->Type, m_pPickedItem->Level,
                     m_pPickedItem->ExcellentFlags, m_pPickedItem->AncientDiscriminator, true);
    }
}

CPickedItem* mu::ui::window::CInventoryCtrl::ms_pPickedItem = nullptr;

// cppcheck-suppress uninitMemberVar
mu::ui::window::CInventoryCtrl::CInventoryCtrl()
{
    Init();
}

mu::ui::window::CInventoryCtrl::~CInventoryCtrl()
{
    Release();
}

void mu::ui::window::CInventoryCtrl::Init()
{
    m_pNewItemMng = nullptr;
    m_pOwner = nullptr;
    m_Geometry = {};
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

void mu::ui::window::CInventoryCtrl::LoadImages()
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
#endif // LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
}

void mu::ui::window::CInventoryCtrl::UnloadImages()
{
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
    DeleteBitmap(IMAGE_ITEM_SQUARE_BOTTOM_RECT);
    DeleteBitmap(IMAGE_ITEM_SQUARE_TOP_RECT);
    DeleteBitmap(IMAGE_ITEM_SQUARE_FOR_1_BY_1);
#endif // LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY

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

void mu::ui::window::CInventoryCtrl::SetItemColorState(ITEM* pItem)
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

bool mu::ui::window::CInventoryCtrl::CanChangeItemColorState(ITEM* pItem)
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

    if (pItem->Type == ITEM_WIZARDS_RING && (pItem->Level == 1 || pItem->Level == 2))
    {
        return false;
    }

    if (pItem->Type >= ITEM_RING_OF_ICE && pItem->Type <= ITEM_RING_OF_POISON ||
        pItem->Type == ITEM_TRANSFORMATION_RING ||
        pItem->Type >= ITEM_PENDANT_OF_LIGHTING && pItem->Type <= ITEM_PENDANT_OF_FIRE ||
        pItem->Type == ITEM_WIZARDS_RING ||
        pItem->Type >= ITEM_RING_OF_FIRE && pItem->Type <= ITEM_PENDANT_OF_ABILITY ||
        pItem->Type >= ITEM_MOONSTONE_PENDANT && pItem->Type <= ITEM_GAME_MASTER_TRANSFORMATION_RING
#ifdef PJH_ADD_PANDA_CHANGERING
        || pItem->Type == ITEM_PANDA_TRANSFORMATION_RING
#endif // PJH_ADD_PANDA_CHANGERING
        || pItem->Type == ITEM_SKELETON_TRANSFORMATION_RING || pItem->Type == ITEM_PET_PANDA ||
        pItem->Type == ITEM_DEMON || pItem->Type == ITEM_SPIRIT_OF_GUARDIAN || pItem->Type == ITEM_PET_SKELETON ||
        pItem->Type == ITEM_HELPER + 107 || pItem->Type == ITEM_HELPER + 109 || pItem->Type == ITEM_HELPER + 110 ||
        pItem->Type == ITEM_HELPER + 111 || pItem->Type == ITEM_HELPER + 112 || pItem->Type == ITEM_HELPER + 113 ||
        pItem->Type == ITEM_HELPER + 114 || pItem->Type == ITEM_HELPER + 115
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
        || g_pMyInventory->IsInvenItem(pItem->Type)
#endif // LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
    )
    {
        return true;
    }

    if (pItem->Type >= ITEM_HELPER && pItem->Type <= ITEM_DARK_RAVEN_ITEM || pItem->Type == ITEM_HORN_OF_FENRIR ||
        pItem->Type == ITEM_PET_UNICORN)
    {
        return true;
    }

    if (IsWingItem(pItem) == true)
    {
        return true;
    }

    return false;
}

bool mu::ui::window::CInventoryCtrl::Create(STORAGE_TYPE storageType, CItemMng* pNewItemMng, CObject* pOwner,
                                           int x, int y, int nColumn,
                                           int nRow, int nIndexOffset)
{
    m_StorageType = storageType;
    m_nIndexOffset = nIndexOffset;
    if (m_pdwItemCheckBox || false == m_vecItem.empty())
        return false;
    if (pNewItemMng == nullptr)
        return false;

    m_pNewItemMng = pNewItemMng;
    m_pOwner = pOwner;
    m_Geometry = {static_cast<float>(x), static_cast<float>(y), UI::Items::GridGeometry::DefaultPitch,
                  UI::Items::GridGeometry::DefaultPitch, nColumn, nRow};
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
void mu::ui::window::CInventoryCtrl::Release()
{
    RemoveAllItems();
    UnloadImages();

    SAFE_DELETE(m_pdwItemCheckBox);

    Init();
}

bool mu::ui::window::CInventoryCtrl::AddItem(int iLinealPos, std::span<const BYTE> itemData)
{
    iLinealPos -= m_nIndexOffset;
    if (iLinealPos < 0 || iLinealPos >= m_nColumn * m_nRow)
        return false;

    const int iColumnX = iLinealPos % m_nColumn;
    const int iRowY = iLinealPos / m_nColumn;

    return AddItem(iColumnX, iRowY, itemData);
}

bool mu::ui::window::CInventoryCtrl::AddItem(int iColumnX, int iRowY, std::span<const BYTE> itemData)
{
    if (iColumnX < 0 || iRowY < 0 || iColumnX >= m_nColumn || iRowY >= m_nRow)
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

bool mu::ui::window::CInventoryCtrl::AddItem(int iColumnX, int iRowY, ITEM* pItem)
{
    if (iColumnX < 0 || iRowY < 0 || iColumnX >= m_nColumn || iRowY >= m_nRow)
        return false;

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

bool mu::ui::window::CInventoryCtrl::AddItem(int iColumnX, int iRowY, BYTE byType, BYTE bySubType, BYTE byLevel,
                                            BYTE byDurability, BYTE byOption1, BYTE byOptionEx, BYTE byOption380,
                                            BYTE byOptionHarmony)
{
    if (iColumnX < 0 || iRowY < 0 || iColumnX >= m_nColumn || iRowY >= m_nRow)
        return false;

    ITEM* pNewItem = m_pNewItemMng->CreateItem(byType, bySubType, byLevel, byDurability, byOption1, byOptionEx,
                                               byOption380, byOptionHarmony);
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

void mu::ui::window::CInventoryCtrl::RemoveItem(ITEM* pItem)
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

bool mu::ui::window::CInventoryCtrl::RemoveItemAt(int iLinealPos)
{
    iLinealPos -= m_nIndexOffset;
    ITEM* pItem = this->FindItemFromSlotIndex(iLinealPos, true);
    if (pItem == nullptr)
    {
        return false;
    }

    this->RemoveItem(pItem);
    return true;
}

ITEM* mu::ui::window::CInventoryCtrl::FindItemFromSlotIndex(const int slotIndex, const bool recoverIfMissing)
{
    if (slotIndex < 0 || slotIndex >= m_nColumn * m_nRow)
    {
        return nullptr;
    }

    const DWORD key = m_pdwItemCheckBox[slotIndex];
    if (key <= 1)
    {
        return nullptr;
    }

    ITEM* pItem = this->FindItemByKey(key);
    if (pItem != nullptr)
    {
        return pItem;
    }

    if (recoverIfMissing)
    {
        this->ClearSlotKey(key);
        this->RequestInventoryRefresh();
    }

    return nullptr;
}

void mu::ui::window::CInventoryCtrl::ClearSlotKey(const DWORD key)
{
    if (key == 0)
    {
        return;
    }

    for (int i = 0; i < m_nColumn * m_nRow; ++i)
    {
        if (m_pdwItemCheckBox[i] == key)
        {
            m_pdwItemCheckBox[i] = 0;
        }
    }
}

void mu::ui::window::CInventoryCtrl::RequestInventoryRefresh() const
{
    static DWORD lastRefreshRequestTick = 0;
    const DWORD currentTick = GetTickCount();
    if (currentTick - lastRefreshRequestTick < 1000)
    {
        return;
    }

    lastRefreshRequestTick = currentTick;

    if (SocketClient != nullptr && SocketClient->ToGameServer() != nullptr)
    {
        SocketClient->ToGameServer()->SendInventoryRequest();
    }
}

void mu::ui::window::CInventoryCtrl::RemoveAllItems()
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

size_t mu::ui::window::CInventoryCtrl::GetNumberOfItems()
{
    return m_vecItem.size();
}

ITEM* mu::ui::window::CInventoryCtrl::GetItem(int iIndex)
{
    if (iIndex < 0 || iIndex >= static_cast<int>(m_vecItem.size()))
        return nullptr;
    return m_vecItem[iIndex];
}

void mu::ui::window::CInventoryCtrl::SetSquareColorNormal(float fRed, float fGreen, float fBlue)
{
    Vector(fRed, fGreen, fBlue, m_afColorStateNormal);
}

void mu::ui::window::CInventoryCtrl::GetSquareColorNormal(float* pfParams) const
{
    Vector(m_afColorStateNormal[0], m_afColorStateNormal[1], m_afColorStateNormal[2], pfParams);
}

void mu::ui::window::CInventoryCtrl::SetSquareColorWarning(float fRed, float fGreen, float fBlue)
{
    Vector(fRed, fGreen, fBlue, m_afColorStateWarning);
}

void mu::ui::window::CInventoryCtrl::GetSquareColorWarning(float* pfParams) const
{
    Vector(m_afColorStateWarning[0], m_afColorStateWarning[1], m_afColorStateWarning[2], pfParams);
}

ITEM* mu::ui::window::CInventoryCtrl::FindItem(int iLinealPos)
{
    iLinealPos -= m_nIndexOffset;
    return this->FindItemFromSlotIndex(iLinealPos, true);
}

ITEM* mu::ui::window::CInventoryCtrl::FindItem(int iColumnX, int iRowY)
{
    return FindItem(iRowY * m_nColumn + iColumnX + m_nIndexOffset);
}

ITEM* mu::ui::window::CInventoryCtrl::FindItemByKey(DWORD dwKey)
{
    auto li = m_vecItem.begin();
    for (; li != m_vecItem.end(); ++li)
        if ((*li)->Key == dwKey)
            return (*li);
    return nullptr;
}

ITEM* mu::ui::window::CInventoryCtrl::FindTypeItem(short int siType)
{
    auto li = m_vecItem.begin();
    for (; li != m_vecItem.end(); ++li)
        if ((*li)->Type == siType)
            return (*li);
    return nullptr;
}

bool mu::ui::window::CInventoryCtrl::IsItem(short int siType)
{
    auto li = m_vecItem.begin();
    for (; li != m_vecItem.end(); ++li)
        if ((*li)->Type == siType)
            return true;
    return false;
}

int mu::ui::window::CInventoryCtrl::GetItemCount(short int siType, int iLevel)
{
    int count = 0;
    auto li = m_vecItem.begin();
    for (; li != m_vecItem.end(); ++li)
    {
        if ((*li)->Type == siType)
        {
            if (iLevel == -1 || (*li)->Level == iLevel)
            {
                count += ((*li)->Durability == 0) ? 1 : (*li)->Durability;
            }
        }
    }
    return count;
}

int mu::ui::window::CInventoryCtrl::FindItemIndex(short int siType, int iLevel)
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

int mu::ui::window::CInventoryCtrl::FindItemReverseIndex(short sType, int iLevel)
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

int mu::ui::window::CInventoryCtrl::GetIndexByItem(ITEM* pItem)
{
    if (pItem == nullptr)
    {
        return -1;
    }

    return this->GetIndex(pItem->x, pItem->y);
}

ITEM* mu::ui::window::CInventoryCtrl::FindItemPointedSquareIndex()
{
    if (m_iPointedSquareIndex != -1)
    {
        ITEM* pItem = nullptr;
        pItem = FindItemByKey(m_pdwItemCheckBox[m_iPointedSquareIndex - m_nIndexOffset]);
        return pItem;
    }

    return nullptr;
}

int mu::ui::window::CInventoryCtrl::GetPointedSquareIndex()
{
    return m_iPointedSquareIndex;
}

ITEM* mu::ui::window::CInventoryCtrl::FindItemAtPt(int x, int y)
{
    const int iIndex = GetIndexAtPt(x, y);
    return FindItem(iIndex);
}

int mu::ui::window::CInventoryCtrl::FindEmptySlot(IN int cx, IN int cy)
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
bool mu::ui::window::CInventoryCtrl::FindEmptySlot(IN int cx, IN int cy, OUT int& iColumnX, OUT int& iColumnY)
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

int mu::ui::window::CInventoryCtrl::GetNumItemByKey(DWORD dwItemKey)
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

int mu::ui::window::CInventoryCtrl::GetNumItemByType(short sItemType)
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

int mu::ui::window::CInventoryCtrl::GetEmptySlotCount()
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

bool mu::ui::window::CInventoryCtrl::UpdateMouseEvent()
{
    if (m_EventState == EVENT_NONE && mu::ui::window::IsNone(VK_LBUTTON) && m_iPointedSquareIndex != -1)
    {
        m_EventState = EVENT_HOVER;
    }
    else if (m_EventState == EVENT_HOVER && mu::ui::window::IsRelease(VK_LBUTTON) && m_iPointedSquareIndex != -1 &&
             nullptr == GetPickedItem() && false == IsLocked() && m_bRepairMode == false)
    {
        m_EventState = EVENT_PICKING;
        ITEM* pItem = this->FindItem(m_iPointedSquareIndex);
        if (pItem)
        {
            if (CreatePickedItem(this, pItem, true))
            {
                RemoveItem(pItem);
                return false;
            }
        }
    }
    else if (m_EventState == EVENT_HOVER && mu::ui::window::IsNone(VK_LBUTTON) && m_iPointedSquareIndex != -1 &&
             nullptr == GetPickedItem() && (m_pdwItemCheckBox[m_iPointedSquareIndex - m_nIndexOffset] > 1) &&
             g_pNewUIMng)
    {
        ITEM* pItem = this->FindItem(m_iPointedSquareIndex);
        if (pItem != nullptr && pItem != m_pToolTipItem)
        {
            CreateItemToolTip(pItem);

            if ((pItem->Type == ITEM_DARK_HORSE_ITEM) || (pItem->Type == ITEM_DARK_RAVEN_ITEM))
            {
                const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[m_pToolTipItem->Type];
                const UI::Items::GridRect box =
                    m_Geometry.CellsRect(m_pToolTipItem->x, m_pToolTipItem->y, pItemAttr->Width, 1);
                giPetManager::RequestPetInfo(static_cast<int>(box.x + box.width / 2), static_cast<int>(box.y), pItem);
            }
        }
    }
    return true;
}

bool mu::ui::window::CInventoryCtrl::Update()
{
    if (IsVisible())
    {
        UpdateProcess();
    }
    return true;
}

void mu::ui::window::CInventoryCtrl::UpdateProcess()
{
    const int iCurSquareIndex = GetIndexAtPt(MouseX, MouseY);
    if (iCurSquareIndex != m_iPointedSquareIndex)
    {
        if ((GetPickedItem() == nullptr) && (g_pMyShopInventory->IsEnableInputValueTextBox() == false))
            giPetManager::InitItemBackup();
        m_iPointedSquareIndex = iCurSquareIndex;
    }

    bool hasValidPointedItem = false;
    if (m_iPointedSquareIndex != -1)
    {
        hasValidPointedItem = this->FindItem(m_iPointedSquareIndex) != nullptr;
    }

    if (m_iPointedSquareIndex == -1 || !hasValidPointedItem)
    {
        m_EventState = EVENT_NONE;
        DeleteItemToolTip();
    }
}

namespace
{
// The stack count the original drew on an item (RenderNumberOfItem()), or 0 for none.
int StackCount(const ITEM* pItem)
{
    const int type = pItem->Type;
    const bool stack = (type >= ITEM_POTION && type <= ITEM_ANTIDOTE)
        || (type >= ITEM_JACK_OLANTERN_BLESSINGS && type <= ITEM_JACK_OLANTERN_DRINK)
        || (type >= ITEM_SMALL_SHIELD_POTION && type <= ITEM_LARGE_COMPLEX_POTION)
        || (type >= ITEM_POTION + 70 && type <= ITEM_POTION + 71) || type == ITEM_POTION + 94
        || (type >= ITEM_POTION + 78 && type <= ITEM_POTION + 82)
        || (type >= ITEM_CHERRY_BLOSSOM_WINE && type <= ITEM_GOLDEN_CHERRY_BLOSSOM_BRANCH)
        || type == ITEM_POTION + 133;
    if (stack && pItem->Durability > 1)
        return pItem->Durability;
    if (COMGEM::isCompiledGem(pItem))
        return (pItem->Level + 1) * COMGEM::FIRST;
    return 0;
}

const char* TintClass(BYTE colorState)
{
    switch (colorState)
    {
    case ITEM_COLOR_DURABILITY_50: return "tint-durability-50";
    case ITEM_COLOR_DURABILITY_70: return "tint-durability-70";
    case ITEM_COLOR_DURABILITY_80: return "tint-durability-80";
    case ITEM_COLOR_DURABILITY_100: return "tint-durability-100";
    case ITEM_COLOR_TRADE_WARNING: return "tint-untradeable";
    default: return "tint-normal";
    }
}

} // namespace

// Whether dropping `pPickItem` on `pTargetItem` acts on it (Render()'s green cell).
bool mu::ui::window::CInventoryCtrl::DropActsOn(ITEM* pPickItem, ITEM* pTargetItem)
{
    bool bSuccess = false;
    const int iType = pTargetItem->Type;
    const int iDurability = pTargetItem->Durability;

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
            bSuccess = true;
    }

    if (pPickItem->Type == ITEM_JEWEL_OF_BLESS && iType == ITEM_HORN_OF_FENRIR && iDurability != 255)
        bSuccess = true;

    if (bSuccess == false && m_pOwner == g_pMyInventory)
        bSuccess = AreItemsStackable(pPickItem, pTargetItem);
    if (Check_LuckyItem(pTargetItem->Type))
    {
        bSuccess = false;
        if (pPickItem->Type == ITEM_POTION + 161)
        {
            if (pTargetItem->Jewel_Of_Harmony_Option == 0)
                bSuccess = true;
        }
        else if (pPickItem->Type == ITEM_POTION + 160)
        {
            if (pTargetItem->Durability > 0)
                bSuccess = true;
        }
    }
    return bSuccess;
}

// Render()'s cell pass, as state: the tint under each item, the drop preview under the item on the
// cursor, and each item's stack count.
void mu::ui::window::CInventoryCtrl::UpdateCells()
{
    m_Cells.assign(static_cast<std::size_t>(m_nColumn * m_nRow), UI::Items::ItemGridCell{});

    for (int iCurSquareIndex = 0; iCurSquareIndex < m_nColumn * m_nRow; ++iCurSquareIndex)
    {
        const DWORD slotKey = m_pdwItemCheckBox[iCurSquareIndex];
        if (slotKey <= 1)
            continue;
        ITEM* pItem = FindItemByKey(slotKey);
        if (pItem == nullptr)
        {
            ClearSlotKey(slotKey);
            RequestInventoryRefresh();
            continue;
        }
        if (CanChangeItemColorState(pItem) == true)
            SetItemColorState(pItem);
        m_Cells[iCurSquareIndex].tint = TintClass(pItem->byColorState);
    }

    for (const ITEM* pItem : m_vecItem)
    {
        const int count = StackCount(pItem);
        if (count <= 0)
            continue;
        const int topRight = pItem->y * m_nColumn + pItem->x + ItemAttribute[pItem->Type].Width - 1;
        if (topRight >= 0 && topRight < static_cast<int>(m_Cells.size()))
            m_Cells[topRight].count = std::to_string(count);
    }

    m_bCanPushItem = false;
    if (ms_pPickedItem == nullptr || !ms_pPickedItem->IsVisible())
        return;

    // The picked item's own position is in its owner window's space; every placed window has its
    // own, so take the item's box from the pointer in this grid's space.
    const POINT itemTopLeft = ms_pPickedItem->TopLeftIn(this, MouseX, MouseY);
    const SIZE pickedSize = ms_pPickedItem->SizeIn(this);
    RECT rcPickedItem{itemTopLeft.x, itemTopLeft.y, itemTopLeft.x + pickedSize.cx, itemTopLeft.y + pickedSize.cy};
    RECT rcInventory, rcIntersect;
    GetRect(rcInventory);
    if (!IntersectRect(&rcIntersect, &rcPickedItem, &rcInventory))
        return;

    ITEM* pPickItem = ms_pPickedItem->GetItem();
    const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pPickItem->Type];
    int iColumnX = 0, iRowY = 0;
    int nItemColumn = pItemAttr->Width, nItemRow = pItemAttr->Height;
    m_Geometry.CellOf(static_cast<float>(itemTopLeft.x), static_cast<float>(itemTopLeft.y), iColumnX, iRowY);

    bool bWarning = false;
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
    m_bCanPushItem = bWarning;
    if (iColumnX < 0 || iColumnX >= m_nColumn || iRowY < 0 || iRowY >= m_nRow)
        return;

    // The window's "normal" colour turns red where it refuses the item (the mix and lucky item
    // windows set it), and a box hanging off the grid is all red.
    const bool blocked = m_afColorStateNormal[0] >= 0.9f && m_afColorStateNormal[1] <= 0.3f;
    for (int y = 0; y < nItemRow; y++)
    {
        for (int x = 0; x < nItemColumn; x++)
        {
            const int iCurSquareIndex = (iRowY + y) * m_nColumn + (iColumnX + x);
            const char* state = blocked ? "drop-blocked" : "drop-free";
            if (bWarning)
            {
                state = "drop-invalid";
            }
            else if (m_pdwItemCheckBox[iCurSquareIndex] > 1)
            {
                ITEM* pTargetItem = FindItemByKey(m_pdwItemCheckBox[iCurSquareIndex]);
                state = pTargetItem && DropActsOn(pPickItem, pTargetItem) ? "drop-valid" : "drop-invalid";
            }
            m_Cells[iCurSquareIndex].drop = state;
        }
    }
}

void mu::ui::window::CInventoryCtrl::Render()
{
    UpdateCells();
    if (m_pToolTipItem && GetPickedItem() == nullptr)
        RenderItemToolTip();
}

void mu::ui::window::CInventoryCtrl::SetPos(int x, int y)
{
    m_Geometry = {static_cast<float>(x), static_cast<float>(y), m_Geometry.PitchX(), m_Geometry.PitchY(),
                  m_Geometry.Columns(), m_Geometry.Rows()};
}

POINT mu::ui::window::CInventoryCtrl::GetPos() const
{
    return {static_cast<LONG>(std::lround(m_Geometry.Left())), static_cast<LONG>(std::lround(m_Geometry.Top()))};
}

void mu::ui::window::CInventoryCtrl::FollowGrid(Rml::ElementDocument* doc, const char* gridId, const POINT& panelPos,
                                                int offsetX, int offsetY)
{
    float x = static_cast<float>(panelPos.x + offsetX);
    float y = static_cast<float>(panelPos.y + offsetY);
    UI::RmlBridge::RefreshLogicalAnchorPosition(doc, "panel", gridId, panelPos, x, y);

    // A cell's margin box, so a theme may space its cells apart.
    float pitchX = m_Geometry.PitchX();
    float pitchY = m_Geometry.PitchY();
    Rml::Element* grid = doc ? doc->GetElementById(gridId) : nullptr;
    if (Rml::Element* cell = grid ? grid->QuerySelector(".item-cell") : nullptr)
    {
        const Rml::Vector2f size = cell->GetBox().GetSize(Rml::BoxArea::Margin);
        if (size.x > 0.f && size.y > 0.f)
        {
            pitchX = size.x;
            pitchY = size.y;
        }
    }

    m_Geometry = {std::round(x), std::round(y), pitchX, pitchY, m_nColumn, m_nRow};
}

int mu::ui::window::CInventoryCtrl::GetNumberOfColumn() const
{
    return m_nColumn;
}

int mu::ui::window::CInventoryCtrl::GetNumberOfRow() const
{
    return m_nRow;
}

void mu::ui::window::CInventoryCtrl::GetRect(RECT& rcBox)
{
    const UI::Items::GridRect box = m_Geometry.Bounds();
    rcBox.left = static_cast<LONG>(std::lround(box.x));
    rcBox.top = static_cast<LONG>(std::lround(box.y));
    rcBox.right = static_cast<LONG>(std::lround(box.x + box.width));
    rcBox.bottom = static_cast<LONG>(std::lround(box.y + box.height));
}

CInventoryCtrl::EVENT_STATE mu::ui::window::CInventoryCtrl::GetEventState()
{
    return m_EventState;
}

CObject* mu::ui::window::CInventoryCtrl::GetOwner() const
{
    return m_pOwner;
}

CObject* mu::ui::window::CInventoryCtrl::GetLayoutOwner() const
{
    return m_pOwner;
}

bool mu::ui::window::CInventoryCtrl::IsVisible() const
{
    if (m_pOwner)
        return (m_pOwner->IsVisible() && m_bShow);
    return m_bShow;
}

void mu::ui::window::CInventoryCtrl::ShowInventory()
{
    m_bShow = true;
}

void mu::ui::window::CInventoryCtrl::HideInventory()
{
    m_bShow = false;
}

bool mu::ui::window::CInventoryCtrl::IsLocked() const
{
    return m_bLock;
}

void mu::ui::window::CInventoryCtrl::LockInventory()
{
    m_bLock = true;
}

void mu::ui::window::CInventoryCtrl::UnlockInventory()
{
    m_bLock = false;
}

int mu::ui::window::CInventoryCtrl::GetIndexAtPt(int x, int y)
{
    int iColumnX, iRowY;
    if (GetSquarePosAtPt(x, y, iColumnX, iRowY))
        return iRowY * m_nColumn + iColumnX + m_nIndexOffset;
    return -1;
}

bool mu::ui::window::CInventoryCtrl::GetSquarePosAtPt(int x, int y, int& iColumnX, int& iRowY)
{
    return m_Geometry.CellAt(static_cast<float>(x), static_cast<float>(y), iColumnX, iRowY);
}

bool mu::ui::window::CInventoryCtrl::CheckSlot(int startIndex, int width, int height)
{
    if (!UI::Items::Grid::Fits(startIndex, width, height, m_nColumn, m_nRow))
    {
        return false;
    }

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            const int iIndex = startIndex + (y * m_nColumn) + x;

            if (iIndex >= (m_nColumn * m_nRow))
            {
                return false;
            }

            const DWORD slotKey = m_pdwItemCheckBox[iIndex];
            if (slotKey != 0)
            {
                if (slotKey == 1 || this->FindItemByKey(slotKey) != nullptr)
                {
                    return false;
                }

                this->ClearSlotKey(slotKey);
                this->RequestInventoryRefresh();
            }
        }
    }

    return true;
}

bool mu::ui::window::CInventoryCtrl::CheckSlot(int iColumnX, int iRowY, int width, int height)
{
    const int iIndex = iRowY * m_nColumn + iColumnX;
    return CheckSlot(iIndex, width, height);
}

int CInventoryCtrl::GetIndex(int column, int row)
{
    return column + row * m_nColumn + m_nIndexOffset;
}

bool mu::ui::window::CInventoryCtrl::CheckPtInRect(int x, int y)
{
    return m_Geometry.Contains(static_cast<float>(x), static_cast<float>(y));
}

bool mu::ui::window::CInventoryCtrl::CheckRectInRect(const RECT& rcBox)
{
    RECT rcSquare;
    GetRect(rcSquare);

    if (rcBox.left >= rcSquare.left && rcBox.right <= rcSquare.right && rcBox.top >= rcSquare.top &&
        rcBox.bottom <= rcSquare.bottom)
        return true;
    return false;
}

bool mu::ui::window::CInventoryCtrl::CanMove(int iLinealPos, ITEM* pItem)
{
    const auto startIndex = iLinealPos - m_nIndexOffset;
    if (startIndex < 0 || startIndex >= m_nColumn * m_nRow)
    {
        return false;
    }

    const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
    return CheckSlot(startIndex, pItemAttr->Width, pItemAttr->Height);
}

bool mu::ui::window::CInventoryCtrl::CanMove(int iColumnX, int iRowY, ITEM* pItem)
{
    const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
    return CheckSlot(iColumnX, iRowY, pItemAttr->Width, pItemAttr->Height);
}

bool mu::ui::window::CInventoryCtrl::CanMoveToPt(int x, int y, ITEM* pItem)
{
    int iColumnX, iRowY;
    if (GetSquarePosAtPt(x, y, iColumnX, iRowY))
        return CanMove(iColumnX, iRowY, pItem);
    return false;
}

void mu::ui::window::CInventoryCtrl::SetToolTipType(TOOLTIP_TYPE ToolTipType)
{
    m_ToolTipType = ToolTipType;
}

void mu::ui::window::CInventoryCtrl::CreateItemToolTip(ITEM* pItem)
{
    if (m_pToolTipItem)
        DeleteItemToolTip();

    if (g_pNewItemMng)
        m_pToolTipItem = g_pNewItemMng->CreateItem(pItem);
}

void mu::ui::window::CInventoryCtrl::DeleteItemToolTip()
{
    if (m_pToolTipItem && g_pNewItemMng)
    {
        g_pNewItemMng->DeleteItem(m_pToolTipItem);
        m_pToolTipItem = nullptr;
    }
    UI::Tooltip::HideLegacyTextList();
}

void mu::ui::window::CInventoryCtrl::SetRepairMode(bool bRepair)
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

bool mu::ui::window::CInventoryCtrl::IsRepairMode()
{
    return m_bRepairMode;
}

void mu::ui::window::CInventoryCtrl::RenderItemToolTip()
{
    if (m_pToolTipItem)
    {
        const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[m_pToolTipItem->Type];
        const UI::Items::GridRect box = m_Geometry.CellsRect(m_pToolTipItem->x, m_pToolTipItem->y, pItemAttr->Width, 1);
        const int iTargetX = static_cast<int>(box.x + box.width / 2);
        int iTargetY = static_cast<int>(box.y);

        if (pItemAttr->Height == 1)
        {
            iTargetY += static_cast<int>(box.height / 2);
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

CPickedItem* mu::ui::window::CInventoryCtrl::GetPickedItem()
{
    return ms_pPickedItem;
}

bool mu::ui::window::CInventoryCtrl::CreatePickedItem(CInventoryCtrl* pSrc, ITEM* pItem,
                                                     bool preservePickupAnchor)
{
    if (g_pNewItemMng)
    {
        ms_pPickedItem = new CPickedItem;
        return ms_pPickedItem->Create(g_pNewItemMng, pSrc, pItem, preservePickupAnchor);
    }
    return false;
}

void mu::ui::window::CInventoryCtrl::DeletePickedItem()
{
    if (ms_pPickedItem)
    {
        CInventoryCtrl* pOwner = ms_pPickedItem->GetOwnerInventory();
        if (pOwner)
        {
            pOwner->SetEventState(CInventoryCtrl::EVENT_NONE);
        }
    }

    SAFE_DELETE(ms_pPickedItem);
}

void mu::ui::window::CInventoryCtrl::BackupPickedItem()
{
    if (ms_pPickedItem && EquipmentItem == false)
    {
        CInventoryCtrl* pOwner = ms_pPickedItem->GetOwnerInventory();
        ITEM* pItemObj = ms_pPickedItem->GetItem();
        if (pOwner)
        {
            if (pOwner->AddItem(pItemObj->x, pItemObj->y, pItemObj))
            {
                DeletePickedItem();
            }
            else
            {
                pOwner->RequestInventoryRefresh();
            }
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

void mu::ui::window::CInventoryCtrl::SetEventState(EVENT_STATE es)
{
    m_EventState = es;
}

void mu::ui::window::CInventoryCtrl::Render3D()
{
    auto li = m_vecItem.begin();
    for (; li != m_vecItem.end(); ++li)
    {
        const ITEM* pItem = (*li);
        const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];

        const UI::Items::GridRect box = m_Geometry.CellsRect(pItem->x, pItem->y, pItemAttr->Width, pItemAttr->Height);

        RenderItem3D(box.x, box.y, box.width, box.height, pItem->Type, pItem->Level, pItem->ExcellentFlags, pItem->AncientDiscriminator,
                     false);
    }
}

bool mu::ui::window::CInventoryCtrl::AreItemsStackable(ITEM* pSourceItem, ITEM* pTargetItem)
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

    if (iSrcType == ITEM_SIEGE_POTION && iTarType == ITEM_SIEGE_POTION &&
        (iSrcDurability < 250 && iTarDurability < 250))
    {
        return true;
    }

    if ((iSrcType >= ITEM_POTION && iSrcType <= ITEM_ANTIDOTE && iSrcType != ITEM_SIEGE_POTION) &&
        (iTarType >= ITEM_POTION && iTarType <= ITEM_ANTIDOTE && iTarType != ITEM_SIEGE_POTION) &&
        (iSrcDurability < 3 && iTarDurability < 3))
    {
        return true;
    }

    if ((iSrcType >= ITEM_SMALL_COMPLEX_POTION && iSrcType <= ITEM_LARGE_COMPLEX_POTION) &&
        (iTarType >= ITEM_SMALL_COMPLEX_POTION && iTarType <= ITEM_LARGE_COMPLEX_POTION) &&
        (iSrcDurability < 3 && iTarDurability < 3))
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

    if ((iSrcType >= ITEM_SPLINTER_OF_ARMOR && iSrcType <= ITEM_CLAW_OF_BEAST) &&
        (iTarType >= ITEM_SPLINTER_OF_ARMOR && iTarType <= ITEM_CLAW_OF_BEAST))
    {
        return true;
    }

    if ((iSrcType >= ITEM_JACK_OLANTERN_BLESSINGS && iSrcType <= ITEM_JACK_OLANTERN_DRINK) &&
        (iTarType >= ITEM_JACK_OLANTERN_BLESSINGS && iTarType <= ITEM_JACK_OLANTERN_DRINK) &&
        (iSrcDurability < 3 && iTarDurability < 3))
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

    if (iSrcType == ITEM_CHERRY_BLOSSOM_WINE && iTarType == ITEM_CHERRY_BLOSSOM_WINE &&
        (iSrcDurability < 3 && iTarDurability < 3))
    {
        return true;
    }

    if (iSrcType == ITEM_CHERRY_BLOSSOM_RICE_CAKE && iTarType == ITEM_CHERRY_BLOSSOM_RICE_CAKE &&
        (iSrcDurability < 3 && iTarDurability < 3))
    {
        return true;
    }

    if (iSrcType == ITEM_CHERRY_BLOSSOM_FLOWER_PETAL && iTarType == ITEM_CHERRY_BLOSSOM_FLOWER_PETAL &&
        (iSrcDurability < 3 && iTarDurability < 3))
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

    if (iSrcType == ITEM_GOLDEN_CHERRY_BLOSSOM_BRANCH && iTarType == ITEM_GOLDEN_CHERRY_BLOSSOM_BRANCH &&
        (iSrcDurability < 50 && iTarDurability < 50))
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

    if (iSrcType == ITEM_SUSPICIOUS_SCRAP_OF_PAPER && iTarType == ITEM_SUSPICIOUS_SCRAP_OF_PAPER &&
        (iSrcDurability < 5 && iTarDurability < 5))
    {
        return true;
    }

    if (iSrcType == ITEM_POTION + 133 && (iSrcDurability < 50 && iTarDurability < 50))
    {
        return true;
    }

    return false;
}

bool mu::ui::window::CInventoryCtrl::CanPushItem()
{
    return m_bCanPushItem;
}

bool mu::ui::window::CInventoryCtrl::CanUpgradeItem(ITEM* pSourceItem, ITEM* pTargetItem)
{
    const int iTargetLevel = pTargetItem->Level;

    if (((pTargetItem->Type >= ITEM_SWORD && pTargetItem->Type < ITEM_WING) && (pTargetItem->Type != ITEM_BOLT) &&
         (pTargetItem->Type != ITEM_ARROWS)) ||
        (pTargetItem->Type >= ITEM_WING && pTargetItem->Type <= ITEM_WINGS_OF_DARKNESS) ||
        (pTargetItem->Type >= ITEM_WING_OF_STORM && pTargetItem->Type <= ITEM_WING_OF_DIMENSION))
    {
        if ((pSourceItem->Type == ITEM_JEWEL_OF_BLESS) && (iTargetLevel >= 0 && iTargetLevel <= 5))
        {
            return true;
        }

        if ((pSourceItem->Type == ITEM_JEWEL_OF_SOUL) && (iTargetLevel >= 0 && iTargetLevel <= 8))
        {
            return true;
        }
    }

    return false;
}
