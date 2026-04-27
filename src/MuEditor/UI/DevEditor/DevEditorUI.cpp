#include "stdafx.h"

#ifdef _EDITOR

#include "DevEditorUI.h"
#include "imgui.h"
#include "Camera/CameraManager.h"
#include "Camera/CameraMode.h"
#include "Camera/CameraConfig.h"
#include "Camera/OrbitalCamera.h"
#include "Camera/FreeFlyCamera.h"
#include "CameraMove.h"
#include "ZzzCharacter.h"
#include "GameConfig/GameConfig.h"
#include "UI/Console/MuEditorConsoleUI.h"

// External C functions
extern "C" CameraManager& CameraManager_Instance();
extern "C" OrbitalCamera* GetOrbitalCameraInstance();

// External camera state
extern CameraState g_Camera;

// Forward declarations for camera accessors
extern "C" int GetCurrentCameraMode();
extern "C" float GetOrbitalCameraRadius();
extern "C" void GetOrbitalCameraAngles(float* outYaw, float* outPitch);

// CCameraMove wrapper
extern "C" CCameraMove* CCameraMove__GetInstancePtr();

// CHARACTER external
extern CHARACTER* Hero;

namespace
{
    // Horizontal FOV slider range (degrees). Matches the CameraConfig clamp used elsewhere.
    constexpr float MIN_HFOV = 10.0f;
    constexpr float MAX_HFOV = 150.0f;

    // Camera mode enum values as exposed by GetCurrentCameraMode()
    constexpr int CAMERA_MODE_DEFAULT = 0;
    constexpr int CAMERA_MODE_ORBITAL = 1;
    constexpr int CAMERA_MODE_FREEFLY = 2;

    // Login-scene render-distance slider range (world units)
    constexpr float LOGIN_DIST_MIN = 1000.0f;
    constexpr float LOGIN_DIST_MAX = 30000.0f;

    // Custom resolution input clamping (pixels)
    constexpr int CUSTOM_RES_MAX_WIDTH  = 3840;
    constexpr int CUSTOM_RES_MAX_HEIGHT = 2160;

    // World-to-tile scale (100 world units = 1 tile)
    constexpr float WORLD_TO_TILE_DIVISOR = 100.0f;
}

CDevEditorUI& CDevEditorUI::GetInstance()
{
    static CDevEditorUI instance;
    return instance;
}

void CDevEditorUI::ApplyOrbitalOverrideToConfig(CameraConfig& cfg) const
{
    (void)cfg;
    // No Orbital config fields are exposed yet; this hook is here so the
    // extern-C dispatcher has somewhere to route once sliders are added.
}

void CDevEditorUI::ApplyDefaultOverrideToConfig(CameraConfig& cfg) const
{
    if (!m_DefaultOverride.enabled) return;
    // hFov / terrainCullRange intentionally NOT overridden: the Default camera
    // derives its horizontal extent and terrain cull shape from a hardcoded 2D
    // trapezoid (see WidthFar/WidthNear in ZzzLodTerrain.cpp), not from a
    // symmetric FOV or a terrainCullRange radius. Width multipliers (exposed
    // via DevEditor_GetDefaultTrapezoidMultipliers) are the real knobs there.
    cfg.nearPlane        = m_DefaultOverride.nearPlane;
    cfg.farPlane         = m_DefaultOverride.farPlane;
}

