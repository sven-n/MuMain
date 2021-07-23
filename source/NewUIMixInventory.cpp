// NewUIMixInventory.cpp: implementation of the CNewUIMixInventory class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIMixInventory.h"
#include "NewUISystem.h"
#include "NewUICustomMessageBox.h"
#include "MixMgr.h"
#include "ZzzBMD.h"
#include "ZzzEffect.h"
#include "ZzzObject.h"
#include "ZzzInventory.h"
#include "ZzzInterface.h"
#include "ZzzInfomation.h"
#include "ZzzCharacter.h"
#include "wsclientinline.h"
#include "DSPlaySound.h"
#ifdef ADD_SOCKET_MIX
#include "SocketSystem.h"
#endif	// ADD_SOCKET_MIX

using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNewUIMixInventory::CNewUIMixInventory() 
{
	m_pNewUIMng = NULL;
	m_pNewInventoryCtrl = NULL;
	m_Pos.x = m_Pos.y = 0;
	m_iMixState = MIX_READY;
	m_iMixEffectTimer = 0;
}
CNewUIMixInventory::~CNewUIMixInventory() { Release(); }

bool CNewUIMixInventory::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if(NULL == pNewUIMng || NULL == g_pNewUI3DRenderMng || NULL == g_pNewItemMng)
		return false;

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_MIXINVENTORY, this);

	m_pNewInventoryCtrl = new CNewUIInventoryCtrl;
	if(false == m_pNewInventoryCtrl->Create(g_pNewUI3DRenderMng, g_pNewItemMng, this, x+15, y+110, 8, 4))
	{
		SAFE_DELETE(m_pNewInventoryCtrl);
		return false;
	}

	SetPos(x, y);

	LoadImages();

#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	POINT ptBtn = { m_Pos.x+INVENTORY_WIDTH*0.5f-22.f, m_Pos.y+380 };
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	m_BtnMix.ChangeButtonImgState(true, IMAGE_MIXINVENTORY_MIXBTN, false);
	m_BtnMix.ChangeButtonInfo(m_Pos.x+INVENTORY_WIDTH*0.5f-22.f, m_Pos.y+380, 44.f, 35.f);
	m_BtnMix.ChangeToolTipText(GlobalText[591], true); // 조합

	m_pNewInventoryCtrl->GetSquareColorNormal(m_fInventoryColor);
	m_pNewInventoryCtrl->GetSquareColorWarning(m_fInventoryWarningColor);

	Show(false);

	return true;
}
void CNewUIMixInventory::Release()
{
	UnloadImages();

	SAFE_DELETE(m_pNewInventoryCtrl);

	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}
	if(g_pNewUI3DRenderMng)
		g_pNewUI3DRenderMng->DeleteUI2DEffectObject(UI2DEffectCallback);
}

void CNewUIMixInventory::SetMixState(int iMixState)
{
	m_iMixState = iMixState;

	if (iMixState == MIX_REQUESTED)
	{
		// 이펙트 ON!
		m_iMixEffectTimer = 50;

		m_pNewInventoryCtrl->LockInventory();
		g_pMyInventory->GetInventoryCtrl()->LockInventory();
#ifdef YDG_FIX_LOCK_MIX_BUTTON_WHILE_MIXING
		m_BtnMix.Lock();
#endif	// YDG_FIX_LOCK_MIX_BUTTON_WHILE_MIXING
	}
	else
	{
		m_pNewInventoryCtrl->UnlockInventory();
		g_pMyInventory->GetInventoryCtrl()->UnlockInventory();
#ifdef YDG_FIX_LOCK_MIX_BUTTON_WHILE_MIXING
		m_BtnMix.UnLock();
#endif	// YDG_FIX_LOCK_MIX_BUTTON_WHILE_MIXING
	}
}

bool CNewUIMixInventory::InsertItem(int iIndex, BYTE* pbyItemPacket)
{
	if(m_pNewInventoryCtrl)
		return m_pNewInventoryCtrl->AddItem(iIndex, pbyItemPacket);
	return false;
}

void CNewUIMixInventory::DeleteItem(int iIndex)
{
	if(m_pNewInventoryCtrl)
	{
		ITEM* pItem = m_pNewInventoryCtrl->FindItem(iIndex);
		if(pItem != NULL)
			m_pNewInventoryCtrl->RemoveItem(pItem);
	}
}

void CNewUIMixInventory::DeleteAllItems()
{
	if(m_pNewInventoryCtrl)
		m_pNewInventoryCtrl->RemoveAllItems();
}

void CNewUIMixInventory::OpeningProcess()
{
	g_MixRecipeMgr.SetPlusChaosRate(0);
	SendRequestCrywolfBenefitPlusChaosRate();

	SetMixState(SEASON3B::CNewUIMixInventory::MIX_READY);
	
	if (g_MixRecipeMgr.GetMixInventoryType() == SEASON3A::MIXTYPE_GOBLIN_NORMAL)
	{
		SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CChaosMixMenuMsgBoxLayout));
	}
}

bool CNewUIMixInventory::ClosingProcess()
{
 	if (g_pMixInventory->GetInventoryCtrl()->GetNumberOfItems() > 0 || CNewUIInventoryCtrl::GetPickedItem() != NULL)
 	{
		// 조합창에 아이템이 있으면 창을 닫을 수 없다
		g_pChatListBox->AddText("", GlobalText[593], SEASON3B::TYPE_ERROR_MESSAGE);
 		return false;
 	}
	else
	{
		switch (g_MixRecipeMgr.GetMixInventoryType())
		{
		case SEASON3A::MIXTYPE_GOBLIN_NORMAL:
		case SEASON3A::MIXTYPE_GOBLIN_CHAOSITEM:
		case SEASON3A::MIXTYPE_GOBLIN_ADD380:
			SendRequestMixExit();
			break;
		case SEASON3A::MIXTYPE_CASTLE_SENIOR:
			SendRequestMixExit();
			break;
		case SEASON3A::MIXTYPE_TRAINER:
			SendExitInventory();
			break;
		case SEASON3A::MIXTYPE_OSBOURNE:
		case SEASON3A::MIXTYPE_JERRIDON:
		case SEASON3A::MIXTYPE_ELPIS:
			SendRequestMixExit();
			break;
#if defined PSW_NEW_CHAOS_CARD || defined PBG_FIX_CHAOSCARD
		case SEASON3A::MIXTYPE_CHAOS_CARD:
			SendRequestMixExit();
			break;
#endif	// defined PSW_NEW_CHAOS_CARD || defined PBG_FIX_CHAOSCARD
#ifdef CSK_EVENT_CHERRYBLOSSOM
		case SEASON3A::MIXTYPE_CHERRYBLOSSOM:
			SendRequestMixExit();
			break;
#endif	// CSK_EVENT_CHERRYBLOSSOM
#ifdef ADD_SOCKET_MIX
		case SEASON3A::MIXTYPE_ATTACH_SOCKET:
		case SEASON3A::MIXTYPE_DETACH_SOCKET:
			m_SocketListBox.Clear();
		case SEASON3A::MIXTYPE_EXTRACT_SEED:
		case SEASON3A::MIXTYPE_SEED_SPHERE:
			SendRequestMixExit();
			break;
#endif	// ADD_SOCKET_MIX
		default:
			break;
		}
		g_pMixInventory->DeleteAllItems();
		g_MixRecipeMgr.ClearCheckRecipeResult();	// 닫을때 조합 검사 결과를 초기화 해준다.
		return true;
	}
}

