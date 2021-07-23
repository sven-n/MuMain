//*****************************************************************************
// File: NewUIQuestProgressByEtc.cpp
//
// Desc: implementation of the CNewUIQuestProgressByEtc class.
//
// producer: Ahn Sang-Gyu
//*****************************************************************************

#include "stdafx.h"
#include "NewUIQuestProgressByEtc.h"
#include "NewUISystem.h"
#include "wsclientinline.h"
#include "UsefulDef.h"

#ifdef ASG_ADD_UI_QUEST_PROGRESS_ETC

using namespace SEASON3B;

#define QPE_NPC_MAX_LINE_PER_PAGE	7	// 페이지당 NPC 대사 최대 줄 수.
#define QPE_TEXT_GAP				15	// 텍스트 간격.
#define QPE_LIST_BOX_LINE_NUM		12	// 요구 사항, 보상 리스트 박스 줄 수.

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNewUIQuestProgressByEtc::CNewUIQuestProgressByEtc()
{
	m_pNewUIMng = NULL;
	m_Pos.x = m_Pos.y = 0;
}

CNewUIQuestProgressByEtc::~CNewUIQuestProgressByEtc()
{
	Release();
}

//*****************************************************************************
// 함수 이름 : Create()
// 함수 설명 : 퀘스트 진행 창 생성.
// 매개 변수 : pNewUIMng	: CNewUIManager 오브젝트 주소.
//			   x			: x 좌표.
//			   y			: y 좌표.
//*****************************************************************************
bool CNewUIQuestProgressByEtc::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if (NULL == pNewUIMng)
		return false;

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC, this);

	SetPos(x, y);

	LoadImages();

#ifdef ASG_ADD_UI_QUEST_PROGRESS
	// 대화 진행 L,R 버튼.
	m_btnProgressL.ChangeButtonImgState(true, IMAGE_QPE_BTN_L);
#endif	// ASG_ADD_UI_QUEST_PROGRESS
	m_btnProgressL.ChangeButtonInfo(x + 131, y + 165, 17, 18);
#ifdef ASG_ADD_UI_QUEST_PROGRESS
	m_btnProgressR.ChangeButtonImgState(true, IMAGE_QPE_BTN_R);
#endif	// ASG_ADD_UI_QUEST_PROGRESS
	m_btnProgressR.ChangeButtonInfo(x + 153, y + 165, 17, 18);

	// 퀘스트 처리 버튼.
	m_btnComplete.ChangeText(GlobalText[2811]);	// "확     인"
	m_btnComplete.ChangeButtonImgState(true, IMAGE_QPE_BTN_COMPLETE, true);
	m_btnComplete.ChangeButtonInfo(x + (QPE_WIDTH - 108) / 2, y + 362, 108, 29);

	// 닫기 버튼.
	m_btnClose.ChangeButtonImgState(true, IMAGE_QPE_BTN_CLOSE);
	m_btnClose.ChangeButtonInfo(x + 13, y + 392, 36, 29);
	m_btnClose.ChangeToolTipText(GlobalText[1002], true);

	// 요구사항, 보상 리스트 박스.
	m_RequestRewardListBox.SetNumRenderLine(QPE_LIST_BOX_LINE_NUM);
	m_RequestRewardListBox.SetSize(174, 158);

	Show(false);

	return true;
}

//*****************************************************************************
// 함수 이름 : Release()
// 함수 설명 : 창 Release.
//*****************************************************************************
void CNewUIQuestProgressByEtc::Release()
{
	UnloadImages();

	if (m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}
}

//*****************************************************************************
// 함수 이름 : SetPos()
// 함수 설명 : 창 위치 지정.
//*****************************************************************************
void CNewUIQuestProgressByEtc::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;

	m_btnProgressL.SetPos(x + 131, y + 165);			// 대화 진행 L 버튼.
	m_btnProgressR.SetPos(x + 153, y + 165);			// 대화 진행 R 버튼.
	m_btnComplete.SetPos(x + (QPE_WIDTH - 108) / 2, y + 362);// 퀘스트 완료 버튼.
	m_btnClose.SetPos(x + 13, y + 392);

	m_RequestRewardListBox.SetPosition(m_Pos.x+9, m_Pos.y+360);
}

