#pragma once

namespace Data
{
// A model has at most this many meshes: MAX_MESH of the model drawing (Render/Models/ZzzBMD.h; LookDrawing.cpp checks
// that the two agree). The data that names a mesh takes the numbers below it.
inline constexpr int MaxModelMeshes = 50;
} // namespace Data
