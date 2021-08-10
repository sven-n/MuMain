///////////////////////////////////////////////////////////////////////////////
// 
// 
//
// 
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "UIControls.h"
#include "UIWindows.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "ZzzInventory.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "DSPlaySound.h"
#include "wsclientinline.h"
#include "./Utilities/Log/ErrorReport.h"
#include "ReadScript.h"
#include "GMCrywolf1st.h"
#include "CMVP1stDirection.h"
#include "UIManager.h"
#include "NewUISystem.h"


extern BYTE m_CrywolfState;

float g_fScreenRate_x = 0;
float g_fScreenRate_y = 0;
int g_iWidthEx = 5;

///////////////////////////////////////////
extern int g_iChatInputType;

/////////////////////////////////////////////

#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
extern BOOL g_bUseWindowMode;
#endif

#define ARRAY_SIZE(pArray) (sizeof(pArray)/sizeof(pArray[0]))
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef KJH_FIX_BTS158_TEXT_CUT_ROUTINE
int CutStr(const char* pszSrcText, char * pTextOut, const int iTargetPixelWidth, const int iMaxOutLine, const int iOutStrLength, const int iFirstLineTab /* = 0 */)
{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	if(iFirstLineTab < 0)	return 0;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	if (pszSrcText == NULL)
	{
		assert(!"텍스트 자르기 입력값 오류");
		return 0;
	}

#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE

	int iConversionType = (g_pMultiLanguage->IsCharUTF8(pszSrcText)) ? CP_UTF8 : g_pMultiLanguage->GetCodePage();

	std::wstring wstrUTF16 = L"";
	std::string strText = "";
	g_pMultiLanguage->ConvertCharToWideStr(wstrUTF16, pszSrcText);
	int iCharIndex = 0, iLineIndex = 0;
	int iScreenRatePixelWidth = iTargetPixelWidth*g_fScreenRate_x-5;				// 타겟 픽셀 길이를 스크린에 따라 유동적으로 바꾼다.(-5은 여백을 남기기 위함)
	SIZE iSize;


	while (iCharIndex < wstrUTF16.length() && iLineIndex < iMaxOutLine)
	{
		g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), wstrUTF16.c_str(), iCharIndex, &iSize);
		g_pMultiLanguage->ConvertWideCharToStr(strText, (wstrUTF16.substr(0, iCharIndex)).c_str(), iConversionType);
		
		// 첫번째 줄은 넘어온 값(예: 이름 표시 길이)만큼을 뺀 나머지로 계산한다.
		if (iLineIndex == 0) 
			iSize.cx += iFirstLineTab;

		// 1. 현재 글자를 포함했을 때의 문자열의 길이가 제한된 길이보다 크거나 
		// 2. Array 'pTextOut'의 size 보다 클 경우  
		if (iSize.cx >= iScreenRatePixelWidth || strText.length() >= iOutStrLength-1)
		{
			// 빈칸이 있을 경우 바로 전 빈칸까지 
			// 빈칸이 없을 경우 바로 전의 글자까지 UTF-8로 변환한 후 저장
			int iPosLastSpace = wstrUTF16.find_last_of(L" ", iCharIndex);
			iCharIndex = (iPosLastSpace == std::wstring::npos) ? iCharIndex-1 : iPosLastSpace;

			g_pMultiLanguage->ConvertWideCharToStr(strText, (wstrUTF16.substr(0, iCharIndex+1)).c_str(), iConversionType);
			strncpy(pTextOut, strText.c_str(), iOutStrLength);
			
			// 복사될 array를 가리키고 있는 pTextOut포인터의 위치를 iOutStrLength만큼 뒤로 옮긴다.
			// (pTextOut[i][] => pTextOut[i+1][] 과 같은 역할)
			iLineIndex++;
			pTextOut += iOutStrLength;

			// 나머지 문자열을 가지고 반복
			wstrUTF16 = wstrUTF16.substr(iCharIndex+1, wstrUTF16.length()-iCharIndex-1);
			// 초기화 
			iCharIndex = 0;
		}
		else 
		{
			iCharIndex++;
		}
	}

	if (iLineIndex < iMaxOutLine && wstrUTF16.substr(0, iCharIndex).length() > 0)
	{
		g_pMultiLanguage->ConvertWideCharToStr(strText, (wstrUTF16.substr(0, iCharIndex)).c_str(), iConversionType);
		strncpy(pTextOut, strText.c_str(), iOutStrLength);
		iLineIndex++;
	}

	return iLineIndex;

#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE

	//memset(pTextOut, 0, iMaxOutLine * iOutStrLength);		// 메모리 침범함.
	
	const char *pWorkStr = pszSrcText;						// 작업용 Str 포인터
	int iSrcTotalStrLength = unicode::_strlen(pszSrcText)+1;				// 전체 문자갯수(Null문자 포함)
	int iDestStrLineIndex = 0;									// 현재 라인
	int iDestTotalStrLength = 0;								// 현재 문자 갯수
	bool bEndofSrcStr = false;									// 원본 문자열의 끝인가?
	int iStrByteSize = 0;					

	int iScreenRatePixelWidth = iTargetPixelWidth*g_fScreenRate_x;				// 타겟 픽셀 길이를 스크린에 따라 유동적으로 바꾼다.
	
	for( iDestStrLineIndex = 0 ; iDestStrLineIndex < iMaxOutLine ; iDestStrLineIndex++ )
	{
		int iDestStrLength = 0;
		SIZE iDestStrPixelSize;
		iDestStrPixelSize.cx = 0;

		for( iDestStrLength = 0 ; iDestStrLength < iOutStrLength ; )
		{
			iStrByteSize = _mbclen((unsigned char*)&pWorkStr[iDestStrLength]);		// 한 문자당 바이트

			iDestStrLength += iStrByteSize;
			iDestTotalStrLength += iStrByteSize;
			
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
			g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), pWorkStr, iDestStrLength, &iDestStrPixelSize);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
			unicode::_GetTextExtentPoint(g_pRenderText->GetFontDC(), pWorkStr, iDestStrLength, &iDestStrPixelSize);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE

			if( iDestTotalStrLength >= iSrcTotalStrLength )
			{
				bEndofSrcStr = true;
				break;
			}
			
			if( iDestStrPixelSize.cx >= iScreenRatePixelWidth )
				break;
		}

		// 넘어가는(픽셀,문자갯수) 마지막문자 빼기
		iDestStrLength -= iStrByteSize;
		iDestTotalStrLength -= iStrByteSize;

		strncpy(pTextOut, pWorkStr, iDestStrLength);
		pTextOut[iDestStrLength] = '\0';

		if( bEndofSrcStr == true )
			break;

		pWorkStr += iDestStrLength;
		pTextOut += iOutStrLength;
	}

	return iDestStrLineIndex+1;
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
}

// Wrapped Function임, 이걸 쓰지말고, CutStr을 사용하삼.
int CutText3(const char* pszText, char * pTextOut, const int TargetWidth, const int iMaxOutLine, const int iOutStrLength, const int iFirstLineTab, const BOOL bReverseWrite)
{
	return CutStr(pszText, pTextOut, TargetWidth, iMaxOutLine, iOutStrLength, iFirstLineTab );
}

#else  // KJH_FIX_BTS158_TEXT_CUT_ROUTINE

// 요건 버그 덩어리~
int CutText3(const char* pszText, char * pTextOut, const int TargetWidth, const int iMaxOutLine, const int iOutStrLength, const int iFirstLineTab, const BOOL bReverseWrite)
{
	if (pszText == NULL)
	{
		assert(!"텍스트 자르기 입력값 오류");
		return 0;
	}
	const char * pWorkText = pszText;
	//memset(pTextOut, 0, iMaxOutLine * iOutStrLength);
	int iTextIndex = 0;
	int index = 0;
	SIZE TextSize;
	while(1)
	{
		if (index + 1 > iMaxOutLine) return index + 1;
		char * pszTextOut = pTextOut + iOutStrLength * (bReverseWrite == TRUE ? iMaxOutLine - 1 - index : index);	// 결과 저장용

		// 텍스트 길이 체크
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), pWorkText, lstrlen(pWorkText), &TextSize);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		unicode::_GetTextExtentPoint(g_pRenderText->GetFontDC(), pWorkText, lstrlen(pWorkText), &TextSize);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		int TextWidth = int(TextSize.cx / g_fScreenRate_x);	// 작업 문장 너비
		if (TextSize.cx == 0) return index;

		if (TextWidth > TargetWidth - (index == 0 ? iFirstLineTab : 0))
		{
			// 잘라야 한다

			// 길이 정하기
			int resultLength = (TargetWidth - (index == 0 ? iFirstLineTab : 0)) / (TextWidth / lstrlen(pWorkText));	// 길이로 맞춰보고 시작
			int MoveSize = int((float)resultLength / 2.0f + 0.5f);
			while(TRUE)
			{
				if (MoveSize == 0)
					break;

				MoveSize = int((float)MoveSize / 2.0f + 0.5f);
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
				g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), pWorkText, resultLength, &TextSize);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
				unicode::_GetTextExtentPoint(g_pRenderText->GetFontDC(), pWorkText, resultLength, &TextSize);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
				if (TextSize.cx / g_fScreenRate_x > TargetWidth - (index == 0 ? iFirstLineTab : 0) + 4)
				{
					resultLength -= MoveSize;
					if (MoveSize == 1) break;
				}
				else if (TextSize.cx / g_fScreenRate_x < TargetWidth - (index == 0 ? iFirstLineTab : 0) - 4)
				{
					resultLength += MoveSize;
				}
				else
				{
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
					// 버퍼를 작게 잡았을경우 버퍼를 우선으로 자른다
					if(resultLength >= iOutStrLength-2)
						resultLength = iOutStrLength-2;
#endif //KJH_ADD_INGAMESHOP_UI_SYSTEM
					break;
				}
			}
			int iTextSize = 0;
			for (int i = 0; i < resultLength;)
			{
				if (i > resultLength) break;
				i += _mbclen(( unsigned char*)&pWorkText[i]);	// 한글이면 2, 아니면 1 을 리턴
				iTextSize = i;
			}

			strncpy(pszTextOut, pWorkText, iTextSize);
			pszTextOut[iTextSize] = '\0';
			pWorkText += iTextSize;
			++index;
		}
		else
		{
			// 끝
			strncpy(pszTextOut, pWorkText, iOutStrLength);
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
 			pszTextOut[iOutStrLength] = '\0';
#endif //KJH_ADD_INGAMESHOP_UI_SYSTEM
			return index + 1;
		}
	}
	return index + 1;
}

#endif // KJH_FIX_BTS158_TEXT_CUT_ROUTINE
////////////////////////////////////////////////////////////////////////////////////////////////////

void CutText4(const char* pszSource, char * pszResult1, char * pszResult2, int iCutCount)
{
	if (pszSource == NULL || pszSource[0] == '\0') return;

	int iLength = strlen(pszSource);
	int iMove = 0;
	int iTextSize = 0;
	for (int i = 0; i < iLength; )
	{
		if (_mbclen((UCHAR *)&pszSource[i]) == 2)
		{
			iMove = 2;
		}
		else
		{
			iMove = 1;
		}
		if (i + iMove > iCutCount) break;
		else i += iMove;

		iTextSize = i;
	}
	strncpy(pszResult1, pszSource, iTextSize);
	pszResult1[iTextSize] = '\0';
	if (pszResult2 != NULL)
	{
		strncpy(pszResult2, pszSource + iTextSize, iLength - iTextSize);
		pszResult2[iLength - iTextSize] = '\0';
	}
}

BOOL g_bUseChatListBox = TRUE;

extern int DeleteGuildIndex;
extern bool ChatWindowEnable;
void SetLineColor(int iType, float fAlphaRate = 1.0f);

DWORD g_dwLastUIID = 0;
DWORD CreateUIID()
{
	return ++g_dwLastUIID;
}
DWORD g_dwActiveUIID = 0;
DWORD g_dwMouseUseUIID = 0;

DWORD g_dwTopWindow = 0;
DWORD g_dwKeyFocusUIID = 0;
DWORD g_dwCurrentPressedButtonID = 0;


BOOL CheckMouseIn(int iPos_x, int iPos_y, int iWidth, int iHeight, int CoordType)
{
	if (CoordType == COORDINATE_TYPE_LEFT_DOWN)
	{
		if(MouseX >= iPos_x && MouseX < iPos_x + iWidth && 
			MouseY >= iPos_y - iHeight && MouseY < iPos_y)
			return TRUE;
		else return FALSE;
	}
	else
	{
		if(MouseX >= iPos_x && MouseX < iPos_x + iWidth && 
			MouseY >= iPos_y && MouseY < iPos_y + iHeight)
			return TRUE;
		else return FALSE;
	}
}

void CUIMessage::SendUIMessage(int iMessage, int iParam1, int iParam2)
{
	static UI_MESSAGE tempmsg;
	tempmsg.m_iMessage = iMessage;
	tempmsg.m_iParam1 = iParam1;
	tempmsg.m_iParam2 = iParam2;

	m_MessageList.push_back(tempmsg);
}

void CUIMessage::GetUIMessage()
{
	if (m_MessageList.empty()) return;

	std::deque<UI_MESSAGE>::iterator msgiter = m_MessageList.begin();
	m_WorkMessage.m_iMessage = msgiter->m_iMessage;
	m_WorkMessage.m_iParam1 = msgiter->m_iParam1;
	m_WorkMessage.m_iParam2 = msgiter->m_iParam2;

	m_MessageList.pop_front();
}

CUIControl::CUIControl()
{
	m_dwUIID = CreateUIID();
	m_dwParentUIID = 0;
	SetState(0);
	m_iOptions = 0;
	SetPosition(0, 0);
	SetSize(100, 100);
	SetArrangeType();
	SetResizeType();
	m_iCoordType = COORDINATE_TYPE_LEFT_TOP;
}

void CUIControl::SetState(int iState)
{
	m_iState = iState;
}

int CUIControl::GetState()
{
	return m_iState;
}

void CUIControl::SetPosition(int iPos_x, int iPos_y)
{
	m_iPos_x = iPos_x;
	m_iPos_y = iPos_y;
}

void CUIControl::SetSize(int iWidth, int iHeight)
{
	m_iWidth = iWidth;
	m_iHeight = iHeight;
}

void CUIControl::SetArrangeType(int iArrangeType, int iRelativePos_x, int iRelativePos_y)
{
	m_iArrangeType = iArrangeType;
	m_iRelativePos_x = iRelativePos_x;
	m_iRelativePos_y = iRelativePos_y;
}

void CUIControl::SetResizeType(int iResizeType, int iRelativeWidth, int iRelativeHeight)
{
	m_iResizeType = iResizeType;
	m_iRelativeWidth = iRelativeWidth;
	m_iRelativeHeight = iRelativeHeight;
}

void CUIControl::SendUIMessageDirect(int iMessage, int iParam1, int iParam2)
{
	SendUIMessage(iMessage, iParam1, iParam2);
	DoAction(TRUE);
}

BOOL CUIControl::DoAction(BOOL bMessageOnly)
{
	while (m_MessageList.empty() == FALSE)
	{
		GetUIMessage();
		if (HandleMessage() == FALSE)
			DefaultHandleMessage();
	}

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	DoActionSub();
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	DoActionSub(bMessageOnly);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING

	if (bMessageOnly == TRUE) return 0;
	
	if (::CheckMouseIn(m_iPos_x, m_iPos_y, m_iWidth, m_iHeight, m_iCoordType))
	{
		if (g_dwMouseUseUIID == 0)
		{
			if (g_dwActiveUIID == 0 || g_dwActiveUIID == GetUIID())
			{
				g_dwMouseUseUIID = GetUIID();
			}
			else 
			{
				return FALSE;
			}
		}
		if (GetState() == UISTATE_NORMAL)
		{
			if (g_dwMouseUseUIID != GetUIID() && g_dwActiveUIID != GetUIID())
				return FALSE;

			if (MouseLButton == true)
			{
				CUIControl * pRootWindow = NULL;
				if (GetParentUIID() == 0) 
				{
					pRootWindow = this;
				}
				else 
				{
					pRootWindow = g_pWindowMgr->GetWindow(GetParentUIID());
				}

				while (pRootWindow != NULL && pRootWindow->GetParentUIID() != 0)
				{
					pRootWindow = g_pWindowMgr->GetWindow(pRootWindow->GetParentUIID());
				}
				if (pRootWindow != NULL && g_pWindowMgr != NULL)
				{
					if (g_pWindowMgr->IsWindow(pRootWindow->GetUIID()) == TRUE)
						g_pWindowMgr->SendUIMessage(UI_MESSAGE_SELECT, pRootWindow->GetUIID(), 0);
				}
			}
		}
	}
	return DoMouseAction();
}

void CUIControl::DefaultHandleMessage()
{
	switch(m_WorkMessage.m_iMessage)
	{
	case UI_MESSAGE_P_MOVE:
		if (m_dwParentUIID != 0)
		{
			CUIBaseWindow * pWindow = g_pWindowMgr->GetWindow(m_dwParentUIID);
			if (pWindow != NULL)
			{
				switch(m_iArrangeType)
				{
				case 0:
					SetPosition(pWindow->RPos_x(m_iRelativePos_x),pWindow->RPos_y(m_iRelativePos_y));
					break;
				case 1:
					SetPosition(pWindow->RPos_x(-1 * m_iRelativePos_x) + pWindow->RWidth(),pWindow->RPos_y(m_iRelativePos_y));
					break;
				case 2:
					SetPosition(pWindow->RPos_x(m_iRelativePos_x),pWindow->RPos_y(-1 * m_iRelativePos_y) + pWindow->RHeight());
					break;
				case 3:
					SetPosition(pWindow->RPos_x(-1 * m_iRelativePos_x) + pWindow->RWidth(),pWindow->RPos_y(-1 * m_iRelativePos_y) + pWindow->RHeight());
					break;
				default:
					break;
				}
			}
		}
		break;
	case UI_MESSAGE_P_RESIZE:
		if (m_dwParentUIID != 0)
		{
			CUIBaseWindow * pWindow = g_pWindowMgr->GetWindow(m_dwParentUIID);
			if (pWindow != NULL)
			{
				switch(m_iResizeType)
				{
				case 0:
					if (m_iRelativeWidth == 0 && m_iRelativeHeight == 0) break;
					else SetSize(m_iRelativeWidth, m_iRelativeHeight);
					break;
				case 1:
					SetSize(pWindow->RWidth() + m_iRelativeWidth, m_iRelativeHeight);
					break;
				case 2:
					SetSize(m_iRelativeWidth, pWindow->RHeight() + m_iRelativeHeight);
					break;
				case 3:
					SetSize(pWindow->RWidth() + m_iRelativeWidth, pWindow->RHeight() + m_iRelativeHeight);
					break;
				default:
					break;
				}
			}
		}
		break;
	default:
		break;
	}
}

CUIButton::CUIButton()
{
	m_dwButtonID = 0;
	m_pszCaption = NULL;
	SetSize(70, 20);
	m_bMouseState = FALSE;
}

CUIButton::~CUIButton()
{
	if (m_pszCaption != NULL)
	{
		delete [] m_pszCaption;
		m_pszCaption = NULL;
	}
	if (g_dwCurrentPressedButtonID == GetUIID()) g_dwCurrentPressedButtonID = 0;
}

void CUIButton::Init(DWORD dwButtonID, const char* pszCaption)
{
	m_dwButtonID = dwButtonID;
	SetCaption(pszCaption);
}

void CUIButton::SetCaption(const char* pszCaption)
{
	if (pszCaption == NULL) return;
	if (m_pszCaption != NULL)
	{
		if (strcmp(pszCaption, m_pszCaption) == 0) return;
		delete [] m_pszCaption;
	}
	m_pszCaption = new char[strlen(pszCaption) + 1];
	strncpy(m_pszCaption, pszCaption, strlen(pszCaption) + 1);
	//m_pszCaption[strlen(pszCaption)] = '\0';
}

BOOL CUIButton::DoMouseAction()
{
	if (GetState() == UISTATE_DISABLE)
	{
		return FALSE;
	}

	if (/*MouseOnWindow == false && */::CheckMouseIn(m_iPos_x, m_iPos_y, m_iWidth, m_iHeight) == TRUE)
	{
		MouseOnWindow = true;
		if (MouseLButtonPush && g_dwCurrentPressedButtonID == 0)
		{
			m_bMouseState = TRUE;
			g_dwCurrentPressedButtonID = GetUIID();
//			MouseLButton = false;
//			MouseLButtonPush = false;
		}
		if (MouseLButtonPop && g_dwCurrentPressedButtonID == GetUIID())
		{
			g_pWindowMgr->SendUIMessageToWindow(GetParentUIID(), UI_MESSAGE_BTNLCLICK, m_dwButtonID, 0);
			MouseLButtonPop = false;
			PlayBuffer(SOUND_CLICK01);
			m_bMouseState = FALSE;
			g_dwCurrentPressedButtonID = 0;
			return TRUE;
		}
	}
	else if (m_bMouseState == TRUE)
	{
		MouseLButton = false;
		MouseLButtonPush = false;
		MouseLButtonPop = false;
		g_dwCurrentPressedButtonID = 0;
		m_bMouseState = FALSE;
	}

	return FALSE;
}

void CUIButton::Render()
{
	EnableAlphaTest();

	if (GetState() == UISTATE_DISABLE)
	{
		glColor4f(1.0f, 0.4f, 0.4f, 1.0f);
		RenderBitmap(BITMAP_INTERFACE_EX+9, m_iPos_x, m_iPos_y,
			(float)m_iWidth, (float)m_iHeight, 0.f,0.f,49.f/64.f,16.f/16.f);
		if (m_pszCaption != NULL)
		{
			SIZE TextSize;
			const int TextLen = lstrlen(m_pszCaption);
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
			g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), m_pszCaption, TextLen, &TextSize);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
			unicode::_GetTextExtentPoint(g_pRenderText->GetFontDC(), m_pszCaption, TextLen, &TextSize);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
			g_pRenderText->SetTextColor(230, 220, 200, 255);
			g_pRenderText->SetBgColor(0);
			g_pRenderText->RenderText(m_iPos_x + (m_iWidth - (float)TextSize.cx / g_fScreenRate_x + 0.5f) / 2,
				m_iPos_y + 1 + (m_iHeight - (float)TextSize.cy / g_fScreenRate_y + 0.5f) / 2, m_pszCaption);
		}
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		return;
	}

	if (::CheckMouseIn(m_iPos_x, m_iPos_y, m_iWidth, m_iHeight) == TRUE)
	{
		if (m_bMouseState == TRUE)
			glColor4f(0.6f, 0.6f, 0.6f, 1.0f);
		else
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}
	else
	{
		glColor4f(0.8f, 0.8f, 0.8f, 1.0f);
	}

	if (m_bMouseState == TRUE)
		RenderBitmap(BITMAP_INTERFACE_EX+9, m_iPos_x + 1, m_iPos_y + 1,
			(float)m_iWidth - 1, (float)m_iHeight - 1, 0.f,0.f,48.f/64.f,15.f/16.f);
	else
		RenderBitmap(BITMAP_INTERFACE_EX+9, m_iPos_x, m_iPos_y,
			(float)m_iWidth, (float)m_iHeight, 0.f,0.f,49.f/64.f,16.f/16.f);

	if (m_pszCaption != NULL)
	{
		SIZE TextSize;
		const int TextLen = lstrlen(m_pszCaption);

#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), m_pszCaption, TextLen, &TextSize);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		unicode::_GetTextExtentPoint(g_pRenderText->GetFontDC(), m_pszCaption, TextLen, &TextSize);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		g_pRenderText->SetTextColor(230, 220, 200, 255);
		g_pRenderText->SetBgColor(0, 0, 0, 0);
		
		if (m_bMouseState == TRUE)
		{
			g_pRenderText->RenderText(m_iPos_x + 1 + (m_iWidth - (float)TextSize.cx / g_fScreenRate_x + 0.5f) / 2,
				m_iPos_y + 2 + (m_iHeight - (float)TextSize.cy / g_fScreenRate_y + 0.5f) / 2, m_pszCaption);
		}
		else
		{
			g_pRenderText->RenderText(m_iPos_x + (m_iWidth - (float)TextSize.cx / g_fScreenRate_x + 0.5f) / 2,
				m_iPos_y + 1 + (m_iHeight - (float)TextSize.cy / g_fScreenRate_y + 0.5f) / 2, m_pszCaption);
		}
	}
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
 	DisableAlphaBlend();
}

#ifndef KJH_ADD_INGAMESHOP_UI_SYSTEM
const int UIMAX_TEXT_LINE = 150;
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

template <class T>
CUITextListBox<T>::CUITextListBox()
{
	m_iCoordType = COORDINATE_TYPE_LEFT_DOWN;
	m_iScrollType = UILISTBOX_SCROLL_DOWNUP;
	m_bUseMultiline = FALSE;
	m_bUseSelectLine = FALSE;
	m_bPressCursorKey = 0;
	m_bNewTypeScrollBar = TRUE;
	SLSetSelectLine(0);
	m_bUseNewUIScrollBar = FALSE;
}

template <class T>
CUITextListBox<T>::~CUITextListBox()
{
	Clear();
}

template <class T>
void CUITextListBox<T>::Clear()
{
	m_TextList.clear();
	ResetCheckedLine();
	SLSetSelectLine(0);
	m_iCurrentRenderEndLine = 0;
}

template <class T>
void CUITextListBox<T>::ResetCheckedLine(BOOL bFlag)
{
	std::deque<T>::iterator EndIter = m_TextList.end();
	for (m_TextListIter = m_TextList.begin(); m_TextListIter != EndIter; ++m_TextListIter)
	{
		m_TextListIter->m_bIsSelected = bFlag;
	}
}

template <class T>
BOOL CUITextListBox<T>::HaveCheckedLine()
{
	std::deque<T>::iterator EndIter = m_TextList.end();
	for (m_TextListIter = m_TextList.begin(); m_TextListIter != EndIter; ++m_TextListIter)
	{
		if (m_TextListIter->m_bIsSelected == TRUE) return TRUE;
	}
	return FALSE;
}

template <class T>
int CUITextListBox<T>::GetCheckedLines(std::deque<T*> * pSelectLineList)
{
	int iSelectLineNum = 0;
	std::deque<T>::iterator EndIter = m_TextList.end();
	for (m_TextListIter = m_TextList.begin(); m_TextListIter != EndIter; ++m_TextListIter)
	{
		if (m_TextListIter->m_bIsSelected == TRUE)
		{
			++iSelectLineNum;
			pSelectLineList->push_back(&(*m_TextListIter));
		}
	}
	return iSelectLineNum;
}

template <class T>
void CUITextListBox<T>::SLSetSelectLine(int iLineNum)
{
	m_iSelectLineNum = iLineNum;
	if (m_iSelectLineNum > GetLineNum()) m_iSelectLineNum = GetLineNum();
}

template <class T>
void CUITextListBox<T>::SLSelectPrevLine(int iLineNum)
{
	if (m_TextList.empty())
	{
		m_iSelectLineNum = 0;
		return;
	}
	else if (m_iSelectLineNum == 0) return;

	m_iSelectLineNum += iLineNum;
	if (m_iSelectLineNum > GetLineNum()) m_iSelectLineNum = GetLineNum();
}

template <class T>
void CUITextListBox<T>::SLSelectNextLine(int iLineNum)
{
	if (m_TextList.empty())
	{
		m_iSelectLineNum = 0;
		return;
	}
	else if (m_iSelectLineNum == 0) return;

	m_iSelectLineNum -= iLineNum;
	if (m_iSelectLineNum < 1) m_iSelectLineNum = 1;
}

template <class T>

typename std::deque<T>::iterator CUITextListBox<T>::SLGetSelectLine()
{
	if (m_TextList.empty())
	{
		m_iSelectLineNum = 0;
		return m_TextList.end();
	}
	else if (m_iSelectLineNum == 0) return m_TextList.end();

	int iLineCount = 1;
	for (std::deque<T>::iterator resultIter = m_TextList.begin(); resultIter != m_TextList.end(); ++resultIter, ++iLineCount)
	{
		if (iLineCount == m_iSelectLineNum) return resultIter;
	}
	assert(!"라인 계산 오류");
	return m_TextList.end();
}

template <class T>
void CUITextListBox<T>::RemoveText()
{
	int iSize = m_TextList.size();
	if (iSize >= m_iMaxLineCount)
	{
		for (int i = 0; i < iSize - m_iMaxLineCount; ++i)
		{
			m_TextList.pop_back();
		}
		SLSetSelectLine(0);
	}
}

template <class T>
void CUITextListBox<T>::MoveRenderLine()
{
	if (m_bUseMultiline == TRUE) m_TextListIter = m_RenderTextList.begin();
	else m_TextListIter = m_TextList.begin();
	for (int i = 0; i < m_iCurrentRenderEndLine; ++i, ++m_TextListIter);
}

template <class T>
BOOL CUITextListBox<T>::CheckMouseInBox()
{
 	return ::CheckMouseIn(m_iPos_x, m_iPos_y, m_iWidth, m_iHeight, COORDINATE_TYPE_LEFT_DOWN);
}

template <class T>
void CUITextListBox<T>::Render()
{
	RenderInterface();

	MoveRenderLine();
	
	glColor3f(1.f,1.f,1.f);

	g_pRenderText->SetFont(g_hFont);

	for (int i = 0; i < m_iNumRenderLine; ++i)
	{
		if (m_TextListIter == m_TextList.end()) break;
		BOOL bResult = RenderDataLine(i);
		if (bResult < 0)
		{
			i -= bResult;
		}
		else if (bResult == FALSE)
		{
			--i;
		}
		++m_TextListIter;
	}
	RenderCoveredInterface();
	
}

template <class T>
void CUITextListBox<T>::ComputeScrollBar()
{
	if (m_bUseNewUIScrollBar == TRUE)
	{
		m_fScrollBarRange_top = m_iPos_y - m_iHeight;
		m_fScrollBarRange_bottom = m_iPos_y;
		m_fScrollBarHeight = 20;	// 크기 제한

		float fPosRate = 0;
		if (GetLineNum() - GetBoxSize() <= 0) fPosRate = 1;
		else fPosRate = (GetLineNum() > 0 ? (float)m_iCurrentRenderEndLine / (GetLineNum() - GetBoxSize()) : 1);
		m_fScrollBarPos_y = m_fScrollBarRange_bottom - m_fScrollBarHeight * 1.5f
			- (m_fScrollBarRange_bottom - m_fScrollBarRange_top - m_fScrollBarHeight * 1.5f) * fPosRate;
	}
	else
	{
		if (m_bNewTypeScrollBar == TRUE)
		{
			m_fScrollBarRange_top = m_iPos_y - m_iHeight + 12;
			m_fScrollBarRange_bottom = m_iPos_y - 12;
		}
		else
		{
			m_fScrollBarRange_top = m_iPos_y - m_iHeight + 21;
			m_fScrollBarRange_bottom = m_iPos_y - 17;
		}

		float fLineRate = (GetLineNum() > 0 ? (float)m_iNumRenderLine / (float)GetLineNum() : 1);
		m_fScrollBarHeight = (m_fScrollBarRange_bottom - m_fScrollBarRange_top) * (fLineRate < 1 ? fLineRate : 1);
		if (m_fScrollBarHeight < 2) m_fScrollBarHeight = 2;	// 크기 제한
		float fPosRate = (GetLineNum() > 0 ? (float)m_iCurrentRenderEndLine / (float)GetLineNum() : 0);
		m_fScrollBarPos_y = m_fScrollBarRange_bottom 
			- ((m_fScrollBarRange_bottom - m_fScrollBarRange_top - (m_fScrollBarHeight > 2 ? 0 : m_fScrollBarHeight)) * fPosRate)
			- m_fScrollBarHeight;
	}
}

template <class T>
void CUITextListBox<T>::Scrolling(int iValue)
{
	if (m_bUseMultiline == TRUE)
	{
		if (m_RenderTextList.size() < (size_t)m_iNumRenderLine) return;
	}
	else if (m_TextList.size() < (size_t)m_iNumRenderLine) return;

	m_iCurrentRenderEndLine += -1 * iValue;
	if (m_iCurrentRenderEndLine < 0) m_iCurrentRenderEndLine = 0;
	else if (m_iCurrentRenderEndLine > GetLineNum() - m_iNumRenderLine) m_iCurrentRenderEndLine = GetLineNum() - m_iNumRenderLine;
}

template <class T>
void CUITextListBox<T>::Resize(int iValue)
{
	m_iNumRenderLine += iValue * 3;
	if (m_iNumRenderLine < 3) m_iNumRenderLine = 3;
}

template <class T>
BOOL CUITextListBox<T>::HandleMessage()
{
	switch(m_WorkMessage.m_iMessage)
	{
	case UI_MESSAGE_P_RESIZE:
		{
			CalcLineNum();
			if (m_dwParentUIID != 0)
			{
				int iOldNumRenderLine = m_iNumRenderLine;
				SetNumRenderLine(float(m_iHeight) / 13.0f - 0.5f);

				if (m_iScrollType == UILISTBOX_SCROLL_UPDOWN && (int)m_RenderTextList.size() >= m_iNumRenderLine)
				{
					m_iCurrentRenderEndLine += (iOldNumRenderLine - m_iNumRenderLine);
				}
				Scrolling(0);
			}
		}
		break;
	case UI_MESSAGE_LISTSCRLTOP:	// 스크롤 맨 위로
		Scrolling(-1000);
		if (m_bUseSelectLine == TRUE && m_TextList.empty() == FALSE)
		{
			SLSetSelectLine(1000);
		}
		break;
	case UI_MESSAGE_LISTSELUP:
	case UI_MESSAGE_LISTSELDOWN:
		if (m_bUseSelectLine == TRUE && m_TextList.size() >= 1)
		{
			if (m_WorkMessage.m_iMessage == UI_MESSAGE_LISTSELUP)
			{
				SLSelectPrevLine();
			}
			else if (m_WorkMessage.m_iMessage == UI_MESSAGE_LISTSELDOWN)
			{
				SLSelectNextLine();
			}

			if ((int)m_TextList.size() <= m_iNumRenderLine) break;

			int iLine = SLGetSelectLineNum() - 1;
			int iTarget = 0;
			if (iLine >= m_iCurrentRenderEndLine + m_iNumRenderLine)
			{
				iTarget = iLine - m_iNumRenderLine + 1;
			}
			else if (iLine < m_iCurrentRenderEndLine)
			{
				iTarget = iLine;
			}
			else break;
			Scrolling(m_iCurrentRenderEndLine - iTarget);
		}
		break;
	default:
		break;
	}

	return FALSE;
}

