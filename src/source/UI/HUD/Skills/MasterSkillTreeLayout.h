#pragma once

#include <cstdint>
#include <string>

// Geometry and presentation rules of the master skill tree (CMasterLevel), free of any game state
// so they can be unit-tested. Every position is in the tree's own 640x480 reference space, the
// .hud-board the themes draw it on.
namespace UI::Skills::MasterTree
{
inline constexpr int kColumnCount = 3;
inline constexpr int kNodesPerRank = 4;

struct NodePosition
{
    int left;
    int top;
};

// Column (0..2) of a tree node's group, its slot within its rank (0..3) and its rank (1..):
// the top-left corner of the node's 50x38 box. The icon sits 8 right and 5 down of it.
//
// Each theme draws the grid itself, from the column/slot/rank the model carries, so this is not
// what places a node any more: it is what the skill hint falls back to before the theme's own
// layout can be read back. The two have to agree, the same way
// a window's hit-box fallback has to agree with its drawn panel width.
NodePosition NodeBoxPosition(int column, int slotInRank, int rank);

// A node's slot within its rank, from its 1-based index in the tree data.
int SlotInRank(int treeIndex);

// The @spritesheet sprite drawing a skill's icon cell (SKILL_ATTRIBUTE::Magic_Icon) from the
// lit atlas (new_Master_Icon.jpg) when the node can be raised, else from the grey one
// (new_Master_Non_Icon.jpg).
std::string IconSpriteName(int magicIcon, bool usable);

// The master level and experience as the server sends them: total experience, not progress
// within the level.
struct MasterExperience
{
    int level;
    std::int64_t experience;
    std::int64_t nextLevelExperience;
};

// Progress through the current master level in percent, as the original header shows it: a
// master level's range starts where the experience curve puts total level `level` + 400
// (master levels continue the normal 400 levels).
double ExperiencePercent(const MasterExperience& master);
} // namespace UI::Skills::MasterTree
