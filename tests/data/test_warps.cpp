#include "stdafx.h"

#include "doctest.h"

#include "Data/DataHandler/MapData/WarpStorage.h"
#include "Data/DataHandler/TextFile.h"
#include "Data/GameData/MapData/WarpsJson.h"
#include "GameLogic/Warps/ServerWarpList.h"
#include "I18N/All.h"
#include "Network/MoveCommandData.h"

#include <algorithm>
#include <filesystem>
#include <optional>
#include <string>
#include <utility>
#include <vector>

using Data::Items::HasFatal;
using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using Data::Maps::Warp;
using Data::Maps::WarpList;
using SEASON3B::CMoveCommandData;

namespace
{
struct ReadResult
{
    WarpList list;
    std::vector<ItemDataIssue> issues;
};

ReadResult Read(const std::string& text)
{
    ReadResult result;
    Data::Maps::ReadWarpsJson(text, "Warps.json", result.list, result.issues);
    Data::Maps::ValidateWarps(result.list, "Warps.json", result.issues);
    return result;
}

// Puts back the M window's list, the language of its names, whether they follow the UI language and the UI
// language as the test found them: the list is the game's for the whole run.
class MoveListScope
{
public:
    MoveListScope()
        : m_list(CMoveCommandData::GetInstance()->GetDataFileWarps()),
          m_nameLocale(CMoveCommandData::GetInstance()->GetWarpNameLocale()),
          m_uiLocale(I18N::GetCurrentLocale()), m_followed(CMoveCommandData::GetInstance()->FollowsUiLanguage())
    {
    }

    ~MoveListScope()
    {
        CMoveCommandData& data = *CMoveCommandData::GetInstance();
        if (!m_followed)
        {
            data.StopFollowingUiLanguage();
        }
        I18N::SetLocale(m_uiLocale.c_str());
        data.RestoreDataFileWarpList();
        data.SetDataFileWarps(m_list, m_nameLocale);
    }

    MoveListScope(const MoveListScope&) = delete;
    MoveListScope& operator=(const MoveListScope&) = delete;

private:
    WarpList m_list;
    std::string m_nameLocale;
    std::string m_uiLocale;
    bool m_followed;
};

std::string File(const std::string& warps)
{
    return R"({ "formatVersion": 1, "warps": [ )" + warps + " ] }";
}

constexpr const char* Icarus = R"({"index": 23, "name": {"en": "Icarus", "es": "Ícaro"}, "command": "Icarus",)"
                               R"( "level": 170, "zen": 10000, "gate": 63})";

std::string Messages(const std::vector<ItemDataIssue>& issues)
{
    std::string text;
    for (const ItemDataIssue& issue : issues)
    {
        text += issue.ToString() + "\n";
    }
    return text;
}

bool HasIssueAbout(const std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const std::string& field)
{
    return std::any_of(issues.begin(), issues.end(),
                       [&](const ItemDataIssue& issue) { return issue.severity == severity && issue.field == field; });
}

std::filesystem::path ShippedFile()
{
    return std::filesystem::path(MU_TEST_DATA_DIR) / "Maps" / "Warps.json";
}
} // namespace

TEST_CASE("Warps: a warp reads with its names, command, level, zen and gate [data][warps]")
{
    const ReadResult result = Read(File(Icarus));
    INFO(Messages(result.issues));
    CHECK(result.issues.empty());
    REQUIRE(result.list.warps.size() == 1);
    const Warp& warp = result.list.warps[0];
    CHECK(warp.index == 23);
    CHECK(warp.names.Get("en") == "Icarus");
    CHECK(warp.names.Get("es") == "Ícaro");
    CHECK(warp.names.Get("pt") == "Icarus"); // falls back to English
    CHECK(warp.command == "Icarus");
    CHECK(warp.level == 170);
    CHECK(warp.zen == 10000);
    CHECK(warp.gate == 63);
}

