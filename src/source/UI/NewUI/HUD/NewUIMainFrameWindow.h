// NewUIMainFrameWindow.h: interface for the CNewUIMainFrameWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIMAINFRAMEWINDOW_H__46A029CA_44A5_4050_9216_FA8A25EC4629__INCLUDED_)
#define AFX_NEWUIMAINFRAMEWINDOW_H__46A029CA_44A5_4050_9216_FA8A25EC4629__INCLUDED_

#pragma once

#include "UI/NewUI/NewUIBase.h"
#include "Render/Textures/ZzzTexture.h"
#include "UI/NewUI/NewUI3DRenderMng.h"
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

    class CNewUISkillList : public CNewUIObj
    {
        enum
        {
            SKILLHOTKEY_COUNT = 10
        };
        enum EVENT_STATE
        {
            EVENT_NONE = 0,

            // currentskill
            EVENT_BTN_HOVER_CURRENTSKILL,
            EVENT_BTN_DOWN_CURRENTSKILL,

            // skillhotkey
            EVENT_BTN_HOVER_SKILLHOTKEY,
            EVENT_BTN_DOWN_SKILLHOTKEY,

            // skilllist
            EVENT_BTN_HOVER_SKILLLIST,
            EVENT_BTN_DOWN_SKILLLIST,
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
        void RenderSkillInfo();
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

        bool IsSkillListUp();

        static void UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB);

    private:
        void LoadImages();
        void UnloadImages();
        bool IsArrayUp(BYTE bySkill);
        bool IsArrayIn(BYTE bySkill);
        void UseHotKey(int iHotKey);

        void RenderSkillIcon(int iIndex, float x, float y, float width, float height);
        void RenderSkillDelay(int iIndex, float x, float y, float width, float height);
        void RenderPetSkill();

        void ResetMouseLButton();

    private:
        CNewUIManager* m_pNewUIMng;
        CNewUI3DRenderMng* m_pNewUI3DRenderMng;

        bool m_bHotKeySkillListUp;
        int m_iHotKeySkillType[SKILLHOTKEY_COUNT];

        bool m_bSkillList;

        bool m_bRenderSkillInfo;
        int m_iRenderSkillInfoType;
        int m_iRenderSkillInfoPosX;
        int m_iRenderSkillInfoPosY;

        EVENT_STATE m_EventState;
        WORD m_wHeroPriorSkill;
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
            // Known limitation, not yet fixed (feedback: "not sure if this HUD follows the
            // config.ini scale %" -- confirmed it doesn't): barsScale comes from
            // UI::Scaling::BottomHudScale alone, which has never read GameConfig::
            // GetUIScalePercent() (grepped UITransform.cpp -- no reference), matching the
            // original legacy CNewUIMainFrameWindow's own behavior (this HUD band predates that
            // config option). Every RmlUi-authored length in main_frame.rcss is `px`, not `dp`,
            // specifically so it tracks bars_scale exactly instead of being scaled a second time
            // by RmlUiRuntime's context-wide density-independent-pixel ratio (see that ratio's own
            // ApplyUIScale() comment). Composing UIScalePercent into barsScale here would need the
            // SAME extra factor folded into Render()/Render3D()/UseHotKeyItemRButton()'s own
            // BottomHudCenterTransform() calls (still real UI::Scaling C++, used for the
            // still-legacy item-hotkey band's hit-testing and 3D icon placement) to keep the two
            // in sync -- deferred rather than done as a quick multiply-in-place here, to avoid
            // touching gameplay-critical potion click hit-testing under this pilot's own scope.
            // Revisit alongside the rest of this carve-out once Phase 2/3 land and the whole
            // mechanism gets replaced with the branch's normal fixed-dp/UIScalePercent policy.
            float barsLeft = 0.f, barsTop = 0.f, barsScale = 1.f;

            float hpFraction = 0.f, mpFraction = 0.f, agFraction = 0.f, sdFraction = 0.f;
            Rml::String hpText, mpText, agText, sdText;
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
        };
        RmlModelBinder<MainFrameRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;

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
