
#include "stdafx.h"
#include "UI/Inventory/MixInventory.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"
#include "UI/Dialogs/CustomMessageBox.h"
#include "UI/Dialogs/GenericConfirmDialog.h"
#include "UI/Core/WindowCommon.h" // ShowChaosMixMenuDialog
#include "GameLogic/Items/MixMgr.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Effects/ZzzEffect.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzInventory.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzCharacter.h"
#include "I18N/All.h"

#include "Audio/DSPlaySound.h"
#include "Network/Server/SocketSystem.h"

// RmlUi migration -- see this class's header comment.
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "Core/Utilities/StringUtils.h"
#include <RmlUi/Core/ComputedValues.h>
#include <algorithm>
#include <array>
#include "UI/RmlBridge/RmlSyncField.h"
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/ElementUtilities.h>

using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
// Native RenderText(x, y, text, boxWidth, ...) scales a line down to fit boxWidth
// (UI::Scaling::FontScaleForBounds). The factor for a line measured in `probe`'s font (a bold
// 1em element of this window); 1 = fits or no probe. `probeUnitsPerLayoutUnit` converts the
// probe's measurement to the window's own layout units, in which boxWidth is given (a theme may
// lay its text out in physical pixels inside a counter-scaled layer). Like native, a line never
// shrinks below the minimum font size (`minimumFit` of the current one); longer lines overflow.
float TextFitScale(Rml::Element* probe, const wchar_t* text, float boxWidth, float probeUnitsPerLayoutUnit,
                   float minimumFit)
{
    if (probe == nullptr || text == nullptr || text[0] == L'\0' || probeUnitsPerLayoutUnit <= 0.f)
        return 1.f;
    const float width =
        static_cast<float>(Rml::ElementUtilities::GetStringWidth(probe, StringUtils::WideToNarrow(text))) /
        probeUnitsPerLayoutUnit;
    return width <= boxWidth ? 1.f : std::max(boxWidth / width, minimumFit);
}
} // namespace

CMixInventory::CMixInventory()
{
    m_pNewUIMng = NULL;
    m_pNewInventoryCtrl = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_iMixState = MIX_READY;
    m_iMixEffectTimer = 0;
}
CMixInventory::~CMixInventory() { Release(); }

bool CMixInventory::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng || NULL == g_pNewItemMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_MIXINVENTORY, this);

    m_pNewInventoryCtrl = new CInventoryCtrl;
    if (false == m_pNewInventoryCtrl->Create(STORAGE_TYPE::CHAOS_MIX, g_pNewItemMng, this, x + 15, y + 110, 8, 4))
    {
        SAFE_DELETE(m_pNewInventoryCtrl);
        return false;
    }

    SetPos(x, y);


    m_pNewInventoryCtrl->GetSquareColorNormal(m_fInventoryColor);
    m_pNewInventoryCtrl->GetSquareColorWarning(m_fInventoryWarningColor);

    BuildRmlUi();

    Show(false);

    return true;
}

void CMixInventory::BindRmlModel(Rml::DataModelConstructor& c, MixInventoryRmlModel& model)
{
    c.Bind("root_x", &model.rootX);
    c.Bind("root_y", &model.rootY);
    c.Bind("root_scale", &model.rootScale);
    UI::Items::RegisterItemGridCells(c);
    c.Bind("grid_cells", &model.gridCells);
    c.Bind("text_px", &model.textPx);
    c.Bind("title", &model.title);
    c.Bind("mix_visible", &model.mixVisible);
    c.Bind("mix_locked", &model.mixLocked);
    c.Bind("mix_tooltip", &model.mixTooltip);

    c.Bind("show_tax_rate", &model.showTaxRate);
    c.Bind("tax_rate_text", &model.taxRateText);
    c.Bind("tax_rate_fit", &model.taxRateFit);

    c.Bind("show_recipe", &model.showRecipe);
    c.Bind("recipe_line1", &model.recipeLine1);
    c.Bind("recipe_line2", &model.recipeLine2);
    c.Bind("show_recipe_line2", &model.showRecipeLine2);
    c.Bind("recipe_ready", &model.recipeReady);

    c.Bind("show_success_rate", &model.showSuccessRate);
    c.Bind("success_rate_text", &model.successRateText);
    c.Bind("success_boosted", &model.successBoosted);

    c.Bind("show_required_zen", &model.showRequiredZen);
    c.Bind("required_zen_text", &model.requiredZenText);

    c.Bind("show_prediction", &model.showPrediction);
    c.Bind("prediction_text", &model.predictionText);

    auto mixLine = c.RegisterStruct<MixLine>();
    mixLine.RegisterMember("text", &MixLine::text);
    mixLine.RegisterMember("kind", &MixLine::kind);
    mixLine.RegisterMember("row", &MixLine::row);
    mixLine.RegisterMember("align_left", &MixLine::alignLeft);
    mixLine.RegisterMember("fit", &MixLine::fit);
    c.RegisterArray<std::vector<MixLine>>();
    c.Bind("source_lines", &model.sourceLines);
    c.Bind("status_lines", &model.statusLines);
    c.Bind("advice_lines", &model.adviceLines);
    c.Bind("description_lines", &model.descriptionLines);
    c.Bind("descriptions_lowered", &model.descriptionsLowered);

    c.Bind("show_socket_prompt", &model.showSocketPrompt);
    c.Bind("socket_prompt_text", &model.socketPromptText);

    c.Bind("show_socket_list", &model.showSocketList);
    auto socketLine = c.RegisterStruct<SocketListLine>();
    socketLine.RegisterMember("text", &SocketListLine::text);
    socketLine.RegisterMember("index", &SocketListLine::index);
    socketLine.RegisterMember("selected", &SocketListLine::selected);
    c.RegisterArray<std::vector<SocketListLine>>();
    c.Bind("socket_lines", &model.socketLines);

    c.BindEventCallback("mix_inventory_select_socket",
        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
        {
            if (arguments.size() == 1)
                SelectSocket(arguments[0].Get<int>(-1));
        });

    c.BindEventCallback("mix_inventory_mix_click",
        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
        {
            // Mirrors the old BtnProcess()'s MIX_FINISHED gate (native button was
            // simply not rendered/updated in that state; the RmlUi button is hidden
            // via mix_visible for the same reason, this is defense in depth).
            if (GetMixState() == MIX_FINISHED)
                return;
            Mix();
        });
}

void CMixInventory::OnRmlReloaded()
{
    m_SocketTextDirty = true;
}

void CMixInventory::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void CMixInventory::Release()
{
    m_ItemTarget.Disable();

    SAFE_DELETE(m_pNewInventoryCtrl);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
    m_RmlView.Release();
}

void CMixInventory::SetMixState(int iMixState)
{
    m_iMixState = iMixState;

    if (iMixState == MIX_REQUESTED)
    {
        m_iMixEffectTimer = 50;
        m_pNewInventoryCtrl->LockInventory();
        g_pMyInventory->GetInventoryCtrl()->LockInventory();
    }
    else
    {
        m_pNewInventoryCtrl->UnlockInventory();
        g_pMyInventory->GetInventoryCtrl()->UnlockInventory();
    }
    // Mix button's locked look (mix_locked) is refreshed from SyncRmlModel() each Update(), same
    // as it reads GetMixState() -- no need to push it from here too.
}

