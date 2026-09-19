
#include "stdafx.h"
#include <algorithm>
#include "I18N/All.h"

#include "UI/HUD/MainFrameWindow.h"	// self
#include "UI/Options/OptionWindow.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Widgets/UIBaseDef.h"
#include "Audio/DSPlaySound.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzInventory.h"

#include "GameLogic/Items/CSItemOption.h"
#include "GameLogic/Events/CSChaosCastle.h"
#include "World/MapInfra/MapManager.h"
#include "Character/CharacterManager.h"
#include "GameLogic/Skills/SkillManager.h"
#include "UI/HUD/Skills/SkillTooltip.h"
#include "UI/Scaling/UITransform.h"
#include "Core/Time/CTimCheck.h"
#include "GameLogic/Social/MonkSystem.h"

#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
#include "GameShop/InGameShopSystem.h"
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME

// RmlUi migration -- see this class's header comment.
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Core/Utilities/StringUtils.h"
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>
#include "GameLogic/Quests/QuestMng.h"
#include "UI/Party/FriendWindow.h"

namespace
{
    // Still used by RenderLeftFrame()/RenderCenterFrame(), the two chrome regions that still host legacy content.
    constexpr float kHudTop = 429.0f;
    constexpr float kHudContentHeight = 41.0f;
    constexpr float kLeftBandWidth = 152.0f;
    constexpr float kCenterBandStart = 152.0f;
    constexpr float kMenu1CenterWidth = 104.0f;
    constexpr float kMenu2Start = 256.0f;
    constexpr float kMenu2Width = 128.0f;
    constexpr float kMenu3Start = 384.0f;
    constexpr float kMenu3CenterWidth = 104.0f;
}

mu::ui::window::CMainFrameWindow::CMainFrameWindow()
{
    m_bExpEffect = false;
    m_dwExpEffectTime = 0;
    m_dwPreExp = 0;
    m_dwGetExp = 0;
    m_bButtonBlink = false;
}

mu::ui::window::CMainFrameWindow::~CMainFrameWindow()
{
    Release();
}

