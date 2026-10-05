#pragma once

#ifdef _EDITOR

#include "Data/GameData/EffectData/EffectKind.h"

#include <functional>
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

// What a slot holds right now.
struct EffectAsset
{
    bool loaded = false;
    // The file loaded into the slot (UTF-8), when known.
    std::string file;
};

// Reads what a slot holds without loading anything.
using EffectAssetProbe = std::function<EffectAsset(EffectAssetSlot slot, int type)>;
} // namespace MuEditor::Effects

#endif // _EDITOR