TEST_CASE("Warps: wrong values are fatal and name the warp and field [data][warps]")
{
    const auto fatalAt = [](const std::string& warp, const std::string& field)
    {
        const ReadResult result = Read(File(warp));
        INFO(Messages(result.issues));
        CHECK(result.list.warps.empty());
        CHECK(HasIssueAbout(result.issues, ItemDataIssueSeverity::Fatal, field));
    };
    fatalAt(R"({"name": "Arena", "command": "Arena", "level": 1, "zen": 1, "gate": 1})", "warps[0].index");
    fatalAt(R"({"index": 65536, "name": "Arena", "command": "Arena", "level": 1, "zen": 1, "gate": 1})",
            "warps[0].index");
    fatalAt(R"({"index": 1, "command": "Arena", "level": 1, "zen": 1, "gate": 1})", "warps[0].name");
    fatalAt(R"({"index": 1, "name": {"pt": "Arena"}, "command": "Arena", "level": 1, "zen": 1, "gate": 1})",
            "warps[0].name");
    fatalAt(R"({"index": 1, "name": "Arena", "command": "", "level": 1, "zen": 1, "gate": 1})", "warps[0].command");
    fatalAt(R"({"index": 1, "name": "Arena", "command": "Arena", "level": -1, "zen": 1, "gate": 1})", "warps[0].level");
    fatalAt(R"({"index": 1, "name": "Arena", "command": "Arena", "level": 1, "zen": "1", "gate": 1})", "warps[0].zen");
    fatalAt(R"({"index": 1, "name": "Arena", "command": "Arena", "level": 1, "zen": 1})", "warps[0].gate");
}

TEST_CASE("Warps: an unknown field is a warning, the warp is kept [data][warps]")
{
    const ReadResult result =
        Read(File(R"({"index": 1, "name": "Arena", "command": "Arena", "level": 1, "zen": 1, "gate": 1, "x": 2})"));
    CHECK_FALSE(HasFatal(result.issues));
    CHECK(HasIssueAbout(result.issues, ItemDataIssueSeverity::Warning, "warps[0].x"));
    CHECK(result.list.warps.size() == 1);
}

TEST_CASE("Warps: two warps with one index are fatal, a name longer than the game holds a warning [data][warps]")
{
    const ReadResult twice = Read(File(std::string(Icarus) + ", " + Icarus));
    CHECK(HasIssueAbout(twice.issues, ItemDataIssueSeverity::Fatal, "warps[1].index"));

    const std::string longName(32, 'a');
    const ReadResult longOne = Read(File(R"({"index": 1, "name": {"en": "Arena", "pt": ")" + longName +
                                         R"("}, "command": "Arena", "level": 1, "zen": 1, "gate": 1})"));
    INFO(Messages(longOne.issues));
    CHECK_FALSE(HasFatal(longOne.issues));
    CHECK(HasIssueAbout(longOne.issues, ItemDataIssueSeverity::Warning, "warps[0].name.pt"));
    CHECK_FALSE(HasIssueAbout(longOne.issues, ItemDataIssueSeverity::Warning, "warps[0].name.en"));
}

TEST_CASE("Warps: the shipped file reads without problems and writes back as it is [data][warps]")
{
    const Data::Maps::WarpsLoadResult result = Data::Maps::LoadWarpsFile(ShippedFile());
    INFO(Messages(result.issues));
    CHECK(result.issues.empty());
    CHECK(result.list.warps.size() == 37);

    const std::optional<std::string> text = Data::ReadTextFile(ShippedFile());
    REQUIRE(text);
    std::string lf = *text;
    lf.erase(std::remove(lf.begin(), lf.end(), '\r'), lf.end());
    CHECK(Data::Maps::WriteWarpsJson(result.list) == lf);
}

