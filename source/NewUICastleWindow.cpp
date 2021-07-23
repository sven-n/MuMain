// NewUICastleWindow.cpp: implementation of the CNewUICastleWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "NewUICastleWindow.h"
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
#include "MixMgr.h"
#include "UISenatus.h"

using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNewUICastleWindow::CNewUICastleWindow() 
{
	m_pNewUIMng = NULL;
	m_Pos.x = m_Pos.y = 0;
	m_iNumCurOpenTab = TAB_GATE_MANAGING;
	m_iCurrMsgBoxRequest = CASTLE_MSGREQ_NULL;
}
CNewUICastleWindow::~CNewUICastleWindow() { Release(); }

bool CNewUICastleWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if(NULL == pNewUIMng)
		return false;

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_SENATUS, this);

	SetPos(x, y);

	LoadImages();

	list<unicode::t_string> ltext;
	ltext.push_back(GlobalText[1589]);
	ltext.push_back(GlobalText[1590]);
	ltext.push_back(GlobalText[1591]);
	ltext.push_back(GlobalText[1640]);

	m_TabBtn.CreateRadioGroup( 4, IMAGE_CASTLEWINDOW_TAB_BTN );
	m_TabBtn.ChangeRadioText(ltext);
	m_TabBtn.ChangeRadioButtonInfo(true, m_Pos.x+12.f, m_Pos.y+32.f, 40, 22 );
	m_TabBtn.ChangeFrame( m_iNumCurOpenTab );

	m_BtnExit.ChangeButtonImgState(true, IMAGE_CASTLEWINDOW_EXIT_BTN, false);
	m_BtnExit.ChangeButtonInfo(m_Pos.x+13, m_Pos.y+391, 36, 29);
	m_BtnExit.ChangeToolTipText(GlobalText[1002], true); // 225 "닫기(I,V)"

	InitButton(&m_BtnBuy, m_Pos.x + INVENTORY_WIDTH / 2 - 27, m_Pos.y + 250, GlobalText[1558]);
	InitButton(&m_BtnRepair, m_Pos.x + 110, m_Pos.y + 260, GlobalText[1559]);
	InitButton(&m_BtnUpgradeHP, m_Pos.x + 110, m_Pos.y + 310, GlobalText[1550]);
	InitButton(&m_BtnUpgradeDefense, m_Pos.x + 110, m_Pos.y + 334, GlobalText[1550]);
	InitButton(&m_BtnUpgradeRecover, m_Pos.x + 110, m_Pos.y + 358, GlobalText[1550]);
	InitButton(&m_BtnApplyTax, m_Pos.x + 120, m_Pos.y + 133, GlobalText[1106]);
	InitButton(&m_BtnWithdraw, m_Pos.x + 120, m_Pos.y + 322, GlobalText[236]);
	
	m_BtnChaosTaxUp.ChangeButtonImgState( true, IMAGE_CASTLEWINDOW_SCROLL_UP_BTN, true );
	m_BtnChaosTaxDn.ChangeButtonImgState( true, IMAGE_CASTLEWINDOW_SCROLL_DOWN_BTN, true );
	m_BtnNPCTaxUp.ChangeButtonImgState( true, IMAGE_CASTLEWINDOW_SCROLL_UP_BTN, true );
	m_BtnNPCTaxDn.ChangeButtonImgState( true, IMAGE_CASTLEWINDOW_SCROLL_DOWN_BTN, true );
	m_BtnChaosTaxUp.ChangeButtonInfo(m_Pos.x+158, m_Pos.y+73, 15, 13);
	m_BtnChaosTaxDn.ChangeButtonInfo(m_Pos.x+158, m_Pos.y+86, 15, 13);
	m_BtnNPCTaxUp.ChangeButtonInfo(m_Pos.x+158, m_Pos.y+101, 15, 13);
	m_BtnNPCTaxDn.ChangeButtonInfo(m_Pos.x+158, m_Pos.y+114, 15, 13);
	
	Show(false);
	
	return true;
}

void CNewUICastleWindow::InitButton(CNewUIButton * pNewUIButton, int iPos_x, int iPos_y, const unicode::t_char * pCaption)
{
	pNewUIButton->ChangeText(pCaption);
	pNewUIButton->ChangeTextBackColor(RGBA(255,255,255,0));
	pNewUIButton->ChangeButtonImgState( true, IMAGE_CASTLEWINDOW_BUTTON, true );
	pNewUIButton->ChangeButtonInfo(iPos_x, iPos_y, 53, 23);
	pNewUIButton->ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
	pNewUIButton->ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
}

void CNewUICastleWindow::Release()
{
	UnloadImages();

	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}
}

void CNewUICastleWindow::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
}

bool CNewUICastleWindow::UpdateMouseEvent()
{
	switch(m_iNumCurOpenTab)
	{
	case TAB_GATE_MANAGING:
		UpdateGateManagingTab();
		break;
	case TAB_STATUE_MANAGING:
		UpdateStatueManagingTab();
		break;
	case TAB_TAX_MANAGING:
		UpdateTaxManagingTab();
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

bool CNewUICastleWindow::UpdateKeyEvent()
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_SENATUS) == true)
	{
		if(SEASON3B::IsPress(VK_ESCAPE) == true)
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_SENATUS);
			PlayBuffer(SOUND_CLICK01);

			return false;
		}
	}	
	return true;
}

bool CNewUICastleWindow::Update()
{
	if(IsVisible())
	{
		int iNumCurOpenTab = m_TabBtn.UpdateMouseEvent();
		
		if( iNumCurOpenTab == RADIOGROUPEVENT_NONE )	// 탭 버튼의 변경이 없을시에는 Update skip
			return true;
		
		m_iNumCurOpenTab = iNumCurOpenTab;

		if (iNumCurOpenTab == TAB_CASTLE_MIX)
		{
			// 성주 조합 열기
			g_MixRecipeMgr.SetMixType(SEASON3A::MIXTYPE_CASTLE_SENIOR);
//	 		g_pNewUISystem->Hide(SEASON3B::INTERFACE_SENATUS);
			g_pNewUISystem->Show(SEASON3B::INTERFACE_MIXINVENTORY);
		}
	}
	return true;
}
bool CNewUICastleWindow::Render()
{
	EnableAlphaTest();
	glColor4f(1.f, 1.f, 1.f, 1.f);
	
	RenderFrame();
	
	m_TabBtn.Render();

	switch(m_iNumCurOpenTab)
	{
	case TAB_GATE_MANAGING:
		RenderGateManagingTab();
		break;
	case TAB_STATUE_MANAGING:
		RenderStatueManagingTab();
		break;
	case TAB_TAX_MANAGING:
		RenderTaxManagingTab();
		break;
	}
	
	// 닫기 버튼 표시
	m_BtnExit.Render();

	DisableAlphaBlend();

	return true;
}

void CNewUICastleWindow::OpeningProcess()
{
	m_iNumCurOpenTab = TAB_GATE_MANAGING;
	m_TabBtn.ChangeFrame(TAB_GATE_MANAGING);
	
	g_SenatusInfo.SetCurrGate(0);
	g_SenatusInfo.SetCurrStatue(0);

	SendRequestBCNPCList(1);	// 성문 NPC 리스트 요청
	SendRequestBCNPCList(2);	// 석상 NPC 리스트 요청
	SendRequestBCGetTaxInfo();
}

void CNewUICastleWindow::ClosingProcess()
{
	SendRequestMixExit();
}

float CNewUICastleWindow::GetLayerDepth()
{ return 5.0f; }

