///////////////////////////////////////////////////////////////////////////////
// SceneManager.cpp - Scene management and rendering orchestration
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <vector>
#include "SceneManager.h"

//=============================================================================
// Frame Timing State Implementation
//=============================================================================

// Global instance
FrameTimingState g_frameTiming;

//=============================================================================
// Scene Manager Implementation
//=============================================================================
#include "SceneCommon.h"
#include "WebzenScene.h"
#include "LoginScene.h"
#include "CharacterScene.h"
#include "MainScene.h"
#include "../LoadingScene.h"
#include "../DSPlaySound.h"
#include "../ZzzOpenglUtil.h"
#include "../PhysicsManager.h"
#include "../Time/Timer.h"
#include "../Input.h"
#include "../UIMng.h"
#include "../WSclient.h"
#include "../w_CursedTemple.h"
#include "../ServerListManager.h"
#include "../NewUISystem.h"
#include "../ZzzInterface.h"
#include "../GlobalText.h"
#include "../ZzzAI.h"
#include "../Winmain.h"

// External declarations
extern int GrabScreen;
extern int WaterTextureNumber;
extern float g_Luminosity;
extern int g_iNoMouseTime;
extern CPhysicsManager g_PhysicsManager;
extern EGameScene SceneFlag;
extern CTimer* g_pTimer;
extern DWORD g_dwMouseUseUIID;
extern wchar_t GrabFileName[256];
extern CHARACTER* Hero;
extern int HeroTile;
extern bool Destroy;
extern double WorldTime;
extern float FPS_ANIMATION_FACTOR;

void SetTargetFps(double targetFps)
{
    if (IsVSyncEnabled() && targetFps >= GetFPSLimit())
    {
        targetFps = -1;
    }

    g_frameTiming.SetTargetFps(targetFps);
}

double GetTargetFps()
{
    return g_frameTiming.GetTargetFps();
}

void UpdateSceneState()
{
    g_pNewKeyInput->ScanAsyncKeyState();

    g_dwMouseUseUIID = 0;

    switch (SceneFlag)
    {
    case LOG_IN_SCENE:
        NewMoveLogInScene();
        break;

    case CHARACTER_SCENE:
        NewMoveCharacterScene();
        break;

    case MAIN_SCENE:
        MoveMainScene();
        break;
    }

    MoveNotices();

    if (PressKey(VK_SNAPSHOT))
    {
        if (GrabEnable)
            GrabEnable = false;
        else
            GrabEnable = true;
    }

    const bool addTimeStampToCapture = !HIBYTE(GetAsyncKeyState(VK_SHIFT));
    wchar_t screenshotText[256];
    if (GrabEnable)
    {
        SYSTEMTIME st;
        GetLocalTime(&st);
        swprintf(GrabFileName, L"Screen(%02d_%02d-%02d_%02d)-%04d.jpg", st.wMonth, st.wDay, st.wHour, st.wMinute, GrabScreen);
        swprintf(screenshotText, GlobalText[459], GrabFileName);
        wchar_t lpszTemp[64];
        swprintf(lpszTemp, L" [%ls / %ls]", g_ServerListManager->GetSelectServerName(), Hero->ID);
        wcscat(screenshotText, lpszTemp);
        if (addTimeStampToCapture)
        {
            g_pSystemLogBox->AddText(screenshotText, SEASON3B::TYPE_SYSTEM_MESSAGE);
        }

        std::vector<unsigned char> Buffer(WindowWidth * WindowHeight * 3);

        glReadPixels(0, 0, WindowWidth, WindowHeight, GL_RGB, GL_UNSIGNED_BYTE, Buffer.data());

        WriteJpeg(GrabFileName, WindowWidth, WindowHeight, Buffer.data(), 100);

        GrabScreen++;
        GrabScreen %= 10000;
    }

    if (GrabEnable && !addTimeStampToCapture)
    {
        g_pSystemLogBox->AddText(screenshotText, SEASON3B::TYPE_SYSTEM_MESSAGE);
    }

    GrabEnable = false;
}