TEST_CASE("Warps: the names follow the UI language, English where a warp has none, live [data][warps]")
{
    const ReadResult result = Read(File(std::string(Icarus) + R"(, {"index": 8, "name": {"en": "Dungeon", "es": )"
                                        R"("Calabozo", "pt": "Masmorra"}, "command": "Dungeon", "level": 30,)"
                                        R"( "zen": 3000, "gate": 2})"));
    REQUIRE(result.list.warps.size() == 2);
    const MoveListScope scope;
    CMoveCommandData& data = *CMoveCommandData::GetInstance();
    const auto names = [&]
    {
        std::vector<std::wstring> shown;
        for (const auto* entry : data.GetMoveCommandDatalist())
        {
            shown.push_back(std::wstring(entry->_ReqInfo.szMainMapName) + L"/" + entry->_ReqInfo.szSubMapName);
        }
        return shown;
    };

    I18N::SetLocale("en");
    data.SetDataFileWarps(result.list, I18N::GetCurrentLocale());
    data.FollowUiLanguage();
    CHECK(names() == std::vector<std::wstring>{L"Icarus/Icarus", L"Dungeon/Dungeon"});

    const uint32_t version = data.GetListVersion();
    I18N::SetLocale("es");
    CHECK(data.GetListVersion() != version);
    // The /move name stays English.
    CHECK(names() == std::vector<std::wstring>{L"Ícaro/Icarus", L"Calabozo/Dungeon"});

    I18N::SetLocale("de");
    CHECK(names() == std::vector<std::wstring>{L"Icarus/Icarus", L"Dungeon/Dungeon"});

    // The server's list: its levels stay, a warp only it has keeps its name.
    std::vector<GameLogic::Warps::ServerWarp> serverWarps{{8, 1, 35, 3000, L"Dungeon"}, {26, 34, 190, 1, L"Crywolf"}};
    data.ApplyServerWarpList(serverWarps);
    I18N::SetLocale("pt");
    CHECK(names() == std::vector<std::wstring>{L"Masmorra/Dungeon", L"Crywolf/Crywolf"});
    CHECK(data.GetMoveCommandDataByIndex(8)->_ReqInfo.iReqLevel == 35);

    data.RestoreDataFileWarpList();
    CHECK(names() == std::vector<std::wstring>{L"Icarus/Icarus", L"Masmorra/Dungeon"});
    I18N::SetLocale("en");
    CHECK(names() == std::vector<std::wstring>{L"Icarus/Icarus", L"Dungeon/Dungeon"});
}

// OpenMU sends the master level and resets (WarpRequirementsExtended) right after its list, only for the warps
// which need them.
TEST_CASE("Warps: the server's master level and resets apply to its list until it is restored [data][warps]")
{
    const ReadResult result = Read(File(R"({"index": 23, "name": "Icarus", "command": "Icarus", "level": 170,)"
                                        R"( "zen": 10000, "gate": 63, "minMasterLevel": 5, "minResets": 2},)"
                                        R"( {"index": 8, "name": "Dungeon", "command": "Dungeon", "level": 30,)"
                                        R"( "zen": 3000, "gate": 2})"));
    REQUIRE(result.list.warps.size() == 2);
    const MoveListScope scope;
    CMoveCommandData& data = *CMoveCommandData::GetInstance();
    data.SetDataFileWarps(result.list, "en");
    const auto minimums = [&](int index)
    {
        const auto* entry = data.GetMoveCommandDataByIndex(index);
        REQUIRE(entry != nullptr);
        return std::pair{entry->_ReqInfo.iReqMasterLevel, entry->_ReqInfo.iReqResets};
    };
    CHECK(minimums(23) == std::pair{5, 2});

    // Without a list of the server there is nothing to apply them to.
    data.ApplyServerWarpRequirements({{23, 9, 9}});
    CHECK(minimums(23) == std::pair{5, 2});

    // The server's list counts, like its level and zen: until the requirements arrive, no warp needs any.
    data.ApplyServerWarpList({{23, 10, 170, 10000, L"Icarus"}, {8, 1, 30, 3000, L"Dungeon"}, {300, 79, 1, 0, L"Market"}});
    CHECK(minimums(23) == std::pair{0, 0});
    CHECK(minimums(8) == std::pair{0, 0});

    // A warp without an entry needs none; an entry for a warp the list doesn't have changes nothing.
    data.ApplyServerWarpRequirements({{23, 0, 7}, {300, 40, 0}, {999, 1, 1}, {23, 3, 3}});
    CHECK(minimums(23) == std::pair{0, 7});
    CHECK(minimums(8) == std::pair{0, 0});
    CHECK(minimums(300) == std::pair{40, 0});
    CHECK(data.GetMoveCommandDataByIndex(999) == nullptr);

    // A new list of the server comes without requirements until its own message follows.
    data.ApplyServerWarpList({{23, 10, 170, 10000, L"Icarus"}, {300, 79, 1, 0, L"Market"}});
    CHECK(minimums(23) == std::pair{0, 0});
    CHECK(minimums(300) == std::pair{0, 0});
    data.ApplyServerWarpRequirements({{300, 40, 0}});

    data.RestoreDataFileWarpList();
    CHECK(minimums(23) == std::pair{5, 2});
    CHECK(minimums(8) == std::pair{0, 0});
    CHECK(data.GetMoveCommandDataByIndex(300) == nullptr);
}

