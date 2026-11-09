#include "stdafx.h"
#include "doctest.h"

#include "Audio/DSPlaySound.h"
#include "Engine/Object/ZzzCharacter.h"
#include "GameLogic/Events/w_CursedTemple.h"
#include "Network/Server/WSclient.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Effects/ZzzEffect.h"
#include "World/GameMaps/GMEmpireGuardian2.h"
#include "World/GameMaps/GM_Kanturu_2nd.h"
#include "World/MapInfra/MapManager.h"
#include "World/MapInfra/w_MapProcess.h"
#include "../monsters/MonsterModelRecorder.h"

#include <algorithm>
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
constexpr float InFlightAttackFrame = 3.f;

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

    void SetMonsterAttacker(int type = MODEL_TWIN_TAIL)
    {
        auto& attacker = characters[AttackerSlot];
        attacker.Object.Kind = KIND_MONSTER;
        attacker.Object.Type = static_cast<short>(type);
        attacker.Object.BoneTransform = bones.get();
        auto& model = Models[type];
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
        CHECK(attacker.ReceivedAttackTargetUnknown == (targetSlot == -1));
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
        CHECK_FALSE(attacker.ReceivedAttackTargetUnknown);
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
    CHECK_FALSE(attacker.ReceivedAttackTargetUnknown);
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
    CHECK_FALSE(attacker.ReceivedAttackTargetUnknown);
    attacker.AttackTime = static_cast<float>(g_iLimitAttackTime - 1);
    audio.TakePlayedSounds();
    MoveCharacter(&attacker, &attacker.Object);
    CHECK(audio.TakePlayedSounds().empty());
}

TEST_CASE("Dark Ghost target-clearing attacks retain meteor without a generic hit [combat][received_attack]")
{
    for (const int target : {VictimKey, MissingKey})
    {
        CAPTURE(target);
        MonsterRecorder::LoadScope audio(WD_70EMPIREGUARDIAN2);
        AttackScope scope;
        scope.SetMonsterAttacker(MODEL_DARK_GHOST);
        auto& attacker = scope.characters[AttackerSlot];
        attacker.MonsterIndex = MONSTER_DARK_GHOST;
        auto map = GMEmpireGuardian2::Make();
        for (int i = 0; i < MAX_EFFECTS; ++i)
            Effects[i].Live = false;
        scope.Receive(target, AT_ATTACK1);
        REQUIRE(attacker.Object.CurrentAction == MONSTER01_ATTACK1);
        CHECK(attacker.TargetCharacter == (target == VictimKey ? VictimSlot : -1));
        CHECK(attacker.ReceivedAttackTargetUnknown == (target == MissingKey));
        audio.TakePlayedSounds();

        CHECK(map->AttackEffectMonster(&attacker, &attacker.Object, &Models[MODEL_DARK_GHOST]));
        CHECK(attacker.TargetCharacter == -1);
        const auto effects =
            std::count_if(Effects, Effects + MAX_EFFECTS, [](const auto& e) { return e.Live && e.Type == MODEL_FIRE; });
        CHECK(effects == (target == VictimKey ? 1 : 0));
        const auto attackSounds = audio.TakePlayedSounds();
        REQUIRE(attackSounds.size() == 1);
        CHECK(attackSounds[0].id == SOUND_METEORITE01);
        CHECK(attackSounds[0].object == nullptr);
        CHECK_FALSE(attacker.ReceivedAttackTargetUnknown);

        CHECK(map->AttackEffectMonster(&attacker, &attacker.Object, &Models[MODEL_DARK_GHOST]));
        CHECK(audio.TakePlayedSounds().empty());
        CHECK(std::count_if(Effects, Effects + MAX_EFFECTS,
                            [](const auto& e) { return e.Live && e.Type == MODEL_FIRE; }) == effects);

        attacker.AttackTime = static_cast<float>(g_iLimitAttackTime - 1);
        MoveCharacter(&attacker, &attacker.Object);
        CHECK(attacker.AttackTime == 0);
        CHECK(audio.TakePlayedSounds().empty());
        for (int i = 0; i < MAX_EFFECTS; ++i)
            Effects[i].Live = false;
    }
}