void CDevEditorUI::Render(bool* p_open)
{
    if (!p_open || !*p_open)
        return;

    ImGui::SetNextWindowSize(ImVec2(450, 500), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Dev Editor", p_open))
    {
        ImGui::End();
        return;
    }

    // Tab bar
    if (ImGui::BeginTabBar("DevEditorTabs"))
    {
        if (ImGui::BeginTabItem("Scenes"))
        {
            RenderScenesTab();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Graphics"))
        {
            RenderGraphicsTab();
            ImGui::EndTabItem();
        }

        // Future tabs can be added here
        // if (ImGui::BeginTabItem("Performance")) { ... }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void CDevEditorUI::RenderScenesTab()
{
    extern EGameScene SceneFlag;
    auto& camMgr = CameraManager_Instance();
    int cameraMode = GetCurrentCameraMode();

    // In FreeFly mode we edit the spectated camera's config, otherwise the active one.
    ICamera* currentCamera = camMgr.GetActiveCamera();
    if (camMgr.GetCurrentMode() == CameraMode::FreeFly)
    {
        if (ICamera* spectated = camMgr.GetSpectatedCamera())
            currentCamera = spectated;
    }

    RenderCameraModeControls();
    RenderCameraSummaryLine(cameraMode);
    ImGui::Separator();

    if (SceneFlag == LOG_IN_SCENE && ImGui::CollapsingHeader("Login Scene"))
        RenderLoginSceneSection();

    if (SceneFlag == CHARACTER_SCENE && ImGui::CollapsingHeader("Character Scene"))
    {
        ImGui::Indent();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Nothing here yet.");
        ImGui::Unindent();
    }

    if (SceneFlag == MAIN_SCENE && ImGui::CollapsingHeader("Game Scene"))
        RenderGameSceneSection(cameraMode, currentCamera);

    if (ImGui::CollapsingHeader("Debug"))
        RenderScenesDebugSection();
}

void CDevEditorUI::RenderCameraModeControls()
{
    auto& camMgr = CameraManager::Instance();
    const bool isFreeFly = (camMgr.GetCurrentMode() == CameraMode::FreeFly);

    if (!isFreeFly)
    {
        if (ImGui::Button("Switch to FreeFly", ImVec2(250, 0)))
            camMgr.SetCameraMode(CameraMode::FreeFly);
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", camMgr.GetActiveCamera()->GetName());
        return;
    }

    if (ImGui::Button("Switch to Game Camera", ImVec2(250, 0)))
    {
        ICamera* spectated = camMgr.GetSpectatedCamera();
        CameraMode target = CameraMode::Default;
        if (spectated && strcmp(spectated->GetName(), "Orbital") == 0)
            target = CameraMode::Orbital;
        camMgr.SetCameraMode(target);
    }
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "FreeFly");

    if (ICamera* spectated = camMgr.GetSpectatedCamera())
    {
        ImGui::Text("Spectating: %s", spectated->GetName());
        ImGui::SameLine();
        vec3_t snapPos, snapAngle;
        if (camMgr.GetSpectatedCameraState(snapPos, snapAngle))
        {
            if (ImGui::Button("Snap to Spectated"))
            {
                auto* freeFly = static_cast<FreeFlyCamera*>(camMgr.GetActiveCamera());
                freeFly->SnapToPosition(snapPos, snapAngle[2], snapAngle[0]);
            }
        }
    }
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Arrows/PgUp/PgDn=Move  RMB=Look  Shift=Fast");
}

void CDevEditorUI::RenderCameraSummaryLine(int cameraMode)
{
    const char* modeName;
    switch (cameraMode)
    {
        case CAMERA_MODE_DEFAULT: modeName = "Default"; break;
        case CAMERA_MODE_ORBITAL: modeName = "Orbital"; break;
        case CAMERA_MODE_FREEFLY: modeName = "FreeFly"; break;
        default:                  modeName = "Unknown"; break;
    }
    ImGui::Text("%s | Pos: %.0f, %.0f, %.0f | Tile: (%d, %d) | Pitch: %.1f Yaw: %.1f",
                modeName, g_Camera.Position[0], g_Camera.Position[1], g_Camera.Position[2],
                (int)(g_Camera.Position[0] / WORLD_TO_TILE_DIVISOR),
                (int)(g_Camera.Position[1] / WORLD_TO_TILE_DIVISOR),
                g_Camera.Angle[0], g_Camera.Angle[2]);
}

void CDevEditorUI::RenderLoginSceneSection()
{
    ImGui::Indent();

    ImGui::PushItemWidth(150);
    ImGui::InputFloat("Offset X", &g_LoginSceneOffsetX, 50.0f, 200.0f, "%.1f");
    ImGui::InputFloat("Offset Y", &g_LoginSceneOffsetY, 50.0f, 200.0f, "%.1f");
    ImGui::InputFloat("Offset Z", &g_LoginSceneOffsetZ, 50.0f, 200.0f, "%.1f");
    ImGui::InputFloat("Pitch", &g_LoginSceneAnglePitch, 1.0f, 5.0f, "%.1f");
    ImGui::InputFloat("Yaw", &g_LoginSceneAngleYaw, 1.0f, 5.0f, "%.1f");
    ImGui::PopItemWidth();

    if (ImGui::Button("Reset Offsets"))
    {
        g_LoginSceneOffsetX   = LoginSceneCameraDefaults::OFFSET_X;
        g_LoginSceneOffsetY   = LoginSceneCameraDefaults::OFFSET_Y;
        g_LoginSceneOffsetZ   = LoginSceneCameraDefaults::OFFSET_Z;
        g_LoginSceneAnglePitch = LoginSceneCameraDefaults::ANGLE_PITCH;
        g_LoginSceneAngleYaw   = LoginSceneCameraDefaults::ANGLE_YAW;
    }

    ImGui::Spacing();

    // Tour mode controls
    if (CCameraMove* cameraMove = CCameraMove__GetInstancePtr())
    {
        BOOL isTourMode = cameraMove->IsTourMode();
        BOOL isTourPaused = cameraMove->IsTourPaused();

        ImGui::Text("Tour: %s%s", isTourMode ? "ACTIVE" : "INACTIVE",
                    (isTourMode && isTourPaused) ? " (PAUSED)" : "");

        if (isTourMode)
        {
            if (isTourPaused)
            {
                if (ImGui::Button("Resume")) cameraMove->PauseTour(FALSE);
            }
            else
            {
                if (ImGui::Button("Pause")) cameraMove->PauseTour(TRUE);
            }
            ImGui::SameLine();
            if (ImGui::Button("Restart") && Hero)
            {
                cameraMove->SetTourMode(FALSE, FALSE, 0);
                cameraMove->PlayCameraWalk(Hero->Object.Position, 1000);
                cameraMove->SetTourMode(TRUE, FALSE, 0);
            }
        }
        else if (ImGui::Button("Start Tour") && Hero)
        {
            cameraMove->PlayCameraWalk(Hero->Object.Position, 1000);
            cameraMove->SetTourMode(TRUE, FALSE, 0);
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Render Distances:");
    ImGui::PushItemWidth(200);
    ImGui::SliderFloat("Terrain ViewFar", &m_LoginTerrainDist, LOGIN_DIST_MIN, LOGIN_DIST_MAX, "%.0f");
    ImGui::SliderFloat("Object Distance", &m_LoginObjectDist, LOGIN_DIST_MIN, LOGIN_DIST_MAX, "%.0f");
    ImGui::PopItemWidth();
    if (ImGui::Button("Reset Distances"))
    {
        m_LoginTerrainDist = LoginSceneCameraDefaults::RENDER_TERRAIN_DIST;
        m_LoginObjectDist  = LoginSceneCameraDefaults::RENDER_OBJECT_DIST;
    }

    ImGui::Unindent();
}

void CDevEditorUI::RenderGameSceneSection(int cameraMode, ICamera* currentCamera)
{
    ImGui::Indent();

    if (currentCamera)
    {
        const CameraConfig& cfg = currentCamera->GetConfig();
        ImGui::Text("Near: %.0f  Far: %.0f  ViewFar: %.0f  ProjFar: %.0f",
                    cfg.nearPlane, cfg.farPlane, g_Camera.ViewFar,
                    g_Camera.ViewFar * RENDER_DISTANCE_MULTIPLIER);
    }

    // Route panel by the currently-focused camera name rather than camera mode,
    // so that when FreeFly is spectating Default/Orbital the override panel for
    // the spectated camera stays visible and editable.
    const char* focusedName = currentCamera ? currentCamera->GetName() : nullptr;
    const bool focusingDefault = focusedName && strcmp(focusedName, "Default") == 0;
    const bool focusingOrbital = focusedName && strcmp(focusedName, "Orbital") == 0;

    if (focusingDefault)
        RenderDefaultCameraOverridePanel();
    else if (focusingOrbital)
        RenderOrbitalCameraOverridePanel();
    else
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                           "Switch to (or spectate) Default or Orbital to edit overrides.");

    if (focusingOrbital)
    {
        ImGui::Spacing();
        float radius = GetOrbitalCameraRadius();
        float orbitalYaw = 0.0f, orbitalPitch = 0.0f;
        GetOrbitalCameraAngles(&orbitalYaw, &orbitalPitch);
        ImGui::Text("Orbital: Zoom=%.0f  Yaw=%.1f  Pitch=%.1f", radius, orbitalYaw, orbitalPitch);
    }

    ImGui::Unindent();
}

void CDevEditorUI::RenderDefaultCameraOverridePanel()
{
    DevEditorDefaultCameraOverride& ov = m_DefaultOverride;

    ImGui::Checkbox("Override Default Camera Config", &ov.enabled);
    if (!ov.enabled)
    {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                           "Hero-relative camera with hardcoded 2D trapezoid culling.");
        return;
    }

    ImGui::PushItemWidth(200);

    ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "View Frustum");
    ImGui::SliderFloat("Far Plane##def",   &ov.farPlane,         500.0f, 20000.0f, "%.0f");

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "Camera Offset (world units, from hero)");
    ImGui::SliderFloat("Offset X",    &ov.offsetX, -2000.0f, 2000.0f, "%.0f");
    ImGui::SliderFloat("Offset Y",    &ov.offsetY, -2000.0f, 2000.0f, "%.0f");
    ImGui::SliderFloat("Offset Z",    &ov.offsetZ, -1000.0f, 1000.0f, "%.0f");

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "2D Culling Trapezoid Width");
    ImGui::SliderFloat("Bottom (near) x", &ov.widthNearMul, 0.25f, 4.0f, "%.2f");
    ImGui::SliderFloat("Top (far) x",     &ov.widthFarMul,  0.25f, 4.0f, "%.2f");

    ImGui::Spacing();
    extern bool FogEnable;
    ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "Fog");
    ImGui::Checkbox("Override Fog##def", &ov.fogOverride);
    if (ov.fogOverride)
    {
        ImGui::SameLine();
        ImGui::Checkbox("Fog On##def", &ov.fogOn);
    }
    ImGui::SameLine();
    ImGui::TextColored(FogEnable ? ImVec4(0.5f,1.0f,0.5f,1.0f) : ImVec4(1.0f,0.5f,0.5f,1.0f),
                       FogEnable ? "ON" : "OFF");
    float startDisp = ov.fogStartPct * 100.0f, endDisp = ov.fogEndPct * 100.0f;
    if (ImGui::SliderFloat("Fog Start %##def", &startDisp, 0.0f, 200.0f, "%.0f%%")) ov.fogStartPct = startDisp / 100.0f;
    if (ImGui::SliderFloat("Fog End %##def",   &endDisp,   0.0f, 200.0f, "%.0f%%")) ov.fogEndPct   = endDisp   / 100.0f;
    ImGui::TextColored(ImVec4(0.7f, 1.0f, 0.7f, 1.0f),
                       "Fog: %.0f - %.0f (ViewFar=%.0f)",
                       g_Camera.ViewFar * ov.fogStartPct, g_Camera.ViewFar * ov.fogEndPct,
                       g_Camera.ViewFar);

    ImGui::PopItemWidth();
    ImGui::Spacing();
    if (ImGui::Button("Reset to Camera Defaults##def"))
    {
        const CameraConfig cfg = CameraConfig::ForMainSceneDefaultCamera();
        ov.nearPlane = cfg.nearPlane;
        ov.farPlane  = cfg.farPlane;
        ov.offsetX = ov.offsetY = ov.offsetZ = 0.0f;
        ov.widthNearMul = ov.widthFarMul = 1.0f;
        ov.fogStartPct = 1.00f;
        ov.fogEndPct   = 1.25f;
    }
}

