// DefaultCamera.cpp - Legacy third-person follow camera implementation
// Extracted from CameraUtility.cpp

#include "stdafx.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
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

#ifdef _EDITOR
// DevEditor config override functions (global scope required for extern "C")
extern "C" bool DevEditor_IsConfigOverrideEnabled();
extern "C" void DevEditor_GetCameraConfig(float* outFOV, float* outNearPlane, float* outFarPlane, float* outTerrainCullRange);
#endif

DefaultCamera::DefaultCamera(CameraState& state)
    : m_State(state)
    , m_Config(CameraConfig::ForGameplay())  // Phase 1: Initialize with gameplay config
{
    // Phase 5: Initialize frustum cache to force first update
    m_LastFrustumPosition[0] = m_LastFrustumPosition[1] = m_LastFrustumPosition[2] = 0.0f;
    m_LastFrustumAngle[0] = m_LastFrustumAngle[1] = m_LastFrustumAngle[2] = 0.0f;
    m_LastFrustumViewFar = 0.0f;

    // Phase 5: Initialize DevEditor config cache to force first update
    m_LastEditorFOV = 0.0f;
    m_LastEditorFarPlane = 0.0f;
    m_LastEditorNearPlane = 0.0f;
    m_LastEditorTerrainCullRange = 0.0f;
}

bool DefaultCamera::IsHeroValid() const
{
    extern CHARACTER* Hero;
    return (Hero != nullptr && Hero->Object.Live);
}

void DefaultCamera::Reset()
{
    m_State.Reset();
    // Phase 5: Reset scene tracking to force config reload on next Update()
    m_LastSceneFlag = -1;
    m_bFreeCameraMode = false;
}

void DefaultCamera::ResetForScene(EGameScene scene)
{
    // Phase 5: Proper scene-specific reset using switch statement
    switch (scene)
    {
        case CHARACTER_SCENE:
        {
            // Load CharacterScene config
            m_Config = CameraConfig::ForCharacterScene();

            // FORCE all CharacterScene values DIRECTLY to state
            m_State.ViewFar = m_Config.farPlane;      // 4100
            m_State.FOV = 30.0f;                       // OpenGL perspective FOV
            m_State.TopViewEnable = false;

            // Set static CharacterScene position
            m_State.Angle[0] = -84.5f;
            m_State.Angle[1] = 0.0f;
            m_State.Angle[2] = -75.0f;
            m_State.Position[0] = 9758.93f;
            m_State.Position[1] = 18913.11f;
            m_State.Position[2] = 675.5f;
            break;
        }

        case MAIN_SCENE:
        {
            // Load MainScene config
            m_Config = CameraConfig::ForGameplay();

            // Set MainScene defaults
            m_State.ViewFar = m_Config.farPlane;      // 2400
            m_State.FOV = 30.0f;
            m_State.TopViewEnable = false;

            // Position will be calculated from Hero in Update()
            if (IsHeroValid())
            {
                extern CHARACTER* Hero;
                m_State.Position[0] = Hero->Object.Position[0];
                m_State.Position[1] = Hero->Object.Position[1];
                m_State.Position[2] = Hero->Object.Position[2];
            }
            break;
        }

        case LOG_IN_SCENE:
        case SERVER_LIST_SCENE:
        case WEBZEN_SCENE:
        case LOADING_SCENE:
        default:
        {
            // Use gameplay config as default
            m_Config = CameraConfig::ForGameplay();
            m_State.ViewFar = m_Config.farPlane;
            m_State.FOV = 30.0f;
            m_State.TopViewEnable = false;
            break;
        }
    }

    // Force frustum rebuild
    m_LastFrustumPosition[0] = m_LastFrustumPosition[1] = m_LastFrustumPosition[2] = -999999.0f;
    m_LastFrustumAngle[0] = m_LastFrustumAngle[1] = m_LastFrustumAngle[2] = -999999.0f;
    m_LastFrustumViewFar = -999999.0f;

#ifdef _EDITOR
    // Clear DevEditor cache
    m_LastEditorFOV = -1.0f;
    m_LastEditorFarPlane = -1.0f;
    m_LastEditorNearPlane = -1.0f;
    m_LastEditorTerrainCullRange = -1.0f;
#endif
}

