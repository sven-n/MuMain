#include "stdafx.h"

#include "doctest.h"

#include "Render/Look/LookFileSystem.h"
#include "Render/Look/LookFiles.h"
#include "Render/Look/LookTuningCommand.h"

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

const std::string LookFile = "Data/Render/Look.json";
const std::string MoodFile = "Data/Maps/LightMood.json";

const std::string ShippedLook = R"({
  "formatVersion": 1,
  "default": { "lighting.rim.strength": 0.35, "bloom.strength": 0.6 },
  "maps": [ { "world": 1, "name": "Dungeon", "grading.strength": 0.8 } ]
})";

const std::string ShippedMood = R"({
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
    fileSystem.files[MoodFile] = ShippedMood;
    return fileSystem;
}

LookFiles Load(FakeFileSystem& fileSystem)
{
    Render::Look::LookFilesLoad loaded = Render::Look::LoadLookFiles(fileSystem, LookFile, MoodFile);
    REQUIRE_FALSE(Data::Items::HasErrors(loaded.issues));
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

// Same documents, so the same live values.
bool SameFiles(const LookFiles& left, const LookFiles& right)
{
    return left.look.dump() == right.look.dump() && left.mood.dump() == right.mood.dump() &&
           left.lookData == right.lookData && left.moodData.defaultMood == right.moodData.defaultMood &&
           left.moodData.maps == right.moodData.maps;
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
    CHECK(Effective(files, "bloom.strength", Lorencia) == Approx(0.9));
    CHECK(Effective(files, "bloom.strength", Dungeon) == Approx(0.9));

    // A map value applies on that map only.
    const LookEditResult mapEdit = Set(files, "lighting.rim.strength", 0.5, Lorencia);
    REQUIRE_MESSAGE(mapEdit.applied, mapEdit.error);
    CHECK(Effective(files, "lighting.rim.strength", Lorencia) == Approx(0.5));
    CHECK(SourceOf(files, "lighting.rim.strength", Lorencia) == LookValueSource::Map);
    CHECK(Effective(files, "lighting.rim.strength", Noria) == Approx(0.35));
    CHECK(SourceOf(files, "lighting.rim.strength", Noria) == LookValueSource::Default);
    CHECK(SourceOf(files, "dynamic.wrap", Noria) == LookValueSource::BuiltIn);

    // Mood values edit LightMood.json the same way.
    const LookEditResult moodEdit = Set(files, "mood.sun", 0.9, Noria);
    REQUIRE_MESSAGE(moodEdit.applied, moodEdit.error);
    CHECK(moodEdit.moodChanged);
    CHECK(files.moodData.ForWorld(Noria).sun == Approx(0.9));
    CHECK(files.moodData.ForWorld(Lorencia).sun == Approx(0.6));
}

TEST_CASE("$look set refuses an unknown name, a value out of range and figures below base [render][look][tuning]")
{
    FakeFileSystem fileSystem = Shipped();
    LookFiles files = Load(fileSystem);
    const LookFiles before = files;

    const LookEditResult unknown = Set(files, "bloom.strenght", 0.9);
    CHECK_FALSE(unknown.applied);
    CHECK_MESSAGE(unknown.error.find("bloom.strenght") != std::string::npos, unknown.error);

    const LookEditResult range = Set(files, "grading.strength", 0.05);
    CHECK_FALSE(range.applied);
    CHECK_MESSAGE(range.error.find("grading.strength") != std::string::npos, range.error);
    CHECK_FALSE(Set(files, "mood.dynamicGain", 3.0).applied);

    // The current map's figures below its effective base: both values are named.
    const LookEditResult rule = Set(files, "mood.figures", 0.5, Noria);
    CHECK_FALSE(rule.applied);
    CHECK_MESSAGE(rule.error.find("figures") != std::string::npos, rule.error);
    CHECK_MESSAGE(rule.error.find("not below base 0.7") != std::string::npos, rule.error);
    // Raising the default base above the default figures breaks the rule too.
    CHECK_FALSE(Set(files, "mood.base", 0.8).applied);

    // Several edits apply all or none.
    const std::vector<LookEdit> edits{{"bloom.strength", 0.9}, {"dynamic.wrap", 0.9}};
    CHECK_FALSE(Render::Look::SetLookValues(files, edits, std::nullopt).applied);

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

    // One name: in the default and the current map's entry; a new, now empty entry goes.
    REQUIRE(Render::Look::ResetLookValues(files, loaded, std::string_view("bloom.strength"), Lorencia).applied);
    CHECK(Effective(files, "bloom.strength", Lorencia) == Approx(0.6));
    CHECK(Effective(files, "dynamic.wrap", Lorencia) == Approx(0.2));
    CHECK(files.look["maps"].size() == 1);

    REQUIRE(Render::Look::ResetLookValues(files, loaded, std::nullopt, Lorencia).applied);
    CHECK(SameFiles(files, loaded));
    CHECK_FALSE(Render::Look::ResetLookValues(files, loaded, std::string_view("no.such"), Lorencia).applied);
}

TEST_CASE("a candidate becomes the live values, with the shipped mood when it has none [render][look][tuning]")
{
    FakeFileSystem fileSystem = Shipped();
    fileSystem.files["Candidates/a/Look.json"] = R"({ "formatVersion": 1, "default": { "bloom.strength": 0.9 } })";
    fileSystem.files["Candidates/a/LightMood.json"] =
        R"({ "formatVersion": 1, "default": { "base": 0.5, "figures": 0.6, "dynamicGain": 1.5, "background": 1, "sun": 0.8 } })";
    fileSystem.files["Candidates/b/Look.json"] = R"({ "formatVersion": 1, "default": { "bloom.strength": 1.1 } })";
    fileSystem.files["Candidates/bad/Look.json"] = R"({ "formatVersion": 1, "default": { "bloom.strength": 5 } })";
    fileSystem.files["Candidates/badmood/Look.json"] = R"({ "formatVersion": 1 })";
    fileSystem.files["Candidates/badmood/LightMood.json"] = R"({ "formatVersion": 1, "default": { "base": 0.7 } })";

    const auto candidate = [&fileSystem](const std::string& name)
    {
        std::vector<Data::Items::ItemDataIssue> issues;
        return Render::Look::LoadCandidateFiles(fileSystem, "Candidates/" + name + "/Look.json",
                                                "Candidates/" + name + "/LightMood.json", MoodFile, issues);
    };

    const std::optional<LookFiles> a = candidate("a");
    REQUIRE(a.has_value());
    CHECK(a->lookData.ForWorld(Lorencia)[LookValue::BloomStrength] == Approx(0.9));
    CHECK(a->moodData.ForWorld(Lorencia).base == Approx(0.5));

    // B after A equals B loaded directly: B has no mood, so the shipped one applies, not A's.
    const std::optional<LookFiles> b = candidate("b");
    REQUIRE(b.has_value());
    CHECK(b->moodData.ForWorld(Lorencia).base == Approx(0.7));
    CHECK(b->moodData.ForWorld(Dungeon).sun == Approx(0.3));
    CHECK(SameFiles(*b, *candidate("b")));

    // An invalid look file, an invalid mood file or a missing candidate gives nothing to apply.
    CHECK_FALSE(candidate("bad").has_value());
    CHECK_FALSE(candidate("badmood").has_value());
    CHECK_FALSE(candidate("missing").has_value());

    // Back to the shipped files.
    CHECK(Load(fileSystem).lookData.ForWorld(Lorencia)[LookValue::BloomStrength] == Approx(0.6));
}

TEST_CASE("$look save keeps only a map entry's overrides and leaves other entries as they were [render][look][tuning]")
{
    FakeFileSystem fileSystem = Shipped();
    LookFiles files = Load(fileSystem);
    REQUIRE(Set(files, "bloom.strength", 0.9).applied);
    REQUIRE(Set(files, "dynamic.wrap", 0.3, Lorencia).applied);
    REQUIRE(Set(files, "mood.sun", 0.4, Dungeon).applied);

    const Render::Look::LookSaveResult saved = Render::Look::SaveLookFiles(fileSystem, files, LookFile, MoodFile);
    REQUIRE_MESSAGE(saved.saved, saved.message);
    CHECK(fileSystem.files.size() == 2); // no temporaries or backups left

    const std::string look = fileSystem.files[LookFile];
    CHECK_MESSAGE(look.find(R"("bloom.strength": 0.9)") != std::string::npos, look);
    CHECK_MESSAGE(look.find(R"({ "world": 1, "name": "Dungeon", "grading.strength": 0.8 })") != std::string::npos,
                  look);
    CHECK_MESSAGE(look.find(R"({ "world": 0, "name": "Map", "dynamic.wrap": 0.3 })") != std::string::npos, look);

    const std::string mood = fileSystem.files[MoodFile];
    CHECK_MESSAGE(mood.find(R"({ "world": 1, "name": "Dungeon", "sun": 0.4 })") != std::string::npos, mood);
    CHECK_MESSAGE(mood.find(R"({ "world": 10, "name": "Icarus", "background": 0.7, "sun": 0.8 })") != std::string::npos,
                  mood);
    CHECK_MESSAGE(mood.find(R"("dynamicGain": 1.0)") != std::string::npos, mood);
    CHECK(mood.find("Dungeon") < mood.find("Icarus"));

    // Loading the saved files gives the same values.
    CHECK(SameFiles(Load(fileSystem), files));
}

TEST_CASE("a failed save leaves both files as they were [render][look][tuning]")
{
    FakeFileSystem fileSystem = Shipped();
    LookFiles files = Load(fileSystem);
    REQUIRE(Set(files, "bloom.strength", 0.9).applied);
    REQUIRE(Set(files, "mood.sun", 0.4).applied);
    const std::map<std::string, std::string> before = fileSystem.files;

    SUBCASE("a temporary cannot be written")
    {
        fileSystem.failWrite.insert(MoodFile + ".tmp");
        const Render::Look::LookSaveResult result = Render::Look::SaveLookFiles(fileSystem, files, LookFile, MoodFile);
        CHECK_FALSE(result.saved);
        CHECK_MESSAGE(result.message.find("LightMood.json.tmp") != std::string::npos, result.message);
        CHECK(fileSystem.files == before);
    }
    SUBCASE("the second file cannot be replaced: the first is restored")
    {
        fileSystem.failRenameTo.insert(MoodFile);
        const Render::Look::LookSaveResult result = Render::Look::SaveLookFiles(fileSystem, files, LookFile, MoodFile);
        CHECK_FALSE(result.saved);
        CHECK_FALSE(result.backupsKept);
        CHECK_MESSAGE(result.message.find("restored") != std::string::npos, result.message);
        CHECK(fileSystem.files == before);
    }
    SUBCASE("the restore fails too: the backups are kept and named")
    {
        fileSystem.failRenameTo.insert(MoodFile);
        fileSystem.failCopyTo.insert(LookFile);
        const Render::Look::LookSaveResult result = Render::Look::SaveLookFiles(fileSystem, files, LookFile, MoodFile);
        CHECK_FALSE(result.saved);
        CHECK(result.backupsKept);
        CHECK(fileSystem.files[LookFile + ".bak"] == ShippedLook);
        CHECK(fileSystem.files[MoodFile + ".bak"] == ShippedMood);
        CHECK(fileSystem.files[MoodFile] == ShippedMood);
        for (const std::string& name : {LookFile, MoodFile, LookFile + ".bak", MoodFile + ".bak"})
        {
            CHECK_MESSAGE(result.message.find(name) != std::string::npos, result.message);
        }
    }
    // The live values are not touched by a save.
    CHECK(Effective(files, "bloom.strength", Lorencia) == Approx(0.9));
}

TEST_CASE(
    "a missing look profile is edited from the built-in values; an invalid one is not edited [render][look][tuning]")
{
    FakeFileSystem fileSystem;
    fileSystem.files[MoodFile] = ShippedMood;
    LookFiles files = Render::Look::LoadLookFiles(fileSystem, LookFile, MoodFile).files;
    CHECK(files.lookUsable);
    REQUIRE(Set(files, "bloom.strength", 0.9).applied);

    fileSystem.files[LookFile] = R"({ "formatVersion": 1, "default": { "bloom.strength": 9 } })";
    LookFiles invalid = Render::Look::LoadLookFiles(fileSystem, LookFile, MoodFile).files;
    CHECK_FALSE(invalid.lookUsable);
    CHECK(Effective(invalid, "bloom.strength", Lorencia) == Approx(0.6));
    CHECK_FALSE(Set(invalid, "bloom.strength", 0.9).applied);
    // Its mood is still editable, and saving leaves the invalid look file alone.
    REQUIRE(Set(invalid, "mood.sun", 0.5).applied);
    REQUIRE(Render::Look::SaveLookFiles(fileSystem, invalid, LookFile, MoodFile).saved);
    CHECK(fileSystem.files[LookFile] == R"({ "formatVersion": 1, "default": { "bloom.strength": 9 } })");
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
