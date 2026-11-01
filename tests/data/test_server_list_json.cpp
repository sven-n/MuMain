#include "stdafx.h"

#include "doctest.h"

#include "Core/Text/Utf8.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ServerListData/ServerListJson.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <vector>

using Data::Items::HasFatal;
using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using Data::ServerList::ChannelNamePlace;
using Data::ServerList::ChannelSettings;
using Data::ServerList::RuleVip;
using Data::ServerList::ServerGroup;
using Data::ServerList::ServerListData;

namespace
{
struct ReadResult
{
    ServerListData list;
    std::vector<ItemDataIssue> issues;
};

ReadResult Read(const std::string& text)
{
    ReadResult result;
    Data::ServerList::ReadServerListJson(text, "ServerList.json", result.list, result.issues);
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

bool HasIssueAbout(const std::vector<ItemDataIssue>& issues, const std::string& field, const std::string& message,
                   std::optional<ItemDataIssueSeverity> severity = std::nullopt)
{
    return std::any_of(issues.begin(), issues.end(),
                       [&](const ItemDataIssue& issue)
                       {
                           return issue.field == field && issue.message.find(message) != std::string::npos &&
                                  (!severity || issue.severity == *severity);
                       });
}

bool HasWarnings(const std::vector<ItemDataIssue>& issues)
{
    return std::any_of(issues.begin(), issues.end(),
                       [](const ItemDataIssue& issue) { return issue.severity == ItemDataIssueSeverity::Warning; });
}

// One group with nothing else around it.
std::string GroupFile(const std::string& group)
{
    return R"({ "formatVersion": 2, "vipTypes": [ {"id": "gold", "name": "Gold"} ], "groups": [ )" + group + " ] }";
}

// The file with these VIP types and no group.
std::string VipTypesFile(const std::string& vipTypes)
{
    return R"({ "formatVersion": 2, "vipTypes": [ )" + vipTypes + R"( ], "groups": [] })";
}

// The file with these texts and no group.
std::string NamesFile(const std::string& names)
{
    return R"({ "formatVersion": 2, "names": )" + names + R"(, "groups": [] })";
}

ChannelSettings Channel(bool pvp, const std::string& vip)
{
    ChannelSettings channel;
    channel.pvp = pvp;
    channel.vip = vip;
    return channel;
}

// The game's languages.
constexpr std::array<const char*, 12> GameLocales = {"en", "de", "es", "id", "ja",    "pl",
                                                     "pt", "ru", "tl", "uk", "zh-CN", "zh-TW"};

const std::string ValidFile = R"({
  "formatVersion": 2,
  "vipTypes": [
    {"id": "gold", "name": {"en": "Gold", "es": "Oro"}},
    {"id": "silver", "name": "Silver"}
  ],
  "colors": [
    {"id": "goldRed", "name": "Gold red", "shades": {"legacy": "#FF5A5A", "modern": "#C0564E"}}
  ],
  "buttonColors": [
    {"vip": "gold", "pvp": false, "color": "goldRed"},
    {"vip": true, "color": "silver"},
    {"vip": false, "pvp": true, "color": "gray"}
  ],
  "names": {
    "nonPvp": {"en": "Non-PvP", "de": "Nicht-PvP"},
    "login": {"en": "[{name}-{channel} {vip} {pvp}]", "es": "[{name}-{channel} {pvp} {vip}]"}
  },
  "groups": [
    { "id": 0, "name": "Valhalla",
      "channels": {"1": {"vip": "gold", "pvp": false}, "5": {"vip": "gold"}, "6": {"pvp": false}, "7": {"vip": "silver"}, "20": {}} },
    { "id": 3, "name": "Kanturu", "description": "No PvP here", "testServer": true }
  ]
})";
} // namespace

