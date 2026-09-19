#include "stdafx.h"

#include "GroundTruthCapture.h"

#include <algorithm>

namespace mu
{
double GroundTruthCapture::ComputeSSIM(const unsigned char* imageA, const unsigned char* imageB, int width, int height,
                                       int channels)
{
    if (imageA == nullptr || imageB == nullptr || width <= 0 || height <= 0 || channels <= 0)
    {
        return 0.0;
    }

    constexpr double luminanceRange = 255.0;
    constexpr double c1 = (0.01 * luminanceRange) * (0.01 * luminanceRange);
    constexpr double c2 = (0.03 * luminanceRange) * (0.03 * luminanceRange);
    constexpr int windowSize = 8;

    const auto luminance = [channels](const unsigned char* pixel)
    {
        if (channels == 1)
        {
            return static_cast<double>(pixel[0]);
        }

        const double red = pixel[0];
        const double green = pixel[channels > 1 ? 1 : 0];
        const double blue = pixel[channels > 2 ? 2 : 0];
        return 0.2126 * red + 0.7152 * green + 0.0722 * blue;
    };

    double scoreSum = 0.0;
    int windowCount = 0;
    for (int top = 0; top < height; top += windowSize)
    {
        for (int left = 0; left < width; left += windowSize)
        {
            const int bottom = std::min(top + windowSize, height);
            const int right = std::min(left + windowSize, width);
            const int sampleCount = (bottom - top) * (right - left);

            double meanA = 0.0;
            double meanB = 0.0;
            for (int y = top; y < bottom; ++y)
            {
                for (int x = left; x < right; ++x)
                {
                    const int offset = (y * width + x) * channels;
                    meanA += luminance(imageA + offset);
                    meanB += luminance(imageB + offset);
                }
            }
            meanA /= sampleCount;
            meanB /= sampleCount;

            double varianceA = 0.0;
            double varianceB = 0.0;
            double covariance = 0.0;
            for (int y = top; y < bottom; ++y)
            {
                for (int x = left; x < right; ++x)
                {
                    const int offset = (y * width + x) * channels;
                    const double deltaA = luminance(imageA + offset) - meanA;
                    const double deltaB = luminance(imageB + offset) - meanB;
                    varianceA += deltaA * deltaA;
                    varianceB += deltaB * deltaB;
                    covariance += deltaA * deltaB;
                }
            }

            const double divisor = std::max(1, sampleCount - 1);
            varianceA /= divisor;
            varianceB /= divisor;
            covariance /= divisor;

            const double numerator = (2.0 * meanA * meanB + c1) * (2.0 * covariance + c2);
            const double denominator = (meanA * meanA + meanB * meanB + c1) * (varianceA + varianceB + c2);
            scoreSum += std::clamp(denominator > 0.0 ? numerator / denominator : 1.0, 0.0, 1.0);
            ++windowCount;
        }
    }

    return scoreSum / windowCount;
}
}
