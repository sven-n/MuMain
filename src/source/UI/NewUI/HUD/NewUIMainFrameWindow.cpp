//////////////////////////////////////////////////////////////////////
// NewUIMainFrameWindow.cpp: implementation of the CNewUIMainFrameWindow class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <algorithm>
#include "I18N/All.h"

#include "UI/NewUI/HUD/NewUIMainFrameWindow.h"	// self
#include "UI/NewUI/Options/NewUIOptionWindow.h"
#include "UI/NewUI/NewUISystem.h"
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
#include "UI/NewUI/HUD/Skills/SkillTooltip.h"
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
#include "UI/NewUI/Party/NewUIFriendWindow.h"

namespace
{
    // Still used by RenderLeftFrame()/RenderCenterFrame() -- the two chrome regions that still
    // host legacy content (item hotkeys, skill list) and so stay legacy-rendered; see this class's
    // header comment for why the other two bands' chrome moved to RmlUi instead.
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

SEASON3B::CNewUIMainFrameWindow::CNewUIMainFrameWindow()
{
    m_bExpEffect = false;
    m_dwExpEffectTime = 0;
    m_dwPreExp = 0;
    m_dwGetExp = 0;
    m_bButtonBlink = false;
}

SEASON3B::CNewUIMainFrameWindow::~CNewUIMainFrameWindow()
{
    Release();
}

void SEASON3B::CNewUIMainFrameWindow::LoadImages()
{
    LoadBitmap(L"Interface\\newui_menu01.jpg", IMAGE_MENU_1, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_menu02.jpg", IMAGE_MENU_2, GL_LINEAR);
    LoadBitmap(L"Interface\\partCharge1\\newui_menu03.jpg", IMAGE_MENU_3, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_menu02-03.jpg", IMAGE_MENU_2_1, GL_LINEAR);
}

void SEASON3B::CNewUIMainFrameWindow::UnloadImages()
{
    DeleteBitmap(IMAGE_MENU_1);
    DeleteBitmap(IMAGE_MENU_2);
    DeleteBitmap(IMAGE_MENU_3);
    DeleteBitmap(IMAGE_MENU_2_1);
}

bool SEASON3B::CNewUIMainFrameWindow::Create(CNewUIManager* pNewUIMng, CNewUI3DRenderMng* pNewUI3DRenderMng)
{
    if (NULL == pNewUIMng || NULL == pNewUI3DRenderMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_MAINFRAME, this);

    m_pNewUI3DRenderMng = pNewUI3DRenderMng;
    m_pNewUI3DRenderMng->Add3DRenderObj(this, ITEMHOTKEYNUMBER_CAMERA_Z_ORDER);

    LoadImages();

    // RmlUi migration -- see this class's header comment. Guarded like every other hybrid
    // window's Create() (re-run on resolution change), so the document/model are created once,
    // ever.
    if (!m_pRmlDoc && RmlUiRuntime::Instance().IsCreated())
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

                // Phase 2 (skill list) -- see docs/rmlui-ui-system's Phase 2 plan and
                // MainFrameRmlModel::skillGridOpen's own header comment.
                c.Bind("skill_slot_0_cooldown", &model.skillSlot0Cooldown);
                c.Bind("skill_slot_1_cooldown", &model.skillSlot1Cooldown);
                c.Bind("skill_slot_2_cooldown", &model.skillSlot2Cooldown);
                c.Bind("skill_slot_3_cooldown", &model.skillSlot3Cooldown);
                c.Bind("skill_slot_4_cooldown", &model.skillSlot4Cooldown);
                c.Bind("current_skill_cooldown", &model.currentSkillCooldown);

                auto skillCell = c.RegisterStruct<SkillCellEntry>();
                skillCell.RegisterMember("left", &SkillCellEntry::left);
                skillCell.RegisterMember("top", &SkillCellEntry::top);
                skillCell.RegisterMember("skill_index", &SkillCellEntry::skillIndex);
                skillCell.RegisterMember("is_pet", &SkillCellEntry::isPet);
                skillCell.RegisterMember("is_current", &SkillCellEntry::isCurrent);
                skillCell.RegisterMember("cooldown_fraction", &SkillCellEntry::cooldownFraction);
                c.RegisterArray<std::vector<SkillCellEntry>>();

                c.Bind("skill_grid_open", &model.skillGridOpen);
                c.Bind("skill_grid_cells", &model.skillGridCells);
                c.Bind("pet_skill_cells", &model.petSkillCells);

                auto tooltipLine = c.RegisterStruct<SkillTooltipLineEntry>();
                tooltipLine.RegisterMember("text", &SkillTooltipLineEntry::text);
                tooltipLine.RegisterMember("color_blue", &SkillTooltipLineEntry::colorBlue);
                tooltipLine.RegisterMember("color_red", &SkillTooltipLineEntry::colorRed);
                tooltipLine.RegisterMember("color_dark_red", &SkillTooltipLineEntry::colorDarkRed);
                tooltipLine.RegisterMember("bold", &SkillTooltipLineEntry::bold);
                c.RegisterArray<std::vector<SkillTooltipLineEntry>>();

                c.Bind("skill_tooltip_visible", &model.skillTooltipVisible);
                c.Bind("skill_tooltip_left", &model.skillTooltipLeft);
                c.Bind("skill_tooltip_top", &model.skillTooltipTop);
                c.Bind("skill_tooltip_lines", &model.skillTooltipLines);

                // Phase 2 click/hover bindings -- route straight into CNewUISkillList (this
                // document's model is owned by CNewUIMainFrameWindow, but g_pSkillList has no
                // RmlUi document of its own, same established split as GetHotKeySlotNumber() etc,
                // see NewUIMainFrameWindow.h's own comment on that). Args are literal ints in the
                // RML expression itself (e.g. data-event-click="skill_hotkey_click(0)"), or the
                // bound cell's own skill_index field for the data-for'd grid/pet lists (e.g.
                // "skill_grid_click(cell.skill_index)") -- Variant::Get<int>() resolves either.
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

        // Deliberately NOT Show()n here -- Create() runs during WebzenScene()'s own boot-time
        // loading screen (LoadMainSceneInterface(), called well before SceneFlag ever reaches
        // MAIN_SCENE), and this document renders every frame once shown regardless of scene (see
        // SyncDocVisibility()'s own comment). An eager Show() here raced ahead of the first
        // SyncMainSceneHudVisibility() gate check, so the HUD flashed once on whatever frame
        // WebzenScene() happened to render before flipping SceneFlag to LOG_IN_SCENE. Left
        // hidden (RmlUi documents start unshown after LoadDocument()); SyncDocVisibility(),
        // called every frame regardless of scene, shows it the first time the gate actually
        // allows it.
    }

    Show(true);

    return true;
}

void SEASON3B::CNewUIMainFrameWindow::Release()
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

    // See CMuHelperBar::Release()'s identical rationale -- RmlUi renders last in the frame
    // regardless of scene, so this object's own release has no other way to hide it.
    if (m_pRmlDoc)
        m_pRmlDoc->Hide();
}

bool SEASON3B::CNewUIMainFrameWindow::Render()
{
    // Thin passthrough, not a full no-op -- see this class's header comment. Only the two chrome
    // bands/content calls that still host legacy content (item hotkeys' left band, skill list's
    // center band) remain; the right (buttons) and experience bands, plus the bars, moved to
    // RmlUi (main_frame.rml/.rcss) and are synced every frame by SyncRmlModel() (called from
    // Update()) instead of drawn here.
    //
    // 2026-09-01: the item-hotkey ("potion") band now reuses centerTransform, not its own
    // BottomHudLeftTransform -- per explicit feedback, anchor it next to the HP bar (which sits
    // at reference x=158, just right of this band's own x=0-152) rather than the legacy
    // window-left-edge anchor. Both bands still use their own internal 0-152-relative reference
    // coordinates unmodified; only which transform places that local space on screen changed.
    //
    // 2026-09-02: leftTransform/centerTransform are no longer identical -- each now carries its
    // OWN theme-provided offset (GetItemHotkeyOffsetX()/GetSkillListOffsetX(), read from
    // main_frame.rml's #item_hotkey_anchor/#skill_list_anchor markers -- see those methods' own
    // header comment in NewUIMainFrameWindow.h) on top of the same base BottomHudCenterTransform,
    // so a theme can independently reposition the item-hotkey and skill-hotkey bands via ordinary
    // RCSS. Both offsets default to 0 for legacy (unchanged behavior).
    //
    // 2026-09-02 bug fix: `* baseTransform.scaleX`, not a bare add -- confirmed by reading
    // Element::GetAbsoluteOffset() (ThirdParty/RmlUi/Source/Core/Element.cpp) directly:
    // it accumulates plain layout offsets up the tree and never looks at CSS `transform` at all,
    // so the marker's reported position ignores #bars's own `transform: scale(bars_scale)`
    // entirely -- GetItemHotkeyOffsetX()/GetSkillListOffsetX() are therefore UNSCALED reference-
    // pixel deltas (effectively just the marker's own local `left` value), not real screen
    // pixels. Transform::offsetX IS real screen pixels (screenX = offsetX + refX*scaleX), so
    // adding the raw delta under-shifted the actual icons relative to the correctly-scaled
    // RmlUi-only outline boxes around them (#item_slots/#skill_slots, themes/modern/
    // main_frame.rcss) -- reported as "the boxes for the potions and skills ... seem misaligned".
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
        UI::Scaling::ScopedActiveTransform layout(leftTransform, true);
        RenderLeftRegion();
    }
    {
        UI::Scaling::ScopedActiveTransform layout(centerTransform, true);
        RenderCenterRegion();
    }
    DisableAlphaBlend();

