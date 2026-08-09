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
    // Dedups by comparing the incoming bone-matrix bytes against the last upload, so
    // repeated calls with identical content (e.g. body + every equipped armor piece of
    // one character, all sharing the same active bone palette) skip the repack +
    // glBufferSubData entirely. This is content comparison, not pointer comparison:
    // several callers (weapons, wings, effects) share a single fixed-address scratch
    // buffer across genuinely different objects, so the same pointer can carry different
    // content from one call to the next — only comparing the actual bytes is safe here.
    void UploadBones(const void* boneTransforms, int numBones);

    bool IsCreated() const { return m_UBOHandle.IsValid(); }

private:
    BoneUBO() = default;
    ~BoneUBO();

    BoneUBO(const BoneUBO&) = delete;
    BoneUBO& operator=(const BoneUBO&) = delete;

    RHI::BufferHandle m_UBOHandle;
    float  m_MatrixBuffer[GPU_MAX_BONES * 16]; // std140 mat4 array (column-major)

    float  m_LastSourceSnapshot[GPU_MAX_BONES * 12]; // raw source bytes from the last upload
    size_t m_LastSourceBytes = 0;
    bool   m_HasSnapshot     = false;
};
