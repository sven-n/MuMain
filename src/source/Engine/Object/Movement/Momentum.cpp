#include "stdafx.h"
#include "Engine/Object/Movement/Momentum.h"

#include "Engine/Object/EnhancedMovement.h"
#include "Engine/Object/Movement/FrameTime.h"
#include "Engine/Object/Movement/MovementTuning.h"
#include "Render/Textures/ZzzOpenglUtil.h"

#include <algorithm>

namespace Engine::Characters
{
float StepMomentum(CHARACTER* c)
{
    CharacterMotion& motion = c->Motion;
    if (!EnhancedMovementEnabled())
    {
        // Full speed at once, as before; the next walk with it on starts from rest.
        motion.MomentumStart = 1.f;
        motion.MomentumSpeed = 1.f;
        motion.MomentumFrame = 0;
        return 1.f;
    }
    const bool fromRest = motion.MomentumFrame == 0 || RENDERED_FRAME - motion.MomentumFrame > MomentumRestFrames;
    if (fromRest)
    {
        motion.MomentumStart = 0.f;
    }
    motion.MomentumFrame = RENDERED_FRAME;
    motion.MomentumStart = std::min(1.f, motion.MomentumStart + FrameSeconds() / MomentumStartSeconds);
    // Eased: it slows out of the start.
    const float start = motion.MomentumStart;
    motion.MomentumSpeed = start * start * (3.f - 2.f * start);
    return motion.MomentumSpeed;
}
} // namespace Engine::Characters
