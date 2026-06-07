#pragma once

// CharacterScene.h - Character selection scene

#include "Core/Platform/WinCompat.h"

// Character scene lifecycle
void CreateCharacterScene();
void NewMoveCharacterScene();
bool NewRenderCharacterScene(HDC hDC);

// Character management
void StartGame();
