#include <doctest.h>

#include "Render/Renderer/BitmapGpuResources.h"

#include <algorithm>
#include <array>
#include <barrier>
#include <cstdint>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

namespace
{
using Owner = Render::Renderer::BitmapGpuResources;
template <typename T> T* Handle(int n);
template <typename T> int Index(T* handle);

struct Fake
{
    static inline Fake* active = nullptr;
    std::array<int, 128> tokens{};
    int transfers = 0, samplers = 0, textures = 0, waits = 0, releases = 0, fences = 0;
    int failureAt = -1, creates = 0;
    const char* failure = "original SDL error";
    bool failWait = false, failSubmit = false, failMap = false, failAcquire = false, failBegin = false;
    bool failTexture = false;
    bool mapped = false, cycled = false, overwrite = false, canceled = false;
    std::array<std::vector<std::uint8_t>, Owner::kSlotCount> bytes;
    std::array<bool, Owner::kSlotCount> busy{};
    std::vector<SDL_GPUSamplerCreateInfo> descriptors;
    SDL_GPUTextureTransferInfo source{};
    SDL_GPUTextureRegion destination{};
    std::string lastEvent;

    Fake()
    {
        active = this;
        for (auto& b : bytes)
            b.resize(Owner::kSlotCapacity);
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
        if (active->ShouldFail())
            return nullptr;
        return Handle<SDL_GPUTransferBuffer>(++active->transfers);
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
        ++active->textures;
        return active->failTexture ? nullptr : Handle<SDL_GPUTexture>(40 + active->textures % 20);
    }
    static void ReleaseTexture(SDL_GPUDevice*, SDL_GPUTexture*)
    {
        ++active->releases;
    }
    static void* Map(SDL_GPUDevice*, SDL_GPUTransferBuffer* buffer, bool cycle)
    {
        auto index = (Index(buffer) - 1) % static_cast<int>(Owner::kSlotCount);
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
    }
    static void End(SDL_GPUCopyPass*) {}
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
        const auto slot = (Index(active->source.transfer_buffer) - 1) % static_cast<int>(Owner::kSlotCount);
        active->busy[slot] = true;
        ++active->fences;
        return Handle<SDL_GPUFence>(static_cast<int>(slot) + 1);
    }
    static bool Wait(SDL_GPUDevice*, bool all, SDL_GPUFence* const* fences, Uint32 count)
    {
        CHECK(all);
        CHECK(count == 1);
        ++active->waits;
        if (active->failWait)
            return false;
        active->busy[Index(*fences) - 1] = false;
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
        c.cancel = Cancel;
        c.submit = Submit;
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

TEST_CASE("bitmap owner uses only three uncycled slots for repeated uploads")
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
        CHECK(fake.source.offset == 0);
        CHECK(fake.source.pixels_per_row == 128);
        CHECK(fake.source.rows_per_layer == 2);
        CHECK(fake.destination.w == 65);
        CHECK(fake.destination.h == 2);
        fake.Calls().releaseTexture(Device(), result.texture);
    }
    CHECK(fake.transfers == 3);
    CHECK(fake.samplers == 4);
    CHECK(fake.waits == 10001 - 3);
    CHECK(fake.fences == 3);
    CHECK_FALSE(fake.overwrite);
    CHECK_FALSE(fake.cycled);
    CHECK(owner.Shutdown());
    CHECK(fake.fences == 0);
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
        const auto slot = (Index(fake.source.transfer_buffer) - 1) % static_cast<int>(Owner::kSlotCount);
        const auto pitch = fake.source.pixels_per_row * 4;
        CHECK(std::memcmp(fake.bytes[slot].data(), pixels.data(), static_cast<std::size_t>(width) * 4u) == 0);
        const auto visibleRowBytes = static_cast<std::size_t>(width) * 4u;
        CHECK(std::memcmp(fake.bytes[slot].data() + pitch, pixels.data() + visibleRowBytes, visibleRowBytes) == 0);
        if (fake.waits == 0) // Fresh slots have zeroed poison space; reuse need not clear padding.
        {
            CHECK(std::all_of(fake.bytes[slot].begin() + visibleRowBytes, fake.bytes[slot].begin() + pitch,
                              [](std::uint8_t byte) { return byte == 0; }));
            CHECK(std::all_of(fake.bytes[slot].begin() + pitch + visibleRowBytes,
                              fake.bytes[slot].begin() + std::size_t{2} * pitch,
                              [](std::uint8_t byte) { return byte == 0; }));
        }
        fake.Calls().releaseTexture(Device(), result.texture);
    }
    std::vector<std::uint8_t> maximum(Owner::kSlotCapacity, 0x2a);
    auto maxResult = owner.Upload(maximum, 1024, 1024, filter, wrap);
    REQUIRE(maxResult);
    fake.Calls().releaseTexture(Device(), maxResult.texture);
    const int before = fake.textures;
    CHECK_FALSE(owner.Upload(maximum, 0, 1, filter, wrap));
    CHECK_FALSE(owner.Upload(maximum, -1, 1, filter, wrap));
    CHECK_FALSE(owner.Upload(maximum, 1025, 1, filter, wrap));
    CHECK_FALSE(owner.Upload(maximum, 1, 1025, filter, wrap));
    CHECK_FALSE(owner.Upload(std::span(maximum).first(3), 1, 1, filter, wrap));
    CHECK_FALSE(owner.Upload(maximum, 1, 1, static_cast<SDL_GPUFilter>(99), wrap));
    CHECK(fake.textures == before);
    CHECK(owner.Shutdown());
}

TEST_CASE("bitmap owner faults on GPU errors and preserves error before cleanup")
{
    for (int stage = 0; stage < 5; ++stage)
    {
        Fake fake;
        Owner owner(fake.Calls());
        REQUIRE(owner.Initialize(Device()));
        fake.failTexture = stage == 0;
        fake.failMap = stage == 1;
        fake.failAcquire = stage == 2;
        fake.failBegin = stage == 3;
        fake.failSubmit = stage == 4;
        std::array<std::uint8_t, 4> pixels{};
        CHECK_FALSE(owner.Upload(pixels, 1, 1, filter, wrap));
        CHECK(owner.IsFaulted());
        CHECK(owner.LastError() == "original SDL error");
        CHECK(fake.canceled == (stage == 3));
        CHECK_FALSE(owner.Upload(pixels, 1, 1, filter, wrap));
        CHECK(fake.textures == 1);
        owner.Shutdown();
    }
    Fake fake;
    Owner owner(fake.Calls());
    REQUIRE(owner.Initialize(Device()));
    std::array<std::uint8_t, 4> pixels{};
    auto result = owner.Upload(pixels, 1, 1, filter, wrap);
    REQUIRE(result);
    fake.Calls().releaseTexture(Device(), result.texture);
    for (int i = 0; i < 2; ++i)
    {
        result = owner.Upload(pixels, 1, 1, filter, wrap);
        REQUIRE(result);
        fake.Calls().releaseTexture(Device(), result.texture);
    }
    fake.failWait = true;
    CHECK_FALSE(owner.Upload(pixels, 1, 1, filter, wrap));
    CHECK(owner.IsFaulted());
    CHECK(fake.fences == 3);
    CHECK(fake.textures == 3);
    CHECK(owner.Shutdown());
    CHECK(fake.fences == 0);
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
