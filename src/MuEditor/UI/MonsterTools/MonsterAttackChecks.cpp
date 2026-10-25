#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterAttackChecks.h"

#include "MonsterAnimationFacts.h"
#include "MonsterAttackReaders.h"
#include "MonsterModelValues.h"

#include "Core/Globals/_define.h"
#include "Core/Globals/_enum.h"
#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/DataHandler/MonsterData/MonsterModelLoader.h"
#include "Data/GameData/MonsterData/MonsterAnimations.h"
#include "Data/GameData/MonsterData/MonsterAttacks.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterDefinition.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "Render/Models/ZzzBMD.h"

#include <map>
#include <string>

namespace MuEditor::Monsters::AttackChecks
{
namespace
{
using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using Data::Monsters::MonsterAttackEntry;
using Data::Monsters::MonsterAttackRandom;
using Data::Monsters::MonsterAttacks;
using Data::Monsters::MonsterDefinition;

struct MonsterProblems
{
    const MonsterDefinition& monster;
    std::vector<ItemDataIssue>& issues;
    std::string source;

    void Add(const std::string& message) const
    {
        issues.push_back({ItemDataIssueSeverity::Warning, source, ItemDataIssue::NoItem, monster.number,
                          "monster " + std::to_string(monster.number) + " attacks", message});
    }
};

std::string AnimationLabel(int animation)
{
    return Data::Monsters::GetMonsterAnimationName(animation) + " (" + std::to_string(animation) + ")";
}

// "monster skills 36, 41", "player skill 12".
std::string SkillList(const char* one, const char* more, const std::vector<int>& skills)
{
    std::string text = skills.size() == 1 ? one : more;
    for (size_t i = 0; i < skills.size(); ++i)
    {
        text += (i == 0 ? " " : ", ") + std::to_string(skills[i]);
    }
    return text;
}

// What plays an animation: its skills, any other attack, a draw.
struct AnimationUses
{
    std::vector<int> monsterSkills;
    std::vector<int> skills;
    bool otherwise = false;
    bool draw = false;

