#pragma once

#include "Render/Renderer/MuRenderer.h"

#include <array>
#include <cstdint>
#include <vector>

typedef struct _Mesh_t Mesh_t;

namespace Render::Models::Gltf
{
struct SkinnedVertex;
struct LodMesh;
struct PackedMesh;
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

// The GPU vertex of a glTF vertex: its bind position, normal and tangent
// (PackTangent), its joints and their weights as unorm bytes that sum to 255,
// the largest weight first (the shader takes a vertex whose first weight is
// above 0 as weighted). The bone indices of the BMD fields name the dominant
// joint.
[[nodiscard]] mu::SkinnedWeightedVertex3D PackWeightedSkinnedVertex(const Gltf::SkinnedVertex& vertex, float u,
                                                                    float v);

// The GPU tangent of a glTF vertex (SkinnedVertex::bindTangent): xyz as snorm16
// (clamped to -1..1) and w as +-32767 for the handedness; a tangent with w 0
// (no normal map) packs as all 0, which the shader reads as "no tangent".
[[nodiscard]] std::array<std::int16_t, 4> PackTangent(const std::array<float, 4>& tangent);

// The palette of a weighted draw: per joint of `model`, 12 floats (a 3x4
// row-major matrix) that take a bind-space position to where
// Gltf::SkinnedModel::SkinPosition puts it for the joint: the bone matrix
// times the joint's inverse bind matrix, scaled like BMD::SkinVertex scales -
// the joint-space position by `restPoseScale` (0: unscaled) when `boneScale`
// is 1, else the rotated position by `boneScale`.
void BuildWeightedSkinPalette(const Gltf::SkinnedModel& model, const float (*boneMatrices)[3][4], float boneScale,
                              float restPoseScale, std::vector<float>& palette);

// A model file's mesh at LOD `lod` packed for an indexed draw: `lodMesh`
// when that LOD draws its own mesh, else BMD mesh `meshIndex` (`mesh`) on the
// file's skin. Its corners, expanded from the indices (mu::ExpandIndexedCorners),
// are PackWeightedSkinnedVertex of every triangle corner in triangle order,
// texture coordinates + 0 (the draw's sum with no wave). Packed on the first
// call and kept on the model (SkinnedModel::packedMeshes), unchanged after
// that: a draw with a wave adds it on a copy, and the renderer may cache the
// mesh on the GPU under its vertices' address (SkinnedModel's destructor
// releases it).
[[nodiscard]] Gltf::PackedMesh& PackedWeightedMeshOf(const Gltf::SkinnedModel& model, const Mesh_t& mesh, int meshIndex,
                                                     int lod, const Gltf::LodMesh* lodMesh);
} // namespace Render::Models
