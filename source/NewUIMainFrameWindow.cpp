// NewUIMainFrameWindow.cpp: implementation of the CNewUIMainFrameWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "NewUIMainFrameWindow.h"	// self
#include "NewUIOptionWindow.h"
#include "NewUISystem.h"
#include "UIBaseDef.h"
#include "DSPlaySound.h"
#include "ZzzInfomation.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"
#include "wsclientinline.h"
#include "CSItemOption.h"
#include "CSChaosCastle.h"

#ifdef PJH_FIX_SPRIT
#include "GIPetManager.h"
#endif //PJH_FIX_SPRIT
#ifndef PBG_MOD_STAMINA_UI
#ifdef PBG_ADD_SECRETBUFF
#include "FatigueTimeSystem.h"
#endif //PBG_ADD_SECRETBUFF
#endif //PBG_MOD_STAMINA_UI
#ifdef YDG_ADD_DOPPELGANGER_UI
#include "GMDoppelGanger1.h"
#include "GMDoppelGanger2.h"
#include "GMDoppelGanger3.h"
#include "GMDoppelGanger4.h"
#endif	// YDG_ADD_DOPPELGANGER_UI
#ifdef ASG_ADD_UI_QUEST_PROGRESS_ETC
#include "./Time/CTimCheck.h"
#endif	// ASG_ADD_UI_QUEST_PROGRESS_ETC
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "InGameShopSystem.h"
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
#include "MonkSystem.h"
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
#ifdef LDK_ADD_SCALEFORM
#include "CGFxProcess.h"
#endif //LDK_ADD_SCALEFORM

extern float g_fScreenRate_x;
extern float g_fScreenRate_y;
extern int  MouseUpdateTime;
extern int  MouseUpdateTimeMax;
extern int World;
extern int SelectedCharacter;
extern int Attacking;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SEASON3B::CNewUIMainFrameWindow::CNewUIMainFrameWindow() 
{
	m_bExpEffect = false;
	m_dwExpEffectTime = 0;
	m_dwPreExp = 0;
	m_dwGetExp = 0;
#ifdef ASG_ADD_UI_QUEST_PROGRESS_ETC
	m_bButtonBlink = false;
#endif	// ASG_ADD_UI_QUEST_PROGRESS_ETC
}

SEASON3B::CNewUIMainFrameWindow::~CNewUIMainFrameWindow() 
{ 
	Release(); 
}

void SEASON3B::CNewUIMainFrameWindow::LoadImages()
{
	LoadBitmap("Interface\\newui_menu01.jpg", IMAGE_MENU_1, GL_LINEAR);
	LoadBitmap("Interface\\newui_menu02.jpg", IMAGE_MENU_2, GL_LINEAR);
#if defined NEW_USER_INTERFACE_MAINFRAME_BUTTON || defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
	LoadBitmap("Interface\\partCharge1\\newui_menu03.jpg", IMAGE_MENU_3, GL_LINEAR);
#else //defined NEW_USER_INTERFACE_MAINFRAME_BUTTON || defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
	LoadBitmap("Interface\\newui_menu03.jpg", IMAGE_MENU_3, GL_LINEAR);
#endif //defined NEW_USER_INTERFACE_MAINFRAME_BUTTON || defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
	LoadBitmap("Interface\\newui_menu02-03.jpg", IMAGE_MENU_2_1, GL_LINEAR);
	LoadBitmap("Interface\\newui_menu_blue.jpg", IMAGE_GAUGE_BLUE, GL_LINEAR);
	LoadBitmap("Interface\\newui_menu_green.jpg", IMAGE_GAUGE_GREEN, GL_LINEAR);
	LoadBitmap("Interface\\newui_menu_red.jpg", IMAGE_GAUGE_RED, GL_LINEAR);
	LoadBitmap("Interface\\newui_menu_ag.jpg", IMAGE_GAUGE_AG, GL_LINEAR);
	LoadBitmap("Interface\\newui_menu_sd.jpg", IMAGE_GAUGE_SD, GL_LINEAR);
	LoadBitmap("Interface\\newui_exbar.jpg", IMAGE_GAUGE_EXBAR, GL_LINEAR);
	LoadBitmap("Interface\\Exbar_Master.jpg", IMAGE_MASTER_GAUGE_BAR, GL_LINEAR);
#if defined NEW_USER_INTERFACE_MAINFRAME_BUTTON || defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
	LoadBitmap("Interface\\partCharge1\\newui_menu_Bt05.jpg", IMAGE_MENU_BTN_CSHOP, GL_LINEAR, GL_CLAMP);
	LoadBitmap("Interface\\partCharge1\\newui_menu_Bt01.jpg", IMAGE_MENU_BTN_CHAINFO, GL_LINEAR, GL_CLAMP);
	LoadBitmap("Interface\\partCharge1\\newui_menu_Bt02.jpg", IMAGE_MENU_BTN_MYINVEN, GL_LINEAR, GL_CLAMP);
	LoadBitmap("Interface\\partCharge1\\newui_menu_Bt03.jpg", IMAGE_MENU_BTN_FRIEND, GL_LINEAR, GL_CLAMP);
	LoadBitmap("Interface\\partCharge1\\newui_menu_Bt04.jpg", IMAGE_MENU_BTN_WINDOW, GL_LINEAR, GL_CLAMP);
#else //defined NEW_USER_INTERFACE_MAINFRAME_BUTTON || defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
	LoadBitmap("Interface\\newui_menu_Bt01.jpg", IMAGE_MENU_BTN_CHAINFO, GL_LINEAR, GL_CLAMP);
	LoadBitmap("Interface\\newui_menu_Bt02.jpg", IMAGE_MENU_BTN_MYINVEN, GL_LINEAR, GL_CLAMP);
	LoadBitmap("Interface\\newui_menu_Bt03.jpg", IMAGE_MENU_BTN_FRIEND, GL_LINEAR, GL_CLAMP);
	LoadBitmap("Interface\\newui_menu_Bt04.jpg", IMAGE_MENU_BTN_WINDOW, GL_LINEAR, GL_CLAMP);
#endif //defined NEW_USER_INTERFACE_MAINFRAME_BUTTON || defined PBG_ADD_INGAMESHOP_UI_MAINFRAME

}

void SEASON3B::CNewUIMainFrameWindow::UnloadImages()
{
	DeleteBitmap(IMAGE_MENU_1);
	DeleteBitmap(IMAGE_MENU_2);
	DeleteBitmap(IMAGE_MENU_3);
	DeleteBitmap(IMAGE_MENU_2_1);
	DeleteBitmap(IMAGE_GAUGE_BLUE);
	DeleteBitmap(IMAGE_GAUGE_GREEN);
	DeleteBitmap(IMAGE_GAUGE_RED);
	DeleteBitmap(IMAGE_GAUGE_AG);
	DeleteBitmap(IMAGE_GAUGE_SD);
	DeleteBitmap(IMAGE_GAUGE_EXBAR);
	DeleteBitmap(IMAGE_MENU_BTN_CHAINFO);
	DeleteBitmap(IMAGE_MENU_BTN_MYINVEN);
	DeleteBitmap(IMAGE_MENU_BTN_FRIEND);
	DeleteBitmap(IMAGE_MENU_BTN_WINDOW);
}

bool SEASON3B::CNewUIMainFrameWindow::Create(CNewUIManager* pNewUIMng, CNewUI3DRenderMng* pNewUI3DRenderMng)
{
	if(NULL == pNewUIMng || NULL == pNewUI3DRenderMng)
		return false;

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_MAINFRAME, this);

	m_pNewUI3DRenderMng = pNewUI3DRenderMng;
	m_pNewUI3DRenderMng->Add3DRenderObj(this, ITEMHOTKEYNUMBER_CAMERA_Z_ORDER);

	LoadImages();

	SetButtonInfo();

	Show(true);
	
	return true;
}

void SEASON3B::CNewUIMainFrameWindow::SetButtonInfo()
{	
#if defined NEW_USER_INTERFACE_MAINFRAME_BUTTON || defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
	int x_Next = 489;
	int y_Next = 480-51;
	int x_Add = 30;
	int y_Add = 41;
	m_BtnCShop.ChangeTextBackColor(RGBA(255,255,255,0));
	m_BtnCShop.ChangeButtonImgState( true, IMAGE_MENU_BTN_CSHOP, true );
	m_BtnCShop.ChangeButtonInfo(x_Next, y_Next, x_Add, y_Add);
	x_Next += x_Add;
	m_BtnCShop.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
	m_BtnCShop.ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
	m_BtnCShop.ChangeToolTipText(GlobalText[2277], true);
#else //defined NEW_USER_INTERFACE_MAINFRAME_BUTTON || defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
	int x_Next = 488;
	int y_Next = 480-51;
	int x_Add = 38;
	int y_Add = 42;
#endif //defined NEW_USER_INTERFACE_MAINFRAME_BUTTON || defined PBG_ADD_INGAMESHOP_UI_MAINFRAME

	m_BtnChaInfo.ChangeTextBackColor(RGBA(255,255,255,0));
	m_BtnChaInfo.ChangeButtonImgState( true, IMAGE_MENU_BTN_CHAINFO, true );
	m_BtnChaInfo.ChangeButtonInfo(x_Next, y_Next, x_Add, y_Add);	
	x_Next += x_Add;
	m_BtnChaInfo.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
	m_BtnChaInfo.ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
	// 362 "케릭터(C)"
	m_BtnChaInfo.ChangeToolTipText(GlobalText[362], true);

	m_BtnMyInven.ChangeTextBackColor(RGBA(255,255,255,0));
	m_BtnMyInven.ChangeButtonImgState( true, IMAGE_MENU_BTN_MYINVEN, true );
	m_BtnMyInven.ChangeButtonInfo(x_Next, y_Next, x_Add, y_Add);	
	x_Next += x_Add;
	m_BtnMyInven.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
	m_BtnMyInven.ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
	// 363 "장비(I,V)"
	m_BtnMyInven.ChangeToolTipText(GlobalText[363], true);

	m_BtnFriend.ChangeTextBackColor(RGBA(255,255,255,0));
	m_BtnFriend.ChangeButtonImgState( true, IMAGE_MENU_BTN_FRIEND, true );
	m_BtnFriend.ChangeButtonInfo(x_Next, y_Next, x_Add, y_Add);	
	x_Next += x_Add;
	m_BtnFriend.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
	m_BtnFriend.ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
	// 1043 "내친구(F)"
	m_BtnFriend.ChangeToolTipText(GlobalText[1043], true);

	m_BtnWindow.ChangeTextBackColor(RGBA(255,255,255,0));
	m_BtnWindow.ChangeButtonImgState( true, IMAGE_MENU_BTN_WINDOW, true );
	m_BtnWindow.ChangeButtonInfo(x_Next, y_Next, x_Add, y_Add);	
	m_BtnWindow.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
	m_BtnWindow.ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
	// 1744 "메뉴(U)"
	m_BtnWindow.ChangeToolTipText(GlobalText[1744], true);
}

void SEASON3B::CNewUIMainFrameWindow::Release()
{
	UnloadImages();

	if(m_pNewUI3DRenderMng)
	{
		m_pNewUI3DRenderMng->Remove3DRenderObj(this);
		m_pNewUI3DRenderMng = NULL;
	}

	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}
}

bool SEASON3B::CNewUIMainFrameWindow::Render()
{
	EnableAlphaTest();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// 틀 렌더링
	RenderFrame();

	// HotKey 아이템들 갯수 렌더링
	m_pNewUI3DRenderMng->RenderUI2DEffect(ITEMHOTKEYNUMBER_CAMERA_Z_ORDER, UI2DEffectCallback, this, 0, 0);

	// Skill 아이콘 렌더링
	g_pSkillList->RenderCurrentSkillAndHotSkillList();

	// Skill 설명시에 AlphaTest가 꺼지는 것 같음.
	EnableAlphaTest();

	// Life, Mana 렌더링
	RenderLifeMana();

	// SD 게이지 렌더링
	RenderGuageSD();

	// AG 게이지 렌더링
	RenderGuageAG();

	// 버튼 렌더링
	RenderButtons();

	// 경험치 렌더링
	RenderExperience();

	DisableAlphaBlend();

	return true;
}

void SEASON3B::CNewUIMainFrameWindow::Render3D()
{
	m_ItemHotKey.RenderItems();
}

void SEASON3B::CNewUIMainFrameWindow::UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB)
{
#ifdef LDK_ADD_SCALEFORM
	if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
	{
		g_pMainFrame->RenderHotKeyItemCount();
	}
#else //LDK_ADD_SCALEFORM
	g_pMainFrame->RenderHotKeyItemCount();
#endif //LDK_ADD_SCALEFORM
}

bool SEASON3B::CNewUIMainFrameWindow::IsVisible() const
{
	return CNewUIObj::IsVisible();
}

void SEASON3B::CNewUIMainFrameWindow::RenderFrame()
{
	float width, height;	// 이미지 실제 넓이, 높이 값
	float x, y;				// 이미지 위치 x, y 값
	
	// MainFrame 틀 먼저 렌더링
	width = 256.f; height = 51.f;
	x = 0.f; y = 480.f - height;
	SEASON3B::RenderImage(IMAGE_MENU_1, x, y, width, height);
	width = 128.f;
	x = 256.f; 
	SEASON3B::RenderImage(IMAGE_MENU_2, x, y, width, height);
	width = 256.f;
	x = 256.f + 128.f;
	SEASON3B::RenderImage(IMAGE_MENU_3, x, y, width, height);

	if(g_pSkillList->IsSkillListUp() == true)
	{
		width = 160.f; height = 40.f;
		x = 222.f;
		SEASON3B::RenderImage(IMAGE_MENU_2_1, x, y, width, height);
	}
}

void SEASON3B::CNewUIMainFrameWindow::RenderLifeMana()
{
	WORD wLifeMax, wLife, wManaMax, wMana;


	if(IsMasterLevel( Hero->Class ) == true )
	{
		wLifeMax = Master_Level_Data.wMaxLife;
		wLife = min(max(0, CharacterAttribute->Life), wLifeMax);
		wManaMax = Master_Level_Data.wMaxMana;
		wMana = min(max(0, CharacterAttribute->Mana), wManaMax);
	}
	else
	{
		wLifeMax = CharacterAttribute->LifeMax;
		wLife = min(max(0, CharacterAttribute->Life), wLifeMax);
		wManaMax = CharacterAttribute->ManaMax;
		wMana = min(max(0, CharacterAttribute->Mana), wManaMax);
	}

	// 생명력 수치가 20%보다 낮으면 심장소리 내주는 센스
	if(wLifeMax > 0)
	{
		if(wLife > 0 && (wLife / (float)wLifeMax) < 0.2f)
		{
			PlayBuffer(SOUND_HEART);
		}
	}

	// 나눗셈 예외처리
#ifdef _VS2008PORTING
	float fLife = 0.f;
	float fMana = 0.f;	// 확률
#else // _VS2008PORTING
	float fLife, fMana;	// 확률
#endif // _VS2008PORTING
#ifdef PBG_ADD_NEWCHAR_MONK
	//초기화 되지 않는 값사용
	fLife =0;
	fMana =0;
#endif //PBG_ADD_NEWCHAR_MONK

	if(wLifeMax > 0)
	{
		fLife = (wLifeMax - wLife) / (float)wLifeMax;
	}
	if(wManaMax > 0)	
	{
		fMana = (wManaMax - wMana) / (float)wManaMax;
	}

	float width, height;	// 이미지 실제 넓이, 높이 값
	float x, y;				// 이미지 위치 x, y 값
	float fY, fH, fV;

	// life
	width = 45.f; 
	x = 158;
	height = 39.f; 
	y = 480.f - 48.f;
	
	// 독속성에 맞았으면 녹색에너지 그리고 아니면 빨강에너지 그린다.	
	fY = y + (fLife * height);
	fH = height - (fLife * height);
	fV = fLife;
	if( g_isCharacterBuff((&Hero->Object), eDeBuff_Poison) )
	{
		RenderBitmap(IMAGE_GAUGE_GREEN, x, fY, width, fH, 0.f, fV*height/64.f, width/64.f, (1.0f - fV)*height/64.f);
	}
	else
	{
		RenderBitmap(IMAGE_GAUGE_RED, x, fY, width, fH, 0.f, fV*height/64.f, width/64.f, (1.0f - fV)*height/64.f);
	}

	SEASON3B::RenderNumber(x+25, 480-18, wLife);

	// life 설명
	char strTipText[256];
	if(SEASON3B::CheckMouseIn(x, y, width, height) == true)
	{
		sprintf(strTipText, GlobalText[358], wLife, wLifeMax);
		RenderTipText((int)x, (int)418, strTipText);
	}

	// mana
	width = 45.f; 
	x = 256.f + 128.f + 53.f; 
	height = 39.f;
	y = 480.f - 48.f;

	fY = y + (fMana * height);
	fH = height - (fMana * height);
	fV = fMana;
	RenderBitmap(IMAGE_GAUGE_BLUE, x, fY, width, fH, 0.f, fV*height/64.f, width/64.f, (1.0f - fV)*height/64.f);

	SEASON3B::RenderNumber(x+30, 480-18, wMana);

	// mana 설명
	if(SEASON3B::CheckMouseIn(x, y, width, height) == true)
	{
		sprintf(strTipText, GlobalText[359], wMana, wManaMax);
		RenderTipText((int)x, (int)418, strTipText);
	}
}

