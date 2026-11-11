#include "stdafx.h"

#include "LookDrawing.h"

#include "Core/Globals/_enum.h"
#include "Core/Math/ZzzMathLib.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Core/Utilities/Random.h"
#include "Core/Utilities/_GlobalFunctions.h"
#include "Engine/Object/w_ObjectInfo.h"
#include "Data/GameData/Common/MeshLimits.h"
#include "Data/GameData/LookData/LookStore.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "Render/Textures/ZzzOpenglUtil.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Render::Items::Looks
{
namespace
{
using namespace Data::Looks;

constexpr double Pi = 3.14159265358979323846;

static_assert(Data::MaxModelMeshes == MAX_MESH, "the data names the meshes a model can have");

// The texture index of each texture file a layer names; BITMAP_UNKNOWN for a file that is missing.
std::unordered_map<std::string, GLuint> s_textures;

int MotionFlag(Motion motion)
{
    switch (motion)
    {
    case Motion::SlowDrift:
        return RENDER_CHROME;
    case Motion::FastSweep:
        return RENDER_CHROME2;
    case Motion::FollowsLight:
        return RENDER_CHROME3;
    case Motion::Swirl:
        return RENDER_CHROME4;
    case Motion::StrongSwirl:
        return RENDER_CHROME5;
    case Motion::Stripes:
        return RENDER_CHROME6;
    case Motion::VerySlowDrift:
        return RENDER_CHROME7;
    case Motion::OilSheen:
        return RENDER_OIL;
    case Motion::StillShine:
        return RENDER_METAL;
    }
    return RENDER_CHROME;
}

// The render flags of a layer's draws.
int LayerFlags(const Layer& layer, int renderType)
{
    if (layer.blend == Blend::AsDrawing)
    {
        return renderType;
    }
    const int flags = layer.kind == LayerKind::Texture     ? RENDER_TEXTURE
                      : layer.kind == LayerKind::FlatColor ? RENDER_COLOR
                                                           : MotionFlag(layer.motion);
    return layer.blend == Blend::Glow ? flags | RENDER_BRIGHT : flags;
}

// The object of one drawing, which random values are drawn for; each random value of a look takes the next salt.
struct Moment
{
    const OBJECT* object = nullptr;
    std::uint32_t salt = 0;
};

// The angle at the clock `time` (milliseconds) of a sine that turns `radiansPerCycle` in `seconds`, as the looks in
// code wrote it: the clock times a float constant.
float Angle(double radiansPerCycle, double seconds, double time)
{
    const auto rate = static_cast<float>(radiansPerCycle / (seconds * 1000.0));
    return static_cast<float>(time * static_cast<double>(rate));
}

// Milliseconds, rounded to the microsecond, so 0.08 seconds count 80 milliseconds.
double Milliseconds(double seconds)
{
    return std::round(seconds * 1.0e6) / 1.0e3;
}

float ValueAt(const Shape& shape, Moment& moment)
{
    const double from = shape.from;
    const double to = shape.to;
    switch (shape.kind)
    {
    case ShapeKind::Wave:
    case ShapeKind::Bounce:
    case ShapeKind::Scroll:
    case ShapeKind::Ramp:
    case ShapeKind::Zigzag:
    case ShapeKind::Frames:
        return ShapeAtClock(shape, WorldTime);
    case ShapeKind::Turn:
    {
        // As the code wrote it: the angle in whole degrees, a sine over its first half turn.
        const int angle = moment.object != nullptr ? static_cast<int>(moment.object->Angle[1]) % 360 : 0;
        if (angle <= 0 || angle > 180)
        {
            return static_cast<float>(from);
        }
        return static_cast<float>(from) +
               sinf(Q_PI * static_cast<float>(angle) / 180.0f) * static_cast<float>(to - from);
    }
    case ShapeKind::Random:
    {
        const int count = std::max(1, static_cast<int>(std::lround((to - from) / shape.step)) + 1);
        const std::int32_t number = Random::DrawRand(moment.object, ++moment.salt);
        return static_cast<float>(from) + static_cast<float>(number % count) * static_cast<float>(shape.step);
    }
    }
    return static_cast<float>(from);
}

} // namespace

float ShapeAtClock(const Shape& shape, double time)
{
    const double from = shape.from;
    const double to = shape.to;
    switch (shape.kind)
    {
    case ShapeKind::Wave:
    {
        const auto amplitude = static_cast<float>((to - from) / 2.0);
        const auto middle = static_cast<float>((to + from) / 2.0);
        return sinf(Angle(2.0 * Pi, shape.seconds, time)) * amplitude + middle;
    }
    case ShapeKind::Bounce:
        return static_cast<float>(from) + fabsf(sinf(Angle(Pi, shape.seconds, time))) * static_cast<float>(to - from);
    case ShapeKind::Scroll:
    {
        const auto rate = static_cast<float>(shape.perSecond / 1000.0);
        return static_cast<float>(time * static_cast<double>(rate));
    }
    case ShapeKind::Ramp:
    {
        const int period = std::max(1, static_cast<int>(Milliseconds(shape.seconds)));
        const auto rate = static_cast<float>((to - from) / period);
        return static_cast<float>(from) + static_cast<float>(static_cast<long long>(time) % period) * rate;
    }
    case ShapeKind::Zigzag:
    {
        // As the code wrote it: the clock in steps of the rise, folded back at the top. A falling zigzag folds the
        // same way downwards, and one that neither rises nor falls stays at `from`.
        const double rise = std::fabs(to - from);
        if (rise == 0.0)
        {
            return static_cast<float>(from);
        }
        const double rate = rise / std::max(1.0, Milliseconds(shape.seconds));
        const auto at = static_cast<float>(std::fmod(time * rate, 2.0 * rise));
        const auto up = static_cast<float>(rise);
        const float folded = at < up ? at : static_cast<float>(2.0 * rise) - at;
        return static_cast<float>(from) + (to < from ? -folded : folded);
    }
    case ShapeKind::Frames:
    {
        const double each = std::max(1.0, Milliseconds(shape.seconds));
        const int frame = static_cast<int>(std::floor(time / each)) % std::max(1, shape.count);
        return static_cast<float>(frame) * static_cast<float>(shape.step);
    }
    case ShapeKind::Random:
    case ShapeKind::Turn:
        break;
    }
    return static_cast<float>(from);
}

namespace
{
float NumberAt(const Number& number, Moment& moment)
{
    if (const Shape* shape = std::get_if<Shape>(&number))
    {
        return ValueAt(*shape, moment);
    }
    return std::get<float>(number);
}

float ValueOf(const Value& value, float item, float drawing, Moment& moment)
{
    if (const float* number = std::get_if<float>(&value))
    {
        return *number;
    }
    if (const Shape* shape = std::get_if<Shape>(&value))
    {
        return shape->timesItem ? item * ValueAt(*shape, moment) : ValueAt(*shape, moment);
    }
    return std::holds_alternative<FromDrawing>(value) ? drawing : item;
}

int GlowMeshOf(const GlowMesh& mesh, int item)
{
    if (const int* number = std::get_if<int>(&mesh))
    {
        return *number;
    }
    if (std::holds_alternative<AllGlowMeshes>(mesh))
    {
        // ResolveMaterial draws every mesh as a glowing one below -1.
        return -2;
    }
    return std::holds_alternative<NoGlowMesh>(mesh) ? -1 : item;
}

// "Item/InGameShop/membership_item_red.jpg" as the bitmaps name their files.
std::wstring BitmapFileName(const std::string& file)
{
    std::wstring name = L"Data\\";
    for (const char character : file)
    {
        name += character == '/' ? L'\\' : static_cast<wchar_t>(static_cast<unsigned char>(character));
    }
    return name;
}

// The texture a layer draws with: -1 for the mesh's own, nullopt when the layer names one that is missing.
std::optional<int> TextureOf(const Layer& layer)
{
    if (layer.texture.empty())
    {
        return -1;
    }
    const auto found = s_textures.find(layer.texture);
    if (found == s_textures.end() || found->second == BITMAP_UNKNOWN)
    {
        return std::nullopt;
    }
    return static_cast<int>(found->second);
}

// What a layer's draws are drawn with in one drawing.
struct LayerDraw
{
    int texture = -1;
    int flags = 0;
    float opacity = 1.f;
    int glowMesh = -1;
    float glowStrength = 0.f;
    float u = 0.f;
    float v = 0.f;
};

// The layer's draws at this moment; nullopt when it names a texture that is missing.
std::optional<LayerDraw> DrawOf(const Layer& layer, const DrawLook& look, float alpha, int renderType, Moment& moment)
{
    const std::optional<int> texture = TextureOf(layer);
    if (!texture)
    {
        return std::nullopt;
    }
    LayerDraw draw{.texture = *texture, .flags = LayerFlags(layer, renderType)};
    draw.opacity = ValueOf(layer.opacity, look.alpha, alpha, moment);
    draw.glowMesh = GlowMeshOf(layer.glowMesh, look.blendMesh);
    draw.glowStrength = ValueOf(layer.glowStrength, look.blendMeshLight, look.blendMeshLight, moment);
    draw.u = ValueOf(layer.scrollU, look.blendMeshU, look.blendMeshU, moment);
    draw.v = ValueOf(layer.scrollV, look.blendMeshV, look.blendMeshV, moment);
    return draw;
}

void DrawMeshes(const Layer& layer, BMD* b, const DrawLook& look, const LayerDraw& draw)
{
    if (layer.meshes.none)
    {
        return;
    }
    const std::vector<int>& meshes = layer.meshes.meshes;
    if (layer.meshes.IsAll())
    {
        b->RenderBody(draw.flags, draw.opacity, draw.glowMesh, draw.glowStrength, draw.u, draw.v,
                      layer.meshes.evenHidden ? -1 : look.hiddenMesh, draw.texture);
        return;
    }
    if (layer.meshes.allBut && meshes.size() == 1)
    {
        b->RenderBody(draw.flags, draw.opacity, draw.glowMesh, draw.glowStrength, draw.u, draw.v, meshes.front(),
                      draw.texture);
        return;
    }
    if (layer.meshes.allBut)
    {
        // RenderBody leaving out several meshes.
        b->BeginRender(draw.opacity);
        for (int mesh = 0; mesh < b->NumMeshs; ++mesh)
        {
            if (std::ranges::find(meshes, mesh) == meshes.end() && !b->IsMeshHidden(mesh, -1))
            {
                b->RenderScriptedMesh(mesh, draw.flags, draw.opacity, draw.glowMesh, draw.glowStrength, draw.u, draw.v,
                                      draw.texture);
            }
        }
        b->EndRender();
        return;
    }
    for (const int mesh : meshes)
    {
        b->RenderMesh(mesh, draw.flags, draw.opacity, draw.glowMesh, draw.glowStrength, draw.u, draw.v, draw.texture);
    }
}

void DrawLayer(const Layer& layer, BMD* b, const DrawLook& look, float alpha, int renderType, Moment& moment)
{
    const std::optional<LayerDraw> draw = DrawOf(layer, look, alpha, renderType, moment);
    if (!draw)
    {
        return;
    }
    const int scrolling = b->StreamMesh;
    b->StreamMesh = layer.scrollingMesh.value_or(scrolling);
    DrawMeshes(layer, b, look, *draw);
    b->StreamMesh = scrolling;
}

// Pixels the alpha test drops (BITMAP_t::hasHoles) in the texture of a mesh.
bool TextureHasHoles(const BMD* b, int mesh)
{
    const BITMAP_t* texture = Bitmaps.GetTexture(b->IndexTexture[b->Meshs[mesh].Texture]);
    return texture != nullptr && texture->hasHoles;
}

// Whether a layer draws the mesh in a look drawn mesh by mesh: every mesh and every mesh but some by RenderBody's rules
// (texture scripts, the item's hidden mesh), or a mesh the layer names.
bool DrawsMesh(const Layer& layer, const BMD* b, int mesh, const DrawLook& look)
{
    if (layer.meshes.none)
    {
        return false;
    }
    const std::vector<int>& meshes = layer.meshes.meshes;
    if (layer.meshes.IsAll())
    {
        return !b->IsMeshHidden(mesh, layer.meshes.evenHidden ? -1 : look.hiddenMesh);
    }
    const bool named = std::ranges::find(meshes, mesh) != meshes.end();
    if (layer.meshes.allBut)
    {
        return !named && !b->IsMeshHidden(mesh, -1);
    }
    return named;
}

// Whether the look is for this drawing (its Not for, design L11).
bool IsFor(const NotFor& notFor, int renderType)
{
    if (notFor.doppelganger && (renderType & RENDER_DOPPELGANGER) != 0)
    {
        return false;
    }
    return !notFor.outsideMonsterHands || (renderType & RENDER_EXTRA) != 0;
}

bool HasBuff(LookBuff buff, const OBJECT* o)
{
    switch (buff)
    {
    case LookBuff::InfinityArrow:
        return o != nullptr && g_isCharacterBuff(o, eBuff_InfinityArrow);
    }
    return false;
}

int s_inventoryDrawings = 0;

bool AtPlace(Place place, const BMD* b)
{
    switch (place)
    {
    case Place::Worn:
        return !b->HideSkin;
    case Place::InventoryAndGround:
        return b->HideSkin;
    case Place::Inventory:
        return b->HideSkin && s_inventoryDrawings > 0;
    case Place::Ground:
        return b->HideSkin && s_inventoryDrawings == 0;
    }
    return false;
}

// Whether a layer's Only when holds in this drawing (design L11).
bool Holds(const OnlyWhen& when, const BMD* b, const OBJECT* o, const DrawLook& look)
{
    if (when.place && !AtPlace(*when.place, b))
    {
        return false;
    }
    if (when.level && (look.level < when.level->first || look.level > when.level->second))
    {
        return false;
    }
    if (when.hiddenMesh && look.hiddenMesh != *when.hiddenMesh)
    {
        return false;
    }
    if (when.withoutBuff && HasBuff(*when.withoutBuff, o))
    {
        return false;
    }
    return !when.buff || HasBuff(*when.buff, o);
}

// A light of the look at this moment, from the light the drawing handed the look.
std::array<float, 3> LightOf(const LayerLight& light, const vec3_t modelLight, Moment& moment)
{
    std::array<float, 3> color{};
    if (light.brightness)
    {
        // As the code scaled it: the light times the brightness over its brightest channel.
        const float brightness = NumberAt(*light.brightness, moment);
        const float brightest = std::max({modelLight[0], modelLight[1], modelLight[2]});
        for (int channel = 0; channel < 3; ++channel)
        {
            color[channel] = brightest > 0.f ? modelLight[channel] * (brightness / brightest) : brightness;
        }
        return color;
    }
    for (int channel = 0; channel < 3; ++channel)
    {
        const float value = NumberAt(light.color[channel], moment);
        color[channel] = light.tint ? value * modelLight[channel] : value;
    }
    return color;
}

// Puts a light of the look on the model.
void LightLayer(const LayerLight& light, const vec3_t modelLight, BMD* b, Moment& moment)
{
    const std::array<float, 3> color = LightOf(light, modelLight, moment);
    Vector(color[0], color[1], color[2], b->BodyLight);
}

// A layer of a look drawn mesh by mesh, its light and values taken once, in the order the layers take them. A layer
// that draws nothing (no texture, the kind "none") still has its light.
struct MeshPass
{
    const Layer* layer = nullptr;
    std::optional<std::array<float, 3>> light;
    std::optional<LayerDraw> draw;
    // The light the layer finds, as the layers before it leave it.
    std::array<float, 3> found{};
};

// Draws a look with a layer under the holes mesh by mesh, as RenderBody draws a model (design L8): on a mesh whose
// texture has holes, the layers under the holes first, solid and on both faces, then the others; on the other meshes
// the layers in their order. Each mesh starts in the light the drawing handed the look, and a layer finds the light the
// layers before it left, as in a look drawn layer by layer: a layer's own light stays for the layers after it also on
// a mesh it doesn't draw. A layer under the holes leaves the light as it found it, also where it goes over: its own
// light and the light a faded glowing pass dims are for its draw only.
void DrawMeshByMesh(const LookDefinition& style, BMD* b, const OBJECT* o, const DrawLook& look, float alpha,
                    int renderType, const vec3_t modelLight, Moment& moment)
{
    // Kept between draws, so a draw takes no memory once the longest look was drawn.
    static std::vector<MeshPass> passes;
    passes.clear();
    std::array<float, 3> found{modelLight[0], modelLight[1], modelLight[2]};
    for (const Layer& layer : style.layers)
    {
        if (!Holds(layer.onlyWhen, b, o, look))
        {
            continue;
        }
        MeshPass pass{.layer = &layer, .found = found};
        if (layer.light)
        {
            pass.light = LightOf(*layer.light, modelLight, moment);
            if (layer.where != Where::UnderHoles)
            {
                found = *pass.light;
            }
        }
        pass.draw = DrawOf(layer, look, alpha, renderType, moment);
        if (pass.draw || (pass.light && layer.where != Where::UnderHoles))
        {
            passes.push_back(pass);
        }
    }
    const auto setLight = [b](const std::array<float, 3>& light)
    { Vector(light[0], light[1], light[2], b->BodyLight); };
    const auto drawPass = [b, &setLight](const MeshPass& pass, int mesh, int flags)
    {
        if (pass.light)
        {
            setLight(*pass.light);
        }
        const int scrolling = b->StreamMesh;
        b->StreamMesh = pass.layer->scrollingMesh.value_or(scrolling);
        const LayerDraw& draw = *pass.draw;
        b->RenderScriptedMesh(mesh, flags, draw.opacity, draw.glowMesh, draw.glowStrength, draw.u, draw.v,
                              draw.texture);
        b->StreamMesh = scrolling;
    };
    const auto drawKeepingLight = [b, &drawPass](const MeshPass& pass, int mesh, int flags)
    {
        vec3_t light;
        VectorCopy(b->BodyLight, light);
        drawPass(pass, mesh, flags);
        VectorCopy(light, b->BodyLight);
    };
    const bool seeThrough = look.alpha < 0.99f;
    b->BeginRender(look.alpha);
    for (int mesh = 0; mesh < b->NumMeshs; ++mesh)
    {
        const bool holes = !seeThrough && TextureHasHoles(b, mesh);
        // A glowing mesh, and every mesh below a glow mesh of -1 (all), adds its light: the layer goes over it.
        const auto goesUnder = [b, mesh, holes](const MeshPass& pass)
        {
            return holes && pass.draw && pass.layer->where == Where::UnderHoles && pass.draw->glowMesh >= -1 &&
                   !b->IsMeshGlowing(mesh, pass.draw->glowMesh);
        };
        const auto draws = [b, mesh, &look](const MeshPass& pass)
        { return pass.draw && DrawsMesh(*pass.layer, b, mesh, look); };
        for (const MeshPass& pass : passes)
        {
            if (goesUnder(pass) && draws(pass))
            {
                // Solid: without its blend, in the light the layers before it leave.
                setLight(pass.found);
                drawKeepingLight(pass, mesh, MotionFlag(pass.layer->motion) | RENDER_TWOSIDED);
            }
        }
        setLight({modelLight[0], modelLight[1], modelLight[2]});
        for (const MeshPass& pass : passes)
        {
            if (goesUnder(pass))
            {
                continue;
            }
            if (!draws(pass))
            {
                // Its light stays for the layers after it, as on the meshes it draws.
                if (pass.light && pass.layer->where != Where::UnderHoles)
                {
                    setLight(*pass.light);
                }
                continue;
            }
            if (pass.layer->where == Where::UnderHoles)
            {
                drawKeepingLight(pass, mesh, pass.draw->flags);
            }
            else
            {
                drawPass(pass, mesh, pass.draw->flags);
            }
        }
    }
    b->EndRender();
}

void LeaveItemValues(const ItemValues& values, const vec3_t modelLight, BMD* b, DrawLook& look, Moment& moment)
{
    const auto leave = [&moment](const std::optional<Number>& value, float& field)
    {
        if (value)
        {
            field = NumberAt(*value, moment);
        }
    };
    leave(values.opacity, look.alpha);
    look.blendMesh = values.glowMesh.value_or(look.blendMesh);
    leave(values.glowStrength, look.blendMeshLight);
    leave(values.scrollU, look.blendMeshU);
    leave(values.scrollV, look.blendMeshV);
    look.hiddenMesh = values.hiddenMesh.value_or(look.hiddenMesh);
    b->StreamMesh = values.scrollingMesh.value_or(b->StreamMesh);
    if (values.light)
    {
        LightLayer(*values.light, modelLight, b, moment);
    }
}
// Finds the texture's index, loading it when no code loaded it. A missing one is logged when `logMissing` and it wasn't
// missing before.
void PrepareTexture(const std::string& look, const std::string& texture, bool logMissing)
{
    const auto known = s_textures.find(texture);
    const bool missingBefore = known != s_textures.end() && known->second == BITMAP_UNKNOWN;
    // A texture the game loaded already (an item effect texture) keeps its index.
    const std::wstring file = BitmapFileName(texture);
    const BITMAP_t* loaded = Bitmaps.FindTexture(file);
    const GLuint index = loaded != nullptr ? loaded->BitmapIndex : Bitmaps.LoadImage(file, GL_LINEAR, GL_REPEAT);
    if (index == BITMAP_UNKNOWN && logMissing && !missingBefore)
    {
        MU_LOG_ERROR(mu::log::Get("data"),
                     "Render style data error: {}: texture \"{}\" is not found; the layer is skipped", look, texture);
    }
    s_textures[texture] = index;
}

// The textures of one look that are not found yet: new names, and names missing before, whose file may have come since.
void PrepareMissingTextures(const LookDefinition& style, bool logMissing)
{
    for (const Layer& layer : style.layers)
    {
        if (layer.texture.empty())
        {
            continue;
        }
        const auto known = s_textures.find(layer.texture);
        if (known == s_textures.end() || known->second == BITMAP_UNKNOWN)
        {
            PrepareTexture(style.name, layer.texture, logMissing);
        }
    }
}
} // namespace

bool DrawRenderStyle(const LookDefinition& style, BMD* b, const OBJECT* o, DrawLook& look, float alpha, int renderType)
{
    if (!IsFor(style.notFor, renderType))
    {
        return false;
    }
    // A look only for a monster's hands has the mark as its condition, not as a part of its draws.
    if (style.notFor.outsideMonsterHands)
    {
        renderType &= ~RENDER_EXTRA;
    }
    Moment moment{.object = o};
    vec3_t modelLight;
    VectorCopy(b->BodyLight, modelLight);
    const bool meshByMesh =
        std::ranges::any_of(style.layers, [b, o, &look](const Layer& layer)
                            { return layer.where == Where::UnderHoles && Holds(layer.onlyWhen, b, o, look); });
    if (meshByMesh)
    {
        DrawMeshByMesh(style, b, o, look, alpha, renderType, modelLight, moment);
    }
    else
    {
        // A layer without a light of its own keeps the one on the model, as the looks in code drew.
        for (const Layer& layer : style.layers)
        {
            if (!Holds(layer.onlyWhen, b, o, look))
            {
                continue;
            }
            if (layer.light)
            {
                LightLayer(*layer.light, modelLight, b, moment);
            }
            DrawLayer(layer, b, look, alpha, renderType, moment);
        }
    }
    LeaveItemValues(style.itemValuesAfter, modelLight, b, look, moment);
    return true;
}

InventoryDrawing::InventoryDrawing()
{
    ++s_inventoryDrawings;
}

InventoryDrawing::~InventoryDrawing()
{
    --s_inventoryDrawings;
}

bool InventoryDrawing::Active()
{
    return s_inventoryDrawings > 0;
}

void ApplyItemEffect(const LookDefinition& effect, BMD* b, const OBJECT* o, DrawLook& look, float& alpha, int& level)
{
    Moment moment{.object = o};
    vec3_t modelLight;
    VectorCopy(b->BodyLight, modelLight);
    for (const ItemValuesBefore& entry : effect.itemValuesBefore)
    {
        look.level = level;
        if (!Holds(entry.onlyWhen, b, o, look))
        {
            continue;
        }
        LeaveItemValues(entry.values, modelLight, b, look, moment);
        if (entry.drawingOpacity)
        {
            alpha = NumberAt(*entry.drawingOpacity, moment);
        }
        if (entry.glowsLikeLevel)
        {
            level = *entry.glowsLikeLevel;
        }
    }
    look.level = level;
}

void SpawnItemEffect(const LookDefinition& effect, OBJECT* o)
{
    if (!effect.objectLight)
    {
        return;
    }
    Moment moment{.object = o};
    for (int channel = 0; channel < 3; ++channel)
    {
        o->Light[channel] = NumberAt((*effect.objectLight)[channel], moment);
    }
}

void PrepareTextures()
{
    for (const LookDefinition& style : g_LookStore.GetRenderStyles())
    {
        for (const Layer& layer : style.layers)
        {
            if (!layer.texture.empty())
            {
                PrepareTexture(style.name, layer.texture, true);
            }
        }
    }
}

void PrepareTextures(const LookDefinition& style)
{
    PrepareMissingTextures(style, true);
}

void PrepareDraftTextures(const LookDefinition& style)
{
    PrepareMissingTextures(style, false);
}
int RenderFlagOf(Motion motion)
{
    return MotionFlag(motion);
}
} // namespace Render::Items::Looks
