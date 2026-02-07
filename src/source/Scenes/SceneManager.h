#pragma once

// SceneManager.h - Top-level scene orchestration

#include <windows.h>

//=============================================================================
// Frame Timing State
//=============================================================================

class FrameTimingState {
private:
    double targetFps = -1.0;      // -1 = uncapped, >0 = specific FPS limit
    double msPerFrame = 0.0;      // 0 = no frame limiting (uncapped)

public:
    double lastRenderTickCount = 0.0;
    double currentTickCount = 0.0;
    double lastWaterChange = 0.0;

    /**
     * Set target frames per second for frame rate limiting
     * @param fps Target FPS. Use -1 or any negative value for uncapped (unlimited) framerate.
     *            Use positive values (e.g., 60, 144) to cap framerate at specific FPS.
     *            Zero is ignored and keeps current settings.
     *
     * Example:
     *   SetTargetFps(60)   -> Caps framerate at 60 FPS
     *   SetTargetFps(-1)   -> Uncapped framerate (render as fast as possible)
     *   SetTargetFps(144)  -> Caps framerate at 144 FPS
     */
    void SetTargetFps(double fps) {
        if (fps < 0) {
            // Negative FPS means unlimited/uncapped framerate
            targetFps = -1;
            msPerFrame = 0.0;  // Zero msPerFrame = no frame limiting
        }
        else if (fps > 0) {
            // Positive FPS means cap at specific framerate
            targetFps = fps;
            msPerFrame = 1000.0 / fps;  // Calculate milliseconds per frame
        }
        // If fps == 0, do nothing (keep current settings)
    }

    /**
     * Get current target FPS setting
     * @return Target FPS. Returns -1 if uncapped, positive value if capped.
     */
    double GetTargetFps() const {
        return targetFps;
    }

    /**
     * Get milliseconds per frame for current FPS setting
     * @return Milliseconds per frame. Returns 0.0 if uncapped (no limiting).
     */
    double GetMsPerFrame() const {
        return msPerFrame;
    }

    /**
     * Check if enough time has passed to render the next frame
     * @return true if frame should be rendered now, false if should wait
     *
     * Behavior:
     * - Uncapped mode (msPerFrame <= 0): Always returns true (render every frame)
     * - Capped mode: Returns true only when enough time has elapsed for target FPS
     */
    bool ShouldRenderNextFrame() const {
        if (msPerFrame <= 0.0) {
            return true;  // Uncapped: always render
        }
        return (currentTickCount - lastRenderTickCount) >= msPerFrame;
    }

    /**
     * Update the current tick count (call once per game loop iteration)
     * @param time Current time in milliseconds from timer
     */
    void UpdateCurrentTime(double time) {
        currentTickCount = time;
    }

    /**
     * Mark that a frame was just rendered (updates lastRenderTickCount)
     * Call this after completing a frame render.
     */
    void MarkFrameRendered() {
        lastRenderTickCount = currentTickCount;
    }

    /**
     * Get the time elapsed since last frame was rendered
     * @return Milliseconds since last frame
     */
    double GetCurrentFrameTime() const {
        return currentTickCount - lastRenderTickCount;
    }
};

// Global frame timing state
extern FrameTimingState g_frameTiming;

//=============================================================================
// Scene orchestration
//=============================================================================

void UpdateSceneState();
void RenderScene(HDC Hdc);
void MainScene(HDC hDC);

// FPS management (legacy - use g_frameTiming instead)
void SetTargetFps(double targetFps);
double GetTargetFps();

// Debug overlay controls
void SetShowDebugInfo(bool enabled);
void SetShowFpsCounter(bool enabled);
void ResetFrameStats();
