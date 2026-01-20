// CameraUtility.cpp - Camera movement and positioning utilities
// Extracted from ZzzScene.cpp as part of scene refactoring

#include "stdafx.h"
#include "CameraUtility.h"
#include "../Scenes/SceneCore.h"
#include "../ZzzInterface.h"
#include "../ZzzOpenglUtil.h"
#include "../ZzzObject.h"
#include "../ZzzCharacter.h"
#include "../ZzzLodTerrain.h"
#include "../MapManager.h"
#include "../CameraMove.h"
#include "../NewUISystem.h"
#include "../CDirection.h"
#include "../GMBattleCastle.h"
#include "../GMDoppelGanger1.h"
#include "../GMDoppelGanger2.h"
#include "../w_MapHeaders.h"
#include "../UIManager.h"

// External variable declarations (definitions remain in ZzzScene.cpp)
extern short g_shCameraLevel;
extern float g_fSpecialHeight;
extern float CameraDistanceTarget;
extern float CameraDistance;
extern float Camera3DFov;
extern bool Camera3DRoll;

/**
 * @brief Calculates camera view distance based on scene and world settings.
 */
static float CalculateCameraViewFar(int sceneFlag)
{
    if (battleCastle::InBattleCastle2(Hero->Object.Position))
    {
        return 3000.f;
    }

    if (gMapManager.InBattleCastle() && sceneFlag == MAIN_SCENE)
    {
        return 2500.f;
    }

    if (gMapManager.WorldActive == WD_51HOME_6TH_CHAR)
    {
        return 2800.f * 1.15f;
    }

    if (gMapManager.IsPKField() || IsDoppelGanger2())
    {
        return 3700.0f;
    }

    // Handle camera level based view distance
    switch (g_shCameraLevel)
    {
    case 0:
        if (sceneFlag == LOG_IN_SCENE)
        {
            return CameraViewFar; // Use existing value
        }
        else if (sceneFlag == CHARACTER_SCENE)
        {
            return 3500.f;
        }
        else if (g_Direction.m_CKanturu.IsMayaScene())
        {
            return 2000.f * 10.0f * 0.115f;
        }
        else
        {
            return 2000.f;
        }
    case 1: return 2500.f;
    case 2: return 2600.f;
    case 3: return 2950.f;
    case 4:
    case 5: return 3200.f;
    default: return 2000.f;
    }
}

/**
 * @brief Adjusts hero height based on terrain and mount status.
 */
static void AdjustHeroHeight()
{
    if (gMapManager.InChaosCastle() == false || !Hero->Object.m_bActionStart)
    {
        // Skip height adjustment for Kanturu 3rd when action has started
        if (gMapManager.WorldActive == WD_39KANTURU_3RD && Hero->Object.m_bActionStart)
        {
            // No-op: Hero maintains current Z position during action in Kanturu 3rd
        }
        else if (gMapManager.WorldActive == -1 || Hero->Helper.Type != MODEL_HORN_OF_DINORANT || Hero->SafeZone)
        {
            Hero->Object.Position[2] = RequestTerrainHeight(Hero->Object.Position[0], Hero->Object.Position[1]);
        }
        else
        {
            float baseHeight = RequestTerrainHeight(Hero->Object.Position[0], Hero->Object.Position[1]);
            if (gMapManager.WorldActive == WD_8TARKAN || gMapManager.WorldActive == WD_10HEAVEN)
                Hero->Object.Position[2] = baseHeight + 90.f;
            else
                Hero->Object.Position[2] = baseHeight + 30.f;
        }
    }
}

/**
 * @brief Calculates camera position based on scene and world state.
 */
