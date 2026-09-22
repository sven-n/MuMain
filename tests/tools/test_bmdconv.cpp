#include "doctest.h"

#include "stdafx.h"

#include "BmdSmdConverter.h"
#include "Render/Models/SMD.h"
#include "Render/Models/ZzzBMD.h"
#include "SmdValidator.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace
{
namespace fs = std::filesystem;
using namespace Tools::BmdConv;

constexpr float kPositionTolerance = 5e-3f;
constexpr float kNormalTolerance = 5e-3f;
constexpr float kUvTolerance = 1e-4f;
constexpr float kKeyTolerance = 1e-4f;

fs::path DataPath(const char* relative)
{
    return fs::path(MU_TEST_DATA_DIR) / relative;
}

fs::path FreshScratchDirectory(const std::string& name)
{
    const fs::path directory = fs::path(MU_TEST_SCRATCH_DIR) / name;
    fs::remove_all(directory);
    fs::create_directories(directory);
    return directory;
}

bool Near(float a, float b, float tolerance)
{
    return std::fabs(a - b) <= tolerance;
}

bool SameVertex(const WorldVertex& a, const WorldVertex& b)
{
    for (int axis = 0; axis < 3; ++axis)
    {
        if (!Near(a.position[axis], b.position[axis], kPositionTolerance) ||
            !Near(a.normal[axis], b.normal[axis], kNormalTolerance))
        {
            return false;
        }
    }
    return Near(a.u, b.u, kUvTolerance) && Near(a.v, b.v, kUvTolerance) && a.bone == b.bone;
}

bool SameTriangle(const WorldTriangle& a, const WorldTriangle& b)
{
    return a.texture == b.texture && SameVertex(a.corners[0], b.corners[0]) && SameVertex(a.corners[1], b.corners[1]) &&
           SameVertex(a.corners[2], b.corners[2]);
}

// Meshes that share a texture are merged by the engine's SMD converter, so triangle order is
// not guaranteed; compare the two triangle sets instead.
int CountUnmatchedTriangles(const std::vector<WorldTriangle>& before, const std::vector<WorldTriangle>& after)
{
    std::vector<bool> used(after.size(), false);
    int unmatched = 0;
    for (const WorldTriangle& triangle : before)
    {
        bool found = false;
        for (size_t index = 0; index < after.size() && !found; ++index)
        {
            if (!used[index] && SameTriangle(triangle, after[index]))
            {
                used[index] = true;
                found = true;
            }
        }
        if (!found)
        {
            ++unmatched;
        }
    }
    return unmatched;
}

void CheckSkeletonsMatch(const BMD& original, const BMD& copy)
{
    REQUIRE(copy.NumBones == original.NumBones);
    REQUIRE(copy.NumActions == original.NumActions);
    for (int bone = 0; bone < original.NumBones; ++bone)
    {
        const Bone_t& expected = original.Bones[bone];
        const Bone_t& actual = copy.Bones[bone];
        CHECK(actual.Dummy == expected.Dummy);
        if (expected.Dummy || actual.Dummy)
        {
            continue;
        }
        CHECK(std::string(actual.Name, strnlen(actual.Name, 32)) ==
              std::string(expected.Name, strnlen(expected.Name, 32)));
        CHECK(actual.Parent == expected.Parent);
    }
}

void CheckKeyframesMatch(const BMD& original, const BMD& copy)
{
    for (int action = 0; action < original.NumActions; ++action)
    {
        CHECK(copy.Actions[action].NumAnimationKeys == original.Actions[action].NumAnimationKeys);
        CHECK(copy.Actions[action].LockPositions == original.Actions[action].LockPositions);
        const int keys = std::min(copy.Actions[action].NumAnimationKeys, original.Actions[action].NumAnimationKeys);
        int mismatches = 0;
        for (int bone = 0; bone < original.NumBones; ++bone)
        {
            if (original.Bones[bone].Dummy || copy.Bones[bone].Dummy)
            {
                continue;
            }
            const BoneMatrix_t& expected = original.Bones[bone].BoneMatrixes[action];
            const BoneMatrix_t& actual = copy.Bones[bone].BoneMatrixes[action];
            for (int key = 0; key < keys; ++key)
            {
                for (int axis = 0; axis < 3; ++axis)
                {
                    if (!Near(expected.Position[key][axis], actual.Position[key][axis], kKeyTolerance) ||
                        !Near(expected.Rotation[key][axis], actual.Rotation[key][axis], kKeyTolerance))
                    {
                        ++mismatches;
                    }
                }
            }
        }
        CHECK_MESSAGE(mismatches == 0, "action " << action << " has " << mismatches << " differing keyframe values");
    }
}

void RequireRoundTrip(const char* relativeModel)
{
    BMD original;
    std::string error;
    REQUIRE_MESSAGE(LoadBmd(DataPath(relativeModel), original, error), error);

    const std::string stem = fs::path(relativeModel).stem().string();
    const fs::path scratch = FreshScratchDirectory(stem);
    const auto exported = ExportBmdToSmd(original, scratch, stem);
    REQUIRE_MESSAGE(exported.ok, exported.error);
    CHECK(exported.outputs.size() == static_cast<size_t>(original.NumActions) + 2);

    std::vector<ActionManifestEntry> actions;
    REQUIRE_MESSAGE(ReadActionManifest(scratch / (stem + ".actions.txt"), actions, error), error);
    REQUIRE(actions.size() == static_cast<size_t>(original.NumActions));

    ImportRequest request;
    request.referenceSmd = scratch / (stem + ".smd");
    for (const auto& action : actions)
    {
        request.animations.push_back({action.file, action.lockPositions});
    }
    const fs::path rebuilt = scratch / (stem + ".roundtrip.bmd");
    const auto imported = ImportSmdToBmd(request, rebuilt);
    REQUIRE_MESSAGE(imported.ok, imported.error);

    BMD copy;
    REQUIRE_MESSAGE(LoadBmd(rebuilt, copy, error), error);
    CHECK(copy.Version == 0xC);
    CheckSkeletonsMatch(original, copy);
    CheckKeyframesMatch(original, copy);

    std::vector<WorldTriangle> before;
    std::vector<WorldTriangle> after;
    CollectWorldTriangles(original, before);
    CollectWorldTriangles(copy, after);
    CHECK(before.size() > 0);
    CHECK(after.size() == before.size());
    CHECK(CountUnmatchedTriangles(before, after) == 0);

    const CompareReport report = CompareModels(original, copy, kPositionTolerance);
    CHECK_MESSAGE(report.equivalent(), DescribeComparison(report));
    CHECK(report.maxBoneDistance < kPositionTolerance);
}

void WriteText(const fs::path& file, const std::string& text)
{
    std::ofstream out(file, std::ios::binary | std::ios::trunc);
    out << text;
}

const char* kMinimalReference = "version 1\n"
                                "nodes\n"
                                "0 \"root\" -1\n"
                                "end\n"
                                "skeleton\n"
                                "time 0\n"
                                "0 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000\n"
                                "end\n"
                                "triangles\n"
                                "stone01.jpg\n"
                                "0 0.000000 0.000000 0.000000 0.000000 0.000000 1.000000 0.000000 0.000000\n"
                                "0 10.000000 0.000000 0.000000 0.000000 0.000000 1.000000 1.000000 0.000000\n"
                                "0 0.000000 10.000000 0.000000 0.000000 0.000000 1.000000 0.000000 1.000000\n"
                                "end\n";

bool AnyMessageContains(const std::vector<std::string>& messages, const std::string& needle)
{
    for (const auto& message : messages)
    {
        if (message.find(needle) != std::string::npos)
        {
            return true;
        }
    }
    return false;
}
} // namespace

