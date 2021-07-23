// NewGatemanWindow.cpp: implementation of the CNewGatemanWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIGatemanWindow.h"
#include "NewUISystem.h"
#include "NewUICustomMessageBox.h"
#include "ZzzBMD.h"
#include "ZzzEffect.h"
#include "ZzzObject.h"
#include "ZzzInventory.h"
#include "ZzzInterface.h"
#include "ZzzInfomation.h"
#include "ZzzCharacter.h"
#include "wsclientinline.h"
#include "DSPlaySound.h"
#include "UIGateKeeper.h"

extern CUIGateKeeper*		g_pUIGateKeeper;

using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNewUIGatemanWindow::CNewUIGatemanWindow()
{
	m_pNewUIMng = NULL;
	m_Pos.x = m_Pos.y = 0;
}

CNewUIGatemanWindow::~CNewUIGatemanWindow()
{
	Release();
}

bool CNewUIGatemanWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if(NULL == pNewUIMng)
		return false;

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_GATEKEEPER, this);

	SetPos(x, y);

	LoadImages();

	m_BtnExit.ChangeButtonImgState(true, IMAGE_GATEMANWINDOW_EXIT_BTN, false);
	m_BtnExit.ChangeButtonInfo(m_Pos.x+13, m_Pos.y+391, 36, 29);
	m_BtnExit.ChangeToolTipText(GlobalText[1002], true); // 225 "닫기(I,V)"

	InitButton(&m_BtnEnter, m_Pos.x + INVENTORY_WIDTH / 2 - 27, m_Pos.y + 320, GlobalText[1593]);
	InitButton(&m_BtnSet, m_Pos.x + INVENTORY_WIDTH / 2 - 27, m_Pos.y + 220, GlobalText[1619]);

	m_BtnFeeUp.ChangeButtonImgState( true, IMAGE_GATEMANWINDOW_SCROLL_UP_BTN, true );
	m_BtnFeeDn.ChangeButtonImgState( true, IMAGE_GATEMANWINDOW_SCROLL_DOWN_BTN, true );
	m_BtnFeeUp.ChangeButtonInfo(m_Pos.x+118, m_Pos.y+180, 15, 13);
	m_BtnFeeDn.ChangeButtonInfo(m_Pos.x+118, m_Pos.y+196, 15, 13);

	Show(false);

	return true;
}

void CNewUIGatemanWindow::InitButton(CNewUIButton * pNewUIButton, int iPos_x, int iPos_y, const unicode::t_char * pCaption)
{
	pNewUIButton->ChangeText(pCaption);
	pNewUIButton->ChangeTextBackColor(RGBA(255,255,255,0));
	pNewUIButton->ChangeButtonImgState( true, IMAGE_GATEMANWINDOW_BUTTON, true );
	pNewUIButton->ChangeButtonInfo(iPos_x, iPos_y, 53, 23);
	pNewUIButton->ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
	pNewUIButton->ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
}

void CNewUIGatemanWindow::Release()
{
	UnloadImages();

	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}
}

void CNewUIGatemanWindow::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
}

bool CNewUIGatemanWindow::UpdateMouseEvent()
{
	switch(g_pUIGateKeeper->GetType())
	{
	case TOUCH_TYPE_PERSON:
		UpdateGuestMode();
		break;
	case TOUCH_TYPE_GUILD_STAFF:
		UpdateGuildMemeberMode();
		break;
	case TOUCH_TYPE_GUILD_MASTER:
		UpdateGuildMasterMode();
		break;
	}

	//. 버튼 처리
	if(true == BtnProcess())	//. 처리가 완료 되었다면
		return false;

	//. 인벤토리 내의 영역 클릭시 하위 UI처리 및 이동 불가
	if(CheckMouseIn(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT))
		return false;

	return true;
}

bool CNewUIGatemanWindow::UpdateKeyEvent()
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_GATEKEEPER) == true)
	{
		if(SEASON3B::IsPress(VK_ESCAPE) == true)
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_GATEKEEPER);
			PlayBuffer(SOUND_CLICK01);
			return false;
		}
	}
	return true;
}


bool CNewUIGatemanWindow::Update()
{
	if(IsVisible())
	{
	}
	return true;
}
bool CNewUIGatemanWindow::Render()
{
	EnableAlphaTest();
	glColor4f(1.f, 1.f, 1.f, 1.f);
	
	RenderFrame();
	
	switch(g_pUIGateKeeper->GetType())
	{
	case TOUCH_TYPE_PERSON:
		RenderGuestMode();
		break;
	case TOUCH_TYPE_GUILD_STAFF:
		RenderGuildMemeberMode();
		break;
	case TOUCH_TYPE_GUILD_MASTER:
		RenderGuildMasterMode();
		break;
	}

	// 닫기 버튼 표시
	m_BtnExit.Render();

	DisableAlphaBlend();

	return true;
}