void CDevEditorUI::RenderOrbitalCameraOverridePanel()
{
    DevEditorOrbitalCameraOverride& ov = m_OrbitalOverride;

    // Seed the trapezoid from the natural view pyramid when the override is
    // first enabled. This way enabling at "defaults" == identical hull to
    // override-off, and user sees zero visible change until they touch a slider.
    auto seedFromNaturalPyramid = [&ov]()
    {
        if (ICamera* cam = CameraManager::Instance().GetActiveCamera())
        {
            const CameraConfig& cfg = cam->GetConfig();
            extern unsigned int WindowWidth, WindowHeight;
            const float aspect = (float)WindowWidth / (float)WindowHeight;
            const float vFov = HFovToVFov(cfg.hFov, aspect);
            const float tanHalf = tanf(vFov * 0.5f * Q_PI / 180.0f);
            ov.farDist   = cfg.terrainCullRange;
            ov.farWidth  = 2.0f * tanHalf * cfg.terrainCullRange * aspect;
            ov.nearDist  = 0.0f;
            ov.nearWidth = 800.0f;  // covers camera footprint so static 3D objects don't pop
        }
    };

    static bool s_wasEnabled = false;
    ImGui::Checkbox("Override Orbital Camera Config", &ov.enabled);
    if (ov.enabled && !s_wasEnabled) seedFromNaturalPyramid();
    s_wasEnabled = ov.enabled;

    if (!ov.enabled)
    {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                           "Tunes the 2D terrain-cull hull; does not touch FOV / far clip.");
        return;
    }

    ImGui::PushItemWidth(200);

    ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "2D Culling Trapezoid (world units)");
    // InputFloat: type any value directly, or use the +/- buttons (step = fine, Ctrl+click = coarse).
    ImGui::InputFloat("Far distance##orb",         &ov.farDist,   100.0f, 500.0f, "%.0f");
    ImGui::InputFloat("Top (far) width##orb",      &ov.farWidth,  100.0f, 500.0f, "%.0f");
    ImGui::InputFloat("Near distance##orb",        &ov.nearDist,   50.0f, 250.0f, "%.0f");
    ImGui::InputFloat("Bottom (near) width##orb",  &ov.nearWidth,  50.0f, 250.0f, "%.0f");
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                       "View-aligned: follows camera yaw + pitch (tracks what you look at).");

    ImGui::Spacing();
    extern bool FogEnable;
    ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "Fog");
    ImGui::Checkbox("Fog##orb", &ov.fogOn);
    ImGui::SameLine();
    ImGui::TextColored(FogEnable ? ImVec4(0.5f,1.0f,0.5f,1.0f) : ImVec4(1.0f,0.5f,0.5f,1.0f),
                       FogEnable ? "ON" : "OFF");
    float startDisp = ov.fogStartPct * 100.0f, endDisp = ov.fogEndPct * 100.0f;
    if (ImGui::InputFloat("Fog Start %##orb", &startDisp, 5.0f, 25.0f, "%.0f%%")) ov.fogStartPct = startDisp / 100.0f;
    if (ImGui::InputFloat("Fog End %##orb",   &endDisp,   5.0f, 25.0f, "%.0f%%")) ov.fogEndPct   = endDisp   / 100.0f;
    ImGui::TextColored(ImVec4(0.7f, 1.0f, 0.7f, 1.0f),
                       "Fog: %.0f - %.0f (ViewFar=%.0f)",
                       g_Camera.ViewFar * ov.fogStartPct, g_Camera.ViewFar * ov.fogEndPct,
                       g_Camera.ViewFar);

    ImGui::PopItemWidth();
    ImGui::Spacing();
    if (ImGui::Button("Reset to Natural Pyramid##orb"))
    {
        seedFromNaturalPyramid();
        ov.fogStartPct = 1.00f;
        ov.fogEndPct   = 1.25f;
    }
}

