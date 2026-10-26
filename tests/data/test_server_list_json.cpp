#include "stdafx.h"

#include "doctest.h"

#include "Core/Text/Utf8.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ServerListData/ServerListJson.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <vector>

using Data::Items::HasErrors;
using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using Data::ServerList::ChannelType;
using Data::ServerList::ServerGroup;

namespace
{
struct ReadResult
{
    std::vector<ServerGroup> groups;
    std::vector<ItemDataIssue> issues;
};

ReadResult Read(const std::string& text)
{
    ReadResult result;
    Data::ServerList::ReadServerListJson(text, "ServerList.json", result.groups, result.issues);
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

bool HasIssueAbout(const std::vector<ItemDataIssue>& issues, const std::string& field, const std::string& message)
{
    return std::any_of(issues.begin(), issues.end(), [&](const ItemDataIssue& issue)
                       { return issue.field == field && issue.message.find(message) != std::string::npos; });
}

bool HasWarnings(const std::vector<ItemDataIssue>& issues)
{
    return std::any_of(issues.begin(), issues.end(),
                       [](const ItemDataIssue& issue) { return issue.severity == ItemDataIssueSeverity::Warning; });
}

// One group with nothing else around it.
std::string GroupFile(const std::string& group)
{
    return R"({ "formatVersion": 1, "groups": [ )" + group + " ] }";
}

const std::string ValidFile = R"({
  "formatVersion": 1,
  "groups": [
    { "id": 0, "name": "Valhalla", "channels": {"1": "gold", "5": "goldPvp", "6": "nonPvp", "20": "normal"} },
    { "id": 3, "name": "Kanturu", "description": "No PvP here", "testServer": true }
  ]
})";
} // namespace

TEST_CASE("ServerListJson: reads the groups in their order")
{
    const ReadResult read = Read(ValidFile);

    CHECK_MESSAGE(read.issues.empty(), Messages(read.issues));
    REQUIRE(read.groups.size() == 2);

    const ServerGroup& valhalla = read.groups[0];
    CHECK(valhalla.id == 0);
    CHECK(valhalla.name == "Valhalla");
    CHECK(valhalla.description.empty());
    CHECK_FALSE(valhalla.testServer);
    CHECK(valhalla.channels[0] == ChannelType::Gold);
    CHECK(valhalla.channels[1] == ChannelType::Normal);
    CHECK(valhalla.channels[4] == ChannelType::GoldPvp);
    CHECK(valhalla.channels[5] == ChannelType::NonPvp);
    CHECK(valhalla.channels[19] == ChannelType::Normal);

    const ServerGroup& kanturu = read.groups[1];
    CHECK(kanturu.id == 3);
    CHECK(kanturu.description == "No PvP here");
    CHECK(kanturu.testServer);
    CHECK(std::all_of(kanturu.channels.begin(), kanturu.channels.end(),
                      [](ChannelType type) { return type == ChannelType::Normal; }));
}

TEST_CASE("ServerListJson: writes what it reads")
{
    const ReadResult read = Read(ValidFile);
    const std::string written = Data::ServerList::WriteServerListJson(read.groups);
    const ReadResult readBack = Read(written);

    CHECK_MESSAGE(readBack.issues.empty(), Messages(readBack.issues));
    CHECK(readBack.groups == read.groups);
    // Normal channels and empty or false fields are left out.
    CHECK(written.find("\"20\"") == std::string::npos);
    CHECK(written.find("\"description\": \"\"") == std::string::npos);
    CHECK(written.find("\"testServer\": false") == std::string::npos);
    // The channels of a group are on one line.
    CHECK(written.find(R"("channels": {"1": "gold", "5": "goldPvp", "6": "nonPvp"})") != std::string::npos);
}

TEST_CASE("ServerListJson: an empty list is a valid file")
{
    const ReadResult read = Read(R"({ "formatVersion": 1, "groups": [] })");

    CHECK(read.issues.empty());
    CHECK(read.groups.empty());
    CHECK(Data::ServerList::WriteServerListJson(read.groups) == "{\n  \"formatVersion\": 1,\n  \"groups\": []\n}\n");
}

