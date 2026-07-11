#include <array>
#include <cstdint>
#include <utility>
#include <vector>

#include <doctest.h>
#include <SDL3/SDL_gpu.h>

#include "Render/Renderer/FramePixelReadback.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Renderer/SdlGpuPixelFormat.h"

namespace
{

class MinimalRenderer final : public mu::IMuRenderer
{
public:
    void RenderQuad2D(std::span<const mu::Vertex2D>, std::uint32_t) override {}
    void RenderTriangles(std::span<const mu::Vertex3D>, std::uint32_t) override {}
    void RenderQuadStrip(std::span<const mu::Vertex3D>, std::uint32_t) override {}
    void SetBlendMode(mu::BlendMode) override {}
    void DisableBlend() override {}
    void SetDepthTest(bool) override {}
    void SetFog(const mu::FogParams&) override {}
    void BeginScene(int, int, int, int) override {}
    void EndScene() override {}
    void Begin2DPass() override {}
    void End2DPass() override {}
    void ClearScreen() override {}
    void RenderLines(std::span<const mu::Vertex3D>, std::uint32_t) override {}
};

} // namespace

TEST_CASE("renderer rejects unsupported frame readback by default [frame readback]")
{
    MinimalRenderer renderer;
    mu::FramePixels pixels;

    CHECK_FALSE(renderer.RequestFramePixels());
    CHECK_FALSE(renderer.ConsumeFramePixels(pixels));
}

TEST_CASE("SDL GPU color formats map to frame pixel channel order [frame readback][pixel readback]")
{
    CHECK(mu::GetSdlGpuPixelChannelOrder(SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM) ==
          mu::PixelChannelOrder::Rgba);
    CHECK(mu::GetSdlGpuPixelChannelOrder(SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM_SRGB) ==
          mu::PixelChannelOrder::Rgba);
    CHECK(mu::GetSdlGpuPixelChannelOrder(SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM) ==
          mu::PixelChannelOrder::Bgra);
    CHECK(mu::GetSdlGpuPixelChannelOrder(SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM_SRGB) ==
          mu::PixelChannelOrder::Bgra);
    CHECK_FALSE(mu::GetSdlGpuPixelChannelOrder(SDL_GPU_TEXTUREFORMAT_D16_UNORM).has_value());
}

TEST_CASE("SDL GPU frame capture target is readable and matches physical dimensions [frame readback][pixel readback]")
{
    constexpr Uint32 width = 2560u;
    constexpr Uint32 height = 1440u;

    const auto info = mu::GetSdlGpuFrameCaptureTextureInfo(
        SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM_SRGB, width, height);

    REQUIRE(info.has_value());
    CHECK(info->type == SDL_GPU_TEXTURETYPE_2D);
    CHECK(info->format == SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM_SRGB);
    CHECK(info->usage == (SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET));
    CHECK(info->width == width);
    CHECK(info->height == height);
    CHECK(info->layer_count_or_depth == 1u);
    CHECK(info->num_levels == 1u);
    CHECK(info->sample_count == SDL_GPU_SAMPLECOUNT_1);
}

TEST_CASE("SDL GPU frame capture target rejects invalid inputs [frame readback][pixel readback]")
{
    CHECK_FALSE(mu::GetSdlGpuFrameCaptureTextureInfo(
                    SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM, 0u, 1080u)
                    .has_value());
    CHECK_FALSE(mu::GetSdlGpuFrameCaptureTextureInfo(
                    SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM, 1920u, 0u)
                    .has_value());
    CHECK_FALSE(mu::GetSdlGpuFrameCaptureTextureInfo(SDL_GPU_TEXTUREFORMAT_D16_UNORM, 1920u, 1080u)
                    .has_value());
}

TEST_CASE("RGBA readback converts to tightly packed top-down RGB [frame readback][pixel readback]")
{
    const std::array<std::uint8_t, 8> source{255, 0, 0, 7, 0, 255, 0, 9};
    mu::FramePixels output;

    CHECK(mu::ConvertToTopDownRgb(source, 2, 1, 8, mu::PixelChannelOrder::Rgba, false, output));
    CHECK(output.width == 2);
    CHECK(output.height == 1);
    CHECK(output.rgb == std::vector<std::uint8_t>{255, 0, 0, 0, 255, 0});
}