//*****************************************************************************
// 함수 이름 : UpdateMouseEvent()
// 함수 설명 : 마우스 이벤트 처리.
// 반환 값	 : true면 창 뒤로도 이벤트를 처리.
//*****************************************************************************
bool CNewUIQuestProgressByEtc::UpdateMouseEvent()
{
	if (REQUEST_REWARD_MODE == m_eLowerView)
		m_RequestRewardListBox.DoAction();

	if (ProcessBtns())
		return false;

	if (UpdateSelTextMouseEvent())
		return false;

	// 창 영역 클릭시 하위 UI처리 및 이동 불가
	if(CheckMouseIn(m_Pos.x, m_Pos.y, QPE_WIDTH, QPE_HEIGHT))
		return false;

	return true;
}

//*****************************************************************************
// 함수 이름 : ProcessBtns()
// 함수 설명 : 버튼 아이템 이벤트 처리.
// 반환 값	 : 처리 했으면 true.
//*****************************************************************************
bool CNewUIQuestProgressByEtc::ProcessBtns()
{
	if (m_btnClose.UpdateMouseEvent())
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC);
		return true;
	}
	else if (SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(m_Pos.x+169, m_Pos.y+7, 13, 12))
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC);
		return true;
	}
	else if (m_btnProgressR.UpdateMouseEvent())
	{
		if (m_nSelNPCPage == m_nMaxNPCPage)
		{
			if (NON_PLAYER_WORDS_MODE == m_eLowerView)
				m_eLowerView = PLAYER_WORDS_MODE;
		}
		else
			m_nSelNPCPage = MIN(++m_nSelNPCPage, m_nMaxNPCPage);
		::PlayBuffer(SOUND_CLICK01);

		if (m_nSelNPCPage == m_nMaxNPCPage && NON_PLAYER_WORDS_MODE != m_eLowerView)
			m_btnProgressR.Lock();
		if (0 != m_nMaxNPCPage)
			m_btnProgressL.UnLock();

		return true;
	}
	else if (m_btnProgressL.UpdateMouseEvent())
	{
		m_nSelNPCPage = MAX(--m_nSelNPCPage, 0);
		::PlayBuffer(SOUND_CLICK01);

		if (0 == m_nSelNPCPage)
			m_btnProgressL.Lock();
		m_btnProgressR.UnLock();

		return true;
	}
	else if (m_bRequestComplete && m_bCanClick)	// 요구 사항을 만족하는가?
	{
		if (m_btnComplete.UpdateMouseEvent())
		{
			SendRequestQuestComplete(m_dwCurQuestIndex);
			::PlayBuffer(SOUND_CLICK01);
			m_bCanClick = false;
			return true;
		}
	}

	return false;
}

//*****************************************************************************
// 함수 이름 : UpdateSelTextMouseEvent()
// 함수 설명 : 선택문 마우스 이벤트 처리.
// 반환 값	 : 처리 했으면 true.
//*****************************************************************************
bool CNewUIQuestProgressByEtc::UpdateSelTextMouseEvent()
{
	if (PLAYER_WORDS_MODE != m_eLowerView || !m_bCanClick)
		return false;

	m_nSelAnswer = 0;
	if (MouseX < m_Pos.x+11 || MouseX > m_Pos.x+179)
		return false;

	int nTopY;
	int nBottomY = m_Pos.y+203;
	int i;
	for (i = 0; i < QM_MAX_ANSWER; ++i)
	{
		nTopY = nBottomY;
		nBottomY += m_anAnswerLine[i] * QPE_TEXT_GAP;

		if (nTopY <= MouseY && MouseY < nBottomY)
		{
			m_nSelAnswer = i + 1;
			if (SEASON3B::IsRelease(VK_LBUTTON))
			{
				SendQuestSelAnswer(m_dwCurQuestIndex, (BYTE)m_nSelAnswer);
				::PlayBuffer(SOUND_CLICK01);
				m_bCanClick = false;
				return true;
			}
			break;
		}
	}

	return false;
}

//*****************************************************************************
// 함수 이름 : UpdateKeyEvent()
// 함수 설명 : 키보드 입력 처리.
// 반환 값	 : true면 창 뒤로도 이벤트를 처리.
//*****************************************************************************
bool CNewUIQuestProgressByEtc::UpdateKeyEvent()
{
	if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC))
	{
		if(SEASON3B::IsPress(VK_ESCAPE))
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC);
			return false;
		}
	}

	return true;
}

