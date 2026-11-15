#pragma once

#include "Engine/Object/EnhancedMovement.h"

// Global settings a movement test changes, put back when the test ends, also when a REQUIRE ends it early: otherwise
// every later test in the binary would run with them.

// FPS_ANIMATION_FACTOR set for a test and put back after it.
class AnimationFactor
{
public:
    explicit AnimationFactor(float factor) : m_saved(FPS_ANIMATION_FACTOR)
    {
        FPS_ANIMATION_FACTOR = factor;
    }
    ~AnimationFactor()
    {
        FPS_ANIMATION_FACTOR = m_saved;
    }
    AnimationFactor(const AnimationFactor&) = delete;
    AnimationFactor& operator=(const AnimationFactor&) = delete;

private:
    float m_saved;
};

// Enhanced Movement switched for a test and put back after it.
class EnhancedMovementSwitch
{
public:
    explicit EnhancedMovementSwitch(bool enabled) : m_saved(Engine::Characters::EnhancedMovementEnabled())
    {
        Engine::Characters::SetEnhancedMovementEnabled(enabled);
    }
    ~EnhancedMovementSwitch()
    {
        Engine::Characters::SetEnhancedMovementEnabled(m_saved);
    }
    EnhancedMovementSwitch(const EnhancedMovementSwitch&) = delete;
    EnhancedMovementSwitch& operator=(const EnhancedMovementSwitch&) = delete;

private:
    bool m_saved;
};