    return true;
}

void SEASON3B::CNewUIMainFrameWindow::Render3D()
{
    // centerTransform, not BottomHudLeftTransform -- see Render()'s header comment (2026-09-01,
    // item-hotkey band now anchors next to the HP bar instead of the window's left edge).
    //
    // 2026-09-01: `true` (transformMouse) is required here, not optional -- RenderItems() ->
    // RenderItem3D() (ZzzInventory.cpp) does its own hover-to-animate check via CheckMouseIn(),
    // which compares raw MouseX/MouseY against this function's reference-space sx/sy directly, no
    // ConvertPositionX/Y involved. Without transformMouse, MouseX/MouseY stay real screen pixels
    // while the rect is reference-space, so the hover test only lines up when centerTransform
    // happens to be identity scale/offset -- at any other window size the real cursor has to sit
    // well left of the actual on-screen icon before the raw numbers happen to satisfy the box test
    // (reported as "need to hover further left than the intended potion"). UseHotKeyItemRButton()
    // right below already passes true for exactly this reason -- Render3D()'s hover path was the
    // one call site that didn't match.
    //
    // 2026-09-02: += GetItemHotkeyOffsetX() * scaleX -- must match Render()'s own leftTransform
    // exactly, or the 3D icons render in a different place than where RenderLeftFrame()'s chrome
    // and the click hit-test (UseHotKeyItemRButton()) expect them. `* scaleX` is required, not
    // optional -- see Render()'s own comment on this same bug (GetItemHotkeyOffsetX() is an
    // unscaled reference-pixel delta, Transform::offsetX is real screen pixels).
    auto transform = UI::Scaling::BottomHudCenterTransform(WindowWidth, WindowHeight);
    transform.offsetX += GetItemHotkeyOffsetX() * transform.scaleX;
    UI::Scaling::ScopedActiveTransform layout(transform, true);
    m_ItemHotKey.RenderItems();
}

void SEASON3B::CNewUIMainFrameWindow::UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB)
{
    g_pMainFrame->RenderHotKeyItemCount();
}

bool SEASON3B::CNewUIMainFrameWindow::IsVisible() const
{
    return CNewUIObj::IsVisible();
}

void SEASON3B::CNewUIMainFrameWindow::RenderLeftRegion()
{
    m_pNewUI3DRenderMng->RenderUI2DEffect(ITEMHOTKEYNUMBER_CAMERA_Z_ORDER, UI2DEffectCallback, this, 0, 0);
}

void SEASON3B::CNewUIMainFrameWindow::RenderCenterRegion()
{
    // HP/MP/AG/SD bars moved to RmlUi (SyncRmlModel()/main_frame.rcss) -- the skill row/current-
    // skill icon stays legacy, out of scope for this pilot (see this class's header comment).
    g_pSkillList->RenderCurrentSkillAndHotSkillList();
}

// 2026-09-01: theme-aware for the modern theme (feedback: "the potions/skill icons are still
// using the old sprites as background", "we need to solve this ... since the inventory UI
// contains lots of 3D items too"). These two functions are the ONE place in the whole frame
// where this is actually fixable without touching RmlUi's render architecture: RmlUiRuntime
// hooks a single SetPreSubmitCallback that runs once, after EVERYTHING else in the frame
// (world, legacy 2D chrome, AND the 3D-composited item/skill icons -- see Render3D()/
// RenderLeftRegion()) is already recorded (RmlUiRuntime.cpp's own Create() comment). RmlUi
// therefore can never paint "behind" those 3D icons, in this window or any other -- an
// RmlUi-drawn background here would always cover them, not sit under them (this is exactly why
// main_frame.rml's #item_slots/#skill_slots outlines below are border-only, no fill). This
// function, by contrast, already runs in the correct pass -- it draws the legacy sprite chrome
// BEFORE Render3D()'s icon compositing happens later in the same frame, the same ordering that
// has always put icons on top of this exact background. Swapping the sprite for a flat
// RenderColorQuadARGB() here, for modern theme only, gets a real (not outline-only) themed
// background that correctly sits behind the icons, using the one call site that was already
// proven to composite correctly with them -- the general pattern to reuse for Inventory's own
// still-legacy 3D item icons whenever that window's turn comes, not a one-off here.
//
// Colors match themes/modern/main_frame.rcss's .slot-fill/.slot-frame tokens exactly (rgba(10,10,
// 10,150) / rgba(255,255,255,60)) so the legacy-drawn panel and the RmlUi-drawn gauges/buttons
// sitting on top of it read as one consistent surface, not two different systems -- the "unified
// styling" ask. One panel spans the full legacy chrome extent (x=0-488, kLeftBandWidth's end
// through kMenu3Start+kMenu3CenterWidth) since RenderLeftFrame()/RenderCenterFrame() are called
// back-to-back under the identical transform (Render()'s leftTransform is centerTransform's own
// alias) -- drawn as two flush quads (one per function, no visible seam, same flat color) rather
// than restructuring these into one function, to keep each function's own responsibility (its
// own band) unchanged for legacy theme.
void SEASON3B::CNewUIMainFrameWindow::RenderLeftFrame()
{
    if (UI::RmlBridge::GetActiveThemeName() == "modern")
    {
        // 2026-09-02: no border lines here anymore -- this panel's own top/bottom/outer-edge
        // outline read as a visible divider once the whole bottom HUD strip got a single
        // continuous top border of its own (#gauge_frame, themes/modern/main_frame.rcss), drawn
        // over this same span. Fill only; the unified RmlUi border is now the only outline.
        RenderColorQuadARGB(0.0f, kHudTop, kLeftBandWidth, kHudContentHeight, 0x960A0A0Au);
        return;
    }

    RenderImageStretch(IMAGE_MENU_1, 0.0f, kHudTop, kLeftBandWidth, kHudContentHeight,
                       0.0f, 0.0f, kLeftBandWidth, kHudContentHeight);
}

