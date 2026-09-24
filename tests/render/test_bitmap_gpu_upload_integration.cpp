#include "Render/Renderer/BitmapGpuResources.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

namespace
{
using Owner = Render::Renderer::BitmapGpuResources;
constexpr int kIterations = 1200;
constexpr int kSmallWidth = 65;
constexpr int kSmallHeight = 3;
constexpr int kMaxDimension = 1024;
constexpr int kUploadWidth = 1;
constexpr int kUploadHeight = 1;
struct ValidationLog
{
    SDL_LogOutputFunction previous = nullptr;
    void* previousData = nullptr;
    bool layerEnabled = false;
    int validationErrors = 0;
    static void Capture(void* userdata, int category, SDL_LogPriority priority, const char* message)
    {
        auto& log = *static_cast<ValidationLog*>(userdata);
        if (std::strstr(message, "Validation layers enabled"))
            log.layerEnabled = true;
        if (std::strstr(message, "Validation Error") || std::strstr(message, "VUID-"))
            ++log.validationErrors;
        if (log.previous)
            log.previous(log.previousData, category, priority, message);
    }
};
struct Dimensions
{
    int width;
    int height;
};
int transferCreates = 0;
int samplerCreates = 0;
int mappedWidth = 0;
constexpr std::uint8_t kPaddingPoison = 0xa5;
constexpr std::array<Dimensions, 3> kSizes{
    {{kUploadWidth, kUploadHeight}, {kSmallWidth, kSmallHeight}, {kMaxDimension, kMaxDimension}}};

void* MapWithPaddingPoison(SDL_GPUDevice* device, SDL_GPUTransferBuffer* transfer, bool cycle)
{
    void* mapped = SDL_MapGPUTransferBuffer(device, transfer, cycle);
    if (mapped && mappedWidth == kSmallWidth)
    {
        // Poison padded rows before the real uploader writes visible pixels.
        constexpr std::size_t kRowBytes = kSmallWidth * Owner::kBytesPerPixel;
        constexpr std::size_t kPaddedPitch =
            (kRowBytes + Owner::kRowAlignment - 1) / Owner::kRowAlignment * Owner::kRowAlignment;
        std::memset(mapped, kPaddingPoison, kPaddedPitch * kSmallHeight);
    }
    return mapped;
}

int SizeIndex(Dimensions size)
{
    for (std::size_t i = 0; i < kSizes.size(); ++i)
        if (kSizes[i].width == size.width && kSizes[i].height == size.height)
            return static_cast<int>(i);
    return -1;
}

SDL_GPUSampler* CountedSampler(SDL_GPUDevice* device, const SDL_GPUSamplerCreateInfo* info)
{
    ++samplerCreates;
    return SDL_CreateGPUSampler(device, info);
}
SDL_GPUTransferBuffer* CountedCreate(SDL_GPUDevice* device, const SDL_GPUTransferBufferCreateInfo* info)
{
    ++transferCreates;
    return SDL_CreateGPUTransferBuffer(device, info);
}

// Test-only allocator adapter: the production staging, copy, submit and fence
// calls remain real. This isolates staging pressure from destination creation.
struct Destinations
{
    static inline Destinations* active = nullptr;
    std::array<SDL_GPUTexture*, 3> handles{};
    int requests = 0;
    int allocations = 0;

