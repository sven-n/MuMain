#include "stdafx.h"
#include "UI/Inventory/InventoryExtension.h"
#include "I18N/All.h"

#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"

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
    LoadImages();

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

    c.Bind("root_x", &model.rootX);
    c.Bind("root_y", &model.rootY);
    c.Bind("root_scale", &model.rootScale);
    UI::Items::RegisterItemGridCells(c);
    c.Bind("grid_cells_1", &model.gridCells1);
    c.Bind("grid_cells_2", &model.gridCells2);
    c.Bind("grid_cells_3", &model.gridCells3);
    c.Bind("grid_cells_4", &model.gridCells4);
    c.Bind("panel_width", &model.panelWidth);
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
    UnloadImages();

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
    // Top-right corner close "X" (shared frame): hides + swallows the click.
    if (g_pNewUISystem->HandleFrameCornerClose(m_Pos, INTERFACE_INVENTORY_EXT))
        return false;

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

    // #panel's own live RCSS size is the source of truth -- WIDTH/HEIGHT only cover the first
    // frame after Create()/Show(true)/a theme switch, before RmlUi's next layout pass.
    float panelWidth = WIDTH;
    float panelHeight = HEIGHT;
    UI::RmlBridge::RefreshLogicalPanelSize(m_RmlView.Document(), "panel", panelWidth, panelHeight);
    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, static_cast<int>(panelWidth), static_cast<int>(panelHeight)).Contains(MouseX, MouseY))
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

bool CInventoryExtension::InventoryProcess()
{
    // #panel's own live RCSS size is the source of truth -- WIDTH/HEIGHT only cover the first
    // frame after Create()/Show(true)/a theme switch, before RmlUi's next layout pass.
    float panelWidth = WIDTH;
    float panelHeight = HEIGHT;
    UI::RmlBridge::RefreshLogicalPanelSize(m_RmlView.Document(), "panel", panelWidth, panelHeight);
    if (!mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, static_cast<int>(panelWidth), static_cast<int>(panelHeight)).Contains(MouseX, MouseY))
    {
        return false;
    }

    for (auto* extension : m_extensions)
    {
        if (extension->CheckPtInRect(MouseX, MouseY))
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

void CInventoryExtension::RenderFrame() const
{
    const auto x = static_cast<float>(m_Pos.x);
    const auto y = static_cast<float>(m_Pos.y);

    // Locked (not-yet-purchased) pages' table/empty-slot backing art only -- the outer frame and
    // the numbered lock glyph on top of it moved to RmlUi (see this class's header comment).
    for (int i = MAX_INVENTORY_EXT_COUNT - 1; i >= CharacterAttribute->InventoryExtensions; --i)
    {
        RenderImage(IMAGE_EXTENSION_TABLE, x + 11, y + 42 + i * HEIGHT_PER_EXT, 173, HEIGHT_PER_EXT);
        RenderImage(IMAGE_EXTENSION_EMPTY, x + 15, y + 45 + i * HEIGHT_PER_EXT, 161, HEIGHT_PER_EXT - (EXT_BORDER * 2));
    }
}

void CInventoryExtension::SyncRmlModel()
{
    m_ItemTarget.Sync(m_RmlView.Document() ? m_RmlView.Document()->GetElementById("item_view") : nullptr, IsVisible());
    if (!m_RmlView.Document()) return;
    UI::RmlBridge::SyncDocumentVisibility(m_RmlView.Document(), IsVisible());

    UI::RmlBridge::SyncRootTransform(m_RmlView.Binder(), m_Pos);
    UI::RmlBridge::SyncPanelWidth(m_RmlView.Binder(), m_RmlView.Document());
    if (m_extensions[0])
        m_extensions[0]->FollowGrid(m_RmlView.Document(), "item_grid_1", m_Pos, 15, 45);
    if (m_extensions[1])
        m_extensions[1]->FollowGrid(m_RmlView.Document(), "item_grid_2", m_Pos, 15, 132);
    if (m_extensions[2])
        m_extensions[2]->FollowGrid(m_RmlView.Document(), "item_grid_3", m_Pos, 15, 219);
    if (m_extensions[3])
        m_extensions[3]->FollowGrid(m_RmlView.Document(), "item_grid_4", m_Pos, 15, 306);
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

    // Locked-page lock glyph list -- rebuilt unconditionally every call, same "rebuild every
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

void CInventoryExtension::LoadImages()
{
    // Frame/exit-button art moved to RmlUi (inventory_extension.rcss).
    // Numbered lock glyphs moved to RmlUi too -- only the locked page's table/empty-slot backing
    // art stays native (see this class's header comment).
    LoadBitmap(L"Interface\\newui_item_add_marking_non.jpg", IMAGE_EXTENSION_EMPTY, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_add_table.tga", IMAGE_EXTENSION_TABLE, GL_LINEAR);
}

void CInventoryExtension::UnloadImages()
{
    DeleteBitmap(IMAGE_EXTENSION_EMPTY);
    DeleteBitmap(IMAGE_EXTENSION_TABLE);
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

// Into #item_view (m_ItemTarget), in this window's layout space: the locked extensions' art, then
// the items.
void CInventoryExtension::RenderItems()
{
    DisableDepthTest();
    EnableAlphaTest();
    RenderFrame();
    DisableAlphaBlend();
    EnableDepthTest();
    if (m_extensions[0] && m_extensions[0]->IsVisible())
        m_extensions[0]->Render3D();
    if (m_extensions[1] && m_extensions[1]->IsVisible())
        m_extensions[1]->Render3D();
    if (m_extensions[2] && m_extensions[2]->IsVisible())
        m_extensions[2]->Render3D();
    if (m_extensions[3] && m_extensions[3]->IsVisible())
        m_extensions[3]->Render3D();
}