TEST_CASE("bmdconv round-trips a static item model through SMD [tools][bmdconv]")
{
    RequireRoundTrip("Item/Sword01.bmd");
}

TEST_CASE("bmdconv round-trips an animated monster model through SMD [tools][bmdconv]")
{
    RequireRoundTrip("Monster/Monster01.bmd");
}

TEST_CASE("bmdconv converts a minimal hand-written SMD into a loadable BMD [tools][bmdconv]")
{
    const fs::path scratch = FreshScratchDirectory("minimal");
    const fs::path reference = scratch / "stone.smd";
    WriteText(reference, kMinimalReference);

    ImportRequest request;
    request.referenceSmd = reference;
    const fs::path output = scratch / "stone.bmd";
    const auto result = ImportSmdToBmd(request, output);
    REQUIRE_MESSAGE(result.ok, result.error);

    BMD model;
    std::string error;
    REQUIRE_MESSAGE(LoadBmd(output, model, error), error);
    CHECK(model.NumMeshs == 1);
    CHECK(model.NumBones == 1);
    CHECK(model.NumActions == 1);
    CHECK(model.Meshs[0].NumTriangles == 1);
    CHECK(std::string(model.Textures[0].FileName, strnlen(model.Textures[0].FileName, 32)) == "stone01.jpg");
    CHECK(std::string(model.Name, strnlen(model.Name, 32)) == "stone.smd");
}

