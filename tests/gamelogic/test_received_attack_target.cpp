#include "stdafx.h"
#include "doctest.h"

#include "Audio/DSPlaySound.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Network/Server/WSclient.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Effects/ZzzEffect.h"
#include "World/MapInfra/MapManager.h"
#include "World/MapInfra/w_MapProcess.h"
#include "../monsters/MonsterModelRecorder.h"

#include <array>
#include <memory>

void ReceiveAction(const BYTE* receiveBuffer, int size);
void MoveCharacter(CHARACTER* c, OBJECT* o);
extern int HeroIndex;
extern int AttackPlayer;
extern int AttackHand;
extern int g_iLimitAttackTime;

namespace
{
constexpr int HeroSlot = 0;
constexpr int AttackerSlot = 1;
constexpr int VictimSlot = 2;
constexpr int ObserverKey = 0x0200;
constexpr int AttackerKey = 0x0201;
constexpr int VictimKey = 0x0202;
constexpr int MissingKey = 0x0203;
constexpr int PreviousHeroHit = 4;
constexpr int PreviousVictimHit = 3;
constexpr int HeroLife = 200;
constexpr int HeroShield = 100;
constexpr int SyntheticAnimationKeys = 100;

struct AttackScope
{
    std::unique_ptr<CHARACTER[]> characters = std::make_unique<CHARACTER[]>(MAX_CHARACTERS_CLIENT + 1);
    std::unique_ptr<BMD[]> models = std::make_unique<BMD[]>(MAX_MODELS);
    std::unique_ptr<vec34_t[]> bones = std::make_unique<vec34_t[]>(MAX_BONES);
    std::unique_ptr<CHARACTER_MACHINE> machine = std::make_unique<CHARACTER_MACHINE>();
    CHARACTER_MACHINE* previousMachine = CharacterMachine;
    CHARACTER_ATTRIBUTE* previousAttribute = CharacterAttribute;
    CHARACTER* previousCharacters = CharactersClient;
    CHARACTER* previousHero = Hero;
    BMD* previousModels = Models;
    int previousHeroIndex = HeroIndex;
    int previousHeroKey = HeroKey;
    int previousAttacker = AttackPlayer;
    int previousHand = AttackHand;
    int previousAttackLimit = g_iLimitAttackTime;
    float previousFrameFactor = FPS_ANIMATION_FACTOR;
    MapProcessPtr previousMapProcess = g_MapProcess;

    AttackScope()
    {
        FPS_ANIMATION_FACTOR = 1.f;
        g_MapProcess = MapProcess::Make();
        CharactersClient = characters.get();
        Models = models.get();
        // Synthetic timelines let completion tests advance without model assets.
        Models[MODEL_PLAYER].Actions = new Action_t[MAX_PLAYER_ACTION]{};
        Models[MODEL_PLAYER].NumActions = MAX_PLAYER_ACTION;
        for (int i = 0; i < MAX_PLAYER_ACTION; ++i)
            Models[MODEL_PLAYER].Actions[i].NumAnimationKeys = SyntheticAnimationKeys;
        CharacterMachine = machine.get();
        CharacterAttribute = &machine->Character;
        CharacterAttribute->Life = HeroLife;
        CharacterAttribute->Shield = HeroShield;
        HeroIndex = HeroSlot;
        HeroKey = ObserverKey;
        Hero = &characters[HeroSlot];
        for (int slot = HeroSlot; slot <= VictimSlot; ++slot)
        {
            auto& c = characters[slot];
            c.Key = static_cast<short>(ObserverKey + slot);
            c.Object.Live = true;
            c.Object.Kind = KIND_PLAYER;
            c.Object.Type = MODEL_PLAYER;
            c.Object.CurrentAction = PLAYER_STOP_MALE;
            c.m_iDeleteTime = -128;
            c.Helper.Type = -1;
            c.Weapon[0].Type = c.Weapon[1].Type = -1;
        }
        Hero->Hit = PreviousHeroHit;
        characters[VictimSlot].Hit = PreviousVictimHit;
    }

    void SetMonsterAttacker()
    {
        auto& attacker = characters[AttackerSlot];
        attacker.Object.Kind = KIND_MONSTER;
        attacker.Object.Type = MODEL_TWIN_TAIL;
        attacker.Object.BoneTransform = bones.get();
        auto& model = Models[MODEL_TWIN_TAIL];
        model.Actions = new Action_t[MAX_PLAYER_ACTION]{};
        model.NumActions = MAX_PLAYER_ACTION;
        for (int i = 0; i < MAX_PLAYER_ACTION; ++i)
            model.Actions[i].NumAnimationKeys = SyntheticAnimationKeys;
    }

