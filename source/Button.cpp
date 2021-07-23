//*****************************************************************************
// File: Button.cpp
//
// Desc: implementation of the CButton class.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#include "stdafx.h"
#include "Button.h"
#include "Input.h"

#include "ZzzOpenglUtil.h"

// 텍스트 랜더를 위한 #include. ㅜㅜ
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"

#include "DSPlaySound.h"
#include "UIControls.h"

extern float g_fScreenRate_x;
extern float g_fScreenRate_y;

CButton*	CButton::m_pBtnHeld;		// 붙잡은 버튼의 포인터.

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CButton::CButton() : m_szText(NULL), m_adwTextColorMap(NULL)
{

}

CButton::~CButton()
{
	Release();
}

void CButton::Release()
{
	ReleaseText();
}

//*****************************************************************************
// 함수 이름 : Create()
// 함수 설명 : 버튼 생성.
//			   버튼에 사용될 텍스처는 각 이미지가 세로로 배열되어 있어야함.
//			   이미지 번호는 맨위가 0번임.
//			   버튼에는 Push버튼과 Check버튼이 있는데 Push버튼은 nDisableFrame
//			  까지만 사용됨.
//			   체크버튼 여부는 m_anImgMap[BTN_UP_CHECK]가 0 이상이면 체크 버튼
//			  임.
// 매개 변수 : nWidth			: 버튼의 픽셀 너비.
//			   nHeight			: 버튼의 픽셀 높이.
//			   nTexID			: 텍스처 ID.
//			   nMaxFrame		: 텍스처 내의 이미지 개수.(기본값 1)
//			   nDownFrame		: 눌렸을 때의 이미지 번호.(기본값 -1)
//								  음수일 경우 0번 이미지가 사용됨.
//			   nActiveFrame		: 마우스 커서가 버튼 위에 있을때 이미지 번호.
//								  (기본값 -1)
//								  음수일 경우 0번 이미지가 사용됨.
//			   nDisableFrame	: 사용 불가능 상태일때 이미지 번호.(기본값 -1)
//								  음수일 경우 랜더하지 않음.
//			   nCheckUpFrame	: 체크 되었고 평상시에 이미지 번호.(기본값 -1)
//								  음수일 경우 체크 버튼이 아님.
//			   nCheckDownFrame	: 체크 되었고 눌렸을 때의 이미지 번호.
//								  (기본값 -1)
//								  음수일 경우 nCheckUpFrame 이미지 사용.
//			   nCheckActiveFrame: 체크 되었고 마우스 커서가 버튼 위에 있을때 이
//								 미지 번호.(기본값 -1)
//								  음수일 경우 nCheckUpFrame 이미지 사용.
//			   nCheckDisableFrame: 체크 되었고 사용불가능 상태일때 이미지 번호.
//								 (기본값 -1)
//								  음수일 경우 랜더하지 않음.
//*****************************************************************************
void CButton::Create(int nWidth, int nHeight, int nTexID, int nMaxFrame,
					 int nDownFrame, int nActiveFrame, int nDisableFrame,
					 int nCheckUpFrame, int nCheckDownFrame,
					 int nCheckActiveFrame, int nCheckDisableFrame)
{
	Release();

// 기초 클래스 오브젝트 생성.
	// 버튼 텍스처는 각 프레임이 세로로 배열되어 있음.
	SFrameCoord* aFrameCoord = new SFrameCoord[nMaxFrame];
	for (int i = 0; i < nMaxFrame; ++i)
	{
		aFrameCoord[i].nX = 0;
		aFrameCoord[i].nY = nHeight * i;
	}

	CSprite::Create(nWidth, nHeight, nTexID, nMaxFrame, aFrameCoord);

	delete [] aFrameCoord;

	CSprite::SetAction(0, nMaxFrame - 1);	// 이미지를 바꿀 수 있게 범위 지정.

// 버튼 관련 생성 처리.
	// 버튼 이미지 맵핑.
	m_anImgMap[BTN_UP] = 0;
	m_anImgMap[BTN_DOWN] = nDownFrame > -1 ? nDownFrame : 0;
	m_anImgMap[BTN_ACTIVE] = nActiveFrame > -1 ? nActiveFrame : 0;
	m_anImgMap[BTN_DISABLE] = nDisableFrame;

	m_anImgMap[BTN_UP_CHECK] = nCheckUpFrame;
	m_anImgMap[BTN_DOWN_CHECK] = nCheckDownFrame > -1
		? nCheckDownFrame : m_anImgMap[BTN_UP_CHECK];
	m_anImgMap[BTN_ACTIVE_CHECK] = nCheckActiveFrame > -1
		? nCheckActiveFrame : m_anImgMap[BTN_UP_CHECK];
	m_anImgMap[BTN_DISABLE_CHECK] = nCheckDisableFrame;

	m_bClick = m_bCheck = false;
	m_bEnable = m_bActive = true;
}

