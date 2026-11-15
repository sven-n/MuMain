#pragma once

class OBJECT;

namespace Render
{
// How one draw shows an object's model. It starts as the object's own look; the item effects and styles and the
// map draws change it for the draw, and the object keeps its look.
struct DrawLook
{
    float alpha = 1.f;
    int blendMesh = -1;
    float blendMeshLight = 1.f;
    float blendMeshU = 0.f;
    float blendMeshV = 0.f;
    int hiddenMesh = -1;
    // The level the item is drawn like, for the conditions of its look (design L11): its glow level, as the render
    // level option caps it and its item effect leaves it. Read only.
    int level = 0;

    static DrawLook Of(const OBJECT& o);
};
} // namespace Render

using Render::DrawLook;