TEST_CASE("Received basic attacks replace interrupted skill feedback [combat][received_attack]")
{
    for (const int target : {VictimKey, MissingKey})
    {
        CAPTURE(target);
        MonsterRecorder::LoadScope audio(WD_0LORENCIA);
        AttackScope scope;
        auto& attacker = scope.characters[AttackerSlot];
        attacker.Skill = AT_SKILL_ICE;
        attacker.AttackTime = 1.f;
        for (int i = 0; i < MAX_EFFECTS; ++i)
            Effects[i].Live = false;
        scope.Receive(target, AT_ATTACK2);
        CHECK(attacker.Skill == 0);
        audio.TakePlayedSounds();
        attacker.AttackTime = static_cast<float>(g_iLimitAttackTime - 1);
        MoveCharacter(&attacker, &attacker.Object);
        const auto sounds = audio.TakePlayedSounds();
        const auto hits =
            std::count_if(sounds.begin(), sounds.end(), [](const auto& sound)
                          { return sound.id >= SOUND_ATTACK_MELEE_HIT1 && sound.id <= SOUND_ATTACK_MELEE_HIT4; });
        CHECK(hits == 1);
        for (const auto& sound : sounds)
        {
            CHECK(sound.id != SOUND_ICE);
            CHECK(sound.object == &attacker.Object);
        }
        CHECK(std::none_of(Effects, Effects + MAX_EFFECTS,
                           [](const auto& e) { return e.Live && (e.Type == MODEL_ICE || e.Type == MODEL_ICE_SMALL); }));
        for (int i = 0; i < MAX_EFFECTS; ++i)
            Effects[i].Live = false;
    }
}

TEST_CASE("Received basic attacks play a monster's stored player-skill attack [combat][received_attack]")
{
    // Fire Golem: "skills": {"50": "attack1"}, "otherwise": "attack2" (docs/monster-data.md).
    for (const int skill : {static_cast<int>(AT_SKILL_BOSS), 0})
    {
        CAPTURE(skill);
        MonsterRecorder::LoadScope audio(WD_31HUNTING_GROUND);
        AttackScope scope;
        scope.SetMonsterAttacker(MODEL_FIRE_GOLEM);
        auto& attacker = scope.characters[AttackerSlot];
        attacker.MonsterIndex = MONSTER_FIRE_GOLEM;
        attacker.Skill = static_cast<WORD>(skill);
        attacker.AttackTime = 1.f;
        scope.Receive(VictimKey, AT_ATTACK1);
        CHECK(attacker.Object.CurrentAction == (skill != 0 ? MONSTER01_ATTACK1 : MONSTER01_ATTACK2));
        CHECK(attacker.Skill == 0);
        CHECK(attacker.TargetCharacter == VictimSlot);
    }
}

TEST_CASE("ReceiveAction keeps zero target absent beside a live key-zero character [combat][received_attack]")
{
    for (const BYTE action : {static_cast<BYTE>(AT_ATTACK1), static_cast<BYTE>(AT_ATTACK2)})
    {
        CAPTURE(action);
        MonsterRecorder::LoadScope audio(WD_0LORENCIA);
        AttackScope scope;
        auto& decorative = scope.characters[VictimSlot];
        decorative.Key = 0;
        REQUIRE(decorative.Object.Live);
        auto& attacker = scope.characters[AttackerSlot];
        for (int i = 0; i < MAX_PARTICLES; ++i)
            Particles[i].Live = false;
        for (int i = 0; i < MAX_JOINTS; ++i)
            Joints[i].Live = false;

        scope.Receive(0, action);
        scope.CheckAttack(-1);
        audio.TakePlayedSounds();
        attacker.AttackFlag = ATTACK_DIE;
        attacker.AttackTime = static_cast<float>(g_iLimitAttackTime - 1);
        MoveCharacter(&attacker, &attacker.Object);
        const auto sounds = audio.TakePlayedSounds();
        REQUIRE(sounds.size() == 1);
        CHECK((sounds[0].id >= SOUND_ATTACK_MELEE_HIT1 && sounds[0].id <= SOUND_ATTACK_MELEE_HIT4));
        CHECK(sounds[0].object == &attacker.Object);
        CHECK(decorative.Key == 0);
        CHECK(decorative.Hit == PreviousVictimHit);
        CHECK(Hero->Hit == PreviousHeroHit);
        CHECK(std::none_of(Particles, Particles + MAX_PARTICLES,
                           [](const auto& p) { return p.Live && p.Type == BITMAP_BLOOD + 1; }));
        CHECK(std::none_of(Joints, Joints + MAX_JOINTS,
                           [](const auto& j) { return j.Live && j.Type == BITMAP_JOINT_ENERGY; }));
        for (int i = 0; i < MAX_PARTICLES; ++i)
            Particles[i].Live = false;
        for (int i = 0; i < MAX_JOINTS; ++i)
            Joints[i].Live = false;
    }
}

