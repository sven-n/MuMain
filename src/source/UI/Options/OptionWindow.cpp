
#include "stdafx.h"
#include "UI/Options/OptionWindow.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "Render/Textures/ZzzTexture.h"
#include "Audio/DSPlaySound.h"
#include "Data/GameConfig/GameConfig.h"
#include "Data/GameConfig/GameConfigConstants.h"
#include "Audio/AudioPlayer.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Core/Utilities/StringUtils.h"
#include "Scenes/SceneManager.h"
#include "Scenes/MainScene.h"
#include "App/Platform/Windows/Winmain.h"
#include "UI/Core/SceneUICoordinator.h"
#include "UI/Windows/RememberPasswordPrompt.h"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include "I18N/All.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Elements/ElementFormControl.h>
#include <RmlUi/Core/Event.h>
#include <RmlUi/Core/EventListener.h>
#include <functional>

extern int m_MusicOnOff;
extern int m_SoundOnOff;
extern unsigned int WindowWidth, WindowHeight;
extern BOOL g_bUseWindowMode;
void ReinitializeFonts();
std::vector<std::pair<int, int>> MuGetSupportedDisplayResolutions();
void MuApplyWindowResolution(unsigned int width, unsigned int height, bool windowed);
float ConvertX(float x);
float ConvertY(float y);

using namespace SEASON3B;
using namespace mu::ui::window;

// Grace period (in SyncRmlModel() calls, i.e. frames) after BuildRmlUi() during which the
// resolution/language/font <select> change callbacks are ignored -- see m_rmlSyncCount's own
// comment in OptionWindow.h.
static constexpr int kRmlSelectSettleFrames = 5;

std::vector<UI::Options::DisplayResolution>
UI::Options::NormalizeDisplayResolutions(std::vector<DisplayResolution> resolutions)
{
    resolutions.erase(std::remove_if(resolutions.begin(), resolutions.end(), [](const DisplayResolution& resolution)
                                     { return resolution.first <= 0 || resolution.second <= 0; }),
                      resolutions.end());
    std::sort(resolutions.begin(), resolutions.end());
    resolutions.erase(std::unique(resolutions.begin(), resolutions.end()), resolutions.end());
    return resolutions;
}

int UI::Options::FindExactDisplayResolutionIndex(const std::vector<DisplayResolution>& resolutions, int width,
                                                 int height)
{
    const auto match = std::find(resolutions.begin(), resolutions.end(), DisplayResolution(width, height));
    if (match == resolutions.end())
    {
        return -1;
    }

    return static_cast<int>(std::distance(resolutions.begin(), match));
}

int UI::Options::FindClosestDisplayResolutionIndex(const std::vector<DisplayResolution>& resolutions, int width,
                                                   int height)
{
    if (resolutions.empty())
    {
        return -1;
    }

    int bestIndex = 0;
    const auto distanceSquared = [width, height](const DisplayResolution& resolution)
    {
        const long long deltaWidth = static_cast<long long>(resolution.first) - width;
        const long long deltaHeight = static_cast<long long>(resolution.second) - height;
        return deltaWidth * deltaWidth + deltaHeight * deltaHeight;
    };
    long long bestDistance = distanceSquared(resolutions.front());

    for (size_t i = 1; i < resolutions.size(); ++i)
    {
        const long long distance = distanceSquared(resolutions[i]);
        if (distance < bestDistance)
        {
            bestIndex = static_cast<int>(i);
            bestDistance = distance;
        }
    }

    return bestIndex;
}

const std::vector<int>& UI::Options::UIScalePercentChoices()
{
    // Ascending; 100 (the default) is deliberately one of them so "back to normal" is a single
    // click. The floor matches GameConfig's own clamp (CfgMinUIScalePercent) -- below it the
    // options window's own rows get too small to click the setting back up. The ceiling stops at
    // 200 rather than the clamp's 300: past 200 the window no longer fits on a 1280x720 client,
    // which is the smallest size the resolution row offers on a typical display.
    static const std::vector<int> choices = {50, 60, 70, 80, 90, 100, 125, 150, 200};
    return choices;
}

int UI::Options::FindClosestUIScaleIndex(int percent)
{
    const auto& choices = UIScalePercentChoices();
    int bestIndex = 0;
    int bestDistance = std::abs(choices.front() - percent);
    for (size_t i = 1; i < choices.size(); ++i)
    {
        const int distance = std::abs(choices[i] - percent);
        if (distance < bestDistance)
        {
            bestIndex = static_cast<int>(i);
            bestDistance = distance;
        }
    }

    return bestIndex;
}

// I18N locale codes paired with each language's display name in that language, held as wide
// strings so the label vector can be rebuilt without per-frame conversions.
static const struct { const char* code; const wchar_t* label; } s_Languages[] = {
    { "en",    L"English" },
    // Non-ASCII names use universal-character-name escapes for charset-safe MSVC compilation.
    { "de",    L"Deutsch" },
    { "es",    L"Español" },                                                  // Español
    { "id",    L"Bahasa Indonesia" },
    { "ja",    L"日本語" },                                       // 日本語
    { "pl",    L"Polski" },
    { "pt",    L"Português" },                                                // Português
    { "ru",    L"Русский" },                   // Русский
    { "tl",    L"Tagalog" },
    { "uk",    L"Українська" }, // Українська
    { "zh-TW", L"繁體中文" },                                      // 繁體中文
};
static const int s_NumLanguages = sizeof(s_Languages) / sizeof(s_Languages[0]);

// UI font families offered by the font row. `name` is the GameConfig font family value; empty =
// platform default. Curated entries are bundled in ./fonts so they resolve without a system
// install.
static const struct { const wchar_t* name; const wchar_t* label; } s_Fonts[] = {
    { L"",                L"Default" },
    { L"Liberation Sans", L"Liberation Sans" },
    { L"DejaVu Sans",     L"DejaVu Sans" },
};
static const int s_NumFonts = sizeof(s_Fonts) / sizeof(s_Fonts[0]);

// FPS cap choices offered by the Video tab's FPS Limit row -- -1 matches SceneManager::SetTargetFps's
// own "uncapped" sentinel and GameConfig::GetFpsCap()'s default, so "Uncapped" doubles as this
// setting's untouched-by-the-user state (see Winmain.cpp's EffectiveOffVSyncTargetFps()).
static const int s_FpsCapValues[] = { 24, 30, 60, 120, 144, -1 };
static const int s_NumFpsCapValues = sizeof(s_FpsCapValues) / sizeof(s_FpsCapValues[0]);

namespace
{
    // Self-owning, same pattern as RmlDraggable.cpp's own DragMoveListener -- deletes itself in
    // OnDetach() per RmlUi's AddEventListener contract, so the caller never needs to track or clean
    // it up. Used for the close button (m_pCloseButtonEl) instead of a data-event-click binding
    // since that element isn't part of this document's declarative markup/data model wiring
    // anymore -- see m_pCloseButtonEl's own comment in OptionWindow.h.
    class ClickListener : public Rml::EventListener
    {
    public:
        explicit ClickListener(std::function<void()> onClick) : m_OnClick(std::move(onClick)) {}
        void ProcessEvent(Rml::Event&) override { if (m_OnClick) m_OnClick(); }
        void OnDetach(Rml::Element*) override { delete this; }
    private:
        std::function<void()> m_OnClick;
    };
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

mu::ui::window::COptionWindow::COptionWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = 0;
    m_Pos.y = 0;

