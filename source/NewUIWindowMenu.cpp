// NewUIWindowMenu.cpp: implementation of the CNewUIWindowMenu class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIWindowMenu.h"
#include "NewUISystem.h"
#include "NewUICustomMessageBox.h"
#include "ZzzTexture.h"
#include "UIControls.h"
#include "DSPlaySound.h"

using namespace SEASON3B;



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SEASON3B::CNewUIWindowMenu::CNewUIWindowMenu()
{
	m_pNewUIMng = NULL;
	m_Pos.x = m_Pos.y = 0;
	m_iSelectedIndex = -1;
}

SEASON3B::CNewUIWindowMenu::~CNewUIWindowMenu()
{
	Release();
}

bool SEASON3B::CNewUIWindowMenu::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if( NULL == pNewUIMng )
		return false;
	
	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_WINDOW_MENU, this);
	
	SetPos(x, y);
	
	LoadImages();

	Show(false);

	return true;
}

void SEASON3B::CNewUIWindowMenu::Release()
{
	UnloadImages();
	
	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj( this );
		m_pNewUIMng = NULL;
	}
}

void SEASON3B::CNewUIWindowMenu::SetPos(int x, int y)
{
#ifdef PBG_ADD_GENSRANKING
	// 갯수에 의해 포지션을 잡도록 조절
	m_Pos.x = STANDARD_POS_X;
	m_Pos.y = STANDARD_POS_Y - (20*(MENU_MAX_INDEX-4));
#else //PBG_ADD_GENSRANKING
	m_Pos.x = x;
	m_Pos.y = y;
#endif //PBG_ADD_GENSRANKING
}

bool SEASON3B::CNewUIWindowMenu::UpdateMouseEvent()
{
	POINT pt = { m_Pos.x, m_Pos.y+20 };

	for(int i=0; i<
#ifdef PJH_ADD_MINIMAP
#ifdef PBG_ADD_GENSRANKING
		MENU_MAX_INDEX
#else //PBG_ADD_GENSRANKING
		5
#endif //PBG_ADD_GENSRANKING
#else
		4
#endif //PJH_ADD_MINIMAP
		; ++i)
	{
		if(CheckMouseIn(pt.x, pt.y, 112, 20) == true)
		{
			m_iSelectedIndex = i;
			break;
		}

		pt.y += 20.f;
	}

	if(m_iSelectedIndex > -1 && SEASON3B::IsRelease(VK_LBUTTON))
	{
		switch(m_iSelectedIndex)
		{
		case 0:	// 시스템 메뉴
			{
				g_pNewUISystem->Hide(SEASON3B::INTERFACE_WINDOW_MENU);
				SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CSystemMenuMsgBoxLayout));
				return false;
			}
			break;
		case 1:	// 도움말
			{
				if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_HELP))
				{
					g_pHelp->AutoUpdateIndex();
				}
				else
				{
					g_pNewUISystem->Show(SEASON3B::INTERFACE_HELP);		
				}
				return false;
			}
			break;
		case 2:	// 길드
			{
				g_pNewUISystem->Show(SEASON3B::INTERFACE_GUILDINFO);
				g_pNewUISystem->Hide(SEASON3B::INTERFACE_WINDOW_MENU);
				return false;
			}
			break;
		case 3:	// 지도
			{
				g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MOVEMAP);
				return false;
			}
			break;
#ifdef PJH_ADD_MINIMAP
		case 4:	// 지도
			{
				g_pNewUISystem->Hide(SEASON3B::INTERFACE_WINDOW_MENU);
				if(g_pNewUIMiniMap->m_bSuccess == false)
				{
					g_pNewUISystem->Hide(SEASON3B::INTERFACE_MINI_MAP);
				}
				else
					g_pNewUISystem->Toggle( SEASON3B::INTERFACE_MINI_MAP );
				return false;
			}
			break;
#endif //PJH_ADD_MINIMAP
#ifdef PBG_ADD_GENSRANKING
		case 5: // 겐스 정보창
			{
				if(g_pNewUIGensRanking->SetGensInfo())
				{
					g_pNewUISystem->Show(SEASON3B::INTERFACE_GENSRANKING);
					g_pNewUISystem->Hide(SEASON3B::INTERFACE_WINDOW_MENU);
				}
				return false;
			}
			break;