void mu::ui::window::CMainFrameWindow::LoadImages()
{
    LoadBitmap(L"Interface\\newui_menu01.jpg", IMAGE_MENU_1, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_menu02.jpg", IMAGE_MENU_2, GL_LINEAR);
    LoadBitmap(L"Interface\\partCharge1\\newui_menu03.jpg", IMAGE_MENU_3, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_menu02-03.jpg", IMAGE_MENU_2_1, GL_LINEAR);
}

void mu::ui::window::CMainFrameWindow::UnloadImages()
{
    DeleteBitmap(IMAGE_MENU_1);
    DeleteBitmap(IMAGE_MENU_2);
    DeleteBitmap(IMAGE_MENU_3);
    DeleteBitmap(IMAGE_MENU_2_1);
}

bool mu::ui::window::CMainFrameWindow::Create(CManager* pNewUIMng, C3DRenderMng* pNewUI3DRenderMng)
{
    if (NULL == pNewUIMng || NULL == pNewUI3DRenderMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_MAINFRAME, this);

    m_pNewUI3DRenderMng = pNewUI3DRenderMng;
    m_pNewUI3DRenderMng->Add3DRenderObj(this, ITEMHOTKEYNUMBER_CAMERA_Z_ORDER);

    LoadImages();

    // Guarded so the doc/model are created once, even though Create() re-runs on resolution change.
    if (!m_pRmlDoc && RmlUiRuntime::Instance().IsCreated())
        BuildRmlUi();

    Show(true);

    return true;
}

void mu::ui::window::CMainFrameWindow::BuildRmlUi()
{
    const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "main_frame",
            [this](Rml::DataModelConstructor& c, MainFrameRmlModel& model)
            {
                c.Bind("bars_left", &model.barsLeft);
                c.Bind("bars_top", &model.barsTop);
                c.Bind("bars_scale", &model.barsScale);

                c.Bind("hp_fraction", &model.hpFraction);
                c.Bind("mp_fraction", &model.mpFraction);
                c.Bind("ag_fraction", &model.agFraction);
                c.Bind("sd_fraction", &model.sdFraction);
                c.Bind("hp_text", &model.hpText);
                c.Bind("mp_text", &model.mpText);
                c.Bind("ag_text", &model.agText);
                c.Bind("sd_text", &model.sdText);
                c.Bind("hp_current_text", &model.hpCurrentText);
                c.Bind("mp_current_text", &model.mpCurrentText);
                c.Bind("ag_current_text", &model.agCurrentText);
                c.Bind("sd_current_text", &model.sdCurrentText);
                c.Bind("hp_tooltip", &model.hpTooltip);
                c.Bind("mp_tooltip", &model.mpTooltip);
                c.Bind("ag_tooltip", &model.agTooltip);
                c.Bind("sd_tooltip", &model.sdTooltip);
                c.Bind("poisoned", &model.poisoned);

                c.Bind("exp_fraction", &model.expFraction);
                c.Bind("exp_digit", &model.expDigit);
                c.Bind("exp_tooltip", &model.expTooltip);

                c.Bind("cshop_open", &model.cShopOpen);
                c.Bind("chainfo_open", &model.chaInfoOpen);
                c.Bind("myinven_open", &model.myInvenOpen);
                c.Bind("friend_open", &model.friendOpen);
                c.Bind("window_open", &model.windowOpen);
                c.Bind("cshop_tooltip", &model.cShopTooltip);
                c.Bind("chainfo_tooltip", &model.chaInfoTooltip);
                c.Bind("myinven_tooltip", &model.myInvenTooltip);
                c.Bind("friend_tooltip", &model.friendTooltip);
                c.Bind("window_tooltip", &model.windowTooltip);

                c.Bind("chainfo_alert", &model.chaInfoAlert);
                c.Bind("friend_alert", &model.friendAlert);

                c.Bind("skill_slot_0_selected", &model.skillSlot0Selected);
                c.Bind("skill_slot_1_selected", &model.skillSlot1Selected);
                c.Bind("skill_slot_2_selected", &model.skillSlot2Selected);
                c.Bind("skill_slot_3_selected", &model.skillSlot3Selected);
                c.Bind("skill_slot_4_selected", &model.skillSlot4Selected);

                c.Bind("skill_slot_0_hotkey", &model.skillSlot0Hotkey);
                c.Bind("skill_slot_1_hotkey", &model.skillSlot1Hotkey);
                c.Bind("skill_slot_2_hotkey", &model.skillSlot2Hotkey);
                c.Bind("skill_slot_3_hotkey", &model.skillSlot3Hotkey);
                c.Bind("skill_slot_4_hotkey", &model.skillSlot4Hotkey);

                // Skill list cooldown bindings -- see MainFrameRmlModel::skillGridOpen's own
                // header comment.
                c.Bind("skill_slot_0_cooldown", &model.skillSlot0Cooldown);
                c.Bind("skill_slot_1_cooldown", &model.skillSlot1Cooldown);
                c.Bind("skill_slot_2_cooldown", &model.skillSlot2Cooldown);
                c.Bind("skill_slot_3_cooldown", &model.skillSlot3Cooldown);
                c.Bind("skill_slot_4_cooldown", &model.skillSlot4Cooldown);
                c.Bind("current_skill_cooldown", &model.currentSkillCooldown);

                // See CCharMakeWin::BuildRmlUi()'s comment on why this must re-run in full every
                // call, including from ReloadRmlTheme() -- no guard here.
                auto skillCell = c.RegisterStruct<SkillCellEntry>();
                skillCell.RegisterMember("left", &SkillCellEntry::left);
                skillCell.RegisterMember("top", &SkillCellEntry::top);
                skillCell.RegisterMember("skill_index", &SkillCellEntry::skillIndex);
                skillCell.RegisterMember("is_pet", &SkillCellEntry::isPet);
                skillCell.RegisterMember("is_current", &SkillCellEntry::isCurrent);
                skillCell.RegisterMember("cooldown_fraction", &SkillCellEntry::cooldownFraction);
                c.RegisterArray<std::vector<SkillCellEntry>>();

                auto tooltipLine = c.RegisterStruct<SkillTooltipLineEntry>();
                tooltipLine.RegisterMember("text", &SkillTooltipLineEntry::text);
                tooltipLine.RegisterMember("color_blue", &SkillTooltipLineEntry::colorBlue);
                tooltipLine.RegisterMember("color_red", &SkillTooltipLineEntry::colorRed);
                tooltipLine.RegisterMember("color_dark_red", &SkillTooltipLineEntry::colorDarkRed);
                tooltipLine.RegisterMember("bold", &SkillTooltipLineEntry::bold);
                c.RegisterArray<std::vector<SkillTooltipLineEntry>>();

                c.Bind("skill_grid_open", &model.skillGridOpen);
                c.Bind("skill_grid_cells", &model.skillGridCells);
                c.Bind("pet_skill_cells", &model.petSkillCells);

                c.Bind("skill_tooltip_visible", &model.skillTooltipVisible);
                c.Bind("skill_tooltip_left", &model.skillTooltipLeft);
                c.Bind("skill_tooltip_top", &model.skillTooltipTop);
                c.Bind("skill_tooltip_lines", &model.skillTooltipLines);

                // Skill list click/hover bindings route into CSkillList (g_pSkillList has no RmlUi
                // doc of its own). Args are literal ints in RML (e.g. skill_hotkey_click(0)) or the
                // cell's skill_index for data-for'd lists; Variant::Get<int>() resolves either.
                c.BindEventCallback("skill_hotkey_click",
                    [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args) { g_pSkillList->OnHotkeySlotClick(args.empty() ? 0 : args[0].Get<int>()); });
                c.BindEventCallback("skill_hotkey_hover",
                    [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args) { g_pSkillList->OnHotkeySlotHover(args.empty() ? 0 : args[0].Get<int>()); });
                c.BindEventCallback("skill_current_click",
                    [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { g_pSkillList->OnCurrentSkillClick(); });
                c.BindEventCallback("skill_current_hover",
                    [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { g_pSkillList->OnCurrentSkillHover(); });
                c.BindEventCallback("skill_grid_click",
                    [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args) { g_pSkillList->OnGridCellClick(args.empty() ? -1 : args[0].Get<int>()); });
                c.BindEventCallback("skill_grid_hover",
                    [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args) { g_pSkillList->OnGridCellHover(args.empty() ? -1 : args[0].Get<int>()); });
                c.BindEventCallback("skill_pet_click",
                    [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args) { g_pSkillList->OnPetCellClick(args.empty() ? -1 : args[0].Get<int>()); });
                c.BindEventCallback("skill_pet_hover",
                    [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args) { g_pSkillList->OnPetCellHover(args.empty() ? -1 : args[0].Get<int>()); });
                c.BindEventCallback("skill_unhover",
                    [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { g_pSkillList->OnUnhover(); });

                // Item hotkey chrome (#item_slots hover/stack-count/right-click). m_ItemHotKey is
                // a member, so these lambdas capture [this] directly.
                c.Bind("item_slot_0_hovered", &model.itemSlot0Hovered);
                c.Bind("item_slot_1_hovered", &model.itemSlot1Hovered);
                c.Bind("item_slot_2_hovered", &model.itemSlot2Hovered);
                c.Bind("item_slot_3_hovered", &model.itemSlot3Hovered);
                c.Bind("item_slot_0_count", &model.itemSlot0Count);
                c.Bind("item_slot_1_count", &model.itemSlot1Count);
                c.Bind("item_slot_2_count", &model.itemSlot2Count);
                c.Bind("item_slot_3_count", &model.itemSlot3Count);

                c.BindEventCallback("item_hotkey_hover",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args) { m_ItemHotKey.OnHotkeySlotHover(args.empty() ? 0 : args[0].Get<int>()); });
                c.BindEventCallback("item_hotkey_unhover",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { m_ItemHotKey.OnUnhover(); });
                // Uses data-event-mouseup, not data-event-click -- RmlUi's Context only dispatches
                // Click for the left button. Mouseup fires for any button; "button" param == 1 means right-click.
                c.BindEventCallback("item_hotkey_rightclick",
                    [this](Rml::DataModelHandle, Rml::Event& event, const Rml::VariantList& args)
                    {
                        if (event.GetParameter<int>("button", -1) != 1) return;
                        m_ItemHotKey.OnHotkeySlotRightClick(args.empty() ? 0 : args[0].Get<int>());
                    });

                c.BindEventCallback("mainframe_cshop_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickCShop(); });
                c.BindEventCallback("mainframe_chainfo_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickChaInfo(); });
                c.BindEventCallback("mainframe_myinven_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickMyInven(); });
                c.BindEventCallback("mainframe_friend_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickFriend(); });
                c.BindEventCallback("mainframe_window_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickWindow(); });
            });

        if (modelCreated)
            m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), "Data/Interface/RmlUi/main_frame.rml");

        // Gated on ThemeProvidesOwnIconChrome() (see m_BgRmlBinder's header comment) -- themes
        // without that capability don't ship main_frame_bg.rml.
        if (UI::RmlBridge::ThemeProvidesOwnIconChrome())
        {
            if (Rml::Context* bgContext = RmlUiRuntime::Instance().GetBackgroundContext())
            {
                const bool bgModelCreated = m_BgRmlBinder.Create(bgContext, "main_frame_bg",
                    [](Rml::DataModelConstructor& c, MainFrameBgRmlModel& model)
                    {
                        c.Bind("root_x", &model.rootX);
                        c.Bind("root_y", &model.rootY);
                        c.Bind("root_scale", &model.rootScale);
                        c.Bind("left_offset_x", &model.leftOffsetX);
                        c.Bind("center_offset_x", &model.centerOffsetX);
                    });
                if (bgModelCreated)
                {
                    m_pRmlBgDoc = UI::RmlBridge::LoadThemedDocument(bgContext, "Data/Interface/RmlUi/main_frame_bg.rml");
                    // Not Show()n here -- CManager::Render()'s centralized RenderBackgroundLayer()
                    // call runs every frame regardless of this window's own visibility, so
                    // SyncDocVisibility() now gates m_pRmlBgDoc the same way it gates m_pRmlDoc.
                }
            }
        }

        // Not Show()n here -- Create() runs before SceneFlag reaches MAIN_SCENE; an eager Show()
        // here let the HUD flash once before the first scene gate check. Left hidden;
        // SyncDocVisibility() shows it once the gate allows it.
}

void mu::ui::window::CMainFrameWindow::ReloadRmlTheme()
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
    // Next frame's Update()/SyncDocVisibility() self-corrects live state/visibility for both docs.
}

void mu::ui::window::CMainFrameWindow::Release()
{
    UnloadImages();

    if (m_pNewUI3DRenderMng)
    {
        m_pNewUI3DRenderMng->Remove3DRenderObj(this);
        m_pNewUI3DRenderMng = NULL;
    }

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }

    // Hide directly since RmlUi renders last in the frame regardless of scene (see CMuHelperBar::Release()).
    if (m_pRmlDoc)
        m_pRmlDoc->Hide();

    // Not load-bearing like m_pRmlDoc's Hide() above -- unreachable once RemoveUIObj() takes effect. Hidden anyway, defensively.
    if (m_pRmlBgDoc)
        m_pRmlBgDoc->Hide();
}

bool mu::ui::window::CMainFrameWindow::Render()
{
    // Thin passthrough, not a full no-op: only the two chrome bands with legacy content (item
    // hotkeys, skill list) still draw here; the rest moved to RmlUi and is synced by
    // SyncRmlModel().
    //
    // leftTransform/centerTransform each add their own theme-provided offset
    // (GetItemHotkeyOffsetX()/GetSkillListOffsetX()) on top of the shared BottomHudCenterTransform,
    // so a theme can reposition the item-hotkey/skill-hotkey bands independently via RCSS.
    //
    // `* baseTransform.scaleX` is required: GetAbsoluteOffset() ignores CSS `transform: scale()`,
    // so the offsets above are unscaled reference-pixel deltas, not real screen pixels -- adding
    // them raw would under-shift the icons relative to the correctly-scaled RmlUi outline boxes.
    EnableAlphaTest();

    const auto baseTransform = UI::Scaling::BottomHudCenterTransform(WindowWidth, WindowHeight);

    auto leftTransform = baseTransform;
    leftTransform.offsetX += GetItemHotkeyOffsetX() * baseTransform.scaleX;

    auto centerTransform = baseTransform;
    centerTransform.offsetX += GetSkillListOffsetX() * baseTransform.scaleX;

    {
        UI::Scaling::ScopedActiveTransform layout(leftTransform);
        RenderLeftFrame();
    }
    {
        UI::Scaling::ScopedActiveTransform layout(centerTransform);
        RenderCenterFrame();
    }

    {
        UI::Scaling::ScopedActiveTransform layout(centerTransform, true);
        RenderCenterRegion();
    }
    DisableAlphaBlend();

    return true;
}

void mu::ui::window::CMainFrameWindow::Render3D()
{
    // Uses centerTransform (item-hotkey band anchors next to the HP bar, not the window's left edge).
    //
    // transformMouse=true is required: RenderItem3D()'s hover check compares raw MouseX/MouseY
    // against reference-space coordinates directly, so without it the hover test only lines up at
    // identity scale/offset. Left independent of #item_slots' own RmlUi hover highlight since
    // RenderItem3D() is a shared free function used elsewhere too.
    //
    // `* scaleX` must match Render()'s leftTransform exactly, or the 3D icons render somewhere
    // other than where RenderLeftFrame()'s chrome and #item_slots' hit-testing expect them.
    auto transform = UI::Scaling::BottomHudCenterTransform(WindowWidth, WindowHeight);
    transform.offsetX += GetItemHotkeyOffsetX() * transform.scaleX;
    UI::Scaling::ScopedActiveTransform layout(transform, true);
    m_ItemHotKey.RenderItems();
}

bool mu::ui::window::CMainFrameWindow::IsVisible() const
{
    return CObject::IsVisible();
}

void mu::ui::window::CMainFrameWindow::RenderCenterRegion()
{
    // HP/MP/AG/SD bars moved to RmlUi; the skill row/current-skill icon stays legacy.
    g_pSkillList->RenderCurrentSkillAndHotSkillList();
}

// Theme-aware background fill behind the still-legacy 3D-composited item/skill icons. RmlUi's main
// context always renders after the 3D-composited icons, so a background drawn through it would
// cover them instead of sitting behind them -- painted instead by CManager::Render()'s centralized
// RenderBackgroundLayer() call (before any window's Render()/Render3D() this frame). General
// mechanism: any other window sharing C3DRenderMng can use it too.
//
// main_frame_bg.rcss's colors match main_frame.rcss's .slot-fill/.slot-frame tokens exactly so the
// RmlUi-drawn panel and the RmlUi-drawn gauges/buttons on top of it read as one surface.
void mu::ui::window::CMainFrameWindow::RenderLeftFrame()
{
    if (UI::RmlBridge::ThemeProvidesOwnIconChrome())
    {
        // #bg_left is already painted by this point -- see the function comment above.
        return;
    }

    RenderImageStretch(IMAGE_MENU_1, 0.0f, kHudTop, kLeftBandWidth, kHudContentHeight,
                       0.0f, 0.0f, kLeftBandWidth, kHudContentHeight);
}

void mu::ui::window::CMainFrameWindow::RenderCenterFrame()
{
    if (UI::RmlBridge::ThemeProvidesOwnIconChrome())
    {
        // Panel spans 214-424 (skill icons' 222-416 footprint padded 8px each side, matching
        // RenderLeftFrame()'s potion padding) so the two chrome panels meet flush with no gap.
        // The fill itself lives in main_frame_bg.rml's #bg_center, painted by CManager::Render()'s
        // centralized RenderBackgroundLayer() call (see RenderLeftFrame()'s comment).
        //
        // This highlight overlay stays a legacy quad (dynamic, frame-conditional, not worth
        // porting) -- modern equivalent of the legacy IMAGE_MENU_2_1 highlight below.
        if (g_pSkillList->IsSkillListUp())
            RenderColorQuadARGB(222.0f, kHudTop, 160.0f, 40.0f, 0x40FFFFFFu);
        return;
    }

    RenderImageStretch(IMAGE_MENU_1, kCenterBandStart, kHudTop, kMenu1CenterWidth, kHudContentHeight,
                       kCenterBandStart, 0.0f, kMenu1CenterWidth, kHudContentHeight);
    RenderImageStretch(IMAGE_MENU_2, kMenu2Start, kHudTop, kMenu2Width, kHudContentHeight,
                       0.0f, 0.0f, kMenu2Width, kHudContentHeight);
    RenderImageStretch(IMAGE_MENU_3, kMenu3Start, kHudTop, kMenu3CenterWidth, kHudContentHeight,
                       0.0f, 0.0f, kMenu3CenterWidth, kHudContentHeight);

    if (g_pSkillList->IsSkillListUp())
        RenderImage(IMAGE_MENU_2_1, 222.0f, kHudTop, 160.0f, 40.0f);
}

// RenderRightFrame()/RenderExperienceBackground()/RenderLifeMana()/RenderGuageAG()/RenderGuageSD()/
// RenderExperience() removed -- moved to RmlUi (see SyncRmlModel()).

// RenderButtons()/RenderCharInfoButton()/RenderFriendButton()/RenderFriendButtonState()/BtnProcess()
// removed -- the 5 corner buttons moved to RmlUi (see Create()).
bool mu::ui::window::CMainFrameWindow::UpdateMouseEvent()
{
    // RmlUi's own context does hit-testing now; never consumes the legacy mouse event.
    return true;
}

bool mu::ui::window::CMainFrameWindow::UpdateKeyEvent()
{
    if (m_ItemHotKey.UpdateKeyEvent() == false)
    {
        return false;
    }
    return true;
}

bool mu::ui::window::CMainFrameWindow::Update()
{
    if (m_bExpEffect == true)
    {
        if (timeGetTime() - m_dwExpEffectTime > 2000)
        {
            m_bExpEffect = false;
            m_dwExpEffectTime = 0;
            m_dwGetExp = 0;
        }
    }

    // Button clicks, polled-and-cleared like other windows' RmlClickX() pattern. Logic ported
    // verbatim from the legacy BtnProcess(), minus the CButton hit-test wrapper.
    if (m_bRmlMyInvenClicked)
    {
        m_bRmlMyInvenClicked = false;
        g_pNewUISystem->Toggle(mu::ui::window::INTERFACE_INVENTORY);
        PlayBuffer(SOUND_CLICK01);
    }
    if (m_bRmlChaInfoClicked)
    {
        m_bRmlChaInfoClicked = false;
        g_pNewUISystem->Toggle(mu::ui::window::INTERFACE_CHARACTER);
        PlayBuffer(SOUND_CLICK01);
        if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_CHARACTER))
            g_QuestMng.SendQuestIndexByEtcSelection();
    }
    if (m_bRmlFriendClicked)
    {
        m_bRmlFriendClicked = false;
        if (gMapManager.InChaosCastle() == true)
        {
            PlayBuffer(SOUND_CLICK01);
        }
        else
        {
            int iLevel = CharacterAttribute->Level;
            if (iLevel < 6)
            {
                if (g_pSystemLogBox->CheckChatRedundancy(I18N::Game::YouMustBeAtLeastLevel6ToUseTheMyFriendFunction) == FALSE)
                {
                    g_pSystemLogBox->AddText(I18N::Game::YouMustBeAtLeastLevel6ToUseTheMyFriendFunction, mu::ui::window::TYPE_SYSTEM_MESSAGE);
                }
            }
            else
            {
                g_pNewUISystem->Toggle(mu::ui::window::INTERFACE_FRIEND);
            }
            PlayBuffer(SOUND_CLICK01);
        }
    }
    if (m_bRmlWindowClicked)
    {
        m_bRmlWindowClicked = false;
        g_pNewUISystem->Toggle(mu::ui::window::INTERFACE_WINDOW_MENU);
        PlayBuffer(SOUND_CLICK01);
    }
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
    if (m_bRmlCShopClicked)
    {
        m_bRmlCShopClicked = false;
        if (g_pInGameShop->IsInGameShopOpen() == false)
        {
            // Nothing to do until the server confirms the shop is open (matches legacy BtnProcess()).
        }
        else
        {
#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
            if (g_InGameShopSystem->IsScriptDownload() == true)
            {
                g_InGameShopSystem->ScriptDownload();
            }
            if (g_InGameShopSystem->IsBannerDownload() == true)
            {
                g_InGameShopSystem->BannerDownload();
            }
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD
            if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_INGAMESHOP) == false)
            {
                if (g_InGameShopSystem->GetIsRequestShopOpenning() == false)
                {
                    SocketClient->ToGameServer()->SendCashShopOpenState(0);
                    g_InGameShopSystem->SetIsRequestShopOpenning(true);
#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
                    SetBtnState(MAINFRAME_BTN_PARTCHARGE, true);
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD
                }
            }
            else
            {
                SocketClient->ToGameServer()->SendCashShopOpenState(1);
                g_pNewUISystem->Hide(mu::ui::window::INTERFACE_INGAMESHOP);
            }
        }
    }
#endif //defined PBG_ADD_INGAMESHOP_UI_MAINFRAME

    SyncRmlModel();

    return true;
}

