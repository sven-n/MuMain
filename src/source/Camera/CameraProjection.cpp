#include "stdafx.h"
#include "CameraProjection.h"
#include "CameraState.h"
#include "CameraConfig.h"
#include "Render/Renderer/MuRenderer.h"
#include "UI/Scaling/UITransform.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// External window dimensions
extern unsigned int WindowWidth;
extern unsigned int WindowHeight;
extern int OpenglWindowX;
extern int OpenglWindowY;
extern int OpenglWindowWidth;
extern int OpenglWindowHeight;
extern EGameScene SceneFlag;

void CameraProjection::UpdateView(CameraState& state, int x, int y, int width, int height)
{
    width = std::max(width, 1);
    height = std::max(height, 1);

    // The ops the renderer's matrix stack applied, so the view matches what it built bit for bit.
    glm::mat4 view(1.f);
    view = glm::rotate(view, glm::radians(state.Angle[1]), glm::vec3(0.f, 1.f, 0.f));
    if (state.TopViewEnable == false)
        view = glm::rotate(view, glm::radians(state.Angle[0]), glm::vec3(1.f, 0.f, 0.f));
    view = glm::rotate(view, glm::radians(state.Angle[2]), glm::vec3(0.f, 0.f, 1.f));
    view = glm::translate(view, glm::vec3(-state.Position[0], -state.Position[1], -state.Position[2]));
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            state.Matrix[i][j] = view[j][i];
        }
    }

    state.ScreenCenterX = x + width / 2;
    state.ScreenCenterY = y + height / 2;
    state.ScreenCenterYFlip = WindowHeight - state.ScreenCenterY;

    const float aspect = static_cast<float>(width) / height;
    const float fovRad = state.FOV * 0.5f * Q_PI / 180.0f;
    state.PerspectiveX = tanf(fovRad) / (float)(width / 2) * aspect;
    state.PerspectiveY = tanf(fovRad) / (float)(height / 2);
}

void CameraProjection::ApplyView(const CameraState& state, int width, int height)
{
    width = std::max(width, 1);
    height = std::max(height, 1);

    auto& renderer = mu::GetRenderer();
    renderer.SetMatrixMode(GL_PROJECTION);
    renderer.LoadIdentity();
    gluPerspective(state.FOV, static_cast<float>(width) / height, state.ViewNear,
                   state.ViewFar * RENDER_DISTANCE_MULTIPLIER);

    float view[16] = {};
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            view[j * 4 + i] = state.Matrix[i][j];
        }
    }
    view[15] = 1.f;
    renderer.SetMatrixMode(GL_MODELVIEW);
    renderer.LoadMatrix(view);
}

float CameraProjection::WorldAspectRatio()
{
    if (SceneFlag == MAIN_SCENE)
        return UI::Scaling::WorldViewportAspect(WindowWidth, WindowHeight, g_Camera.TopViewEnable);
    return static_cast<float>(WindowWidth) / WindowHeight;
}

void CameraProjection::SetViewport(int x, int y, int width, int height)
{
    OpenglWindowX = x;
    OpenglWindowY = y;

    mu::GetRenderer().SetViewport(x, y, width, height);
    mu::GetRenderer().SetScissor(x, y, width, height);
}

void CameraProjection::ScreenToWorldRay(const CameraState& state, int sx, int sy,
                                         vec3_t outTarget, bool bFixView)
{
    // Convert active logical coordinates to window pixels. World picking uses
    // the screen-overlay transform; panel item previews use the panel transform.
    sx = static_cast<int>(static_cast<float>(sx) * g_fScreenRate_x + g_fScreenOffset_x);
    sy = static_cast<int>(static_cast<float>(sy) * g_fScreenRate_y + g_fScreenOffset_y);

    vec3_t p1, p2;

    float farDist = bFixView ? state.ViewFar : RENDER_ITEMVIEW_FAR;

    p1[0] = (float)(sx - state.ScreenCenterX) * farDist * state.PerspectiveX;
    p1[1] = -(float)(sy - state.ScreenCenterY) * farDist * state.PerspectiveY;
    p1[2] = -farDist;

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
    *outX = *outX * REFERENCE_WIDTH / (int)WindowWidth;
    *outY = *outY * REFERENCE_HEIGHT / (int)WindowHeight;
}

bool CameraProjection::WorldToWindowPixel(const CameraState& state, const vec3_t worldPos, float* outX, float* outY)
{
    vec3_t transformPos;
    VectorTransform(worldPos, state.Matrix, transformPos);
    if (transformPos[2] >= 0.0f)
    {
        return false;
    }

    *outX = static_cast<float>(state.ScreenCenterX) - transformPos[0] / (state.PerspectiveX * transformPos[2]);
    *outY = static_cast<float>(state.ScreenCenterY) + transformPos[1] / (state.PerspectiveY * transformPos[2]);
    return true;
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

    // Backends without depth readback leave the far-depth fallback unchanged.
    GLfloat depth = 1.f;
    mu::GetRenderer().ReadPixels(x, y, 1, 1, &depth);

    // Expected window-space depth from a standard gluPerspective projection:
    //   z_window = (f / (f - n)) * (1 + n / z_eye)        with z_eye < 0
    // This evaluates to 0 at the near plane and 1 at the far plane. The
    // earlier approximation (1 - n/-z + n/f) was off by n/f at the near plane,
    // which can cause false occlusion for objects very close to the camera.
    const float n = state.ViewNear;
    const float f = state.ViewFar;
    const float z_eye = worldPosition[2];  // negative for points in front of camera
    const float expected = (f / (f - n)) * (1.0f + n / z_eye);

    return (depth >= expected);
}

void CameraProjection::GetOpenGLMatrix(float outMatrix[3][4])
{
    float openglMatrix[16];
    mu::GetRenderer().GetMatrix(GL_MODELVIEW_MATRIX, openglMatrix);

    // Convert from OpenGL 4×4 to our 3×4 format
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            outMatrix[i][j] = openglMatrix[j * 4 + i];
        }
    }
}
