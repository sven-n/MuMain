#include "stdafx.h"
#include "Render/Core/BoneUBO.h"
#include <cstring>

BoneUBO& BoneUBO::Instance()
{
    static BoneUBO instance;
    return instance;
}

BoneUBO::~BoneUBO()
{
    Destroy();
}

void BoneUBO::Create()
{
    if (m_UBOHandle.IsValid()) return;

    memset(m_MatrixBuffer, 0, sizeof(m_MatrixBuffer));

    // Initialize with identity matrices
    for (int i = 0; i < GPU_MAX_BONES; i++) {
        float* m = m_MatrixBuffer + i * 16;
        m[0]  = 1.0f; m[5]  = 1.0f;
        m[10] = 1.0f; m[15] = 1.0f;
    }

    // Binding Slot 2 (Slot 1 reserved for SceneUBO fog) -- unchanged from the pre-RHI binding.
    m_UBOHandle = RHI::CreateUniformBlock(sizeof(m_MatrixBuffer), 2);
    RHI::UpdateUniformBlock(m_UBOHandle, m_MatrixBuffer, sizeof(m_MatrixBuffer));
}

void BoneUBO::Destroy()
{
    if (m_UBOHandle.IsValid()) {
        RHI::DestroyUniformBlock(m_UBOHandle);
        m_UBOHandle = {};
    }
}

void BoneUBO::Bind()
{
    if (!m_UBOHandle.IsValid()) Create();
    // RHI::CreateUniformBlock already bound this handle to slot 2 at creation, and
    // RHI_GL's uniform-block binding point (unlike a texture unit) isn't clobbered by
    // anything else that could bind in between -- nothing left to (re)do here.
}

void BoneUBO::UploadBones(const void* boneTransforms, int numBones)
{
    if (!m_UBOHandle.IsValid()) Create();
    if (!m_UBOHandle.IsValid() || !boneTransforms || numBones <= 0) return;

    if (numBones > GPU_MAX_BONES) numBones = GPU_MAX_BONES;

    // MU Online BoneTransform is float[MAX_BONES][3][4] (a 3x4 matrix per bone)
    // We convert 3x4 affine matrices into std140 column-major 4x4 mat4 arrays.
    const float* src = static_cast<const float*>(boneTransforms);
    const size_t srcBytes = static_cast<size_t>(numBones) * 12 * sizeof(float);

    // Same object's palette as last call (e.g. the next equipped armor piece on the same
    // character) — already uploaded, skip the repack + GPU upload. Compared by content, not
    // by pointer: several callers (weapons, wings, effects) share one fixed-address scratch
    // buffer across genuinely different objects, so pointer identity alone can't tell "same
    // data" apart from "different object, same address, different data".
    if (m_HasSnapshot && srcBytes == m_LastSourceBytes && std::memcmp(src, m_LastSourceSnapshot, srcBytes) == 0)
        return;

    std::memcpy(m_LastSourceSnapshot, src, srcBytes);
    m_LastSourceBytes = srcBytes;
    m_HasSnapshot = true;

    for (int b = 0; b < numBones; b++) {
        const float* m3x4 = src + b * 12; // 3 rows, 4 columns
        float* dst = m_MatrixBuffer + b * 16;

        // Column-major mat4:
        // col 0
        dst[0]  = m3x4[0];  // m[0][0]
        dst[1]  = m3x4[4];  // m[1][0]
        dst[2]  = m3x4[8];  // m[2][0]
        dst[3]  = 0.0f;
        // col 1
        dst[4]  = m3x4[1];  // m[0][1]
        dst[5]  = m3x4[5];  // m[1][1]
        dst[6]  = m3x4[9];  // m[2][1]
        dst[7]  = 0.0f;
        // col 2
        dst[8]  = m3x4[2];  // m[0][2]
        dst[9]  = m3x4[6];  // m[1][2]
        dst[10] = m3x4[10]; // m[2][2]
        dst[11] = 0.0f;
        // col 3 (translation)
        dst[12] = m3x4[3];  // m[0][3]
        dst[13] = m3x4[7];  // m[1][3]
        dst[14] = m3x4[11]; // m[2][3]
        dst[15] = 1.0f;
    }

    // Always upload the FULL m_MatrixBuffer, not just the touched numBones*16 floats: RHI's
    // uniform-block update is a Map(WRITE_DISCARD) under D3D11 (CreateUniformBlock's doc
    // comment on RHI_D3D11's side), which invalidates the ENTIRE previous GPU allocation, not
    // just the written sub-range like GL's glBufferSubData did pre-RHI. A partial upload would
    // leave every bone past numBones as undefined garbage instead of the Create()-time identity
    // matrix once a second real UploadBones() call landed under D3D11 -- m_MatrixBuffer already
    // holds correct identity padding out to GPU_MAX_BONES from Create(), so this costs nothing
    // extra on GL (same bytes, just more of them) and removes the D3D11 footgun outright.
    RHI::UpdateUniformBlock(m_UBOHandle, m_MatrixBuffer, sizeof(m_MatrixBuffer));
}
