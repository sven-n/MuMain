#pragma once

#include <array>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

// The looks of items as data (docs/item-data.md, "Render styles and item effects"): named render styles in
// Data/Effects/RenderStyles.json and item effects in Data/Effects/ItemEffects.json. A render style draws an item in
// layers; each layer draws some of the model's meshes in one way. Looks still in code (ItemRenderStyles.cpp) are found
// by the same names when no file has them.
namespace Data::Looks
{
// The two kinds of looks (design L13): a render style draws the item (Data/Effects/RenderStyles.json), an item effect
// does what comes before the drawing (Data/Effects/ItemEffects.json). A look has only its kind's parts.
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
    // The mesh in the light alone, without a texture (RENDER_COLOR): a flat color, as a dark backing.
    FlatColor,
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
    // Darkens what is below it (RENDER_DARK), as the Pumpkin of Luck's reflection.
    Darken,
    // The drawing's own render type, which a doppelganger or a monster's hands set (RenderBody(RenderType, ...)).
    AsDrawing,
};

// Which meshes a layer draws.
struct MeshSet
{
    // Empty `meshes` and not `allBut`: every mesh, with the model's texture scripts, leaving out the item's hidden
    // mesh (`evenHidden`: drawing it too). A list: those meshes, one after the other. allBut: every mesh but those,
    // with the texture scripts. `none`: no mesh, a layer that draws nothing, as the default of a layer the values of
    // some items turn on (design L12).
    std::vector<int> meshes;
    bool allBut = false;
    bool evenHidden = false;
    bool none = false;

    [[nodiscard]] bool IsAll() const
    {
        return meshes.empty() && !allBut && !none;
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
    // From `from` straight up to `to` in `seconds`, then straight back down in as long (design L16).
    Zigzag,
    // From the item's turning angle (design L16): `from` at no turn, `to` at a quarter turn, `from` again at half a
    // turn (a sine of the angle), and `from` for the other half.
    Turn,
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
    // As a layer's value: the item's value times the shape (design L16), as the Phoenix Soul items pulse their own
    // opacity and glow strength. A number of a light or of item values reads the shape alone.
    bool timesItem = false;

    bool operator==(const Shape&) const = default;
};

// A number, or one that changes over time.
using Number = std::variant<float, Shape>;

// Where an opacity, glow strength or scroll comes from: the item's value (what the drawing hands the look), the
// drawing's opacity, a number, or a shape.
struct FromItem
{
    // The item's value times this (design L59): the Cursed Castle Water glows at 1.5 and 0.25 of the item's.
    float times = 1.f;

    bool operator==(const FromItem&) const = default;
};
struct FromDrawing
{
    bool operator==(const FromDrawing&) const = default;
};
using Value = std::variant<FromItem, FromDrawing, float, Shape>;

// The glow mesh of a layer: the item's, none, every mesh, or a mesh.
struct ItemGlowMesh
{
    bool operator==(const ItemGlowMesh&) const = default;
};
struct NoGlowMesh
{
    bool operator==(const NoGlowMesh&) const = default;
};
struct AllGlowMeshes
{
    bool operator==(const AllGlowMeshes&) const = default;
};
using GlowMesh = std::variant<ItemGlowMesh, NoGlowMesh, AllGlowMeshes, int>;

// The light a layer is drawn in, red, green and blue. Without one a layer is drawn in the light the layer before left
// on the model: for the first layer the model's light, the one the drawing hands the look (the item's, darker at +14
// and +15); a faded glowing pass dims it for the layers after it.
struct LayerLight
{
    // A tint multiplies the model's light, channel by channel ({"tint": [1, 1, 1]} is the model's light again);
    // otherwise `color` is the light.
    bool tint = false;
    std::array<Number, 3> color{1.f, 1.f, 1.f};
    // The model's light at this brightness instead (design L16): scaled so its brightest channel is this, a grey of it
    // for a black light; `tint` and `color` are not read then.
    std::optional<Number> brightness;

