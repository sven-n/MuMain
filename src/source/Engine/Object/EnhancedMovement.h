#pragma once

// The Enhanced Movement option ([Render] EnhancedMovement): the character realism features (smooth turning, look-at,
// nods, flight feel, wing and hair lag, mount lean, the drawn sudden turn, stride matching, momentum, slope lean, foot
// placement, pose fades). Off, characters move and are drawn as they were before them; whatever a feature had applied
// fades out or is taken off. Set at start-up from config.ini and by the options window, read every frame.
namespace Engine::Characters
{
namespace Detail
{
inline bool s_enhancedMovement = true;
} // namespace Detail

[[nodiscard]] inline bool EnhancedMovementEnabled()
{
    return Detail::s_enhancedMovement;
}

// From the next frame on; nothing is saved.
inline void SetEnhancedMovementEnabled(bool enabled)
{
    Detail::s_enhancedMovement = enabled;
}
} // namespace Engine::Characters
