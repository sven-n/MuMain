#include "stdafx.h"

#include "doctest.h"

#include "Render/Look/LookFileSystem.h"
#include "Render/Look/LookFiles.h"
#include "Render/Look/LookTuningCommand.h"

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

using Data::Maps::LookValue;
using doctest::Approx;
using Render::Look::LookEdit;
using Render::Look::LookEditResult;
using Render::Look::LookFiles;
using Render::Look::LookValueSource;

namespace
{
constexpr int Lorencia = 0;
constexpr int Dungeon = 1;
constexpr int Noria = 3;
constexpr int Icarus = 10;
constexpr int BloodCastle2 = 12;
constexpr int BloodCastle3 = 13;

const std::string LookFile = "Data/Render/Look.json";
const std::string MoodFile = "Data/Maps/LightMood.json";

const std::string ShippedLook = R"({
  "formatVersion": 2,
  "default": {
    "lighting.rim.strength": 0.35,
    "bloom.strength": 0.6,
    "mood.base": 0.7,
    "mood.figures": 0.7,
    "mood.sun": 0.6
  },
  "environments": {
    "dim": {"mood.figures": 0.75},
    "fixed": {"mood.sun": 1}
  },
  "maps": [
    {"maps": [1], "name": "Dungeon", "grading.strength": 0.8, "mood.sun": 0.3},
    {"maps": [10], "name": "Icarus", "mood.background": 0.7, "mood.sun": 0.8},
    {"maps": [11, 12, 13], "name": "Blood Castle", "environment": "fixed"}
  ]
}
)";

// The same look as the files of format 1.
const std::string LegacyLook = R"({
  "formatVersion": 1,
  "default": { "lighting.rim.strength": 0.35, "bloom.strength": 0.6 },
  "maps": [ { "world": 1, "name": "Dungeon", "grading.strength": 0.8 } ]
})";

const std::string LegacyMood = R"({
  "formatVersion": 1,
  "default": { "base": 0.7, "figures": 0.7, "dynamicGain": 1.0, "background": 1.0, "sun": 0.6 },
  "maps": [
    { "world": 1, "name": "Dungeon", "sun": 0.3 },
    { "world": 10, "name": "Icarus", "background": 0.7, "sun": 0.8 }
  ]
})";

// Files in memory; any write, copy or rename onto a listed path fails.
class FakeFileSystem final : public Render::Look::LookFileSystem
{
public:
    std::map<std::string, std::string> files;
    std::set<std::string> failWrite;
    std::set<std::string> failCopyTo;
    std::set<std::string> failRenameTo;

    bool Exists(const std::filesystem::path& path) override
    {
        return files.contains(path.generic_string());
    }
    std::optional<std::string> Read(const std::filesystem::path& path) override
    {
        const auto file = files.find(path.generic_string());
        return file == files.end() ? std::nullopt : std::optional<std::string>(file->second);
    }
    bool Write(const std::filesystem::path& path, const std::string& text) override
    {
        if (failWrite.contains(path.generic_string()))
        {
            return false;
        }
        files[path.generic_string()] = text;
        return true;
    }
    bool Copy(const std::filesystem::path& from, const std::filesystem::path& to) override
    {
        if (failCopyTo.contains(to.generic_string()) || !Exists(from))
        {
            return false;
        }
        files[to.generic_string()] = files[from.generic_string()];
        return true;
    }
    bool Rename(const std::filesystem::path& from, const std::filesystem::path& to) override
    {
        if (failRenameTo.contains(to.generic_string()) || !Exists(from))
        {
            return false;
        }
        files[to.generic_string()] = files[from.generic_string()];
        files.erase(from.generic_string());
        return true;
    }
    bool Remove(const std::filesystem::path& path) override
    {
        files.erase(path.generic_string());
        return true;
    }
};

FakeFileSystem Shipped()
{
    FakeFileSystem fileSystem;
    fileSystem.files[LookFile] = ShippedLook;
    return fileSystem;
}

FakeFileSystem Legacy()
{
    FakeFileSystem fileSystem;
    fileSystem.files[LookFile] = LegacyLook;
    fileSystem.files[MoodFile] = LegacyMood;
    return fileSystem;
}

