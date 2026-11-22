#include "stdafx.h"

#include "doctest.h"

#include "Data/DataHandler/BuffData/BuffStorage.h"
#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/BuffData/BuffDatabase.h"
#include "Data/GameData/BuffData/BuffsJson.h"
#include "Data/GameData/SkillData/MasterSkillsJson.h"
#include "GameLogic/Buffs/w_BuffScriptLoader.h"
#include "BuffTableScope.h"
#include "TestFiles.h"

#include <algorithm>
#include <filesystem>
#include <map>
#include <memory>
#include <set>
#include <span>
#include <string>
#include <utility>
#include <vector>

using Data::Buffs::BuffDefinition;
using Data::Buffs::BuffKind;
using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;

namespace
{
struct ReadResult
{
    std::vector<BuffDefinition> buffs;
    std::vector<ItemDataIssue> issues;
};

ReadResult Read(const std::string& text)
{
    ReadResult result;
    Data::Buffs::ReadBuffsJson(text, "Buffs.json", result.buffs, result.issues);
    return result;
}

std::string File(const std::string& entries)
{
    return R"({ "formatVersion": 1, "buffs": [ )" + entries + " ] }";
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

bool HasIssueAbout(const std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const std::string& field,
                   const std::string& message)
{
    return std::any_of(issues.begin(), issues.end(),
                       [&](const ItemDataIssue& issue)
                       {
                           return issue.severity == severity && issue.field == field &&
                                  issue.message.find(message) != std::string::npos;
                       });
}

BuffDefinition InfinityArrow()
{
    BuffDefinition buff;
    buff.number = 6;
    buff.name = "infinityArrow";
    buff.group = 6;
    buff.displayName.Set("en", "Infinity Arrow");
    buff.displayName.Set("de", "Unendlicher Pfeil");
    buff.description.Set("en", "Arrows are not consumed.\nIncreases your damage.");
    buff.family = "infinityArrow";
    buff.cancelSkill = 77;
    return buff;
}

BuffDefinition SealOfAscension()
{
    BuffDefinition buff;
    buff.number = 40;
    buff.name = "sealOfAscension";
    buff.group = 24;
    buff.displayName.Set("en", "Seal of Ascension");
    buff.item = Data::Buffs::BuffItemLink{13, 43};
    buff.duration = Data::Buffs::BuffValue{1800};
    buff.powerUps.push_back({"experienceRate", Data::Items::ItemOptionAdd::Add, Data::Buffs::BuffValue{150}});
    return buff;
}

// The Max AG Boost Aura: its value per level of the hero.
BuffDefinition MaxAgBoostAura()
{
    BuffDefinition buff;
    buff.number = 113;
    buff.name = "maxAgBoostAura";
    buff.group = 58;
    buff.displayName.Set("en", "Max AG Boost Aura");
    buff.item = Data::Buffs::BuffItemLink{13, 104};
    // Longer than an option slot holds: the vitality items last 30 days.
    buff.duration = Data::Buffs::BuffValue{2592000};
    Data::Buffs::BuffValue perLevel{0};
    perLevel.per.push_back({"level", 1, 0});
    buff.powerUps.push_back({"maximumAbility", Data::Items::ItemOptionAdd::Add, perLevel});
    return buff;
}

BuffDefinition Poison()
{
    BuffDefinition buff;
    buff.number = 55;
    buff.name = "poison";
    buff.kind = BuffKind::Debuff;
    buff.group = 27;
    buff.displayName.Set("en", "Poison");
    return buff;
}

// What the strip shows of the active buff numbers, one flag per number.
std::vector<bool> ShownOnStrip(const std::vector<int>& active)
{
    std::unique_ptr<bool[]> shown = std::make_unique<bool[]>(active.size());
    g_BuffDatabase.PickShownOnStrip(active, std::span<bool>(shown.get(), active.size()));
    return std::vector<bool>(shown.get(), shown.get() + active.size());
}

} // namespace

TEST_CASE("Buffs: entries read back as written (buffs design B1, B10, B12) [data][buffs]")
{
    const std::vector<BuffDefinition> buffs = {InfinityArrow(), SealOfAscension(), MaxAgBoostAura(), Poison()};
    const std::string text = Data::Buffs::WriteBuffsJson(buffs);
    const ReadResult read = Read(text);

    INFO(Messages(read.issues));
    CHECK(read.issues.empty());
    CHECK(read.buffs == buffs);
    CHECK(text.find("\"hidden\"") == std::string::npos);
    CHECK(text.find("\"item\": {\n        \"group\": 13,\n        \"number\": 43\n      }") != std::string::npos);
}

TEST_CASE("Buffs: wrong values are fatal and name the entry and field [data][buffs]")
{
    const auto fatalAbout = [](const std::string& entry, const std::string& field, const std::string& message)
    {
        const ReadResult read = Read(File(entry));
        INFO(Messages(read.issues));
        CHECK(read.buffs.empty());
        CHECK(HasIssueAbout(read.issues, ItemDataIssueSeverity::Fatal, field, message));
    };
    const std::string tail = R"(, "displayName": {"en": "A"})";
    fatalAbout(R"({ "number": 0, "name": "a", "kind": "buff", "group": 1)" + tail + " }", "buffs[0].number",
               "from 1 to 255");
    fatalAbout(R"({ "number": 1, "name": "a b", "kind": "buff", "group": 1)" + tail + " }", "buffs[0].name",
               "letters and digits");
    fatalAbout(R"({ "number": 1, "name": "a", "kind": "aura", "group": 1)" + tail + " }", "buffs[0].kind",
               "buff or debuff");
    fatalAbout(R"({ "number": 1, "name": "a", "kind": "buff", "group": 256)" + tail + " }", "buffs[0].group",
               "from 0 to 255");
    fatalAbout(R"({ "number": 1, "name": "a", "kind": "buff", "group": 1, "displayName": {"de": "A"} })",
               "buffs[0].displayName", "English");
    fatalAbout(R"({ "number": 1, "name": "a", "kind": "buff", "group": 1)" + tail +
                   R"(, "item": {"group": 16, "number": 1} })",
               "buffs[0].item.group", "from 0 to 15");
    fatalAbout(R"({ "number": 1, "name": "a", "kind": "buff", "group": 1)" + tail + R"(, "tier": 1 })", "buffs[0].tier",
               "needs a family");
    fatalAbout(R"({ "number": 1, "name": "a", "kind": "buff", "group": 1)" + tail + R"(, "cancelSkill": 650 })",
               "buffs[0].cancelSkill", "from 1 to 649");
    fatalAbout(R"({ "number": 1, "name": "a", "kind": "buff", "group": 1)" + tail + R"(, "hidden": 1 })",
               "buffs[0].hidden", "true or false");
    fatalAbout(R"({ "number": 1, "name": "a", "kind": "buff", "group": 1)" + tail + R"(, "family": "a b" })",
               "buffs[0].family", "letters and digits");
    fatalAbout(R"({ "number": 1, "name": "a", "kind": "buff", "group": 1)" + tail + R"(, "item": [13, 43] })",
               "buffs[0].item", "an object with group and number");
    fatalAbout(R"({ "number": 1, "name": "a", "kind": "buff", "group": 1)" + tail + R"(, "description": {"de": "B"} })",
               "buffs[0].description", "English");
    fatalAbout(R"({ "number": 1, "name": "a", "kind": "buff", "group": 1)" + tail + R"(, "duration": 1800 })",
               "buffs[0].duration", "must be an object");
    fatalAbout(R"({ "number": 1, "name": "a", "kind": "buff", "group": 1)" + tail +
                   R"(, "duration": {"values": 31536001} })",
               "buffs[0].duration.values", "from 0 to 31536000");
    fatalAbout(R"({ "number": 1, "name": "a", "kind": "buff", "group": 1)" + tail + R"(, "powerUps": [] })",
               "buffs[0].powerUps", "must be a list");
    fatalAbout(R"({ "number": 1, "name": "a", "kind": "buff", "group": 1)" + tail +
                   R"(, "powerUps": [{"attribute": "defenseBase"}] })",
               "buffs[0].powerUps[0]", "exactly one of add, addFinal and percent");
    fatalAbout(R"({ "number": 1, "name": "a", "kind": "buff", "group": 1)" + tail +
                   R"(, "powerUps": [{"attribute": "defenseBase", "add": {"values": 1}, "percent": {"values": 1}}] })",
               "buffs[0].powerUps[0]", "exactly one of add, addFinal and percent");
    fatalAbout(R"({ "number": 1, "name": "a", "kind": "buff", "group": 1)" + tail +
                   R"(, "powerUps": [{"attribute": "defenseBase", "add": {"values": 65536}}] })",
               "buffs[0].powerUps[0].add.values", "from 0 to 65535");
    fatalAbout(R"({ "number": 1, "name": "a", "kind": "buff", "group": 1)" + tail +
                   R"(, "powerUps": [{"add": {"values": 1}}] })",
               "buffs[0].powerUps[0].attribute", "letters and digits");
    // A list is one value per item level, and a buff has none.
    fatalAbout(R"({ "number": 1, "name": "a", "kind": "buff", "group": 1)" + tail +
                   R"(, "duration": {"values": [1800]} })",
               "buffs[0].duration", "one number, not a list");
    fatalAbout(R"({ "number": 1, "name": "a", "kind": "buff", "group": 1)" + tail +
                   R"(, "duration": {"values": 1800, "per": [{"attribute": "level", "times": 60}]} })",
               "buffs[0].duration", "without per terms");
    fatalAbout(R"({ "number": 1, "name": "a", "kind": "buff", "group": 1)" + tail +
                   R"(, "powerUps": [{"attribute": "defenseBase", "add": {"values": [60, 70]}}] })",
               "buffs[0].powerUps[0].add", "one number, not a list");
}

TEST_CASE("Buffs: a power-up of an attribute no buff can raise is an error [data][buffs]")
{
    BuffDefinition buff = SealOfAscension();
    buff.powerUps.push_back({"luck", Data::Items::ItemOptionAdd::Add, Data::Buffs::BuffValue{1}});
    std::vector<ItemDataIssue> issues;
    Data::Buffs::ValidateBuffs(std::vector<BuffDefinition>{buff}, {}, "Buffs.json", issues);

    INFO(Messages(issues));
    REQUIRE(issues.size() == 1);
    CHECK(
        HasIssueAbout(issues, ItemDataIssueSeverity::Error, "buffs[0].powerUps[1].attribute", "luck is no attribute"));
}

TEST_CASE("Buffs: a power-up's term on a hero attribute the client doesn't read is a warning [data][buffs]")
{
    BuffDefinition buff = MaxAgBoostAura();
    buff.powerUps[0].value.per[0].attribute = "lvl";
    std::vector<ItemDataIssue> issues;
    Data::Buffs::ValidateBuffs(std::vector<BuffDefinition>{buff}, {}, "Buffs.json", issues);

    INFO(Messages(issues));
    REQUIRE(issues.size() == 1);
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Warning, "buffs[0].powerUps[0].add.per[0].attribute",
                        "no hero attribute lvl"));
}

