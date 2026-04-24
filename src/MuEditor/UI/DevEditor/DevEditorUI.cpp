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
        RenderGameSceneSection(cameraMode, currentCamera, m_LastActiveCameraName);

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

void CDevEditorUI::RenderGameSceneSection(int cameraMode, ICamera* currentCamera, const char*& lastCameraName)
{
    ImGui::Indent();

    const char* currentCameraName = currentCamera ? currentCamera->GetName() : nullptr;

    // Re-sync FOV when the active/spectated camera changes under us
    if (m_ConfigOverrideEnabled && lastCameraName != currentCameraName)
    {
        if (currentCamera)
            m_HFOV = currentCamera->GetConfig().hFov;
        lastCameraName = currentCameraName;
    }

    const bool previousOverrideState = m_ConfigOverrideEnabled;
    ImGui::Checkbox("Override Camera Config", &m_ConfigOverrideEnabled);

    if (m_ConfigOverrideEnabled && !previousOverrideState)
    {
        if (currentCamera)
            m_HFOV = currentCamera->GetConfig().hFov;
        lastCameraName = currentCameraName;
    }
    if (!m_ConfigOverrideEnabled && previousOverrideState)
    {
        lastCameraName = nullptr;
        m_FogOverride = false;
    }

    if (currentCamera)
    {
        const CameraConfig& cfg = currentCamera->GetConfig();
        ImGui::Text("Near: %.0f  Far: %.0f  ViewFar: %.0f  ProjFar: %.0f",
                    cfg.nearPlane, cfg.farPlane, g_Camera.ViewFar,
                    g_Camera.ViewFar * RENDER_DISTANCE_MULTIPLIER);
    }

    if (m_ConfigOverrideEnabled)
    {
        extern unsigned int WindowWidth, WindowHeight;
        const float aspect = (float)WindowWidth / (float)WindowHeight;
        const float computedVFov = HFovToVFov(m_HFOV, aspect);

        ImGui::PushItemWidth(200);
        ImGui::SliderFloat("H-FOV", &m_HFOV, MIN_HFOV, MAX_HFOV, "%.1f");
        ImGui::TextColored(ImVec4(0.7f, 1.0f, 0.7f, 1.0f),
                          "V-FOV: %.1f (aspect %.2f)", computedVFov, aspect);

        ImGui::Spacing();
        extern bool FogEnable;

        ImGui::Checkbox("Override Fog", &m_FogOverride);
        if (m_FogOverride)
        {
            ImGui::SameLine();
            ImGui::Checkbox("Fog On", &m_FogOverrideValue);
        }
        ImGui::SameLine();
        if (FogEnable)
            ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "ON");
        else
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "OFF");

        float startDisplay = m_FogStartPct * 100.0f;
        float endDisplay   = m_FogEndPct   * 100.0f;
        if (ImGui::SliderFloat("Fog Start %", &startDisplay, 0.0f, 200.0f, "%.0f%%"))
            m_FogStartPct = startDisplay / 100.0f;
        if (ImGui::SliderFloat("Fog End %", &endDisplay, 0.0f, 200.0f, "%.0f%%"))
            m_FogEndPct = endDisplay / 100.0f;

        const float actualStart = g_Camera.ViewFar * m_FogStartPct;
        const float actualEnd   = g_Camera.ViewFar * m_FogEndPct;
        ImGui::TextColored(ImVec4(0.7f, 1.0f, 0.7f, 1.0f),
                          "Fog: %.0f - %.0f (ViewFar=%.0f)", actualStart, actualEnd, g_Camera.ViewFar);
        ImGui::PopItemWidth();

        ImGui::Spacing();
        if (ImGui::Button("Reset##config"))
        {
            if (currentCamera)
                m_HFOV = currentCamera->GetConfig().hFov;
            m_FogStartPct = 1.00f;
            m_FogEndPct   = 1.25f;
        }
    }
    else if (currentCamera)
    {
        extern unsigned int WindowWidth, WindowHeight;
        const float aspect = (float)WindowWidth / (float)WindowHeight;
        const float hFov = currentCamera->GetConfig().hFov;
        ImGui::Text("  FOV: %.1f H / %.1f V  Fog: %.0f%%/%.0f%%",
                    hFov, HFovToVFov(hFov, aspect),
                    m_FogStartPct * 100.0f, m_FogEndPct * 100.0f);
    }

    if (cameraMode == CAMERA_MODE_ORBITAL)
    {
        ImGui::Spacing();
        float radius = GetOrbitalCameraRadius();
        float orbitalYaw = 0.0f, orbitalPitch = 0.0f;
        GetOrbitalCameraAngles(&orbitalYaw, &orbitalPitch);
        ImGui::Text("Orbital: Zoom=%.0f  Yaw=%.1f  Pitch=%.1f", radius, orbitalYaw, orbitalPitch);
    }

    ImGui::Unindent();
}