TEST_CASE("bmdconv converts a skeleton-only SMD (no triangles) into a loadable BMD [tools][bmdconv]")
{
    const fs::path scratch = FreshScratchDirectory("skeleton-only");
    const fs::path reference = scratch / "rig.smd";
    WriteText(reference, "version 1\nnodes\n0 \"root\" -1\n1 \"child\" 0\nend\nskeleton\ntime 0\n"
                         "0 0.000000 0.000000 0.000000 0.000000 0.000000 0.000000\n"
                         "1 0.000000 0.000000 10.000000 0.000000 0.000000 0.000000\nend\ntriangles\nend\n");

    ImportRequest request;
    request.referenceSmd = reference;
    const fs::path output = scratch / "rig.bmd";
    const auto result = ImportSmdToBmd(request, output);
    REQUIRE_MESSAGE(result.ok, result.error);

    BMD model;
    std::string error;
    REQUIRE_MESSAGE(LoadBmd(output, model, error), error);
    CHECK(model.NumMeshs == 0);
    CHECK(model.NumBones == 2);
    CHECK(model.NumActions == 1);

    const auto exported = ExportBmdToSmd(model, scratch / "again", "rig");
    CHECK_MESSAGE(exported.ok, exported.error);
}

TEST_CASE("SMD validator rejects what the engine parser cannot read [tools][bmdconv]")
{
    const fs::path scratch = FreshScratchDirectory("validator");

    SUBCASE("a minimal reference passes")
    {
        WriteText(scratch / "ok.smd", kMinimalReference);
        const auto result = ValidateSmd(scratch / "ok.smd", SmdKind::Reference);
        const std::string firstError = result.errors.empty() ? std::string() : result.errors.front();
        CHECK_MESSAGE(result.ok(), firstError);
        CHECK(result.stats.nodeCount == 1);
        CHECK(result.stats.triangleCount == 1);
        CHECK(result.stats.materials.size() == 1);
    }

    SUBCASE("materials must be engine tokens")
    {
        std::string text = kMinimalReference;
        text.replace(text.find("stone01.jpg"), std::string("stone01.jpg").size(), "01-stone.jpg");
        WriteText(scratch / "material.smd", text);
        const auto result = ValidateSmd(scratch / "material.smd", SmdKind::Reference);
        CHECK_FALSE(result.ok());
        CHECK(AnyMessageContains(result.errors, "must start with a letter"));
    }

    SUBCASE("scientific notation is rejected")
    {
        std::string text = kMinimalReference;
        text.replace(text.find("10.000000 0.000000 0.000000 0.000000 0.000000 1.000000 1.000000"), 9, "1e+01    ");
        WriteText(scratch / "exponent.smd", text);
        const auto result = ValidateSmd(scratch / "exponent.smd", SmdKind::Reference);
        CHECK_FALSE(result.ok());
        CHECK(AnyMessageContains(result.errors, "plain decimal number"));
    }

    SUBCASE("animations are capped at the engine frame table size")
    {
        std::string text = "version 1\nnodes\n0 \"root\" -1\nend\nskeleton\n";
        for (int frame = 0; frame < TIME_MAX + 1; ++frame)
        {
            text += "time " + std::to_string(frame) + "\n0 0.0 0.0 0.0 0.0 0.0 0.0\n";
        }
        text += "end\n";
        WriteText(scratch / "long.smd", text);
        const auto result = ValidateSmd(scratch / "long.smd", SmdKind::Animation);
        CHECK_FALSE(result.ok());
        CHECK(AnyMessageContains(result.errors, "at most " + std::to_string(TIME_MAX)));
    }

    SUBCASE("bone names are limited to 31 characters and Du-names warn")
    {
        std::string text = kMinimalReference;
        text.replace(text.find("\"root\""), 6, "\"Dummy_with_a_very_long_name_that_overflows\"");
        WriteText(scratch / "names.smd", text);
        const auto result = ValidateSmd(scratch / "names.smd", SmdKind::Reference);
        CHECK_FALSE(result.ok());
        CHECK(AnyMessageContains(result.errors, "31 characters"));
        CHECK(AnyMessageContains(result.warnings, "dummy"));
    }

    SUBCASE("every frame must list every bone")
    {
        const std::string text = "version 1\nnodes\n0 \"root\" -1\n1 \"child\" 0\nend\nskeleton\ntime 0\n"
                                 "0 0.0 0.0 0.0 0.0 0.0 0.0\nend\n";
        WriteText(scratch / "sparse.smd", text);
        const auto result = ValidateSmd(scratch / "sparse.smd", SmdKind::Animation);
        CHECK_FALSE(result.ok());
        CHECK(AnyMessageContains(result.errors, "every bone in every frame"));
    }
}
