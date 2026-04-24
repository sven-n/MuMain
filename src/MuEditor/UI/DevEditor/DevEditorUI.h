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


    // Debug visualization accessors
    bool ShouldShowCharacterCullingSpheres() const { return m_ShowCharacterCullingSpheres; }
    bool ShouldShowItemCullingSpheres() const { return m_ShowItemCullingSpheres; }
    bool ShouldShowTileGrid() const { return m_ShowTileGrid; }

    // Culling radius accessors
    float GetCullRadiusItem() const { return m_CullRadiusItem; }

private:
    CDevEditorUI() = default;
    ~CDevEditorUI() = default;

    void RenderScenesTab();
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

    // Debug visualization flags
    bool m_ShowCharacterCullingSpheres = false;
    bool m_ShowItemCullingSpheres = false;
    bool m_ShowTileGrid = false;

    // Runtime adjustable culling radii
    float m_CullRadiusItem = 100.0f;

    // Login scene render distances (adjustable via sliders)
    float m_LoginTerrainDist = 3995.0f;
    float m_LoginObjectDist = 5903.0f;

public:
    // Login scene accessors
    float GetLoginTerrainDist() const { return m_LoginTerrainDist; }
    float GetLoginObjectDist() const { return m_LoginObjectDist; }
};

#define g_DevEditorUI CDevEditorUI::GetInstance()

#endif // _EDITOR
