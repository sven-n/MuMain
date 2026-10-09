//*****************************************************************************
// File: NewUIStorageInventory.cpp
//*****************************************************************************

#include "stdafx.h"
#include "UI/Inventory/StorageInventoryExt.h"
#include "I18N/All.h"

#include "Audio/DSPlaySound.h"
#include "UI/Core/WindowSystem.h"
#include "UI/RmlBridge/RmlPointer.h"
#include "UI/RmlBridge/RmlWindowClose.h"
#include "UI/Dialogs/CustomMessageBox.h"
#include "Engine/Object/ZzzInventory.h"
#include "UI/Inventory/MyInventory.h"

// RmlUi migration -- see this class's header comment.
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/Scaling/UITransform.h"
#include "Core/Utilities/StringUtils.h"
#include <RmlUi/Core/ElementDocument.h>

using namespace SEASON3B;
using namespace mu::ui::window;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
// cppcheck-suppress uninitMemberVar
CStorageInventoryExt::CStorageInventoryExt()
{
    m_pNewUIMng = nullptr;
    m_pNewInventoryCtrl = nullptr;
    m_Pos.x = m_Pos.y = 0;
    m_nBackupSourceInvenIndex = -1;
}

CStorageInventoryExt::~CStorageInventoryExt()
{
    Release();
}

bool CStorageInventoryExt::Create(CManager* pNewUIMng, int x, int y)
{
    if (nullptr == pNewUIMng || nullptr == g_pNewItemMng)
    {
        return false;
    }

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(INTERFACE_STORAGE_EXT, this);

    m_pNewInventoryCtrl = new CInventoryCtrl;
    if (false == m_pNewInventoryCtrl->Create(STORAGE_TYPE::VAULT, g_pNewItemMng, this, x + 15,
                                             y + 36, 8, 15, MAX_SHOP_INVENTORY))
    {
        SAFE_DELETE(m_pNewInventoryCtrl);
        return false;
    }

    SetPos(x, y);
    SetItemAutoMove(false);

    BuildRmlUi();

    Show(false);

    return true;
}

void CStorageInventoryExt::BindRmlModel(Rml::DataModelConstructor& c, StorageExtRmlModel& model)
{
    UI::RmlBridge::BindWindowClose(c, [] { g_pNewUISystem->Hide(INTERFACE_STORAGE); });
    UI::Items::RegisterItemGridCells(c);
    c.Bind("grid_cells", &model.gridCells);
    c.Bind("text_px", &model.textPx);
    c.Bind("title", &model.title);
    c.Bind("exit_tooltip", &model.exitTooltip);

    c.BindEventCallback("storage_ext_exit_click",
        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
        {
            g_pNewUISystem->Hide(INTERFACE_STORAGE_EXT);
        });
}

void CStorageInventoryExt::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void CStorageInventoryExt::Release()
{
    m_ItemTarget.Disable();
    SAFE_DELETE(m_pNewInventoryCtrl);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = nullptr;
    }

    m_RmlView.Release();
}

void CStorageInventoryExt::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CStorageInventoryExt::IsPointerOverPanel()
{
    // #panel takes no pointer events, so the grid's clicks stay native; its drawn box still holds
    // the pointer.
    Rml::ElementDocument* document = m_RmlView.Document();
    return UI::RmlBridge::IsPointerWithin(document != nullptr ? document->GetElementById("panel") : nullptr);
}

bool CStorageInventoryExt::UpdateMouseEvent()
{
    if (m_pNewInventoryCtrl && false == m_pNewInventoryCtrl->UpdateMouseEvent())
        return false;

    ProcessInventoryCtrl();

    if (IsPointerOverPanel())
    {
        if (IsPress(VK_RBUTTON))
        {
            MouseRButton = false;
            MouseRButtonPop = false;
            MouseRButtonPush = false;
            return false;
        }

        if (IsNone(VK_LBUTTON) == false)
        {
            return false;
        }
    }

    return true;
}

bool CStorageInventoryExt::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(INTERFACE_STORAGE) == true)
    {
        if (IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(INTERFACE_STORAGE);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }
    return true;
}

bool CStorageInventoryExt::Update()
{
    if (m_pNewInventoryCtrl && false == m_pNewInventoryCtrl->Update())
        return false;

    SyncRmlModel();
    return true;
}

bool CStorageInventoryExt::Render()
{
    EnableAlphaTest();

    if (m_pNewInventoryCtrl)
    {
        m_pNewInventoryCtrl->Render();
    }

    DisableAlphaBlend();

    return true;
}

