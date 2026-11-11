#include "stdafx.h"

#include "LookEffects.h"

#include <array>
#include <utility>
#include <variant>

namespace Data::Looks
{
namespace
{
constexpr std::array<EffectKind, 7> GalleryEffects{EffectKind::Glow,    EffectKind::Pulse,   EffectKind::Shine,
                                                   EffectKind::Overlay, EffectKind::Flowing, EffectKind::Color,
                                                   EffectKind::Frames};

const Shape* ShapeOf(const Value& value)
{
    return std::get_if<Shape>(&value);
}

bool Swings(const Value& value)
{
    const Shape* shape = ShapeOf(value);
    return shape != nullptr && (shape->kind == ShapeKind::Wave || shape->kind == ShapeKind::Bounce);
}

bool Slides(const Value& value)
{
    const Shape* shape = ShapeOf(value);
    return shape != nullptr && (shape->kind == ShapeKind::Scroll || shape->kind == ShapeKind::Ramp);
}

bool Steps(const Value& value)
{
    const Shape* shape = ShapeOf(value);
    return shape != nullptr && (shape->kind == ShapeKind::Frames || shape->kind == ShapeKind::Random);
}

MeshSet MeshesOf(std::vector<int> meshes)
{
    MeshSet set;
    set.meshes = std::move(meshes);
    return set;
}

// The glow of the layer's first mesh, of every mesh for a layer of every mesh.
GlowMesh GlowOn(const MeshSet& meshes)
{
    return meshes.meshes.empty() ? GlowMesh(AllGlowMeshes{}) : GlowMesh(meshes.meshes.front());
}

// The mesh a flowing texture or frames move: the layer's first mesh; on every mesh, the item's glowing mesh (every
// mesh glowing would move none).
GlowMesh MovedMesh(const MeshSet& meshes)
{
    return meshes.meshes.empty() ? GlowMesh(FromItem{}) : GlowMesh(meshes.meshes.front());
}
} // namespace

bool DrawsGlowing(const GlowMesh& mesh)
{
    return std::holds_alternative<int>(mesh) || std::holds_alternative<AllGlowMeshes>(mesh);
}

LayerEffect DescribeEffect(const Layer& layer)
{
    LayerEffect effect;
    effect.glows = DrawsGlowing(layer.glowMesh);
    effect.pulses = Swings(layer.glowStrength) || Swings(layer.opacity);
    effect.flows = Slides(layer.scrollU) || Slides(layer.scrollV);
    effect.frames = Steps(layer.scrollU) || Steps(layer.scrollV);
    effect.colored = layer.light.has_value();
    // The main effect, from what a layer does the most to what is left of it.
    if (layer.kind == LayerKind::Reflection)
    {
        effect.kind = EffectKind::Shine;
    }
    else if (!layer.texture.empty())
    {
        effect.kind = EffectKind::Overlay;
    }
    else if (effect.frames)
    {
        effect.kind = EffectKind::Frames;
    }
    else if (effect.flows)
    {
        effect.kind = EffectKind::Flowing;
    }
    // A layer added over what is below it (the glow blend) brightens its parts: a glow too.
    else if (Swings(layer.glowStrength) || (layer.blend == Blend::Glow && Swings(layer.opacity)))
    {
        effect.kind = EffectKind::Pulse;
    }
    else if (effect.glows)
    {
        effect.kind = EffectKind::Glow;
    }
    else if (effect.colored)
    {
        effect.kind = EffectKind::Color;
    }
    else if (layer.blend == Blend::Glow)
    {
        effect.kind = EffectKind::Glow;
    }
    // What the main effect is, the rest shows only what it adds.
    switch (effect.kind)
    {
    case EffectKind::Glow:
        effect.glows = false;
        break;
    case EffectKind::Pulse:
        effect.glows = false;
        effect.pulses = false;
        break;
    case EffectKind::Flowing:
        effect.flows = false;
        break;
    case EffectKind::Frames:
        effect.frames = false;
        break;
    case EffectKind::Color:
        effect.colored = false;
        break;
    default:
        break;
    }
    return effect;
}

std::span<const EffectKind> GetGalleryEffects()
{
    return GalleryEffects;
}

bool NeedsOnePart(EffectKind kind)
{
    return kind == EffectKind::Flowing || kind == EffectKind::Frames;
}

Layer MakeEffectLayer(EffectKind kind, std::vector<int> meshes)
{
    Layer layer;
    layer.meshes = MeshesOf(std::move(meshes));
    switch (kind)
    {
    case EffectKind::Base:
        break;
    case EffectKind::Glow:
        layer.glowMesh = GlowOn(layer.meshes);
        layer.glowStrength = 1.0f;
        break;
    case EffectKind::Pulse:
        layer.glowMesh = GlowOn(layer.meshes);
        layer.glowStrength = Shape{.kind = ShapeKind::Wave, .from = 0.2, .to = 0.8, .seconds = 3.1415925};
        break;
    case EffectKind::Shine:
        layer.kind = LayerKind::Reflection;
        layer.motion = Motion::SlowDrift;
        layer.blend = Blend::Glow;
        break;
    case EffectKind::Overlay:
        layer.blend = Blend::Glow;
        layer.texture = "Effect/Chrome01.jpg";
        break;
    case EffectKind::Flowing:
        layer.glowMesh = MovedMesh(layer.meshes);
        layer.scrollV = Shape{.kind = ShapeKind::Scroll, .perSecond = 0.5};
        break;
    case EffectKind::Color:
        layer.light = LayerLight{.tint = false, .color = {0.6f, 0.8f, 1.0f}};
        break;
    case EffectKind::Frames:
        layer.glowMesh = MovedMesh(layer.meshes);
        layer.scrollU = Shape{.kind = ShapeKind::Frames, .seconds = 0.08, .step = 0.25, .count = 4};
        break;
    }
    return layer;
}
} // namespace Data::Looks
