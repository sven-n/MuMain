#include "stdafx.h"
#include "UI/NewUI/HUD/Skills/SkillTooltipModel.h"

#include "Character/CharacterManager.h"
#include "Core/Globals/_enum.h"
#include "I18N/All.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzInventory.h"   // PartyNumber, STRP_*
#include "GameLogic/Skills/SkillManager.h"
#include "UI/Legacy/UIJewelHarmony.h"     // StrengthenCapability
#include "UI/Legacy/UIManager.h"          // g_pUIJewelHarmonyinfo

// Forward declaration: defined in ZzzInventory.cpp, no header.
bool GetAttackDamage(int* iMinDamage, int* iMaxDamage);

namespace UI::Skills::Tooltip
{

namespace
{
constexpr int GLOBAL_TEXT_REQUIRED_LEVEL = 76;
constexpr int GLOBAL_TEXT_REQUIRED_STRENGTH = 73;
constexpr int GLOBAL_TEXT_REQUIRED_DEXTERITY = 75;
constexpr int GLOBAL_TEXT_REQUIRED_ENERGY = 77;
constexpr int GLOBAL_TEXT_REQUIRED_CHARISMA = 698;
constexpr int GLOBAL_TEXT_NEED_MORE_STAT = 74;
constexpr int GLOBAL_TEXT_MASTERY_TYPE_BASE = 1080;

// SkillAttribute.Delay is stored in milliseconds; the tooltip displays it as
// seconds with one decimal. No GlobalText entry exists for this line yet, so
// the format string lives here until a localized one is added.
constexpr wchar_t kCooldownFormat[] = L"Cooldown: %.1f sec";
constexpr float kMillisPerSecond = 1000.0f;

bool IsCastleSiegeOnlySkill(int skillType)
{
    switch (skillType)
    {
    case AT_SKILL_RUSH:
    case AT_SKILL_SPACE_SPLIT:
    case AT_SKILL_DEEPIMPACT:
    case AT_SKILL_JAVELIN:
    case AT_SKILL_SPIRAL_SLASH:
    case AT_SKILL_DEATH_CANNON:
    case AT_SKILL_OCCUPY:
        return true;
    default:
        return false;
    }
}

// ---- Tiny line emitters. All operate on outModel.count; bounds are clamped
//      to MAX_TOOLTIP_LINES to avoid overflowing the fixed buffer.

Line& NextSlot(Model& m)
{
    if (m.count >= MAX_TOOLTIP_LINES) return m.lines[MAX_TOOLTIP_LINES - 1];
    return m.lines[m.count++];
}

void AddBlank(Model& m)
{
    Line& l = NextSlot(m);
    l.text[0] = L'\n';
    l.text[1] = L'\0';
    l.color = LineColor::White;
    l.isBold = false;
    l.isBlank = true;
    ++m.skipCount;
}

void AddRaw(Model& m, const wchar_t* text, LineColor color, bool bold = false)
{
    Line& l = NextSlot(m);
    wcsncpy(l.text, text, MAX_TOOLTIP_LINE_TEXT - 1);
    l.text[MAX_TOOLTIP_LINE_TEXT - 1] = L'\0';
    l.color = color;
    l.isBold = bold;
    l.isBlank = false;
}

void AddFormatted(Model& m, int globalTextIdx, LineColor color, int v1)
{
    Line& l = NextSlot(m);
    mu_swprintf(l.text, I18N::Game::Lookup(globalTextIdx), v1);
    l.color = color;
    l.isBold = false;
    l.isBlank = false;
}

void AddFormatted(Model& m, int globalTextIdx, LineColor color, int v1, int v2)
{
    Line& l = NextSlot(m);
    mu_swprintf(l.text, I18N::Game::Lookup(globalTextIdx), v1, v2);
    l.color = color;
    l.isBold = false;
    l.isBlank = false;
}

void AddFormattedWide(Model& m, int globalTextIdx, LineColor color, const wchar_t* arg)
{
    Line& l = NextSlot(m);
    mu_swprintf(l.text, I18N::Game::Lookup(globalTextIdx), arg);
    l.color = color;
    l.isBold = false;
    l.isBlank = false;
}

// Requirement line. In game mode, compares value vs current and colors
// red/white accordingly, optionally emitting a `(lacking N)` deficit line.
// In editor mode (currentValue == -1), always white, no deficit line.
void AddRequirementLine(Model& m, int requiredValue, int currentValue, int reqStringIndex)
{
    if (requiredValue <= 0) return;

    const bool editorMode = (currentValue < 0);
    const bool requirementMet = editorMode || (currentValue >= requiredValue);

    {
        Line& l = NextSlot(m);
        mu_swprintf(l.text, I18N::Game::Lookup(reqStringIndex), requiredValue);
        l.color = requirementMet ? LineColor::White : LineColor::Red;
        l.isBold = false;
        l.isBlank = false;
    }

    if (editorMode || requirementMet) return;

    Line& deficit = NextSlot(m);
    mu_swprintf(deficit.text, I18N::Game::Lookup(GLOBAL_TEXT_NEED_MORE_STAT), requiredValue - currentValue);
    deficit.color = LineColor::Red;
    deficit.isBold = false;
    deficit.isBlank = false;
}

// =====================================================================
// Section emitters. Tooltip layout, top to bottom:
//
//   [HEADER]        blank (top padding), name (bold blue), blank
//   [BANNER TOP]    red-bg flavor banners (e.g. Infinity Arrow info)
//   [BODY damage]   character-specific damage / buff values
//   [BODY stats]    range, mana, ability gauge
//   [REQUIREMENTS]  level / str / dex / energy / cha (color-coded in game mode)
//   [BANNER BOTTOM] red-bg warnings, brand info, flavor notes
//   [BLUE TAGS]     blue descriptor tags (mastery type, Expansion of Wizardry)
//
// Each populated section ends with a blank line, which the renderer draws
// at half text-height — giving ~5px spacing between sections and bottom
// padding. Empty sections add nothing, so absent content doesn't compound
// the gap.
//
// Each emitter is called unconditionally from BuildModel; the emitter
// itself decides whether the section has anything to add.
// =====================================================================

// Emit a trailing blank as a section separator, but only if the section
// actually added at least one line.
void EndSection(Model& m, int countBeforeSection)
{
    if (m.count > countBeforeSection) AddBlank(m);
}

void EmitHeader(Model& m, const wchar_t* name)
{
    AddBlank(m);
    AddRaw(m, name, LineColor::Blue, /*bold=*/true);
    AddBlank(m);
}

void EmitTopBanners(Model& m, int skillType)
{
    const int before = m.count;
    if (skillType == AT_SKILL_INFINITY_ARROW || skillType == AT_SKILL_INFINITY_ARROW_STR)
    {
        AddRaw(m, I18N::Game::ArrowWillNotDecreaseDuringActivation, LineColor::DarkRed);
    }
    EndSection(m, before);
}

// Locals shared by the EmitBodyDamage sub-emitters. Built once per call and
// passed by const ref so each helper stays self-contained without recomputing
// from globals.
struct DamageContext
{
    int heroClass;
    WORD strength;
    WORD dexterity;
    WORD vitality;
    WORD energy;
    WORD charisma;
    int magicMin;
    int magicMax;
    int skillMin;
    int skillMax;
    int skillAttackPowerRate;
};

DamageContext BuildDamageContext(int skillType)
{
    DamageContext ctx{};
    ctx.heroClass = gCharacterManager.GetBaseClass(Hero->Class);
    ctx.dexterity = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
    ctx.energy    = CharacterAttribute->Energy + CharacterAttribute->AddEnergy;
    ctx.strength  = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
    ctx.vitality  = CharacterAttribute->Vitality + CharacterAttribute->AddVitality;
    ctx.charisma  = CharacterAttribute->Charisma + CharacterAttribute->AddCharisma;

    gCharacterManager.GetMagicSkillDamage(skillType, &ctx.magicMin, &ctx.magicMax);
    gCharacterManager.GetSkillDamage(skillType, &ctx.skillMin, &ctx.skillMax);

    int attackMin = 0, attackMax = 0;
    GetAttackDamage(&attackMin, &attackMax);
    ctx.skillMin += attackMin;
    ctx.skillMax += attackMax;

    // Jewel of Harmony skill-power bonuses from equipped weapons.
    StrengthenCapability rightinfo{}, leftinfo{};
    ITEM* rightweapon = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT];
    ITEM* leftweapon = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT];
    if (g_pUIJewelHarmonyinfo && rightweapon->Level >= rightweapon->Jewel_Of_Harmony_OptionLevel)
        g_pUIJewelHarmonyinfo->GetStrengthenCapability(&rightinfo, rightweapon, 1);
    if (g_pUIJewelHarmonyinfo && leftweapon->Level >= leftweapon->Jewel_Of_Harmony_OptionLevel)
        g_pUIJewelHarmonyinfo->GetStrengthenCapability(&leftinfo, leftweapon, 1);
    if (rightinfo.SI_isSP)
    {
        ctx.skillAttackPowerRate += rightinfo.SI_SP.SI_skillattackpower;
        ctx.skillAttackPowerRate += rightinfo.SI_SP.SI_magicalpower;
    }
    if (leftinfo.SI_isSP)
        ctx.skillAttackPowerRate += leftinfo.SI_SP.SI_skillattackpower;