TEST_CASE("Warps: the M window's entry has the name of its language, the command as /move name [data][warps]")
{
    const ReadResult result = Read(File(Icarus));
    REQUIRE(result.list.warps.size() == 1);
    const auto spanish = CMoveCommandData::MakeMoveReqInfo(result.list.warps[0], "es");
    CHECK(std::wstring(spanish.szMainMapName) == L"Ícaro");
    CHECK(std::wstring(spanish.szSubMapName) == L"Icarus");
    CHECK(spanish.index == 23);
    CHECK(spanish.iReqLevel == 170);
    CHECK(spanish.iReqZen == 10000);
    CHECK(spanish.iGateNum == 63);
    CHECK(spanish.iReqMasterLevel == 0);
    CHECK(spanish.iReqResets == 0);
    const auto portuguese = CMoveCommandData::MakeMoveReqInfo(result.list.warps[0], "pt");
    CHECK(std::wstring(portuguese.szMainMapName) == L"Icarus");
}

TEST_CASE("Warps: the move rules read and write back [data][warps]")
{
    const std::string text =
        R"({ "formatVersion": 1, "classLevelDiscount": [{"class": "darkLord", "numerator": 2, "denominator": 3}],)"
        R"( "warps": [ {"index": 23, "name": "Icarus", "command": "Icarus", "level": 170, "zen": 10000, "gate": 63,)"
        R"( "noClassDiscount": true, "minMasterLevel": 5, "minResets": 2, "pvpServersOnly": true, "gensOnly": true,)"
        R"( "needsOneOf": [{"tag": "flying"}, {"wings": true}],)"
        R"( "notWith": [{"item": {"group": 13, "number": 2}},)"
        R"( {"tag": "transformationRing", "except": {"tag": "pandaOrSkeleton"}}],)"
        R"( "blockedMessage": "YouCanEnterIcarusOnlyWithWingsDinorantFenrirr"} ] })";
    const ReadResult result = Read(text);
    INFO(Messages(result.issues));
    CHECK(result.issues.empty());
    REQUIRE(result.list.warps.size() == 1);
    REQUIRE(result.list.classLevelDiscounts.size() == 1);
    CHECK(result.list.classLevelDiscounts[0] == Data::Maps::ClassLevelDiscount{CLASS_DARK_LORD, 2, 3});
    const Warp& warp = result.list.warps[0];
    CHECK(warp.noClassDiscount);
    CHECK(warp.minMasterLevel == 5);
    CHECK(warp.minResets == 2);
    CHECK(warp.pvpServersOnly);
    CHECK(warp.gensOnly);
    REQUIRE(warp.needsOneOf.size() == 2);
    CHECK(warp.needsOneOf[0].kind == Data::Maps::WarpItemMatch::Kind::Tag);
    CHECK(warp.needsOneOf[0].tag == "flying");
    CHECK(warp.needsOneOf[1].kind == Data::Maps::WarpItemMatch::Kind::Wings);
    REQUIRE(warp.notWith.size() == 2);
    CHECK(warp.notWith[0].kind == Data::Maps::WarpItemMatch::Kind::Item);
    CHECK(warp.notWith[0].group == 13);
    CHECK(warp.notWith[0].number == 2);
    CHECK(warp.notWith[1].tag == "transformationRing");
    CHECK(warp.notWith[1].exceptTag == "pandaOrSkeleton");
    // The tags are values once read, so the rules look no names up.
    CHECK(warp.needsOneOf[0].tagValue == Data::Items::ItemTag::Flying);
    CHECK_FALSE(warp.needsOneOf[0].exceptTagValue.has_value());
    CHECK(warp.notWith[1].tagValue == Data::Items::ItemTag::TransformationRing);
    CHECK(warp.notWith[1].exceptTagValue == Data::Items::ItemTag::PandaOrSkeleton);
    CHECK(warp.blockedMessage == "YouCanEnterIcarusOnlyWithWingsDinorantFenrirr");

    const ReadResult again = Read(Data::Maps::WriteWarpsJson(result.list));
    CHECK(again.issues.empty());
    CHECK(again.list == result.list);

    // Without rules, a warp writes none of their fields.
    const ReadResult plain = Read(File(Icarus));
    const std::string written = Data::Maps::WriteWarpsJson(plain.list);
    CHECK(written.find("noClassDiscount") == std::string::npos);
    CHECK(written.find("classLevelDiscount") == std::string::npos);
    CHECK(written.find("needsOneOf") == std::string::npos);
    CHECK(written.find("minResets") == std::string::npos);
}