void CNewUICastleWindow::LoadImages()
{
	LoadBitmap("Interface\\newui_msgbox_back.jpg", IMAGE_CASTLEWINDOW_BACK, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back01.tga", IMAGE_CASTLEWINDOW_TOP, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-L.tga", IMAGE_CASTLEWINDOW_LEFT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-R.tga", IMAGE_CASTLEWINDOW_RIGHT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back03.tga", IMAGE_CASTLEWINDOW_BOTTOM, GL_LINEAR);
	LoadBitmap("Interface\\newui_exit_00.tga", IMAGE_CASTLEWINDOW_EXIT_BTN, GL_LINEAR);
	LoadBitmap("Interface\\newui_guild_tab04.tga", IMAGE_CASTLEWINDOW_TAB_BTN, GL_LINEAR);
	LoadBitmap("Interface\\newui_myquest_Line.tga", IMAGE_CASTLEWINDOW_LINE, GL_LINEAR);
	LoadBitmap("Interface\\newui_btn_empty_very_small.tga", IMAGE_CASTLEWINDOW_BUTTON, GL_LINEAR);
	
	LoadBitmap("Interface\\newui_item_table01(L).tga", IMAGE_CASTLEWINDOW_TABLE_TOP_LEFT);
	LoadBitmap("Interface\\newui_item_table01(R).tga", IMAGE_CASTLEWINDOW_TABLE_TOP_RIGHT);
	LoadBitmap("Interface\\newui_item_table02(L).tga", IMAGE_CASTLEWINDOW_TABLE_BOTTOM_LEFT);
	LoadBitmap("Interface\\newui_item_table02(R).tga", IMAGE_CASTLEWINDOW_TABLE_BOTTOM_RIGHT);
	LoadBitmap("Interface\\newui_item_table03(Up).tga", IMAGE_CASTLEWINDOW_TABLE_TOP_PIXEL);
	LoadBitmap("Interface\\newui_item_table03(Dw).tga", IMAGE_CASTLEWINDOW_TABLE_BOTTOM_PIXEL);
	LoadBitmap("Interface\\newui_item_table03(L).tga", IMAGE_CASTLEWINDOW_TABLE_LEFT_PIXEL);
	LoadBitmap("Interface\\newui_item_table03(R).tga", IMAGE_CASTLEWINDOW_TABLE_RIGHT_PIXEL);

	LoadBitmap("Interface\\newui_item_money2.tga", IMAGE_CASTLEWINDOW_MONEY, GL_LINEAR);
	LoadBitmap("Interface\\newui_Bt_scroll_up.jpg", IMAGE_CASTLEWINDOW_SCROLL_UP_BTN, GL_LINEAR);
	LoadBitmap("Interface\\newui_Bt_scroll_dn.jpg", IMAGE_CASTLEWINDOW_SCROLL_DOWN_BTN, GL_LINEAR);
}
void CNewUICastleWindow::UnloadImages()
{
	DeleteBitmap(IMAGE_CASTLEWINDOW_BOTTOM);
	DeleteBitmap(IMAGE_CASTLEWINDOW_RIGHT);
	DeleteBitmap(IMAGE_CASTLEWINDOW_LEFT);
	DeleteBitmap(IMAGE_CASTLEWINDOW_TOP);
	DeleteBitmap(IMAGE_CASTLEWINDOW_BACK);
	DeleteBitmap(IMAGE_CASTLEWINDOW_EXIT_BTN);
	DeleteBitmap(IMAGE_CASTLEWINDOW_TAB_BTN);
	DeleteBitmap(IMAGE_CASTLEWINDOW_LINE);
	DeleteBitmap(IMAGE_CASTLEWINDOW_BUTTON);
	DeleteBitmap(IMAGE_CASTLEWINDOW_TABLE_RIGHT_PIXEL);
	DeleteBitmap(IMAGE_CASTLEWINDOW_TABLE_LEFT_PIXEL);
	DeleteBitmap(IMAGE_CASTLEWINDOW_TABLE_BOTTOM_PIXEL);
	DeleteBitmap(IMAGE_CASTLEWINDOW_TABLE_TOP_PIXEL);
	DeleteBitmap(IMAGE_CASTLEWINDOW_TABLE_BOTTOM_RIGHT);
	DeleteBitmap(IMAGE_CASTLEWINDOW_TABLE_BOTTOM_LEFT);
	DeleteBitmap(IMAGE_CASTLEWINDOW_TABLE_TOP_RIGHT);
	DeleteBitmap(IMAGE_CASTLEWINDOW_TABLE_TOP_LEFT);
	DeleteBitmap(IMAGE_CASTLEWINDOW_MONEY);
	DeleteBitmap(IMAGE_CASTLEWINDOW_SCROLL_UP_BTN);
	DeleteBitmap(IMAGE_CASTLEWINDOW_SCROLL_DOWN_BTN);
}

void CNewUICastleWindow::RenderFrame()
{
	RenderImage(IMAGE_CASTLEWINDOW_BACK, m_Pos.x, m_Pos.y, 190.f, 429.f);
	RenderImage(IMAGE_CASTLEWINDOW_TOP, m_Pos.x, m_Pos.y, 190.f, 64.f);
	RenderImage(IMAGE_CASTLEWINDOW_LEFT, m_Pos.x, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_CASTLEWINDOW_RIGHT, m_Pos.x+INVENTORY_WIDTH-21, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_CASTLEWINDOW_BOTTOM, m_Pos.x, m_Pos.y+INVENTORY_HEIGHT-45, 190.f, 45.f);

	unicode::t_char szText[256] = { 0, };
	float fPos_x = m_Pos.x + 15.0f, fPos_y = m_Pos.y;
	float fLine_y = 13.0f;

	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetTextColor(220, 220, 220, 255);
	g_pRenderText->SetBgColor(0, 0, 0, 0);

	// 조합창 제목 표시
	unicode::_sprintf(szText, "%s", GlobalText[1588]);
	g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y, szText, 160.0f, 0, RT3_SORT_CENTER);
}

bool CNewUICastleWindow::BtnProcess()
{
	POINT ptExitBtn1 = { m_Pos.x+169, m_Pos.y+7 };
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	POINT ptExitBtn2 = { m_Pos.x+13, m_Pos.y+391 };
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX

	//. Exit1 버튼 (기본처리)
	if(SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_SENATUS);
	}

	// 닫기 버튼
	if(m_BtnExit.UpdateMouseEvent() == true)
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_SENATUS);
		return true;
	}

	return false;
}

