#pragma once

#ifdef _EDITOR

#include "Data/GameData/EffectData/EffectKind.h"

#include <functional>
#include <optional>
#include <string>

namespace MuEditor::Effects
{
// What a type draws: the slot of its number in the models or the textures.
enum class EffectAssetSlot
{
    // An effect with a model number: the model of its slot.
    Model,
    // An effect with a texture number, and every sprite: the texture of its
    // number.
    Texture,
    // A particle or joint with a texture number: that texture, unless the code
    // of its SubType chooses another.
    DefaultTexture,
    // A particle or joint with a model number: the code always chooses a
    // texture.
    TextureChosenInCode,
};

EffectAssetSlot GetAssetSlot(Data::Effects::EffectKind kind, int type);

// The model slots below MAX_WORLD_OBJECTS hold the objects of the current
// map; an effect with such a number draws one of them.
bool IsWorldObjectSlot(EffectAssetSlot slot, int type);

// An effect that draws an object of a map (IsWorldObjectSlot) draws its own
// model only on that map: the castle walls on the Battle Castle siege map,
// the falling stone on the Kalima maps. A world of that map, for its name;
// nullopt for the other types.
std::optional<int> GetHomeWorld(int type);
bool IsHomeWorld(int type, int world);

// What a slot holds right now.
struct EffectAsset
{
    bool loaded = false;
    // The file loaded into the slot (UTF-8), when known.
    std::string file;
    // The world that was active when it was loaded
    // (Core::AssetLoadWorld::LoadingScreen for the loading screen), when
    // recorded.
    std::optional<int> loadWorld;
};

// What loaded a slot's asset, seen from the current map.
enum class AssetOrigin
{
    NotLoaded,
    LoadingScreen,
    ThisMap,
    EarlierMap,
    // Loaded, but nothing recorded what loaded it.
    Unknown,
};

AssetOrigin ClassifyAssetOrigin(const EffectAsset& asset, int currentWorld);

// Reads what a slot holds without loading anything.
using EffectAssetProbe = std::function<EffectAsset(EffectAssetSlot slot, int type)>;
} // namespace MuEditor::Effects

#endif // _EDITOR
