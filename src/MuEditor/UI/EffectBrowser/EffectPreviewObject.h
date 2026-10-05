#pragma once

#ifdef _EDITOR

#include <cstdint>

class OBJECT;

namespace Render::Effects
{
struct EffectDescriptor;
}

namespace MuEditor::Effects
{
// Makes `o` what CreateEffect makes of `type` with `subType`, called without
// light, angle and scale arguments at the origin: the setup every effect gets
// and the creation values of its row (`descriptor`, null without one). The
// creation hook is not run and `o` stays outside the effect pools. An effect
// whose values start it invisible is made visible. Returns the PreviewNote
// flags this adds.
std::uint16_t BuildPreviewEffectObject(OBJECT& o, int type, int subType,
                                       const Render::Effects::EffectDescriptor* descriptor);

// Whether MoveEffect plays the model's animation for this type, in the code
// it runs for every effect.
bool IsAnimatedByMoveEffect(int type, int subType);
} // namespace MuEditor::Effects

#endif // _EDITOR
