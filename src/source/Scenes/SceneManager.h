#pragma once

// SceneManager.h - Top-level scene orchestration

#include <windows.h>

//=============================================================================
// Frame Timing State
//=============================================================================

class FrameTimingState {
private:
    double targetFps = 60.0;
    double msPerFrame = 1000.0 / 60.0;

public:
    double lastRenderTickCount = 0.0;
    double currentTickCount = 0.0;
    double lastWaterChange = 0.0;

    // Set target frames per second
    void SetTargetFps(double fps) {
        if (fps > 0) {
            targetFps = fps;
            msPerFrame = 1000.0 / fps;
        }
    }

    // Get current target FPS
    double GetTargetFps() const {
        return targetFps;
    }

    // Get milliseconds per frame
    double GetMsPerFrame() const {
        return msPerFrame;
    }

    // Check if enough time has passed to render next frame
    bool ShouldRenderNextFrame() const {
        return (currentTickCount - lastRenderTickCount) >= msPerFrame;
    }

    // Update current time
    void UpdateCurrentTime(double time) {
        currentTickCount = time;
    }

    // Mark that a frame was just rendered
    void MarkFrameRendered() {
        lastRenderTickCount = currentTickCount;
    }

    // Get current frame time in ms
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
