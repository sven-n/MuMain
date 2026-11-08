#pragma once

// CPU texture decoding with the real cache; no GPU device, no textures left between cases.
class PlayerTextureScope
{
public:
    PlayerTextureScope();
    ~PlayerTextureScope();
    PlayerTextureScope(const PlayerTextureScope&) = delete;
    PlayerTextureScope& operator=(const PlayerTextureScope&) = delete;
};
