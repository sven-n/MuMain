//*****************************************************************************
// File: NewUIBattleSoccerScore.cpp
//
// Desc: implementation of the CNewUIBattleSoccerScore class.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#include "stdafx.h"
#include "NewUIBattleSoccerScore.h"
#include "UIControls.h"
#include "WSclient.h"
#include "ZzzBMD.h"
#include "ZzzCharacter.h"
#include "ZzzInventory.h"
#include "ZzzTexture.h"

using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SEASON3B::CNewUIBattleSoccerScore::CNewUIBattleSoccerScore()
{
	m_pNewUIMng = NULL;
	m_Pos.x = m_Pos.y = 0;
}

SEASON3B::CNewUIBattleSoccerScore::~CNewUIBattleSoccerScore()
{
	Release();
}

//*****************************************************************************
// 함수 이름 : Create()
// 함수 설명 : 창 생성.
// 매개 변수 : pNewUIMng	: CNewUIManager 오브젝트 주소.
//			   x			: x 좌표.
//			   y			: y 좌표.
//*****************************************************************************
bool SEASON3B::CNewUIBattleSoccerScore::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if (NULL == pNewUIMng)
		return false;

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_BATTLE_SOCCER_SCORE, this);

	SetPos(x, y);

	LoadImages();

	Show(false);	

	return true;
}

//*****************************************************************************
// 함수 이름 : Release()
// 함수 설명 : 창 Release.
//*****************************************************************************
void SEASON3B::CNewUIBattleSoccerScore::Release()
{
	UnloadImages();

	if (m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}
}

//*****************************************************************************
// 함수 이름 : SetPos()
// 함수 설명 : 창 위치 지정.
//*****************************************************************************
void SEASON3B::CNewUIBattleSoccerScore::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
}

//*****************************************************************************
// 함수 이름 : UpdateMouseEvent()
// 함수 설명 : 마우스 이벤트 처리.
// 반환 값	 : true면 창 뒤로도 이벤트를 처리.
//*****************************************************************************
bool SEASON3B::CNewUIBattleSoccerScore::UpdateMouseEvent()
{
	return true;
}

//*****************************************************************************
// 함수 이름 : UpdateKeyEvent()
// 함수 설명 : 키보드 입력 처리.
// 반환 값	 : true면 창 뒤로도 이벤트를 처리.
//*****************************************************************************
bool SEASON3B::CNewUIBattleSoccerScore::UpdateKeyEvent()
{
	return true;
}

//*****************************************************************************
// 함수 이름 : Update()
// 함수 설명 : 기타 매 프레임 일어나는 이벤트 처리.
// 반환 값	 : true면 창 뒤로도 이벤트를 처리.
//*****************************************************************************
bool SEASON3B::CNewUIBattleSoccerScore::Update()
{
	return true;
}

//*****************************************************************************
// 함수 이름 : Render()
// 함수 설명 : 창 렌더.
// 반환 값	 : true면 성공.
//*****************************************************************************
bool SEASON3B::CNewUIBattleSoccerScore::Render()
{
	::EnableAlphaTest();

	::glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	RenderBackImage();
	RenderContents();

	::DisableAlphaBlend();
	
	return true;
}

//*****************************************************************************
// 함수 이름 : RenderBackImage()
// 함수 설명 : 창 바탕 이미지 렌더.
//*****************************************************************************
void SEASON3B::CNewUIBattleSoccerScore::RenderBackImage()
{
	RenderImage(IMAGE_BSS_BACK,
		m_Pos.x, m_Pos.y, float(BSS_WIDTH), float(BSS_HEIGHT));
}

