#include "doctest.h"

#include "ContentMemoryStorage.h"
#include "Core/Content/ContentStore.h"

#include <SDL3/SDL_log.h>

#include <atomic>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace
{
Content::Path P(const char* text)
{
    return *Content::Path::From(text);
}

// The content for one test case, closed again at its end.
class MemoryContent
{
public:
    explicit MemoryContent(ContentMemoryStorage::Files files)
    {
        Content::Open(ContentMemoryStorage::Open(std::make_shared<ContentMemoryStorage::Files>(std::move(files))));
    }
    ~MemoryContent()
    {
        Content::Close();
    }
};

struct LogCapture
{
    LogCapture()
    {
        SDL_GetLogOutputFunction(&previous, &previousUserdata);
        SDL_SetLogOutputFunction([](void* userdata, int, SDL_LogPriority, const char* message)
                                 { static_cast<LogCapture*>(userdata)->lines.emplace_back(message); }, this);
    }
    ~LogCapture()
    {
        SDL_SetLogOutputFunction(previous, previousUserdata);
    }

    SDL_LogOutputFunction previous = nullptr;
    void* previousUserdata = nullptr;
    std::vector<std::string> lines;
};
} // namespace

TEST_CASE("content reads a file in any case and with either separator [content]")
{
    MemoryContent content({{"Data/Sound/Hit.wav", "hit"}});

    CHECK(Content::ReadAll(P("Data/Sound/Hit.wav")) == "hit");
    CHECK(Content::ReadAll(P("data\\SOUND\\hit.WAV")) == "hit");
    CHECK(Content::Exists(P("DATA/sound/hit.wav")));
}

TEST_CASE("a missing file or a folder is not read, and says which path [content]")
{
    MemoryContent content({{"Data/Sound/Hit.wav", "hit"}});

    std::string error;
    CHECK_FALSE(Content::ReadAll(P("Data/Sound/Miss.wav"), &error));
    CHECK(error == "Data/Sound/Miss.wav: not found");
    CHECK_FALSE(Content::ReadAll(P("Data/Sound"), &error));
    CHECK(error == "Data/Sound: not a file");
    CHECK_FALSE(Content::Exists(P("Data/Sound")));
    CHECK_FALSE(Content::Exists(P("Nope/Hit.wav")));
}

TEST_CASE("an empty file reads as no bytes [content]")
{
    MemoryContent content({{"Data/Empty.txt", ""}});
    CHECK(Content::ReadAll(P("Data/Empty.txt")) == "");
}

TEST_CASE("a folder lists its files and folders by their stored names [content]")
{
    MemoryContent content({{"Data/Items/Items.json", "{}"},
                           {"Data/Items/Models/Group00.json", "{}"},
                           {"Data/Sound/Hit.wav", "hit"},
                           {"Readme.txt", "r"}});

    const std::vector<Content::Entry> items = Content::List("data/items");
    REQUIRE(items.size() == 2);
    CHECK(items[0].name == "Items.json");
    CHECK_FALSE(items[0].folder);
    CHECK(items[1].name == "Models");
    CHECK(items[1].folder);

    const std::vector<Content::Entry> root = Content::List("");
    REQUIRE(root.size() == 2);
    CHECK(root[0].name == "Data");
    CHECK(root[1].name == "Readme.txt");

    CHECK(Content::List("Data/Missing").empty());
    CHECK(Content::List("Readme.txt").empty());
}

TEST_CASE("two names that differ only in case read the first in sort order, with one warning [content]")
{
    MemoryContent content({{"Data/a/x.txt", "from a"}, {"Data/A/x.txt", "from A"}});
    LogCapture log;

    // 'A' sorts before 'a'.
    CHECK(Content::ReadAll(P("data/a/X.TXT")) == "from A");
    CHECK(Content::ReadAll(P("DATA/a/x.TXT")) == "from A");
    REQUIRE(log.lines.size() == 1);
    CHECK(log.lines[0].find("differ only in case") != std::string::npos);
}