void CNewUICastleWindow::UpdateGateManagingTab()
{
	POINT ptOrigin = { m_Pos.x, m_Pos.y+55+6+12 };
	// 성문버튼
	if( MouseLButtonPush )
	{
		// 공성맵 위인지 체크
		if( CheckMouseIn( ptOrigin.x+15, ptOrigin.y, 160.f, 165.f ) )
		{
			if( CheckMouseIn( ptOrigin.x+82, ptOrigin.y+35, 24, 24 ) )
				g_SenatusInfo.SetCurrGate(0);
			else if( CheckMouseIn( ptOrigin.x+64, ptOrigin.y+83, 24, 24 ) )
				g_SenatusInfo.SetCurrGate(1);
			else if( CheckMouseIn( ptOrigin.x+100, ptOrigin.y+83, 24, 24 ) )
				g_SenatusInfo.SetCurrGate(2);
			else if( CheckMouseIn( ptOrigin.x+48, ptOrigin.y+135, 24, 24 ) )
				g_SenatusInfo.SetCurrGate(3);
			else if( CheckMouseIn( ptOrigin.x+82, ptOrigin.y+135, 24, 24 ) )
				g_SenatusInfo.SetCurrGate(4);
			else if( CheckMouseIn( ptOrigin.x+116, ptOrigin.y+135, 24, 24 ) )
				g_SenatusInfo.SetCurrGate(5);
		}
	}

	SEASON3B::CNewUICommonMessageBox* pMsgBox = NULL;
	unicode::t_char szText[256] = { 0, };
	if(m_BtnBuy.UpdateMouseEvent() == true)
	{
		SetCurrMsgBoxRequest(CASTLE_MSGREQ_BUY_GATE);
		SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCastleMsgBoxLayout), &pMsgBox);
		pMsgBox->AddMsg(GlobalText[1551]);	// 1551 "선택하신 성문의 구입에는"
		unicode::_sprintf( szText, GlobalText[1617], g_SenatusInfo.GetRepairCost(&g_SenatusInfo.GetCurrGateInfo()) );	// 1617 "%d 젠이 소모됩니다."
		InsertComma(szText, g_SenatusInfo.GetRepairCost(&g_SenatusInfo.GetCurrGateInfo()));
		pMsgBox->AddMsg(szText);
		pMsgBox->AddMsg(GlobalText[1612]);	// 1612 "구입하시겠습니까?"
	}
	else if(m_BtnRepair.UpdateMouseEvent() == true)
	{
		SetCurrMsgBoxRequest(CASTLE_MSGREQ_REPAIR_GATE);
		SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCastleMsgBoxLayout), &pMsgBox);
		pMsgBox->AddMsg(GlobalText[1552]);	// 1551 "선택하신 성문의 구입에는"
		unicode::_sprintf( szText, GlobalText[1617], g_SenatusInfo.GetRepairCost(&g_SenatusInfo.GetCurrGateInfo()) );	// 1617 "%d 젠이 소모됩니다."
		InsertComma(szText, g_SenatusInfo.GetRepairCost(&g_SenatusInfo.GetCurrGateInfo()));
		pMsgBox->AddMsg(szText);
		pMsgBox->AddMsg(GlobalText[1612]);	// 1612 "구입하시겠습니까?"
	}
	else if(m_BtnUpgradeHP.UpdateMouseEvent() == true)
	{
		SetCurrMsgBoxRequest(CASTLE_MSGREQ_UPGRADE_GATE_HP);
		SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCastleMsgBoxLayout), &pMsgBox);
		pMsgBox->AddMsg(GlobalText[1555]);	// 1555 "선택하신 성문의 내구도 업그레이드에는"
		// 1553 "수호의보석 %d 개와 %d 젠이 소모됩니다."
		if( g_SenatusInfo.GetHPLevel( &g_SenatusInfo.GetCurrGateInfo() ) == 0 )
			unicode::_sprintf( szText, GlobalText[1553], 2, 1000000 );
		else if( g_SenatusInfo.GetHPLevel( &g_SenatusInfo.GetCurrGateInfo() ) == 1 )
			unicode::_sprintf( szText, GlobalText[1553], 3, 1000000 );
		else
			unicode::_sprintf( szText, GlobalText[1553], 4, 1000000 );
		InsertComma(szText, 1000000);
		pMsgBox->AddMsg(szText);
		pMsgBox->AddMsg(GlobalText[1554]);	// 1554 "수리하겠습니까?"
	}
	else if(m_BtnUpgradeDefense.UpdateMouseEvent() == true)
	{
		SetCurrMsgBoxRequest(CASTLE_MSGREQ_UPGRADE_GATE_DEFENSE);
		SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCastleMsgBoxLayout), &pMsgBox);
		pMsgBox->AddMsg(GlobalText[1556]);	// 1556 "선택하신 성문의 방어력 업그레이드에는"
		if( g_SenatusInfo.GetDefenseLevel( &g_SenatusInfo.GetCurrGateInfo() ) == 0 )
			unicode::_sprintf( szText, GlobalText[1553], 2, 3000000 );
		else if( g_SenatusInfo.GetDefenseLevel( &g_SenatusInfo.GetCurrGateInfo() ) == 1 )
			unicode::_sprintf( szText, GlobalText[1553], 3, 3000000 );
		else
			unicode::_sprintf( szText, GlobalText[1553], 4, 3000000 );
		InsertComma(szText, 3000000);
		pMsgBox->AddMsg(szText);
		pMsgBox->AddMsg(GlobalText[1554]);	// 1554 "수리하겠습니까?"
	}
}

