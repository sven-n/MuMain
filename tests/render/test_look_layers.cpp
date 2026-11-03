#include "stdafx.h"

#include "doctest.h"

#include "Render/Look/LookFileSystem.h"
#include "Render/Look/LookLayers.h"
#include "Render/Look/TuningControls.h"

#include <map>
#include <optional>
#include <string>
#include <vector>

using Data::Maps::LookValue;
using Render::Look::LookEdit;
using Render::Look::LookLayers;
using Render::Look::LookValueSource;

namespace
{
constexpr int Lorencia = 0;
constexpr int Icarus = 10;

const Render::Look::TuningGroupSet AllGroups{true, true, true, true};

const std::string LookText = R"({
  "formatVersion": 1,
  "default": { "bloom.strength": 1.0, "lighting.highlightKnee": 0.8 },
  "maps": [ { "world": 10, "name": "Icarus", "bloom.strength": 0.3 } ]
})";

const std::string MoodText = R"({
  "formatVersion": 1,
  "default": { "base": 0.7, "figures": 0.7, "dynamicGain": 1.0, "background": 1.0, "sun": 0.6 },
  "maps": []
})";

// Reads only; the layers never write.
class MemoryFileSystem final : public Render::Look::LookFileSystem
{
public:
    std::map<std::string, std::string> files{{"Look.json", LookText}, {"LightMood.json", MoodText}};

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
    REQUIRE(loaded.files.moodUsable);
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

TEST_CASE("a full $look reset rebuilds the mood with the player's values restored [render][look][tuning]")
{
    MoodProbe probe;
    LookLayers layers(probe.Hook());
    probe.layers = &layers;
    layers.UseFiles(Files());
    layers.SetApplyingGroups(AllGroups);
    layers.SetPersonal(PersonalBloom(0.5));
    CHECK(Bloom(layers, Lorencia) == 0.5);

    const LookEdit edit{"bloom.strength", 0.2};
    REQUIRE(layers.Set({&edit, 1}, std::nullopt).applied);
    CHECK(Bloom(layers, Lorencia) == 0.2);

    // A full reset restores the mood too: its rebuild reads the look values. It must already see
    // the developer's edit gone, and nothing may keep what it read before the marks were cleared.
    probe.bloomSeen.clear();
    const std::uint32_t before = layers.Revision();
    REQUIRE(layers.Reset(std::nullopt, Lorencia).applied);
    CHECK(layers.Revision() != before);
    REQUIRE(probe.bloomSeen.size() == 1);
    CHECK(probe.bloomSeen[0] == 0.5);
    CHECK(Bloom(layers, Lorencia) == 0.5);
    CHECK(layers.DeveloperEdits() == Render::Look::DeveloperLookEdits{});
}

TEST_CASE("a developer's set is marked before the mood rebuild reads it [render][look][tuning]")
{
    MoodProbe probe;
    LookLayers layers(probe.Hook());
    probe.layers = &layers;
    layers.UseFiles(Files());
    layers.SetApplyingGroups(AllGroups);
    layers.SetPersonal(PersonalBloom(0.5));

    // A mood edit together with a look edit: the rebuild sees the developer's bloom, not the player's.
    const LookEdit edits[] = {{"bloom.strength", 0.2}, {"mood.sun", 0.5}};
    probe.bloomSeen.clear();
    const Render::Look::LookEditResult result = layers.Set(edits, std::nullopt);
    REQUIRE_MESSAGE(result.applied, result.error);
    CHECK(result.moodChanged);
    REQUIRE(probe.bloomSeen.size() == 1);
    CHECK(probe.bloomSeen[0] == 0.2);
    CHECK(Bloom(layers, Lorencia) == 0.2);

    // A refused edit changes nothing, marks nothing.
    const LookEdit refused{"bloom.strength", 9.0};
    const std::uint32_t revision = layers.Revision();
    CHECK_FALSE(layers.Set({&refused, 1}, Icarus).applied);
    CHECK(layers.Revision() == revision);
    CHECK(layers.DeveloperEdits().worlds.empty());
}

TEST_CASE("a map-scoped developer edit leaves the player's values on other maps [render][look][tuning]")
{
    LookLayers layers;
    layers.UseFiles(Files());
    layers.SetApplyingGroups(AllGroups);
    layers.SetPersonal(PersonalBloom(0.5));

    // `$look set map bloom.strength 0.2` in Icarus, then to Lorencia and back.
    const LookEdit edit{"bloom.strength", 0.2};
    REQUIRE(layers.Set({&edit, 1}, Icarus, "Icarus").applied);
    CHECK(Bloom(layers, Icarus) == 0.2);
    CHECK(Bloom(layers, Lorencia) == 0.5);
    CHECK(Bloom(layers, Icarus) == 0.2);

    // Describe agrees: the player's value in Lorencia, the developer's map value in Icarus.
    const std::vector<Render::Look::LookValueState> lorencia = layers.Describe(Lorencia, "bloom.strength");
    REQUIRE(lorencia.size() == 1);
    CHECK(lorencia[0].source == LookValueSource::Personal);
    CHECK(lorencia[0].value == 0.5);
    const std::vector<Render::Look::LookValueState> icarus = layers.Describe(Icarus, "bloom.strength");
    REQUIRE(icarus.size() == 1);
    CHECK(icarus[0].source == LookValueSource::Map);
    CHECK(icarus[0].value == 0.2);

    // A named reset made in Lorencia does not touch Icarus's edit ...
    REQUIRE(layers.Reset(std::string_view("bloom.strength"), Lorencia).applied);
    CHECK(Bloom(layers, Icarus) == 0.2);
    // ... one made in Icarus restores its entry and the player's value there.
    REQUIRE(layers.Reset(std::string_view("bloom.strength"), Icarus).applied);
    CHECK(Bloom(layers, Icarus) == 0.5);
    CHECK(Bloom(layers, Lorencia) == 0.5);
}

TEST_CASE("a saved developer edit is the files' value, under the player's [render][look][tuning]")
{
    LookLayers layers;
    layers.UseFiles(Files());
    layers.SetApplyingGroups(AllGroups);
    layers.SetPersonal(PersonalBloom(0.5));

    const LookEdit edit{"bloom.strength", 0.2};
    REQUIRE(layers.Set({&edit, 1}, std::nullopt).applied);
    CHECK(Bloom(layers, Lorencia) == 0.2);
    layers.MarkSaved();
    CHECK(layers.DeveloperEdits() == Render::Look::DeveloperLookEdits{});
    CHECK(Bloom(layers, Lorencia) == 0.5);
    // Without the player's value, the saved one applies.
    layers.SetPersonal({});
    CHECK(Bloom(layers, Lorencia) == 0.2);
}

TEST_CASE("a default developer edit shows where no map entry overrides it [render][look][tuning]")
{
    LookLayers layers;
    layers.UseFiles(Files());
    layers.SetApplyingGroups(AllGroups);
    layers.SetPersonal(PersonalBloom(0.5));

    const LookEdit edit{"bloom.strength", 1.8};
    REQUIRE(layers.Set({&edit, 1}, std::nullopt).applied);
    CHECK(Bloom(layers, Lorencia) == 1.8);
    // Icarus keeps its own entry, over which the player's value applies.
    CHECK(Bloom(layers, Icarus) == 0.5);

    // A named reset clears the default mark: the player's value everywhere again.
    REQUIRE(layers.Reset(std::string_view("bloom.strength"), Lorencia).applied);
    CHECK(Bloom(layers, Lorencia) == 0.5);

    // Loading files clears every mark.
    REQUIRE(layers.Set({&edit, 1}, std::nullopt).applied);
    layers.UseFiles(Files());
    CHECK(Bloom(layers, Lorencia) == 0.5);
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
    CHECK(layers.ApplyingGroups() == Render::Look::TuningGroupSet{false, false, true, true});

    // The renderer recovers and the look is applied again: the rebuild reads the player's knee.
    rendererCanLight = true;
    Render::Look::ApplyLookFeatures(requested, appliers);
    REQUIRE(kneeSeen.size() == 2);
    CHECK(kneeSeen.back() == 0.6);
    CHECK(layers.Active(Lorencia)[LookValue::HighlightKnee] == 0.6);
    CHECK(layers.ApplyingGroups() == Render::Look::TuningGroupSet{true, true, true, true});
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
    CHECK(layers.ApplyingGroups() == Render::Look::TuningGroupSet{true, true, true, false});
}
