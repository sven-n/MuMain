// NewUIQuickCommandWindow.cpp: implementation of the CNewUIQuickCommandWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIQuickCommandWindow.h"
#include "NewUISystem.h"
#include "DSPlaySound.h"

using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SEASON3B::CNewUIQuickCommandWindow::CNewUIQuickCommandWindow()
{
	m_pNewUIMng = NULL;
	m_Pos.x = 0;
	m_Pos.y = 0;

	m_iSelectedIndex = -1;
	m_iSelectedCharacterIndex = -1;
}

SEASON3B::CNewUIQuickCommandWindow::~CNewUIQuickCommandWindow()
{
	Release();
}

bool SEASON3B::CNewUIQuickCommandWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if( NULL == pNewUIMng )
		return false;
	
	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_QUICK_COMMAND, this);

	LoadImages();
	
	SetPos(x, y);

	Show(false);

	return true;
}

void SEASON3B::CNewUIQuickCommandWindow::Release()
{
	UnloadImages();

	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj( this );
		m_pNewUIMng = NULL;
	}
}

void SEASON3B::CNewUIQuickCommandWindow::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
}

bool SEASON3B::CNewUIQuickCommandWindow::UpdateMouseEvent()
{
	if(m_iSelectedCharacterIndex < 0)
	{
		return true;
	}

	POINT pt = { m_Pos.x, m_Pos.y+38 };

#ifdef YDG_ADD_NEW_DUEL_UI
	for(int i = 0; i < 5; ++i)
#else	// YDG_ADD_NEW_DUEL_UI
	for(int i=0; i<4; ++i)
#endif	// YDG_ADD_NEW_DUEL_UI
	{
		if(CheckMouseIn(pt.x, pt.y, 112, 19) == true)
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
		case 0:		// 거래
			{
				CHARACTER* pCha = &CharactersClient[m_iSelectedCharacterIndex];
				g_pCommandWindow->CommandTrade(pCha);
				CloseQuickCommand();

				return false;
			}
			break;
		case 1:		// 구매
			{
				CHARACTER* pCha = &CharactersClient[m_iSelectedCharacterIndex];
				g_pCommandWindow->CommandPurchase(pCha);
				CloseQuickCommand();

				return false;
			}
			break;
		case 2:		// 파티
			{
				CHARACTER* pCha = &CharactersClient[m_iSelectedCharacterIndex];
				g_pCommandWindow->CommandParty(pCha->Key);
				CloseQuickCommand();
				
				return false;
			}
			break;
		case 3:		// 따라가기
			{
				g_pCommandWindow->CommandFollow(m_iSelectedCharacterIndex);
				CloseQuickCommand();

				return false;
			}
			break;
#ifdef YDG_ADD_NEW_DUEL_UI
		case 4:		// 결투
			{
				CHARACTER* pCha = &CharactersClient[m_iSelectedCharacterIndex];
				g_pCommandWindow->CommandDual(pCha);
				CloseQuickCommand();

				return false;
			}
			break;
#endif	// YDG_ADD_NEW_DUEL_UI
		}
	}

#ifdef YDG_ADD_NEW_DUEL_UI
	if(CheckMouseIn(m_Pos.x, m_Pos.y+30, 112, 110) == false)
#else	// YDG_ADD_NEW_DUEL_UI
	if(CheckMouseIn(m_Pos.x, m_Pos.y+30, 112, 90) == false)
#endif	// YDG_ADD_NEW_DUEL_UI
	{
		m_iSelectedIndex = -1;

		if(SEASON3B::IsRelease(VK_LBUTTON))
		{
			CloseQuickCommand();
			return false;
		}
	}

#ifdef YDG_ADD_NEW_DUEL_UI
	if(CheckMouseIn(m_Pos.x, m_Pos.y, 112, 140))
#else	// YDG_ADD_NEW_DUEL_UI
	if(CheckMouseIn(m_Pos.x, m_Pos.y, 112, 120))
#endif	// YDG_ADD_NEW_DUEL_UI
	{
		return false;
	}

	return true;
}

bool SEASON3B::CNewUIQuickCommandWindow::UpdateKeyEvent()
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUICK_COMMAND) == true)
	{
		if(SEASON3B::IsPress(VK_ESCAPE) == true)
		{
			CloseQuickCommand();
			PlayBuffer(SOUND_CLICK01);

			return false;
		}
	}

	return true;
}

