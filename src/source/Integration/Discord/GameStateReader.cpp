#include "stdafx.h"
#include "Integration/Discord/GameStateReader.h"

#include "Character/CharacterManager.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzInventory.h"
#include "Network/Server/WSclient.h"
#include "Scenes/SceneCore.h"
#include "World/GameMaps/GMBattleCastle.h"
#include "World/MapInfra/MapManager.h"

#include "I18N/All.h"

namespace
{
using Integration::Discord::PresenceSnapshot;

// The master-level editions of Blood Castle and Chaos Castle continue the
// numbering of the ordinary ones.
constexpr int MasterBloodCastleLevel = 8;
constexpr int MasterChaosCastleLevel = 7;

constexpr std::size_t LocationBufferLength = 128;

std::wstring EventWithLevel(const wchar_t* eventName, int level)
{
    wchar_t text[LocationBufferLength]{};
    mu_swprintf_s(text, LocationBufferLength, I18N::Game::DiscordEventLevel, eventName, level);
    return text;
}

// Events with numbered levels. Zero for any other map.
int EventLevel(int map)
{
    if (map == WD_52BLOODCASTLE_MASTER_LEVEL)
    {
        return MasterBloodCastleLevel;
    }
    if (map == WD_53CAOSCASTLE_MASTER_LEVEL)
    {
        return MasterChaosCastleLevel;
    }
    if (gMapManager.InBloodCastle(map))
    {
        return map - WD_11BLOODCASTLE1 + 1;
    }
    if (gMapManager.InChaosCastle(map))
    {
        return map - WD_18CHAOS_CASTLE + 1;
    }
    if (gMapManager.IsCursedTemple())
    {
        return map - WD_45CURSEDTEMPLE_LV1 + 1;
    }
    return 0;
}

bool IsEventMap(int map)
{
    return EventLevel(map) > 0 || gMapManager.InDevilSquare() || gMapManager.IsEmpireGuardian() ||
           (map >= WD_65DOPPLEGANGER1 && map <= WD_68DOPPLEGANGER4);
}

void ReadLocation(PresenceSnapshot& snapshot)
{
    const int map = gMapManager.WorldActive;
    if (gMapManager.InBattleCastle(map) && battleCastle::IsBattleCastleStart())
    {
        snapshot.location = I18N::Game::DiscordCastleSiege;
        snapshot.inEvent = true;
        return;
    }

    const wchar_t* mapName = gMapManager.GetMapName(map);
    const int eventLevel = EventLevel(map);
    snapshot.location = eventLevel > 0 ? EventWithLevel(mapName, eventLevel) : std::wstring(mapName);
    snapshot.inEvent = IsEventMap(map);
}

void ReadCharacter(PresenceSnapshot& snapshot)
{
    const CLASS_TYPE characterClass = CharacterAttribute->Class;
    snapshot.className = gCharacterManager.GetCharacterClassText(characterClass);
    snapshot.level = CharacterAttribute->Level;
    snapshot.masterLevel = gCharacterManager.IsMasterLevel(characterClass) ? Master_Level_Data.nMLevel : 0;
}

void ReadParty(PresenceSnapshot& snapshot)
{
    snapshot.partyMembers = PartyNumber;
    snapshot.partyCapacity = MAX_PARTYS;
}

std::optional<PresenceSnapshot> ReadWorld()
{
    if (LoadingWorld != 0 || Hero == nullptr || CharacterAttribute == nullptr)
    {
        return std::nullopt;
    }

    PresenceSnapshot snapshot;
    snapshot.scene = PresenceSnapshot::Scene::World;
    ReadCharacter(snapshot);
    ReadLocation(snapshot);
    ReadParty(snapshot);
    return snapshot;
}

PresenceSnapshot SceneOnly(PresenceSnapshot::Scene scene)
{
    PresenceSnapshot snapshot;
    snapshot.scene = scene;
    return snapshot;
}
} // namespace

namespace Integration::Discord::GameState
{
std::optional<PresenceSnapshot> Read()
{
    switch (SceneFlag)
    {
    case SERVER_LIST_SCENE:
    case WEBZEN_SCENE:
    case LOG_IN_SCENE:
        return SceneOnly(PresenceSnapshot::Scene::Login);
    case CHARACTER_SCENE:
        return SceneOnly(PresenceSnapshot::Scene::CharacterSelect);
    case MAIN_SCENE:
        return ReadWorld();
    case LOADING_SCENE:
    default:
        return std::nullopt;
    }
}
} // namespace Integration::Discord::GameState
