
#include "stdafx.h"
#include "UI/Options/OptionWindow.h"
#include "UI/Core/WindowSystem.h"
#include "UI/RmlBridge/RmlPointer.h"
#include "Render/Textures/TexturePack.h"
#include "Render/Textures/ZzzTexture.h"
#include "Audio/DSPlaySound.h"
#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameConfig/GameConfig.h"
#include "Data/GameConfig/GameConfigConstants.h"
#include "Render/Look/LookPresetSetting.h"
#include "Render/Look/LookProfileSetting.h"
#include "Render/Look/TuningSetting.h"
#include "Render/Lighting/DynamicLightsSetting.h"
#include "Render/Lighting/EnhancedLightingSetting.h"
#include "Render/Lighting/LightMoodScales.h"
#include "Render/PostProcessing/PostProcessingSetting.h"
#include "UI/Options/TuningRows.h"
#include "Audio/AudioPlayer.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlDraggable.h"
#include "UI/RmlBridge/RmlLevelGauge.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/Theme/ThemeSelection.h"
#include "UI/RmlBridge/RmlTooltip.h"
#include "Core/Utilities/StringUtils.h"
#include "Scenes/SceneManager.h"
#include "Scenes/MainScene.h"
#include "App/Platform/Windows/Winmain.h"
#include "UI/Core/SceneUICoordinator.h"
#include "UI/Windows/RememberPasswordPrompt.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <optional>
#include <cstring>
#include "I18N/All.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/ElementUtilities.h>
#include <RmlUi/Core/Event.h>
#include <RmlUi/Core/EventListener.h>
#include <RmlUi/Core/ScrollTypes.h>
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
    // Ascending, spanning GameConfig's clamp (CfgMinUIScalePercent..CfgMaxUIScalePercent); 100 (the
    // default) is one of them so "back to normal" is a single click.
    static const std::vector<int> choices = {75, 80, 90, 100, 125, 150};
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
// clang-format off
static const struct { const char* code; const wchar_t* label; } s_Languages[] = {
    { "en",    L"English" },
    { "de",    L"Deutsch" },
    { "es",    L"Español" },                                                  // Español
    { "id",    L"Bahasa Indonesia" },
    { "ja",    L"日本語" },                                       // 日本語
    { "pl",    L"Polski" },
    { "pt",    L"Português" },                                                // Português
    { "ru",    L"Русский" },                   // Русский
    { "tl",    L"Tagalog" },
    { "uk",    L"Українська" }, // Українська
    { "zh-CN", L"简体中文" },
    { "zh-TW", L"繁體中文" },                                      // 繁體中文
};
// clang-format on
static const int s_NumLanguages = sizeof(s_Languages) / sizeof(s_Languages[0]);

// UI font families offered by the font row. `name` is the GameConfig font family value; empty =
// platform default. Curated entries are bundled in ./fonts so they resolve without a system
// install.
namespace
{
struct FontOption
{
    const wchar_t* name;
    const wchar_t* label;
};
} // namespace

static const FontOption s_Fonts[] = {
    {L"", L"Default"},
    {L"Liberation Sans", L"Liberation Sans"},
    {L"DejaVu Sans", L"DejaVu Sans"},
    {L"Noto Sans TC", L"Noto Sans TC"},
    {L"Noto Sans SC", L"Noto Sans SC"},
};
static const int s_NumFonts = sizeof(s_Fonts) / sizeof(s_Fonts[0]);

// FPS cap choices offered by the Video tab's FPS Limit row -- -1 matches SceneManager::SetTargetFps's
// own "uncapped" sentinel and GameConfig::GetFpsCap()'s default, so "Uncapped" doubles as this
// setting's untouched-by-the-user state (see Winmain.cpp's EffectiveOffVSyncTargetFps()).
static const int s_FpsCapValues[] = { 24, 30, 60, 120, 144, -1 };
static const int s_NumFpsCapValues = sizeof(s_FpsCapValues) / sizeof(s_FpsCapValues[0]);

// The Video tab's Anti-aliasing and Anisotropic filtering rows, in the values config.ini holds
// ([Render] Antialiasing, Anisotropy); the first entry of each is "Off".
static constexpr int s_AntialiasingValues[] = {0, 2, 4, 8};
static constexpr int s_AnisotropyValues[] = {1, 2, 4, 8, 16};

namespace
{
// The row's entry for a config value: the largest entry at or below it, as GameConfig rounds.
template <std::size_t N> int FindValueIndex(const int (&values)[N], int value)
{
    int index = 0;
    for (int i = 0; i < static_cast<int>(N); ++i)
    {
        if (values[i] <= value)
            index = i;
    }
    return index;
}

template <std::size_t N> std::vector<Rml::String> MultiplierLabels(const int (&values)[N])
{
    std::vector<Rml::String> labels;
    labels.reserve(N);
    labels.push_back(StringUtils::WideToNarrow(I18N::Game::Off));
    for (std::size_t i = 1; i < N; ++i)
        labels.push_back(std::to_string(values[i]) + "x");
    return labels;
}

// A level no tuning track has (they start at 0), for ApplyLevelGaugeEvent's current level. The
// sliders bind no wheel event, the only one the gauge would step from it.
constexpr int kNoTuningLevel = -1;

bool ValidTuningGroup(int group)
{
    return group >= 0 && group < static_cast<int>(Render::Look::TuningGroupCount);
}

bool ValidTuningControl(int control)
{
    return control >= 0 && control < static_cast<int>(Render::Look::TuningControlCount);
}

// What a tuning slider shows: what applies. A developer's session strength of the light mood goes
// over the player's value, which goes over the look profile's and the map's.
double CurrentTuningValue(const Render::Look::TuningControl& control)
{
    if (!control.lookValue)
        if (const std::optional<int> session = Render::Look::SessionOverrides().lightMoodStrength)
            return static_cast<double>(*session);
    if (const std::optional<double> value = Render::Look::PersonalTuningSession().Override(control.id))
        return *value;
    if (control.lookValue)
        return Render::Look::Active()[*control.lookValue];
    return Render::Lighting::LightMood::MaxStrength;
}

UI::Options::TuningLock TuningLockOf(int group)
{
    using Render::Look::EnhancedFeature;
    using Render::Look::EnhancedFeatureChosen;
    const auto feature = static_cast<EnhancedFeature>(group);
    const Render::Look::FeatureOverrides session = Render::Look::SessionOverrides();
    UI::Options::TuningFeatureState state;
    state.chosen = EnhancedFeatureChosen(feature);
    switch (feature)
    {
    case EnhancedFeature::Lighting:
        state.active = Render::Lighting::EnhancedLightingActive();
        state.sessionOff = session.enhancedLighting == false;
        break;
    case EnhancedFeature::DynamicLights:
        state.active = Render::Lighting::DynamicLightsActive();
        state.sessionOff = session.dynamicLights == false || session.enhancedLighting == false;
        break;
    case EnhancedFeature::LightMood:
        state.sessionOff = session.lightMood == false;
        state.sessionSet = session.lightMoodStrength.has_value();
        break;
    case EnhancedFeature::PostProcessing:
        state.active = Render::PostProcessing::PostProcessingActive();
        state.sessionOff = session.postProcessing == false;
        break;
    }
    return UI::Options::TuningGroupLock(static_cast<Render::Look::TuningGroup>(group),
                                        Render::Look::ActiveLookPreset() == LookPreset::Enhanced,
                                        EnhancedFeatureChosen(EnhancedFeature::Lighting), state);
}

Rml::String TuningStateClass(UI::Options::TuningRowState state)
{
    switch (state)
    {
    case UI::Options::TuningRowState::Custom:
        return "custom";
    case UI::Options::TuningRowState::Unsaved:
        return "unsaved";
    case UI::Options::TuningRowState::Inherited:
        break;
    }
    return "inherited";
}
} // namespace

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

