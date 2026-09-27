#include "stdafx.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "Render/Renderer/BitmapUpload.h"
#include "Render/Renderer/MuRenderer.h"

#include <doctest.h>
#include <turbojpeg.h>
#include <SDL3/SDL_gpu.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <span>
#include <string>
#include <vector>

namespace
{
struct UploadRecord
{
    std::vector<std::uint8_t> pixels;
    int width = 0;
    int height = 0;
    bool reject = false;
    int calls = 0;
    int releasedTextures = 0;
    std::map<std::uint32_t, void*> textures;
    std::map<std::uint32_t, void*> samplers;
};
UploadRecord record;

class TestRenderer : public mu::IMuRenderer
{
public:
    void RenderQuad2D(std::span<const mu::Vertex2D>, std::uint32_t) override {}
    void RenderTriangles(std::span<const mu::Vertex3D>, std::uint32_t) override {}
    void RenderQuad3D(std::span<const mu::Vertex3D>, std::uint32_t) override {}
    void RenderQuadStrip(std::span<const mu::Vertex3D>, std::uint32_t) override {}
    void SetBlendMode(mu::BlendMode) override {}
    void DisableBlend() override {}
    void SetDepthTest(bool) override {}
    void SetFog(const mu::FogParams&) override {}
    void BeginScene(int, int, int, int) override {}
    void EndScene() override {}
    void Begin2DPass() override {}
    void End2DPass() override {}
    void ClearScreen() override {}
    void RenderLines(std::span<const mu::Vertex3D>, std::uint32_t) override {}
    void* GetDevice() override
    {
        return reinterpret_cast<void*>(1);
    }
};

std::filesystem::path MakeFixtureDir(const char* name)
{
    const auto path = std::filesystem::current_path() / (std::string("bitmap_loader_") + name);
    std::filesystem::create_directories(path);
    return path;
}
void WriteTga(const std::filesystem::path& path)
{
    // OZT has sixteen prefix bytes followed by width/height, depth and pixels.
    std::array<std::uint8_t, 22 + 16> bytes{};
    bytes[16] = 2;
    bytes[18] = 2;
    bytes[20] = 32;
    for (std::size_t i = 0; i < 4; ++i)
    {
        bytes[22 + i * 4] = static_cast<std::uint8_t>(10 + i);
        bytes[23 + i * 4] = static_cast<std::uint8_t>(20 + i);
        bytes[24 + i * 4] = static_cast<std::uint8_t>(30 + i);
        bytes[25 + i * 4] = static_cast<std::uint8_t>(40 + i);
    }
    std::ofstream out(path, std::ios::binary);
    out.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}
void WriteJpeg(const std::filesystem::path& path)
{
    std::array<std::uint8_t, std::size_t{2} * 2 * 3> rgb{};
    rgb.fill(87);
    tjhandle handle = tjInitCompress();
    REQUIRE(handle != nullptr);
    unsigned char* compressed = nullptr;
    unsigned long length = 0;
    REQUIRE(tjCompress2(handle, rgb.data(), 2, 0, 2, TJPF_RGB, &compressed, &length, TJSAMP_444, 100, 0) == 0);
    std::ofstream out(path, std::ios::binary);
    std::array<char, 24> prefix{};
    out.write(prefix.data(), prefix.size());
    out.write(reinterpret_cast<const char*>(compressed), static_cast<std::streamsize>(length));
    tjFree(compressed);
    tjDestroy(handle);
}
} // namespace

namespace mu
{
IMuRenderer& GetRenderer()
{
    static TestRenderer renderer;
    return renderer;
}
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters): matches the production bridge signature.
Render::Renderer::BitmapGpuResources::Result UploadBitmapPixels(std::span<const std::uint8_t> pixels, int width,
                                                                int height, SDL_GPUFilter, SDL_GPUSamplerAddressMode)
{
    ++record.calls;
    record.pixels.assign(pixels.begin(), pixels.end());
    record.width = width;
    record.height = height;
    if (record.reject)
        return {nullptr, nullptr, "fixture reject", "rejected upload"};
    alignas(std::max_align_t) static std::array<std::byte, 64> textureStorage{};
    alignas(std::max_align_t) static std::array<std::byte, 64> samplerStorage{};
    return {reinterpret_cast<SDL_GPUTexture*>(textureStorage.data()),
            reinterpret_cast<SDL_GPUSampler*>(samplerStorage.data()),
            {},
            {}};
}
void RegisterTexture(std::uint32_t id, void* texture)
{
    record.textures[id] = texture;
}
void UnregisterTexture(std::uint32_t id)
{
    record.textures.erase(id);
}
void RegisterSampler(std::uint32_t id, void* sampler)
{
    record.samplers[id] = sampler;
}
void UnregisterSampler(std::uint32_t id)
{
    record.samplers.erase(id);
}
void ClearTextureRegistry()
{
    record.textures.clear();
}
void ClearSamplerRegistry()
{
    record.samplers.clear();
}
} // namespace mu

namespace mu
{
void ReleaseBitmapTexture(SDL_GPUDevice*, SDL_GPUTexture*)
{
    ++record.releasedTextures;
}
const char* BitmapUploadDriverName()
{
    return "fixture";
}
} // namespace mu

