// CameraUtility.cpp - Camera movement and positioning utilities
// Extracted from ZzzScene.cpp as part of scene refactoring

#include "stdafx.h"
#include "CameraUtility.h"
#include "../ZzzScene.h"
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

bool MoveMainCamera()
{
    bool bLockCamera = false;

    if (
        gMapManager.WorldActive == WD_73NEW_LOGIN_SCENE
        && CCameraMove::GetInstancePtr()->IsTourMode())
        CameraFOV = 65.0f;
    else
        CameraFOV = 30.f;

#ifdef ENABLE_EDIT2
    {
        bool EditMove = false;
        if (!g_pUIManager->IsInputEnable())
        {
            if (HIBYTE(GetAsyncKeyState(VK_INSERT)) == 128)
                CameraAngle[2] += 15;
            if (HIBYTE(GetAsyncKeyState(VK_DELETE)) == 128)
                CameraAngle[2] -= 15;
            if (HIBYTE(GetAsyncKeyState(VK_HOME)) == 128)
                CameraAngle[2] = -45;

            if (CameraAngle[2] < -360)
                CameraAngle[2] += 360;
            else if (CameraAngle[2] > 0)
                CameraAngle[2] -= 360;


            vec3_t p1, p2;
            Vector(0.f, 0.f, 0.f, p1);
            FLOAT Velocity = sqrtf(TERRAIN_SCALE * TERRAIN_SCALE) * 1.25f * FPS_ANIMATION_FACTOR;

            if (HIBYTE(GetAsyncKeyState(VK_LEFT)) == 128)
            {
                Vector(-Velocity, -Velocity, 0.f, p1);
                EditMove = true;
            }
            if (HIBYTE(GetAsyncKeyState(VK_RIGHT)) == 128)
            {
                Vector(Velocity, Velocity, 0.f, p1);
                EditMove = true;
            }
            if (HIBYTE(GetAsyncKeyState(VK_UP)) == 128)
            {
                Vector(-Velocity, Velocity, 0.f, p1);
                EditMove = true;
            }
            if (HIBYTE(GetAsyncKeyState(VK_DOWN)) == 128)
            {
                Vector(Velocity, -Velocity, 0.f, p1);
                EditMove = true;
            }

            glPushMatrix();
            glLoadIdentity();
            glRotatef(-CameraAngle[2], 0.f, 0.f, 1.f);
            float Matrix[3][4];
            GetOpenGLMatrix(Matrix);
            glPopMatrix();
            VectorRotate(p1, Matrix, p2);
            VectorAdd(Hero->Object.Position, p2, Hero->Object.Position);
        }

        if (gMapManager.InChaosCastle() == false || !Hero->Object.m_bActionStart)
        {
            if (gMapManager.WorldActive == WD_39KANTURU_3RD && Hero->Object.m_bActionStart)
            {
            }
            else
                if (gMapManager.WorldActive == -1 || Hero->Helper.Type != MODEL_HORN_OF_DINORANT || Hero->SafeZone)
                {
                    Hero->Object.Position[2] = RequestTerrainHeight(Hero->Object.Position[0], Hero->Object.Position[1]);
                }
                else
                {
                    if (gMapManager.WorldActive == WD_8TARKAN || gMapManager.WorldActive == WD_10HEAVEN)
                        Hero->Object.Position[2] = RequestTerrainHeight(Hero->Object.Position[0], Hero->Object.Position[1]) + 90.f;
                    else
                        Hero->Object.Position[2] = RequestTerrainHeight(Hero->Object.Position[0], Hero->Object.Position[1]) + 30.f;
                }
        }

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

    CameraAngle[0] = 0.f;
    CameraAngle[1] = 0.f;

    if (CameraTopViewEnable)
    {
        CameraViewFar = 3200.f;
        //CameraViewFar = 60000.f;
        CameraPosition[0] = Hero->Object.Position[0];
        CameraPosition[1] = Hero->Object.Position[1];
        CameraPosition[2] = CameraViewFar;
    }
    else
    {
        int iIndex = TERRAIN_INDEX((Hero->PositionX), (Hero->PositionY));
        vec3_t Position, TransformPosition;
        float Matrix[3][4];

        if (battleCastle::InBattleCastle2(Hero->Object.Position))
        {
            CameraViewFar = 3000.f;
        }
        else if (gMapManager.InBattleCastle() && SceneFlag == MAIN_SCENE)
        {
            CameraViewFar = 2500.f;
        }
        else if (gMapManager.WorldActive == WD_51HOME_6TH_CHAR)
        {
            CameraViewFar = 2800.f * 1.15f;
        }
        else if (gMapManager.IsPKField() || IsDoppelGanger2())
        {
            CameraViewFar = 3700.0f;
        }
        else
        {
            switch (g_shCameraLevel)
            {
            case 0:
                if (SceneFlag == LOG_IN_SCENE)
                {
                }
                else if (SceneFlag == CHARACTER_SCENE)
                {
                    CameraViewFar = 3500.f;
                }
                else if (g_Direction.m_CKanturu.IsMayaScene())
                {
                    CameraViewFar = 2000.f * 10.0f * 0.115f;
                }
                else
                {
                    CameraViewFar = 2000.f;
                }
                break;
            case 1: CameraViewFar = 2500.f; break;
            case 2: CameraViewFar = 2600.f; break;
            case 3: CameraViewFar = 2950.f; break;
            case 5:
            case 4: CameraViewFar = 3200.f; break;
            }
        }

        Vector(0.f, -CameraDistance, 0.f, Position);//-750
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
            vec3_t temp = { 0.0f,0.0f,-100.0f };
            VectorAdd(TransformPosition, temp, TransformPosition);
        }

        VectorAdd(Position, TransformPosition, CameraPosition);

        if (gMapManager.InBattleCastle() == true)
        {
            CameraPosition[2] = 255.f;//700
        }
        else if (CCameraMove::GetInstancePtr()->IsTourMode());
        else
        {
            CameraPosition[2] = Hero->Object.Position[2];//700
        }

        if ((TerrainWall[iIndex] & TW_HEIGHT) == TW_HEIGHT)
        {
            CameraPosition[2] = g_fSpecialHeight = 1200.f + 1;
        }
        CameraPosition[2] += CameraDistance - 150.f;//700

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

        if (g_fCameraCustomDistance > 0)
        {
            vec3_t angle = { 0.f, 0.f, -45.f };
            Vector(0.f, g_fCameraCustomDistance, 0.f, Position);
            AngleMatrix(angle, Matrix);
            VectorIRotate(Position, Matrix, TransformPosition);
            VectorAdd(CameraPosition, TransformPosition, CameraPosition);
        }
        else if (g_fCameraCustomDistance < 0)
        {
            vec3_t angle = { 0.f, 0.f, -45.f };
            Vector(0.f, g_fCameraCustomDistance, 0.f, Position);
            AngleMatrix(angle, Matrix);
            VectorIRotate(Position, Matrix, TransformPosition);
            VectorAdd(CameraPosition, TransformPosition, CameraPosition);
        }
    }
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
            case 0: CameraDistanceTarget = 1000.f; CameraDistance += (CameraDistanceTarget - CameraDistance) / 3; break;
            case 1: CameraDistanceTarget = 1100.f; CameraDistance += (CameraDistanceTarget - CameraDistance) / 3; break;
            case 2: CameraDistanceTarget = 1200.f; CameraDistance += (CameraDistanceTarget - CameraDistance) / 3; break;
            case 3: CameraDistanceTarget = 1300.f; CameraDistance += (CameraDistanceTarget - CameraDistance) / 3; break;
            case 4: CameraDistanceTarget = 1400.f; CameraDistance += (CameraDistanceTarget - CameraDistance) / 3; break;
            case 5: CameraDistanceTarget = g_Direction.m_fCameraViewFar; CameraDistance += (CameraDistanceTarget - CameraDistance) / 3; break;
            }
        }
    }

    return bLockCamera;
}