LookFiles Load(FakeFileSystem& fileSystem)
{
    Render::Look::LookFilesLoad loaded = Render::Look::LoadLookFiles(fileSystem, LookFile, MoodFile);
    REQUIRE_FALSE(Data::Items::HasFatal(loaded.issues));
    return loaded.files;
}

LookEditResult Set(LookFiles& files, const std::string& name, double value, std::optional<int> world = std::nullopt)
{
    const LookEdit edit{name, value};
    return Render::Look::SetLookValues(files, std::span<const LookEdit>(&edit, 1), world, world ? "Map" : "");
}

double Effective(const LookFiles& files, const std::string& name, int world)
{
    const std::vector<Render::Look::LookValueState> states = Render::Look::DescribeLookValues(files, world, name);
    REQUIRE(states.size() == 1);
    return states[0].value;
}

LookValueSource SourceOf(const LookFiles& files, const std::string& name, int world)
{
    return Render::Look::DescribeLookValues(files, world, name).at(0).source;
}

Data::Maps::LightMood MoodOn(const LookFiles& files, int world)
{
    return Data::Maps::MoodOf(files.lookData.ForWorld(world));
}

// Same values on every map: the same file text (entries may be in another order in memory).
bool SameFiles(const LookFiles& left, const LookFiles& right)
{
    return left.lookUsable == right.lookUsable && left.saveBlocked == right.saveBlocked &&
           Data::Maps::WriteLookProfileJson(left.lookData) == Data::Maps::WriteLookProfileJson(right.lookData);
}

bool Mentions(const std::vector<Data::Items::ItemDataIssue>& issues, const std::string& text)
{
    return std::any_of(issues.begin(), issues.end(), [&text](const Data::Items::ItemDataIssue& issue)
                       { return issue.ToString().find(text) != std::string::npos; });
}
} // namespace

TEST_CASE("$look set changes the default or the current map's entry [render][look][tuning]")
{
    FakeFileSystem fileSystem = Shipped();
    LookFiles files = Load(fileSystem);

    const LookEditResult defaultEdit = Set(files, "bloom.strength", 0.9);
    REQUIRE_MESSAGE(defaultEdit.applied, defaultEdit.error);
    REQUIRE(defaultEdit.oldValues.size() == 1);
    CHECK(defaultEdit.oldValues[0] == Approx(0.6));
    CHECK_FALSE(defaultEdit.moodChanged);
    CHECK(Effective(files, "bloom.strength", Lorencia) == Approx(0.9));
    CHECK(Effective(files, "bloom.strength", Dungeon) == Approx(0.9));

    // A map value applies on that map only.
    const LookEditResult mapEdit = Set(files, "lighting.rim.strength", 0.5, Lorencia);
    REQUIRE_MESSAGE(mapEdit.applied, mapEdit.error);
    CHECK(mapEdit.entryMaps == std::vector<int>{Lorencia});
    CHECK(Effective(files, "lighting.rim.strength", Lorencia) == Approx(0.5));
    CHECK(SourceOf(files, "lighting.rim.strength", Lorencia) == LookValueSource::Map);
    CHECK(Effective(files, "lighting.rim.strength", Noria) == Approx(0.35));
    CHECK(SourceOf(files, "lighting.rim.strength", Noria) == LookValueSource::Default);
    CHECK(SourceOf(files, "dynamic.wrap", Noria) == LookValueSource::BuiltIn);

    // Mood values are look values: the same edit, and the map's mood changes.
    const LookEditResult moodEdit = Set(files, "mood.sun", 0.9, Noria);
    REQUIRE_MESSAGE(moodEdit.applied, moodEdit.error);
    CHECK(moodEdit.moodChanged);
    CHECK(MoodOn(files, Noria).sun == Approx(0.9));
    CHECK(MoodOn(files, Lorencia).sun == Approx(0.6));
}

TEST_CASE("$look set map changes the entry the map shares with its variants [render][look][tuning]")
{
    FakeFileSystem fileSystem = Shipped();
    LookFiles files = Load(fileSystem);
    const std::size_t entries = files.lookData.maps.size();

    const LookEditResult edit = Set(files, "bloom.strength", 0.4, BloodCastle2);
    REQUIRE_MESSAGE(edit.applied, edit.error);
    CHECK(edit.entryMaps == std::vector<int>{11, 12, 13});
    CHECK(files.lookData.maps.size() == entries);
    CHECK(Effective(files, "bloom.strength", BloodCastle3) == Approx(0.4));
    CHECK(SourceOf(files, "bloom.strength", BloodCastle3) == LookValueSource::Map);
    CHECK(Effective(files, "bloom.strength", Lorencia) == Approx(0.6));
}

