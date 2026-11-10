#pragma once

#include "Render/Sprites/GlobalBitmap.h"
#include "Render/Sprites/TextureDecodePool.h"
#include "Render/Textures/MipChain.h"

#include <atomic>
#include <cstdint>
#include <deque>
#include <future>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
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
    // Told about a failed load, when set.
    CGlobalBitmap::LoadFailureReporter reportFailure = nullptr;
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

// The memory a decoded texture's pixels hold until it commits.
std::size_t DecodedBytes(const TextureDecodeResult& result);

// Whether the file a load reads is there, in the pack or in Data, without reading it.
bool TextureFileExists(const Textures::TexturePackFiles& pack, const std::wstring& path);

// The loads of a load batch (TextureLoadBatch), decoding on the pool until
// they are committed.
struct PendingTextureLoads
{
    struct Load
    {
        TextureDecodeRequest request;
        std::future<TextureDecodeResult> result;
    };
    std::unordered_map<GLuint, Load> loads;
    // The order they were asked for, which is the order they commit in; an
    // index committed early is skipped.
    std::deque<GLuint> order;
    std::size_t queued = 0;
    unsigned threads = TextureDecodePool::DefaultThreadCount();
    // The pixels of finished decodes still waiting to commit, which bounds how far loads may run ahead.
    std::atomic<std::size_t> decodedBytes = 0;
    // Started with the first queued load; declared last, so its workers stop before the rest goes.
    std::optional<TextureDecodePool> pool;
};
} // namespace Render::Sprites
