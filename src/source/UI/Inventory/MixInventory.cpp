
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
// lay its text out in physical pixels inside a counter-scaled layer).
float TextFitScale(Rml::Element* probe, const wchar_t* text, float boxWidth, float probeUnitsPerLayoutUnit)
{
    if (probe == nullptr || text == nullptr || text[0] == L'\0' || probeUnitsPerLayoutUnit <= 0.f)
        return 1.f;
    const float width =
        static_cast<float>(Rml::ElementUtilities::GetStringWidth(probe, StringUtils::WideToNarrow(text))) /
        probeUnitsPerLayoutUnit;
    return width <= boxWidth ? 1.f : boxWidth / width;
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
    if (NULL == pNewUIMng || NULL == g_pNewUI3DRenderMng || NULL == g_pNewItemMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_MIXINVENTORY, this);

    m_pNewInventoryCtrl = new CInventoryCtrl;
    if (false == m_pNewInventoryCtrl->Create(STORAGE_TYPE::CHAOS_MIX, g_pNewUI3DRenderMng, g_pNewItemMng, this, x + 15, y + 110, 8, 4))
    {
        SAFE_DELETE(m_pNewInventoryCtrl);
        return false;
    }

    SetPos(x, y);

    LoadImages();

    m_pNewInventoryCtrl->GetSquareColorNormal(m_fInventoryColor);
    m_pNewInventoryCtrl->GetSquareColorWarning(m_fInventoryWarningColor);

    BuildRmlUi();
    UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });

    Show(false);

    return true;
}

void CMixInventory::BuildRmlUi()
{
    // Guarded so the document/model are created once, even if Create() re-runs on resolution change.
    if (!m_pRmlDoc && RmlUiRuntime::Instance().IsCreated())
    {
        const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "mix_inventory",
            [this](Rml::DataModelConstructor& c, MixInventoryRmlModel& model)
            {
                c.Bind("root_x", &model.rootX);
                c.Bind("root_y", &model.rootY);
                c.Bind("root_scale", &model.rootScale);
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
                c.Bind("recipe_color", &model.recipeColor);

                c.Bind("show_success_rate", &model.showSuccessRate);
                c.Bind("success_rate_text", &model.successRateText);
                c.Bind("success_rate_color", &model.successRateColor);

                c.Bind("show_required_zen", &model.showRequiredZen);
                c.Bind("required_zen_text", &model.requiredZenText);

                c.Bind("show_prediction", &model.showPrediction);
                c.Bind("prediction_text", &model.predictionText);

                auto mixLine = c.RegisterStruct<MixLine>();
                mixLine.RegisterMember("text", &MixLine::text);
                mixLine.RegisterMember("color", &MixLine::color);
                mixLine.RegisterMember("top", &MixLine::top);
                mixLine.RegisterMember("align_left", &MixLine::alignLeft);
                mixLine.RegisterMember("fit", &MixLine::fit);
                c.RegisterArray<std::vector<MixLine>>();
                c.Bind("source_lines", &model.sourceLines);
                c.Bind("status_lines", &model.statusLines);
                c.Bind("advice_lines", &model.adviceLines);
                c.Bind("description_lines", &model.descriptionLines);

                c.Bind("show_socket_prompt", &model.showSocketPrompt);
                c.Bind("socket_prompt_text", &model.socketPromptText);

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
            });

        if (modelCreated)
            m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), "Data/Interface/RmlUi/mix_inventory.rml");

        // Frame background panel uses the background context -- see MixInventoryBgRmlModel (MixInventory.h).
        if (Rml::Context* bgContext = RmlUiRuntime::Instance().GetBackgroundContext())
        {
            const bool bgModelCreated = m_BgRmlBinder.Create(bgContext, "mix_inventory_bg",
                [](Rml::DataModelConstructor& c, MixInventoryBgRmlModel& model)
                {
                    c.Bind("root_x", &model.rootX);
                    c.Bind("root_y", &model.rootY);
                    c.Bind("root_scale", &model.rootScale);
                });
            if (bgModelCreated)
            {
                // Starts hidden -- CreateBackgroundDocument() no longer Show()s eagerly (see its
                // own comment, RmlTheme.h); SyncRmlModel() below is what shows/hides it.
                m_pRmlBgDoc = UI::RmlBridge::CreateBackgroundDocument("Data/Interface/RmlUi/mix_inventory_bg.rml");
            }
        }

        // Not Show()n here -- m_pRmlDoc's visibility follows this window's own Show()/Hide() via
        // SyncRmlModel(), not an eager Show() at Create() time.
    }
}

