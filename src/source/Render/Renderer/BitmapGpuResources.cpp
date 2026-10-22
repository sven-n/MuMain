#include "BitmapGpuResources.h"

#include <SDL3/SDL_error.h>

#include <algorithm>
#include <cstring>
#include <vector>

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
}

void BitmapGpuResources::LoseSlot(Slot& slot)
{
    slot.lost = true;
    m_faulted = std::all_of(m_slots.begin(), m_slots.end(), [](const Slot& each) { return each.lost; });
    if (!m_faulted)
    {
        AdvanceSlot();
    }
}

void BitmapGpuResources::AdvanceSlot()
{
    do
    {
        m_next = (m_next + 1) % kSlotCount;
    } while (m_slots[m_next].lost);
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
            m_faulted = true; // no retry on this device lifetime
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
            m_faulted = true;
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
        LoseSlot(slot);
        return false;
    }
    m_calls.releaseFence(m_device, slot.fence);
    slot.fence = nullptr;
    return true;
}

namespace
{
// A band of rows of one mip level, staged in one slot.
struct StagedPiece
{
    const std::uint8_t* pixels = nullptr;
    std::uint32_t level = 0;
    std::uint64_t width = 0;
    std::uint64_t top = 0;
    std::uint64_t rows = 0;
    std::uint64_t rowBytes = 0;
    std::uint64_t pitch = 0;
    std::uint64_t offset = 0;
};

std::uint64_t AlignUp(std::uint64_t value, std::uint64_t alignment)
{
    return (value + alignment - 1) / alignment * alignment;
}

// The base level and every halving of the longer side down to 1 pixel.
Uint32 MipLevelCount(int width, int height)
{
    Uint32 levels = 1;
    for (int size = std::max(width, height); size > 1; size >>= 1)
    {
        ++levels;
    }
    return levels;
}
} // namespace

