#include "stdafx.h"

#include "BmdSmdConverter.h"

#include "SmdValidator.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Models/SMD.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <functional>
#include <iomanip>
#include <memory>
#include <sstream>

namespace Tools::BmdConv
{
namespace
{
constexpr size_t kNameCapacity = 32;   // char[32] fields in Bone_t, Texture_t and BMD::Name
constexpr size_t kMaxPathLength = 259; // wchar_t[260] path buffers in BMD::Open2 / BMD::Save2
constexpr int kOutputPrecision = 6;    // Blender Source Tools also writes 6 decimals
constexpr int kActionDigits = 2;       // <stem>_a00.smd
constexpr float kRadiansToDegrees = 180.f / Q_PI;
constexpr int kQuadCorners = 4;
constexpr const char* kManifestSuffix = ".actions.txt";

std::string FixedString(const char* text, size_t capacity)
{
    return std::string(text, strnlen(text, capacity));
}

std::string ActionFileName(const std::string& stem, int action)
{
    std::ostringstream name;
    name << stem << "_a" << std::setw(kActionDigits) << std::setfill('0') << action << ".smd";
    return name.str();
}

bool HasKeys(const Bone_t& bone, int action)
{
    return !bone.Dummy && bone.BoneMatrixes != nullptr && bone.BoneMatrixes[action].Position != nullptr &&
           bone.BoneMatrixes[action].Rotation != nullptr;
}

struct BoneFrame
{
    float rotation[3][4] = {};
    vec3_t origin = {};
};

void SetIdentity(BoneFrame& frame)
{
    std::memset(frame.rotation, 0, sizeof(frame.rotation));
    frame.rotation[0][0] = 1.f;
    frame.rotation[1][1] = 1.f;
    frame.rotation[2][2] = 1.f;
    frame.origin[0] = frame.origin[1] = frame.origin[2] = 0.f;
}

// Mirrors the forward pass of FixupSMD (SMD2BMD.cpp): rotations are stored in radians and
// composed parent-first. The bind pose used for export is action 0, frame 0.
void ComputePose(const BMD& model, int action, int key, std::vector<BoneFrame>& frames)
{
    frames.assign(static_cast<size_t>(model.NumBones), BoneFrame{});
    std::vector<bool> resolved(static_cast<size_t>(model.NumBones), false);
    std::function<void(int)> resolve = [&](int index)
    {
        if (resolved[index])
        {
            return;
        }
        resolved[index] = true; // set first so a malformed parent cycle cannot recurse forever
        const Bone_t& bone = model.Bones[index];
        BoneFrame& frame = frames[index];
        if (action < 0 || action >= model.NumActions || !HasKeys(bone, action) ||
            key >= model.Actions[action].NumAnimationKeys)
        {
            SetIdentity(frame);
            return;
        }
        vec3_t degrees;
        for (int axis = 0; axis < 3; ++axis)
        {
            degrees[axis] = bone.BoneMatrixes[action].Rotation[key][axis] * kRadiansToDegrees;
        }
        float local[3][4] = {};
        AngleMatrix(degrees, local);
        const float* position = bone.BoneMatrixes[action].Position[key];
        const int parent = bone.Parent;
        if (parent < 0 || parent >= model.NumBones || parent == index)
        {
            std::memcpy(frame.rotation, local, sizeof(local));
            VectorCopy(position, frame.origin);
            return;
        }
        resolve(parent);
        const BoneFrame& parentFrame = frames[parent];
        R_ConcatTransforms(parentFrame.rotation, local, frame.rotation);
        vec3_t rotated;
        VectorTransform(position, parentFrame.rotation, rotated);
        VectorAdd(rotated, parentFrame.origin, frame.origin);
    };
    for (int index = 0; index < model.NumBones; ++index)
    {
        resolve(index);
    }
}

std::string TextureNameForMesh(const BMD& model, int meshIndex)
{
    const Mesh_t& mesh = model.Meshs[meshIndex];
    const int textureIndex = (mesh.Texture >= 0 && mesh.Texture < model.NumMeshs) ? mesh.Texture : meshIndex;
    return FixedString(model.Textures[textureIndex].FileName, kNameCapacity);
}

// The engine tokenizer reads a material name as [A-Za-z][A-Za-z0-9._]*, so anything else
// has to be rewritten before it can round-trip through an SMD file.
std::string SanitizeMaterialName(const std::string& name)
{
    std::string sanitized;
    for (char ch : name)
    {
        const bool allowed = std::isalnum(static_cast<unsigned char>(ch)) || ch == '.' || ch == '_';
        sanitized += allowed ? ch : '_';
    }
    if (sanitized.empty())
    {
        sanitized = "material";
    }
    if (!std::isalpha(static_cast<unsigned char>(sanitized[0])))
    {
        sanitized.insert(sanitized.begin(), 'm');
    }
    if (sanitized.size() >= kNameCapacity)
    {
        sanitized.resize(kNameCapacity - 1);
    }
    return sanitized;
}

std::string BoneNameForExport(const BMD& model, int index)
{
    const Bone_t& bone = model.Bones[index];
    if (bone.Dummy)
    {
        std::ostringstream name;
        name << "Dummy" << std::setw(2) << std::setfill('0') << index;
        return name.str();
    }
    std::string name = FixedString(bone.Name, kNameCapacity);
    name.erase(std::remove(name.begin(), name.end(), '"'), name.end());
    if (name.empty())
    {
        name = "Bone" + std::to_string(index);
    }
    return name;
}

bool CornerIndicesValid(const Mesh_t& mesh, const Triangle_t& triangle, int corner)
{
    return triangle.VertexIndex[corner] >= 0 && triangle.VertexIndex[corner] < mesh.NumVertices &&
           triangle.NormalIndex[corner] >= 0 && triangle.NormalIndex[corner] < mesh.NumNormals &&
           triangle.TexCoordIndex[corner] >= 0 && triangle.TexCoordIndex[corner] < mesh.NumTexCoords;
}

WorldVertex ExpandCorner(const BMD& model, const Mesh_t& mesh, const Triangle_t& triangle, int corner,
                         const std::vector<BoneFrame>& frames)
{
    WorldVertex vertex;
    const Vertex_t& source = mesh.Vertices[triangle.VertexIndex[corner]];
    const Normal_t& normal = mesh.Normals[triangle.NormalIndex[corner]];
    const TexCoord_t& uv = mesh.TexCoords[triangle.TexCoordIndex[corner]];
    vertex.bone = (source.Node >= 0 && source.Node < model.NumBones) ? source.Node : 0;
    const BoneFrame& frame = frames[static_cast<size_t>(vertex.bone)];
    vec3_t rotated;
    VectorTransform(source.Position, frame.rotation, rotated);
    VectorAdd(rotated, frame.origin, vertex.position);
    VectorRotate(normal.Normal, frame.rotation, vertex.normal);
    vertex.u = uv.TexCoordU;
    vertex.v = 1.f - uv.TexCoordV; // the SMD parser stores 1 - v (ParseTriangles)
    return vertex;
}

void AppendTriangle(std::vector<WorldTriangle>& triangles, int meshIndex, const std::string& texture,
                    const WorldVertex& a, const WorldVertex& b, const WorldVertex& c)
{
    WorldTriangle triangle;
    triangle.mesh = meshIndex;
    triangle.texture = texture;
    triangle.corners[0] = a;
    triangle.corners[1] = b;
    triangle.corners[2] = c;
    triangles.push_back(std::move(triangle));
}

void WriteFloat(std::ostream& out, float value)
{
    out << std::fixed << std::setprecision(kOutputPrecision) << value;
}

void WriteVector(std::ostream& out, const float* values, int count)
{
    for (int index = 0; index < count; ++index)
    {
        out << ' ';
        WriteFloat(out, values[index]);
    }
}

void WriteNodes(std::ostream& out, const BMD& model, const std::vector<std::string>& boneNames)
{
    out << "nodes\n";
    for (int index = 0; index < model.NumBones; ++index)
    {
        const Bone_t& bone = model.Bones[index];
        const int parent = bone.Dummy ? -1 : bone.Parent;
        out << index << " \"" << boneNames[static_cast<size_t>(index)] << "\" " << parent << '\n';
    }
    out << "end\n";
}

void WriteFrame(std::ostream& out, const BMD& model, int action, int key)
{
    static const vec3_t zero = {0.f, 0.f, 0.f};
    for (int index = 0; index < model.NumBones; ++index)
    {
        const Bone_t& bone = model.Bones[index];
        const bool hasKey =
            action < model.NumActions && HasKeys(bone, action) && key < model.Actions[action].NumAnimationKeys;
        const float* position = hasKey ? bone.BoneMatrixes[action].Position[key] : zero;
        const float* rotation = hasKey ? bone.BoneMatrixes[action].Rotation[key] : zero;
        out << index;
        WriteVector(out, position, 3);
        WriteVector(out, rotation, 3);
        out << '\n';
    }
}

void WriteReferenceSkeleton(std::ostream& out, const BMD& model)
{
    out << "skeleton\ntime 0\n";
    WriteFrame(out, model, 0, 0);
    out << "end\n";
}

void WriteTriangles(std::ostream& out, const std::vector<WorldTriangle>& triangles,
                    const std::vector<std::string>& materialNames)
{
    out << "triangles\n";
    for (const WorldTriangle& triangle : triangles)
    {
        out << materialNames[static_cast<size_t>(triangle.mesh)] << '\n';
        for (const WorldVertex& corner : triangle.corners)
        {
            out << corner.bone;
            WriteVector(out, corner.position, 3);
            WriteVector(out, corner.normal, 3);
            out << ' ';
            WriteFloat(out, corner.u);
            out << ' ';
            WriteFloat(out, corner.v);
            out << '\n';
        }
    }
    out << "end\n";
}

bool OpenOutput(std::ofstream& out, const std::filesystem::path& file, std::string& error)
{
    out.open(file, std::ios::binary | std::ios::trunc);
    if (!out)
    {
        error = "cannot write " + file.string();
        return false;
    }
    return true;
}

std::vector<std::string> MaterialNamesForExport(const BMD& model, std::vector<std::string>& warnings)
{
    std::vector<std::string> names;
    for (int mesh = 0; mesh < model.NumMeshs; ++mesh)
    {
        const std::string original = TextureNameForMesh(model, mesh);
        const std::string sanitized = SanitizeMaterialName(original);
        if (sanitized != original)
        {
            warnings.push_back("mesh " + std::to_string(mesh) + ": texture name '" + original +
                               "' cannot be read back by the engine SMD parser; written as '" + sanitized + "'");
        }
        names.push_back(sanitized);
    }
    return names;
}

void WriteManifest(std::ostream& out, const BMD& model, const std::string& stem,
                   const std::vector<std::string>& materialNames)
{
    out << "# bmdconv action manifest for " << stem << ".bmd. The order is the action index the game uses.\n";
    out << "# Re-import: bmdconv smd2bmd " << stem << ".smd <output.bmd> --manifest " << stem << kManifestSuffix
        << "\n";
    out << "model " << FixedString(model.Name, kNameCapacity) << '\n';
    for (int action = 0; action < model.NumActions; ++action)
    {
        const Action_t& entry = model.Actions[action];
        out << "action " << action << " file=" << ActionFileName(stem, action) << " keys=" << entry.NumAnimationKeys
            << " lock=" << (entry.LockPositions ? 1 : 0) << '\n';
    }
    for (int mesh = 0; mesh < model.NumMeshs; ++mesh)
    {
        out << "texture " << mesh << ' ' << materialNames[static_cast<size_t>(mesh)]
            << " original=" << TextureNameForMesh(model, mesh) << '\n';
    }
}

bool SplitPath(const std::filesystem::path& file, std::wstring& directory, std::wstring& name, std::string& error)
{
    directory = file.parent_path().wstring();
    if (!directory.empty())
    {
        directory += L'/';
    }
    name = file.filename().wstring();
    if (directory.size() + name.size() > kMaxPathLength)
    {
        error = "path is longer than " + std::to_string(kMaxPathLength) +
                " characters, which the engine's file routines cannot hold: " + file.string();
        return false;
    }
    return true;
}

void AssignModelName(BMD& model, const std::string& requested, const std::filesystem::path& reference)
{
    std::string name = requested.empty() ? reference.stem().string() + ".smd" : requested;
    if (name.size() >= kNameCapacity)
    {
        name.resize(kNameCapacity - 1);
    }
    std::memset(model.Name, 0, sizeof(model.Name));
    std::memcpy(model.Name, name.data(), name.size());
}

bool ValidateInputs(const ImportRequest& request, ConversionResult& result, int& expectedTriangles)
{
    const auto reference = ValidateSmd(request.referenceSmd, SmdKind::Reference);
    expectedTriangles = reference.stats.triangleCount;
    for (const auto& warning : reference.warnings)
    {
        result.warnings.push_back(request.referenceSmd.filename().string() + ": " + warning);
    }
    if (!reference.ok())
    {
        result.error = request.referenceSmd.string() + ": " + reference.errors.front();
        return false;
    }
    for (const AnimationInput& animation : request.animations)
    {
        const auto validation = ValidateSmd(animation.file, SmdKind::Animation);
        for (const auto& warning : validation.warnings)
        {
            result.warnings.push_back(animation.file.filename().string() + ": " + warning);
        }
        if (!validation.ok())
        {
            result.error = animation.file.string() + ": " + validation.errors.front();
            return false;
        }
        if (validation.stats.nodeCount != reference.stats.nodeCount)
        {
            result.error = animation.file.string() + ": skeleton has " + std::to_string(validation.stats.nodeCount) +
                           " bones but the reference has " + std::to_string(reference.stats.nodeCount);
            return false;
        }
    }
    return true;
}

int CountTriangles(const BMD& model)
{
    int triangles = 0;
    for (int mesh = 0; mesh < model.NumMeshs; ++mesh)
    {
        triangles += model.Meshs[mesh].NumTriangles;
    }
    return triangles;
}

bool RunEngineImport(const ImportRequest& request, BMD& scratch, int expectedTriangles, std::string& error)
{
    Models = &scratch;
    std::wstring reference = request.referenceSmd.wstring();
    const int actionCount = request.animations.empty() ? 1 : static_cast<int>(request.animations.size());
    if (!OpenSMDModel(0, reference.data(), actionCount, request.flipWinding))
    {
        error = "the engine SMD parser could not read " + request.referenceSmd.string();
        return false;
    }
    const int parsedTriangles = CountTriangles(scratch);
    if (parsedTriangles != expectedTriangles)
    {
        error = "the engine SMD parser produced " + std::to_string(parsedTriangles) + " triangles but the file lists " +
                std::to_string(expectedTriangles) + " (" + request.referenceSmd.string() + ")";
        return false;
    }
    if (request.animations.empty())
    {
        OpenSMDAnimation(0, reference.data(), false);
        return true;
    }
    for (const AnimationInput& animation : request.animations)
    {
        std::wstring file = animation.file.wstring();
        if (!OpenSMDAnimation(0, file.data(), animation.lockPositions))
        {
            error = "the engine SMD parser could not read " + animation.file.string();
            return false;
        }
    }
    return true;
}

bool ParseManifestField(const std::string& token, const std::string& key, std::string& value)
{
    if (token.compare(0, key.size(), key) != 0)
    {
        return false;
    }
    value = token.substr(key.size());
    return true;
}
} // namespace

bool LoadBmd(const std::filesystem::path& file, BMD& model, std::string& error)
{
    std::wstring directory;
    std::wstring name;
    if (!SplitPath(file, directory, name, error))
    {
        return false;
    }
    if (!std::filesystem::exists(file))
    {
        error = "file not found: " + file.string();
        return false;
    }
    if (!model.Open2(directory.c_str(), name.c_str()))
    {
        error = "not a BMD model the engine can read (bad header, unsupported version or truncated): " + file.string();
        return false;
    }
    return true;
}

void CollectWorldTriangles(const BMD& model, std::vector<WorldTriangle>& triangles)
{
    triangles.clear();
    std::vector<BoneFrame> frames;
    ComputePose(model, 0, 0, frames);
    for (int meshIndex = 0; meshIndex < model.NumMeshs; ++meshIndex)
    {
        const Mesh_t& mesh = model.Meshs[meshIndex];
        const std::string texture = TextureNameForMesh(model, meshIndex);
        for (int index = 0; index < mesh.NumTriangles; ++index)
        {
            const Triangle_t& triangle = mesh.Triangles[index];
            const int corners = triangle.Polygon == kQuadCorners ? kQuadCorners : 3;
            bool valid = true;
            for (int corner = 0; corner < corners; ++corner)
            {
                valid = valid && CornerIndicesValid(mesh, triangle, corner);
            }
            if (!valid)
            {
                continue;
            }
            WorldVertex expanded[kQuadCorners];
            for (int corner = 0; corner < corners; ++corner)
            {
                expanded[corner] = ExpandCorner(model, mesh, triangle, corner, frames);
            }
            AppendTriangle(triangles, meshIndex, texture, expanded[0], expanded[1], expanded[2]);
            if (corners == kQuadCorners)
            {
                AppendTriangle(triangles, meshIndex, texture, expanded[0], expanded[2], expanded[3]);
            }
        }
    }
}

std::string DescribeModel(const BMD& model, const std::string& label)
{
    std::vector<WorldTriangle> triangles;
    CollectWorldTriangles(model, triangles);
    float minimum[3] = {0.f, 0.f, 0.f};
    float maximum[3] = {0.f, 0.f, 0.f};
    bool first = true;
    for (const WorldTriangle& triangle : triangles)
    {
        for (const WorldVertex& corner : triangle.corners)
        {
            for (int axis = 0; axis < 3; ++axis)
            {
                minimum[axis] = first ? corner.position[axis] : std::min(minimum[axis], corner.position[axis]);
                maximum[axis] = first ? corner.position[axis] : std::max(maximum[axis], corner.position[axis]);
            }
            first = false;
        }
    }

    std::ostringstream out;
    out << std::fixed << std::setprecision(2);
    out << "model " << label << '\n';
    out << "  name field: " << FixedString(model.Name, kNameCapacity)
        << "  version: " << static_cast<int>(model.Version) << '\n';
    out << "  meshes: " << model.NumMeshs << "  bones: " << model.NumBones << "  actions: " << model.NumActions
        << "  triangles: " << triangles.size() << '\n';
    out << "  bounds (bind pose): min " << minimum[0] << ' ' << minimum[1] << ' ' << minimum[2] << "  max "
        << maximum[0] << ' ' << maximum[1] << ' ' << maximum[2] << "  size " << (maximum[0] - minimum[0]) << ' '
        << (maximum[1] - minimum[1]) << ' ' << (maximum[2] - minimum[2]) << '\n';
    for (int mesh = 0; mesh < model.NumMeshs; ++mesh)
    {
        const Mesh_t& entry = model.Meshs[mesh];
        out << "  mesh " << mesh << ": triangles=" << entry.NumTriangles << " vertices=" << entry.NumVertices
            << " normals=" << entry.NumNormals << " uvs=" << entry.NumTexCoords
            << " texture=" << TextureNameForMesh(model, mesh);
        if (entry.m_csTScript != nullptr)
        {
            out << " flags:" << (entry.m_csTScript->getBright() ? " bright" : "")
                << (entry.m_csTScript->getHiddenMesh() ? " hidden" : "")
                << (entry.m_csTScript->getStreamMesh() ? " stream" : "")
                << (entry.m_csTScript->getNoneBlendMesh() ? " noneblend" : "");
        }
        out << '\n';
    }
    for (int bone = 0; bone < model.NumBones; ++bone)
    {
        const Bone_t& entry = model.Bones[bone];
        out << "  bone " << bone << ": \"" << BoneNameForExport(model, bone)
            << "\" parent=" << (entry.Dummy ? -1 : entry.Parent) << (entry.Dummy ? " dummy" : "") << '\n';
    }
    for (int action = 0; action < model.NumActions; ++action)
    {
        const Action_t& entry = model.Actions[action];
        out << "  action " << action << ": keys=" << entry.NumAnimationKeys << " lock=" << (entry.LockPositions ? 1 : 0)
            << '\n';
    }
    return out.str();
}

ConversionResult ExportBmdToSmd(const BMD& model, const std::filesystem::path& outputDirectory, const std::string& stem)
{
    ConversionResult result;
    std::error_code ec;
    std::filesystem::create_directories(outputDirectory, ec);
    if (ec)
    {
        result.error = "cannot create " + outputDirectory.string() + ": " + ec.message();
        return result;
    }
    if (model.NumBones <= 0)
    {
        result.error = "model has no bones";
        return result;
    }
    if (model.NumMeshs <= 0)
    {
        result.warnings.push_back("model has no meshes (skeleton and animations only); the triangles block is empty");
    }

    std::vector<std::string> boneNames;
    for (int bone = 0; bone < model.NumBones; ++bone)
    {
        boneNames.push_back(BoneNameForExport(model, bone));
    }
    const std::vector<std::string> materialNames = MaterialNamesForExport(model, result.warnings);
    std::vector<WorldTriangle> triangles;
    CollectWorldTriangles(model, triangles);

    const std::filesystem::path referenceFile = outputDirectory / (stem + ".smd");
    std::ofstream reference;
    if (!OpenOutput(reference, referenceFile, result.error))
    {
        return result;
    }
    reference << "version 1\n";
    WriteNodes(reference, model, boneNames);
    WriteReferenceSkeleton(reference, model);
    WriteTriangles(reference, triangles, materialNames);
    result.outputs.push_back(referenceFile);

    for (int action = 0; action < model.NumActions; ++action)
    {
        const std::filesystem::path actionFile = outputDirectory / ActionFileName(stem, action);
        std::ofstream animation;
        if (!OpenOutput(animation, actionFile, result.error))
        {
            return result;
        }
        animation << "version 1\n";
        WriteNodes(animation, model, boneNames);
        animation << "skeleton\n";
        for (int key = 0; key < model.Actions[action].NumAnimationKeys; ++key)
        {
            animation << "time " << key << '\n';
            WriteFrame(animation, model, action, key);
        }
        animation << "end\n";
        result.outputs.push_back(actionFile);
    }

    const std::filesystem::path manifestFile = outputDirectory / (stem + kManifestSuffix);
    std::ofstream manifest;
    if (!OpenOutput(manifest, manifestFile, result.error))
    {
        return result;
    }
    WriteManifest(manifest, model, stem, materialNames);
    result.outputs.push_back(manifestFile);
    result.ok = true;
    return result;
}

bool ReadActionManifest(const std::filesystem::path& manifest, std::vector<ActionManifestEntry>& entries,
                        std::string& error)
{
    entries.clear();
    std::ifstream input(manifest);
    if (!input)
    {
        error = "cannot open manifest " + manifest.string();
        return false;
    }
    std::string line;
    int lineNumber = 0;
    while (std::getline(input, line))
    {
        ++lineNumber;
        std::istringstream tokens(line);
        std::string keyword;
        tokens >> keyword;
        if (keyword != "action")
        {
            continue;
        }
        ActionManifestEntry entry;
        tokens >> entry.index;
        std::string token;
        std::string value;
        while (tokens >> token)
        {
            if (ParseManifestField(token, "file=", value))
            {
                entry.file = manifest.parent_path() / value;
            }
            else if (ParseManifestField(token, "keys=", value))
            {
                entry.keys = std::atoi(value.c_str());
            }
            else if (ParseManifestField(token, "lock=", value))
            {
                entry.lockPositions = value == "1";
            }
        }
        if (entry.file.empty())
        {
            error = manifest.string() + ": line " + std::to_string(lineNumber) + " has no file= field";
            return false;
        }
        entries.push_back(entry);
    }
    return true;
}

ConversionResult ImportSmdToBmd(const ImportRequest& request, const std::filesystem::path& outputBmd)
{
    ConversionResult result;
    int expectedTriangles = 0;
    if (!ValidateInputs(request, result, expectedTriangles))
    {
        return result;
    }
    std::wstring directory;
    std::wstring name;
    if (!SplitPath(outputBmd, directory, name, result.error))
    {
        return result;
    }
    std::error_code ec;
    if (!outputBmd.parent_path().empty())
    {
        std::filesystem::create_directories(outputBmd.parent_path(), ec);
    }

    // The engine parser writes into the global Models[] table; point it at a scratch model
    // for the duration of the conversion and hand the slot back afterwards.
    auto scratch = std::make_unique<BMD>();
    if (!RunEngineImport(request, *scratch, expectedTriangles, result.error))
    {
        Models = nullptr;
        return result;
    }
    AssignModelName(*scratch, request.modelName, request.referenceSmd);
    const bool saved = scratch->Save2(directory.data(), name.data());
    Models = nullptr;
    if (!saved)
    {
        result.error = "cannot write " + outputBmd.string();
        return result;
    }

    BMD verification;
    if (!LoadBmd(outputBmd, verification, result.error))
    {
        result.error = "the written file does not load back: " + result.error;
        return result;
    }
    if (verification.NumMeshs != scratch->NumMeshs || verification.NumBones != scratch->NumBones ||
        verification.NumActions != scratch->NumActions)
    {
        result.error = "the written file loads with different mesh/bone/action counts than expected";
        return result;
    }
    result.outputs.push_back(outputBmd);
    result.ok = true;
    return result;
}
namespace
{
float Distance(const float* a, const float* b)
{
    const float dx = a[0] - b[0];
    const float dy = a[1] - b[1];
    const float dz = a[2] - b[2];
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

bool CornersMatch(const WorldTriangle& a, const WorldTriangle& b, float tolerance, float& worstDistance)
{
    if (a.texture != b.texture)
    {
        return false;
    }
    float worst = 0.f;
    for (int corner = 0; corner < 3; ++corner)
    {
        const float distance = Distance(a.corners[corner].position, b.corners[corner].position);
        if (distance > tolerance || a.corners[corner].bone != b.corners[corner].bone)
        {
            return false;
        }
        worst = std::max(worst, distance);
    }
    worstDistance = worst;
    return true;
}

void CompareTriangles(const BMD& a, const BMD& b, float tolerance, CompareReport& report)
{
    std::vector<WorldTriangle> trianglesA;
    std::vector<WorldTriangle> trianglesB;
    CollectWorldTriangles(a, trianglesA);
    CollectWorldTriangles(b, trianglesB);
    report.trianglesA = static_cast<int>(trianglesA.size());
    report.trianglesB = static_cast<int>(trianglesB.size());
    std::vector<bool> used(trianglesB.size(), false);
    for (const WorldTriangle& triangle : trianglesA)
    {
        bool matched = false;
        for (size_t index = 0; index < trianglesB.size() && !matched; ++index)
        {
            float worst = 0.f;
            if (!used[index] && CornersMatch(triangle, trianglesB[index], tolerance, worst))
            {
                used[index] = true;
                matched = true;
                report.maxCornerDistance = std::max(report.maxCornerDistance, worst);
            }
        }
        if (!matched)
        {
            ++report.unmatchedTriangles;
        }
    }
}

void CompareSkeletons(const BMD& a, const BMD& b, CompareReport& report)
{
    const int bones = std::min(a.NumBones, b.NumBones);
    for (int bone = 0; bone < bones; ++bone)
    {
        const bool sameDummy = a.Bones[bone].Dummy == b.Bones[bone].Dummy;
        const bool sameName = a.Bones[bone].Dummy || FixedString(a.Bones[bone].Name, kNameCapacity) ==
                                                         FixedString(b.Bones[bone].Name, kNameCapacity);
        if (!sameDummy || !sameName)
        {
            ++report.differingBoneNames;
        }
    }
    const int actions = std::min(a.NumActions, b.NumActions);
    std::vector<BoneFrame> framesA;
    std::vector<BoneFrame> framesB;
    for (int action = 0; action < actions; ++action)
    {
        const int keysA = a.Actions[action].NumAnimationKeys;
        const int keysB = b.Actions[action].NumAnimationKeys;
        if (keysA != keysB)
        {
            report.notes.push_back("action " + std::to_string(action) + ": " + std::to_string(keysA) + " vs " +
                                   std::to_string(keysB) + " keyframes");
        }
        for (int key = 0; key < std::min(keysA, keysB); ++key)
        {
            ComputePose(a, action, key, framesA);
            ComputePose(b, action, key, framesB);
            for (int bone = 0; bone < bones; ++bone)
            {
                report.maxBoneDistance =
                    std::max(report.maxBoneDistance, Distance(framesA[bone].origin, framesB[bone].origin));
            }
        }
    }
}
} // namespace

CompareReport CompareModels(const BMD& a, const BMD& b, float positionTolerance)
{
    CompareReport report;
    report.meshesA = a.NumMeshs;
    report.meshesB = b.NumMeshs;
    report.bonesA = a.NumBones;
    report.bonesB = b.NumBones;
    report.actionsA = a.NumActions;
    report.actionsB = b.NumActions;
    if (a.NumBones != b.NumBones)
    {
        report.notes.push_back("bone count differs: the game addresses bones by index (attachments, head, feet)");
    }
    if (a.NumActions != b.NumActions)
    {
        report.notes.push_back("action count differs: the game addresses animations by index");
    }
    if (a.NumMeshs != b.NumMeshs)
    {
        report.notes.push_back("mesh count differs (meshes sharing a texture are merged by the SMD path)");
    }
    CompareTriangles(a, b, positionTolerance, report);
    CompareSkeletons(a, b, report);
    return report;
}

std::string DescribeComparison(const CompareReport& report)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision(4);
    out << "meshes " << report.meshesA << " vs " << report.meshesB << ", bones " << report.bonesA << " vs "
        << report.bonesB << ", actions " << report.actionsA << " vs " << report.actionsB << ", triangles "
        << report.trianglesA << " vs " << report.trianglesB << '\n';
    out << "unmatched triangles: " << report.unmatchedTriangles << "  max corner distance: " << report.maxCornerDistance
        << "  max bone distance: " << report.maxBoneDistance << "  differing bone names: " << report.differingBoneNames
        << '\n';
    for (const auto& note : report.notes)
    {
        out << "note: " << note << '\n';
    }
    out << (report.equivalent() ? "EQUIVALENT" : "DIFFERENT") << '\n';
    return out.str();
}
} // namespace Tools::BmdConv
