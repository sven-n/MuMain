#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"
#include "HeroScope.h"
#include "TooltipDataScope.h"

#include "Core/Utilities/StringUtils.h"
#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/DataHandler/SkillData/MasterSkillStorage.h"
#include "Data/GameData/BuffData/BuffDatabase.h"
#include "Data/GameData/BuffData/BuffLineTexts.h"
#include "Data/GameData/SkillData/MasterSkillDatabase.h"
#include "Data/GameData/SkillData/MasterSkillsJson.h"
#include "GameLogic/Buffs/BuffValues.h"
#include "GameLogic/Buffs/SkillBuffValues.h"
#include "GameLogic/Buffs/w_Buff.h"
#include "UI/HUD/Skills/SkillBuffLines.h"

#include <algorithm>
#include <filesystem>
#include <limits>
#include <string>
#include <vector>

// The skill buffs' values (Data/Buffs/Buffs.json, buffs PR 2b): OpenMU's numbers with the hero as the caster, the
// master skills of Data/Skills/MasterSkills.json and the skill tooltip's lines. The expected numbers are worked out
// by hand from OpenMU's definitions (VersionSeasonSix/SkillsInitializer.cs and the effect initializers).
namespace
{
const std::filesystem::path ClientDirectory = std::filesystem::path(MU_TEST_DATA_DIR).parent_path();

using Data::Buffs::BuffTermAttribute;
using GameLogic::Buffs::BuffCasterStats;
using GameLogic::Buffs::ComputeSkillBuff;
using GameLogic::Buffs::CutToWhole;
using GameLogic::Buffs::SkillBuffValues;

// The master skills' values at level 10 as the server sends them: F502, F181, F803, F632 and F120 of OpenMU's
// formulas, f(10) = ((10 - 30)^3 + 25000) / 2994.
constexpr double LevelTen = (-8000.0 + 25000.0) / 2994.0;
constexpr float F502AtTen = static_cast<float>((0.8 + LevelTen) * 5);
constexpr float F181AtTen = static_cast<float>((1 + LevelTen) * 1.5);
constexpr float F803AtTen = static_cast<float>((0.8 + LevelTen) * 8);
constexpr float F632AtTen = static_cast<float>((0.85 + LevelTen) * 6);
constexpr float F120AtTen = static_cast<float>(1 + LevelTen);

BuffCasterStats Stats(std::initializer_list<std::pair<BuffTermAttribute, double>> values)
{
    BuffCasterStats stats{};
    for (const auto& [attribute, value] : values)
    {
        stats[static_cast<size_t>(attribute)] = value;
    }
    stats[static_cast<size_t>(BuffTermAttribute::TotalStrengthAndAgility)] =
        stats[static_cast<size_t>(BuffTermAttribute::TotalStrength)] +
        stats[static_cast<size_t>(BuffTermAttribute::TotalAgility)];
    return stats;
}

// The total of the buff's power-up of that attribute, or a number no value has.
double Total(const SkillBuffValues& values, std::string_view attribute)
{
    for (size_t index = 0; index < values.powerUpCount; ++index)
    {
        if (values.buff->powerUps[index].attribute == attribute)
        {
            return values.powerUps[index].total;
        }
    }
    return -1e9;
}

void Learn(HeroScope& hero, int skill, float value)
{
    hero.Hero().MasterSkillInfo[skill] = CSkillTreeInfo(10, value, value);
}

// The lines one per row, for a failing check's message.
std::string Shown(const std::vector<std::wstring>& lines)
{
    std::string text;
    for (const std::wstring& line : lines)
    {
        text += "[" + StringUtils::WideToNarrow(line.c_str()) + "]\n";
    }
    return text;
}

// The texts of the model's lines, in their order.
std::vector<std::wstring> LineTexts(const UI::Skills::Tooltip::Model& model)
{
    std::vector<std::wstring> texts;
    for (int line = 0; line < model.count; ++line)
    {
        texts.emplace_back(model.lines[line].text);
    }
    return texts;
}

struct SkillBuffScope
{
    ClientDataScope client{ClientDirectory};
    bool loaded = []
    {
        std::string errorMessage;
        return g_ItemDataHandler.Load(errorMessage);
    }();
    TooltipDataScope data;
    HeroScope hero;
};
} // namespace

