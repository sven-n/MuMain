#include "stdafx.h"

#include "doctest.h"

#include "Data/GameData/MapData/LightMood.h"
#include "Data/GameData/MapData/LookProfileEdits.h"

#include <string>
#include <vector>

using Data::Maps::LookProfileData;
using Data::Maps::LookValue;
using Data::Maps::LookValueEdit;

namespace
{
constexpr int Lorencia = 0;
constexpr int BloodCastle1 = 11;
constexpr int BloodCastle2 = 12;

LookProfileData Profile()
{
    LookProfileData profile;
    std::vector<Data::Items::ItemDataIssue> issues;
    Data::Maps::ReadLookProfileJson(R"({"formatVersion": 2,
        "default": {"bloom.strength": 1, "mood.base": 0.7, "mood.figures": 0.7},
        "environments": {"dim": {"mood.figures": 0.75}},
        "maps": [{"maps": [11, 12, 52], "name": "Blood Castle", "grading.strength": 0.8}]})",
                                    "Look.json", profile, issues);
    REQUIRE(issues.empty());
    return profile;
}

// Three environments in this order (a: 0, b: 1, c: 2); map 1 uses c, maps 2 and 3 share an entry using a.
LookProfileData ProfileWithEnvironments()
{
    LookProfileData profile;
    std::vector<Data::Items::ItemDataIssue> issues;
    Data::Maps::ReadLookProfileJson(R"({"formatVersion": 2,
        "default": {"bloom.strength": 1, "mood.base": 0.7, "mood.figures": 0.7},
        "environments": {"a": {"mood.base": 0.75, "mood.figures": 0.75}, "b": {}, "c": {"bloom.strength": 0.5}},
        "maps": [{"maps": [1], "environment": "c"}, {"maps": [2, 3], "environment": "a", "grading.strength": 0.8}]})",
                                    "Look.json", profile, issues);
    REQUIRE(issues.empty());
    return profile;
}

const std::string& EnvironmentNameOf(const LookProfileData& profile, int world)
{
    const Data::Maps::MapLookOverrides* entry = profile.FindMap(world);
    REQUIRE(entry);
    REQUIRE(entry->environment);
    return profile.environments[*entry->environment].name;
}
} // namespace

using Data::Maps::LookScope;

TEST_CASE("an environment edit reaches every map that uses it, and names those maps [data][look][edits]")
{
    LookProfileData profile = ProfileWithEnvironments();
    const LookValueEdit edit{LookValue::BloomStrength, 0.4};
    const Data::Maps::LookEditOutcome outcome =
        Data::Maps::ApplyLookEdits(profile, {&edit, 1}, LookScope::Environment(0));
    REQUIRE_MESSAGE(outcome.applied, outcome.error);
    CHECK(outcome.entryMaps == std::vector<int>{2, 3});
    CHECK(outcome.oldValues == std::vector<double>{1.0});
    CHECK(profile.ForWorld(3)[LookValue::BloomStrength] == 0.4);
    CHECK(profile.ForWorld(1)[LookValue::BloomStrength] == 0.5);

    const LookProfileData before = profile;
    CHECK_FALSE(Data::Maps::ApplyLookEdits(profile, {&edit, 1}, LookScope::Environment(3)).applied);
    CHECK(profile == before);
}

TEST_CASE("a value removed from a scope keeps the entry, and is refused when it breaks a rule [data][look][edits]")
{
    LookProfileData profile = ProfileWithEnvironments();
    const LookProfileData before = profile;

    // Without its own figures, environment a would light figures (0.7) below its base (0.75).
    const Data::Maps::LookEditOutcome broken =
        Data::Maps::ClearLookValue(profile, LookValue::MoodFigures, LookScope::Environment(0));
    CHECK_FALSE(broken.applied);
    CHECK_MESSAGE(broken.error.find("environments.a") != std::string::npos, broken.error);
    CHECK(profile == before);

    // The shared entry keeps its maps when it names no value of its own any more.
    const Data::Maps::LookEditOutcome cleared =
        Data::Maps::ClearLookValue(profile, LookValue::GradingStrength, LookScope::Map(3));
    REQUIRE_MESSAGE(cleared.applied, cleared.error);
    CHECK(cleared.entryMaps == std::vector<int>{2, 3});
    REQUIRE(profile.FindMap(2));
    CHECK(profile.FindMap(2)->maps == std::vector<int>{2, 3});
    CHECK_FALSE(profile.FindMap(2)->overrides[LookValue::GradingStrength].has_value());

    // A map without an entry has nothing to remove.
    const LookProfileData now = profile;
    CHECK(Data::Maps::ClearLookValue(profile, LookValue::BloomStrength, LookScope::Map(Lorencia)).applied);
    CHECK(profile == now);

    REQUIRE(Data::Maps::ClearLookValue(profile, LookValue::BloomStrength, LookScope::Default()).applied);
    CHECK_FALSE(profile.defaultOverrides[LookValue::BloomStrength].has_value());
}

