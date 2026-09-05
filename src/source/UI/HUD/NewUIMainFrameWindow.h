// NewUIMainFrameWindow.h: interface for the CNewUIMainFrameWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIMAINFRAMEWINDOW_H__46A029CA_44A5_4050_9216_FA8A25EC4629__INCLUDED_)
#define AFX_NEWUIMAINFRAMEWINDOW_H__46A029CA_44A5_4050_9216_FA8A25EC4629__INCLUDED_

#pragma once

#include <vector>

#include "UI/Core/NewUIBase.h"
#include "Render/Textures/ZzzTexture.h"
#include "UI/Core/NewUI3DRenderMng.h"
#include "UI/RmlBridge/RmlModelBinder.h"

namespace Rml { class ElementDocument; }

namespace SEASON3B
{
    enum
    {
        HOTKEY_Q = 0,
        HOTKEY_W,
        HOTKEY_E,
        HOTKEY_R,
        HOTKEY_COUNT
    };

    enum
    {
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
        MAINFRAME_BTN_PARTCHARGE = 0,
#endif //defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
        MAINFRAME_BTN_CHAINFO,
        MAINFRAME_BTN_MYINVEN,
        MAINFRAME_BTN_FRIEND,
        MAINFRAME_BTN_WINDOW,
    };

    enum KINDOFSKILL
    {
        KOS_COMMAND = 1,
        KOS_SKILL1,
        KOS_SKILL2,
        KOS_SKILL3,
    };

    class CNewUIItemHotKey
    {
    public:
        CNewUIItemHotKey();
        virtual ~CNewUIItemHotKey();

        bool UpdateKeyEvent();

        void SetHotKey(int iHotKey, int iItemType, int iItemLevel);
        int GetHotKey(int iHotKey);
        int GetHotKeyLevel(int iHotKey);
        void UseItemRButton();
        void RenderItems();
        void RenderItemCount();

    private:
        int GetHotKeyItemIndex(int iType, bool bItemCount = false);
        bool GetHotKeyCommonItem(IN int iHotKey, OUT int& iStart, OUT int& iEnd);
        int GetHotKeyItemCount(int iType);

        int m_iHotKeyItemType[HOTKEY_COUNT];
        int m_iHotKeyItemLevel[HOTKEY_COUNT];
    };

    // RmlUi migration, Phase 2 of 3 of the CNewUIMainFrameWindow pilot (see
    // docs/rmlui-ui-system/STATUS.md). One interactive overlay cell in the expanded skill grid or
    // pet-command row -- position/eligibility/cooldown computed fresh in
    // CNewUISkillList::Update() every frame the grid is open. Icon/box-frame ART IS NOT PART OF
    // THIS STRUCT and stays a legacy 2D draw at the same position (mid-implementation scope
    // adjustment, see the Phase 2 plan: RenderSkillIcon()'s atlas lookup is too irregular -- mixed
    // 8/12-column addressing, a separate master-level atlas -- to port blind without a way to
    // visually verify against the real decoded textures). This struct only drives RmlUi's
    // interactive overlay: hit target (click/hover replacing the old EVENT_STATE machine),
    // cooldown wipe, and (modern theme) selection highlight.
    struct SkillCellEntry
    {
        float left = 0.f, top = 0.f;   // px, in #bars's own local reference space -- matches the
                                        // legacy zig-zag/pet-row position exactly (same values the
                                        // still-legacy icon/box draw uses for this same cell)
        int skillIndex = -1;           // CharacterAttribute->Skill[] index (grid) or the raw
                                        // AT_PET_COMMAND_* enum value (pet row) -- passed back
                                        // verbatim to the click handler, matching the legacy click
                                        // branches' own `Hero->CurrentSkill = i` (see
                                        // CNewUISkillList::OnGridCellClick()'s comment for why this
                                        // does NOT go through UseHotKey())
        bool isPet = false;            // true for pet-row entries -- click routes to the pet path
        bool isCurrent = false;        // Hero->CurrentSkill == skillIndex
        float cooldownFraction = 0.f;  // 0 = ready; shrinks toward 0 as the skill's delay counts
                                        // down -- same bottom-anchored-wipe intent as
                                        // RenderSkillDelay()'s retired ARGB-quad draw
    };

