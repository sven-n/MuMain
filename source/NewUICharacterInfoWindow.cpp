// NewUICharacterInfoWindow.cpp: implementation of the CNewUICharacterInfoWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "NewUICharacterInfoWindow.h"
#include "NewUISystem.h"
#include "CSItemOption.h"
#include "ZzzBMD.h"
#include "ZzzCharacter.h"
#include "UIControls.h"
#include "ZzzInterface.h"
#include "ZzzScene.h"
#include "Local.h"
#include "ZzzInventory.h"
#include "UIJewelHarmony.h"
#include "UIManager.h"
#include "wsclientinline.h"
#ifdef KJH_ADD_SERVER_LIST_SYSTEM
#include "ServerListManager.h"
#endif // KJH_ADD_SERVER_LIST_SYSTEM

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace SEASON3B;

#ifndef KJH_ADD_SERVER_LIST_SYSTEM			// #ifndef
extern int ServerLocalSelect;
#endif // KJH_ADD_SERVER_LIST_SYSTEM

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
SEASON3B::CNewUICharacterInfoWindow::CNewUICharacterInfoWindow() 
{
	m_pNewUIMng = NULL;
	m_Pos.x = m_Pos.y = 0;
}

SEASON3B::CNewUICharacterInfoWindow::~CNewUICharacterInfoWindow() 
{ 
	Release(); 
}

bool SEASON3B::CNewUICharacterInfoWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if(NULL == pNewUIMng)
		return false;

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_CHARACTER, this);

	SetPos(x, y);

	LoadImages();

	SetButtonInfo();

	Show(false);

	return true;
}

void SEASON3B::CNewUICharacterInfoWindow::SetButtonInfo()
{
	unicode::t_char strText[256];

	// 힘 스탯 버튼
	m_BtnStat[STAT_STRENGTH].ChangeButtonImgState(true, IMAGE_CHAINFO_BTN_STAT, false);
	m_BtnStat[STAT_STRENGTH].ChangeButtonInfo( m_Pos.x + 160, m_Pos.y + HEIGHT_STRENGTH+2, 16, 15 );

	// 민첩 스탯 버튼
	m_BtnStat[STAT_DEXTERITY].ChangeButtonImgState(true, IMAGE_CHAINFO_BTN_STAT, false);
	m_BtnStat[STAT_DEXTERITY].ChangeButtonInfo( m_Pos.x + 160, m_Pos.y + HEIGHT_DEXTERITY+2, 16, 15 );

	// 체력 스탯 버튼
	m_BtnStat[STAT_VITALITY].ChangeButtonImgState(true, IMAGE_CHAINFO_BTN_STAT, false);
	m_BtnStat[STAT_VITALITY].ChangeButtonInfo( m_Pos.x + 160, m_Pos.y + HEIGHT_VITALITY+2, 16, 15 );

	// 에너지 스탯 버튼
	m_BtnStat[STAT_ENERGY].ChangeButtonImgState(true, IMAGE_CHAINFO_BTN_STAT, false);
	m_BtnStat[STAT_ENERGY].ChangeButtonInfo( m_Pos.x + 160, m_Pos.y + HEIGHT_ENERGY+2, 16, 15 );

	// 통솔 스탯 버튼
	m_BtnStat[STAT_CHARISMA].ChangeButtonImgState(true, IMAGE_CHAINFO_BTN_STAT, false);
	m_BtnStat[STAT_CHARISMA].ChangeButtonInfo( m_Pos.x + 160, m_Pos.y + HEIGHT_CHARISMA+2, 16, 15 );

	// exit
	m_BtnExit.ChangeButtonImgState(true, IMAGE_CHAINFO_BTN_EXIT, false);
	m_BtnExit.ChangeButtonInfo( m_Pos.x + 13, m_Pos.y + 392, 36, 29 );
	// 927 "닫기 (%s)"
	unicode::_sprintf(strText, GlobalText[927], "C");
	m_BtnExit.ChangeToolTipText(strText, true);

	// quest
	m_BtnQuest.ChangeButtonImgState(true, IMAGE_CHAINFO_BTN_QUEST, false);
	m_BtnQuest.ChangeButtonInfo( m_Pos.x + 50, m_Pos.y + 392, 36, 29 );
	// 1140 "퀘스트"
	unicode::_sprintf(strText, "%s(%s)", GlobalText[1140], "T");
	m_BtnQuest.ChangeToolTipText(strText, true);

	// pet
	m_BtnPet.ChangeButtonImgState(true, IMAGE_CHAINFO_BTN_PET, false);
	m_BtnPet.ChangeButtonInfo( m_Pos.x + 87, m_Pos.y + 392, 36, 29 );
	// 1217 "Pet"
	m_BtnPet.ChangeToolTipText(GlobalText[1217], true);

	m_BtnMasterLevel.ChangeButtonImgState(true, IMAGE_CHAINFO_BTN_MASTERLEVEL, false);
	m_BtnMasterLevel.ChangeButtonInfo( m_Pos.x + 124, m_Pos.y + 392, 36, 29 );
	// 1749 "마스터 스킬 트리(A)"
	m_BtnMasterLevel.ChangeToolTipText(GlobalText[1749], true);
}

void SEASON3B::CNewUICharacterInfoWindow::Release()
{
	UnloadImages();

	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}
}

void SEASON3B::CNewUICharacterInfoWindow::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
}

bool SEASON3B::CNewUICharacterInfoWindow::UpdateMouseEvent()
{
	// 버튼 마우스이벤트 처리
	if(BtnProcess() == true)	// 처리가 완료 되었다면
	{
		return false;
	}

	if(CheckMouseIn(m_Pos.x, m_Pos.y, CHAINFO_WINDOW_WIDTH, CHAINFO_WINDOW_HEIGHT))
	{
		return false;
	}

	return true;
}

bool SEASON3B::CNewUICharacterInfoWindow::BtnProcess()
{
	POINT ptExitBtn1 = { m_Pos.x+169, m_Pos.y+7 };

	//. Exit1 버튼 (기본처리)
	if(SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_CHARACTER);
		return true;
	}

        //  스텟 포인트가 있다.
	if(CharacterAttribute->LevelUpPoint > 0)
	{
		int iBaseClass = GetBaseClass(Hero->Class);
		int iCount = 0;
		if(iBaseClass == CLASS_DARK_LORD)
		{
			iCount = 5;
		}
		else
		{
			iCount = 4;
		}
		for(int i=0; i<iCount; ++i)
		{
			// 테섭이거나 체험 서버이거나 FOR_LAST_TEST 디파인 처리 되어 있으면 휠버튼으로 스텟 올리는거 가능하게 처리
#ifdef KJH_ADD_SERVER_LIST_SYSTEM
			if( g_ServerListManager->IsTestServer() == true
#else // KJH_ADD_SERVER_LIST_SYSTEM
			if(IsTestServer() == TRUE
#endif // KJH_ADD_SERVER_LIST_SYSTEM
#if defined FOR_LAST_TEST || defined EXP_SERVER_PATCH
				|| true
#endif // FOR_LAST_TEST	
				)
			{
				POINT ptStatBtn = {0, 0};
				ptStatBtn = m_BtnStat[i].GetPos();
				if((SEASON3B::IsRepeat(VK_MBUTTON)  && CheckMouseIn(ptStatBtn.x, ptStatBtn.y, 16, 15))
					|| m_BtnStat[i].UpdateMouseEvent() == true )
				{
					SendRequestAddPoint ( i );
					return true;	
				}
			}
			else
			{
				// 스텟 버튼 클릭하면 스텟 포인트 처리
				if(m_BtnStat[i].UpdateMouseEvent() == true)
				{
					SendRequestAddPoint ( i );
					return true;
				}
			}
		}
	}

	
	if(m_BtnExit.UpdateMouseEvent() == true)
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_CHARACTER);
		return true;
	}

	if(m_BtnQuest.UpdateMouseEvent() == true)
	{
		g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MYQUEST);
		return true;
	}

	if(m_BtnPet.UpdateMouseEvent() == true)
	{
		g_pNewUISystem->Toggle(SEASON3B::INTERFACE_PET);
		return true;
	}

	if(m_BtnMasterLevel.UpdateMouseEvent() == true)
	{
		if(IsMasterLevel( Hero->Class ) == true 
#ifdef KJH_FIX_WOPS_K22193_SUMMONER_MASTERSKILL_UI_ABNORMAL_TEXT
			&& GetCharacterClass(Hero->Class) != CLASS_DIMENSIONMASTER			// 2008.06.13 현재는 소환술사 마스터스킬 불가. 풀릴때 define 주석처리
#endif // KJH_FIX_WOPS_K22193_SUMMONER_MASTERSKILL_UI_ABNORMAL_TEXT
#ifdef PBG_ADD_NEWCHAR_MONK
			&& GetCharacterClass(Hero->Class) != CLASS_TEMPLENIGHT
#endif //PBG_ADD_NEWCHAR_MONK
			)
			g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MASTER_LEVEL);
		return true;
	}
	return false;
}

bool SEASON3B::CNewUICharacterInfoWindow::UpdateKeyEvent()
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHARACTER) == true)
	{
		if(SEASON3B::IsPress(VK_ESCAPE) == true)
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_CHARACTER);
			PlayBuffer(SOUND_CLICK01);
	
			return false;
		}
	}

	return true;
}

bool SEASON3B::CNewUICharacterInfoWindow::Update()
{
	return true;
}

void SEASON3B::CNewUICharacterInfoWindow::RenderFrame()
{
	// 프레임
	RenderImage(IMAGE_CHAINFO_BACK, m_Pos.x, m_Pos.y, 190.f, 429.f);
	RenderImage(IMAGE_CHAINFO_TOP, m_Pos.x, m_Pos.y, 190.f, 64.f);
	RenderImage(IMAGE_CHAINFO_LEFT, m_Pos.x, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_CHAINFO_RIGHT, m_Pos.x+190-21, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_CHAINFO_BOTTOM, m_Pos.x, m_Pos.y+429-45, 190.f, 45.f);

	// 테이블 렌더링
	glColor4f(0.0f, 0.0f, 0.0f, 0.3f);
	RenderColor(m_Pos.x+12, m_Pos.y+48, 160, 66);
	EndRenderColor();
	RenderImage(IMAGE_CHAINFO_TABLE_TOP_LEFT, m_Pos.x+12, m_Pos.y+48, 14, 14);
	RenderImage(IMAGE_CHAINFO_TABLE_TOP_RIGHT, m_Pos.x+12+165-14, m_Pos.y+48, 14, 14);
	RenderImage(IMAGE_CHAINFO_TABLE_BOTTOM_LEFT, m_Pos.x+12, m_Pos.y+119-14, 14, 14);
	RenderImage(IMAGE_CHAINFO_TABLE_BOTTOM_RIGHT, m_Pos.x+12+165-14, m_Pos.y+119-14, 14, 14);
	
	// 윗라인, 아래라인
#ifdef _VS2008PORTING
	for(int x=m_Pos.x+12+14; x<m_Pos.x+12+165-14; ++x)
#else // _VS2008PORTING
	int x, y;
	for(x=m_Pos.x+12+14; x<m_Pos.x+12+165-14; ++x)
#endif // _VS2008PORTING
	{
		RenderImage(IMAGE_CHAINFO_TABLE_TOP_PIXEL, x, m_Pos.y+48, 1, 14);
		RenderImage(IMAGE_CHAINFO_TABLE_BOTTOM_PIXEL, x, m_Pos.y+119-14, 1, 14);
	}
	// 중간라인
#ifdef _VS2008PORTING
	for(int x=m_Pos.x+14; x<m_Pos.x+12+165-4; ++x)
#else // _VS2008PORTING
	for(x=m_Pos.x+14; x<m_Pos.x+12+165-4; ++x)
#endif // _VS2008PORTING
	{
		RenderImage(IMAGE_CHAINFO_TABLE_BOTTOM_PIXEL, x, m_Pos.y+48+12, 1, 14);
	}
	// 왼쪽 라인, 오른쪽 라인
#ifdef _VS2008PORTING
	for(int y=m_Pos.y+48+14; y<m_Pos.y+119-14; y++)
#else // _VS2008PORTING
	for(y=m_Pos.y+48+14; y<m_Pos.y+119-14; y++)
#endif // _VS2008PORTING
	{
		RenderImage(IMAGE_CHAINFO_TABLE_LEFT_PIXEL, m_Pos.x+12, y, 14, 1);
		RenderImage(IMAGE_CHAINFO_TABLE_RIGHT_PIXEL, m_Pos.x+12+165-14, y, 14, 1);
	}

	// 텍스트박스
	RenderImage(IMAGE_CHAINFO_TEXTBOX, m_Pos.x+11, m_Pos.y+HEIGHT_STRENGTH, 170.f, 21.f);
	RenderImage(IMAGE_CHAINFO_TEXTBOX, m_Pos.x+11, m_Pos.y+HEIGHT_DEXTERITY, 170.f, 21.f);
	RenderImage(IMAGE_CHAINFO_TEXTBOX, m_Pos.x+11, m_Pos.y+HEIGHT_VITALITY, 170.f, 21.f);
	RenderImage(IMAGE_CHAINFO_TEXTBOX, m_Pos.x+11, m_Pos.y+HEIGHT_ENERGY, 170.f, 21.f);
	if(GetBaseClass(Hero->Class) == CLASS_DARK_LORD)
	{
		RenderImage(IMAGE_CHAINFO_TEXTBOX, m_Pos.x+11, m_Pos.y+HEIGHT_CHARISMA, 170.f, 21.f);
	}
}

bool SEASON3B::CNewUICharacterInfoWindow::Render()
{
	EnableAlphaTest();

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// 프레임을 그린다.
	RenderFrame();
	
	// 텍스트를 그린다.
	RenderTexts();

	// 버튼을 그린다.
	RenderButtons();

	DisableAlphaBlend();

	return true;
}

