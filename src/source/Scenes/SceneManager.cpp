///////////////////////////////////////////////////////////////////////////////
// SceneManager.cpp - Scene management and rendering orchestration
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Core/Input/KeyState.h"
#include <vector>
#include <algorithm>
#include <numeric>
#include "SceneManager.h"
#include "Core/Utilities/FrameProfiler.h"
#ifdef __ANDROID__
#include <android/log.h>
#include <chrono>
#endif
#include "Core/Utilities/PlatformInfo.h"
#include "Render/Core/ImmediateRenderer.h"
#include "Render/Shaders/PassthroughShader.h"
#include "Render/Core/RenderConfig.h"
#include "Render/RHI/RHI.h"

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
#include "Audio/DSPlaySound.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Engine/Physics/PhysicsManager.h"
#include "Core/Time/Timer.h"
#include "Core/Input/Input.h"
#include "UI/Legacy/UIMng.h"
#include "Network/Server/WSclient.h"
#include "Network/Reconnect/ReconnectManager.h"
#include "UI/NewUI/Dialogs/ReconnectDialog.h"
#include "GameLogic/Events/w_CursedTemple.h"
#include "Network/Server/ServerListManager.h"
#include "UI/NewUI/NewUISystem.h"
#include "Engine/Object/ZzzInterface.h"
#include "UI/NewUI/HUD/Notices.h"
#include "I18N/All.h"
#include "Engine/AI/ZzzAI.h"
#include "App/Platform/Windows/Winmain.h"
#include "Camera/CameraManager.h"
#include "Camera/CameraMode.h"

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

static bool g_bShowDebugInfo =
#ifdef _DEBUG
    true;
#else
    false;
#endif

static bool g_bShowFpsCounter = false;

void SetShowDebugInfo(bool enabled)
{
    g_bShowDebugInfo = enabled;
    if (enabled) g_bShowFpsCounter = false;
}

void SetShowFpsCounter(bool enabled)
{
    g_bShowFpsCounter = enabled;
    if (enabled) g_bShowDebugInfo = false;
}

// GLP-01: independent of the two flags above -- $glstats can be shown alongside $details or
// $fpscounter, not just standalone. Also the single switch that gates FrameProfiler's counter/
// GPU-timer increments themselves (FrameProfiler::g_CountersEnabled), so turning the overlay
// off also stops paying for the instrumentation.
void SetShowGLStats(bool enabled)
{
    FrameProfiler::g_CountersEnabled = enabled;
}

//=============================================================================
// Frame Statistics Tracker
//=============================================================================

static constexpr int FRAME_HISTORY_SIZE = 300;      // ~5 seconds at 60fps
static constexpr float MIN_FRAME_TIME_MS = 0.5f;    // clamp to 2000fps max
static constexpr double STATS_UPDATE_INTERVAL = 500.0; // ms between percentile recalculations
static constexpr int MIN_FRAMES_FOR_STATS = 10;
static constexpr float GRAPH_MAX_MS = 33.3f;        // graph Y-axis scale (30fps)
static constexpr float THRESHOLD_60FPS_MS = 16.67f;  // 60 FPS threshold
static constexpr float THRESHOLD_40FPS_MS = 25.0f;   // 40 FPS threshold
static constexpr float DEBUG_TEXT_X = 10.0f;          // debug overlay X position
static constexpr int DEBUG_TEXT_Y_START = 26;         // debug overlay Y start
static constexpr int DEBUG_TEXT_LINE_HEIGHT = 10;     // line spacing
static constexpr float DEBUG_GRAPH_WIDTH = 200.0f;    // frame graph width
static constexpr float DEBUG_GRAPH_HEIGHT = 40.0f;    // frame graph height
static constexpr float DEBUG_GRAPH_Y_OFFSET = 2.0f;   // gap between text and graph

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
 * @brief Captures the current frame buffer and saves it as a JPEG screenshot.
 */