    m_bAutoAttack = true;
    m_bWhisperSound = false;
    m_bSlideHelp = true;
    m_iVolumeLevel = GameConfig::GetInstance().GetSoundVolume();
    m_iMusicLevel = GameConfig::GetInstance().GetMusicVolume();
    m_iRenderLevel = 4;
    m_bRenderAllEffects = true;
    m_iResolutionIndex = 0;
    m_bWindowedMode = (g_bUseWindowMode == TRUE);
    m_iLanguageIndex = FindCurrentLanguageIndex();
    m_iFontIndex = FindCurrentFontIndex();

    m_iActiveTab = 0;

    m_bVsyncEnabled = GameConfig::GetInstance().GetVSyncEnabled();
    m_iFpsCapIndex = FindCurrentFpsCapIndex();

    m_bDisableEffects = GameConfig::GetInstance().GetDisableEffects();
    m_bDisableParticles = GameConfig::GetInstance().GetDisableParticles();
    m_bDisableSkillEffectModels = GameConfig::GetInstance().GetDisableSkillEffectModels();
    m_bDisableBoids = GameConfig::GetInstance().GetDisableBoids();
    m_bDisableWingShadow = GameConfig::GetInstance().GetDisableWingShadow();

    m_iThemeIndex = FindCurrentThemeIndex();
    m_iUIScaleIndex = FindCurrentUIScaleIndex();
}

mu::ui::window::COptionWindow::~COptionWindow()
{
    Release();
}

bool mu::ui::window::COptionWindow::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    // x/y from the caller are WindowSystem.cpp's old reference-resolution constant
    // ((640/2)-(190/2), 5) -- meaningless now that this window is centered by window_shell's own
    // `.center-both` CSS, the same as every other window_shell consumer. Kept as a parameter only
    // for call-site compatibility.
    (void)x;
    (void)y;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_OPTION, this);
    LoadImages();
    InitResolutionCombo();
    InitLanguageCombo();
    InitFontCombo();

    if (RmlUiRuntime::Instance().IsCreated())
        BuildRmlUi();

    Show(false);
    return true;
}

void mu::ui::window::COptionWindow::BuildRmlUi()
{
    const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "option_window",
        [this](Rml::DataModelConstructor& c, OptionRmlModel& model)
        {
            c.Bind("positioned", &model.positioned);
            c.Bind("root_x", &model.rootX);
            c.Bind("root_y", &model.rootY);

            c.Bind("has_title", &model.hasTitle);
            c.Bind("title", &model.title);

            c.Bind("active_tab", &model.activeTab);
            c.Bind("tab_gameplay_label", &model.tabGameplayLabel);
            c.Bind("tab_audio_label", &model.tabAudioLabel);
            c.Bind("tab_video_label", &model.tabVideoLabel);
            c.Bind("tab_graphics_label", &model.tabGraphicsLabel);
            c.Bind("tab_ui_label", &model.tabUiLabel);
            c.Bind("tab_general_label", &model.tabGeneralLabel);

            c.Bind("open_dropdown", &model.openDropdown);

            c.Bind("auto_attack", &model.autoAttack);
            c.Bind("auto_attack_label", &model.autoAttackLabel);
            c.Bind("whisper_sound", &model.whisperSound);
            c.Bind("whisper_sound_label", &model.whisperSoundLabel);
            c.Bind("slide_help", &model.slideHelp);
            c.Bind("slide_help_label", &model.slideHelpLabel);
            c.Bind("render_all_effects", &model.renderAllEffects);
            c.Bind("render_all_effects_label", &model.renderAllEffectsLabel);
            c.Bind("windowed_mode", &model.windowedMode);
            c.Bind("windowed_mode_label", &model.windowedModeLabel);

            c.Bind("sound_volume", &model.soundVolume);
            c.Bind("sound_volume_max", &model.soundVolumeMax);
            c.Bind("sound_volume_label", &model.soundVolumeLabel);
            c.Bind("music_volume", &model.musicVolume);
            c.Bind("music_volume_max", &model.musicVolumeMax);
            c.Bind("music_volume_label", &model.musicVolumeLabel);
            c.Bind("render_level", &model.renderLevel);
            c.Bind("render_level_max", &model.renderLevelMax);
            c.Bind("render_level_label", &model.renderLevelLabel);

            c.RegisterArray<std::vector<Rml::String>>();
            c.Bind("resolution_labels", &model.resolutionLabels);
            c.Bind("resolution_index", &model.resolutionIndex);
            c.Bind("resolution_row_label", &model.resolutionRowLabel);
            c.Bind("resolution_value_label", &model.resolutionValueLabel);
            c.Bind("language_labels", &model.languageLabels);
            c.Bind("language_index", &model.languageIndex);
            c.Bind("language_row_label", &model.languageRowLabel);
            c.Bind("language_value_label", &model.languageValueLabel);
            c.Bind("font_labels", &model.fontLabels);
            c.Bind("font_index", &model.fontIndex);
            c.Bind("font_row_label", &model.fontRowLabel);
            c.Bind("font_value_label", &model.fontValueLabel);

            // Video tab additions.
            c.Bind("vsync_enabled", &model.vsyncEnabled);
            c.Bind("vsync_label", &model.vsyncLabel);
            c.Bind("fps_cap_labels", &model.fpsCapLabels);
            c.Bind("fps_cap_index", &model.fpsCapIndex);
            c.Bind("fps_cap_row_label", &model.fpsCapRowLabel);
            c.Bind("fps_cap_value_label", &model.fpsCapValueLabel);

            // Graphics tab -- DXP-23's per-system effect-cost toggles.
            c.Bind("disable_effects", &model.disableEffects);
            c.Bind("disable_effects_label", &model.disableEffectsLabel);
            c.Bind("disable_particles", &model.disableParticles);
            c.Bind("disable_particles_label", &model.disableParticlesLabel);
            c.Bind("disable_skill_effect_models", &model.disableSkillEffectModels);
            c.Bind("disable_skill_effect_models_label", &model.disableSkillEffectModelsLabel);
            c.Bind("disable_boids", &model.disableBoids);
            c.Bind("disable_boids_label", &model.disableBoidsLabel);
            c.Bind("disable_wing_shadow", &model.disableWingShadow);
            c.Bind("disable_wing_shadow_label", &model.disableWingShadowLabel);

            // Interface/UI tab.
            c.Bind("show_fps_counter", &model.showFpsCounter);
            c.Bind("show_fps_counter_label", &model.showFpsCounterLabel);
            c.Bind("show_debug_info", &model.showDebugInfo);
            c.Bind("show_debug_info_label", &model.showDebugInfoLabel);
            c.Bind("theme_labels", &model.themeLabels);
            c.Bind("theme_index", &model.themeIndex);
            c.Bind("theme_row_label", &model.themeRowLabel);
            c.Bind("theme_value_label", &model.themeValueLabel);
            c.Bind("ui_scale_labels", &model.uiScaleLabels);
            c.Bind("ui_scale_index", &model.uiScaleIndex);
            c.Bind("ui_scale_row_label", &model.uiScaleRowLabel);
            c.Bind("ui_scale_value_label", &model.uiScaleValueLabel);
            c.Bind("ui_scale_tooltip", &model.uiScaleTooltip);

            c.BindEventCallback("option_select_tab",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                {
                    if (arguments.size() == 1)
                        RmlClickSelectTab(arguments[0].Get<int>(-1));
                });

            c.BindEventCallback("option_toggle_auto_attack",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlToggleAutoAttack(); });
            c.BindEventCallback("option_toggle_whisper_sound",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlToggleWhisperSound(); });
            c.BindEventCallback("option_toggle_slide_help",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlToggleSlideHelp(); });
            c.BindEventCallback("option_toggle_render_all_effects",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlToggleRenderAllEffects(); });
            c.BindEventCallback("option_toggle_windowed_mode",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlToggleWindowedMode(); });

            // Range/select inputs: `data-value` two-way-binds for display, but the callback reads
            // the just-committed value straight off the event's own target element rather than
            // trusting that data-value's internal write-back listener already ran first on this
            // same "change" event -- listener ordering between two independently-attached
            // listeners for the same event isn't part of the documented contract.
            c.BindEventCallback("option_sound_volume_changed",
                [this](Rml::DataModelHandle, Rml::Event& ev, const Rml::VariantList&)
                {
                    if (auto* control = rmlui_dynamic_cast<Rml::ElementFormControl*>(ev.GetTargetElement()))
                        RmlSoundVolumeChanged(std::atoi(control->GetValue().c_str()));
                });
            c.BindEventCallback("option_music_volume_changed",
                [this](Rml::DataModelHandle, Rml::Event& ev, const Rml::VariantList&)
                {
                    if (auto* control = rmlui_dynamic_cast<Rml::ElementFormControl*>(ev.GetTargetElement()))
                        RmlMusicVolumeChanged(std::atoi(control->GetValue().c_str()));
                });
            c.BindEventCallback("option_render_level_changed",
                [this](Rml::DataModelHandle, Rml::Event& ev, const Rml::VariantList&)
                {
                    if (auto* control = rmlui_dynamic_cast<Rml::ElementFormControl*>(ev.GetTargetElement()))
                        RmlRenderLevelChanged(std::atoi(control->GetValue().c_str()));
                });
            // .option-dropdown custom control (replaces the resolution/language/font/fps-cap/theme
            // native <select>s, see model.openDropdown's own comment) -- one shared toggle callback
            // and one shared option-click callback, dispatching by dropdownId rather than one pair
            // of callbacks per dropdown.
            c.BindEventCallback("option_toggle_dropdown",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                {
                    if (arguments.size() == 1)
                        RmlToggleDropdown(arguments[0].Get<int>(-1));
                });
            c.BindEventCallback("option_dropdown_option_click",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                {
                    if (arguments.size() == 2)
                        RmlDropdownOptionClick(arguments[0].Get<int>(-1), arguments[1].Get<int>(-1));
                });

            c.BindEventCallback("option_toggle_vsync",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlToggleVsync(); });

            c.BindEventCallback("option_toggle_disable_effects",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlToggleDisableEffects(); });
            c.BindEventCallback("option_toggle_disable_particles",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlToggleDisableParticles(); });
            c.BindEventCallback("option_toggle_disable_skill_effect_models",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlToggleDisableSkillEffectModels(); });
            c.BindEventCallback("option_toggle_disable_boids",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlToggleDisableBoids(); });
            c.BindEventCallback("option_toggle_disable_wing_shadow",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlToggleDisableWingShadow(); });

            c.BindEventCallback("option_toggle_show_fps_counter",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlToggleShowFpsCounter(); });
            c.BindEventCallback("option_toggle_show_debug_info",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlToggleShowDebugInfo(); });

        });

    (void)modelCreated;

    // Not draggable for now -- window_shell_header's id and GameConfig's "option_window" position
    // slot are still there, ready to wire back up via UI::RmlBridge::MakeDraggable() the same way
    // CMyInventory's #title does, once dragging is revisited.
    m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(),
        "Data/Interface/RmlUi/option_window.rml");
    m_pPanelEl = m_pRmlDoc ? m_pRmlDoc->GetElementById("panel") : nullptr;

    // Built directly here, not as option_window.rml markup -- a direct #panel child like
    // generic_confirm_dialog.rcss's own buttons, living in window_shell's shared footer anchor
    // (a sibling of #content, since #content is window_shell's only splice target and this button
    // must sit outside it). See m_pCloseButtonEl's own comment in OptionWindow.h for why this
    // isn't authored as `{{close_label}}` RML markup instead.
    m_pCloseButtonEl = nullptr;
    if (m_pRmlDoc)
    {
        if (Rml::Element* footer = m_pRmlDoc->GetElementById("window_shell_footer"))
        {
            Rml::ElementPtr btn = m_pRmlDoc->CreateElement("div");
            m_pCloseButtonEl = footer->AppendChild(std::move(btn));
            m_pCloseButtonEl->SetClassNames("btn btn-cancel option-close-btn");
            m_pCloseButtonEl->AddEventListener(Rml::EventId::Click, new ClickListener([this]() { RmlClickClose(); }));
        }
    }
    m_lastCloseButtonLabel.clear();
}

