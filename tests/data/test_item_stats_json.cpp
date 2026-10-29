#include "stdafx.h"

#include "doctest.h"

#include "Data/GameData/ItemData/ItemStatProfiles.h"
#include "Data/GameData/ItemData/ItemStatsJson.h"

#include <string>
#include <vector>

using namespace Data::Items;

namespace
{
constexpr const char* Source = "ItemStats.json";

// A small file in the written format: two tables, the default, two profiles and
// two profile rules.
constexpr const char* SmallFile = R"({
  "formatVersion": 1,
  "dropLevelBonus": {"excellent": 25, "ancient": 30},
  "tables": [
    {
      "name": "linear",
      "caption": {"en": "Linear", "de": "Linear"},
      "perLevel": 1
    },
    {
      "name": "steps",
      "caption": {"en": "Steps"},
      "values": [0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 31, 36, 42, 49, 57, 66]
    }
  ],
  "default": {
    "damage": {"table": "steps", "excellent": {"times": 25, "add": 5}, "ancient": {"add": 5, "dropLevelDivisor": 40}},
    "magicPower": {"table": "steps", "excellent": {"times": 25, "add": 5}, "ancient": {"add": 2, "dropLevelDivisor": 60}, "perLevelAfterHalving": 2},
    "blockRate": {"table": "steps", "excellent": {"times": 25, "add": 5}, "ancient": "none"},
    "defense": {"table": "steps", "excellent": {"times": 12, "add": 4, "dropLevelDivisor": 5}, "ancient": {"times": 3, "add": 2, "dropLevelDivisor": 30}},
    "magicDefense": {"table": "steps", "excellent": "none", "ancient": "none"},
    "requirements": {
      "levelPerItemLevel": 0,
      "excellentLevelAdd": 20,
      "strength": {"add": 20, "levelWeight": 3, "percent": 3},
      "dexterity": {"add": 20, "levelWeight": 3, "percent": 3},
      "energy": {"add": 20, "levelWeight": 3, "percent": 4},
      "vitality": {"add": 20, "levelWeight": 3, "percent": 3},
      "leadership": "base",
      "strengthPerOptionLevel": 0
    }
  },
  "profiles": [
    {
      "name": "sceptre",
      "magicPower": {"perLevelAfterHalving": 0}
    },
    {
      "name": "shield",
      "defense": {"table": "linear", "excellent": "none", "ancient": {"times": 20, "add": 2, "afterLevelBonus": true}},
      "requirements": {
        "levelPerItemLevel": 4,
        "energy": {"add": 20, "percent": 4, "basis": "levelRequirement"},
        "strengthPerOptionLevel": 5
      }
    }
  ],
  "profileRules": [
    {"profile": "sceptre", "tags": ["sceptre"]},
    {"profile": "shield", "group": [6], "weaponKind": ["shield"]}
  ]
}
)";

struct ReadResult
{
    ItemStatFile file;
    std::vector<ItemDataIssue> issues;
};

ReadResult Read(const std::string& text)
{
    ReadResult result;
    ReadItemStatsJson(text, Source, result.file, result.issues);
    return result;
}

bool HasIssue(const std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const std::string& field)
{
    for (const ItemDataIssue& issue : issues)
    {
        if (issue.severity == severity && issue.field == field)
        {
            return true;
        }
    }
    return false;
}

// The small file with `from` replaced by `to` (once).
std::string Changed(const std::string& from, const std::string& to)
{
    std::string text = SmallFile;
    const size_t position = text.find(from);
    REQUIRE(position != std::string::npos);
    text.replace(position, from.size(), to);
    return text;
}
} // namespace

TEST_CASE("The item stat file reads and writes back the same [data][items]")
{
    const ReadResult result = Read(SmallFile);
    CHECK(result.issues.empty());
    CHECK(WriteItemStatsJson(result.file) == SmallFile);

    REQUIRE(result.file.tables.size() == 2);
    CHECK(result.file.excellentDropLevelBonus == 25);
    CHECK(result.file.ancientDropLevelBonus == 30);
    CHECK(result.file.tables[0].name == "linear");
    CHECK(result.file.tables[0].perLevel == 1);
    CHECK(result.file.tables[0].caption.Get("de") == "Linear");
    CHECK(result.file.tables[1].values[15] == 66);
    REQUIRE(result.file.profiles.size() == 2);
    CHECK(result.file.profiles[1].stats[static_cast<size_t>(ItemStat::Defense)].excellent.state ==
          WrittenStatTerm::State::None);
    REQUIRE(result.file.profileRules.size() == 2);
    CHECK(result.file.profileRules[0].profile == "sceptre");
    CHECK(result.file.profileRules[0].tags == ItemTagSet{ItemTag::Sceptre});
    CHECK(result.file.profileRules[1].groups.test(6));
    CHECK(result.file.profileRules[1].groups.count() == 1);
    CHECK(result.file.profileRules[1].weaponKinds.test(static_cast<size_t>(WeaponKind::Shield)));
}

