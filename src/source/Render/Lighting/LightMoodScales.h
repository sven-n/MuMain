#pragma once

#include "Data/GameData/MapData/LightMood.h"

#include <optional>
#include <string_view>

// The arithmetic of the light mood: how a map's mood (Data/Maps/LightMood.json)
// scales the Classic light. The static (baked) light is darkened where it is
// built, so the dynamic lights added on top of it each frame keep their
// strength and stand out.
namespace Render::Lighting::LightMood
{

// The factors applied to the Classic light; all 1 is Classic.
struct Scales
{
    float base = 1.f;
    float figures = 1.f;
    float dynamicGain = 1.f;
    float background = 1.f;
    // The directional (sun) part of the model light.
    float sun = 1.f;
    // Where a lit model colour starts to roll off towards white instead of
    // being cut off at 1; 1 is the Classic cut-off.
    float highlightKnee = 1.f;
    // The baked light at which a place keeps its full brightness; 0 darkens evenly.
    float shadowKeep = 0.f;
    // How much bluer the darkened shade gets, 0 to 0.5.
    float shadeCool = 0.f;
    // The scale places at the shadow keep get (never below base, figures: the figure scale).
    float brightScale = 1.f;

    bool operator==(const Scales&) const = default;
};

inline constexpr int MinStrength = 0;
inline constexpr int MaxStrength = 100;

// With the full mood, lit model colours above this roll off smoothly towards 1,
// so the faces turned to the sun keep their shading instead of turning flat white.
inline constexpr float HighlightShoulderStart = 0.8f;

// The mood at a strength from 0 (Classic) to 100 percent (the full mood):
// every value v becomes 1 + strength × (v − 1), the highlight knee included.
// `highlightKnee` is the full mood's knee (the look profile's lighting.highlightKnee).
[[nodiscard]] Scales Blend(const Data::Maps::LightMood& mood, int strengthPercent,
                           float highlightKnee = HighlightShoulderStart);

// Per CELL: adds Classic static × (figure factor − terrain factor), leaving dynamic light
// untouched. Interpolate this correction only AFTER the factors have been applied per cell;
// dividing sampled darkened static by a factor of sampled weight is not equivalent.
void CorrectFigureLight(const Scales& scales, const float classicStatic[3], float light[3], float brightWeight = 0.f);

// Classic lights the player's own character with a fixed light instead of the
// terrain light; it darkens with the other figures.
void ScaleFixedFigureLight(const Scales& scales, const float classicLight[3], float light[3], float brightWeight = 0.f);

// How much of a place keeps its Classic brightness, 0 to 1, from the luminance of
// its undarkened static light: 0 up to 0.3 x shadowKeep, 1 from shadowKeep on,
// smooth in between; always 0 without a shadow keep.
[[nodiscard]] float BrightWeight(const Scales& scales, float luminance);

// The factors the static light of a place with this bright weight is multiplied
// by, per colour channel: base (tinted by shadeCool) in the shade, max(brightScale, base) where
// bright.
void StaticFactor(const Scales& scales, float brightWeight, float factor[3]);
// The same for the static light on figures, with the figure scale.
void FigureFactor(const Scales& scales, float brightWeight, float factor[3]);

// The model light vector scaled by the sun: Classic lights a face with
// max(0.8 × dot(n, L) + 0.4, 0.2), so this scales the directional part only.
void ScaleSunDirection(const Scales& scales, float direction[3]);

// A lit model colour channel past the highlight knee: unchanged up to the knee,
// then k + (1 − k)(1 − e^(−(v − k)/(1 − k))), which leaves the knee with
// slope 1 and approaches 1 without reaching it. A knee of 1 (Classic) leaves
// the value as it is; the Classic cut-off at 1 follows later. The model
// shaders compute the same curve.
[[nodiscard]] float RollOffHighlight(float value, float knee);

// A dynamic light value as it is added to the frame's terrain light: brightening
// (positive) values times the gain, darkening (negative) ones as they are.
[[nodiscard]] float ScaleDynamicLight(float value, float gain);

// A `$lightmood` argument: "on", "off" or a strength from 0 to 100.
struct Command
{
    std::optional<bool> enabled;
    std::optional<int> strength;
};
// nullopt for anything else; case and surrounding spaces are ignored.
[[nodiscard]] std::optional<Command> ParseCommandArgument(std::wstring_view argument);

} // namespace Render::Lighting::LightMood
