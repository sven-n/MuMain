#include "Render/Renderer/GpuDevice.h"

namespace Render
{

// SDL picks the driver; the target adds no properties of its own.
void AddTargetGpuDeviceProperties(SDL_PropertiesID) {}

bool TargetFramesMayWait()
{
    return true;
}

} // namespace Render
