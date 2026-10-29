#include "stdafx.h"

#include "doctest.h"

// The client's view of the maps (World/MapInfra/MapManager.h).

#include "I18N/All.h"
#include "Network/Server/WSclient.h"
#include "World/MapInfra/MapManager.h"

#include <array>
#include <cstring>
#include <string>

namespace
{
// The hero's map, for the life of the scope.
class HeroMapScope
{
public:
    explicit HeroMapScope(int world) : m_previous(gMapManager.WorldActive)
    {
        gMapManager.WorldActive = world;
    }

    ~HeroMapScope()
    {
        gMapManager.WorldActive = m_previous;
    }

    HeroMapScope(const HeroMapScope&) = delete;
    HeroMapScope& operator=(const HeroMapScope&) = delete;

private:
    int m_previous;
};

std::wstring NameOf(int map)
{
    return gMapManager.GetMapName(map);
}
} // namespace

TEST_CASE("Map names: a map is named by its own number, wherever the hero is [maps]")
{
    for (const int heroMap : {static_cast<int>(WD_0LORENCIA), static_cast<int>(WD_41CHANGEUP3RD_1ST),
                              static_cast<int>(WD_42CHANGEUP3RD_2ND), static_cast<int>(WD_45CURSEDTEMPLE_LV1)})
    {
        INFO("hero on map " << heroMap);
        const HeroMapScope hero(heroMap);
        // A party member in Lorencia was in the barracks while the hero was.
        CHECK(NameOf(WD_0LORENCIA) == I18N::Game::Lorencia);
        CHECK(NameOf(WD_41CHANGEUP3RD_1ST) == I18N::Game::BalgassBarrack);
        CHECK(NameOf(WD_42CHANGEUP3RD_2ND) == I18N::Game::BalgassRestingPlace);
        CHECK(NameOf(WD_45CURSEDTEMPLE_LV1) == I18N::Game::IllusionTemple);
        CHECK(NameOf(WD_45CURSEDTEMPLE_LV6) == I18N::Game::IllusionTemple);
        CHECK(NameOf(WD_56MAP_SWAMP_OF_QUIET) == I18N::Game::SwampOfPeace);
    }
}

TEST_CASE("Maps: the server's map number stays, Devil Square 5-7 loads as Devil Square (D67) [maps]")
{
    const HeroMapScope hero(WD_0LORENCIA);
    gMapManager.SetServerMap(32);
    CHECK(gMapManager.WorldActive == WD_9DEVILSQUARE);
    CHECK(gMapManager.GetServerMap() == 32);
    gMapManager.SetServerMap(WD_9DEVILSQUARE);
    CHECK(gMapManager.WorldActive == WD_9DEVILSQUARE);
    CHECK(gMapManager.GetServerMap() == WD_9DEVILSQUARE);
    gMapManager.SetServerMap(WD_24HELLAS_7);
    CHECK(gMapManager.WorldActive == WD_24HELLAS_7);
    CHECK(gMapManager.GetServerMap() == WD_24HELLAS_7);

    // Where WorldActive was set alone, its number.
    gMapManager.SetServerMap(32);
    gMapManager.WorldActive = WD_0LORENCIA;
    CHECK(gMapManager.GetServerMap() == WD_0LORENCIA);
}

TEST_CASE("Maps: the stage the server sends with the map, none where WorldActive was set alone (D67) [maps]")
{
    const HeroMapScope hero(WD_0LORENCIA);
    gMapManager.SetServerStage(2);
    gMapManager.SetServerMap(WD_9DEVILSQUARE);
    CHECK(gMapManager.GetServerStage() == 2);
    // The map loads after the packet set the stage.
    gMapManager.SetServerMap(32);
    CHECK(gMapManager.GetServerStage() == 2);
    gMapManager.SetServerStage(0);
    CHECK(gMapManager.GetServerStage() == 0);

    gMapManager.SetServerStage(6);
    gMapManager.WorldActive = WD_0LORENCIA;
    CHECK(gMapManager.GetServerStage() == 0);
    gMapManager.SetServerStage(0);
}

TEST_CASE("Maps: MapChanged carries the stage at byte 10, a shorter packet none (D67) [maps][network]")
{
    // OpenMU's C3 1C 0F to the extended client in Devil Square 2: map 9 at
    // 130/90, stage 2.
    const std::array<BYTE, 15> packet = {0xC3, 15, 0x1C, 0x0F, 1, 0, 9, 130, 90, 1, 2, 0, 0, 0, 0};
    PRECEIVE_TELEPORT_POSITION teleport{};
    std::memcpy(&teleport, packet.data(), sizeof(teleport));
    CHECK(teleport.Map == WD_9DEVILSQUARE);
    CHECK(GetTeleportStage(teleport) == 2);
    teleport.Header.Size = 10;
    CHECK(GetTeleportStage(teleport) == 0);
}

TEST_CASE("Maps: a map's files folder, one for all rounds of an event (D67) [maps]")
{
    struct Folder
    {
        int map;
        int folder;
    };
    for (const Folder expected :
         {Folder{WD_0LORENCIA, 1}, Folder{WD_9DEVILSQUARE, 10}, Folder{32, 10}, Folder{WD_11BLOODCASTLE1, 12},
          Folder{WD_11BLOODCASTLE1 + 6, 12}, Folder{WD_52BLOODCASTLE_MASTER_LEVEL, 12}, Folder{WD_18CHAOS_CASTLE, 19},
          Folder{WD_53CAOSCASTLE_MASTER_LEVEL, 19}, Folder{WD_24HELLAS, 25}, Folder{WD_24HELLAS_7, 25},
          Folder{WD_45CURSEDTEMPLE_LV1, 47}, Folder{WD_45CURSEDTEMPLE_LV6, 47}, Folder{WD_56MAP_SWAMP_OF_QUIET, 57},
          Folder{WD_79UNITEDMARKETPLACE, 80}})
    {
        INFO("map " << expected.map);
        CHECK(gMapManager.GetFilesFolder(expected.map) == expected.folder);
    }
}