bool CMixInventory::InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket)
{
    if (m_pNewInventoryCtrl)
        return m_pNewInventoryCtrl->AddItem(iIndex, pbyItemPacket);
    return false;
}

void CMixInventory::DeleteItem(int iIndex)
{
    if (m_pNewInventoryCtrl)
    {
        ITEM* pItem = m_pNewInventoryCtrl->FindItem(iIndex);
        if (pItem != NULL)
            m_pNewInventoryCtrl->RemoveItem(pItem);
    }
}

void CMixInventory::DeleteAllItems()
{
    if (m_pNewInventoryCtrl)
        m_pNewInventoryCtrl->RemoveAllItems();
}

void CMixInventory::OpeningProcess()
{
    g_MixRecipeMgr.SetPlusChaosRate(0);
    SocketClient->ToGameServer()->SendCrywolfChaosRateBenefitRequest();

    SetMixState(mu::ui::window::CMixInventory::MIX_READY);

    if (g_MixRecipeMgr.GetMixInventoryType() == SEASON3A::MIXTYPE_GOBLIN_NORMAL)
    {
        mu::ui::window::ShowChaosMixMenuDialog();
    }
}

bool CMixInventory::ClosingProcess()
{
    if (g_pMixInventory->GetInventoryCtrl()->GetNumberOfItems() > 0 || CInventoryCtrl::GetPickedItem() != NULL)
    {
        g_pSystemLogBox->AddText(I18N::Game::CloseInventoryAfterMovingYourItemsInTheInventory, mu::ui::window::TYPE_ERROR_MESSAGE);
        return false;
    }

    switch (g_MixRecipeMgr.GetMixInventoryType())
    {
    case SEASON3A::MIXTYPE_GOBLIN_NORMAL:
    case SEASON3A::MIXTYPE_GOBLIN_CHAOSITEM:
    case SEASON3A::MIXTYPE_GOBLIN_ADD380:
    case SEASON3A::MIXTYPE_CASTLE_SENIOR:
    case SEASON3A::MIXTYPE_OSBOURNE:
    case SEASON3A::MIXTYPE_JERRIDON:
    case SEASON3A::MIXTYPE_ELPIS:
    case SEASON3A::MIXTYPE_CHAOS_CARD:
    case SEASON3A::MIXTYPE_CHERRYBLOSSOM:
    case SEASON3A::MIXTYPE_EXTRACT_SEED:
    case SEASON3A::MIXTYPE_SEED_SPHERE:
        SocketClient->ToGameServer()->SendCraftingDialogCloseRequest();
        break;
    case SEASON3A::MIXTYPE_TRAINER:
        SocketClient->ToGameServer()->SendCloseNpcRequest();
        break;
    case SEASON3A::MIXTYPE_ATTACH_SOCKET:
    case SEASON3A::MIXTYPE_DETACH_SOCKET:
        m_SocketSelection.ClearSelection();
        SocketClient->ToGameServer()->SendCraftingDialogCloseRequest();
        break;
    default:
        break;
    }
    g_pMixInventory->DeleteAllItems();
    g_MixRecipeMgr.ClearCheckRecipeResult();
    return true;
}

void CMixInventory::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;

    m_pNewInventoryCtrl->SetPos(x + 15, y + 110);
}

bool CMixInventory::UpdateMouseEvent()
{
    if (m_pNewInventoryCtrl && false == m_pNewInventoryCtrl->UpdateMouseEvent())
        return false;

    if (true == InventoryProcess())
        return false;

    if (true == BtnProcess())
        return false;

    // #panel's own live RCSS size is the source of truth -- INVENTORY_WIDTH/HEIGHT only cover the
    // first frame after Create()/Show(true)/a theme switch, before RmlUi's next layout pass.
    float panelWidth = INVENTORY_WIDTH;
    float panelHeight = INVENTORY_HEIGHT;
    UI::RmlBridge::RefreshLogicalPanelSize(m_RmlView.Document(), "panel", panelWidth, panelHeight);
    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, static_cast<int>(panelWidth), static_cast<int>(panelHeight)).Contains(MouseX, MouseY))
    {
        if (mu::ui::window::IsPress(VK_RBUTTON))
        {
            // Right-click sends a craft-box item back to the inventory.
            ProcessMixItemAutoMoveToInventory();
            MouseRButton = false;
            MouseRButtonPop = false;
            MouseRButtonPush = false;
            return false;
        }

        if (mu::ui::window::IsNone(VK_LBUTTON) == false)
        {
            return false;
        }
    }

    return true;
}
bool CMixInventory::UpdateKeyEvent()
{
    return true;
}
bool CMixInventory::Update()
{
    if (m_pNewInventoryCtrl && false == m_pNewInventoryCtrl->Update())
        return false;

    if (IsVisible())
    {
        CheckMixInventory();
        RefreshSocketOptions();
    }

    SyncRmlModel();
    return true;
}
bool CMixInventory::Render()
{
    if (m_pNewInventoryCtrl)
        m_pNewInventoryCtrl->Render();
    return true;
}

