
#if !defined(AFX_NEWUIOPTIONWINDOW_H__1469FA1D_7C15_4AFE_AD6E_59C303E72BC0__INCLUDED_)
#define AFX_NEWUIOPTIONWINDOW_H__1469FA1D_7C15_4AFE_AD6E_59C303E72BC0__INCLUDED_

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "UI/Core/WindowManager.h"
#include "UI/Inventory/MyInventory.h"
#include "UI/RmlBridge/RmlThemedView.h"

namespace Rml { class ElementDocument; class Element; class Event; }

namespace UI::Options
{
using DisplayResolution = std::pair<int, int>;

std::vector<DisplayResolution> NormalizeDisplayResolutions(std::vector<DisplayResolution> resolutions);
int FindExactDisplayResolutionIndex(const std::vector<DisplayResolution>& resolutions, int width, int height);
int FindClosestDisplayResolutionIndex(const std::vector<DisplayResolution>& resolutions, int width, int height);

// The UI tab's UI-scale row offers this fixed ladder of percentages (ascending), the same shape
// the FPS Limit row's own value table has. Free function rather than a private static table so the
// list and the lookup below are testable without standing up the window (tests/ui/test_ui_scaling.cpp).
const std::vector<int>& UIScalePercentChoices();
// Index of the offered percentage to show for `percent`, which need not be one of them --
// GameConfig's own bounds are wider than this ladder, so a hand-edited config.ini legitimately
// holds values in between. Ties resolve to the lower entry.
int FindClosestUIScaleIndex(int percent);
} // namespace UI::Options

namespace mu::ui::window
{
    class COptionWindow : public CObject
    {
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    public:
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
        // Kept even though this window no longer renders through the legacy bitmap-atlas system. No
        // other window aliases BITMAP_OPTION_BEGIN any more (the MU Helper did, until its port), but
        // LoadImages() below also loads slots aliased from CMessageBoxMng/CMyInventory -- check those
        // owners load their own before removing it.
        enum IMAGE_LIST
        {
            IMAGE_OPTION_FRAME_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK,
            IMAGE_OPTION_BTN_CLOSE = CMessageBoxMng::IMAGE_MSGBOX_BTN_CLOSE,
            IMAGE_OPTION_FRAME_DOWN = CMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,

            IMAGE_OPTION_FRAME_UP = BITMAP_OPTION_BEGIN,
            IMAGE_OPTION_FRAME_LEFT,
            IMAGE_OPTION_FRAME_RIGHT,
            IMAGE_OPTION_LINE,
            IMAGE_OPTION_POINT,
            IMAGE_OPTION_BTN_CHECK,
            IMAGE_OPTION_EFFECT_BACK,
            IMAGE_OPTION_EFFECT_COLOR,
            IMAGE_OPTION_VOLUME_BACK,
            IMAGE_OPTION_VOLUME_COLOR,
        };

    public:
        COptionWindow();
        virtual ~COptionWindow();

        bool Create(CManager* pNewUIMng);
        void Release();

        void Show(bool bShow) override;

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 10.5f
        float GetKeyEventOrder();	// 10.f;

        void OpenningProcess();
        void ClosingProcess();

        void SetAutoAttack(bool bAuto);
        bool IsAutoAttack();
        void SetWhisperSound(bool bSound);
        bool IsWhisperSound();
        void SetSlideHelp(bool bHelp);
        bool IsSlideHelp();
        void SetVolumeLevel(int iVolume);
        int GetVolumeLevel();
        void SetRenderLevel(int iRender);
        int GetRenderLevel();
        void SetRenderAllEffects(bool bRenderAllEffects);
        bool GetRenderAllEffects();

    private:
        void LoadImages();
        void UnloadImages();

        void InitResolutionCombo();
        void InitLanguageCombo();
        void InitFontCombo();

        void ApplyResolution();
        int FindCurrentResolutionIndex();
        void SyncResolutionComboToWindow();
        void ApplyWindowModeToggle();

        void ApplyLanguage();
        int FindCurrentLanguageIndex();

        void ApplyFont();
        int FindCurrentFontIndex();

        // Video tab's FPS Limit row -- indexes the fixed s_FpsCapValues table (.cpp), no combo-init
        // needed since the list never changes at runtime (unlike resolutions).
        int FindCurrentFpsCapIndex();

        // Interface/UI tab's UI Theme row -- indexes {legacy, modern} against
        // UI::RmlBridge::GetActiveThemeName().
        int FindCurrentThemeIndex();

