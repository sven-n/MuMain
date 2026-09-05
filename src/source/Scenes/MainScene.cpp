///////////////////////////////////////////////////////////////////////////////
// MainScene.cpp - Main game scene implementation
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Engine/Object/EditObjects.h"
#include "UI/Chat/Chat.h"
#include "MainScene.h"
#include "SceneCommon.h"
#include "Camera/CameraUtility.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Render/Renderer/MuRenderer.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "Engine/Object/ZzzInterface.h"
#include "Input/Selection.h"
#include "Render/Effects/ZzzEffect.h"
#include "World/MapInfra/MapManager.h"
#include "UI/Core/SceneUICoordinator.h"
#include "UI/Core/NewUISystem.h"
#include "GameLogic/Social/PartyManager.h"
#include "GameLogic/Events/Cinematic/CDirection.h"
#include "GameLogic/Pets/w_PetProcess.h"
#include "Core/Utilities/Log/muConsoleDebug.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Core/Utilities/FrameProfiler.h"
#include "Network/Server/WSclient.h"
#include "Network/Reconnect/ReconnectManager.h"
#include "Engine/AI/GOBoid.h"
#include "GameLogic/Items/PersonalShopTitleImp.h"
#include "UI/Core/UIManager.h"
#include "Engine/Object/ZzzInventory.h"
#include "World/MapInfra/PortalMgr.h"
#include "Guild/GuildCache.h"
#include "UI/HUD/UIMapName.h"
#include "UI/Scaling/UITransform.h"
#include "Camera/CameraProjection.h"
#include "Camera/CameraManager.h"
#include "Camera/CameraMode.h"
#ifdef _EDITOR
#include "Camera/FrustumRenderer.h"
#include "Camera/CameraDebugLog.h"
#endif

// External declarations
#ifdef _EDITOR
extern "C" bool DevEditor_IsDebugVisualizationEnabled();
// DevEditor render toggle functions
extern "C" bool DevEditor_ShouldRenderTerrain();
extern "C" bool DevEditor_ShouldRenderStaticObjects();
extern "C" bool DevEditor_ShouldRenderEffects();
extern "C" bool DevEditor_ShouldRenderDroppedItems();
extern "C" bool DevEditor_ShouldRenderItemLabels();
extern "C" bool DevEditor_ShouldRenderEquippedItems();
extern "C" bool DevEditor_ShouldRenderWeatherEffects();
extern "C" bool DevEditor_ShouldRenderUI();
extern "C" bool DevEditor_IsCameraFogOverrideEnabled(const char* cameraName);
extern "C" bool DevEditor_GetCameraFogOverrideValue(const char* cameraName);
#endif

extern HWND g_hWnd;
extern CErrorReport g_ErrorReport;
extern float EarthQuake;
extern int CheckSkill;
extern int MouseY;
extern int LoadingWorld;
// LoadingScene.cpp -- closes the RmlUi loading-screen wallpaper it keeps open past its own
// one-frame flash. Called below once LoadingWorld drops below the "world ready" threshold.
extern void HideLoadingSceneOverlay();
extern DWORD g_dwKeyFocusUIID;
extern int ErrorMessage;
extern int HeroTile;
extern CHARACTER* Hero;
extern CUIManager* g_pUIManager;
extern CUIMapName* g_pUIMapName;
extern int MouseX;
extern vec3_t MouseTarget;
extern int EditFlag;

static bool RequireLeavesEffect()
{
    return (gMapManager.WorldActive == WD_0LORENCIA && HeroTile != 4) ||
           (gMapManager.WorldActive == WD_2DEVIAS && HeroTile != 3 && HeroTile < 10) ||
           gMapManager.WorldActive == WD_3NORIA ||
           gMapManager.WorldActive == WD_7ATLANSE ||
           gMapManager.InDevilSquare() ||
           gMapManager.WorldActive == WD_10HEAVEN ||
           gMapManager.InChaosCastle() ||
           gMapManager.InBattleCastle() ||
           M31HuntingGround::IsInHuntingGround() ||
           M33Aida::IsInAida() ||
           M34CryWolf1st::IsCyrWolf1st() ||
           gMapManager.WorldActive == WD_42CHANGEUP3RD_2ND ||
           IsIceCity() ||
           IsSantaTown() ||
           gMapManager.IsPKField() ||
           IsDoppelGanger2() ||
           gMapManager.IsEmpireGuardian1() ||
           gMapManager.IsEmpireGuardian2() ||
           gMapManager.IsEmpireGuardian3() ||
           gMapManager.IsEmpireGuardian4() ||
           IsUnitedMarketPlace();
}

