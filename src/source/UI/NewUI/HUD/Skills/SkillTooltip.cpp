#include "stdafx.h"
#include "UI/NewUI/HUD/Skills/SkillTooltip.h"

#include "Character/CharacterManager.h"
#include "Data/Translation/GlobalText.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzInventory.h"     // TextList / TextListColor / TextBold externs, PartyNumber, STRP_*
#include "GameLogic/Pets/GIPetManager.h"
#include "GameLogic/Skills/SkillManager.h"
#include "UI/Legacy/UIControls.h"           // g_pRenderText macro
#include "UI/Legacy/UIJewelHarmony.h"       // StrengthenCapability
#include "UI/Legacy/UIManager.h"            // g_pUIJewelHarmonyinfo

// Forward declaration for GetAttackDamage (defined in ZzzInventory.cpp but
// not declared in any header). RenderTipTextList is declared in
// ZzzInventory.h (already included above).
bool GetAttackDamage(int* iMinDamage, int* iMaxDamage);

namespace
{
constexpr int GLOBAL_TEXT_REQUIRED_LEVEL = 76;
constexpr int GLOBAL_TEXT_REQUIRED_STRENGTH = 73;
constexpr int GLOBAL_TEXT_REQUIRED_DEXTERITY = 75;
constexpr int GLOBAL_TEXT_REQUIRED_ENERGY = 77;
constexpr int GLOBAL_TEXT_REQUIRED_CHARISMA = 698;
constexpr int GLOBAL_TEXT_NEED_MORE_STAT = 74;

void AddSkillRequirementLine(int requiredValue, int currentValue, int reqStringIndex, int& TextNum)
{
    if (requiredValue <= 0) return;

    mu_swprintf(TextList[TextNum], GlobalText[reqStringIndex], requiredValue);
    const bool requirementMet = (currentValue >= requiredValue);
    TextListColor[TextNum] = requirementMet ? TEXT_COLOR_WHITE : TEXT_COLOR_RED;
    TextBold[TextNum] = false;
    ++TextNum;

    if (requirementMet) return;

    mu_swprintf(TextList[TextNum], GlobalText[GLOBAL_TEXT_NEED_MORE_STAT], requiredValue - currentValue);
    TextListColor[TextNum] = TEXT_COLOR_RED;
    TextBold[TextNum] = false;
    ++TextNum;
}
}

