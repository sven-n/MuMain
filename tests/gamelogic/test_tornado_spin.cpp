#include "stdafx.h"
#include "doctest.h"

#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Render/Effects/ZzzEffect.h"

#include <memory>

namespace
{
constexpr float TornadoRange = 150.f;
constexpr int HeroIndex = 0;

// The client's characters (the hero among them, as in game) and the hero's skills for the scope, put back afterwards.
class TornadoScope
{
public:
    TornadoScope()
        : m_characters(std::make_unique<CHARACTER[]>(MAX_CHARACTERS_CLIENT)), m_previousCharacters(CharactersClient),
          m_previousHero(Hero), m_previousAttribute(CharacterAttribute)
    {
        CharactersClient = m_characters.get();
        Hero = &CharactersClient[HeroIndex];
        CharacterAttribute = &m_attribute;
        Place(HeroIndex, KIND_PLAYER, 0.f);
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
        c.Key = static_cast<decltype(c.Key)>(index);
        c.MonsterIndex = static_cast<decltype(c.MonsterIndex)>(0);
        c.NotRotateOnMagicHit = false;
        c.Object.Live = true;
        c.Object.Visible = true;
        c.Object.Kind = kind;
        c.Object.Position[0] = x;
        c.Object.Position[1] = 0.f;
        return c;
    }

private:
    std::unique_ptr<CHARACTER[]> m_characters;
    CHARACTER_ATTRIBUTE m_attribute{};
    CHARACTER* m_previousCharacters;
    CHARACTER* m_previousHero;
    CHARACTER_ATTRIBUTE* m_previousAttribute;
};
} // namespace

TEST_CASE("A tornado spins every monster in its range and no one else [render]")
{
    TornadoScope scope;
    CHARACTER& inRange = scope.Place(1, KIND_MONSTER, 100.f);
    CHARACTER& edge = scope.Place(2, KIND_MONSTER, TornadoRange);
    CHARACTER& outOfRange = scope.Place(3, KIND_MONSTER, TornadoRange + 1.f);
    CHARACTER& npc = scope.Place(4, KIND_NPC, 50.f);
    // A player is never spun, not even the one under the cursor when the hero cast it (the cast's PKKey).
    CHARACTER& player = scope.Place(5, KIND_PLAYER, 50.f);
    Hero->Object.PKKey = player.Key;
    CHARACTER& dead = scope.Place(6, KIND_MONSTER, 50.f);
    dead.Dead = 1;
    CHARACTER& hidden = scope.Place(7, KIND_MONSTER, 50.f);
    hidden.Object.Visible = false;

    vec3_t center = {0.f, 0.f, 0.f};
    SpinMonstersInTornado(center, TornadoRange);

    CHECK(inRange.StormTime > 0.f);
    CHECK(edge.StormTime > 0.f);
    CHECK(outOfRange.StormTime == 0.f);
    CHECK(npc.StormTime == 0.f);
    CHECK(player.StormTime == 0.f);
    CHECK(Hero->StormTime == 0.f);
    CHECK(dead.StormTime == 0.f);
    CHECK(hidden.StormTime == 0.f);
}

TEST_CASE("Gates, statues and other immobile monsters don't spin in a tornado [render]")
{
    TornadoScope scope;
    // Their setup marks most of them, e.g. the Castle Siege gate and the Blood Castle gate.
    CHARACTER& marked = scope.Place(1, KIND_MONSTER, 50.f);
    marked.NotRotateOnMagicHit = true;
    // The rest are known by number, e.g. Selupan.
    CHARACTER& selupan = scope.Place(2, KIND_MONSTER, 50.f);
    selupan.MonsterIndex = MONSTER_SELUPAN;

    vec3_t center = {0.f, 0.f, 0.f};
    SpinMonstersInTornado(center, TornadoRange);

    CHECK(marked.StormTime == 0.f);
    CHECK(selupan.StormTime == 0.f);
}

TEST_CASE("Other area skills spin nothing, even with a tornado skill selected [render]")
{
    TornadoScope scope;
    CHARACTER& monster = scope.Place(1, KIND_MONSTER, 50.f);
    // A Fire Scream passes the hero's current skill, here Twister, and its own position.
    scope.Attribute().Skill[0] = AT_SKILL_STORM;
    Hero->CurrentSkill = 0;

    vec3_t center = {0.f, 0.f, 0.f};
    // It never sends a request either: the server calculates the hits.
    CHECK_FALSE(AttackCharacterRange(Hero->CurrentSkill, center, 100.f, 0, -1));
    CHECK(monster.StormTime == 0.f);
}