TEST_CASE("Warps: wrong values of the move rules are fatal [data][warps]")
{
    const std::string base = R"({"index": 1, "name": "Arena", "command": "Arena", "level": 1, "zen": 1, "gate": 1, )";
    const auto fatalAt = [&](const std::string& rule, const std::string& field)
    {
        const ReadResult result = Read(File(base + rule + "}"));
        INFO(rule << "\n" << Messages(result.issues));
        CHECK(result.list.warps.empty());
        CHECK(HasIssueAbout(result.issues, ItemDataIssueSeverity::Fatal, field));
    };
    fatalAt(R"("noClassDiscount": 1)", "warps[0].noClassDiscount");
    fatalAt(R"("pvpServersOnly": "yes")", "warps[0].pvpServersOnly");
    fatalAt(R"("gensOnly": null)", "warps[0].gensOnly");
    fatalAt(R"("minMasterLevel": -1)", "warps[0].minMasterLevel");
    fatalAt(R"("minResets": 1.5)", "warps[0].minResets");
    fatalAt(R"("needsOneOf": {"wings": true})", "warps[0].needsOneOf");
    fatalAt(R"("needsOneOf": [{"wings": false}])", "warps[0].needsOneOf[0].wings");
    fatalAt(R"("needsOneOf": [{}])", "warps[0].needsOneOf[0]");
    fatalAt(R"("needsOneOf": [{"wings": true, "tag": "flying"}])", "warps[0].needsOneOf[0]");
    fatalAt(R"("notWith": [{"item": {"group": 16, "number": 2}}])", "warps[0].notWith[0].item.group");
    fatalAt(R"("notWith": [{"item": {"group": 13, "number": 512}}])", "warps[0].notWith[0].item.number");
    fatalAt(R"("notWith": [{"item": [13, 2]}])", "warps[0].notWith[0].item");
    fatalAt(R"("notWith": [{"tag": ""}])", "warps[0].notWith[0].tag");
    fatalAt(R"("notWith": [{"item": {"group": 13, "number": 2}, "except": {"tag": "flying"}}])", "warps[0].notWith[0]");
    fatalAt(R"("notWith": [{"tag": "mount", "except": "flying"}])", "warps[0].notWith[0].except");
    fatalAt(R"("blockedMessage": 3)", "warps[0].blockedMessage");

    const auto fileFatalAt = [](const std::string& discounts, const std::string& field)
    {
        const ReadResult result =
            Read(R"({ "formatVersion": 1, "classLevelDiscount": )" + discounts + R"(, "warps": [] })");
        INFO(discounts << "\n" << Messages(result.issues));
        CHECK(HasIssueAbout(result.issues, ItemDataIssueSeverity::Fatal, field));
    };
    fileFatalAt(R"({"class": "darkLord"})", "classLevelDiscount");
    fileFatalAt(R"([{"class": "dragon", "numerator": 2, "denominator": 3}])", "classLevelDiscount[0].class");
    fileFatalAt(R"([{"class": "darkLord", "numerator": -1, "denominator": 3}])", "classLevelDiscount[0].numerator");
    fileFatalAt(R"([{"class": "darkLord", "numerator": 2, "denominator": 0}])", "classLevelDiscount[0].denominator");
    fileFatalAt(R"([{"class": "darkLord", "numerator": 2}])", "classLevelDiscount[0].denominator");
    fileFatalAt(R"([{"class": "darkLord", "numerator": 2, "denominator": 3},)"
                R"( {"class": "darkLord", "numerator": 1, "denominator": 2}])",
                "classLevelDiscount[1].class");
}

