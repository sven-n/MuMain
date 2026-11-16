#include <doctest.h>

#include "Render/Renderer/BitmapGpuResources.h"

#include <algorithm>
#include <array>
#include <barrier>
#include <cstdint>
#include <cstring>
#include <map>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace
{
using Owner = Render::Renderer::BitmapGpuResources;
template <typename T> T* Handle(int n);
template <typename T> int Index(T* handle);

struct Fake
{
    static inline Fake* active = nullptr;
    // Handles: transfer buffer n is token kTransferBase + n, its fence kFenceBase + n.
    static constexpr int kTransferBase = 100, kFenceBase = 600;
    std::array<int, 1100> tokens{};
    int transfers = 0, samplers = 0, textures = 0, waits = 0, finishedWaits = 0, releases = 0, fences = 0, mipmaps = 0;
    int failureAt = -1, creates = 0;
    const char* failure = "original SDL error";
    bool failWait = false, failSubmit = false, failMap = false, failAcquire = false, failBegin = false;
    bool failTexture = false, failGrow = false;
    // Whether the GPU has finished a submission by the time its fence is queried.
    bool gpuFinishes = true;
    bool mapped = false, cycled = false, overwrite = false, canceled = false;
    std::map<int, std::vector<std::uint8_t>> bytes;
    std::map<int, bool> busy;
    std::vector<SDL_GPUSamplerCreateInfo> descriptors;
    SDL_GPUTextureTransferInfo source{};
    SDL_GPUTextureRegion destination{};
    SDL_GPUTextureCreateInfo texture{};
    std::vector<std::pair<SDL_GPUTextureTransferInfo, SDL_GPUTextureRegion>> uploads;
    Uint32 textureLevels = 0;
    std::string lastEvent;

    Fake()
    {
        active = this;
    }
    ~Fake()
    {
        active = nullptr;
    }
    bool ShouldFail()
    {
        return creates++ == failureAt;
    }
    static SDL_GPUTransferBuffer* CreateTransfer(SDL_GPUDevice*, const SDL_GPUTransferBufferCreateInfo* info)
    {
        CHECK(info->size == Owner::kSlotCapacity);
        if (active->ShouldFail() ||
            (active->failGrow && active->transfers >= static_cast<int>(Owner::kMinimumSlotCount)))
            return nullptr;
        ++active->transfers;
        active->bytes[active->transfers].resize(Owner::kSlotCapacity);
        return Handle<SDL_GPUTransferBuffer>(kTransferBase + active->transfers);
    }
    static void ReleaseTransfer(SDL_GPUDevice*, SDL_GPUTransferBuffer*)
    {
        ++active->releases;
    }
    static SDL_GPUSampler* CreateSampler(SDL_GPUDevice*, const SDL_GPUSamplerCreateInfo* info)
    {
        active->descriptors.push_back(*info);
        if (active->ShouldFail())
            return nullptr;
        return Handle<SDL_GPUSampler>(20 + ++active->samplers);
    }
    static void ReleaseSampler(SDL_GPUDevice*, SDL_GPUSampler*)
    {
        ++active->releases;
    }
    static SDL_GPUTexture* CreateTexture(SDL_GPUDevice*, const SDL_GPUTextureCreateInfo* info)
    {
        CHECK(info->format == SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM);
        active->texture = *info;
        active->textureLevels = info->num_levels;
        ++active->textures;
        return active->failTexture ? nullptr : Handle<SDL_GPUTexture>(40 + active->textures % 20);
    }
    static void ReleaseTexture(SDL_GPUDevice*, SDL_GPUTexture*)
    {
        ++active->releases;
    }
    static void* Map(SDL_GPUDevice*, SDL_GPUTransferBuffer* buffer, bool cycle)
    {
        const int index = Index(buffer) - kTransferBase;
        active->cycled |= cycle;
        active->overwrite |= active->busy[index];
        active->mapped = true;
        if (active->failMap)
            return nullptr;
        return active->bytes[index].data();
    }
    static void Unmap(SDL_GPUDevice*, SDL_GPUTransferBuffer*)
    {
        active->mapped = false;
    }
    static SDL_GPUCommandBuffer* Acquire(SDL_GPUDevice*)
    {
        return active->failAcquire ? nullptr : Handle<SDL_GPUCommandBuffer>(1);
    }
    static SDL_GPUCopyPass* Begin(SDL_GPUCommandBuffer*)
    {
        return active->failBegin ? nullptr : Handle<SDL_GPUCopyPass>(1);
    }
    static void Upload(SDL_GPUCopyPass*, const SDL_GPUTextureTransferInfo* src, const SDL_GPUTextureRegion* dst,
                       bool cycle)
    {
        CHECK_FALSE(active->mapped);
        active->cycled |= cycle;
        active->source = *src;
        active->destination = *dst;
        active->uploads.emplace_back(*src, *dst);
    }
    static void End(SDL_GPUCopyPass*) {}
    static void GenerateMipmaps(SDL_GPUCommandBuffer*, SDL_GPUTexture*)
    {
        ++active->mipmaps;
    }
    static bool Cancel(SDL_GPUCommandBuffer*)
    {
        active->canceled = true;
        active->failure = "cleanup error";
        return true;
    }
    static SDL_GPUFence* Submit(SDL_GPUCommandBuffer*)
    {
        if (active->failSubmit)
            return nullptr;
        const int slot = Index(active->source.transfer_buffer) - kTransferBase;
        active->busy[slot] = true;
        ++active->fences;
        return Handle<SDL_GPUFence>(kFenceBase + slot);
    }
    static bool Query(SDL_GPUDevice*, SDL_GPUFence* fence)
    {
        const int slot = Index(fence) - kFenceBase;
        if (active->gpuFinishes)
            active->busy[slot] = false;
        return !active->busy[slot];
    }
    static bool Wait(SDL_GPUDevice*, bool all, SDL_GPUFence* const* fences, Uint32 count)
    {
        CHECK(all);
        CHECK(count == 1);
        // Only a wait on a fence the GPU hasn't finished blocks; one on a finished fence returns at once.
        if (!active->busy[Index(*fences) - kFenceBase])
        {
            ++active->finishedWaits;
            return true;
        }
        ++active->waits;
        if (active->failWait)
            return false;
        active->busy[Index(*fences) - kFenceBase] = false;
        return true;
    }
    static void ReleaseFence(SDL_GPUDevice*, SDL_GPUFence*)
    {
        --active->fences;
        ++active->releases;
    }
    static const char* Error()
    {
        return active->failure;
    }
    Owner::Calls Calls() const
    {
        Owner::Calls c{};
        c.createTransfer = CreateTransfer;
        c.releaseTransfer = ReleaseTransfer;
        c.createSampler = CreateSampler;
        c.releaseSampler = ReleaseSampler;
        c.createTexture = CreateTexture;
        c.releaseTexture = ReleaseTexture;
        c.map = Map;
        c.unmap = Unmap;
        c.acquire = Acquire;
        c.beginCopy = Begin;
        c.upload = Upload;
        c.endCopy = End;
        c.generateMipmaps = GenerateMipmaps;
        c.cancel = Cancel;
        c.submit = Submit;
        c.query = Query;
        c.wait = Wait;
        c.releaseFence = ReleaseFence;
        c.getError = Error;
        return c;
    }
};
template <typename T> T* Handle(int n)
{
    return reinterpret_cast<T*>(&Fake::active->tokens[n]);
}
template <typename T> int Index(T* handle)
{
    return static_cast<int>(reinterpret_cast<int*>(handle) - Fake::active->tokens.data());
}
// The staging bytes an upload's source points at.
const std::uint8_t* Staged(const SDL_GPUTextureTransferInfo& source)
{
    return Fake::active->bytes[Index(source.transfer_buffer) - Fake::kTransferBase].data() + source.offset;
}

constexpr auto filter = SDL_GPU_FILTER_LINEAR;
constexpr auto wrap = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
SDL_GPUDevice* Device()
{
    return Handle<SDL_GPUDevice>(1);
}
} // namespace

