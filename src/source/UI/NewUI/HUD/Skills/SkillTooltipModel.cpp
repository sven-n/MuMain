#include "stdafx.h"
#include "UI/NewUI/HUD/Skills/SkillTooltipModel.h"

#include "Character/CharacterManager.h"
#include "Core/Globals/_enum.h"
#include "Data/Translation/GlobalText.h"
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
    mu_swprintf(l.text, GlobalText[globalTextIdx], v1);
    l.color = color;
    l.isBold = false;
    l.isBlank = false;
}

void AddFormatted(Model& m, int globalTextIdx, LineColor color, int v1, int v2)
{
    Line& l = NextSlot(m);
    mu_swprintf(l.text, GlobalText[globalTextIdx], v1, v2);
    l.color = color;
    l.isBold = false;
    l.isBlank = false;
}

void AddFormattedWide(Model& m, int globalTextIdx, LineColor color, const wchar_t* arg)
{
    Line& l = NextSlot(m);
    mu_swprintf(l.text, GlobalText[globalTextIdx], arg);
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
        mu_swprintf(l.text, GlobalText[reqStringIndex], requiredValue);
        l.color = requirementMet ? LineColor::White : LineColor::Red;
        l.isBold = false;
        l.isBlank = false;
    }

    if (editorMode || requirementMet) return;

    Line& deficit = NextSlot(m);
    mu_swprintf(deficit.text, GlobalText[GLOBAL_TEXT_NEED_MORE_STAT], requiredValue - currentValue);
    deficit.color = LineColor::Red;
    deficit.isBold = false;
    deficit.isBlank = false;
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
            if (CharacterMachine->Equipment[0].Special[i] == AT_SKILL_FORCE_WAVE)
            {
                mu_swprintf(lpszName, L"%ls", GlobalText[1200]);
                break;
            }
        }
    }

    // [blank, name, blank] header.
    AddBlank(outModel);
    AddRaw(outModel, lpszName, LineColor::Blue, /*bold=*/true);
    AddBlank(outModel);

    // ---- Character-specific damage calculations and class buff values.
    if (options.includeCharacterSpecific)
    {
        const int HeroClass = gCharacterManager.GetBaseClass(Hero->Class);

        const WORD Dexterity = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
        const WORD Energy = CharacterAttribute->Energy + CharacterAttribute->AddEnergy;
        const WORD Strength = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
        const WORD Vitality = CharacterAttribute->Vitality + CharacterAttribute->AddVitality;
        const WORD Charisma = CharacterAttribute->Charisma + CharacterAttribute->AddCharisma;

        int iMinDamage = 0, iMaxDamage = 0;
        int iSkillMinDamage = 0, iSkillMaxDamage = 0;
        gCharacterManager.GetMagicSkillDamage(SkillType, &iMinDamage, &iMaxDamage);
        gCharacterManager.GetSkillDamage(SkillType, &iSkillMinDamage, &iSkillMaxDamage);

        int AttackDamageMin = 0, AttackDamageMax = 0;
        GetAttackDamage(&AttackDamageMin, &AttackDamageMax);
        iSkillMinDamage += AttackDamageMin;
        iSkillMaxDamage += AttackDamageMax;

        // Jewel of Harmony skill-power bonuses from equipped weapons.
        int skillattackpowerRate = 0;
        StrengthenCapability rightinfo{}, leftinfo{};
        ITEM* rightweapon = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT];
        ITEM* leftweapon = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT];
        if (g_pUIJewelHarmonyinfo && rightweapon->Level >= rightweapon->Jewel_Of_Harmony_OptionLevel)
            g_pUIJewelHarmonyinfo->GetStrengthenCapability(&rightinfo, rightweapon, 1);
        if (g_pUIJewelHarmonyinfo && leftweapon->Level >= leftweapon->Jewel_Of_Harmony_OptionLevel)
            g_pUIJewelHarmonyinfo->GetStrengthenCapability(&leftinfo, leftweapon, 1);
        if (rightinfo.SI_isSP)
        {
            skillattackpowerRate += rightinfo.SI_SP.SI_skillattackpower;
            skillattackpowerRate += rightinfo.SI_SP.SI_magicalpower;
        }
        if (leftinfo.SI_isSP)
            skillattackpowerRate += leftinfo.SI_SP.SI_skillattackpower;

        // Wizard/Summoner: Soul Barrier specifics + regular damage.
        if (HeroClass == CLASS_WIZARD || HeroClass == CLASS_SUMMONER)
        {
            if (SkillType == AT_SKILL_SOUL_BARRIER
                || SkillType == AT_SKILL_SOUL_BARRIER_STR
                || SkillType == AT_SKILL_SOUL_BARRIER_PROFICIENCY)
            {
                int iDamageShield = (int)(10 + (Dexterity / 50.f) + (Energy / 200.f));
                if (SkillType == AT_SKILL_SOUL_BARRIER_STR || SkillType == AT_SKILL_SOUL_BARRIER_PROFICIENCY)
                {
                    auto additionalValue = CharacterAttribute->MasterSkillInfo[SkillType].GetSkillValue();
                    iDamageShield += (int)additionalValue;
                }
                int iDeleteMana = (int)(CharacterAttribute->ManaMax * 0.02f);
                int iLimitTime = (int)(60 + (Energy / 40.f));

                AddFormatted(outModel, 578, LineColor::White, iDamageShield);
                AddFormatted(outModel, 880, LineColor::White, iDeleteMana);
                AddFormatted(outModel, 881, LineColor::White, iLimitTime);
            }
            else if (SkillType != AT_SKILL_EXPANSION_OF_WIZARDRY
                && SkillType != AT_SKILL_EXPANSION_OF_WIZARDRY_STR
                && SkillType != AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY
                && SkillType != AT_SKILL_ALICE_SLEEP
                && SkillType != AT_SKILL_ALICE_SLEEP_STR)
            {
                WORD bySkill = SkillType;
                if (!(AT_SKILL_STUN <= bySkill && bySkill <= AT_SKILL_MANA)
                    && !(AT_SKILL_ALICE_THORNS <= bySkill && bySkill <= AT_SKILL_ALICE_ENERVATION)
                    && bySkill != AT_SKILL_TELEPORT && bySkill != AT_SKILL_TELEPORT_ALLY)
                {
                    if (AT_SKILL_SUMMON_EXPLOSION <= bySkill && bySkill <= AT_SKILL_SUMMON_POLLUTION)
                    {
                        gCharacterManager.GetCurseSkillDamage(bySkill, &iMinDamage, &iMaxDamage);
                        AddFormatted(outModel, 1692, LineColor::White, iMinDamage, iMaxDamage);
                    }
                    else
                    {
                        AddFormatted(outModel, 170, LineColor::White,
                            iMinDamage + skillattackpowerRate, iMaxDamage + skillattackpowerRate);
                    }
                }
            }
        }

        // Knight/Dark/Elf/Dark Lord/Rage Fighter: physical skill damage.
        if (HeroClass == CLASS_KNIGHT || HeroClass == CLASS_DARK || HeroClass == CLASS_ELF
            || HeroClass == CLASS_DARK_LORD || HeroClass == CLASS_RAGEFIGHTER)
        {
            switch (SkillType)
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
                break;
            case AT_SKILL_PARTY_TELEPORT:
            case AT_SKILL_ADD_CRITICAL:
            case AT_SKILL_ADD_CRITICAL_STR1:
            case AT_SKILL_ADD_CRITICAL_STR2:
            case AT_SKILL_ADD_CRITICAL_STR3:
                break;
            case AT_SKILL_EARTHSHAKE:
            case AT_SKILL_EARTHSHAKE_STR:
            case AT_SKILL_EARTHSHAKE_MASTERY:
                AddRaw(outModel, GlobalText[1237], LineColor::DarkRed);
                break;
            case AT_SKILL_BRAND_OF_SKILL:
                break;
            case AT_SKILL_PLASMA_STORM_FENRIR:
            case AT_SKILL_RECOVER:
            case AT_SKILL_ATT_UP_OURFORCES:
            case AT_SKILL_HP_UP_OURFORCES:
            case AT_SKILL_HP_UP_OURFORCES_STR:
            case AT_SKILL_DEF_UP_OURFORCES:
            case AT_SKILL_DEF_UP_OURFORCES_STR:
            case AT_SKILL_DEF_UP_OURFORCES_MASTERY:
                break;
            default:
                AddFormatted(outModel, 879, LineColor::White,
                    iSkillMinDamage, iSkillMaxDamage + skillattackpowerRate);
                break;
            }
        }

        // Plasma Storm Fenrir: class-specific damage formula.
        if (SkillType == AT_SKILL_PLASMA_STORM_FENRIR)
        {
            int iSkillDamage = 0;
            gSkillManager.GetSkillInformation_Damage(AT_SKILL_PLASMA_STORM_FENRIR, &iSkillDamage);

            if (HeroClass == CLASS_KNIGHT || HeroClass == CLASS_DARK)
                iSkillMinDamage = (Strength / 3) + (Dexterity / 5) + (Vitality / 5) + (Energy / 7) + iSkillDamage;
            else if (HeroClass == CLASS_WIZARD || HeroClass == CLASS_SUMMONER)
                iSkillMinDamage = (Strength / 5) + (Dexterity / 5) + (Vitality / 7) + (Energy / 3) + iSkillDamage;
            else if (HeroClass == CLASS_ELF)
                iSkillMinDamage = (Strength / 5) + (Dexterity / 3) + (Vitality / 7) + (Energy / 5) + iSkillDamage;
            else if (HeroClass == CLASS_DARK_LORD)
                iSkillMinDamage = (Strength / 5) + (Dexterity / 5) + (Vitality / 7) + (Energy / 3) + (Charisma / 3) + iSkillDamage;
            else if (HeroClass == CLASS_RAGEFIGHTER)
                iSkillMinDamage = (Strength / 5) + (Dexterity / 5) + (Vitality / 3) + (Energy / 7) + iSkillDamage;

            iSkillMaxDamage = iSkillMinDamage + 30;
            AddFormatted(outModel, 879, LineColor::White,
                iSkillMinDamage, iSkillMaxDamage + skillattackpowerRate);
        }

        // Elf buff-value computations.
        if (HeroClass == CLASS_ELF)
        {
            switch (SkillType)
            {
            case AT_SKILL_HEALING_STR:
            {
                int value = (Energy / 5) + 5;
                auto boostPercent = CharacterAttribute->MasterSkillInfo[AT_SKILL_HEALING_STR].GetSkillValue();
                value += static_cast<int>((value * boostPercent) / 100.0);
                AddFormatted(outModel, 171, LineColor::White, value);
                break;
            }
            case AT_SKILL_HEALING:
                AddFormatted(outModel, 171, LineColor::White, Energy / 5 + 5);
                break;
            case AT_SKILL_DEFENSE_STR:
            case AT_SKILL_DEFENSE_MASTERY:
            {
                int value = Energy / 8 + 2;
                auto boostPercent = CharacterAttribute->MasterSkillInfo[AT_SKILL_DEFENSE_STR].GetSkillValue();
                auto masteryBoostPercent = CharacterAttribute->MasterSkillInfo[AT_SKILL_DEFENSE_MASTERY].GetSkillValue();
                value += static_cast<int>((value * (boostPercent + masteryBoostPercent)) / 100.0);
                AddFormatted(outModel, 172, LineColor::White, value);
                break;
            }
            case AT_SKILL_DEFENSE:
                AddFormatted(outModel, 172, LineColor::White, Energy / 8 + 2);
                break;
            case AT_SKILL_ATTACK_STR:
            case AT_SKILL_ATTACK_MASTERY:
            {
                int value = Energy / 7 + 3;
                auto boostPercent = CharacterAttribute->MasterSkillInfo[AT_SKILL_ATTACK_STR].GetSkillValue();
                auto masteryBoostPercent = CharacterAttribute->MasterSkillInfo[AT_SKILL_ATTACK_MASTERY].GetSkillValue();
                value += static_cast<int>((value * (boostPercent + masteryBoostPercent)) / 100.0);
                AddFormatted(outModel, 173, LineColor::White, value);
                break;
            }
            case AT_SKILL_ATTACK:
                AddFormatted(outModel, 173, LineColor::White, Energy / 7 + 3);
                break;
            case AT_SKILL_RECOVER:
            {
                int Cal = Energy / 4;
                AddFormatted(outModel, 1782, LineColor::White, (int)((float)Cal + (float)CharacterAttribute->Level));
                break;
            }
            default:
                break;
            }
        }
    }

    // ---- Static fields (range, mana, ag) - present in both modes.
    if (SkillType != AT_SKILL_EXPANSION_OF_WIZARDRY
        && SkillType != AT_SKILL_EXPANSION_OF_WIZARDRY_STR
        && SkillType != AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY)
    {
        if (iDistance) AddFormatted(outModel, 174, LineColor::White, iDistance);
    }

    AddFormatted(outModel, 175, LineColor::White, iMana);
    if (iSkillMana > 0) AddFormatted(outModel, 360, LineColor::White, iSkillMana);

    // ---- Requirements. In game mode, color-coded vs current player stats.
    //      In editor mode (-1 sentinels) always white, no deficit lines.
    int reqEnergy = 0;
    gSkillManager.GetSkillInformation_Energy(SkillType, &reqEnergy);

    int curLevel = -1, curStr = -1, curDex = -1, curEnergy = -1, curCha = -1;
    if (options.includeCharacterSpecific)
    {
        curLevel = CharacterAttribute->Level;
        curStr = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
        curDex = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
        curEnergy = CharacterAttribute->Energy + CharacterAttribute->AddEnergy;
        curCha = CharacterAttribute->Charisma + CharacterAttribute->AddCharisma;
    }

    AddRequirementLine(outModel, SkillAttribute[SkillType].Level, curLevel, GLOBAL_TEXT_REQUIRED_LEVEL);
    AddRequirementLine(outModel, SkillAttribute[SkillType].Strength, curStr, GLOBAL_TEXT_REQUIRED_STRENGTH);
    AddRequirementLine(outModel, SkillAttribute[SkillType].Dexterity, curDex, GLOBAL_TEXT_REQUIRED_DEXTERITY);
    AddRequirementLine(outModel, reqEnergy, curEnergy, GLOBAL_TEXT_REQUIRED_ENERGY);
    AddRequirementLine(outModel, SkillAttribute[SkillType].Charisma, curCha, GLOBAL_TEXT_REQUIRED_CHARISMA);

    // ---- Knight extension warnings (character-specific).
    if (options.includeCharacterSpecific && gCharacterManager.GetBaseClass(Hero->Class) == CLASS_KNIGHT)
    {
        if (SkillType == AT_SKILL_IMPALE)
        {
            AddRaw(outModel, GlobalText[96], LineColor::DarkRed);
        }

        if (Hero->byExtensionSkill == 1 && CharacterAttribute->Level >= 220)
        {
            if ((SkillType >= AT_SKILL_FALLING_SLASH && SkillType <= AT_SKILL_SLASH)
                || SkillType == AT_SKILL_FALLING_SLASH_STR
                || SkillType == AT_SKILL_LUNGE_STR
                || SkillType == AT_SKILL_CYCLONE_STR
                || SkillType == AT_SKILL_CYCLONE_STR_MG
                || SkillType == AT_SKILL_SLASH_STR
                || SkillType == AT_SKILL_TWISTING_SLASH
                || SkillType == AT_SKILL_TWISTING_SLASH_STR
                || SkillType == AT_SKILL_TWISTING_SLASH_STR_MG
                || SkillType == AT_SKILL_TWISTING_SLASH_MASTERY
                || SkillType == AT_SKILL_RAGEFUL_BLOW
                || SkillType == AT_SKILL_RAGEFUL_BLOW_STR
                || SkillType == AT_SKILL_RAGEFUL_BLOW_MASTERY
                || SkillType == AT_SKILL_DEATHSTAB
                || SkillType == AT_SKILL_DEATHSTAB_STR)
            {
                AddRaw(outModel, GlobalText[99], LineColor::DarkRed);
            }
            else if (SkillType == AT_SKILL_STRIKE_OF_DESTRUCTION
                || SkillType == AT_SKILL_STRIKE_OF_DESTRUCTION_STR)
            {
                AddRaw(outModel, GlobalText[2115], LineColor::DarkRed);
            }
        }
    }

    // ---- Mastery type label (both modes).
    const BYTE MasteryType = gSkillManager.GetSkillMasteryType(static_cast<ActionSkillType>(SkillType));
    if (MasteryType != 255)
    {
        AddRaw(outModel, GlobalText[GLOBAL_TEXT_MASTERY_TYPE_BASE + MasteryType], LineColor::Blue);
    }

    // ---- Skill brand / master info (both modes - data-only).
    const int BrandType = SkillAttribute[SkillType].SkillBrand;
    const int SkillUseType = SkillAttribute[SkillType].SkillUseType;
    if (SkillUseType == SKILL_USE_TYPE_BRAND)
    {
        AddFormattedWide(outModel, 1480, LineColor::DarkRed, SkillAttribute[BrandType].Name);
        AddFormatted(outModel, 1481, LineColor::DarkRed, SkillAttribute[BrandType].Damage);
    }
    if (SkillUseType == SKILL_USE_TYPE_MASTER)
    {
        AddRaw(outModel, GlobalText[1482], LineColor::DarkRed);
        AddFormatted(outModel, 1483, LineColor::DarkRed, SkillAttribute[SkillType].KillCount);
    }

    // ---- Dark Lord party teleport warning (character-specific).
    if (options.includeCharacterSpecific
        && gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK_LORD
        && SkillType == AT_SKILL_PARTY_TELEPORT && PartyNumber <= 0)
    {
        AddRaw(outModel, GlobalText[1185], LineColor::DarkRed);
    }

    // ---- Plasma Storm Fenrir extra notes (skill-type-only, both modes).
    if (SkillType == AT_SKILL_PLASMA_STORM_FENRIR)
    {
        AddRaw(outModel, GlobalText[1926], LineColor::DarkRed);
        AddRaw(outModel, GlobalText[1927], LineColor::DarkRed);
    }

    // ---- Infinity Arrow special case: overrides the header section.
    //      Replaces lines 1..5 with [name, blank, info, mana, ag] and resets
    //      the model count to 6 with skipCount 2 so subsequent additions
    //      (footer notes) start at line 6.
    if (SkillType == AT_SKILL_INFINITY_ARROW || SkillType == AT_SKILL_INFINITY_ARROW_STR)
    {
        wcsncpy(outModel.lines[1].text, lpszName, MAX_TOOLTIP_LINE_TEXT - 1);
        outModel.lines[1].text[MAX_TOOLTIP_LINE_TEXT - 1] = L'\0';
        outModel.lines[1].color = LineColor::Blue;
        outModel.lines[1].isBold = true;
        outModel.lines[1].isBlank = false;

        outModel.lines[2].text[0] = L'\n';
        outModel.lines[2].text[1] = L'\0';
        outModel.lines[2].color = LineColor::White;
        outModel.lines[2].isBold = false;
        outModel.lines[2].isBlank = true;

        wcsncpy(outModel.lines[3].text, GlobalText[2040], MAX_TOOLTIP_LINE_TEXT - 1);
        outModel.lines[3].text[MAX_TOOLTIP_LINE_TEXT - 1] = L'\0';
        outModel.lines[3].color = LineColor::DarkRed;
        outModel.lines[3].isBold = false;
        outModel.lines[3].isBlank = false;

        mu_swprintf(outModel.lines[4].text, GlobalText[175], iMana);
        outModel.lines[4].color = LineColor::White;
        outModel.lines[4].isBold = false;
        outModel.lines[4].isBlank = false;

        mu_swprintf(outModel.lines[5].text, GlobalText[360], iSkillMana);
        outModel.lines[5].color = LineColor::White;
        outModel.lines[5].isBold = false;
        outModel.lines[5].isBlank = false;

        outModel.count = 6;
        outModel.skipCount = 2;
    }

    // ---- Footer blank.
    AddBlank(outModel);

    // ---- Castle-siege-only badge (white text on red background).
    if (IsCastleSiegeOnlySkill(SkillType))
    {
        AddRaw(outModel, GlobalText[2047], LineColor::DarkRed);
    }

    // ---- Stun / Invisible / Buff-removal: usable in CS with kill count.
    if (SkillType == AT_SKILL_STUN || SkillType == AT_SKILL_REMOVAL_STUN
        || SkillType == AT_SKILL_INVISIBLE || SkillType == AT_SKILL_REMOVAL_INVISIBLE
        || SkillType == AT_SKILL_REMOVAL_BUFF)
    {
        AddRaw(outModel, GlobalText[2048], LineColor::DarkRed);
    }

    // ---- Impale flavor note.
    if (SkillType == AT_SKILL_IMPALE)
    {
        AddRaw(outModel, GlobalText[2049], LineColor::DarkRed);
    }

    // ---- Expansion of Wizardry tag.
    if (SkillType == AT_SKILL_EXPANSION_OF_WIZARDRY
        || SkillType == AT_SKILL_EXPANSION_OF_WIZARDRY_STR
        || SkillType == AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY)
    {
        AddRaw(outModel, GlobalText[2054], LineColor::Blue);
    }
}

}  // namespace UI::Skills::Tooltip
