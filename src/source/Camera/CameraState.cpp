#include "stdafx.h"
#include "CameraState.h"
// Note: stdafx.h includes:
// - _types.h which has IdentityVector3D() for clearing vec3_t
// - ZzzMathLib.h which has AngleMatrix()

// Global camera state instance
CameraState g_Camera;

CameraState::CameraState()
{
    Reset();
}

void CameraState::Reset()
{
    // Transform
    IdentityVector3D(Position);  // Clears vec3_t to zero (defined in _types.h)
    IdentityVector3D(Angle);
    memset(Matrix, 0, sizeof(Matrix));

    // View frustum
    ViewNear = 20.0f;
    ViewFar = 2000.0f;
    FOV = 55.0f;
    TopViewEnable = false;

    // Camera behavior
    Distance = 1000.0f;
    DistanceTarget = 1000.0f;
    ZoomLevel = 0;
    FOV3D = 0.0f;
    Roll3D = false;
    CustomDistance = 0.0f;

    // Projection cache
    PerspectiveX = 0.0f;
    PerspectiveY = 0.0f;
    ScreenCenterX = 0;
    ScreenCenterY = 0;
    ScreenCenterYFlip = 0;
}

void CameraState::UpdateMatrix()
{
    // Create rotation matrix from angles
    float matrix[3][4];
    AngleMatrix(Angle, matrix);
    memcpy(Matrix, matrix, sizeof(Matrix));
}
