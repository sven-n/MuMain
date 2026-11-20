#pragma once

namespace Data
{
// A model has at most this many meshes: MAX_MESH of the model drawing (Render/Models/ZzzBMD.h; LookDrawing.cpp checks
// that the two agree). The data that names a mesh takes the numbers below it.
inline constexpr int MaxModelMeshes = 50;

// A model has at most this many bones: MAX_BONES of the model drawing, the size of every bone buffer a pose fills
// (LookDrawing.cpp checks that the two agree). The data that names a bone takes the numbers below it.
inline constexpr int MaxModelBones = 200;
} // namespace Data