void CNewUICastleWindow::UpdateStatueManagingTab()
{
	POINT ptOrigin = { m_Pos.x, m_Pos.y+55+6+12 };
	// 성문버튼
	if( MouseLButtonPush )
	{
		// 공성맵 위인지 체크
		if( CheckMouseIn( ptOrigin.x+15, ptOrigin.y, 160.f, 165.f ) )
		{
			if( CheckMouseIn( ptOrigin.x+82, ptOrigin.y+20, 24, 24 ) )
				g_SenatusInfo.SetCurrStatue(0);
			else if( CheckMouseIn( ptOrigin.x+82, ptOrigin.y+65, 24, 24 ) )
				g_SenatusInfo.SetCurrStatue(1);
			else if( CheckMouseIn( ptOrigin.x+64, ptOrigin.y+110, 24, 24 ) )
				g_SenatusInfo.SetCurrStatue(2);
			else if( CheckMouseIn( ptOrigin.x+100, ptOrigin.y+110, 24, 24 ) )
				g_SenatusInfo.SetCurrStatue(3);
		}
	}

	SEASON3B::CNewUICommonMessageBox* pMsgBox = NULL;
	unicode::t_char szText[256] = { 0, };
	if(m_BtnBuy.UpdateMouseEvent() == true)
	{
		SetCurrMsgBoxRequest(CASTLE_MSGREQ_BUY_STATUE);
		SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCastleMsgBoxLayout), &pMsgBox);
		pMsgBox->AddMsg(GlobalText[1610]);	// 1610 "선택하신 석상의 구입에는"
		unicode::_sprintf( szText, GlobalText[1617], g_SenatusInfo.GetRepairCost(&g_SenatusInfo.GetCurrStatueInfo()) );	// 1617 "%d 젠이 소모됩니다."
		InsertComma(szText, g_SenatusInfo.GetRepairCost(&g_SenatusInfo.GetCurrStatueInfo()));
		pMsgBox->AddMsg(szText);
		pMsgBox->AddMsg(GlobalText[1612]);	// 1612 "구입하시겠습니까?"
	}
	else if(m_BtnRepair.UpdateMouseEvent() == true)
	{
		SetCurrMsgBoxRequest(CASTLE_MSGREQ_REPAIR_STATUE);
		SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCastleMsgBoxLayout), &pMsgBox);
		pMsgBox->AddMsg(GlobalText[1611]);	// 1611 "선택하신 석상의 수리에는"
		unicode::_sprintf( szText, GlobalText[1617], g_SenatusInfo.GetRepairCost(&g_SenatusInfo.GetCurrStatueInfo()) );	// 1617 "%d 젠이 소모됩니다."
		InsertComma(szText, g_SenatusInfo.GetRepairCost(&g_SenatusInfo.GetCurrStatueInfo()));
		pMsgBox->AddMsg(szText);
		pMsgBox->AddMsg(GlobalText[1554]);	// 1554 "수리하겠습니까?"
	}
	else if(m_BtnUpgradeHP.UpdateMouseEvent() == true)
	{
		SetCurrMsgBoxRequest(CASTLE_MSGREQ_UPGRADE_STATUE_HP);
		SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCastleMsgBoxLayout), &pMsgBox);
		pMsgBox->AddMsg(GlobalText[1613]);		// 1613 "선택하신 성문의 내구도 업그레이드에는"
		// 1553 "수호의보석 %d 개와 %d 젠이 소모됩니다."
		if( g_SenatusInfo.GetHPLevel( &g_SenatusInfo.GetCurrStatueInfo() ) == 0 )
			unicode::_sprintf( szText, GlobalText[1553], 3, 1000000 );
		else if( g_SenatusInfo.GetHPLevel( &g_SenatusInfo.GetCurrStatueInfo() ) == 1 )
			unicode::_sprintf( szText, GlobalText[1553], 5, 1000000 );
		else
			unicode::_sprintf( szText, GlobalText[1553], 7, 1000000 );
		InsertComma(szText, 1000000);
		pMsgBox->AddMsg(szText);
		pMsgBox->AddMsg(GlobalText[1554]);	// 1554 "수리하겠습니까?"
	}
	else if(m_BtnUpgradeDefense.UpdateMouseEvent() == true)
	{
		SetCurrMsgBoxRequest(CASTLE_MSGREQ_UPGRADE_STATUE_DEFENSE);
		SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCastleMsgBoxLayout), &pMsgBox);
		pMsgBox->AddMsg(GlobalText[1614]);		// 1614 "선택하신 성문의 방어력 업그레이드에는"
		// 1553 "수호의보석 %d 개와 %d 젠이 소모됩니다."
		if( g_SenatusInfo.GetDefenseLevel( &g_SenatusInfo.GetCurrStatueInfo() ) == 0 )
			unicode::_sprintf( szText, GlobalText[1553], 3, 3000000 );
		else if( g_SenatusInfo.GetDefenseLevel( &g_SenatusInfo.GetCurrStatueInfo() ) == 1 )
			unicode::_sprintf( szText, GlobalText[1553], 5, 3000000 );
		else
			unicode::_sprintf( szText, GlobalText[1553], 7, 3000000 );
		InsertComma(szText, 3000000);
		pMsgBox->AddMsg(szText);
		pMsgBox->AddMsg(GlobalText[1554]);	// 1554 "수리하겠습니까?"
	}
	else if(m_BtnUpgradeRecover.UpdateMouseEvent() == true)
	{
		SetCurrMsgBoxRequest(CASTLE_MSGREQ_UPGRADE_STATUE_RECOVER);
		SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCastleMsgBoxLayout), &pMsgBox);
		pMsgBox->AddMsg(GlobalText[1615]);		// 1615 "선택하신 석상의 회복력 업그레이드에는"
		// 1553 "수호의보석 %d 개와 %d 젠이 소모됩니다."
		if( g_SenatusInfo.GetRecoverLevel( &g_SenatusInfo.GetCurrStatueInfo() ) == 0 )
			unicode::_sprintf( szText, GlobalText[1553], 3, 5000000 );
		else if( g_SenatusInfo.GetRecoverLevel( &g_SenatusInfo.GetCurrStatueInfo() ) == 1 )
			unicode::_sprintf( szText, GlobalText[1553], 5, 5000000 );
		else
			unicode::_sprintf( szText, GlobalText[1553], 7, 5000000 );
		InsertComma(szText, 5000000);
		pMsgBox->AddMsg(szText);
		pMsgBox->AddMsg(GlobalText[1554]);	// 1554 "수리하겠습니까?"
	}
}

void CNewUICastleWindow::UpdateTaxManagingTab()
{
	SEASON3B::CNewUICommonMessageBox* pMsgBox = NULL;
	unicode::t_char szText[256] = { 0, };
	if(m_BtnApplyTax.UpdateMouseEvent() == true)
	{
		SetCurrMsgBoxRequest(CASTLE_MSGREQ_APPLY_TAX);
		SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCastleMsgBoxLayout), &pMsgBox);
		unicode::_sprintf( szText, GlobalText[1566], g_SenatusInfo.GetChaosTaxRate() );	// 1566 "카오스 조합 고블린 세율 %d%%"
		pMsgBox->AddMsg(szText);
		unicode::_sprintf( szText, GlobalText[1567], g_SenatusInfo.GetNormalTaxRate() );	// 1567 "각종 NPC 세율 %d%%"
		pMsgBox->AddMsg(szText);
		pMsgBox->AddMsg(GlobalText[1568]);	// 1568 "적용하시겠습니까?"
	}
	else if(m_BtnWithdraw.UpdateMouseEvent() == true)
	{
		SetCurrMsgBoxRequest(CASTLE_MSGREQ_WITHDRAW);
		SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCastleWithdrawMsgBoxLayout), &pMsgBox);
	}
	else if(m_BtnChaosTaxUp.UpdateMouseEvent() == true)
	{
		g_SenatusInfo.PlusChaosTaxRate(1);
	}
	else if(m_BtnChaosTaxDn.UpdateMouseEvent() == true)
	{
		g_SenatusInfo.PlusChaosTaxRate(-1);
	}
	else if(m_BtnNPCTaxUp.UpdateMouseEvent() == true)
	{
		g_SenatusInfo.PlusNormalTaxRate(1);
	}
	else if(m_BtnNPCTaxDn.UpdateMouseEvent() == true)
	{
		g_SenatusInfo.PlusNormalTaxRate(-1);
	}
}

void CNewUICastleWindow::RenderOutlineUpper(float fPos_x, float fPos_y, float fWidth, float fHeight)
{
	POINT ptOrigin = { fPos_x, fPos_y };
	float fBoxWidth = fWidth;
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	float fBoxHeight = fHeight;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX

	// 위쪽 테두리
	RenderImage(IMAGE_CASTLEWINDOW_TABLE_TOP_LEFT, ptOrigin.x+12, ptOrigin.y-4, 14, 14);
	RenderImage(IMAGE_CASTLEWINDOW_TABLE_TOP_RIGHT, ptOrigin.x+fBoxWidth+4, ptOrigin.y-4, 14, 14);
	// 위쪽 라인
	RenderImage(IMAGE_CASTLEWINDOW_TABLE_TOP_PIXEL, ptOrigin.x+25, ptOrigin.y-4, fBoxWidth-21, 14);
	glColor4f(0.0f, 0.0f, 0.0f, 0.3f);
	RenderColor(ptOrigin.x+15, ptOrigin.y-3, fBoxWidth-2, 15);
	EndRenderColor();
}

void CNewUICastleWindow::RenderOutlineLower(float fPos_x, float fPos_y, float fWidth, float fHeight)
{
	POINT ptOrigin = { fPos_x, fPos_y };
	float fBoxWidth = fWidth;
	float fBoxHeight = fHeight;

	// 왼쪽 라인, 오른쪽 라인
	RenderImage(IMAGE_CASTLEWINDOW_TABLE_LEFT_PIXEL, ptOrigin.x+12, ptOrigin.y+9, 14, fBoxHeight);
	RenderImage(IMAGE_CASTLEWINDOW_TABLE_RIGHT_PIXEL, ptOrigin.x+fBoxWidth+4, ptOrigin.y+9, 14, fBoxHeight);
	// 중간라인
	RenderImage(IMAGE_CASTLEWINDOW_TABLE_BOTTOM_PIXEL, ptOrigin.x+15, ptOrigin.y+3, fBoxWidth-2, 14);
	// 아래쪽 테두리
	RenderImage(IMAGE_CASTLEWINDOW_TABLE_BOTTOM_LEFT, ptOrigin.x+12, ptOrigin.y+fBoxHeight+3, 14, 14);
	RenderImage(IMAGE_CASTLEWINDOW_TABLE_BOTTOM_RIGHT, ptOrigin.x+fBoxWidth+4, ptOrigin.y+fBoxHeight+3, 14, 14);
	// 아래쪽 라인
	RenderImage(IMAGE_CASTLEWINDOW_TABLE_BOTTOM_PIXEL, ptOrigin.x+25, ptOrigin.y+fBoxHeight+3, fBoxWidth-21, 14);
}

