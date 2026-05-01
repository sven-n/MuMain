#include "stdafx.h"

#ifdef _EDITOR

#include "DevEditorUI.h"
#include "imgui.h"
#include "Translation/i18n.h"
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
    if (!ImGui::Begin(EDITOR_TEXT("label_dev_editor_title"), p_open))
    {
        ImGui::End();
        return;
    }

    // Tab bar
    if (ImGui::BeginTabBar("DevEditorTabs"))
    {
        if (ImGui::BeginTabItem(EDITOR_TEXT("dev_tab_scenes")))
        {
            RenderScenesTab();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem(EDITOR_TEXT("dev_tab_graphics")))
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

    if (SceneFlag == LOG_IN_SCENE && ImGui::CollapsingHeader(EDITOR_TEXT("dev_section_login_scene")))
        RenderLoginSceneSection();

    if (SceneFlag == CHARACTER_SCENE && ImGui::CollapsingHeader(EDITOR_TEXT("dev_section_character_scene")))
    {
        ImGui::Indent();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s", EDITOR_TEXT("dev_msg_nothing_here"));
        ImGui::Unindent();
    }

    if (SceneFlag == MAIN_SCENE && ImGui::CollapsingHeader(EDITOR_TEXT("dev_section_game_scene")))
        RenderGameSceneSection(cameraMode, currentCamera);

    if (ImGui::CollapsingHeader(EDITOR_TEXT("dev_section_debug")))
        RenderScenesDebugSection();
}

void CDevEditorUI::RenderCameraModeControls()
{
    auto& camMgr = CameraManager::Instance();
    const bool isFreeFly = (camMgr.GetCurrentMode() == CameraMode::FreeFly);

    if (!isFreeFly)
    {
        if (ImGui::Button(EDITOR_TEXT("dev_btn_switch_to_freefly"), ImVec2(250, 0)))
            camMgr.SetCameraMode(CameraMode::FreeFly);
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", camMgr.GetActiveCamera()->GetName());
        return;
    }

    if (ImGui::Button(EDITOR_TEXT("dev_btn_switch_to_game_camera"), ImVec2(250, 0)))
    {
        ICamera* spectated = camMgr.GetSpectatedCamera();
        CameraMode target = CameraMode::Default;
        if (spectated && strcmp(spectated->GetName(), "Orbital") == 0)
            target = CameraMode::Orbital;
        camMgr.SetCameraMode(target);
    }
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "%s", EDITOR_TEXT("dev_label_freefly"));

    if (ICamera* spectated = camMgr.GetSpectatedCamera())
    {
        ImGui::Text("%s: %s", EDITOR_TEXT("dev_label_spectating"), spectated->GetName());
        ImGui::SameLine();
        vec3_t snapPos, snapAngle;
        if (camMgr.GetSpectatedCameraState(snapPos, snapAngle))
        {
            if (ImGui::Button(EDITOR_TEXT("dev_btn_snap_to_spectated")))
            {
                auto* freeFly = static_cast<FreeFlyCamera*>(camMgr.GetActiveCamera());
                freeFly->SnapToPosition(snapPos, snapAngle[2], snapAngle[0]);
            }
        }
    }
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s", EDITOR_TEXT("dev_label_freefly_help"));
}

void CDevEditorUI::RenderCameraSummaryLine(int cameraMode)
{
    const char* modeName;
    switch (cameraMode)
    {
        case CAMERA_MODE_DEFAULT: modeName = EDITOR_TEXT("dev_label_camera_default"); break;
        case CAMERA_MODE_ORBITAL: modeName = EDITOR_TEXT("dev_label_camera_orbital"); break;
        case CAMERA_MODE_FREEFLY: modeName = EDITOR_TEXT("dev_label_freefly");        break;
        default:                  modeName = EDITOR_TEXT("dev_label_camera_unknown"); break;
    }
    ImGui::Text("%s | %s: %.0f, %.0f, %.0f | %s: (%d, %d) | %s: %.1f %s: %.1f",
                modeName,
                EDITOR_TEXT("dev_label_pos"),
                g_Camera.Position[0], g_Camera.Position[1], g_Camera.Position[2],
                EDITOR_TEXT("dev_label_tile"),
                (int)(g_Camera.Position[0] / WORLD_TO_TILE_DIVISOR),
                (int)(g_Camera.Position[1] / WORLD_TO_TILE_DIVISOR),
                EDITOR_TEXT("dev_label_pitch"), g_Camera.Angle[0],
                EDITOR_TEXT("dev_label_yaw"),   g_Camera.Angle[2]);
}