    // One line of a skill tooltip. Field-for-field mirror of UI::Skills::Tooltip::Line
    // (SkillTooltipModel.h) so CNewUIMainFrameWindow::SyncRmlModel() can copy directly -- kept as
    // its own RmlUi-facing type rather than reusing Line itself since Rml::DataModelConstructor
    // needs a bindable Rml::String, not Line's fixed wchar_t[] buffer.
    struct SkillTooltipLineEntry
    {
        Rml::String text;
        // Discrete bool-per-color flags, not a class-name string -- matches this codebase's own
        // established data-class-* convention (data-class-poisoned/open/selected elsewhere in
        // this same file) rather than introducing string-equality expressions into RML, which no
        // other window here has ever needed. White (UI::Skills::Tooltip::LineColor::White) is the
        // implicit default: no flag true -> themes/{legacy,modern}/main_frame.rcss's plain
        // .tt-line text color applies.
        bool colorBlue = false;
        bool colorRed = false;
        bool colorDarkRed = false;
        bool bold = false;
    };

    class CNewUISkillList : public CNewUIObj
    {
        enum
        {
            SKILLHOTKEY_COUNT = 10
        };

    public:
        enum IMAGE_LIST
        {
            IMAGE_SKILL1 = BITMAP_INTERFACE_NEW_SKILLICON_BEGIN,
            IMAGE_SKILL2,
            IMAGE_COMMAND,
            IMAGE_SKILL3,
            IMAGE_SKILLBOX,
            IMAGE_SKILLBOX_USE,
            IMAGE_NON_SKILL1,
            IMAGE_NON_SKILL2,
            IMAGE_NON_COMMAND,
            IMAGE_NON_SKILL3,
        };

        CNewUISkillList();
        virtual ~CNewUISkillList();

        bool Create(CNewUIManager* pNewUIMng, CNewUI3DRenderMng* pNewUI3DRenderMng);
        void Release();

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();
        float GetLayerDepth();		// 10.6f

        WORD GetHeroPriorSkill();
        void SetHeroPriorSkill(BYTE bySkill);

        void Reset();

        void SetHotKey(int iHotKey, int iSkillType);
        int GetHotKey(int iHotKey);
        int GetSkillIndex(int iSkillType);
        void RenderCurrentSkillAndHotSkillList();

        // 2026-09-01: exposes RenderCurrentSkillAndHotSkillList()'s own "is this hotkey-row slot
        // (0-4) currently showing the equipped/active skill" check to CNewUIMainFrameWindow's
        // SyncRmlModel() (main_frame.rml's #skill_slot_0..4), so modern theme can highlight the
        // selection with a bound CSS class instead of the legacy IMAGE_SKILLBOX_USE sprite
        // (feedback: "the selected skill indicator ... seems to use the legacy sprite outline ...
        // change this to just programmatic outline"). Duplicates that function's own iIndex/pet
        // math rather than refactoring it out -- matches this file's existing style of small
        // near-identical index blocks (see the 3 IMAGE_SKILLBOX_USE call sites).
        bool IsHotKeySlotCurrentSkill(int iSlotIndex);

        // 2026-09-02: exposes the same hotkey-number RenderSkillIcon()'s own RenderNumber(x+20,
        // y+20, iHotKey) call already computes (search m_iHotKeySkillType[] for this slot's own
        // index -- reduces to iIndex itself barring duplicate assignments) to
        // CNewUIMainFrameWindow::SyncRmlModel(), so modern theme can show it as a
        // .slot-hotkey-label span (upper-left corner, main_frame.rml/.rcss) instead of the legacy
        // digit-sprite subscript in RenderSkillIcon() (feedback: "move the assigned number
        // shortcut ... to the upper left corner too, same as Q W E R"). Returns -1 for an empty
        // slot (m_iHotKeySkillType[iIndex] == -1) -- SyncRmlModel() turns that into an empty
        // label string, same "nothing bound, draw nothing" behavior RenderCurrentSkillAndHotSkillList()'s
        // own `continue` already has for the legacy path.
        int GetHotKeySlotNumber(int iSlotIndex);

        // 2026-09-02, Phase 2: per-slot cooldown fraction for the compact hotkey row + current-
        // skill slot (0 == ready). Mirrors RenderSkillDelay()'s own iSkillDelay/iSkillMaxDelay
        // fraction, computed independently per slot (same "duplicate rather than refactor" style
        // as the two getters above) since RenderSkillDelay() itself takes an icon index, not a
        // hotkey-slot index, and the two don't share a convenient common call shape.
        float GetHotKeySlotCooldownFraction(int iSlotIndex);
        float GetCurrentSkillCooldownFraction();