void DefaultCamera::OnActivate(const CameraState& previousState)
{
    // Phase 5: Load scene-specific camera config
    extern EGameScene SceneFlag;
    if (SceneFlag == CHARACTER_SCENE)
    {
        m_Config = CameraConfig::ForCharacterScene();
    }
    else
    {
        // Use gameplay config for MainScene and LoginScene
        // LoginScene uses CCameraMove tour mode which overrides everything anyway
        m_Config = CameraConfig::ForGameplay();
    }

    // When returning to DefaultCamera, reset to default state
    // Only inherit distance for smoother transition
    m_State.Distance = previousState.Distance;
    m_State.DistanceTarget = previousState.DistanceTarget;

    // Reset ALL angles to default
    // DefaultCamera uses fixed angle of -45 degrees (isometric view)
    m_State.Angle[0] = 0.0f;
    m_State.Angle[1] = 0.0f;
    m_State.Angle[2] = -45.0f;

    // Note: DefaultCamera maintains Angle[2] = -45 throughout gameplay
    // The camera positioning is relative to character's facing direction

    // Phase 5: If Hero is invalid (LoginScene/CharacterScene), use scene-specific position
    if (!IsHeroValid())
    {
        // CharacterScene has hardcoded position in SetCameraAngle()
        if (SceneFlag == CHARACTER_SCENE)
        {
            m_State.Angle[0] = -84.5f;
            m_State.Angle[1] = 0.0f;
            m_State.Angle[2] = -75.0f;
            m_State.Position[0] = 9758.93f;
            m_State.Position[1] = 18913.11f;
            m_State.Position[2] = 675.5f;
        }
        else if (SceneFlag == LOG_IN_SCENE)
        {
            // Phase 5 fix: Initialize to LoginScene default camera position
            // Tour mode will update this in Update(), but we need valid initial position
            // for first frame frustum. These values match old LoginScene WALK_PATHS[0]
            // after transformation: {0, -1000, 500} with angles {-80, 0, 0}
            if (CCameraMove::GetInstancePtr()->IsTourMode())
            {
                vec3_t tourPos;
                CCameraMove::GetInstancePtr()->GetCurrentCameraPos(tourPos);
                // Check if tour position is valid (not at origin)
                if (tourPos[0] != 0.0f || tourPos[1] != 0.0f || tourPos[2] != 0.0f)
                {
                    m_State.Position[0] = tourPos[0];
                    m_State.Position[1] = tourPos[1];
                    m_State.Position[2] = tourPos[2];
                    m_State.Angle[0] = CCameraMove::GetInstancePtr()->GetAngleFrustum();
                    m_State.Angle[1] = 0.0f;
                    m_State.Angle[2] = CCameraMove::GetInstancePtr()->GetCameraAngle();
                }
                else
                {
                    // Fallback: Use reasonable position similar to CharacterScene
                    // LoginScene world is at WD_73NEW_LOGIN_SCENE, camera should see the scene
                    m_State.Position[0] = 5000.0f;  // Center-ish of typical MU map
                    m_State.Position[1] = 5000.0f;
                    m_State.Position[2] = 1000.0f;  // Higher up for better view
                    m_State.Angle[0] = -60.0f;  // Looking down
                    m_State.Angle[1] = 0.0f;
                    m_State.Angle[2] = 0.0f;
                }
            }
        }
        // else: maintain previous position if no Hero and not in recognized scene
    }
    else if (SceneFlag == MAIN_SCENE)
    {
        // Phase 5 fix: In MainScene, don't use previous camera state
        // Position will be calculated in Update() based on Hero
        // Just initialize to Hero position to prevent one frame of wrong frustum
        extern CHARACTER* Hero;
        m_State.Position[0] = Hero->Object.Position[0];
        m_State.Position[1] = Hero->Object.Position[1];
        m_State.Position[2] = Hero->Object.Position[2];
    }

    // Phase 3 fix: Initialize frustum immediately on activation
    UpdateFrustum();
}

void DefaultCamera::OnDeactivate()
{
    // Nothing to cleanup
}

