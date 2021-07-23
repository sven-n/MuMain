// NewUIGateSwitchWindow.cpp: implementation of the CNewUIGateSwitchWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIGateSwitchWindow.h"
#include "NewUISystem.h"
#include "ZzzBMD.h"
#include "ZzzEffect.h"
#include "ZzzObject.h"
#include "ZzzInventory.h"
#include "ZzzInterface.h"
#include "ZzzInfomation.h"
#include "ZzzCharacter.h"
#include "wsclientinline.h"
#include "npcGateSwitch.h"

using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNewUIGateSwitchWindow::CNewUIGateSwitchWindow()
{
	m_pNewUIMng = NULL;
	m_Pos.x = m_Pos.y = 0;
}

CNewUIGateSwitchWindow::~CNewUIGateSwitchWindow()
{
	Release();
}

bool CNewUIGateSwitchWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if(NULL == pNewUIMng)
		return false;

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_GATESWITCH, this);

	SetPos(x, y);

	LoadImages();

	m_BtnExit.ChangeButtonImgState(true, IMAGE_GATESWITCHWINDOW_EXIT_BTN, false);
	m_BtnExit.ChangeButtonInfo(m_Pos.x+13, m_Pos.y+391, 36, 29);
	m_BtnExit.ChangeToolTipText(GlobalText[1002], true); // 225 "닫기(I,V)"

	InitButton(&m_BtnOpen, m_Pos.x + 41, m_Pos.y + 320, GlobalText[1479]);
	
	Show(false);

	return true;
}

void CNewUIGateSwitchWindow::InitButton(CNewUIButton * pNewUIButton, int iPos_x, int iPos_y, const unicode::t_char * pCaption)
{
	pNewUIButton->ChangeText(pCaption);
	pNewUIButton->ChangeTextBackColor(RGBA(255,255,255,0));
	pNewUIButton->ChangeButtonImgState( true, IMAGE_GATESWITCHWINDOW_BUTTON, true );
	pNewUIButton->ChangeButtonInfo(iPos_x, iPos_y, 108, 29);
	pNewUIButton->ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
	pNewUIButton->ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
}

void CNewUIGateSwitchWindow::Release()
{
	UnloadImages();

	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}
}

void CNewUIGateSwitchWindow::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
}

bool CNewUIGateSwitchWindow::UpdateMouseEvent()
{
	if(m_BtnOpen.UpdateMouseEvent() == true)
	{
		npcGateSwitch::SendToggleGate();
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_GATESWITCH);
	}

	//. 버튼 처리
	if(true == BtnProcess())	//. 처리가 완료 되었다면
		return false;

	//. 인벤토리 내의 영역 클릭시 하위 UI처리 및 이동 불가
	if(CheckMouseIn(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT))
		return false;

	return true;
}

bool CNewUIGateSwitchWindow::UpdateKeyEvent()
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_GATESWITCH) == true)
	{
		if(SEASON3B::IsPress(VK_ESCAPE) == true)
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_GATESWITCH);
			PlayBuffer(SOUND_CLICK01);
			return false;
		}
	}
	return true;
}

bool CNewUIGateSwitchWindow::Update()
{
	return true;
}

bool CNewUIGateSwitchWindow::Render()
{
	EnableAlphaTest();
	glColor4f(1.f, 1.f, 1.f, 1.f);
	
	RenderFrame();
	
	POINT ptOrigin = { m_Pos.x, m_Pos.y+50 };

	//  설명.
    g_pRenderText->RenderText(ptOrigin.x+95, ptOrigin.y, GlobalText[1476], 0, 0, RT3_WRITE_CENTER); ptOrigin.y += 17;
	g_pRenderText->RenderText(ptOrigin.x+95, ptOrigin.y, GlobalText[1477], 0, 0, RT3_WRITE_CENTER); ptOrigin.y += 17;

	g_pRenderText->SetBgColor(160, 0, 0, 255);
	g_pRenderText->RenderText( ptOrigin.x+95, ptOrigin.y, GlobalText[1478], 0, 0, RT3_WRITE_CENTER); ptOrigin.y += 17;
	g_pRenderText->SetBgColor(0);

	RenderOutlineUpper( m_Pos.x+0, m_Pos.y+120, 162, 159 );
	RenderOutlineLower( m_Pos.x+0, m_Pos.y+120, 162, 159 );
	if (npcGateSwitch::IsGateOpened())
	{
		RenderBitmap ( BITMAP_INTERFACE_EX+41, m_Pos.x+17.5f, m_Pos.y+120, 155, 168, 0.f, 0.f, 155/256.f, 168/256.f );
		m_BtnOpen.ChangeText(GlobalText[1002]);
	}
	else
	{
		RenderBitmap ( BITMAP_INTERFACE_EX+40, m_Pos.x+17.5f, m_Pos.y+120, 155, 168, 0.f, 0.f, 155/256.f, 168/256.f );
		m_BtnOpen.ChangeText(GlobalText[1479]);
	}

	// 성문 열기 버튼 표시
	m_BtnOpen.Render();

	// 닫기 버튼 표시
	m_BtnExit.Render();

	DisableAlphaBlend();

	return true;
}

