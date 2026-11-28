#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"
#include "HeroScope.h"
#include "TooltipDataScope.h"
#include "CharacterMachineScope.h"
#include "Network/Server/WSclient.h"

#include "Core/Utilities/StringUtils.h"
#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/DataHandler/SkillData/MasterSkillStorage.h"
#include "Data/GameData/BuffData/BuffDatabase.h"
#include "Data/GameData/BuffData/BuffLineTexts.h"
#include "Data/GameData/SkillData/MasterSkillDatabase.h"
#include "Data/GameData/SkillData/MasterSkillsJson.h"
#include "GameLogic/Buffs/BuffValues.h"
#include "GameLogic/Buffs/SkillBuffValues.h"
#include "GameLogic/Buffs/BuffServerValues.h"
#include "GameLogic/Buffs/BuffTooltipLines.h"
#include "GameLogic/Buffs/w_BuffTimeControl.h"
#include "GameLogic/Buffs/w_Buff.h"
#include "UI/HUD/Skills/SkillBuffLines.h"

#include <algorithm>
#include <bit>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
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

namespace
{
// An extended C1 07 (MUnique/OpenMU#1476) for buff `buff` on player 0x1234 with `seconds` left and these values.
std::vector<std::uint8_t>
ExtendedStatus(int buff, std::uint32_t seconds,
               std::initializer_list<std::tuple<std::string_view, std::uint8_t, float>> values)
{
    std::vector<std::uint8_t> packet = {0xC1, 0, 0x07, 1, 0x12, 0x34, static_cast<std::uint8_t>(buff)};
    for (int shift = 0; shift < 32; shift += 8)
    {
        packet.push_back(static_cast<std::uint8_t>(seconds >> shift));
    }
    packet.push_back(static_cast<std::uint8_t>(values.size()));
    for (const auto& [id, kind, value] : values)
    {
        const Data::Buffs::OpenMuAttributeId bytes = Data::Buffs::ParseOpenMuId(id);
        packet.insert(packet.end(), bytes.begin(), bytes.end());
        packet.push_back(kind);
        const auto bits = std::bit_cast<std::uint32_t>(value);
        for (int shift = 0; shift < 32; shift += 8)
        {
            packet.push_back(static_cast<std::uint8_t>(bits >> shift));
        }
    }
    packet[1] = static_cast<std::uint8_t>(packet.size());
    return packet;
}

constexpr std::string_view CriticalDamageBonusId = "33F53519-16F3-44C2-9D36-432C36329C78";
constexpr std::string_view SoulBarrierId = "CBC5404A-6232-4BF3-9B85-9AA0AE0F9BA4";
constexpr std::string_view UnknownId = "00000000-0000-0000-0000-000000000001";

std::string_view OpenMuIdOf(std::string_view name)
{
    return Data::Buffs::BuffAttributes[Data::Buffs::FindBuffAttribute(name)].openMuId;
}

std::vector<std::wstring> Texts(const std::vector<GameLogic::Buffs::BuffTooltipLine>& lines)
{
    std::vector<std::wstring> texts;
    for (const GameLogic::Buffs::BuffTooltipLine& line : lines)
    {
        texts.push_back(line.note ? L"(note) " + line.text : line.text);
    }
    return texts;
}

const Data::Buffs::BuffDefinition& Entry(int number)
{
    const Data::Buffs::BuffDefinition* buff = g_BuffDatabase.Find(number);
    REQUIRE(buff != nullptr);
    return *buff;
}
} // namespace

