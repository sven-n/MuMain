#include "stdafx.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "Render/Sprites/TextureDecode.h"
#include "Render/Renderer/BitmapUpload.h"
#include "Render/Renderer/MuRenderer.h"
#include "../data/ClientFolderScope.h"

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
#include <system_error>
#include <vector>

namespace
{
// The image-type field follows the four-byte OZT prefix and two TGA header bytes.
constexpr std::size_t OztImageTypeOffset = 4 + 2;
constexpr std::uint8_t UncompressedTrueColor = 2;
struct UploadRecord
{
    std::vector<std::uint8_t> pixels;
    int width = 0;
    int height = 0;
    bool reject = false;
    int calls = 0;
    bool mipmapped = false;
    // Upscaled art creates a real filtered sampler, which the fake device cannot.
    bool noDevice = false;
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
        return record.noDevice ? nullptr : reinterpret_cast<void*>(1);
    }
};

std::filesystem::path MakeFixtureDir(const char* name)
{
    const auto path = std::filesystem::current_path() / (std::string("bitmap_loader_") + name);
    std::filesystem::create_directories(path);
    return path;
}
// `shade` is added to every channel, so two files can be told apart by their pixels.
void WriteTga(const std::filesystem::path& path, std::uint8_t shade = 0)
{
    // OZT has a four-byte prefix followed by the 18-byte TGA header and pixels.
    std::array<std::uint8_t, 22 + 16> bytes{};
    bytes[OztImageTypeOffset] = UncompressedTrueColor;
    bytes[16] = 2;
    bytes[18] = 2;
    bytes[20] = 32;
    for (std::size_t i = 0; i < 4; ++i)
    {
        bytes[22 + i * 4] = static_cast<std::uint8_t>(shade + 10 + i);
        bytes[23 + i * 4] = static_cast<std::uint8_t>(shade + 20 + i);
        bytes[24 + i * 4] = static_cast<std::uint8_t>(shade + 30 + i);
        bytes[25 + i * 4] = static_cast<std::uint8_t>(shade + 40 + i);
    }
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary);
    out.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}
// The same image, marked as upscaled 2x (Render/Textures/UpscaleMarker.h).
void WriteMarkedTga(const std::filesystem::path& path)
{
    WriteTga(path);
    std::fstream file(path, std::ios::binary | std::ios::in | std::ios::out);
    const std::array<char, 4> marker{'M', 'U', 'X', 2};
    file.write(marker.data(), marker.size());
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
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary);
    std::array<char, 24> prefix{};
    out.write(prefix.data(), prefix.size());
    out.write(reinterpret_cast<const char*>(compressed), static_cast<std::streamsize>(length));
    tjFree(compressed);
    tjDestroy(handle);
}

// A fixture folder for one test, emptied first and removed afterwards.
struct FixtureFolder
{
    explicit FixtureFolder(const char* name) : path(MakeFixtureDir(name))
    {
        std::filesystem::remove_all(path);
        std::filesystem::create_directories(path);
    }
    ~FixtureFolder()
    {
        std::error_code error;
        std::filesystem::remove_all(path, error);
    }
    FixtureFolder(const FixtureFolder&) = delete;
    FixtureFolder& operator=(const FixtureFolder&) = delete;

    std::filesystem::path path;
};

