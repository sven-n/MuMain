#pragma once

#include <string_view>

// Post-processing of the 3D scene, switched by the look preset (Render::Look::ApplyLookPreset). The
// bloom and grading values come from the look profile (Render::Look).
namespace Render::PostProcessing
{

// Switches post-processing from the next frame on; nothing is saved. Returns
// false when it was turned on but the renderer cannot post-process; frames then
// stay unprocessed.
bool SetPostProcessingEnabled(bool enabled);

// Hands the look profile's bloom and grading values of the active world to the
// renderer. Call once per frame, before the scene's end.
void SubmitPostProcessingLook();

// `$post view scene|source|bloom`: for diagnosis, shows the bloom source (the
// additive draws alone) or the blurred bloom in place of the scene. Not saved;
// false for an unknown view name.
bool SetPostProcessingView(std::wstring_view name);

} // namespace Render::PostProcessing