TEST_CASE("BGRA readback selects red and blue channels [frame readback][pixel readback]")
{
    const std::array<std::uint8_t, 8> source{4, 3, 2, 1, 8, 7, 6, 5};
    mu::FramePixels output;

    CHECK(mu::ConvertToTopDownRgb(source, 2, 1, 8, mu::PixelChannelOrder::Bgra, false, output));
    CHECK(output.rgb == std::vector<std::uint8_t>{2, 3, 4, 6, 7, 8});
}

TEST_CASE("readback ignores padded bytes at the end of each row [frame readback][pixel readback]")
{
    const std::array<std::uint8_t, 16> source{
        1, 2, 3, 4, 99, 98, 97, 96,
        5, 6, 7, 8, 95, 94, 93, 92,
    };
    mu::FramePixels output;

    CHECK(mu::ConvertToTopDownRgb(source, 1, 2, 8, mu::PixelChannelOrder::Rgba, false, output));
    CHECK(output.rgb == std::vector<std::uint8_t>{1, 2, 3, 5, 6, 7});
}

TEST_CASE("bottom-up readback is reversed to top-down RGB [frame readback][pixel readback]")
{
    const std::array<std::uint8_t, 8> source{
        10, 11, 12, 13,
        20, 21, 22, 23,
    };
    mu::FramePixels output;

    CHECK(mu::ConvertToTopDownRgb(source, 1, 2, 4, mu::PixelChannelOrder::Rgba, true, output));
    CHECK(output.rgb == std::vector<std::uint8_t>{20, 21, 22, 10, 11, 12});
}

TEST_CASE("readback rejects source buffers that cannot cover every row [frame readback][pixel readback]")
{
    const std::array<std::uint8_t, 7> source{255, 0, 0, 7, 0, 255, 0};
    mu::FramePixels output;

    CHECK_FALSE(mu::ConvertToTopDownRgb(source, 2, 1, 8, mu::PixelChannelOrder::Rgba, false, output));
}

TEST_CASE("readback rejects zero dimensions [frame readback][pixel readback]")
{
    const std::array<std::uint8_t, 4> source{255, 0, 0, 7};
    mu::FramePixels output;

    CHECK_FALSE(mu::ConvertToTopDownRgb(source, 0, 1, 0, mu::PixelChannelOrder::Rgba, false, output));
    CHECK_FALSE(mu::ConvertToTopDownRgb(source, 1, 0, 0, mu::PixelChannelOrder::Rgba, false, output));
}

TEST_CASE("readback rejects row pitches smaller than a source row [frame readback][pixel readback]")
{
    const std::array<std::uint8_t, 7> source{255, 0, 0, 7, 0, 255, 0};
    mu::FramePixels output;

    CHECK_FALSE(mu::ConvertToTopDownRgb(source, 2, 1, 7, mu::PixelChannelOrder::Rgba, false, output));
}

TEST_CASE("readback rejects unknown channel orders [frame readback][pixel readback]")
{
    const std::array<std::uint8_t, 4> source{255, 0, 0, 7};
    mu::FramePixels output;
    const auto invalidOrder = static_cast<mu::PixelChannelOrder>(0xFF);

    CHECK_FALSE(mu::ConvertToTopDownRgb(source, 1, 1, 4, invalidOrder, false, output));
}

TEST_CASE("only one frame readback can be outstanding [frame readback]")
{
    mu::FrameReadbackState state;

    CHECK(state.Request());
    CHECK_FALSE(state.Request());
    CHECK(state.IsPending());
}

TEST_CASE("completed readback is retained until consumed [frame readback]")
{
    mu::FrameReadbackState state;
    mu::FramePixels pixels{2, 1, {1, 2, 3, 4, 5, 6}};

    REQUIRE(state.Request());
    state.Complete(std::move(pixels));

    CHECK_FALSE(state.IsPending());
    CHECK_FALSE(state.Request());

    mu::FramePixels consumed = state.Consume();
    CHECK(consumed.width == 2);
    CHECK(consumed.height == 1);
    CHECK(consumed.rgb == std::vector<std::uint8_t>{1, 2, 3, 4, 5, 6});
    CHECK(state.Request());
}

TEST_CASE("failed readback resets the request state [frame readback]")
{
    mu::FrameReadbackState state;

    REQUIRE(state.Request());
    state.Fail();

    CHECK_FALSE(state.IsPending());
    CHECK(state.Request());
    state.Reset();
    CHECK_FALSE(state.IsPending());
    CHECK(state.Request());
}
