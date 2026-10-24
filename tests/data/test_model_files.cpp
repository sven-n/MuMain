#include "stdafx.h"

#include "doctest.h"

#include "Core/Globals/_enum.h"
#include "Data/DataHandler/ModelData/ModelFileDataHandler.h"
#include "Data/GameData/ModelData/ModelFileSymbols.h"
#include "Data/GameData/ModelData/ModelFiles.h"

#include <algorithm>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <random>
#include <regex>
#include <string>
#include <utility>
#include <vector>

using Data::Items::HasErrors;
using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using Data::Models::ActionClip;
using Data::Models::DataDirectories;
using Data::Models::ModelFileEntry;
using Data::Models::ModelFiles;

namespace
{
struct ReadResult
{
    std::vector<ModelFileEntry> models;
    std::vector<ItemDataIssue> issues;
};

ReadResult Read(const std::string& text)
{
    ReadResult result;
    Data::Models::ReadModelFilesJson(text, "ModelFiles.json", result.models, result.issues);
    return result;
}

std::string Messages(const std::vector<ItemDataIssue>& issues)
{
    std::string text;
    for (const ItemDataIssue& issue : issues)
    {
        text += issue.ToString() + "\n";
    }
    return text;
}

bool HasIssue(const std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const std::string& field,
              const std::string& message)
{
    return std::any_of(issues.begin(), issues.end(), [&](const ItemDataIssue& issue) {
        return issue.severity == severity && issue.field == field && issue.message.find(message) != std::string::npos;
    });
}

std::string Table(const std::string& models)
{
    return R"({ "formatVersion": 1, "models": [)" + models + "] }";
}

// A fresh directory under the system's temporary directory, removed afterwards.
class TemporaryDirectory
{
public:
    TemporaryDirectory()
    {
        static std::atomic<int> counter{0};
        path_ = std::filesystem::temp_directory_path() /
                ("model_files_test_" + std::to_string(std::random_device{}()) + "_" + std::to_string(counter++));
        std::filesystem::remove_all(path_);
        std::filesystem::create_directories(path_);
    }
    ~TemporaryDirectory()
    {
        std::error_code error;
        std::filesystem::remove_all(path_, error);
    }
    TemporaryDirectory(const TemporaryDirectory&) = delete;
    TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;

    const std::filesystem::path& Path() const { return path_; }

    void Write(const std::filesystem::path& relative, const std::string& text) const
    {
        const std::filesystem::path file = path_ / relative;
        std::filesystem::create_directories(file.parent_path());
        std::ofstream(file, std::ios::binary) << text;
    }

private:
    std::filesystem::path path_;
};
} // namespace

TEST_CASE("ModelFileSymbols name every monster model once, sorted by number")
{
    const auto symbols = Data::Models::GetModelFileSymbols();
    REQUIRE(!symbols.empty());
    for (size_t i = 1; i < symbols.size(); ++i)
    {
        CHECK(symbols[i - 1].type < symbols[i].type);
    }
    for (const auto& symbol : symbols)
    {
        CHECK(symbol.type >= MODEL_MONSTER01);
        CHECK(symbol.type < MODEL_MONSTER_END);
    }
    const auto* yeti = Data::Models::FindModelFileSymbol("MODEL_ELITE_YETI");
    REQUIRE(yeti != nullptr);
    CHECK(yeti->type == MODEL_ELITE_YETI);
    CHECK(Data::Models::FindModelFileSymbol("MODEL_PLAYER") == nullptr);
}

// Core/Globals/_enum.h's MODEL_MONSTER01 + N block, which ModelFileSymbols
// mirrors by hand: every "MODEL_X = MODEL_MONSTER01 + N," line.
static std::vector<std::pair<std::string, int>> ReadMonsterModelEnum()
{
    std::ifstream in(MU_TEST_ENUM_HEADER);
    REQUIRE(in.good());
    static const std::regex entry(R"(^\s*(MODEL_\w+)\s*=\s*MODEL_MONSTER01\s*\+\s*(\d+)\s*,)");
    std::vector<std::pair<std::string, int>> models;
    std::string line;
    while (std::getline(in, line))
    {
        std::smatch match;
        if (std::regex_search(line, match, entry))
        {
            models.emplace_back(match[1].str(), MODEL_MONSTER01 + std::stoi(match[2].str()));
        }
    }
    return models;
}