TEST_CASE("Buffs: a power-up added another way than add is a warning [data][buffs]")
{
    BuffDefinition buff = SealOfAscension();
    buff.powerUps[0].add = Data::Items::ItemOptionAdd::Percent;
    std::vector<ItemDataIssue> issues;
    Data::Buffs::ValidateBuffs(std::vector<BuffDefinition>{buff}, {}, "Buffs.json", issues);

    INFO(Messages(issues));
    REQUIRE(issues.size() == 1);
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Warning, "buffs[0].powerUps[0]", "percent is read but"));
}

TEST_CASE("Buffs: a debuff's power-up is a warning [data][buffs]")
{
    BuffDefinition buff = SealOfAscension();
    buff.kind = Data::Buffs::BuffKind::Debuff;
    std::vector<ItemDataIssue> issues;
    Data::Buffs::ValidateBuffs(std::vector<BuffDefinition>{buff}, {}, "Buffs.json", issues);

    INFO(Messages(issues));
    REQUIRE(issues.size() == 1);
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Warning, "buffs[0].powerUps",
                        "stats don't count a debuff's power-ups"));
}

TEST_CASE("Buffs: a file without a list of buffs or of a newer format is fatal [data][buffs]")
{
    const ReadResult noList = Read(R"({ "formatVersion": 1 })");
    INFO(Messages(noList.issues));
    CHECK(HasIssueAbout(noList.issues, ItemDataIssueSeverity::Fatal, "buffs", "not a list of buffs"));
    const ReadResult notList = Read(R"({ "formatVersion": 1, "buffs": {} })");
    INFO(Messages(notList.issues));
    CHECK(HasIssueAbout(notList.issues, ItemDataIssueSeverity::Fatal, "buffs", "not a list of buffs"));
    const ReadResult newer = Read(R"({ "formatVersion": 2, "buffs": [] })");
    INFO(Messages(newer.issues));
    CHECK(HasIssueAbout(newer.issues, ItemDataIssueSeverity::Fatal, "formatVersion", ""));
}

