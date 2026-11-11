#pragma once

namespace Scenes::PostProcessing
{

// Marks the end of the frame's 3D world. Call once per frame in every scene
// that draws one, after its last 3D draw (sprites and particles included) and
// before its UI: only what was drawn before is post-processed.
void EndSceneGeometry();

// Draws the pets. Like their owners, pets keep their Classic look: no bloom from them.
void RenderPetsWithoutBloom();

} // namespace Scenes::PostProcessing