void SEASON3B::CNewUIMainFrameWindow::RenderGuageAG()
{
	float x, y, width, height;
	float fY, fH, fV;

	WORD dwMaxSkillMana,dwSkillMana;

	if(IsMasterLevel(Hero->Class) == true)
	{
		
		dwMaxSkillMana = max(1, Master_Level_Data.wMaxBP);
		dwSkillMana = min(dwMaxSkillMana, CharacterAttribute->SkillMana);
	}
	else
	{
		dwMaxSkillMana = max(1, CharacterAttribute->SkillManaMax);
		dwSkillMana = min(dwMaxSkillMana, CharacterAttribute->SkillMana);
	}

	// 나눗셈 예외처리
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	float fSkillMana = 0.0f;	// 확률
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	float fSkillMana;	// 확률
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	if(dwMaxSkillMana > 0)
	{
		fSkillMana = (dwMaxSkillMana - dwSkillMana) / (float)dwMaxSkillMana;
	}

	width = 16.f, height = 39.f;
	x = 256 + 128 + 36; y = 480.f - 49.f;
	fY = y + (fSkillMana * height);
	fH = height - (fSkillMana * height);
	fV = fSkillMana;

	RenderBitmap(IMAGE_GAUGE_AG, x, fY, width, fH, 0.f, fV*height/64.f, width/16.f, (1.0f - fV)*height/64.f);
	SEASON3B::RenderNumber(x+10, 480-18, (int)dwSkillMana);

	if(SEASON3B::CheckMouseIn(x, y, width, height) == true)
	{	
		char strTipText[256];

        sprintf(strTipText, GlobalText[214], dwSkillMana, dwMaxSkillMana);
		RenderTipText((int)x-20, (int)418, strTipText);
    }
}

void SEASON3B::CNewUIMainFrameWindow::RenderGuageSD()
{
	float x, y, width, height;
	float fY, fH, fV;
    WORD wMaxShield,wShield;


	//Master_Level_Data.wMaxShield
	if(IsMasterLevel(Hero->Class) == true)
	{
		wMaxShield = max (1, Master_Level_Data.wMaxShield);
		wShield = min (wMaxShield, CharacterAttribute->Shield);
	}
	else
	{
		wMaxShield = max (1, CharacterAttribute->ShieldMax);
		wShield = min (wMaxShield, CharacterAttribute->Shield);
	}
	
	// 나눗셈 예외처리
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	float fShield = 0.0f;	// 확률
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	float fShield;	// 확률
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	if(wMaxShield > 0)
	{
		fShield = (wMaxShield - wShield) / (float)wMaxShield;
	}

	width = 16.f, height = 39.f;
	x = 204; y = 480.f - 49.f;
	fY = y + (fShield * height);
	fH = height - (fShield * height);
	fV = fShield;

	RenderBitmap(IMAGE_GAUGE_SD, x, fY, width, fH, 0.f, fV*height/64.f, width/16.f, (1.0f - fV)*height/64.f);
	SEASON3B::RenderNumber(x+15, 480-18, (int)wShield);

	height = 39.f;
	y = 480.f - 10.f - 39.f;
	if(SEASON3B::CheckMouseIn(x, y, width, height) == true)
	{	
		char strTipText[256];

		// 2037 " S D : %d / %d"
        sprintf(strTipText, GlobalText[2037], wShield, wMaxShield);
		RenderTipText((int)x-20, (int)418, strTipText);
    }
}

void SEASON3B::CNewUIMainFrameWindow::RenderExperience()
{
	
	__int64 wLevel;				// 현재 레벨
	__int64 dwNexExperience;	// 다음 레벨업 경험치
	__int64 dwExperience;		// 현재 경험치
	double x, y, width, height;


	if(IsMasterLevel(CharacterAttribute->Class) == true)
	{
		wLevel = (__int64)Master_Level_Data.nMLevel;	// 현재 마스터 레벨
		dwNexExperience = (__int64)Master_Level_Data.lNext_MasterLevel_Experince;
		dwExperience = (__int64)Master_Level_Data.lMasterLevel_Experince;
	}
	else
	{
		wLevel = CharacterAttribute->Level;
		dwNexExperience = CharacterAttribute->NextExperince;
		dwExperience = CharacterAttribute->Experience;
	}

	if(IsMasterLevel(CharacterAttribute->Class) == true)
	{
		x = 0; y = 470; width = 6; height = 4;

		__int64 iTotalLevel = wLevel + 400;				// 종합레벨 - 400렙이 만렙이기 때문에 더해준다.
		__int64 iTOverLevel = iTotalLevel - 255;		// 255레벨 이상 기준 레벨
		__int64 iBaseExperience = 0;					// 레벨 초기 경험치

		__int64 iData_Master =	// A
			(
				(
					(__int64)9 + (__int64)iTotalLevel
				)
				* (__int64)iTotalLevel
				* (__int64)iTotalLevel
				* (__int64)10
			)
			+
			(
				(
					(__int64)9 + (__int64)iTOverLevel
				)
				* (__int64)iTOverLevel
				* (__int64)iTOverLevel
				* (__int64)1000
			);
		iBaseExperience = (iData_Master - (__int64)3892250000) / (__int64)2;	// B

		// 레벨업 경험치
		double fNeedExp = (double)dwNexExperience - (double)iBaseExperience;

		// 현재 획득한 경험치
		double fExp = (double)dwExperience - (double)iBaseExperience;

		if(dwExperience < iBaseExperience)	// 에러
		{
			fExp = 0.f;
		}

		double fExpBarNum = 0.f;	// 경험치바 단계(0 ~ 9) -> 오른쪽 숫자
		if(fExp > 0.f && fNeedExp > 0)
		{
			fExpBarNum = ((double)fExp / (double)fNeedExp) * (double)10.f;
		}

		double fProgress = fExpBarNum - __int64(fExpBarNum);	// 0.0 ~ 1.0까지 게이지바 한줄에서의 길이

		if(m_bExpEffect == true)
		{
			double fPreProgress = 0.f;	// 원래 있어서 이펙트 안칠해도 되는 부분
			double fPreExp = (double)m_loPreExp - (double)iBaseExperience;	// 이전 경험치
			if(m_loPreExp < iBaseExperience)	// 경험치바 한줄 넘어감
			{
				x = 2.f; y = 473.f; width = fProgress * 629.f; height = 4.f;
				RenderBitmap(IMAGE_MASTER_GAUGE_BAR, x, y, width, height, 0.f, 0.f, 6.f/8.f, 4.f/4.f);
				//만렙해제
				glColor4f(1.f, 1.f, 1.f, 0.6f);
				RenderColor(x, y, width, height);
				EndRenderColor();
			}
			else	// 경험치바 넘어가지 않음
			{
				int iPreExpBarNum = 0;	// 이전 경험치바 길이 (0.0 ~ 1.0)
				int iExpBarNum = 0;		// 새 경험치바 길이 (0.0 ~ 1.0)
				if(fPreExp > 0.f && fNeedExp > 0.f)
				{
					fPreProgress = ((double)fPreExp / (double)fNeedExp) * (double)10.f;
					iPreExpBarNum = (int)fPreProgress;
					fPreProgress = (double)fPreProgress - __int64(fPreProgress);	// 0.0 ~ 1.0까지 게이지바 한줄에서의 길이
				}
				iExpBarNum = (int)fExpBarNum;

				if(iExpBarNum > iPreExpBarNum)
				{
					x = 2.f; y = 473.f; width = fProgress * 629.f; height = 4.f;
					RenderBitmap(IMAGE_MASTER_GAUGE_BAR, x, y, width, height, 0.f, 0.f, 6.f/8.f, 4.f/4.f);
					glColor4f(1.f, 1.f, 1.f, 0.6f);
					RenderColor(x, y, width, height);
					EndRenderColor();
				}
				else
				{
					double fGapProgress = 0.f;
					fGapProgress = (double)fProgress - (double)fPreProgress;
					x = 2.f; y = 473.f; width = (double)fPreProgress * (double)629.f; height = 4.f;
					RenderBitmap(IMAGE_MASTER_GAUGE_BAR, x, y, width, height, 0.f, 0.f, 6.f/8.f, 4.f/4.f);

					x += width; width = (double)fGapProgress * (double)629.f;
					RenderBitmap(IMAGE_MASTER_GAUGE_BAR, x, y, width, height, 0.f, 0.f, 6.f/8.f, 4.f/4.f);
					glColor4f(1.f, 1.f, 1.f, 0.6f);
					RenderColor(x, y, width, height);
					EndRenderColor();
				}
			}
		}
		else
		{
			x = 2.f; y = 473.f; width = fProgress * 629.f; height = 4.f;
			RenderBitmap(IMAGE_MASTER_GAUGE_BAR, x, y, width, height, 0.f, 0.f, 6.f/8.f, 4.f/4.f);
		}

		// 경험치 바 숫자
		int iExp = (int)fExpBarNum;
		x = 635.f; y = 469.f;
		SEASON3B::RenderNumber(x, y, iExp);

		x = 2.f; y = 473.f; width = 629.f; height = 4.f;
		if(SEASON3B::CheckMouseIn(x, y, width, height) == true)
		{	
			char strTipText[256];
	
			// 1748 "경험치 :%I64d / %I64d"
			sprintf(strTipText, GlobalText[1748], dwExperience, dwNexExperience);
#ifndef PBG_MOD_STAMINA_UI
#ifdef PBG_ADD_SECRETBUFF
			// 피로도 시스템을 사용하는가
			if(g_FatigueTimeSystem->IsFatigueSystem())
			{
				char strfatigue[128]={0,};
				// 임시사용 차후 수정 "피로도 : %d"
				sprintf(strfatigue, GlobalText[2867], g_FatigueTimeSystem->GetFatiguePercentage());

				strcat(strTipText, "    ");
				strcat(strTipText, strfatigue);
			}
#endif //PBG_ADD_SECRETBUFF
#endif //PBG_MOD_STAMINA_UI
			RenderTipText(280, 418, strTipText);
		}
	}
	else
	{
		x = 0; y = 470; width = 6; height = 4;

		WORD wPriorLevel = wLevel - 1;
		DWORD dwPriorExperience = 0;

		if(wPriorLevel > 0)
		{
			dwPriorExperience = (9 + wPriorLevel) * wPriorLevel * wPriorLevel * 10;

			if(wPriorLevel > 255)
			{
				int iLevelOverN = wPriorLevel - 255;
				dwPriorExperience += (9 + iLevelOverN) * iLevelOverN * iLevelOverN * 1000;
			}
		}

		// 레벨업 경험치
		float fNeedExp = dwNexExperience - dwPriorExperience;

		// 현재 획득한 경험치
		float fExp = dwExperience - dwPriorExperience;

		if(dwExperience < dwPriorExperience)
		{
			fExp = 0.f;
		}

		// 경험치바 단계(0 ~ 9)
		float fExpBarNum = 0.f;
		if(fExp > 0.f && fNeedExp > 0)
		{
			fExpBarNum = (fExp / fNeedExp) * 10.f;
		}

		float fProgress = fExpBarNum;
		fProgress = fProgress - (int)fProgress;

		if(m_bExpEffect == true)
		{
			float fPreProgress = 0.f;
			fExp = m_dwPreExp - dwPriorExperience;
			if(m_dwPreExp < dwPriorExperience)
			{
				x = 2.f; y = 473.f; width = fProgress * 629.f; height = 4.f;
				RenderBitmap(IMAGE_GAUGE_EXBAR, x, y, width, height, 0.f, 0.f, 6.f/8.f, 4.f/4.f);
				//만렙해제
				glColor4f(1.f, 1.f, 1.f, 0.4f);
				RenderColor(x, y, width, height);
				EndRenderColor();
			}
			else
			{
				int iPreExpBarNum = 0;
				int iExpBarNum = 0;
				if(fExp > 0.f && fNeedExp > 0.f)
				{
					fPreProgress = (fExp / fNeedExp) * 10.f;
					iPreExpBarNum = (int)fPreProgress;
					fPreProgress = fPreProgress - (int)fPreProgress;
				}

				iExpBarNum = (int)fExpBarNum;

				if(iExpBarNum > iPreExpBarNum)
				{
					x = 2.f; y = 473.f; width = fProgress * 629.f; height = 4.f;
					RenderBitmap(IMAGE_GAUGE_EXBAR, x, y, width, height, 0.f, 0.f, 6.f/8.f, 4.f/4.f);
					glColor4f(1.f, 1.f, 1.f, 0.4f);
					RenderColor(x, y, width, height);
					EndRenderColor();
				}
				else
				{
					float fGapProgress = 0.f;
					fGapProgress = fProgress - fPreProgress;
					x = 2.f; y = 473.f; width = fPreProgress * 629.f; height = 4.f;
					RenderBitmap(IMAGE_GAUGE_EXBAR, x, y, width, height, 0.f, 0.f, 6.f/8.f, 4.f/4.f);
					x += width; width = fGapProgress * 629.f;
					RenderBitmap(IMAGE_GAUGE_EXBAR, x, y, width, height, 0.f, 0.f, 6.f/8.f, 4.f/4.f);
					glColor4f(1.f, 1.f, 1.f, 0.4f);
					RenderColor(x, y, width, height);
					EndRenderColor();
				}
			}
		}
		else
		{
			x = 2.f; y = 473.f; width = fProgress * 629.f; height = 4.f;
			RenderBitmap(IMAGE_GAUGE_EXBAR, x, y, width, height, 0.f, 0.f, 6.f/8.f, 4.f/4.f);
		}

		// 경험치 바 숫자
		int iExp = (int)fExpBarNum;
		x = 635.f; y = 469.f;
		SEASON3B::RenderNumber(x, y, iExp);

		x = 2.f; y = 473.f; width = 629.f; height = 4.f;
		if(SEASON3B::CheckMouseIn(x, y, width, height) == true)
		{	
			char strTipText[256];
	
			// 1748 "경험치 :%I64d / %I64d"
			sprintf(strTipText, GlobalText[1748], dwExperience, dwNexExperience);
#ifndef PBG_MOD_STAMINA_UI
#ifdef PBG_ADD_SECRETBUFF
			if(g_FatigueTimeSystem->IsFatigueSystem())
			{
				char strfatigue[128]={0,};
				// 임시사용 차후 수정 "피로도 : %d"
				sprintf(strfatigue, GlobalText[2867], g_FatigueTimeSystem->GetFatiguePercentage());
				strcat(strTipText, "    ");
				strcat(strTipText, strfatigue);
			}
#endif //PBG_ADD_SECRETBUFF
#endif //PBG_MOD_STAMINA_UI
			RenderTipText(280, 418, strTipText);
		}
	}
}

void SEASON3B::CNewUIMainFrameWindow::RenderHotKeyItemCount()
{
	m_ItemHotKey.RenderItemCount();
}

void SEASON3B::CNewUIMainFrameWindow::RenderButtons()
{
#if defined NEW_USER_INTERFACE_MAINFRAME_BUTTON || defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
	m_BtnCShop.Render();
#endif //defined NEW_USER_INTERFACE_MAINFRAME_BUTTON || defined PBG_ADD_INGAMESHOP_UI_MAINFRAME

#ifdef ASG_ADD_UI_QUEST_PROGRESS_ETC
	RenderCharInfoButton();
#else	// ASG_ADD_UI_QUEST_PROGRESS_ETC
	m_BtnChaInfo.Render();
#endif	// ASG_ADD_UI_QUEST_PROGRESS_ETC
	m_BtnMyInven.Render();

	RenderFriendButton();

	m_BtnWindow.Render();
}

#ifdef ASG_ADD_UI_QUEST_PROGRESS_ETC
void SEASON3B::CNewUIMainFrameWindow::RenderCharInfoButton()
{
	m_BtnChaInfo.Render();

	// 기타 상황에 의한 (NPC 클릭이 아닌)퀘스트가 없다면.
	if (g_QuestMng.IsQuestIndexByEtcListEmpty())
		return;

	if (g_Time.GetTimeCheck(5, 500))	// 0.5초 마다 true를 주는 5번 인덱스 타이머에 등록 및 체크.
		m_bButtonBlink = !m_bButtonBlink;

	if (m_bButtonBlink)
	{
		// 캐릭터 정보창과 기타 퀘스트 진행창이 안보일 때 깜박임.
#if defined NEW_USER_INTERFACE_MAINFRAME_BUTTON || defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
		if (!(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC)
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHARACTER)))
			RenderImage(IMAGE_MENU_BTN_CHAINFO, 489+30, 480-51, 30, 41, 0.0f, 41.f);
#else //defined NEW_USER_INTERFACE_MAINFRAME_BUTTON || defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
		if (!(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC)
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHARACTER)))
			RenderImage(IMAGE_MENU_BTN_CHAINFO, 488, 480-51, 38, 42, 0.0f, 42.f);
#endif//defined NEW_USER_INTERFACE_MAINFRAME_BUTTON || defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
	}
}
#endif	// ASG_ADD_UI_QUEST_PROGRESS_ETC

void SEASON3B::CNewUIMainFrameWindow::RenderFriendButton()
{
	m_BtnFriend.Render();

	int iBlinkTemp = g_pFriendMenu->GetBlinkTemp();
	BOOL bIsAlertTime = (iBlinkTemp % 24 < 12);

	if (g_pFriendMenu->IsNewChatAlert() && bIsAlertTime)		// 채팅말 경고 이미지
	{
		RenderFriendButtonState();	
	}
	if (g_pFriendMenu->IsNewMailAlert())			// 편지 경고이미지
	{
		if (bIsAlertTime)
		{
			RenderFriendButtonState();

			if (iBlinkTemp % 24 == 11) 
			{
				g_pFriendMenu->IncreaseLetterBlink();
			}
		}
	}
	else if (g_pLetterList->CheckNoReadLetter())	// 편지 미확인 이미지
	{
		RenderFriendButtonState();
	}

	g_pFriendMenu->IncreaseBlinkTemp();
}

