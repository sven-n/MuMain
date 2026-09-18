
#if !defined(AFX_NEWUIOPTIONWINDOW_H__1469FA1D_7C15_4AFE_AD6E_59C303E72BC0__INCLUDED_)
#define AFX_NEWUIOPTIONWINDOW_H__1469FA1D_7C15_4AFE_AD6E_59C303E72BC0__INCLUDED_

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "UI/Core/WindowManager.h"
#include "UI/Inventory/MyInventory.h"
#include "UI/RmlBridge/RmlModelBinder.h"

namespace Rml { class ElementDocument; class Element; }

namespace UI::Options
{
using DisplayResolution = std::pair<int, int>;

std::vector<DisplayResolution> NormalizeDisplayResolutions(std::vector<DisplayResolution> resolutions);
int FindExactDisplayResolutionIndex(const std::vector<DisplayResolution>& resolutions, int width, int height);
int FindClosestDisplayResolutionIndex(const std::vector<DisplayResolution>& resolutions, int width, int height);
} // namespace UI::Options

namespace mu::ui::window
{
    class COptionWindow : public CObject
    {
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    public:
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
        // Kept even though this window no longer renders through the legacy bitmap-atlas system --
        // CUIMuHelper's own hunt/pick-range "+" buttons alias these same texture slots and expect
        // LoadImages() below to have populated them (same reason CCharacterInfoWindow/
        // CMyQuestInfoWindow keep their own LoadImages()).
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

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);
        void Show(bool bShow) override;
        // Rebuilds m_pRmlDoc against whichever theme is now active -- needed because this window's
        // own UI-Theme dropdown (Interface/UI tab) can switch the active theme from inside itself;
        // CManager::ReloadAllRmlThemes() sweeps every registered window through this same override.
        void ReloadRmlTheme() override;

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

        void OnSoundVolumeChanged();
        void OnMusicVolumeChanged();

        // RmlUi wiring
        void BuildRmlUi();
        void SyncRmlModel();
        // Performs a theme switch recorded by RmlThemeChanged(), deferred to Update() -- see
        // m_bPendingThemeSwitch's own comment for why this can't happen synchronously inside the
        // <select>'s own change event.
        void ApplyPendingThemeSwitch();

        // Invoked directly from RmlUi data-event-click/-change bindings (see BuildRmlUi()), not
        // polled. RmlClickSelectTab first, matching MyQuestInfoWindow's own tab-callback ordering.
        void RmlClickSelectTab(int nTab);
        void RmlToggleAutoAttack();
        void RmlToggleWhisperSound();
        void RmlToggleSlideHelp();
        void RmlToggleRenderAllEffects();
        void RmlToggleWindowedMode();
        void RmlSoundVolumeChanged(int value);
        void RmlMusicVolumeChanged(int value);
        void RmlRenderLevelChanged(int value);
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
        void RmlClickClose();

        struct OptionRmlModel
        {
            // window_shell's positioning/dragging extension -- always false for now (not
            // draggable), so window_shell's own `.center-both` CSS centers this window exactly
            // the same way it does every other window_shell consumer; no C++-computed position
            // needed. Still bound below since window_shell.rml's template always references these
            // three fields regardless of value. Re-enable by setting positioned=true and rootX/
            // rootY to real device pixels (see window_shell.rml's own comment on why NOT
            // UI::Scaling::GetActiveTransform()'s older reference-resolution convention) if
            // dragging comes back.
            bool positioned = false;
            float rootX = 0.f, rootY = 0.f;

            bool hasTitle = true;
            Rml::String title;
            Rml::String closeLabel;

            // Tab state -- bound/diffed first, matching MyQuestInfoWindow's own convention for its
            // own activeTab field (see BuildRmlUi()/SyncRmlModel()). 0=Gameplay, 1=Audio, 2=Video,
            // 3=Graphics, 4=Interface/UI, 5=General.
            int activeTab = 0;
            Rml::String tabGameplayLabel;
            Rml::String tabAudioLabel;
            Rml::String tabVideoLabel;
            Rml::String tabGraphicsLabel;
            Rml::String tabUiLabel;
            Rml::String tabGeneralLabel;

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
            int soundVolumeMax = 10;
            Rml::String soundVolumeLabel;
            int musicVolume = 0;
            int musicVolumeMax = 10;
            Rml::String musicVolumeLabel;
            int renderLevel = 4;
            int renderLevelMax = 5;
            Rml::String renderLevelLabel;

            std::vector<Rml::String> resolutionLabels;
            int resolutionIndex = 0;
            Rml::String resolutionRowLabel;
            std::vector<Rml::String> languageLabels;
            int languageIndex = 0;
            Rml::String languageRowLabel;
            std::vector<Rml::String> fontLabels;
            int fontIndex = 0;
            Rml::String fontRowLabel;

            // Video tab additions.
            bool vsyncEnabled = true;
            Rml::String vsyncLabel;
            std::vector<Rml::String> fpsCapLabels;
            int fpsCapIndex = 0;
            Rml::String fpsCapRowLabel;

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
        };
        RmlModelBinder<OptionRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;
        // Cached at BuildRmlUi() time -- UpdateMouseEvent()'s hit-test reads this element's own
        // live rendered position/size (GetAbsoluteOffset()/GetOffsetWidth()/GetOffsetHeight())
        // rather than approximating them from hardcoded dp constants, so the hit-test rect can
        // never drift from wherever `.center-both` (or a future positioned/dragged mode) actually
        // put the panel, in either theme.
        Rml::Element* m_pPanelEl = nullptr;

    private:
        CManager* m_pNewUIMng;
        POINT						m_Pos;

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
        // A theme switch destroys and rebuilds every registered RmlUi document (including this
        // window's own, via CManager::ReloadAllRmlThemes()) -- doing that synchronously inside
        // RmlThemeChanged() would tear down m_pRmlDoc while still unwinding through RmlUi's own
        // event-dispatch call stack for the very "change" event that triggered it. Deferred
        // instead: RmlThemeChanged() only records the request; ApplyPendingThemeSwitch() (called
        // from Update(), outside any RmlUi event) performs it on the next tick.
        bool m_bPendingThemeSwitch = false;
        int m_iPendingThemeIndex = 0;

        // Counts SyncRmlModel() calls since BuildRmlUi() -- the resolution/language/font <select>s
        // fire a handful of spurious "change" events of their own while RmlUi settles their
        // data-for option list against the freshly-populated data-value index (see
        // RmlResolutionChanged's own comment); real user input can't possibly land in this window
        // this soon after Create() since it isn't even shown yet, so change events are ignored
        // until the settle window has elapsed.
        int m_rmlSyncCount = 0;
    };
}

#endif // !defined(AFX_NEWUIOPTIONWINDOW_H__1469FA1D_7C15_4AFE_AD6E_59C303E72BC0__INCLUDED_)