    bool operator==(const LayerLight&) const = default;
};

// Where a reflection goes on a mesh (design L8).
enum class Where
{
    // Over the texture, with the layer's blend.
    Over,
    // Under a texture with holes (BITMAP_t::hasHoles): drawn before it, solid and on both faces, so it fills the holes
    // and the texture covers the rest (the Magic Gladiator sets, #915). On a mesh without holes, a glowing one, and on
    // every mesh of a see-through drawing it goes over the texture with its blend. A look with such a layer is drawn
    // mesh by mesh, as RenderBody draws a model.
    UnderHoles,
};

// Where an item is drawn, for a layer's condition (design L11): worn or held, with its skin, or in the inventory and on
// the ground, where the skin is hidden (BMD::HideSkin).
enum class Place
{
    Worn,
    InventoryAndGround,
    // In the inventory (design L16): the inventory's drawing, which draws items in white light.
    Inventory,
    // On the ground: without the skin, outside the inventory.
    Ground,
};

// A buff on the character a layer can wait for.
enum class LookBuff
{
    InfinityArrow,
};

// When a layer draws (design L11): each part that is set must hold, otherwise the layer is skipped.
struct OnlyWhen
{
    std::optional<Place> place;
    // The level the item is drawn like (DrawLook::level), from and to, both included.
    std::optional<std::pair<int, int>> level;
    // The item's hidden mesh in this drawing (DrawLook::hiddenMesh); -1 for none.
    std::optional<int> hiddenMesh;
    std::optional<LookBuff> buff;
    // A buff the character must not have.
    std::optional<LookBuff> withoutBuff;

    [[nodiscard]] bool IsEmpty() const
    {
        return !place && !level && !hiddenMesh && !buff && !withoutBuff;
    }
    bool operator==(const OnlyWhen&) const = default;
};

// The fields each item sets itself (design L12): a field's key in the file ("meshes", "light") and the name of the
// value in the model entry of an item that uses the look ("chromeMeshes"). The field keeps the look's value, which an
// item that sets none draws with; an item's null leaves the field out (the item's own, or none).
using EachItemFields = std::map<std::string, std::string>;

struct Layer
{
    LayerKind kind = LayerKind::Texture;
    Motion motion = Motion::SlowDrift;
    Blend blend = Blend::Normal;
    // Reflections only.
    Where where = Where::Over;
    MeshSet meshes;
    // A texture file under Data, instead of the mesh's own; empty for the mesh's own.
    std::string texture;
    Value opacity = FromItem{};
    GlowMesh glowMesh = ItemGlowMesh{};
    Value glowStrength = FromItem{};
    Value scrollU = FromItem{};
    Value scrollV = FromItem{};
    std::optional<LayerLight> light;
    // The mesh that scrolls while this layer draws (BMD::StreamMesh): drawn without the light of the scene, at full
    // opacity. Without one the model's stays.
    std::optional<int> scrollingMesh;
    OnlyWhen onlyWhen;
    EachItemFields eachItem;

    bool operator==(const Layer&) const = default;
};

// The drawings a render style is not for (design L11): the game draws the item plainly then, as it draws an item that
// names no style.
struct NotFor
{
    // An item of a doppelganger (RENDER_DOPPELGANGER).
    bool doppelganger = false;
    // Every drawing but in a monster's hands (RENDER_EXTRA). The mark is then the look's own: its layers draw without
    // it, also those that draw as the drawing asks.
    bool outsideMonsterHands = false;

    [[nodiscard]] bool IsEmpty() const
    {
        return !doppelganger && !outsideMonsterHands;
    }
    bool operator==(const NotFor&) const = default;
};

// What a render style leaves on the item for what is drawn after it in the same drawing (the +7 glow, the excellent
// glow): each value it sets. The scrolling mesh and the light are the model's: the scrolling mesh stays for later
// drawings of the model too.
struct ItemValues
{
    std::optional<Number> opacity;
    std::optional<int> glowMesh;
    std::optional<Number> glowStrength;
    std::optional<Number> scrollU;
    std::optional<Number> scrollV;
    std::optional<int> hiddenMesh;
    std::optional<int> scrollingMesh;
    // The model's light after the look (design L46): a color, or a tint of the light the drawing handed the look.
    // Without one the model keeps the light of the last layer.
    std::optional<LayerLight> light;
    EachItemFields eachItem;