TEST_CASE("Greater Damage is 3 + energy/7, the master skills multiply it and extend its duration [data][buffs]")
{
    SkillBuffScope scope;
    REQUIRE(scope.loaded);
    SkillBuffValues values;
    REQUIRE(
        ComputeSkillBuff(AT_SKILL_ATTACK, scope.hero.Hero(), Stats({{BuffTermAttribute::TotalEnergy, 1200}}), values));
    CHECK(values.buff->number == 1);
    CHECK(CutToWhole(Total(values, "greaterDamageBonus")) == 174);
    CHECK(CutToWhole(values.duration.total) == 60);

    // OpenMU adds the two master skills' parts before it multiplies: (3 + 1500/7) x (1 + 0.3239 + 0.3239) = 358.
    Learn(scope.hero, AT_SKILL_ATTACK_STR, F502AtTen);
    Learn(scope.hero, AT_SKILL_ATTACK_MASTERY, F502AtTen);
    REQUIRE(ComputeSkillBuff(AT_SKILL_ATTACK_MASTERY, scope.hero.Hero(),
                             Stats({{BuffTermAttribute::TotalEnergy, 1500}}), values));
    CHECK(values.powerUps[0].masterCount == 2);
    CHECK(CutToWhole(Total(values, "greaterDamageBonus")) == 358);
    CHECK(CutToWhole(values.duration.total) == 92);

    // Shown as the Strengthener, the Mastery above it doesn't count.
    REQUIRE(ComputeSkillBuff(AT_SKILL_ATTACK_STR, scope.hero.Hero(), Stats({{BuffTermAttribute::TotalEnergy, 1500}}),
                             values));
    CHECK(values.powerUps[0].masterCount == 1);
    CHECK(CutToWhole(values.duration.total) == 60);
}

TEST_CASE("Soul Barrier absorbs 10 % + energy/200 % + agility/50 %, and its proficiency extends it [data][buffs]")
{
    SkillBuffScope scope;
    REQUIRE(scope.loaded);
    const BuffCasterStats stats = Stats({{BuffTermAttribute::TotalEnergy, 2000},
                                         {BuffTermAttribute::TotalAgility, 500},
                                         {BuffTermAttribute::MaximumMana, 3000}});
    SkillBuffValues values;
    REQUIRE(ComputeSkillBuff(AT_SKILL_SOUL_BARRIER, scope.hero.Hero(), stats, values));
    CHECK(CutToWhole(Total(values, "soulBarrierReceiveDecrement") * 100) == 30);
    CHECK(CutToWhole(Total(values, "soulBarrierManaTollPerHit")) == 60);
    CHECK(CutToWhole(values.duration.total) == 110);

    // The proficiency (404) counts the strengthener below it (403): absorb + 10 %, duration + 51 s. The old tooltip
    // added 404's seconds to the absorb instead.
    Learn(scope.hero, AT_SKILL_SOUL_BARRIER_STR, F181AtTen);
    Learn(scope.hero, AT_SKILL_SOUL_BARRIER_PROFICIENCY, F803AtTen);
    REQUIRE(ComputeSkillBuff(AT_SKILL_SOUL_BARRIER_PROFICIENCY, scope.hero.Hero(), stats, values));
    CHECK(CutToWhole(Total(values, "soulBarrierReceiveDecrement") * 100) == 40);
    CHECK(CutToWhole(values.duration.total) == 161);
}

TEST_CASE("Critical Damage Increase stops its duration at 180 s before its master skill extends it [data][buffs]")
{
    SkillBuffScope scope;
    REQUIRE(scope.loaded);
    const BuffCasterStats stats =
        Stats({{BuffTermAttribute::TotalEnergy, 2000}, {BuffTermAttribute::TotalLeadership, 1500}});
    SkillBuffValues values;
    REQUIRE(ComputeSkillBuff(AT_SKILL_ADD_CRITICAL, scope.hero.Hero(), stats, values));
    CHECK(CutToWhole(Total(values, "criticalDamageBonus")) == 126);
    CHECK(values.duration.stoppedAtMaximum);
    CHECK(CutToWhole(values.duration.total) == 180);

    Learn(scope.hero, AT_SKILL_ADD_CRITICAL_STR1, F632AtTen);
    REQUIRE(ComputeSkillBuff(AT_SKILL_ADD_CRITICAL_STR1, scope.hero.Hero(), stats, values));
    CHECK(CutToWhole(Total(values, "criticalDamageBonus")) == 165);

    Learn(scope.hero, AT_SKILL_ADD_CRITICAL_STR2, F803AtTen);
    REQUIRE(ComputeSkillBuff(AT_SKILL_ADD_CRITICAL_STR2, scope.hero.Hero(), stats, values));
    CHECK(CutToWhole(Total(values, "criticalDamageBonus")) == 165);
    CHECK(CutToWhole(values.duration.total) == 180 + CutToWhole(F803AtTen));
}