TEST_CASE("$look set refuses an unknown name, a value out of range and figures below base [render][look][tuning]")
{
    FakeFileSystem fileSystem = Shipped();
    LookFiles files = Load(fileSystem);
    const LookFiles before = files;

    const LookEditResult unknown = Set(files, "bloom.strenght", 0.9);
    CHECK_FALSE(unknown.applied);
    CHECK_MESSAGE(unknown.error.find("bloom.strenght") != std::string::npos, unknown.error);

    const LookEditResult range = Set(files, "grading.strength", -0.05);
    CHECK_FALSE(range.applied);
    CHECK_MESSAGE(range.error.find("grading.strength") != std::string::npos, range.error);
    CHECK_FALSE(Set(files, "mood.dynamicGain", 3.0).applied);

    // The current map's figures below its effective base: both values are named.
    const LookEditResult rule = Set(files, "mood.figures", 0.5, Noria);
    CHECK_FALSE(rule.applied);
    CHECK_MESSAGE(rule.error.find("mood.figures") != std::string::npos, rule.error);
    CHECK_MESSAGE(rule.error.find("mood.base 0.7") != std::string::npos, rule.error);
    // Raising the default base above the default figures breaks the rule too.
    CHECK_FALSE(Set(files, "mood.base", 0.8).applied);

    // Several edits apply all or none.
    const std::vector<LookEdit> edits{{"bloom.strength", 0.9}, {"dynamic.wrap", 0.9}};
    CHECK_FALSE(Render::Look::SetLookValues(files, edits, std::nullopt).applied);

    CHECK(SameFiles(files, before));
}

TEST_CASE("several edits are checked together, on everything below them [render][look][tuning]")
{
    FakeFileSystem fileSystem = Shipped();
    LookFiles files = Load(fileSystem);

    // Each alone breaks the rule; together they keep it.
    const std::vector<LookEdit> together{{"mood.base", 0.75}, {"mood.figures", 0.75}};
    const LookEditResult raised = Render::Look::SetLookValues(files, together, std::nullopt);
    REQUIRE_MESSAGE(raised.applied, raised.error);
    CHECK(MoodOn(files, Lorencia).base == Approx(0.75));

    // The default stays valid, but the "dim" environment overrides only figures (0.75), now below base.
    const LookFiles before = files;
    const std::vector<LookEdit> higher{{"mood.base", 0.8}, {"mood.figures", 0.8}};
    const LookEditResult refused = Render::Look::SetLookValues(files, higher, std::nullopt);
    CHECK_FALSE(refused.applied);
    CHECK_MESSAGE(refused.error.find("environments.dim") != std::string::npos, refused.error);
    CHECK(SameFiles(files, before));
}

TEST_CASE("$look reset goes back to the loaded values [render][look][tuning]")
{
    FakeFileSystem fileSystem = Shipped();
    const LookFiles loaded = Load(fileSystem);
    LookFiles files = loaded;

    REQUIRE(Set(files, "bloom.strength", 0.9).applied);
    REQUIRE(Set(files, "bloom.strength", 1.2, Lorencia).applied);
    REQUIRE(Set(files, "dynamic.wrap", 0.2).applied);
    REQUIRE(Set(files, "grading.strength", 0.5, Dungeon).applied);

    // One name: in the default and the current map's entry; a new, now empty entry goes.
    REQUIRE(Render::Look::ResetLookValues(files, loaded, std::string_view("bloom.strength"), Lorencia).applied);
    CHECK(Effective(files, "bloom.strength", Lorencia) == Approx(0.6));
    CHECK(Effective(files, "dynamic.wrap", Lorencia) == Approx(0.2));
    CHECK(files.lookData.maps.size() == loaded.lookData.maps.size());
    // A loaded value comes back as it was; one the loaded entry did not name goes again.
    REQUIRE(Render::Look::ResetLookValues(files, loaded, std::string_view("grading.strength"), Dungeon).applied);
    CHECK(Effective(files, "grading.strength", Dungeon) == Approx(0.8));
    REQUIRE(Set(files, "bloom.strength", 0.5, Dungeon).applied);
    REQUIRE(Render::Look::ResetLookValues(files, loaded, std::string_view("bloom.strength"), Dungeon).applied);
    CHECK_FALSE(files.lookData.FindMap(Dungeon)->overrides[LookValue::BloomStrength].has_value());

    REQUIRE(Render::Look::ResetLookValues(files, loaded, std::nullopt, Lorencia).applied);
    CHECK(SameFiles(files, loaded));
    CHECK_FALSE(Render::Look::ResetLookValues(files, loaded, std::string_view("no.such"), Lorencia).applied);
}

