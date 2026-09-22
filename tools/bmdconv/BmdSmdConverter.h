#pragma once

#include <filesystem>
#include <string>
#include <vector>

class BMD;

namespace Tools::BmdConv
{
struct AnimationInput
{
    std::filesystem::path file;
    bool lockPositions = false;
};

struct ImportRequest
{
    std::filesystem::path referenceSmd;
    std::vector<AnimationInput> animations; // empty: the reference skeleton becomes the only action
    bool flipWinding = false;
    std::string modelName; // empty: derived from the reference file name
};

struct ActionManifestEntry
{
    int index = 0;
    std::filesystem::path file;
    int keys = 0;
    bool lockPositions = false;
};

struct WorldVertex
{
    float position[3] = {};
    float normal[3] = {};
    float u = 0.f;
    float v = 0.f;
    int bone = 0;
};

// One triangle in model space, i.e. after the bind pose (action 0, frame 0) was applied.
struct WorldTriangle
{
    int mesh = 0;
    std::string texture;
    WorldVertex corners[3];
};

struct ConversionResult
{
    bool ok = false;
    std::string error;
    std::vector<std::string> warnings;
    std::vector<std::filesystem::path> outputs;
};

// Reads a .bmd with the engine's own loader (BMD::Open2). Supports the plain (0xA) and the
// encrypted (0xC) versions the client reads.
bool LoadBmd(const std::filesystem::path& file, BMD& model, std::string& error);

// Human readable summary (name, meshes, bones, actions, bounds) for the `info` command.
std::string DescribeModel(const BMD& model, const std::string& label);

// Expands the model's bone-local vertices into model space using action 0, frame 0.
void CollectWorldTriangles(const BMD& model, std::vector<WorldTriangle>& triangles);

// Writes <stem>.smd (reference mesh + skeleton), one <stem>_aNN.smd per action and a
// <stem>.actions.txt manifest that records the action order and lock flags.
ConversionResult ExportBmdToSmd(const BMD& model, const std::filesystem::path& outputDirectory,
                                const std::string& stem);

// Reads a manifest written by ExportBmdToSmd; file paths are resolved next to the manifest.
bool ReadActionManifest(const std::filesystem::path& manifest, std::vector<ActionManifestEntry>& entries,
                        std::string& error);

// Validates the SMD files, runs them through the engine's SMD parser and writes an
// encrypted BMD (version 0xC) with BMD::Save2. The result is re-opened to prove it loads.
ConversionResult ImportSmdToBmd(const ImportRequest& request, const std::filesystem::path& outputBmd);

struct CompareReport
{
    int meshesA = 0;
    int meshesB = 0;
    int bonesA = 0;
    int bonesB = 0;
    int actionsA = 0;
    int actionsB = 0;
    int trianglesA = 0;
    int trianglesB = 0;
    int unmatchedTriangles = 0;    // triangles of A without a counterpart in B (within tolerance)
    float maxCornerDistance = 0.f; // largest position difference among matched corners
    float maxBoneDistance = 0.f;   // largest world-space bone position difference over shared actions/keys
    int differingBoneNames = 0;
    std::vector<std::string> notes;

    bool equivalent() const
    {
        return bonesA == bonesB && actionsA == actionsB && trianglesA == trianglesB && unmatchedTriangles == 0 &&
               differingBoneNames == 0;
    }
};

// Compares two models by their model-space triangles (bind pose) and by the world-space bone
// positions of every shared action and keyframe. Mesh order and mesh count are allowed to
// differ, because the engine's SMD converter merges meshes that share a texture.
CompareReport CompareModels(const BMD& a, const BMD& b, float positionTolerance);
std::string DescribeComparison(const CompareReport& report);
} // namespace Tools::BmdConv