void CNewUIMixInventory::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
}

bool CNewUIMixInventory::UpdateMouseEvent()
{
	if(m_pNewInventoryCtrl && false == m_pNewInventoryCtrl->UpdateMouseEvent())
		return false;
	
	if(true == InventoryProcess())
		return false;

	//. 버튼 처리
	if(true == BtnProcess())	//. 처리가 완료 되었다면
		return false;

	//. 인벤토리 내의 영역 클릭시 하위 UI처리 및 이동 불가
	if(CheckMouseIn(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT))
	{
		if(SEASON3B::IsPress(VK_RBUTTON))
		{
			MouseRButton = false;
			MouseRButtonPop = false;
			MouseRButtonPush = false;
			return false;
		}

		if(SEASON3B::IsNone(VK_LBUTTON) == false)
		{
			return false;
		}
	}

	return true;
}
bool CNewUIMixInventory::UpdateKeyEvent()
{
	return true;
}
bool CNewUIMixInventory::Update()
{
	if(m_pNewInventoryCtrl && false == m_pNewInventoryCtrl->Update())
		return false;

	if (IsVisible())
	{
		CheckMixInventory();
#ifdef ADD_SOCKET_MIX
		switch (g_MixRecipeMgr.GetMixInventoryType())
		{
		case SEASON3A::MIXTYPE_ATTACH_SOCKET:
		case SEASON3A::MIXTYPE_DETACH_SOCKET:
			{
				if (m_SocketListBox.GetLineNum() == 0)
				{
					char szText[64] = { 0, };
					char szSocketText[64] = { 0, };
					for (int i = 0; i < g_MixRecipeMgr.GetFirstItemSocketCount(); ++i)
					{
						if (g_MixRecipeMgr.GetFirstItemSocketSeedID(i) == SOCKET_EMPTY)
						{
							sprintf(szSocketText, GlobalText[2652]);	// "장착아이템 없음"
						}
						else
						{
 							g_SocketItemMgr.CreateSocketOptionText(szSocketText, g_MixRecipeMgr.GetFirstItemSocketSeedID(i), g_MixRecipeMgr.GetFirstItemSocketShpereLv(i));
						}
						sprintf(szText, "%d: %s", i + 1, szSocketText);
						m_SocketListBox.AddText(i, szText);
					}
					m_SocketListBox.SLSetSelectLine(0);
				}
				else
				{
					if (g_MixRecipeMgr.GetFirstItemSocketCount() == 0)
					{
						// 제거
						m_SocketListBox.Clear();
					}
				}
				m_SocketListBox.DoAction();
			}
			break;
		}
#endif //ADD_SOCKET_MIX
	}

	return true;
}
bool CNewUIMixInventory::Render()
{
	EnableAlphaTest();
	glColor4f(1.f, 1.f, 1.f, 1.f);
	
	RenderFrame();

	if(m_pNewInventoryCtrl)
		m_pNewInventoryCtrl->Render();

	// 조합 효과
	if (GetMixState() >= MIX_REQUESTED && g_pNewUI3DRenderMng)
		g_pNewUI3DRenderMng->RenderUI2DEffect(INVENTORY_CAMERA_Z_ORDER, UI2DEffectCallback, this, 0, 0);

	DisableAlphaBlend();

	return true;
}

float CNewUIMixInventory::GetLayerDepth()
{ return 3.4f; }

CNewUIInventoryCtrl* CNewUIMixInventory::GetInventoryCtrl() const
{ return m_pNewInventoryCtrl; }

void CNewUIMixInventory::UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB)
{
	if(pClass)
	{
		CNewUIMixInventory* pMixInventory = (CNewUIMixInventory*)pClass;
		pMixInventory->RenderMixEffect();
	}
}

void CNewUIMixInventory::LoadImages()
{
	LoadBitmap("Interface\\newui_msgbox_back.jpg", IMAGE_MIXINVENTORY_BACK, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back04.tga", IMAGE_MIXINVENTORY_TOP, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-L.tga", IMAGE_MIXINVENTORY_LEFT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-R.tga", IMAGE_MIXINVENTORY_RIGHT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back03.tga", IMAGE_MIXINVENTORY_BOTTOM, GL_LINEAR);
	LoadBitmap("Interface\\newui_bt_mix.tga", IMAGE_MIXINVENTORY_MIXBTN, GL_LINEAR);

#ifdef ADD_SOCKET_MIX
	LoadBitmap("Interface\\newui_scrollbar_up.tga", CNewUIGuardWindow::IMAGE_GUARDWINDOW_SCROLL_TOP);
	LoadBitmap("Interface\\newui_scrollbar_m.tga", CNewUIGuardWindow::IMAGE_GUARDWINDOW_SCROLL_MIDDLE);
	LoadBitmap("Interface\\newui_scrollbar_down.tga", CNewUIGuardWindow::IMAGE_GUARDWINDOW_SCROLL_BOTTOM);
	LoadBitmap("Interface\\newui_scroll_on.tga", CNewUIGuardWindow::IMAGE_GUARDWINDOW_SCROLLBAR_ON, GL_LINEAR);
	LoadBitmap("Interface\\newui_scroll_off.tga", CNewUIGuardWindow::IMAGE_GUARDWINDOW_SCROLLBAR_OFF, GL_LINEAR);
#endif	// ADD_SOCKET_MIX
}
void CNewUIMixInventory::UnloadImages()
{
	DeleteBitmap(IMAGE_MIXINVENTORY_BOTTOM);
	DeleteBitmap(IMAGE_MIXINVENTORY_RIGHT);
	DeleteBitmap(IMAGE_MIXINVENTORY_LEFT);
	DeleteBitmap(IMAGE_MIXINVENTORY_TOP);
	DeleteBitmap(IMAGE_MIXINVENTORY_BACK);
	DeleteBitmap(IMAGE_MIXINVENTORY_MIXBTN);

#ifdef ADD_SOCKET_MIX
	DeleteBitmap(CNewUIGuardWindow::IMAGE_GUARDWINDOW_SCROLL_TOP);
	DeleteBitmap(CNewUIGuardWindow::IMAGE_GUARDWINDOW_SCROLL_MIDDLE);
	DeleteBitmap(CNewUIGuardWindow::IMAGE_GUARDWINDOW_SCROLL_BOTTOM);
	DeleteBitmap(CNewUIGuardWindow::IMAGE_GUARDWINDOW_SCROLLBAR_ON);
	DeleteBitmap(CNewUIGuardWindow::IMAGE_GUARDWINDOW_SCROLLBAR_OFF);
#endif	// ADD_SOCKET_MIX
}