BitmapGpuResources::Result BitmapGpuResources::Upload(std::span<const std::uint8_t> pixels, int width, int height,
                                                      SDL_GPUFilter filter, SDL_GPUSamplerAddressMode wrap,
                                                      std::span<const Textures::MipLevel> mips, bool generateMips)
{
    if (!m_device)
    {
        return {nullptr, nullptr, "upload: uninitialized", "the bitmap uploader is not initialized"};
    }
    if (!OnOwnerThread())
    {
        // A rejected caller must not race with the owner's diagnostics or SDL calls.
        return {nullptr, nullptr, "upload: wrong thread", "upload must run on the owner thread"};
    }
    if (m_faulted)
    {
        return {nullptr, nullptr, "upload: faulted", m_error};
    }
    if (width <= 0 || height <= 0 || width > kMaximumWidth || height > kMaximumHeight ||
        (filter != SDL_GPU_FILTER_NEAREST && filter != SDL_GPU_FILTER_LINEAR) ||
        (wrap != SDL_GPU_SAMPLERADDRESSMODE_REPEAT && wrap != SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE))
    {
        Fail("upload: invalid dimensions or sampler configuration", false);
        return Failure();
    }

    // Each level in bands of rows that fit one slot: a whole level up to the unscaled limits.
    std::vector<StagedPiece> pieces;
    std::uint64_t levelWidth = static_cast<std::uint64_t>(width);
    std::uint64_t levelHeight = static_cast<std::uint64_t>(height);
    const auto addLevel = [&](std::span<const std::uint8_t> levelPixels, std::uint32_t level)
    {
        const std::uint64_t rowBytes = levelWidth * kBytesPerPixel;
        const std::uint64_t pitch = AlignUp(rowBytes, kRowAlignment);
        if (levelPixels.size() < rowBytes * levelHeight)
        {
            return false;
        }
        const std::uint64_t bandRows = kSlotCapacity / pitch;
        for (std::uint64_t top = 0; top < levelHeight; top += bandRows)
        {
            pieces.push_back({levelPixels.data() + top * rowBytes, level, levelWidth, top,
                              std::min(bandRows, levelHeight - top), rowBytes, pitch});
        }
        return true;
    };
    if (!addLevel(pixels, 0))
    {
        Fail("upload: insufficient pixels or staging capacity", false);
        return Failure();
    }
    for (std::uint32_t level = 1; level <= mips.size(); ++level)
    {
        const Textures::MipLevel& mip = mips[level - 1];
        const bool belowSmallest = levelWidth == 1 && levelHeight == 1;
        levelWidth = std::max<std::uint64_t>(1, levelWidth / 2);
        levelHeight = std::max<std::uint64_t>(1, levelHeight / 2);
        if (belowSmallest || mip.width != levelWidth || mip.height != levelHeight || !addLevel(mip.rgba, level))
        {
            Fail("upload: invalid mip chain", false);
            return Failure();
        }
    }
    const Uint32 levelCount = generateMips && mips.empty() ? MipLevelCount(width, height)
                                                           : static_cast<Uint32>(mips.size() + 1);

    SDL_GPUTexture* texture = nullptr;
    std::size_t next = 0;
    while (next < pieces.size())
    {
        auto& slot = m_slots[m_next];
        if (!Retire(slot))
        {
            if (texture)
            {
                m_calls.releaseTexture(m_device, texture);
            }
            return Failure();
        }

        if (!texture)
        {
            SDL_GPUTextureCreateInfo textureInfo{};
            textureInfo.type = SDL_GPU_TEXTURETYPE_2D;
            textureInfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
            textureInfo.width = static_cast<Uint32>(width);
            textureInfo.height = static_cast<Uint32>(height);
            textureInfo.layer_count_or_depth = 1;
            textureInfo.num_levels = levelCount;
            // The GPU renders the lower levels it builds into the texture.
            textureInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER |
                                (generateMips && levelCount > 1 ? SDL_GPU_TEXTUREUSAGE_COLOR_TARGET
                                                                : SDL_GPUTextureUsageFlags{0});
            texture = m_calls.createTexture(m_device, &textureInfo);
            if (!texture)
            {
                Fail("create bitmap texture", true);
                return Failure();
            }
        }

        void* mapped = m_calls.map(m_device, slot.transfer, false);
        if (!mapped)
        {
            Fail("map bitmap transfer buffer", true);
            m_calls.releaseTexture(m_device, texture);
            return Failure();
        }
        // As many consecutive pieces as fit the slot; a single piece always does.
        auto* target = static_cast<std::uint8_t*>(mapped);
        const std::size_t first = next;
        std::uint64_t used = 0;
        while (next < pieces.size())
        {
            StagedPiece& piece = pieces[next];
            const std::uint64_t offset = next == first ? 0 : AlignUp(used, kLevelAlignment);
            if (offset + piece.pitch * piece.rows > kSlotCapacity)
            {
                break;
            }
            for (std::uint64_t row = 0; row < piece.rows; ++row)
            {
                std::memcpy(target + offset + row * piece.pitch, piece.pixels + row * piece.rowBytes,
                            static_cast<std::size_t>(piece.rowBytes));
            }
            piece.offset = offset;
            used = offset + piece.pitch * piece.rows;
            ++next;
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
        for (std::size_t index = first; index < next; ++index)
        {
            const StagedPiece& piece = pieces[index];
            SDL_GPUTextureTransferInfo source{};
            source.transfer_buffer = slot.transfer;
            source.offset = static_cast<Uint32>(piece.offset);
            source.pixels_per_row = static_cast<Uint32>(piece.pitch / kBytesPerPixel);
            source.rows_per_layer = static_cast<Uint32>(piece.rows);
            SDL_GPUTextureRegion destination{};
            destination.texture = texture;
            destination.mip_level = piece.level;
            destination.y = static_cast<Uint32>(piece.top);
            destination.w = static_cast<Uint32>(piece.width);
            destination.h = static_cast<Uint32>(piece.rows);
            destination.d = 1;
            m_calls.upload(pass, &source, &destination, false);
        }
        m_calls.endCopy(pass);
        if (generateMips && levelCount > 1 && next == pieces.size())
        {
            m_calls.generateMipmaps(command, texture);
        }

        // SDL consumes the command even on a failed submit. An uncertain slot is
        // never reused; only device teardown may reclaim it after that failure.
        SDL_GPUFence* fence = m_calls.submit(command);
        if (!fence)
        {
            Fail("submit bitmap upload and acquire fence", true);
            m_calls.releaseTexture(m_device, texture);
            LoseSlot(slot);
            return Failure();
        }
        slot.fence = fence;
        AdvanceSlot();
    }
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
    for (auto& slot : m_slots)
    {
        if (!slot.lost && !Retire(slot))
        {
            retired = false;
            break;
        }
    }
    // On device failure SDL handles remain valid until device destruction;
    // do not treat an unsuccessful wait as proof that staging can be reused.
    ReleaseHandles();
    m_slots = {};
    m_device = nullptr;
    m_thread = {};
    m_next = 0;
    m_faulted = false;
    return retired;
}
} // namespace Render::Renderer
