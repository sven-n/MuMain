#pragma once

// LoginScene.h - Login scene management

#include "Core/Platform/WinCompat.h"
#include "Render/Textures/ZzzOpenglUtil.h"  // For vec3_t

// Login scene lifecycle
void CreateLogInScene();
void NewMoveLogInScene();
bool NewRenderLogInScene(HDC hDC);

// The login screen's music, asked for every frame: the credits' theme while they are open, the login theme
// otherwise and on the frame a login goes through, so the character select keeps the login theme.
void PlayLogInSceneMusic();

// Camera animation
void MoveCamera();
void MoveCharacterCamera(vec3_t Origin, vec3_t Position, vec3_t Angle);

// LoginScene camera accessors (for external use)
int GetLoginCameraCount();
int GetLoginCameraWalkCut();

// State variables (extern - defined in ZzzScene.cpp)
extern int DeleteGuildIndex;