void CDevEditorUI::RenderScenesDebugSection()
{
    ImGui::Indent();

    // Debug visualization
    ImGui::Checkbox("Character Cuboids", &m_ShowCharacterCullingSpheres);
    ImGui::SameLine();
    ImGui::Checkbox("Dropped Item Spheres", &m_ShowItemCullingSpheres);
    ImGui::Checkbox("Tile Grid", &m_ShowTileGrid);

    ImGui::PushItemWidth(150);
    ImGui::InputFloat("Dropped Item Cull Radius", &m_CullRadiusItem, 10.0f, 50.0f, "%.1f");
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

    // Render Toggles
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Render Toggles:");

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
// and (partially) fullscreen toggle. Updates WindowWidth/Height, screen rate, OpenGL
// viewport, config file, fonts, and resizes the window if in windowed mode.
void CDevEditorUI::ApplyNewWindowSize(int newWidth, int newHeight, const char* logReason)
{
    extern unsigned int WindowWidth, WindowHeight;
    extern float g_fScreenRate_x, g_fScreenRate_y;
    extern int OpenglWindowWidth, OpenglWindowHeight;
    extern BOOL g_bUseWindowMode;
    extern HWND g_hWnd;
    extern void ReinitializeFonts();
    extern void UpdateResolutionDependentSystems();

    WindowWidth  = (unsigned)newWidth;
    WindowHeight = (unsigned)newHeight;

    g_fScreenRate_x = (float)WindowWidth  / (float)REFERENCE_WIDTH;
    g_fScreenRate_y = (float)WindowHeight / (float)REFERENCE_HEIGHT;

    OpenglWindowWidth  = (int)WindowWidth;
    OpenglWindowHeight = (int)WindowHeight;

    GameConfig::GetInstance().SetWindowSize(WindowWidth, WindowHeight);
    GameConfig::GetInstance().Save();

    ReinitializeFonts();
    UpdateResolutionDependentSystems();

    if (g_bUseWindowMode && g_hWnd)
    {
        RECT windowRect = { 0, 0, (LONG)WindowWidth, (LONG)WindowHeight };
        AdjustWindowRect(&windowRect,
                         WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER | WS_CLIPCHILDREN,
                         FALSE);
        SetWindowPos(g_hWnd, HWND_TOP, 0, 0,
                     windowRect.right - windowRect.left,
                     windowRect.bottom - windowRect.top,
                     SWP_NOMOVE | SWP_NOZORDER);
    }

    char msg[128];
    sprintf_s(msg, "Window resized to %s%ux%u", logReason ? logReason : "", WindowWidth, WindowHeight);
    g_MuEditorConsoleUI.LogEditor(msg);
}

void CDevEditorUI::RenderGraphicsTab()
{
    ImGui::Text("Graphics Settings");
    ImGui::Separator();

    RenderGraphicsDebugInfo();
    ImGui::Separator();
    ImGui::Spacing();

    RenderWindowSizePresets();
    ImGui::Separator();
    ImGui::Spacing();

    RenderCustomResolutionInput();
    ImGui::Separator();
    ImGui::Spacing();

    RenderFullscreenToggle();
    ImGui::Separator();

    // Info
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "Camera Auto-Adapts:");
    ImGui::Text("• FOV stays constant (set in Camera tab)");
    ImGui::Text("• Aspect ratio adjusts automatically");
    ImGui::Text("• Wider screens = more horizontal view");
    ImGui::Text("• Frustum cone updates each frame");

    ImGui::Separator();

    extern unsigned int WindowWidth, WindowHeight;
    const float aspectRatio = (float)WindowWidth / (float)WindowHeight;
    ImGui::Text("Current Aspect Ratio: %.3f (%s)", aspectRatio,
                aspectRatio > 1.7f ? "Ultra-wide" :
                aspectRatio > 1.6f ? "16:10" :
                aspectRatio > 1.5f ? "16:9" :
                aspectRatio > 1.4f ? "3:2" :
                aspectRatio > 1.2f ? "5:4" : "4:3");

    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.5f, 1.0f), "Settings Auto-Save:");
    ImGui::Text("• Settings save automatically when:");
    ImGui::Text("  - Changing resolution");
    ImGui::Text("  - Toggling window mode");
    ImGui::Text("  - Exiting the game");
    ImGui::Spacing();

    extern BOOL g_bUseWindowMode;
    if (ImGui::Button("Save Settings Now", ImVec2(200, 0)))
    {
        GameConfig::GetInstance().SetWindowSize(WindowWidth, WindowHeight);
        GameConfig::GetInstance().SetWindowMode(g_bUseWindowMode == TRUE);
        GameConfig::GetInstance().Save();
        g_MuEditorConsoleUI.LogEditor("Settings saved to config.ini");
    }
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "(Saves to config.ini)");
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

