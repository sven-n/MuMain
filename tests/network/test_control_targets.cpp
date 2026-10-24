// doctest unit tests for the control socket's `target` field.
//
// Run: ctest --test-dir <build directory> --build-config Release -R "Control targets"

#include "stdafx.h"

#include "doctest.h"

#include "App/Control/ControlProtocol.h"
#include "App/Control/ControlTargets.h"
#include "Engine/Object/ZzzCharacter.h"

#include <cstddef>
#include <map>
#include <string>
#include <vector>

using App::Control::Request;
using App::Control::Targets::ReadMonsterNumber;
using App::Control::Targets::ResolveTarget;
using App::Control::Targets::TargetLookup;

namespace
{
constexpr int HeroKey = 1;
constexpr int HeroX = 100;
constexpr int HeroY = 100;

// The fields of the request's `target` object, as the commands read them.
std::map<std::string, std::string> TargetFields(const char* line)
{
    const Request request = Request::Parse(line);
    std::map<std::string, std::string> fields;
    REQUIRE(request.GetStringMap("target", fields));
    return fields;
}

// The lookup of an attack on {"monster": <number>}, with the key it found.
TargetLookup ResolveMonster(int number, int& key)
{
    const std::string line = R"({"cmd":"attack","target":{"monster":)" + std::to_string(number) + "}}";
    return ResolveTarget(Request::Parse(line), key);
}

// A character table of the test's own in place of the game's, with the hero
// in its first slot.
class CharacterTableScope
{
public:
    CharacterTableScope()
        : m_characters(MAX_CHARACTERS_CLIENT), m_previousCharacters(CharactersClient), m_previousHero(Hero)
    {
        CharactersClient = m_characters.data();
        CHARACTER& hero = Place(HeroKey, HeroX, HeroY);
        hero.Object.Kind = KIND_PLAYER;
        Hero = &hero;
    }

    ~CharacterTableScope()
    {
        CharactersClient = m_previousCharacters;
        Hero = m_previousHero;
    }

    CharacterTableScope(const CharacterTableScope&) = delete;
    CharacterTableScope& operator=(const CharacterTableScope&) = delete;

    // A living character of this object kind and monster number, `x` and `y`
    // tiles away from the hero.
    CHARACTER& Add(int key, int kind, EMonsterType number, int x, int y)
    {
        CHARACTER& character = Place(key, HeroX + x, HeroY + y);
        character.Object.Kind = kind;
        character.MonsterIndex = number;
        return character;
    }

private:
    CHARACTER& Place(int key, int x, int y)
    {
        CHARACTER& character = m_characters[m_count++];
        character.Key = static_cast<SHORT>(key);
        character.Object.Live = true;
        character.PositionX = x;
        character.PositionY = y;
        return character;
    }

    std::vector<CHARACTER> m_characters;
    CHARACTER* m_previousCharacters;
    CHARACTER* m_previousHero;
    std::size_t m_count = 0;
};
} // namespace

TEST_CASE("Control targets read a monster number [network][control-targets]")
{
    int number = -1;

    CHECK(ReadMonsterNumber(TargetFields(R"({"cmd":"attack","target":{"monster":251}})"), number));
    CHECK(number == 251);
    CHECK(ReadMonsterNumber(TargetFields(R"({"cmd":"attack","target":{"monster":0}})"), number));
    CHECK(number == 0);
    CHECK(ReadMonsterNumber(TargetFields(R"({"cmd":"attack","target":{"monster":1023}})"), number));
    CHECK(number == 1023);
    // Python's json writes a float like this; the id form (GetInt) takes it too.
    CHECK(ReadMonsterNumber(TargetFields(R"({"cmd":"attack","target":{"monster":251.0}})"), number));
    CHECK(number == 251);
    // The fields keep a JSON string as it is, so it reads like the number.
    CHECK(ReadMonsterNumber(TargetFields(R"({"cmd":"attack","target":{"monster":"235"}})"), number));
    CHECK(number == 235);
}

