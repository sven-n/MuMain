#pragma once

#include <SDL3/SDL_properties.h>

#include <string_view>

namespace Render
{

// Fills the properties SDL_CreateGPUDeviceWithProperties gets: every shipped shader format, the debug mode, the
// target's own properties, and then config.ini's [Render] Backend as the driver name unless it is empty or "default".
void FillGpuDeviceProperties(SDL_PropertiesID props, std::string_view configBackend, bool debugMode);

// The target's own device properties, from the adapter its CMake picks (MU_TARGET_ADAPTERS).
void AddTargetGpuDeviceProperties(SDL_PropertiesID props);

} // namespace Render
