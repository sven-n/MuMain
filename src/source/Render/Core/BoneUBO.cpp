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

    memset(m_BoneRows, 0, sizeof(m_BoneRows));

    // Initialize with identity transforms -- row form: row0=(1,0,0,0), row1=(0,1,0,0),
    // row2=(0,0,1,0) (the implicit 4th row (0,0,0,1) that made these identity as a mat4 is now
    // the shader's job to supply, not this buffer's).
    for (int i = 0; i < GPU_MAX_BONES; i++) {
        float* m = m_BoneRows + i * 12;
        m[0] = 1.0f; m[5] = 1.0f; m[10] = 1.0f;
    }

    // Binding Slot 2 (Slot 1 reserved for SceneUBO fog) -- unchanged from the pre-RHI binding.
    m_UBOHandle = RHI::CreateUniformBlock(sizeof(m_BoneRows), 2);
    RHI::UpdateUniformBlock(m_UBOHandle, m_BoneRows, sizeof(m_BoneRows));
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

void BoneUBO::UploadBones(const void* boneTransforms, int numBones, unsigned int version)
{
    if (!m_UBOHandle.IsValid()) Create();
    if (!m_UBOHandle.IsValid() || !boneTransforms || numBones <= 0) return;

    // Same object's palette as last call (e.g. the next equipped armor piece on the same
    // character) — already uploaded, skip the repack + GPU upload.
    if (boneTransforms == m_LastUploadedPtr && version == m_LastUploadedVersion) return;
    m_LastUploadedPtr = boneTransforms;
    m_LastUploadedVersion = version;

    if (numBones > GPU_MAX_BONES) numBones = GPU_MAX_BONES;

    // GLP-11: BoneTransform is float[MAX_BONES][3][4] -- row-major 3x4 affine, already exactly
    // the layout the vec4[3*N] palette wants (row i of bone b is 4 contiguous floats). No
    // transpose, no per-element repack -- straight memcpy of the touched prefix.
    const float* src = static_cast<const float*>(boneTransforms);
    memcpy(m_BoneRows, src, (size_t)numBones * 12 * sizeof(float));

    // Always upload the FULL m_BoneRows, not just the touched numBones*12 floats: RHI's
    // uniform-block update is a Map(WRITE_DISCARD) under D3D11 (CreateUniformBlock's doc
    // comment on RHI_D3D11's side), which invalidates the ENTIRE previous GPU allocation, not
    // just the written sub-range like GL's glBufferSubData did pre-RHI. A partial upload would
    // leave every bone past numBones as undefined garbage instead of the Create()-time identity
    // rows once a second real UploadBones() call landed under D3D11 -- m_BoneRows already
    // holds correct identity padding out to GPU_MAX_BONES from Create(), so this costs nothing
    // extra on GL (same bytes, just more of them) and removes the D3D11 footgun outright.
    RHI::UpdateUniformBlock(m_UBOHandle, m_BoneRows, sizeof(m_BoneRows));
}
