#include "UI/HUD/Skills/MasterSkillTreeLayout.h"

namespace UI::Skills::MasterTree
{
namespace
{
constexpr int kColumnLeft[kColumnCount] = {11, 221, 431};
constexpr int kFirstRankTop = 55;
constexpr int kSlotPitch = 49;
constexpr int kRankPitch = 41;

// Master levels continue the character levels: the total level is the master level plus
// the 400 normal levels, and the curve gains a second term above level 255.
constexpr std::int64_t kNormalMaxLevel = 400;
constexpr std::int64_t kSecondCurveLevel = 255;
constexpr std::int64_t kFirstMasterLevelExperience = 3892250000;
} // namespace

NodePosition NodeBoxPosition(int column, int slotInRank, int rank)
{
    return {kColumnLeft[column] + kSlotPitch * slotInRank, kFirstRankTop + kRankPitch * (rank - 1)};
}

int SlotInRank(int treeIndex)
{
    return (treeIndex - 1) % kNodesPerRank;
}

std::string IconSpriteName(int magicIcon, bool usable)
{
    return std::string(usable ? "master-icon-lit-" : "master-icon-grey-") + std::to_string(magicIcon);
}

double ExperiencePercent(const MasterExperience& master)
{
    const std::int64_t totalLevel = master.level + kNormalMaxLevel;
    const std::int64_t overLevel = totalLevel - kSecondCurveLevel;
    const std::int64_t curve =
        (9 + totalLevel) * totalLevel * totalLevel * 10 + (9 + overLevel) * overLevel * overLevel * 1000;
    const std::int64_t levelStart = (curve - kFirstMasterLevelExperience) / 2;

    const double levelRange = static_cast<double>(master.nextLevelExperience) - static_cast<double>(levelStart);
    const double gained = static_cast<double>(master.experience) - static_cast<double>(levelStart);
    return gained / levelRange * 100.0;
}
} // namespace UI::Skills::MasterTree
