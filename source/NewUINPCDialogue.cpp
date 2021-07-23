//*****************************************************************************
// File: NewUINPCDialogue.cpp
//
// Desc: implementation of the CNewUINPCDialogue class.
//
// producer: Ahn Sang-Gyu
//*****************************************************************************

#include "stdafx.h"
#include "NewUINPCDialogue.h"
#include "wsclientinline.h"
#include "CRTDBG.h"

#ifdef ASG_ADD_UI_NPC_DIALOGUE

using namespace SEASON3B;

#define ND_NPC_MAX_LINE_PER_PAGE		7	// NPC 대사 페이지당 최대 줄 수.
#define ND_SEL_TEXT_MAX_LINE_PER_PAGE	11	// 선택문 페이지당 최대 줄 수.
#define ND_TEXT_GAP						15	// 텍스트 간격.

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNewUINPCDialogue::CNewUINPCDialogue()
{
	m_pNewUIMng = NULL;
	m_Pos.x = m_Pos.y = 0;
	m_dwContributePoint = 0;
}

CNewUINPCDialogue::~CNewUINPCDialogue()
{
	Release();
}

//*****************************************************************************
// 함수 이름 : Create()
// 함수 설명 : NPC 대화 창 생성.
// 매개 변수 : pNewUIMng	: CNewUIManager 오브젝트 주소.
//			   x			: x 좌표.
//			   y			: y 좌표.
//*****************************************************************************
bool CNewUINPCDialogue::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if (NULL == pNewUIMng)
		return false;

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_NPC_DIALOGUE, this);

	SetPos(x, y);

	LoadImages();

	// 대화 진행 L,R 버튼.
	m_btnProgressL.ChangeButtonImgState(true, IMAGE_ND_BTN_L);
	m_btnProgressL.ChangeButtonInfo(x + 131, y + 165, 17, 18);
	m_btnProgressR.ChangeButtonImgState(true, IMAGE_ND_BTN_R);
	m_btnProgressR.ChangeButtonInfo(x + 153, y + 165, 17, 18);

	// 선택문 페이지 넘기기 L,R 버튼.
	m_btnSelTextL.ChangeButtonImgState(true, IMAGE_ND_BTN_L);
	m_btnSelTextL.ChangeButtonInfo(x + 131, y + 372, 17, 18);
	m_btnSelTextR.ChangeButtonImgState(true, IMAGE_ND_BTN_R);
	m_btnSelTextR.ChangeButtonInfo(x + 153, y + 372, 17, 18);

	// 닫기 버튼.
	m_btnClose.ChangeButtonImgState(true, IMAGE_ND_BTN_CLOSE);
	m_btnClose.ChangeButtonInfo(x + 13, y + 392, 36, 29);
	m_btnClose.ChangeToolTipText(GlobalText[1002], true);

	m_nSelTextCount = 0;
	m_bQuestListMode = false;

	Show(false);

	return true;
}

//*****************************************************************************
// 함수 이름 : Release()
// 함수 설명 : 창 Release.
//*****************************************************************************
void CNewUINPCDialogue::Release()
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
void CNewUINPCDialogue::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;

	m_btnProgressL.SetPos(x + 131, y + 165);			// 대화 진행 L 버튼.
	m_btnProgressR.SetPos(x + 153, y + 165);			// 대화 진행 R 버튼.
	m_btnSelTextL.SetPos(x + 131, y + 372);			// 대화 진행 L 버튼.
	m_btnSelTextR.SetPos(x + 153, y + 372);			// 대화 진행 R 버튼.
	m_btnClose.SetPos(x + 13, y + 392);
}

//*****************************************************************************
// 함수 이름 : UpdateMouseEvent()
// 함수 설명 : 마우스 이벤트 처리.
// 반환 값	 : true면 창 뒤로도 이벤트를 처리.
//*****************************************************************************
bool CNewUINPCDialogue::UpdateMouseEvent()
{
	if (ProcessBtns())
		return false;

	if (UpdateSelTextMouseEvent())
		return false;

	// 창 영역 클릭시 하위 UI처리 및 이동 불가
	if(CheckMouseIn(m_Pos.x, m_Pos.y, ND_WIDTH, ND_HEIGHT))
		return false;

	return true;
}