void mu::ui::window::COptionWindow::InitResolutionCombo()
{
    m_resolutions = UI::Options::NormalizeDisplayResolutions(MuGetSupportedDisplayResolutions());
    if (m_resolutions.empty())
    {
        const int fallbackWidth = WindowWidth > 0 ? static_cast<int>(WindowWidth) : CfgDefaults::CfgDefaultWindowWidth;
        const int fallbackHeight =
            WindowHeight > 0 ? static_cast<int>(WindowHeight) : CfgDefaults::CfgDefaultWindowHeight;
        m_resolutions.emplace_back(fallbackWidth, fallbackHeight);
    }

    m_iResolutionIndex = FindCurrentResolutionIndex();
    m_resolutionLabels.clear();
    m_resolutionLabels.reserve(m_resolutions.size());
    for (const auto& [width, height] : m_resolutions)
    {
        m_resolutionLabels.push_back(std::to_wstring(width) + L" x " + std::to_wstring(height));
    }
}

void mu::ui::window::COptionWindow::InitLanguageCombo()
{
    m_iLanguageIndex = FindCurrentLanguageIndex();
}

void mu::ui::window::COptionWindow::InitFontCombo()
{
    m_iFontIndex = FindCurrentFontIndex();
}

void mu::ui::window::COptionWindow::Release()
{
    UnloadImages();

    if (m_pRmlDoc)
    {
        m_pRmlDoc->Close();
        m_pRmlDoc = nullptr;
    }

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void mu::ui::window::COptionWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

void mu::ui::window::COptionWindow::Show(bool bShow)
{
    mu::ui::window::CObject::Show(bShow);
    if (m_pRmlDoc)
    {
        if (bShow) m_pRmlDoc->Show();
        else m_pRmlDoc->Hide();
    }
}

void mu::ui::window::COptionWindow::ReloadRmlTheme()
{
    // Same shape CGenericMenuDialog::ReloadRmlTheme() already establishes -- needed here because
    // this window's own UI Theme dropdown (Interface/UI tab) can trigger a theme switch from
    // inside itself, and CManager::ReloadAllRmlThemes() sweeps every registered window through
    // this override, this one included.
    if (!m_pRmlDoc)
        return;

    const bool wasVisible = m_pRmlDoc->IsVisible();
    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;
    m_pPanelEl = nullptr;

    BuildRmlUi();
    if (wasVisible)
    {
        SyncRmlModel();
        if (m_pRmlDoc)
            m_pRmlDoc->Show();
    }
}

bool mu::ui::window::COptionWindow::UpdateMouseEvent()
{
    // RmlUi's #panel owns all hit-testing for its own controls now; just claim the rest of the
    // window's own screen rect so a click here doesn't fall through to the world/scene behind it
    // -- this window isn't modal (unlike CGenericMenuDialog's UpdateMouseEvent(), which just
    // returns !IsVisible()), so a real rect is needed rather than blocking every click outright.
    // Read the panel's own live rendered position/size straight from RmlUi rather than
    // approximating them from hardcoded dp constants -- a hardcoded guess drifts from wherever
    // `.center-both`/window_shell actually puts the panel. INTERFACE_OPTION must map to
    // LayoutMode::Legacy (identity transform) in UILayoutPolicy.cpp's table, not the default
    // LayoutMode::Dialog (640x480-reference rescale) -- otherwise MouseX/MouseY is remapped into a
    // different coordinate space than this hit-test rect, and clicks fall through to
    // world/character movement instead.
    //
    // Defensive re-fetch: m_pPanelEl should already be valid whenever m_pRmlDoc is (BuildRmlUi()
    // sets both together), but if it's ever out of sync -- e.g. a future change re-parents/renames
    // #panel without updating this cache -- silently returning "unclaimed" here would reopen this
    // exact click-through bug with no diagnostic trail. Re-resolving costs one GetElementById() at
    // most, only in that already-broken case.
    if (!m_pPanelEl && m_pRmlDoc)
        m_pPanelEl = m_pRmlDoc->GetElementById("panel");

    if (m_pPanelEl)
    {
        const Rml::Vector2f offset = m_pPanelEl->GetAbsoluteOffset(Rml::BoxArea::Border);
        const int panelWidthPx = static_cast<int>(m_pPanelEl->GetOffsetWidth());
        const int panelHeightPx = static_cast<int>(m_pPanelEl->GetOffsetHeight());
        if (mu::ui::window::WindowGeometry(static_cast<int>(offset.x), static_cast<int>(offset.y), panelWidthPx,
                                            panelHeightPx)
                .Contains(MouseX, MouseY))
            return false;
    }

    return true;
}

bool mu::ui::window::COptionWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_OPTION) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_OPTION);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }

    return true;
}

