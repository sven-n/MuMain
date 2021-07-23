// NewUIMyQuestInfoWindow.cpp: implementation of the CNewUIMyQuestInfoWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIMyQuestInfoWindow.h"
#include "wsclientinline.h"
#include "csQuest.h"

using namespace SEASON3B;

extern int g_iNumLineMessageBoxCustom;
extern int g_iNumAnswer;
extern char g_lpszMessageBoxCustom[NUM_LINE_CMB][MAX_LENGTH_CMB];

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SEASON3B::CNewUIMyQuestInfoWindow::CNewUIMyQuestInfoWindow()
{
	m_pNewUIMng = NULL;
	m_Pos.x = m_Pos.y = 0;
}

SEASON3B::CNewUIMyQuestInfoWindow::~CNewUIMyQuestInfoWindow()
{
	Release(); 
}

bool SEASON3B::CNewUIMyQuestInfoWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if(NULL == pNewUIMng)
		return false;

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_MYQUEST, this);

	SetPos(x, y);

	LoadImages();

	SetButtonInfo();

#ifdef ASG_MOD_UI_QUEST_INFO
	m_eTabBtnIndex = TAB_QUEST;
#endif	// ASG_MOD_UI_QUEST_INFO

	Show(false);

	return true;
}

void SEASON3B::CNewUIMyQuestInfoWindow::Release()
{
	UnloadImages();

	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}
}
		
void SEASON3B::CNewUIMyQuestInfoWindow::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;

#ifndef ASG_MOD_UI_QUEST_INFO		// 정리시 삭제.
	m_BtnTab.ChangeRadioButtonInfo( true, m_Pos.x+12.f, m_Pos.y+202.f, 56, 22 );
#endif	// ASG_MOD_UI_QUEST_INFO
	m_BtnExit.ChangeButtonInfo( m_Pos.x+13, m_Pos.y+392, 36, 29 );
#ifdef ASG_MOD_UI_QUEST_INFO
	m_btnQuestOpen.ChangeButtonInfo(m_Pos.x+50, m_Pos.y+392, 36, 29);		
	m_btnQuestGiveUp.ChangeButtonInfo(m_Pos.x+87, m_Pos.y+392, 36, 29);

	m_CurQuestListBox.SetPosition(m_Pos.x+9, m_Pos.y+160);
	m_QuestContentsListBox.SetPosition(m_Pos.x+9, m_Pos.y+390);
#endif	// ASG_MOD_UI_QUEST_INFO
}
		
bool SEASON3B::CNewUIMyQuestInfoWindow::UpdateMouseEvent()
{
#ifdef ASG_MOD_UI_QUEST_INFO
	if (m_eTabBtnIndex == TAB_QUEST)
	{
		m_CurQuestListBox.DoAction();
		m_QuestContentsListBox.DoAction();
	}
#endif	// ASG_MOD_UI_QUEST_INFO

	// 버튼 마우스이벤트 처리
	if(BtnProcess() == true)	// 처리가 완료 되었다면
	{
		return false;
	}

	if(CheckMouseIn(m_Pos.x, m_Pos.y, MYQUESTINFO_WINDOW_WIDTH, MYQUESTINFO_WINDOW_HEIGHT))
	{
		return false;
	}
	
	return true;
}

bool SEASON3B::CNewUIMyQuestInfoWindow::BtnProcess()
{
	POINT ptExitBtn1 = { m_Pos.x+169, m_Pos.y+7 };

	//. Exit1 버튼 (기본처리)

	if(SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_MYQUEST);
		return true;
	}

	if(m_BtnExit.UpdateMouseEvent() == true)
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_MYQUEST);
		return true;
	}

#ifdef ASG_MOD_UI_QUEST_INFO
	TAB_BUTTON_INDEX eTabBtnIndex = UpdateTabBtn();	// 탭을 클릭시에만 아래 if로 진행.
	if (eTabBtnIndex == TAB_QUEST)
	{
		// 진행 가능한 퀘스트 리스트가 없다면 없다고 메시지 표시.
		if (0 == m_CurQuestListBox.GetLineNum())
			SetMessage(2825);
		return true;
	}
	else if (eTabBtnIndex == TAB_JOB_CHANGE)
	{
/*		BYTE byState = g_csQuest.getCurrQuestState();
		if (byState == QUEST_NONE || byState == QUEST_NO || byState == QUEST_ERROR)
			SetMessage(930);
		else if(byState == QUEST_ING )
			SetMessage(931);
		else if(byState == QUEST_END )
			SetMessage(932);
		*/
		return true;
	}
	else if (eTabBtnIndex == TAB_CASTLE_TEMPLE)
	{
		SendRequestEventCount(2);	// 블러드캐슬
		SendRequestEventCount(3);	// 환영사원
		return true;
	}

	if (m_eTabBtnIndex == TAB_QUEST)
	{
		if (m_btnQuestOpen.UpdateMouseEvent())
		{
			::PlayBuffer(SOUND_CLICK01);
			g_pQuestProgressByEtc->SetContents(GetSelQuestIndex());
			g_pNewUISystem->Show(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC);
			return true;
		}
		
		if (m_btnQuestGiveUp.UpdateMouseEvent())
		{
			::PlayBuffer(SOUND_CLICK01);
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CQuestGiveUpMsgBoxLayout));
			return true;
		}
	}
