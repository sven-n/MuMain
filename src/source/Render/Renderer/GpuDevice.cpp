#include "Render/Renderer/GpuDevice.h"

#include "Render/Renderer/GpuShaderFormats.h"

#include <SDL3/SDL_gpu.h>

#include <string>

namespace Render
{

void FillGpuDeviceProperties(SDL_PropertiesID props, std::string_view configBackend, bool debugMode)
{
    for (const ShaderFormat& entry : kShaderFormats)
    {
        SDL_SetBooleanProperty(props, entry.createProperty, true);
    }
    SDL_SetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_DEBUGMODE_BOOLEAN, debugMode);
    AddTargetGpuDeviceProperties(props);
    if (!configBackend.empty() && configBackend != "default")
    {
        SDL_SetStringProperty(props, SDL_PROP_GPU_DEVICE_CREATE_NAME_STRING, std::string(configBackend).c_str());
    }
}

} // namespace Render