void CStorageInventoryExt::SyncRmlModel()
{
    m_ItemTarget.Sync(m_RmlView.Document() ? m_RmlView.Document()->GetElementById("item_view") : nullptr, IsVisible());
    if (!m_RmlView.Document()) return;
    UI::RmlBridge::SyncDocumentVisibility(m_RmlView.Document(), IsVisible());

    if (m_pNewInventoryCtrl)
        m_pNewInventoryCtrl->FollowGridPx(m_RmlView.Document(), "item_grid");
    if (m_pNewInventoryCtrl && m_RmlView.GetModel().gridCells != m_pNewInventoryCtrl->Cells())
    {
        m_RmlView.GetModel().gridCells = m_pNewInventoryCtrl->Cells();
        m_RmlView.MarkDirty("grid_cells");
    }
    UI::RmlBridge::SyncNativeTextSize(m_RmlView.Binder());

    auto& model = m_RmlView.GetModel();
    auto syncWide = [&](Rml::String StorageExtRmlModel::* field, const char* boundName, const wchar_t* text)
    {
        const Rml::String value = StringUtils::WideToNarrow(text);
        if (model.*field != value) { model.*field = value; m_RmlView.MarkDirty(boundName); }
    };

    syncWide(&StorageExtRmlModel::title, "title", I18N::Game::ExpandedVault);
    syncWide(&StorageExtRmlModel::exitTooltip, "exit_tooltip", I18N::Game::Close388);
}

float CStorageInventoryExt::GetLayerDepth()
{
    return 2.2f;
}

CInventoryCtrl* CStorageInventoryExt::GetInventoryCtrl() const
{
    return m_pNewInventoryCtrl;
}

bool CStorageInventoryExt::ProcessClosing() const
{
    if (EquipmentItem)
        return false;

    CInventoryCtrl::BackupPickedItem();
    DeleteAllItems();
    SocketClient->ToGameServer()->SendVaultClosed();
    return true;
}

bool CStorageInventoryExt::InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket) const
{
    if (m_pNewInventoryCtrl)
        return m_pNewInventoryCtrl->AddItem(iIndex, pbyItemPacket);

    return false;
}

void CStorageInventoryExt::DeleteAllItems() const
{
    if (m_pNewInventoryCtrl)
        m_pNewInventoryCtrl->RemoveAllItems();
}

void CStorageInventoryExt::ProcessInventoryCtrl()
{
    if (nullptr == m_pNewInventoryCtrl)
    {
        return;
    }

    if (const auto pPickedItem = CInventoryCtrl::GetPickedItem())
    {
        ITEM* pItemObj = pPickedItem->GetItem();
        if (pItemObj == nullptr)
        {
            return;
        }

        if (IsPress(VK_LBUTTON) || IsRelease(VK_LBUTTON))
        {
            const int nDstIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);

            if (nDstIndex >= 0 && m_pNewInventoryCtrl->CanMove(nDstIndex, pItemObj))
            {
                const int nSrcIndex = pPickedItem->GetSourceLinealPos();
                const auto sourceStorageType = pPickedItem->GetSourceStorageType();
                const auto targetStorageType = m_pNewInventoryCtrl->GetStorageType();
                SendRequestEquipmentItem(sourceStorageType, nSrcIndex, pItemObj, targetStorageType, nDstIndex);
            }
        }
        else
        {
            if (::IsStoreBan(pItemObj))
            {
                m_pNewInventoryCtrl->SetSquareColorNormal(1.0f, 0.0f, 0.0f);
            }
            else
            {
                m_pNewInventoryCtrl->SetSquareColorNormal(0.1f, 0.4f, 0.8f);
            }
        }
    }
    else if (IsPress(VK_RBUTTON))
    {
        ProcessStorageItemAutoMove();
    }
}

void CStorageInventoryExt::ProcessStorageItemAutoMove()
{
    if (g_pPickedItem)
        if (g_pPickedItem->GetItem())
            return;

    if (IsItemAutoMove())
        return;

    if (const auto pItemObj = m_pNewInventoryCtrl->FindItemAtPointer())
    {
        const int nDstIndex = g_pMyInventory->FindEmptySlotIncludingExtensions(pItemObj);
        if (-1 != nDstIndex)
        {
            SetItemAutoMove(true);

            const int nSrcIndex = m_pNewInventoryCtrl->GetIndexByItem(pItemObj);
            g_pStorageInventory->SendRequestItemToMyInven(pItemObj, nSrcIndex, nDstIndex);

            PlayBuffer(SOUND_GET_ITEM01);
        }
    }
}

