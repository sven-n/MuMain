// NewUIDuelWatchUserListWindow.cpp: implementation of the CNewUIDuelWatchUserListWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIDuelWatchUserListWindow.h"
#include "NewUISystem.h"
#include "DuelMgr.h"

#ifdef YDG_ADD_NEW_DUEL_WATCH_BUFF

using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNewUIDuelWatchUserListWindow::CNewUIDuelWatchUserListWindow()
{

}

CNewUIDuelWatchUserListWindow::~CNewUIDuelWatchUserListWindow()
{
#ifdef LDS_FIX_MEMORYLEAK_WHERE_NEWUI_DEINITIALIZE
	Release();
#endif // LDS_FIX_MEMORYLEAK_WHERE_NEWUI_DEINITIALIZE
}

bool CNewUIDuelWatchUserListWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if(NULL == pNewUIMng)
		return false;

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_DUELWATCH_USERLIST, this);

	SetPos(x, y);

	LoadImages();

	Show(false);

	return true;
}

void CNewUIDuelWatchUserListWindow::Release()
{
	UnloadImages();

	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}
}

void CNewUIDuelWatchUserListWindow::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
}

bool CNewUIDuelWatchUserListWindow::UpdateMouseEvent()
{
	// 버튼 처리
	if(true == BtnProcess())	//. 처리가 완료 되었다면
		return false;

	POINT ptSize = { 57, 17 };
	POINT ptOrigin = { m_Pos.x, m_Pos.y - (ptSize.y + 1) * g_DuelMgr.GetDuelWatchUserCount() };

	// 인벤토리 내의 영역 클릭시 하위 UI처리 및 이동 불가
	if(CheckMouseIn(ptOrigin.x, ptOrigin.y, ptSize.x, (ptSize.y + 1) * g_DuelMgr.GetDuelWatchUserCount() + 10))
		return false;

	return true;
}

bool CNewUIDuelWatchUserListWindow::UpdateKeyEvent()
{
	return true;
}


bool CNewUIDuelWatchUserListWindow::Update()
{
// 	if(IsVisible())
// 	{
// 	}
	return true;
}

bool CNewUIDuelWatchUserListWindow::Render()
{
	EnableAlphaTest();
	glColor4f(1.f, 1.f, 1.f, 1.f);
	
	RenderFrame();
	
	extern float g_fScreenRate_y;

	SIZE TextSize;
#ifdef LJH_FIX_CHANING_FONT_FOR_DUAL_WATCHUSER_NAME
	g_pRenderText->SetFont(g_hFontBold);
#endif //LJH_FIX_CHANING_FONT_FOR_DUAL_WATCHUSER_NAME
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), "Q", 1, &TextSize);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	unicode::_GetTextExtentPoint(g_pRenderText->GetFontDC(), "Q", 1, &TextSize);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	float fFontHeight = TextSize.cy / g_fScreenRate_y;;		// 현재 해상도 기준으로 사용

	POINT ptSize = { 57, 17 };
	POINT ptOrigin = { m_Pos.x, m_Pos.y - (ptSize.y + 1) * g_DuelMgr.GetDuelWatchUserCount() + (ptSize.y - fFontHeight) / 2 + 1 };

	for (int i = 0; i < g_DuelMgr.GetDuelWatchUserCount(); ++i)
	{
		g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, g_DuelMgr.GetDuelWatchUser(i), ptSize.x, 0, RT3_SORT_CENTER);
		ptOrigin.y += ptSize.y + 1;
	}

	DisableAlphaBlend();

	return true;
}

void CNewUIDuelWatchUserListWindow::OpeningProcess()
{
}

void CNewUIDuelWatchUserListWindow::ClosingProcess()
{
}

float CNewUIDuelWatchUserListWindow::GetLayerDepth()
{ return 5.0f; }

void CNewUIDuelWatchUserListWindow::LoadImages()
{
	LoadBitmap("Interface\\Pk_box.tga", IMAGE_DUELWATCH_USERLIST_BOX, GL_LINEAR);
}

void CNewUIDuelWatchUserListWindow::UnloadImages()
{
	DeleteBitmap(IMAGE_DUELWATCH_USERLIST_BOX);
}

void CNewUIDuelWatchUserListWindow::RenderFrame()
{
	POINT ptSize = { 57, 17 };

	int i;
	glColor4f(0.f, 0.f, 0.f, 0.8f);
	for (i = 0; i < g_DuelMgr.GetDuelWatchUserCount(); ++i)
	{
		RenderColor(m_Pos.x, m_Pos.y - (ptSize.y + 1) * (i + 1), ptSize.x, ptSize.y);
	}
	EndRenderColor();
	
	for (i = 0; i < g_DuelMgr.GetDuelWatchUserCount(); ++i)
	{
		RenderImage(IMAGE_DUELWATCH_USERLIST_BOX, m_Pos.x, m_Pos.y - (ptSize.y + 1) * (i + 1), ptSize.x, ptSize.y);
	}
}

bool CNewUIDuelWatchUserListWindow::BtnProcess()
{
	return false;
}
#endif	// YDG_ADD_NEW_DUEL_WATCH_BUFF