TEST_CASE("ServerListJson: reads the groups, VIP types and texts in their order")
{
    const ReadResult read = Read(ValidFile);

    CHECK_MESSAGE(read.issues.empty(), Messages(read.issues));
    REQUIRE(read.list.vipTypes.size() == 2);
    CHECK(read.list.vipTypes[0].id == "gold");
    CHECK(read.list.vipTypes[0].name.Get("en") == "Gold");
    CHECK(read.list.vipTypes[0].name.Get("es") == "Oro");
    CHECK(read.list.vipTypes[1].id == "silver");

    REQUIRE(read.list.colors.size() == 1);
    CHECK(read.list.colors[0].id == "goldRed");
    CHECK(read.list.colors[0].name == "Gold red");
    CHECK(read.list.colors[0].shades.at("legacy") == "#FF5A5A");
    CHECK(read.list.colors[0].shades.at("modern") == "#C0564E");

    REQUIRE(read.list.buttonColors.size() == 3);
    CHECK(read.list.buttonColors[0].vipMatch == RuleVip::VipType);
    CHECK(read.list.buttonColors[0].vip == "gold");
    CHECK(read.list.buttonColors[0].pvp == false);
    CHECK(read.list.buttonColors[0].color == "goldRed");
    CHECK(read.list.buttonColors[1].vipMatch == RuleVip::AnyVipType);
    CHECK_FALSE(read.list.buttonColors[1].pvp.has_value());
    CHECK(read.list.buttonColors[2].vipMatch == RuleVip::NoVipType);
    CHECK(read.list.buttonColors[2].pvp == true);

    CHECK(read.list.names.nonPvp.Get("de") == "Nicht-PvP");
    CHECK(read.list.names.Template(ChannelNamePlace::Login).Get("es") == "[{name}-{channel} {pvp} {vip}]");
    // Texts the file leaves out are the built-in ones.
    const Data::ServerList::ChannelNames builtIn = Data::ServerList::GetDefaultChannelNames();
    CHECK(read.list.names.pvp == builtIn.pvp);
    CHECK(read.list.names.Template(ChannelNamePlace::Button) == builtIn.Template(ChannelNamePlace::Button));

    REQUIRE(read.list.groups.size() == 2);
    const ServerGroup& valhalla = read.list.groups[0];
    CHECK(valhalla.id == 0);
    CHECK(valhalla.name == "Valhalla");
    CHECK(valhalla.description.empty());
    CHECK_FALSE(valhalla.testServer);
    CHECK(valhalla.channels[0] == Channel(false, "gold"));
    CHECK(valhalla.channels[1] == Channel(true, ""));
    CHECK(valhalla.channels[4] == Channel(true, "gold"));
    CHECK(valhalla.channels[5] == Channel(false, ""));
    CHECK(valhalla.channels[6] == Channel(true, "silver"));
    CHECK(valhalla.channels[19] == Channel(true, ""));

    const ServerGroup& kanturu = read.list.groups[1];
    CHECK(kanturu.id == 3);
    CHECK(kanturu.description == "No PvP here");
    CHECK(kanturu.testServer);
    CHECK(std::all_of(kanturu.channels.begin(), kanturu.channels.end(),
                      [](const ChannelSettings& channel) { return channel == ChannelSettings{}; }));
}