void CNewUIMixInventory::RenderFrame()
{
	RenderImage(IMAGE_MIXINVENTORY_BACK, m_Pos.x, m_Pos.y, 190.f, 429.f);
	RenderImage(IMAGE_MIXINVENTORY_TOP, m_Pos.x, m_Pos.y, 190.f, 64.f);
	RenderImage(IMAGE_MIXINVENTORY_LEFT, m_Pos.x, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_MIXINVENTORY_RIGHT, m_Pos.x+INVENTORY_WIDTH-21, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_MIXINVENTORY_BOTTOM, m_Pos.x, m_Pos.y+INVENTORY_HEIGHT-45, 190.f, 45.f);

	unicode::t_char szText[256] = { 0, };
	float fPos_x = m_Pos.x + 15.0f, fPos_y = m_Pos.y;
	float fLine_y = 13.0f;

	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetTextColor(220, 220, 220, 255);
	g_pRenderText->SetBgColor(0, 0, 0, 0);

	// 조합창 제목 표시
	switch (g_MixRecipeMgr.GetMixInventoryType())
	{
	case SEASON3A::MIXTYPE_GOBLIN_NORMAL:
		unicode::_sprintf(szText, "%s", GlobalText[735]);
		break;
	case SEASON3A::MIXTYPE_GOBLIN_CHAOSITEM:
		unicode::_sprintf(szText, "%s", GlobalText[736]);
		break;
	case SEASON3A::MIXTYPE_GOBLIN_ADD380:
		unicode::_sprintf(szText, "%s", GlobalText[2193]);
		break;
	case SEASON3A::MIXTYPE_CASTLE_SENIOR:
		fLine_y += 5.0f;
		unicode::_sprintf(szText, "%s", GlobalText[1640]);
		break;
	case SEASON3A::MIXTYPE_TRAINER:
		unicode::_sprintf(szText, "%s", GlobalText[1205]);
		break;
	case SEASON3A::MIXTYPE_OSBOURNE:
		fLine_y += 5.0f;
		unicode::_sprintf(szText, "%s", GlobalText[2061]);
		break;
	case SEASON3A::MIXTYPE_JERRIDON:
		fLine_y += 5.0f;
		unicode::_sprintf(szText, "%s", GlobalText[2062]);
		break;
	case SEASON3A::MIXTYPE_ELPIS:
		fLine_y += 5.0f;
		unicode::_sprintf(szText, "%s", GlobalText[2063]);
		break;
#ifdef PSW_NEW_CHAOS_CARD
	case SEASON3A::MIXTYPE_CHAOS_CARD:
		fLine_y += 5.0f;
		unicode::_sprintf(szText, "%s", GlobalText[2265]);
		break;
#endif	// PSW_NEW_CHAOS_CARD
#ifdef CSK_EVENT_CHERRYBLOSSOM
	case SEASON3A::MIXTYPE_CHERRYBLOSSOM:
		fLine_y += 5.0f;
		unicode::_sprintf(szText, "%s", GlobalText[2563]);
		break;
#endif	// CSK_EVENT_CHERRYBLOSSOM
#ifdef ADD_SOCKET_MIX
	case SEASON3A::MIXTYPE_EXTRACT_SEED:
		fLine_y += 5.0f;
		unicode::_sprintf(szText, "%s", GlobalText[2660]);	// "추 출"
		break;
	case SEASON3A::MIXTYPE_SEED_SPHERE:
		fLine_y += 5.0f;
		unicode::_sprintf(szText, "%s", GlobalText[2661]);	// "합 성"
		break;
	case SEASON3A::MIXTYPE_ATTACH_SOCKET:
		fLine_y += 5.0f;
		unicode::_sprintf(szText, "%s", GlobalText[2662]);	// "장 착"
		break;
	case SEASON3A::MIXTYPE_DETACH_SOCKET:
		fLine_y += 5.0f;
		unicode::_sprintf(szText, "%s", GlobalText[2663]);	// "파 괴"
		break;
#endif	// ADD_SOCKET_MIX
	default:
		fLine_y += 5.0f;
		unicode::_sprintf(szText, "%s", GlobalText[583]);
		break;
	}
	g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y, szText, 160.0f, 0, RT3_SORT_CENTER);

	// 세율 표시
	fLine_y += 12;
	switch (g_MixRecipeMgr.GetMixInventoryType())
	{
	case SEASON3A::MIXTYPE_GOBLIN_NORMAL:
	case SEASON3A::MIXTYPE_GOBLIN_CHAOSITEM:
	case SEASON3A::MIXTYPE_GOBLIN_ADD380:
	case SEASON3A::MIXTYPE_TRAINER:
		unicode::_sprintf(szText, GlobalText[1623], g_nChaosTaxRate );
		g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y, szText, 160.0f, 0, RT3_SORT_CENTER);
		break;
	default:
		fLine_y -= 5;
		break;
	}

	if (GetMixState() == MIX_FINISHED)	// 조합창 이름까지만 출력
	{
		return;
	}

	// 조합 이름 표시 (최대 2줄)
	fLine_y += 24;
 	if (!g_MixRecipeMgr.IsReadyToMix())
		g_pRenderText->SetTextColor(255, 48, 48, 255);
	else
		g_pRenderText->SetTextColor(255, 255, 48, 255);
	g_pRenderText->SetBgColor(40, 40, 40, 128);

	g_MixRecipeMgr.GetCurRecipeName(szText, 1);
	g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y, szText);
	fLine_y += 10;
	if (g_MixRecipeMgr.GetCurRecipeName(szText, 2))	// 2번째 줄이 있으면 출력해준다.
		g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y, szText);
	fLine_y += 10;

	// 조합 성공 확률 표시
	switch (g_MixRecipeMgr.GetMixInventoryType())
	{
	case SEASON3A::MIXTYPE_GOBLIN_NORMAL:
	case SEASON3A::MIXTYPE_GOBLIN_CHAOSITEM:
	case SEASON3A::MIXTYPE_GOBLIN_ADD380:
	case SEASON3A::MIXTYPE_OSBOURNE:
	case SEASON3A::MIXTYPE_ELPIS:
	case SEASON3A::MIXTYPE_TRAINER:
#ifdef ADD_SOCKET_MIX
	case SEASON3A::MIXTYPE_EXTRACT_SEED:
	case SEASON3A::MIXTYPE_SEED_SPHERE:
#endif	// ADD_SOCKET_MIX
		if (g_MixRecipeMgr.IsReadyToMix() &&
			g_MixRecipeMgr.GetPlusChaosRate() > 0 && g_MixRecipeMgr.GetCurRecipe()->m_bMixOption == 'F')
		{
			g_pRenderText->SetTextColor(255, 255, 48, 255);
			g_pRenderText->SetBgColor(40, 40, 40, 128);
			unicode::_sprintf(szText, GlobalText[584],GlobalText[591],g_MixRecipeMgr.GetSuccessRate());
			unicode::_sprintf(szText, "%s + %d%%", szText, g_MixRecipeMgr.GetPlusChaosRate());
			g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y, szText);
			g_pRenderText->SetTextColor(210, 230, 255, 255);
		}
		else
		{
			g_pRenderText->SetTextColor(210, 230, 255, 255);
			g_pRenderText->SetBgColor(40, 40, 40, 128);
			switch (g_MixRecipeMgr.GetMixInventoryType())
			{
			case SEASON3A::MIXTYPE_GOBLIN_NORMAL:
			case SEASON3A::MIXTYPE_GOBLIN_CHAOSITEM:
			case SEASON3A::MIXTYPE_GOBLIN_ADD380:
#ifdef ADD_SOCKET_MIX
			case SEASON3A::MIXTYPE_EXTRACT_SEED:
			case SEASON3A::MIXTYPE_SEED_SPHERE:
#endif	// ADD_SOCKET_MIX
				unicode::_sprintf(szText,GlobalText[584],GlobalText[591],g_MixRecipeMgr.GetSuccessRate());
				break;
			case SEASON3A::MIXTYPE_TRAINER:
				unicode::_sprintf(szText,GlobalText[584],GlobalText[1212],g_MixRecipeMgr.GetSuccessRate());
				break;
			case SEASON3A::MIXTYPE_OSBOURNE:
				unicode::_sprintf(szText,GlobalText[584],GlobalText[2061],g_MixRecipeMgr.GetSuccessRate());
				break;
			case SEASON3A::MIXTYPE_ELPIS:
				unicode::_sprintf(szText,GlobalText[584],GlobalText[2063],g_MixRecipeMgr.GetSuccessRate());
				break;
			}
			g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y, szText);
		}
		fLine_y += 20;
		break;
	}

	// 조합시 필요한 젠 표시
	switch (g_MixRecipeMgr.GetMixInventoryType())
	{
	case SEASON3A::MIXTYPE_GOBLIN_NORMAL:
	case SEASON3A::MIXTYPE_GOBLIN_CHAOSITEM:
	case SEASON3A::MIXTYPE_GOBLIN_ADD380:
	case SEASON3A::MIXTYPE_CASTLE_SENIOR:
#ifdef YDG_FIX_TRAINER_MIX_DISPLAY_REQUIRED_ZEN
	case SEASON3A::MIXTYPE_TRAINER:
#endif	// YDG_FIX_TRAINER_MIX_DISPLAY_REQUIRED_ZEN
	case SEASON3A::MIXTYPE_JERRIDON:
#ifdef ADD_SOCKET_MIX
	case SEASON3A::MIXTYPE_EXTRACT_SEED:
	case SEASON3A::MIXTYPE_SEED_SPHERE:
	case SEASON3A::MIXTYPE_ATTACH_SOCKET:
	case SEASON3A::MIXTYPE_DETACH_SOCKET:
#endif	// ADD_SOCKET_MIX
		{
			unicode::t_char szGoldText[32];
			unicode::t_char szGoldText2[32];

			ConvertGold(g_MixRecipeMgr.GetReqiredZen(),szGoldText);
			ConvertChaosTaxGold(g_MixRecipeMgr.GetReqiredZen(),szGoldText2);
			if (g_MixRecipeMgr.IsReadyToMix() && g_MixRecipeMgr.GetCurRecipe()->m_bRequiredZenType == 'C') //  축복,영혼의 물약은 비용 표시 단위가 다르다
			{
#if SELECTED_LANGUAGE==LANGUAGE_PHILIPPINES
				// 글로벌, 필리핀
    			unicode::_sprintf(szText,GlobalText[1636]);
#else // SELECTED_LANGUAGE==LANGUAGE_PHILIPPINES
				// 한국
    			unicode::_sprintf(szText,GlobalText[1636],szGoldText2,szGoldText);
#endif // SELECTED_LANGUAGE==LANGUAGE_PHILIPPINES
			}
			else
			{
				unicode::_sprintf(szText,GlobalText[1622],szGoldText2,szGoldText);
			}

			g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y, szText);
		}
		fLine_y += 20;
		break;
	}

	// 조합 가이드 출력 (예상조합+조합재료)
	fLine_y = 203;
	int iTextPos_y = 0;			// Render한 다음 개행 규칙
	if (g_MixRecipeMgr.GetMostSimilarRecipe() != NULL)	// 유사 조합을 찾았으면
	{
		// 예상조합 이름을 출력
		g_pRenderText->SetTextColor(220, 220, 220, 255);
		g_pRenderText->SetBgColor(40, 40, 40, 128);
		
		unicode::t_char szTempText[2][100];
		int iTextLines = 0;
		if (!g_MixRecipeMgr.IsReadyToMix() && g_MixRecipeMgr.GetMostSimilarRecipeName(szTempText[0], 1) == TRUE)
		{
			unicode::_sprintf(szText, GlobalText[2334], szTempText[0]);	// "예상 조합: %s"
#ifdef KJH_FIX_BTS158_TEXT_CUT_ROUTINE
			iTextLines = CutStr(szText, szTempText[0], 150, 2, 100);
			
			for(int i=0 ; i<iTextLines ; i++)
			{
				if( i >= 2 )
					break;

				g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y + (iTextPos_y*15), szTempText[i]);
				iTextPos_y++;
			}
#else // KJH_FIX_BTS158_TEXT_CUT_ROUTINE
			iTextLines = CutText3(szText, szTempText[0], 150, 2, 100);

			g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y, szTempText[0]);
			if (iTextLines == 2)
				g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y+(++iTextPos_y)*15, szTempText[1]);
