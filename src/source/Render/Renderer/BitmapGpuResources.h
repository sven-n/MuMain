#pragma once

#include "Render/Sprites/BitmapTextureLimits.h"

#include <SDL3/SDL_gpu.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <string>
#include <thread>

namespace Render::Renderer
{
class BitmapGpuResources final
{
public:
    static constexpr std::size_t kSlotCount = 3;
    static constexpr std::uint64_t kBytesPerPixel = 4;
    static constexpr std::uint64_t kRowAlignment = 256;
    static_assert(Sprites::kBitmapMaximumWidth > 0 && Sprites::kBitmapMaximumHeight > 0);
    static_assert(static_cast<std::uint64_t>(Sprites::kBitmapMaximumWidth) <=
                  (std::numeric_limits<std::uint64_t>::max() - kRowAlignment + 1) / kBytesPerPixel);
    static constexpr std::uint64_t kMaximumPitch =
        (static_cast<std::uint64_t>(Sprites::kBitmapMaximumWidth) * kBytesPerPixel + kRowAlignment - 1) /
        kRowAlignment * kRowAlignment;
    static_assert(kMaximumPitch <= std::numeric_limits<std::uint64_t>::max() /
                                       static_cast<std::uint64_t>(Sprites::kBitmapMaximumHeight));
    static constexpr std::uint64_t kSlotCapacity =
        kMaximumPitch * static_cast<std::uint64_t>(Sprites::kBitmapMaximumHeight);
    static_assert(kSlotCapacity <= std::numeric_limits<Uint32>::max());
    static_assert(kSlotCapacity <= std::numeric_limits<std::size_t>::max());

    struct Calls
    {
        decltype(&SDL_CreateGPUTransferBuffer) createTransfer = &SDL_CreateGPUTransferBuffer;
        decltype(&SDL_ReleaseGPUTransferBuffer) releaseTransfer = &SDL_ReleaseGPUTransferBuffer;
        decltype(&SDL_CreateGPUSampler) createSampler = &SDL_CreateGPUSampler;
        decltype(&SDL_ReleaseGPUSampler) releaseSampler = &SDL_ReleaseGPUSampler;
        decltype(&SDL_CreateGPUTexture) createTexture = &SDL_CreateGPUTexture;
        decltype(&SDL_ReleaseGPUTexture) releaseTexture = &SDL_ReleaseGPUTexture;
        decltype(&SDL_MapGPUTransferBuffer) map = &SDL_MapGPUTransferBuffer;
        decltype(&SDL_UnmapGPUTransferBuffer) unmap = &SDL_UnmapGPUTransferBuffer;
        decltype(&SDL_AcquireGPUCommandBuffer) acquire = &SDL_AcquireGPUCommandBuffer;
        decltype(&SDL_BeginGPUCopyPass) beginCopy = &SDL_BeginGPUCopyPass;
        decltype(&SDL_UploadToGPUTexture) upload = &SDL_UploadToGPUTexture;
        decltype(&SDL_EndGPUCopyPass) endCopy = &SDL_EndGPUCopyPass;
        decltype(&SDL_CancelGPUCommandBuffer) cancel = &SDL_CancelGPUCommandBuffer;
        decltype(&SDL_SubmitGPUCommandBufferAndAcquireFence) submit = &SDL_SubmitGPUCommandBufferAndAcquireFence;
        decltype(&SDL_WaitForGPUFences) wait = &SDL_WaitForGPUFences;
        decltype(&SDL_ReleaseGPUFence) releaseFence = &SDL_ReleaseGPUFence;
        decltype(&SDL_GetError) getError = &SDL_GetError;
    };

    struct Result
    {
        SDL_GPUTexture* texture = nullptr; // Caller owns this handle on success.
        SDL_GPUSampler* sampler = nullptr; // Borrowed until Shutdown.
        std::string stage;
        std::string error;
        explicit operator bool() const
        {
            return texture != nullptr;
        }
    };

    BitmapGpuResources();
    explicit BitmapGpuResources(Calls calls);
    ~BitmapGpuResources() = default;
    BitmapGpuResources(const BitmapGpuResources&) = delete;
    BitmapGpuResources& operator=(const BitmapGpuResources&) = delete;

    // Initialize/Shutdown and destruction must be externally serialized with all calls.
    // During an initialized lifetime, wrong-thread uploads reject without changing owner state.
    bool Initialize(SDL_GPUDevice* device);
    Result Upload(std::span<const std::uint8_t> pixels, int width, int height, SDL_GPUFilter filter,
                  SDL_GPUSamplerAddressMode wrap);
    // Call before destroying the device, after clearing non-owning sampler registries.
    bool Shutdown();
    const std::string& LastError() const
    {
        return m_error;
    }
    const std::string& LastStage() const
    {
        return m_stage;
    }
    bool IsFaulted() const
    {
        return m_faulted;
    }

private:
    struct Slot
    {
        SDL_GPUTransferBuffer* transfer = nullptr;
        SDL_GPUFence* fence = nullptr;
    };
    bool OnOwnerThread() const;
    void Fail(const char* stage, bool gpuError);
    bool Retire(Slot& slot);
    void ReleaseHandles();
    Result Failure() const;

    Calls m_calls;
    SDL_GPUDevice* m_device = nullptr;
    std::thread::id m_thread;
    std::array<Slot, kSlotCount> m_slots{};
    std::array<SDL_GPUSampler*, 4> m_samplers{};
    std::size_t m_next = 0;
    bool m_faulted = false;
    std::string m_stage;
    std::string m_error;
};
} // namespace Render::Renderer