void SEASON3B::CNewUIMainFrameWindow::RenderFriendButtonState()
{
#if defined NEW_USER_INTERFACE_MAINFRAME_BUTTON || defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_FRIEND) == true)
	{
		RenderImage(IMAGE_MENU_BTN_FRIEND, 489+(30*3), 480-51, 30, 41, 0.0f, 123.f);
	}
	else
	{
		RenderImage(IMAGE_MENU_BTN_FRIEND, 489+(30*3), 480-51, 30, 41, 0.0f, 41.f);
	}
#else //defined NEW_USER_INTERFACE_MAINFRAME_BUTTON || defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_FRIEND) == true)
	{
		RenderImage(IMAGE_MENU_BTN_FRIEND, 488+76, 480-51, 38, 42, 0.0f, 126.f);
	}
	else
	{
		RenderImage(IMAGE_MENU_BTN_FRIEND, 488+76, 480-51, 38, 42, 0.0f, 42.f);
	}
#endif//defined NEW_USER_INTERFACE_MAINFRAME_BUTTON || defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
}

bool SEASON3B::CNewUIMainFrameWindow::UpdateMouseEvent()
{
	if( g_pNewUIHotKey->IsStateGameOver() == true )
	{
		return true;
	}

	if(BtnProcess() == true)
	{
		return false;
	}

	return true;
}

bool SEASON3B::CNewUIMainFrameWindow::BtnProcess()
{
	if(g_pNewUIHotKey->CanUpdateKeyEventRelatedMyInventory() == true)
	{
		if(m_BtnMyInven.UpdateMouseEvent() == true)
		{
			g_pNewUISystem->Toggle(SEASON3B::INTERFACE_INVENTORY);
			PlayBuffer(SOUND_CLICK01);
			return true;
		}
	}
	else if(g_pNewUIHotKey->CanUpdateKeyEvent() == true)
	{
		if(m_BtnMyInven.UpdateMouseEvent() == true)
		{
			g_pNewUISystem->Toggle(SEASON3B::INTERFACE_INVENTORY);
			PlayBuffer(SOUND_CLICK01);
			return true;
		}
		else if(m_BtnChaInfo.UpdateMouseEvent() == true)
		{
			g_pNewUISystem->Toggle(SEASON3B::INTERFACE_CHARACTER);
			PlayBuffer(SOUND_CLICK01);
#ifdef ASG_ADD_UI_QUEST_PROGRESS_ETC
			if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHARACTER))
				g_QuestMng.SendQuestIndexByEtcSelection();	// 기타 상황에 의한 퀘스트 인덱스를 선택해서 서버로 알림.
#endif	// ASG_ADD_UI_QUEST_PROGRESS_ETC
			return true;
		}
		else if(m_BtnFriend.UpdateMouseEvent() == true)
		{
#ifdef CSK_FIX_CHAOSFRIENDWINDOW
			if( InChaosCastle() == true )
			{
				PlayBuffer(SOUND_CLICK01);
				return true;
			}
#endif //! CSK_FIX_CHAOSFRIENDWINDOW	// 정리할 때 지워야 하는 소스

			int iLevel = CharacterAttribute->Level;
			
			if(iLevel < 6)
			{
				if(g_pChatListBox->CheckChatRedundancy(GlobalText[1067]) == FALSE)
				{
					g_pChatListBox->AddText("",GlobalText[1067],SEASON3B::TYPE_SYSTEM_MESSAGE);	// "레벨 6부터 내친구 기능 사용이 가능합니다."
				}
			}
			else
			{
				g_pNewUISystem->Toggle(SEASON3B::INTERFACE_FRIEND);
			}
			PlayBuffer(SOUND_CLICK01);
			return true;
		}
		else if(m_BtnWindow.UpdateMouseEvent() == true)
		{
			g_pNewUISystem->Toggle(SEASON3B::INTERFACE_WINDOW_MENU);
			PlayBuffer(SOUND_CLICK01);
			return true;
		}
#if defined NEW_USER_INTERFACE_SHELL && !defined LDK_MOD_GLOBAL_PORTAL_CASHSHOP_BUTTON_DENY
		else if(m_BtnCShop.UpdateMouseEvent() == true)
		{
			if( g_pNewUISystem->IsVisible( SEASON3B::INTERFACE_PARTCHARGE_SHOP ) == false ) 
			{
				TheShopServerProxy().SetShopIn();
				return false;
			}
			else 
			{
				TheShopServerProxy().SetShopOut();
				return false;
			}
		}
#endif //NEW_USER_INTERFACE_SHELL
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
		else if(m_BtnCShop.UpdateMouseEvent() == true)
		{
			// 인게임샵이 열리면 안돼는 상태
			if(g_pInGameShop->IsInGameShopOpen() == false)
				return false;

#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
			// 스크립트 다운로드
			if( g_InGameShopSystem->IsScriptDownload() == true )
			{
				if( g_InGameShopSystem->ScriptDownload() == false )
					return false;
			}
			
			// 배너 다운로드
			if( g_InGameShopSystem->IsBannerDownload() == true )
			{
#ifdef KJH_FIX_INGAMESHOP_INIT_BANNER
				if( g_InGameShopSystem->BannerDownload() == true )
				{
					// 배너 초기화
					g_pInGameShop->InitBanner(g_InGameShopSystem->GetBannerFileName(), g_InGameShopSystem->GetBannerURL());
				}
#else // KJH_FIX_INGAMESHOP_INIT_BANNER
				g_InGameShopSystem->BannerDownload();
#endif // KJH_FIX_INGAMESHOP_INIT_BANNER
			}
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD
			
			if( g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INGAMESHOP) == false)
			{
				// 샵 Open 요청중 상태가 아니면 
				if( g_InGameShopSystem->GetIsRequestShopOpenning() == false )		
				{
					SendRequestIGS_CashShopOpen(0);		// 샵 Open요청
					g_InGameShopSystem->SetIsRequestShopOpenning(true);

#ifdef LDK_ADD_SCALEFORM
					if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
					{
#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
						g_pMainFrame->SetBtnState(MAINFRAME_BTN_PARTCHARGE, true);
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD
					}
#else //LDK_ADD_SCALEFORM
#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
					g_pMainFrame->SetBtnState(MAINFRAME_BTN_PARTCHARGE, true);
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD
#endif //LDK_ADD_SCALEFORM

				}
			}
			else
			{
				SendRequestIGS_CashShopOpen(1);		// 샵 Close요청
				g_pNewUISystem->Hide(SEASON3B::INTERFACE_INGAMESHOP);
			}		

			return true;
		}
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
	}

	return false;
}

bool SEASON3B::CNewUIMainFrameWindow::UpdateKeyEvent()
{
#ifdef LDK_ADD_SCALEFORM
	if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
	{
		if(m_ItemHotKey.UpdateKeyEvent() == false)
		{
			return false;
		}
	}
#else //LDK_ADD_SCALEFORM
	if(m_ItemHotKey.UpdateKeyEvent() == false)
	{
		return false;
	}
#endif //LDK_ADD_SCALEFORM

	return true;
}

bool SEASON3B::CNewUIMainFrameWindow::Update()
{
	if(m_bExpEffect == true)
	{
		if(timeGetTime() - m_dwExpEffectTime > 2000)
		{
			m_bExpEffect = false;
			m_dwExpEffectTime = 0;
			m_dwGetExp = 0;
		}
	}
	
	return true;
}

float SEASON3B::CNewUIMainFrameWindow::GetLayerDepth()
{
	return 10.6f;
}

float SEASON3B::CNewUIMainFrameWindow::GetKeyEventOrder()
{
	return 2.9f;
}

void SEASON3B::CNewUIMainFrameWindow::SetItemHotKey(int iHotKey, int iItemType, int iItemLevel)
{
	m_ItemHotKey.SetHotKey(iHotKey, iItemType, iItemLevel);	
}

int SEASON3B::CNewUIMainFrameWindow::GetItemHotKey(int iHotKey)
{
	return m_ItemHotKey.GetHotKey(iHotKey);
}

int SEASON3B::CNewUIMainFrameWindow::GetItemHotKeyLevel(int iHotKey)
{
	return m_ItemHotKey.GetHotKeyLevel(iHotKey);
}

void SEASON3B::CNewUIMainFrameWindow::UseHotKeyItemRButton()
{
	m_ItemHotKey.UseItemRButton();
}

void SEASON3B::CNewUIMainFrameWindow::UpdateItemHotKey()
{
	m_ItemHotKey.UpdateKeyEvent();
}

void SEASON3B::CNewUIMainFrameWindow::ResetSkillHotKey()
{
	g_pSkillList->Reset();
}

void SEASON3B::CNewUIMainFrameWindow::SetSkillHotKey(int iHotKey, int iSkillType)
{
	g_pSkillList->SetHotKey(iHotKey, iSkillType);
}

int SEASON3B::CNewUIMainFrameWindow::GetSkillHotKey(int iHotKey)
{
	return g_pSkillList->GetHotKey(iHotKey);
}

#ifdef PBG_FIX_SKILLHOTKEY
void SEASON3B::CNewUIMainFrameWindow::SetSkillHotKeyClear(int iHotKey)
{
	g_pSkillList->SetHotKeyClear(iHotKey);
}
#endif //PBG_FIX_SKILLHOTKEY

int SEASON3B::CNewUIMainFrameWindow::GetSkillHotKeyIndex(int iSkillType)
{
	return g_pSkillList->GetSkillIndex(iSkillType);
}

//////////////////////////////////////////////////////////////////////////

SEASON3B::CNewUIItemHotKey::CNewUIItemHotKey()
{
	for(int i=0; i<HOTKEY_COUNT; ++i)
	{
		m_iHotKeyItemType[i] = -1;
		m_iHotKeyItemLevel[i] = 0;
	}
}

SEASON3B::CNewUIItemHotKey::~CNewUIItemHotKey()
{

}

bool SEASON3B::CNewUIItemHotKey::UpdateKeyEvent()
{
	int iIndex = -1;

	if(SEASON3B::IsPress('Q') == true)
	{
		iIndex = GetHotKeyItemIndex(HOTKEY_Q);
	}
	else if(SEASON3B::IsPress('W') == true)
	{
		iIndex = GetHotKeyItemIndex(HOTKEY_W);
	}
	else if(SEASON3B::IsPress('E') == true)
	{
		iIndex = GetHotKeyItemIndex(HOTKEY_E);	
	}
	else if(SEASON3B::IsPress('R') == true)
	{
		iIndex = GetHotKeyItemIndex(HOTKEY_R);
	}
	
	if(iIndex != -1)
	{
		ITEM* pItem = NULL;
		pItem = g_pMyInventory->FindItem(iIndex);
#ifdef PSW_SECRET_ITEM
		if( ( pItem->Type>=ITEM_POTION+78 && pItem->Type<=ITEM_POTION+82 ) )
		{
			list<eBuffState> secretPotionbufflist;
			secretPotionbufflist.push_back( eBuff_SecretPotion1 );
			secretPotionbufflist.push_back( eBuff_SecretPotion2 );
			secretPotionbufflist.push_back( eBuff_SecretPotion3 );
			secretPotionbufflist.push_back( eBuff_SecretPotion4 );
			secretPotionbufflist.push_back( eBuff_SecretPotion5 );
			
			if( g_isCharacterBufflist( (&Hero->Object), secretPotionbufflist ) != eBuffNone ) {
				SEASON3B::CreateOkMessageBox(GlobalText[2530], RGBA(255, 30, 0, 255) );	
			}
			else {
				SendRequestUse(iIndex, 0);
			}
		}
		else
#endif //PSW_SECRET_ITEM			
		{
			SendRequestUse(iIndex, 0);
		}
		return false;
	}

	return true;
}

int SEASON3B::CNewUIItemHotKey::GetHotKeyItemIndex(int iType, bool bItemCount)
{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	int iStartItemType = 0, iEndItemType = 0;
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	int iStartItemType, iEndItemType = 0;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	int i, j;

	switch(iType)
	{
	case HOTKEY_Q:
		if(GetHotKeyCommonItem(iType, iStartItemType, iEndItemType) == false)
		{
			// 마나물약(W키)
			if(m_iHotKeyItemType[iType] >= ITEM_POTION+4 && m_iHotKeyItemType[iType] <= ITEM_POTION+6)
            {
                iStartItemType = ITEM_POTION+6; iEndItemType = ITEM_POTION+4;
            }
			else
			{
				iStartItemType = ITEM_POTION+3; iEndItemType = ITEM_POTION+0;
			}
		}
		break;
	case HOTKEY_W:
		if(GetHotKeyCommonItem(iType, iStartItemType, iEndItemType) == false)
		{
			// 치료물약(Q키)
			if(m_iHotKeyItemType[iType] >= ITEM_POTION+0 && m_iHotKeyItemType[iType] <= ITEM_POTION+3)
            {
                iStartItemType = ITEM_POTION+3; iEndItemType = ITEM_POTION+0;
            }
			else
			{
				iStartItemType = ITEM_POTION+6; iEndItemType = ITEM_POTION+4;
			}	
		}
		break;
	case HOTKEY_E:
		if(GetHotKeyCommonItem(iType, iStartItemType, iEndItemType) == false)
		{
			// 치료물약(Q키)
			if(m_iHotKeyItemType[iType] >= ITEM_POTION+0 && m_iHotKeyItemType[iType] <= ITEM_POTION+3)
            {
                iStartItemType = ITEM_POTION+3; iEndItemType = ITEM_POTION+0;
            }
			// 마나물약(W키)
			else if(m_iHotKeyItemType[iType] >= ITEM_POTION+4 && m_iHotKeyItemType[iType] <= ITEM_POTION+6)
            {
                iStartItemType = ITEM_POTION+6; iEndItemType = ITEM_POTION+4;
            }
			else
			{
				iStartItemType = ITEM_POTION+8; iEndItemType = ITEM_POTION+8;
			}
		}
		break;
	case HOTKEY_R:
		if(GetHotKeyCommonItem(iType, iStartItemType, iEndItemType) == false)
		{
			// 치료물약(Q키)
			if(m_iHotKeyItemType[iType] >= ITEM_POTION+0 && m_iHotKeyItemType[iType] <= ITEM_POTION+3)
            {
                iStartItemType = ITEM_POTION+3; iEndItemType = ITEM_POTION+0;
            }
			// 마나물약(W키)
			else if(m_iHotKeyItemType[iType] >= ITEM_POTION+4 && m_iHotKeyItemType[iType] <= ITEM_POTION+6)
            {
                iStartItemType = ITEM_POTION+6; iEndItemType = ITEM_POTION+4;
            }
			else
			{
				iStartItemType = ITEM_POTION+37; iEndItemType = ITEM_POTION+35;	
			}
		}
		break;	
	}

	int iItemCount = 0;
	ITEM* pItem = NULL;

	int iNumberofItems = g_pMyInventory->GetInventoryCtrl()->GetNumberOfItems();
	for(i=iStartItemType; i>=iEndItemType; --i)
	{
		if(bItemCount)
		{
			for(j=0; j<iNumberofItems; ++j)
			{
				pItem = g_pMyInventory->GetInventoryCtrl()->GetItem(j);
				if(pItem == NULL)
				{
					continue;
				}

				// Type과 Level이 맞거나 물약종류이면
				if( 
					(pItem->Type == i && ((pItem->Level>>3)&15) == m_iHotKeyItemLevel[iType])
				|| (pItem->Type == i && (pItem->Type >= ITEM_POTION+0 && pItem->Type <= ITEM_POTION+3)) 
				)
				{
					if(pItem->Type == ITEM_POTION+9			// 술
						|| pItem->Type == ITEM_POTION+10	// 마을귀환문서
						|| pItem->Type == ITEM_POTION+20	// 사랑의묘약
						)
					{
						iItemCount++;
					}
					else
					{
						iItemCount += pItem->Durability;
					}
				}
			}
		}
		else
		{
			int iIndex = -1;
			// 물약종류이면 레벨 관계없이 검색한다.
			if(i >= ITEM_POTION+0 && i <= ITEM_POTION+3)	
			{
				iIndex = g_pMyInventory->FindItemReverseIndex(i);
			}
			else
			{
				iIndex = g_pMyInventory->FindItemReverseIndex(i, m_iHotKeyItemLevel[iType]);
			}
			
			if (-1 != iIndex)
			{
				pItem = g_pMyInventory->FindItem(iIndex);
				if((pItem->Type != ITEM_POTION+7		// 공성물약이 아니고
					&& pItem->Type != ITEM_POTION+10	// 마을귀환문서가 아니고
					&& pItem->Type != ITEM_POTION+20)	// 사랑의묘약이 아니거나
					|| ((pItem->Level>>3)&15) == m_iHotKeyItemLevel[iType] // 아이템 레벨이 같으면
					)
				{
					return iIndex;
				}
			}
		}
	}

/* 물약 단축키 인벤토리 순서 버그 수정 전 소스
	for(i=iStartItemType; i>=iEndItemType; --i)
	{
		int iNumberofItems = g_pMyInventory->GetInventoryCtrl()->GetNumberOfItems();
		for(j=0; j<iNumberofItems; ++j)
		{
			pItem = g_pMyInventory->GetInventoryCtrl()->GetItem(j);
			if(pItem == NULL)
			{
				continue;
			}

			if(pItem->Type == i
				&& ((pItem->Type != ITEM_POTION+7	// 공성물약이 아니고
					&& pItem->Type != ITEM_POTION+10 // 마을귀환문서가 아니고
					&& pItem->Type != ITEM_POTION+20) // 사랑의묘약이 아니거나
					|| (((pItem->Level>>3)&15) == m_iHotKeyItemLevel[iType])) // 아이템 레벨이 같으면
				)
			{
				// 아이템의 갯수를 얻는 경우
				if(bItemCount == true)
				{
					if(pItem->Type == ITEM_POTION+9			// 술
						|| pItem->Type == ITEM_POTION+10	// 마을귀환문서
						|| pItem->Type == ITEM_POTION+20)	// 사랑의묘약
					{
          				iItemCount++;
					}
					else
					{
           				iItemCount += pItem->Durability;
					}
				}
				else
				{
					int iIndex = pItem->x + (pItem->y * g_pMyInventory->GetInventoryCtrl()->GetNumberOfColumn());
					return iIndex;
				}
			}
		}
	}
*/
	if(bItemCount == true)
	{
		return iItemCount;
	}

	return -1;
}