// The pixel the TGA fixture uploads first (see the success test), for a file
// written with this shade.
constexpr std::uint8_t FirstPixel(std::uint8_t shade)
{
    return static_cast<std::uint8_t>(shade + 32);
}
constexpr std::uint8_t PackShade = 100;
// Batches in these tests decode on this many threads, so how many loads run ahead does not depend on the machine.
constexpr unsigned TestDecodeThreads = 8;
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
                                                                int height, SDL_GPUFilter, SDL_GPUSamplerAddressMode,
                                                                std::span<const Render::Textures::MipLevel> mips,
                                                                bool generateMips)
{
    ++record.calls;
    record.mipmapped = generateMips || !mips.empty();
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
// Texture filtering stays off in these tests: no world texture gets a chain.
void RegisterFilteredSampler(std::uint32_t, void*) {}
void MarkFilteredSamplerPending(std::uint32_t) {}
void TakeFilteredSamplerRequests(std::vector<std::uint32_t>& requests)
{
    requests.clear();
}
void SetTextureFilteringEnabled(bool) {}
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
double BitmapUploadFenceWaitSeconds()
{
    return 0.0;
}
void BeginBitmapUploadBatch() {}
void EndBitmapUploadBatch() {}
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
    CHECK(bitmaps.GetUsedTextureMemory() == 32); // The RGBA base level of each.
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

TEST_CASE("bitmap loader uploads marked files mipmapped at their original size")
{
    record = {};
    record.noDevice = true;
    auto dir = MakeFixtureDir("upscaled");
    WriteMarkedTga(dir / "marked.OZT");
    CGlobalBitmap bitmaps;
    constexpr std::uint32_t textureId = 506u;
    constexpr std::uint32_t canvasId = 507u;
    REQUIRE(bitmaps.LoadImage(textureId, (dir / "marked.tga").wstring()));
    CHECK(record.width == 2);
    CHECK(record.mipmapped);
    const BITMAP_t* texture = bitmaps.FindTexture(textureId);
    CHECK(texture->Width == 1.0f);
    CHECK(texture->Height == 1.0f);
    CHECK(texture->Buffer == nullptr);
    CHECK(bitmaps.GetUsedTextureMemory() == 16);

    // A canvas keeps its pixels, brought back to the original size.
    REQUIRE(bitmaps.LoadImage(canvasId, (dir / "marked.tga").wstring(), GL_NEAREST, GL_CLAMP_TO_EDGE, true, false,
                              true));
    CHECK(record.width == 1);
    CHECK_FALSE(record.mipmapped);
    CHECK(bitmaps.FindTexture(canvasId)->Buffer != nullptr);
    CHECK(bitmaps.GetUsedTextureMemory() == 20);
    bitmaps.UnloadAllImages();
    CHECK(bitmaps.GetUsedTextureMemory() == 0);
    std::filesystem::remove_all(dir);
}

TEST_CASE("the loader reads a texture's copy in the active pack first [texture_pack]")
{
    record = {};
    const FixtureFolder fixture("pack_copy");
    const auto& dir = fixture.path;
    WriteTga(dir / "Data" / "Interface" / "frame.OZT");
    WriteTga(dir / "Data" / "TexturePacks" / "test" / "Interface" / "frame.OZT", PackShade);
    const ClientFolderScope client(dir);
    CGlobalBitmap bitmaps;
    bitmaps.SetTexturePack(L"test");
    constexpr std::uint32_t id = 510u;

    REQUIRE(bitmaps.LoadImage(id, L"Data\\Interface\\frame.tga"));
    REQUIRE(record.pixels.size() == 16);
    CHECK(record.pixels[0] == FirstPixel(PackShade));
    CHECK(bitmaps.FindTexture(id)->fromTexturePack);
    bitmaps.UnloadAllImages();
}

TEST_CASE("a texture the pack lacks is read from Data [texture_pack]")
{
    record = {};
    const FixtureFolder fixture("pack_missing");
    const auto& dir = fixture.path;
    WriteTga(dir / "Data" / "Interface" / "frame.OZT");
    WriteTga(dir / "Data" / "TexturePacks" / "test" / "Interface" / "other.OZT", PackShade);
    const ClientFolderScope client(dir);
    CGlobalBitmap bitmaps;
    bitmaps.SetTexturePack(L"test");
    constexpr std::uint32_t id = 511u;

    REQUIRE(bitmaps.LoadImage(id, L"Data\\Interface\\frame.tga"));
    CHECK(record.pixels[0] == FirstPixel(0));
    CHECK_FALSE(bitmaps.FindTexture(id)->fromTexturePack);
    bitmaps.UnloadAllImages();
}

TEST_CASE("a pack copy that does not load falls back to the original [texture_pack]")
{
    record = {};
    const FixtureFolder fixture("pack_broken");
    const auto& dir = fixture.path;
    const auto packFolder = dir / "Data" / "TexturePacks" / "test" / "Interface";
    WriteTga(dir / "Data" / "Interface" / "frame.OZT");
    WriteTga(packFolder / "frame.OZT", PackShade);
    std::filesystem::resize_file(packFolder / "frame.OZT", 10);
    WriteJpeg(dir / "Data" / "Interface" / "photo.OZJ");
    WriteJpeg(packFolder / "photo.OZJ");
    std::filesystem::resize_file(packFolder / "photo.OZJ", 30);
    const ClientFolderScope client(dir);
    CGlobalBitmap bitmaps;
    bitmaps.SetTexturePack(L"test");
    constexpr std::uint32_t tgaId = 512u;
    constexpr std::uint32_t jpegId = 513u;

    REQUIRE(bitmaps.LoadImage(tgaId, L"Data\\Interface\\frame.tga"));
    CHECK(record.pixels[0] == FirstPixel(0));
    CHECK_FALSE(bitmaps.FindTexture(tgaId)->fromTexturePack);
    REQUIRE(bitmaps.LoadImage(jpegId, L"Data\\Interface\\photo.jpg"));
    CHECK_FALSE(bitmaps.FindTexture(jpegId)->fromTexturePack);
    CHECK(record.calls == 2);
    bitmaps.UnloadAllImages();
}

TEST_CASE("pack files match the requested path in any case [texture_pack]")
{
    record = {};
    const FixtureFolder fixture("pack_case");
    const auto& dir = fixture.path;
    WriteTga(dir / "Data" / "Interface" / "frame.OZT");
    WriteTga(dir / "Data" / "TexturePacks" / "test" / "interface" / "FRAME.ozt", PackShade);
    const ClientFolderScope client(dir);
    CGlobalBitmap bitmaps;
    bitmaps.SetTexturePack(L"test");
    constexpr std::uint32_t id = 514u;

    REQUIRE(bitmaps.LoadImage(id, L"Data\\Interface\\frame.tga"));
    CHECK(record.pixels[0] == FirstPixel(PackShade));
    bitmaps.UnloadAllImages();
}

TEST_CASE("without a pack, or with a missing one, textures come from Data [texture_pack]")
{
    record = {};
    const FixtureFolder fixture("pack_none");
    const auto& dir = fixture.path;
    WriteTga(dir / "Data" / "Interface" / "frame.OZT");
    WriteTga(dir / "Data" / "TexturePacks" / "test" / "Interface" / "frame.OZT", PackShade);
    const ClientFolderScope client(dir);
    CGlobalBitmap bitmaps;
    constexpr std::uint32_t id = 515u;

    REQUIRE(bitmaps.LoadImage(id, L"Data\\Interface\\frame.tga"));
    CHECK(record.pixels[0] == FirstPixel(0));
    bitmaps.UnloadImage(id, true);

    bitmaps.SetTexturePack(L"missing");
    REQUIRE(bitmaps.LoadImage(id, L"Data\\Interface\\frame.tga"));
    CHECK(record.pixels[0] == FirstPixel(0));
    bitmaps.UnloadAllImages();
}

TEST_CASE("decoding a texture file reaches neither the uploader nor the registry")
{
    record = {};
    const FixtureFolder fixture("decode_only");
    const auto& dir = fixture.path;
    WriteTga(dir / "frame.OZT");
    WriteJpeg(dir / "photo.OZJ");
    const Render::Textures::TexturePackFiles noPack;

    Render::Sprites::TextureDecodeRequest tga;
    tga.index = 520u;
    tga.filename = (dir / "frame.tga").wstring();
    tga.path = (dir / "frame.OZT").wstring();
    const Render::Sprites::TextureDecodeResult tgaResult = Render::Sprites::DecodeTextureFile(noPack, tga);
    REQUIRE(tgaResult.texture);
    CHECK(tgaResult.errors.empty());
    const Render::Sprites::DecodedTexture& tgaTexture = *tgaResult.texture;
    CHECK(tgaTexture.bitmap->BitmapIndex == 520u);
    CHECK(tgaTexture.bitmap->Components == 4);
    CHECK(tgaTexture.uploadWidth == 2);
    CHECK(tgaTexture.uploadHeight == 2);
    REQUIRE(tgaTexture.UploadPixels().size() == 16);
    CHECK(tgaTexture.UploadPixels()[0] == FirstPixel(0));

    Render::Sprites::TextureDecodeRequest jpeg;
    jpeg.index = 521u;
    jpeg.filename = (dir / "photo.jpg").wstring();
    jpeg.path = (dir / "photo.OZJ").wstring();
    jpeg.jpeg = true;
    const Render::Sprites::TextureDecodeResult jpegResult = Render::Sprites::DecodeTextureFile(noPack, jpeg);
    REQUIRE(jpegResult.texture);
    CHECK(jpegResult.texture->bitmap->Components == 3);
    REQUIRE(jpegResult.texture->UploadPixels().size() == 16);
    CHECK(jpegResult.texture->UploadPixels()[3] == 255u);

    CHECK(record.calls == 0);
    CHECK(record.textures.empty());
    CHECK(record.samplers.empty());
}

TEST_CASE("a texture file that does not decode comes back with its error")
{
    record = {};
    const FixtureFolder fixture("decode_error");
    const auto& dir = fixture.path;
    WriteTga(dir / "frame.OZT");
    std::filesystem::resize_file(dir / "frame.OZT", 10);
    const Render::Textures::TexturePackFiles noPack;

    Render::Sprites::TextureDecodeRequest request;
    request.filename = (dir / "frame.tga").wstring();
    request.path = (dir / "frame.OZT").wstring();
    const Render::Sprites::TextureDecodeResult truncated = Render::Sprites::DecodeTextureFile(noPack, request);
    CHECK_FALSE(truncated.texture);
    REQUIRE(truncated.errors.size() == 1);
    CHECK(truncated.errors[0].find(L"file too small") != std::wstring::npos);

    // A missing file is not an error report line, as before.
    request.path = (dir / "missing.OZT").wstring();
    const Render::Sprites::TextureDecodeResult missing = Render::Sprites::DecodeTextureFile(noPack, request);
    CHECK_FALSE(missing.texture);
    CHECK(missing.errors.empty());
}

namespace
{
std::vector<std::wstring> reportedFailures;
void RecordFailure(const wchar_t* filename)
{
    reportedFailures.emplace_back(filename);
}
} // namespace

TEST_CASE("a load batch uploads nothing until a bitmap is asked for or the batch ends")
{
    record = {};
    const FixtureFolder fixture("batch");
    const auto& dir = fixture.path;
    WriteTga(dir / "a.OZT");
    WriteTga(dir / "b.OZT", PackShade);
    WriteJpeg(dir / "c.OZJ");
    CGlobalBitmap bitmaps;

    bitmaps.BeginLoadBatch(TestDecodeThreads);
    CHECK(bitmaps.LoadImage(530u, (dir / "a.tga").wstring()));
    CHECK(bitmaps.LoadImage(531u, (dir / "b.tga").wstring()));
    CHECK(bitmaps.LoadImage(532u, (dir / "c.jpg").wstring()));
    CHECK(record.calls == 0);
    CHECK(bitmaps.GetNumberOfTexture() == 0);

    // Asking for one waits for it alone.
    const BITMAP_t* second = bitmaps.FindTexture(531u);
    REQUIRE(second != nullptr);
    CHECK(record.calls == 1);
    CHECK(record.pixels[0] == FirstPixel(PackShade));
    CHECK(second->Width == 2.0f);
    CHECK(bitmaps.GetNumberOfTexture() == 1);

    bitmaps.EndLoadBatch();
    CHECK(record.calls == 3);
    // The rest committed in the order asked for: the JPEG last.
    CHECK(record.pixels.size() == 16);
    CHECK(record.pixels[3] == 255u);
    CHECK(bitmaps.GetNumberOfTexture() == 3);
    CHECK(record.textures.size() == 3);
    CHECK(bitmaps.GetUsedTextureMemory() == 48);
    bitmaps.UnloadAllImages();
}

TEST_CASE("a queued load that is unloaded or loaded again commits first")
{
    record = {};
    const FixtureFolder fixture("batch_unload");
    const auto& dir = fixture.path;
    WriteTga(dir / "a.OZT");
    CGlobalBitmap bitmaps;

    bitmaps.BeginLoadBatch(TestDecodeThreads);
    CHECK(bitmaps.LoadImage(540u, (dir / "a.tga").wstring()));
    CHECK(bitmaps.LoadImage(540u, (dir / "a.tga").wstring()));
    CHECK(record.calls == 1);
    CHECK(bitmaps.FindTexture(540u)->Ref == 2);

    CHECK(bitmaps.LoadImage(541u, (dir / "a.tga").wstring()));
    bitmaps.UnloadImage(541u);
    CHECK(record.calls == 2);
    CHECK(bitmaps.FindTexture(541u) == nullptr);
    bitmaps.EndLoadBatch();
    CHECK(record.calls == 2);
    CHECK(bitmaps.GetNumberOfTexture() == 1);
    bitmaps.UnloadAllImages();
}

TEST_CASE("a queued load that fails is reported when it commits")
{
    record = {};
    reportedFailures.clear();
    const FixtureFolder fixture("batch_failure");
    const auto& dir = fixture.path;
    WriteTga(dir / "broken.OZT");
    std::filesystem::resize_file(dir / "broken.OZT", 10);
    CGlobalBitmap bitmaps;

    bitmaps.BeginLoadBatch(TestDecodeThreads);
    bitmaps.BeginLoadBatch(TestDecodeThreads);
    CHECK(bitmaps.LoadImage(550u, (dir / "broken.tga").wstring(), GL_NEAREST, GL_CLAMP_TO_EDGE, true, false, false,
                            &RecordFailure));
    // A missing file fails at once.
    CHECK_FALSE(bitmaps.LoadImage(551u, (dir / "missing.tga").wstring(), GL_NEAREST, GL_CLAMP_TO_EDGE, true, false,
                                  false, &RecordFailure));
    REQUIRE(reportedFailures.size() == 1);
    CHECK(reportedFailures[0] == (dir / "missing.tga").wstring());
    // The inner batch ending commits nothing.
    bitmaps.EndLoadBatch();
    CHECK(reportedFailures.size() == 1);
    bitmaps.EndLoadBatch();
    REQUIRE(reportedFailures.size() == 2);
    CHECK(reportedFailures[1] == (dir / "broken.tga").wstring());
    CHECK(record.calls == 0);
    CHECK(bitmaps.GetNumberOfTexture() == 0);

    // Outside a batch the failure is reported at once, as before.
    CHECK_FALSE(bitmaps.LoadImage(552u, (dir / "broken.tga").wstring(), GL_NEAREST, GL_CLAMP_TO_EDGE, true, false,
                                  false, &RecordFailure));
    CHECK(reportedFailures.size() == 3);
}

TEST_CASE("a canvas loads at once inside a load batch")
{
    record = {};
    const FixtureFolder fixture("batch_canvas");
    const auto& dir = fixture.path;
    WriteTga(dir / "canvas.OZT");
    CGlobalBitmap bitmaps;

    bitmaps.BeginLoadBatch(TestDecodeThreads);
    CHECK(bitmaps.LoadImage(560u, (dir / "canvas.tga").wstring(), GL_NEAREST, GL_CLAMP_TO_EDGE, true, false, true));
    CHECK(record.calls == 1);
    bitmaps.EndLoadBatch();
    CHECK(record.calls == 1);
    bitmaps.UnloadAllImages();
}

TEST_CASE("inside a load batch a search over folders finds the folder that has the file")
{
    record = {};
    const FixtureFolder fixture("batch_search");
    const auto& dir = fixture.path;
    WriteTga(dir / "second" / "skin.OZT");
    CGlobalBitmap bitmaps;

    bitmaps.BeginLoadBatch(TestDecodeThreads);
    CHECK(bitmaps.LoadImage((dir / "first" / "skin.tga").wstring()) == BITMAP_UNKNOWN);
    const GLuint found = bitmaps.LoadImage((dir / "second" / "skin.tga").wstring());
    CHECK(found != BITMAP_UNKNOWN);
    CHECK(record.calls == 0);
    bitmaps.EndLoadBatch();
    REQUIRE(bitmaps.FindTexture(found) != nullptr);
    CHECK(bitmaps.GetNumberOfTexture() == 1);
    bitmaps.UnloadAllImages();
}

namespace
{
CGlobalBitmap* quittingBitmaps = nullptr;
// What the error dialog's Quit does: the game tears down and unloads every bitmap.
void QuitOnFailure(const wchar_t* filename)
{
    reportedFailures.emplace_back(filename);
    quittingBitmaps->UnloadAllImages();
}
} // namespace

TEST_CASE("quitting from a failed queued load unloads the batch cleanly")
{
    record = {};
    reportedFailures.clear();
    const FixtureFolder fixture("batch_quit");
    const auto& dir = fixture.path;
    WriteTga(dir / "broken.OZT");
    std::filesystem::resize_file(dir / "broken.OZT", 10);
    WriteTga(dir / "good.OZT");
    CGlobalBitmap bitmaps;
    quittingBitmaps = &bitmaps;

    bitmaps.BeginLoadBatch(TestDecodeThreads);
    CHECK(bitmaps.LoadImage(600u, (dir / "broken.tga").wstring(), GL_NEAREST, GL_CLAMP_TO_EDGE, true, false, false,
                            &QuitOnFailure));
    CHECK(bitmaps.LoadImage(601u, (dir / "good.tga").wstring()));
    // Looking the broken one up by name commits it; its failure quits while the lookup runs.
    CHECK(bitmaps.FindTexture((dir / "broken.tga").wstring()) == nullptr);
    REQUIRE(reportedFailures.size() == 1);
    CHECK(record.calls == 1);
    CHECK(bitmaps.GetNumberOfTexture() == 0);
    bitmaps.EndLoadBatch();
    CHECK(bitmaps.GetNumberOfTexture() == 0);
    CHECK(record.textures.empty());
    quittingBitmaps = nullptr;
}

TEST_CASE("a load batch commits older loads instead of holding every decoded file")
{
    record = {};
    const FixtureFolder fixture("batch_window");
    const auto& dir = fixture.path;
    constexpr int files = 40;
    for (int i = 0; i < files; ++i)
    {
        WriteTga(dir / ("t" + std::to_string(i) + ".OZT"));
    }
    CGlobalBitmap bitmaps;

    bitmaps.BeginLoadBatch(TestDecodeThreads);
    for (int i = 0; i < files; ++i)
    {
        CHECK(bitmaps.LoadImage(700u + static_cast<std::uint32_t>(i),
                                (dir / ("t" + std::to_string(i) + ".tga")).wstring()));
    }
    // At most one load per worker is still waiting to commit.
    constexpr int window = static_cast<int>(TestDecodeThreads);
    CHECK(record.calls >= files - window);
    CHECK(bitmaps.GetNumberOfTexture() == static_cast<std::size_t>(record.calls));
    bitmaps.EndLoadBatch();
    CHECK(record.calls == files);
    CHECK(bitmaps.GetNumberOfTexture() == files);
    bitmaps.UnloadAllImages();
}