TEST_CASE("ModelFileSymbols mirror every MODEL_MONSTER01 + N name of _enum.h")
{
    const auto models = ReadMonsterModelEnum();
    const auto symbols = Data::Models::GetModelFileSymbols();
    REQUIRE(!models.empty());
    CHECK(models.size() == symbols.size());
    for (const auto& [name, type] : models)
    {
        CAPTURE(name);
        const auto* symbol = Data::Models::FindModelFileSymbol(name);
        REQUIRE(symbol != nullptr);
        CHECK(symbol->type == type);
    }
    for (const auto& symbol : symbols)
    {
        CAPTURE(symbol.code);
        const bool inEnum = std::any_of(models.begin(), models.end(), [&](const auto& model) {
            return model.first == symbol.code && model.second == symbol.type;
        });
        CHECK(inEnum);
    }
}

TEST_CASE("ReadModelFilesJson reads models, files and the action table")
{
    const ReadResult result = Read(Table(R"(
        { "model": "MODEL_ELITE_YETI", "file": "Monster/Monster14.glb",
          "actions": { "2": { "clip": "walk", "crossfade": 3 }, "6": { "clip": "die" }, "0": { "crossfade": 0.5 } } },
        { "model": "MODEL_HOUND", "file": "Monster/Monster02.glb" })"));
    INFO(Messages(result.issues));
    CHECK(result.issues.empty());
    REQUIRE(result.models.size() == 2);
    const ModelFileEntry& yeti = result.models[0];
    CHECK(yeti.model == "MODEL_ELITE_YETI");
    CHECK(yeti.type == MODEL_ELITE_YETI);
    CHECK(yeti.file == "Monster/Monster14.glb");
    REQUIRE(yeti.actions.size() == 3);
    CHECK(yeti.actions.at(2) == ActionClip{"walk", 3.f});
    CHECK(yeti.actions.at(6) == ActionClip{"die", std::nullopt});
    CHECK(yeti.actions.at(0) == ActionClip{"", 0.5f});
    CHECK(result.models[1].type == MODEL_HOUND);
    CHECK(result.models[1].actions.empty());
}

TEST_CASE("ReadModelFilesJson reads the LOD settings, with defaults for three levels")
{
    const ReadResult result = Read(Table(R"(
        { "model": "MODEL_ELITE_YETI", "file": "Monster/Monster14.glb",
          "lod": { "screenHeights": [0.3, 0.12], "hysteresis": 0.25 } },
        { "model": "MODEL_HOUND", "file": "Monster/Monster02.glb", "lod": { "screenHeights": [0.4] } },
        { "model": "MODEL_YETI", "file": "Monster/Monster13.glb" })"));
    INFO(Messages(result.issues));
    CHECK(result.issues.empty());
    REQUIRE(result.models.size() == 3);
    CHECK(result.models[0].lod == Data::Models::LodSettings{{0.3f, 0.12f}, 0.25f});
    CHECK(result.models[1].lod.screenHeights == std::vector<float>{0.4f});
    CHECK(result.models[1].lod.hysteresis == Data::Models::LodSettings{}.hysteresis);
    // Defaults: LOD 1 below a fifth of the window's height, LOD 2 below a tenth, 10 % hysteresis.
    CHECK(result.models[2].lod == Data::Models::LodSettings{{0.2f, 0.1f}, 0.1f});
}

TEST_CASE("ReadModelFilesJson reports bad LOD settings and leaves the entry out")
{
    for (const char* lod : {R"("lod": 3)", R"("lod": { "screenHeights": [0.1, 0.2] })",
                            R"("lod": { "screenHeights": [0.3, 0.2, 0.1] })", R"("lod": { "screenHeights": [0] })",
                            R"("lod": { "screenHeights": "near" })", R"("lod": { "hysteresis": 1 })",
                            R"("lod": { "hysteresis": -0.1 })"})
    {
        CAPTURE(lod);
        const ReadResult result =
            Read(Table(std::string(R"({ "model": "MODEL_ELITE_YETI", "file": "a.glb", )") + lod + " }"));
        CHECK(result.models.empty());
        CHECK(HasErrors(result.issues));
    }
    const ReadResult unknown =
        Read(Table(R"({ "model": "MODEL_ELITE_YETI", "file": "a.glb", "lod": { "distance": 3 } })"));
    REQUIRE(unknown.models.size() == 1);
    CHECK(HasIssue(unknown.issues, ItemDataIssueSeverity::Warning, "models[0].lod.distance", "unknown field"));
}

TEST_CASE("ReadModelFilesJson reports table errors and leaves the entry out")
{
    SUBCASE("unknown model")
    {
        const ReadResult result = Read(Table(R"({ "model": "MODEL_NOTHING", "file": "Monster/x.glb" })"));
        CHECK(result.models.empty());
        CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Error, "models[0].model", "MODEL_NOTHING is no model"));
    }
    SUBCASE("model that is not a monster")
    {
        const ReadResult result = Read(Table(R"({ "model": "MODEL_PLAYER", "file": "Player/x.glb" })"));
        CHECK(result.models.empty());
        CHECK(HasErrors(result.issues));
    }
    SUBCASE("missing file")
    {
        const ReadResult result = Read(Table(R"({ "model": "MODEL_ELITE_YETI" })"));
        CHECK(result.models.empty());
        CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Error, "models[0].file", "missing"));
    }
    SUBCASE("paths outside Data/")
    {
        for (const char* file : {"/abs/x.glb", "../x.glb", "Monster/../../x.glb", "Monster\\\\x.glb", "C:/x.glb",
                                 "Monster//x.glb", ""})
        {
            CAPTURE(file);
            const ReadResult result =
                Read(Table(std::string(R"({ "model": "MODEL_ELITE_YETI", "file": ")") + file + "\" }"));
            CHECK(result.models.empty());
            CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Error, "models[0].file", "relative to Data/"));
        }
    }
    SUBCASE("model named twice")
    {
        const ReadResult result = Read(Table(R"({ "model": "MODEL_ELITE_YETI", "file": "a.glb" },
                                                { "model": "MODEL_ELITE_YETI", "file": "b.glb" })"));
        REQUIRE(result.models.size() == 1);
        CHECK(result.models[0].file == "a.glb");
        CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Error, "models[1]", "more than one file"));
    }
    SUBCASE("bad action entries")
    {
        const ReadResult result = Read(Table(R"({ "model": "MODEL_ELITE_YETI", "file": "a.glb",
            "actions": { "x": { "clip": "a" }, "-1": { "clip": "a" }, "01": { "clip": "a" },
                         "40000": { "clip": "a" }, "3": { "crossfade": -1 }, "4": { "clip": 7 }, "5": 2 } })"));
        CHECK(result.models.empty());
        for (const char* field : {"models[0].actions.x", "models[0].actions.-1", "models[0].actions.01",
                                  "models[0].actions.40000"})
        {
            CAPTURE(field);
            CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Error, field, "no action index"));
        }
        CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Error, "models[0].actions.3.crossfade", "ticks"));
        CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Error, "models[0].actions.4.clip", "clip name"));
        CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Error, "models[0].actions.5", "must be an object"));
    }
    SUBCASE("file header")
    {
        CHECK(HasErrors(Read(R"({ "models": [] })").issues));
        CHECK(HasErrors(Read(R"({ "formatVersion": 2, "models": [] })").issues));
        CHECK(HasErrors(Read(R"({ "formatVersion": 1 })").issues));
        CHECK(HasErrors(Read("not json").issues));
    }
}