bool SEASON3B::CNewUIItemHotKey::GetHotKeyCommonItem(IN int iHotKey, OUT int& iStart, OUT int& iEnd)
{
	switch(m_iHotKeyItemType[iHotKey])
	{
	case ITEM_POTION+7:		// 공성물약
	case ITEM_POTION+8:		// 해독물약
	case ITEM_POTION+9:		// 술
	case ITEM_POTION+10:	// 마을귀환문서
	case ITEM_POTION+20:	// 사랑의 묘약
	case ITEM_POTION+46:	// 잭오랜턴의축복
	case ITEM_POTION+47:	// 잭오랜턴의분노
	case ITEM_POTION+48:	// 잭오랜턴의외침
	case ITEM_POTION+49:	// 잭오랜턴의음식
	case ITEM_POTION+50:	// 잭오랜턴의음료
#ifdef PSW_ELITE_ITEM
	case ITEM_POTION+70:    // 부분유료화 엘리트 체력 물약
	case ITEM_POTION+71:    // 부분유료화 엘리트 마나 물약
#endif //PSW_ELITE_ITEM
#ifdef PSW_ELITE_ITEM
	case ITEM_POTION+78:    // 부분유료화 힘의 비약
	case ITEM_POTION+79:    // 부분유료화 민첩의 비약
	case ITEM_POTION+80:    // 부분유료화 체력의 비약
	case ITEM_POTION+81:    // 부분유료화 에너지의 비약
	case ITEM_POTION+82:    // 부분유료화 통솔의
#endif //PSW_ELITE_ITEM
#ifdef PSW_NEW_ELITE_ITEM
	case ITEM_POTION+94:    // 부분유료화 엘리트 중간 체력 물약
#endif //PSW_NEW_ELITE_ITEM	
#ifdef CSK_EVENT_CHERRYBLOSSOM
	case ITEM_POTION+85:	// 벚꽃술
	case ITEM_POTION+86:	// 벚꽃경단
	case ITEM_POTION+87:	// 벚꽃잎
#endif //CSK_EVENT_CHERRYBLOSSOM
#ifdef YDG_ADD_CS7_ELITE_SD_POTION
	case ITEM_POTION+133:	// 엘리트SD회복물약
#endif	// YDG_ADD_CS7_ELITE_SD_POTION
		// 사랑의 묘약이 아니거나 레벨이 0이면
		if(m_iHotKeyItemType[iHotKey] != ITEM_POTION+20 || m_iHotKeyItemLevel[iHotKey] == 0)
		{
			iStart = iEnd = m_iHotKeyItemType[iHotKey];
			return true;
		}
		break;
	default:
		// SD 회복물약
		if(m_iHotKeyItemType[iHotKey] >= ITEM_POTION+35 && m_iHotKeyItemType[iHotKey] <= ITEM_POTION+37)
		{
			iStart = ITEM_POTION+37; iEnd = ITEM_POTION+35;
			return true;
		}
		// 복합물약
		else if(m_iHotKeyItemType[iHotKey] >= ITEM_POTION+38 && m_iHotKeyItemType[iHotKey] <= ITEM_POTION+40)
		{
			iStart = ITEM_POTION+40; iEnd = ITEM_POTION+38;
			return true;
		}
		break;
	}

	return false;
}

int SEASON3B::CNewUIItemHotKey::GetHotKeyItemCount(int iType)
{
	return 0;
}

void SEASON3B::CNewUIItemHotKey::SetHotKey(int iHotKey, int iItemType, int iItemLevel)
{
	if(iHotKey != -1 && CNewUIMyInventory::CanRegisterItemHotKey(iItemType) == true
		)
	{
		m_iHotKeyItemType[iHotKey] = iItemType;
		m_iHotKeyItemLevel[iHotKey] = iItemLevel;
	}
	else
	{
		m_iHotKeyItemType[iHotKey] = -1;
		m_iHotKeyItemLevel[iHotKey] = 0;
	}
}

int SEASON3B::CNewUIItemHotKey::GetHotKey(int iHotKey)
{
	if(iHotKey != -1)
	{
		return m_iHotKeyItemType[iHotKey];
	}

	return -1;
}

int SEASON3B::CNewUIItemHotKey::GetHotKeyLevel(int iHotKey)
{
	if(iHotKey != -1)
	{
		return m_iHotKeyItemLevel[iHotKey];
	}
	
	return 0;
}

void SEASON3B::CNewUIItemHotKey::RenderItems()
{
	float x, y, width, height;

	for(int i=0; i<HOTKEY_COUNT; ++i)
	{
		int iIndex = GetHotKeyItemIndex(i);
		if(iIndex != -1)
		{
			ITEM* pItem = g_pMyInventory->FindItem(iIndex);
			if(pItem)
			{
				x = 10 + (i*38); y = 443; width = 20; height = 20;
				RenderItem3D(x, y, width, height, pItem->Type, pItem->Level, 0, 0);
			}
		}
	}
}

void SEASON3B::CNewUIItemHotKey::RenderItemCount()
{
	float x, y, width, height;

	glColor4f(1.f, 1.f, 1.f, 1.f);

	for(int i=0; i<HOTKEY_COUNT; ++i)
	{
		int iCount = GetHotKeyItemIndex(i, true);
		if(iCount > 0)
		{
			x = 30 + (i*38); y = 457; width = 8; height = 9;
			SEASON3B::RenderNumber(x, y, iCount);
		}
	}
}

void SEASON3B::CNewUIItemHotKey::UseItemRButton()
{
	int x, y, width, height;

	for(int i=0; i<HOTKEY_COUNT; ++i)
	{
		x = 10 + (i * 38); y = 445; width = 20; height = 20;
		if(SEASON3B::CheckMouseIn(x, y, width, height) == true)
		{
			if(MouseRButtonPush)
			{
				MouseRButtonPush = false;
				int iIndex = GetHotKeyItemIndex(i);
				if(iIndex != -1)
				{
					SendRequestUse(iIndex, 0);
					break;
				}
			}
		}
	}	
}

//////////////////////////////////////////////////////////////////////////

SEASON3B::CNewUISkillList::CNewUISkillList()
{
	m_pNewUIMng = NULL;
	Reset();
}

SEASON3B::CNewUISkillList::~CNewUISkillList()
{
	Release();
}

bool SEASON3B::CNewUISkillList::Create(CNewUIManager* pNewUIMng, CNewUI3DRenderMng* pNewUI3DRenderMng)
{
	if(NULL == pNewUIMng)
		return false;
	
	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_SKILL_LIST, this);

	m_pNewUI3DRenderMng = pNewUI3DRenderMng;

	LoadImages();

	Show(true);

	return true;
}

void SEASON3B::CNewUISkillList::Release()
{
	if(m_pNewUI3DRenderMng)
	{
		m_pNewUI3DRenderMng->DeleteUI2DEffectObject(UI2DEffectCallback);
	}
	
	UnloadImages();

	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}
}

void SEASON3B::CNewUISkillList::Reset()
{
	m_bSkillList = false;
	m_bHotKeySkillListUp = false;

	m_bRenderSkillInfo = false;
	m_iRenderSkillInfoType = 0;
	m_iRenderSkillInfoPosX = 0;
	m_iRenderSkillInfoPosY = 0;

	for(int i=0; i<SKILLHOTKEY_COUNT; ++i)
	{
		m_iHotKeySkillType[i] = -1;
	}

	m_EventState = EVENT_NONE;
}

void SEASON3B::CNewUISkillList::LoadImages()
{
	LoadBitmap("Interface\\newui_skill.jpg", IMAGE_SKILL1, GL_LINEAR);
	LoadBitmap("Interface\\newui_skill2.jpg", IMAGE_SKILL2, GL_LINEAR);
	LoadBitmap("Interface\\newui_command.jpg", IMAGE_COMMAND, GL_LINEAR);
	LoadBitmap("Interface\\newui_skillbox.jpg", IMAGE_SKILLBOX, GL_LINEAR);
	LoadBitmap("Interface\\newui_skillbox2.jpg", IMAGE_SKILLBOX_USE, GL_LINEAR);
#ifdef KJH_ADD_SKILLICON_RENEWAL
	LoadBitmap("Interface\\newui_non_skill.jpg", IMAGE_NON_SKILL1, GL_LINEAR);
	LoadBitmap("Interface\\newui_non_skill2.jpg", IMAGE_NON_SKILL2, GL_LINEAR);
	LoadBitmap("Interface\\newui_non_command.jpg", IMAGE_NON_COMMAND, GL_LINEAR);
#endif // KJH_ADD_SKILLICON_RENEWAL
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	LoadBitmap("Interface\\newui_skill3.jpg", IMAGE_SKILL3, GL_LINEAR);
	LoadBitmap("Interface\\newui_non_skill3.jpg", IMAGE_NON_SKILL3, GL_LINEAR);
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
}

void SEASON3B::CNewUISkillList::UnloadImages()
{
	DeleteBitmap(IMAGE_SKILL1);
	DeleteBitmap(IMAGE_SKILL2);
	DeleteBitmap(IMAGE_COMMAND);
	DeleteBitmap(IMAGE_SKILLBOX);
	DeleteBitmap(IMAGE_SKILLBOX_USE);
#ifdef KJH_ADD_SKILLICON_RENEWAL
	DeleteBitmap(IMAGE_NON_SKILL1);
	DeleteBitmap(IMAGE_NON_SKILL2);
	DeleteBitmap(IMAGE_NON_COMMAND);
#endif // KJH_ADD_SKILLICON_RENEWAL
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	DeleteBitmap(IMAGE_SKILL3);
	DeleteBitmap(IMAGE_NON_SKILL3);
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
}

bool SEASON3B::CNewUISkillList::UpdateMouseEvent()
{
#ifdef MOD_SKILLLIST_UPDATEMOUSE_BLOCK
	if(GFxProcess::GetInstancePtr()->GetUISelect() == 1)
	{
		return true;
	}
#endif //MOD_SKILLLIST_UPDATEMOUSE_BLOCK

#ifdef YDG_ADD_NEW_DUEL_WATCH_BUFF
	// 관전 버프시 메인 프레임을 사용할 수 없다
	if(g_isCharacterBuff((&Hero->Object), eBuff_DuelWatch))
	{
		m_bSkillList = false;
		return true;
	}
#endif	// YDG_ADD_NEW_DUEL_WATCH_BUFF
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

	// lock 사용
	BYTE bySkillNumber = CharacterAttribute->SkillNumber;	// 스킬 갯수
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	BYTE bySkillMasterNumber = CharacterAttribute->SkillMasterNumber;	// 마스터 스킬 갯수
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

	float x, y, width, height;

	m_bRenderSkillInfo = false;

	if(bySkillNumber <= 0)
	{
		return true;
	}
	
	// 현재 사용중인 스킬
	x = 385.f; y = 431.f; width = 32.f; height = 38.f;
	if(m_EventState == EVENT_NONE && MouseLButtonPush == false 
		&& SEASON3B::CheckMouseIn(x, y, width, height) == true)
	{
		m_EventState = EVENT_BTN_HOVER_CURRENTSKILL;
		return true;
	}
	if(m_EventState == EVENT_BTN_HOVER_CURRENTSKILL && MouseLButtonPush == false
		&& SEASON3B::CheckMouseIn(x, y, width, height) == false)
	{
		m_EventState = EVENT_NONE;
		return true;
	}
	if(m_EventState == EVENT_BTN_HOVER_CURRENTSKILL && (MouseLButtonPush == true || MouseLButtonDBClick == true)
		&& SEASON3B::CheckMouseIn(x, y, width, height) == true)
	{
		m_EventState = EVENT_BTN_DOWN_CURRENTSKILL;
		return false;
	}
	if(m_EventState == EVENT_BTN_DOWN_CURRENTSKILL)
	{
		if(MouseLButtonPush == false && MouseLButtonDBClick == false)
		{
			if(SEASON3B::CheckMouseIn(x, y, width, height) == true)
			{		
				m_bSkillList = !m_bSkillList;
				PlayBuffer(SOUND_CLICK01);
				m_EventState = EVENT_NONE;
				return false;
			}
			m_EventState = EVENT_NONE;
			return true;
		}
		
	}
	
	if(m_EventState == EVENT_BTN_HOVER_CURRENTSKILL)
	{
		m_bRenderSkillInfo = true;
		m_iRenderSkillInfoType = Hero->CurrentSkill;
		m_iRenderSkillInfoPosX = x - 5;
		m_iRenderSkillInfoPosY = y;
		
		return false;
	}
	else if(m_EventState == EVENT_BTN_DOWN_CURRENTSKILL)
	{
		return false;
	}
	
	// 현재 리스트에 표현된 5개 스킬
	x = 222.f; y =431.f; width = 32.f*5.f; height = 38.f;
	if(m_EventState == EVENT_NONE && MouseLButtonPush == false 
		&& SEASON3B::CheckMouseIn(x, y, width, height) == true)
	{
		m_EventState = EVENT_BTN_HOVER_SKILLHOTKEY;
		return true;
	}
	if(m_EventState == EVENT_BTN_HOVER_SKILLHOTKEY && MouseLButtonPush == false
		&& SEASON3B::CheckMouseIn(x, y, width, height) == false)
	{
		m_EventState = EVENT_NONE;
		return true;
	}
	if(m_EventState == EVENT_BTN_HOVER_SKILLHOTKEY && MouseLButtonPush == true
		&& SEASON3B::CheckMouseIn(x, y, width, height) == true)
	{
		m_EventState = EVENT_BTN_DOWN_SKILLHOTKEY;
		return false;
	}
	
	x = 190.f; y =431.f; width = 32.f; height = 38.f;
	int iStartIndex = (m_bHotKeySkillListUp == true) ? 6 : 1;
	for(int i=0, iIndex = iStartIndex; i<5; ++i, iIndex++)
	{
		x += width;

		if(iIndex == 10)
		{
			iIndex = 0;
		}
		if(SEASON3B::CheckMouseIn(x, y, width, height) == true)
		{
			if(m_iHotKeySkillType[iIndex] == -1)
			{
				if(m_EventState == EVENT_BTN_HOVER_SKILLHOTKEY)
				{
					m_bRenderSkillInfo = false;
					m_iRenderSkillInfoType = -1;
				}
				if(m_EventState == EVENT_BTN_DOWN_SKILLHOTKEY && MouseLButtonPush == false)
				{
					m_EventState = EVENT_NONE;
				}
				continue;
			}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

			// lock 사용
			WORD bySkillType = CharacterAttribute->Skill[m_iHotKeySkillType[iIndex]];
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

			if(bySkillType == 0 || ( bySkillType >= AT_SKILL_STUN && bySkillType <= AT_SKILL_REMOVAL_BUFF ))
				continue;

			BYTE bySkillUseType = SkillAttribute[bySkillType].SkillUseType;

			if(bySkillUseType == SKILL_USE_TYPE_MASTERLEVEL)
			{
				continue;
			}

			if(m_EventState == EVENT_BTN_HOVER_SKILLHOTKEY)
			{
				m_bRenderSkillInfo = true;
				m_iRenderSkillInfoType = m_iHotKeySkillType[iIndex];
				m_iRenderSkillInfoPosX = x - 5;
				m_iRenderSkillInfoPosY = y;
				return true;
			}
			if(m_EventState == EVENT_BTN_DOWN_SKILLHOTKEY)
			{
				if(MouseLButtonPush == false)
				{
					if(m_iRenderSkillInfoType == m_iHotKeySkillType[iIndex])
					{
						m_EventState = EVENT_NONE;
#ifdef CSK_FIX_WOPS_K29010_HELLBUST
#ifdef PBG_ADD_NEWCHAR_MONK
						m_wHeroPriorSkill = CharacterAttribute->Skill[Hero->CurrentSkill];
#else //PBG_ADD_NEWCHAR_MONK
						m_byHeroPriorSkill = CharacterAttribute->Skill[Hero->CurrentSkill];
#endif //PBG_ADD_NEWCHAR_MONK
#endif // CSK_FIX_WOPS_K29010_HELLBUST
						Hero->CurrentSkill = m_iHotKeySkillType[iIndex];
						PlayBuffer(SOUND_CLICK01);
						return false;
					}
					else
					{
						m_EventState = EVENT_NONE;
					}
				}
			}
		}
	}
	
	x = 222.f; y =431.f; width = 32.f*5.f; height = 38.f;
	if(m_EventState == EVENT_BTN_DOWN_SKILLHOTKEY)
	{
		if(MouseLButtonPush == false && SEASON3B::CheckMouseIn(x, y, width, height) == false)
		{
			m_EventState = EVENT_NONE;
			return true;	
		}
		return false;
	}
	
	// 스킬 리스트
	if(m_bSkillList == false)		//// 아래부터는 스킬 리스트가 있을경우에만 처리 ////
		return true;
	
	WORD bySkillType = 0;

	int iSkillCount = 0;
	bool bMouseOnSkillList = false;
	
	x = 385.f; y = 390; width = 32; height = 38;
	float fOrigX = 385.f;
	
	EVENT_STATE PrevEventState = m_EventState;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#ifdef _VS2008PORTING
	for(int i=0; i<MAX_MAGIC; ++i)
#else // _VS2008PORTING
	for(i=0; i<MAX_MAGIC; ++i)
#endif // _VS2008PORTING
	{
		// 공성 스킬 제외하고 스킬 타입이 있으면
		bySkillType = CharacterAttribute->Skill[i];

		if(bySkillType == 0 || ( bySkillType >= AT_SKILL_STUN && bySkillType <= AT_SKILL_REMOVAL_BUFF ))
			continue;

		BYTE bySkillUseType = SkillAttribute[bySkillType].SkillUseType;

		if(bySkillUseType == SKILL_USE_TYPE_MASTERLEVEL)
		{
			continue;
		}


		if(iSkillCount == 18)
		{
			y -= height;
		}
		
		if(iSkillCount < 14)
		{
			int iRemainder = iSkillCount % 2;
			int iQuotient = iSkillCount / 2;
			
			// 짝수계열이면
			if(iRemainder == 0)
			{
				x = fOrigX + iQuotient * width;
			}
			// 홀수계열이면
			else
			{
				x = fOrigX - (iQuotient + 1) * width;
			}
		}
		else if(iSkillCount >= 14 && iSkillCount < 18)
		{
			x = fOrigX - (8 * width) - ((iSkillCount - 14) * width);
		}
		else
		{
			x = fOrigX - (12 * width) + ((iSkillCount - 17) * width);
		}

		iSkillCount++;
		
		if(SEASON3B::CheckMouseIn(x, y, width, height) == true)
		{
			bMouseOnSkillList = true;
			if(m_EventState == EVENT_NONE && MouseLButtonPush == false)
			{
				m_EventState = EVENT_BTN_HOVER_SKILLLIST;
				break;
			}
		}
		
		if(m_EventState == EVENT_BTN_HOVER_SKILLLIST && MouseLButtonPush == true
			&& SEASON3B::CheckMouseIn(x, y, width, height) == true)
		{
			m_EventState = EVENT_BTN_DOWN_SKILLLIST;
			break;
		}
		
		if(m_EventState == EVENT_BTN_HOVER_SKILLLIST && MouseLButtonPush == false
			&& SEASON3B::CheckMouseIn(x, y, width, height) == true)
		{
			m_bRenderSkillInfo = true;
			m_iRenderSkillInfoType = i;
			m_iRenderSkillInfoPosX = x;
			m_iRenderSkillInfoPosY = y;
		}
		
		if(m_EventState == EVENT_BTN_DOWN_SKILLLIST && MouseLButtonPush == false
			&& m_iRenderSkillInfoType == i && SEASON3B::CheckMouseIn(x, y, width, height) == true)
		{
			m_EventState = EVENT_NONE;

#ifdef CSK_FIX_WOPS_K29010_HELLBUST
#ifdef PBG_ADD_NEWCHAR_MONK
			m_wHeroPriorSkill = CharacterAttribute->Skill[Hero->CurrentSkill];
#else //PBG_ADD_NEWCHAR_MONK
			m_byHeroPriorSkill = CharacterAttribute->Skill[Hero->CurrentSkill];
#endif //PBG_ADD_NEWCHAR_MONK
#endif // CSK_FIX_WOPS_K29010_HELLBUST

			Hero->CurrentSkill = i;
			m_bSkillList = false;
	
			PlayBuffer(SOUND_CLICK01);
			return false;
		}
	}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

	if(PrevEventState != m_EventState)
	{
		if(m_EventState == EVENT_NONE || m_EventState == EVENT_BTN_HOVER_SKILLLIST)
			return true;
		return false;
	}

	// 펫 스킬
	if(Hero->m_pPet != NULL)
	{
		x = 353.f; y = 352; width = 32; height = 38;
#ifdef _VS2008PORTING
		for(int i=AT_PET_COMMAND_DEFAULT; i<AT_PET_COMMAND_END; ++i)
#else // _VS2008PORTING
		for(i=AT_PET_COMMAND_DEFAULT; i<AT_PET_COMMAND_END; ++i)
#endif // _VS2008PORTING
		{
			if(SEASON3B::CheckMouseIn(x, y, width, height) == true)
			{
				bMouseOnSkillList = true;
				
				if(m_EventState == EVENT_NONE && MouseLButtonPush == false)
				{
					m_EventState = EVENT_BTN_HOVER_SKILLLIST;
					return true;
				}
				if(m_EventState == EVENT_BTN_HOVER_SKILLLIST && MouseLButtonPush == true)
				{
					m_EventState = EVENT_BTN_DOWN_SKILLLIST;
					return false;
				}
				
				if(m_EventState == EVENT_BTN_HOVER_SKILLLIST)
				{
					m_bRenderSkillInfo = true;
					m_iRenderSkillInfoType = i;
					m_iRenderSkillInfoPosX = x;
					m_iRenderSkillInfoPosY = y;
				}
				if(m_EventState == EVENT_BTN_DOWN_SKILLLIST && MouseLButtonPush == false
					&& m_iRenderSkillInfoType == i)
				{
					m_EventState = EVENT_NONE;

#ifdef CSK_FIX_WOPS_K29010_HELLBUST
#ifdef PBG_ADD_NEWCHAR_MONK
					m_wHeroPriorSkill = CharacterAttribute->Skill[Hero->CurrentSkill];
#else //PBG_ADD_NEWCHAR_MONK
					m_byHeroPriorSkill = CharacterAttribute->Skill[Hero->CurrentSkill];
#endif //PBG_ADD_NEWCHAR_MONK
#endif // CSK_FIX_WOPS_K29010_HELLBUST

					Hero->CurrentSkill = i;
					m_bSkillList = false;
					PlayBuffer(SOUND_CLICK01);
					return false;
				}
			}
			x += width;	
		}
	}

	if(bMouseOnSkillList == false && m_EventState == EVENT_BTN_HOVER_SKILLLIST)
	{
		m_EventState = EVENT_NONE;
		return true;
	}
	if(bMouseOnSkillList == false && MouseLButtonPush == false
		&& m_EventState == EVENT_BTN_DOWN_SKILLLIST )
	{
		m_EventState = EVENT_NONE;
		return false;
	}
	if(m_EventState == EVENT_BTN_DOWN_SKILLLIST)
	{
		if(MouseLButtonPush == false)
		{
			m_EventState = EVENT_NONE;
			return true;
		}
		return false;
	}
	
	return true;
}