TEST_CASE("a format 1 look and its light mood load as one profile [render][look][tuning]")
{
    FakeFileSystem legacy = Legacy();
    FakeFileSystem shipped = Shipped();
    const LookFiles upgraded = Load(legacy);
    const LookFiles current = Load(shipped);
    CHECK_FALSE(upgraded.saveBlocked);

    // Every map the same, look and mood (the shipped fixture adds only "dim", "fixed" and Blood Castle).
    for (const int world : {Lorencia, Dungeon, Noria, Icarus, 200})
    {
        CAPTURE(world);
        CHECK(upgraded.lookData.ForWorld(world) == current.lookData.ForWorld(world));
    }

    // Saving writes format 2 into Look.json only.
    const Render::Look::LookSaveResult saved = Render::Look::SaveLookFiles(legacy, upgraded, LookFile);
    REQUIRE_MESSAGE(saved.saved, saved.message);
    CHECK(legacy.files[MoodFile] == LegacyMood);
    CHECK(legacy.files[LookFile].find("\"formatVersion\": 2") != std::string::npos);
    // Read again, the old mood file is no longer read but reported.
    const Render::Look::LookFilesLoad again = Render::Look::LoadLookFiles(legacy, LookFile, MoodFile);
    CHECK(again.files.lookData == upgraded.lookData);
    CHECK_FALSE(Data::Items::HasFatal(again.issues));
    CHECK(Mentions(again.issues, "no longer read"));
}

TEST_CASE("a format 1 look without a usable light mood keeps Classic light and cannot be saved [render][look][tuning]")
{
    for (const bool missing : {true, false})
    {
        CAPTURE(missing);
        FakeFileSystem fileSystem = Legacy();
        if (missing)
        {
            fileSystem.files.erase(MoodFile);
        }
        else
        {
            fileSystem.files[MoodFile] = R"({ "formatVersion": 1, "default": { "base": 0.7 } })";
        }
        const Render::Look::LookFilesLoad loaded = Render::Look::LoadLookFiles(fileSystem, LookFile, MoodFile);
        LookFiles files = loaded.files;
        CHECK(Data::Items::HasFatal(loaded.issues));
        CHECK(files.lookUsable);
        REQUIRE(files.saveBlocked.has_value());
        CHECK_MESSAGE(files.saveBlocked->find(MoodFile) != std::string::npos, *files.saveBlocked);
        // The look values apply, the mood is Classic.
        CHECK(Effective(files, "bloom.strength", Lorencia) == Approx(0.6));
        CHECK(MoodOn(files, Dungeon) == Data::Maps::LightMood{});
        // Live edits still work; the save is refused and changes nothing.
        REQUIRE(Set(files, "bloom.strength", 0.9).applied);
        const std::map<std::string, std::string> before = fileSystem.files;
        const Render::Look::LookSaveResult result = Render::Look::SaveLookFiles(fileSystem, files, LookFile);
        CHECK_FALSE(result.saved);
        CHECK_MESSAGE(result.message.find(MoodFile) != std::string::npos, result.message);
        CHECK(fileSystem.files == before);
    }
}

