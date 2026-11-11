#include "stdafx.h"

#ifdef _EDITOR

#include "EffectTypeAsset.h"

#include "Core/Globals/_enum.h"
#include "Core/Utilities/AssetLoadWorld.h"
#include "World/MapInfra/MapManager.h"

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

// The map code that creates them: GMBattleCastle.cpp when a wall breaks,
// GMHellas.cpp for the falling stones.
std::optional<int> GetHomeWorld(int type)
{
    if (type >= BATTLE_CASTLE_WALL1 && type <= BATTLE_CASTLE_WALL4)
        return WD_30BATTLECASTLE;
    if (type == MODEL_KALIMA_FALLING_STONE)
        return WD_24HELLAS;
    return std::nullopt;
}

bool IsHomeWorld(int type, int world)
{
    if (type >= BATTLE_CASTLE_WALL1 && type <= BATTLE_CASTLE_WALL4)
        return gMapManager.InBattleCastle(world);
    if (type == MODEL_KALIMA_FALLING_STONE)
        return gMapManager.InHellas(world);
    return false;
}

AssetOrigin ClassifyAssetOrigin(const EffectAsset& asset, int currentWorld)
{
    if (!asset.loaded)
        return AssetOrigin::NotLoaded;
    if (!asset.loadWorld)
        return AssetOrigin::Unknown;
    if (*asset.loadWorld == Core::AssetLoadWorld::LoadingScreen)
        return AssetOrigin::LoadingScreen;
    return *asset.loadWorld == currentWorld ? AssetOrigin::ThisMap : AssetOrigin::EarlierMap;
}
} // namespace MuEditor::Effects

#endif // _EDITOR
