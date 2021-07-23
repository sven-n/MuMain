// GameCensorship.cpp: implementation of the CGameCensorship class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GameCensorship.h"
#include "Local.h"

#ifndef KJH_ADD_SERVER_LIST_SYSTEM			// #ifndef
extern int ServerLocalSelect;
#endif // KJH_ADD_SERVER_LIST_SYSTEM
extern float g_fScreenRate_x;
extern float g_fScreenRate_y;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


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
#if defined PBG_MOD_GAMECENSORSHIP || defined PBG_MOD_GAMECENSORSHIP_RED
#ifndef PBG_MOD_GAMECENSORSHIP_RED
			if(BLUE_MU::IsBlueMuServer())
#endif //PBG_MOD_GAMECENSORSHIP_RED
				if(i==SPRITE_12 || i==SPRITE_15
#ifdef PBG_MOD_GAMECENSORSHIP_RED
					|| (i == SPRITE_FEAR)
#endif //PBG_MOD_GAMECENSORSHIP_RED
					)
					continue;
#endif //PBG_MOD_GAMECENSORSHIP
				m_ImageSprite[i].Render();
			}
		}
		break;
	case STATE_12:
		{
#ifdef PBG_FIX_GAMECENSORSHIP_1215
			m_ImageSprite[SPRITE_18].Render();
			m_ImageSprite[SPRITE_VIOLENCE].Render();
			break;
#else //PBG_FIX_GAMECENSORSHIP_1215
#ifdef PBG_MOD_GAMECENSORSHIP
			if(BLUE_MU::IsBlueMuServer())
				break;		//여기에 들어올 경우는 없다
#endif //PBG_MOD_GAMECENSORSHIP
#endif //PBG_FIX_GAMECENSORSHIP_1215
			m_ImageSprite[SPRITE_12].Render();
			m_ImageSprite[SPRITE_FEAR].Render();
			m_ImageSprite[SPRITE_VIOLENCE].Render();
		}
		break;
	case STATE_15:
		{
#ifdef PBG_FIX_GAMECENSORSHIP_1215
			m_ImageSprite[SPRITE_18].Render();
			m_ImageSprite[SPRITE_VIOLENCE].Render();
			break;
#else //PBG_FIX_GAMECENSORSHIP_1215
#ifdef PBG_MOD_GAMECENSORSHIP
			if(BLUE_MU::IsBlueMuServer())
				break;		//여기에 들어올 경우는 없다
#endif //PBG_MOD_GAMECENSORSHIP
#endif //PBG_FIX_GAMECENSORSHIP_1215
			m_ImageSprite[SPRITE_15].Render();
			m_ImageSprite[SPRITE_FEAR].Render();
			m_ImageSprite[SPRITE_VIOLENCE].Render();
		}
		break;
	case STATE_18:
		{
			m_ImageSprite[SPRITE_18].Render();
#ifndef PBG_MOD_GAMECENSORSHIP_RED
			m_ImageSprite[SPRITE_FEAR].Render();
#endif //PBG_MOD_GAMECENSORSHIP_RED
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
#ifdef PBG_ADD_NEWLOGO_IMAGECHANGE
#ifdef PBG_MOD_GAMECENSORSHIP_RED
			m_ImageSprite[SPRITE_18].SetPosition((iScreenWidth-iWidth*2), 5);
#else //PBG_MOD_GAMECENSORSHIP_RED
			m_ImageSprite[SPRITE_12].SetPosition((iScreenWidth-iWidth*5), 5);
			m_ImageSprite[SPRITE_15].SetPosition((iScreenWidth-iWidth*4), 5);
			m_ImageSprite[SPRITE_18].SetPosition((iScreenWidth-iWidth*3), 5);
			m_ImageSprite[SPRITE_FEAR].SetPosition((iScreenWidth-iWidth*2), 5);
#endif //PBG_MOD_GAMECENSORSHIP_RED
			m_ImageSprite[SPRITE_VIOLENCE].SetPosition((iScreenWidth-iWidth*1), 5);
#else //PBG_ADD_NEWLOGO_IMAGECHANGE
			m_ImageSprite[SPRITE_12].SetPosition((iScreenWidth-iWidth*5), 62);
			m_ImageSprite[SPRITE_15].SetPosition((iScreenWidth-iWidth*4), 62);
			m_ImageSprite[SPRITE_18].SetPosition((iScreenWidth-iWidth*3), 62);
			m_ImageSprite[SPRITE_FEAR].SetPosition((iScreenWidth-iWidth*2), 62);
			m_ImageSprite[SPRITE_VIOLENCE].SetPosition((iScreenWidth-iWidth*1), 62);
#endif //PBG_ADD_NEWLOGO_IMAGECHANGE
		}
		break;
#ifndef PBG_FIX_GAMECENSORSHIP_1215
	case STATE_12:
		{
			m_ImageSprite[SPRITE_12].SetPosition(iScreenWidth-iWidth*3, 0);
			m_ImageSprite[SPRITE_FEAR].SetPosition(iScreenWidth-iWidth*2, 0);
			m_ImageSprite[SPRITE_VIOLENCE].SetPosition(iScreenWidth-iWidth, 0);
		}
		break;
	case STATE_15:
		{
			m_ImageSprite[SPRITE_15].SetPosition(iScreenWidth-iWidth*3, 0);
			m_ImageSprite[SPRITE_FEAR].SetPosition(iScreenWidth-iWidth*2, 0);
			m_ImageSprite[SPRITE_VIOLENCE].SetPosition(iScreenWidth-iWidth, 0);
		}
		break;
#endif //PBG_FIX_GAMECENSORSHIP_1215
#ifdef PBG_FIX_GAMECENSORSHIP_1215
	case STATE_12:
	case STATE_15:
#endif //PBG_FIX_GAMECENSORSHIP_1215
	case STATE_18:
		{
#ifdef PBG_MOD_GAMECENSORSHIP_RED
			m_ImageSprite[SPRITE_18].SetPosition(iScreenWidth-iWidth*2, 0);
#else //PBG_MOD_GAMECENSORSHIP_RED
			m_ImageSprite[SPRITE_18].SetPosition(iScreenWidth-iWidth*3, 0);
			m_ImageSprite[SPRITE_FEAR].SetPosition(iScreenWidth-iWidth*2, 0);
#endif //PBG_MOD_GAMECENSORSHIP_RED
			m_ImageSprite[SPRITE_VIOLENCE].SetPosition(iScreenWidth-iWidth, 0);
		}
		break;
	}
}

#ifndef KJH_ADD_SERVER_LIST_SYSTEM				// #ifndef
void SEASON3A::CGameCensorship::SetGameState()
{
	DWORD dwState;

	// 레알서버이면
	if(strcmp(ServerList[ServerSelectHi].Name, GlobalText[1829]) == 0
#ifdef PBG_MOD_GAMECENSORSHIP
		|| BLUE_MU::IsBlueMuServer()
#endif //PBG_MOD_GAMECENSORSHIP
		)
	{
		dwState = STATE_18;
	}
	// 레알서버가 아니고
	else
	{
		// NON_PVP 이면
		if(IsNonPvpServer( ServerSelectHi, ServerLocalSelect) == TRUE)
		{
			dwState = STATE_12;
		}
		// PVP 이면
		else
		{
			dwState = STATE_15;
		}
	}

	SetState(dwState);
}
#endif // KJH_ADD_SERVER_LIST_SYSTEM