TEST_CASE("a candidate becomes the live values, with the shipped mood when it has none [render][look][tuning]")
{
    FakeFileSystem fileSystem = Shipped();
    fileSystem.files["Candidates/a/Look.json"] = R"({ "formatVersion": 1, "default": { "bloom.strength": 0.9 } })";
    fileSystem.files["Candidates/a/LightMood.json"] =
        R"({ "formatVersion": 1, "default": { "base": 0.5, "figures": 0.6, "dynamicGain": 1.5, "background": 1, "sun": 0.8 } })";
    fileSystem.files["Candidates/b/Look.json"] = R"({ "formatVersion": 1, "default": { "bloom.strength": 1.1 } })";
    fileSystem.files["Candidates/c/Look.json"] =
        R"({ "formatVersion": 2, "default": { "mood.base": 0.4, "mood.figures": 0.5 } })";
    fileSystem.files["Candidates/c/LightMood.json"] = LegacyMood;
    fileSystem.files["Candidates/bad/Look.json"] = R"({ "formatVersion": 1, "default": { "bloom.strength": 5 } })";
    fileSystem.files["Candidates/badmood/Look.json"] = R"({ "formatVersion": 1 })";
    // Format 1 with a mood value of its own, which would override the shipped moods it shows.
    fileSystem.files["Candidates/v1mood/Look.json"] =
        R"({ "formatVersion": 1, "environments": { "dim": { "mood.sun": 0.1 } } })";
    fileSystem.files["Candidates/badmood/LightMood.json"] = R"({ "formatVersion": 1, "default": { "base": 0.7 } })";

    std::vector<Data::Items::ItemDataIssue> issues;
    const auto candidate = [&fileSystem, &issues](const std::string& name)
    {
        issues.clear();
        return Render::Look::LoadCandidateFiles(fileSystem, "Candidates/" + name + "/Look.json",
                                                "Candidates/" + name + "/LightMood.json", LookFile, MoodFile, issues);
    };

    const std::optional<LookFiles> a = candidate("a");
    REQUIRE(a.has_value());
    CHECK(a->lookData.ForWorld(Lorencia)[LookValue::BloomStrength] == Approx(0.9));
    CHECK(MoodOn(*a, Lorencia).base == Approx(0.5));

    // B after A equals B loaded directly: B has no mood, so the shipped profile's applies, not A's.
    const std::optional<LookFiles> b = candidate("b");
    REQUIRE(b.has_value());
    CHECK(MoodOn(*b, Lorencia).base == Approx(0.7));
    CHECK(MoodOn(*b, Dungeon).sun == Approx(0.3));
    CHECK(MoodOn(*b, BloodCastle2).sun == Approx(1.0));
    CHECK(SameFiles(*b, *candidate("b")));
    // Only to look at: it cannot be saved, and a default mood edit still reaches the maps it copied.
    REQUIRE(b->saveBlocked.has_value());
    LookFiles edited = *b;
    REQUIRE(Set(edited, "mood.base", 0.6).applied);
    CHECK(MoodOn(edited, BloodCastle2).base == Approx(0.6));
    CHECK_FALSE(Render::Look::SaveLookFiles(fileSystem, edited, LookFile).saved);
    CHECK_FALSE(a->saveBlocked.has_value());

    // A format 2 candidate has its mood; a LightMood.json beside it is not read.
    const std::optional<LookFiles> c = candidate("c");
    REQUIRE(c.has_value());
    CHECK(MoodOn(*c, Dungeon).base == Approx(0.4));
    CHECK(Mentions(issues, "no longer read"));

    // An invalid look file, an invalid mood file or a missing candidate gives nothing to apply.
    CHECK_FALSE(candidate("bad").has_value());
    CHECK_FALSE(candidate("badmood").has_value());
    CHECK_FALSE(candidate("v1mood").has_value());
    CHECK(Mentions(issues, "is a format 2 value"));
    CHECK_FALSE(candidate("missing").has_value());

    // Back to the shipped file.
    CHECK(Load(fileSystem).lookData.ForWorld(Lorencia)[LookValue::BloomStrength] == Approx(0.6));
}