void SEASON3B::CNewUIMainFrameWindow::RenderCenterFrame()
{
    if (UI::RmlBridge::GetActiveThemeName() == "modern")
    {
        // 2026-09-02: was kCenterBandStart(152) to kMenu3Start+kMenu3CenterWidth(488), a 336-unit
        // span sized to cover the vertical HP/SD/AG/MP gauges that used to be interleaved inside
        // it (redesign #3 and earlier) -- now that those gauges have moved out to their own
        // stacks at the canvas edges (redesign #5, themes/modern/main_frame.rcss), that width is
        // just dead chrome space around the skill icons (their own real footprint is 222-416),
        // and since this whole quad also now shifts by GetSkillListOffsetX() (Render()'s
        // centerTransform), its old right edge landed PAST the new MP/AG stack's own left edge --
        // reported as "the background/container for the potions and skills list seems extended
        // ... not sure if this was intended to contain also the gauge/bars" (it wasn't). Narrowed
        // to hug just the skill icons, padded 8px each side to match RenderLeftFrame()'s own
        // potions padding (152 vs their 142-wide native span) -- 214 = 222-8, 424 = 416+8. These
        // two numbers are also chosen so this panel's shifted left edge (214+78=292) lands
        // exactly where RenderLeftFrame()'s own shifted right edge does
        // (kLeftBandWidth+GetItemHotkeyOffsetX() = 152+140=292) -- the two chrome panels meet
        // flush, no gap or overlap, despite using two different offsets.
        // 2026-09-02: no border lines here anymore -- same reasoning as RenderLeftFrame()'s
        // identical change (this panel's own outline read as a divider once the whole strip got
        // one continuous top border of its own, #gauge_frame). Fill only.
        const float left = 214.0f;
        const float right = 424.0f;
        RenderColorQuadARGB(left, kHudTop, right - left, kHudContentHeight, 0x960A0A0Au);

        // Modern equivalent of the legacy IMAGE_MENU_2_1 "skill list expanded" highlight below --
        // same trigger/rect, flat translucent overlay instead of a sprite frame.
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

// RenderRightFrame()/RenderExperienceBackground() (chrome) and RenderLifeMana()/RenderGuageAG()/
// RenderGuageSD()/RenderExperience() (content, computed each frame into the RmlUi model instead --
// see SyncRmlModel()) removed -- moved to RmlUi (main_frame.rml/.rcss). See this class's header
// comment for why these bands, but not the left/center bands, could move (no remaining legacy
// content occupies the same pixels).

void SEASON3B::CNewUIMainFrameWindow::RenderHotKeyItemCount()
{
    m_ItemHotKey.RenderItemCount();
}

// RenderButtons()/RenderCharInfoButton()/RenderFriendButton()/RenderFriendButtonState() and
// BtnProcess() removed -- the 5 corner buttons moved to RmlUi (data-event-click bindings, see
// Create()); RmlUi's own Context now does hit-testing for them, so this never has legacy button
// objects left to check.
bool SEASON3B::CNewUIMainFrameWindow::UpdateMouseEvent()
{
    // RmlUi's own context does hit-testing now (see this class's header comment) -- never
    // consumes the legacy mouse event.
    return true;
}

bool SEASON3B::CNewUIMainFrameWindow::UpdateKeyEvent()
{
    if (m_ItemHotKey.UpdateKeyEvent() == false)
    {
        return false;
    }
    return true;
}

bool SEASON3B::CNewUIMainFrameWindow::Update()
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

    // Button clicks -- polled-and-cleared exactly like every other migrated window's RmlClickX()
    // pattern (see CMuHelperBar::Update()). Logic ported verbatim from the legacy BtnProcess(),
    // minus the CNewUIButton hit-test wrapper (RmlUi's data-event-click already tells us the click
    // landed).
    if (m_bRmlMyInvenClicked)
    {
        m_bRmlMyInvenClicked = false;
        g_pNewUISystem->Toggle(SEASON3B::INTERFACE_INVENTORY);
        PlayBuffer(SOUND_CLICK01);
    }
    if (m_bRmlChaInfoClicked)
    {
        m_bRmlChaInfoClicked = false;
        g_pNewUISystem->Toggle(SEASON3B::INTERFACE_CHARACTER);
        PlayBuffer(SOUND_CLICK01);
        if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHARACTER))
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
                    g_pSystemLogBox->AddText(I18N::Game::YouMustBeAtLeastLevel6ToUseTheMyFriendFunction, SEASON3B::TYPE_SYSTEM_MESSAGE);
                }
            }
            else
            {
                g_pNewUISystem->Toggle(SEASON3B::INTERFACE_FRIEND);
            }
            PlayBuffer(SOUND_CLICK01);
        }
    }
    if (m_bRmlWindowClicked)
    {
        m_bRmlWindowClicked = false;
        g_pNewUISystem->Toggle(SEASON3B::INTERFACE_WINDOW_MENU);
        PlayBuffer(SOUND_CLICK01);
    }
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
    if (m_bRmlCShopClicked)
    {
        m_bRmlCShopClicked = false;
        if (g_pInGameShop->IsInGameShopOpen() == false)
        {
            // matches the legacy BtnProcess()'s early-out -- nothing to do until the server
            // confirms the shop is actually open.
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
            if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INGAMESHOP) == false)
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
                g_pNewUISystem->Hide(SEASON3B::INTERFACE_INGAMESHOP);
            }
        }
    }
#endif //defined PBG_ADD_INGAMESHOP_UI_MAINFRAME

    SyncRmlModel();

    return true;
}

