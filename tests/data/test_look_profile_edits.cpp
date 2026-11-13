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
} // namespace

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