#endif //PBG_ADD_GENSRANKING
		}
	}

	if(CheckMouseIn(m_Pos.x, m_Pos.y+20, 112, 
#ifdef PBG_ADD_GENSRANKING
		95 + 20*(MENU_MAX_INDEX-5)
#else //PBG_ADD_GENSRANKING
		95
#endif //PBG_ADD_GENSRANKING
		) == false)
	{
		m_iSelectedIndex = -1;
	}
	else
	{
		return false;
	}
	
	return true;
}

bool SEASON3B::CNewUIWindowMenu::UpdateKeyEvent()
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_WINDOW_MENU) == true)
	{
		if(SEASON3B::IsPress(VK_ESCAPE) == true)
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_WINDOW_MENU);
			PlayBuffer(SOUND_CLICK01);

			return false;
		}
	}

	return true;
}

bool SEASON3B::CNewUIWindowMenu::Update()
{
	return true;
}

bool SEASON3B::CNewUIWindowMenu::Render()
{
	EnableAlphaTest();

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	RenderFrame();
	RenderTexts();
	RenderArrow();

	DisableAlphaBlend();

	return true;
}

void SEASON3B::CNewUIWindowMenu::RenderFrame()
{
	float x, y, width, height;
	
	x = m_Pos.x; y = m_Pos.y; width = 112.f; height = 
#ifdef PJH_ADD_MINIMAP
#ifdef PBG_ADD_GENSRANKING
		// 기본 + (총갯수-기본갯수)*15
		105 + (MENU_MAX_INDEX - 4)*20
#else //PBG_ADD_GENSRANKING
		120
#endif //PBG_ADD_GENSRANKING
#else
		105
#endif //PJH_ADD_MINIMAP
		;
	// 바탕
	RenderBitmap(IMAGE_WINDOW_MENU_BACK, x, y, width, height, 0.f, 0.f, width/256.f, height/512.f);

	// 상단
	y = m_Pos.y;
	RenderImage(IMAGE_WINDOW_MENU_FRAME_UP, m_Pos.x, y, 112.f, 45.f);
	y += 45.f;

#ifdef PBG_ADD_GENSRANKING
	// 총높이 - (상단+하단) / 중간이미지
	float Middle_TempCnt = (int)((height - 90)/15)+1; 
#endif //PBG_ADD_GENSRANKING
	// 중단
	for(int i=0; i<
#ifdef PJH_ADD_MINIMAP
#ifdef PBG_ADD_GENSRANKING
		Middle_TempCnt
#else //PBG_ADD_GENSRANKING
		2
#endif //PBG_ADD_GENSRANKING
#else
		1
#endif //PJH_ADD_MINIMAP
		; ++i)
	{
		RenderImage(IMAGE_WINDOW_MENU_FRAME_MIDDLE, m_Pos.x, y, 112.f, 15.f);
		y += 15.f;
	}
	// 하단
#ifdef PBG_ADD_GENSRANKING
	// 메인프레임 바로 위로 맞춘다
	y = 480 - 50 - 45;
#endif //PBG_ADD_GENSRANKING
	RenderImage(IMAGE_WINDOW_MENU_FRAME_DOWN, m_Pos.x, y, 112.f, 45.f);

	// 라인
#ifdef PBG_ADD_GENSRANKING
	y = m_Pos.y + 33.f;
#else //PBG_ADD_GENSRANKING
	y = m_Pos.y + 30.f;
#endif //PBG_ADD_GENSRANKING
#ifdef _VS2008PORTING
	for(int i=0; i<
#ifdef PJH_ADD_MINIMAP
#ifdef PBG_ADD_GENSRANKING
		// 총갯수-1
		MENU_MAX_INDEX-1
#else //PBG_ADD_GENSRANKING
		4
#endif //PBG_ADD_GENSRANKING
#else
		3
#endif	//#ifdef PJH_ADD_MINIMAP
		; ++i)
#else // _VS2008PORTING
	for(i=0; i<
#ifdef PJH_ADD_MINIMAP
#ifdef PBG_ADD_GENSRANKING
		// 총갯수-1
		MENU_MAX_INDEX-1
#else //PBG_ADD_GENSRANKING
		4
#endif //PBG_ADD_GENSRANKING
#else
		3
#endif	//#ifdef PJH_ADD_MINIMAP
		; ++i)
#endif // _VS2008PORTING
	{
		RenderImage(IMAGE_WINDOW_MENU_LINE, m_Pos.x + 15.f, y, 82.f, 2.f);
		y += 20.f;
	}
}

