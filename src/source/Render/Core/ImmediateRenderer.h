#pragma once

#include "stdafx.h"
#include <cstdint>

// ImmediateRenderer (IR) provides a streaming VBO bridge that mirrors the legacy
// glBegin / glEnd immediate mode API, decomposing GL_QUADS to GL_TRIANGLES.
namespace IR {
    void Create();
    void Destroy();

    void Begin(GLenum mode);
    void Color4f(float r, float g, float b, float a);
    void Color4ub(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    void Color3f(float r, float g, float b);
    void Color3fv(const float* c);
    void TexCoord2f(float u, float v);
    void Vertex3fv(const float* p);
    void Vertex3f(float x, float y, float z);
    void Vertex2f(float x, float y);   // z = 0

    // GLP-19: End() no longer draws. It marks the accumulated vertices as a pending batch; the
    // next Begin() merges into it when the render state and topology still match, so a run of
    // same-state quads becomes ONE draw call instead of one per quad.
    void End();

    // Issues the pending batch, if any. Idempotent. Must be called before anything that changes
    // state the pending vertices were submitted under, and before the frame is presented --
    // PlatformSwapBuffers() does the latter. The state hooks that call this live in
    // PassthroughShader (uniform changes) and BindTexture (texture changes); everything else is
    // caught by the state-key comparison in Begin().
    void Flush();
}