template <class T>
BOOL CUITextListBox<T>::DoMouseAction()
{
	BOOL bResult = FALSE;
	if (m_bUseSelectLine == TRUE && g_dwKeyFocusUIID == GetUIID())
	{
		BOOL bKeyPress = FALSE;
#ifdef KWAK_FIX_KEY_STATE_RUNTIME_ERR
		if(SEASON3B::IsPress(VK_LEFT) == TRUE)		{}
		if(SEASON3B::IsPress(VK_RIGHT) == TRUE)		{}
		if(SEASON3B::IsPress(VK_UP) == TRUE)
		{
			bKeyPress = TRUE;
			if (m_bPressCursorKey == 0)
			{
				SendUIMessage(UI_MESSAGE_LISTSELUP, 0, 0);
				m_bPressCursorKey = 1;
			}
			else if (m_bPressCursorKey > 5)
				SendUIMessage(UI_MESSAGE_LISTSELUP, 0, 0);
			else
				++m_bPressCursorKey;
		}
		if(SEASON3B::IsPress(VK_DOWN) == TRUE)
		{
			bKeyPress = TRUE;
			if (m_bPressCursorKey == 0)
			{
				SendUIMessage(UI_MESSAGE_LISTSELDOWN, 0, 0);
				m_bPressCursorKey = 1;
			}
			else if (m_bPressCursorKey > 5)
				SendUIMessage(UI_MESSAGE_LISTSELDOWN, 0, 0);
			else
				++m_bPressCursorKey;
		}
#else // KWAK_FIX_KEY_STATE_RUNTIME_ERR
		if(HIBYTE(GetAsyncKeyState(VK_LEFT ))==128)
		{
		}
		if(HIBYTE(GetAsyncKeyState(VK_RIGHT))==128)
		{
		}
		if(HIBYTE(GetAsyncKeyState(VK_UP   ))==128)
		{
			bKeyPress = TRUE;
			if (m_bPressCursorKey == 0)
			{
				SendUIMessage(UI_MESSAGE_LISTSELUP, 0, 0);
				m_bPressCursorKey = 1;
			}
			else if (m_bPressCursorKey > 5)
				SendUIMessage(UI_MESSAGE_LISTSELUP, 0, 0);
			else
				++m_bPressCursorKey;
		}
		if(HIBYTE(GetAsyncKeyState(VK_DOWN ))==128)
		{
			bKeyPress = TRUE;
			if (m_bPressCursorKey == 0)
			{
				SendUIMessage(UI_MESSAGE_LISTSELDOWN, 0, 0);
				m_bPressCursorKey = 1;
			}
			else if (m_bPressCursorKey > 5)
				SendUIMessage(UI_MESSAGE_LISTSELDOWN, 0, 0);
			else
				++m_bPressCursorKey;
		}
#endif // KWAK_FIX_KEY_STATE_RUNTIME_ERR
		if (bKeyPress == FALSE)
		{
			m_bPressCursorKey = 0;
		}
	}
 	if(CheckMouseInBox())	// 상자 안
	{
		if (MouseWheel != 0)
		{
			Scrolling(-3 * MouseWheel);
			MouseWheel = 0;
		}
		if (MouseLButtonPush && GetState() == UISTATE_NORMAL)
		{
			g_dwKeyFocusUIID = GetUIID();

			DoSubMouseAction();

			int iNewTypePos_x = 0;

			if (m_bUseNewUIScrollBar == TRUE)
			{
				if (GetLineNum() < m_iNumRenderLine);
				else if(::CheckMouseIn(m_iPos_x + m_iWidth - m_fScrollBarWidth, m_fScrollBarPos_y,
					m_fScrollBarWidth, m_fScrollBarHeight))
				{
					if (GetState() == UISTATE_NORMAL && g_dwActiveUIID == 0)
					{
						g_dwActiveUIID = GetUIID();
						SetState(UISTATE_SCROLL);
						m_fScrollBarClickPos_y = MouseY - m_fScrollBarPos_y;
					}
				}
				else if(::CheckMouseIn(m_iPos_x + m_iWidth - m_fScrollBarWidth, m_fScrollBarRange_top,
					m_fScrollBarWidth, m_fScrollBarPos_y - m_fScrollBarRange_top))
				{
					if (GetParentUIID() > 0 && g_pWindowMgr->IsRenderFrame() == FALSE);
					else if (m_bScrollBarClick == FALSE)
					{
						Scrolling(-1 * m_iNumRenderLine);
						m_bScrollBarClick = TRUE;
					}
					else if (m_bScrollBarClick > 15)
					{
						Scrolling(-1 * m_iNumRenderLine);
					}
					else
					{
						if (GetParentUIID() == 0 || g_pWindowMgr->IsRenderFrame() == TRUE)
							m_bScrollBarClick++;
					}
				}
				else if(::CheckMouseIn(m_iPos_x + m_iWidth - m_fScrollBarWidth, m_fScrollBarPos_y + m_fScrollBarHeight,
					m_fScrollBarWidth, m_fScrollBarRange_bottom - m_fScrollBarPos_y - m_fScrollBarHeight))
				{
					if (GetParentUIID() > 0 && g_pWindowMgr->IsRenderFrame() == FALSE);
					else if (m_bScrollBarClick == FALSE)
					{
						Scrolling(m_iNumRenderLine);
						m_bScrollBarClick = TRUE;
					}
					else if (m_bScrollBarClick > 15)
					{
						Scrolling(m_iNumRenderLine);
					}
					else
					{
						if (GetParentUIID() == 0 || g_pWindowMgr->IsRenderFrame() == TRUE)
							m_bScrollBarClick++;
					}
				}
			}
			else
			{
				if (m_bNewTypeScrollBar == TRUE) iNewTypePos_x = 8;

				if(::CheckMouseIn(m_iPos_x + m_iWidth - 21 + iNewTypePos_x,
					m_iPos_y - m_iHeight + (m_bNewTypeScrollBar == TRUE ? 0 : 8), 13, 13))
				{
					if (m_bScrollBtnClick == FALSE)
					{
						Scrolling(-1);
						PlayBuffer(SOUND_CLICK01);
						m_bScrollBtnClick = TRUE;
					}
					else if (m_bScrollBtnClick > 15)
					{
						Scrolling(-1);
					}
					else
					{
						if (GetParentUIID() == 0 || g_pWindowMgr->IsRenderFrame() == TRUE)
							m_bScrollBtnClick++;
					}
				}
				if(::CheckMouseIn(m_iPos_x + m_iWidth - 21 + iNewTypePos_x, m_iPos_y - (m_bNewTypeScrollBar == TRUE ? 13 : 21), 13, 13))
				{
					if (m_bScrollBtnClick == FALSE)
					{
						Scrolling(1);
						PlayBuffer(SOUND_CLICK01);
						m_bScrollBtnClick = TRUE;
					}
					else if (m_bScrollBtnClick > 15)
					{
						Scrolling(1);
					}
					else
					{
						if (GetParentUIID() == 0 || g_pWindowMgr->IsRenderFrame() == TRUE)
							m_bScrollBtnClick++;
					}
				}
				if (GetLineNum() < m_iNumRenderLine);

				else if(::CheckMouseIn(m_iPos_x + m_iWidth - 19 + iNewTypePos_x, m_fScrollBarPos_y,
					m_fScrollBarWidth, m_fScrollBarHeight))
				{
					if (GetState() == UISTATE_NORMAL && g_dwActiveUIID == 0)
					{
						g_dwActiveUIID = GetUIID();
						SetState(UISTATE_SCROLL);
						m_fScrollBarClickPos_y = MouseY - m_fScrollBarPos_y;
					}
				}

				else if(::CheckMouseIn(m_iPos_x + m_iWidth - 19 + iNewTypePos_x, m_fScrollBarRange_top,
					m_fScrollBarWidth, m_fScrollBarPos_y - m_fScrollBarRange_top))
				{
					if (GetParentUIID() > 0 && g_pWindowMgr->IsRenderFrame() == FALSE);
					else if (m_bScrollBarClick == FALSE)
					{
						Scrolling(-1 * m_iNumRenderLine);
						m_bScrollBarClick = TRUE;
					}
					else if (m_bScrollBarClick > 15)
					{
						Scrolling(-1 * m_iNumRenderLine);
					}
					else
					{
						if (GetParentUIID() == 0 || g_pWindowMgr->IsRenderFrame() == TRUE)
							m_bScrollBarClick++;
					}
				}
				else if(::CheckMouseIn(m_iPos_x + m_iWidth - 19 + iNewTypePos_x, m_fScrollBarPos_y + m_fScrollBarHeight,
					m_fScrollBarWidth, m_fScrollBarRange_bottom - m_fScrollBarPos_y - m_fScrollBarHeight))
				{
					if (GetParentUIID() > 0 && g_pWindowMgr->IsRenderFrame() == FALSE);
					else if (m_bScrollBarClick == FALSE)
					{
						Scrolling(m_iNumRenderLine);
						m_bScrollBarClick = TRUE;
					}
					else if (m_bScrollBarClick > 15)
					{
						Scrolling(m_iNumRenderLine);
					}
					else
					{
						if (GetParentUIID() == 0 || g_pWindowMgr->IsRenderFrame() == TRUE)
							m_bScrollBarClick++;
					}
				}
			}
		}
		else
		{
			m_bScrollBtnClick = FALSE;
			m_bScrollBarClick = FALSE;
		}
		MouseOnWindow = true;
		bResult = TRUE;
	}

	if (GetState() == UISTATE_RESIZE)
	{
		if (MouseLButtonPush)
		{
			MouseOnWindow = true;
			m_iNumRenderLine = (m_iPos_y - MouseY + 5) / 40 * 3;
			if (m_iNumRenderLine < 3) m_iNumRenderLine = 3;
			else if (m_iNumRenderLine > 30) m_iNumRenderLine = 30;

			if (GetLineNum() < m_iNumRenderLine);
			else if (GetLineNum() - m_iCurrentRenderEndLine < m_iNumRenderLine) 
				m_iCurrentRenderEndLine = GetLineNum() - m_iNumRenderLine;
		}
		else
		{
			SetState(UISTATE_NORMAL);
			if (g_dwActiveUIID == GetUIID()) g_dwActiveUIID = 0;
		}
	}
	else if (GetState() == UISTATE_SCROLL)
	{
		if (MouseLButtonPush)
		{
			MouseOnWindow = true;
			if (m_bUseNewUIScrollBar == TRUE)
			{
				m_fScrollBarPos_y = (float)MouseY - m_fScrollBarClickPos_y;
				if (m_fScrollBarPos_y < m_fScrollBarRange_top)
				{
					m_fScrollBarPos_y = m_fScrollBarRange_top;
				}
				else if (m_fScrollBarPos_y > m_fScrollBarRange_bottom - m_fScrollBarHeight * 1.5f)
				{
					m_fScrollBarPos_y = m_fScrollBarRange_bottom - m_fScrollBarHeight * 1.5f;
				}

				if (m_fScrollBarPos_y < m_fScrollBarRange_top)
				{
					m_iCurrentRenderEndLine = GetLineNum() - m_iNumRenderLine;
				}
				else if (m_fScrollBarPos_y > m_fScrollBarRange_bottom - m_fScrollBarHeight)
				{
					m_iCurrentRenderEndLine = 0;
				}
 				else
				{
					float fRate = (m_fScrollBarRange_bottom - m_fScrollBarPos_y - m_fScrollBarHeight * 1.5f)
						/ (m_fScrollBarRange_bottom - m_fScrollBarRange_top - m_fScrollBarHeight * 1.5f);
					m_iCurrentRenderEndLine = fRate * (float)(GetLineNum() - GetBoxSize()) + 0.5f;
				}
			}
			else
			{
				m_fScrollBarPos_y = (float)MouseY - m_fScrollBarClickPos_y;
				if (m_fScrollBarPos_y < m_fScrollBarRange_top)
					m_iCurrentRenderEndLine = GetLineNum() - m_iNumRenderLine;
				else if (m_fScrollBarPos_y > m_fScrollBarRange_bottom - m_fScrollBarHeight)
					m_iCurrentRenderEndLine = 0;
				else
					m_iCurrentRenderEndLine = (m_fScrollBarRange_bottom - m_fScrollBarPos_y - m_fScrollBarHeight + 0.5f)
						/ (m_fScrollBarRange_bottom - m_fScrollBarRange_top - (m_fScrollBarHeight > 2 ? 0 : 1.0f))
						* (float)GetLineNum();
			}
		}
		else
		{
			SetState(UISTATE_NORMAL);
			if (g_dwActiveUIID == GetUIID()) g_dwActiveUIID = 0;
		}
	}

	MoveRenderLine();
	if (g_dwTopWindow == 0)
	{
		for (int i = 0; i < m_iNumRenderLine; ++i)
		{
			m_TextListIter = m_TextList.begin();

			if (m_TextListIter == m_TextList.end()) break;
			BOOL bResult = DoLineMouseAction(i);
			if (bResult < 0)
			{
				i -= bResult;
			}
			else if (bResult == FALSE)
			{
				--i;
			}
			++m_TextListIter;
		}
	}
	return bResult;
}

CUIGuildListBox::CUIGuildListBox()
{
	m_iMaxLineCount = UIMAX_TEXT_LINE;
	m_iCurrentRenderEndLine = 0;
	m_iNumRenderLine = 24;

	m_fScrollBarRange_top = 0;
	m_fScrollBarRange_bottom = 0;

	m_fScrollBarPos_y = 0;
	m_fScrollBarWidth = 13;
	m_fScrollBarHeight = 0;

	m_fScrollBarClickPos_y = 0;
	m_bScrollBtnClick = FALSE;
	m_bScrollBarClick = FALSE;

	m_bIsGuildMaster = FALSE;
	m_bNewTypeScrollBar = FALSE;

//	SetPosiion(213, 415);
//	SetSize(213, 250);
	m_iNumRenderLine = 18;
	SetPosition(460, 340);
	SetSize(170, 250);
}

void CUIGuildListBox::AddText(const char * pszID, BYTE Number, BYTE Server)
{
	if (pszID == NULL || pszID[0] == '\0') return;

	if (GetLineNum() == 0)
	{
		if (strcmp(pszID, Hero->ID) == NULL) m_bIsGuildMaster = TRUE;
		else m_bIsGuildMaster = FALSE;
	}

	static GUILDLIST_TEXT text;
	text.m_bIsSelected = FALSE;
	strncpy(text.m_szID, pszID, MAX_ID_SIZE + 1);
	//memcpy(text.m_szID, pszID, strlen(pszID) + 1);
	text.m_Number = Number;
	text.m_Server = Server;
	m_TextList.push_front(text);

	RemoveText();
	SLSetSelectLine(0);
	if (GetLineNum() > m_iNumRenderLine) ++m_iCurrentRenderEndLine;

	if (GetLineNum() < m_iNumRenderLine);
	else if (GetLineNum() - m_iCurrentRenderEndLine < m_iNumRenderLine) 
		m_iCurrentRenderEndLine = GetLineNum() - m_iNumRenderLine;
}

void CUIGuildListBox::RenderInterface()
{
	// 틀 출력
	const int iFillImageHeight = 40;
	const int iFrameImageHeight = 5;

	int iBlockHeight = m_iNumRenderLine / 3;
	m_iHeight = iFillImageHeight * iBlockHeight + iFrameImageHeight * 2;
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	int yPos = m_iPos_y - m_iHeight;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING

	ComputeScrollBar();

	if (GetLineNum() >= m_iNumRenderLine)
	{
		RenderBitmap(BITMAP_INTERFACE_EX+4, (float)m_iPos_x + m_iWidth - 19, (float)m_iPos_y - m_iHeight + 8,
			13.0f, 13.0f, 0.0f, 0.0f, 13.0f/16.0f, 13.0f/16.0f);	// ▲
		EnableAlphaTest();
		RenderBitmap(BITMAP_INTERFACE_EX+4, (float)m_iPos_x + m_iWidth - 19, (float)m_iPos_y - 4,
			13.0f, -13.0f, 0.0f, 0.0f, 13.0f/16.0f, 13.0f/16.0f);	// ▼
		DisableAlphaBlend();

		RenderBitmap(BITMAP_INTERFACE_EX+3, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth - 6, m_fScrollBarRange_top,
			m_fScrollBarWidth, m_fScrollBarRange_bottom - m_fScrollBarRange_top, 0.0f, 0.0f, 13.0f/16.0f, 13.0f/16.0f);

		RenderBitmap(BITMAP_INTERFACE_EX+2, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth - 5, m_fScrollBarPos_y,
			m_fScrollBarWidth - 2, m_fScrollBarHeight, 0.0f, 0.0f, 11.0f/16.0f, 11.0f/16.0f);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CUIGuildListBox::RenderDataLine(int iLineNumber)
{
	EnableAlphaTest();
	int iPos_x = m_iPos_x + 8;
	int iPos_y;
	if (GetLineNum() > m_iNumRenderLine) iPos_y = m_iPos_y - 16 - iLineNumber * 13;
	else iPos_y = m_iPos_y - 16 - (iLineNumber - GetLineNum() + m_iNumRenderLine) * 13;

	g_pRenderText->SetFont(g_hFontBold);

	if(++m_TextListIter == m_TextList.end())
	{
		--m_TextListIter;
		g_pRenderText->SetBgColor(40, 40, 150, 255);
		if ( m_TextListIter->m_Server != 255)
	     	g_pRenderText->SetTextColor(0xFFFFFFFF);
		else
			g_pRenderText->SetTextColor(255, 196, 196, 196);

		CreateGuildMark(Hero->GuildMarkIndex);
		RenderBitmap(BITMAP_GUILD,(float)iPos_x,(float)iPos_y,8,8);
		iPos_x += 13;
	}
	else
	{
		--m_TextListIter;
		g_pRenderText->SetBgColor(0x00000000);
		if ( m_TextListIter->m_Server != 255)
	     	g_pRenderText->SetTextColor(210, 230, 255, 255);
		else
			g_pRenderText->SetTextColor(210, 196, 196, 196);
	}

	char Text[MAX_TEXT_LENGTH + 1] = {0};
	sprintf(Text,"%s",m_TextListIter->m_szID);
	g_pRenderText->RenderText(iPos_x, iPos_y, Text);

	if (m_TextListIter->m_Server != 255/* && m_TextListIter->m_Number != 0*/)
	{
		g_pRenderText->SetBgColor(255, 196, 0, 255);
		g_pRenderText->SetTextColor(0x00000000);
		sprintf(Text,"(%d)", m_TextListIter->m_Server + 1);
		g_pRenderText->RenderText(m_iPos_x + m_iWidth - 60, iPos_y, Text);
	}

	if(m_bIsGuildMaster == TRUE || strcmp(m_TextListIter->m_szID, Hero->ID) == NULL)
	{
		float fWidth=13;
		float fHeight=11;
		float x = (float)m_iPos_x + m_iWidth - 22 - fWidth;
		float y = (float)iPos_y - 1;
		RenderBitmap(BITMAP_INVENTORY_BUTTON,x,y,fWidth,fHeight,0.f,0.f,24.f/32.f,24.f/32.f);
		if(MouseX >= x && MouseX < x + fWidth && MouseY >= y && MouseY < y + fHeight)
		{
			if(MouseLButton)
			{
				RenderBitmap(BITMAP_INVENTORY_BUTTON+1,x,y,fWidth,fHeight,0.f,0.f,24.f/32.f,24.f/32.f);
			}

			g_pRenderText->SetTextColor(255, 255, 255, 255);
			g_pRenderText->SetBgColor(0, 0, 0, 255);

			if(strcmp(m_TextListIter->m_szID, Hero->ID) == NULL && strcmp(GuildList[0].Name,Hero->ID)==NULL)
     			RenderTipText((int)x-20,(int)y,GlobalText[188]);
			else
     			RenderTipText((int)x-20,(int)y,GlobalText[189]);
		}
	}

	g_pRenderText->SetFont(g_hFont);
	DisableAlphaBlend();
	return TRUE;
}

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
BOOL CUIGuildListBox::DoLineMouseAction(int iLineNumber)
{
	if (::CheckMouseIn(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13))
	{
		if (MouseLButtonPush)
		{
			SLSetSelectLine(m_iCurrentRenderEndLine + iLineNumber + 1);
			m_TextListIter->m_bIsSelected = (m_TextListIter->m_bIsSelected + 1) % 2;	// T/F Reverse
			MouseLButtonPush = false;
		}
	}
	return TRUE;
}

int CUIGuildListBox::GetRenderLinePos_y(int iLineNumber)
{
	if (GetLineNum() > m_iNumRenderLine)
		return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1)* 13 - iLineNumber * 13 + 3);
	else
		return (m_iPos_y - m_iHeight + (GetLineNum() - 1) * 13 - iLineNumber * 13 + 3);
}
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING

BOOL CUIGuildListBox::DoSubMouseAction()
{
	m_TextListIter = m_TextList.begin();
	for (int i = 0; i < m_iCurrentRenderEndLine; ++i, ++m_TextListIter);
	
	for (int i = 0; i < m_iNumRenderLine; ++i)
	{
		if (m_TextListIter == m_TextList.end()) break;

		if(m_bIsGuildMaster == TRUE || strcmp(m_TextListIter->m_szID, Hero->ID) == NULL)
		{
			int iPos_y;
			if (GetLineNum() > m_iNumRenderLine) iPos_y = m_iPos_y - 16 - i * 13;
			else iPos_y = m_iPos_y - 16 - (i - GetLineNum() + m_iNumRenderLine) * 13;

			float fWidth=12;
			float fHeight=10;
			float x = (float)m_iPos_x + m_iWidth - 20 - fWidth;
			float y = (float)iPos_y;
			if(MouseX >= x && MouseX < x + fWidth && MouseY >= y && MouseY < y + fHeight)
			{
				if(MouseLButtonPush)
				{
					MouseLButtonPush = false;
					PlayBuffer(SOUND_CLICK01);
					DeleteGuildIndex = GetLineNum() - 1 - i - m_iCurrentRenderEndLine;
					ErrorMessage = MESSAGE_DELETE_GUILD;
					ClearInput(FALSE);
					InputEnable = false;
					InputNumber = 1;
					InputTextMax[0] = g_iLengthAuthorityCode;
					InputTextHide[0] = 1;
				}
			}
		}

		++m_TextListIter;
	}

	return TRUE;
}

CUISimpleChatListBox::CUISimpleChatListBox()
{
	m_iMaxLineCount = UIMAX_TEXT_LINE;
	m_iCurrentRenderEndLine = 0;
	m_iNumRenderLine = 6;

	m_fScrollBarRange_top = 0;
	m_fScrollBarRange_bottom = 0;

	m_fScrollBarPos_y = 0;
	m_fScrollBarWidth = 13;
	m_fScrollBarHeight = 0;

	m_fScrollBarClickPos_y = 0;
	m_bScrollBtnClick = FALSE;
	m_bScrollBarClick = FALSE;

	m_bUseMultiline = TRUE;
}

void CUISimpleChatListBox::AddText(const char * pszID, const char * pszText, int iType, int iColor)
{
	if (pszID[0] == '\0' && pszText[0] == '\0')
	{
		return;
	}

	static WHISPER_TEXT text;
	text.m_bIsSelected = FALSE;
	strncpy(text.m_szID, pszID, MAX_ID_SIZE + 1);
	//memcpy(text.m_szID, pszID, strlen(pszID) + 1);
	text.m_iType = iType;
	text.m_iColor = iColor;
	text.m_uiEmptyLines = 0;

	strncpy(text.m_szText, pszText, MAX_TEXT_LENGTH + 1);
	//memcpy(text.m_szText, pszText, strlen(pszText) + 1);
	m_TextList.push_front(text);

	//RemoveText();
	//if (m_iCurrentRenderEndLine != 0) ++m_iCurrentRenderEndLine;

//	if (GetLineNum() < m_iNumRenderLine);
//	else if (GetLineNum() - m_iCurrentRenderEndLine < m_iNumRenderLine) 
//		m_iCurrentRenderEndLine = GetLineNum() - m_iNumRenderLine;

	// 줄 개수 재 계산
	CalcLineNum();
}

void CUISimpleChatListBox::Render()
{
	
	// 틀 출력
	RenderInterface();

	// 내용
	MoveRenderLine();
	
	glColor3f(1.f,1.f,1.f);
	g_pRenderText->SetFont(g_hFont);

	for (int i = 0; i < m_iNumRenderLine; ++i)
	{
	
		if (m_TextListIter == m_RenderTextList.end()) break;
		BOOL bResult = RenderDataLine(i);
		if (bResult < 0)
		{
			i -= bResult;
		}
		else if (bResult == FALSE)
		{
			--i;
		}
		++m_TextListIter;
	
	}

	RenderCoveredInterface();
	
}

void CUISimpleChatListBox::RenderInterface()
{
	ComputeScrollBar();

	//if (GetLineNum() >= m_iNumRenderLine)
	{
		if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - m_iHeight - 1, 13.0f, 13.0f) == TRUE)
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1,
				13.0f, 13.0f, 13.0f/16.0f, 29.0f/32.0f, -13.0f/16.0f, -13.0f/32.0f);
		else
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1,
				13.0f, 13.0f, 0.0f, 3.0f/32.0f, 13.0f/16.0f, 13.0f/32.0f);	// ▲
		
		if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - 12, 13.0f, 13.0f) == TRUE)
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12,
				13.0f, 13.0f, 13.0f/16.0f, 16.0f/32.0f, -13.0f/16.0f, -13.0f/32.0f);
		else
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12,
				13.0f, 13.0f, 0.0f, 16.0f/32.0f, 13.0f/16.0f, 13.0f/32.0f);	// ▼

		SetLineColor(2);
		RenderColor((float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 1, m_fScrollBarRange_top,(float)1,(float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
		RenderColor((float)m_iPos_x + m_iWidth, m_fScrollBarRange_top,(float)1,(float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
		EndRenderColor();

		if (GetLineNum() >= m_iNumRenderLine)
		{
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y,
				m_fScrollBarWidth - 2, m_fScrollBarHeight, 0.0f, 1.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y,
				m_fScrollBarWidth - 2, 1, 0.0f, 0.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y + m_fScrollBarHeight - 1,
				m_fScrollBarWidth - 2, 1, 0.0f, 2.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
		}
		else
		{
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top,
				m_fScrollBarWidth - 2, m_fScrollBarRange_bottom - m_fScrollBarRange_top, 0.0f, 1.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top,
				m_fScrollBarWidth - 2, 1, 0.0f, 0.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top + m_fScrollBarHeight - 1,
				m_fScrollBarWidth - 2, 1, 0.0f, 2.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
		}
	}
}

void CUISimpleChatListBox::CalcLineNum()
{
	if (m_dwParentUIID != 0)	// 사이즈 바꾸고 시작하자
	{
		CUIBaseWindow * pWindow = g_pWindowMgr->GetWindow(m_dwParentUIID);
		if (pWindow != NULL)
		{
			switch(m_iResizeType)
			{
			case 0:
				if (m_iRelativeWidth == 0 && m_iRelativeHeight == 0) break;
				else SetSize(m_iRelativeWidth, m_iRelativeHeight);
				break;
			case 1:
				SetSize(pWindow->RWidth() + m_iRelativeWidth, m_iRelativeHeight);
				break;
			case 2:
				SetSize(m_iRelativeWidth, pWindow->RHeight() + m_iRelativeHeight);
				break;
			case 3:
				SetSize(pWindow->RWidth() + m_iRelativeWidth, pWindow->RHeight() + m_iRelativeHeight);
				break;
			default:
				break;
			}
		}
	}

	m_RenderTextList.clear();
	std::deque<WHISPER_TEXT>::reverse_iterator iter;
	for (iter = m_TextList.rbegin(); iter != m_TextList.rend(); ++iter)
	{
		AddTextToRenderList(iter->m_szID, iter->m_szText, iter->m_iType, iter->m_iColor);
	}
}

void CUISimpleChatListBox::AddTextToRenderList(const char * pszID, const char * pszText, int iType, int iColor)
{
	if (pszID == NULL || pszText == NULL) return;

	static WHISPER_TEXT text;
	strncpy(text.m_szID, pszID, MAX_ID_SIZE + 1);
	//memcpy(text.m_szID, pszID, strlen(pszID) + 1);
	text.m_iType = iType;
	text.m_iColor = iColor;
	text.m_uiEmptyLines = 0;

	if (strlen(pszID) + strlen(pszText) >= 20)
	{
		SIZE nameSize;
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), pszID, lstrlen(pszID), &nameSize);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		unicode::_GetTextExtentPoint(g_pRenderText->GetFontDC(), pszID, lstrlen(pszID), &nameSize);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE

		char Text1[10][MAX_TEXT_LENGTH + 1] = {{0},{0},{0},{0},{0}};
		int iLine = CutText3(pszText, Text1[0], m_iWidth - 30, 10, MAX_TEXT_LENGTH + 1, (nameSize.cx + 5) / g_fScreenRate_x);

		if (Text1[0][0] != '\0')
		{
			strncpy(text.m_szText, Text1[0], MAX_TEXT_LENGTH + 1);
			//memcpy(text.m_szText, Text1[0], strlen(Text1[0]) + 2);
			m_RenderTextList.push_front(text);
		}

		for (int i = 1; i < iLine; ++i)
		{
			if (Text1[i][0] != '\0')
			{
				text.m_szID[0] = '\0';
				//memcpy(text.m_szID, "", 1);
				strncpy(text.m_szText, Text1[i], MAX_TEXT_LENGTH + 1);
				//memcpy(text.m_szText, Text1[i], strlen(Text1[i]) + 2);
				m_RenderTextList.push_front(text);
			}
		}
	}
	else
	{
		strncpy(text.m_szText, pszText, MAX_TEXT_LENGTH + 1);
		//memcpy(text.m_szText, pszText, strlen(pszText) + 1);
		m_RenderTextList.push_front(text);
	}

// 	RemoveText();
//	if (m_iCurrentRenderEndLine != 0) ++m_iCurrentRenderEndLine;

//	if (GetLineNum() < m_iNumRenderLine);
//	else if (GetLineNum() - m_iCurrentRenderEndLine < m_iNumRenderLine) 
//		m_iCurrentRenderEndLine = GetLineNum() - m_iNumRenderLine;
}

BOOL CUISimpleChatListBox::RenderDataLine(int iLineNumber)
{
	char Text[MAX_TEXT_LENGTH + 1] = {0};
	
	SIZE TextSize = {0, 0};
	// 이름
	if(m_TextListIter->m_szID[0] != NULL)
	{
		switch(m_TextListIter->m_iType)
		{
		case 0:
			g_pRenderText->SetTextColor(0, 0, 0, 255);
			g_pRenderText->SetBgColor(255, 200, 50, 0);
			break;
		case 1:
			g_pRenderText->SetTextColor(100, 150, 255, 255);
			g_pRenderText->SetBgColor(0, 0, 0, 0);
			break;
		case 2:
			g_pRenderText->SetTextColor(255, 30, 0, 255);
			g_pRenderText->SetBgColor(0, 0, 0, 0);
			break;
		case 3:
			g_pRenderText->SetTextColor(239, 220, 205, 255);
			g_pRenderText->SetBgColor(0, 0, 0, 0);
			break;
		case 4:
			g_pRenderText->SetTextColor(0, 0, 0, 255);
			g_pRenderText->SetBgColor(0, 200, 255, 0);
			break;
		case 5:
			g_pRenderText->SetTextColor(0, 0, 0, 255);
			g_pRenderText->SetBgColor(0, 255, 150, 0);
			break;
		}
		
		EnableAlphaTest();
		sprintf(Text,"%s: ",m_TextListIter->m_szID);
		g_pRenderText->RenderText(m_iPos_x + 8, m_iPos_y - 16 - iLineNumber * 13, Text, 0, 0, RT3_SORT_LEFT, &TextSize);
		DisableAlphaBlend();
	}
	
	switch(m_TextListIter->m_iType)
	{
	case 0:
		g_pRenderText->SetTextColor(0, 0, 0, 255);
		g_pRenderText->SetBgColor(255, 200, 50, 0);
		break;
	case 1:
		g_pRenderText->SetTextColor(70, 165, 210, 255);
		g_pRenderText->SetBgColor(0, 0, 0, 0);
		break;
	case 2:
		g_pRenderText->SetTextColor(255, 30, 0, 255);
		g_pRenderText->SetBgColor(0, 0, 0, 0);
		break;
	case 3:
		g_pRenderText->SetTextColor(230, 220, 200, 255);
		g_pRenderText->SetBgColor(0, 0, 0, 0);
		break;
	case 4:
		g_pRenderText->SetTextColor(0, 0, 0, 255);
		g_pRenderText->SetBgColor(0, 200, 255, 0);
		break;
	case 5:
		g_pRenderText->SetTextColor(0, 0, 0, 255);
		g_pRenderText->SetBgColor(0, 255, 150, 0);
		break;
	}
		
	EnableAlphaTest();
	sprintf(Text,"%s",m_TextListIter->m_szText);
	g_pRenderText->RenderText(m_iPos_x + 8 + TextSize.cx, m_iPos_y - 16 - iLineNumber * 13, Text);
	DisableAlphaBlend();

	return TRUE;
}

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
BOOL CUISimpleChatListBox::DoLineMouseAction(int iLineNumber)
{
	if (::CheckMouseIn(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13))
	{
		if (MouseLButtonPush)
		{
			SLSetSelectLine(m_iCurrentRenderEndLine + iLineNumber + 1);
			m_TextListIter->m_bIsSelected = (m_TextListIter->m_bIsSelected + 1) % 2;	// T/F Reverse
			MouseLButtonPush = false;
		}
	}
	return TRUE;
}

int CUISimpleChatListBox::GetRenderLinePos_y(int iLineNumber)
{
	if (GetLineNum() > m_iNumRenderLine)
		return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1)* 13 - iLineNumber * 13 + 3);
	else
		return (m_iPos_y - m_iHeight + (GetLineNum() - 1) * 13 - iLineNumber * 13 + 3);
}
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING

CUIChatPalListBox::CUIChatPalListBox()
{
	m_iMaxLineCount = UIMAX_TEXT_LINE;
	m_iCurrentRenderEndLine = 0;
	m_iNumRenderLine = 6;

	m_fScrollBarRange_top = 0;
	m_fScrollBarRange_bottom = 0;

	m_fScrollBarPos_y = 0;
	m_fScrollBarWidth = 13;
	m_fScrollBarHeight = 0;

	m_fScrollBarClickPos_y = 0;
	m_bScrollBtnClick = FALSE;
	m_bScrollBarClick = FALSE;

	m_iLayoutType = 0;

	m_bUseSelectLine = TRUE;

	m_iColumnWidth[0] = m_iColumnWidth[1] = m_iColumnWidth[2] = m_iColumnWidth[3] = 0;
	SIZE TextSize;
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), "ZZZZZZZZZZ", lstrlen("ZZZZZZZZZZ"), &TextSize);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	unicode::_GetTextExtentPoint(g_pRenderText->GetFontDC(), "ZZZZZZZZZZ", lstrlen("ZZZZZZZZZZ"), &TextSize);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	SetColumnWidth(0, TextSize.cx / g_fScreenRate_x + 8);
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), GlobalText[1022], lstrlen(GlobalText[1022]), &TextSize);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	unicode::_GetTextExtentPoint(g_pRenderText->GetFontDC(), GlobalText[1022], lstrlen(GlobalText[1022]), &TextSize);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	SetColumnWidth(1, TextSize.cx / g_fScreenRate_x + 8);

	m_bForceEditList = FALSE;
}

void CUIChatPalListBox::AddText(const char * pszID, BYTE Number, BYTE Server)
{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	if(pszID == NULL)	return;
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	if (pszID == "")	return;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING

	static GUILDLIST_TEXT text;
	text.m_bIsSelected = FALSE;
	strncpy(text.m_szID, pszID, MAX_ID_SIZE);
	text.m_szID[MAX_ID_SIZE] = '\0';
	//memcpy(text.m_szID, pszID, strlen(pszID) + 1);
	text.m_Number = Number;
	text.m_Server = Server;
	m_TextList.push_front(text);

	RemoveText();
	if (GetLineNum() > m_iNumRenderLine) ++m_iCurrentRenderEndLine;

	if (GetLineNum() < m_iNumRenderLine);
	else if (GetLineNum() - m_iCurrentRenderEndLine < m_iNumRenderLine) 
		m_iCurrentRenderEndLine = GetLineNum() - m_iNumRenderLine;

	if (m_TextList.empty() == FALSE)
	{
		SLSetSelectLine(1);
		m_iCurrentRenderEndLine = 0;
	}
	m_bForceEditList = FALSE;
}

void CUIChatPalListBox::DeleteText(const char * pszID)
{
	if (pszID == NULL || strlen(pszID) == 0) return;
	for (m_TextListIter = m_TextList.begin(); m_TextListIter != m_TextList.end(); ++m_TextListIter)
	{
		if (strncmp(m_TextListIter->m_szID, pszID, MAX_ID_SIZE) == 0)
			break;
	}
	if (m_TextListIter == m_TextList.end()) return;

	if (m_bForceEditList == TRUE || m_TextList.size() == 1)
	{
		m_TextList.erase(m_TextListIter);
		SLSetSelectLine(0);
		return;
	}
	if (SLGetSelectLineNum() != 1) SLSelectNextLine();
	m_TextList.erase(m_TextListIter);
}