void CDevEditorUI::RenderScenesDebugSection()
{
    ImGui::Indent();

    // Debug Visualization — wireframes overlaid on the scene
    ImGui::Text("Debug Visualization:");
    ImGui::Columns(2, nullptr, false);
    ImGui::Checkbox("Character Pick Boxes", &m_ShowCharacterPickBoxes);
    ImGui::Checkbox("Item Pick Boxes",      &m_ShowItemPickBoxes);
    ImGui::NextColumn();
    ImGui::Checkbox("Item Cull Sphere",     &m_ShowItemCullSphere);
    ImGui::Checkbox("Tile Grid",            &m_ShowTileGrid);
    ImGui::Columns(1);

    ImGui::PushItemWidth(150);
    ImGui::InputFloat("Item Cull Radius", &m_CullRadiusItem, 10.0f, 50.0f, "%.1f");
    if (m_CullRadiusItem < 0.0f) m_CullRadiusItem = 0.0f;
    ImGui::PopItemWidth();

    ImGui::Spacing();

    if (OrbitalCamera* orbitalCam = GetOrbitalCameraInstance())
    {
        vec3_t target = {0, 0, 0};
        orbitalCam->GetTargetPosition(target);
        ImGui::Text("Orbital Target: %.0f, %.0f, %.0f  Tile: (%d, %d)",
                    target[0], target[1], target[2],
                    (int)(target[0] / WORLD_TO_TILE_DIVISOR),
                    (int)(target[1] / WORLD_TO_TILE_DIVISOR));
    }

    // Rendering — toggles for what gets drawn each frame
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Rendering:");

    ImGui::Columns(2, nullptr, false);
    ImGui::Checkbox("Terrain", &m_RenderTerrain);
    ImGui::Checkbox("Static Objects", &m_RenderStaticObjects);
    ImGui::Checkbox("Effects", &m_RenderEffects);
    ImGui::NextColumn();
    ImGui::Checkbox("Dropped Items", &m_RenderDroppedItems);
    ImGui::Checkbox("Weather", &m_RenderWeatherEffects);
    ImGui::Checkbox("Item Labels", &m_RenderItemLabels);
    ImGui::Columns(1);

    if (ImGui::Button("All ON"))
    {
        m_RenderTerrain = m_RenderStaticObjects = m_RenderEffects = true;
        m_RenderDroppedItems = m_RenderWeatherEffects = m_RenderItemLabels = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("All OFF"))
    {
        m_RenderTerrain = m_RenderStaticObjects = m_RenderEffects = false;
        m_RenderDroppedItems = m_RenderWeatherEffects = m_RenderItemLabels = false;
    }

    ImGui::Spacing();
    ImGui::Separator();

    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "TODO - Not Working:");
    ImGui::BeginDisabled();
    ImGui::Columns(2, nullptr, false);
    ImGui::Checkbox("Shaders", &m_RenderShaders);
    ImGui::Checkbox("Skill Effects", &m_RenderSkillEffects);
    ImGui::NextColumn();
    ImGui::Checkbox("Equipped Items", &m_RenderEquippedItems);
    ImGui::Checkbox("UI", &m_RenderUI);
    ImGui::Columns(1);

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "TODO - Not Implemented:");
    ImGui::Columns(3, nullptr, false);
    ImGui::Checkbox("Hero", &m_RenderHero);
    ImGui::NextColumn();
    ImGui::Checkbox("NPCs", &m_RenderNPCs);
    ImGui::NextColumn();
    ImGui::Checkbox("Monsters", &m_RenderMonsters);
    ImGui::Columns(1);
    ImGui::EndDisabled();

    ImGui::Unindent();
}