bool mu::ui::window::COptionWindow::Update()
{
    // Outside any RmlUi event dispatch -- safe to destroy/rebuild m_pRmlDoc here if a theme switch
    // was requested (see m_bPendingThemeSwitch's own comment), and to resize the window for a new
    // UI scale (m_bPendingUIScaleApply's own comment).
    ApplyPendingThemeSwitch();
    ApplyPendingUIScale();
    SyncRmlModel();
    return true;
}

bool mu::ui::window::COptionWindow::Render()
{
    // RmlUi's #panel owns all chrome/text/control rendering now; nothing left to draw natively.
    return true;
}

float mu::ui::window::COptionWindow::GetLayerDepth()	//. 10.5f
{
    return 10.5f;
}

float mu::ui::window::COptionWindow::GetKeyEventOrder()	// 10.f;
{
    return 10.0f;
}

void mu::ui::window::COptionWindow::OpenningProcess()
{
    // Resync state that may have been changed externally while the window was hidden -- including
    // via the console-only $vsync/$effects .../$theme commands these same settings are now also
    // exposed through, so opening this window always reflects the current live truth.
    InitResolutionCombo();
    m_iLanguageIndex = FindCurrentLanguageIndex();
    m_iFontIndex = FindCurrentFontIndex();
    m_bWindowedMode = (g_bUseWindowMode == TRUE);

    m_bVsyncEnabled = GameConfig::GetInstance().GetVSyncEnabled();
    m_iFpsCapIndex = FindCurrentFpsCapIndex();

    m_bDisableEffects = GameConfig::GetInstance().GetDisableEffects();
    m_bDisableParticles = GameConfig::GetInstance().GetDisableParticles();
    m_bDisableSkillEffectModels = GameConfig::GetInstance().GetDisableSkillEffectModels();
    m_bDisableBoids = GameConfig::GetInstance().GetDisableBoids();
    m_bDisableWingShadow = GameConfig::GetInstance().GetDisableWingShadow();

    m_iThemeIndex = FindCurrentThemeIndex();
    m_iUIScaleIndex = FindCurrentUIScaleIndex();
}

void mu::ui::window::COptionWindow::ClosingProcess()
{
}

void mu::ui::window::COptionWindow::LoadImages()
{
    LoadBitmap(L"Interface\\newui_button_close.tga", IMAGE_OPTION_BTN_CLOSE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_OPTION_FRAME_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_OPTION_FRAME_DOWN, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_option_top.tga", IMAGE_OPTION_FRAME_UP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_option_back06(L).tga", IMAGE_OPTION_FRAME_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_option_back06(R).tga", IMAGE_OPTION_FRAME_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_option_line.jpg", IMAGE_OPTION_LINE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_option_point.tga", IMAGE_OPTION_POINT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_option_check.tga", IMAGE_OPTION_BTN_CHECK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_option_effect03.tga", IMAGE_OPTION_EFFECT_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_option_effect04.tga", IMAGE_OPTION_EFFECT_COLOR, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_option_volume01.tga", IMAGE_OPTION_VOLUME_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_option_volume02.tga", IMAGE_OPTION_VOLUME_COLOR, GL_LINEAR);
}

void mu::ui::window::COptionWindow::UnloadImages()
{
    DeleteBitmap(IMAGE_OPTION_BTN_CLOSE);
    DeleteBitmap(IMAGE_OPTION_FRAME_BACK);
    DeleteBitmap(IMAGE_OPTION_FRAME_DOWN);
    DeleteBitmap(IMAGE_OPTION_FRAME_UP);
    DeleteBitmap(IMAGE_OPTION_FRAME_LEFT);
    DeleteBitmap(IMAGE_OPTION_FRAME_RIGHT);
    DeleteBitmap(IMAGE_OPTION_LINE);
    DeleteBitmap(IMAGE_OPTION_POINT);
    DeleteBitmap(IMAGE_OPTION_BTN_CHECK);
    DeleteBitmap(IMAGE_OPTION_EFFECT_BACK);
    DeleteBitmap(IMAGE_OPTION_EFFECT_COLOR);
    DeleteBitmap(IMAGE_OPTION_VOLUME_BACK);
    DeleteBitmap(IMAGE_OPTION_VOLUME_COLOR);
}

void mu::ui::window::COptionWindow::RmlClickSelectTab(int nTab)
{
    if (nTab < 0 || nTab > 5)
        return;
    m_iActiveTab = nTab;
    PlayBuffer(SOUND_CLICK01);
}

void mu::ui::window::COptionWindow::RmlToggleDropdown(int dropdownId)
{
    if (dropdownId < 0 || dropdownId > 5)
        return;
    // Toggle: clicking the currently-open one's own value box closes it; clicking any other
    // (including a different dropdown's) overwrites m_iOpenDropdown, which closes whatever else
    // was open as a side effect -- see model.openDropdown's own comment for why a single field
    // gives exclusivity for free.
    m_iOpenDropdown = (m_iOpenDropdown == dropdownId) ? -1 : dropdownId;
    PlayBuffer(SOUND_CLICK01);
}

void mu::ui::window::COptionWindow::RmlDropdownOptionClick(int dropdownId, int optionIndex)
{
    switch (dropdownId)
    {
    case 0:
        RmlResolutionChanged(optionIndex);
        break;
    case 1:
        RmlFpsCapChanged(optionIndex);
        break;
    case 2:
        RmlThemeChanged(optionIndex);
        break;
    case 3:
        RmlLanguageChanged(optionIndex);
        break;
    case 4:
        RmlFontChanged(optionIndex);
        break;
    case 5:
        RmlUIScaleChanged(optionIndex);
        break;
    default:
        break;
    }
    m_iOpenDropdown = -1;
}

void mu::ui::window::COptionWindow::RmlToggleAutoAttack()
{
    m_bAutoAttack = !m_bAutoAttack;
}

void mu::ui::window::COptionWindow::RmlToggleWhisperSound()
{
    m_bWhisperSound = !m_bWhisperSound;
}

void mu::ui::window::COptionWindow::RmlToggleSlideHelp()
{
    m_bSlideHelp = !m_bSlideHelp;
}

void mu::ui::window::COptionWindow::RmlToggleRenderAllEffects()
{
    m_bRenderAllEffects = !m_bRenderAllEffects;
}

void mu::ui::window::COptionWindow::RmlToggleWindowedMode()
{
    m_bWindowedMode = !m_bWindowedMode;
    ApplyWindowModeToggle();
}

