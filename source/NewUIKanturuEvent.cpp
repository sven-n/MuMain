// NewUIKanturu2ndEnterNpc.cpp: implementation of the CNewUIKanturu2ndEnterNpc class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIKanturuEvent.h"
#include "NewUICommonMessageBox.h"
#include "NewUISystem.h"
#include "ZzzAI.h"
#include "ZzzEffect.h"
#include "wsclientinline.h"
#include "ChangeRingManager.h"
#include "GM_Kanturu_3rd.h"
#include "CDirection.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SEASON3B::CNewUIKanturu2ndEnterNpc::CNewUIKanturu2ndEnterNpc()
{	
	m_pNewUIMng = NULL;
	m_Pos.x = m_Pos.y = 0;

	m_pNpcObject = NULL;

	m_dwRefreshTime = 0;
	m_dwRefreshButtonGapTime = 0;

	Initialize();
}

SEASON3B::CNewUIKanturu2ndEnterNpc::~CNewUIKanturu2ndEnterNpc()
{
	Release();
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::Initialize()
{
	m_bNpcAnimation = false;
	m_bEnterRequest = false;

	m_iStateTextNum	=	0;
	ZeroMemory(m_strSubject, sizeof(m_strSubject));
	for(int i=0; i<KANTURU2ND_STATETEXT_MAX; i++)
	{
		ZeroMemory(m_strStateText[i], sizeof(m_strStateText[i]));
	}
}

bool SEASON3B::CNewUIKanturu2ndEnterNpc::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if(NULL == pNewUIMng)
		return false;

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_KANTURU2ND_ENTERNPC, this);

	SetPos(x, y);

	LoadImages();

	SetButtonInfo();

	Show(false);

	return true;
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::Release()
{
	UnloadImages();

	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
}

bool SEASON3B::CNewUIKanturu2ndEnterNpc::UpdateMouseEvent()
{
	// 버튼 마우스이벤트 처리
	if(BtnProcess() == true)	// 처리가 완료 되었다면
	{
		return false;
	}

	if(CheckMouseIn(m_Pos.x, m_Pos.y, KANTURU2ND_ENTER_WINDOW_WIDTH, KANTURU2ND_ENTER_WINDOW_HEIGHT))
	{
		return false;
	}

	return true;
}

bool SEASON3B::CNewUIKanturu2ndEnterNpc::UpdateKeyEvent()
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_KANTURU2ND_ENTERNPC) == true)
	{
		if(SEASON3B::IsPress(VK_ESCAPE) == true)
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_KANTURU2ND_ENTERNPC);
			PlayBuffer(SOUND_CLICK01);
			return false;
		}
	}
	
	return true;
}	

bool SEASON3B::CNewUIKanturu2ndEnterNpc::Update()
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_KANTURU2ND_ENTERNPC) == true)
	{
		// 자동 정보 갱신 업데이트
		if(timeGetTime() - m_dwRefreshTime > KANTURU2ND_REFRESH_GAPTIME)
		{
			SendRequestKanturu3rdInfo();
		}
	}

	return true;
}

bool SEASON3B::CNewUIKanturu2ndEnterNpc::Render()
{
	EnableAlphaTest();

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// 프레임을 그린다.
	RenderFrame();

	RenderTexts();

	RenderButtons();

	return true;
}