TEST_CASE("The item stat writer keeps the order of the profile rules, one rule per line [data][items]")
{
    ItemStatFile file = Read(SmallFile).file;
    std::swap(file.profileRules[0], file.profileRules[1]);
    // Values are written in the order of their names, groups from the lowest.
    StatProfileRule armor{.profile = "default"};
    armor.slots.set(static_cast<size_t>(ItemSlot::Boots));
    armor.slots.set(static_cast<size_t>(ItemSlot::Helm));
    armor.groups.set(11);
    armor.groups.set(7);
    armor.requirements.set(static_cast<size_t>(ItemRequirement::Energy));
    armor.requirements.set(static_cast<size_t>(ItemRequirement::Level));
    file.profileRules.push_back(armor);

    const std::string text = WriteItemStatsJson(file);
    CHECK(text.find(R"({"profile": "shield", "group": [6], "weaponKind": ["shield"]},)") <
          text.find(R"({"profile": "sceptre", "tags": ["sceptre"]},)"));
    CHECK(
        text.find(
            R"({"profile": "default", "group": [7, 11], "slot": ["helm", "boots"], "requirements": ["level", "energy"]})") !=
        std::string::npos);

    const ReadResult reread = Read(text);
    CHECK(reread.issues.empty());
    CHECK(reread.file.profileRules == file.profileRules);
}

TEST_CASE("An item stat file without profile rules has none [data][items]")
{
    const ReadResult result = Read(Changed(R"(,
  "profileRules": [
    {"profile": "sceptre", "tags": ["sceptre"]},
    {"profile": "shield", "group": [6], "weaponKind": ["shield"]}
  ])",
                                           ""));
    CHECK(result.issues.empty());
    CHECK(result.file.profileRules.empty());
    CHECK(WriteItemStatsJson(result.file).find(R"("profileRules": [])") != std::string::npos);
}

TEST_CASE("The item stat writer sorts by name and keeps one value for every level as one number [data][items]")
{
    ItemStatFile file = Read(SmallFile).file;
    std::swap(file.tables[0], file.tables[1]);
    std::swap(file.profiles[0], file.profiles[1]);
    file.tables[1].perLevel = 0;
    file.tables[1].values.fill(7);

    const std::string text = WriteItemStatsJson(file);
    CHECK(text.find(R"("name": "linear",)") < text.find(R"("name": "steps",)"));
    CHECK(text.find(R"("name": "sceptre",)") < text.find(R"("name": "shield",)"));
    CHECK(text.find(R"("values": 7)") != std::string::npos);

    const ReadResult reread = Read(text);
    CHECK(reread.issues.empty());
    CHECK(reread.file.tables[0].values[9] == 7);
}