TEST_CASE("Buffs: a number or a name used twice, a tier twice or a family of buffs and debuffs is fatal [data][buffs]")
{
    BuffDefinition second = InfinityArrow();
    second.number = 143;
    second.name = "infinityArrow";
    std::vector<ItemDataIssue> issues;
    Data::Buffs::ValidateBuffs(std::vector<BuffDefinition>{InfinityArrow(), second}, {}, "Buffs.json", issues);
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Fatal, "buffs[1].name", "buffs[0] too"));
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Fatal, "buffs[1].tier", "tier 0 twice"));

    issues.clear();
    BuffDefinition sameNumber = Poison();
    sameNumber.number = 6;
    Data::Buffs::ValidateBuffs(std::vector<BuffDefinition>{InfinityArrow(), sameNumber}, {}, "Buffs.json", issues);
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Fatal, "buffs[1].number", "buffs[0] too"));

    issues.clear();
    BuffDefinition debuffTier = InfinityArrow();
    debuffTier.number = 143;
    debuffTier.name = "infinityArrowStrengthener";
    debuffTier.kind = BuffKind::Debuff;
    debuffTier.tier = 1;
    Data::Buffs::ValidateBuffs(std::vector<BuffDefinition>{InfinityArrow(), debuffTier}, {}, "Buffs.json", issues);
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Fatal, "buffs[1].kind", "buffs and debuffs"));
}

TEST_CASE("Buffs: a problem of the checks names the entry's place in the file [data][buffs]")
{
    const std::string text = File(R"({ "number": 1, "name": "a", "kind": "aura", "group": 1, "displayName": "A" },
                                    { "number": 2, "name": "b", "kind": "buff", "group": 1, "displayName": "B" },
                                    { "number": 2, "name": "c", "kind": "buff", "group": 1, "displayName": "C" })");
    ReadResult read;
    std::vector<size_t> entries;
    Data::Buffs::ReadBuffsJson(text, "Buffs.json", read.buffs, read.issues, &entries);
    CHECK(entries == std::vector<size_t>{1, 2});
    Data::Buffs::ValidateBuffs(read.buffs, entries, "Buffs.json", read.issues);

    INFO(Messages(read.issues));
    CHECK(HasIssueAbout(read.issues, ItemDataIssueSeverity::Fatal, "buffs[0].kind", "buff or debuff"));
    CHECK(HasIssueAbout(read.issues, ItemDataIssueSeverity::Fatal, "buffs[2].number", "2 is buffs[1] too"));
}

