// LegacyCamera.cpp — Direct port of main branch MoveMainCamera()
// This is the EXACT original camera code for A/B performance comparison.

#include "stdafx.h"
#include "LegacyCamera.h"
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

// External variable declarations (same as original CameraUtility.cpp on main)
extern short g_shCameraLevel;
extern float g_fSpecialHeight;
extern float CameraDistanceTarget;
extern float CameraDistance;

LegacyCamera::LegacyCamera(CameraState& state)
    : m_State(state)
    , m_Config(CameraConfig::ForMainSceneDefaultCamera())
{
}

void LegacyCamera::Reset()
{
    m_State.Reset();
}

void LegacyCamera::OnActivate(const CameraState& previousState)
{
    m_Config = CameraConfig::ForMainSceneDefaultCamera();
}

void LegacyCamera::OnDeactivate()
{
}

// ========== Original main branch functions (direct ports) ==========

float LegacyCamera::CalculateCameraViewFar()
{
    if (battleCastle::InBattleCastle2(Hero->Object.Position))
        return 3000.f;

    if (gMapManager.InBattleCastle() && SceneFlag == MAIN_SCENE)
        return 2500.f;

    if (gMapManager.WorldActive == WD_51HOME_6TH_CHAR)
        return 2800.f * 1.15f;

    if (gMapManager.IsPKField() || IsDoppelGanger2())
        return 3700.0f;

    switch (g_shCameraLevel)
    {
    case 0:
        if (SceneFlag == LOG_IN_SCENE)
            return m_State.ViewFar; // Use existing value
        else if (SceneFlag == CHARACTER_SCENE)
            return 3500.f;
        else if (g_Direction.m_CKanturu.IsMayaScene())
            return 2000.f * 10.0f * 0.115f;
        else
            return 2000.f;
    case 1: return 2500.f;
    case 2: return 2600.f;
    case 3: return 2950.f;
    case 4:
    case 5: return 3200.f;
    default: return 2000.f;
    }
}

void LegacyCamera::AdjustHeroHeight()
{
    if (gMapManager.InChaosCastle() == false || !Hero->Object.m_bActionStart)
    {
        if (gMapManager.WorldActive == WD_39KANTURU_3RD && Hero->Object.m_bActionStart)
        {
            // No-op
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

void LegacyCamera::CalculateCameraPosition()
{
    int iIndex = TERRAIN_INDEX((Hero->PositionX), (Hero->PositionY));
    vec3_t Position, TransformPosition;
    float Matrix[3][4];

    m_State.ViewFar = CalculateCameraViewFar();

    Vector(0.f, -CameraDistance, 0.f, Position);
    AngleMatrix(m_State.Angle, Matrix);
    VectorIRotate(Position, Matrix, TransformPosition);

    if (SceneFlag == MAIN_SCENE)
    {
        g_pCatapultWindow->GetCameraPos(Position);
    }
    else if (CCameraMove::GetInstancePtr()->IsTourMode())
    {
        CCameraMove::GetInstancePtr()->UpdateTourWayPoint();
        CCameraMove::GetInstancePtr()->GetCurrentCameraPos(Position);
        m_State.ViewFar = 390.f * CCameraMove::GetInstancePtr()->GetCurrentCameraDistanceLevel();
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

    VectorAdd(Position, TransformPosition, m_State.Position);

    if (gMapManager.InBattleCastle())
    {
        m_State.Position[2] = 255.f;
    }
    else if (!CCameraMove::GetInstancePtr()->IsTourMode())
    {
        m_State.Position[2] = Hero->Object.Position[2];
    }

    if ((TerrainWall[iIndex] & TW_HEIGHT) == TW_HEIGHT)
    {
        m_State.Position[2] = g_fSpecialHeight = 1200.f + 1;
    }
    m_State.Position[2] += CameraDistance - 150.f;
}

void LegacyCamera::SetCameraAngle()
{
    if (CCameraMove::GetInstancePtr()->IsTourMode())
    {
        CCameraMove::GetInstancePtr()->SetAngleFrustum(-112.5f);
        m_State.Angle[0] = CCameraMove::GetInstancePtr()->GetAngleFrustum();
        m_State.Angle[1] = 0.0f;
        m_State.Angle[2] = CCameraMove::GetInstancePtr()->GetCameraAngle();
    }
    else if (SceneFlag == CHARACTER_SCENE)
    {
        m_State.Angle[0] = -84.5f;
        m_State.Angle[1] = 0.0f;
        m_State.Angle[2] = -75.0f;
        m_State.Position[0] = 9758.93f;
        m_State.Position[1] = 18913.11f;
        m_State.Position[2] = 675.5f;
    }
    else
    {
        m_State.Angle[0] = -48.5f;
    }

    extern float EarthQuake;
    m_State.Angle[0] += EarthQuake;
}

void LegacyCamera::UpdateCustomCameraDistance()
{
    extern float g_fCameraCustomDistance;
    int iIndex = TERRAIN_INDEX((Hero->PositionX), (Hero->PositionY));

    if ((TerrainWall[iIndex] & TW_CAMERA_UP) == TW_CAMERA_UP)
    {
        if (g_fCameraCustomDistance <= CUSTOM_CAMERA_DISTANCE1)
            g_fCameraCustomDistance += 10;
    }
    else
    {
        if (g_fCameraCustomDistance > 0)
            g_fCameraCustomDistance -= 10;
    }
}

void LegacyCamera::UpdateCameraDistance()
{
    if (gMapManager.WorldActive == 5)
    {
        m_State.Angle[0] += sinf(WorldTime * 0.0005f) * 2.f;
        m_State.Angle[1] += sinf(WorldTime * 0.0008f) * 2.5f;
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

void LegacyCamera::SetCameraFOV()
{
    if (gMapManager.WorldActive == WD_73NEW_LOGIN_SCENE
        && CCameraMove::GetInstancePtr()->IsTourMode())
    {
        m_State.FOV = 65.0f;
    }
    else
    {
        m_State.FOV = 30.f;
    }
}

// ========== Main Update — exact replica of main branch MoveMainCamera() ==========

bool LegacyCamera::Update()
{
    SetCameraFOV();

    m_State.Angle[0] = 0.f;
    m_State.Angle[1] = 0.f;

    if (m_State.TopViewEnable)
    {
        m_State.ViewFar = 3200.f;
        m_State.Position[0] = Hero->Object.Position[0];
        m_State.Position[1] = Hero->Object.Position[1];
        m_State.Position[2] = m_State.ViewFar;
    }
    else
    {
        CalculateCameraPosition();
        SetCameraAngle();
        UpdateCustomCameraDistance();
    }

    UpdateCameraDistance();

    // Sync to g_Camera (same as original — globals were the camera state)
    // No frustum building — that's the whole point of Legacy mode

    return false;
}
