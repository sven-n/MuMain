#pragma once

#include "Core/Globals/_define.h"

#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// Which animation a monster plays for the skill it attacks with (monsters
// design, D48): the `attacks` of a monster entry (Data/Monsters/Monsters.json).
namespace Data::Monsters
{
// No animation: the animation that plays goes on.
constexpr int NoAttackAnimation = -1;

// A draw that picks the animation for what no entry names, in place of
// `otherwise`: the old code's random choices.
enum class MonsterAttackRandom
{
    // Attack 1 or attack 2 by rand() % 2 (Hellas).
    Rand,
    // Attack 1 when rand_fps_check(2), else attack 2; the chance depends on
    // the frame rate (Kanturu 1st, P18).
    RandFps,
    // Bali's draw: rand() % 8 above 2 picks attack 1 or 2 by rand() % 2,
    // above 0 attack 3, else attack 4.
    Bali,
};

// When the monster skill (CHARACTER::MonsterSkill) is cleared after the
// animation was chosen; the player skill (CHARACTER::Skill) is never cleared.
enum class MonsterSkillClear
{
    Never,
    // When an entry of monsterSkills or skills named it.
    OnMatch,
    // When no entry named it.
    OnNoMatch,
    Always,
    // After the skill's own attack, the one its packet starts: its effects,
    // which read the skill while it plays, still see it; it is cleared when
    // the attack ends or a hit's shock stops it, at once when it plays no
    // attack, and by any other attack, which is a normal one (P4).
    AfterAttack,
};

// A skill number and the animation it plays (an animation number,
// MonsterAnimations.h, or NoAttackAnimation).
using MonsterAttackEntry = std::pair<int, int>;

struct MonsterAttacks
{
    // The monster skills (packet 0x69, CHARACTER::MonsterSkill), sorted.
    std::vector<MonsterAttackEntry> monsterSkills;
    // The player skills (packets 0x19 and 0x1E, CHARACTER::Skill), sorted.
    std::vector<MonsterAttackEntry> skills;
    // What any other skill, a stale skill or a normal attack plays.
    int otherwise = MONSTER01_ATTACK1;
    // Picks that in place of `otherwise` when it is set.
    std::optional<MonsterAttackRandom> otherwiseRandom;
    MonsterSkillClear clearSkill = MonsterSkillClear::Never;
    // A SOUND_ id the attacks play each time they play (the traps, Q-MN28);
    // empty: none.
    std::string sound;
    // At the monster (a 3D sound) instead of everywhere alike.
    bool soundAtMonster = false;

    bool operator==(const MonsterAttacks&) const = default;
};

// What the attacks choose for a monster skill and a player skill: a monster
// skill entry first, then a player skill entry, else `otherwise`.
struct MonsterAttackChoice
{
    // The animation, or NoAttackAnimation; with `random` the draw picks it.
    int animation = NoAttackAnimation;
    std::optional<MonsterAttackRandom> random;
    bool clearMonsterSkill = false;

    bool operator==(const MonsterAttackChoice&) const = default;
};
MonsterAttackChoice ChooseMonsterAttack(const MonsterAttacks& attacks, int monsterSkill, int skill);

// Whether the monster skill is cleared after the skill's own attack
// (AfterAttack).
bool ClearsMonsterSkillAfterAttack(const MonsterAttacks& attacks);

std::string_view GetMonsterAttackRandomName(MonsterAttackRandom random);
std::optional<MonsterAttackRandom> FindMonsterAttackRandom(std::string_view name);
std::string_view GetMonsterSkillClearName(MonsterSkillClear clear);
std::optional<MonsterSkillClear> FindMonsterSkillClear(std::string_view name);
} // namespace Data::Monsters