// Shared helper: applies a new window size. Used by preset buttons, custom-size apply,
void CDevEditorUI::RenderGraphicsTab()
{
    ImGui::Text("Graphics Debug Info");
    ImGui::Separator();

    RenderGraphicsDebugInfo();
}

void CDevEditorUI::RenderGraphicsDebugInfo()
{
    extern unsigned int WindowWidth, WindowHeight;
    extern BOOL g_bUseWindowMode;
    extern HWND g_hWnd;
    extern int OpenglWindowWidth, OpenglWindowHeight;
    extern float g_fScreenRate_x, g_fScreenRate_y;

    ImGui::Text("Current Resolution: %u x %u", WindowWidth, WindowHeight);
    ImGui::Text("OpenGL Viewport: %d x %d", OpenglWindowWidth, OpenglWindowHeight);
    ImGui::Text("Screen Rate: %.2f x %.2f", g_fScreenRate_x, g_fScreenRate_y);
    ImGui::Text("Window Mode: %s", g_bUseWindowMode ? "Windowed" : "Fullscreen");

    int clientWidth = 0, clientHeight = 0;
    float calculatedScaleX = 0, calculatedScaleY = 0;
    if (g_hWnd)
    {
        RECT clientRect;
        GetClientRect(g_hWnd, &clientRect);
        clientWidth  = clientRect.right  - clientRect.left;
        clientHeight = clientRect.bottom - clientRect.top;
        ImGui::Text("Actual Window Client: %d x %d", clientWidth, clientHeight);

        calculatedScaleX = (float)clientWidth  / (float)REFERENCE_WIDTH;
        calculatedScaleY = (float)clientHeight / (float)REFERENCE_HEIGHT;
        ImGui::Text("Calculated Scale from Client: %.2f x %.2f", calculatedScaleX, calculatedScaleY);
    }

    if (WindowWidth != OpenglWindowWidth || WindowHeight != OpenglWindowHeight)
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "WARNING: Window size mismatch detected!");

    ImGui::Spacing();
    if (ImGui::Button("Copy Debug Info to Clipboard", ImVec2(250, 0)))
    {
        char debugInfo[1024];
        sprintf_s(debugInfo,
            "=== Graphics Debug Info ===\n"
            "Current Resolution: %u x %u\n"
            "OpenGL Viewport: %d x %d\n"
            "Screen Rate: %.2f x %.2f\n"
            "Window Mode: %s\n"
            "Actual Window Client: %d x %d\n"
            "Calculated Scale from Client: %.2f x %.2f\n"
            "Mismatch: %s\n"
            "UI Reference System: %d x %d\n"
            "Expected UI Scale: WindowWidth/%d = %.2f, WindowHeight/%d = %.2f\n"
            "Aspect Ratio: %.3f\n",
            WindowWidth, WindowHeight,
            OpenglWindowWidth, OpenglWindowHeight,
            g_fScreenRate_x, g_fScreenRate_y,
            g_bUseWindowMode ? "Windowed" : "Fullscreen",
            clientWidth, clientHeight,
            calculatedScaleX, calculatedScaleY,
            (WindowWidth != OpenglWindowWidth || WindowHeight != OpenglWindowHeight) ? "YES" : "NO",
            REFERENCE_WIDTH, REFERENCE_HEIGHT,
            REFERENCE_WIDTH, (float)WindowWidth / (float)REFERENCE_WIDTH,
            REFERENCE_HEIGHT, (float)WindowHeight / (float)REFERENCE_HEIGHT,
            (float)WindowWidth / (float)WindowHeight
        );
        ImGui::SetClipboardText(debugInfo);
        g_MuEditorConsoleUI.LogEditor("Debug info copied to clipboard");
    }
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "(Paste in Discord/notepad)");
}