TEST_CASE("The extended C1 07 brings the time left and each value by its OpenMU attribute (buffs design B22) [buffs]")
{
    using GameLogic::Buffs::ReadExtendedBuffStatus;
    using GameLogic::Buffs::SentValueKind;

    const std::vector<std::uint8_t> packet =
        ExtendedStatus(5, 180, {{CriticalDamageBonusId, 0, 126.0f}, {UnknownId, 2, 7.0f}});
    REQUIRE(packet.size() == 12 + 2 * 21);
    const auto status = ReadExtendedBuffStatus(packet);
    REQUIRE(status.has_value());
    CHECK(status->playerKey == 0x1234);
    CHECK(status->buff == 5);
    CHECK(status->seconds == 180);
    REQUIRE(status->values.size() == 1);
    CHECK(status->values[0].attribute == Data::Buffs::FindBuffAttribute("criticalDamageBonus"));
    CHECK(status->values[0].kind == SentValueKind::AddRaw);
    CHECK(status->values[0].value == doctest::Approx(126.0f));
    // B23: an id the client doesn't know is kept apart, for the log.
    REQUIRE(status->unknown.size() == 1);
    CHECK(status->unknown[0].id == Data::Buffs::ParseOpenMuId(UnknownId));
    CHECK_FALSE(status->unknown[0].knownAttribute);
    // A known attribute with a kind the client doesn't know is kept apart too, as such.
    const auto unknownKind = ReadExtendedBuffStatus(ExtendedStatus(5, 180, {{CriticalDamageBonusId, 9, 126.0f}}));
    REQUIRE(unknownKind.has_value());
    CHECK(unknownKind->values.empty());
    REQUIRE(unknownKind->unknown.size() == 1);
    CHECK(unknownKind->unknown[0].knownAttribute);
    CHECK(unknownKind->unknown[0].kind == 9);

    // The short 0x07 of an older server or of someone else's buff, and the end of a buff, aren't the extended one.
    const std::vector<std::uint8_t> shortPacket = {0xC1, 7, 0x07, 1, 0x12, 0x34, 5};
    CHECK_FALSE(ReadExtendedBuffStatus(shortPacket).has_value());
    std::vector<std::uint8_t> ended = packet;
    ended[3] = 0;
    CHECK_FALSE(ReadExtendedBuffStatus(ended).has_value());
    // A length that doesn't fit the value count.
    std::vector<std::uint8_t> cut(packet.begin(), packet.end() - 1);
    cut[1] = static_cast<std::uint8_t>(cut.size());
    CHECK_FALSE(ReadExtendedBuffStatus(cut).has_value());

    // A factor raises its attribute by what it adds to 1.
    CHECK(GameLogic::Buffs::SentValueAmount({0, SentValueKind::Multiplicate, 1.25f}) == doctest::Approx(0.25));
    CHECK(GameLogic::Buffs::SentValueAmount({0, SentValueKind::AddFinal, 30.0f}) == doctest::Approx(30.0));
    // OpenMU keeps the critical damage chance as a fraction, the client in percent (the Scroll of Battle's 20).
    CHECK(GameLogic::Buffs::SentValueAmount({Data::Buffs::FindBuffAttribute("criticalDamageChance"),
                                             SentValueKind::AddRaw, 0.2f}) == doctest::Approx(20.0));
    CHECK(GameLogic::Buffs::SentValueAmount({Data::Buffs::FindBuffAttribute("criticalDamageChance"),
                                             SentValueKind::Multiplicate, 1.1f}) == doctest::Approx(10.0));
}

TEST_CASE("A buff's time is kept by its number and every new time restarts it (buffs design section 4, B25) [buffs]")
{
    SkillBuffScope scope;
    REQUIRE(scope.loaded);
    BuffTimeControlPtr times = BuffTimeControl::Make();
    std::uint64_t now = 1000;
    times->SetClock([&now] { return now; });
    const auto blessings = static_cast<eBuffState>(35); // Jack O'Lantern Blessings, 30 minutes
    const auto criticalDamage = static_cast<eBuffState>(5);

    times->RegisterBuffTime(criticalDamage, 180);
    CHECK(times->GetRemainingMilliseconds(criticalDamage) == 180000u);
    CHECK(times->GetSource(criticalDamage) == BuffTimeControl::TimeSource::Sent);
    now += 30000;
    CHECK(times->GetRemainingMilliseconds(criticalDamage) == 150000u);
    CHECK(times->GetFullMilliseconds(criticalDamage) == 180000u);
    // A recast restarts it (P2).
    times->SetSentTime(criticalDamage, 200);
    CHECK(times->GetRemainingMilliseconds(criticalDamage) == 200000u);
    // B25: a server that sends no time for the buff ends the time.
    times->SetSentTime(criticalDamage, 0);
    CHECK_FALSE(times->GetRemainingMilliseconds(criticalDamage).has_value());
    // Without a time from the server, a skill buff has none, an item buff its duration in Buffs.json.
    times->RegisterBuffTime(criticalDamage, 0);
    CHECK_FALSE(times->GetRemainingMilliseconds(criticalDamage).has_value());
    times->RegisterBuffTime(blessings, 0);
    CHECK(times->GetSource(blessings) == BuffTimeControl::TimeSource::Fallback);
    CHECK(times->GetFullMilliseconds(blessings) == static_cast<DWORD>(GameLogic::Buffs::GetDuration(Entry(35)) * 1000));
    // A time past what 32 bits of milliseconds hold (49.7 days) keeps its length.
    const std::uint32_t sixtyDays = 60 * 24 * 60 * 60;
    times->SetSentTime(criticalDamage, sixtyDays);
    CHECK(times->GetRemainingMilliseconds(criticalDamage) == std::uint64_t{sixtyDays} * 1000);
    // A time past its end is 0 left; the buff's end removes it.
    now += 100000000;
    CHECK(times->GetRemainingMilliseconds(blessings) == 0u);
    CHECK(times->UnRegisterBuffTime(blessings));
    CHECK_FALSE(times->GetRemainingMilliseconds(blessings).has_value());
}