bool CStorageInventoryExt::ProcessMyInvenItemAutoMove(CInventoryCtrl* sourceCtrl)
{
    if (g_pPickedItem && g_pPickedItem->GetItem())
    {
        return false;
    }

    if (IsItemAutoMove())
    {
        return false;
    }

    if (sourceCtrl == nullptr)
    {
        sourceCtrl = g_pMyInventory->GetInventoryCtrl();
    }

    if (sourceCtrl == nullptr)
    {
        return false;
    }

    if (const auto pItemObj = sourceCtrl->FindItemAtPointer())
    {
        if (pItemObj->Type == ITEM_WIZARDS_RING)
            return false;

        const int emptySlotIndex = FindEmptySlot(pItemObj);
        if (emptySlotIndex != -1)
        {
            const int nSrcIndex = sourceCtrl->GetIndexByItem(pItemObj);
            if (nSrcIndex < 0)
            {
                return false;
            }

            SetItemAutoMove(true, nSrcIndex);
            g_pStorageInventory->SendRequestItemToStorage(pItemObj, nSrcIndex, emptySlotIndex);
            PlayBuffer(SOUND_GET_ITEM01);
            return true;
        }
    }

    return false;
}

void CStorageInventoryExt::SetItemAutoMove(bool bItemAutoMove, int nSourceInvenIndex)
{
    m_bItemAutoMove = bItemAutoMove;

    if (bItemAutoMove)
    {
        // The cells under the pointer now, in each grid's own space, for the server's answer.
        m_nBackupStorageCell = m_pNewInventoryCtrl->GetIndexAtPointer();
        m_nBackupInventoryCell = g_pMyInventory->GetInventoryCtrl()->GetIndexAtPointer();
        m_nBackupSourceInvenIndex = nSourceInvenIndex;
    }
    else
    {
        m_nBackupStorageCell = m_nBackupInventoryCell = -1;
        m_nBackupSourceInvenIndex = -1;
    }
}

int CStorageInventoryExt::FindEmptySlot(const ITEM* pItemObj) const
{
    if (pItemObj == nullptr)
        return -1;

    const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItemObj->Type];

    if (m_pNewInventoryCtrl)
        return m_pNewInventoryCtrl->FindEmptySlot(pItemAttr->Width, pItemAttr->Height);

    return -1;
}

void CStorageInventoryExt::ProcessToReceiveStorageItems(int nIndex, std::span<const BYTE> pbyItemPacket)
{
    CInventoryCtrl::DeletePickedItem();

    if (IsItemAutoMove())
    {
        if (m_nBackupSourceInvenIndex >= MAX_EQUIPMENT_INDEX && m_nBackupSourceInvenIndex < MAX_MY_INVENTORY_INDEX)
        {
            g_pMyInventory->DeleteItem(m_nBackupSourceInvenIndex);
        }
        else if (m_nBackupSourceInvenIndex >= MAX_MY_INVENTORY_INDEX &&
                 m_nBackupSourceInvenIndex < MAX_MY_INVENTORY_EX_INDEX)
        {
            g_pMyInventoryExt->DeleteItem(m_nBackupSourceInvenIndex);
        }
        else
        {
            CInventoryCtrl* pMyInvenCtrl = g_pMyInventory->GetInventoryCtrl();
            ITEM* pItemObj = pMyInvenCtrl->FindItem(m_nBackupInventoryCell);
            g_pMyInventory->GetInventoryCtrl()->RemoveItem(pItemObj);
        }

        SetItemAutoMove(false);
    }

    InsertItem(nIndex, pbyItemPacket);
}

void CStorageInventoryExt::ProcessStorageItemAutoMoveSuccess()
{
    if (!IsVisible())
        return;

    if (IsItemAutoMove())
    {
        ITEM* pItemObj = m_pNewInventoryCtrl->FindItem(m_nBackupStorageCell);
        m_pNewInventoryCtrl->RemoveItem(pItemObj);

        SetItemAutoMove(false);
    }
}

void CStorageInventoryExt::ProcessStorageItemAutoMoveFailure()
{
    if (!IsVisible())
        return;

    SetItemAutoMove(false);
}

int CStorageInventoryExt::GetPointedItemIndex() const
{
    return m_pNewInventoryCtrl->GetPointedSquareIndex();
}

// Into #item_view (m_ItemTarget), in window pixels (the grids' FollowGridPx()).
void CStorageInventoryExt::RenderItems()
{
    if (m_pNewInventoryCtrl && m_pNewInventoryCtrl->IsVisible())
        m_pNewInventoryCtrl->Render3D();
}