TEST_CASE("Swell Life counts the party members near the caster [data][buffs]")
{
    SkillBuffScope scope;
    REQUIRE(scope.loaded);
    SkillBuffValues values;
    REQUIRE(ComputeSkillBuff(AT_SKILL_SWELL_LIFE, scope.hero.Hero(),
                             Stats({{BuffTermAttribute::TotalEnergy, 500},
                                    {BuffTermAttribute::TotalVitality, 1500},
                                    {BuffTermAttribute::NearbyPartyMemberCount, 3}}),
                             values));
    // 0.12 + 500/2000 + 1500/10000 + 3/100 = 0.55, for 60 + 500/180 = 62 s.
    CHECK(CutToWhole(Total(values, "swellLifeHealthIncrease") * 100) == 55);
    CHECK(CutToWhole(values.duration.total) == 62);
}

TEST_CASE("Berserker's damage and wizardry follow its mana bonus [data][buffs]")
{
    SkillBuffScope scope;
    REQUIRE(scope.loaded);
    const BuffCasterStats stats = Stats({{BuffTermAttribute::TotalEnergy, 1500},
                                         {BuffTermAttribute::TotalStrength, 300},
                                         {BuffTermAttribute::TotalAgility, 400}});
    SkillBuffValues values;
    REQUIRE(ComputeSkillBuff(AT_SKILL_ALICE_BERSERKER, scope.hero.Hero(), stats, values));
    CHECK(CutToWhole(Total(values, "berserkerManaMultiplier") * 100) == 50);
    CHECK(CutToWhole(Total(values, "berserkerHealthDecrement") * 100) == -15);
    CHECK(CutToWhole(Total(values, "berserkerMinPhysDmgBonus")) == 77);
    CHECK(CutToWhole(Total(values, "berserkerMaxPhysDmgBonus")) == 91);
    CHECK(CutToWhole(Total(values, "berserkerWizardryMultiplier") * 100) == 50);
    CHECK(CutToWhole(Total(values, "berserkerCurseMultiplier") * 100) == 50);
    CHECK(CutToWhole(values.duration.total) == 105);

    // At most 10 % less life, from 1800 energy on.
    REQUIRE(ComputeSkillBuff(AT_SKILL_ALICE_BERSERKER, scope.hero.Hero(),
                             Stats({{BuffTermAttribute::TotalEnergy, 3000}}), values));
    CHECK(CutToWhole(Total(values, "berserkerHealthDecrement") * 100) == -10);

    // Its proficiency (470) casts Berserker (81), not the skill data's 151, and raises the proficiency multiplier by
    // 10 %: OpenMU adds it to the wizardry multiplier (ClassSummoner.cs) and multiplies the final wizardry and curse
    // damage by it (AttackableExtensions.cs).
    Learn(scope.hero, AT_SKILL_BerserkerProficiency, F181AtTen);
    REQUIRE(ComputeSkillBuff(AT_SKILL_BerserkerProficiency, scope.hero.Hero(), stats, values));
    CHECK(values.buff->number == 81);
    CHECK(CutToWhole(Total(values, "berserkerProficiencyMultiplier") * 100) == 10);
    CHECK(CutToWhole(Total(values, "berserkerWizardryMultiplier") * 100) == 60);
    CHECK(CutToWhole(Total(values, "berserkerCurseMultiplier") * 100) == 50);
}

