#include <doctest.h>

#include "Render/Models/GpuSkinningPath.h"

TEST_CASE("only ineligible meshes select CPU skinning [render][gpu_skinning_path]")
{
    CHECK(Render::Models::ResolveGpuSkinningPath(false, false) ==
          Render::Models::GpuSkinningPath::CpuIneligible);
    CHECK(Render::Models::ResolveGpuSkinningPath(true, true) ==
          Render::Models::GpuSkinningPath::GpuSubmitted);
    CHECK(Render::Models::ResolveGpuSkinningPath(true, false) == Render::Models::GpuSkinningPath::GpuFailed);
}