void SEASON3B::CNewUIWindowMenu::RenderTexts()
{
	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetBgColor(0);
	g_pRenderText->SetTextColor(255, 255, 255, 255);

	int iTextNumber[] = {1741, 1742, 364, 1743
#ifdef PJH_ADD_MINIMAP
		,3055
#endif //PJH_ADD_MINIMAP
#ifdef PBG_ADD_GENSRANKING
		//겐스정보(B)
		,3103
#endif //PBG_ADD_GENSRANKING
	};
	// 1741 "시스템(Esc)"
	// 1742 "도움말(F1)"
	// 364 "길드(G)"
	// 1743 "지도(M)"
	float y = m_Pos.y + 20.f;
	for(int i=0; i<
#ifdef PJH_ADD_MINIMAP
#ifdef PBG_ADD_GENSRANKING
		MENU_MAX_INDEX
#else //PBG_ADD_GENSRANKING
		5
#endif //PBG_ADD_GENSRANKING
#else
		4
#endif //PJH_ADD_MINIMAP
		; ++i)
	{
		if(m_iSelectedIndex == i)
		{
			g_pRenderText->SetTextColor(255, 255, 0, 255);
		}
		else
		{
			g_pRenderText->SetTextColor(255, 255, 255, 255);
		}
		g_pRenderText->RenderText(m_Pos.x, y, GlobalText[iTextNumber[i]], 112, 0, RT3_SORT_CENTER);
		y += 20.f;
	}
}

void SEASON3B::CNewUIWindowMenu::RenderArrow()
{
	if(m_iSelectedIndex < 0)
	{
		return;
	}

	float x, y;
	x = m_Pos.x + 16.f;
#ifdef PBG_ADD_GENSRANKING
	y = m_Pos.y + 20.f + (m_iSelectedIndex * 20);
#else //PBG_ADD_GENSRANKING
	y = m_Pos.y + 18.f + (m_iSelectedIndex * 20);
#endif //PBG_ADD_GENSRANKING
	RenderImage(IMAGE_WINDOW_MENU_ARROWL, x, y, 6.f, 9.f);
	x = m_Pos.x + 90.f;
	RenderImage(IMAGE_WINDOW_MENU_ARROWR, x, y, 6.f, 9.f);	
}

float SEASON3B::CNewUIWindowMenu::GetLayerDepth()
{
	return 10.4f;
}

float SEASON3B::CNewUIWindowMenu::GetKeyEventOrder()
{
	return 10.f;
}

void SEASON3B::CNewUIWindowMenu::OpenningProcess()
{
	m_iSelectedIndex = -1;
}

void SEASON3B::CNewUIWindowMenu::ClosingProcess()
{

}

void SEASON3B::CNewUIWindowMenu::LoadImages()
{
	LoadBitmap("Interface\\newui_msgbox_back.jpg", IMAGE_WINDOW_MENU_BACK, GL_LINEAR);
	LoadBitmap("Interface\\newui_commamd01.tga", IMAGE_WINDOW_MENU_FRAME_UP, GL_LINEAR);
	LoadBitmap("Interface\\newui_commamd02.tga", IMAGE_WINDOW_MENU_FRAME_MIDDLE, GL_LINEAR);
	LoadBitmap("Interface\\newui_commamd03.tga", IMAGE_WINDOW_MENU_FRAME_DOWN, GL_LINEAR);
	LoadBitmap("Interface\\newui_commamd_Line.jpg", IMAGE_WINDOW_MENU_LINE, GL_LINEAR );
	LoadBitmap("Interface\\newui_arrow(L).tga", IMAGE_WINDOW_MENU_ARROWL, GL_LINEAR );
	LoadBitmap("Interface\\newui_arrow(R).tga", IMAGE_WINDOW_MENU_ARROWR, GL_LINEAR );
}

void SEASON3B::CNewUIWindowMenu::UnloadImages()
{
	DeleteBitmap(IMAGE_WINDOW_MENU_BACK);
	DeleteBitmap(IMAGE_WINDOW_MENU_FRAME_UP);
	DeleteBitmap(IMAGE_WINDOW_MENU_FRAME_MIDDLE);
	DeleteBitmap(IMAGE_WINDOW_MENU_FRAME_DOWN);
	DeleteBitmap(IMAGE_WINDOW_MENU_LINE);
	DeleteBitmap(IMAGE_WINDOW_MENU_ARROWL);
	DeleteBitmap(IMAGE_WINDOW_MENU_ARROWR);
}