void SEASON3B::CNewUICharacterInfoWindow::RenderTexts()
{
	// ID, 서버명, 클래스명
	RenderSubjectTexts();

	// 레벨, 경험치, 포인트
	RenderTableTexts();

	// 속성
	RenderAttribute();
}

void SEASON3B::CNewUICharacterInfoWindow::RenderSubjectTexts()
{
	// 캐릭터 ID는 무조건 안시이다.
	char strID[256];
	sprintf(strID, "%s", CharacterAttribute->Name);
	// 캐릭터 클래스명
	unicode::t_char strClassName[256];
	unicode::_sprintf(strClassName, "(%s)", GetCharacterClassText(CharacterAttribute->Class));
	
	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetBgColor(20, 20, 20, 20);
	// 캐릭터ID 칼라 세팅
	SetPlayerColor(Hero->PK);
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y+12, strID, 190, 0, RT3_SORT_CENTER);

	// 클래스명과 서버명
#ifdef KJH_ADD_SERVER_LIST_SYSTEM
	unicode::t_char strServerName[MAX_TEXT_LENGTH];
#ifdef ASG_MOD_SERVER_LIST_ADD_CHARGED_CHANNEL
	const char* apszGlobalText[4]
		= { GlobalText[461], GlobalText[460], GlobalText[3130], GlobalText[3131] };
	sprintf(strServerName, apszGlobalText[g_ServerListManager->GetNonPVPInfo()],
		g_ServerListManager->GetSelectServerName(), g_ServerListManager->GetSelectServerIndex());
#else	// ASG_MOD_SERVER_LIST_ADD_CHARGED_CHANNEL
	if(g_ServerListManager->IsNonPvP() == true)		// NonPvP인가?
	{
		sprintf(strServerName, GlobalText[460], 
			g_ServerListManager->GetSelectServerName(), 
			g_ServerListManager->GetSelectServerIndex());
	}
	else
	{
		sprintf(strServerName, GlobalText[461], 
			g_ServerListManager->GetSelectServerName(), 
			g_ServerListManager->GetSelectServerIndex());
	}
#endif	// ASG_MOD_SERVER_LIST_ADD_CHARGED_CHANNEL
#else KJH_ADD_SERVER_LIST_SYSTEM
	unicode::t_char strServerName[256];
#if defined	ADD_EXPERIENCE_SERVER
	// 테스트 서버이고, 서버 번호가 10 이상 이면.
	// 559 "테스트"
	if (0 == unicode::_strcmp(ServerList[ServerSelectHi].Name, GlobalText[559]) && ServerLocalSelect >= 10)
	{
		// 461 "[%s-%d서버]"
		// 1699 "시즌2체험(PC방)"
		unicode::_sprintf(strServerName, GlobalText[461], GlobalText[1699], ServerLocalSelect - 9);
	}
	else
#elif defined ADD_EVENT_SERVER_NAME
	// 테스트 서버이고, 서버 번호가 6 이상 이면.
	// 559 "테스트"
	if (0 == unicode::_strcmp(ServerList[ServerSelectHi].Name, GlobalText[559]) && ServerLocalSelect >= 6)
	{
		// 461 "[%s-%d서버]"
		// 19 "이벤트"
		unicode::_sprintf(strServerName, GlobalText[461], GlobalText[19], ServerLocalSelect - 5);
	}
	else
#endif	// ADD_EXPERIENCE_SERVER ||	ADD_EVENT_SERVER_NAME
	{
		if(IsNonPvpServer(ServerSelectHi, ServerLocalSelect))
		{
			// 460 "[%s-%d(Non-PVP)서버]"
			unicode::_sprintf(strServerName, GlobalText[460], ServerList[ServerSelectHi].Name, ServerLocalSelect);
		}
		else
		{
			// 461 "[%s-%d서버]"
			unicode::_sprintf(strServerName, GlobalText[461], ServerList[ServerSelectHi].Name, ServerLocalSelect);
		}
	}	
#endif // KJH_ADD_SERVER_LIST_SYSTEM
		

    float fAlpha = sinf(WorldTime * 0.001f) + 1.f;
	g_pRenderText->SetTextColor(255, 255, 255, 127*(2.f-fAlpha));
    g_pRenderText->RenderText(m_Pos.x, m_Pos.y+27, strClassName, 190, 0, RT3_SORT_CENTER); //  캐릭 클래스.
	g_pRenderText->SetTextColor(255, 255, 255, 127*fAlpha);
    g_pRenderText->RenderText(m_Pos.x, m_Pos.y+27, strServerName, 190, 0, RT3_SORT_CENTER);  //  서버명.
}

void SEASON3B::CNewUICharacterInfoWindow::RenderTableTexts()
{
	// 레벨
	unicode::t_char strLevel[32];
	unicode::t_char strExp[128];
	unicode::t_char strPoint[128];

	if(IsMasterLevel(CharacterAttribute->Class) == true)
	{
		unicode::_sprintf(strLevel, GlobalText[1745]);
		unicode::_sprintf(strExp, "----------");
	}
	else
	{
		// 200 "레  벨: %d"
		unicode::_sprintf(strLevel, GlobalText[200], CharacterAttribute->Level);
		// 201 "경험치: %u/%u"
		unicode::_sprintf(strExp, GlobalText[201], CharacterAttribute->Experience, CharacterAttribute->NextExperince);
		//
	}
	if(CharacterAttribute->Level > 9) // 레벨 10 이상일때
	{
		int iMinus, iMaxMinus;
		if(CharacterAttribute->wMinusPoint == 0)
		{
			iMinus = 0;
		}
		else
		{
			iMinus = -CharacterAttribute->wMinusPoint;
		}

		iMaxMinus = -CharacterAttribute->wMaxMinusPoint;

		// 1412 "생성"
		// 1903 "감소"
		unicode::_sprintf(strPoint, "%s %d/%d | %s %d/%d", 
			GlobalText[1412], CharacterAttribute->AddPoint, CharacterAttribute->MaxAddPoint, 
			GlobalText[1903], iMinus, iMaxMinus);
	}
	else	// 레벨 9 이하일때  
	{
		// 1412 "생성"
		// 1903 "감소"
		wsprintf(strPoint, "%s %d/%d | %s %d/%d", GlobalText[1412], 0, 0, GlobalText[1903], 0, 0);
	}
	
	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetTextColor(230, 230, 0, 255);
	g_pRenderText->SetBgColor(0, 0, 0, 0);
	g_pRenderText->RenderText(m_Pos.x+18, m_Pos.y+58, strLevel);


	// 레벨업 포인트
	if(CharacterAttribute->LevelUpPoint > 0)
	{
		unicode::t_char strLevelUpPoint[128];
		// 217 "포인트: %d"
		if(IsMasterLevel(CharacterAttribute->Class) == false || CharacterAttribute->LevelUpPoint > 0)
		{
			unicode::_sprintf(strLevelUpPoint, GlobalText[217], CharacterAttribute->LevelUpPoint);
		}
		else
			unicode::_sprintf(strLevelUpPoint, "");
		g_pRenderText->SetFont(g_hFontBold);
		g_pRenderText->SetTextColor(255, 138, 0, 255);
		g_pRenderText->SetBgColor(0, 0, 0, 0);
		g_pRenderText->RenderText(m_Pos.x+110, m_Pos.y+58, strLevelUpPoint);
	}
	
	// 경험치
	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->SetBgColor(0, 0, 0, 0);
	g_pRenderText->RenderText(m_Pos.x+18, m_Pos.y+75, strExp);

	// 생성, 감소 성공 확률 표시
	int iAddPoint, iMinusPoint;
	// 생성확률
	if(CharacterAttribute->AddPoint <= 10)	// 10포인트까지는 무조건 100%
	{
		iAddPoint = 100;
	}
	else
	{
		int iTemp = 0;
		if(CharacterAttribute->MaxAddPoint != 0)
		{
			iTemp = (CharacterAttribute->AddPoint*100)/CharacterAttribute->MaxAddPoint;
		}
		if(iTemp <= 10)
		{
			iAddPoint = 70;
		}
		else if(iTemp > 10 && iTemp <= 30)
		{
			iAddPoint = 60;
		}
		else if(iTemp > 30 && iTemp <= 50)
		{
			iAddPoint = 50;
		}
		else if(iTemp > 50)
		{
			iAddPoint = 40;
		}
	}

	// 감소확률
	if(CharacterAttribute->wMinusPoint <= 10)	// 10포인트까지는 무조건 100%
	{
		iMinusPoint = 100;
	}
	else
	{
		int iTemp = 0;
		if(CharacterAttribute->wMaxMinusPoint != 0)
		{
			iTemp = (CharacterAttribute->wMinusPoint*100)/CharacterAttribute->wMaxMinusPoint;
		}

		if(iTemp <= 10)
		{
			iMinusPoint = 70;
		}
		else if(iTemp > 10 && iTemp <= 30)
		{
			iMinusPoint = 60;
		}
		else if(iTemp > 30 && iTemp <= 50)
		{
			iMinusPoint = 50;
		}
		else if(iTemp > 50)
		{
			iMinusPoint = 40;
		}
	}


	unicode::t_char strPointProbability[128];
	// 1907 "생성:%d%%|감소:%d%%"
	unicode::_sprintf(strPointProbability, GlobalText[1907], iAddPoint, iMinusPoint);
	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(76, 197, 254, 255);
	g_pRenderText->SetBgColor(0, 0, 0, 0);
	g_pRenderText->RenderText(m_Pos.x+18, m_Pos.y+88, strPointProbability);

	g_pRenderText->SetTextColor(76, 197, 254, 255);
	g_pRenderText->SetBgColor(0, 0, 0, 0);
	g_pRenderText->RenderText(m_Pos.x+18, m_Pos.y+101, strPoint);
}

void SEASON3B::CNewUICharacterInfoWindow::RenderAttribute()
{
	//////////////////////////////////////////////////////////////////////////
	//						         힘										//
	//////////////////////////////////////////////////////////////////////////

	g_pRenderText->SetFont(g_hFontBold);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

	WORD wStrength;

	// memorylock 사용
	wStrength = CharacterAttribute->Strength + CharacterAttribute->AddStrength;

	// memorylock 사용
#ifdef PSW_SECRET_ITEM
	if( g_isCharacterBuff((&Hero->Object), eBuff_SecretPotion1) )
	{
		g_pRenderText->SetTextColor(255, 120, 0, 255);
	}
	else
#endif //PSW_SECRET_ITEM
    if(CharacterAttribute->AddStrength)
    {
		g_pRenderText->SetTextColor(100, 150, 255, 255);
    }
    else
    {
		g_pRenderText->SetTextColor(230, 230, 0, 255);
    }

	unicode::t_char strStrength[32];
	unicode::_sprintf(strStrength, "%d", wStrength);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

	g_pRenderText->SetBgColor(0);
	// 166 " 힘 "
	g_pRenderText->RenderText(m_Pos.x+12, m_Pos.y+HEIGHT_STRENGTH+6, GlobalText[166], 74, 0, RT3_SORT_CENTER);
	g_pRenderText->RenderText(m_Pos.x+86, m_Pos.y+HEIGHT_STRENGTH+6, strStrength, 86, 0, RT3_SORT_CENTER);

	//  공격력. 율. 계산 부분
	unicode::t_char strAttakMamage[256];
	int iAttackDamageMin = 0;
	int iAttackDamageMax = 0;

	// 함수내부에서 memorylock 사용
	bool bAttackDamage = GetAttackDamage( &iAttackDamageMin, &iAttackDamageMax );

#ifndef _VS2008PORTING			// #ifndef
	int i = 0;
#endif // _VS2008PORTING
	int Add_Dex = 0;		//대인공격율추가
	int Add_Rat = 0;		//공격성공율추가
	int Add_Dfe = 0;		//대인방어율추가
	int Add_Att_Max = 0;	//캐릭터공격력추가(max)
	int Add_Att_Min = 0;	//캐릭터공격력추가(min)
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	int Add_Dfe_Rat = 0;	//캐릭터 방어율추가
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	int Add_Ch_Dfe = 0;		//캐릭터 방어력추가
#ifdef PJH_SEASON4_MASTER_RANK4
	//AT_SKILL_MAX_MANA_UP
	int Add_Mana_Max = 0;
	int Add_Mana_Min = 0;
#endif //
#ifdef _VS2008PORTING
	for(int i=0; i<MAX_MAGIC; ++i)
#else // _VS2008PORTING
	for(i=0; i<MAX_MAGIC; ++i)
#endif // _VS2008PORTING
	{
		if(CharacterAttribute->Skill[i] >= AT_SKILL_TOMAN_ATTACKUP && CharacterAttribute->Skill[i] < AT_SKILL_TOMAN_ATTACKUP + 5)
		{
			Add_Dex = SkillAttribute[CharacterAttribute->Skill[i]].Damage;
		}
		else
		if(CharacterAttribute->Skill[i] >= AT_SKILL_ATTACK_RATEUP && CharacterAttribute->Skill[i] < AT_SKILL_ATTACK_RATEUP + 5)
		{
			Add_Rat = SkillAttribute[CharacterAttribute->Skill[i]].Damage;
		}
		else
		if(CharacterAttribute->Skill[i] >= AT_SKILL_TOMAN_DEFENCEUP && CharacterAttribute->Skill[i] < AT_SKILL_TOMAN_DEFENCEUP + 5)
		{
			Add_Dfe = SkillAttribute[CharacterAttribute->Skill[i]].Damage;
		}
		else
		if(CharacterAttribute->Skill[i] >= AT_SKILL_DEF_UP && CharacterAttribute->Skill[i] < AT_SKILL_DEF_UP + 5)
		{
			Add_Ch_Dfe = SkillAttribute[CharacterAttribute->Skill[i]].Damage;
		}
//		else
//		if(CharacterAttribute->Skill[i] >= AT_SKILL_MAX_HP_UP && CharacterAttribute->Skill[i] < AT_SKILL_MAX_HP_UP + 5)
//		{
//			Add_Life = SkillAttribute[CharacterAttribute->Skill[i]].Damage;
//		}
//		else
//		if(CharacterAttribute->Skill[i] >= AT_SKILL_MAX_AG_UP && CharacterAttribute->Skill[i] < AT_SKILL_MAX_AG_UP + 5)
//		{
//			Add_Ag = SkillAttribute[CharacterAttribute->Skill[i]].Damage;
//		}
		else
		if(CharacterAttribute->Skill[i] >= AT_SKILL_MAX_ATTACKRATE_UP && CharacterAttribute->Skill[i] < AT_SKILL_MAX_ATTACKRATE_UP + 5)
		{
			Add_Att_Max = SkillAttribute[CharacterAttribute->Skill[i]].Damage;
		}
#ifdef PJH_SEASON4_MASTER_RANK4
		else
		if(CharacterAttribute->Skill[i] >= AT_SKILL_MAX_ATT_MAGIC_UP && CharacterAttribute->Skill[i] < AT_SKILL_MAX_ATT_MAGIC_UP + 5)
		{
			Add_Mana_Max = Add_Att_Max = SkillAttribute[CharacterAttribute->Skill[i]].Damage;
		}
		else
		if(CharacterAttribute->Skill[i] >= AT_SKILL_MIN_ATT_MAGIC_UP && CharacterAttribute->Skill[i] < AT_SKILL_MIN_ATT_MAGIC_UP + 5)
		{
			Add_Mana_Min = Add_Att_Min = SkillAttribute[CharacterAttribute->Skill[i]].Damage;
		}
		else
		if(CharacterAttribute->Skill[i] >= AT_SKILL_MAX_MANA_UP && CharacterAttribute->Skill[i] < AT_SKILL_MAX_MANA_UP + 5)
		{
			Add_Mana_Max = SkillAttribute[CharacterAttribute->Skill[i]].Damage;
		}
		else
		if(CharacterAttribute->Skill[i] >= AT_SKILL_MIN_MANA_UP && CharacterAttribute->Skill[i] < AT_SKILL_MIN_MANA_UP + 5)
		{
			Add_Mana_Min = SkillAttribute[CharacterAttribute->Skill[i]].Damage;
		}
#endif //PJH_SEASON4_MASTER_RANK4
		else
		if(CharacterAttribute->Skill[i] >= AT_SKILL_MIN_ATTACKRATE_UP && CharacterAttribute->Skill[i] < AT_SKILL_MIN_ATTACKRATE_UP + 5)
		{
			Add_Att_Min = SkillAttribute[CharacterAttribute->Skill[i]].Damage;
		}
		//AT_SKILL_MIN_ATTACKRATE_UP
	}



	// memorylock 사용
	ITEM* pWeaponRight = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT];
	ITEM* pWeaponLeft = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT];
	
	// memorylock 사용
    int iAttackRating = CharacterAttribute->AttackRating + Add_Rat;
	int iAttackRatingPK = CharacterAttribute->AttackRatingPK + Add_Dex;
