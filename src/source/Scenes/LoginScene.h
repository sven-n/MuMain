#pragma once

// LoginScene.h - Login scene management

#include <windows.h>
#include "../ZzzOpenglUtil.h"  // For vec3_t

// Login scene lifecycle
void CreateLogInScene();
void NewMoveLogInScene();
bool NewRenderLogInScene(HDC hDC);

// Camera animation
void MoveCamera();
void MoveCharacterCamera(vec3_t Origin, vec3_t Position, vec3_t Angle);

// Character deletion
void DeleteCharacter();

// State variables (extern - defined in ZzzScene.cpp)
extern int DeleteGuildIndex;
extern bool MenuCancel;
extern int MenuY;
extern int MenuX;
extern int CameraWalkCut;
extern int CurrentCameraCount;
extern int CurrentCameraWalkType;
extern int CurrentCameraNumber;
extern float CurrentCameraPosition[3];
extern float CurrentCameraAngle[3];
extern float CurrentCameraWalkDelta[6];
extern float CameraWalk[36];