void mu::ui::window::CMainFrameWindow::SyncRmlModel()
{
    if (!m_pRmlDoc) return;

    auto& model = m_RmlBinder.GetModel();

    // --- HP/MP/AG/SD/EXP: fraction/text/tooltip helper, mirrors CMuHelperBar's syncLabel() ---
    auto syncFloat = [this](float MainFrameRmlModel::* field, const char* boundName, float value)
    {
        if (m_RmlBinder.GetModel().*field != value)
        {
            m_RmlBinder.GetModel().*field = value;
            m_RmlBinder.MarkDirty(boundName);
        }
    };
    auto syncBool = [this](bool MainFrameRmlModel::* field, const char* boundName, bool value)
    {
        if (m_RmlBinder.GetModel().*field != value)
        {
            m_RmlBinder.GetModel().*field = value;
            m_RmlBinder.MarkDirty(boundName);
        }
    };
    auto syncText = [this](Rml::String MainFrameRmlModel::* field, const char* boundName, const Rml::String& value)
    {
        if (m_RmlBinder.GetModel().*field != value)
        {
            m_RmlBinder.GetModel().*field = value;
            m_RmlBinder.MarkDirty(boundName);
        }
    };
    auto syncWide = [&](Rml::String MainFrameRmlModel::* field, const char* boundName, const wchar_t* text)
    {
        syncText(field, boundName, StringUtils::WideToNarrow(text));
    };

    // Shared #bars/#buttons/#exp transform, tracking the legacy center-band chrome's scale (see
    // MainFrameRmlModel::barsLeft). Maps static reference-pixel coordinates onto real window
    // pixels: screenPos = refPos * scale + offset.
    {
        const auto centerTransform = UI::Scaling::BottomHudCenterTransform(WindowWidth, WindowHeight);
        syncFloat(&MainFrameRmlModel::barsLeft, "bars_left", centerTransform.offsetX);
        syncFloat(&MainFrameRmlModel::barsTop, "bars_top", centerTransform.offsetY);
        syncFloat(&MainFrameRmlModel::barsScale, "bars_scale", centerTransform.scaleX);

        // Item-hotkey/skill-hotkey band offsets, read from #item_hotkey_anchor/#skill_list_anchor's
        // real screen position and turned into a delta from centerTransform's offsetX; Render3D()
        // and CSkillList apply this to keep render and hit-testing in sync. One frame of lag is
        // possible (harmless -- these markers only move on theme change).
        if (Rml::Element* pAnchor = m_pRmlDoc->GetElementById("item_hotkey_anchor"))
            m_fItemHotkeyOffsetX = pAnchor->GetAbsoluteOffset().x - centerTransform.offsetX;
        if (Rml::Element* pAnchor = m_pRmlDoc->GetElementById("skill_list_anchor"))
            m_fSkillListOffsetX = pAnchor->GetAbsoluteOffset().x - centerTransform.offsetX;

        // Background-layer panel tracks the same bars_left/top/scale plus the two anchor deltas,
        // so it matches the legacy chrome the Render3D() icons composite against.
        if (m_pRmlBgDoc)
        {
            auto& bg = m_BgRmlBinder.GetModel();
            bg.rootX = centerTransform.offsetX;
            bg.rootY = centerTransform.offsetY;
            bg.rootScale = centerTransform.scaleX;
            bg.leftOffsetX = m_fItemHotkeyOffsetX;
            bg.centerOffsetX = m_fSkillListOffsetX;
            m_BgRmlBinder.MarkDirty("root_x");
            m_BgRmlBinder.MarkDirty("root_y");
            m_BgRmlBinder.MarkDirty("root_scale");
            m_BgRmlBinder.MarkDirty("left_offset_x");
            m_BgRmlBinder.MarkDirty("center_offset_x");
        }
    }

    // HP/MP -- legacy RenderLifeMana(). fLife/fMana there are the EMPTY fraction; store filled.
    DWORD wLifeMax, wLife, wManaMax, wMana;
    if (gCharacterManager.IsMasterLevel(Hero->Class) == true)
    {
        wLifeMax = Master_Level_Data.wMaxLife;
        wLife = std::min<int>(std::max<int>(0, CharacterAttribute->Life), wLifeMax);
        wManaMax = Master_Level_Data.wMaxMana;
        wMana = std::min<int>(std::max<int>(0, CharacterAttribute->Mana), wManaMax);
    }
    else
    {
        wLifeMax = CharacterAttribute->LifeMax;
        wLife = std::min<int>(std::max<int>(0, CharacterAttribute->Life), wLifeMax);
        wManaMax = CharacterAttribute->ManaMax;
        wMana = std::min<int>(std::max<int>(0, CharacterAttribute->Mana), wManaMax);
    }
    if (wLifeMax > 0 && wLife > 0 && (wLife / (float)wLifeMax) < 0.2f)
        PlayBuffer(SOUND_HEART);

    syncFloat(&MainFrameRmlModel::hpFraction, "hp_fraction", wLifeMax > 0 ? wLife / (float)wLifeMax : 0.f);
    syncFloat(&MainFrameRmlModel::mpFraction, "mp_fraction", wManaMax > 0 ? wMana / (float)wManaMax : 0.f);
    syncBool(&MainFrameRmlModel::poisoned, "poisoned", g_isCharacterBuff((&Hero->Object), eDeBuff_Poison));

    // "X / Y" (current/max), not just "X" -- shared model field, both themes.
    wchar_t szNum[32] = {};
    mu_swprintf(szNum, L"%d / %d", wLife, wLifeMax);
    syncWide(&MainFrameRmlModel::hpText, "hp_text", szNum);
    mu_swprintf(szNum, L"%d / %d", wMana, wManaMax);
    syncWide(&MainFrameRmlModel::mpText, "mp_text", szNum);
    mu_swprintf(szNum, L"%d", wLife);
    syncWide(&MainFrameRmlModel::hpCurrentText, "hp_current_text", szNum);
    mu_swprintf(szNum, L"%d", wMana);
    syncWide(&MainFrameRmlModel::mpCurrentText, "mp_current_text", szNum);

    wchar_t szTip[256] = {};
    mu_swprintf(szTip, I18N::Game::LifeDD, wLife, wLifeMax);
    syncWide(&MainFrameRmlModel::hpTooltip, "hp_tooltip", szTip);
    mu_swprintf(szTip, I18N::Game::ManaDD359, wMana, wManaMax);
    syncWide(&MainFrameRmlModel::mpTooltip, "mp_tooltip", szTip);

    // AG (stamina/skill-mana) -- legacy RenderGuageAG().
    DWORD dwMaxSkillMana, dwSkillMana;
    if (gCharacterManager.IsMasterLevel(Hero->Class) == true)
    {
        dwMaxSkillMana = std::max<int>(1, Master_Level_Data.wMaxBP);
        dwSkillMana = std::min<int>(dwMaxSkillMana, CharacterAttribute->SkillMana);
    }
    else
    {
        dwMaxSkillMana = std::max<int>(1, CharacterAttribute->SkillManaMax);
        dwSkillMana = std::min<int>(dwMaxSkillMana, CharacterAttribute->SkillMana);
    }
    syncFloat(&MainFrameRmlModel::agFraction, "ag_fraction", dwSkillMana / (float)dwMaxSkillMana);
    mu_swprintf(szNum, L"%d / %d", dwSkillMana, dwMaxSkillMana);
    syncWide(&MainFrameRmlModel::agText, "ag_text", szNum);
    mu_swprintf(szNum, L"%d", dwSkillMana);
    syncWide(&MainFrameRmlModel::agCurrentText, "ag_current_text", szNum);
    mu_swprintf(szTip, I18N::Game::AGDD, dwSkillMana, dwMaxSkillMana);
    syncWide(&MainFrameRmlModel::agTooltip, "ag_tooltip", szTip);

    // SD (shield) -- legacy RenderGuageSD().
    DWORD wMaxShield, wShield;
    if (gCharacterManager.IsMasterLevel(Hero->Class) == true)
    {
        wMaxShield = std::max<int>(1, Master_Level_Data.wMaxShield);
        wShield = std::min<int>(wMaxShield, CharacterAttribute->Shield);
    }
    else
    {
        wMaxShield = std::max<int>(1, CharacterAttribute->ShieldMax);
        wShield = std::min<int>(wMaxShield, CharacterAttribute->Shield);
    }
    syncFloat(&MainFrameRmlModel::sdFraction, "sd_fraction", wShield / (float)wMaxShield);
    mu_swprintf(szNum, L"%d / %d", wShield, wMaxShield);
    syncWide(&MainFrameRmlModel::sdText, "sd_text", szNum);
    mu_swprintf(szNum, L"%d", wShield);
    syncWide(&MainFrameRmlModel::sdCurrentText, "sd_current_text", szNum);
    mu_swprintf(szTip, I18N::Game::SDDD, wShield, wMaxShield);
    syncWide(&MainFrameRmlModel::sdTooltip, "sd_tooltip", szTip);

    // EXP -- legacy RenderExperience(), flash-highlight overlay intentionally not reproduced (see
    // this class's header comment). expFraction is progress *within* the current decile, matching
    // buildExpSegment()'s original digit/fraction split.
    {
        __int64 wLevel, dwNexExperience, dwExperience;
        const bool masterActive = gCharacterManager.IsMasterExperienceActive(CharacterAttribute->Class, CharacterAttribute->Level);
        if (masterActive)
        {
            wLevel = (__int64)Master_Level_Data.nMLevel;
            dwNexExperience = (__int64)Master_Level_Data.lNext_MasterLevel_Experince;
            dwExperience = (__int64)Master_Level_Data.lMasterLevel_Experince;
        }
        else
        {
            wLevel = CharacterAttribute->Level;
            dwNexExperience = CharacterAttribute->NextExperience;
            dwExperience = CharacterAttribute->Experience;
        }

        __int64 lowerBound;
        if (masterActive)
        {
            const __int64 iTotalLevel = wLevel + 400;
            const __int64 iTOverLevel = iTotalLevel - 255;
            const __int64 iData_Master =
                (((__int64)9 + iTotalLevel) * iTotalLevel * iTotalLevel * (__int64)10)
                + (((__int64)9 + iTOverLevel) * iTOverLevel * iTOverLevel * (__int64)1000);
            lowerBound = (iData_Master - (__int64)3892250000) / (__int64)2;
        }
        else
        {
            const __int64 iPriorLevel = wLevel - 1;
            __int64 iPriorExperience = 0;
            if (iPriorLevel > 0)
            {
                iPriorExperience = (9 + iPriorLevel) * iPriorLevel * iPriorLevel * 10;
                if (iPriorLevel > 255)
                {
                    const __int64 iLevelOverN = iPriorLevel - 255;
                    iPriorExperience += (9 + iLevelOverN) * iLevelOverN * iLevelOverN * 1000;
                }
            }
            lowerBound = iPriorExperience;
        }

        __int64 upperBound = dwNexExperience;
        if (upperBound < lowerBound)
            upperBound = lowerBound;

        const double fNeedExp = static_cast<double>(upperBound - lowerBound);
        const double fClampedExp = std::clamp(static_cast<double>(dwExperience), static_cast<double>(lowerBound), static_cast<double>(upperBound));
        const double fRatio = (fNeedExp > 0.0) ? std::clamp((fClampedExp - static_cast<double>(lowerBound)) / fNeedExp, 0.0, 1.0) : 0.0;

        const double scaled = std::clamp(fRatio, 0.0, 1.0) * 10.0;
        int iExp = std::clamp(static_cast<int>(scaled), 0, 9);
        double fProgress = std::clamp(scaled - static_cast<double>(static_cast<long long>(scaled)), 0.0, 1.0);
        if (fRatio >= 1.0) { iExp = 9; fProgress = 1.0; }

        syncFloat(&MainFrameRmlModel::expFraction, "exp_fraction", static_cast<float>(fProgress));
        wchar_t szExp[8] = {};
        mu_swprintf(szExp, L"%d", iExp);
        syncWide(&MainFrameRmlModel::expDigit, "exp_digit", szExp);
        mu_swprintf(szTip, I18N::Game::EXPI64dI64d, dwExperience, dwNexExperience);
        syncWide(&MainFrameRmlModel::expTooltip, "exp_tooltip", szTip);
    }

    // Button tooltips: static strings, re-checked every frame for consistency but effectively set-once.
    syncWide(&MainFrameRmlModel::chaInfoTooltip, "chainfo_tooltip", I18N::Game::CharacterC);
    syncWide(&MainFrameRmlModel::myInvenTooltip, "myinven_tooltip", I18N::Game::InventoryIV);
    syncWide(&MainFrameRmlModel::friendTooltip, "friend_tooltip", I18N::Game::FriendF);
    syncWide(&MainFrameRmlModel::windowTooltip, "window_tooltip", I18N::Game::MenuU);
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
    syncWide(&MainFrameRmlModel::cShopTooltip, "cshop_tooltip", I18N::Game::MUItemShopX);
#endif //defined PBG_ADD_INGAMESHOP_UI_MAINFRAME

    // Char-info quest-available blink -- legacy RenderCharInfoButton().
    bool chaInfoAlert = false;
    if (!g_QuestMng.IsQuestIndexByEtcListEmpty())
    {
        if (g_Time.GetTimeCheck(5, 500))
            m_bButtonBlink = !m_bButtonBlink;
        chaInfoAlert = m_bButtonBlink
            && !(g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_QUEST_PROGRESS_ETC)
                || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_CHARACTER));
    }
    syncBool(&MainFrameRmlModel::chaInfoAlert, "chainfo_alert", chaInfoAlert);

    // Friend mail/chat blink -- legacy RenderFriendButton()/RenderFriendButtonState().
    const int iBlinkTemp = g_pFriendMenu->GetBlinkTemp();
    const bool bIsAlertTime = (iBlinkTemp % 24 < 12);
    bool friendAlert = false;
    if (g_pFriendMenu->IsNewChatAlert() && bIsAlertTime)
    {
        friendAlert = true;
    }
    if (g_pFriendMenu->IsNewMailAlert())
    {
        if (bIsAlertTime)
        {
            friendAlert = true;
            if (iBlinkTemp % 24 == 11)
                g_pFriendMenu->IncreaseLetterBlink();
        }
    }
    else if (g_pLetterList->CheckNoReadLetter())
    {
        friendAlert = true;
    }
    g_pFriendMenu->IncreaseBlinkTemp();
    syncBool(&MainFrameRmlModel::friendAlert, "friend_alert", friendAlert);

    // Skill-hotkey row selection highlight (modern theme only; see MainFrameRmlModel::
    // skillSlot0Selected and CSkillList::IsHotKeySlotCurrentSkill()).
    syncBool(&MainFrameRmlModel::skillSlot0Selected, "skill_slot_0_selected", g_pSkillList->IsHotKeySlotCurrentSkill(0));
    syncBool(&MainFrameRmlModel::skillSlot1Selected, "skill_slot_1_selected", g_pSkillList->IsHotKeySlotCurrentSkill(1));
    syncBool(&MainFrameRmlModel::skillSlot2Selected, "skill_slot_2_selected", g_pSkillList->IsHotKeySlotCurrentSkill(2));
    syncBool(&MainFrameRmlModel::skillSlot3Selected, "skill_slot_3_selected", g_pSkillList->IsHotKeySlotCurrentSkill(3));
    syncBool(&MainFrameRmlModel::skillSlot4Selected, "skill_slot_4_selected", g_pSkillList->IsHotKeySlotCurrentSkill(4));

    // Skill-hotkey number labels (modern theme only; see MainFrameRmlModel::skillSlot0Hotkey and
    // CSkillList::GetHotKeySlotNumber()). -1 (empty slot) becomes an empty string.
    auto hotkeyText = [](int hotkey) { return hotkey >= 0 ? std::to_string(hotkey) : Rml::String(); };
    syncText(&MainFrameRmlModel::skillSlot0Hotkey, "skill_slot_0_hotkey", hotkeyText(g_pSkillList->GetHotKeySlotNumber(0)));
    syncText(&MainFrameRmlModel::skillSlot1Hotkey, "skill_slot_1_hotkey", hotkeyText(g_pSkillList->GetHotKeySlotNumber(1)));
    syncText(&MainFrameRmlModel::skillSlot2Hotkey, "skill_slot_2_hotkey", hotkeyText(g_pSkillList->GetHotKeySlotNumber(2)));
    syncText(&MainFrameRmlModel::skillSlot3Hotkey, "skill_slot_3_hotkey", hotkeyText(g_pSkillList->GetHotKeySlotNumber(3)));
    syncText(&MainFrameRmlModel::skillSlot4Hotkey, "skill_slot_4_hotkey", hotkeyText(g_pSkillList->GetHotKeySlotNumber(4)));

    // Skill list cooldown fractions.
    syncFloat(&MainFrameRmlModel::skillSlot0Cooldown, "skill_slot_0_cooldown", g_pSkillList->GetHotKeySlotCooldownFraction(0));
    syncFloat(&MainFrameRmlModel::skillSlot1Cooldown, "skill_slot_1_cooldown", g_pSkillList->GetHotKeySlotCooldownFraction(1));
    syncFloat(&MainFrameRmlModel::skillSlot2Cooldown, "skill_slot_2_cooldown", g_pSkillList->GetHotKeySlotCooldownFraction(2));
    syncFloat(&MainFrameRmlModel::skillSlot3Cooldown, "skill_slot_3_cooldown", g_pSkillList->GetHotKeySlotCooldownFraction(3));
    syncFloat(&MainFrameRmlModel::skillSlot4Cooldown, "skill_slot_4_cooldown", g_pSkillList->GetHotKeySlotCooldownFraction(4));

    // Item hotkey chrome (#item_slots hover border + stack-count). m_ItemHotKey is a member, read directly.
    auto stackCountText = [](int count) { return count > 0 ? std::to_string(count) : Rml::String(); };
    syncBool(&MainFrameRmlModel::itemSlot0Hovered, "item_slot_0_hovered", m_ItemHotKey.GetHoveredSlot() == 0);
    syncBool(&MainFrameRmlModel::itemSlot1Hovered, "item_slot_1_hovered", m_ItemHotKey.GetHoveredSlot() == 1);
    syncBool(&MainFrameRmlModel::itemSlot2Hovered, "item_slot_2_hovered", m_ItemHotKey.GetHoveredSlot() == 2);
    syncBool(&MainFrameRmlModel::itemSlot3Hovered, "item_slot_3_hovered", m_ItemHotKey.GetHoveredSlot() == 3);
    syncText(&MainFrameRmlModel::itemSlot0Count, "item_slot_0_count", stackCountText(m_ItemHotKey.GetSlotItemCount(0)));
    syncText(&MainFrameRmlModel::itemSlot1Count, "item_slot_1_count", stackCountText(m_ItemHotKey.GetSlotItemCount(1)));
    syncText(&MainFrameRmlModel::itemSlot2Count, "item_slot_2_count", stackCountText(m_ItemHotKey.GetSlotItemCount(2)));
    syncText(&MainFrameRmlModel::itemSlot3Count, "item_slot_3_count", stackCountText(m_ItemHotKey.GetSlotItemCount(3)));
    syncFloat(&MainFrameRmlModel::currentSkillCooldown, "current_skill_cooldown", g_pSkillList->GetCurrentSkillCooldownFraction());

    syncBool(&MainFrameRmlModel::skillGridOpen, "skill_grid_open", g_pSkillList->IsSkillGridOpen());

    // Dynamic-count lists, copied unconditionally every frame while the grid is open (cooldown
    // fractions change every frame anyway, so a change-check would rarely help).
    if (model.skillGridOpen)
    {
        model.skillGridCells = g_pSkillList->GetGridSnapshot();
        model.petSkillCells = g_pSkillList->GetPetSnapshot();
        m_RmlBinder.MarkDirty("skill_grid_cells");
        m_RmlBinder.MarkDirty("pet_skill_cells");
    }

    // Shared skill tooltip: one hover target queued at a time (QueueTooltip()/OnUnhover()).
    // BuildModelForSlot() is the same content resolution SkillTooltip.cpp's Render() uses; only
    // the destination (RmlUi vs. legacy TextList) differs.
    if (g_pSkillList->IsTooltipPending())
    {
        UI::Skills::Tooltip::Model tooltipModel;
        if (UI::Skills::Tooltip::BuildModelForSlot(g_pSkillList->GetTooltipSkillIndex(), tooltipModel))
        {
            model.skillTooltipLines.clear();
            for (int i = 0; i < tooltipModel.count; ++i)
            {
                const UI::Skills::Tooltip::Line& src = tooltipModel.lines[i];
                SkillTooltipLineEntry line;
                line.text = StringUtils::WideToNarrow(src.text);
                line.colorBlue = (src.color == UI::Skills::Tooltip::LineColor::Blue);
                line.colorRed = (src.color == UI::Skills::Tooltip::LineColor::Red);
                line.colorDarkRed = (src.color == UI::Skills::Tooltip::LineColor::DarkRed);
                line.bold = src.isBold;
                model.skillTooltipLines.push_back(line);
            }
            model.skillTooltipLeft = g_pSkillList->GetTooltipAnchorX();
            model.skillTooltipTop = g_pSkillList->GetTooltipAnchorY();
            model.skillTooltipVisible = true;
            m_RmlBinder.MarkDirty("skill_tooltip_lines");
            m_RmlBinder.MarkDirty("skill_tooltip_left");
            m_RmlBinder.MarkDirty("skill_tooltip_top");
            m_RmlBinder.MarkDirty("skill_tooltip_visible");
        }
        else
        {
            syncBool(&MainFrameRmlModel::skillTooltipVisible, "skill_tooltip_visible", false);
        }
    }
    else
    {
        syncBool(&MainFrameRmlModel::skillTooltipVisible, "skill_tooltip_visible", false);
    }
}