#ifdef PJH_SEASON4_MASTER_RANK4
	iAttackDamageMax += Add_Att_Max;
	iAttackDamageMin += Add_Att_Min;
#endif //PJH_SEASON4_MASTER_RANK4

	if( g_isCharacterBuff((&Hero->Object), eBuff_AddAG) ) 
    {
    	WORD wDexterity;
		// memorylock 사용
	    wDexterity = CharacterAttribute->Dexterity+ CharacterAttribute->AddDexterity;
        iAttackRating += wDexterity;
		iAttackRatingPK += wDexterity;
        if(PartyNumber >= 3)
        {
			int iPlusRating = (wDexterity * ((PartyNumber - 2) * 0.01f));
            iAttackRating += iPlusRating;
			iAttackRatingPK = iPlusRating;
        }
    }
	if( g_isCharacterBuff((&Hero->Object), eBuff_HelpNpc) )
	{
		int iTemp = 0;
		// memorylock 사용
		if(CharacterAttribute->Level > 180)
		{
			iTemp = (180 / 3) + 45;
		}
		else
		{
			// memorylock 사용
			iTemp = (CharacterAttribute->Level / 3) + 45;
		}
		
		iAttackDamageMin += iTemp;
		iAttackDamageMax += iTemp;
	}

#ifdef ASG_ADD_SKILL_BERSERKER
	// 버서커 버프에 걸려 있는가?
	if (g_isCharacterBuff((&Hero->Object), eBuff_Berserker))
	{
		int nTemp = CharacterAttribute->Strength + CharacterAttribute->AddStrength
			+ CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
		float fTemp = int(CharacterAttribute->Energy / 30) / 100.f;
		iAttackDamageMin += nTemp / 7 * fTemp;
		iAttackDamageMax += nTemp / 4 * fTemp;
	}
#endif	// ASG_ADD_SKILL_BERSERKER

	int iMinIndex = 0, iMaxIndex = 0, iMagicalIndex = 0; 

	StrengthenCapability SC_r, SC_l;

	int rlevel = (pWeaponRight->Level>>3)&15;

	if( rlevel >= pWeaponRight->Jewel_Of_Harmony_OptionLevel )
	{
		g_pUIJewelHarmonyinfo->GetStrengthenCapability( &SC_r, pWeaponRight, 1 );

		if( SC_r.SI_isSP )
		{
			iMinIndex     = SC_r.SI_SP.SI_minattackpower;
			iMaxIndex     = SC_r.SI_SP.SI_maxattackpower;
			iMagicalIndex = SC_r.SI_SP.SI_magicalpower;
		}
	}

	int llevel = (pWeaponLeft->Level>>3)&15;

	if( llevel >= pWeaponLeft->Jewel_Of_Harmony_OptionLevel )
	{
		g_pUIJewelHarmonyinfo->GetStrengthenCapability( &SC_l, pWeaponLeft, 1 );

		if( SC_l.SI_isSP )
		{
			iMinIndex     += SC_l.SI_SP.SI_minattackpower;
			iMaxIndex     += SC_l.SI_SP.SI_maxattackpower;
			iMagicalIndex += SC_l.SI_SP.SI_magicalpower;
		}
	}

	int iDefenseRate = 0, iAttackPowerRate = 0;
	
	StrengthenCapability rightinfo, leftinfo;

	int iRightLevel = (pWeaponRight->Level>>3)&15;

	if( iRightLevel >= pWeaponRight->Jewel_Of_Harmony_OptionLevel )
	{
		g_pUIJewelHarmonyinfo->GetStrengthenCapability( &rightinfo, pWeaponRight, 1 );
	}

	int iLeftLevel = (pWeaponLeft->Level>>3)&15;

	if( iLeftLevel >= pWeaponLeft->Jewel_Of_Harmony_OptionLevel )
	{
		g_pUIJewelHarmonyinfo->GetStrengthenCapability( &leftinfo, pWeaponLeft, 1 );
	}

	if( rightinfo.SI_isSP )
	{
		iAttackPowerRate += rightinfo.SI_SP.SI_attackpowerRate;
	}
	if( leftinfo.SI_isSP )
	{
		iAttackPowerRate += leftinfo.SI_SP.SI_attackpowerRate;
	}

	for( int k = EQUIPMENT_WEAPON_LEFT; k < MAX_EQUIPMENT; ++k )
	{
		StrengthenCapability defenseinfo;

		// memorylock 사용
		ITEM* pItem = &CharacterMachine->Equipment[k];

		int eqlevel = (pItem->Level>>3)&15;

		if( eqlevel >= pItem->Jewel_Of_Harmony_OptionLevel )
		{
			g_pUIJewelHarmonyinfo->GetStrengthenCapability( &defenseinfo, pItem, 2 );
		}

		if( defenseinfo.SI_isSD )
		{
			iDefenseRate += defenseinfo.SI_SD.SI_defenseRate;
		}
	}

	int itemoption380Attack = 0;
	int itemoption380Defense = 0;
	
	for(int j=0 ; j<MAX_EQUIPMENT; ++j)
	{
		// memorylock 사용
		bool is380item = CharacterMachine->Equipment[j].option_380;
		int i380type  = CharacterMachine->Equipment[j].Type;
		
		if( is380item && i380type > -1 && i380type < MAX_ITEM )
		{
			ITEM_ADD_OPTION item380option;
			
			// memorylock 사용
			item380option = g_pItemAddOptioninfo->GetItemAddOtioninfo( CharacterMachine->Equipment[j].Type );

			//공격력
			if( item380option.m_byOption1 == 1 )
			{
				itemoption380Attack += item380option.m_byValue1;
			}

			if( item380option.m_byOption2 == 1 )
			{
				itemoption380Attack += item380option.m_byValue2;
			}

			//방어력
			if( item380option.m_byOption1 == 3 )
			{
				itemoption380Defense += item380option.m_byValue1;
			}

			if( item380option.m_byOption2 == 3)
			{
				itemoption380Defense += item380option.m_byValue2;
			}
		}
	}

	ITEM *pItemRingLeft, *pItemRingRight;
	
	// 크리스마스 변신 반지를 착용하고 있다면 추가 공격력 20을 더해준다.
	// memorylock 사용
	pItemRingLeft = &CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];
	pItemRingRight = &CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];
	if(pItemRingLeft && pItemRingRight)
	{
#ifdef YDG_FIX_CHANGERING_STATUS_EXPIRE_CHECK
		// 만기 체크
		int iNonExpiredLRingType = -1;
		int iNonExpiredRRingType = -1;
#ifdef LJH_FIX_IGNORING_EXPIRATION_PERIOD
		iNonExpiredLRingType = pItemRingLeft->Type;
		iNonExpiredRRingType = pItemRingRight->Type;
#else  //LJH_FIX_IGNORING_EXPIRATION_PERIOD
		if(!pItemRingLeft->bPeriodItem || !pItemRingLeft->bExpiredPeriod)	// 왼손
		{
			iNonExpiredLRingType = pItemRingLeft->Type;
		}
		if(!pItemRingRight->bPeriodItem || !pItemRingRight->bExpiredPeriod)	// 오른손
		{
			iNonExpiredRRingType = pItemRingRight->Type;
		}
#endif //LJH_FIX_IGNORING_EXPIRATION_PERIOD

		// 착용중인 변신 반지들의 데미지 증가 중에 가장 높은 값을 저장 
		int maxIAttackDamageMin = 0;
		int maxIAttackDamageMax = 0;
		if(iNonExpiredLRingType == ITEM_HELPER+41 || iNonExpiredRRingType == ITEM_HELPER+41)
		{
			// 착용중인 변신 반지들 중 크리스마스 변신 반지에 의해 가장 높은 공격데미지 추가 
			maxIAttackDamageMin = max(maxIAttackDamageMin, 20);	
			maxIAttackDamageMax = max(maxIAttackDamageMax, 20);
		}
#ifdef PJH_ADD_PANDA_CHANGERING
		if(iNonExpiredLRingType == ITEM_HELPER+76 || iNonExpiredRRingType == ITEM_HELPER+76)
		{
			// 착용중인 변신 반지들 중 팬더 변신 반지에 의해 가장 높은 공격데미지 추가 
			maxIAttackDamageMin = max(maxIAttackDamageMin, 30);	
			maxIAttackDamageMax = max(maxIAttackDamageMax, 30);
		}
#endif //PJH_ADD_PANDA_CHANGERING
#ifdef YDG_ADD_SKELETON_CHANGE_RING
		if(iNonExpiredLRingType == ITEM_HELPER+122 || iNonExpiredRRingType == ITEM_HELPER+122)	// 스켈레톤 변신반지
		{
			// 착용중인 변신 반지들 중 스켈레톤 변신 반지에 의해 가장 높은 공격데미지 추가 
			maxIAttackDamageMin = max(maxIAttackDamageMin, 40);	
			maxIAttackDamageMax = max(maxIAttackDamageMax, 40);
		}
#endif	// YDG_ADD_SKELETON_CHANGE_RING

		// 최종 구해진 마법공격력에 착용중인 변신 반지들의 데미지 증가 중에 가장 높은 값을 더해준다.
		iAttackDamageMin += maxIAttackDamageMin;	// 최소 공격 데미지
		iAttackDamageMax += maxIAttackDamageMax;	// 최대 공격 데미지


#else	// YDG_FIX_CHANGERING_STATUS_EXPIRE_CHECK	// 소스정리시 삭제할 부분!!!
#ifdef LJH_FIX_CHANGE_RING_DAMAGE_BUG
		// 착용중인 변신 반지들의 데미지 증가 중에 가장 높은 값을 저장 
		int maxIAttackDamageMin = 0;
		int maxIAttackDamageMax = 0;

		if(pItemRingLeft->Type == ITEM_HELPER+41 || pItemRingRight->Type == ITEM_HELPER+41)
		{
			// 착용중인 변신 반지들 중 크리스마스 변신 반지에 의해 가장 높은 공격데미지 추가 
			maxIAttackDamageMin = max(maxIAttackDamageMin, 20);	
			maxIAttackDamageMax = max(maxIAttackDamageMax, 20);
		}
#ifdef PJH_ADD_PANDA_CHANGERING
		if(pItemRingLeft->Type == ITEM_HELPER+76 || pItemRingRight->Type == ITEM_HELPER+76)
		{
			// 착용중인 변신 반지들 중 팬더 변신 반지에 의해 가장 높은 공격데미지 추가 
			maxIAttackDamageMin = max(maxIAttackDamageMin, 30);	
			maxIAttackDamageMax = max(maxIAttackDamageMax, 30);
		}
#endif //PJH_ADD_PANDA_CHANGERING

		// 최종 구해진 마법공격력에 착용중인 변신 반지들의 데미지 증가 중에 가장 높은 값을 더해준다.
		iAttackDamageMin += maxIAttackDamageMin;	// 최소 공격 데미지
		iAttackDamageMax += maxIAttackDamageMax;	// 최대 공격 데미지

#else	// LJH_FIX_CHANGE_RING_DAMAGE_BUG
		if(pItemRingLeft->Type == ITEM_HELPER+41 || pItemRingRight->Type == ITEM_HELPER+41)
		{
			// 최종 구해진 공격력에다가 20를 더해준다.
			iAttackDamageMin += 20;	// 최소 공격 데미지
			iAttackDamageMax += 20;	// 최대 공격 데미지
		}
#ifdef PJH_ADD_PANDA_CHANGERING
		else
		if(pItemRingLeft->Type == ITEM_HELPER+76 || pItemRingRight->Type == ITEM_HELPER+76)
		{
			// 최종 구해진 공격력에다가 20를 더해준다.
			iAttackDamageMin += 30;	// 최소 공격 데미지
			iAttackDamageMax += 30;	// 최대 공격 데미지
		}
#endif //PJH_ADD_PANDA_CHANGERING
#endif	// LJH_FIX_CHANGE_RING_DAMAGE_BUG
#endif	// YDG_FIX_CHANGERING_STATUS_EXPIRE_CHECK	// 소스정리시 삭제할 부분!!!
	}
	
	// memorylock 사용
	ITEM *pItemHelper = &CharacterMachine->Equipment[EQUIPMENT_HELPER];
	if(pItemHelper)
	{
		if(pItemHelper->Type == ITEM_HELPER+37 && pItemHelper->Option1 == 0x04)
		{
			// 환영 펜릴은 공격력은
			// memorylock 사용
			WORD wLevel = CharacterAttribute->Level;
			// 최종 구해진 공격력에다가 (Level/25)를 더해준다.
			iAttackDamageMin += (wLevel / 12);	// 최소 공격 데미지
			iAttackDamageMax += (wLevel / 12);	// 최대 공격 데미지
		}
#ifdef LDK_FIX_PC4_GUARDIAN_DEMON_INFO
		if(pItemHelper->Type == ITEM_HELPER+64)
		{
#ifdef LDK_MOD_PC4_GUARDIAN_EXPIREDPERIOD_NOTPRINT_INFO
			//만료된 기간제 아이템이면 출력 안함
			if(false == pItemHelper->bExpiredPeriod)
			{
				// memorylock 사용
				iAttackDamageMin += int(float(iAttackDamageMin) * 0.4f);	// 최소 공격 데미지
				iAttackDamageMax += int(float(iAttackDamageMax) * 0.4f);	// 최대 공격 데미지
			}
#else //LDK_MOD_PC4_GUARDIAN_EXPIREDPERIOD_NOTPRINT_INFO
			// memorylock 사용
			iAttackDamageMin += int(float(iAttackDamageMin) * 0.4f);	// 최소 공격 데미지
			iAttackDamageMax += int(float(iAttackDamageMax) * 0.4f);	// 최대 공격 데미지
#endif //LDK_MOD_PC4_GUARDIAN_EXPIREDPERIOD_NOTPRINT_INFO
		}
#endif //LDK_FIX_PC4_GUARDIAN_DEMON_INFO
#ifdef YDG_ADD_SKELETON_PET
		if(pItemHelper->Type == ITEM_HELPER+123)	// 스켈레톤 펫
		{
#ifdef LJH_FIX_IGNORING_EXPIRATION_PERIOD
			// memorylock 사용
			iAttackDamageMin += int(float(iAttackDamageMin) * 0.2f);	// 최소 공격 데미지
			iAttackDamageMax += int(float(iAttackDamageMax) * 0.2f);	// 최대 공격 데미지
#else  //LJH_FIX_IGNORING_EXPIRATION_PERIOD
			//만료된 기간제 아이템이면 출력 안함
			if(false == pItemHelper->bExpiredPeriod)
			{
				// memorylock 사용
				iAttackDamageMin += int(float(iAttackDamageMin) * 0.2f);	// 최소 공격 데미지
				iAttackDamageMax += int(float(iAttackDamageMax) * 0.2f);	// 최대 공격 데미지
			}
#endif //LJH_FIX_IGNORING_EXPIRATION_PERIOD
		}
#endif	// YDG_ADD_SKELETON_PET
#ifdef PBG_FIX_SATAN_VALUEINCREASE
		if(pItemHelper->Type == ITEM_HELPER+1 
#if SELECTED_LANGUAGE == LANGUAGE_KOREAN	//해외에서 기간제 사용시 디파인제거 할것
			&& !(pItemHelper->bExpiredPeriod)
#endif //SELECTED_LANGUAGE == LANGUAGE_KOREAN
			)
		{
			iAttackDamageMin += int(float(iAttackDamageMin) * 0.3f);
			iAttackDamageMax += int(float(iAttackDamageMax) * 0.3f);
		}
#endif //PBG_FIX_SATAN_VALUEINCREASE
	}

    if( iAttackRating > 0 )
    {
		// 203 "공격력(율): %d~%d (%d)"
		if( iAttackDamageMin + iMinIndex >= iAttackDamageMax + iMaxIndex )
		{
			unicode::_sprintf(strAttakMamage, GlobalText[203], iAttackDamageMax + iMaxIndex, iAttackDamageMax + iMaxIndex, iAttackRating);
		}
		else
		{
			unicode::_sprintf(strAttakMamage, GlobalText[203], iAttackDamageMin + iMinIndex, iAttackDamageMax + iMaxIndex, iAttackRating);
		}
    }
    else
    {
		// 204 "공 격 력: %d~%d"
		if( iAttackDamageMin + iMinIndex >= iAttackDamageMax + iMaxIndex )
		{
			unicode::_sprintf(strAttakMamage, GlobalText[204], iAttackDamageMax + iMaxIndex, iAttackDamageMax + iMaxIndex);
		}
		else
		{
			unicode::_sprintf(strAttakMamage, GlobalText[204], iAttackDamageMin + iMinIndex, iAttackDamageMax + iMaxIndex);
		}
    }

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetBgColor(0, 0, 0, 0);

	if(bAttackDamage)
	{
		g_pRenderText->SetTextColor(100, 150, 255, 255);
	}
	else
	{
		g_pRenderText->SetTextColor(255, 255, 255, 255);
	}

	if(g_isCharacterBuff((&Hero->Object), eBuff_Hellowin2)) // 잭오랜턴의 분노
	{
		g_pRenderText->SetTextColor(255, 0, 240, 255);
	}

