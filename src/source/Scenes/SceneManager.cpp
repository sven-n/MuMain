///////////////////////////////////////////////////////////////////////////////
// SceneManager.cpp - Scene management and rendering orchestration
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Core/Input/KeyState.h"
#include "Core/Input/SyntheticInput.h"
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <iterator>
#include "SceneManager.h"
#include "Core/Utilities/FrameProfiler.h"
#include "UI/Diagnostics/DiagnosticsOverlay.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Core/Utilities/PlatformInfo.h"
#include "Render/Text/CUIRenderText.h"

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
#include "LoadingScene.h"
#include "LoginSceneOverlay.h"
#include "ScreenshotCaptureState.h"
#include "Audio/DSPlaySound.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Renderer/RenderUtils.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Engine/Physics/PhysicsManager.h"
#include "Core/Time/Timer.h"
#include "Core/Input/Input.h"
#include "UI/Core/SceneUICoordinator.h"
#include "Network/Server/WSclient.h"
#include "Network/Reconnect/ReconnectManager.h"
#include "UI/Dialogs/ReconnectDialog.h"
#include "GameLogic/Events/w_CursedTemple.h"
#include "Network/Server/ServerListManager.h"
#include "UI/Core/WindowSystem.h"
#include "Engine/Object/ZzzInterface.h"
#include "UI/HUD/Notices.h"
#include "I18N/All.h"
#include "Engine/AI/ZzzAI.h"
#include "App/Platform/Windows/Winmain.h"
#include "Camera/CameraManager.h"
#include "Camera/CameraMode.h"
#include "Camera/CameraProjection.h"
#include "Scenes/SceneNames.h"

#ifdef _EDITOR
#include "../MuEditor/Core/MuEditorCore.h"
#include "imgui.h"
#endif

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
extern size_t g_LastActiveCharacterCount;
extern bool g_LastAnimationWasParallel;

namespace
{
    void ClearMousePressState()
    {
        MouseLButtonPush = false;
        MouseRButtonPush = false;
        MouseMButtonPush = false;
        Core::Input::ClearLeftMouseButtonPressEdge();
    }
}

static bool g_bShowDebugInfo =
#ifdef _DEBUG
    true;
#else
    false;
#endif

static bool g_bShowFpsCounter = false;

void SetShowDebugInfo(bool enabled)
{
    // The statistics only sample while the overlay draws: restart them, or the first frame
    // would count all the time the overlay was off as one slow frame.
    if (enabled && !g_bShowDebugInfo)
        ResetFrameStats();
    g_bShowDebugInfo = enabled;
    if (enabled) g_bShowFpsCounter = false;
}

bool GetShowDebugInfo()
{
    return g_bShowDebugInfo;
}

void SetShowFpsCounter(bool enabled)
{
    g_bShowFpsCounter = enabled;
    if (enabled) g_bShowDebugInfo = false;
}

bool GetShowFpsCounter()
{
    return g_bShowFpsCounter;
}

void SetShowGLStats(bool enabled)
{
    FrameProfiler::g_CountersEnabled = enabled;
    mu::GetRenderer().SetStatsEnabled(enabled);
}

//=============================================================================
// Frame Statistics Tracker
//=============================================================================

static constexpr int FRAME_HISTORY_SIZE = 300;      // ~5 seconds at 60fps
static constexpr float MIN_FRAME_TIME_MS = 0.5f;    // clamp to 2000fps max
static constexpr double STATS_UPDATE_INTERVAL = 500.0; // ms between percentile recalculations
static constexpr int MIN_FRAMES_FOR_STATS = 10;

static float s_frameTimesMs[FRAME_HISTORY_SIZE] = {};
static int s_frameIndex = 0;
static int s_frameCount = 0;
static double s_lastFrameTime = 0.0;
static double s_highestFps = 0.0;

// Percentile stats (updated periodically)
static float s_avgFps = 0.0f;
static float s_onePercentLow = 0.0f;
static float s_slowestFrameFps = 0.0f;
static double s_lastStatsUpdate = 0.0;

