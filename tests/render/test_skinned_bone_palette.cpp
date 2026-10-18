#include "doctest.h"

#include "Render/Renderer/SkinnedBonePalette.h"

#include <array>
#include <cstdint>

namespace
{
struct TestVertex
{
    std::int32_t positionBoneIndex;
    std::int32_t normalBoneIndex;
};
} // namespace

TEST_CASE("a mesh indexes one past its highest position or normal bone [render][skinning]")
{
    const std::array<TestVertex, 3> vertices{{{0, 0}, {4, 1}, {2, 7}}};
    CHECK(Render::BonesIndexed(std::span<const TestVertex>(vertices)) == 8u);

    const std::array<TestVertex, 1> single{{{0, 0}}};
    CHECK(Render::BonesIndexed(std::span<const TestVertex>(single)) == 1u);
}

TEST_CASE("a mesh with no bone indices counts as unknown [render][skinning]")
{
    CHECK(Render::BonesIndexed(std::span<const TestVertex>()) == 0u);
    const std::array<TestVertex, 1> unbound{{{-1, -1}}};
    CHECK(Render::BonesIndexed(std::span<const TestVertex>(unbound)) == 0u);
}

TEST_CASE("a draw uploads the bones it indexes, all of them when unknown [render][skinning]")
{
    CHECK(Render::PaletteBonesToUpload(200u, 3u) == 3u);
    CHECK(Render::PaletteBonesToUpload(200u, 0u) == 200u);
    // Never more than the palette holds, whatever the mesh claims.
    CHECK(Render::PaletteBonesToUpload(200u, 250u) == 200u);
}