//*****************************************************************************
// 함수 이름 : Update()
// 함수 설명 : 기타 매 프레임 일어나는 이벤트 처리.
// 반환 값	 : true면 창 뒤로도 이벤트를 처리.
//*****************************************************************************
bool CNewUIQuestProgressByEtc::Update()
{
	return true;
}

//*****************************************************************************
// 함수 이름 : Render()
// 함수 설명 : 창 렌더.
// 반환 값	 : true면 성공.
//*****************************************************************************
bool CNewUIQuestProgressByEtc::Render()
{
	::EnableAlphaTest();

	::glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	RenderBackImage();
	RenderSelTextBlock();

	RenderText();

	if (!m_btnProgressL.IsLock())
		m_btnProgressL.Render();
	if (!m_btnProgressR.IsLock())
		m_btnProgressR.Render();

	if (REQUEST_REWARD_MODE == m_eLowerView)
	{
		m_btnComplete.Render();
		m_RequestRewardListBox.Render();
		::EnableAlphaTest();	// 위 리스트 박스로 인해 아래의 닫기 버튼의 알파값이 안먹음.
	}

	m_btnClose.Render();

	::DisableAlphaBlend();
	
	return true;
}

//*****************************************************************************
// 함수 이름 : RenderBackImage()
// 함수 설명 : 창 바탕 이미지 렌더.
//*****************************************************************************
void CNewUIQuestProgressByEtc::RenderBackImage()
{
	RenderImage(IMAGE_QPE_BACK, m_Pos.x, m_Pos.y, float(QPE_WIDTH), float(QPE_HEIGHT));
	RenderImage(IMAGE_QPE_TOP, m_Pos.x, m_Pos.y, float(QPE_WIDTH), 64.f);
	RenderImage(IMAGE_QPE_LEFT, m_Pos.x, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_QPE_RIGHT, m_Pos.x+QPE_WIDTH-21, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_QPE_BOTTOM, m_Pos.x, m_Pos.y+QPE_HEIGHT-45, float(QPE_WIDTH), 45.f);

	RenderImage(IMAGE_QPE_LINE, m_Pos.x+1, m_Pos.y+181, 188.f, 21.f);
}

//*****************************************************************************
// 함수 이름 : RenderSelTextBlock()
// 함수 설명 : 선택문 블럭 렌더.
//*****************************************************************************
void CNewUIQuestProgressByEtc::RenderSelTextBlock()
{
	if (PLAYER_WORDS_MODE != m_eLowerView)
		return;

	if (0 == m_nSelAnswer)
		return;

	int nBlockPosY = m_Pos.y+203;
	int i;
	for (i = 0; i < m_nSelAnswer - 1; ++i)
		nBlockPosY += QPE_TEXT_GAP * m_anAnswerLine[i];

	::glColor4f(0.5f, 0.7f, 0.3f, 0.5f);
	::RenderColor(m_Pos.x+11, nBlockPosY, 168.f, QPE_TEXT_GAP * m_anAnswerLine[m_nSelAnswer - 1]);
	::EndRenderColor();
}

//*****************************************************************************
// 함수 이름 : RenderText()
// 함수 설명 : 텍스트 렌더.
//*****************************************************************************
void CNewUIQuestProgressByEtc::RenderText()
{
	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetBgColor(0);

// 제목
	g_pRenderText->SetTextColor(230, 230, 230, 255);
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y+12, "Quest",
		QPE_WIDTH, 0, RT3_SORT_CENTER);
	g_pRenderText->SetTextColor(36, 242, 252, 255);
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y+27,
		g_QuestMng.GetSubject(m_dwCurQuestIndex), QPE_WIDTH, 0, RT3_SORT_CENTER);

	g_pRenderText->SetFont(g_hFont);
// NPC 대사.
	g_pRenderText->SetTextColor(255, 230, 210, 255);
	int i;
	for (i = 0; i < QPE_NPC_MAX_LINE_PER_PAGE; ++i)
		g_pRenderText->RenderText(m_Pos.x+13, m_Pos.y+59+(QPE_TEXT_GAP*i),
			m_aszNPCWords[i + QPE_NPC_MAX_LINE_PER_PAGE * m_nSelNPCPage],
			0, 0, RT3_SORT_LEFT);

