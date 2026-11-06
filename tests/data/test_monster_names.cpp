#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"
#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Engine/Object/ZzzCharacter.h"
#include "GameLogic/Monsters/MonsterNames.h"
#include "I18N/All.h"

#include <array>
#include <filesystem>
#include <string>
#include <vector>

using Data::Monsters::MonsterDefinition;
using Data::Monsters::MonsterKind;

namespace
{
MonsterDefinition Monster(int number, const char* english, const char* portuguese, MonsterKind kind)
{
    MonsterDefinition monster;
    monster.number = number;
    monster.names.Set("en", english);
    if (portuguese != nullptr)
    {
        monster.names.Set("pt", portuguese);
    }
    monster.kind = kind;
    return monster;
}

// A character table of the test's own, in place of the game's.
class CharacterTableScope
{
public:
    CharacterTableScope() : m_characters(MAX_CHARACTERS_CLIENT), m_previous(CharactersClient)
    {
        CharactersClient = m_characters.data();
    }

    ~CharacterTableScope()
    {
        CharactersClient = m_previous;
    }

    CHARACTER& Add(int number, int kind, const wchar_t* name)
    {
        CHARACTER& character = m_characters[m_count++];
        character.Object.Live = true;
        character.Object.Kind = kind;
        character.MonsterIndex = static_cast<EMonsterType>(number);
        wcscpy_s(character.ID, std::size(character.ID), name);
        return character;
    }

private:
    std::vector<CHARACTER> m_characters;
    CHARACTER* m_previous;
    std::size_t m_count = 0;
};

// The numbers the handler last told about a name change.
std::vector<int> g_toldNumbers;

void RememberNamesChanged(int number)
{
    g_toldNumbers.push_back(number);
}
} // namespace

TEST_CASE("Monster names: a refresh renames living monsters and NPCs, not names with a player in them [data][monsters]")
{
    const std::array monsters = {Monster(2, "Budge Dragon", "Dragão Lento", MonsterKind::Monster),
                                 Monster(251, "Hanzo the Blacksmith", "Hanzo o Ferreiro", MonsterKind::Npc)};
    g_MonsterDatabase.Build(monsters, "en");
    CharacterTableScope table;
    CHARACTER& dragon = table.Add(2, KIND_MONSTER, L"Budge Dragon");
    CHARACTER& hanzo = table.Add(251, KIND_NPC, L"Hanzo the Blacksmith");
    CHARACTER& summon = table.Add(2, KIND_PLAYER, L"Budge Dragon of Maxi");
    CHARACTER& transformed = table.Add(2, KIND_NPC, L"Maxi");
    transformed.Change = true;
    CHARACTER& player = table.Add(MONSTER_UNDEFINED, KIND_PLAYER, L"Maxi");
    CHARACTER& gone = table.Add(2, KIND_MONSTER, L"Budge Dragon");
    gone.Object.Live = false;
    CHARACTER& unknown = table.Add(76, KIND_MONSTER, L"");

    g_MonsterDatabase.SetDisplayLocale("pt");

    SUBCASE("one number")
    {
        GameLogic::Monsters::Names::RefreshShownNames(251);

        CHECK(std::wstring(hanzo.ID) == L"Hanzo o Ferreiro");
        CHECK(std::wstring(dragon.ID) == L"Budge Dragon");
    }
    SUBCASE("all numbers")
    {
        GameLogic::Monsters::Names::RefreshShownNames();

        CHECK(std::wstring(dragon.ID) == L"Dragão Lento");
        CHECK(std::wstring(hanzo.ID) == L"Hanzo o Ferreiro");
        CHECK(std::wstring(summon.ID) == L"Budge Dragon of Maxi");
        CHECK(std::wstring(transformed.ID) == L"Maxi");
        CHECK(std::wstring(player.ID) == L"Maxi");
        CHECK(std::wstring(gone.ID) == L"Budge Dragon");
        CHECK(std::wstring(unknown.ID).empty());
    }
}

TEST_CASE("Monster names: without a character table a refresh does nothing [data][monsters]")
{
    CHARACTER* previous = CharactersClient;
    CharactersClient = nullptr;

    GameLogic::Monsters::Names::RefreshShownNames();

    CharactersClient = previous;
}

TEST_CASE("Monster names: the loaded names follow the UI language [data][monsters]")
{
    ClientDataScope scope(std::filesystem::path(MU_TEST_DATA_DIR).parent_path());
    std::string error;
    I18N::SetLocale("pt");
    REQUIRE(g_MonsterDataHandler.Load(error));
    CHECK(std::wstring(g_MonsterDatabase.FindDisplayName(2)) == L"Dragão Lento");
    g_toldNumbers.clear();
    g_MonsterDataHandler.SetNamesChangedCallback(&RememberNamesChanged);

    I18N::SetLocale("en");

    CHECK(std::wstring(g_MonsterDatabase.FindDisplayName(2)) == L"Budge Dragon");
    CHECK(g_toldNumbers == std::vector<int>{Data::Monsters::MonsterDataHandler::AllNumbers});
    g_MonsterDataHandler.SetNamesChangedCallback(nullptr);
}