void CNewUIGateSwitchWindow::OpeningProcess()
{
}

void CNewUIGateSwitchWindow::ClosingProcess()
{
	SendExitInventory();
}

float CNewUIGateSwitchWindow::GetLayerDepth()
{ return 5.0f; }

void CNewUIGateSwitchWindow::LoadImages()
{
	LoadBitmap("Interface\\newui_msgbox_back.jpg", IMAGE_GATESWITCHWINDOW_BACK, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back01.tga", IMAGE_GATESWITCHWINDOW_TOP, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-L.tga", IMAGE_GATESWITCHWINDOW_LEFT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-R.tga", IMAGE_GATESWITCHWINDOW_RIGHT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back03.tga", IMAGE_GATESWITCHWINDOW_BOTTOM, GL_LINEAR);
	LoadBitmap("Interface\\newui_exit_00.tga", IMAGE_GATESWITCHWINDOW_EXIT_BTN, GL_LINEAR);
	LoadBitmap("Interface\\newui_btn_empty.tga", IMAGE_GATESWITCHWINDOW_BUTTON, GL_LINEAR);
	
	LoadBitmap("Interface\\newui_item_table01(L).tga", IMAGE_GATESWITCHWINDOW_TABLE_TOP_LEFT);
	LoadBitmap("Interface\\newui_item_table01(R).tga", IMAGE_GATESWITCHWINDOW_TABLE_TOP_RIGHT);
	LoadBitmap("Interface\\newui_item_table02(L).tga", IMAGE_GATESWITCHWINDOW_TABLE_BOTTOM_LEFT);
	LoadBitmap("Interface\\newui_item_table02(R).tga", IMAGE_GATESWITCHWINDOW_TABLE_BOTTOM_RIGHT);
	LoadBitmap("Interface\\newui_item_table03(Up).tga", IMAGE_GATESWITCHWINDOW_TABLE_TOP_PIXEL);
	LoadBitmap("Interface\\newui_item_table03(Dw).tga", IMAGE_GATESWITCHWINDOW_TABLE_BOTTOM_PIXEL);
	LoadBitmap("Interface\\newui_item_table03(L).tga", IMAGE_GATESWITCHWINDOW_TABLE_LEFT_PIXEL);
	LoadBitmap("Interface\\newui_item_table03(R).tga", IMAGE_GATESWITCHWINDOW_TABLE_RIGHT_PIXEL);
}
void CNewUIGateSwitchWindow::UnloadImages()
{
	DeleteBitmap(IMAGE_GATESWITCHWINDOW_BOTTOM);
	DeleteBitmap(IMAGE_GATESWITCHWINDOW_RIGHT);
	DeleteBitmap(IMAGE_GATESWITCHWINDOW_LEFT);
	DeleteBitmap(IMAGE_GATESWITCHWINDOW_TOP);
	DeleteBitmap(IMAGE_GATESWITCHWINDOW_BACK);
	DeleteBitmap(IMAGE_GATESWITCHWINDOW_EXIT_BTN);
	DeleteBitmap(IMAGE_GATESWITCHWINDOW_BUTTON);
	DeleteBitmap(IMAGE_GATESWITCHWINDOW_TABLE_RIGHT_PIXEL);
	DeleteBitmap(IMAGE_GATESWITCHWINDOW_TABLE_LEFT_PIXEL);
	DeleteBitmap(IMAGE_GATESWITCHWINDOW_TABLE_BOTTOM_PIXEL);
	DeleteBitmap(IMAGE_GATESWITCHWINDOW_TABLE_TOP_PIXEL);
	DeleteBitmap(IMAGE_GATESWITCHWINDOW_TABLE_BOTTOM_RIGHT);
	DeleteBitmap(IMAGE_GATESWITCHWINDOW_TABLE_BOTTOM_LEFT);
	DeleteBitmap(IMAGE_GATESWITCHWINDOW_TABLE_TOP_RIGHT);
	DeleteBitmap(IMAGE_GATESWITCHWINDOW_TABLE_TOP_LEFT);
}

