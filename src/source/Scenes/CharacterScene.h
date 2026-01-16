#pragma once

// CharacterScene.h - Character selection scene

#include <windows.h>

// Character scene lifecycle
void CreateCharacterScene();
void NewMoveCharacterScene();
bool NewRenderCharacterScene(HDC hDC);

// Character management
void StartGame();

// State variables (extern - defined in ZzzScene.cpp)
extern int SelectedHero;