TEST_CASE("a map's environment is set on the entry it shares [data][look][edits]")
{
    LookProfileData profile = Profile();
    profile.environments.push_back({"dark", {}});

    const Data::Maps::LookEditOutcome shared = Data::Maps::SetMapEnvironment(profile, BloodCastle2, 1);
    REQUIRE_MESSAGE(shared.applied, shared.error);
    CHECK(shared.entryMaps == std::vector<int>{11, 12, 52});
    CHECK(EnvironmentNameOf(profile, 52) == "dark");

    // A map without an entry gets one with only that map.
    REQUIRE(Data::Maps::SetMapEnvironment(profile, Lorencia, 0, "Lorencia").applied);
    CHECK(EnvironmentNameOf(profile, Lorencia) == "dim");
    CHECK(profile.FindMap(Lorencia)->name == "Lorencia");

    REQUIRE(Data::Maps::SetMapEnvironment(profile, BloodCastle1, std::nullopt).applied);
    CHECK_FALSE(profile.FindMap(BloodCastle2)->environment.has_value());

    const LookProfileData before = profile;
    const Data::Maps::LookEditOutcome unknown = Data::Maps::SetMapEnvironment(profile, BloodCastle1, 5);
    CHECK_FALSE(unknown.applied);
    CHECK(profile == before);
}

TEST_CASE("environments are added and renamed under names no other environment has [data][look][edits]")
{
    LookProfileData profile = ProfileWithEnvironments();

    REQUIRE(Data::Maps::AddEnvironment(profile, "caves").applied);
    REQUIRE(profile.environments.size() == 4);
    CHECK(profile.environments.back().name == "caves");
    CHECK(profile.environments.back().overrides == Data::Maps::LookOverrides{});

    const LookProfileData before = profile;
    const Data::Maps::LookEditOutcome twice = Data::Maps::AddEnvironment(profile, "b");
    CHECK_FALSE(twice.applied);
    CHECK_MESSAGE(twice.error.find("environments.b") != std::string::npos, twice.error);
    CHECK_FALSE(Data::Maps::AddEnvironment(profile, "").applied);
    CHECK(profile == before);

    // The maps keep their environment through a rename.
    REQUIRE(Data::Maps::RenameEnvironment(profile, 2, "cold").applied);
    CHECK(EnvironmentNameOf(profile, 1) == "cold");
    CHECK_FALSE(Data::Maps::RenameEnvironment(profile, 2, "a").applied);
    CHECK_FALSE(Data::Maps::RenameEnvironment(profile, 9, "x").applied);
    CHECK(EnvironmentNameOf(profile, 1) == "cold");
}

TEST_CASE("an environment in use stays, and removing another renumbers the entries after it [data][look][edits]")
{
    LookProfileData profile = ProfileWithEnvironments();
    CHECK(Data::Maps::MapsUsingEnvironment(profile, 0) == std::vector<int>{2, 3});
    CHECK(Data::Maps::MapsUsingEnvironment(profile, 1).empty());

    const LookProfileData before = profile;
    const Data::Maps::LookEditOutcome used = Data::Maps::RemoveEnvironment(profile, 0);
    CHECK_FALSE(used.applied);
    CHECK_MESSAGE(used.error.find("2, 3") != std::string::npos, used.error);
    CHECK(profile == before);

    REQUIRE(Data::Maps::RemoveEnvironment(profile, 1).applied);
    REQUIRE(profile.environments.size() == 2);
    CHECK(EnvironmentNameOf(profile, 1) == "c");
    CHECK(EnvironmentNameOf(profile, 2) == "a");
    CHECK(profile.ForWorld(1)[LookValue::BloomStrength] == 0.5);
}