#ifdef PSW_SCROLL_ITEM
	if( g_isCharacterBuff((&Hero->Object), eBuff_EliteScroll3) )
	{
		g_pRenderText->SetTextColor(255, 0, 240, 255);
	}
#endif //PSW_SCROLL_ITEM

#ifdef CSK_EVENT_CHERRYBLOSSOM
	if( g_isCharacterBuff((&Hero->Object), eBuff_CherryBlossom_Petal) )
	{
		g_pRenderText->SetTextColor(255, 0, 240, 255);
	}
#endif //CSK_EVENT_CHERRYBLOSSOM

	int iY = HEIGHT_STRENGTH+25;
	g_pRenderText->RenderText(m_Pos.x+20, m_Pos.y+HEIGHT_STRENGTH+25, strAttakMamage);

	if(iAttackRatingPK > 0)
    {
		if( itemoption380Attack != 0 || iAttackPowerRate != 0 )
		{
			// 2109 "대인 공격율: %d (+%d)"
			unicode::_sprintf(strAttakMamage, GlobalText[2109], iAttackRatingPK, itemoption380Attack + iAttackPowerRate );
		}
		else
		{
			// 2044 "대인 공격율: %d"
			unicode::_sprintf(strAttakMamage, GlobalText[2044], iAttackRatingPK);
		}

		iY += 13;
		g_pRenderText->RenderText(m_Pos.x+20, m_Pos.y+iY, strAttakMamage);	
    }

	//////////////////////////////////////////////////////////////////////////
	//							민첩성										//
	//////////////////////////////////////////////////////////////////////////

	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetBgColor(0);


	// 추가 민첩성이 있으면 색깔 다르게
	// memorylock 사용
#ifdef PSW_SECRET_ITEM
	if( g_isCharacterBuff((&Hero->Object), eBuff_SecretPotion2) )
	{
		g_pRenderText->SetTextColor(255, 120, 0, 255);
	}
	else
#endif //PSW_SECRET_ITEM
    if( CharacterAttribute->AddDexterity )
    {
		g_pRenderText->SetTextColor(100, 150, 255, 255);
    }
    else
    {
		g_pRenderText->SetTextColor(230, 230, 0, 255);
    }

	unicode::t_char strDexterity[32];
	// 민첩성 계산
	WORD wDexterity;
	// 민첩성은 캐릭터의 민첩성에 추가 민첩성을 더해서 계산
	// memorylock 사용
	wDexterity = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
	unicode::_sprintf(strDexterity, "%d", wDexterity);
	// 1702 "민첩성"
	g_pRenderText->RenderText(m_Pos.x+12, m_Pos.y+HEIGHT_DEXTERITY+6, GlobalText[1702], 74, 0, RT3_SORT_CENTER);
	g_pRenderText->RenderText(m_Pos.x+86, m_Pos.y+HEIGHT_DEXTERITY+6, strDexterity, 86, 0, RT3_SORT_CENTER);

	//민첩성 계산 부분 
	bool bDexSuccess = true;
	int iBaseClass = GetBaseClass(Hero->Class);

	// 장비를 전부 착용하고 있는가 검사
#ifdef _VS2008PORTING
	for(int i=EQUIPMENT_HELM; i<=EQUIPMENT_BOOTS; ++i)
#else // _VS2008PORTING
	i = 0;
	for(i=EQUIPMENT_HELM; i<=EQUIPMENT_BOOTS; ++i)
#endif // _VS2008PORTING
	{
        //  마검일 경우 헬멧을 제외한 장비.
        if( iBaseClass == CLASS_DARK )
        {
			// 장비를 하나라도 착용하고 있지 않거나
			// 장비중 내구도가 0이면 Success는 false
			// memorylock 사용
		    if((CharacterMachine->Equipment[i].Type == -1 && ( i!=EQUIPMENT_HELM && iBaseClass == CLASS_DARK )) 
				|| (CharacterMachine->Equipment[i].Type != -1 && CharacterMachine->Equipment[i].Durability<=0 ))
		    {
                bDexSuccess = false;
			    break;
		    }
        }
#ifdef PBG_ADD_NEWCHAR_MONK
		else if(iBaseClass == CLASS_RAGEFIGHTER)
		{
			if((CharacterMachine->Equipment[i].Type == -1 && ( i!=EQUIPMENT_GLOVES && iBaseClass == CLASS_RAGEFIGHTER)) 
				|| (CharacterMachine->Equipment[i].Type != -1 && CharacterMachine->Equipment[i].Durability<=0 ))
		    {
                bDexSuccess = false;
			    break;
		    }
		}
#endif //PBG_ADD_NEWCHAR_MONK
        else	// 마검이 아닐 경우
        {
			// 장비를 하나라도 착용하고 있지 않거나
			// 장비중 내구도가 0이면 Success는 false
			// memorylock 사용
		    if((CharacterMachine->Equipment[i].Type == -1 ) ||
               (CharacterMachine->Equipment[i].Type != -1 && CharacterMachine->Equipment[i].Durability <= 0 ))
		    {
                bDexSuccess = false;
			    break;
		    }
        }
	}

	// 모든 장비를 착용하고 있고
	if(bDexSuccess)
	{
        //  장비가 같은 종류의 것인지 검사한다.
        int iType;
        if(iBaseClass == CLASS_DARK)	// 마검일 경우
        {
			// memorylock 사용
		    iType = CharacterMachine->Equipment[EQUIPMENT_ARMOR].Type;
            //  마검 갑옷중 아틀란스, 선더호크, 아스릴, 볼케이노, 발리언트 갑옷에만 적용
            if(
				(iType != ITEM_ARMOR+15) 
				&& (iType != ITEM_ARMOR+20) 
				&& (iType != ITEM_ARMOR+23) 
				&& (iType != ITEM_ARMOR+32)
				&& (iType != ITEM_ARMOR+37)
#ifdef KJH_FIX_EQUIPED_FULL_SOCKETITEM_APPLY_TO_STAT
				&& (iType != ITEM_ARMOR+47)				// 팬텀
				&& (iType != ITEM_ARMOR+48)				// 디스트로이
#endif // KJH_FIX_EQUIPED_FULL_SOCKETITEM_APPLY_TO_STAT
				)
            {
				// 다른 갑옷이면 Success는 false
                bDexSuccess = false;
            }

            iType = iType % MAX_ITEM_INDEX;
        }
        else	// 마검이 아닐 경우 타입만 입력해 놓고 Success는 무조건 true
        {
			// memorylock 사용
		    iType = CharacterMachine->Equipment[EQUIPMENT_HELM].Type % MAX_ITEM_INDEX;
        }

		// 마검의 특정 갑옷이거나 다른 클래스이면 다른 장비들을 검사해서 
		// 하나라도 다른 타입이면 풀셋이 아닌걸로 판별
        if(bDexSuccess)
        {
#ifdef _VS2008PORTING
		    for(int i=EQUIPMENT_ARMOR; i<=EQUIPMENT_BOOTS; ++i)
#else // _VS2008PORTING
		    for(i=EQUIPMENT_ARMOR; i<=EQUIPMENT_BOOTS; ++i)
#endif // _VS2008PORTING
		    {
#ifdef PBG_ADD_NEWCHAR_MONK
			// 레이지 파이터는 장갑을 검사하지 않는다.
				if(iBaseClass==CLASS_RAGEFIGHTER && i == EQUIPMENT_GLOVES)
					continue;
#endif //PBG_ADD_NEWCHAR_MONK
				// 헬멧의 타입과 하나라도 다른 타입이면 Success는 false
				// memorylock 사용
			    if(iType != CharacterMachine->Equipment[i].Type % MAX_ITEM_INDEX)
			    {
				    bDexSuccess = false;
				    break;
			    }
		    }
        }
	}

	// 캐릭터 방어력 계산 부분
	// 캐릭터의 방어력
	// memorylock 사용
	int t_adjdef = CharacterAttribute->Defense + Add_Ch_Dfe;

	// 캐릭터의 현재 상태가 방어력 향상이면
	// 캐릭터의 방어력에 (자신의 레벨 / 5 + 50) 만큼을 더해준다.
	if( g_isCharacterBuff((&Hero->Object), eBuff_HelpNpc) )
	{
		// memorylock 사용
		if(CharacterAttribute->Level > 180)
		{
			t_adjdef += 180 / 5 + 50;
		}
		else
		{
			// memorylock 사용
			t_adjdef += (CharacterAttribute->Level / 5 + 50);
		}
	}