    ~AttackScope()
    {
        characters[AttackerSlot].Object.BoneTransform = nullptr;
        g_MapProcess = previousMapProcess;
        FPS_ANIMATION_FACTOR = previousFrameFactor;
        g_iLimitAttackTime = previousAttackLimit;
        AttackHand = previousHand;
        AttackPlayer = previousAttacker;
        HeroKey = previousHeroKey;
        HeroIndex = previousHeroIndex;
        CharacterAttribute = previousAttribute;
        CharacterMachine = previousMachine;
        Models = previousModels;
        Hero = previousHero;
        CharactersClient = previousCharacters;
    }

    void Receive(int targetKey, BYTE action)
    {
        // Server action 0x18: big-endian attacker at 3/4, target at 7/8.
        const std::array<BYTE, 9> packet{0xC1,
                                         9,
                                         0x18,
                                         AttackerKey >> 8,
                                         AttackerKey & 0xFF,
                                         1,
                                         action,
                                         static_cast<BYTE>(targetKey >> 8),
                                         static_cast<BYTE>(targetKey & 0xFF)};
        ReceiveAction(packet.data(), static_cast<int>(packet.size()));
    }

    void CheckAttack(int targetSlot) const
    {
        const auto& attacker = characters[AttackerSlot];
        CHECK(attacker.TargetCharacter == targetSlot);
        CHECK(attacker.AttackTime == 1);
        CHECK(attacker.Object.AnimationFrame == 0);
        CHECK(attacker.PendingAttackHitSound);
        CHECK(AttackPlayer == AttackerSlot);
        CHECK(Hero->Hit == PreviousHeroHit);
        CHECK(Hero->Object.CurrentAction == PLAYER_STOP_MALE);
        CHECK(CharacterAttribute->Life == HeroLife);
        CHECK(CharacterAttribute->Shield == HeroShield);
    }
};
} // namespace

TEST_CASE("ReceiveAction retains the packet victim for both basic attacks [combat][received_attack]")
{
    AttackScope scope;
    for (const BYTE action : {static_cast<BYTE>(AT_ATTACK1), static_cast<BYTE>(AT_ATTACK2)})
    {
        CAPTURE(action);
        scope.Receive(VictimKey, action);
        scope.CheckAttack(VictimSlot);
        CHECK(scope.characters[VictimSlot].Hit == PreviousVictimHit);
    }
}

TEST_CASE("ReceiveAction preserves direct hero targets [combat][received_attack]")
{
    AttackScope scope;
    scope.Receive(ObserverKey, AT_ATTACK2);
    scope.CheckAttack(HeroSlot);
}

TEST_CASE("ReceiveAction clears zero and unknown targets without hero fallback [combat][received_attack]")
{
    AttackScope scope;
    for (const int target : {0, MissingKey})
    {
        CAPTURE(target);
        scope.characters[AttackerSlot].TargetCharacter = VictimSlot;
        scope.Receive(target, AT_ATTACK1);
        scope.CheckAttack(-1);
    }
}

TEST_CASE("ReceiveAction clears a target that departed before receipt [combat][received_attack]")
{
    AttackScope scope;
    scope.Receive(VictimKey, AT_ATTACK1);
    scope.characters[VictimSlot].Object.Live = false;
    scope.Receive(VictimKey, AT_ATTACK2);
    scope.CheckAttack(-1);
}

TEST_CASE("ReceiveAction handles monster attackers with unknown targets [combat][received_attack]")
{
    AttackScope scope;
    scope.SetMonsterAttacker();
    for (const BYTE action : {static_cast<BYTE>(AT_ATTACK1), static_cast<BYTE>(AT_ATTACK2)})
    {
        scope.Receive(MissingKey, action);
        scope.CheckAttack(-1);
    }
}

TEST_CASE("MoveCharacter keeps received hit sounds without a target [combat][received_attack]")
{
    MonsterRecorder::LoadScope audio(WD_0LORENCIA);
    AttackScope scope;
    auto& attacker = scope.characters[AttackerSlot];
    for (const int target : {MissingKey, VictimKey})
    {
        CAPTURE(target);
        scope.Receive(target, AT_ATTACK1);
        audio.TakePlayedSounds();
        // Reach the existing delayed completion without running unrelated animation frames.
        attacker.AttackTime = static_cast<float>(g_iLimitAttackTime - 1);
        MoveCharacter(&attacker, &attacker.Object);
        const auto sounds = audio.TakePlayedSounds();

        int hits = 0;
        for (const auto& sound : sounds)
        {
            if (sound.id >= SOUND_ATTACK_MELEE_HIT1 && sound.id <= SOUND_ATTACK_MELEE_HIT4)
            {
                ++hits;
                CHECK(sound.object == &attacker.Object);
            }
        }
        CHECK(hits == 1);
        CHECK(attacker.AttackTime == 0);
        CHECK_FALSE(attacker.PendingAttackHitSound);
        MoveCharacter(&attacker, &attacker.Object);
        CHECK(audio.TakePlayedSounds().empty());
    }
}