void ResetFrameStats()
{
    memset(s_frameTimesMs, 0, sizeof(s_frameTimesMs));
    s_frameIndex = 0;
    s_frameCount = 0;
    s_lastFrameTime = 0.0;
    s_highestFps = 0.0;
    s_avgFps = 0.0f;
    s_onePercentLow = 0.0f;
    s_slowestFrameFps = 0.0f;
    s_lastStatsUpdate = 0.0;
}

static void UpdateFrameStats()
{
    double now = WorldTime;
    if (s_lastFrameTime > 0.0)
    {
        double dt = now - s_lastFrameTime;
        if (dt < MIN_FRAME_TIME_MS) dt = MIN_FRAME_TIME_MS;
        s_frameTimesMs[s_frameIndex] = static_cast<float>(dt);
        s_frameIndex = (s_frameIndex + 1) % FRAME_HISTORY_SIZE;
        if (s_frameCount < FRAME_HISTORY_SIZE) s_frameCount++;

        double instantaneousFps = 1000.0 / dt;
        if (instantaneousFps > s_highestFps) s_highestFps = instantaneousFps;
    }
    s_lastFrameTime = now;

    // Update percentile stats periodically
    if (now - s_lastStatsUpdate > STATS_UPDATE_INTERVAL && s_frameCount > MIN_FRAMES_FOR_STATS)
    {
        s_lastStatsUpdate = now;

        // Copy and sort frame times (descending = slowest first)
        static float sorted[FRAME_HISTORY_SIZE];
        memcpy(sorted, s_frameTimesMs, sizeof(float) * s_frameCount);
        std::sort(sorted, sorted + s_frameCount, std::greater<float>());

        // Average
        float sum = std::accumulate(sorted, sorted + s_frameCount, 0.0f);
        float avgMs = sum / s_frameCount;
        s_avgFps = (avgMs > 0.0f) ? 1000.0f / avgMs : 0.0f;

        // 1% low: average of the slowest 1% of frames
        int onePercCount = std::max(1, s_frameCount / 100);
        float onePercSum = std::accumulate(sorted, sorted + onePercCount, 0.0f);
        float onePercAvgMs = onePercSum / onePercCount;
        s_onePercentLow = (onePercAvgMs > 0.0f) ? 1000.0f / onePercAvgMs : 0.0f;

        // Slowest frame: the single slowest frame in the window
        s_slowestFrameFps = (sorted[0] > 0.0f) ? 1000.0f / sorted[0] : 0.0f;
    }
}

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

/**
 * @brief Generates a timestamped filename and message for screenshot capture.
 * @param outFileName Buffer to receive the filename
 * @param outMessage Buffer to receive the log message
 */
static void GenerateScreenshotFilename(wchar_t* outFileName, wchar_t* outMessage)
{
    SYSTEMTIME st;
    GetLocalTime(&st);
    mu_swprintf(outFileName, L"Screen(%02d_%02d-%02d_%02d)-%04d.jpg",
        st.wMonth, st.wDay, st.wHour, st.wMinute, GrabScreen);
    mu_swprintf(outMessage, I18N::Game::SScreenshotSaved, outFileName);

    wchar_t lpszTemp[64];
    mu_swprintf(lpszTemp, L" [%ls / %ls]", g_ServerListManager->GetSelectServerName(), Hero->ID);
    wcscat(outMessage, lpszTemp);
}

/**
 * @brief Names a scripted capture that was given no path of its own.
 *
 * Seconds and a counter of this process' own keep two captures of the same
 * minute apart; the player's rolling counter and the shared `GrabFileName`
 * buffer stay untouched, so a pending Print Screen still reports its own name.
 */
static std::wstring GenerateScriptedScreenshotFilename()
{
    static int scriptedCaptureCount = 0;

    SYSTEMTIME st;
    GetLocalTime(&st);

    wchar_t fileName[256];
    mu_swprintf(fileName, L"Screen(%02d_%02d-%02d_%02d_%02d)-%04d.jpg", st.wMonth, st.wDay, st.wHour, st.wMinute,
                st.wSecond, scriptedCaptureCount);
    scriptedCaptureCount = (scriptedCaptureCount + 1) % 10000;

    return fileName;
}

static ScreenshotCaptureState g_screenshotCapture;