    [[nodiscard]] bool IsEmpty() const
    {
        return !opacity && !glowMesh && !glowStrength && !scrollU && !scrollV && !hiddenMesh && !scrollingMesh &&
               !light && eachItem.empty();
    }
    bool operator==(const ItemValues&) const = default;
};

// Item values an item effect sets before the drawing (design L13), when its Only when holds. The entries apply in
// their order, so a condition can read the level an entry before it set.
struct ItemValuesBefore
{
    OnlyWhen onlyWhen;
    // Without the light: the drawing sets the model's light after the item effect.
    ItemValues values;
    // The opacity the drawing hands the look (a layer's "drawing" opacity): the Devil's Eye draws at 2.
    std::optional<Number> drawingOpacity;
    // The level the item glows like from here on, which the looks' level conditions read too: potions above +0 glow
    // like +7.
    std::optional<int> glowsLikeLevel;

    [[nodiscard]] bool IsEmpty() const
    {
        return values.IsEmpty() && !drawingOpacity && !glowsLikeLevel;
    }
    bool operator==(const ItemValuesBefore&) const = default;
};

// What a thing on bones is (design L7, step 3; section 7b): a type of the effect catalogue (FX1) of this kind.
enum class BoneThingKind
{
    Sprite,
    Particle,
    // Lightning between two bones (CreateJoint).
    Joint,
    Effect,
};

// How a chance draws (design L56): once in `oneIn` frames at the game's reference frame rate (rand_fps_check), or
// Random::Rand() % oneIn == 0, as the monster sounds draw theirs.
enum class ChanceDraw
{
    Fps,
    Plain,
};

// A chance per frame (design L56), for each bone or pair, or once for all of them.
struct Chance
{
    int oneIn = 1;
    ChanceDraw draw = ChanceDraw::Fps;
    bool once = false;

    bool operator==(const Chance&) const = default;
};

// Where things on bones are created (design L55): at each bone, between each pair of bones (lightning, from the first
// to the second), or at the item object's own place.
struct BonePlaces
{
    std::vector<int> bones;
    std::vector<std::pair<int, int>> between;
    bool origin = false;

    bool operator==(const BonePlaces&) const = default;
};

// One thing an item effect creates on the item's bones each frame (section 7b), in the update from the item's pose.
struct BoneThing
{
    BoneThingKind kind = BoneThingKind::Sprite;
    // The type's name in the effect catalogue.
    std::string type;
    int subType = 0;
    Number scale = 1.f;
    // Red, green and blue: the sprite's color, the light of a particle, lightning or effect.
    std::array<Number, 3> color{1.f, 1.f, 1.f};
    // A sprite's rotation.
    Number rotation = 0.f;
    // Lightning from the second bone of each pair to the first.
    bool reversed = false;
    // A thing of a group rolls this too, after the group's (the Wing of Storm's thunder).
    std::optional<Chance> chance;

    bool operator==(const BoneThing&) const = default;
};

// An entry of "onBones": one thing, or a group of things ("together") that share its places and its chance, so they
// come together (design L56). A single thing's chance is the entry's.
struct OnBones
{
    BonePlaces places;
    std::optional<Chance> chance;
    std::vector<BoneThing> things;
    bool together = false;

    bool operator==(const OnBones&) const = default;
};

struct LookDefinition
{
    std::string name;
    // A render style's parts.
    std::vector<Layer> layers;
    ItemValues itemValuesAfter;
    NotFor notFor;
    // Draws alone (design L47): drawn where the item effects that drew the model drew, right after the item effect, in
    // the object's own light, with nothing of the drawing code after it (no level light, level glow, excellent glow or
    // buff tint).
    bool drawsAlone = false;
    // An item effect's parts: the item values it sets before the drawing, and the light the update puts on the
    // item's object (OBJECT::Light, design L45), red, green and blue, which the ground item's next draw adds to its
    // light.
    std::vector<ItemValuesBefore> itemValuesBefore;
    std::optional<std::array<Number, 3>> objectLight;
    // The things it creates on the item's bones each frame (design L7, step 3).
    std::vector<OnBones> onBones;
    // How fast the update plays the item object's animation (BMD::PlayAnimation), as the Cursed Castle Water's ground
    // item moves (design L29); without one the update plays none.
    std::optional<float> animationSpeed;

    bool operator==(const LookDefinition&) const = default;
};
} // namespace Data::Looks
