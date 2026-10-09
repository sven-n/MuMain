#include "stdafx.h"
#include "UI/Inventory/InventoryExtension.h"
#include "I18N/All.h"

#include "UI/Core/WindowSystem.h"
#include "UI/RmlBridge/RmlPointer.h"
#include "UI/RmlBridge/RmlWindowClose.h"

// RmlUi migration -- see this class's header comment.
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlNativeTextSize.h"
#include "UI/Scaling/UITransform.h"
#include "Core/Utilities/StringUtils.h"
#include <RmlUi/Core/ElementDocument.h>

using namespace SEASON3B;
using namespace mu::ui::window;

// cppcheck-suppress uninitMemberVar
CInventoryExtension::CInventoryExtension()
{
    Init();
}

CInventoryExtension::~CInventoryExtension()
{
    Release();
}

void CInventoryExtension::Init()
{
    m_pNewUIMng = nullptr;
    m_Pos.x = m_Pos.y = 0;
}

bool CInventoryExtension::Create(CManager* pNewUIMng, int x, int y)
{
    if (nullptr == pNewUIMng || nullptr == g_pNewItemMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(INTERFACE_INVENTORY_EXT, this);

    // Creates all 4 extension boxes upfront; only the ones unlocked for the character are used.
    int i = 0;
    for (auto& m_extension : m_extensions)
    {
        m_extension = new CInventoryCtrl();

        const int indexOffset = MAX_MY_INVENTORY_INDEX + i * MAX_INVENTORY_EXT_ONE;
        if (false == m_extension->Create(STORAGE_TYPE::INVENTORY, g_pNewItemMng, this, x + 15,
                                         y + 45 + HEIGHT_PER_EXT * i, COLUMN_INVENTORY, ROW_INVENTORY_EXT, indexOffset))
        {
            SAFE_DELETE(m_extension);
            return false;
        }

        if (m_extension)
        {
            m_extension->SetToolTipType(TOOLTIP_TYPE_INVENTORY);
        }

        i++;
    }

    SetPos(x, y);

    BuildRmlUi();

    Show(false);

    return true;
}

void CInventoryExtension::BindRmlModel(Rml::DataModelConstructor& c, InventoryExtensionRmlModel& model)
{
    // See CCharMakeWin::BindRmlModel()'s comment on why this must re-run in full every
    // call, including on a theme switch -- no guard here.
    auto lockedPage = c.RegisterStruct<LockedExtPageEntry>();
    lockedPage.RegisterMember("number", &LockedExtPageEntry::number);
    c.RegisterArray<std::vector<LockedExtPageEntry>>();

    UI::RmlBridge::BindWindowClose(c, INTERFACE_INVENTORY_EXT);
    UI::Items::RegisterItemGridCells(c);
    c.Bind("grid_cells_1", &model.gridCells1);
    c.Bind("grid_cells_2", &model.gridCells2);
    c.Bind("grid_cells_3", &model.gridCells3);
    c.Bind("grid_cells_4", &model.gridCells4);
    c.Bind("text_px", &model.textPx);
    c.Bind("title", &model.title);
    c.Bind("exit_tooltip", &model.exitTooltip);
    c.Bind("locked_pages", &model.lockedPages);

    c.BindEventCallback("inventory_extension_exit_click",
        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
        {
            g_pNewUISystem->Hide(INTERFACE_INVENTORY_EXT);
        });
}

void CInventoryExtension::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void CInventoryExtension::Release()
{
    m_ItemTarget.Disable();

    for (auto extension : m_extensions)
    {
        if (extension)
        {
            SAFE_DELETE(extension);
        }
    }

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = nullptr;
    }

    m_RmlView.Release();
}