#else	// ASG_MOD_UI_QUEST_INFO
	int iIndex = m_BtnTab.UpdateMouseEvent();
	if(iIndex == TAB_CASTLE)
	{
		SendRequestEventCount( 2 );	// 블러드캐슬
		return true;
	}
	else if(iIndex == TAB_TEMPLE)
	{
		SendRequestEventCount( 3 );	// 환영사원
		return true;
	}
	else if(iIndex != RADIOGROUPEVENT_NONE)
	{
		return true;
	}
#endif	// ASG_MOD_UI_QUEST_INFO

	return false;
}

bool SEASON3B::CNewUIMyQuestInfoWindow::UpdateKeyEvent()
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MYQUEST) == true)
	{
		if(SEASON3B::IsPress(VK_ESCAPE) == true)
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_MYQUEST);
#ifndef ASG_MOD_UI_QUEST_INFO
			PlayBuffer(SOUND_CLICK01);
#endif	// ASG_MOD_UI_QUEST_INFO
			return false;
		}
	}

	return true;
}

bool SEASON3B::CNewUIMyQuestInfoWindow::Update()
{
	return true;
}

bool SEASON3B::CNewUIMyQuestInfoWindow::Render()
{
	EnableAlphaTest();

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// 프레임을 그린다.
	RenderFrame();
#ifdef ASG_MOD_UI_QUEST_INFO
	RenderTabBtn();
#endif	// ASG_MOD_UI_QUEST_INFO

	// 제목을 그린다.
	RenderSubjectTexts();

#ifndef ASG_MOD_UI_QUEST_INFO	// 정리시 삭제.
	// 퀘스트 내용을 그린다.
	RenderQuestTexts();
#endif	// ASG_MOD_UI_QUEST_INFO

#ifdef ASG_MOD_UI_QUEST_INFO
	m_BtnExit.Render();

	if (m_eTabBtnIndex == TAB_QUEST)
	{
		RenderQuestInfo();
	}
	else if(m_eTabBtnIndex == TAB_JOB_CHANGE)
	{
		RenderImage(IMAGE_MYQUEST_LINE, m_Pos.x, m_Pos.y+182, 188.f, 21.f);	// 라인
		RenderJobChangeContents();
		RenderJobChangeState();
	}
	else if(m_eTabBtnIndex == TAB_CASTLE_TEMPLE)
	{
		RenderImage(IMAGE_MYQUEST_LINE, m_Pos.x, m_Pos.y+210, 188.f, 21.f);	// 라인
		RenderCastleInfo();
		RenderTempleInfo();
	}
#else	// ASG_MOD_UI_QUEST_INFO
	int iIndex = m_BtnTab.GetCurButtonIndex();

	if(iIndex == TAB_QUEST)
	{
		RenderTabQuest();
	}
	else if(iIndex == TAB_CASTLE)
	{
		RenderTabCastle();
	}
	else if(iIndex == TAB_TEMPLE)
	{
		RenderTabTemple();
	}

	// 버튼 그린다.
	RenderButtons();
#endif	// ASG_MOD_UI_QUEST_INFO

	DisableAlphaBlend();
	
	return true;
}
		
float SEASON3B::CNewUIMyQuestInfoWindow::GetLayerDepth()
{
	return 3.3f;
}

void SEASON3B::CNewUIMyQuestInfoWindow::LoadImages()
{
	LoadBitmap("Interface\\newui_msgbox_back.jpg", IMAGE_MYQUEST_BACK, GL_LINEAR);
#ifdef ASG_MOD_UI_QUEST_INFO
	LoadBitmap("Interface\\newui_item_back01.tga", IMAGE_MYQUEST_TOP, GL_LINEAR);
#else	// ASG_MOD_UI_QUEST_INFO
	LoadBitmap("Interface\\newui_item_back04.tga", IMAGE_MYQUEST_TOP, GL_LINEAR);
#endif	// ASG_MOD_UI_QUEST_INFO
	LoadBitmap("Interface\\newui_item_back02-L.tga", IMAGE_MYQUEST_LEFT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-R.tga", IMAGE_MYQUEST_RIGHT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back03.tga", IMAGE_MYQUEST_BOTTOM, GL_LINEAR);
	LoadBitmap("Interface\\newui_exit_00.tga", IMAGE_MYQUEST_BTN_EXIT, GL_LINEAR);
#ifndef ASG_MOD_UI_QUEST_INFO		// 정리시 삭제.
	LoadBitmap("Interface\\newui_guild_tab04.tga", IMAGE_MYQUEST_TAB_BUTTON, GL_LINEAR);
#endif	// ASG_MOD_UI_QUEST_INFO

	LoadBitmap("Interface\\newui_myquest_Line.tga", IMAGE_MYQUEST_LINE, GL_LINEAR);
#ifdef ASG_MOD_UI_QUEST_INFO
	LoadBitmap("Interface\\Quest_Bt_open.tga", IMAGE_MYQUEST_BTN_OPEN, GL_LINEAR);
	LoadBitmap("Interface\\Quest_Bt_cast.tga", IMAGE_MYQUEST_BTN_GIVE_UP, GL_LINEAR);
	LoadBitmap("Interface\\Quest_tab01.tga", IMAGE_MYQUEST_TAB_BACK, GL_LINEAR);
	LoadBitmap("Interface\\Quest_tab02.tga", IMAGE_MYQUEST_TAB_SMALL, GL_LINEAR);
	LoadBitmap("Interface\\Quest_tab03.tga", IMAGE_MYQUEST_TAB_BIG, GL_LINEAR);
#endif	// ASG_MOD_UI_QUEST_INFO
}