static void CaptureScreenshot()
{
    std::vector<unsigned char> Buffer(WindowWidth * WindowHeight * 3);
    // DXP-12: RHI::ReadColorFramebuffer's contract is top-down (unlike raw glReadPixels'
    // bottom-up) -- WriteJpeg's bottomUp=false tells turbojpeg to skip its own flip.
    RHI::ReadColorFramebuffer(0, 0, static_cast<int>(WindowWidth), static_cast<int>(WindowHeight), Buffer.data());
    WriteJpeg(GrabFileName, WindowWidth, WindowHeight, Buffer.data(), 100, false);

    GrabScreen++;
    GrabScreen %= 10000;
}

/**
 * @brief Handles screenshot capture toggle and execution.
 */
static void HandleScreenshotCapture()
{
    if (PressKey(VK_SNAPSHOT))
    {
        GrabEnable = !GrabEnable;
    }

    if (!GrabEnable)
    {
        return;
    }

    const bool addTimeStampToCapture = !Core::Input::IsKeyDown(VK_SHIFT);
    wchar_t screenshotText[256];

    GenerateScreenshotFilename(GrabFileName, screenshotText);

    if (addTimeStampToCapture)
    {
        g_pSystemLogBox->AddText(screenshotText, SEASON3B::TYPE_SYSTEM_MESSAGE);
    }

    CaptureScreenshot();

    if (!addTimeStampToCapture)
    {
        g_pSystemLogBox->AddText(screenshotText, SEASON3B::TYPE_SYSTEM_MESSAGE);
    }

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
    g_pNewKeyInput->ScanAsyncKeyState();
    g_dwMouseUseUIID = 0;

    UpdateActiveScene();
    UI::Notices::Move();
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
 * @brief Sets both the OpenGL clear color and the global fog color to the same RGB.
 *
 * Every world uses the fog color as its clear color, so this keeps them in sync
 * and avoids duplicating the two assignments at every call site.
 */
static void SetClearAndFogColor(float r, float g, float b)
{
    extern GLfloat FogColor[4];
    SetClearColor(r, g, b, 1.f);
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
        SetClearAndFogColor(0.75f, 0.85f, 1.0f);       // Light snowy blue
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

    ClearColorAndDepthBuffers();
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
 * @brief Renders a frame time graph using raw OpenGL quads.
 *
 * Draws a bar chart of recent frame times inside BeginBitmap's 2D ortho projection.
 * Coordinates are in virtual 640x480 space, converted to window pixels.
 */
static void RenderFrameGraph(float graphX, float graphY, float graphW, float graphH)
{
    if (s_frameCount < 2)
        return;

    // Convert virtual 640x480 coords to actual window pixels
    float gx = graphX * (float)WindowWidth / (float)REFERENCE_WIDTH;
    float gy = graphY * (float)WindowHeight / (float)REFERENCE_HEIGHT;
    float gw = graphW * (float)WindowWidth / (float)REFERENCE_WIDTH;
    float gh = graphH * (float)WindowHeight / (float)REFERENCE_HEIGHT;

    // Flip Y for OpenGL (origin bottom-left)
    float glBottom = (float)WindowHeight - gy - gh;
    float glTop = (float)WindowHeight - gy;

    DisableTexture2D();
    EnableBlend();
    SetBlendFuncAlpha();

    IR::Begin(GL_QUADS);
    PassthroughShader::Instance().SetUseTexture(false);
    IR::Color4f(0.0f, 0.0f, 0.0f, 0.5f);
    IR::Vertex2f(gx, glBottom);
    IR::Vertex2f(gx + gw, glBottom);
    IR::Vertex2f(gx + gw, glTop);
    IR::Vertex2f(gx, glTop);
    IR::End();

    float target60 = THRESHOLD_60FPS_MS / GRAPH_MAX_MS;
    float lineY = glBottom + target60 * gh;
    IR::Begin(GL_LINES);
    PassthroughShader::Instance().SetUseTexture(false);
    IR::Color4f(0.3f, 0.8f, 0.3f, 0.5f);
    IR::Vertex2f(gx, lineY);
    IR::Vertex2f(gx + gw, lineY);
    IR::End();

    float barW = gw / FRAME_HISTORY_SIZE;
    int oldest = (s_frameCount < FRAME_HISTORY_SIZE) ? 0 : s_frameIndex;

    IR::Begin(GL_QUADS);
    PassthroughShader::Instance().SetUseTexture(false);
    for (int i = 0; i < s_frameCount; i++)
    {
        int idx = (oldest + i) % FRAME_HISTORY_SIZE;
        float ms = s_frameTimesMs[idx];
        float norm = std::min(ms / GRAPH_MAX_MS, 1.0f);
        float barH = norm * gh;

        if (ms < THRESHOLD_60FPS_MS)
            IR::Color4f(0.2f, 0.9f, 0.2f, 0.8f);
        else if (ms < THRESHOLD_40FPS_MS)
            IR::Color4f(0.9f, 0.9f, 0.2f, 0.8f);
        else
            IR::Color4f(0.9f, 0.2f, 0.2f, 0.8f);

        float bx = gx + i * barW;
        IR::Vertex2f(bx, glBottom);
        IR::Vertex2f(bx + barW, glBottom);
        IR::Vertex2f(bx + barW, glBottom + barH);
        IR::Vertex2f(bx, glBottom + barH);
    }
    IR::End();

    EnableTexture2D();
}

/**
 * @brief Renders debug information overlay.
 *
 * Shows FPS stats, percentile lows, mouse position, camera info, and frame time graph.
 */
static void RenderDebugInfo()
{
    if (!g_bShowDebugInfo)
        return;

    UpdateFrameStats();

    BeginBitmap();

    wchar_t szLine[128];
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetBgColor(0, 0, 0, 100);
    g_pRenderText->SetTextColor(255, 255, 255, 200);

    int y = DEBUG_TEXT_Y_START;
    mu_swprintf(szLine, L"FPS: %.1f  Avg: %.1f  Max: %.1f  Vsync: %d  CPU: %.1f%%",
        FPS_AVG, s_avgFps, s_highestFps, IsVSyncEnabled(), CPU_AVG);
    g_pRenderText->RenderText((int)DEBUG_TEXT_X, y, szLine); y += DEBUG_TEXT_LINE_HEIGHT;

    mu_swprintf(szLine, L"1%% Low: %.1f  Slowest: %.1f  Frame: %.2fms",
        s_onePercentLow, s_slowestFrameFps,
        (s_avgFps > 0.0f) ? 1000.0f / s_avgFps : 0.0f);
    g_pRenderText->RenderText((int)DEBUG_TEXT_X, y, szLine); y += DEBUG_TEXT_LINE_HEIGHT;

    mu_swprintf(szLine, L"MousePos: %d %d %d", MouseX, MouseY, MouseLButtonPush);
    g_pRenderText->RenderText((int)DEBUG_TEXT_X, y, szLine); y += DEBUG_TEXT_LINE_HEIGHT;

    mu_swprintf(szLine, L"Camera3D: %.1f %.1f:%.1f:%.1f", g_Camera.FOV, g_Camera.Angle[0], g_Camera.Angle[1], g_Camera.Angle[2]);
    g_pRenderText->RenderText((int)DEBUG_TEXT_X, y, szLine); y += DEBUG_TEXT_LINE_HEIGHT;

    // Compile-time build info: configuration, feature flags, compiler, arch,
    // and the binary's build timestamp. Useful for verifying which build is
    // actually running without having to check executable metadata.
    constexpr const char* kBuildType =
#if defined(_DEBUG) || defined(DEBUG)
        "Debug";
#else
        "Release";
#endif
    constexpr const char* kEditor =
#ifdef _EDITOR
        "Editor";
#else
        "NoEditor";
#endif
    constexpr const char* kCompiler =
#if defined(__MINGW32__) || defined(__MINGW64__)
        "MinGW";
#elif defined(__clang__)
        "Clang";
#elif defined(_MSC_VER)
        "MSVC";
#elif defined(__GNUC__)
        "GCC";
#else
        "Unknown";
#endif
    constexpr const char* kArch =
#if defined(_WIN64) || defined(__x86_64__) || defined(__aarch64__)
        "x64";
#else
        "x86";
#endif
    mu_swprintf(szLine, L"Build: %hs %hs %hs %hs  %hs %hs",
             kBuildType, kEditor, kCompiler, kArch, __DATE__, __TIME__);
    g_pRenderText->RenderText((int)DEBUG_TEXT_X, y, szLine); y += DEBUG_TEXT_LINE_HEIGHT;

    // Runtime OS name and version (compile-time arch above doesn't capture which
    // OS build the binary is actually running on).
    mu_swprintf(szLine, L"OS: %ls", Core::Platform::GetOSVersionString().c_str());
    g_pRenderText->RenderText((int)DEBUG_TEXT_X, y, szLine); y += DEBUG_TEXT_LINE_HEIGHT;

    // Active camera mode (cycled with F9).
    mu_swprintf(szLine, L"Camera: %hs", CameraModeToString(CameraManager::Instance().GetCurrentMode()));
    g_pRenderText->RenderText((int)DEBUG_TEXT_X, y, szLine); y += DEBUG_TEXT_LINE_HEIGHT;

    // Per-pass frame timing (ms) — accumulated by FRAME_PROFILE scopes around the
    // major render passes in MainScene. Reset just below so next frame starts fresh.
    using FP = FrameProfiler::Pass;
    mu_swprintf(szLine, L"Frame ms  T:%5.2f  O:%5.2f  C:%5.2f  I:%5.2f  E:%5.2f  Oth:%5.2f",
             FrameProfiler::AccumulatorMs(FP::Terrain),
             FrameProfiler::AccumulatorMs(FP::Objects),
             FrameProfiler::AccumulatorMs(FP::Characters),
             FrameProfiler::AccumulatorMs(FP::Items),
             FrameProfiler::AccumulatorMs(FP::Effects),
             FrameProfiler::AccumulatorMs(FP::Other)); // 1-frame-lagged: debug-overlay/reconnect-dialog render cost only now (Present split out below, DXP-23)
    g_pRenderText->RenderText((int)DEBUG_TEXT_X, y, szLine); y += DEBUG_TEXT_LINE_HEIGHT;

    // DXP-23: UI = RenderMainSceneUI() self-time (was previously unmeasured, fell outside every
    // FRAME_PROFILE scope) -- this frame's own value, RenderCurrentScene() already ran above.
    // Present = PlatformSwapBuffers() self-time, split out of Other so a large reading
    // unambiguously points at GPU-stall wait rather than HUD render cost -- 1-frame-lagged like
    // Oth above, since the swap itself only happens after this function returns.
    mu_swprintf(szLine, L"UI:%6.2f  Present:%6.2f",
             FrameProfiler::AccumulatorMs(FP::UI),
             FrameProfiler::AccumulatorMs(FP::Present));
    g_pRenderText->RenderText((int)DEBUG_TEXT_X, y, szLine); y += DEBUG_TEXT_LINE_HEIGHT;

    // Move/update-phase cost of particle & effect simulation (UpdateGameEntities(), not the
    // render-side Effects pass above) — added to gauge whether MoveEffects()/MoveParticles()
    // are worth parallelizing on a worker thread pool (see feature-ffp-shader-port task memory).
    mu_swprintf(szLine, L"MoveSim ms  MoveFx:%5.2f  MovePart:%5.2f",
             FrameProfiler::AccumulatorMs(FP::MoveEffects),
             FrameProfiler::AccumulatorMs(FP::MoveParticles));
    g_pRenderText->RenderText((int)DEBUG_TEXT_X, y, szLine); y += DEBUG_TEXT_LINE_HEIGHT;

    // DXP-20 baseline: BMD::Transform() self-time (CPU skinning + per-vertex/normal loops),
    // summed across every body transformed this frame (subset of the Objects/Chars/Items passes
    // above, not additive with them). Judge the whole GPU-skinning task against this number.
    mu_swprintf(szLine, L"Skinning ms  Transform:%5.2f", FrameProfiler::AccumulatorMs(FP::Skinning));
    g_pRenderText->RenderText((int)DEBUG_TEXT_X, y, szLine); y += DEBUG_TEXT_LINE_HEIGHT;

    // TEMP diagnostic (2026-07-31, Devil Square FPS investigation) — splits the Characters
    // pass above into "waiting on the animation thread pool" vs "everything else" (actual
    // per-character RenderMesh calls), and shows whether this tick's animation ran on the
    // worker thread pool or sequentially on the main thread (PARALLEL_ANIMATION_THRESHOLD = 20
    // active characters, ZzzCharacter.cpp). Remove once the FPS investigation is resolved.
    {
        extern size_t g_LastActiveCharacterCount;
        extern bool   g_LastAnimationWasParallel;
        float charWaitMs = FrameProfiler::AccumulatorMs(FP::CharWait);
        float charRenderMs = FrameProfiler::AccumulatorMs(FP::Characters) - charWaitMs;
        mu_swprintf(szLine, L"CharDbg  Active:%3d  Parallel:%d  Wait:%5.2f  Render:%5.2f",
            (int)g_LastActiveCharacterCount, (int)g_LastAnimationWasParallel, charWaitMs, charRenderMs);
        g_pRenderText->RenderText((int)DEBUG_TEXT_X, y, szLine); y += DEBUG_TEXT_LINE_HEIGHT;
    }

    // Frame time graph below text
    RenderFrameGraph(DEBUG_TEXT_X, (float)y + DEBUG_GRAPH_Y_OFFSET, DEBUG_GRAPH_WIDTH, DEBUG_GRAPH_HEIGHT);

    g_pRenderText->SetFont(g_hFont);
    EndBitmap();

    // FrameProfiler::ResetFrame() used to live here, which meant AccumulatorMs only ever reset
    // when $details itself was on -- with $glstats added as an independent overlay reading the
    // same accumulators, that would show a running session total instead of a per-frame value
    // whenever $details was off. Reset now happens once per frame unconditionally, after every
    // reader (this function and RenderGLStats() below) has had a chance to read -- see the
    // FrameProfiler::ResetFrame()/ResetCounters()/AdvanceGpuTimers() call in MainScene().
}

/**
 * @brief Renders the $glstats overlay: per-pass GL call/draw/buffer counters and GPU pass
 * timers (GLP-01). Independent of $details -- reads the same FrameProfiler accumulators but
 * is gated by its own flag (FrameProfiler::g_CountersEnabled, set via SetShowGLStats()).
 */
static void RenderGLStats()
{
    if (!FrameProfiler::g_CountersEnabled)
        return;

    BeginBitmap();

    wchar_t szLine[128];
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetBgColor(0, 0, 0, 100);
    g_pRenderText->SetTextColor(255, 255, 255, 200);

    // Right-hand column so this can be shown alongside $details' left-aligned overlay without
    // the two overlapping.
    const float x = DEBUG_TEXT_X + 260.0f;
    int y = DEBUG_TEXT_Y_START;

    using FP = FrameProfiler::Pass;
    using FC = FrameProfiler::Counter;

    mu_swprintf(szLine, L"GLStats  Pass       CPUms  GPUms     GL   Draw BufUpd(Orph)");
    g_pRenderText->RenderText((int)x, y, szLine); y += DEBUG_TEXT_LINE_HEIGHT;

    // One row per GL-call-bearing pass. The passes left out are the ones that genuinely issue no
    // GL calls of their own -- MoveEffects, MoveParticles (update-phase simulation), Skinning and
    // CharWait (CPU work) and Present (buffer swap); $details already covers their CPU cost.
    //
    // GLP-24: `Other` used to be left out too, on the same justification, and that was wrong -- it
    // is the remainder bucket, not a CPU-only pass, and it was the frame's largest GL producer.
    // The list had been copied from kGpuTimedPasses, where excluding Other IS correct (it has no
    // timestamp pair). Rows must sum to the printed totals below; if they don't, a pass is missing
    // from this list. Other's GPU-ms column reads 0 by design -- it is untimed, not free.
    static constexpr FP kRows[] = {
        FP::Terrain, FP::Objects, FP::Characters, FP::Items, FP::Effects,
        FP::Sprites, FP::Particles, FP::Joints, FP::UI, FP::Overlay, FP::Other
    };
    for (FP pass : kRows)
    {
        const int gpuIdx = FrameProfiler::GpuTimedIndex(pass);
        const float gpuMs = (gpuIdx >= 0) ? FrameProfiler::GpuMs(pass) : 0.0f;
        // '!' marks a GPU reading that dropped entries at kMaxEntriesPerPass -- a truncated sum
        // must never be mistaken for a complete one.
        const char* truncFlag = (gpuIdx >= 0 && FrameProfiler::GpuMsTruncated(pass)) ? "!" : " ";
        mu_swprintf(szLine, L"%-10hs %6.2f %6.2f%hs %5u %5u %5u(%u)",
            FrameProfiler::kPassNames[(int)pass],
            FrameProfiler::AccumulatorMs(pass),
            gpuMs,
            truncFlag,
            FrameProfiler::CounterValue(pass, FC::GLCalls),
            FrameProfiler::CounterValue(pass, FC::DrawCalls),
            FrameProfiler::CounterValue(pass, FC::BufferUpdates),
            FrameProfiler::CounterValue(pass, FC::BufferOrphans));
        g_pRenderText->RenderText((int)x, y, szLine); y += DEBUG_TEXT_LINE_HEIGHT;
    }

    // Frame totals -- includes binds/uniform writes, which aren't broken out per-row above
    // (would make the table too wide to read at a glance).
    mu_swprintf(szLine, L"Total  GL:%u  Draw:%u  BufUpd:%u(%u)  ProgBind:%u  TexBind:%u  UniWr:%u  UboSkip:%u",
        FrameProfiler::CounterValue(FC::GLCalls),
        FrameProfiler::CounterValue(FC::DrawCalls),
        FrameProfiler::CounterValue(FC::BufferUpdates),
        FrameProfiler::CounterValue(FC::BufferOrphans),
        FrameProfiler::CounterValue(FC::ProgramBinds),
        FrameProfiler::CounterValue(FC::TextureBinds),
        FrameProfiler::CounterValue(FC::UniformWrites),
        FrameProfiler::CounterValue(FC::UboSkips)); // GLP-10
    g_pRenderText->RenderText((int)x, y, szLine); y += DEBUG_TEXT_LINE_HEIGHT;

    // ImmediateRenderer batching. Vtx/Draw is the headline: an IR batch is 6 vertices per quad
    // after decomposition, so ~6 means every sprite is still its own draw call and GLP-19's
    // batching is merging nothing. Anything well above that means runs are merging.
    const uint32_t irDraws = FrameProfiler::CounterValue(FC::IRDraws);
    const uint32_t irVertices = FrameProfiler::CounterValue(FC::IRVertices);
    const float vtxPerDraw = (irDraws > 0) ? ((float)irVertices / (float)irDraws) : 0.0f;
    mu_swprintf(szLine, L"IR     Draw:%u  Vtx:%u  Vtx/Draw:%.1f",
        irDraws, irVertices, vtxPerDraw);
    g_pRenderText->RenderText((int)x, y, szLine); y += DEBUG_TEXT_LINE_HEIGHT;

    // What cut those batches short. The largest bucket is where a batching fix has to aim; a
    // breakdown dominated by Tex means draws are ordered so the texture keeps changing, which
    // is a scheduling problem rather than a batching one.
    mu_swprintf(szLine, L"IRbreak Tex:%u Blend:%u Depth:%u Prog:%u Uni:%u Mtx:%u Draw:%u Other:%u",
        FrameProfiler::CounterValue(FC::IRBreakTexture),
        FrameProfiler::CounterValue(FC::IRBreakBlend),
        FrameProfiler::CounterValue(FC::IRBreakDepth),
        FrameProfiler::CounterValue(FC::IRBreakProgram),
        FrameProfiler::CounterValue(FC::IRBreakUniform),
        FrameProfiler::CounterValue(FC::IRBreakMatrix),
        FrameProfiler::CounterValue(FC::IRBreakDraw),
        FrameProfiler::CounterValue(FC::IRBreakOther));
    g_pRenderText->RenderText((int)x, y, szLine); y += DEBUG_TEXT_LINE_HEIGHT;

    // Per-pass batch quality for the three IR-heavy passes. GLP-24 split these out of Other
    // precisely because they were the frame's largest draw-call producers while invisible;
    // this is the same split for batching.
    static constexpr FP kIRRows[] = { FP::Sprites, FP::Particles, FP::Joints, FP::UI };
    for (FP pass : kIRRows)
    {
        const uint32_t passDraws = FrameProfiler::CounterValue(pass, FC::IRDraws);
        if (passDraws == 0) continue;

        const uint32_t passVertices = FrameProfiler::CounterValue(pass, FC::IRVertices);
        mu_swprintf(szLine, L"  %-9hs Draw:%5u  Vtx/Draw:%5.1f  Tex:%u Blend:%u Draw:%u",
            FrameProfiler::kPassNames[(int)pass],
            passDraws,
            (float)passVertices / (float)passDraws,
            FrameProfiler::CounterValue(pass, FC::IRBreakTexture),
            FrameProfiler::CounterValue(pass, FC::IRBreakBlend),
            FrameProfiler::CounterValue(pass, FC::IRBreakDraw));
        g_pRenderText->RenderText((int)x, y, szLine); y += DEBUG_TEXT_LINE_HEIGHT;
    }

    g_pRenderText->SetFont(g_hFont);
    EndBitmap();
}

/**
 * @brief Renders a simple FPS counter overlay showing only current FPS.
 */
static void RenderFpsCounter()
{
    if (!g_bShowFpsCounter)
        return;

    BeginBitmap();

    wchar_t szLine[64];
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetBgColor(0, 0, 0, 100);
    g_pRenderText->SetTextColor(255, 255, 255, 200);

    mu_swprintf(szLine, L"FPS: %.1f", FPS_AVG);
    g_pRenderText->RenderText((int)DEBUG_TEXT_X, DEBUG_TEXT_Y_START, szLine);

    g_pRenderText->SetFont(g_hFont);
    EndBitmap();
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
        CUIMng::Instance().PopUpMsgWin(MESSAGE_SERVER_LOST);
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
        {
            // GLP-24: tagged Overlay, not Other. These three render text as roughly one IR quad per
            // glyph, so with $glstats on they were adding hundreds of draw calls to the very bucket
            // the overlay exists to investigate -- an observer effect big enough to mislead. Keep
            // the Reset/Advance calls inside this scope; see the comment below for the ordering.
            FRAME_PROFILE(Overlay);
            RenderDebugInfo();
            RenderGLStats();
            RenderFpsCounter();
            UI::Reconnect::RenderDialog();

            // Once per frame, unconditionally -- see the comment at the end of RenderDebugInfo()
            // for why this can't live inside either overlay function. AdvanceGpuTimers() must run
            // after this frame's Terrain/Objects/Characters/Items/Effects/UI passes have all
            // issued their GpuTimerBegin/End calls, which RenderCurrentScene() above guarantees.
#ifdef __ANDROID__
            // AH-1118 batching pass: dump the frame's GL statistics to logcat
            // every ~5s, before the reset below wipes them.
            {
                FrameProfiler::g_CountersEnabled = true;
                using clock = std::chrono::steady_clock;
                static clock::time_point s_lastDump{};
                const clock::time_point nowT = clock::now();
                if (nowT - s_lastDump >= std::chrono::seconds(5))
                {
                    s_lastDump = nowT;
                    using FrameProfiler::Counter;
                    using FrameProfiler::CounterValue;
                    using FrameProfiler::Pass;
                    __android_log_print(ANDROID_LOG_INFO, "MuMainGL",
                        "tot gl=%u draw=%u bufUp=%u orphan=%u prog=%u tex=%u uni=%u",
                        CounterValue(Counter::GLCalls), CounterValue(Counter::DrawCalls),
                        CounterValue(Counter::BufferUpdates), CounterValue(Counter::BufferOrphans),
                        CounterValue(Counter::ProgramBinds), CounterValue(Counter::TextureBinds),
                        CounterValue(Counter::UniformWrites));
                    for (int p = 0; p < static_cast<int>(Pass::Count_); ++p)
                    {
                        const uint32_t gl = CounterValue(static_cast<Pass>(p), Counter::GLCalls);
                        if (gl >= 200)
                        {
                            __android_log_print(ANDROID_LOG_INFO, "MuMainGL",
                                "pass %s gl=%u draw=%u tex=%u buf=%u",
                                FrameProfiler::kPassNames[p], gl,
                                CounterValue(static_cast<Pass>(p), Counter::DrawCalls),
                                CounterValue(static_cast<Pass>(p), Counter::TextureBinds),
                                CounterValue(static_cast<Pass>(p), Counter::BufferUpdates));
                        }
                    }
                    __android_log_print(ANDROID_LOG_INFO, "MuMainGL",
                        "IR draws=%u verts=%u brk tex=%u blend=%u depth=%u prog=%u uni=%u mtx=%u draw=%u oth=%u",
                        CounterValue(Counter::IRDraws), CounterValue(Counter::IRVertices),
                        CounterValue(Counter::IRBreakTexture), CounterValue(Counter::IRBreakBlend),
                        CounterValue(Counter::IRBreakDepth), CounterValue(Counter::IRBreakProgram),
                        CounterValue(Counter::IRBreakUniform), CounterValue(Counter::IRBreakMatrix),
                        CounterValue(Counter::IRBreakDraw), CounterValue(Counter::IRBreakOther));
                }
            }
#endif
            FrameProfiler::ResetFrame();
            FrameProfiler::ResetCounters();
            FrameProfiler::AdvanceGpuTimers();
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
            // DXP-23: split into its own Present bucket (was folded into Other). A large reading
            // here means the CPU is stalling waiting for the GPU command queue to drain (GPU-side
            // cost, e.g. vsync block or genuine fill-rate/shader cost), not CPU-side render logic.
            {
                FRAME_PROFILE(Present);
                PlatformSwapBuffers();
            }
        }

        CheckServerConnection();
        ManageMainSceneAudio();
    }
    catch (const std::exception& e)
    {
        // DXP-16: was OutputDebugStringA-only, invisible without a debugger attached. Since
        // PlatformSwapBuffers() above only runs `if (Success)` (Success comes from
        // RenderCurrentScene()'s return value), anything throwing in that path silently skips the
        // frame's Present() with zero trace in MuError.log -- mirror it to g_ErrorReport too so a
        // skipped-frame theory is checkable instead of invisible.
        char errorMsg[256];
        sprintf_s(errorMsg, sizeof(errorMsg), "Exception in MainScene: %s", e.what());
        OutputDebugStringA(errorMsg);
        g_ErrorReport.Write(L"[MainScene] std::exception: %hs\r\n", e.what());
    }
    catch (...)
    {
        // DXP-16: pairs with the std::exception catch above -- a non-std exception (SEH,
        // _com_error, etc.) previously had no handler here at all, which for MSVC's default SEH
        // translation could itself explain a skipped/aborted frame with zero log trace anywhere.
        g_ErrorReport.Write(L"[MainScene] non-std exception (SEH/other)\r\n");
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

    // Pairs with PlatformSwapBuffers()'s RHI::EndFrame() call -- reserved per-frame
    // backend hook (currently a no-op on GL).
    RHI::BeginFrame();

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