void CMixInventory::SyncRmlModel()
{
    m_ItemTarget.Sync(m_RmlView.Document() ? m_RmlView.Document()->GetElementById("item_view") : nullptr, IsVisible());
    if (!m_RmlView.Document()) return;
    UI::RmlBridge::SyncDocumentVisibility(m_RmlView.Document(), IsVisible());

    UI::RmlBridge::SyncRootTransform(m_RmlView.Binder(), m_Pos);
    if (m_pNewInventoryCtrl)
        m_pNewInventoryCtrl->FollowGrid(m_RmlView.Document(), "item_grid", m_Pos, 15, 110);
    if (m_pNewInventoryCtrl && m_RmlView.GetModel().gridCells != m_pNewInventoryCtrl->Cells())
    {
        m_RmlView.GetModel().gridCells = m_pNewInventoryCtrl->Cells();
        m_RmlView.MarkDirty("grid_cells");
    }
    UI::RmlBridge::SyncNativeTextSize(m_RmlView.Binder());

    auto& model = m_RmlView.GetModel();
    auto syncWide = [&](Rml::String MixInventoryRmlModel::* field, const char* boundName, const wchar_t* text)
    {
        const Rml::String value = StringUtils::WideToNarrow(text);
        if (model.*field != value) { model.*field = value; m_RmlView.MarkDirty(boundName); }
    };
    auto syncBool = [&](bool MixInventoryRmlModel::* field, const char* boundName, bool value)
    {
        if (model.*field != value) { model.*field = value; m_RmlView.MarkDirty(boundName); }
    };

    // Former RenderFrame() title switch -- same cases, same strings, now driving this RmlUi title
    // span instead of a native RenderText() call.
    const wchar_t* titleText = I18N::Game::Chaos;
    switch (g_MixRecipeMgr.GetMixInventoryType())
    {
    case SEASON3A::MIXTYPE_GOBLIN_NORMAL:    titleText = I18N::Game::RegularCombination; break;
    case SEASON3A::MIXTYPE_GOBLIN_CHAOSITEM: titleText = I18N::Game::ChaosWeaponCombination; break;
    case SEASON3A::MIXTYPE_GOBLIN_ADD380:    titleText = I18N::Game::ItemOptionCombination; break;
    case SEASON3A::MIXTYPE_CASTLE_SENIOR:    titleText = I18N::Game::Store1640; break;
    case SEASON3A::MIXTYPE_TRAINER:          titleText = I18N::Game::ResurrectSpirit; break;
    case SEASON3A::MIXTYPE_OSBOURNE:         titleText = I18N::Game::Refine; break;
    case SEASON3A::MIXTYPE_JERRIDON:         titleText = I18N::Game::Restore; break;
    case SEASON3A::MIXTYPE_ELPIS:            titleText = I18N::Game::Refine; break;
    case SEASON3A::MIXTYPE_CHAOS_CARD:       titleText = I18N::Game::ChaosCardCombination; break;
    case SEASON3A::MIXTYPE_CHERRYBLOSSOM:    titleText = I18N::Game::SpiritOfCherryBlossoms; break;
    case SEASON3A::MIXTYPE_EXTRACT_SEED:     titleText = I18N::Game::Extraction; break;
    case SEASON3A::MIXTYPE_SEED_SPHERE:      titleText = I18N::Game::Assembly; break;
    case SEASON3A::MIXTYPE_ATTACH_SOCKET:    titleText = I18N::Game::Application; break;
    case SEASON3A::MIXTYPE_DETACH_SOCKET:    titleText = I18N::Game::Destruction; break;
    default:                                 titleText = I18N::Game::Chaos; break;
    }
    syncWide(&MixInventoryRmlModel::title, "title", titleText);

    // Mirrors RenderFrame()'s former end-of-function tooltip switch (m_BtnMix.ChangeToolTipText()
    // per mix type) -- same cases, same strings, now driving the RmlUi Mix button's tooltip span.
    const wchar_t* tooltipText = I18N::Game::Combining;
    switch (g_MixRecipeMgr.GetMixInventoryType())
    {
    case SEASON3A::MIXTYPE_TRAINER:       tooltipText = I18N::Game::Resurrection; break;
    case SEASON3A::MIXTYPE_OSBOURNE:      tooltipText = I18N::Game::Refine; break;
    case SEASON3A::MIXTYPE_JERRIDON:      tooltipText = I18N::Game::Restore; break;
    case SEASON3A::MIXTYPE_ELPIS:         tooltipText = I18N::Game::Refine; break;
    case SEASON3A::MIXTYPE_EXTRACT_SEED:  tooltipText = I18N::Game::Extraction; break;
    case SEASON3A::MIXTYPE_SEED_SPHERE:   tooltipText = I18N::Game::Assembly; break;
    case SEASON3A::MIXTYPE_ATTACH_SOCKET: tooltipText = I18N::Game::Application; break;
    case SEASON3A::MIXTYPE_DETACH_SOCKET: tooltipText = I18N::Game::Destruction; break;
    default:                              tooltipText = I18N::Game::Combining; break;
    }
    syncWide(&MixInventoryRmlModel::mixTooltip, "mix_tooltip", tooltipText);

    // Mirrors RenderFrame()'s former early-return-at-MIX_FINISHED (which stopped the button from
    // rendering at all) and SetMixState()'s former m_BtnMix.Lock()/UnLock() calls.
    syncBool(&MixInventoryRmlModel::mixVisible, "mix_visible", GetMixState() != MIX_FINISHED);
    syncBool(&MixInventoryRmlModel::mixLocked, "mix_locked", GetMixState() == MIX_REQUESTED);

    SyncMixContentModel();
    SyncSocketListModel();
}

bool CMixInventory::RefreshSocketOptions()
{
    const int mixType = g_MixRecipeMgr.GetMixInventoryType();
    const bool shown = mixType == SEASON3A::MIXTYPE_ATTACH_SOCKET || mixType == SEASON3A::MIXTYPE_DETACH_SOCKET;
    const int count = shown ? std::clamp(g_MixRecipeMgr.GetFirstItemSocketCount(), 0, MAX_SOCKETS) : 0;
    std::array<UI::Inventory::SocketListSelection::Option, MAX_SOCKETS> options{};
    for (int i = 0; i < count; ++i)
        options[i] = {g_MixRecipeMgr.GetFirstItemSocketSeedID(i), g_MixRecipeMgr.GetFirstItemSocketShpereLv(i)};
    const bool changed = m_SocketSelection.Update(mixType, {options.data(), static_cast<size_t>(count)});
    m_SocketTextDirty |= changed;
    return changed;
}

void CMixInventory::SelectSocket(int index)
{
    if (!IsVisible() || GetMixState() != MIX_READY)
        return;
    const bool itemsChanged = CheckMixInventory();
    const bool optionsChanged = RefreshSocketOptions();
    if (itemsChanged || optionsChanged)
        return;
    if (m_SocketSelection.Select(index))
        SyncSocketListModel();
}

float CMixInventory::GetLayerDepth()
{
    return 3.4f;
}

CInventoryCtrl* CMixInventory::GetInventoryCtrl() const
{
    return m_pNewInventoryCtrl;
}

void CMixInventory::SyncSocketListModel()
{
    auto& model = m_RmlView.GetModel();
    const int mixType = g_MixRecipeMgr.GetMixInventoryType();
    const bool shown = mixType == SEASON3A::MIXTYPE_ATTACH_SOCKET || mixType == SEASON3A::MIXTYPE_DETACH_SOCKET;
    SyncField(m_RmlView.Binder(), &MixInventoryRmlModel::showSocketList, "show_socket_list", shown);
    if (m_SocketTextDirty)
    {
        model.socketLines.clear();
        const auto options = m_SocketSelection.Options();
        for (size_t i = 0; i < options.size(); ++i)
        {
            wchar_t description[64] = {};
            if (options[i].seed == SOCKET_EMPTY)
                mu_swprintf(description, I18N::Game::NoItemApplication);
            else
                g_SocketItemMgr.CreateSocketOptionText(description, options[i].seed, options[i].sphereLevel);
            wchar_t text[128] = {};
            mu_swprintf(text, L"%d: %ls", static_cast<int>(i) + 1, description);
            model.socketLines.push_back({StringUtils::WideToNarrow(text), static_cast<int>(i), false});
        }
        m_SocketTextDirty = false;
        m_RmlView.MarkDirty("socket_lines");
    }
    for (auto& row : model.socketLines)
    {
        const bool selected = row.index == m_SocketSelection.Selected();
        if (row.selected == selected)
            continue;
        row.selected = selected;
        m_RmlView.MarkDirty("socket_lines");
    }
}

bool CMixInventory::BtnProcess()
{
    // Top-right corner close "X" (shared frame): hides + swallows the click.
    g_pNewUISystem->HandleFrameCornerClose(m_Pos, mu::ui::window::INTERFACE_MIXINVENTORY);

    return false;
}