#endif // KJH_FIX_BTS158_TEXT_CUT_ROUTINE


		}
		// 예상조합 재료를 출력
		int iResult;
		for (int iLine = 0; iLine < 8; ++iLine)
		{
			iResult = g_MixRecipeMgr.GetSourceName(iLine, szText);
			if (iResult == SEASON3A::MIX_SOURCE_ERROR) break;
			else if (iResult == SEASON3A::MIX_SOURCE_NO) g_pRenderText->SetTextColor(255, 50, 20, 255);
			else if (iResult == SEASON3A::MIX_SOURCE_PARTIALLY) g_pRenderText->SetTextColor(210, 230, 255, 255);
			else if (iResult == SEASON3A::MIX_SOURCE_YES) g_pRenderText->SetTextColor(255, 255, 48, 255);
			
#ifdef KJH_FIX_BTS158_TEXT_CUT_ROUTINE
			iTextLines = CutStr(szText, szTempText[0], 156, 2, 100);
			
			for(int i=0 ; i<iTextLines ; i++)
			{
				if( i >= 2 )
					break;
				
				g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y + (iTextPos_y*15), szTempText[i]);
				iTextPos_y++;
			}
#else // KJH_FIX_BTS158_TEXT_CUT_ROUTINE
			iTextLines = CutText3(szText, szTempText[0], 156, 2, 100);

			g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y+(++iTextPos_y)*15, szTempText[0]);
			if (iTextLines == 2)
				g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y+(++iTextPos_y)*15, szTempText[1]);
#endif // KJH_FIX_BTS158_TEXT_CUT_ROUTINE
			
		}
	}
	else if (g_MixRecipeMgr.IsMixInit())	// 조합창에 아무 아이템도 올리지 않았다면
	{
		g_pRenderText->SetTextColor(255, 50, 20, 255);
		g_pRenderText->SetBgColor(40, 40, 40, 128);
		
		unicode::_sprintf(szText, GlobalText[2346]);	// "조합 재료를 올려주세요."
		g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y, szText);
#ifdef KJH_FIX_BTS158_TEXT_CUT_ROUTINE
		iTextPos_y++;
#endif // KJH_FIX_BTS158_TEXT_CUT_ROUTINE
		
	}
	else	// 조합할 수 없는 아이템들이 올라왔을때
	{
		g_pRenderText->SetTextColor(255, 50, 20, 255);
		g_pRenderText->SetBgColor(40, 40, 40, 128);

#ifdef LDK_FIX_CHAR_NUMBER_OVER
		//해외 글자수가 너무 길어서 두줄로 나눔(2008.08.11)
		unicode::_sprintf(szText, GlobalText[2334]," ");	// "예상 조합:"
		g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y, szText);

		unicode::_sprintf(szText, GlobalText[601]);	// "조합불가"
		g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y+(++iTextPos_y)*15, szText);

#else //LDK_FIX_CHAR_NUMBER_OVER

		unicode::_sprintf(szText, GlobalText[2334], GlobalText[601]);	// "예상 조합: %s" 조합불가
		g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y, szText);
		