TEST_CASE("ServerListJson: writes what it reads")
{
    const ReadResult read = Read(ValidFile);
    const std::string written = Data::ServerList::WriteServerListJson(read.list);
    const ReadResult readBack = Read(written);

    CHECK_MESSAGE(readBack.issues.empty(), Messages(readBack.issues));
    CHECK(readBack.list == read.list);
    // Channels with PvP on and no VIP type, and empty or false fields are left out.
    CHECK(written.find("\"20\"") == std::string::npos);
    CHECK(written.find("\"description\": \"\"") == std::string::npos);
    CHECK(written.find("\"testServer\": false") == std::string::npos);
    // Each rule on a line of its own, a color's shades on the line of the color.
    CHECK(written.find("\n    {\"vip\": \"gold\", \"pvp\": false, \"color\": \"goldRed\"},\n") != std::string::npos);
    CHECK(written.find(R"("shades": {"legacy": "#FF5A5A", "modern": "#C0564E"})") != std::string::npos);
    // The channels of a group are on one line, and a VIP type's names on the line of its name.
    CHECK(
        written.find(
            R"("channels": {"1": {"vip": "gold", "pvp": false}, "5": {"vip": "gold"}, "6": {"pvp": false}, "7": {"vip": "silver"}})") !=
        std::string::npos);
    CHECK(written.find(R"("name": {"en": "Gold", "es": "Oro"})") != std::string::npos);
    // Every text is written, the built-in ones too.
    CHECK(written.find(R"("screenshot": {)") != std::string::npos);
}

TEST_CASE("ServerListJson: the folders of the list's own colors are kept, without colors the list lacks")
{
    ReadResult read = Read(ValidFile);
    REQUIRE_FALSE(read.list.colors.empty());
    const std::string color = read.list.colors.front().id;
    read.list.colorFolders = {{"Load", {{"Warnings", {}, {color, "gone"}}}, {}}};
    const std::string written = Data::ServerList::WriteServerListJson(read.list);
    const ReadResult readBack = Read(written);

    CHECK_MESSAGE(readBack.issues.empty(), Messages(readBack.issues));
    REQUIRE(readBack.list.colorFolders.size() == 1);
    CHECK(readBack.list.colorFolders[0].folders[0].colors == std::vector<std::string>{color});
    // Each folder on a line of its own, before the colors.
    CHECK(written.find(R"({"name": "Load", "folders": [{"name": "Warnings", "colors": [")") != std::string::npos);
    CHECK(written.find(R"("colorFolders": [)") < written.find(R"("colors": [)"));
    // A list without folders writes none.
    read.list.colorFolders.clear();
    CHECK(Data::ServerList::WriteServerListJson(read.list).find("colorFolders") == std::string::npos);
}

TEST_CASE("ServerListJson: color folders may hold the theme colors the list uses")
{
    ReadResult read = Read(ValidFile);
    read.list.colorFolders = {{"Theme", {}, {"gray", "server-load-base", "notAColor"}}};
    const ReadResult readBack = Read(Data::ServerList::WriteServerListJson(read.list));

    CHECK_MESSAGE(readBack.issues.empty(), Messages(readBack.issues));
    REQUIRE(readBack.list.colorFolders.size() == 1);
    CHECK(readBack.list.colorFolders[0].colors == std::vector<std::string>{"gray", "server-load-base"});
    CHECK(Data::ServerList::IsThemeColorEntry("silver"));
    CHECK(Data::ServerList::IsThemeColorEntry("server-load-empty"));
    CHECK_FALSE(Data::ServerList::IsThemeColorEntry("server-load"));
    CHECK(std::string(Data::ServerList::GetThemeColorEntryToken("gray")) == "server-button-gray");
}

TEST_CASE("ServerListJson: an empty list is a valid file")
{
    const ReadResult read = Read(R"({ "formatVersion": 2, "groups": [] })");

    CHECK(read.issues.empty());
    CHECK(read.list.groups.empty());
    CHECK(read.list.vipTypes.empty());
    CHECK(read.list == ServerListData{});
    CHECK(Read(Data::ServerList::WriteServerListJson(read.list)).list == read.list);
}

TEST_CASE("ServerListJson: the file needs version 2 and a list of groups")
{
    CHECK(HasFatal(Read("not json").issues));
    CHECK(HasFatal(Read(R"({ "groups": [] })").issues));
    CHECK(HasFatal(Read(R"({ "formatVersion": 3, "groups": [] })").issues));

    const ReadResult version1 = Read(R"({ "formatVersion": 1, "groups": [ { "id": 0, "name": "A" } ] })");
    CHECK(HasIssueAbout(version1.issues, "formatVersion", "version 1 is not read", ItemDataIssueSeverity::Fatal));
    CHECK(version1.list.groups.empty());

    const ReadResult noList = Read(R"({ "formatVersion": 2, "groups": {} })");
    CHECK(HasIssueAbout(noList.issues, "groups", "not a list"));
}

TEST_CASE("ServerListJson: a group needs an id and a name")
{
    const ReadResult noId = Read(GroupFile(R"({ "name": "Valhalla" })"));
    CHECK(HasIssueAbout(noId.issues, "groups[0].id", "missing"));
    CHECK(noId.list.groups.empty());

    const ReadResult badId = Read(GroupFile(R"({ "id": 3277, "name": "Valhalla" })"));
    CHECK(HasIssueAbout(badId.issues, "groups[0].id", "from 0 to 3276"));

    const ReadResult negativeId = Read(GroupFile(R"({ "id": -1, "name": "Valhalla" })"));
    CHECK(HasFatal(negativeId.issues));

    const ReadResult noName = Read(GroupFile(R"({ "id": 0 })"));
    CHECK(HasIssueAbout(noName.issues, "groups[0].name", "missing or empty"));

    const ReadResult emptyName = Read(GroupFile(R"({ "id": 0, "name": "" })"));
    CHECK(HasIssueAbout(emptyName.issues, "groups[0].name", "missing or empty"));

    const ReadResult notAnObject = Read(GroupFile("5"));
    CHECK(HasIssueAbout(notAnObject.issues, "groups[0]", "must be an object"));
}

TEST_CASE("ServerListJson: a group id is in the list once")
{
    const ReadResult read = Read(R"({ "formatVersion": 2, "groups": [
        { "id": 1, "name": "Helheim" }, { "id": 1, "name": "Midgard" } ] })");

    CHECK(HasIssueAbout(read.issues, "groups[1].id", "more than once"));
    REQUIRE(read.list.groups.size() == 1);
    CHECK(read.list.groups[0].name == "Helheim");
}

TEST_CASE("ServerListJson: channels are numbered 1 to 20 and are objects of PvP and a VIP type")
{
    const ReadResult zero = Read(GroupFile(R"({ "id": 0, "name": "A", "channels": {"0": {}} })"));
    CHECK(HasIssueAbout(zero.issues, "groups[0].channels.0", "1 to 20"));

    const ReadResult tooHigh = Read(GroupFile(R"({ "id": 0, "name": "A", "channels": {"21": {}} })"));
    CHECK(HasIssueAbout(tooHigh.issues, "groups[0].channels.21", "1 to 20"));

    const ReadResult notANumber = Read(GroupFile(R"({ "id": 0, "name": "A", "channels": {"1a": {}} })"));
    CHECK(HasFatal(notANumber.issues));

    // A version 1 type name.
    const ReadResult typeName = Read(GroupFile(R"({ "id": 0, "name": "A", "channels": {"1": "gold"} })"));
    CHECK(HasIssueAbout(typeName.issues, "groups[0].channels.1", R"(must be an object with "pvp" and "vip")"));

    const ReadResult pvpNotABool = Read(GroupFile(R"({ "id": 0, "name": "A", "channels": {"1": {"pvp": 0}} })"));
    CHECK(HasIssueAbout(pvpNotABool.issues, "groups[0].channels.1.pvp", "true or false"));

    const ReadResult vipNotAText = Read(GroupFile(R"({ "id": 0, "name": "A", "channels": {"1": {"vip": 1}} })"));
    CHECK(HasIssueAbout(vipNotAText.issues, "groups[0].channels.1.vip", "the id of a VIP type"));

    const ReadResult notAnObject = Read(GroupFile(R"({ "id": 0, "name": "A", "channels": ["gold"] })"));
    CHECK(HasIssueAbout(notAnObject.issues, "groups[0].channels", "object"));
}

TEST_CASE("ServerListJson: a channel with a VIP type the list does not have has none")
{
    const ReadResult read =
        Read(GroupFile(R"({ "id": 0, "name": "A", "channels": {"1": {"vip": "silver", "pvp": false}} })"));

    CHECK_FALSE(HasFatal(read.issues));
    CHECK(HasIssueAbout(read.issues, "groups[0].channels.1.vip", "\"silver\" is not the id of a VIP type",
                        ItemDataIssueSeverity::Error));
    REQUIRE(read.list.groups.size() == 1);
    CHECK(read.list.groups[0].channels[0] == Channel(false, ""));
}

TEST_CASE("ServerListJson: a VIP type needs an id used once and an English name")
{
    const ReadResult noId = Read(VipTypesFile(R"({"name": "Gold"})"));
    CHECK(HasIssueAbout(noId.issues, "vipTypes[0].id", "missing", ItemDataIssueSeverity::Fatal));
    CHECK(noId.list.vipTypes.empty());

    for (const char* id : {R"("Gold")", R"("2gold")", R"("gold plus")", R"("")", "5"})
    {
        CAPTURE(id);
        const ReadResult badId = Read(VipTypesFile(std::string(R"({"id": )") + id + R"(, "name": "Gold"})"));
        CHECK(HasIssueAbout(badId.issues, "vipTypes[0].id", "lowercase letter", ItemDataIssueSeverity::Fatal));
    }

    const ReadResult twice = Read(VipTypesFile(R"({"id": "gold", "name": "Gold"}, {"id": "gold", "name": "Golden"})"));
    CHECK(HasIssueAbout(twice.issues, "vipTypes[1].id", "more than one VIP type", ItemDataIssueSeverity::Fatal));
    REQUIRE(twice.list.vipTypes.size() == 1);
    CHECK(twice.list.vipTypes[0].name.GetNeutral() == "Gold");

    const ReadResult noName = Read(VipTypesFile(R"({"id": "gold"})"));
    CHECK(HasIssueAbout(noName.issues, "vipTypes[0].name", "missing", ItemDataIssueSeverity::Fatal));

    const ReadResult noEnglish = Read(VipTypesFile(R"({"id": "gold", "name": {"es": "Oro"}})"));
    CHECK(HasIssueAbout(noEnglish.issues, "vipTypes[0].name", "English name is missing", ItemDataIssueSeverity::Fatal));

    const ReadResult notAnObject = Read(VipTypesFile("5"));
    CHECK(HasIssueAbout(notAnObject.issues, "vipTypes[0]", "must be an object"));

    const ReadResult notAList = Read(R"({ "formatVersion": 2, "vipTypes": {}, "groups": [] })");
    CHECK(HasIssueAbout(notAList.issues, "vipTypes", "a list of VIP types"));
}

// The file with the VIP type gold, these colors and these rules, and no group.
std::string ColorsFile(const std::string& colors, const std::string& rules)
{
    return R"({ "formatVersion": 2, "vipTypes": [ {"id": "gold", "name": "Gold"} ], "colors": [ )" + colors +
           R"( ], "buttonColors": [ )" + rules + R"( ], "groups": [] })";
}

TEST_CASE("ServerListJson: a color needs an id that is no theme color's and is used once")
{
    const ReadResult noId = Read(ColorsFile(R"({"name": "Red"})", ""));
    CHECK(HasIssueAbout(noId.issues, "colors[0].id", "missing", ItemDataIssueSeverity::Fatal));

    const ReadResult themeName = Read(ColorsFile(R"({"id": "orange"})", ""));
    CHECK(HasIssueAbout(themeName.issues, "colors[0].id", "the name of a theme color", ItemDataIssueSeverity::Fatal));

    const ReadResult twice = Read(ColorsFile(R"({"id": "red"}, {"id": "red"})", ""));
    CHECK(HasIssueAbout(twice.issues, "colors[1].id", "more than one color", ItemDataIssueSeverity::Fatal));

    const ReadResult notAList = Read(R"({ "formatVersion": 2, "colors": {}, "groups": [] })");
    CHECK(HasIssueAbout(notAList.issues, "colors", "a list of colors", ItemDataIssueSeverity::Fatal));
}

TEST_CASE("ServerListJson: a shade that is not #RRGGBB is not used")
{
    const ReadResult read =
        Read(ColorsFile(R"({"id": "red", "shades": {"legacy": "#FF0000", "modern": "red", "dark": "#12345"}})", ""));

    CHECK_FALSE(HasFatal(read.issues));
    CHECK(HasIssueAbout(read.issues, "colors[0].shades.modern", "not a color like #FF5A5A",
                        ItemDataIssueSeverity::Error));
    CHECK(HasIssueAbout(read.issues, "colors[0].shades.dark", "", ItemDataIssueSeverity::Error));
    REQUIRE(read.list.colors.size() == 1);
    CHECK(read.list.colors[0].shades.size() == 1);
    CHECK(read.list.colors[0].shades.at("legacy") == "#FF0000");
}

TEST_CASE("ServerListJson: a shade's theme is the theme folder in lowercase")
{
    const ReadResult read = Read(
        ColorsFile(R"({"id": "red", "shades": {"Legacy": "#FF0000", "MODERN": "#AA0000", "modern": "#110000"}})", ""));

    CHECK_FALSE(HasFatal(read.issues));
    CHECK(HasIssueAbout(read.issues, "colors[0].shades.modern", "has a shade already", ItemDataIssueSeverity::Warning));
    REQUIRE(read.list.colors.size() == 1);
    CHECK(read.list.colors[0].shades.size() == 2);
    CHECK(read.list.colors[0].shades.at("legacy") == "#FF0000");
    CHECK(read.list.colors[0].shades.at("modern") == "#AA0000");
}

TEST_CASE("ServerListJson: a rule that would color other channels is not used")
{
    const ReadResult unknownVip = Read(ColorsFile("", R"({"vip": "silver", "color": "gray"})"));
    CHECK(HasIssueAbout(unknownVip.issues, "buttonColors[0].vip", "\"silver\" is not the id of a VIP type",
                        ItemDataIssueSeverity::Error));
    CHECK(unknownVip.list.buttonColors.empty());

    const ReadResult badVip = Read(ColorsFile("", R"({"vip": 1, "color": "gray"})"));
    CHECK(HasIssueAbout(badVip.issues, "buttonColors[0].vip", "true or false", ItemDataIssueSeverity::Error));

    const ReadResult badPvp = Read(ColorsFile("", R"({"pvp": "on", "color": "gray"})"));
    CHECK(HasIssueAbout(badPvp.issues, "buttonColors[0].pvp", "true or false", ItemDataIssueSeverity::Error));
    CHECK(badPvp.list.buttonColors.empty());

    const ReadResult same =
        Read(ColorsFile("", R"({"vip": "gold", "color": "orange"}, {"vip": "gold", "color": "gray"})"));
    CHECK(HasIssueAbout(same.issues, "buttonColors[1]", "the same channels as an earlier rule",
                        ItemDataIssueSeverity::Error));
    REQUIRE(same.list.buttonColors.size() == 1);
    CHECK(same.list.buttonColors[0].color == "orange");

    const ReadResult notAList = Read(R"({ "formatVersion": 2, "buttonColors": {}, "groups": [] })");
    CHECK(HasIssueAbout(notAList.issues, "buttonColors", "a list of button color rules", ItemDataIssueSeverity::Fatal));
}

TEST_CASE("ServerListJson: a rule's unknown color is the default color")
{
    const ReadResult read =
        Read(ColorsFile(R"({"id": "red"})", R"({"vip": "gold", "color": "purple"}, {"pvp": true}, {"color": "red"})"));

    CHECK_FALSE(HasFatal(read.issues));
    CHECK(HasIssueAbout(read.issues, "buttonColors[0].color", "not a theme color", ItemDataIssueSeverity::Warning));
    CHECK(HasIssueAbout(read.issues, "buttonColors[1].color", "not a theme color", ItemDataIssueSeverity::Warning));
    REQUIRE(read.list.buttonColors.size() == 3);
    CHECK(read.list.buttonColors[0].color == "default");
    CHECK(read.list.buttonColors[1].color == "default");
    CHECK(read.list.buttonColors[2].color == "red");
}

TEST_CASE("ServerListJson: a text without its English one has the built-in English text")
{
    const ReadResult read = Read(NamesFile(R"({"nonPvp": {"es": "no PvP"}, "pvp": ""})"));

    CHECK_MESSAGE(read.issues.empty(), Messages(read.issues));
    CHECK(read.list.names.nonPvp.GetNeutral() == "Non-PvP");
    CHECK(read.list.names.nonPvp.Get("es") == "no PvP");
    // An English text given empty stays empty.
    CHECK(read.list.names.pvp.GetNeutral().empty());
}

TEST_CASE("ServerListJson: a template shows what it cannot fill as written, an error")
{
    const ReadResult unknown = Read(NamesFile(R"({"login": {"en": "[{name} {load}]", "es": "[{vipName}]"}})"));
    CHECK_FALSE(HasFatal(unknown.issues));
    CHECK(HasIssueAbout(unknown.issues, "names.login.en", "{load} is not a placeholder of the login template",
                        ItemDataIssueSeverity::Error));
    CHECK(HasIssueAbout(unknown.issues, "names.login.es", "{vipName}", ItemDataIssueSeverity::Error));
    CHECK(unknown.list.names.Template(ChannelNamePlace::Login).GetNeutral() == "[{name} {load}]");

    // The button has {load}, the screenshot message {character}.
    const ReadResult known = Read(NamesFile(R"({"button": "{name} {load}", "screenshot": "{name} / {character}"})"));
    CHECK_MESSAGE(known.issues.empty(), Messages(known.issues));

    const ReadResult braces = Read(NamesFile(R"({"characterInfo": "[{name"})"));
    CHECK(HasIssueAbout(braces.issues, "names.characterInfo.en", "", ItemDataIssueSeverity::Error));
}

TEST_CASE("ServerListJson: texts need language codes and are cut when too long")
{
    const ReadResult badLocale = Read(NamesFile(R"({"pvp": {"en": "PvP", "e s": "x"}})"));
    CHECK(HasIssueAbout(badLocale.issues, "names.pvp.e s", "not a language code", ItemDataIssueSeverity::Fatal));

    const std::string longText(300, 'x');
    const ReadResult tooLong =
        Read(NamesFile(R"({"pvp": {"en": ")" + longText + R"(", "de": ")" + longText + R"("}})"));
    CHECK_FALSE(HasFatal(tooLong.issues));
    CHECK(HasIssueAbout(tooLong.issues, "names.pvp.en", "longer than 254 characters"));
    CHECK(HasIssueAbout(tooLong.issues, "names.pvp.de", "longer than 254 characters"));
    CHECK(tooLong.list.names.pvp.GetNeutral().size() == 254);
    CHECK(tooLong.list.names.pvp.Get("de").size() == 254);

    const ReadResult longVipName = Read(VipTypesFile(R"({"id": "gold", "name": ")" + std::string(40, 'g') + R"("})"));
    CHECK(HasIssueAbout(longVipName.issues, "vipTypes[0].name.en", "longer than 32 characters"));
    REQUIRE(longVipName.list.vipTypes.size() == 1);
    CHECK(longVipName.list.vipTypes[0].name.GetNeutral() == std::string(32, 'g'));

    const ReadResult notAnObject = Read(NamesFile("[]"));
    CHECK(HasIssueAbout(notAnObject.issues, "names", "an object of texts"));
}

TEST_CASE("ServerListJson: a translation in a language the game does not have is a warning")
{
    const ReadResult read = Read(ValidFile);
    std::vector<ItemDataIssue> issues;
    const std::array<const char*, 2> locales = {"en", "es"};
    Data::ServerList::ValidateServerListLocales(read.list, locales, "ServerList.json", issues);

    CHECK(HasIssueAbout(issues, "names.nonPvp.de", "no language \"de\"", ItemDataIssueSeverity::Warning));
    CHECK_FALSE(HasIssueAbout(issues, "vipTypes[0].name.es", ""));
    CHECK(issues.size() == 1);
}

TEST_CASE("ServerListJson: a name or description that is too long is cut")
{
    const std::string longName(40, 'x');
    const std::string longDescription(300, 'y');
    const ReadResult read =
        Read(GroupFile(R"({ "id": 0, "name": ")" + longName + R"(", "description": ")" + longDescription + R"(" })"));

    CHECK_FALSE(HasFatal(read.issues));
    CHECK(HasIssueAbout(read.issues, "groups[0].name", "longer than 32 characters"));
    CHECK(HasIssueAbout(read.issues, "groups[0].description", "longer than 254 characters"));
    REQUIRE(read.list.groups.size() == 1);
    CHECK(read.list.groups[0].name == std::string(32, 'x'));
    CHECK(read.list.groups[0].description.size() == 254);
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

    REQUIRE(read.list.groups.size() == 1);
    CHECK(read.list.groups[0].name == name.substr(0, 64));
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
    REQUIRE(onlyEmoji.list.groups.size() == 1);
    CHECK(onlyEmoji.list.groups[0].name == emojis.substr(0, 16 * emoji.size()));

    // 31 letters and an emoji are 33 units: the emoji goes, whole.
    const std::string mixed = std::string(31, 'x') + emoji;
    const ReadResult cut = Read(GroupFile(R"({ "id": 0, "name": ")" + mixed + R"(" })"));
    REQUIRE(cut.list.groups.size() == 1);
    CHECK(cut.list.groups[0].name == std::string(31, 'x'));
    CHECK(HasIssueAbout(cut.issues, "groups[0].name", "an emoji counts as two"));

    // 30 letters and an emoji are exactly 32.
    std::string fits = std::string(30, 'x') + emoji;
    CHECK_FALSE(Core::Text::CutToUtf16Length(fits, Data::ServerList::MaxServerGroupNameLength));
    CHECK(fits == std::string(30, 'x') + emoji);
}

TEST_CASE("ServerListJson: unknown fields and a second test server are warnings")
{
    const ReadResult read = Read(R"({ "formatVersion": 2, "color": "red",
        "vipTypes": [ {"id": "gold", "name": "Gold", "level": 2} ],
        "names": { "title": "x" },
        "groups": [
        { "id": 0, "name": "A", "testServer": true, "order": 3, "channels": {"1": {"vip": "gold", "gold": true}} },
        { "id": 1, "name": "B", "testServer": true } ] })");

    CHECK_FALSE(HasFatal(read.issues));
    CHECK(HasWarnings(read.issues));
    CHECK(HasIssueAbout(read.issues, "color", "unknown field"));
    CHECK(HasIssueAbout(read.issues, "vipTypes[0].level", "unknown field"));
    CHECK(HasIssueAbout(read.issues, "names.title", "unknown field"));
    CHECK(HasIssueAbout(read.issues, "groups[0].order", "unknown field"));
    CHECK(HasIssueAbout(read.issues, "groups[0].channels.1.gold", "unknown field"));
    CHECK(HasIssueAbout(read.issues, "testServer", "only the first test server group"));
    CHECK(read.list.groups.size() == 2);
}

TEST_CASE("ServerListJson: wrong types are errors")
{
    CHECK(HasIssueAbout(Read(GroupFile(R"({ "id": 0, "name": 5 })")).issues, "groups[0].name", "must be a text"));
    CHECK(HasIssueAbout(Read(GroupFile(R"({ "id": 0, "name": "A", "testServer": 1 })")).issues, "groups[0].testServer",
                        "true or false"));
    CHECK(HasIssueAbout(Read(GroupFile(R"({ "id": "0", "name": "A" })")).issues, "groups[0].id", "whole number"));
}

// The file with these colors and this load section, and no group.
std::string LoadFile(const std::string& load)
{
    return R"({ "formatVersion": 2,
        "colors": [ {"id": "loadRed", "name": "Load red", "shades": {"legacy": "#FF5A5A"}} ],
        "load": )" +
           load + R"(, "groups": [] })";
}

TEST_CASE("ServerListJson: the load section gives the bar's colors and the load texts' look")
{
    const ReadResult read = Read(LoadFile(R"({
        "bar": {"base": "default", "full": "loadRed", "empty": "gray"},
        "full": {"text": {"en": "Full!", "de": "Voll!"}, "color": "loadRed", "bold": true},
        "overloaded": {"text": "Busy", "italic": true} })"));

    CHECK_MESSAGE(read.issues.empty(), Messages(read.issues));
    const Data::ServerList::LoadSettings& load = read.list.load;
    CHECK(load.bar.base == "default");
    CHECK(load.bar.full == "loadRed");
    CHECK(load.bar.empty == "gray");
    CHECK(load.full.text.Get("de") == "Voll!");
    CHECK(load.full.text.GetNeutral() == "Full!");
    CHECK(load.full.color == "loadRed");
    CHECK(load.full.bold);
    CHECK_FALSE(load.full.italic);
    CHECK(load.overloaded.text.GetNeutral() == "Busy");
    CHECK(load.overloaded.color == "default");
    CHECK(load.overloaded.italic);

    // Written and read again, it is the same.
    const ReadResult again = Read(Data::ServerList::WriteServerListJson(read.list));
    CHECK_MESSAGE(again.issues.empty(), Messages(again.issues));
    CHECK(again.list.load == load);
}

TEST_CASE("ServerListJson: without a load section, the built-in texts and the theme's bar")
{
    const ReadResult read = Read(NamesFile("{}"));
    CHECK(read.list.load == Data::ServerList::GetDefaultLoadSettings());
    CHECK(read.list.load.full.text.GetNeutral() == "(Full)");
    CHECK(read.list.load.overloaded.text.GetNeutral() == "Overloaded");
}

TEST_CASE("ServerListJson: a load text without English keeps the built-in English one")
{
    const ReadResult read = Read(LoadFile(R"x({ "full": {"text": {"de": "(Voll)"}} })x"));
    CHECK(read.issues.empty());
    CHECK(read.list.load.full.text.GetNeutral() == "(Full)");
    CHECK(read.list.load.full.text.Get("de") == "(Voll)");
}

TEST_CASE("ServerListJson: a color the list does not have is a warning and the default color")
{
    const ReadResult read =
        Read(LoadFile(R"({ "bar": {"base": "nope"}, "overloaded": {"color": 5}, "full": {"size": 2} })"));
    CHECK_FALSE(HasFatal(read.issues));
    CHECK(HasIssueAbout(read.issues, "load.bar.base", "not a theme color", ItemDataIssueSeverity::Warning));
    CHECK(HasIssueAbout(read.issues, "load.overloaded.color", "not a theme color", ItemDataIssueSeverity::Warning));
    CHECK(HasIssueAbout(read.issues, "load.full.size", "unknown field"));
    CHECK(read.list.load.bar.base == "default");
    CHECK(read.list.load.overloaded.color == "default");
}

TEST_CASE("ServerListJson: a load section of the wrong shape is fatal, a long load text cut")
{
    CHECK(HasIssueAbout(Read(LoadFile("[]")).issues, "load", "must be an object"));
    CHECK(HasIssueAbout(Read(LoadFile(R"({ "bar": "red" })")).issues, "load.bar", "must be an object"));
    CHECK(HasIssueAbout(Read(LoadFile(R"x({ "full": "(Full)" })x")).issues, "load.full", "must be an object"));
    CHECK(HasIssueAbout(Read(LoadFile(R"({ "full": {"bold": "yes"} })")).issues, "load.full.bold", "true or false"));

    const ReadResult cut = Read(LoadFile(R"({ "full": {"text": ")" + std::string(40, 'x') + R"("} })"));
    CHECK(HasIssueAbout(cut.issues, "load.full.text.en", "longer than 32", ItemDataIssueSeverity::Warning));
    CHECK(cut.list.load.full.text.GetNeutral() == std::string(32, 'x'));
}

TEST_CASE("ServerListJson: the shipped file reads without problems and is written as the editor writes it")
{
    // MU_TEST_DATA_DIR is the client folder's Data; the file name starts with it.
    const std::filesystem::path file =
        std::filesystem::path(MU_TEST_DATA_DIR).parent_path() / Data::ServerList::ServerListFile;
    const Data::ServerList::ServerListLoadResult loaded = Data::ServerList::LoadServerListFile(file, GameLocales);

    CHECK_MESSAGE(loaded.issues.empty(), Messages(loaded.issues));
    REQUIRE(loaded.list.groups.size() == 3);
    CHECK(loaded.list.groups[0].name == "Valhalla");
    CHECK(loaded.list.groups[1].name == "Helheim");
    CHECK(loaded.list.groups[2].name == "Midgard");
    // Channels 1 to 4 Gold with PvP off, 5 Gold with PvP, 6 to 8 PvP off, the rest PvP.
    for (const ServerGroup& group : loaded.list.groups)
    {
        CAPTURE(group.name);
        CHECK(group.channels[0] == Channel(false, "gold"));
        CHECK(group.channels[3] == Channel(false, "gold"));
        CHECK(group.channels[4] == Channel(true, "gold"));
        CHECK(group.channels[5] == Channel(false, ""));
        CHECK(group.channels[7] == Channel(false, ""));
        CHECK(group.channels[8] == Channel(true, ""));
    }
    REQUIRE(loaded.list.vipTypes.size() == 1);
    // Gold channels orange, channels without a VIP type gray with PvP on.
    REQUIRE(loaded.list.buttonColors.size() == 2);
    CHECK(loaded.list.buttonColors[0].vip == "gold");
    CHECK(loaded.list.buttonColors[0].color == "orange");
    CHECK(loaded.list.buttonColors[1].vipMatch == RuleVip::NoVipType);
    CHECK(loaded.list.buttonColors[1].color == "gray");
    // Both load texts bold in the list's red, "Overloaded" also italic, in every language; the
    // bar in each theme's own colors.
    const Data::ServerList::LoadSettings& load = loaded.list.load;
    CHECK(load.bar == Data::ServerList::LoadBarColors{});
    CHECK(load.full.color == "loadRed");
    CHECK(load.full.bold);
    CHECK(load.overloaded.italic);
    CHECK(load.full.text.GetNeutral() == "(Full)");
    CHECK(load.overloaded.text.GetNeutral() == "Overloaded");
    for (const char* locale : GameLocales)
    {
        CAPTURE(locale);
        if (std::string_view(locale) != Data::LocalizedString::NeutralLocale)
        {
            CHECK(load.full.text.HasOwn(locale));
            CHECK(load.overloaded.text.HasOwn(locale));
        }
    }

    const std::optional<std::string> text = Data::Items::Json::ReadTextFile(file);
    REQUIRE(text.has_value());
    CHECK(Data::ServerList::WriteServerListJson(loaded.list) == *text);
}

TEST_CASE("ServerListJson: a missing file is an error")
{
    const Data::ServerList::ServerListLoadResult loaded =
        Data::ServerList::LoadServerListFile("does/not/exist/ServerList.json", GameLocales);

    CHECK(HasIssueAbout(loaded.issues, "", "could not be read"));
    CHECK(loaded.list.groups.empty());
}

TEST_CASE("ServerListJson: save writes the file and refuses a list with errors")
{
    const std::filesystem::path folder = std::filesystem::temp_directory_path() / "mu_server_list_save_test";
    std::filesystem::remove_all(folder);
    const std::filesystem::path file = folder / "Local" / "ServerList.json";
    const ReadResult read = Read(ValidFile);

    std::vector<ItemDataIssue> issues;
    REQUIRE(Data::ServerList::SaveServerListFile(file, read.list, issues));
    CHECK(issues.empty());
    const Data::ServerList::ServerListLoadResult saved = Data::ServerList::LoadServerListFile(file, GameLocales);
    CHECK(saved.issues.empty());
    CHECK(saved.list == read.list);

    ServerListData broken = read.list;
    broken.groups[1].name.clear();
    broken.groups.push_back(read.list.groups[0]);
    broken.vipTypes.push_back(read.list.vipTypes[0]);
    std::vector<ItemDataIssue> refused;
    CHECK_FALSE(Data::ServerList::SaveServerListFile(file, broken, refused));
    CHECK(HasIssueAbout(refused, "groups[1].name", "missing or empty"));
    CHECK(HasIssueAbout(refused, "groups[2].id", "more than once"));
    CHECK(HasIssueAbout(refused, "vipTypes[2].id", "more than one VIP type"));
    // The file stays as it was.
    CHECK(Data::ServerList::LoadServerListFile(file, GameLocales).list == read.list);
    CHECK_FALSE(std::filesystem::exists(file.string() + ".tmp"));

    std::filesystem::remove_all(folder);
}

TEST_CASE("ServerListJson: a save that cannot write or replace the file names it and leaves no temporary file")
{
    const std::filesystem::path folder = std::filesystem::temp_directory_path() / "mu_server_list_blocked_test";
    std::filesystem::remove_all(folder);
    std::filesystem::create_directories(folder);
    const ServerListData list = Read(ValidFile).list;

    // A file where the Local folder should be: nothing can be written there.
    const std::filesystem::path blocked = folder / "Local";
    std::ofstream(blocked) << "not a folder";
    const std::filesystem::path unwritable = blocked / "ServerList.json";
    std::vector<ItemDataIssue> issues;
    CHECK_FALSE(Data::ServerList::SaveServerListFile(unwritable, list, issues));
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].source == unwritable.generic_string());
    CHECK(issues[0].message == "could not be written");
    CHECK_FALSE(std::filesystem::exists(unwritable.string() + ".tmp"));

    // A folder with the file's name: the written file cannot take its place.
    const std::filesystem::path irreplaceable = folder / "ServerList.json";
    std::filesystem::create_directories(irreplaceable / "inside");
    issues.clear();
    CHECK_FALSE(Data::ServerList::SaveServerListFile(irreplaceable, list, issues));
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].source == irreplaceable.generic_string());
    CHECK(issues[0].message.starts_with("could not be replaced: "));
    CHECK_FALSE(std::filesystem::exists(irreplaceable.string() + ".tmp"));

    std::filesystem::remove_all(folder);
}
