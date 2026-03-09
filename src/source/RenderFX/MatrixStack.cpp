// MatrixStack.cpp: Software matrix stack implementation.
// Story 4.2.1 — Flow Code: VS1-RENDER-ABSTRACT-CORE
//
// Implements mu::Matrix4x4 and mu::MatrixStack.
// No OpenGL calls — pure C++20 math only.
//
// Matrix storage convention (matches OpenGL column-major float[16]):
//   m[col*4 + row], so:
//     m[0..3]  = column 0 (basis X)
//     m[4..7]  = column 1 (basis Y)
//     m[8..11] = column 2 (basis Z)
//     m[12..14]= column 3 (translation Tx,Ty,Tz); m[15]=1
//
// This convention places translation at m[12], m[13], m[14] — matching
// OpenGL's internal layout and the test expectations in test_murenderer.cpp.
#include "stdafx.h"
#include "MatrixStack.h"

namespace mu
{

// ---------------------------------------------------------------------------
// Matrix4x4: Constructor — build the identity matrix in column-major layout.
// Diagonal elements m[0], m[5], m[10], m[15] = 1.0f; all others = 0.0f.
// ---------------------------------------------------------------------------
Matrix4x4::Matrix4x4()
{
    for (int i = 0; i < 16; ++i)
    {
        m[i] = 0.0f;
    }
    // Diagonal: col=0,row=0 → m[0]; col=1,row=1 → m[5]; col=2,row=2 → m[10]; col=3,row=3 → m[15]
    m[0] = 1.0f;
    m[5] = 1.0f;
    m[10] = 1.0f;
    m[15] = 1.0f;
}

// ---------------------------------------------------------------------------
// Matrix4x4::operator*: Column-major 4×4 matrix multiplication.
//
// Using column-major indexing: m[col*4 + row]
// C[col][row] = sum_k A[k][row] * B[col][k]
//             = sum_k A.m[k*4+row] * B.m[col*4+k]
// ---------------------------------------------------------------------------
Matrix4x4 Matrix4x4::operator*(const Matrix4x4& other) const
{
    Matrix4x4 result;
    for (int col = 0; col < 4; ++col)
    {
        for (int row = 0; row < 4; ++row)
        {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k)
            {
                // A[k][row] = m[k*4+row], B[col][k] = other.m[col*4+k]
                sum += m[k * 4 + row] * other.m[col * 4 + k];
            }
            result.m[col * 4 + row] = sum;
        }
    }
    return result;
}

// ---------------------------------------------------------------------------
// MatrixStack: Constructor — push one identity matrix so Top() is always valid.
// ---------------------------------------------------------------------------
MatrixStack::MatrixStack()
{
    m_stack.push(Matrix4x4{});
}

// ---------------------------------------------------------------------------
// Push: Copy the current top onto the stack (equivalent to glPushMatrix).
// ---------------------------------------------------------------------------
void MatrixStack::Push()
{
    m_stack.push(m_stack.top());
}

// ---------------------------------------------------------------------------
// Pop: Discard the top entry.
// Guard: do not pop when only one entry remains to prevent underflow.
// ---------------------------------------------------------------------------
void MatrixStack::Pop()
{
    if (m_stack.size() > 1)
    {
        m_stack.pop();
    }
}

// ---------------------------------------------------------------------------
// Translate: Post-multiply the top matrix by a pure translation matrix T(x,y,z).
//
// Column-major translation matrix T:
//   col 0: [1,0,0,0]  col 1: [0,1,0,0]  col 2: [0,0,1,0]  col 3: [x,y,z,1]
//   → m[0]=1, m[5]=1, m[10]=1, m[15]=1, m[12]=x, m[13]=y, m[14]=z
//
// After top = top * T, the translation accumulates: new_tx = old_tx + x
// (This matches glTranslatef post-multiplication semantics.)
// ---------------------------------------------------------------------------
void MatrixStack::Translate(float x, float y, float z)
{
    Matrix4x4 t; // identity
    t.m[12] = x;
    t.m[13] = y;
    t.m[14] = z;
    m_stack.top() = m_stack.top() * t;
}

// ---------------------------------------------------------------------------
// Top: Return const reference to the current top matrix.
// ---------------------------------------------------------------------------
const Matrix4x4& MatrixStack::Top() const
{
    return m_stack.top();
}

// ---------------------------------------------------------------------------
// IsEmpty: Returns true if the internal stack is empty.
// Under normal use this is always false after construction.
// ---------------------------------------------------------------------------
bool MatrixStack::IsEmpty() const
{
    return m_stack.empty();
}

} // namespace mu