void CInventoryExtension::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CInventoryExtension::UpdateMouseEvent()
{
    for (int i = 0; i < CharacterAttribute->InventoryExtensions; i++)
    {
        if (const auto m_extension = m_extensions[i])
        {
            if (!m_extension->UpdateMouseEvent())
            {
                return false;
            }

            if (InventoryProcess())
            {
                return false;
            }
        }
    }

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

bool CInventoryExtension::IsPointerOverPanel()
{
    // #panel takes no pointer events, so the grids' clicks stay native; its drawn box still holds
    // the pointer.
    Rml::ElementDocument* document = m_RmlView.Document();
    return UI::RmlBridge::IsPointerWithin(document != nullptr ? document->GetElementById("panel") : nullptr);
}

bool CInventoryExtension::InventoryProcess()
{
    if (!IsPointerOverPanel())
    {
        return false;
    }

    for (auto* extension : m_extensions)
    {
        if (extension->ContainsPointer())
        {
            return g_pMyInventory->HandleInventoryActions(extension);
        }
    }

    return false;
}

bool CInventoryExtension::UpdateKeyEvent()
{
    return true;
}

bool CInventoryExtension::Update()
{
    for (int i = 0; i < CharacterAttribute->InventoryExtensions; i++)
    {
        if (const auto& extension = m_extensions[i])
        {
            if (extension && !extension->Update())
            {
                return false;
            }
        }
    }

    SyncRmlModel();
    return true;
}

bool CInventoryExtension::Render()
{
    for (int i = 0; i < CharacterAttribute->InventoryExtensions; i++)
    {
        if (const auto& m_extension = m_extensions[i])
        {
            m_extension->Render();
        }
    }
    return true;
}

void CInventoryExtension::SyncRmlModel()
{
    m_ItemTarget.Sync(m_RmlView.Document() ? m_RmlView.Document()->GetElementById("item_view") : nullptr, IsVisible());
    if (!m_RmlView.Document()) return;
    UI::RmlBridge::SyncDocumentVisibility(m_RmlView.Document(), IsVisible());

    if (m_extensions[0])
        m_extensions[0]->FollowGridPx(m_RmlView.Document(), "item_grid_1");
    if (m_extensions[1])
        m_extensions[1]->FollowGridPx(m_RmlView.Document(), "item_grid_2");
    if (m_extensions[2])
        m_extensions[2]->FollowGridPx(m_RmlView.Document(), "item_grid_3");
    if (m_extensions[3])
        m_extensions[3]->FollowGridPx(m_RmlView.Document(), "item_grid_4");
    if (m_extensions[0] && m_RmlView.GetModel().gridCells1 != m_extensions[0]->Cells())
    {
        m_RmlView.GetModel().gridCells1 = m_extensions[0]->Cells();
        m_RmlView.MarkDirty("grid_cells_1");
    }
    if (m_extensions[1] && m_RmlView.GetModel().gridCells2 != m_extensions[1]->Cells())
    {
        m_RmlView.GetModel().gridCells2 = m_extensions[1]->Cells();
        m_RmlView.MarkDirty("grid_cells_2");
    }
    if (m_extensions[2] && m_RmlView.GetModel().gridCells3 != m_extensions[2]->Cells())
    {
        m_RmlView.GetModel().gridCells3 = m_extensions[2]->Cells();
        m_RmlView.MarkDirty("grid_cells_3");
    }
    if (m_extensions[3] && m_RmlView.GetModel().gridCells4 != m_extensions[3]->Cells())
    {
        m_RmlView.GetModel().gridCells4 = m_extensions[3]->Cells();
        m_RmlView.MarkDirty("grid_cells_4");
    }
    UI::RmlBridge::SyncNativeTextSize(m_RmlView.Binder());

    auto& model = m_RmlView.GetModel();
    auto syncWide = [&](Rml::String InventoryExtensionRmlModel::* field, const char* boundName, const wchar_t* text)
    {
        const Rml::String value = StringUtils::WideToNarrow(text);
        if (model.*field != value) { model.*field = value; m_RmlView.MarkDirty(boundName); }
    };

    syncWide(&InventoryExtensionRmlModel::title, "title", I18N::Game::ExpandedInventory);
    syncWide(&InventoryExtensionRmlModel::exitTooltip, "exit_tooltip", I18N::Game::Close388);

    // Locked-page list -- rebuilt unconditionally every call, same "rebuild every
    // frame" convention as CBuffStrip's buff list (list is tiny: at most MAX_INVENTORY_EXT_COUNT
    // entries). Reflects CharacterAttribute->InventoryExtensions, the purchased-page count.
    model.lockedPages.clear();
    for (int i = 0; i < MAX_INVENTORY_EXT_COUNT; ++i)
    {
        if (i < CharacterAttribute->InventoryExtensions)
            continue;

        // Which page it is; the theme places it and picks its marking (the pages are sparse, so
        // the number is what identifies a row, not its place in the list).
        LockedExtPageEntry entry;
        entry.number = i + 1;
        model.lockedPages.push_back(entry);
    }
    m_RmlView.MarkDirty("locked_pages");
}

float CInventoryExtension::GetLayerDepth()
{
    return 4.55;
}

CInventoryCtrl* CInventoryExtension::TryGetExtensionByInventoryIndex(int iIndex) const
{
    const auto index = iIndex - MAX_MY_INVENTORY_INDEX;
    const auto extensionIndex = index / MAX_INVENTORY_EXT_ONE;
    if (extensionIndex >= 0 && extensionIndex < MAX_INVENTORY_EXT_COUNT)
    {
        return m_extensions[extensionIndex];
    }

    return nullptr;
}

ITEM* CInventoryExtension::FindItem(int iIndex) const
{
    if (const auto& extension = TryGetExtensionByInventoryIndex(iIndex))
    {
        return extension->FindItem(iIndex);
    }
    return nullptr;
}

bool CInventoryExtension::InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket) const
{
    if (const auto& extension = TryGetExtensionByInventoryIndex(iIndex))
    {
        return extension->AddItem(iIndex, pbyItemPacket);
    }

    return false;
}

