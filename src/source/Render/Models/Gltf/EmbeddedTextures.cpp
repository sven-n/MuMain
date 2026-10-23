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
    if (loaded->w > Render::Sprites::kBitmapMaximumWidth || loaded->h > Render::Sprites::kBitmapMaximumHeight)
    {
        reason = "it is " + std::to_string(loaded->w) + "x" + std::to_string(loaded->h) + ", above the " +
                 std::to_string(Render::Sprites::kBitmapMaximumWidth) + "x" +
                 std::to_string(Render::Sprites::kBitmapMaximumHeight) + " texture limit";
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

int OpenEmbeddedTextures(BMD& model, const std::wstring& modelFile, int wrap, int filter)
{
    const SkinnedModel* skinned = SkinnedModelOf(model);
    if (!skinned)
    {
        return 0;
    }
    std::vector<GLuint> loaded(skinned->images.size(), BITMAP_UNKNOWN);
    std::vector<bool> tried(skinned->images.size(), false);
    int textured = 0;
    for (int mesh = 0; mesh < model.NumMeshs && mesh < static_cast<int>(skinned->meshImages.size()); ++mesh)
    {
        const int image = skinned->meshImages[mesh];
        if (image < 0)
        {
            continue;
        }
        if (!tried[image])
        {
            tried[image] = true;
            const EmbeddedImage& embedded = skinned->images[image];
            DecodedImage decoded;
            std::string reason;
            if (!DecodeEmbeddedImage(embedded, decoded, reason))
            {
                MU_LOG_WARN(mu::log::Get("render"), "Image {} of {} is not used, {}; its meshes keep their texture.",
                            embedded.name, Core::Text::ToUtf8(modelFile.c_str()), reason);
                continue;
            }
            const std::wstring name = modelFile + L"#" + Core::Text::FromUtf8(embedded.name);
            loaded[image] = Bitmaps.LoadImagePixels(name, decoded.rgba, decoded.width, decoded.height, decoded.hasAlpha,
                                                    static_cast<GLuint>(filter), static_cast<GLuint>(wrap));
            if (loaded[image] == BITMAP_UNKNOWN)
            {
                MU_LOG_WARN(mu::log::Get("render"), "Image {} of {} could not be loaded as texture.", embedded.name,
                            Core::Text::ToUtf8(modelFile.c_str()));
            }
        }
        if (loaded[image] != BITMAP_UNKNOWN)
        {
            model.IndexTexture[mesh] = loaded[image];
            ++textured;
        }
    }
    return textured;
}
} // namespace Render::Models::Gltf
