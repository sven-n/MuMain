#include "stdafx.h"

#include "doctest.h"

#include "Data/GameData/MapData/LookProfileEdits.h"
#include "Render/Look/LookFileSystem.h"
#include "Render/Look/LookLayers.h"
#include "Render/Look/TuningControls.h"

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

using Data::Maps::LookValue;
using Render::Look::LookEdit;
using Render::Look::LookLayers;
using Render::Look::LookPreview;
using Render::Look::LookValueSource;

namespace
{
constexpr int Lorencia = 0;
constexpr int Icarus = 10;
constexpr int BloodCastle2 = 12;
constexpr int BloodCastle3 = 13;

const Render::Look::TuningGroupSet AllGroups{true, true, true, true, true};

const std::string LookText = R"({
  "formatVersion": 2,
  "default": { "bloom.strength": 1.0, "lighting.highlightKnee": 0.8, "mood.base": 0.7, "mood.figures": 0.7,
               "mood.sun": 0.6 },
  "maps": [ { "maps": [10], "name": "Icarus", "bloom.strength": 0.3 },
            { "maps": [11, 12, 13], "name": "Blood Castle" } ]
})";

// Reads only; the layers never write.
class MemoryFileSystem final : public Render::Look::LookFileSystem
{
public:
    std::map<std::string, std::string> files{{"Look.json", LookText}};

    bool Exists(const std::filesystem::path& path) override
    {
        return files.contains(path.generic_string());
    }
    std::optional<std::string> Read(const std::filesystem::path& path) override
    {
        const auto file = files.find(path.generic_string());
        return file == files.end() ? std::nullopt : std::optional<std::string>(file->second);
    }
    bool Write(const std::filesystem::path&, const std::string&) override
    {
        return false;
    }
    bool Copy(const std::filesystem::path&, const std::filesystem::path&) override
    {
        return false;
    }
    bool Rename(const std::filesystem::path&, const std::filesystem::path&) override
    {
        return false;
    }
    bool Remove(const std::filesystem::path&) override
    {
        return false;
    }
};

Render::Look::LookFiles Files()
{
    MemoryFileSystem fileSystem;
    Render::Look::LookFilesLoad loaded = Render::Look::LoadLookFiles(fileSystem, "Look.json", "LightMood.json");
    REQUIRE(loaded.issues.empty());
    REQUIRE(loaded.files.lookUsable);
    return loaded.files;
}

// A light mood rebuild that, like Lighting::LightMood's, reads the look values while it rebuilds.
struct MoodProbe
{
    LookLayers* layers = nullptr;
    int world = Lorencia;
    std::vector<double> bloomSeen;

    LookLayers::MoodRebuild Hook()
    {
        return [this](const Data::Maps::LightMoodData&)
        {
            if (layers)
            {
                bloomSeen.push_back(layers->Active(world)[LookValue::BloomStrength]);
            }
        };
    }
};

double Bloom(LookLayers& layers, int world)
{
    return layers.Active(world)[LookValue::BloomStrength];
}

Data::Maps::LookOverrides PersonalBloom(double value)
{
    Data::Maps::LookOverrides personal;
    personal[LookValue::BloomStrength] = value;
    return personal;
}
} // namespace

TEST_CASE("skill-light tuning chosen once applies globally across map changes [render][look][tuning]")
{
    Data::Maps::LookOverrides personal;
    personal[LookValue::DynamicEffectGain] = 1.0;
    personal[LookValue::DynamicEffectRange] = 1.5;
    personal[LookValue::DynamicEffectRangeMax] = 4.0;
    personal[LookValue::DynamicEffectCap] = 1.4;

    LookLayers layers;
    layers.UseFiles(Files());
    layers.SetApplyingGroups(AllGroups);
    layers.SetPersonal(personal);
    for (const int world : {Lorencia, Icarus, Lorencia})
    {
        CAPTURE(world);
        const Data::Maps::LookValues& values = layers.Active(world);
        CHECK(values[LookValue::DynamicEffectGain] == 1.0);
        CHECK(values[LookValue::DynamicEffectRange] == 1.5);
        CHECK(values[LookValue::DynamicEffectRangeMax] == 4.0);
        CHECK(values[LookValue::DynamicEffectCap] == 1.4);
        CHECK(layers.Personal() == personal);
    }
}

