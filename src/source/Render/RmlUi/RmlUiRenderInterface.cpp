#include "stdafx.h"
#include "RmlUiRenderInterface.h"

#include "Render/Renderer/MuRenderer.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "Core/Globals/_TextureIndex.h"
#include "Core/Utilities/StringUtils.h"
#include <algorithm>

RmlUiRenderInterface::RmlUiRenderInterface(SDL_GPUDevice* device, SDL_Window* window)
    : RenderInterface_SDL_GPU(device, window)
{
}

Rml::TextureHandle RmlUiRenderInterface::LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source)
{
    // Routed through CGlobalBitmap (the same ref-counted, OZJ/OZT-aware pipeline every other
    // texture in the game uses -- GlobalBitmap.cpp) rather than the base class's own LoadTexture
    // (which reads the file directly via Rml::FileInterface + SDL_image's generic format
    // sniffing). This isn't just for cache-sharing parity with feature/rmlui-migration's
    // RHI-based version -- it's required: this game's actual art assets are almost entirely
    // OZT/OZJ, a custom container format SDL_image has no reader for, so the base class's own
    // LoadTexture cannot load any of this game's existing images at all.
    //
    // RML/RCSS conventionally author asset paths with forward slashes; every existing in-tree
    // caller of Bitmaps.LoadImage passes backslash-separated paths (e.g. "Interface\\Foo.jpg"),
    // matching this codebase's own path-handling convention -- normalize rather than assume
    // forward-slash tolerance extends to this legacy loader too.
    std::string normalized = source;
    std::replace(normalized.begin(), normalized.end(), '/', '\\');
    const std::wstring wpath = StringUtils::NarrowToWide(normalized); // Rml::String is std::string (RmlUi/Config/Config.h)
    const GLuint bitmapIndex = Bitmaps.LoadImage(wpath, GL_LINEAR, GL_CLAMP_TO_EDGE);
    if (bitmapIndex == BITMAP_UNKNOWN) return 0;

    BITMAP_t* bmp = Bitmaps.GetTexture(bitmapIndex);
    if (!bmp) return 0;

    // BITMAP_t::TextureNumber is GLuint-typed for legacy naming continuity, but on this backend
    // it holds CGlobalBitmap's own logical texture id (set from CreateTexture()'s return value,
    // GlobalBitmap.cpp) -- not a real GL name. Resolve it to the raw SDL_GPUTexture* the base
    // class's RenderGeometry/command-replay code expects as a TextureHandle.
    void* rawTexture = mu::GetRenderer().GetRawTexture(static_cast<std::uint32_t>(bmp->TextureNumber));
    if (!rawTexture) return 0;

    texture_dimensions = { static_cast<int>(bmp->Width), static_cast<int>(bmp->Height) };
    const auto handle = reinterpret_cast<Rml::TextureHandle>(rawTexture);
    m_FileBackedBitmapIndexByHandle.emplace(handle, static_cast<std::uint32_t>(bitmapIndex));
    return handle;
}

void RmlUiRenderInterface::ReleaseTexture(Rml::TextureHandle texture_handle)
{
    auto it = m_FileBackedBitmapIndexByHandle.find(texture_handle);
    if (it != m_FileBackedBitmapIndexByHandle.end())
    {
        // Ref-counted; frees the shared bitmap (and its GPU texture, via CGlobalBitmap's own
        // teardown path) only once every other reference has also released it. Must NOT also
        // queue the base class's SDL_ReleaseGPUTexture command below -- that texture is owned
        // by this engine's shared cache, not by this RenderInterface.
        Bitmaps.UnloadImage(static_cast<GLuint>(it->second), /*bForce=*/false);
        m_FileBackedBitmapIndexByHandle.erase(it);
        return;
    }

    // Not a FileBacked handle -- one of the base class's own textures (GenerateTexture's font/
    // glyph atlases). Let it manage that lifetime itself, same as before this override existed.
    RenderInterface_SDL_GPU::ReleaseTexture(texture_handle);
}