TEST_CASE("ReadModelFilesJson warns about unknown fields and empty actions but keeps the entry")
{
    const ReadResult result = Read(R"({ "formatVersion": 1, "comment": "x", "models": [
        { "model": "MODEL_ELITE_YETI", "file": "a.glb", "scale": 2,
          "actions": { "1": { "clip": "idle", "speed": 2 }, "2": {} } } ] })");
    INFO(Messages(result.issues));
    CHECK_FALSE(HasErrors(result.issues));
    REQUIRE(result.models.size() == 1);
    CHECK(result.models[0].actions.size() == 1);
    CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Warning, "comment", "unknown field"));
    CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Warning, "models[0].scale", "unknown field"));
    CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Warning, "models[0].actions.1.speed", "unknown field"));
    CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Warning, "models[0].actions.2", "neither a clip"));
}

TEST_CASE("LoadModelFiles: no table is no model and no issue")
{
    TemporaryDirectory game;
    const ModelFiles files = Data::Models::LoadModelFiles(DataDirectories{std::nullopt, game.Path()});
    CHECK(files.models.empty());
    CHECK(files.issues.empty());
    CHECK(files.table.empty());
}

TEST_CASE("ReadModelFileTable: a game without the table and without an overlay draws every model from its BMD")
{
    TemporaryDirectory game;
    const char* noOverlay = nullptr;
    SUBCASE("MU_DATA_OVERLAY unset") {}
    SUBCASE("MU_DATA_OVERLAY empty")
    {
        noOverlay = "";
    }
    const ModelFiles files = Data::Models::ReadModelFileTable(noOverlay, game.Path() / "Data");
    CHECK(files.models.empty());
    CHECK(files.issues.empty());
    CHECK(files.Find(MODEL_ELITE_YETI) == nullptr);
}

