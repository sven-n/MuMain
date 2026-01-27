// DefaultCamera.cpp - Legacy third-person follow camera implementation
// Extracted from CameraUtility.cpp

#include "stdafx.h"
#include "DefaultCamera.h"
#include "CameraProjection.h"
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

// External variable declarations
extern short g_shCameraLevel;
extern float g_fSpecialHeight;

DefaultCamera::DefaultCamera(CameraState& state)
    : m_State(state)
{
}

void DefaultCamera::Reset()
{
    m_State.Reset();
}

bool DefaultCamera::Update()
{
    bool bLockCamera = false;

    SetCameraFOV();

#ifdef ENABLE_EDIT2
    HandleEditorMode();
#endif

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

    return bLockCamera;
}

void DefaultCamera::CalculateCameraViewFar()
{
    if (battleCastle::InBattleCastle2(Hero->Object.Position))
    {
        m_State.ViewFar = 3000.f;
        return;
    }

    if (gMapManager.InBattleCastle() && SceneFlag == MAIN_SCENE)
    {
        m_State.ViewFar = 2500.f;
        return;
    }

    if (gMapManager.WorldActive == WD_51HOME_6TH_CHAR)
    {
        m_State.ViewFar = 2800.f * 1.15f;
        return;
    }

    if (gMapManager.IsPKField() || IsDoppelGanger2())
    {
        m_State.ViewFar = 3700.0f;
        return;
    }

    // Handle camera level based view distance
    switch (g_shCameraLevel)
    {
    case 0:
        if (SceneFlag == LOG_IN_SCENE)
        {
            // Use existing value - don't change m_State.ViewFar
        }
        else if (SceneFlag == CHARACTER_SCENE)
        {
            m_State.ViewFar = 3500.f;
        }
        else if (g_Direction.m_CKanturu.IsMayaScene())
        {
            m_State.ViewFar = 2000.f * 10.0f * 0.115f;
        }
        else
        {
            m_State.ViewFar = 2000.f;
        }
        break;
    case 1: m_State.ViewFar = 2500.f; break;
    case 2: m_State.ViewFar = 2600.f; break;
    case 3: m_State.ViewFar = 2950.f; break;
    case 4:
    case 5: m_State.ViewFar = 3200.f; break;
    default: m_State.ViewFar = 2000.f; break;
    }
}

void DefaultCamera::AdjustHeroHeight()
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

void DefaultCamera::CalculateCameraPosition()
{
    int iIndex = TERRAIN_INDEX((Hero->PositionX), (Hero->PositionY));
    vec3_t Position, TransformPosition;
    float Matrix[3][4];

    CalculateCameraViewFar();

    Vector(0.f, -m_State.Distance, 0.f, Position);
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
    m_State.Position[2] += m_State.Distance - 150.f;

    // Apply custom camera distance for special terrain
    if (m_State.CustomDistance != 0.f)
    {
        vec3_t angle = { 0.f, 0.f, -45.f };
        Vector(0.f, m_State.CustomDistance, 0.f, Position);
        AngleMatrix(angle, Matrix);
        VectorIRotate(Position, Matrix, TransformPosition);
        VectorAdd(m_State.Position, TransformPosition, m_State.Position);
    }
}

void DefaultCamera::SetCameraAngle()
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

    m_State.Angle[0] += EarthQuake;
}

void DefaultCamera::UpdateCustomCameraDistance()
{
    int iIndex = TERRAIN_INDEX((Hero->PositionX), (Hero->PositionY));

    if ((TerrainWall[iIndex] & TW_CAMERA_UP) == TW_CAMERA_UP)
    {
        if (m_State.CustomDistance <= CUSTOM_CAMERA_DISTANCE1)
        {
            m_State.CustomDistance += 10;
        }
    }
    else
    {
        if (m_State.CustomDistance > 0)
        {
            m_State.CustomDistance -= 10;
        }
    }
}

void DefaultCamera::UpdateCameraDistance()
{
    if (gMapManager.WorldActive == 5)
    {
        m_State.Angle[0] += sinf(WorldTime * 0.0005f) * 2.f;
        m_State.Angle[1] += sinf(WorldTime * 0.0008f) * 2.5f;
    }
    else if (CCameraMove::GetInstancePtr()->IsTourMode())
    {
        m_State.DistanceTarget = 1100.f * CCameraMove::GetInstancePtr()->GetCurrentCameraDistanceLevel() * 0.1f;
        m_State.Distance = m_State.DistanceTarget;
    }
    else
    {
        if (gMapManager.InBattleCastle())
        {
            m_State.DistanceTarget = 1100.f;
            m_State.Distance = m_State.DistanceTarget;
        }
        else
        {
            switch (g_shCameraLevel)
            {
            case 0: m_State.DistanceTarget = 1000.f; break;
            case 1: m_State.DistanceTarget = 1100.f; break;
            case 2: m_State.DistanceTarget = 1200.f; break;
            case 3: m_State.DistanceTarget = 1300.f; break;
            case 4: m_State.DistanceTarget = 1400.f; break;
            case 5: m_State.DistanceTarget = g_Direction.m_fCameraViewFar; break;
            }
            m_State.Distance += (m_State.DistanceTarget - m_State.Distance) / 3;
        }
    }
}

void DefaultCamera::SetCameraFOV()
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

#ifdef ENABLE_EDIT2
void DefaultCamera::HandleEditorMode()
{
    bool EditMove = false;

    if (!g_pUIManager->IsInputEnable())
    {
        // Handle camera angle rotation
        if (HIBYTE(GetAsyncKeyState(VK_INSERT)) == 128)
            m_State.Angle[2] += 15;
        if (HIBYTE(GetAsyncKeyState(VK_DELETE)) == 128)
            m_State.Angle[2] -= 15;
        if (HIBYTE(GetAsyncKeyState(VK_HOME)) == 128)
            m_State.Angle[2] = -45;

        m_State.Angle[2] = fmodf(m_State.Angle[2] + 360.0f, 360.0f) - 360.0f;

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
        glRotatef(-m_State.Angle[2], 0.f, 0.f, 1.f);
        float Matrix[3][4];
        CameraProjection::GetOpenGLMatrix(Matrix);
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
}
#endif //ENABLE_EDIT2
