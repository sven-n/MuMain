#include "LightMoodScales.h"

#include <algorithm>
#include <cmath>
#include <cwctype>
#include <string>

namespace Render::Lighting::LightMood
{
namespace
{
constexpr int ColorChannels = 3;
constexpr float PercentScale = 100.f;

float BlendValue(double value, float strength)
{
    return 1.f + strength * (static_cast<float>(value) - 1.f);
}

std::wstring Normalize(std::wstring_view argument)
{
    const auto first = argument.find_first_not_of(L" \t");
    if (first == std::wstring_view::npos)
    {
        return {};
    }
    const auto last = argument.find_last_not_of(L" \t");
    std::wstring text(argument.substr(first, last - first + 1));
    std::transform(text.begin(), text.end(), text.begin(),
                   [](wchar_t c) { return static_cast<wchar_t>(std::towlower(c)); });
    return text;
}

std::optional<int> ParseStrength(const std::wstring& text)
{
    constexpr size_t MaxDigits = 3;
    if (text.empty() || text.size() > MaxDigits ||
        !std::all_of(text.begin(), text.end(), [](wchar_t c) { return std::iswdigit(c) != 0; }))
    {
        return std::nullopt;
    }
    const int strength = std::stoi(text);
    if (strength < MinStrength || strength > MaxStrength)
    {
        return std::nullopt;
    }
    return strength;
}
} // namespace

Scales Blend(const Data::Maps::LightMood& mood, int strengthPercent, float highlightKnee)
{
    const float strength = static_cast<float>(std::clamp(strengthPercent, MinStrength, MaxStrength)) / PercentScale;
    return {BlendValue(mood.base, strength),
            BlendValue(mood.figures, strength),
            BlendValue(mood.dynamicGain, strength),
            BlendValue(mood.background, strength),
            BlendValue(mood.sun, strength),
            BlendValue(highlightKnee, strength),
            strength > 0.f ? static_cast<float>(mood.shadowKeep) : 0.f,
            static_cast<float>(mood.shadeCool) * strength,
            BlendValue(mood.brightScale, strength)};
}

namespace
{
// The shade tint: red and green darken a little more than blue.
void ShadeTint(const Scales& scales, float tint[3])
{
    tint[0] = 1.f - scales.shadeCool;
    tint[1] = 1.f - 0.5f * scales.shadeCool;
    tint[2] = 1.f;
}

void Factor(const Scales& scales, float scale, float brightWeight, float factor[3])
{
    float tint[3];
    ShadeTint(scales, tint);
    const float weight = std::clamp(brightWeight, 0.f, 1.f);
    for (int i = 0; i < ColorChannels; ++i)
    {
        const float shade = scale * tint[i];
        const float bright = std::max(scales.brightScale, scale);
        factor[i] = shade + weight * (bright - shade);
    }
}
} // namespace

float BrightWeight(const Scales& scales, float luminance)
{
    if (scales.shadowKeep <= 0.f)
    {
        return 0.f;
    }
    constexpr float ShadeEnd = 0.3f;
    const float low = ShadeEnd * scales.shadowKeep;
    const float t = std::clamp((luminance - low) / (scales.shadowKeep - low), 0.f, 1.f);
    return t * t * (3.f - 2.f * t);
}

void StaticFactor(const Scales& scales, float brightWeight, float factor[3])
{
    Factor(scales, scales.base, brightWeight, factor);
}

void FigureFactor(const Scales& scales, float brightWeight, float factor[3])
{
    Factor(scales, scales.figures, brightWeight, factor);
}

void CorrectFigureLight(const Scales& scales, const float classicStatic[3], float light[3], float brightWeight)
{
    float baseFactor[3];
    float figureFactor[3];
    StaticFactor(scales, brightWeight, baseFactor);
    FigureFactor(scales, brightWeight, figureFactor);
    for (int i = 0; i < ColorChannels; ++i)
    {
        light[i] += classicStatic[i] * (figureFactor[i] - baseFactor[i]);
    }
}

void ScaleFixedFigureLight(const Scales& scales, const float classicLight[3], float light[3], float brightWeight)
{
    float factor[3];
    FigureFactor(scales, brightWeight, factor);
    for (int i = 0; i < ColorChannels; ++i)
    {
        light[i] = classicLight[i] * factor[i];
    }
}

void ScaleSunDirection(const Scales& scales, float direction[3])
{
    if (scales.sun == 1.f)
    {
        return;
    }
    for (int i = 0; i < ColorChannels; ++i)
    {
        direction[i] *= scales.sun;
    }
}

float RollOffHighlight(float value, float knee)
{
    if (knee >= 1.f || value <= knee)
    {
        return value;
    }
    const float range = 1.f - knee;
    return knee + range * (1.f - std::exp(-(value - knee) / range));
}

float ScaleDynamicLight(float value, float gain)
{
    return value > 0.f ? value * gain : value;
}

std::optional<Command> ParseCommandArgument(std::wstring_view argument)
{
    const std::wstring text = Normalize(argument);
    if (text == L"on")
    {
        return Command{true, std::nullopt};
    }
    if (text == L"off")
    {
        return Command{false, std::nullopt};
    }
    if (const std::optional<int> strength = ParseStrength(text))
    {
        return Command{std::nullopt, strength};
    }
    return std::nullopt;
}

} // namespace Render::Lighting::LightMood