TEST_CASE("CreateCharacterPointer clears received attack feedback on slot reuse [combat][received_attack]")
{
    MonsterRecorder::LoadScope audio(WD_0LORENCIA);
    AttackScope scope;
    auto& attacker = scope.characters[AttackerSlot];
    scope.Receive(MissingKey, AT_ATTACK1);
    REQUIRE(attacker.ReceivedAttackTargetUnknown);
    attacker.Skill = AT_SKILL_ICE;
    attacker.AttackTime = static_cast<float>(g_iLimitAttackTime - 1);
    attacker.LastAttackEffectTime = 1;
    attacker.TargetCharacter = VictimSlot;
    attacker.AttackFlag = ATTACK_DIE;

    CreateCharacterPointer(&attacker, MODEL_PLAYER, 0, 0, 0.f);
    const std::unique_ptr<vec34_t[]> reusedBones(attacker.Object.BoneTransform);
    CHECK(attacker.Skill == 0);
    CHECK(attacker.AttackTime == 0);
    CHECK(attacker.LastAttackEffectTime == -1);
    CHECK(attacker.TargetCharacter == -1);
    CHECK(attacker.AttackFlag == ATTACK_FAIL);
    CHECK_FALSE(attacker.ReceivedAttackTargetUnknown);

    // A timer-only animation start must not inherit the previous occupant's receipt.
    attacker.Object.CurrentAction = PLAYER_ATTACK_FIST;
    attacker.AttackTime = static_cast<float>(g_iLimitAttackTime - 1);
    audio.TakePlayedSounds();
    MoveCharacter(&attacker, &attacker.Object);
    CHECK(attacker.AttackTime == 0);
    CHECK(audio.TakePlayedSounds().empty());
}

TEST_CASE("Received cannon attacks restart consumed effect steps [combat][received_attack]")
{
    MonsterRecorder::LoadScope audio(WD_38KANTURU_2ND);
    AttackScope scope;
    scope.SetMonsterAttacker(MODEL_TRAP_CANON);
    auto& attacker = scope.characters[AttackerSlot];
    auto* o = &attacker.Object;
    auto* b = &Models[MODEL_TRAP_CANON];
    auto* victimObject = &scope.characters[VictimSlot].Object;
    for (int i = 0; i < MAX_JOINTS; ++i)
        Joints[i].Live = false;
    const auto countVictimJoints = [victimObject]
    {
        return std::count_if(
            Joints, Joints + MAX_JOINTS, [victimObject](const auto& joint)
            { return joint.Live && joint.Type == BITMAP_JOINT_ENERGY && joint.Target == victimObject; });
    };

    attacker.AttackFlag = ATTACK_DIE;
    scope.Receive(MissingKey, AT_ATTACK1);
    scope.CheckAttack(-1);
    CHECK(attacker.AttackFlag == ATTACK_DIE);
    CHECK(M38Kanturu2nd::AttackEffect_Kanturu2nd_Monster(&attacker, o, b));
    CHECK(attacker.LastAttackEffectTime == 1);
    CHECK(std::none_of(Joints, Joints + MAX_JOINTS, [](const auto& joint) { return joint.Live; }));

    scope.Receive(VictimKey, AT_ATTACK2);
    scope.CheckAttack(VictimSlot);
    CHECK(attacker.AttackFlag == ATTACK_DIE);
    CHECK(attacker.LastAttackEffectTime == -1);
    CHECK(M38Kanturu2nd::AttackEffect_Kanturu2nd_Monster(&attacker, o, b));
    CHECK(attacker.LastAttackEffectTime == 1);
    CHECK(countVictimJoints() == 1);
    CHECK(M38Kanturu2nd::AttackEffect_Kanturu2nd_Monster(&attacker, o, b));
    CHECK(countVictimJoints() == 1);
    for (int i = 0; i < MAX_JOINTS; ++i)
        Joints[i].Live = false;
}