float mu::ui::window::CMainFrameWindow::GetLayerDepth()
{
    return 10.6f;
}

float mu::ui::window::CMainFrameWindow::GetKeyEventOrder()
{
    return 2.9f;
}

void mu::ui::window::CMainFrameWindow::SetItemHotKey(int iHotKey, int iItemType, int iItemLevel)
{
    m_ItemHotKey.SetHotKey(iHotKey, iItemType, iItemLevel);
}

int mu::ui::window::CMainFrameWindow::GetItemHotKey(int iHotKey)
{
    return m_ItemHotKey.GetHotKey(iHotKey);
}

int mu::ui::window::CMainFrameWindow::GetItemHotKeyLevel(int iHotKey)
{
    return m_ItemHotKey.GetHotKeyLevel(iHotKey);
}

void mu::ui::window::CMainFrameWindow::UpdateItemHotKey()
{
    m_ItemHotKey.UpdateKeyEvent();
}

void mu::ui::window::CMainFrameWindow::ResetSkillHotKey()
{
    g_pSkillList->Reset();
}

void mu::ui::window::CMainFrameWindow::SetSkillHotKey(int iHotKey, int iSkillType)
{
    g_pSkillList->SetHotKey(iHotKey, iSkillType);
}

int mu::ui::window::CMainFrameWindow::GetSkillHotKey(int iHotKey)
{
    return g_pSkillList->GetHotKey(iHotKey);
}

int mu::ui::window::CMainFrameWindow::GetSkillHotKeyIndex(int iSkillType)
{
    return g_pSkillList->GetSkillIndex(iSkillType);
}

mu::ui::window::CItemHotKey::CItemHotKey()
{
    for (int i = 0; i < HOTKEY_COUNT; ++i)
    {
        m_iHotKeyItemType[i] = -1;
        m_iHotKeyItemLevel[i] = 0;
    }
}

mu::ui::window::CItemHotKey::~CItemHotKey()
{
}

bool mu::ui::window::CItemHotKey::UpdateKeyEvent()
{
    int iIndex = -1;

    if (mu::ui::window::IsPress('Q') == true)
    {
        iIndex = GetHotKeyItemIndex(HOTKEY_Q);
    }
    else if (mu::ui::window::IsPress('W') == true)
    {
        iIndex = GetHotKeyItemIndex(HOTKEY_W);
    }
    else if (mu::ui::window::IsPress('E') == true)
    {
        iIndex = GetHotKeyItemIndex(HOTKEY_E);
    }
    else if (mu::ui::window::IsPress('R') == true)
    {
        iIndex = GetHotKeyItemIndex(HOTKEY_R);
    }

    if (iIndex != -1)
    {
        ITEM* pItem = NULL;
        pItem = g_pMyInventory->FindItem(iIndex);
        if ((pItem->Type >= ITEM_POTION + 78 && pItem->Type <= ITEM_POTION + 82))
        {
            std::list<eBuffState> secretPotionbufflist;
            secretPotionbufflist.push_back(eBuff_SecretPotion1);
            secretPotionbufflist.push_back(eBuff_SecretPotion2);
            secretPotionbufflist.push_back(eBuff_SecretPotion3);
            secretPotionbufflist.push_back(eBuff_SecretPotion4);
            secretPotionbufflist.push_back(eBuff_SecretPotion5);

            if (g_isCharacterBufflist((&Hero->Object), secretPotionbufflist) != eBuffNone) {
                mu::ui::window::CreateOkMessageBox(I18N::Game::YouCannotUseThisItemWhileThePotionEffectsRemainActive, RGBA(255, 30, 0, 255));
            }
            else {
                SendRequestUse(iIndex, 0);
            }
        }
        else

        {
            SendRequestUse(iIndex, 0);
        }
        return false;
    }

    return true;
}