#ifdef ASG_ADD_SKILL_BERSERKER
	// 버서커 버프에 걸려 있는가?
	if (g_isCharacterBuff((&Hero->Object), eBuff_Berserker))
	{
		int nTemp = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
		float fTemp = (40 - int(CharacterAttribute->Energy / 60)) / 100.f;
		fTemp = MAX(fTemp, 0.1f);
		t_adjdef -= nTemp / 3 * fTemp;
	}
#endif	// ASG_ADD_SKILL_BERSERKER

	int maxdefense = 0; 

	// 캐릭터가 강화 아이템을 착용하고 있으면
#ifdef _VS2008PORTING
	for(int j=0; j<MAX_EQUIPMENT; ++j)
#else // _VS2008PORTING
	for(j=0; j<MAX_EQUIPMENT; ++j)
#endif // _VS2008PORTING
	{
		// memorylock 사용
		int TempLevel = (CharacterMachine->Equipment[j].Level>>3)&15;
		if( TempLevel >= CharacterMachine->Equipment[j].Jewel_Of_Harmony_OptionLevel )
		{
			StrengthenCapability SC;

			// memorylock 사용
			g_pUIJewelHarmonyinfo->GetStrengthenCapability( &SC, &CharacterMachine->Equipment[j], 2 );

			if( SC.SI_isSD )
			{
				// 강화 아이템 디펜스 저장
				maxdefense += SC.SI_SD.SI_defense;
			}
		}
	}

	// 엘리트 해골전사 변신반지를 착용하고 있다면 추가 방어력 10% 더해준다.
	int iChangeRingAddDefense = 0;
	// memorylock 사용
	pItemRingLeft = &CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];
	pItemRingRight = &CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];
	if(pItemRingLeft->Type == ITEM_HELPER+39 || pItemRingRight->Type == ITEM_HELPER+39)
	{
		// 최종 구해진 방어력의 10% 값을 구해논다.
		iChangeRingAddDefense = (t_adjdef + maxdefense) / 10;
	}

#ifdef PJH_ADD_PANDA_PET
	if(Hero->Helper.Type == MODEL_HELPER+80) //펜더펫
		iChangeRingAddDefense += 50;
#endif //PJH_ADD_PANDA_PET
#ifdef LDK_FIX_CS7_UNICORN_PET_INFO
	if(Hero->Helper.Type == MODEL_HELPER+106) //유니콘
		iChangeRingAddDefense += 50;
#endif //LDK_FIX_CS7_UNICORN_PET_INFO
	unicode::t_char strBlocking[256];

#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	int nAdd_FulBlocking = 0;
	if(g_isCharacterBuff((&Hero->Object), eBuff_Def_up_Ourforces))
	{
#ifdef PBG_MOD_RAGEFIGHTERSOUND
		int _AddStat = (10 + (float)(CharacterAttribute->Energy-80) / 10);
#else //PBG_MOD_RAGEFIGHTERSOUND
		int _AddStat = (10 + (float)(CharacterAttribute->Energy-160) / 50);
#endif //PBG_MOD_RAGEFIGHTERSOUND

		if(_AddStat>100)
			_AddStat = 100;

		_AddStat = CharacterAttribute->SuccessfulBlocking * _AddStat / 100;
		nAdd_FulBlocking += _AddStat;
	}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
	// 장비가 풀셋이면
	if(bDexSuccess)
	{
		// 캐릭터의 방어율이 존재하면
		// memorylock 사용
		if(CharacterAttribute->SuccessfulBlocking > 0)
		{
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
			// 206 "방어력(율): %d (%d +%d)"
			// memorylock 사용
			if(nAdd_FulBlocking)
			{
				unicode::_sprintf(strBlocking, GlobalText[206], t_adjdef + maxdefense + iChangeRingAddDefense,
				CharacterAttribute->SuccessfulBlocking,(CharacterAttribute->SuccessfulBlocking) / 10 + nAdd_FulBlocking);
			}
			else
			{
				unicode::_sprintf(strBlocking, GlobalText[206], t_adjdef + maxdefense + iChangeRingAddDefense,
				CharacterAttribute->SuccessfulBlocking,(CharacterAttribute->SuccessfulBlocking) / 10);
			}
#else //PBG_ADD_NEWCHAR_MONK_SKILL
			// 206 "방어력(율): %d (%d +%d)"
			// memorylock 사용
			unicode::_sprintf(strBlocking, GlobalText[206], 
				t_adjdef + maxdefense + iChangeRingAddDefense,
				CharacterAttribute->SuccessfulBlocking,
#ifdef YDG_FIX_INVALID_SET_DEFENCE_RATE_BONUS
				(CharacterAttribute->SuccessfulBlocking) / 10
#else	// YDG_FIX_INVALID_SET_DEFENCE_RATE_BONUS
				(t_adjdef + iChangeRingAddDefense) / 10
#endif	// YDG_FIX_INVALID_SET_DEFENCE_RATE_BONUS
				);
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
		}
		else // 캐릭터의 방어율이 없으면
		{
			// 207 "방 어 력: %d (+%d)"
          	unicode::_sprintf(strBlocking, GlobalText[207],
				t_adjdef + maxdefense + iChangeRingAddDefense,
				(t_adjdef + iChangeRingAddDefense) / 10
				);
		}
	}
	else	// 장비가 풀셋이 아니라면
	{
		// memorylock 사용
		if(CharacterAttribute->SuccessfulBlocking > 0)
		{
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
			// 208 "방어력(율): %d (%d)"
			if(nAdd_FulBlocking)
			{
				unicode::_sprintf(strBlocking, GlobalText[206],t_adjdef + maxdefense + iChangeRingAddDefense,CharacterAttribute->SuccessfulBlocking, nAdd_FulBlocking);
			}
			else
			{
				unicode::_sprintf(strBlocking, GlobalText[208],t_adjdef + maxdefense + iChangeRingAddDefense,CharacterAttribute->SuccessfulBlocking);
			}
#else //PBG_ADD_NEWCHAR_MONK_SKILL
			// 208 "방어력(율): %d (%d)"
          	unicode::_sprintf(strBlocking, GlobalText[208],
				t_adjdef + maxdefense + iChangeRingAddDefense,
				CharacterAttribute->SuccessfulBlocking
				);
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
		}
		else
		{
			// 209 "방 어 력: %d"
          	unicode::_sprintf(strBlocking, GlobalText[209],
				t_adjdef + maxdefense + iChangeRingAddDefense
				);
		}
	}

	iY = HEIGHT_DEXTERITY+24;
	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(255, 255, 255, 255);

	if(g_isCharacterBuff((&Hero->Object), eBuff_Hellowin3)) // 잭오랜턴의 외침
	{
		g_pRenderText->SetTextColor(255, 0, 240, 255);
	}

#ifdef PSW_SCROLL_ITEM
	if( g_isCharacterBuff((&Hero->Object), eBuff_EliteScroll2) )
	{
		g_pRenderText->SetTextColor(255, 0, 240, 255);
	}
#endif //PSW_SCROLL_ITEM
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	if(g_isCharacterBuff((&Hero->Object), eBuff_Def_up_Ourforces))
	{
		g_pRenderText->SetTextColor(100, 150, 255, 255);
	}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
	g_pRenderText->SetBgColor(0);
	g_pRenderText->RenderText(m_Pos.x+20, m_Pos.y+iY, strBlocking);

	//  공격 속도.
	// memorylock 사용
	WORD wAttackSpeed = CLASS_WIZARD == iBaseClass || CLASS_SUMMONER == iBaseClass
		? CharacterAttribute->MagicSpeed : CharacterAttribute->AttackSpeed;
	
	// 64 "공격속도: %d"
    unicode::_sprintf(strBlocking, GlobalText[64], wAttackSpeed);
	iY += 13;

	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(255, 255, 255, 255);

	if(g_isCharacterBuff((&Hero->Object), eBuff_Hellowin1)) // 잭오랜턴의 축복
	{
		g_pRenderText->SetTextColor(255, 0, 240, 255);
	}

#ifdef PSW_SCROLL_ITEM
	if( g_isCharacterBuff((&Hero->Object), eBuff_EliteScroll1) )
	{
		g_pRenderText->SetTextColor(255, 0, 240, 255);
	}
#endif //PSW_SCROLL_ITEM

#ifdef LDK_ADD_PC4_GUARDIAN
	ITEM *phelper = &CharacterMachine->Equipment[EQUIPMENT_HELPER  ];
	if( phelper->Durability != 0 && 
		(phelper->Type == ITEM_HELPER+64
#ifdef YDG_ADD_SKELETON_PET
		|| phelper->Type == ITEM_HELPER+123		// 스켈레톤 펫
#endif	// YDG_ADD_SKELETON_PET
		) )
	{
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_ATTACKSPEED
		if( IsRequireEquipItem(phelper) )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_ATTACKSPEED
		{
#ifdef LDK_MOD_PC4_GUARDIAN_EXPIREDPERIOD_NOTPRINT_INFO
			//만료된 기간제 아이템이면 출력 안함
			if(false == pItemHelper->bExpiredPeriod)
			{
				g_pRenderText->SetTextColor(255, 0, 240, 255);
			}
#else //LDK_MOD_PC4_GUARDIAN_EXPIREDPERIOD_NOTPRINT_INFO
			g_pRenderText->SetTextColor(255, 0, 240, 255);
#endif //LDK_MOD_PC4_GUARDIAN_EXPIREDPERIOD_NOTPRINT_INFO
		}
	}
#endif //LDK_ADD_PC4_GUARDIAN
	g_pRenderText->SetBgColor(0);
	g_pRenderText->RenderText(m_Pos.x+20, m_Pos.y+iY, strBlocking);

	//	대인전 방성률
	if( itemoption380Defense != 0 || iDefenseRate != 0 )
	{
		// 2110 "대인 방어율: %d (+%d)"
		// memorylock 사용
		unicode::_sprintf(strBlocking, GlobalText[2110], CharacterAttribute->SuccessfulBlockingPK + Add_Dfe, itemoption380Defense + iDefenseRate);
	}
	else
	{
		// 2045 "대인 방어율: %d"
		// memorylock 사용
		unicode::_sprintf(strBlocking, GlobalText[2045], CharacterAttribute->SuccessfulBlockingPK + Add_Dfe);
	}

	iY += 13;
	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->SetBgColor(0);
	g_pRenderText->RenderText(m_Pos.x+20, m_Pos.y+iY, strBlocking);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	//							체력										//
	//////////////////////////////////////////////////////////////////////////

	g_pRenderText->SetFont(g_hFontBold);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

	WORD wVitality;
	// memorylock 사용
	wVitality = CharacterAttribute->Vitality+ CharacterAttribute->AddVitality;
	// memorylock 사용
#ifdef PSW_SECRET_ITEM
	if( g_isCharacterBuff((&Hero->Object), eBuff_SecretPotion3) )
	{
		g_pRenderText->SetTextColor(255, 120, 0, 255);
	}
	else
#endif //PSW_SECRET_ITEM
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	if(g_isCharacterBuff((&Hero->Object), eBuff_Hp_up_Ourforces))
	{
#ifdef PBG_MOD_RAGEFIGHTERSOUND
		CharacterMachine->CalculateAll();
		wVitality = CharacterAttribute->Vitality+ CharacterAttribute->AddVitality;
#else //PBG_MOD_RAGEFIGHTERSOUND
		WORD _AddStat = (WORD)(30+(WORD)((CharacterAttribute->Energy-380)/10));
		if(_AddStat > 200)
		{
			_AddStat = 200;
		}
		wVitality = CharacterAttribute->Vitality + _AddStat;
#endif //PBG_MOD_RAGEFIGHTERSOUND
		g_pRenderText->SetTextColor(100, 150, 255, 255);
	}
	else
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
    if( CharacterAttribute->AddVitality )
    {
		g_pRenderText->SetTextColor(100, 150, 255, 255);
    }
    else
    {
		g_pRenderText->SetTextColor(230, 230, 0, 255);
    }

	unicode::t_char strVitality[256];
	unicode::_sprintf(strVitality, "%d", wVitality);
	g_pRenderText->SetBgColor(0);
	// 169 "체력"
	g_pRenderText->RenderText(m_Pos.x+12, m_Pos.y+HEIGHT_VITALITY+6, GlobalText[169], 74, 0, RT3_SORT_CENTER);
	g_pRenderText->RenderText(m_Pos.x+86, m_Pos.y+HEIGHT_VITALITY+6, strVitality, 86, 0, RT3_SORT_CENTER);
	
	g_pRenderText->SetFont(g_hFont);
	// 211 " 생   명 : %d / %d"
	// memorylock 사용
	if(IsMasterLevel( Hero->Class ) == true )
	{
		unicode::_sprintf(strVitality, GlobalText[211],CharacterAttribute->Life,Master_Level_Data.wMaxLife);
	}
	else
	{
		unicode::_sprintf(strVitality, GlobalText[211],CharacterAttribute->Life,CharacterAttribute->LifeMax);
	}
	g_pRenderText->SetTextColor(255, 255, 255, 255);

	if(g_isCharacterBuff((&Hero->Object), eBuff_Hellowin4)) // 잭오랜턴의 음식
	{
		g_pRenderText->SetTextColor(255, 0, 240, 255);
	}

