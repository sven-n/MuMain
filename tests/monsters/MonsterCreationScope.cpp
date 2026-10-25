#include "stdafx.h"

#include "MonsterCreationScope.h"

#include "Core/Globals/_enum.h"
#include "Engine/Object/ZzzCharacter.h"
#include "World/MapInfra/MapManager.h"
#include "World/MapInfra/w_MapProcess.h"

extern int HeroKey;
extern int HeroIndex;

namespace MonsterRecorder
{
namespace
{
// Keys the server never sends a monster with.
constexpr int HeroTestKey = 0x7F00;
constexpr int MonsterTestKey = 0x7F01;
constexpr unsigned char TileX = 100;
constexpr unsigned char TileY = 100;
} // namespace

struct CreationScope::State
{
    std::unique_ptr<CHARACTER[]> characters{new CHARACTER[MAX_CHARACTERS_CLIENT + 1]{}};
    CHARACTER* previousCharacters = nullptr;
    CHARACTER* previousHero = nullptr;
    int previousHeroKey = 0;
    int previousHeroIndex = 0;
    MapProcessPtr previousMapProcess;
};

CreationScope::CreationScope(int world) : m_load(std::make_unique<LoadScope>(world)), m_state(std::make_unique<State>())
{
    m_state->previousCharacters = CharactersClient;
    m_state->previousHero = Hero;
    m_state->previousHeroKey = HeroKey;
    m_state->previousHeroIndex = HeroIndex;
    m_state->previousMapProcess = g_MapProcess;

    CharactersClient = m_state->characters.get();
    Hero = &CharactersClient[0];
    HeroIndex = 0;
    HeroKey = HeroTestKey;
    CreateCharacterPointer(Hero, MODEL_PLAYER, TileX, TileY, 0.0f);
    Hero->Key = HeroTestKey;
    g_MapProcess = MapProcess::Make();
}

CreationScope::~CreationScope()
{
    for (int i = 0; i <= MAX_CHARACTERS_CLIENT; ++i)
    {
        delete[] CharactersClient[i].Object.BoneTransform;
        CharactersClient[i].Object.BoneTransform = nullptr;
    }
    g_MapProcess = m_state->previousMapProcess;
    HeroIndex = m_state->previousHeroIndex;
    HeroKey = m_state->previousHeroKey;
    Hero = m_state->previousHero;
    CharactersClient = m_state->previousCharacters;
}

void CreationScope::SetWorld(int world)
{
    gMapManager.WorldActive = world;
}

CHARACTER* CreationScope::Create(int number)
{
    return CreateMonster(static_cast<EMonsterType>(number), TileX + 2, TileY, MonsterTestKey);
}

int CreationScope::CreatedModel(int number)
{
    CHARACTER* character = CreateMonster(static_cast<EMonsterType>(number), TileX + 2, TileY, MonsterTestKey);
    if (character == nullptr)
    {
        return -1;
    }
    const int model = character->Object.Type;
    character->Object.Live = false;
    return model;
}
} // namespace MonsterRecorder
