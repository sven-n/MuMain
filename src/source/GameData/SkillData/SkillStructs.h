#pragma once

#include "_define.h"  // For MAX_SKILL_NAME, MAX_CLASS, MAX_DUTY_CLASS

// SKILL_ATTRIBUTE - Single source of truth for all builds
// This is the in-memory representation using UTF-16 strings
typedef struct
{
    wchar_t Name[MAX_SKILL_NAME];           // 32 wchar_t (UTF-16)
    WORD Level;                              // Required level
    WORD Damage;                             // Base damage
    WORD Mana;                               // Mana cost
    WORD AbilityGuage;                       // AG cost
    DWORD Distance;                          // Range/distance
    int Delay;                               // Cast delay
    int Energy;                              // Energy requirement
    WORD Charisma;                           // Charisma requirement
    BYTE MasteryType;                        // Mastery type
    BYTE SkillUseType;                       // Usage type
    DWORD SkillBrand;                        // Brand flags
    BYTE KillCount;                          // Kill count requirement
    BYTE RequireDutyClass[MAX_DUTY_CLASS];   // Duty class requirements (3)
    BYTE RequireClass[MAX_CLASS];            // Class requirements (7)
    BYTE SkillRank;                          // Skill rank
    WORD Magic_Icon;                         // Icon index
    BYTE TypeSkill;                          // Skill type
    int Strength;                            // Strength requirement
    int Dexterity;                           // Dexterity requirement
    BYTE ItemSkill;                          // Item skill flag
    BYTE IsDamage;                           // Damage flag
    WORD Effect;                             // Effect ID
} SKILL_ATTRIBUTE;