TEST_CASE("Warps: an unknown item tag or message key is a warning, the warp is kept [data][warps]")
{
    const ReadResult result = Read(File(
        R"({"index": 1, "name": "Arena", "command": "Arena", "level": 1, "zen": 1, "gate": 1,)"
        R"( "needsOneOf": [{"tag": "dragonish"}], "notWith": [{"tag": "mount", "except": {"tag": "nope"}, "x": 1}],)"
        R"( "blockedMessage": "NoSuchText"})"));
    INFO(Messages(result.issues));
    CHECK_FALSE(HasFatal(result.issues));
    CHECK(result.list.warps.size() == 1);
    CHECK(HasIssueAbout(result.issues, ItemDataIssueSeverity::Warning, "warps[0].needsOneOf[0].tag"));
    CHECK(HasIssueAbout(result.issues, ItemDataIssueSeverity::Warning, "warps[0].notWith[0].except.tag"));
    CHECK(HasIssueAbout(result.issues, ItemDataIssueSeverity::Warning, "warps[0].notWith[0].x"));
    CHECK(HasIssueAbout(result.issues, ItemDataIssueSeverity::Warning, "warps[0].blockedMessage"));
}

TEST_CASE("Warps: the shipped file has the move rules of the old code [data][warps]")
{
    const Data::Maps::WarpsLoadResult result = Data::Maps::LoadWarpsFile(ShippedFile());
    REQUIRE(result.issues.empty());
    const auto find = [&](int index) -> const Warp*
    {
        const auto found = std::find_if(result.list.warps.begin(), result.list.warps.end(),
                                        [&](const Warp& warp) { return warp.index == index; });
        return found != result.list.warps.end() ? &*found : nullptr;
    };
    CHECK(result.list.classLevelDiscounts.size() == 3);
    REQUIRE(find(33) != nullptr);
    CHECK(find(33)->noClassDiscount);
    REQUIRE(find(42) != nullptr);
    CHECK(find(42)->pvpServersOnly);
    CHECK(find(42)->gensOnly);
    REQUIRE(find(23) != nullptr);
    CHECK(find(23)->needsOneOf.size() == 2);
    CHECK(find(23)->notWith.size() == 2);
    for (const int atlans : {11, 12, 13})
    {
        REQUIRE(find(atlans) != nullptr);
        CHECK(find(atlans)->notWith.size() == 2);
        CHECK(find(atlans)->blockedMessage == "YouCannotGoToAtlansWhileRidingAUnicorn");
    }
    int withRules = 0;
    for (const Warp& warp : result.list.warps)
    {
        const bool hasRules = warp.noClassDiscount || warp.pvpServersOnly || warp.gensOnly ||
                              !warp.needsOneOf.empty() || !warp.notWith.empty() || warp.minMasterLevel != 0 ||
                              warp.minResets != 0;
        withRules += hasRules ? 1 : 0;
    }
    CHECK(withRules == 6);
}

