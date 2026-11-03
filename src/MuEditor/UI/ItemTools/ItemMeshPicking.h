#pragma once

#ifdef _EDITOR

#include <array>
#include <optional>

class BMD;

namespace MuEditor::Effects
{
class EffectPreviewCamera;
}

// Which mesh a click on the Item Model preview hits: a ray from the mouse through the preview camera, tested against
// the triangles of the item's meshes as they are posed for the drawing; the nearest hit wins.
namespace MuEditor::Items::MeshPicking
{
using Vector = std::array<float, 3>;

struct Ray
{
    Vector origin{};
    // A unit vector.
    Vector direction{};
};

// The ray through a point of the preview: `viewX` and `viewY` from -1 to 1, left to right and bottom to top, in a
// view `aspect` (width to height) wide, as EffectPreviewScene::Draw sets up the projection.
[[nodiscard]] Ray RayThroughView(const MuEditor::Effects::EffectPreviewCamera& camera, float aspect, float viewX,
                                 float viewY);

// How far along the ray it hits the triangle a, b, c from either side; nothing when it misses.
[[nodiscard]] std::optional<float> HitTriangle(const Ray& ray, const float* a, const float* b, const float* c);

// The nearest hit of one of the model's meshes, its vertices posed in `vertices` (a quad is two triangles).
[[nodiscard]] std::optional<float> HitMesh(const Ray& ray, const BMD& model, int mesh, float (*vertices)[3]);
} // namespace MuEditor::Items::MeshPicking

#endif // _EDITOR
