#include "stdafx.h"
#include "UI/HUD/Skills/SkillIconCell.h"

#include "Core/Globals/_enum.h"
#include "Engine/Object/ZzzInfomation.h"

namespace UI::Skills
{
    IconCell ResolveIconCell(int skillType, int skillUseType, int magicIcon)
    {
        if (skillType <= 0)
            return {};

        // Native tested this LAST, after the whole cascade below, and let it override whatever the
        // cascade picked -- so testing it first is exactly equivalent. It is also why the copies
        // that used to live in the MU Helper windows could diverge harmlessly: every branch they
        // disagreed on (ALICE_SLEEP_STR, LIGHTNING_SHOCK_STR, ALICE_BERSERKER_STR) is a skill id
        // at or past this line, and so never reached the cascade's own result.
        if (skillType >= AT_SKILL_MASTER_BEGIN)
            return { IconSheet::Master, magicIcon % 25, magicIcon / 25 };

        const int s = skillType;

        if (s >= AT_PET_COMMAND_DEFAULT && s <= AT_PET_COMMAND_END)
            return { IconSheet::Command, (s - AT_PET_COMMAND_DEFAULT) % 8, (s - AT_PET_COMMAND_DEFAULT) / 8 };
        if (s == AT_SKILL_PLASMA_STORM_FENRIR)
            return { IconSheet::Command, 4, 0 };
        if (s >= AT_SKILL_ALICE_DRAINLIFE && s <= AT_SKILL_ALICE_THORNS)
            return { IconSheet::Skill2, (s - AT_SKILL_ALICE_DRAINLIFE) % 8, 3 };
        if (s >= AT_SKILL_ALICE_SLEEP && s <= AT_SKILL_ALICE_BLIND)
            return { IconSheet::Skill2, (s - AT_SKILL_ALICE_SLEEP + 4) % 8, 3 };
        if (s == AT_SKILL_ALICE_BERSERKER)
            return { IconSheet::Skill2, 10, 3 };
        // No `% 8` here, unlike its neighbours -- native's own formula, kept as it is.
        if (s >= AT_SKILL_ALICE_WEAKNESS && s <= AT_SKILL_ALICE_ENERVATION)
            return { IconSheet::Skill2, s - AT_SKILL_ALICE_WEAKNESS + 8, 3 };
        if (s >= AT_SKILL_SUMMON_EXPLOSION && s <= AT_SKILL_SUMMON_REQUIEM)
            return { IconSheet::Skill2, (s - AT_SKILL_SUMMON_EXPLOSION + 6) % 8, 3 };
        if (s == AT_SKILL_SUMMON_POLLUTION)
            return { IconSheet::Skill2, 11, 3 };
        if (s == AT_SKILL_STRIKE_OF_DESTRUCTION)
            return { IconSheet::Skill2, 7, 2 };
        if (s == AT_SKILL_CHAOTIC_DISEIER)
            return { IconSheet::Skill2, 3, 8 };
        if (s == AT_SKILL_RECOVER)
            return { IconSheet::Skill2, 9, 2 };
        if (s == AT_SKILL_MULTI_SHOT)
            return { IconSheet::Skill2, 0, 8 };
        if (s == AT_SKILL_FLAME_STRIKE)
            return { IconSheet::Skill2, 1, 8 };
        if (s == AT_SKILL_GIGANTIC_STORM)
            return { IconSheet::Skill2, 2, 8 };
        if (s == AT_SKILL_LIGHTNING_SHOCK)
            return { IconSheet::Skill2, 2, 3 };
        if (s == AT_SKILL_EXPANSION_OF_WIZARDRY)
            return { IconSheet::Skill2, 8, 2 };
        if (skillUseType == 4)
            return { IconSheet::Skill2, magicIcon % 12, magicIcon / 12 + 4 };
        if (s >= AT_SKILL_KILLING_BLOW)
            return { IconSheet::Skill3, (s - AT_SKILL_KILLING_BLOW) % 12, (s - AT_SKILL_KILLING_BLOW) / 12 };
        if (s >= AT_SKILL_SPIRAL_SLASH)
            return { IconSheet::Skill2, (s - AT_SKILL_SPIRAL_SLASH) % 8, (s - AT_SKILL_SPIRAL_SLASH) / 8 };
        return { IconSheet::Skill1, (s - 1) % 8, (s - 1) / 8 };
    }

    IconCell ResolveIconCell(int skillType)
    {
        if (skillType <= 0 || SkillAttribute == nullptr)
            return {};
        return ResolveIconCell(skillType, SkillAttribute[skillType].SkillUseType, SkillAttribute[skillType].Magic_Icon);
    }

    std::string IconSpriteName(const IconCell& cell)
    {
        const char* sheet = nullptr;
        switch (cell.sheet)
        {
        case IconSheet::Skill1:  sheet = "skill1"; break;
        case IconSheet::Skill2:  sheet = "skill2"; break;
        case IconSheet::Skill3:  sheet = "skill3"; break;
        case IconSheet::Command: sheet = "command"; break;
        case IconSheet::Master:  sheet = "master"; break;
        case IconSheet::None:    return {};
        }
        return std::string("skill-icon-") + sheet + "-" + std::to_string(cell.col) + "-" + std::to_string(cell.row);
    }
}