//*****************************************************************************
// 함수 이름 : ProcessBtns()
// 함수 설명 : 버튼 아이템 이벤트 처리.
// 반환 값	 : 처리 했으면 true.
//*****************************************************************************
bool CNewUINPCDialogue::ProcessBtns()
{
	if (m_btnClose.UpdateMouseEvent())
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_NPC_DIALOGUE);
		return true;
	}
	else if (SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(m_Pos.x+169, m_Pos.y+7, 13, 12))
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_NPC_DIALOGUE);
		return true;
	}
	else if (m_btnProgressR.UpdateMouseEvent())
	{
		m_nSelNPCPage = MIN(++m_nSelNPCPage, m_nMaxNPCPage);
		if (m_nSelNPCPage == m_nMaxNPCPage)
		{
			if (NON_SEL_TEXTS_MODE == m_eLowerView)
				m_eLowerView = SEL_TEXTS_MODE;
		}
		
		::PlayBuffer(SOUND_CLICK01);
		
		if (m_nSelNPCPage == m_nMaxNPCPage && NON_SEL_TEXTS_MODE != m_eLowerView)
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
	else if (m_btnSelTextR.UpdateMouseEvent())
	{
		m_nSelSelTextPage = MIN(++m_nSelSelTextPage, m_nMaxSelTextPage);
		::PlayBuffer(SOUND_CLICK01);

		if (m_nSelSelTextPage == m_nMaxSelTextPage)
			m_btnSelTextR.Lock();
		if (0 != m_nMaxSelTextPage)
			m_btnSelTextL.UnLock();
		
		return true;
	}
	else if (m_btnSelTextL.UpdateMouseEvent())
	{
		m_nSelSelTextPage = MAX(--m_nSelSelTextPage, 0);
		::PlayBuffer(SOUND_CLICK01);

		if (0 == m_nSelSelTextPage)
			m_btnSelTextL.Lock();
		m_btnSelTextR.UnLock();
		
		return true;
	}

	return false;
}

