#include "stdafx.h"

// The attack animations of monsters in the game (monsters design, MN2): the
// attacks of their entries, as SetPlayerAttack plays them on their maps, and
// the fixes of the problems the old code had.

#include "MonsterCreationScope.h"

#include "Audio/DSPlaySound.h"
#include "Core/Globals/_define.h"
#include "Core/Globals/_enum.h"
#include "Core/Utilities/Random.h"
#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Engine/Object/MonsterSlotSound.h"
#include "Engine/Object/ZzzCharacter.h"
#include "GameLogic/Monsters/MonsterAttackPlay.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "World/MapInfra/MapManager.h"
#include "World/MapInfra/w_MapProcess.h"

#include <doctest.h>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <iterator>
#include <vector>

// The end of a monster's animation (MoveCharacter calls it).
void EtcStopAnimationSetting(CHARACTER* c, OBJECT* o);

namespace
{
constexpr int NoMonsterSkill = 0xFFFF;

// The shipped monster list and a creation scope on the world.
class AttackScope
{
public:
    explicit AttackScope(int world)
        : m_scope(world), m_monsters(Data::Monsters::LoadMonstersFile(MonsterRecorder::ClientFolder() / "Data" /
                                                                      "Monsters" / "Monsters.json")
                                         .monsters)
    {
        REQUIRE(m_monsters.size() > 500);
        g_MonsterDatabase.Build(m_monsters, "en");
        g_MapProcess = MapProcess::Make();
    }

    ~AttackScope()
    {
        g_MonsterDatabase.Build(m_monsters, "en");
    }

    AttackScope(const AttackScope&) = delete;
    AttackScope& operator=(const AttackScope&) = delete;

    CHARACTER& Create(int number)
    {
        CHARACTER* character = m_scope.Create(number);
        REQUIRE(character != nullptr);
        return *character;
    }

    // The ids of the sounds played since the last call.
    std::vector<int> TakePlayedSounds()
    {
        std::vector<int> ids;
        for (const MonsterRecorder::PlayedSound& sound : m_scope.Load().TakePlayedSounds())
        {
            ids.push_back(sound.id);
        }
        return ids;
    }

    // The monster list with the monster's entry without attacks, for the
    // life of the scope.
    void RemoveAttacks(int number)
    {
        SetAttacks(number, std::nullopt);
    }

    // The monster list with these attacks in the monster's entry, for the
    // life of the scope.
    void SetAttacks(int number, const std::optional<Data::Monsters::MonsterAttacks>& attacks)
    {
        Change(number, [&](Data::Monsters::MonsterDefinition& monster) { monster.attacks = attacks; });
    }

    // The monster list with the monster's entry changed, for the life of the
    // scope.
    template <typename Edit> void Change(int number, Edit edit)
    {
        std::vector<Data::Monsters::MonsterDefinition> changed = m_monsters;
        const auto monster =
            std::find_if(changed.begin(), changed.end(),
                         [number](const Data::Monsters::MonsterDefinition& entry) { return entry.number == number; });
        REQUIRE(monster != changed.end());
        REQUIRE(monster->attacks.has_value());
        edit(*monster);
        g_MonsterDatabase.Build(changed, "en");
    }

private:
    MonsterRecorder::CreationScope m_scope;
    std::vector<Data::Monsters::MonsterDefinition> m_monsters;
};

// The model table without one model's entry, for the life of the scope.
class ModelTableWithout
{
public:
    explicit ModelTableWithout(int slot)
    {
        std::vector<Data::Monsters::MonsterModelDefinition> changed;
        for (const Data::Monsters::MonsterModelDefinition* model : g_MonsterModelDatabase.GetAll())
        {
            m_models.push_back(*model);
            if (g_MonsterModelDatabase.Find(slot) != model)
            {
                changed.push_back(*model);
            }
        }
        REQUIRE(changed.size() + 1 == m_models.size());
        g_MonsterModelDatabase.Build(changed);
    }

    ~ModelTableWithout()
    {
        g_MonsterModelDatabase.Build(m_models);
    }