TEST_CASE("$look save writes the one file in its fixed layout [render][look][tuning]")
{
    FakeFileSystem fileSystem = Shipped();
    LookFiles files = Load(fileSystem);
    REQUIRE(Set(files, "bloom.strength", 0.9).applied);
    REQUIRE(Set(files, "dynamic.wrap", 0.3, Lorencia).applied);
    REQUIRE(Set(files, "mood.sun", 0.4, Dungeon).applied);

    const Render::Look::LookSaveResult saved = Render::Look::SaveLookFiles(fileSystem, files, LookFile);
    REQUIRE_MESSAGE(saved.saved, saved.message);
    CHECK(fileSystem.files.size() == 1); // no temporaries, backups or mood file

    const std::string look = fileSystem.files[LookFile];
    CHECK_MESSAGE(look.find("    \"bloom.strength\": 0.9,\n") != std::string::npos, look);
    CHECK_MESSAGE(look.find(R"({"maps": [0], "name": "Map", "dynamic.wrap": 0.3})") != std::string::npos, look);
    CHECK_MESSAGE(look.find(R"({"maps": [1], "name": "Dungeon", "grading.strength": 0.8, "mood.sun": 0.4})") !=
                      std::string::npos,
                  look);
    CHECK(look.find("\"maps\": [0]") < look.find("\"maps\": [1]"));

    // Loading the saved file gives the same values, and saving it again the same text.
    const LookFiles reloaded = Load(fileSystem);
    CHECK(SameFiles(reloaded, files));
    REQUIRE(Render::Look::SaveLookFiles(fileSystem, reloaded, LookFile).saved);
    CHECK(fileSystem.files[LookFile] == look);
}

TEST_CASE("a failed save leaves the file as it was [render][look][tuning]")
{
    FakeFileSystem fileSystem = Shipped();
    LookFiles files = Load(fileSystem);
    REQUIRE(Set(files, "bloom.strength", 0.9).applied);
    const std::map<std::string, std::string> before = fileSystem.files;

    SUBCASE("the temporary cannot be written")
    {
        fileSystem.failWrite.insert(LookFile + ".tmp");
        const Render::Look::LookSaveResult result = Render::Look::SaveLookFiles(fileSystem, files, LookFile);
        CHECK_FALSE(result.saved);
        CHECK_MESSAGE(result.message.find("Look.json.tmp") != std::string::npos, result.message);
    }
    SUBCASE("the original cannot be backed up")
    {
        fileSystem.failCopyTo.insert(LookFile + ".bak");
        CHECK_FALSE(Render::Look::SaveLookFiles(fileSystem, files, LookFile).saved);
    }
    SUBCASE("the file cannot be replaced")
    {
        fileSystem.failRenameTo.insert(LookFile);
        const Render::Look::LookSaveResult result = Render::Look::SaveLookFiles(fileSystem, files, LookFile);
        CHECK_FALSE(result.saved);
        CHECK_MESSAGE(result.message.find("nothing was saved") != std::string::npos, result.message);
    }
    CHECK(fileSystem.files == before);
    // The live values are not touched by a save.
    CHECK(Effective(files, "bloom.strength", Lorencia) == Approx(0.9));
}

TEST_CASE(
    "a missing look profile is edited from the built-in values, an invalid one is not edited [render][look][tuning]")
{
    FakeFileSystem fileSystem;
    LookFiles files = Render::Look::LoadLookFiles(fileSystem, LookFile, MoodFile).files;
    CHECK(files.lookUsable);
    REQUIRE(Set(files, "bloom.strength", 0.9).applied);

    fileSystem.files[LookFile] = R"({ "formatVersion": 2, "default": { "bloom.strength": 9 } })";
    LookFiles invalid = Render::Look::LoadLookFiles(fileSystem, LookFile, MoodFile).files;
    CHECK_FALSE(invalid.lookUsable);
    CHECK(Effective(invalid, "bloom.strength", Lorencia) == Approx(0.6));
    CHECK_FALSE(Set(invalid, "bloom.strength", 0.9).applied);
    CHECK_FALSE(Set(invalid, "mood.sun", 0.5).applied);
    // Saving leaves the invalid file alone.
    CHECK_FALSE(Render::Look::SaveLookFiles(fileSystem, invalid, LookFile).saved);
    CHECK(fileSystem.files[LookFile] == R"({ "formatVersion": 2, "default": { "bloom.strength": 9 } })");
}