void mu::ui::window::COptionWindow::RmlSoundVolumeChanged(int value)
{
    m_iVolumeLevel = std::clamp(value, 0, 10);
    OnSoundVolumeChanged();
}

void mu::ui::window::COptionWindow::RmlMusicVolumeChanged(int value)
{
    m_iMusicLevel = std::clamp(value, 0, 10);
    OnMusicVolumeChanged();
}

void mu::ui::window::COptionWindow::RmlRenderLevelChanged(int value)
{
    m_iRenderLevel = std::clamp(value, 0, 5);
}

void mu::ui::window::COptionWindow::RmlResolutionChanged(int index)
{
    // Only ever called from RmlDropdownOptionClick(), itself only reachable from an explicit
    // option click -- no self-firing spurious-event risk the way RmlUi's native <select>
    // (WidgetDropDown) had (this settle-frame guard predates the custom-dropdown rewrite, kept as
    // a harmless no-op-this-early safety net rather than removed, see m_rmlSyncCount's own
    // comment).
    if (m_rmlSyncCount < kRmlSelectSettleFrames)
        return;
    if (index < 0 || index >= static_cast<int>(m_resolutions.size()))
        return;
    if (index == m_iResolutionIndex)
        return;
    m_iResolutionIndex = index;
    ApplyResolution();
}

void mu::ui::window::COptionWindow::RmlLanguageChanged(int index)
{
    // See RmlResolutionChanged's own comment -- same settle-frame guard applies here.
    if (m_rmlSyncCount < kRmlSelectSettleFrames)
        return;
    if (index < 0 || index >= s_NumLanguages)
        return;
    m_iLanguageIndex = index;
    ApplyLanguage();
}

void mu::ui::window::COptionWindow::RmlFontChanged(int index)
{
    // See RmlResolutionChanged's own comment -- same settle-frame guard applies here.
    if (m_rmlSyncCount < kRmlSelectSettleFrames)
        return;
    if (index < 0 || index >= s_NumFonts)
        return;
    m_iFontIndex = index;
    ApplyFont();
}

void mu::ui::window::COptionWindow::RmlToggleVsync()
{
    m_bVsyncEnabled = !m_bVsyncEnabled;
    // Persists + applies live next frame -- see this function's own doc comment (Winmain.cpp).
    // Calling GameConfig::SetVSyncEnabled() directly here would persist without applying.
    MuSetVSyncPreference(m_bVsyncEnabled);
}

void mu::ui::window::COptionWindow::RmlFpsCapChanged(int index)
{
    // See RmlResolutionChanged's own comment -- same settle-frame guard applies here.
    if (m_rmlSyncCount < kRmlSelectSettleFrames)
        return;
    if (index < 0 || index >= s_NumFpsCapValues)
        return;
    if (index == m_iFpsCapIndex)
        return;
    m_iFpsCapIndex = index;

    const int fps = s_FpsCapValues[index];
    // Unconditional, same as the console-only `$fps <N>` diagnostic this replaces -- it doesn't
    // check VSync state either; if VSync is on, this can transiently override it until the next
    // VSync toggle re-asserts -1 (a pre-existing characteristic of SetTargetFps, not new here).
    SetTargetFps(fps);
    GameConfig::GetInstance().SetFpsCap(fps);
    GameConfig::GetInstance().Save();
}

void mu::ui::window::COptionWindow::RmlToggleDisableEffects()
{
    m_bDisableEffects = !m_bDisableEffects;
    SetDisableEffects(m_bDisableEffects);
    GameConfig::GetInstance().SetDisableEffects(m_bDisableEffects);
    GameConfig::GetInstance().Save();
}

void mu::ui::window::COptionWindow::RmlToggleDisableParticles()
{
    m_bDisableParticles = !m_bDisableParticles;
    SetDisableParticles(m_bDisableParticles);
    GameConfig::GetInstance().SetDisableParticles(m_bDisableParticles);
    GameConfig::GetInstance().Save();
}

void mu::ui::window::COptionWindow::RmlToggleDisableSkillEffectModels()
{
    m_bDisableSkillEffectModels = !m_bDisableSkillEffectModels;
    SetDisableSkillEffectModels(m_bDisableSkillEffectModels);
    GameConfig::GetInstance().SetDisableSkillEffectModels(m_bDisableSkillEffectModels);
    GameConfig::GetInstance().Save();
}

void mu::ui::window::COptionWindow::RmlToggleDisableBoids()
{
    m_bDisableBoids = !m_bDisableBoids;
    SetDisableBoids(m_bDisableBoids);
    GameConfig::GetInstance().SetDisableBoids(m_bDisableBoids);
    GameConfig::GetInstance().Save();
}

void mu::ui::window::COptionWindow::RmlToggleDisableWingShadow()
{
    m_bDisableWingShadow = !m_bDisableWingShadow;
    SetDisableWingShadow(m_bDisableWingShadow);
    GameConfig::GetInstance().SetDisableWingShadow(m_bDisableWingShadow);
    GameConfig::GetInstance().Save();
}

void mu::ui::window::COptionWindow::RmlToggleShowFpsCounter()
{
    // No member/persistence -- SetShowFpsCounter() also clears ShowDebugInfo internally
    // (mutually exclusive), and SyncRmlModel() reads both live every frame, so the other
    // checkbox reflects the change on its own without any extra bookkeeping here.
    SetShowFpsCounter(!GetShowFpsCounter());
}

void mu::ui::window::COptionWindow::RmlToggleShowDebugInfo()
{
    SetShowDebugInfo(!GetShowDebugInfo());
}

void mu::ui::window::COptionWindow::RmlThemeChanged(int index)
{
    // See RmlResolutionChanged's own comment -- same settle-frame guard applies here.
    if (m_rmlSyncCount < kRmlSelectSettleFrames)
        return;
    if (index < 0 || index > 1)
        return;
    if (index == m_iThemeIndex)
        return;

    // Deferred to Update() -- see m_bPendingThemeSwitch's own comment (OptionWindow.h) for why
    // this can't run synchronously here: it would destroy m_pRmlDoc mid-dispatch of the very
    // dropdown-option click event that called this (RmlDropdownOptionClick(), itself invoked from
    // a data-event-click on one of the option divs this document is about to unload).
    m_iPendingThemeIndex = index;
    m_bPendingThemeSwitch = true;
}

void mu::ui::window::COptionWindow::ApplyPendingThemeSwitch()
{
    if (!m_bPendingThemeSwitch)
        return;
    m_bPendingThemeSwitch = false;

    const int index = m_iPendingThemeIndex;
    m_iThemeIndex = index;

    // Exact sequence `$theme <legacy|modern>` (muConsoleDebug.cpp) already uses at runtime, plus
    // Save() -- that command is explicitly session-only, this UI control should persist.
    const std::string themeName = (index == 1) ? "modern" : "legacy";
    if (!UI::RmlBridge::ThemeExists(themeName))
        return;

    GameConfig::GetInstance().SetRmlTheme(StringUtils::NarrowToWide(themeName));
    GameConfig::GetInstance().Save();
    UI::RmlBridge::SetActiveThemeName(themeName);
    CSceneUICoordinator::Instance().GetNewStyleMng().ReloadAllRmlThemes();
    if (mu::ui::window::CManager* newUIMng = g_pNewUIMng)
        newUIMng->ReloadAllRmlThemes();
    UI::Login::ReloadRmlTheme();
}

void mu::ui::window::COptionWindow::RmlUIScaleChanged(int index)
{
    // See RmlResolutionChanged's own comment -- same settle-frame guard applies here.
    if (m_rmlSyncCount < kRmlSelectSettleFrames)
        return;
    const auto& choices = UI::Options::UIScalePercentChoices();
    if (index < 0 || index >= static_cast<int>(choices.size()))
        return;
    if (index == m_iUIScaleIndex)
        return;

    m_iUIScaleIndex = index;
    // Deferred to Update() -- see m_bPendingUIScaleApply's own comment (OptionWindow.h).
    m_iPendingUIScalePercent = choices[index];
    m_bPendingUIScaleApply = true;
}