void CNewUICastleWindow::RenderGateManagingTab()
{
	LPPMSG_NPCDBLIST pNPCInfo = &g_SenatusInfo.GetCurrGateInfo();
	POINT ptOrigin = { m_Pos.x, m_Pos.y+55 };

	g_pRenderText->SetFont(g_hFontBold);

	ptOrigin.y += 6;
	RenderOutlineUpper(ptOrigin.x, ptOrigin.y, 160, 165);
	// 제목 텍스트
	g_pRenderText->RenderText( ptOrigin.x, ptOrigin.y, GlobalText[1557], 190, 0, RT3_SORT_CENTER);
	// 공성맵 전체이미지
	RenderBitmap( BITMAP_INTERFACE_EX+35, ptOrigin.x+15, ptOrigin.y+12, 160.f, 165.f, 0.f, 0.f, 160.f/256.f, 165.f/256.f );
	RenderOutlineLower(ptOrigin.x, ptOrigin.y, 160, 165);

	ptOrigin.y += 12;
	// 성문버튼 이미지
	RenderCastleItem( ptOrigin.x+82, ptOrigin.y+35, &g_SenatusInfo.GetGateInfo(0) );
	RenderCastleItem( ptOrigin.x+64, ptOrigin.y+83, &g_SenatusInfo.GetGateInfo(1) );
	RenderCastleItem( ptOrigin.x+100, ptOrigin.y+83, &g_SenatusInfo.GetGateInfo(2) );
	RenderCastleItem( ptOrigin.x+48, ptOrigin.y+135, &g_SenatusInfo.GetGateInfo(3) );
	RenderCastleItem( ptOrigin.x+82, ptOrigin.y+135, &g_SenatusInfo.GetGateInfo(4) );
	RenderCastleItem( ptOrigin.x+116, ptOrigin.y+135, &g_SenatusInfo.GetGateInfo(5) );

	g_pRenderText->SetFont(g_hFont);
	
	ptOrigin.y += 173;
	if( pNPCInfo->btNpcLive == 0 )
	{
		m_BtnBuy.Render();
	}
	else
	{
		if (g_SenatusInfo.IsGateRepairable())
		{
			m_BtnRepair.UnLock();
			m_BtnRepair.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
			m_BtnRepair.ChangeTextColor(RGBA(255, 255, 255, 255));
		}
		else
		{
			m_BtnRepair.Lock();
			m_BtnRepair.ChangeImgColor(BUTTON_STATE_UP, RGBA(100, 100, 100, 255));
			m_BtnRepair.ChangeTextColor(RGBA(100, 100, 100, 255));
		}
		if (g_SenatusInfo.IsGateHPUpgradable())
		{
			m_BtnUpgradeHP.UnLock();
			m_BtnUpgradeHP.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
			m_BtnUpgradeHP.ChangeTextColor(RGBA(255, 255, 255, 255));
		}
		else
		{
			m_BtnUpgradeHP.Lock();
			m_BtnUpgradeHP.ChangeImgColor(BUTTON_STATE_UP, RGBA(100, 100, 100, 255));
			m_BtnUpgradeHP.ChangeTextColor(RGBA(100, 100, 100, 255));
		}
		if (g_SenatusInfo.IsGateDefeseUpgradable())
		{
			m_BtnUpgradeDefense.UnLock();
			m_BtnUpgradeDefense.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
			m_BtnUpgradeDefense.ChangeTextColor(RGBA(255, 255, 255, 255));
		}
		else
		{
			m_BtnUpgradeDefense.Lock();
			m_BtnUpgradeDefense.ChangeImgColor(BUTTON_STATE_UP, RGBA(100, 100, 100, 255));
			m_BtnUpgradeDefense.ChangeTextColor(RGBA(100, 100, 100, 255));
		}

		m_BtnRepair.Render();
		m_BtnUpgradeHP.Render();
		m_BtnUpgradeDefense.Render();

		g_pRenderText->SetBgColor(0x00000000);
		g_pRenderText->SetTextColor(0xFFFFFFFF);
		
		unicode::t_char szTemp[256];
		unicode::_sprintf( szTemp, GlobalText[1560], pNPCInfo->iNpcHp, pNPCInfo->iNpcMaxHp );		// 1560 "내구도 : %d/%d"
		InsertComma(szTemp, pNPCInfo->iNpcHp);
		InsertComma(szTemp, pNPCInfo->iNpcMaxHp);
		g_pRenderText->RenderText( ptOrigin.x+20, ptOrigin.y, szTemp );
		ptOrigin.y += 13;
		unicode::_sprintf( szTemp, GlobalText[1561], g_SenatusInfo.GetDefense(pNPCInfo->iNpcNumber,pNPCInfo->iNpcDfLevel) );	// 1561 "방어력 : %d"
		g_pRenderText->RenderText( ptOrigin.x+20, ptOrigin.y, szTemp );

		
		ptOrigin.y += 35;
		RenderOutlineUpper(ptOrigin.x, ptOrigin.y, 160, 78);
		RenderOutlineLower(ptOrigin.x, ptOrigin.y, 160, 78);

		g_pRenderText->SetFont(g_hFontBold);
		g_pRenderText->RenderText( ptOrigin.x, ptOrigin.y, GlobalText[1550], 190, 0, RT3_SORT_CENTER );	// 1550 "강화"
		g_pRenderText->SetFont(g_hFont);
		
		ptOrigin.y += 24;
		unicode::_sprintf( szTemp, GlobalText[1563], g_SenatusInfo.GetNextAddHP( pNPCInfo ) );		// 1563 "내구도 +%d"
		InsertComma(szTemp, g_SenatusInfo.GetNextAddHP( pNPCInfo ));
		g_pRenderText->RenderText( ptOrigin.x+30, ptOrigin.y, szTemp );
		ptOrigin.y += 23;
		unicode::_sprintf( szTemp, GlobalText[1564], g_SenatusInfo.GetNextAddDefense( pNPCInfo ) );	// 1564 "방어력 +%d"
		InsertComma(szTemp, g_SenatusInfo.GetNextAddDefense( pNPCInfo ));
		g_pRenderText->RenderText( ptOrigin.x+30, ptOrigin.y, szTemp );
	}
}