mu::ui::window::COptionWindow::COptionWindow()
{
    m_pNewUIMng = NULL;

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
    m_bTranslateItemNames = GameConfig::GetInstance().GetTranslateItemNames();
    m_iFontIndex = FindCurrentFontIndex();

    m_iActiveTab = 0;

    m_bVsyncEnabled = GameConfig::GetInstance().GetVSyncEnabled();
    m_iFpsCapIndex = FindCurrentFpsCapIndex();

    m_bDisableEffects = GameConfig::GetInstance().GetDisableEffects();
    m_bDisableParticles = GameConfig::GetInstance().GetDisableParticles();
    m_bDisableSkillEffectModels = GameConfig::GetInstance().GetDisableSkillEffectModels();
    m_bDisableBoids = GameConfig::GetInstance().GetDisableBoids();
    m_bDisableWingShadow = GameConfig::GetInstance().GetDisableWingShadow();
    m_bDisableBodyShadows = GameConfig::GetInstance().GetDisableBodyShadows();

    m_iThemeIndex = -1;
    m_iTexturePackIndex = 0;
    m_iUIScaleIndex = FindCurrentUIScaleIndex();
}

mu::ui::window::COptionWindow::~COptionWindow()
{
    Release();
}

bool mu::ui::window::COptionWindow::Create(CManager* pNewUIMng)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_OPTION, this);
    InitResolutionCombo();
    InitLanguageCombo();
    InitFontCombo();
    RefreshTexturePackChoices();

    BuildRmlUi();

    Show(false);
    return true;
}

void mu::ui::window::COptionWindow::BindRmlModel(Rml::DataModelConstructor& c, OptionRmlModel& model)
{
    c.Bind("positioned", &model.positioned);
    c.Bind("root_x", &model.rootX);
    c.Bind("root_y", &model.rootY);

    c.Bind("has_title", &model.hasTitle);
    c.Bind("title", &model.title);
    c.Bind("close_label", &model.closeLabel);

    c.Bind("active_tab", &model.activeTab);
    c.Bind("tab_gameplay_label", &model.tabGameplayLabel);
    c.Bind("tab_audio_label", &model.tabAudioLabel);
    c.Bind("tab_display_label", &model.tabDisplayLabel);
    c.Bind("tab_graphics_label", &model.tabGraphicsLabel);
    c.Bind("tab_interface_label", &model.tabInterfaceLabel);
    c.Bind("section_quality_label", &model.sectionQualityLabel);
    c.Bind("section_effects_label", &model.sectionEffectsLabel);
    c.Bind("section_shadows_label", &model.sectionShadowsLabel);

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
    c.Bind("sound_volume_label", &model.soundVolumeLabel);
    c.Bind("music_volume", &model.musicVolume);
    c.Bind("music_volume_label", &model.musicVolumeLabel);
    c.Bind("render_level", &model.renderLevel);
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
    c.Bind("translate_item_names", &model.translateItemNames);
    c.Bind("translate_item_names_label", &model.translateItemNamesLabel);
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
    c.Bind("disable_body_shadows", &model.disableBodyShadows);
    c.Bind("disable_body_shadows_label", &model.disableBodyShadowsLabel);
    c.Bind("skill_effect_models_locked", &model.skillEffectModelsLocked);
    c.Bind("boids_locked", &model.boidsLocked);
    c.Bind("body_shadows_locked", &model.bodyShadowsLocked);
    c.Bind("wing_shadow_locked", &model.wingShadowLocked);
    c.Bind("enhanced_graphics", &model.enhancedGraphics);
    c.Bind("look_lighting", &model.lookLighting);
    c.Bind("look_dynamic_lights", &model.lookDynamicLights);
    c.Bind("look_light_mood", &model.lookLightMood);
    c.Bind("look_post_processing", &model.lookPostProcessing);
    c.Bind("look_features_locked", &model.lookFeaturesLocked);
    c.Bind("look_dynamic_lights_locked", &model.lookDynamicLightsLocked);
    c.Bind("look_lighting_label", &model.lookLightingLabel);
    c.Bind("look_dynamic_lights_label", &model.lookDynamicLightsLabel);
    c.Bind("look_light_mood_label", &model.lookLightMoodLabel);
    c.Bind("look_post_processing_label", &model.lookPostProcessingLabel);
    c.Bind("enhanced_graphics_label", &model.enhancedGraphicsLabel);
    auto tuningRow = c.RegisterStruct<TuningRowModel>();
    tuningRow.RegisterMember("control", &TuningRowModel::control);
    tuningRow.RegisterMember("label", &TuningRowModel::label);
    tuningRow.RegisterMember("help", &TuningRowModel::help);
    tuningRow.RegisterMember("value_text", &TuningRowModel::valueText);
    tuningRow.RegisterMember("min_text", &TuningRowModel::minText);
    tuningRow.RegisterMember("max_text", &TuningRowModel::maxText);
    tuningRow.RegisterMember("state_text", &TuningRowModel::stateText);
    tuningRow.RegisterMember("state", &TuningRowModel::state);
    tuningRow.RegisterMember("fill_percent", &TuningRowModel::fillPercent);
    c.RegisterArray<std::vector<TuningRowModel>>();
    for (std::size_t group = 0; group < TuningGroupCount; ++group)
    {
        const std::string suffix = "_" + std::to_string(group);
        c.Bind("tuning_rows" + suffix, &model.tuningRows[group]);
        c.Bind("tuning_more_rows" + suffix, &model.tuningMoreRows[group]);
        c.Bind("tuning_expanded" + suffix, &model.tuningExpanded[group]);
        c.Bind("tuning_more_expanded" + suffix, &model.tuningMoreExpanded[group]);
        c.Bind("tuning_locked" + suffix, &model.tuningLocked[group]);
        c.Bind("tuning_reason" + suffix, &model.tuningReason[group]);
    }
    c.Bind("tuning_more_label", &model.tuningMoreLabel);
    c.Bind("tuning_reset_label", &model.tuningResetLabel);
    c.Bind("tuning_reset_group_label", &model.tuningResetGroupLabel);
    c.Bind("tuning_reset_all_label", &model.tuningResetAllLabel);
    c.Bind("tuning_save_message", &model.tuningSaveMessage);
    c.Bind("antialiasing_labels", &model.antialiasingLabels);
    c.Bind("antialiasing_index", &model.antialiasingIndex);
    c.Bind("antialiasing_row_label", &model.antialiasingRowLabel);
    c.Bind("antialiasing_value_label", &model.antialiasingValueLabel);
    c.Bind("antialiasing_tooltip", &model.antialiasingTooltip);
    c.Bind("anisotropy_labels", &model.anisotropyLabels);
    c.Bind("anisotropy_index", &model.anisotropyIndex);
    c.Bind("anisotropy_row_label", &model.anisotropyRowLabel);
    c.Bind("anisotropy_value_label", &model.anisotropyValueLabel);
    c.Bind("anisotropy_tooltip", &model.anisotropyTooltip);

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
    c.Bind("texture_pack_labels", &model.texturePackLabels);
    c.Bind("texture_pack_index", &model.texturePackIndex);
    c.Bind("texture_pack_row_label", &model.texturePackRowLabel);
    c.Bind("texture_pack_value_label", &model.texturePackValueLabel);
    c.Bind("texture_pack_tooltip", &model.texturePackTooltip);

    c.BindEventCallback("option_click_close",
        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickClose(); });

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
    c.BindEventCallback("option_toggle_translate_item_names",
                        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        { RmlToggleTranslateItemNames(); });

    c.BindEventCallback("option_gauge",
        [this](Rml::DataModelHandle, Rml::Event& ev, const Rml::VariantList& args)
        {
            if (args.size() == 1)
                RmlGaugeEvent(ev, args[0].Get<int>(-1));
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
    c.BindEventCallback("option_toggle_disable_body_shadows",
        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlToggleDisableBodyShadows(); });
    c.BindEventCallback("option_toggle_enhanced_graphics",
                        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        { RmlToggleEnhancedGraphics(); });
    c.BindEventCallback("option_toggle_enhanced_feature",
        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
        {
            if (arguments.size() == 1)
                RmlToggleEnhancedFeature(arguments[0].Get<int>(-1));
        });

    const auto intArgument = [](const Rml::VariantList& arguments)
    { return arguments.size() == 1 ? arguments[0].Get<int>(-1) : -1; };
    c.BindEventCallback("option_toggle_tuning_group",
                        [this, intArgument](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                        { RmlToggleTuningGroup(intArgument(arguments)); });
    c.BindEventCallback("option_toggle_tuning_more",
                        [this, intArgument](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                        { RmlToggleTuningMore(intArgument(arguments)); });
    c.BindEventCallback("option_tuning_pointer",
                        [this, intArgument](Rml::DataModelHandle, Rml::Event& event, const Rml::VariantList& arguments)
                        { RmlTuningPointer(event, intArgument(arguments)); });
    c.BindEventCallback("option_tuning_key",
                        [this, intArgument](Rml::DataModelHandle, Rml::Event& event, const Rml::VariantList& arguments)
                        { RmlTuningKey(event, intArgument(arguments)); });
    c.BindEventCallback("option_tuning_commit",
                        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlTuningCommit(); });
    c.BindEventCallback("option_reset_tuning",
                        [this, intArgument](Rml::DataModelHandle, Rml::Event& event, const Rml::VariantList& arguments)
                        {
                            event.StopPropagation();
                            RmlResetTuning(intArgument(arguments));
                        });
    c.BindEventCallback("option_reset_tuning_group",
                        [this, intArgument](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                        { RmlResetTuningGroup(intArgument(arguments)); });
    c.BindEventCallback("option_reset_all_tuning",
                        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlResetAllTuning(); });

    c.BindEventCallback("option_toggle_show_fps_counter",
        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlToggleShowFpsCounter(); });
    c.BindEventCallback("option_toggle_show_debug_info",
        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlToggleShowDebugInfo(); });

}