TEST_CASE("The other skill buffs have OpenMU's values and maximums [data][buffs]")
{
    SkillBuffScope scope;
    REQUIRE(scope.loaded);
    SkillBuffValues values;
    // Expansion of Wizardry: energy/45, at most 100.
    REQUIRE(ComputeSkillBuff(AT_SKILL_EXPANSION_OF_WIZARDRY, scope.hero.Hero(),
                             Stats({{BuffTermAttribute::TotalEnergy, 5000}}), values));
    CHECK(values.powerUps[0].stoppedAtMaximum);
    CHECK(CutToWhole(Total(values, "minimumWizBaseDmg")) == 100);
    // Ignore Defense: -0.0104 + energy/10000.
    REQUIRE(ComputeSkillBuff(AT_SKILL_ATT_UP_OURFORCES, scope.hero.Hero(),
                             Stats({{BuffTermAttribute::TotalEnergy, 1000}}), values));
    CHECK(CutToWhole(Total(values, "defenseIgnoreChance") * 100) == 8);
    // Increase Health: 16.8 + energy/10, at most 200.
    REQUIRE(ComputeSkillBuff(AT_SKILL_HP_UP_OURFORCES, scope.hero.Hero(),
                             Stats({{BuffTermAttribute::TotalEnergy, 1237}}), values));
    CHECK(CutToWhole(Total(values, "totalVitality")) == 140);
    REQUIRE(ComputeSkillBuff(AT_SKILL_HP_UP_OURFORCES, scope.hero.Hero(),
                             Stats({{BuffTermAttribute::TotalEnergy, 2000}}), values));
    CHECK(CutToWhole(Total(values, "totalVitality")) == 200);
    // Increase Block: 2 + energy/10, flat as OpenMU adds it.
    REQUIRE(ComputeSkillBuff(AT_SKILL_DEF_UP_OURFORCES, scope.hero.Hero(),
                             Stats({{BuffTermAttribute::TotalEnergy, 500}}), values));
    CHECK(CutToWhole(Total(values, "increaseBlockBonus")) == 52);
    // Damage Reflection: 0.30 + energy/4200, at most 0.60.
    REQUIRE(ComputeSkillBuff(AT_SKILL_ALICE_THORNS, scope.hero.Hero(), Stats({{BuffTermAttribute::TotalEnergy, 3000}}),
                             values));
    CHECK(CutToWhole(Total(values, "damageReflection") * 100) == 60);
    // Infinity Arrow's strengthener (441) casts Infinity Arrow (6) and adds its damage.
    Learn(scope.hero, AT_SKILL_INFINITY_ARROW_STR, F120AtTen);
    REQUIRE(ComputeSkillBuff(AT_SKILL_INFINITY_ARROW_STR, scope.hero.Hero(), Stats({}), values));
    CHECK(values.buff->number == 6);
    CHECK(Total(values, "attackDamageIncrease") == doctest::Approx(F120AtTen / 100.0));
}

TEST_CASE("A chain to a power-up past the ones the computation keeps is left out [data][buffs]")
{
    SkillBuffScope scope;
    REQUIRE(scope.loaded);
    std::vector<Data::Buffs::BuffDefinition> buffs = g_BuffDatabase.GetAll();
    const auto greaterDamage = std::find_if(buffs.begin(), buffs.end(),
                                            [](const Data::Buffs::BuffDefinition& buff) { return buff.number == 1; });
    REQUIRE(greaterDamage != buffs.end());
    // Power-ups up to one past the kept ones, each of another attribute; the first adds the last one's value.
    for (const Data::Buffs::BuffAttribute& attribute : Data::Buffs::BuffAttributes)
    {
        if (greaterDamage->powerUps.size() > GameLogic::Buffs::MaxSkillBuffPowerUps)
        {
            break;
        }
        if (attribute.name != greaterDamage->powerUps[0].attribute)
        {
            Data::Buffs::BuffPowerUp powerUp;
            powerUp.attribute = std::string(attribute.name);
            powerUp.value.values = 1000;
            greaterDamage->powerUps.push_back(powerUp);
        }
    }
    greaterDamage->powerUps[0].valueOf = {greaterDamage->powerUps.back().attribute};
    g_BuffDatabase.Build(buffs, "en");

    SkillBuffValues values;
    REQUIRE(
        ComputeSkillBuff(AT_SKILL_ATTACK, scope.hero.Hero(), Stats({{BuffTermAttribute::TotalEnergy, 1200}}), values));
    CHECK(values.powerUpCount == GameLogic::Buffs::MaxSkillBuffPowerUps);
    CHECK(CutToWhole(Total(values, "greaterDamageBonus")) == 174);
}