const char * CUIChatPalListBox::GetNameByNumber(BYTE byNumber)
{
	if (byNumber == 255) return NULL;
	for (m_TextListIter = m_TextList.begin(); m_TextListIter != m_TextList.end(); ++m_TextListIter)
	{
		if (m_TextListIter->m_Number == byNumber)
			return m_TextListIter->m_szID;
	}
	return NULL;
}

void CUIChatPalListBox::MakeTitleText(char * pszTitleText)
{
	if (pszTitleText == NULL || m_TextList.empty() == TRUE) return;
	int iNameNum = 0;
	std::deque<GUILDLIST_TEXT>::reverse_iterator riter;
	for (riter = m_TextList.rbegin(); riter != m_TextList.rend(); ++riter)
	{
		if (strncmp(riter->m_szID, Hero->ID, MAX_ID_SIZE) != 0)
		{
			if (iNameNum > 0)
			{
				strcat(pszTitleText, ", ");
			}

			strncat(pszTitleText, riter->m_szID, MAX_ID_SIZE);
			++iNameNum;

			if (iNameNum >= 3)
			{
				strcat(pszTitleText, "...");
				break;
			}
		}
	}
}

void CUIChatPalListBox::SetNumRenderLine(int iLine)
{
	if (iLine < m_iNumRenderLine && iLine < GetLineNum()) ++m_iCurrentRenderEndLine;
	else if (iLine > GetLineNum()) m_iCurrentRenderEndLine = 0;
	m_iNumRenderLine = iLine;
}

void CUIChatPalListBox::RenderInterface()
{
	ComputeScrollBar();

	//if (GetLineNum() >= m_iNumRenderLine)
	{
		//-_- 버튼
		if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - m_iHeight - 1, 13.0f, 13.0f) == TRUE)
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1,
				13.0f, 13.0f, 13.0f/16.0f, 29.0f/32.0f, -13.0f/16.0f, -13.0f/32.0f);
		else
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1,
				13.0f, 13.0f, 0.0f, 3.0f/32.0f, 13.0f/16.0f, 13.0f/32.0f);	// ▲
		
		if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - 12, 13.0f, 13.0f) == TRUE)
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12,
				13.0f, 13.0f, 13.0f/16.0f, 16.0f/32.0f, -13.0f/16.0f, -13.0f/32.0f);
		else
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12,
				13.0f, 13.0f, 0.0f, 16.0f/32.0f, 13.0f/16.0f, 13.0f/32.0f);	// ▼

		SetLineColor(2);
		RenderColor((float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 1, m_fScrollBarRange_top,(float)1,(float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
		RenderColor((float)m_iPos_x + m_iWidth, m_fScrollBarRange_top,(float)1,(float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
		EndRenderColor();

		if (GetLineNum() >= m_iNumRenderLine)
		{
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y,
				m_fScrollBarWidth - 2, m_fScrollBarHeight, 0.0f, 1.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y,
				m_fScrollBarWidth - 2, 1, 0.0f, 0.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y + m_fScrollBarHeight - 1,
				m_fScrollBarWidth - 2, 1, 0.0f, 2.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
		}
		else
		{
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top,
				m_fScrollBarWidth - 2, m_fScrollBarRange_bottom - m_fScrollBarRange_top, 0.0f, 1.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top,
				m_fScrollBarWidth - 2, 1, 0.0f, 0.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top + m_fScrollBarHeight - 1,
				m_fScrollBarWidth - 2, 1, 0.0f, 2.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
		}
	}
}

int CUIChatPalListBox::GetRenderLinePos_y(int iLineNumber)
{
	if (GetLineNum() > m_iNumRenderLine)
		return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1)* 13 - iLineNumber * 13 + 3);
	else
		return (m_iPos_y - m_iHeight + (GetLineNum() - 1) * 13 - iLineNumber * 13 + 3);
}

BOOL CUIChatPalListBox::RenderDataLine(int iLineNumber)
{
	EnableAlphaTest();

	if (SLGetSelectLineNum() == m_iCurrentRenderEndLine + iLineNumber + 1)
	{
		if (g_dwKeyFocusUIID == GetUIID())
			glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
		else 
			glColor4f(1.0f, 1.0f, 1.0f, 0.3f);
		RenderColor(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glEnable(GL_TEXTURE_2D);
		g_pRenderText->SetTextColor(0, 0, 0, 255);
	}
	else
	{
		g_pRenderText->SetTextColor(230, 220, 200, 255);
	}

	g_pRenderText->SetBgColor(0);

	int iPos_x = m_iPos_x + 3;
	int iPos_y = GetRenderLinePos_y(iLineNumber);

	char Text[MAX_TEXT_LENGTH + 1] = {0};
	sprintf(Text,"%s",m_TextListIter->m_szID);
	g_pRenderText->RenderText(iPos_x + GetColumnPos_x(0), iPos_y, Text);

	if (m_iLayoutType == 1)
	{
		if (m_TextListIter->m_Server == 0xFF)
		{
			sprintf(Text,GlobalText[1039]);
		}
		else if (m_TextListIter->m_Server == 0xFE)
		{
			sprintf(Text,GlobalText[1039]);	
		}
		else if (m_TextListIter->m_Server == 0xFD)
		{
			sprintf(Text,GlobalText[1041]);
		}
		else if (m_TextListIter->m_Server == 0xFC)
		{
			sprintf(Text,GlobalText[1039]);
		}
//		else if (m_TextListIter->m_Server == 0xFB)
//		{
//			sprintf(Text,GlobalText[1040]);
//		}
		else
		{	
			sprintf(Text, GlobalText[1042], m_TextListIter->m_Server + 1);
		}
		g_pRenderText->RenderText(iPos_x + 4  + GetColumnPos_x(1), iPos_y, Text);
	}
	DisableAlphaBlend();

	return TRUE;
}

BOOL CUIChatPalListBox::DoLineMouseAction(int iLineNumber)
{
	if (::CheckMouseIn(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13))
	{
		if (MouseLButtonPush)
		{
			SLSetSelectLine(m_iCurrentRenderEndLine + iLineNumber + 1);
			m_TextListIter->m_bIsSelected = (m_TextListIter->m_bIsSelected + 1) % 2;	// T/F Reverse
			MouseLButtonPush = false;
		}
		if (MouseLButtonDBClick)
		{
			g_pWindowMgr->SendUIMessageToWindow(GetParentUIID(), UI_MESSAGE_LISTDBLCLICK, GetUIID(), 0);
			MouseLButtonDBClick = false;
		}
	}
	return TRUE;
}

CUIWindowListBox::CUIWindowListBox()
{
	m_iMaxLineCount = UIMAX_TEXT_LINE;
	m_iCurrentRenderEndLine = 0;
	m_iNumRenderLine = 6;

	m_fScrollBarRange_top = 0;
	m_fScrollBarRange_bottom = 0;

	m_fScrollBarPos_y = 0;
	m_fScrollBarWidth = 13;
	m_fScrollBarHeight = 0;

	m_fScrollBarClickPos_y = 0;
	m_bScrollBtnClick = FALSE;
	m_bScrollBarClick = FALSE;

	m_bUseSelectLine = TRUE;
}

void CUIWindowListBox::AddText(DWORD dwUIID, const char* pszTitle, int iStatus)
{
	static WINDOWLIST_TEXT text;
	text.m_bIsSelected = FALSE;
	text.m_dwUIID = dwUIID;
	strncpy(text.m_szTitle, pszTitle, 64);
	text.m_iStatus = iStatus;

	m_TextList.push_front(text);
	SLSetSelectLine(0);

	if (m_iCurrentRenderEndLine != 0) ++m_iCurrentRenderEndLine;

	if (GetLineNum() < m_iNumRenderLine);
	else if (GetLineNum() - m_iCurrentRenderEndLine < m_iNumRenderLine) 
		m_iCurrentRenderEndLine = GetLineNum() - m_iNumRenderLine;

	if (m_TextList.size() == 1)
		SLSetSelectLine(1);

	if (m_TextList.empty() == FALSE)
	{
		SLSetSelectLine(1);
		m_iCurrentRenderEndLine = 0;
	}
}

void CUIWindowListBox::SetNumRenderLine(int iLine)
{
	if (iLine < m_iNumRenderLine && iLine < GetLineNum()) ++m_iCurrentRenderEndLine;
	else if (iLine > GetLineNum()) m_iCurrentRenderEndLine = 0;
	m_iNumRenderLine = iLine;
}

void CUIWindowListBox::RenderInterface()
{
	ComputeScrollBar();

	//if (GetLineNum() >= m_iNumRenderLine)
	{
		if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - m_iHeight - 1, 13.0f, 13.0f) == TRUE)
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1,
				13.0f, 13.0f, 13.0f/16.0f, 29.0f/32.0f, -13.0f/16.0f, -13.0f/32.0f);
		else
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1,
				13.0f, 13.0f, 0.0f, 3.0f/32.0f, 13.0f/16.0f, 13.0f/32.0f);	// ▲
		
		if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - 12, 13.0f, 13.0f) == TRUE)
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12,
				13.0f, 13.0f, 13.0f/16.0f, 16.0f/32.0f, -13.0f/16.0f, -13.0f/32.0f);
		else
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12,
				13.0f, 13.0f, 0.0f, 16.0f/32.0f, 13.0f/16.0f, 13.0f/32.0f);	// ▼

		SetLineColor(2);
		RenderColor((float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 1, m_fScrollBarRange_top,(float)1,(float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
		RenderColor((float)m_iPos_x + m_iWidth, m_fScrollBarRange_top,(float)1,(float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
		EndRenderColor();

		if (GetLineNum() >= m_iNumRenderLine)
		{
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y,
				m_fScrollBarWidth - 2, m_fScrollBarHeight, 0.0f, 1.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y,
				m_fScrollBarWidth - 2, 1, 0.0f, 0.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y + m_fScrollBarHeight - 1,
				m_fScrollBarWidth - 2, 1, 0.0f, 2.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
		}
		else
		{
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top,
				m_fScrollBarWidth - 2, m_fScrollBarRange_bottom - m_fScrollBarRange_top, 0.0f, 1.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top,
				m_fScrollBarWidth - 2, 1, 0.0f, 0.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top + m_fScrollBarHeight - 1,
				m_fScrollBarWidth - 2, 1, 0.0f, 2.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
		}
	}
}

int CUIWindowListBox::GetRenderLinePos_y(int iLineNumber)
{
	if (GetLineNum() > m_iNumRenderLine)
		return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1)* 13 - iLineNumber * 13 + 3);
	else
		return (m_iPos_y - m_iHeight + (GetLineNum() - 1) * 13 - iLineNumber * 13 + 3);
}

BOOL CUIWindowListBox::RenderDataLine(int iLineNumber)
{
	EnableAlphaTest();

	if (SLGetSelectLineNum() == m_iCurrentRenderEndLine + iLineNumber + 1)
	{
		if (g_dwKeyFocusUIID == GetUIID()) glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
		else glColor4f(1.0f, 1.0f, 1.0f, 0.3f);
		RenderColor(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glEnable(GL_TEXTURE_2D);
		g_pRenderText->SetTextColor(0, 0, 0, 255);
	}
	else
	{
		g_pRenderText->SetTextColor(230, 220, 200, 255);
	}

	g_pRenderText->SetBgColor(0);

	int iPos_x = m_iPos_x + 8;
	int iPos_y = GetRenderLinePos_y(iLineNumber);

	char Text[MAX_TEXT_LENGTH + 1] = {0};
	sprintf(Text,"%s",m_TextListIter->m_szTitle);
	g_pRenderText->RenderText(iPos_x, iPos_y, Text);

	DisableAlphaBlend();

	return TRUE;
}

BOOL CUIWindowListBox::DoLineMouseAction(int iLineNumber)
{
	if (::CheckMouseIn(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13))
	{
		if (MouseLButtonPush)
		{
			SLSetSelectLine(m_iCurrentRenderEndLine + iLineNumber + 1);
			m_TextListIter->m_bIsSelected = (m_TextListIter->m_bIsSelected + 1) % 2;	// T/F Reverse
			g_pWindowMgr->SendUIMessageToWindow(GetParentUIID(), UI_MESSAGE_LISTDBLCLICK, GetUIID(), 0);
			MouseLButtonPush = false;
		}
	}
	return TRUE;
}

void CUIWindowListBox::DeleteText(DWORD dwUIID)
{
	BOOL bFind = FALSE;
	for (m_TextListIter = m_TextList.begin(); m_TextListIter != m_TextList.end(); ++m_TextListIter)
	{
		if (m_TextListIter->m_dwUIID == dwUIID)
		{
			bFind = TRUE;
			break;
		}
	}
	if (bFind == FALSE) return;
	
	if (m_TextList.size() == 1)
	{
		m_TextList.erase(m_TextListIter);
		SLSetSelectLine(0);
		return;
	}
	if (SLGetSelectLineNum() != 1) SLSelectNextLine();
	m_TextList.erase(m_TextListIter);
}

void RenderCheckBox(int iPos_x, int iPos_y, BOOL bFlag)
{
	DisableAlphaBlend();
	SetLineColor(0);
	RenderColor(iPos_x, iPos_y, 9, 1);
	RenderColor(iPos_x, iPos_y + 8, 9, 1);
	RenderColor(iPos_x, iPos_y, 1, 9);
	RenderColor(iPos_x + 8, iPos_y, 1, 9);
	EndRenderColor();
	EnableAlphaTest();
	if (bFlag == TRUE)
		RenderBitmap(BITMAP_INTERFACE_EX+13,iPos_x + 2,iPos_y + 2,5.0f,5.0f,0.f,0.f,5.f/8.f,5.f/8.f);
}

CUILetterListBox::CUILetterListBox()
{
	m_iMaxLineCount = UIMAX_TEXT_LINE;
	m_iCurrentRenderEndLine = 0;
	m_iNumRenderLine = 6;

	m_fScrollBarRange_top = 0;
	m_fScrollBarRange_bottom = 0;

	m_fScrollBarPos_y = 0;
	m_fScrollBarWidth = 13;
	m_fScrollBarHeight = 0;

	m_fScrollBarClickPos_y = 0;
	m_bScrollBtnClick = FALSE;
	m_bScrollBarClick = FALSE;

	m_bUseSelectLine = TRUE;

	m_iColumnWidth[0] = m_iColumnWidth[1] = m_iColumnWidth[2] = m_iColumnWidth[3] = 0;
	SIZE TextSize;

	SetColumnWidth(0, 15 + 10);
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), GlobalText[1028], lstrlen(GlobalText[1028]), &TextSize);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	unicode::_GetTextExtentPoint(g_pRenderText->GetFontDC(), GlobalText[1028], lstrlen(GlobalText[1028]), &TextSize);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	SetColumnWidth(1, TextSize.cx / g_fScreenRate_x + 8);
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), GlobalText[1029], lstrlen(GlobalText[1029]), &TextSize);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	unicode::_GetTextExtentPoint(g_pRenderText->GetFontDC(), GlobalText[1029], lstrlen(GlobalText[1029]), &TextSize);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	SetColumnWidth(2, TextSize.cx / g_fScreenRate_x + 8);
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), GlobalText[1030], lstrlen(GlobalText[1030]), &TextSize);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	unicode::_GetTextExtentPoint(g_pRenderText->GetFontDC(), GlobalText[1030], lstrlen(GlobalText[1030]), &TextSize);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	SetColumnWidth(3, TextSize.cx / g_fScreenRate_x + 8);

	m_bForceEditList = FALSE;
}

DWORD g_dwLastLetterID = 0;

void CUILetterListBox::AddText(const char * pszID, const char * pszText, const char * pszDate, const char * pszTime, BOOL bIsRead)
{
	static LETTERLIST_TEXT text;
	text.m_bIsSelected = FALSE;
	strncpy(text.m_szID, pszID, MAX_ID_SIZE + 1);
	strncpy(text.m_szText, pszText, MAX_TEXT_LENGTH + 1);
	strncpy(text.m_szDate, pszDate, 16);
	strncpy(text.m_szTime, pszTime, 16);
	text.m_bIsRead = bIsRead;
	text.m_dwLetterID = ++g_dwLastLetterID;

	m_TextList.push_front(text);
	SLSetSelectLine(0);

	if (m_iCurrentRenderEndLine != 0) ++m_iCurrentRenderEndLine;

	if (GetLineNum() < m_iNumRenderLine);
	else if (GetLineNum() - m_iCurrentRenderEndLine < m_iNumRenderLine) 
		m_iCurrentRenderEndLine = GetLineNum() - m_iNumRenderLine;

	if (m_TextList.empty() == FALSE)
	{
		SLSetSelectLine(1);
		m_iCurrentRenderEndLine = 0;
	}
	m_bForceEditList = FALSE;
}

void CUILetterListBox::DeleteText(DWORD dwLetterID)
{
	for (m_TextListIter = m_TextList.begin(); m_TextListIter != m_TextList.end(); ++m_TextListIter)
	{
		if (m_TextListIter->m_dwLetterID == dwLetterID)
			break;
	}

	if (m_bForceEditList == TRUE || m_TextList.size() == 1)
	{
		m_TextList.erase(m_TextListIter);
		SLSetSelectLine(0);
		return;
	}
	if (SLGetSelectLineNum() != 1) SLSelectNextLine();
	m_TextList.erase(m_TextListIter);
}

void CUILetterListBox::SetNumRenderLine(int iLine)
{
	if (iLine < m_iNumRenderLine && iLine < GetLineNum()) ++m_iCurrentRenderEndLine;
	else if (iLine > GetLineNum()) m_iCurrentRenderEndLine = 0;
	m_iNumRenderLine = iLine;
}