#endif //LDK_FIX_CHAR_NUMBER_OVER	
	}
	
	// 조합 설명 출력
	++iTextPos_y;
	g_pRenderText->SetTextColor(255, 50, 20, 255);
	g_pRenderText->SetBgColor(40, 40, 40, 128);
	
	if (g_MixRecipeMgr.IsReadyToMix())	// 조합 가능 상태에는 스크립트의 조합 설명을 출력
	{
		if (g_MixRecipeMgr.GetCurRecipeDesc(szText, 1) == TRUE)
			g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y+(++iTextPos_y)*15, szText);
		if (g_MixRecipeMgr.GetCurRecipeDesc(szText, 2) == TRUE)
			g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y+(++iTextPos_y)*15, szText);
		if (g_MixRecipeMgr.GetCurRecipeDesc(szText, 3) == TRUE)
			g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y+(++iTextPos_y)*15, szText);
		
	}
	else if (g_MixRecipeMgr.GetMostSimilarRecipe() != NULL)	// 유사 조합을 찾았으면 조합 가이드 출력
	{
		// 예상조합의 설명 출력
		if (g_MixRecipeMgr.GetRecipeAdvice(szText, 1) == TRUE)
			g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y+(++iTextPos_y)*15, szText);
		if (g_MixRecipeMgr.GetRecipeAdvice(szText, 2) == TRUE)
			g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y+(++iTextPos_y)*15, szText);
		if (g_MixRecipeMgr.GetRecipeAdvice(szText, 3) == TRUE)
			g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y+(++iTextPos_y)*15, szText);
		
	}
		
	RenderMixDescriptions(fPos_x, fPos_y);

	// 조합 버튼 표시
	switch (g_MixRecipeMgr.GetMixInventoryType())
	{
	case SEASON3A::MIXTYPE_TRAINER:
		m_BtnMix.ChangeToolTipText(GlobalText[1212], true);
		break;
	case SEASON3A::MIXTYPE_OSBOURNE:
		m_BtnMix.ChangeToolTipText(GlobalText[2061], true);
		break;
	case SEASON3A::MIXTYPE_JERRIDON:
		m_BtnMix.ChangeToolTipText(GlobalText[2062], true);
		break;
	case SEASON3A::MIXTYPE_ELPIS:
		m_BtnMix.ChangeToolTipText(GlobalText[2063], true);
		break;
#ifdef ADD_SOCKET_MIX
	case SEASON3A::MIXTYPE_EXTRACT_SEED:
		m_BtnMix.ChangeToolTipText(GlobalText[2660], true);	// "추 출"
		break;
	case SEASON3A::MIXTYPE_SEED_SPHERE:
		m_BtnMix.ChangeToolTipText(GlobalText[2661], true);	// "합 성"
		break;
	case SEASON3A::MIXTYPE_ATTACH_SOCKET:
		m_BtnMix.ChangeToolTipText(GlobalText[2662], true);	// "장 착"
		break;
	case SEASON3A::MIXTYPE_DETACH_SOCKET:
		m_BtnMix.ChangeToolTipText(GlobalText[2663], true);	// "파 괴"
		break;
#endif	// ADD_SOCKET_MIX
	default:
		m_BtnMix.ChangeToolTipText(GlobalText[591], true);
		break;
	}
	m_BtnMix.Render();
}

bool CNewUIMixInventory::BtnProcess()
{
	POINT ptExitBtn1 = { m_Pos.x+169, m_Pos.y+7 };

	//. Exit1 버튼 (기본처리)
	if(SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_MIXINVENTORY);
	}

	if (GetMixState() == MIX_FINISHED)	// 조합 버튼 무시
	{
		return false;
	}

	// 조합 버튼
	if(m_BtnMix.UpdateMouseEvent() == true)
	{
		Mix();
		return true;
	}

	return false;
}