void CDevEditorUI::RenderWindowSizePresets()
{
    ImGui::Text("Window Size Presets:");

    static const struct { int width; int height; const char* label; } resolutions[] = {
        {  640,  480, "640 x 480 (4:3)"   },
        {  800,  600, "800 x 600 (4:3)"   },
        { 1024,  768, "1024 x 768 (4:3)"  },
        { 1280, 1024, "1280 x 1024 (5:4)" },
        { 1280,  720, "1280 x 720 (16:9)" },
        { 1600,  900, "1600 x 900 (16:9)" },
        { 1600, 1200, "1600 x 1200 (4:3)" },
        { 1680, 1050, "1680 x 1050 (16:10)" },
        { 1920, 1080, "1920 x 1080 (16:9)" },
        { 2560, 1440, "2560 x 1440 (16:9)" },
    };
    constexpr int NUM_RESOLUTIONS = sizeof(resolutions) / sizeof(resolutions[0]);

    for (int i = 0; i < NUM_RESOLUTIONS; i++)
    {
        if (i > 0 && i % 2 == 0)
            ImGui::Spacing();

        if (ImGui::Button(resolutions[i].label, ImVec2(200, 0)))
            ApplyNewWindowSize(resolutions[i].width, resolutions[i].height, "");

        if (i % 2 == 0)
            ImGui::SameLine();
    }
}

void CDevEditorUI::RenderCustomResolutionInput()
{
    ImGui::Text("Custom Resolution:");
    static int customWidth = 1920;
    static int customHeight = 1080;

    ImGui::PushItemWidth(150);
    ImGui::InputInt("Width", &customWidth, 10, 100);
    ImGui::InputInt("Height", &customHeight, 10, 100);
    ImGui::PopItemWidth();

    if (customWidth  < REFERENCE_WIDTH)        customWidth  = REFERENCE_WIDTH;
    if (customWidth  > CUSTOM_RES_MAX_WIDTH)   customWidth  = CUSTOM_RES_MAX_WIDTH;
    if (customHeight < REFERENCE_HEIGHT)       customHeight = REFERENCE_HEIGHT;
    if (customHeight > CUSTOM_RES_MAX_HEIGHT)  customHeight = CUSTOM_RES_MAX_HEIGHT;

    if (ImGui::Button("Apply Custom Size", ImVec2(200, 0)))
        ApplyNewWindowSize(customWidth, customHeight, "custom ");
}

