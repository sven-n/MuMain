#include "BitmapGpuResources.h"

#include <SDL3/SDL_error.h>

#include <cstring>

namespace Render::Renderer
{
BitmapGpuResources::BitmapGpuResources() = default;
BitmapGpuResources::BitmapGpuResources(Calls calls) : m_calls(calls) {}

bool BitmapGpuResources::OnOwnerThread() const
{
    return m_thread == std::this_thread::get_id();
}

void BitmapGpuResources::Fail(const char* stage, bool gpuError)
{
    m_stage = stage;
    const char* text = gpuError ? m_calls.getError() : nullptr;
    m_error = text && *text ? text : (gpuError ? "SDL GPU operation failed" : stage);
    if (gpuError)
    {
        m_faulted = true;
    }
}

BitmapGpuResources::Result BitmapGpuResources::Failure() const
{
    return {nullptr, nullptr, m_stage, m_error};
}

bool BitmapGpuResources::Initialize(SDL_GPUDevice* device)
{
    if (!device || m_device || m_faulted)
    {
        Fail("initialize: invalid device, already initialized or failed lifetime", false);
        return false;
    }
    m_device = device;
    m_thread = std::this_thread::get_id();
    m_faulted = false;
    m_next = 0;
    m_stage.clear();
    m_error.clear();

    SDL_GPUTransferBufferCreateInfo transferInfo{};
    transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transferInfo.size = static_cast<Uint32>(kSlotCapacity);
    for (auto& slot : m_slots)
    {
        slot.transfer = m_calls.createTransfer(device, &transferInfo);
        if (!slot.transfer)
        {
            Fail("initialize: create transfer buffer", true);
            ReleaseHandles();
            m_device = nullptr;
            return false;
        }
    }

    for (std::size_t i = 0; i < m_samplers.size(); ++i)
    {
        SDL_GPUSamplerCreateInfo info{};
        const auto filter = (i / 2 == 0) ? SDL_GPU_FILTER_NEAREST : SDL_GPU_FILTER_LINEAR;
        const auto wrap = (i % 2 == 0) ? SDL_GPU_SAMPLERADDRESSMODE_REPEAT : SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
        info.min_filter = filter;
        info.mag_filter = filter;
        info.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
        info.address_mode_u = wrap;
        info.address_mode_v = wrap;
        info.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
        m_samplers[i] = m_calls.createSampler(device, &info);
        if (!m_samplers[i])
        {
            Fail("initialize: create bitmap sampler", true);
            m_stage += (i < 2 ? " nearest/" : " linear/");
            m_stage += (i % 2 == 0 ? "repeat" : "clamp-to-edge");
            ReleaseHandles();
            m_device = nullptr;
            return false;
        }
    }
    return true;
}

bool BitmapGpuResources::Retire(Slot& slot)
{
    if (!slot.fence)
    {
        return true;
    }
    if (!m_calls.wait(m_device, true, &slot.fence, 1))
    {
        Fail("wait for upload fence", true);
        return false;
    }
    m_calls.releaseFence(m_device, slot.fence);
    slot.fence = nullptr;
    return true;
}

BitmapGpuResources::Result BitmapGpuResources::Upload(std::span<const std::uint8_t> pixels, int width, int height,
                                                      SDL_GPUFilter filter, SDL_GPUSamplerAddressMode wrap)
{
    if (!OnOwnerThread())
    {
        // A rejected caller must not race with the owner's diagnostics or SDL calls.
        return {nullptr, nullptr, "upload: wrong thread", "upload must run on the owner thread"};
    }
    if (!m_device || m_faulted)
    {
        Fail("upload: uninitialized or faulted", false);
        return Failure();
    }
    if (width <= 0 || height <= 0 || width > Sprites::kBitmapMaximumWidth || height > Sprites::kBitmapMaximumHeight ||
        (filter != SDL_GPU_FILTER_NEAREST && filter != SDL_GPU_FILTER_LINEAR) ||
        (wrap != SDL_GPU_SAMPLERADDRESSMODE_REPEAT && wrap != SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE))
    {
        Fail("upload: invalid dimensions or sampler configuration", false);
        return Failure();
    }
    const auto rowBytes = static_cast<std::uint64_t>(width) * kBytesPerPixel;
    const auto pitch = (rowBytes + kRowAlignment - 1) / kRowAlignment * kRowAlignment;
    const auto sourceBytes = rowBytes * static_cast<std::uint64_t>(height);
    const auto stagingBytes = pitch * static_cast<std::uint64_t>(height);
    if (pixels.size() < sourceBytes || stagingBytes > kSlotCapacity)
    {
        Fail("upload: insufficient pixels or staging capacity", false);
        return Failure();
    }

    auto& slot = m_slots[m_next];
    if (!Retire(slot))
    {
        return Failure();
    }

    SDL_GPUTextureCreateInfo textureInfo{};
    textureInfo.type = SDL_GPU_TEXTURETYPE_2D;
    textureInfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    textureInfo.width = static_cast<Uint32>(width);
    textureInfo.height = static_cast<Uint32>(height);
    textureInfo.layer_count_or_depth = 1;
    textureInfo.num_levels = 1;
    textureInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
    SDL_GPUTexture* texture = m_calls.createTexture(m_device, &textureInfo);
    if (!texture)
    {
        Fail("create bitmap texture", true);
        return Failure();
    }

    void* mapped = m_calls.map(m_device, slot.transfer, false);
    if (!mapped)
    {
        Fail("map bitmap transfer buffer", true);
        m_calls.releaseTexture(m_device, texture);
        return Failure();
    }
    auto* target = static_cast<std::uint8_t*>(mapped);
    for (int row = 0; row < height; ++row)
    {
        std::memcpy(target + static_cast<std::size_t>(row) * pitch,
                    pixels.data() + static_cast<std::size_t>(row) * rowBytes, static_cast<std::size_t>(rowBytes));
    }
    m_calls.unmap(m_device, slot.transfer);

    SDL_GPUCommandBuffer* command = m_calls.acquire(m_device);
    if (!command)
    {
        Fail("acquire bitmap upload command buffer", true);
        m_calls.releaseTexture(m_device, texture);
        return Failure();
    }
    SDL_GPUCopyPass* pass = m_calls.beginCopy(command);
    if (!pass)
    {
        Fail("begin bitmap upload copy pass", true);
        m_calls.cancel(command);
        m_calls.releaseTexture(m_device, texture);
        return Failure();
    }
    SDL_GPUTextureTransferInfo source{};
    source.transfer_buffer = slot.transfer;
    source.offset = 0;
    source.pixels_per_row = static_cast<Uint32>(pitch / kBytesPerPixel);
    source.rows_per_layer = static_cast<Uint32>(height);
    SDL_GPUTextureRegion destination{};
    destination.texture = texture;
    destination.w = static_cast<Uint32>(width);
    destination.h = static_cast<Uint32>(height);
    destination.d = 1;
    m_calls.upload(pass, &source, &destination, false);
    m_calls.endCopy(pass);

    // SDL consumes the command even on a failed submit. An uncertain slot is
    // never reused; only device teardown may reclaim it after that failure.
    SDL_GPUFence* fence = m_calls.submit(command);
    if (!fence)
    {
        Fail("submit bitmap upload and acquire fence", true);
        m_calls.releaseTexture(m_device, texture);
        return Failure();
    }
    slot.fence = fence;
    m_next = (m_next + 1) % kSlotCount;
    m_stage.clear();
    m_error.clear();
    const std::size_t samplerIndex =
        (filter == SDL_GPU_FILTER_LINEAR ? 2u : 0u) + (wrap == SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE ? 1u : 0u);
    return {texture, m_samplers[samplerIndex], {}, {}};
}

void BitmapGpuResources::ReleaseHandles()
{
    for (auto& slot : m_slots)
    {
        if (slot.fence)
        {
            m_calls.releaseFence(m_device, slot.fence);
            slot.fence = nullptr;
        }
        if (slot.transfer)
        {
            m_calls.releaseTransfer(m_device, slot.transfer);
            slot.transfer = nullptr;
        }
    }
    for (auto& sampler : m_samplers)
    {
        if (sampler)
        {
            m_calls.releaseSampler(m_device, sampler);
            sampler = nullptr;
        }
    }
}

bool BitmapGpuResources::Shutdown()
{
    if (!m_device)
    {
        m_faulted = false;
        m_thread = {};
        return true;
    }
    if (!OnOwnerThread())
    {
        // Preserve owner-thread diagnostics; lifecycle calls are externally serialized.
        return false;
    }
    bool retired = true;
    if (!m_faulted)
    {
        for (auto& slot : m_slots)
        {
            if (!Retire(slot))
            {
                retired = false;
                break;
            }
        }
    }
    // On device failure SDL handles remain valid until device destruction;
    // do not treat an unsuccessful wait as proof that staging can be reused.
    ReleaseHandles();
    m_device = nullptr;
    m_thread = {};
    m_next = 0;
    m_faulted = false;
    return retired;
}
} // namespace Render::Renderer