// 플래이어 대사.
	if (PLAYER_WORDS_MODE == m_eLowerView)
	{
		g_pRenderText->SetTextColor(255, 230, 210, 255);
		for (i = 0; i < QPE_PLAYER_LINE_MAX; ++i)
			g_pRenderText->RenderText(m_Pos.x+13, m_Pos.y+207+(QPE_TEXT_GAP*i),
				m_aszPlayerWords[i], 0, 0, RT3_SORT_LEFT);
	}
}

bool CNewUIQuestProgressByEtc::IsVisible() const
{ return CNewUIObj::IsVisible(); }

//*****************************************************************************
// 함수 이름 : GetLayerDepth()
// 함수 설명 : 창의 레이어값을 얻음.
//*****************************************************************************
float CNewUIQuestProgressByEtc::GetLayerDepth()
{
	return 3.1f;
}

//*****************************************************************************
// 함수 이름 : LoadImages()
// 함수 설명 : 이미지 리소스 로드.
//*****************************************************************************
void CNewUIQuestProgressByEtc::LoadImages()
{
	LoadBitmap("Interface\\newui_msgbox_back.jpg", IMAGE_QPE_BACK, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back04.tga", IMAGE_QPE_TOP, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-L.tga", IMAGE_QPE_LEFT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-R.tga", IMAGE_QPE_RIGHT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back03.tga", IMAGE_QPE_BOTTOM, GL_LINEAR);

	LoadBitmap("Interface\\newui_myquest_Line.tga", IMAGE_QPE_LINE, GL_LINEAR);
#ifdef ASG_ADD_UI_QUEST_PROGRESS
	LoadBitmap("Interface\\Quest_bt_L.tga", IMAGE_QPE_BTN_L, GL_LINEAR);
	LoadBitmap("Interface\\Quest_bt_R.tga", IMAGE_QPE_BTN_R, GL_LINEAR);
#endif	// ASG_ADD_UI_QUEST_PROGRESS
	LoadBitmap("Interface\\newui_btn_empty.tga", IMAGE_QPE_BTN_COMPLETE, GL_LINEAR);
	LoadBitmap("Interface\\newui_exit_00.tga", IMAGE_QPE_BTN_CLOSE, GL_LINEAR);
}

//*****************************************************************************
// 함수 이름 : UnloadImages()
// 함수 설명 : 이미지 리소스 삭제.
//*****************************************************************************
void CNewUIQuestProgressByEtc::UnloadImages()
{
	DeleteBitmap(IMAGE_QPE_BTN_CLOSE);
	DeleteBitmap(IMAGE_QPE_BTN_COMPLETE);
#ifdef ASG_ADD_UI_QUEST_PROGRESS
	DeleteBitmap(IMAGE_QPE_BTN_R);
	DeleteBitmap(IMAGE_QPE_BTN_L);
#endif	// ASG_ADD_UI_QUEST_PROGRESS
	DeleteBitmap(IMAGE_QPE_LINE);

	DeleteBitmap(IMAGE_QPE_BOTTOM);
	DeleteBitmap(IMAGE_QPE_RIGHT);
	DeleteBitmap(IMAGE_QPE_LEFT);
	DeleteBitmap(IMAGE_QPE_TOP);
	DeleteBitmap(IMAGE_QPE_BACK);
}

//*****************************************************************************
// 함수 이름 : ProcessOpening()
// 함수 설명 : 창을 열 때 처리.
//*****************************************************************************
void CNewUIQuestProgressByEtc::ProcessOpening()
{
	::PlayBuffer(SOUND_INTERFACE01);
}

//*****************************************************************************
// 함수 이름 : ProcessClosing()
// 함수 설명 : 창을 닫을 때 처리.
// 반환 값	 : true 면 닫기 성공.
//*****************************************************************************
bool CNewUIQuestProgressByEtc::ProcessClosing()
{
	g_QuestMng.DelQuestIndexByEtcList(m_dwCurQuestIndex);
	m_dwCurQuestIndex = 0;
	SendExitInventory();
	::PlayBuffer(SOUND_CLICK01);
	return true;
}