float SEASON3B::CNewUIKanturu2ndEnterNpc::GetLayerDepth()	//. 5.0f
{
	return 10.1f;
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::SetNpcObject(OBJECT* pObj)
{
	m_pNpcObject = pObj;
}

bool SEASON3B::CNewUIKanturu2ndEnterNpc::IsNpcAnimation()
{
	return m_bNpcAnimation;
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::SetNpcAnimation(bool bValue)
{
	m_bNpcAnimation = bValue;
}

bool SEASON3B::CNewUIKanturu2ndEnterNpc::IsEnterRequest()
{
	return m_bEnterRequest;
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::SetEnterRequest(bool bValue)
{
	m_bEnterRequest = bValue;
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::CreateMessageBox(BYTE btResult)
{
	unicode::t_char strMessage[256];
	if(btResult == POPUP_FAILED || btResult == POPUP_FAILED2)
	{
		unicode::_strcpy(strMessage, GlobalText[2170]);	// "입장에 실패하였습니다."
	}
	else if(btResult == POPUP_UNIRIA)
	{
		unicode::_strcpy(strMessage, GlobalText[569]);	// "유니리아를 타고는 이동할수 없습니다."
	}
	else if(btResult == POPUP_CHANGERING)
	{
		unicode::_strcpy(strMessage, GlobalText[2175]);	// "변신반지를 착용하고는 이동할 수 없습니다."
	}
	else if(btResult == POPUP_NOT_HELPER)
	{
		unicode::_strcpy(strMessage, GlobalText[2176]);	// "디노란트, 다크호스, 펜릴을 타거나 날개나 망토를 착용하여야 이동 가능합니다." 
	}
	else
	{
		unicode::_strcpy(strMessage, GlobalText[2170+btResult]);
	}

	SEASON3B::CreateOkMessageBox(strMessage);
		
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::ReceiveKanturu3rdInfo(BYTE btState, BYTE btDetailState, BYTE btEnter, BYTE btUserCount, int iRemainTime)
{
	if(m_pNpcObject && m_pNpcObject->CurrentAction == KANTURU2ND_NPC_ANI_ROT)
	{
		return;
	}

	// 메세지 박스가 없어야 연다.
	if(g_MessageBox->IsEmpty() == false)
	{
		return;
	}

	//정보 갱신전에 초기화
	Initialize();

	m_byState = btState;

	// 입장하기 버튼 처리
	if(btEnter == 1)
	{
		m_BtnEnter.UnLock();
		m_BtnEnter.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
		m_BtnEnter.ChangeTextColor(RGBA(255, 255, 255, 255));
	}
	else
	{
		m_BtnEnter.Lock();
		m_BtnEnter.ChangeImgColor(BUTTON_STATE_UP, RGBA(100, 100, 100, 255));
		m_BtnEnter.ChangeTextColor(RGBA(100, 100, 100, 255));
	}
	
	// 텍스트 세팅
	if(btState == KANTURU_STATE_TOWER)
	{
		if(btDetailState == KANTURU_TOWER_REVITALIXATION || btDetailState == KANTURU_TOWER_NOTIFY)
		{
			unicode::_strcpy(m_strSubject, GlobalText[2149]);	// "정제의 탑으로 이동 가능합니다."
			unicode::_strcpy(m_strStateText[0], GlobalText[2150]);	// "정제의 탑으로 갈 수 있는 통로가 개방되어 있습니다."
			unicode::_sprintf(m_strStateText[1], GlobalText[2151], iRemainTime/3600); // "%d 시간뒤에 정제의 탑으로 가는 통로가 폐쇄될 예정입니다."
			m_iStateTextNum = 2;
		}
		else
		{
			unicode::_strcpy(m_strSubject, GlobalText[2174]); // "정제의 탑 입장이 통제됩니다."
			unicode::_strcpy(m_strStateText[0], GlobalText[2160]); // "정제의 탑으로 가기 위해서는 나이트메어의 힘에 잠식된 마야와 마야를 조종하고 있는 나이트메어를 쓰러트려 길을 열어야 합니다."
			unicode::_strcpy(m_strStateText[1], GlobalText[2161]); // "마야의 안정성과 보안 유지를 위해 출입이 제한되어있습니다. 수석 연구원 용으로 발급된 '문스톤 펜던트'가 필요합니다."
			m_iStateTextNum = 2;
		}
	}
	else if(btState == KANTURU_STATE_MAYA_BATTLE)
	{
		if(btDetailState != KANTURU_MAYA_DIRECTION_STANBY1 
			&& btDetailState != KANTURU_MAYA_DIRECTION_STANBY2
			&& btDetailState != KANTURU_MAYA_DIRECTION_STANBY3)
		{
			unicode::_strcpy(m_strSubject, GlobalText[2152]);	// "현재 마야와의 전투가 진행중입니다."
			unicode::_sprintf(m_strStateText[0], GlobalText[2153], btUserCount);	// "%d명이 정제의 탑으로 통하는 길을 열기 위해 전투를 벌이고 있습니다. ..... (생략)"
		}
		else
		{
			unicode::_strcpy(m_strSubject, GlobalText[2163]);	// "탑으로 가는 길을 열기 위해 더 많은 사람이 필요 합니다."

			if(btDetailState == KANTURU_MAYA_DIRECTION_STANBY1)
			{
				if(btUserCount < 15)
				{
					unicode::_strcpy(m_strStateText[0], GlobalText[2164]);	// "지금 입장하실 수 있습니다."
				}
				else if(btUserCount == 15)
				{
					unicode::_strcpy(m_strStateText[0], GlobalText[2172]);	// "15인이 이미 입장해 있기 때문에 더 이상 입장 할 수 없습니다."
				}
				else
				{
					if(m_BtnEnter.IsLock() == false)
					{
						unicode::_strcpy(m_strStateText[0], GlobalText[2164]);	// "지금 입장하실 수 있습니다."
					}
					else
					{
						unicode::_strcpy(m_strStateText[0], GlobalText[2172]);	// "15인이 이미 입장해 있기 때문에 더 이상 입장 할 수 없습니다."
					}
				}
				m_iStateTextNum = 1;
			}
			else if(btDetailState == KANTURU_MAYA_DIRECTION_STANBY2)
			{
				if(btUserCount < 15)
				{
					unicode::_sprintf(m_strStateText[0], GlobalText[2165], btUserCount);	// "나이트메어가 마야의 왼손에 대한 제어권을 상실하였습니다. 그러나 그 과정에서 피해가 발생하여 현재 %d명이 살아남았습니다."
					unicode::_sprintf(m_strStateText[1], GlobalText[2167], 15-btUserCount);	// "앞으로 %d명의 힘이 더 필요합니다."
					m_iStateTextNum = 2;
				}
				else if(btUserCount == 15)
				{
					unicode::_strcpy(m_strStateText[0], GlobalText[2168]);	// "나이트메어가 마야의 왼손에 대한 제어권을 상실하였습니다."
					m_iStateTextNum = 1;
				}
				
			}
			else if(btDetailState == KANTURU_MAYA_DIRECTION_STANBY3)
			{
				if(btUserCount < 15)
				{
					unicode::_sprintf(m_strStateText[0], GlobalText[2166], btUserCount);	// "나이트메어가 마야의 오른손에 대한 제어권을 상실하였습니다. 그러나 그 과정에서 피해가 발생하여 현재 %d명이 살아남았습니다."
					unicode::_sprintf(m_strStateText[1], GlobalText[2167], 15-btUserCount);	// "앞으로 %d명의 힘이 더 필요합니다."
					m_iStateTextNum = 2;
				}
				else if(btUserCount == 15)
				{
					unicode::_strcpy(m_strStateText[0], GlobalText[2168]);	// "나이트메어가 마야의 오른손에 대한 제어권을 상실하였습니다."
					m_iStateTextNum = 1;
				}
			}
			else
			{
				if(m_BtnEnter.IsLock() == false)
				{
					unicode::_strcpy(m_strStateText[0], GlobalText[2164]);	// "지금 입장하실 수 있습니다."

					m_iStateTextNum = 1;
				}
			}
		}

		if(btDetailState == KANTURU_MAYA_DIRECTION_NOTIFY || btDetailState == KANTURU_MAYA_DIRECTION_MONSTER1 || btDetailState == KANTURU_MAYA_DIRECTION_MAYA1
			|| btDetailState == KANTURU_MAYA_DIRECTION_END_MAYA1 || btDetailState == KANTURU_MAYA_DIRECTION_ENDCYCLE_MAYA1)
		{
			unicode::_sprintf(m_strStateText[1], GlobalText[2154], btUserCount);	// "현재 %d명이 마야의 왼손과 전투중입니다."
			m_iStateTextNum = 2;
		}
		else if(btDetailState == KANTURU_MAYA_DIRECTION_MONSTER2 || btDetailState == KANTURU_MAYA_DIRECTION_MAYA2
			|| btDetailState == KANTURU_MAYA_DIRECTION_END_MAYA2 || btDetailState == KANTURU_MAYA_DIRECTION_ENDCYCLE_MAYA2)
		{
			unicode::_sprintf(m_strStateText[1], GlobalText[2155], btUserCount);	// "현재 %d명이 마야의 오른손과 전투중입니다."
			m_iStateTextNum = 2;
		}
		else if(btDetailState == KANTURU_MAYA_DIRECTION_MONSTER3 || btDetailState == KANTURU_MAYA_DIRECTION_MAYA3
			|| btDetailState == KANTURU_MAYA_DIRECTION_END_MAYA3 || btDetailState == KANTURU_MAYA_DIRECTION_ENDCYCLE_MAYA3)
		{
			unicode::_sprintf(m_strStateText[1], GlobalText[2156], btUserCount);	// "현재 %d명이 마야의 양손과 전투중입니다."
			m_iStateTextNum = 2;
		}
		else if(btDetailState == KANTURU_MAYA_DIRECTION_NONE || btDetailState == KANTURU_MAYA_DIRECTION_END
			|| btDetailState == KANTURU_MAYA_DIRECTION_ENDCYCLE)
		{
			m_iStateTextNum = 1;
		}	
	}
	else if(btState == KANTURU_STATE_NIGHTMARE_BATTLE)
	{
		unicode::_strcpy(m_strSubject, GlobalText[2152]);	// "현재 마야와의 전투가 진행중입니다."
		unicode::_sprintf(m_strStateText[0], GlobalText[2153], btUserCount);	// "%d명이 정제의 탑으로 통하는 길을 열기 위해 전투를 벌이고 있습니다. ..... (생략)"
		unicode::_sprintf(m_strStateText[1], GlobalText[2157], btUserCount);	// "현재 %d명이 나이트메어와 전투중입니다."
		m_iStateTextNum = 2;
	}
	else if(btState == KANTURU_STATE_STANDBY)
	{
		unicode::_strcpy(m_strSubject, GlobalText[2158]);	// "곧 보스전이 개시됩니다."
		if(btDetailState == 1)	// STANBY_START
		{
			unicode::_sprintf(m_strStateText[0], GlobalText[2159], iRemainTime/60); // "나이트메어의 힘이 마야를 잠식하고 있습니다. 그로 인해 정제의 탑 내부가 불안정한 상태이기에 %d 분동안 입장이 통제됩니다."
		}
		else // STANBY_NONE || STANBY_NOTIFY || STANBY_END || STANBY_ENDCYCLE
		{
			unicode::_sprintf(m_strStateText[0], GlobalText[2162]); // "잠시 후 마야가 있는 곳으로 접근이 가능합니다."
		}
		unicode::_sprintf(m_strStateText[1], GlobalText[2160]); // "정제의 탑으로 가기 위해서는 나이트메어의 힘에 잠식된 마야와 마야를 조종하고 있는 나이트메어를 쓰러트려 길을 열어야 합니다."
		unicode::_sprintf(m_strStateText[2], GlobalText[2161]); // "마야의 안정성과 보안 유지를 위해 출입이 제한되어있습니다. 수석 연구원 용으로 발급된 '문스톤 펜던트'가 필요합니다."
		m_iStateTextNum = 3;
	}
	else
	{
		unicode::_strcpy(m_strSubject, GlobalText[2170]);	// "입장에 실패하였습니다."
	}

	// UI 창이 안열려있다면 연다.
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_KANTURU2ND_ENTERNPC) == false)
	{
		g_pNewUISystem->Show(SEASON3B::INTERFACE_KANTURU2ND_ENTERNPC);
	}
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::ReceiveKanturu3rdEnter(BYTE btResult)
{
	m_bEnterRequest = false;
	CreateMessageBox(btResult);
	
	m_pNpcObject->AnimationFrame = 0;
	m_bNpcAnimation = false;
	SetAction(m_pNpcObject, KANTURU2ND_NPC_ANI_STOP);

	// 이펙트 제거
	DeleteJoint(BITMAP_JOINT_ENERGY, NULL);

	g_pNewUISystem->Hide(SEASON3B::INTERFACE_KANTURU2ND_ENTERNPC);
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::SendRequestKanturu3rdInfo()
{
	SendRequestKanturuInfo();
	m_dwRefreshTime	= timeGetTime();
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::SendRequestKanturu3rdEnter()
{
	SendRequestKanturuEnter()
	m_bEnterRequest = true;
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::LoadImages()
{
	LoadBitmap("Interface\\newui_msgbox_top.tga", IMAGE_KANTURU2ND_TOP, GL_LINEAR);
	LoadBitmap("Interface\\newui_msgbox_middle.tga", IMAGE_KANTURU2ND_MIDDLE, GL_LINEAR);
	LoadBitmap("Interface\\newui_msgbox_bottom.tga", IMAGE_KANTURU2ND_BOTTOM, GL_LINEAR);
	LoadBitmap("Interface\\newui_msgbox_back.jpg", IMAGE_KANTURU2ND_BACK, GL_LINEAR);
	LoadBitmap("Interface\\newui_btn_empty_very_small.tga", IMAGE_KANTURU2ND_BTN, GL_LINEAR);
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::UnloadImages()
{
	DeleteBitmap(IMAGE_KANTURU2ND_TOP);
	DeleteBitmap(IMAGE_KANTURU2ND_MIDDLE);
	DeleteBitmap(IMAGE_KANTURU2ND_BOTTOM);
	DeleteBitmap(IMAGE_KANTURU2ND_BACK);
	DeleteBitmap(IMAGE_KANTURU2ND_BTN);
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::SetButtonInfo()
{
	// 2148 "정보갱신"
	m_BtnRefresh.ChangeText(GlobalText[2148]);
	m_BtnRefresh.ChangeTextBackColor(RGBA(255, 255, 255, 0));
	m_BtnRefresh.ChangeButtonImgState(true, IMAGE_KANTURU2ND_BTN, true);
	m_BtnRefresh.ChangeButtonInfo(m_Pos.x+17, m_Pos.y+220, 53, 23);
	m_BtnRefresh.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
	m_BtnRefresh.ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));

	// 2147 "입장하기"
	m_BtnEnter.ChangeText(GlobalText[2147]);
	m_BtnEnter.ChangeTextBackColor(RGBA(255, 255, 255, 0));
	m_BtnEnter.ChangeButtonImgState(true, IMAGE_KANTURU2ND_BTN, true);
	m_BtnEnter.ChangeButtonInfo(m_Pos.x+87, m_Pos.y+220, 53, 23);
	m_BtnEnter.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
	m_BtnEnter.ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));

	// 1002 "닫기"
	m_BtnClose.ChangeText(GlobalText[1002]);
	m_BtnClose.ChangeTextBackColor(RGBA(255, 255, 255, 0));
	m_BtnClose.ChangeButtonImgState(true, IMAGE_KANTURU2ND_BTN, true);
	m_BtnClose.ChangeButtonInfo(m_Pos.x+157, m_Pos.y+220, 53, 23);
	m_BtnClose.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
	m_BtnClose.ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
}

bool SEASON3B::CNewUIKanturu2ndEnterNpc::BtnProcess()
{
	// 자동갱신 버튼 업데이트
	if(m_BtnRefresh.IsLock() == true)
	{
		if(timeGetTime() - m_dwRefreshButtonGapTime > KANTURU2ND_REFRESHBUTTON_GAPTIME)
		{
			m_BtnRefresh.UnLock();
			m_BtnRefresh.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
			m_BtnRefresh.ChangeTextColor(RGBA(255, 255, 255, 255));
		}
	}
	else if(m_BtnRefresh.UpdateMouseEvent() == true)
	{
		SendRequestKanturu3rdInfo();
				
		m_BtnRefresh.Lock();
		m_BtnRefresh.ChangeImgColor(BUTTON_STATE_UP, RGBA(100, 100, 100, 255));
		m_BtnRefresh.ChangeTextColor(RGBA(100, 100, 100, 255));

		m_dwRefreshButtonGapTime = timeGetTime();
		return true;
	}

	if(m_BtnEnter.UpdateMouseEvent() == true)
	{
		if(m_pNpcObject)
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_KANTURU2ND_ENTERNPC);

			// 정제의 탑 상태일때는 검사 안해도 된다.
			if(m_byState == KANTURU_STATE_TOWER)
			{
				SetAction(m_pNpcObject, KANTURU2ND_NPC_ANI_ROT);
				m_bNpcAnimation = true;
				return true;
			}
			
			// 예외 처리
			ITEM *pItemHelper, *pItemRingLeft, *pItemRingRight, *pItemWing;
			pItemHelper = &CharacterMachine->Equipment[EQUIPMENT_HELPER];
			pItemRingLeft = &CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];
			pItemRingRight = &CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];
			pItemWing = &CharacterMachine->Equipment[EQUIPMENT_WING];
			
			// 유니리아를 탄 경우
			if(pItemHelper->Type == ITEM_HELPER+2)	
			{
				CreateMessageBox(POPUP_UNIRIA);
				return true;
			}
			
			// 변신반지 착용한 경우
			if(g_ChangeRingMgr->CheckChangeRing(pItemRingLeft->Type) 
				|| g_ChangeRingMgr->CheckChangeRing(pItemRingRight->Type))
			{
				CreateMessageBox(POPUP_CHANGERING);
				return true;
			}
			
			// 날개, 망토, 디노란트, 펜릴, 다크호스이 없는 경우
			if(	!((pItemWing->Type >= ITEM_WING+0 && pItemWing->Type <= ITEM_WING+6)	// 날개
#ifdef ADD_ALICE_WINGS_1
				|| (pItemWing->Type >= ITEM_WING+36 && pItemWing->Type <= ITEM_WING+43)	// 3차 날개, 소환술사 날개.
#else	// ADD_ALICE_WINGS_1
				|| (pItemWing->Type >= ITEM_WING+36 && pItemWing->Type <= ITEM_WING+40 )	// 3차 날개들
#endif	// ADD_ALICE_WINGS_1
#ifdef LDK_ADD_INGAMESHOP_SMALL_WING			// 기간제 날개 작은(군망, 재날, 요날, 천날, 사날)
				|| ( ITEM_WING+130 <= pItemWing->Type && pItemWing->Type <= ITEM_WING+134 )
#endif //LDK_ADD_INGAMESHOP_SMALL_WING
				|| pItemHelper->Type == ITEM_HELPER+3	// 디노란트
				|| pItemHelper->Type == ITEM_HELPER+4	// 다크호스
				|| pItemWing->Type == ITEM_HELPER+30	// 망토
				|| pItemHelper->Type == ITEM_HELPER+37	// 펜릴
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
				|| (pItemWing->Type >= ITEM_WING+49 && pItemWing->Type <= ITEM_WING+50 )	// 레이지파이터 날개
				|| (pItemWing->Type == ITEM_WING+135)	// 작은 무인의 망토
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
				) )
			{
				CreateMessageBox(POPUP_NOT_HELPER);
				return true;
			}
			
			// 문스톤펜던트
			if(pItemRingLeft->Type == ITEM_HELPER+38 || pItemRingRight->Type == ITEM_HELPER+38)	// 문스톤펜던트
			{
				SetAction(m_pNpcObject, KANTURU2ND_NPC_ANI_ROT);
				m_bNpcAnimation = true;
			}
			else
			{
				CreateMessageBox(POPUP_NOT_MUNSTONE);	// 문스톤펜던트 아이템이 없음.
				return true;
			}
			
		}

		return true;
	}

	if(m_BtnClose.UpdateMouseEvent() == true)
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_KANTURU2ND_ENTERNPC);

		return true;
	}
	return false;
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::RenderFrame()
{
	float x, y, width, height;

	// 메세지박스 바탕
	x = m_Pos.x; y = m_Pos.y + 2.f, width = KANTURU2ND_ENTER_WINDOW_WIDTH - MSGBOX_BACK_BLANK_WIDTH; height = KANTURU2ND_ENTER_WINDOW_HEIGHT - MSGBOX_BACK_BLANK_HEIGHT;
	RenderImage(IMAGE_KANTURU2ND_BACK, x, y, width, height);

	// 메세지박스 윗부분
	x = m_Pos.x; y = m_Pos.y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
	RenderImage(IMAGE_KANTURU2ND_TOP, x, y, width, height);

	// 메세지박스 중간부분
	x = m_Pos.x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
	for(int i=0; i<10; ++i)
	{
		RenderImage(IMAGE_KANTURU2ND_MIDDLE, x, y, width, height);
		y += height;
	}
	
	// 메세지박스 아랫부분
	x = m_Pos.x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
	RenderImage(IMAGE_KANTURU2ND_BOTTOM, x, y, width, height);
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::RenderButtons()
{
	m_BtnEnter.Render();
	m_BtnRefresh.Render();
	m_BtnClose.Render();
}

void SEASON3B::CNewUIKanturu2ndEnterNpc::RenderTexts()
{
	// 제목 텍스트 렌더링
	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetTextColor(0xFF49B0FF);
	g_pRenderText->SetBgColor(0);
	
	int iTextY = m_Pos.y+30;
	int iLine;
	unicode::t_char strTemp[3][52];
	ZeroMemory(strTemp, sizeof(strTemp));
	iLine = SeparateTextIntoLines(m_strSubject, strTemp[0], 3, 52);
	for(int i=0; i<iLine; i++)
	{
		g_pRenderText->RenderText(m_Pos.x, iTextY, strTemp[i], KANTURU2ND_ENTER_WINDOW_WIDTH, 0, RT3_SORT_CENTER);
		iTextY += 12;
	}
	
	iTextY += 20;
	
	// 세부 상태 렌더링
	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(0xFF61F191);
#ifdef _VS2008PORTING
	for(int i=0; i<m_iStateTextNum; i++)
#else // _VS2008PORTING
	for(i=0; i<m_iStateTextNum; i++)
#endif // _VS2008PORTING
	{
		iLine = SeparateTextIntoLines(m_strStateText[i], strTemp[0], 3, 52);
		for(int j=0; j<iLine; j++)
		{
			g_pRenderText->RenderText(m_Pos.x, iTextY, strTemp[j], KANTURU2ND_ENTER_WINDOW_WIDTH, 0, RT3_SORT_CENTER);
			iTextY += 12;
		}

		iTextY += 15;

		g_pRenderText->SetTextColor(CLRDW_BR_YELLOW);
		
		ZeroMemory(strTemp[i], sizeof(strTemp[i]));
	}
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


SEASON3B::CNewUIKanturuInfoWindow::CNewUIKanturuInfoWindow()
{
	m_pNewUIMng = NULL;
	m_Pos.x = m_Pos.y = 0;
	
	m_iMinute = 0;
	m_iSecond = 0;
	m_dwSyncTime = 0;
}

SEASON3B::CNewUIKanturuInfoWindow::~CNewUIKanturuInfoWindow()
{
	Release();
}

bool SEASON3B::CNewUIKanturuInfoWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if(NULL == pNewUIMng)
		return false;

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_KANTURU_INFO, this);

	SetPos(x, y);

	LoadImages();

	Show(false);

	return true;
}