namespace UI::Skills::Tooltip
{

void Render(int sx, int sy, int Type, int SkillNum, int iRenderPoint /*= STRP_NONE*/)
{
    wchar_t lpszName[256];
    int  iMinDamage, iMaxDamage;
    int  HeroClass = gCharacterManager.GetBaseClass(Hero->Class);
    int  iMana, iDistance, iSkillMana;
    int  TextNum = 0;
    int  SkipNum = 0;

    if (giPetManager::RenderPetCmdInfo(sx, sy, Type)) return;

    int  AttackDamageMin, AttackDamageMax;
    int  iSkillMinDamage, iSkillMaxDamage;

    int  SkillType = CharacterAttribute->Skill[Type];
    gCharacterManager.GetMagicSkillDamage(CharacterAttribute->Skill[Type], &iMinDamage, &iMaxDamage);
    gCharacterManager.GetSkillDamage(CharacterAttribute->Skill[Type], &iSkillMinDamage, &iSkillMaxDamage);

    GetAttackDamage(&AttackDamageMin, &AttackDamageMax);

    iSkillMinDamage += AttackDamageMin;
    iSkillMaxDamage += AttackDamageMax;
    gSkillManager.GetSkillInformation(CharacterAttribute->Skill[Type], 1, lpszName, &iMana, &iDistance, &iSkillMana);

    if (CharacterAttribute->Skill[Type] == AT_SKILL_FORCE && Hero->Weapon[0].Type != -1)
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

    mu_swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
    mu_swprintf(TextList[TextNum], L"%ls", lpszName);
    TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = true; TextNum++;
    mu_swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

    WORD Dexterity;
    WORD Energy;
    WORD Strength;
    WORD Vitality;
    WORD Charisma;

    Dexterity = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
    Energy = CharacterAttribute->Energy + CharacterAttribute->AddEnergy;
    Strength = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
    Vitality = CharacterAttribute->Vitality + CharacterAttribute->AddVitality;
    Charisma = CharacterAttribute->Charisma + CharacterAttribute->AddCharisma;

    int skillattackpowerRate = 0;

    StrengthenCapability rightinfo, leftinfo;

    ITEM* rightweapon = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT];
    ITEM* leftweapon = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT];

    int rightlevel = rightweapon->Level;

    if (rightlevel >= rightweapon->Jewel_Of_Harmony_OptionLevel)
    {
        g_pUIJewelHarmonyinfo->GetStrengthenCapability(&rightinfo, rightweapon, 1);
    }

    int leftlevel = leftweapon->Level;

    if (leftlevel >= leftweapon->Jewel_Of_Harmony_OptionLevel)
    {
        g_pUIJewelHarmonyinfo->GetStrengthenCapability(&leftinfo, leftweapon, 1);
    }

    if (rightinfo.SI_isSP)
    {
        skillattackpowerRate += rightinfo.SI_SP.SI_skillattackpower;
        skillattackpowerRate += rightinfo.SI_SP.SI_magicalpower;
    }
    if (leftinfo.SI_isSP)
    {
        skillattackpowerRate += leftinfo.SI_SP.SI_skillattackpower;
    }

    if (HeroClass == CLASS_WIZARD || HeroClass == CLASS_SUMMONER)
    {
        if (CharacterAttribute->Skill[Type] == AT_SKILL_SOUL_BARRIER
            || CharacterAttribute->Skill[Type] == AT_SKILL_SOUL_BARRIER_STR
            || CharacterAttribute->Skill[Type] == AT_SKILL_SOUL_BARRIER_PROFICIENCY
            )
        {
            int iDamageShield;

            if (CharacterAttribute->Skill[Type] == AT_SKILL_SOUL_BARRIER)
            {
                iDamageShield = (int)(10 + (Dexterity / 50.f) + (Energy / 200.f));
            }
            else if (CharacterAttribute->Skill[Type] == AT_SKILL_SOUL_BARRIER_STR
                || CharacterAttribute->Skill[Type] == AT_SKILL_SOUL_BARRIER_PROFICIENCY)
            {
                auto additionalValue = CharacterAttribute->MasterSkillInfo[CharacterAttribute->Skill[Type]].GetSkillValue();
                iDamageShield = (int)(10 + (Dexterity / 50.f) + (Energy / 200.f));
                iDamageShield += (int)additionalValue;
            }

            int iDeleteMana = (int)(CharacterAttribute->ManaMax * 0.02f);
            int iLimitTime = (int)(60 + (Energy / 40.f));

            mu_swprintf(TextList[TextNum], GlobalText[578], iDamageShield);
            TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;

            mu_swprintf(TextList[TextNum], GlobalText[880], iDeleteMana);
            TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;

            mu_swprintf(TextList[TextNum], GlobalText[881], iLimitTime);
            TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
        }
        else if (SkillType != AT_SKILL_EXPANSION_OF_WIZARDRY
            && SkillType != AT_SKILL_EXPANSION_OF_WIZARDRY_STR
            && SkillType != AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY
            && SkillType != AT_SKILL_ALICE_SLEEP
            && SkillType != AT_SKILL_ALICE_SLEEP_STR
            )
        {
            WORD bySkill = CharacterAttribute->Skill[Type];
            if (!(AT_SKILL_STUN <= bySkill && bySkill <= AT_SKILL_MANA) && !(AT_SKILL_ALICE_THORNS <= bySkill && bySkill <= AT_SKILL_ALICE_ENERVATION) && bySkill != AT_SKILL_TELEPORT && bySkill != AT_SKILL_TELEPORT_ALLY)
            {
                if (AT_SKILL_SUMMON_EXPLOSION <= bySkill && bySkill <= AT_SKILL_SUMMON_POLLUTION)
                {
                    gCharacterManager.GetCurseSkillDamage(bySkill, &iMinDamage, &iMaxDamage);
                    mu_swprintf(TextList[TextNum], GlobalText[1692], iMinDamage, iMaxDamage);
                }
                else
                    mu_swprintf(TextList[TextNum], GlobalText[170], iMinDamage + skillattackpowerRate, iMaxDamage + skillattackpowerRate);
                TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
            }
        }
    }
    if (HeroClass == CLASS_KNIGHT || HeroClass == CLASS_DARK || HeroClass == CLASS_ELF || HeroClass == CLASS_DARK_LORD
        || HeroClass == CLASS_RAGEFIGHTER)
    {
        switch (CharacterAttribute->Skill[Type])
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
            mu_swprintf(TextList[TextNum], GlobalText[1237]);
            TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
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
            mu_swprintf(TextList[TextNum], GlobalText[879], iSkillMinDamage, iSkillMaxDamage + skillattackpowerRate);
            TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
            break;
        }
    }

    if (CharacterAttribute->Skill[Type] == AT_SKILL_PLASMA_STORM_FENRIR)
    {
        int iSkillDamage;
        gSkillManager.GetSkillInformation_Damage(AT_SKILL_PLASMA_STORM_FENRIR, &iSkillDamage);

        if (HeroClass == CLASS_KNIGHT || HeroClass == CLASS_DARK)
        {
            iSkillMinDamage = (Strength / 3) + (Dexterity / 5) + (Vitality / 5) + (Energy / 7) + iSkillDamage;
        }
        else if (HeroClass == CLASS_WIZARD || HeroClass == CLASS_SUMMONER)
        {
            iSkillMinDamage = (Strength / 5) + (Dexterity / 5) + (Vitality / 7) + (Energy / 3) + iSkillDamage;
        }
        else if (HeroClass == CLASS_ELF)
        {
            iSkillMinDamage = (Strength / 5) + (Dexterity / 3) + (Vitality / 7) + (Energy / 5) + iSkillDamage;
        }
        else if (HeroClass == CLASS_DARK_LORD)
        {
            iSkillMinDamage = (Strength / 5) + (Dexterity / 5) + (Vitality / 7) + (Energy / 3) + (Charisma / 3) + iSkillDamage;
        }
        else if (HeroClass == CLASS_RAGEFIGHTER)
        {
            iSkillMinDamage = (Strength / 5) + (Dexterity / 5) + (Vitality / 3) + (Energy / 7) + iSkillDamage;
        }

        iSkillMaxDamage = iSkillMinDamage + 30;

        mu_swprintf(TextList[TextNum], GlobalText[879], iSkillMinDamage, iSkillMaxDamage + skillattackpowerRate);
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
    }

    if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_ELF)
    {
        bool Success = true;
        switch (CharacterAttribute->Skill[Type])
        {
        case AT_SKILL_HEALING_STR:
        {
            int value = (Energy / 5) + 5;
            auto boostPercent = CharacterAttribute->MasterSkillInfo[AT_SKILL_HEALING_STR].GetSkillValue();
            value += static_cast<int>((value * boostPercent) / 100.0);
            mu_swprintf(TextList[TextNum], GlobalText[171], value);
        }
        break;
        case AT_SKILL_HEALING:
            mu_swprintf(TextList[TextNum], GlobalText[171], Energy / 5 + 5);
            break;
        case AT_SKILL_DEFENSE_STR:
        case AT_SKILL_DEFENSE_MASTERY:
        {
            int value = Energy / 8 + 2;
            auto boostPercent = CharacterAttribute->MasterSkillInfo[AT_SKILL_DEFENSE_STR].GetSkillValue();
            auto masteryBoostPercent = CharacterAttribute->MasterSkillInfo[AT_SKILL_DEFENSE_MASTERY].GetSkillValue();;
            value += static_cast<int>((value * (boostPercent + masteryBoostPercent)) / 100.0);
            mu_swprintf(TextList[TextNum], GlobalText[172], value);
        }
        break;
        case AT_SKILL_DEFENSE:
            mu_swprintf(TextList[TextNum], GlobalText[172], Energy / 8 + 2);
            break;
        case AT_SKILL_ATTACK_STR:
        case AT_SKILL_ATTACK_MASTERY:
        {
            int value = Energy / 7 + 3;
            auto boostPercent = CharacterAttribute->MasterSkillInfo[AT_SKILL_ATTACK_STR].GetSkillValue();
            auto masteryBoostPercent = CharacterAttribute->MasterSkillInfo[AT_SKILL_ATTACK_MASTERY].GetSkillValue();
            value += static_cast<int>((value * (boostPercent + masteryBoostPercent)) / 100.0);
            mu_swprintf(TextList[TextNum], GlobalText[173], value);
        }
        break;
        case AT_SKILL_ATTACK:
            mu_swprintf(TextList[TextNum], GlobalText[173], Energy / 7 + 3);
            break;
        case AT_SKILL_RECOVER:
        {
            int Cal = Energy / 4;
            mu_swprintf(TextList[TextNum], GlobalText[1782], (int)((float)Cal + (float)CharacterAttribute->Level));
        }
        break;
        default:Success = false;
        }
        if (Success)
        {
            TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
        }
    }

    if (SkillType != AT_SKILL_EXPANSION_OF_WIZARDRY
        && SkillType != AT_SKILL_EXPANSION_OF_WIZARDRY_STR
        && SkillType != AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY)
    {
        if (iDistance)
        {
            mu_swprintf(TextList[TextNum], GlobalText[174], iDistance);
            TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
        }
    }

    mu_swprintf(TextList[TextNum], GlobalText[175], iMana);
    TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
    if (iSkillMana > 0)
    {
        mu_swprintf(TextList[TextNum], GlobalText[360], iSkillMana);
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
    }

    int reqEnergy = 0;
    gSkillManager.GetSkillInformation_Energy(SkillType, &reqEnergy);

    AddSkillRequirementLine(SkillAttribute[SkillType].Level, CharacterAttribute->Level, GLOBAL_TEXT_REQUIRED_LEVEL, TextNum);
    AddSkillRequirementLine(SkillAttribute[SkillType].Strength, Strength, GLOBAL_TEXT_REQUIRED_STRENGTH, TextNum);
    AddSkillRequirementLine(SkillAttribute[SkillType].Dexterity, Dexterity, GLOBAL_TEXT_REQUIRED_DEXTERITY, TextNum);
    AddSkillRequirementLine(reqEnergy, Energy, GLOBAL_TEXT_REQUIRED_ENERGY, TextNum);
    AddSkillRequirementLine(SkillAttribute[SkillType].Charisma, Charisma, GLOBAL_TEXT_REQUIRED_CHARISMA, TextNum);

    if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_KNIGHT)
    {
        if (CharacterAttribute->Skill[Type] == AT_SKILL_IMPALE)
        {
            mu_swprintf(TextList[TextNum], GlobalText[96]);
            TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
        }

        if (Hero->byExtensionSkill == 1 && CharacterAttribute->Level >= 220)
        {
            // Combo skills:
            if ((CharacterAttribute->Skill[Type] >= AT_SKILL_FALLING_SLASH && CharacterAttribute->Skill[Type] <= AT_SKILL_SLASH)
                || CharacterAttribute->Skill[Type] == AT_SKILL_FALLING_SLASH_STR
                || CharacterAttribute->Skill[Type] == AT_SKILL_LUNGE_STR
                || CharacterAttribute->Skill[Type] == AT_SKILL_CYCLONE_STR
                || CharacterAttribute->Skill[Type] == AT_SKILL_CYCLONE_STR_MG
                || CharacterAttribute->Skill[Type] == AT_SKILL_SLASH_STR
                || CharacterAttribute->Skill[Type] == AT_SKILL_TWISTING_SLASH
                || CharacterAttribute->Skill[Type] == AT_SKILL_TWISTING_SLASH_STR
                || CharacterAttribute->Skill[Type] == AT_SKILL_TWISTING_SLASH_STR_MG
                || CharacterAttribute->Skill[Type] == AT_SKILL_TWISTING_SLASH_MASTERY
                || CharacterAttribute->Skill[Type] == AT_SKILL_RAGEFUL_BLOW
                || CharacterAttribute->Skill[Type] == AT_SKILL_RAGEFUL_BLOW_STR
                || CharacterAttribute->Skill[Type] == AT_SKILL_RAGEFUL_BLOW_MASTERY
                || CharacterAttribute->Skill[Type] == AT_SKILL_DEATHSTAB
                || CharacterAttribute->Skill[Type] == AT_SKILL_DEATHSTAB_STR
                )
            {
                mu_swprintf(TextList[TextNum], GlobalText[99]);
                TextListColor[TextNum] = TEXT_COLOR_DARKRED;
                TextBold[TextNum] = false;
                TextNum++;
            }
            else if (CharacterAttribute->Skill[Type] == AT_SKILL_STRIKE_OF_DESTRUCTION
                || CharacterAttribute->Skill[Type] == AT_SKILL_STRIKE_OF_DESTRUCTION_STR)
            {
                mu_swprintf(TextList[TextNum], GlobalText[2115]);
                TextListColor[TextNum] = TEXT_COLOR_DARKRED;
                TextBold[TextNum] = false;
                TextNum++;
            }
        }
    }

    BYTE MasteryType = gSkillManager.GetSkillMasteryType(CharacterAttribute->Skill[Type]);
    if (MasteryType != 255)
    {
        mu_swprintf(TextList[TextNum], GlobalText[1080 + MasteryType]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
    }

    int SkillUseType;
    int BrandType = SkillAttribute[SkillType].SkillBrand;
    SkillUseType = SkillAttribute[SkillType].SkillUseType;
    if (SkillUseType == SKILL_USE_TYPE_BRAND)
    {
        mu_swprintf(TextList[TextNum], GlobalText[1480], SkillAttribute[BrandType].Name);
        TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
        mu_swprintf(TextList[TextNum], GlobalText[1481], SkillAttribute[BrandType].Damage);
        TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
    }
    SkillUseType = SkillAttribute[SkillType].SkillUseType;
    if (SkillUseType == SKILL_USE_TYPE_MASTER)
    {
        mu_swprintf(TextList[TextNum], GlobalText[1482]);
        TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
        mu_swprintf(TextList[TextNum], GlobalText[1483], SkillAttribute[SkillType].KillCount);
        TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
    }

    if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK_LORD)
    {
        if (CharacterAttribute->Skill[Type] == AT_SKILL_PARTY_TELEPORT && PartyNumber <= 0)
        {
            mu_swprintf(TextList[TextNum], GlobalText[1185]);
            TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
        }
    }

    if (CharacterAttribute->Skill[Type] == AT_SKILL_PLASMA_STORM_FENRIR)
    {
        mu_swprintf(TextList[TextNum], GlobalText[1926]);
        TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
        mu_swprintf(TextList[TextNum], GlobalText[1927]);
        TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
    }

    if (CharacterAttribute->Skill[Type] == AT_SKILL_INFINITY_ARROW || CharacterAttribute->Skill[Type] == AT_SKILL_INFINITY_ARROW_STR)
    {
        mu_swprintf(TextList[1], lpszName);
        TextListColor[1] = TEXT_COLOR_BLUE; TextBold[1] = true;
        mu_swprintf(TextList[2], L"\n");
        mu_swprintf(TextList[3], GlobalText[2040]);
        TextListColor[3] = TEXT_COLOR_DARKRED; TextBold[3] = false;
        mu_swprintf(TextList[4], GlobalText[175], iMana);
        TextListColor[4] = TEXT_COLOR_WHITE; TextBold[4] = false;
        mu_swprintf(TextList[5], GlobalText[360], iSkillMana);
        TextListColor[5] = TEXT_COLOR_WHITE; TextBold[5] = false;
        TextNum = 6; SkipNum = 2;
    }

    mu_swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

    if (CharacterAttribute->Skill[Type] == AT_SKILL_RUSH || CharacterAttribute->Skill[Type] == AT_SKILL_SPACE_SPLIT
        || CharacterAttribute->Skill[Type] == AT_SKILL_DEEPIMPACT || CharacterAttribute->Skill[Type] == AT_SKILL_JAVELIN
        || CharacterAttribute->Skill[Type] == AT_SKILL_SPIRAL_SLASH || CharacterAttribute->Skill[Type] == AT_SKILL_DEATH_CANNON
        || CharacterAttribute->Skill[Type] == AT_SKILL_OCCUPY
        )
    {
        mu_swprintf(TextList[TextNum], GlobalText[2047]);
        TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
    }
    if (CharacterAttribute->Skill[Type] == AT_SKILL_STUN || CharacterAttribute->Skill[Type] == AT_SKILL_REMOVAL_STUN
        || CharacterAttribute->Skill[Type] == AT_SKILL_INVISIBLE || CharacterAttribute->Skill[Type] == AT_SKILL_REMOVAL_INVISIBLE
        || CharacterAttribute->Skill[Type] == AT_SKILL_REMOVAL_BUFF)
    {
        mu_swprintf(TextList[TextNum], GlobalText[2048]);
        TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
    }
    if (CharacterAttribute->Skill[Type] == AT_SKILL_IMPALE)
    {
        mu_swprintf(TextList[TextNum], GlobalText[2049]);
        TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
    }

    if (SkillType == AT_SKILL_EXPANSION_OF_WIZARDRY || SkillType == AT_SKILL_EXPANSION_OF_WIZARDRY_STR || SkillType == AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY)
    {
        mu_swprintf(TextList[TextNum], GlobalText[2054]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
    }

    SIZE TextSize = { 0, 0 };
    GetTextExtentPoint32(g_pRenderText->GetFontDC(), TextList[0], 1, &TextSize);

    if (iRenderPoint == STRP_NONE)
    {
        int Height = ((TextNum - SkipNum) * TextSize.cy + SkipNum * TextSize.cy / 2) / g_fScreenRate_y;
        sy -= Height;
    }

    RenderTipTextList(sx, sy, TextNum, 0, RT3_SORT_CENTER, iRenderPoint);
}

}  // namespace UI::Skills::Tooltip