void CNewUICastleWindow::RenderStatueManagingTab()
{
	POINT ptOrigin = { m_Pos.x, m_Pos.y+55 };
	LPPMSG_NPCDBLIST pNPCInfo = &g_SenatusInfo.GetCurrStatueInfo();

	g_pRenderText->SetFont(g_hFontBold);
	
	ptOrigin.y += 6;
	RenderOutlineUpper(ptOrigin.x, ptOrigin.y, 160, 165);
	// 제목 텍스트
	g_pRenderText->RenderText( ptOrigin.x, ptOrigin.y, GlobalText[1557], 190, 0, RT3_SORT_CENTER);
	// 공성맵 전체이미지
	RenderBitmap( BITMAP_INTERFACE_EX+35, ptOrigin.x+15, ptOrigin.y+12, 160.f, 165.f, 0.f, 0.f, 160.f/256.f, 165.f/256.f );
	RenderOutlineLower(ptOrigin.x, ptOrigin.y, 160, 165);

	ptOrigin.y += 12;
	// 성문버튼 이미지
	RenderCastleItem( ptOrigin.x+82, ptOrigin.y+20, &g_SenatusInfo.GetStatueInfo(0) );
	RenderCastleItem( ptOrigin.x+82, ptOrigin.y+65, &g_SenatusInfo.GetStatueInfo(1) );
	RenderCastleItem( ptOrigin.x+64, ptOrigin.y+110, &g_SenatusInfo.GetStatueInfo(2) );
	RenderCastleItem( ptOrigin.x+100, ptOrigin.y+110, &g_SenatusInfo.GetStatueInfo(3) );

	g_pRenderText->SetFont(g_hFont);
	
	ptOrigin.y += 173;
	if( pNPCInfo->btNpcLive == 0 )
	{
		m_BtnBuy.Render();
	}
	else
	{
		if (g_SenatusInfo.IsStatueRepairable())
		{
			m_BtnRepair.UnLock();
			m_BtnRepair.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
			m_BtnRepair.ChangeTextColor(RGBA(255, 255, 255, 255));
		}
		else
		{
			m_BtnRepair.Lock();
			m_BtnRepair.ChangeImgColor(BUTTON_STATE_UP, RGBA(100, 100, 100, 255));
			m_BtnRepair.ChangeTextColor(RGBA(100, 100, 100, 255));
		}
		if (g_SenatusInfo.IsStatueHPUpgradable())
		{
			m_BtnUpgradeHP.UnLock();
			m_BtnUpgradeHP.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
			m_BtnUpgradeHP.ChangeTextColor(RGBA(255, 255, 255, 255));
		}
		else
		{
			m_BtnUpgradeHP.Lock();
			m_BtnUpgradeHP.ChangeImgColor(BUTTON_STATE_UP, RGBA(100, 100, 100, 255));
			m_BtnUpgradeHP.ChangeTextColor(RGBA(100, 100, 100, 255));
		}
		if (g_SenatusInfo.IsStatueDefeseUpgradable())
		{
			m_BtnUpgradeDefense.UnLock();
			m_BtnUpgradeDefense.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
			m_BtnUpgradeDefense.ChangeTextColor(RGBA(255, 255, 255, 255));
		}
		else
		{
			m_BtnUpgradeDefense.Lock();
			m_BtnUpgradeDefense.ChangeImgColor(BUTTON_STATE_UP, RGBA(100, 100, 100, 255));
			m_BtnUpgradeDefense.ChangeTextColor(RGBA(100, 100, 100, 255));
		}
		if (g_SenatusInfo.IsStatueRecoverUpgradable())
		{
			m_BtnUpgradeRecover.UnLock();
			m_BtnUpgradeRecover.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
			m_BtnUpgradeRecover.ChangeTextColor(RGBA(255, 255, 255, 255));
		}
		else
		{
			m_BtnUpgradeRecover.Lock();
			m_BtnUpgradeRecover.ChangeImgColor(BUTTON_STATE_UP, RGBA(100, 100, 100, 255));
			m_BtnUpgradeRecover.ChangeTextColor(RGBA(100, 100, 100, 255));
		}

		m_BtnRepair.Render();
		m_BtnUpgradeHP.Render();
		m_BtnUpgradeDefense.Render();
		m_BtnUpgradeRecover.Render();

		g_pRenderText->SetTextColor(0xFFFFFFFF);
		g_pRenderText->SetBgColor(0);
		
		unicode::t_char szTemp[256];
		unicode::_sprintf( szTemp, GlobalText[1560], pNPCInfo->iNpcHp, pNPCInfo->iNpcMaxHp );						// 1560 "내구도 : %d/%d"
		InsertComma(szTemp, pNPCInfo->iNpcHp);
		InsertComma(szTemp, pNPCInfo->iNpcMaxHp);
		g_pRenderText->RenderText( ptOrigin.x+20, ptOrigin.y, szTemp);
		ptOrigin.y += 13;
		unicode::_sprintf( szTemp, GlobalText[1561], g_SenatusInfo.GetDefense(pNPCInfo->iNpcNumber,pNPCInfo->iNpcDfLevel) );	// 1561 "방어력 : %d"
		g_pRenderText->RenderText( ptOrigin.x+20, ptOrigin.y, szTemp);
		ptOrigin.y += 13;
		unicode::_sprintf( szTemp, GlobalText[1562], g_SenatusInfo.GetRecover(pNPCInfo->iNpcNumber,pNPCInfo->iNpcRgLevel) );	// 1562 "회복력 : %d%%"
		g_pRenderText->RenderText( ptOrigin.x+20, ptOrigin.y, szTemp);

		ptOrigin.y += 22;
		RenderOutlineUpper(ptOrigin.x, ptOrigin.y, 160, 78);
		RenderOutlineLower(ptOrigin.x, ptOrigin.y, 160, 78);
		g_pRenderText->SetFont(g_hFontBold);
		g_pRenderText->RenderText( ptOrigin.x, ptOrigin.y, GlobalText[1550], 190, 0, RT3_SORT_CENTER);	// 1550 "강화"
		g_pRenderText->SetFont(g_hFont);

		ptOrigin.y += 24;
		unicode::_sprintf( szTemp, GlobalText[1563], g_SenatusInfo.GetNextAddHP( pNPCInfo ) );		// 1563 "내구도 +%d"
		InsertComma(szTemp, g_SenatusInfo.GetNextAddHP( pNPCInfo ));
		g_pRenderText->RenderText( ptOrigin.x+30, ptOrigin.y, szTemp );
		ptOrigin.y += 23;
		unicode::_sprintf( szTemp, GlobalText[1564], g_SenatusInfo.GetNextAddDefense( pNPCInfo ) );	// 1564 "방어력 +%d"
		InsertComma(szTemp, g_SenatusInfo.GetNextAddDefense( pNPCInfo ));
		g_pRenderText->RenderText( ptOrigin.x+30, ptOrigin.y, szTemp );
		ptOrigin.y += 23;
		unicode::_sprintf( szTemp, GlobalText[1565], g_SenatusInfo.GetNextAddRecover( pNPCInfo ) );	// 1565 "회복력 +%d%%"
		InsertComma(szTemp, g_SenatusInfo.GetNextAddRecover( pNPCInfo ));
		g_pRenderText->RenderText( ptOrigin.x+30, ptOrigin.y, szTemp );
	}
}