// Set while a scripted capture (control socket) is pending; a human's Print
// Screen leaves it empty and behaves exactly as before.
static ScreenshotCompletion g_screenshotCompletion;
// JPEG quality of the pending capture.
static int g_screenshotQuality = BestScreenshotQuality;

// The readback is only ready on a later frame than the one that asked for it,
// and a request may arrive either before or after this frame's consume pass
// (Print Screen comes from the scene update, a scripted capture from the
// control socket's poll). A frame the renderer skips (no swapchain image: a
// minimized window, or too many frames in flight) delivers nothing and leaves
// the request for the next one, so the wait allows for a few skipped frames:
// about a second at 60 frames per second.
constexpr int MaxScreenshotConsumeAttempts = 60;
static int g_screenshotConsumeAttempts = 0;

static bool PrepareJpegPixels(mu::FramePixels& pixels)
{
    const std::size_t rowBytes = static_cast<std::size_t>(pixels.width) * 3;
    const std::size_t expectedBytes = rowBytes * pixels.height;
    if (rowBytes == 0 || pixels.rgb.size() != expectedBytes)
    {
        return false;
    }

    for (std::uint32_t row = 0; row < pixels.height / 2; ++row)
    {
        auto top = pixels.rgb.begin() + static_cast<std::size_t>(row) * rowBytes;
        auto bottom = pixels.rgb.begin() + static_cast<std::size_t>(pixels.height - row - 1) * rowBytes;
        std::swap_ranges(top, top + rowBytes, bottom);
    }
    return true;
}

// Hands the outcome to a scripted caller, if one is waiting, and forgets it.
// A failed capture is reported too, so no caller waits for its deadline.
static void ReportScreenshotOutcome(bool saved, const std::wstring& path, int width, int height)
{
    if (!g_screenshotCompletion)
    {
        return;
    }

    ScreenshotCompletion completion;
    completion.swap(g_screenshotCompletion);
    completion(ScreenshotOutcome{saved, path, width, height});
}

static void ConsumeScreenshot()
{
    if (!g_screenshotCapture.HasPending())
    {
        return;
    }

    const std::wstring fileName = g_screenshotCapture.FileName();

    mu::FramePixels pixels;
    if (!mu::GetRenderer().ConsumeFramePixels(pixels))
    {
        ++g_screenshotConsumeAttempts;
        if (g_screenshotConsumeAttempts < MaxScreenshotConsumeAttempts)
        {
            return;
        }

        // The request may still be pending in the renderer; drop it, or it
        // would block the next capture.
        mu::GetRenderer().CancelFramePixels();
        g_screenshotCapture.Clear();
        ReportScreenshotOutcome(false, fileName, 0, 0);
        return;
    }

    if (!PrepareJpegPixels(pixels))
    {
        g_screenshotCapture.Clear();
        ReportScreenshotOutcome(false, fileName, 0, 0);
        return;
    }

    const int width = static_cast<int>(pixels.width);
    const int height = static_cast<int>(pixels.height);
    std::wstring writtenName = fileName;
    const bool saved = WriteJpeg(writtenName.data(), width, height, pixels.rgb.data(), g_screenshotQuality);

    // A scripted capture has no message: the system log belongs to the player's
    // own Print Screen, and so does the rolling screenshot counter.
    const bool isPlayerCapture = !g_screenshotCapture.Message().empty();
    if (saved && isPlayerCapture)
    {
        g_pSystemLogBox->AddText(g_screenshotCapture.Message().c_str(), mu::ui::window::TYPE_SYSTEM_MESSAGE);
    }

    if (isPlayerCapture)
    {
        GrabScreen++;
        GrabScreen %= 10000;
    }

    g_screenshotCapture.Clear();
    ReportScreenshotOutcome(saved, fileName, width, height);
}

// Starts a capture of the next rendered frame. `message` is the system-log line
// the player sees; a scripted capture passes none.
static bool BeginScreenshotCapture(const std::wstring& fileName, const std::wstring& message)
{
    if (!g_screenshotCapture.Begin(fileName, message))
    {
        return false;
    }

    g_screenshotConsumeAttempts = 0;

    // A previous capture may have given up waiting for its pixels; they can
    // still arrive, and this capture must not report the earlier frame.
    mu::GetRenderer().CancelFramePixels();

    if (!mu::GetRenderer().RequestFramePixels())
    {
        g_screenshotCapture.Clear();
        return false;
    }

    return true;
}

