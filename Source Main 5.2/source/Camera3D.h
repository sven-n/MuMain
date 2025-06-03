#pragma once

class CCamera
{
public:
	CCamera();
	virtual ~CCamera();
	void Toggle();
	void Restore();
	void Update();
private:
	DWORD LastPress;
	int m_CursorX;
	int m_CursorY;
};

extern CCamera gCamera;