#pragma once

#ifdef _EDITOR

#include "CameraMove.h"

struct CameraConfig;

// Per-camera override state used by the DevEditor Game Scene panel.
// Default and Orbital cameras use fundamentally different projection/culling
// paths, so each gets its own set of fields. Common fields are duplicated
// rather than unified to keep the UI surface honest (each slider ties to a
// specific hook in the correct camera).
struct DevEditorDefaultCameraOverride
{
    bool  enabled          = false;
    // CameraConfig values that actually have an effect for this camera:
    //   nearPlane → g_Camera.ViewNear → gluPerspective near clip
    //   farPlane  → g_Camera.ViewFar  → gluPerspective far clip + fog scale
    // terrainCullRange is intentionally omitted: the Default camera's terrain
    // culling uses a hardcoded 2D trapezoid (see ZzzLodTerrain.cpp legacy path),
    // not CameraConfig.terrainCullRange. Use the width multipliers below instead.
    float nearPlane        = 500.0f;
    float farPlane         = 3000.0f;
    // Camera position offset from the default (hero-relative) placement.
    // Applied at the end of DefaultCamera::CalculateCameraPosition.
    float offsetX          = 0.0f;
    float offsetY          = 0.0f;
    float offsetZ          = 0.0f;
    // Multipliers applied to the hardcoded 2D culling trapezoid widths
    // (WidthNear = "bottom" of the trapezoid, WidthFar = "top").
    float widthNearMul     = 1.0f;
    float widthFarMul      = 1.0f;
    // Fog
    bool  fogOverride      = false;
    bool  fogOn            = true;
    float fogStartPct      = 1.00f;
    float fogEndPct        = 1.25f;
};

// Orbital override: tunes the 2D terrain-cull hull and fog. Does NOT touch the
// 3D render frustum (FOV / near / far) — those are deliberately left on the
// original Orbital path. The 2D hull naturally starts as a pyramid (apex at
// the camera, 4 far corners at terrainCullRange). When nearDepthFrac > 0 it
// becomes a real trapezoid with a non-zero near edge.
struct DevEditorOrbitalCameraOverride
{
    bool  enabled          = false;
    // View-aligned trapezoid in absolute world units. Corners are built in
    // camera-local coordinates (forward along view -Z, lateral along view X)
    // then transformed via the camera matrix, so the shape tracks yaw AND
    // pitch — the trapezoid always matches what the camera is actually looking at.
    float farDist      = 5000.0f;  // distance from camera to the far edge
    float farWidth     = 8000.0f;  // total width at the far edge
    float nearDist     = 0.0f;     // distance from camera to the near edge (0 = at camera)
    float nearWidth    = 800.0f;   // total width at the near edge. Non-zero is needed to
                                   // include tiles at the camera footprint — the apex pyramid
                                   // (nearWidth=0) leaves gaps where tile-center tests fail,
                                   // and static 3D objects anchored to those tiles pop in/out.
                                   // 800 covers the tile footprint under and around the camera.
    // Fog: single toggle, defaults to on (matches Orbital's vanilla behavior).
    // Applied whenever the Orbital override is enabled.
    bool  fogOn            = true;
    float fogStartPct      = 1.00f;
    float fogEndPct        = 1.25f;
};

class CDevEditorUI
{
public:
    static CDevEditorUI& GetInstance();

    void Render(bool* p_open);

    // Per-camera override accessors. Default drives the real tuning panel;
    // Orbital's is scaffolding with no sliders attached yet.
    const DevEditorDefaultCameraOverride& GetDefaultOverride() const { return m_DefaultOverride; }
    const DevEditorOrbitalCameraOverride& GetOrbitalOverride() const { return m_OrbitalOverride; }

    // Applies the enabled Default override onto `cfg` in-place (far / near / terrainCull only).
    // Trapezoid width multipliers and position offset are read separately.
    void ApplyDefaultOverrideToConfig(CameraConfig& cfg) const;
    // Orbital: no-op today (struct has no config-driving fields). Kept so the
    // extern-C dispatcher has a target and won't need refactoring later.
    void ApplyOrbitalOverrideToConfig(CameraConfig& cfg) const;

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
    bool ShouldShowCharacterPickBoxes() const { return m_ShowCharacterPickBoxes; }
    bool ShouldShowItemPickBoxes() const { return m_ShowItemPickBoxes; }
    bool ShouldShowItemCullSphere() const { return m_ShowItemCullSphere; }
    bool ShouldShowTileGrid() const { return m_ShowTileGrid; }

    // Culling radius accessors
    float GetCullRadiusItem() const { return m_CullRadiusItem; }


private:
    CDevEditorUI() = default;
    ~CDevEditorUI() = default;

    void RenderScenesTab();
    void RenderGraphicsTab();

    // RenderScenesTab sections
    void RenderCameraModeControls();
    void RenderCameraSummaryLine(int cameraMode);
    void RenderLoginSceneSection();
    void RenderGameSceneSection(int cameraMode, class ICamera* currentCamera);
    void RenderDefaultCameraOverridePanel();
    void RenderOrbitalCameraOverridePanel();
    void RenderScenesDebugSection();

    // RenderGraphicsTab section
    void RenderGraphicsDebugInfo();

    DevEditorDefaultCameraOverride m_DefaultOverride;
    DevEditorOrbitalCameraOverride m_OrbitalOverride;


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
    bool m_ShowCharacterPickBoxes = false;  // Wireframe of OBB used by SelectCharacter
    bool m_ShowItemPickBoxes = false;       // Wireframe of OBB used by SelectItem
    bool m_ShowItemCullSphere = false;      // Wireframe of frustum-cull sphere for items
    bool m_ShowTileGrid = false;

    // Runtime adjustable culling radii
    float m_CullRadiusItem = 100.0f;

    // Login scene render distances (adjustable via sliders).
    // Defaults live in LoginSceneCameraDefaults (CameraMove.h) so the non-editor
    // fallback in ZzzObject.cpp uses the same source of truth.
    float m_LoginTerrainDist = LoginSceneCameraDefaults::RENDER_TERRAIN_DIST;
    float m_LoginObjectDist  = LoginSceneCameraDefaults::RENDER_OBJECT_DIST;

public:
    // Login scene accessors
    float GetLoginTerrainDist() const { return m_LoginTerrainDist; }
    float GetLoginObjectDist() const { return m_LoginObjectDist; }
};

#define g_DevEditorUI CDevEditorUI::GetInstance()

#endif // _EDITOR