TEST_CASE("Buffs: what the game cannot show is a warning [data][buffs]")
{
    BuffDefinition unknown = Poison();
    unknown.number = eBuff_Count;
    BuffDefinition longName = SealOfAscension();
    longName.displayName.Set("es", std::string(Data::Buffs::MaxDisplayNameLength + 1, 'a'));
    longName.displayName.Set("zh_CN", "A");
    const std::vector<BuffDefinition> buffs = {InfinityArrow(), unknown, longName};
    const char* const uiLocales[] = {"en", "de", "es"};
    std::vector<ItemDataIssue> issues;
    Data::Buffs::ValidateBuffsForGame(buffs, std::vector<size_t>{0, 1, 3}, uiLocales, "Buffs.json", issues);

    INFO(Messages(issues));
    CHECK(issues.size() == 3);
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Warning, "buffs[1].number", "never shows"));
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Warning, "buffs[3].displayName", "the es name is cut"));
    CHECK(
        HasIssueAbout(issues, ItemDataIssueSeverity::Warning, "buffs[3].displayName", "the zh_CN text is never shown"));
}

TEST_CASE("Buffs: an item the item list does not have is an error [data][buffs]")
{
    BuffDefinition moved = SealOfAscension();
    moved.item = Data::Buffs::BuffItemLink{13, 99};
    const std::vector<BuffDefinition> buffs = {InfinityArrow(), SealOfAscension(), moved};
    const auto itemExists = [](int group, int number) { return group == 13 && number == 43; };
    std::vector<ItemDataIssue> issues;
    Data::Buffs::ValidateBuffItems(buffs, std::vector<size_t>{0, 1, 4}, itemExists, "Buffs.json", issues);

    INFO(Messages(issues));
    CHECK(issues.size() == 1);
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Error, "buffs[4].item", "names item (13, 99)"));
}

TEST_CASE("Buffs: an unknown field is a warning, and the entry is kept [data][buffs]")
{
    const ReadResult read =
        Read(File(R"({ "number": 1, "name": "a", "kind": "buff", "group": 1, "displayName": "A", "colour": 3 })"));
    CHECK(read.buffs.size() == 1);
    CHECK(HasIssueAbout(read.issues, ItemDataIssueSeverity::Warning, "buffs[0].colour", "unknown field"));

    const ReadResult inItem = Read(File(R"({ "number": 1, "name": "a", "kind": "buff", "group": 1, "displayName": "A",)"
                                        R"( "item": {"group": 13, "number": 43, "level": 1} })"));
    CHECK(inItem.buffs.size() == 1);
    CHECK(HasIssueAbout(inItem.issues, ItemDataIssueSeverity::Warning, "buffs[0].item.level", "unknown field"));
}

TEST_CASE("Buffs: the shipped file loads without problems and is written as the writer writes it [data][buffs]")
{
    const std::filesystem::path file = std::filesystem::path(MU_TEST_DATA_DIR) / "Buffs" / "Buffs.json";
    const Data::Buffs::BuffsLoadResult loaded = Data::Buffs::LoadBuffsFile(file);

    INFO(Messages(loaded.issues));
    CHECK(loaded.issues.empty());
    CHECK(loaded.buffs.size() == 158);
    std::string text = TestFiles::ReadWholeFile(file);
    std::erase(text, '\r');
    CHECK(text == Data::Buffs::WriteBuffsJson(loaded.buffs));
}

TEST_CASE("Buffs: every item the shipped buffs name is in the shipped item data [data][buffs][items]")
{
    const std::filesystem::path data = std::filesystem::path(MU_TEST_DATA_DIR);
    const Data::Buffs::BuffsLoadResult buffs = Data::Buffs::LoadBuffsFile(data / "Buffs" / "Buffs.json");
    const Data::Items::ItemDataLoadResult items = Data::Items::LoadItemDataDirectory(data / "Items");
    REQUIRE(buffs.issues.empty());
    REQUIRE(items.items.size() > 900);
    std::set<std::pair<int, int>> shipped;
    for (const Data::Items::ItemDefinition& item : items.items)
    {
        shipped.emplace(item.group, item.number);
    }

    std::vector<ItemDataIssue> issues;
    Data::Buffs::ValidateBuffItems(
        buffs.buffs, {}, [&](int group, int number) { return shipped.contains({group, number}); }, "Buffs.json",
        issues);
    INFO(Messages(issues));
    CHECK(issues.empty());
}