void CMixInventory::SyncMixContentModel()
{
    if (!m_RmlView.Document())
        return;

    // The bold title measures lines in the window's own text font (the theme sizes it 1em). A
    // counter-scaled title (legacy .sharp-text) measures in physical pixels.
    Rml::Element* fitProbe = m_RmlView.Document()->GetElementById("title");
    float probeUnitsPerLayoutUnit = 1.f;
    if (fitProbe != nullptr && fitProbe->GetComputedValues().has_local_transform())
        probeUnitsPerLayoutUnit = GetLayoutTransform().scaleX;
    const auto layout = GetLayoutTransform();
    const float minimumFit = static_cast<float>(UI::Scaling::MinimumFontPointSize(UI::Scaling::FontRole::Normal)) /
                             static_cast<float>(UI::Scaling::FontPointSize(UI::Scaling::FontRole::Normal, layout));
    auto& model = m_RmlView.GetModel();
    auto syncWide = [&](Rml::String MixInventoryRmlModel::* field, const char* boundName, const wchar_t* text)
    {
        const Rml::String value = StringUtils::WideToNarrow(text);
        if (model.*field != value) { model.*field = value; m_RmlView.MarkDirty(boundName); }
    };
    auto syncBool = [&](bool MixInventoryRmlModel::* field, const char* boundName, bool value)
    {
        if (model.*field != value) { model.*field = value; m_RmlView.MarkDirty(boundName); }
    };
    auto makeColor = [](int r, int g, int b, int a) -> Rml::String
    {
        wchar_t buf[32];
        mu_swprintf(buf, L"rgba(%d,%d,%d,%d)", r, g, b, a);
        return StringUtils::WideToNarrow(buf);
    };
    auto syncColor = [&](Rml::String MixInventoryRmlModel::* field, const char* boundName, int r, int g, int b, int a)
    {
        const Rml::String value = makeColor(r, g, b, a);
        if (model.*field != value) { model.*field = value; m_RmlView.MarkDirty(boundName); }
    };
    auto syncLines = [&](std::vector<MixLine> MixInventoryRmlModel::* field, const char* boundName,
        std::vector<MixLine> newLines)
    {
        if (model.*field != newLines) { model.*field = std::move(newLines); m_RmlView.MarkDirty(boundName); }
    };

    wchar_t szText[256] = {};
    const int mixType = g_MixRecipeMgr.GetMixInventoryType();

    // Tax rate -- former RenderFrame() first switch's TaxRateDChangedInRealTime line. NOT gated by
    // MIX_FINISHED below (this rendered before RenderFrame()'s own early-return check).
    bool showTax = false;
    switch (mixType)
    {
    case SEASON3A::MIXTYPE_GOBLIN_NORMAL:
    case SEASON3A::MIXTYPE_GOBLIN_CHAOSITEM:
    case SEASON3A::MIXTYPE_GOBLIN_ADD380:
    case SEASON3A::MIXTYPE_TRAINER:
        showTax = true;
        mu_swprintf(szText, I18N::Game::TaxRateDChangedInRealTime, g_nChaosTaxRate);
        break;
    default:
        break;
    }
    syncBool(&MixInventoryRmlModel::showTaxRate, "show_tax_rate", showTax);
    if (showTax)
    {
        constexpr float kTaxRateBoxWidth = 160.f; // native RenderText(..., 160.0f, ...)
        syncWide(&MixInventoryRmlModel::taxRateText, "tax_rate_text", szText);
        const float fit = TextFitScale(fitProbe, szText, kTaxRateBoxWidth, probeUnitsPerLayoutUnit, minimumFit);
        if (model.taxRateFit != fit)
        {
            model.taxRateFit = fit;
            m_RmlView.MarkDirty("tax_rate_fit");
        }
    }

    // Recipe result name onward -- hidden entirely once MIX_FINISHED, mirroring RenderFrame()'s own
    // early return (nothing past that point ever rendered either).
    const bool showRecipe = (GetMixState() != MIX_FINISHED);
    syncBool(&MixInventoryRmlModel::showRecipe, "show_recipe", showRecipe);
    if (!showRecipe)
        return;

    syncBool(&MixInventoryRmlModel::recipeReady, "recipe_ready", g_MixRecipeMgr.IsReadyToMix() == TRUE);

    g_MixRecipeMgr.GetCurRecipeName(szText, 1);
    syncWide(&MixInventoryRmlModel::recipeLine1, "recipe_line1", szText);
    szText[0] = L'\0';
    const bool showRecipeLine2 = (g_MixRecipeMgr.GetCurRecipeName(szText, 2) == TRUE);
    syncBool(&MixInventoryRmlModel::showRecipeLine2, "show_recipe_line2", showRecipeLine2);
    if (showRecipeLine2)
        syncWide(&MixInventoryRmlModel::recipeLine2, "recipe_line2", szText);

    // Success rate -- type-gated exactly like RenderFrame()'s own second switch.
    bool showSuccess = false;
    switch (mixType)
    {
    case SEASON3A::MIXTYPE_GOBLIN_NORMAL:
    case SEASON3A::MIXTYPE_GOBLIN_CHAOSITEM:
    case SEASON3A::MIXTYPE_GOBLIN_ADD380:
    case SEASON3A::MIXTYPE_OSBOURNE:
    case SEASON3A::MIXTYPE_ELPIS:
    case SEASON3A::MIXTYPE_TRAINER:
    case SEASON3A::MIXTYPE_EXTRACT_SEED:
    case SEASON3A::MIXTYPE_SEED_SPHERE:
        showSuccess = true;
        if (g_MixRecipeMgr.IsReadyToMix() &&
            g_MixRecipeMgr.GetPlusChaosRate() > 0 && g_MixRecipeMgr.GetCurRecipe()->m_bMixOption == 'F')
        {
            mu_swprintf(szText, I18N::Game::SSuccessRateD, I18N::Game::Combining, g_MixRecipeMgr.GetSuccessRate());
            mu_swprintf(szText, L"%ls + %d%%", szText, g_MixRecipeMgr.GetPlusChaosRate());
            syncBool(&MixInventoryRmlModel::successBoosted, "success_boosted", true);
        }
        else
        {
            switch (mixType)
            {
            case SEASON3A::MIXTYPE_GOBLIN_NORMAL:
            case SEASON3A::MIXTYPE_GOBLIN_CHAOSITEM:
            case SEASON3A::MIXTYPE_GOBLIN_ADD380:
            case SEASON3A::MIXTYPE_EXTRACT_SEED:
            case SEASON3A::MIXTYPE_SEED_SPHERE:
                mu_swprintf(szText, I18N::Game::SSuccessRateD, I18N::Game::Combining, g_MixRecipeMgr.GetSuccessRate());
                break;
            case SEASON3A::MIXTYPE_TRAINER:
                mu_swprintf(szText, I18N::Game::SSuccessRateD, I18N::Game::Resurrection, g_MixRecipeMgr.GetSuccessRate());
                break;
            case SEASON3A::MIXTYPE_OSBOURNE:
            case SEASON3A::MIXTYPE_ELPIS:
                mu_swprintf(szText, I18N::Game::SSuccessRateD, I18N::Game::Refine, g_MixRecipeMgr.GetSuccessRate());
                break;
            }
            syncBool(&MixInventoryRmlModel::successBoosted, "success_boosted", false);
        }
        break;
    default:
        break;
    }
    syncBool(&MixInventoryRmlModel::showSuccessRate, "show_success_rate", showSuccess);
    if (showSuccess)
        syncWide(&MixInventoryRmlModel::successRateText, "success_rate_text", szText);

    // Required zen -- type-gated exactly like RenderFrame()'s own third switch. Always rendered in
    // the same (210,230,255) tone natively by the time this ran (every branch above it resets to
    // that color before falling through), so no separate color field is needed.
    bool showZen = false;
    switch (mixType)
    {
    case SEASON3A::MIXTYPE_GOBLIN_NORMAL:
    case SEASON3A::MIXTYPE_GOBLIN_CHAOSITEM:
    case SEASON3A::MIXTYPE_GOBLIN_ADD380:
    case SEASON3A::MIXTYPE_CASTLE_SENIOR:
    case SEASON3A::MIXTYPE_TRAINER:
    case SEASON3A::MIXTYPE_JERRIDON:
    case SEASON3A::MIXTYPE_EXTRACT_SEED:
    case SEASON3A::MIXTYPE_SEED_SPHERE:
    case SEASON3A::MIXTYPE_ATTACH_SOCKET:
    case SEASON3A::MIXTYPE_DETACH_SOCKET:
    {
        showZen = true;
        wchar_t szGoldText[32];
        wchar_t szGoldText2[32];
        ConvertGold(g_MixRecipeMgr.GetReqiredZen(), szGoldText);
        ConvertChaosTaxGold(g_MixRecipeMgr.GetReqiredZen(), szGoldText2);
        if (g_MixRecipeMgr.IsReadyToMix() && g_MixRecipeMgr.GetCurRecipe()->m_bRequiredZenType == 'C')
            mu_swprintf(szText, I18N::Game::RequiredZenForPotionSS, szGoldText2, szGoldText);
        else
            mu_swprintf(szText, I18N::Game::RequiredZenSS, szGoldText2, szGoldText);
        break;
    }
    default:
        break;
    }
    syncBool(&MixInventoryRmlModel::showRequiredZen, "show_required_zen", showZen);
    if (showZen)
        syncWide(&MixInventoryRmlModel::requiredZenText, "required_zen_text", szText);

    // Prediction / source checklist / status message -- former fLine_y=203 block. Long strings are
    // bound whole, for RmlUi's own text layout to wrap (mix_inventory.rcss), rather than
    // pre-splitting them with the native CutStr()/pixel-width measurement RenderFrame() used to --
    // that measurement is native-GDI-calibrated, a different system than RmlUi's own font
    // rendering (see NPCDialogue.cpp's own comment on this same class of problem).
    std::vector<MixLine> sourceLines;
    std::vector<MixLine> statusLines;
    bool showPrediction = false;
    wchar_t predictionText[256] = {};

    if (g_MixRecipeMgr.GetMostSimilarRecipe() != NULL)
    {
        wchar_t szName[100] = {};
        if (!g_MixRecipeMgr.IsReadyToMix() && g_MixRecipeMgr.GetMostSimilarRecipeName(szName, 1) == TRUE)
        {
            showPrediction = true;
            mu_swprintf(predictionText, I18N::Game::AssemblyPredictionS, szName);
        }

        for (int iLine = 0; iLine < 8; ++iLine)
        {
            const int iResult = g_MixRecipeMgr.GetSourceName(iLine, szText);
            if (iResult == SEASON3A::MIX_SOURCE_ERROR) break;

            Rml::String kind;
            if (iResult == SEASON3A::MIX_SOURCE_NO) kind = "missing";
            else if (iResult == SEASON3A::MIX_SOURCE_PARTIALLY) kind = "partial";
            else if (iResult == SEASON3A::MIX_SOURCE_YES) kind = "ready";

            sourceLines.push_back({ StringUtils::WideToNarrow(szText), kind });
        }
    }
    else if (g_MixRecipeMgr.IsMixInit())
    {
        statusLines.push_back({ StringUtils::WideToNarrow(I18N::Game::PleaseUploadTheAssemblyItems), "missing" });
    }
    else
    {
        mu_swprintf(szText, I18N::Game::AssemblyPredictionS, L" ");
        statusLines.push_back({ StringUtils::WideToNarrow(szText), "missing" });
        statusLines.push_back({ StringUtils::WideToNarrow(I18N::Game::ImproperItemsForCombination), "missing" });
    }

    syncBool(&MixInventoryRmlModel::showPrediction, "show_prediction", showPrediction);
    if (showPrediction)
        syncWide(&MixInventoryRmlModel::predictionText, "prediction_text", predictionText);
    syncLines(&MixInventoryRmlModel::sourceLines, "source_lines", sourceLines);
    syncLines(&MixInventoryRmlModel::statusLines, "status_lines", statusLines);

    // Recipe description (ready to mix) or advice for the closest match (not ready) -- former
    // trailing block, always rendered in the same reddish tone natively.
    std::vector<MixLine> adviceLines;
    const Rml::String adviceColor = "missing";
    if (g_MixRecipeMgr.IsReadyToMix())
    {
        if (g_MixRecipeMgr.GetCurRecipeDesc(szText, 1) == TRUE) adviceLines.push_back({ StringUtils::WideToNarrow(szText), adviceColor });
        if (g_MixRecipeMgr.GetCurRecipeDesc(szText, 2) == TRUE) adviceLines.push_back({ StringUtils::WideToNarrow(szText), adviceColor });
        if (g_MixRecipeMgr.GetCurRecipeDesc(szText, 3) == TRUE) adviceLines.push_back({ StringUtils::WideToNarrow(szText), adviceColor });
    }
    else if (g_MixRecipeMgr.GetMostSimilarRecipe() != NULL)
    {
        if (g_MixRecipeMgr.GetRecipeAdvice(szText, 1) == TRUE) adviceLines.push_back({ StringUtils::WideToNarrow(szText), adviceColor });
        if (g_MixRecipeMgr.GetRecipeAdvice(szText, 2) == TRUE) adviceLines.push_back({ StringUtils::WideToNarrow(szText), adviceColor });
        if (g_MixRecipeMgr.GetRecipeAdvice(szText, 3) == TRUE) adviceLines.push_back({ StringUtils::WideToNarrow(szText), adviceColor });
    }
    syncLines(&MixInventoryRmlModel::adviceLines, "advice_lines", adviceLines);

    // Former RenderMixDescriptions() -- static per-mix-type instructional text, with the native
    // position of every line (a theme may place them there or simply flow them).
    std::vector<MixLine> descriptionLines;
    bool showSocketPrompt = false;
    wchar_t socketPromptText[128] = {};
    const Rml::String white = "normal";
    const Rml::String warning = "warning";
    // Native boxes: 160 centred at x+15, or 200 left-aligned from x+5 -- which runs 15 past the
    // 190-wide window; the left box is kept inside it (180) so no line overflows the frame.
    constexpr float kCentredDescriptionWidth = 160.f;
    constexpr float kLeftDescriptionWidth = 180.f;
    // RenderMixDescriptions() walked its own block in 13-unit rows, skipping some; the row it
    // chose is what travels, and the theme turns it into a top.
    bool descriptionsLowered = false;
    auto describeAt = [&](bool lowered, const wchar_t* text, const Rml::String& kind, int row, bool alignLeft)
    {
        descriptionsLowered = lowered;
        const float fit = TextFitScale(fitProbe, text, alignLeft ? kLeftDescriptionWidth : kCentredDescriptionWidth,
                                       probeUnitsPerLayoutUnit, minimumFit);
        descriptionLines.push_back({StringUtils::WideToNarrow(text), kind, row, alignLeft, fit});
    };
    auto describe = [&](const wchar_t* text, const Rml::String& kind, int row, bool alignLeft = false)
    { describeAt(false, text, kind, row, alignLeft); };
    switch (mixType)
    {
    case SEASON3A::MIXTYPE_CASTLE_SENIOR:
        for (int i = 0; i < 6; ++i)
            describeAt(true, I18N::Game::Lookup(1644 + i), "dim", i,
                       false);
        break;
    case SEASON3A::MIXTYPE_OSBOURNE:
        describe(I18N::Game::RefineTheItemToCreate, white, 0);
        describe(I18N::Game::TheRefiningStone, white, 1);
        mu_swprintf(szText, I18N::Game::SForOnlyS, I18N::Game::Refine, I18N::Game::WeaponsOrShields);
        describe(szText, white, 2);
        describe(I18N::Game::Allowed, white, 3);
        describe(I18N::Game::ItemWillDisappearWhenFailed, "loss", 4);
        break;
    case SEASON3A::MIXTYPE_JERRIDON:
        describe(I18N::Game::RestorationIsDeletingThe, white, 0);
        describe(I18N::Game::ReinforcementOption, white, 1);
        describe(I18N::Game::OfTheWeapons, white, 2);
        describe(I18N::Game::ForRestoringReinforcedItem, white, 3);
        describe(I18N::Game::ReinforcementOptionHasToBe, white, 4);
        describe(I18N::Game::DeletedThroughRestoration, white, 5);
        break;
    case SEASON3A::MIXTYPE_ELPIS:
        describe(I18N::Game::GettingThroughRefiningProcess, white, 0);
        describe(I18N::Game::OfJewelOfHarmonyOrignal, white, 1);
        describe(I18N::Game::GemstoneWillGiveMorePower, white, 2);
        break;
    case SEASON3A::MIXTYPE_CHAOS_CARD:
        describe(I18N::Game::Warning2223, warning, 4);
        describe(I18N::Game::CombinationsCanBeUsedOnceAtATime, white, 6, true);
        describe(I18N::Game::MoreThan2X4SpaceInInventoryIsNeeded, white, 7, true);
        describe(I18N::Game::YouCanAchieveSpecialItemsWithCombinations, white, 8, true);
        break;
    case SEASON3A::MIXTYPE_CHERRYBLOSSOM:
        describe(I18N::Game::Warning2223, warning, 0);
        describe(I18N::Game::_255GoldenCherryBlossomBranches, white, 2, true);
        describe(I18N::Game::OnlyTheSameTypeOfCherryBlossomsBranchesCanBeUploaded, white, 3, true);
        describe(I18N::Game::MoreThan2X4SpaceInInventoryIsNeeded, white, 4, true);
        break;
    case SEASON3A::MIXTYPE_ATTACH_SOCKET:
        showSocketPrompt = true;
        mu_swprintf(socketPromptText, L"%ls", I18N::Game::SelectApplicableSocket);
        break;
    case SEASON3A::MIXTYPE_DETACH_SOCKET:
        showSocketPrompt = true;
        mu_swprintf(socketPromptText, L"%ls", I18N::Game::SelectDestructibleSocket);
        break;
    default:
        break;
    }
    syncLines(&MixInventoryRmlModel::descriptionLines, "description_lines", descriptionLines);
    syncBool(&MixInventoryRmlModel::descriptionsLowered, "descriptions_lowered", descriptionsLowered);
    syncBool(&MixInventoryRmlModel::showSocketPrompt, "show_socket_prompt", showSocketPrompt);
    if (showSocketPrompt)
        syncWide(&MixInventoryRmlModel::socketPromptText, "socket_prompt_text", socketPromptText);
}

