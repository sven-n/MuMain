#include "stdafx.h"

#include "doctest.h"

#include "Data/GameData/MonsterData/NpcNameText.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

using Data::Items::HasErrors;
using Data::Items::ItemDataIssue;

namespace
{
struct ReadResult
{
    std::map<int, std::string> names;
    std::vector<ItemDataIssue> issues;
};

ReadResult Read(const std::string& text)
{
    ReadResult result;
    Data::Monsters::ReadNpcNameText(text, "NpcName_Eng.txt", result.names, result.issues);
    return result;
}

bool HasIssueAbout(const std::vector<ItemDataIssue>& issues, const std::string& field, const std::string& message)
{
    return std::any_of(issues.begin(), issues.end(), [&](const ItemDataIssue& issue)
                       { return issue.field == field && issue.message.find(message) != std::string::npos; });
}
} // namespace

TEST_CASE("Name files: read the names by number, skipping comments, up to end [data][monsters]")
{
    const ReadResult read = Read("// MAX : 512\t\t\t\r\n"
                                 "//\xC2\xBA\xED\xB7\xAF\r\n"
                                 "84\t1\t\"Chief Skeleton Warrior\"\t\r\n"
                                 "\r\n"
                                 "  2 1 \"Budge Dragon\" // the first monster\r\n"
                                 "252\t1\t\"\"\r\n"
                                 "end\r\n"
                                 "3\t1\t\"After the end\"\r\n");

    CHECK(read.issues.empty());
    REQUIRE(read.names.size() == 3);
    CHECK(read.names.at(84) == "Chief Skeleton Warrior");
    CHECK(read.names.at(2) == "Budge Dragon");
    CHECK(read.names.at(252).empty());
}

TEST_CASE("Name files: bad lines, numbers outside 0-1023 and doubles are skipped with a warning [data][monsters]")
{
    const ReadResult read = Read("abc\n"
                                 "1024\t1\t\"Too high\"\n"
                                 "5\t1\t\"Five\"\n"
                                 "5\t1\t\"Five again\"\n"
                                 "6\t1\tNoQuotes\n");

    CHECK_FALSE(HasErrors(read.issues));
    CHECK(HasIssueAbout(read.issues, "line 1", "skipped"));
    CHECK(HasIssueAbout(read.issues, "line 2", "0 to 1023"));
    CHECK(HasIssueAbout(read.issues, "line 4", "has a name already"));
    CHECK(HasIssueAbout(read.issues, "line 5", "skipped"));
    REQUIRE(read.names.size() == 1);
    CHECK(read.names.at(5) == "Five");
}

TEST_CASE("Name files: a name longer than the game holds is cut [data][monsters]")
{
    const ReadResult read = Read("7\t1\t\"" + std::string(70, 'x') + "\"\n");

    CHECK(HasIssueAbout(read.issues, "line 1", "was cut"));
    CHECK(read.names.at(7) == std::string(64, 'x'));
}

TEST_CASE("Name files: written sorted with an end line, and read back the same [data][monsters]")
{
    const std::map<int, std::string> names = {{84, "Chief Skeleton Warrior"}, {2, "Budge Dragon"}, {252, ""}};
    std::vector<ItemDataIssue> issues;

    const std::string text = Data::Monsters::WriteNpcNameText(names, issues);

    CHECK(issues.empty());
    CHECK(text == "2\t1\t\"Budge Dragon\"\r\n"
                  "84\t1\t\"Chief Skeleton Warrior\"\r\n"
                  "252\t1\t\"\"\r\n"
                  "end\r\n");
    CHECK(Read(text).names == names);
}

TEST_CASE("Name files: a name with a quote is left out of the written file [data][monsters]")
{
    std::vector<ItemDataIssue> issues;

    const std::string text = Data::Monsters::WriteNpcNameText({{7, "Say \"hi\""}, {8, "Fine"}}, issues);

    CHECK(text == "8\t1\t\"Fine\"\r\nend\r\n");
    CHECK(HasIssueAbout(issues, "monster 7 name", "quote"));
}
