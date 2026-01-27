#include "stdafx.h"

#ifdef _EDITOR

#include "DevEditorUI.h"
#include "imgui.h"
#include "Camera/CameraManager.h"
#include "Camera/OrbitalCamera.h"

// External C functions
extern "C" CameraManager& CameraManager_Instance();
extern "C" OrbitalCamera* GetOrbitalCameraInstance();

// External camera state
extern CameraState g_Camera;

// Forward declarations for camera accessors
extern "C" int GetCurrentCameraMode();
extern "C" float GetOrbitalCameraRadius();
extern "C" void GetOrbitalCameraAngles(float* outYaw, float* outPitch);

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
}

#endif // _EDITOR