    ModelTableWithout(const ModelTableWithout&) = delete;
    ModelTableWithout& operator=(const ModelTableWithout&) = delete;

private:
    std::vector<Data::Monsters::MonsterModelDefinition> m_models;
};

// An attack with these skills, from a walk; the animation it plays. A
// monster skill comes with its packet (0x69), which starts the attack.
int Attack(CHARACTER& character, int monsterSkill, int skill = 0)
{
    character.MonsterSkill = static_cast<WORD>(monsterSkill);
    character.Skill = static_cast<WORD>(skill);
    character.Object.CurrentAction = MONSTER01_WALK;
    character.Object.AnimationFrame = 0.0f;
    if (monsterSkill != NoMonsterSkill)
    {
        const GameLogic::Monsters::Attacks::SkillAttackScope skillAttack(character);
        SetPlayerAttack(&character);
    }
    else
    {
        SetPlayerAttack(&character);
    }
    return character.Object.CurrentAction;
}

// A normal attack packet where the monster is now, the monster skill as it
// is; the animation it plays.
int NormalAttack(CHARACTER& character)
{
    character.Skill = 0;
    SetPlayerAttack(&character);
    return character.Object.CurrentAction;
}
} // namespace

TEST_CASE("Monster attacks: the Kanturu 1st warriors play attack 2 for their skill at any count (P3) "
          "[monsters][attacks]")
{
    AttackScope scope(WD_37KANTURU_1ST);
    struct Warrior
    {
        int number;
        int skill;
    };
    for (const Warrior warrior : {Warrior{MONSTER_KENTAUROS, ATMON_SKILL_NUM9},
                                  Warrior{MONSTER_BERSERKER_WARRIOR, ATMON_SKILL_EX_BERSERKERWARRIOR_ATTACKSKILL},
                                  Warrior{MONSTER_KENTAUROS_WARRIOR, ATMON_SKILL_EX_KENTAURUSWARRIOR_ATTACKSKILL},
                                  Warrior{MONSTER_GENOCIDER_WARRIOR, ATMON_SKILL_EX_GENOSIDEWARRIOR_ATTACKSKILL}})
    {
        INFO("monster " << warrior.number);
        CHARACTER& character = scope.Create(warrior.number);
        // The generic pattern played attack 1 at counts 0 and 3 and
        // overwrote the skill's animation.
        for (const BYTE count : {BYTE{0}, BYTE{1}, BYTE{3}})
        {
            character.SwordCount = count;
            CHECK(Attack(character, warrior.skill) == MONSTER01_ATTACK2);
            CHECK(character.MonsterSkill == NoMonsterSkill);
            character.SwordCount = count;
            CHECK(Attack(character, NoMonsterSkill) == MONSTER01_ATTACK1);
        }
    }
}

TEST_CASE("Monster attacks: a rotation plays in turn, counted for each monster from its creation (Q-MN69) "
          "[monsters][attacks]")
{
    AttackScope scope(WD_37KANTURU_1ST);
    Data::Monsters::MonsterAttacks rotation;
    rotation.otherwise = {MONSTER01_ATTACK2, MONSTER01_ATTACK1, MONSTER01_ATTACK1};
    scope.SetAttacks(MONSTER_BERSERKER, rotation);
    CHARACTER& first = scope.Create(MONSTER_BERSERKER);
    const std::array<int, 4> expected = {MONSTER01_ATTACK2, MONSTER01_ATTACK1, MONSTER01_ATTACK1, MONSTER01_ATTACK2};
    for (size_t turn = 0; turn < expected.size(); ++turn)
    {
        INFO("turn " << turn);
        // The generic pattern's count does not move the rotation.
        first.SwordCount = static_cast<BYTE>(turn * 5 + 1);
        CHECK(NormalAttack(first) == expected[turn]);
    }

    // A monster created in the same slot starts its rotation again.
    CHARACTER& second = scope.Create(MONSTER_BERSERKER);
    REQUIRE(&second == &first);
    CHECK(NormalAttack(second) == MONSTER01_ATTACK2);
    CHECK(NormalAttack(second) == MONSTER01_ATTACK1);
}

