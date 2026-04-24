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
    extern CameraState g_Camera;
    extern CameraManager& CameraManager_Instance();
    auto& camMgrRef = CameraManager_Instance();
    int cameraMode = GetCurrentCameraMode();

    // Get the camera whose config we're editing — in FreeFly, that's the spectated camera
    ICamera* currentCamera = camMgrRef.GetActiveCamera();
    if (camMgrRef.GetCurrentMode() == CameraMode::FreeFly)
    {
        ICamera* spectated = camMgrRef.GetSpectatedCamera();
        if (spectated)
            currentCamera = spectated;
    }
    const char* currentCameraName = currentCamera ? currentCamera->GetName() : nullptr;

    // ===== FreeFly Camera (always available) =====
    {
        auto& camMgr = CameraManager::Instance();
        CameraMode currentMode = camMgr.GetCurrentMode();
        bool isFreeFly = (currentMode == CameraMode::FreeFly);

        if (isFreeFly)
        {
            if (ImGui::Button("Switch to Game Camera", ImVec2(250, 0)))
            {
                ICamera* spectated = camMgr.GetSpectatedCamera();
                if (spectated)
                {
                    const char* name = spectated->GetName();
                    if (strcmp(name, "Orbital") == 0)
                        camMgr.SetCameraMode(CameraMode::Orbital);
                    else
                        camMgr.SetCameraMode(CameraMode::Default);
                }
                else
                    camMgr.SetCameraMode(CameraMode::Default);
            }
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "FreeFly");

            ICamera* spectated = camMgr.GetSpectatedCamera();
            if (spectated)
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
        else
        {
            if (ImGui::Button("Switch to FreeFly", ImVec2(250, 0)))
            {
                camMgr.SetCameraMode(CameraMode::FreeFly);
            }
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", camMgr.GetActiveCamera()->GetName());
        }
    }

    // Camera info summary line
    {
        const char* modeName;
        switch (cameraMode)
        {
            case 0: modeName = "Default"; break;
            case 1: modeName = "Orbital"; break;
            case 2: modeName = "FreeFly"; break;
            default: modeName = "Unknown"; break;
        }
        ImGui::Text("%s | Pos: %.0f, %.0f, %.0f | Tile: (%d, %d) | Pitch: %.1f Yaw: %.1f",
                    modeName, g_Camera.Position[0], g_Camera.Position[1], g_Camera.Position[2],
                    (int)(g_Camera.Position[0] / 100.0f), (int)(g_Camera.Position[1] / 100.0f),
                    g_Camera.Angle[0], g_Camera.Angle[2]);
    }

    ImGui::Separator();

    // ===== Login Scene (only visible in login scene) =====
    if (SceneFlag == LOG_IN_SCENE && ImGui::CollapsingHeader("Login Scene"))
    {
        ImGui::Indent();

        // Camera Offsets
        extern float g_LoginSceneOffsetX;
        extern float g_LoginSceneOffsetY;
        extern float g_LoginSceneOffsetZ;
        extern float g_LoginSceneAnglePitch;
        extern float g_LoginSceneAngleYaw;

        ImGui::PushItemWidth(150);
        ImGui::InputFloat("Offset X", &g_LoginSceneOffsetX, 50.0f, 200.0f, "%.1f");
        ImGui::InputFloat("Offset Y", &g_LoginSceneOffsetY, 50.0f, 200.0f, "%.1f");
        ImGui::InputFloat("Offset Z", &g_LoginSceneOffsetZ, 50.0f, 200.0f, "%.1f");
        ImGui::InputFloat("Pitch", &g_LoginSceneAnglePitch, 1.0f, 5.0f, "%.1f");
        ImGui::InputFloat("Yaw", &g_LoginSceneAngleYaw, 1.0f, 5.0f, "%.1f");
        ImGui::PopItemWidth();

        if (ImGui::Button("Reset Offsets"))
        {
            g_LoginSceneOffsetX = -300.0f;
            g_LoginSceneOffsetY = 650.0f;
            g_LoginSceneOffsetZ = 950.0f;
            g_LoginSceneAnglePitch = 40.0f;
            g_LoginSceneAngleYaw = -5.0f;
        }

        ImGui::Spacing();

        // Tour Mode Controls
        extern CCameraMove* CCameraMove__GetInstancePtr();
        CCameraMove* cameraMove = CCameraMove__GetInstancePtr();
        if (cameraMove)
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
                if (ImGui::Button("Restart"))
                {
                    if (Hero)
                    {
                        cameraMove->SetTourMode(FALSE, FALSE, 0);
                        cameraMove->PlayCameraWalk(Hero->Object.Position, 1000);
                        cameraMove->SetTourMode(TRUE, FALSE, 0);
                    }
                }
            }
            else
            {
                if (ImGui::Button("Start Tour"))
                {
                    if (Hero)
                    {
                        cameraMove->PlayCameraWalk(Hero->Object.Position, 1000);
                        cameraMove->SetTourMode(TRUE, FALSE, 0);
                    }
                }
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Render Distances:");
        ImGui::PushItemWidth(200);
        ImGui::SliderFloat("Terrain ViewFar", &m_LoginTerrainDist, 1000.0f, 30000.0f, "%.0f");
        ImGui::SliderFloat("Object Distance", &m_LoginObjectDist, 1000.0f, 30000.0f, "%.0f");
        ImGui::PopItemWidth();
        if (ImGui::Button("Reset Distances"))
        {
            m_LoginTerrainDist = 3995.0f;
            m_LoginObjectDist = 5903.0f;
        }

        ImGui::Unindent();
    }

    // ===== Character Scene (only visible in character scene) =====
    if (SceneFlag == CHARACTER_SCENE && ImGui::CollapsingHeader("Character Scene"))
    {
        ImGui::Indent();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Nothing here yet.");
        ImGui::Unindent();
    }

    // ===== Game Scene (only visible in main game) =====
    if (SceneFlag == MAIN_SCENE && ImGui::CollapsingHeader("Game Scene"))
    {
        ImGui::Indent();

        // Camera Config Override
        if (m_ConfigOverrideEnabled && m_LastActiveCameraName != currentCameraName)
        {
            if (currentCamera)
                m_HFOV = currentCamera->GetConfig().hFov;
            m_LastActiveCameraName = currentCameraName;
        }

        bool previousOverrideState = m_ConfigOverrideEnabled;
        ImGui::Checkbox("Override Camera Config", &m_ConfigOverrideEnabled);

        if (m_ConfigOverrideEnabled && !previousOverrideState)
        {
            if (currentCamera)
                m_HFOV = currentCamera->GetConfig().hFov;
            m_LastActiveCameraName = currentCameraName;
        }
        if (!m_ConfigOverrideEnabled && previousOverrideState)
        {
            m_LastActiveCameraName = nullptr;
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
            float aspect = (float)WindowWidth / (float)WindowHeight;
            float computedVFov = HFovToVFov(m_HFOV, aspect);

            ImGui::PushItemWidth(200);
            ImGui::SliderFloat("H-FOV", &m_HFOV, 10.0f, 150.0f, "%.1f");
            ImGui::TextColored(ImVec4(0.7f, 1.0f, 0.7f, 1.0f),
                              "V-FOV: %.1f (aspect %.2f)", computedVFov, aspect);

            // Fog controls
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
            float endDisplay = m_FogEndPct * 100.0f;
            if (ImGui::SliderFloat("Fog Start %", &startDisplay, 0.0f, 200.0f, "%.0f%%"))
                m_FogStartPct = startDisplay / 100.0f;
            if (ImGui::SliderFloat("Fog End %", &endDisplay, 0.0f, 200.0f, "%.0f%%"))
                m_FogEndPct = endDisplay / 100.0f;

            float actualStart = g_Camera.ViewFar * m_FogStartPct;
            float actualEnd = g_Camera.ViewFar * m_FogEndPct;
            ImGui::TextColored(ImVec4(0.7f, 1.0f, 0.7f, 1.0f),
                              "Fog: %.0f - %.0f (ViewFar=%.0f)", actualStart, actualEnd, g_Camera.ViewFar);
            ImGui::PopItemWidth();

            ImGui::Spacing();
            if (ImGui::Button("Reset##config"))
            {
                if (currentCamera)
                    m_HFOV = currentCamera->GetConfig().hFov;
                m_FogStartPct = 1.00f;
                m_FogEndPct = 1.25f;
            }
        }
        else if (currentCamera)
        {
            extern unsigned int WindowWidth, WindowHeight;
            float aspect = (float)WindowWidth / (float)WindowHeight;
            float hFov = currentCamera->GetConfig().hFov;
            ImGui::Text("  FOV: %.1f H / %.1f V  Fog: %.0f%%/%.0f%%",
                        hFov, HFovToVFov(hFov, aspect),
                        m_FogStartPct * 100.0f, m_FogEndPct * 100.0f);
        }

        // Orbital info
        if (cameraMode == 1)
        {
            ImGui::Spacing();
            float radius = GetOrbitalCameraRadius();
            float orbitalYaw = 0.0f, orbitalPitch = 0.0f;
            GetOrbitalCameraAngles(&orbitalYaw, &orbitalPitch);
            ImGui::Text("Orbital: Zoom=%.0f  Yaw=%.1f  Pitch=%.1f", radius, orbitalYaw, orbitalPitch);
        }

        ImGui::Unindent();
    }

    // ===== Debug =====
    if (ImGui::CollapsingHeader("Debug"))
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

        extern OrbitalCamera* GetOrbitalCameraInstance();
        OrbitalCamera* orbitalCam = GetOrbitalCameraInstance();
        if (orbitalCam)
        {
            vec3_t target = {0, 0, 0};
            orbitalCam->GetTargetPosition(target);
            ImGui::Text("Orbital Target: %.0f, %.0f, %.0f  Tile: (%d, %d)",
                        target[0], target[1], target[2],
                        (int)(target[0] / 100.0f), (int)(target[1] / 100.0f));
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

        // TODO: Not working yet (tested - too complex)
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
}