TEST_CASE("an edit switches the Player preview to Authoring, without the player's values [render][look][tuning]")
{
    LookLayers layers;
    layers.UseFiles(Files());
    layers.SetApplyingGroups(AllGroups);
    layers.SetPersonal(PersonalBloom(0.5));
    REQUIRE(layers.Preview() == LookPreview::Player);
    CHECK(Bloom(layers, Lorencia) == 0.5);

    // A refused edit changes nothing, the preview neither.
    const LookEdit refused{"bloom.strength", 9.0};
    const std::uint32_t revision = layers.Revision();
    CHECK_FALSE(layers.Set({&refused, 1}, Icarus).applied);
    CHECK(layers.Revision() == revision);
    CHECK(layers.Preview() == LookPreview::Player);

    // `$look set map bloom.strength 0.2` in Icarus: the authored values everywhere, not the player's.
    const LookEdit edit{"bloom.strength", 0.2};
    REQUIRE(layers.Set({&edit, 1}, Icarus, "Icarus").applied);
    CHECK(layers.Preview() == LookPreview::Authoring);
    CHECK(Bloom(layers, Icarus) == 0.2);
    CHECK(Bloom(layers, Lorencia) == 1.0);
    std::vector<Render::Look::LookValueState> states = layers.Describe(Lorencia, "bloom.strength");
    REQUIRE(states.size() == 1);
    CHECK(states[0].source == LookValueSource::Default);
    CHECK(states[0].value == 1.0);

    // Back to the Player preview: the player's value over every map, the edit kept underneath.
    layers.SetPreview(LookPreview::Player);
    CHECK(Bloom(layers, Icarus) == 0.5);
    states = layers.Describe(Icarus, "bloom.strength");
    REQUIRE(states.size() == 1);
    CHECK(states[0].source == LookValueSource::Personal);
    CHECK(layers.Live().lookData.FindMap(Icarus)->overrides[LookValue::BloomStrength] == 0.2);
    // The next edit authors again.
    REQUIRE(layers.Set({&edit, 1}, std::nullopt).applied);
    CHECK(layers.Preview() == LookPreview::Authoring);
}

TEST_CASE("the mood rebuild sees the preview an edit switched to [render][look][tuning]")
{
    MoodProbe probe;
    LookLayers layers(probe.Hook());
    probe.layers = &layers;
    layers.UseFiles(Files());
    layers.SetApplyingGroups(AllGroups);
    layers.SetPersonal(PersonalBloom(0.5));

    // A look edit alone still rebuilds the mood once, since the player's values leave with the preview.
    const LookEdit edit{"bloom.strength", 0.2};
    probe.bloomSeen.clear();
    const Render::Look::LookEditResult result = layers.Set({&edit, 1}, std::nullopt);
    REQUIRE_MESSAGE(result.applied, result.error);
    CHECK_FALSE(result.moodChanged);
    REQUIRE(probe.bloomSeen.size() == 1);
    CHECK(probe.bloomSeen[0] == 0.2);

    // Switching back rebuilds it with the player's value; the same preview again rebuilds nothing.
    probe.bloomSeen.clear();
    layers.SetPreview(LookPreview::Player);
    layers.SetPreview(LookPreview::Player);
    REQUIRE(probe.bloomSeen.size() == 1);
    CHECK(probe.bloomSeen[0] == 0.5);

    // An edit made in Authoring rebuilds only for a mood change.
    layers.SetPreview(LookPreview::Authoring);
    probe.bloomSeen.clear();
    const LookEdit bloom{"bloom.strength", 0.4};
    REQUIRE(layers.Set({&bloom, 1}, std::nullopt).applied);
    CHECK(probe.bloomSeen.empty());
    const LookEdit mood{"mood.sun", 0.5};
    REQUIRE(layers.Set({&mood, 1}, std::nullopt).applied);
    REQUIRE(probe.bloomSeen.size() == 1);
    CHECK(probe.bloomSeen[0] == 0.4);
}

TEST_CASE("save, reset and new files keep the preview [render][look][tuning]")
{
    LookLayers layers;
    layers.UseFiles(Files());
    layers.SetApplyingGroups(AllGroups);
    layers.SetPersonal(PersonalBloom(0.5));

    const LookEdit edit{"bloom.strength", 0.2};
    REQUIRE(layers.Set({&edit, 1}, std::nullopt).applied);
    // Saved, the authored value still shows: the preview, not the file, decides.
    layers.MarkSaved();
    CHECK(layers.Preview() == LookPreview::Authoring);
    CHECK(Bloom(layers, Lorencia) == 0.2);
    CHECK(layers.Loaded().lookData == layers.Live().lookData);

    const LookEdit more{"bloom.strength", 1.8};
    REQUIRE(layers.Set({&more, 1}, std::nullopt).applied);
    REQUIRE(layers.Reset(std::nullopt, Lorencia).applied);
    CHECK(layers.Preview() == LookPreview::Authoring);
    CHECK(Bloom(layers, Lorencia) == 0.2);

    layers.UseFiles(Files());
    CHECK(layers.Preview() == LookPreview::Authoring);
    CHECK(Bloom(layers, Lorencia) == 1.0);
    layers.SetPreview(LookPreview::Player);
    CHECK(Bloom(layers, Lorencia) == 0.5);
}