void CNewUIGatemanWindow::OpeningProcess()
{
}

void CNewUIGatemanWindow::ClosingProcess()
{
	SendExitInventory();
}

float CNewUIGatemanWindow::GetLayerDepth()
{ return 5.0f; }

void CNewUIGatemanWindow::LoadImages()
{
	LoadBitmap("Interface\\newui_msgbox_back.jpg", IMAGE_GATEMANWINDOW_BACK, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back01.tga", IMAGE_GATEMANWINDOW_TOP, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-L.tga", IMAGE_GATEMANWINDOW_LEFT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-R.tga", IMAGE_GATEMANWINDOW_RIGHT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back03.tga", IMAGE_GATEMANWINDOW_BOTTOM, GL_LINEAR);
	LoadBitmap("Interface\\newui_exit_00.tga", IMAGE_GATEMANWINDOW_EXIT_BTN, GL_LINEAR);
	LoadBitmap("Interface\\newui_btn_empty_very_small.tga", IMAGE_GATEMANWINDOW_BUTTON, GL_LINEAR);
	LoadBitmap("Interface\\newui_Bt_scroll_up.jpg", IMAGE_GATEMANWINDOW_SCROLL_UP_BTN, GL_LINEAR);
	LoadBitmap("Interface\\newui_Bt_scroll_dn.jpg", IMAGE_GATEMANWINDOW_SCROLL_DOWN_BTN, GL_LINEAR);
}
void CNewUIGatemanWindow::UnloadImages()
{
	DeleteBitmap(IMAGE_GATEMANWINDOW_BOTTOM);
	DeleteBitmap(IMAGE_GATEMANWINDOW_RIGHT);
	DeleteBitmap(IMAGE_GATEMANWINDOW_LEFT);
	DeleteBitmap(IMAGE_GATEMANWINDOW_TOP);
	DeleteBitmap(IMAGE_GATEMANWINDOW_BACK);
	DeleteBitmap(IMAGE_GATEMANWINDOW_EXIT_BTN);
	DeleteBitmap(IMAGE_GATEMANWINDOW_BUTTON);
	DeleteBitmap(IMAGE_GATEMANWINDOW_SCROLL_UP_BTN);
	DeleteBitmap(IMAGE_GATEMANWINDOW_SCROLL_DOWN_BTN);
}

void CNewUIGatemanWindow::RenderFrame()
{
	RenderImage(IMAGE_GATEMANWINDOW_BACK, m_Pos.x, m_Pos.y, 190.f, 429.f);
	RenderImage(IMAGE_GATEMANWINDOW_TOP, m_Pos.x, m_Pos.y, 190.f, 64.f);
	RenderImage(IMAGE_GATEMANWINDOW_LEFT, m_Pos.x, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_GATEMANWINDOW_RIGHT, m_Pos.x+INVENTORY_WIDTH-21, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_GATEMANWINDOW_BOTTOM, m_Pos.x, m_Pos.y+INVENTORY_HEIGHT-45, 190.f, 45.f);

	unicode::t_char szText[256] = { 0, };
	float fPos_x = m_Pos.x + 15.0f, fPos_y = m_Pos.y;
	float fLine_y = 13.0f;

	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetTextColor(220, 220, 220, 255);
	g_pRenderText->SetBgColor(0, 0, 0, 0);

	// 창 제목 표시
	unicode::_sprintf(szText, "%s", GlobalText[1596]);
	g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y, szText, 160.0f, 0, RT3_SORT_CENTER);
}

bool CNewUIGatemanWindow::BtnProcess()
{
	POINT ptExitBtn1 = { m_Pos.x+169, m_Pos.y+7 };
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	POINT ptExitBtn2 = { m_Pos.x+13, m_Pos.y+391 };
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX

	//. Exit1 버튼 (기본처리)
	if(SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_GATEKEEPER);
	}

	// 닫기 버튼
	if(m_BtnExit.UpdateMouseEvent() == true)
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_GATEKEEPER);
		return true;
	}

	return false;
}