TEST_CASE("The buff tooltip shows the server's values, '?' without them, and item buffs' own (B7, B8, B26) [buffs]")
{
    SkillBuffScope scope;
    REQUIRE(scope.loaded);
    using GameLogic::Buffs::BuildBuffValueLines;
    using GameLogic::Buffs::ReadExtendedBuffStatus;
    const BuffCasterStats owner = Stats({});

    GameLogic::Buffs::ServerBuffValues sent;
    CHECK(Texts(BuildBuffValueLines(Entry(5), sent, owner)) ==
          std::vector<std::wstring>{L"Critical damage +?", L"(note) The value depends on its caster"});

    // Without values, a part only a master skill fills gets no "?" line.
    const std::vector<std::wstring> berserkerUnknown = Texts(BuildBuffValueLines(Entry(81), sent, owner));
    CHECK(std::find(berserkerUnknown.begin(), berserkerUnknown.end(), L"Final wizardry and curse damage +? %") ==
          berserkerUnknown.end());
    CHECK(std::find(berserkerUnknown.begin(), berserkerUnknown.end(), L"Maximum mana +? %") != berserkerUnknown.end());

    auto read = ReadExtendedBuffStatus(ExtendedStatus(5, 180, {{CriticalDamageBonusId, 0, 126.9f}}));
    REQUIRE(read.has_value());
    sent.Set(5, read->values);
    CHECK(Texts(BuildBuffValueLines(Entry(5), sent, owner)) == std::vector<std::wstring>{L"Critical damage +126"});

    read = ReadExtendedBuffStatus(ExtendedStatus(4, 60, {{SoulBarrierId, 0, 0.3f}}));
    REQUIRE(read.has_value());
    sent.Set(4, read->values);
    CHECK(Texts(BuildBuffValueLines(Entry(4), sent, owner)).front() == L"Damage absorbed 30 %");

    // Berserker chains the server's values as its entry does: the damage bonuses times the mana multiplier, and the
    // wizardry multiplier (not one of the server's effect) adds the mana and the proficiency multipliers.
    read = ReadExtendedBuffStatus(ExtendedStatus(81, 0,
                                                 {{OpenMuIdOf("berserkerManaMultiplier"), 0, 0.5f},
                                                  {OpenMuIdOf("berserkerHealthDecrement"), 0, -0.2f},
                                                  {OpenMuIdOf("berserkerMinPhysDmgBonus"), 0, 150.0f},
                                                  {OpenMuIdOf("berserkerMaxPhysDmgBonus"), 0, 190.0f},
                                                  {OpenMuIdOf("berserkerCurseMultiplier"), 0, 0.0f},
                                                  {OpenMuIdOf("berserkerProficiencyMultiplier"), 0, 0.1f}}));
    REQUIRE(read.has_value());
    sent.Set(81, read->values);
    CHECK(Texts(BuildBuffValueLines(Entry(81), sent, owner)) ==
          std::vector<std::wstring>{L"Maximum mana +50 %", L"Maximum life -20 %", L"Minimum damage +75",
                                    L"Maximum damage +95", L"Wizardry damage +60 %", L"Curse damage +50 %",
                                    L"Final wizardry and curse damage +10 %"});

    // OpenMU sends Berserker's life decrement without the -10 % its class rules stop it at; the entry's maximum does.
    read = ReadExtendedBuffStatus(ExtendedStatus(81, 0, {{OpenMuIdOf("berserkerHealthDecrement"), 0, 0.1f}}));
    REQUIRE(read.has_value());
    sent.Set(81, read->values);
    CHECK(Texts(BuildBuffValueLines(Entry(81), sent, owner)) == std::vector<std::wstring>{L"Maximum life -10 %"});

    // Increase Health's master skill adds its part after the 200 the base stops at, so the sent 290 stays.
    read = ReadExtendedBuffStatus(ExtendedStatus(155, 0, {{OpenMuIdOf("totalVitality"), 2, 290.0f}}));
    REQUIRE(read.has_value());
    sent.Set(155, read->values);
    CHECK(Texts(BuildBuffValueLines(Entry(155), sent, owner)) == std::vector<std::wstring>{L"Vitality +290"});

    // B26: an item buff's lines from Buffs.json; one with a description shows that instead.
    CHECK(Texts(BuildBuffValueLines(Entry(45), sent, owner)) == std::vector<std::wstring>{L"Defense +60"});
    CHECK(Texts(BuildBuffValueLines(Entry(36), sent, owner)) == std::vector<std::wstring>{L"Attack power +25"});
    CHECK(BuildBuffValueLines(Entry(43), sent, owner).empty());
    CHECK(BuildBuffValueLines(Entry(104), sent, owner).empty());
}