void SEASON3B::CNewUIMainFrameWindow::SyncRmlModel()
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

    // Shared #bars/#buttons/#exp transform -- must track the still-legacy center-band chrome's
    // own window-size-driven scale (see MainFrameRmlModel::barsLeft's header comment for the full
    // history of how this became one shared group instead of three independent ones). All three
    // use static *reference-pixel*, UN-rebased coordinates identical to the legacy render calls'
    // own constants -- this transform alone maps that whole local reference space onto real
    // window pixels (screenPos = refPos * scale + offset, same formula PositionX()/PositionY()
    // compute internally).
    {
        const auto centerTransform = UI::Scaling::BottomHudCenterTransform(WindowWidth, WindowHeight);
        syncFloat(&MainFrameRmlModel::barsLeft, "bars_left", centerTransform.offsetX);
        syncFloat(&MainFrameRmlModel::barsTop, "bars_top", centerTransform.offsetY);
        syncFloat(&MainFrameRmlModel::barsScale, "bars_scale", centerTransform.scaleX);

        // 2026-09-02: item-hotkey/skill-hotkey band offsets -- read from #item_hotkey_anchor/
        // #skill_list_anchor's real screen position (Element::GetAbsoluteOffset()) and turned
        // into a delta from centerTransform's own unshifted offsetX. The still-legacy Render()/
        // Render3D()/UseHotKeyItemRButton() (item hotkey) and CNewUISkillList's own Render()/
        // UpdateMouseEvent() (skill hotkey) each apply this delta to whichever transform they use,
        // keeping render AND click hit-testing in sync automatically -- see
        // GetItemHotkeyOffsetX()'s own header comment (NewUIMainFrameWindow.h) for why this reads
        // an RmlUi element instead of a per-theme C++ branch. One frame of lag is possible here
        // (this runs before this frame's own RmlUi Update(), so the marker reflects last frame's
        // layout) -- harmless in practice, these markers only move when the active theme changes,
        // not every frame.
        if (Rml::Element* pAnchor = m_pRmlDoc->GetElementById("item_hotkey_anchor"))
            m_fItemHotkeyOffsetX = pAnchor->GetAbsoluteOffset().x - centerTransform.offsetX;
        if (Rml::Element* pAnchor = m_pRmlDoc->GetElementById("skill_list_anchor"))
            m_fSkillListOffsetX = pAnchor->GetAbsoluteOffset().x - centerTransform.offsetX;
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

    // 2026-09-02: "X / Y" (current/max), not just "X" -- feedback: "the gauge bars can also
    // display the max value instead of just the current value". Shared model field, both themes.
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

    // Button tooltips -- static strings, re-checked every frame like CMuHelperBar's syncLabel()
    // for consistency, but effectively set-once.
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
            && !(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC)
                || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHARACTER));
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

    // Skill-hotkey row selection highlight (modern theme only, see MainFrameRmlModel::
    // skillSlot0Selected's own comment and CNewUISkillList::IsHotKeySlotCurrentSkill()) -- read
    // every frame like everything else above; g_pSkillList itself is still fully legacy (Phase 2),
    // this just also mirrors its per-slot selected state into this pilot's own model.
    syncBool(&MainFrameRmlModel::skillSlot0Selected, "skill_slot_0_selected", g_pSkillList->IsHotKeySlotCurrentSkill(0));
    syncBool(&MainFrameRmlModel::skillSlot1Selected, "skill_slot_1_selected", g_pSkillList->IsHotKeySlotCurrentSkill(1));
    syncBool(&MainFrameRmlModel::skillSlot2Selected, "skill_slot_2_selected", g_pSkillList->IsHotKeySlotCurrentSkill(2));
    syncBool(&MainFrameRmlModel::skillSlot3Selected, "skill_slot_3_selected", g_pSkillList->IsHotKeySlotCurrentSkill(3));
    syncBool(&MainFrameRmlModel::skillSlot4Selected, "skill_slot_4_selected", g_pSkillList->IsHotKeySlotCurrentSkill(4));

    // Skill-hotkey number labels (modern theme only -- see MainFrameRmlModel::skillSlot0Hotkey's
    // own comment and CNewUISkillList::GetHotKeySlotNumber()). -1 (empty slot) becomes an empty
    // string, same "draw nothing" behavior the legacy digit-sprite path already has for that case.
    auto hotkeyText = [](int hotkey) { return hotkey >= 0 ? std::to_string(hotkey) : Rml::String(); };
    syncText(&MainFrameRmlModel::skillSlot0Hotkey, "skill_slot_0_hotkey", hotkeyText(g_pSkillList->GetHotKeySlotNumber(0)));
    syncText(&MainFrameRmlModel::skillSlot1Hotkey, "skill_slot_1_hotkey", hotkeyText(g_pSkillList->GetHotKeySlotNumber(1)));
    syncText(&MainFrameRmlModel::skillSlot2Hotkey, "skill_slot_2_hotkey", hotkeyText(g_pSkillList->GetHotKeySlotNumber(2)));
    syncText(&MainFrameRmlModel::skillSlot3Hotkey, "skill_slot_3_hotkey", hotkeyText(g_pSkillList->GetHotKeySlotNumber(3)));
    syncText(&MainFrameRmlModel::skillSlot4Hotkey, "skill_slot_4_hotkey", hotkeyText(g_pSkillList->GetHotKeySlotNumber(4)));

    // Phase 2 (skill list) additions -- see docs/rmlui-ui-system's Phase 2 plan.
    syncFloat(&MainFrameRmlModel::skillSlot0Cooldown, "skill_slot_0_cooldown", g_pSkillList->GetHotKeySlotCooldownFraction(0));
    syncFloat(&MainFrameRmlModel::skillSlot1Cooldown, "skill_slot_1_cooldown", g_pSkillList->GetHotKeySlotCooldownFraction(1));
    syncFloat(&MainFrameRmlModel::skillSlot2Cooldown, "skill_slot_2_cooldown", g_pSkillList->GetHotKeySlotCooldownFraction(2));
    syncFloat(&MainFrameRmlModel::skillSlot3Cooldown, "skill_slot_3_cooldown", g_pSkillList->GetHotKeySlotCooldownFraction(3));
    syncFloat(&MainFrameRmlModel::skillSlot4Cooldown, "skill_slot_4_cooldown", g_pSkillList->GetHotKeySlotCooldownFraction(4));
    syncFloat(&MainFrameRmlModel::currentSkillCooldown, "current_skill_cooldown", g_pSkillList->GetCurrentSkillCooldownFraction());

    syncBool(&MainFrameRmlModel::skillGridOpen, "skill_grid_open", g_pSkillList->IsSkillGridOpen());

    // Dynamic-count lists -- SkillCellEntry is shared verbatim between CNewUISkillList's own
    // snapshot and this model (no per-field conversion needed). Unconditional copy+MarkDirty every
    // frame while the grid is open, matching CBuffStrip's own proven "unconditional MarkDirty()
    // correctly re-renders the list" approach (newui-tier-adapter.md) -- cooldown fractions change
    // every frame anyway, so a change-check would rarely save anything.
    if (model.skillGridOpen)
    {
        model.skillGridCells = g_pSkillList->GetGridSnapshot();
        model.petSkillCells = g_pSkillList->GetPetSnapshot();
        m_RmlBinder.MarkDirty("skill_grid_cells");
        m_RmlBinder.MarkDirty("pet_skill_cells");
    }

    // Shared skill tooltip -- one hover target queued at a time (QueueTooltip()/OnUnhover(),
    // CNewUISkillList). BuildModelForSlot() is the same content resolution (pet-command dispatch,
    // BuildModel()) SkillTooltip.cpp's own Render() uses for this pilot's still-legacy siblings
    // (NewUIMuHelper.cpp, NewUISiegeWarBase.cpp) -- only the destination (RmlUi vs. legacy
    // TextList) differs.
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