void mu::ui::window::COptionWindow::ApplyPendingUIScale()
{
    if (!m_bPendingUIScaleApply)
        return;
    m_bPendingUIScaleApply = false;

    GameConfig::GetInstance().SetUIScalePercent(m_iPendingUIScalePercent);
    GameConfig::GetInstance().Save();
    // Same re-apply the `ui scale` developer command uses: nothing recomputes the scale on its own,
    // but every resolution-dependent system (RmlUi's dp ratio on all three contexts, the legacy
    // CWin layout, the 3D UI cameras) does so on a resize, so a resize to the size the window
    // already has is what makes the new scale take effect live. Deliberately NOT followed by
    // GameConfig::SetWindowSize()/Save() the way ApplyResolution() is: the window size does not
    // change here, and persisting it from this path would write back whatever size the resize
    // settled on instead of the user's own [Window] setting.
    MuApplyWindowResolution(WindowWidth, WindowHeight, g_bUseWindowMode != FALSE);

    // The clamp may have coerced the request (a choice outside GameConfig's bounds can't happen
    // today, but the ladder and the clamp are independent constants), so re-seed from what was
    // actually stored rather than from the clicked index.
    m_iUIScaleIndex = FindCurrentUIScaleIndex();
}

void mu::ui::window::COptionWindow::RmlClickClose()
{
    g_pNewUISystem->Hide(mu::ui::window::INTERFACE_OPTION);
    PlayBuffer(SOUND_CLICK01);
}

void mu::ui::window::COptionWindow::OnSoundVolumeChanged()
{
    m_SoundOnOff = (m_iVolumeLevel > 0) ? 1 : 0;
    SetEffectVolumeLevel(m_iVolumeLevel);
    GameConfig::GetInstance().SetSoundVolume(m_iVolumeLevel);
    GameConfig::GetInstance().Save();
}

void mu::ui::window::COptionWindow::OnMusicVolumeChanged()
{
    // Mute via volume only, not by stopping the stream -- stopping loses the current track until
    // the next scene change, so raising the slider back up would stay silent.
    m_MusicOnOff = (m_iMusicLevel > 0) ? 1 : 0;

    AudioPlayer::SetMusicVolume(m_iMusicLevel);

    GameConfig::GetInstance().SetMusicVolume(m_iMusicLevel);
    GameConfig::GetInstance().Save();
}

void mu::ui::window::COptionWindow::SetAutoAttack(bool bAuto)
{
    m_bAutoAttack = bAuto;
}

bool mu::ui::window::COptionWindow::IsAutoAttack()
{
    return m_bAutoAttack;
}

void mu::ui::window::COptionWindow::SetWhisperSound(bool bSound)
{
    m_bWhisperSound = bSound;
}

bool mu::ui::window::COptionWindow::IsWhisperSound()
{
    return m_bWhisperSound;
}

void mu::ui::window::COptionWindow::SetSlideHelp(bool bHelp)
{
    m_bSlideHelp = bHelp;
}

bool mu::ui::window::COptionWindow::IsSlideHelp()
{
    return m_bSlideHelp;
}

void mu::ui::window::COptionWindow::SetVolumeLevel(int iVolume)
{
    m_iVolumeLevel = iVolume;
}

int mu::ui::window::COptionWindow::GetVolumeLevel()
{
    return m_iVolumeLevel;
}

void mu::ui::window::COptionWindow::SetRenderLevel(int iRender)
{
    m_iRenderLevel = iRender;
}

int mu::ui::window::COptionWindow::GetRenderLevel()
{
    return m_iRenderLevel;
}

void mu::ui::window::COptionWindow::SetRenderAllEffects(bool bRenderAllEffects)
{
    m_bRenderAllEffects = bRenderAllEffects;
}

bool mu::ui::window::COptionWindow::GetRenderAllEffects()
{
    return m_bRenderAllEffects;
}

int mu::ui::window::COptionWindow::FindCurrentResolutionIndex()
{
    return UI::Options::FindClosestDisplayResolutionIndex(m_resolutions, static_cast<int>(WindowWidth),
                                                          static_cast<int>(WindowHeight));
}

int mu::ui::window::COptionWindow::FindCurrentLanguageIndex()
{
    const char* current = I18N::GetCurrentLocale();
    if (current == nullptr) return 0;
    for (int i = 0; i < s_NumLanguages; ++i)
    {
        if (std::strcmp(s_Languages[i].code, current) == 0)
            return i;
    }
    return 0;  // default to English
}

void mu::ui::window::COptionWindow::ApplyLanguage()
{
    const char* code = s_Languages[m_iLanguageIndex].code;

    // Wide string round-trips through GameConfig's string I/O; locale codes are ASCII-safe.
    std::wstring wide(code, code + std::strlen(code));

    // Re-selecting the active language is a no-op; skip the relocalize and disk write.
    if (GameConfig::GetInstance().GetUILocale() == wide)
        return;

    I18N::SetLocale(code);
    GameConfig::GetInstance().SetUILocale(wide);
    GameConfig::GetInstance().Save();
}

int mu::ui::window::COptionWindow::FindCurrentFontIndex()
{
    const std::wstring current = GameConfig::GetInstance().GetFontSelection();
    for (int i = 0; i < s_NumFonts; ++i)
    {
        if (current == s_Fonts[i].name)
            return i;
    }
    return 0;  // default ("")
}

void mu::ui::window::COptionWindow::ApplyFont()
{
    // Re-selecting the active font is a no-op; skip the font rebuild and disk write.
    if (GameConfig::GetInstance().GetFontSelection() == s_Fonts[m_iFontIndex].name)
        return;

    GameConfig::GetInstance().SetFontSelection(s_Fonts[m_iFontIndex].name);
    // Recreate the GDI fonts from config so the change takes effect live.
    ReinitializeFonts();
    GameConfig::GetInstance().Save();
}

int mu::ui::window::COptionWindow::FindCurrentFpsCapIndex()
{
    const int fpsCap = GameConfig::GetInstance().GetFpsCap();
    for (int i = 0; i < s_NumFpsCapValues; ++i)
    {
        if (s_FpsCapValues[i] == fpsCap)
            return i;
    }
    return s_NumFpsCapValues - 1;  // "Uncapped" -- last entry, matches CfgDefaultFpsCap (-1)
}

int mu::ui::window::COptionWindow::FindCurrentThemeIndex()
{
    return UI::RmlBridge::GetActiveThemeName() == "modern" ? 1 : 0;
}

int mu::ui::window::COptionWindow::FindCurrentUIScaleIndex()
{
    return UI::Options::FindClosestUIScaleIndex(GameConfig::GetInstance().GetUIScalePercent());
}

void mu::ui::window::COptionWindow::ApplyResolution()
{
    if (m_iResolutionIndex < 0 || m_iResolutionIndex >= static_cast<int>(m_resolutions.size()))
    {
        return;
    }

    const auto [newWidth, newHeight] = m_resolutions[m_iResolutionIndex];

    // Resize through SDL (MuApplyWindowResolution), not raw Win32 SetWindowPos -- SDL clamps a
    // non-resizable window straight back to its current size otherwise.
    MuApplyWindowResolution(static_cast<unsigned int>(newWidth), static_cast<unsigned int>(newHeight),
                            g_bUseWindowMode != FALSE);

    // SDL may have coerced the request; WindowWidth/Height now hold the actual result, so
    // persist that and snap the selection to it.
    SyncResolutionComboToWindow();
    GameConfig::GetInstance().SetWindowSize(WindowWidth, WindowHeight);
    GameConfig::GetInstance().Save();
}