TEST_CASE("The character stats sum the server's values over the buffs whose values come from the caster (B24) [buffs]")
{
    SkillBuffScope scope;
    REQUIRE(scope.loaded);
    using GameLogic::Buffs::SentValue;
    using GameLogic::Buffs::SentValueKind;
    const int minimumWizardry = Data::Buffs::FindBuffAttribute("minimumWizBaseDmg");
    const int totalVitality = Data::Buffs::FindBuffAttribute("totalVitality");

    GameLogic::Buffs::ServerBuffValues sent;
    // Expansion of Wizardry (82) and its master skill's buff (139) both count; the data says so, not the code.
    sent.Set(82, {SentValue{minimumWizardry, SentValueKind::AddRaw, 60.0f}});
    sent.Set(139, {SentValue{minimumWizardry, SentValueKind::AddRaw, 40.0f}});
    // An item buff's values are its own (Buffs.json), so the Elixir of Health's sent vitality doesn't count again.
    sent.Set(52, {SentValue{totalVitality, SentValueKind::AddRaw, 50.0f}});
    const auto all = [](int) { return true; };

    CHECK(GameLogic::Buffs::SumSentValues(sent, minimumWizardry, all) == doctest::Approx(100.0));
    CHECK(GameLogic::Buffs::SumSentValues(sent, minimumWizardry, [](int buff) { return buff == 82; }) ==
          doctest::Approx(60.0));
    CHECK_FALSE(GameLogic::Buffs::SumSentValues(sent, totalVitality, all).has_value());
    CHECK_FALSE(GameLogic::Buffs::SumSentValues(sent, minimumWizardry, [](int) { return false; }).has_value());
}

void ReceiveSkillStatus(std::span<const BYTE> ReceiveBuffer);
void ReceiveBuffState(const BYTE* ReceiveBuffer);
void ResetHeroForNewWorld();

namespace
{
// The hero as the packet handlers find it: a live character with a key in the client's character list, and a
// character machine for the stats the handlers recompute.
struct HandlerScope
{
    static constexpr int HeroKey = 0x1234;

    SkillBuffScope data;
    CharacterMachineScope machine;
    // One more than the list holds: FindCharacterIndex answers MAX_CHARACTERS_CLIENT for a key it doesn't find.
    std::unique_ptr<CHARACTER[]> characters = std::make_unique<CHARACTER[]>(MAX_CHARACTERS_CLIENT + 1);
    CHARACTER* previousCharacters = CharactersClient;
    CHARACTER* previousHero = Hero;

    HandlerScope()
    {
        CharactersClient = characters.get();
        Hero = &CharactersClient[0];
        Hero->Key = HeroKey;
        Hero->Object.Live = true;
        TheBuffTimeControl().Clear();
        GameLogic::Buffs::HeroServerBuffValues().ClearAll();
    }

    ~HandlerScope()
    {
        TheBuffTimeControl().Clear();
        GameLogic::Buffs::HeroServerBuffValues().ClearAll();
        Hero = previousHero;
        CharactersClient = previousCharacters;
    }