//*****************************************************************************
// 함수 이름 : RenderContents()
// 함수 설명 : 내용 렌더.
//*****************************************************************************
void SEASON3B::CNewUIBattleSoccerScore::RenderContents()
{
	unicode::t_char szTemp[128];
	int nX = m_Pos.x + 30;
	int nY = m_Pos.y + 33;

	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetBgColor(0, 0, 0, 128);

	if (EnableGuildWar && Hero->GuildMarkIndex!=-1)
	{
		if (HeroSoccerTeam == 0)
			g_pRenderText->SetTextColor(255, 60, 0, 255);
		else
			g_pRenderText->SetTextColor(0, 150, 255, 255);

		unicode::_sprintf(szTemp, "%d", GuildWarScore[0]);
		g_pRenderText->RenderText(nX, nY, szTemp);				// 점수
        ::CreateGuildMark(Hero->GuildMarkIndex);
        ::RenderBitmap(BITMAP_GUILD, float(nX+21), float(nY), 8, 8);// 길드 마크
		g_pRenderText->RenderText(nX+33, nY, GuildMark[Hero->GuildMarkIndex].GuildName);// 길드명

		if (HeroSoccerTeam == 0)
			g_pRenderText->SetTextColor(0, 150, 255, 255);
		else
			g_pRenderText->SetTextColor(255, 60, 0, 255);

		unicode::_sprintf(szTemp, "%d", GuildWarScore[1]);
		g_pRenderText->RenderText(nX, nY+22, szTemp);			// 점수
        ::CreateGuildMark(FindGuildMark(GuildWarName));
        ::RenderBitmap(BITMAP_GUILD, float(nX+21), float(nY+22), 8, 8);// 길드 마크
		g_pRenderText->RenderText(nX+33, nY+22, GuildWarName);	// 길드명
	}
	else if (SoccerObserver)
	{
		g_pRenderText->SetTextColor(255, 60, 0, 255);
		unicode::_sprintf(szTemp, "%d", GuildWarScore[0]);
		g_pRenderText->RenderText(nX, nY, szTemp);
        ::CreateGuildMark(FindGuildMark(SoccerTeamName[0]));
        ::RenderBitmap(BITMAP_GUILD, float(nX+21), float(nY), 8, 8);
		g_pRenderText->RenderText(nX+33, nY, SoccerTeamName[0]);

		g_pRenderText->SetTextColor(0, 150, 255, 255);
		unicode::_sprintf(szTemp,"%d",GuildWarScore[1]);
		g_pRenderText->RenderText(nX, nY+22, szTemp);
        ::CreateGuildMark(FindGuildMark(SoccerTeamName[1]));
        ::RenderBitmap(BITMAP_GUILD, float(nX+21), float(nY+22), 8, 8);
		g_pRenderText->RenderText(nX+33, nY+22, SoccerTeamName[1]);
	}
}

//*****************************************************************************
// 함수 이름 : FindGuildMark()
// 함수 설명 : 길드명으로 길드 마크를 찾음.
// 반환 값	 : 길드마크 인덱스.
// 매개 변수 : pszGuildName	: 길드명.
//*****************************************************************************
int SEASON3B::CNewUIBattleSoccerScore::FindGuildMark(char* pszGuildName)
{
	for (int i = 0; i < MARK_EDIT; ++i)
	{
		MARK_t *p = &GuildMark[i];
		if (strcmp(p->GuildName, pszGuildName) == NULL)
		{
			return i;
		}
	}
	return 0;
}

//*****************************************************************************
// 함수 이름 : GetLayerDepth()
// 함수 설명 : 창의 레이어값을 얻음.
//*****************************************************************************
float SEASON3B::CNewUIBattleSoccerScore::GetLayerDepth()
{
	return 1.8f;
}

//*****************************************************************************
// 함수 이름 : LoadImages()
// 함수 설명 : 이미지 리소스 로드.
//*****************************************************************************
void SEASON3B::CNewUIBattleSoccerScore::LoadImages()
{
	LoadBitmap("Interface\\newui_Figure_ground.tga", IMAGE_BSS_BACK, GL_LINEAR);
}

//*****************************************************************************
// 함수 이름 : UnloadImages()
// 함수 설명 : 이미지 리소스 삭제.
//*****************************************************************************
void SEASON3B::CNewUIBattleSoccerScore::UnloadImages()
{
	DeleteBitmap(IMAGE_BSS_BACK);
}