int CMixInventory::Rtn_MixRequireZen(int _nMixZen, int _nTax)
{
    if (_nTax)		_nMixZen += ((LONGLONG)_nMixZen * g_nChaosTaxRate) / 100;
    return _nMixZen;
}

bool CMixInventory::PrepareSocketMix()
{
    if (g_MixRecipeMgr.GetMixInventoryType() == SEASON3A::MIXTYPE_ATTACH_SOCKET)
    {
        const int iSelectedLine = m_SocketSelection.Selected();

        for (int i = 0; i < g_MixRecipeMgr.GetFirstItemSocketCount(); ++i)
        {
            BYTE bySocketSeedID = g_MixRecipeMgr.GetFirstItemSocketSeedID(i);
            if (bySocketSeedID != SOCKET_EMPTY)
            {
                BYTE bySeedSphereID = g_MixRecipeMgr.GetSeedSphereID(0);
                if (bySocketSeedID == bySeedSphereID)
                {
                    g_pSystemLogBox->AddText(I18N::Game::YouCannotApplyTheSameTypeOfSphere, mu::ui::window::TYPE_ERROR_MESSAGE);
                    return false;
                }
            }
        }

        if (iSelectedLine < 0)
        {
            g_pSystemLogBox->AddText(I18N::Game::YouMustSelectTheSocket, mu::ui::window::TYPE_ERROR_MESSAGE);
            return false;
        }
        else if (iSelectedLine >= g_MixRecipeMgr.GetFirstItemSocketCount()
            || g_MixRecipeMgr.GetFirstItemSocketSeedID(iSelectedLine) != SOCKET_EMPTY)
        {
            g_pSystemLogBox->AddText(I18N::Game::ItSAlreadyAppliedOnTheCharacter, mu::ui::window::TYPE_ERROR_MESSAGE);
            return false;
        }

        g_MixRecipeMgr.SetMixSubType(iSelectedLine);
    }
    else if (g_MixRecipeMgr.GetMixInventoryType() == SEASON3A::MIXTYPE_DETACH_SOCKET)
    {
        const int iSelectedLine = m_SocketSelection.Selected();
        if (iSelectedLine < 0)
        {
            g_pSystemLogBox->AddText(I18N::Game::YouMustSelectTheDestructibleSocket, mu::ui::window::TYPE_ERROR_MESSAGE);
            return false;
        }
        else if (iSelectedLine >= g_MixRecipeMgr.GetFirstItemSocketCount()
            || g_MixRecipeMgr.GetFirstItemSocketSeedID(iSelectedLine) == SOCKET_EMPTY)
        {
            g_pSystemLogBox->AddText(I18N::Game::ThereAreNoDestructibleSeedSpheres, mu::ui::window::TYPE_ERROR_MESSAGE);
            return false;
        }
        g_MixRecipeMgr.SetMixSubType(iSelectedLine);
    }

    return true;
}