TEST_CASE("Warps: a name in another language or a name that is another warp's /move name is a warning [data][warps]")
{
    const ReadResult result = Read(File(
        R"({"index": 1, "name": {"en": "Arena", "es": "Arena"}, "command": "Arena", "level": 1, "zen": 1, "gate": 1},)"
        R"( {"index": 2, "name": {"en": "Arena2", "es": "Arena"}, "command": "Arena2", "level": 1, "zen": 1, "gate": 1},)"
        R"( {"index": 3, "name": "arena2", "command": "Arena3", "level": 1, "zen": 1, "gate": 1},)"
        R"( {"index": 4, "name": "Other", "command": "ARENA2", "level": 1, "zen": 1, "gate": 1},)"
        R"( {"index": 5, "name": "Fine", "command": "Fine", "level": 1, "zen": 1, "gate": 1})"));
    INFO(Messages(result.issues));
    CHECK_FALSE(HasFatal(result.issues));
    CHECK(HasIssueAbout(result.issues, ItemDataIssueSeverity::Warning, "warps[1].name.es"));
    CHECK_FALSE(HasIssueAbout(result.issues, ItemDataIssueSeverity::Warning, "warps[1].name.en"));
    CHECK(HasIssueAbout(result.issues, ItemDataIssueSeverity::Warning, "warps[2].name.en"));
    CHECK(HasIssueAbout(result.issues, ItemDataIssueSeverity::Warning, "warps[3].command"));
    CHECK_FALSE(HasIssueAbout(result.issues, ItemDataIssueSeverity::Warning, "warps[4].command"));
    CHECK_FALSE(HasIssueAbout(result.issues, ItemDataIssueSeverity::Warning, "warps[4].name.en"));
}

// Warp 2 has no Spanish name: Spanish shows its English one, which is warp 1's Spanish name.
TEST_CASE("Warps: a shared name in a language without its own name is a warning about the English name [data][warps]")
{
    const ReadResult result = Read(File(
        R"({"index": 1, "name": {"en": "Arena", "es": "Plaza"}, "command": "Arena", "level": 1, "zen": 1, "gate": 1},)"
        R"( {"index": 2, "name": "Plaza", "command": "Plaza2", "level": 1, "zen": 1, "gate": 1})"));
    INFO(Messages(result.issues));
    CHECK_FALSE(HasFatal(result.issues));
    CHECK(HasIssueAbout(result.issues, ItemDataIssueSeverity::Warning, "warps[1].name.en"));
    CHECK_FALSE(HasIssueAbout(result.issues, ItemDataIssueSeverity::Warning, "warps[1].name.es"));
    REQUIRE(result.issues.size() == 1);
    CHECK(result.issues[0].message.find("(es, where the English name is shown)") != std::string::npos);
}

// Warp 2's English name is warp 1's; that it also stands in for warp 0's Spanish name doesn't hide it.
TEST_CASE("Warps: a shared English name is listed before a clash in a language where it only stands in [data][warps]")
{
    const ReadResult result = Read(File(
        R"({"index": 1, "name": {"en": "Stadium", "es": "Arena"}, "command": "Stadium", "level": 1, "zen": 1, "gate": 1},)"
        R"( {"index": 2, "name": "Arena", "command": "Arena", "level": 1, "zen": 1, "gate": 1},)"
        R"( {"index": 3, "name": "Arena", "command": "Arena3", "level": 1, "zen": 1, "gate": 1})"));
    INFO(Messages(result.issues));
    const auto warp2 = std::find_if(result.issues.begin(), result.issues.end(), [](const ItemDataIssue& issue)
                                    { return issue.field == "warps[2].name.en"; });
    REQUIRE(warp2 != result.issues.end());
    CHECK(warp2->message.find("warps[1]") != std::string::npos);
}

TEST_CASE("Warps: two names that differ only in case are a warning [data][warps]")
{
    const ReadResult result = Read(File(R"({"index": 1, "name": "Arena", "command": "Stadium", "level": 1, "zen": 1, "gate": 1},)"
                                        R"( {"index": 2, "name": "ARENA", "command": "Arena2", "level": 1, "zen": 1, "gate": 1})"));
    INFO(Messages(result.issues));
    CHECK(HasIssueAbout(result.issues, ItemDataIssueSeverity::Warning, "warps[1].name.en"));
}

