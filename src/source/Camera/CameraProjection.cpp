#include "stdafx.h"
#include "CameraProjection.h"
#include "CameraState.h"

// External window dimensions
extern unsigned int WindowWidth;
extern unsigned int WindowHeight;
extern int OpenglWindowX;
extern int OpenglWindowY;
extern int OpenglWindowWidth;
extern int OpenglWindowHeight;

void CameraProjection::SetupPerspective(CameraState& state, float fov, float aspect,
                                          float zNear, float zFar)
{
    // Set up OpenGL perspective
    gluPerspective(fov, aspect, zNear, zFar);

    // Cache screen center
    state.ScreenCenterX = OpenglWindowX + OpenglWindowWidth / 2;
    state.ScreenCenterY = OpenglWindowY + OpenglWindowHeight / 2;
    state.ScreenCenterYFlip = WindowWidth - state.ScreenCenterY;

    // Cache perspective factors for screen-space transformations
    float aspectY = (float)WindowHeight / (float)OpenglWindowHeight;
    float fovRad = fov * 0.5f * Q_PI / 180.0f;

    state.PerspectiveX = tanf(fovRad) / (float)(OpenglWindowWidth / 2) * aspect;
    state.PerspectiveY = tanf(fovRad) / (float)(OpenglWindowHeight / 2) * aspectY;
}

void CameraProjection::SetViewport(int x, int y, int width, int height)
{
    // Update cached viewport dimensions
    OpenglWindowX = x;
    OpenglWindowY = y;
    OpenglWindowWidth = width;
    OpenglWindowHeight = height;

    // Set OpenGL viewport (Y coordinate is flipped)
    glViewport(x, WindowHeight - (y + height), width, height);
}

void CameraProjection::ScreenToWorldRay(const CameraState& state, int sx, int sy,
                                         vec3_t outTarget, bool bFixView)
{
    // Convert reference coordinates to actual pixels
    sx = sx * WindowWidth / 640;
    sy = sy * WindowHeight / 480;

    vec3_t p1, p2;

    // Choose far plane based on use case
    // FIX: Use 1.4x multiplier to match terrain rendering distance (prevents "visible but not clickable" terrain)
    float farDist = bFixView ? (state.ViewFar * 1.4f) : RENDER_ITEMVIEW_FAR;

    // Calculate ray endpoint at far plane
    p1[0] = (float)(sx - state.ScreenCenterX) * farDist * state.PerspectiveX;
    p1[1] = -(float)(sy - state.ScreenCenterY) * farDist * state.PerspectiveY;
    p1[2] = -farDist;

    // Extract camera position from matrix and transform ray
    p2[0] = -state.Matrix[0][3];
    p2[1] = -state.Matrix[1][3];
    p2[2] = -state.Matrix[2][3];
    VectorIRotate(p2, state.Matrix, MousePosition);
    VectorIRotate(p1, state.Matrix, p2);
    VectorAdd(MousePosition, p2, outTarget);
}

void CameraProjection::WorldToScreen(const CameraState& state, const vec3_t worldPos,
                                      int* outX, int* outY)
{
    vec3_t transformPos;
    VectorTransform(worldPos, state.Matrix, transformPos);

    // Project to screen
    *outX = -(int)(transformPos[0] / state.PerspectiveX / transformPos[2]) + state.ScreenCenterX;
    *outY = (int)(transformPos[1] / state.PerspectiveY / transformPos[2]) + state.ScreenCenterY;

    // Convert to 640×480 reference coordinates
    *outX = *outX * 640 / (int)WindowWidth;
    *outY = *outY * 480 / (int)WindowHeight;
}

void CameraProjection::TransformPosition(const CameraState& state, const vec3_t position,
                                          vec3_t outWorldPosition, int* outX, int* outY)
{
    vec3_t temp;
    VectorSubtract(position, state.Position, temp);
    VectorRotate(temp, state.Matrix, outWorldPosition);

    // Project to screen (pixel coordinates)
    *outX = (int)(outWorldPosition[0] / state.PerspectiveX / -outWorldPosition[2]) + state.ScreenCenterX;
    *outY = (int)(outWorldPosition[1] / state.PerspectiveY / -outWorldPosition[2]) + state.ScreenCenterYFlip;
}

bool CameraProjection::TestDepthBuffer(const CameraState& state, const vec3_t position)
{
    vec3_t worldPosition;
    int x, y;
    TransformPosition(state, position, worldPosition, &x, &y);

    // Check if within viewport
    if (x < OpenglWindowX || y < OpenglWindowY ||
        x >= OpenglWindowX + OpenglWindowWidth ||
        y >= OpenglWindowY + OpenglWindowHeight)
    {
        return false;
    }

    // Read depth buffer
    GLfloat depth;
    glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

    // Calculate expected depth
    float z = 1.0f - state.ViewNear / -worldPosition[2] + state.ViewNear / state.ViewFar;

    return (depth >= z);
}

void CameraProjection::GetOpenGLMatrix(float outMatrix[3][4])
{
    float openglMatrix[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, openglMatrix);

    // Convert from OpenGL 4×4 to our 3×4 format
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            outMatrix[i][j] = openglMatrix[j * 4 + i];
        }
    }
}