    HandlerScope(const HandlerScope&) = delete;
    HandlerScope& operator=(const HandlerScope&) = delete;
};

void SendItemBuff(int buff, int seconds, bool add)
{
    PMSG_ITEMEFFECTCANCEL packet{};
    packet.byEffectOption = add ? 0 : 1;
    packet.wEffectTime = seconds;
    packet.byBuffType = static_cast<BYTE>(buff);
    ReceiveBuffState(reinterpret_cast<const BYTE*>(&packet));
}
} // namespace

TEST_CASE("OpenMU's order: 0x2D starts an item buff's time, the extended C1 07 then wins (section 4, B25) [buffs]")
{
    HandlerScope scope;
    REQUIRE(scope.data.loaded);
    constexpr int Blessings = 35; // Jack O'Lantern Blessings, 30 minutes in Buffs.json
    const auto buff = static_cast<eBuffState>(Blessings);
    const std::string_view attackSpeed = OpenMuIdOf("attackSpeedAny");

    // 0x2D without a time (the effect's time is off on the server): the item buff's own duration, for now.
    SendItemBuff(Blessings, 0, true);
    CHECK(TheBuffTimeControl().GetSource(buff) == BuffTimeControl::TimeSource::Fallback);
    CHECK(TheBuffTimeControl().GetFullMilliseconds(buff) == 1800000u);

    // The extended C1 07 that follows says the server sends no time: none shows.
    const std::vector<std::uint8_t> noTime = ExtendedStatus(Blessings, 0, {{attackSpeed, 0, 10.0f}});
    std::vector<std::uint8_t> packet = noTime;
    packet[4] = static_cast<std::uint8_t>(HandlerScope::HeroKey >> 8);
    packet[5] = static_cast<std::uint8_t>(HandlerScope::HeroKey & 0xFF);
    ReceiveSkillStatus(packet);
    CHECK_FALSE(TheBuffTimeControl().GetRemainingMilliseconds(buff).has_value());
    REQUIRE(GameLogic::Buffs::HeroServerBuffValues().Find(Blessings) != nullptr);
    CHECK(GameLogic::Buffs::HeroServerBuffValues().Find(Blessings)->front().value == doctest::Approx(10.0f));

    // A recast of the buff that is already on: the new time and values replace the old ones.
    packet = ExtendedStatus(Blessings, 900, {{attackSpeed, 0, 15.0f}});
    packet[4] = static_cast<std::uint8_t>(HandlerScope::HeroKey >> 8);
    packet[5] = static_cast<std::uint8_t>(HandlerScope::HeroKey & 0xFF);
    ReceiveSkillStatus(packet);
    CHECK(TheBuffTimeControl().GetSource(buff) == BuffTimeControl::TimeSource::Sent);
    CHECK(TheBuffTimeControl().GetFullMilliseconds(buff) == 900000u);
    CHECK(GameLogic::Buffs::HeroServerBuffValues().Find(Blessings)->front().value == doctest::Approx(15.0f));

    // The end (0x2D off) takes both away.
    SendItemBuff(Blessings, 0, false);
    CHECK_FALSE(TheBuffTimeControl().GetRemainingMilliseconds(buff).has_value());
    CHECK(GameLogic::Buffs::HeroServerBuffValues().Find(Blessings) == nullptr);
}

TEST_CASE("Joining a world forgets the times and values the server sent for the hero's buffs (section 4) [buffs]")
{
    HandlerScope scope;
    REQUIRE(scope.data.loaded);
    constexpr int CriticalDamage = 5;
    std::vector<std::uint8_t> packet = ExtendedStatus(CriticalDamage, 180, {{CriticalDamageBonusId, 0, 126.0f}});
    packet[4] = static_cast<std::uint8_t>(HandlerScope::HeroKey >> 8);
    packet[5] = static_cast<std::uint8_t>(HandlerScope::HeroKey & 0xFF);
    ReceiveSkillStatus(packet);
    REQUIRE(TheBuffTimeControl().GetRemainingMilliseconds(static_cast<eBuffState>(CriticalDamage)).has_value());
    REQUIRE(GameLogic::Buffs::HeroServerBuffValues().Find(CriticalDamage) != nullptr);

    // What ReceiveJoinMapServer does before it reads the packet; the hero is the scope's own.
    ResetHeroForNewWorld();
    CHECK_FALSE(TheBuffTimeControl().GetRemainingMilliseconds(static_cast<eBuffState>(CriticalDamage)).has_value());
    CHECK(GameLogic::Buffs::HeroServerBuffValues().Find(CriticalDamage) == nullptr);
}