        // Despite the name, this reports whether the compact hotkey row is scrolled to its "upper"
        // set (6-9,0 vs 1-5, m_bHotKeySkillListUp) -- a legacy naming trap, NOT whether the
        // expanded grid popup is open. Only ever correct for RenderCenterFrame()'s own
        // "IMAGE_MENU_2_1 highlight" trigger (NewUIMainFrameWindow.cpp), which predates this pilot
        // and was never about the grid either. Use IsSkillGridOpen() below for the grid's own
        // open/closed state -- a 2026-09-02 bug (main_frame.rml's #skill_grid/#pet_skill_row bound
        // to THIS method instead) meant clicking the current-skill slot never visibly opened the
        // grid; SyncRmlModel() now binds skill_grid_open to IsSkillGridOpen() instead.
        bool IsSkillListUp();

        // 2026-09-02, Phase 2: the actual "is the expanded skill grid popup open" state
        // (m_bSkillList) -- see IsSkillListUp()'s own comment for why that similarly-named,
        // pre-existing method is NOT this.
        bool IsSkillGridOpen() const { return m_bSkillList; }

        // 2026-09-02, Phase 2: click/hover entry points bound from main_frame.rml's
        // data-event-click/mouseover/mouseout (Create(), CNewUIMainFrameWindow.cpp) -- replace the
        // old EVENT_STATE hover/down/release machine entirely (RmlUi's own Context now does
        // hit-testing). Each pair mirrors one of the 3 widgets UpdateMouseEvent() used to gate
        // sequentially; see each .cpp definition's own comment for the exact legacy behavior
        // preserved (which is NOT always the same as UseHotKey()'s keyboard-press path -- the
        // original mouse-click branches never called UseHotKey() and so never applied its
        // pet-check/auto-attack-cancel-on-teleport rule; preserved faithfully, not "fixed").
        void OnHotkeySlotClick(int iSlotIndex);
        void OnHotkeySlotHover(int iSlotIndex);
        void OnCurrentSkillClick();
        void OnCurrentSkillHover();
        void OnGridCellClick(int iSkillIndex);
        void OnGridCellHover(int iSkillIndex);
        void OnPetCellClick(int iSkillIndex);
        void OnPetCellHover(int iSkillIndex);
        void OnUnhover();

        // 2026-09-02, Phase 2: the two dynamic-count overlay lists CNewUIMainFrameWindow::
        // SyncRmlModel() copies into the shared main_frame RmlUi model's skill_grid_cells/
        // pet_skill_cells arrays every frame. Populated by Update() (while the grid is open) from
        // the exact same position/filter logic Render()'s still-legacy icon/box draw uses for the
        // same cells -- computed once, read by both, not duplicated (unlike the small getters
        // above, this one's genuinely shared by two call sites within this same class).
        const std::vector<SkillCellEntry>& GetGridSnapshot() const { return m_GridSnapshot; }
        const std::vector<SkillCellEntry>& GetPetSnapshot() const { return m_PetSnapshot; }

        // 2026-09-02, Phase 2: true whenever a hover callback above has a tooltip queued for
        // SyncRmlModel() to copy this frame -- consumed (not cleared) every frame like every other
        // polled field in this pilot; cleared only by OnUnhover()/a new hover target replacing it.
        bool IsTooltipPending() const { return m_bTooltipPending; }
        int GetTooltipSkillIndex() const { return m_iTooltipSkillIndex; }
        float GetTooltipAnchorX() const { return m_fTooltipAnchorX; }
        float GetTooltipAnchorY() const { return m_fTooltipAnchorY; }

    private:
        void LoadImages();
        void UnloadImages();
        bool IsArrayUp(BYTE bySkill);
        bool IsArrayIn(BYTE bySkill);
        void UseHotKey(int iHotKey);

        void RenderSkillIcon(int iIndex, float x, float y, float width, float height);
        // RenderSkillDelay()/RenderPetSkill() removed, Phase 2 -- see RebuildGridSnapshot()'s own
        // comment (cooldown math lives in the free function ComputeSkillCooldownFraction() now,
        // NewUIMainFrameWindow.cpp; pet-row drawing is inlined into Render() directly).

        // 2026-09-02, Phase 2: rebuilds m_GridSnapshot/m_PetSnapshot from the current
        // m_bSkillList/pet state -- same iteration/filter/zig-zag-position math the legacy grid
        // loop always used (CharacterAttribute->Skill[] scan, buff-range + MASTERLEVEL skip,
        // AT_PET_COMMAND_DEFAULT..END for the pet row), called once per frame from Update() while
        // the grid is open. Render() (still-legacy icon/box art) iterates the resulting snapshot
        // instead of recomputing positions itself.
        void RebuildGridSnapshot();