    return ctx;
}

// Wizard/Summoner: Soul Barrier specifics + regular magical damage.
void EmitMagicalDamage(Model& m, int skillType, const DamageContext& ctx)
{
    if (ctx.heroClass != CLASS_WIZARD && ctx.heroClass != CLASS_SUMMONER) return;

    if (skillType == AT_SKILL_SOUL_BARRIER
        || skillType == AT_SKILL_SOUL_BARRIER_STR
        || skillType == AT_SKILL_SOUL_BARRIER_PROFICIENCY)
    {
        int damageShield = (int)(10 + (ctx.dexterity / 50.f) + (ctx.energy / 200.f));
        if (skillType == AT_SKILL_SOUL_BARRIER_STR || skillType == AT_SKILL_SOUL_BARRIER_PROFICIENCY)
        {
            auto additionalValue = CharacterAttribute->MasterSkillInfo[skillType].GetSkillValue();
            damageShield += (int)additionalValue;
        }
        int deleteMana = (int)(CharacterAttribute->ManaMax * 0.02f);
        int limitTime  = (int)(60 + (ctx.energy / 40.f));

        AddFormatted(m, 578, LineColor::White, damageShield);
        AddFormatted(m, 880, LineColor::White, deleteMana);
        AddFormatted(m, 881, LineColor::White, limitTime);
        return;
    }

    if (skillType == AT_SKILL_EXPANSION_OF_WIZARDRY
        || skillType == AT_SKILL_EXPANSION_OF_WIZARDRY_STR
        || skillType == AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY
        || skillType == AT_SKILL_ALICE_SLEEP
        || skillType == AT_SKILL_ALICE_SLEEP_STR) return;

    const WORD s = skillType;
    if ((AT_SKILL_STUN <= s && s <= AT_SKILL_MANA)
        || (AT_SKILL_ALICE_THORNS <= s && s <= AT_SKILL_ALICE_ENERVATION)
        || s == AT_SKILL_TELEPORT || s == AT_SKILL_TELEPORT_ALLY) return;

    if (AT_SKILL_SUMMON_EXPLOSION <= s && s <= AT_SKILL_SUMMON_POLLUTION)
    {
        int curseMin = 0, curseMax = 0;
        gCharacterManager.GetCurseSkillDamage(s, &curseMin, &curseMax);
        AddFormatted(m, 1692, LineColor::White, curseMin, curseMax);
        return;
    }

    AddFormatted(m, 170, LineColor::White,
        ctx.magicMin + ctx.skillAttackPowerRate, ctx.magicMax + ctx.skillAttackPowerRate);
}