void SEASON3B::CNewUIKanturuInfoWindow::Release()
{
	UnloadImages();

	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}
}

void SEASON3B::CNewUIKanturuInfoWindow::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
}

bool SEASON3B::CNewUIKanturuInfoWindow::UpdateMouseEvent()
{
	return true;
}

bool SEASON3B::CNewUIKanturuInfoWindow::UpdateKeyEvent()
{
	return true;
}

bool SEASON3B::CNewUIKanturuInfoWindow::Update()
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_KANTURU_INFO))
	{
		if(M39Kanturu3rd::IsInKanturu3rd() == false)
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_KANTURU_INFO);
		}
	}
	
	return true;
}

bool SEASON3B::CNewUIKanturuInfoWindow::Render()
{
	EnableAlphaTest();

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// 프레임을 그린다.
	RenderFrame();

	// 정보 렌더링
	RenderInfo();
	
	return true;
}

void SEASON3B::CNewUIKanturuInfoWindow::RenderFrame()
{
	RenderImage(IMAGE_KANTURUINFO_WINDOW, m_Pos.x, m_Pos.y, 99.f, 78.f);
}

void SEASON3B::CNewUIKanturuInfoWindow::RenderInfo()
{
	g_pRenderText->SetFont(g_hFontBold);

	unicode::t_char strText[256];
	// 2180 "현재인원 :  %d"
	unicode::_sprintf(strText, GlobalText[2180], UserCount);
	g_pRenderText->SetBgColor(0);
	g_pRenderText->SetTextColor(134, 134, 199, 255);
	g_pRenderText->RenderText(m_Pos.x+10, m_Pos.y+15, strText);

	if(g_Direction.m_CKanturu.m_iMayaState == KANTURU_MAYA_DIRECTION_MAYA1
		|| g_Direction.m_CKanturu.m_iMayaState == KANTURU_MAYA_DIRECTION_MAYA2
		|| g_Direction.m_CKanturu.m_iMayaState == KANTURU_MAYA_DIRECTION_MAYA3)
	{
		// 2182 "남은몬스터 : 보스"
		g_pRenderText->RenderText(m_Pos.x+10, m_Pos.y+35, GlobalText[2182]);
	}
	else
	{
		// 2183 "남은몬스터 :  %d"
		unicode::_sprintf(strText, GlobalText[2183], MonsterCount);
		g_pRenderText->RenderText(m_Pos.x+10, m_Pos.y+35, strText);
	}

	// 시간
	int iCurrentTime = (GetTickCount() - m_dwSyncTime) / 1000;
	int iPastSecond = m_iSecond - iCurrentTime;

	m_iMinute = iPastSecond / 60;
	int iSecond;
	
	// 서버로 부터 60초 이하의 값이 올때를 대비한 예외 처리
	if(m_iMinute <= 0)	
	{
		iSecond = 0;
	}
	else
	{
		iSecond = iPastSecond % (60 * m_iMinute);
	}

	static DWORD dwTime = timeGetTime();
	static bool bRender = true;
	if(timeGetTime() - dwTime > 500)
	{
		dwTime = timeGetTime();
		bRender = !bRender;
	}

	if(bRender)
	{
		g_pRenderText->RenderText(m_Pos.x+48, m_Pos.y+57, ":");
	}

	glColor3f( 134.f/255.f, 134.f/255.f, 199.f/255.f );
	SEASON3B::RenderNumber(m_Pos.x+35, m_Pos.y+55, m_iMinute, 1.f);
	SEASON3B::RenderNumber(m_Pos.x+65, m_Pos.y+55, iSecond, 1.f);
}

float SEASON3B::CNewUIKanturuInfoWindow::GetLayerDepth()
{
	return 1.92f;
}

float SEASON3B::CNewUIKanturuInfoWindow::GetKeyEventOrder()
{
	return 9.1f;
}

void SEASON3B::CNewUIKanturuInfoWindow::LoadImages()
{
	LoadBitmap("Interface\\newui_Figure_kantru.tga", IMAGE_KANTURUINFO_WINDOW, GL_LINEAR);
}

void SEASON3B::CNewUIKanturuInfoWindow::UnloadImages()
{
	DeleteBitmap(IMAGE_KANTURUINFO_WINDOW);
}

void SEASON3B::CNewUIKanturuInfoWindow::SetTime(int iTimeLimit)
{
	m_iMinute = 0;
	m_iSecond = iTimeLimit/1000;
	m_dwSyncTime = GetTickCount();
}