        // 2026-09-02, Phase 2: queues a tooltip for the given skill/pet-command index, anchored at
        // (x, y) in #bars's own local reference space -- shared by all 5 hover entry points above
        // rather than duplicated per widget, since the actual tooltip content build
        // (UI::Skills::Tooltip::BuildModel(), pet-command dispatch) is identical regardless of
        // which widget triggered it.
        void QueueTooltip(int iSkillIndex, float x, float y);

        void ResetMouseLButton();

    private:
        CNewUIManager* m_pNewUIMng;
        CNewUI3DRenderMng* m_pNewUI3DRenderMng;

        bool m_bHotKeySkillListUp;
        int m_iHotKeySkillType[SKILLHOTKEY_COUNT];

        bool m_bSkillList;

        WORD m_wHeroPriorSkill;

        // Phase 2 additions -- replace EVENT_STATE (removed) and m_bRenderSkillInfo/
        // m_iRenderSkillInfoType/PosX/PosY (removed, see QueueTooltip()'s comment).
        std::vector<SkillCellEntry> m_GridSnapshot;
        std::vector<SkillCellEntry> m_PetSnapshot;

        bool m_bTooltipPending = false;
        int m_iTooltipSkillIndex = -1;
        float m_fTooltipAnchorX = 0.f, m_fTooltipAnchorY = 0.f;

        // 2026-09-02: replaces EVENT_STATE's EVENT_BTN_HOVER_SKILLLIST as the "is Ctrl+digit
        // assignment armed" signal for UpdateKeyEvent()'s SetHotKey() path -- set by
        // OnGridCellHover(), cleared by OnUnhover()/a different hover target. -1 == nothing
        // hovered in the grid right now.
        int m_iHoveredGridSkillIndex = -1;
    };

    // RmlUi migration (2026-09-01) -- Phase 1 of 3 of the third CNewUIObj-tier pilot (see
    // docs/rmlui-ui-system/STATUS.md's "What's migrated"). This legacy file actually welds three
    // classes together: this one (frame chrome + HP/MP/AG/SD/EXP bars + 5 corner buttons -- the
    // part this pilot ports), CNewUISkillList (skill hotkey row/grid/pet commands, still fully
    // legacy, Phase 2), and CNewUIItemHotKey (QWER item slots, still fully legacy -- its icons are
    // 3D-rendered meshes composited via a separate Render3D()/INewUI3DRenderObj camera pass with
    // no RmlUi equivalent, Phase 3). See the "Third CNewUIObj pilot" plan for the full rationale.
    //
    // Render() is now a *thin passthrough*, not a full no-op like CMuHelperBar/CBuffStrip -- this
    // window is the first case where out-of-scope legacy content (the skill hotkey row/current-
    // skill icon, and the item-hotkey stack-count text) shares the exact same screen region as
    // content this pilot ports. RmlUi always paints last in the frame (SetPreSubmitCallback), so
    // moving the *shared* center-band background chrome to RmlUi would draw it on top of (occlude)
    // the still-legacy skill row painted earlier in the same frame. Render() therefore keeps
    // calling RenderLeftFrame()/RenderCenterFrame() (background chrome for the two regions that
    // still host legacy content) and RenderLeftRegion()/g_pSkillList->RenderCurrentSkillAndHot-
    // SkillList() (the legacy content itself) exactly as before -- only RenderRightFrame()/
    // RenderExperienceBackground() (chrome for regions with NO remaining legacy content) and
    // RenderButtons()/RenderLifeMana()/RenderGuageAG()/RenderGuageSD()/RenderExperience() (the
    // parts this pilot actually ports) are removed, their C++ implementations deleted rather than
    // kept-but-unused (matches CMuHelperBar's precedent for dead legacy-button machinery).
    //
    // UpdateMouseEvent()/UpdateKeyEvent() stay real where they still gate legacy behavior:
    // UpdateMouseEvent() drops its BtnProcess() call entirely (RmlUi's own Context now does hit-
    // testing for the 5 corner buttons -- see RmlUiRuntime::IsMouseOverUI()) and always reports
    // "not consumed"; UpdateKeyEvent() is UNCHANGED (still gates the legacy CNewUIItemHotKey's
    // Q/W/E/R key handling, out of scope). Create()/Release()/GetLayerDepth()/GetKeyEventOrder()/
    // Render3D()/IsVisible() stay real. Update() still does real work every frame (gauge
    // fractions/colors, EXP digit/fraction, button open-state/alert-blink booleans) and pushes it
    // into the RmlUi model via MarkDirty().
    //
    // Deliberate simplifications (called out, not silently dropped -- see the plan's own list):
    // "gained EXP" 2s flash overlay not reproduced; HP/MP/AG/SD/EXP numeric readouts use plain
    // RCSS text instead of the legacy digit-sprite atlas; gauge fill uses a flat background-color
    // instead of the legacy gauge texture (RmlUi's `image()` decorator stretches-to-fit, which
    // would visibly distort a texture whose element height changes every frame -- the "clipped
    // oversized image" alternative is the exact technique CBuffStrip already proved doesn't clip
    // absolutely-positioned children in this RmlUi build); the 5 corner buttons reproduce only the
    // legacy sprite sheet's "normal" and "panel-open" frames (2 of the real 4: hover/pressed-hover
    // frame-swaps are replaced by a plain CSS brightness filter on :hover).
    class CNewUIMainFrameWindow : public CNewUIObj, public INewUI3DRenderObj
    {
    public:
        enum IMAGE_LIST
        {
            // Gauge/button textures (IMAGE_GAUGE_*, IMAGE_MENU_BTN_*) are no longer loaded via
            // this legacy CGlobalBitmap-backed enum -- RmlUi loads the same source art files
            // directly through its own render interface (see main_frame.rcss's @spritesheet
            // blocks), same split already established by every prior CNewUIObj-tier port.
            IMAGE_MENU_1 = BITMAP_INTERFACE_NEW_MAINFRAME_BEGIN,	// newui_menu01.jpg
            IMAGE_MENU_2,		// newui_menu02.jpg
            IMAGE_MENU_3,		// newui_menu03.jpg
            IMAGE_MENU_2_1,
        };