TEST_CASE("bitmap owner fixed allocations and sampler descriptors")
{
    for (int fail = 0; fail < 7; ++fail)
    {
        Fake fake;
        fake.failureAt = fail;
        Owner owner(fake.Calls());
        CHECK_FALSE(owner.Initialize(Device()));
        CHECK(owner.LastError() == "original SDL error");
        CHECK_FALSE(owner.Initialize(Device())); // no retry on the failed device lifetime
        CHECK(owner.Shutdown());
        CHECK(fake.releases == fail);
    }
    Fake fake;
    Owner owner(fake.Calls());
    REQUIRE(owner.Initialize(Device()));
    CHECK(fake.transfers == 3);
    CHECK(fake.samplers == 4);
    for (int i = 0; i < 4; ++i)
    {
        const auto& d = fake.descriptors[i];
        CHECK(d.min_filter == (i < 2 ? SDL_GPU_FILTER_NEAREST : SDL_GPU_FILTER_LINEAR));
        CHECK(d.mag_filter == d.min_filter);
        CHECK(d.mipmap_mode == SDL_GPU_SAMPLERMIPMAPMODE_NEAREST);
        CHECK(d.address_mode_u ==
              (i % 2 == 0 ? SDL_GPU_SAMPLERADDRESSMODE_REPEAT : SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE));
        CHECK(d.address_mode_v == d.address_mode_u);
        CHECK(d.address_mode_w == SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE);
    }
    CHECK(owner.Shutdown());
    CHECK(owner.Shutdown());
    CHECK(fake.releases == 7);
}

