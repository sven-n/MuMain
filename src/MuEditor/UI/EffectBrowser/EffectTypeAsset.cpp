#include "stdafx.h"

#ifdef _EDITOR

#include "EffectTypeAsset.h"

#include "Core/Globals/_enum.h"

namespace MuEditor::Effects
{
using Data::Effects::EffectKind;

// RenderEffects and RenderObject draw the model of an effect's number below
// MAX_MODELS; above it the number is a texture. Particles and joints start
// with the texture of their number (ZzzEffectParticle.cpp, ZzzEffectJoint.cpp).
EffectAssetSlot GetAssetSlot(EffectKind kind, int type)
{
    const bool modelNumber = type < MAX_MODELS;
    switch (kind)
    {
    case EffectKind::Effect:
        return modelNumber ? EffectAssetSlot::Model : EffectAssetSlot::Texture;
    case EffectKind::Particle:
    case EffectKind::Joint:
        return modelNumber ? EffectAssetSlot::TextureChosenInCode : EffectAssetSlot::DefaultTexture;
    case EffectKind::Sprite:
        break;
    }
    return EffectAssetSlot::Texture;
}

bool IsWorldObjectSlot(EffectAssetSlot slot, int type)
{
    return slot == EffectAssetSlot::Model && type >= MODEL_WORLD_OBJECT && type < MAX_WORLD_OBJECTS;
}
} // namespace MuEditor::Effects

#endif // _EDITOR