static void RequestScreenshot()
{
    wchar_t screenshotText[256];
    GenerateScreenshotFilename(GrabFileName, screenshotText);

    if (BeginScreenshotCapture(GrabFileName, screenshotText))
    {
        g_screenshotQuality = BestScreenshotQuality;
    }
}

void CancelScriptedScreenshot()
{
    if (!g_screenshotCapture.HasPending())
    {
        return;
    }

    g_screenshotCapture.Clear();
    g_screenshotCompletion = nullptr;
    mu::GetRenderer().CancelFramePixels();
}

bool RequestScriptedScreenshot(const std::wstring& path, int quality, ScreenshotCompletion onComplete)
{
    if (g_screenshotCapture.HasPending())
    {
        return false;
    }

    std::wstring fileName = path;
    if (fileName.empty())
    {
        fileName = GenerateScriptedScreenshotFilename();
    }

    if (!BeginScreenshotCapture(fileName, L""))
    {
        return false;
    }

    g_screenshotQuality = std::clamp(quality, 1, BestScreenshotQuality);
    g_screenshotCompletion = std::move(onComplete);
    return true;
}

/**
 * @brief Handles screenshot capture toggle and execution.
 */
static void HandleScreenshotCapture()
{
    ConsumeScreenshot();

    if (PressKey(VK_SNAPSHOT))
    {
        GrabEnable = !GrabEnable;
    }

    if (!GrabEnable)
    {
        return;
    }

    RequestScreenshot();
    GrabEnable = false;
}

/**
 * @brief Updates the active scene based on current scene flag.
 */
static void UpdateActiveScene()
{
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
}

/**
 * @brief Updates scene state, handles input, and manages screenshot capture.
 */