#ifdef PSW_SCROLL_ITEM
	if( g_isCharacterBuff((&Hero->Object), eBuff_EliteScroll5) )
	{
		g_pRenderText->SetTextColor(255, 0, 240, 255);
	}
#endif //PSW_SCROLL_ITEM
#ifdef CSK_EVENT_CHERRYBLOSSOM
	if( g_isCharacterBuff((&Hero->Object), eBuff_CherryBlossom_RiceCake) )
	{
		g_pRenderText->SetTextColor(255, 0, 240, 255);
	}
#endif //CSK_EVENT_CHERRYBLOSSOM
#ifdef LDK_ADD_PC4_GUARDIAN
	if( phelper->Durability != 0 && phelper->Type == ITEM_HELPER+65 )
	{
#ifdef PSW_BUGFIX_REQUIREEQUIPITEM_ATTACKSPEED
		if( IsRequireEquipItem(phelper) )
#endif //PSW_BUGFIX_REQUIREEQUIPITEM_ATTACKSPEED
		{
#ifdef LDK_MOD_PC4_GUARDIAN_EXPIREDPERIOD_NOTPRINT_INFO
			//만료된 기간제 아이템이면 출력 안함
			if(false == pItemHelper->bExpiredPeriod)
			{
				g_pRenderText->SetTextColor(255, 0, 240, 255);
			}
#else //LDK_MOD_PC4_GUARDIAN_EXPIREDPERIOD_NOTPRINT_INFO
			g_pRenderText->SetTextColor(255, 0, 240, 255);
#endif //LDK_MOD_PC4_GUARDIAN_EXPIREDPERIOD_NOTPRINT_INFO
		}
	}
#endif //LDK_ADD_PC4_GUARDIAN
	g_pRenderText->SetBgColor(0);
	iY = HEIGHT_VITALITY+24;
	g_pRenderText->RenderText(m_Pos.x+20, m_Pos.y+iY, strVitality);

#ifdef PBG_ADD_NEWCHAR_MONK
	if(iBaseClass==CLASS_RAGEFIGHTER)
	{
		iY += 13;
		//물리공격력
		unicode::_sprintf(strVitality, GlobalText[3155], 50+(wVitality/10));
		g_pRenderText->RenderText(m_Pos.x+20, m_Pos.y+iY, strVitality);
	}
#endif //PBG_ADD_NEWCHAR_MONK
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	//							에너지										//
	//////////////////////////////////////////////////////////////////////////

	g_pRenderText->SetFont(g_hFontBold);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
	
    WORD wEnergy;
	// memorylock 사용
	wEnergy = CharacterAttribute->Energy+ CharacterAttribute->AddEnergy;
	// memorylock 사용
#ifdef PSW_SECRET_ITEM
	if( g_isCharacterBuff((&Hero->Object), eBuff_SecretPotion4) )
	{
		g_pRenderText->SetTextColor(255, 120, 0, 255);
	}
	else
#endif //PSW_SECRET_ITEM
    if(CharacterAttribute->AddEnergy)
    {
		g_pRenderText->SetTextColor(100, 150, 255, 255);
    }
    else
    {
		g_pRenderText->SetTextColor(230, 230, 0, 255);
    }

	unicode::t_char strEnergy[256];
	unicode::_sprintf(strEnergy, "%d", wEnergy);

	g_pRenderText->SetBgColor(0);
	// 168 "에너지"
	g_pRenderText->RenderText(m_Pos.x+12, m_Pos.y+HEIGHT_ENERGY+6, GlobalText[168], 74, 0, RT3_SORT_CENTER);
	g_pRenderText->RenderText(m_Pos.x+86, m_Pos.y+HEIGHT_ENERGY+6, strEnergy, 86, 0, RT3_SORT_CENTER);
	
	g_pRenderText->SetFont(g_hFont);
	
	// 213 " 마   나 : %d / %d"
	if(IsMasterLevel( Hero->Class ) == true )
	{
		unicode::_sprintf(strEnergy, GlobalText[213], CharacterAttribute->Mana,Master_Level_Data.wMaxMana);
	}
	else
		unicode::_sprintf(strEnergy, GlobalText[213], CharacterAttribute->Mana,CharacterAttribute->ManaMax);

	g_pRenderText->SetBgColor(0);
	g_pRenderText->SetTextColor(255, 255, 255, 255);

	if(g_isCharacterBuff((&Hero->Object), eBuff_Hellowin5)) // 잭오랜턴의 음료
	{
		g_pRenderText->SetTextColor(255, 0, 240, 255);
	}

#ifdef PSW_SCROLL_ITEM
	if( g_isCharacterBuff((&Hero->Object), eBuff_EliteScroll6) )
	{
		g_pRenderText->SetTextColor(255, 0, 240, 255);
	}
#endif //PSW_SCROLL_ITEM
#ifdef CSK_EVENT_CHERRYBLOSSOM
	if( g_isCharacterBuff((&Hero->Object), eBuff_CherryBlossom_Liguor) )
	{
		g_pRenderText->SetTextColor(255, 0, 240, 255);
	}
#endif //CSK_EVENT_CHERRYBLOSSOM
	iY = HEIGHT_ENERGY+24;
	g_pRenderText->RenderText(m_Pos.x+18, m_Pos.y+iY, strEnergy);

	if (iBaseClass == CLASS_WIZARD || iBaseClass == CLASS_DARK || iBaseClass == CLASS_SUMMONER)
	{
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		int Level = (pWeaponRight->Level>>3)&15;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		int iMagicDamageMin;
		int iMagicDamageMax;
		// memorylock 사용
		CharacterMachine->GetMagicSkillDamage( CharacterAttribute->Skill[Hero->CurrentSkill], &iMagicDamageMin, &iMagicDamageMax);
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		int iMagicDamageMinInitial = iMagicDamageMin;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		int iMagicDamageMaxInitial = iMagicDamageMax;

#ifdef PJH_SEASON4_MASTER_RANK4
	iMagicDamageMin += Add_Mana_Min;
	iMagicDamageMax += Add_Mana_Max;
#endif //PJH_SEASON4_MASTER_RANK4
		// memorylock 사용
		if( CharacterAttribute->Ability & ABILITY_PLUS_DAMAGE)
		{
			iMagicDamageMin += 10;
			iMagicDamageMax += 10;
		}

		int maxMg = 0; 

#ifdef _VS2008PORTING
		for(int j=0; j<MAX_EQUIPMENT; ++j)
#else // _VS2008PORTING
		for(j=0; j<MAX_EQUIPMENT; ++j)
#endif // _VS2008PORTING
		{
			// memorylock 사용
			int TempLevel = (CharacterMachine->Equipment[j].Level>>3)&15;

			// memorylock 사용
			if(  TempLevel >= CharacterMachine->Equipment[j].Jewel_Of_Harmony_OptionLevel )
			{
				StrengthenCapability SC;
				// memorylock 사용
				g_pUIJewelHarmonyinfo->GetStrengthenCapability( &SC, &CharacterMachine->Equipment[j], 1 );

				if( SC.SI_isSP )
				{
					maxMg     += SC.SI_SP.SI_magicalpower;
				}
			}
		}

		// 크리스마스 변신 반지를 끼고 있으면
		// memorylock 사용
		pItemRingLeft = &CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];
		pItemRingRight = &CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];

#ifdef YDG_FIX_CHANGERING_STATUS_EXPIRE_CHECK
		// 만기 체크
		int iNonExpiredLRingType = -1;
		int iNonExpiredRRingType = -1;
#ifdef LJH_FIX_IGNORING_EXPIRATION_PERIOD
		iNonExpiredLRingType = pItemRingLeft->Type;
		iNonExpiredRRingType = pItemRingRight->Type;
#else  //LJH_FIX_IGNORING_EXPIRATION_PERIOD
		if(!pItemRingLeft->bPeriodItem || !pItemRingLeft->bExpiredPeriod)	// 왼손
		{
			iNonExpiredLRingType = pItemRingLeft->Type;
		}
		if(!pItemRingRight->bPeriodItem || !pItemRingRight->bExpiredPeriod)	// 오른손
		{
			iNonExpiredRRingType = pItemRingRight->Type;
		}
#endif //LJH_FIX_IGNORING_EXPIRATION_PERIOD

		int maxIMagicDamageMin = 0;
		int maxIMagicDamageMax = 0;

		if(iNonExpiredLRingType == ITEM_HELPER+41 || iNonExpiredRRingType == ITEM_HELPER+41)
		{
			//  착용중인변신 반지들 중 크리스마스 변신 반지에 의해 가장 높은 마력 데미지 추가.
			maxIMagicDamageMin = max(maxIMagicDamageMin, 20);		// 최소 마력 데미지
			maxIMagicDamageMax = max(maxIMagicDamageMax, 20);		// 최소 마력 데미지
		}
#ifdef PJH_ADD_PANDA_CHANGERING
		if(iNonExpiredLRingType == ITEM_HELPER+76 || iNonExpiredRRingType == ITEM_HELPER+76)
		{
			// 착용중인 변신 반지들 중 팬더 변신 반지에 의해 가장 높은 마력 데미지 추가.
			maxIMagicDamageMin = max(maxIMagicDamageMin, 30);		// 최소 마력 데미지
			maxIMagicDamageMax = max(maxIMagicDamageMax, 30);		// 최소 마력 데미지
		}
#endif	// PJH_ADD_PANDA_CHANGERING
#ifdef YDG_ADD_SKELETON_CHANGE_RING
		if(iNonExpiredLRingType == ITEM_HELPER+122 || iNonExpiredRRingType == ITEM_HELPER+122)	// 스켈레톤 변신반지
		{
			// 착용중인 변신 반지들 중 스켈레톤 변신 반지에 의해 가장 높은 마력 데미지 추가 
			maxIMagicDamageMin = max(maxIMagicDamageMin, 40);		// 최소 마력 데미지
			maxIMagicDamageMax = max(maxIMagicDamageMax, 40);		// 최소 마력 데미지
		}
#endif	// YDG_ADD_SKELETON_CHANGE_RING

		// 최종 구해진 마법공격력에 착용중인 변신 반지들의 데미지 증가 중에 가장 높은 값을 더해준다.
		iMagicDamageMin += maxIMagicDamageMin;	// 최소 마력 데미지
		iMagicDamageMax += maxIMagicDamageMax;	// 최대 마력 데미지


#else	// YDG_FIX_CHANGERING_STATUS_EXPIRE_CHECK	// 소스정리시 삭제할 부분!!!
#ifdef LJH_FIX_CHANGE_RING_DAMAGE_BUG
		int maxIMagicDamageMin = 0;
		int maxIMagicDamageMax = 0;

		if(pItemRingLeft->Type == ITEM_HELPER+41 || pItemRingRight->Type == ITEM_HELPER+41)
		{
			//  착용중인변신 반지들 중 크리스마스 변신 반지에 의해 가장 높은 공격데미지 추가.
			maxIMagicDamageMin = max(maxIMagicDamageMin, 20);		// 최소 마력 데미지
			maxIMagicDamageMax = max(maxIMagicDamageMax, 20);		// 최소 마력 데미지
		}
#ifdef PJH_ADD_PANDA_CHANGERING
		if(pItemRingLeft->Type == ITEM_HELPER+76 || pItemRingRight->Type == ITEM_HELPER+76)
		{
			// 착용중인 변신 반지들 중 팬더 변신 반지에 의해 가장 높은 공격데미지 추가.
			maxIMagicDamageMin = max(maxIMagicDamageMin, 30);		// 최소 마력 데미지
			maxIMagicDamageMax = max(maxIMagicDamageMax, 30);		// 최소 마력 데미지
		}
#endif	// PJH_ADD_PANDA_CHANGERING

		// 최종 구해진 마법공격력에 착용중인 변신 반지들의 데미지 증가 중에 가장 높은 값을 더해준다.
		iMagicDamageMin += maxIMagicDamageMin;	// 최소 공격 데미지
		iMagicDamageMax += maxIMagicDamageMax;	// 최대 공격 데미지

#else  //LJH_FIX_CHANGE_RING_DAMAGE_BUG
		if(pItemRingLeft->Type == ITEM_HELPER+41 || pItemRingRight->Type == ITEM_HELPER+41)
		{
			// 최종 구해진 마법공격력에다가 20를 더해준다.
			iMagicDamageMin += 20;	// 최소 마력 데미지
			iMagicDamageMax += 20;	// 최대 마력 데미지
		}
