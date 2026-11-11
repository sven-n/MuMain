#pragma once

#include <functional>

// The camera the original drew every UI item with: an identity view at a
// `fieldOfView`-degree vertical field of view over the whole window. Its
// projection is cropped to the box (window pixels from the top left), so the
// box fills the open capture and each RenderItem3D() call (its rectangles in
// window pixels) lands in it exactly where it would have landed on screen;
// native 2D drawn with the depth test off maps to the box the same way.
// g_Camera, the camera's screen centre and the matrices are restored after
// `draw`.
namespace UI::Items
{
void DrawUnderItemCamera(float left, float top, float width, float height, float fieldOfView,
                         const std::function<void()>& draw);
} // namespace UI::Items
