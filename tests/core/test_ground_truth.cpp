// Story 4.1.1: Ground Truth Capture Mechanism [VS1-RENDER-GROUNDTRUTH-CAPTURE]
//
// GREEN PHASE: GroundTruthCapture.h/cpp are implemented. All SSIM tests pass.
//
// NOTE: Tests are pure-logic only — no OpenGL calls.
// ComputeSSIM() is always compiled (no ENABLE_GROUND_TRUTH_CAPTURE guard).
// glReadPixels-dependent code is excluded on non-Windows builds.
//
// Run with: ctest --test-dir MuMain/build -R ground_truth

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cstdint>
#include <cstring>
#include <vector>

#include "GroundTruthCapture.h"

using Catch::Matchers::WithinAbs;

// ---------------------------------------------------------------------------
// AC-STD-2 (AC-5): SSIM on identical buffers — expects score >= 0.99 (PASS)
// ---------------------------------------------------------------------------
TEST_CASE("AC-5: SSIM on identical buffers returns score >= 0.99", "[core][ground_truth][ac-5]")
{
    SECTION("8x8 identical solid-gray buffers produce SSIM = 1.0")
    {
        constexpr int width = 8;
        constexpr int height = 8;
        constexpr int channels = 3;
        constexpr std::size_t size = static_cast<std::size_t>(width * height * channels);

        std::vector<unsigned char> imgA(size, 128u);
        std::vector<unsigned char> imgB(size, 128u);

        double score = mu::GroundTruthCapture::ComputeSSIM(
            imgA.data(), imgB.data(), width, height, channels);

        REQUIRE(score >= 0.99);
    }

    SECTION("16x16 identical gradient buffers produce SSIM = 1.0")
    {
        constexpr int width = 16;
        constexpr int height = 16;
        constexpr int channels = 3;
        constexpr std::size_t size = static_cast<std::size_t>(width * height * channels);

        std::vector<unsigned char> imgA(size);
        for (std::size_t i = 0; i < size; ++i)
        {
            imgA[i] = static_cast<unsigned char>(i % 256);
        }
        std::vector<unsigned char> imgB = imgA; // exact copy

        double score = mu::GroundTruthCapture::ComputeSSIM(
            imgA.data(), imgB.data(), width, height, channels);

        REQUIRE(score >= 0.99);
    }

    SECTION("SSIM score for identical images is capped at 1.0")
    {
        constexpr int width = 8;
        constexpr int height = 8;
        constexpr int channels = 3;
        constexpr std::size_t size = static_cast<std::size_t>(width * height * channels);

        std::vector<unsigned char> imgA(size, 200u);
        std::vector<unsigned char> imgB(size, 200u);

        double score = mu::GroundTruthCapture::ComputeSSIM(
            imgA.data(), imgB.data(), width, height, channels);

        REQUIRE(score <= 1.0);
        REQUIRE(score >= 0.99);
    }
}

