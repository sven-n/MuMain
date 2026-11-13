#include "stdafx.h"

#include "doctest.h"

#include "Data/DataHandler/BuffData/BuffStorage.h"
#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/BuffData/BuffDatabase.h"
#include "Data/GameData/BuffData/BuffsJson.h"
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
    const std::vector<BuffDefinition> buffs = {InfinityArrow(), SealOfAscension(), Poison()};
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