void CNewUIMixInventory::RenderMixDescriptions(float fPos_x, float fPos_y)
{
	unicode::t_char szText[256] = { 0, };
	switch (g_MixRecipeMgr.GetMixInventoryType())
	{
	case SEASON3A::MIXTYPE_GOBLIN_NORMAL:
	case SEASON3A::MIXTYPE_GOBLIN_CHAOSITEM:
	case SEASON3A::MIXTYPE_GOBLIN_ADD380:
		break;
	case SEASON3A::MIXTYPE_CASTLE_SENIOR:
		{
			g_pRenderText->SetBgColor(0, 0, 0, 0);
			g_pRenderText->SetTextColor(200, 200, 200, 255);
			for (int i = 0; i < 6; ++i)
				g_pRenderText->RenderText(fPos_x, fPos_y+270+i*13, GlobalText[1644+i], 160.0f, 0, RT3_SORT_CENTER);
		}
		break;
	case SEASON3A::MIXTYPE_TRAINER:
		break;
	case SEASON3A::MIXTYPE_OSBOURNE:
		{
			g_pRenderText->SetBgColor(0, 0, 0, 0);
			g_pRenderText->SetTextColor(255, 255, 255, 255);
			unicode::_sprintf( szText, GlobalText[2220] );
			g_pRenderText->RenderText(fPos_x, fPos_y+250+0*13, szText, 160.0f, 0, RT3_SORT_CENTER);
			unicode::_sprintf( szText, GlobalText[2221] );
			g_pRenderText->RenderText(fPos_x, fPos_y+250+1*13, szText, 160.0f, 0, RT3_SORT_CENTER);
			unicode::_sprintf( szText, GlobalText[2084], GlobalText[2061], GlobalText[2082] );
			g_pRenderText->RenderText(fPos_x, fPos_y+250+2*13, szText, 160.0f, 0, RT3_SORT_CENTER);
			unicode::_sprintf( szText, GlobalText[2098] );
			g_pRenderText->RenderText(fPos_x, fPos_y+250+3*13, szText, 160.0f, 0, RT3_SORT_CENTER);
			g_pRenderText->SetTextColor(255, 0, 0, 255);
			unicode::_sprintf( szText, GlobalText[2222] );
			g_pRenderText->RenderText(fPos_x, fPos_y+250+4*13, szText, 160.0f, 0, RT3_SORT_CENTER);
		}
		break;
	case SEASON3A::MIXTYPE_JERRIDON:
		{
			g_pRenderText->SetBgColor(0, 0, 0, 0);
			g_pRenderText->SetTextColor(255, 255, 255, 255);
			unicode::_sprintf( szText, GlobalText[2102] );
			g_pRenderText->RenderText(fPos_x, fPos_y+250+0*13, szText, 160.0f, 0, RT3_SORT_CENTER);
			unicode::_sprintf( szText, GlobalText[2103] );
			g_pRenderText->RenderText(fPos_x, fPos_y+250+1*13, szText, 160.0f, 0, RT3_SORT_CENTER);
			unicode::_sprintf( szText, GlobalText[2104] );
			g_pRenderText->RenderText(fPos_x, fPos_y+250+2*13, szText, 160.0f, 0, RT3_SORT_CENTER);
			unicode::_sprintf( szText, GlobalText[2088] );
			g_pRenderText->RenderText(fPos_x, fPos_y+250+3*13, szText, 160.0f, 0, RT3_SORT_CENTER);
			unicode::_sprintf( szText, GlobalText[2100] );
			g_pRenderText->RenderText(fPos_x, fPos_y+250+4*13, szText, 160.0f, 0, RT3_SORT_CENTER);
			unicode::_sprintf( szText, GlobalText[2101] );
			g_pRenderText->RenderText(fPos_x, fPos_y+250+5*13, szText, 160.0f, 0, RT3_SORT_CENTER);
		}
		break;
	case SEASON3A::MIXTYPE_ELPIS:
			g_pRenderText->SetBgColor(0, 0, 0, 0);
			g_pRenderText->SetTextColor(255, 255, 255, 255);
			unicode::_sprintf( szText, GlobalText[2071] );
			g_pRenderText->RenderText(fPos_x, fPos_y+250+0*13, szText, 160.0f, 0, RT3_SORT_CENTER);
			unicode::_sprintf( szText, GlobalText[2095] );
			g_pRenderText->RenderText(fPos_x, fPos_y+250+1*13, szText, 160.0f, 0, RT3_SORT_CENTER);
			unicode::_sprintf( szText, GlobalText[2096] );
			g_pRenderText->RenderText(fPos_x, fPos_y+250+2*13, szText, 160.0f, 0, RT3_SORT_CENTER);
		break;
#ifdef PSW_NEW_CHAOS_CARD
	case SEASON3A::MIXTYPE_CHAOS_CARD:
		{
			g_pRenderText->SetBgColor(0, 0, 0, 0);
			
			g_pRenderText->SetTextColor(255, 40, 20, 255);
			unicode::_sprintf( szText, GlobalText[2223] );
			g_pRenderText->RenderText(fPos_x, fPos_y+250+4*13, szText, 160.0f, 0, RT3_SORT_CENTER);
			
			g_pRenderText->SetTextColor(255, 255, 255, 255);
			unicode::_sprintf( szText, GlobalText[2262] );
			g_pRenderText->RenderText(fPos_x-10, fPos_y+250+6*13, szText, 200.0f, 0, RT3_SORT_LEFT);
			
			g_pRenderText->SetTextColor(255, 255, 255, 255);
			unicode::_sprintf( szText, GlobalText[2306] );
			g_pRenderText->RenderText(fPos_x-10, fPos_y+250+7*13, szText, 200.0f, 0, RT3_SORT_LEFT);

			g_pRenderText->SetTextColor(255, 255, 255, 255);
			unicode::_sprintf( szText, GlobalText[2261] );
			g_pRenderText->RenderText(fPos_x-10, fPos_y+250+8*13, szText, 200.0f, 0, RT3_SORT_LEFT);
		}
		break;
#endif	// PSW_NEW_CHAOS_CARD
#ifdef CSK_EVENT_CHERRYBLOSSOM
	case SEASON3A::MIXTYPE_CHERRYBLOSSOM:
		{
			g_pRenderText->SetBgColor(0, 0, 0, 0);

#if SELECTED_LANGUAGE == LANGUAGE_KOREAN
			g_pRenderText->SetTextColor(255, 40, 20, 255);
			unicode::_sprintf( szText, GlobalText[2223] );
			g_pRenderText->RenderText(fPos_x, fPos_y+250+0*13, szText, 160.0f, 0, RT3_SORT_CENTER);
			
			g_pRenderText->SetTextColor(255, 220, 20, 255);
			unicode::_sprintf( szText, GlobalText[2565] );
			g_pRenderText->RenderText(fPos_x-10, fPos_y+250+2*13, szText, 160.0f, 0, RT3_SORT_LEFT);
			
			g_pRenderText->SetTextColor(255, 255, 255, 255);
			unicode::_sprintf( szText, GlobalText[2540] );
			g_pRenderText->RenderText(fPos_x-10, fPos_y+250+3*13, szText, 180.0f, 0, RT3_SORT_LEFT);
			
			g_pRenderText->SetTextColor(255, 255, 255, 255);
			unicode::_sprintf( szText, GlobalText[2306] );
			g_pRenderText->RenderText(fPos_x-10, fPos_y+250+4*13, szText, 200.0f, 0, RT3_SORT_LEFT);
#else //SELECTED_LANGUAGE == LANGUAGE_KOREAN
			g_pRenderText->SetTextColor(255, 40, 20, 255);
			unicode::_sprintf( szText, GlobalText[2223] );
			g_pRenderText->RenderText(fPos_x, fPos_y+250+0*13, szText, 160.0f, 0, RT3_SORT_CENTER);

#if SELECTED_LANGUAGE == LANGUAGE_ENGLISH && defined(LDS_MOD_EVENTCHERRYBLOSSOM_FORENG)
			g_pRenderText->SetTextColor(255, 255, 255, 255);
			unicode::_sprintf( szText, GlobalText[2565] );		// 황금 벚꽃가지 255개
			g_pRenderText->RenderText(fPos_x-10, fPos_y+250+2*13, szText, 160.0f, 0, RT3_SORT_LEFT);
			
			g_pRenderText->SetTextColor(255, 255, 255, 255);
			unicode::_sprintf( szText, GlobalText[2540] );
			g_pRenderText->RenderText(fPos_x-10, fPos_y+250+3*13, szText, 180.0f, 0, RT3_SORT_LEFT);
			
			g_pRenderText->SetTextColor(255, 255, 255, 255);
			unicode::_sprintf( szText, GlobalText[2306] );
			g_pRenderText->RenderText(fPos_x-10, fPos_y+250+4*13, szText, 200.0f, 0, RT3_SORT_LEFT);
#else // SELECTED_LANGUAGE == LANGUAGE_ENGLISH && defined(LDS_MOD_EVENTCHERRYBLOSSOM_FORENG)
			g_pRenderText->SetTextColor(255, 255, 255, 255);
			unicode::_sprintf( szText, GlobalText[2546] );		// 하얀 벚꽃가지 10개
			g_pRenderText->RenderText(fPos_x-10, fPos_y+250+2*13, szText, 160.0f, 0, RT3_SORT_LEFT);
			
			g_pRenderText->SetTextColor(255, 40, 20, 255);
			unicode::_sprintf( szText, GlobalText[2547] );		// 붉은 벚꽃가지 30개
			g_pRenderText->RenderText(fPos_x-10, fPos_y+250+3*13, szText, 160.0f, 0, RT3_SORT_LEFT);
			
			g_pRenderText->SetTextColor(255, 220, 20, 255);
			unicode::_sprintf( szText, GlobalText[2548] );		// 황금 벚꽃가지 50개
			g_pRenderText->RenderText(fPos_x-10, fPos_y+250+4*13, szText, 160.0f, 0, RT3_SORT_LEFT);
			
			g_pRenderText->SetTextColor(255, 255, 255, 255);
			unicode::_sprintf( szText, GlobalText[2540] );
			g_pRenderText->RenderText(fPos_x-10, fPos_y+250+5*13, szText, 180.0f, 0, RT3_SORT_LEFT);
			
			g_pRenderText->SetTextColor(255, 255, 255, 255);
			unicode::_sprintf( szText, GlobalText[2306] );
			g_pRenderText->RenderText(fPos_x-10, fPos_y+250+6*13, szText, 200.0f, 0, RT3_SORT_LEFT);
#endif // SELECTED_LANGUAGE == LANGUAGE_ENGLISH && defined(LDS_MOD_EVENTCHERRYBLOSSOM_FORENG)

#endif //SELECTED_LANGUAGE == LANGUAGE_KOREAN
		}
		break;
#endif	// CSK_EVENT_CHERRYBLOSSOM
#ifdef ADD_SOCKET_MIX
	case SEASON3A::MIXTYPE_ATTACH_SOCKET:
		g_pRenderText->SetBgColor(0, 0, 0, 0);
		g_pRenderText->SetTextColor(200, 200, 200, 255);
		unicode::_sprintf( szText, GlobalText[2674] );	// "장착할 소켓 선택"
		g_pRenderText->RenderText(fPos_x, fPos_y+280+0*13, szText, 160.0f, 0, RT3_SORT_CENTER);
		m_SocketListBox.Render();
		break;
	case SEASON3A::MIXTYPE_DETACH_SOCKET:
		g_pRenderText->SetBgColor(0, 0, 0, 0);
		g_pRenderText->SetTextColor(200, 200, 200, 255);
		unicode::_sprintf( szText, GlobalText[2675] );	// "파괴할 소켓 선택"
		g_pRenderText->RenderText(fPos_x, fPos_y+280+0*13, szText, 160.0f, 0, RT3_SORT_CENTER);
		m_SocketListBox.Render();
		break;
#endif	// ADD_SOCKET_MIX
	default:
		break;
	}
}