bool DefaultCamera::Update()
{
    // Phase 5: Detect scene transitions and properly reset for new scene
    extern EGameScene SceneFlag;
    if (m_LastSceneFlag != (int)SceneFlag)
    {
        // Scene changed - use dedicated reset function
        m_LastSceneFlag = (int)SceneFlag;
        ResetForScene(SceneFlag);
    }

    // Phase 5: Handle WASD+QE free camera movement (toggle with F8)
    if (HIBYTE(GetAsyncKeyState(VK_F8)) == 128)
    {
        static bool bF8Pressed = false;
        if (!bF8Pressed)
        {
            m_bFreeCameraMode = !m_bFreeCameraMode;
            bF8Pressed = true;
        }
    }
    else
    {
        static bool bF8Pressed = false;
        bF8Pressed = false;
    }

    if (m_bFreeCameraMode)
    {
        HandleFreeCameraMovement();
    }

    bool bLockCamera = false;

    // Initialize default angles (SetCameraAngle will override these for specific scenes)
    m_State.Angle[0] = 0.f;
    m_State.Angle[1] = 0.f;
    m_State.Angle[2] = -45.f;

    SetCameraFOV();

#ifdef ENABLE_EDIT2
    HandleEditorMode();
#endif

    if (m_State.TopViewEnable)
    {
        m_State.ViewFar = 3200.f;
        // Phase 5: NULL check for Hero (may not exist in LoginScene/CharacterScene)
        if (IsHeroValid())
        {
            m_State.Position[0] = Hero->Object.Position[0];
            m_State.Position[1] = Hero->Object.Position[1];
            m_State.Position[2] = m_State.ViewFar;
        }
        // else: maintain current position (fallback for non-MainScene)
    }
    else
    {
        CalculateCameraPosition();
        SetCameraAngle();
        UpdateCustomCameraDistance();
    }

    UpdateCameraDistance();

    // Phase 5 fix: Update frustum only when camera state actually changes
    // This avoids expensive frustum rebuild every frame (20-25% performance gain)
    if (NeedsFrustumUpdate())
    {
        UpdateFrustum();
    }

    // Phase 5: Debug text rendering to verify camera values
    {
        g_pRenderText->SetFont(g_hFixFont);
        g_pRenderText->SetTextColor(255, 255, 0, 255);  // Yellow text
        g_pRenderText->SetBgColor(0, 0, 0, 180);        // Semi-transparent black background

        wchar_t debugText[256];
        int yPos = 10;
        const int lineHeight = 15;

        // Camera type and scene
        swprintf(debugText, 256, L"Camera: DefaultCamera | Scene: %d | FreeMode: %s",
                 (int)SceneFlag, m_bFreeCameraMode ? L"ON" : L"OFF");
        g_pRenderText->RenderText(10, yPos, debugText);
        yPos += lineHeight;

        // State values
        swprintf(debugText, 256, L"State.ViewFar: %.0f | State.FOV: %.1f", m_State.ViewFar, m_State.FOV);
        g_pRenderText->RenderText(10, yPos, debugText);
        yPos += lineHeight;

        // Config values
        swprintf(debugText, 256, L"Config.farPlane: %.0f | Config.fov: %.1f", m_Config.farPlane, m_Config.fov);
        g_pRenderText->RenderText(10, yPos, debugText);
        yPos += lineHeight;

        // Near plane and culling
        swprintf(debugText, 256, L"Config.nearPlane: %.0f | Config.terrainCullRange: %.0f",
                 m_Config.nearPlane, m_Config.terrainCullRange);
        g_pRenderText->RenderText(10, yPos, debugText);
    }

    return bLockCamera;
}