bool SEASON3B::CNewUISkillList::UpdateKeyEvent()
{
#ifdef _VS2008PORTING
	for(int i=0; i<9; ++i)
#else // _VS2008PORTING
	int i;
	for(i=0; i<9; ++i)
#endif // _VS2008PORTING
	{
		if(SEASON3B::IsPress('1' + i))
		{
			UseHotKey(i+1);
		}
	}

	if(SEASON3B::IsPress('0'))
	{
		UseHotKey(0);
	}

	if(m_EventState == EVENT_BTN_HOVER_SKILLLIST)
	{
		if(SEASON3B::IsRepeat(VK_CONTROL))
		{
#ifdef _VS2008PORTING
			for(int i=0; i<9; ++i)
#else // _VS2008PORTING
			for(i=0; i<9; ++i)
#endif // _VS2008PORTING
			{
				if(SEASON3B::IsPress('1' + i))
				{
					SetHotKey(i+1, m_iRenderSkillInfoType);

					return false;
				}
			}

			if(SEASON3B::IsPress('0'))
			{
				SetHotKey(0, m_iRenderSkillInfoType);

				return false;
			}
		}
	}

	if(SEASON3B::IsRepeat(VK_SHIFT))
	{
#ifdef _VS2008PORTING
		for(int i=0; i<4; ++i)
#else // _VS2008PORTING
		for(i=0; i<4; ++i)
#endif // _VS2008PORTING
		{
			if(SEASON3B::IsPress('1' + i))
			{
				Hero->CurrentSkill = AT_PET_COMMAND_DEFAULT + i;
				return false;
			}
		}
	}
		
	return true;
}