TEST_CASE("MoveCharacter leaves untargeted local melee and magic silent [combat][received_attack]")
{
    MonsterRecorder::LoadScope audio(WD_0LORENCIA);
    AttackScope scope;
    auto& attacker = scope.characters[AttackerSlot];
    for (const WORD skill : {static_cast<WORD>(0), static_cast<WORD>(AT_SKILL_DEFENSE)})
    {
        CAPTURE(skill);
        SetPlayerAttack(&attacker);
        attacker.TargetCharacter = -1;
        attacker.Skill = skill;
        attacker.AttackTime = static_cast<float>(g_iLimitAttackTime - 1);
        audio.TakePlayedSounds();
        MoveCharacter(&attacker, &attacker.Object);
        for (const auto& sound : audio.TakePlayedSounds())
            CHECK_FALSE((sound.id >= SOUND_ATTACK_MELEE_HIT1 && sound.id <= SOUND_ATTACK_MELEE_HIT4));
    }
}

TEST_CASE("MoveCharacter keeps monster hit sounds with an unknown target [combat][received_attack]")
{
    MonsterRecorder::LoadScope audio(WD_0LORENCIA);
    AttackScope scope;
    auto& attacker = scope.characters[AttackerSlot];
    scope.SetMonsterAttacker();
    scope.Receive(MissingKey, AT_ATTACK2);
    scope.CheckAttack(-1);
    audio.TakePlayedSounds();
    attacker.AttackTime = static_cast<float>(g_iLimitAttackTime - 1);
    MoveCharacter(&attacker, &attacker.Object);
    const auto sounds = audio.TakePlayedSounds();
    REQUIRE(sounds.size() == 1);
    CHECK((sounds[0].id >= SOUND_ATTACK_MELEE_HIT1 && sounds[0].id <= SOUND_ATTACK_MELEE_HIT4));
    CHECK(sounds[0].object == &attacker.Object);
}

TEST_CASE(
    "MoveCharacter keeps no-target kill flashes off and retains monster sound exclusions [combat][received_attack]")
{
    MonsterRecorder::LoadScope audio(WD_0LORENCIA);
    AttackScope scope;
    auto& attacker = scope.characters[AttackerSlot];
    scope.Receive(MissingKey, AT_ATTACK1);
    attacker.MonsterIndex = MONSTER_MOLT;
    attacker.AttackFlag = ATTACK_DIE;
    attacker.AttackTime = static_cast<float>(g_iLimitAttackTime - 1);
    audio.TakePlayedSounds();
    for (int i = 0; i < MAX_JOINTS; ++i)
        Joints[i].Live = false;
    MoveCharacter(&attacker, &attacker.Object);
    CHECK(audio.TakePlayedSounds().empty());
    for (int i = 0; i < MAX_JOINTS; ++i)
        CHECK_FALSE(Joints[i].Live);
}

TEST_CASE("A replacement local attack clears a pending received hit sound [combat][received_attack]")
{
    MonsterRecorder::LoadScope audio(WD_0LORENCIA);
    AttackScope scope;
    auto& attacker = scope.characters[AttackerSlot];
    scope.Receive(MissingKey, AT_ATTACK1);
    SetPlayerAttack(&attacker);
    CHECK_FALSE(attacker.PendingAttackHitSound);
    attacker.AttackTime = static_cast<float>(g_iLimitAttackTime - 1);
    audio.TakePlayedSounds();
    MoveCharacter(&attacker, &attacker.Object);
    CHECK(audio.TakePlayedSounds().empty());
}

TEST_CASE("A replacement magic action clears a pending received hit sound [combat][received_attack]")
{
    MonsterRecorder::LoadScope audio(WD_0LORENCIA);
    AttackScope scope;
    auto& attacker = scope.characters[AttackerSlot];
    scope.Receive(MissingKey, AT_ATTACK1);
    attacker.Skill = AT_SKILL_DEFENSE;
    SetPlayerMagic(&attacker);
    CHECK_FALSE(attacker.PendingAttackHitSound);
    attacker.AttackTime = static_cast<float>(g_iLimitAttackTime - 1);
    audio.TakePlayedSounds();
    MoveCharacter(&attacker, &attacker.Object);
    CHECK(audio.TakePlayedSounds().empty());
}
