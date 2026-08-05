#pragma once

#include "stdafx.h"
#include "Render/RHI/RHI.h"

constexpr int GPU_MAX_BONES = 200;

// DXP-14 increment 3: backed by RHI::CreateUniformBlock/UpdateUniformBlock instead of raw GL
// calls (same mechanical, GL-behavior-preserving migration GlobalUBO/SceneUBO got in increment
// 1) -- BMDMeshShader's BoneMatrices cbuffer depends on it existing under both backends.
class BoneUBO {
public:
    static BoneUBO& Instance();

    void Create();
    void Destroy();
    void Bind();

    // Upload bone matrix palette for current character (numBones <= GPU_MAX_BONES).
    // boneTransforms is vec3_t[MAX_BONES][3][4] or vec3_t BoneTransform[MAX_BONES][4].
    // `version` is g_BoneTransformVersion (ZzzBMD.h) at the time of the call — combined
    // with the pointer, it lets repeated calls with the same (pointer, version) pair
    // (e.g. body + every equipped armor piece of one character, all sharing the same
    // active bone palette) skip the repack + glBufferSubData entirely. Pointer identity
    // alone is NOT safe here — see SetActiveBoneTransform()'s callers for why.
    void UploadBones(const void* boneTransforms, int numBones, unsigned int version);

    bool IsCreated() const { return m_UBOHandle.IsValid(); }

private:
    BoneUBO() = default;
    ~BoneUBO();

    BoneUBO(const BoneUBO&) = delete;
    BoneUBO& operator=(const BoneUBO&) = delete;

    RHI::BufferHandle m_UBOHandle;
    float  m_MatrixBuffer[GPU_MAX_BONES * 16]; // std140 mat4 array (column-major)

    const void*  m_LastUploadedPtr     = nullptr;
    unsigned int m_LastUploadedVersion = 0xFFFFFFFFu; // sentinel: never matches a real version
};
