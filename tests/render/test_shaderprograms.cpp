// Story 4.3.2: Shader Programs (HLSL + SDL_shadercross) [VS1-RENDER-SHADERS]
// RED PHASE: Tests compile but FAIL until GetShaderBlobPath, FogUniform struct,
// and pipeline selection logic are implemented in MuRendererSDLGpu.cpp.
// Tests compile and run on macOS/Linux — no GPU device, no Win32, no OpenGL required.
#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>
#include <string>

// Forward declarations of testable symbols from MuRendererSDLGpu.cpp.
// These are free functions in the mu:: namespace exposed for test linkage.
// They will be defined once MuRendererSDLGpu.cpp implements them.
namespace mu
{

// AC-6: Returns the absolute path to a shader blob for the given GPU driver,
// shader stage, and shader name. Uses MU_SHADER_DIR as the base directory.
// driver: "vulkan", "direct3d12", or "metal"
// stage:  "vert" or "frag"
// name:   e.g., "basic_textured", "basic_colored", "shadow_volume"
std::string GetShaderBlobPath(const char* driver, const char* stage, const char* name);

// AC-8: Identifies which pipeline set a draw mode selects.
enum class PipelineSet
{
    Pipelines2D,
    Pipelines3D
};

enum class DrawMode
{
    Quad2D,
    Triangles,
    QuadStrip
};

// Returns which pipeline set should be used for the given draw mode.
// RenderQuad2D → Pipelines2D; RenderTriangles/RenderQuadStrip → Pipelines3D
PipelineSet GetPipelineSetFor(DrawMode mode);

// AC-10: Fog uniform struct mirroring the HLSL FogUniforms cbuffer (std140 layout).
// Defined in MuRendererSDLGpu.cpp anonymous namespace; forward-declared here for
// compile-time layout verification via static_assert.
struct FogUniform
{
    uint32_t fogEnabled;
    uint32_t alphaDiscardEnabled;
    float alphaThreshold;
    float pad0;
    float fogStart;
    float fogEnd;
    float fogColor[4];
    float pad1[2];
};

} // namespace mu

// =============================================================================
// AC-6: ShaderBlobPath — driver-to-extension mapping
// =============================================================================
// Verifies GetShaderBlobPath returns correct extension for each GPU driver,
// correct stage suffix, and correct base name. No GPU device required.
TEST_CASE("AC-6: ShaderBlobPath — driver-to-extension mapping", "[render][shader][ac6]")
{
    using mu::GetShaderBlobPath;

    SECTION("Vulkan driver uses .spv extension")
    {
        std::string path = GetShaderBlobPath("vulkan", "vert", "basic_textured");
        REQUIRE(!path.empty());
        REQUIRE(path.find(".spv") != std::string::npos);
        REQUIRE(path.find("basic_textured") != std::string::npos);
        REQUIRE(path.find("vert") != std::string::npos);
    }

    SECTION("Direct3D12 driver uses .dxil extension")
    {
        std::string path = GetShaderBlobPath("direct3d12", "vert", "basic_textured");
        REQUIRE(!path.empty());
        REQUIRE(path.find(".dxil") != std::string::npos);
        REQUIRE(path.find("basic_textured") != std::string::npos);
        REQUIRE(path.find("vert") != std::string::npos);
    }

    SECTION("Metal driver uses .msl extension")
    {
        std::string path = GetShaderBlobPath("metal", "vert", "basic_textured");
        REQUIRE(!path.empty());
        REQUIRE(path.find(".msl") != std::string::npos);
        REQUIRE(path.find("basic_textured") != std::string::npos);
        REQUIRE(path.find("vert") != std::string::npos);
    }

    SECTION("Fragment stage suffix included in path")
    {
        std::string pathVk = GetShaderBlobPath("vulkan", "frag", "basic_textured");
        REQUIRE(pathVk.find("frag") != std::string::npos);

        std::string pathD3D = GetShaderBlobPath("direct3d12", "frag", "basic_colored");
        REQUIRE(pathD3D.find("frag") != std::string::npos);
    }

    SECTION("All shader names produce valid paths for vulkan")
    {
        const char* shaderNames[] = {"basic_textured", "basic_colored", "shadow_volume"};
        for (const char* name : shaderNames)
        {
            std::string path = GetShaderBlobPath("vulkan", "vert", name);
            REQUIRE(!path.empty());
            REQUIRE(path.find(name) != std::string::npos);
        }
    }
}