float SEASON3B::CNewUIMainFrameWindow::GetLayerDepth()
{
    return 10.6f;
}

float SEASON3B::CNewUIMainFrameWindow::GetKeyEventOrder()
{
    return 2.9f;
}

void SEASON3B::CNewUIMainFrameWindow::SetItemHotKey(int iHotKey, int iItemType, int iItemLevel)
{
    m_ItemHotKey.SetHotKey(iHotKey, iItemType, iItemLevel);
}

int SEASON3B::CNewUIMainFrameWindow::GetItemHotKey(int iHotKey)
{
    return m_ItemHotKey.GetHotKey(iHotKey);
}

int SEASON3B::CNewUIMainFrameWindow::GetItemHotKeyLevel(int iHotKey)
{
    return m_ItemHotKey.GetHotKeyLevel(iHotKey);
}

void SEASON3B::CNewUIMainFrameWindow::UseHotKeyItemRButton()
{
    // centerTransform, not BottomHudLeftTransform, += GetItemHotkeyOffsetX() * scaleX -- must
    // match Render3D()'s (and Render()'s leftTransform) exactly, or right-click hit-testing lands
    // on the wrong screen position. `* scaleX` -- see Render()'s own comment on this bug.
    auto transform = UI::Scaling::BottomHudCenterTransform(WindowWidth, WindowHeight);
    transform.offsetX += GetItemHotkeyOffsetX() * transform.scaleX;
    UI::Scaling::ScopedActiveTransform layout(transform, true);
    m_ItemHotKey.UseItemRButton();
}

void SEASON3B::CNewUIMainFrameWindow::UpdateItemHotKey()
{
    m_ItemHotKey.UpdateKeyEvent();
}

void SEASON3B::CNewUIMainFrameWindow::ResetSkillHotKey()
{
    g_pSkillList->Reset();
}

void SEASON3B::CNewUIMainFrameWindow::SetSkillHotKey(int iHotKey, int iSkillType)
{
    g_pSkillList->SetHotKey(iHotKey, iSkillType);
}

int SEASON3B::CNewUIMainFrameWindow::GetSkillHotKey(int iHotKey)
{
    return g_pSkillList->GetHotKey(iHotKey);
}

int SEASON3B::CNewUIMainFrameWindow::GetSkillHotKeyIndex(int iSkillType)
{
    return g_pSkillList->GetSkillIndex(iSkillType);
}

SEASON3B::CNewUIItemHotKey::CNewUIItemHotKey()
{
    for (int i = 0; i < HOTKEY_COUNT; ++i)
    {
        m_iHotKeyItemType[i] = -1;
        m_iHotKeyItemLevel[i] = 0;
    }
}

SEASON3B::CNewUIItemHotKey::~CNewUIItemHotKey()
{
}