TEST_CASE("Item stat profiles lay their terms over the default [data][items]")
{
    const ItemStatProfiles profiles(Read(SmallFile).file);

    CHECK(profiles.Find("") == ItemStatProfiles::DefaultProfile);
    CHECK(profiles.Find("default") == ItemStatProfiles::DefaultProfile);
    CHECK(profiles.Find("shield") == 2);
    CHECK(profiles.Find("unknown") == ItemStatProfiles::UnknownProfile);
    CHECK(profiles.GetProfileNames() == std::vector<std::string>{"sceptre", "shield"});
    CHECK(profiles.GetAncientDropLevelBonus() == 30);

    const ItemStatProfile& defaults = profiles.Get(ItemStatProfiles::DefaultProfile);
    const ItemStatProfile& sceptre = profiles.Get(profiles.Find("sceptre"));
    const ItemStatProfile& shield = profiles.Get(profiles.Find("shield"));

    // The sceptre writes only the magic power's per level term; the rest is the default.
    CHECK(defaults.Get(ItemStat::MagicPower).perLevelAfterHalving == 2);
    CHECK(sceptre.Get(ItemStat::MagicPower).perLevelAfterHalving == 0);
    CHECK(sceptre.Get(ItemStat::MagicPower).excellent == defaults.Get(ItemStat::MagicPower).excellent);
    CHECK(sceptre.Get(ItemStat::Defense).ancient == defaults.Get(ItemStat::Defense).ancient);

    // A term the profile writes replaces the default's as a whole: the shield's
    // ancient term has no drop level divisor although the default's has one.
    const StatTerms& shieldDefense = shield.Get(ItemStat::Defense);
    CHECK_FALSE(shieldDefense.excellent.has_value());
    REQUIRE(shieldDefense.ancient.has_value());
    CHECK(shieldDefense.ancient->times == 20);
    CHECK(shieldDefense.ancient->dropLevelDivisor == 0);
    CHECK(shieldDefense.ancient->afterLevelBonus);

    // Requirements: the shield's terms replace the default's; the rest is the default.
    CHECK(profiles.GetExcellentDropLevelBonus() == 25);
    const RequirementTerms& defaultRequirements = defaults.requirements;
    CHECK(defaultRequirements.levelPerItemLevel == 0);
    CHECK(defaultRequirements.excellentLevelAdd == 20);
    CHECK(defaultRequirements.Get(ItemRequirementStat::Leadership).keepBase);
    CHECK(defaultRequirements.Get(ItemRequirementStat::Energy).percent == 4);
    const RequirementTerms& shieldRequirements = shield.requirements;
    CHECK(shieldRequirements.levelPerItemLevel == 4);
    CHECK(shieldRequirements.excellentLevelAdd == 20);
    CHECK(shieldRequirements.strengthPerOptionLevel == 5);
    const RequirementFormula& energy = shieldRequirements.Get(ItemRequirementStat::Energy);
    CHECK_FALSE(energy.keepBase);
    CHECK(energy.levelRequirementBasis);
    CHECK(energy.levelWeight == 0);
    CHECK(shieldRequirements.Get(ItemRequirementStat::Strength) ==
          defaultRequirements.Get(ItemRequirementStat::Strength));
    CHECK(sceptre.requirements.strengthPerOptionLevel == 0);

    // Level bonus: the table's value of the level (+15's above it) plus its part per level.
    CHECK(profiles.GetLevelBonus(defaults.Get(ItemStat::Defense), 10) == 31);
    CHECK(profiles.GetLevelBonus(defaults.Get(ItemStat::Defense), 20) == 66);
    CHECK(profiles.GetLevelBonus(shieldDefense, 20) == 20);

    // An index the file does not have is the default.
    CHECK(profiles.Get(99).Get(ItemStat::MagicPower).perLevelAfterHalving == 2);
}

TEST_CASE("Item stat profiles keep the profile rules [data][items]")
{
    ItemStatFile file = Read(SmallFile).file;
    StatProfileRule wings{.profile = "default"};
    wings.slots.set(static_cast<size_t>(ItemSlot::Wings));
    file.profileRules.push_back(wings);
    // The constructor does not validate: an unknown profile gives the default.
    file.profileRules.push_back(StatProfileRule{.profile = "staff"});
    const ItemStatProfiles profiles(file);
    REQUIRE(profiles.GetRules().size() == 4);
    CHECK(profiles.GetRuleProfile(0) == "sceptre");
    CHECK(profiles.GetRuleProfile(1) == "shield");
    CHECK(profiles.GetRuleProfile(2).empty());
    CHECK(profiles.GetRuleProfile(3).empty());
    CHECK(profiles.GetRuleProfile(NoStatProfileRule).empty());

    ItemDefinition shield;
    shield.group = 6;
    shield.weaponKind = WeaponKind::Shield;
    CHECK(profiles.FindRule(shield) == 1);
    shield.tags.Set(ItemTag::Sceptre);
    CHECK(profiles.FindRule(shield) == 0);
}

TEST_CASE("Item stat profiles without a file keep every stat at its base [data][items]")
{
    const ItemStatProfiles profiles;
    CHECK(profiles.Find("") == ItemStatProfiles::DefaultProfile);
    CHECK(profiles.Find("shield") == ItemStatProfiles::UnknownProfile);
    const StatTerms& damage = profiles.Get(ItemStatProfiles::DefaultProfile).Get(ItemStat::Damage);
    CHECK(profiles.GetLevelBonus(damage, 15) == 0);
    CHECK_FALSE(damage.excellent.has_value());
    CHECK_FALSE(damage.ancient.has_value());
    const RequirementTerms& requirements = profiles.Get(ItemStatProfiles::DefaultProfile).requirements;
    CHECK(requirements.Get(ItemRequirementStat::Strength).keepBase);
    CHECK(requirements.levelPerItemLevel == 0);
    CHECK(requirements.excellentLevelAdd == 0);
}

