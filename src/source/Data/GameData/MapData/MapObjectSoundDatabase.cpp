#include "stdafx.h"

#include "MapObjectSoundDatabase.h"

namespace Data::Maps
{
MapObjectSoundDatabase& MapObjectSoundDatabase::GetInstance()
{
    static MapObjectSoundDatabase database;
    return database;
}

void MapObjectSoundDatabase::Build(std::span<const MapObjectSound> sounds)
{
    m_sounds.assign(sounds.begin(), sounds.end());
    ++m_version;
}
} // namespace Data::Maps