bool SEASON3B::CNewUISkillList::IsArrayUp(BYTE bySkill)
{
	for(int i=0; i<SKILLHOTKEY_COUNT; ++i)
	{
		if(m_iHotKeySkillType[i] == bySkill)
		{
			if(i == 0 || i > 5)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}

	return false;
}

bool SEASON3B::CNewUISkillList::IsArrayIn(BYTE bySkill)
{
	for(int i=0; i<SKILLHOTKEY_COUNT; ++i)
	{
		if(m_iHotKeySkillType[i] == bySkill)
		{
			return true;
		}
	}

	return false;
}

void SEASON3B::CNewUISkillList::SetHotKey(int iHotKey, int iSkillType)
{
	for(int i=0; i<SKILLHOTKEY_COUNT; ++i)
	{
		if(m_iHotKeySkillType[i] == iSkillType)
		{
			m_iHotKeySkillType[i] = -1;
			break;
		}
	}	

	m_iHotKeySkillType[iHotKey] = iSkillType;
}
#ifdef PBG_FIX_SKILLHOTKEY
void SEASON3B::CNewUISkillList::SetHotKeyClear(int iHotKey)
{
	for(int i=0; i<SKILLHOTKEY_COUNT; ++i)
	{
		if(m_iHotKeySkillType[i] == iHotKey)
		{
			m_iHotKeySkillType[i] = -1;
			break;
		}
	}	
}
#endif //PBG_FIX_SKILLHOTKEY
int SEASON3B::CNewUISkillList::GetHotKey(int iHotKey)
{
	return m_iHotKeySkillType[iHotKey];
}

int SEASON3B::CNewUISkillList::GetSkillIndex(int iSkillType)
{
	int iReturn = -1;
	for(int i=0; i<MAX_MAGIC; ++i)
	{
		if(CharacterAttribute->Skill[i] == iSkillType)
		{
			iReturn = i;
			break;
		}
	}

	return iReturn;
}

void SEASON3B::CNewUISkillList::UseHotKey(int iHotKey)
{
	if(m_iHotKeySkillType[iHotKey] != -1)
	{
		// 펫 스킬 예외처리
		if(m_iHotKeySkillType[iHotKey] >= AT_PET_COMMAND_DEFAULT && m_iHotKeySkillType[iHotKey] < AT_PET_COMMAND_END)
		{
			if(Hero->m_pPet == NULL)
			{
				return;
			}
		}

		//  순간이동 법서를 선택하면은 자동 공격이 정지된다.

		WORD wHotKeySkill = CharacterAttribute->Skill[m_iHotKeySkillType[iHotKey]];

		if(wHotKeySkill == 0)
		{
			return;
		}

#ifdef CSK_FIX_WOPS_K29010_HELLBUST
#ifdef PBG_ADD_NEWCHAR_MONK
		m_wHeroPriorSkill = CharacterAttribute->Skill[Hero->CurrentSkill];
#else //PBG_ADD_NEWCHAR_MONK
		m_byHeroPriorSkill = CharacterAttribute->Skill[Hero->CurrentSkill];
#endif //PBG_ADD_NEWCHAR_MONK
#endif // CSK_FIX_WOPS_K29010_HELLBUST
		Hero->CurrentSkill = m_iHotKeySkillType[iHotKey];

		WORD bySkill = CharacterAttribute->Skill[Hero->CurrentSkill];


        if ( 
			g_pOption->IsAutoAttack() == true
			&& World != WD_6STADIUM 
			&& InChaosCastle()==false 
			&& ( bySkill == AT_SKILL_TELEPORT || bySkill == AT_SKILL_TELEPORT_B ) )
        {
            SelectedCharacter = -1;
            Attacking = -1;
        }
	}
}

bool SEASON3B::CNewUISkillList::Update()
{
	// 현재 사용 중인 스킬 인덱스가 들어가 있는 스킬 아이콘 5개 렌더링
	if(IsArrayIn(Hero->CurrentSkill) == true)
	{
		if(IsArrayUp(Hero->CurrentSkill) == true)
		{
			m_bHotKeySkillListUp = true;
		}
		else
		{
			m_bHotKeySkillListUp = false;
		}
	}

	// 펫이 없는데 현재 스킬이 펫 스킬일 경우 예외 처리
	if(Hero->m_pPet == NULL)
	{
		if(Hero->CurrentSkill >= AT_PET_COMMAND_DEFAULT && Hero->CurrentSkill < AT_PET_COMMAND_END)
		{
			Hero->CurrentSkill = 0;
		}
	}

	return true;
}

void SEASON3B::CNewUISkillList::RenderCurrentSkillAndHotSkillList()
{
	int i;
	float x, y, width, height;
	
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// lock 사용
	BYTE bySkillNumber = CharacterAttribute->SkillNumber;
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	
	// 스킬 갯수가 1개 이상이면
	if(bySkillNumber > 0)
	{
		int iStartSkillIndex = 1;
		if(m_bHotKeySkillListUp)
		{
			iStartSkillIndex = 6;
		}
		
		// 현재 사용중인 핫키 스킬 5개 렌더링
		x = 190; y =431; width = 32; height = 38;
		for(i=0; i<5; ++i)
		{
			x += width;
			
			int iIndex = iStartSkillIndex + i;
			if(iIndex == 10)
			{
				iIndex = 0;
			}
			
			// 비워있는거 예외 처리
			if(m_iHotKeySkillType[iIndex] == -1)
			{
				continue;
			}
			
			// 펫 스킬 예외 처리
			if(m_iHotKeySkillType[iIndex] >= AT_PET_COMMAND_DEFAULT && m_iHotKeySkillType[iIndex] < AT_PET_COMMAND_END)
			{
				if(Hero->m_pPet == NULL)
				{
					continue;
				}
			}
			
#ifdef KJH_ADD_SKILLICON_RENEWAL
			if(Hero->CurrentSkill == m_iHotKeySkillType[iIndex])
			{
				SEASON3B::RenderImage(IMAGE_SKILLBOX_USE, x, y, width, height);
			}
			
			RenderSkillIcon(m_iHotKeySkillType[iIndex], x+6, y+6, 20, 28);
#else // KJH_ADD_SKILLICON_RENEWAL
			if(Hero->CurrentSkill == m_iHotKeySkillType[iIndex])
			{
				glColor3f(1.f, 0.9f, 0.8f);
				SEASON3B::RenderImage(IMAGE_SKILLBOX_USE, x, y, width, height);
			}
			else
			{
				glColor3f(0.6f,0.7f,0.8f);
			}
			
			RenderSkillIcon(m_iHotKeySkillType[iIndex], x+6, y+6, 20, 28);
			if(m_EventState == EVENT_BTN_DOWN_SKILLHOTKEY
				&& m_iRenderSkillInfoType == m_iHotKeySkillType[iIndex])
			{
				EnableAlphaBlend();
				glColor4f(0.15f, 0.15f, 0.15f, 0.5f);
				RenderColor(x+6, y+6, 20, 28);
				DisableAlphaBlend();
				EnableAlphaTest();
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			}
#endif // KJH_ADD_SKILLICON_RENEWAL
		}
		
		// 현재 사용 중인 스킬 아이콘 렌더링
		x = 392; y = 437; width = 20; height = 28;
		
#ifdef KJH_ADD_SKILLICON_RENEWAL
		RenderSkillIcon(Hero->CurrentSkill, x, y, width, height);
#else // KJH_ADD_SKILLICON_RENEWAL
		glColor3f(1.f, 0.9f, 0.8f);
		RenderSkillIcon(Hero->CurrentSkill, x, y, width, height);
		
		if(m_EventState == EVENT_BTN_DOWN_CURRENTSKILL)
		{
			EnableAlphaBlend();
			glColor4f(0.15f, 0.15f, 0.15f, 0.5f);
			RenderColor(x, y, width, height);
			DisableAlphaBlend();
			EnableAlphaTest();
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		}
#endif // KJH_ADD_SKILLICON_RENEWAL
	}
}

bool SEASON3B::CNewUISkillList::Render()
{
	int i;
	float x, y, width, height;
	
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// lock 사용
	BYTE bySkillNumber = CharacterAttribute->SkillNumber;
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	
	// 스킬 갯수가 1개 이상이면
	if(bySkillNumber > 0)
	{
		// 스킬 리스트 렌더링
		if(m_bSkillList == true)
		{
			x = 385; y = 390; width = 32; height = 38;
			float fOrigX = 385.f;
            int iSkillType  = 0;
			int iSkillCount = 0;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

			// lock 사용
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
            int iSkillNumber = min(CharacterAttribute->SkillNumber - CharacterAttribute->SkillMasterNumber, 36);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX

			for(i=0; i<MAX_MAGIC; ++i)
			{
				// lock 사용
                iSkillType = CharacterAttribute->Skill[i];
				
				if(iSkillType != 0 && (iSkillType < AT_SKILL_STUN || iSkillType > AT_SKILL_REMOVAL_BUFF))
				{
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

					// lock 사용
                    BYTE bySkillUseType = SkillAttribute[iSkillType].SkillUseType;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
                    
                    if(bySkillUseType == SKILL_USE_TYPE_MASTER || bySkillUseType == SKILL_USE_TYPE_MASTERLEVEL)
					{
                        continue;
					}

					if(iSkillCount == 18)
					{
						y -= height;
					}

					if(iSkillCount < 14)
					{
						int iRemainder = iSkillCount % 2;
						int iQuotient = iSkillCount / 2;

						// 짝수계열이면
						if(iRemainder == 0)
						{
							x = fOrigX + iQuotient * width;
						}
						// 홀수계열이면
						else
						{
							x = fOrigX - (iQuotient + 1) * width;
						}
					}
					else if(iSkillCount >= 14 && iSkillCount < 18)
					{
						x = fOrigX - (8 * width) - ((iSkillCount - 14) * width);
					}
					else
					{
						x = fOrigX - (12 * width) + ((iSkillCount - 17) * width);
					}

					iSkillCount++;

#ifdef KJH_ADD_SKILLICON_RENEWAL
					if(i == Hero->CurrentSkill)
					{
						SEASON3B::RenderImage(IMAGE_SKILLBOX_USE, x, y, width, height);
					}
					else
					{
						SEASON3B::RenderImage(IMAGE_SKILLBOX, x, y, width, height);	
					}
					
					RenderSkillIcon(i, x+6, y+6, 20, 28);
#else // KJH_ADD_SKILLICON_RENEWAL
					if(i == Hero->CurrentSkill)
					{
						glColor3f(1.f, 0.9f, 0.8f);
						SEASON3B::RenderImage(IMAGE_SKILLBOX_USE, x, y, width, height);
					}
					else
					{
						glColor3f(1.f, 0.9f, 0.8f);
						SEASON3B::RenderImage(IMAGE_SKILLBOX, x, y, width, height);	
						glColor3f(0.6f,0.7f,0.8f);
					}

					RenderSkillIcon(i, x+6, y+6, 20, 28);
					if(m_EventState == EVENT_BTN_DOWN_SKILLLIST
						&& m_iRenderSkillInfoType == i)
					{
						EnableAlphaBlend();
						glColor4f(0.15f, 0.15f, 0.15f, 0.5f);
						RenderColor(x+6, y+6, 20, 28);
						DisableAlphaBlend();
						EnableAlphaTest();
						glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
					}
#endif // KJH_ADD_SKILLICON_RENEWAL
				}
			}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

			RenderPetSkill();
		}
	}
	
	// 스킬 정보 렌더링
	if(m_bRenderSkillInfo == true && m_pNewUI3DRenderMng)
	{
		m_pNewUI3DRenderMng->RenderUI2DEffect(INVENTORY_CAMERA_Z_ORDER, UI2DEffectCallback, this, 0, 0);

		m_bRenderSkillInfo = false;
	}

	return true;
}

void SEASON3B::CNewUISkillList::RenderSkillInfo()
{
	::RenderSkillInfo(m_iRenderSkillInfoPosX+15, m_iRenderSkillInfoPosY-10, m_iRenderSkillInfoType);
}

float SEASON3B::CNewUISkillList::GetLayerDepth()
{
	return 5.2f;
}

#ifdef CSK_FIX_WOPS_K29010_HELLBUST
#ifdef PBG_ADD_NEWCHAR_MONK
WORD SEASON3B::CNewUISkillList::GetHeroPriorSkill()
{
	return m_wHeroPriorSkill;
}
#else //PBG_ADD_NEWCHAR_MONK
BYTE SEASON3B::CNewUISkillList::GetHeroPriorSkill()
{
	return m_byHeroPriorSkill;
}
#endif //PBG_ADD_NEWCHAR_MONK

void SEASON3B::CNewUISkillList::SetHeroPriorSkill(BYTE bySkill)
{
#ifdef PBG_ADD_NEWCHAR_MONK
	m_wHeroPriorSkill = bySkill;
#else //PBG_ADD_NEWCHAR_MONK
	m_byHeroPriorSkill = bySkill;
#endif //PBG_ADD_NEWCHAR_MONK
}
#endif // CSK_FIX_WOPS_K29010_HELLBUST



void SEASON3B::CNewUISkillList::RenderPetSkill()
{
	// 펫이 존재하지 않는다면
	if(Hero->m_pPet == NULL)
	{
		return;
	}

	float x, y, width, height;
	
	x = 353.f; y = 352; width = 32; height = 38;
	for(int i=AT_PET_COMMAND_DEFAULT; i<AT_PET_COMMAND_END; ++i)
	{
#ifdef KJH_ADD_SKILLICON_RENEWAL
		if(i == Hero->CurrentSkill)
		{
			SEASON3B::RenderImage(IMAGE_SKILLBOX_USE, x, y, width, height);
		}
		else
		{
			SEASON3B::RenderImage(IMAGE_SKILLBOX, x, y, width, height);	
		}
		
        RenderSkillIcon(i, x+6, y+6, 20, 28);
#else // KJH_ADD_SKILLICON_RENEWAL
		if(i == Hero->CurrentSkill)
		{
			glColor3f(1.f, 0.9f, 0.8f);
			SEASON3B::RenderImage(IMAGE_SKILLBOX_USE, x, y, width, height);
		}
		else
		{
			glColor3f(1.f, 0.9f, 0.8f);
			SEASON3B::RenderImage(IMAGE_SKILLBOX, x, y, width, height);	
			glColor3f(0.6f,0.7f,0.8f);
		}

        RenderSkillIcon(i, x+6, y+6, 20, 28);
		if(m_EventState == EVENT_BTN_DOWN_SKILLLIST
			&& m_iRenderSkillInfoType == i)
		{
			EnableAlphaBlend();
			glColor4f(0.15f, 0.15f, 0.15f, 0.5f);
			RenderColor(x+6, y+6, 20, 28);
			DisableAlphaBlend();
			EnableAlphaTest();
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		}
#endif // KJH_ADD_SKILLICON_RENEWAL

		x += width;
	}
}

void SEASON3B::CNewUISkillList::RenderSkillIcon(int iIndex, float x, float y, float width, float height)
{
	WORD bySkillType = CharacterAttribute->Skill[iIndex];

	if(bySkillType == 0)
	{
		return;
	}

	if(iIndex >= AT_PET_COMMAND_DEFAULT)    //  팻 명령.
    {
        bySkillType = iIndex;
    }

#ifdef KJH_ADD_SKILLICON_RENEWAL
	//////////////////////////////////////////////////////////////////////////
	// 사용할수 없는 스킬일때 처리하는 부분 - 사용할수 없는 스킬 : true
	bool bCantSkill = false;
#endif // KJH_ADD_SKILLICON_RENEWAL

	//////////////////////////////////////////////////////////////////////////
	// 스킬 아이콘 빨갛게 처리하는 부분 - 스킬 리뉴얼 작업으로 이미지를 바꾸는 형태로 변경

    BYTE bySkillUseType = SkillAttribute[bySkillType].SkillUseType;
	int Skill_Icon = SkillAttribute[bySkillType].Magic_Icon;

#ifdef PBG_FIX_SKILL_DEMENDCONDITION
	//스킬의 스텟요구량 검사후 미달되는것은 빨갛게 처리한다.
 	if( !SKILLCONDITION::DemendConditionCheckSkill( bySkillType ) )
 	{
 		//스킬의 스텟요구량을 만족안한다면.
#ifdef KJH_ADD_SKILLICON_RENEWAL
		bCantSkill = true;
#else // KJH_ADD_SKILLICON_RENEWAL
		glColor3f(1.f, 0.5f, 0.5f);
#endif // KJH_ADD_SKILLICON_RENEWAL
	}
#endif //PBG_FIX_SKILL_DEMENDCONDITION


	// 공성 스킬 검사
	if(IsCanBCSkill(bySkillType) == false)
	{
#ifdef KJH_ADD_SKILLICON_RENEWAL
		bCantSkill = true;
#else // KJH_ADD_SKILLICON_RENEWAL
		glColor3f(1.f, 0.5f, 0.5f);
#endif // KJH_ADD_SKILLICON_RENEWAL
	}
	if( g_isCharacterBuff((&Hero->Object), eBuff_AddSkill) && bySkillUseType == SKILL_USE_TYPE_BRAND )
	{
#ifdef KJH_ADD_SKILLICON_RENEWAL
		bCantSkill = true;
#else // KJH_ADD_SKILLICON_RENEWAL
		glColor3f(1.f, 0.5f, 0.5f);
#endif // KJH_ADD_SKILLICON_RENEWAL
	}

	if(bySkillType == AT_SKILL_SPEAR 
		&& ( Hero->Helper.Type<MODEL_HELPER+2 || Hero->Helper.Type>MODEL_HELPER+3 ) 
		&& Hero->Helper.Type != MODEL_HELPER+37)
	{
#ifdef KJH_ADD_SKILLICON_RENEWAL
		bCantSkill = true;
#else // KJH_ADD_SKILLICON_RENEWAL
		glColor3f(1.f, 0.5f, 0.5f);
#endif // KJH_ADD_SKILLICON_RENEWAL
	}

	if(bySkillType == AT_SKILL_SPEAR 
		&& (Hero->Helper.Type == MODEL_HELPER+2 || Hero->Helper.Type == MODEL_HELPER+3 
		|| Hero->Helper.Type == MODEL_HELPER+37))
	{
		int iTypeL = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;
		int iTypeR = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;
		if((iTypeL < ITEM_SPEAR || iTypeL >= ITEM_BOW)
			&& (iTypeR < ITEM_SPEAR || iTypeR >= ITEM_BOW))
		{
#ifdef KJH_ADD_SKILLICON_RENEWAL
			bCantSkill = true;
#else // KJH_ADD_SKILLICON_RENEWAL
			glColor3f(1.f, 0.5f, 0.5f);
#endif // KJH_ADD_SKILLICON_RENEWAL
		}
	}

	if(bySkillType >= AT_SKILL_BLOCKING && bySkillType <= AT_SKILL_SWORD5 
		&& (Hero->Helper.Type == MODEL_HELPER+2 || Hero->Helper.Type == MODEL_HELPER+3 || Hero->Helper.Type == MODEL_HELPER+37))
	{
#ifdef KJH_ADD_SKILLICON_RENEWAL
		bCantSkill = true;
#else // KJH_ADD_SKILLICON_RENEWAL
		glColor3f(1.f, 0.5f, 0.5f);
#endif // KJH_ADD_SKILLICON_RENEWAL
	}

	if((bySkillType == AT_SKILL_ICE_BLADE
#ifdef PJH_SEASON4_MASTER_RANK4
		||(AT_SKILL_POWER_SLASH_UP <= bySkillType && AT_SKILL_POWER_SLASH_UP+4 >= bySkillType)
#endif //PJH_SEASON4_MASTER_RANK4
		)
		&& (Hero->Helper.Type == MODEL_HELPER+2 || Hero->Helper.Type == MODEL_HELPER+3 || Hero->Helper.Type == MODEL_HELPER+37))
	{
#ifdef KJH_ADD_SKILLICON_RENEWAL
		bCantSkill = true;
#else // KJH_ADD_SKILLICON_RENEWAL
		glColor3f(1.f, 0.5f, 0.5f);
#endif // KJH_ADD_SKILLICON_RENEWAL
	}

#ifdef PJH_SEASON4_MASTER_RANK4
	/*
	else
	if(bySkillType == AT_SKILL_ICE_BLADE ||(AT_SKILL_POWER_SLASH_UP <= bySkillType && AT_SKILL_POWER_SLASH_UP+4 >= bySkillType))
	{
		glColor3f(1.f, 0.5f, 0.5f);
		for(int j=0;j<2;j++)
		{
			if(Hero->Weapon[j].Type==MODEL_SWORD+21||Hero->Weapon[j].Type==MODEL_SWORD+23||Hero->Weapon[j].Type==MODEL_SWORD+28||
				Hero->Weapon[j].Type==MODEL_SWORD+25||Hero->Weapon[j].Type==MODEL_SWORD+31
				)	//21 = 데스블레이드,23 = 익스플로전블레이드,25 = 소드댄서,28 = 룬바스타드,31 = 데쓰브로드
			{
				glColor3f(1.f, 1.f, 1.f);
				break;
			}
		}
	}
	*/
#endif //PJH_SEASON4_MASTER_RANK4   
                     
    int iEnergy = CharacterAttribute->Energy+CharacterAttribute->AddEnergy;

	// 스킬사용시 스텟검사 (에너지만)
    if(g_csItemOption.IsDisableSkill(bySkillType, iEnergy))
	{
#ifdef KJH_ADD_SKILLICON_RENEWAL
		bCantSkill = true;
#else // KJH_ADD_SKILLICON_RENEWAL
        glColor3f(1.f, 0.5f, 0.5f);
#endif // KJH_ADD_SKILLICON_RENEWAL
	}

    if(bySkillType == AT_SKILL_PARTY_TELEPORT && PartyNumber <= 0)
    {
#ifdef KJH_ADD_SKILLICON_RENEWAL
		bCantSkill = true;
#else // KJH_ADD_SKILLICON_RENEWAL
        glColor3f(1.f, 0.5f, 0.5f);
#endif // KJH_ADD_SKILLICON_RENEWAL
    }

#ifdef YDG_ADD_DOPPELGANGER_UI
	if (bySkillType == AT_SKILL_PARTY_TELEPORT && (IsDoppelGanger1() || IsDoppelGanger2() || IsDoppelGanger3() || IsDoppelGanger4()))
	{
#ifdef KJH_ADD_SKILLICON_RENEWAL
		bCantSkill = true;
#else // KJH_ADD_SKILLICON_RENEWAL
        glColor3f(1.f, 0.5f, 0.5f);
#endif // KJH_ADD_SKILLICON_RENEWAL
	}
#endif	// YDG_ADD_DOPPELGANGER_UI

	if(bySkillType == AT_SKILL_DARK_HORSE || (AT_SKILL_ASHAKE_UP <= bySkillType && bySkillType <= AT_SKILL_ASHAKE_UP+4))
	{
		BYTE byDarkHorseLife = 0;
		byDarkHorseLife = CharacterMachine->Equipment[EQUIPMENT_HELPER].Durability;
		if(byDarkHorseLife == 0 || Hero->Helper.Type != MODEL_HELPER+4)		// 다크호스의 HP가 0 이거나, 다크호스가 없을때
		{
#ifdef KJH_ADD_SKILLICON_RENEWAL
			bCantSkill = true;
#else // KJH_ADD_SKILLICON_RENEWAL
			if(Hero->Helper.Type != MODEL_HELPER+4)
				glColor3f(0.5f, 0.5f, 0.5f);
			else
				glColor3f(1.f, 0.5f, 0.5f);
#endif // KJH_ADD_SKILLICON_RENEWAL
		}
	}
#ifdef PJH_FIX_SPRIT
/*박종훈*/
	if( bySkillType>=AT_PET_COMMAND_DEFAULT && bySkillType<AT_PET_COMMAND_END )
	{
		int iCharisma = CharacterAttribute->Charisma+CharacterAttribute->AddCharisma;	// 마이너스 열매 작업
		PET_INFO PetInfo;
		giPetManager::GetPetInfo(PetInfo, 421-PET_TYPE_DARK_SPIRIT);
		int RequireCharisma = (185+(PetInfo.m_wLevel*15));
		if( RequireCharisma > iCharisma ) 
#ifdef KJH_ADD_SKILLICON_RENEWAL
		{
			bCantSkill = true;
		}
#else // KJH_ADD_SKILLICON_RENEWAL
			glColor3f(1.f, 0.5f, 0.5f);
#endif // KJH_ADD_SKILLICON_RENEWAL
	}
#endif //PJH_FIX_SPRIT
	if( (bySkillType == AT_SKILL_INFINITY_ARROW)
#ifdef KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
		|| (bySkillType == AT_SKILL_SWELL_OF_MAGICPOWER)
#endif // KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
		)
	{
#ifdef PJH_FIX_SKILL
		if(g_csItemOption.IsDisableSkill(bySkillType, iEnergy))
		{
#ifdef KJH_ADD_SKILLICON_RENEWAL
			bCantSkill = true;
#else // KJH_ADD_SKILLICON_RENEWAL
			glColor3f(1.f, 0.5f, 0.5f);
#endif // KJH_ADD_SKILLICON_RENEWAL
		}
#endif //PJH_FIX_SKILL
		if( ( g_isCharacterBuff((&Hero->Object), eBuff_InfinityArrow) )
#ifdef KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
			|| ( g_isCharacterBuff((&Hero->Object), eBuff_SwellOfMagicPower) )
#endif // KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
			)
		{
#ifdef KJH_ADD_SKILLICON_RENEWAL
			bCantSkill = true;
#else // KJH_ADD_SKILLICON_RENEWAL
			glColor3f(1.f, 0.5f, 0.5f);
#endif // KJH_ADD_SKILLICON_RENEWAL
		}
	}
#ifdef KJH_FIX_WOPS_K20674_CHECK_STAT_USE_SKILL
	// 블러드어택 (마검사) 일때 스텟을 검사하여 요구스텟이 충분치 아니하면 스킬아이콘 빨갛게 처리
	// 다른스킬도 이와같이 처리 해주어야 한다. (시즌4 본섭 적용 후에 꼭하자!!)
	if( bySkillType == AT_SKILL_REDUCEDEFENSE
#ifdef YDG_FIX_BLOCK_STAFF_WHEEL
		|| (AT_SKILL_BLOOD_ATT_UP <= bySkillType && bySkillType <= AT_SKILL_BLOOD_ATT_UP+4)
#endif	// YDG_FIX_BLOCK_STAFF_WHEEL
		)
	{
		WORD Strength;
		const WORD wRequireStrength = 596;
		Strength = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
		if(Strength < wRequireStrength)
		{
#ifdef KJH_ADD_SKILLICON_RENEWAL
			bCantSkill = true;
#else // KJH_ADD_SKILLICON_RENEWAL
			glColor3f(1.f, 0.5f, 0.5f);
#endif // KJH_ADD_SKILLICON_RENEWAL
		}
#ifdef YDG_FIX_STAFF_FLAMESTRIKE_IN_CHAOSCASLE
		int iTypeL = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;
		int iTypeR = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;
		
		if ( !( iTypeR!=-1 && 
		( iTypeR<ITEM_STAFF || iTypeR>=ITEM_STAFF+MAX_ITEM_INDEX ) &&
		( iTypeL<ITEM_STAFF || iTypeL>=ITEM_STAFF+MAX_ITEM_INDEX ) ) )
		{
#ifdef KJH_ADD_SKILLICON_RENEWAL
			bCantSkill = true;
#else // KJH_ADD_SKILLICON_RENEWAL
			glColor3f(1.f, 0.5f, 0.5f);
#endif // KJH_ADD_SKILLICON_RENEWAL
		}
#endif	// YDG_FIX_STAFF_FLAMESTRIKE_IN_CHAOSCASLE
	}
#endif //KJH_FIX_WOPS_K20674_CHECK_STAT_USE_SKILL
	
#ifdef LDK_FIX_CHECK_STAT_USE_SKILL_PIERCING
	// 아이스에로우 (요정) 일때 스텟을 검사하여 요구스텟이 충분치 아니하면 스킬아이콘 빨갛게 처리
	// 다른스킬도 이와같이 처리 해주어야 한다. (시즌4 본섭 적용 후에 꼭하자!!)
	switch( bySkillType )
	{
	//case AT_SKILL_PIERCING:
	case AT_SKILL_PARALYZE:
		{
			WORD  Dexterity;
			const WORD wRequireDexterity = 646;
			Dexterity = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
			if(Dexterity < wRequireDexterity)
			{
#ifdef KJH_ADD_SKILLICON_RENEWAL
				bCantSkill = true;
#else // KJH_ADD_SKILLICON_RENEWAL
				glColor3f(1.f, 0.5f, 0.5f);
#endif // KJH_ADD_SKILLICON_RENEWAL
			}
		}break;
	}
#endif //LDK_FIX_CHECK_STAT_USE_SKILL_PIERCING

#ifdef YDG_FIX_BLOCK_STAFF_WHEEL
	if( bySkillType == AT_SKILL_WHEEL
		|| (AT_SKILL_TORNADO_SWORDA_UP <= bySkillType && bySkillType <= AT_SKILL_TORNADO_SWORDA_UP+4)
		|| (AT_SKILL_TORNADO_SWORDB_UP <= bySkillType && bySkillType <= AT_SKILL_TORNADO_SWORDB_UP+4)
		)
	{
		int iTypeL = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;
		int iTypeR = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;
		
		if ( !( iTypeR!=-1 && ( iTypeR<ITEM_STAFF || iTypeR>=ITEM_STAFF+MAX_ITEM_INDEX ) && ( iTypeL<ITEM_STAFF || iTypeL>=ITEM_STAFF+MAX_ITEM_INDEX ) ) )
#ifdef KJH_ADD_SKILLICON_RENEWAL
		{
			bCantSkill = true;
		}
#else // KJH_ADD_SKILLICON_RENEWAL
			glColor3f ( 1.f, 0.5f, 0.5f );
#endif // KJH_ADD_SKILLICON_RENEWAL
	}
#endif	// YDG_FIX_BLOCK_STAFF_WHEEL

	if(InChaosCastle() == true)
	{
		//카오스 캐슬에서는 다크스피릿, 다크호스, 디노란트 스킬 등이 사용 불가능
		if( bySkillType == AT_SKILL_DARK_HORSE || bySkillType == AT_SKILL_RIDER
			|| (bySkillType >= AT_PET_COMMAND_DEFAULT && bySkillType <= AT_PET_COMMAND_TARGET)
			||(AT_SKILL_ASHAKE_UP <= bySkillType && bySkillType <= AT_SKILL_ASHAKE_UP+4))
		{
#ifdef KJH_ADD_SKILLICON_RENEWAL
			bCantSkill = true;
#else // KJH_ADD_SKILLICON_RENEWAL
			glColor3f(1.f, 0.5f, 0.5f);
#endif // KJH_ADD_SKILLICON_RENEWAL
		}
	}
	else
	{
		//카오스 캐슬이 아니더라도 죽었으면 스킬 사용 불가능
		if(bySkillType == AT_SKILL_DARK_HORSE || (AT_SKILL_ASHAKE_UP <= bySkillType && bySkillType <= AT_SKILL_ASHAKE_UP+4))
		{
			BYTE byDarkHorseLife = 0;
			byDarkHorseLife = CharacterMachine->Equipment[EQUIPMENT_HELPER].Durability;
			if(byDarkHorseLife == 0) 
			{
#ifdef KJH_ADD_SKILLICON_RENEWAL
				bCantSkill = true;
#else // KJH_ADD_SKILLICON_RENEWAL
				glColor3f ( 1.f, 0.5f, 0.5f );
#endif // KJH_ADD_SKILLICON_RENEWAL
			}
		}
	}

	int iCharisma = CharacterAttribute->Charisma+CharacterAttribute->AddCharisma;	// 마이너스 열매 작업

	if(g_csItemOption.IsDisableSkill(bySkillType, iEnergy, iCharisma))	// 통솔포인트 비교해서 사용 못하는 스킬이면 빨갛게 처리
	{
#ifdef KJH_ADD_SKILLICON_RENEWAL
		bCantSkill = true;
#else // KJH_ADD_SKILLICON_RENEWAL
		glColor3f(1.f, 0.5f, 0.5f);
#endif // KJH_ADD_SKILLICON_RENEWAL
	}

#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	if(!g_CMonkSystem.IsSwordformGlovesUseSkill(bySkillType))
	{
		bCantSkill = true;
	}
	if(g_CMonkSystem.IsRideNotUseSkill(bySkillType, Hero->Helper.Type))
	{
		bCantSkill = true;
	}

	ITEM* pLeftRing = &CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];
	ITEM* pRightRing = &CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];
	
	if(g_CMonkSystem.IsChangeringNotUseSkill(pLeftRing->Type, pRightRing->Type, pLeftRing->Level, pRightRing->Level)
		&& (GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER))
	{
		bCantSkill = true;
	}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
	//////////////////////////////////////////////////////////////////////////
	
	// 실제 렌더링 하는 곳
	float fU, fV;
	int iKindofSkill = 0;
	
	
