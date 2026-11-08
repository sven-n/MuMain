#pragma once

#include "Data/GameData/LookData/LookDefinition.h"

#include <span>
#include <vector>

// The effects of the Style Editor (design L35, L40): how a layer of a render style reads in designer terms, and the
// layer each effect of the gallery starts with. Only the editor uses them; the file keeps the layers (L8 to L10).
namespace Data::Looks
{
enum class EffectKind
{
    // The item's own texture on its parts: a look's base, its first row.
    Base,
    // A part drawn glowing at a strength, or its texture added over what is below it (the glow blend).
    Glow,
    // A glow whose strength (or an added layer's opacity) waves or bounces.
    Pulse,
    // A reflection moving over a part.
    Shine,
    // Another texture over a part.
    Overlay,
    // A texture that slides across a part.
    Flowing,
    // A part in a color, or tinted.
    Color,
    // A texture stepping through frames or jumping at random.
    Frames,
};

// What a layer does: its main effect, and what else it does, which its settings show too (an overlay that glows and
// flows).
struct LayerEffect
{
    EffectKind kind = EffectKind::Base;
    bool glows = false;
    bool pulses = false;
    bool flows = false;
    bool frames = false;
    bool colored = false;

    bool operator==(const LayerEffect&) const = default;
};

LayerEffect DescribeEffect(const Layer& layer);

// Whether a glow mesh draws a mesh of the layer glowing: one, or all ("none" and the item's are none of the layer's
// own). The effect fields show a glow's fields for it, as DescribeEffect calls the layer glowing.
bool DrawsGlowing(const GlowMesh& mesh);

// The effects the gallery offers, in its order (Base is no effect of it).
std::span<const EffectKind> GetGalleryEffects();

// Whether the effect goes on one part only: a flowing texture and frames move the texture of their layer's glowing
// mesh (the drawing moves the texture of that mesh or of the scrolling mesh only), and a layer has one.
bool NeedsOnePart(EffectKind kind);

// A new layer of the effect on `meshes` (empty: every mesh), with values the moved looks use: a pulse from 0.2 to 0.8
// in 3.14 seconds as the Viper Bow's, a texture flowing 0.5 a second as the Dark Reign Blade's, four frames of 0.08
// seconds as the Flamberge's. A flowing texture and frames make their first mesh the glowing one, which they move.
Layer MakeEffectLayer(EffectKind kind, std::vector<int> meshes);
} // namespace Data::Looks