        CNewUIMainFrameWindow();
        virtual ~CNewUIMainFrameWindow();

        bool Create(CNewUIManager* pNewUIMng, CNewUI3DRenderMng* pNewUI3DRenderMng);
        void Release();

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();
        void Render3D();

        bool IsVisible() const;

        float GetLayerDepth();		// 10.2f
        float GetKeyEventOrder();	// 7.f

        void SetItemHotKey(int iHotKey, int iItemType, int iItemLevel);
        int GetItemHotKey(int iHotKey);
        int GetItemHotKeyLevel(int iHotKey);
        void UseHotKeyItemRButton();
        //void RenderHotKeyItems();
        void UpdateItemHotKey();

        void ResetSkillHotKey();
        void SetSkillHotKey(int iHotKey, int iSkillType);
        int GetSkillHotKey(int iHotKey);
        int GetSkillHotKeyIndex(int iSkillType);

        void SetPreExp_Wide(__int64 dwPreExp);
        void SetGetExp_Wide(__int64 dwGetExp);

        void SetPreExp(__int64 dwPreExp);
        void SetGetExp(__int64 dwGetExp);

        // buttons -- called externally (NewUISystem.cpp) whenever a button's target panel opens/
        // closes, to sync the button's "open" visual state. Now sets a bound model boolean instead
        // of swapping CNewUIButton sprite frames -- see this class's header comment.
        void SetBtnState(int iBtnType, bool bStateDown);

        static void UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB);

        // Invoked from the RmlUi document's data-event-click bindings (see Create()). Polled-and-
        // cleared exactly like every other migrated window's RmlClickX() pattern.
        void RmlClickCShop() { m_bRmlCShopClicked = true; }
        void RmlClickChaInfo() { m_bRmlChaInfoClicked = true; }
        void RmlClickMyInven() { m_bRmlMyInvenClicked = true; }
        void RmlClickFriend() { m_bRmlFriendClicked = true; }
        void RmlClickWindow() { m_bRmlWindowClicked = true; }

        // 2026-09-02: real-pixel X offset read from main_frame.rml's #item_hotkey_anchor/
        // #skill_list_anchor .layout-anchor markers (SyncRmlModel()) -- lets the still-legacy
        // item-hotkey (potion) and skill-hotkey bands' render AND click-hit-testing follow
        // wherever the active theme's own RCSS positions those markers, generically, with no
        // per-theme C++ branch (architecture-principles.md §16/§18/§19/§23). CNewUISkillList
        // reads GetSkillListOffsetX() via the g_pMainFrame global since it owns no RmlUi document
        // of its own to query directly.
        float GetItemHotkeyOffsetX() const { return m_fItemHotkeyOffsetX; }
        float GetSkillListOffsetX() const { return m_fSkillListOffsetX; }

    private:
        void LoadImages();
        void UnloadImages();

