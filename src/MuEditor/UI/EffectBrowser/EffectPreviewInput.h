#pragma once

#ifdef _EDITOR

#include "EffectPreviewCamera.h"

namespace MuEditor::Effects::PreviewInput
{
// The view of a preview as an ImGui item of `width` x `height`: dragging turns
// the camera, dragging with the middle mouse button moves the picture, the
// wheel zooms, a double click puts the camera back; a tooltip says so. The
// view is the last item afterwards, for the picture's rectangle.
void RenderView(const char* id, float width, float height, EffectPreviewCamera& camera);
} // namespace MuEditor::Effects::PreviewInput

#endif // _EDITOR