TEST_CASE("Control targets refuse anything else as a monster number [network][control-targets]")
{
    int number = 7;
    for (const char* line : {
             R"({"cmd":"attack","target":{"monster":1024}})",
             R"({"cmd":"attack","target":{"monster":-1}})",
             R"({"cmd":"attack","target":{"monster":251.5}})",
             R"({"cmd":"attack","target":{"monster":"251."}})",
             R"({"cmd":"attack","target":{"monster":"251x"}})",
             R"({"cmd":"attack","target":{"monster":"Hanzo"}})",
             R"({"cmd":"attack","target":{"monster":null}})",
             R"({"cmd":"attack","target":{"monster":251,"name":"Hanzo"}})",
             R"({"cmd":"attack","target":{"number":251}})",
             R"({"cmd":"attack","target":{}})",
         })
    {
        CAPTURE(line);
        CHECK_FALSE(ReadMonsterNumber(TargetFields(line), number));
        CHECK(number == 7);
    }
}

TEST_CASE("Control targets answer a malformed monster target as malformed [network][control-targets]")
{
    const Request request = Request::Parse(R"({"cmd":"attack","target":{"monster":5000}})");
    int key = 0;

    CHECK(App::Control::Targets::ResolveTarget(request, key) == App::Control::Targets::TargetLookup::Malformed);
    CHECK(key == -1);
}

TEST_CASE("Control targets find a monster and an NPC by their number [network][control-targets]")
{
    CharacterTableScope characters;
    characters.Add(101, KIND_MONSTER, MONSTER_SPIDER, 3, 0);
    characters.Add(102, KIND_NPC, MONSTER_HANZO_THE_BLACKSMITH, 0, 2);
    int key = 0;

    CHECK(ResolveMonster(MONSTER_SPIDER, key) == TargetLookup::Found);
    CHECK(key == 101);
    CHECK(ResolveMonster(MONSTER_HANZO_THE_BLACKSMITH, key) == TargetLookup::Found);
    CHECK(key == 102);
    CHECK(ResolveMonster(MONSTER_BUDGE_DRAGON, key) == TargetLookup::NotInView);
    CHECK(key == -1);
}

TEST_CASE("Control targets do not find traps, other objects or players by their number [network][control-targets]")
{
    CharacterTableScope characters;
    characters.Add(101, KIND_TRAP, MONSTER_LANCE_TRAP, 1, 0);
    characters.Add(102, KIND_TMP, MONSTER_CHERRY_BLOSSOM_TREE, 2, 0);
    // A summon is a player with the monster number of what it looks like.
    characters.Add(103, KIND_PLAYER, MONSTER_GOBLIN, 3, 0);

    for (const EMonsterType number : {MONSTER_LANCE_TRAP, MONSTER_CHERRY_BLOSSOM_TREE, MONSTER_GOBLIN})
    {
        CAPTURE(number);
        int key = 0;
        CHECK(ResolveMonster(number, key) == TargetLookup::NotInView);
        CHECK(key == -1);
    }
}

TEST_CASE("Control targets do not find a dead monster [network][control-targets]")
{
    CharacterTableScope characters;
    characters.Add(101, KIND_MONSTER, MONSTER_SPIDER, 1, 0).Dead = 1.0f;
    int key = 0;

    CHECK(ResolveMonster(MONSTER_SPIDER, key) == TargetLookup::NotInView);
    CHECK(key == -1);

    // A living one further away is the target instead.
    characters.Add(102, KIND_MONSTER, MONSTER_SPIDER, 5, 0);
    CHECK(ResolveMonster(MONSTER_SPIDER, key) == TargetLookup::Found);
    CHECK(key == 102);
}

TEST_CASE("Control targets find the nearest monster of a number in tiles [network][control-targets]")
{
    CharacterTableScope characters;
    characters.Add(101, KIND_MONSTER, MONSTER_SPIDER, 8, 0);
    // Six tiles away on the diagonal, although further in a straight line.
    characters.Add(102, KIND_MONSTER, MONSTER_SPIDER, -6, 6);
    characters.Add(103, KIND_MONSTER, MONSTER_SPIDER, 0, -7);
    int key = 0;

    CHECK(ResolveMonster(MONSTER_SPIDER, key) == TargetLookup::Found);
    CHECK(key == 102);
}

TEST_CASE("Control targets take the first monster in the table on a tie [network][control-targets]")
{
    CharacterTableScope characters;
    characters.Add(101, KIND_MONSTER, MONSTER_SPIDER, 9, 0);
    characters.Add(102, KIND_MONSTER, MONSTER_SPIDER, -4, 0);
    characters.Add(103, KIND_MONSTER, MONSTER_SPIDER, 0, 4);
    int key = 0;

    CHECK(ResolveMonster(MONSTER_SPIDER, key) == TargetLookup::Found);
    CHECK(key == 102);
}