void CMixInventory::ConfirmMix(int mixType, int mixId, int socketIndex, const std::vector<DWORD>& itemKeys)
{
    if (!IsVisible() || GetMixState() != MIX_READY)
        return;
    CheckMixInventory();
    RefreshSocketOptions();
    if (g_MixRecipeMgr.GetMixInventoryType() != mixType || g_MixRecipeMgr.GetCurMixID() != mixId)
        return;
    if (mixType == SEASON3A::MIXTYPE_ATTACH_SOCKET || mixType == SEASON3A::MIXTYPE_DETACH_SOCKET)
    {
        if (itemKeys != m_SocketItemKeys || socketIndex != m_SocketSelection.Selected() || !PrepareSocketMix())
            return;
    }
    SetMixState(MIX_REQUESTED);
    SocketClient->ToGameServer()->SendChaosMachineMixRequest(
        static_cast<ChaosMachineMixType>(mixId), g_MixRecipeMgr.GetMixSubType());
}

bool CMixInventory::Mix()
{
    CheckMixInventory();
    RefreshSocketOptions();

    PlayBuffer(SOUND_CLICK01);

    DWORD dwGold = CharacterMachine->Gold;
    int	  nMixZen = g_MixRecipeMgr.GetReqiredZen();

    nMixZen = Rtn_MixRequireZen(nMixZen, g_nChaosTaxRate);

    if (nMixZen > (int)dwGold)
    {
        g_pSystemLogBox->AddText(I18N::Game::NotEnoughZenToCombineItems, mu::ui::window::TYPE_ERROR_MESSAGE);
        return false;
    }

    if (!g_MixRecipeMgr.IsReadyToMix())
    {
        wchar_t szText[100];
        mu_swprintf(szText, I18N::Game::YouAreLackOfSItems, I18N::Game::Combining);
        g_pSystemLogBox->AddText(szText, mu::ui::window::TYPE_ERROR_MESSAGE);
        return false;
    }

    int iLevel = CharacterAttribute->Level;
    if (iLevel < g_MixRecipeMgr.GetCurRecipe()->m_iRequiredLevel)
    {
        wchar_t szText[100];
        wchar_t szText2[100];
        g_MixRecipeMgr.GetCurRecipeName(szText2, 1);
        mu_swprintf(szText, I18N::Game::FromAboveTheLevelDSEnabledAndOn, g_MixRecipeMgr.GetCurRecipe()->m_iRequiredLevel, szText2);
        g_pSystemLogBox->AddText(szText, mu::ui::window::TYPE_ERROR_MESSAGE);
        return false;
    }

    if (g_MixRecipeMgr.GetCurRecipe()->m_iWidth != -1 &&
        g_pMyInventory->FindEmptySlot(g_MixRecipeMgr.GetCurRecipe()->m_iWidth, g_MixRecipeMgr.GetCurRecipe()->m_iHeight) == -1)
    {
        g_pSystemLogBox->AddText(I18N::Game::CombineItemsAfterOrganizingYourInventory, mu::ui::window::TYPE_ERROR_MESSAGE);
        return false;
    }

    if (!PrepareSocketMix())
        return false;

#ifdef LJH_MOD_CANNOT_USE_CHARMITEM_AND_CHAOSCHARMITEM_SIMULTANEOUSLY
    if (g_MixRecipeMgr.GetTotalChaosCharmCount() > 0 && g_MixRecipeMgr.GetTotalCharmCount() > 0)
    {
        g_pSystemLogBox->AddText(I18N::Game::YouCannotUseTheTalismanOf, mu::ui::window::TYPE_ERROR_MESSAGE);
        return FALSE;
    }
#endif //LJH_MOD_CANNOT_USE_CHARMITEM_AND_CHAOSCHARMITEM_SIMULTANEOUSLY

    if (CInventoryCtrl::GetPickedItem() == NULL)
    {
        wchar_t strText[256];
        if (g_MixRecipeMgr.GetCurRecipe()->m_iMixName[1] == 0)
        {
            mu_swprintf(strText, L"%ls", I18N::Game::Lookup(g_MixRecipeMgr.GetCurRecipe()->m_iMixName[0]));
        }
        else if (g_MixRecipeMgr.GetCurRecipe()->m_iMixName[2] == 0)
        {
            mu_swprintf(strText, L"%ls %ls", I18N::Game::Lookup(g_MixRecipeMgr.GetCurRecipe()->m_iMixName[0]),
                I18N::Game::Lookup(g_MixRecipeMgr.GetCurRecipe()->m_iMixName[1]));
        }
        else
        {
            mu_swprintf(strText, L"%ls %ls %ls", I18N::Game::Lookup(g_MixRecipeMgr.GetCurRecipe()->m_iMixName[0]),
                I18N::Game::Lookup(g_MixRecipeMgr.GetCurRecipe()->m_iMixName[1]),
                I18N::Game::Lookup(g_MixRecipeMgr.GetCurRecipe()->m_iMixName[2]));
        }

        mu::ui::window::GenericDialogConfig cfg;
        cfg.showCancel = true;
        cfg.lines = {
            { strText, true },
            { I18N::Game::DoYouWantToCombineYourItems, false },
        };
        cfg.onPrimary = [mixType = g_MixRecipeMgr.GetMixInventoryType(), mixId = g_MixRecipeMgr.GetCurMixID(),
                         socketIndex = m_SocketSelection.Selected(), itemKeys = m_SocketItemKeys]
        {
            g_pMixInventory->ConfirmMix(mixType, mixId, socketIndex, itemKeys);
        };
        mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
        return true;
    }

    return false;
}

