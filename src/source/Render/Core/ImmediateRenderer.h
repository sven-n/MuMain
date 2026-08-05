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
    void End();  // flushes to GPU, issues glDrawArrays
}