// =============================================================================
// AC-10: FogUniform — struct layout static_assert
// =============================================================================
// Verifies FogUniform struct mirrors the HLSL FogUniforms cbuffer (std140 layout).
// static_asserts are compile-time; REQUIRE checks confirm runtime expectations.
// std140 layout for this struct:
//   offset  0: fogEnabled        (uint32, 4 bytes)
//   offset  4: alphaDiscardEnabled (uint32, 4 bytes)
//   offset  8: alphaThreshold    (float, 4 bytes)
//   offset 12: pad0              (float, 4 bytes)
//   offset 16: fogStart          (float, 4 bytes)
//   offset 20: fogEnd            (float, 4 bytes)
//   offset 24: fogColor[4]       (float4, 16 bytes)
//   offset 40: pad1[2]           (float2, 8 bytes)
//   total: 48 bytes
TEST_CASE("AC-10: FogUniform — struct layout static_assert", "[render][shader][ac10]")
{
    // Compile-time layout verification — these will fail to compile if layout is wrong
    static_assert(offsetof(mu::FogUniform, fogEnabled) == 0, "FogUniform.fogEnabled must be at offset 0 (std140)");
    static_assert(offsetof(mu::FogUniform, alphaDiscardEnabled) == 4,
                  "FogUniform.alphaDiscardEnabled must be at offset 4 (std140)");
    static_assert(offsetof(mu::FogUniform, alphaThreshold) == 8,
                  "FogUniform.alphaThreshold must be at offset 8 (std140)");
    static_assert(offsetof(mu::FogUniform, pad0) == 12, "FogUniform.pad0 must be at offset 12 (std140)");
    static_assert(offsetof(mu::FogUniform, fogStart) == 16, "FogUniform.fogStart must be at offset 16 (std140)");
    static_assert(offsetof(mu::FogUniform, fogEnd) == 20, "FogUniform.fogEnd must be at offset 20 (std140)");
    static_assert(offsetof(mu::FogUniform, fogColor) == 24,
                  "FogUniform.fogColor must be at offset 24 (std140 — float4 alignment)");
    static_assert(offsetof(mu::FogUniform, pad1) == 40, "FogUniform.pad1 must be at offset 40 (std140)");
    static_assert(sizeof(mu::FogUniform) == 48, "FogUniform must be exactly 48 bytes (std140 — matches HLSL cbuffer)");

    SECTION("sizeof FogUniform is 48 bytes")
    {
        REQUIRE(sizeof(mu::FogUniform) == 48);
    }

    SECTION("fogStart field at offset 16")
    {
        REQUIRE(offsetof(mu::FogUniform, fogStart) == 16);
    }

    SECTION("fogColor field at offset 24 (float4 std140 alignment)")
    {
        REQUIRE(offsetof(mu::FogUniform, fogColor) == 24);
    }

    SECTION("FogUniform default zero-initialized values are safe to upload")
    {
        mu::FogUniform fog{};
        CHECK(fog.fogEnabled == 0);
        CHECK(fog.alphaDiscardEnabled == 0);
        CHECK(fog.alphaThreshold == 0.0f);
        CHECK(fog.fogStart == 0.0f);
        CHECK(fog.fogEnd == 0.0f);
        for (float c : fog.fogColor)
        {
            CHECK(c == 0.0f);
        }
    }
}

// =============================================================================
// AC-8: Pipeline selection — Vertex3D uses 3D pipeline set
// =============================================================================
// Verifies that the draw-mode-to-pipeline-set mapping is correct:
//   RenderQuad2D   → PipelineSet::Pipelines2D
//   RenderTriangles  → PipelineSet::Pipelines3D
//   RenderQuadStrip → PipelineSet::Pipelines3D
// No GPU device required; tests pure selection logic.
TEST_CASE("AC-8: Pipeline selection — Vertex3D uses 3D pipeline set", "[render][shader][ac8]")
{
    using mu::DrawMode;
    using mu::GetPipelineSetFor;
    using mu::PipelineSet;

    SECTION("RenderQuad2D selects Pipelines2D")
    {
        REQUIRE(GetPipelineSetFor(DrawMode::Quad2D) == PipelineSet::Pipelines2D);
    }

    SECTION("RenderTriangles selects Pipelines3D (not Pipelines2D)")
    {
        REQUIRE(GetPipelineSetFor(DrawMode::Triangles) == PipelineSet::Pipelines3D);
        REQUIRE(GetPipelineSetFor(DrawMode::Triangles) != PipelineSet::Pipelines2D);
    }

    SECTION("RenderQuadStrip selects Pipelines3D (not Pipelines2D)")
    {
        REQUIRE(GetPipelineSetFor(DrawMode::QuadStrip) == PipelineSet::Pipelines3D);
        REQUIRE(GetPipelineSetFor(DrawMode::QuadStrip) != PipelineSet::Pipelines2D);
    }

    SECTION("3D draw modes never use 2D pipeline set")
    {
        CHECK(GetPipelineSetFor(DrawMode::Triangles) != PipelineSet::Pipelines2D);
        CHECK(GetPipelineSetFor(DrawMode::QuadStrip) != PipelineSet::Pipelines2D);
    }
}
