#pragma once

// SceneManager.h - Top-level scene orchestration

#include <windows.h>

// Scene orchestration
void UpdateSceneState();
void RenderScene(HDC Hdc);
void MainScene(HDC hDC);

// FPS management
void SetTargetFps(double targetFps);
double GetTargetFps();
