#include "stdafx.h"
#include "PlayerTextureScope.h"

#include "Render/Sprites/GlobalBitmap.h"
#include "Render/Sprites/GpuUploadSkip.h"

PlayerTextureScope::PlayerTextureScope()
{
    Bitmaps.UnloadAllImages();
    Bitmaps.SetTexturePack(L"");
    Render::Sprites::SetGpuUploadsSkipped(true);
}

PlayerTextureScope::~PlayerTextureScope()
{
    Bitmaps.UnloadAllImages();
    Bitmaps.SetTexturePack(L"");
    Render::Sprites::SetGpuUploadsSkipped(false);
}
