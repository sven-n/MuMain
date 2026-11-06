#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include <doctest.h>

#include "Render/Textures/MipChain.h"

namespace
{
std::vector<std::uint8_t> MakeImage(std::uint32_t width, std::uint32_t height)
{
    std::vector<std::uint8_t> rgba(static_cast<std::size_t>(width) * height * 4u);
    for (std::size_t i = 0; i < rgba.size(); ++i)
    {
        rgba[i] = static_cast<std::uint8_t>(i * 7u);
    }
    return rgba;
}

std::vector<std::pair<std::uint32_t, std::uint32_t>> ChainSizes(std::uint32_t width, std::uint32_t height)
{
    const std::vector<std::uint8_t> image = MakeImage(width, height);
    std::vector<std::pair<std::uint32_t, std::uint32_t>> sizes;
    for (const Render::Textures::MipLevel& level : Render::Textures::BuildMipChain(image, width, height))
    {
        CHECK(level.rgba.size() == static_cast<std::size_t>(level.width) * level.height * 4u);
        sizes.emplace_back(level.width, level.height);
    }
    return sizes;
}

using Sizes = std::vector<std::pair<std::uint32_t, std::uint32_t>>;
} // namespace

TEST_CASE("a full mip chain ends at 1x1 [render][mipmap]")
{
    CHECK(Render::Textures::CountMipLevels(512, 512) == 10);
    CHECK(Render::Textures::CountMipLevels(256, 64) == 9);
    CHECK(Render::Textures::CountMipLevels(3, 5) == 3);
    CHECK(Render::Textures::CountMipLevels(1, 1) == 1);
    CHECK(Render::Textures::CountMipLevels(0, 7) == 0);
}

TEST_CASE("mip chain levels halve each axis down to one texel [render][mipmap]")
{
    CHECK(ChainSizes(512, 512) ==
          Sizes{{256, 256}, {128, 128}, {64, 64}, {32, 32}, {16, 16}, {8, 8}, {4, 4}, {2, 2}, {1, 1}});
    CHECK(ChainSizes(256, 64) == Sizes{{128, 32}, {64, 16}, {32, 8}, {16, 4}, {8, 2}, {4, 1}, {2, 1}, {1, 1}});
    CHECK(ChainSizes(3, 5) == Sizes{{1, 2}, {1, 1}});
    CHECK(ChainSizes(1, 1).empty());
}

TEST_CASE("box filter averages each 2x2 block per channel [render][mipmap]")
{
    // 2x2 image: four texels with distinct channels.
    const std::vector<std::uint8_t> image = {
        0,  10, 100, 255, //
        4,  20, 100, 255, //
        8,  30, 100, 0,   //
        12, 40, 100, 0,   //
    };

    const Render::Textures::MipLevel level = Render::Textures::DownsampleBox(image, 2, 2);

    REQUIRE(level.width == 1);
    REQUIRE(level.height == 1);
    CHECK(level.rgba == std::vector<std::uint8_t>{6, 25, 100, 128});
}

TEST_CASE("an odd axis folds its last row or column into the last texel [render][mipmap]")
{
    // 3x1 image, red channel 0, 30, 90: one output texel averages all three.
    const std::vector<std::uint8_t> image = {
        0,  0, 0, 255, //
        30, 0, 0, 255, //
        90, 0, 0, 255, //
    };

    const Render::Textures::MipLevel level = Render::Textures::DownsampleBox(image, 3, 1);

    REQUIRE(level.width == 1);
    REQUIRE(level.height == 1);
    CHECK(level.rgba == std::vector<std::uint8_t>{40, 0, 0, 255});
}

TEST_CASE("a too small input builds no chain [render][mipmap]")
{
    const std::vector<std::uint8_t> image(4u * 4u * 4u - 1u);

    CHECK(Render::Textures::BuildMipChain(image, 4, 4).empty());
    CHECK(Render::Textures::DownsampleBox(image, 4, 4).rgba.empty());
}