        void RenderLeftRegion();
        void RenderCenterRegion();
        void RenderLeftFrame();
        void RenderCenterFrame();
        void RenderHotKeyItemCount();

        void SyncRmlModel();

    public:
        __int64	m_loPreExp;
        __int64	m_loGetExp;

    private:
        CNewUIManager* m_pNewUIMng;
        CNewUI3DRenderMng* m_pNewUI3DRenderMng;

        CNewUIItemHotKey m_ItemHotKey;

        bool m_bExpEffect;
        DWORD m_dwExpEffectTime;

        __int64 m_dwPreExp;
        __int64 m_dwGetExp;

        // See GetItemHotkeyOffsetX()/GetSkillListOffsetX()'s own comment above. Computed fresh in
        // SyncRmlModel() every frame from the #item_hotkey_anchor/#skill_list_anchor markers'
        // current Element::GetAbsoluteOffset(); 0.f (no shift) until the RmlUi document exists.
        float m_fItemHotkeyOffsetX = 0.f;
        float m_fSkillListOffsetX = 0.f;

        bool m_bButtonBlink;

        struct MainFrameRmlModel
        {
            // #bars/#buttons/#exp are ONE shared transform group (2026-09-01, after several
            // rounds of feedback converged here): all three sit inside CNewUISkillList's still-
            // legacy center-band chrome and item-hotkey band (kept legacy -- see this class's own
            // header comment), which scale with window size via UI::Scaling::BottomHudScale
            // (clamped 1x-2x), a completely different system from this branch's standard
            // fixed-dp/UIScalePercent policy. This is the same "genuinely computed per-frame
            // position" carve-out layout-and-scaling.md already documents for CCharInfoBalloon --
            // this group binds left/top/scale from UI::Scaling::BottomHudCenterTransform every
            // frame (SyncRmlModel()) so it tracks the legacy chrome exactly.
            //
            // Buttons and exp were originally separate containers, each bound from their OWN
            // BottomHud{Right,Experience}Transform (anchored to the window's right edge / full
            // window width respectively) -- matching how the legacy renderer itself independently
            // positions those two bands. Two rounds of feedback found that wrong in different
            // ways: (1) buttons/exp being independently-anchored, rather than part of this same
            // group, meant they didn't track a shared anchor point correctly (a real bug -- see
            // git history for the PositionX/PositionY-vs-raw-offset mixup this caused, most
            // visible at 720p where BottomHudScale's offset happened to be exactly 0); (2) even
            // once positioned "correctly" by their own independent transforms, the *legacy visual
            // design itself* (buttons flush to the window's right edge, exp spanning the full
            // real window width) wasn't what was wanted -- the request was to anchor buttons next
            // to the mana bar and run exp from the item-hotkey band's start to the button group's
            // end, i.e. explicitly PART of this one shared 640-reference-unit group, not
            // independently full-width/edge-anchored. So buttons/exp now reuse these same 3
            // fields and plain *un-rebased* reference-pixel child coordinates, exactly like #bars
            // -- see main_frame.rcss's #bars/#buttons/#exp rules (buttons/exp are nested INSIDE
            // #bars in main_frame.rml specifically so they inherit this transform for free,
            // rather than needing their own redundant bindings of the same 3 values).
            //
            // FIXED (was "not sure if this HUD follows the config.ini scale %" -- confirmed it
            // didn't): UI::Scaling::BottomHudScale() itself now folds in
            // GameConfig::GetUIScalePercent() as a post-clamp multiplier (UITransform.cpp), so
            // barsScale picks it up automatically here, and so does every other caller of
            // BottomHudScale/BottomHudCenterTransform codebase-wide -- in particular
            // Render()/Render3D()/UseHotKeyItemRButton()'s own BottomHudCenterTransform() calls
            // (still real UI::Scaling C++, used for the still-legacy item-hotkey band's
            // hit-testing and 3D icon placement) -- for free, from the one shared function, with
            // no separate wiring needed here. This is exactly why the fix landed in
            // BottomHudScale() itself rather than as a multiply-in-place on barsScale alone: the
            // two call sites can't drift out of sync the way CCharSelMainWin's independent
            // calculators once did (layout-and-scaling.md). Every RmlUi-authored length in
            // main_frame.rcss is still `px`, not `dp`, so it continues to track bars_scale exactly
            // instead of being scaled a second time by RmlUiRuntime's context-wide
            // density-independent-pixel ratio (see that ratio's own ApplyUIScale() comment) --
            // that split is unchanged by this fix. Verify by exercising a potion/skill at more
            // than one UIScalePercent value and resolution, not just a visual check, before
            // trusting this in play (docs/rmlui-ui-system/layout-and-scaling.md).
            float barsLeft = 0.f, barsTop = 0.f, barsScale = 1.f;