void CNewUIGatemanWindow::UpdateGuildMasterMode()
{
	POINT ptOrigin = { m_Pos.x, m_Pos.y+50 };
	
	if(SEASON3B::IsPress(VK_LBUTTON) && SEASON3B::CheckMouseIn( ptOrigin.x+35, ptOrigin.y+60, 100, 16 ))
	{
		g_pUIGateKeeper->SendPublicSetting();
	}
	else if(m_BtnSet.UpdateMouseEvent() == true)
	{
		g_pUIGateKeeper->SendEnteranceFee();
	}
	else if(m_BtnFeeUp.UpdateMouseEvent() == true)
	{
		g_pUIGateKeeper->EnteranceFeeUp();
	}
	else if(m_BtnFeeDn.UpdateMouseEvent() == true)
	{
		g_pUIGateKeeper->EnteranceFeeDown();
	}
	else if(m_BtnEnter.UpdateMouseEvent() == true)
	{
		g_pUIGateKeeper->SendEnter();
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_GATEKEEPER);
	}
}

void CNewUIGatemanWindow::UpdateGuildMemeberMode()
{
	if(m_BtnEnter.UpdateMouseEvent() == true)
	{
		g_pUIGateKeeper->SendEnter();
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_GATEKEEPER);
	}
}

void CNewUIGatemanWindow::UpdateGuestMode()
{
	if(m_BtnEnter.UpdateMouseEvent() == true)
	{
#ifdef _VS2008PORTING
        if ( g_pUIGateKeeper->IsPublic() && (int)CharacterMachine->Gold >= g_pUIGateKeeper->GetEnteranceFee() )
#else // _VS2008PORTING
        if ( g_pUIGateKeeper->IsPublic() && CharacterMachine->Gold >= g_pUIGateKeeper->GetEnteranceFee() )
#endif // _VS2008PORTING
        {
            //  서버에 입장을 한다.
			g_pUIGateKeeper->SendEnter();
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_GATEKEEPER);
        }
#ifdef _VS2008PORTING
        else if ((int)CharacterMachine->Gold < g_pUIGateKeeper->GetEnteranceFee())
#else // _VS2008PORTING
        else if (CharacterMachine->Gold < g_pUIGateKeeper->GetEnteranceFee())
#endif // _VS2008PORTING
		{
			// 젠부족 메시지 박스
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CGatemanMoneyMsgBoxLayout));
        }
	}
}

void CNewUIGatemanWindow::RenderGuildMasterMode()
{
	POINT ptOrigin = { m_Pos.x, m_Pos.y+50 };
	g_pRenderText->SetFont(g_hFontBold);
	// 입장제한
	g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[1597], 190, 0, RT3_SORT_CENTER);
	g_pRenderText->SetFont(g_hFont);
	ptOrigin.y += 20;
	g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[1624], 190, 0, RT3_SORT_CENTER);
    ptOrigin.y += 10;
	g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[1625], 190, 0, RT3_SORT_CENTER);
    ptOrigin.y += 10;
	g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[1626], 190, 0, RT3_SORT_CENTER);
    ptOrigin.y += 20;

	RenderCheckBox( ptOrigin.x+35, ptOrigin.y, g_pUIGateKeeper->IsPublic() );
	// 1598 "일반 캐릭터 입장가능"
	g_pRenderText->RenderText(ptOrigin.x+55, ptOrigin.y, GlobalText[1598]);
	unicode::t_char szText[256];
	unicode::t_char szGold[64];
	ptOrigin.y += 18;
    ConvertGold ( g_pUIGateKeeper->GetEnteranceFee(), szGold );
    unicode::_sprintf( szText, GlobalText[1602], szGold );	            // 1602 "입장료 : %d Zen"
	g_pRenderText->RenderText( ptOrigin.x+35, ptOrigin.y, szText );

	// 입장료 설정
    glColor4f ( 0.f, 0.f, 0.f, 0.3f );
    RenderColor ( ptOrigin.x+20, ptOrigin.y+52, 90.f, 30.f );
    glColor3f ( 1.f, 1.f, 1.f );

    EnableAlphaTest ();
	ptOrigin.y += 30;
	
	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[1599], 190, 0, RT3_SORT_CENTER);
	
	g_pRenderText->SetFont(g_hFont);
    ConvertGold ( g_pUIGateKeeper->GetViewEnteranceFee(), szGold );

#ifdef PSW_BUGFIX_GATEMANWINDOW_TEXT
	unicode::_sprintf( szText, "%s %s", szGold, GlobalText[224] );							// 224 "젠"
	g_pRenderText->RenderText(ptOrigin.x+30+50, ptOrigin.y+32, szText, 0, 0, RT3_WRITE_RIGHT_TO_LEFT);
#else  //PSW_BUGFIX_GATEMANWINDOW_TEXT
	g_pRenderText->RenderText(ptOrigin.x+30+60, ptOrigin.y+32, szGold, 0, 0, RT3_WRITE_RIGHT_TO_LEFT);
	g_pRenderText->RenderText(ptOrigin.x+30+70, ptOrigin.y+32, GlobalText[224], 0, 0, RT3_WRITE_RIGHT_TO_LEFT);