TEST_CASE("Buffs: the table gives each buff's texts in the display language, English where it has none [data][buffs]")
{
    const BuffTableScope table({InfinityArrow(), SealOfAscension()}, "de");

    REQUIRE(g_BuffDatabase.FindTexts(6) != nullptr);
    CHECK(g_BuffDatabase.FindTexts(6)->name == L"Unendlicher Pfeil");
    CHECK(g_BuffDatabase.FindTexts(6)->descriptionLines ==
          std::vector<std::wstring>{L"Arrows are not consumed.", L"Increases your damage."});
    CHECK(g_BuffDatabase.FindTexts(40)->name == L"Seal of Ascension");
    CHECK(g_BuffDatabase.FindTexts(40)->descriptionLines.empty());
    CHECK(g_BuffDatabase.Find(7) == nullptr);

    g_BuffDatabase.SetDisplayLocale("en");
    CHECK(g_BuffDatabase.FindTexts(6)->name == L"Infinity Arrow");
}

TEST_CASE("Buffs: the game's buff info comes from the table and follows a new display language [data][buffs]")
{
    const BuffTableScope table({InfinityArrow(), SealOfAscension(), Poison()}, "de");
    BuffScriptLoaderPtr loader = BuffScriptLoader::Make();

    const BuffInfo arrow = loader->GetBuffinfo(static_cast<eBuffState>(6));
    CHECK(std::wstring(arrow.s_BuffName) == L"Unendlicher Pfeil");
    CHECK(arrow.s_BuffEffectType == 6);
    CHECK(arrow.s_ItemType == BuffInfo::NoItem);
    CHECK_FALSE(arrow.HasItem());
    CHECK(arrow.s_BuffDescriptlist.size() == 2);
    CHECK(loader->IsBuffClass(static_cast<eBuffState>(6)) == eBuffClass_Buff);
    CHECK(loader->IsBuffClass(static_cast<eBuffState>(55)) == eBuffClass_DeBuff);
    CHECK(loader->IsBuffClass(static_cast<eBuffState>(7)) == eBuffClass_Count);

    const BuffInfo seal = loader->GetBuffinfo(static_cast<eBuffState>(40));
    CHECK(seal.s_ItemType == 13);
    CHECK(seal.s_ItemIndex == 43);

    g_BuffDatabase.SetDisplayLocale("en");
    CHECK(std::wstring(loader->GetBuffinfo(static_cast<eBuffState>(6)).s_BuffName) == L"Infinity Arrow");
}

TEST_CASE("Buffs: an item that gives two buffs finds the lower number, whatever the file's order [data][buffs]")
{
    BuffDefinition sealOfHealing = SealOfAscension();
    sealOfHealing.number = 29;
    sealOfHealing.name = "sealOfHealing";
    sealOfHealing.group = 29;
    const BuffTableScope table({SealOfAscension(), sealOfHealing}, "en");
    BuffScriptLoaderPtr loader = BuffScriptLoader::Make();

    CHECK(loader->GetBuffIndex(ITEMINDEX(13, 43)) == 29);
    CHECK(loader->GetBuffType(ITEMINDEX(13, 43)) == 29);
    CHECK(loader->GetBuffIndex(ITEMINDEX(13, 44)) == -1);
    REQUIRE(g_BuffDatabase.FindByItem(13, 43) != nullptr);
    CHECK(g_BuffDatabase.FindByItem(13, 43)->number == 29);
    CHECK(g_BuffDatabase.FindByItem(13, 44) == nullptr);
}

TEST_CASE("Buffs: a new table forgets the items of the old one [data][buffs]")
{
    {
        const BuffTableScope table({SealOfAscension()}, "en");
        CHECK(g_BuffDatabase.FindByItem(13, 43) != nullptr);
    }
    CHECK(g_BuffDatabase.FindByItem(13, 43) == nullptr);
}

TEST_CASE("Buffs: the shipped file keeps the strip rules of the old client's code [data][buffs]")
{
    const std::filesystem::path file = std::filesystem::path(MU_TEST_DATA_DIR) / "Buffs" / "Buffs.json";
    const Data::Buffs::BuffsLoadResult loaded = Data::Buffs::LoadBuffsFile(file);
    REQUIRE(loaded.issues.empty());

    // The old BuffStrip.cpp's NormalizeBuffState and BuffTier: each family's buffs from tier 0 up.
    const std::map<std::string, std::vector<int>> oldFamilies = {
        {"swellLife", {8, 135, 136}},
        {"expansionOfWizardry", {82, 138, 139}},
        {"criticalDamageIncrease", {5, 149, 148}},
        {"infinityArrow", {6, 143}},
        {"blind", {73, 144}},
        {"poisonArrow", {159, 160}},
        {"bless", {142, 161}},
        {"ironDefense", {134, 165}},
        {"bloodHowl", {166, 167}},
    };
    // SetDisableRenderBuff's effects.
    const std::vector<int> oldHidden = {83, 84, 85, 120};
    // CancellableSkill, with the strengthened tiers OpenMU also cancels.
    const std::map<int, int> cancelSkills = {{6, 77}, {82, 233}, {138, 380}, {139, 383}, {143, 441}};

    std::map<std::string, std::vector<int>> families;
    std::vector<int> hidden;
    std::map<int, int> cancels;
    std::vector<const BuffDefinition*> byNumber(Data::Buffs::MaxBuffNumber + 1, nullptr);
    for (const BuffDefinition& buff : loaded.buffs)
    {
        byNumber[buff.number] = &buff;
        if (!buff.family.empty())
        {
            families[buff.family].push_back(buff.number);
        }
        if (buff.hidden)
        {
            hidden.push_back(buff.number);
        }
        if (buff.cancelSkill != 0)
        {
            cancels[buff.number] = buff.cancelSkill;
        }
    }
    for (auto& [family, numbers] : families)
    {
        std::sort(numbers.begin(), numbers.end(),
                  [&](int left, int right) { return byNumber[left]->tier < byNumber[right]->tier; });
    }
    std::sort(hidden.begin(), hidden.end());
    CHECK(hidden == oldHidden);
    CHECK(cancels == cancelSkills);
    REQUIRE(families == oldFamilies);
    for (const auto& [family, numbers] : oldFamilies)
    {
        for (size_t tier = 0; tier < numbers.size(); ++tier)
        {
            INFO(family << " buff " << numbers[tier]);
            CHECK(byNumber[numbers[tier]]->tier == static_cast<int>(tier));
        }
    }
}

