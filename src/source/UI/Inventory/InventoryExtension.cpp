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
        if (false == m_extension->Create(STORAGE_TYPE::INVENTORY, g_pNewUI3DRenderMng, g_pNewItemMng, this, x + 15,
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
    UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });

    Show(false);

    return true;
}

void CInventoryExtension::BuildRmlUi()
{
    // Guarded so the document/model are created once, even if Create() re-runs on resolution change.
    if (!m_pRmlDoc && RmlUiRuntime::Instance().IsCreated())
    {
        const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "inventory_extension",
            [this](Rml::DataModelConstructor& c, InventoryExtensionRmlModel& model)
            {
                // See CCharMakeWin::BuildRmlUi()'s comment on why this must re-run in full every
                // call, including from ReloadRmlTheme() -- no guard here.
                auto lockedPage = c.RegisterStruct<LockedExtPageEntry>();
                lockedPage.RegisterMember("top", &LockedExtPageEntry::top);
                lockedPage.RegisterMember("number", &LockedExtPageEntry::number);
                lockedPage.RegisterMember("decorator", &LockedExtPageEntry::decorator);
                c.RegisterArray<std::vector<LockedExtPageEntry>>();

                c.Bind("root_x", &model.rootX);
                c.Bind("root_y", &model.rootY);
                c.Bind("root_scale", &model.rootScale);
                c.Bind("text_px", &model.textPx);
                c.Bind("title", &model.title);
                c.Bind("exit_tooltip", &model.exitTooltip);
                c.Bind("locked_pages", &model.lockedPages);

                c.BindEventCallback("inventory_extension_exit_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                    {
                        g_pNewUISystem->Hide(INTERFACE_INVENTORY_EXT);
                    });
            });

        if (modelCreated)
            m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), "Data/Interface/RmlUi/inventory_extension.rml");

        // Frame background panel uses the background context -- see InventoryExtensionBgRmlModel (InventoryExtension.h).
        if (Rml::Context* bgContext = RmlUiRuntime::Instance().GetBackgroundContext())
        {
            const bool bgModelCreated = m_BgRmlBinder.Create(bgContext, "inventory_extension_bg",
                [](Rml::DataModelConstructor& c, InventoryExtensionBgRmlModel& model)
                {
                    c.Bind("root_x", &model.rootX);
                    c.Bind("root_y", &model.rootY);
                    c.Bind("root_scale", &model.rootScale);
                });
            if (bgModelCreated)
            {
                // Starts hidden -- CreateBackgroundDocument() no longer Show()s eagerly (see its
                // own comment, RmlTheme.h); SyncRmlModel() below is what shows/hides it.
                m_pRmlBgDoc = UI::RmlBridge::CreateBackgroundDocument("Data/Interface/RmlUi/inventory_extension_bg.rml");
            }
        }

        // Not Show()n here -- m_pRmlDoc's visibility follows this window's own Show()/Hide() via
        // SyncRmlModel(), not an eager Show() at Create() time.
    }
}

void CInventoryExtension::ReloadRmlTheme()
{
    if (!m_pRmlDoc) return; // never opened -- BuildRmlUi() will simply pick up the new theme whenever it first is

    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    if (m_pRmlBgDoc)
    {
        if (Rml::Context* bgContext = RmlUiRuntime::Instance().GetBackgroundContext())
        {
            m_BgRmlBinder.Destroy(bgContext);
            bgContext->UnloadDocument(m_pRmlBgDoc);
        }
        m_pRmlBgDoc = nullptr;
    }

    BuildRmlUi();
    // Next frame's Update()/SyncRmlModel() self-corrects live state/visibility for both docs.
}

void CInventoryExtension::Release()
{
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
        UI::RmlBridge::UnregisterForThemeReload(this);
        m_pNewUIMng = nullptr;
    }
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
    // frame after Create()/Show(true)/ReloadRmlTheme(), before RmlUi's next layout pass.
    float panelWidth = WIDTH;
    float panelHeight = HEIGHT;
    UI::RmlBridge::RefreshLogicalPanelSize(m_pRmlDoc, "panel", panelWidth, panelHeight);
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
    // frame after Create()/Show(true)/ReloadRmlTheme(), before RmlUi's next layout pass.
    float panelWidth = WIDTH;
    float panelHeight = HEIGHT;
    UI::RmlBridge::RefreshLogicalPanelSize(m_pRmlDoc, "panel", panelWidth, panelHeight);
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
    if (g_pNewUISystem->IsVisible(INTERFACE_INVENTORY_EXT) == false)
    {
        return true;
    }

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
    EnableAlphaTest();

    // Frame background panel is RmlUi, routed through the background context (see
    // InventoryExtensionBgRmlModel), painted by CManager::Render()'s centralized
    // RenderBackgroundLayer() call before this window's own Render()/Render3D() run.
    RenderFrame();

    for (int i = 0; i < CharacterAttribute->InventoryExtensions; i++)
    {
        if (const auto& m_extension = m_extensions[i])
        {
            m_extension->Render();
        }
    }

    DisableAlphaBlend();
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
    if (m_pRmlBgDoc)
    {
        UI::RmlBridge::SyncRootTransform(m_BgRmlBinder, m_Pos);

        // RenderBackgroundLayer() renders whatever's shown in the shared background context
        // regardless of caller, so this Hide()/Show() is what keeps the bg panel hidden when closed.
        UI::RmlBridge::SyncDocumentVisibility(m_pRmlBgDoc, IsVisible());
    }

    if (!m_pRmlDoc) return;
    UI::RmlBridge::SyncDocumentVisibility(m_pRmlDoc, IsVisible());

    UI::RmlBridge::SyncRootTransform(m_RmlBinder, m_Pos);
    UI::RmlBridge::SyncNativeTextSize(m_RmlBinder);

    auto& model = m_RmlBinder.GetModel();
    auto syncWide = [&](Rml::String InventoryExtensionRmlModel::* field, const char* boundName, const wchar_t* text)
    {
        const Rml::String value = StringUtils::WideToNarrow(text);
        if (model.*field != value) { model.*field = value; m_RmlBinder.MarkDirty(boundName); }
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

        LockedExtPageEntry entry;
        entry.top = 71.f + static_cast<float>(i) * HEIGHT_PER_EXT;
        entry.number = i + 1;
        entry.decorator = Rml::String("image(ext-lock-") + static_cast<char>('0' + entry.number) + ")";
        model.lockedPages.push_back(entry);
    }
    m_RmlBinder.MarkDirty("locked_pages");
}

float CInventoryExtension::GetLayerDepth()
{
    return 4.55;
}

void CInventoryExtension::LoadImages()
{
    // Frame/exit-button art moved to RmlUi (inventory_extension.rcss/inventory_extension_bg.rcss).
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
