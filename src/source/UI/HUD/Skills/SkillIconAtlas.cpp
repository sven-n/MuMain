#include "UI/HUD/Skills/SkillIconAtlas.h"

#include "Core/Platform/WinCompat.h" // BYTE, used by _enum.h
#include "Core/Globals/_enum.h"
#include "UI/HUD/Skills/MasterSkillTreeLayout.h"

namespace UI::Skills::Icon
{
namespace
{
constexpr int kMasterAtlasColumns = 25;

SkillIcon Cell(Atlas atlas, int column, int row, bool usable)
{
    return SkillIcon{atlas, column, row, usable};
}

const char* AtlasSpritePrefix(Atlas atlas)
{
    switch (atlas)
    {
    case Atlas::Skill1:
        return "skill1";
    case Atlas::Skill2:
        return "skill2";
    case Atlas::Skill3:
        return "skill3";
    case Atlas::Command:
        return "command";
    default:
        return nullptr;
    }
}
} // namespace

SkillIcon ResolveSkillIcon(const SkillIconSource& skill)
{
    const int skillType = skill.skillType;
    const int magicIcon = skill.magicIcon;
    const bool usable = skill.usable;

    if (skillType == 0)
    {
        return {};
    }

    if (skillType >= AT_SKILL_MASTER_BEGIN)
    {
        return Cell(Atlas::Master, magicIcon % kMasterAtlasColumns, magicIcon / kMasterAtlasColumns, usable);
    }

    // The original's own upper bound is inclusive.
    if (skillType >= AT_PET_COMMAND_DEFAULT && skillType <= AT_PET_COMMAND_END)
    {
        const int command = skillType - AT_PET_COMMAND_DEFAULT;
        return Cell(Atlas::Command, command % 8, command / 8, usable);
    }
    if (skillType == AT_SKILL_PLASMA_STORM_FENRIR)
        return Cell(Atlas::Command, 4, 0, usable);
    if (skillType >= AT_SKILL_ALICE_DRAINLIFE && skillType <= AT_SKILL_ALICE_THORNS)
        return Cell(Atlas::Skill2, (skillType - AT_SKILL_ALICE_DRAINLIFE) % 8, 3, usable);
    if (skillType >= AT_SKILL_ALICE_SLEEP && skillType <= AT_SKILL_ALICE_BLIND)
        return Cell(Atlas::Skill2, (skillType - AT_SKILL_ALICE_SLEEP + 4) % 8, 3, usable);
    if (skillType == AT_SKILL_ALICE_BERSERKER)
        return Cell(Atlas::Skill2, 10, 3, usable);
    if (skillType >= AT_SKILL_ALICE_WEAKNESS && skillType <= AT_SKILL_ALICE_ENERVATION)
        return Cell(Atlas::Skill2, skillType - AT_SKILL_ALICE_WEAKNESS + 8, 3, usable);
    if (skillType >= AT_SKILL_SUMMON_EXPLOSION && skillType <= AT_SKILL_SUMMON_REQUIEM)
        return Cell(Atlas::Skill2, (skillType - AT_SKILL_SUMMON_EXPLOSION + 6) % 8, 3, usable);
    if (skillType == AT_SKILL_SUMMON_POLLUTION)
        return Cell(Atlas::Skill2, 11, 3, usable);
    if (skillType == AT_SKILL_STRIKE_OF_DESTRUCTION)
        return Cell(Atlas::Skill2, 7, 2, usable);
    if (skillType == AT_SKILL_CHAOTIC_DISEIER)
        return Cell(Atlas::Skill2, 3, 8, usable);
    if (skillType == AT_SKILL_RECOVER)
        return Cell(Atlas::Skill2, 9, 2, usable);
    if (skillType == AT_SKILL_MULTI_SHOT)
        return Cell(Atlas::Skill2, 0, 8, usable);
    if (skillType == AT_SKILL_FLAME_STRIKE)
        return Cell(Atlas::Skill2, 1, 8, usable);
    if (skillType == AT_SKILL_GIGANTIC_STORM)
        return Cell(Atlas::Skill2, 2, 8, usable);
    if (skillType == AT_SKILL_LIGHTNING_SHOCK)
        return Cell(Atlas::Skill2, 2, 3, usable);
    if (skillType == AT_SKILL_EXPANSION_OF_WIZARDRY)
        return Cell(Atlas::Skill2, 8, 2, usable);
    if (skill.skillUseType == SKILL_USE_TYPE_MASTERACTIVE)
        return Cell(Atlas::Skill2, magicIcon % 12, magicIcon / 12 + 4, usable);
    if (skillType >= AT_SKILL_KILLING_BLOW)
        return Cell(Atlas::Skill3, (skillType - AT_SKILL_KILLING_BLOW) % 12, (skillType - AT_SKILL_KILLING_BLOW) / 12,
                    usable);
    if (skillType >= AT_SKILL_SPIRAL_SLASH)
        return Cell(Atlas::Skill2, (skillType - AT_SKILL_SPIRAL_SLASH) % 8, (skillType - AT_SKILL_SPIRAL_SLASH) / 8,
                    usable);
    return Cell(Atlas::Skill1, (skillType - 1) % 8, (skillType - 1) / 8, usable);
}

std::string IconSpriteName(const SkillIcon& icon)
{
    if (icon.atlas == Atlas::None)
    {
        return {};
    }
    if (icon.atlas == Atlas::Master)
    {
        return UI::Skills::MasterTree::IconSpriteName(icon.row * kMasterAtlasColumns + icon.column, icon.usable);
    }
    return std::string("skill-icon-") + AtlasSpritePrefix(icon.atlas) + (icon.usable ? "-lit-" : "-grey-") +
           std::to_string(icon.column) + "-" + std::to_string(icon.row);
}
} // namespace UI::Skills::Icon
