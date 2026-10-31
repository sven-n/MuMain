#include "stdafx.h"

#include "MapObjectSounds.h"

#include "Audio/SoundNumbers.h"
#include "Core/Text/Utf8.h"
#include "Data/GameData/MapData/MapObjectSoundDatabase.h"

#include <algorithm>
#include <array>

namespace World::MapObjectSounds
{
namespace
{
using Data::Maps::MapObjectSound;

constexpr int ObjectTypeCount = Data::Maps::MaxMapObjectType + 1;

struct LoadedMap
{
    LoadedMap()
    {
        sounds.fill(Audio::SoundNumbers::NoNumber);
    }

    int serverMap = -1;
    int stage = 0;
    // The table's version the sounds were loaded from.
    int version = -1;
    // The sound of each object type, NoNumber for none.
    std::array<ESound, ObjectTypeCount> sounds{};
};

LoadedMap& Loaded()
{
    static LoadedMap loaded;
    return loaded;
}

// "Data/Sound/a.wav" -> "Data\Sound\a.wav", as LoadWaveFile takes it.
std::wstring ToLoaderPath(const std::string& path)
{
    std::wstring loaderPath = Core::Text::FromUtf8(path.c_str());
    std::replace(loaderPath.begin(), loaderPath.end(), L'/', L'\\');
    return loaderPath;
}

ESound LoadSound(const MapObjectSound& sound)
{
    const std::wstring file = ToLoaderPath(sound.file);
    const ESound number = Audio::SoundNumbers::ForFile(file);
    if (number != Audio::SoundNumbers::NoNumber && !IsSoundLoaded(number, file.c_str(), sound.channels, false))
    {
        LoadWaveFile(number, file.c_str(), sound.channels, false);
    }
    return number;
}

// Loads again when the table changed since the map loaded its sounds.
void FollowTable()
{
    const LoadedMap& loaded = Loaded();
    if (loaded.serverMap >= 0 && loaded.version != g_MapObjectSoundDatabase.GetVersion())
    {
        LoadForMap(loaded.serverMap, loaded.stage);
    }
}
} // namespace

void LoadForMap(int serverMap, int stage)
{
    LoadedMap& loaded = Loaded();
    loaded.serverMap = serverMap;
    loaded.stage = stage;
    loaded.version = g_MapObjectSoundDatabase.GetVersion();
    loaded.sounds.fill(Audio::SoundNumbers::NoNumber);
    for (const MapObjectSound& sound : g_MapObjectSoundDatabase.GetAll())
    {
        // An entry the Map Editor added has no file yet.
        if (sound.file.empty() || !Data::Maps::Names(sound.maps, serverMap, stage))
        {
            continue;
        }
        const ESound number = LoadSound(sound);
        for (const int type : sound.objects)
        {
            if (type >= 0 && type < ObjectTypeCount)
            {
                loaded.sounds[static_cast<size_t>(type)] = number;
            }
        }
    }
}

void OnObjectDrawn(const OBJECT& object)
{
    FollowTable();
    if (const std::optional<ESound> sound = FindSound(object.Type))
    {
        PlayBufferOnFreeChannel(*sound);
    }
}

std::optional<ESound> FindSound(int objectType)
{
    if (objectType < 0 || objectType >= ObjectTypeCount)
    {
        return std::nullopt;
    }
    const ESound sound = Loaded().sounds[static_cast<size_t>(objectType)];
    return sound != Audio::SoundNumbers::NoNumber ? std::optional<ESound>(sound) : std::nullopt;
}
} // namespace World::MapObjectSounds