#ifdef LEM_FIX_MIXREQUIREZEN	// Rtn_MixRequireZen 함수  [lem.2010.7.29]
int CNewUIMixInventory::Rtn_MixRequireZen( int _nMixZen, int _nTax )
{
	if( _nTax )		_nMixZen += ((LONGLONG)_nMixZen*g_nChaosTaxRate)/100;
	return _nMixZen;
}
#endif	// LEM_FIX_MIXREQUIREZEN

bool CNewUIMixInventory::Mix()
{
	PlayBuffer(SOUND_CLICK01);

	DWORD dwGold	= CharacterMachine->Gold;
	int	  nMixZen	= g_MixRecipeMgr.GetReqiredZen();

#ifdef LEM_FIX_MIXREQUIREZEN	// Rtn_MixRequireZen 호출 [lem.2010.7.29]
	nMixZen = Rtn_MixRequireZen( nMixZen, g_nChaosTaxRate );
#endif	// LEM_FIX_MIXREQUIREZEN


#ifdef _VS2008PORTING
	if ( nMixZen > (int)dwGold)	// 조합에 충분한 젠이 있는가
#else // _VS2008PORTING
	if ( nMixZen > dwGold)	// 조합에 충분한 젠이 있는가
#endif // _VS2008PORTING

    {
		// 젠이 부족합니다.
		g_pChatListBox->AddText("", GlobalText[596], SEASON3B::TYPE_ERROR_MESSAGE);
		return false;
	}

	if (!g_MixRecipeMgr.IsReadyToMix())	// 조합할 재료가 다 올라와 있는가
	{
        unicode::t_char szText[100];
        unicode::_sprintf (szText, GlobalText[580], GlobalText[591]);	// 재료가 부족합니다.
		g_pChatListBox->AddText("", szText, SEASON3B::TYPE_ERROR_MESSAGE);
		return false;
	}

	// 레벨 제한 검사
	int iLevel = CharacterAttribute->Level;
	if (iLevel < g_MixRecipeMgr.GetCurRecipe()->m_iRequiredLevel)
	{
		unicode::t_char szText[100];
		unicode::t_char szText2[100];
		g_MixRecipeMgr.GetCurRecipeName(szText2, 1);
		wsprintf (szText, GlobalText[2347], g_MixRecipeMgr.GetCurRecipe()->m_iRequiredLevel, szText2);	
		// %d 레벨 이상부터 해당 조합이 가능합니다.
		g_pChatListBox->AddText("", szText, SEASON3B::TYPE_ERROR_MESSAGE);
		return false;
	}

	// 조합 결과물을 담을 인벤 빈공간 체크
	if (g_MixRecipeMgr.GetCurRecipe()->m_iWidth != -1 &&	// -1이면 빈공간 검사 안함
		g_pMyInventory->FindEmptySlot(g_MixRecipeMgr.GetCurRecipe()->m_iWidth,
		g_MixRecipeMgr.GetCurRecipe()->m_iHeight) == -1)
	{
		// 빈공간이 부족합니다.
		g_pChatListBox->AddText("", GlobalText[581], SEASON3B::TYPE_ERROR_MESSAGE);
		return false;
	}

#ifdef ADD_SOCKET_MIX
	// 소켓 아이템 조합시 선택가능한 소켓인가
	if (g_MixRecipeMgr.GetMixInventoryType() == SEASON3A::MIXTYPE_ATTACH_SOCKET)
	{
		int iSelectedLine = m_SocketListBox.GetLineNum() - m_SocketListBox.SLGetSelectLineNum();

		for (int i = 0; i < m_SocketListBox.GetLineNum(); ++i)
		{
			BYTE bySocketSeedID = g_MixRecipeMgr.GetFirstItemSocketSeedID(i);	// 소켓아이템의 i번째 소켓의 시드ID
			if (bySocketSeedID != SOCKET_EMPTY)
			{
				BYTE bySeedSphereID = g_MixRecipeMgr.GetSeedSphereID(0);	// 장착할 시드스피어의 시드ID
				if (bySocketSeedID == bySeedSphereID)
				{
					// 같은 종류의 시드 스피어를 장착할 수 없다
					g_pChatListBox->AddText("", GlobalText[2683], SEASON3B::TYPE_ERROR_MESSAGE);	// "같은 종류의 시드스피어는 장비할 수 없습니다."
					return false;
				}
			}
		}

		if (m_SocketListBox.SLGetSelectLineNum() == 0)
		{
			// 소켓을 선택해야 합니다.
			g_pChatListBox->AddText("", GlobalText[2676], SEASON3B::TYPE_ERROR_MESSAGE);	// "장착할 소켓을 선택해야 합니다."
			return false;
		}
		else if (iSelectedLine > g_MixRecipeMgr.GetFirstItemSocketCount()
			|| g_MixRecipeMgr.GetFirstItemSocketSeedID(iSelectedLine) != SOCKET_EMPTY)
		{
			// 장착 할 수 없는 소켓입니다.
			g_pChatListBox->AddText("", GlobalText[2677], SEASON3B::TYPE_ERROR_MESSAGE);	// "이미 장착되어 있습니다."
			return false;
		}

		g_MixRecipeMgr.SetMixSubType(iSelectedLine);
	}
	else if (g_MixRecipeMgr.GetMixInventoryType() == SEASON3A::MIXTYPE_DETACH_SOCKET)
	{
		int iSelectedLine = m_SocketListBox.GetLineNum() - m_SocketListBox.SLGetSelectLineNum();
		if (m_SocketListBox.SLGetSelectLineNum() == 0)
		{
			// 소켓을 선택해야 합니다.
			g_pChatListBox->AddText("", GlobalText[2678], SEASON3B::TYPE_ERROR_MESSAGE);	// "파괴할 소켓을 선택해야 합니다."
			return false;
		}
		else if (iSelectedLine > g_MixRecipeMgr.GetFirstItemSocketCount()
			|| g_MixRecipeMgr.GetFirstItemSocketSeedID(iSelectedLine) == SOCKET_EMPTY)
		{
			// 장착 할 수 없는 소켓입니다.
			g_pChatListBox->AddText("", GlobalText[2679], SEASON3B::TYPE_ERROR_MESSAGE);	// "파괴할 시드스피어가 없습니다."
			return false;
		}
		g_MixRecipeMgr.SetMixSubType(iSelectedLine);
	}
#endif	// ADD_SOCKET_MIX

#ifdef LJH_MOD_CANNOT_USE_CHARMITEM_AND_CHAOSCHARMITEM_SIMULTANEOUSLY
	if (g_MixRecipeMgr.GetTotalChaosCharmCount() > 0 && g_MixRecipeMgr.GetTotalCharmCount() > 0 )
	{
		// 3286 "카오스조합부적과 행운의부적은 동시에 사용이 불가능 합니다."
		g_pChatListBox->AddText("", GlobalText[3286], SEASON3B::TYPE_ERROR_MESSAGE);
		return FALSE;
	}
#endif //LJH_MOD_CANNOT_USE_CHARMITEM_AND_CHAOSCHARMITEM_SIMULTANEOUSLY

	if (CNewUIInventoryCtrl::GetPickedItem() == NULL)	// 들고있는 아이템이 없으면 조합 실행
	{
		SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CMixCheckMsgBoxLayout));
		return true;
	}

	return false;
}