void SEASON3B::CNewUIMyQuestInfoWindow::UnloadImages()
{
#ifdef ASG_MOD_UI_QUEST_INFO
	DeleteBitmap(IMAGE_MYQUEST_TAB_BIG);
	DeleteBitmap(IMAGE_MYQUEST_TAB_SMALL);
	DeleteBitmap(IMAGE_MYQUEST_TAB_BACK);
	DeleteBitmap(IMAGE_MYQUEST_BTN_GIVE_UP);
	DeleteBitmap(IMAGE_MYQUEST_BTN_OPEN);
#endif	// ASG_MOD_UI_QUEST_INFO
	DeleteBitmap(IMAGE_MYQUEST_LINE);

#ifndef ASG_MOD_UI_QUEST_INFO		// 정리시 삭제.
	DeleteBitmap(IMAGE_MYQUEST_TAB_BUTTON);
#endif	// ASG_MOD_UI_QUEST_INFO
	DeleteBitmap(IMAGE_MYQUEST_BTN_EXIT);
	DeleteBitmap(IMAGE_MYQUEST_BOTTOM);
	DeleteBitmap(IMAGE_MYQUEST_RIGHT);
	DeleteBitmap(IMAGE_MYQUEST_LEFT);
	DeleteBitmap(IMAGE_MYQUEST_TOP);
	DeleteBitmap(IMAGE_MYQUEST_BACK);
}

void SEASON3B::CNewUIMyQuestInfoWindow::RenderFrame()
{
	// 프레임
	RenderImage(IMAGE_MYQUEST_BACK, m_Pos.x, m_Pos.y, 190.f, 429.f);
	RenderImage(IMAGE_MYQUEST_TOP, m_Pos.x, m_Pos.y, 190.f, 64.f);
	RenderImage(IMAGE_MYQUEST_LEFT, m_Pos.x, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_MYQUEST_RIGHT, m_Pos.x+190-21, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_MYQUEST_BOTTOM, m_Pos.x, m_Pos.y+429-45, 190.f, 45.f);
#ifndef ASG_MOD_UI_QUEST_INFO		// 정리시 삭제.
	// 라인
	RenderImage(IMAGE_MYQUEST_LINE, m_Pos.x, m_Pos.y+182, 188.f, 21.f);
#endif	// ASG_MOD_UI_QUEST_INFO
}

void SEASON3B::CNewUIMyQuestInfoWindow::RenderSubjectTexts()
{
	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetTextColor(230, 230, 230, 255);
	g_pRenderText->SetBgColor(0);
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y+12, "Quest", 190, 0, RT3_SORT_CENTER);
#ifndef ASG_MOD_UI_QUEST_INFO		// 정리시 삭제.
	g_pRenderText->SetTextColor(36, 242, 252, 255);
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y+27, g_csQuest.getQuestTitleWindow(), 190, 0, RT3_SORT_CENTER);
#endif	// ASG_MOD_UI_QUEST_INFO
}

#ifdef ASG_MOD_UI_QUEST_INFO
//*****************************************************************************
// 함수 이름 : RenderQuestInfo()
// 함수 설명 : (퀘스트 탭 선택시)퀘스트 정보 랜더.
//*****************************************************************************
void SEASON3B::CNewUIMyQuestInfoWindow::RenderQuestInfo()
{
#ifdef KJW_FIX_QUEST_INFO_DUPLICATE
#endif // KJW_FIX_QUEST_INFO_DUPLICATE
	RenderImage(IMAGE_MYQUEST_LINE, m_Pos.x, m_Pos.y+160, 188.f, 21.f);	// 라인

	if (0 == m_CurQuestListBox.GetLineNum())	// 퀘스트 리스트가 없으면.
	{
		g_pRenderText->SetFont(g_hFontBold);
		g_pRenderText->SetTextColor(255, 255, 0, 255);
		g_pRenderText->SetBgColor(0);
		int i;
		for (i = 0; i < m_nMsgLine; ++i)
			g_pRenderText->RenderText(m_Pos.x+23, m_Pos.y+96+18*i,
				m_aszMsg[i], 0, 0, RT3_SORT_LEFT);
	}
	else
		m_CurQuestListBox.Render();

	m_btnQuestOpen.Render();
	m_btnQuestGiveUp.Render();

	m_QuestContentsListBox.Render();
}

//*****************************************************************************
// 함수 이름 : RenderJobChangeContents()
// 함수 설명 : (전직 탭 선택시)전직 내용 윈도우 상단에 랜더.
//*****************************************************************************
void SEASON3B::CNewUIMyQuestInfoWindow::RenderJobChangeContents()
{
	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetTextColor(36, 242, 252, 255);
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y+58, g_csQuest.getQuestTitleWindow(), 190, 0, RT3_SORT_CENTER);

	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->SetBgColor(0);

	int iY = m_Pos.y + 76;
	for(int i=0; i<g_iNumLineMessageBoxCustom; ++i)
	{
		g_pRenderText->RenderText(m_Pos.x, iY, g_lpszMessageBoxCustom[i], 190.f, 0.f, RT3_SORT_CENTER);
		iY += 16;
	}
}
#else	// ASG_MOD_UI_QUEST_INFO
void SEASON3B::CNewUIMyQuestInfoWindow::RenderQuestTexts()
{
	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->SetBgColor(0);

	int iY = m_Pos.y + 70;
	for(int i=0; i<g_iNumLineMessageBoxCustom; ++i)
	{
		g_pRenderText->RenderText(m_Pos.x, iY, g_lpszMessageBoxCustom[i], 190.f, 0.f, RT3_SORT_CENTER);
		iY += 18;
	}
}
#endif	// ASG_MOD_UI_QUEST_INFO