TEST_CASE("LoadModelFiles: a file found nowhere is an error")
{
    TemporaryDirectory game;
    game.Write("Models/ModelFiles.json", Table(R"({ "model": "MODEL_ELITE_YETI", "file": "Monster/Monster14.glb" },
                                                  { "model": "MODEL_HOUND", "file": "Monster/Monster02.glb" })"));
    game.Write("Monster/Monster02.glb", "glb");
    const ModelFiles files = Data::Models::LoadModelFiles(DataDirectories{std::nullopt, game.Path()});
    REQUIRE(files.models.size() == 1);
    CHECK(files.models[0].type == MODEL_HOUND);
    CHECK(files.models[0].path == game.Path() / "Monster/Monster02.glb");
    CHECK(files.Find(MODEL_HOUND) == &files.models[0]);
    CHECK(files.Find(MODEL_ELITE_YETI) == nullptr);
    CHECK(HasIssue(files.issues, ItemDataIssueSeverity::Error, "MODEL_ELITE_YETI", "Monster/Monster14.glb is not found"));
}

TEST_CASE("LoadModelFiles: the overlay's Data/ wins for the table and the files")
{
    TemporaryDirectory game;
    TemporaryDirectory overlayRoot;
    game.Write("Models/ModelFiles.json", Table(R"({ "model": "MODEL_HOUND", "file": "Monster/Monster02.glb" })"));
    game.Write("Monster/Monster02.glb", "game");
    game.Write("Monster/Monster14.glb", "game");

    // MU_DATA_OVERLAY's value as getenv gives it: narrow text, also where paths are wide (Windows).
    const std::string overlay = overlayRoot.Path().string();
    std::vector<ItemDataIssue> issues;
    SUBCASE("overlay table and file")
    {
        overlayRoot.Write("Data/Models/ModelFiles.json",
                          Table(R"({ "model": "MODEL_ELITE_YETI", "file": "Monster/Monster14.glb" })"));
        overlayRoot.Write("Data/Monster/Monster14.glb", "overlay");
        const DataDirectories directories =
            Data::Models::MakeDataDirectories(overlay.c_str(), game.Path(), issues);
        CHECK(issues.empty());
        REQUIRE(directories.overlay.has_value());
        const ModelFiles files = Data::Models::LoadModelFiles(directories);
        INFO(Messages(files.issues));
        CHECK(files.issues.empty());
        CHECK(files.table == overlayRoot.Path() / "Data/Models/ModelFiles.json");
        // The overlay's table replaces the game's: the hound has no file.
        REQUIRE(files.models.size() == 1);
        CHECK(files.Find(MODEL_HOUND) == nullptr);
        CHECK(files.models[0].path == overlayRoot.Path() / "Data/Monster/Monster14.glb");
    }
    SUBCASE("overlay table, game file")
    {
        overlayRoot.Write("Data/Models/ModelFiles.json",
                          Table(R"({ "model": "MODEL_ELITE_YETI", "file": "Monster/Monster14.glb" })"));
        const DataDirectories directories =
            Data::Models::MakeDataDirectories(overlay.c_str(), game.Path(), issues);
        const ModelFiles files = Data::Models::LoadModelFiles(directories);
        REQUIRE(files.models.size() == 1);
        CHECK(files.models[0].path == game.Path() / "Monster/Monster14.glb");
    }
    SUBCASE("game table, overlay file")
    {
        overlayRoot.Write("Data/Monster/Monster02.glb", "overlay");
        const DataDirectories directories =
            Data::Models::MakeDataDirectories(overlay.c_str(), game.Path(), issues);
        const ModelFiles files = Data::Models::LoadModelFiles(directories);
        CHECK(files.table == game.Path() / "Models/ModelFiles.json");
        REQUIRE(files.models.size() == 1);
        CHECK(files.models[0].path == overlayRoot.Path() / "Data/Monster/Monster02.glb");
    }
    SUBCASE("no overlay")
    {
        for (const char* value : {static_cast<const char*>(nullptr), ""})
        {
            const DataDirectories directories = Data::Models::MakeDataDirectories(value, game.Path(), issues);
            CHECK_FALSE(directories.overlay.has_value());
            const ModelFiles files = Data::Models::LoadModelFiles(directories);
            REQUIRE(files.models.size() == 1);
            CHECK(files.models[0].path == game.Path() / "Monster/Monster02.glb");
        }
        CHECK(issues.empty());
    }
    SUBCASE("an overlay without Data/ is left out with a warning")
    {
        const std::string missing = (overlayRoot.Path() / "missing").string();
        const DataDirectories directories = Data::Models::MakeDataDirectories(missing.c_str(), game.Path(), issues);
        CHECK_FALSE(directories.overlay.has_value());
        CHECK_FALSE(HasErrors(issues));
        CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, "", "no directory"));
        CHECK(Data::Models::LoadModelFiles(directories).models.size() == 1);
    }
}

TEST_CASE("The shipped model file table reads without issues")
{
    const ModelFiles files = Data::Models::LoadModelFiles(DataDirectories{std::nullopt, MU_TEST_DATA_DIR});
    INFO(Messages(files.issues));
    CHECK(files.issues.empty());
    CHECK_FALSE(files.table.empty());
}