// Dragged by any part that is not a control (base.rcss blocks those; legacy shows no title to
// grab) for the session; the position is not saved. A theme whose panel fills the screen marks it
// `fullscreen`, and there is nothing to drag.
void mu::ui::window::COptionWindow::OnRmlBuilt()
{
    RefreshThemeChoices();
    Rml::ElementDocument* document = m_RmlView.Document();
    m_pPanelEl = document->GetElementById("panel");
    if (m_pPanelEl && m_pPanelEl->IsClassSet("fullscreen"))
        return;
    UI::RmlBridge::MakeDraggable(document, document, nullptr,
                                 [document] { UI::RmlBridge::KeepInsideWindow(document); });
}

void mu::ui::window::COptionWindow::BuildRmlUi()
{
    m_RmlView.Ensure();
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
    m_RmlView.Release();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void mu::ui::window::COptionWindow::Show(bool bShow)
{
    mu::ui::window::CObject::Show(bShow);
    if (m_RmlView.Document())
    {
        // A hidden window skips its sync, so it catches up before its document shows.
        if (bShow)
        {
            SyncRmlModel();
            m_RmlView.Document()->Show();
        }
        else
            m_RmlView.Document()->Hide();
    }
    // Closing the window ends a tuning interaction too (a slider still held or focused).
    if (!bShow)
        FinishTuningInteraction();
}

bool mu::ui::window::COptionWindow::UpdateMouseEvent()
{
    // RmlUi's #panel owns the hit-testing of its controls; over the panel (the dragged document in
    // legacy, a full-screen layer in modern), a click never falls through to the world or the scene
    // behind it. The window isn't modal, so the rest of the screen stays the world's.
    if (!m_pPanelEl && m_RmlView.Document())
        m_pPanelEl = m_RmlView.Document()->GetElementById("panel");

    return !UI::RmlBridge::IsPointerOver(m_pPanelEl);
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
    // Outside any RmlUi event dispatch -- safe to destroy/rebuild m_RmlView.Document() here if a theme or
    // font switch was requested (see m_bPendingThemeSwitch's own comment), and to resize the window for a new
    // UI scale (m_bPendingUIScaleApply's own comment).
    ApplyPendingThemeSwitch();
    ApplyPendingFont();
    ApplyPendingUIScale();
    SyncRmlModel();
    ScrollOpenDropdownToSelection();
    return true;
}

void mu::ui::window::COptionWindow::ScrollOpenDropdownToSelection()
{
    if (!m_bScrollDropdownPending || !m_RmlView.Document())
        return;
    if (m_iOpenDropdown == -1)
    {
        m_bScrollDropdownPending = false;
        return;
    }

    // The click that opened it runs before the data model shows the list: wait for a frame in
    // which the list is no longer hidden.
    m_RmlView.Document()->UpdateDocument();
    Rml::ElementList lists;
    m_RmlView.Document()->GetElementsByClassName(lists, "option-dropdown-list");
    for (Rml::Element* list : lists)
    {
        if (list->IsClassSet("hidden"))
            continue;
        m_bScrollDropdownPending = false;
        Rml::ElementList selected;
        list->GetElementsByClassName(selected, "selected");
        if (!selected.empty())
            selected.front()->ScrollIntoView(
                Rml::ScrollIntoViewOptions(Rml::ScrollAlignment::Center, Rml::ScrollAlignment::Nearest,
                                           Rml::ScrollBehavior::Instant, Rml::ScrollParentage::Closest));
    }
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
    m_bTranslateItemNames = GameConfig::GetInstance().GetTranslateItemNames();
    m_iFontIndex = FindCurrentFontIndex();
    m_bWindowedMode = (g_bUseWindowMode == TRUE);

    m_bVsyncEnabled = GameConfig::GetInstance().GetVSyncEnabled();
    m_iFpsCapIndex = FindCurrentFpsCapIndex();

    m_bDisableEffects = GameConfig::GetInstance().GetDisableEffects();
    m_bDisableParticles = GameConfig::GetInstance().GetDisableParticles();
    m_bDisableSkillEffectModels = GameConfig::GetInstance().GetDisableSkillEffectModels();
    m_bDisableBoids = GameConfig::GetInstance().GetDisableBoids();
    m_bDisableWingShadow = GameConfig::GetInstance().GetDisableWingShadow();
    m_bDisableBodyShadows = GameConfig::GetInstance().GetDisableBodyShadows();

    RefreshThemeChoices();
    m_iUIScaleIndex = FindCurrentUIScaleIndex();
    RefreshTexturePackChoices();
}

void mu::ui::window::COptionWindow::ClosingProcess()
{
}

void mu::ui::window::COptionWindow::RmlClickSelectTab(int nTab)
{
    if (nTab < 0 || nTab > 4)
        return;
    m_iActiveTab = nTab;
    PlayBuffer(SOUND_CLICK01);
}

// A row another setting covers shows ticked and locked (OptionRmlModel's comment).
void mu::ui::window::COptionWindow::SyncImpliedEffectRows(OptionRmlModel& model)
{
    const auto syncFlag = [this](bool& field, const char* name, bool value)
    {
        if (field != value)
        {
            field = value;
            m_RmlView.MarkDirty(name);
        }
    };
    const bool noBodyShadows = !m_bRenderAllEffects;
    const bool noWingShadow = noBodyShadows || m_bDisableBodyShadows;
    syncFlag(model.skillEffectModelsLocked, "skill_effect_models_locked", m_bDisableEffects);
    syncFlag(model.boidsLocked, "boids_locked", m_bDisableEffects);
    syncFlag(model.bodyShadowsLocked, "body_shadows_locked", noBodyShadows);
    syncFlag(model.wingShadowLocked, "wing_shadow_locked", noWingShadow);
    syncFlag(model.disableSkillEffectModels, "disable_skill_effect_models", m_bDisableSkillEffectModels || m_bDisableEffects);
    syncFlag(model.disableBoids, "disable_boids", m_bDisableBoids || m_bDisableEffects);
    syncFlag(model.disableBodyShadows, "disable_body_shadows", m_bDisableBodyShadows || noBodyShadows);
    syncFlag(model.disableWingShadow, "disable_wing_shadow", m_bDisableWingShadow || noWingShadow);
}

void mu::ui::window::COptionWindow::RmlToggleDropdown(int dropdownId)
{
    if (dropdownId < 0 || dropdownId >= DropdownCount)
        return;
    // Toggle: clicking the currently-open one's own value box closes it; clicking any other
    // (including a different dropdown's) overwrites m_iOpenDropdown, which closes whatever else
    // was open as a side effect -- see model.openDropdown's own comment for why a single field
    // gives exclusivity for free.
    m_iOpenDropdown = (m_iOpenDropdown == dropdownId) ? -1 : dropdownId;
    m_bScrollDropdownPending = (m_iOpenDropdown != -1);
    PlayBuffer(SOUND_CLICK01);
}

void mu::ui::window::COptionWindow::RmlDropdownOptionClick(int dropdownId, int optionIndex)
{
    switch (dropdownId)
    {
    case DropdownResolution:
        RmlResolutionChanged(optionIndex);
        break;
    case DropdownFpsCap:
        RmlFpsCapChanged(optionIndex);
        break;
    case DropdownTheme:
        RmlThemeChanged(optionIndex);
        break;
    case DropdownLanguage:
        RmlLanguageChanged(optionIndex);
        break;
    case DropdownFont:
        RmlFontChanged(optionIndex);
        break;
    case DropdownUIScale:
        RmlUIScaleChanged(optionIndex);
        break;
    case DropdownAntialiasing:
        RmlAntialiasingChanged(optionIndex);
        break;
    case DropdownAnisotropy:
        RmlAnisotropyChanged(optionIndex);
        break;
    case DropdownTexturePack:
        RmlTexturePackChanged(optionIndex);
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
    // Five cells, levels 0..4; the game reads level 4 as everything on.
    m_iRenderLevel = std::clamp(value, 0, 4);
}

void mu::ui::window::COptionWindow::RmlGaugeEvent(Rml::Event& event, int gauge)
{
    // Native's volume bars rounded to the nearest of 0..10, and a press left of the bar read 0.
    const auto volumeFromPointer = [](float x, float width)
    { return x < 0.f ? 0 : static_cast<int>(10.f * x / width + 0.5f); };
    // The effect bar shows level + 1 cells, so the pointer's cell is the level.
    const auto renderFromPointer = [](float x, float width)
    { return x < 0.f ? 0 : static_cast<int>(5.f * x / width); };

    switch (gauge)
    {
    case 0:
        if (const auto next = UI::RmlBridge::ApplyLevelGaugeEvent(event, m_iVolumeLevel, 10, volumeFromPointer))
            RmlSoundVolumeChanged(*next);
        break;
    case 1:
        if (const auto next = UI::RmlBridge::ApplyLevelGaugeEvent(event, m_iMusicLevel, 10, volumeFromPointer))
            RmlMusicVolumeChanged(*next);
        break;
    case 2:
        if (const auto next = UI::RmlBridge::ApplyLevelGaugeEvent(event, m_iRenderLevel, 4, renderFromPointer))
            RmlRenderLevelChanged(*next);
        break;
    default:
        break;
    }
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

void mu::ui::window::COptionWindow::RmlToggleTranslateItemNames()
{
    m_bTranslateItemNames = !m_bTranslateItemNames;
    GameConfig::GetInstance().SetTranslateItemNames(m_bTranslateItemNames);
    GameConfig::GetInstance().Save();
    g_ItemDataHandler.SetTranslateItemNames(m_bTranslateItemNames);
}

void mu::ui::window::COptionWindow::RmlFontChanged(int index)
{
    // See RmlResolutionChanged's own comment -- same settle-frame guard applies here.
    if (m_rmlSyncCount < kRmlSelectSettleFrames)
        return;
    if (index < 0 || index >= s_NumFonts)
        return;
    m_iFontIndex = index;
    // Deferred to Update() -- the font change rebuilds every RmlUi document, see m_bPendingThemeSwitch.
    m_bPendingFontApply = true;
}

void mu::ui::window::COptionWindow::ApplyPendingFont()
{
    if (!m_bPendingFontApply)
        return;
    m_bPendingFontApply = false;
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
    if (m_bDisableEffects)
        return; // Disable Effects already skips them
    m_bDisableSkillEffectModels = !m_bDisableSkillEffectModels;
    SetDisableSkillEffectModels(m_bDisableSkillEffectModels);
    GameConfig::GetInstance().SetDisableSkillEffectModels(m_bDisableSkillEffectModels);
    GameConfig::GetInstance().Save();
}

void mu::ui::window::COptionWindow::RmlToggleDisableBoids()
{
    if (m_bDisableEffects)
        return; // Disable Effects already skips them
    m_bDisableBoids = !m_bDisableBoids;
    SetDisableBoids(m_bDisableBoids);
    GameConfig::GetInstance().SetDisableBoids(m_bDisableBoids);
    GameConfig::GetInstance().Save();
}

void mu::ui::window::COptionWindow::RmlToggleDisableWingShadow()
{
    if (!m_bRenderAllEffects || m_bDisableBodyShadows)
        return; // no body shadow is drawn, the wing's included
    m_bDisableWingShadow = !m_bDisableWingShadow;
    SetDisableWingShadow(m_bDisableWingShadow);
    GameConfig::GetInstance().SetDisableWingShadow(m_bDisableWingShadow);
    GameConfig::GetInstance().Save();
}

void mu::ui::window::COptionWindow::RmlToggleDisableBodyShadows()
{
    if (!m_bRenderAllEffects)
        return; // without full effects no body shadow is drawn
    m_bDisableBodyShadows = !m_bDisableBodyShadows;
    SetDisableBodyShadows(m_bDisableBodyShadows);
    GameConfig::GetInstance().SetDisableBodyShadows(m_bDisableBodyShadows);
    GameConfig::GetInstance().Save();
}

// The look preset through the one function start-up and `$look classic|enhanced` use: it saves
// [Render] Look, switches the Enhanced features and clears developer overrides.
void mu::ui::window::COptionWindow::RmlToggleEnhancedGraphics()
{
    const bool enhanced = Render::Look::ActiveLookPreset() == LookPreset::Enhanced;
    Render::Look::ApplyLookPreset(enhanced ? LookPreset::Classic : LookPreset::Enhanced);
}

void mu::ui::window::COptionWindow::RmlToggleEnhancedFeature(int feature)
{
    using Render::Look::EnhancedFeature;
    if (feature < 0 || feature > static_cast<int>(EnhancedFeature::PostProcessing))
        return;
    if (Render::Look::ActiveLookPreset() != LookPreset::Enhanced)
        return;
    const auto chosen = static_cast<EnhancedFeature>(feature);
    if (chosen == EnhancedFeature::DynamicLights && !Render::Look::EnhancedFeatureChosen(EnhancedFeature::Lighting))
        return; // dynamic lights act only with Enhanced lighting
    Render::Look::ChooseEnhancedFeature(chosen, !Render::Look::EnhancedFeatureChosen(chosen));
}

void mu::ui::window::COptionWindow::RmlToggleTuningGroup(int group)
{
    if (!ValidTuningGroup(group))
        return;
    m_tuningExpanded[group] = !m_tuningExpanded[group];
    PlayBuffer(SOUND_CLICK01);
}

void mu::ui::window::COptionWindow::RmlToggleTuningMore(int group)
{
    if (!ValidTuningGroup(group))
        return;
    m_tuningMoreExpanded[group] = !m_tuningMoreExpanded[group];
    PlayBuffer(SOUND_CLICK01);
}

bool mu::ui::window::COptionWindow::TuningGroupUnlocked(int group) const
{
    return ValidTuningGroup(group) && TuningLockOf(group) == UI::Options::TuningLock::None;
}

void mu::ui::window::COptionWindow::RmlTuningPointer(Rml::Event& event, int control)
{
    if (!ValidTuningControl(control))
        return;
    const Render::Look::TuningControl& tuning =
        Render::Look::Control(static_cast<Render::Look::TuningControlId>(control));
    if (!TuningGroupUnlocked(static_cast<int>(tuning.group)))
        return;
    // The track as a level gauge with one level per step. The gauge is told no level is current
    // (kNoTuningLevel), so it reports the pointer's level every time; whether that changes the value
    // is decided against the exact value, which an inherited one between two steps may not be on.
    const int steps = Render::Look::TuningStepCount(tuning);
    const auto fromPointer = [steps](float x, float width)
    { return x < 0.f ? 0 : static_cast<int>(std::lround(static_cast<float>(steps) * x / width)); };
    const std::optional<int> level = UI::RmlBridge::ApplyLevelGaugeEvent(event, kNoTuningLevel, steps, fromPointer);
    if (!level)
        return;
    const std::optional<double> next = Render::Look::TuningPointerValue(tuning, CurrentTuningValue(tuning), *level);
    if (next && Render::Look::PersonalTuningSession().Preview(tuning.id, *next))
        m_tuningPreviewPending = true;
}

void mu::ui::window::COptionWindow::RmlTuningKey(Rml::Event& event, int control)
{
    if (!ValidTuningControl(control))
        return;
    const Render::Look::TuningControl& tuning =
        Render::Look::Control(static_cast<Render::Look::TuningControlId>(control));
    // The gauge decodes the key and keeps it; the value steps from the exact value, which an
    // inherited one between two steps may not be on.
    const std::optional<UI::RmlBridge::LevelGaugeKeyMove> move = UI::RmlBridge::LevelGaugeKey(event);
    if (!move || !TuningGroupUnlocked(static_cast<int>(tuning.group)))
        return;
    using To = UI::RmlBridge::LevelGaugeKeyMove::To;
    const double next = move->to == To::Min ? tuning.min
                        : move->to == To::Max
                            ? tuning.max
                            : Render::Look::StepTuningValue(tuning, CurrentTuningValue(tuning), move->steps);
    if (Render::Look::PersonalTuningSession().Preview(tuning.id, next))
        m_tuningPreviewPending = true;
}

void mu::ui::window::COptionWindow::RmlTuningCommit()
{
    FinishTuningInteraction();
}

void mu::ui::window::COptionWindow::FinishTuningInteraction()
{
    // Saves only; a pending Mood strength is applied by the frame loop (ApplyPendingTuning()).
    Render::Look::TuningSession& session = Render::Look::PersonalTuningSession();
    // A failed save is tried again when the window closes, never per frame.
    if (m_tuningPreviewPending || (!IsVisible() && session.SaveState() != Render::Look::TuningSaveState::Saved))
    {
        m_tuningPreviewPending = false;
        (void)session.Commit();
    }
}

void mu::ui::window::COptionWindow::RmlResetTuning(int control)
{
    if (!ValidTuningControl(control))
        return;
    Render::Look::PersonalTuningSession().Reset(static_cast<Render::Look::TuningControlId>(control));
    PlayBuffer(SOUND_CLICK01);
}

void mu::ui::window::COptionWindow::RmlResetTuningGroup(int group)
{
    if (!ValidTuningGroup(group))
        return;
    Render::Look::PersonalTuningSession().ResetGroup(static_cast<Render::Look::TuningGroup>(group));
    PlayBuffer(SOUND_CLICK01);
}

void mu::ui::window::COptionWindow::RmlResetAllTuning()
{
    Render::Look::PersonalTuningSession().ResetAll();
    PlayBuffer(SOUND_CLICK01);
}

void mu::ui::window::COptionWindow::SyncTuningModel(OptionRmlModel& model)
{
    const auto syncText = [this](Rml::String& field, const std::string& name, const std::string& value)
    {
        if (field != value)
        {
            field = value;
            m_RmlView.MarkDirty(name);
        }
    };
    const auto syncFlag = [this](bool& field, const std::string& name, bool value)
    {
        if (field != value)
        {
            field = value;
            m_RmlView.MarkDirty(name);
        }
    };
    syncText(model.tuningMoreLabel, "tuning_more_label", UI::Options::TuningMoreSettingsLabel());
    syncText(model.tuningResetLabel, "tuning_reset_label", UI::Options::TuningResetLabel());
    syncText(model.tuningResetGroupLabel, "tuning_reset_group_label", UI::Options::TuningResetGroupLabel());
    syncText(model.tuningResetAllLabel, "tuning_reset_all_label", UI::Options::TuningResetAllLabel());

    const Render::Look::TuningSession& session = Render::Look::PersonalTuningSession();
    syncText(model.tuningSaveMessage, "tuning_save_message", UI::Options::TuningSaveMessage(session));

    std::array<std::vector<TuningRowModel>, TuningGroupCount> rows;
    std::array<std::vector<TuningRowModel>, TuningGroupCount> moreRows;
    for (const Render::Look::TuningControl& control : Render::Look::TuningControls())
    {
        const UI::Options::TuningRow row = UI::Options::BuildTuningRow(control, CurrentTuningValue(control),
                                                                       UI::Options::TuningStateOf(session, control.id));
        const TuningRowModel bound{static_cast<int>(control.id),
                                   row.label,
                                   row.help,
                                   row.valueText,
                                   row.minText,
                                   row.maxText,
                                   row.stateText,
                                   TuningStateClass(row.state),
                                   row.fillPercent};
        (control.specialist ? moreRows : rows)[static_cast<std::size_t>(control.group)].push_back(bound);
    }
    for (std::size_t group = 0; group < TuningGroupCount; ++group)
    {
        const std::string suffix = "_" + std::to_string(group);
        if (model.tuningRows[group] != rows[group])
        {
            model.tuningRows[group] = std::move(rows[group]);
            m_RmlView.MarkDirty("tuning_rows" + suffix);
        }
        if (model.tuningMoreRows[group] != moreRows[group])
        {
            model.tuningMoreRows[group] = std::move(moreRows[group]);
            m_RmlView.MarkDirty("tuning_more_rows" + suffix);
        }
        syncFlag(model.tuningExpanded[group], "tuning_expanded" + suffix, m_tuningExpanded[group]);
        syncFlag(model.tuningMoreExpanded[group], "tuning_more_expanded" + suffix, m_tuningMoreExpanded[group]);
        const UI::Options::TuningLock lock = TuningLockOf(static_cast<int>(group));
        syncFlag(model.tuningLocked[group], "tuning_locked" + suffix, lock != UI::Options::TuningLock::None);
        syncText(model.tuningReason[group], "tuning_reason" + suffix, UI::Options::TuningLockReason(lock));
    }
}

// MuSetAntialiasingPreference saves the count and applies it while the Enhanced look is active.
void mu::ui::window::COptionWindow::RmlAntialiasingChanged(int index)
{
    // See RmlResolutionChanged's own comment -- same settle-frame guard applies here.
    if (m_rmlSyncCount < kRmlSelectSettleFrames)
        return;
    if (index < 0 || index >= static_cast<int>(std::size(s_AntialiasingValues)))
        return;
    MuSetAntialiasingPreference(s_AntialiasingValues[index]);
}

void mu::ui::window::COptionWindow::RmlAnisotropyChanged(int index)
{
    // See RmlResolutionChanged's own comment -- same settle-frame guard applies here.
    if (m_rmlSyncCount < kRmlSelectSettleFrames)
        return;
    if (index < 0 || index >= static_cast<int>(std::size(s_AnisotropyValues)))
        return;
    MuSetAnisotropyPreference(s_AnisotropyValues[index]);
}

void mu::ui::window::COptionWindow::RmlTexturePackChanged(int index)
{
    // See RmlResolutionChanged's own comment -- same settle-frame guard applies here.
    if (m_rmlSyncCount < kRmlSelectSettleFrames)
        return;
    if (index < 0 || index >= static_cast<int>(m_texturePackNames.size()))
        return;
    m_iTexturePackIndex = index;
    // Compared with the saved name, not the shown index: a saved pack that is no longer installed
    // shows as Original, and choosing Original has to clear it.
    const std::wstring& chosen = m_texturePackNames[index];
    if (chosen == GameConfig::GetInstance().GetTexturePack())
        return;
    GameConfig::GetInstance().SetTexturePack(chosen);
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
    if (index < 0 || index >= static_cast<int>(m_themeNames.size()))
        return;
    if (m_themeNames[static_cast<size_t>(index)] == UI::RmlBridge::GetActiveThemeName())
        return;

    // Deferred to Update() -- see m_bPendingThemeSwitch's own comment (OptionWindow.h) for why
    // this can't run synchronously here: it would destroy m_RmlView.Document() mid-dispatch of the very
    // dropdown-option click event that called this (RmlDropdownOptionClick(), itself invoked from
    // a data-event-click on one of the option divs this document is about to unload).
    m_pendingThemeName = m_themeNames[static_cast<size_t>(index)];
    m_bPendingThemeSwitch = true;
}

void mu::ui::window::COptionWindow::ApplyPendingThemeSwitch()
{
    if (!m_bPendingThemeSwitch)
        return;
    m_bPendingThemeSwitch = false;

    const std::string themeName = std::exchange(m_pendingThemeName, {});
    UI::Theme::Select(themeName, UI::Theme::Persistence::Saved);
    RefreshThemeChoices();
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
    // RmlUi text draws with the same family (font tokens), so its documents are rebuilt with it.
    UI::RmlBridge::ApplySelectedFontFamily(StringUtils::WideToNarrow(s_Fonts[m_iFontIndex].name));
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

void mu::ui::window::COptionWindow::RefreshTexturePackChoices()
{
    m_texturePackNames = Render::Textures::TexturePack::ListInstalledPacks();
    m_texturePackNames.insert(m_texturePackNames.begin(), std::wstring());
    m_iTexturePackIndex = FindCurrentTexturePackIndex();

    // Pack names are folder names, so they only change here. The first entry is the localized
    // "Original", which SyncRmlModel() keeps following a live language switch.
    auto& model = m_RmlView.GetModel();
    model.texturePackLabels.clear();
    model.texturePackLabels.reserve(m_texturePackNames.size());
    model.texturePackLabels.push_back(StringUtils::WideToNarrow(I18N::Game::OriginalTextures));
    for (size_t i = 1; i < m_texturePackNames.size(); ++i)
        model.texturePackLabels.push_back(StringUtils::WideToNarrow(m_texturePackNames[i].c_str()));
    m_RmlView.MarkDirty("texture_pack_labels");
}

// Folder names compare in any case, as the loader opens them. A saved pack that is no longer
// installed shows as the original textures, which is what the loader falls back to.
int mu::ui::window::COptionWindow::FindCurrentTexturePackIndex()
{
    const std::wstring current = GameConfig::GetInstance().GetTexturePack();
    if (current.empty())
        return 0;
    const auto found =
        std::find_if(m_texturePackNames.begin() + 1, m_texturePackNames.end(), [&current](const std::wstring& name)
                     { return Render::Textures::TexturePack::SameName(name, current); });
    if (found == m_texturePackNames.end())
        return 0;
    return static_cast<int>(found - m_texturePackNames.begin());
}

int mu::ui::window::COptionWindow::FindCurrentThemeIndex()
{
    const std::string& active = UI::RmlBridge::GetActiveThemeName();
    for (size_t i = 0; i < m_themeNames.size(); ++i)
    {
        if (m_themeNames[i] == active)
            return static_cast<int>(i);
    }
    return -1;
}

void mu::ui::window::COptionWindow::RefreshThemeChoices()
{
    m_themeNames = UI::RmlBridge::DiscoverAvailableThemes();
    m_iThemeIndex = FindCurrentThemeIndex();
    auto& model = m_RmlView.GetModel();
    model.themeLabels.clear();
    model.themeLabels.reserve(m_themeNames.size());
    for (const std::string& name : m_themeNames)
        model.themeLabels.push_back(UI::RmlBridge::GetThemeDisplayName(name));
    m_RmlView.MarkDirty("theme_labels");
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

void mu::ui::window::COptionWindow::SyncLabel(Rml::String& field, const char* fieldName, const wchar_t* text)
{
    const Rml::String narrow = StringUtils::WideToNarrow(text);
    if (field != narrow)
    {
        field = narrow;
        m_RmlView.MarkDirty(fieldName);
    }
}

void mu::ui::window::COptionWindow::SyncRmlModel()
{
    if (!m_RmlView.Document())
        return;

    // Saturates rather than wrapping; only ever compared against kRmlSelectSettleFrames below.
    if (m_rmlSyncCount < 1000)
        ++m_rmlSyncCount;
    if (!IsVisible())
        return;

    auto& model = m_RmlView.GetModel();

    // Diffed first, matching MyQuestInfoWindow::SyncRmlModel()'s own ordering for its activeTab.
    if (model.activeTab != m_iActiveTab) { model.activeTab = m_iActiveTab; m_RmlView.MarkDirty("active_tab"); }
    if (model.openDropdown != m_iOpenDropdown) { model.openDropdown = m_iOpenDropdown; m_RmlView.MarkDirty("open_dropdown"); }

    // Re-fetched every sync, not just once at BuildRmlUi() time -- native re-rendered every one of
    // these from the live I18N::Game::* pointer every frame, so a language switch made from this
    // very window relabels it immediately rather than only on next reopen.
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
            m_RmlView.MarkDirty(fieldName);
        }
    };
    SyncLabel(model.title, "title", I18N::Game::Option385);
    SyncLabel(model.closeLabel, "close_label", I18N::Game::Close);
    SyncLabel(model.tabGameplayLabel, "tab_gameplay_label", I18N::Game::Gameplay);
    SyncLabel(model.tabAudioLabel, "tab_audio_label", I18N::Game::Audio);
    SyncLabel(model.tabDisplayLabel, "tab_display_label", I18N::Game::Display);
    SyncLabel(model.tabGraphicsLabel, "tab_graphics_label", I18N::Game::Graphics);
    SyncLabel(model.tabInterfaceLabel, "tab_interface_label", I18N::Game::Interface);
    SyncLabel(model.sectionQualityLabel, "section_quality_label", I18N::Game::Quality);
    SyncLabel(model.sectionEffectsLabel, "section_effects_label", I18N::Game::Effects);
    SyncLabel(model.sectionShadowsLabel, "section_shadows_label", I18N::Game::Shadows);
    SyncLabel(model.autoAttackLabel, "auto_attack_label", I18N::Game::AutomaticAttack);
    SyncLabel(model.whisperSoundLabel, "whisper_sound_label", I18N::Game::BeepSoundForWhispering);
    SyncLabel(model.slideHelpLabel, "slide_help_label", I18N::Game::SlideHelp);
    SyncLabel(model.renderAllEffectsLabel, "render_all_effects_label", I18N::Game::RenderFullEffects);
    SyncLabel(model.windowedModeLabel, "windowed_mode_label", I18N::Game::WindowedMode);
    SyncLabel(model.soundVolumeLabel, "sound_volume_label", I18N::Game::SoundVolume);
    SyncLabel(model.musicVolumeLabel, "music_volume_label", I18N::Game::MusicVolume);
    SyncLabel(model.renderLevelLabel, "render_level_label", I18N::Game::EffectLimitation);
    SyncLabel(model.resolutionRowLabel, "resolution_row_label", I18N::Game::Resolution);
    SyncLabel(model.languageRowLabel, "language_row_label", I18N::Game::Language);
    SyncLabel(model.translateItemNamesLabel, "translate_item_names_label", I18N::Game::TranslateItemNames);
    SyncLabel(model.fontRowLabel, "font_row_label", I18N::Game::Font);
    SyncLabel(model.vsyncLabel, "vsync_label", I18N::Game::VSync);
    SyncLabel(model.fpsCapRowLabel, "fps_cap_row_label", I18N::Game::FPSLimit);
    SyncLabel(model.disableEffectsLabel, "disable_effects_label", I18N::Game::DisableEffects);
    SyncLabel(model.disableParticlesLabel, "disable_particles_label", I18N::Game::DisableParticles);
    SyncLabel(model.disableSkillEffectModelsLabel, "disable_skill_effect_models_label",
              I18N::Game::DisableSkillEffectModels);
    SyncLabel(model.disableBoidsLabel, "disable_boids_label", I18N::Game::DisableAmbientWildlife);
    SyncLabel(model.disableWingShadowLabel, "disable_wing_shadow_label", I18N::Game::DisableWingShadow);
    SyncLabel(model.disableBodyShadowsLabel, "disable_body_shadows_label", I18N::Game::DisableShadows);
    SyncLabel(model.enhancedGraphicsLabel, "enhanced_graphics_label", I18N::Game::EnhancedGraphics);
    SyncLabel(model.lookLightingLabel, "look_lighting_label", I18N::Game::EnhancedLighting);
    SyncLabel(model.lookDynamicLightsLabel, "look_dynamic_lights_label", I18N::Game::DynamicLights);
    SyncLabel(model.lookLightMoodLabel, "look_light_mood_label", I18N::Game::LightMood);
    SyncLabel(model.lookPostProcessingLabel, "look_post_processing_label", I18N::Game::BloomAndColourGrading);
    SyncLabel(model.antialiasingRowLabel, "antialiasing_row_label", I18N::Game::AntiAliasing);
    SyncLabel(model.antialiasingTooltip, "antialiasing_tooltip", I18N::Game::AntiAliasingTooltip);
    SyncLabel(model.anisotropyRowLabel, "anisotropy_row_label", I18N::Game::AnisotropicFiltering);
    SyncLabel(model.anisotropyTooltip, "anisotropy_tooltip", I18N::Game::AnisotropicFilteringTooltip);
    SyncLabel(model.showFpsCounterLabel, "show_fps_counter_label", I18N::Game::ShowFPSCounter);
    SyncLabel(model.showDebugInfoLabel, "show_debug_info_label", I18N::Game::ShowDebugInfo);
    SyncLabel(model.themeRowLabel, "theme_row_label", I18N::Game::UITheme);
    SyncLabel(model.uiScaleRowLabel, "ui_scale_row_label", I18N::Game::UIScale);
    SyncLabel(model.uiScaleTooltip, "ui_scale_tooltip", I18N::Game::UIScaleTooltip);

    // positioned/root_x/root_y stay at their model defaults (false/0/0, see OptionRmlModel's own
    // comment): window_shell's `.center-both` CSS places the window, a drag moves it.

    if (model.autoAttack != m_bAutoAttack) { model.autoAttack = m_bAutoAttack; m_RmlView.MarkDirty("auto_attack"); }
    if (model.whisperSound != m_bWhisperSound) { model.whisperSound = m_bWhisperSound; m_RmlView.MarkDirty("whisper_sound"); }
    if (model.slideHelp != m_bSlideHelp) { model.slideHelp = m_bSlideHelp; m_RmlView.MarkDirty("slide_help"); }
    if (model.renderAllEffects != m_bRenderAllEffects) { model.renderAllEffects = m_bRenderAllEffects; m_RmlView.MarkDirty("render_all_effects"); }
    if (model.windowedMode != m_bWindowedMode) { model.windowedMode = m_bWindowedMode; m_RmlView.MarkDirty("windowed_mode"); }

    if (model.soundVolume != m_iVolumeLevel) { model.soundVolume = m_iVolumeLevel; m_RmlView.MarkDirty("sound_volume"); }
    if (model.musicVolume != m_iMusicLevel) { model.musicVolume = m_iMusicLevel; m_RmlView.MarkDirty("music_volume"); }
    if (model.renderLevel != m_iRenderLevel) { model.renderLevel = m_iRenderLevel; m_RmlView.MarkDirty("render_level"); }

    std::vector<Rml::String> newResolutionLabels;
    newResolutionLabels.reserve(m_resolutionLabels.size());
    for (const auto& label : m_resolutionLabels)
        newResolutionLabels.push_back(StringUtils::WideToNarrow(label.c_str()));
    if (newResolutionLabels != model.resolutionLabels)
    {
        model.resolutionLabels = std::move(newResolutionLabels);
        m_RmlView.MarkDirty("resolution_labels");
    }
    if (model.resolutionIndex != m_iResolutionIndex) { model.resolutionIndex = m_iResolutionIndex; m_RmlView.MarkDirty("resolution_index"); }
    syncDropdownValue(model.resolutionValueLabel, "resolution_value_label", model.resolutionLabels, model.resolutionIndex);

    if (model.languageLabels.empty())
    {
        model.languageLabels.reserve(s_NumLanguages);
        for (int i = 0; i < s_NumLanguages; ++i)
            model.languageLabels.push_back(StringUtils::WideToNarrow(s_Languages[i].label));
        m_RmlView.MarkDirty("language_labels");
    }
    if (model.languageIndex != m_iLanguageIndex) { model.languageIndex = m_iLanguageIndex; m_RmlView.MarkDirty("language_index"); }
    syncDropdownValue(model.languageValueLabel, "language_value_label", model.languageLabels, model.languageIndex);
    if (model.translateItemNames != m_bTranslateItemNames)
    {
        model.translateItemNames = m_bTranslateItemNames;
        m_RmlView.MarkDirty("translate_item_names");
    }

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
        m_RmlView.MarkDirty("font_labels");
    }
    if (model.fontIndex != m_iFontIndex) { model.fontIndex = m_iFontIndex; m_RmlView.MarkDirty("font_index"); }
    syncDropdownValue(model.fontValueLabel, "font_value_label", model.fontLabels, model.fontIndex);

    if (model.vsyncEnabled != m_bVsyncEnabled) { model.vsyncEnabled = m_bVsyncEnabled; m_RmlView.MarkDirty("vsync_enabled"); }

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
        m_RmlView.MarkDirty("fps_cap_labels");
    }
    if (model.fpsCapIndex != m_iFpsCapIndex) { model.fpsCapIndex = m_iFpsCapIndex; m_RmlView.MarkDirty("fps_cap_index"); }
    syncDropdownValue(model.fpsCapValueLabel, "fps_cap_value_label", model.fpsCapLabels, model.fpsCapIndex);

    if (model.disableEffects != m_bDisableEffects) { model.disableEffects = m_bDisableEffects; m_RmlView.MarkDirty("disable_effects"); }
    if (model.disableParticles != m_bDisableParticles) { model.disableParticles = m_bDisableParticles; m_RmlView.MarkDirty("disable_particles"); }
    SyncImpliedEffectRows(model);

    // Read live, not from a member -- SetShowFpsCounter()/SetShowDebugInfo() (SceneManager.h) are
    // mutually exclusive, so this reflects either checkbox toggling the other one off immediately,
    // including when toggled via the $fpscounter/$details console commands instead of this window.
    const bool enhancedGraphics = Render::Look::ActiveLookPreset() == LookPreset::Enhanced;
    if (model.enhancedGraphics != enhancedGraphics)
    {
        model.enhancedGraphics = enhancedGraphics;
        m_RmlView.MarkDirty("enhanced_graphics");
    }
    {
        using Render::Look::EnhancedFeature;
        using Render::Look::EnhancedFeatureChosen;
        const auto syncFlag = [this](bool& field, const char* name, bool value)
        {
            if (field != value)
            {
                field = value;
                m_RmlView.MarkDirty(name);
            }
        };
        const bool lighting = EnhancedFeatureChosen(EnhancedFeature::Lighting);
        syncFlag(model.lookLighting, "look_lighting", lighting);
        syncFlag(model.lookDynamicLights, "look_dynamic_lights", lighting && EnhancedFeatureChosen(EnhancedFeature::DynamicLights));
        syncFlag(model.lookLightMood, "look_light_mood", EnhancedFeatureChosen(EnhancedFeature::LightMood));
        syncFlag(model.lookPostProcessing, "look_post_processing", EnhancedFeatureChosen(EnhancedFeature::PostProcessing));
        syncFlag(model.lookFeaturesLocked, "look_features_locked", !enhancedGraphics);
        syncFlag(model.lookDynamicLightsLocked, "look_dynamic_lights_locked", !enhancedGraphics || !lighting);
    }
    SyncTuningModel(model);

    // Rebuilt every sync for the localized "Off", as the FPS Limit row's "Uncapped".
    const auto syncMultiplierRow = [&](std::vector<Rml::String>& labels, const char* labelsName, int& index,
                                       const char* indexName, Rml::String& valueLabel, const char* valueName,
                                       std::vector<Rml::String> newLabels, int newIndex)
    {
        if (labels != newLabels)
        {
            labels = std::move(newLabels);
            m_RmlView.MarkDirty(labelsName);
        }
        if (index != newIndex)
        {
            index = newIndex;
            m_RmlView.MarkDirty(indexName);
        }
        syncDropdownValue(valueLabel, valueName, labels, index);
    };
    syncMultiplierRow(model.antialiasingLabels, "antialiasing_labels", model.antialiasingIndex, "antialiasing_index",
                      model.antialiasingValueLabel, "antialiasing_value_label", MultiplierLabels(s_AntialiasingValues),
                      FindValueIndex(s_AntialiasingValues, GameConfig::GetInstance().GetAntialiasing()));
    syncMultiplierRow(model.anisotropyLabels, "anisotropy_labels", model.anisotropyIndex, "anisotropy_index",
                      model.anisotropyValueLabel, "anisotropy_value_label", MultiplierLabels(s_AnisotropyValues),
                      FindValueIndex(s_AnisotropyValues, GameConfig::GetInstance().GetAnisotropy()));

    const bool showFpsCounter = GetShowFpsCounter();
    const bool showDebugInfo = GetShowDebugInfo();
    if (model.showFpsCounter != showFpsCounter) { model.showFpsCounter = showFpsCounter; m_RmlView.MarkDirty("show_fps_counter"); }
    if (model.showDebugInfo != showDebugInfo) { model.showDebugInfo = showDebugInfo; m_RmlView.MarkDirty("show_debug_info"); }

    m_iThemeIndex = FindCurrentThemeIndex();
    if (model.themeIndex != m_iThemeIndex) { model.themeIndex = m_iThemeIndex; m_RmlView.MarkDirty("theme_index"); }
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
        m_RmlView.MarkDirty("ui_scale_labels");
    }
    if (model.uiScaleIndex != m_iUIScaleIndex)
    {
        model.uiScaleIndex = m_iUIScaleIndex;
        m_RmlView.MarkDirty("ui_scale_index");
    }
    syncDropdownValue(model.uiScaleValueLabel, "ui_scale_value_label", model.uiScaleLabels, model.uiScaleIndex);

    // The pack labels are built in RefreshTexturePackChoices(); only the localized "Original"
    // entry can change here.
    SyncLabel(model.texturePackRowLabel, "texture_pack_row_label", I18N::Game::TexturePack);
    SyncLabel(model.texturePackTooltip, "texture_pack_tooltip", I18N::Game::TexturePackTooltip);
    if (!model.texturePackLabels.empty())
        SyncLabel(model.texturePackLabels.front(), "texture_pack_labels", I18N::Game::OriginalTextures);
    if (model.texturePackIndex != m_iTexturePackIndex)
    {
        model.texturePackIndex = m_iTexturePackIndex;
        m_RmlView.MarkDirty("texture_pack_index");
    }
    syncDropdownValue(model.texturePackValueLabel, "texture_pack_value_label", model.texturePackLabels,
                      model.texturePackIndex);
}