// Knight/Dark/Elf/Dark Lord/Rage Fighter: physical skill damage.
void EmitPhysicalDamage(Model& m, int skillType, const DamageContext& ctx)
{
    if (ctx.heroClass != CLASS_KNIGHT && ctx.heroClass != CLASS_DARK
        && ctx.heroClass != CLASS_ELF && ctx.heroClass != CLASS_DARK_LORD
        && ctx.heroClass != CLASS_RAGEFIGHTER) return;

    switch (skillType)
    {
    case AT_SKILL_TELEPORT:
    case AT_SKILL_TELEPORT_ALLY:
    case AT_SKILL_SOUL_BARRIER:
    case AT_SKILL_SOUL_BARRIER_STR:
    case AT_SKILL_SOUL_BARRIER_PROFICIENCY:
    case AT_SKILL_BLOCKING:
    case AT_SKILL_SWELL_LIFE:
    case AT_SKILL_SWELL_LIFE_STR:
    case AT_SKILL_SWELL_LIFE_PROFICIENCY:
    case AT_SKILL_HEALING:
    case AT_SKILL_HEALING_STR:
    case AT_SKILL_DEFENSE:
    case AT_SKILL_DEFENSE_STR:
    case AT_SKILL_DEFENSE_MASTERY:
    case AT_SKILL_ATTACK:
    case AT_SKILL_ATTACK_STR:
    case AT_SKILL_ATTACK_MASTERY:
    case AT_SKILL_SUMMON:
    case AT_SKILL_SUMMON + 1:
    case AT_SKILL_SUMMON + 2:
    case AT_SKILL_SUMMON + 3:
    case AT_SKILL_SUMMON + 4:
    case AT_SKILL_SUMMON + 5:
    case AT_SKILL_SUMMON + 6:
    case AT_SKILL_SUMMON + 7:
    case AT_SKILL_IMPROVE_AG:
    case AT_SKILL_STUN:
    case AT_SKILL_REMOVAL_STUN:
    case AT_SKILL_MANA:
    case AT_SKILL_INVISIBLE:
    case AT_SKILL_REMOVAL_INVISIBLE:
    case AT_SKILL_REMOVAL_BUFF:
    case AT_SKILL_INFINITY_ARROW:
    case AT_SKILL_INFINITY_ARROW_STR:
    case AT_SKILL_PARTY_TELEPORT:
    case AT_SKILL_ADD_CRITICAL:
    case AT_SKILL_ADD_CRITICAL_STR1:
    case AT_SKILL_ADD_CRITICAL_STR2:
    case AT_SKILL_ADD_CRITICAL_STR3:
    case AT_SKILL_BRAND_OF_SKILL:
    case AT_SKILL_PLASMA_STORM_FENRIR:
    case AT_SKILL_RECOVER:
    case AT_SKILL_ATT_UP_OURFORCES:
    case AT_SKILL_HP_UP_OURFORCES:
    case AT_SKILL_HP_UP_OURFORCES_STR:
    case AT_SKILL_DEF_UP_OURFORCES:
    case AT_SKILL_DEF_UP_OURFORCES_STR:
    case AT_SKILL_DEF_UP_OURFORCES_MASTERY:
        return;
    case AT_SKILL_EARTHSHAKE:
    case AT_SKILL_EARTHSHAKE_STR:
    case AT_SKILL_EARTHSHAKE_MASTERY:
        AddRaw(m, I18N::Game::CheckTheDetailsInPetInformationWindow, LineColor::DarkRed);
        return;
    default:
        AddFormatted(m, 879, LineColor::White,
            ctx.skillMin, ctx.skillMax + ctx.skillAttackPowerRate);
        return;
    }
}