void CUILetterListBox::RenderInterface()
{
	ComputeScrollBar();

	//if (GetLineNum() >= m_iNumRenderLine)
	{
		if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - m_iHeight - 1, 13.0f, 13.0f) == TRUE)
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1,
				13.0f, 13.0f, 13.0f/16.0f, 29.0f/32.0f, -13.0f/16.0f, -13.0f/32.0f);
		else
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1,
				13.0f, 13.0f, 0.0f, 3.0f/32.0f, 13.0f/16.0f, 13.0f/32.0f);	// ▲
		
		if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - 12, 13.0f, 13.0f) == TRUE)
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12,
				13.0f, 13.0f, 13.0f/16.0f, 16.0f/32.0f, -13.0f/16.0f, -13.0f/32.0f);
		else
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12,
				13.0f, 13.0f, 0.0f, 16.0f/32.0f, 13.0f/16.0f, 13.0f/32.0f);	// ▼

		SetLineColor(2);
		RenderColor((float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 1, m_fScrollBarRange_top,(float)1,(float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
		RenderColor((float)m_iPos_x + m_iWidth, m_fScrollBarRange_top,(float)1,(float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
		EndRenderColor();

		if (GetLineNum() >= m_iNumRenderLine)
		{
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y,
				m_fScrollBarWidth - 2, m_fScrollBarHeight, 0.0f, 1.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y,
				m_fScrollBarWidth - 2, 1, 0.0f, 0.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y + m_fScrollBarHeight - 1,
				m_fScrollBarWidth - 2, 1, 0.0f, 2.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
		}
		else
		{
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top,
				m_fScrollBarWidth - 2, m_fScrollBarRange_bottom - m_fScrollBarRange_top, 0.0f, 1.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top,
				m_fScrollBarWidth - 2, 1, 0.0f, 0.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top + m_fScrollBarHeight - 1,
				m_fScrollBarWidth - 2, 1, 0.0f, 2.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
		}
	}
}

int CUILetterListBox::GetRenderLinePos_y(int iLineNumber)
{
	if (GetLineNum() > m_iNumRenderLine)
		return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1)* 13 - iLineNumber * 13 + 3);
	else
		return (m_iPos_y - m_iHeight + (GetLineNum() - 1) * 13 - iLineNumber * 13 + 3);
}

BOOL CUILetterListBox::RenderDataLine(int iLineNumber)
{
	EnableAlphaTest();

	if (SLGetSelectLineNum() == m_iCurrentRenderEndLine + iLineNumber + 1)
	{
		if (g_dwKeyFocusUIID == GetUIID()) glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
		else glColor4f(1.0f, 1.0f, 1.0f, 0.3f);
		RenderColor(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glEnable(GL_TEXTURE_2D);
		g_pRenderText->SetTextColor(0, 0, 0, 255);
	}
	else
	{
		g_pRenderText->SetTextColor(230, 220, 200, 255);
	}

	g_pRenderText->SetBgColor(0);

	int iPos_x = m_iPos_x;
	int iPos_y = GetRenderLinePos_y(iLineNumber);

	char Text[MAX_TEXT_LENGTH + 1] = {0};
	RenderCheckBox(iPos_x + 1,iPos_y - 1, m_TextListIter->m_bIsSelected);
	if (m_TextListIter->m_bIsRead == TRUE)
		RenderBitmap(BITMAP_INTERFACE_EX+14,iPos_x + 1 + 10,iPos_y - 2,13.0f,10.0f,0.f,9.f/32.f,13.f/16.f,10.f/32.f);
	else
		RenderBitmap(BITMAP_INTERFACE_EX+14,iPos_x + 1 + 10,iPos_y - 1,13.0f,9.0f,0.f,0.f,13.f/16.f,9.f/32.f);

	strncpy(Text, m_TextListIter->m_szID, MAX_TEXT_LENGTH);
	g_pRenderText->RenderText(iPos_x + 4 + GetColumnPos_x(1), iPos_y, Text, GetColumnWidth(1) - 4, 0, RT3_SORT_LEFT_CLIP);
	strncpy(Text, m_TextListIter->m_szDate + 2, MAX_TEXT_LENGTH);
	g_pRenderText->RenderText(iPos_x + GetColumnPos_x(2), iPos_y, Text, GetColumnWidth(2), 0, RT3_SORT_CENTER);
	int iMaxWidth = m_iWidth - m_fScrollBarWidth - GetColumnPos_x(3) - 4;
	g_pRenderText->RenderText(iPos_x + 4 + GetColumnPos_x(3), iPos_y, m_TextListIter->m_szText, iMaxWidth, 0, RT3_SORT_LEFT_CLIP);
	DisableAlphaBlend();

	return TRUE;
}

BOOL CUILetterListBox::DoLineMouseAction(int iLineNumber)
{
	if (::CheckMouseIn(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13))
	{
		if (MouseLButtonPush)
		{
			if (::CheckMouseIn(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, 12, 13))
			{
				m_TextListIter->m_bIsSelected = (m_TextListIter->m_bIsSelected + 1) % 2;	// T/F Reverse
				LETTERLIST_TEXT * lt = g_pLetterList->GetLetter(m_TextListIter->m_dwLetterID);
				lt->m_bIsSelected = m_TextListIter->m_bIsSelected;
			}
			SLSetSelectLine(m_iCurrentRenderEndLine + iLineNumber + 1);
			MouseLButtonPush = false;
		}
		if (MouseLButtonDBClick && MouseX > m_iPos_x + 12)
		{
			g_pWindowMgr->SendUIMessageToWindow(GetParentUIID(), UI_MESSAGE_LISTDBLCLICK, GetUIID(), 0);
			MouseLButtonDBClick = false;
		}
	}
	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

CUILetterTextListBox::CUILetterTextListBox()
{
	m_iMaxLineCount = UIMAX_TEXT_LINE;
	m_iCurrentRenderEndLine = 0;
	m_iNumRenderLine = 6;	// 3의 배수로 -_-

	m_fScrollBarRange_top = 0;
	m_fScrollBarRange_bottom = 0;

	m_fScrollBarPos_y = 0;
	m_fScrollBarWidth = 13;
	m_fScrollBarHeight = 0;

	m_fScrollBarClickPos_y = 0;
	m_bScrollBtnClick = FALSE;
	m_bScrollBarClick = FALSE;

	m_bUseMultiline = TRUE;
	m_iScrollType = UILISTBOX_SCROLL_UPDOWN;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void CUILetterTextListBox::AddText(const char * pszText)
{
	if (pszText == NULL || pszText[0] == '\0')
	{
		return;
	}

	static LETTER_TEXT text;
	text.m_bIsSelected = FALSE;
	strncpy(text.m_szText, pszText, MAX_LETTERTEXT_LENGTH + 1);
	m_TextList.push_front(text);
	SLSetSelectLine(0);

	// 줄 개수 재 계산
	CalcLineNum();
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void CUILetterTextListBox::Render()
{
	// 틀 출력
	RenderInterface();

	// 내용
	MoveRenderLine();
	
	glColor3f(1.f,1.f,1.f);
	g_pRenderText->SetFont(g_hFont);

	for (int i = 0; i < m_iNumRenderLine; ++i)
	{
		if (m_TextListIter == m_RenderTextList.end()) break;
		BOOL bResult = RenderDataLine(i);
		if (bResult < 0)	// 공란 삽입 (-1 * 공란 개수)
		{
			i -= bResult;
		}
		else if (bResult == FALSE)	// 라인 스킵
		{
			--i;
		}
		++m_TextListIter;
	}

	RenderCoveredInterface();
}

void CUILetterTextListBox::RenderInterface()
{
	// 스크롤 바
	ComputeScrollBar();

	//if (GetLineNum() >= m_iNumRenderLine)
	{
		//-_- 버튼
		if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - m_iHeight - 1, 13.0f, 13.0f) == TRUE)
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1,
				13.0f, 13.0f, 13.0f/16.0f, 29.0f/32.0f, -13.0f/16.0f, -13.0f/32.0f);
		else
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1,
				13.0f, 13.0f, 0.0f, 3.0f/32.0f, 13.0f/16.0f, 13.0f/32.0f);	// ▲
		
		if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - 12, 13.0f, 13.0f) == TRUE)
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12,
				13.0f, 13.0f, 13.0f/16.0f, 16.0f/32.0f, -13.0f/16.0f, -13.0f/32.0f);
		else
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12,
				13.0f, 13.0f, 0.0f, 16.0f/32.0f, 13.0f/16.0f, 13.0f/32.0f);	// ▼

		SetLineColor(2);
		RenderColor((float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 1, m_fScrollBarRange_top,(float)1,(float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
		RenderColor((float)m_iPos_x + m_iWidth, m_fScrollBarRange_top,(float)1,(float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
		EndRenderColor();

		if (GetLineNum() >= m_iNumRenderLine)
		{
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y,
				m_fScrollBarWidth - 2, m_fScrollBarHeight, 0.0f, 1.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y,
				m_fScrollBarWidth - 2, 1, 0.0f, 0.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y + m_fScrollBarHeight - 1,
				m_fScrollBarWidth - 2, 1, 0.0f, 2.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
		}
		else
		{
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top,
				m_fScrollBarWidth - 2, m_fScrollBarRange_bottom - m_fScrollBarRange_top, 0.0f, 1.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top,
				m_fScrollBarWidth - 2, 1, 0.0f, 0.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top + m_fScrollBarHeight - 1,
				m_fScrollBarWidth - 2, 1, 0.0f, 2.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void CUILetterTextListBox::CalcLineNum()
{
	if (m_dwParentUIID != 0)	// 사이즈 바꾸고 시작하자
	{
		CUIBaseWindow * pWindow = g_pWindowMgr->GetWindow(m_dwParentUIID);
		if (pWindow != NULL)
		{
			switch(m_iResizeType)
			{
			case 0:	// 절대;절대
				if (m_iRelativeWidth == 0 && m_iRelativeHeight == 0) break;	// 사이즈 변하지 않는것
				else SetSize(m_iRelativeWidth, m_iRelativeHeight);
				break;
			case 1:	// 상대;절대
				SetSize(pWindow->RWidth() + m_iRelativeWidth, m_iRelativeHeight);
				break;
			case 2:	// 절대;상대
				SetSize(m_iRelativeWidth, pWindow->RHeight() + m_iRelativeHeight);
				break;
			case 3:	// 상대;상대
				SetSize(pWindow->RWidth() + m_iRelativeWidth, pWindow->RHeight() + m_iRelativeHeight);
				break;
			default:
				break;
			}
		}
	}

	m_RenderTextList.clear();
	std::deque<LETTER_TEXT>::reverse_iterator iter;
	for (iter = m_TextList.rbegin(); iter != m_TextList.rend(); ++iter)
	{
		// 텍스트 만들기
		AddTextToRenderList(iter->m_szText);
	}
}

void CUILetterTextListBox::AddTextToRenderList(const char * pszText)
{
	if (pszText == NULL) return;

	static LETTER_TEXT text;
	if (strlen(pszText) >= 20)	// 사이즈 오버
	{
		static char Text1[80][MAX_TEXT_LENGTH + 1];
		int iLine = CutText3(pszText, Text1[0], m_iWidth - 30, 80, MAX_TEXT_LENGTH + 1);

		for (int i = 0; i < iLine; ++i)
		{
			if (Text1[i][0] != '\0')
			{
				strncpy(text.m_szText, Text1[i], MAX_TEXT_LENGTH + 1);
				//memcpy(text.m_szText, Text1[i], strlen(Text1[i]) + 2);
				m_RenderTextList.push_front(text);
			}
		}
	}
	else
	{
		strncpy(text.m_szText, pszText, MAX_TEXT_LENGTH + 1);
		//memcpy(text.m_szText, pszText, strlen(pszText) + 1);
		m_RenderTextList.push_front(text);
	}

// 	RemoveText();
//	if (m_iCurrentRenderEndLine != 0) ++m_iCurrentRenderEndLine;

//	if (GetLineNum() < m_iNumRenderLine);
//	else if (GetLineNum() - m_iCurrentRenderEndLine < m_iNumRenderLine) 
//		m_iCurrentRenderEndLine = GetLineNum() - m_iNumRenderLine;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

int CUILetterTextListBox::GetRenderLinePos_y(int iLineNumber)
{
	if (GetLineNum() > m_iNumRenderLine)
		return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1)* 13 - iLineNumber * 13 + 3);
	else
		return (m_iPos_y - m_iHeight + (GetLineNum() - 1) * 13 - iLineNumber * 13 + 3);
}
////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CUILetterTextListBox::RenderDataLine(int iLineNumber)
{
	EnableAlphaTest();
	char Text[MAX_TEXT_LENGTH + 1] = {0};
	// 내용
	g_pRenderText->SetTextColor(230, 220, 200, 255);
	g_pRenderText->SetBgColor(0, 0, 0, 0);

	sprintf(Text,"%s",m_TextListIter->m_szText);
	int iPos_x = m_iPos_x + 10;
	int iPos_y = GetRenderLinePos_y(iLineNumber);
	g_pRenderText->RenderText(iPos_x, iPos_y, Text);

	DisableAlphaBlend();

	return TRUE;
}

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
BOOL CUILetterTextListBox::DoLineMouseAction(int iLineNumber)
{
	if (::CheckMouseIn(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13))
	{
		if (MouseLButtonPush)
		{
			SLSetSelectLine(m_iCurrentRenderEndLine + iLineNumber + 1);
			m_TextListIter->m_bIsSelected = (m_TextListIter->m_bIsSelected + 1) % 2;	// T/F Reverse
			MouseLButtonPush = false;
		}
	}
	return TRUE;
}
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef ADD_SOCKET_MIX
CUISocketListBox::CUISocketListBox()
{
	m_iMaxLineCount = UIMAX_TEXT_LINE;
	m_iCurrentRenderEndLine = 0;
	m_iNumRenderLine = 6;	// 3의 배수로 -_-

	m_fScrollBarRange_top = 0;
	m_fScrollBarRange_bottom = 0;

	m_fScrollBarPos_y = 0;
	m_fScrollBarWidth = 13;
	m_fScrollBarHeight = 0;

	m_fScrollBarClickPos_y = 0;
	m_bScrollBtnClick = FALSE;
	m_bScrollBarClick = FALSE;

	m_bUseSelectLine = TRUE;
	m_bUseNewUIScrollBar = TRUE;
	m_iScrollType = UILISTBOX_SCROLL_UPDOWN;
	
	SetPosition(275, 360);
	SetSize(160, 65);
}

void CUISocketListBox::AddText(int iSocketIndex, const char * pszText)
{
	static SOCKETLIST_TEXT text;
	text.m_bIsSelected = FALSE;
	text.m_iSocketIndex = iSocketIndex;
	strncpy(text.m_szText, pszText, 64);

	m_TextList.push_front(text);
	SLSetSelectLine(0);

	if (m_iCurrentRenderEndLine != 0) ++m_iCurrentRenderEndLine;

	if (GetLineNum() < m_iNumRenderLine);
	else if (GetLineNum() - m_iCurrentRenderEndLine < m_iNumRenderLine) 
		m_iCurrentRenderEndLine = GetLineNum() - m_iNumRenderLine;

	if (m_TextList.size() == 1)
		SLSetSelectLine(1);

	if (m_TextList.empty() == FALSE)
	{
		SLSetSelectLine(1);
		m_iCurrentRenderEndLine = 0;
	}
}

void CUISocketListBox::SetNumRenderLine(int iLine)
{
	if (iLine < m_iNumRenderLine && iLine < GetLineNum()) ++m_iCurrentRenderEndLine;
	else if (iLine > GetLineNum()) m_iCurrentRenderEndLine = 0;
	m_iNumRenderLine = iLine;
}

void CUISocketListBox::RenderInterface()
{
	// 배경칠
	EnableAlphaTest();
	SetLineColor(7, 0.4f);
	RenderColor(m_iPos_x - 1, m_iPos_y - m_iHeight - 1, m_iWidth + 1, m_iHeight + 2);
	EndRenderColor();

	// 스크롤 바
	if (GetState() != UISTATE_SCROLL)
		ComputeScrollBar();

	g_pGuardWindow->RenderScrollBarFrame(m_iPos_x + m_iWidth - 8, m_fScrollBarRange_top, m_fScrollBarRange_bottom - m_fScrollBarRange_top);
	g_pGuardWindow->RenderScrollBar(m_iPos_x + m_iWidth - 12, m_fScrollBarPos_y, (GetState() == UISTATE_SCROLL && MouseLButtonPush));
}

int CUISocketListBox::GetRenderLinePos_y(int iLineNumber)
{
	if (GetLineNum() > m_iNumRenderLine)
		return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1)* 13 - iLineNumber * 13 + 3);
	else
		return (m_iPos_y - m_iHeight + (GetLineNum() - 1) * 13 - iLineNumber * 13 + 3);
}
////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CUISocketListBox::RenderDataLine(int iLineNumber)
{
	EnableAlphaTest();

	if (SLGetSelectLineNum() == m_iCurrentRenderEndLine + iLineNumber + 1)
	{
		if (g_dwKeyFocusUIID == GetUIID()) glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
		else glColor4f(1.0f, 1.0f, 1.0f, 0.3f);
		RenderColor(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glEnable(GL_TEXTURE_2D);
		g_pRenderText->SetTextColor(0, 0, 0, 255);
	}
	else
	{
		g_pRenderText->SetTextColor(230, 220, 200, 255);
	}

	g_pRenderText->SetBgColor(0);

	int iPos_x = m_iPos_x + 8;
	int iPos_y = GetRenderLinePos_y(iLineNumber);

	char Text[MAX_TEXT_LENGTH + 1] = {0};
	sprintf(Text,"%s",m_TextListIter->m_szText);
	g_pRenderText->RenderText(iPos_x, iPos_y, Text);

	DisableAlphaBlend();

	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CUISocketListBox::DoLineMouseAction(int iLineNumber)
{
	if (::CheckMouseIn(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13))
	{
		if (MouseLButtonPush)
		{
			SLSetSelectLine(m_iCurrentRenderEndLine + iLineNumber + 1);
			m_TextListIter->m_bIsSelected = (m_TextListIter->m_bIsSelected + 1) % 2;	// T/F Reverse
			g_pWindowMgr->SendUIMessageToWindow(GetParentUIID(), UI_MESSAGE_LISTDBLCLICK, GetUIID(), 0);
			MouseLButtonPush = false;
		}
	}
	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void CUISocketListBox::DeleteText(int iSocketIndex)
{
	BOOL bFind = FALSE;
	for (m_TextListIter = m_TextList.begin(); m_TextListIter != m_TextList.end(); ++m_TextListIter)
	{
		if (m_TextListIter->m_iSocketIndex == iSocketIndex)
		{
			bFind = TRUE;
			break;
		}
	}
	if (bFind == FALSE) return;
	
	if (m_TextList.size() == 1)
	{
		m_TextList.erase(m_TextListIter);
		SLSetSelectLine(0);
		return;
	}
	if (SLGetSelectLineNum() != 1) SLSelectNextLine();
	m_TextList.erase(m_TextListIter);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif	// ADD_SOCKET_MIX

CUIRenderText::CUIRenderText() : m_pRenderText(NULL), m_iRenderTextType(-1) {}
CUIRenderText::~CUIRenderText() { Release(); }

CUIRenderText* CUIRenderText::GetInstance()
{
	static CUIRenderText s_RenderText;
	return &s_RenderText;
}

bool CUIRenderText::Create(int iRenderTextType, HDC hDC)
{
	if(iRenderTextType == 0)	// 구버전
	{
		m_pRenderText = new CUIRenderTextOriginal;
		if(false == m_pRenderText->Create(hDC))
			return false;
	}
	else if(iRenderTextType == 1)	// 새버전
	{
		//m_pRenderText = new CUIRenderTextAdvance;		/* 사용하지 않으므로 */
	}
	return true;
}
void CUIRenderText::Release()
{
	if(m_pRenderText)
	{
		delete m_pRenderText;
		m_pRenderText = NULL;
	}
}

int CUIRenderText::GetRenderTextType() const { return m_iRenderTextType; }

HDC CUIRenderText::GetFontDC() const
{
	if(m_pRenderText)
		return m_pRenderText->GetFontDC();
	return NULL;
}
BYTE* CUIRenderText::GetFontBuffer() const
{
	if(m_pRenderText)
		return m_pRenderText->GetFontBuffer();
	return NULL;
}
DWORD CUIRenderText::GetTextColor() const
{
	if(m_pRenderText)
		return m_pRenderText->GetTextColor();
	return 0;
}
DWORD CUIRenderText::GetBgColor() const
{
	if(m_pRenderText)
		m_pRenderText->GetBgColor();
	return 0;
}

void CUIRenderText::SetTextColor(BYTE byRed, BYTE byGreen, BYTE byBlue, BYTE byAlpha)
{
	if(m_pRenderText)
		m_pRenderText->SetTextColor(byRed, byGreen, byBlue, byAlpha);
}
void CUIRenderText::SetTextColor(DWORD dwColor)
{
	if(m_pRenderText)
		m_pRenderText->SetTextColor(dwColor);
}
void CUIRenderText::SetBgColor(BYTE byRed, BYTE byGreen, BYTE byBlue, BYTE byAlpha)
{
	if(m_pRenderText)
		m_pRenderText->SetBgColor(byRed, byGreen, byBlue, byAlpha);
}
void CUIRenderText::SetBgColor(DWORD dwColor)
{
	if(m_pRenderText)
		m_pRenderText->SetBgColor(dwColor);
}

void CUIRenderText::SetFont(HFONT hFont)
{
	if(m_pRenderText)
		m_pRenderText->SetFont(hFont);
}

void CUIRenderText::RenderText(int iPos_x, int iPos_y, const char* pszText, 
							   int iBoxWidth /* = 0 */, int iBoxHeight /* = 0 */, 
							   int iSort /* = RT3_SORT_LEFT */, OUT SIZE* lpTextSize /* = NULL */)
{
#ifdef SYSTEM_NOT_RENDER_TEXT
	return;
#endif // SYSTEM_NOT_RENDER_TEXT
	if(m_pRenderText)
	{
		m_pRenderText->RenderText(iPos_x, iPos_y, pszText, iBoxWidth, iBoxHeight, iSort, lpTextSize);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
CUIRenderTextOriginal::CUIRenderTextOriginal() 
{
	m_hFontDC = NULL;
	m_hBitmap = NULL;
	m_pFontBuffer = NULL;
	m_dwTextColor = m_dwBackColor = 0;
}
CUIRenderTextOriginal::~CUIRenderTextOriginal() { Release(); }

bool CUIRenderTextOriginal::Create(HDC hDC)
{
	BITMAPINFO* DIB_INFO;
    DIB_INFO = (BITMAPINFO*)new BYTE[ sizeof(BITMAPINFOHEADER) + sizeof(PALETTEENTRY) * 256 ];
    memset( DIB_INFO, 0x00, sizeof(BITMAPINFOHEADER) );
    DIB_INFO->bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    DIB_INFO->bmiHeader.biWidth       = 640*g_fScreenRate_x;		//. 640 기준
	DIB_INFO->bmiHeader.biHeight      = -(480*g_fScreenRate_y);		//. 480 기준
    DIB_INFO->bmiHeader.biPlanes      = 1;
    DIB_INFO->bmiHeader.biBitCount    = 24;
    DIB_INFO->bmiHeader.biCompression = BI_RGB;
	
	m_hBitmap = CreateDIBSection(hDC, DIB_INFO, DIB_RGB_COLORS, (void**)&m_pFontBuffer, NULL, NULL);
	m_hFontDC = CreateCompatibleDC(hDC);
	SelectObject(m_hFontDC, m_hBitmap);
	SelectObject(m_hFontDC, g_hFont);
	m_dwBackColor = 0;				//. Default Background Color;
	m_dwTextColor = 0xFFFFFFFF;		//. Default Text Color
	
	delete [] DIB_INFO;
	
	if(NULL == m_hFontDC || NULL == m_hBitmap) 
	{
		Release();
		return false;
	}
	return true;
}
void CUIRenderTextOriginal::Release()
{
	if (m_hFontDC != NULL)
	{
		DeleteDC(m_hFontDC);
		m_hFontDC = NULL;
		m_pFontBuffer = NULL;
	}
	if (m_hBitmap != NULL)
	{
		DeleteObject(m_hBitmap);
		m_hBitmap = NULL;
	}
}

HDC CUIRenderTextOriginal::GetFontDC() const { return m_hFontDC; }
BYTE* CUIRenderTextOriginal::GetFontBuffer() const { return m_pFontBuffer; }

DWORD CUIRenderTextOriginal::GetTextColor() const { return m_dwTextColor; }
DWORD CUIRenderTextOriginal::GetBgColor() const { return m_dwBackColor; }

void CUIRenderTextOriginal::SetTextColor(BYTE byRed, BYTE byGreen, BYTE byBlue, BYTE byAlpha)
{
	m_dwTextColor = (byRed)+(byGreen<<8)+(byBlue<<16)+(byAlpha<<24);
}
void CUIRenderTextOriginal::SetTextColor(DWORD dwColor) { m_dwTextColor = dwColor; }
void CUIRenderTextOriginal::SetBgColor(BYTE byRed, BYTE byGreen, BYTE byBlue, BYTE byAlpha)
{
	m_dwBackColor = (byRed)+(byGreen<<8)+(byBlue<<16)+(byAlpha<<24);
}
void CUIRenderTextOriginal::SetBgColor(DWORD dwColor) { m_dwBackColor = dwColor; }

void CUIRenderTextOriginal::SetFont(HFONT hFont) { SelectObject(m_hFontDC, hFont); }

void CUIRenderTextOriginal::WriteText(int iOffset, int iWidth, int iHeight)
{
	const int LIMIT_WIDTH = 256, LIMIT_HEIGHT = 32;		// 폰트 텍스쳐의 가로, 세로 사이즈
	
	SIZE FontDCSize = { 640*g_fScreenRate_x, 480*g_fScreenRate_y };
	int iPitch = ((FontDCSize.cx*24+31)&~31)>>3;

	BITMAP_t * pBitmapFont = &Bitmaps[BITMAP_FONT];
	for(int y = 0; y < iHeight; ++y)
	{
		int SrcIndex = y*iPitch+iOffset;
		int DstIndex = y * LIMIT_WIDTH*4;
		for(int x = 0; x < iWidth; ++x)
		{
			if((SrcIndex > iPitch*FontDCSize.cy) || (DstIndex > LIMIT_WIDTH*4*LIMIT_HEIGHT))
			{
#ifdef _DEBUG
				__asm { int 3 };			//. 절대로 들어오면 안됨: 꼭 디버깅 하세요.
#endif // _DEBUG
				return;
			}

			if(*(m_pFontBuffer+SrcIndex) == 255)	// 글자
			{
				*((unsigned int *)(pBitmapFont->Buffer + DstIndex)) = m_dwTextColor;
			}
			else									// 배경
			{
				//*((unsigned int *)(pBitmapFont->Buffer + DstIndex)) = m_dwBackColor;
				*((unsigned int *)(pBitmapFont->Buffer + DstIndex)) = 0;	//. 투과
			}
			SrcIndex += 3;
			DstIndex += 4;
		}
	}
}
void CUIRenderTextOriginal::UploadText(int sx,int sy,int Width,int Height)
{
	BITMAP_t *b = &Bitmaps[BITMAP_FONT];
	float TextureU = 0.f, TextureV = 0.f;
	if(sx < 0)
	{
		TextureU = (-sx+0.01f)/b->Width;
		Width += sx;
		sx = 0.f;
	}
	else if(sx+Width > (int)WindowWidth)
	{
		Width = WindowWidth - sx;
	}
	if(sy < 0)
	{
		TextureV = (-sy+0.01f)/b->Height;
		Height += sy;
		sy = 0.f;
	}
	else if(sy+Height > (int)WindowHeight)
	{
		Height = WindowHeight - sy;
	}
	if(Width > 0 && Height > 0 && sx+Width > 0 && sy+Height > 0)
	{
		glBindTexture(GL_TEXTURE_2D,b->TextureNumber);
		glTexImage2D(GL_TEXTURE_2D,0,b->Components,(int)b->Width,(int)b->Height,0,GL_RGBA,GL_UNSIGNED_BYTE,b->Buffer);

		float TextureUWidth = (Width+0.01f)/b->Width;
		float TextureVHeight = (Height+0.01f)/b->Height;
		RenderBitmap(BITMAP_FONT, (float)sx, (float)sy, (float)Width, (float)Height,
			TextureU, TextureV, TextureUWidth, TextureVHeight, false, false);
	}
}

void CUIRenderTextOriginal::RenderText(int iPos_x, int iPos_y, const unicode::t_char* pszText, 
										int iBoxWidth /* = 0 */, int iBoxHeight /* = 0 */, 
										int iSort /* = RT3_SORT_LEFT */, OUT SIZE* lpTextSize /* = NULL */)
{
	if (pszText == NULL || (pszText[0] == '\0' && iBoxWidth == 0)) return;
	if (strlen(pszText) <= 0 && iBoxWidth == 0) return;
	
	SIZE RealTextSize;

#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	if (pszText[0] == '\0') 
		g_pMultiLanguage->_GetTextExtentPoint32(m_hFontDC, "0", 1, &RealTextSize);
	else 
		g_pMultiLanguage->_GetTextExtentPoint32(m_hFontDC,pszText,lstrlen(pszText),&RealTextSize);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	unicode::_GetTextExtentPoint(m_hFontDC,pszText,unicode::_strlen(pszText),&RealTextSize);
	if (pszText[0] == '\0') 
		unicode::_GetTextExtentPoint(m_hFontDC, "0", 1, &RealTextSize);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	
	MU_POINTF RealBoxPos = { (float)iPos_x*g_fScreenRate_x, (float)iPos_y*g_fScreenRate_y };
	SIZEF RealBoxSize = { (float)iBoxWidth*g_fScreenRate_x, (float)iBoxHeight*g_fScreenRate_y };
	SIZEF RealRenderingSize = { RealTextSize.cx, RealTextSize.cy };		// 현재 해상도 기준
	
	if(RealBoxSize.cx == 0)
		RealBoxSize.cx = RealTextSize.cx;
	if(RealBoxSize.cy == 0)
		RealBoxSize.cy = RealTextSize.cy;

	int iTab = 0;
	int iClipMove = 0;

	if (iSort == RT3_SORT_LEFT_CLIP)
	{
		if(RealRenderingSize.cx > RealBoxSize.cx)
		{
			iClipMove = RealRenderingSize.cx - RealBoxSize.cx;
			RealRenderingSize.cx = RealBoxSize.cx;
		}
	}
	else if (iSort == RT3_SORT_LEFT)
	{
		if(RealRenderingSize.cx > RealBoxSize.cx)
			RealRenderingSize.cx = RealBoxSize.cx;
	}
	else if(iSort == RT3_SORT_CENTER)
	{
		if(RealRenderingSize.cx > RealBoxSize.cx)
		{
			iClipMove = (RealRenderingSize.cx - RealBoxSize.cx) / 2;
			RealRenderingSize.cx = RealBoxSize.cx;
		}
		else
		{
			iTab = (RealBoxSize.cx - RealRenderingSize.cx) / 2;
		}
	}
	else if (iSort == RT3_SORT_RIGHT)
	{
		if(RealRenderingSize.cx > RealBoxSize.cx)
		{
			iClipMove = RealRenderingSize.cx - RealBoxSize.cx;
			RealRenderingSize.cx = RealBoxSize.cx;
		}
		else
		{
			iTab = RealBoxSize.cx - RealRenderingSize.cx;
		}
	}
	else if (iSort == RT3_WRITE_RIGHT_TO_LEFT)
	{
		if(RealRenderingSize.cx > RealBoxSize.cx)
		{
			iClipMove = RealRenderingSize.cx - RealBoxSize.cx;
			RealRenderingSize.cx = RealBoxSize.cx;
		}
		else
		{
			iTab = RealBoxSize.cx - RealRenderingSize.cx;
		}
		RealBoxPos.x -= RealBoxSize.cx;
	}
	else if (iSort == RT3_WRITE_CENTER)
	{
		//. 중앙 정렬
		if(RealRenderingSize.cx > RealBoxSize.cx)
		{
			iClipMove = (RealRenderingSize.cx - RealBoxSize.cx) / 2;
			RealRenderingSize.cx = RealBoxSize.cx;
		}
		else
		{
			iTab = (RealBoxSize.cx - RealRenderingSize.cx) / 2;
		}
		RealBoxPos.x -= (RealBoxSize.cx/2);
	}

	const int LIMIT_WIDTH = 256;

#ifndef DEBUG_FONT_TEXTURE_TEST
	if(m_dwBackColor != 0)
#endif //! DEBUG_FONT_TEXTURE_TEST
	{	//. 배경 박스 그리기
		EnableAlphaTest();
		glColor4ub(GetRed(m_dwBackColor), GetGreen(m_dwBackColor), 
			GetBlue(m_dwBackColor), GetAlpha(m_dwBackColor));
		RenderColor(RealBoxPos.x/g_fScreenRate_x, RealBoxPos.y/g_fScreenRate_y, 
			RealBoxSize.cx/g_fScreenRate_x, RealBoxSize.cy/g_fScreenRate_y);
		EndRenderColor();
	}

	if(pszText[0] != 0x0a)
	{
		::SetBkColor(m_hFontDC, RGB(0, 0, 0));
		::SetTextColor(m_hFontDC, RGB(255,255,255));
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		g_pMultiLanguage->_TextOut(m_hFontDC, 0, 0, pszText, lstrlen(pszText));
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE 
		unicode::_TextOut(m_hFontDC, 0, 0, pszText, unicode::_strlen(pszText));
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	}
	
	int iRealRenderWidth = RealRenderingSize.cx;
	int iNumberOfSections = (RealRenderingSize.cx / LIMIT_WIDTH) + ((iRealRenderWidth % LIMIT_WIDTH >= 0) ? 1 : 0);
	for(int i=0; i<iNumberOfSections; i++)
	{
		SIZE RealSectionLine = { LIMIT_WIDTH, RealRenderingSize.cy };
		if(i == iNumberOfSections-1)
			RealSectionLine.cx = iRealRenderWidth % LIMIT_WIDTH;
		
		WriteText(LIMIT_WIDTH*i*3+iClipMove, RealSectionLine.cx, RealSectionLine.cy);
		UploadText(RealBoxPos.x+LIMIT_WIDTH*i+iTab, RealBoxPos.y, RealSectionLine.cx, RealSectionLine.cy);
	}

	if(lpTextSize)
	{
		lpTextSize->cx = RealRenderingSize.cx/g_fScreenRate_x;
		lpTextSize->cy = RealRenderingSize.cy/g_fScreenRate_y;
	}
}

DWORD g_dwBKConv = IME_CMODE_ALPHANUMERIC;
DWORD g_dwBKSent = IME_SMODE_NONE;
BOOL g_bForceIMEConv = FALSE;
BOOL g_bForceIMESent = FALSE;
BOOL g_bBKOpenState = TRUE;
BOOL g_bIMEBlock = FALSE;

void SaveIMEStatus()
{
	HIMC hIMC = ImmGetContext(g_hWnd);
	ImmGetConversionStatus(hIMC, &g_dwBKConv, &g_dwBKSent);
	ImmSetConversionStatus(hIMC, IME_CMODE_ALPHANUMERIC, IME_SMODE_NONE);
	ImmReleaseContext(g_hWnd, hIMC);
}

void RestoreIMEStatus()
{
	HIMC hIMC = ImmGetContext(g_hWnd);
	ImmSetConversionStatus(hIMC, g_dwBKConv, g_dwBKSent);
	ImmReleaseContext(g_hWnd, hIMC);
}

void CheckTextInputBoxIME(int iMode)
{
	if (g_bIMEBlock == FALSE) return;
	if (iMode & IME_CONVERSIONMODE)
	{
		if(/*InputEnable == false && */g_bForceIMEConv == TRUE)
		{
			g_bForceIMEConv = FALSE;
			return;
		}

		HIMC hIMC = ImmGetContext(g_hWnd);

		DWORD dwConv, dwSent;
		ImmGetConversionStatus(hIMC, &dwConv, &dwSent);
		if (dwConv != IME_CMODE_ALPHANUMERIC)
		{
			g_dwBKConv = dwConv;

			// IME 상태를 영문으로
			//CreateWhisper("","IMEConv 상태를 영문으로",3);
			g_bForceIMEConv = TRUE;
			ImmSetConversionStatus(hIMC, IME_CMODE_ALPHANUMERIC, IME_SMODE_NONE);
		}

		ImmReleaseContext(g_hWnd, hIMC);
//		g_bForceIMEConv = FALSE;
	}
	if (iMode & IME_SENTENCEMODE)
	{
		if(/*InputEnable == false && */g_bForceIMESent == TRUE)
		{
			g_bForceIMESent = FALSE;
			return;
		}

		HIMC hIMC = ImmGetContext(g_hWnd);
		DWORD dwConv, dwSent;
		ImmGetConversionStatus(hIMC, &dwConv, &dwSent);
		if (dwSent != IME_SMODE_NONE)
		{
			g_dwBKSent = dwSent;
			g_bForceIMESent = TRUE;
			ImmSetConversionStatus(hIMC, IME_CMODE_ALPHANUMERIC, IME_SMODE_NONE);
		}
		ImmReleaseContext(g_hWnd, hIMC);
//		g_bForceIMESent = FALSE;
	}
}

CUITextInputBox::CUITextInputBox()
{
	m_hParentWnd = NULL;
	m_hEditWnd = NULL;
	m_hOldProc = NULL;
	m_hMemDC = NULL;
	m_hBitmap = NULL;
	m_pFontBuffer = NULL;

	m_dwTextColor = _ARGB(255, 255, 255, 255);
	m_dwBackColor = _ARGB(255, 0, 0, 0);
	m_dwSelectBackColor = _ARGB(255, 150, 150, 150);
	m_iCaretBlinkTemp = 0;
	m_fCaretWidth = 0;
	m_fCaretHeight = 0;

	m_iRealWindowPos_x = 0;
	m_iRealWindowPos_y = 0;
	m_bIsReady = FALSE;
	m_bLock = FALSE;
	m_bPasswordInput = FALSE;
	
	SetPosition(193, 422);

	m_pTabTarget = NULL;
	m_bUseMultiLine = FALSE;
	m_bScrollBtnClick = FALSE;
	m_bScrollBarClick = FALSE;
	m_iNumLines = 0;
	m_fScrollBarWidth = 0;
	m_fScrollBarRange_top = 0;
	m_fScrollBarRange_bottom = 0;
	m_fScrollBarHeight = 0;
	m_fScrollBarPos_y = 0;
	m_fScrollBarClickPos_y = 0;
}

CUITextInputBox::~CUITextInputBox()
{
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	SetWindowLongW(m_hEditWnd, GWL_WNDPROC, (LONG)m_hOldProc);
#else //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	SetWindowLong(m_hEditWnd, GWL_WNDPROC, (LONG)m_hOldProc);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	m_hOldProc = NULL;

	if(m_hEditWnd != NULL)
	{
		DestroyWindow(m_hEditWnd);
		m_hEditWnd = NULL;
	}
	if (m_hMemDC != NULL)
	{
		DeleteDC(m_hMemDC);
		m_hMemDC = NULL;
	}
	if (m_hBitmap != NULL)
	{
		DeleteObject(m_hBitmap);
		m_hBitmap = NULL;
	}
}

BOOL ClipboardCheck(HWND hWnd)
{
	BOOL bClipboardIsNumber = TRUE;
    if (OpenClipboard(hWnd))
	{
		HGLOBAL hglb = GetClipboardData(CF_TEXT);
		if(hglb) {
			LPSTR lpstr = (LPSTR)GlobalLock(hglb);
			int iLength = strlen(lpstr);
			for (int i = 0; i < iLength; ++i)
			{
				if (lpstr[i]<'0' || lpstr[i]>'9')
				{
					bClipboardIsNumber = FALSE;
					break;
				}
			}
			GlobalUnlock(hglb);
		}
		CloseClipboard();
    }
	else
	{
		bClipboardIsNumber = FALSE;
	}
	return bClipboardIsNumber;
}

LRESULT CALLBACK EditWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	CUITextInputBox *pTextInputBox = (CUITextInputBox *)GetWindowLongW(hWnd, GWL_USERDATA);
#else //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	CUITextInputBox *pTextInputBox = (CUITextInputBox *)GetWindowLong(hWnd, GWL_USERDATA);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	
	if (pTextInputBox == NULL) 
		return 0;

#ifdef KWAK_FIX_KEY_STATE_RUNTIME_ERR
	if(	SEASON3B::IsPress(VK_UP) == TRUE
		|| SEASON3B::IsPress(VK_DOWN) == TRUE
		|| SEASON3B::IsPress(VK_LEFT) == TRUE
		|| SEASON3B::IsPress(VK_RIGHT) == TRUE)
#else // KWAK_FIX_KEY_STATE_RUNTIME_ERR
	if (HIBYTE(GetAsyncKeyState(VK_UP)) == 128 
		|| HIBYTE(GetAsyncKeyState(VK_DOWN)) == 128 
		|| HIBYTE(GetAsyncKeyState(VK_LEFT)) == 128 
		|| HIBYTE(GetAsyncKeyState(VK_RIGHT)) == 128)
#endif // KWAK_FIX_KEY_STATE_RUNTIME_ERR
	{
		pTextInputBox->m_iCaretBlinkTemp = 0;	// 방향키 눌렀을때 커서 깜박임 처리
	}

	switch(msg)
	{
#ifdef KWAK_FIX_ALT_KEYDOWN_MENU_BLOCK
	case WM_SYSKEYDOWN:
		{
			return 0;
		}
		break;
#endif // KWAK_FIX_ALT_KEYDOWN_MENU_BLOCK
	case WM_CHAR:
		pTextInputBox->m_iCaretBlinkTemp = 0;
        switch(wParam)
        {			
		case VK_ESCAPE:
			return 0;
			break;
		case VK_RETURN:
			if (g_pFriendMenu->IsHotkeyEnable() == TRUE) 
				break;	// 단축키 M 사용
			if (pTextInputBox->IsLocked() == TRUE || pTextInputBox->UseMultiline() == TRUE) 
				break;
			if (pTextInputBox->GetParentUIID() == 0)
			{
				SendMessage(pTextInputBox->GetParentHandle(), msg, wParam, lParam);
			}
			else
			{
				g_pWindowMgr->SendUIMessageToWindow(pTextInputBox->GetParentUIID(), UI_MESSAGE_TEXTINPUT, 0, 0);
			}
			return 0;
			break;
		case VK_TAB:
			if (pTextInputBox->GetTabTarget() != NULL && pTextInputBox->GetTabTarget()->GetState() == UISTATE_NORMAL)
			{
				pTextInputBox->GetTabTarget()->GiveFocus(TRUE);
				pTextInputBox->GetTabTarget()->m_iCaretBlinkTemp = 0;
			}
			else if (pTextInputBox->GetParentUIID() == 0)
			{
				SendMessage(pTextInputBox->GetParentHandle(), msg, wParam, lParam);
			}
			return 0;
			break;
		default:
			{
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
				wchar_t Char = (wchar_t)(wParam);
#else //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
				char Char = (TCHAR)(wParam);           
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
				if (Char>=25 && Char<=28) return 0;

#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
				// 전각으로 된 기본 글자들을 반각으로 변환
				wParam = g_pMultiLanguage->ConvertFulltoHalfWidthChar(wParam);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE

				if (pTextInputBox->CheckOption(UIOPTION_NUMBERONLY))
				{
					if (wParam == VK_BACK);
					else if (wParam == 0x03 || wParam == 0x18);	// Ctrl+C, Ctrl+X
					else if (Char==0x16 && ClipboardCheck(hWnd) == TRUE);	// CTRL+V
					else if (Char<'0' || Char>'9') return 0;
				}
				else if (pTextInputBox->CheckOption(UIOPTION_SERIALNUMBER))
				{
					if (wParam == VK_BACK);
					else if (Char>='0' && Char<='9');
					else if (Char>='A' && Char<='Z');
					else if (Char>='a' && Char<='z') wParam -= 32;
					else return 0;
				}
#ifdef LJH_ADD_RESTRICTION_ON_ID
				else if (pTextInputBox->CheckOption(UIOPTION_NOLOCALIZEDCHARACTERS))
				{
					if (wParam == VK_BACK);
					else if (Char >= 33 && Char <= 126);
					else return 0;
				}
#endif //LJH_ADD_RESTRICTION_ON_ID
			}
			break;
		}
		break;
	case WM_IME_COMPOSITION:
	case WM_IME_STARTCOMPOSITION:
	case WM_IME_ENDCOMPOSITION:
	case WM_IME_NOTIFY:
		pTextInputBox->SetIMEPosition();

		SendMessage(pTextInputBox->GetParentHandle(), msg, wParam, lParam);
		
		if (msg == WM_IME_NOTIFY && (pTextInputBox->CheckOption(UIOPTION_SERIALNUMBER) || pTextInputBox->CheckOption(UIOPTION_NUMBERONLY)))
		{
			switch (wParam)
			{
			case IMN_SETOPENSTATUS:
				{
					CheckTextInputBoxIME(IME_CONVERSIONMODE);
				}
				break;
			}
		}
		break;
	default:
		if (PressKey(VK_F5))
			g_pFriendMenu->ShowMenu(TRUE);
		break;
	}
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	return CallWindowProcW(pTextInputBox->m_hOldProc, hWnd, msg, wParam, lParam );
#else //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	return CallWindowProc(pTextInputBox->m_hOldProc, hWnd, msg, wParam, lParam );
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void CUITextInputBox::SetIMEPosition()
{
	if (m_bIsReady == FALSE || m_hEditWnd == NULL) return;

	int iSetPos_x = m_iPos_x * g_fScreenRate_x + WindowWidth;
	int iSetPos_y = m_iPos_y * g_fScreenRate_y + WindowHeight;

	int iTargetPos_x = iSetPos_x - m_iRealWindowPos_x - WindowWidth;
	int iTargetPos_y = iSetPos_y - m_iRealWindowPos_y - WindowHeight;

	POINT pt = {0,0};
	GetCaretPos(&pt);
	iTargetPos_x += pt.x;
	iTargetPos_y += pt.y;

	HIMC hIMC = ImmGetContext(m_hEditWnd);
	if (hIMC == NULL) return;

	COMPOSITIONFORM cpf;
	ImmGetCompositionWindow(hIMC, &cpf);
	cpf.dwStyle = CFS_FORCE_POSITION;

	if (cpf.ptCurrentPos.x == iTargetPos_x && cpf.ptCurrentPos.y == iTargetPos_y)
	{
		ImmReleaseContext(m_hEditWnd, hIMC);
		return;
	}

	cpf.ptCurrentPos.x = iTargetPos_x;
	cpf.ptCurrentPos.y = iTargetPos_y;

	ImmSetCompositionWindow(hIMC, &cpf);
	ImmReleaseContext(m_hEditWnd, hIMC);
}

void CUITextInputBox::GetText(char * pszText, int iGetLenght)
{
	if (pszText == NULL) return;
	GetWindowText(m_hEditWnd, pszText, iGetLenght);
}

#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
void CUITextInputBox::GetText(wchar_t * pwszText, int iGetLenght)
{
	if (pwszText == NULL) return;
	GetWindowTextW(m_hEditWnd, pwszText, iGetLenght);
}
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE

void CUITextInputBox::SetText(const char * pszText)
{
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE 
	if (pszText == NULL)
	{
		SetWindowTextW(m_hEditWnd, L"");
		return ;
	}
	
	std::wstring wstrText;
	g_pMultiLanguage->ConvertCharToWideStr(wstrText, pszText);
	
	if (wstrText.length() > MAX_TEXT_LENGTH) return;
	SetWindowTextW(m_hEditWnd, wstrText.c_str());

#else //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	if (pszText == NULL)	// 지워라
	{
		SetWindowText(m_hEditWnd, "");
	}
	else	// 바꿔라
	{
		if (strlen(pszText) > MAX_TEXT_LENGTH) return;
		SetWindowText(m_hEditWnd, pszText);
	}
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void CUITextInputBox::SetTextLimit(int iLimit)
{
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	SendMessageW(m_hEditWnd, EM_SETLIMITTEXT, iLimit, 0);
#else //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	SendMessage(m_hEditWnd, EM_SETLIMITTEXT, iLimit, 0);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void CUITextInputBox::SetSize(int iWidth, int iHeight)
{
	if(iWidth == 0 || iHeight == 0) return;
	if(iWidth == m_iWidth && iHeight == m_iHeight) return;
	
	m_iWidth = iWidth;
	m_iHeight = iHeight;
	
	//. DC를 사이즈에 맞게 지우고 새로 생성
	if (m_hMemDC != NULL)
	{
		DeleteDC(m_hMemDC);
		m_hMemDC = NULL;
		m_pFontBuffer = NULL;
	}
	if (m_hBitmap != NULL)
	{
		DeleteObject(m_hBitmap);
		m_hBitmap = NULL;
	}

	HDC hDC = GetDC(m_hParentWnd);
	BITMAPINFO * DIB_INFO;
	DIB_INFO = (BITMAPINFO*)new BYTE[sizeof(BITMAPINFOHEADER) + sizeof(PALETTEENTRY) * 256];
	memset( DIB_INFO, 0x00, sizeof(BITMAPINFOHEADER) );
	DIB_INFO->bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	DIB_INFO->bmiHeader.biWidth       = iWidth*g_fScreenRate_x;
	DIB_INFO->bmiHeader.biHeight      = -(iHeight*g_fScreenRate_y);
	DIB_INFO->bmiHeader.biPlanes      = 1;
	DIB_INFO->bmiHeader.biBitCount    = 24;
	DIB_INFO->bmiHeader.biCompression = BI_RGB;
	
	m_hBitmap = CreateDIBSection(hDC, DIB_INFO, DIB_RGB_COLORS, (void**)&m_pFontBuffer, NULL, NULL);
	m_hMemDC = CreateCompatibleDC(hDC);
	SelectObject(m_hMemDC, m_hBitmap);
	SetFont(g_hFont);
	
	delete [] DIB_INFO;
	
	if(!m_hMemDC || !m_hBitmap) 
	{
		if(m_hEditWnd != NULL)
		{
			DestroyWindow(m_hEditWnd);
			m_hEditWnd = NULL;
		}
		if (m_hMemDC != NULL)
		{
			DeleteDC(m_hMemDC);
			m_hMemDC = NULL;
			m_pFontBuffer = NULL;
		}
		if (m_hBitmap != NULL)
		{
			DeleteObject(m_hBitmap);
			m_hBitmap = NULL;
		}
		return;
	}
	
	if (m_hEditWnd != NULL)
	{
		SetWindowPos(m_hEditWnd, 0, 0, 0, iWidth*g_fScreenRate_x, iHeight*g_fScreenRate_y, SWP_NOMOVE | SWP_NOZORDER);
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		SendMessageW(m_hEditWnd, EM_SETSEL, (WPARAM)0, (LPARAM)0);
#else //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		SendMessage(m_hEditWnd, EM_SETSEL, (WPARAM)0, (LPARAM)0);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void CUITextInputBox::Init(HWND hWnd, int iWidth, int iHeight, int iMaxLength, BOOL bIsPassword)
{
	m_hParentWnd = hWnd;
	
	DWORD dwOptionFlag = 0;
	if (bIsPassword == TRUE)
	{
		dwOptionFlag |= ES_PASSWORD;
		m_bPasswordInput = TRUE;
	}
	if (m_bUseMultiLine == TRUE)
	{
		dwOptionFlag |= ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL;
	}
	else
	{
		dwOptionFlag |= ES_AUTOHSCROLL;
	}
	
	m_iRealWindowPos_x = m_iPos_x * g_fScreenRate_x + WindowWidth;
	m_iRealWindowPos_y = m_iPos_y * g_fScreenRate_y + WindowHeight;

#ifdef DEBUG_FONT_TEXTURE_TEST
	static int iMove = 0;		//! test
	iMove += 30;
	m_iRealWindowPos_x = 0;
	m_iRealWindowPos_y = iMove;
#endif // #ifdef DEBUG_FONT_TEXTURE_TEST

#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	m_hEditWnd = CreateWindowW(L"edit", NULL,WS_CHILD | WS_VISIBLE | dwOptionFlag,
		m_iRealWindowPos_x, m_iRealWindowPos_y, iWidth*g_fScreenRate_x, iHeight*g_fScreenRate_y,
		m_hParentWnd, (HMENU)ID_UICEDIT,g_hInst, NULL);
	
	SetSize(iWidth, iHeight);
	if(m_hEditWnd)
	{
		SetTextLimit(iMaxLength);
		m_hOldProc = (WNDPROC)SetWindowLongW(m_hEditWnd, GWL_WNDPROC, (LONG)EditWndProc);
		SetWindowLongW(m_hEditWnd, GWL_USERDATA, (LONG)this);
		ShowWindow(m_hEditWnd, SW_HIDE);
	}
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	m_hEditWnd = CreateWindow("edit", NULL,WS_CHILD | WS_VISIBLE | dwOptionFlag,
		m_iRealWindowPos_x, m_iRealWindowPos_y, iWidth*g_fScreenRate_x, iHeight*g_fScreenRate_y,
		m_hParentWnd, (HMENU)ID_UICEDIT,g_hInst, NULL);
	
	SetSize(iWidth, iHeight);
	if(m_hEditWnd)
	{
		SetTextLimit(iMaxLength);
		m_hOldProc = (WNDPROC)SetWindowLong(m_hEditWnd, GWL_WNDPROC, (LONG)EditWndProc);
		SetWindowLong(m_hEditWnd, GWL_USERDATA, (LONG)this);
		ShowWindow(m_hEditWnd, SW_HIDE);
	}
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE

#ifdef PBG_ADD_INGAMESHOPMSGBOX
	m_bUseScrollbarRender = true;
#endif //PBG_ADD_INGAMESHOPMSGBOX
}

void CUITextInputBox::SetState(int iState)
{
	if (m_hEditWnd == NULL) return;
	m_iState = iState;
	if (m_iState == UISTATE_HIDE)
		ShowWindow(m_hEditWnd, SW_HIDE);
	else
	{
		ShowWindow(m_hEditWnd, SW_SHOW);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void CUITextInputBox::GiveFocus(BOOL bSel)
{
	if (m_hEditWnd == NULL) return;

	if (g_iChatInputType == 1 && GetFocus() == g_hWnd && !CheckOption(UIOPTION_SERIALNUMBER) && !CheckOption(UIOPTION_NUMBERONLY))
	{
		SetFocus(m_hEditWnd);
		RestoreIMEStatus();
	}
	else
		SetFocus(m_hEditWnd);
	g_dwKeyFocusUIID = GetUIID();
	if (bSel == TRUE)
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE 
		PostMessageW(m_hEditWnd, EM_SETSEL, ( WPARAM)0, ( LPARAM)-1);	// 전체 선택
#else //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		PostMessage(m_hEditWnd, EM_SETSEL, ( WPARAM)0, ( LPARAM)-1);	// 전체 선택
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE	
	else
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE 
		PostMessageW(m_hEditWnd, EM_SETSEL, ( WPARAM)-2, ( LPARAM)-1);	// 커서 맨 뒤로
#else //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		PostMessage(m_hEditWnd, EM_SETSEL, ( WPARAM)-2, ( LPARAM)-1);	// 커서 맨 뒤로
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE	
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CUITextInputBox::UploadText(int sx,int sy,int Width,int Height)
{
	BITMAP_t *b = &Bitmaps[BITMAP_FONT];
	float TextureU = 0.f, TextureV = 0.f;
	if(sx < 0)
	{
		TextureU = (-sx+0.01f)/b->Width;
		Width += sx;
		sx = 0.f;
	}
	else if(sx+Width > (int)WindowWidth)
	{
		Width = WindowWidth - sx;
	}
	if(sy < 0)
	{
		TextureV = (-sy+0.01f)/b->Height;
		Height += sy;
		sy = 0.f;
	}
	else if(sy+Height > (int)WindowHeight)
	{
		Height = WindowHeight - sy;
	}
	if(Width > 0 && Height > 0 && sx+Width > 0 && sy+Height > 0)
	{
		glBindTexture(GL_TEXTURE_2D,b->TextureNumber);
		glTexImage2D(GL_TEXTURE_2D,0,b->Components,(int)b->Width,(int)b->Height,0,GL_RGBA,GL_UNSIGNED_BYTE,b->Buffer);

		float TextureUWidth = (Width+0.01f)/b->Width;
		float TextureVHeight = (Height+0.01f)/b->Height;
		RenderBitmap(BITMAP_FONT, (float)sx, (float)sy, (float)Width, (float)Height,
			TextureU, TextureV, TextureUWidth, TextureVHeight, false, false);
	}
}

void CUITextInputBox::WriteText(int iOffset, int iWidth, int iHeight)
{
	BOOL bIsCaretTime = (GetFocus() == m_hEditWnd && m_iCaretBlinkTemp % 24 < 12);
	POINT pt;
	GetCaretPos(&pt);

	SIZE RealBoxSize = { m_iWidth*g_fScreenRate_x, m_iHeight*g_fScreenRate_y };
	const int LIMIT_WIDTH = 256, LIMIT_HEIGHT = 32;		// 폰트 텍스쳐의 가로, 세로 사이즈
	int iPitch = ((RealBoxSize.cx*24+31)&~31)>>3;

	int iSectionX = (iOffset % iPitch) / (LIMIT_WIDTH*3);
	int iSectionY = iOffset / (iPitch*LIMIT_HEIGHT);

	RECT rcCaret = { pt.x-LIMIT_WIDTH*iSectionX, pt.y-LIMIT_HEIGHT*iSectionY, 
		pt.x-LIMIT_WIDTH*iSectionX+m_fCaretWidth, pt.y-LIMIT_HEIGHT*iSectionY+m_fCaretHeight };

	BITMAP_t * pBitmapFont = &Bitmaps[BITMAP_FONT];
	for(int y = 0; y < iHeight; ++y)
	{
		int SrcIndex = y*iPitch+iOffset;
		int DstIndex = y * LIMIT_WIDTH*4;
		for(int x = 0; x < iWidth; ++x)
		{
			POINT ptProcessing = { x, y };
			if((SrcIndex > iPitch*RealBoxSize.cy) || (DstIndex > LIMIT_WIDTH*4*LIMIT_HEIGHT))
			{
#ifdef _DEBUG
				__asm { int 3 };			//. 절대로 들어오면 안됨: 꼭 디버깅 하세요.
#endif // _DEBUG
				return;
			}

			if(*(m_pFontBuffer+SrcIndex) == 255)	// 글자
			{
				//. 현재 픽셀이 캐럿 영역안에 있는가?
				if(bIsCaretTime && PtInRect(&rcCaret, ptProcessing))
					*((unsigned int *)(pBitmapFont->Buffer + DstIndex)) = m_dwBackColor;
				else
					*((unsigned int *)(pBitmapFont->Buffer + DstIndex)) = m_dwTextColor;
			}
			else if(*(m_pFontBuffer+SrcIndex) == 0)	// 배경
			{
				//. 현재 픽셀이 캐럿 영역안에 있는가?
				if(bIsCaretTime && PtInRect(&rcCaret, ptProcessing))
					*((unsigned int *)(pBitmapFont->Buffer + DstIndex)) = _ARGB(255, 200, 200, 200);
				else
					*((unsigned int *)(pBitmapFont->Buffer + DstIndex)) = m_dwBackColor;
			}
			else			// 선택영역 배경
			{
				//. 현재 픽셀이 캐럿 영역안에 있는가?
				if(bIsCaretTime && PtInRect(&rcCaret, ptProcessing))
					*((unsigned int *)(pBitmapFont->Buffer + DstIndex)) = _ARGB(255, 200, 200, 200);
				else
					*((unsigned int *)(pBitmapFont->Buffer + DstIndex)) = m_dwSelectBackColor;
			}
			SrcIndex += 3;
			DstIndex += 4;
		}
	}
}
void CUITextInputBox::Render()
{
	m_bIsReady = TRUE;
	if (m_hEditWnd == NULL || IsWindowVisible(m_hEditWnd) == FALSE) return;

	POINT RealWndPos = { m_iPos_x*g_fScreenRate_x, m_iPos_y*g_fScreenRate_y };
	SIZE RealWndSize = { m_iWidth*g_fScreenRate_x, m_iHeight*g_fScreenRate_y };

	//. Caret Setting
	m_fCaretWidth = 2.f;
	if (m_fCaretHeight == 0)
	{
		SIZE TextSize;
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		g_pMultiLanguage->_GetTextExtentPoint32(m_hMemDC, "Q", 1, &TextSize);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		unicode::_GetTextExtentPoint(m_hMemDC, "Q", 1, &TextSize);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		m_fCaretHeight = TextSize.cy;
	}
	
	if (CheckOption(UIOPTION_PAINTBACK))
	{
		EnableAlphaTest();
		glColor4f(0.f, 0.f, 0.f, 1.f);
		RenderColor(m_iPos_x, m_iPos_y, m_iWidth, m_iHeight);
		EndRenderColor();
	}
	
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	CallWindowProcW(m_hOldProc, m_hEditWnd, WM_ERASEBKGND, (WPARAM)m_hMemDC, 0);
	CallWindowProcW(m_hOldProc, m_hEditWnd, WM_PAINT, (WPARAM)m_hMemDC, 0);
#else //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	CallWindowProc(m_hOldProc, m_hEditWnd, WM_ERASEBKGND, (WPARAM)m_hMemDC, 0);
	CallWindowProc(m_hOldProc, m_hEditWnd, WM_PAINT, (WPARAM)m_hMemDC, 0);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE

	const int LIMIT_WIDTH = 256, LIMIT_HEIGHT = 32;
	SIZE RealTextLine = { 0, 0 };
	
	if (m_bUseMultiLine == FALSE)
	{
		char TextCheck[MAX_TEXT_LENGTH + 1] = {0, };
		GetText(TextCheck);
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		wchar_t TextCheckUTF16[MAX_TEXT_LENGTH+1] = {'\0'};
		GetText(TextCheckUTF16);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		SIZE TextSize;

		if (IsPassword() == FALSE)
		{
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
			g_pMultiLanguage->_GetTextExtentPoint32(m_hMemDC, TextCheckUTF16, wcslen(TextCheckUTF16), &TextSize);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
			unicode::_GetTextExtentPoint(m_hMemDC, TextCheck, strlen(TextCheck), &TextSize);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		}
		else
		{
			char szPasswd[MAX_TEXT_LENGTH + 1];
			memset(szPasswd, '*', MAX_TEXT_LENGTH);
			szPasswd[MAX_TEXT_LENGTH] = '\0';
			g_pRenderText->SetFont(g_hFontBold);

#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
			g_pMultiLanguage->_GetTextExtentPoint32(m_hMemDC, szPasswd, strlen(TextCheck), &TextSize);	
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
			unicode::_GetTextExtentPoint(m_hMemDC, szPasswd, strlen(TextCheck), &TextSize);	
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
			g_pRenderText->SetFont(g_hFont);
		}
		RealTextLine.cx = TextSize.cx + m_fCaretWidth;
		RealTextLine.cy = (TextSize.cy > m_fCaretHeight) ? TextSize.cy : m_fCaretHeight;
		if(RealTextLine.cx > RealWndSize.cx)		//. 렌더영역 너비보다 문자열 너비가 크다면
			RealTextLine.cx = RealWndSize.cx;
		if(RealTextLine.cy > RealWndSize.cy)		//. 렌더영역 너비보다 문자열 높이가 크다면
			RealTextLine.cy = RealWndSize.cy;
		
		//. 256(텍스쳐 사이즈)을 넘어가는 텍스트는 x개의 구간으로 쪼깬다.
		//. 256을 넘지 않으면 iNumberOfSections == 1이 된다
		int iNumberOfSections = (RealTextLine.cx / LIMIT_WIDTH) + ((RealTextLine.cx % LIMIT_WIDTH >= 0) ? 1 : 0);
		for(int i=0; i<iNumberOfSections; i++)
		{
			SIZE RealSectionLine = { LIMIT_WIDTH, RealTextLine.cy };
			if(i == iNumberOfSections-1)
				RealSectionLine.cx = RealTextLine.cx % LIMIT_WIDTH;
			
			WriteText(LIMIT_WIDTH*i*3, RealSectionLine.cx, RealSectionLine.cy);
			UploadText(RealWndPos.x+LIMIT_WIDTH*i, RealWndPos.y, RealSectionLine.cx, RealSectionLine.cy);	

#ifdef DEBUG_FONT_TEXTURE_TEST
			EnableAlphaTest();
			glColor4f(0.0f, 0.7f, 0.0f, 0.3f);
			RenderColor((RealWndPos.x+LIMIT_WIDTH*i)/g_fScreenRate_x, RealWndPos.y/g_fScreenRate_y, 
				RealSectionLine.cx/g_fScreenRate_x, RealSectionLine.cy/g_fScreenRate_y);
			//DisableAlphaBlend();
			EndRenderColor();
			//SEASON3B::RenderImage(BITMAP_FONT, 0, 250, 256, 32);	//! test
#endif	// DEBUG_FONT_TEXTURE_TEST
		}
	}
	else							//. 멀티 라인
	{
		m_iNumLines = RealWndSize.cy / m_fCaretHeight;
		RealTextLine.cx = RealWndSize.cx; 
		RealTextLine.cy = RealWndSize.cy;

		int iNumberOfXSections = (RealTextLine.cx / LIMIT_WIDTH) + ((RealTextLine.cx % LIMIT_WIDTH > 0) ? 1 : 0);
		int iNumberOfYSections = (RealTextLine.cy / LIMIT_HEIGHT) + ((RealTextLine.cy % LIMIT_HEIGHT > 0) ? 1 : 0);
		int iPitch = ((RealWndSize.cx*24+31)&~31)>>3;

		for(int y=0; y<iNumberOfYSections; y++)
		{
			SIZE RealSectionLine = { LIMIT_WIDTH, LIMIT_HEIGHT };
			if(y == iNumberOfYSections-1)
				RealSectionLine.cy = RealTextLine.cy % LIMIT_HEIGHT;

			for(int x=0; x<iNumberOfXSections; x++)
			{
				if(x == iNumberOfXSections-1)
					RealSectionLine.cx = RealTextLine.cx % LIMIT_WIDTH;

				WriteText(iPitch*LIMIT_HEIGHT*y + LIMIT_WIDTH*x*3, RealSectionLine.cx, RealSectionLine.cy);
				UploadText(RealWndPos.x+LIMIT_WIDTH*x, RealWndPos.y+LIMIT_HEIGHT*y, RealSectionLine.cx, RealSectionLine.cy);
#ifdef DEBUG_FONT_TEXTURE_TEST
				EnableAlphaTest();
				glColor4f(0.0f, 0.7f, 0.0f, 0.3f);
				RenderColor((RealWndPos.x+LIMIT_WIDTH*x)/g_fScreenRate_x, (RealWndPos.y+LIMIT_HEIGHT*y)/g_fScreenRate_y, 
					RealSectionLine.cx/g_fScreenRate_x, RealSectionLine.cy/g_fScreenRate_y);
				//DisableAlphaBlend();
				EndRenderColor();
				//SEASON3B::RenderImage(BITMAP_FONT, 0, 100, 256, 32);	//! test
#endif // DEBUG_FONT_TEXTURE_TEST
			}
		}
#ifdef PBG_ADD_INGAMESHOPMSGBOX
		if(GetUseScrollbar())
#endif //PBG_ADD_INGAMESHOPMSGBOX
		RenderScrollbar();
	}

	++m_iCaretBlinkTemp;
}

void CUITextInputBox::RenderScrollbar()
{
	float fScrollPos = GetScrollPos(m_hEditWnd, SB_VERT);
	float fLineNum = SendMessage(m_hEditWnd, EM_GETLINECOUNT, 0, 0);
	//m_iNumLines = 15;
	float fLineRate = m_iNumLines / fLineNum;
	float fPosRate = fScrollPos / fLineNum;

	m_fScrollBarWidth = 13;
	m_fScrollBarRange_top = m_iPos_y + 9;
	m_fScrollBarRange_bottom = m_iPos_y + m_iHeight - 9;
	m_fScrollBarHeight = (m_fScrollBarRange_bottom - m_fScrollBarRange_top) * (fLineRate > 1 ? 1 : fLineRate);
	m_fScrollBarPos_y = m_fScrollBarRange_top
		+ (m_fScrollBarRange_bottom - m_fScrollBarRange_top) * (fPosRate > 1 ? 1 : fPosRate);
	
	//if (GetLineNum() >= m_iNumRenderLine)
	{
		if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x+m_iWidth-m_fScrollBarWidth, m_iPos_y - 4, 13.0f, 13.0f) == TRUE)
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x+m_iWidth-m_fScrollBarWidth, (float)m_iPos_y - 4,
				13.0f, 13.0f, 13.0f/16.0f, 29.0f/32.0f, -13.0f/16.0f, -13.0f/32.0f);
		else
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x+m_iWidth-m_fScrollBarWidth, (float)m_iPos_y - 4,
				13.0f, 13.0f, 0.0f, 3.0f/32.0f, 13.0f/16.0f, 13.0f/32.0f);	// ▲
		
		if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x+m_iWidth-m_fScrollBarWidth, m_iPos_y + m_iHeight - 9, 13.0f, 13.0f) == TRUE)
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x+m_iWidth-m_fScrollBarWidth, (float)m_iPos_y + m_iHeight - 9,
				13.0f, 13.0f, 13.0f/16.0f, 16.0f/32.0f, -13.0f/16.0f, -13.0f/32.0f);
		else
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x+m_iWidth-m_fScrollBarWidth, (float)m_iPos_y + m_iHeight - 9,
				13.0f, 13.0f, 0.0f, 16.0f/32.0f, 13.0f/16.0f, 13.0f/32.0f);	// ▼

		EnableAlphaTest();
		SetLineColor(2);
		RenderColor((float)m_iPos_x+m_iWidth-m_fScrollBarWidth, m_fScrollBarRange_top,1.0f,(float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
		RenderColor((float)m_iPos_x+m_iWidth-1, m_fScrollBarRange_top,1.0f,(float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
		EndRenderColor();
		DisableAlphaBlend();

		RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x+m_iWidth-m_fScrollBarWidth+1, m_fScrollBarPos_y,
			m_fScrollBarWidth-1, m_fScrollBarHeight, 0.0f, 1.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
		RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x+m_iWidth-m_fScrollBarWidth+1, m_fScrollBarPos_y,
			m_fScrollBarWidth-2, 1, 0.0f, 0.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
		RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x+m_iWidth-m_fScrollBarWidth+1, m_fScrollBarPos_y + m_fScrollBarHeight,
			m_fScrollBarWidth-2, 1, 0.0f, 2.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
	}

}

void CUITextInputBox::SetFont(HFONT hFont)
{
	if (m_hEditWnd == NULL || hFont == NULL)
		return;
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	SendMessageW(m_hEditWnd, WM_SETFONT, (UINT)hFont, FALSE);
#else //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	SendMessage(m_hEditWnd, WM_SETFONT, (UINT)hFont, FALSE);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	SelectObject(m_hMemDC, hFont);
}

BOOL CUITextInputBox::DoMouseAction()
{
	BOOL bResult = FALSE;
	if (::CheckMouseIn(m_iPos_x, m_iPos_y - 4, m_iWidth, m_iHeight + 8) == TRUE)
	{
		if (MouseLButtonPush && GetState() == UISTATE_NORMAL)
		{
			// 위로 버튼
			if(::CheckMouseIn(m_iPos_x + m_iWidth - 15, m_iPos_y - 4, 13, 13))
			{
				if (m_bScrollBtnClick == FALSE)
				{
					//Scrolling(-1);
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
					SendMessageW(m_hEditWnd, EM_SCROLL, SB_LINEUP, 0);
#else //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
					SendMessage(m_hEditWnd, EM_SCROLL, SB_LINEUP, 0);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
					PlayBuffer(SOUND_CLICK01);
					m_bScrollBtnClick = TRUE;
				}
				else if (m_bScrollBtnClick > 15)
				{
					//Scrolling(-1);
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
					SendMessageW(m_hEditWnd, EM_SCROLL, SB_LINEUP, 0);
#else //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
					SendMessage(m_hEditWnd, EM_SCROLL, SB_LINEUP, 0);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
				}
				else
				{
					if (GetParentUIID() == 0 || g_pWindowMgr->IsRenderFrame() == TRUE)
						m_bScrollBtnClick++;
				}
			}
			if(::CheckMouseIn(m_iPos_x + m_iWidth - 15, m_iPos_y + m_iHeight - 9, 13, 13))
			{
				if (m_bScrollBtnClick == FALSE)
				{
					//Scrolling(1);
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
					SendMessageW(m_hEditWnd, EM_SCROLL, SB_LINEDOWN, 0);
#else //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
					SendMessage(m_hEditWnd, EM_SCROLL, SB_LINEDOWN, 0);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
					PlayBuffer(SOUND_CLICK01);
					m_bScrollBtnClick = TRUE;
				}
				else if (m_bScrollBtnClick > 15)
				{
					//Scrolling(1);
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
					SendMessageW(m_hEditWnd, EM_SCROLL, SB_LINEDOWN, 0);
#else //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
					SendMessage(m_hEditWnd, EM_SCROLL, SB_LINEDOWN, 0);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
				}
				else
				{
					if (GetParentUIID() == 0 || g_pWindowMgr->IsRenderFrame() == TRUE)
						m_bScrollBtnClick++;
				}
			}
//			if (GetLineNum() < m_iNumRenderLine);
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
			if (SendMessageW(m_hEditWnd, EM_GETLINECOUNT, 0, 0) < m_iNumLines);
#else //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
			if (SendMessage(m_hEditWnd, EM_GETLINECOUNT, 0, 0) < m_iNumLines);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
			// 스크롤바 끌기
			else if(::CheckMouseIn(m_iPos_x + m_iWidth - 14, m_fScrollBarPos_y,
				m_fScrollBarWidth, m_fScrollBarHeight))
			{
				if (GetState() == UISTATE_NORMAL && g_dwActiveUIID == 0)
				{
					g_dwActiveUIID = GetUIID();
					SetState(UISTATE_SCROLL);
					m_fScrollBarClickPos_y = MouseY - m_fScrollBarPos_y;
				}
			}
			// 스크롤바 클릭
			else if(::CheckMouseIn(m_iPos_x + m_iWidth - 14, m_fScrollBarRange_top,
				m_fScrollBarWidth, m_fScrollBarPos_y - m_fScrollBarRange_top))
			{
				if (GetParentUIID() > 0 && g_pWindowMgr->IsRenderFrame() == FALSE);
				else if (m_bScrollBarClick == FALSE)
				{
					//Scrolling(-1 * m_iNumRenderLine);
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
					SendMessageW(m_hEditWnd, EM_SCROLL, SB_PAGEUP, 0);
#else //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
					SendMessage(m_hEditWnd, EM_SCROLL, SB_PAGEUP, 0);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
					m_bScrollBarClick = TRUE;
				}
				else if (m_bScrollBarClick > 15)
				{
					//Scrolling(-1 * m_iNumRenderLine);
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
					SendMessageW(m_hEditWnd, EM_SCROLL, SB_PAGEUP, 0);
#else //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
					SendMessage(m_hEditWnd, EM_SCROLL, SB_PAGEUP, 0);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
				}
				else
				{
					if (GetParentUIID() == 0 || g_pWindowMgr->IsRenderFrame() == TRUE)
						m_bScrollBarClick++;
				}
			}
			else if(::CheckMouseIn(m_iPos_x + m_iWidth - 14, m_fScrollBarPos_y + m_fScrollBarHeight,
				m_fScrollBarWidth, m_fScrollBarRange_bottom - m_fScrollBarPos_y - m_fScrollBarHeight))
			{
				if (GetParentUIID() > 0 && g_pWindowMgr->IsRenderFrame() == FALSE);
				else if (m_bScrollBarClick == FALSE)
				{
					//Scrolling(m_iNumRenderLine);
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
					SendMessageW(m_hEditWnd, EM_SCROLL, SB_PAGEDOWN, 0);
#else //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
					SendMessage(m_hEditWnd, EM_SCROLL, SB_PAGEDOWN, 0);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
					m_bScrollBarClick = TRUE;
				}
				else if (m_bScrollBarClick > 15)
				{
					//Scrolling(m_iNumRenderLine);
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
					SendMessageW(m_hEditWnd, EM_SCROLL, SB_PAGEDOWN, 0);
#else //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
					SendMessage(m_hEditWnd, EM_SCROLL, SB_PAGEDOWN, 0);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
				}
				else
				{
					if (GetParentUIID() == 0 || g_pWindowMgr->IsRenderFrame() == TRUE)
						m_bScrollBarClick++;
				}
			}
		}
		else
		{
			m_bScrollBtnClick = FALSE;
			m_bScrollBarClick = FALSE;
		}
		MouseOnWindow = true;
		bResult = TRUE;

		if (MouseLButtonPush)
		{
			GiveFocus(TRUE);
//			MouseLButtonPush = false;
			MouseUpdateTime = 0;
			MouseUpdateTimeMax = 6;
			//PlayBuffer(SOUND_CLICK01);

//			return TRUE;
		}
	}

	if (GetState() == UISTATE_SCROLL)
	{
		if (MouseLButtonPush)
		{
			MouseOnWindow = true;
			// 역계산
			m_fScrollBarPos_y = (float)MouseY - m_fScrollBarClickPos_y;
			int iCurrentRenderEndLine = (m_fScrollBarPos_y - m_fScrollBarRange_top + 0.5f)
				/ (m_fScrollBarRange_bottom - m_fScrollBarRange_top)
				* (float)SendMessage(m_hEditWnd, EM_GETLINECOUNT, 0, 0);
			SetScrollPos(m_hEditWnd, SB_VERT, iCurrentRenderEndLine, TRUE);
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
			SendMessageW(m_hEditWnd, EM_LINESCROLL, 0, -10000);
			SendMessageW(m_hEditWnd, EM_LINESCROLL, 0, iCurrentRenderEndLine);
#else //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
			SendMessage(m_hEditWnd, EM_LINESCROLL, 0, -10000);
			SendMessage(m_hEditWnd, EM_LINESCROLL, 0, iCurrentRenderEndLine);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		}
		else
		{
			SetState(UISTATE_NORMAL);
			if (g_dwActiveUIID == GetUIID()) g_dwActiveUIID = 0;
		}
	}
	
	return bResult;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void CUIChatInputBox::Init(HWND hWnd)
{
#if defined FOR_WORK || defined USER_WINDOW_MODE || (defined WINDOWMODE)
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
	if (g_bUseWindowMode == TRUE)
#endif
	{
		m_bFocusLose = FALSE;
		m_iBackupFocus = 0;
	}
#endif
	m_TextInputBox.Init(hWnd, 180, 14, 50);
	m_TextInputBox.SetPosition(193, 422);
	m_TextInputBox.SetTextColor(255, 255, 230, 210);
	m_TextInputBox.GiveFocus();

	m_BuddyInputBox.Init(hWnd, 50, 14, 10);
	m_BuddyInputBox.SetPosition(376, 422);
	m_BuddyInputBox.SetTextColor(255, 200, 200, 200);
	m_BuddyInputBox.SetBackColor(0, 25, 25, 25);

	m_CurrentHistoryLine = m_HistoryList.begin();
	m_bHistoryMode = FALSE;
	memset(m_szTempText, 0, MAX_TEXT_LENGTH + 1);
	SetFocus(g_hWnd);

	// 현재 IME 상태를 저장
	HIMC hIMC = ImmGetContext(g_hWnd);
	ImmGetConversionStatus(hIMC, &g_dwBKConv, &g_dwBKSent);
	ImmReleaseContext(g_hWnd, hIMC);
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void CUIChatInputBox::Reset()
{
	InputIndex = 0;
	TabMove(0);
	ClearTexts();
	RemoveHistory(TRUE);
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void CUIChatInputBox::Render()
{
#if defined FOR_WORK || defined USER_WINDOW_MODE ||(defined WINDOWMODE)
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
	if (g_bUseWindowMode == TRUE)
#endif
	{
		if (m_bFocusLose == TRUE)
		{
			if (m_iBackupFocus == 1)	// 귓속말
			{
				m_BuddyInputBox.GiveFocus();
			}
			else	// 기본
			{
				m_TextInputBox.GiveFocus();
			}
			m_bFocusLose = FALSE;
		}
	}
#endif
	m_TextInputBox.Render();
	m_BuddyInputBox.Render();
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void CUIChatInputBox::TabMove(int iBoxNumber)
{
	if (GetState() == UISTATE_HIDE) return;
	if (iBoxNumber == 1)// && GetFocus() == m_TextInputBox.GetHandle())	// 귓속말
	{
		m_BuddyInputBox.GiveFocus();
		PostMessage(m_BuddyInputBox.GetHandle(), EM_SETSEL, ( WPARAM)0, ( LPARAM)-1);
	}
	else// if (GetFocus() == m_BuddyInputBox.GetHandle())	// 기본
	{
		m_TextInputBox.GiveFocus();
		PostMessage(m_TextInputBox.GetHandle(), EM_SETSEL, ( WPARAM)0, ( LPARAM)-1);
	}
#if defined FOR_WORK || defined USER_WINDOW_MODE || (defined WINDOWMODE)
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
	if (g_bUseWindowMode == TRUE)
#endif
		m_iBackupFocus = iBoxNumber;
#endif
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void CUIChatInputBox::GetTexts(char * pText, char * pBuddyText)
{
	m_TextInputBox.GetText(pText);
	m_BuddyInputBox.GetText(pBuddyText);
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void CUIChatInputBox::ClearTexts()
{
	m_TextInputBox.SetText(NULL);
	m_BuddyInputBox.SetText(NULL);
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void CUIChatInputBox::SetState(int iState)
{
	m_BuddyInputBox.SetState(iState);
	m_TextInputBox.SetState(iState);
	if (iState == UISTATE_NORMAL)
	{
		m_TextInputBox.GiveFocus();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void CUIChatInputBox::SetFont(HFONT hFont)
{
	m_TextInputBox.SetFont(hFont);
	m_BuddyInputBox.SetFont(hFont);
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void CUIChatInputBox::SetText(BOOL bSetText, const char * pText, BOOL bSetBuddyText, const char * pBuddyText)
{
	if (bSetText == TRUE) m_TextInputBox.SetText(pText);
	if (bSetBuddyText == TRUE) m_BuddyInputBox.SetText(pBuddyText);
}
////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CUIChatInputBox::DoMouseAction()
{
	if (m_TextInputBox.DoAction() == TRUE) InputIndex = 0;
	if (m_BuddyInputBox.DoAction() == TRUE) InputIndex = 1;
	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

const int MAX_HISTORY_LINES = 10;

void CUIChatInputBox::AddHistory(const char * pszText)
{
	if (pszText == NULL || pszText[0] == NULL) return;

	char * pszSaveText = new char[strlen(pszText) + 1];
	strncpy(pszSaveText, pszText, strlen(pszText) + 1);
	m_HistoryList.push_front(pszSaveText);
	m_CurrentHistoryLine = m_HistoryList.begin();
	m_bHistoryMode = FALSE;

	RemoveHistory(FALSE);
}

void CUIChatInputBox::MoveHistory(int iDegree)
{
	if (m_HistoryList.empty() == TRUE) return;

	HIMC hIMC = ImmGetContext(m_TextInputBox.GetHandle());
	if (hIMC != NULL)
	{
		char cComText[MAX_TEXT_LENGTH + 1] = {0};
		ImmGetCompositionString(hIMC, GCS_COMPSTR, cComText, MAX_TEXT_LENGTH);
		if (cComText[0] != '\0') 
			return;

		ImmReleaseContext(m_TextInputBox.GetHandle(), hIMC);
	}
	else return;
	
	if (iDegree == 10000)
	{
		m_CurrentHistoryLine = m_HistoryList.begin();
		m_bHistoryMode = FALSE;
	}
	else if (iDegree > 0)
	{
		PlayBuffer(SOUND_CLICK01);
		for (int i = 0; i < iDegree; ++i)
		{
			if (m_CurrentHistoryLine != m_HistoryList.begin()) --m_CurrentHistoryLine;
			else
			{
				if (m_bHistoryMode == TRUE)
				{
					SetText(TRUE, m_szTempText, FALSE, NULL);
					SendMessage(m_TextInputBox.GetHandle(), EM_SETSEL, ( WPARAM)10000, ( LPARAM)-1);
					m_TextInputBox.m_iCaretBlinkTemp = 0;
					m_bHistoryMode = FALSE;
				}
				return;
			}
		}
	}
	else if (iDegree < 0)
	{
		PlayBuffer(SOUND_CLICK01);
		for (int i = 0; i > iDegree; --i)
		{
			if (m_CurrentHistoryLine != m_HistoryList.end())
			{
				if (m_bHistoryMode == FALSE)
				{
					m_TextInputBox.GetText(m_szTempText);
					m_bHistoryMode = TRUE;
				}
				else ++m_CurrentHistoryLine;
			}
		}
		if (m_CurrentHistoryLine == m_HistoryList.end()) --m_CurrentHistoryLine;
	}
	SetText(TRUE, *m_CurrentHistoryLine, FALSE, NULL);
	SendMessage(m_TextInputBox.GetHandle(), EM_SETSEL, ( WPARAM)10000, ( LPARAM)-1);
	m_TextInputBox.m_iCaretBlinkTemp = 0;
}

void CUIChatInputBox::RemoveHistory(BOOL bClear)
{
	if (bClear == TRUE)
	{
		for (m_HistoryListIter = m_HistoryList.begin(); m_HistoryListIter != m_HistoryList.end(); ++m_HistoryListIter)
		{
			if (*m_HistoryListIter != NULL)
			{
				delete [] *m_HistoryListIter;
				*m_HistoryListIter = NULL;
			}
		}
		m_HistoryList.clear();
	}
	else if (m_HistoryList.size() > MAX_HISTORY_LINES)
	{
		m_HistoryListIter = m_HistoryList.begin();
		for (int i = 0; i < MAX_HISTORY_LINES; ++i, ++m_HistoryListIter);
		for (; m_HistoryListIter != m_HistoryList.end(); ++m_HistoryListIter)
		{
			if (*m_HistoryListIter != NULL)
			{
				delete [] *m_HistoryListIter;
				*m_HistoryListIter = NULL;
			}
		}
		int nDelCount = m_HistoryList.size() - MAX_HISTORY_LINES;

		for( int i=0 ; i<nDelCount ; ++i )
			m_HistoryList.pop_back();
	}
}

void CUILoginInputBox::Init(HWND hWnd)
{
	m_TextInputBox.Init(hWnd, 100, 14, 10);
	m_TextInputBox.SetPosition(294, 300);
	m_TextInputBox.SetTextColor(255, 255, 230, 210);
	m_TextInputBox.SetBackColor(0, 45, 45, 45);
	m_TextInputBox.GiveFocus();
	
	m_BuddyInputBox.Init(hWnd, 100, 14, 10, TRUE);
	m_BuddyInputBox.SetPosition(294, 350);
	m_BuddyInputBox.SetTextColor(255, 255, 230, 210);
	m_BuddyInputBox.SetBackColor(0, 45, 45, 45);

	SetFocus(g_hWnd);
}

void CUIMercenaryInputBox::Init(HWND hWnd)
{
	m_TextInputBox.Init(hWnd, 100, 14, 10);
	m_TextInputBox.SetPosition(294, 300);
	m_TextInputBox.SetTextColor(255, 255, 230, 210);
	m_TextInputBox.SetBackColor(0, 45, 45, 45);
	m_TextInputBox.SetOption(UIOPTION_NUMBERONLY | UIOPTION_ENTERIMECHKOFF);
	m_TextInputBox.GiveFocus();
	
	m_BuddyInputBox.Init(hWnd, 100, 14, 10);
	m_BuddyInputBox.SetPosition(294, 350);
	m_BuddyInputBox.SetTextColor(255, 255, 230, 210);
	m_BuddyInputBox.SetBackColor(0, 45, 45, 45);
	m_BuddyInputBox.SetOption(UIOPTION_NUMBERONLY | UIOPTION_ENTERIMECHKOFF);

	SetFocus(g_hWnd);
}

CUISlideHelp::CUISlideHelp()
{
	m_fMaxMoveSpeed = 2.5f;
	m_dwSlideTextColor = 0;
	m_dwTimer = 0;
	m_hFont = NULL;
	m_bBlink = FALSE;
}

CUISlideHelp::~CUISlideHelp()
{
}

void CUISlideHelp::Init(BOOL bBold, BOOL bBlink)
{
	if (bBold == TRUE) 
	{
		m_hFont = g_hFontBold;
	}
	else
	{
		m_hFont = g_hFont;
	}

	m_bBlink = bBlink;

	m_dwCurrentSecond = 10;
	m_dwTimer = timeGetTime();

	memset(m_szSlideTextA, 0, SLIDE_TEXT_LENGTH);
	memset(m_szSlideTextB, 0, SLIDE_TEXT_LENGTH);

#ifdef KJW_FIX_SLIDE_MOVE
	m_fMovePosition = 640.0f;
#else // KJW_FIX_SLIDE_MOVE
	m_fMovePosition = 0.0f;
#endif // KJW_FIX_SLIDE_MOVE
	m_fMoveAccel = 1.0f;
	m_fMoveSpeed = 0;
	m_iCutLength = 0;
	m_iCutSize = 0;
	m_pszSlideText = m_szSlideTextA;
	m_iFontHeight = 0;
	m_iAlphaRate = 0;
}

BOOL CUISlideHelp::DoMouseAction()
{
	return FALSE;
}

BOOL CUISlideHelp::HaveText()
{
	return (m_pszSlideText[0] == '\0' || (int)strlen(m_pszSlideText) < m_iCutLength);
}

int g_iNoticeInverse = 0;

void CUISlideHelp::Render(BOOL bForceFadeOut)
{
	if(g_pOption->IsSlideHelp() == false) 
	{
		return;
	}

	BOOL bFadeOut = FALSE;

	if (m_pszSlideText[0] == '\0' || (int)strlen(m_pszSlideText) < m_iCutLength)
	{
		bFadeOut = TRUE;
	}
	else if (bForceFadeOut == TRUE)
	{
		bFadeOut = TRUE;
	}

	if (bFadeOut == FALSE)
	{
		if (m_iAlphaRate < 205) m_iAlphaRate += 30;
		if (m_iAlphaRate > 205) m_iAlphaRate = 205;
	}
	else
	{
		if (m_iAlphaRate > 0) m_iAlphaRate -= 30;
		if (m_iAlphaRate < 0) m_iAlphaRate = 0;
	}

	if (m_iAlphaRate <= 0)
	{
		return;
	}

	EnableAlphaTest();
	glColor4ub(0, 0, 0, (m_iAlphaRate > 180 ? m_iAlphaRate : (m_iAlphaRate - 25 < 0 ? 0 : m_iAlphaRate - 25)));

	RenderColor(0, m_iPos_y - 3, WindowWidth, 1);
	RenderColor(0, m_iPos_y + m_iFontHeight + 2, WindowWidth, 1);
	glColor4ub(0, 0, 0, (m_iAlphaRate - 25 < 0 ? 0 : m_iAlphaRate - 25));
	RenderColor(0, m_iPos_y - 2, WindowWidth, m_iFontHeight + 4);

	EndRenderColor();

	if (bFadeOut == TRUE && bForceFadeOut == FALSE)
	{
		DisableAlphaBlend();
		return;
	}

	g_pRenderText->SetFont(m_hFont);
	SlideMove();
	
	if (m_iFontHeight == 0)
	{
		SIZE TextSize = {0, 0};
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), "Z", 1, &TextSize);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		unicode::_GetTextExtentPoint(g_pRenderText->GetFontDC(), "Z", 1, &TextSize);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		m_iFontHeight = TextSize.cy /= g_fScreenRate_y;

		if(GetPosition_y() >= m_iFontHeight)	
		{
			SetPosition(GetPosition_x(), GetPosition_y() - m_iFontHeight);
		}
	}

	g_pRenderText->SetTextColor(m_dwSlideTextColor & 0x00FFFFFF);

	BYTE byAlpha = m_dwSlideTextColor >> 24;
	byAlpha = (float)byAlpha * ((m_iAlphaRate > 180 ? m_iAlphaRate : (m_iAlphaRate - 25 < 0 ? 0 : m_iAlphaRate - 25)) + 50) / 255.0f;
	if (m_bBlink == TRUE && g_iNoticeInverse % 10 < 5) byAlpha /= 2;

	g_pRenderText->SetTextColor(g_pRenderText->GetTextColor() + (byAlpha << 24));
	g_pRenderText->SetBgColor(0);
	g_pRenderText->RenderText(m_fMovePosition, m_iPos_y, m_pszSlideText);
	DisableAlphaBlend();

	ComputeSpeed();

	++g_iNoticeInverse;
}

void CUISlideHelp::SlideMove()
{
	if (m_iCutSize == 0)
	{
		m_iCutSize = CheckCutSize(m_pszSlideText, 4);
		m_iCutSize /= g_fScreenRate_x;
	}

	if (m_fMovePosition < m_iCutSize * -1)
	{
#ifdef KJW_FIX_SLIDE_MOVE
		memset(m_pszSlideText, 0, SLIDE_TEXT_LENGTH);
		m_fMovePosition = 640.0f;
		m_iCutSize = CheckCutSize(m_pszSlideText, 4);
		m_iCutSize /= g_fScreenRate_x;
#else // KJW_FIX_SLIDE_MOVE
		char * pCopyText = (m_pszSlideText == m_szSlideTextA ? m_szSlideTextB : m_szSlideTextA);
		memset(pCopyText, 0, SLIDE_TEXT_LENGTH);
		strncpy(pCopyText, m_pszSlideText + m_iCutLength, strlen(m_pszSlideText) - m_iCutLength);
		m_pszSlideText = pCopyText;
		m_fMovePosition += m_iCutSize;
		
		m_iCutSize = CheckCutSize(m_pszSlideText, 4);
		m_iCutSize /= g_fScreenRate_x;
#endif // KJW_FIX_SLIDE_MOVE
	}
}

void CUISlideHelp::ComputeSpeed()
{
	if (::CheckMouseIn(0, m_iPos_y - 3, WindowWidth, m_iFontHeight + 6) == FALSE)
	{
		m_fMoveAccel = 1.0f;
	}
	else
	{
		m_fMoveAccel = -1.0f;
	}

	m_fMoveSpeed += m_fMoveAccel;
	if (m_fMoveSpeed >= m_fMaxMoveSpeed)
	{
		m_fMoveSpeed = m_fMaxMoveSpeed;
	}
	else if (m_fMoveSpeed < 0)
	{
		m_fMoveSpeed = 0;
	}
	m_fMovePosition -= m_fMoveSpeed;
}

BOOL CUISlideHelp::AddSlideText(const char * pszNewText, DWORD dwTextColor)
{
	if (pszNewText == NULL || pszNewText[0] == '\0') return TRUE;

	m_dwSlideTextColor = dwTextColor;
	g_pRenderText->SetFont(m_hFont);

#ifndef KJW_FIX_SLIDE_MOVE
	char szSpace[SLIDE_TEXT_LENGTH];
	memset(szSpace, 0, SLIDE_TEXT_LENGTH);
	
	SIZE TextSize = {0, 0};
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), L"ZZ", 1, &TextSize);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	unicode::_GetTextExtentPoint(g_pRenderText->GetFontDC(), " ", 1, &TextSize);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	
	float fNumSpace = 640.0f / (float)TextSize.cx * g_fScreenRate_x;
	
	if (fNumSpace > SLIDE_TEXT_LENGTH)
	{
		fNumSpace = SLIDE_TEXT_LENGTH;
	}
	if (strlen(m_pszSlideText) + fNumSpace + strlen(pszNewText) > SLIDE_TEXT_LENGTH)
	{
		return FALSE;
	}

	memset(szSpace, ' ', int(fNumSpace));
	strcat(m_pszSlideText, szSpace);
#endif // KJW_FIX_SLIDE_MOVE
	strcat(m_pszSlideText, pszNewText);
	return TRUE;
}

int CUISlideHelp::CheckCutSize(const char * pszSource, int iNeedValue)
{
	if (pszSource == NULL || pszSource[0] == '\0') return 0;

	int iLength = strlen(pszSource);
	int iMove = 0;
	int iTextSize = 0;
	for (int i = 0; i < iLength; )
	{
		iTextSize = i;
		if (_mbclen((UCHAR *)&pszSource[i]) == 2)
		{
			iMove = 2;
		}
		else
		{
			iMove = 1;
		}
		if (i + iMove > iNeedValue)
		{
			break;
		}
		else
		{
			i += iMove;
		}
	}
	m_iCutLength = iTextSize;

	SIZE TextSize = {0, 0};
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), pszSource, m_iCutLength, &TextSize);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	unicode::_GetTextExtentPoint(g_pRenderText->GetFontDC(), pszSource, m_iCutLength, &TextSize);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	return TextSize.cx;
}

void CUISlideHelp::SetScrollSpeed(float fSpeed)
{
	if (fSpeed <= 0) 
	{
		fSpeed = 2.5f;
	}

	m_fMaxMoveSpeed = fSpeed * g_fScreenRate_x;
}

void CUISlideHelp::AddSlide(int iLoopCount, int iLoopDelay, const char * pszText, int iType, float fSpeed, DWORD dwTextColor)
{
	if (SceneFlag != MAIN_SCENE) return;
	if (pszText == NULL || pszText[0] == '\0') return;
	if (iLoopCount > 30) return;
	int iLength = strlen(pszText);

	// 공지 Type
	//	1	- 긴급 메시지 (중요도 상) : 다른 공지 무시하고 바로 다음번에 등장
	//	0	- 필독 메시지 (중요도 중) : 공지 시간에 맞게 등장
	//	-1	- 일반 메시지 (중요도 하) : 공지 시간에 맞게 등장하나 등장 시간이 지체될 경우 삭제됨
	SLIDE_QUEUE_DATA slidedata;
	slidedata.m_iType = iType;
	slidedata.m_pszText = new char[iLength + 1];
	memset(slidedata.m_pszText, 0, iLength + 1);
	strncpy(slidedata.m_pszText, pszText, iLength + 1);
	slidedata.m_fSpeed = fSpeed;
	slidedata.m_dwTextColor = dwTextColor;
	slidedata.m_bLastData = FALSE;

	for (int i = 0; i < iLoopCount; ++i)
	{
		if (i + 1 == iLoopCount)
		{
			slidedata.m_bLastData = TRUE;
		}

		if (iType == 1)	// 긴급 메시지
		{
			m_SlideQueue.insert(std::pair<DWORD, SLIDE_QUEUE_DATA>(0, slidedata));
		}
		else
		{
			m_SlideQueue.insert(std::pair<DWORD, SLIDE_QUEUE_DATA>(m_dwCurrentSecond + i * iLoopDelay, slidedata));
		}
	}
}

void CUISlideHelp::CheckTime()
{
	DWORD dwCheckTime = timeGetTime();
	if (dwCheckTime > m_dwTimer + 1000)
	{
		m_dwTimer = timeGetTime();
		++m_dwCurrentSecond;
	}
}

void CUISlideHelp::ManageSlide()
{
	BOOL bFadeOut = FALSE;

	if (m_pszSlideText[0] == '\0' || (int)strlen(m_pszSlideText) < m_iCutLength) 
	{
		bFadeOut = TRUE;
	}
	if (bFadeOut == FALSE) 
	{
		return;
	}

	CheckTime();
	for (m_SlideQueueIter = m_SlideQueue.begin(); m_SlideQueueIter != m_SlideQueue.end(); ++m_SlideQueueIter)
	{
		if (m_SlideQueueIter->first > m_dwCurrentSecond) break;
		else
		{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
			BOOL bResult = FALSE;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
			if ( m_SlideQueueIter->second.m_iType == -1 && m_SlideQueueIter->first + 60 < m_dwCurrentSecond );	// 시간 지나면 삭제
			else if ( AddSlideText( m_SlideQueueIter->second.m_pszText, m_SlideQueueIter->second.m_dwTextColor ) == FALSE ) break;
			
			SetScrollSpeed(m_SlideQueueIter->second.m_fSpeed);
			if (m_SlideQueueIter->second.m_bLastData == TRUE)
			{
				delete [] m_SlideQueueIter->second.m_pszText;
				m_SlideQueueIter->second.m_pszText = NULL;
			}
			m_SlideQueue.erase(m_SlideQueueIter);
			break;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////

CSlideHelpMgr::CSlideHelpMgr()
{
	m_iCreateDelay = 10;
	m_fHelpSlideSpeed = 2.5f;
}

CSlideHelpMgr::~CSlideHelpMgr()
{
	ClearSlideText();
}

void CSlideHelpMgr::Init()
{
	m_HelpSlide.Init();
	m_NoticeSlide.Init(TRUE, FALSE);

	m_HelpSlide.SetPosition(0, 0);	
	m_NoticeSlide.SetPosition(0, 0);
	//m_HelpSlide.SetPosition(0, 429);
	//m_NoticeSlide.SetPosition(0, 429);
	
	SetTimer( g_hWnd, SLIDEHELP_TIMER, m_iCreateDelay * 1000, 0);
	CreateSlideText();
}

void CSlideHelpMgr::Render()
{
	if (m_NoticeSlide.HaveText() == FALSE)
	{
		m_HelpSlide.Render(TRUE);
	}
	else if (m_NoticeSlide.GetAlphaRate() <= 0)
	{
		m_HelpSlide.Render();
	}

	if (m_HelpSlide.GetAlphaRate() <= 0)
	{
		m_NoticeSlide.Render();
	}
}

void CSlideHelpMgr::CreateSlideText()
{
	if (SceneFlag != MAIN_SCENE) 
		return;
	if(g_pOption->IsSlideHelp() == false)
	{
		return;
	}
	if (m_HelpSlide.GetAlphaRate() > 0) 
		return;

	//////////////////////////////////////////////////////////////////////////
	int iLevel = CharacterMachine->Character.Level;
	//////////////////////////////////////////////////////////////////////////

	const char * pszNewText = GetSlideText(iLevel);
	
	AddSlide(1, 0, pszNewText, 1, m_fHelpSlideSpeed);
}

void CSlideHelpMgr::OpenSlideTextFile(const char * szFileName)	// slide.bmd 로딩
{
	for (int i = 0; i < SLIDE_LEVEL_MAX; ++i)
	{
		if (!m_SlideTextList[i].empty())
		{
			ClearSlideText();
			break;
		}
	}
	
	FILE *fp = fopen(szFileName,"rb");
	if(fp == NULL)
	{
		char Text[256];
    	sprintf(Text,"%s - File not exist.",szFileName);
		g_ErrorReport.Write( Text);
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);
		return;
	}
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	int Size = sizeof(SLIDEHELP);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	SLIDEHELP SlideHelp;
	fread(&SlideHelp, sizeof(SLIDEHELP), 1, fp);
	BuxConvert((BYTE*)&SlideHelp, sizeof(SLIDEHELP));
	fclose(fp);

	SetCreateDelay(SlideHelp.iCreateDelay);
	m_fHelpSlideSpeed = SlideHelp.fSpeed;

	for (int i = 0; i < SLIDE_LEVEL_MAX; ++i)
	{
		m_iLevelCap[i] = SlideHelp.SlideHelp[i].iLevel;
		m_iTextNumber[i] = SlideHelp.SlideHelp[i].iNumber;
		for (int j = 0; j < m_iTextNumber[i]; ++j)
		{
			//if (m_SlideTextList[i].size() >= m_iTextNumber[i]) break;	// 더이상 못넣게
			int iLength = strlen(SlideHelp.SlideHelp[i].szSlideHelpText[j]);
			char * pszText = new char[iLength + 1];
			memset(pszText, 0, iLength + 1);
			strncpy(pszText, SlideHelp.SlideHelp[i].szSlideHelpText[j], iLength);
			m_SlideTextList[i].push_back(pszText);
		}
	}
}

void CSlideHelpMgr::ClearSlideText()						// 슬라이드 텍스트 지우기
{
	for (int i = 0; i < SLIDE_LEVEL_MAX; ++i)
	{
		m_iLevelCap[i] = 0;
		m_iTextNumber[i] = 0;
		for (m_SlideTextListIter = m_SlideTextList[i].begin(); m_SlideTextListIter != m_SlideTextList[i].end(); ++m_SlideTextListIter)
		{
			if (*m_SlideTextListIter != NULL)
			{
				delete [] *m_SlideTextListIter;
				*m_SlideTextListIter = NULL;
			}
		}
		m_SlideTextList[i].clear();
	}
}

const char * CSlideHelpMgr::GetSlideText(int iLevel)			// 현재 레벨의 슬라이드 텍스트 얻기
{
	int iHelpType = -1;
	for (int i = 0; i < SLIDE_LEVEL_MAX; ++i)
	{
		if (iLevel <= m_iLevelCap[i])
		{
			iHelpType = i;
			break;
		}
	}
	if (iHelpType == -1) return NULL;	// 슬라이드 표시 레벨 초과
	if (m_iTextNumber[iHelpType] == 0) return NULL;

	int iRandom = rand() % m_iTextNumber[iHelpType];
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	int ii = m_SlideTextList[iHelpType].size();
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING

	if ((unsigned int)iRandom >= m_SlideTextList[iHelpType].size()) return NULL;

	m_SlideTextListIter = m_SlideTextList[iHelpType].begin();
	for (int i = 0; i < iRandom; ++i)
		++m_SlideTextListIter;

	return *m_SlideTextListIter;
}

void CSlideHelpMgr::AddSlide(int iLoopCount, int iLoopDelay, const char * pszText, int iType, float fSpeed, DWORD dwTextColor)
{
	if (SceneFlag != MAIN_SCENE) return;
	if (pszText == NULL || pszText[0] == '\0') return;
	if (iLoopCount > 30) return;
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	int iLength = strlen(pszText);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING

	// 공지 Type
	//	5	- 긴급 공지 (중요도 최상)	: 다른 공지 무시하고 바로 다음번에 등장
	//	4	- 필독 공지 (중요도 상)		: 공지 시간에 맞게 등장
	//	3	- 일반 공지 (중요도 중상)	: 공지 시간에 맞게 등장하나 등장 시간이 지체될 경우 삭제됨
	//	2	- 긴급 메시지 (중요도 중)	: 다른 공지 무시하고 바로 다음번에 등장
	//	1	- 필독 메시지 (중요도 하)	: 공지 시간에 맞게 등장
	//	0	- 일반 메시지 (중요도 최하)	: 공지 시간에 맞게 등장하나 등장 시간이 지체될 경우 삭제됨
	switch(iType)
	{
	case 2:	case 1:	case 0:
		m_HelpSlide.AddSlide(iLoopCount, iLoopDelay, pszText, iType - 1, fSpeed, dwTextColor);
		break;
	case 5:	case 4:	case 3:
		m_NoticeSlide.AddSlide(iLoopCount, iLoopDelay, pszText, iType - 4, fSpeed, dwTextColor);
		break;
	default:
		break;
	};
}

void CSlideHelpMgr::ManageSlide()
{
	m_NoticeSlide.ManageSlide();
	m_HelpSlide.ManageSlide();
}

BOOL CSlideHelpMgr::IsIdle()
{
	return (m_NoticeSlide.HaveText() && m_HelpSlide.HaveText());
}

CUIGuildNoticeListBox::CUIGuildNoticeListBox()
{
	m_iMaxLineCount = UIMAX_TEXT_LINE;
	m_iCurrentRenderEndLine = 0;
	m_iNumRenderLine = 6;

	m_fScrollBarRange_top = 0;
	m_fScrollBarRange_bottom = 0;

	m_fScrollBarPos_y = 0;
	m_fScrollBarWidth = 13;
	m_fScrollBarHeight = 0;

	m_fScrollBarClickPos_y = 0;
	m_bScrollBtnClick = FALSE;
	m_bScrollBarClick = FALSE;

	m_bUseSelectLine = TRUE;
}

void CUIGuildNoticeListBox::AddText( const char* szContent )
{
	if( szContent == NULL || szContent[0] == '\0') return;

	static GUILDLOG_TEXT text;
	strcpy( text.m_szContent, szContent );
	m_TextList.push_front(text);

	RemoveText();
	SLSetSelectLine(0);
	if (GetLineNum() > m_iNumRenderLine) ++m_iCurrentRenderEndLine;

//	if (m_iCurrentRenderEndLine != 0) ++m_iCurrentRenderEndLine;

	if (GetLineNum() < m_iNumRenderLine);
	else if (GetLineNum() - m_iCurrentRenderEndLine < m_iNumRenderLine) 
		m_iCurrentRenderEndLine = GetLineNum() - m_iNumRenderLine;

	if (m_TextList.size() == 1)
		SLSetSelectLine(1);

	if (m_TextList.empty() == FALSE)
	{
		SLSetSelectLine(GetLineNum());
//		m_iCurrentRenderEndLine = 0;
		Scrolling(-10000);
	}
}

void CUIGuildNoticeListBox::SetNumRenderLine(int iLine)
{
	if (iLine < m_iNumRenderLine && iLine < GetLineNum()) ++m_iCurrentRenderEndLine;
	else if (iLine > GetLineNum()) m_iCurrentRenderEndLine = 0;
	m_iNumRenderLine = iLine;
}

void CUIGuildNoticeListBox::RenderInterface()
{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	return;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING

	// 배경칠
	EnableAlphaTest();
	SetLineColor(7, 0.4f);
	RenderColor(m_iPos_x - 1, m_iPos_y - m_iHeight - 1, m_iWidth + 1, m_iHeight + 2);

	EndRenderColor();

	// 스크롤 바
	ComputeScrollBar();

	//if (GetLineNum() >= m_iNumRenderLine)
	{
		//-_- 버튼
		if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - m_iHeight - 1, 13.0f, 13.0f) == TRUE)
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1,
				13.0f, 13.0f, 13.0f/16.0f, 29.0f/32.0f, -13.0f/16.0f, -13.0f/32.0f);
		else
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1,
				13.0f, 13.0f, 0.0f, 3.0f/32.0f, 13.0f/16.0f, 13.0f/32.0f);	// ▲
		
		if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - 12, 13.0f, 13.0f) == TRUE)
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12,
				13.0f, 13.0f, 13.0f/16.0f, 16.0f/32.0f, -13.0f/16.0f, -13.0f/32.0f);
		else
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12,
				13.0f, 13.0f, 0.0f, 16.0f/32.0f, 13.0f/16.0f, 13.0f/32.0f);	// ▼

		EnableAlphaTest();
		SetLineColor(2);
		RenderColor((float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 1, m_fScrollBarRange_top,(float)1,(float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
		RenderColor((float)m_iPos_x + m_iWidth, m_fScrollBarRange_top,(float)1,(float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
		EndRenderColor();

		if (GetLineNum() >= m_iNumRenderLine)
		{
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y,
				m_fScrollBarWidth - 2, m_fScrollBarHeight, 0.0f, 1.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y,
				m_fScrollBarWidth - 2, 1, 0.0f, 0.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y + m_fScrollBarHeight - 1,
				m_fScrollBarWidth - 2, 1, 0.0f, 2.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
		}
		else
		{
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top,
				m_fScrollBarWidth - 2, m_fScrollBarRange_bottom - m_fScrollBarRange_top, 0.0f, 1.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top,
				m_fScrollBarWidth - 2, 1, 0.0f, 0.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top + m_fScrollBarHeight - 1,
				m_fScrollBarWidth - 2, 1, 0.0f, 2.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
		}
	}
}

int CUIGuildNoticeListBox::GetRenderLinePos_y(int iLineNumber)
{
	if (GetLineNum() > m_iNumRenderLine)
		return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1)* 13 - iLineNumber * 13 + 3);
	else
		return (m_iPos_y - m_iHeight + (GetLineNum() - 1) * 13 - iLineNumber * 13 + 3);
}

BOOL CUIGuildNoticeListBox::RenderDataLine(int iLineNumber)
{
	EnableAlphaTest();

	if (SLGetSelectLineNum() == m_iCurrentRenderEndLine + iLineNumber + 1)
	{
		glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
		RenderColor(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		g_pRenderText->SetTextColor(0, 0, 0, 255);
	}
	else
	{
		g_pRenderText->SetTextColor(230, 220, 200, 255);
	}
	glEnable(GL_TEXTURE_2D);
	g_pRenderText->SetBgColor(0);

	int iPos_x = m_iPos_x + 4;
	int iPos_y = GetRenderLinePos_y(iLineNumber);

	// 로그내용	
	g_pRenderText->RenderText(iPos_x, iPos_y, m_TextListIter->m_szContent, 0, 0, RT3_SORT_LEFT );

	DisableAlphaBlend();

	return TRUE;
}

BOOL CUIGuildNoticeListBox::DoLineMouseAction(int iLineNumber)
{
	if (::CheckMouseIn(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13))
	{
		if (MouseLButtonPush)
		{
			SLSetSelectLine(m_iCurrentRenderEndLine + iLineNumber + 1);
			m_TextListIter->m_bIsSelected = (m_TextListIter->m_bIsSelected + 1) % 2;	// T/F Reverse
			MouseLButtonPush = false;
		}
	}
	return TRUE;
}

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
void CUIGuildNoticeListBox::DeleteText(DWORD dwGuildIndex)
{
}
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING

CUINewGuildMemberListBox::CUINewGuildMemberListBox()
{
	m_iMaxLineCount = UIMAX_TEXT_LINE;
	m_iCurrentRenderEndLine = 0;
	m_iNumRenderLine = 18;	// 3의 배수로 -_-

	m_fScrollBarRange_top = 0;
	m_fScrollBarRange_bottom = 0;

	m_fScrollBarPos_y = 0;
	m_fScrollBarWidth = 13;
	m_fScrollBarHeight = 0;

	m_fScrollBarClickPos_y = 0;
	m_bScrollBtnClick = FALSE;
	m_bScrollBarClick = FALSE;

	m_bUseSelectLine = TRUE;

	SetPosition(466, 360);
	SetSize(157, 235);

//	SetPosition(466, 345);
//	SetSize(157, 235);
}

void CUINewGuildMemberListBox::AddText(const char * pszID, BYTE Number, BYTE Server, BYTE GuildStatus)
{
	if (pszID == NULL || pszID[0] == '\0') return;

	if (GetLineNum() == 0)	// 길드마스터인가
	{
		if (strcmp(pszID, Hero->ID) == NULL) m_bIsGuildMaster = TRUE;
		else m_bIsGuildMaster = FALSE;
	}

	static GUILDLIST_TEXT text;
	text.m_bIsSelected = FALSE;
	strncpy(text.m_szID, pszID, MAX_ID_SIZE + 1);
	//memcpy(text.m_szID, pszID, strlen(pszID) + 1);
	text.m_Number = Number;
	text.m_Server = Server;
	text.m_GuildStatus = GuildStatus;
	m_TextList.push_front(text);

	RemoveText();
	SLSetSelectLine(0);
	if (GetLineNum() > m_iNumRenderLine) ++m_iCurrentRenderEndLine;

//	if (m_iCurrentRenderEndLine != 0) ++m_iCurrentRenderEndLine;

	if (GetLineNum() < m_iNumRenderLine);
	else if (GetLineNum() - m_iCurrentRenderEndLine < m_iNumRenderLine) 
		m_iCurrentRenderEndLine = GetLineNum() - m_iNumRenderLine;

	if (m_TextList.size() == 1)
		SLSetSelectLine(1);

	if (m_TextList.empty() == FALSE)
	{
		SLSetSelectLine(GetLineNum());
//		m_iCurrentRenderEndLine = 0;
		Scrolling(-10000);
	}
}

void CUINewGuildMemberListBox::SetNumRenderLine(int iLine)
{
	if (iLine < m_iNumRenderLine && iLine < GetLineNum()) ++m_iCurrentRenderEndLine;
	else if (iLine > GetLineNum()) m_iCurrentRenderEndLine = 0;
	m_iNumRenderLine = iLine;
}

void CUINewGuildMemberListBox::RenderInterface()
{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	return;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING

	// 배경칠
	EnableAlphaTest();
	SetLineColor(7, 0.4f);
	RenderColor(m_iPos_x - 1, m_iPos_y - m_iHeight - 1, m_iWidth + 1, m_iHeight + 2);


	EndRenderColor();

	// 스크롤 바
	ComputeScrollBar();

	//if (GetLineNum() >= m_iNumRenderLine)
	{
		//-_- 버튼
		if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - m_iHeight - 1, 13.0f, 13.0f) == TRUE)
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1,
				13.0f, 13.0f, 13.0f/16.0f, 29.0f/32.0f, -13.0f/16.0f, -13.0f/32.0f);
		else
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1,
				13.0f, 13.0f, 0.0f, 3.0f/32.0f, 13.0f/16.0f, 13.0f/32.0f);	// ▲
		
		if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - 12, 13.0f, 13.0f) == TRUE)
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12,
				13.0f, 13.0f, 13.0f/16.0f, 16.0f/32.0f, -13.0f/16.0f, -13.0f/32.0f);
		else
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12,
				13.0f, 13.0f, 0.0f, 16.0f/32.0f, 13.0f/16.0f, 13.0f/32.0f);	// ▼

		EnableAlphaTest();
		SetLineColor(2);
		RenderColor((float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 1, m_fScrollBarRange_top,(float)1,(float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
		RenderColor((float)m_iPos_x + m_iWidth, m_fScrollBarRange_top,(float)1,(float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
		EndRenderColor();

		if (GetLineNum() >= m_iNumRenderLine)
		{
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y,
				m_fScrollBarWidth - 2, m_fScrollBarHeight, 0.0f, 1.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y,
				m_fScrollBarWidth - 2, 1, 0.0f, 0.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y + m_fScrollBarHeight - 1,
				m_fScrollBarWidth - 2, 1, 0.0f, 2.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
		}
		else
		{
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top,
				m_fScrollBarWidth - 2, m_fScrollBarRange_bottom - m_fScrollBarRange_top, 0.0f, 1.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top,
				m_fScrollBarWidth - 2, 1, 0.0f, 0.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top + m_fScrollBarHeight - 1,
				m_fScrollBarWidth - 2, 1, 0.0f, 2.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
		}
	}

	// 목록
	g_pRenderText->RenderText(m_iPos_x + 14, m_iPos_y - m_iHeight - 12, GlobalText[1389]);
	g_pRenderText->RenderText(m_iPos_x + 65, m_iPos_y - m_iHeight - 12, GlobalText[1307]);
	g_pRenderText->RenderText(m_iPos_x + 106, m_iPos_y - m_iHeight - 12, GlobalText[1022]);
}

int CUINewGuildMemberListBox::GetRenderLinePos_y(int iLineNumber)
{
	if (GetLineNum() > m_iNumRenderLine)
		return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1)* 13 - iLineNumber * 13 + 3);
	else
		return (m_iPos_y - m_iHeight + (GetLineNum() - 1) * 13 - iLineNumber * 13 + 3);
}

BOOL CUINewGuildMemberListBox::RenderDataLine(int iLineNumber)
{
	EnableAlphaTest();

	int iCharacterLevel;
	if (m_TextListIter->m_GuildStatus == 128)	// 길드마스터
		iCharacterLevel = 0;
	else if (m_TextListIter->m_GuildStatus == 64)	// 부길드마스터
		iCharacterLevel = 1;
	else if (m_TextListIter->m_GuildStatus == 32)	// 배틀마스터
		iCharacterLevel = 2;
	else	// 일반 길드원
		iCharacterLevel = 3;

	if (iCharacterLevel == 0)	// 길드마스터
	{
		glColor4ub(255, 100, 50, 127);
		RenderColor(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13);
		glColor4f(255, 255, 255, 255);
	}
	else if (iCharacterLevel == 1)	// 부길드마스터
	{
		glColor4ub(255, 150, 80, 127);
		RenderColor(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13);
		glColor4f(255, 255, 255, 255);
	}
	else if (iCharacterLevel == 2)	// 배틀마스터
	{
		glColor4ub(255, 200, 100, 127);
		RenderColor(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13);
		glColor4f(255, 255, 255, 255);
	}
	
	if (SLGetSelectLineNum() == m_iCurrentRenderEndLine + iLineNumber + 1)
	{
		glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
		RenderColor(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		g_pRenderText->SetTextColor(0, 0, 0, 255);
	}
	else
	{
		g_pRenderText->SetTextColor(230, 220, 200, 255);
	}
	glEnable(GL_TEXTURE_2D);
	g_pRenderText->SetBgColor(0);

	int iPos_x = m_iPos_x + 8;
	int iPos_y = GetRenderLinePos_y(iLineNumber);

	char Text[MAX_TEXT_LENGTH + 1] = {0};
	sprintf(Text,"%s",m_TextListIter->m_szID);
	g_pRenderText->RenderText(iPos_x, iPos_y, Text);

	if (iCharacterLevel == 0) g_pRenderText->RenderText(iPos_x + 45, iPos_y, GlobalText[1300], 70, 0, RT3_SORT_CENTER);
	else if (iCharacterLevel == 1) g_pRenderText->RenderText(iPos_x + 45, iPos_y, GlobalText[1301], 70, 0, RT3_SORT_CENTER);
	else if (iCharacterLevel == 2) g_pRenderText->RenderText(iPos_x + 45, iPos_y, GlobalText[1302], 70, 0, RT3_SORT_CENTER);
//	else g_pRenderText->RenderText(iPos_x + 45, iPos_y, "일반 길드원", 60, 0, RT3_SORT_CENTER);

	if (m_TextListIter->m_Server != 255/* && m_TextListIter->m_Number != 0*/)
	{	
		// 접속중일때는 서버 이름 표시
		g_pRenderText->SetBgColor(0);
		g_pRenderText->SetTextColor(255, 196, 0, 255);

		sprintf(Text,"%d", m_TextListIter->m_Server + 1);
		g_pRenderText->RenderText(m_iPos_x + m_iWidth - 30, iPos_y, Text);
	}

	DisableAlphaBlend();

	return TRUE;
}

BOOL CUINewGuildMemberListBox::DoLineMouseAction(int iLineNumber)
{
	if (::CheckMouseIn(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13))
	{
		if (MouseLButtonPush)
		{
			SLSetSelectLine(m_iCurrentRenderEndLine + iLineNumber + 1);
			m_TextListIter->m_bIsSelected = (m_TextListIter->m_bIsSelected + 1) % 2;	// T/F Reverse
			MouseLButtonPush = false;
		}
	}
	return TRUE;
}

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else //KWAK_FIX_COMPILE_LEVEL4_WARNING
void CUINewGuildMemberListBox::DeleteText(DWORD dwUIID)
{
//	BOOL bFind = FALSE;
//	for (m_TextListIter = m_TextList.begin(); m_TextListIter != m_TextList.end(); ++m_TextListIter)
//	{
//		if (m_TextListIter->m_dwUIID == dwUIID)
//		{
//			bFind = TRUE;
//			break;
//		}
//	}
//	if (bFind == FALSE) return;
//	
//	if (m_TextList.size() == 1)
//	{
//		m_TextList.erase(m_TextListIter);
//		SLSetSelectLine(0);
//		return;
//	}
//	if (SLGetSelectLineNum() != 1) SLSelectNextLine();
//	m_TextList.erase(m_TextListIter);
}
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING

CUIUnionGuildListBox::CUIUnionGuildListBox()
{
	m_iMaxLineCount = UIMAX_TEXT_LINE;
	m_iCurrentRenderEndLine = 0;
	m_iNumRenderLine = 6;	// 3의 배수로 -_-

	m_fScrollBarRange_top = 0;
	m_fScrollBarRange_bottom = 0;

	m_fScrollBarPos_y = 0;
	m_fScrollBarWidth = 13;
	m_fScrollBarHeight = 0;

	m_fScrollBarClickPos_y = 0;
	m_bScrollBtnClick = FALSE;
	m_bScrollBarClick = FALSE;

	m_bUseSelectLine = TRUE;


	SetPosition(466, 360);
	SetSize(157, 235);

}

void CUIUnionGuildListBox::AddText(BYTE* pGuildMark, const char* szGuildName, int nMemberCount)
{
	if (szGuildName == NULL || szGuildName[0] == '\0') return;

	static UNIONGUILD_TEXT text;
	text.m_bIsSelected = FALSE;
	memcpy( text.GuildMark, pGuildMark, sizeof(BYTE)*64 );
	strncpy(text.szName, szGuildName, MAX_GUILDNAME);
	text.szName[MAX_GUILDNAME] = NULL;
	text.nMemberCount = nMemberCount;
	m_TextList.push_front(text);

	RemoveText();
	SLSetSelectLine(0);
	if (GetLineNum() > m_iNumRenderLine) ++m_iCurrentRenderEndLine;

//	if (m_iCurrentRenderEndLine != 0) ++m_iCurrentRenderEndLine;

	if (GetLineNum() < m_iNumRenderLine);
	else if (GetLineNum() - m_iCurrentRenderEndLine < m_iNumRenderLine) 
		m_iCurrentRenderEndLine = GetLineNum() - m_iNumRenderLine;

	if (m_TextList.size() == 1)
		SLSetSelectLine(1);

	if (m_TextList.empty() == FALSE)
	{
		SLSetSelectLine(GetLineNum());
//		m_iCurrentRenderEndLine = 0;
		Scrolling(-10000);
	}
}

void CUIUnionGuildListBox::SetNumRenderLine(int iLine)
{
	if (iLine < m_iNumRenderLine && iLine < GetLineNum()) ++m_iCurrentRenderEndLine;
	else if (iLine > GetLineNum()) m_iCurrentRenderEndLine = 0;
	m_iNumRenderLine = iLine;
}

void CUIUnionGuildListBox::RenderInterface()
{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	return;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING

	// 배경칠
	EnableAlphaTest();
	SetLineColor(7, 0.4f);
	RenderColor(m_iPos_x - 1, m_iPos_y - m_iHeight - 1, m_iWidth + 1, m_iHeight + 2);

	EndRenderColor();

	// 스크롤 바
	ComputeScrollBar();

	//if (GetLineNum() >= m_iNumRenderLine)
	{
		//-_- 버튼
		if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - m_iHeight - 1, 13.0f, 13.0f) == TRUE)
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1,
				13.0f, 13.0f, 13.0f/16.0f, 29.0f/32.0f, -13.0f/16.0f, -13.0f/32.0f);
		else
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1,
				13.0f, 13.0f, 0.0f, 3.0f/32.0f, 13.0f/16.0f, 13.0f/32.0f);	// ▲
		
		if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - 12, 13.0f, 13.0f) == TRUE)
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12,
				13.0f, 13.0f, 13.0f/16.0f, 16.0f/32.0f, -13.0f/16.0f, -13.0f/32.0f);
		else
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12,
				13.0f, 13.0f, 0.0f, 16.0f/32.0f, 13.0f/16.0f, 13.0f/32.0f);	// ▼

		EnableAlphaTest();
		SetLineColor(2);
		RenderColor((float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 1, m_fScrollBarRange_top,(float)1,(float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
		RenderColor((float)m_iPos_x + m_iWidth, m_fScrollBarRange_top,(float)1,(float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
		EndRenderColor();

		if (GetLineNum() >= m_iNumRenderLine)
		{
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y,
				m_fScrollBarWidth - 2, m_fScrollBarHeight, 0.0f, 1.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y,
				m_fScrollBarWidth - 2, 1, 0.0f, 0.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y + m_fScrollBarHeight - 1,
				m_fScrollBarWidth - 2, 1, 0.0f, 2.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
		}
		else
		{
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top,
				m_fScrollBarWidth - 2, m_fScrollBarRange_bottom - m_fScrollBarRange_top, 0.0f, 1.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top,
				m_fScrollBarWidth - 2, 1, 0.0f, 0.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top + m_fScrollBarHeight - 1,
				m_fScrollBarWidth - 2, 1, 0.0f, 2.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
		}
	}

	// 길드명
	g_pRenderText->RenderText(m_iPos_x+15, m_iPos_y - m_iHeight - 12, GlobalText[182] );
	// 길드원수
	g_pRenderText->RenderText(m_iPos_x+113, m_iPos_y - m_iHeight - 12, GlobalText[1330] );

}

int CUIUnionGuildListBox::GetRenderLinePos_y(int iLineNumber)
{
	if (GetLineNum() > m_iNumRenderLine)
		return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1)* 13 - iLineNumber * 13 + 3);
	else
		return (m_iPos_y - m_iHeight + (GetLineNum() - 1) * 13 - iLineNumber * 13 + 3);
}
////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CUIUnionGuildListBox::RenderDataLine(int iLineNumber)
{
	EnableAlphaTest();

	if (SLGetSelectLineNum() == m_iCurrentRenderEndLine + iLineNumber + 1)
	{
		glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
		RenderColor(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		g_pRenderText->SetTextColor(0, 0, 0, 255);
	}
	else
	{
		g_pRenderText->SetTextColor(230, 220, 220, 255);
	}
	glEnable(GL_TEXTURE_2D);
	g_pRenderText->SetBgColor(0);

	int iPos_x = m_iPos_x + 4;
	int iPos_y = GetRenderLinePos_y(iLineNumber);

	// 길드마크
	memcpy( GuildMark[MARK_EDIT].Mark, m_TextListIter->GuildMark, sizeof(BYTE)*64 );
	CreateGuildMark(MARK_EDIT);
	RenderBitmap(BITMAP_GUILD,(float)iPos_x,(float)iPos_y,8,8);
	if( Hero->GuildMarkIndex >= 0 )
		memcpy( GuildMark[MARK_EDIT].Mark, GuildMark[Hero->GuildMarkIndex].Mark, sizeof(BYTE)*64 );
	else
		memset( GuildMark[MARK_EDIT].Mark, 0, 64 );

	char Text[MAX_TEXT_LENGTH + 1] = {0};
	// 길드명
	sprintf(Text,"%s", m_TextListIter->szName);
	g_pRenderText->RenderText(iPos_x + 12, iPos_y, Text);

	// 길드원수
	sprintf(Text,"%d", m_TextListIter->nMemberCount);
	g_pRenderText->RenderText(iPos_x + 138, iPos_y, Text, 0, 0, RT3_WRITE_RIGHT_TO_LEFT);

	DisableAlphaBlend();

	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CUIUnionGuildListBox::DoLineMouseAction(int iLineNumber)
{
	if (::CheckMouseIn(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13))
	{
		if (MouseLButtonPush)
		{
			SLSetSelectLine(m_iCurrentRenderEndLine + iLineNumber + 1);
			m_TextListIter->m_bIsSelected = (m_TextListIter->m_bIsSelected + 1) % 2;	// T/F Reverse
			MouseLButtonPush = false;
		}
	}
	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
void CUIUnionGuildListBox::DeleteText(DWORD dwGuildIndex)
{
}
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING

int CUIUnionGuildListBox::GetTextCount()
{
	return m_TextList.size();
}

//////////////////////////////////////////////////////////////////////////

#include "CComGem.h"

CUIUnmixgemList::CUIUnmixgemList()
{
	m_iMaxLineCount = UIMAX_TEXT_LINE;
	m_iCurrentRenderEndLine = 0;
	m_iNumRenderLine = MAX_LINE_UNMIXLIST;
	
	m_fScrollBarRange_top = 0;
	m_fScrollBarRange_bottom = 0;

	m_fScrollBarWidth = 13;
	m_fScrollBarHeight = 0;

	m_fScrollBarClickPos_y = 0;
	m_bScrollBtnClick = FALSE;
	m_bScrollBarClick = FALSE;

	m_bUseSelectLine = TRUE;

	SetPosition(0, 0);
	SetSize(180, 109);
}


void CUIUnmixgemList::SetNumRenderLine(int iLine)
{
	if (iLine < m_iNumRenderLine && iLine < GetLineNum()) ++m_iCurrentRenderEndLine;
	else if (iLine > GetLineNum()) m_iCurrentRenderEndLine = 0;
	m_iNumRenderLine = iLine;	
}

bool lessfunc(const UNMIX_TEXT& lhs, const UNMIX_TEXT& rhs)
{
#ifdef LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX
	return (lhs.m_iInvenIdx > rhs.m_iInvenIdx );
#else // LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX
	return (lhs.m_iInvenIdx < rhs.m_iInvenIdx );
#endif // LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX
}

void CUIUnmixgemList::Sort()
{
	sort(m_TextList.begin(), m_TextList.end(), lessfunc);
}

void CUIUnmixgemList::AddText(int iIndex, BYTE cComType )
{
	if(iIndex < 0 || iIndex > MAX_INVENTORY || cComType == COMGEM::NOCOM) return;
	
	for(unsigned int i = 0 ; i < m_TextList.size(); ++i)
	{
		const UNMIX_TEXT& rt = m_TextList[i];
		if(rt.m_iInvenIdx == iIndex) return;
	}

	UNMIX_TEXT	t;
	t.m_bIsSelected = FALSE;
	t.m_cLevel = cComType;
	t.m_iInvenIdx = iIndex;
	m_TextList.push_back(t);

	RemoveText();
	SLSetSelectLine(0);
	
	if (GetLineNum() > m_iNumRenderLine) ++m_iCurrentRenderEndLine;

	if (GetLineNum() < m_iNumRenderLine);
	else if (GetLineNum() - m_iCurrentRenderEndLine < m_iNumRenderLine) 
		m_iCurrentRenderEndLine = GetLineNum() - m_iNumRenderLine;

	Sort();
}

void CUIUnmixgemList::RenderInterface()
{
	EnableAlphaTest();
	SetLineColor(7, 0.4f);
	RenderColor(m_iPos_x - 1, m_iPos_y - m_iHeight - 1, m_iWidth + 1, m_iHeight + 2);

	EndRenderColor();

	// 스크롤 바
	ComputeScrollBar();

	//if (GetLineNum() >= m_iNumRenderLine)
	{
		//-_- 버튼
		if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - m_iHeight - 1, 13.0f, 13.0f) == TRUE)
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1,
				13.0f, 13.0f, 13.0f/16.0f, 29.0f/32.0f, -13.0f/16.0f, -13.0f/32.0f);
		else
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1,
				13.0f, 13.0f, 0.0f, 3.0f/32.0f, 13.0f/16.0f, 13.0f/32.0f);	// ▲
		
		if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - 12, 13.0f, 13.0f) == TRUE)
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12,
				13.0f, 13.0f, 13.0f/16.0f, 16.0f/32.0f, -13.0f/16.0f, -13.0f/32.0f);
		else
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12,
				13.0f, 13.0f, 0.0f, 16.0f/32.0f, 13.0f/16.0f, 13.0f/32.0f);	// ▼

		EnableAlphaTest();
		SetLineColor(2);
		RenderColor((float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 1, m_fScrollBarRange_top,(float)1,(float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
		RenderColor((float)m_iPos_x + m_iWidth, m_fScrollBarRange_top,(float)1,(float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
		EndRenderColor();

		if (GetLineNum() >= m_iNumRenderLine)
		{
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y,
				m_fScrollBarWidth - 2, m_fScrollBarHeight, 0.0f, 1.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y,
				m_fScrollBarWidth - 2, 1, 0.0f, 0.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y + m_fScrollBarHeight - 1,
				m_fScrollBarWidth - 2, 1, 0.0f, 2.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
		}
		else
		{
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top,
				m_fScrollBarWidth - 2, m_fScrollBarRange_bottom - m_fScrollBarRange_top, 0.0f, 1.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top,
				m_fScrollBarWidth - 2, 1, 0.0f, 0.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
			RenderBitmap(BITMAP_INTERFACE_EX+12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top + m_fScrollBarHeight - 1,
				m_fScrollBarWidth - 2, 1, 0.0f, 2.0f/32.0f, 11.0f/16.0f, 1.0f/32.0f);
		}
	}
}

BOOL CUIUnmixgemList::RenderDataLine(int iLineNumber)
{
	EnableAlphaTest();

	if (SLGetSelectLineNum() == m_iCurrentRenderEndLine + iLineNumber + 1)
	{
		glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
		RenderColor(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		g_pRenderText->SetTextColor(0, 0, 0, 255);
	}
	else
	{
		g_pRenderText->SetTextColor(230, 220, 200, 255);
	}
	glEnable(GL_TEXTURE_2D);
	g_pRenderText->SetBgColor(0);
	int iPos_x = m_iPos_x + 4;
	int iPos_y = GetRenderLinePos_y(iLineNumber);
	
	//텍스트 출력
	char oText[MAX_GLOBAL_TEXT_STRING] = {0,};

#ifdef LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX // 보석조합 해체 리스트 출력
	ITEM* pItem = g_pMyInventory->GetInventoryCtrl()->FindItem(m_TextListIter->m_iInvenIdx);
	int	  nIdx	= COMGEM::Check_Jewel( pItem->Type );
	sprintf(oText, "%s,  %d", GlobalText[COMGEM::GetJewelIndex(nIdx, COMGEM::eGEM_NAME)], (m_TextListIter->m_cLevel + 1) * 10);
#else
	if(COMGEM::m_cGemType == COMGEM::CELE) sprintf(oText, "%s,  %d", GlobalText[1806], (m_TextListIter->m_cLevel + 1) * 10);
	else sprintf(oText, "%s,  %d", GlobalText[1807], (m_TextListIter->m_cLevel + 1) * 10);
#endif // LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX
	
	
	g_pRenderText->RenderText(iPos_x + 2, iPos_y, oText);

	DisableAlphaBlend();
	return TRUE;
}

BOOL CUIUnmixgemList::DoLineMouseAction(int iLineNumber)
{
	if (::CheckMouseIn(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13))
	{
		if (MouseLButtonPush)
		{
			SLSetSelectLine(m_iCurrentRenderEndLine + iLineNumber + 1);
			m_TextListIter->m_bIsSelected = (m_TextListIter->m_bIsSelected + 1) % 2;	// T/F Reverse
			MouseLButtonPush = false;
		}

		//더블클릭했을 경우 
		if (MouseLButtonDBClick)
		{
			SLSetSelectLine(m_iCurrentRenderEndLine + iLineNumber + 1);
			UNMIX_TEXT* pt = GetSelectedText();

			if(pt->m_cLevel != COMGEM::NOCOM && pt->m_iInvenIdx > 0 && pt->m_iInvenIdx < MAX_INVENTORY)
				COMGEM::SelectFromList(pt->m_iInvenIdx, pt->m_cLevel);

			MouseLButtonDBClick = false;
		}
	}
	return TRUE;
}

int CUIUnmixgemList::GetRenderLinePos_y(int iLineNumber)
{
	if (GetLineNum() > m_iNumRenderLine)
		return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1)* 13 - iLineNumber * 13 + 3);
	else
		return (m_iPos_y - m_iHeight + (GetLineNum() - 1) * 13 - iLineNumber * 13 + 3);
}

//////////////////////////////////////////////////////////////////////////

CUIBCDeclareGuildListBox::CUIBCDeclareGuildListBox()
{
	m_iMaxLineCount = UIMAX_TEXT_LINE;
	m_iCurrentRenderEndLine = 0;
	m_iNumRenderLine = 18;	// 3의 배수로 -_-

	m_fScrollBarRange_top = 0;
	m_fScrollBarRange_bottom = 0;

	m_fScrollBarPos_y = 0;
	m_fScrollBarWidth = 13;
	m_fScrollBarHeight = 0;

	m_fScrollBarClickPos_y = 0;
	m_bScrollBtnClick = FALSE;
	m_bScrollBarClick = FALSE;

	m_bUseSelectLine = TRUE;

	m_bUseNewUIScrollBar = TRUE;

	SetPosition(465, 364);
	SetSize(160, 235);
}

void CUIBCDeclareGuildListBox::AddText( const char* szGuildName, int nMarkCount, BYTE byIsGiveUp, BYTE bySeqNum )
{
	if (szGuildName == NULL || szGuildName[0] == '\0') return;

	static BCDECLAREGUILD_TEXT text;
	text.m_bIsSelected = FALSE;
	strncpy(text.szName, szGuildName, MAX_GUILDNAME);
	text.szName[MAX_GUILDNAME] = NULL;
	text.nCount = nMarkCount;
	text.byIsGiveUp = byIsGiveUp;
	text.bySeqNum = bySeqNum;
	m_TextList.push_front(text);

	RemoveText();
	SLSetSelectLine(0);
	if (GetLineNum() > m_iNumRenderLine) ++m_iCurrentRenderEndLine;

//	if (m_iCurrentRenderEndLine != 0) ++m_iCurrentRenderEndLine;

	if (GetLineNum() < m_iNumRenderLine);
	else if (GetLineNum() - m_iCurrentRenderEndLine < m_iNumRenderLine) 
		m_iCurrentRenderEndLine = GetLineNum() - m_iNumRenderLine;

	if (m_TextList.size() == 1)
		SLSetSelectLine(1);

	if (m_TextList.empty() == FALSE)
	{
		SLSetSelectLine(GetLineNum());
//		m_iCurrentRenderEndLine = 0;
		Scrolling(-10000);
	}
}

bool BCDeclareGuildSortByMark( const BCDECLAREGUILD_TEXT& lhs, const BCDECLAREGUILD_TEXT& rhs )
{
	return (lhs.nCount < rhs.nCount);
}

void CUIBCDeclareGuildListBox::Sort()
{
	sort( m_TextList.begin(), m_TextList.end(), BCDeclareGuildSortByMark );
}

void CUIBCDeclareGuildListBox::SetNumRenderLine(int iLine)
{
	if (iLine < m_iNumRenderLine && iLine < GetLineNum()) ++m_iCurrentRenderEndLine;
	else if (iLine > GetLineNum()) m_iCurrentRenderEndLine = 0;
	m_iNumRenderLine = iLine;
}

void CUIBCDeclareGuildListBox::RenderInterface()
{
	EnableAlphaTest();
	SetLineColor(7, 0.4f);
	RenderColor(m_iPos_x - 1, m_iPos_y - m_iHeight - 1, m_iWidth + 1, m_iHeight + 2);
	EndRenderColor();

	if (GetState() != UISTATE_SCROLL)
		ComputeScrollBar();

	g_pGuardWindow->RenderScrollBarFrame(m_iPos_x + m_iWidth - 8, m_fScrollBarRange_top, m_fScrollBarRange_bottom - m_fScrollBarRange_top);
	g_pGuardWindow->RenderScrollBar(m_iPos_x + m_iWidth - 12, m_fScrollBarPos_y, (GetState() == UISTATE_SCROLL && MouseLButtonPush));
	g_pRenderText->RenderText(m_iPos_x+5, m_iPos_y - m_iHeight - 12, GlobalText[182] );
	g_pRenderText->RenderText(m_iPos_x+50, m_iPos_y - m_iHeight - 12, GlobalText[1528] );
	g_pRenderText->RenderText(m_iPos_x+98, m_iPos_y - m_iHeight - 12, GlobalText[1529] );
	g_pRenderText->RenderText(m_iPos_x+123, m_iPos_y - m_iHeight - 12, GlobalText[1530] );
}

int CUIBCDeclareGuildListBox::GetRenderLinePos_y(int iLineNumber)
{
	if (GetLineNum() > m_iNumRenderLine)
		return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1)* 13 - iLineNumber * 13 + 3);
	else
		return (m_iPos_y - m_iHeight + (GetLineNum() - 1) * 13 - iLineNumber * 13 + 3);
}

BOOL CUIBCDeclareGuildListBox::RenderDataLine(int iLineNumber)
{
	char Text[MAX_TEXT_LENGTH + 1] = {0};

	sprintf(Text,"%s", m_TextListIter->szName);

	if ( (strcmp( GuildMark[Hero->GuildMarkIndex].UnionName, Text )!=NULL && strcmp( GuildMark[Hero->GuildMarkIndex].GuildName, Text )!=NULL) )
	{
		return FALSE;
	}

	EnableAlphaTest();

	if (SLGetSelectLineNum() == m_iCurrentRenderEndLine + iLineNumber + 1)
	{
		glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
		RenderColor(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		g_pRenderText->SetTextColor(0, 0, 0, 255);
	}
	else
	{
		g_pRenderText->SetTextColor(230, 220, 200, 255);
	}
	glEnable(GL_TEXTURE_2D);
	g_pRenderText->SetBgColor(0);
	
	int iPos_x = m_iPos_x + 4;
	int iPos_y = GetRenderLinePos_y(iLineNumber);

	g_pRenderText->RenderText(iPos_x + 2, iPos_y, Text);

	sprintf(Text,"%d", m_TextListIter->nCount);
	g_pRenderText->RenderText(iPos_x + 70, iPos_y, Text, 0, 0, RT3_WRITE_RIGHT_TO_LEFT);

	if( m_TextListIter->byIsGiveUp )
		sprintf(Text,"%s", GlobalText[1531] );
	else
		sprintf(Text,"%s", GlobalText[1532] );
	g_pRenderText->RenderText(iPos_x + 120, iPos_y, Text, 0, 0, RT3_WRITE_RIGHT_TO_LEFT);

	sprintf(Text,"%u", m_TextListIter->bySeqNum);
	g_pRenderText->RenderText(iPos_x + 140, iPos_y, Text, 0, 0, RT3_WRITE_RIGHT_TO_LEFT);

	DisableAlphaBlend();

	return TRUE;
}

BOOL CUIBCDeclareGuildListBox::DoLineMouseAction(int iLineNumber)
{
	if (::CheckMouseIn(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13))
	{
		if (MouseLButtonPush)
		{
			SLSetSelectLine(m_iCurrentRenderEndLine + iLineNumber + 1);
			m_TextListIter->m_bIsSelected = (m_TextListIter->m_bIsSelected + 1) % 2;	// T/F Reverse
			MouseLButtonPush = false;
		}
	}
	return TRUE;
}

void CUIBCDeclareGuildListBox::DeleteText(DWORD dwGuildIndex)
{
}

CUIBCGuildListBox::CUIBCGuildListBox()
{
	m_iMaxLineCount = UIMAX_TEXT_LINE;
	m_iCurrentRenderEndLine = 0;
	m_iNumRenderLine = 15;	// 3의 배수로 -_-

	m_fScrollBarRange_top = 0;
	m_fScrollBarRange_bottom = 0;

	m_fScrollBarPos_y = 0;
	m_fScrollBarWidth = 13;
	m_fScrollBarHeight = 0;

	m_fScrollBarClickPos_y = 0;
	m_bScrollBtnClick = FALSE;
	m_bScrollBarClick = FALSE;

	m_bUseSelectLine = TRUE;

	Select_Guild = -1;
	m_bUseNewUIScrollBar = TRUE;

	SetPosition(465, 324);
	SetSize(160, 195);
}

void CUIBCGuildListBox::AddText( const char* szGuildName, BYTE byJoinSide, BYTE byGuildInvolved 
								,int iGuildScore
								)
{
	if (szGuildName == NULL || szGuildName[0] == '\0') return;

	static BCGUILD_TEXT text;
	text.m_bIsSelected = FALSE;
	strncpy(text.szName, szGuildName, MAX_GUILDNAME);
	text.szName[MAX_GUILDNAME] = NULL;
	text.byJoinSide = byJoinSide;
	text.byGuildInvolved = byGuildInvolved;
	text.iGuildScore = iGuildScore;
	m_TextList.push_front(text);

	RemoveText();
	SLSetSelectLine(0);
	if (GetLineNum() > m_iNumRenderLine) ++m_iCurrentRenderEndLine;

//	if (m_iCurrentRenderEndLine != 0) ++m_iCurrentRenderEndLine;

	if (GetLineNum() < m_iNumRenderLine);
	else if (GetLineNum() - m_iCurrentRenderEndLine < m_iNumRenderLine) 
		m_iCurrentRenderEndLine = GetLineNum() - m_iNumRenderLine;

	if (m_TextList.size() == 1)
		SLSetSelectLine(1);

	if (m_TextList.empty() == FALSE)
	{
		SLSetSelectLine(GetLineNum());
//		m_iCurrentRenderEndLine = 0;
		Scrolling(-10000);
	}
}

void CUIBCGuildListBox::SetNumRenderLine(int iLine)
{
	if (iLine < m_iNumRenderLine && iLine < GetLineNum()) ++m_iCurrentRenderEndLine;
	else if (iLine > GetLineNum()) m_iCurrentRenderEndLine = 0;
	m_iNumRenderLine = iLine;
}

void CUIBCGuildListBox::RenderInterface()
{
	// 배경칠
	EnableAlphaTest();
	SetLineColor(7, 0.4f);
	RenderColor(m_iPos_x - 1, m_iPos_y - m_iHeight - 1, m_iWidth + 1, m_iHeight + 2);

	//남겨놔...
	SetLineColor(0, 0.4f);
	RenderColor(m_iPos_x - 1, m_iPos_y + 25 - 1, m_iWidth  - 100 + 1, 20);

	SetLineColor(7, 0.4f);
	RenderColor(m_iPos_x - 1 + m_iWidth - 100 + 1, m_iPos_y + 25 - 1, m_iWidth - 60, 20);
	EndRenderColor();

	// 스크롤 바
	if (GetState() != UISTATE_SCROLL)
		ComputeScrollBar();

	g_pGuardWindow->RenderScrollBarFrame(m_iPos_x + m_iWidth - 8, m_fScrollBarRange_top, m_fScrollBarRange_bottom - m_fScrollBarRange_top);
	g_pGuardWindow->RenderScrollBar(m_iPos_x + m_iWidth - 12, m_fScrollBarPos_y, (GetState() == UISTATE_SCROLL && MouseLButtonPush));

	// 길드명
	g_pRenderText->RenderText(m_iPos_x+5, m_iPos_y - m_iHeight - 12, GlobalText[182] );
	// 진영
	g_pRenderText->RenderText(m_iPos_x+80, m_iPos_y - m_iHeight - 12, GlobalText[1603] );	// 1603 "진영"
	// 주체
	g_pRenderText->RenderText(m_iPos_x+120, m_iPos_y - m_iHeight - 12, GlobalText[1604] );	// 1604 "주체"

	g_pRenderText->RenderText(m_iPos_x + 18, m_iPos_y + 31 - 1, GlobalText[1977] );	// 1977 "점수"
}

int CUIBCGuildListBox::GetRenderLinePos_y(int iLineNumber)
{
	if (GetLineNum() > m_iNumRenderLine)
		return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1)* 13 - iLineNumber * 13 + 3);
	else
		return (m_iPos_y - m_iHeight + (GetLineNum() - 1) * 13 - iLineNumber * 13 + 3);
}

BOOL CUIBCGuildListBox::RenderDataLine(int iLineNumber)
{
	EnableAlphaTest();

	if (SLGetSelectLineNum() == m_iCurrentRenderEndLine + iLineNumber + 1)
	{
		glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
		RenderColor(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		g_pRenderText->SetTextColor(0, 0, 0, 255);
	}
	else
	if( m_TextListIter->byJoinSide == 1 )
	{
		glColor4f(1.0f, 1.0f, 1.0f, 0.2f);
		RenderColor(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		g_pRenderText->SetTextColor(230, 220, 200, 255);
	}
	else
	{
		g_pRenderText->SetTextColor(230, 220, 200, 255);
	}
	glEnable(GL_TEXTURE_2D);
	g_pRenderText->SetBgColor(0);
	
	int iPos_x = m_iPos_x + 4;
	int iPos_y = GetRenderLinePos_y(iLineNumber);

	char Text[MAX_TEXT_LENGTH + 1] = {0};
	// 길드명
	sprintf(Text,"%s", m_TextListIter->szName);
	g_pRenderText->RenderText(iPos_x + 2, iPos_y, Text);

	// 진영
	if( m_TextListIter->byJoinSide == 1 )
		sprintf(Text,"%s", GlobalText[1606] );	// 1606 "수성측"
	else
		sprintf(Text,"%s", GlobalText[1605] );	// 1605 "공성측"
	g_pRenderText->RenderText(iPos_x + 100, iPos_y, Text, 0, 0, RT3_WRITE_RIGHT_TO_LEFT);

	// 주체
	if( m_TextListIter->byGuildInvolved == 1 )
		sprintf(Text,"%s", GlobalText[1607] );	// 1607 "주체"
	else
		sprintf(Text,"%s", GlobalText[1608] );	// 1608 "보조"

	g_pRenderText->RenderText(iPos_x + 137, iPos_y, Text, 0, 0, RT3_WRITE_RIGHT_TO_LEFT);

	g_pRenderText->SetTextColor(230, 220, 200, 255);
	char Dummy[300];	if (Select_Guild == iLineNumber)
	{
		if( m_TextListIter->byJoinSide == 1 )
			wsprintf(Dummy,"--");
		else
			wsprintf(Dummy,"%s :     %d",m_TextListIter->szName,m_TextListIter->iGuildScore);
		g_pRenderText->RenderText(m_iPos_x + 60, m_iPos_y + 31 - 1, Dummy );	// 1977 "점수"
	}

	DisableAlphaBlend();
	

	return TRUE;
}

BOOL CUIBCGuildListBox::DoLineMouseAction(int iLineNumber)
{
	if (::CheckMouseIn(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13))
	{
		if (MouseLButtonPush)
		{

			Select_Guild = iLineNumber;

			SLSetSelectLine(m_iCurrentRenderEndLine + iLineNumber + 1);
			m_TextListIter->m_bIsSelected = (m_TextListIter->m_bIsSelected + 1) % 2;	// T/F Reverse
			MouseLButtonPush = false;
		}
	}
	return TRUE;
}

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
void CUIBCGuildListBox::DeleteText(DWORD dwGuildIndex)
{

}
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING

void RenderGoldRect(float fPos_x, float fPos_y, float fWidth, float fHeight, int iFillType = 0)
{
	switch (iFillType)
	{
	case 1:
		glColor4ub(146, 144, 141, 200);
		RenderColor(fPos_x, fPos_y, fWidth, fHeight);
		EndRenderColor();
		break;
	default:
		break;
	};

	RenderBitmap(BITMAP_INVENTORY+19, fPos_x, fPos_y, fWidth, 2,
		10/256.f, 5/16.f, 170.f/256.f, 2.f/16.f);
	RenderBitmap(BITMAP_INVENTORY+19, fPos_x, fPos_y + fHeight, fWidth + 1, 2,
		10/256.f, 5/16.f, 170.f/256.f, 2.f/16.f);
    RenderBitmap(BITMAP_INVENTORY, fPos_x, fPos_y, 2, fHeight,
		1.f/256.f, 5/16.f, 2.f/256.f, 125.f/256.f);
    RenderBitmap(BITMAP_INVENTORY, fPos_x + fWidth, fPos_y, 2, fHeight,
		1.f/256.f, 5/16.f, 2.f/256.f, 125.f/256.f);
}
////////////////////////////////////////////////////////////////////////////////////////////////////

CUIMoveCommandListBox::CUIMoveCommandListBox()
{
	m_iMaxLineCount = UIMAX_TEXT_LINE;
	m_iCurrentRenderEndLine = 0;

	m_iNumRenderLine = 21;	// 3의 배수로 -_-

	m_fScrollBarRange_top = 0;
	m_fScrollBarRange_bottom = 0;

	m_fScrollBarPos_y = 0;
	m_fScrollBarWidth = 13;
	m_fScrollBarHeight = 0;

	m_fScrollBarClickPos_y = 0;
	m_bScrollBtnClick = FALSE;
	m_bScrollBarClick = FALSE;

	m_bUseSelectLine = TRUE;

	m_bUseNewUIScrollBar = TRUE;

	SetPosition(465, 324);
	SetSize(160, 195);
}

void CUIMoveCommandListBox::AddText( int iIndex, const char* szMapName, const char* szSubMapName, int iReqLevel, int iReqZen, int iGateNum )
{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	if (szMapName == NULL || szMapName[0] == '\0' || iIndex < 0)
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	if (szMapName == NULL || szMapName[0] == '\0') 
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
		return;

	static MOVECOMMAND_TEXT text;
	text.m_bIsSelected = FALSE;
	text.m_bCanMove = FALSE;
	strncpy(text.szMainMapName, szMapName, 32);
	strncpy(text.szSubMapName, szSubMapName, 32);
	text.iReqLevel = iReqLevel;
	text.iReqZen = iReqZen;
	text.iGateNum = iGateNum;
	
	m_TextList.push_front(text);

	RemoveText();
	SLSetSelectLine(0);
	if (GetLineNum() > m_iNumRenderLine) ++m_iCurrentRenderEndLine;

//	if (m_iCurrentRenderEndLine != 0) ++m_iCurrentRenderEndLine;

	if (GetLineNum() < m_iNumRenderLine);
	else if (GetLineNum() - m_iCurrentRenderEndLine < m_iNumRenderLine) 
		m_iCurrentRenderEndLine = GetLineNum() - m_iNumRenderLine;

// 	if (m_TextList.size() == 1)
// 		SLSetSelectLine(1);
// 
// 	if (m_TextList.empty() == FALSE)
// 	{
// 		SLSetSelectLine(GetLineNum());
// //		m_iCurrentRenderEndLine = 0;
// 		Scrolling(-10000);
// 	}
}

void CUIMoveCommandListBox::SetNumRenderLine(int iLine)
{
	if (iLine < m_iNumRenderLine && iLine < GetLineNum()) 
		++m_iCurrentRenderEndLine;
	else if (iLine > GetLineNum()) 
		m_iCurrentRenderEndLine = 0;

	m_iNumRenderLine = iLine;
}

void CUIMoveCommandListBox::RenderInterface()
{

}

int CUIMoveCommandListBox::GetRenderLinePos_y(int iLineNumber)
{
	if (GetLineNum() > m_iNumRenderLine)
		return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1)* 13 - iLineNumber * 13 + 3);
	else
		return (m_iPos_y - m_iHeight + (GetLineNum() - 1) * 13 - iLineNumber * 13 + 3);
}

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
BOOL CUIMoveCommandListBox::RenderDataLine(int iLineNumber)
{
	return TRUE;
}

BOOL CUIMoveCommandListBox::DoLineMouseAction(int iLineNumber)
{
	return TRUE;
}
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef ASG_MOD_UI_QUEST_INFO

CUICurQuestListBox::CUICurQuestListBox()
{
	m_iMaxLineCount = UIMAX_TEXT_LINE;
	m_iCurrentRenderEndLine = 0;
	m_iNumRenderLine = 8;

	m_fScrollBarRange_top = 0;
	m_fScrollBarRange_bottom = 0;

	m_fScrollBarPos_y = 0;
	m_fScrollBarWidth = 13;
	m_fScrollBarHeight = 0;

	m_fScrollBarClickPos_y = 0;
	m_bScrollBtnClick = FALSE;
	m_bScrollBarClick = FALSE;

	m_bUseSelectLine = TRUE;
	m_bUseNewUIScrollBar = TRUE;
	m_iScrollType = UILISTBOX_SCROLL_DOWNUP;

	SetSize(174, 105);
}

void CUICurQuestListBox::AddText(DWORD dwQuestIndex, const char* pszText)
{
	static SCurQuestItem sCurQuestItem;
	sCurQuestItem.m_bIsSelected = FALSE;
	sCurQuestItem.m_dwIndex = dwQuestIndex;
	strncpy(sCurQuestItem.m_szText, pszText, 64);

	m_TextList.push_front(sCurQuestItem);	// 뒤부터 랜더하기 때문에 push_front 사용.

	SendUIMessageDirect(UI_MESSAGE_LISTSCRLTOP, 0, 0);	// 스크롤바 썸 위치를 맨 위로.
	SLSetSelectLine(0);									// 선택 없음.
}

void CUICurQuestListBox::SetNumRenderLine(int iLine)
{
	if (iLine < m_iNumRenderLine && iLine < GetLineNum())
		++m_iCurrentRenderEndLine;
	else if (iLine > GetLineNum())
		m_iCurrentRenderEndLine = 0;

	m_iNumRenderLine = iLine;
}

void CUICurQuestListBox::RenderInterface()
{
	if (GetState() != UISTATE_SCROLL)
		ComputeScrollBar();

	// 렌더되는 줄 수 보다 리스트 줄 수가 작거나 같으면 스크롤바를 안그림.
	if (GetLineNum() <= m_iNumRenderLine)
		return;

	g_pGuardWindow->RenderScrollBarFrame(m_iPos_x + m_iWidth - 8, m_fScrollBarRange_top,
		m_fScrollBarRange_bottom - m_fScrollBarRange_top);
	g_pGuardWindow->RenderScrollBar(m_iPos_x + m_iWidth - 12, m_fScrollBarPos_y,
		(GetState() == UISTATE_SCROLL && MouseLButtonPush));
}

int CUICurQuestListBox::GetRenderLinePos_y(int iLineNumber)
{
	if (GetLineNum() > m_iNumRenderLine)
		return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1)* 13 - iLineNumber * 13 + 3);
	else
		return (m_iPos_y - m_iHeight + (GetLineNum() - 1) * 13 - iLineNumber * 13 + 3);
}

