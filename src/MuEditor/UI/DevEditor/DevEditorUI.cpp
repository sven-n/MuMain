#include "stdafx.h"

#ifdef _EDITOR

#include "DevEditorUI.h"
#include "imgui.h"
#include "Camera/CameraManager.h"
#include "Camera/OrbitalCamera.h"
#include "CameraMove.h"
#include "ZzzCharacter.h"

// External C functions
extern "C" CameraManager& CameraManager_Instance();
extern "C" OrbitalCamera* GetOrbitalCameraInstance();

// External camera state
extern CameraState g_Camera;

// Forward declarations for camera accessors
extern "C" int GetCurrentCameraMode();
extern "C" float GetOrbitalCameraRadius();
extern "C" void GetOrbitalCameraAngles(float* outYaw, float* outPitch);
extern "C" void GetActiveCameraConfig(float* outFOV, float* outNearPlane, float* outFarPlane, float* outTerrainCullRange);

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
        if (ImGui::BeginTabItem("Camera"))
        {
            RenderCameraTab();
            ImGui::EndTabItem();
        }

        // Future tabs can be added here
        // if (ImGui::BeginTabItem("Graphics")) { ... }
        // if (ImGui::BeginTabItem("Performance")) { ... }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void CDevEditorUI::RenderCameraTab()
{
    ImGui::Text("Camera System Controls");
    ImGui::Separator();

    // Debug visualization toggle
    if (ImGui::Checkbox("Show Frustum Trapezoid", &m_ShowDebugVisualization))
    {
        // Toggle applied immediately
    }
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(Red/Blue lines on ground)");

    ImGui::Separator();

    // Current camera info
    ImGui::Text("Current Camera Info:");
    ImGui::Indent();

    int cameraMode = GetCurrentCameraMode();
    const char* modeName;
    switch (cameraMode)
    {
        case 0: modeName = "Default"; break;
        case 1: modeName = "Orbital"; break;
#ifdef _EDITOR
        case 2: modeName = "FreeFly"; break;
#endif
        default: modeName = "Unknown"; break;
    }
    ImGui::Text("Mode: %s (Press F9 to cycle)", modeName);

    ImGui::Spacing();

    // Camera position in world space
    ImGui::Text("Position: X=%.1f, Y=%.1f, Z=%.1f",
                g_Camera.Position[0], g_Camera.Position[1], g_Camera.Position[2]);

    // Camera angles (pitch and yaw from CameraState)
    ImGui::Text("Pitch: %.1f°, Yaw: %.1f°",
                g_Camera.Angle[0], g_Camera.Angle[1]);

    if (cameraMode == 1)  // Orbital camera
    {
        ImGui::Spacing();
        float radius = GetOrbitalCameraRadius();
        ImGui::Text("Zoom Distance: %.0f units", radius);

        // Get orbital-specific rotation info
        float orbitalYaw = 0.0f, orbitalPitch = 0.0f;
        GetOrbitalCameraAngles(&orbitalYaw, &orbitalPitch);
        ImGui::Text("Orbital Yaw: %.1f°, Pitch: %.1f°", orbitalYaw, orbitalPitch);

        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                          "(200 = closest, 800 = default, 2000 = farthest)");
    }

    ImGui::Unindent();
    ImGui::Separator();

    // Enable/Disable override
    ImGui::Checkbox("Override Frustum Values", &m_OverrideEnabled);
    ImGui::Separator();

    if (m_OverrideEnabled)
    {
        ImGui::PushItemWidth(200);

        // Main controls
        ImGui::Text("Distance Controls:");
        ImGui::InputFloat("ViewFar (Far Plane)", &m_CameraViewFar, 100.0f, 500.0f, "%.1f");
        ImGui::InputFloat("ViewNear (Near Plane)", &m_CameraViewNear, 10.0f, 100.0f, "%.1f");
        ImGui::InputFloat("ViewTarget (Pivot)", &m_CameraViewTarget, 10.0f, 100.0f, "%.1f");

        ImGui::Separator();
        ImGui::Text("Width Controls:");
        ImGui::InputFloat("WidthFar (Top Width)", &m_WidthFar, 50.0f, 100.0f, "%.1f");
        ImGui::InputFloat("WidthNear (Bottom Width)", &m_WidthNear, 10.0f, 50.0f, "%.1f");

        ImGui::PopItemWidth();

        ImGui::Separator();

        // Presets
        ImGui::Text("Presets:");
        if (ImGui::Button("Default (Best)"))
        {
            m_CameraViewFar = 1100.0f;
            m_CameraViewNear = -530.0f;
            m_CameraViewTarget = 0.0f;
            m_WidthFar = 700.0f;
            m_WidthNear = 330.0f;
        }
        ImGui::SameLine();
        if (ImGui::Button("Old Default"))
        {
            m_CameraViewFar = 5100.0f;
            m_CameraViewNear = 969.0f;
            m_CameraViewTarget = 2397.0f;
            m_WidthFar = 2250.0f;
            m_WidthNear = 540.0f;
        }
        ImGui::SameLine();
        if (ImGui::Button("Wide View"))
        {
            m_CameraViewFar = 6000.0f;
            m_CameraViewNear = 1140.0f;
            m_CameraViewTarget = 2820.0f;
            m_WidthFar = 3000.0f;
            m_WidthNear = 700.0f;
        }

        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Values are applied in real-time!");
        ImGui::Text("Red trapezoid shows frustum on ground");
    }
    else
    {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Enable override to edit values");
    }

    ImGui::Separator();
    ImGui::Spacing();

    // New CameraConfig controls
    ImGui::Text("Camera Configuration (CameraConfig)");
    ImGui::Separator();

    ImGui::Checkbox("Override Camera Config", &m_ConfigOverrideEnabled);
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(FOV, near/far planes, cull range)");

    if (m_ConfigOverrideEnabled)
    {
        ImGui::PushItemWidth(200);

        ImGui::Text("3D Frustum Parameters:");
        ImGui::SliderFloat("FOV (degrees)", &m_FOV, 10.0f, 120.0f, "%.1f");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "(60° = default)");

        ImGui::InputFloat("Near Plane", &m_NearPlane, 1.0f, 10.0f, "%.1f");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "(10 = default)");

        ImGui::InputFloat("Far Plane", &m_FarPlane, 100.0f, 500.0f, "%.1f");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "(2400 = default)");

        ImGui::Spacing();
        ImGui::Text("2D Terrain Culling:");
        ImGui::InputFloat("Terrain Cull Range", &m_TerrainCullRange, 50.0f, 200.0f, "%.1f");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "(1100 = default)");

        ImGui::PopItemWidth();

        ImGui::Spacing();
        if (ImGui::Button("Reset to Defaults"))
        {
            m_FOV = 72.0f;
            m_NearPlane = 10.0f;
            m_FarPlane = 4800.0f;
            m_TerrainCullRange = 4200.0f;
        }

        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Changes applied in real-time!");
        ImGui::Text("Note: These override CameraConfig values");
    }
    else
    {
        // Phase 5: Display current camera config (read-only, single source of truth)
        float currentFOV, currentNearPlane, currentFarPlane, currentTerrainCullRange;
        GetActiveCameraConfig(&currentFOV, &currentNearPlane, &currentFarPlane, &currentTerrainCullRange);

        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "Current Camera Config (Scene-Specific):");
        ImGui::Text("  FOV: %.1f°", currentFOV);
        ImGui::Text("  Near Plane: %.1f", currentNearPlane);
        ImGui::Text("  Far Plane: %.1f", currentFarPlane);
        ImGui::Text("  Terrain Cull Range: %.1f", currentTerrainCullRange);
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Enable override to customize these values");
    }

    ImGui::Separator();
    ImGui::Spacing();

    // Phase 5 Debug: Camera Debug Info & Custom Origin
    ImGui::Text("Camera Debug Info");
    ImGui::Separator();

    // Get current camera info
    extern CameraState g_Camera;
    ImGui::Text("Current Camera Position:");
    ImGui::Text("  X: %.2f  Y: %.2f  Z: %.2f", g_Camera.Position[0], g_Camera.Position[1], g_Camera.Position[2]);
    ImGui::Text("Tile: (%d, %d)", (int)(g_Camera.Position[0] / 100.0f), (int)(g_Camera.Position[1] / 100.0f));

    // Get orbital camera target (if active)
    extern OrbitalCamera* GetOrbitalCameraInstance();
    OrbitalCamera* orbitalCam = GetOrbitalCameraInstance();
    if (orbitalCam)
    {
        vec3_t target = {0, 0, 0};
        orbitalCam->GetTargetPosition(target);
        ImGui::Text("Orbital Target:");
        ImGui::Text("  X: %.2f  Y: %.2f  Z: %.2f", target[0], target[1], target[2]);
        ImGui::Text("Tile: (%d, %d)", (int)(target[0] / 100.0f), (int)(target[1] / 100.0f));
    }

    ImGui::Spacing();
    ImGui::Separator();

    // Custom Origin Controls
    ImGui::Text("Custom Origin (Orbital Camera)");
    ImGui::Separator();

    ImGui::Checkbox("Enable Custom Origin", &m_CustomOriginEnabled);
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "(Override Hero position)");

    if (m_CustomOriginEnabled)
    {
        ImGui::PushItemWidth(150);
        ImGui::InputFloat("Origin X (tiles)", &m_CustomOriginX, 1.0f, 10.0f, "%.1f");
        ImGui::InputFloat("Origin Y (tiles)", &m_CustomOriginY, 1.0f, 10.0f, "%.1f");
        ImGui::InputFloat("Origin Z (height)", &m_CustomOriginZ, 10.0f, 50.0f, "%.1f");
        ImGui::PopItemWidth();

        ImGui::Text("World Units: (%.0f, %.0f, %.0f)",
            m_CustomOriginX * 100.0f, m_CustomOriginY * 100.0f, m_CustomOriginZ);

        if (ImGui::Button("Set to Current Camera Position"))
        {
            m_CustomOriginX = g_Camera.Position[0] / 100.0f;
            m_CustomOriginY = g_Camera.Position[1] / 100.0f;
            m_CustomOriginZ = g_Camera.Position[2];
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset to 0,0,100"))
        {
            m_CustomOriginX = 0.0f;
            m_CustomOriginY = 0.0f;
            m_CustomOriginZ = 100.0f;
        }
    }
    else
    {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Enable to set custom orbit center");
    }

    ImGui::Spacing();
    ImGui::Separator();

    // LoginScene Camera Offset Controls
    ImGui::Text("LoginScene Camera Offset");
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                      "Adjust tour waypoint positions in real-time");

    extern float g_LoginSceneOffsetX;
    extern float g_LoginSceneOffsetY;
    extern float g_LoginSceneOffsetZ;

    ImGui::PushItemWidth(150);
    ImGui::InputFloat("Offset X (Left/Right)", &g_LoginSceneOffsetX, 50.0f, 200.0f, "%.1f");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "(0 = default)");

    ImGui::InputFloat("Offset Y (Forward/Back)", &g_LoginSceneOffsetY, 50.0f, 200.0f, "%.1f");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "(1000 = default)");

    ImGui::InputFloat("Offset Z (Up/Down)", &g_LoginSceneOffsetZ, 50.0f, 200.0f, "%.1f");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "(700 = default)");

    ImGui::Spacing();
    ImGui::Text("Angle Offsets");

    extern float g_LoginSceneAnglePitch;
    extern float g_LoginSceneAngleYaw;

    ImGui::InputFloat("Pitch (Up/Down Tilt)", &g_LoginSceneAnglePitch, 1.0f, 5.0f, "%.1f");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "(0 = default)");

    ImGui::InputFloat("Yaw (Left/Right Rotation)", &g_LoginSceneAngleYaw, 1.0f, 5.0f, "%.1f");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "(0 = default)");
    ImGui::PopItemWidth();

    if (ImGui::Button("Reset to Defaults"))
    {
        g_LoginSceneOffsetX = -300.0f;
        g_LoginSceneOffsetY = 650.0f;
        g_LoginSceneOffsetZ = 950.0f;
        g_LoginSceneAnglePitch = 40.0f;
        g_LoginSceneAngleYaw = -5.0f;
    }

    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Changes apply in real-time!");

    ImGui::Spacing();
    ImGui::Text("Tour Mode Controls");
    ImGui::Separator();

    extern CCameraMove* CCameraMove__GetInstancePtr();
    CCameraMove* cameraMove = CCameraMove__GetInstancePtr();

    if (cameraMove)
    {
        BOOL isTourMode = cameraMove->IsTourMode();
        BOOL isTourPaused = cameraMove->IsTourPaused();

        ImGui::Text("Tour Mode: %s", isTourMode ? "ACTIVE" : "INACTIVE");
        if (isTourMode)
        {
            ImGui::Text("Status: %s", isTourPaused ? "PAUSED" : "RUNNING");
        }

        ImGui::Spacing();

        // Control buttons
        if (isTourMode)
        {
            // Pause/Resume toggle
            if (isTourPaused)
            {
                if (ImGui::Button("Resume"))
                {
                    cameraMove->PauseTour(FALSE);
                }
            }
            else
            {
                if (ImGui::Button("Pause"))
                {
                    cameraMove->PauseTour(TRUE);
                }
            }
            ImGui::SameLine();
        }

        // Restart button (always available when tour is active)
        if (isTourMode)
        {
            if (ImGui::Button("Restart"))
            {
                if (Hero)
                {
                    cameraMove->SetTourMode(FALSE, FALSE, 0);  // Disable first
                    cameraMove->PlayCameraWalk(Hero->Object.Position, 1000);
                    cameraMove->SetTourMode(TRUE, FALSE, 0);   // Re-enable from waypoint 0
                }
            }
        }
        else
        {
            // Start Tour button (only when not active)
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
    else
    {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Camera Move system not available");
    }

    ImGui::Spacing();
    ImGui::Separator();

    // Character Scene - Target Specific Character
    extern EGameScene SceneFlag;
    if (SceneFlag == CHARACTER_SCENE)
    {
        ImGui::Text("Character Scene - Target Character");
        ImGui::Separator();

        ImGui::Checkbox("Target Specific Character", &m_TargetCharacterEnabled);
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "(Orbit specific character 1-5)");

        if (m_TargetCharacterEnabled)
        {
            // Character selector (1-5)
            const char* charLabels[] = { "Character 1", "Character 2", "Character 3", "Character 4", "Character 5" };
            ImGui::PushItemWidth(150);
            ImGui::Combo("Target Character", &m_TargetCharacterIndex, charLabels, 5);
            ImGui::PopItemWidth();

            // Display character info if available
            extern CHARACTER* CharactersClient;
            if (CharactersClient && CharactersClient[m_TargetCharacterIndex].Object.Live)
            {
                CHARACTER* targetChar = &CharactersClient[m_TargetCharacterIndex];
                ImGui::Text("Character Position:");
                ImGui::Text("  X: %.2f  Y: %.2f  Z: %.2f",
                    targetChar->Object.Position[0],
                    targetChar->Object.Position[1],
                    targetChar->Object.Position[2]);

                if (ImGui::Button("Set Origin to This Character"))
                {
                    m_CustomOriginEnabled = true;
                    m_CustomOriginX = targetChar->Object.Position[0] / 100.0f;
                    m_CustomOriginY = targetChar->Object.Position[1] / 100.0f;
                    m_CustomOriginZ = targetChar->Object.Position[2];
                }
            }
            else
            {
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "Character %d not loaded", m_TargetCharacterIndex + 1);
            }
        }
        else
        {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Enable to target specific character");
        }

        ImGui::Separator();
        ImGui::Spacing();
    }

    // Render Toggles Section
    ImGui::Text("Rendering Toggles");
    ImGui::Separator();

    // Working toggles
    ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "✓ Working Toggles:");
    ImGui::Columns(2, nullptr, false);

    ImGui::Checkbox("Terrain", &m_RenderTerrain);
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "(Ground tiles)");

    ImGui::Checkbox("Static Objects", &m_RenderStaticObjects);
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "(Trees, stones, walls)");

    ImGui::Checkbox("Effects", &m_RenderEffects);
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "(Particles, magic)");

    ImGui::NextColumn();

    ImGui::Checkbox("Dropped Items", &m_RenderDroppedItems);
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "(Ground loot)");

    ImGui::Checkbox("Weather Effects", &m_RenderWeatherEffects);
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "(Rain, snow, leaves)");

    ImGui::Checkbox("Item Labels", &m_RenderItemLabels);
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "(Item text boxes)");

    ImGui::Columns(1);

    ImGui::Spacing();
    ImGui::Separator();

    // Not working section
    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "✗ Not Working (tested - too complex):");
    ImGui::BeginDisabled();
    ImGui::Columns(2, nullptr, false);

    ImGui::Checkbox("Shaders", &m_RenderShaders);
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(Too low level)");

    ImGui::Checkbox("Skill Effects", &m_RenderSkillEffects);
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(Too complex)");

    ImGui::NextColumn();

    ImGui::Checkbox("Equipped Items", &m_RenderEquippedItems);
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(Character system)");

    ImGui::Checkbox("UI", &m_RenderUI);
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(Needs grouping)");

    ImGui::Columns(1);
    ImGui::EndDisabled();

    ImGui::Spacing();
    ImGui::Separator();

    // Not implemented section
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "⊘ Not Implemented:");
    ImGui::BeginDisabled();
    ImGui::Columns(3, nullptr, false);

    ImGui::Checkbox("Hero", &m_RenderHero);
    ImGui::NextColumn();
    ImGui::Checkbox("NPCs", &m_RenderNPCs);
    ImGui::NextColumn();
    ImGui::Checkbox("Monsters", &m_RenderMonsters);

    ImGui::Columns(1);
    ImGui::EndDisabled();

    ImGui::Spacing();
    ImGui::Text("Quick Actions:");
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
    ImGui::SameLine();
    if (ImGui::Button("Only Static"))
    {
        m_RenderTerrain = true;
        m_RenderStaticObjects = true;
        m_RenderEffects = m_RenderDroppedItems = m_RenderWeatherEffects = m_RenderItemLabels = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Only Items"))
    {
        m_RenderTerrain = true;
        m_RenderDroppedItems = true;
        m_RenderItemLabels = true;
        m_RenderStaticObjects = m_RenderEffects = m_RenderWeatherEffects = false;
    }

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Legend: ✓ Working  ✗ Not Working  ⊘ Not Implemented");
}

