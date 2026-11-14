#include "Render/Renderer/GpuDevice.h"

#include <SDL3/SDL_gpu.h>

namespace Render
{

// SDL's own pick on Windows is Direct3D 12, whose vsync does not cap the frame rate correctly.
void AddTargetGpuDeviceProperties(SDL_PropertiesID props)
{
    SDL_SetStringProperty(props, SDL_PROP_GPU_DEVICE_CREATE_NAME_STRING, "vulkan");
}

} // namespace Render