#endif //LJH_FIX_CHANGE_RING_DAMAGE_BUG
#endif	// YDG_FIX_CHANGERING_STATUS_EXPIRE_CHECK	// 소스정리시 삭제할 부분!!!
		
		// memorylock 사용
		pItemHelper = &CharacterMachine->Equipment[EQUIPMENT_HELPER];
		if(pItemHelper)
		{
			if(pItemHelper->Type == ITEM_HELPER+37 && pItemHelper->Option1 == 0x04)
			{
				// 환영 펜릴은 마법공격력은 
				WORD wLevel = CharacterAttribute->Level;
				// 최종 구해진 마법공격력에다가 (Level/25)를 더해준다.
				iMagicDamageMin += (wLevel / 25);	// 최소 마력 데미지
				iMagicDamageMax += (wLevel / 25);	// 최대 마력 데미지
			}
#ifdef LDK_FIX_PC4_GUARDIAN_DEMON_INFO
			if(pItemHelper->Type == ITEM_HELPER+64)
			{
#ifdef LDK_MOD_PC4_GUARDIAN_EXPIREDPERIOD_NOTPRINT_INFO
				//만료된 기간제 아이템이면 출력 안함
				if(false == pItemHelper->bExpiredPeriod)
				{
					// memorylock 사용
					iMagicDamageMin += int(float(iMagicDamageMin) * 0.4f);	// 최소 마력 데미지
					iMagicDamageMax += int(float(iMagicDamageMax) * 0.4f);	// 최대 마력 데미지
				}
#else //LDK_MOD_PC4_GUARDIAN_EXPIREDPERIOD_NOTPRINT_INFO
				// memorylock 사용
				iMagicDamageMin += int(float(iMagicDamageMin) * 0.4f);	// 최소 마력 데미지
				iMagicDamageMax += int(float(iMagicDamageMax) * 0.4f);	// 최대 마력 데미지
#endif //LDK_MOD_PC4_GUARDIAN_EXPIREDPERIOD_NOTPRINT_INFO
			}
#endif //LDK_FIX_PC4_GUARDIAN_DEMON_INFO
#ifdef YDG_ADD_SKELETON_PET
			if(pItemHelper->Type == ITEM_HELPER+123)	// 스켈레톤 펫
			{
#ifdef LJH_FIX_IGNORING_EXPIRATION_PERIOD
				// memorylock 사용
				iMagicDamageMin += int(float(iMagicDamageMin) * 0.2f);	// 최소 마력 데미지
				iMagicDamageMax += int(float(iMagicDamageMax) * 0.2f);	// 최대 마력 데미지
#else  //LJH_FIX_IGNORING_EXPIRATION_PERIOD
				//만료된 기간제 아이템이면 출력 안함
				if(false == pItemHelper->bExpiredPeriod)
				{
					// memorylock 사용
					iMagicDamageMin += int(float(iMagicDamageMin) * 0.2f);	// 최소 마력 데미지
					iMagicDamageMax += int(float(iMagicDamageMax) * 0.2f);	// 최대 마력 데미지
				}
#endif //LJH_FIX_IGNORING_EXPIRATION_PERIOD
			}
#endif	// YDG_ADD_SKELETON_PET
#ifdef PBG_FIX_SATAN_VALUEINCREASE
			if(pItemHelper->Type == ITEM_HELPER+1 
#if SELECTED_LANGUAGE == LANGUAGE_KOREAN		//해외에서 기간제 사용시 디파인제거 할것
				&& !(pItemHelper->bExpiredPeriod)
#endif //SELECTED_LANGUAGE == LANGUAGE_KOREAN
				)
			{
				iMagicDamageMin += int(float(iMagicDamageMin) * 0.3f);
				iMagicDamageMax += int(float(iMagicDamageMax) * 0.3f);
			}
#endif //PBG_FIX_SATAN_VALUEINCREASE
		}

#ifdef ASG_ADD_SKILL_BERSERKER
		// 버서커 버프에 걸려 있는가?
		if (g_isCharacterBuff((&Hero->Object), eBuff_Berserker))
		{
			int nTemp = CharacterAttribute->Energy + CharacterAttribute->AddEnergy;
			float fTemp = int(CharacterAttribute->Energy / 30) / 100.f;
			iMagicDamageMin += nTemp / 9 * fTemp;
			iMagicDamageMax += nTemp / 4 * fTemp;
		}
#endif	// ASG_ADD_SKILL_BERSERKER

		// 마검사 검
		if( ( pWeaponRight->Type >= MODEL_STAFF-MODEL_ITEM 
				&& pWeaponRight->Type < (MODEL_STAFF+MAX_ITEM_INDEX-MODEL_ITEM) ) 
            || pWeaponRight->Type == (MODEL_SWORD+31-MODEL_ITEM)
			|| pWeaponRight->Type == (MODEL_SWORD+23-MODEL_ITEM)
			|| pWeaponRight->Type == (MODEL_SWORD+25-MODEL_ITEM)
            || pWeaponRight->Type == (MODEL_SWORD+21-MODEL_ITEM) 
#ifdef KJH_FIX_RUNE_BASTARD_SWORD_TOOLTIP
			|| pWeaponRight->Type == (MODEL_SWORD+28-MODEL_ITEM) 
#endif // KJH_FIX_RUNE_BASTARD_SWORD_TOOLTIP
           )
        {
            float magicPercent = (float)(pWeaponRight->MagicPower)/100;

            //  내구력에 따른 마법 공격력 감소.
            ITEM_ATTRIBUTE *p = &ItemAttribute[pWeaponRight->Type];
            float   percent = CalcDurabilityPercent(pWeaponRight->Durability, p->MagicDur, pWeaponRight->Level, pWeaponRight->Option1, pWeaponRight->ExtOption);
            
            magicPercent = magicPercent - magicPercent*percent;
			// 215 " 마   력 : %d~%d (+%d)"
			unicode::_sprintf(strEnergy, GlobalText[215],
				iMagicDamageMin + maxMg,
				iMagicDamageMax + maxMg,
				(int)((iMagicDamageMaxInitial+maxMg)*magicPercent));
        }
		else
		{
			// 216 " 마   력 : %d~%d"
			unicode::_sprintf(strEnergy, GlobalText[216],
				iMagicDamageMin + maxMg,
				iMagicDamageMax + maxMg);
		}

		iY += 13;
		g_pRenderText->SetBgColor(0);
		g_pRenderText->SetTextColor(255, 255, 255, 255);

		if(g_isCharacterBuff((&Hero->Object), eBuff_Hellowin2)) // 잭오랜턴의 분노
		{
			g_pRenderText->SetTextColor(255, 0, 240, 255);
		}

#ifdef PSW_SCROLL_ITEM
		if( g_isCharacterBuff((&Hero->Object), eBuff_EliteScroll4) )
		{
			g_pRenderText->SetTextColor(255, 0, 240, 255);
		}
#endif //PSW_SCROLL_ITEM

#ifdef CSK_EVENT_CHERRYBLOSSOM
		if( g_isCharacterBuff((&Hero->Object), eBuff_CherryBlossom_Petal) )
		{
			g_pRenderText->SetTextColor(255, 0, 240, 255);
		}
#endif //CSK_EVENT_CHERRYBLOSSOM		
		g_pRenderText->RenderText(m_Pos.x + 18, m_Pos.y + iY, strEnergy);
	}

	// 저주력 계산.
	if (iBaseClass == CLASS_SUMMONER)
	{
		int iCurseDamageMin = 0;
		int iCurseDamageMax = 0;
		// memorylock 사용
		CharacterMachine->GetCurseSkillDamage(CharacterAttribute->Skill[Hero->CurrentSkill], &iCurseDamageMin, &iCurseDamageMax);

#ifdef ASG_ADD_SKILL_BERSERKER
		// 버서커 버프에 걸려 있는가?
		if (g_isCharacterBuff((&Hero->Object), eBuff_Berserker))
		{
			int nTemp = CharacterAttribute->Energy + CharacterAttribute->AddEnergy;
			float fTemp = int(CharacterAttribute->Energy / 30) / 100.f;
			iCurseDamageMin += nTemp / 9 * fTemp;
			iCurseDamageMax += nTemp / 4 * fTemp;
		}
#endif	// ASG_ADD_SKILL_BERSERKER

#ifdef YDG_FIX_CHANGERING_STATUS_EXPIRE_CHECK
		// 만기 체크
		int iNonExpiredLRingType = -1;
		int iNonExpiredRRingType = -1;
#ifdef LJH_FIX_IGNORING_EXPIRATION_PERIOD
		iNonExpiredLRingType = pItemRingLeft->Type;
		iNonExpiredRRingType = pItemRingRight->Type;
#else  //LJH_FIX_IGNORING_EXPIRATION_PERIOD
		if(!pItemRingLeft->bPeriodItem || !pItemRingLeft->bExpiredPeriod)	// 왼손
		{
			iNonExpiredLRingType = pItemRingLeft->Type;
		}
		if(!pItemRingRight->bPeriodItem || !pItemRingRight->bExpiredPeriod)	// 오른손
		{
			iNonExpiredRRingType = pItemRingRight->Type;
		}
#endif //LJH_FIX_IGNORING_EXPIRATION_PERIOD
		
		int maxICurseDamageMin = 0;
		int maxICurseDamageMax = 0;

#ifdef PJH_ADD_PANDA_CHANGERING
		// 팬더 변신반지를 착용 (저주력 +30)
		if(iNonExpiredLRingType == ITEM_HELPER+76 || iNonExpiredRRingType == ITEM_HELPER+76)
		{
			// 착용중인 변신 반지들 중 팬더 변신 반지에 의해 가장 높은 저주력 데미지 추가.
			maxICurseDamageMin = max(maxICurseDamageMin, 30);		// 최소 저주력 데미지
			maxICurseDamageMax = max(maxICurseDamageMax, 30);		// 최소 저주력 데미지
		}
#endif	// PJH_ADD_PANDA_CHANGERING
#ifdef YDG_ADD_SKELETON_CHANGE_RING
		if(iNonExpiredLRingType == ITEM_HELPER+122 || iNonExpiredRRingType == ITEM_HELPER+122)	// 스켈레톤 변신반지
		{
			// 착용중인 변신 반지들 중 스켈레톤 변신 반지에 의해 가장 높은 저주력 데미지 추가 
			maxICurseDamageMin = max(maxICurseDamageMin, 40);		// 최소 저주력 데미지
			maxICurseDamageMax = max(maxICurseDamageMax, 40);		// 최소 저주력 데미지
		}
#endif	// YDG_ADD_SKELETON_CHANGE_RING
		// 최종 구해진 마법공격력에 착용중인 변신 반지들의 데미지 증가 중에 가장 높은 값을 더해준다.
		iCurseDamageMin += maxICurseDamageMin;	// 최소 저주력 데미지
		iCurseDamageMax += maxICurseDamageMax;	// 최대 저주력 데미지


#else	// YDG_FIX_CHANGERING_STATUS_EXPIRE_CHECK	// 소스정리시 삭제할 부분!!!
#ifdef LJH_FIX_CHANGE_RING_DAMAGE_BUG
		// 반지에 대한 정보를 얻음
		pItemRingLeft = &CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];
		pItemRingRight = &CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];

		int maxICurseDamageMin = 0;
		int maxICurseDamageMax = 0;

#ifdef PJH_ADD_PANDA_CHANGERING
		// 팬더 변신반지를 착용 (저주력 +30)
		if(pItemRingLeft->Type == ITEM_HELPER+76 || pItemRingRight->Type == ITEM_HELPER+76)
		{
			// 착용중인 변신 반지들 중 팬더 변신 반지에 의해 가장 높은 공격데미지 추가.
			maxICurseDamageMin = max(maxICurseDamageMin, 30);		// 최소 마력 데미지
			maxICurseDamageMax = max(maxICurseDamageMax, 30);		// 최소 마력 데미지
		}
#endif	// PJH_ADD_PANDA_CHANGERING
		
		// 최종 구해진 마법공격력에 착용중인 변신 반지들의 데미지 증가 중에 가장 높은 값을 더해준다.
		iCurseDamageMin += maxICurseDamageMin;	// 최소 공격 데미지
		iCurseDamageMax += maxICurseDamageMax;	// 최대 공격 데미지

#endif //LJH_FIX_CHANGE_RING_DAMAGE_BUG
#endif	// YDG_FIX_CHANGERING_STATUS_EXPIRE_CHECK	// 소스정리시 삭제할 부분!!!

#ifdef LDK_FIX_PC4_GUARDIAN_DEMON_INFO
		// memorylock 사용
		pItemHelper = &CharacterMachine->Equipment[EQUIPMENT_HELPER];
		if(pItemHelper)
		{
			if(pItemHelper->Type == ITEM_HELPER+64)
			{
#ifdef LDK_MOD_PC4_GUARDIAN_EXPIREDPERIOD_NOTPRINT_INFO
				//만료된 기간제 아이템이면 출력 안함
				if(false == pItemHelper->bExpiredPeriod)
				{
					// memorylock 사용
					iCurseDamageMin += int(float(iCurseDamageMin) * 0.4f);	// 최소 공격 데미지
					iCurseDamageMax += int(float(iCurseDamageMax) * 0.4f);	// 최대 공격 데미지
				}
#else //LDK_MOD_PC4_GUARDIAN_EXPIREDPERIOD_NOTPRINT_INFO
				// memorylock 사용
				iCurseDamageMin += int(float(iCurseDamageMin) * 0.4f);	// 최소 공격 데미지
				iCurseDamageMax += int(float(iCurseDamageMax) * 0.4f);	// 최대 공격 데미지
#endif //LDK_MOD_PC4_GUARDIAN_EXPIREDPERIOD_NOTPRINT_INFO
			}
#ifdef YDG_ADD_SKELETON_PET
			if(pItemHelper->Type == ITEM_HELPER+123)	// 스켈레톤 펫
			{
#ifdef LJH_FIX_IGNORING_EXPIRATION_PERIOD
				// memorylock 사용
				iCurseDamageMin += int(float(iCurseDamageMin) * 0.2f);	// 최소 공격 데미지
				iCurseDamageMax += int(float(iCurseDamageMax) * 0.2f);	// 최대 공격 데미지
#else  //LJH_FIX_IGNORING_EXPIRATION_PERIOD
				//만료된 기간제 아이템이면 출력 안함
				if(false == pItemHelper->bExpiredPeriod)
				{
					// memorylock 사용
					iCurseDamageMin += int(float(iCurseDamageMin) * 0.2f);	// 최소 공격 데미지
					iCurseDamageMax += int(float(iCurseDamageMax) * 0.2f);	// 최대 공격 데미지
				}
#endif //LJH_FIX_IGNORING_EXPIRATION_PERIOD
			}
#endif	// YDG_ADD_SKELETON_PET
		}