static bool ShouldRenderLeaves()
{
    return (gMapManager.WorldActive == WD_2DEVIAS && HeroTile != 3 && HeroTile < 10) ||
           IsIceCity() ||
           IsSantaTown() ||
           gMapManager.IsPKField() ||
           IsDoppelGanger2() ||
           gMapManager.IsEmpireGuardian1() ||
           gMapManager.IsEmpireGuardian2() ||
           gMapManager.IsEmpireGuardian3() ||
           gMapManager.IsEmpireGuardian4() ||
           IsUnitedMarketPlace();
}

/**
 * @brief Performs one-time initialization when entering the main game scene.
 *
 * This function is called once when transitioning from character selection to the main game.
 * It performs the following tasks:
 * - Sends character selection to the game server
 * - Initializes UI systems (chat, party, guild, etc.)
 * - Sets up camera and input configuration
 * - Clears previous scene state and prepares for gameplay
 *
 * @note This function should only be called once per main scene entry.
 */
static void InitializeMainScene()
{
    g_pMainFrame->ResetSkillHotKey();

    MU_LOG_INFO(mu::log::Get("scenes"), "Character selected: slot {}", SelectedHero + 1);

    InitMainScene = true;

    MU_LOG_DEBUG(mu::log::Get("network"), "SendRequestJoinMapServer");

    CurrentProtocolState = REQUEST_JOIN_MAP_SERVER;
    SocketClient->ToGameServer()->SendSelectCharacter(MU_C16(CharactersClient[SelectedHero].ID));

    // Remember which character is in play so auto-reconnect can re-select it.
    ReconnectManager::Instance().CacheCharacter(CharactersClient[SelectedHero].ID);

    CSceneUICoordinator::Instance().CreateMainScene();

    g_Camera.Angle[2] = -45.f;

    ClearInput();
    InputEnable = false;
    TabInputEnable = false;
    InputTextWidth = 256;
    InputTextMax[0] = 42;
    InputTextMax[1] = 10;
    InputNumber = 2;
    for (int i = 0; i < MAX_WHISPER; i++)
    {
        g_pChatListBox->AddText(L"", L"", SEASON3B::TYPE_WHISPER_MESSAGE);
    }

    g_GuildNotice[0][0] = '\0';
    g_GuildNotice[1][0] = '\0';

    g_pPartyManager->Create();

    g_pChatListBox->ClearAll();
    g_pSystemLogBox->ClearAll();

    g_pSlideHelpMgr->Init();
    g_pUIMapName->Init();
    g_pNewUIMuHelper->Reset();

    g_GuildCache.Reset();
    g_PortalMgr.Reset();

    ClearAllObjectBlurs();

    SetFocus(g_hWnd);

    g_ErrorReport.Write(L"> Main Scene init success. ");
    g_ErrorReport.WriteCurrentTime();

    g_ConsoleDebug->Write(MCD_NORMAL, L"MainScene Init Success");
}

/**
 * @brief Resets per-frame state variables at the start of each frame.
 *
 * Initializes frame-dependent state including:
 * - Earthquake effect damping
 * - Terrain lighting
 * - UI interaction flags (inventory, skill checks, mouse window state)
 *
 * @note Called every frame during the main scene update loop.
 */
static void InitializeSceneFrame()
{
    EarthQuake *= 0.2f;
    InitTerrainLight();

    CheckInventory = NULL;
    CheckSkill = -1;
    MouseOnWindow = false;
}

/**
 * @brief Updates user interface and processes player input.
 *
 * Handles all UI-related updates and input processing including:
 * - Party system updates
 * - New UI system updates
 * - Mouse and keyboard input handling
 * - Window focus management
 * - Interface movement and tournament interface updates
 *
 * @note Only processes input when not in top-view camera mode and loading is complete.
 * @note Skips processing if g_Camera.TopViewEnable is true or LoadingWorld >= 30.
 */