            float hpFraction = 0.f, mpFraction = 0.f, agFraction = 0.f, sdFraction = 0.f;
            Rml::String hpText, mpText, agText, sdText;

            // 2026-09-02: current-value-only readout ("935", not "935 / 935") -- legacy theme's
            // own gauge numbers use this instead of hpText/etc (feedback: "remove the Y in the
            // X/Y value of the gauge bars" for legacy specifically, without touching modern).
            // Computed unconditionally alongside hpText/etc, both themes, same values just
            // formatted differently -- modern's main_frame.rml still binds hpText/etc; only which
            // field each theme's own markup references differs, no C++ theme check anywhere.
            Rml::String hpCurrentText, mpCurrentText, agCurrentText, sdCurrentText;
            Rml::String hpTooltip, mpTooltip, agTooltip, sdTooltip;
            bool poisoned = false; // true -> HP fill swaps red to green (eDeBuff_Poison)

            // EXP: legacy RenderExperience() resets its visual fill every 10% of the level (see
            // buildExpSegment() in the .cpp) and shows which decile via a separate digit readout --
            // expFraction is progress *within* the current decile (0..1), not overall level
            // progress; expDigit (0-9) is that decile number, rendered as plain text. Width/
            // position now come from the shared bars_left/top/scale group (see above), spanning
            // reference x=0 (item-hotkey band's own start) to x=640 (button group's own end) --
            // no longer the full real window width the legacy EXP band itself used.
            float expFraction = 0.f;
            Rml::String expDigit;
            Rml::String expTooltip;

            // Corner buttons. "Open" mirrors SetBtnState()'s bStateDown (true while the button's
            // target panel is visible); tooltips are static per-button strings pushed once.
            bool cShopOpen = false, chaInfoOpen = false, myInvenOpen = false, friendOpen = false, windowOpen = false;
            Rml::String cShopTooltip, chaInfoTooltip, myInvenTooltip, friendTooltip, windowTooltip;

            // Alert-blink dots -- replace the legacy RenderCharInfoButton()/RenderFriendButton()/
            // RenderFriendButtonState() frame-swap blink with a simple bound boolean + CSS dot
            // (same convention as every other migrated alert/indicator), computed every frame in
            // SyncRmlModel() from the same g_Time.GetTimeCheck()/g_pFriendMenu state the legacy
            // code read.
            bool chaInfoAlert = false, friendAlert = false;

            // 2026-09-01: which of the still-legacy skill-hotkey row's 5 slots (#skill_slot_0..4,
            // main_frame.rml) currently shows the equipped/active skill -- see
            // CNewUISkillList::IsHotKeySlotCurrentSkill()'s own comment. Modern theme binds this to
            // a CSS highlight class instead of the legacy IMAGE_SKILLBOX_USE sprite that function
            // used to always draw; legacy theme keeps that real sprite (own established look,
            // unaffected by this). 5 separate named fields, not an array -- c.Bind() (RmlUi's
            // DataModelConstructor) takes a pointer-to-member of a scalar field, same as every
            // other bool in this struct.
            bool skillSlot0Selected = false, skillSlot1Selected = false, skillSlot2Selected = false,
                 skillSlot3Selected = false, skillSlot4Selected = false;

            // 2026-09-02: the hotkey number (1-9,0) each of #skill_slot_0..4 is bound to -- see
            // CNewUISkillList::GetHotKeySlotNumber()'s own comment. Empty string for an unbound
            // slot (RmlUi's {{ }} interpolation of an empty string renders nothing, same "draw
            // nothing" behavior the legacy digit-sprite path already has for that case).
            Rml::String skillSlot0Hotkey, skillSlot1Hotkey, skillSlot2Hotkey, skillSlot3Hotkey, skillSlot4Hotkey;

            // Phase 2 (skill list) additions -- see docs/rmlui-ui-system's Phase 2 plan and
            // CNewUISkillList's own new members/methods (NewUIMainFrameWindow.h, right above this
            // class). Cooldown wipe for the compact row + current-skill slot: same "5 separate
            // fields" convention as skillSlot0..4Hotkey above, plus one for #current_skill_slot.
            float skillSlot0Cooldown = 0.f, skillSlot1Cooldown = 0.f, skillSlot2Cooldown = 0.f,
                  skillSlot3Cooldown = 0.f, skillSlot4Cooldown = 0.f;
            float currentSkillCooldown = 0.f;