TEST_CASE("repeated uploads share the open slot and never wait while the GPU keeps up")
{
    Fake fake;
    Owner owner(fake.Calls());
    REQUIRE(owner.Initialize(Device()));
    CHECK(Owner::kSlotCapacity == 4u * 1024u * 1024u);
    std::vector<std::uint8_t> pixels(std::size_t{65} * 2u * 4u, 0x5a);
    SDL_GPUSampler* shared = nullptr;
    for (int i = 0; i < 10001; ++i)
    {
        auto result = owner.Upload(pixels, 65, 2, filter, wrap);
        REQUIRE(result);
        if (!shared)
            shared = result.sampler;
        CHECK(result.sampler == shared);
        // 65 pixels take a 512-byte pitch: each upload's two rows are 1 KiB, 4096 to a slot.
        CHECK(fake.source.offset == static_cast<Uint32>(i % 4096) * 1024u);
        CHECK(fake.source.rows_per_layer == 2);
        CHECK(fake.destination.w == 65);
        CHECK(fake.destination.h == 2);
        fake.Calls().releaseTexture(Device(), result.texture);
    }
    CHECK(fake.transfers == 3);
    CHECK(fake.samplers == 4);
    CHECK(fake.waits == 0);
    CHECK(fake.fences == 2); // two full slots submitted; the third is still open
    CHECK_FALSE(fake.overwrite);
    CHECK_FALSE(fake.cycled);
    CHECK(owner.Shutdown());
    CHECK(fake.fences == 0);
}

TEST_CASE("bitmap owner stages rows at the platform's row pitch")
{
#if defined(__APPLE__)
    static_assert(Owner::kRowAlignment == Owner::kBytesPerPixel);
    constexpr Uint32 kExpectedPixelsPerRow = 65;
#else
    static_assert(Owner::kRowAlignment == 256u);
    constexpr Uint32 kExpectedPixelsPerRow = 128;
#endif
    Fake fake;
    Owner owner(fake.Calls());
    REQUIRE(owner.Initialize(Device()));
    std::vector<std::uint8_t> pixels(std::size_t{65} * 2u * 4u, 0x5a);
    auto result = owner.Upload(pixels, 65, 2, filter, wrap);
    REQUIRE(result);
    CHECK(fake.source.pixels_per_row == kExpectedPixelsPerRow);
    fake.Calls().releaseTexture(Device(), result.texture);
    CHECK(owner.Shutdown());
}

TEST_CASE("bitmap owner checks dimensions and copies exact rows")
{
    Fake fake;
    Owner owner(fake.Calls());
    REQUIRE(owner.Initialize(Device()));
    for (int width : {1, 2, 63, 64, 65, 1024})
    {
        std::vector<std::uint8_t> pixels(static_cast<std::size_t>(width) * 8u + 1u, 0xa5);
        auto result = owner.Upload(std::span(pixels).first(pixels.size() - 1), width, 2, filter, wrap);
        REQUIRE(result);
        const std::uint8_t* staged = Staged(fake.source);
        const auto pitch = fake.source.pixels_per_row * 4;
        CHECK(std::memcmp(staged, pixels.data(), static_cast<std::size_t>(width) * 4u) == 0);
        const auto visibleRowBytes = static_cast<std::size_t>(width) * 4u;
        CHECK(std::memcmp(staged + pitch, pixels.data() + visibleRowBytes, visibleRowBytes) == 0);
        // The staged bytes are fresh here (the fake zeroes new slots): row padding is left alone.
        CHECK(std::all_of(staged + visibleRowBytes, staged + pitch, [](std::uint8_t byte) { return byte == 0; }));
        CHECK(std::all_of(staged + pitch + visibleRowBytes, staged + std::size_t{2} * pitch,
                          [](std::uint8_t byte) { return byte == 0; }));
        fake.Calls().releaseTexture(Device(), result.texture);
    }
    std::vector<std::uint8_t> maximum(Owner::kSlotCapacity, 0x2a);
    auto maxResult = owner.Upload(maximum, 1024, 1024, filter, wrap);
    REQUIRE(maxResult);
    fake.Calls().releaseTexture(Device(), maxResult.texture);
    const int before = fake.textures;
    CHECK_FALSE(owner.Upload(maximum, 0, 1, filter, wrap));
    CHECK_FALSE(owner.Upload(maximum, -1, 1, filter, wrap));
    CHECK_FALSE(owner.Upload(maximum, Owner::kMaximumWidth + 1, 1, filter, wrap));
    CHECK_FALSE(owner.Upload(maximum, 1, Owner::kMaximumHeight + 1, filter, wrap));
    CHECK_FALSE(owner.Upload(std::span(maximum).first(3), 1, 1, filter, wrap));
    CHECK_FALSE(owner.Upload(maximum, 1, 1, static_cast<SDL_GPUFilter>(99), wrap));
    CHECK(fake.textures == before);
    CHECK(owner.Shutdown());
}

