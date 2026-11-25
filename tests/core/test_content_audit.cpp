#include "doctest.h"

#include "ContentMemoryStorage.h"
#include "Core/Content/ContentAudit.h"
#include "Core/Content/ContentStore.h"

#include <SDL3/SDL_log.h>

#include <memory>
#include <string>
#include <vector>

namespace
{
Content::Path P(const char* text)
{
    return *Content::Path::From(text);
}

// The content of one test case with the audit on, both closed again at its end.
class AuditedContent
{
public:
    explicit AuditedContent(ContentMemoryStorage::Files files)
    {
        Content::Open(ContentMemoryStorage::Open(std::make_shared<ContentMemoryStorage::Files>(std::move(files))));
    }
    ~AuditedContent()
    {
        Content::Audit::Stop();
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

    // The audit's lines only.
    std::vector<std::string> Audited() const
    {
        std::vector<std::string> audited;
        for (const std::string& line : lines)
        {
            if (line.find("outside the file lists") != std::string::npos)
            {
                audited.push_back(line);
            }
        }
        return audited;
    }

    SDL_LogOutputFunction previous = nullptr;
    void* previousUserdata = nullptr;
    std::vector<std::string> lines;
};

const ContentMemoryStorage::Files Lists = {
    {"Data/Maps/Files/Shared.json", R"({"files": ["Data/Shared/A.bmd", "Data/Shared/B.OZJ"]})"},
    {"Data/Maps/Files/Map3.json", R"({"files": ["Data/Map3/Tree.bmd"]})"},
    {"Data/Maps/Files/Monsters.json", R"({"7": ["Data/Monster/Boss.bmd"], "8": ["Data/Monster/Imp.bmd"]})"},
};

ContentMemoryStorage::Files WithFiles(std::initializer_list<const char*> names)
{
    ContentMemoryStorage::Files files = Lists;
    for (const char* name : names)
    {
        files[name] = "x";
    }
    return files;
}
} // namespace

TEST_CASE("the lists' files are read by key, in lower case [content audit]")
{
    const auto lists = Content::Audit::ParseLists(R"({"files": ["Data/A.bmd", "Data\\B.bmd"]})");
    REQUIRE(lists.count("files") == 1);
    CHECK(lists.at("files") == std::set<std::string>{"data/a.bmd", "data/b.bmd"});

    const auto monsters = Content::Audit::ParseLists("{\n \"0\": [\"a\"],\n \"12\": [\"b\", \"c\"]\n}");
    CHECK(monsters.at("0").size() == 1);
    CHECK(monsters.at("12") == std::set<std::string>{"b", "c"});
}

TEST_CASE("a read inside the lists logs nothing, one outside logs once [content audit]")
{
    AuditedContent content(WithFiles({"Data/Shared/A.bmd", "Data/Map3/Tree.bmd", "Data/Monster/Boss.bmd",
                                      "Data/Monster/Imp.bmd", "Data/Other/Stray.bmd"}));
    REQUIRE(Content::Audit::Start());
    Content::Audit::EnterMap(3);
    Content::Audit::NoteMonster(7);

    LogCapture log;
    CHECK(Content::ReadAll(P("Data/Shared/A.bmd")));
    CHECK(Content::ReadAll(P("data/map3/TREE.bmd")));
    CHECK(Content::ReadAll(P("Data/Monster/Boss.bmd")));
    CHECK(log.Audited().empty());

    CHECK(Content::ReadAll(P("Data/Other/Stray.bmd")));
    REQUIRE(log.Audited().size() == 1);
    CHECK(log.Audited()[0].find("Data/Other/Stray.bmd") != std::string::npos);
    CHECK(log.Audited()[0].find("map 3") != std::string::npos);

    // Once per file, whatever the spelling; another outside file logs again.
    CHECK(Content::ReadAll(P("Data/Other/Stray.bmd")));
    CHECK(Content::ReadAll(P("DATA/other/stray.BMD")));
    CHECK(log.Audited().size() == 1);
    CHECK(Content::ReadAll(P("Data/Monster/Imp.bmd")));
    CHECK(log.Audited().size() == 2);
}

TEST_CASE("before any map is entered the audit says so [content audit]")
{
    AuditedContent content(WithFiles({"Data/Map3/Tree.bmd", "Data/Monster/Boss.bmd"}));
    REQUIRE(Content::Audit::Start());

    LogCapture log;
    CHECK(Content::ReadAll(P("Data/Map3/Tree.bmd")));
    REQUIRE(log.Audited().size() == 1);
    CHECK(log.Audited()[0].find("no map yet") != std::string::npos);
}

TEST_CASE("sounds, music and the lists themselves are not audited [content audit]")
{
    AuditedContent content(WithFiles({"Data/Sound/Hit.wav", "Data/Music/Town.mp3"}));
    REQUIRE(Content::Audit::Start());

    LogCapture log;
    CHECK(Content::ReadAll(P("Data/Sound/Hit.wav")));
    CHECK(Content::ReadAll(P("Data/Music/Town.mp3")));
    CHECK(Content::ReadAll(P("Data/Maps/Files/Map3.json")));
    CHECK(log.Audited().empty());
}

TEST_CASE("without the audit nothing is loaded and nothing is logged [content audit]")
{
    AuditedContent content(WithFiles({"Data/Other/Stray.bmd"}));
    CHECK_FALSE(Content::Audit::Active());

    LogCapture log;
    CHECK(Content::ReadAll(P("Data/Other/Stray.bmd")));
    CHECK(log.lines.empty());
}

TEST_CASE("the audit stays off when Shared.json can't be read [content audit]")
{
    AuditedContent content({{"Data/Other/Stray.bmd", "x"}});

    LogCapture log;
    CHECK_FALSE(Content::Audit::Start());
    CHECK_FALSE(Content::Audit::Active());
    CHECK(Content::ReadAll(P("Data/Other/Stray.bmd")));
    CHECK(log.Audited().empty());
}

TEST_CASE("a name that is not a file is not a read of content [content audit]")
{
    AuditedContent content(WithFiles({}));
    REQUIRE(Content::Audit::Start());

    LogCapture log;
    CHECK_FALSE(Content::ReadAll(P("Data/Other/Missing.OZJ")));
    CHECK(log.Audited().empty());
}