void CMixInventory::ReloadRmlTheme()
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

void CMixInventory::Release()
{
    UnloadImages();

    SAFE_DELETE(m_pNewInventoryCtrl);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        UI::RmlBridge::UnregisterForThemeReload(this);
        m_pNewUIMng = NULL;
    }
    if (g_pNewUI3DRenderMng)
        g_pNewUI3DRenderMng->DeleteUI2DEffectObject(UI2DEffectCallback);
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
        m_SocketListBox.Clear();
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
    // first frame after Create()/Show(true)/ReloadRmlTheme(), before RmlUi's next layout pass.
    float panelWidth = INVENTORY_WIDTH;
    float panelHeight = INVENTORY_HEIGHT;
    UI::RmlBridge::RefreshLogicalPanelSize(m_pRmlDoc, "panel", panelWidth, panelHeight);
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
        switch (g_MixRecipeMgr.GetMixInventoryType())
        {
        case SEASON3A::MIXTYPE_ATTACH_SOCKET:
        case SEASON3A::MIXTYPE_DETACH_SOCKET:
        {
            if (m_SocketListBox.GetLineNum() == 0)
            {
                wchar_t szText[64] = { 0, };
                wchar_t szSocketText[64] = { 0, };
                for (int i = 0; i < g_MixRecipeMgr.GetFirstItemSocketCount(); ++i)
                {
                    if (g_MixRecipeMgr.GetFirstItemSocketSeedID(i) == SOCKET_EMPTY)
                    {
                        mu_swprintf(szSocketText, I18N::Game::NoItemApplication);
                    }
                    else
                    {
                        g_SocketItemMgr.CreateSocketOptionText(szSocketText, g_MixRecipeMgr.GetFirstItemSocketSeedID(i), g_MixRecipeMgr.GetFirstItemSocketShpereLv(i));
                    }
                    mu_swprintf(szText, L"%d: %ls", i + 1, szSocketText);
                    m_SocketListBox.AddText(i, szText);
                }
                m_SocketListBox.SLSetSelectLine(0);
            }
            else
            {
                if (g_MixRecipeMgr.GetFirstItemSocketCount() == 0)
                {
                    m_SocketListBox.Clear();
                }
            }
            m_SocketListBox.DoAction();
        }
        break;
        }
    }

    SyncRmlModel();
    return true;
}
bool CMixInventory::Render()
{
    EnableAlphaTest();

    // Frame background panel is RmlUi, routed through the background context (see
    // MixInventoryBgRmlModel), painted by CManager::Render()'s centralized RenderBackgroundLayer()
    // call before this window's own Render()/Render3D() run. Recipe/tax-rate/success-rate content
    // is RmlUi too now (SyncMixContentModel()) -- only the socket list box below is still a real
    // native interactive widget, not presentation.
    const int mixType = g_MixRecipeMgr.GetMixInventoryType();
    if (mixType == SEASON3A::MIXTYPE_ATTACH_SOCKET || mixType == SEASON3A::MIXTYPE_DETACH_SOCKET)
        m_SocketListBox.Render();

    if (m_pNewInventoryCtrl)
        m_pNewInventoryCtrl->Render();

    if (GetMixState() >= MIX_REQUESTED && g_pNewUI3DRenderMng)
        g_pNewUI3DRenderMng->RenderUI2DEffect(INVENTORY_CAMERA_Z_ORDER, UI2DEffectCallback, this, 0, 0);

    DisableAlphaBlend();

    return true;
}

void CMixInventory::SyncRmlModel()
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
    auto syncWide = [&](Rml::String MixInventoryRmlModel::* field, const char* boundName, const wchar_t* text)
    {
        const Rml::String value = StringUtils::WideToNarrow(text);
        if (model.*field != value) { model.*field = value; m_RmlBinder.MarkDirty(boundName); }
    };
    auto syncBool = [&](bool MixInventoryRmlModel::* field, const char* boundName, bool value)
    {
        if (model.*field != value) { model.*field = value; m_RmlBinder.MarkDirty(boundName); }
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
}

float CMixInventory::GetLayerDepth()
{
    return 3.4f;
}

CInventoryCtrl* CMixInventory::GetInventoryCtrl() const
{
    return m_pNewInventoryCtrl;
}