TEST_CASE("Monster attacks: a map variant plays on its maps, the monster's attacks elsewhere (Q-MN65) "
          "[monsters][attacks]")
{
    AttackScope scope(WD_24HELLAS);
    scope.Change(MONSTER_DEATH_CENTURION_1,
                 [](Data::Monsters::MonsterDefinition& monster)
                 {
                     monster.attacks->otherwise = {MONSTER01_ATTACK1};
                     monster.attacks->otherwiseRandom.reset();
                     Data::Monsters::MonsterMapVariant kalima2;
                     kalima2.maps = {{WD_24HELLAS + 1}};
                     kalima2.attacks.emplace();
                     kalima2.attacks->otherwise = {MONSTER01_ATTACK2};
                     monster.mapVariants = {kalima2};
                 });
    CHARACTER& centurion = scope.Create(MONSTER_DEATH_CENTURION_1);
    gMapManager.SetServerMap(WD_24HELLAS + 1);
    CHECK(NormalAttack(centurion) == MONSTER01_ATTACK2);
    gMapManager.SetServerMap(WD_24HELLAS);
    CHECK(NormalAttack(centurion) == MONSTER01_ATTACK1);

    // Devil Square 5-7 is the server's map 32, which the client loads as
    // world 9 (D67).
    scope.Change(MONSTER_DARK_ELF,
                 [](Data::Monsters::MonsterDefinition& monster)
                 {
                     Data::Monsters::MonsterMapVariant devilSquare5To7;
                     devilSquare5To7.maps = {{32}};
                     devilSquare5To7.attacks.emplace();
                     devilSquare5To7.attacks->otherwise = {MONSTER01_ATTACK2};
                     monster.mapVariants = {devilSquare5To7};
                 });
    gMapManager.SetServerMap(32);
    CHARACTER& elf = scope.Create(MONSTER_DARK_ELF);
    CHECK(NormalAttack(elf) == MONSTER01_ATTACK2);
    gMapManager.SetServerMap(WD_9DEVILSQUARE);
    CHECK(NormalAttack(elf) == MONSTER01_ATTACK1);
}

TEST_CASE("Monster attacks: a map variant of a stage plays in that stage of Devil Square only (D67) "
          "[monsters][attacks]")
{
    AttackScope scope(WD_9DEVILSQUARE);
    scope.Change(MONSTER_DARK_ELF,
                 [](Data::Monsters::MonsterDefinition& monster)
                 {
                     Data::Monsters::MonsterMapVariant stage2;
                     stage2.maps = {{WD_9DEVILSQUARE, 2}};
                     stage2.attacks.emplace();
                     stage2.attacks->otherwise = {MONSTER01_ATTACK2};
                     monster.mapVariants = {stage2};
                 });
    gMapManager.SetServerStage(2);
    gMapManager.SetServerMap(WD_9DEVILSQUARE);
    CHARACTER& elf = scope.Create(MONSTER_DARK_ELF);
    CHECK(NormalAttack(elf) == MONSTER01_ATTACK2);
    gMapManager.SetServerStage(3);
    CHECK(NormalAttack(elf) == MONSTER01_ATTACK1);
    gMapManager.SetServerStage(0);
    CHECK(NormalAttack(elf) == MONSTER01_ATTACK1);
}

TEST_CASE("Monster attacks: the shipped map variants keep the Fire Golem's Devil Square pattern and Erkanne still in "
          "the market (Q-MN64) [monsters][attacks]")
{
    AttackScope scope(WD_31HUNTING_GROUND);
    CHARACTER& golem = scope.Create(MONSTER_FIRE_GOLEM);
    golem.SwordCount = 0;
    CHECK(NormalAttack(golem) == MONSTER01_ATTACK2);
    // In Devil Square 5-7 it is round 5's boss and plays the generic pattern.
    gMapManager.SetServerMap(32);
    golem.SwordCount = 0;
    for (const int expected : {MONSTER01_ATTACK1, MONSTER01_ATTACK2, MONSTER01_ATTACK2, MONSTER01_ATTACK1})
    {
        CHECK(NormalAttack(golem) == expected);
    }

    gMapManager.SetServerMap(WD_79UNITEDMARKETPLACE);
    CHARACTER& erkanne = scope.Create(MONSTER_LUCAS);
    CHECK(Attack(erkanne, NoMonsterSkill) == MONSTER01_WALK);
    CHECK(Attack(erkanne, 62) == MONSTER01_WALK);
    CHECK(erkanne.MonsterSkill == 62);
}