// Plasma Storm Fenrir uses a class-specific damage formula.
void EmitPlasmaStormFenrirDamage(Model& m, int skillType, const DamageContext& ctx)
{
    if (skillType != AT_SKILL_PLASMA_STORM_FENRIR) return;

    int skillDamageBase = 0;
    gSkillManager.GetSkillInformation_Damage(AT_SKILL_PLASMA_STORM_FENRIR, &skillDamageBase);

    int minDamage = 0;
    if (ctx.heroClass == CLASS_KNIGHT || ctx.heroClass == CLASS_DARK)
        minDamage = (ctx.strength / 3) + (ctx.dexterity / 5) + (ctx.vitality / 5) + (ctx.energy / 7) + skillDamageBase;
    else if (ctx.heroClass == CLASS_WIZARD || ctx.heroClass == CLASS_SUMMONER)
        minDamage = (ctx.strength / 5) + (ctx.dexterity / 5) + (ctx.vitality / 7) + (ctx.energy / 3) + skillDamageBase;
    else if (ctx.heroClass == CLASS_ELF)
        minDamage = (ctx.strength / 5) + (ctx.dexterity / 3) + (ctx.vitality / 7) + (ctx.energy / 5) + skillDamageBase;
    else if (ctx.heroClass == CLASS_DARK_LORD)
        minDamage = (ctx.strength / 5) + (ctx.dexterity / 5) + (ctx.vitality / 7) + (ctx.energy / 3) + (ctx.charisma / 3) + skillDamageBase;
    else if (ctx.heroClass == CLASS_RAGEFIGHTER)
        minDamage = (ctx.strength / 5) + (ctx.dexterity / 5) + (ctx.vitality / 3) + (ctx.energy / 7) + skillDamageBase;

    const int maxDamage = minDamage + 30;
    AddFormatted(m, 879, LineColor::White, minDamage, maxDamage + ctx.skillAttackPowerRate);
}