TEST_CASE("bitmap owner fails one upload on GPU errors and faults only without a usable slot")
{
    // Texture, map, acquire and begin-copy fail before the slot is in use: only that upload fails.
    for (int stage = 0; stage < 4; ++stage)
    {
        Fake fake;
        Owner owner(fake.Calls());
        REQUIRE(owner.Initialize(Device()));
        fake.failTexture = stage == 0;
        fake.failMap = stage == 1;
        fake.failAcquire = stage == 2;
        fake.failBegin = stage == 3;
        std::array<std::uint8_t, 4> pixels{};
        CHECK_FALSE(owner.Upload(pixels, 1, 1, filter, wrap));
        CHECK_FALSE(owner.IsFaulted());
        CHECK(owner.LastError() == "original SDL error");
        CHECK(fake.canceled == (stage == 3));
        fake.failTexture = fake.failMap = fake.failAcquire = fake.failBegin = false;
        auto next = owner.Upload(pixels, 1, 1, filter, wrap);
        REQUIRE(next);
        fake.Calls().releaseTexture(Device(), next.texture);
        owner.Shutdown();
    }
    {
        // A failed submit (at the flush that sends the open slot) takes that slot out of service; the next upload
        // uses another, and the uploader faults only once every slot is lost.
        Fake fake;
        Owner owner(fake.Calls());
        REQUIRE(owner.Initialize(Device()));
        std::array<std::uint8_t, 4> pixels{};
        for (std::size_t lost = 1; lost <= Owner::kMinimumSlotCount; ++lost)
        {
            auto staged = owner.Upload(pixels, 1, 1, filter, wrap);
            REQUIRE(staged);
            fake.Calls().releaseTexture(Device(), staged.texture);
            fake.failSubmit = true;
            CHECK_FALSE(owner.Flush());
            fake.failSubmit = false;
            CHECK(owner.IsFaulted() == (lost == Owner::kMinimumSlotCount));
        }
        const auto texturesBefore = fake.textures;
        const auto rejected = owner.Upload(pixels, 1, 1, filter, wrap);
        CHECK_FALSE(rejected);
        CHECK(rejected.error == "original SDL error"); // the cause is kept, not the rejection
        CHECK(fake.textures == texturesBefore);
        owner.Shutdown();
    }
}

TEST_CASE("bitmap owner rejects uploads from a different thread and can recreate after shutdown")
{
    Fake fake;
    Owner owner(fake.Calls());
    REQUIRE(owner.Initialize(Device()));
    std::array<std::uint8_t, 4> pixels{};
    CHECK_FALSE(owner.Upload(pixels, 0, 1, filter, wrap));
    const auto previousStage = owner.LastStage();
    const auto previousError = owner.LastError();
    Owner::Result rejected;
    bool shutdownAccepted = true;
    std::thread worker(
        [&]
        {
            rejected = owner.Upload(pixels, 1, 1, filter, wrap);
            shutdownAccepted = owner.Shutdown();
        });
    worker.join();
    CHECK_FALSE(rejected);
    CHECK(rejected.stage == "upload: wrong thread");
    CHECK_FALSE(shutdownAccepted);
    CHECK(owner.LastStage() == previousStage);
    CHECK(owner.LastError() == previousError);
    CHECK(fake.textures == 0);
    CHECK(owner.Shutdown());
    REQUIRE(owner.Initialize(Device()));
    auto result = owner.Upload(pixels, 1, 1, filter, wrap);
    REQUIRE(result);
    fake.Calls().releaseTexture(Device(), result.texture);
    CHECK(owner.Shutdown());
}