// Points the resolution selection at the window's real size; if it's not a listed mode, keeps the
// current selection (config still records the real size).
void mu::ui::window::COptionWindow::SyncResolutionComboToWindow()
{
    const int listed = UI::Options::FindExactDisplayResolutionIndex(m_resolutions, static_cast<int>(WindowWidth),
                                                                    static_cast<int>(WindowHeight));
    if (listed < 0)
        return;
    m_iResolutionIndex = listed;
}

// Switches windowed/fullscreen through SDL (MuApplyWindowResolution), not raw Win32 calls --
// driving the OS directly left SDL's state inconsistent with later resolution changes.
void mu::ui::window::COptionWindow::ApplyWindowModeToggle()
{
    g_bUseWindowMode = m_bWindowedMode ? TRUE : FALSE;
    GameConfig::GetInstance().SetWindowMode(m_bWindowedMode);

    MuApplyWindowResolution(WindowWidth, WindowHeight, m_bWindowedMode);

    // The mode switch may have coerced the size, so save after applying it.
    SyncResolutionComboToWindow();
    GameConfig::GetInstance().SetWindowSize(WindowWidth, WindowHeight);
    GameConfig::GetInstance().Save();
}

void mu::ui::window::COptionWindow::SyncRmlModel()
{
    if (!m_pRmlDoc)
        return;

    // Saturates rather than wrapping; only ever compared against kRmlSelectSettleFrames below.
    if (m_rmlSyncCount < 1000)
        ++m_rmlSyncCount;

    auto& model = m_RmlBinder.GetModel();

    // Diffed first, matching MyQuestInfoWindow::SyncRmlModel()'s own ordering for its activeTab.
    if (model.activeTab != m_iActiveTab) { model.activeTab = m_iActiveTab; m_RmlBinder.MarkDirty("active_tab"); }
    if (model.openDropdown != m_iOpenDropdown) { model.openDropdown = m_iOpenDropdown; m_RmlBinder.MarkDirty("open_dropdown"); }

    // Re-fetched every sync, not just once at BuildRmlUi() time -- native re-rendered every one of
    // these from the live I18N::Game::* pointer every frame, so a language switch made from this
    // very window relabels it immediately rather than only on next reopen.
    const auto syncLabel = [this](Rml::String& field, const char* fieldName, const wchar_t* text)
    {
        const Rml::String narrow = StringUtils::WideToNarrow(text);
        if (field != narrow)
        {
            field = narrow;
            m_RmlBinder.MarkDirty(fieldName);
        }
    };
    // .option-dropdown's own closed-state box shows this instead of a {{}} array-index expression
    // -- see model.resolutionValueLabel's own comment (OptionWindow.h) for why.
    const auto syncDropdownValue = [this](Rml::String& field, const char* fieldName,
                                            const std::vector<Rml::String>& labels, int index)
    {
        const Rml::String value =
            (index >= 0 && index < static_cast<int>(labels.size())) ? labels[index] : Rml::String();
        if (field != value)
        {
            field = value;
            m_RmlBinder.MarkDirty(fieldName);
        }
    };
    syncLabel(model.title, "title", I18N::Game::Option385);
    // Imperative, not a bound field -- see m_pCloseButtonEl's own comment for why.
    if (m_pCloseButtonEl)
    {
        const Rml::String closeLabelText = StringUtils::WideToNarrow(I18N::Game::Close);
        if (m_lastCloseButtonLabel != closeLabelText)
        {
            m_lastCloseButtonLabel = closeLabelText;
            m_pCloseButtonEl->SetInnerRML(closeLabelText);
        }
    }
    syncLabel(model.tabGameplayLabel, "tab_gameplay_label", I18N::Game::Gameplay);
    syncLabel(model.tabAudioLabel, "tab_audio_label", I18N::Game::Audio);
    syncLabel(model.tabVideoLabel, "tab_video_label", I18N::Game::Video);
    syncLabel(model.tabGraphicsLabel, "tab_graphics_label", I18N::Game::Graphics);
    syncLabel(model.tabUiLabel, "tab_ui_label", I18N::Game::UI);
    syncLabel(model.tabGeneralLabel, "tab_general_label", I18N::Game::General);
    syncLabel(model.autoAttackLabel, "auto_attack_label", I18N::Game::AutomaticAttack);
    syncLabel(model.whisperSoundLabel, "whisper_sound_label", I18N::Game::BeepSoundForWhispering);
    syncLabel(model.slideHelpLabel, "slide_help_label", I18N::Game::SlideHelp);
    syncLabel(model.renderAllEffectsLabel, "render_all_effects_label", I18N::Game::RenderFullEffects);
    syncLabel(model.windowedModeLabel, "windowed_mode_label", I18N::Game::WindowedMode);
    syncLabel(model.soundVolumeLabel, "sound_volume_label", I18N::Game::SoundVolume);
    syncLabel(model.musicVolumeLabel, "music_volume_label", I18N::Game::MusicVolume);
    syncLabel(model.renderLevelLabel, "render_level_label", I18N::Game::EffectLimitation);
    syncLabel(model.resolutionRowLabel, "resolution_row_label", I18N::Game::Resolution);
    syncLabel(model.languageRowLabel, "language_row_label", I18N::Game::Language);
    syncLabel(model.fontRowLabel, "font_row_label", I18N::Game::Font);
    syncLabel(model.vsyncLabel, "vsync_label", I18N::Game::VSync);
    syncLabel(model.fpsCapRowLabel, "fps_cap_row_label", I18N::Game::FPSLimit);
    syncLabel(model.disableEffectsLabel, "disable_effects_label", I18N::Game::DisableEffects);
    syncLabel(model.disableParticlesLabel, "disable_particles_label", I18N::Game::DisableParticles);
    syncLabel(model.disableSkillEffectModelsLabel, "disable_skill_effect_models_label",
              I18N::Game::DisableSkillEffectModels);
    syncLabel(model.disableBoidsLabel, "disable_boids_label", I18N::Game::DisableAmbientWildlife);
    syncLabel(model.disableWingShadowLabel, "disable_wing_shadow_label", I18N::Game::DisableWingShadow);
    syncLabel(model.showFpsCounterLabel, "show_fps_counter_label", I18N::Game::ShowFPSCounter);
    syncLabel(model.showDebugInfoLabel, "show_debug_info_label", I18N::Game::ShowDebugInfo);
    syncLabel(model.themeRowLabel, "theme_row_label", I18N::Game::UITheme);
    syncLabel(model.uiScaleRowLabel, "ui_scale_row_label", I18N::Game::UIScale);
    syncLabel(model.uiScaleTooltip, "ui_scale_tooltip", I18N::Game::UIScaleTooltip);

    // positioned/root_x/root_y stay at their model defaults (false/0/0, see OptionRmlModel's own
    // comment) -- nothing to sync while dragging is off; window_shell's `.center-both` CSS owns
    // positioning entirely, same as every other window_shell consumer.

    if (model.autoAttack != m_bAutoAttack) { model.autoAttack = m_bAutoAttack; m_RmlBinder.MarkDirty("auto_attack"); }
    if (model.whisperSound != m_bWhisperSound) { model.whisperSound = m_bWhisperSound; m_RmlBinder.MarkDirty("whisper_sound"); }
    if (model.slideHelp != m_bSlideHelp) { model.slideHelp = m_bSlideHelp; m_RmlBinder.MarkDirty("slide_help"); }
    if (model.renderAllEffects != m_bRenderAllEffects) { model.renderAllEffects = m_bRenderAllEffects; m_RmlBinder.MarkDirty("render_all_effects"); }
    if (model.windowedMode != m_bWindowedMode) { model.windowedMode = m_bWindowedMode; m_RmlBinder.MarkDirty("windowed_mode"); }

    if (model.soundVolume != m_iVolumeLevel) { model.soundVolume = m_iVolumeLevel; m_RmlBinder.MarkDirty("sound_volume"); }
    if (model.musicVolume != m_iMusicLevel) { model.musicVolume = m_iMusicLevel; m_RmlBinder.MarkDirty("music_volume"); }
    if (model.renderLevel != m_iRenderLevel) { model.renderLevel = m_iRenderLevel; m_RmlBinder.MarkDirty("render_level"); }

    std::vector<Rml::String> newResolutionLabels;
    newResolutionLabels.reserve(m_resolutionLabels.size());
    for (const auto& label : m_resolutionLabels)
        newResolutionLabels.push_back(StringUtils::WideToNarrow(label.c_str()));
    if (newResolutionLabels != model.resolutionLabels)
    {
        model.resolutionLabels = std::move(newResolutionLabels);
        m_RmlBinder.MarkDirty("resolution_labels");
    }
    if (model.resolutionIndex != m_iResolutionIndex) { model.resolutionIndex = m_iResolutionIndex; m_RmlBinder.MarkDirty("resolution_index"); }
    syncDropdownValue(model.resolutionValueLabel, "resolution_value_label", model.resolutionLabels, model.resolutionIndex);

    if (model.languageLabels.empty())
    {
        model.languageLabels.reserve(s_NumLanguages);
        for (int i = 0; i < s_NumLanguages; ++i)
            model.languageLabels.push_back(StringUtils::WideToNarrow(s_Languages[i].label));
        m_RmlBinder.MarkDirty("language_labels");
    }
    if (model.languageIndex != m_iLanguageIndex) { model.languageIndex = m_iLanguageIndex; m_RmlBinder.MarkDirty("language_index"); }
    syncDropdownValue(model.languageValueLabel, "language_value_label", model.languageLabels, model.languageIndex);

    // Rebuilt every sync (not just once) so the localized "Default" entry follows a live language
    // switch, same as native's own GetFontLabels().
    std::vector<Rml::String> newFontLabels;
    newFontLabels.reserve(s_NumFonts);
    newFontLabels.push_back(StringUtils::WideToNarrow(I18N::Game::DefaultFont));
    for (int i = 1; i < s_NumFonts; ++i)
        newFontLabels.push_back(StringUtils::WideToNarrow(s_Fonts[i].label));
    if (newFontLabels != model.fontLabels)
    {
        model.fontLabels = std::move(newFontLabels);
        m_RmlBinder.MarkDirty("font_labels");
    }
    if (model.fontIndex != m_iFontIndex) { model.fontIndex = m_iFontIndex; m_RmlBinder.MarkDirty("font_index"); }
    syncDropdownValue(model.fontValueLabel, "font_value_label", model.fontLabels, model.fontIndex);

    if (model.vsyncEnabled != m_bVsyncEnabled) { model.vsyncEnabled = m_bVsyncEnabled; m_RmlBinder.MarkDirty("vsync_enabled"); }

    // Rebuilt every sync (not just once), same reasoning as font_labels' own "Default" entry --
    // the localized "Uncapped" entry needs to follow a live language switch.
    std::vector<Rml::String> newFpsCapLabels;
    newFpsCapLabels.reserve(s_NumFpsCapValues);
    for (int i = 0; i < s_NumFpsCapValues; ++i)
    {
        newFpsCapLabels.push_back(s_FpsCapValues[i] < 0
            ? StringUtils::WideToNarrow(I18N::Game::Uncapped)
            : StringUtils::WideToNarrow((std::to_wstring(s_FpsCapValues[i]) + L" FPS").c_str()));
    }
    if (newFpsCapLabels != model.fpsCapLabels)
    {
        model.fpsCapLabels = std::move(newFpsCapLabels);
        m_RmlBinder.MarkDirty("fps_cap_labels");
    }
    if (model.fpsCapIndex != m_iFpsCapIndex) { model.fpsCapIndex = m_iFpsCapIndex; m_RmlBinder.MarkDirty("fps_cap_index"); }
    syncDropdownValue(model.fpsCapValueLabel, "fps_cap_value_label", model.fpsCapLabels, model.fpsCapIndex);

    if (model.disableEffects != m_bDisableEffects) { model.disableEffects = m_bDisableEffects; m_RmlBinder.MarkDirty("disable_effects"); }
    if (model.disableParticles != m_bDisableParticles) { model.disableParticles = m_bDisableParticles; m_RmlBinder.MarkDirty("disable_particles"); }
    if (model.disableSkillEffectModels != m_bDisableSkillEffectModels) { model.disableSkillEffectModels = m_bDisableSkillEffectModels; m_RmlBinder.MarkDirty("disable_skill_effect_models"); }
    if (model.disableBoids != m_bDisableBoids) { model.disableBoids = m_bDisableBoids; m_RmlBinder.MarkDirty("disable_boids"); }
    if (model.disableWingShadow != m_bDisableWingShadow) { model.disableWingShadow = m_bDisableWingShadow; m_RmlBinder.MarkDirty("disable_wing_shadow"); }

    // Read live, not from a member -- SetShowFpsCounter()/SetShowDebugInfo() (SceneManager.h) are
    // mutually exclusive, so this reflects either checkbox toggling the other one off immediately,
    // including when toggled via the $fpscounter/$details console commands instead of this window.
    const bool showFpsCounter = GetShowFpsCounter();
    const bool showDebugInfo = GetShowDebugInfo();
    if (model.showFpsCounter != showFpsCounter) { model.showFpsCounter = showFpsCounter; m_RmlBinder.MarkDirty("show_fps_counter"); }
    if (model.showDebugInfo != showDebugInfo) { model.showDebugInfo = showDebugInfo; m_RmlBinder.MarkDirty("show_debug_info"); }

    // Rebuilt every sync, same reasoning as fps_cap_labels above.
    std::vector<Rml::String> newThemeLabels = {
        StringUtils::WideToNarrow(I18N::Game::Legacy),
        StringUtils::WideToNarrow(I18N::Game::Modern),
    };
    if (newThemeLabels != model.themeLabels)
    {
        model.themeLabels = std::move(newThemeLabels);
        m_RmlBinder.MarkDirty("theme_labels");
    }
    if (model.themeIndex != m_iThemeIndex) { model.themeIndex = m_iThemeIndex; m_RmlBinder.MarkDirty("theme_index"); }
    syncDropdownValue(model.themeValueLabel, "theme_value_label", model.themeLabels, model.themeIndex);

    // Built once -- unlike the font/fps-cap/theme lists, no entry is a localized word, so a live
    // language switch can't change any of these labels. The space before the sign is deliberate
    // ("100 %"), matching the "60 FPS" spacing of the FPS Limit row above.
    if (model.uiScaleLabels.empty())
    {
        const auto& choices = UI::Options::UIScalePercentChoices();
        model.uiScaleLabels.reserve(choices.size());
        for (const int percent : choices)
            model.uiScaleLabels.push_back(StringUtils::WideToNarrow((std::to_wstring(percent) + L" %").c_str()));
        m_RmlBinder.MarkDirty("ui_scale_labels");
    }
    if (model.uiScaleIndex != m_iUIScaleIndex)
    {
        model.uiScaleIndex = m_iUIScaleIndex;
        m_RmlBinder.MarkDirty("ui_scale_index");
    }
    syncDropdownValue(model.uiScaleValueLabel, "ui_scale_value_label", model.uiScaleLabels, model.uiScaleIndex);
}