TEST_CASE("an edit on a shared entry reaches every map of it, and its reset too [render][look][tuning]")
{
    LookLayers layers;
    layers.UseFiles(Files());

    // `$look set map bloom.strength 0.2` in Blood Castle 2, then to Blood Castle 3: the same entry.
    const LookEdit edit{"bloom.strength", 0.2};
    const Render::Look::LookEditResult set = layers.Set({&edit, 1}, BloodCastle2, "Blood Castle");
    REQUIRE_MESSAGE(set.applied, set.error);
    CHECK(set.entryMaps == std::vector<int>{11, 12, 13});
    CHECK(Bloom(layers, BloodCastle3) == 0.2);
    const std::vector<Render::Look::LookValueState> states = layers.Describe(BloodCastle3, "bloom.strength");
    REQUIRE(states.size() == 1);
    CHECK(states[0].source == LookValueSource::Map);
    CHECK(Bloom(layers, Lorencia) == 1.0);

    // A named reset made in Blood Castle 3 restores the entry on all its maps.
    REQUIRE(layers.Reset(std::string_view("bloom.strength"), BloodCastle3).applied);
    for (const int world : {11, BloodCastle2, BloodCastle3})
    {
        CAPTURE(world);
        CHECK(Bloom(layers, world) == 1.0);
    }
    CHECK_FALSE(layers.Live().lookData.FindMap(BloodCastle2)->overrides[LookValue::BloomStrength].has_value());
}

TEST_CASE("the mood rebuild reads the files' mood, never a player's [render][look][tuning]")
{
    std::vector<Data::Maps::LightMoodData> rebuilt;
    LookLayers layers([&rebuilt](const Data::Maps::LightMoodData& moods) { rebuilt.push_back(moods); });
    layers.UseFiles(Files());
    layers.SetApplyingGroups(AllGroups);
    Data::Maps::LookOverrides personal;
    personal[LookValue::MoodBase] = 0.3;
    layers.SetPersonal(personal);
    CHECK(layers.Active(Lorencia)[LookValue::MoodBase] == 0.7);

    const LookEdit edit{"mood.sun", 0.5};
    REQUIRE(layers.Set({&edit, 1}, std::nullopt).applied);
    REQUIRE_FALSE(rebuilt.empty());
    CHECK(rebuilt.back().defaultMood.base == 0.7);
    CHECK(rebuilt.back().defaultMood.sun == 0.5);
}

TEST_CASE("an environment's value is named as the source of a map's [render][look][tuning]")
{
    MemoryFileSystem fileSystem;
    fileSystem.files["Look.json"] = R"({
  "formatVersion": 2,
  "default": { "bloom.strength": 1.0 },
  "environments": { "enclosed": { "bloom.strength": 0.3 } },
  "maps": [ { "maps": [1], "name": "Dungeon", "environment": "enclosed" } ]
})";
    Render::Look::LookFilesLoad loaded = Render::Look::LoadLookFiles(fileSystem, "Look.json", "LightMood.json");
    REQUIRE(loaded.issues.empty());
    constexpr int Enclosed = 1;

    LookLayers layers;
    layers.UseFiles(loaded.files);
    layers.SetApplyingGroups(AllGroups);
    layers.SetPersonal(PersonalBloom(0.5));
    layers.SetPreview(LookPreview::Authoring);

    CHECK(Bloom(layers, Enclosed) == 0.3);
    const std::vector<Render::Look::LookValueState> states = layers.Describe(Enclosed, "bloom.strength");
    REQUIRE(states.size() == 1);
    CHECK(states[0].source == LookValueSource::Environment);
    CHECK(states[0].value == 0.3);
    CHECK(std::string(Render::Look::ToString(LookValueSource::Environment)) == "environment");
}