void CInventoryExtension::DeleteItem(int iIndex) const
{
    if (const auto& extension = TryGetExtensionByInventoryIndex(iIndex))
    {
        if (extension->RemoveItemAt(iIndex))
        {
            return;
        }

        if (const auto pPickedItem = CInventoryCtrl::GetPickedItem())
        {
            if (GetOwnerOf(pPickedItem) && pPickedItem->GetSourceLinealPos() == iIndex)
            {
                CInventoryCtrl::DeletePickedItem();
            }
        }
    }
}

void CInventoryExtension::DeleteAllItems() const
{
    for (auto* extension : m_extensions)
    {
        if (extension)
        {
            extension->RemoveAllItems();
        }
    }
}

int CInventoryExtension::FindEmptySlot(int cx, int cy, const CInventoryCtrl* excluded) const
{
    if (CharacterAttribute == nullptr)
    {
        return -1;
    }

    for (int i = 0; i < CharacterAttribute->InventoryExtensions; ++i)
    {
        auto* extension = m_extensions[i];
        if (extension && extension != excluded)
        {
            const int emptySlot = extension->FindEmptySlot(cx, cy);
            if (emptySlot != -1)
            {
                return emptySlot;
            }
        }
    }

    return -1;
}

CInventoryCtrl* CInventoryExtension::GetOwnerOf(const CPickedItem* pPickedItem) const
{
    if (!pPickedItem)
    {
        return nullptr;
    }

    const auto* ownerOfItem = pPickedItem->GetOwnerInventory();

    for (auto* extension : m_extensions)
    {
        if (extension == ownerOfItem)
        {
            return extension;
        }
    }

    return nullptr;
}

// Into #item_view (m_ItemTarget), in window pixels (the grids' FollowGridPx()): the items.
void CInventoryExtension::RenderItems()
{
    if (m_extensions[0] && m_extensions[0]->IsVisible())
        m_extensions[0]->Render3D();
    if (m_extensions[1] && m_extensions[1]->IsVisible())
        m_extensions[1]->Render3D();
    if (m_extensions[2] && m_extensions[2]->IsVisible())
        m_extensions[2]->Render3D();
    if (m_extensions[3] && m_extensions[3]->IsVisible())
        m_extensions[3]->Render3D();
}