// Accessors for external use
extern "C"
{
    // Camera info accessors
    int GetCurrentCameraMode()
    {
        auto& manager = CameraManager_Instance();
        return static_cast<int>(manager.GetCurrentMode());
    }

    float GetOrbitalCameraRadius()
    {
        auto& manager = CameraManager_Instance();
        if (static_cast<int>(manager.GetCurrentMode()) == 1)  // CameraMode::Orbital
        {
            auto* orbital = GetOrbitalCameraInstance();
            if (orbital)
            {
                return orbital->GetRadius();
            }
        }
        return 800.0f;  // Default
    }

    // Note: GetOrbitalCameraAngles is implemented in CameraManager.cpp

    // Per-camera CameraConfig overrides — only apply in MAIN_SCENE.
    // Cameras pass their own GetName() ("Default" / "Orbital") so each maintains
    // an independent override state in the DevEditor UI.
    bool DevEditor_IsCameraOverrideEnabled(const char* cameraName)
    {
        extern EGameScene SceneFlag;
        if (SceneFlag != MAIN_SCENE || !cameraName) return false;
        if (strcmp(cameraName, "Default") == 0) return g_DevEditorUI.GetDefaultOverride().enabled;
        if (strcmp(cameraName, "Orbital") == 0) return g_DevEditorUI.GetOrbitalOverride().enabled;
        return false;
    }

    void DevEditor_ApplyCameraOverride(const char* cameraName, CameraConfig* cfg)
    {
        extern EGameScene SceneFlag;
        if (SceneFlag != MAIN_SCENE || !cfg || !cameraName) return;
        if (strcmp(cameraName, "Default") == 0) g_DevEditorUI.ApplyDefaultOverrideToConfig(*cfg);
        else if (strcmp(cameraName, "Orbital") == 0) g_DevEditorUI.ApplyOrbitalOverrideToConfig(*cfg);
    }

    bool DevEditor_IsCameraFogOverrideEnabled(const char* cameraName)
    {
        extern EGameScene SceneFlag;
        if (SceneFlag != MAIN_SCENE || !cameraName) return false;
        if (strcmp(cameraName, "Default") == 0)
        {
            const auto& ov = g_DevEditorUI.GetDefaultOverride();
            return ov.enabled && ov.fogOverride;
        }
        if (strcmp(cameraName, "Orbital") == 0)
        {
            // Orbital has a single Fog toggle that always applies when the
            // override is enabled (no separate fogOverride gate).
            const auto& ov = g_DevEditorUI.GetOrbitalOverride();
            return ov.enabled;
        }
        return false;
    }

    bool DevEditor_GetCameraFogOverrideValue(const char* cameraName)
    {
        if (cameraName)
        {
            if (strcmp(cameraName, "Default") == 0) return g_DevEditorUI.GetDefaultOverride().fogOn;
            if (strcmp(cameraName, "Orbital") == 0) return g_DevEditorUI.GetOrbitalOverride().fogOn;
        }
        return true;
    }

    void DevEditor_GetCameraFogRange(const char* cameraName, float viewFar, float* outStart, float* outEnd)
    {
        float startPct = 1.00f, endPct = 1.25f;
        if (cameraName)
        {
            if (strcmp(cameraName, "Default") == 0)
            {
                startPct = g_DevEditorUI.GetDefaultOverride().fogStartPct;
                endPct   = g_DevEditorUI.GetDefaultOverride().fogEndPct;
            }
            else if (strcmp(cameraName, "Orbital") == 0)
            {
                const auto& ov = g_DevEditorUI.GetOrbitalOverride();
                startPct = ov.fogStartPct;
                endPct   = ov.fogEndPct;
                // Scale fog off the trapezoid's Far distance when override is on,
                // so sliding "Far distance" also pushes fog out to match.
                if (ov.enabled) viewFar = ov.farDist;
            }
        }
        if (outStart) *outStart = viewFar * startPct;
        if (outEnd)   *outEnd   = viewFar * endPct;
    }

    // Default-camera-only accessors (position offset + 2D trapezoid width multipliers).
    // Return identity/zero when override disabled or not in MAIN_SCENE.
    void DevEditor_GetDefaultCameraOffset(float* outX, float* outY, float* outZ)
    {
        extern EGameScene SceneFlag;
        const auto& ov = g_DevEditorUI.GetDefaultOverride();
        const bool active = (SceneFlag == MAIN_SCENE) && ov.enabled;
        if (outX) *outX = active ? ov.offsetX : 0.0f;
        if (outY) *outY = active ? ov.offsetY : 0.0f;
        if (outZ) *outZ = active ? ov.offsetZ : 0.0f;
    }

    // Orbital 2D culling: returns `true` when a custom view-aligned trapezoid
    // should replace the view-cone pyramid. Values are in absolute world units
    // in camera-local space (forward = view -Z, lateral = view X).
    bool DevEditor_GetOrbitalHullTrapezoid(float* outFarDist, float* outFarWidth,
                                           float* outNearDist, float* outNearWidth)
    {
        extern EGameScene SceneFlag;
        const auto& ov = g_DevEditorUI.GetOrbitalOverride();
        const bool active = (SceneFlag == MAIN_SCENE) && ov.enabled;
        if (!active) return false;
        if (outFarDist)   *outFarDist   = ov.farDist;
        if (outFarWidth)  *outFarWidth  = ov.farWidth;
        if (outNearDist)  *outNearDist  = ov.nearDist;
        if (outNearWidth) *outNearWidth = ov.nearWidth;
        return true;
    }

    void DevEditor_GetDefaultTrapezoidMultipliers(float* outNearMul, float* outFarMul)
    {
        extern EGameScene SceneFlag;
        const auto& ov = g_DevEditorUI.GetDefaultOverride();
        const bool active = (SceneFlag == MAIN_SCENE) && ov.enabled;
        if (outNearMul) *outNearMul = active ? ov.widthNearMul : 1.0f;
        if (outFarMul)  *outFarMul  = active ? ov.widthFarMul  : 1.0f;
    }



    // Render toggle accessors
    bool DevEditor_ShouldRenderTerrain()
    {
        return g_DevEditorUI.ShouldRenderTerrain();
    }

    bool DevEditor_ShouldRenderStaticObjects()
    {
        return g_DevEditorUI.ShouldRenderStaticObjects();
    }

    bool DevEditor_ShouldRenderEffects()
    {
        return g_DevEditorUI.ShouldRenderEffects();
    }

    bool DevEditor_ShouldRenderDroppedItems()
    {
        return g_DevEditorUI.ShouldRenderDroppedItems();
    }

    bool DevEditor_ShouldRenderItemLabels()
    {
        return g_DevEditorUI.ShouldRenderItemLabels();
    }

    bool DevEditor_ShouldRenderEquippedItems()
    {
        return g_DevEditorUI.ShouldRenderEquippedItems();
    }

    bool DevEditor_ShouldRenderWeatherEffects()
    {
        return g_DevEditorUI.ShouldRenderWeatherEffects();
    }

    bool DevEditor_ShouldRenderUI()
    {
        return g_DevEditorUI.ShouldRenderUI();
    }

    // Not implemented (always return true)
    bool DevEditor_ShouldRenderHero()
    {
        return g_DevEditorUI.ShouldRenderHero();
    }

    bool DevEditor_ShouldRenderNPCs()
    {
        return g_DevEditorUI.ShouldRenderNPCs();
    }

    bool DevEditor_ShouldRenderMonsters()
    {
        return g_DevEditorUI.ShouldRenderMonsters();
    }

    // New untested toggles
    bool DevEditor_ShouldRenderShaders()
    {
        return g_DevEditorUI.ShouldRenderShaders();
    }

    bool DevEditor_ShouldRenderSkillEffects()
    {
        return g_DevEditorUI.ShouldRenderSkillEffects();
    }
}