TEST_CASE("Monster attacks: a Battle Castle guard takes its attack state, its attacks play as data (Q-MN64) "
          "[monsters][attacks]")
{
    AttackScope scope(WD_30BATTLECASTLE);
    CHARACTER& archer = scope.Create(MONSTER_BATTLE_GUARD1);
    archer.Object.AI = 0;
    CHECK(Attack(archer, NoMonsterSkill) == MONSTER01_SHOCK);
    CHECK(archer.Object.AI != 0);
}

TEST_CASE("Monster attacks: \"generic\" plays as a monster without attacks, at any count (Q-MN69) "
          "[monsters][attacks]")
{
    AttackScope scope(WD_37KANTURU_1ST);
    CHARACTER& character = scope.Create(MONSTER_BERSERKER);
    for (int count = 0; count < 256; count += 7)
    {
        INFO("SwordCount " << count);
        scope.RemoveAttacks(MONSTER_BERSERKER);
        character.SwordCount = static_cast<BYTE>(count);
        const int without = NormalAttack(character);
        const BYTE countWithout = character.SwordCount;

        Data::Monsters::MonsterAttacks generic;
        generic.otherwiseGeneric = true;
        scope.SetAttacks(MONSTER_BERSERKER, generic);
        character.SwordCount = static_cast<BYTE>(count);
        CHECK(NormalAttack(character) == without);
        CHECK(character.SwordCount == countWithout);
    }
}

TEST_CASE("Monster attacks: Selupan and the Dark Ghost clear their skill when the attack ends (P4) "
          "[monsters][attacks]")
{
    {
        AttackScope scope(WD_58ICECITY_BOSS);
        CHARACTER& selupan = scope.Create(MONSTER_SELUPAN);
        CHECK(Attack(selupan, 34) == MONSTER01_ATTACK2);
        // The effects of the skill read it while the attack plays.
        CHECK(selupan.MonsterSkill == 34);
        EtcStopAnimationSetting(&selupan, &selupan.Object);
        CHECK(selupan.MonsterSkill == NoMonsterSkill);
        // The next normal attack does not repeat the skill (attack 2).
        CHECK(Attack(selupan, selupan.MonsterSkill) == MONSTER01_ATTACK4);
    }
    {
        AttackScope scope(WD_58ICECITY_BOSS);
        CHARACTER& selupan = scope.Create(MONSTER_SELUPAN);
        // A hit's shock ends attack 3 before its end, which clears the skill.
        CHECK(Attack(selupan, 36) == MONSTER01_ATTACK3);
        SetPlayerShock(&selupan, 1);
        CHECK(selupan.Object.CurrentAction == MONSTER01_SHOCK);
        CHECK(selupan.MonsterSkill == NoMonsterSkill);
        CHECK(Attack(selupan, selupan.MonsterSkill) == MONSTER01_ATTACK4);
        // Attack 1 and 2 are not shocked: the skill stays until they end.
        CHECK(Attack(selupan, 34) == MONSTER01_ATTACK2);
        SetPlayerShock(&selupan, 1);
        CHECK(selupan.Object.CurrentAction == MONSTER01_ATTACK2);
        CHECK(selupan.MonsterSkill == 34);
        // Skill 41 plays no attack, so it is cleared at once.
        CHECK(Attack(selupan, 41) == MONSTER01_WALK);
        CHECK(selupan.MonsterSkill == NoMonsterSkill);
        CHECK(Attack(selupan, selupan.MonsterSkill) == MONSTER01_ATTACK4);
    }
    {
        AttackScope scope(WD_58ICECITY_BOSS);
        CHARACTER& selupan = scope.Create(MONSTER_SELUPAN);
        scope.TakePlayedSounds();
        // The heal (39) plays attack 4 with its sound and effects.
        CHECK(Attack(selupan, 39) == MONSTER01_ATTACK4);
        const std::vector<int> heal = scope.TakePlayedSounds();
        CHECK(std::count(heal.begin(), heal.end(), static_cast<int>(SOUND_RAKLION_SERUFAN_CURE)) == 1);
        // A normal attack while it plays is no second heal: the attack goes
        // on, and the skill is cleared.
        selupan.Object.AnimationFrame = 2.0f;
        CHECK(NormalAttack(selupan) == MONSTER01_ATTACK4);
        CHECK(selupan.Object.AnimationFrame == 2.0f);
        CHECK(selupan.MonsterSkill == NoMonsterSkill);
        const std::vector<int> normal = scope.TakePlayedSounds();
        CHECK(std::count(normal.begin(), normal.end(), static_cast<int>(SOUND_RAKLION_SERUFAN_CURE)) == 0);
    }
    {
        AttackScope scope(WD_70EMPIREGUARDIAN2);
        CHARACTER& ghost = scope.Create(MONSTER_DARK_GHOST);
        // A normal attack while skill 52's attack 2 plays is attack 1, as for
        // the monsters that clear their skill when it is chosen.
        CHECK(Attack(ghost, 52) == MONSTER01_ATTACK2);
        CHECK(NormalAttack(ghost) == MONSTER01_ATTACK1);
        CHECK(ghost.MonsterSkill == NoMonsterSkill);
    }
    {
        AttackScope scope(WD_70EMPIREGUARDIAN2);
        CHARACTER& ghost = scope.Create(MONSTER_DARK_GHOST);
        CHECK(Attack(ghost, 51) == MONSTER01_ATTACK2);
        CHECK(ghost.MonsterSkill == 51);
        EtcStopAnimationSetting(&ghost, &ghost.Object);
        CHECK(ghost.MonsterSkill == NoMonsterSkill);
        CHECK(Attack(ghost, ghost.MonsterSkill) == MONSTER01_ATTACK1);
    }
}