TEST_CASE("the player's highlight knee follows Enhanced lighting and is kept while it is off [render][look][tuning]")
{
    LookLayers layers;
    layers.UseFiles(Files());
    Render::Look::LookFeatures features = Render::Look::PresetFeatures(LookPreset::Enhanced);
    layers.SetApplyingGroups(Render::Look::TuningGroupsApplying(features));
    Data::Maps::LookOverrides personal = PersonalBloom(0.5);
    personal[LookValue::HighlightKnee] = 0.6;
    layers.SetPersonal(personal);
    CHECK(layers.Active(Lorencia)[LookValue::HighlightKnee] == 0.6);

    // Enhanced lighting off, the darker map light still on: the map's knee, a new revision (the light
    // mood recomputes its scales from it), the player's value kept.
    features.enhancedLighting = false;
    const std::uint32_t revision = layers.Revision();
    layers.SetApplyingGroups(Render::Look::TuningGroupsApplying(features));
    CHECK(layers.Revision() != revision);
    CHECK(layers.Active(Lorencia)[LookValue::HighlightKnee] == 0.8);
    CHECK(layers.Active(Lorencia)[LookValue::BloomStrength] == 0.5);
    CHECK(layers.Personal()[LookValue::HighlightKnee] == 0.6);
    const std::vector<Render::Look::LookValueState> knee = layers.Describe(Lorencia, "lighting.highlightKnee");
    REQUIRE(knee.size() == 1);
    CHECK(knee[0].source == LookValueSource::Default);

    // On again: the player's knee is back.
    features.enhancedLighting = true;
    layers.SetApplyingGroups(Render::Look::TuningGroupsApplying(features));
    CHECK(layers.Active(Lorencia)[LookValue::HighlightKnee] == 0.6);

    // Classic shows none of the player's values and keeps them all.
    layers.SetApplyingGroups(Render::Look::TuningGroupsApplying(Render::Look::PresetFeatures(LookPreset::Classic)));
    CHECK(layers.Active(Lorencia) == layers.Inherited(Lorencia));
    CHECK(layers.Personal() == personal);
}

TEST_CASE("an unchanged player value or group set keeps the cache and revision [render][look][tuning]")
{
    LookLayers layers;
    layers.UseFiles(Files());
    layers.SetApplyingGroups(AllGroups);
    layers.SetPersonal(PersonalBloom(0.5));
    const std::uint32_t revision = layers.Revision();
    layers.SetPersonal(PersonalBloom(0.5));
    layers.SetApplyingGroups(AllGroups);
    CHECK(layers.Revision() == revision);
}

TEST_CASE("a refused Enhanced lighting applies the map's knee to the mood until the renderer accepts it "
          "[render][look][tuning]")
{
    // The mood rebuild reads the knee while it rebuilds, as Lighting::LightMood's does.
    LookLayers layers;
    layers.UseFiles(Files());
    Data::Maps::LookOverrides personal = PersonalBloom(0.5);
    personal[LookValue::HighlightKnee] = 0.6;
    layers.SetPersonal(personal);

    bool rendererCanLight = false;
    std::vector<double> kneeSeen;
    std::vector<bool> moodEnabled;
    const Render::Look::LookFeatureAppliers appliers{
        {},
        [&rendererCanLight](bool enabled) { return enabled && rendererCanLight; },
        [](bool enabled) { return enabled; },
        {},
        [&layers](const Render::Look::TuningGroupSet& applying) { layers.SetApplyingGroups(applying); },
        [&](bool enabled, int)
        {
            moodEnabled.push_back(enabled);
            kneeSeen.push_back(layers.Active(Lorencia)[LookValue::HighlightKnee]);
        },
        {}};
    const Render::Look::LookFeatures requested = Render::Look::PresetFeatures(LookPreset::Enhanced);
    REQUIRE(requested.enhancedLighting);
    REQUIRE(requested.lightMood);

    // Requested, refused: the mood stays on with the map's knee; bloom (post-processing drawn) is
    // still the player's, and the player's knee stays stored.
    Render::Look::ApplyLookFeatures(requested, appliers);
    REQUIRE(kneeSeen.size() == 1);
    CHECK(moodEnabled.back());
    CHECK(kneeSeen.back() == 0.8);
    CHECK(layers.Active(Lorencia)[LookValue::HighlightKnee] == 0.8);
    CHECK(layers.Active(Lorencia)[LookValue::BloomStrength] == 0.5);
    CHECK(layers.Personal()[LookValue::HighlightKnee] == 0.6);
    CHECK(layers.ApplyingGroups() == Render::Look::TuningGroupSet{false, false, true, true, false});

    // The renderer recovers and the look is applied again: the rebuild reads the player's knee.
    rendererCanLight = true;
    Render::Look::ApplyLookFeatures(requested, appliers);
    REQUIRE(kneeSeen.size() == 2);
    CHECK(kneeSeen.back() == 0.6);
    CHECK(layers.Active(Lorencia)[LookValue::HighlightKnee] == 0.6);
    CHECK(layers.ApplyingGroups() == Render::Look::TuningGroupSet{true, true, true, true, true});
}