        // Interface/UI tab's UI Scale row -- indexes UI::Options::UIScalePercentChoices() against
        // GameConfig::GetUIScalePercent().
        int FindCurrentUIScaleIndex();

        void OnSoundVolumeChanged();
        void OnMusicVolumeChanged();

        // RmlUi wiring
        void BuildRmlUi();
        void SyncRmlModel();
        // Performs a theme switch recorded by RmlThemeChanged(), deferred to Update() -- see
        // m_bPendingThemeSwitch's own comment for why this can't happen synchronously inside the
        // dropdown-option click event that recorded it.
        void ApplyPendingThemeSwitch();
        // Same deferral for the UI-scale row -- see m_bPendingUIScaleApply's own comment.
        void ApplyPendingUIScale();

        // Invoked directly from RmlUi data-event-click/-change bindings (see BindRmlModel()), not
        // polled. RmlClickSelectTab first, matching MyQuestInfoWindow's own tab-callback ordering.
        void RmlClickSelectTab(int nTab);
        // Custom-dropdown mechanism (option_window.rml's own .option-dropdown family; RmlUi's
        // native <select> isn't used here) -- dropdownId is 0=resolution,1=fpsCap,2=theme,
        // 3=language,4=font,5=uiScale, matching model.openDropdown's own comment (OptionWindow.h).
        // RmlDropdownOptionClick dispatches to RmlResolutionChanged()/RmlFpsCapChanged()/etc. per
        // dropdownId.
        void RmlToggleDropdown(int dropdownId);
        // A just-opened dropdown shows its current value, as native's list boxes open on it: run
        // the frame after opening, once the list is laid out.
        void ScrollOpenDropdownToSelection();
        void RmlDropdownOptionClick(int dropdownId, int optionIndex);
        void RmlToggleAutoAttack();
        void RmlToggleWhisperSound();
        void RmlToggleSlideHelp();
        void RmlToggleRenderAllEffects();
        void RmlToggleWindowedMode();
        void RmlSoundVolumeChanged(int value);
        void RmlMusicVolumeChanged(int value);
        void RmlRenderLevelChanged(int value);
        // The volume and effect-limit gauges' pointer and wheel events (0 sound, 1 music, 2 effects).
        void RmlGaugeEvent(Rml::Event& event, int gauge);
        void RmlResolutionChanged(int index);
        void RmlLanguageChanged(int index);
        void RmlFontChanged(int index);
        // Graphics tab -- DXP-23's per-system effect-cost toggles (MainScene.h), promoted from
        // console-only ($effects ...) diagnostics to a persisted, in-game-options setting.
        void RmlToggleVsync();
        void RmlFpsCapChanged(int index);
        void RmlToggleDisableEffects();
        void RmlToggleDisableParticles();
        void RmlToggleDisableSkillEffectModels();
        void RmlToggleDisableBoids();
        void RmlToggleDisableWingShadow();
        // Interface/UI tab -- Show FPS Counter/Show Debug Info stay ephemeral (SceneManager.h
        // globals, no GameConfig persistence), matching $fpscounter/$details today.
        void RmlToggleShowFpsCounter();
        void RmlToggleShowDebugInfo();
        void RmlThemeChanged(int index);
        void RmlUIScaleChanged(int index);
        void RmlClickClose();

        struct OptionRmlModel
        {
            // window_shell's positioning extension -- always false: `.center-both` centres this
            // window, and a drag (MakeDraggable()) moves it for the session without
            // a C++-computed position. Still bound since window_shell.rml's template always
            // references these three fields.
            bool positioned = false;
            float rootX = 0.f, rootY = 0.f;

            bool hasTitle = true;
            Rml::String title;
            Rml::String closeLabel;

            // Tab state -- bound/diffed first, matching MyQuestInfoWindow's own convention for its
            // own activeTab field (see BindRmlModel()/SyncRmlModel()). 0=Gameplay, 1=Audio, 2=Video,
            // 3=Graphics, 4=Interface/UI, 5=General.
            int activeTab = 0;
            Rml::String tabGameplayLabel;
            Rml::String tabAudioLabel;
            Rml::String tabVideoLabel;
            Rml::String tabGraphicsLabel;
            Rml::String tabUiLabel;
            Rml::String tabGeneralLabel;

