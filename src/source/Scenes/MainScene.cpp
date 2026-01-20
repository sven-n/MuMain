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

// External declarations
extern float CameraAngle[3];
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

    CameraAngle[2] = -45.f;

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
 * @note Skips processing if CameraTopViewEnable is true or LoadingWorld >= 30.
 */
static void UpdateUIAndInput()
{
    if (CameraTopViewEnable || LoadingWorld >= 30)
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

    if (!CameraTopViewEnable)
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

    FogEnable = false;

    vec3_t pos;

    if (MoveMainCamera() == true)
    {
        VectorCopy(Hero->Object.StartPosition, pos);
    }
    else
    {
        g_pCatapultWindow->GetCameraPos(pos);

        if (g_Direction.IsDirection() && g_Direction.m_bDownHero == false)
        {
            g_Direction.GetCameraPosition(pos);
        }
    }

    int Width, Height;

    BYTE byWaterMap = 0;

    if (CameraTopViewEnable == false)
    {
        Height = 480 - 48;
    }
    else
    {
        Height = 480;
    }

    Width = GetScreenWidth();
    if (gMapManager.WorldActive == WD_0LORENCIA)
    {
        glClearColor(10 / 256.f, 20 / 256.f, 14 / 256.f, 1.f);
    }
    else if (gMapManager.WorldActive == WD_2DEVIAS)
    {
        glClearColor(0.f / 256.f, 0.f / 256.f, 10.f / 256.f, 1.f);
    }
    else if (gMapManager.WorldActive == WD_10HEAVEN)
    {
        glClearColor(3.f / 256.f, 25.f / 256.f, 44.f / 256.f, 1.f);
    }
    else if (gMapManager.InChaosCastle() == true)
    {
        glClearColor(0 / 256.f, 0 / 256.f, 0 / 256.f, 1.f);
    }
    else if (gMapManager.WorldActive >= WD_45CURSEDTEMPLE_LV1 && gMapManager.WorldActive <= WD_45CURSEDTEMPLE_LV6)
    {
        glClearColor(9.f / 256.f, 8.f / 256.f, 33.f / 256.f, 1.f);
    }
    else if (gMapManager.InHellas() == true)
    {
        byWaterMap = 1;
        glClearColor(0.f / 256.f, 0.f / 256.f, 0.f / 256.f, 1.f);
    }
    else
    {
        glClearColor(0 / 256.f, 0 / 256.f, 0 / 256.f, 1.f);
    }

    BeginOpengl(0, 0, Width, Height);

    CreateFrustrum((float)Width / (float)640, (float)Height / 480.f, pos);

    if (gMapManager.InBattleCastle())
    {
        if (battleCastle::InBattleCastle2(Hero->Object.Position))
        {
            vec3_t Color = { 0.f, 0.f, 0.f };
            battleCastle::StartFog(Color);
        }
        else
        {
            glDisable(GL_FOG);
        }
    }

    CreateScreenVector(MouseX, MouseY, MouseTarget);

    if (IsWaterTerrain() == false)
    {
        if (gMapManager.WorldActive == WD_39KANTURU_3RD)
        {
            if (!g_Direction.m_CKanturu.IsMayaScene())
                RenderTerrain(false);
        }
        else
            if (gMapManager.WorldActive != WD_10HEAVEN && gMapManager.WorldActive != -1)
            {
                if (gMapManager.IsPKField() || IsDoppelGanger2())
                {
                    RenderObjects();
                }
                RenderTerrain(false);
            }
    }

    if (!gMapManager.IsPKField() && !IsDoppelGanger2())
        RenderObjects();

    RenderEffectShadows();
    RenderBoids();

    RenderCharactersClient();

    if (EditFlag != EDIT_NONE)
    {
        RenderTerrain(true);
    }
    if (!CameraTopViewEnable)
        RenderItems();

    RenderFishs();
    RenderMount();
    RenderLeaves();

    if (!gMapManager.InChaosCastle())
        ThePetProcess().RenderPets();

    RenderBoids(true);
    RenderObjects_AfterCharacter();

    RenderJoints(byWaterMap);
    RenderEffects();
    RenderBlurs();
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
        BeginOpengl(0, 0, Width, Height);
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

        BeginOpengl(0, 0, Width, Height);
    }

    if (gMapManager.InBattleCastle())
    {
        if (battleCastle::InBattleCastle2(Hero->Object.Position))
        {
            battleCastle::EndFog();
        }
    }

    SelectObjects();
    BeginBitmap();
    RenderObjectDescription();

    if (CameraTopViewEnable == false)
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
    EndOpengl();

    return true;
}