static void CalculateCameraPosition(vec3_t outCameraPosition)
{
    int iIndex = TERRAIN_INDEX((Hero->PositionX), (Hero->PositionY));
    vec3_t Position, TransformPosition;
    float Matrix[3][4];

    CameraViewFar = CalculateCameraViewFar(SceneFlag);

    Vector(0.f, -CameraDistance, 0.f, Position);
    AngleMatrix(CameraAngle, Matrix);
    VectorIRotate(Position, Matrix, TransformPosition);

    if (SceneFlag == MAIN_SCENE)
    {
        g_pCatapultWindow->GetCameraPos(Position);
    }
    else if (CCameraMove::GetInstancePtr()->IsTourMode())
    {
        CCameraMove::GetInstancePtr()->UpdateTourWayPoint();
        CCameraMove::GetInstancePtr()->GetCurrentCameraPos(Position);
        CameraViewFar = 390.f * CCameraMove::GetInstancePtr()->GetCurrentCameraDistanceLevel();
    }

    if (g_Direction.IsDirection() && !g_Direction.m_bDownHero)
    {
        Hero->Object.Position[2] = 300.0f;
        g_shCameraLevel = g_Direction.GetCameraPosition(Position);
    }
    else if (gMapManager.IsPKField() || IsDoppelGanger2())
    {
        g_shCameraLevel = 5;
    }
    else if (IsDoppelGanger1())
    {
        g_shCameraLevel = 5;
    }
    else g_shCameraLevel = 0;

    if (CCameraMove::GetInstancePtr()->IsTourMode())
    {
        vec3_t temp = { 0.0f, 0.0f, -100.0f };
        VectorAdd(TransformPosition, temp, TransformPosition);
    }

    VectorAdd(Position, TransformPosition, outCameraPosition);

    if (gMapManager.InBattleCastle())
    {
        outCameraPosition[2] = 255.f;
    }
    else if (!CCameraMove::GetInstancePtr()->IsTourMode())
    {
        outCameraPosition[2] = Hero->Object.Position[2];
    }

    if ((TerrainWall[iIndex] & TW_HEIGHT) == TW_HEIGHT)
    {
        outCameraPosition[2] = g_fSpecialHeight = 1200.f + 1;
    }
    outCameraPosition[2] += CameraDistance - 150.f;

    // Apply custom camera distance for special terrain
    if (g_fCameraCustomDistance != 0.f)
    {
        vec3_t angle = { 0.f, 0.f, -45.f };
        Vector(0.f, g_fCameraCustomDistance, 0.f, Position);
        AngleMatrix(angle, Matrix);
        VectorIRotate(Position, Matrix, TransformPosition);
        VectorAdd(outCameraPosition, TransformPosition, outCameraPosition);
    }
}

/**
 * @brief Sets camera angle based on scene and tour mode.
 */
static void SetCameraAngle()
{
    if (CCameraMove::GetInstancePtr()->IsTourMode())
    {
        CCameraMove::GetInstancePtr()->SetAngleFrustum(-112.5f);
        CameraAngle[0] = CCameraMove::GetInstancePtr()->GetAngleFrustum();
        CameraAngle[1] = 0.0f;
        CameraAngle[2] = CCameraMove::GetInstancePtr()->GetCameraAngle();
    }
    else if (SceneFlag == CHARACTER_SCENE)
    {
        CameraAngle[0] = -84.5f;
        CameraAngle[1] = 0.0f;
        CameraAngle[2] = -75.0f;
        CameraPosition[0] = 9758.93f;
        CameraPosition[1] = 18913.11f;
        CameraPosition[2] = 675.5f;
    }
    else
    {
        CameraAngle[0] = -48.5f;
    }

    CameraAngle[0] += EarthQuake;
}

/**
 * @brief Updates custom camera distance based on terrain flags.
 */
static void UpdateCustomCameraDistance()
{
    int iIndex = TERRAIN_INDEX((Hero->PositionX), (Hero->PositionY));

    if ((TerrainWall[iIndex] & TW_CAMERA_UP) == TW_CAMERA_UP)
    {
        if (g_fCameraCustomDistance <= CUSTOM_CAMERA_DISTANCE1)
        {
            g_fCameraCustomDistance += 10;
        }
    }
    else
    {
        if (g_fCameraCustomDistance > 0)
        {
            g_fCameraCustomDistance -= 10;
        }
    }
}

/**
 * @brief Updates camera distance based on world and camera level.
 */
static void UpdateCameraDistance()
{
    if (gMapManager.WorldActive == 5)
    {
        CameraAngle[0] += sinf(WorldTime * 0.0005f) * 2.f;
        CameraAngle[1] += sinf(WorldTime * 0.0008f) * 2.5f;
    }
    else if (CCameraMove::GetInstancePtr()->IsTourMode())
    {
        CameraDistanceTarget = 1100.f * CCameraMove::GetInstancePtr()->GetCurrentCameraDistanceLevel() * 0.1f;
        CameraDistance = CameraDistanceTarget;
    }
    else
    {
        if (gMapManager.InBattleCastle())
        {
            CameraDistanceTarget = 1100.f;
            CameraDistance = CameraDistanceTarget;
        }
        else
        {
            switch (g_shCameraLevel)
            {
            case 0: CameraDistanceTarget = 1000.f; break;
            case 1: CameraDistanceTarget = 1100.f; break;
            case 2: CameraDistanceTarget = 1200.f; break;
            case 3: CameraDistanceTarget = 1300.f; break;
            case 4: CameraDistanceTarget = 1400.f; break;
            case 5: CameraDistanceTarget = g_Direction.m_fCameraViewFar; break;
            }
            CameraDistance += (CameraDistanceTarget - CameraDistance) / 3;
        }
    }
}