#ifdef ASG_MOD_UI_QUEST_INFO
//*****************************************************************************
// 함수 이름 : RenderJobChangeState()
// 함수 설명 : (전직 탭 선택시)전직 상황 윈도우 하단에 랜더.
//*****************************************************************************
void SEASON3B::CNewUIMyQuestInfoWindow::RenderJobChangeState()
{
	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetTextColor(255, 255, 0, 255);
	g_pRenderText->SetBgColor(0);

	BYTE byState = g_csQuest.getCurrQuestState();
	if (byState == QUEST_NONE || byState == QUEST_NO || byState == QUEST_ERROR)
		SetMessage(930);
	else if(byState == QUEST_ING )
		SetMessage(931);
	else if(byState == QUEST_END )
		SetMessage(932);

	int i;
	for (i = 0; i < m_nMsgLine; ++i)
		g_pRenderText->RenderText(m_Pos.x+23, m_Pos.y+283+18*i, m_aszMsg[i], 0, 0, RT3_SORT_LEFT);
}
#else	// ASG_MOD_UI_QUEST_INFO
void SEASON3B::CNewUIMyQuestInfoWindow::RenderTabQuest()
{
	g_pRenderText->SetFont(g_hFontBig);
	g_pRenderText->SetTextColor(255, 255, 0, 255);
	g_pRenderText->SetBgColor(0, 0, 0, 0);

	BYTE byState = g_csQuest.getCurrQuestState();
	if (byState == QUEST_NONE || byState == QUEST_NO || QUEST_ERROR)
	{
		// 930 "퀘스트: 미수행"
		g_pRenderText->RenderText(m_Pos.x, m_Pos.y+280, GlobalText[930], 190, 0, RT3_SORT_CENTER);
	}
	else if(byState == QUEST_ING )
	{
		// 931 "퀘스트: 수  행"
		g_pRenderText->RenderText(m_Pos.x, m_Pos.y+280, GlobalText[931], 190, 0, RT3_SORT_CENTER);
	}
	else if(byState == QUEST_END )
	{
		// 932 "퀘스트: 완  료"
		g_pRenderText->RenderText(m_Pos.x, m_Pos.y+280, GlobalText[932], 190, 0, RT3_SORT_CENTER);
	}
}
#endif	// ASG_MOD_UI_QUEST_INFO

#ifdef ASG_MOD_UI_QUEST_INFO
//*****************************************************************************
// 함수 이름 : RenderCastleInfo()
// 함수 설명 : (캐슬/템플 탭 선택시)캐슬 정보 윈도우 상단에 랜더.
//*****************************************************************************
void SEASON3B::CNewUIMyQuestInfoWindow::RenderCastleInfo()
#else	// ASG_MOD_UI_QUEST_INFO
void SEASON3B::CNewUIMyQuestInfoWindow::RenderTabCastle()
#endif	// ASG_MOD_UI_QUEST_INFO
{
	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetTextColor(255, 255, 0, 255);
	g_pRenderText->SetBgColor(0, 0, 0, 0);

	// 56 "블러드 캐슬"
#ifdef ASG_MOD_UI_QUEST_INFO
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y+105, GlobalText[56], 190, 0, RT3_SORT_CENTER);
#else	// ASG_MOD_UI_QUEST_INFO
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y+260, GlobalText[56], 190, 0, RT3_SORT_CENTER);
#endif	// ASG_MOD_UI_QUEST_INFO

	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->SetBgColor(0, 0, 0, 0);

// 수정사유: 일본의 경우 블러드캐슬과 환영사원의 입장 횟수 제한이 없음.
#ifdef LJH_ADD_JPN_MODIFYING_QUESTINFO_ABOUT_BC_AND_CT
	// 868 "%d회 입장가능"
#ifdef ASG_MOD_UI_QUEST_INFO
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y+125, GlobalText[868], 190, 0, RT3_SORT_CENTER);
#else	// ASG_MOD_UI_QUEST_INFO
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y+280, GlobalText[868], 190, 0, RT3_SORT_CENTER);
#endif	// ASG_MOD_UI_QUEST_INFO
	
	// 829 "하루 %d회를 초과하여 입장할 수 없습니다."
#ifdef ASG_MOD_UI_QUEST_INFO
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y+145, GlobalText[829], 190, 0, RT3_SORT_CENTER);
#else	// ASG_MOD_UI_QUEST_INFO
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y+300, GlobalText[829], 190, 0, RT3_SORT_CENTER);
#endif	// ASG_MOD_UI_QUEST_INFO


#else	//LJH_ADD_JPN_MODIFYING_QUESTINFO_ABOUT_BC_AND_CT
	// 868 "%d회 입장가능"
	unicode::t_char strText[256];
	unicode::_sprintf(strText, GlobalText[868], g_csQuest.GetEventCount(2));