bool CNewUIMixInventory::InventoryProcess()
{
	CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();

	if(m_pNewInventoryCtrl && pPickedItem)
	{
		int iCurInventory = g_MixRecipeMgr.GetMixInventoryEquipmentIndex();

		ITEM* pItemObj = pPickedItem->GetItem();
		if (GetMixState() == MIX_READY && g_MixRecipeMgr.IsMixSource(pPickedItem->GetItem()) &&
			pPickedItem->GetOwnerInventory() == g_pMyInventory->GetInventoryCtrl())	// 인벤토리에서 조합창으로
		{
			m_pNewInventoryCtrl->SetSquareColorNormal(m_fInventoryColor[0], m_fInventoryColor[1], m_fInventoryColor[2]);
			if (SEASON3B::IsPress(VK_LBUTTON))
			{
				int iSourceIndex = pPickedItem->GetSourceLinealPos();
				int iTargetIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);
				if(iTargetIndex != -1 && m_pNewInventoryCtrl->CanMove(iTargetIndex, pItemObj))
				{
 					if(SendRequestEquipmentItem(REQUEST_EQUIPMENT_INVENTORY, MAX_EQUIPMENT_INDEX+iSourceIndex, 
 						pItemObj, iCurInventory, iTargetIndex))
 					{
 						return true;
 					}
				}
			}
		}
		else if(pPickedItem->GetOwnerInventory() == m_pNewInventoryCtrl)	// 조합창에서 조합창으로	
		{
			m_pNewInventoryCtrl->SetSquareColorNormal(m_fInventoryColor[0], m_fInventoryColor[1], m_fInventoryColor[2]);
			if (SEASON3B::IsPress(VK_LBUTTON))
			{
				int iSourceIndex = pPickedItem->GetSourceLinealPos();
				int iTargetIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);
				if(iTargetIndex != -1 && m_pNewInventoryCtrl->CanMove(iTargetIndex, pItemObj))
				{
 					if(SendRequestEquipmentItem(iCurInventory, iSourceIndex,
						pItemObj, iCurInventory, iTargetIndex))
 					{
 						return true;
 					}
				}
			}
		}
		/* CNewUIInventoryCtrl를 사용하지 않는 예외 */
		/* ITEM::ex_src_type 값으로 구분한다. (ITEM구조체 참조) */
		// ex_src_type검사는 맨 마지막에 해줘야 한다. 장비창이 아니더라도 ex_src_type이 1로 유지된다.
		else if (GetMixState() == MIX_READY && g_MixRecipeMgr.IsMixSource(pPickedItem->GetItem()) &&
			pItemObj->ex_src_type == ITEM_EX_SRC_EQUIPMENT)		// 장비창에서 조합창으로 (장비창은 CNewUIInventoryCtrl를 사용 하지 않으므로)
		{
			m_pNewInventoryCtrl->SetSquareColorNormal(m_fInventoryColor[0], m_fInventoryColor[1], m_fInventoryColor[2]);
			if (SEASON3B::IsPress(VK_LBUTTON))
			{
				int iSourceIndex = pPickedItem->GetSourceLinealPos();	// return ITEM::lineal_pos;
				int iTargetIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);
				if(iTargetIndex != -1 && m_pNewInventoryCtrl->CanMove(iTargetIndex, pItemObj))
				{
					SendRequestEquipmentItem(REQUEST_EQUIPMENT_INVENTORY, iSourceIndex, 
						pItemObj, iCurInventory, iTargetIndex);	
					return true;
				}
			}
		}
		else
		{
			m_pNewInventoryCtrl->SetSquareColorNormal(m_fInventoryWarningColor[0], m_fInventoryWarningColor[1], m_fInventoryWarningColor[2]);
		}
	}
	return false;
}

void CNewUIMixInventory::CheckMixInventory()
{
	g_MixRecipeMgr.ResetMixItemInventory();
	ITEM * pItem = NULL;
#ifdef _VS2008PORTING
	for (int i = 0; i < (int)m_pNewInventoryCtrl->GetNumberOfItems(); ++i)
#else // _VS2008PORTING
	for (int i = 0; i < m_pNewInventoryCtrl->GetNumberOfItems(); ++i)
#endif // _VS2008PORTING
	{
		pItem = m_pNewInventoryCtrl->GetItem(i);
		g_MixRecipeMgr.AddItemToMixItemInventory(pItem);
	}
	g_MixRecipeMgr.CheckMixInventory();
}

void CNewUIMixInventory::RenderMixEffect()
{
	if (m_iMixEffectTimer <= 0)
	{
		return;
	}
	else
	{
		--m_iMixEffectTimer;
	}
	
#ifdef MR0
	VPManager::Disable();
#endif //MR0
	EnableAlphaBlend();
#ifdef _VS2008PORTING
	for (int i = 0; i < (int)m_pNewInventoryCtrl->GetNumberOfItems(); ++i)
#else // _VS2008PORTING
	for (int i = 0; i < m_pNewInventoryCtrl->GetNumberOfItems(); ++i)
#endif // _VS2008PORTING
	{
		int iWidth = ItemAttribute[m_pNewInventoryCtrl->GetItem(i)->Type].Width;
		int iHeight = ItemAttribute[m_pNewInventoryCtrl->GetItem(i)->Type].Height;

		for (int h = 0; h < iHeight; ++h)
		{
			for (int w = 0; w < iWidth; ++w)
			{
				glColor3f((float)(rand()%6+6)*0.1f,(float)(rand()%4+4)*0.1f,0.2f);
				float Rotate = (float)((int)(WorldTime)%100)*20.f;
				float Scale = 5.f+(rand()%10);
				float x = m_pNewInventoryCtrl->GetPos().x + 
					(m_pNewInventoryCtrl->GetItem(i)->x + w) * INVENTORY_SQUARE_WIDTH +
					(rand()%INVENTORY_SQUARE_WIDTH);
				float y = m_pNewInventoryCtrl->GetPos().y + 
					(m_pNewInventoryCtrl->GetItem(i)->y + h) * INVENTORY_SQUARE_WIDTH +
					(rand()%INVENTORY_SQUARE_WIDTH);
				RenderBitmapRotate(BITMAP_SHINY  ,x,y,Scale,Scale,0);
				RenderBitmapRotate(BITMAP_SHINY  ,x,y,Scale,Scale,Rotate);
				RenderBitmapRotate(BITMAP_SHINY+1,x,y,Scale*3.f,Scale*3.f,Rotate);
				RenderBitmapRotate(BITMAP_LIGHT,x,y,Scale*6.f,Scale*6.f,0);
			}
		}
	}
	DisableAlphaBlend();
#ifdef MR0
	VPManager::Enable();
#endif //MR0
}


int SEASON3B::CNewUIMixInventory::GetPointedItemIndex()
{
	return m_pNewInventoryCtrl->GetPointedSquareIndex();
}