TEST_CASE("bitmap owner keeps concurrent wrong-thread rejections local")
{
    Fake fake;
    Owner owner(fake.Calls());
    REQUIRE(owner.Initialize(Device()));
    std::array<std::uint8_t, 4> pixels{};
    constexpr int attempts = 1000;
    std::barrier start(3);
    std::array<int, 2> rejections{};
    auto reject = [&](std::size_t index)
    {
        start.arrive_and_wait();
        for (int i = 0; i < attempts; ++i)
        {
            const auto result = owner.Upload(pixels, 1, 1, filter, wrap);
            if (!result && result.stage == "upload: wrong thread")
            {
                ++rejections[index];
            }
        }
    };
    std::thread first(reject, 0);
    std::thread second(reject, 1);
    start.arrive_and_wait();
    for (int i = 0; i < attempts; ++i)
    {
        const auto result = owner.Upload(pixels, 1, 1, filter, wrap);
        CHECK(static_cast<bool>(result));
        fake.Calls().releaseTexture(Device(), result.texture);
    }
    first.join();
    second.join();
    CHECK(rejections[0] == attempts);
    CHECK(rejections[1] == attempts);
    CHECK(owner.LastStage().empty());
    CHECK(owner.LastError().empty());
    CHECK(fake.textures == attempts);
    CHECK(owner.Shutdown());
}

TEST_CASE("bitmap sampler keys select four device-owned handles")
{
    Fake fake;
    Owner owner(fake.Calls());
    REQUIRE(owner.Initialize(Device()));
    std::array<std::uint8_t, 4> pixels{};
    std::array<SDL_GPUSampler*, 4> selected{};
    for (int i = 0; i < 4000; ++i)
    {
        const auto key = i % 4;
        const auto selectedFilter = key < 2 ? SDL_GPU_FILTER_NEAREST : SDL_GPU_FILTER_LINEAR;
        const auto selectedWrap =
            key % 2 == 0 ? SDL_GPU_SAMPLERADDRESSMODE_REPEAT : SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
        auto result = owner.Upload(pixels, 1, 1, selectedFilter, selectedWrap);
        REQUIRE(result);
        if (!selected[key])
            selected[key] = result.sampler;
        CHECK(result.sampler == selected[key]);
        fake.Calls().releaseTexture(Device(), result.texture);
    }
    for (std::size_t i = 0; i < selected.size(); ++i)
    {
        for (std::size_t j = i + 1; j < selected.size(); ++j)
            CHECK(selected[i] != selected[j]);
    }
    CHECK(fake.samplers == 4);
    CHECK(owner.Shutdown());
}

TEST_CASE("bitmap owner stages upscaled images in bands and builds their mips last")
{
    Fake fake;
    Owner owner(fake.Calls());
    REQUIRE(owner.Initialize(Device()));
    constexpr int width = 2048;
    constexpr int height = 1030;
    std::vector<std::uint8_t> pixels(std::size_t{width} * height * 4u);
    for (std::size_t row = 0; row < height; ++row)
        std::fill_n(pixels.begin() + static_cast<std::ptrdiff_t>(row * width * 4u), width * 4, static_cast<std::uint8_t>(row % 251));
    auto result = owner.Upload(pixels, width, height, filter, wrap, {}, true);
    REQUIRE(result);
    // 8 KiB rows: 512 per 4 MiB slot, so three bands, the last of 6 rows.
    CHECK(fake.uploads.size() == 3);
    CHECK(fake.destination.y == 1024);
    CHECK(fake.destination.h == 6);
    CHECK(fake.source.rows_per_layer == 6);
    CHECK(Staged(fake.source)[0] == 1024 % 251);
    CHECK(fake.textures == 1);
    CHECK(fake.texture.num_levels == 12);
    CHECK((fake.texture.usage & SDL_GPU_TEXTUREUSAGE_COLOR_TARGET) != 0);
    CHECK(fake.mipmaps == 1);
    fake.Calls().releaseTexture(Device(), result.texture);

    auto plain = owner.Upload(std::span(pixels).first(16), 2, 2, filter, wrap);
    REQUIRE(plain);
    CHECK(fake.texture.num_levels == 1);
    CHECK(fake.mipmaps == 1);
    fake.Calls().releaseTexture(Device(), plain.texture);
    CHECK(owner.Shutdown());
}

namespace
{
// The levels below a square image of `size` texels, each filled with its level number.
std::vector<Render::Textures::MipLevel> Chain(std::uint32_t size)
{
    std::vector<Render::Textures::MipLevel> chain;
    std::uint8_t level = 1;
    for (std::uint32_t edge = size / 2; edge >= 1; edge /= 2, ++level)
    {
        chain.push_back({edge, edge, std::vector<std::uint8_t>(static_cast<std::size_t>(edge) * edge * 4u, level)});
    }
    return chain;
}
} // namespace

