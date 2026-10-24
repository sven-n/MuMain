#include "stdafx.h"

#include "Render/Models/Gltf/EmbeddedTextures.h"

#include "Core/Text/Utf8.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Render/Models/Gltf/SkinnedModel.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Sprites/BitmapTextureLimits.h"
#include "Render/Sprites/GlobalBitmap.h"

#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_surface.h>

#include <cstring>
#include <memory>

namespace Render::Models::Gltf
{
namespace
{
constexpr const char* PngMimeType = "image/png";

struct SurfaceDeleter
{
    void operator()(SDL_Surface* surface) const
    {
        SDL_DestroySurface(surface);
    }
};
using SurfacePointer = std::unique_ptr<SDL_Surface, SurfaceDeleter>;
} // namespace

bool DecodeEmbeddedImage(const EmbeddedImage& image, DecodedImage& decoded, std::string& reason)
{
    if (image.mimeType != PngMimeType)
    {
        reason = "its type \"" + image.mimeType + "\" is not " + PngMimeType;
        return false;
    }
    SDL_IOStream* stream = SDL_IOFromConstMem(image.bytes.data(), image.bytes.size());
    const SurfacePointer loaded{stream ? SDL_LoadPNG_IO(stream, true) : nullptr};
    if (!loaded)
    {
        reason = std::string("it is no readable PNG: ") + SDL_GetError();
        return false;
    }
    if (loaded->w > Render::Sprites::kModelTextureMaximumWidth ||
        loaded->h > Render::Sprites::kModelTextureMaximumHeight)
    {
        reason = "it is " + std::to_string(loaded->w) + "x" + std::to_string(loaded->h) + ", above the " +
                 std::to_string(Render::Sprites::kModelTextureMaximumWidth) + "x" +
                 std::to_string(Render::Sprites::kModelTextureMaximumHeight) + " texture limit";
        return false;
    }
    const SurfacePointer rgba{SDL_ConvertSurface(loaded.get(), SDL_PIXELFORMAT_RGBA32)};
    if (!rgba)
    {
        reason = std::string("it cannot be converted to RGBA: ") + SDL_GetError();
        return false;
    }
    decoded.width = rgba->w;
    decoded.height = rgba->h;
    decoded.hasAlpha = SDL_ISPIXELFORMAT_ALPHA(loaded->format);
    const std::size_t rowBytes = static_cast<std::size_t>(rgba->w) * 4u;
    decoded.rgba.resize(rowBytes * static_cast<std::size_t>(rgba->h));
    for (int row = 0; row < rgba->h; ++row)
    {
        std::memcpy(&decoded.rgba[rowBytes * static_cast<std::size_t>(row)],
                    static_cast<const std::uint8_t*>(rgba->pixels) + static_cast<std::size_t>(row) * rgba->pitch,
                    rowBytes);
    }
    return true;
}

namespace
{
// The images of a model loaded as textures on first use: each image once.
class EmbeddedTextureLoader
{
public:
    EmbeddedTextureLoader(const SkinnedModel& model, const std::wstring& modelFile, int wrap, int filter)
        : model_(model), modelFile_(modelFile), wrap_(wrap), filter_(filter),
          loaded_(model.images.size(), BITMAP_UNKNOWN), tried_(model.images.size(), false)
    {
    }

    // The texture of image `image` (-1: none), BITMAP_UNKNOWN when it cannot be used (logged once).
    GLuint Texture(int image)
    {
        if (image < 0 || image >= static_cast<int>(loaded_.size()))
        {
            return BITMAP_UNKNOWN;
        }
        if (!tried_[image])
        {
            tried_[image] = true;
            loaded_[image] = Load(model_.images[image]);
        }
        return loaded_[image];
    }

private:
    GLuint Load(const EmbeddedImage& embedded) const
    {
        DecodedImage decoded;
        std::string reason;
        if (!DecodeEmbeddedImage(embedded, decoded, reason))
        {
            MU_LOG_WARN(mu::log::Get("render"), "Image {} of {} is not used, {}; its meshes keep their texture.",
                        embedded.name, Core::Text::ToUtf8(modelFile_.c_str()), reason);
            return BITMAP_UNKNOWN;
        }
        const std::wstring name = modelFile_ + L"#" + Core::Text::FromUtf8(embedded.name);
        const GLuint texture =
            Bitmaps.LoadImagePixels(name, decoded.rgba, decoded.width, decoded.height, decoded.hasAlpha,
                                    static_cast<GLuint>(filter_), static_cast<GLuint>(wrap_));
        if (texture == BITMAP_UNKNOWN)
        {
            MU_LOG_WARN(mu::log::Get("render"), "Image {} of {} could not be loaded as texture.", embedded.name,
                        Core::Text::ToUtf8(modelFile_.c_str()));
        }
        return texture;
    }

    const SkinnedModel& model_;
    const std::wstring& modelFile_;
    int wrap_;
    int filter_;
    std::vector<GLuint> loaded_;
    std::vector<bool> tried_;
};

// The texture of a coarser LOD's mesh without a usable image of its own: that
// of the BMD mesh with the same texture name, else of the BMD mesh of its index.
GLuint LodFallbackTexture(const BMD& model, const LodMesh& mesh, int index)
{
    for (int i = 0; i < model.NumMeshs; ++i)
    {
        if (!mesh.texture.empty() && mesh.texture == model.Textures[i].FileName)
        {
            return model.IndexTexture[i];
        }
    }
    return index < model.NumMeshs ? model.IndexTexture[index] : BITMAP_UNKNOWN;
}
} // namespace

int OpenEmbeddedTextures(BMD& model, const std::wstring& modelFile, int wrap, int filter)
{
    const SkinnedModel* skinned = SkinnedModelOf(model);
    if (!skinned)
    {
        return 0;
    }
    EmbeddedTextureLoader loader(*skinned, modelFile, wrap, filter);
    int textured = 0;
    for (int mesh = 0; mesh < model.NumMeshs && mesh < static_cast<int>(skinned->meshImages.size()); ++mesh)
    {
        const GLuint texture = loader.Texture(skinned->meshImages[mesh]);
        if (texture != BITMAP_UNKNOWN)
        {
            model.IndexTexture[mesh] = texture;
            ++textured;
        }
    }
    skinned->meshNormalTextures.assign(skinned->meshNormalMaps.size(), BITMAP_UNKNOWN);
    for (size_t mesh = 0; mesh < skinned->meshNormalMaps.size(); ++mesh)
    {
        skinned->meshNormalTextures[mesh] = loader.Texture(skinned->meshNormalMaps[mesh].image);
    }
    skinned->lodTextures.resize(skinned->lods.size());
    skinned->lodNormalTextures.resize(skinned->lods.size());
    for (size_t lod = 0; lod < skinned->lods.size(); ++lod)
    {
        const std::vector<LodMesh>& meshes = skinned->lods[lod];
        std::vector<unsigned int>& textures = skinned->lodTextures[lod];
        std::vector<unsigned int>& normalTextures = skinned->lodNormalTextures[lod];
        textures.resize(meshes.size());
        normalTextures.resize(meshes.size());
        for (size_t mesh = 0; mesh < meshes.size(); ++mesh)
        {
            const GLuint texture = loader.Texture(meshes[mesh].image);
            textures[mesh] =
                texture != BITMAP_UNKNOWN ? texture : LodFallbackTexture(model, meshes[mesh], static_cast<int>(mesh));
            normalTextures[mesh] = loader.Texture(meshes[mesh].normalMap.image);
        }
    }
    return textured;
}
} // namespace Render::Models::Gltf