//*****************************************************************************
// 함수 이름 : SetContents()
// 함수 설명 : 창의 내용 세팅.
// 매개 변수 : dwQuestIndex	: 퀘스트 인덱스
//*****************************************************************************
void CNewUIQuestProgressByEtc::SetContents(DWORD dwQuestIndex)
{
	if (0 == dwQuestIndex)
		return;

	m_dwCurQuestIndex = dwQuestIndex;

	SetCurNPCWords();
	m_bCanClick = true;

	m_btnProgressL.Lock();

	if (NULL != g_QuestMng.GetAnswer(m_dwCurQuestIndex, 0))		// 선택문이 있다면.	
	{
		SetCurPlayerWords();
		m_eLowerView = NON_PLAYER_WORDS_MODE;
		m_nSelAnswer = 0;

		m_btnProgressR.UnLock();
	}
	else
	{
		SetCurRequestReward();
		m_eLowerView = REQUEST_REWARD_MODE;

		if (0 == m_nMaxNPCPage)					// 최대 페이지 수가 1페이지라면.
			m_btnProgressR.Lock();
		else
			m_btnProgressR.UnLock();
	}
}

//*****************************************************************************
// 함수 이름 : SetCurNPCWords()
// 함수 설명 : 현재 퀘스트 인덱스의 NPC 대사 세팅.
//*****************************************************************************
void CNewUIQuestProgressByEtc::SetCurNPCWords()
{
	if (0 == m_dwCurQuestIndex)
		return;

	::memset(m_aszNPCWords[0], 0, sizeof(char) * QPE_NPC_LINE_MAX * QPE_WORDS_ROW_MAX);

	g_pRenderText->SetFont(g_hFont);	// DivideStringByPixel()함수 전에 폰트를 설정해주어야 함.
	int nLine = ::DivideStringByPixel(&m_aszNPCWords[0][0],
		QPE_NPC_LINE_MAX, QPE_WORDS_ROW_MAX, g_QuestMng.GetNPCWords(m_dwCurQuestIndex), 160);

	if (1 > nLine)
		return;

	m_nMaxNPCPage = (nLine - 1) / QPE_NPC_MAX_LINE_PER_PAGE;
	m_nSelNPCPage = 0;
}

//*****************************************************************************
// 함수 이름 : SetCurPlayerWords()
// 함수 설명 : 현재 퀘스트 인덱스의 플래이어 대사, 선택문 세팅.
//*****************************************************************************
void CNewUIQuestProgressByEtc::SetCurPlayerWords()
{
	if (0 == m_dwCurQuestIndex)
		return;

	::memset(m_aszPlayerWords[0], 0, sizeof(char) * QPE_PLAYER_LINE_MAX * QPE_WORDS_ROW_MAX);
	::memset(m_anAnswerLine, 0, sizeof(int) * QM_MAX_ANSWER);

	g_pRenderText->SetFont(g_hFont);	// DivideStringByPixel()함수 전에 폰트를 설정해주어야 함.

	char szAnswer[2 * QPE_WORDS_ROW_MAX];
	const char* pszAnswer;
	int nPlayerWordsRow = 0;
	int i;
	for (i = 0; i < QM_MAX_ANSWER; ++i)		
	{
	// 선택문 앞에 번호 붙이기.
		::sprintf(szAnswer, "%d.", i + 1);
		pszAnswer = g_QuestMng.GetAnswer(m_dwCurQuestIndex, i);
		if (NULL == pszAnswer)
			break;
		::strcat(szAnswer, pszAnswer);

	// 행 나누기. 선택문 1개는 최대 2행임.
		m_anAnswerLine[i] = ::DivideStringByPixel(&m_aszPlayerWords[nPlayerWordsRow][0],
			2, QPE_WORDS_ROW_MAX, szAnswer, 160, false);

		nPlayerWordsRow += m_anAnswerLine[i];

		if (QPE_PLAYER_LINE_MAX <= nPlayerWordsRow)
			break;
	}
}