TEST_CASE("Monster attacks: the Dark Iron Knight plays attack 2 for skill 77, the Dark Coolutin for 32 (P5) "
          "[monsters][attacks]")
{
    AttackScope scope(WD_57ICECITY);
    CHARACTER& knight = scope.Create(MONSTER_DARK_IRON_KNIGHT);
    CHECK(Attack(knight, 77) == MONSTER01_ATTACK2);
    CHECK(Attack(knight, 32) == MONSTER01_ATTACK1);
    CHARACTER& coolutin = scope.Create(MONSTER_DARK_COOLUTIN);
    CHECK(Attack(coolutin, 32) == MONSTER01_ATTACK2);
    CHECK(Attack(coolutin, 77) == MONSTER01_ATTACK1);
}

TEST_CASE("Monster attacks: the random Kanturu 1st monsters draw 50/50 at any frame rate (P18) [monsters][attacks]")
{
    AttackScope scope(WD_37KANTURU_1ST);
    const float savedFps = FPS_ANIMATION_FACTOR;
    for (const int number : {MONSTER_BERSERKER, MONSTER_GIGANTIS, MONSTER_GENOCIDER, MONSTER_SPLINTER_WOLF,
                             MONSTER_IRON_RIDER, MONSTER_GIGANTIS_WARRIOR})
    {
        INFO("monster " << number);
        CHARACTER& character = scope.Create(number);
        for (const unsigned int seed : {1u, 2u, 3u, 4u, 5u, 6u})
        {
            std::srand(seed);
            const int expected = MONSTER01_ATTACK1 + std::rand() % 2;
            // Before, the chance of attack 1 fell with the frame rate.
            for (const float fps : {1.0f, 0.25f})
            {
                FPS_ANIMATION_FACTOR = fps;
                std::srand(seed);
                Random::Seed(seed);
                CHECK(Attack(character, NoMonsterSkill) == expected);
            }
        }
    }
    FPS_ANIMATION_FACTOR = savedFps;
}

TEST_CASE("Monster attacks: the Berserker skill starts an Empire Guardian boss's rage, also when its entry has "
          "no attacks [monsters][attacks]")
{
    for (const bool withAttacks : {true, false})
    {
        INFO("attacks in the entry: " << withAttacks);
        AttackScope scope(WD_69EMPIREGUARDIAN1);
        if (!withAttacks)
        {
            scope.RemoveAttacks(MONSTER_RAYMOND);
        }
        CHARACTER& raymond = scope.Create(MONSTER_RAYMOND);
        Attack(raymond, ATMON_SKILL_EMPIREGUARDIAN_BERSERKER);
        // In its rage, each attack plays appear.
        CHECK(Attack(raymond, NoMonsterSkill) == MONSTER01_APEAR);
    }
}

