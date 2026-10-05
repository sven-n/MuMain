#pragma once

#ifdef _EDITOR

#include "EffectStages.h"
#include "EffectTypeAsset.h"

#include "Data/GameData/EffectData/EffectKind.h"

#include <cstdint>
#include <vector>

namespace MuEditor::Effects
{
// How the preview shows what a type's slot holds.
enum class PreviewDraw
{
    // The model of an effect with a model number.
    Model,
    // The texture, facing the camera.
    Sprite,
    // The texture lying on the ground, for the effects RenderEffectShadows
    // draws there.
    GroundDecal,
    // Nothing to show; the notes say why.
    None,
};

// What the preview says under its view.
enum PreviewNote : std::uint16_t
{
    NoteNothingLoaded = 1 << 0,
    NoteTextureChosenInCode = 1 << 1,
    // A model the game itself never draws: no drawing case and outside the
    // skill models the switch's default draws.
    NoteNotDrawnByGame = 1 << 2,
    // A particle or lightning texture the code of a SubType can change.
    NoteCodeMayChooseTexture = 1 << 3,
    NoteWorldObjectSlot = 1 << 4,
    // The creation values start the effect transparent or at scale 0; its
    // move code fades or grows it in. The preview shows it.
    NoteStartsInvisible = 1 << 5,
    NoteHiddenModel = 1 << 6,
    NoteNoAnimation = 1 << 7,
    // The item's own sprites, particles and effects are removed again.
    NoteItemEffectsLeftOut = 1 << 8,
    // The slot holds an object of this map, not the effect's model: an
    // effect drawing a map object away from its home map.
    NoteNotItsModel = 1 << 9,
};

struct PreviewSubject
{
    PreviewDraw draw = PreviewDraw::None;
    std::uint16_t notes = 0;
};

// What the preview shows for a type of `kind` with this slot, whether the
// slot holds something now and whether that is another map's object, and its
// stages (effects only).
PreviewSubject DescribePreviewSubject(Data::Effects::EffectKind kind, int type, EffectAssetSlot slot, bool loaded,
                                      bool foreignMapObject, const EffectStages& stages);

// The SubTypes the preview offers for a row with variants, one per column of
// the creation table: first one no variant names (the row's own values),
// then the first SubType of each variant.
std::vector<int> PreviewSubTypes(const std::vector<std::vector<int>>& variantSubTypes);

struct PreviewTextureSize
{
    int width = 0;
    int height = 0;
};

// The size of the texture for a view of `width` by `height` points at the
// display's `scale`: rounded up to steps, so small changes keep the texture,
// and kept within what a capture can hold.
PreviewTextureSize ChoosePreviewTextureSize(float width, float height, float scale);
} // namespace MuEditor::Effects

#endif // _EDITOR