/**
 * @brief Updates UI and input systems for login and character scenes.
 *
 * @param dDeltaTick Time delta for frame updates
 */
static void UpdateLoginAndCharacterScenes()
{
    double dDeltaTick = g_pTimer->GetTimeElapsed();
    dDeltaTick = MIN(dDeltaTick, 200.0 * FPS_ANIMATION_FACTOR);

    CInput::Instance().Update();
    CUIMng::Instance().Update(dDeltaTick);
}

/**
 * @brief Updates water animation texture cycling.
 *
 * Advances water texture animation based on elapsed time at reference FPS rate.
 */
static void UpdateWaterAnimation()
{
    constexpr int NumberOfWaterTextures = 32;
    const double timePerFrame = 1000 / REFERENCE_FPS;
    auto time_since_last_render = g_frameTiming.currentTickCount - g_frameTiming.lastWaterChange;
    while (time_since_last_render > timePerFrame)
    {
        WaterTextureNumber++;
        WaterTextureNumber %= NumberOfWaterTextures;
        time_since_last_render -= timePerFrame;
        g_frameTiming.lastWaterChange = g_frameTiming.currentTickCount;
    }
}

/**
 * @brief Updates core game systems (physics, bitmaps, audio positioning).
 */
static void UpdateCoreSystems()
{
    g_PhysicsManager.Move(0.025f * FPS_ANIMATION_FACTOR);
    Bitmaps.Manage();
    Set3DSoundPosition();
}

/**
 * @brief Sets the OpenGL clear color based on the current world/map.
 *
 * Different maps have different background colors for visual atmosphere.
 */