bool CMixInventory::InventoryProcess()
{
    CPickedItem* pPickedItem = CInventoryCtrl::GetPickedItem();

    if (m_pNewInventoryCtrl && pPickedItem)
    {
        const auto iCurInventory = g_MixRecipeMgr.GetMixInventoryEquipmentIndex();

        ITEM* pItemObj = pPickedItem->GetItem();
        if (GetMixState() == MIX_READY && g_MixRecipeMgr.IsMixSource(pPickedItem->GetItem()) &&
            pPickedItem->GetOwnerInventory() == g_pMyInventory->GetInventoryCtrl())
        {
            m_pNewInventoryCtrl->SetSquareColorNormal(m_fInventoryColor[0], m_fInventoryColor[1], m_fInventoryColor[2]);
            if (mu::ui::window::IsPress(VK_LBUTTON))
            {
                int iSourceIndex = pPickedItem->GetSourceLinealPos();
                int iTargetIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);
                if (iTargetIndex != -1 && m_pNewInventoryCtrl->CanMove(iTargetIndex, pItemObj))
                {
                    if (SendRequestEquipmentItem(STORAGE_TYPE::INVENTORY, iSourceIndex,
                        pItemObj, iCurInventory, iTargetIndex))
                    {
                        return true;
                    }
                }
            }
        }
        else if (pPickedItem->GetOwnerInventory() == m_pNewInventoryCtrl)
        {
            m_pNewInventoryCtrl->SetSquareColorNormal(m_fInventoryColor[0], m_fInventoryColor[1], m_fInventoryColor[2]);
            if (mu::ui::window::IsPress(VK_LBUTTON))
            {
                int iSourceIndex = pPickedItem->GetSourceLinealPos();
                int iTargetIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);
                if (iTargetIndex != -1 && m_pNewInventoryCtrl->CanMove(iTargetIndex, pItemObj))
                {
                    if (SendRequestEquipmentItem(iCurInventory, iSourceIndex,
                        pItemObj, iCurInventory, iTargetIndex))
                    {
                        return true;
                    }
                }
            }
        }
        else if (GetMixState() == MIX_READY && g_MixRecipeMgr.IsMixSource(pPickedItem->GetItem()) &&
            pItemObj->ex_src_type == ITEM_EX_SRC_EQUIPMENT)
        {
            m_pNewInventoryCtrl->SetSquareColorNormal(m_fInventoryColor[0], m_fInventoryColor[1], m_fInventoryColor[2]);
            if (mu::ui::window::IsPress(VK_LBUTTON))
            {
                int iSourceIndex = pPickedItem->GetSourceLinealPos();
                int iTargetIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);
                if (iTargetIndex != -1 && m_pNewInventoryCtrl->CanMove(iTargetIndex, pItemObj))
                {
                    SendRequestEquipmentItem(STORAGE_TYPE::INVENTORY, iSourceIndex,
                        pItemObj, iCurInventory, iTargetIndex);
                    return true;
                }
            }
        }
        else
        {
            m_pNewInventoryCtrl->SetSquareColorNormal(m_fInventoryWarningColor[0], m_fInventoryWarningColor[1], m_fInventoryWarningColor[2]);
        }
    }
    return false;
}