void CDevEditorUI::RenderGraphicsTab()
{
    ImGui::Text("Graphics Settings");
    ImGui::Separator();

    // Window dimensions
    extern unsigned int WindowWidth;
    extern unsigned int WindowHeight;
    extern BOOL g_bUseWindowMode;
    extern HWND g_hWnd;
    extern int OpenglWindowWidth;
    extern int OpenglWindowHeight;
    extern float g_fScreenRate_x;
    extern float g_fScreenRate_y;

    // Display all debug info
    ImGui::Text("Current Resolution: %u x %u", WindowWidth, WindowHeight);
    ImGui::Text("OpenGL Viewport: %d x %d", OpenglWindowWidth, OpenglWindowHeight);
    ImGui::Text("Screen Rate: %.2f x %.2f", g_fScreenRate_x, g_fScreenRate_y);
    ImGui::Text("Window Mode: %s", g_bUseWindowMode ? "Windowed" : "Fullscreen");

    int clientWidth = 0, clientHeight = 0;
    float calculatedScaleX = 0, calculatedScaleY = 0;

    // Show actual window client rect
    if (g_hWnd)
    {
        RECT clientRect;
        GetClientRect(g_hWnd, &clientRect);
        clientWidth = clientRect.right - clientRect.left;
        clientHeight = clientRect.bottom - clientRect.top;
        ImGui::Text("Actual Window Client: %d x %d", clientWidth, clientHeight);

        // Calculate what the UI *thinks* the scaling should be
        calculatedScaleX = (float)clientWidth / 640.0f;
        calculatedScaleY = (float)clientHeight / 480.0f;
        ImGui::Text("Calculated Scale from Client: %.2f x %.2f", calculatedScaleX, calculatedScaleY);
    }

    // Show if there's a mismatch
    if (WindowWidth != OpenglWindowWidth || WindowHeight != OpenglWindowHeight)
    {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "WARNING: Window size mismatch detected!");
    }

    // Copy to clipboard button
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
            "UI Reference System: 640 x 480\n"
            "Expected UI Scale: WindowWidth/640 = %.2f, WindowHeight/480 = %.2f\n"
            "Aspect Ratio: %.3f\n",
            WindowWidth, WindowHeight,
            OpenglWindowWidth, OpenglWindowHeight,
            g_fScreenRate_x, g_fScreenRate_y,
            g_bUseWindowMode ? "Windowed" : "Fullscreen",
            clientWidth, clientHeight,
            calculatedScaleX, calculatedScaleY,
            (WindowWidth != OpenglWindowWidth || WindowHeight != OpenglWindowHeight) ? "YES" : "NO",
            (float)WindowWidth / 640.0f, (float)WindowHeight / 480.0f,
            (float)WindowWidth / (float)WindowHeight
        );
        ImGui::SetClipboardText(debugInfo);
        g_MuEditorConsoleUI.LogEditor("Debug info copied to clipboard");
    }
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "(Paste in Discord/notepad)");

    ImGui::Separator();
    ImGui::Spacing();

    // Window size presets
    ImGui::Text("Window Size Presets:");

    static const struct { int width; int height; const char* label; } resolutions[] = {
        { 640, 480, "640 x 480 (4:3)" },
        { 800, 600, "800 x 600 (4:3)" },
        { 1024, 768, "1024 x 768 (4:3)" },
        { 1280, 1024, "1280 x 1024 (5:4)" },
        { 1280, 720, "1280 x 720 (16:9)" },
        { 1600, 900, "1600 x 900 (16:9)" },
        { 1600, 1200, "1600 x 1200 (4:3)" },
        { 1680, 1050, "1680 x 1050 (16:10)" },
        { 1920, 1080, "1920 x 1080 (16:9)" },
        { 2560, 1440, "2560 x 1440 (16:9)" },
    };

    for (int i = 0; i < 10; i++)
    {
        if (i > 0 && i % 2 == 0)
            ImGui::Spacing();

        if (ImGui::Button(resolutions[i].label, ImVec2(200, 0)))
        {
            // Update window dimensions
            WindowWidth = resolutions[i].width;
            WindowHeight = resolutions[i].height;

            // Update screen rate factors
            extern float g_fScreenRate_x;
            extern float g_fScreenRate_y;
            g_fScreenRate_x = (float)WindowWidth / 640.0f;
            g_fScreenRate_y = (float)WindowHeight / 480.0f;

            // Update OpenGL viewport dimensions
            extern int OpenglWindowWidth;
            extern int OpenglWindowHeight;
            OpenglWindowWidth = WindowWidth;
            OpenglWindowHeight = WindowHeight;

            // Save to config file
            GameConfig::GetInstance().SetWindowSize(WindowWidth, WindowHeight);
             GameConfig::GetInstance().Save();

            // Reinitialize fonts and update resolution-dependent systems
            extern void ReinitializeFonts();
            extern void UpdateResolutionDependentSystems();
            ReinitializeFonts();
            UpdateResolutionDependentSystems();

            // Resize window if in windowed mode
            if (g_bUseWindowMode && g_hWnd)
            {
                RECT windowRect = { 0, 0, (LONG)WindowWidth, (LONG)WindowHeight };
                AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

                int windowWidth = windowRect.right - windowRect.left;
                int windowHeight = windowRect.bottom - windowRect.top;

                SetWindowPos(g_hWnd, HWND_TOP, 0, 0, windowWidth, windowHeight,
                            SWP_NOMOVE | SWP_NOZORDER);
            }

            char msg[128];
            sprintf_s(msg, "Window resized to %ux%u", WindowWidth, WindowHeight);
            g_MuEditorConsoleUI.LogEditor(msg);
        }

        if (i % 2 == 0)
            ImGui::SameLine();
    }

    ImGui::Separator();
    ImGui::Spacing();

    // Custom resolution
    ImGui::Text("Custom Resolution:");
    static int customWidth = 1920;
    static int customHeight = 1080;

    ImGui::PushItemWidth(150);
    ImGui::InputInt("Width", &customWidth, 10, 100);
    ImGui::InputInt("Height", &customHeight, 10, 100);
    ImGui::PopItemWidth();

    // Clamp values
    if (customWidth < 640) customWidth = 640;
    if (customWidth > 3840) customWidth = 3840;
    if (customHeight < 480) customHeight = 480;
    if (customHeight > 2160) customHeight = 2160;

    if (ImGui::Button("Apply Custom Size", ImVec2(200, 0)))
    {
        WindowWidth = customWidth;
        WindowHeight = customHeight;

        extern float g_fScreenRate_x;
        extern float g_fScreenRate_y;
        g_fScreenRate_x = (float)WindowWidth / 640.0f;
        g_fScreenRate_y = (float)WindowHeight / 480.0f;

        // Update OpenGL viewport dimensions
        extern int OpenglWindowWidth;
        extern int OpenglWindowHeight;
        OpenglWindowWidth = WindowWidth;
        OpenglWindowHeight = WindowHeight;

        // Save to config file
         GameConfig::GetInstance().SetWindowSize(WindowWidth, WindowHeight);
         GameConfig::GetInstance().Save();

        // Reinitialize fonts and update resolution-dependent systems
        ReinitializeFonts();
        UpdateResolutionDependentSystems();

        if (g_bUseWindowMode && g_hWnd)
        {
            RECT windowRect = { 0, 0, (LONG)WindowWidth, (LONG)WindowHeight };
            AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

            int windowWidth = windowRect.right - windowRect.left;
            int windowHeight = windowRect.bottom - windowRect.top;

            SetWindowPos(g_hWnd, HWND_TOP, 0, 0, windowWidth, windowHeight,
                        SWP_NOMOVE | SWP_NOZORDER);
        }

        char msg[128];
        sprintf_s(msg, "Window resized to custom %ux%u", WindowWidth, WindowHeight);
        g_MuEditorConsoleUI.LogEditor(msg);
    }

    ImGui::Separator();
    ImGui::Spacing();

    // Fullscreen toggle
    ImGui::Text("Display Mode:");

    bool isWindowed = g_bUseWindowMode == TRUE;
    if (ImGui::Checkbox("Windowed Mode", &isWindowed))
    {
        g_bUseWindowMode = isWindowed ? TRUE : FALSE;

        // Update OpenGL viewport dimensions
        extern int OpenglWindowWidth;
        extern int OpenglWindowHeight;
        OpenglWindowWidth = WindowWidth;
        OpenglWindowHeight = WindowHeight;

        // Save window mode to config
         GameConfig::GetInstance().SetWindowMode(g_bUseWindowMode == TRUE);
         GameConfig::GetInstance().Save();

        // Reinitialize fonts and update resolution-dependent systems
        extern void ReinitializeFonts();
        extern void UpdateResolutionDependentSystems();
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

                int windowWidth = windowRect.right - windowRect.left;
                int windowHeight = windowRect.bottom - windowRect.top;

                SetWindowPos(g_hWnd, HWND_NOTOPMOST, 100, 100, windowWidth, windowHeight,
                            SWP_SHOWWINDOW | SWP_FRAMECHANGED);

                g_MuEditorConsoleUI.LogEditor("Switched to windowed mode");
            }
            else
            {
                // Switch to fullscreen
                DEVMODE dmScreenSettings;
                memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
                dmScreenSettings.dmSize = sizeof(dmScreenSettings);
                dmScreenSettings.dmPelsWidth = WindowWidth;
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

    ImGui::Separator();
    ImGui::Spacing();

    // Info
    ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "Camera Auto-Adapts:");
    ImGui::Text("• FOV stays constant (set in Camera tab)");
    ImGui::Text("• Aspect ratio adjusts automatically");
    ImGui::Text("• Wider screens = more horizontal view");
    ImGui::Text("• Frustum cone updates each frame");

    ImGui::Separator();

    // Aspect ratio info
    float aspectRatio = (float)WindowWidth / (float)WindowHeight;
    ImGui::Text("Current Aspect Ratio: %.3f (%s)", aspectRatio,
                aspectRatio > 1.7f ? "Ultra-wide" :
                aspectRatio > 1.6f ? "16:10" :
                aspectRatio > 1.5f ? "16:9" :
                aspectRatio > 1.4f ? "3:2" :
                aspectRatio > 1.2f ? "5:4" : "4:3");

    ImGui::Separator();
    ImGui::Spacing();

    // Manual save button
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.5f, 1.0f), "Settings Auto-Save:");
    ImGui::Text("• Settings save automatically when:");
    ImGui::Text("  - Changing resolution");
    ImGui::Text("  - Toggling window mode");
    ImGui::Text("  - Exiting the game");
    ImGui::Spacing();

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

    void DevEditor_GetCameraConfig(float* outHFOV, float* outNearPlane, float* outFarPlane, float* outTerrainCullRange)
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
