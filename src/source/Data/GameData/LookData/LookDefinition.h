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
    // mesh (`evenHidden`: drawing it too). A list: those meshes, one after the other. allBut: every mesh but those,
    // with the texture scripts.
    std::vector<int> meshes;
    bool allBut = false;
    bool evenHidden = false;

    [[nodiscard]] bool IsAll() const
    {
        return meshes.empty() && !allBut;
    }
    bool operator==(const MeshSet&) const = default;
};

// How a value changes over time (design L9), in terms a designer can picture; the drawing turns them into the numbers
// of the clock (WorldTime, in milliseconds).
enum class ShapeKind
{
    // A sine between `from` and `to`, starting halfway and going up; `seconds` a cycle.
    Wave,
    // From `from` up to `to` and back down (the absolute sine); `seconds` a bounce.
    Bounce,
    // `perSecond` more every second, without end (a texture that scrolls).
    Scroll,
    // From `from` towards `to` in `seconds`, then back to `from` (whole milliseconds, as the clock counts).
    Ramp,
    // `count` steps of `step` from 0, `secondsEach` each, then again from 0 (a texture's frames).
    Frames,
    // A value from `from` to `to` in steps of `step`, a new one each frame (Random::DrawRand, fixed for the frame).
    Random,
};

// Its numbers are the file's, so a look moved from code gets the code's float constants back exactly: from -0.2 and
// to 1.2 give the amplitude 0.7 and the middle 0.5, which floats of them would not.
struct Shape
{
    ShapeKind kind = ShapeKind::Wave;
    double from = 0.0;
    double to = 0.0;
    // Wave, Bounce, Ramp: one cycle. Frames: one frame.
    double seconds = 0.0;
    double perSecond = 0.0;
    double step = 0.0;
    int count = 0;

    bool operator==(const Shape&) const = default;
};

// A number, or one that changes over time.
using Number = std::variant<float, Shape>;

// Where an opacity, glow strength or scroll comes from: the item's value (what the drawing hands the look), the
// drawing's opacity, a number, or a shape.
struct FromItem
{
    bool operator==(const FromItem&) const = default;
};
struct FromDrawing
{
    bool operator==(const FromDrawing&) const = default;
};
using Value = std::variant<FromItem, FromDrawing, float, Shape>;

// The glow mesh of a layer: the item's, none, every mesh, or a mesh.
struct NoGlowMesh
{
    bool operator==(const NoGlowMesh&) const = default;
};
struct AllGlowMeshes
{
    bool operator==(const AllGlowMeshes&) const = default;
};
using GlowMesh = std::variant<FromItem, NoGlowMesh, AllGlowMeshes, int>;

// The light a layer is drawn in, red, green and blue. Without one a layer is drawn in the light the layer before left
// on the model: for the first layer the model's light, the one the drawing hands the look (the item's, darker at +14
// and +15); a faded glowing pass dims it for the layers after it.
struct LayerLight
{
    // A tint multiplies the model's light, channel by channel ({"tint": [1, 1, 1]} is the model's light again);
    // otherwise `color` is the light.
    bool tint = false;
    std::array<Number, 3> color{1.f, 1.f, 1.f};

    bool operator==(const LayerLight&) const = default;
};

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
    std::optional<LayerLight> light;
    // The mesh that scrolls while this layer draws (BMD::StreamMesh): drawn without the light of the scene, at full
    // opacity. Without one the model's stays.
    std::optional<int> scrollingMesh;

    bool operator==(const Layer&) const = default;
};

// What a render style leaves on the item for what is drawn after it in the same drawing (the +7 glow, the excellent
// glow): each value it sets. The scrolling mesh is the model's: it stays for later drawings of the model too.
struct ItemValues
{
    std::optional<Number> opacity;
    std::optional<int> glowMesh;
    std::optional<Number> glowStrength;
    std::optional<Number> scrollU;
    std::optional<Number> scrollV;
    std::optional<int> hiddenMesh;
    std::optional<int> scrollingMesh;

    [[nodiscard]] bool IsEmpty() const
    {
        return !opacity && !glowMesh && !glowStrength && !scrollU && !scrollV && !hiddenMesh && !scrollingMesh;
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
