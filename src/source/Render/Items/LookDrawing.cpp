#include "stdafx.h"

#include "LookDrawing.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Data/GameData/LookData/LookStore.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Sprites/GlobalBitmap.h"

#include <algorithm>
#include <string>
#include <unordered_map>

namespace Render::Items::Looks
{
namespace
{
using namespace Data::Looks;

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

float ValueOf(const Value& value, float item, float drawing)
{
    if (const float* number = std::get_if<float>(&value))
    {
        return *number;
    }
    return std::holds_alternative<FromDrawing>(value) ? drawing : item;
}

int GlowMeshOf(const GlowMesh& mesh, int item)
{
    if (const int* number = std::get_if<int>(&mesh))
    {
        return *number;
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

void DrawLayer(const Layer& layer, BMD* b, const DrawLook& look, float alpha, int renderType)
{
    const std::optional<int> texture = TextureOf(layer);
    if (!texture)
    {
        return;
    }
    const int flags = LayerFlags(layer, renderType);
    const float opacity = ValueOf(layer.opacity, look.alpha, alpha);
    const int glowMesh = GlowMeshOf(layer.glowMesh, look.blendMesh);
    const float glowStrength = ValueOf(layer.glowStrength, look.blendMeshLight, look.blendMeshLight);
    const float u = ValueOf(layer.scrollU, look.blendMeshU, look.blendMeshU);
    const float v = ValueOf(layer.scrollV, look.blendMeshV, look.blendMeshV);
    const std::vector<int>& meshes = layer.meshes.meshes;
    if (layer.meshes.IsAll())
    {
        b->RenderBody(flags, opacity, glowMesh, glowStrength, u, v, look.hiddenMesh, *texture);
        return;
    }
    if (layer.meshes.allBut && meshes.size() == 1)
    {
        b->RenderBody(flags, opacity, glowMesh, glowStrength, u, v, meshes.front(), *texture);
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
                b->RenderScriptedMesh(mesh, flags, opacity, glowMesh, glowStrength, u, v, *texture);
            }
        }
        b->EndRender();
        return;
    }
    for (const int mesh : meshes)
    {
        b->RenderMesh(mesh, flags, opacity, glowMesh, glowStrength, u, v, *texture);
    }
}

void LeaveItemValues(const ItemValues& values, DrawLook& look)
{
    look.alpha = values.opacity.value_or(look.alpha);
    look.blendMesh = values.glowMesh.value_or(look.blendMesh);
    look.blendMeshLight = values.glowStrength.value_or(look.blendMeshLight);
    look.blendMeshU = values.scrollU.value_or(look.blendMeshU);
    look.blendMeshV = values.scrollV.value_or(look.blendMeshV);
    look.hiddenMesh = values.hiddenMesh.value_or(look.hiddenMesh);
}
} // namespace

void DrawRenderStyle(const LookDefinition& style, BMD* b, const OBJECT* /*o*/, DrawLook& look, float alpha,
                     int renderType)
{
    for (const Layer& layer : style.layers)
    {
        DrawLayer(layer, b, look, alpha, renderType);
    }
    LeaveItemValues(style.itemValuesAfter, look);
}

void PrepareTextures()
{
    for (const LookDefinition& style : g_LookStore.GetRenderStyles())
    {
        for (const Layer& layer : style.layers)
        {
            if (layer.texture.empty() || s_textures.contains(layer.texture))
            {
                continue;
            }
            const GLuint texture = Bitmaps.LoadImage(BitmapFileName(layer.texture), GL_LINEAR, GL_REPEAT);
            if (texture == BITMAP_UNKNOWN)
            {
                MU_LOG_ERROR(mu::log::Get("data"),
                             "Render style data error: {}: texture \"{}\" is not found; the layer is skipped",
                             style.name, layer.texture);
            }
            s_textures[layer.texture] = texture;
        }
    }
}
} // namespace Render::Items::Looks