void UpdateSceneState()
{
    // Scripted input advances here, once per rendered frame, so the scan
    // below sees an injected key or click exactly as it sees a device.
    Core::Input::Synthetic::BeginFrame();
    g_pNewKeyInput->ScanAsyncKeyState();
    g_dwMouseUseUIID = 0;

    UpdateActiveScene();
    UI::Notices::Move();
    Scenes::LoginOverlay::HideOutsideLoginScene();
    HandleScreenshotCapture();
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
    CSceneUICoordinator::Instance().Update(dDeltaTick);
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
    auto time_since_last_render = g_frameTiming.GetCurrentTickCount() - g_frameTiming.GetLastWaterChange();
    while (time_since_last_render > timePerFrame)
    {
        WaterTextureNumber++;
        WaterTextureNumber %= NumberOfWaterTextures;
        time_since_last_render -= timePerFrame;
        g_frameTiming.SetLastWaterChange(g_frameTiming.GetCurrentTickCount());
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
 * @brief Sets both the OpenGL clear color and the global fog color to the same RGB.
 *
 * Every world uses the fog color as its clear color, so this keeps them in sync
 * and avoids duplicating the two assignments at every call site.
 */
static void SetClearAndFogColor(float r, float g, float b)
{
    extern GLfloat FogColor[4];
    mu::GetRenderer().SetClearColor(r, g, b, 1.f);
    FogColor[0] = r;
    FogColor[1] = g;
    FogColor[2] = b;
    FogColor[3] = 1.f;
}

/**
 * @brief Sets the OpenGL clear color based on the current world/map.
 *
 * Different maps have different background colors for visual atmosphere.
 */
static void SetWorldClearColor()
{
    // Convenience: build a 0-255 color at call site; dividing by 256 matches legacy values.
    constexpr float BYTE_TO_FLOAT = 1.f / 256.f;
    auto rgb8 = [](int r, int g, int b) {
        SetClearAndFogColor(r * BYTE_TO_FLOAT, g * BYTE_TO_FLOAT, b * BYTE_TO_FLOAT);
    };

    const int world = gMapManager.WorldActive;

    if (world == WD_0LORENCIA)
        rgb8(10, 20, 14);                              // Dark green
    else if (world == WD_2DEVIAS)
        rgb8(0, 0, 10);                                // Dark navy abyss
    else if (world == WD_10HEAVEN)
        rgb8(3, 25, 44);                               // Blue
    else if (world == WD_73NEW_LOGIN_SCENE || world == WD_74NEW_CHARACTER_SCENE)
        SetClearAndFogColor(0.f, 0.f, 0.f);            // Black
    else if (gMapManager.InHellas(world))
        rgb8(30, 40, 40);                              // Teal
    else if (gMapManager.InChaosCastle())
        SetClearAndFogColor(0.f, 0.f, 0.f);            // Black
    else if (gMapManager.InBattleCastle() && battleCastle::InBattleCastle2(Hero->Object.Position))
        SetClearAndFogColor(0.f, 0.f, 0.f);            // Black
    else if (world >= WD_45CURSEDTEMPLE_LV1 && world <= WD_45CURSEDTEMPLE_LV6)
        rgb8(9, 8, 33);                                // Dark purple
    else if (world == WD_51HOME_6TH_CHAR)
        rgb8(178, 178, 178);                           // Gray
    else if (world == WD_65DOPPLEGANGER1)
        rgb8(148, 179, 223);                           // Light blue
    else
        SetClearAndFogColor(0.f, 0.f, 0.f);            // Black (default)

    mu::GetRenderer().ClearScreen();
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

// Loading screens never reach the overlay, so the first frame of a new scene would otherwise
// count the whole load as one slow frame; the previous scene's frames say nothing about this one.
static void RestartFrameStatsOnSceneChange()
{
    static EGameScene s_statsScene = SceneFlag;
    if (SceneFlag == s_statsScene)
        return;

    ResetFrameStats();
    s_statsScene = SceneFlag;
}

static void UpdateDiagnostics()
{
    if (g_bShowDebugInfo)
    {
        RestartFrameStatsOnSceneChange();
        UpdateFrameStats();
    }
    UI::Diagnostics::FrameSummary summary;
    summary.details = g_bShowDebugInfo;
    summary.counters = FrameProfiler::g_CountersEnabled;
    summary.fpsOnly = g_bShowFpsCounter;
    summary.fps = FPS_AVG;
    summary.averageFps = s_avgFps;
    summary.lowFps = s_onePercentLow;
    constexpr double MillisecondsPerSecond = 1000.0;
    summary.frameMs = s_avgFps > 0 ? MillisecondsPerSecond / s_avgFps : 0;
    summary.cpu = CPU_AVG;
    summary.vsync = IsVSyncEnabled();
    summary.history = std::span<const float>(s_frameTimesMs, static_cast<size_t>(s_frameCount));
    summary.oldestSample = s_frameCount < FRAME_HISTORY_SIZE ? 0 : s_frameIndex;
    UI::Diagnostics::Update(summary);
}

/**
 * @brief Checks and handles server connection loss.
 */
static void CheckServerConnection()
{
    if (SocketClient != nullptr && SocketClient->IsConnected())
    {
        return;
    }

    // A reconnect already in progress manages its own connection attempts.
    if (ReconnectManager::Instance().IsActive())
    {
        return;
    }

    // Auto-reconnect only makes sense in-game, where the server restores the
    // character's saved position. Other scenes keep the original behaviour.
    if (SceneFlag == MAIN_SCENE && ReconnectManager::Instance().HasSession())
    {
        g_ErrorReport.Write(L"> Connection lost in game - starting auto-reconnect. ");
        g_ErrorReport.WriteCurrentTime();
        g_ConsoleDebug->Write(MCD_NORMAL, L"Connection lost in game - starting auto-reconnect");
        // Grab the clean game frame now (front buffer, dialog not yet drawn) so
        // the brief re-login phase shows it frozen instead of a black screen.
        UI::Reconnect::CaptureBackground();
        ReconnectManager::Instance().RequestBegin();
        return;
    }

    static BOOL s_bClosed = FALSE;
    if (!s_bClosed)
    {
        s_bClosed = TRUE;
        g_ErrorReport.Write(L"> Connection closed. ");
        g_ErrorReport.WriteCurrentTime();
        g_ConsoleDebug->Write(MCD_NORMAL, L"Connection closed");
        CSceneUICoordinator::Instance().PopUpMsgWin(MESSAGE_SERVER_LOST);
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

static void LogFrameTiming()
{
    static bool enabled = std::getenv("MU_RENDER_TIMING") != nullptr;
    static unsigned frameCounter = 0;
    constexpr unsigned kLogInterval = 60;
    if (!enabled || ++frameCounter % kLogInterval != 0)
        return;

    using Counter = FrameProfiler::Counter;
    using Pass = FrameProfiler::Pass;
    const mu::RendererStats stats = mu::GetRenderer().GetFrameStats();
    const auto logger = mu::log::Get("render");
    logger->info(
        "[RENDER diag] requested={} submitted={} pipeline_binds={} sampler_binds={} vertex_uniform_pushes={} "
        "fragment_uniform_pushes={} merged_2d={} glyph_uploads={} skin_gpu={} skin_cpu_ineligible={} skin_failed={}",
        stats.requestedDrawCalls, stats.submittedDrawCalls, stats.pipelineBinds, stats.samplerBinds,
        stats.vertexUniformPushes, stats.fragmentUniformPushes, stats.merged2DDrawCalls,
        FrameProfiler::CompletedCounter(Counter::GlyphUploads),
        FrameProfiler::CompletedCounter(Counter::GpuSkinningSubmissions),
        FrameProfiler::CompletedCounter(Counter::CpuSkinningIneligible),
        FrameProfiler::CompletedCounter(Counter::GpuSkinningFailures));
    logger->info(
        "[FRAME timing] terrain={:.2f}ms objects={:.2f}ms characters={:.2f}ms items={:.2f}ms "
        "effects={:.2f}ms other={:.2f}ms sprites={:.2f}ms particles={:.2f}ms joints={:.2f}ms "
        "skin_gpu={} skin_cpu_ineligible={} skin_failed={}",
        FrameProfiler::CompletedMs(Pass::Terrain), FrameProfiler::CompletedMs(Pass::Objects),
        FrameProfiler::CompletedMs(Pass::Characters), FrameProfiler::CompletedMs(Pass::Items),
        FrameProfiler::CompletedMs(Pass::Effects), FrameProfiler::CompletedMs(Pass::Other),
        FrameProfiler::CompletedMs(Pass::Sprites), FrameProfiler::CompletedMs(Pass::Particles),
        FrameProfiler::CompletedMs(Pass::Joints),
        FrameProfiler::CompletedCounter(Counter::GpuSkinningSubmissions),
        FrameProfiler::CompletedCounter(Counter::CpuSkinningIneligible),
        FrameProfiler::CompletedCounter(Counter::GpuSkinningFailures));
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

        LogFrameTiming();
        {
            FRAME_PROFILE(Overlay);
            UpdateDiagnostics();
            UI::Reconnect::RenderDialog();
        }

        if (Success)
        {
#ifdef _EDITOR
            // Always render ImGui (shows "Open Editor" button when closed, or full UI when open)
            g_MuEditorCore.Render();

            // Render game cursor on top of ImGui if not hovering UI
            extern bool g_bRenderGameCursor;
            if (g_bRenderGameCursor)
            {
                BeginBitmap();
                RenderCursor();
                EndBitmap();
            }
#endif
        }

        CheckServerConnection();
        ManageMainSceneAudio();
    }
    catch (const std::exception& e)
    {
        wchar_t errorMessage[256] = {};
        mbstowcs(errorMessage, e.what(), 255);
        g_ErrorReport.Write(L"Exception in MainScene: %ls\r\n", errorMessage);
    }
}

void RenderScene(HDC hDC)
{
    CalcFPS();
    UpdateSceneState();

    // Drive auto-reconnect after the scene loops have advanced this frame. It
    // runs across all scenes because reconnect passes through the login,
    // character and loading scenes on its way back into the game.
    ReconnectManager::Instance().Update();

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
            Destroy = true;
        }
    }
    catch (const std::exception& e)
    {
        wchar_t errorMessage[256] = {};
        mbstowcs(errorMessage, e.what(), 255);
        g_ErrorReport.Write(L"Exception in RenderScene: %ls\r\n", errorMessage);
    }

    // SDL may deliver button-down and button-up in one event batch. Keep these
    // one-shot flags alive until scene logic has consumed the rendered frame.
    ClearMousePressState();
}