// Elf buff-value computations (Healing/Defense/Attack/Recover).
void EmitElfBuffValues(Model& m, int skillType, const DamageContext& ctx)
{
    if (ctx.heroClass != CLASS_ELF) return;

    switch (skillType)
    {
    case AT_SKILL_HEALING_STR:
    {
        int value = (ctx.energy / 5) + 5;
        auto boostPercent = CharacterAttribute->MasterSkillInfo[AT_SKILL_HEALING_STR].GetSkillValue();
        value += static_cast<int>((value * boostPercent) / 100.0);
        AddFormatted(m, 171, LineColor::White, value);
        return;
    }
    case AT_SKILL_HEALING:
        AddFormatted(m, 171, LineColor::White, ctx.energy / 5 + 5);
        return;
    case AT_SKILL_DEFENSE_STR:
    case AT_SKILL_DEFENSE_MASTERY:
    {
        int value = ctx.energy / 8 + 2;
        auto boostPercent = CharacterAttribute->MasterSkillInfo[AT_SKILL_DEFENSE_STR].GetSkillValue();
        auto masteryBoostPercent = CharacterAttribute->MasterSkillInfo[AT_SKILL_DEFENSE_MASTERY].GetSkillValue();
        value += static_cast<int>((value * (boostPercent + masteryBoostPercent)) / 100.0);
        AddFormatted(m, 172, LineColor::White, value);
        return;
    }
    case AT_SKILL_DEFENSE:
        AddFormatted(m, 172, LineColor::White, ctx.energy / 8 + 2);
        return;
    case AT_SKILL_ATTACK_STR:
    case AT_SKILL_ATTACK_MASTERY:
    {
        int value = ctx.energy / 7 + 3;
        auto boostPercent = CharacterAttribute->MasterSkillInfo[AT_SKILL_ATTACK_STR].GetSkillValue();
        auto masteryBoostPercent = CharacterAttribute->MasterSkillInfo[AT_SKILL_ATTACK_MASTERY].GetSkillValue();
        value += static_cast<int>((value * (boostPercent + masteryBoostPercent)) / 100.0);
        AddFormatted(m, 173, LineColor::White, value);
        return;
    }
    case AT_SKILL_ATTACK:
        AddFormatted(m, 173, LineColor::White, ctx.energy / 7 + 3);
        return;
    case AT_SKILL_RECOVER:
    {
        int cal = ctx.energy / 4;
        AddFormatted(m, 1782, LineColor::White, (int)((float)cal + (float)CharacterAttribute->Level));
        return;
    }
    default:
        return;
    }
}

void EmitBodyDamage(Model& m, int skillType)
{
    const int before = m.count;
    const DamageContext ctx = BuildDamageContext(skillType);
    EmitMagicalDamage(m, skillType, ctx);
    EmitPhysicalDamage(m, skillType, ctx);
    EmitPlasmaStormFenrirDamage(m, skillType, ctx);
    EmitElfBuffValues(m, skillType, ctx);
    EndSection(m, before);
}

void EmitBodyStats(Model& m, int skillType, int iDistance, int iMana, int iSkillMana, int iDelayMs)
{
    const int before = m.count;
    if (skillType != AT_SKILL_EXPANSION_OF_WIZARDRY
        && skillType != AT_SKILL_EXPANSION_OF_WIZARDRY_STR
        && skillType != AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY)
    {
        if (iDistance) AddFormatted(m, 174, LineColor::White, iDistance);
    }

    AddFormatted(m, 175, LineColor::White, iMana);
    if (iSkillMana > 0) AddFormatted(m, 360, LineColor::White, iSkillMana);

    if (iDelayMs > 0)
    {
        wchar_t buf[MAX_TOOLTIP_LINE_TEXT];
        mu_swprintf(buf, kCooldownFormat, iDelayMs / kMillisPerSecond);
        AddRaw(m, buf, LineColor::White);
    }
    EndSection(m, before);
}