bool SEASON3B::CNewUIQuickCommandWindow::Update()
{
	if(m_iSelectedCharacterIndex >= 0)
	{
		CHARACTER* pCha = &CharactersClient[m_iSelectedCharacterIndex];	
		
		// 대소문자 구분함 
		// 만약 아이디가 틀려진 경우 사라지게 예외처리
		// 죽었거나, 플레이어가 아니면
		if(strcmp(pCha->ID, m_strID) != 0	
			|| pCha->Object.Live == false
			|| pCha->Object.Kind != KIND_PLAYER)
		{
			CloseQuickCommand();
		}

		// 거리 계산해서 3타일 이상되면 사라지게 처리
		float fPos_x = pCha->Object.Position[0] - Hero->Object.Position[0];
		float fPos_y = pCha->Object.Position[1] - Hero->Object.Position[1];
		float fDistance = sqrtf((fPos_x * fPos_x) + (fPos_y * fPos_y));

		if(fDistance > 300.f)
		{
			CloseQuickCommand();
		}
	}
	else
	{
		CloseQuickCommand();
	}
	
	return true;
}

bool SEASON3B::CNewUIQuickCommandWindow::Render()
{
	EnableAlphaTest();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	if(m_iSelectedCharacterIndex < 0)
	{
		return true;
	}

	RenderFrame();
	RenderContents();
	RenderArrow();
	
	DisableAlphaBlend();

	return true;
}

void SEASON3B::CNewUIQuickCommandWindow::RenderFrame()
{
	float x, y, width, height;
	
#ifdef YDG_ADD_NEW_DUEL_UI
	x = m_Pos.x; y = m_Pos.y; width = 112.f; height = 140;
#else	// YDG_ADD_NEW_DUEL_UI
	x = m_Pos.x; y = m_Pos.y; width = 112.f; height = 120;
#endif	// YDG_ADD_NEW_DUEL_UI
	// 바탕
	RenderImage(IMAGE_QUICKCOMMAND_BACK, x, y, width, height);

	// 상단
	y = m_Pos.y;
	RenderImage(IMAGE_QUICKCOMMAND_FRAME_UP, m_Pos.x, y, 112.f, 45.f);
	y += 45.f;

	// 중단
#ifdef YDG_ADD_NEW_DUEL_UI
	for(int i = 0; i < 3; ++i)
#else	// YDG_ADD_NEW_DUEL_UI
	for(int i=0; i<2; ++i)
#endif	// YDG_ADD_NEW_DUEL_UI
	{
		RenderImage(IMAGE_QUICKCOMMAND_FRAME_MIDDLE, m_Pos.x, y, 112.f, 15.f);
		y += 15.f;
	}
#ifdef YDG_ADD_NEW_DUEL_UI
	// 15의 배수로 크기를 늘리는게 적절하지 않아 어중간한 길이를 맞춤 -> 추후 더 추가되면 조정 필요;
	RenderImage(IMAGE_QUICKCOMMAND_FRAME_MIDDLE, m_Pos.x, y, 112.f, 5.f);
	y += 5.f;
#endif	// YDG_ADD_NEW_DUEL_UI

	// 하단
	RenderImage(IMAGE_QUICKCOMMAND_FRAME_DOWN, m_Pos.x, y, 112.f, 45.f);

	// 라인
	y = m_Pos.y + 55.f;
#ifdef YDG_ADD_NEW_DUEL_UI
#ifdef _VS2008PORTING
	for(int i = 0; i < 4; ++i)
#else // _VS2008PORTING
	for(i = 0; i < 4; ++i)
#endif // _VS2008PORTING
#else	// YDG_ADD_NEW_DUEL_UI
#ifdef _VS2008PORTING
	for(int i=0; i<3; ++i)
#else // _VS2008PORTING
	for(i=0; i<3; ++i)
#endif // _VS2008PORTING
#endif	// YDG_ADD_NEW_DUEL_UI
	{
		RenderImage(IMAGE_QUICKCOMMAND_LINE, m_Pos.x + 15.f, y, 82.f, 2.f);
		y += 19.f;
	}
}