TEST_CASE("Buffs: the strip shows the highest tier of each shipped family and no hidden effect [data][buffs]")
{
    const std::filesystem::path file = std::filesystem::path(MU_TEST_DATA_DIR) / "Buffs" / "Buffs.json";
    const Data::Buffs::BuffsLoadResult loaded = Data::Buffs::LoadBuffsFile(file);
    REQUIRE(loaded.issues.empty());
    const BuffTableScope table(loaded.buffs, "en");

    std::map<std::string, std::vector<const BuffDefinition*>> families;
    for (const BuffDefinition& buff : loaded.buffs)
    {
        const std::vector<int> alone = {buff.number};
        CHECK(ShownOnStrip(alone)[0] == !buff.hidden);
        if (!buff.family.empty() && !buff.hidden)
        {
            families[buff.family].push_back(&buff);
        }
    }
    for (const auto& [family, tiers] : families)
    {
        // All tiers on at once, in the file's order and reversed: only the highest shows.
        std::vector<int> active;
        const BuffDefinition* highest = tiers.front();
        for (const BuffDefinition* buff : tiers)
        {
            active.push_back(buff->number);
            highest = buff->tier > highest->tier ? buff : highest;
        }
        for (int pass = 0; pass < 2; ++pass)
        {
            const std::vector<bool> shown = ShownOnStrip(active);
            for (size_t index = 0; index < active.size(); ++index)
            {
                INFO(family << " buff " << active[index]);
                CHECK(shown[index] == (active[index] == highest->number));
            }
            std::reverse(active.begin(), active.end());
        }
    }
    CHECK(families.at("infinityArrow").size() == 2);
    CHECK(families.at("expansionOfWizardry").size() == 3);

    const std::vector<int> mixed = {6, 55, 143, 0};
    CHECK(ShownOnStrip(mixed) == std::vector<bool>{false, true, true, false});
    // The picks of one call leave nothing behind for the next.
    CHECK(ShownOnStrip({6}) == std::vector<bool>{true});
}

// A caster's buff (a skill buff, buffs design B17) takes OpenMU's numbers: decimals, negatives, a maximum and terms in
// its duration. An owner's buff keeps Webzen's whole numbers.
TEST_CASE("Buffs: a caster's buff reads OpenMU's decimals, negatives and maximums [data][buffs]")
{
    const std::string entry = R"({ "number": 81, "name": "berserker", "kind": "buff", "group": 1,
        "displayName": {"en": "Berserker"}, "valuesFrom": "caster",
        "duration": {"values": 30, "per": [{"attribute": "totalEnergy", "divisor": 20}], "maximum": 180},
        "powerUps": [
          {"attribute": "berserkerManaMultiplier", "add": {"values": 0,
            "per": [{"attribute": "totalEnergy", "divisor": 3000}], "maximum": 1}},
          {"attribute": "berserkerHealthDecrement", "add": {"values": -0.4,
            "per": [{"attribute": "totalEnergy", "divisor": 6000}], "maximum": -0.1}},
          {"attribute": "berserkerMinPhysDmgBonus", "add": {"values": 140,
            "per": [{"attribute": "maximumMana", "times": 0.02}]}, "multipliedBy": "berserkerManaMultiplier"},
          {"attribute": "berserkerWizardryMultiplier", "add": {"values": 0}, "valueOf": "berserkerManaMultiplier"},
          {"attribute": "berserkerCurseMultiplier", "add": {"values": 0},
            "valueOf": ["berserkerManaMultiplier", "berserkerWizardryMultiplier"]}] })";
    const ReadResult read = Read(File(entry));
    INFO(Messages(read.issues));
    REQUIRE(read.buffs.size() == 1);
    CHECK(read.issues.empty());
    const BuffDefinition& buff = read.buffs.front();
    CHECK(buff.valuesFrom == Data::Buffs::BuffValuesFrom::Caster);
    REQUIRE(buff.duration.has_value());
    CHECK(buff.duration->maximum == 180);
    CHECK(buff.duration->per.front().divisor == 20);
    REQUIRE(buff.powerUps.size() == 5);
    CHECK(buff.powerUps[1].value.values == doctest::Approx(-0.4));
    CHECK(*buff.powerUps[1].value.maximum == doctest::Approx(-0.1));
    CHECK(buff.powerUps[2].value.per.front().times == doctest::Approx(0.02));
    CHECK(buff.powerUps[2].multipliedBy == "berserkerManaMultiplier");
    CHECK(buff.powerUps[3].valueOf == std::vector<std::string>{"berserkerManaMultiplier"});
    CHECK(buff.powerUps[4].valueOf ==
          std::vector<std::string>{"berserkerManaMultiplier", "berserkerWizardryMultiplier"});

    // Written and read back, it is the same.
    const ReadResult again = Read(Data::Buffs::WriteBuffsJson(read.buffs));
    INFO(Messages(again.issues));
    REQUIRE(again.buffs.size() == 1);
    CHECK(again.buffs.front() == buff);

    std::vector<ItemDataIssue> issues;
    Data::Buffs::ValidateBuffs(read.buffs, {}, "Buffs.json", issues);
    INFO(Messages(issues));
    CHECK(issues.empty());
}