TEST_CASE("A master skill that only extends the duration is no part of a power-up the attributes lack [data][buffs]")
{
    SkillBuffScope scope;
    REQUIRE(scope.loaded);
    std::vector<Data::Buffs::BuffDefinition> buffs = g_BuffDatabase.GetAll();
    const auto soulBarrier = std::find_if(buffs.begin(), buffs.end(), [](const Data::Buffs::BuffDefinition& buff)
                                          { return buff.name == "soulBarrier"; });
    REQUIRE(soulBarrier != buffs.end());
    // Buffs.json reports an unknown attribute as an error and loads it with no index, like a master skill that raises
    // nothing.
    Data::Buffs::BuffPowerUp unknown;
    unknown.attribute = "noSuchAttribute";
    unknown.value.values = 5;
    soulBarrier->powerUps.push_back(unknown);
    g_BuffDatabase.Build(buffs, "en");

    Learn(scope.hero, AT_SKILL_SOUL_BARRIER_STR, F181AtTen);
    Learn(scope.hero, AT_SKILL_SOUL_BARRIER_PROFICIENCY, F803AtTen);
    SkillBuffValues values;
    REQUIRE(ComputeSkillBuff(AT_SKILL_SOUL_BARRIER_PROFICIENCY, scope.hero.Hero(), Stats({}), values));
    CHECK(Total(values, "noSuchAttribute") == doctest::Approx(5));
    CHECK(values.powerUps[values.powerUpCount - 1].masterCount == 0);
}

TEST_CASE("A total out of an int's range is cut to the nearest int [data][buffs]")
{
    CHECK(CutToWhole(1e300) == std::numeric_limits<int>::max());
    CHECK(CutToWhole(-1e300) == std::numeric_limits<int>::min());
    CHECK(CutToWhole(-2.7) == -2);
}

TEST_CASE("The tooltip data scope puts the buff line texts back [data][buffs]")
{
    const Data::Items::ItemOptionTexts before = g_BuffLineTexts.GetTexts();
    const std::string beforeLocale = g_BuffLineTexts.GetDisplayLocale();
    g_BuffLineTexts.SetTexts({}, "de");
    {
        SkillBuffScope scope;
        CHECK(g_BuffLineTexts.Get(Data::Buffs::BuffLineText::Duration) != nullptr);
    }
    CHECK(g_BuffLineTexts.GetDisplayLocale() == "de");
    CHECK(g_BuffLineTexts.Get(Data::Buffs::BuffLineText::Duration) == nullptr);
    g_BuffLineTexts.SetTexts(before, beforeLocale);
}

TEST_CASE("A skill that casts no buff, or a buff without caster values, computes nothing [data][buffs]")
{
    SkillBuffScope scope;
    REQUIRE(scope.loaded);
    SkillBuffValues values;
    CHECK_FALSE(ComputeSkillBuff(AT_SKILL_FIREBALL, scope.hero.Hero(), Stats({}), values));
    CHECK(values.buff == nullptr);
}

TEST_CASE("The character stats don't count a caster's buff: only the server knows the caster [data][buffs]")
{
    SkillBuffScope scope;
    REQUIRE(scope.loaded);
    Buff active;
    active.RegisterBuff(static_cast<eBuffState>(130));
    CHECK(GameLogic::Buffs::SumPowerUps(active, "totalVitality") == 0);
}