void CDevEditorUI::RenderLoginSceneSection()
{
    ImGui::Indent();

    ImGui::PushItemWidth(150);
    ImGui::InputFloat(EDITOR_TEXT("dev_label_offset_x"), &g_LoginSceneOffsetX, 50.0f, 200.0f, "%.1f");
    ImGui::InputFloat(EDITOR_TEXT("dev_label_offset_y"), &g_LoginSceneOffsetY, 50.0f, 200.0f, "%.1f");
    ImGui::InputFloat(EDITOR_TEXT("dev_label_offset_z"), &g_LoginSceneOffsetZ, 50.0f, 200.0f, "%.1f");
    ImGui::InputFloat(EDITOR_TEXT("dev_label_pitch"), &g_LoginSceneAnglePitch, 1.0f, 5.0f, "%.1f");
    ImGui::InputFloat(EDITOR_TEXT("dev_label_yaw"), &g_LoginSceneAngleYaw, 1.0f, 5.0f, "%.1f");
    ImGui::PopItemWidth();

    if (ImGui::Button(EDITOR_TEXT("dev_btn_reset_offsets")))
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

        ImGui::Text("%s: %s%s",
                    EDITOR_TEXT("dev_label_tour"),
                    isTourMode ? EDITOR_TEXT("dev_label_active") : EDITOR_TEXT("dev_label_inactive"),
                    (isTourMode && isTourPaused) ? EDITOR_TEXT("dev_label_paused_suffix") : "");

        if (isTourMode)
        {
            if (isTourPaused)
            {
                if (ImGui::Button(EDITOR_TEXT("dev_btn_resume"))) cameraMove->PauseTour(FALSE);
            }
            else
            {
                if (ImGui::Button(EDITOR_TEXT("dev_btn_pause"))) cameraMove->PauseTour(TRUE);
            }
            ImGui::SameLine();
            if (ImGui::Button(EDITOR_TEXT("dev_btn_restart")) && Hero)
            {
                cameraMove->SetTourMode(FALSE, FALSE, 0);
                cameraMove->PlayCameraWalk(Hero->Object.Position, 1000);
                cameraMove->SetTourMode(TRUE, FALSE, 0);
            }
        }
        else if (ImGui::Button(EDITOR_TEXT("dev_btn_start_tour")) && Hero)
        {
            cameraMove->PlayCameraWalk(Hero->Object.Position, 1000);
            cameraMove->SetTourMode(TRUE, FALSE, 0);
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("%s", EDITOR_TEXT("dev_label_render_distances"));
    ImGui::PushItemWidth(200);
    ImGui::SliderFloat(EDITOR_TEXT("dev_label_terrain_viewfar"), &m_LoginTerrainDist, LOGIN_DIST_MIN, LOGIN_DIST_MAX, "%.0f");
    ImGui::SliderFloat(EDITOR_TEXT("dev_label_object_distance"), &m_LoginObjectDist, LOGIN_DIST_MIN, LOGIN_DIST_MAX, "%.0f");
    ImGui::PopItemWidth();
    if (ImGui::Button(EDITOR_TEXT("dev_btn_reset_distances")))
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
        ImGui::Text("%s: %.0f  %s: %.0f  %s: %.0f  %s: %.0f",
                    EDITOR_TEXT("dev_label_near"),    cfg.nearPlane,
                    EDITOR_TEXT("dev_label_far"),     cfg.farPlane,
                    EDITOR_TEXT("dev_label_viewfar"), g_Camera.ViewFar,
                    EDITOR_TEXT("dev_label_projfar"), g_Camera.ViewFar * RENDER_DISTANCE_MULTIPLIER);
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
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s",
                           EDITOR_TEXT("dev_msg_switch_to_default_orbital"));

    if (focusingOrbital)
    {
        ImGui::Spacing();
        float radius = GetOrbitalCameraRadius();
        float orbitalYaw = 0.0f, orbitalPitch = 0.0f;
        GetOrbitalCameraAngles(&orbitalYaw, &orbitalPitch);
        ImGui::Text("%s: %s=%.0f  %s=%.1f  %s=%.1f",
                    EDITOR_TEXT("dev_label_camera_orbital"),
                    EDITOR_TEXT("dev_label_zoom"),  radius,
                    EDITOR_TEXT("dev_label_yaw"),   orbitalYaw,
                    EDITOR_TEXT("dev_label_pitch"), orbitalPitch);
    }

    ImGui::Unindent();
}