static void UpdateUIAndInput()
{
    if (g_Camera.TopViewEnable || LoadingWorld >= 30)
        return;

    if (UI::Scaling::BottomHudContainsWindowPoint(WindowWidth, WindowHeight,
                                                  g_fWindowMouseX, g_fWindowMouseY))
        MouseOnWindow = true;

    g_pPartyManager->Update();
    g_pNewUISystem->Update();

    if (MouseLButton == true &&
        false == g_pNewUISystem->CheckMouseUse() &&
        g_dwMouseUseUIID == 0 &&
        g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHATINPUTBOX) == false)
    {
        g_pWindowMgr->SetWindowsEnable(FALSE);
        g_pFriendMenu->HideMenu();
        g_dwKeyFocusUIID = 0;
        if (GetFocus() != g_hWnd)
        {
            SaveIMEStatus();
            SetFocus(g_hWnd);
        }
    }

    MoveInterface();
    MoveTournamentInterface();

    if (ErrorMessage != MESSAGE_LOG_OUT)
        g_pUIManager->UpdateInput();
}

/**
 * @brief Updates all game entities and visual effects.
 *
 * Performs per-frame updates for all game world entities:
 * - World objects and items
 * - Environmental effects (leaves, boids, fish)
 * - Chat messages and player shops
 * - Player hero and other characters
 * - Mounts and pets
 * - Visual effects (particles, joints, pointers)
 * - Direction indicators
 *
 * @note Some updates are conditional based on camera mode (e.g., items only update when not in top-view).
 * @note Includes editor object updates when ENABLE_EDIT is defined.
 */
static void UpdateGameEntities()
{
    MoveObjects();

    if (!g_Camera.TopViewEnable)
        MoveItems();

    if (RequireLeavesEffect())
    {
        MoveLeaves();
    }

    MoveBoids();
    MoveFishs();
    UI::Chat::MoveChat();
    UpdatePersonalShopTitleImp();
    MoveHero();
    MoveCharactersClient();
    MoveMounts();
    ThePetProcess().UpdatePets();
    MovePoints();
    {
        FRAME_PROFILE(MoveEffects);
        MoveEffects();
    }
    MoveJoints();
    {
        FRAME_PROFILE(MoveParticles);
        MoveParticles();
    }
    MovePointers();

    g_Direction.CheckDirection();

#ifdef ENABLE_EDIT
    Editor::EditObjects();
#endif //ENABLE_EDIT
}

/**
 * @brief Main update function for the game scene.
 *
 * This is the primary per-frame update loop for the main gameplay scene.
 * It orchestrates initialization, server connection waiting, and frame updates by calling:
 * 1. InitializeMainScene() - One-time setup (first call only)
 * 2. Server join synchronization - Waits for server response before enabling rendering
 * 3. InitializeSceneFrame() - Per-frame state reset
 * 4. UpdateUIAndInput() - UI and input processing
 * 5. UpdateGameEntities() - Game world and entity updates
 *
 * @note Returns early if EnableMainRender is false (waiting for server join).
 */
void MoveMainScene()
{
    if (!InitMainScene)
    {
        InitializeMainScene();
    }

    if (CurrentProtocolState == RECEIVE_JOIN_MAP_SERVER)
    {
        EnableMainRender = true;
    }

    if (EnableMainRender == false)
    {
        return;
    }

    InitializeSceneFrame();

    // While the reconnect dialog is up it's modal: block world clicks so they
    // don't move the hero and instead reach the dialog's Cancel button.
    if (ReconnectManager::Instance().IsActive())
        MouseOnWindow = true;

    UpdateUIAndInput();

    if (ErrorMessage != 0)
        MouseOnWindow = true;

    UpdateGameEntities();

    g_ConsoleDebug->UpdateMainScene();
}

/**
 * @brief Sets up OpenGL viewport and clear color for main scene.
 *
 * @param outWidth Output screen width
 * @param outHeight Output screen height
 * @param outByWaterMap Output water map flag (0=normal, 1=hellas water, 2=water terrain)
 * @param cameraPos Camera position for frustum
 */