static void SetWorldClearColor()
{
    if (gMapManager.WorldActive == WD_10HEAVEN)
    {
        glClearColor(3.f / 256.f, 25.f / 256.f, 44.f / 256.f, 1.f);
    }
    else if (gMapManager.WorldActive == WD_73NEW_LOGIN_SCENE || gMapManager.WorldActive == WD_74NEW_CHARACTER_SCENE)
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    }
    else if (gMapManager.InHellas(gMapManager.WorldActive))
    {
        glClearColor(30.f / 256.f, 40.f / 256.f, 40.f / 256.f, 1.f);
    }
    else if (gMapManager.InChaosCastle() == true)
    {
        glClearColor(0.f, 0.f, 0.f, 1.f);
    }
    else if (gMapManager.InBattleCastle() && battleCastle::InBattleCastle2(Hero->Object.Position))
    {
        glClearColor(0.f, 0.f, 0.f, 1.f);
    }
    else if (gMapManager.WorldActive >= WD_45CURSEDTEMPLE_LV1 && gMapManager.WorldActive <= WD_45CURSEDTEMPLE_LV6)
    {
        glClearColor(9.f / 256.f, 8.f / 256.f, 33.f / 256.f, 1.f);
    }
    else if (gMapManager.WorldActive == WD_51HOME_6TH_CHAR
        )
    {
        glClearColor(178.f / 256.f, 178.f / 256.f, 178.f / 256.f, 1.f);
    }
    else if (gMapManager.WorldActive == WD_65DOPPLEGANGER1)
    {
        glClearColor(148.f / 256.f, 179.f / 256.f, 223.f / 256.f, 1.f);
    }
    else
    {
        glClearColor(0 / 256.f, 0 / 256.f, 0 / 256.f, 1.f);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

/**
 * @brief Renders the appropriate scene based on current SceneFlag.
 *
 * @param hDC Device context for rendering
 * @return true if rendering succeeded, false otherwise
 */
static bool RenderCurrentScene(HDC hDC)
{
    bool Success = false;

    if (SceneFlag == LOG_IN_SCENE)
    {
        Success = NewRenderLogInScene(hDC);
    }
    else if (SceneFlag == CHARACTER_SCENE)
    {
        Success = NewRenderCharacterScene(hDC);
    }
    else if (SceneFlag == MAIN_SCENE)
    {
        Success = RenderMainScene();
    }

    g_PhysicsManager.Render();
    return Success;
}

/**
 * @brief Renders debug information overlay in development builds.
 *
 * Shows FPS, mouse position, and camera info on screen.
 */
static void RenderDebugInfo()
{
#if defined(_DEBUG) || defined(LDS_FOR_DEVELOPMENT_TESTMODE) || defined(LDS_UNFIXED_FIXEDFRAME_FORDEBUG)
    BeginBitmap();
    wchar_t szDebugText[128];
    swprintf(szDebugText, L"FPS: %.1f Vsync: %d CPU: %.1f%%", FPS_AVG, IsVSyncEnabled(), CPU_AVG);
    wchar_t szMousePos[128];
    swprintf(szMousePos, L"MousePos : %d %d %d", MouseX, MouseY, MouseLButtonPush);
    wchar_t szCamera3D[128];
    swprintf(szCamera3D, L"Camera3D : %.1f %.1f:%.1f:%.1f", CameraFOV, CameraAngle[0], CameraAngle[1], CameraAngle[2]);
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetBgColor(0, 0, 0, 100);
    g_pRenderText->SetTextColor(255, 255, 255, 200);
    g_pRenderText->RenderText(10, 26, szDebugText);
    g_pRenderText->RenderText(10, 36, szMousePos);
    g_pRenderText->RenderText(10, 46, szCamera3D);
    g_pRenderText->SetFont(g_hFont);
    EndBitmap();
#endif // defined(_DEBUG) || defined(LDS_FOR_DEVELOPMENT_TESTMODE) || defined(LDS_UNFIXED_FIXEDFRAME_FORDEBUG)
}

/**
 * @brief Checks and handles server connection loss.
 */
static void CheckServerConnection()
{
    if (SocketClient == nullptr || !SocketClient->IsConnected())
    {
        static BOOL s_bClosed = FALSE;
        if (!s_bClosed)
        {
            s_bClosed = TRUE;
            g_ErrorReport.Write(L"> Connection closed. ");
            g_ErrorReport.WriteCurrentTime();
            g_ConsoleDebug->Write(MCD_NORMAL, L"Connection closed");
            CUIMng::Instance().PopUpMsgWin(MESSAGE_SERVER_LOST);
        }
    }
}

/**
 * @brief Plays ambient sound effects for the current world/map.
 *
 * Handles world-specific ambient sounds like wind, rain, desert, water, etc.
 */
static void PlayWorldAmbientSounds()
{
    switch (gMapManager.WorldActive)
    {
    case WD_0LORENCIA:
                if (HeroTile == 4)
                {
                    StopBuffer(SOUND_WIND01, true);
                    StopBuffer(SOUND_RAIN01, true);
                }
                else
                {
                    PlayBuffer(SOUND_WIND01, NULL, true);
                    if (RainCurrent > 0)
                        PlayBuffer(SOUND_RAIN01, NULL, true);
                }
                break;
            case WD_1DUNGEON:
                PlayBuffer(SOUND_DUNGEON01, NULL, true);
                break;
            case WD_2DEVIAS:
                if (HeroTile == 3 || HeroTile >= 10)
                    StopBuffer(SOUND_WIND01, true);
                else
                    PlayBuffer(SOUND_WIND01, NULL, true);
                break;
            case WD_3NORIA:
                PlayBuffer(SOUND_WIND01, NULL, true);
                if (rand_fps_check(512))
                    PlayBuffer(SOUND_FOREST01);
                break;
            case WD_4LOSTTOWER:
                PlayBuffer(SOUND_TOWER01, NULL, true);
                break;
            case WD_5UNKNOWN:
                //PlayBuffer(SOUND_BOSS01,NULL,true);
                break;
            case WD_7ATLANSE:
                PlayBuffer(SOUND_WATER01, NULL, true);
                break;
            case WD_8TARKAN:
                PlayBuffer(SOUND_DESERT01, NULL, true);
                break;
            case WD_10HEAVEN:
                PlayBuffer(SOUND_HEAVEN01, NULL, true);
                if (rand_fps_check(100))
                {
                    //                PlayBuffer(SOUND_HEAVEN01);
                }
                else if (rand_fps_check(10))
                {
                    //                PlayBuffer(SOUND_THUNDERS02);
                }
                break;
            case WD_58ICECITY_BOSS:
                PlayBuffer(SOUND_WIND01, NULL, true);
                break;
            case WD_79UNITEDMARKETPLACE:
            {
                PlayBuffer(SOUND_WIND01, NULL, true);
                PlayBuffer(SOUND_RAIN01, NULL, true);
            }
            break;
#ifdef ASG_ADD_MAP_KARUTAN
            case WD_80KARUTAN1:
                PlayBuffer(SOUND_KARUTAN_DESERT_ENV, NULL, true);
                break;
            case WD_81KARUTAN2:
                if (HeroTile == 12)
                {
                    StopBuffer(SOUND_KARUTAN_DESERT_ENV, true);
                    PlayBuffer(SOUND_KARUTAN_KARDAMAHAL_ENV, NULL, true);
                }
                else
                {
                    StopBuffer(SOUND_KARUTAN_KARDAMAHAL_ENV, true);
                    PlayBuffer(SOUND_KARUTAN_DESERT_ENV, NULL, true);
                }
                break;
#endif	// ASG_ADD_MAP_KARUTAN
    }
}

/**
 * @brief Stops ambient sounds that don't belong to the current world.
 *
 * Ensures only the current world's ambient sounds are playing.
 */
static void StopInactiveAmbientSounds()
{
    if (gMapManager.WorldActive != WD_0LORENCIA && gMapManager.WorldActive != WD_2DEVIAS && gMapManager.WorldActive != WD_3NORIA && gMapManager.WorldActive != WD_58ICECITY_BOSS && gMapManager.WorldActive != WD_79UNITEDMARKETPLACE)
    {
        StopBuffer(SOUND_WIND01, true);
    }
    if (gMapManager.WorldActive != WD_0LORENCIA && gMapManager.InDevilSquare() == false && gMapManager.WorldActive != WD_79UNITEDMARKETPLACE)
    {
        StopBuffer(SOUND_RAIN01, true);
    }
    if (gMapManager.WorldActive != WD_1DUNGEON)
    {
        StopBuffer(SOUND_DUNGEON01, true);
    }
    if (gMapManager.WorldActive != WD_3NORIA)
    {
        StopBuffer(SOUND_FOREST01, true);
    }
    if (gMapManager.WorldActive != WD_4LOSTTOWER)
    {
        StopBuffer(SOUND_TOWER01, true);
    }
    if (gMapManager.WorldActive != WD_7ATLANSE)
    {
        StopBuffer(SOUND_WATER01, true);
    }
    if (gMapManager.WorldActive != WD_8TARKAN)
    {
        StopBuffer(SOUND_DESERT01, true);
    }
    if (gMapManager.WorldActive != WD_10HEAVEN)
    {
        StopBuffer(SOUND_HEAVEN01, true);
    }
    if (gMapManager.WorldActive != WD_51HOME_6TH_CHAR)
    {
        StopBuffer(SOUND_ELBELAND_VILLAGEPROTECTION01, true);
        StopBuffer(SOUND_ELBELAND_WATERFALLSMALL01, true);
        StopBuffer(SOUND_ELBELAND_WATERWAY01, true);
        StopBuffer(SOUND_ELBELAND_ENTERDEVIAS01, true);
        StopBuffer(SOUND_ELBELAND_WATERSMALL01, true);
        StopBuffer(SOUND_ELBELAND_RAVINE01, true);
        StopBuffer(SOUND_ELBELAND_ENTERATLANCE01, true);
    }
#ifdef ASG_ADD_MAP_KARUTAN
    if (!IsKarutanMap())
        StopBuffer(SOUND_KARUTAN_DESERT_ENV, true);
    if (gMapManager.WorldActive != WD_80KARUTAN1)
        StopBuffer(SOUND_KARUTAN_INSECT_ENV, true);
    if (gMapManager.WorldActive != WD_81KARUTAN2)
        StopBuffer(SOUND_KARUTAN_KARDAMAHAL_ENV, true);
#endif	// ASG_ADD_MAP_KARUTAN
}

/**
 * @brief Manages background music playback for the current world/map.
 *
 * Plays and stops background music tracks based on world and player location.
 */
static void ManageBackgroundMusic()
{
    if (gMapManager.WorldActive == WD_0LORENCIA)
    {
        if (Hero->SafeZone)
        {
            if (HeroTile == 4)
                PlayMp3(MUSIC_PUB);
            else
                PlayMp3(MUSIC_MAIN_THEME);
        }
    }
    else
    {
        StopMp3(MUSIC_PUB);
        StopMp3(MUSIC_MAIN_THEME);
    }

    if (gMapManager.WorldActive == WD_2DEVIAS)
    {
        if (Hero->SafeZone)
        {
            if ((Hero->PositionX) >= 205 && (Hero->PositionX) <= 214 &&
                (Hero->PositionY) >= 13 && (Hero->PositionY) <= 31)
            {
                PlayMp3(MUSIC_CHURCH);
            }
            else
            {
                PlayMp3(MUSIC_DEVIAS);
            }
        }
    }
    else
    {
        StopMp3(MUSIC_CHURCH);
        StopMp3(MUSIC_DEVIAS);
    }

    if (gMapManager.WorldActive == WD_3NORIA)
    {
        if (Hero->SafeZone)
            PlayMp3(MUSIC_NORIA);
    }
    else
    {
        StopMp3(MUSIC_NORIA);
    }

    if (gMapManager.WorldActive == WD_1DUNGEON || gMapManager.WorldActive == WD_5UNKNOWN)
    {
        PlayMp3(MUSIC_DUNGEON);
    }
    else
    {
        StopMp3(MUSIC_DUNGEON);
    }

    if (gMapManager.WorldActive == WD_7ATLANSE) {
        PlayMp3(MUSIC_ATLANS);
    }
    else {
        StopMp3(MUSIC_ATLANS);
    }

    if (gMapManager.WorldActive == WD_10HEAVEN) {
        PlayMp3(MUSIC_ICARUS);
    }
    else {
        StopMp3(MUSIC_ICARUS);
    }

    if (gMapManager.WorldActive == WD_8TARKAN) {
        PlayMp3(MUSIC_TARKAN);
    }
    else {
        StopMp3(MUSIC_TARKAN);
    }

    if (gMapManager.WorldActive == WD_4LOSTTOWER) {
        PlayMp3(MUSIC_LOSTTOWER_A);
    }
    else {
        StopMp3(MUSIC_LOSTTOWER_A);
    }

    if (gMapManager.InHellas(gMapManager.WorldActive)) {
        PlayMp3(MUSIC_KALIMA);
    }
    else {
        StopMp3(MUSIC_KALIMA);
    }

    if (gMapManager.WorldActive == WD_31HUNTING_GROUND) {
        PlayMp3(MUSIC_BC_HUNTINGGROUND);
    }
    else {
        StopMp3(MUSIC_BC_HUNTINGGROUND);
    }

    if (gMapManager.WorldActive == WD_33AIDA) {
        PlayMp3(MUSIC_BC_ADIA);
    }
    else {
        StopMp3(MUSIC_BC_ADIA);
    }

    M34CryWolf1st::ChangeBackGroundMusic(gMapManager.WorldActive);
    M39Kanturu3rd::ChangeBackGroundMusic(gMapManager.WorldActive);

    if (gMapManager.WorldActive == WD_37KANTURU_1ST)
        PlayMp3(MUSIC_KANTURU_1ST);
    else
        StopMp3(MUSIC_KANTURU_1ST);

    M38Kanturu2nd::PlayBGM();
    SEASON3A::CGM3rdChangeUp::Instance().PlayBGM();

    if (gMapManager.IsCursedTemple())
    {
        g_CursedTemple->PlayBGM();
    }

    if (gMapManager.WorldActive == WD_51HOME_6TH_CHAR) {
        PlayMp3(MUSIC_ELBELAND);
    }
    else {
        StopMp3(MUSIC_ELBELAND);
    }

    if (gMapManager.WorldActive == WD_56MAP_SWAMP_OF_QUIET) {
        PlayMp3(MUSIC_SWAMP_OF_QUIET);
    }
    else {
        StopMp3(MUSIC_SWAMP_OF_QUIET);
    }

    g_Raklion.PlayBGM();
    g_SantaTown.PlayBGM();
    g_PKField.PlayBGM();
    g_DoppelGanger1.PlayBGM();
    g_EmpireGuardian1.PlayBGM();
    g_EmpireGuardian2.PlayBGM();
    g_EmpireGuardian3.PlayBGM();
    g_EmpireGuardian4.PlayBGM();
    g_UnitedMarketPlace.PlayBGM();
#ifdef ASG_ADD_MAP_KARUTAN
    g_Karutan1.PlayBGM();
#endif	// ASG_ADD_MAP_KARUTAN
}

/**
 * @brief Manages all audio (ambient sounds and music) for the main game scene.
 *
 * Orchestrates three audio subsystems:
 * - World-specific ambient sound effects
 * - Stopping inactive ambient sounds
 * - Background music management
 *
 * @note Only active when SceneFlag == MAIN_SCENE
 */
static void ManageMainSceneAudio()
{
    if (SceneFlag != MAIN_SCENE)
        return;

    PlayWorldAmbientSounds();
    StopInactiveAmbientSounds();
    ManageBackgroundMusic();
}

/**
 * @brief Main scene rendering and update function.
 *
 * This is the primary entry point for rendering all game scenes (login, character, main game).
 * Orchestrates:
 * - Input/UI updates for login and character scenes
 * - Water animation updates
 * - Core system updates (physics, bitmaps, audio positioning)
 * - Scene-specific rendering
 * - Audio management for the main game scene
 * - Debug information rendering
 * - Server connection monitoring
 *
 * @param hDC Device context for rendering
 */
void MainScene(HDC hDC)
{
    if (SceneFlag == LOG_IN_SCENE || SceneFlag == CHARACTER_SCENE)
    {
        UpdateLoginAndCharacterScenes();
    }

    UpdateWaterAnimation();

    if (Destroy)
    {
        return;
    }

    UpdateCoreSystems();
    SetWorldClearColor();

    bool Success = false;

    try
    {
        Success = RenderCurrentScene(hDC);
        RenderDebugInfo();

        if (Success)
        {
            SwapBuffers(hDC);
        }

        CheckServerConnection();
        ManageMainSceneAudio();
    }
    catch (const std::exception& e)
    {
        // Log exception in MainScene
        char errorMsg[256];
        sprintf_s(errorMsg, sizeof(errorMsg), "Exception in MainScene: %s", e.what());
        OutputDebugStringA(errorMsg);
    }
}

void RenderScene(HDC hDC)
{
    CalcFPS();
    UpdateSceneState();

    g_frameTiming.MarkFrameRendered();

    try
    {
        g_Luminosity = sinf(WorldTime * 0.004f) * 0.15f + 0.6f;
        switch (SceneFlag)
        {
        case WEBZEN_SCENE:
            WebzenScene(hDC);
            break;
        case LOADING_SCENE:
            LoadingScene(hDC);
            break;
        case LOG_IN_SCENE:
        case CHARACTER_SCENE:
        case MAIN_SCENE:
            MainScene(hDC);
            break;
        }

        if (g_iNoMouseTime > 31)
        {
            KillGLWindow();
        }
    }
    catch (const std::exception& e)
    {
        // Log exception in RenderScene
        char errorMsg[256];
        sprintf_s(errorMsg, sizeof(errorMsg), "Exception in RenderScene: %s", e.what());
        OutputDebugStringA(errorMsg);
    }
}
