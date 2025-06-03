// Camera.cpp: implementation of the CCamera class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Camera3D.h"
#include "ZzzLodTerrain.h"
#include "NewUISystem.h"

CCamera gCamera;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCamera::CCamera() // OK
{
	this->LastPress = GetTickCount();
	this->m_CursorX = MouseX;
	this->m_CursorY = MouseY;
}

CCamera::~CCamera() // OK
{

}

void CCamera::Toggle() // OK
{
	m_CameraOnOff ^= 1;
}

void CCamera::Restore() // OK
{
	CameraZoom = 0;
	CameraAngle[2] = -45.f;
	AngleY3D = 0;
	AngleZ3D = 0;
}


void CCamera::Update() // OK
{
	if (SceneFlag != MAIN_SCENE) return;

	if (HIBYTE(GetAsyncKeyState(VK_F11)) == 128)
	{
		if ((GetTickCount() - this->LastPress) > 2000) {
			this->LastPress = GetTickCount();
			this->Toggle();
		}
		return;
	}

	if (!m_CameraOnOff) return;

	if (HIBYTE(GetAsyncKeyState(VK_F10)) == 128)
	{
		this->Restore();

		return;
	}

	if (MouseWheel > 0 && CameraZoom > -18) {
		CameraZoom -= 2;

		if (CameraZoom < -8) {
			AngleY3D -= 4.0f;
			AngleZ3D -= 68.0f;
		}
	}

	if (MouseWheel < 0 && CameraZoom < 8) {
		CameraZoom += 2;

		if (CameraZoom > -18 && CameraZoom <= -8) {
			AngleY3D += 4.0f;
			AngleZ3D += 68.0f;
		}
	}

	if (MouseWheel != 0)
		MouseWheel = 0;

	this->m_CursorX = MouseX;

	this->m_CursorY = MouseY;

	return;
}