static void SetupMainSceneViewport(int& outWidth, int& outHeight, BYTE& outByWaterMap, vec3_t cameraPos)
{
    outByWaterMap = 0;

    const auto viewport = UI::Scaling::WorldViewport(WindowWidth, WindowHeight, g_Camera.TopViewEnable);
    outWidth = viewport.width;
    outHeight = viewport.height;

    // NOTE: Clear color is set by SceneManager::SetWorldClearColor() before this function is called
    // All background colors are now centralized in SceneManager.cpp

    BeginOpenglPhysical(viewport.x, viewport.y, viewport.width, viewport.height);
    CreateFrustrum(static_cast<float>(viewport.width) / WindowWidth,
                   static_cast<float>(viewport.height) / WindowHeight, cameraPos);

    // Setup fog for battle castle
    if (gMapManager.InBattleCastle())
    {
        if (battleCastle::InBattleCastle2(Hero->Object.Position))
        {
            vec3_t Color = { 0.f, 0.f, 0.f };
            battleCastle::StartFog(Color);
        }
        // Don't disable fog - let BeginOpengl() handle it based on FogEnable
    }
    CameraProjection::ScreenToWorldRay(g_Camera, MouseX, MouseY, MouseTarget);
}

// DXP-23 diagnostic toggle -- see MainScene.h's SetDisableEffects() doc comment.
static bool g_bDisableEffectsDebug = false;

void SetDisableEffects(bool disabled)
{
    g_bDisableEffectsDebug = disabled;
}

// DXP-23 diagnostic toggles, finer-grained bisection -- see MainScene.h doc comments.
static bool g_bDisableSpritesDebug = false;
static bool g_bDisableParticlesDebug = false;
static bool g_bDisableSkillEffectModelsDebug = false;
static bool g_bDisableBoidsDebug = false;

void SetDisableSprites(bool disabled)
{
    g_bDisableSpritesDebug = disabled;
}
void SetDisableParticles(bool disabled)
{
    g_bDisableParticlesDebug = disabled;
}
void SetDisableSkillEffectModels(bool disabled)
{
    g_bDisableSkillEffectModelsDebug = disabled;
}
void SetDisableBoids(bool disabled)
{
    g_bDisableBoidsDebug = disabled;
}
bool IsSpritesDisabledDebug()
{
    return g_bDisableSpritesDebug;
}
bool IsParticlesDisabledDebug()
{
    return g_bDisableParticlesDebug;
}
bool IsSkillEffectModelsDisabledDebug()
{
    return g_bDisableSkillEffectModelsDebug;
}
bool IsBoidsDisabledDebug()
{
    return g_bDisableBoidsDebug;
}

static bool g_bDisableWingShadowDebug = false;
void SetDisableWingShadow(bool disabled)
{
    g_bDisableWingShadowDebug = disabled;
}
bool IsWingShadowDisabledDebug()
{
    return g_bDisableWingShadowDebug;
}

static bool g_bDisableJointsDebug = false;
void SetDisableJoints(bool disabled)
{
    g_bDisableJointsDebug = disabled;
}
bool IsJointsDisabledDebug()
{
    return g_bDisableJointsDebug;
}

static bool g_bDisableWingExtraLayersDebug = false;
void SetDisableWingExtraLayers(bool disabled)
{
    g_bDisableWingExtraLayersDebug = disabled;
}
bool IsWingExtraLayersDisabledDebug()
{
    return g_bDisableWingExtraLayersDebug;
}

/**
 * @brief Renders all 3D game entities (terrain, objects, characters, effects).
 *
 * @param byWaterMap Water map mode flag (passed by reference, may be modified)
 * @param width Screen width for water terrain rendering
 * @param height Screen height for water terrain rendering
 */