//*****************************************************************************
// 함수 이름 : Show()
// 함수 설명 : 버튼을 보여줌.
// 매개 변수 : bShow	: true이면 보여줌.
//*****************************************************************************
void CButton::Show(bool bShow)
{
	CSprite::Show(bShow);
	if (!bShow)
		m_bClick = false;
}

//*****************************************************************************
// 함수 이름 : CursorInObject()
// 함수 설명 : m_bActive가 true일 때 버튼 안에 마우스 커서가 있으면 TRUE, 없으
//			  면 FALSE를 리턴.
//*****************************************************************************
BOOL CButton::CursorInObject()
{
	if (!m_bActive)
		return FALSE;

	return CSprite::CursorInObject();
}

//*****************************************************************************
// 함수 이름 : Update()
// 함수 설명 : 버튼의 자동 처리 및 버튼 이미지 선택.
// 매개 변수 : dDeltaTick	: 이전 Update()호출 후부터 지금 Update()까지 시간.
//							  (기본값: 0.0)
//*****************************************************************************
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
void CButton::Update()
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
void CButton::Update(double dDeltaTick)
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
{
	// 보여주지 않을 경우 return.
	if (!CSprite::m_bShow)
		return;

	CInput& rInput = CInput::Instance();
	// 텍스트 렌더시 RenderText3()는 가상 좌표값을 넣어야 된다. 따라서 실제 좌
	//표로 변환하는 과정에서 소숫점 이하가 짤리므로 0.5를 더해주어 오차를 없엔
	//다. 또한 버튼을 눌렀을 때 텍스트를 1픽셀 아래로 그려야 하므로 0.5값에 +1
	//을 해준다. 
	m_fTextAddYPos = 0.5f;

	if (m_bEnable/* && m_bActive*/)	// 사용 가능하고 반응이 가능한 상태이면.
	{
	// 버튼 내부 상태 설정.
		// 마우스 커서가 버튼 위에 있고 마우스 왼쪽 버튼을 눌렀는가?
		if (CursorInObject() && rInput.IsLBtnDn())
			m_pBtnHeld = this;		// 붙잡은 버튼은 자신임.

		m_bClick = false;				// m_bClick(클릭인가?)의 초기화.
		// 클릭은 버튼을 떼는 순간에 이루어져야 하므로 매번 처음에 초기화 함.

		// 마우스 왼쪽 버튼을 뗐는가?
		if (rInput.IsLBtnUp())
		{
			// 마우스 커서가 버튼 위에 있고 붙잡은 버튼이 자신인가? (1번 if)
			if (CursorInObject() && this == m_pBtnHeld)
			{
				m_bClick = true;				// 클릭임.

				::PlayBuffer(SOUND_CLICK01);	// 클릭 사운드.

				if (-1 < m_anImgMap[BTN_UP_CHECK])	// 체크 버튼인가?
					m_bCheck = !m_bCheck;		// 토글.
			}

			if (this == m_pBtnHeld)	// 붙잡은 버튼이 자신인가? (2번 if)
				m_pBtnHeld = NULL;	// 붙잡은 버튼은 없음.
			// '2번 if'처럼 비교하지 않을 경우, 버튼을 뗐을 때 m_pBtnHeld에
			//는 무조건 NULL이 대입된다. 이와 같을 때 버튼이 여러개일 경우 두번
			//째 버튼을 클릭했을 때 첫번째 버튼의 Update()에서 'm_pBtnHeld =
			//NULL'이므로 두번째 버튼의 Update()에서 '1번 if'가 실패하게 되어
			//두번째 버튼이 클릭되지 않은것으로 인식되는 오류가 발생된다. 따라
			//서 m_pBtnHeld이 this일 때만 m_pBtnHeld에 NULL을 대입해준다.
		}

	// 버튼 이미지 설정.
		// 마우스 커서가 버튼 위에 있고 버튼을 붙잡고 있지 않은가?
		if (CursorInObject() && NULL == m_pBtnHeld)
			if (m_bCheck)	// 체크 상태라면.
			{
				CSprite::SetNowFrame(m_anImgMap[BTN_ACTIVE_CHECK]);
				if (NULL != m_szText)
					m_dwTextColor = m_adwTextColorMap[BTN_ACTIVE_CHECK];
			}
			else			// 체크 상태가 아니면.
			{
				// 현재 동작은 Highlight되고 버튼 Up인 이미지.
				CSprite::SetNowFrame(m_anImgMap[BTN_ACTIVE]);
				if (NULL != m_szText)
					m_dwTextColor = m_adwTextColorMap[BTN_ACTIVE];
			}

		// 마우스 커서가 버튼 위에 있고 붙잡은 버튼이 자신인가?
		else if (CursorInObject() && this == m_pBtnHeld)
		{
			m_fTextAddYPos = 1.5f;	// 텍스트를 1픽셀 밑으로.(오차 보정 +0.5f)

			if (m_bCheck)	// 체크 상태라면.
			{
				CSprite::SetNowFrame(m_anImgMap[BTN_DOWN_CHECK]);
				if (NULL != m_szText)
					m_dwTextColor = m_adwTextColorMap[BTN_DOWN_CHECK];
			}
			else			// 체크 상태가 아니면.
			{
				// 현재 동작은 Highlight되고 버튼 Down인 이미지.
				CSprite::SetNowFrame(m_anImgMap[BTN_DOWN]);
				if (NULL != m_szText)
					m_dwTextColor = m_adwTextColorMap[BTN_DOWN];
			}
		}

		// 그 외 경우는.(마우스 커서가 버튼 위에 있지 않은 상태)
		else
			if (m_bCheck)	// 체크 상태라면.
			{
				CSprite::SetNowFrame(m_anImgMap[BTN_UP_CHECK]);
				if (NULL != m_szText)
					m_dwTextColor = m_adwTextColorMap[BTN_UP_CHECK];
			}
			else			// 체크 상태가 아니면.
			{
				// 현재 동작은 Highlight되지 않고 버튼 Up인 이미지.
				CSprite::SetNowFrame(m_anImgMap[BTN_UP]);
				if (NULL != m_szText)
					m_dwTextColor = m_adwTextColorMap[BTN_UP];
			}
	}
	else	// 사용 불가능이면.
	{
		m_bClick = false;
		if (rInput.IsLBtnUp() && this == m_pBtnHeld)
			m_pBtnHeld = NULL;	// 붙잡은 버튼은 없음.

		if (m_bCheck)	// 체크 상태라면.
		{
			CSprite::SetNowFrame(m_anImgMap[BTN_DISABLE_CHECK]);
			if (NULL != m_szText)
				m_dwTextColor = m_adwTextColorMap[BTN_DISABLE_CHECK];
		}
		else			// 체크 상태가 아니면.
		{
			CSprite::SetNowFrame(m_anImgMap[BTN_DISABLE]);
			if (NULL != m_szText)
				m_dwTextColor = m_adwTextColorMap[BTN_DISABLE];
		}
	}

//	CSprite::Update(dDeltaTick);	// 버튼 Animation.
}

