#pragma once

#include "MUHelper/MuHelperData.h"

#include <string>

// State the MU Helper settings windows share: the config window, its detail panel and the skill
// picker all edit one staged copy of the bot's settings, which is pushed into g_MuHelper only on
// Save. The windows themselves never own it.
namespace UI::MuHelper
{
    MUHelper::ConfigData& StagedConfig();

    // Digits allowed in the settings' numeric fields (seconds, intervals).
    inline constexpr int MaxNumberDigits = 3;

    // Parses a numeric field the way native did: the whole string must be a number, anything else
    // (including an empty field) reads as 0.
    int ParseIntInput(const wchar_t* text);

    // A skill set to activate "on condition" needs both a pre-condition (which monsters count) and a
    // sub-condition (how many): with either group empty the bot's count never passes, and the skill
    // silently never fires. Fills an empty group with the detail panel's own Initialization defaults
    // -- monsters within hunting range, more than two -- and leaves an existing choice alone.
    void EnsureConditionDefaults(uint32_t& conditionBits);

    // Which of the config window's class-specific controls a base class sees. These are game rules,
    // so they live here and not in any theme's RCSS; the RML only hides what these say to hide.
    // Replaces native's four Register*Character tables (a class x control mask), which said the
    // same thing control by control.
    struct ClassFeatures
    {
        bool skill3 = false;          // the third attack skill slot, its timing and its Setting
        bool combo = false;           // Dark Knight's combo
        bool pet = false;             // Dark Lord's Dark Raven and its three modes
        bool party = false;           // party support and its Setting
        bool autoHeal = false;        // elf
        bool drainLife = false;       // summoner
        bool potionSummoner = false;  // the summoner's potion Setting sits 5 units higher
        int potionPage = -1;          // EMuHelperDetailPage the potion Setting opens
        int partyPage = -1;           // EMuHelperDetailPage the party Setting opens, if `party`
    };

    // `baseClass` is gCharacterManager.GetBaseClass()'s CLASS_* value.
    ClassFeatures ResolveClassFeatures(int baseClass);

    // A skill's icon as a data-style-decorator value ("image(<sprite>)", or "none" for no skill).
    // Always the lit icon: the settings show a skill's choice, not whether it can be used now.
    std::string SkillIconDecorator(int skillType);
}

// The detail panel's pages. The values line up with the config window's own button ids
// (BUTTON_ID_SKILL2_CONFIG == SUB_PAGE_SKILL2_CONFIG), which is how its "Setting" buttons map to a
// page -- keep them unscoped and in this order.
enum EMuHelperDetailPage
{
    SUB_PAGE_NONE = -1,
    SUB_PAGE_SKILL2_CONFIG = 2,
    SUB_PAGE_SKILL3_CONFIG,
    SUB_PAGE_POTION_CONFIG_ELF,
    SUB_PAGE_POTION_CONFIG_SUMMY,
    SUB_PAGE_POTION_CONFIG,
    SUB_PAGE_PARTY_CONFIG,
    SUB_PAGE_PARTY_CONFIG_ELF
};