//*****************************************************************************
// 함수 이름 : SetCurRequestReward()
// 함수 설명 : 현재 퀘스트 인덱스의 요구사항, 보상 세팅.
//*****************************************************************************
void CNewUIQuestProgressByEtc::SetCurRequestReward()
{
	if (0 == m_dwCurQuestIndex)
		return;

	// 요구사항, 보상 정보가 없다면 리턴.
	const SQuestRequestReward* pQuestRequestReward
		= g_QuestMng.GetRequestReward(m_dwCurQuestIndex);
	if (NULL == pQuestRequestReward)
		return;

	m_RequestRewardListBox.Clear();

	// 요구사항, 보상 텍스트를 얻어온다.
#ifdef ASG_ADD_QUEST_REQUEST_REWARD_TYPE
	SRequestRewardText aRequestRewardText[13];	// 요구사항 제목(1)+요구사항(5)+보상 제목(1)+보상(5)+랜덤보상 제목(1)
	m_bRequestComplete
		= g_QuestMng.GetRequestRewardText(aRequestRewardText, 13, m_dwCurQuestIndex);
#else	// ASG_ADD_QUEST_REQUEST_REWARD_TYPE
	SRequestRewardText aRequestRewardText[12];
	m_bRequestComplete
		= g_QuestMng.GetRequestRewardText(aRequestRewardText, 12, m_dwCurQuestIndex);
#endif	// ASG_ADD_QUEST_REQUEST_REWARD_TYPE

	int i = 0;
	int j, nLoop;
#ifdef ASG_ADD_QUEST_REQUEST_REWARD_TYPE
	for (j = 0; j < 3; ++j)
	{
		if (0 == j)			// 요구사항
		{
			nLoop = 1 + pQuestRequestReward->m_byRequestCount;
		}
		else if (1 == j && pQuestRequestReward->m_byGeneralRewardCount)	// 일반보상
		{
			m_RequestRewardListBox.AddText(g_hFont, 0xffffffff, RT3_SORT_LEFT, " ");	// 공백 1줄.
			nLoop = 1 + pQuestRequestReward->m_byGeneralRewardCount + i;
		}
		else if (2 == j && pQuestRequestReward->m_byRandRewardCount)	// 랜덤보상
		{
			m_RequestRewardListBox.AddText(g_hFont, 0xffffffff, RT3_SORT_LEFT, " ");	// 공백 1줄.
			nLoop = 1 + pQuestRequestReward->m_byRandRewardCount + i;
		}
		else
			nLoop = 0;
#else	// ASG_ADD_QUEST_REQUEST_REWARD_TYPE
	for (j = 0; j < 2; ++j)
	{
		if (0 == j)	// 요구사항
		{
			nLoop = 1 + pQuestRequestReward->m_byRequestCount;
		}
		else		// 보상
		{
			m_RequestRewardListBox.AddText(g_hFont, 0xffffffff, RT3_SORT_LEFT, " ");	// 공백 1줄.
			nLoop = 1 + pQuestRequestReward->m_byRewardCount + i;
		}
#endif	// ASG_ADD_QUEST_REQUEST_REWARD_TYPE

		for (; i < nLoop; ++i)
			m_RequestRewardListBox.AddText(&aRequestRewardText[i], RT3_SORT_CENTER);
	}

#ifdef ASG_MOD_QUEST_OK_BTN_DISABLE
	EnableCompleteBtn(m_bRequestComplete);
#else	// ASG_MOD_QUEST_OK_BTN_DISABLE
	if (m_bRequestComplete)	// 요구사항을 만족하는가?
	{
		m_btnComplete.UnLock();
		m_btnComplete.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
		m_btnComplete.ChangeTextColor(RGBA(255, 230, 210, 255));
	}
	else
	{
		m_btnComplete.Lock();
		m_btnComplete.ChangeImgColor(BUTTON_STATE_UP, RGBA(100, 100, 100, 255));
		m_btnComplete.ChangeTextColor(RGBA(170, 170, 170, 255));
	}
#endif	// ASG_MOD_QUEST_OK_BTN_DISABLE
}

#ifdef ASG_MOD_QUEST_OK_BTN_DISABLE
//*****************************************************************************
// 함수 이름 : EnableCompleteBtn()
// 함수 설명 : 확인 버튼 활성화 또는 비활성화.
// 매개 변수 : bEnable	: 활성화이면 ture.
//*****************************************************************************
void CNewUIQuestProgressByEtc::EnableCompleteBtn(bool bEnable)
{
	if (bEnable)
	{
		m_btnComplete.UnLock();
		m_btnComplete.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
		m_btnComplete.ChangeTextColor(RGBA(255, 230, 210, 255));
	}
	else
	{
		m_btnComplete.Lock();
		m_btnComplete.ChangeImgColor(BUTTON_STATE_UP, RGBA(100, 100, 100, 255));
		m_btnComplete.ChangeTextColor(RGBA(170, 170, 170, 255));
	}
}
#endif	// ASG_MOD_QUEST_OK_BTN_DISABLE

#endif	// ASG_ADD_UI_QUEST_PROGRESS_ETC