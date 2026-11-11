#pragma once

#include "Render/Sprites/BitmapTextureLimits.h"
#include "Render/Textures/MipChain.h"

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
#if defined(__APPLE__)
    // SDL's Metal backend (up to 3.4.20 at least) ignores pixels_per_row and reads tightly packed rows
    // (libsdl-org/SDL#16474). Drop this once the SDL submodule includes libsdl-org/SDL#16610.
    static constexpr std::uint64_t kRowAlignment = kBytesPerPixel;
#else
    static constexpr std::uint64_t kRowAlignment = 256;
#endif
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
    // Upscaled images: wider ones need at least one row per slot.
    static constexpr int kMaximumWidth = Sprites::kBitmapMaximumWidth * Sprites::kBitmapMaximumScale;
    static constexpr int kMaximumHeight = Sprites::kBitmapMaximumHeight * Sprites::kBitmapMaximumScale;
    static_assert(kMaximumPitch * Sprites::kBitmapMaximumScale <= kSlotCapacity);
    // Where a mip level starts when it shares a slot with the level before it:
    // the D3D12 placement alignment, which keeps every backend on its direct
    // copy path.
    static constexpr std::uint64_t kLevelAlignment = 512;

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
        decltype(&SDL_GenerateMipmapsForGPUTexture) generateMipmaps = &SDL_GenerateMipmapsForGPUTexture;
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
    // `mips` are the levels below `pixels`, each half the size of the one
    // before it (at least 1x1); with `generateMips` instead, the GPU builds the
    // whole chain after the upload. A level larger than one slot is staged in
    // bands of rows; consecutive pieces that fit one slot share a submission.
    Result Upload(std::span<const std::uint8_t> pixels, int width, int height, SDL_GPUFilter filter,
                  SDL_GPUSamplerAddressMode wrap, std::span<const Textures::MipLevel> mips = {},
                  bool generateMips = false);
    // Between BeginBatch and EndBatch, uploads share a submission until a slot is
    // full, so a load waits on a fence per slot instead of one per texture. A
    // texture is ready on the GPU once its submission is in: Flush submits the
    // open one, and the frame submit must come after it.
    void BeginBatch();
    bool EndBatch();
    bool Flush();
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
    // Every slot lost a submission or a fence wait; uploads stay off until Shutdown.
    bool IsFaulted() const
    {
        return m_faulted;
    }
    // Time spent waiting for a slot's previous upload to finish.
    double FenceWaitSeconds() const
    {
        return m_fenceWaitSeconds;
    }

private:
    struct Slot
    {
        SDL_GPUTransferBuffer* transfer = nullptr;
        SDL_GPUFence* fence = nullptr;
        // A failed submission or fence wait leaves the GPU's use of it unknown: never reused.
        bool lost = false;
    };
    bool OnOwnerThread() const;
    void Fail(const char* stage, bool gpuError);
    bool Retire(Slot& slot);
    bool SubmitOpen();
    void LoseSlot(Slot& slot);
    void AdvanceSlot();
    void ReleaseHandles();
    Result Failure() const;

    Calls m_calls;
    SDL_GPUDevice* m_device = nullptr;
    std::thread::id m_thread;
    std::array<Slot, kSlotCount> m_slots{};
    std::array<SDL_GPUSampler*, 4> m_samplers{};
    std::size_t m_next = 0;
    // The command buffer staging into m_slots[m_next], and how much of that slot it used.
    SDL_GPUCommandBuffer* m_open = nullptr;
    std::uint64_t m_openUsed = 0;
    bool m_batching = false;
    bool m_faulted = false;
    double m_fenceWaitSeconds = 0.0;
    std::string m_stage;
    std::string m_error;
};
} // namespace Render::Renderer