TEST_CASE("ServerListJson: the file needs its version and a list of groups")
{
    CHECK(HasErrors(Read("not json").issues));
    CHECK(HasErrors(Read(R"({ "groups": [] })").issues));
    CHECK(HasErrors(Read(R"({ "formatVersion": 2, "groups": [] })").issues));

    const ReadResult noList = Read(R"({ "formatVersion": 1, "groups": {} })");
    CHECK(HasIssueAbout(noList.issues, "groups", "not a list"));
}

TEST_CASE("ServerListJson: a group needs an id and a name")
{
    const ReadResult noId = Read(GroupFile(R"({ "name": "Valhalla" })"));
    CHECK(HasIssueAbout(noId.issues, "groups[0].id", "missing"));
    CHECK(noId.groups.empty());

    const ReadResult badId = Read(GroupFile(R"({ "id": 3277, "name": "Valhalla" })"));
    CHECK(HasIssueAbout(badId.issues, "groups[0].id", "from 0 to 3276"));

    const ReadResult negativeId = Read(GroupFile(R"({ "id": -1, "name": "Valhalla" })"));
    CHECK(HasErrors(negativeId.issues));

    const ReadResult noName = Read(GroupFile(R"({ "id": 0 })"));
    CHECK(HasIssueAbout(noName.issues, "groups[0].name", "missing or empty"));

    const ReadResult emptyName = Read(GroupFile(R"({ "id": 0, "name": "" })"));
    CHECK(HasIssueAbout(emptyName.issues, "groups[0].name", "missing or empty"));

    const ReadResult notAnObject = Read(GroupFile("5"));
    CHECK(HasIssueAbout(notAnObject.issues, "groups[0]", "must be an object"));
}

TEST_CASE("ServerListJson: a group id is in the list once")
{
    const ReadResult read = Read(R"({ "formatVersion": 1, "groups": [
        { "id": 1, "name": "Helheim" }, { "id": 1, "name": "Midgard" } ] })");

    CHECK(HasIssueAbout(read.issues, "groups[1].id", "more than once"));
    REQUIRE(read.groups.size() == 1);
    CHECK(read.groups[0].name == "Helheim");
}

TEST_CASE("ServerListJson: channels are numbered 1 to 20 and have a known type")
{
    const ReadResult zero = Read(GroupFile(R"({ "id": 0, "name": "A", "channels": {"0": "gold"} })"));
    CHECK(HasIssueAbout(zero.issues, "groups[0].channels.0", "1 to 20"));

    const ReadResult tooHigh = Read(GroupFile(R"({ "id": 0, "name": "A", "channels": {"21": "gold"} })"));
    CHECK(HasIssueAbout(tooHigh.issues, "groups[0].channels.21", "1 to 20"));

    const ReadResult notANumber = Read(GroupFile(R"({ "id": 0, "name": "A", "channels": {"1a": "gold"} })"));
    CHECK(HasErrors(notANumber.issues));

    const ReadResult unknownType = Read(GroupFile(R"({ "id": 0, "name": "A", "channels": {"1": "silver"} })"));
    CHECK(HasIssueAbout(unknownType.issues, "groups[0].channels.1", "normal, nonPvp, goldPvp or gold"));

    const ReadResult notAnObject = Read(GroupFile(R"({ "id": 0, "name": "A", "channels": ["gold"] })"));
    CHECK(HasIssueAbout(notAnObject.issues, "groups[0].channels", "object"));
}

TEST_CASE("ServerListJson: a name or description that is too long is cut")
{
    const std::string longName(40, 'x');
    const std::string longDescription(300, 'y');
    const ReadResult read =
        Read(GroupFile(R"({ "id": 0, "name": ")" + longName + R"(", "description": ")" + longDescription + R"(" })"));

    CHECK_FALSE(HasErrors(read.issues));
    CHECK(HasIssueAbout(read.issues, "groups[0].name", "longer than 32 characters"));
    CHECK(HasIssueAbout(read.issues, "groups[0].description", "longer than 254 characters"));
    REQUIRE(read.groups.size() == 1);
    CHECK(read.groups[0].name == std::string(32, 'x'));
    CHECK(read.groups[0].description.size() == 254);
}