TEST_CASE("a full 1024x1024 mip chain uploads through the fixed staging slots")
{
    Fake fake;
    Owner owner(fake.Calls());
    REQUIRE(owner.Initialize(Device()));
    const std::vector<std::uint8_t> top(std::size_t{1024} * 1024u * 4u, 0);
    const auto chain = Chain(1024);
    REQUIRE(chain.size() == 10);

    auto result = owner.Upload(top, 1024, 1024, filter, wrap, chain);
    REQUIRE(result);
    CHECK(fake.textures == 1);
    CHECK(fake.textureLevels == 11);
    // Level 0 fills a slot by itself and goes when the next level doesn't fit; the ten smaller levels share the next
    // slot, which stays open for later uploads until the flush.
    CHECK(fake.fences == 1);
    REQUIRE(fake.uploads.size() == 11);
    for (std::size_t level = 0; level < fake.uploads.size(); ++level)
    {
        const auto& [source, destination] = fake.uploads[level];
        const Uint32 edge = 1024u >> level;
        CHECK(destination.mip_level == level);
        CHECK(destination.w == edge);
        CHECK(destination.h == edge);
        CHECK(source.offset % Owner::kLevelAlignment == 0);
        CHECK(source.offset + static_cast<std::uint64_t>(source.pixels_per_row) * 4u * source.rows_per_layer <=
              Owner::kSlotCapacity);
        CHECK(Staged(source)[0] == level);
    }
    CHECK(owner.Flush());
    CHECK(fake.fences == 2);
}

TEST_CASE("a mip chain whose levels do not halve is refused before anything is created")
{
    Fake fake;
    Owner owner(fake.Calls());
    REQUIRE(owner.Initialize(Device()));
    const std::vector<std::uint8_t> top(std::size_t{64} * 64u * 4u, 0);
    auto chain = Chain(64);
    chain[1] = {20, 20, std::vector<std::uint8_t>(std::size_t{20} * 20u * 4u, 2)};

    const auto result = owner.Upload(top, 64, 64, filter, wrap, chain);
    CHECK_FALSE(result);
    CHECK(fake.textures == 0);
    CHECK(fake.uploads.empty());
    CHECK(owner.LastStage() == "upload: invalid mip chain");
}

TEST_CASE("bitmap uploads share a submission until the slot is full or the flush")
{
    Fake fake;
    Owner owner(fake.Calls());
    REQUIRE(owner.Initialize(Device()));
    // 65 pixels of 4 bytes take a 512-byte pitch; two rows are 1024 bytes, a multiple of the level alignment.
    std::vector<std::uint8_t> small(std::size_t{65} * 2u * 4u, 0x5a);
    for (int i = 0; i < 100; ++i)
    {
        auto result = owner.Upload(small, 65, 2, filter, wrap);
        REQUIRE(result);
        CHECK(fake.source.offset == static_cast<Uint32>(i) * 1024u);
        fake.Calls().releaseTexture(Device(), result.texture);
    }
    CHECK(fake.fences == 0);
    CHECK(fake.waits == 0);
    CHECK(owner.Flush());
    CHECK(fake.fences == 1);

    // A full-size image fills a slot: each one submits the batch it started.
    std::vector<std::uint8_t> full(std::size_t{1024} * 1024u * 4u, 0x11);
    for (int i = 0; i < 5; ++i)
    {
        auto result = owner.Upload(full, 1024, 1024, filter, wrap);
        REQUIRE(result);
        CHECK(fake.source.offset == 0);
        fake.Calls().releaseTexture(Device(), result.texture);
    }
    CHECK(owner.Flush());
    CHECK(owner.Flush());
    CHECK(fake.waits == 0);
    CHECK(fake.transfers == 3); // the GPU kept up: no slot was added
    // A finished slot is still waited on, which returns at once: SDL's Vulkan backend cleans up there.
    CHECK(fake.finishedWaits > 0);
    CHECK_FALSE(fake.overwrite);
    CHECK(owner.Shutdown());
    CHECK(fake.fences == 0);
}

TEST_CASE("an open slot at shutdown is submitted and retired")
{
    Fake fake;
    Owner owner(fake.Calls());
    REQUIRE(owner.Initialize(Device()));
    std::vector<std::uint8_t> small(std::size_t{65} * 2u * 4u, 0x5a);
    auto result = owner.Upload(small, 65, 2, filter, wrap);
    REQUIRE(result);
    CHECK(fake.fences == 0);
    CHECK(owner.Shutdown());
    CHECK(fake.fences == 0);
    CHECK(fake.waits == 1);
}