    std::string Describe() const
    {
        std::vector<std::string> parts;
        if (!monsterSkills.empty())
        {
            parts.push_back(SkillList("monster skill", "monster skills", monsterSkills));
        }
        if (!skills.empty())
        {
            parts.push_back(SkillList("player skill", "player skills", skills));
        }
        if (otherwise)
        {
            parts.emplace_back("any other attack");
        }
        if (draw)
        {
            parts.emplace_back("the draw");
        }
        std::string text;
        for (size_t i = 0; i < parts.size(); ++i)
        {
            text += (i == 0 ? "" : (i + 1 == parts.size() ? " and " : ", ")) + parts[i];
        }
        return text;
    }
};

// The animations the draw can pick (MonsterAttackPlay's Draw).
std::vector<int> DrawnAnimations(MonsterAttackRandom random)
{
    if (random == MonsterAttackRandom::Bali)
    {
        return {MONSTER01_ATTACK1, MONSTER01_ATTACK2, MONSTER01_ATTACK3, MONSTER01_ATTACK4};
    }
    return {MONSTER01_ATTACK1, MONSTER01_ATTACK2};
}

std::map<int, AnimationUses> CollectUses(const MonsterAttacks& attacks)
{
    std::map<int, AnimationUses> uses;
    for (const auto& [skill, animation] : attacks.monsterSkills)
    {
        uses[animation].monsterSkills.push_back(skill);
    }
    for (const auto& [skill, animation] : attacks.skills)
    {
        uses[animation].skills.push_back(skill);
    }
    if (attacks.otherwiseRandom)
    {
        for (const int animation : DrawnAnimations(*attacks.otherwiseRandom))
        {
            uses[animation].draw = true;
        }
    }
    else
    {
        uses[attacks.otherwise].otherwise = true;
    }
    uses.erase(Data::Monsters::NoAttackAnimation);
    return uses;
}

void CheckMonsterSkills(const MonsterAttacks& attacks, const MonsterProblems& problems)
{
    for (const MonsterAttackEntry& entry : attacks.monsterSkills)
    {
        // ReceiveMonsterSkill plays an attack for the skills 1 to 77 only.
        if (entry.first <= ATMON_SKILL_BIGIN || entry.first >= ATMON_SKILL_END)
        {
            problems.Add("monster skill " + std::to_string(entry.first) +
                         ": packet 0x69 plays no attack for it (only for the skills 1 to " +
                         std::to_string(ATMON_SKILL_END - 1) + ")");
        }
    }
}

void CheckAnimation(int slot, int animation, const AnimationUses& uses, const MonsterDefinition& monster,
                    const OpenAnimationCount& openAnimationCount, const MonsterProblems& problems)
{
    const std::string played = AnimationLabel(animation) + " is played by " + uses.Describe();
    const std::optional<int> count = openAnimationCount(slot);
    const Data::Monsters::MonsterModelDefinition* model = g_MonsterModelDatabase.Find(slot);
    if (count && animation >= *count)
    {
        problems.Add(played + ", which " + monster.model + " does not have: the animation that plays goes on");
        return;
    }
    if (model != nullptr && ModelValues::GetSpeed(*model, slot, animation) <= 0.0f)
    {
        problems.Add(played + ", whose speed in " + monster.model +
                     " is 0: the monster stands still in it until its next action");
        return;
    }
    if (animation != MONSTER01_STOP1 && AnimationFacts::IsMonsterRange(slot) &&
        !AnimationFacts::GetNextAnimation(slot, animation))
    {
        problems.Add(played +
                     ": the model does not go back to stop1 after it, so it starts again or holds its last "
                     "frame until the monster's next action (only stop2, shock and the attacks go back, and appear "
                     "for Maya's hands and Selupan)");
    }
    if (animation != MONSTER01_ATTACK1 && animation != MONSTER01_ATTACK2)
    {
        problems.Add(played + ": the game counts only attack1 and attack2 as attacks; a hit plays shock over any other "
                              "animation, and only they have an impact frame");
    }
}

void CheckMonster(const MonsterDefinition& monster, const OpenAnimationCount& openAnimationCount,
                  const MonsterProblems& problems)
{
    const MonsterAttacks& attacks = *monster.attacks;
    const int slot = Data::Monsters::FindMonsterModelReference(monster.model).value_or(-1);
    if (slot == MODEL_PLAYER)
    {
        problems.Add("the monster has the player's shape: it chooses a player attack from its equipment, and "
                     "its attacks do nothing");
        return;
    }
    if (AttackReaders::FindReaders(monster.number, slot).empty())
    {
        problems.Add("no handler answers for this number and model on any map: the attacks do nothing, and the "
                     "monster plays attack1, attack2, attack2");
        return;
    }
    CheckMonsterSkills(attacks, problems);
    if (slot < 0)
    {
        return;
    }
    for (const auto& [animation, uses] : CollectUses(attacks))
    {
        CheckAnimation(slot, animation, uses, monster, openAnimationCount, problems);
    }
}
} // namespace

std::optional<int> GetOpenAnimationCount(int slot)
{
    if (!Data::Monsters::ModelLoader::IsOpen(slot))
    {
        return std::nullopt;
    }
    return Models[slot].NumActions;
}

void Validate(std::vector<ItemDataIssue>& issues, const OpenAnimationCount& openAnimationCount)
{
    const std::string source = Data::Monsters::GetMonstersFile().generic_string();
    for (int number = 0; number < Data::Monsters::MonsterNumberCount; ++number)
    {
        const MonsterDefinition* monster = g_MonsterDatabase.Find(number);
        if (monster == nullptr || !monster->attacks)
        {
            continue;
        }
        CheckMonster(*monster, openAnimationCount, MonsterProblems{*monster, issues, source});
    }
}
} // namespace MuEditor::Monsters::AttackChecks

#endif // _EDITOR