bool SEASON3B::CNewUIItemHotKey::UpdateKeyEvent()
{
    int iIndex = -1;

    if (SEASON3B::IsPress('Q') == true)
    {
        iIndex = GetHotKeyItemIndex(HOTKEY_Q);
    }
    else if (SEASON3B::IsPress('W') == true)
    {
        iIndex = GetHotKeyItemIndex(HOTKEY_W);
    }
    else if (SEASON3B::IsPress('E') == true)
    {
        iIndex = GetHotKeyItemIndex(HOTKEY_E);
    }
    else if (SEASON3B::IsPress('R') == true)
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
                SEASON3B::CreateOkMessageBox(I18N::Game::YouCannotUseThisItemWhileThePotionEffectsRemainActive, RGBA(255, 30, 0, 255));
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

int SEASON3B::CNewUIItemHotKey::GetHotKeyItemIndex(int iType, bool bItemCount)
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

bool SEASON3B::CNewUIItemHotKey::GetHotKeyCommonItem(IN int iHotKey, OUT int& iStart, OUT int& iEnd)
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

int SEASON3B::CNewUIItemHotKey::GetHotKeyItemCount(int iType)
{
    return 0;
}

void SEASON3B::CNewUIItemHotKey::SetHotKey(int iHotKey, int iItemType, int iItemLevel)
{
    if (iHotKey != -1 && CNewUIMyInventory::CanRegisterItemHotKey(iItemType) == true
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

int SEASON3B::CNewUIItemHotKey::GetHotKey(int iHotKey)
{
    if (iHotKey != -1)
    {
        return m_iHotKeyItemType[iHotKey];
    }

    return -1;
}

int SEASON3B::CNewUIItemHotKey::GetHotKeyLevel(int iHotKey)
{
    if (iHotKey != -1)
    {
        return m_iHotKeyItemLevel[iHotKey];
    }

    return 0;
}

void SEASON3B::CNewUIItemHotKey::RenderItems()
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

void SEASON3B::CNewUIItemHotKey::RenderItemCount()
{
    float x, y, width, height;

    for (int i = 0; i < HOTKEY_COUNT; ++i)
    {
        int iCount = GetHotKeyItemIndex(i, true);
        if (iCount > 0)
        {
            x = 30 + (i * 38); y = 457; width = 8; height = 9;
            SEASON3B::RenderNumber(x, y, iCount);
        }
    }
}

void SEASON3B::CNewUIItemHotKey::UseItemRButton()
{
    int x, y, width, height;

    for (int i = 0; i < HOTKEY_COUNT; ++i)
    {
        x = 10 + (i * 38); y = 445; width = 20; height = 20;
        if (SEASON3B::CheckMouseIn(x, y, width, height) == true)
        {
            if (MouseRButtonPush)
            {
                MouseRButtonPush = false;
                int iIndex = GetHotKeyItemIndex(i);
                if (iIndex != -1)
                {
                    SendRequestUse(iIndex, 0);
                    break;
                }
            }
        }
    }
}

SEASON3B::CNewUISkillList::CNewUISkillList()
{
    m_pNewUIMng = NULL;
    Reset();
}

SEASON3B::CNewUISkillList::~CNewUISkillList()
{
    Release();
}

bool SEASON3B::CNewUISkillList::Create(CNewUIManager* pNewUIMng, CNewUI3DRenderMng* pNewUI3DRenderMng)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_SKILL_LIST, this);

    m_pNewUI3DRenderMng = pNewUI3DRenderMng;

    LoadImages();

    Show(true);

    return true;
}

void SEASON3B::CNewUISkillList::Release()
{
    // 2026-09-02, Phase 2: the UI2DEffectObject registration/DeleteUI2DEffectObject() call here
    // is removed along with UI2DEffectCallback/RenderSkillInfo() -- the tooltip no longer queues
    // through CNewUI3DRenderMng's 2D-effect-in-3D-pass mechanism at all now that it's a plain
    // RmlUi element (which always composites last in the frame regardless -- see README.md's
    // Frame Lifecycle section), so there's nothing registered on this object to unregister.
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void SEASON3B::CNewUISkillList::Reset()
{
    m_bSkillList = false;
    m_bHotKeySkillListUp = false;

    for (int i = 0; i < SKILLHOTKEY_COUNT; ++i)
    {
        m_iHotKeySkillType[i] = -1;
    }

    // Phase 2 additions, replacing the removed m_bRenderSkillInfo/m_iRenderSkillInfoType/PosX/PosY
    // and m_EventState resets above.
    m_GridSnapshot.clear();
    m_PetSnapshot.clear();
    m_bTooltipPending = false;
    m_iTooltipSkillIndex = -1;
    m_fTooltipAnchorX = 0.f;
    m_fTooltipAnchorY = 0.f;
    m_iHoveredGridSkillIndex = -1;
}

void SEASON3B::CNewUISkillList::LoadImages()
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

void SEASON3B::CNewUISkillList::UnloadImages()
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

bool SEASON3B::CNewUISkillList::UpdateMouseEvent()
{
    // RmlUi migration, Phase 2 (see docs/rmlui-ui-system's Phase 2 plan) -- the old hand-rolled
    // EVENT_STATE hover/down/release machine for the current-skill icon, compact hotkey row, and
    // expanded grid/pet row is retired entirely. RmlUi's own Context now does hit-testing for all
    // of them (data-event-click/mouseover/mouseout, main_frame.rml -- see OnHotkeySlotClick()/
    // OnCurrentSkillClick()/OnGridCellClick()/OnPetCellClick() and their *Hover() counterparts),
    // same "always not consumed" convention every other fully-ported CNewUIObj-tier widget uses
    // (newui-tier-adapter.md).
    return true;
}

bool SEASON3B::CNewUISkillList::UpdateKeyEvent()
{
    for (int i = 0; i < 9; ++i)
    {
        if (SEASON3B::IsPress('1' + i))
        {
            UseHotKey(i + 1);
        }
    }

    if (SEASON3B::IsPress('0'))
    {
        UseHotKey(0);
    }

    // 2026-09-02, Phase 2: m_iHoveredGridSkillIndex replaces EVENT_STATE==EVENT_BTN_HOVER_SKILLLIST
    // as the "Ctrl+digit assignment armed" signal -- set by OnGridCellHover()/OnPetCellHover(),
    // cleared by OnUnhover(). Same Ctrl+digit-while-hovering-the-grid assignment behavior as
    // before, unaffected by the EVENT_STATE removal.
    if (m_iHoveredGridSkillIndex != -1)
    {
        if (SEASON3B::IsRepeat(VK_CONTROL))
        {
            for (int i = 0; i < 9; ++i)
            {
                if (SEASON3B::IsPress('1' + i))
                {
                    SetHotKey(i + 1, m_iHoveredGridSkillIndex);

                    return false;
                }
            }

            if (SEASON3B::IsPress('0'))
            {
                SetHotKey(0, m_iHoveredGridSkillIndex);

                return false;
            }
        }
    }

    if (SEASON3B::IsRepeat(VK_SHIFT))
    {
        for (int i = 0; i < 4; ++i)
        {
            if (SEASON3B::IsPress('1' + i))
            {
                Hero->CurrentSkill = AT_PET_COMMAND_DEFAULT + i;
                return false;
            }
        }
    }

    return true;
}

bool SEASON3B::CNewUISkillList::IsArrayUp(BYTE bySkill)
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

bool SEASON3B::CNewUISkillList::IsArrayIn(BYTE bySkill)
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

void SEASON3B::CNewUISkillList::SetHotKey(int iHotKey, int iSkillType)
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

int SEASON3B::CNewUISkillList::GetHotKey(int iHotKey)
{
    return m_iHotKeySkillType[iHotKey];
}

int SEASON3B::CNewUISkillList::GetSkillIndex(int iSkillType)
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

void SEASON3B::CNewUISkillList::UseHotKey(int iHotKey)
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

bool SEASON3B::CNewUISkillList::Update()
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

    // 2026-09-02, Phase 2: refresh the RmlUi-facing overlay snapshot (GetGridSnapshot()/
    // GetPetSnapshot()) while the expanded grid is open -- see RebuildGridSnapshot()'s own
    // comment. Left stale (harmless, hidden via skill_grid_open) while closed.
    if (m_bSkillList)
    {
        RebuildGridSnapshot();
    }

    return true;
}

void SEASON3B::CNewUISkillList::RenderCurrentSkillAndHotSkillList()
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
                // 2026-09-01: audited 2026-09-02 against legacy-theme-modernization.md and kept
                // deliberately, same exception class as RenderLeftFrame()/RenderCenterFrame()'s
                // own GetActiveThemeName() checks just above in this file -- NOT an oversight, so
                // don't "fix" this by deleting the branch outright.
                //
                // Modern theme: #skill_slot_0..4 (main_frame.rml/.rcss) highlight the selected
                // slot with a bound CSS class instead (feedback: "seems to use the legacy sprite
                // outline ... change this to just programmatic outline"), synced every frame from
                // IsHotKeySlotCurrentSkill() below (same iIndex/pet logic as here). RmlUi always
                // composites last in the frame (README.md's Frame Lifecycle section), so that
                // outline will paint on top of this sprite regardless -- unconditionally drawing
                // IMAGE_SKILLBOX_USE for modern would show a doubled highlight (legacy sprite
                // underneath, RmlUi outline on top), reintroducing the exact "uses the legacy
                // sprite outline" look the feedback above asked to remove. This isn't a paint-
                // order *impossibility* (unlike RenderLeftFrame()'s background fill, which really
                // can't sit behind the 3D icons) -- it's a real, working design choice to suppress
                // the sprite for modern, kept as C++ only because a raw RenderBitmap() call has no
                // RmlUi-side element to gate from RCSS.
                //
                // Legacy theme: keeps the real sprite -- its own established look, matching
                // legacy-theme-modernization.md's "preserve legacy sprites/textures where
                // appropriate" (this is real pixel-parity art, not reproducible with a plain CSS
                // outline). main_frame.rml/.rcss (legacy) already bind #skill_slot_N's own
                // .selected class for symmetry with modern, but deliberately leave it unstyled
                // (see themes/legacy/main_frame.rcss's own comment) so only this sprite shows,
                // never a doubled highlight there either.
                //
                // Retiring this branch for real means porting IMAGE_SKILLBOX_USE's source art into
                // an RmlUi @spritesheet for legacy theme's own .selected rule (theming-and-
                // modding.md's asset-reuse pattern) so legacy no longer needs this C++ draw call
                // at all -- real work, not a quick fix, and belongs with the rest of Phase 2
                // (CNewUISkillList -> RmlUi, STATUS.md's "What's migrated") rather than bundled
                // into an unrelated cleanup pass.
                if (UI::RmlBridge::GetActiveThemeName() != "modern")
                    SEASON3B::RenderImage(IMAGE_SKILLBOX_USE, x, y, width, height);
            }
            RenderSkillIcon(m_iHotKeySkillType[iIndex], x + 6, y + 6, 20, 28);
        }

        x = 392; y = 437; width = 20; height = 28;
        RenderSkillIcon(Hero->CurrentSkill, x, y, width, height);
    }
}