TEST_CASE("truncated OZT headers and pixel payloads never reach the uploader")
{
    record = {};
    const auto dir = MakeFixtureDir("truncated");
    const auto file = dir / "image.OZT";
    CGlobalBitmap bitmaps;
    constexpr std::uint32_t id = 506u;
    constexpr std::uintmax_t completeSize = 22 + 16;
    for (std::uintmax_t length = 0; length < completeSize; ++length)
    {
        WriteTga(file);
        std::filesystem::resize_file(file, length);
        CHECK_FALSE(bitmaps.LoadImage(id, (dir / "image.tga").wstring()));
        CHECK(bitmaps.GetNumberOfTexture() == 0);
        CHECK(bitmaps.GetUsedTextureMemory() == 0);
        CHECK(record.calls == 0);
        CHECK(record.textures.empty());
        CHECK(record.samplers.empty());
    }
    WriteTga(file);
    CHECK(bitmaps.LoadImage(id, (dir / "image.tga").wstring()));
    CHECK(record.calls == 1);
    bitmaps.UnloadAllImages();
    std::filesystem::remove_all(dir);
}

TEST_CASE("decoded bitmap upload failure leaves registry and accounting unchanged")
{
    record = {};
    auto dir = MakeFixtureDir("failure");
    WriteTga(dir / "failed.OZT");
    WriteJpeg(dir / "failed-jpeg.OZJ");
    CGlobalBitmap bitmaps;
    record.reject = true;
    CHECK_FALSE(bitmaps.LoadImage(501u, (dir / "failed.tga").wstring()));
    REQUIRE(record.pixels.size() == 16);
    CHECK(record.pixels[0] == 32);
    CHECK_FALSE(bitmaps.LoadImage(502u, (dir / "failed-jpeg.jpg").wstring()));
    REQUIRE(record.pixels.size() == 16);
    CHECK(record.pixels[0] >= 82);
    CHECK(record.pixels[0] <= 92);
    CHECK(record.calls == 2);
    CHECK(record.textures.empty());
    CHECK(record.samplers.empty());
    CHECK(bitmaps.GetNumberOfTexture() == 0);
    CHECK(bitmaps.GetUsedTextureMemory() == 0);
    CHECK(record.releasedTextures == 0);
    std::filesystem::remove_all(dir);
}

TEST_CASE("decoded bitmap success publishes pixels and borrowed sampler until unload")
{
    record = {};
    auto dir = MakeFixtureDir("success");
    WriteTga(dir / "image.OZT");
    WriteJpeg(dir / "image-jpeg.OZJ");
    CGlobalBitmap bitmaps;
    constexpr std::uint32_t tgaId = 503u;
    constexpr std::uint32_t jpegId = 504u;
    CHECK(bitmaps.LoadImage(tgaId, (dir / "image.tga").wstring()));
    REQUIRE(record.pixels.size() == 16);
    CHECK(record.width == 2);
    CHECK(record.height == 2);
    CHECK(record.pixels[0] == 32); // Bottom row is first after vertical flip, BGR -> RGB.
    CHECK(record.pixels[3] == 42);
    CHECK(record.pixels[8] == 30); // Top row follows, preserving row and channel order.
    REQUIRE(bitmaps.FindTexture(tgaId) != nullptr);
    CHECK(bitmaps.GetUsedTextureMemory() == 16);
    CHECK(record.samplers.at(tgaId) == bitmaps.FindTexture(tgaId)->sdlSampler);
    CHECK(bitmaps.LoadImage(jpegId, (dir / "image-jpeg.jpg").wstring()));
    REQUIRE(record.pixels.size() == 16);
    CHECK(record.pixels[0] >= 82);
    CHECK(record.pixels[0] <= 92);
    CHECK(record.pixels[3] == 255);
    CHECK(bitmaps.GetUsedTextureMemory() == 28);
    CHECK(bitmaps.LoadImage(tgaId, (dir / "image.tga").wstring()));
    CHECK(record.calls == 2);
    CHECK(bitmaps.FindTexture(tgaId)->Ref == 2);
    bitmaps.UnloadImage(tgaId);
    CHECK(bitmaps.FindTexture(tgaId) != nullptr);
    CHECK(record.releasedTextures == 0);
    bitmaps.UnloadImage(tgaId);
    CHECK(bitmaps.FindTexture(tgaId) == nullptr);
    CHECK(record.samplers.count(tgaId) == 0);
    CHECK(record.releasedTextures == 1);
    bitmaps.UnloadAllImages();
    CHECK(record.textures.empty());
    CHECK(record.samplers.empty());
    CHECK(bitmaps.GetUsedTextureMemory() == 0);
    CHECK(record.releasedTextures == 2);
    std::filesystem::remove_all(dir);
}

TEST_CASE("bitmap replacement keeps existing ID and refcount semantics on rejected upload")
{
    record = {};
    auto dir = MakeFixtureDir("replacement");
    WriteTga(dir / "original.OZT");
    WriteTga(dir / "replacement.OZT");
    CGlobalBitmap bitmaps;
    constexpr std::uint32_t id = 505u;
    const auto original = (dir / "original.tga").wstring();
    const auto replacement = (dir / "replacement.tga").wstring();
    REQUIRE(bitmaps.LoadImage(id, original));
    CHECK(bitmaps.LoadImage(id, original));
    CHECK(bitmaps.FindTexture(id)->Ref == 2);
    record.reject = true;
    CHECK_FALSE(bitmaps.LoadImage(id, replacement));
    CHECK(bitmaps.GetNumberOfTexture() == 0);
    CHECK(bitmaps.GetUsedTextureMemory() == 0);
    CHECK(record.textures.count(id) == 0);
    CHECK(record.samplers.count(id) == 0);
    CHECK(record.releasedTextures == 1);
    record.reject = false;
    REQUIRE(bitmaps.LoadImage(id, replacement));
    CHECK(bitmaps.FindTexture(id)->Ref == 1);
    CHECK(bitmaps.GetUsedTextureMemory() == 16);
    CHECK(record.textures.count(id) == 1);
    CHECK(record.samplers.count(id) == 1);
    bitmaps.UnloadImage(id, true);
    CHECK(record.releasedTextures == 2);
    CHECK(record.samplers.empty());
    std::filesystem::remove_all(dir);
}