void CNewUICastleWindow::RenderTaxManagingTab()
{
	POINT ptOrigin = { m_Pos.x, m_Pos.y+55 };
	g_pRenderText->SetTextColor(0xFFFFFFFF);
	g_pRenderText->SetBgColor(0);
	
	ptOrigin.y += 6;

	EnableAlphaTest();
	glColor4f( 0.4f, 0.4f, 0.4f, 0.5f );
	RenderColor( ptOrigin.x+15, ptOrigin.y+14, 150, 24 );
	RenderColor( ptOrigin.x+15, ptOrigin.y+42, 150, 24 );
	EndRenderColor();

	RenderOutlineUpper(ptOrigin.x, ptOrigin.y, 160, 55);

	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->RenderText( ptOrigin.x, ptOrigin.y, GlobalText[1572], 190, 0, RT3_SORT_CENTER );	// 1572 "세 율 조 절"
	
	RenderOutlineLower(ptOrigin.x, ptOrigin.y, 160, 55);

	// 중간라인
	RenderImage(IMAGE_CASTLEWINDOW_TABLE_BOTTOM_PIXEL, ptOrigin.x+15, ptOrigin.y+30, 160-2, 14);

	m_BtnChaosTaxUp.Render();
	m_BtnChaosTaxDn.Render();
	m_BtnNPCTaxUp.Render();
	m_BtnNPCTaxDn.Render();

	unicode::t_char szTemp[256];
	ptOrigin.y += 23;
	g_pRenderText->SetFont(g_hFont);
	
	// 1573 "카오스 조합 고블린 : %d(%d)%%"
	unicode::_sprintf( szTemp, GlobalText[1573], g_SenatusInfo.GetRealTaxRateChaos(), g_SenatusInfo.GetChaosTaxRate() );
	g_pRenderText->RenderText( ptOrigin.x, ptOrigin.y, szTemp, 175, 0, RT3_SORT_CENTER );
	ptOrigin.y += 25;
	// 1574 "각종 NPC : %d(%d)%%"
	unicode::_sprintf( szTemp, GlobalText[1574], g_SenatusInfo.GetRealTaxRateStore(), g_SenatusInfo.GetNormalTaxRate() );
	g_pRenderText->RenderText( ptOrigin.x, ptOrigin.y, szTemp, 175, 0, RT3_SORT_CENTER );

	m_BtnApplyTax.Render();

#if SELECTED_LANGUAGE == LANGUAGE_PHILIPPINES || SELECTED_LANGUAGE == LANGUAGE_ENGLISH
	// 가운데 정렬
	ptOrigin.y += 53;
	g_pRenderText->RenderText( ptOrigin.x+15, ptOrigin.y, GlobalText[1575], 160, 0, RT3_SORT_CENTER );	// 1575 "해당 NPC : 떠돌이 상인, 물약소녀,"
	ptOrigin.y += 13;
	g_pRenderText->RenderText( ptOrigin.x+15, ptOrigin.y, GlobalText[1576], 160, 0, RT3_SORT_CENTER );	// 1576 "마담 ,마법사, 대장장이, 알렉스,"
	ptOrigin.y += 13;
	g_pRenderText->RenderText( ptOrigin.x+15, ptOrigin.y, GlobalText[1577], 160, 0, RT3_SORT_CENTER );	// 1577 "라라요정, 장인, 톰슨켄넬, 무기상"
#else	// SELECTED_LANGUAGE == LANGUAGE_PHILIPPINES || SELECTED_LANGUAGE == LANGUAGE_ENGLISH
	ptOrigin.y += 53;
	g_pRenderText->RenderText( ptOrigin.x+32, ptOrigin.y, GlobalText[1575] );	// 1575 "해당 NPC : 떠돌이 상인, 물약소녀,"
	ptOrigin.y += 13;
	g_pRenderText->RenderText( ptOrigin.x+32, ptOrigin.y, GlobalText[1576] );	// 1576 "마담 ,마법사, 대장장이, 알렉스,"
	ptOrigin.y += 13;
	g_pRenderText->RenderText( ptOrigin.x+32, ptOrigin.y, GlobalText[1577] );	// 1577 "라라요정, 장인, 톰슨켄넬, 무기상"
#endif	// SELECTED_LANGUAGE == LANGUAGE_PHILIPPINES || SELECTED_LANGUAGE == LANGUAGE_ENGLISH

	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetTextColor(0xFF947BBB);
	
	ptOrigin.y += 20;
#ifdef ASG_FIX_TEXT_SCRIPT_PERCENT_TREATMENT
	unicode::_sprintf(szTemp, GlobalText[1578]);	// 1578 "조합고블린 최대 3%%, NPC 최대 3%%"
	g_pRenderText->RenderText( ptOrigin.x+15, ptOrigin.y, szTemp, 160, 0, RT3_SORT_CENTER );
#else	// ASG_FIX_TEXT_SCRIPT_PERCENT_TREATMENT
	g_pRenderText->RenderText( ptOrigin.x+15, ptOrigin.y, GlobalText[1578], 160, 0, RT3_SORT_CENTER );	// 1578 "조절 범위는 0%~200% 입니다."
#endif	// ASG_FIX_TEXT_SCRIPT_PERCENT_TREATMENT
	ptOrigin.y += 12;
#ifdef ASG_FIX_TEXT_SCRIPT_PERCENT_TREATMENT
	unicode::_sprintf(szTemp, GlobalText[1579]);	// 1579 "공성 직후부터 다음 공성 전까지" , 글로벌:"Maximum Tax rates: 3%%"
	g_pRenderText->RenderText( ptOrigin.x+15, ptOrigin.y, szTemp, 160, 0, RT3_SORT_CENTER );
#else	// ASG_FIX_TEXT_SCRIPT_PERCENT_TREATMENT
	g_pRenderText->RenderText( ptOrigin.x+15, ptOrigin.y, GlobalText[1579], 160, 0, RT3_SORT_CENTER );	// 1579 "공성 직후부터 다음 공성 전까지"
#endif	// ASG_FIX_TEXT_SCRIPT_PERCENT_TREATMENT
	ptOrigin.y += 12;
	g_pRenderText->RenderText( ptOrigin.x+15, ptOrigin.y, GlobalText[1580], 160, 0, RT3_SORT_CENTER );	// 1580 "조절가능합니다."
	ptOrigin.y += 12;
	g_pRenderText->RenderText( ptOrigin.x+15, ptOrigin.y, GlobalText[1581], 160, 0, RT3_SORT_CENTER );	// 1581 "조절 제한 횟수는 없으며,"
	ptOrigin.y += 12;
	g_pRenderText->RenderText( ptOrigin.x+15, ptOrigin.y, GlobalText[1582], 160, 0, RT3_SORT_CENTER );	// 1582 "성을 차지한 길드마스터만이"
	ptOrigin.y += 12;
	g_pRenderText->RenderText( ptOrigin.x+15, ptOrigin.y, GlobalText[1583], 160, 0, RT3_SORT_CENTER );	// 1583 "조절가능합니다."

	ptOrigin.y += 10;
	RenderBitmap(IMAGE_CASTLEWINDOW_LINE, ptOrigin.x+1, ptOrigin.y,
		188.f, 21, 0.f, 0.f, 188/256.f, 21/32.f);

	g_pRenderText->SetTextColor(0xFFFFFFFF);
	
	ptOrigin.y += 18;
	RenderImage(IMAGE_CASTLEWINDOW_MONEY, ptOrigin.x+10, ptOrigin.y, 170.f, 24.f);

	unicode::_sprintf( szTemp, GlobalText[1246] );	// 1584 "성의 보유 Zen : %I64d"
	g_pRenderText->RenderText( ptOrigin.x+14, ptOrigin.y+7, szTemp );

	//unicode::t_char szGoldText[32];
	//ConvertGold(g_SenatusInfo.GetCastleMoney(),szGoldText);

	unicode::_sprintf( szTemp, "%I64d", g_SenatusInfo.GetCastleMoney() );	// 1584 "성의 보유 Zen : %I64d"
#ifdef YDG_FIX_CATLE_MONEY_INT64_TYPE_CRASH
	InsertComma64(szTemp, g_SenatusInfo.GetCastleMoney());
#else	// YDG_FIX_CATLE_MONEY_INT64_TYPE_CRASH
	InsertComma(szTemp, g_SenatusInfo.GetCastleMoney());
#endif	// YDG_FIX_CATLE_MONEY_INT64_TYPE_CRASH
	g_pRenderText->RenderText( ptOrigin.x+90, ptOrigin.y+7, szTemp, 80, 0, RT3_SORT_RIGHT );
	
	m_BtnWithdraw.Render();

	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetTextColor(0xFF947BBB);
	
	ptOrigin.y += 54;
	g_pRenderText->RenderText( ptOrigin.x+15, ptOrigin.y, GlobalText[1585], 160, 0, RT3_SORT_CENTER );	// 1585 "세금은 성의 보유 Zen으로 되며"
	ptOrigin.y += 12;
	g_pRenderText->RenderText( ptOrigin.x+15, ptOrigin.y, GlobalText[1586], 160, 0, RT3_SORT_CENTER );	// 1586 "성의 운영에 관련되는 젠 소모는"
	ptOrigin.y += 12;
	g_pRenderText->RenderText( ptOrigin.x+15, ptOrigin.y, GlobalText[1587], 160, 0, RT3_SORT_CENTER );	// 1587 "이곳에서 빠져 나가게 됩니다."
}