//*****************************************************************************
// 함수 이름 : Render()
// 함수 설명 : 버튼 렌더.
//*****************************************************************************
void CButton::Render()
{
	if (!CSprite::m_bShow)
		return;

	// 사용 불가능 상태에서 해당 이미지 번호가 음수이면 랜더를 안함.
	if (!m_bEnable)
	{
		if (m_bCheck && m_anImgMap[BTN_DISABLE_CHECK] < 0)
			return;
		if (!m_bCheck && m_anImgMap[BTN_DISABLE] < 0)
			return;
	}

	CSprite::Render();

// 텍스트 렌더.
	if (NULL == m_szText)
		return;

	// 텍스트 색상 지정.
	g_pRenderText->SetTextColor(m_dwTextColor);
	g_pRenderText->SetBgColor(0);
	g_pRenderText->SetFont(g_hFixFont);

	// 텍스트 상대 좌표 구함.
	SIZE size;
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), m_szText, ::strlen(m_szText), &size);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	unicode::_GetTextExtentPoint32(g_pRenderText->GetFontDC(), m_szText, ::strlen(m_szText), &size);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE

	float fTextRelativeYPos = ((CSprite::GetHeight() - size.cy) / 2.0f);

	// 기존 텍스트 렌더 함수를 사용하므로 좌표를 가상 좌표로 변환 해주어야 한다.
	g_pRenderText->RenderText(int(CSprite::GetXPos() / g_fScreenRate_x),
		int(((float)CSprite::GetYPos() + fTextRelativeYPos) / g_fScreenRate_y
		+ m_fTextAddYPos), m_szText, CSprite::GetWidth() / g_fScreenRate_x, 0, RT3_SORT_CENTER);
}

//*****************************************************************************
// 함수 이름 : ReleaseText()
// 함수 설명 : 텍스트 관련 Release.
//*****************************************************************************
void CButton::ReleaseText()
{
	SAFE_DELETE_ARRAY(m_szText);
	SAFE_DELETE_ARRAY(m_adwTextColorMap);
}

//*****************************************************************************
// 함수 이름 : SetText()
// 함수 설명 : 버튼에 텍스트를 씀.
//			   색상은 ARGB() 이용. 색상은 DWORD 타잎의 배열임.
//			   체크 버튼인 경우 색상 배열 크기가 8, 아닌경우 4.
// 매개 변수 : pszText	: 문자열.
//			   adwColor	: 색상 배열 이름(포인터).
//*****************************************************************************
void CButton::SetText(const char* pszText, DWORD* adwColor)
{
	ReleaseText();

	// 문자열 카피.
	m_szText = new char[::strlen(pszText) + 1];
	::strcpy(m_szText, pszText);

	// 텍스트 컬러 개수.
	int nTextColor = -1 < m_anImgMap[BTN_UP_CHECK]	// 체크 버튼인가?
		? BTN_IMG_MAX : BTN_IMG_MAX / 2;

	// 문자색 배열 카피.
	m_adwTextColorMap = new DWORD[nTextColor];
	::memcpy(m_adwTextColorMap, adwColor, sizeof(DWORD) * nTextColor);
}