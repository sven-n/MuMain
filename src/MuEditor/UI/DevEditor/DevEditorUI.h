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

    // Render toggle accessors
    bool ShouldRenderTerrain() const { return m_RenderTerrain; }
    bool ShouldRenderStaticObjects() const { return m_RenderStaticObjects; }
    bool ShouldRenderEffects() const { return m_RenderEffects; }
    bool ShouldRenderDroppedItems() const { return m_RenderDroppedItems; }
    bool ShouldRenderItemLabels() const { return m_RenderItemLabels; }
    bool ShouldRenderEquippedItems() const { return m_RenderEquippedItems; }
    bool ShouldRenderWeatherEffects() const { return m_RenderWeatherEffects; }
    bool ShouldRenderUI() const { return m_RenderUI; }
    // Not implemented (for future):
    bool ShouldRenderHero() const { return m_RenderHero; }
    bool ShouldRenderNPCs() const { return m_RenderNPCs; }
    bool ShouldRenderMonsters() const { return m_RenderMonsters; }
    bool ShouldRenderShaders() const { return m_RenderShaders; }
    bool ShouldRenderSkillEffects() const { return m_RenderSkillEffects; }

    // Phase 5: Custom Origin accessors
    bool IsCustomOriginEnabled() const { return m_CustomOriginEnabled; }
    float GetCustomOriginX() const { return m_CustomOriginX; }
    float GetCustomOriginY() const { return m_CustomOriginY; }
    float GetCustomOriginZ() const { return m_CustomOriginZ; }

    // Phase 5: Character targeting accessors
    bool IsTargetCharacterEnabled() const { return m_TargetCharacterEnabled; }
    int GetTargetCharacterIndex() const { return m_TargetCharacterIndex; }

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
    float m_FOV = 60.0f;              // Field of view in degrees (Phase 5: changed default to 60)
    float m_NearPlane = 10.0f;        // Near clipping plane
    float m_FarPlane = 2400.0f;       // Far clipping plane (max view distance)
    float m_TerrainCullRange = 1100.0f;  // 2D terrain culling range
    const char* m_LastActiveCameraName = nullptr;  // Track which camera we're overriding for

    // Phase 5 Debug: Custom Origin Controls
    bool m_CustomOriginEnabled = false;
    float m_CustomOriginX = 0.0f;     // X coordinate (0-255 game tiles, converted to world units)
    float m_CustomOriginY = 0.0f;     // Y coordinate (0-255 game tiles, converted to world units)
    float m_CustomOriginZ = 100.0f;   // Z height (world units)

    // Phase 5: Character Scene - Target specific character
    bool m_TargetCharacterEnabled = false;
    int m_TargetCharacterIndex = 0;   // 0-4 for characters 1-5

    // Render toggle flags
    // Working toggles
    bool m_RenderTerrain = true;          // ✓ WORKING - Ground tiles
    bool m_RenderStaticObjects = true;    // ✓ WORKING - Trees, stones, walls
    bool m_RenderEffects = true;          // ✓ WORKING - Particles, magic
    bool m_RenderDroppedItems = true;     // ✓ WORKING - Ground loot
    bool m_RenderWeatherEffects = true;   // ✓ WORKING - Rain, snow, leaves, mist
    bool m_RenderItemLabels = true;       // ✓ WORKING - Item text boxes on ground

    // Not working (too complex - tested and failed)
    bool m_RenderShaders = true;          // ✗ NOT WORKING - Too low level
    bool m_RenderSkillEffects = true;     // ✗ NOT WORKING - Too complex
    bool m_RenderEquippedItems = true;    // ✗ NOT WORKING - Equipment on characters (too complex)
    bool m_RenderUI = true;               // ✗ NOT WORKING - Interface (needs extensive grouping)

    // Not implemented (too complex)
    bool m_RenderHero = true;             // NOT IMPLEMENTED
    bool m_RenderNPCs = true;             // NOT IMPLEMENTED
    bool m_RenderMonsters = true;         // NOT IMPLEMENTED
};

#define g_DevEditorUI CDevEditorUI::GetInstance()

#endif // _EDITOR
