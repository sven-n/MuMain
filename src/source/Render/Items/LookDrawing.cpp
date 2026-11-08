#include "stdafx.h"

#include "LookDrawing.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Core/Utilities/Random.h"
#include "Data/GameData/Common/MeshLimits.h"
#include "Data/GameData/LookData/LookStore.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "Render/Textures/ZzzOpenglUtil.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <unordered_map>

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
    const int flags = layer.kind == LayerKind::Texture ? RENDER_TEXTURE : MotionFlag(layer.motion);
    return layer.blend == Blend::Glow ? flags | RENDER_BRIGHT : flags;
}

// The object of one drawing, which random values are drawn for; each random value of a look takes the next salt.
struct Moment
{
    const OBJECT* object = nullptr;
    std::uint32_t salt = 0;
};

// The angle at this moment of a sine that turns `radiansPerCycle` in `seconds`, as the looks in code wrote it:
// WorldTime times a float constant.
float Angle(double radiansPerCycle, double seconds)
{
    const auto rate = static_cast<float>(radiansPerCycle / (seconds * 1000.0));
    return static_cast<float>(WorldTime * static_cast<double>(rate));
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
    {
        const auto amplitude = static_cast<float>((to - from) / 2.0);
        const auto middle = static_cast<float>((to + from) / 2.0);
        return sinf(Angle(2.0 * Pi, shape.seconds)) * amplitude + middle;
    }
    case ShapeKind::Bounce:
        return static_cast<float>(from) + fabsf(sinf(Angle(Pi, shape.seconds))) * static_cast<float>(to - from);
    case ShapeKind::Scroll:
    {
        const auto rate = static_cast<float>(shape.perSecond / 1000.0);
        return static_cast<float>(WorldTime * static_cast<double>(rate));
    }
    case ShapeKind::Ramp:
    {
        const int period = std::max(1, static_cast<int>(Milliseconds(shape.seconds)));
        const auto rate = static_cast<float>((to - from) / period);
        return static_cast<float>(from) + static_cast<float>(static_cast<int>(WorldTime) % period) * rate;
    }
    case ShapeKind::Frames:
    {
        const double each = std::max(1.0, Milliseconds(shape.seconds));
        const int frame = static_cast<int>(std::floor(WorldTime / each)) % std::max(1, shape.count);
        return static_cast<float>(frame) * static_cast<float>(shape.step);
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
        return ValueAt(*shape, moment);
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

void DrawMeshes(const Layer& layer, BMD* b, const DrawLook& look, int texture, int flags, float opacity, int glowMesh,
                float glowStrength, float u, float v)
{
    const std::vector<int>& meshes = layer.meshes.meshes;
    if (layer.meshes.IsAll())
    {
        b->RenderBody(flags, opacity, glowMesh, glowStrength, u, v, layer.meshes.evenHidden ? -1 : look.hiddenMesh,
                      texture);
        return;
    }
    if (layer.meshes.allBut && meshes.size() == 1)
    {
        b->RenderBody(flags, opacity, glowMesh, glowStrength, u, v, meshes.front(), texture);
        return;
    }
    if (layer.meshes.allBut)
    {
        // RenderBody leaving out several meshes.
        b->BeginRender(opacity);
        for (int mesh = 0; mesh < b->NumMeshs; ++mesh)
        {
            if (std::ranges::find(meshes, mesh) == meshes.end() && !b->IsMeshHidden(mesh, -1))
            {
                b->RenderScriptedMesh(mesh, flags, opacity, glowMesh, glowStrength, u, v, texture);
            }
        }
        b->EndRender();
        return;
    }
    for (const int mesh : meshes)
    {
        b->RenderMesh(mesh, flags, opacity, glowMesh, glowStrength, u, v, texture);
    }
}

void DrawLayer(const Layer& layer, BMD* b, const DrawLook& look, float alpha, int renderType, Moment& moment)
{
    const std::optional<int> texture = TextureOf(layer);
    if (!texture)
    {
        return;
    }
    const int flags = LayerFlags(layer, renderType);
    const float opacity = ValueOf(layer.opacity, look.alpha, alpha, moment);
    const int glowMesh = GlowMeshOf(layer.glowMesh, look.blendMesh);
    const float glowStrength = ValueOf(layer.glowStrength, look.blendMeshLight, look.blendMeshLight, moment);
    const float u = ValueOf(layer.scrollU, look.blendMeshU, look.blendMeshU, moment);
    const float v = ValueOf(layer.scrollV, look.blendMeshV, look.blendMeshV, moment);
    if (!layer.scrollingMesh)
    {
        DrawMeshes(layer, b, look, *texture, flags, opacity, glowMesh, glowStrength, u, v);
        return;
    }
    const int scrolling = b->StreamMesh;
    b->StreamMesh = *layer.scrollingMesh;
    DrawMeshes(layer, b, look, *texture, flags, opacity, glowMesh, glowStrength, u, v);
    b->StreamMesh = scrolling;
}

// Puts the layer's light on the model, from the light the drawing handed the look.
void LightLayer(const LayerLight& light, const vec3_t modelLight, BMD* b, Moment& moment)
{
    for (int channel = 0; channel < 3; ++channel)
    {
        const float color = NumberAt(light.color[channel], moment);
        b->BodyLight[channel] = light.tint ? color * modelLight[channel] : color;
    }
}

void LeaveItemValues(const ItemValues& values, BMD* b, DrawLook& look, Moment& moment)
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

void DrawRenderStyle(const LookDefinition& style, BMD* b, const OBJECT* o, DrawLook& look, float alpha, int renderType)
{
    Moment moment{.object = o};
    vec3_t modelLight;
    VectorCopy(b->BodyLight, modelLight);
    // A layer without a light of its own keeps the one on the model, as the looks in code drew.
    for (const Layer& layer : style.layers)
    {
        if (layer.light)
        {
            LightLayer(*layer.light, modelLight, b, moment);
        }
        DrawLayer(layer, b, look, alpha, renderType, moment);
    }
    LeaveItemValues(style.itemValuesAfter, b, look, moment);
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
} // namespace Render::Items::Looks
