#include "stdafx.h"
#include "doctest.h"

#include "Engine/Object/ZzzCharacter.h"
#include "Network/Server/WSclient.h"
#include "Render/Models/ZzzBMD.h"

#include <array>
#include <memory>

void ReceiveAction(const BYTE* receiveBuffer, int size);
extern int HeroIndex;
extern int AttackPlayer;
extern int AttackHand;

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

struct AttackScope
{
    std::unique_ptr<CHARACTER[]> characters = std::make_unique<CHARACTER[]>(MAX_CHARACTERS_CLIENT);
    std::unique_ptr<BMD[]> models = std::make_unique<BMD[]>(MODEL_PLAYER + 1);
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

    AttackScope()
    {
        CharactersClient = characters.get();
        Models = models.get();
        // Attack speed writes this table; receiving an action needs no animation keys or assets.
        Models[MODEL_PLAYER].Actions = new Action_t[MAX_PLAYER_ACTION]{};
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
            c.Helper.Type = -1;
            c.Weapon[0].Type = c.Weapon[1].Type = -1;
        }
        Hero->Hit = PreviousHeroHit;
        characters[VictimSlot].Hit = PreviousVictimHit;
    }

    ~AttackScope()
    {
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