void CNewUIGateSwitchWindow::RenderFrame()
{
	RenderImage(IMAGE_GATESWITCHWINDOW_BACK, m_Pos.x, m_Pos.y, 190.f, 429.f);
	RenderImage(IMAGE_GATESWITCHWINDOW_TOP, m_Pos.x, m_Pos.y, 190.f, 64.f);
	RenderImage(IMAGE_GATESWITCHWINDOW_LEFT, m_Pos.x, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_GATESWITCHWINDOW_RIGHT, m_Pos.x+INVENTORY_WIDTH-21, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_GATESWITCHWINDOW_BOTTOM, m_Pos.x, m_Pos.y+INVENTORY_HEIGHT-45, 190.f, 45.f);

	unicode::t_char szText[256] = { 0, };
	float fPos_x = m_Pos.x + 15.0f, fPos_y = m_Pos.y;
	float fLine_y = 13.0f;

	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetTextColor(220, 220, 220, 255);
	g_pRenderText->SetBgColor(0, 0, 0, 0);

	// 창 제목 표시
	unicode::_sprintf(szText, "%s", GlobalText[1475]);
	g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y, szText, 160.0f, 0, RT3_SORT_CENTER);
}

bool CNewUIGateSwitchWindow::BtnProcess()
{
	POINT ptExitBtn1 = { m_Pos.x+169, m_Pos.y+7 };
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	POINT ptExitBtn2 = { m_Pos.x+13, m_Pos.y+391 };
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX

	//. Exit1 버튼 (기본처리)
	if(SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_GATESWITCH);
	}

	// 닫기 버튼
	if(m_BtnExit.UpdateMouseEvent() == true)
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_GATESWITCH);
		return true;
	}

	return false;
}

void CNewUIGateSwitchWindow::RenderOutlineUpper(float fPos_x, float fPos_y, float fWidth, float fHeight)
{
	POINT ptOrigin = { fPos_x, fPos_y };
	float fBoxWidth = fWidth;
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	float fBoxHeight = fHeight;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX

	// 위쪽 테두리
	RenderImage(IMAGE_GATESWITCHWINDOW_TABLE_TOP_LEFT, ptOrigin.x+12, ptOrigin.y-4, 14, 14);
	RenderImage(IMAGE_GATESWITCHWINDOW_TABLE_TOP_RIGHT, ptOrigin.x+fBoxWidth+4, ptOrigin.y-4, 14, 14);
	// 위쪽 라인
	RenderImage(IMAGE_GATESWITCHWINDOW_TABLE_TOP_PIXEL, ptOrigin.x+25, ptOrigin.y-4, fBoxWidth-21, 14);
	//glColor4f(0.0f, 0.0f, 0.0f, 0.3f);
	//RenderColor(ptOrigin.x+15, ptOrigin.y-3, fBoxWidth-2, 15);
	//EndRenderColor();
}

void CNewUIGateSwitchWindow::RenderOutlineLower(float fPos_x, float fPos_y, float fWidth, float fHeight)
{
	POINT ptOrigin = { fPos_x, fPos_y };
	float fBoxWidth = fWidth;
	float fBoxHeight = fHeight;

	// 왼쪽 라인, 오른쪽 라인
	RenderImage(IMAGE_GATESWITCHWINDOW_TABLE_LEFT_PIXEL, ptOrigin.x+12, ptOrigin.y+9, 14, fBoxHeight);
	RenderImage(IMAGE_GATESWITCHWINDOW_TABLE_RIGHT_PIXEL, ptOrigin.x+fBoxWidth+4, ptOrigin.y+9, 14, fBoxHeight);
	// 중간라인
	//RenderImage(IMAGE_GATESWITCHWINDOW_TABLE_BOTTOM_PIXEL, ptOrigin.x+15, ptOrigin.y+3, fBoxWidth-2, 14);
	// 아래쪽 테두리
	RenderImage(IMAGE_GATESWITCHWINDOW_TABLE_BOTTOM_LEFT, ptOrigin.x+12, ptOrigin.y+fBoxHeight+3, 14, 14);
	RenderImage(IMAGE_GATESWITCHWINDOW_TABLE_BOTTOM_RIGHT, ptOrigin.x+fBoxWidth+4, ptOrigin.y+fBoxHeight+3, 14, 14);
	// 아래쪽 라인
	RenderImage(IMAGE_GATESWITCHWINDOW_TABLE_BOTTOM_PIXEL, ptOrigin.x+25, ptOrigin.y+fBoxHeight+3, fBoxWidth-21, 14);
}