#endif //PSW_BUGFIX_GATEMANWINDOW_TEXT

    //  설정 버튼.
    ptOrigin.y += 20;
	m_BtnFeeUp.Render();
	ptOrigin.y += 20;
	m_BtnFeeDn.Render();
	ptOrigin.y += 25;
	EnableAlphaTest();
	m_BtnSet.Render();
	ptOrigin.y += 25;

    //  입장료 설명.
	EnableAlphaTest();
	g_pRenderText->SetBgColor(0x00000000);
	g_pRenderText->SetTextColor(0xFFFFFFFF);
    ConvertGold ( g_pUIGateKeeper->GetMaxEnteranceFee(), szGold );
    unicode::_sprintf ( szText, GlobalText[1600], szGold );
	g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, szText, 190, 0, RT3_SORT_CENTER);
	
	ptOrigin.y += 13;
	g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[1601], 190, 0, RT3_SORT_CENTER);
	
	ptOrigin.y += 13;
    ConvertGold ( g_pUIGateKeeper->GetAddEnteranceFee(), szGold );
    unicode::_sprintf ( szText, GlobalText[1618], szGold );
	g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, szText, 190, 0, RT3_SORT_CENTER);

	ptOrigin.y += 50;
	m_BtnEnter.ChangeButtonInfo(m_Pos.x + INVENTORY_WIDTH / 2 - 27, m_Pos.y + 320, 53, 23);
	m_BtnEnter.Render();
}

void CNewUIGatemanWindow::RenderGuildMemeberMode()
{
	POINT ptOrigin = { m_Pos.x, m_Pos.y+50 };
	g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[1634], 190, 0, RT3_SORT_CENTER);
	
	ptOrigin.y += 50;
	m_BtnEnter.ChangeButtonInfo(m_Pos.x + INVENTORY_WIDTH / 2 - 27, m_Pos.y + 100, 53, 23);
	m_BtnEnter.Render();
}

void CNewUIGatemanWindow::RenderGuestMode()
{
	POINT ptOrigin = { m_Pos.x, m_Pos.y+50 };
	if ( g_pUIGateKeeper->IsPublic() )    //  입장 가능.
	{
		unicode::t_char szText[256];
		unicode::t_char szGold[64];
		ConvertGold ( g_pUIGateKeeper->GetEnteranceFee(), szGold );
		unicode::_sprintf ( szText, GlobalText[1632], szGold );

#ifdef _VS2008PORTING
		if ( g_pUIGateKeeper->GetEnteranceFee() > (int)CharacterMachine->Gold )
#else // _VS2008PORTING
		if ( g_pUIGateKeeper->GetEnteranceFee() > CharacterMachine->Gold )
#endif // _VS2008PORTING
		{
			g_pRenderText->SetTextColor(255, 100, 50, 255);
		}
		else
		{
			g_pRenderText->SetTextColor(255, 255, 100, 255);
		}

		g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, szText, 190, 0, RT3_SORT_CENTER);
		
		ptOrigin.y += 10;
		
		g_pRenderText->SetTextColor(0xFFFFFFFF);
		g_pRenderText->SetBgColor(0x00000000);
		g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[1633], 190, 0, RT3_SORT_CENTER);
		
		ptOrigin.y += 10;
		g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[1634], 190, 0, RT3_SORT_CENTER);
		
		ptOrigin.y += 10;

		m_BtnEnter.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
		m_BtnEnter.ChangeTextColor(RGBA(255, 255, 255, 255));
		m_BtnEnter.UnLock();
	}
	else                //  입장 불가능.
	{
		g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[1627], 190, 0, RT3_SORT_CENTER);
		ptOrigin.y += 10;
		g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[1629], 190, 0, RT3_SORT_CENTER);
		ptOrigin.y += 10;
		g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[1630], 190, 0, RT3_SORT_CENTER);
		ptOrigin.y += 10;
		g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[1631], 190, 0, RT3_SORT_CENTER);
		ptOrigin.y += 10;
    
		m_BtnEnter.ChangeImgColor(BUTTON_STATE_UP, RGBA(100, 100, 100, 255));
		m_BtnEnter.ChangeTextColor(RGBA(100, 100, 100, 255));
		m_BtnEnter.Lock();
	}
	ptOrigin.y += 10;

	m_BtnEnter.ChangeButtonInfo(m_Pos.x + INVENTORY_WIDTH / 2 - 27, m_Pos.y + 100, 53, 23);
	m_BtnEnter.Render();
}