void CDevEditorUI::RenderDefaultCameraOverridePanel()
{
    DevEditorDefaultCameraOverride& ov = m_DefaultOverride;

    // PushID disambiguates widgets that share labels with the Orbital panel
    // (formerly the ##def suffix on each label) so translated labels stay clean.
    ImGui::PushID("def");

    ImGui::Checkbox(EDITOR_TEXT("dev_chk_override_default_camera"), &ov.enabled);
    if (!ov.enabled)
    {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s",
                           EDITOR_TEXT("dev_msg_default_camera_help"));
        ImGui::PopID();
        return;
    }

    ImGui::PushItemWidth(200);

    ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "%s", EDITOR_TEXT("dev_label_view_frustum"));
    ImGui::SliderFloat(EDITOR_TEXT("dev_label_far_plane"), &ov.farPlane, 500.0f, 20000.0f, "%.0f");

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "%s", EDITOR_TEXT("dev_label_camera_offset"));
    ImGui::SliderFloat(EDITOR_TEXT("dev_label_offset_x"), &ov.offsetX, -2000.0f, 2000.0f, "%.0f");
    ImGui::SliderFloat(EDITOR_TEXT("dev_label_offset_y"), &ov.offsetY, -2000.0f, 2000.0f, "%.0f");
    ImGui::SliderFloat(EDITOR_TEXT("dev_label_offset_z"), &ov.offsetZ, -1000.0f, 1000.0f, "%.0f");

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "%s", EDITOR_TEXT("dev_label_culling_trapezoid_width"));
    ImGui::SliderFloat(EDITOR_TEXT("dev_label_bottom_near_mul"), &ov.widthNearMul, 0.25f, 4.0f, "%.2f");
    ImGui::SliderFloat(EDITOR_TEXT("dev_label_top_far_mul"),     &ov.widthFarMul,  0.25f, 4.0f, "%.2f");

    ImGui::Spacing();
    extern bool FogEnable;
    ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "%s", EDITOR_TEXT("dev_label_fog"));
    ImGui::Checkbox(EDITOR_TEXT("dev_chk_override_fog"), &ov.fogOverride);
    if (ov.fogOverride)
    {
        ImGui::SameLine();
        ImGui::Checkbox(EDITOR_TEXT("dev_chk_fog_on"), &ov.fogOn);
    }
    ImGui::SameLine();
    ImGui::TextColored(FogEnable ? ImVec4(0.5f,1.0f,0.5f,1.0f) : ImVec4(1.0f,0.5f,0.5f,1.0f),
                       "%s", FogEnable ? EDITOR_TEXT("dev_label_on") : EDITOR_TEXT("dev_label_off"));
    float startDisp = ov.fogStartPct * 100.0f, endDisp = ov.fogEndPct * 100.0f;
    if (ImGui::SliderFloat(EDITOR_TEXT("dev_label_fog_start_pct"), &startDisp, 0.0f, 200.0f, "%.0f%%")) ov.fogStartPct = startDisp / 100.0f;
    if (ImGui::SliderFloat(EDITOR_TEXT("dev_label_fog_end_pct"),   &endDisp,   0.0f, 200.0f, "%.0f%%")) ov.fogEndPct   = endDisp   / 100.0f;
    ImGui::TextColored(ImVec4(0.7f, 1.0f, 0.7f, 1.0f),
                       "%s: %.0f - %.0f (%s=%.0f)",
                       EDITOR_TEXT("dev_label_fog"),
                       g_Camera.ViewFar * ov.fogStartPct, g_Camera.ViewFar * ov.fogEndPct,
                       EDITOR_TEXT("dev_label_viewfar"), g_Camera.ViewFar);

    ImGui::PopItemWidth();
    ImGui::Spacing();
    if (ImGui::Button(EDITOR_TEXT("dev_btn_reset_camera_defaults")))
    {
        const CameraConfig cfg = CameraConfig::ForMainSceneDefaultCamera();
        ov.nearPlane = cfg.nearPlane;
        ov.farPlane  = cfg.farPlane;
        ov.offsetX = ov.offsetY = ov.offsetZ = 0.0f;
        ov.widthNearMul = ov.widthFarMul = 1.0f;
        ov.fogStartPct = 1.00f;
        ov.fogEndPct   = 1.25f;
    }

    ImGui::PopID();
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

    ImGui::PushID("orb");

    static bool s_wasEnabled = false;
    ImGui::Checkbox(EDITOR_TEXT("dev_chk_override_orbital_camera"), &ov.enabled);
    if (ov.enabled && !s_wasEnabled) seedFromNaturalPyramid();
    s_wasEnabled = ov.enabled;

    if (!ov.enabled)
    {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s",
                           EDITOR_TEXT("dev_msg_orbital_camera_help"));
        ImGui::PopID();
        return;
    }

    ImGui::PushItemWidth(200);

    ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "%s", EDITOR_TEXT("dev_label_culling_trapezoid"));
    // InputFloat: type any value directly, or use the +/- buttons (step = fine, Ctrl+click = coarse).
    ImGui::InputFloat(EDITOR_TEXT("dev_label_far_distance"),       &ov.farDist,   100.0f, 500.0f, "%.0f");
    ImGui::InputFloat(EDITOR_TEXT("dev_label_top_far_width"),      &ov.farWidth,  100.0f, 500.0f, "%.0f");
    ImGui::InputFloat(EDITOR_TEXT("dev_label_near_distance"),      &ov.nearDist,   50.0f, 250.0f, "%.0f");
    ImGui::InputFloat(EDITOR_TEXT("dev_label_bottom_near_width"),  &ov.nearWidth,  50.0f, 250.0f, "%.0f");
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s",
                       EDITOR_TEXT("dev_msg_view_aligned"));

    ImGui::Spacing();
    extern bool FogEnable;
    ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "%s", EDITOR_TEXT("dev_label_fog"));
    ImGui::Checkbox(EDITOR_TEXT("dev_chk_fog_on"), &ov.fogOn);
    ImGui::SameLine();
    ImGui::TextColored(FogEnable ? ImVec4(0.5f,1.0f,0.5f,1.0f) : ImVec4(1.0f,0.5f,0.5f,1.0f),
                       "%s", FogEnable ? EDITOR_TEXT("dev_label_on") : EDITOR_TEXT("dev_label_off"));
    float startDisp = ov.fogStartPct * 100.0f, endDisp = ov.fogEndPct * 100.0f;
    if (ImGui::InputFloat(EDITOR_TEXT("dev_label_fog_start_pct"), &startDisp, 5.0f, 25.0f, "%.0f%%")) ov.fogStartPct = startDisp / 100.0f;
    if (ImGui::InputFloat(EDITOR_TEXT("dev_label_fog_end_pct"),   &endDisp,   5.0f, 25.0f, "%.0f%%")) ov.fogEndPct   = endDisp   / 100.0f;
    ImGui::TextColored(ImVec4(0.7f, 1.0f, 0.7f, 1.0f),
                       "%s: %.0f - %.0f (%s=%.0f)",
                       EDITOR_TEXT("dev_label_fog"),
                       g_Camera.ViewFar * ov.fogStartPct, g_Camera.ViewFar * ov.fogEndPct,
                       EDITOR_TEXT("dev_label_viewfar"), g_Camera.ViewFar);

    ImGui::PopItemWidth();
    ImGui::Spacing();
    if (ImGui::Button(EDITOR_TEXT("dev_btn_reset_natural_pyramid")))
    {
        seedFromNaturalPyramid();
        ov.fogStartPct = 1.00f;
        ov.fogEndPct   = 1.25f;
    }

    ImGui::PopID();
}