void CNewUICastleWindow::RenderCastleItem(int nPosX, int nPosY, LPPMSG_NPCDBLIST pInfo)
{
	const int nHPBlockSize = 24 / (g_SenatusInfo.GetMaxHPLevel()+1);
	const int nDefenseBlockSize = 24 / (g_SenatusInfo.GetMaxDefenseLevel()+1);
	const int nRecoverBlockSize = 24 / (g_SenatusInfo.GetMaxRecoverLevel()+1);

	if( g_SenatusInfo.IsGate(pInfo) )	// 성문
	{
		glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

		// 구입한 성문의 업그레이드 정도를 보여준다
		if( pInfo->btNpcLive )
		{
			int nHP = g_SenatusInfo.GetHPLevel( pInfo );
			int nDefense = g_SenatusInfo.GetDefenseLevel( pInfo );
			float fHPRate = pInfo->iNpcHp / (float)pInfo->iNpcMaxHp;

            DisableAlphaBlend();

			RenderColor( nPosX, nPosY-10, nHPBlockSize*(nHP+1), 3 );			// 내구도 최대량표시 (업그레이드된만큼)
			RenderColor( nPosX, nPosY-5, 24, 3 );								// 방어력 최대레벨표시
			glColor4f( 1.0f, 0.0f, 0.0f, 1.0f );
			RenderColor( nPosX, nPosY-10, (nHPBlockSize*(nHP+1))*fHPRate, 3 );	// 내구도 표시
			glColor4f( 0.0f, 0.0f, 0.0f, 1.0f );
			RenderColor( nPosX, nPosY-10, 24, 1 );								// 내구도 레벨 바탕 표시
			RenderColor( nPosX, nPosY-7, 24, 1 );								// 내구도 레벨 바탕 표시
			RenderColor( nPosX, nPosY-10, 1, 3 );								// 내구도 레벨 바탕 표시
			RenderColor( nPosX+24, nPosY-10, 1, 3 );							// 내구도 레벨 바탕 표시
			glColor4f( 0.0f, 1.0f, 0.0f, 1.0f );
			RenderColor( nPosX, nPosY-5, nDefenseBlockSize*(nDefense+1), 3 );	// 방어력레벨 표시
			EndRenderColor();
		}

		// 현재선택된건지 표시
		if( pInfo->iNpcIndex == g_SenatusInfo.GetCurrGate()+1 )
			RenderBitmap( BITMAP_INTERFACE_EX+37, nPosX, nPosY, 24.f, 24.f, 0.f, 0.f, 24.f/32.f, 24.f/32.f );
		else
			RenderBitmap( BITMAP_INTERFACE_EX+36, nPosX, nPosY, 24.f, 24.f, 0.f, 0.f, 24.f/32.f, 24.f/32.f );
	}
	if( g_SenatusInfo.IsStatue(pInfo) )	// 석상
	{
		glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

		// 구입한 석상의 업그레이드 정도를 보여준다
		if( pInfo->btNpcLive )
		{
			int nHP = g_SenatusInfo.GetHPLevel( pInfo );
			int nDefense = g_SenatusInfo.GetDefenseLevel( pInfo );
			int nRecover = g_SenatusInfo.GetRecoverLevel( pInfo );
			float fHPRate = pInfo->iNpcHp / (float)pInfo->iNpcMaxHp;

            DisableAlphaBlend();

			RenderColor( nPosX, nPosY-15, nHPBlockSize*(nHP+1), 3 );			// 내구도 최대량표시 (업그레이드된만큼)
			RenderColor( nPosX, nPosY-10, 24, 3 );								// 방어력 최대레벨표시
			RenderColor( nPosX, nPosY-5, 24, 3 );								// 회복력 최대레벨표시
			glColor4f( 1.0f, 0.0f, 0.0f, 1.0f );
			RenderColor( nPosX, nPosY-15, (nHPBlockSize*(nHP+1))*fHPRate, 3 );	// 내구도 표시
			glColor4f( 0.0f, 1.0f, 0.0f, 1.0f );
			RenderColor( nPosX, nPosY-10, nDefenseBlockSize*(nDefense+1), 3 );	// 방어력레벨 표시
			glColor4f( 1.0f, 1.0f, 0.0f, 1.0f );
			RenderColor( nPosX, nPosY-5, nRecoverBlockSize*(nRecover+1), 3 );	// 회복력레벨 표시
			EndRenderColor();
		}

		// 현재선택된건지 표시
		if( pInfo->iNpcIndex == g_SenatusInfo.GetCurrStatue()+1 )
			RenderBitmap( BITMAP_INTERFACE_EX+39, nPosX, nPosY, 24.f, 24.f, 0.f, 0.f, 24.f/32.f, 24.f/32.f );
		else
			RenderBitmap( BITMAP_INTERFACE_EX+38, nPosX, nPosY, 24.f, 24.f, 0.f, 0.f, 24.f/32.f, 24.f/32.f );
	}
	EnableAlphaTest();
}

void CNewUICastleWindow::InsertComma(unicode::t_char * pszText, DWORD dwNumber)
{
	unicode::t_char szNumber[32];
	unicode::_sprintf(szNumber, "%d", dwNumber);

	unicode::t_char szTemp[256];
	unicode::_strncpy(szTemp, pszText, 256);
	unicode::t_char * pszTextBegin = szTemp;
	unicode::t_char * pszTextFound = unicode::_strstr(szTemp, szNumber);
	unicode::t_char * pszTextNext = pszTextFound + unicode::_strlen(szNumber);
	*pszTextFound = '\0';
	ConvertGold(dwNumber, szNumber);

	unicode::_sprintf(pszText, "%s%s%s", pszTextBegin, szNumber, pszTextNext);
}

#ifdef YDG_FIX_CATLE_MONEY_INT64_TYPE_CRASH
void CNewUICastleWindow::InsertComma64(unicode::t_char * pszText, __int64 iNumber)
{
	unicode::t_char szNumber[32];
	unicode::_sprintf(szNumber, "%I64d", iNumber);

	unicode::t_char szTemp[256];
	unicode::_strncpy(szTemp, pszText, 256);
	unicode::t_char * pszTextBegin = szTemp;
	unicode::t_char * pszTextFound = unicode::_strstr(szTemp, szNumber);
	unicode::t_char * pszTextNext = pszTextFound + unicode::_strlen(szNumber);
	*pszTextFound = '\0';
	ConvertGold64(iNumber, szNumber);

	unicode::_sprintf(pszText, "%s%s%s", pszTextBegin, szNumber, pszTextNext);
}
#endif	// YDG_FIX_CATLE_MONEY_INT64_TYPE_CRASH