#endif //LDK_FIX_PC4_GUARDIAN_DEMON_INFO

		// 소환술사용 왼손 무기인 책인가?
		if (ITEM_STAFF+21 <= pWeaponLeft->Type && pWeaponLeft->Type <= ITEM_STAFF+29)
		{
			float fCursePercent = (float)(pWeaponLeft->MagicPower)/100;

			//  내구력에 따른 저주력 감소.
			ITEM_ATTRIBUTE *p = &ItemAttribute[pWeaponLeft->Type];
			float fPercent = ::CalcDurabilityPercent(pWeaponLeft->Durability,
				p->MagicDur, pWeaponLeft->Level, pWeaponLeft->Option1,
				pWeaponLeft->ExtOption);
        
			fCursePercent -= fCursePercent*fPercent;
			// 1693 " 저   주   력 : %d ~ %d(+%d)"
			unicode::_sprintf(strEnergy, GlobalText[1693],
				iCurseDamageMin, iCurseDamageMax,
				(int)((iCurseDamageMax)*fCursePercent));
		}
		else
		{
			// 1694 " 저   주   력 : %d ~ %d"
			unicode::_sprintf(strEnergy, GlobalText[1694],
				iCurseDamageMin, iCurseDamageMax);
		}

		iY += 13;
		g_pRenderText->RenderText(m_Pos.x + 18, m_Pos.y + iY, strEnergy);
	}

	iY += 13;
	if( iBaseClass==CLASS_KNIGHT )
	{
		// 582 "스킬 공격력: %d%%"
		unicode::_sprintf(strEnergy, GlobalText[582], 200+(wEnergy/10));
		g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + iY, strEnergy);
	}
	if ( iBaseClass==CLASS_DARK )
	{
		// 582 "스킬 공격력: %d%%"
		unicode::_sprintf(strEnergy,GlobalText[582], 200);
		g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + iY, strEnergy);
	}
	if ( iBaseClass==CLASS_DARK_LORD )
	{
		unicode::_sprintf( strEnergy, GlobalText[582], 200+(wEnergy/20) );
		g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + iY, strEnergy);
	}
#ifdef PBG_ADD_NEWCHAR_MONK
	if(iBaseClass==CLASS_RAGEFIGHTER)
	{
		//마법공격력
		unicode::_sprintf(strEnergy, GlobalText[3156], 50+(wEnergy/10));
		g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + iY, strEnergy);
		iY += 13;
		//범위공격력
		unicode::_sprintf(strEnergy, GlobalText[3157], 100+(wDexterity/8+wEnergy/10));
		g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + iY, strEnergy);
	}
#endif //PBG_ADD_NEWCHAR_MONK

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	//							통솔										//
	//////////////////////////////////////////////////////////////////////////

	if(iBaseClass == CLASS_DARK_LORD)
	{
		g_pRenderText->SetFont(g_hFontBold);
		g_pRenderText->SetBgColor(0);
		
        WORD wCharisma;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

		// memorylock 사용
        wCharisma= CharacterAttribute->Charisma+ CharacterAttribute->AddCharisma;
		// memorylock 사용
#ifdef PSW_SECRET_ITEM
		if( g_isCharacterBuff((&Hero->Object), eBuff_SecretPotion5) )
		{
			g_pRenderText->SetTextColor(255, 120, 0, 255);
		}
		else
#endif //PSW_SECRET_ITEM
        if(CharacterAttribute->AddCharisma )
        {
			g_pRenderText->SetTextColor(100, 150, 255, 255);
        }
        else
        {
			g_pRenderText->SetTextColor(230, 230, 0, 255);
        }
		
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

		unicode::t_char strCharisma[256];
        unicode::_sprintf( strCharisma, "%d", wCharisma );
		// 1900 "통솔"
		g_pRenderText->RenderText(m_Pos.x+12, m_Pos.y+HEIGHT_CHARISMA+6, GlobalText[1900], 74, 0, RT3_SORT_CENTER);
		g_pRenderText->RenderText(m_Pos.x+86, m_Pos.y+HEIGHT_CHARISMA+6, strCharisma, 86, 0, RT3_SORT_CENTER);
	}
}

void SEASON3B::CNewUICharacterInfoWindow::RenderButtons()
{
	int iBaseClass = GetBaseClass(Hero->Class);
	int iCount = 0;
	if(iBaseClass == CLASS_DARK_LORD)
	{
		iCount = 5;
	}
	else
	{
		iCount = 4;
	}

	if(CharacterAttribute->LevelUpPoint > 0)
	{
		for(int i=0; i<iCount; ++i)
		{
			m_BtnStat[i].Render();
		}
	}

	m_BtnExit.Render();
	m_BtnQuest.Render();
	m_BtnPet.Render();
	m_BtnMasterLevel.Render();
}

float SEASON3B::CNewUICharacterInfoWindow::GetLayerDepth()
{ 
	return 5.1f; 
}

void SEASON3B::CNewUICharacterInfoWindow::LoadImages()
{
	LoadBitmap("Interface\\newui_msgbox_back.jpg", IMAGE_CHAINFO_BACK, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back04.tga", IMAGE_CHAINFO_TOP, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-L.tga", IMAGE_CHAINFO_LEFT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-R.tga", IMAGE_CHAINFO_RIGHT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back03.tga", IMAGE_CHAINFO_BOTTOM, GL_LINEAR);

	LoadBitmap("Interface\\newui_item_table01(L).tga", IMAGE_CHAINFO_TABLE_TOP_LEFT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_table01(R).tga", IMAGE_CHAINFO_TABLE_TOP_RIGHT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_table02(L).tga", IMAGE_CHAINFO_TABLE_BOTTOM_LEFT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_table02(R).tga", IMAGE_CHAINFO_TABLE_BOTTOM_RIGHT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_table03(Up).tga", IMAGE_CHAINFO_TABLE_TOP_PIXEL, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_table03(Dw).tga", IMAGE_CHAINFO_TABLE_BOTTOM_PIXEL, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_table03(L).tga", IMAGE_CHAINFO_TABLE_LEFT_PIXEL, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_table03(R).tga", IMAGE_CHAINFO_TABLE_RIGHT_PIXEL, GL_LINEAR);

	LoadBitmap("Interface\\newui_exit_00.tga", IMAGE_CHAINFO_BTN_EXIT, GL_LINEAR);

	LoadBitmap("Interface\\newui_cha_textbox02.tga", IMAGE_CHAINFO_TEXTBOX, GL_LINEAR);
	LoadBitmap("Interface\\newui_chainfo_btn_level.tga", IMAGE_CHAINFO_BTN_STAT, GL_LINEAR);
	LoadBitmap("Interface\\newui_chainfo_btn_quest.tga", IMAGE_CHAINFO_BTN_QUEST, GL_LINEAR);
	LoadBitmap("Interface\\newui_chainfo_btn_pet.tga", IMAGE_CHAINFO_BTN_PET, GL_LINEAR);
	LoadBitmap("Interface\\newui_chainfo_btn_master.tga", IMAGE_CHAINFO_BTN_MASTERLEVEL, GL_LINEAR);
}

void SEASON3B::CNewUICharacterInfoWindow::UnloadImages()
{
	DeleteBitmap(IMAGE_CHAINFO_BTN_MASTERLEVEL);
	DeleteBitmap(IMAGE_CHAINFO_BTN_PET);
	DeleteBitmap(IMAGE_CHAINFO_BTN_QUEST);
	DeleteBitmap(IMAGE_CHAINFO_BTN_STAT);
	DeleteBitmap(IMAGE_CHAINFO_TEXTBOX);

	DeleteBitmap(IMAGE_CHAINFO_BTN_EXIT);

	DeleteBitmap(IMAGE_CHAINFO_BOTTOM);
	DeleteBitmap(IMAGE_CHAINFO_RIGHT);
	DeleteBitmap(IMAGE_CHAINFO_LEFT);
	DeleteBitmap(IMAGE_CHAINFO_TOP);
	DeleteBitmap(IMAGE_CHAINFO_BACK);

	DeleteBitmap(IMAGE_CHAINFO_TABLE_RIGHT_PIXEL);
	DeleteBitmap(IMAGE_CHAINFO_TABLE_LEFT_PIXEL);
	DeleteBitmap(IMAGE_CHAINFO_TABLE_BOTTOM_PIXEL);
	DeleteBitmap(IMAGE_CHAINFO_TABLE_TOP_PIXEL);
	DeleteBitmap(IMAGE_CHAINFO_TABLE_BOTTOM_RIGHT);
	DeleteBitmap(IMAGE_CHAINFO_TABLE_BOTTOM_LEFT);
	DeleteBitmap(IMAGE_CHAINFO_TABLE_TOP_RIGHT);
	DeleteBitmap(IMAGE_CHAINFO_TABLE_TOP_LEFT);
}

void SEASON3B::CNewUICharacterInfoWindow::OpenningProcess()
{

#ifdef KJH_FIX_WOPS_K28597_EQUIPED_LV10_DEFENSEITEM
	ResetEquipmentLevel();
#endif // KJH_FIX_WOPS_K28597_EQUIPED_LV10_DEFENSEITEM

	// 버튼 정도 초기화
	if(IsMasterLevel(Hero->Class) == true
#ifdef KJH_FIX_WOPS_K22193_SUMMONER_MASTERSKILL_UI_ABNORMAL_TEXT
		&& GetCharacterClass(Hero->Class) != CLASS_DIMENSIONMASTER			// 2008.06.13 현재는 소환술사 마스터스킬 불가. 풀릴때 define 주석처리
#endif // KJH_FIX_WOPS_K22193_SUMMONER_MASTERSKILL_UI_ABNORMAL_TEXT
#ifdef PBG_ADD_NEWCHAR_MONK
		&& GetCharacterClass(Hero->Class) != CLASS_TEMPLENIGHT
#endif //PBG_ADD_NEWCHAR_MONK
		)
	{
		m_BtnMasterLevel.UnLock();
		m_BtnMasterLevel.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
		m_BtnMasterLevel.ChangeTextColor(RGBA(255, 255, 255, 255));
	}
	else
	{
		m_BtnMasterLevel.Lock();
		m_BtnMasterLevel.ChangeImgColor(BUTTON_STATE_UP, RGBA(100, 100, 100, 255));
		m_BtnMasterLevel.ChangeTextColor(RGBA(100, 100, 100, 255));
	}

	// 세트 아이템 옵션 클래스 초기화
	g_csItemOption.init();

	// 내구력이 0이 아니면 계산을 전부 다시 한다.

	if( CharacterMachine->IsZeroDurability() )
	{
		CharacterMachine->CalculateAll();
	}

#ifdef ASG_ADD_NEW_QUEST_SYSTEM
// 캐릭터 창을 여는 퀘스트일 때 서버로 한 번만 알림.
	if (g_QuestMng.IsIndexInCurQuestIndexList(0x10009))	// 0x10009가 캐릭터 창을 여는 퀘스트 인덱스.
	{
		if (g_QuestMng.IsEPRequestRewardState(0x10009))
		{
#ifdef ASG_MOD_UI_QUEST_INFO
			g_pMyQuestInfoWindow->UnselectQuestList();	// 퀘스트 정보창의 퀘스트 리스트를 선택하지 않은 상태로.
#endif	// ASG_MOD_UI_QUEST_INFO
			SendSatisfyQuestRequestFromClient(0x10009);	// 서버로 캐릭터 정보창이 열렸음을 알림.
			g_QuestMng.SetEPRequestRewardState(0x10009, false);	// 서버로 한 번만 알리기 위해.
		}
	}
#endif	// ASG_ADD_NEW_QUEST_SYSTEM
}

#ifdef KJH_FIX_WOPS_K28597_EQUIPED_LV10_DEFENSEITEM
// 현재 장비된 아이템레벨을 다시 셋팅한다.
// 캐릭터 정보창 열었을때 호출.
//* WSClient::ReceiveCreatePlayerViewport() 에서 압축된 레벨 정보를 주인공 케릭터장비 레벨로
//* 셋팅하기 때문에 맞지 않다. 
//* 실제 아이템 레벨로 변경해 주어야 한다.
void SEASON3B::CNewUICharacterInfoWindow::ResetEquipmentLevel()
{
	ITEM *pItem = CharacterMachine->Equipment;
	Hero->Weapon[0].Level = (pItem[EQUIPMENT_WEAPON_RIGHT].Level>>3)&15;
	Hero->Weapon[1].Level = (pItem[EQUIPMENT_WEAPON_LEFT ].Level>>3)&15;
	Hero->BodyPart[BODYPART_HELM ].Level = (pItem[EQUIPMENT_HELM].Level>>3)&15;
	Hero->BodyPart[BODYPART_ARMOR].Level = (pItem[EQUIPMENT_ARMOR].Level>>3)&15;
	Hero->BodyPart[BODYPART_PANTS].Level = (pItem[EQUIPMENT_PANTS].Level>>3)&15;
	Hero->BodyPart[BODYPART_GLOVES].Level = (pItem[EQUIPMENT_GLOVES].Level>>3)&15;
	Hero->BodyPart[BODYPART_BOOTS].Level = (pItem[EQUIPMENT_BOOTS].Level>>3)&15;

	CheckFullSet(Hero);	
}
#endif // KJH_FIX_WOPS_K28597_EQUIPED_LV10_DEFENSEITEM