TEST_CASE("a refused post-processing keeps the player's bloom stored but unapplied [render][look][tuning]")
{
    LookLayers layers;
    layers.UseFiles(Files());
    layers.SetPersonal(PersonalBloom(0.5));
    const Render::Look::LookFeatureAppliers appliers{{},
                                                     [](bool enabled) { return enabled; },
                                                     [](bool) { return false; },
                                                     {},
                                                     [&layers](const Render::Look::TuningGroupSet& applying)
                                                     { layers.SetApplyingGroups(applying); },
                                                     {},
                                                     {}};
    Render::Look::ApplyLookFeatures(Render::Look::PresetFeatures(LookPreset::Enhanced), appliers);
    CHECK(Bloom(layers, Lorencia) == 1.0);
    CHECK(layers.Personal()[LookValue::BloomStrength] == 0.5);
    CHECK(layers.ApplyingGroups() == Render::Look::TuningGroupSet{true, true, true, false, true});
}

TEST_CASE("a replaced profile goes live in the Authoring preview, and an invalid one changes nothing "
          "[render][look][tuning]")
{
    LookLayers layers;
    layers.UseFiles(Files());
    layers.SetApplyingGroups(AllGroups);
    layers.SetPersonal(PersonalBloom(0.5));
    const std::uint32_t files = layers.FilesGeneration();
    const Data::Maps::LookProfileData loaded = layers.Live().lookData;

    // Figures below base: refused, nothing changes, the preview neither.
    Data::Maps::LookProfileData broken = loaded;
    broken.defaultOverrides[LookValue::MoodFigures] = 0.5;
    const Render::Look::LookEditResult refused = layers.Replace(broken);
    CHECK_FALSE(refused.applied);
    CHECK_FALSE(refused.error.empty());
    CHECK(layers.Live().lookData == loaded);
    CHECK(layers.Preview() == LookPreview::Player);

    // The Graphics Look window gives Blood Castle an environment of its own.
    Data::Maps::LookProfileData edited = loaded;
    REQUIRE(Data::Maps::AddEnvironment(edited, "dark").applied);
    const Data::Maps::LookValueEdit bloom{LookValue::BloomStrength, 0.2};
    REQUIRE(Data::Maps::ApplyLookEdits(edited, {&bloom, 1}, Data::Maps::LookScope::Environment(0)).applied);
    REQUIRE(Data::Maps::SetMapEnvironment(edited, BloodCastle2, 0).applied);

    const Render::Look::LookEditResult replaced = layers.Replace(edited);
    REQUIRE_MESSAGE(replaced.applied, replaced.error);
    CHECK_FALSE(replaced.moodChanged);
    CHECK(layers.Preview() == LookPreview::Authoring);
    CHECK(layers.Live().lookData == edited);
    CHECK(Bloom(layers, BloodCastle3) == 0.2);
    CHECK(Bloom(layers, Lorencia) == 1.0);

    // Back to the files' profile (an undo): the files' values, still as authored.
    REQUIRE(layers.Replace(loaded).applied);
    CHECK(Bloom(layers, BloodCastle3) == 1.0);
    CHECK(layers.Preview() == LookPreview::Authoring);

    // A mood change says so: the light mood rebuilds the map light from it.
    Data::Maps::LookProfileData sunnier = loaded;
    sunnier.defaultOverrides[LookValue::MoodSun] = 0.9;
    CHECK(layers.Replace(sunnier).moodChanged);
    CHECK(layers.FilesGeneration() == files);
    layers.UseFiles(Files());
    CHECK(layers.FilesGeneration() != files);
}

TEST_CASE("a profile replaced on an unusable file is refused [render][look][tuning]")
{
    LookLayers layers;
    Render::Look::LookFiles files = Files();
    files.lookUsable = false;
    layers.UseFiles(files);
    Data::Maps::LookProfileData edited = layers.Live().lookData;
    edited.defaultOverrides[LookValue::BloomStrength] = 0.4;
    CHECK_FALSE(layers.Replace(edited).applied);
    CHECK(layers.Live().lookData != edited);
}