bool SEASON3B::CNewUISkillList::IsHotKeySlotCurrentSkill(int iSlotIndex)
{
    // Mirrors RenderCurrentSkillAndHotSkillList()'s own loop exactly (iStartSkillIndex/iIndex
    // wraparound, the -1 "empty slot" check, the pet-command "no pet -> treat as empty" check) --
    // see that function's own comment for why this duplicates rather than shares the math.
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

int SEASON3B::CNewUISkillList::GetHotKeySlotNumber(int iSlotIndex)
{
    // Mirrors RenderCurrentSkillAndHotSkillList()'s own loop exactly (iStartSkillIndex/iIndex
    // wraparound, the -1 "empty slot" check, the pet-command "no pet -> treat as empty" check) --
    // same duplication precedent as IsHotKeySlotCurrentSkill() just above. Unlike that function,
    // this doesn't also check Hero->CurrentSkill -- the number shows for every occupied slot, not
    // just the currently-active one (matching RenderSkillIcon()'s own RenderNumber() call, which
    // draws for every icon it renders, not only the equipped skill's).
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

bool SEASON3B::CNewUISkillList::Render()
{
    BYTE bySkillNumber = CharacterAttribute->SkillNumber;

    // 2026-09-02: same missing-transform bug UpdateMouseEvent() used to have (see git history) --
    // this is the expanded skill grid, ALSO registered directly with CNewUIManager and called by
    // its generic, untransformed Render() dispatch, so it was rendering under the global
    // LegacyUiTransform baseline instead of the centerTransform (+GetSkillListOffsetX()) the
    // compact hotkey row it expands from actually uses (RenderCurrentSkillAndHotSkillList(), via
    // CNewUIMainFrameWindow::RenderCenterRegion()) -- meaning the expanded grid would visually
    // misalign from that row at any non-4:3 resolution. Scoped to the WHOLE function (single,
    // unconditional block) since this simpler function has no early returns to worry about.
    // `* transform.scaleX` -- see Render()'s own comment on this bug.
    auto transform = UI::Scaling::BottomHudCenterTransform(WindowWidth, WindowHeight);
    transform.offsetX += g_pMainFrame->GetSkillListOffsetX() * transform.scaleX;
    UI::Scaling::ScopedActiveTransform layout(transform, true);

    // 2026-09-02, Phase 2 (mid-implementation scope adjustment -- see NewUIMainFrameWindow.h's
    // SkillCellEntry comment): icon art stays legacy 2D for both themes (the atlas lookup is too
    // irregular to port blind). The box-FRAME sprite (IMAGE_SKILLBOX/IMAGE_SKILLBOX_USE) is
    // different -- same audited exception already applied to the compact row's selected-highlight
    // sprite (RenderCurrentSkillAndHotSkillList(), see that call site's own comment): modern theme
    // suppresses it entirely, relying on .skill-cell's own programmatic border + .selected
    // highlight (main_frame.rcss) instead, matching the hotkey slots' look (feedback: "the skill
    // list are using the legacy border instead of the programmatic border already used in the
    // skill hotkeys slots"). Legacy theme keeps the real sprite, its own established look. This
    // loop now just DRAWS from m_GridSnapshot/m_PetSnapshot (positions/skill indices/current-
    // selected already computed by Update()'s RebuildGridSnapshot() call) instead of also
    // computing them itself -- hit-testing/tooltip-queueing (the rest of the old Render() body)
    // moved to RmlUi entirely, see OnGridCellClick()/OnGridCellHover() etc.
    if (bySkillNumber > 0 && m_bSkillList == true)
    {
        const bool bDrawBoxSprite = (UI::RmlBridge::GetActiveThemeName() != "modern");
        for (const SkillCellEntry& entry : m_GridSnapshot)
        {
            if (bDrawBoxSprite)
                SEASON3B::RenderImage(entry.isCurrent ? IMAGE_SKILLBOX_USE : IMAGE_SKILLBOX, entry.left, entry.top, 32.f, 38.f);
            RenderSkillIcon(entry.skillIndex, entry.left + 6.f, entry.top + 6.f, 20.f, 28.f);
        }
        for (const SkillCellEntry& entry : m_PetSnapshot)
        {
            if (bDrawBoxSprite)
                SEASON3B::RenderImage(entry.isCurrent ? IMAGE_SKILLBOX_USE : IMAGE_SKILLBOX, entry.left, entry.top, 32.f, 38.f);
            RenderSkillIcon(entry.skillIndex, entry.left + 6.f, entry.top + 6.f, 20.f, 28.f);
        }
    }

    return true;
}

float SEASON3B::CNewUISkillList::GetLayerDepth()
{
    return 5.2f;
}

WORD SEASON3B::CNewUISkillList::GetHeroPriorSkill()
{
    return m_wHeroPriorSkill;
}

void SEASON3B::CNewUISkillList::SetHeroPriorSkill(BYTE bySkill)
{
    m_wHeroPriorSkill = bySkill;
}

void SEASON3B::CNewUISkillList::RenderSkillIcon(int iIndex, float x, float y, float width, float height)
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

    // 2026-09-02: the hotkey-number subscript this used to draw here (RenderNumber(x+20, y+20,
    // ...), a search through m_iHotKeySkillType[] for this icon's own slot) is retired -- both
    // themes now show it through RmlUi instead (#skill_slot_0..4's .skill-hotkey-label,
    // main_frame.rml/.rcss, bound from CNewUISkillList::GetHotKeySlotNumber() every frame),
    // matching feedback ("move the legacy subscripts from C++ code to RmlUi scope too ... the
    // legacy theme will rely on rml instead of hardcoded behavior") -- no theme check needed here
    // at all now since neither theme's C++ path draws it any more.

    if ((bySkillType == AT_SKILL_CHAIN_DRIVE
        || bySkillType == AT_SKILL_CHAIN_DRIVE_STR
        || bySkillType == AT_SKILL_DRAGON_KICK
        || bySkillType == AT_SKILL_DRAGON_ROAR
        || bySkillType == AT_SKILL_DRAGON_ROAR_STR) && (bCantSkill))
        return;

    // 2026-09-02, Phase 2: RenderSkillDelay() call removed here -- the cooldown wipe moves to
    // RmlUi (SkillCellEntry::cooldownFraction / ComputeSkillCooldownFraction(), computed
    // independently in RebuildGridSnapshot()/GetHotKeySlotCooldownFraction()/
    // GetCurrentSkillCooldownFraction() -- see those for the exact same fraction math and the
    // same 5-skill exclusion this tail condition used to gate).
}

namespace
{
    // 2026-09-02, Phase 2: RenderSkillDelay()'s own fraction math (iSkillDelay/iSkillMaxDelay),
    // with its exact draw call replaced by a plain return -- see the gate/internal-resolution
    // split comment below for why this isn't quite a 1-line change. Feeds
    // SkillCellEntry::cooldownFraction (RebuildGridSnapshot()) and
    // GetHotKeySlotCooldownFraction()/GetCurrentSkillCooldownFraction().
    //
    // Known, deliberately accepted gap: the original RenderSkillIcon() additionally suppressed
    // RenderSkillDelay() for AT_SKILL_CHAIN_DRIVE/_STR, AT_SKILL_DRAGON_KICK, AT_SKILL_DRAGON_ROAR/
    // _STR specifically when bCantSkill was also true (an early `return;` before reaching the
    // delay call, RenderSkillIcon()'s own tail) -- reproducing that here would need this whole
    // ~200-line eligibility block re-derived a second time for 4 skills' rare double-condition.
    // Not reproduced: the cooldown wipe may show for those 4 skills even while bCantSkill is also
    // true, a minor double-signal, not a functional bug -- revisit only if this proves visible.
    float ComputeSkillCooldownFraction(int iIndex)
    {
        // Mirrors RenderSkillIcon()'s own bySkillType resolution -- used only to decide whether the
        // delay overlay applies at all (the 5-skill exclusion list), matching that function's own
        // gate before it used to call RenderSkillDelay(). RenderSkillDelay() itself, preserved
        // below, resolves its OWN (non-pet-aware) skill type from CharacterAttribute->Skill[iIndex]
        // directly -- a pre-existing inconsistency between the two original functions, preserved
        // exactly rather than "fixed" here.
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

void SEASON3B::CNewUISkillList::RebuildGridSnapshot()
{
    // 2026-09-02, Phase 2: same iteration/filter/zig-zag-position math the legacy grid-drawing
    // loop always used (see git history's old Render()), now producing data instead of drawing.
    // Render() (still-legacy icon/box art) iterates the resulting snapshot instead of recomputing
    // positions itself -- computed once, read by both.
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

void SEASON3B::CNewUISkillList::QueueTooltip(int iSkillIndex, float x, float y)
{
    m_bTooltipPending = true;
    m_iTooltipSkillIndex = iSkillIndex;
    m_fTooltipAnchorX = x;
    m_fTooltipAnchorY = y;
}

float SEASON3B::CNewUISkillList::GetHotKeySlotCooldownFraction(int iSlotIndex)
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

float SEASON3B::CNewUISkillList::GetCurrentSkillCooldownFraction()
{
    return ComputeSkillCooldownFraction(Hero->CurrentSkill);
}

// 2026-09-02, Phase 2: click/hover entry points bound from main_frame.rml's data-event-click/
// mouseover/mouseout (Create(), CNewUIMainFrameWindow.cpp) -- see each one's own comment for the
// exact legacy mouse-click/hover behavior preserved (traced from the retired EVENT_STATE machine,
// see git history's old UpdateMouseEvent()).
void SEASON3B::CNewUISkillList::OnHotkeySlotClick(int iSlotIndex)
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

    // Mirrors the legacy EVENT_BTN_DOWN_SKILLHOTKEY release branch exactly -- deliberately NOT
    // UseHotKey(), which the mouse click never went through even before this port (only the
    // keyboard 0-9 press does). UseHotKey() additionally applies a pet-ownership check and the
    // teleport/auto-attack-cancel rule that the original mouse click never applied; preserved
    // faithfully here, not "fixed"/unified with the keyboard path.
    WORD bySkillType = CharacterAttribute->Skill[m_iHotKeySkillType[iIndex]];
    if (bySkillType == 0 || (bySkillType >= AT_SKILL_STUN && bySkillType <= AT_SKILL_REMOVAL_BUFF))
        return;
    if (SkillAttribute[bySkillType].SkillUseType == SKILL_USE_TYPE_MASTERLEVEL)
        return;

    m_wHeroPriorSkill = CharacterAttribute->Skill[Hero->CurrentSkill];
    Hero->CurrentSkill = m_iHotKeySkillType[iIndex];
    PlayBuffer(SOUND_CLICK01);
}

void SEASON3B::CNewUISkillList::OnHotkeySlotHover(int iSlotIndex)
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

    // Anchor: local x of this slot in #bars's own reference space -- matches
    // RenderCurrentSkillAndHotSkillList()'s own x for the same slot (190 + (iSlotIndex+1)*32).
    QueueTooltip(m_iHotKeySkillType[iIndex], 190.f + (iSlotIndex + 1) * 32.f, 431.f);
}

void SEASON3B::CNewUISkillList::OnCurrentSkillClick()
{
    m_bSkillList = !m_bSkillList;
    PlayBuffer(SOUND_CLICK01);
}

void SEASON3B::CNewUISkillList::OnCurrentSkillHover()
{
    QueueTooltip(Hero->CurrentSkill, 392.f, 437.f);
}

void SEASON3B::CNewUISkillList::OnGridCellClick(int iSkillIndex)
{
    m_wHeroPriorSkill = CharacterAttribute->Skill[Hero->CurrentSkill];
    Hero->CurrentSkill = iSkillIndex;
    m_bSkillList = false;
    PlayBuffer(SOUND_CLICK01);
}

void SEASON3B::CNewUISkillList::OnGridCellHover(int iSkillIndex)
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

void SEASON3B::CNewUISkillList::OnPetCellClick(int iSkillIndex)
{
    m_wHeroPriorSkill = CharacterAttribute->Skill[Hero->CurrentSkill];
    Hero->CurrentSkill = iSkillIndex;
    m_bSkillList = false;
    PlayBuffer(SOUND_CLICK01);
}

void SEASON3B::CNewUISkillList::OnPetCellHover(int iSkillIndex)
{
    // 2026-09-02: pet-row entries arm Ctrl+digit assignment the same way grid entries do -- the
    // legacy code's own m_EventState machine never distinguished the two loops for this purpose
    // (both set m_iRenderSkillInfoType on hover, UpdateKeyEvent()'s check didn't care which loop
    // set it); preserved, not a new capability.
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

void SEASON3B::CNewUISkillList::OnUnhover()
{
    m_bTooltipPending = false;
    m_iTooltipSkillIndex = -1;
    m_iHoveredGridSkillIndex = -1;
}

bool SEASON3B::CNewUISkillList::IsSkillListUp()
{
    return m_bHotKeySkillListUp;
}

void SEASON3B::CNewUISkillList::ResetMouseLButton()
{
    MouseLButton = false;
    MouseLButtonPop = false;
    MouseLButtonPush = false;
}

void SEASON3B::CNewUIMainFrameWindow::SetPreExp_Wide(__int64 dwPreExp)
{
    m_loPreExp = dwPreExp;
}

void SEASON3B::CNewUIMainFrameWindow::SetGetExp_Wide(__int64 dwGetExp)
{
    m_loGetExp = dwGetExp;

    if (m_loGetExp > 0)
    {
        m_bExpEffect = true;
        m_dwExpEffectTime = timeGetTime();
    }
}

void SEASON3B::CNewUIMainFrameWindow::SetPreExp(__int64 dwPreExp)
{
    m_dwPreExp = dwPreExp;
}

void SEASON3B::CNewUIMainFrameWindow::SetGetExp(__int64 dwGetExp)
{
    m_dwGetExp = dwGetExp;

    if (m_dwGetExp > 0)
    {
        m_bExpEffect = true;
        m_dwExpEffectTime = timeGetTime();
    }
}


void SEASON3B::CNewUIMainFrameWindow::SetBtnState(int iBtnType, bool bStateDown)
{
    // Sets a bound "open" model boolean instead of swapping legacy CNewUIButton sprite frames --
    // see this class's header comment. main_frame.rcss selects the "panel open" sprite rect when
    // the corresponding *_open field is true.
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

void SEASON3B::CNewUIMainFrameWindow::SyncDocVisibility(bool sceneAllowsShow)
{
    if (!m_pRmlDoc) return;

    if (IsVisible() && sceneAllowsShow)
        m_pRmlDoc->Show();
    else
        m_pRmlDoc->Hide();
}