TEST_CASE("Buffs: an owner's buff keeps whole numbers, and valuesFrom is owner or caster [data][buffs]")
{
    const std::string head = R"({ "number": 1, "name": "a", "kind": "buff", "group": 1, "displayName": {"en": "A"})";
    const ReadResult decimal =
        Read(File(head + R"(, "powerUps": [{"attribute": "defenseBase", "add": {"values": 1.5}}] })"));
    CHECK(
        HasIssueAbout(decimal.issues, ItemDataIssueSeverity::Fatal, "buffs[0].powerUps[0].add.values", "whole number"));
    const ReadResult negative =
        Read(File(head + R"(, "powerUps": [{"attribute": "defenseBase", "add": {"values": -1}}] })"));
    CHECK(HasIssueAbout(negative.issues, ItemDataIssueSeverity::Fatal, "buffs[0].powerUps[0].add.values",
                        "from 0 to 65535"));
    const ReadResult source = Read(File(head + R"(, "valuesFrom": "target" })"));
    CHECK(HasIssueAbout(source.issues, ItemDataIssueSeverity::Fatal, "buffs[0].valuesFrom", "owner or caster"));
    const ReadResult term = Read(File(head + R"(, "valuesFrom": "caster", "powerUps": [{"attribute": "defenseBase",
        "add": {"values": 1, "per": [{"attribute": "totalEnergy", "times": 1, "divisor": 2}]}}] })"));
    CHECK(HasIssueAbout(term.issues, ItemDataIssueSeverity::Fatal, "buffs[0].powerUps[0].add.per[0]",
                        "exactly one of times and divisor"));

    // A caster's numbers stay in bounds too, so its totals fit an int: a value, a maximum and a duration.
    const ReadResult huge = Read(File(head + R"(, "valuesFrom": "caster", "powerUps": [{"attribute": "defenseBase",
        "add": {"values": 1, "maximum": -2000000}}] })"));
    CHECK(HasIssueAbout(huge.issues, ItemDataIssueSeverity::Fatal, "buffs[0].powerUps[0].add.maximum",
                        "from -1000000 to 1000000"));
    const ReadResult year = Read(File(head + R"(, "valuesFrom": "caster", "duration": {"values": 40000000} })"));
    CHECK(HasIssueAbout(year.issues, ItemDataIssueSeverity::Fatal, "buffs[0].duration.values", "to 31536000"));
}

