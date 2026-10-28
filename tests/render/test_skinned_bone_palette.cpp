#include "doctest.h"

#include "Render/Models/Gltf/SkinnedModel.h"
#include "Render/Models/SkinnedVertexPacking.h"
#include "Render/Renderer/SkinnedBonePalette.h"

#include <array>
#include <cstdint>

namespace
{
using Vertex = mu::SkinnedVertex3D;

Vertex BmdVertex(std::int32_t positionBone, std::int32_t normalBone)
{
    constexpr float Position[3]{};
    constexpr float Normal[3]{0.f, 0.f, 1.f};
    return Render::Models::PackBmdSkinnedVertex(Position, positionBone, Normal, normalBone, 0.f, 0.f);
}

Vertex WeightedVertex(short secondaryJoint)
{
    Render::Models::Gltf::SkinnedVertex vertex;
    vertex.count = 2;
    vertex.joints = {0, secondaryJoint, 0, 0};
    vertex.weights = {0.8f, 0.2f, 0.f, 0.f};
    return Render::Models::PackWeightedSkinnedVertex(vertex, 0.f, 0.f);
}
} // namespace

TEST_CASE("a mesh indexes one past its highest position or normal bone [render][skinning]")
{
    const std::array<Vertex, 3> vertices{BmdVertex(0, 0), BmdVertex(4, 1), BmdVertex(2, 7)};
    CHECK(Render::BonesIndexed(std::span<const Vertex>(vertices)) == 8u);

    const std::array<Vertex, 1> single{BmdVertex(0, 0)};
    CHECK(Render::BonesIndexed(std::span<const Vertex>(single)) == 1u);
}

TEST_CASE("a packed weighted mesh retains its secondary-only bone [render][skinning]")
{
    const std::array<Vertex, 1> vertices{WeightedVertex(1)};
    REQUIRE(vertices[0].positionBoneIndex == 0);
    REQUIRE(vertices[0].normalBoneIndex == 0);
    REQUIRE(vertices[0].weights[0] == 204u);
    REQUIRE(vertices[0].weights[1] == 51u);
    const std::uint32_t bonesIndexed = Render::BonesIndexed(std::span<const Vertex>(vertices));
    CHECK(bonesIndexed == 2u);
    CHECK(Render::PaletteBonesToUpload(200u, bonesIndexed) == 2u);
}

TEST_CASE("a packed weighted mesh retains a sparse high secondary joint [render][skinning]")
{
    constexpr short SecondaryJoint = 173;
    const std::array<Vertex, 1> vertices{WeightedVertex(SecondaryJoint)};
    REQUIRE(vertices[0].positionBoneIndex == 0);
    REQUIRE(vertices[0].joints[1] == SecondaryJoint);
    const std::uint32_t bonesIndexed = Render::BonesIndexed(std::span<const Vertex>(vertices));
    CHECK(bonesIndexed == static_cast<std::uint32_t>(SecondaryJoint) + 1u);
    CHECK(Render::PaletteBonesToUpload(200u, bonesIndexed) == bonesIndexed);
    CHECK(Render::PaletteBonesToUpload(100u, bonesIndexed) == 100u);
}

TEST_CASE("a packed weighted mesh counts all four positive-weight lanes [render][skinning]")
{
    Render::Models::Gltf::SkinnedVertex vertex;
    vertex.count = 4;
    vertex.joints = {0, 1, 2, 127};
    vertex.weights = {0.6f, 0.2f, 0.15f, 0.05f};
    const std::array<Vertex, 1> vertices{Render::Models::PackWeightedSkinnedVertex(vertex, 0.f, 0.f)};
    REQUIRE(vertices[0].weights[3] > 0u);
    CHECK(Render::BonesIndexed(std::span<const Vertex>(vertices)) == 128u);
}

TEST_CASE("a mesh ignores inactive joint slots in weighted and BMD vertices [render][skinning]")
{
    Vertex weighted = WeightedVertex(1);
    weighted.joints[2] = 255u;
    weighted.joints[3] = 254u;
    REQUIRE(weighted.weights[2] == 0u);
    REQUIRE(weighted.weights[3] == 0u);
    const std::array<Vertex, 1> weightedVertices{weighted};
    CHECK(Render::BonesIndexed(std::span<const Vertex>(weightedVertices)) == 2u);

    Vertex bmd = BmdVertex(4, 7);
    bmd.joints[0] = 255u;
    bmd.joints[1] = 254u;
    REQUIRE(bmd.weights[0] == 0u);
    const std::array<Vertex, 2> mixedVertices{weighted, bmd};
    CHECK(Render::BonesIndexed(std::span<const Vertex>(mixedVertices)) == 8u);
}

TEST_CASE("a mesh with no bone indices counts as unknown [render][skinning]")
{
    CHECK(Render::BonesIndexed(std::span<const Vertex>()) == 0u);
    const std::array<Vertex, 1> unbound{BmdVertex(-1, -1)};
    CHECK(Render::BonesIndexed(std::span<const Vertex>(unbound)) == 0u);
}

TEST_CASE("a draw uploads the bones it indexes, all of them when unknown [render][skinning]")
{
    CHECK(Render::PaletteBonesToUpload(200u, 3u) == 3u);
    CHECK(Render::PaletteBonesToUpload(200u, 0u) == 200u);
    // Never more than the palette holds, whatever the mesh claims.
    CHECK(Render::PaletteBonesToUpload(200u, 250u) == 200u);
    CHECK(Render::PaletteBonesToUpload(0u, 3u) == 0u);
    CHECK(Render::PaletteBonesToUpload(0u, 0u) == 0u);
}