            // Which custom dropdown (.option-dropdown) is currently open, -1 = none. A single
            // field rather than one bool per dropdown gives exclusivity for free -- opening one
            // overwrites whichever other id was here, no separate "close the others" step needed.
            // 0=resolution, 1=fpsCap, 2=theme, 3=language, 4=font, 5=uiScale (RmlToggleDropdown()/
            // RmlDropdownOptionClick()'s own ids). Replaces RmlUi's native <select>/<option>
            // (WidgetDropDown) entirely -- see option_window.rml's own history for why: that
            // widget's own generated selectbox/selectvalue/selectarrow sub-elements need CSS this
            // theme never gave them (no position set programmatically, unlike WidgetSlider), and
            // even after fixing that, selecting an option still didn't reliably reach this
            // window's own change callback. option_window.rml itself flagged this as a real risk
            // up front ("spike before trusting; a data-for custom dropdown is the fallback") --
            // this is that fallback, reusing the exact data-for + data-class + data-event-click
            // mechanism the tab bar above already proves reliable.
            int openDropdown = -1;

            bool autoAttack = true;
            Rml::String autoAttackLabel;
            bool whisperSound = false;
            Rml::String whisperSoundLabel;
            bool slideHelp = true;
            Rml::String slideHelpLabel;
            bool renderAllEffects = true;
            Rml::String renderAllEffectsLabel;
            bool windowedMode = false;
            Rml::String windowedModeLabel;

            int soundVolume = 0;
            Rml::String soundVolumeLabel;
            int musicVolume = 0;
            Rml::String musicVolumeLabel;
            int renderLevel = 4;
            Rml::String renderLevelLabel;

            std::vector<Rml::String> resolutionLabels;
            int resolutionIndex = 0;
            Rml::String resolutionRowLabel;
            // Text shown in the dropdown's own closed-state box -- resolutionLabels[resolutionIndex],
            // kept as its own diffed field rather than a {{}} array-index expression (this engine's
            // binding-expression grammar is otherwise untested for that, see DataExpression.cpp).
            Rml::String resolutionValueLabel;
            std::vector<Rml::String> languageLabels;
            int languageIndex = 0;
            Rml::String languageRowLabel;
            Rml::String languageValueLabel;
            std::vector<Rml::String> fontLabels;
            int fontIndex = 0;
            Rml::String fontRowLabel;
            Rml::String fontValueLabel;

            // Video tab additions.
            bool vsyncEnabled = true;
            Rml::String vsyncLabel;
            std::vector<Rml::String> fpsCapLabels;
            int fpsCapIndex = 0;
            Rml::String fpsCapRowLabel;
            Rml::String fpsCapValueLabel;

            // Graphics tab -- DXP-23's per-system effect-cost toggles, promoted to a real setting.
            bool disableEffects = false;
            Rml::String disableEffectsLabel;
            bool disableParticles = false;
            Rml::String disableParticlesLabel;
            bool disableSkillEffectModels = false;
            Rml::String disableSkillEffectModelsLabel;
            bool disableBoids = false;
            Rml::String disableBoidsLabel;
            bool disableWingShadow = false;
            Rml::String disableWingShadowLabel;

            // Interface/UI tab.
            bool showFpsCounter = false;
            Rml::String showFpsCounterLabel;
            bool showDebugInfo = false;
            Rml::String showDebugInfoLabel;
            std::vector<Rml::String> themeLabels;
            int themeIndex = 0;
            Rml::String themeRowLabel;
            Rml::String themeValueLabel;
            std::vector<Rml::String> uiScaleLabels;
            int uiScaleIndex = 0;
            Rml::String uiScaleRowLabel;
            Rml::String uiScaleValueLabel;
            // First hover tooltip in this window -- the row's label alone can't say what the
            // percentage multiplies (option_window.rml's own .option-row-tip markup shows it on
            // hover, purely in RCSS, the same `:hover` mechanism main_frame.rcss's own gauge
            // tooltips use).
            Rml::String uiScaleTooltip;
        };
        void BindRmlModel(Rml::DataModelConstructor& c, OptionRmlModel& model);
        void OnRmlBuilt();
        UI::RmlBridge::ThemedView<OptionRmlModel> m_RmlView{"option_window",
            [this](Rml::DataModelConstructor& c, OptionRmlModel& model) { BindRmlModel(c, model); },
            {{"Data/Interface/RmlUi/option_window.rml"}},
            {.afterBuild = [this] { OnRmlBuilt(); }, .beforeUnload = [this] { m_pPanelEl = nullptr; }}};
        // Cached after every build -- UpdateMouseEvent()'s hit-test reads this element's own
        // live rendered position/size (GetAbsoluteOffset()/GetOffsetWidth()/GetOffsetHeight())
        // rather than approximating them from hardcoded dp constants, so the hit-test rect can
        // never drift from wherever `.center-both` (or a future positioned/dragged mode) actually
        // put the panel, in either theme.
        Rml::Element* m_pPanelEl = nullptr;

