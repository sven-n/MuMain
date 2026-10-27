#pragma once

#include "Engine/AI/ZzzAI.h"
#include "Render/Textures/ZzzOpenglUtil.h"

#include <cmath>

// Time for the movement features, which work in seconds so they look the same at any frame rate.
namespace Engine::Characters
{
// The seconds this frame stands for (FPS_ANIMATION_FACTOR frames at REFERENCE_FPS).
inline float FrameSeconds()
{
    return FPS_ANIMATION_FACTOR / static_cast<float>(REFERENCE_FPS);
}

// The share of the way left that an ease toward a target covers in `seconds`, getting most of the way there in about
// `easeSeconds` (an exponential ease, the same at any frame rate).
inline float EaseStep(float seconds, float easeSeconds)
{
    return seconds > 0.f ? 1.f - std::exp(-seconds / easeSeconds) : 0.f;
}
} // namespace Engine::Characters