static void RenderGameWorld(BYTE& byWaterMap, int width, int height)
{
#ifdef _EDITOR
    // DevEditor render toggle checks
    bool renderTerrain = DevEditor_ShouldRenderTerrain();
    bool renderStatic = DevEditor_ShouldRenderStaticObjects();
    bool renderEffects = DevEditor_ShouldRenderEffects() && !g_bDisableEffectsDebug;
    bool renderDroppedItems = DevEditor_ShouldRenderDroppedItems();
    bool renderWeatherEffects = DevEditor_ShouldRenderWeatherEffects();
#else
    bool renderTerrain = true;
    bool renderStatic = true;
    bool renderEffects = !g_bDisableEffectsDebug;
    bool renderDroppedItems = true;
    bool renderWeatherEffects = true;
#endif

    if (IsWaterTerrain() == false && renderTerrain)
    {
        if (gMapManager.WorldActive == WD_39KANTURU_3RD)
        {
            if (!g_Direction.m_CKanturu.IsMayaScene())
                { FRAME_PROFILE(Terrain); RenderTerrain(false); }
        }
        else
            if (gMapManager.WorldActive != WD_10HEAVEN && gMapManager.WorldActive != -1)
            {
                if ((gMapManager.IsPKField() || IsDoppelGanger2()) && renderStatic)
                {
                    FRAME_PROFILE(Objects); RenderObjects();
                }
                { FRAME_PROFILE(Terrain); RenderTerrain(false); }
            }
    }

    if (!gMapManager.IsPKField() && !IsDoppelGanger2() && renderStatic)
        { FRAME_PROFILE(Objects); RenderObjects(); }

    if (renderEffects)
    {
        RenderEffectShadows();
        RenderBoids();
    }

    { FRAME_PROFILE(Characters); RenderCharactersClient(); }

    if (EditFlag != EDIT_NONE && renderTerrain)
    {
        FRAME_PROFILE(Terrain); RenderTerrain(true);
    }
    if (!g_Camera.TopViewEnable && renderDroppedItems)
        { FRAME_PROFILE(Items); RenderItems(); }

    RenderFishs();
    RenderMount();

    if (renderWeatherEffects)
        RenderLeaves();

    if (!gMapManager.InChaosCastle())
        ThePetProcess().RenderPets();

    if (renderEffects)
        RenderBoids(true);

    if (renderStatic)
        { FRAME_PROFILE(Objects); RenderObjects_AfterCharacter(); }

    { FRAME_PROFILE(Joints); RenderJoints(byWaterMap); }

    if (renderEffects)
    {
        FRAME_PROFILE(Effects);
        RenderEffects();
        RenderBlurs();
    }
    CheckSprites();
    BeginSprite();

    if (ShouldRenderLeaves())
    {
        RenderLeaves();
    }

    { FRAME_PROFILE(Sprites); RenderSprites(); }
    { FRAME_PROFILE(Particles); RenderParticles(); }

    if (IsWaterTerrain() == false)
    {
        RenderPoints(byWaterMap);
    }

    EndSprite();

    RenderAfterEffects();

    if (IsWaterTerrain() == true)
    {
        byWaterMap = 2;

        EndOpengl();
        BeginOpenglPhysical(0, 0, width, height);
        RenderWaterTerrain();
        { FRAME_PROFILE(Joints); RenderJoints(byWaterMap); }
        { FRAME_PROFILE(Effects); RenderEffects(true); RenderBlurs(); }
        CheckSprites();
        BeginSprite();

        if (gMapManager.WorldActive == WD_2DEVIAS && HeroTile != 3 && HeroTile < 10)
            RenderLeaves();

        { FRAME_PROFILE(Sprites); RenderSprites(byWaterMap); }
        { FRAME_PROFILE(Particles); RenderParticles(byWaterMap); }
        RenderPoints(byWaterMap);

        EndSprite();
        EndOpengl();

        BeginOpenglPhysical(0, 0, width, height);
    }

    if (gMapManager.InBattleCastle())
    {
        if (battleCastle::InBattleCastle2(Hero->Object.Position))
        {
            battleCastle::EndFog();
        }
    }
}

/**
 * @brief Renders UI elements and overlays for the main scene.
 */