#ifdef ASG_MOD_UI_QUEST_INFO
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y+125, strText, 190, 0, RT3_SORT_CENTER);
#else	// ASG_MOD_UI_QUEST_INFO
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y+280, strText, 190, 0, RT3_SORT_CENTER);
#endif	// ASG_MOD_UI_QUEST_INFO
	
	// 829 "하루 %d회를 초과하여 입장할 수 없습니다."
	unicode::_sprintf(strText, GlobalText[829], 6);
#ifdef ASG_MOD_UI_QUEST_INFO
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y+145, strText, 190, 0, RT3_SORT_CENTER);
#else	// ASG_MOD_UI_QUEST_INFO
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y+300, strText, 190, 0, RT3_SORT_CENTER);
#endif	// ASG_MOD_UI_QUEST_INFO
#endif	//LJH_ADD_JPN_MODIFYING_QUESTINFO_ABOUT_BC_AND_CT
}

#ifdef ASG_MOD_UI_QUEST_INFO
//*****************************************************************************
// 함수 이름 : RenderTempleInfo()
// 함수 설명 : (캐슬/템플 탭 선택시)탬플 정보 윈도우 상단에 랜더.
//*****************************************************************************
void SEASON3B::CNewUIMyQuestInfoWindow::RenderTempleInfo()
#else	// ASG_MOD_UI_QUEST_INFO
void SEASON3B::CNewUIMyQuestInfoWindow::RenderTabTemple()
#endif	// ASG_MOD_UI_QUEST_INFO
{
	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetTextColor(255, 255, 0, 255);
	g_pRenderText->SetBgColor(0, 0, 0, 0);

	// 2369 "환영 사원"
#ifdef ASG_MOD_UI_QUEST_INFO
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y+285, GlobalText[2369], 190, 0, RT3_SORT_CENTER);
#else	// ASG_MOD_UI_QUEST_INFO
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y+260, GlobalText[2369], 190, 0, RT3_SORT_CENTER);
#endif	// ASG_MOD_UI_QUEST_INFO

	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->SetBgColor(0, 0, 0, 0);

// 수정사유: 일본의 경우 블러드캐슬과 환영사원의 입장 횟수 제한이 없음.
#ifdef LJH_ADD_JPN_MODIFYING_QUESTINFO_ABOUT_BC_AND_CT
	// 868 "%d회 입장가능"
#ifdef ASG_MOD_UI_QUEST_INFO
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y+305, GlobalText[868], 190, 0, RT3_SORT_CENTER);
#else	// ASG_MOD_UI_QUEST_INFO
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y+280, GlobalText[868], 190, 0, RT3_SORT_CENTER);
#endif	// ASG_MOD_UI_QUEST_INFO
	
	// 829 "하루 %d회를 초과하여 입장할 수 없습니다."
#ifdef ASG_MOD_UI_QUEST_INFO
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y+325, GlobalText[829], 190, 0, RT3_SORT_CENTER);
#else	// ASG_MOD_UI_QUEST_INFO
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y+300, GlobalText[829], 190, 0, RT3_SORT_CENTER);
#endif	// ASG_MOD_UI_QUEST_INFO

#else	//LJH_ADD_JPN_MODIFYING_QUESTINFO_ABOUT_BC_AND_CT
	// 868 "%d회 입장가능"
	unicode::t_char strText[256];
	unicode::_sprintf(strText, GlobalText[868], g_csQuest.GetEventCount(3));
#ifdef ASG_MOD_UI_QUEST_INFO
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y+305, strText, 190, 0, RT3_SORT_CENTER);
#else	// ASG_MOD_UI_QUEST_INFO
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y+280, strText, 190, 0, RT3_SORT_CENTER);
#endif	// ASG_MOD_UI_QUEST_INFO
	
	// 829 "하루 %d회를 초과하여 입장할 수 없습니다."
	unicode::_sprintf(strText, GlobalText[829], 6);
#ifdef ASG_MOD_UI_QUEST_INFO
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y+325, strText, 190, 0, RT3_SORT_CENTER);
#else	// ASG_MOD_UI_QUEST_INFO
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y+300, strText, 190, 0, RT3_SORT_CENTER);
#endif	// ASG_MOD_UI_QUEST_INFO

#endif	//LJH_ADD_JPN_MODIFYING_QUESTINFO_ABOUT_BC_AND_CT
}

#ifndef ASG_MOD_UI_QUEST_INFO	// 정리시 삭제.
void SEASON3B::CNewUIMyQuestInfoWindow::RenderButtons()
{
	m_BtnTab.Render();
	m_BtnExit.Render();
}
#endif	// ASG_MOD_UI_QUEST_INFO

void SEASON3B::CNewUIMyQuestInfoWindow::OpenningProcess()
{
	g_csQuest.ShowQuestPreviewWindow(-1);
}

void SEASON3B::CNewUIMyQuestInfoWindow::ClosingProcess()
{
#ifdef ASG_MOD_UI_QUEST_INFO
	UnselectQuestList();
#endif	// ASG_MOD_UI_QUEST_INFO
	SendExitInventory();
#ifdef ASG_MOD_UI_QUEST_INFO
	::PlayBuffer(SOUND_CLICK01);
#endif	// ASG_MOD_UI_QUEST_INFO
}