#ifdef PJH_FIX_4_BUGFIX_33
	if(g_csItemOption.Special_Option_Check() == false && (bySkillType == AT_SKILL_ICE_BLADE||(AT_SKILL_POWER_SLASH_UP <= bySkillType && AT_SKILL_POWER_SLASH_UP+4 >= bySkillType)))
#ifdef KJH_ADD_SKILLICON_RENEWAL
	{
		bCantSkill = true;
	}
#else // KJH_ADD_SKILLICON_RENEWAL
		glColor3f(1.f, 0.5f, 0.5f);
#endif // KJH_ADD_SKILLICON_RENEWAL

	if(g_csItemOption.Special_Option_Check(1) == false && (bySkillType == AT_SKILL_CROSSBOW||(AT_SKILL_MANY_ARROW_UP <= bySkillType && AT_SKILL_MANY_ARROW_UP+4 >= bySkillType)))
#ifdef KJH_ADD_SKILLICON_RENEWAL
		bCantSkill = true;
#else // KJH_ADD_SKILLICON_RENEWAL
		glColor3f(0.5f, 0.5f, 1.f);
#endif // KJH_ADD_SKILLICON_RENEWAL
#endif //PJH_FIX_4_BUGFIX_33
	

#ifdef KJH_ADD_SKILLICON_RENEWAL
	if(bySkillType >= AT_PET_COMMAND_DEFAULT && bySkillType <= AT_PET_COMMAND_END)    //  팻 명령.
    {
		fU = ((bySkillType - AT_PET_COMMAND_DEFAULT) % 8) * width / 256.f;
		fV = ((bySkillType - AT_PET_COMMAND_DEFAULT) / 8) * height / 256.f;
		iKindofSkill = KOS_COMMAND;
    }
    else if(bySkillType == AT_SKILL_PLASMA_STORM_FENRIR)	// 플라즈마 스톰
	{
		fU = 4 * width / 256.f;
		fV = 0.f;
		iKindofSkill = KOS_COMMAND;
	}
	else if((bySkillType >= AT_SKILL_ALICE_DRAINLIFE && bySkillType <= AT_SKILL_ALICE_THORNS)
		/*
#ifdef PJH_ADD_MASTERSKILL
		|| (AT_SKILL_ALICE_DRAINLIFE_UP<=bySkillType && bySkillType<= AT_SKILL_ALICE_DRAINLIFE_UP+4)
#endif
		*/
		)
	{
		fU = ((bySkillType - AT_SKILL_ALICE_DRAINLIFE) % 8) * width / 256.f;
		fV = 3 * height / 256.f;
		iKindofSkill = KOS_SKILL2;
	}
	else if(bySkillType >= AT_SKILL_ALICE_SLEEP && bySkillType <= AT_SKILL_ALICE_BLIND)
	{
		fU = ((bySkillType - AT_SKILL_ALICE_SLEEP + 4) % 8) * width / 256.f;
		fV = 3 * height / 256.f;
		iKindofSkill = KOS_SKILL2;
	}
#ifdef ASG_ADD_SKILL_BERSERKER
	else if (bySkillType == AT_SKILL_ALICE_BERSERKER)
	{
		fU = 10 * width / 256.f;
		fV = 3 * height / 256.f;
		iKindofSkill = KOS_SKILL2;
	}
#endif	// ASG_ADD_SKILL_BERSERKER
	else if (bySkillType >= AT_SKILL_ALICE_WEAKNESS && bySkillType <= AT_SKILL_ALICE_ENERVATION)
	{
		fU = (bySkillType - AT_SKILL_ALICE_WEAKNESS + 8) * width / 256.f;
		fV = 3 * height / 256.f;
		iKindofSkill = KOS_SKILL2;
	}
	else if(bySkillType >= AT_SKILL_SUMMON_EXPLOSION && bySkillType <= AT_SKILL_SUMMON_REQUIEM)
	{
		fU = ((bySkillType - AT_SKILL_SUMMON_EXPLOSION + 6) % 8) * width / 256.f;
		fV = 3 * height / 256.f;
		iKindofSkill = KOS_SKILL2;
	}
#ifdef ASG_ADD_SUMMON_RARGLE
	else if (bySkillType == AT_SKILL_SUMMON_POLLUTION)
	{
		fU = 11 * width / 256.f;
		fV = 3 * height / 256.f;
		iKindofSkill = KOS_SKILL2;
	}
#endif	// ASG_ADD_SUMMON_RARGLE
#ifdef CSK_ADD_SKILL_BLOWOFDESTRUCTION
	else if (bySkillType == AT_SKILL_BLOW_OF_DESTRUCTION)
	{
		fU = 7 * width / 256.f;
		fV = 2 * height / 256.f;
		iKindofSkill = KOS_SKILL2;
	}
#endif // CSK_ADD_SKILL_BLOWOFDESTRUCTION
#ifdef PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
	else if (bySkillType == AT_SKILL_GAOTIC)
	{
		fU = 3 * width / 256.f;
		fV = 8 * height / 256.f;
		iKindofSkill = KOS_SKILL2;
	}
#endif //#ifdef PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
	else if (bySkillType == AT_SKILL_RECOVER)
	{
		fU = 9 * width / 256.f;
		fV = 2 * height / 256.f;
		iKindofSkill = KOS_SKILL2;
	}
#endif //#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
	else if (bySkillType == AT_SKILL_MULTI_SHOT)
	{
		if (GetEquipedBowType_Skill() == BOWTYPE_NONE)	// 활을 들어야 활성화
		{
			bCantSkill = true;
		}

		fU = 0 * width / 256.f;
		fV = 8 * height / 256.f;
		iKindofSkill = KOS_SKILL2;
	}
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
#ifdef YDG_ADD_SKILL_FLAME_STRIKE
	else if (bySkillType == AT_SKILL_FLAME_STRIKE)
	{
#ifdef YDG_FIX_STAFF_FLAMESTRIKE_IN_CHAOSCASLE
		int iTypeL = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;
		int iTypeR = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;
		
		if ( !( iTypeR!=-1 && 
		( iTypeR<ITEM_STAFF || iTypeR>=ITEM_STAFF+MAX_ITEM_INDEX ) &&
		( iTypeL<ITEM_STAFF || iTypeL>=ITEM_STAFF+MAX_ITEM_INDEX ) ) )
#else	// YDG_FIX_STAFF_FLAMESTRIKE_IN_CHAOSCASLE
//		if (!FindHeroSkill(AT_SKILL_ICE_BLADE))	// 파워슬래시가 있어야 활성화
#ifdef YDG_FIX_MACE_FLAMESTRIKE
		if (!(Hero->Weapon[0].Type>=MODEL_SWORD+0 && Hero->Weapon[0].Type<MODEL_SPEAR+MAX_ITEM_INDEX))	// 검,도끼,둔기,창을 들어야 활성화
#else	// YDG_FIX_MACE_FLAMESTRIKE
		if (!(Hero->Weapon[0].Type>=MODEL_SWORD+0 && Hero->Weapon[0].Type<MODEL_SWORD+MAX_ITEM_INDEX))	// 검을 들어야 활성화
#endif	// YDG_FIX_MACE_FLAMESTRIKE
#endif	// YDG_FIX_STAFF_FLAMESTRIKE_IN_CHAOSCASLE
		{
			bCantSkill = true;
		}

		fU = 1 * width / 256.f;
		fV = 8 * height / 256.f;
		iKindofSkill = KOS_SKILL2;
	}
#endif	// YDG_ADD_SKILL_FLAME_STRIKE
#ifdef YDG_ADD_SKILL_GIGANTIC_STORM
	else if (bySkillType == AT_SKILL_GIGANTIC_STORM)
	{
		fU = 2 * width / 256.f;
		fV = 8 * height / 256.f;
		iKindofSkill = KOS_SKILL2;
	}
#endif	// YDG_ADD_SKILL_GIGANTIC_STORM
#ifdef YDG_ADD_SKILL_LIGHTNING_SHOCK
	else if (bySkillType == AT_SKILL_LIGHTNING_SHOCK)
	{
		fU = 2 * width / 256.f;
		fV = 3 * height / 256.f;
		iKindofSkill = KOS_SKILL2;
	}
#endif	// YDG_ADD_SKILL_LIGHTNING_SHOCK
#ifdef PJH_ADD_MASTERSKILL
	else if(AT_SKILL_LIGHTNING_SHOCK_UP <= bySkillType && bySkillType <= AT_SKILL_LIGHTNING_SHOCK_UP+4)
	{
		fU = 6 * width / 256.f;
		fV = 8 * height / 256.f;
		iKindofSkill = KOS_SKILL2;
	}
#endif
#ifdef KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
	else if( bySkillType == AT_SKILL_SWELL_OF_MAGICPOWER )
	{
		fU = 8 * width / 256.f;
		fV = 2 * height / 256.f;
		iKindofSkill = KOS_SKILL2;
	}
#endif // KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
	else if(bySkillUseType == 4)
	{
		fU = (width/256.f) * (Skill_Icon%12);
		fV = (height/256.f)*((Skill_Icon/12)+4);
		iKindofSkill = KOS_SKILL2;
	}
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	else if(bySkillType >= AT_SKILL_THRUST)
	{
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		float test = 0.0f;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		fU = ((bySkillType - 260) % 12) * width / 256.f;
		fV = ((bySkillType - 260) / 12) * height / 256.f;
		iKindofSkill = KOS_SKILL3;
	}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
	else if(bySkillType >= 57)
    {
		fU = ((bySkillType - 57) % 8) * width / 256.f;
		fV = ((bySkillType - 57) / 8) * height / 256.f;
		iKindofSkill = KOS_SKILL2;
    }
    else
    {
		fU = ((bySkillType - 1) % 8) * width / 256.f;
		fV = ((bySkillType - 1) / 8) * height / 256.f;
		iKindofSkill = KOS_SKILL1;
    }
	int iSkillIndex = 0;
	switch(iKindofSkill)
	{
	case KOS_COMMAND:
		{
			iSkillIndex = IMAGE_COMMAND;
		}break;
	case KOS_SKILL1:
		{
			iSkillIndex = IMAGE_SKILL1;
		}break;
	case KOS_SKILL2:
		{
			iSkillIndex = IMAGE_SKILL2;
		}break;
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	case KOS_SKILL3:
		{
			iSkillIndex = IMAGE_SKILL3;
		}break;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
	}

	if( bCantSkill == true )
	{
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
		iSkillIndex += 6;		// 이미지 인덱스 순서가 일정하다는것을 전제, 이미지 인덱스를 바꾸면 안됨!!!
#else //PBG_ADD_NEWCHAR_MONK_SKILL
		iSkillIndex += 5;		// 이미지 인덱스 순서가 일정하다는것을 전제, 이미지 인덱스를 바꾸면 안됨!!!
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
	}
	
	if( iSkillIndex != 0)
	{
		RenderBitmap(iSkillIndex, x, y, width, height, fU, fV, width/256.f, height/256.f);	
	}

#else // KJH_ADD_SKILLICON_RENEWAL

	if(bySkillType >= AT_PET_COMMAND_DEFAULT && bySkillType <= AT_PET_COMMAND_END)    //  팻 명령.
    {
		fU = ((bySkillType - AT_PET_COMMAND_DEFAULT) % 8) * width / 256.f;
		fV = ((bySkillType - AT_PET_COMMAND_DEFAULT) / 8) * height / 256.f;
		RenderBitmap(IMAGE_COMMAND, x, y, width, height, fU, fV, width/256.f, height/256.f);
    }
    else if(bySkillType == AT_SKILL_PLASMA_STORM_FENRIR)	// 플라즈마 스톰
	{
		fU = 4 * width / 256.f;
		fV = 0.f;
		RenderBitmap(IMAGE_COMMAND, x, y, width, height, fU, fV, width/256.f, height/256.f);
	}
	else if(bySkillType >= AT_SKILL_ALICE_DRAINLIFE && bySkillType <= AT_SKILL_ALICE_THORNS)
	{
		fU = ((bySkillType - AT_SKILL_ALICE_DRAINLIFE) % 8) * width / 256.f;
		fV = 3 * height / 256.f;
		RenderBitmap(IMAGE_SKILL2, x, y, width, height, fU, fV, width/256.f, height/256.f);
	}
	else if(bySkillType >= AT_SKILL_ALICE_SLEEP && bySkillType <= AT_SKILL_ALICE_BLIND)
	{
		fU = ((bySkillType - AT_SKILL_ALICE_SLEEP + 4) % 8) * width / 256.f;
		fV = 3 * height / 256.f;
		RenderBitmap(IMAGE_SKILL2, x, y, width, height, fU, fV, width/256.f, height/256.f);
	}
#ifdef ASG_ADD_SKILL_BERSERKER
	else if (bySkillType == AT_SKILL_ALICE_BERSERKER)
	{
		fU = 10 * width / 256.f;
		fV = 3 * height / 256.f;
		RenderBitmap(IMAGE_SKILL2, x, y, width, height, fU, fV, width/256.f, height/256.f);	
	}