TEST_CASE("ServerListJson: names are counted in UTF-16 units, not bytes")
{
    // 32 two-byte characters fit; the cut never splits one.
    std::string name;
    for (int i = 0; i < 33; ++i)
    {
        name += "\xC3\xA4"; // a-umlaut
    }
    const ReadResult read = Read(GroupFile(R"({ "id": 0, "name": ")" + name + R"(" })"));

    REQUIRE(read.groups.size() == 1);
    CHECK(read.groups[0].name == name.substr(0, 64));
}

TEST_CASE("ServerListJson: an emoji counts as two UTF-16 units")
{
    const std::string emoji = "\xF0\x9F\x98\x80"; // U+1F600, a surrogate pair in UTF-16
    std::string emojis;
    for (int i = 0; i < 17; ++i)
    {
        emojis += emoji;
    }
    const ReadResult onlyEmoji = Read(GroupFile(R"({ "id": 0, "name": ")" + emojis + R"(" })"));
    REQUIRE(onlyEmoji.groups.size() == 1);
    CHECK(onlyEmoji.groups[0].name == emojis.substr(0, 16 * emoji.size()));

    // 31 letters and an emoji are 33 units: the emoji goes, whole.
    const std::string mixed = std::string(31, 'x') + emoji;
    const ReadResult cut = Read(GroupFile(R"({ "id": 0, "name": ")" + mixed + R"(" })"));
    REQUIRE(cut.groups.size() == 1);
    CHECK(cut.groups[0].name == std::string(31, 'x'));
    CHECK(HasIssueAbout(cut.issues, "groups[0].name", "an emoji counts as two"));

    // 30 letters and an emoji are exactly 32.
    std::string fits = std::string(30, 'x') + emoji;
    CHECK_FALSE(Core::Text::CutToUtf16Length(fits, Data::ServerList::MaxServerGroupNameLength));
    CHECK(fits == std::string(30, 'x') + emoji);
}

TEST_CASE("ServerListJson: unknown fields and a second test server are warnings")
{
    const ReadResult read = Read(R"({ "formatVersion": 1, "color": "red", "groups": [
        { "id": 0, "name": "A", "testServer": true, "order": 3 },
        { "id": 1, "name": "B", "testServer": true } ] })");

    CHECK_FALSE(HasErrors(read.issues));
    CHECK(HasWarnings(read.issues));
    CHECK(HasIssueAbout(read.issues, "color", "unknown field"));
    CHECK(HasIssueAbout(read.issues, "groups[0].order", "unknown field"));
    CHECK(HasIssueAbout(read.issues, "testServer", "only the first test server group"));
    CHECK(read.groups.size() == 2);
}

TEST_CASE("ServerListJson: wrong types are errors")
{
    CHECK(HasIssueAbout(Read(GroupFile(R"({ "id": 0, "name": 5 })")).issues, "groups[0].name", "must be a text"));
    CHECK(HasIssueAbout(Read(GroupFile(R"({ "id": 0, "name": "A", "testServer": 1 })")).issues, "groups[0].testServer",
                        "true or false"));
    CHECK(HasIssueAbout(Read(GroupFile(R"({ "id": "0", "name": "A" })")).issues, "groups[0].id", "whole number"));
}

TEST_CASE("ServerListJson: channel type names")
{
    for (const ChannelType type : {ChannelType::Normal, ChannelType::NonPvp, ChannelType::GoldPvp, ChannelType::Gold})
    {
        CHECK(Data::ServerList::FindChannelType(Data::ServerList::GetChannelTypeName(type)) == type);
    }
    CHECK_FALSE(Data::ServerList::FindChannelType("Gold").has_value());
}

