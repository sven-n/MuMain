#include "stdafx.h"

#include "doctest.h"

// The client's view of the maps (World/MapInfra/MapManager.h).

#include "I18N/All.h"
#include "Network/Server/WSclient.h"
#include "World/MapInfra/MapManager.h"

#include <array>
#include <cstring>
#include <span>
#include <string>

namespace
{
// The hero's map, for the life of the scope: the world, and the server map
// and stage a test sets go back to what they were. A server map left behind
// would name a world set alone later (32 for world 9).
class HeroMapScope
{
public:
    explicit HeroMapScope(int world)
        : m_previousWorld(gMapManager.WorldActive), m_previousServerMap(gMapManager.GetServerMap()),
          m_previousStage(gMapManager.GetServerStage())
    {
        gMapManager.WorldActive = world;
    }

    ~HeroMapScope()
    {
        gMapManager.SetServerMap(m_previousServerMap);
        gMapManager.SetServerStage(m_previousStage);
        gMapManager.WorldActive = m_previousWorld;
    }

    HeroMapScope(const HeroMapScope&) = delete;
    HeroMapScope& operator=(const HeroMapScope&) = delete;

private:
    int m_previousWorld;
    int m_previousServerMap;
    int m_previousStage;
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

TEST_CASE("Maps: a move loads the world only for another server map, also between Devil Square 1-4 and 5-7 (D67) "
          "[maps]")
{
    const HeroMapScope hero(WD_0LORENCIA);
    gMapManager.SetServerMap(32);
    CHECK(gMapManager.IsServerMapLoaded(32));
    // World 9 is loaded, but the server's map 9 is Devil Square 1-4.
    CHECK_FALSE(gMapManager.IsServerMapLoaded(WD_9DEVILSQUARE));
    gMapManager.SetServerMap(WD_9DEVILSQUARE);
    CHECK(gMapManager.IsServerMapLoaded(WD_9DEVILSQUARE));
    CHECK_FALSE(gMapManager.IsServerMapLoaded(32));
    gMapManager.SetServerMap(WD_3NORIA);
    CHECK(gMapManager.IsServerMapLoaded(WD_3NORIA));
    CHECK_FALSE(gMapManager.IsServerMapLoaded(WD_9DEVILSQUARE));
}

TEST_CASE("Maps: a test's hero map leaves no server map or stage behind [maps]")
{
    {
        const HeroMapScope hero(WD_0LORENCIA);
        gMapManager.SetServerStage(6);
        gMapManager.SetServerMap(SecondDevilSquareMapId);
    }
    // World 9 set alone is Devil Square 1-4, not the 32 of the scope before.
    const HeroMapScope hero(WD_9DEVILSQUARE);
    CHECK(gMapManager.GetServerMap() == WD_9DEVILSQUARE);
    CHECK(gMapManager.GetServerStage() == 0);
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

TEST_CASE("Maps: MapChangedExtended carries the stage at 15, the 15-byte MapChanged none (D67) [maps][network]")
{
    // OpenMU's C3 1C 0F to the extended client in Devil Square 2: map 9 at
    // 130/90, stage 2.
    const std::array<BYTE, 16> extended = {0xC3, 16, 0x1C, 0x0F, 1, 0, 9, 130, 90, 1, 0, 0, 0, 0, 0, 2};
    PRECEIVE_TELEPORT_POSITION teleport{};
    static_assert(sizeof(teleport) == extended.size());
    std::memcpy(&teleport, extended.data(), sizeof(teleport));
    CHECK(teleport.Map == WD_9DEVILSQUARE);
    CHECK(GetTeleportStage(teleport) == 2);

    // The original MapChanged, whose bytes 10 to 14 aren't initialized,
    // followed by a byte of the next packet in the receive buffer.
    const std::array<BYTE, 16> original = {0xC3, 15, 0x1C, 0x0F, 1, 0, 9, 130, 90, 1, 2, 7, 7, 7, 7, 2};
    std::memcpy(&teleport, original.data(), sizeof(teleport));
    CHECK(teleport.Map == WD_9DEVILSQUARE);
    CHECK(GetTeleportStage(teleport) == 0);
}

TEST_CASE("Maps: CharacterInformationExtended carries the stage at 92, a 92-byte packet none (D67) [maps][network]")
{
    // OpenMU's C3 F3 03 to the extended client which logs in again in Devil
    // Square 2 (93 bytes, the stage last), and a byte of the next packet in
    // the receive buffer.
    std::array<BYTE, 94> buffer{0xC3, 93, 0xF3, 0x03};
    buffer[92] = 2;
    buffer[93] = 7;
    CHECK(GetJoinMapServerStage(std::span<const BYTE>(buffer.data(), 93)) == 2);

    // The 92-byte packet of a server which doesn't send the stage.
    buffer[1] = 92;
    CHECK(GetJoinMapServerStage(std::span<const BYTE>(buffer.data(), 92)) == 0);
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