BOOL CUICurQuestListBox::RenderDataLine(int iLineNumber)
{
	EnableAlphaTest();
	
	if (SLGetSelectLineNum() == m_iCurrentRenderEndLine + iLineNumber + 1)
	{
		::glColor4f(0.5f, 0.7f, 0.3f, 0.5f);
		RenderColor(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glEnable(GL_TEXTURE_2D);
	}

	g_pRenderText->SetTextColor(255, 230, 210, 255);
	g_pRenderText->SetBgColor(0);
	
	int iPos_x = m_iPos_x + 5;
	int iPos_y = GetRenderLinePos_y(iLineNumber);
	
	char Text[MAX_TEXT_LENGTH + 1] = {0};
	sprintf(Text,"%s",m_TextListIter->m_szText);
	g_pRenderText->RenderText(iPos_x, iPos_y, Text);
	
	return TRUE;
}

BOOL CUICurQuestListBox::DoLineMouseAction(int iLineNumber)
{
	if (::CheckMouseIn(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13))
	{
		if (MouseLButtonPush)	// MouseLButtonDBClick
		{
			MouseLButtonPush = false;

			int nSelLine = m_iCurrentRenderEndLine + iLineNumber + 1;

			// 선택한 줄이 이전에 선택한 줄이라면 리턴.
			if (SLGetSelectLineNum() == nSelLine)
				return TRUE;

			::PlayBuffer(SOUND_CLICK01);

			SLSetSelectLine(nSelLine);
			m_TextListIter->m_bIsSelected = (m_TextListIter->m_bIsSelected + 1) % 2;	// T/F Reverse

			// 기타상황에 의한 퀘스트라면 퀘스트 열기 버튼 활성화, 아니라면 비활성화.
			m_TextListIter = SLGetSelectLine();
			if (g_QuestMng.IsQuestByEtc(m_TextListIter->m_dwIndex))
				g_pMyQuestInfoWindow->QuestOpenBtnEnable(true);
			else
				g_pMyQuestInfoWindow->QuestOpenBtnEnable(false);

			// 퀘스트 포기 버튼 활성화.
			g_pMyQuestInfoWindow->QuestGiveUpBtnEnable(true);

			// 퀘스트 요약를 세팅.
			g_pMyQuestInfoWindow->SetSelQuestSummary();

			// 서버에 퀘스트 요구사항, 보상 정보를 요구.
			SendRequestProgressQuestRequestReward(m_TextListIter->m_dwIndex);
		}
	}

	return TRUE;
}

void CUICurQuestListBox::DeleteText(DWORD dwQuestIndex)
{
	BOOL bFind = FALSE;
	for (m_TextListIter = m_TextList.begin(); m_TextListIter != m_TextList.end(); ++m_TextListIter)
	{
		if (m_TextListIter->m_dwIndex == dwQuestIndex)
		{
			bFind = TRUE;
			break;
		}
	}

	if (bFind == FALSE)
		return;
	
	if (m_TextList.size() == 1)
	{
		m_TextList.erase(m_TextListIter);
		SLSetSelectLine(0);
		return;
	}

	if (SLGetSelectLineNum() != 1)
		SLSelectNextLine();

	m_TextList.erase(m_TextListIter);
}
#endif	// ASG_MOD_UI_QUEST_INFO

#ifdef ASG_ADD_NEW_QUEST_SYSTEM
CUIQuestContentsListBox::CUIQuestContentsListBox()
{
	m_iMaxLineCount = UIMAX_TEXT_LINE;
	m_iCurrentRenderEndLine = 0;
	m_iNumRenderLine = 16;

	m_fScrollBarRange_top = 0;
	m_fScrollBarRange_bottom = 0;

	m_fScrollBarPos_y = 0;
	m_fScrollBarWidth = 13;
	m_fScrollBarHeight = 0;

	m_fScrollBarClickPos_y = 0;
	m_bScrollBtnClick = FALSE;
	m_bScrollBarClick = FALSE;

	m_bUseSelectLine = FALSE;
	m_bUseNewUIScrollBar = TRUE;
	m_iScrollType = UILISTBOX_SCROLL_DOWNUP;
	
	SetSize(174, 208);
}

void CUIQuestContentsListBox::AddText(HFONT hFont, DWORD dwColor, int nSort, const char* pszText)
{
	static SQuestContents sQuestContents;

	sQuestContents.m_hFont = hFont;
	sQuestContents.m_dwColor = dwColor;
	sQuestContents.m_nSort = nSort;
	strncpy(sQuestContents.m_szText, pszText, 64);
	sQuestContents.m_dwType = 0;
	sQuestContents.m_wIndex = 0;
	sQuestContents.m_pItem = NULL;

	m_TextList.push_front(sQuestContents);	// 뒤부터 랜더하기 때문에 push_front 사용.

	SendUIMessageDirect(UI_MESSAGE_LISTSCRLTOP, 0, 0);	// 스크롤바 썸 위치를 맨 위로.
}

void CUIQuestContentsListBox::AddText(SRequestRewardText* pRequestRewardText, int nSort)
{
	static SQuestContents sQuestContents;
	
	sQuestContents.m_hFont = pRequestRewardText->m_hFont;
	sQuestContents.m_dwColor = pRequestRewardText->m_dwColor;
	sQuestContents.m_nSort = nSort;
	strncpy(sQuestContents.m_szText, pRequestRewardText->m_szText, 64);
	sQuestContents.m_eRequestReward = pRequestRewardText->m_eRequestReward;
	sQuestContents.m_dwType = pRequestRewardText->m_dwType;
	sQuestContents.m_wIndex = pRequestRewardText->m_wIndex;
	sQuestContents.m_pItem = pRequestRewardText->m_pItem;
	
	m_TextList.push_front(sQuestContents);	// 뒤부터 랜더하기 때문에 push_front 사용.
	
	SendUIMessageDirect(UI_MESSAGE_LISTSCRLTOP, 0, 0);	// 스크롤바 썸 위치를 맨 위로.
}

void CUIQuestContentsListBox::RenderInterface()
{
	if (GetState() != UISTATE_SCROLL)
		ComputeScrollBar();

	// 렌더되는 줄 수 보다 리스트 줄 수가 크면 스크롤바를 그림.
	if (GetLineNum() > m_iNumRenderLine)
	{
		g_pGuardWindow->RenderScrollBarFrame(m_iPos_x + m_iWidth - 8, m_fScrollBarRange_top,
			m_fScrollBarRange_bottom - m_fScrollBarRange_top);
		g_pGuardWindow->RenderScrollBar(m_iPos_x + m_iWidth - 12, m_fScrollBarPos_y,
			(GetState() == UISTATE_SCROLL && MouseLButtonPush));
	}
}

int CUIQuestContentsListBox::GetRenderLinePos_y(int iLineNumber)
{
	if (GetLineNum() > m_iNumRenderLine)
		return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1)* 13 - iLineNumber * 13 + 3);
	else
		return (m_iPos_y - m_iHeight + (GetLineNum() - 1) * 13 - iLineNumber * 13 + 3);
}