TEST_CASE("a profile whose environments share a name, or have none, is refused [data][look]")
{
    LookProfileData profile = ProfileWithEnvironments();
    profile.environments[1].name = "a";
    std::vector<Data::Items::ItemDataIssue> issues;
    CHECK_FALSE(Data::Maps::ValidateLookProfile(profile, "Look.json", issues));

    profile.environments[1].name.clear();
    issues.clear();
    CHECK_FALSE(Data::Maps::ValidateLookProfile(profile, "Look.json", issues));
}

TEST_CASE("a map edit changes the entry every variant of the map shares [data][look][edits]")
{
    LookProfileData profile = Profile();
    const LookValueEdit edit{LookValue::BloomStrength, 0.4};
    const Data::Maps::LookEditOutcome outcome = Data::Maps::ApplyLookEdits(profile, {&edit, 1}, BloodCastle2);
    REQUIRE_MESSAGE(outcome.applied, outcome.error);
    CHECK(outcome.entryMaps == std::vector<int>{11, 12, 52});
    CHECK(outcome.oldValues == std::vector<double>{1.0});
    CHECK_FALSE(outcome.moodChanged);
    CHECK(profile.maps.size() == 1);
    CHECK(profile.ForWorld(BloodCastle1)[LookValue::BloomStrength] == 0.4);
    CHECK(profile.ForWorld(52)[LookValue::BloomStrength] == 0.4);

    // A map without an entry gets one with only that map.
    const Data::Maps::LookEditOutcome own = Data::Maps::ApplyLookEdits(profile, {&edit, 1}, Lorencia, "Lorencia");
    REQUIRE_MESSAGE(own.applied, own.error);
    CHECK(own.entryMaps == std::vector<int>{Lorencia});
    REQUIRE(profile.FindMap(Lorencia));
    CHECK(profile.FindMap(Lorencia)->name == "Lorencia");
}

TEST_CASE("an edit for a map outside 0 to 255, or with nothing to set, changes nothing [data][look][edits]")
{
    LookProfileData profile = Profile();
    const LookProfileData before = profile;
    const LookValueEdit edit{LookValue::BloomStrength, 0.4};
    const Data::Maps::LookEditOutcome outside = Data::Maps::ApplyLookEdits(profile, {&edit, 1}, 300);
    CHECK_FALSE(outside.applied);
    CHECK_MESSAGE(outside.error.find("has map 300") != std::string::npos, outside.error);
    CHECK(profile == before);

    const Data::Maps::LookEditOutcome none = Data::Maps::ApplyLookEdits(profile, {}, Lorencia);
    CHECK(none.applied);
    CHECK(none.entryMaps.empty());
    CHECK(profile == before);
}

TEST_CASE("edits apply as one change, checked on the whole profile [data][look][edits]")
{
    LookProfileData profile = Profile();

    // Each alone breaks figures >= base; together they keep it, and the mood changes.
    const LookValueEdit together[] = {{LookValue::MoodBase, 0.72}, {LookValue::MoodFigures, 0.72}};
    const Data::Maps::LookEditOutcome raised = Data::Maps::ApplyLookEdits(profile, together, std::nullopt);
    REQUIRE_MESSAGE(raised.applied, raised.error);
    CHECK(raised.moodChanged);

    // A default that stays valid itself but breaks the "dim" environment is refused, nothing changed.
    const LookProfileData before = profile;
    const LookValueEdit higher[] = {{LookValue::MoodBase, 0.8}, {LookValue::MoodFigures, 0.8}};
    const Data::Maps::LookEditOutcome refused = Data::Maps::ApplyLookEdits(profile, higher, std::nullopt);
    CHECK_FALSE(refused.applied);
    CHECK_MESSAGE(refused.error.find("environments.dim.mood.figures") != std::string::npos, refused.error);
    CHECK(refused.oldValues.empty());
    CHECK(profile == before);

    // Out of range: refused before anything else.
    const LookValueEdit range{LookValue::GradingStrength, 1.5};
    const Data::Maps::LookEditOutcome outside = Data::Maps::ApplyLookEdits(profile, {&range, 1}, BloodCastle1);
    CHECK_FALSE(outside.applied);
    CHECK_MESSAGE(outside.error.find("grading.strength: 1.5 is outside 0 to 1") != std::string::npos, outside.error);
    CHECK(profile == before);
}

