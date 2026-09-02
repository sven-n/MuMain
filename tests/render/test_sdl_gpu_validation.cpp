#include <doctest.h>

#include "Render/Renderer/D3D12Diagnostics.h"
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

TEST_CASE("D3D12 diagnostics require the D3D12 driver and an enabled value [render][d3d12]")
{
    CHECK(Render::D3D12DiagnosticEnabled("direct3d12", "1"));
    CHECK_FALSE(Render::D3D12DiagnosticEnabled("direct3d12", "0"));
    CHECK_FALSE(Render::D3D12DiagnosticEnabled("vulkan", "1"));
    CHECK_FALSE(Render::D3D12DiagnosticEnabled(nullptr, "1"));
    CHECK_FALSE(Render::D3D12DiagnosticEnabled("direct3d12", nullptr));
}