/**
 * @brief Sets field of view based on world and tour mode.
 */
static void SetCameraFOV()
{
    if (gMapManager.WorldActive == WD_73NEW_LOGIN_SCENE
        && CCameraMove::GetInstancePtr()->IsTourMode())
    {
        CameraFOV = 65.0f;
    }
    else
    {
        CameraFOV = 30.f;
    }
}

/**
 * @brief Handles editor-mode camera and movement controls.
 * @return True if editor movement occurred
 */
#ifdef ENABLE_EDIT2
static bool HandleEditorMode()
{
    bool EditMove = false;

    if (!g_pUIManager->IsInputEnable())
    {
        // Handle camera angle rotation
        if (HIBYTE(GetAsyncKeyState(VK_INSERT)) == 128)
            CameraAngle[2] += 15;
        if (HIBYTE(GetAsyncKeyState(VK_DELETE)) == 128)
            CameraAngle[2] -= 15;
        if (HIBYTE(GetAsyncKeyState(VK_HOME)) == 128)
            CameraAngle[2] = -45;

        CameraAngle[2] = fmodf(CameraAngle[2] + 360.0f, 360.0f) - 360.0f;

        // Handle movement input
        vec3_t p1, p2;
        Vector(0.f, 0.f, 0.f, p1);
        FLOAT Velocity = TERRAIN_SCALE * 1.25f * FPS_ANIMATION_FACTOR;

        struct KeyMapping {
            int vKey;
            float x;
            float y;
        };

        const KeyMapping keyMappings[] = {
            {VK_LEFT,  -Velocity, -Velocity},
            {VK_RIGHT,  Velocity,  Velocity},
            {VK_UP,    -Velocity,  Velocity},
            {VK_DOWN,   Velocity, -Velocity}
        };

        for (const auto& mapping : keyMappings)
        {
            if (HIBYTE(GetAsyncKeyState(mapping.vKey)) == 128)
            {
                Vector(mapping.x, mapping.y, 0.f, p1);
                EditMove = true;
                break;
            }
        }

        // Apply rotation and movement
        glPushMatrix();
        glLoadIdentity();
        glRotatef(-CameraAngle[2], 0.f, 0.f, 1.f);
        float Matrix[3][4];
        GetOpenGLMatrix(Matrix);
        glPopMatrix();
        VectorRotate(p1, Matrix, p2);
        VectorAdd(Hero->Object.Position, p2, Hero->Object.Position);
    }

    AdjustHeroHeight();

    if (EditMove)
    {
        BYTE PathX[1];
        BYTE PathY[1];
        PathX[0] = (BYTE)(Hero->Object.Position[0] / TERRAIN_SCALE);
        PathY[0] = (BYTE)(Hero->Object.Position[1] / TERRAIN_SCALE);

        SendCharacterMove(Hero->Key, Hero->Object.Angle[2], 1, PathX, PathY, PathX[0], PathY[0]);

        Hero->Path.PathNum = 0;
    }

    return EditMove;
}
#endif //ENABLE_EDIT2

/**
 * @brief Main camera controller - updates camera position, angle, and distance.
 * @return True if camera is locked
 */
bool MoveMainCamera()
{
    bool bLockCamera = false;

    SetCameraFOV();

#ifdef ENABLE_EDIT2
    HandleEditorMode();
#endif

    CameraAngle[0] = 0.f;
    CameraAngle[1] = 0.f;

    if (CameraTopViewEnable)
    {
        CameraViewFar = 3200.f;
        CameraPosition[0] = Hero->Object.Position[0];
        CameraPosition[1] = Hero->Object.Position[1];
        CameraPosition[2] = CameraViewFar;
    }
    else
    {
        CalculateCameraPosition(CameraPosition);
        SetCameraAngle();
        UpdateCustomCameraDistance();
    }

    UpdateCameraDistance();

    return bLockCamera;
}
