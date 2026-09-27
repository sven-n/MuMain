#pragma once

#include <string>

// Which cell of which skill icon atlas the HUD draws for a skill (current-skill slot, hotkey row,
// expanded skill list, pet-command row), free of any game state so it can be unit-tested. The
// cells are 20x28 px; the four normal atlases (newui_skill.jpg, newui_skill2.jpg,
// newui_skill3.jpg, newui_command.jpg and their newui_non_* grey twins) are 256x256, the master
// atlases are the tree's (MasterSkillTreeLayout.h).
namespace UI::Skills::Icon
{
enum class Atlas
{
    None,
    Skill1,
    Skill2,
    Skill3,
    Command,
    Master,
};

struct SkillIcon
{
    Atlas atlas = Atlas::None;
    int column = 0;
    int row = 0;
    // Lit when the skill can be used now, else the grey twin atlas.
    bool usable = true;
};

struct SkillIconSource
{
    // The skill number (ActionSkillType), or the pet command for the pet-command row.
    int skillType = 0;
    // Its SKILL_ATTRIBUTE::SkillUseType and Magic_Icon.
    int skillUseType = 0;
    int magicIcon = 0;
    // Whether the caller's game-state checks allow the skill now.
    bool usable = true;
};

// Skill 0 has no icon. Mirrors the original CNewUISkillList::RenderSkillIcon() cell choice,
// special cases included.
SkillIcon ResolveSkillIcon(const SkillIconSource& skill);

// The @spritesheet sprite of an icon (skill_icons.rcss / master_skill_icons.rcss), or an empty
// string for Atlas::None.
std::string IconSpriteName(const SkillIcon& icon);
} // namespace UI::Skills::Icon
