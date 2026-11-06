#include "stdafx.h"

#include "Render/Look/LookProfileSetting.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Render/Lighting/LightMoodSetting.h"
#include "Render/Look/LookFiles.h"
#include "Render/Look/LookLayers.h"
#include "Render/Look/LookTuning.h"
#include "World/MapInfra/MapManager.h"

#if defined(MU_ENABLE_CONTROL_SOCKET) || defined(_EDITOR)
#include "Core/Text/Utf8.h"
#include "Render/Look/LookPresetSetting.h"
#include "Render/Look/LookTuningCommand.h"
#endif

#include <utility>

namespace Render::Look
{
namespace
{
using Data::Maps::LookValue;
using Data::Maps::LookValues;

struct State
{
    // The live documents and everything layered over them (Render/Look/LookLayers.h); a mood change
    // rebuilds the static map light.
    LookLayers layers{[](const Data::Maps::LightMoodData& mood) { Lighting::LightMood::SetMoodData(mood); }};
    std::string source = "shipped";
    // The saves SaveLive made (SaveCount).
    int saves = 0;
};

State s_state;

void LogIssues(const std::vector<Data::Items::ItemDataIssue>& issues)
{
    for (const Data::Items::ItemDataIssue& issue : issues)
    {
        mu::log::Get("render")->warn("Look profile: {}", issue.ToString());
    }
}

void UseFiles(LookFiles files, std::string source)
{
    s_state.source = std::move(source);
    s_state.layers.UseFiles(std::move(files));
}

LookFilesLoad LoadShipped(LookFileSystem& fileSystem)
{
    LookFilesLoad loaded = LoadLookFiles(fileSystem, Data::Maps::LookProfileFile, Data::Maps::LightMoodFile);
    LogIssues(loaded.issues);
    if (!loaded.files.lookUsable)
    {
        mu::log::Get("render")->warn("Look profile unusable; the built-in values apply");
    }
    if (!loaded.files.moodUsable)
    {
        mu::log::Get("render")->warn("Light mood data unusable; the world keeps its Classic light");
    }
    return loaded;
}
} // namespace

void LoadProfile()
{
    DiskLookFileSystem fileSystem;
    UseFiles(LoadShipped(fileSystem).files, "shipped");
}

const LookValues& Active()
{
    return s_state.layers.Active(gMapManager.WorldActive);
}

void SetPersonalLookOverrides(const Data::Maps::LookOverrides& overrides)
{
    s_state.layers.SetPersonal(overrides);
}

void SetPersonalTuningGroups(const TuningGroupSet& applying)
{
    s_state.layers.SetApplyingGroups(applying);
}

LookValues InheritedValues()
{
    return s_state.layers.Inherited(gMapManager.WorldActive);
}

float Value(LookValue value)
{
    return static_cast<float>(Active()[value]);
}

std::uint32_t Revision()
{
    return s_state.layers.Revision();
}

Lighting::LightHistorySettings LightHistorySettings()
{
    const LookValues& values = Active();
    Lighting::LightHistorySettings settings;
    settings.mapSmoothingSeconds = static_cast<float>(values[LookValue::DynamicMapSmoothing]);
    settings.effectGain = static_cast<float>(values[LookValue::DynamicEffectGain]);
    settings.effectRangeScale = static_cast<float>(values[LookValue::DynamicEffectRange]);
    settings.effectRangeMax = static_cast<float>(values[LookValue::DynamicEffectRangeMax]);
    settings.effectFadeSeconds = static_cast<float>(values[LookValue::DynamicEffectFade]);
    settings.effectCap = static_cast<float>(values[LookValue::DynamicEffectCap]);
    return settings;
}

#if defined(MU_ENABLE_CONTROL_SOCKET) || defined(_EDITOR)
namespace
{
// An edit's result: applied, the next frame uses the live values (Render/Look/LookLayers.h has
// already rebuilt the map light for a mood edit); refused, nothing changed.
bool Applied(const LookEditResult& result, const char* what)
{
    if (!result.applied)
    {
        mu::log::Get("render")->warn("$look {} refused: {}", what, result.error);
        return false;
    }
    return true;
}

std::string MapName(int world)
{
    const wchar_t* name = gMapManager.GetMapName(world);
    return name ? Core::Text::ToUtf8(name) : std::string();
}

void LogValue(const LookValueState& state, int world)
{
    mu::log::Get("render")->info(
        "$look {} = {} ({}{}), range {} to {}", state.name, state.value, ToString(state.source),
        state.source == LookValueSource::Map ? " " + std::to_string(world) : std::string(), state.min, state.max);
}

bool Show(std::optional<std::string_view> name)
{
    const std::vector<LookValueState> states = DescribeLive(name);
    if (states.empty())
    {
        mu::log::Get("render")->warn("$look show refused: {}: is not a look value", *name);
        return false;
    }
    const int world = gMapManager.WorldActive;
    mu::log::Get("render")->info("$look values on world {} from {}; preset {}, overrides: {}", world, s_state.source,
                                 ActiveLookPreset() == LookPreset::Enhanced ? "Enhanced" : "Classic",
                                 DescribeFeatureOverrides());
    for (const LookValueState& state : states)
    {
        LogValue(state, world);
    }
    mu::log::Get("render")->flush();
    return true;
}
} // namespace

const LookFiles& LiveFiles()
{
    return s_state.layers.Live();
}

const std::string& LiveSource()
{
    return s_state.source;
}

std::vector<LookValueState> DescribeLive(std::optional<std::string_view> name)
{
    return s_state.layers.Describe(gMapManager.WorldActive, name);
}

bool SetLive(std::span<const LookEdit> edits, bool currentMap)
{
    const int world = gMapManager.WorldActive;
    if (currentMap && world < 0)
    {
        mu::log::Get("render")->warn("$look set map refused: no map is active");
        return false;
    }
    const std::optional<int> scope = currentMap ? std::optional<int>(world) : std::nullopt;
    const LookEditResult result = s_state.layers.Set(edits, scope, currentMap ? MapName(world) : std::string());
    if (!Applied(result, "set"))
    {
        return false;
    }
    for (std::size_t index = 0; index < edits.size(); ++index)
    {
        mu::log::Get("render")->info("$look set {}{}: {} -> {}", currentMap ? "map " + std::to_string(world) + " " : "",
                                     edits[index].name, result.oldValues[index], edits[index].value);
    }
    return true;
}

bool ResetLive(std::optional<std::string_view> name)
{
    if (!Applied(s_state.layers.Reset(name, gMapManager.WorldActive), "reset"))
    {
        return false;
    }
    mu::log::Get("render")->info("$look reset {} to the values loaded from {}",
                                 name ? std::string(*name) : "every value", s_state.source);
    return true;
}

bool LoadLive(std::string_view candidate)
{
    DiskLookFileSystem fileSystem;
    if (candidate.empty())
    {
        UseFiles(LoadShipped(fileSystem).files, "shipped");
        mu::log::Get("render")->info("$look load: the shipped files");
        return true;
    }
    const std::filesystem::path folder = std::filesystem::path("Data/Render/Candidates") / std::string(candidate);
    std::vector<Data::Items::ItemDataIssue> issues;
    std::optional<LookFiles> files = LoadCandidateFiles(fileSystem, folder / "Look.json", folder / "LightMood.json",
                                                        Data::Maps::LightMoodFile, issues);
    LogIssues(issues);
    if (!files)
    {
        mu::log::Get("render")->warn("$look load refused: candidate {} is unusable; nothing changed", candidate);
        return false;
    }
    UseFiles(std::move(*files), std::string(candidate));
    mu::log::Get("render")->info("$look load: candidate {}", candidate);
    return true;
}

bool SaveLive()
{
    DiskLookFileSystem fileSystem;
    const LookSaveResult result =
        SaveLookFiles(fileSystem, s_state.layers.Live(), Data::Maps::LookProfileFile, Data::Maps::LightMoodFile);
    if (!result.saved)
    {
        mu::log::Get("render")->error("$look save failed: {}", result.message);
        return false;
    }
    // What was saved is now what a reset goes back to.
    s_state.layers.MarkSaved();
    ++s_state.saves;
    mu::log::Get("render")->info("$look save: {}", result.message);
    return true;
}

int SaveCount()
{
    return s_state.saves;
}

bool HandleDeveloperCommand(std::wstring_view command)
{
    if (const std::optional<LookTuningCommand> alias = ParseLookTuningCommand(command))
    {
        std::vector<LookEdit> edits;
        edits.reserve(alias->count);
        for (std::size_t index = 0; index < alias->count; ++index)
        {
            edits.push_back(
                {Data::Maps::Info(alias->assignments[index].value).name, alias->assignments[index].newValue});
        }
        (void)SetLive(edits, false);
        return true;
    }
    const std::optional<LookCommand> look = ParseLookCommand(command);
    if (!look)
    {
        return false;
    }
    const std::optional<std::string_view> name =
        look->name.empty() ? std::nullopt : std::optional<std::string_view>(look->name);
    switch (look->verb)
    {
    case LookVerb::Show:
        (void)Show(name);
        break;
    case LookVerb::Set:
    {
        const LookEdit edit{look->name, look->value};
        (void)SetLive(std::span<const LookEdit>(&edit, 1), look->map);
        break;
    }
    case LookVerb::Reset:
        (void)ResetLive(name);
        break;
    case LookVerb::Load:
        (void)LoadLive(look->name);
        break;
    case LookVerb::Save:
        (void)SaveLive();
        break;
    }
    return true;
}
#endif

} // namespace Render::Look