void SEASON3B::CNewUIQuickCommandWindow::RenderContents()
{
	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetTextColor(0, 255, 0, 255);
	g_pRenderText->SetBgColor(0);

	// ID부분
	int y = m_Pos.y + 14;
	g_pRenderText->RenderText(m_Pos.x, y, m_strID, 112, 0, RT3_SORT_CENTER);
	y += 30;

	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(255, 255, 255, 255);

	// 944 "파티"
	// 943 "거래"
	// 1124 "구매"
	// 948 "따라가기"
#ifdef YDG_ADD_NEW_DUEL_UI
	int iGlobalText[] = { 943, 1124, 944, 948, 949 };
	for(int i = 0; i < 5; ++i)
#else	// YDG_ADD_NEW_DUEL_UI
	int iGlobalText[4] = { 943, 1124, 944, 948 };
	for(int i=0; i<4; ++i)
#endif	// YDG_ADD_NEW_DUEL_UI
	{
		if(m_iSelectedIndex == i)
		{
			g_pRenderText->SetTextColor(255, 255, 0, 255);
		}
		else
		{
			g_pRenderText->SetTextColor(255, 255, 255, 255);
		}
		g_pRenderText->RenderText(m_Pos.x, y, GlobalText[iGlobalText[i]], 112, 0, RT3_SORT_CENTER);
		y += 19.f;
	}
}

void SEASON3B::CNewUIQuickCommandWindow::RenderArrow()
{
	if(m_iSelectedIndex < 0)
	{
		return;
	}

	float x, y;
	x = m_Pos.x + 16.f;
	y = m_Pos.y + 43.f + (m_iSelectedIndex * 19);
	RenderImage(IMAGE_QUICKCOMMAND_ARROWL, x, y, 6.f, 9.f);
	x = m_Pos.x + 90.f;
	RenderImage(IMAGE_QUICKCOMMAND_ARROWR, x, y, 6.f, 9.f);	
}

float SEASON3B::CNewUIQuickCommandWindow::GetLayerDepth()
{
	return 2.0f;
}

float SEASON3B::CNewUIQuickCommandWindow::GetKeyEventOrder()
{
	return 10.f;
}

void SEASON3B::CNewUIQuickCommandWindow::OpenningProcess()
{
	m_iSelectedIndex = -1;
	m_iSelectedCharacterIndex = -1;
}

void SEASON3B::CNewUIQuickCommandWindow::ClosingProcess()
{
	m_iSelectedIndex = -1;
	m_iSelectedCharacterIndex = -1;
}

void SEASON3B::CNewUIQuickCommandWindow::LoadImages()
{
	LoadBitmap("Interface\\newui_msgbox_back.jpg", IMAGE_QUICKCOMMAND_BACK, GL_LINEAR);
	LoadBitmap("Interface\\newui_commamd04.tga", IMAGE_QUICKCOMMAND_FRAME_UP, GL_LINEAR);
	LoadBitmap("Interface\\newui_commamd02.tga", IMAGE_QUICKCOMMAND_FRAME_MIDDLE, GL_LINEAR);
	LoadBitmap("Interface\\newui_commamd03.tga", IMAGE_QUICKCOMMAND_FRAME_DOWN, GL_LINEAR);
	LoadBitmap("Interface\\newui_commamd_Line.jpg", IMAGE_QUICKCOMMAND_LINE, GL_LINEAR );
	LoadBitmap("Interface\\newui_arrow(L).tga", IMAGE_QUICKCOMMAND_ARROWL, GL_LINEAR );
	LoadBitmap("Interface\\newui_arrow(R).tga", IMAGE_QUICKCOMMAND_ARROWR, GL_LINEAR );
}

void SEASON3B::CNewUIQuickCommandWindow::UnloadImages()
{
	DeleteBitmap(IMAGE_QUICKCOMMAND_BACK);
	DeleteBitmap(IMAGE_QUICKCOMMAND_FRAME_UP);
	DeleteBitmap(IMAGE_QUICKCOMMAND_FRAME_MIDDLE);
	DeleteBitmap(IMAGE_QUICKCOMMAND_FRAME_DOWN);
	DeleteBitmap(IMAGE_QUICKCOMMAND_LINE);
	DeleteBitmap(IMAGE_QUICKCOMMAND_ARROWL);
	DeleteBitmap(IMAGE_QUICKCOMMAND_ARROWR);
}

void SEASON3B::CNewUIQuickCommandWindow::OpenQuickCommand(const char* strID, int iIndex, int x, int y)
{
	g_pNewUISystem->Show(SEASON3B::INTERFACE_QUICK_COMMAND);

	SetID(strID);
	SetSelectedCharacterIndex(iIndex);
	SetPos(x, y);
}

void SEASON3B::CNewUIQuickCommandWindow::CloseQuickCommand()
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUICK_COMMAND) == true)
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_QUICK_COMMAND);
	}
}

void SEASON3B::CNewUIQuickCommandWindow::SetID(const char* strID)
{
	strcpy(m_strID, strID);
}

void SEASON3B::CNewUIQuickCommandWindow::SetSelectedCharacterIndex(int iIndex)
{
	m_iSelectedCharacterIndex = iIndex;
}