static void RenderMainSceneUI()
{
    Input::Selection::SelectObjects();
    BeginBitmap();
    RenderObjectDescription();

    if (g_Camera.TopViewEnable == false)
    {
        RenderInterface(true);
    }
    RenderTournamentInterface();
    EndBitmap();

    g_pPartyManager->Render();
    g_pNewUISystem->Render();

    BeginBitmap();
    RenderInfomation();

#ifdef ENABLE_EDIT
    RenderDebugWindow();
#endif //ENABLE_EDIT

    EndBitmap();

    // RenderCursor() used to be called here directly, but that runs as part of this function's
    // normal legacy-2D pass -- earlier in the frame than RmlUi's own render pass (RmlUi always
    // renders last, see docs/rmlui-ui-system/README.md's Frame lifecycle section), so any RmlUi
    // content on screen during gameplay (the ESC menu, and this scene's own RmlUi HUD pilot)
    // would paint over the cursor. Now drawn from Winmain.cpp's SetPostRmlUiCallback instead,
    // which fires after RmlUi's pass and (as of 2026-08-31) covers MAIN_SCENE too -- same fix
    // already applied to LOG_IN_SCENE/CHARACTER_SCENE when their own RmlUi content first shipped.
}

/**
 * @brief Main rendering function for the game scene.
 *
 * Orchestrates the complete rendering pipeline:
 * 1. Determines camera position based on camera mode
 * 2. Sets up viewport and clear color
 * 3. Renders 3D world (terrain, objects, characters, effects)
 * 4. Renders UI and overlays
 *
 * @return true if rendering succeeded, false if rendering was skipped
 */
bool RenderMainScene()
{
    if (EnableMainRender == false)
    {
        return false;
    }

    if ((LoadingWorld) > 30)
    {
        // Keep something on screen every frame while the world/hero data isn't back from the
        // server yet. LoadingScene.cpp's own flash only drew one frame (SceneFlag flips to
        // MAIN_SCENE immediately after it, well before LoadingWorld actually drops below 30) --
        // without a fresh clear here each of the frames in between, the swapchain just kept
        // presenting whatever stale buffer content was left, seen as flicker between black and
        // that one flash frame. The RmlUi loading wallpaper (kept Shown by LoadingScene(), not
        // closed after its flash) renders on top of this clear automatically, same as any other
        // persistent RmlUi document.
        BeginOpengl();
        mu::GetRenderer().ClearScreen();
        EndOpengl();
        return false;
    }

    // World is ready this frame -- close the loading wallpaper now, before it (or the mainframe
    // HUD it was gating alongside, see CNewUISystem::SyncMainSceneHudVisibility) would otherwise
    // sit on top of the real scene about to render below.
    HideLoadingSceneOverlay();

    // Per-camera fog default: Orbital uses fog (noticeable at longer view distances),
    // Default camera's fog zone sits at/beyond its far clip and reads as visual noise,
    // so fog is off by default for Default. DevEditor can override either below.
    if (ICamera* active = CameraManager::Instance().GetActiveCamera())
    {
        const char* name = active->GetName();
        if (strcmp(name, "Default") == 0)       FogEnable = false;
        else if (strcmp(name, "Orbital") == 0)  FogEnable = true;
    }

#ifdef _EDITOR
    // DevEditor override: allow forcing fog on/off for debugging.
    if (ICamera* active = CameraManager::Instance().GetActiveCamera())
    {
        const char* name = active->GetName();
        if (DevEditor_IsCameraFogOverrideEnabled(name))
            FogEnable = DevEditor_GetCameraFogOverrideValue(name);
    }
#endif

    vec3_t cameraPos;
    int width, height;
    BYTE byWaterMap;

    // Determine camera position
    if (MoveMainCamera() == true)
    {
        VectorCopy(Hero->Object.StartPosition, cameraPos);
    }
    else
    {
        g_pCatapultWindow->GetCameraPos(cameraPos);

        if (g_Direction.IsDirection() && g_Direction.m_bDownHero == false)
        {
            g_Direction.GetCameraPosition(cameraPos);
        }
    }

    SetupMainSceneViewport(width, height, byWaterMap, cameraPos);
    RenderGameWorld(byWaterMap, width, height);

#ifdef _EDITOR
    // Render spectated camera frustum wireframe when in FreeFly mode
    CameraMode cameraMode = CameraManager::Instance().GetCurrentMode();
    if (cameraMode == CameraMode::FreeFly)
    {
        ICamera* spectated = CameraManager::Instance().GetSpectatedCamera();
        if (spectated)
            RenderFrustumWireframe(spectated->GetFrustum());
    }
#endif

    {
        FRAME_PROFILE(UI);
        RenderMainSceneUI();
    }

    EndOpengl();

    return true;
}
