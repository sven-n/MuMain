#include "stdafx.h"

#include "MapObjectSounds.h"

#include "Audio/SoundNumbers.h"
#include "Data/GameData/Common/DataPaths.h"
#include "Data/GameData/MapData/MapObjectSoundDatabase.h"
#include "Audio/SoundOverlay.h"
#include "Audio/SoundSource.h"
#include "Engine/Object/ZzzCharacter.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <utility>
#include <vector>

namespace World::MapObjectSounds
{
namespace
{
using Data::Maps::MapObjectSound;

constexpr int ObjectTypeCount = Data::Maps::MaxMapObjectType + 1;
// The distances of DistanceLevel (100 units a tile) and its lowest level.
constexpr float FullVolumeDistance = 300.0f;
constexpr float QuietestDistance = 1500.0f;
constexpr float QuietestLevel = 0.25f;

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
    // The sounds that any object plays (Q-MP8).
    std::vector<ESound> anyObject;
};

LoadedMap& Loaded()
{
    static LoadedMap loaded;
    return loaded;
}

// A copy a drawn object asks for, in the order they were drawn.
struct DrawnStart
{
    ESound sound = Audio::SoundNumbers::NoNumber;
    // By the object's distance from the hero (DistanceLevel).
    float level = 1.0f;
    int objectType = -1;
    std::array<float, 3> position{};
};

// The starts of the drawn objects until PlayDrawnSounds, and the loudest
// level of each of their sounds: kept between frames (no allocation each
// frame), only the main loop uses them.
std::vector<DrawnStart>& DrawnStarts()
{
    static std::vector<DrawnStart> starts;
    return starts;
}

std::vector<std::pair<ESound, float>>& LoudestLevels()
{
    static std::vector<std::pair<ESound, float>> levels;
    return levels;
}

// The map-wide sounds asked for a start in this frame already: by its first
// drawn object, as more starts of a sound that plays could not start a copy.
bool& MapWideNoted()
{
    static bool noted = false;
    return noted;
}

void NoteStart(const OBJECT& object, ESound sound, float level)
{
    DrawnStarts().push_back({sound, level, object.Type, {object.Position[0], object.Position[1], object.Position[2]}});
}

// The loudest level each sound of the starts has: a map plays a handful.
void FindLoudestLevels(const std::vector<DrawnStart>& starts)
{
    std::vector<std::pair<ESound, float>>& levels = LoudestLevels();
    levels.clear();
    for (const DrawnStart& start : starts)
    {
        const auto same = std::find_if(levels.begin(), levels.end(), [&start](const std::pair<ESound, float>& entry)
                                       { return entry.first == start.sound; });
        if (same == levels.end())
        {
            levels.emplace_back(start.sound, start.level);
        }
        else
        {
            same->second = std::max(same->second, start.level);
        }
    }
}

float LoudestLevel(ESound sound)
{
    for (const auto& [number, level] : LoudestLevels())
    {
        if (number == sound)
        {
            return level;
        }
    }
    return 1.0f;
}

ESound LoadSound(const MapObjectSound& sound)
{
    const std::wstring file = Data::Paths::ToLoaderPath(sound.file);
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

// The object's distance from the hero on the ground; 0 without a hero.
float DistanceFromHero(const OBJECT& object)
{
    if (Hero == nullptr)
    {
        return 0.0f;
    }
    const float dx = object.Position[0] - Hero->Object.Position[0];
    const float dy = object.Position[1] - Hero->Object.Position[1];
    return std::sqrt(dx * dx + dy * dy);
}

#ifdef _EDITOR
// The Sound Mixer's overlay marks the drawn object with its volume rings.
void NoteForOverlay(const OBJECT& object, ESound sound)
{
    if (!Audio::Overlay::IsOn())
    {
        return;
    }
    Audio::Overlay::DrawnSource drawn;
    drawn.kind = Audio::SoundKind::MapObject;
    drawn.sourceNumber = object.Type;
    drawn.soundNumber = static_cast<int>(sound);
    drawn.position = {object.Position[0], object.Position[1], object.Position[2]};
    drawn.inner = FullVolumeDistance;
    drawn.outer = QuietestDistance;
    Audio::Overlay::NoteDrawn(drawn);
}
#endif
} // namespace

void LoadForMap(int serverMap, int stage)
{
    LoadedMap& loaded = Loaded();
    loaded.serverMap = serverMap;
    loaded.stage = stage;
    loaded.version = g_MapObjectSoundDatabase.GetVersion();
    loaded.sounds.fill(Audio::SoundNumbers::NoNumber);
    loaded.anyObject.clear();
    for (const MapObjectSound& sound : g_MapObjectSoundDatabase.GetAll())
    {
        // An entry the Map Editor added has no file yet.
        if (sound.file.empty() || !Data::Maps::Names(sound.maps, serverMap, stage))
        {
            continue;
        }
        const ESound number = LoadSound(sound);
        if (sound.anyObject && number != Audio::SoundNumbers::NoNumber)
        {
            loaded.anyObject.push_back(number);
        }
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
#ifdef _EDITOR
        NoteForOverlay(object, *sound);
#endif
        NoteStart(object, *sound, DistanceLevel(DistanceFromHero(object)));
    }
    // A map-wide sound: at full volume, as no one object is its place, and
    // once a frame, as its starts would all be the same.
    if (!MapWideNoted())
    {
        for (const ESound sound : Loaded().anyObject)
        {
            NoteStart(object, sound, 1.0f);
        }
        MapWideNoted() = true;
    }
}

void PlayDrawnSounds()
{
    std::vector<DrawnStart>& starts = DrawnStarts();
    FindLoudestLevels(starts);
    for (const DrawnStart& start : starts)
    {
        const Audio::ScopedSoundSource source(Audio::SoundKind::MapObject, {}, start.objectType, start.position.data());
        PlayBufferOnFreeChannel(start.sound, LoudestLevel(start.sound));
    }
    starts.clear();
    MapWideNoted() = false;
}

float DistanceLevel(float distance)
{
    if (distance <= FullVolumeDistance)
    {
        return 1.0f;
    }
    if (distance >= QuietestDistance)
    {
        return QuietestLevel;
    }
    const float along = (distance - FullVolumeDistance) / (QuietestDistance - FullVolumeDistance);
    return 1.0f - along * (1.0f - QuietestLevel);
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
