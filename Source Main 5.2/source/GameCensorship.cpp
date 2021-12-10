// GameCensorship.cpp: implementation of the CGameCensorship class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GameCensorship.h"
#include "Local.h"

extern float g_fScreenRate_x;
extern float g_fScreenRate_y;

SEASON3A::CGameCensorship::CGameCensorship()
{
	m_bVisible = false;

	m_fScreenRateX = 1.f;
	m_fScreenRateY = 1.f;

	Create();
}

SEASON3A::CGameCensorship::~CGameCensorship()
{

}

SEASON3A::CGameCensorship* SEASON3A::CGameCensorship::GetInstance()
{
	static CGameCensorship s_GameCensorship;
	return &s_GameCensorship;
}

void SEASON3A::CGameCensorship::Create()
{
	LoadImage();

	m_fScreenRateX = (float)WindowWidth / 800.0f;
	m_fScreenRateY = (float)WindowHeight / 600.0f;
	m_ImageSprite[SPRITE_12].Create(99, 116, IMAGE_12, 0, NULL, 0, 0, false, SPR_SIZING_DATUMS_LT, m_fScreenRateX, m_fScreenRateY);
	m_ImageSprite[SPRITE_15].Create(99, 116, IMAGE_15, 0, NULL, 0, 0, false, SPR_SIZING_DATUMS_LT, m_fScreenRateX, m_fScreenRateY);
	m_ImageSprite[SPRITE_18].Create(99, 116, IMAGE_18, 0, NULL, 0, 0, false, SPR_SIZING_DATUMS_LT, m_fScreenRateX, m_fScreenRateY);
	m_ImageSprite[SPRITE_FEAR].Create(99, 116, IMAGE_FEAR, 0, NULL, 0, 0, false, SPR_SIZING_DATUMS_LT, m_fScreenRateX, m_fScreenRateY);
	m_ImageSprite[SPRITE_VIOLENCE].Create(99, 116, IMAGE_VIOLENCE, 0, NULL, 0, 0, false, SPR_SIZING_DATUMS_LT, m_fScreenRateX, m_fScreenRateY);

	for(int i=0; i<SPRITE_COUNT; ++i)
	{
		m_ImageSprite[i].SetSize(50, 58);
		m_ImageSprite[i].Show();
	}
}	

void SEASON3A::CGameCensorship::LoadImage()
{
	LoadBitmap("Interface\\gamecensorship_12.tga", IMAGE_12, GL_LINEAR);
	LoadBitmap("Interface\\gamecensorship_15.tga", IMAGE_15, GL_LINEAR);
	LoadBitmap("Interface\\gamecensorship_18.tga", IMAGE_18, GL_LINEAR);
	LoadBitmap("Interface\\gamecensorship_fear.tga", IMAGE_FEAR, GL_LINEAR);
	LoadBitmap("Interface\\gamecensorship_vio.tga", IMAGE_VIOLENCE, GL_LINEAR);
}

void SEASON3A::CGameCensorship::Release()
{
	DeleteBitmap(IMAGE_12);
	DeleteBitmap(IMAGE_15);
	DeleteBitmap(IMAGE_18);
	DeleteBitmap(IMAGE_FEAR);
	DeleteBitmap(IMAGE_VIOLENCE);
}

void SEASON3A::CGameCensorship::Update()
{
	if(m_bVisible == true)
	{
		if(m_Timer.GetTimeElapsed() >= 4000)
		{
			m_bVisible = false;
		}
	}
}

void SEASON3A::CGameCensorship::Render()
{
	if(m_bVisible == false)
		return;

	switch(m_dwState)
	{
	case STATE_LOADING:
		{
			for(int i=0; i<SPRITE_COUNT; ++i)
			{
				m_ImageSprite[i].Render();
			}
		}
		break;
	case STATE_12:
		{
			m_ImageSprite[SPRITE_18].Render();
			m_ImageSprite[SPRITE_VIOLENCE].Render();
			break;
		}
		break;
	case STATE_15:
		{
			m_ImageSprite[SPRITE_18].Render();
			m_ImageSprite[SPRITE_VIOLENCE].Render();
			break;
		}
		break;
	case STATE_18:
		{
			m_ImageSprite[SPRITE_18].Render();
			m_ImageSprite[SPRITE_FEAR].Render();
			m_ImageSprite[SPRITE_VIOLENCE].Render();
		}
		break;
	}
}

bool SEASON3A::CGameCensorship::IsVisible()
{
	return m_bVisible;
}

void SEASON3A::CGameCensorship::SetVisible(bool bVisible)
{
	m_bVisible = bVisible;

	if(m_bVisible == true)
	{
		m_Timer.ResetTimer();
	}
}

void SEASON3A::CGameCensorship::SetState(DWORD dwState)
{
	m_dwState = dwState;

	int iScreenWidth = 800;
	int iWidth = 50;

	switch(m_dwState)
	{
	case STATE_LOADING:
		{
			m_ImageSprite[SPRITE_12].SetPosition((iScreenWidth-iWidth*5), 5);
			m_ImageSprite[SPRITE_15].SetPosition((iScreenWidth-iWidth*4), 5);
			m_ImageSprite[SPRITE_18].SetPosition((iScreenWidth-iWidth*3), 5);
			m_ImageSprite[SPRITE_FEAR].SetPosition((iScreenWidth-iWidth*2), 5);
			m_ImageSprite[SPRITE_VIOLENCE].SetPosition((iScreenWidth-iWidth*1), 5);
			m_ImageSprite[SPRITE_12].SetPosition((iScreenWidth-iWidth*5), 62);
			m_ImageSprite[SPRITE_15].SetPosition((iScreenWidth-iWidth*4), 62);
			m_ImageSprite[SPRITE_18].SetPosition((iScreenWidth-iWidth*3), 62);
			m_ImageSprite[SPRITE_FEAR].SetPosition((iScreenWidth-iWidth*2), 62);
			m_ImageSprite[SPRITE_VIOLENCE].SetPosition((iScreenWidth-iWidth*1), 62);
		}
		break;
	case STATE_12:
	case STATE_15:
	case STATE_18:
		{
			m_ImageSprite[SPRITE_18].SetPosition(iScreenWidth-iWidth*3, 0);
			m_ImageSprite[SPRITE_FEAR].SetPosition(iScreenWidth-iWidth*2, 0);
			m_ImageSprite[SPRITE_VIOLENCE].SetPosition(iScreenWidth-iWidth, 0);
		}
		break;
	}
}