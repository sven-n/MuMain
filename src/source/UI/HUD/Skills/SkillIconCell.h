#pragma once

#include <string>

// Where a skill's icon sits in the native skill atlases -- the UV half of
// CSkillList::RenderSkillIcon(), with no rendering and no "can cast right now" greying. Pure, so it
// is shared by the native HUD draw, the RmlUi ports that bind a generated @spritesheet cell per icon
// (skill_icons.rcss, Tools/gen_skill_icon_sheets.py), and the unit tests. No RmlUi dependency, same
// rule as SkillTooltipModel.h.
namespace UI::Skills
{
    // The atlas a cell lives in. Every sheet is a grid of 20x28 cells.
    enum class IconSheet
    {
        None,     // no icon (an empty slot)
        Skill1,   // newui_skill.jpg      256x256, 12x9 cells
        Skill2,   // newui_skill2.jpg     256x256, 12x9 cells
        Skill3,   // newui_skill3.jpg     256x256, 12x9 cells
        Command,  // newui_command.jpg    256x256, 12x9 cells
        Master,   // new_Master_Icon.jpg  512x512, 25x18 cells
    };

    struct IconCell
    {
        IconSheet sheet = IconSheet::None;
        int col = 0;
        int row = 0;

        bool operator==(const IconCell&) const = default;
    };

    // `skillUseType` and `magicIcon` are SkillAttribute[skillType].SkillUseType / .Magic_Icon,
    // passed in so this is testable without the global skill table.
    IconCell ResolveIconCell(int skillType, int skillUseType, int magicIcon);

    // Reads SkillAttribute[] for the two attributes above.
    IconCell ResolveIconCell(int skillType);

    // The generated @spritesheet sprite for a cell, e.g. "skill-icon-skill2-3-8", or empty for
    // IconSheet::None. Must match what Tools/gen_skill_icon_sheets.py emits.
    std::string IconSpriteName(const IconCell& cell);
}
