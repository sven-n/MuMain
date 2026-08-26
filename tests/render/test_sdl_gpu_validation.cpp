#include <doctest.h>

#include "Render/Renderer/SdlGpuValidation.h"

TEST_CASE("GPU validation follows assertion policy [render][sdl_gpu_validation]")
{
    CHECK(Render::GpuValidationEnabled(false));
    CHECK_FALSE(Render::GpuValidationEnabled(true));
#ifdef NDEBUG
    CHECK_FALSE(Render::kGpuValidationEnabled);
#else
    CHECK(Render::kGpuValidationEnabled);
#endif
}
