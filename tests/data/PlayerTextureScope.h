#pragma once

#include "Render/Sprites/GlobalBitmap.h"
#include "Render/Sprites/GpuUploadSkip.h"

// CPU texture decoding with the real cache; no GPU device, no textures left between cases.
class PlayerTextureScope
{
public:
    PlayerTextureScope()
    {
        Bitmaps.UnloadAllImages();
        Bitmaps.SetTexturePack(L"");
        Render::Sprites::SetGpuUploadsSkipped(true);
    }
    ~PlayerTextureScope()
    {
        Bitmaps.UnloadAllImages();
        Bitmaps.SetTexturePack(L"");
        Render::Sprites::SetGpuUploadsSkipped(false);
    }
    PlayerTextureScope(const PlayerTextureScope&) = delete;
    PlayerTextureScope& operator=(const PlayerTextureScope&) = delete;
};