void SEASON3B::CNewUIMyQuestInfoWindow::SetButtonInfo()
{
#ifndef ASG_MOD_UI_QUEST_INFO	// 정리시 삭제.
	// Tab Button	
	// 탭버튼 Text설정
	list<unicode::t_string> TextList;
	// 1140 "퀘스트"
	TextList.push_back( GlobalText[1140] );
	// 1142 "캐슬"
	TextList.push_back( GlobalText[1142] );
	// 2369 "환영 사원"
	TextList.push_back( GlobalText[2369] );
	m_BtnTab.CreateRadioGroup( 3, IMAGE_MYQUEST_TAB_BUTTON );
	m_BtnTab.ChangeRadioText(TextList);
	m_BtnTab.ChangeRadioButtonInfo( true, m_Pos.x+12.f, m_Pos.y+202.f, 56, 22 );
	m_BtnTab.ChangeFrame( TAB_QUEST );
#endif	// ASG_MOD_UI_QUEST_INFO

	// Exit Button
	m_BtnExit.ChangeButtonImgState(true, IMAGE_MYQUEST_BTN_EXIT, false);
	m_BtnExit.ChangeButtonInfo( m_Pos.x+13, m_Pos.y+392, 36, 29 );		
	m_BtnExit.ChangeToolTipText(GlobalText[1002], true);	// 1002 "닫기"

#ifdef ASG_MOD_UI_QUEST_INFO
	m_btnQuestOpen.ChangeButtonImgState(true, IMAGE_MYQUEST_BTN_OPEN, false);
	m_btnQuestOpen.ChangeButtonInfo(m_Pos.x+50, m_Pos.y+392, 36, 29);		
	m_btnQuestOpen.ChangeToolTipText(GlobalText[2822], true);	// 2822 "전직"

	m_btnQuestGiveUp.ChangeButtonImgState(true, IMAGE_MYQUEST_BTN_GIVE_UP, false);
	m_btnQuestGiveUp.ChangeButtonInfo(m_Pos.x+87, m_Pos.y+392, 36, 29);		
	m_btnQuestGiveUp.ChangeToolTipText(GlobalText[2823], true);	// 2823 "캐슬 / 사원"
#endif	// ASG_MOD_UI_QUEST_INFO
}

#ifdef ASG_MOD_UI_QUEST_INFO
//*****************************************************************************
// 함수 이름 : UpdateTabBtn()
// 함수 설명 : 탭 버튼 업데이트. TAB_NON 외의 경우만 m_eTabBtnIndex에 저장.
// 반환 값	 : TAB_BUTTON_INDEX 타입으로 '퀘스트'탭 클릭 시 TAB_QUEST, '전직'탭
//			  클릭 시 TAB_JOB_CHANGE, '캐슬 / 사원'탭 클릭 시 TAB_CASTLE_TEMPLE
//			  리턴. 그 외 TAB_NON 리턴.
//*****************************************************************************
CNewUIMyQuestInfoWindow::TAB_BUTTON_INDEX CNewUIMyQuestInfoWindow::UpdateTabBtn()
{
	if (!(SEASON3B::IsPress(VK_LBUTTON)))
		return TAB_NON;

	if (!CheckMouseIn(m_Pos.x+10, m_Pos.y+27, 166, 22))
		return TAB_NON;

	if (CheckMouseIn(m_Pos.x+10, m_Pos.y+27, 48, 22))
		m_eTabBtnIndex = TAB_QUEST;
	else if (CheckMouseIn(m_Pos.x+57, m_Pos.y+27, 48, 22))
		m_eTabBtnIndex = TAB_JOB_CHANGE;
	else if (CheckMouseIn(m_Pos.x+104, m_Pos.y+27, 72, 22))
		m_eTabBtnIndex = TAB_CASTLE_TEMPLE;

	::PlayBuffer(SOUND_CLICK01);

	return m_eTabBtnIndex;
}

//*****************************************************************************
// 함수 이름 : RenderTabBtn()
// 함수 설명 : 탭 버튼 렌더.
//*****************************************************************************
void CNewUIMyQuestInfoWindow::RenderTabBtn()
{
	RenderImage(IMAGE_MYQUEST_TAB_BACK, m_Pos.x+10, m_Pos.y+27, 166.f, 22.f);

	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetBgColor(0);

	if (m_eTabBtnIndex == TAB_QUEST)
	{
		RenderImage(IMAGE_MYQUEST_TAB_SMALL, m_Pos.x+10, m_Pos.y+27, 48.f, 22.f);
		g_pRenderText->SetTextColor(255, 255, 255, 255);
		g_pRenderText->RenderText(m_Pos.x+10, m_Pos.y+34, GlobalText[1140], 48, 0, RT3_SORT_CENTER);	// "퀘스트"
		g_pRenderText->SetTextColor(181, 181, 181, 181);
		g_pRenderText->RenderText(m_Pos.x+57, m_Pos.y+35, GlobalText[2821], 48, 0, RT3_SORT_CENTER);	// "전직"
		g_pRenderText->RenderText(m_Pos.x+104, m_Pos.y+35, GlobalText[2824], 72, 0, RT3_SORT_CENTER);	// "캐슬 / 사원"
	}
	else if (m_eTabBtnIndex == TAB_JOB_CHANGE)
	{
		RenderImage(IMAGE_MYQUEST_TAB_SMALL, m_Pos.x+57, m_Pos.y+27, 48.f, 22.f);
		g_pRenderText->SetTextColor(255, 255, 255, 255);
		g_pRenderText->RenderText(m_Pos.x+57, m_Pos.y+34, GlobalText[2821], 48, 0, RT3_SORT_CENTER);	// "전직"
		g_pRenderText->SetTextColor(181, 181, 181, 181);
		g_pRenderText->RenderText(m_Pos.x+10, m_Pos.y+35, GlobalText[1140], 48, 0, RT3_SORT_CENTER);	// "퀘스트"
		g_pRenderText->RenderText(m_Pos.x+104, m_Pos.y+35, GlobalText[2824], 72, 0, RT3_SORT_CENTER);	// "캐슬 / 사원"
	}
	else if (m_eTabBtnIndex == TAB_CASTLE_TEMPLE)
	{
		RenderImage(IMAGE_MYQUEST_TAB_BIG, m_Pos.x+104, m_Pos.y+27, 72.f, 22.f);
		g_pRenderText->SetTextColor(255, 255, 255, 255);
		g_pRenderText->RenderText(m_Pos.x+104, m_Pos.y+34, GlobalText[2824], 72, 0, RT3_SORT_CENTER);	// "캐슬 / 사원"
		g_pRenderText->SetTextColor(181, 181, 181, 181);
		g_pRenderText->RenderText(m_Pos.x+10, m_Pos.y+35, GlobalText[1140], 48, 0, RT3_SORT_CENTER);	// "퀘스트"
		g_pRenderText->RenderText(m_Pos.x+57, m_Pos.y+35, GlobalText[2821], 48, 0, RT3_SORT_CENTER);	// "전직"
		
	}
}

