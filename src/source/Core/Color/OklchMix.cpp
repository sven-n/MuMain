#include "stdafx.h"

#include "OklchMix.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace Core::Color
{
namespace
{
using Vector = std::array<double, 3>;
using Matrix = std::array<Vector, 3>;

// Björn Ottosson's OKLab (https://bottosson.github.io/posts/oklab/): linear sRGB to cone
// responses, their cube roots to Lab, and the inverses.
constexpr Matrix LinearToLms = {{{0.4122214708, 0.5363325363, 0.0514459929},
                                 {0.2119034982, 0.6806995451, 0.1073969566},
                                 {0.0883024619, 0.2817188376, 0.6299787005}}};
constexpr Matrix LmsToLab = {{{0.2104542553, 0.7936177850, -0.0040720468},
                              {1.9779984951, -2.4285922050, 0.4505937099},
                              {0.0259040371, 0.7827717662, -0.8086757660}}};
constexpr Matrix LabToLms = {
    {{1.0, 0.3963377774, 0.2158037573}, {1.0, -0.1055613458, -0.0638541728}, {1.0, -0.0894841775, -1.2914855480}}};
constexpr Matrix LmsToLinear = {{{4.0767416621, -3.3077115913, 0.2309699292},
                                 {-1.2684380046, 2.6097574011, -0.3413193965},
                                 {-0.0041960863, -0.7034186147, 1.7076147010}}};

// The sRGB transfer curve (IEC 61966-2-1).
constexpr double SrgbLinearLimit = 0.04045;
constexpr double LinearSrgbLimit = 0.0031308;
constexpr double SrgbSlope = 12.92;
constexpr double SrgbOffset = 0.055;
constexpr double SrgbGamma = 2.4;
constexpr double ChannelMax = 255.0;
// Below this chroma a color is gray and has no hue of its own.
constexpr double GrayChroma = 1e-4;
constexpr double Pi = 3.14159265358979323846;

Vector Multiply(const Matrix& matrix, const Vector& vector)
{
    Vector result{};
    for (size_t row = 0; row < result.size(); ++row)
    {
        result[row] = matrix[row][0] * vector[0] + matrix[row][1] * vector[1] + matrix[row][2] * vector[2];
    }
    return result;
}

double ToLinear(std::uint8_t channel)
{
    const double value = channel / ChannelMax;
    return value <= SrgbLinearLimit ? value / SrgbSlope
                                    : std::pow((value + SrgbOffset) / (1.0 + SrgbOffset), SrgbGamma);
}

std::uint8_t ToChannel(double linear)
{
    const double value = linear <= LinearSrgbLimit
                             ? linear * SrgbSlope
                             : (1.0 + SrgbOffset) * std::pow(linear, 1.0 / SrgbGamma) - SrgbOffset;
    return static_cast<std::uint8_t>(std::lround(std::clamp(value, 0.0, 1.0) * ChannelMax));
}

// Lightness, chroma, hue (radians).
struct Lch
{
    double lightness = 0.0;
    double chroma = 0.0;
    double hue = 0.0;
};

Lch ToLch(Rgb color)
{
    Vector lms = Multiply(LinearToLms, {ToLinear(color.red), ToLinear(color.green), ToLinear(color.blue)});
    for (double& value : lms)
    {
        value = std::cbrt(value);
    }
    const Vector lab = Multiply(LmsToLab, lms);
    return {lab[0], std::hypot(lab[1], lab[2]), std::atan2(lab[2], lab[1])};
}

Rgb ToRgb(const Lch& color)
{
    Vector lms =
        Multiply(LabToLms, {color.lightness, color.chroma * std::cos(color.hue), color.chroma * std::sin(color.hue)});
    for (double& value : lms)
    {
        value = value * value * value;
    }
    const Vector linear = Multiply(LmsToLinear, lms);
    return {ToChannel(linear[0]), ToChannel(linear[1]), ToChannel(linear[2])};
}

// The hue step from `from` to `to` the shorter way around.
double HueStep(double from, double to)
{
    return std::remainder(to - from, 2.0 * Pi);
}
} // namespace

Rgb MixOklch(Rgb from, Rgb to, float share)
{
    const double t = std::clamp(static_cast<double>(share), 0.0, 1.0);
    Lch start = ToLch(from);
    Lch end = ToLch(to);
    if (start.chroma < GrayChroma)
    {
        start.hue = end.hue;
    }
    if (end.chroma < GrayChroma)
    {
        end.hue = start.hue;
    }
    const Lch mixed{start.lightness + (end.lightness - start.lightness) * t,
                    start.chroma + (end.chroma - start.chroma) * t, start.hue + HueStep(start.hue, end.hue) * t};
    return ToRgb(mixed);
}
} // namespace Core::Color
