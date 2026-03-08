#pragma once

// CharacterScene.h - Character selection scene

#ifdef _WIN32
#include <windows.h>
#else
#include "Platform/PlatformTypes.h"
#include "Platform/PlatformCompat.h"
#endif

// Character scene lifecycle
void CreateCharacterScene();
void NewMoveCharacterScene();
bool NewRenderCharacterScene(HDC hDC);

// Character management
void StartGame();