//*****************************************************************************
// 함수 이름 : UnselectQuestList()
// 함수 설명 : 퀘스트 리스트를 선택하지 않은 상태로 함.
//*****************************************************************************
void CNewUIMyQuestInfoWindow::UnselectQuestList()
{
	m_CurQuestListBox.SLSetSelectLine(0);
	m_QuestContentsListBox.Clear();
	QuestOpenBtnEnable(false);
	QuestGiveUpBtnEnable(false);
}

//*****************************************************************************
// 함수 이름 : SetCurQuestList()
// 함수 설명 : 진행중인 퀘스트 리스트를 세팅한다.
// 매개 변수 : pDWordList	: 진행중인 퀘스트 인덱스 리스트 주소.
//*****************************************************************************
void CNewUIMyQuestInfoWindow::SetCurQuestList(DWordList* pDWordList)
{
	m_CurQuestListBox.Clear();

	char szInput[64];
	char szOutput[64];
	g_pRenderText->SetFont(g_hFont);

	int i;
	DWordList::iterator iter;
	for (iter = pDWordList->begin(), i = 1; iter != pDWordList->end(); advance(iter, 1), ++i)
	{
		::sprintf(szInput, "%d.%s", i, g_QuestMng.GetSubject(*iter));
		::ReduceStringByPixel(szOutput, 64, szInput, 150);
		m_CurQuestListBox.AddText(*iter, szOutput);
	}

	// 퀘스트 탭이고 진행 가능한 퀘스트 리스트가 없다면 없다고 메시지 표시.
	if (m_eTabBtnIndex == TAB_QUEST && 0 == m_CurQuestListBox.GetLineNum())
		SetMessage(2825);

	// 선택한 퀘스트가 없으므로 퀘스트 내용 리스트 박스 초기화.
	m_QuestContentsListBox.Clear();
	QuestOpenBtnEnable(false);
	QuestGiveUpBtnEnable(false);
}

//*****************************************************************************
// 함수 이름 : SetSelQuestSummary()
// 함수 설명 : 선택한 퀘스트의 제목 요약을 세팅한다.
//*****************************************************************************
void CNewUIMyQuestInfoWindow::SetSelQuestSummary()
{
	m_QuestContentsListBox.Clear();

	// 선택한 퀘스트 인덱스.
	DWORD dwSelQuestIndex = GetSelQuestIndex();

	if (0 == dwSelQuestIndex)
		return;

	// 퀘스트 제목.
	m_QuestContentsListBox.AddText(
		g_hFontBold, 0xff0ab9ff, RT3_SORT_CENTER, g_QuestMng.GetSubject(dwSelQuestIndex));

	// 퀘스트 요약.
	g_pRenderText->SetFont(g_hFont);
	char aszSummary[8][64];
	int nLine = ::DivideStringByPixel(
		&aszSummary[0][0], 8, 64, g_QuestMng.GetSummary(dwSelQuestIndex), 150);
	int i;
	for (i = 0; i < nLine; ++i)
		m_QuestContentsListBox.AddText(g_hFont, 0xffd2e6ff, RT3_SORT_LEFT, aszSummary[i]);
}

