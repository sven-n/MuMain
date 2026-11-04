#pragma once

#include <array>
#include <optional>
#include <string>
#include <variant>
#include <vector>

// The looks of items as data (docs/item-data.md, "Render styles and item effects"): named render styles in
// Data/Effects/RenderStyles.json and item effects in Data/Effects/ItemEffects.json. A render style draws an item in
// layers; each layer draws some of the model's meshes in one way. Looks still in code (ItemRenderStyles.cpp) are found
// by the same names when no file has them.
namespace Data::Looks
{
enum class LookKind
{
    RenderStyle,
    ItemEffect,
};

// What a layer draws its meshes with.
enum class LayerKind
{
    // The mesh's own texture, or the layer's.
    Texture,
    // A reflection that moves over the mesh (chrome); its Motion says how.
    Reflection,
};

// How a reflection moves: the chrome variants of the model drawing.
enum class Motion
{
    SlowDrift,     // RENDER_CHROME
    FastSweep,     // RENDER_CHROME2
    FollowsLight,  // RENDER_CHROME3
    Swirl,         // RENDER_CHROME4
    StrongSwirl,   // RENDER_CHROME5
    Stripes,       // RENDER_CHROME6
    VerySlowDrift, // RENDER_CHROME7
    OilSheen,      // RENDER_OIL
    StillShine,    // RENDER_METAL
};

// How a layer's pixels mix with what is drawn below.
enum class Blend
{
    Normal,
    // Added (RENDER_BRIGHT).
    Glow,
    // The drawing's own render type, which a doppelganger or a monster's hands set (RenderBody(RenderType, ...)).
    AsDrawing,
};

// Which meshes a layer draws.
struct MeshSet
{
    // Empty `meshes` and not `allBut`: every mesh, with the model's texture scripts, leaving out the item's hidden
    // mesh. A list: those meshes, one after the other. allBut: every mesh but those, with the texture scripts.
    std::vector<int> meshes;
    bool allBut = false;

    [[nodiscard]] bool IsAll() const
    {
        return meshes.empty() && !allBut;
    }
    bool operator==(const MeshSet&) const = default;
};

// Where an opacity, glow strength or scroll comes from: the item's value (what the drawing hands the look), the
// drawing's opacity, or a number.
struct FromItem
{
    bool operator==(const FromItem&) const = default;
};
struct FromDrawing
{
    bool operator==(const FromDrawing&) const = default;
};
using Value = std::variant<FromItem, FromDrawing, float>;

// The glow mesh of a layer: the item's, none, or a mesh.
struct NoGlowMesh
{
    bool operator==(const NoGlowMesh&) const = default;
};
using GlowMesh = std::variant<FromItem, NoGlowMesh, int>;

struct Layer
{
    LayerKind kind = LayerKind::Texture;
    Motion motion = Motion::SlowDrift;
    Blend blend = Blend::Normal;
    MeshSet meshes;
    // A texture file under Data, instead of the mesh's own; empty for the mesh's own.
    std::string texture;
    Value opacity = FromItem{};
    GlowMesh glowMesh = FromItem{};
    Value glowStrength = FromItem{};
    Value scrollU = FromItem{};
    Value scrollV = FromItem{};

    bool operator==(const Layer&) const = default;
};

// What a render style leaves on the item for what is drawn after it in the same drawing (the +7 glow, the excellent
// glow): each value it sets.
struct ItemValues
{
    std::optional<float> opacity;
    std::optional<int> glowMesh;
    std::optional<float> glowStrength;
    std::optional<float> scrollU;
    std::optional<float> scrollV;
    std::optional<int> hiddenMesh;

    [[nodiscard]] bool IsEmpty() const
    {
        return !opacity && !glowMesh && !glowStrength && !scrollU && !scrollV && !hiddenMesh;
    }
    bool operator==(const ItemValues&) const = default;
};

struct LookDefinition
{
    std::string name;
    std::vector<Layer> layers;
    ItemValues itemValuesAfter;

    bool operator==(const LookDefinition&) const = default;
};
} // namespace Data::Looks
