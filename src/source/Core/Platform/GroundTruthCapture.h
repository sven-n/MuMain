#pragma once

namespace mu
{
class GroundTruthCapture
{
public:
    GroundTruthCapture() = delete;

    [[nodiscard]] static double ComputeSSIM(const unsigned char* imageA, const unsigned char* imageB, int width,
                                            int height, int channels);
};
}