    private:
        CManager* m_pNewUIMng;


        bool m_bAutoAttack;
        bool m_bWhisperSound;
        bool m_bSlideHelp;
        int m_iVolumeLevel;     // Sound volume (0=off, 10=max)
        int m_iMusicLevel;      // Music volume (0=off, 10=max)
        int m_iRenderLevel;
        bool m_bRenderAllEffects;
        int m_iResolutionIndex;
        bool m_bWindowedMode;
        int m_iLanguageIndex;
        int m_iFontIndex;

        std::vector<UI::Options::DisplayResolution> m_resolutions;
        std::vector<std::wstring> m_resolutionLabels;

        int m_iActiveTab = 0;
        // -1 = none open. See model.openDropdown's own comment (OptionRmlModel) for the id scheme.
        int m_iOpenDropdown = -1;
        bool m_bScrollDropdownPending = false;

        // Video tab additions -- m_bVsyncEnabled seeded from GameConfig::GetVSyncEnabled() (same
        // place m_bWindowedMode seeds from g_bUseWindowMode); m_iFpsCapIndex indexes
        // s_FpsCapValues (kFpsCapValues in the .cpp), seeded by matching GameConfig::GetFpsCap().
        bool m_bVsyncEnabled;
        int m_iFpsCapIndex;

        // Graphics tab -- DXP-23's per-system effect-cost toggles, seeded from their matching new
        // GameConfig getters; each one calls both the live MainScene.h setter and its GameConfig
        // counterpart+Save() on toggle, same "apply live + persist" shape VSync's own wiring uses.
        bool m_bDisableEffects;
        bool m_bDisableParticles;
        bool m_bDisableSkillEffectModels;
        bool m_bDisableBoids;
        bool m_bDisableWingShadow;

        // Interface/UI tab -- m_iThemeIndex seeded from UI::RmlBridge::GetActiveThemeName()
        // (0=legacy/1=modern). Show FPS Counter/Show Debug Info need no members -- SyncRmlModel()
        // reads SceneManager.h's GetShowFpsCounter()/GetShowDebugInfo() live every frame instead
        // (see OptionRmlModel's own comment on why: they're mutually exclusive globals with no
        // GameConfig backing).
        int m_iThemeIndex;
        // Indexes UI::Options::UIScalePercentChoices(); seeded from GameConfig::GetUIScalePercent()
        // (and re-seeded in OpenningProcess(), so a scale set from outside this window -- e.g. a
        // hand-edited config.ini -- shows up on the next open).
        int m_iUIScaleIndex;
        // A theme switch destroys and rebuilds every registered RmlUi document (including this
        // window's own, via UI::RmlBridge::ReloadAllThemedDocuments()) -- doing that synchronously
        // inside RmlThemeChanged() would tear down the document while still unwinding through RmlUi's
        // own event-dispatch call stack for the very "change" event that triggered it. Deferred
        // instead: RmlThemeChanged() only records the request; ApplyPendingThemeSwitch() (called
        // from Update(), outside any RmlUi event) performs it on the next tick.
        bool m_bPendingThemeSwitch = false;
        int m_iPendingThemeIndex = 0;
        // Applying a UI scale resizes the window to its current size (MuApplyWindowResolution),
        // which re-runs every resolution-dependent system, re-asserts RmlUi's dp ratio on all
        // three contexts and pumps SDL while settling the window -- too much to run while still
        // unwinding through RmlUi's own dispatch of the option click that asked for it. Deferred to
        // Update() for the same reason m_bPendingThemeSwitch is, even though this path (unlike a
        // theme switch) does not itself destroy the document.
        bool m_bPendingUIScaleApply = false;
        int m_iPendingUIScalePercent = 0;

        // Counts SyncRmlModel() calls since BuildRmlUi(). Acts as a harmless no-op-this-early
        // guard on RmlDropdownOptionClick()'s own callers -- real user input can't land in this
        // window this soon after Create() since it isn't shown yet.
        int m_rmlSyncCount = 0;
    };
}

#endif // !defined(AFX_NEWUIOPTIONWINDOW_H__1469FA1D_7C15_4AFE_AD6E_59C303E72BC0__INCLUDED_)