#ifndef _WIN32
TEST_CASE("a look profile whose existence cannot be checked is unusable, not missing [render][look][tuning]")
{
    // A file name longer than any file system allows: the existence check itself fails (ENAMETOOLONG).
    const std::filesystem::path lookFile = std::filesystem::temp_directory_path() / (std::string(300, 'x') + ".json");
    Render::Look::DiskLookFileSystem disk;
    CHECK(disk.Exists(lookFile));
    CHECK_FALSE(disk.Exists(std::filesystem::temp_directory_path() / "mu_no_such_look_profile.json"));

    const Render::Look::LookFilesLoad loaded =
        Render::Look::LoadLookFiles(disk, lookFile, std::filesystem::temp_directory_path() / "mu_no_such_mood.json");
    CHECK_FALSE(loaded.files.lookUsable);
    bool readError = false;
    for (const auto& issue : loaded.issues)
    {
        readError = readError || (issue.ToString().find(lookFile.generic_string()) != std::string::npos &&
                                  issue.ToString().find("could not be read") != std::string::npos);
    }
    CHECK(readError);
}
#endif

TEST_CASE("$look commands parse their verb, scope, name and value [render][look][tuning]")
{
    using Render::Look::LookVerb;
    using Render::Look::ParseLookCommand;

    const auto show = ParseLookCommand(L"$look show");
    REQUIRE(show.has_value());
    CHECK(show->verb == LookVerb::Show);
    CHECK(show->name.empty());
    CHECK(ParseLookCommand(L"$look show bloom.strength")->name == "bloom.strength");

    const auto set = ParseLookCommand(L"$look set bloom.strength 0.9");
    REQUIRE(set.has_value());
    CHECK(set->verb == LookVerb::Set);
    CHECK_FALSE(set->map);
    CHECK(set->value == Approx(0.9));
    const auto setMap = ParseLookCommand(L"$look set map mood.sun 0.4");
    REQUIRE(setMap.has_value());
    CHECK(setMap->map);
    CHECK(setMap->name == "mood.sun");

    CHECK(ParseLookCommand(L"$look reset")->verb == LookVerb::Reset);
    CHECK(ParseLookCommand(L"$look reset dynamic.wrap")->name == "dynamic.wrap");
    CHECK(ParseLookCommand(L"$look load")->name.empty());
    CHECK(ParseLookCommand(L"$look load chosen-b")->name == "chosen-b");
    CHECK(ParseLookCommand(L"$look save")->verb == LookVerb::Save);

    // Not these commands: the preset, a bad value or candidate, extra words.
    CHECK_FALSE(ParseLookCommand(L"$look classic").has_value());
    CHECK_FALSE(ParseLookCommand(L"$look enhanced").has_value());
    CHECK_FALSE(ParseLookCommand(L"$look set bloom.strength high").has_value());
    CHECK_FALSE(ParseLookCommand(L"$look set map bloom.strength").has_value());
    CHECK_FALSE(ParseLookCommand(L"$look load ../x").has_value());
    CHECK_FALSE(ParseLookCommand(L"$look load ..").has_value());
    CHECK_FALSE(ParseLookCommand(L"$look save now").has_value());
    CHECK_FALSE(ParseLookCommand(L"$looks show").has_value());

    // The aliases set the same names.
    const auto knee = Render::Look::ParseLookTuningCommand(L"$lightmood knee 0.9");
    REQUIRE(knee.has_value());
    CHECK(knee->assignments[0].value == LookValue::HighlightKnee);
}

TEST_CASE("a value an environment sets names the environment as its source [render][look][tuning]")
{
    FakeFileSystem fileSystem;
    fileSystem.files[LookFile] = R"({
  "formatVersion": 2,
  "default": { "bloom.strength": 1.0, "ao.strength": 0.5 },
  "environments": { "enclosed": { "bloom.strength": 0.4 } },
  "maps": [ { "maps": [1], "name": "Dungeon", "environment": "enclosed", "ao.strength": 0.2 } ]
})";
    LookFiles files = Load(fileSystem);
    constexpr int Open = 0;
    CHECK(SourceOf(files, "bloom.strength", Dungeon) == LookValueSource::Environment);
    CHECK(Effective(files, "bloom.strength", Dungeon) == Approx(0.4));
    CHECK(SourceOf(files, "ao.strength", Dungeon) == LookValueSource::Map);
    CHECK(SourceOf(files, "bloom.strength", Open) == LookValueSource::Default);
}