TEST_CASE("a busy ring grows instead of waiting, reports what is in flight, and trims back once idle")
{
    Fake fake;
    Owner owner(fake.Calls());
    REQUIRE(owner.Initialize(Device()));
    fake.gpuFinishes = false;
    std::vector<std::uint8_t> full(std::size_t{1024} * 1024u * 4u, 0x11);
    // Each full image fills a slot; the GPU finishes none of them.
    for (int i = 0; i < 10; ++i)
    {
        auto result = owner.Upload(full, 1024, 1024, filter, wrap);
        REQUIRE(result);
        fake.Calls().releaseTexture(Device(), result.texture);
    }
    CHECK(fake.waits == 0);
    CHECK(fake.transfers == 10);
    CHECK_FALSE(fake.overwrite);
    CHECK(owner.BytesInFlight() == 10 * Owner::kSlotCapacity); // nine submitted, one open and full
    CHECK(owner.OverStagingCap());

    fake.gpuFinishes = true;
    CHECK(owner.BytesInFlight() == Owner::kSlotCapacity); // only the open slot
    CHECK_FALSE(owner.OverStagingCap());
    const int releasesBefore = fake.releases;
    for (int frame = 1; frame < Owner::kIdleFramesBeforeTrim; ++frame)
        owner.TrimIdleSlots();
    CHECK(fake.releases == releasesBefore);
    owner.TrimIdleSlots();
    // Down to the minimum; the open slot is kept.
    CHECK(fake.releases - releasesBefore == 10 - static_cast<int>(Owner::kMinimumSlotCount));

    // The trimmed ring still works.
    auto after = owner.Upload(full, 1024, 1024, filter, wrap);
    REQUIRE(after);
    fake.Calls().releaseTexture(Device(), after.texture);
    CHECK(owner.Shutdown());
    CHECK(fake.fences == 0);
}

TEST_CASE("an upload outside any load reaches the GPU with the next flush, even frames later")
{
    Fake fake;
    Owner owner(fake.Calls());
    REQUIRE(owner.Initialize(Device()));
    std::vector<std::uint8_t> small(std::size_t{65} * 2u * 4u, 0x5a);
    auto first = owner.Upload(small, 65, 2, filter, wrap);
    REQUIRE(first);
    owner.TrimIdleSlots(); // a frame passes without a flush (no frame was drawn)
    auto second = owner.Upload(small, 65, 2, filter, wrap);
    REQUIRE(second);
    CHECK(fake.fences == 0);
    REQUIRE(fake.uploads.size() == 2);
    CHECK(fake.uploads[0].first.transfer_buffer == fake.uploads[1].first.transfer_buffer);
    CHECK(owner.Flush());
    CHECK(fake.fences == 1);
    CHECK(fake.waits == 0);
    fake.Calls().releaseTexture(Device(), first.texture);
    fake.Calls().releaseTexture(Device(), second.texture);
    CHECK(owner.Shutdown());
}

TEST_CASE("runtime uploads carry a submission across frames and an upload larger than the open slot")
{
    Fake fake;
    Owner owner(fake.Calls());
    REQUIRE(owner.Initialize(Device()));
    std::vector<std::uint8_t> small(std::size_t{65} * 2u * 4u, 0x5a);
    for (int frame = 0; frame < 4; ++frame)
    {
        auto result = owner.Upload(small, 65, 2, filter, wrap);
        REQUIRE(result);
        CHECK(fake.source.offset == 0u); // each frame opens a fresh slot after its flush
        fake.Calls().releaseTexture(Device(), result.texture);
        CHECK(owner.Flush());
    }
    CHECK(fake.fences == 3);
    CHECK_FALSE(owner.IsFaulted());
    // A 1024x1024 RGBA image is exactly one slot: it fills the open slot by itself.
    std::vector<std::uint8_t> large(Owner::kSlotCapacity, 0x3c);
    auto big = owner.Upload(large, 1024, 1024, filter, wrap);
    REQUIRE(big);
    CHECK(fake.source.offset == 0u);
    CHECK(fake.uploads.size() == 5u); // four small frames plus the large image
    fake.Calls().releaseTexture(Device(), big.texture);
    CHECK(owner.Flush());
    CHECK_FALSE(owner.IsFaulted());
    CHECK(owner.Shutdown());
    CHECK(fake.fences == 0);
}

