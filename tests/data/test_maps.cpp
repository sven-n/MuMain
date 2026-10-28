#include "stdafx.h"

#include "doctest.h"

// The client's view of the maps (World/MapInfra/MapManager.h).

#include "I18N/All.h"
#include "World/MapInfra/MapManager.h"

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