//*****************************************************************************
// 함수 이름 : SetSelQuestRequestReward()
// 함수 설명 : 선택한 퀘스트의 요구사항, 보상을 세팅한다.
//*****************************************************************************
void CNewUIMyQuestInfoWindow::SetSelQuestRequestReward()
{
	// 선택한 퀘스트 인덱스.
	DWORD dwSelQuestIndex = GetSelQuestIndex();

	if (0 == dwSelQuestIndex)
		return;

	// 요구사항, 보상 정보가 존재하지 않는 QS(Quest State)라면 리턴.
	if (!g_QuestMng.IsRequestRewardQS(dwSelQuestIndex))
		return;

	// 요구사항, 보상 정보가 없다면 리턴.
	const SQuestRequestReward* pQuestRequestReward = g_QuestMng.GetRequestReward(dwSelQuestIndex);
	if (NULL == pQuestRequestReward)
		return;

	// 요구사항, 보상 텍스트를 얻어온다.
#ifdef ASG_ADD_QUEST_REQUEST_REWARD_TYPE
	SRequestRewardText aRequestRewardText[13];	// 요구사항 제목(1)+요구사항(5)+보상 제목(1)+보상(5)+랜덤보상 제목(1)
	g_QuestMng.GetRequestRewardText(aRequestRewardText, 13, dwSelQuestIndex);
#else	// ASG_ADD_QUEST_REQUEST_REWARD_TYPE
	SRequestRewardText aRequestRewardText[12];
	g_QuestMng.GetRequestRewardText(aRequestRewardText, 12, dwSelQuestIndex);
#endif	// ASG_ADD_QUEST_REQUEST_REWARD_TYPE

	int i = 0;
	int j, nLoop;
#ifdef ASG_ADD_QUEST_REQUEST_REWARD_TYPE
	for (j = 0; j < 3; ++j)
	{
		if (0 == j)			// 요구사항
		{
			m_QuestContentsListBox.AddText(g_hFont, 0xffffffff, RT3_SORT_LEFT, " ");	// 공백 1줄.
			nLoop = 1 + pQuestRequestReward->m_byRequestCount;
		}
		else if (1 == j && pQuestRequestReward->m_byGeneralRewardCount)	// 일반보상
		{
			m_QuestContentsListBox.AddText(g_hFont, 0xffffffff, RT3_SORT_LEFT, " ");	// 공백 1줄.
			nLoop = 1 + pQuestRequestReward->m_byGeneralRewardCount + i;
		}
		else if (2 == j && pQuestRequestReward->m_byRandRewardCount)	// 랜덤보상
		{
			m_QuestContentsListBox.AddText(g_hFont, 0xffffffff, RT3_SORT_LEFT, " ");	// 공백 1줄.
			nLoop = 1 + pQuestRequestReward->m_byRandRewardCount + i;
		}
		else
			nLoop = 0;
#else	// ASG_ADD_QUEST_REQUEST_REWARD_TYPE
	for (j = 0; j < 2; ++j)
	{
		// 공백 1줄.
		m_QuestContentsListBox.AddText(g_hFont, 0xffffffff, RT3_SORT_LEFT, " ");

		// 요구사항(j == 0) 또는 보상(j == 1).
		nLoop = 0 == j
			? 1 + pQuestRequestReward->m_byRequestCount
			: 1 + pQuestRequestReward->m_byRewardCount + i;
#endif	// ASG_ADD_QUEST_REQUEST_REWARD_TYPE

		for (; i < nLoop; ++i)
			m_QuestContentsListBox.AddText(&aRequestRewardText[i], RT3_SORT_CENTER);
	}
}

//*****************************************************************************
// 함수 이름 : QuestOpenBtnEnable()
// 함수 설명 : 퀘스트 열기 버튼을 활성화 또는 비활성화 시킨다.
// 매개 변수 : bEnable	: true이면 활성화.
//*****************************************************************************
void CNewUIMyQuestInfoWindow::QuestOpenBtnEnable(bool bEnable)
{
	if (bEnable)
	{
		m_btnQuestOpen.UnLock();
		m_btnQuestOpen.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
	}
	else
	{
		m_btnQuestOpen.Lock();
		m_btnQuestOpen.ChangeImgColor(BUTTON_STATE_UP, RGBA(100, 100, 100, 255));	
	}
}

//*****************************************************************************
// 함수 이름 : QuestGiveUpBtnLock()
// 함수 설명 : 퀘스트 포기 버튼을 활성화 또는 비활성화 시킨다.
// 매개 변수 : bEnable	: true이면 활성화.
//*****************************************************************************
void CNewUIMyQuestInfoWindow::QuestGiveUpBtnEnable(bool bEnable)
{
	if (bEnable)
	{
		m_btnQuestGiveUp.UnLock();
		m_btnQuestGiveUp.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
	}
	else
	{
		m_btnQuestGiveUp.Lock();
		m_btnQuestGiveUp.ChangeImgColor(BUTTON_STATE_UP, RGBA(100, 100, 100, 255));
	}
}

//*****************************************************************************
// 함수 이름 : GetSelQuestIndex()
// 함수 설명 : 진행중인 퀘스트 리스트에서 선택된 퀘스트의 인덱스를 얻는다.
// 반환 값	 : 선택된 퀘스트의 인덱스.
//*****************************************************************************
DWORD CNewUIMyQuestInfoWindow::GetSelQuestIndex()
{
	SCurQuestItem* pCurQuestItem = m_CurQuestListBox.GetSelectedText();
	if (NULL == pCurQuestItem)
		return 0;

	return pCurQuestItem->m_dwIndex;
}

//*****************************************************************************
// 함수 이름 : SetMessage()
// 함수 설명 : m_aszMsg에 GlobalText[]의 문자열을 세팅한다.
// 매개 변수 : nGlobalTextIndex	: GlobalText[]의 인덱스.
//*****************************************************************************
void CNewUIMyQuestInfoWindow::SetMessage(int nGlobalTextIndex)
{
	::memset(&m_aszMsg[0][0], 0, sizeof(char)*2*64);
	g_pRenderText->SetFont(g_hFontBold);
	m_nMsgLine = ::DivideStringByPixel(&m_aszMsg[0][0], 2, 64, GlobalText[nGlobalTextIndex], 140);
}

#endif	// ASG_MOD_UI_QUEST_INFO