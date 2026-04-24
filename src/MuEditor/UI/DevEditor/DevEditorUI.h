#pragma once

#ifdef _EDITOR

class CDevEditorUI
{
public:
    static CDevEditorUI& GetInstance();

    void Render(bool* p_open);

    // CameraConfig accessors
    bool IsConfigOverrideEnabled() const { return m_ConfigOverrideEnabled; }
    float GetHFOV() const { return m_HFOV; }

    // Fog control accessors
    bool IsFogOverrideEnabled() const { return m_FogOverride; }
    bool GetFogOverrideValue() const { return m_FogOverrideValue; }
    float GetFogStartPct() const { return m_FogStartPct; }
    float GetFogEndPct() const { return m_FogEndPct; }

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

    // Culling sphere visualization accessors
    bool ShouldShowTerrainCullingSpheres() const { return m_ShowTerrainCullingSpheres; }
    bool ShouldShowObjectCullingSpheres() const { return m_ShowObjectCullingSpheres; }
    bool ShouldShowCharacterCullingSpheres() const { return m_ShowCharacterCullingSpheres; }
    bool ShouldShowTileGrid() const { return m_ShowTileGrid; }

    // Culling radius accessors
    float GetCullRadiusTerrain() const { return m_CullRadiusTerrain; }
    float GetCullRadiusCharacter() const { return m_CullRadiusCharacter; }
    float GetCullRadiusItem() const { return m_CullRadiusItem; }

private:
    CDevEditorUI() = default;
    ~CDevEditorUI() = default;

    void RenderCameraTab();
    void RenderGraphicsTab();

    // CameraConfig values for live editing
    bool m_ConfigOverrideEnabled = false;
    float m_HFOV = 90.0f;             // Horizontal field of view in degrees
    const char* m_LastActiveCameraName = nullptr;  // Track which camera we're overriding for

    // Fog control values
    bool m_FogOverride = false;       // Override pitch-based fog enable/disable
    bool m_FogOverrideValue = true;   // Fog state when override is active
    float m_FogStartPct = 1.00f;     // Fog start as fraction of ViewFar (100%)
    float m_FogEndPct = 1.25f;       // Fog end as fraction of ViewFar (125%)

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

    // Culling sphere visualization flags
    bool m_ShowTerrainCullingSpheres = false;    // Show terrain tile culling spheres
    bool m_ShowObjectCullingSpheres = false;     // Show object culling spheres (trees, items, etc.)
    bool m_ShowCharacterCullingSpheres = false;  // Show character culling spheres
    bool m_ShowTileGrid = false;                 // Show terrain tile borders

    // Runtime adjustable culling radii
    float m_CullRadiusTerrain = 100.0f;
    float m_CullRadiusCharacter = 100.0f;
    float m_CullRadiusItem = 100.0f;
};

#define g_DevEditorUI CDevEditorUI::GetInstance()

#endif // _EDITOR
