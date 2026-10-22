#include "Render/Textures/MipChain.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace Render::Textures
{

namespace
{
constexpr std::size_t BytesPerTexel = 4;
constexpr std::size_t AlphaOffset = 3;

// 4x4 ordered-dither ranks; spreads the texels that pass among equal ones.
constexpr std::array<std::uint8_t, 16> DitherRank = {0, 8, 2, 10, 12, 4, 14, 6, 3, 11, 1, 9, 15, 7, 13, 5};
constexpr std::size_t DitherBuckets = DitherRank.size();

[[nodiscard]] std::uint8_t DitherRankAt(std::uint32_t x, std::uint32_t y)
{
    return DitherRank[(y % 4u) * 4u + (x % 4u)];
}

[[nodiscard]] std::size_t ByteCount(std::uint32_t width, std::uint32_t height)
{
    return static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * BytesPerTexel;
}

[[nodiscard]] std::uint32_t HalfSize(std::uint32_t size)
{
    return std::max<std::uint32_t>(1u, size / 2u);
}

// Source texels [begin, end) that destination texel `index` averages.
struct Footprint
{
    std::uint32_t begin;
    std::uint32_t end;
};

[[nodiscard]] Footprint FootprintOf(std::uint32_t index, std::uint32_t sourceSize, std::uint32_t targetSize)
{
    if (sourceSize == 1u)
    {
        return {0u, 1u};
    }
    const std::uint32_t begin = index * 2u;
    const std::uint32_t end = index + 1u == targetSize ? sourceSize : begin + 2u;
    return {begin, end};
}
} // namespace

std::uint32_t CountMipLevels(std::uint32_t width, std::uint32_t height)
{
    if (width == 0u || height == 0u)
    {
        return 0u;
    }

    std::uint32_t size = std::max(width, height);
    std::uint32_t levels = 1u;
    while (size > 1u)
    {
        size /= 2u;
        ++levels;
    }
    return levels;
}

MipLevel DownsampleBox(std::span<const std::uint8_t> rgba, std::uint32_t width, std::uint32_t height)
{
    MipLevel level;
    if (width == 0u || height == 0u || rgba.size() < ByteCount(width, height))
    {
        return level;
    }

    level.width = HalfSize(width);
    level.height = HalfSize(height);
    level.rgba.resize(ByteCount(level.width, level.height));

    for (std::uint32_t y = 0; y < level.height; ++y)
    {
        const Footprint rows = FootprintOf(y, height, level.height);
        for (std::uint32_t x = 0; x < level.width; ++x)
        {
            const Footprint columns = FootprintOf(x, width, level.width);
            std::uint32_t sum[BytesPerTexel] = {};
            for (std::uint32_t sy = rows.begin; sy < rows.end; ++sy)
            {
                const std::uint8_t* row = rgba.data() + ByteCount(width, sy);
                for (std::uint32_t sx = columns.begin; sx < columns.end; ++sx)
                {
                    const std::uint8_t* texel = row + static_cast<std::size_t>(sx) * BytesPerTexel;
                    for (std::size_t channel = 0; channel < BytesPerTexel; ++channel)
                    {
                        sum[channel] += texel[channel];
                    }
                }
            }

            const std::uint32_t count = (rows.end - rows.begin) * (columns.end - columns.begin);
            std::uint8_t* target =
                level.rgba.data() + ByteCount(level.width, y) + static_cast<std::size_t>(x) * BytesPerTexel;
            for (std::size_t channel = 0; channel < BytesPerTexel; ++channel)
            {
                target[channel] = static_cast<std::uint8_t>((sum[channel] + count / 2u) / count);
            }
        }
    }
    return level;
}

std::vector<MipLevel> BuildMipChain(std::span<const std::uint8_t> rgba, std::uint32_t width, std::uint32_t height)
{
    std::vector<MipLevel> chain;
    const std::uint32_t levels = CountMipLevels(width, height);
    if (levels <= 1u || rgba.size() < ByteCount(width, height))
    {
        return chain;
    }

    chain.reserve(levels - 1u);
    std::span<const std::uint8_t> source = rgba;
    std::uint32_t sourceWidth = width;
    std::uint32_t sourceHeight = height;
    for (std::uint32_t level = 1; level < levels; ++level)
    {
        chain.push_back(DownsampleBox(source, sourceWidth, sourceHeight));
        const MipLevel& built = chain.back();
        source = built.rgba;
        sourceWidth = built.width;
        sourceHeight = built.height;
    }
    return chain;
}

AlphaTest AlphaTest::FromReference(float reference)
{
    const float scaled = std::floor(reference * 255.0f);
    if (scaled < 0.0f)
    {
        return {0u};
    }
    return {static_cast<std::uint8_t>(std::min(scaled + 1.0f, 255.0f))};
}

float AlphaCoverage(std::span<const std::uint8_t> rgba, std::uint32_t width, std::uint32_t height, AlphaTest alphaTest)
{
    const std::size_t texels = static_cast<std::size_t>(width) * height;
    if (texels == 0u || rgba.size() < ByteCount(width, height))
    {
        return 0.0f;
    }

    std::size_t passing = 0;
    for (std::size_t i = 0; i < texels; ++i)
    {
        if (rgba[i * BytesPerTexel + AlphaOffset] >= alphaTest.minPassingAlpha)
        {
            ++passing;
        }
    }
    return static_cast<float>(passing) / static_cast<float>(texels);
}

void PreserveAlphaCoverage(MipLevel& level, float coverage, AlphaTest alphaTest)
{
    const std::uint8_t minPassingAlpha = alphaTest.minPassingAlpha;
    const std::size_t texels = static_cast<std::size_t>(level.width) * level.height;
    if (texels == 0u || minPassingAlpha == 0u || level.rgba.size() < ByteCount(level.width, level.height))
    {
        return;
    }

    std::array<std::size_t, 256> histogram = {};
    for (std::size_t i = 0; i < texels; ++i)
    {
        ++histogram[level.rgba[i * BytesPerTexel + AlphaOffset]];
    }

    std::size_t passingNow = 0;
    for (std::size_t alpha = minPassingAlpha; alpha < histogram.size(); ++alpha)
    {
        passingNow += histogram[alpha];
    }

    const double wanted = std::clamp(static_cast<double>(coverage), 0.0, 1.0) * static_cast<double>(texels);
    const auto target = std::min(static_cast<std::size_t>(std::llround(wanted)), texels);
    if (passingNow == target)
    {
        return;
    }

    // The `target` texels with the highest alpha pass. `boundary` is the
    // lowest alpha among them; `tiedPassing` of the texels with exactly that
    // alpha pass, the others fail.
    int boundary = 256;
    std::size_t tiedPassing = 0;
    std::size_t above = 0;
    for (int alpha = 255; alpha >= 0 && target > 0u; --alpha)
    {
        if (above + histogram[alpha] >= target)
        {
            boundary = alpha;
            tiedPassing = target - above;
            break;
        }
        above += histogram[alpha];
    }

    // The scale maps the gap between passing and failing alpha onto the test
    // threshold, so the alpha of texels away from the boundary keeps its shape.
    double gapCenter = 0.0;
    if (target == 0u)
    {
        int highest = 255;
        while (highest > 0 && histogram[highest] == 0u)
        {
            --highest;
        }
        gapCenter = highest + 0.5;
    }
    else if (tiedPassing < histogram[boundary])
    {
        gapCenter = boundary;
    }
    else
    {
        int highestFailing = boundary - 1;
        while (highestFailing >= 0 && histogram[highestFailing] == 0u)
        {
            --highestFailing;
        }
        gapCenter = highestFailing >= 0 ? (boundary + highestFailing) / 2.0 : boundary;
    }
    const double scale = gapCenter > 0.0 ? (minPassingAlpha - 0.5) / gapCenter : 1.0;

    // Among the tied texels, those in the lower dither buckets pass; within
    // the cut bucket, the first ones in row order do.
    std::array<std::size_t, DitherBuckets> tiedPerBucket = {};
    if (boundary < 256)
    {
        for (std::uint32_t y = 0; y < level.height; ++y)
        {
            for (std::uint32_t x = 0; x < level.width; ++x)
            {
                if (level.rgba[ByteCount(level.width, y) + x * BytesPerTexel + AlphaOffset] == boundary)
                {
                    ++tiedPerBucket[DitherRankAt(x, y)];
                }
            }
        }
    }
    std::size_t cutBucket = DitherBuckets;
    std::size_t passingInCutBucket = 0;
    std::size_t remaining = tiedPassing;
    for (std::size_t bucket = 0; bucket < DitherBuckets && remaining > 0u; ++bucket)
    {
        if (tiedPerBucket[bucket] >= remaining)
        {
            cutBucket = bucket;
            passingInCutBucket = remaining;
            break;
        }
        remaining -= tiedPerBucket[bucket];
    }

    const auto highestFailing = static_cast<std::uint8_t>(minPassingAlpha - 1u);
    for (std::uint32_t y = 0; y < level.height; ++y)
    {
        for (std::uint32_t x = 0; x < level.width; ++x)
        {
            std::uint8_t& alpha = level.rgba[ByteCount(level.width, y) + x * BytesPerTexel + AlphaOffset];
            bool passes = alpha > boundary;
            if (alpha == boundary)
            {
                const std::size_t bucket = DitherRankAt(x, y);
                if (bucket < cutBucket)
                {
                    passes = true;
                }
                else if (bucket == cutBucket && passingInCutBucket > 0u)
                {
                    passes = true;
                    --passingInCutBucket;
                }
            }

            const auto scaled = static_cast<std::uint8_t>(std::clamp(std::lround(alpha * scale), 0L, 255L));
            alpha = passes ? std::max(scaled, minPassingAlpha) : std::min(scaled, highestFailing);
        }
    }
}

std::vector<MipLevel> BuildCoveragePreservingMipChain(std::span<const std::uint8_t> rgba, std::uint32_t width,
                                                      std::uint32_t height, AlphaTest alphaTest)
{
    std::vector<MipLevel> chain = BuildMipChain(rgba, width, height);
    const float coverage = AlphaCoverage(rgba, width, height, alphaTest);
    for (MipLevel& level : chain)
    {
        PreserveAlphaCoverage(level, coverage, alphaTest);
    }
    return chain;
}

} // namespace Render::Textures