TEST_CASE("Item stat file errors [data][items]")
{
    using Severity = ItemDataIssueSeverity;
    CHECK(HasIssue(Read(Changed(R"("dropLevelBonus": {"excellent": 25, "ancient": 30},)", "")).issues, Severity::Fatal,
                   "dropLevelBonus"));
    CHECK(HasIssue(Read(Changed(R"("excellent": 25, )", "")).issues, Severity::Fatal, "dropLevelBonus.excellent"));
    CHECK(HasIssue(Read(Changed(R"("leadership": "base",)", "")).issues, Severity::Fatal,
                   "default.requirements.leadership"));
    CHECK(HasIssue(Read(Changed(R"("excellentLevelAdd": 20,)", "")).issues, Severity::Fatal,
                   "default.requirements.excellentLevelAdd"));
    CHECK(HasIssue(Read(Changed(R"("leadership": "base")", R"("leadership": "keep")")).issues, Severity::Fatal,
                   "default.requirements.leadership"));
    CHECK(HasIssue(Read(Changed(R"("basis": "levelRequirement")", R"("basis": "dropLevel")")).issues, Severity::Fatal,
                   "profiles.shield.requirements.energy.basis"));
    CHECK(HasIssue(Read(Changed(R"("strengthPerOptionLevel": 5)", R"("strengthPerOptionLevel": 256)")).issues,
                   Severity::Fatal, "profiles.shield.requirements.strengthPerOptionLevel"));
    CHECK(HasIssue(Read(Changed(R"("ancient": 30)", R"("ancient": -1)")).issues, Severity::Fatal,
                   "dropLevelBonus.ancient"));
    CHECK(
        HasIssue(Read(Changed(R"("name": "linear")", R"("name": "steps")")).issues, Severity::Fatal, "tables[1].name"));
    CHECK(HasIssue(Read(Changed(R"("name": "linear")", R"("name": "lin ear")")).issues, Severity::Fatal,
                   "tables[0].name"));
    // Item descriptions write a table as {table:<name>}, apart from the built-in values, so a table may be named like
    // one: the file reads without errors, with the table and the profile that names it.
    std::string levelTable = Changed(R"("name": "linear")", R"("name": "level")");
    const std::string linearUse = R"("table": "linear")";
    levelTable.replace(levelTable.find(linearUse), linearUse.size(), R"("table": "level")");
    const ReadResult level = Read(levelTable);
    CHECK_FALSE(HasFatal(level.issues));
    REQUIRE_FALSE(level.file.tables.empty());
    CHECK(level.file.tables[0].name == "level");
    CHECK(HasIssue(Read(Changed(R"("perLevel": 1)", R"("caption2": 1)")).issues, Severity::Fatal, "tables.linear"));
    CHECK(HasIssue(Read(Changed("[0, 3, 6, 9, 12, 15,", "[0, 3, 6, 9, 12,")).issues, Severity::Fatal,
                   "tables.steps.values"));
    CHECK(HasIssue(Read(Changed("[0, 3, 6,", "[0, -3, 6,")).issues, Severity::Fatal, "tables.steps.values[1]"));
    CHECK(HasIssue(Read(Changed(R"("magicDefense": {"table": "steps", "excellent": "none", "ancient": "none"})",
                                R"("magicDefense": {"table": "steps", "excellent": "none"})"))
                       .issues,
                   Severity::Fatal, "default.magicDefense.ancient"));
    CHECK(HasIssue(Read(Changed(R"(,
    "magicDefense": {"table": "steps", "excellent": "none", "ancient": "none"})",
                                ""))
                       .issues,
                   Severity::Fatal, "default.magicDefense"));
    CHECK(HasIssue(Read(Changed(R"(, "perLevelAfterHalving": 2})", "}")).issues, Severity::Fatal,
                   "default.magicPower.perLevelAfterHalving"));
    CHECK(HasIssue(Read(Changed(R"("defense": {"table": "linear")", R"("defense": {"table": "missing")")).issues,
                   Severity::Fatal, "profiles.shield.defense.table"));
    CHECK(HasIssue(Read(Changed(R"("dropLevelDivisor": 40)", R"("dropLevelDivisor": 0)")).issues, Severity::Fatal,
                   "default.damage.ancient.dropLevelDivisor"));
    CHECK(HasIssue(Read(Changed(R"("ancient": "none"})", R"("ancient": "nothing"})")).issues, Severity::Fatal,
                   "default.blockRate.ancient"));
    CHECK(HasIssue(Read(Changed(R"("name": "sceptre")", R"("name": "default")")).issues, Severity::Fatal,
                   "profiles[0].name"));
    CHECK(HasIssue(Read(Changed(R"("name": "sceptre")", R"("name": "shield")")).issues, Severity::Fatal,
                   "profiles[1].name"));
    CHECK(HasIssue(Read(Changed(R"("afterLevelBonus": true)", R"("afterLevelBonus": 1)")).issues, Severity::Fatal,
                   "profiles.shield.defense.ancient.afterLevelBonus"));
    CHECK(HasIssue(Read(R"({"formatVersion": 2})").issues, Severity::Fatal, "formatVersion"));

    // Profile rules
    const std::string sceptreRule = R"({"profile": "sceptre", "tags": ["sceptre"]})";
    const std::string shieldRule = R"({"profile": "shield", "group": [6], "weaponKind": ["shield"]})";
    CHECK(HasIssue(Read(Changed(R"("profileRules": [)", R"("profileRules": 5, "unused": [)")).issues, Severity::Fatal,
                   "profileRules"));
    CHECK(HasIssue(Read(Changed(sceptreRule, "7")).issues, Severity::Fatal, "profileRules[0]"));
    CHECK(HasIssue(Read(Changed(sceptreRule, R"({"tags": ["sceptre"]})")).issues, Severity::Fatal,
                   "profileRules[0].profile"));
    CHECK(HasIssue(Read(Changed(sceptreRule, R"({"profile": "staff", "tags": ["sceptre"]})")).issues, Severity::Fatal,
                   "profileRules[0].profile"));
    CHECK(HasIssue(Read(Changed(sceptreRule, R"({"profile": "sceptre", "colour": ["red"]})")).issues, Severity::Fatal,
                   "profileRules[0].colour"));
    CHECK(HasIssue(Read(Changed(shieldRule, R"({"profile": "shield", "weaponKind": []})")).issues, Severity::Fatal,
                   "profileRules[1].weaponKind"));
    CHECK(HasIssue(Read(Changed(shieldRule, R"({"profile": "shield", "weaponKind": ["shields"]})")).issues,
                   Severity::Fatal, "profileRules[1].weaponKind[0]"));
    CHECK(HasIssue(Read(Changed(shieldRule, R"({"profile": "shield", "group": [6, 16]})")).issues, Severity::Fatal,
                   "profileRules[1].group[1]"));
    CHECK(HasIssue(Read(Changed(shieldRule, R"({"profile": "shield", "requirements": ["luck"]})")).issues,
                   Severity::Fatal, "profileRules[1].requirements[0]"));
    // "default" names the default; a rule for it needs no profile.
    const ReadResult defaultRule = Read(Changed(sceptreRule, R"({"profile": "default", "slot": ["wings"]})"));
    CHECK(defaultRule.issues.empty());
}