int mu::ui::window::CItemHotKey::GetHotKeyItemIndex(int iType, bool bItemCount)
{
    int iStartItemType = 0, iEndItemType = 0;
    int i, j;

    switch (iType)
    {
    case HOTKEY_Q:
        if (GetHotKeyCommonItem(iType, iStartItemType, iEndItemType) == false)
        {
            if (m_iHotKeyItemType[iType] >= ITEM_SMALL_MANA_POTION && m_iHotKeyItemType[iType] <= ITEM_LARGE_MANA_POTION)
            {
                iStartItemType = ITEM_LARGE_MANA_POTION; iEndItemType = ITEM_SMALL_MANA_POTION;
            }
            else
            {
                iStartItemType = ITEM_LARGE_HEALING_POTION; iEndItemType = ITEM_APPLE;
            }
        }
        break;
    case HOTKEY_W:
        if (GetHotKeyCommonItem(iType, iStartItemType, iEndItemType) == false)
        {
            if (m_iHotKeyItemType[iType] >= ITEM_APPLE && m_iHotKeyItemType[iType] <= ITEM_LARGE_HEALING_POTION)
            {
                iStartItemType = ITEM_LARGE_HEALING_POTION; iEndItemType = ITEM_APPLE;
            }
            else
            {
                iStartItemType = ITEM_LARGE_MANA_POTION; iEndItemType = ITEM_SMALL_MANA_POTION;
            }
        }
        break;
    case HOTKEY_E:
        if (GetHotKeyCommonItem(iType, iStartItemType, iEndItemType) == false)
        {
            if (m_iHotKeyItemType[iType] >= ITEM_APPLE && m_iHotKeyItemType[iType] <= ITEM_LARGE_HEALING_POTION)
            {
                iStartItemType = ITEM_LARGE_HEALING_POTION; iEndItemType = ITEM_APPLE;
            }
            else if (m_iHotKeyItemType[iType] >= ITEM_SMALL_MANA_POTION && m_iHotKeyItemType[iType] <= ITEM_LARGE_MANA_POTION)
            {
                iStartItemType = ITEM_LARGE_MANA_POTION; iEndItemType = ITEM_SMALL_MANA_POTION;
            }
            else
            {
                iStartItemType = ITEM_ANTIDOTE; iEndItemType = ITEM_ANTIDOTE;
            }
        }
        break;
    case HOTKEY_R:
        if (GetHotKeyCommonItem(iType, iStartItemType, iEndItemType) == false)
        {
            if (m_iHotKeyItemType[iType] >= ITEM_APPLE && m_iHotKeyItemType[iType] <= ITEM_LARGE_HEALING_POTION)
            {
                iStartItemType = ITEM_LARGE_HEALING_POTION; iEndItemType = ITEM_APPLE;
            }
            else if (m_iHotKeyItemType[iType] >= ITEM_SMALL_MANA_POTION && m_iHotKeyItemType[iType] <= ITEM_LARGE_MANA_POTION)
            {
                iStartItemType = ITEM_LARGE_MANA_POTION; iEndItemType = ITEM_SMALL_MANA_POTION;
            }
            else
            {
                iStartItemType = ITEM_LARGE_SHIELD_POTION; iEndItemType = ITEM_SMALL_SHIELD_POTION;
            }
        }
        break;
    }

    int iItemCount = 0;
    ITEM* pItem = NULL;

    int iNumberofItems = g_pMyInventory->GetInventoryCtrl()->GetNumberOfItems();
    for (i = iStartItemType; i >= iEndItemType; --i)
    {
        if (bItemCount)
        {
            for (j = 0; j < iNumberofItems; ++j)
            {
                pItem = g_pMyInventory->GetInventoryCtrl()->GetItem(j);
                if (pItem == NULL)
                {
                    continue;
                }

                if (
                    (pItem->Type == i && pItem->Level == m_iHotKeyItemLevel[iType])
                    || (pItem->Type == i && (pItem->Type >= ITEM_APPLE && pItem->Type <= ITEM_LARGE_HEALING_POTION))
                    )
                {
                    if (pItem->Type == ITEM_ALE
                        || pItem->Type == ITEM_TOWN_PORTAL_SCROLL
                        || pItem->Type == ITEM_POTION + 20
                        )
                    {
                        iItemCount++;
                    }
                    else
                    {
                        iItemCount += pItem->Durability;
                    }
                }
            }
        }
        else
        {
            int iIndex = -1;
            if (i >= ITEM_APPLE && i <= ITEM_LARGE_HEALING_POTION)
            {
                iIndex = g_pMyInventory->FindItemReverseIndex(i);
            }
            else
            {
                iIndex = g_pMyInventory->FindItemReverseIndex(i, m_iHotKeyItemLevel[iType]);
            }

            if (-1 != iIndex)
            {
                pItem = g_pMyInventory->FindItem(iIndex);
                if ((pItem->Type != ITEM_SIEGE_POTION
                    && pItem->Type != ITEM_TOWN_PORTAL_SCROLL
                    && pItem->Type != ITEM_POTION + 20)
                    || pItem->Level == m_iHotKeyItemLevel[iType]
                    )
                {
                    return iIndex;
                }
            }
        }
    }

    if (bItemCount == true)
    {
        return iItemCount;
    }

    return -1;
}

bool mu::ui::window::CItemHotKey::GetHotKeyCommonItem(IN int iHotKey, OUT int& iStart, OUT int& iEnd)
{
    switch (m_iHotKeyItemType[iHotKey])
    {
    case ITEM_SIEGE_POTION:
    case ITEM_ANTIDOTE:
    case ITEM_ALE:
    case ITEM_TOWN_PORTAL_SCROLL:
    case ITEM_POTION + 20:
    case ITEM_JACK_OLANTERN_BLESSINGS:
    case ITEM_JACK_OLANTERN_WRATH:
    case ITEM_JACK_OLANTERN_CRY:
    case ITEM_JACK_OLANTERN_FOOD:
    case ITEM_JACK_OLANTERN_DRINK:
    case ITEM_POTION + 70:
    case ITEM_POTION + 71:
    case ITEM_POTION + 78:
    case ITEM_POTION + 79:
    case ITEM_POTION + 80:
    case ITEM_POTION + 81:
    case ITEM_POTION + 82:
    case ITEM_POTION + 94:
    case ITEM_CHERRY_BLOSSOM_WINE:
    case ITEM_CHERRY_BLOSSOM_RICE_CAKE:
    case ITEM_CHERRY_BLOSSOM_FLOWER_PETAL:
    case ITEM_POTION + 133:
        if (m_iHotKeyItemType[iHotKey] != ITEM_POTION + 20 || m_iHotKeyItemLevel[iHotKey] == 0)
        {
            iStart = iEnd = m_iHotKeyItemType[iHotKey];
            return true;
        }
        break;
    default:
        if (m_iHotKeyItemType[iHotKey] >= ITEM_SMALL_SHIELD_POTION && m_iHotKeyItemType[iHotKey] <= ITEM_LARGE_SHIELD_POTION)
        {
            iStart = ITEM_LARGE_SHIELD_POTION; iEnd = ITEM_SMALL_SHIELD_POTION;
            return true;
        }
        else if (m_iHotKeyItemType[iHotKey] >= ITEM_SMALL_COMPLEX_POTION && m_iHotKeyItemType[iHotKey] <= ITEM_LARGE_COMPLEX_POTION)
        {
            iStart = ITEM_LARGE_COMPLEX_POTION; iEnd = ITEM_SMALL_COMPLEX_POTION;
            return true;
        }
        break;
    }
    return false;
}

int mu::ui::window::CItemHotKey::GetHotKeyItemCount(int iType)
{
    return 0;
}

void mu::ui::window::CItemHotKey::SetHotKey(int iHotKey, int iItemType, int iItemLevel)
{
    if (iHotKey != -1 && CMyInventory::CanRegisterItemHotKey(iItemType) == true
        )
    {
        m_iHotKeyItemType[iHotKey] = iItemType;
        m_iHotKeyItemLevel[iHotKey] = iItemLevel;
    }
    else
    {
        m_iHotKeyItemType[iHotKey] = -1;
        m_iHotKeyItemLevel[iHotKey] = 0;
    }
}

int mu::ui::window::CItemHotKey::GetHotKey(int iHotKey)
{
    if (iHotKey != -1)
    {
        return m_iHotKeyItemType[iHotKey];
    }

    return -1;
}

int mu::ui::window::CItemHotKey::GetHotKeyLevel(int iHotKey)
{
    if (iHotKey != -1)
    {
        return m_iHotKeyItemLevel[iHotKey];
    }

    return 0;
}

void mu::ui::window::CItemHotKey::RenderItems()
{
    float x, y, width, height;

    for (int i = 0; i < HOTKEY_COUNT; ++i)
    {
        int iIndex = GetHotKeyItemIndex(i);
        if (iIndex != -1)
        {
            ITEM* pItem = g_pMyInventory->FindItem(iIndex);
            if (pItem)
            {
                x = 10 + (i * 38); y = 443; width = 20; height = 20;
                RenderItem3D(x, y, width, height, pItem->Type, pItem->Level, 0, 0);
            }
        }
    }
}

void mu::ui::window::CItemHotKey::OnHotkeySlotRightClick(int iSlotIndex)
{
    // RmlUi's Context now does hit-testing for these 4 slots (data-event-mouseup). `Hero->Dead !=
    // 0` reproduces the old call site's own alive guard.
    if (Hero->Dead != 0)
    {
        return;
    }
    int iIndex = GetHotKeyItemIndex(iSlotIndex);
    if (iIndex != -1)
    {
        SendRequestUse(iIndex, 0);
    }
}

int mu::ui::window::CItemHotKey::GetSlotItemCount(int iSlotIndex)
{
    return GetHotKeyItemIndex(iSlotIndex, true);
}

mu::ui::window::CSkillList::CSkillList()
{
    m_pNewUIMng = NULL;
    Reset();
}

mu::ui::window::CSkillList::~CSkillList()
{
    Release();
}

bool mu::ui::window::CSkillList::Create(CManager* pNewUIMng, C3DRenderMng* pNewUI3DRenderMng)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_SKILL_LIST, this);

    m_pNewUI3DRenderMng = pNewUI3DRenderMng;

    LoadImages();

    Show(true);

    return true;
}