TEST_CASE("Monster attacks: Castle Gate 1 counts its hits from 0 when it is set up, also without a model entry "
          "[monsters][attacks]")
{
    AttackScope scope(WD_30BATTLECASTLE);
    CHARACTER& gate = scope.Create(MONSTER_CASTLE_GATE1);
    REQUIRE(gate.Object.Type == MODEL_CASTLE_GATE1);
    gate.SwordCount = 5;
    CreateCharacterPointer(&gate, MODEL_CASTLE_GATE1, 0, 0, 0.0f);
    CHECK(gate.SwordCount == 0);

    const ModelTableWithout withoutEntry(MODEL_CASTLE_GATE1);
    gate.SwordCount = 5;
    CreateCharacterPointer(&gate, MODEL_CASTLE_GATE1, 0, 0, 0.0f);
    CHECK(gate.SwordCount == 0);
}

namespace
{
// The model's sound slots, put back at the end of the scope.
class SlotSoundsScope
{
public:
    explicit SlotSoundsScope(BMD& model) : m_model(model), m_saved(std::to_array(model.Sounds)) {}

    ~SlotSoundsScope()
    {
        std::copy(m_saved.begin(), m_saved.end(), std::begin(m_model.Sounds));
    }

    SlotSoundsScope(const SlotSoundsScope&) = delete;
    SlotSoundsScope& operator=(const SlotSoundsScope&) = delete;

private:
    BMD& m_model;
    std::array<short, MAX_MONSTER_SOUND> m_saved;
};

// How often the slot's sound is among the ids.
long long CountSlotSound(const std::vector<int>& ids, short sound)
{
    return std::count(ids.begin(), ids.end(), static_cast<int>(SOUND_MONSTER) + sound);
}
} // namespace

TEST_CASE("Monster sounds: a monster with SubType 9 plays its second set's slot sounds, also where its first set "
          "has none (P9) [monsters][sounds]")
{
    using Engine::Object::MonsterSlotSound::NoSound;
    AttackScope scope(WD_0LORENCIA);
    CHARACTER& bull = scope.Create(MONSTER_BULL_FIGHTER);
    bull.Object.SubType = Engine::Object::MonsterSlotSound::SecondSetSubType;
    BMD& model = Models[bull.Object.Type];
    const SlotSoundsScope slots(model);
    const short idle = model.Sounds[0];
    const short attack = model.Sounds[2];
    const short die = model.Sounds[4];
    REQUIRE(idle != NoSound);
    REQUIRE(attack != NoSound);
    REQUIRE(die != NoSound);
    // Only the second set names sounds, the same one for both slots of a pair.
    std::fill(std::begin(model.Sounds), std::end(model.Sounds), NoSound);
    model.Sounds[5] = idle;
    model.Sounds[6] = idle;
    model.Sounds[7] = attack;
    model.Sounds[8] = attack;
    model.Sounds[9] = die;
    scope.TakePlayedSounds();

    // Standing plays an idle sound now and then (walking checks the same
    // slot, but needs the hero's equipment).
    const float savedFps = FPS_ANIMATION_FACTOR;
    FPS_ANIMATION_FACTOR = 1.0f;
    std::srand(1);
    Random::Seed(1);
    for (int i = 0; i < 200; ++i)
    {
        SetPlayerStop(&bull);
    }
    FPS_ANIMATION_FACTOR = savedFps;
    CHECK(CountSlotSound(scope.TakePlayedSounds(), idle) > 0);

    Attack(bull, NoMonsterSkill);
    CHECK(CountSlotSound(scope.TakePlayedSounds(), attack) == 1);

    bull.Object.CurrentAction = MONSTER01_STOP1;
    bull.Object.AnimationFrame = 0.0f;
    SetPlayerShock(&bull, 1);
    CHECK(CountSlotSound(scope.TakePlayedSounds(), attack) == 1);

    SetPlayerDie(&bull);
    CHECK(CountSlotSound(scope.TakePlayedSounds(), die) == 1);
}