TEST_CASE("Item stat file warnings [data][items]")
{
    using Severity = ItemDataIssueSeverity;
    const auto warningsOnly = [](const std::vector<ItemDataIssue>& issues)
    { return !HasFatal(issues) && !issues.empty(); };

    const ReadResult unknownKey =
        Read(Changed(R"("perLevelAfterHalving": 0})", R"("perLevelAfterHalving": 0, "speed": 1})"));
    CHECK(warningsOnly(unknownKey.issues));
    CHECK(HasIssue(unknownKey.issues, Severity::Warning, "profiles.sceptre.magicPower.speed"));

    // The per level term after halving belongs to the magic power only.
    const ReadResult wrongStat =
        Read(Changed(R"("ancient": "none"})", R"("ancient": "none", "perLevelAfterHalving": 1})"));
    CHECK(HasIssue(wrongStat.issues, Severity::Warning, "default.blockRate.perLevelAfterHalving"));

    // afterLevelBonus belongs to the ancient term only.
    const ReadResult excellentAfter =
        Read(Changed(R"("excellent": {"times": 12, "add": 4, "dropLevelDivisor": 5})",
                     R"("excellent": {"times": 12, "add": 4, "dropLevelDivisor": 5, "afterLevelBonus": true})"));
    CHECK(HasIssue(excellentAfter.issues, Severity::Warning, "default.defense.excellent.afterLevelBonus"));

    // A table no stat uses can be one for item descriptions, so the file alone does not warn about it
    // (ValidateTableUses does, with the items).
    const ReadResult unusedTable = Read(Changed(R"("defense": {"table": "linear", )", R"("defense": {)"));
    CHECK(unusedTable.issues.empty());
    CHECK_FALSE(ItemStatProfiles(unusedTable.file).IsTableUsedByStats("linear"));
    CHECK(ItemStatProfiles(unusedTable.file).IsTableUsedByStats("steps"));

    const ReadResult noEnglish = Read(Changed(R"({"en": "Steps"})", R"({"de": "Stufen"})"));
    CHECK(HasIssue(noEnglish.issues, Severity::Warning, "tables.steps.caption"));
}