void mu::ui::window::CSkillList::Release()
{
    // The tooltip is a plain RmlUi element now, so nothing needs unregistering via
    // UI2DEffectObject/DeleteUI2DEffectObject() here.
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void mu::ui::window::CSkillList::Reset()
{
    m_bSkillList = false;
    m_bHotKeySkillListUp = false;

    for (int i = 0; i < SKILLHOTKEY_COUNT; ++i)
    {
        m_iHotKeySkillType[i] = -1;
    }

    m_GridSnapshot.clear();
    m_PetSnapshot.clear();
    m_bTooltipPending = false;
    m_iTooltipSkillIndex = -1;
    m_fTooltipAnchorX = 0.f;
    m_fTooltipAnchorY = 0.f;
    m_iHoveredGridSkillIndex = -1;
}

void mu::ui::window::CSkillList::LoadImages()
{
    LoadBitmap(L"Interface\\newui_skill.jpg", IMAGE_SKILL1, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_skill2.jpg", IMAGE_SKILL2, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_command.jpg", IMAGE_COMMAND, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_skillbox.jpg", IMAGE_SKILLBOX, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_skillbox2.jpg", IMAGE_SKILLBOX_USE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_non_skill.jpg", IMAGE_NON_SKILL1, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_non_skill2.jpg", IMAGE_NON_SKILL2, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_non_command.jpg", IMAGE_NON_COMMAND, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_skill3.jpg", IMAGE_SKILL3, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_non_skill3.jpg", IMAGE_NON_SKILL3, GL_LINEAR);
}

void mu::ui::window::CSkillList::UnloadImages()
{
    DeleteBitmap(IMAGE_SKILL1);
    DeleteBitmap(IMAGE_SKILL2);
    DeleteBitmap(IMAGE_COMMAND);
    DeleteBitmap(IMAGE_SKILLBOX);
    DeleteBitmap(IMAGE_SKILLBOX_USE);
    DeleteBitmap(IMAGE_NON_SKILL1);
    DeleteBitmap(IMAGE_NON_SKILL2);
    DeleteBitmap(IMAGE_NON_COMMAND);
    DeleteBitmap(IMAGE_SKILL3);
    DeleteBitmap(IMAGE_NON_SKILL3);
}

bool mu::ui::window::CSkillList::UpdateMouseEvent()
{
    // RmlUi's own Context now does hit-testing for the current-skill icon, hotkey row, and
    // grid/pet row (see OnHotkeySlotClick()/OnCurrentSkillClick()/OnGridCellClick()/
    // OnPetCellClick() and their *Hover() counterparts).
    return true;
}

bool mu::ui::window::CSkillList::UpdateKeyEvent()
{
    for (int i = 0; i < 9; ++i)
    {
        if (mu::ui::window::IsPress('1' + i))
        {
            UseHotKey(i + 1);
        }
    }

    if (mu::ui::window::IsPress('0'))
    {
        UseHotKey(0);
    }

    // m_iHoveredGridSkillIndex arms Ctrl+digit assignment; set by OnGridCellHover()/OnPetCellHover(), cleared by OnUnhover().
    if (m_iHoveredGridSkillIndex != -1)
    {
        if (mu::ui::window::IsRepeat(VK_CONTROL))
        {
            for (int i = 0; i < 9; ++i)
            {
                if (mu::ui::window::IsPress('1' + i))
                {
                    SetHotKey(i + 1, m_iHoveredGridSkillIndex);

                    return false;
                }
            }

            if (mu::ui::window::IsPress('0'))
            {
                SetHotKey(0, m_iHoveredGridSkillIndex);

                return false;
            }
        }
    }

    if (mu::ui::window::IsRepeat(VK_SHIFT))
    {
        for (int i = 0; i < 4; ++i)
        {
            if (mu::ui::window::IsPress('1' + i))
            {
                Hero->CurrentSkill = AT_PET_COMMAND_DEFAULT + i;
                return false;
            }
        }
    }

    return true;
}

bool mu::ui::window::CSkillList::IsArrayUp(BYTE bySkill)
{
    for (int i = 0; i < SKILLHOTKEY_COUNT; ++i)
    {
        if (m_iHotKeySkillType[i] == bySkill)
        {
            if (i == 0 || i > 5)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }

    return false;
}

bool mu::ui::window::CSkillList::IsArrayIn(BYTE bySkill)
{
    for (int i = 0; i < SKILLHOTKEY_COUNT; ++i)
    {
        if (m_iHotKeySkillType[i] == bySkill)
        {
            return true;
        }
    }

    return false;
}

void mu::ui::window::CSkillList::SetHotKey(int iHotKey, int iSkillType)
{
    for (int i = 0; i < SKILLHOTKEY_COUNT; ++i)
    {
        if (m_iHotKeySkillType[i] == iSkillType)
        {
            m_iHotKeySkillType[i] = -1;
            break;
        }
    }

    m_iHotKeySkillType[iHotKey] = iSkillType;
}

int mu::ui::window::CSkillList::GetHotKey(int iHotKey)
{
    return m_iHotKeySkillType[iHotKey];
}

int mu::ui::window::CSkillList::GetSkillIndex(int iSkillType)
{
    // special handling for skills with different skill id for the trigger
    if (iSkillType == AT_SKILL_NOVA_BEGIN)
    {
        iSkillType = AT_SKILL_NOVA;
    }

    int iReturn = -1;
    for (int i = 0; i < MAX_MAGIC; ++i)
    {
        if (CharacterAttribute->Skill[i] == iSkillType)
        {
            iReturn = i;
            break;
        }
    }

    return iReturn;
}

void mu::ui::window::CSkillList::UseHotKey(int iHotKey)
{
    if (m_iHotKeySkillType[iHotKey] != -1)
    {
        if (m_iHotKeySkillType[iHotKey] >= AT_PET_COMMAND_DEFAULT && m_iHotKeySkillType[iHotKey] < AT_PET_COMMAND_END)
        {
            if (Hero->m_pPet == NULL)
            {
                return;
            }
        }

        auto wHotKeySkill = CharacterAttribute->Skill[m_iHotKeySkillType[iHotKey]];

        if (wHotKeySkill == 0)
        {
            return;
        }

        m_wHeroPriorSkill = CharacterAttribute->Skill[Hero->CurrentSkill];

        Hero->CurrentSkill = m_iHotKeySkillType[iHotKey];

        auto bySkill = CharacterAttribute->Skill[Hero->CurrentSkill];

        if (
            g_pOption->IsAutoAttack() == true
            && gMapManager.WorldActive != WD_6STADIUM
            && gMapManager.InChaosCastle() == false
            && (bySkill == AT_SKILL_TELEPORT || bySkill == AT_SKILL_TELEPORT_ALLY))
        {
            SelectedCharacter = -1;
            Attacking = -1;
        }
    }
}

bool mu::ui::window::CSkillList::Update()
{
    if (IsArrayIn(Hero->CurrentSkill) == true)
    {
        if (IsArrayUp(Hero->CurrentSkill) == true)
        {
            m_bHotKeySkillListUp = true;
        }
        else
        {
            m_bHotKeySkillListUp = false;
        }
    }

    if (Hero->m_pPet == NULL)
    {
        if (Hero->CurrentSkill >= AT_PET_COMMAND_DEFAULT && Hero->CurrentSkill < AT_PET_COMMAND_END)
        {
            Hero->CurrentSkill = 0;
        }
    }

    // Refreshes the RmlUi-facing overlay snapshot while the grid is open; left stale (harmless,
    // hidden) while closed.
    if (m_bSkillList)
    {
        RebuildGridSnapshot();
    }

    return true;
}

void mu::ui::window::CSkillList::RenderCurrentSkillAndHotSkillList()
{
    int i;
    float x, y, width, height;

    BYTE bySkillNumber = CharacterAttribute->SkillNumber;

    if (bySkillNumber > 0)
    {
        int iStartSkillIndex = 1;
        if (m_bHotKeySkillListUp)
        {
            iStartSkillIndex = 6;
        }

        x = 190; y = 431; width = 32; height = 38;
        for (i = 0; i < 5; ++i)
        {
            x += width;

            int iIndex = iStartSkillIndex + i;
            if (iIndex == 10)
            {
                iIndex = 0;
            }

            if (m_iHotKeySkillType[iIndex] == -1)
            {
                continue;
            }

            if (m_iHotKeySkillType[iIndex] >= AT_PET_COMMAND_DEFAULT && m_iHotKeySkillType[iIndex] < AT_PET_COMMAND_END)
            {
                if (Hero->m_pPet == NULL)
                {
                    continue;
                }
            }

            if (Hero->CurrentSkill == m_iHotKeySkillType[iIndex])
            {
                // Suppressed for modern theme -- #skill_slot_0..4's RmlUi highlight (synced from
                // IsHotKeySlotCurrentSkill()) always paints on top, so drawing this sprite too
                // would double up. Legacy theme keeps the sprite (its own established look, not
                // reproducible with a plain CSS outline).
                if (!UI::RmlBridge::ThemeProvidesOwnIconChrome())
                    mu::ui::window::RenderImage(IMAGE_SKILLBOX_USE, x, y, width, height);
            }
            RenderSkillIcon(m_iHotKeySkillType[iIndex], x + 6, y + 6, 20, 28);
        }

        x = 392; y = 437; width = 20; height = 28;
        RenderSkillIcon(Hero->CurrentSkill, x, y, width, height);
    }
}

bool mu::ui::window::CSkillList::IsHotKeySlotCurrentSkill(int iSlotIndex)
{
    // Mirrors RenderCurrentSkillAndHotSkillList()'s loop (wraparound, empty-slot, no-pet checks).
    if (iSlotIndex < 0 || iSlotIndex >= 5)
        return false;

    if (CharacterAttribute->SkillNumber == 0)
        return false;

    int iStartSkillIndex = m_bHotKeySkillListUp ? 6 : 1;
    int iIndex = iStartSkillIndex + iSlotIndex;
    if (iIndex == 10)
        iIndex = 0;

    if (m_iHotKeySkillType[iIndex] == -1)
        return false;

    if (m_iHotKeySkillType[iIndex] >= AT_PET_COMMAND_DEFAULT && m_iHotKeySkillType[iIndex] < AT_PET_COMMAND_END)
    {
        if (Hero->m_pPet == NULL)
            return false;
    }

    return Hero->CurrentSkill == m_iHotKeySkillType[iIndex];
}

int mu::ui::window::CSkillList::GetHotKeySlotNumber(int iSlotIndex)
{
    // Same loop as IsHotKeySlotCurrentSkill(), but doesn't check Hero->CurrentSkill -- the number
    // shows for every occupied slot, not just the active one.
    if (iSlotIndex < 0 || iSlotIndex >= 5)
        return -1;

    if (CharacterAttribute->SkillNumber == 0)
        return -1;

    int iStartSkillIndex = m_bHotKeySkillListUp ? 6 : 1;
    int iIndex = iStartSkillIndex + iSlotIndex;
    if (iIndex == 10)
        iIndex = 0;

    if (m_iHotKeySkillType[iIndex] == -1)
        return -1;

    if (m_iHotKeySkillType[iIndex] >= AT_PET_COMMAND_DEFAULT && m_iHotKeySkillType[iIndex] < AT_PET_COMMAND_END)
    {
        if (Hero->m_pPet == NULL)
            return -1;
    }

    return iIndex;
}

bool mu::ui::window::CSkillList::Render()
{
    BYTE bySkillNumber = CharacterAttribute->SkillNumber;

    // Without this, the expanded grid (registered directly with CManager, generic untransformed
    // Render() dispatch) would render under the baseline transform instead of matching the compact
    // hotkey row's centerTransform, misaligning at non-4:3 resolutions.
    auto transform = UI::Scaling::BottomHudCenterTransform(WindowWidth, WindowHeight);
    transform.offsetX += g_pMainFrame->GetSkillListOffsetX() * transform.scaleX;
    UI::Scaling::ScopedActiveTransform layout(transform, true);

    // Icon art stays legacy 2D (atlas lookup too irregular to port, see SkillCellEntry). The
    // box-frame sprite is suppressed for modern theme (relies on .skill-cell's own border/
    // highlight instead); legacy theme keeps it. This loop only draws from
    // m_GridSnapshot/m_PetSnapshot -- hit-testing/tooltip-queueing moved to RmlUi (OnGridCellClick() etc).
    if (bySkillNumber > 0 && m_bSkillList == true)
    {
        const bool bDrawBoxSprite = !UI::RmlBridge::ThemeProvidesOwnIconChrome();
        for (const SkillCellEntry& entry : m_GridSnapshot)
        {
            if (bDrawBoxSprite)
                mu::ui::window::RenderImage(entry.isCurrent ? IMAGE_SKILLBOX_USE : IMAGE_SKILLBOX, entry.left, entry.top, 32.f, 38.f);
            RenderSkillIcon(entry.skillIndex, entry.left + 6.f, entry.top + 6.f, 20.f, 28.f);
        }
        for (const SkillCellEntry& entry : m_PetSnapshot)
        {
            if (bDrawBoxSprite)
                mu::ui::window::RenderImage(entry.isCurrent ? IMAGE_SKILLBOX_USE : IMAGE_SKILLBOX, entry.left, entry.top, 32.f, 38.f);
            RenderSkillIcon(entry.skillIndex, entry.left + 6.f, entry.top + 6.f, 20.f, 28.f);
        }
    }

    return true;
}

float mu::ui::window::CSkillList::GetLayerDepth()
{
    return 5.2f;
}

WORD mu::ui::window::CSkillList::GetHeroPriorSkill()
{
    return m_wHeroPriorSkill;
}

void mu::ui::window::CSkillList::SetHeroPriorSkill(BYTE bySkill)
{
    m_wHeroPriorSkill = bySkill;
}

void mu::ui::window::CSkillList::RenderSkillIcon(int iIndex, float x, float y, float width, float height)
{
    auto bySkillType = CharacterAttribute->Skill[iIndex];

    if (bySkillType == 0)
    {
        return;
    }

    if (iIndex >= AT_PET_COMMAND_DEFAULT)
    {
        bySkillType = (ActionSkillType)iIndex;
    }

    bool bCantSkill = false;

    BYTE bySkillUseType = SkillAttribute[bySkillType].SkillUseType;
    int Skill_Icon = SkillAttribute[bySkillType].Magic_Icon;

    if (!gSkillManager.AreSkillAttributeRequirementsMet(bySkillType))
    {
        bCantSkill = true;
    }

    if (IsCanBCSkill(bySkillType) == false)
    {
        bCantSkill = true;
    }
    if (g_isCharacterBuff((&Hero->Object), eBuff_AddSkill) && bySkillUseType == SKILL_USE_TYPE_BRAND)
    {
        bCantSkill = true;
    }
    auto isSittingOnPet = (Hero->Helper.Type == MODEL_HORN_OF_UNIRIA || Hero->Helper.Type == MODEL_HORN_OF_DINORANT || Hero->Helper.Type == MODEL_HORN_OF_FENRIR);
    if (bySkillType == AT_SKILL_IMPALE && !isSittingOnPet)
    {
        bCantSkill = true;
    }

    if (bySkillType == AT_SKILL_IMPALE && isSittingOnPet)
    {
        int iTypeL = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;
        int iTypeR = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;
        if ((iTypeL < ITEM_SPEAR || iTypeL >= ITEM_BOW) && (iTypeR < ITEM_SPEAR || iTypeR >= ITEM_BOW))
        {
            bCantSkill = true;
        }
    }

    if (isSittingOnPet
        && ((bySkillType >= AT_SKILL_BLOCKING && bySkillType <= AT_SKILL_SLASH)
            || bySkillType == AT_SKILL_FALLING_SLASH_STR
            || bySkillType == AT_SKILL_LUNGE_STR
            || bySkillType == AT_SKILL_CYCLONE_STR
            || bySkillType == AT_SKILL_CYCLONE_STR_MG
            || bySkillType == AT_SKILL_SLASH_STR
            ))
    {
        bCantSkill = true;
    }

    if ((bySkillType == AT_SKILL_POWER_SLASH || bySkillType == AT_SKILL_POWER_SLASH_STR)
        && isSittingOnPet)
    {
        bCantSkill = true;
    }

    if (bySkillType == AT_SKILL_PARTY_TELEPORT && PartyNumber <= 0)
    {
        bCantSkill = true;
    }

    if (bySkillType == AT_SKILL_PARTY_TELEPORT && (IsDoppelGanger1() || IsDoppelGanger2() || IsDoppelGanger3() || IsDoppelGanger4()))
    {
        bCantSkill = true;
    }

    if (bySkillType == AT_SKILL_EARTHSHAKE || bySkillType == AT_SKILL_EARTHSHAKE_STR || bySkillType == AT_SKILL_EARTHSHAKE_MASTERY)
    {
        BYTE byDarkHorseLife = 0;
        byDarkHorseLife = CharacterMachine->Equipment[EQUIPMENT_HELPER].Durability;
        if (byDarkHorseLife == 0 || Hero->Helper.Type != MODEL_DARK_HORSE_ITEM)
        {
            bCantSkill = true;
        }
    }
#ifdef PJH_FIX_SPRIT
    /*박종훈*/
    if (bySkillType >= AT_PET_COMMAND_DEFAULT && bySkillType < AT_PET_COMMAND_END)
    {
        int iCharisma = CharacterAttribute->Charisma + CharacterAttribute->AddCharisma;
        PET_INFO PetInfo;
        giPetManager::GetPetInfo(PetInfo, 421 - PET_TYPE_DARK_SPIRIT);
        int RequireCharisma = (185 + (PetInfo.m_wLevel * 15));
        if (RequireCharisma > iCharisma)
        {
            bCantSkill = true;
        }
    }
#endif //PJH_FIX_SPRIT
    if ((bySkillType == AT_SKILL_INFINITY_ARROW)
        || (bySkillType == AT_SKILL_INFINITY_ARROW_STR)
        || (bySkillType == AT_SKILL_EXPANSION_OF_WIZARDRY)
        || (bySkillType == AT_SKILL_EXPANSION_OF_WIZARDRY_STR)
        || (bySkillType == AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY)
        )
    {
        if ((g_isCharacterBuff((&Hero->Object), eBuff_InfinityArrow)) || (g_isCharacterBuff((&Hero->Object), eBuff_SwellOfMagicPower)))
        {
            bCantSkill = true;
        }
    }

    if (bySkillType == AT_SKILL_FIRE_SLASH || bySkillType == AT_SKILL_FIRE_SLASH_STR)
    {
        WORD Strength;
        const WORD wRequireStrength = 596;
        Strength = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
        if (Strength < wRequireStrength)
        {
            bCantSkill = true;
        }
        int iTypeL = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;
        int iTypeR = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;

        if (!(iTypeR != -1 && (iTypeR < ITEM_STAFF || iTypeR >= ITEM_STAFF + MAX_ITEM_INDEX) && (iTypeL < ITEM_STAFF || iTypeL >= ITEM_STAFF + MAX_ITEM_INDEX)))
        {
            bCantSkill = true;
        }
    }

    switch (bySkillType)
    {
        //case AT_SKILL_PIERCING:
    case AT_SKILL_ICE_ARROW:
    case AT_SKILL_ICE_ARROW_STR:
    {
        WORD  Dexterity;
        const WORD wRequireDexterity = 646;
        Dexterity = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
        if (Dexterity < wRequireDexterity)
        {
            bCantSkill = true;
        }
    }break;
    }

    if (bySkillType == AT_SKILL_TWISTING_SLASH
        || bySkillType == AT_SKILL_TWISTING_SLASH_STR
        || bySkillType == AT_SKILL_TWISTING_SLASH_STR_MG
        || bySkillType == AT_SKILL_TWISTING_SLASH_MASTERY
        || bySkillType == AT_SKILL_RAGEFUL_BLOW
        || bySkillType == AT_SKILL_RAGEFUL_BLOW_STR
        || bySkillType == AT_SKILL_RAGEFUL_BLOW_MASTERY
        || bySkillType == AT_SKILL_DEATHSTAB
        || bySkillType == AT_SKILL_DEATHSTAB_STR
        )
    {
        int iTypeL = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;
        int iTypeR = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;

        if (!(iTypeR != -1 && (iTypeR < ITEM_STAFF || iTypeR >= ITEM_STAFF + MAX_ITEM_INDEX) && (iTypeL < ITEM_STAFF || iTypeL >= ITEM_STAFF + MAX_ITEM_INDEX)))
        {
            bCantSkill = true;
        }
    }

    if (gMapManager.InChaosCastle() == true)
    {
        if (bySkillType == AT_SKILL_EARTHSHAKE
            || bySkillType == AT_SKILL_EARTHSHAKE_STR
            || bySkillType == AT_SKILL_EARTHSHAKE_MASTERY
            || bySkillType == AT_SKILL_RIDER
            || (static_cast<int>(bySkillType) >= static_cast<int>(AT_PET_COMMAND_DEFAULT) && static_cast<int>(bySkillType) <= static_cast<int>(AT_PET_COMMAND_TARGET))
            )
        {
            bCantSkill = true;
        }
    }
    else
    {
        if (bySkillType == AT_SKILL_EARTHSHAKE
            || bySkillType == AT_SKILL_EARTHSHAKE_STR
            || bySkillType == AT_SKILL_EARTHSHAKE_MASTERY)
        {
            BYTE byDarkHorseLife = 0;
            byDarkHorseLife = CharacterMachine->Equipment[EQUIPMENT_HELPER].Durability;
            if (byDarkHorseLife == 0)
            {
                bCantSkill = true;
            }
        }
    }

    if (!g_CMonkSystem.IsSwordformGlovesUseSkill(bySkillType))
    {
        bCantSkill = true;
    }
    if (g_CMonkSystem.IsRideNotUseSkill(bySkillType, Hero->Helper.Type))
    {
        bCantSkill = true;
    }

    ITEM* pLeftRing = &CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];
    ITEM* pRightRing = &CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];

    if (g_CMonkSystem.IsChangeringNotUseSkill(pLeftRing->Type, pRightRing->Type, pLeftRing->Level, pRightRing->Level)
        && (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER))
    {
        bCantSkill = true;
    }

    float fU, fV;
    int iKindofSkill = 0;

    if (!g_csItemOption.IsNonWeaponSkillOrIsSkillEquipped(bySkillType))
    {
        bCantSkill = true;
    }

    if (static_cast<int>(bySkillType) >= static_cast<int>(AT_PET_COMMAND_DEFAULT) && static_cast<int>(bySkillType) <= static_cast<int>(AT_PET_COMMAND_END))
    {
        fU = ((static_cast<int>(bySkillType) - AT_PET_COMMAND_DEFAULT) % 8) * width / 256.f;
        fV = ((static_cast<int>(bySkillType) - AT_PET_COMMAND_DEFAULT) / 8) * height / 256.f;
        iKindofSkill = KOS_COMMAND;
    }
    else if (bySkillType == AT_SKILL_PLASMA_STORM_FENRIR)
    {
        fU = 4 * width / 256.f;
        fV = 0.f;
        iKindofSkill = KOS_COMMAND;
    }
    else if ((bySkillType >= AT_SKILL_ALICE_DRAINLIFE && bySkillType <= AT_SKILL_ALICE_THORNS))
    {
        fU = ((bySkillType - AT_SKILL_ALICE_DRAINLIFE) % 8) * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType >= AT_SKILL_ALICE_SLEEP && bySkillType <= AT_SKILL_ALICE_BLIND)
    {
        fU = ((bySkillType - AT_SKILL_ALICE_SLEEP + 4) % 8) * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType == AT_SKILL_ALICE_BERSERKER)
    {
        fU = 10 * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType >= AT_SKILL_ALICE_WEAKNESS && bySkillType <= AT_SKILL_ALICE_ENERVATION)
    {
        fU = (bySkillType - AT_SKILL_ALICE_WEAKNESS + 8) * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType >= AT_SKILL_SUMMON_EXPLOSION && bySkillType <= AT_SKILL_SUMMON_REQUIEM)
    {
        fU = ((bySkillType - AT_SKILL_SUMMON_EXPLOSION + 6) % 8) * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType == AT_SKILL_SUMMON_POLLUTION)
    {
        fU = 11 * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType == AT_SKILL_STRIKE_OF_DESTRUCTION)
    {
        fU = 7 * width / 256.f;
        fV = 2 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType == AT_SKILL_CHAOTIC_DISEIER)
    {
        fU = 3 * width / 256.f;
        fV = 8 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType == AT_SKILL_RECOVER)
    {
        fU = 9 * width / 256.f;
        fV = 2 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType == AT_SKILL_MULTI_SHOT)
    {
        if (gCharacterManager.GetEquipedBowType_Skill() == BOWTYPE_NONE)
        {
            bCantSkill = true;
        }

        fU = 0 * width / 256.f;
        fV = 8 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType == AT_SKILL_FLAME_STRIKE)
    {
        int iTypeL = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;
        int iTypeR = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;

        if (!(iTypeR != -1 && (iTypeR < ITEM_STAFF || iTypeR >= ITEM_STAFF + MAX_ITEM_INDEX) && (iTypeL < ITEM_STAFF || iTypeL >= ITEM_STAFF + MAX_ITEM_INDEX)))
        {
            bCantSkill = true;
        }

        fU = 1 * width / 256.f;
        fV = 8 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType == AT_SKILL_GIGANTIC_STORM)
    {
        fU = 2 * width / 256.f;
        fV = 8 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType == AT_SKILL_LIGHTNING_SHOCK)
    {
        fU = 2 * width / 256.f;
        fV = 3 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType == AT_SKILL_EXPANSION_OF_WIZARDRY)
    {
        fU = 8 * width / 256.f;
        fV = 2 * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillUseType == 4)
    {
        fU = (width / 256.f) * (Skill_Icon % 12);
        fV = (height / 256.f) * ((Skill_Icon / 12) + 4);
        iKindofSkill = KOS_SKILL2;
    }
    else if (bySkillType >= AT_SKILL_KILLING_BLOW)
    {
        fU = ((bySkillType - AT_SKILL_KILLING_BLOW) % 12) * width / 256.f;
        fV = ((bySkillType - AT_SKILL_KILLING_BLOW) / 12) * height / 256.f;
        iKindofSkill = KOS_SKILL3;
    }
    else if (bySkillType >= AT_SKILL_SPIRAL_SLASH)
    {
        fU = ((bySkillType - AT_SKILL_SPIRAL_SLASH) % 8) * width / 256.f;
        fV = ((bySkillType - AT_SKILL_SPIRAL_SLASH) / 8) * height / 256.f;
        iKindofSkill = KOS_SKILL2;
    }
    else
    {
        fU = ((bySkillType - 1) % 8) * width / 256.f;
        fV = ((bySkillType - 1) / 8) * height / 256.f;
        iKindofSkill = KOS_SKILL1;
    }
    int iSkillIndex = 0;
    switch (iKindofSkill)
    {
    case KOS_COMMAND:
    {
        iSkillIndex = IMAGE_COMMAND;
    }break;
    case KOS_SKILL1:
    {
        iSkillIndex = IMAGE_SKILL1;
    }break;
    case KOS_SKILL2:
    {
        iSkillIndex = IMAGE_SKILL2;
    }break;
    case KOS_SKILL3:
    {
        iSkillIndex = IMAGE_SKILL3;
    }break;
    }

    if (bySkillType >= AT_SKILL_MASTER_BEGIN)
    {
        if (bCantSkill)
        {
            RenderImage(BITMAP_INTERFACE_MASTER_BEGIN + 3, x, y, width, height, (20.f / 512.f) * (Skill_Icon % 25), ((28.f / 512.f) * ((Skill_Icon / 25))), 20.f / 512.f, 28.f / 512.f);
        }
        else
        {
            RenderImage(BITMAP_INTERFACE_MASTER_BEGIN + 2, x, y, width, height, (20.f / 512.f)* (Skill_Icon % 25), ((28.f / 512.f)* ((Skill_Icon / 25))), 20.f / 512.f, 28.f / 512.f);
        }
    }
    else
    {
        if (bCantSkill == true)
        {
            iSkillIndex += 6;
        }

        if (iSkillIndex != 0)
        {
            RenderBitmap(iSkillIndex, x, y, width, height, fU, fV, width / 256.f, height / 256.f);
        }
    }

    // Hotkey-number subscript retired -- both themes show it via RmlUi (#skill_slot_0..4's .skill-hotkey-label) instead.

    if ((bySkillType == AT_SKILL_CHAIN_DRIVE
        || bySkillType == AT_SKILL_CHAIN_DRIVE_STR
        || bySkillType == AT_SKILL_DRAGON_KICK
        || bySkillType == AT_SKILL_DRAGON_ROAR
        || bySkillType == AT_SKILL_DRAGON_ROAR_STR) && (bCantSkill))
        return;

    // The cooldown wipe lives in RmlUi (SkillCellEntry::cooldownFraction / ComputeSkillCooldownFraction()).
}

namespace
{
    // RenderSkillDelay()'s own fraction math (iSkillDelay/iSkillMaxDelay), draw call replaced with
    // a plain return. Feeds SkillCellEntry::cooldownFraction and
    // GetHotKeySlotCooldownFraction()/GetCurrentSkillCooldownFraction().
    //
    // Known gap: unlike the original, this doesn't suppress the cooldown wipe for
    // AT_SKILL_CHAIN_DRIVE/_STR, AT_SKILL_DRAGON_KICK/_ROAR/_STR when bCantSkill is also true --
    // a minor double-signal for those 4 skills, not a functional bug.
    float ComputeSkillCooldownFraction(int iIndex)
    {
        // Mirrors RenderSkillIcon()'s bySkillType resolution, used only for the 5-skill exclusion
        // gate below. RenderSkillDelay()'s own resolution (further down) is NOT pet-aware -- a
        // pre-existing inconsistency between the two, preserved rather than fixed.
        WORD bySkillTypeForGate = CharacterAttribute->Skill[iIndex];
        if (iIndex >= AT_PET_COMMAND_DEFAULT)
            bySkillTypeForGate = (WORD)iIndex;

        if (bySkillTypeForGate == AT_SKILL_INFINITY_ARROW || bySkillTypeForGate == AT_SKILL_INFINITY_ARROW_STR
            || bySkillTypeForGate == AT_SKILL_EXPANSION_OF_WIZARDRY || bySkillTypeForGate == AT_SKILL_EXPANSION_OF_WIZARDRY_STR
            || bySkillTypeForGate == AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY)
            return 0.f;

        // From here down: RenderSkillDelay()'s own original body, draw call replaced with a
        // fraction return.
        int iSkillDelay = CharacterAttribute->SkillDelay[iIndex];
        if (iSkillDelay <= 0)
            return 0.f;

        int iSkillType = CharacterAttribute->Skill[iIndex];

        if (iSkillType == AT_SKILL_PLASMA_STORM_FENRIR && !CheckAttack())
            return 0.f;

        int iSkillMaxDelay = SkillAttribute[iSkillType].Delay;
        if (iSkillMaxDelay == 0)
            return 0.f; // avoid a divide-by-zero the original's own float division would also hit

        return iSkillDelay / (float)iSkillMaxDelay;
    }
}

void mu::ui::window::CSkillList::RebuildGridSnapshot()
{
    // Same iteration/filter/zig-zag-position math the legacy grid loop used, now producing data
    // instead of drawing. Render() iterates the resulting snapshot.
    m_GridSnapshot.clear();
    m_PetSnapshot.clear();

    if (CharacterAttribute->SkillNumber == 0)
        return;

    float x = 385.f, y = 390.f;
    constexpr float width = 32.f, height = 38.f;
    const float fOrigX = 385.f;
    int iSkillCount = 0;

    for (int i = 0; i < MAX_MAGIC; ++i)
    {
        int iSkillType = CharacterAttribute->Skill[i];

        if (iSkillType == 0 || (iSkillType >= AT_SKILL_STUN && iSkillType <= AT_SKILL_REMOVAL_BUFF))
            continue;

        BYTE bySkillUseType = SkillAttribute[iSkillType].SkillUseType;
        if (bySkillUseType == SKILL_USE_TYPE_MASTER || bySkillUseType == SKILL_USE_TYPE_MASTERLEVEL)
            continue;

        if (iSkillCount == 18)
        {
            y -= height;
        }

        if (iSkillCount < 14)
        {
            int iRemainder = iSkillCount % 2;
            int iQuotient = iSkillCount / 2;
            x = (iRemainder == 0) ? (fOrigX + iQuotient * width) : (fOrigX - (iQuotient + 1) * width);
        }
        else if (iSkillCount < 18)
        {
            x = fOrigX - (8 * width) - ((iSkillCount - 14) * width);
        }
        else
        {
            x = fOrigX - (12 * width) + ((iSkillCount - 17) * width);
        }

        iSkillCount++;

        SkillCellEntry entry;
        entry.left = x;
        entry.top = y;
        entry.skillIndex = i;
        entry.isPet = false;
        entry.isCurrent = (i == Hero->CurrentSkill);
        entry.cooldownFraction = ComputeSkillCooldownFraction(i);
        m_GridSnapshot.push_back(entry);
    }

    if (Hero->m_pPet != NULL)
    {
        float px = 353.f, py = 352.f;
        for (int i = AT_PET_COMMAND_DEFAULT; i < AT_PET_COMMAND_END; ++i)
        {
            SkillCellEntry entry;
            entry.left = px;
            entry.top = py;
            entry.skillIndex = i;
            entry.isPet = true;
            entry.isCurrent = (i == Hero->CurrentSkill);
            entry.cooldownFraction = ComputeSkillCooldownFraction(i);
            m_PetSnapshot.push_back(entry);
            px += width;
        }
    }
}

void mu::ui::window::CSkillList::QueueTooltip(int iSkillIndex, float x, float y)
{
    m_bTooltipPending = true;
    m_iTooltipSkillIndex = iSkillIndex;
    m_fTooltipAnchorX = x;
    m_fTooltipAnchorY = y;
}

float mu::ui::window::CSkillList::GetHotKeySlotCooldownFraction(int iSlotIndex)
{
    if (iSlotIndex < 0 || iSlotIndex >= 5)
        return 0.f;
    if (CharacterAttribute->SkillNumber == 0)
        return 0.f;

    int iStartSkillIndex = m_bHotKeySkillListUp ? 6 : 1;
    int iIndex = iStartSkillIndex + iSlotIndex;
    if (iIndex == 10)
        iIndex = 0;

    if (m_iHotKeySkillType[iIndex] == -1)
        return 0.f;

    if (m_iHotKeySkillType[iIndex] >= AT_PET_COMMAND_DEFAULT && m_iHotKeySkillType[iIndex] < AT_PET_COMMAND_END)
    {
        if (Hero->m_pPet == NULL)
            return 0.f;
    }

    return ComputeSkillCooldownFraction(m_iHotKeySkillType[iIndex]);
}

float mu::ui::window::CSkillList::GetCurrentSkillCooldownFraction()
{
    return ComputeSkillCooldownFraction(Hero->CurrentSkill);
}

// Click/hover entry points bound from main_frame.rml's data-event-click/mouseover/mouseout (see Create(), CMainFrameWindow.cpp).
void mu::ui::window::CSkillList::OnHotkeySlotClick(int iSlotIndex)
{
    if (iSlotIndex < 0 || iSlotIndex >= 5)
        return;
    if (CharacterAttribute->SkillNumber == 0)
        return;

    int iStartSkillIndex = m_bHotKeySkillListUp ? 6 : 1;
    int iIndex = iStartSkillIndex + iSlotIndex;
    if (iIndex == 10)
        iIndex = 0;

    if (m_iHotKeySkillType[iIndex] == -1)
        return;

    // Mirrors the legacy mouse-click branch, deliberately NOT UseHotKey() -- the original mouse
    // click never went through it either (only the keyboard 0-9 press does), so its pet-check/
    // auto-attack-cancel rule doesn't apply here.
    WORD bySkillType = CharacterAttribute->Skill[m_iHotKeySkillType[iIndex]];
    if (bySkillType == 0 || (bySkillType >= AT_SKILL_STUN && bySkillType <= AT_SKILL_REMOVAL_BUFF))
        return;
    if (SkillAttribute[bySkillType].SkillUseType == SKILL_USE_TYPE_MASTERLEVEL)
        return;

    m_wHeroPriorSkill = CharacterAttribute->Skill[Hero->CurrentSkill];
    Hero->CurrentSkill = m_iHotKeySkillType[iIndex];
    PlayBuffer(SOUND_CLICK01);
}

void mu::ui::window::CSkillList::OnHotkeySlotHover(int iSlotIndex)
{
    if (iSlotIndex < 0 || iSlotIndex >= 5)
        return;
    if (CharacterAttribute->SkillNumber == 0)
        return;

    int iStartSkillIndex = m_bHotKeySkillListUp ? 6 : 1;
    int iIndex = iStartSkillIndex + iSlotIndex;
    if (iIndex == 10)
        iIndex = 0;

    if (m_iHotKeySkillType[iIndex] == -1)
        return;

    WORD bySkillType = CharacterAttribute->Skill[m_iHotKeySkillType[iIndex]];
    if (bySkillType == 0 || (bySkillType >= AT_SKILL_STUN && bySkillType <= AT_SKILL_REMOVAL_BUFF))
        return;
    if (SkillAttribute[bySkillType].SkillUseType == SKILL_USE_TYPE_MASTERLEVEL)
        return;

    // Anchor matches RenderCurrentSkillAndHotSkillList()'s x for the same slot (190 + (iSlotIndex+1)*32).
    QueueTooltip(m_iHotKeySkillType[iIndex], 190.f + (iSlotIndex + 1) * 32.f, 431.f);
}

void mu::ui::window::CSkillList::OnCurrentSkillClick()
{
    m_bSkillList = !m_bSkillList;
    PlayBuffer(SOUND_CLICK01);
}

void mu::ui::window::CSkillList::OnCurrentSkillHover()
{
    QueueTooltip(Hero->CurrentSkill, 392.f, 437.f);
}

void mu::ui::window::CSkillList::OnGridCellClick(int iSkillIndex)
{
    m_wHeroPriorSkill = CharacterAttribute->Skill[Hero->CurrentSkill];
    Hero->CurrentSkill = iSkillIndex;
    m_bSkillList = false;
    PlayBuffer(SOUND_CLICK01);
}

void mu::ui::window::CSkillList::OnGridCellHover(int iSkillIndex)
{
    m_iHoveredGridSkillIndex = iSkillIndex;
    for (const SkillCellEntry& entry : m_GridSnapshot)
    {
        if (entry.skillIndex == iSkillIndex)
        {
            QueueTooltip(iSkillIndex, entry.left, entry.top);
            break;
        }
    }
}

void mu::ui::window::CSkillList::OnPetCellClick(int iSkillIndex)
{
    m_wHeroPriorSkill = CharacterAttribute->Skill[Hero->CurrentSkill];
    Hero->CurrentSkill = iSkillIndex;
    m_bSkillList = false;
    PlayBuffer(SOUND_CLICK01);
}

void mu::ui::window::CSkillList::OnPetCellHover(int iSkillIndex)
{
    // Pet-row entries arm Ctrl+digit assignment the same way grid entries do (legacy behavior, preserved).
    m_iHoveredGridSkillIndex = iSkillIndex;
    for (const SkillCellEntry& entry : m_PetSnapshot)
    {
        if (entry.skillIndex == iSkillIndex)
        {
            QueueTooltip(iSkillIndex, entry.left, entry.top);
            break;
        }
    }
}

void mu::ui::window::CSkillList::OnUnhover()
{
    m_bTooltipPending = false;
    m_iTooltipSkillIndex = -1;
    m_iHoveredGridSkillIndex = -1;
}

bool mu::ui::window::CSkillList::IsSkillListUp()
{
    return m_bHotKeySkillListUp;
}

void mu::ui::window::CSkillList::ResetMouseLButton()
{
    MouseLButton = false;
    MouseLButtonPop = false;
    MouseLButtonPush = false;
}

void mu::ui::window::CMainFrameWindow::SetPreExp_Wide(__int64 dwPreExp)
{
    m_loPreExp = dwPreExp;
}

void mu::ui::window::CMainFrameWindow::SetGetExp_Wide(__int64 dwGetExp)
{
    m_loGetExp = dwGetExp;

    if (m_loGetExp > 0)
    {
        m_bExpEffect = true;
        m_dwExpEffectTime = timeGetTime();
    }
}

void mu::ui::window::CMainFrameWindow::SetPreExp(__int64 dwPreExp)
{
    m_dwPreExp = dwPreExp;
}

void mu::ui::window::CMainFrameWindow::SetGetExp(__int64 dwGetExp)
{
    m_dwGetExp = dwGetExp;

    if (m_dwGetExp > 0)
    {
        m_bExpEffect = true;
        m_dwExpEffectTime = timeGetTime();
    }
}


void mu::ui::window::CMainFrameWindow::SetBtnState(int iBtnType, bool bStateDown)
{
    // Sets a bound "open" model boolean; main_frame.rcss selects the panel-open sprite rect when true.
    if (!m_pRmlDoc) return;

    switch (iBtnType)
    {
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
    case MAINFRAME_BTN_PARTCHARGE:
        if (m_RmlBinder.GetModel().cShopOpen != bStateDown)
        {
            m_RmlBinder.GetModel().cShopOpen = bStateDown;
            m_RmlBinder.MarkDirty("cshop_open");
        }
        break;
#endif //defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
    case MAINFRAME_BTN_CHAINFO:
        if (m_RmlBinder.GetModel().chaInfoOpen != bStateDown)
        {
            m_RmlBinder.GetModel().chaInfoOpen = bStateDown;
            m_RmlBinder.MarkDirty("chainfo_open");
        }
        break;
    case MAINFRAME_BTN_MYINVEN:
        if (m_RmlBinder.GetModel().myInvenOpen != bStateDown)
        {
            m_RmlBinder.GetModel().myInvenOpen = bStateDown;
            m_RmlBinder.MarkDirty("myinven_open");
        }
        break;
    case MAINFRAME_BTN_FRIEND:
        if (m_RmlBinder.GetModel().friendOpen != bStateDown)
        {
            m_RmlBinder.GetModel().friendOpen = bStateDown;
            m_RmlBinder.MarkDirty("friend_open");
        }
        break;
    case MAINFRAME_BTN_WINDOW:
        if (m_RmlBinder.GetModel().windowOpen != bStateDown)
        {
            m_RmlBinder.GetModel().windowOpen = bStateDown;
            m_RmlBinder.MarkDirty("window_open");
        }
        break;
    }
}

void mu::ui::window::CMainFrameWindow::SyncDocVisibility(bool sceneAllowsShow)
{
    const bool show = IsVisible() && sceneAllowsShow;

    if (m_pRmlDoc)
    {
        if (show) m_pRmlDoc->Show(); else m_pRmlDoc->Hide();
    }

    // m_pRmlBgDoc needs the same gate: CManager::Render()'s centralized RenderBackgroundLayer()
    // call replays whatever's Show()n in the shared background context every frame, regardless of
    // whether this window itself is visible.
    if (m_pRmlBgDoc)
    {
        if (show) m_pRmlBgDoc->Show(); else m_pRmlBgDoc->Hide();
    }
}