BOOL CUIQuestContentsListBox::RenderDataLine(int iLineNumber)
{
	EnableAlphaTest();

	g_pRenderText->SetFont(m_TextListIter->m_hFont);
	g_pRenderText->SetTextColor(m_TextListIter->m_dwColor);
	g_pRenderText->SetBgColor(0);

	int iPos_x = m_iPos_x + 5;
	int iPos_y = GetRenderLinePos_y(iLineNumber);
	
	char Text[MAX_TEXT_LENGTH + 1] = {0};
	sprintf(Text,"%s",m_TextListIter->m_szText);
	g_pRenderText->RenderText(iPos_x, iPos_y, Text, GetWidth() - 18, 0, m_TextListIter->m_nSort);
	
	return TRUE;
}

// 리스트 박스 랜더 중 가장 나중에 호출.
void CUIQuestContentsListBox::RenderCoveredInterface()
{
// 선택한 항목이 있으면 아이템 또는 스킬 툴팁 랜더.
	m_TextListIter = SLGetSelectLine();
	
	if (SLGetSelectLine() == m_TextList.end())
		return;
	
	// 아이템인가?
	if (QUEST_REQUEST_ITEM == m_TextListIter->m_dwType
		|| QUEST_REWARD_ITEM == m_TextListIter->m_dwType)
	{
		int nX = m_iPos_x + GetWidth() / 2;
		int nY = GetRenderLinePos_y(SLGetSelectLineNum());
		::RenderItemInfo(nX, nY, m_TextListIter->m_pItem, false, 0, true);
	}
}