void DefaultCamera::HandleFreeCameraMovement()
{
    // Phase 5: WASD+QE free camera movement
    const float moveSpeed = 50.0f;
    const float rotSpeed = 2.0f;

    // Forward/Backward: W/S
    if (HIBYTE(GetAsyncKeyState('W')) == 128)
    {
        float angle = m_State.Angle[2] * M_PI / 180.0f;
        m_State.Position[0] += sinf(angle) * moveSpeed;
        m_State.Position[1] += cosf(angle) * moveSpeed;
    }
    if (HIBYTE(GetAsyncKeyState('S')) == 128)
    {
        float angle = m_State.Angle[2] * M_PI / 180.0f;
        m_State.Position[0] -= sinf(angle) * moveSpeed;
        m_State.Position[1] -= cosf(angle) * moveSpeed;
    }

    // Strafe Left/Right: A/D
    if (HIBYTE(GetAsyncKeyState('A')) == 128)
    {
        float angle = (m_State.Angle[2] + 90.0f) * M_PI / 180.0f;
        m_State.Position[0] += sinf(angle) * moveSpeed;
        m_State.Position[1] += cosf(angle) * moveSpeed;
    }
    if (HIBYTE(GetAsyncKeyState('D')) == 128)
    {
        float angle = (m_State.Angle[2] - 90.0f) * M_PI / 180.0f;
        m_State.Position[0] += sinf(angle) * moveSpeed;
        m_State.Position[1] += cosf(angle) * moveSpeed;
    }

    // Up/Down: E/Q
    if (HIBYTE(GetAsyncKeyState('E')) == 128)
    {
        m_State.Position[2] += moveSpeed;
    }
    if (HIBYTE(GetAsyncKeyState('Q')) == 128)
    {
        m_State.Position[2] -= moveSpeed;
    }

    // Mouse look: Arrow keys for rotation
    if (HIBYTE(GetAsyncKeyState(VK_UP)) == 128)
    {
        m_State.Angle[0] += rotSpeed;
    }
    if (HIBYTE(GetAsyncKeyState(VK_DOWN)) == 128)
    {
        m_State.Angle[0] -= rotSpeed;
    }
    if (HIBYTE(GetAsyncKeyState(VK_LEFT)) == 128)
    {
        m_State.Angle[2] += rotSpeed;
    }
    if (HIBYTE(GetAsyncKeyState(VK_RIGHT)) == 128)
    {
        m_State.Angle[2] -= rotSpeed;
    }

    // Force frustum update when in free camera mode
    UpdateFrustum();
}

void DefaultCamera::CalculateCameraViewFar()
{
    // Phase 5: Use config's farPlane as single source of truth
    // Apply multipliers for specific scenes/maps that need different view distances
    float baseFarPlane = m_Config.farPlane;

    // Phase 5: NULL check for Hero
    if (IsHeroValid() && battleCastle::InBattleCastle2(Hero->Object.Position))
    {
        m_State.ViewFar = baseFarPlane * 1.25f;  // 25% more for BattleCastle2
        return;
    }

    if (gMapManager.InBattleCastle() && SceneFlag == MAIN_SCENE)
    {
        m_State.ViewFar = baseFarPlane * 1.04f;  // Slightly more for BattleCastle
        return;
    }

    if (gMapManager.WorldActive == WD_51HOME_6TH_CHAR)
    {
        m_State.ViewFar = baseFarPlane * 1.34f;  // 34% more for 6th character home
        return;
    }

    if (gMapManager.IsPKField() || IsDoppelGanger2())
    {
        m_State.ViewFar = baseFarPlane * 1.54f;  // 54% more for PK field
        return;
    }

    // Handle camera level based view distance (zoom levels)
    switch (g_shCameraLevel)
    {
    case 0:
        if (SceneFlag == LOG_IN_SCENE)
        {
            // LoginScene uses its own config with massive far plane
            m_State.ViewFar = m_Config.farPlane;
        }
        else if (SceneFlag == CHARACTER_SCENE)
        {
            // CharacterScene uses its own config (FOV 71, Far 4100)
            m_State.ViewFar = m_Config.farPlane;
        }
        else if (g_Direction.m_CKanturu.IsMayaScene())
        {
            m_State.ViewFar = baseFarPlane * 0.96f;  // Slightly less for Kanturu Maya
        }
        else
        {
            // Default gameplay: use config's farPlane directly
            m_State.ViewFar = baseFarPlane;
        }
        break;
    case 1: m_State.ViewFar = baseFarPlane * 1.04f; break;  // Zoom level 1
    case 2: m_State.ViewFar = baseFarPlane * 1.08f; break;  // Zoom level 2
    case 3: m_State.ViewFar = baseFarPlane * 1.23f; break;  // Zoom level 3
    case 4:
    case 5: m_State.ViewFar = baseFarPlane * 1.33f; break;  // Zoom level 4-5
    default: m_State.ViewFar = baseFarPlane; break;
    }
}

