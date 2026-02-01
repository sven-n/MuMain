///////////////////////////////////////////////////////////////////////////////
// MainScene.cpp - Main game scene implementation
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainScene.h"
#include "SceneCommon.h"
#include "../Camera/CameraUtility.h"
#include "../ZzzOpenglUtil.h"
#include "../ZzzObject.h"
#include "../ZzzCharacter.h"
#include "../ZzzLodTerrain.h"
#include "../ZzzInterface.h"
#include "../ZzzEffect.h"
#include "../MapManager.h"
#include "../UIMng.h"
#include "../NewUISystem.h"
#include "../PartyManager.h"
#include "../CDirection.h"
#include "../w_PetProcess.h"
#include "../Utilities/Log/muConsoleDebug.h"
#include "../ZzzInterface.h"
#include "../WSclient.h"
#include "../GOBoid.h"
#include "../PersonalShopTitleImp.h"
#include "../UIManager.h"
#include "../ZzzInventory.h"
#include "../PortalMgr.h"
#include "../Guild/GuildCache.h"
#include "../UIMapName.h"
#include "Camera/CameraProjection.h"
#include "Camera/CameraManager.h"

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
#endif

extern HWND g_hWnd;
extern CErrorReport g_ErrorReport;
extern float EarthQuake;
extern int CheckSkill;
extern int MouseY;
extern int LoadingWorld;
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

    g_ConsoleDebug->Write(MCD_NORMAL, L"Join the game with the following character: %ls", CharactersClient[SelectedHero].ID);
    g_ErrorReport.Write(L"> Character selected <%d> \"%ls\"\r\n", SelectedHero + 1, CharactersClient[SelectedHero].ID);

    InitMainScene = true;

    g_ConsoleDebug->Write(MCD_SEND, L"SendRequestJoinMapServer");

    CurrentProtocolState = REQUEST_JOIN_MAP_SERVER;
    SocketClient->ToGameServer()->SendSelectCharacter(CharactersClient[SelectedHero].ID);

    CUIMng::Instance().CreateMainScene();

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

    if (MouseY >= (int)(480 - 48))
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
    MoveChat();
    UpdatePersonalShopTitleImp();
    MoveHero();
    MoveCharactersClient();
    MoveMounts();
    ThePetProcess().UpdatePets();
    MovePoints();
    MoveEffects();
    MoveJoints();
    MoveParticles();
    MovePointers();

    g_Direction.CheckDirection();

#ifdef ENABLE_EDIT
    EditObjects();
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
    UpdateUIAndInput();

    if (ErrorMessage != NULL)
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

    if (g_Camera.TopViewEnable == false)
    {
        outHeight = 480 - 48;
    }
    else
    {
        outHeight = 480;
    }

    outWidth = GetScreenWidth();

    // NOTE: Clear color is set by SceneManager::SetWorldClearColor() before this function is called
    // All background colors are now centralized in SceneManager.cpp

    BeginOpengl(0, 0, outWidth, outHeight);

    // Note: Frustum is now updated by camera system in RenderGameWorld()
    // No need to call CreateFrustrum() here anymore

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

/**
 * @brief Renders all 3D game entities (terrain, objects, characters, effects).
 *
 * @param byWaterMap Water map mode flag (passed by reference, may be modified)
 * @param width Screen width for water terrain rendering
 * @param height Screen height for water terrain rendering
 */
static void RenderGameWorld(BYTE& byWaterMap, int width, int height)
{
    // Phase 3: Get active camera for direct culling
    ICamera* activeCamera = CameraManager::Instance().GetActiveCamera();

#ifdef _EDITOR
    // DevEditor render toggle checks
    bool renderTerrain = DevEditor_ShouldRenderTerrain();
    bool renderStatic = DevEditor_ShouldRenderStaticObjects();
    bool renderEffects = DevEditor_ShouldRenderEffects();
    bool renderDroppedItems = DevEditor_ShouldRenderDroppedItems();
    bool renderWeatherEffects = DevEditor_ShouldRenderWeatherEffects();
#else
    bool renderTerrain = true;
    bool renderStatic = true;
    bool renderEffects = true;
    bool renderDroppedItems = true;
    bool renderWeatherEffects = true;
#endif

    // NOTE: RenderSky() disabled - BITMAP_SKY texture doesn't exist in codebase
    // Sky appearance is controlled by glClearColor() set earlier

    if (IsWaterTerrain() == false && renderTerrain)
    {
        if (gMapManager.WorldActive == WD_39KANTURU_3RD)
        {
            if (!g_Direction.m_CKanturu.IsMayaScene())
                RenderTerrain(false, activeCamera);
        }
        else
            if (gMapManager.WorldActive != WD_10HEAVEN && gMapManager.WorldActive != -1)
            {
                if ((gMapManager.IsPKField() || IsDoppelGanger2()) && renderStatic)
                {
                    RenderObjects(activeCamera);
                }
                RenderTerrain(false, activeCamera);
            }
    }

    if (!gMapManager.IsPKField() && !IsDoppelGanger2() && renderStatic)
        RenderObjects(activeCamera);

    if (renderEffects)
    {
        RenderEffectShadows();
        RenderBoids();
    }

    RenderCharactersClient();

    if (EditFlag != EDIT_NONE && renderTerrain)
    {
        RenderTerrain(true);
    }
    if (!g_Camera.TopViewEnable && renderDroppedItems)
        RenderItems();

    RenderFishs();
    RenderMount();

    if (renderWeatherEffects)
        RenderLeaves();

    if (!gMapManager.InChaosCastle())
        ThePetProcess().RenderPets();

    if (renderEffects)
        RenderBoids(true);

    if (renderStatic)
        RenderObjects_AfterCharacter(activeCamera);

    RenderJoints(byWaterMap);

    if (renderEffects)
    {
        RenderEffects();
        RenderBlurs();
    }
    CheckSprites();
    BeginSprite();

    if (ShouldRenderLeaves())
    {
        RenderLeaves();
    }

    RenderSprites();
    RenderParticles();

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
        BeginOpengl(0, 0, width, height);
        RenderWaterTerrain();
        RenderJoints(byWaterMap);
        RenderEffects(true);
        RenderBlurs();
        CheckSprites();
        BeginSprite();

        if (gMapManager.WorldActive == WD_2DEVIAS && HeroTile != 3 && HeroTile < 10)
            RenderLeaves();

        RenderSprites(byWaterMap);
        RenderParticles(byWaterMap);
        RenderPoints(byWaterMap);

        EndSprite();
        EndOpengl();

        BeginOpengl(0, 0, width, height);
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
    SelectObjects();
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
    BeginBitmap();

    RenderCursor();

    EndBitmap();
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
        return false;
    }

    // Enable fog with colors matching background for atmospheric depth
    FogEnable = true;

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
    RenderMainSceneUI();

    // Debug: Draw frustum trapezoid visualization on top of everything (if enabled)
#ifdef _EDITOR
    if (DevEditor_IsDebugVisualizationEnabled())
    {
        RenderFrustrum2DDebug();
    }
#endif

    EndOpengl();

    return true;
}