void CMixInventory::UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB)
{
    if (pClass)
    {
        auto* pMixInventory = (CMixInventory*)pClass;
        pMixInventory->RenderMixEffect();
    }
}

void CMixInventory::LoadImages()
{
    // Frame/top/sides/bottom + Mix button sprites are RmlUi now (mix_inventory[_bg].rcss);
    // these scrollbar images remain for m_SocketListBox, which stays fully native.
    LoadBitmap(L"Interface\\newui_scrollbar_up.tga", CGuardWindow::IMAGE_GUARDWINDOW_SCROLL_TOP);
    LoadBitmap(L"Interface\\newui_scrollbar_m.tga", CGuardWindow::IMAGE_GUARDWINDOW_SCROLL_MIDDLE);
    LoadBitmap(L"Interface\\newui_scrollbar_down.tga", CGuardWindow::IMAGE_GUARDWINDOW_SCROLL_BOTTOM);
    LoadBitmap(L"Interface\\newui_scroll_on.tga", CGuardWindow::IMAGE_GUARDWINDOW_SCROLLBAR_ON, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_scroll_off.tga", CGuardWindow::IMAGE_GUARDWINDOW_SCROLLBAR_OFF, GL_LINEAR);
}
void CMixInventory::UnloadImages()
{
    DeleteBitmap(CGuardWindow::IMAGE_GUARDWINDOW_SCROLL_TOP);
    DeleteBitmap(CGuardWindow::IMAGE_GUARDWINDOW_SCROLL_MIDDLE);
    DeleteBitmap(CGuardWindow::IMAGE_GUARDWINDOW_SCROLL_BOTTOM);
    DeleteBitmap(CGuardWindow::IMAGE_GUARDWINDOW_SCROLLBAR_ON);
    DeleteBitmap(CGuardWindow::IMAGE_GUARDWINDOW_SCROLLBAR_OFF);
}

bool CMixInventory::BtnProcess()
{
    // Top-right corner close "X" (shared frame): hides + swallows the click.
    g_pNewUISystem->HandleFrameCornerClose(m_Pos, mu::ui::window::INTERFACE_MIXINVENTORY);

    return false;
}

void CMixInventory::SyncMixContentModel()
{
    if (!m_pRmlDoc)
        return;

    // The bold title measures lines in the window's own text font (the theme sizes it 1em). A
    // counter-scaled title (legacy .sharp-text) measures in physical pixels.
    Rml::Element* fitProbe = m_pRmlDoc->GetElementById("title");
    float probeUnitsPerLayoutUnit = 1.f;
    if (fitProbe != nullptr && fitProbe->GetComputedValues().has_local_transform())
        probeUnitsPerLayoutUnit = UI::Scaling::TransformForLayout(GetLayoutMode(), WindowWidth, WindowHeight).scaleX;
    auto& model = m_RmlBinder.GetModel();
    auto syncWide = [&](Rml::String MixInventoryRmlModel::* field, const char* boundName, const wchar_t* text)
    {
        const Rml::String value = StringUtils::WideToNarrow(text);
        if (model.*field != value) { model.*field = value; m_RmlBinder.MarkDirty(boundName); }
    };
    auto syncBool = [&](bool MixInventoryRmlModel::* field, const char* boundName, bool value)
    {
        if (model.*field != value) { model.*field = value; m_RmlBinder.MarkDirty(boundName); }
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
        if (model.*field != value) { model.*field = value; m_RmlBinder.MarkDirty(boundName); }
    };
    auto syncLines = [&](std::vector<MixLine> MixInventoryRmlModel::* field, const char* boundName,
        std::vector<MixLine> newLines)
    {
        if (model.*field != newLines) { model.*field = std::move(newLines); m_RmlBinder.MarkDirty(boundName); }
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
        const float fit = TextFitScale(fitProbe, szText, kTaxRateBoxWidth, probeUnitsPerLayoutUnit);
        if (model.taxRateFit != fit)
        {
            model.taxRateFit = fit;
            m_RmlBinder.MarkDirty("tax_rate_fit");
        }
    }

    // Recipe result name onward -- hidden entirely once MIX_FINISHED, mirroring RenderFrame()'s own
    // early return (nothing past that point ever rendered either).
    const bool showRecipe = (GetMixState() != MIX_FINISHED);
    syncBool(&MixInventoryRmlModel::showRecipe, "show_recipe", showRecipe);
    if (!showRecipe)
        return;

    if (g_MixRecipeMgr.IsReadyToMix())
        syncColor(&MixInventoryRmlModel::recipeColor, "recipe_color", 255, 255, 48, 255);
    else
        syncColor(&MixInventoryRmlModel::recipeColor, "recipe_color", 255, 48, 48, 255);

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
            syncColor(&MixInventoryRmlModel::successRateColor, "success_rate_color", 255, 255, 48, 255);
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
            syncColor(&MixInventoryRmlModel::successRateColor, "success_rate_color", 210, 230, 255, 255);
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

            Rml::String color;
            if (iResult == SEASON3A::MIX_SOURCE_NO) color = makeColor(255, 50, 20, 255);
            else if (iResult == SEASON3A::MIX_SOURCE_PARTIALLY) color = makeColor(210, 230, 255, 255);
            else if (iResult == SEASON3A::MIX_SOURCE_YES) color = makeColor(255, 255, 48, 255);

            sourceLines.push_back({ StringUtils::WideToNarrow(szText), color });
        }
    }
    else if (g_MixRecipeMgr.IsMixInit())
    {
        statusLines.push_back({ StringUtils::WideToNarrow(I18N::Game::PleaseUploadTheAssemblyItems), makeColor(255, 50, 20, 255) });
    }
    else
    {
        mu_swprintf(szText, I18N::Game::AssemblyPredictionS, L" ");
        statusLines.push_back({ StringUtils::WideToNarrow(szText), makeColor(255, 50, 20, 255) });
        statusLines.push_back({ StringUtils::WideToNarrow(I18N::Game::ImproperItemsForCombination), makeColor(255, 50, 20, 255) });
    }

    syncBool(&MixInventoryRmlModel::showPrediction, "show_prediction", showPrediction);
    if (showPrediction)
        syncWide(&MixInventoryRmlModel::predictionText, "prediction_text", predictionText);
    syncLines(&MixInventoryRmlModel::sourceLines, "source_lines", sourceLines);
    syncLines(&MixInventoryRmlModel::statusLines, "status_lines", statusLines);

    // Recipe description (ready to mix) or advice for the closest match (not ready) -- former
    // trailing block, always rendered in the same reddish tone natively.
    std::vector<MixLine> adviceLines;
    const Rml::String adviceColor = makeColor(255, 50, 20, 255);
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
    const Rml::String white = makeColor(255, 255, 255, 255);
    const Rml::String warning = makeColor(255, 40, 20, 255);
    constexpr float kDescriptionTop = 250.f; // RenderMixDescriptions()'s fPos_y + 250 block
    constexpr float kCastleSeniorDescriptionTop = 270.f;
    constexpr float kDescriptionRow = 13.f;
    // Native boxes: 160 centred at x+15, or 200 left-aligned from x+5 -- which runs 15 past the
    // 190-wide window; the left box is kept inside it (180) so no line overflows the frame.
    constexpr float kCentredDescriptionWidth = 160.f;
    constexpr float kLeftDescriptionWidth = 180.f;
    auto describeAt = [&](float blockTop, const wchar_t* text, const Rml::String& color, int row, bool alignLeft)
    {
        const float top = blockTop + static_cast<float>(row) * kDescriptionRow;
        const float fit = TextFitScale(fitProbe, text, alignLeft ? kLeftDescriptionWidth : kCentredDescriptionWidth,
                                       probeUnitsPerLayoutUnit);
        descriptionLines.push_back({StringUtils::WideToNarrow(text), color, top, alignLeft, fit});
    };
    auto describe = [&](const wchar_t* text, const Rml::String& color, int row, bool alignLeft = false)
    { describeAt(kDescriptionTop, text, color, row, alignLeft); };
    switch (mixType)
    {
    case SEASON3A::MIXTYPE_CASTLE_SENIOR:
        for (int i = 0; i < 6; ++i)
            describeAt(kCastleSeniorDescriptionTop, I18N::Game::Lookup(1644 + i), makeColor(200, 200, 200, 255), i,
                       false);
        break;
    case SEASON3A::MIXTYPE_OSBOURNE:
        describe(I18N::Game::RefineTheItemToCreate, white, 0);
        describe(I18N::Game::TheRefiningStone, white, 1);
        mu_swprintf(szText, I18N::Game::SForOnlyS, I18N::Game::Refine, I18N::Game::WeaponsOrShields);
        describe(szText, white, 2);
        describe(I18N::Game::Allowed, white, 3);
        describe(I18N::Game::ItemWillDisappearWhenFailed, makeColor(255, 0, 0, 255), 4);
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
    syncBool(&MixInventoryRmlModel::showSocketPrompt, "show_socket_prompt", showSocketPrompt);
    if (showSocketPrompt)
        syncWide(&MixInventoryRmlModel::socketPromptText, "socket_prompt_text", socketPromptText);
}