TEST_CASE("ServerListJson: the shipped file reads without problems and is written as the editor writes it")
{
    // MU_TEST_DATA_DIR is the client folder's Data; the file name starts with it.
    const std::filesystem::path file =
        std::filesystem::path(MU_TEST_DATA_DIR).parent_path() / Data::ServerList::ServerListFile;
    const Data::ServerList::ServerListLoadResult loaded = Data::ServerList::LoadServerListFile(file);

    CHECK_MESSAGE(loaded.issues.empty(), Messages(loaded.issues));
    REQUIRE(loaded.groups.size() == 3);
    CHECK(loaded.groups[0].name == "Valhalla");
    CHECK(loaded.groups[1].name == "Helheim");
    CHECK(loaded.groups[2].name == "Midgard");

    const std::optional<std::string> text = Data::Items::Json::ReadTextFile(file);
    REQUIRE(text.has_value());
    CHECK(Data::ServerList::WriteServerListJson(loaded.groups) == *text);
}

TEST_CASE("ServerListJson: a missing file is an error")
{
    const Data::ServerList::ServerListLoadResult loaded =
        Data::ServerList::LoadServerListFile("does/not/exist/ServerList.json");

    CHECK(HasIssueAbout(loaded.issues, "", "could not be read"));
    CHECK(loaded.groups.empty());
}

TEST_CASE("ServerListJson: save writes the file and refuses a list with errors")
{
    const std::filesystem::path folder = std::filesystem::temp_directory_path() / "mu_server_list_save_test";
    std::filesystem::remove_all(folder);
    const std::filesystem::path file = folder / "Local" / "ServerList.json";
    const ReadResult read = Read(ValidFile);

    std::vector<ItemDataIssue> issues;
    REQUIRE(Data::ServerList::SaveServerListFile(file, read.groups, issues));
    CHECK(issues.empty());
    const Data::ServerList::ServerListLoadResult saved = Data::ServerList::LoadServerListFile(file);
    CHECK(saved.issues.empty());
    CHECK(saved.groups == read.groups);

    std::vector<ServerGroup> broken = read.groups;
    broken[1].name.clear();
    broken.push_back(read.groups[0]);
    std::vector<ItemDataIssue> refused;
    CHECK_FALSE(Data::ServerList::SaveServerListFile(file, broken, refused));
    CHECK(HasIssueAbout(refused, "groups[1].name", "missing or empty"));
    CHECK(HasIssueAbout(refused, "groups[2].id", "more than once"));
    // The file stays as it was.
    CHECK(Data::ServerList::LoadServerListFile(file).groups == read.groups);
    CHECK_FALSE(std::filesystem::exists(file.string() + ".tmp"));

    std::filesystem::remove_all(folder);
}

TEST_CASE("ServerListJson: a save that cannot write or replace the file names it and leaves no temporary file")
{
    const std::filesystem::path folder = std::filesystem::temp_directory_path() / "mu_server_list_blocked_test";
    std::filesystem::remove_all(folder);
    std::filesystem::create_directories(folder);
    const std::vector<ServerGroup> groups = Read(ValidFile).groups;

    // A file where the Local folder should be: nothing can be written there.
    const std::filesystem::path blocked = folder / "Local";
    std::ofstream(blocked) << "not a folder";
    const std::filesystem::path unwritable = blocked / "ServerList.json";
    std::vector<ItemDataIssue> issues;
    CHECK_FALSE(Data::ServerList::SaveServerListFile(unwritable, groups, issues));
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].source == unwritable.generic_string());
    CHECK(issues[0].message == "could not be written");
    CHECK_FALSE(std::filesystem::exists(unwritable.string() + ".tmp"));

    // A folder with the file's name: the written file cannot take its place.
    const std::filesystem::path irreplaceable = folder / "ServerList.json";
    std::filesystem::create_directories(irreplaceable / "inside");
    issues.clear();
    CHECK_FALSE(Data::ServerList::SaveServerListFile(irreplaceable, groups, issues));
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].source == irreplaceable.generic_string());
    CHECK(issues[0].message.starts_with("could not be replaced: "));
    CHECK_FALSE(std::filesystem::exists(irreplaceable.string() + ".tmp"));

    std::filesystem::remove_all(folder);
}