// DoLineMouseAction() 함수 호출전에 호출 됨.
void CUIQuestContentsListBox::DoActionSub(BOOL bMessageOnly)
{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	bMessageOnly = FALSE;
	SLSetSelectLine(bMessageOnly);	// 마우스 커서가 아이템, 스킬 텍스트 항목 위에 없을 때 툴팁을 안그리기 위함.
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	SLSetSelectLine(0);	// 마우스 커서가 아이템, 스킬 텍스트 항목 위에 없을 때 툴팁을 안그리기 위함.
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
}

BOOL CUIQuestContentsListBox::DoLineMouseAction(int iLineNumber)
{
	// 각 항목에 마우스 커서가 있으면 항목 선택.
	if (::CheckMouseIn(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13))
	{		
		int nSelLine = m_iCurrentRenderEndLine + iLineNumber + 1;
		SLSetSelectLine(nSelLine);
	}

	return TRUE;
}
#endif	// ASG_ADD_NEW_QUEST_SYSTEM 

#ifdef PBG_ADD_INGAMESHOP_UI_ITEMSHOP
CUIInGameShopListBox::CUIInGameShopListBox()
{
	m_iMaxLineCount = UIMAX_TEXT_LINE;
	m_iCurrentRenderEndLine = 0;
	m_iNumRenderLine = 9;

	m_fScrollBarRange_top = 0;
	m_fScrollBarRange_bottom = 0;

	m_fScrollBarPos_y = 0;
	m_fScrollBarWidth = 13;
	m_fScrollBarHeight = 0;

	m_fScrollBarClickPos_y = 0;
	m_bScrollBtnClick = FALSE;
	m_bScrollBarClick = FALSE;

	m_bUseSelectLine = TRUE;
	m_bUseNewUIScrollBar = TRUE;
	m_iScrollType = UILISTBOX_SCROLL_UPDOWN;

	SetSize(LISTBOX_WIDTH, LISTBOX_HEIGHT);
	SetPosition(490, 360);
}

