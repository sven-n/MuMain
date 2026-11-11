#pragma once

#include "MonsterModelRecorder.h"

#include <memory>
#include <string_view>
#include <vector>

class CHARACTER;

// The monster recorder's creation part: what CreateMonster needs, as the game
// sets it up: a character table with the hero in it, the map process, and
// the model slots of a LoadScope. The models a creation opens stay open for
// the scope, so creating the monsters of many maps opens each model once.
namespace MonsterRecorder
{
// How the recorder brings a monster to life (monsters design, section 10):
// by CreateMonster alone, by a packet of the server's, made up, through its
// handler, or as a cutscene's monster.
enum class CreationPath
{
    CreateMonster,
    // 0x13: a monster or NPC that comes into view; one the server has just
    // spawned (its appear); one that teleported.
    Viewport,
    ViewportNew,
    ViewportTeleport,
    // 0x1F: a summon (the Kalima gates come from CreateHellGate); a new one.
    Summon,
    SummonNew,
    // 0x45: a player transformed into the monster; newly.
    Transform,
    TransformNew,
    // 0x68: the castle siege's preview of a gate's surroundings.
    PreviewPort,
    // CDirection::SummonCreateMonster with its summon effects.
    Cinematic,
};

const char* Describe(CreationPath path);

class CreationScope
{
public:
    // The keys of the hero and of the created monster, which the server
    // never sends. Bit 0x4000 is 0x13's teleport flag.
    static constexpr int TestHeroKey = 0x3F00;
    static constexpr int TestMonsterKey = 0x3F01;

    explicit CreationScope(int world);
    ~CreationScope();

    CreationScope(const CreationScope&) = delete;
    CreationScope& operator=(const CreationScope&) = delete;

    void SetWorld(int world);
    // The map as the server sends it, as LoadWorld sets it: WorldActive
    // becomes the world the client loads for it (Devil Square for 32), and
    // the stage is the one the server sent with it (0: none).
    void SetServerMap(int serverMap, int stage);
    // Turns a state the monster data's state variants name on or off
    // (Data::Monsters::GetMonsterStates): the castle siege, Raklion's waiting
    // boss, Crywolf's battle about to start. The scope ends with the siege
    // off, Raklion idle and Crywolf in no state. A transformed
    // player is the path 0x45 (Create), not a state the scope sets.
    void SetSpawnState(std::string_view state, bool on);

    // Creates the monster with CreateMonster on the world that is set, and
    // returns the character's model slot. The character is gone afterwards:
    // the next creation finds its slot free.
    int CreatedModel(int number);

    // Creates the monster with CreateMonster on the world that is set and
    // returns the character, which lives until the next Create or
    // CreatedModel; nullptr when it was not created.
    CHARACTER* Create(int number);
    // The same by the path: the packet's handler or the cutscene makes the
    // monster at the tile beside the hero, facing the direction 3 (90).
    CHARACTER* Create(int number, CreationPath path);
    // The key the monster comes with by the path: a teleported one keeps the
    // flag in its key (0x13), a cutscene's monster gets the cutscene's.
    static int KeyOf(CreationPath path);
    // The buffs the packets of the next creations carry (eBuffState values,
    // a castle gate's "gate is open"); none when the scope starts.
    void SetPacketBuffs(std::vector<int> buffs);
    // The server gives the character a buff or takes it (0x07,
    // ReceiveSkillStatus), and ends a skill's debuff on it (0x1B,
    // ReceiveMagicFinish: AT_SKILL_POISON ends poison).
    static void SendBuffState(const CHARACTER& character, int buff, bool add);
    static void SendMagicFinish(const CHARACTER& character, int skill);

    LoadScope& Load()
    {
        return *m_load;
    }

private:
    struct State;
    std::unique_ptr<LoadScope> m_load;
    std::unique_ptr<State> m_state;
};
} // namespace MonsterRecorder
