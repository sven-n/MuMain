#pragma once

#ifdef _EDITOR

class CDevEditorUI
{
public:
    static CDevEditorUI& GetInstance();

    void Render(bool* p_open);

    // Accessors for external use (called from C code in ZzzLodTerrain.cpp)
    bool IsOverrideEnabled() const { return m_OverrideEnabled; }
    bool IsDebugVisualizationEnabled() const { return m_ShowDebugVisualization; }
    void SetDebugVisualizationEnabled(bool enabled) { m_ShowDebugVisualization = enabled; }
    float GetCameraViewFar() const { return m_CameraViewFar; }
    float GetCameraViewNear() const { return m_CameraViewNear; }
    float GetCameraViewTarget() const { return m_CameraViewTarget; }
    float GetWidthFar() const { return m_WidthFar; }
    float GetWidthNear() const { return m_WidthNear; }

    // New CameraConfig accessors
    bool IsConfigOverrideEnabled() const { return m_ConfigOverrideEnabled; }
    float GetFOV() const { return m_FOV; }
    float GetNearPlane() const { return m_NearPlane; }
    float GetFarPlane() const { return m_FarPlane; }
    float GetTerrainCullRange() const { return m_TerrainCullRange; }

private:
    CDevEditorUI() = default;
    ~CDevEditorUI() = default;

    void RenderCameraTab();

    // Camera frustum values for live editing (best settings for Default/Orbital camera)
    float m_CameraViewFar = 1100.0f;
    float m_CameraViewNear = -530.0f;
    float m_CameraViewTarget = 0.0f;
    float m_WidthFar = 700.0f;
    float m_WidthNear = 330.0f;

    bool m_OverrideEnabled = false;
    bool m_ShowDebugVisualization = true;

    // New CameraConfig values for live editing
    bool m_ConfigOverrideEnabled = false;
    float m_FOV = 30.0f;              // Field of view in degrees
    float m_NearPlane = 10.0f;        // Near clipping plane
    float m_FarPlane = 2400.0f;       // Far clipping plane (max view distance)
    float m_TerrainCullRange = 1100.0f;  // 2D terrain culling range
};

#define g_DevEditorUI CDevEditorUI::GetInstance()

#endif // _EDITOR
