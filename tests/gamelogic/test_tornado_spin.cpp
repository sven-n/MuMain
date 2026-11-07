#include "stdafx.h"
#include "doctest.h"

#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Render/Effects/ZzzEffect.h"

#include <memory>

namespace
{
constexpr float TornadoRange = 150.f;
constexpr short TargetedPlayerKey = 7;

// The client's characters, the hero and its skills for the scope, put back afterwards.
class TornadoScope
{
public:
    TornadoScope()
        : m_characters(std::make_unique<CHARACTER[]>(MAX_CHARACTERS_CLIENT)), m_previousCharacters(CharactersClient),
          m_previousHero(Hero), m_previousAttribute(CharacterAttribute)
    {
        CharactersClient = m_characters.get();
        Hero = &m_hero;
        CharacterAttribute = &m_attribute;
    }

    ~TornadoScope()
    {
        CharactersClient = m_previousCharacters;
        Hero = m_previousHero;
        CharacterAttribute = m_previousAttribute;
    }

    TornadoScope(const TornadoScope&) = delete;
    TornadoScope& operator=(const TornadoScope&) = delete;

    CHARACTER_ATTRIBUTE& Attribute()
    {
        return m_attribute;
    }

    // A live, visible character of that kind at (x, 0).
    CHARACTER& Place(int index, int kind, float x)
    {
        CHARACTER& c = CharactersClient[index];
        c.StormTime = 0.f;
        c.Dead = 0;
        c.Key = 0;
        c.MonsterIndex = static_cast<decltype(c.MonsterIndex)>(0);
        c.Object.Live = true;
        c.Object.Visible = true;
        c.Object.Kind = kind;
        c.Object.Position[0] = x;
        c.Object.Position[1] = 0.f;
        return c;
    }

private:
    std::unique_ptr<CHARACTER[]> m_characters;
    CHARACTER m_hero{};
    CHARACTER_ATTRIBUTE m_attribute{};
    CHARACTER* m_previousCharacters;
    CHARACTER* m_previousHero;
    CHARACTER_ATTRIBUTE* m_previousAttribute;
};
} // namespace

TEST_CASE("A tornado spins the monsters and the targeted player in its range, as the original client did [render]")
{
    TornadoScope scope;
    scope.Attribute().Skill[0] = AT_SKILL_STORM;
    CHARACTER& inRange = scope.Place(0, KIND_MONSTER, 100.f);
    CHARACTER& edge = scope.Place(1, KIND_MONSTER, TornadoRange);
    CHARACTER& outOfRange = scope.Place(2, KIND_MONSTER, TornadoRange + 1.f);
    CHARACTER& npc = scope.Place(3, KIND_NPC, 50.f);
    CHARACTER& target = scope.Place(4, KIND_PLAYER, 50.f);
    target.Key = TargetedPlayerKey;
    CHARACTER& bystander = scope.Place(5, KIND_PLAYER, 50.f);
    bystander.Key = TargetedPlayerKey + 1;
    // Immobile monsters keep their orientation.
    CHARACTER& gate = scope.Place(6, KIND_MONSTER, 50.f);
    gate.MonsterIndex = MONSTER_CASTLE_GATE;
    CHARACTER& dead = scope.Place(7, KIND_MONSTER, 50.f);
    dead.Dead = 1;
    CHARACTER& hidden = scope.Place(8, KIND_MONSTER, 50.f);
    hidden.Object.Visible = false;

    vec3_t center = {0.f, 0.f, 0.f};
    // It never sends a request: the server calculates the hits.
    CHECK_FALSE(AttackCharacterRange(0, center, TornadoRange, 0, TargetedPlayerKey));

    CHECK(inRange.StormTime > 0.f);
    CHECK(edge.StormTime > 0.f);
    CHECK(target.StormTime > 0.f);
    CHECK(outOfRange.StormTime == 0.f);
    CHECK(npc.StormTime == 0.f);
    CHECK(bystander.StormTime == 0.f);
    CHECK(gate.StormTime == 0.f);
    CHECK(dead.StormTime == 0.f);
    CHECK(hidden.StormTime == 0.f);
}

TEST_CASE("Only Twister and Evil Spirit spin what they reach [render]")
{
    TornadoScope scope;
    CHARACTER& monster = scope.Place(0, KIND_MONSTER, 50.f);
    vec3_t center = {0.f, 0.f, 0.f};

    scope.Attribute().Skill[0] = AT_SKILL_EVIL_SPIRIT;
    AttackCharacterRange(0, center, TornadoRange, 0);
    CHECK(monster.StormTime > 0.f);

    monster.StormTime = 0.f;
    scope.Attribute().Skill[1] = AT_SKILL_TELEPORT;
    AttackCharacterRange(1, center, TornadoRange, 0);
    CHECK(monster.StormTime == 0.f);
    // A skill index outside the hero's skills is nothing.
    AttackCharacterRange(-1, center, TornadoRange, 0);
    AttackCharacterRange(MAX_SKILLS, center, TornadoRange, 0);
    CHECK(monster.StormTime == 0.f);
}