TEST_CASE("a reset puts back the loaded value, or none, and drops an entry the edits added [data][look][edits]")
{
    const LookProfileData loaded = Profile();
    LookProfileData profile = loaded;
    const LookValueEdit grading{LookValue::GradingStrength, 0.3};
    const LookValueEdit bloom{LookValue::BloomStrength, 0.3};
    REQUIRE(Data::Maps::ApplyLookEdits(profile, {&grading, 1}, BloodCastle2).applied);
    REQUIRE(Data::Maps::ApplyLookEdits(profile, {&bloom, 1}, BloodCastle2).applied);
    REQUIRE(Data::Maps::ApplyLookEdits(profile, {&bloom, 1}, Lorencia).applied);

    // Through another map of the entry: the loaded value comes back on all of them.
    const Data::Maps::LookEditOutcome reset =
        Data::Maps::ResetLookValue(profile, loaded, LookValue::GradingStrength, BloodCastle1);
    REQUIRE_MESSAGE(reset.applied, reset.error);
    CHECK(reset.entryMaps == std::vector<int>{11, 12, 52});
    CHECK(profile.ForWorld(BloodCastle2)[LookValue::GradingStrength] == 0.8);
    // A value the loaded entry did not name goes again.
    REQUIRE(Data::Maps::ResetLookValue(profile, loaded, LookValue::BloomStrength, BloodCastle1).applied);
    CHECK_FALSE(profile.FindMap(BloodCastle2)->overrides[LookValue::BloomStrength].has_value());
    // Lorencia's entry was added by the edits and names nothing now: it goes.
    REQUIRE(Data::Maps::ResetLookValue(profile, loaded, LookValue::BloomStrength, Lorencia).applied);
    CHECK(profile.FindMap(Lorencia) == nullptr);
    CHECK(profile == loaded);
}

TEST_CASE("the moods of every map follow the profile [data][look][edits]")
{
    const LookProfileData profile = Profile();
    const Data::Maps::LightMoodData moods = Data::Maps::MoodsOf(profile);
    CHECK(moods.defaultMood.base == 0.7);
    CHECK(moods.maps.empty());
    CHECK(moods.ForWorld(BloodCastle1) == moods.defaultMood);

    LookProfileData edited = profile;
    const LookValueEdit sun{LookValue::MoodSun, 0.5};
    REQUIRE(Data::Maps::ApplyLookEdits(edited, {&sun, 1}, BloodCastle2).applied);
    const Data::Maps::LightMoodData after = Data::Maps::MoodsOf(edited);
    REQUIRE(after.maps.size() == 3);
    CHECK(after.ForWorld(52).sun == 0.5);
    CHECK(after.ForWorld(Lorencia).sun == 1.0);
}

TEST_CASE("an entry of one map that names nothing any more goes, a shared one stays [data][look][edits]")
{
    LookProfileData profile = ProfileWithEnvironments();
    const LookValueEdit edit{LookValue::BloomStrength, 0.4};
    REQUIRE(Data::Maps::ApplyLookEdits(profile, {&edit, 1}, LookScope::Map(Lorencia), "Lorencia").applied);
    REQUIRE(profile.FindMap(Lorencia));
    REQUIRE(Data::Maps::ClearLookValue(profile, LookValue::BloomStrength, LookScope::Map(Lorencia)).applied);
    CHECK(profile.FindMap(Lorencia) == nullptr);

    // Map 1's entry names only its environment: without it, the entry goes.
    REQUIRE(Data::Maps::SetMapEnvironment(profile, 1, std::nullopt).applied);
    CHECK(profile.FindMap(1) == nullptr);

    // Maps 2 and 3 keep sharing their entry without an environment.
    REQUIRE(Data::Maps::SetMapEnvironment(profile, 2, std::nullopt).applied);
    REQUIRE(profile.FindMap(3));
    CHECK(profile.FindMap(3)->maps == std::vector<int>{2, 3});
}