//*****************************************************************************
// 함수 이름 : UpdateSelTextMouseEvent()
// 함수 설명 : 선택문 마우스 이벤트 처리.
// 반환 값	 : 처리 했으면 true.
//*****************************************************************************
bool CNewUINPCDialogue::UpdateSelTextMouseEvent()
{
	if (SEL_TEXTS_MODE != m_eLowerView || !m_bCanClick)
		return false;

	m_nSelSelText = 0;
	if (MouseX < m_Pos.x+11 || MouseX > m_Pos.x+179)
		return false;

	int i;

	int nStartSelText = 0;	// 검사를 시작할 선택문.
	for (i = 0; i < m_nSelSelTextPage; ++i)
		nStartSelText += m_anSelTextCountPerPage[i];

	int nEndSelText = nStartSelText + m_anSelTextCountPerPage[m_nSelSelTextPage];

	int nTopY;
	int nBottomY = m_Pos.y+203;
	for (i = nStartSelText; i < nEndSelText; ++i)
	{
		nTopY = nBottomY;
		nBottomY += m_anSelTextLine[i] * ND_TEXT_GAP;

		if (nTopY <= MouseY && MouseY < nBottomY)
		{
			m_nSelSelText = i + 1;
			if (SEASON3B::IsRelease(VK_LBUTTON))
			{
				m_bCanClick = false;
				ProcessSelTextResult();
				::PlayBuffer(SOUND_CLICK01);
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
bool CNewUINPCDialogue::UpdateKeyEvent()
{
	if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPC_DIALOGUE))
	{
		if (SEASON3B::IsPress(VK_ESCAPE))
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_NPC_DIALOGUE);
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
bool CNewUINPCDialogue::Update()
{
	return true;
}

//*****************************************************************************
// 함수 이름 : Render()
// 함수 설명 : 창 렌더.
// 반환 값	 : true면 성공.
//*****************************************************************************
bool CNewUINPCDialogue::Render()
{
	::EnableAlphaTest();

	::glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	RenderBackImage();
	RenderSelTextBlock();
	RenderText();
#ifdef ASG_ADD_GENS_SYSTEM
	RenderContributePoint();
#endif	// ASG_ADD_GENS_SYSTEM

	if (!m_btnProgressL.IsLock())
		m_btnProgressL.Render();
	if (!m_btnProgressR.IsLock())
		m_btnProgressR.Render();

	if (!m_btnSelTextL.IsLock())
		m_btnSelTextL.Render();
	if (!m_btnSelTextR.IsLock())
		m_btnSelTextR.Render();

	m_btnClose.Render();

	::DisableAlphaBlend();
	
	return true;
}

//*****************************************************************************
// 함수 이름 : RenderBackImage()
// 함수 설명 : 창 바탕 이미지 렌더.
//*****************************************************************************
void CNewUINPCDialogue::RenderBackImage()
{
	RenderImage(IMAGE_ND_BACK, m_Pos.x, m_Pos.y, float(ND_WIDTH), float(ND_HEIGHT));
	RenderImage(IMAGE_ND_TOP, m_Pos.x, m_Pos.y, float(ND_WIDTH), 64.f);
	RenderImage(IMAGE_ND_LEFT, m_Pos.x, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_ND_RIGHT, m_Pos.x+ND_WIDTH-21, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_ND_BOTTOM, m_Pos.x, m_Pos.y+ND_HEIGHT-45, float(ND_WIDTH), 45.f);

	RenderImage(IMAGE_ND_LINE, m_Pos.x+1, m_Pos.y+181, 188.f, 21.f);
}

//*****************************************************************************
// 함수 이름 : RenderSelTextBlock()
// 함수 설명 : 선택문 블럭 렌더.
//*****************************************************************************
void CNewUINPCDialogue::RenderSelTextBlock()
{
	if (SEL_TEXTS_MODE != m_eLowerView)
		return;

	if (0 == m_nSelSelText)
		return;

	int i;

	int nStartSelText = 0;	// 검사를 시작할 선택문.
	for (i = 0; i < m_nSelSelTextPage; ++i)
		nStartSelText += m_anSelTextCountPerPage[i];	

	int nBlockPosY = m_Pos.y+203;

	for (i = nStartSelText; i < m_nSelSelText - 1; ++i)
		nBlockPosY += ND_TEXT_GAP * m_anSelTextLine[i];

	::glColor4f(0.5f, 0.7f, 0.3f, 0.5f);
	::RenderColor(m_Pos.x+11, nBlockPosY, 168.f, ND_TEXT_GAP * m_anSelTextLine[m_nSelSelText - 1]);
	::EndRenderColor();
}

//*****************************************************************************
// 함수 이름 : RenderText()
// 함수 설명 : 텍스트 렌더.
//*****************************************************************************
void CNewUINPCDialogue::RenderText()
{
	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetBgColor(0);

// NPC 이름.
	g_pRenderText->SetTextColor(150, 255, 240, 255);
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y+12, g_QuestMng.GetNPCName(),
		ND_WIDTH, 0, RT3_SORT_CENTER);

	g_pRenderText->SetFont(g_hFont);
// NPC 대사.
	g_pRenderText->SetTextColor(255, 230, 210, 255);
	int i;
	for (i = 0; i < ND_NPC_MAX_LINE_PER_PAGE; ++i)
		g_pRenderText->RenderText(m_Pos.x+13, m_Pos.y+59+(ND_TEXT_GAP*i),
			m_aszNPCWords[i + ND_NPC_MAX_LINE_PER_PAGE * m_nSelNPCPage],
			0, 0, RT3_SORT_LEFT);

// 플래이어 대사.
	if (SEL_TEXTS_MODE == m_eLowerView)
	{
		g_pRenderText->SetTextColor(255, 230, 210, 255);
		
		int nStartSelTextLine = 0;	// 검사를 시작할 선택문.
		for (i = 0; i < m_nSelSelTextPage; ++i)
			nStartSelTextLine += m_anSelTextLinePerPage[i];

		for (i = 0; i < m_anSelTextLinePerPage[m_nSelSelTextPage]; ++i)
			g_pRenderText->RenderText(m_Pos.x+13, m_Pos.y+207+(ND_TEXT_GAP*i),
				m_aszSelTexts[nStartSelTextLine+i], 0, 0, RT3_SORT_LEFT);
	}
}

#ifdef ASG_ADD_GENS_SYSTEM
//*****************************************************************************
// 함수 이름 : RenderText()
// 함수 설명 : 기여도 렌더.
//*****************************************************************************
void CNewUINPCDialogue::RenderContributePoint()
{
	// 자신과 같은 세력의 겐스 NPC인 경우에만.
	if ((543 == g_QuestMng.GetNPCIndex() && 1 == Hero->m_byGensInfluence)
		|| (544 == g_QuestMng.GetNPCIndex() && 2 == Hero->m_byGensInfluence))
	{
		RenderImage(IMAGE_ND_CONTRIBUTE_BG, m_Pos.x+11, m_Pos.y+27, 168.f, 18.f);

		char szContribute[32];
		::sprintf(szContribute, GlobalText[2986], m_dwContributePoint);	// 2986 "획득기여도:%u"
		g_pRenderText->SetTextColor(255, 230, 210, 255);
		g_pRenderText->RenderText(m_Pos.x, m_Pos.y+30, szContribute, ND_WIDTH, 0, RT3_SORT_CENTER);
	}
}
#endif	// ASG_ADD_GENS_SYSTEM

bool CNewUINPCDialogue::IsVisible() const
{ return CNewUIObj::IsVisible(); }

//*****************************************************************************
// 함수 이름 : GetLayerDepth()
// 함수 설명 : 창의 레이어값을 얻음.
//*****************************************************************************
float CNewUINPCDialogue::GetLayerDepth()
{
	return 3.1f;
}

//*****************************************************************************
// 함수 이름 : LoadImages()
// 함수 설명 : 이미지 리소스 로드.
//*****************************************************************************
void CNewUINPCDialogue::LoadImages()
{
	LoadBitmap("Interface\\newui_msgbox_back.jpg", IMAGE_ND_BACK, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back04.tga", IMAGE_ND_TOP, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-L.tga", IMAGE_ND_LEFT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-R.tga", IMAGE_ND_RIGHT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back03.tga", IMAGE_ND_BOTTOM, GL_LINEAR);

	LoadBitmap("Interface\\newui_myquest_Line.tga", IMAGE_ND_LINE, GL_LINEAR);
	LoadBitmap("Interface\\Quest_bt_L.tga", IMAGE_ND_BTN_L, GL_LINEAR);
	LoadBitmap("Interface\\Quest_bt_R.tga", IMAGE_ND_BTN_R, GL_LINEAR);
	LoadBitmap("Interface\\newui_exit_00.tga", IMAGE_ND_BTN_CLOSE, GL_LINEAR);
#ifdef ASG_ADD_GENS_SYSTEM
	LoadBitmap("Interface\\Gens_point.tga", IMAGE_ND_CONTRIBUTE_BG, GL_LINEAR);
#endif	// ASG_ADD_GENS_SYSTEM
}

//*****************************************************************************
// 함수 이름 : UnloadImages()
// 함수 설명 : 이미지 리소스 삭제.
//*****************************************************************************
void CNewUINPCDialogue::UnloadImages()
{
#ifdef ASG_ADD_GENS_SYSTEM
	DeleteBitmap(IMAGE_ND_CONTRIBUTE_BG);
#endif	// ASG_ADD_GENS_SYSTEM
	DeleteBitmap(IMAGE_ND_BTN_CLOSE);
	DeleteBitmap(IMAGE_ND_BTN_R);
	DeleteBitmap(IMAGE_ND_BTN_L);
	DeleteBitmap(IMAGE_ND_LINE);

	DeleteBitmap(IMAGE_ND_BOTTOM);
	DeleteBitmap(IMAGE_ND_RIGHT);
	DeleteBitmap(IMAGE_ND_LEFT);
	DeleteBitmap(IMAGE_ND_TOP);
	DeleteBitmap(IMAGE_ND_BACK);
}

//*****************************************************************************
// 함수 이름 : ProcessOpening()
// 함수 설명 : 창을 열 때 처리.
//*****************************************************************************
void CNewUINPCDialogue::ProcessOpening()
{
	m_bQuestListMode = false;
	SetContents(0);
	::PlayBuffer(SOUND_INTERFACE01);
}

//*****************************************************************************
// 함수 이름 : ProcessClosing()
// 함수 설명 : 창을 닫을 때 처리.
// 반환 값	 : true 면 닫기 성공.
//*****************************************************************************
bool CNewUINPCDialogue::ProcessClosing()
{
	m_dwCurDlgIndex = 0;
	m_dwContributePoint = 0;
	m_bQuestListMode = false;
	SendExitInventory();
	::PlayBuffer(SOUND_CLICK01);
	return true;
}

//*****************************************************************************
// 함수 이름 : SetContents()
// 매개 변수 : dwDlgIndex	: 대화 인덱스
// 함수 설명 : 창의 내용 세팅.(퀘스트 리스트인 경우 제외.)
//*****************************************************************************
void CNewUINPCDialogue::SetContents(DWORD dwDlgIndex)
{
	m_dwCurDlgIndex = dwDlgIndex;
	SetCurNPCWords();
	SetCurSelTexts();
	m_bCanClick = true;
	m_btnProgressL.Lock();
	m_btnSelTextL.Lock();
	m_nSelSelText = 0;
}

//*****************************************************************************
// 함수 이름 : SetCurNPCWords()
// 함수 설명 : 현재 대화 인덱스의 NPC 대사 세팅.
// 매개 변수 : nQuestListCount	: 퀘스트 리스트 개수.(기본값 0)
//*****************************************************************************
void CNewUINPCDialogue::SetCurNPCWords(int nQuestListCount)
{
	::memset(m_aszNPCWords[0], 0, sizeof(char) * ND_NPC_LINE_MAX * ND_WORDS_ROW_MAX);

	g_pRenderText->SetFont(g_hFont);	// DivideStringByPixel()함수 전에 폰트를 설정해주어야 함.
	const char* pszSrc;
	if (m_bQuestListMode)	// 퀘스트 리스트 모드인가?
		// 리스트가 있을 때 대사와 없을 때 대사 선택.(QuestWords.txt의 1501,1502번)
		pszSrc = 0 < nQuestListCount ? g_QuestMng.GetWords(1501) : g_QuestMng.GetWords(1502);
	else
		pszSrc = g_QuestMng.GetNPCDlgNPCWords(m_dwCurDlgIndex);

	int nLine = ::DivideStringByPixel(&m_aszNPCWords[0][0], ND_NPC_LINE_MAX, ND_WORDS_ROW_MAX,
		pszSrc, 160);

	if (1 > nLine)
		return;

	m_nMaxNPCPage = (nLine - 1) / ND_NPC_MAX_LINE_PER_PAGE;
	if (1 <= m_nMaxNPCPage)
	{
	// NPC 대사가 2페이지 이상이면 선택문을 표시 안하고, 우측 진행 버튼을 나타나게 함.
		m_eLowerView = NON_SEL_TEXTS_MODE;
		m_btnProgressR.UnLock();
	}
	else
	{
		m_eLowerView = SEL_TEXTS_MODE;
		m_btnProgressR.Lock();
	}

	m_nSelNPCPage = 0;
}

//*****************************************************************************
// 함수 이름 : SetCurSelTexts()
// 함수 설명 : 현재 대화 인덱스의 선택문 세팅.
//*****************************************************************************
void CNewUINPCDialogue::SetCurSelTexts()
{
	::memset(m_aszSelTexts[0], 0, sizeof(char) * ND_SEL_TEXT_LINE_MAX * ND_WORDS_ROW_MAX);
	::memset(m_anSelTextLine, 0, sizeof(int) * (ND_QUEST_INDEX_MAX_COUNT+1));

	g_pRenderText->SetFont(g_hFont);	// DivideStringByPixel()함수 전에 폰트를 설정해주어야 함.

	char szAnswer[2 * ND_WORDS_ROW_MAX];
	const char* pszAnswer;
	int nSelTextLineSum = 0;	// 선택문 줄 합계.
	int i;
	for (i = 0; i < QM_MAX_ND_ANSWER; ++i)		
	{
	// 선택문 앞에 번호 붙이기.
		::sprintf(szAnswer, "%d. ", i + 1);
		pszAnswer = g_QuestMng.GetNPCDlgAnswer(m_dwCurDlgIndex, i);
		if (NULL == pszAnswer)
			break;
		::strcat(szAnswer, pszAnswer);

	// 행 나누기. 선택문 1개는 최대 2행임.
		m_anSelTextLine[i] = ::DivideStringByPixel(&m_aszSelTexts[nSelTextLineSum][0],
			2, ND_WORDS_ROW_MAX, szAnswer, 160, false);

		nSelTextLineSum += m_anSelTextLine[i];

		if (ND_SEL_TEXT_LINE_MAX <= nSelTextLineSum)
			break;
	}

	m_nSelTextCount = i;

	CalculateSelTextMaxPage(i);
}

//*****************************************************************************
// 함수 이름 : CalculateSelTextMaxPage()
// 함수 설명 : 선택문 최대 페이지 구하기.
// 매개 변수 : nSelTextCount	: 선택문 개수.
//*****************************************************************************
void CNewUINPCDialogue::CalculateSelTextMaxPage(int nSelTextCount)
{
	m_nSelSelTextPage = 0;
	m_nMaxSelTextPage = 0;
	::memset(m_anSelTextLinePerPage, 0, sizeof(int)*ND_SEL_TEXT_PAGE_LIMIT);
	::memset(m_anSelTextCountPerPage, 0, sizeof(int)*ND_SEL_TEXT_PAGE_LIMIT);
	
	int i;
	for (i = 0; i < nSelTextCount; ++i)
	{
		++m_anSelTextCountPerPage[m_nMaxSelTextPage];
		m_anSelTextLinePerPage[m_nMaxSelTextPage] += m_anSelTextLine[i];

		// 페이지당 제한 줄 수가 넘어가면.
		if (m_anSelTextLinePerPage[m_nMaxSelTextPage] > ND_SEL_TEXT_MAX_LINE_PER_PAGE)
		{
			// 현재 페이지에 계산된 걸 빼고 다음 페이지에 계산함.
			--m_anSelTextCountPerPage[m_nMaxSelTextPage];
			m_anSelTextLinePerPage[m_nMaxSelTextPage] -= m_anSelTextLine[i];
			if (ND_SEL_TEXT_PAGE_LIMIT == ++m_nMaxSelTextPage)
				break;
			++m_anSelTextCountPerPage[m_nMaxSelTextPage];
			m_anSelTextLinePerPage[m_nMaxSelTextPage] += m_anSelTextLine[i];
		}
	}
	
	if (1 <= m_nMaxSelTextPage)	// 2페이지 이상이면.
		m_btnSelTextR.UnLock();	// 우측 진행 버튼을 나타나게 함.
	else
		m_btnSelTextR.Lock();
}

//*****************************************************************************
// 함수 이름 : SetQuestListText()
// 함수 설명 : 퀘스트 리스트 세팅.
// 매개 변수 : adwSrcQuestIndex	: 원본 퀘스트 인덱스 배열 주소.
//			   nIndexCount		: 원본 퀘스트 인덱스 개수.
//*****************************************************************************
void CNewUINPCDialogue::SetQuestListText(DWORD* adwSrcQuestIndex, int nIndexCount)
{
	_ASSERT(0 <= nIndexCount && nIndexCount <= ND_QUEST_INDEX_MAX_COUNT);

	::memset(m_adwQuestIndex, 0, sizeof(DWORD)*ND_QUEST_INDEX_MAX_COUNT);
	::memcpy(m_adwQuestIndex, adwSrcQuestIndex, sizeof(DWORD) * nIndexCount);

	m_nSelTextCount = nIndexCount + 1;	// "돌아가기" 선택문 추가로 인한 +1

	::memset(m_aszSelTexts[0], 0, sizeof(char) * ND_SEL_TEXT_LINE_MAX * ND_WORDS_ROW_MAX);
	::memset(m_anSelTextLine, 0, sizeof(int) * (ND_QUEST_INDEX_MAX_COUNT+1));

	char szSelText[2 * ND_WORDS_ROW_MAX];
	const char* pszSelText;
	int nSelTextRow = 0;
	g_pRenderText->SetFont(g_hFont);	// DivideStringByPixel()함수 전에 폰트를 설정해주어야 함.
	int i;
	for (i = 0; i < m_nSelTextCount; ++i)
	{
		// 선택문 앞에 번호 붙이기(퀘스트라면 '[Q]'도 붙임).
		if (m_nSelTextCount - 1 == i)
		{
			// QuestWords.txt의 1007번 "돌아가기"를 마지막 라인에 추가.
			::sprintf(szSelText, "%d. ", i + 1);
			pszSelText = g_QuestMng.GetWords(1007);
		}
		else
		{
			::sprintf(szSelText, "%d. [Q]", i + 1);	// 퀘스트 진행 항목.
			pszSelText = g_QuestMng.GetSubject(m_adwQuestIndex[i]);
		}

		if (NULL == pszSelText)
			break;
		::strcat(szSelText, pszSelText);

	// 행 나누기. 선택문 1개는 최대 2행임.
		m_anSelTextLine[i] = ::DivideStringByPixel(&m_aszSelTexts[nSelTextRow][0],
			2, ND_WORDS_ROW_MAX, szSelText, 160, false);
		
		nSelTextRow += m_anSelTextLine[i];
		
		if (ND_SEL_TEXT_LINE_MAX < nSelTextRow)
			break;
	}

	CalculateSelTextMaxPage(i);
}

//*****************************************************************************
// 함수 이름 : SetContributePoint()
// 함수 설명 : 기여도 세팅.
// 매개 변수 : dwContributePoint	: 기여도.
//*****************************************************************************
void CNewUINPCDialogue::SetContributePoint(DWORD dwContributePoint)
{
	// 겐스 NPC인 경우만.
	if (543 == g_QuestMng.GetNPCIndex() || 544 == g_QuestMng.GetNPCIndex())
		m_dwContributePoint = dwContributePoint;
}

//*****************************************************************************
// 함수 이름 : ProcessSelTextResult()
// 함수 설명 : 선택문 결과값 처리.
//*****************************************************************************
void CNewUINPCDialogue::ProcessSelTextResult()
{
	if (m_bQuestListMode)	// 퀘스트 리스트 모드인가?
	{
		if (m_nSelSelText == m_nSelTextCount)	// 돌아가기를 선택 했다면.
		{
			m_bQuestListMode = false;
			SetContents(0);
		}
		else
			SendQuestSelection(m_adwQuestIndex[m_nSelSelText - 1], (BYTE)m_nSelSelText);
	}
	else
	{
		int nAnswerResult = g_QuestMng.GetNPCDlgAnswerResult(m_dwCurDlgIndex, m_nSelSelText-1);
		if (900 >= nAnswerResult)
		{
			SetContents(nAnswerResult);
		}
		else
		{
			switch (nAnswerResult)
			{
			case 901:	// 퀘스트
				SendRequestQuestByNPCEPList();
				break;

			case 902:	// 공방업
				SendRequestAPDPUp();
				g_pNewUISystem->Hide(SEASON3B::INTERFACE_NPC_DIALOGUE);
				break;

#ifdef ASG_ADD_GENS_SYSTEM
			case 903:	// 겐스 가입(듀프리언)
				SendRequestGensJoining(1);
				break;

			case 904:	// 겐스 가입(바네르트)
				SendRequestGensJoining(2);
				break;

			case 905:	// 겐스 탈퇴
				SendRequestGensSecession();
				break;
#endif	// ASG_ADD_GENS_SYSTEM
#ifdef PBG_ADD_GENSRANKING
			case 906:	// 보상 받기(듀프리언)
				SendRequestGensReward(1);
				break;
			case 907:	// 보상 받기(바네르트)
				SendRequestGensReward(2);
				break;
#endif //PBG_ADD_GENSRANKING

			default:	// "NPC 대화 스크립트 결과값 오류."
				SetContents(999);
			}
		}
	}
}

//*****************************************************************************
// 함수 이름 : ProcessQuestListReceive()
// 함수 설명 : 퀘스트 리스트 요구 서버 응답 처리.
// 매개 변수 : adwSrcQuestIndex	: 원본 퀘스트 인덱스 배열 주소.
//			   nIndexCount		: 원본 퀘스트 인덱스 개수.
//*****************************************************************************
void CNewUINPCDialogue::ProcessQuestListReceive(DWORD* adwSrcQuestIndex, int nIndexCount)
{
	m_bQuestListMode = true;
	SetCurNPCWords(nIndexCount);
	SetQuestListText(adwSrcQuestIndex, nIndexCount);
	m_bCanClick = true;
	m_btnProgressL.Lock();
	m_btnSelTextL.Lock();
	m_nSelSelText = 0;
}

#ifdef ASG_ADD_GENS_SYSTEM
// 겐스 가입 에러 코드.
enum GENS_JOINING_ERR_CODE
{	
	GJEC_NONE_ERR = 0,					// 문제 없다.
	GJEC_REG_GENS_ERR,					// 이미 가입 되어 있다
	GJEC_GENS_SECEDE_DAY_ERR,			// 겐스에서 탈퇴한지 일주일이 안됐다.
	GJEC_REG_GENS_LV_ERR,				// 레벨 50 미만이다.
	GJEC_REG_GENS_NOT_EQL_GUILDMA_ERR,	// 길드장과 같은 겐스가 아니다.
	GJEC_NONE_REG_GENS_GUILDMA_ERR,		// 길드장이 겐스에 가입이 안되어 있다.
	GJEC_PARTY,							// 파티를 맺고 있으면 가입 불가.
#ifdef ASG_FIX_GENS_JOINING_ERR_CODE_ADD
	GJEC_GUILD_UNION_MASTER				// 연합길드장이면 가입 불가.
#endif	// ASG_FIX_GENS_JOINING_ERR_CODE_ADD
};

//*****************************************************************************
// 함수 이름 : ProcessGensJoiningReceive()
// 함수 설명 : 겐스 가입 서버 응답 처리.
// 매개 변수 : byResult		: 겐스 가입 에러 코드(GENS_JOINING_ERR_CODE).
//			   byInfluence	: 겐스 세력.(0:없음, 1:듀프리언, 2:바네르트)
//*****************************************************************************
void CNewUINPCDialogue::ProcessGensJoiningReceive(BYTE byResult, BYTE byInfluence)
{
	switch (byResult)
	{
	case GJEC_NONE_ERR:
		Hero->m_byGensInfluence = byInfluence;
		SetContents(5);
		break;
	case GJEC_REG_GENS_ERR:
		SetContents(9);
		break;
	case GJEC_GENS_SECEDE_DAY_ERR:
		SetContents(11);
		break;
	case GJEC_REG_GENS_LV_ERR:
		SetContents(8);
		break;
	case GJEC_REG_GENS_NOT_EQL_GUILDMA_ERR:
		SetContents(10);
		break;
	case GJEC_NONE_REG_GENS_GUILDMA_ERR:
		SetContents(12);
		break;
	case GJEC_PARTY:
		SetContents(18);
		break;
#ifdef ASG_FIX_GENS_JOINING_ERR_CODE_ADD
	case GJEC_GUILD_UNION_MASTER:
		SetContents(19);
		break;
#endif	// ASG_FIX_GENS_JOINING_ERR_CODE_ADD
	}
}

// 겐스 탈퇴 에러 코드.
enum GENS_SECEDE_ERR_CODE
{	
	GSEC_NONE_ERR = 0,					// 정상탈퇴
	GSEC_IS_NOT_REG_GENS,				// 가입 되어 있지 않다.
	GSEC_GUILD_MASTER_CAN_NOT_SECEDE,	// 길드장은 불가.
	GSEC_IS_NOT_INFLUENCE_NPC			// 같은 세력 NPC가 아님.
};

#ifdef PBG_ADD_GENSRANKING
//겐스 보상 에러 코드
enum GENS_REWARD_ERR_CODE
{
	GENS_REWARD_CALL = 0,				// 보상됨
	GENS_REWARD_TERM,					// 보상 기간아님
	GENS_REWARD_TARGET,					// 보상 대상자아님
	GENS_REWARD_SPACE,					// 보상 공간부족
	GENS_REWARD_ALREADY,				// 이미지급받았다
	GENS_REWARD_DIFFERENT,				// 다른겐스
	GENS_REWARD_NOT_REG,				// 겐스에 가입되어있지 않다
};
#endif //PBG_ADD_GENSRANKING

//*****************************************************************************
// 함수 이름 : ProcessGensSecessionReceive()
// 함수 설명 : 겐스 탈퇴 서버 응답 처리.
// 매개 변수 : byResult	: 겐스 탈퇴 에러 코드(GENS_SECEDE_ERR_CODE).
//*****************************************************************************
void CNewUINPCDialogue::ProcessGensSecessionReceive(BYTE byResult)
{
	switch (byResult)
	{
	case GSEC_NONE_ERR:
		Hero->m_byGensInfluence = 0;
		SetContents(16);
		break;
	case GSEC_IS_NOT_REG_GENS:
		SetContents(15);
		break;
	case GSEC_GUILD_MASTER_CAN_NOT_SECEDE:
		SetContents(14);
		break;
	case GSEC_IS_NOT_INFLUENCE_NPC:
		SetContents(17);
		break;
	}
}
#endif	// ASG_ADD_GENS_SYSTEM

#ifdef PBG_ADD_GENSRANKING
// 보상관련 에러 코드
void CNewUINPCDialogue::ProcessGensRewardReceive(BYTE byResult)
{
	switch(byResult)
	{
	case GENS_REWARD_CALL:
		SetContents(20);
		break;
	case GENS_REWARD_TERM:
		SetContents(21);
		break;
	case GENS_REWARD_TARGET:
		SetContents(22);
		break;
	case GENS_REWARD_SPACE:
		SetContents(23);
		break;
	case GENS_REWARD_ALREADY:
		SetContents(24);
		break;
	case GENS_REWARD_DIFFERENT:
		SetContents(17);
		break;
	case GENS_REWARD_NOT_REG:
#ifdef PBG_FIX_GENSREWARDNOTREG
		SetContents(25);
#else //PBG_FIX_GENSREWARDNOTREG
		SetContents(15);
#endif //PBG_FIX_GENSREWARDNOTREG
		break;
	}
}
#endif //PBG_ADD_GENSRANKING
#endif	// ASG_ADD_UI_NPC_DIALOGUE