    bool Initialize(SDL_GPUDevice* device)
    {
        active = this;
        for (std::size_t i = 0; i < kSizes.size(); ++i)
        {
            SDL_GPUTextureCreateInfo info{};
            info.type = SDL_GPU_TEXTURETYPE_2D;
            info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
            info.width = static_cast<Uint32>(kSizes[i].width);
            info.height = static_cast<Uint32>(kSizes[i].height);
            info.layer_count_or_depth = 1;
            info.num_levels = 1;
            info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
            handles[i] = SDL_CreateGPUTexture(device, &info);
            if (!handles[i])
                return false;
            ++allocations;
        }
        return true;
    }
    void Release(SDL_GPUDevice* device)
    {
        for (auto*& texture : handles)
        {
            if (texture)
                SDL_ReleaseGPUTexture(device, texture);
            texture = nullptr;
        }
        active = nullptr;
    }
    static SDL_GPUTexture* Create(SDL_GPUDevice*, const SDL_GPUTextureCreateInfo* info)
    {
        ++active->requests;
        const int index = SizeIndex({static_cast<int>(info->width), static_cast<int>(info->height)});
        return index < 0 ? nullptr : active->handles[static_cast<std::size_t>(index)];
    }
    static void BorrowedRelease(SDL_GPUDevice*, SDL_GPUTexture*) {}
};

int CountDescriptors(bool nvidia)
{
#ifdef __linux__
    int count = 0;
    for (const auto& entry : std::filesystem::directory_iterator("/proc/self/fd"))
    {
        ++count;
        if (nvidia)
        {
            std::error_code error;
            auto path = std::filesystem::read_symlink(entry.path(), error).string();
            if (!error && path.find("/dev/nvidia") != std::string::npos)
            {
                // /dev/nvidia fds are reported separately, not assumed to be staging allocations.
            }
            else
            {
                --count;
            }
        }
    }
    return count;
#else
    (void)nvidia;
    return -1;
#endif
}

std::vector<std::uint8_t> Pattern(Dimensions size, int seed)
{
    std::vector<std::uint8_t> pixels(static_cast<std::size_t>(size.width) * static_cast<std::size_t>(size.height) * 4);
    for (std::size_t i = 0; i < pixels.size(); ++i)
        pixels[i] = static_cast<std::uint8_t>((i * 37 + static_cast<std::size_t>(seed) * 13) & 255);
    return pixels;
}

bool CheckDownload(SDL_GPUDevice* device, SDL_GPUTexture* texture, SDL_GPUTransferBuffer* buffer, Dimensions size,
                   const std::vector<std::uint8_t>& expected)
{
    const auto pitch = (static_cast<std::size_t>(size.width) * 4 + 255) / 256 * 256;
    auto* command = SDL_AcquireGPUCommandBuffer(device);
    if (!command)
        return false;
    auto* pass = SDL_BeginGPUCopyPass(command);
    if (!pass)
    {
        SDL_CancelGPUCommandBuffer(command);
        return false;
    }
    SDL_GPUTextureRegion region{};
    region.texture = texture;
    region.w = static_cast<Uint32>(size.width);
    region.h = static_cast<Uint32>(size.height);
    region.d = 1;
    SDL_GPUTextureTransferInfo destination{};
    destination.transfer_buffer = buffer;
    destination.pixels_per_row = static_cast<Uint32>(pitch / 4);
    destination.rows_per_layer = static_cast<Uint32>(size.height);
    SDL_DownloadFromGPUTexture(pass, &region, &destination);
    SDL_EndGPUCopyPass(pass);
    auto* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(command);
    if (!fence)
        return false;
    SDL_GPUFence* fences[] = {fence};
    bool valid = SDL_WaitForGPUFences(device, true, fences, 1);
    if (valid)
    {
        const auto* mapped = static_cast<const std::uint8_t*>(SDL_MapGPUTransferBuffer(device, buffer, false));
        valid = mapped != nullptr;
        for (int row = 0; valid && row < size.height; ++row)
            valid =
                std::memcmp(mapped + static_cast<std::size_t>(row) * pitch,
                            expected.data() + static_cast<std::size_t>(row) * static_cast<std::size_t>(size.width) * 4,
                            static_cast<std::size_t>(size.width) * 4) == 0;
        if (mapped)
            SDL_UnmapGPUTransferBuffer(device, buffer);
    }
    SDL_ReleaseGPUFence(device, fence);
    return valid;
}

bool RunLifetime(SDL_GPUDevice* device, int lifetime)
{
    Destinations destinations;
    if (!destinations.Initialize(device))
    {
        std::fprintf(stderr, "destination allocation: %s\n", SDL_GetError());
        destinations.Release(device);
        return false;
    }
    SDL_GPUTransferBufferCreateInfo readbackInfo{};
    readbackInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD;
    readbackInfo.size = static_cast<Uint32>(Owner::kSlotCapacity);
    auto* readback = SDL_CreateGPUTransferBuffer(device, &readbackInfo);
    if (!readback)
    {
        destinations.Release(device);
        return false;
    }
    Owner::Calls calls{};
    calls.createTexture = Destinations::Create;
    calls.releaseTexture = Destinations::BorrowedRelease;
    calls.createTransfer = CountedCreate;
    calls.createSampler = CountedSampler;
    calls.map = MapWithPaddingPoison;
    Owner owner(calls);
    const int createsAtStart = transferCreates;
    const int samplersAtStart = samplerCreates;
    if (!owner.Initialize(device))
    {
        std::fprintf(stderr, "owner init: %s: %s\n", owner.LastStage().c_str(), owner.LastError().c_str());
        SDL_ReleaseGPUTransferBuffer(device, readback);
        destinations.Release(device);
        return false;
    }
    // Reuse three real textures; serial GPU submissions order writes to each.
    // The download fence waits before checking bytes or ending this interval.
    int peakTotal = 0, peakNvidia = 0;
    int uploads = 0;
    int checkpoints = 0;
    bool good = true;
    for (int interval = 0; interval < 2 && good; ++interval)
    {
        std::array<std::vector<std::uint8_t>, kSizes.size()> latestPixels;
        for (int i = 0; i < kIterations; ++i)
        {
            const bool maximum = i == 2 || ((i % 300 == 299) && (interval != 0 || i != kIterations - 1));
            const int width = maximum ? kMaxDimension : (i % 2 ? kSmallWidth : kUploadWidth);
            const int height = maximum ? kMaxDimension : (i % 2 ? kSmallHeight : kUploadHeight);
            auto pixels = Pattern({width, height}, lifetime + interval * kIterations + i);
            mappedWidth = width;
            auto result =
                owner.Upload(pixels, width, height, SDL_GPU_FILTER_LINEAR, SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE);
            if (!result)
            {
                std::fprintf(stderr, "upload %d: %s: %s\n", uploads, result.stage.c_str(), result.error.c_str());
                good = false;
                break;
            }
            const int sizeIndex = SizeIndex({width, height});
            if (sizeIndex < 0 || result.texture != destinations.handles[static_cast<std::size_t>(sizeIndex)])
            {
                good = false;
                break;
            }
            latestPixels[static_cast<std::size_t>(sizeIndex)] = std::move(pixels);
            ++uploads;
            // Check before later destination writes can hide premature staging reuse.
            // Warm-up/first ring wrap and burst boundaries are checked, while
            // hundreds of uploads between checkpoints remain unsynchronized.
            const bool checkpoint = (i >= 2 && i <= 5) || (i >= 299 && (i - 299) % 300 <= 2) || i == kIterations - 1;
            if (checkpoint)
            {
                ++checkpoints;
                for (std::size_t size = 0; size < kSizes.size(); ++size)
                {
                    if (latestPixels[size].empty())
                    {
                        good = false;
                        break;
                    }
                    if (!CheckDownload(device, destinations.handles[size], readback, kSizes[size], latestPixels[size]))
                    {
                        std::fprintf(stderr, "readback: lifetime=%d interval=%d upload=%d size=%dx%d error=%s\n",
                                     lifetime, interval, i, kSizes[size].width, kSizes[size].height, SDL_GetError());
                        good = false;
                        break;
                    }
                }
                if (!good)
                    break;
            }
            if (i % 100 == 0)
            {
                peakTotal = std::max(peakTotal, CountDescriptors(false));
                peakNvidia = std::max(peakNvidia, CountDescriptors(true));
            }
        }
        const int creations = transferCreates - createsAtStart;
        const int samplers = samplerCreates - samplersAtStart;
        good = good && creations == static_cast<int>(Owner::kSlotCount) && samplers == 4 &&
               destinations.allocations == 3 && destinations.requests == uploads;
        std::printf("lifetime=%d interval=%d uploads=%d checkpoints=%d staging_creates=%d staging_high_water=%d "
                    "sampler_creates=%d destination_allocations=%d destination_requests=%d fd_peak=%d "
                    "nvidia_fd_peak=%d readback=%s\n",
                    lifetime, interval, uploads, checkpoints, creations, creations, samplers, destinations.allocations,
                    destinations.requests, peakTotal, peakNvidia, good ? "pass" : "fail");
    }
    if (!owner.Shutdown())
        good = false;
    // Download completion and owner shutdown retire all submitted references.
    if (!SDL_WaitForGPUIdle(device))
        good = false;
    SDL_ReleaseGPUTransferBuffer(device, readback);
    destinations.Release(device);

    // Separate short production-default ownership smoke: the actual SDL
    // create/release path allocates its own texture, then reads back and unloads it.
    Owner defaultOwner;
    if (!defaultOwner.Initialize(device))
        return false;
    const auto pixels = Pattern({kSmallWidth, kSmallHeight}, lifetime);
    const auto result = defaultOwner.Upload(pixels, kSmallWidth, kSmallHeight, SDL_GPU_FILTER_NEAREST,
                                            SDL_GPU_SAMPLERADDRESSMODE_REPEAT);
    SDL_GPUTransferBufferCreateInfo smokeInfo{};
    smokeInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD;
    smokeInfo.size = static_cast<Uint32>(Owner::kSlotCapacity);
    auto* smokeReadback = SDL_CreateGPUTransferBuffer(device, &smokeInfo);
    const bool smoke = result && smokeReadback &&
                       CheckDownload(device, result.texture, smokeReadback, {kSmallWidth, kSmallHeight}, pixels);
    if (result)
        SDL_ReleaseGPUTexture(device, result.texture);
    if (smokeReadback)
        SDL_ReleaseGPUTransferBuffer(device, smokeReadback);
    if (!defaultOwner.Shutdown())
        good = false;
    std::printf("lifetime=%d default_texture_smoke=%s\n", lifetime, smoke ? "pass" : "fail");
    return good && smoke;
}
} // namespace