// Accessors for external use
extern "C"
{
    bool DevEditor_IsOverrideEnabled()
    {
        return g_DevEditorUI.IsOverrideEnabled();
    }

    bool DevEditor_IsDebugVisualizationEnabled()
    {
        return g_DevEditorUI.IsDebugVisualizationEnabled();
    }

    void DevEditor_SetDebugVisualizationEnabled(bool enabled)
    {
        g_DevEditorUI.SetDebugVisualizationEnabled(enabled);
    }

    void DevEditor_GetFrustumValues(float* outViewFar, float* outViewNear, float* outViewTarget,
                                     float* outWidthFar, float* outWidthNear)
    {
        if (outViewFar) *outViewFar = g_DevEditorUI.GetCameraViewFar();
        if (outViewNear) *outViewNear = g_DevEditorUI.GetCameraViewNear();
        if (outViewTarget) *outViewTarget = g_DevEditorUI.GetCameraViewTarget();
        if (outWidthFar) *outWidthFar = g_DevEditorUI.GetWidthFar();
        if (outWidthNear) *outWidthNear = g_DevEditorUI.GetWidthNear();
    }

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
    bool DevEditor_IsConfigOverrideEnabled()
    {
        return g_DevEditorUI.IsConfigOverrideEnabled();
    }

    void DevEditor_GetCameraConfig(float* outFOV, float* outNearPlane, float* outFarPlane, float* outTerrainCullRange)
    {
        if (outFOV) *outFOV = g_DevEditorUI.GetFOV();
        if (outNearPlane) *outNearPlane = g_DevEditorUI.GetNearPlane();
        if (outFarPlane) *outFarPlane = g_DevEditorUI.GetFarPlane();
        if (outTerrainCullRange) *outTerrainCullRange = g_DevEditorUI.GetTerrainCullRange();
    }

    // Phase 5: Custom Origin accessors
    bool DevEditor_IsCustomOriginEnabled()
    {
        return g_DevEditorUI.IsCustomOriginEnabled();
    }

    void DevEditor_GetCustomOrigin(float* outX, float* outY, float* outZ)
    {
        if (outX) *outX = g_DevEditorUI.GetCustomOriginX() * 100.0f;  // Convert tiles to world units
        if (outY) *outY = g_DevEditorUI.GetCustomOriginY() * 100.0f;
        if (outZ) *outZ = g_DevEditorUI.GetCustomOriginZ();
    }

    // Phase 5: Character targeting accessors
    bool DevEditor_IsTargetCharacterEnabled()
    {
        return g_DevEditorUI.IsTargetCharacterEnabled();
    }

    int DevEditor_GetTargetCharacterIndex()
    {
        return g_DevEditorUI.GetTargetCharacterIndex();
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