            // Expanded skill grid + pet-command row -- mirrors CNewUISkillList::IsSkillGridOpen()
            // (NOT the similarly-named IsSkillListUp(), a legacy naming trap -- see that method's
            // own comment; binding this to it instead was a real 2026-09-02 bug, fixed). Gates
            // #skill_grid/#pet_skill_row visibility in RCSS. Cell arrays are dynamic-count
            // data-for lists (SkillCellEntry, RegisterStruct'd in Create()), same shape as
            // CBuffStrip's already-proven buffs array -- rebuilt every frame the grid is open,
            // left stale (harmless, hidden) while closed.
            bool skillGridOpen = false;
            std::vector<SkillCellEntry> skillGridCells;
            std::vector<SkillCellEntry> petSkillCells;

            // Shared skill tooltip -- one RmlUi element reused for the hotkey row, current-skill
            // slot, grid, and pet row alike (matches the legacy RenderSkillInfo()'s own "one
            // tooltip object, repositioned/repopulated per hover target" shape). left/top are in
            // #bars's own local reference space (same as skillGridCells' own left/top) so the
            // tooltip can simply nest inside #bars too; main_frame.rcss anchors the tooltip's
            // BOTTOM edge at (top) via `transform: translateY(-100%)` so it grows upward
            // regardless of line count, reproducing RenderTipTextList()'s own "sy -= Height"
            // auto-flip without needing that height math ported into C++.
            bool skillTooltipVisible = false;
            float skillTooltipLeft = 0.f, skillTooltipTop = 0.f;
            std::vector<SkillTooltipLineEntry> skillTooltipLines;
        };
        RmlModelBinder<MainFrameRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;

        // RmlUi-behind-3D-icons proof of concept (docs/rmlui-ui-system/STATUS.md's "RmlUi renders
        // last" finding) -- RenderLeftFrame()'s own header comment explains why this exists: the
        // left/center HUD-strip background fill can never be RmlUi-drawn through m_pRmlDoc's own
        // "main" context (it must sit BEHIND the legacy 3D-composited item/skill icons, which
        // always render after everything in that context), so it lives in
        // RmlUiRuntime::GetBackgroundContext() instead, rendered explicitly via
        // RmlUiRuntime::RenderBackgroundLayer() from inside RenderLeftFrame(). Separate document/
        // model/context from m_pRmlDoc's -- RmlUi data models are per-context, and this one only
        // ever needs the 5 fields below, not main_frame's full model.
        struct MainFrameBgRmlModel
        {
            // Mirrors MainFrameRmlModel::barsLeft/barsTop/barsScale exactly (same
            // UI::Scaling::BottomHudCenterTransform() values, synced from the same place in
            // SyncRmlModel()) -- #bg_root (main_frame_bg.rml) uses the identical
            // data-style-left/top/transform:scale() convention #bars does, so this reference-space
            // panel scales/tracks the legacy chrome the same way #bars's own children do.
            float rootX = 0.f, rootY = 0.f, rootScale = 1.f;

            // GetItemHotkeyOffsetX()/GetSkillListOffsetX()'s own values (same members
            // m_fItemHotkeyOffsetX/m_fSkillListOffsetX Render()'s leftTransform/centerTransform
            // already add) -- added on top of rootX inside main_frame_bg.rml's own child elements
            // so the two panels track their real anchors exactly like the legacy quads did.
            float leftOffsetX = 0.f, centerOffsetX = 0.f;
        };
        RmlModelBinder<MainFrameBgRmlModel> m_BgRmlBinder;
        Rml::ElementDocument* m_pRmlBgDoc = nullptr;

        bool m_bRmlCShopClicked = false;
        bool m_bRmlChaInfoClicked = false;
        bool m_bRmlMyInvenClicked = false;
        bool m_bRmlFriendClicked = false;
        bool m_bRmlWindowClicked = false;

    public:
        // Re-derives the RmlUi document's Show()/Hide() state from IsVisible() && sceneAllowsShow
        // -- the third MAIN_SCENE prerequisite every CNewUIObj-tier pilot needs, same as
        // CMuHelperBar/CBuffStrip (see those classes' header comments for the full rationale).
        void SyncDocVisibility(bool sceneAllowsShow);
    };
}

#endif // !defined(AFX_NEWUIMAINFRAMEWINDOW_H__46A029CA_44A5_4050_9216_FA8A25EC4629__INCLUDED_)