TEST_CASE("without memory for another slot a busy ring waits, and a failed wait loses only that slot")
{
    Fake fake;
    Owner owner(fake.Calls());
    REQUIRE(owner.Initialize(Device()));
    fake.gpuFinishes = false;
    fake.failGrow = true;
    std::vector<std::uint8_t> full(std::size_t{1024} * 1024u * 4u, 0x11);
    for (int i = 0; i < 4; ++i)
    {
        auto result = owner.Upload(full, 1024, 1024, filter, wrap);
        REQUIRE(result);
        fake.Calls().releaseTexture(Device(), result.texture);
    }
    CHECK(fake.transfers == 3);
    CHECK(fake.waits == 1);
    CHECK_FALSE(fake.overwrite);

    fake.failWait = true;
    CHECK_FALSE(owner.Upload(full, 1024, 1024, filter, wrap));
    CHECK_FALSE(owner.IsFaulted()); // that slot only
    owner.Shutdown();
}

TEST_CASE("a busy ring stops growing at its ceiling and waits there")
{
    Fake fake;
    Owner owner(fake.Calls());
    REQUIRE(owner.Initialize(Device()));
    fake.gpuFinishes = false;
    std::vector<std::uint8_t> full(std::size_t{1024} * 1024u * 4u, 0x11);
    for (std::size_t i = 0; i <= Owner::kMaximumSlotCount; ++i)
    {
        auto result = owner.Upload(full, 1024, 1024, filter, wrap);
        REQUIRE(result);
        fake.Calls().releaseTexture(Device(), result.texture);
    }
    CHECK(fake.transfers == static_cast<int>(Owner::kMaximumSlotCount));
    CHECK(fake.waits == 1);
    CHECK_FALSE(fake.overwrite);
    owner.Shutdown();
}

TEST_CASE("a trim that removes the next slot never leaves the ring on a lost one")
{
    Fake fake;
    Owner owner(fake.Calls());
    REQUIRE(owner.Initialize(Device()));
    std::vector<std::uint8_t> full(std::size_t{1024} * 1024u * 4u, 0x11);
    const auto upload = [&]
    {
        auto result = owner.Upload(full, 1024, 1024, filter, wrap);
        if (result)
            fake.Calls().releaseTexture(Device(), result.texture);
        return static_cast<bool>(result);
    };
    // Four busy slots; the fourth fails to submit and is lost; a fifth is added.
    fake.gpuFinishes = false;
    for (int i = 0; i < 4; ++i)
        REQUIRE(upload());
    fake.failSubmit = true;
    CHECK_FALSE(upload());
    fake.failSubmit = false;
    REQUIRE(upload());
    auto* const lost = Handle<SDL_GPUTransferBuffer>(Fake::kTransferBase + 4);
    // The GPU catches up; two uploads leave the next slot just before the lost one, with nothing open.
    fake.gpuFinishes = true;
    CHECK(owner.Flush());
    REQUIRE(upload());
    REQUIRE(upload());
    CHECK(owner.Flush());
    for (int frame = 0; frame < Owner::kIdleFramesBeforeTrim; ++frame)
        owner.TrimIdleSlots();
    // Every live slot busy and no memory for another: the upload waits on a live slot, not the lost one.
    fake.gpuFinishes = false;
    fake.failGrow = true;
    REQUIRE(upload());
    CHECK(fake.waits == 1);
    CHECK(fake.uploads.back().first.transfer_buffer != lost);
    owner.Shutdown();
}

TEST_CASE("trimming never removes the last usable slot when lost slots fill the minimum")
{
    Fake fake;
    Owner owner(fake.Calls());
    REQUIRE(owner.Initialize(Device()));
    std::vector<std::uint8_t> full(std::size_t{1024} * 1024u * 4u, 0x11);
    const auto upload = [&]
    {
        auto result = owner.Upload(full, 1024, 1024, filter, wrap);
        if (result)
            fake.Calls().releaseTexture(Device(), result.texture);
        return static_cast<bool>(result);
    };
    // Four slots, the fourth open; it is lost to a failed submit.
    fake.gpuFinishes = false;
    for (int i = 0; i < 4; ++i)
        REQUIRE(upload());
    fake.failSubmit = true;
    CHECK_FALSE(owner.Flush());
    fake.failSubmit = false;
    // The GPU catches up; slots 0 and 1 are lost the same way. Slot 2 is the only usable one left.
    fake.gpuFinishes = true;
    for (int i = 0; i < 2; ++i)
    {
        REQUIRE(upload());
        fake.failSubmit = true;
        CHECK_FALSE(owner.Flush());
        fake.failSubmit = false;
    }
    CHECK_FALSE(owner.IsFaulted());
    for (int frame = 0; frame < Owner::kIdleFramesBeforeTrim; ++frame)
        owner.TrimIdleSlots();
    // Slot 2 was kept, so the ring still uploads.
    CHECK_FALSE(owner.IsFaulted());
    CHECK(upload());
    owner.Shutdown();
}
