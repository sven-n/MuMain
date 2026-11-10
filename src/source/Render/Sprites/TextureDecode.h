#pragma once

#include "Render/Sprites/GlobalBitmap.h"
#include "Render/Textures/MipChain.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace Render::Sprites
{
// One texture file to load, with everything its decode needs copied in.
struct TextureDecodeRequest
{
    GLuint index = 0;
    // The name the bitmap is known by (.jpg or .tga) and the file read for it (.OZJ or .OZT).
    std::wstring filename;
    std::wstring path;
    bool jpeg = false;
    GLuint filter = GL_NEAREST;
    GLuint wrapMode = GL_CLAMP_TO_EDGE;
    bool worldTexture = false;
    bool padToPowerOfTwo = true;
    bool premultiplyAlpha = false;
    bool keepPixels = false;
    // Texture filtering was on when the load was asked for: world textures get every mip level.
    bool filteringEnabled = false;
};

// The levels below the image: built on the CPU (`chain`) or by the GPU (`generate`).
struct MipLevels
{
    std::vector<Textures::MipLevel> chain;
    bool generate = false;
};

// A decoded texture, ready to upload.
struct DecodedTexture
{
    std::unique_ptr<BITMAP_t> bitmap;
    // The RGBA pixels to upload; empty when they are the bitmap's own buffer.
    std::vector<std::uint8_t> rgba;
    int uploadWidth = 0;
    int uploadHeight = 0;
    MipLevels mips;

    std::span<const std::uint8_t> UploadPixels() const
    {
        return rgba.empty() ? std::span<const std::uint8_t>(bitmap->BufferStorage)
                            : std::span<const std::uint8_t>(rgba);
    }
};

struct TextureDecodeResult
{
    std::optional<DecodedTexture> texture;
    // Lines for the error report, in the order they happened.
    std::vector<std::wstring> errors;
    double readSeconds = 0.0;
    double mipSeconds = 0.0;
};

// Reads and decodes a texture file: the pack's copy first, then the original
// when the pack has none or its copy does not decode. It touches no shared
// state, so it can run on any thread.
TextureDecodeResult DecodeTextureFile(const Textures::TexturePackFiles& pack, const TextureDecodeRequest& request);
} // namespace Render::Sprites