TEST_CASE("after a refresh, a file added under another case is found [content]")
{
    auto files =
        std::make_shared<ContentMemoryStorage::Files>(ContentMemoryStorage::Files{{"Data/Items/Items.json", "{}"}});
    Content::Open(ContentMemoryStorage::Open(files));

    CHECK_FALSE(Content::Exists(P("data/items/new.json")));
    (*files)["Data/Items/New.json"] = "{}";
    CHECK_FALSE(Content::Exists(P("data/items/new.json")));
    Content::Refresh();
    CHECK(Content::Exists(P("data/items/new.json")));
    Content::Close();
}

TEST_CASE("a file a saver writes is found in another case once the saver returns [content]")
{
    const std::filesystem::path folder = std::filesystem::temp_directory_path() / "mu_content_saver_test";
    std::filesystem::remove_all(folder);
    std::filesystem::create_directories(folder / "Data" / "World1");
    const std::u8string root = folder.u8string();
    Content::Open(SDL_OpenTitleStorage(reinterpret_cast<const char*>(root.c_str()), 0));

    // The miss lists Data/World1 without the file.
    CHECK_FALSE(Content::Exists(P("data/world1/terrain.map")));
    {
        const Content::RefreshAfterWrite refreshAfterWrite;
        std::ofstream(folder / "Data" / "World1" / "Terrain.map", std::ios::binary) << "map";
    }
    CHECK(Content::ReadAll(P("data/world1/terrain.map")) == "map");

    Content::Close();
    std::filesystem::remove_all(folder);
}

TEST_CASE("content is read from several threads at once [content]")
{
    ContentMemoryStorage::Files files;
    for (int i = 0; i < 64; ++i)
    {
        files["Data/Texture/T" + std::to_string(i) + ".ozj"] = std::to_string(i);
    }
    MemoryContent content(std::move(files));

    std::atomic<int> wrong = 0;
    std::vector<std::thread> threads;
    for (int t = 0; t < 8; ++t)
    {
        threads.emplace_back(
            [&wrong]
            {
                for (int i = 0; i < 64; ++i)
                {
                    const auto path = Content::Path::From("data/texture/t" + std::to_string(i) + ".OZJ");
                    if (Content::ReadAll(*path) != std::to_string(i))
                    {
                        ++wrong;
                    }
                }
            });
    }
    for (std::thread& thread : threads)
    {
        thread.join();
    }
    CHECK(wrong == 0);
}

TEST_CASE("a relative path reads content and an absolute one reads the disk [content]")
{
    const std::filesystem::path disk = std::filesystem::temp_directory_path() / "mu_content_any_path.txt";
    std::ofstream(disk, std::ios::binary) << "disk";
    MemoryContent content({{"Data/Hit.txt", "memory"}});

    CHECK(Content::ReadFile("Data\\hit.TXT") == "memory");
    CHECK(Content::ReadFile(disk) == "disk");
    CHECK(Content::Probe("data/hit.txt") == Content::Found::File);
    CHECK(Content::Probe(disk) == Content::Found::File);
    CHECK(Content::Probe("DATA") == Content::Found::Folder);
    CHECK(Content::Probe("Data/Miss.txt") == Content::Found::Nothing);
    CHECK(Content::Probe(disk.parent_path() / "mu_content_no_such_file") == Content::Found::Nothing);
    CHECK(Content::ListFolder("data").size() == 1);
    CHECK(Content::ListFolder(disk.parent_path()).size() > 0);

    std::filesystem::remove(disk);
}

TEST_CASE("without opened content, files are read from the working directory in any case [content]")
{
    const std::filesystem::path folder = std::filesystem::temp_directory_path() / "mu_content_store_test";
    std::filesystem::remove_all(folder);
    std::filesystem::create_directories(folder / "Data");
    std::ofstream(folder / "Data" / "Hit.txt", std::ios::binary) << "hit";
    const std::filesystem::path previous = std::filesystem::current_path();
    std::filesystem::current_path(folder);

    CHECK(Content::ReadAll(P("Data/Hit.txt")) == "hit");
    CHECK(Content::ReadAll(P("data/HIT.txt")) == "hit");
    CHECK(Content::List("Data").size() == 1);
    const std::vector<Content::Entry> root = Content::List("");
    REQUIRE(root.size() == 1);
    CHECK(root[0].name == "Data");
    CHECK(root[0].folder);

    std::filesystem::current_path(previous);
    std::filesystem::remove_all(folder);
}