#endif // _EDITOR

// C linkage wrappers for external C code (always available, return false when _EDITOR not defined)
extern "C" {
    bool DevEditor_ShouldShowCharacterPickBoxes()
    {
#ifdef _EDITOR
        return g_DevEditorUI.ShouldShowCharacterPickBoxes();
#else
        return false;
#endif
    }

    bool DevEditor_ShouldShowTileGrid()
    {
#ifdef _EDITOR
        return g_DevEditorUI.ShouldShowTileGrid();
#else
        return false;
#endif
    }

    bool DevEditor_ShouldShowItemCullSphere()
    {
#ifdef _EDITOR
        return g_DevEditorUI.ShouldShowItemCullSphere();
#else
        return false;
#endif
    }

    bool DevEditor_ShouldShowItemPickBoxes()
    {
#ifdef _EDITOR
        return g_DevEditorUI.ShouldShowItemPickBoxes();
#else
        return false;
#endif
    }

    float DevEditor_GetCullRadiusItem()
    {
#ifdef _EDITOR
        return g_DevEditorUI.GetCullRadiusItem();
#else
        return 100.0f;
#endif
    }

    float DevEditor_GetLoginTerrainDist()
    {
#ifdef _EDITOR
        return g_DevEditorUI.GetLoginTerrainDist();
#else
        return 10000.0f;
#endif
    }

    float DevEditor_GetLoginObjectDist()
    {
#ifdef _EDITOR
        return g_DevEditorUI.GetLoginObjectDist();
#else
        return 10000.0f;
#endif
    }

}