int main()
{
    if (!std::getenv("MU_RUN_GPU_UPLOAD_TESTS") || std::strcmp(std::getenv("MU_RUN_GPU_UPLOAD_TESTS"), "1") != 0)
    {
        std::puts("GPU upload fixture skipped: set MU_RUN_GPU_UPLOAD_TESTS=1");
        return 77;
    }
    const bool validation =
        std::getenv("MU_GPU_UPLOAD_VALIDATION") && std::strcmp(std::getenv("MU_GPU_UPLOAD_VALIDATION"), "1") == 0;
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::fprintf(stderr, "SDL init: %s\n", SDL_GetError());
        return 1;
    }
    ValidationLog validationLog;
    if (validation)
    {
        SDL_GetLogOutputFunction(&validationLog.previous, &validationLog.previousData);
        SDL_SetLogPriority(SDL_LOG_CATEGORY_GPU, SDL_LOG_PRIORITY_INFO);
        SDL_SetLogOutputFunction(ValidationLog::Capture, &validationLog);
    }
    auto* window = SDL_CreateWindow("bitmap upload regression", 64, 64, SDL_WINDOW_HIDDEN);
    if (!window)
    {
        std::fprintf(stderr, "window: %s\n", SDL_GetError());
        if (validation)
            SDL_SetLogOutputFunction(validationLog.previous, validationLog.previousData);
        SDL_Quit();
        return 1;
    }
    bool good = true;
    for (int lifetime = 0; lifetime < 2; ++lifetime)
    {
        auto* device = SDL_CreateGPUDevice(
            SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL, validation, nullptr);
        if (!device)
        {
            std::fprintf(stderr, "device: %s\n", SDL_GetError());
            good = false;
            break;
        }
        const char* driver = SDL_GetGPUDeviceDriver(device);
        if (validation && driver && std::strcmp(driver, "vulkan") == 0 && !validationLog.layerEnabled)
        {
            std::fprintf(stderr, "Vulkan validation requested but layer not enabled\n");
            SDL_DestroyGPUDevice(device);
            good = false;
            break;
        }
        if (!SDL_ClaimWindowForGPUDevice(device, window))
        {
            std::fprintf(stderr, "claim: %s\n", SDL_GetError());
            SDL_DestroyGPUDevice(device);
            good = false;
            break;
        }
        std::printf("driver=%s lifetime=%d validation_requested=%d vulkan_layer_enabled=%d\n", driver, lifetime,
                    validation, validationLog.layerEnabled);
        good = RunLifetime(device, lifetime);
        SDL_ReleaseWindowFromGPUDevice(device, window);
        SDL_DestroyGPUDevice(device);
        if (!good)
            break;
    }
    SDL_DestroyWindow(window);
    if (validation && validationLog.validationErrors)
    {
        std::fprintf(stderr, "GPU validation errors: %d\n", validationLog.validationErrors);
        good = false;
    }
    if (validation)
        SDL_SetLogOutputFunction(validationLog.previous, validationLog.previousData);
    SDL_Quit();
    return good ? 0 : 1;
}
