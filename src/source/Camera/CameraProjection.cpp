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
    // Convert from 640×480 reference to actual window size
    int actualX = x * WindowWidth / 640;
    int actualY = y * WindowHeight / 480;
    int actualWidth = width * WindowWidth / 640;
    int actualHeight = height * WindowHeight / 480;

    // Set OpenGL viewport (Y coordinate is flipped)
    glViewport(actualX, WindowHeight - (actualY + actualHeight), actualWidth, actualHeight);

    // Update cached viewport dimensions
    OpenglWindowX = actualX;
    OpenglWindowY = actualY;
    OpenglWindowWidth = actualWidth;
    OpenglWindowHeight = actualHeight;
}

void CameraProjection::ScreenToWorldRay(const CameraState& state, int sx, int sy,
                                         vec3_t outTarget, bool bFixView)
{
    // Convert reference coordinates to actual pixels
    sx = sx * WindowWidth / 640;
    sy = sy * WindowHeight / 480;

    vec3_t p1, p2;

    // Choose far plane based on use case
    float farDist = bFixView ? state.ViewFar : RENDER_ITEMVIEW_FAR;

    // Calculate ray endpoint at far plane
    p1[0] = (float)(sx - state.ScreenCenterX) * farDist * state.PerspectiveX;
    p1[1] = -(float)(sy - state.ScreenCenterY) * farDist * state.PerspectiveY;
    p1[2] = -farDist;

    // Transform by camera matrix to get world-space direction
    VectorRotate(p1, state.Matrix, p2);
    VectorAdd(state.Position, p2, outTarget);
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