void DefaultCamera::AdjustHeroHeight()
{
    // Phase 5: NULL check - this function only applies when Hero exists
    if (!IsHeroValid())
        return;

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
    vec3_t Position, TransformPosition;
    float Matrix[3][4];

    CalculateCameraViewFar();

    Vector(0.f, -m_State.Distance, 0.f, Position);
    AngleMatrix(m_State.Angle, Matrix);
    VectorIRotate(Position, Matrix, TransformPosition);

    // Phase 5 fix: Handle tour mode (LoginScene) BEFORE Hero check
    if (CCameraMove::GetInstancePtr()->IsTourMode())
    {
        CCameraMove::GetInstancePtr()->UpdateTourWayPoint();
        CCameraMove::GetInstancePtr()->GetCurrentCameraPos(Position);
        m_State.ViewFar = 390.f * CCameraMove::GetInstancePtr()->GetCurrentCameraDistanceLevel();

        // Tour mode handles camera completely - set position and return
        VectorAdd(Position, TransformPosition, m_State.Position);
        return;
    }

    // Phase 5: NULL check - if Hero invalid and not in tour mode, use fallback
    if (!IsHeroValid())
    {
        // Fallback: Use CharacterScene or LoginScene static positions
        // (SetCameraAngle() sets hardcoded positions for CHARACTER_SCENE)
        return;
    }

    int iIndex = TERRAIN_INDEX((Hero->PositionX), (Hero->PositionY));

    if (SceneFlag == MAIN_SCENE)
    {
        g_pCatapultWindow->GetCameraPos(Position);
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
    // Phase 5: NULL check - custom distance only applies when Hero exists
    if (!IsHeroValid())
        return;

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
        // Phase 5: Keep using 30.0f - this is the OpenGL perspective FOV
        // m_Config.fov (71) is used for frustum plane calculations in BuildFromCamera()
        // These are two different FOV concepts and should not be conflated
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

        // Phase 5: Only apply movement if Hero is valid
        if (IsHeroValid())
        {
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
    }

    AdjustHeroHeight();

    if (EditMove && IsHeroValid())
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

// ========== Phase 1: Configuration & Frustum Management ==========

void DefaultCamera::SetConfig(const CameraConfig& config)
{
    m_Config = config;
    UpdateFrustum();
}

void DefaultCamera::UpdateFrustum()
{
#ifdef _EDITOR
    // Check if DevEditor is overriding config values
    if (DevEditor_IsConfigOverrideEnabled())
    {
        DevEditor_GetCameraConfig(&m_Config.fov, &m_Config.nearPlane, &m_Config.farPlane, &m_Config.terrainCullRange);
    }
#endif

    // Calculate forward and up vectors from current camera state
    vec3_t forward, up, right;

    // DefaultCamera looks at Hero from behind at 45-degree angle
    // Calculate forward as direction from camera to Hero (what we're looking at)
    extern CHARACTER* Hero;
    if (Hero && Hero->Object.Live)
    {
        // Forward points from camera towards Hero
        forward[0] = Hero->Object.Position[0] - m_State.Position[0];
        forward[1] = Hero->Object.Position[1] - m_State.Position[1];
        forward[2] = Hero->Object.Position[2] - m_State.Position[2];
        VectorNormalize(forward);
    }
    else
    {
        // Fallback: Use camera angle to calculate forward direction
        // This handles LoginScene/CharacterScene where Hero doesn't exist
        float Matrix[3][4];
        AngleMatrix(m_State.Angle, Matrix);
        forward[0] = -Matrix[2][0];
        forward[1] = -Matrix[2][1];
        forward[2] = -Matrix[2][2];
        VectorNormalize(forward);
    }

    // Calculate right vector (perpendicular to forward in XY plane)
    vec3_t worldUp = { 0.0f, 0.0f, 1.0f };
    vec3_t forwardTemp, worldUpTemp;
    VectorCopy(forward, forwardTemp);
    VectorCopy(worldUp, worldUpTemp);
    CrossProduct(forwardTemp, worldUpTemp, right);
    VectorNormalize(right);

    // Calculate actual up vector (perpendicular to both forward and right)
    VectorCopy(right, forwardTemp);  // reuse temp
    VectorCopy(forward, worldUpTemp);  // reuse temp
    CrossProduct(forwardTemp, worldUpTemp, up);
    VectorNormalize(up);

    // Build frustum from current configuration
    extern unsigned int WindowWidth;
    extern unsigned int WindowHeight;
    float aspectRatio = (float)WindowWidth / (float)WindowHeight;

    // Use ViewFar for 3D culling distance (varies 2000-3700 based on map/zoom)
    // The Frustum will internally use terrainCullRange for 2D ground projection
    // Phase 5: If DevEditor is overriding, use config.farPlane instead of ViewFar
    float effectiveFarPlane = m_State.ViewFar;
    float effectiveTerrainCullRange = m_Config.terrainCullRange;
#ifdef _EDITOR
    if (DevEditor_IsConfigOverrideEnabled())
    {
        effectiveFarPlane = m_Config.farPlane;
        effectiveTerrainCullRange = m_Config.terrainCullRange;
    }
#endif

    m_Frustum.BuildFromCamera(
        m_State.Position,
        forward,
        up,
        m_Config.fov,
        aspectRatio,
        m_Config.nearPlane,
        effectiveFarPlane,  // Use override or dynamic ViewFar
        effectiveTerrainCullRange  // Separate terrain culling distance
    );

    // Phase 5: Cache current state for next frame's comparison
    VectorCopy(m_State.Position, m_LastFrustumPosition);
    VectorCopy(m_State.Angle, m_LastFrustumAngle);
    m_LastFrustumViewFar = effectiveFarPlane;  // Cache the actual far plane used

#ifdef _EDITOR
    // Cache DevEditor config values
    if (DevEditor_IsConfigOverrideEnabled())
    {
        m_LastEditorFOV = m_Config.fov;
        m_LastEditorFarPlane = m_Config.farPlane;
        m_LastEditorNearPlane = m_Config.nearPlane;
        m_LastEditorTerrainCullRange = m_Config.terrainCullRange;
    }
#endif
}

bool DefaultCamera::NeedsFrustumUpdate() const
{
    // Phase 5: Check if camera state changed since last frustum rebuild
    const float EPSILON = 0.01f;  // Small threshold to avoid floating point comparison issues

#ifdef _EDITOR
    // Check if DevEditor config changed
    if (DevEditor_IsConfigOverrideEnabled())
    {
        float currentFOV, currentNearPlane, currentFarPlane, currentTerrainCullRange;
        DevEditor_GetCameraConfig(&currentFOV, &currentNearPlane, &currentFarPlane, &currentTerrainCullRange);

        if (fabs(currentFOV - m_LastEditorFOV) > EPSILON ||
            fabs(currentFarPlane - m_LastEditorFarPlane) > EPSILON ||
            fabs(currentNearPlane - m_LastEditorNearPlane) > EPSILON ||
            fabs(currentTerrainCullRange - m_LastEditorTerrainCullRange) > EPSILON)
        {
            return true;  // Config changed, rebuild needed
        }
    }
#endif

    // Check position change
    if (fabs(m_State.Position[0] - m_LastFrustumPosition[0]) > EPSILON ||
        fabs(m_State.Position[1] - m_LastFrustumPosition[1]) > EPSILON ||
        fabs(m_State.Position[2] - m_LastFrustumPosition[2]) > EPSILON)
    {
        return true;
    }

    // Check angle change
    if (fabs(m_State.Angle[0] - m_LastFrustumAngle[0]) > EPSILON ||
        fabs(m_State.Angle[1] - m_LastFrustumAngle[1]) > EPSILON ||
        fabs(m_State.Angle[2] - m_LastFrustumAngle[2]) > EPSILON)
    {
        return true;
    }

    // Check ViewFar change
    if (fabs(m_State.ViewFar - m_LastFrustumViewFar) > EPSILON)
    {
        return true;
    }

    return false;  // No significant change, skip frustum rebuild
}
