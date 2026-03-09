// MatrixStack.h: Software model-matrix stack for the MuRenderer abstraction.
// Story 4.2.1 — Flow Code: VS1-RENDER-ABSTRACT-CORE
//
// Replaces the conceptual role of glPushMatrix/glPopMatrix/glTranslatef.
// Used internally by MuRendererGL and future SDL_gpu backend to track the
// current model transform without exposing OpenGL state to game code.
//
// All types are plain C++20 — no OpenGL headers required by callers.
#pragma once

#include <array>
#include <stack>

namespace mu
{

// ---------------------------------------------------------------------------
// Matrix4x4: Column-major 4×4 float matrix (matches OpenGL float[16] layout).
// Default constructor produces the 4×4 identity matrix.
//
// Storage: m[col*4 + row]
//   m[0..3]  = column 0 (X basis)
//   m[4..7]  = column 1 (Y basis)
//   m[8..11] = column 2 (Z basis)
//   m[12..14]= translation (Tx,Ty,Tz); m[15] = 1
// ---------------------------------------------------------------------------
struct Matrix4x4
{
    float m[16]; // column-major: m[col*4 + row]

    // Constructs the identity matrix.
    Matrix4x4();

    // Standard 4×4 matrix multiplication (A * B).
    Matrix4x4 operator*(const Matrix4x4& other) const;
};

// ---------------------------------------------------------------------------
// MatrixStack: Push/pop matrix stack mirroring OpenGL's matrix stack.
//
// Construction initialises the stack with a single identity matrix so that
// Top() is always valid and IsEmpty() never returns true in normal use.
// Pop() is a no-op when only one entry remains (underflow guard).
// ---------------------------------------------------------------------------
class MatrixStack
{
public:
    // Initialises the stack with one identity matrix.
    MatrixStack();

    // Copies the current top onto the stack (equivalent to glPushMatrix).
    void Push();

    // Discards the top entry; no-op if only one entry remains (underflow guard).
    void Pop();

    // Post-multiplies the top matrix by a translation matrix T(x,y,z).
    // Equivalent to: top = top * T, where T has 1s on diagonal and (x,y,z) in
    // column 3 row-major positions m[12], m[13], m[14].
    void Translate(float x, float y, float z);

    // Returns the current top matrix (read-only reference).
    [[nodiscard]] const Matrix4x4& Top() const;

    // Returns true if the internal std::stack is empty.
    // Under normal use this is always false (constructor pushes one identity).
    [[nodiscard]] bool IsEmpty() const;

private:
    std::stack<Matrix4x4> m_stack;
};

} // namespace mu