TEST_CASE("Buffs: a chain to no other power-up, or back to where it starts, is fatal [data][buffs]")
{
    BuffDefinition buff = SealOfAscension();
    buff.valuesFrom = Data::Buffs::BuffValuesFrom::Caster;
    buff.powerUps.front().multipliedBy = "mobility";
    std::vector<ItemDataIssue> issues;
    Data::Buffs::ValidateBuffs(std::vector<BuffDefinition>{buff}, {}, "Buffs.json", issues);
    CHECK(
        HasIssueAbout(issues, ItemDataIssueSeverity::Fatal, "buffs[0].powerUps[0].multipliedBy", "no other power-up"));

    buff.powerUps.push_back({"defenseBase", Data::Items::ItemOptionAdd::Add, Data::Buffs::BuffValue{1}});
    buff.powerUps[0].multipliedBy = "defenseBase";
    buff.powerUps[1].valueOf = {buff.powerUps[0].attribute};
    issues.clear();
    Data::Buffs::ValidateBuffs(std::vector<BuffDefinition>{buff}, {}, "Buffs.json", issues);
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Fatal, "buffs[0].powerUps[0]", "lead back to it"));

    // Of two power-ups of one attribute, a chain names the other one, as the buff table resolves it.
    BuffDefinition twice = SealOfAscension();
    twice.valuesFrom = Data::Buffs::BuffValuesFrom::Caster;
    twice.powerUps.push_back(twice.powerUps.front());
    twice.powerUps[0].multipliedBy = twice.powerUps[0].attribute;
    issues.clear();
    Data::Buffs::ValidateBuffs(std::vector<BuffDefinition>{twice}, {}, "Buffs.json", issues);
    INFO(Messages(issues));
    CHECK_FALSE(
        HasIssueAbout(issues, ItemDataIssueSeverity::Fatal, "buffs[0].powerUps[0].multipliedBy", "no other power-up"));

    // Only the power-ups on the loop are reported: the first leads into a loop of the other two.
    BuffDefinition into = SealOfAscension();
    into.valuesFrom = Data::Buffs::BuffValuesFrom::Caster;
    into.powerUps.clear();
    for (const char* attribute : {"defenseBase", "mobility", "experienceRate"})
    {
        into.powerUps.push_back({attribute, Data::Items::ItemOptionAdd::Add, Data::Buffs::BuffValue{1}});
    }
    into.powerUps[0].multipliedBy = "experienceRate";
    into.powerUps[1].multipliedBy = "experienceRate";
    into.powerUps[2].valueOf = {"mobility"};
    issues.clear();
    Data::Buffs::ValidateBuffs(std::vector<BuffDefinition>{into}, {}, "Buffs.json", issues);
    INFO(Messages(issues));
    const auto loopsAt = [&](const std::string& field)
    {
        return std::any_of(
            issues.begin(), issues.end(), [&](const ItemDataIssue& issue)
            { return issue.field == field && issue.message.find("lead back to it") != std::string::npos; });
    };
    CHECK_FALSE(loopsAt("buffs[0].powerUps[0]"));
    CHECK(loopsAt("buffs[0].powerUps[1]"));
    CHECK(loopsAt("buffs[0].powerUps[2]"));

    // A name twice in valueOf, and more names than the values keep, are errors.
    BuffDefinition list = into;
    list.powerUps[0].multipliedBy.clear();
    list.powerUps[1].multipliedBy.clear();
    list.powerUps[2].valueOf = {"mobility", "defenseBase", "mobility"};
    issues.clear();
    Data::Buffs::ValidateBuffs(std::vector<BuffDefinition>{list}, {}, "Buffs.json", issues);
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Error, "buffs[0].powerUps[2].valueOf[2]", "named twice"));
    list.powerUps[2].valueOf = {"mobility", "defenseBase", "mobility", "defenseBase", "mobility"};
    issues.clear();
    Data::Buffs::ValidateBuffs(std::vector<BuffDefinition>{list}, {}, "Buffs.json", issues);
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Error, "buffs[0].powerUps[2].valueOf", "only the first 4"));
}

// Data/Skills/MasterSkills.json (buffs design B18, B20).
TEST_CASE("Master skills: the file reads back as written, and its checks find what adds nothing [data][buffs]")
{
    const std::string text = R"({ "formatVersion": 1, "masterSkills": [
        {"skill": 420, "raises": "greaterDamageBonus", "how": "multiply"},
        {"skill": 422, "raises": "greaterDamageBonus", "how": "multiply", "extendsDuration": true},
        {"skill": 441, "raises": "attackDamageIncrease", "how": "add", "castsBuff": 6}] })";
    std::vector<Data::Skills::MasterSkillDefinition> skills;
    std::vector<ItemDataIssue> issues;
    Data::Skills::ReadMasterSkillsJson(text, "MasterSkills.json", skills, issues);
    INFO(Messages(issues));
    REQUIRE(skills.size() == 3);
    CHECK(skills[1].how == Data::Skills::MasterSkillHow::Multiply);
    CHECK(skills[1].extendsDuration);
    CHECK(skills[2].castsBuff == 6);
    std::vector<Data::Skills::MasterSkillDefinition> again;
    Data::Skills::ReadMasterSkillsJson(Data::Skills::WriteMasterSkillsJson(skills), "MasterSkills.json", again, issues);
    CHECK(again == skills);

    std::vector<Data::Skills::MasterSkillDefinition> wrong = skills;
    wrong.push_back({420});
    wrong.push_back({500, "speed"});
    wrong.push_back({501});
    issues.clear();
    Data::Skills::ValidateMasterSkills(wrong, {}, "MasterSkills.json", issues);
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Fatal, "masterSkills[3].skill", "is masterSkills[0] too"));
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Error, "masterSkills[4].raises", "no attribute"));
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Warning, "masterSkills[5]", "does nothing"));

    // Against the buffs: castsBuff names a buff of the list, and what a skill raises is a power-up of its buff.
    BuffDefinition infinity = InfinityArrow();
    infinity.powerUps.push_back({"attackDamageIncrease", Data::Items::ItemOptionAdd::Add, Data::Buffs::BuffValue{}});
    issues.clear();
    Data::Skills::ValidateMasterSkillBuffs(
        skills, {}, std::vector<BuffDefinition>{infinity},
        [](int skill) { return skill == 420 || skill == 422 ? 1 : 0; }, "MasterSkills.json", issues);
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Error, "masterSkills[0].raises", "casts no buff"));
    CHECK_FALSE(HasIssueAbout(issues, ItemDataIssueSeverity::Error, "masterSkills[2].raises", ""));
    skills[2].castsBuff = 99;
    issues.clear();
    Data::Skills::ValidateMasterSkillBuffs(skills, {}, std::vector<BuffDefinition>{infinity}, {}, "MasterSkills.json",
                                           issues);
    CHECK(HasIssueAbout(issues, ItemDataIssueSeverity::Error, "masterSkills[2].castsBuff", "no buff of Buffs.json"));
}