void CDevEditorUI::RenderScenesDebugSection()
{
    ImGui::Indent();

    // Debug Visualization — wireframes overlaid on the scene
    ImGui::Text("%s", EDITOR_TEXT("dev_label_debug_visualization"));
    ImGui::Columns(2, nullptr, false);
    ImGui::Checkbox(EDITOR_TEXT("dev_chk_character_pick_boxes"), &m_ShowCharacterPickBoxes);
    ImGui::Checkbox(EDITOR_TEXT("dev_chk_item_pick_boxes"),      &m_ShowItemPickBoxes);
    ImGui::NextColumn();
    ImGui::Checkbox(EDITOR_TEXT("dev_chk_item_cull_sphere"),     &m_ShowItemCullSphere);
    ImGui::Checkbox(EDITOR_TEXT("dev_chk_tile_grid"),            &m_ShowTileGrid);
    ImGui::Columns(1);

    ImGui::PushItemWidth(150);
    ImGui::InputFloat(EDITOR_TEXT("dev_label_item_cull_radius"), &m_CullRadiusItem, 10.0f, 50.0f, "%.1f");
    if (m_CullRadiusItem < 0.0f) m_CullRadiusItem = 0.0f;
    ImGui::PopItemWidth();

    ImGui::Spacing();

    if (OrbitalCamera* orbitalCam = GetOrbitalCameraInstance())
    {
        vec3_t target = {0, 0, 0};
        orbitalCam->GetTargetPosition(target);
        ImGui::Text("%s: %.0f, %.0f, %.0f  %s: (%d, %d)",
                    EDITOR_TEXT("dev_label_orbital_target"),
                    target[0], target[1], target[2],
                    EDITOR_TEXT("dev_label_tile"),
                    (int)(target[0] / WORLD_TO_TILE_DIVISOR),
                    (int)(target[1] / WORLD_TO_TILE_DIVISOR));
    }

    // Rendering — toggles for what gets drawn each frame
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("%s", EDITOR_TEXT("dev_label_rendering"));

    ImGui::Columns(2, nullptr, false);
    ImGui::Checkbox(EDITOR_TEXT("dev_chk_terrain"), &m_RenderTerrain);
    ImGui::Checkbox(EDITOR_TEXT("dev_chk_static_objects"), &m_RenderStaticObjects);
    ImGui::Checkbox(EDITOR_TEXT("dev_chk_effects"), &m_RenderEffects);
    ImGui::NextColumn();
    ImGui::Checkbox(EDITOR_TEXT("dev_chk_dropped_items"), &m_RenderDroppedItems);
    ImGui::Checkbox(EDITOR_TEXT("dev_chk_weather"), &m_RenderWeatherEffects);
    ImGui::Checkbox(EDITOR_TEXT("dev_chk_item_labels"), &m_RenderItemLabels);
    ImGui::Columns(1);

    if (ImGui::Button(EDITOR_TEXT("dev_btn_all_on")))
    {
        m_RenderTerrain = m_RenderStaticObjects = m_RenderEffects = true;
        m_RenderDroppedItems = m_RenderWeatherEffects = m_RenderItemLabels = true;
    }
    ImGui::SameLine();
    if (ImGui::Button(EDITOR_TEXT("dev_btn_all_off")))
    {
        m_RenderTerrain = m_RenderStaticObjects = m_RenderEffects = false;
        m_RenderDroppedItems = m_RenderWeatherEffects = m_RenderItemLabels = false;
    }

    ImGui::Spacing();
    ImGui::Separator();

    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "%s", EDITOR_TEXT("dev_label_todo_not_working"));
    ImGui::BeginDisabled();
    ImGui::Columns(2, nullptr, false);
    ImGui::Checkbox(EDITOR_TEXT("dev_chk_shaders"), &m_RenderShaders);
    ImGui::Checkbox(EDITOR_TEXT("dev_chk_skill_effects"), &m_RenderSkillEffects);
    ImGui::NextColumn();
    ImGui::Checkbox(EDITOR_TEXT("dev_chk_equipped_items"), &m_RenderEquippedItems);
    ImGui::Checkbox(EDITOR_TEXT("dev_chk_ui"), &m_RenderUI);
    ImGui::Columns(1);

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s", EDITOR_TEXT("dev_label_todo_not_implemented"));
    ImGui::Columns(3, nullptr, false);
    ImGui::Checkbox(EDITOR_TEXT("dev_chk_hero"), &m_RenderHero);
    ImGui::NextColumn();
    ImGui::Checkbox(EDITOR_TEXT("dev_chk_npcs"), &m_RenderNPCs);
    ImGui::NextColumn();
    ImGui::Checkbox(EDITOR_TEXT("dev_chk_monsters"), &m_RenderMonsters);
    ImGui::Columns(1);
    ImGui::EndDisabled();

    ImGui::Unindent();
}