namespace
{
// Foliage-like alpha: opaque blobs on a transparent background (0 or 255).
std::vector<std::uint8_t> MakeFoliage(std::uint32_t width, std::uint32_t height)
{
    std::vector<std::uint8_t> rgba(static_cast<std::size_t>(width) * height * 4u);
    for (std::uint32_t y = 0; y < height; ++y)
    {
        for (std::uint32_t x = 0; x < width; ++x)
        {
            const std::uint32_t cellX = x % 16u;
            const std::uint32_t cellY = y % 16u;
            const bool leaf =
                (cellX - 8u) * (cellX - 8u) + (cellY - 8u) * (cellY - 8u) < 20u || (x * 7u + y * 3u) % 23u == 0u;
            std::uint8_t* texel = &rgba[(static_cast<std::size_t>(y) * width + x) * 4u];
            texel[0] = static_cast<std::uint8_t>(x);
            texel[1] = static_cast<std::uint8_t>(y);
            texel[2] = 40;
            texel[3] = leaf ? 255 : 0;
        }
    }
    return rgba;
}

// Smooth alpha ramp with noise, like a soft-edged hair or fence texture.
std::vector<std::uint8_t> MakeSoftEdges(std::uint32_t width, std::uint32_t height)
{
    std::vector<std::uint8_t> rgba(static_cast<std::size_t>(width) * height * 4u);
    std::uint32_t seed = 12345u;
    for (std::size_t i = 0; i < rgba.size(); i += 4u)
    {
        seed = seed * 1664525u + 1013904223u;
        const std::size_t texel = i / 4u;
        const std::uint32_t x = static_cast<std::uint32_t>(texel % width);
        rgba[i + 0] = 200;
        rgba[i + 1] = 180;
        rgba[i + 2] = 160;
        rgba[i + 3] = static_cast<std::uint8_t>(std::min<std::uint32_t>(255u, x * 255u / width + (seed >> 28)));
    }
    return rgba;
}

std::size_t PassingTexels(const Render::Textures::MipLevel& level, std::uint8_t minPassingAlpha)
{
    std::size_t passing = 0;
    for (std::size_t i = 3; i < level.rgba.size(); i += 4u)
    {
        passing += level.rgba[i] >= minPassingAlpha ? 1u : 0u;
    }
    return passing;
}

void CheckCoverageMatches(const std::vector<std::uint8_t>& image, std::uint32_t width, std::uint32_t height)
{
    const auto alphaTest = Render::Textures::AlphaTest::FromReference(Render::Textures::AlphaTestReference);
    const std::uint8_t minPassing = alphaTest.minPassingAlpha;
    const float coverage = Render::Textures::AlphaCoverage(image, width, height, alphaTest);
    const std::vector<Render::Textures::MipLevel> chain =
        Render::Textures::BuildCoveragePreservingMipChain(image, width, height, alphaTest);

    REQUIRE(chain.size() + 1u == Render::Textures::CountMipLevels(width, height));
    for (const Render::Textures::MipLevel& level : chain)
    {
        const double texels = static_cast<double>(level.width) * level.height;
        const double levelCoverage = static_cast<double>(PassingTexels(level, minPassing)) / texels;
        INFO("level " << level.width << "x" << level.height << " coverage " << levelCoverage << " vs " << coverage);
        CHECK(std::abs(levelCoverage - coverage) <= 1.0 / texels);
    }
}
} // namespace

TEST_CASE("the alpha test reference maps to the smallest passing byte [render][mipmap]")
{
    CHECK(Render::Textures::AlphaTest::FromReference(0.25f).minPassingAlpha == 64);
    CHECK(Render::Textures::AlphaTest::FromReference(0.0f).minPassingAlpha == 1);
    CHECK(Render::Textures::AlphaTest::FromReference(-1.0f).minPassingAlpha == 0);
    CHECK(Render::Textures::AlphaTest::FromReference(1.0f).minPassingAlpha == 255);
}