void CUIInGameShopListBox::AddText(IGS_StorageItem& _StorageItem)
{
	static IGS_StorageItem sItem;

	sItem.m_bIsSelected = FALSE;

	sItem.m_iStorageSeq			= _StorageItem.m_iStorageSeq;
	sItem.m_iStorageItemSeq		= _StorageItem.m_iStorageItemSeq;
	sItem.m_iStorageGroupCode	= _StorageItem.m_iStorageGroupCode;
	sItem.m_iProductSeq			= _StorageItem.m_iProductSeq;
	sItem.m_iPriceSeq			= _StorageItem.m_iPriceSeq;
	sItem.m_iNum				= _StorageItem.m_iNum;
	sItem.m_wItemCode			= _StorageItem.m_wItemCode;

	strncpy(sItem.m_szName, _StorageItem.m_szName, MAX_TEXT_LENGTH);
	strncpy(sItem.m_szNum, _StorageItem.m_szNum, MAX_TEXT_LENGTH);
	strncpy(sItem.m_szPeriod, _StorageItem.m_szPeriod, MAX_TEXT_LENGTH);
	strncpy(&(sItem.m_szType), &(_StorageItem.m_szType), sizeof(unicode::t_char));
	strncpy(sItem.m_szSendUserName, _StorageItem.m_szSendUserName, MAX_ID_SIZE+1);
	strncpy(sItem.m_szMessage, _StorageItem.m_szMessage, MAX_GIFT_MESSAGE_SIZE);

	m_TextList.push_front(sItem);

	RemoveText();
	SLSetSelectLine(0);

	if(GetLineNum() > m_iNumRenderLine) 
		++m_iCurrentRenderEndLine;

	if (GetLineNum() < m_iNumRenderLine);
	else if(GetLineNum() - m_iCurrentRenderEndLine < m_iNumRenderLine) 
		m_iCurrentRenderEndLine = GetLineNum() - m_iNumRenderLine;

	if (m_TextList.size() == 1)
		SLSetSelectLine(1);

	if (m_TextList.empty() == FALSE)
	{
		SLSetSelectLine(GetLineNum());
		Scrolling(-10000);
	}
}

#ifndef KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT					// #ifndef
void CUIInGameShopListBox::DelData(int iIndex)
{
	BOOL bFind = FALSE;
	int iCnt = 0;
	for (m_TextListIter = m_TextList.begin(); m_TextListIter != m_TextList.end(); ++m_TextListIter, iCnt++)
	{
		if (iCnt == iIndex)
		{
			bFind = TRUE;
			break;
		}
	}
	
	if (bFind == FALSE)
		return;
	
	if (m_TextList.size() == 1)
	{
		m_TextList.erase(m_TextListIter);
		SLSetSelectLine(0);
		return;
	}
	
	if (SLGetSelectLineNum() != 1)
		SLSelectNextLine();
	
	m_TextList.erase(m_TextListIter);

	RemoveText();

	if(GetLineNum() > m_iNumRenderLine) 
		++m_iCurrentRenderEndLine;

	if (GetLineNum() < m_iNumRenderLine);
	else if(GetLineNum() - m_iCurrentRenderEndLine < m_iNumRenderLine) 
		m_iCurrentRenderEndLine = GetLineNum() - m_iNumRenderLine;
}
#endif // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT

void CUIInGameShopListBox::SetNumRenderLine(int iLine)
{
	if (iLine < m_iNumRenderLine && iLine < GetLineNum())
		++m_iCurrentRenderEndLine;
	else if (iLine > GetLineNum())
		m_iCurrentRenderEndLine = 0;

	m_iNumRenderLine = iLine;
}

void CUIInGameShopListBox::RenderInterface()
{
	if (GetState() != UISTATE_SCROLL)
		ComputeScrollBar();

#ifndef KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT				// #ifndef
	// 렌더되는 줄 수 보다 리스트 줄 수가 작거나 같으면 스크롤바를 안그림.
	if (GetLineNum() <= m_iNumRenderLine)
		return;

	g_pGuardWindow->RenderScrollBarFrame(m_iPos_x + m_iWidth - 8, m_fScrollBarRange_top,
		m_fScrollBarRange_bottom - m_fScrollBarRange_top);
	g_pGuardWindow->RenderScrollBar(m_iPos_x + m_iWidth - 12, m_fScrollBarPos_y,
		(GetState() == UISTATE_SCROLL && MouseLButtonPush));
#endif // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
}

int CUIInGameShopListBox::GetRenderLinePos_y(int iLineNumber)
{
	if (GetLineNum() > m_iNumRenderLine)
		return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1)* 13 - iLineNumber * 13 + 3);
	else
		return (m_iPos_y - m_iHeight + (GetLineNum() - 1) * 13 - iLineNumber * 13 + 3);
}

BOOL CUIInGameShopListBox::RenderDataLine(int iLineNumber)
{
	EnableAlphaTest();
	
	if(SLGetSelectLineNum() == m_iCurrentRenderEndLine + iLineNumber + 1)
	{
		::glColor4f(0.15f, 0.3f, 0.4f, 0.5f);
		RenderColor(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 4, 13);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glEnable(GL_TEXTURE_2D);
	}

	g_pRenderText->SetTextColor(255, 230, 210, 255);
	g_pRenderText->SetBgColor(0);
	
#ifdef KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
	int iPos_x = m_iPos_x;
#else // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
	int iPos_x = m_iPos_x+3;
#endif // KJH_MOD_INGAMESHOP_ITEM_STORAGE_PAGE_UNIT
	int iPos_y = GetRenderLinePos_y(iLineNumber);

	unicode::t_char szItemName[MAX_TEXT_LENGTH];
#ifdef KJH_MOD_RENDER_INGAMESHOP_KEEPBOX_ITEM
	if( m_TextListIter->m_iNum > 1 )
	{
		sprintf(szItemName, "%s(%d)", m_TextListIter->m_szName, m_TextListIter->m_iNum);
		g_pRenderText->RenderText(iPos_x, iPos_y, szItemName, 98, 0, RT3_SORT_LEFT);
	}
	else
	{
		g_pRenderText->RenderText(iPos_x, iPos_y, m_TextListIter->m_szName, 98, 0, RT3_SORT_LEFT);	
	}

	g_pRenderText->RenderText(iPos_x + 102, iPos_y, m_TextListIter->m_szPeriod, 33, 0, RT3_SORT_RIGHT);
#else // KJH_MOD_RENDER_INGAMESHOP_KEEPBOX_ITEM
	if( m_TextListIter->m_iNum <= 0 )
	{
		g_pRenderText->RenderText(iPos_x, iPos_y, m_TextListIter->m_szName, 93, 0, RT3_SORT_LEFT);
	}
	else
	{
		sprintf(szItemName, "%s(%d)", m_TextListIter->m_szName, m_TextListIter->m_iNum);
		g_pRenderText->RenderText(iPos_x, iPos_y, szItemName, 93, 0, RT3_SORT_LEFT);
	}

	g_pRenderText->RenderText(iPos_x + m_iWidth*0.72f, iPos_y, m_TextListIter->m_szPeriod, 28, 0, RT3_SORT_LEFT);
#endif // KJH_MOD_RENDER_INGAMESHOP_KEEPBOX_ITEM

	DisableAlphaBlend(); 

	return TRUE;
}

BOOL CUIInGameShopListBox::DoLineMouseAction(int iLineNumber)
{
	if (::CheckMouseIn(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13))
	{
		if (MouseLButtonPush)	// MouseLButtonDBClick
		{
			MouseLButtonPush = false;

			int nSelLine = m_iCurrentRenderEndLine + iLineNumber + 1;

			// 선택한 줄이 이전에 선택한 줄이라면 리턴.
			if (SLGetSelectLineNum() == nSelLine)
				return TRUE;

			SLSetSelectLine(nSelLine);
			m_TextListIter->m_bIsSelected = (m_TextListIter->m_bIsSelected + 1) % 2;	// T/F Reverse

			m_TextListIter = SLGetSelectLine();
		}
	}
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////////
// 단일 구매창 리스트 박스
CUIBuyingListBox::CUIBuyingListBox()
{
	m_iMaxLineCount = UIMAX_TEXT_LINE;
	m_iCurrentRenderEndLine = 0;
	m_iNumRenderLine = 7;

	m_fScrollBarRange_top = 0;
	m_fScrollBarRange_bottom = 0;

	m_fScrollBarPos_y = 0;
	m_fScrollBarWidth = 13;
	m_fScrollBarHeight = 0;

	m_fScrollBarClickPos_y = 0;
	m_bScrollBtnClick = FALSE;
	m_bScrollBarClick = FALSE;

	m_bUseSelectLine = TRUE;
	m_bUseNewUIScrollBar = TRUE;
	m_iScrollType = UILISTBOX_SCROLL_DOWNUP;

	m_bRenderLineColor = true;

	SetSize(LISTBOX_WIDTH, LISTBOX_HEIGHT);
}

extern char TextList[30][100];

void CUIBuyingListBox::AddText(const char* pszExplanationText)
{
	if(pszExplanationText == NULL || pszExplanationText[0] == '\0')
		return;

	static IGS_BuyList sIGS_Buying;
	sIGS_Buying.m_bIsSelected = FALSE;
	strncpy(sIGS_Buying.m_pszItemExplanation, pszExplanationText, LINE_TEXTMAX);
	m_TextList.push_front(sIGS_Buying);

	RemoveText();
	SLSetSelectLine(0);

	if(GetLineNum() > m_iNumRenderLine) 
		++m_iCurrentRenderEndLine;

	if (GetLineNum() < m_iNumRenderLine);
	else if(GetLineNum() - m_iCurrentRenderEndLine < m_iNumRenderLine) 
		m_iCurrentRenderEndLine = GetLineNum() - m_iNumRenderLine;

	if (m_TextList.size() == 1)
		SLSetSelectLine(1);

	if (m_TextList.empty() == FALSE)
	{
		SLSetSelectLine(GetLineNum());
		Scrolling(-10000);
	}
}
void CUIBuyingListBox::SetNumRenderLine(int iLine)
{
	if (iLine < m_iNumRenderLine && iLine < GetLineNum())
		++m_iCurrentRenderEndLine;
	else if (iLine > GetLineNum())
		m_iCurrentRenderEndLine = 0;

	m_iNumRenderLine = iLine;
}
void CUIBuyingListBox::RenderInterface()
{
	EnableAlphaTest();
	if (GetState() != UISTATE_SCROLL)
		ComputeScrollBar();

	if (GetLineNum() <= m_iNumRenderLine)
		return;

	g_pGuardWindow->RenderScrollBarFrame(m_iPos_x + m_iWidth - 8, m_fScrollBarRange_top,
		m_fScrollBarRange_bottom - m_fScrollBarRange_top);
	g_pGuardWindow->RenderScrollBar(m_iPos_x + m_iWidth - 12, m_fScrollBarPos_y,
		(GetState() == UISTATE_SCROLL && MouseLButtonPush));
	DisableAlphaBlend();
}
int CUIBuyingListBox::GetRenderLinePos_y(int iLineNumber)
{
	if (GetLineNum() > m_iNumRenderLine)
		return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1)* 13 - iLineNumber * 13 + 3);
	else
		return (m_iPos_y - m_iHeight + (GetLineNum() - 1) * 13 - iLineNumber * 13 + 3);
}
BOOL CUIBuyingListBox::RenderDataLine(int iLineNumber)
{
	EnableAlphaTest();
	
	if(SLGetSelectLineNum() == m_iCurrentRenderEndLine + iLineNumber + 1 && GetLineColorRender())
	{
		::glColor4f(0.15f, 0.3f, 0.4f, 0.5f);
		RenderColor(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glEnable(GL_TEXTURE_2D);
	}

	g_pRenderText->SetTextColor(255, 230, 210, 255);
	g_pRenderText->SetBgColor(0);
	
	int iPos_x = m_iPos_x+3;
	int iPos_y = GetRenderLinePos_y(iLineNumber);
	
	char Text[MAX_TEXT_LENGTH + 1] = {0};
	if(m_TextListIter->m_pszItemExplanation != NULL)
	{
		sprintf(Text,"%s",m_TextListIter->m_pszItemExplanation);
		g_pRenderText->RenderText(iPos_x, iPos_y, Text);
	}
	DisableAlphaBlend();

	return TRUE;
}
void CUIBuyingListBox::SetLineColorRender(const bool _LineColor)
{
	m_bRenderLineColor = _LineColor;
}
BOOL CUIBuyingListBox::DoLineMouseAction(int iLineNumber)
{
	if (::CheckMouseIn(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13))
	{
		if (MouseLButtonPush)
		{
			MouseLButtonPush = false;

			int nSelLine = m_iCurrentRenderEndLine + iLineNumber + 1;

			// 선택한 줄이 이전에 선택한 줄이라면 리턴.
			if (SLGetSelectLineNum() == nSelLine)
				return TRUE;

			SLSetSelectLine(nSelLine);
			m_TextListIter->m_bIsSelected = (m_TextListIter->m_bIsSelected + 1) % 2;	// T/F Reverse

			m_TextListIter = SLGetSelectLine();
		}
	}
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////////
// 패키지 구매창 리스트 박스
CUIPackCheckBuyingListBox::CUIPackCheckBuyingListBox()
{
	m_iMaxLineCount = UIMAX_TEXT_LINE;
	m_iCurrentRenderEndLine = 0;
	m_iNumRenderLine = 3;

	m_fScrollBarRange_top = 0;
	m_fScrollBarRange_bottom = 0;

	m_fScrollBarPos_y = 0;
	m_fScrollBarWidth = 13;
	m_fScrollBarHeight = 0;

	m_fScrollBarClickPos_y = 0;
	m_bScrollBtnClick = FALSE;
	m_bScrollBarClick = FALSE;

	m_bUseSelectLine = TRUE;
	m_bUseNewUIScrollBar = TRUE;
	m_iScrollType = UILISTBOX_SCROLL_DOWNUP;
	
	m_iCurrentLine = -1;

	SetSize(LISTBOX_WIDTH, LISTBOX_HEIGHT);
}
CUIPackCheckBuyingListBox::~CUIPackCheckBuyingListBox()
{
	CRadioButton::m_nIterIndex=0;
}
void CUIPackCheckBuyingListBox::AddText(IGS_SelectBuyItem& _Item)
{
	static IGS_SelectBuyItem sItem;

	sItem.m_iPackageSeq	= _Item.m_iPackageSeq;
	sItem.m_iDisplaySeq	= _Item.m_iDisplaySeq;
	sItem.m_iPriceSeq	= _Item.m_iPriceSeq;
	sItem.m_wItemCode	= _Item.m_wItemCode;
#ifndef KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL		// #ifndef
#ifdef KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
	sItem.m_iCashType	= _Item.m_iCashType;
#endif // KJH_MOD_INGAMESHOP_GLOBAL_CASHPOINT_ONLY_GLOBAL
#endif // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	
	strncpy(sItem.m_szItemName, _Item.m_szItemName, MAX_TEXT_LENGTH);
	strncpy(sItem.m_szItemPrice, _Item.m_szItemPrice, MAX_TEXT_LENGTH);
#ifdef KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	strncpy(sItem.m_szItemPriceUnit, _Item.m_szItemPriceUnit, MAX_TEXT_LENGTH);
#endif // KJH_MOD_INGAMESHOP_SELECT_CASHPOINT_SYSYEM_ONLY_GLOBAL
	strncpy(sItem.m_szItemPeriod, _Item.m_szItemPeriod, MAX_TEXT_LENGTH);
	strncpy(sItem.m_szAttribute, _Item.m_szAttribute, MAX_TEXT_LENGTH);

	// 클래스 생성시에 static변수로 갯수를 인덱스로 사용한다
	// 리스트 박스의 선택을 따라 이동하기 때문에 사용할 필요 없음
	CRadioButton::m_nIterIndex++;
	sItem.m_RadioBtn.SetRadioBtnIsEnable(CRadioButton::m_nIterIndex);

	m_TextList.push_front(sItem);

	RemoveText();
	SLSetSelectLine(0);

	if(GetLineNum() > m_iNumRenderLine) 
		++m_iCurrentRenderEndLine;

	if (GetLineNum() < m_iNumRenderLine);
	else if(GetLineNum() - m_iCurrentRenderEndLine < m_iNumRenderLine) 
		m_iCurrentRenderEndLine = GetLineNum() - m_iNumRenderLine;

	if (m_TextList.size() == 1)
		SLSetSelectLine(1);

	if (m_TextList.empty() == FALSE)
	{
		SLSetSelectLine(GetLineNum());
		Scrolling(-10000);
	}
}
void CUIPackCheckBuyingListBox::SetNumRenderLine(int iLine)
{
	if (iLine < m_iNumRenderLine && iLine < GetLineNum())
		++m_iCurrentRenderEndLine;
	else if (iLine > GetLineNum())
		m_iCurrentRenderEndLine = 0;

	m_iNumRenderLine = iLine;
}
void CUIPackCheckBuyingListBox::RenderInterface()
{
	EnableAlphaTest();
	if (GetState() != UISTATE_SCROLL)
		ComputeScrollBar();

	if (GetLineNum() <= m_iNumRenderLine)
		return;

	g_pGuardWindow->RenderScrollBarFrame(m_iPos_x + m_iWidth - 8, m_fScrollBarRange_top,
		m_fScrollBarRange_bottom - m_fScrollBarRange_top);
	g_pGuardWindow->RenderScrollBar(m_iPos_x + m_iWidth - 12, m_fScrollBarPos_y,
		(GetState() == UISTATE_SCROLL && MouseLButtonPush));
	DisableAlphaBlend();
}
BOOL CUIPackCheckBuyingListBox::RenderDataLine(int nLine)
{
	EnableAlphaTest();
	
	if(SLGetSelectLineNum() == m_iCurrentRenderEndLine + nLine + 1)
	{
		::glColor4f(0.07f, 0.31f, 0.31f, 0.5f);
		RenderColor(m_iPos_x+3, GetRenderLinePos_y(nLine)+1, m_iWidth - m_fScrollBarWidth + 1, TEXT_HEIGHTSIZE-6);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glEnable(GL_TEXTURE_2D);
	}

	g_pRenderText->SetTextColor(255, 230, 210, 255);
	g_pRenderText->SetBgColor(0);
	
	int iPos_x = m_iPos_x+3;
	int iPos_y = GetRenderLinePos_y(nLine);
	
	char Text[MAX_TEXT_LENGTH + 1] = {0};

	if(m_TextListIter->m_szItemName != NULL)
	{
 		// 클래스의 인덱스와 체크된 인덱스를 비교 아니라면 false
// 		if(m_TextListIter->m_RadioBtn.GetRadioBtnIsEnable() != CRadioButton::m_nIterIndex)
// 			m_TextListIter->m_RadioBtn.SetCheckState(false);

		if(SLGetSelectLineNum() == m_iCurrentRenderEndLine + nLine + 1)		//선택된 리스트는 무조건 체크상태로 만든다
			m_TextListIter->m_RadioBtn.UpdateActionCheck(true);
		else
			m_TextListIter->m_RadioBtn.UpdateActionCheck(false);

		// 라디오 버튼 박스의 렌더
 		m_TextListIter->m_RadioBtn.SetRadioBtnRect(iPos_x+1, iPos_y+TEXT_HEIGHTSIZE*0.2f);
		m_TextListIter->m_RadioBtn.RadiobuttonBoxRender();

		g_pRenderText->SetFont(g_hFontBold);
		sprintf(Text,"%s",m_TextListIter->m_szItemName);
		g_pRenderText->RenderText(iPos_x+20, iPos_y+5, Text);

		g_pRenderText->SetFont(g_hFont);
		sprintf(Text,"%s",m_TextListIter->m_szAttribute);
		g_pRenderText->RenderText(iPos_x+20, iPos_y+17, Text);
	}
	DisableAlphaBlend();

	return TRUE;
}
BOOL CUIPackCheckBuyingListBox::DoLineMouseAction(int nLine)
{
	// 라디오버튼박스의 마우스 액션
//	m_TextListIter->m_RadioBtn.UpdateMouseAction();

	if(::CheckMouseIn(m_iPos_x, GetRenderLinePos_y(nLine) - 3, m_iWidth - m_fScrollBarWidth + 1, TEXT_HEIGHTSIZE))
	{
		if (MouseLButtonPush)
		{
			MouseLButtonPush = false;

			int nSelLine = m_iCurrentRenderEndLine + nLine + 1;

			// 선택한 줄이 이전에 선택한 줄이라면 리턴.
			if (SLGetSelectLineNum() == nSelLine)
				return TRUE;

			SLSetSelectLine(nSelLine);
			m_TextListIter->m_bIsSelected = (m_TextListIter->m_bIsSelected + 1) % 2;	// T/F Reverse
			
			m_TextListIter = SLGetSelectLine();
		}
	}
	return TRUE;
}
int CUIPackCheckBuyingListBox::GetRenderLinePos_y(int nLine)
{
	// 두줄씩받는다
	if (GetLineNum() > m_iNumRenderLine)
		return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1) * TEXT_HEIGHTSIZE - nLine * TEXT_HEIGHTSIZE + TEXT_HEIGHTSIZE*0.1f);
	else
		return (m_iPos_y - m_iHeight + (GetLineNum() - 1) * TEXT_HEIGHTSIZE - nLine * TEXT_HEIGHTSIZE + TEXT_HEIGHTSIZE*0.1f);
}

BOOL CUIPackCheckBuyingListBox::IsChangeLine()
{
	if( m_iCurrentLine != SLGetSelectLineNum())
	{
		m_iCurrentLine = SLGetSelectLineNum();
		return TRUE;
	}

	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////////
// 라디오버튼 3가지 액션
CRadioButton::CRadioButton()
{
	m_byMouseState = LBTN_DEFAULT;
	m_bCheckState = false;
	m_rtCheckBtn.top = 0;
	m_rtCheckBtn.bottom = BTN_HEIGHT;
	m_rtCheckBtn.left = 0;
	m_rtCheckBtn.right = BTN_WIDTH;
	m_nRadioBtnEnable=0;
}
CRadioButton::~CRadioButton()
{
	// n/a
}
void CRadioButton::SetCheckState(bool _Value)
{
	//체크된 상태
	m_bCheckState =_Value;
}
void CRadioButton::SetRadioBtnIsEnable(int _Value)
{
	// 클래스의 인덱스 차후 활성화된 클래스 인덱스랑 비교
	m_nRadioBtnEnable = _Value;
}
bool CRadioButton::UpdateMouseAction()
{
	bool _Temp = IsRadioBtn(m_rtCheckBtn);

	if(SEASON3B::IsPress(VK_LBUTTON) && _Temp)
	{
		// 누르는 순간
		m_byMouseState = LBTN_DOWN;
		return true;
	}
	else if(SEASON3B::IsRelease(VK_LBUTTON) && _Temp)
	{
		// 뗀 순간
		m_bCheckState ^= 1;
		m_byMouseState = LBTN_UP;
		// 클래스의 인덱스 번호를 static변수에 기억시킨다
		m_nIterIndex = m_nRadioBtnEnable;
		return true;
	}
	else if(SEASON3B::IsRepeat(VK_LBUTTON) && _Temp)
	{
		// 누르고 있는 동안
		m_byMouseState = LBTN_DOWN;
		return true;
	}

	if(m_byMouseState == LBTN_DOWN && !_Temp)
	{
		// 누른 상태에서 외부로 포커스를 갖고 갔을때 과거 상태값으로 돌린다
		m_byMouseState = LBTN_UP;
		return false;
	}
	return false;
}

bool CRadioButton::UpdateActionCheck(int _nState)
{
	//체크 박스의 선택에 따라 라디오 버튼도 선택을 따라가게 한다
	m_bCheckState = _nState;

	if(m_bCheckState)
		m_byMouseState = LBTN_UP;		//리스트 박스의 선택에 따라가기 위해 무조건 체크
	else
		m_byMouseState = LBTN_DEFAULT;	//선택되지 않은 것은 선택되지 않은 이미지 출력토록

	return true;
}

void CRadioButton::SetRadioBtnRect(float _x, float _y, float _width, float _height)
{
	m_rtCheckBtn.top = _y;
	m_rtCheckBtn.bottom = _y + _height;
	m_rtCheckBtn.left = _x;
	m_rtCheckBtn.right = _x + _width;
}
bool CRadioButton::IsRadioBtn(RECT _rt)
{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	bool bSuccess = false;
	if(MouseX >= _rt.left && MouseX < _rt.right && MouseY < _rt.bottom && MouseY >= _rt.top)
	{
		bSuccess = true;
	}
	else
	{
		bSuccess = false;
	}

	return bSuccess;
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	if(MouseX >= _rt.left && MouseX < _rt.right && MouseY < _rt.bottom && MouseY >= _rt.top)
		return true;
	else
		return false;
	return false;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
}
void CRadioButton::RadiobuttonBoxRender()
{
	switch(m_byMouseState)
	{
	case LBTN_DEFAULT:
		// 체크 없는 이미지
		RenderImage(IMAGE_CHECKBTN, m_rtCheckBtn.left, m_rtCheckBtn.top, BTN_WIDTH, BTN_HEIGHT, 0, 0);
		break;
	case LBTN_UP:
		// 체크 상태유무
		m_bCheckState ? RenderImage(IMAGE_CHECKBTN, m_rtCheckBtn.left, m_rtCheckBtn.top, BTN_WIDTH, BTN_HEIGHT, 0, BTN_HEIGHT+BTN_SPACE) : RenderImage(IMAGE_CHECKBTN, m_rtCheckBtn.left, m_rtCheckBtn.top, BTN_WIDTH, BTN_HEIGHT, 0, 0);
		break;
	case LBTN_DOWN:
		// 눌리고 있는 상태를 그린다
		RenderImage(IMAGE_CHECKBTN, m_rtCheckBtn.left, m_rtCheckBtn.top, BTN_WIDTH, BTN_HEIGHT, 0, (BTN_HEIGHT+BTN_SPACE)*2.0f);
		break;
	}
}
void CRadioButton::RenderImage(GLuint uiImageType, float x, float y, float width, float height, float su, float sv)
{
	BITMAP_t *pImage = &Bitmaps[uiImageType];

	float u, v, uw, vh;
	u = ((su + 0.1f) / (float)pImage->Width);
	v = ((sv + 0.1f) / (float)pImage->Height);
	uw = (width - 0.1f) / (float)pImage->Width - (0.1f / (float)pImage->Width);
	vh = (height - 0.1f) / (float)pImage->Height - (0.1f / (float)pImage->Height);

	RenderBitmap(uiImageType, x, y, width, height, u, v, uw, vh);
}
int CRadioButton::m_nIterIndex = 0;
#endif //PBG_ADD_INGAMESHOP_UI_ITEMSHOP