// Shared core for right-click moves: picks the item under the cursor in srcCtrl and moves it to an empty slot in dstCtrl.
bool CMixInventory::AutoMoveItem(CInventoryCtrl* srcCtrl, STORAGE_TYPE srcType,
    CInventoryCtrl* dstCtrl, STORAGE_TYPE dstType, bool requireMixSource)
{
    if (CInventoryCtrl::GetPickedItem())
        return false;

    if (srcCtrl == nullptr || dstCtrl == nullptr || GetMixState() != MIX_READY)
        return false;

    ITEM* pItemObj = srcCtrl->FindItemAtPt(MouseX, MouseY);
    if (pItemObj == nullptr)
        return false;

    if (requireMixSource && !g_MixRecipeMgr.IsMixSource(pItemObj))
        return false;

    const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItemObj->Type];
    const int iTargetIndex = dstCtrl->FindEmptySlot(pItemAttr->Width, pItemAttr->Height);
    if (iTargetIndex < 0 || !dstCtrl->CanMove(iTargetIndex, pItemObj))
        return false;

    if (!CInventoryCtrl::CreatePickedItem(srcCtrl, pItemObj))
        return false;

    CPickedItem* pPickedItem = CInventoryCtrl::GetPickedItem();
    if (pPickedItem == nullptr)
        return false;

    srcCtrl->RemoveItem(pItemObj);
    pPickedItem->HidePickedItem();

    if (!SendRequestEquipmentItem(srcType, pPickedItem->GetSourceLinealPos(), pItemObj, dstType, iTargetIndex))
    {
        CInventoryCtrl::BackupPickedItem();
        return false;
    }

    PlayBuffer(SOUND_GET_ITEM01);
    return true;
}

bool CMixInventory::ProcessMyInvenItemAutoMove(CInventoryCtrl* sourceCtrl)
{
    if (sourceCtrl == nullptr)
        sourceCtrl = g_pMyInventory ? g_pMyInventory->GetInventoryCtrl() : nullptr;

    if (sourceCtrl == nullptr || sourceCtrl->GetStorageType() != STORAGE_TYPE::INVENTORY)
        return false;

    return AutoMoveItem(sourceCtrl, STORAGE_TYPE::INVENTORY,
        m_pNewInventoryCtrl, g_MixRecipeMgr.GetMixInventoryEquipmentIndex(),
        /*requireMixSource*/ true);
}

bool CMixInventory::ProcessMixItemAutoMoveToInventory()
{
    CInventoryCtrl* dstCtrl = g_pMyInventory ? g_pMyInventory->GetInventoryCtrl() : nullptr;
    return AutoMoveItem(m_pNewInventoryCtrl, g_MixRecipeMgr.GetMixInventoryEquipmentIndex(),
        dstCtrl, STORAGE_TYPE::INVENTORY,
        /*requireMixSource*/ false);
}

bool CMixInventory::CheckMixInventory()
{
    g_MixRecipeMgr.ResetMixItemInventory();
    const size_t count = m_pNewInventoryCtrl->GetNumberOfItems();
    bool changed = count != m_SocketItemKeys.size();
    m_SocketItemKeys.resize(count);
    for (size_t i = 0; i < count; ++i)
    {
        ITEM* item = m_pNewInventoryCtrl->GetItem(static_cast<int>(i));
        changed |= m_SocketItemKeys[i] != item->Key;
        m_SocketItemKeys[i] = item->Key;
        g_MixRecipeMgr.AddItemToMixItemInventory(item);
    }
    if (changed)
        m_SocketSelection.ClearSelection();
    g_MixRecipeMgr.CheckMixInventory();
    return changed;
}

void CMixInventory::RenderMixEffect()
{
    if (m_iMixEffectTimer <= 0)
    {
        return;
    }
    else
    {
        --m_iMixEffectTimer;
    }
    EnableAlphaBlend();

    for (int i = 0; i < (int)m_pNewInventoryCtrl->GetNumberOfItems(); ++i)
    {
        int iWidth = ItemAttribute[m_pNewInventoryCtrl->GetItem(i)->Type].Width;
        int iHeight = ItemAttribute[m_pNewInventoryCtrl->GetItem(i)->Type].Height;

        for (int h = 0; h < iHeight; ++h)
        {
            for (int w = 0; w < iWidth; ++w)
            {
                const BYTE red = static_cast<BYTE>((rand() % 6 + 6) * 0.1f * 255.f);
                const BYTE green = static_cast<BYTE>((rand() % 4 + 4) * 0.1f * 255.f);
                const DWORD sparkleColor = RGBA(red, green, 51, 255);
                float Rotate = (float)((int)(WorldTime) % 100) * 20.f;
                float Scale = 5.f + (rand() % 10);
                const UI::Items::GridRect cell = m_pNewInventoryCtrl->Geometry().CellsRect(
                    m_pNewInventoryCtrl->GetItem(i)->x + w, m_pNewInventoryCtrl->GetItem(i)->y + h, 1, 1);
                float x = cell.x + (rand() % (std::max)(1, static_cast<int>(cell.width)));
                float y = cell.y + (rand() % (std::max)(1, static_cast<int>(cell.height)));
                RenderBitmapRotate(BITMAP_SHINY, x, y, Scale, Scale, 0, 0.f, 0.f, 1.f, 1.f, sparkleColor);
                RenderBitmapRotate(BITMAP_SHINY, x, y, Scale, Scale, Rotate, 0.f, 0.f, 1.f, 1.f, sparkleColor);
                RenderBitmapRotate(BITMAP_SHINY + 1, x, y, Scale * 3.f, Scale * 3.f, Rotate,
                    0.f, 0.f, 1.f, 1.f, sparkleColor);
                RenderBitmapRotate(BITMAP_LIGHT, x, y, Scale * 6.f, Scale * 6.f, 0,
                    0.f, 0.f, 1.f, 1.f, sparkleColor);
            }
        }
    }
    DisableAlphaBlend();
}

int mu::ui::window::CMixInventory::GetPointedItemIndex()
{
    return m_pNewInventoryCtrl->GetPointedSquareIndex();
}

// Into #item_view (m_ItemTarget), in this window's layout space.
void CMixInventory::RenderItems()
{
    if (m_pNewInventoryCtrl && m_pNewInventoryCtrl->IsVisible())
        m_pNewInventoryCtrl->Render3D();
    // The sparkle over the items while the mix runs.
    if (GetMixState() >= MIX_REQUESTED)
    {
        DisableDepthTest();
        EnableAlphaTest();
        RenderMixEffect();
        DisableAlphaBlend();
    }
}