TEST_CASE("Warps: two warps with one /move name in any case or one English name are warnings [data][warps]")
{
    const ReadResult result = Read(File(R"({"index": 1, "name": "Arena", "command": "Arena", "level": 1, "zen": 1, "gate": 1},)"
                                        R"( {"index": 2, "name": "Arena2", "command": "ARENA", "level": 1, "zen": 1, "gate": 1},)"
                                        R"( {"index": 3, "name": "Arena", "command": "Arena3", "level": 1, "zen": 1, "gate": 1},)"
                                        R"( {"index": 4, "name": "Other", "command": "Other", "level": 1, "zen": 1, "gate": 1})"));
    INFO(Messages(result.issues));
    CHECK_FALSE(HasFatal(result.issues));
    CHECK(result.list.warps.size() == 4);
    CHECK(HasIssueAbout(result.issues, ItemDataIssueSeverity::Warning, "warps[1].command"));
    CHECK(HasIssueAbout(result.issues, ItemDataIssueSeverity::Warning, "warps[2].name.en"));
    CHECK(result.issues.size() == 2);
}

TEST_CASE("Warps: a space in a name, a strange discount and doubled or empty rule entries are warnings [data][warps]")
{
    const ReadResult result =
        Read(R"({ "formatVersion": 1, "classLevelDiscount": [)"
             R"( {"class": "darkLord", "numerator": 0, "denominator": 3},)"
             R"( {"class": "rageFighter", "numerator": 4, "denominator": 3},)"
             R"( {"class": "magicGladiator", "numerator": 3, "denominator": 3}],)"
             R"( "warps": [ {"index": 1, "name": {"en": "Lost Tower", "pt": "Torre Perdida"}, "command": "Lost Tower",)"
             R"( "level": 1, "zen": 1, "gate": 1, "needsOneOf": [{"wings": true}, {"wings": true}],)"
             R"( "notWith": [{"tag": "mount", "except": {"tag": "mount"}}, {"tag": "mount"}]},)"
             R"( {"index": 2, "name": "Arena", "command": "Arena", "level": 1, "zen": 1, "gate": 1,)"
             R"( "blockedMessage": "YouCannotGoToAtlansWhileRidingAUnicorn"} ] })");
    INFO(Messages(result.issues));
    CHECK_FALSE(HasFatal(result.issues));
    CHECK(result.list.warps.size() == 2);
    for (const char* field :
         {"classLevelDiscount[0].numerator", "classLevelDiscount[1].numerator", "warps[0].name.en", "warps[0].name.pt",
          "warps[0].command", "warps[0].needsOneOf[1]", "warps[0].notWith[0].except", "warps[1].blockedMessage"})
    {
        CAPTURE(field);
        CHECK(HasIssueAbout(result.issues, ItemDataIssueSeverity::Warning, field));
    }
    // Numerator as large as the denominator: the level as it is. A tag entry and the same tag with an "except" differ.
    CHECK_FALSE(HasIssueAbout(result.issues, ItemDataIssueSeverity::Warning, "classLevelDiscount[2].numerator"));
    CHECK_FALSE(HasIssueAbout(result.issues, ItemDataIssueSeverity::Warning, "warps[0].needsOneOf[0]"));
    CHECK_FALSE(HasIssueAbout(result.issues, ItemDataIssueSeverity::Warning, "warps[0].notWith[1]"));
    CHECK(result.issues.size() == 8);
}

TEST_CASE("Warps: an item the item list does not have is an error [data][warps]")
{
    const ReadResult result = Read(File(
        R"({"index": 1, "name": "Arena", "command": "Arena", "level": 1, "zen": 1, "gate": 1,)"
        R"( "needsOneOf": [{"wings": true}, {"item": {"group": 13, "number": 3}}],)"
        R"( "notWith": [{"item": {"group": 13, "number": 2}}, {"tag": "mount"}, {"item": {"group": 0, "number": 9}}]})"));
    REQUIRE(result.issues.empty());
    std::vector<ItemDataIssue> issues;
    Data::Maps::ValidateWarpItems(
        result.list, [](int group, int number) { return group == 13 && number == 2; }, "Warps.json", issues);
    INFO(Messages(issues));
    REQUIRE(issues.size() == 2);
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Error, "warps[0].needsOneOf[1].item"));
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Error, "warps[0].notWith[2].item"));
}