#endif	// ASG_ADD_SKILL_BERSERKER
	else if (bySkillType >= AT_SKILL_ALICE_WEAKNESS && bySkillType <= AT_SKILL_ALICE_ENERVATION)
	{
		fU = (bySkillType - AT_SKILL_ALICE_WEAKNESS + 8) * width / 256.f;
		fV = 3 * height / 256.f;
		RenderBitmap(IMAGE_SKILL2, x, y, width, height, fU, fV, width/256.f, height/256.f);	
	}
	else if(bySkillType >= AT_SKILL_SUMMON_EXPLOSION && bySkillType <= AT_SKILL_SUMMON_REQUIEM)
	{
		fU = ((bySkillType - AT_SKILL_SUMMON_EXPLOSION + 6) % 8) * width / 256.f;
		fV = 3 * height / 256.f;
		RenderBitmap(IMAGE_SKILL2, x, y, width, height, fU, fV, width/256.f, height/256.f);	
	}
#ifdef ASG_ADD_SUMMON_RARGLE
	else if (bySkillType == AT_SKILL_SUMMON_POLLUTION)
	{
		fU = 11 * width / 256.f;
		fV = 3 * height / 256.f;
		RenderBitmap(IMAGE_SKILL2, x, y, width, height, fU, fV, width/256.f, height/256.f);	
	}
#endif	// ASG_ADD_SUMMON_RARGLE
#ifdef CSK_ADD_SKILL_BLOWOFDESTRUCTION
	else if (bySkillType == AT_SKILL_BLOW_OF_DESTRUCTION)
	{
		fU = 7 * width / 256.f;
		fV = 2 * height / 256.f;
		RenderBitmap(IMAGE_SKILL2, x, y, width, height, fU, fV, width/256.f, height/256.f);	
	}
#endif // CSK_ADD_SKILL_BLOWOFDESTRUCTION
#ifdef PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
	else if (bySkillType == AT_SKILL_GAOTIC)
	{
		fU = 3 * width / 256.f;
		fV = 8 * height / 256.f;
		RenderBitmap(IMAGE_SKILL2, x, y, width, height, fU, fV, width/256.f, height/256.f);	
	}
#endif //#ifdef PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
	else if (bySkillType == AT_SKILL_RECOVER)
	{
		fU = 9 * width / 256.f;
		fV = 2 * height / 256.f;
		RenderBitmap(IMAGE_SKILL2, x, y, width, height, fU, fV, width/256.f, height/256.f);	
	}
#endif //#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
	else if (bySkillType == AT_SKILL_MULTI_SHOT)
	{
		if (GetEquipedBowType_Skill() == BOWTYPE_NONE)	// 활을 들어야 활성화
			glColor3f ( 1.f, 0.5f, 0.5f );

		fU = 0 * width / 256.f;
		fV = 8 * height / 256.f;
		RenderBitmap(IMAGE_SKILL2, x, y, width, height, fU, fV, width/256.f, height/256.f);	
	}
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
#ifdef YDG_ADD_SKILL_FLAME_STRIKE
	else if (bySkillType == AT_SKILL_FLAME_STRIKE)
	{
#ifdef YDG_FIX_STAFF_FLAMESTRIKE_IN_CHAOSCASLE
		int iTypeL = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;
		int iTypeR = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;
		
		if ( !( iTypeR!=-1 && 
		( iTypeR<ITEM_STAFF || iTypeR>=ITEM_STAFF+MAX_ITEM_INDEX ) &&
		( iTypeL<ITEM_STAFF || iTypeL>=ITEM_STAFF+MAX_ITEM_INDEX ) ) )
#else	// YDG_FIX_STAFF_FLAMESTRIKE_IN_CHAOSCASLE
//		if (!FindHeroSkill(AT_SKILL_ICE_BLADE))	// 파워슬래시가 있어야 활성화
#ifdef YDG_FIX_MACE_FLAMESTRIKE
		if (!(Hero->Weapon[0].Type>=MODEL_SWORD+0 && Hero->Weapon[0].Type<MODEL_SPEAR+MAX_ITEM_INDEX))	// 검,도끼,둔기,창을 들어야 활성화
#else	// YDG_FIX_MACE_FLAMESTRIKE
		if (!(Hero->Weapon[0].Type>=MODEL_SWORD+0 && Hero->Weapon[0].Type<MODEL_SWORD+MAX_ITEM_INDEX))	// 검을 들어야 활성화
#endif	// YDG_FIX_MACE_FLAMESTRIKE
#endif	// YDG_FIX_STAFF_FLAMESTRIKE_IN_CHAOSCASLE
			glColor3f ( 1.f, 0.5f, 0.5f );

		fU = 1 * width / 256.f;
		fV = 8 * height / 256.f;
		RenderBitmap(IMAGE_SKILL2, x, y, width, height, fU, fV, width/256.f, height/256.f);	
	}
#endif	// YDG_ADD_SKILL_FLAME_STRIKE
#ifdef YDG_ADD_SKILL_GIGANTIC_STORM
	else if (bySkillType == AT_SKILL_GIGANTIC_STORM)
	{
		fU = 2 * width / 256.f;
		fV = 8 * height / 256.f;
		RenderBitmap(IMAGE_SKILL2, x, y, width, height, fU, fV, width/256.f, height/256.f);	
	}
#endif	// YDG_ADD_SKILL_GIGANTIC_STORM
#ifdef YDG_ADD_SKILL_LIGHTNING_SHOCK
	else if (bySkillType == AT_SKILL_LIGHTNING_SHOCK)
	{
		fU = 2 * width / 256.f;
		fV = 3 * height / 256.f;
		RenderBitmap(IMAGE_SKILL2, x, y, width, height, fU, fV, width/256.f, height/256.f);	
	}
#endif	// YDG_ADD_SKILL_LIGHTNING_SHOCK
#ifdef PJH_ADD_MASTERSKILL
	else if(AT_SKILL_LIGHTNING_SHOCK_UP <= bySkillType && bySkillType <= AT_SKILL_LIGHTNING_SHOCK_UP+4)
	{
		fU = 6 * width / 256.f;
		fV = 8 * height / 256.f;
		RenderBitmap(IMAGE_SKILL2, x, y, width, height, fU, fV, width/256.f, height/256.f);	
	}
#endif

#ifdef KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
	else if( bySkillType == AT_SKILL_SWELL_OF_MAGICPOWER )
	{
		fU = 8 * width / 256.f;
		fV = 2 * height / 256.f;
		RenderBitmap(IMAGE_SKILL2, x, y, width, height, fU, fV, width/256.f, height/256.f);	
	}
#endif // KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
	else if(bySkillUseType == 4)
	{
		fU = (width/256.f) * (Skill_Icon%12);
		fV = (height/256.f)*((Skill_Icon/12)+4);
		RenderBitmap(IMAGE_SKILL2, x, y, width, height, fU, fV, width/256.f, height/256.f);
	}
	else if(bySkillType >= 57)
    {
		fU = ((bySkillType - 57) % 8) * width / 256.f;
		fV = ((bySkillType - 57) / 8) * height / 256.f;
		RenderBitmap(IMAGE_SKILL2, x, y, width, height, fU, fV, width/256.f, height/256.f);
    }
    else
    {
		fU = ((bySkillType - 1) % 8) * width / 256.f;
		fV = ((bySkillType - 1) / 8) * height / 256.f;
		RenderBitmap(IMAGE_SKILL1, x, y, width, height, fU, fV, width/256.f, height/256.f);
    }
#endif // KJH_ADD_SKILLICON_RENEWAL

	// 핫키 번호 렌더링
	int iHotKey = -1;
	for(int i=0; i<SKILLHOTKEY_COUNT; ++i)
	{
		if(m_iHotKeySkillType[i] == iIndex)
		{
			iHotKey = i;
			break;
		}
	}

	if(iHotKey != -1)
	{
		glColor3f(1.f, 0.9f, 0.8f);
		SEASON3B::RenderNumber(x+20, y+20, iHotKey);
	}

#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	if((bySkillType == AT_SKILL_GIANTSWING || bySkillType == AT_SKILL_DRAGON_KICK
		|| bySkillType == AT_SKILL_DRAGON_LOWER) && (bCantSkill))
		return;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL

	//  딜레이 값을 찍는다. (딜레이 값이 없어서 사용하고 있지 않음)
#ifdef KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
	if( (bySkillType != AT_SKILL_INFINITY_ARROW) && (bySkillType != AT_SKILL_SWELL_OF_MAGICPOWER))
#else KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
 	if(bySkillType != AT_SKILL_INFINITY_ARROW)
#endif // KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
	{
		RenderSkillDelay(iIndex, x, y, width, height);
	}
	
#ifndef KJH_ADD_SKILLICON_RENEWAL				// #ifndef
	glColor3f(1.f, 1.f, 1.f);
#endif // KJH_ADD_SKILLICON_RENEWAL
}

void SEASON3B::CNewUISkillList::RenderSkillDelay(int iIndex, float x, float y, float width, float height)
{
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

    int iSkillDelay = CharacterAttribute->SkillDelay[iIndex];
    if ( iSkillDelay > 0 )
    {
        int iSkillType  = CharacterAttribute->Skill[iIndex];
		
#ifdef PBG_WOPS_DARKLOAD
		if(iSkillType == AT_SKILL_PLASMA_STORM_FENRIR)
		{
			if(!CheckAttack())
			{
				return;
			}
		}
#endif //PBG_WOPS_DARKLOAD
		//////////////////////////////////////////////////////////////////////////
        int iSkillMaxDelay = SkillAttribute[iSkillType].Delay;
		//////////////////////////////////////////////////////////////////////////
		
        float fPersent = (float)(iSkillDelay/(float)iSkillMaxDelay);
        
		EnableAlphaTest();
        glColor4f(1.f, 0.5f, 0.5f, 0.5f);
        float fdeltaH = height * fPersent;
        RenderColor(x, y+height-fdeltaH, width, fdeltaH);
		EndRenderColor();
    }

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
}

bool SEASON3B::CNewUISkillList::IsSkillListUp()
{
	return m_bHotKeySkillListUp;
}

void SEASON3B::CNewUISkillList::ResetMouseLButton()
{
	MouseLButton = false;
	MouseLButtonPop = false;
	MouseLButtonPush = false;
}

void SEASON3B::CNewUISkillList::UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB)
{
#ifdef LDK_ADD_SCALEFORM
	if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
	{
		if(pClass)
		{
			CNewUISkillList* pSkillList= (CNewUISkillList*)(pClass);
			pSkillList->RenderSkillInfo();
		}
	}
#else //LDK_ADD_SCALEFORM
	if(pClass)
	{
		CNewUISkillList* pSkillList= (CNewUISkillList*)(pClass);
		pSkillList->RenderSkillInfo();
	}
#endif //LDK_ADD_SCALEFORM
}

//////////////////////////////////////////////////////////////////////////
void SEASON3B::CNewUIMainFrameWindow::SetPreExp_Wide(__int64 dwPreExp)
{
	m_loPreExp = dwPreExp;
}

void SEASON3B::CNewUIMainFrameWindow::SetGetExp_Wide(__int64 dwGetExp)
{
	m_loGetExp = dwGetExp;

	if(m_loGetExp > 0)
	{
		m_bExpEffect = true;
		m_dwExpEffectTime = timeGetTime();
	}
}

void SEASON3B::CNewUIMainFrameWindow::SetPreExp(DWORD dwPreExp)
{
	m_dwPreExp = dwPreExp;
}

void SEASON3B::CNewUIMainFrameWindow::SetGetExp(DWORD dwGetExp)
{
	m_dwGetExp = dwGetExp;

	if(m_dwGetExp > 0)
	{
		m_bExpEffect = true;
		m_dwExpEffectTime = timeGetTime();
	}
}

void SEASON3B::CNewUIMainFrameWindow::SetBtnState(int iBtnType, bool bStateDown)
{
	switch(iBtnType)
	{
#if defined NEW_USER_INTERFACE_MAINFRAME_BUTTON || defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
	case MAINFRAME_BTN_PARTCHARGE:
		{
			if(bStateDown) 
			{
				m_BtnCShop.UnRegisterButtonState();
				m_BtnCShop.RegisterButtonState(BUTTON_STATE_UP, IMAGE_MENU_BTN_CSHOP, 2);
				m_BtnCShop.RegisterButtonState(BUTTON_STATE_OVER, IMAGE_MENU_BTN_CSHOP, 3);
				m_BtnCShop.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_MENU_BTN_CSHOP, 2);
				m_BtnCShop.ChangeImgIndex(IMAGE_MENU_BTN_CSHOP, 2);
			}
			else 
			{
				m_BtnCShop.UnRegisterButtonState();
				m_BtnCShop.RegisterButtonState(BUTTON_STATE_UP, IMAGE_MENU_BTN_CSHOP, 0);
				m_BtnCShop.RegisterButtonState(BUTTON_STATE_OVER, IMAGE_MENU_BTN_CSHOP, 1);
				m_BtnCShop.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_MENU_BTN_CSHOP, 2);
				m_BtnCShop.ChangeImgIndex(IMAGE_MENU_BTN_CSHOP, 0);
			}
		}
		break;
#endif //defined NEW_USER_INTERFACE_MAINFRAME_BUTTON || defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
	case MAINFRAME_BTN_CHAINFO:
		{
			if(bStateDown)
			{
				m_BtnChaInfo.UnRegisterButtonState();
				m_BtnChaInfo.RegisterButtonState(BUTTON_STATE_UP, IMAGE_MENU_BTN_CHAINFO, 2);
				m_BtnChaInfo.RegisterButtonState(BUTTON_STATE_OVER, IMAGE_MENU_BTN_CHAINFO, 3);
				m_BtnChaInfo.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_MENU_BTN_CHAINFO, 2);
				m_BtnChaInfo.ChangeImgIndex(IMAGE_MENU_BTN_CHAINFO, 2);
				
			}
			else
			{
				m_BtnChaInfo.UnRegisterButtonState();
				m_BtnChaInfo.RegisterButtonState(BUTTON_STATE_UP, IMAGE_MENU_BTN_CHAINFO, 0);
				m_BtnChaInfo.RegisterButtonState(BUTTON_STATE_OVER, IMAGE_MENU_BTN_CHAINFO, 1);
				m_BtnChaInfo.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_MENU_BTN_CHAINFO, 2);
				m_BtnChaInfo.ChangeImgIndex(IMAGE_MENU_BTN_CHAINFO, 0);
			}
		}
		break;
	case MAINFRAME_BTN_MYINVEN:
		{
			if(bStateDown)
			{
				m_BtnMyInven.UnRegisterButtonState();
				m_BtnMyInven.RegisterButtonState(BUTTON_STATE_UP, IMAGE_MENU_BTN_MYINVEN, 2);
				m_BtnMyInven.RegisterButtonState(BUTTON_STATE_OVER, IMAGE_MENU_BTN_MYINVEN, 3);
				m_BtnMyInven.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_MENU_BTN_MYINVEN, 2);
				m_BtnMyInven.ChangeImgIndex(IMAGE_MENU_BTN_MYINVEN, 2);
			}
			else
			{
				m_BtnMyInven.UnRegisterButtonState();
				m_BtnMyInven.RegisterButtonState(BUTTON_STATE_UP, IMAGE_MENU_BTN_MYINVEN, 0);
				m_BtnMyInven.RegisterButtonState(BUTTON_STATE_OVER, IMAGE_MENU_BTN_MYINVEN, 1);
				m_BtnMyInven.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_MENU_BTN_MYINVEN, 2);
				m_BtnMyInven.ChangeImgIndex(IMAGE_MENU_BTN_MYINVEN, 0);
			}
		}
		break;
	case MAINFRAME_BTN_FRIEND:
		{
			if(bStateDown)
			{
				m_BtnFriend.UnRegisterButtonState();
				m_BtnFriend.RegisterButtonState(BUTTON_STATE_UP, IMAGE_MENU_BTN_FRIEND, 2);
				m_BtnFriend.RegisterButtonState(BUTTON_STATE_OVER, IMAGE_MENU_BTN_FRIEND, 3);
				m_BtnFriend.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_MENU_BTN_FRIEND, 2);
				m_BtnFriend.ChangeImgIndex(IMAGE_MENU_BTN_FRIEND, 2);
			}
			else
			{
				m_BtnFriend.UnRegisterButtonState();
				m_BtnFriend.RegisterButtonState(BUTTON_STATE_UP, IMAGE_MENU_BTN_FRIEND, 0);
				m_BtnFriend.RegisterButtonState(BUTTON_STATE_OVER, IMAGE_MENU_BTN_FRIEND, 1);
				m_BtnFriend.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_MENU_BTN_FRIEND, 2);
				m_BtnFriend.ChangeImgIndex(IMAGE_MENU_BTN_FRIEND, 0);
			}
		}
		break;
	case MAINFRAME_BTN_WINDOW:
		{
			if(bStateDown)
			{
				m_BtnWindow.UnRegisterButtonState();
				m_BtnWindow.RegisterButtonState(BUTTON_STATE_UP, IMAGE_MENU_BTN_WINDOW, 2);
				m_BtnWindow.RegisterButtonState(BUTTON_STATE_OVER, IMAGE_MENU_BTN_WINDOW, 3);
				m_BtnWindow.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_MENU_BTN_WINDOW, 2);
				m_BtnWindow.ChangeImgIndex(IMAGE_MENU_BTN_WINDOW, 2);
			}
			else
			{
				m_BtnWindow.UnRegisterButtonState();
				m_BtnWindow.RegisterButtonState(BUTTON_STATE_UP, IMAGE_MENU_BTN_WINDOW, 0);
				m_BtnWindow.RegisterButtonState(BUTTON_STATE_OVER, IMAGE_MENU_BTN_WINDOW, 1);
				m_BtnWindow.RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_MENU_BTN_WINDOW, 2);
				m_BtnWindow.ChangeImgIndex(IMAGE_MENU_BTN_WINDOW, 0);
			}
		}
		break;
	}
}
