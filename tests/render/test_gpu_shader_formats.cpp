#include <doctest.h>

#include "Render/Renderer/GpuDevice.h"
#include "Render/Renderer/GpuShaderFormats.h"

#include <SDL3/SDL_gpu.h>

#include <string_view>

namespace
{
constexpr const char* kTargetProperty = "test.target.property";
}

// This test's target: it names a driver and adds a property of its own, as a port's adapter would.
void Render::AddTargetGpuDeviceProperties(SDL_PropertiesID props)
{
    SDL_SetStringProperty(props, SDL_PROP_GPU_DEVICE_CREATE_NAME_STRING, "target-driver");
    SDL_SetBooleanProperty(props, kTargetProperty, true);
}

TEST_CASE("A device gets the first shipped shader format it accepts [render][shader_formats]")
{
    const Render::ShaderFormat* vulkan = Render::PickShaderFormat(SDL_GPU_SHADERFORMAT_SPIRV);
    REQUIRE(vulkan != nullptr);
    CHECK(std::string_view(vulkan->extension) == "spv");
    CHECK(std::string_view(vulkan->entrypoint) == "main");

    const Render::ShaderFormat* d3d12 = Render::PickShaderFormat(SDL_GPU_SHADERFORMAT_DXBC | SDL_GPU_SHADERFORMAT_DXIL);
    REQUIRE(d3d12 != nullptr);
    CHECK(d3d12->format == SDL_GPU_SHADERFORMAT_DXIL);
    CHECK(std::string_view(d3d12->extension) == "dxil");

    const Render::ShaderFormat* metal =
        Render::PickShaderFormat(SDL_GPU_SHADERFORMAT_MSL | SDL_GPU_SHADERFORMAT_METALLIB);
    REQUIRE(metal != nullptr);
    CHECK(std::string_view(metal->extension) == "msl");
    CHECK(std::string_view(metal->entrypoint) == "main0");
}

TEST_CASE("A device that accepts no shipped format gets none [render][shader_formats]")
{
    CHECK(Render::PickShaderFormat(SDL_GPU_SHADERFORMAT_DXBC) == nullptr);
    CHECK(Render::PickShaderFormat(SDL_GPU_SHADERFORMAT_METALLIB) == nullptr);
    CHECK(Render::PickShaderFormat(SDL_GPU_SHADERFORMAT_INVALID) == nullptr);
    CHECK(Render::ShaderFormatNames() == "spv, dxil, msl");
}

TEST_CASE("Each shipped format appears once [render][shader_formats]")
{
    SDL_GPUShaderFormat seen = SDL_GPU_SHADERFORMAT_INVALID;
    for (const Render::ShaderFormat& entry : Render::kShaderFormats)
    {
        CHECK((seen & entry.format) == 0);
        seen |= entry.format;
    }
}

TEST_CASE("Device properties ask for every shipped format and keep the target's own [render][shader_formats]")
{
    const SDL_PropertiesID props = SDL_CreateProperties();
    REQUIRE(props != 0);
    Render::FillGpuDeviceProperties(props, "default", true);

    for (const Render::ShaderFormat& entry : Render::kShaderFormats)
    {
        CHECK(SDL_GetBooleanProperty(props, entry.createProperty, false));
    }
    CHECK(SDL_GetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_DEBUGMODE_BOOLEAN, false));
    CHECK(SDL_GetBooleanProperty(props, kTargetProperty, false));
    CHECK(std::string_view(SDL_GetStringProperty(props, SDL_PROP_GPU_DEVICE_CREATE_NAME_STRING, "")) ==
          "target-driver");
    SDL_DestroyProperties(props);
}

TEST_CASE("config.ini's backend overrides the target's driver [render][shader_formats]")
{
    const SDL_PropertiesID props = SDL_CreateProperties();
    REQUIRE(props != 0);
    Render::FillGpuDeviceProperties(props, "direct3d12", false);
    CHECK(std::string_view(SDL_GetStringProperty(props, SDL_PROP_GPU_DEVICE_CREATE_NAME_STRING, "")) == "direct3d12");
    CHECK_FALSE(SDL_GetBooleanProperty(props, SDL_PROP_GPU_DEVICE_CREATE_DEBUGMODE_BOOLEAN, true));
    SDL_DestroyProperties(props);

    const SDL_PropertiesID empty = SDL_CreateProperties();
    REQUIRE(empty != 0);
    Render::FillGpuDeviceProperties(empty, "", false);
    CHECK(std::string_view(SDL_GetStringProperty(empty, SDL_PROP_GPU_DEVICE_CREATE_NAME_STRING, "")) ==
          "target-driver");
    SDL_DestroyProperties(empty);
}