TEST_CASE("The skill tooltip lists a buff value's parts with their rules [data][buffs][tooltip]")
{
    SkillBuffScope scope;
    REQUIRE(scope.loaded);
    const BuffCasterStats stats =
        Stats({{BuffTermAttribute::TotalEnergy, 2000}, {BuffTermAttribute::TotalLeadership, 1500}});
    SkillBuffValues values;
    REQUIRE(ComputeSkillBuff(AT_SKILL_ADD_CRITICAL, scope.hero.Hero(), stats, values));
    UI::Skills::Tooltip::Model model;
    model.Reset();
    UI::Skills::Tooltip::AddSkillBuffLines(model, values, stats);
    const std::vector<std::wstring> expected = {
        L"Critical damage +126",
        L"  energy 2000: +66 (1 per 30)",
        L"  command 1500: +60 (1 per 25)",
        L"Duration: 180 seconds",
        L"  base: +60 s",
        L"  energy 2000: +200 s (1 per 10)",
        L"  at most 180 s",
    };
    INFO(Shown(LineTexts(model)));
    CHECK(LineTexts(model) == expected);
    CHECK(model.lines[0].color == UI::Skills::Tooltip::LineColor::White);
    CHECK(model.lines[1].color == UI::Skills::Tooltip::LineColor::Gray);

    // A master skill's part is named by the skill, in percent for one that multiplies.
    Learn(scope.hero, AT_SKILL_ATTACK_STR, F502AtTen);
    const BuffCasterStats elf = Stats({{BuffTermAttribute::TotalEnergy, 1500}});
    REQUIRE(ComputeSkillBuff(AT_SKILL_ATTACK_STR, scope.hero.Hero(), elf, values));
    model.Reset();
    UI::Skills::Tooltip::AddSkillBuffLines(model, values, elf);
    const std::vector<std::wstring> lines = LineTexts(model);
    INFO(Shown(lines));
    REQUIRE(lines.size() >= 4);
    CHECK(lines[0] == L"Damage +287");
    CHECK(lines[1] == L"  base: +3");
    CHECK(lines[2] == L"  energy 1500: +214 (1 per 7)");
    CHECK(lines[3] == std::wstring(L"  ") + SkillAttribute[AT_SKILL_ATTACK_STR].Name + L": +32.4 %");
}

TEST_CASE("The skill tooltip shows percents and Berserker's chained values [data][buffs][tooltip]")
{
    SkillBuffScope scope;
    REQUIRE(scope.loaded);
    Learn(scope.hero, AT_SKILL_SOUL_BARRIER_STR, F181AtTen);
    Learn(scope.hero, AT_SKILL_SOUL_BARRIER_PROFICIENCY, F803AtTen);
    const BuffCasterStats wizard = Stats({{BuffTermAttribute::TotalEnergy, 2000},
                                          {BuffTermAttribute::TotalAgility, 500},
                                          {BuffTermAttribute::MaximumMana, 3000}});
    SkillBuffValues values;
    REQUIRE(ComputeSkillBuff(AT_SKILL_SOUL_BARRIER_PROFICIENCY, scope.hero.Hero(), wizard, values));
    UI::Skills::Tooltip::Model model;
    model.Reset();
    UI::Skills::Tooltip::AddSkillBuffLines(model, values, wizard);
    const std::wstring strengthener = SkillAttribute[AT_SKILL_SOUL_BARRIER_STR].Name;
    const std::wstring proficiency = SkillAttribute[AT_SKILL_SOUL_BARRIER_PROFICIENCY].Name;
    const std::vector<std::wstring> soulBarrier = {
        L"Damage absorbed 40 %",
        L"  base: +10 %",
        L"  energy 2000: +10 % (1 per 200)",
        L"  agility 500: +10 % (1 per 50)",
        L"  " + strengthener + L": +10 %",
        L"Mana per absorbed hit 60",
        L"  maximum mana 3000: +60 (x0.02)",
        L"Duration: 161 seconds",
        L"  base: +60 s",
        L"  energy 2000: +50 s (1 per 40)",
        L"  " + proficiency + L": +51 s",
    };
    INFO(Shown(LineTexts(model)));
    CHECK(LineTexts(model) == soulBarrier);

    Learn(scope.hero, AT_SKILL_BerserkerProficiency, F181AtTen);
    const BuffCasterStats summoner = Stats({{BuffTermAttribute::TotalEnergy, 1500},
                                            {BuffTermAttribute::TotalStrength, 300},
                                            {BuffTermAttribute::TotalAgility, 400}});
    REQUIRE(ComputeSkillBuff(AT_SKILL_BerserkerProficiency, scope.hero.Hero(), summoner, values));
    model.Reset();
    UI::Skills::Tooltip::AddSkillBuffLines(model, values, summoner);
    const std::wstring berserkerProficiency = SkillAttribute[AT_SKILL_BerserkerProficiency].Name;
    const std::vector<std::wstring> berserker = {
        L"Maximum mana +50 %",
        L"  energy 1500: +50 % (1 per 30)",
        L"Maximum life -15 %",
        L"  base: -40 %",
        L"  energy 1500: +25 % (1 per 60)",
        L"Minimum damage +77",
        L"  base: +140",
        L"  strength and agility 700: +14 (1 per 50)",
        L"  x Maximum mana +50 %",
        L"Maximum damage +91",
        L"  base: +160",
        L"  strength and agility 700: +23 (1 per 30)",
        L"  x Maximum mana +50 %",
        L"Wizardry damage +60 %",
        L"  Maximum mana +50 %",
        L"  Final wizardry and curse damage +10 %",
        L"Curse damage +50 %",
        L"  Maximum mana +50 %",
        L"Final wizardry and curse damage +10 %",
        L"  " + berserkerProficiency + L": +10 %",
        L"Duration: 105 seconds",
        L"  base: +30 s",
        L"  energy 1500: +75 s (1 per 20)",
    };
    INFO(Shown(LineTexts(model)));
    CHECK(LineTexts(model) == berserker);
}

