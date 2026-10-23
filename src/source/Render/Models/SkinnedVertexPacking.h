#pragma once

#include "Render/Renderer/MuRenderer.h"

#include <cstdint>
#include <vector>

namespace Render::Models::Gltf
{
struct SkinnedVertex;
class SkinnedModel;
} // namespace Render::Models::Gltf

// The vertices and bone palette of a GPU-skinned draw (skinned_textured.vert).
// A BMD vertex follows one bone, as it always did; a glTF vertex blends up to
// four joints by weight from bind space, with a palette that already holds
// each joint's bone matrix times its inverse bind matrix.
namespace Render::Models
{
// The GPU vertex of a BMD vertex: its rest position and normal in the space of
// their bones, no weights.
[[nodiscard]] mu::SkinnedVertex3D PackBmdSkinnedVertex(const float position[3], int positionBone, const float normal[3],
                                                       int normalBone, float u, float v);

// The GPU vertex of a glTF vertex: its bind position and normal, its joints and
// their weights as unorm bytes that sum to 255, the largest weight first (the
// shader takes a vertex whose first weight is above 0 as weighted). The bone
// indices of the BMD fields name the dominant joint.
[[nodiscard]] mu::SkinnedVertex3D PackWeightedSkinnedVertex(const Gltf::SkinnedVertex& vertex, float u, float v);

// The palette of a weighted draw: per joint of `model`, 12 floats (a 3x4
// row-major matrix) that take a bind-space position to where
// Gltf::SkinnedModel::SkinPosition puts it for the joint: the bone matrix
// times the joint's inverse bind matrix, scaled like BMD::SkinVertex scales -
// the joint-space position by `restPoseScale` (0: unscaled) when `boneScale`
// is 1, else the rotated position by `boneScale`.
void BuildWeightedSkinPalette(const Gltf::SkinnedModel& model, const float (*boneMatrices)[3][4], float boneScale,
                              float restPoseScale, std::vector<float>& palette);
} // namespace Render::Models