void EmitRequirements(Model& m, const BuildOptions& options, int skillType)
{
    const int before = m.count;

    int reqEnergy = 0;
    gSkillManager.GetSkillInformation_Energy(skillType, &reqEnergy);

    int curLevel = -1, curStr = -1, curDex = -1, curEnergy = -1, curCha = -1;
    if (options.includeCharacterSpecific)
    {
        curLevel = CharacterAttribute->Level;
        curStr = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
        curDex = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
        curEnergy = CharacterAttribute->Energy + CharacterAttribute->AddEnergy;
        curCha = CharacterAttribute->Charisma + CharacterAttribute->AddCharisma;
    }

    AddRequirementLine(m, SkillAttribute[skillType].Level, curLevel, GLOBAL_TEXT_REQUIRED_LEVEL);
    AddRequirementLine(m, SkillAttribute[skillType].Strength, curStr, GLOBAL_TEXT_REQUIRED_STRENGTH);
    AddRequirementLine(m, SkillAttribute[skillType].Dexterity, curDex, GLOBAL_TEXT_REQUIRED_DEXTERITY);
    AddRequirementLine(m, reqEnergy, curEnergy, GLOBAL_TEXT_REQUIRED_ENERGY);
    AddRequirementLine(m, SkillAttribute[skillType].Charisma, curCha, GLOBAL_TEXT_REQUIRED_CHARISMA);
    EndSection(m, before);
}

void EmitBottomBanners(Model& m, const BuildOptions& options, int skillType)
{
    const int before = m.count;

    // Knight extension info banners. These describe the skill itself (weapon
    // requirement, combo membership), so the editor shows them unconditionally.
    // In game mode, the original gating stays: Knight class + extension + lvl
    // 220, so the player only sees them when relevant.
    const bool gameMode = options.includeCharacterSpecific;
    const bool gameModeKnight = gameMode && gCharacterManager.GetBaseClass(Hero->Class) == CLASS_KNIGHT;
    const bool gameModeKnightExt = gameModeKnight && Hero->byExtensionSkill == 1 && CharacterAttribute->Level >= 220;

    if ((!gameMode || gameModeKnight) && skillType == AT_SKILL_IMPALE)
    {
        AddRaw(m, I18N::Game::CanOnlyBeUsedInMovingUnit, LineColor::DarkRed);
    }

    if (!gameMode || gameModeKnightExt)
    {
        if ((skillType >= AT_SKILL_FALLING_SLASH && skillType <= AT_SKILL_SLASH)
            || skillType == AT_SKILL_FALLING_SLASH_STR
            || skillType == AT_SKILL_LUNGE_STR
            || skillType == AT_SKILL_CYCLONE_STR
            || skillType == AT_SKILL_CYCLONE_STR_MG
            || skillType == AT_SKILL_SLASH_STR
            || skillType == AT_SKILL_TWISTING_SLASH
            || skillType == AT_SKILL_TWISTING_SLASH_STR
            || skillType == AT_SKILL_TWISTING_SLASH_STR_MG
            || skillType == AT_SKILL_TWISTING_SLASH_MASTERY
            || skillType == AT_SKILL_RAGEFUL_BLOW
            || skillType == AT_SKILL_RAGEFUL_BLOW_STR
            || skillType == AT_SKILL_RAGEFUL_BLOW_MASTERY
            || skillType == AT_SKILL_DEATHSTAB
            || skillType == AT_SKILL_DEATHSTAB_STR)
        {
            AddRaw(m, I18N::Game::Combo, LineColor::DarkRed);
        }
        else if (skillType == AT_SKILL_STRIKE_OF_DESTRUCTION
            || skillType == AT_SKILL_STRIKE_OF_DESTRUCTION_STR)
        {
            AddRaw(m, I18N::Game::CombinationAvailable2StepOnly, LineColor::DarkRed);
        }
    }

    // Skill brand / master info (both modes - data-only).
    const int BrandType = SkillAttribute[skillType].SkillBrand;
    const int SkillUseType = SkillAttribute[skillType].SkillUseType;
    if (SkillUseType == SKILL_USE_TYPE_BRAND)
    {
        AddFormattedWide(m, 1480, LineColor::DarkRed, SkillAttribute[BrandType].Name);
        AddFormatted(m, 1481, LineColor::DarkRed, SkillAttribute[BrandType].Damage);
    }
    if (SkillUseType == SKILL_USE_TYPE_MASTER)
    {
        AddRaw(m, I18N::Game::ThisIsAMasterSkillInGuildBattleAndCastleSiege, LineColor::DarkRed);
        AddFormatted(m, 1483, LineColor::DarkRed, SkillAttribute[skillType].KillCount);
    }

    // Dark Lord party teleport warning (character-specific).
    if (options.includeCharacterSpecific
        && gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK_LORD
        && skillType == AT_SKILL_PARTY_TELEPORT && PartyNumber <= 0)
    {
        AddRaw(m, I18N::Game::CanOnlyBeUsedDuringParty, LineColor::DarkRed);
    }

    // Plasma Storm Fenrir extra notes (skill-type-only, both modes).
    if (skillType == AT_SKILL_PLASMA_STORM_FENRIR)
    {
        AddRaw(m, I18N::Game::WhenTheAttackIsSuccessfulItWillDecreaseTheDurabilityOf, LineColor::DarkRed);
        AddRaw(m, I18N::Game::OneOfTheCertainWeaponsTo50, LineColor::DarkRed);
    }

    // Castle-siege-only badge.
    if (IsCastleSiegeOnlySkill(skillType))
    {
        AddRaw(m, I18N::Game::OnlyInCastleSiege, LineColor::DarkRed);
    }

    // Stun / Invisible / Buff-removal: usable in CS with kill count.
    if (skillType == AT_SKILL_STUN || skillType == AT_SKILL_REMOVAL_STUN
        || skillType == AT_SKILL_INVISIBLE || skillType == AT_SKILL_REMOVAL_INVISIBLE
        || skillType == AT_SKILL_REMOVAL_BUFF)
    {
        AddRaw(m, I18N::Game::CanBeUsedDuringCastleSiegeWithRequiredKillCount, LineColor::DarkRed);
    }

    // Impale flavor note.
    if (skillType == AT_SKILL_IMPALE)
    {
        AddRaw(m, I18N::Game::CanBeUsedFromTheMountItem, LineColor::DarkRed);
    }
    EndSection(m, before);
}