// ---------------------------------------------------------------------------
// AC-STD-2 (AC-5): SSIM on randomized / dissimilar buffers — expects score < 0.99 (FAIL)
// ---------------------------------------------------------------------------
TEST_CASE("AC-5: SSIM on dissimilar buffers returns score < 0.99", "[core][ground_truth][ac-5]")
{
    SECTION("8x8 inverted buffers produce SSIM well below 0.99")
    {
        constexpr int width = 8;
        constexpr int height = 8;
        constexpr int channels = 3;
        constexpr std::size_t size = static_cast<std::size_t>(width * height * channels);

        // imgA: all white; imgB: all black — maximum contrast, minimum structural similarity
        std::vector<unsigned char> imgA(size, 255u);
        std::vector<unsigned char> imgB(size, 0u);

        double score = mu::GroundTruthCapture::ComputeSSIM(
            imgA.data(), imgB.data(), width, height, channels);

        REQUIRE(score < 0.99);
    }

    SECTION("8x8 checkerboard vs solid buffer produces SSIM < 0.99")
    {
        constexpr int width = 8;
        constexpr int height = 8;
        constexpr int channels = 3;
        constexpr std::size_t size = static_cast<std::size_t>(width * height * channels);

        std::vector<unsigned char> imgA(size);
        std::vector<unsigned char> imgB(size, 128u);

        // Checkerboard pattern: alternating 0/255 per pixel
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                unsigned char val = ((x + y) % 2 == 0) ? 255u : 0u;
                int base = (y * width + x) * channels;
                imgA[base] = val;
                imgA[base + 1] = val;
                imgA[base + 2] = val;
            }
        }

        double score = mu::GroundTruthCapture::ComputeSSIM(
            imgA.data(), imgB.data(), width, height, channels);

        REQUIRE(score < 0.99);
    }

    SECTION("16x16 random-noise vs solid buffer produces SSIM < 0.99")
    {
        constexpr int width = 16;
        constexpr int height = 16;
        constexpr int channels = 3;
        constexpr std::size_t size = static_cast<std::size_t>(width * height * channels);

        std::vector<unsigned char> imgA(size);
        std::vector<unsigned char> imgB(size, 128u);

        // Pseudo-random noise pattern (deterministic seed for reproducibility)
        unsigned int seed = 42u;
        for (std::size_t i = 0; i < size; ++i)
        {
            seed = seed * 1664525u + 1013904223u; // LCG
            imgA[i] = static_cast<unsigned char>(seed & 0xFFu);
        }

        double score = mu::GroundTruthCapture::ComputeSSIM(
            imgA.data(), imgB.data(), width, height, channels);

        REQUIRE(score < 0.99);
    }
}

// ---------------------------------------------------------------------------
// AC-5 (edge cases): SSIM result is bounded and stable
// ---------------------------------------------------------------------------
TEST_CASE("AC-5: SSIM score is bounded in [0.0, 1.0]", "[core][ground_truth][ac-5]")
{
    SECTION("All-zero buffers produce a valid bounded score")
    {
        constexpr int width = 8;
        constexpr int height = 8;
        constexpr int channels = 3;
        constexpr std::size_t size = static_cast<std::size_t>(width * height * channels);

        std::vector<unsigned char> imgA(size, 0u);
        std::vector<unsigned char> imgB(size, 0u);

        double score = mu::GroundTruthCapture::ComputeSSIM(
            imgA.data(), imgB.data(), width, height, channels);

        // Both images identical (all-zero) → should be 1.0 or close
        // The C1/C2 stabilisers prevent division-by-zero for flat images
        REQUIRE(score >= 0.0);
        REQUIRE(score <= 1.0);
    }

    SECTION("Single-channel (luminance) buffers are supported")
    {
        constexpr int width = 8;
        constexpr int height = 8;
        constexpr int channels = 1;
        constexpr std::size_t size = static_cast<std::size_t>(width * height * channels);

        std::vector<unsigned char> imgA(size, 100u);
        std::vector<unsigned char> imgB(size, 100u);

        double score = mu::GroundTruthCapture::ComputeSSIM(
            imgA.data(), imgB.data(), width, height, channels);

        REQUIRE(score >= 0.99);
        REQUIRE(score <= 1.0);
    }
}

// ---------------------------------------------------------------------------
// AC-VAL-2: SSIM correctly identifies identical vs known-different
// (Explicit regression guard that the SSIM function works end-to-end)
// ---------------------------------------------------------------------------
TEST_CASE("AC-VAL-2: SSIM correctly distinguishes identical from different images",
          "[core][ground_truth][ac-val-2]")
{
    constexpr int width = 16;
    constexpr int height = 16;
    constexpr int channels = 3;
    constexpr std::size_t size = static_cast<std::size_t>(width * height * channels);

    // Build two distinct images
    std::vector<unsigned char> identical_a(size, 150u);
    std::vector<unsigned char> identical_b(size, 150u); // same as a
    std::vector<unsigned char> different(size, 10u);    // very different from a

    double same_score = mu::GroundTruthCapture::ComputeSSIM(
        identical_a.data(), identical_b.data(), width, height, channels);
    double diff_score = mu::GroundTruthCapture::ComputeSSIM(
        identical_a.data(), different.data(), width, height, channels);

    // Identical → pass threshold
    REQUIRE(same_score >= 0.99);
    // Different → fail threshold
    REQUIRE(diff_score < 0.99);
    // Identical score > different score
    REQUIRE(same_score > diff_score);
}
