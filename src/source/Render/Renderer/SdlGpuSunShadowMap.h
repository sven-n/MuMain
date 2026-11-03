// SdlGpuSunShadowMap.h: the Enhanced sun shadow map's textures and sampler. The renderer replays the
// scene into Map(); the lit shaders read Bound().
#pragma once

#include <SDL3/SDL_gpu.h>

namespace mu
{

class SdlGpuSunShadowMap
{
public:
    // The map's size for a requested sun.resolution: 1024, 2048 or 4096 texels a side.
    [[nodiscard]] static Uint32 ResolutionFor(Uint32 requested);

    // The comparison sampler and the far 1x1 map bound while there is no map. False when either failed.
    bool CreateResources(SDL_GPUDevice* device);
    [[nodiscard]] bool IsAvailable() const;
    // Everything, at shutdown.
    void Release();

    // This frame asks for a map of ResolutionFor(`requested`) (SetSceneLighting), or for none.
    void Request(Uint32 requested);
    void Cancel();
    [[nodiscard]] Uint32 Resolution() const { return m_resolution; }

    // The far 1x1 map while it still needs its one clear, then null.
    [[nodiscard]] SDL_GPUTexture* FallbackToClear() const;
    void MarkFallbackCleared() { m_fallbackCleared = true; }

    // Starts the frame: the map to draw into at the requested size, or null without a request (the map
    // is then released).
    [[nodiscard]] SDL_GPUTexture* PrepareMap();
    // The map holds this frame's shadows.
    void MarkReady() { m_ready = true; }

    // What the lit shaders sample: this frame's map once drawn, otherwise the far 1x1 one.
    [[nodiscard]] SDL_GPUTexture* Bound() const { return m_ready ? m_map : m_fallback; }
    [[nodiscard]] SDL_GPUSampler* Sampler() const { return m_sampler; }

private:
    [[nodiscard]] SDL_GPUTexture* CreateDepth(Uint32 size) const;
    void ReleaseMap();

    SDL_GPUDevice* m_device = nullptr;
    SDL_GPUSampler* m_sampler = nullptr;
    SDL_GPUTexture* m_fallback = nullptr;
    bool m_fallbackCleared = false;
    SDL_GPUTexture* m_map = nullptr;
    Uint32 m_mapSize = 0u;
    bool m_requested = false;
    Uint32 m_resolution = 0u;
    bool m_ready = false;
};

} // namespace mu