void CDevEditorUI::RenderFullscreenToggle()
{
    extern unsigned int WindowWidth, WindowHeight;
    extern BOOL g_bUseWindowMode;
    extern HWND g_hWnd;
    extern int OpenglWindowWidth, OpenglWindowHeight;
    extern void ReinitializeFonts();
    extern void UpdateResolutionDependentSystems();

    ImGui::Text("Display Mode:");

    bool isWindowed = (g_bUseWindowMode == TRUE);
    if (ImGui::Checkbox("Windowed Mode", &isWindowed))
    {
        g_bUseWindowMode = isWindowed ? TRUE : FALSE;

        OpenglWindowWidth  = (int)WindowWidth;
        OpenglWindowHeight = (int)WindowHeight;

        GameConfig::GetInstance().SetWindowMode(g_bUseWindowMode == TRUE);
        GameConfig::GetInstance().Save();

        ReinitializeFonts();
        UpdateResolutionDependentSystems();

        if (g_hWnd)
        {
            if (g_bUseWindowMode)
            {
                // Switch to windowed
                ChangeDisplaySettings(nullptr, 0);

                DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
                SetWindowLongPtr(g_hWnd, GWL_STYLE, style);

                RECT windowRect = { 0, 0, (LONG)WindowWidth, (LONG)WindowHeight };
                AdjustWindowRect(&windowRect, style, FALSE);

                SetWindowPos(g_hWnd, HWND_NOTOPMOST, 100, 100,
                             windowRect.right - windowRect.left,
                             windowRect.bottom - windowRect.top,
                             SWP_SHOWWINDOW | SWP_FRAMECHANGED);

                g_MuEditorConsoleUI.LogEditor("Switched to windowed mode");
            }
            else
            {
                // Switch to fullscreen
                DEVMODE dmScreenSettings = {};
                dmScreenSettings.dmSize = sizeof(dmScreenSettings);
                dmScreenSettings.dmPelsWidth  = WindowWidth;
                dmScreenSettings.dmPelsHeight = WindowHeight;
                dmScreenSettings.dmBitsPerPel = 32;
                dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

                if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL)
                {
                    DWORD style = WS_POPUP | WS_VISIBLE;
                    SetWindowLongPtr(g_hWnd, GWL_STYLE, style);
                    SetWindowPos(g_hWnd, HWND_TOPMOST, 0, 0, WindowWidth, WindowHeight,
                                SWP_SHOWWINDOW | SWP_FRAMECHANGED);
                    g_MuEditorConsoleUI.LogEditor("Switched to fullscreen mode");
                }
                else
                {
                    g_bUseWindowMode = TRUE;  // Revert on failure
                    g_MuEditorConsoleUI.LogEditor("Failed to switch to fullscreen mode");
                }
            }
        }
    }

    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "(Toggle between windowed and fullscreen)");
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

    // New CameraConfig accessors
    // Game Scene settings — only apply in MAIN_SCENE
    bool DevEditor_IsConfigOverrideEnabled()
    {
        extern EGameScene SceneFlag;
        if (SceneFlag != MAIN_SCENE) return false;
        return g_DevEditorUI.IsConfigOverrideEnabled();
    }

    void DevEditor_GetCameraConfigFOV(float* outHFOV)
    {
        if (outHFOV) *outHFOV = g_DevEditorUI.GetHFOV();
    }

    void DevEditor_GetFogConfig(float* outStart, float* outEnd)
    {
        extern CameraState g_Camera;
        if (outStart) *outStart = g_Camera.ViewFar * g_DevEditorUI.GetFogStartPct();
        if (outEnd) *outEnd = g_Camera.ViewFar * g_DevEditorUI.GetFogEndPct();
    }

    bool DevEditor_IsFogOverrideEnabled()
    {
        extern EGameScene SceneFlag;
        if (SceneFlag != MAIN_SCENE) return false;
        return g_DevEditorUI.IsConfigOverrideEnabled() && g_DevEditorUI.IsFogOverrideEnabled();
    }

    bool DevEditor_GetFogOverrideValue()
    {
        return g_DevEditorUI.GetFogOverrideValue();
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
    bool DevEditor_ShouldShowCharacterCullingSpheres()
    {
#ifdef _EDITOR
        return g_DevEditorUI.ShouldShowCharacterCullingSpheres();
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

    bool DevEditor_ShouldShowItemCullingSpheres()
    {
#ifdef _EDITOR
        return g_DevEditorUI.ShouldShowItemCullingSpheres();
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