TEST_CASE("plain box filtering thins foliage at a distance [render][mipmap]")
{
    // Guards the coverage test below: without rescaling, coverage drifts.
    const std::vector<std::uint8_t> image = MakeFoliage(256, 256);
    const auto alphaTest = Render::Textures::AlphaTest::FromReference(Render::Textures::AlphaTestReference);
    const std::uint8_t minPassing = alphaTest.minPassingAlpha;
    const float coverage = Render::Textures::AlphaCoverage(image, 256, 256, alphaTest);
    const std::vector<Render::Textures::MipLevel> chain = Render::Textures::BuildMipChain(image, 256, 256);

    bool drifted = false;
    for (const Render::Textures::MipLevel& level : chain)
    {
        const double texels = static_cast<double>(level.width) * level.height;
        const double levelCoverage = static_cast<double>(PassingTexels(level, minPassing)) / texels;
        drifted = drifted || std::abs(levelCoverage - coverage) > 1.0 / texels;
    }
    CHECK(drifted);
}

TEST_CASE("each level keeps level 0's alpha-test coverage within one texel [render][mipmap]")
{
    SUBCASE("binary foliage 256x256")
    {
        CheckCoverageMatches(MakeFoliage(256, 256), 256, 256);
    }
    SUBCASE("binary foliage 512x128")
    {
        CheckCoverageMatches(MakeFoliage(512, 128), 512, 128);
    }
    SUBCASE("soft edges 128x128")
    {
        CheckCoverageMatches(MakeSoftEdges(128, 128), 128, 128);
    }
    SUBCASE("non-power-of-two 37x19")
    {
        CheckCoverageMatches(MakeFoliage(37, 19), 37, 19);
    }
}

TEST_CASE("coverage rescaling changes alpha only [render][mipmap]")
{
    const std::vector<std::uint8_t> image = MakeFoliage(64, 64);
    const auto alphaTest = Render::Textures::AlphaTest::FromReference(Render::Textures::AlphaTestReference);
    const std::vector<Render::Textures::MipLevel> plain = Render::Textures::BuildMipChain(image, 64, 64);
    const std::vector<Render::Textures::MipLevel> preserved =
        Render::Textures::BuildCoveragePreservingMipChain(image, 64, 64, alphaTest);

    REQUIRE(plain.size() == preserved.size());
    for (std::size_t level = 0; level < plain.size(); ++level)
    {
        REQUIRE(plain[level].rgba.size() == preserved[level].rgba.size());
        for (std::size_t i = 0; i < plain[level].rgba.size(); ++i)
        {
            if (i % 4u != 3u)
            {
                CHECK(plain[level].rgba[i] == preserved[level].rgba[i]);
            }
        }
    }
}

TEST_CASE("opaque and already matching levels are left unchanged [render][mipmap]")
{
    std::vector<std::uint8_t> opaque = MakeImage(32, 32);
    for (std::size_t i = 3; i < opaque.size(); i += 4u)
    {
        opaque[i] = 255;
    }
    const auto alphaTest = Render::Textures::AlphaTest::FromReference(Render::Textures::AlphaTestReference);

    CHECK(Render::Textures::BuildCoveragePreservingMipChain(opaque, 32, 32, alphaTest).front().rgba ==
          Render::Textures::BuildMipChain(opaque, 32, 32).front().rgba);

    Render::Textures::MipLevel half{2, 1, {0, 0, 0, 255, 0, 0, 0, 0}};
    const std::vector<std::uint8_t> before = half.rgba;
    Render::Textures::PreserveAlphaCoverage(half, 0.5f, alphaTest);
    CHECK(half.rgba == before);
}

TEST_CASE("tied alpha at the pass boundary is split to hit the coverage [render][mipmap]")
{
    // Sixteen texels of alpha 128 (all pass); a quarter must pass.
    Render::Textures::MipLevel level{4, 4, std::vector<std::uint8_t>(std::size_t{64}, 128)};
    const auto alphaTest = Render::Textures::AlphaTest::FromReference(Render::Textures::AlphaTestReference);
    const std::uint8_t minPassing = alphaTest.minPassingAlpha;

    Render::Textures::PreserveAlphaCoverage(level, 0.25f, alphaTest);

    CHECK(PassingTexels(level, minPassing) == 4);
    for (std::size_t i = 3; i < level.rgba.size(); i += 4u)
    {
        CHECK((level.rgba[i] == minPassing || level.rgba[i] == minPassing - 1));
    }
}