TEST_CASE("The skill tooltip shows a percent past an int's range as the largest int [data][buffs][tooltip]")
{
    SkillBuffScope scope;
    REQUIRE(scope.loaded);
    std::vector<Data::Buffs::BuffDefinition> buffs = g_BuffDatabase.GetAll();
    const auto berserker = std::find_if(buffs.begin(), buffs.end(),
                                        [](const Data::Buffs::BuffDefinition& buff) { return buff.number == 81; });
    REQUIRE(berserker != buffs.end());
    // A typo in the data: a fraction of 1e8 is 1e10 %.
    berserker->powerUps[0].value = Data::Buffs::BuffValue{1e8};
    g_BuffDatabase.Build(buffs, "en");

    SkillBuffValues values;
    REQUIRE(ComputeSkillBuff(AT_SKILL_ALICE_BERSERKER, scope.hero.Hero(), Stats({}), values));
    UI::Skills::Tooltip::Model model;
    model.Reset();
    UI::Skills::Tooltip::AddSkillBuffLines(model, values, Stats({}));
    const std::vector<std::wstring> lines = LineTexts(model);
    INFO(Shown(lines));
    REQUIRE(lines.size() >= 2);
    CHECK(lines[0] == L"Maximum mana +2147483647 %");
    CHECK(lines[1] == L"  base: +2147483647 %");
}

TEST_CASE("MasterSkills.json's check finds a skill's buff as the tooltip does, down the chain [data][buffs]")
{
    SkillBuffScope scope;
    REQUIRE(scope.loaded);
    // A castsBuff on the Strengthener counts for the Mastery above it, which has none, in the tooltip and the check:
    // with Berserker (81) there, the Mastery's greaterDamageBonus is no power-up of its buff.
    std::vector<Data::Skills::MasterSkillDefinition> skills = g_MasterSkillDatabase.GetAll();
    const auto find = [&](int number)
    {
        return std::find_if(skills.begin(), skills.end(), [number](const Data::Skills::MasterSkillDefinition& skill)
                            { return skill.skill == number; });
    };
    REQUIRE(find(AT_SKILL_ATTACK_STR) != skills.end());
    REQUIRE(find(AT_SKILL_ATTACK_MASTERY) != skills.end());
    REQUIRE(find(AT_SKILL_ATTACK_MASTERY)->castsBuff == 0);
    CHECK(Data::Skills::FindMasterSkillBuff(AT_SKILL_ATTACK_MASTERY, skills) == 1);
    find(AT_SKILL_ATTACK_STR)->castsBuff = 81;
    CHECK(Data::Skills::FindMasterSkillBuff(AT_SKILL_ATTACK_MASTERY, skills) == 81);

    std::vector<Data::Items::ItemDataIssue> issues;
    Data::Skills::ValidateMasterSkillBuffs(
        skills, {}, g_BuffDatabase.GetAll(),
        [&](int skill) { return Data::Skills::FindMasterSkillBuff(skill, skills); }, "MasterSkills.json", issues);
    const auto mastery = static_cast<size_t>(find(AT_SKILL_ATTACK_MASTERY) - skills.begin());
    const std::string field = Data::Skills::MasterSkillField(mastery, "raises");
    INFO(field);
    CHECK(std::any_of(issues.begin(), issues.end(), [&](const Data::Items::ItemDataIssue& issue)
                      { return issue.field == field && issue.message.find("berserker (81)") != std::string::npos; }));
}