TEST_CASE("Cursed Temple poison signatures keep target-independent audio cadence [combat][received_attack]")
{
    constexpr int SmokePerCall = 5;
    for (const int target : {VictimKey, MissingKey})
    {
        CAPTURE(target);
        MonsterRecorder::LoadScope audio(WD_45CURSEDTEMPLE_LV1);
        AttackScope scope;
        scope.SetMonsterAttacker(MODEL_ILLUSION_SORCERER_SPIRIT_POISON);
        auto& attacker = scope.characters[AttackerSlot];
        attacker.MonsterIndex = MONSTER_ILLUSION_SORCERER_SPIRIT1_POISON;
        auto* o = &attacker.Object;
        auto* b = &Models[o->Type];
        for (int i = 0; i < MAX_PARTICLES; ++i)
            Particles[i].Live = false;
        scope.Receive(target, AT_ATTACK1);
        REQUIRE(o->CurrentAction == MONSTER01_ATTACK1);
        scope.CheckAttack(target == VictimKey ? VictimSlot : -1);
        audio.TakePlayedSounds();

        int expectedSmoke = 0;
        for (const float frame : {0.f, InFlightAttackFrame})
        {
            o->AnimationFrame = frame;
            CHECK(g_CursedTemple->AttackEffectMonster(&attacker, o, b));
            const auto sounds = audio.TakePlayedSounds();
            REQUIRE(sounds.size() == 1);
            CHECK(sounds[0].id == SOUND_HEART);
            CHECK(sounds[0].object == nullptr);
            if (target == VictimKey)
                expectedSmoke += SmokePerCall;
            CHECK(std::count_if(Particles, Particles + MAX_PARTICLES,
                                [](const auto& p) { return p.Live && p.Type == BITMAP_SMOKE; }) == expectedSmoke);
            CHECK(attacker.ReceivedAttackTargetUnknown == (target == MissingKey));
        }

        attacker.AttackTime = static_cast<float>(g_iLimitAttackTime - 1);
        MoveCharacter(&attacker, o);
        const auto completion = audio.TakePlayedSounds();
        CHECK(std::count_if(completion.begin(), completion.end(),
                            [](const auto& sound) { return sound.id == SOUND_HEART && sound.object == nullptr; }) == 1);
        CHECK(std::count_if(completion.begin(), completion.end(),
                            [o](const auto& sound)
                            {
                                return sound.id >= SOUND_ATTACK_MELEE_HIT1 && sound.id <= SOUND_ATTACK_MELEE_HIT4 &&
                                       sound.object == o;
                            }) == 1);
        CHECK(attacker.AttackTime == 0);
        for (int i = 0; i < MAX_PARTICLES; ++i)
            Particles[i].Live = false;
    }
}

TEST_CASE("ReceiveAction preserves the prior walking pose when starting an attack [combat][received_attack]")
{
    constexpr float WalkingAnimationFrame = 5.3f;
    MonsterRecorder::LoadScope audio(WD_70EMPIREGUARDIAN2);
    for (const int target : {VictimKey, MissingKey})
    {
        CAPTURE(target);
        AttackScope scope;
        scope.SetMonsterAttacker(MODEL_DARK_GHOST);
        auto& attacker = scope.characters[AttackerSlot];
        attacker.MonsterIndex = MONSTER_DARK_GHOST;
        attacker.Object.CurrentAction = MONSTER01_WALK;
        attacker.Object.AnimationFrame = WalkingAnimationFrame;
        scope.Receive(target, AT_ATTACK1);
        REQUIRE(attacker.Object.CurrentAction == MONSTER01_ATTACK1);
        CHECK(attacker.Object.PriorAction == MONSTER01_WALK);
        CHECK(attacker.Object.PriorAnimationFrame == WalkingAnimationFrame);
        scope.CheckAttack(target == VictimKey ? VictimSlot : -1);
    }
}

TEST_CASE("Local targetless poison attacks retain signature silence [combat][received_attack]")
{
    MonsterRecorder::LoadScope audio(WD_45CURSEDTEMPLE_LV1);
    AttackScope scope;
    scope.SetMonsterAttacker(MODEL_ILLUSION_SORCERER_SPIRIT_POISON);
    auto& attacker = scope.characters[AttackerSlot];
    attacker.MonsterIndex = MONSTER_ILLUSION_SORCERER_SPIRIT1_POISON;
    attacker.TargetCharacter = -1;
    SetPlayerAttack(&attacker);
    REQUIRE(attacker.Object.CurrentAction == MONSTER01_ATTACK1);
    REQUIRE_FALSE(attacker.ReceivedAttackTargetUnknown);
    audio.TakePlayedSounds();
    for (int i = 0; i < MAX_PARTICLES; ++i)
        Particles[i].Live = false;

    for (const float frame : {0.f, InFlightAttackFrame})
    {
        attacker.Object.AnimationFrame = frame;
        CHECK(g_CursedTemple->AttackEffectMonster(&attacker, &attacker.Object, &Models[attacker.Object.Type]));
        CHECK(audio.TakePlayedSounds().empty());
        CHECK(std::none_of(Particles, Particles + MAX_PARTICLES,
                           [](const auto& p) { return p.Live && p.Type == BITMAP_SMOKE; }));
        CHECK_FALSE(attacker.ReceivedAttackTargetUnknown);
    }
}