// Shared helper: applies a new window size. Used by preset buttons, custom-size apply,
void CDevEditorUI::RenderGraphicsTab()
{
    ImGui::Text("%s", EDITOR_TEXT("dev_label_graphics_debug_info"));
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

    ImGui::Text("%s: %u x %u", EDITOR_TEXT("dev_label_current_resolution"), WindowWidth, WindowHeight);
    ImGui::Text("%s: %d x %d", EDITOR_TEXT("dev_label_opengl_viewport"), OpenglWindowWidth, OpenglWindowHeight);
    ImGui::Text("%s: %.2f x %.2f", EDITOR_TEXT("dev_label_screen_rate"), g_fScreenRate_x, g_fScreenRate_y);
    ImGui::Text("%s: %s", EDITOR_TEXT("dev_label_window_mode"),
                g_bUseWindowMode ? EDITOR_TEXT("dev_label_windowed") : EDITOR_TEXT("dev_label_fullscreen"));

    int clientWidth = 0, clientHeight = 0;
    float calculatedScaleX = 0, calculatedScaleY = 0;
    if (g_hWnd)
    {
        RECT clientRect;
        GetClientRect(g_hWnd, &clientRect);
        clientWidth  = clientRect.right  - clientRect.left;
        clientHeight = clientRect.bottom - clientRect.top;
        ImGui::Text("%s: %d x %d", EDITOR_TEXT("dev_label_actual_window_client"), clientWidth, clientHeight);

        calculatedScaleX = (float)clientWidth  / (float)REFERENCE_WIDTH;
        calculatedScaleY = (float)clientHeight / (float)REFERENCE_HEIGHT;
        ImGui::Text("%s: %.2f x %.2f", EDITOR_TEXT("dev_label_calculated_scale"), calculatedScaleX, calculatedScaleY);
    }

    if (WindowWidth != OpenglWindowWidth || WindowHeight != OpenglWindowHeight)
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", EDITOR_TEXT("dev_warn_window_size_mismatch"));

    ImGui::Spacing();
    if (ImGui::Button(EDITOR_TEXT("dev_btn_copy_debug_info"), ImVec2(250, 0)))
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
        g_MuEditorConsoleUI.LogEditor(EDITOR_TEXT("dev_log_debug_info_copied"));
    }
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "%s", EDITOR_TEXT("dev_label_paste_hint"));
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