void EmitBlueTags(Model& m, int skillType)
{
    const int before = m.count;

    const BYTE MasteryType = gSkillManager.GetSkillMasteryType(static_cast<ActionSkillType>(skillType));
    if (MasteryType != 255)
    {
        AddRaw(m, I18N::Game::Lookup(GLOBAL_TEXT_MASTERY_TYPE_BASE + MasteryType), LineColor::Blue);
    }

    if (skillType == AT_SKILL_EXPANSION_OF_WIZARDRY
        || skillType == AT_SKILL_EXPANSION_OF_WIZARDRY_STR
        || skillType == AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY)
    {
        AddRaw(m, I18N::Game::MinimumWizardryIncrement20, LineColor::Blue);
    }
    EndSection(m, before);
}

}  // namespace

void BuildModel(const BuildOptions& options, Model& outModel)
{
    outModel.Reset();

    if (options.includeCharacterSpecific && (!Hero || !CharacterAttribute || !CharacterMachine)) return;

    const int SkillType = options.skillType;
    if (SkillType < 0 || SkillType >= MAX_SKILLS) return;
    if (SkillAttribute[SkillType].Name[0] == L'\0') return;

    // Resolve display strings + raw fields.
    wchar_t lpszName[256];
    int iMana = 0, iDistance = 0, iSkillMana = 0;
    gSkillManager.GetSkillInformation(SkillType, 1, lpszName, &iMana, &iDistance, &iSkillMana);

    // Force / Force Wave name override (character-specific: depends on the
    // weapon's special options).
    if (options.includeCharacterSpecific && SkillType == AT_SKILL_FORCE && Hero->Weapon[0].Type != -1)
    {
        for (int i = 0; i < CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].SpecialNum; i++)
        {
            if (CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Special[i] == AT_SKILL_FORCE_WAVE)
            {
                mu_swprintf(lpszName, L"%ls", I18N::Game::ForceWave);
                break;
            }
        }
    }

    EmitHeader(outModel, lpszName);
    EmitTopBanners(outModel, SkillType);
    if (options.includeCharacterSpecific) EmitBodyDamage(outModel, SkillType);
    EmitBodyStats(outModel, SkillType, iDistance, iMana, iSkillMana, SkillAttribute[SkillType].Delay);
    EmitRequirements(outModel, options, SkillType);
    EmitBottomBanners(outModel, options, SkillType);
    EmitBlueTags(outModel, SkillType);
}

}  // namespace UI::Skills::Tooltip
