#pragma once

#include "Data/GameData/MapData/MapObjectSound.h"

#include <span>
#include <vector>

namespace Data::Maps
{
// The map object sounds in memory (Data/Maps/ObjectSounds.json, maps design
// MP1), in the file's order. Built at start; the Map Editor's Sounds tab
// replaces it as it edits.
class MapObjectSoundDatabase
{
public:
    static MapObjectSoundDatabase& GetInstance();

    // Replaces all entries.
    void Build(std::span<const MapObjectSound> sounds);
    const std::vector<MapObjectSound>& GetAll() const
    {
        return m_sounds;
    }
    // Changes with every Build: what was taken from the table is old.
    int GetVersion() const
    {
        return m_version;
    }

private:
    MapObjectSoundDatabase() = default;

    std::vector<MapObjectSound> m_sounds;
    int m_version = 0;
};
} // namespace Data::Maps

#define g_MapObjectSoundDatabase Data::Maps::MapObjectSoundDatabase::GetInstance()