int CMixInventory::Rtn_MixRequireZen(int _nMixZen, int _nTax)
{
    if (_nTax)		_nMixZen += ((LONGLONG)_nMixZen * g_nChaosTaxRate) / 100;
    return _nMixZen;
}

bool CMixInventory::Mix()
{
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

    if (g_MixRecipeMgr.GetMixInventoryType() == SEASON3A::MIXTYPE_ATTACH_SOCKET)
    {
        int iSelectedLine = m_SocketListBox.GetLineNum() - m_SocketListBox.SLGetSelectLineNum();

        for (int i = 0; i < m_SocketListBox.GetLineNum(); ++i)
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

        if (m_SocketListBox.SLGetSelectLineNum() == 0)
        {
            g_pSystemLogBox->AddText(I18N::Game::YouMustSelectTheSocket, mu::ui::window::TYPE_ERROR_MESSAGE);
            return false;
        }
        else if (iSelectedLine > g_MixRecipeMgr.GetFirstItemSocketCount()
            || g_MixRecipeMgr.GetFirstItemSocketSeedID(iSelectedLine) != SOCKET_EMPTY)
        {
            g_pSystemLogBox->AddText(I18N::Game::ItSAlreadyAppliedOnTheCharacter, mu::ui::window::TYPE_ERROR_MESSAGE);
            return false;
        }

        g_MixRecipeMgr.SetMixSubType(iSelectedLine);
    }
    else if (g_MixRecipeMgr.GetMixInventoryType() == SEASON3A::MIXTYPE_DETACH_SOCKET)
    {
        int iSelectedLine = m_SocketListBox.GetLineNum() - m_SocketListBox.SLGetSelectLineNum();
        if (m_SocketListBox.SLGetSelectLineNum() == 0)
        {
            g_pSystemLogBox->AddText(I18N::Game::YouMustSelectTheDestructibleSocket, mu::ui::window::TYPE_ERROR_MESSAGE);
            return false;
        }
        else if (iSelectedLine > g_MixRecipeMgr.GetFirstItemSocketCount()
            || g_MixRecipeMgr.GetFirstItemSocketSeedID(iSelectedLine) == SOCKET_EMPTY)
        {
            g_pSystemLogBox->AddText(I18N::Game::ThereAreNoDestructibleSeedSpheres, mu::ui::window::TYPE_ERROR_MESSAGE);
            return false;
        }
        g_MixRecipeMgr.SetMixSubType(iSelectedLine);
    }

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
        cfg.onPrimary = []
        {
            g_pMixInventory->SetMixState(mu::ui::window::CMixInventory::MIX_REQUESTED);
            SocketClient->ToGameServer()->SendChaosMachineMixRequest(
                static_cast<ChaosMachineMixType>(g_MixRecipeMgr.GetCurMixID()),
                g_MixRecipeMgr.GetMixSubType());
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

void CMixInventory::CheckMixInventory()
{
    g_MixRecipeMgr.ResetMixItemInventory();
    ITEM* pItem = NULL;
    for (int i = 0; i < (int)m_pNewInventoryCtrl->GetNumberOfItems(); ++i)
    {
        pItem = m_pNewInventoryCtrl->GetItem(i);
        g_MixRecipeMgr.AddItemToMixItemInventory(pItem);
    }
    g_MixRecipeMgr.CheckMixInventory();
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
                float x = m_pNewInventoryCtrl->GetPos().x +
                    (m_pNewInventoryCtrl->GetItem(i)->x + w) * INVENTORY_SQUARE_WIDTH +
                    (rand() % INVENTORY_SQUARE_WIDTH);
                float y = m_pNewInventoryCtrl->GetPos().y +
                    (m_pNewInventoryCtrl->GetItem(i)->y + h) * INVENTORY_SQUARE_WIDTH +
                    (rand() % INVENTORY_SQUARE_WIDTH);
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
