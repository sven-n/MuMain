#pragma once

#include "stdafx.h"
#include <RmlUi_Renderer_SDL_GPU.h>
#include <unordered_map>

// Rml::RenderInterface implementation for the SDL_GPU renderer. Subclasses RmlUi's own vendored
// RenderInterface_SDL_GPU (ThirdParty/RmlUi/Backends/RmlUi_Renderer_SDL_GPU.cpp) and only
// overrides the two methods that need to integrate with this engine's texture cache:
// LoadTexture/ReleaseTexture. Everything else (CompileGeometry/RenderGeometry/ReleaseGeometry/
// GenerateTexture/scissor/SetTransform, including CSS `transform` support) delegates to the base
// class's own tested pipeline/buffer-pooling code untouched.
class RmlUiRenderInterface : public RenderInterface_SDL_GPU
{
public:
    RmlUiRenderInterface(SDL_GPUDevice* device, SDL_Window* window);

    Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) override;
    void ReleaseTexture(Rml::TextureHandle texture_handle) override;

private:
    // RmlUi's own TextureHandle IS the raw SDL_GPUTexture* (see RenderInterface_SDL_GPU.cpp) --
    // ReleaseTexture only gets that value back, not which path created it, so this engine needs
    // its own kind-tracking. FileBacked entries route release through Bitmaps.UnloadImage()
    // (ref-counted -- the underlying SDL_GPUTexture* is owned by this engine's shared texture
    // cache, not by this class, so it must never be handed to the base class's own
    // SDL_ReleaseGPUTexture command). Anything not in this map is assumed base-class-owned
    // (GenerateTexture's font/glyph atlases) and released via
    // RenderInterface_SDL_GPU::ReleaseTexture as before.
    std::unordered_map<Rml::TextureHandle, std::uint32_t> m_FileBackedBitmapIndexByHandle;
};
