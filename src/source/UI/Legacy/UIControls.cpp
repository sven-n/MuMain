///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Core/Input/KeyState.h"
#include "Core/Time/FrameTimerScheduler.h"
#include "GameLogic/Items/CComGem.h"
#include "UIControls.h"
#include "UIWindows.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Render/Textures/ZzzTexture.h"
#include "Engine/Object/ZzzInventory.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "Audio/DSPlaySound.h"
#include "I18N/All.h"


#include "Core/Utilities/ReadScript.h"
#include "GameLogic/Events/Cinematic/CMVP1stDirection.h"
#include "UIManager.h"
#include "GameLogic/Items/InventoryUtils.h"
#include "UI/NewUI/NewUISystem.h"
#include <vector>

extern BYTE m_CrywolfState;


int g_iWidthEx = 5;

extern int g_iChatInputType;

extern BOOL g_bUseWindowMode;

#define ARRAY_SIZE(pArray) (sizeof(pArray)/sizeof(pArray[0]))

int CutStr(const wchar_t* pszSrcText, wchar_t* pTextOut, const int iTargetPixelWidth, const int iMaxOutLine, const int iOutStrLength, const int iFirstLineTab /* = 0 */)
{
    if (iFirstLineTab < 0)
    {
      return 0;
    }

    if (pszSrcText == nullptr)
    {
        assert(!"CutStr Error");
        return 0;
    }

    auto tempString = std::wstring(pszSrcText);
    int iCharIndex = 0, iLineIndex = 0;
    const int iScreenRatePixelWidth = iTargetPixelWidth * g_fScreenRate_x - 5;

    const int totalCharacters = tempString.length();
    int processedSourceCharacters = 0;
    SIZE iSize;
    while (!tempString.empty() && iLineIndex < iMaxOutLine)
    {
        GetTextExtentPoint32(g_pRenderText->GetFontDC(), tempString.c_str(), tempString.length(), &iSize);

        if (iLineIndex == 0)
            iSize.cx += iFirstLineTab;

        const auto isTooWideInPixels = iSize.cx >= iScreenRatePixelWidth;
        const auto isTooLongInCharacters = (int)tempString.length() >= iOutStrLength - 1;
        if (isTooWideInPixels || isTooLongInCharacters)
        {
          // then remove the last word/token from the string and try next loop iteration again ...
          const auto iPosLastSpace = tempString.find_last_of(L' ');
          iCharIndex = (iPosLastSpace == std::wstring::npos) ? tempString.length() - 1 : iPosLastSpace;
          tempString = tempString.substr(0, iCharIndex);
        }
        else
        {
            // we can copy that to the destination
            tempString.copy(pTextOut, tempString.length(), 0);
            iLineIndex++;
            processedSourceCharacters += tempString.length();

            pTextOut += iOutStrLength; // move destination pointer to the next line
            if (processedSourceCharacters < totalCharacters)
            {
              tempString = std::wstring(pszSrcText + processedSourceCharacters);
            }
            else
            {
              tempString = L"";
              break;
            }
        }
    }


    return iLineIndex;
}

int CutText3(const wchar_t* pszText, wchar_t* pTextOut, const int TargetWidth, const int iMaxOutLine, const int iOutStrLength, const int iFirstLineTab, const BOOL bReverseWrite)
{
    return CutStr(pszText, pTextOut, TargetWidth, iMaxOutLine, iOutStrLength, iFirstLineTab);
}

void CutText4(const wchar_t* pszSource, wchar_t* pszResult1, wchar_t* pszResult2, int iCutCount)
{
    if (pszSource == nullptr || pszSource[0] == '\0') return;
    auto sourceString = std::wstring(pszSource);
    int iLength = sourceString.length();
    int iMove = 2; // might be 4, too
    int iTextSize = 0;
    for (int i = 0; i < iLength; )
    {
        if (i + iMove > iCutCount) break;
        else i += iMove;

        iTextSize = i;
    }
    wcsncpy(pszResult1, pszSource, iTextSize);
    pszResult1[iTextSize] = '\0';
    if (pszResult2 != nullptr)
    {
        wcsncpy(pszResult2, pszSource + iTextSize, iLength - iTextSize);
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
        if (MouseX >= iPos_x && MouseX < iPos_x + iWidth &&
            MouseY >= iPos_y - iHeight && MouseY < iPos_y)
            return TRUE;
        else return FALSE;
    }
    else
    {
        if (MouseX >= iPos_x && MouseX < iPos_x + iWidth &&
            MouseY >= iPos_y && MouseY < iPos_y + iHeight)
            return TRUE;
        else return FALSE;
    }
}

void CUIMessage::SendUIMessage(int iMessage, LONG_PTR iParam1, LONG_PTR iParam2)
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

    auto msgiter = m_MessageList.begin();
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

    DoActionSub(bMessageOnly);

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
                CUIControl* pRootWindow = nullptr;
                if (GetParentUIID() == 0)
                {
                    pRootWindow = this;
                }
                else
                {
                    pRootWindow = g_pWindowMgr->GetWindow(GetParentUIID());
                }

                while (pRootWindow != nullptr && pRootWindow->GetParentUIID() != 0)
                {
                    pRootWindow = g_pWindowMgr->GetWindow(pRootWindow->GetParentUIID());
                }
                if (pRootWindow != nullptr && g_pWindowMgr != nullptr)
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
    switch (m_WorkMessage.m_iMessage)
    {
    case UI_MESSAGE_P_MOVE:
        if (m_dwParentUIID != 0)
        {
            CUIBaseWindow* pWindow = g_pWindowMgr->GetWindow(m_dwParentUIID);
            if (pWindow != nullptr)
            {
                switch (m_iArrangeType)
                {
                case 0:
                    SetPosition(pWindow->RPos_x(m_iRelativePos_x), pWindow->RPos_y(m_iRelativePos_y));
                    break;
                case 1:
                    SetPosition(pWindow->RPos_x(-1 * m_iRelativePos_x) + pWindow->RWidth(), pWindow->RPos_y(m_iRelativePos_y));
                    break;
                case 2:
                    SetPosition(pWindow->RPos_x(m_iRelativePos_x), pWindow->RPos_y(-1 * m_iRelativePos_y) + pWindow->RHeight());
                    break;
                case 3:
                    SetPosition(pWindow->RPos_x(-1 * m_iRelativePos_x) + pWindow->RWidth(), pWindow->RPos_y(-1 * m_iRelativePos_y) + pWindow->RHeight());
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
            CUIBaseWindow* pWindow = g_pWindowMgr->GetWindow(m_dwParentUIID);
            if (pWindow != nullptr)
            {
                switch (m_iResizeType)
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
    m_pszCaption = nullptr;
    SetSize(70, 20);
    m_bMouseState = FALSE;
}

CUIButton::~CUIButton()
{
    if (m_pszCaption != nullptr)
    {
        delete[] m_pszCaption;
        m_pszCaption = nullptr;
    }
    if (g_dwCurrentPressedButtonID == GetUIID()) g_dwCurrentPressedButtonID = 0;
}

void CUIButton::Init(DWORD dwButtonID, const wchar_t* pszCaption)
{
    m_dwButtonID = dwButtonID;
    SetCaption(pszCaption);
}

void CUIButton::SetCaption(const wchar_t* pszCaption)
{
    if (pszCaption == nullptr) return;
    if (m_pszCaption != nullptr)
    {
        if (wcscmp(pszCaption, m_pszCaption) == 0) return;
        delete[] m_pszCaption;
    }
    m_pszCaption = new wchar_t[wcslen(pszCaption) + 1];
    wcsncpy(m_pszCaption, pszCaption, wcslen(pszCaption) + 1);
    //m_pszCaption[wcslen(pszCaption)] = '\0';
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
        RenderBitmap(BITMAP_INTERFACE_EX + 9, m_iPos_x, m_iPos_y, (float)m_iWidth, (float)m_iHeight, 0.f, 0.f, 49.f / 64.f, 16.f / 16.f);

        if (m_pszCaption != nullptr)
        {
            SIZE TextSize;
            const int TextLen = lstrlen(m_pszCaption);
            GetTextExtentPoint32(g_pRenderText->GetFontDC(), m_pszCaption, TextLen, &TextSize);
            g_pRenderText->SetTextColor(230, 220, 200, 255);
            g_pRenderText->SetBgColor(0);
            g_pRenderText->RenderText(m_iPos_x + (m_iWidth - (float)TextSize.cx / g_fScreenRate_x + 0.5f) / 2, m_iPos_y + 1 + (m_iHeight - (float)TextSize.cy / g_fScreenRate_y + 0.5f) / 2, m_pszCaption);
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
        RenderBitmap(BITMAP_INTERFACE_EX + 9, m_iPos_x + 1, m_iPos_y + 1, (float)m_iWidth - 1, (float)m_iHeight - 1, 0.f, 0.f, 48.f / 64.f, 15.f / 16.f);
    else
        RenderBitmap(BITMAP_INTERFACE_EX + 9, m_iPos_x, m_iPos_y, (float)m_iWidth, (float)m_iHeight, 0.f, 0.f, 49.f / 64.f, 16.f / 16.f);

    if (m_pszCaption != nullptr)
    {
        SIZE TextSize;
        const int TextLen = lstrlen(m_pszCaption);

        GetTextExtentPoint32(g_pRenderText->GetFontDC(), m_pszCaption, TextLen, &TextSize);
        g_pRenderText->SetTextColor(230, 220, 200, 255);
        g_pRenderText->SetBgColor(0, 0, 0, 0);

        if (m_bMouseState == TRUE)
        {
            g_pRenderText->RenderText(m_iPos_x + 1 + (m_iWidth - (float)TextSize.cx / g_fScreenRate_x + 0.5f) / 2, m_iPos_y + 2 + (m_iHeight - (float)TextSize.cy / g_fScreenRate_y + 0.5f) / 2, m_pszCaption);
        }
        else
        {
            g_pRenderText->RenderText(m_iPos_x + (m_iWidth - (float)TextSize.cx / g_fScreenRate_x + 0.5f) / 2, m_iPos_y + 1 + (m_iHeight - (float)TextSize.cy / g_fScreenRate_y + 0.5f) / 2, m_pszCaption);
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
    typename std::deque<T>::iterator EndIter = m_TextList.end();
    for (m_TextListIter = m_TextList.begin(); m_TextListIter != EndIter; ++m_TextListIter)
    {
        m_TextListIter->m_bIsSelected = bFlag;
    }
}

template <class T>
BOOL CUITextListBox<T>::HaveCheckedLine()
{
    typename std::deque<T>::iterator EndIter = m_TextList.end();
    for (m_TextListIter = m_TextList.begin(); m_TextListIter != EndIter; ++m_TextListIter)
    {
        if (m_TextListIter->m_bIsSelected == TRUE) return TRUE;
    }
    return FALSE;
}

template <class T>
int CUITextListBox<T>::GetCheckedLines(std::deque<T*>* pSelectLineList)
{
    int iSelectLineNum = 0;
    typename std::deque<T>::iterator EndIter = m_TextList.end();
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
    for (typename std::deque<T>::iterator resultIter = m_TextList.begin(); resultIter != m_TextList.end(); ++resultIter, ++iLineCount)
    {
        if (iLineCount == m_iSelectLineNum) return resultIter;
    }
    assert(!"SLGetSelectLine");
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

    glColor3f(1.f, 1.f, 1.f);

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
        m_fScrollBarHeight = 20;

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
        if (m_fScrollBarHeight < 2) m_fScrollBarHeight = 2;
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
    switch (m_WorkMessage.m_iMessage)
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
    case UI_MESSAGE_LISTSCRLTOP:
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

        if (Core::Input::IsKeyDown(VK_LEFT))
        {
        }
        if (Core::Input::IsKeyDown(VK_RIGHT))
        {
        }
        if (Core::Input::IsKeyDown(VK_UP))
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
        if (Core::Input::IsKeyDown(VK_DOWN))
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
        if (bKeyPress == FALSE)
        {
            m_bPressCursorKey = 0;
        }
    }
    if (CheckMouseInBox())
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
                else if (::CheckMouseIn(m_iPos_x + m_iWidth - m_fScrollBarWidth, m_fScrollBarPos_y,
                    m_fScrollBarWidth, m_fScrollBarHeight))
                {
                    if (GetState() == UISTATE_NORMAL && g_dwActiveUIID == 0)
                    {
                        g_dwActiveUIID = GetUIID();
                        SetState(UISTATE_SCROLL);
                        m_fScrollBarClickPos_y = MouseY - m_fScrollBarPos_y;
                    }
                }
                else if (::CheckMouseIn(m_iPos_x + m_iWidth - m_fScrollBarWidth, m_fScrollBarRange_top,
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
                else if (::CheckMouseIn(m_iPos_x + m_iWidth - m_fScrollBarWidth, m_fScrollBarPos_y + m_fScrollBarHeight,
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

                if (::CheckMouseIn(m_iPos_x + m_iWidth - 21 + iNewTypePos_x,
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
                if (::CheckMouseIn(m_iPos_x + m_iWidth - 21 + iNewTypePos_x, m_iPos_y - (m_bNewTypeScrollBar == TRUE ? 13 : 21), 13, 13))
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

                else if (::CheckMouseIn(m_iPos_x + m_iWidth - 19 + iNewTypePos_x, m_fScrollBarPos_y,
                    m_fScrollBarWidth, m_fScrollBarHeight))
                {
                    if (GetState() == UISTATE_NORMAL && g_dwActiveUIID == 0)
                    {
                        g_dwActiveUIID = GetUIID();
                        SetState(UISTATE_SCROLL);
                        m_fScrollBarClickPos_y = MouseY - m_fScrollBarPos_y;
                    }
                }

                else if (::CheckMouseIn(m_iPos_x + m_iWidth - 19 + iNewTypePos_x, m_fScrollBarRange_top,
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
                else if (::CheckMouseIn(m_iPos_x + m_iWidth - 19 + iNewTypePos_x, m_fScrollBarPos_y + m_fScrollBarHeight,
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
                    float fRate = (m_fScrollBarRange_bottom - m_fScrollBarPos_y - m_fScrollBarHeight * 1.5f) / (m_fScrollBarRange_bottom - m_fScrollBarRange_top - m_fScrollBarHeight * 1.5f);
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
                    m_iCurrentRenderEndLine = (m_fScrollBarRange_bottom - m_fScrollBarPos_y - m_fScrollBarHeight + 0.5f) / (m_fScrollBarRange_bottom - m_fScrollBarRange_top - (m_fScrollBarHeight > 2 ? 0 : 1.0f)) * (float)GetLineNum();
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

template class CUITextListBox<GUILDLIST_TEXT>;
template class CUITextListBox<WHISPER_TEXT>;
template class CUITextListBox<LETTER_TEXT>;
template class CUITextListBox<WINDOWLIST_TEXT>;
template class CUITextListBox<LETTERLIST_TEXT>;
template class CUITextListBox<SOCKETLIST_TEXT>;
template class CUITextListBox<GUILDLOG_TEXT>;
template class CUITextListBox<UNIONGUILD_TEXT>;
template class CUITextListBox<FILTERLIST_TEXT>;
template class CUITextListBox<UNMIX_TEXT>;
template class CUITextListBox<BCDECLAREGUILD_TEXT>;
template class CUITextListBox<BCGUILD_TEXT>;
template class CUITextListBox<MOVECOMMAND_TEXT>;
template class CUITextListBox<SCurQuestItem>;
template class CUITextListBox<SQuestContents>;
template class CUITextListBox<IGS_StorageItem>;
template class CUITextListBox<IGS_BuyList>;
template class CUITextListBox<IGS_SelectBuyItem>;

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

    m_iNumRenderLine = 18;
    SetPosition(460, 340);
    SetSize(170, 250);
}

void CUIGuildListBox::AddText(const wchar_t* pszID, BYTE Number, BYTE Server)
{
    if (pszID == nullptr || pszID[0] == '\0') return;

    if (GetLineNum() == 0)
    {
        if (wcscmp(pszID, Hero->ID) == 0) m_bIsGuildMaster = TRUE;
        else m_bIsGuildMaster = FALSE;
    }

    static GUILDLIST_TEXT text;
    text.m_bIsSelected = FALSE;
    wcsncpy(text.m_szID, pszID, MAX_USERNAME_SIZE + 1);
    //memcpy(text.m_szID, pszID, wcslen(pszID) + 1);
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
    const int iFillImageHeight = 40;
    const int iFrameImageHeight = 5;

    int iBlockHeight = m_iNumRenderLine / 3;
    m_iHeight = iFillImageHeight * iBlockHeight + iFrameImageHeight * 2;

    ComputeScrollBar();

    if (GetLineNum() >= m_iNumRenderLine)
    {
        RenderBitmap(BITMAP_INTERFACE_EX + 4, (float)m_iPos_x + m_iWidth - 19, (float)m_iPos_y - m_iHeight + 8, 13.0f, 13.0f, 0.0f, 0.0f, 13.0f / 16.0f, 13.0f / 16.0f);
        EnableAlphaTest();
        RenderBitmap(BITMAP_INTERFACE_EX + 4, (float)m_iPos_x + m_iWidth - 19, (float)m_iPos_y - 4, 13.0f, -13.0f, 0.0f, 0.0f, 13.0f / 16.0f, 13.0f / 16.0f);
        DisableAlphaBlend();

        RenderBitmap(BITMAP_INTERFACE_EX + 3, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth - 6, m_fScrollBarRange_top, m_fScrollBarWidth, m_fScrollBarRange_bottom - m_fScrollBarRange_top, 0.0f, 0.0f, 13.0f / 16.0f, 13.0f / 16.0f);

        RenderBitmap(BITMAP_INTERFACE_EX + 2, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth - 5, m_fScrollBarPos_y, m_fScrollBarWidth - 2, m_fScrollBarHeight, 0.0f, 0.0f, 11.0f / 16.0f, 11.0f / 16.0f);
    }
}

BOOL CUIGuildListBox::RenderDataLine(int iLineNumber)
{
    EnableAlphaTest();
    int iPos_x = m_iPos_x + 8;
    int iPos_y;
    if (GetLineNum() > m_iNumRenderLine) iPos_y = m_iPos_y - 16 - iLineNumber * 13;
    else iPos_y = m_iPos_y - 16 - (iLineNumber - GetLineNum() + m_iNumRenderLine) * 13;

    g_pRenderText->SetFont(g_hFontBold);

    if (++m_TextListIter == m_TextList.end())
    {
        --m_TextListIter;
        g_pRenderText->SetBgColor(40, 40, 150, 255);
        if (m_TextListIter->m_Server != 255)
            g_pRenderText->SetTextColor(0xFFFFFFFF);
        else
            g_pRenderText->SetTextColor(255, 196, 196, 196);

        CreateGuildMark(Hero->GuildMarkIndex);
        RenderBitmap(BITMAP_GUILD, (float)iPos_x, (float)iPos_y, 8, 8);
        iPos_x += 13;
    }
    else
    {
        --m_TextListIter;
        g_pRenderText->SetBgColor(0x00000000);
        if (m_TextListIter->m_Server != 255)
            g_pRenderText->SetTextColor(210, 230, 255, 255);
        else
            g_pRenderText->SetTextColor(210, 196, 196, 196);
    }

    wchar_t Text[MAX_TEXT_LENGTH + 1] = { 0 };
    mu_swprintf(Text, L"%ls", m_TextListIter->m_szID);
    g_pRenderText->RenderText(iPos_x, iPos_y, Text);

    if (m_TextListIter->m_Server != 255/* && m_TextListIter->m_Number != 0*/)
    {
        g_pRenderText->SetBgColor(255, 196, 0, 255);
        g_pRenderText->SetTextColor(0x00000000);
        mu_swprintf(Text, L"(%d)", m_TextListIter->m_Server + 1);
        g_pRenderText->RenderText(m_iPos_x + m_iWidth - 60, iPos_y, Text);
    }

    if (m_bIsGuildMaster == TRUE || wcscmp(m_TextListIter->m_szID, Hero->ID) == 0)
    {
        float fWidth = 13;
        float fHeight = 11;
        float x = (float)m_iPos_x + m_iWidth - 22 - fWidth;
        float y = (float)iPos_y - 1;
        RenderBitmap(BITMAP_INVENTORY_BUTTON, x, y, fWidth, fHeight, 0.f, 0.f, 24.f / 32.f, 24.f / 32.f);
        if (MouseX >= x && MouseX < x + fWidth && MouseY >= y && MouseY < y + fHeight)
        {
            if (MouseLButton)
            {
                RenderBitmap(BITMAP_INVENTORY_BUTTON + 1, x, y, fWidth, fHeight, 0.f, 0.f, 24.f / 32.f, 24.f / 32.f);
            }

            g_pRenderText->SetTextColor(255, 255, 255, 255);
            g_pRenderText->SetBgColor(0, 0, 0, 255);

            if (wcscmp(m_TextListIter->m_szID, Hero->ID) == 0 && wcscmp(GuildList[0].Name, Hero->ID) == 0)
                RenderTipText((int)x - 20, (int)y, I18N::Game::Disband);
            else
                RenderTipText((int)x - 20, (int)y, I18N::Game::Leave);
        }
    }

    g_pRenderText->SetFont(g_hFont);
    DisableAlphaBlend();
    return TRUE;
}

BOOL CUIGuildListBox::DoSubMouseAction()
{
    m_TextListIter = m_TextList.begin();
    for (int i = 0; i < m_iCurrentRenderEndLine; ++i, ++m_TextListIter);

    for (int i = 0; i < m_iNumRenderLine; ++i)
    {
        if (m_TextListIter == m_TextList.end()) break;

        if (m_bIsGuildMaster == TRUE || wcscmp(m_TextListIter->m_szID, Hero->ID) == 0)
        {
            int iPos_y;
            if (GetLineNum() > m_iNumRenderLine) iPos_y = m_iPos_y - 16 - i * 13;
            else iPos_y = m_iPos_y - 16 - (i - GetLineNum() + m_iNumRenderLine) * 13;

            float fWidth = 12;
            float fHeight = 10;
            float x = (float)m_iPos_x + m_iWidth - 20 - fWidth;
            auto y = (float)iPos_y;
            if (MouseX >= x && MouseX < x + fWidth && MouseY >= y && MouseY < y + fHeight)
            {
                if (MouseLButtonPush)
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

void CUISimpleChatListBox::AddText(const wchar_t* pszID, const wchar_t* pszText, int iType, int iColor)
{
    if (pszID[0] == '\0' && pszText[0] == '\0')
    {
        return;
    }

    static WHISPER_TEXT text;
    text.m_bIsSelected = FALSE;
    wcsncpy(text.m_szID, pszID, MAX_USERNAME_SIZE + 1);
    //memcpy(text.m_szID, pszID, wcslen(pszID) + 1);
    text.m_iType = iType;
    text.m_iColor = iColor;
    text.m_uiEmptyLines = 0;

    wcsncpy(text.m_szText, pszText, MAX_TEXT_LENGTH + 1);
    //memcpy(text.m_szText, pszText, wcslen(pszText) + 1);
    m_TextList.push_front(text);
    CalcLineNum();
}

void CUISimpleChatListBox::Render()
{
    RenderInterface();
    MoveRenderLine();

    glColor3f(1.f, 1.f, 1.f);
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
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1, 13.0f, 13.0f, 13.0f / 16.0f, 29.0f / 32.0f, -13.0f / 16.0f, -13.0f / 32.0f);
        else
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1, 13.0f, 13.0f, 0.0f, 3.0f / 32.0f, 13.0f / 16.0f, 13.0f / 32.0f);

        if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - 12, 13.0f, 13.0f) == TRUE)
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12, 13.0f, 13.0f, 13.0f / 16.0f, 16.0f / 32.0f, -13.0f / 16.0f, -13.0f / 32.0f);
        else
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12, 13.0f, 13.0f, 0.0f, 16.0f / 32.0f, 13.0f / 16.0f, 13.0f / 32.0f);

        SetLineColor(2);
        RenderColor((float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 1, m_fScrollBarRange_top, (float)1, (float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
        RenderColor((float)m_iPos_x + m_iWidth, m_fScrollBarRange_top, (float)1, (float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
        EndRenderColor();

        if (GetLineNum() >= m_iNumRenderLine)
        {
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y, m_fScrollBarWidth - 2, m_fScrollBarHeight, 0.0f, 1.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y, m_fScrollBarWidth - 2, 1, 0.0f, 0.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y + m_fScrollBarHeight - 1, m_fScrollBarWidth - 2, 1, 0.0f, 2.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
        }
        else
        {
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top, m_fScrollBarWidth - 2, m_fScrollBarRange_bottom - m_fScrollBarRange_top, 0.0f, 1.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top, m_fScrollBarWidth - 2, 1, 0.0f, 0.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top + m_fScrollBarHeight - 1, m_fScrollBarWidth - 2, 1, 0.0f, 2.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
        }
    }
}

void CUISimpleChatListBox::CalcLineNum()
{
    if (m_dwParentUIID != 0)
    {
        CUIBaseWindow* pWindow = g_pWindowMgr->GetWindow(m_dwParentUIID);
        if (pWindow != nullptr)
        {
            switch (m_iResizeType)
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

void CUISimpleChatListBox::AddTextToRenderList(const wchar_t* pszID, const wchar_t* pszText, int iType, int iColor)
{
    if (pszID == nullptr || pszText == nullptr) return;

    static WHISPER_TEXT text;
    wcsncpy(text.m_szID, pszID, MAX_USERNAME_SIZE + 1);
    //memcpy(text.m_szID, pszID, wcslen(pszID) + 1);
    text.m_iType = iType;
    text.m_iColor = iColor;
    text.m_uiEmptyLines = 0;

    if (wcslen(pszID) + wcslen(pszText) >= 20)
    {
        SIZE nameSize;
        GetTextExtentPoint32(g_pRenderText->GetFontDC(), pszID, lstrlen(pszID), &nameSize);

        wchar_t Text1[10][MAX_TEXT_LENGTH + 1] = { {0},{0},{0},{0},{0} };
        int iLine = CutText3(pszText, Text1[0], m_iWidth - 30, 10, MAX_TEXT_LENGTH + 1, (nameSize.cx + 5) / g_fScreenRate_x);

        if (Text1[0][0] != '\0')
        {
            wcsncpy(text.m_szText, Text1[0], MAX_TEXT_LENGTH + 1);
            //memcpy(text.m_szText, Text1[0], wcslen(Text1[0]) + 2);
            m_RenderTextList.push_front(text);
        }

        for (int i = 1; i < iLine; ++i)
        {
            if (Text1[i][0] != '\0')
            {
                text.m_szID[0] = '\0';
                //memcpy(text.m_szID, L"", 1);
                wcsncpy(text.m_szText, Text1[i], MAX_TEXT_LENGTH + 1);
                //memcpy(text.m_szText, Text1[i], wcslen(Text1[i]) + 2);
                m_RenderTextList.push_front(text);
            }
        }
    }
    else
    {
        wcsncpy(text.m_szText, pszText, MAX_TEXT_LENGTH + 1);
        //memcpy(text.m_szText, pszText, wcslen(pszText) + 1);
        m_RenderTextList.push_front(text);
    }
}

BOOL CUISimpleChatListBox::RenderDataLine(int iLineNumber)
{
    wchar_t Text[MAX_TEXT_LENGTH + 1] = { 0 };

    SIZE TextSize = { 0, 0 };
    // 이름
    if (m_TextListIter->m_szID[0] != 0)
    {
        switch (m_TextListIter->m_iType)
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
        mu_swprintf(Text, L"%ls: ", m_TextListIter->m_szID);
        g_pRenderText->RenderText(m_iPos_x + 8, m_iPos_y - 16 - iLineNumber * 13, Text, 0, 0, RT3_SORT_LEFT, &TextSize);
        DisableAlphaBlend();
    }

    switch (m_TextListIter->m_iType)
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
    mu_swprintf(Text, L"%ls", m_TextListIter->m_szText);
    g_pRenderText->RenderText(m_iPos_x + 8 + TextSize.cx, m_iPos_y - 16 - iLineNumber * 13, Text);
    DisableAlphaBlend();

    return TRUE;
}

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
    GetTextExtentPoint32(g_pRenderText->GetFontDC(), L"ZZZZZZZZZZZZZ", lstrlen(L"ZZZZZZZZZZZZZ"), &TextSize);
    SetColumnWidth(0, TextSize.cx / g_fScreenRate_x + 8);
    GetTextExtentPoint32(g_pRenderText->GetFontDC(), I18N::Game::Server, wcslen(I18N::Game::Server), &TextSize);
    SetColumnWidth(1, TextSize.cx / g_fScreenRate_x + 8);

    m_bForceEditList = FALSE;
}

void CUIChatPalListBox::AddText(const wchar_t* pszID, BYTE Number, BYTE Server)
{
    if (wcscmp(pszID, L"") == 0)
        return;

    static GUILDLIST_TEXT text;
    text.m_bIsSelected = FALSE;
    wcsncpy(text.m_szID, pszID, MAX_USERNAME_SIZE);
    text.m_szID[MAX_USERNAME_SIZE] = '\0';
    //memcpy(text.m_szID, pszID, wcslen(pszID) + 1);
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

void CUIChatPalListBox::DeleteText(const wchar_t* pszID)
{
    if (pszID == nullptr || wcslen(pszID) == 0) return;
    for (m_TextListIter = m_TextList.begin(); m_TextListIter != m_TextList.end(); ++m_TextListIter)
    {
        if (wcsncmp(m_TextListIter->m_szID, pszID, MAX_USERNAME_SIZE) == 0)
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

const wchar_t* CUIChatPalListBox::GetNameByNumber(BYTE byNumber)
{
    if (byNumber == 255) return nullptr;
    for (m_TextListIter = m_TextList.begin(); m_TextListIter != m_TextList.end(); ++m_TextListIter)
    {
        if (m_TextListIter->m_Number == byNumber)
            return m_TextListIter->m_szID;
    }
    return nullptr;
}

void CUIChatPalListBox::MakeTitleText(wchar_t* pszTitleText)
{
    if (pszTitleText == nullptr || m_TextList.empty() == TRUE) return;
    int iNameNum = 0;
    std::deque<GUILDLIST_TEXT>::reverse_iterator riter;
    for (riter = m_TextList.rbegin(); riter != m_TextList.rend(); ++riter)
    {
        if (wcsncmp(riter->m_szID, Hero->ID, MAX_USERNAME_SIZE) != 0)
        {
            if (iNameNum > 0)
            {
                wcscat(pszTitleText, L", L");
            }

            wcsncat(pszTitleText, riter->m_szID, MAX_USERNAME_SIZE);
            ++iNameNum;

            if (iNameNum >= 3)
            {
                wcscat(pszTitleText, L"...");
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
        if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - m_iHeight - 1, 13.0f, 13.0f) == TRUE)
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1, 13.0f, 13.0f, 13.0f / 16.0f, 29.0f / 32.0f, -13.0f / 16.0f, -13.0f / 32.0f);
        else
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1, 13.0f, 13.0f, 0.0f, 3.0f / 32.0f, 13.0f / 16.0f, 13.0f / 32.0f);	// ▲

        if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - 12, 13.0f, 13.0f) == TRUE)
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12, 13.0f, 13.0f, 13.0f / 16.0f, 16.0f / 32.0f, -13.0f / 16.0f, -13.0f / 32.0f);
        else
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12, 13.0f, 13.0f, 0.0f, 16.0f / 32.0f, 13.0f / 16.0f, 13.0f / 32.0f);	// ▼

        SetLineColor(2);
        RenderColor((float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 1, m_fScrollBarRange_top, (float)1, (float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
        RenderColor((float)m_iPos_x + m_iWidth, m_fScrollBarRange_top, (float)1, (float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
        EndRenderColor();

        if (GetLineNum() >= m_iNumRenderLine)
        {
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y, m_fScrollBarWidth - 2, m_fScrollBarHeight, 0.0f, 1.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y, m_fScrollBarWidth - 2, 1, 0.0f, 0.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y + m_fScrollBarHeight - 1, m_fScrollBarWidth - 2, 1, 0.0f, 2.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
        }
        else
        {
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top, m_fScrollBarWidth - 2, m_fScrollBarRange_bottom - m_fScrollBarRange_top, 0.0f, 1.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top, m_fScrollBarWidth - 2, 1, 0.0f, 0.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top + m_fScrollBarHeight - 1, m_fScrollBarWidth - 2, 1, 0.0f, 2.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
        }
    }
}

int CUIChatPalListBox::GetRenderLinePos_y(int iLineNumber)
{
    if (GetLineNum() > m_iNumRenderLine)
        return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1) * 13 - iLineNumber * 13 + 3);
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

    wchar_t Text[MAX_TEXT_LENGTH + 1] = { 0 };
    mu_swprintf(Text, L"%ls", m_TextListIter->m_szID);
    g_pRenderText->RenderText(iPos_x + GetColumnPos_x(0), iPos_y, Text);

    if (m_iLayoutType == 1)
    {
        if (m_TextListIter->m_Server == 0xFF)
        {
            mu_swprintf(Text, I18N::Game::Offline1039);
        }
        else if (m_TextListIter->m_Server == 0xFE)
        {
            mu_swprintf(Text, I18N::Game::Offline1039);
        }
        else if (m_TextListIter->m_Server == 0xFD)
        {
            mu_swprintf(Text, I18N::Game::CannotUse);
        }
        else if (m_TextListIter->m_Server == 0xFC)
        {
            mu_swprintf(Text, I18N::Game::Offline1039);
        }
        //		else if (m_TextListIter->m_Server == 0xFB)
        //		{
        //			mu_swprintf(Text,I18N::Game::Waiting);
        //		}
        else
        {
            mu_swprintf(Text, I18N::Game::_2dServer, m_TextListIter->m_Server + 1);
        }
        g_pRenderText->RenderText(iPos_x + 4 + GetColumnPos_x(1), iPos_y, Text);
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

void CUIWindowListBox::AddText(DWORD dwUIID, const wchar_t* pszTitle, int iStatus)
{
    static WINDOWLIST_TEXT text;
    text.m_bIsSelected = FALSE;
    text.m_dwUIID = dwUIID;
    memset(text.m_szTitle, 0, sizeof(text.m_szTitle));
    wcsncpy(text.m_szTitle, pszTitle, 64);
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
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1, 13.0f, 13.0f, 13.0f / 16.0f, 29.0f / 32.0f, -13.0f / 16.0f, -13.0f / 32.0f);
        else
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1, 13.0f, 13.0f, 0.0f, 3.0f / 32.0f, 13.0f / 16.0f, 13.0f / 32.0f);

        if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - 12, 13.0f, 13.0f) == TRUE)
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12, 13.0f, 13.0f, 13.0f / 16.0f, 16.0f / 32.0f, -13.0f / 16.0f, -13.0f / 32.0f);
        else
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12, 13.0f, 13.0f, 0.0f, 16.0f / 32.0f, 13.0f / 16.0f, 13.0f / 32.0f);

        SetLineColor(2);
        RenderColor((float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 1, m_fScrollBarRange_top, (float)1, (float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
        RenderColor((float)m_iPos_x + m_iWidth, m_fScrollBarRange_top, (float)1, (float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
        EndRenderColor();

        if (GetLineNum() >= m_iNumRenderLine)
        {
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y, m_fScrollBarWidth - 2, m_fScrollBarHeight, 0.0f, 1.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y, m_fScrollBarWidth - 2, 1, 0.0f, 0.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y + m_fScrollBarHeight - 1, m_fScrollBarWidth - 2, 1, 0.0f, 2.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
        }
        else
        {
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top, m_fScrollBarWidth - 2, m_fScrollBarRange_bottom - m_fScrollBarRange_top, 0.0f, 1.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top, m_fScrollBarWidth - 2, 1, 0.0f, 0.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top + m_fScrollBarHeight - 1, m_fScrollBarWidth - 2, 1, 0.0f, 2.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
        }
    }
}

int CUIWindowListBox::GetRenderLinePos_y(int iLineNumber)
{
    if (GetLineNum() > m_iNumRenderLine)
        return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1) * 13 - iLineNumber * 13 + 3);
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

    wchar_t Text[MAX_TEXT_LENGTH + 1] = { 0 };
    mu_swprintf(Text, L"%ls", m_TextListIter->m_szTitle);
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
        RenderBitmap(BITMAP_INTERFACE_EX + 13, iPos_x + 2, iPos_y + 2, 5.0f, 5.0f, 0.f, 0.f, 5.f / 8.f, 5.f / 8.f);
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
    GetTextExtentPoint32(g_pRenderText->GetFontDC(), I18N::Game::Sender, wcslen(I18N::Game::Sender), &TextSize);
    SetColumnWidth(1, TextSize.cx / g_fScreenRate_x + 8);
    GetTextExtentPoint32(g_pRenderText->GetFontDC(), I18N::Game::DateRcvd, wcslen(I18N::Game::DateRcvd), &TextSize);
    SetColumnWidth(2, TextSize.cx / g_fScreenRate_x + 8);
    GetTextExtentPoint32(g_pRenderText->GetFontDC(), I18N::Game::Title1030, wcslen(I18N::Game::Title1030), &TextSize);
    SetColumnWidth(3, TextSize.cx / g_fScreenRate_x + 8);

    m_bForceEditList = FALSE;
}

DWORD g_dwLastLetterID = 0;

void CUILetterListBox::AddText(const wchar_t* pszID, const wchar_t* pszText, const wchar_t* pszDate, const wchar_t* pszTime, BOOL bIsRead)
{
    static LETTERLIST_TEXT text;
    text.m_bIsSelected = FALSE;
    wcsncpy(text.m_szID, pszID, MAX_USERNAME_SIZE + 1);
    wcsncpy(text.m_szText, pszText, MAX_TEXT_LENGTH + 1);
    wcsncpy(text.m_szDate, pszDate, 16);
    wcsncpy(text.m_szTime, pszTime, 16);
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
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1, 13.0f, 13.0f, 13.0f / 16.0f, 29.0f / 32.0f, -13.0f / 16.0f, -13.0f / 32.0f);
        else
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1, 13.0f, 13.0f, 0.0f, 3.0f / 32.0f, 13.0f / 16.0f, 13.0f / 32.0f);

        if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - 12, 13.0f, 13.0f) == TRUE)
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12, 13.0f, 13.0f, 13.0f / 16.0f, 16.0f / 32.0f, -13.0f / 16.0f, -13.0f / 32.0f);
        else
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12, 13.0f, 13.0f, 0.0f, 16.0f / 32.0f, 13.0f / 16.0f, 13.0f / 32.0f);

        SetLineColor(2);
        RenderColor((float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 1, m_fScrollBarRange_top, (float)1, (float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
        RenderColor((float)m_iPos_x + m_iWidth, m_fScrollBarRange_top, (float)1, (float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
        EndRenderColor();

        if (GetLineNum() >= m_iNumRenderLine)
        {
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y, m_fScrollBarWidth - 2, m_fScrollBarHeight, 0.0f, 1.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y, m_fScrollBarWidth - 2, 1, 0.0f, 0.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y + m_fScrollBarHeight - 1, m_fScrollBarWidth - 2, 1, 0.0f, 2.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
        }
        else
        {
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top, m_fScrollBarWidth - 2, m_fScrollBarRange_bottom - m_fScrollBarRange_top, 0.0f, 1.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top, m_fScrollBarWidth - 2, 1, 0.0f, 0.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top + m_fScrollBarHeight - 1, m_fScrollBarWidth - 2, 1, 0.0f, 2.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
        }
    }
}

int CUILetterListBox::GetRenderLinePos_y(int iLineNumber)
{
    if (GetLineNum() > m_iNumRenderLine)
        return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1) * 13 - iLineNumber * 13 + 3);
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

    wchar_t Text[MAX_TEXT_LENGTH + 1] = { 0 };
    RenderCheckBox(iPos_x + 1, iPos_y - 1, m_TextListIter->m_bIsSelected);
    if (m_TextListIter->m_bIsRead == TRUE)
        RenderBitmap(BITMAP_INTERFACE_EX + 14, iPos_x + 1 + 10, iPos_y - 2, 13.0f, 10.0f, 0.f, 9.f / 32.f, 13.f / 16.f, 10.f / 32.f);
    else
        RenderBitmap(BITMAP_INTERFACE_EX + 14, iPos_x + 1 + 10, iPos_y - 1, 13.0f, 9.0f, 0.f, 0.f, 13.f / 16.f, 9.f / 32.f);

    wcsncpy(Text, m_TextListIter->m_szID, MAX_TEXT_LENGTH);
    g_pRenderText->RenderText(iPos_x + 4 + GetColumnPos_x(1), iPos_y, Text, GetColumnWidth(1) - 4, 0, RT3_SORT_LEFT_CLIP);
    wcsncpy(Text, m_TextListIter->m_szDate, MAX_TEXT_LENGTH);
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
                m_TextListIter->m_bIsSelected = (m_TextListIter->m_bIsSelected + 1) % 2;
                LETTERLIST_TEXT* lt = g_pLetterList->GetLetter(m_TextListIter->m_dwLetterID);
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

CUILetterTextListBox::CUILetterTextListBox()
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
    m_iScrollType = UILISTBOX_SCROLL_UPDOWN;
}

void CUILetterTextListBox::AddText(const wchar_t* pszText)
{
    if (pszText == nullptr || pszText[0] == '\0')
    {
        return;
    }

    static LETTER_TEXT text;
    text.m_bIsSelected = FALSE;
    wcsncpy(text.m_szText, pszText, MAX_LETTERTEXT_LENGTH + 1);
    m_TextList.push_front(text);
    SLSetSelectLine(0);
    CalcLineNum();
}

void CUILetterTextListBox::Render()
{
    RenderInterface();
    MoveRenderLine();

    glColor3f(1.f, 1.f, 1.f);
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

void CUILetterTextListBox::RenderInterface()
{
    ComputeScrollBar();

    {
        if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - m_iHeight - 1, 13.0f, 13.0f) == TRUE)
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1, 13.0f, 13.0f, 13.0f / 16.0f, 29.0f / 32.0f, -13.0f / 16.0f, -13.0f / 32.0f);
        else
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1, 13.0f, 13.0f, 0.0f, 3.0f / 32.0f, 13.0f / 16.0f, 13.0f / 32.0f);

        if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - 12, 13.0f, 13.0f) == TRUE)
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12, 13.0f, 13.0f, 13.0f / 16.0f, 16.0f / 32.0f, -13.0f / 16.0f, -13.0f / 32.0f);
        else
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12, 13.0f, 13.0f, 0.0f, 16.0f / 32.0f, 13.0f / 16.0f, 13.0f / 32.0f);

        SetLineColor(2);
        RenderColor((float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 1, m_fScrollBarRange_top, (float)1, (float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
        RenderColor((float)m_iPos_x + m_iWidth, m_fScrollBarRange_top, (float)1, (float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
        EndRenderColor();

        if (GetLineNum() >= m_iNumRenderLine)
        {
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y, m_fScrollBarWidth - 2, m_fScrollBarHeight, 0.0f, 1.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y, m_fScrollBarWidth - 2, 1, 0.0f, 0.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y + m_fScrollBarHeight - 1, m_fScrollBarWidth - 2, 1, 0.0f, 2.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
        }
        else
        {
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top, m_fScrollBarWidth - 2, m_fScrollBarRange_bottom - m_fScrollBarRange_top, 0.0f, 1.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top, m_fScrollBarWidth - 2, 1, 0.0f, 0.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top + m_fScrollBarHeight - 1, m_fScrollBarWidth - 2, 1, 0.0f, 2.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
        }
    }
}

void CUILetterTextListBox::CalcLineNum()
{
    if (m_dwParentUIID != 0)
    {
        CUIBaseWindow* pWindow = g_pWindowMgr->GetWindow(m_dwParentUIID);
        if (pWindow != nullptr)
        {
            switch (m_iResizeType)
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
    std::deque<LETTER_TEXT>::reverse_iterator iter;
    for (iter = m_TextList.rbegin(); iter != m_TextList.rend(); ++iter)
    {
        AddTextToRenderList(iter->m_szText);
    }
}

void CUILetterTextListBox::AddTextToRenderList(const wchar_t* pszText)
{
    if (pszText == nullptr) return;

    static LETTER_TEXT text;
    if (wcslen(pszText) >= 20)
    {
        static wchar_t Text1[80][MAX_TEXT_LENGTH + 1];
        int iLine = CutText3(pszText, Text1[0], m_iWidth - 30, 80, MAX_TEXT_LENGTH + 1);

        for (int i = 0; i < iLine; ++i)
        {
            if (Text1[i][0] != '\0')
            {
                wcsncpy(text.m_szText, Text1[i], MAX_TEXT_LENGTH + 1);
                m_RenderTextList.push_front(text);
            }
        }
    }
    else
    {
        wcsncpy(text.m_szText, pszText, MAX_TEXT_LENGTH + 1);
        m_RenderTextList.push_front(text);
    }
}

int CUILetterTextListBox::GetRenderLinePos_y(int iLineNumber)
{
    if (GetLineNum() > m_iNumRenderLine)
        return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1) * 13 - iLineNumber * 13 + 3);
    else
        return (m_iPos_y - m_iHeight + (GetLineNum() - 1) * 13 - iLineNumber * 13 + 3);
}

BOOL CUILetterTextListBox::RenderDataLine(int iLineNumber)
{
    EnableAlphaTest();
    wchar_t Text[MAX_TEXT_LENGTH + 1] = { 0 };
    // 내용
    g_pRenderText->SetTextColor(230, 220, 200, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 0);

    mu_swprintf(Text, L"%ls", m_TextListIter->m_szText);
    int iPos_x = m_iPos_x + 10;
    int iPos_y = GetRenderLinePos_y(iLineNumber);
    g_pRenderText->RenderText(iPos_x, iPos_y, Text);

    DisableAlphaBlend();

    return TRUE;
}

CUISocketListBox::CUISocketListBox()
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
    m_bUseNewUIScrollBar = TRUE;
    m_iScrollType = UILISTBOX_SCROLL_UPDOWN;

    SetPosition(275, 360);
    SetSize(160, 65);
}

void CUISocketListBox::AddText(int iSocketIndex, const wchar_t* pszText)
{
    static SOCKETLIST_TEXT text;
    text.m_bIsSelected = FALSE;
    text.m_iSocketIndex = iSocketIndex;
    wcsncpy(text.m_szText, pszText, 64);

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
    EnableAlphaTest();
    SetLineColor(7, 0.4f);
    RenderColor(m_iPos_x - 1, m_iPos_y - m_iHeight - 1, m_iWidth + 1, m_iHeight + 2);
    EndRenderColor();

    if (GetState() != UISTATE_SCROLL)
        ComputeScrollBar();

    g_pGuardWindow->RenderScrollBarFrame(m_iPos_x + m_iWidth - 8, m_fScrollBarRange_top, m_fScrollBarRange_bottom - m_fScrollBarRange_top);
    g_pGuardWindow->RenderScrollBar(m_iPos_x + m_iWidth - 12, m_fScrollBarPos_y, (GetState() == UISTATE_SCROLL && MouseLButtonPush));
}

int CUISocketListBox::GetRenderLinePos_y(int iLineNumber)
{
    if (GetLineNum() > m_iNumRenderLine)
        return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1) * 13 - iLineNumber * 13 + 3);
    else
        return (m_iPos_y - m_iHeight + (GetLineNum() - 1) * 13 - iLineNumber * 13 + 3);
}

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

    wchar_t Text[MAX_TEXT_LENGTH + 1] = { 0 };
    mu_swprintf(Text, L"%ls", m_TextListIter->m_szText);
    g_pRenderText->RenderText(iPos_x, iPos_y, Text);

    DisableAlphaBlend();

    return TRUE;
}

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

CUIRenderText::CUIRenderText() = default;
CUIRenderText::~CUIRenderText() = default;

CUIRenderText* CUIRenderText::GetInstance()
{
    static CUIRenderText s_RenderText;
    return &s_RenderText;
}

bool CUIRenderText::Create(HDC hDC)
{
    if (m_pRenderText)
    {
        return true;
    }

    m_pRenderText = std::make_unique<CUIRenderTextOriginal>();
    if (!m_pRenderText->Create(hDC))
    {
        m_pRenderText.reset();
        return false;
    }
    return true;
}

void CUIRenderText::Release()
{
    m_pRenderText.reset();
}

HDC CUIRenderText::GetFontDC() const
{
    if (m_pRenderText)
        return m_pRenderText->GetFontDC();
    return nullptr;
}
BYTE* CUIRenderText::GetFontBuffer() const
{
    if (m_pRenderText)
        return m_pRenderText->GetFontBuffer();
    return nullptr;
}
DWORD CUIRenderText::GetTextColor() const
{
    if (m_pRenderText)
        return m_pRenderText->GetTextColor();
    return 0;
}
DWORD CUIRenderText::GetBgColor() const
{
    if (m_pRenderText)
        m_pRenderText->GetBgColor();
    return 0;
}

void CUIRenderText::SetTextColor(BYTE byRed, BYTE byGreen, BYTE byBlue, BYTE byAlpha)
{
    if (m_pRenderText)
        m_pRenderText->SetTextColor(byRed, byGreen, byBlue, byAlpha);
}
void CUIRenderText::SetTextColor(DWORD dwColor)
{
    if (m_pRenderText)
        m_pRenderText->SetTextColor(dwColor);
}
void CUIRenderText::SetBgColor(BYTE byRed, BYTE byGreen, BYTE byBlue, BYTE byAlpha)
{
    if (m_pRenderText)
        m_pRenderText->SetBgColor(byRed, byGreen, byBlue, byAlpha);
}
void CUIRenderText::SetBgColor(DWORD dwColor)
{
    if (m_pRenderText)
        m_pRenderText->SetBgColor(dwColor);
}

void CUIRenderText::SetFont(HFONT hFont)
{
    if (m_pRenderText)
        m_pRenderText->SetFont(hFont);
}

void CUIRenderText::RenderText(int iPos_x, int iPos_y, const wchar_t* pszText, int iBoxWidth /* = 0 */, int iBoxHeight /* = 0 */, int iSort /* = RT3_SORT_LEFT */, OUT SIZE* lpTextSize /* = NULL */)
{
    if (m_pRenderText)
    {
        m_pRenderText->RenderText(iPos_x, iPos_y, pszText, iBoxWidth, iBoxHeight, iSort, lpTextSize);
    }
}
CUIRenderTextOriginal::CUIRenderTextOriginal()
{
    m_hFontDC = nullptr;
    m_hBitmap = nullptr;
    m_pFontBuffer = nullptr;
    m_dwTextColor = m_dwBackColor = 0;
}
CUIRenderTextOriginal::~CUIRenderTextOriginal() { Release(); }

bool CUIRenderTextOriginal::Create(HDC hDC)
{
    BITMAPINFO* DIB_INFO;
    DIB_INFO = (BITMAPINFO*)new BYTE[sizeof(BITMAPINFOHEADER) + sizeof(PALETTEENTRY) * 256];
    memset(DIB_INFO, 0x00, sizeof(BITMAPINFOHEADER));
    DIB_INFO->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    DIB_INFO->bmiHeader.biWidth = REFERENCE_WIDTH * g_fScreenRate_x;		//. 640
    DIB_INFO->bmiHeader.biHeight = -(REFERENCE_HEIGHT * g_fScreenRate_y);		//. 480
    DIB_INFO->bmiHeader.biPlanes = 1;
    DIB_INFO->bmiHeader.biBitCount = 24;
    DIB_INFO->bmiHeader.biCompression = BI_RGB;

    m_hBitmap = CreateDIBSection(hDC, DIB_INFO, DIB_RGB_COLORS, (void**)&m_pFontBuffer, nullptr, 0);
    m_hFontDC = CreateCompatibleDC(hDC);
    SelectObject(m_hFontDC, m_hBitmap);
    SelectObject(m_hFontDC, g_hFont);
    m_dwBackColor = 0;				//. Default Background Color;
    m_dwTextColor = 0xFFFFFFFF;		//. Default Text Color

    delete[] DIB_INFO;

    if (nullptr == m_hFontDC || nullptr == m_hBitmap)
    {
        Release();
        return false;
    }
    return true;
}
void CUIRenderTextOriginal::Release()
{
    if (m_hFontDC != nullptr)
    {
        DeleteDC(m_hFontDC);
        m_hFontDC = nullptr;
        m_pFontBuffer = nullptr;
    }
    if (m_hBitmap != nullptr)
    {
        DeleteObject(m_hBitmap);
        m_hBitmap = nullptr;
    }
}

HDC CUIRenderTextOriginal::GetFontDC() const { return m_hFontDC; }
BYTE* CUIRenderTextOriginal::GetFontBuffer() const { return m_pFontBuffer; }

DWORD CUIRenderTextOriginal::GetTextColor() const { return m_dwTextColor; }
DWORD CUIRenderTextOriginal::GetBgColor() const { return m_dwBackColor; }

void CUIRenderTextOriginal::SetTextColor(BYTE byRed, BYTE byGreen, BYTE byBlue, BYTE byAlpha)
{
    m_dwTextColor = (byRed)+(byGreen << 8) + (byBlue << 16) + (byAlpha << 24);
}
void CUIRenderTextOriginal::SetTextColor(DWORD dwColor) { m_dwTextColor = dwColor; }
void CUIRenderTextOriginal::SetBgColor(BYTE byRed, BYTE byGreen, BYTE byBlue, BYTE byAlpha)
{
    m_dwBackColor = (byRed)+(byGreen << 8) + (byBlue << 16) + (byAlpha << 24);
}
void CUIRenderTextOriginal::SetBgColor(DWORD dwColor) { m_dwBackColor = dwColor; }

void CUIRenderTextOriginal::SetFont(HFONT hFont) { SelectObject(m_hFontDC, hFont); }

/// \brief Reads the Picture created by GDI and copies it to the texture bitmap.
void CUIRenderTextOriginal::WriteText(int iOffset, int iWidth, int iHeight)
{
    const int LIMIT_WIDTH = 256, LIMIT_HEIGHT = 32;

    SIZE FontDCSize = { (int)(REFERENCE_WIDTH * g_fScreenRate_x), (int)(REFERENCE_HEIGHT * g_fScreenRate_y) };
    int iPitch = ((FontDCSize.cx * 24 + 31) & ~31) >> 3;

    BITMAP_t* pBitmapFont = &Bitmaps[BITMAP_FONT];
    for (int y = 0; y < iHeight; ++y)
    {
        int SrcIndex = y * iPitch + iOffset;
        int DstIndex = y * LIMIT_WIDTH * 4;
        for (int x = 0; x < iWidth; ++x)
        {
            if ((SrcIndex > iPitch * FontDCSize.cy) || (DstIndex > LIMIT_WIDTH * 4 * LIMIT_HEIGHT))
            {
#ifdef _DEBUG
                __debugbreak();
#endif // _DEBUG
                return;
            }
            if (*(m_pFontBuffer + SrcIndex) == 255)	// we hit a white pixel, so here is Text
            {
                *reinterpret_cast<unsigned int*>(pBitmapFont->Buffer + DstIndex) = m_dwTextColor;
            }
            else if (*(m_pFontBuffer + SrcIndex) != 0) // we hit a semi transparent pixel, so anti aliasing hit here
            {
                // The alpha channel is the highest 8 bits.
                DWORD alpha = *(m_pFontBuffer + SrcIndex);
                alpha += *(m_pFontBuffer + SrcIndex + 1);
                alpha += *(m_pFontBuffer + SrcIndex + 2);
                alpha /= 3;
                alpha <<= 24;
                alpha |= 0x00FFFFFF;
                *reinterpret_cast<unsigned int*>(pBitmapFont->Buffer + DstIndex) = m_dwTextColor & alpha;
            }
            else // it's a black pixel, so there is no text
            {
                *reinterpret_cast<unsigned int*>(pBitmapFont->Buffer + DstIndex) = 0; // Transparent
            }

            SrcIndex += 3; // RBG
            DstIndex += 4; // RGBA
        }
    }
}

/// \brief Binds the previously created texture bitmap to the opengl texture.
void CUIRenderTextOriginal::UploadText(int sx, int sy, int Width, int Height)
{
    BITMAP_t* b = &Bitmaps[BITMAP_FONT];
    int uploadWidth = Width;
    int uploadHeight = Height;
    if (uploadWidth > static_cast<int>(b->Width))
    {
        uploadWidth = static_cast<int>(b->Width);
    }
    if (uploadHeight > static_cast<int>(b->Height))
    {
        uploadHeight = static_cast<int>(b->Height);
    }

    float TextureU = 0.f, TextureV = 0.f;
    if (sx < 0)
    {
        TextureU = (-sx + 0.01f) / b->Width;
        Width += sx;
        sx = 0.f;
    }
    else if (sx + Width > (int)WindowWidth)
    {
        Width = WindowWidth - sx;
    }
    if (sy < 0)
    {
        TextureV = (-sy + 0.01f) / b->Height;
        Height += sy;
        sy = 0.f;
    }
    else if (sy + Height > (int)WindowHeight)
    {
        Height = WindowHeight - sy;
    }
    if (Width > 0 && Height > 0 && sx + Width > 0 && sy + Height > 0)
    {
        glBindTexture(GL_TEXTURE_2D, b->TextureNumber);
        if (uploadWidth > 0 && uploadHeight > 0)
        {
            if (uploadWidth == static_cast<int>(b->Width))
            {
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, uploadWidth, uploadHeight, GL_RGBA, GL_UNSIGNED_BYTE, b->Buffer);
            }
            else
            {
                const size_t tightRowSize = static_cast<size_t>(uploadWidth) * 4;
                const size_t sourceRowSize = static_cast<size_t>(b->Width) * 4;
                m_tightUploadBuffer.resize(tightRowSize * uploadHeight);

                for (int row = 0; row < uploadHeight; ++row)
                {
                    memcpy(
                        m_tightUploadBuffer.data() + tightRowSize * row,
                        b->Buffer + sourceRowSize * row,
                        tightRowSize);
                }

                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, uploadWidth, uploadHeight, GL_RGBA, GL_UNSIGNED_BYTE, m_tightUploadBuffer.data());
            }
        }

        float TextureUWidth = (Width + 0.01f) / b->Width;
        float TextureVHeight = (Height + 0.01f) / b->Height;
        RenderBitmap(BITMAP_FONT, (float)sx, (float)sy, (float)Width, (float)Height,
            TextureU, TextureV, TextureUWidth, TextureVHeight, false, false);
    }
}

/// \brief Renders the text with GDI to the location of m_hFontDC/m_pFontBuffer as black/white picture. Text is white.
void CUIRenderTextOriginal::RenderText(int iPos_x, int iPos_y, const wchar_t* pszText,
                                       int iBoxWidth /* = 0 */, int iBoxHeight /* = 0 */,
                                       int iSort /* = RT3_SORT_LEFT */, OUT SIZE* lpTextSize /* = NULL */)
{
    if (pszText == nullptr || (pszText[0] == '\0' && iBoxWidth == 0)) return;
    if (wcslen(pszText) <= 0 && iBoxWidth == 0) return;

    SIZE RealTextSize;

    if (pszText[0] == '\0')
        GetTextExtentPoint32(m_hFontDC, L"0", 1, &RealTextSize);
    else
        GetTextExtentPoint32(m_hFontDC, pszText, lstrlen(pszText), &RealTextSize);

    MU_POINTF RealBoxPos = { (float)iPos_x * g_fScreenRate_x, (float)iPos_y * g_fScreenRate_y };
    SIZEF RealBoxSize = { (float)iBoxWidth * g_fScreenRate_x, (float)iBoxHeight * g_fScreenRate_y };
    SIZE RealRenderingSize = { RealTextSize.cx, RealTextSize.cy };

    if (RealBoxSize.cx == 0)
        RealBoxSize.cx = RealTextSize.cx;
    if (RealBoxSize.cy == 0)
        RealBoxSize.cy = RealTextSize.cy;

    int iTab = 0;
    int iClipMove = 0;

    if (iSort == RT3_SORT_LEFT_CLIP)
    {
        if (RealRenderingSize.cx > RealBoxSize.cx)
        {
            iClipMove = RealRenderingSize.cx - RealBoxSize.cx;
            RealRenderingSize.cx = RealBoxSize.cx;
        }
    }
    else if (iSort == RT3_SORT_LEFT)
    {
        if (RealRenderingSize.cx > RealBoxSize.cx)
            RealRenderingSize.cx = RealBoxSize.cx;
    }
    else if (iSort == RT3_SORT_CENTER)
    {
        if (RealRenderingSize.cx > RealBoxSize.cx)
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
        if (RealRenderingSize.cx > RealBoxSize.cx)
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
        if (RealRenderingSize.cx > RealBoxSize.cx)
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
        if (RealRenderingSize.cx > RealBoxSize.cx)
        {
            iClipMove = (RealRenderingSize.cx - RealBoxSize.cx) / 2;
            RealRenderingSize.cx = RealBoxSize.cx;
        }
        else
        {
            iTab = (RealBoxSize.cx - RealRenderingSize.cx) / 2;
        }
        RealBoxPos.x -= (RealBoxSize.cx / 2);
    }

    const int LIMIT_WIDTH = 256;

    if (m_dwBackColor != 0)
    {
        EnableAlphaTest();
        glColor4ub(GetRed(m_dwBackColor), GetGreen(m_dwBackColor),
            GetBlue(m_dwBackColor), GetAlpha(m_dwBackColor));
        RenderColor(RealBoxPos.x / g_fScreenRate_x, RealBoxPos.y / g_fScreenRate_y,
            RealBoxSize.cx / g_fScreenRate_x, RealBoxSize.cy / g_fScreenRate_y);
        EndRenderColor();
    }

    if (pszText[0] != 0x0a)
    {
        ::SetBkColor(m_hFontDC, RGB(0, 0, 0));
        ::SetTextColor(m_hFontDC, RGB(255, 255, 255));
        TextOut(m_hFontDC, 0, 0, pszText, lstrlen(pszText));
    }

    int iRealRenderWidth = RealRenderingSize.cx;
    int iNumberOfSections = (RealRenderingSize.cx / LIMIT_WIDTH) + ((iRealRenderWidth % LIMIT_WIDTH >= 0) ? 1 : 0);
    for (int i = 0; i < iNumberOfSections; i++)
    {
        SIZE RealSectionLine = { (long)LIMIT_WIDTH, (long)RealRenderingSize.cy };
        if (i == iNumberOfSections - 1)
            RealSectionLine.cx = iRealRenderWidth % LIMIT_WIDTH;

        WriteText(LIMIT_WIDTH * i * 3 + iClipMove, RealSectionLine.cx, RealSectionLine.cy);
        UploadText(RealBoxPos.x + LIMIT_WIDTH * i + iTab, RealBoxPos.y, RealSectionLine.cx, RealSectionLine.cy);
    }

    if (lpTextSize)
    {
        lpTextSize->cx = RealRenderingSize.cx / g_fScreenRate_x;
        lpTextSize->cy = RealRenderingSize.cy / g_fScreenRate_y;
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
        if (/*InputEnable == false && */g_bForceIMEConv == TRUE)
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
            g_bForceIMEConv = TRUE;
            ImmSetConversionStatus(hIMC, IME_CMODE_ALPHANUMERIC, IME_SMODE_NONE);
        }

        ImmReleaseContext(g_hWnd, hIMC);
        //		g_bForceIMEConv = FALSE;
    }
    if (iMode & IME_SENTENCEMODE)
    {
        if (/*InputEnable == false && */g_bForceIMESent == TRUE)
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

CUITextInputBox* CUITextInputBox::s_pFocusedPortable = nullptr;

CUITextInputBox::CUITextInputBox()
{
    m_hParentWnd = nullptr;

    m_dwTextColor = _ARGB(255, 255, 255, 255);
    m_dwBackColor = _ARGB(255, 0, 0, 0);
    m_dwSelectBackColor = _ARGB(255, 150, 150, 150);

    m_caretTimer.ResetTimer();

    m_bLock = FALSE;
    m_bPasswordInput = FALSE;

    SetPosition(193, 422);

    m_pTabTarget = nullptr;
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
    if (s_pFocusedPortable == this)
        s_pFocusedPortable = nullptr;
}

void CUITextInputBox::GetText(wchar_t* pszText, int iGetLength)
{
    if (pszText == nullptr || iGetLength <= 0) return;
    wcsncpy(pszText, m_portableText.c_str(), iGetLength - 1);
    pszText[iGetLength - 1] = L'\0';
}

void CUITextInputBox::SetText(const wchar_t* pszText)
{
    std::wstring wstrText = (pszText != nullptr) ? std::wstring(pszText) : std::wstring();
    if (wstrText.length() > MAX_TEXT_LENGTH) return;

    if (m_iMaxLength > 0 && static_cast<int>(wstrText.length()) > m_iMaxLength)
        wstrText.resize(m_iMaxLength);
    m_portableText = wstrText;
    m_iCaret = static_cast<int>(m_portableText.length());
    m_iSelAnchor = m_iCaret;
    m_iFirstVisible = 0;
}

void CUITextInputBox::SetTextLimit(int iLimit)
{
    m_iMaxLength = iLimit;
}

void CUITextInputBox::SetSize(int iWidth, int iHeight)
{
    if (iWidth == 0 || iHeight == 0) return;

    // The field renders through g_pRenderText, so size is the only state to keep.
    m_iWidth = iWidth;
    m_iHeight = iHeight;
}

void CUITextInputBox::Init(HWND hWnd, int iWidth, int iHeight, int iMaxLength, BOOL bIsPassword)
{
    m_hParentWnd = hWnd;
    m_bPasswordInput = bIsPassword;
    m_iMaxLength = iMaxLength;
    m_portableText.clear();
    m_iCaret = 0;
    m_iSelAnchor = 0;
    m_iFirstVisible = 0;
    SetSize(iWidth, iHeight);
    m_caretTimer.ResetTimer();
#ifdef PBG_ADD_INGAMESHOPMSGBOX
    m_bUseScrollbarRender = true;
#endif //PBG_ADD_INGAMESHOPMSGBOX
}

void CUITextInputBox::SetState(int iState)
{
    m_iState = iState;
    // A hidden box must not keep keyboard focus, or the SDL loop would keep
    // routing input to an invisible field.
    if (m_iState == UISTATE_HIDE && s_pFocusedPortable == this)
        s_pFocusedPortable = nullptr;
}

void CUITextInputBox::GiveFocus(BOOL SelectText)
{
    if (m_iState == UISTATE_HIDE || m_iState == UISTATE_DISABLE) return;

    s_pFocusedPortable = this;
    g_dwKeyFocusUIID = GetUIID();
    m_caretTimer.ResetTimer();

    const int iLength = static_cast<int>(m_portableText.length());
    if (SelectText == TRUE)
    {
        m_iSelAnchor = 0;
        m_iCaret = iLength;
    }
    else
    {
        m_iCaret = iLength;
        m_iSelAnchor = iLength;
    }
}

void CUITextInputBox::Render()
{
    if (m_iState == UISTATE_HIDE) return;
    RenderPortable();
}

void CUITextInputBox::RenderPortableScrollbar(int iTotalLines, int iVisibleLines)
{
    const float fLineNum = (iTotalLines > 0) ? static_cast<float>(iTotalLines) : 1.0f;
    const float fLineRate = static_cast<float>(iVisibleLines) / fLineNum;
    const float fPosRate = static_cast<float>(m_iScrollLine) / fLineNum;

    m_fScrollBarWidth = 13;
    m_fScrollBarRange_top = m_iPos_y + 9;
    m_fScrollBarRange_bottom = m_iPos_y + m_iHeight - 9;
    m_fScrollBarHeight = (m_fScrollBarRange_bottom - m_fScrollBarRange_top) * (fLineRate > 1 ? 1 : fLineRate);
    m_fScrollBarPos_y = m_fScrollBarRange_top
        + (m_fScrollBarRange_bottom - m_fScrollBarRange_top) * (fPosRate > 1 ? 1 : fPosRate);

    if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - m_fScrollBarWidth, m_iPos_y - 4, 13.0f, 13.0f) == TRUE)
        RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth, (float)m_iPos_y - 4, 13.0f, 13.0f, 13.0f / 16.0f, 29.0f / 32.0f, -13.0f / 16.0f, -13.0f / 32.0f);
    else
        RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth, (float)m_iPos_y - 4, 13.0f, 13.0f, 0.0f, 3.0f / 32.0f, 13.0f / 16.0f, 13.0f / 32.0f);	// up

    if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - m_fScrollBarWidth, m_iPos_y + m_iHeight - 9, 13.0f, 13.0f) == TRUE)
        RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth, (float)m_iPos_y + m_iHeight - 9, 13.0f, 13.0f, 13.0f / 16.0f, 16.0f / 32.0f, -13.0f / 16.0f, -13.0f / 32.0f);
    else
        RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth, (float)m_iPos_y + m_iHeight - 9, 13.0f, 13.0f, 0.0f, 16.0f / 32.0f, 13.0f / 16.0f, 13.0f / 32.0f);	// down

    EnableAlphaTest();
    SetLineColor(2);
    RenderColor((float)m_iPos_x + m_iWidth - m_fScrollBarWidth, m_fScrollBarRange_top, 1.0f, (float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
    RenderColor((float)m_iPos_x + m_iWidth - 1, m_fScrollBarRange_top, 1.0f, (float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
    EndRenderColor();
    DisableAlphaBlend();

    RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 1, m_fScrollBarPos_y, m_fScrollBarWidth - 1, m_fScrollBarHeight, 0.0f, 1.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
    RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 1, m_fScrollBarPos_y, m_fScrollBarWidth - 2, 1, 0.0f, 0.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
    RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 1, m_fScrollBarPos_y + m_fScrollBarHeight, m_fScrollBarWidth - 2, 1, 0.0f, 2.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
}

void CUITextInputBox::SetFont(HFONT hFont)
{
    if (hFont == nullptr)
        return;

    if (hFont == g_hFontBold)     m_fontKind = PortableFontKind::Bold;
    else if (hFont == g_hFontBig) m_fontKind = PortableFontKind::Big;
    else if (hFont == g_hFixFont) m_fontKind = PortableFontKind::Fix;
    else                          m_fontKind = PortableFontKind::Default;
}

BOOL CUITextInputBox::DoPortableMouse()
{
    g_pRenderText->SetFont(CurrentFont());
    const int iLineHeight = LineHeightPx();

    // Thumb drag in progress: track the mouse until the button is released.
    if (m_bUseMultiLine && GetState() == UISTATE_SCROLL)
    {
        if (MouseLButtonPush)
        {
            MouseOnWindow = true;
            std::vector<PortableLine> lines;
            LayoutLines(BuildDisplay(), lines);
            const float range = m_fScrollBarRange_bottom - m_fScrollBarRange_top;
            const float rel = (float)MouseY - m_fScrollBarClickPos_y - m_fScrollBarRange_top;
            int target = (range > 0.f) ? static_cast<int>((rel / range) * lines.size() + 0.5f) : 0;
            const int iMaxScroll = (static_cast<int>(lines.size()) > m_iNumLines) ? (static_cast<int>(lines.size()) - m_iNumLines) : 0;
            if (target < 0) target = 0;
            if (target > iMaxScroll) target = iMaxScroll;
            m_iScrollLine = target;
        }
        else
        {
            SetState(UISTATE_NORMAL);
            if (g_dwActiveUIID == GetUIID()) g_dwActiveUIID = 0;
        }
        return TRUE;
    }

    if (::CheckMouseIn(m_iPos_x, m_iPos_y - 4, m_iWidth, m_iHeight + 8) == FALSE)
        return FALSE;

    MouseOnWindow = true;
    if (GetState() != UISTATE_NORMAL)
        return TRUE;

    // Mouse wheel scrolls a multiline box.
    if (m_bUseMultiLine && MouseWheel != 0)
    {
        m_iScrollLine -= MouseWheel;
        if (m_iScrollLine < 0) m_iScrollLine = 0;
    }

    if (!MouseLButtonPush)
        return TRUE;

    // Scrollbar hit testing (multiline only; geometry set during render).
    if (m_bUseMultiLine)
    {
        if (::CheckMouseIn(m_iPos_x + m_iWidth - 15, m_iPos_y - 4, 13, 13))
        {
            if (m_iScrollLine > 0) --m_iScrollLine;
            return TRUE;
        }
        if (::CheckMouseIn(m_iPos_x + m_iWidth - 15, m_iPos_y + m_iHeight - 9, 13, 13))
        {
            ++m_iScrollLine;  // clamped on next render
            return TRUE;
        }
        if (::CheckMouseIn(m_iPos_x + m_iWidth - 14, m_fScrollBarPos_y, m_fScrollBarWidth, m_fScrollBarHeight))
        {
            if (g_dwActiveUIID == 0)
            {
                g_dwActiveUIID = GetUIID();
                SetState(UISTATE_SCROLL);
                m_fScrollBarClickPos_y = MouseY - m_fScrollBarPos_y;
            }
            return TRUE;
        }
    }

    // Click in the text area: focus and place the caret at the click.
    GiveFocus(FALSE);
    MouseUpdateTime = 0;
    MouseUpdateTimeMax = 6;

    const std::wstring display = BuildDisplay();
    const int targetX = MouseX - m_iPos_x;
    int idx;
    if (m_bUseMultiLine)
    {
        std::vector<PortableLine> lines;
        LayoutLines(display, lines);
        int li = (iLineHeight > 0) ? m_iScrollLine + (MouseY - m_iPos_y) / iLineHeight : 0;
        if (li < 0) li = 0;
        if (li >= static_cast<int>(lines.size())) li = static_cast<int>(lines.size()) - 1;
        idx = IndexAtLineX(display, lines[li], targetX);
    }
    else
    {
        // Single line scrolls horizontally; map against the visible window.
        const PortableLine line{ m_iFirstVisible, static_cast<int>(display.length()) };
        idx = IndexAtLineX(display, line, targetX);
    }
    m_iCaret = idx;
    m_iSelAnchor = idx;
    return TRUE;
}

BOOL CUITextInputBox::DoMouseAction()
{
    return DoPortableMouse();
}

namespace
{
    constexpr int CARET_BLINK_MS = 530;   // half-period of the caret blink
    constexpr int CARET_WIDTH_PX = 2;     // caret bar width in reference pixels
}

HFONT CUITextInputBox::CurrentFont() const
{
    switch (m_fontKind)
    {
    case PortableFontKind::Bold: return g_hFontBold;
    case PortableFontKind::Big:  return g_hFontBig;
    case PortableFontKind::Fix:  return g_hFixFont;
    default:                     return g_hFont;
    }
}

int CUITextInputBox::MeasureWidth(const wchar_t* pszText, int iLength) const
{
    if (pszText == nullptr || iLength <= 0) return 0;
    g_pRenderText->SetFont(CurrentFont());

    SIZE sz = { 0, 0 };
    GetTextExtentPoint32(g_pRenderText->GetFontDC(), pszText, iLength, &sz);
    return (g_fScreenRate_x > 0.f) ? static_cast<int>(sz.cx / g_fScreenRate_x) : sz.cx;
}

void CUITextInputBox::MoveCaret(int iNewCaret, bool bExtendSelection)
{
    const int iLength = static_cast<int>(m_portableText.length());
    if (iNewCaret < 0) iNewCaret = 0;
    if (iNewCaret > iLength) iNewCaret = iLength;

    m_iCaret = iNewCaret;
    if (!bExtendSelection)
        m_iSelAnchor = m_iCaret;

    m_caretTimer.ResetTimer();
}

void CUITextInputBox::InsertChar(wchar_t ch)
{
    if (HasSelection())
        DeleteSelection();

    if (m_iMaxLength > 0 && static_cast<int>(m_portableText.length()) >= m_iMaxLength)
        return;

    m_portableText.insert(m_portableText.begin() + m_iCaret, ch);
    ++m_iCaret;
    m_iSelAnchor = m_iCaret;
    m_caretTimer.ResetTimer();
}

void CUITextInputBox::DeleteSelection()
{
    if (!HasSelection()) return;

    const int iStart = SelectionStart();
    const int iEnd = SelectionEnd();
    m_portableText.erase(m_portableText.begin() + iStart, m_portableText.begin() + iEnd);
    m_iCaret = iStart;
    m_iSelAnchor = iStart;
    if (m_iFirstVisible > static_cast<int>(m_portableText.length()))
        m_iFirstVisible = 0;
}

void CUITextInputBox::SelectAll()
{
    m_iSelAnchor = 0;
    m_iCaret = static_cast<int>(m_portableText.length());
}

std::wstring CUITextInputBox::GetSelectedText() const
{
    if (m_iSelAnchor == m_iCaret) return std::wstring();
    const int iStart = (m_iSelAnchor < m_iCaret) ? m_iSelAnchor : m_iCaret;
    const int iEnd = (m_iSelAnchor < m_iCaret) ? m_iCaret : m_iSelAnchor;
    return m_portableText.substr(iStart, iEnd - iStart);
}

void CUITextInputBox::OnTextInput(const wchar_t* pszText)
{
    if (pszText == nullptr) return;

    for (const wchar_t* p = pszText; *p != L'\0'; ++p)
    {
        wchar_t ch = static_cast<wchar_t>(g_pMultiLanguage->ConvertFulltoHalfWidthChar(*p));
        if (ch < 0x20) continue;  // drop control characters

        if (CheckOption(UIOPTION_NUMBERONLY))
        {
            if (ch < L'0' || ch > L'9') continue;
        }
        else if (CheckOption(UIOPTION_SERIALNUMBER))
        {
            if (ch >= L'0' && ch <= L'9') {}
            else if (ch >= L'A' && ch <= L'Z') {}
            else if (ch >= L'a' && ch <= L'z') ch -= 32;  // force uppercase
            else continue;
        }
#ifdef LJH_ADD_RESTRICTION_ON_ID
        else if (CheckOption(UIOPTION_NOLOCALIZEDCHARACTERS))
        {
            if (ch < 33 || ch > 126) continue;
        }
#endif // LJH_ADD_RESTRICTION_ON_ID

        InsertChar(ch);
    }
}

void CUITextInputBox::OnEditKey(int iVirtualKey, bool bCtrl, bool bShift)
{
    m_caretTimer.ResetTimer();

    switch (iVirtualKey)
    {
    case VK_LEFT:
        MoveCaret(m_iCaret - 1, bShift);
        break;
    case VK_RIGHT:
        MoveCaret(m_iCaret + 1, bShift);
        break;
    case VK_HOME:
        if (m_bUseMultiLine)
        {
            std::vector<PortableLine> lines;
            LayoutLines(m_portableText, lines);
            MoveCaret(lines[CaretToLine(lines)].start, bShift);
        }
        else
        {
            MoveCaret(0, bShift);
        }
        break;
    case VK_END:
        if (m_bUseMultiLine)
        {
            std::vector<PortableLine> lines;
            LayoutLines(m_portableText, lines);
            MoveCaret(lines[CaretToLine(lines)].end, bShift);
        }
        else
        {
            MoveCaret(static_cast<int>(m_portableText.length()), bShift);
        }
        break;
    case VK_UP:
    case VK_DOWN:
        if (m_bUseMultiLine)
        {
            std::vector<PortableLine> lines;
            LayoutLines(m_portableText, lines);
            const int cur = CaretToLine(lines);
            const int target = cur + (iVirtualKey == VK_UP ? -1 : 1);
            if (target >= 0 && target < static_cast<int>(lines.size()))
            {
                const int caretX = MeasureWidth(m_portableText.c_str() + lines[cur].start, m_iCaret - lines[cur].start);
                MoveCaret(IndexAtLineX(m_portableText, lines[target], caretX), bShift);
            }
        }
        break;
    case VK_BACK:
        if (HasSelection())
            DeleteSelection();
        else if (m_iCaret > 0)
        {
            m_portableText.erase(m_portableText.begin() + m_iCaret - 1);
            --m_iCaret;
            m_iSelAnchor = m_iCaret;
        }
        break;
    case VK_DELETE:
        if (HasSelection())
            DeleteSelection();
        else if (m_iCaret < static_cast<int>(m_portableText.length()))
        {
            m_portableText.erase(m_portableText.begin() + m_iCaret);
            m_iSelAnchor = m_iCaret;
        }
        break;
    case VK_RETURN:
        if (IsLocked() == TRUE) break;
        // A multiline box inserts a hard line break; a single-line, unlocked box
        // notifies its owning UI window so Enter confirms (e.g. submits the
        // dialog). The login screen instead reads Enter from the global key
        // poll, so the parentless single-line case needs nothing here.
        if (UseMultiline() == TRUE)
        {
            InsertChar(L'\n');
            break;
        }
        if (g_pFriendMenu->IsHotkeyEnable() == TRUE) break;
        if (GetParentUIID() != 0)
            g_pWindowMgr->SendUIMessageToWindow(GetParentUIID(), UI_MESSAGE_TEXTINPUT, 0, 0);
        break;
    case VK_TAB:
        if (GetTabTarget() != nullptr && GetTabTarget()->GetState() == UISTATE_NORMAL)
            GetTabTarget()->GiveFocus(TRUE);
        break;
    default:
        break;
    }
}

int CUITextInputBox::LineHeightPx() const
{
    SIZE qSize = { 0, 0 };
    GetTextExtentPoint32(g_pRenderText->GetFontDC(), L"Q", 1, &qSize);
    int iLineHeight = (g_fScreenRate_y > 0.f) ? static_cast<int>(qSize.cy / g_fScreenRate_y) : qSize.cy;
    if (iLineHeight <= 0) iLineHeight = 1;
    return iLineHeight;
}

int CUITextInputBox::VisibleLineCount(int iLineHeight) const
{
    int n = (iLineHeight > 0) ? (m_iHeight / iLineHeight) : 1;
    return (n < 1) ? 1 : n;
}

// Break the text into displayed lines: a new line starts after every hard '\n'
// and wherever a paragraph exceeds the box width (wrapped at the last space, or
// mid-word when a single word is too long). Each span is [start, end) in buffer
// indices; end excludes the wrapped space or newline.
void CUITextInputBox::LayoutLines(const std::wstring& display, std::vector<PortableLine>& lines) const
{
    lines.clear();
    const int n = static_cast<int>(display.length());
    const wchar_t* p = display.c_str();

    int paraStart = 0;
    for (int i = 0; i <= n; ++i)
    {
        if (i != n && display[i] != L'\n') continue;

        // Wrap the paragraph [paraStart, i).
        int lineStart = paraStart;
        int lastSpace = -1;
        for (int j = paraStart; j < i; ++j)
        {
            if (display[j] == L' ') lastSpace = j;
            if (j > lineStart && MeasureWidth(p + lineStart, j + 1 - lineStart) > m_iWidth)
            {
                if (lastSpace > lineStart)
                {
                    lines.push_back({ lineStart, lastSpace });
                    lineStart = lastSpace + 1;
                }
                else
                {
                    lines.push_back({ lineStart, j });
                    lineStart = j;
                }
                lastSpace = -1;
            }
        }
        lines.push_back({ lineStart, i });

        paraStart = i + 1;
    }

    if (lines.empty())
        lines.push_back({ 0, 0 });
}

int CUITextInputBox::CaretToLine(const std::vector<PortableLine>& lines) const
{
    // The caret belongs to the last line whose start is <= caret.
    int result = 0;
    for (int i = 0; i < static_cast<int>(lines.size()); ++i)
    {
        if (lines[i].start <= m_iCaret) result = i;
        else break;
    }
    return result;
}

int CUITextInputBox::IndexAtLineX(const std::wstring& display, const PortableLine& line, int targetX) const
{
    int best = line.start;
    int bestDelta = targetX;  // distance from x=0 at line start
    if (bestDelta < 0) bestDelta = -bestDelta;

    const wchar_t* p = display.c_str() + line.start;
    for (int k = line.start + 1; k <= line.end; ++k)
    {
        int x = MeasureWidth(p, k - line.start);
        int delta = x - targetX;
        if (delta < 0) delta = -delta;
        if (delta < bestDelta)
        {
            bestDelta = delta;
            best = k;
        }
    }
    return best;
}

std::wstring CUITextInputBox::BuildDisplay() const
{
    if (m_bPasswordInput)
        return std::wstring(m_portableText.length(), L'*');
    return m_portableText;
}

void CUITextInputBox::RenderPortable()
{
    if (CurrentFont() == nullptr) return;
    g_pRenderText->SetFont(CurrentFont());

    const std::wstring display = BuildDisplay();

    const int iLength = static_cast<int>(display.length());
    if (m_iCaret > iLength) m_iCaret = iLength;
    if (m_iSelAnchor > iLength) m_iSelAnchor = iLength;

    int iLineHeight = LineHeightPx();
    if (!m_bUseMultiLine && iLineHeight > m_iHeight) iLineHeight = m_iHeight;

    // Background fill (shared by both layouts).
    if (CheckOption(UIOPTION_PAINTBACK))
    {
        EnableAlphaTest();
        glColor4f(0.f, 0.f, 0.f, 1.f);
        RenderColor(m_iPos_x, m_iPos_y, m_iWidth, m_iHeight);
        EndRenderColor();
    }
    else if (GetAlpha(m_dwBackColor) > 0)
    {
        EnableAlphaTest();
        glColor4ub(GetRed(m_dwBackColor), GetGreen(m_dwBackColor), GetBlue(m_dwBackColor), GetAlpha(m_dwBackColor));
        RenderColor(m_iPos_x, m_iPos_y, m_iWidth, m_iHeight);
        EndRenderColor();
    }

    if (m_bUseMultiLine)
        RenderPortableMultiline(display, iLineHeight);
    else
        RenderPortableSingleLine(display, iLineHeight);
}

void CUITextInputBox::RenderPortableSingleLine(const std::wstring& display, int iLineHeight)
{
    // Horizontal scroll: never start past the caret, and advance the first
    // visible character until the caret fits inside the box width.
    if (m_iFirstVisible > m_iCaret) m_iFirstVisible = m_iCaret;
    if (m_iFirstVisible < 0) m_iFirstVisible = 0;
    while (m_iFirstVisible < m_iCaret &&
        MeasureWidth(display.c_str() + m_iFirstVisible, m_iCaret - m_iFirstVisible) > m_iWidth)
    {
        ++m_iFirstVisible;
    }

    // Selection highlight, clamped to the visible window.
    if (HasSelection())
    {
        const int iSelStart = SelectionStart();
        const int iSelEnd = SelectionEnd();
        if (iSelEnd > m_iFirstVisible)
        {
            const int iVisStart = (iSelStart > m_iFirstVisible) ? iSelStart : m_iFirstVisible;
            int x0 = MeasureWidth(display.c_str() + m_iFirstVisible, iVisStart - m_iFirstVisible);
            int x1 = MeasureWidth(display.c_str() + m_iFirstVisible, iSelEnd - m_iFirstVisible);
            if (x0 < 0) x0 = 0;
            if (x1 > m_iWidth) x1 = m_iWidth;
            if (x1 > x0)
            {
                EnableAlphaTest();
                glColor4ub(GetRed(m_dwSelectBackColor), GetGreen(m_dwSelectBackColor), GetBlue(m_dwSelectBackColor), GetAlpha(m_dwSelectBackColor));
                RenderColor(m_iPos_x + x0, m_iPos_y, x1 - x0, iLineHeight);
                EndRenderColor();
            }
        }
    }

    // Visible text.
    const int iVisibleLen = static_cast<int>(display.length()) - m_iFirstVisible;
    if (iVisibleLen > 0)
    {
        g_pRenderText->SetBgColor(0);
        g_pRenderText->SetTextColor(m_dwTextColor);
        g_pRenderText->RenderText(m_iPos_x, m_iPos_y, display.c_str() + m_iFirstVisible, m_iWidth, m_iHeight, RT3_SORT_LEFT);
    }

    // Blinking caret (only while focused).
    const bool bFocused = (s_pFocusedPortable == this);
    const bool bBlinkOn = (static_cast<int>(m_caretTimer.GetTimeElapsed()) / CARET_BLINK_MS) % 2 == 0;
    if (bFocused && bBlinkOn)
    {
        int iCaretX = MeasureWidth(display.c_str() + m_iFirstVisible, m_iCaret - m_iFirstVisible);
        if (iCaretX > m_iWidth) iCaretX = m_iWidth;
        EnableAlphaTest();
        glColor4ub(GetRed(m_dwTextColor), GetGreen(m_dwTextColor), GetBlue(m_dwTextColor), 255);
        RenderColor(m_iPos_x + iCaretX, m_iPos_y, CARET_WIDTH_PX, iLineHeight);
        EndRenderColor();
    }
}

void CUITextInputBox::RenderPortableMultiline(const std::wstring& display, int iLineHeight)
{
    std::vector<PortableLine> lines;
    LayoutLines(display, lines);

    const int iTotalLines = static_cast<int>(lines.size());
    const int iVisibleLines = VisibleLineCount(iLineHeight);
    m_iNumLines = iVisibleLines;

    // Vertical scroll: keep the caret line within the visible window.
    const int iCaretLine = CaretToLine(lines);
    if (iCaretLine < m_iScrollLine) m_iScrollLine = iCaretLine;
    if (iCaretLine >= m_iScrollLine + iVisibleLines) m_iScrollLine = iCaretLine - iVisibleLines + 1;
    const int iMaxScroll = (iTotalLines > iVisibleLines) ? (iTotalLines - iVisibleLines) : 0;
    if (m_iScrollLine > iMaxScroll) m_iScrollLine = iMaxScroll;
    if (m_iScrollLine < 0) m_iScrollLine = 0;

    const bool bFocused = (s_pFocusedPortable == this);
    const bool bBlinkOn = (static_cast<int>(m_caretTimer.GetTimeElapsed()) / CARET_BLINK_MS) % 2 == 0;

    const int iSelStart = SelectionStart();
    const int iSelEnd = SelectionEnd();

    g_pRenderText->SetBgColor(0);

    const int iLast = m_iScrollLine + iVisibleLines;
    for (int li = m_iScrollLine; li < iLast && li < iTotalLines; ++li)
    {
        const PortableLine& line = lines[li];
        const int y = m_iPos_y + (li - m_iScrollLine) * iLineHeight;
        const wchar_t* lineText = display.c_str() + line.start;
        const int lineLen = line.end - line.start;

        // Selection highlight for the part of this line inside the selection.
        if (HasSelection() && iSelEnd > line.start && iSelStart <= line.end)
        {
            const int a = (iSelStart > line.start) ? iSelStart : line.start;
            const int b = (iSelEnd < line.end) ? iSelEnd : line.end;
            if (b >= a)
            {
                int x0 = MeasureWidth(lineText, a - line.start);
                int x1 = MeasureWidth(lineText, b - line.start);
                if (x1 > m_iWidth) x1 = m_iWidth;
                if (x1 > x0)
                {
                    EnableAlphaTest();
                    glColor4ub(GetRed(m_dwSelectBackColor), GetGreen(m_dwSelectBackColor), GetBlue(m_dwSelectBackColor), GetAlpha(m_dwSelectBackColor));
                    RenderColor(m_iPos_x + x0, y, x1 - x0, iLineHeight);
                    EndRenderColor();
                }
            }
        }

        if (lineLen > 0)
        {
            const std::wstring lineStr(lineText, lineLen);
            g_pRenderText->SetTextColor(m_dwTextColor);
            g_pRenderText->RenderText(m_iPos_x, y, lineStr.c_str(), m_iWidth, iLineHeight, RT3_SORT_LEFT);
        }

        // Caret on this line.
        if (bFocused && bBlinkOn && m_iCaret >= line.start && iCaretLine == li)
        {
            int iCaretX = MeasureWidth(lineText, m_iCaret - line.start);
            if (iCaretX > m_iWidth) iCaretX = m_iWidth;
            EnableAlphaTest();
            glColor4ub(GetRed(m_dwTextColor), GetGreen(m_dwTextColor), GetBlue(m_dwTextColor), 255);
            RenderColor(m_iPos_x + iCaretX, y, CARET_WIDTH_PX, iLineHeight);
            EndRenderColor();
        }
    }

    if (iTotalLines > iVisibleLines)
        RenderPortableScrollbar(iTotalLines, iVisibleLines);
}

void CUIChatInputBox::Init(HWND hWnd)
{
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

    HIMC hIMC = ImmGetContext(g_hWnd);
    ImmGetConversionStatus(hIMC, &g_dwBKConv, &g_dwBKSent);
    ImmReleaseContext(g_hWnd, hIMC);
}

void CUIChatInputBox::Reset()
{
    InputIndex = 0;
    TabMove(0);
    ClearTexts();
    RemoveHistory(TRUE);
}

void CUIChatInputBox::Render()
{
    m_TextInputBox.Render();
    m_BuddyInputBox.Render();
}

void CUIChatInputBox::TabMove(int iBoxNumber)
{
    if (GetState() == UISTATE_HIDE) return;
    if (iBoxNumber == 1)
        m_BuddyInputBox.GiveFocus(TRUE);
    else
        m_TextInputBox.GiveFocus(TRUE);
}

void CUIChatInputBox::GetTexts(wchar_t* pText, wchar_t* pBuddyText)
{
    m_TextInputBox.GetText(pText);
    m_BuddyInputBox.GetText(pBuddyText);
}

void CUIChatInputBox::ClearTexts()
{
    m_TextInputBox.SetText(nullptr);
    m_BuddyInputBox.SetText(nullptr);
}

void CUIChatInputBox::SetState(int iState)
{
    m_BuddyInputBox.SetState(iState);
    m_TextInputBox.SetState(iState);
    if (iState == UISTATE_NORMAL)
    {
        m_TextInputBox.GiveFocus();
    }
}

void CUIChatInputBox::SetFont(HFONT hFont)
{
    m_TextInputBox.SetFont(hFont);
    m_BuddyInputBox.SetFont(hFont);
}

void CUIChatInputBox::SetText(BOOL bSetText, const wchar_t* pText, BOOL bSetBuddyText, const wchar_t* pBuddyText)
{
    if (bSetText == TRUE) m_TextInputBox.SetText(pText);
    if (bSetBuddyText == TRUE) m_BuddyInputBox.SetText(pBuddyText);
}

BOOL CUIChatInputBox::DoMouseAction()
{
    if (m_TextInputBox.DoAction() == TRUE) InputIndex = 0;
    if (m_BuddyInputBox.DoAction() == TRUE) InputIndex = 1;
    return TRUE;
}

const int MAX_HISTORY_LINES = 10;

void CUIChatInputBox::AddHistory(const wchar_t* pszText)
{
    if (pszText == nullptr || pszText[0] == 0) return;

    wchar_t* pszSaveText = new wchar_t[wcslen(pszText) + 1];
    wcsncpy(pszSaveText, pszText, wcslen(pszText) + 1);
    m_HistoryList.push_front(pszSaveText);
    m_CurrentHistoryLine = m_HistoryList.begin();
    m_bHistoryMode = FALSE;

    RemoveHistory(FALSE);
}

void CUIChatInputBox::MoveHistory(int iDegree)
{
    if (m_HistoryList.empty() == TRUE) return;

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
                    SetText(TRUE, m_szTempText, FALSE, nullptr);
                    m_TextInputBox.m_caretTimer.ResetTimer();
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
    SetText(TRUE, *m_CurrentHistoryLine, FALSE, nullptr);
    m_TextInputBox.m_caretTimer.ResetTimer();
}

void CUIChatInputBox::RemoveHistory(BOOL bClear)
{
    if (bClear == TRUE)
    {
        for (m_HistoryListIter = m_HistoryList.begin(); m_HistoryListIter != m_HistoryList.end(); ++m_HistoryListIter)
        {
            if (*m_HistoryListIter != nullptr)
            {
                delete[] * m_HistoryListIter;
                *m_HistoryListIter = nullptr;
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
            if (*m_HistoryListIter != nullptr)
            {
                delete[] * m_HistoryListIter;
                *m_HistoryListIter = nullptr;
            }
        }
        int nDelCount = m_HistoryList.size() - MAX_HISTORY_LINES;

        for (int i = 0; i < nDelCount; ++i)
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
    m_hFont = nullptr;
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

    m_fMovePosition = (float)REFERENCE_WIDTH;
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
    return (m_pszSlideText[0] == '\0' || (int)wcslen(m_pszSlideText) < m_iCutLength);
}

int g_iNoticeInverse = 0;

void CUISlideHelp::Render(BOOL bForceFadeOut)
{
    if (g_pOption->IsSlideHelp() == false)
    {
        return;
    }

    BOOL bFadeOut = FALSE;

    if (m_pszSlideText[0] == '\0' || (int)wcslen(m_pszSlideText) < m_iCutLength)
    {
        bFadeOut = TRUE;
    }
    else if (bForceFadeOut == TRUE)
    {
        bFadeOut = TRUE;
    }

    if (bFadeOut == FALSE)
    {
        m_iAlphaRate += 30.f * FPS_ANIMATION_FACTOR;
        m_iAlphaRate = std::min<float>(m_iAlphaRate, 205.f);
    }
    else
    {
        m_iAlphaRate -= 30 * FPS_ANIMATION_FACTOR;
        m_iAlphaRate = std::max<float>(m_iAlphaRate, 0.f);
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
        SIZE TextSize = { 0, 0 };
        GetTextExtentPoint32(g_pRenderText->GetFontDC(), L"Z", 1, &TextSize);
        m_iFontHeight = TextSize.cy /= g_fScreenRate_y;

        if (GetPosition_y() >= m_iFontHeight)
        {
            SetPosition(GetPosition_x(), GetPosition_y() - m_iFontHeight);
        }
    }

    g_pRenderText->SetTextColor(m_dwSlideTextColor & 0x00FFFFFF);

    BYTE byAlpha = m_dwSlideTextColor >> 24;
    byAlpha = static_cast<float>(byAlpha) * ((m_iAlphaRate > 180 ? m_iAlphaRate : (m_iAlphaRate - 25 < 0 ? 0 : m_iAlphaRate - 25)) + 50) / 255.0f;
    if (const auto frac = WorldTime - static_cast<long>(WorldTime);
        m_bBlink == TRUE && frac < 0.5)
    {
        byAlpha /= 2;
    }

    g_pRenderText->SetTextColor(g_pRenderText->GetTextColor() + (byAlpha << 24));
    g_pRenderText->SetBgColor(0);
    g_pRenderText->RenderText(static_cast<int>(m_fMovePosition), m_iPos_y, m_pszSlideText);
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
        memset(m_pszSlideText, 0, SLIDE_TEXT_LENGTH);
        m_fMovePosition = (float)REFERENCE_WIDTH;
        m_iCutSize = CheckCutSize(m_pszSlideText, 4);
        m_iCutSize /= g_fScreenRate_x;
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

    m_fMoveSpeed += m_fMoveAccel * FPS_ANIMATION_FACTOR;
    if (m_fMoveSpeed >= m_fMaxMoveSpeed)
    {
        m_fMoveSpeed = m_fMaxMoveSpeed;
    }
    else if (m_fMoveSpeed < 0)
    {
        m_fMoveSpeed = 0;
    }

    m_fMovePosition -= m_fMoveSpeed * FPS_ANIMATION_FACTOR;
}

BOOL CUISlideHelp::AddSlideText(const wchar_t* pszNewText, DWORD dwTextColor)
{
    if (pszNewText == nullptr || pszNewText[0] == '\0') return TRUE;

    m_dwSlideTextColor = dwTextColor;
    g_pRenderText->SetFont(m_hFont);
    wcscat(m_pszSlideText, pszNewText);
    return TRUE;
}

int CUISlideHelp::CheckCutSize(const wchar_t* pszSource, int iNeedValue)
{
    if (pszSource == nullptr || pszSource[0] == L'\0') return 0;

    auto iLength = wcslen(pszSource);
    int iMove = 2; // might be 4, too
    int iTextSize = 0;
    for (int i = 0; i < iLength; )
    {
        iTextSize = i;
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

    SIZE TextSize = { 0, 0 };
    GetTextExtentPoint32(g_pRenderText->GetFontDC(), pszSource, m_iCutLength, &TextSize);
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

void CUISlideHelp::AddSlide(int iLoopCount, int iLoopDelay, const wchar_t* pszText, int iType, float fSpeed, DWORD dwTextColor)
{
    if (SceneFlag != MAIN_SCENE) return;
    if (pszText == nullptr || pszText[0] == '\0') return;
    if (iLoopCount > 30) return;
    int iLength = wcslen(pszText);

    SLIDE_QUEUE_DATA slidedata;
    slidedata.m_iType = iType;
    slidedata.m_pszText = new wchar_t[iLength + 1];
    memset(slidedata.m_pszText, 0, iLength + 1);
    wcsncpy(slidedata.m_pszText, pszText, iLength + 1);
    slidedata.m_fSpeed = fSpeed;
    slidedata.m_dwTextColor = dwTextColor;
    slidedata.m_bLastData = FALSE;

    for (int i = 0; i < iLoopCount; ++i)
    {
        if (i + 1 == iLoopCount)
        {
            slidedata.m_bLastData = TRUE;
        }

        if (iType == 1)
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

    if (m_pszSlideText[0] == '\0' || (int)wcslen(m_pszSlideText) < m_iCutLength)
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
            if (m_SlideQueueIter->second.m_iType == -1 && m_SlideQueueIter->first + 60 < m_dwCurrentSecond);
            else if (AddSlideText(m_SlideQueueIter->second.m_pszText, m_SlideQueueIter->second.m_dwTextColor) == FALSE) break;

            SetScrollSpeed(m_SlideQueueIter->second.m_fSpeed);
            if (m_SlideQueueIter->second.m_bLastData == TRUE)
            {
                delete[] m_SlideQueueIter->second.m_pszText;
                m_SlideQueueIter->second.m_pszText = nullptr;
            }
            m_SlideQueue.erase(m_SlideQueueIter);
            break;
        }
    }
}

CSlideHelpMgr::CSlideHelpMgr()
{
    m_iCreateDelay = 10;
    m_fHelpSlideSpeed = 2.5f;
}

CSlideHelpMgr::~CSlideHelpMgr()
{
    // The slide-help timer's callback captures this; kill it so it cannot fire
    // on a destroyed instance.
    Core::Time::FrameTimerScheduler::Instance().Kill(SLIDEHELP_TIMER);
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

    extern bool g_bWndActive;
    Core::Time::FrameTimerScheduler::Instance().SetRepeating(
        SLIDEHELP_TIMER, m_iCreateDelay * 1000,
        [this] { if (g_bWndActive) CreateSlideText(); });
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
    if (g_pOption->IsSlideHelp() == false)
    {
        return;
    }
    if (m_HelpSlide.GetAlphaRate() > 0)
        return;

    int iLevel = CharacterMachine->Character.Level;

    const wchar_t* pszNewText = GetSlideText(iLevel);

    AddSlide(1, 0, pszNewText, 1, m_fHelpSlideSpeed);
}

void CSlideHelpMgr::OpenSlideTextFile(const wchar_t* szFileName)
{
    for (int i = 0; i < SLIDE_LEVEL_MAX; ++i)
    {
        if (!m_SlideTextList[i].empty())
        {
            ClearSlideText();
            break;
        }
    }

    FILE* fp = _wfopen(szFileName, L"rb");
    if (fp == nullptr)
    {
        wchar_t Text[256];
        mu_swprintf(Text, L"%ls - File not exist.", szFileName);
        g_ErrorReport.Write(Text);
        MessageBox(g_hWnd, Text, nullptr, MB_OK);
        SendMessage(g_hWnd, WM_DESTROY, 0, 0);
        return;
    }

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
            auto charText = SlideHelp.SlideHelp[i].szSlideHelpText[j];
            int iLength = MultiByteToWideChar(CP_UTF8, 0, charText, -1, 0, 0);
            auto pszText = new wchar_t[iLength + 1];
            MultiByteToWideChar(CP_UTF8, 0, charText, -1, pszText, iLength);
            m_SlideTextList[i].push_back(pszText);
        }
    }
}

void CSlideHelpMgr::ClearSlideText()
{
    for (int i = 0; i < SLIDE_LEVEL_MAX; ++i)
    {
        m_iLevelCap[i] = 0;
        m_iTextNumber[i] = 0;
        for (m_SlideTextListIter = m_SlideTextList[i].begin(); m_SlideTextListIter != m_SlideTextList[i].end(); ++m_SlideTextListIter)
        {
            if (*m_SlideTextListIter != nullptr)
            {
                delete[] * m_SlideTextListIter;
                *m_SlideTextListIter = nullptr;
            }
        }
        m_SlideTextList[i].clear();
    }
}

const wchar_t* CSlideHelpMgr::GetSlideText(int iLevel)
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
    if (iHelpType == -1) return nullptr;
    if (m_iTextNumber[iHelpType] == 0) return nullptr;

    int iRandom = rand() % m_iTextNumber[iHelpType];

    if ((unsigned int)iRandom >= m_SlideTextList[iHelpType].size()) return nullptr;

    m_SlideTextListIter = m_SlideTextList[iHelpType].begin();
    for (int i = 0; i < iRandom; ++i)
        ++m_SlideTextListIter;

    return *m_SlideTextListIter;
}

void CSlideHelpMgr::AddSlide(int iLoopCount, int iLoopDelay, const wchar_t* pszText, int iType, float fSpeed, DWORD dwTextColor)
{
    if (SceneFlag != MAIN_SCENE) return;
    if (pszText == nullptr || pszText[0] == '\0') return;
    if (iLoopCount > 30) return;

    switch (iType)
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

void CUIGuildNoticeListBox::AddText(const wchar_t* szContent)
{
    if (szContent == nullptr || szContent[0] == '\0') return;

    static GUILDLOG_TEXT text;
    wcscpy(text.m_szContent, szContent);
    m_TextList.push_front(text);

    RemoveText();
    SLSetSelectLine(0);
    if (GetLineNum() > m_iNumRenderLine) ++m_iCurrentRenderEndLine;

    if (GetLineNum() < m_iNumRenderLine);
    else if (GetLineNum() - m_iCurrentRenderEndLine < m_iNumRenderLine)
        m_iCurrentRenderEndLine = GetLineNum() - m_iNumRenderLine;

    if (m_TextList.size() == 1)
        SLSetSelectLine(1);

    if (m_TextList.empty() == FALSE)
    {
        SLSetSelectLine(GetLineNum());
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
    return;
    EnableAlphaTest();
    SetLineColor(7, 0.4f);
    RenderColor(m_iPos_x - 1, m_iPos_y - m_iHeight - 1, m_iWidth + 1, m_iHeight + 2);
    EndRenderColor();
    ComputeScrollBar();

    //if (GetLineNum() >= m_iNumRenderLine)
    {
        if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - m_iHeight - 1, 13.0f, 13.0f) == TRUE)
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1, 13.0f, 13.0f, 13.0f / 16.0f, 29.0f / 32.0f, -13.0f / 16.0f, -13.0f / 32.0f);
        else
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1, 13.0f, 13.0f, 0.0f, 3.0f / 32.0f, 13.0f / 16.0f, 13.0f / 32.0f);	// ▲

        if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - 12, 13.0f, 13.0f) == TRUE)
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12, 13.0f, 13.0f, 13.0f / 16.0f, 16.0f / 32.0f, -13.0f / 16.0f, -13.0f / 32.0f);
        else
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12, 13.0f, 13.0f, 0.0f, 16.0f / 32.0f, 13.0f / 16.0f, 13.0f / 32.0f);	// ▼

        EnableAlphaTest();
        SetLineColor(2);
        RenderColor((float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 1, m_fScrollBarRange_top, (float)1, (float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
        RenderColor((float)m_iPos_x + m_iWidth, m_fScrollBarRange_top, (float)1, (float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
        EndRenderColor();

        if (GetLineNum() >= m_iNumRenderLine)
        {
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y, m_fScrollBarWidth - 2, m_fScrollBarHeight, 0.0f, 1.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y, m_fScrollBarWidth - 2, 1, 0.0f, 0.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y + m_fScrollBarHeight - 1, m_fScrollBarWidth - 2, 1, 0.0f, 2.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
        }
        else
        {
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top, m_fScrollBarWidth - 2, m_fScrollBarRange_bottom - m_fScrollBarRange_top, 0.0f, 1.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top, m_fScrollBarWidth - 2, 1, 0.0f, 0.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top + m_fScrollBarHeight - 1, m_fScrollBarWidth - 2, 1, 0.0f, 2.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
        }
    }
}

int CUIGuildNoticeListBox::GetRenderLinePos_y(int iLineNumber)
{
    if (GetLineNum() > m_iNumRenderLine)
        return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1) * 13 - iLineNumber * 13 + 3);
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

    g_pRenderText->RenderText(iPos_x, iPos_y, m_TextListIter->m_szContent, 0, 0, RT3_SORT_LEFT);

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

void CUIGuildNoticeListBox::DeleteText(DWORD dwGuildIndex)
{
}

CUINewGuildMemberListBox::CUINewGuildMemberListBox()
{
    m_iMaxLineCount = UIMAX_TEXT_LINE;
    m_iCurrentRenderEndLine = 0;
    m_iNumRenderLine = 18;

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

void CUINewGuildMemberListBox::AddText(const wchar_t* pszID, BYTE Number, BYTE Server, BYTE GuildStatus)
{
    if (pszID == nullptr || pszID[0] == '\0') return;

    if (GetLineNum() == 0)
    {
        if (wcscmp(pszID, Hero->ID) == 0) m_bIsGuildMaster = TRUE;
        else m_bIsGuildMaster = FALSE;
    }

    static GUILDLIST_TEXT text;
    text.m_bIsSelected = FALSE;
    wcsncpy(text.m_szID, pszID, MAX_USERNAME_SIZE + 1);
    //memcpy(text.m_szID, pszID, wcslen(pszID) + 1);
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
    return;
    EnableAlphaTest();
    SetLineColor(7, 0.4f);
    RenderColor(m_iPos_x - 1, m_iPos_y - m_iHeight - 1, m_iWidth + 1, m_iHeight + 2);
    EndRenderColor();
    ComputeScrollBar();

    //if (GetLineNum() >= m_iNumRenderLine)
    {
        if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - m_iHeight - 1, 13.0f, 13.0f) == TRUE)
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1, 13.0f, 13.0f, 13.0f / 16.0f, 29.0f / 32.0f, -13.0f / 16.0f, -13.0f / 32.0f);
        else
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1, 13.0f, 13.0f, 0.0f, 3.0f / 32.0f, 13.0f / 16.0f, 13.0f / 32.0f);	// ▲

        if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - 12, 13.0f, 13.0f) == TRUE)
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12, 13.0f, 13.0f, 13.0f / 16.0f, 16.0f / 32.0f, -13.0f / 16.0f, -13.0f / 32.0f);
        else
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12, 13.0f, 13.0f, 0.0f, 16.0f / 32.0f, 13.0f / 16.0f, 13.0f / 32.0f);	// ▼

        EnableAlphaTest();
        SetLineColor(2);
        RenderColor((float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 1, m_fScrollBarRange_top, (float)1, (float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
        RenderColor((float)m_iPos_x + m_iWidth, m_fScrollBarRange_top, (float)1, (float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
        EndRenderColor();

        if (GetLineNum() >= m_iNumRenderLine)
        {
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y, m_fScrollBarWidth - 2, m_fScrollBarHeight, 0.0f, 1.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y, m_fScrollBarWidth - 2, 1, 0.0f, 0.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y + m_fScrollBarHeight - 1, m_fScrollBarWidth - 2, 1, 0.0f, 2.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
        }
        else
        {
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top, m_fScrollBarWidth - 2, m_fScrollBarRange_bottom - m_fScrollBarRange_top, 0.0f, 1.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top, m_fScrollBarWidth - 2, 1, 0.0f, 0.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top + m_fScrollBarHeight - 1, m_fScrollBarWidth - 2, 1, 0.0f, 2.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
        }
    }

    g_pRenderText->RenderText(m_iPos_x + 14, m_iPos_y - m_iHeight - 12, I18N::Game::Name);
    g_pRenderText->RenderText(m_iPos_x + 65, m_iPos_y - m_iHeight - 12, I18N::Game::Position);
    g_pRenderText->RenderText(m_iPos_x + 106, m_iPos_y - m_iHeight - 12, I18N::Game::Server);
}

int CUINewGuildMemberListBox::GetRenderLinePos_y(int iLineNumber)
{
    if (GetLineNum() > m_iNumRenderLine)
        return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1) * 13 - iLineNumber * 13 + 3);
    else
        return (m_iPos_y - m_iHeight + (GetLineNum() - 1) * 13 - iLineNumber * 13 + 3);
}

BOOL CUINewGuildMemberListBox::RenderDataLine(int iLineNumber)
{
    EnableAlphaTest();

    int iCharacterLevel;
    if (m_TextListIter->m_GuildStatus == 128)
        iCharacterLevel = 0;
    else if (m_TextListIter->m_GuildStatus == 64)
        iCharacterLevel = 1;
    else if (m_TextListIter->m_GuildStatus == 32)
        iCharacterLevel = 2;
    else
        iCharacterLevel = 3;

    if (iCharacterLevel == 0)
    {
        glColor4ub(255, 100, 50, 127);
        RenderColor(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13);
        glColor4f(255, 255, 255, 255);
    }
    else if (iCharacterLevel == 1)
    {
        glColor4ub(255, 150, 80, 127);
        RenderColor(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13);
        glColor4f(255, 255, 255, 255);
    }
    else if (iCharacterLevel == 2)
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

    wchar_t Text[MAX_TEXT_LENGTH + 1] = { 0 };
    mu_swprintf(Text, L"%ls", m_TextListIter->m_szID);
    g_pRenderText->RenderText(iPos_x, iPos_y, Text);

    if (iCharacterLevel == 0) g_pRenderText->RenderText(iPos_x + 45, iPos_y, I18N::Game::Master, 70, 0, RT3_SORT_CENTER);
    else if (iCharacterLevel == 1) g_pRenderText->RenderText(iPos_x + 45, iPos_y, I18N::Game::AssistM, 70, 0, RT3_SORT_CENTER);
    else if (iCharacterLevel == 2) g_pRenderText->RenderText(iPos_x + 45, iPos_y, I18N::Game::BattleM, 70, 0, RT3_SORT_CENTER);

    if (m_TextListIter->m_Server != 255/* && m_TextListIter->m_Number != 0*/)
    {
        g_pRenderText->SetBgColor(0);
        g_pRenderText->SetTextColor(255, 196, 0, 255);

        mu_swprintf(Text, L"%d", m_TextListIter->m_Server + 1);
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

void CUINewGuildMemberListBox::DeleteText(DWORD dwUIID)
{
}

CUIUnionGuildListBox::CUIUnionGuildListBox()
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

    SetPosition(466, 360);
    SetSize(157, 235);
}

void CUIUnionGuildListBox::AddText(BYTE* pGuildMark, const wchar_t* szGuildName, int nMemberCount)
{
    if (szGuildName == nullptr || szGuildName[0] == '\0') return;

    static UNIONGUILD_TEXT text;
    text.m_bIsSelected = FALSE;
    memcpy(text.GuildMark, pGuildMark, sizeof(BYTE) * 64);
    wcsncpy(text.szName, szGuildName, MAX_GUILDNAME);
    text.szName[MAX_GUILDNAME] = 0;
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
    return;

    EnableAlphaTest();
    SetLineColor(7, 0.4f);
    RenderColor(m_iPos_x - 1, m_iPos_y - m_iHeight - 1, m_iWidth + 1, m_iHeight + 2);

    EndRenderColor();

    ComputeScrollBar();

    //if (GetLineNum() >= m_iNumRenderLine)
    {
        if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - m_iHeight - 1, 13.0f, 13.0f) == TRUE)
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1,
                13.0f, 13.0f, 13.0f / 16.0f, 29.0f / 32.0f, -13.0f / 16.0f, -13.0f / 32.0f);
        else
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1,
                13.0f, 13.0f, 0.0f, 3.0f / 32.0f, 13.0f / 16.0f, 13.0f / 32.0f);	// ▲

        if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - 12, 13.0f, 13.0f) == TRUE)
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12,
                13.0f, 13.0f, 13.0f / 16.0f, 16.0f / 32.0f, -13.0f / 16.0f, -13.0f / 32.0f);
        else
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12,
                13.0f, 13.0f, 0.0f, 16.0f / 32.0f, 13.0f / 16.0f, 13.0f / 32.0f);	// ▼

        EnableAlphaTest();
        SetLineColor(2);
        RenderColor((float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 1, m_fScrollBarRange_top, (float)1, (float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
        RenderColor((float)m_iPos_x + m_iWidth, m_fScrollBarRange_top, (float)1, (float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
        EndRenderColor();

        if (GetLineNum() >= m_iNumRenderLine)
        {
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y,
                m_fScrollBarWidth - 2, m_fScrollBarHeight, 0.0f, 1.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y,
                m_fScrollBarWidth - 2, 1, 0.0f, 0.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y + m_fScrollBarHeight - 1,
                m_fScrollBarWidth - 2, 1, 0.0f, 2.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
        }
        else
        {
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top,
                m_fScrollBarWidth - 2, m_fScrollBarRange_bottom - m_fScrollBarRange_top, 0.0f, 1.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top,
                m_fScrollBarWidth - 2, 1, 0.0f, 0.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top + m_fScrollBarHeight - 1,
                m_fScrollBarWidth - 2, 1, 0.0f, 2.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
        }
    }

    g_pRenderText->RenderText(m_iPos_x + 15, m_iPos_y - m_iHeight - 12, I18N::Game::NAME);
    g_pRenderText->RenderText(m_iPos_x + 113, m_iPos_y - m_iHeight - 12, I18N::Game::Members);
}

int CUIUnionGuildListBox::GetRenderLinePos_y(int iLineNumber)
{
    if (GetLineNum() > m_iNumRenderLine)
        return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1) * 13 - iLineNumber * 13 + 3);
    else
        return (m_iPos_y - m_iHeight + (GetLineNum() - 1) * 13 - iLineNumber * 13 + 3);
}

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

    memcpy(GuildMark[MARK_EDIT].Mark, m_TextListIter->GuildMark, sizeof(BYTE) * 64);
    CreateGuildMark(MARK_EDIT);
    RenderBitmap(BITMAP_GUILD, (float)iPos_x, (float)iPos_y, 8, 8);
    if (Hero->GuildMarkIndex >= 0)
        memcpy(GuildMark[MARK_EDIT].Mark, GuildMark[Hero->GuildMarkIndex].Mark, sizeof(BYTE) * 64);
    else
        memset(GuildMark[MARK_EDIT].Mark, 0, 64);

    wchar_t Text[MAX_TEXT_LENGTH + 1] = { 0 };
    mu_swprintf(Text, L"%ls", m_TextListIter->szName);
    g_pRenderText->RenderText(iPos_x + 12, iPos_y, Text);

    mu_swprintf(Text, L"%d", m_TextListIter->nMemberCount);
    g_pRenderText->RenderText(iPos_x + 138, iPos_y, Text, 0, 0, RT3_WRITE_RIGHT_TO_LEFT);

    DisableAlphaBlend();

    return TRUE;
}

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

void CUIUnionGuildListBox::DeleteText(DWORD dwGuildIndex)
{
}

int CUIUnionGuildListBox::GetTextCount()
{
    return m_TextList.size();
}

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
    return (lhs.m_iInvenIdx > rhs.m_iInvenIdx);
}

void CUIUnmixgemList::Sort()
{
    sort(m_TextList.begin(), m_TextList.end(), lessfunc);
}

void CUIUnmixgemList::AddText(int iIndex, BYTE cComType)
{
    if (iIndex < MAX_EQUIPMENT_INDEX || iIndex >= MAX_MY_INVENTORY_EX_INDEX || cComType == COMGEM::NOCOM) return;

    for (unsigned int i = 0; i < m_TextList.size(); ++i)
    {
        const UNMIX_TEXT& rt = m_TextList[i];
        if (rt.m_iInvenIdx == iIndex) return;
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

    ComputeScrollBar();

    //if (GetLineNum() >= m_iNumRenderLine)
    {
        if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - m_iHeight - 1, 13.0f, 13.0f) == TRUE)
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1, 13.0f, 13.0f, 13.0f / 16.0f, 29.0f / 32.0f, -13.0f / 16.0f, -13.0f / 32.0f);
        else
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - m_iHeight - 1, 13.0f, 13.0f, 0.0f, 3.0f / 32.0f, 13.0f / 16.0f, 13.0f / 32.0f);

        if (MouseLButtonPush && ::CheckMouseIn(m_iPos_x + m_iWidth - 12, m_iPos_y - 12, 13.0f, 13.0f) == TRUE)
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12, 13.0f, 13.0f, 13.0f / 16.0f, 16.0f / 32.0f, -13.0f / 16.0f, -13.0f / 32.0f);
        else
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - 12, (float)m_iPos_y - 12, 13.0f, 13.0f, 0.0f, 16.0f / 32.0f, 13.0f / 16.0f, 13.0f / 32.0f);

        EnableAlphaTest();
        SetLineColor(2);
        RenderColor((float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 1, m_fScrollBarRange_top, (float)1, (float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
        RenderColor((float)m_iPos_x + m_iWidth, m_fScrollBarRange_top, (float)1, (float)m_fScrollBarRange_bottom - m_fScrollBarRange_top);
        EndRenderColor();

        if (GetLineNum() >= m_iNumRenderLine)
        {
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y, m_fScrollBarWidth - 2, m_fScrollBarHeight, 0.0f, 1.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y, m_fScrollBarWidth - 2, 1, 0.0f, 0.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarPos_y + m_fScrollBarHeight - 1, m_fScrollBarWidth - 2, 1, 0.0f, 2.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
        }
        else
        {
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top, m_fScrollBarWidth - 2, m_fScrollBarRange_bottom - m_fScrollBarRange_top, 0.0f, 1.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top, m_fScrollBarWidth - 2, 1, 0.0f, 0.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
            RenderBitmap(BITMAP_INTERFACE_EX + 12, (float)m_iPos_x + m_iWidth - m_fScrollBarWidth + 2, m_fScrollBarRange_top + m_fScrollBarHeight - 1, m_fScrollBarWidth - 2, 1, 0.0f, 2.0f / 32.0f, 11.0f / 16.0f, 1.0f / 32.0f);
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

    wchar_t oText[MAX_GLOBAL_TEXT_STRING] = { 0, };

    const ITEM* pItem = FindInventoryItemBySlot(m_TextListIter->m_iInvenIdx);
    if (pItem)
    {
        int	  nIdx = COMGEM::Check_Jewel(pItem->Type);
        mu_swprintf(oText, L"%ls,  %d", I18N::Game::Lookup(COMGEM::GetJewelIndex(nIdx, COMGEM::eGEM_NAME)), (m_TextListIter->m_cLevel + 1) * 10);
    }

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

        if (MouseLButtonDBClick)
        {
            SLSetSelectLine(m_iCurrentRenderEndLine + iLineNumber + 1);
            UNMIX_TEXT* pt = GetSelectedText();

            if (pt->m_cLevel != COMGEM::NOCOM
                && pt->m_iInvenIdx >= MAX_EQUIPMENT_INDEX
                && pt->m_iInvenIdx < MAX_MY_INVENTORY_EX_INDEX)
                COMGEM::SelectFromList(pt->m_iInvenIdx, pt->m_cLevel);

            MouseLButtonDBClick = false;
        }
    }
    return TRUE;
}

int CUIUnmixgemList::GetRenderLinePos_y(int iLineNumber)
{
    if (GetLineNum() > m_iNumRenderLine)
        return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1) * 13 - iLineNumber * 13 + 3);
    else
        return (m_iPos_y - m_iHeight + (GetLineNum() - 1) * 13 - iLineNumber * 13 + 3);
}

CUIBCDeclareGuildListBox::CUIBCDeclareGuildListBox()
{
    m_iMaxLineCount = UIMAX_TEXT_LINE;
    m_iCurrentRenderEndLine = 0;
    m_iNumRenderLine = 18;

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

void CUIBCDeclareGuildListBox::AddText(const wchar_t* szGuildName, int nMarkCount, BYTE byIsGiveUp, BYTE bySeqNum)
{
    if (szGuildName == nullptr || szGuildName[0] == '\0') return;

    static BCDECLAREGUILD_TEXT text;
    text.m_bIsSelected = FALSE;
    wcsncpy(text.szName, szGuildName, MAX_GUILDNAME);
    text.szName[MAX_GUILDNAME] = 0;
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

bool BCDeclareGuildSortByMark(const BCDECLAREGUILD_TEXT& lhs, const BCDECLAREGUILD_TEXT& rhs)
{
    return (lhs.nCount < rhs.nCount);
}

void CUIBCDeclareGuildListBox::Sort()
{
    sort(m_TextList.begin(), m_TextList.end(), BCDeclareGuildSortByMark);
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
    g_pRenderText->RenderText(m_iPos_x + 5, m_iPos_y - m_iHeight - 12, I18N::Game::NAME);
    g_pRenderText->RenderText(m_iPos_x + 50, m_iPos_y - m_iHeight - 12, I18N::Game::NoReg);
    g_pRenderText->RenderText(m_iPos_x + 98, m_iPos_y - m_iHeight - 12, I18N::Game::Stat);
    g_pRenderText->RenderText(m_iPos_x + 123, m_iPos_y - m_iHeight - 12, I18N::Game::Order);
}

int CUIBCDeclareGuildListBox::GetRenderLinePos_y(int iLineNumber)
{
    if (GetLineNum() > m_iNumRenderLine)
        return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1) * 13 - iLineNumber * 13 + 3);
    else
        return (m_iPos_y - m_iHeight + (GetLineNum() - 1) * 13 - iLineNumber * 13 + 3);
}

BOOL CUIBCDeclareGuildListBox::RenderDataLine(int iLineNumber)
{
    wchar_t Text[MAX_TEXT_LENGTH + 1] = { 0 };

    mu_swprintf(Text, L"%ls", m_TextListIter->szName);

    if ((wcscmp(GuildMark[Hero->GuildMarkIndex].UnionName, Text) != 0 && wcscmp(GuildMark[Hero->GuildMarkIndex].GuildName, Text) != 0))
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

    mu_swprintf(Text, L"%d", m_TextListIter->nCount);
    g_pRenderText->RenderText(iPos_x + 70, iPos_y, Text, 0, 0, RT3_WRITE_RIGHT_TO_LEFT);

    if (m_TextListIter->byIsGiveUp)
        mu_swprintf(Text, L"%ls", I18N::Game::Failed);
    else
        mu_swprintf(Text, L"%ls", I18N::Game::Processing);
    g_pRenderText->RenderText(iPos_x + 120, iPos_y, Text, 0, 0, RT3_WRITE_RIGHT_TO_LEFT);

    mu_swprintf(Text, L"%u", m_TextListIter->bySeqNum);
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
    m_iNumRenderLine = 15;

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

void CUIBCGuildListBox::AddText(const wchar_t* szGuildName, BYTE byJoinSide, BYTE byGuildInvolved
    , int iGuildScore
)
{
    if (szGuildName == nullptr || szGuildName[0] == '\0') return;

    static BCGUILD_TEXT text;
    text.m_bIsSelected = FALSE;
    wcsncpy(text.szName, szGuildName, MAX_GUILDNAME);
    text.szName[MAX_GUILDNAME] = 0;
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
    EnableAlphaTest();
    SetLineColor(7, 0.4f);
    RenderColor(m_iPos_x - 1, m_iPos_y - m_iHeight - 1, m_iWidth + 1, m_iHeight + 2);
    SetLineColor(0, 0.4f);
    RenderColor(m_iPos_x - 1, m_iPos_y + 25 - 1, m_iWidth - 100 + 1, 20);

    SetLineColor(7, 0.4f);
    RenderColor(m_iPos_x - 1 + m_iWidth - 100 + 1, m_iPos_y + 25 - 1, m_iWidth - 60, 20);
    EndRenderColor();

    if (GetState() != UISTATE_SCROLL)
        ComputeScrollBar();

    g_pGuardWindow->RenderScrollBarFrame(m_iPos_x + m_iWidth - 8, m_fScrollBarRange_top, m_fScrollBarRange_bottom - m_fScrollBarRange_top);
    g_pGuardWindow->RenderScrollBar(m_iPos_x + m_iWidth - 12, m_fScrollBarPos_y, (GetState() == UISTATE_SCROLL && MouseLButtonPush));
    g_pRenderText->RenderText(m_iPos_x + 5, m_iPos_y - m_iHeight - 12, I18N::Game::NAME);
    g_pRenderText->RenderText(m_iPos_x + 80, m_iPos_y - m_iHeight - 12, I18N::Game::Camp);
    g_pRenderText->RenderText(m_iPos_x + 120, m_iPos_y - m_iHeight - 12, I18N::Game::Maintain);
    g_pRenderText->RenderText(m_iPos_x + 18, m_iPos_y + 31 - 1, I18N::Game::Score);
}

int CUIBCGuildListBox::GetRenderLinePos_y(int iLineNumber)
{
    if (GetLineNum() > m_iNumRenderLine)
        return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1) * 13 - iLineNumber * 13 + 3);
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
        if (m_TextListIter->byJoinSide == 1)
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

    wchar_t Text[MAX_TEXT_LENGTH + 1] = { 0 };
    mu_swprintf(Text, L"%ls", m_TextListIter->szName);
    g_pRenderText->RenderText(iPos_x + 2, iPos_y, Text);

    if (m_TextListIter->byJoinSide == 1)
        mu_swprintf(Text, L"%ls", I18N::Game::DefendingTeam);
    else
        mu_swprintf(Text, L"%ls", I18N::Game::InvadingTeam);
    g_pRenderText->RenderText(iPos_x + 100, iPos_y, Text, 0, 0, RT3_WRITE_RIGHT_TO_LEFT);

    if (m_TextListIter->byGuildInvolved == 1)
        mu_swprintf(Text, L"%ls", I18N::Game::Maintain);
    else
        mu_swprintf(Text, L"%ls", I18N::Game::Assist);

    g_pRenderText->RenderText(iPos_x + 137, iPos_y, Text, 0, 0, RT3_WRITE_RIGHT_TO_LEFT);

    g_pRenderText->SetTextColor(230, 220, 200, 255);
    wchar_t Dummy[300];	if (Select_Guild == iLineNumber)
    {
        if (m_TextListIter->byJoinSide == 1)
            mu_swprintf(Dummy, L"--");
        else
            mu_swprintf(Dummy, L"%ls :     %d", m_TextListIter->szName, m_TextListIter->iGuildScore);
        g_pRenderText->RenderText(m_iPos_x + 60, m_iPos_y + 31 - 1, Dummy);
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

void CUIBCGuildListBox::DeleteText(DWORD dwGuildIndex)
{
}

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

    RenderBitmap(BITMAP_INVENTORY + 19, fPos_x, fPos_y, fWidth, 2, 10 / 256.f, 5 / 16.f, 170.f / 256.f, 2.f / 16.f);
    RenderBitmap(BITMAP_INVENTORY + 19, fPos_x, fPos_y + fHeight, fWidth + 1, 2, 10 / 256.f, 5 / 16.f, 170.f / 256.f, 2.f / 16.f);
    RenderBitmap(BITMAP_INVENTORY, fPos_x, fPos_y, 2, fHeight, 1.f / 256.f, 5 / 16.f, 2.f / 256.f, 125.f / 256.f);
    RenderBitmap(BITMAP_INVENTORY, fPos_x + fWidth, fPos_y, 2, fHeight, 1.f / 256.f, 5 / 16.f, 2.f / 256.f, 125.f / 256.f);
}

CUIMoveCommandListBox::CUIMoveCommandListBox()
{
    m_iMaxLineCount = UIMAX_TEXT_LINE;
    m_iCurrentRenderEndLine = 0;

    m_iNumRenderLine = 21;

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

void CUIMoveCommandListBox::AddText(int iIndex, const wchar_t* szMapName, const wchar_t* szSubMapName, int iReqLevel, int iReqZen, int iGateNum)
{
    if (szMapName == nullptr || szMapName[0] == '\0' || iIndex < 0)
        return;

    static MOVECOMMAND_TEXT text;
    text.m_bIsSelected = FALSE;
    text.m_bCanMove = FALSE;
    wcsncpy(text.szMainMapName, szMapName, 32);
    wcsncpy(text.szSubMapName, szSubMapName, 32);
    text.iReqLevel = iReqLevel;
    text.iReqZen = iReqZen;
    text.iGateNum = iGateNum;

    m_TextList.push_front(text);

    RemoveText();
    SLSetSelectLine(0);
    if (GetLineNum() > m_iNumRenderLine) ++m_iCurrentRenderEndLine;

    if (GetLineNum() < m_iNumRenderLine);
    else if (GetLineNum() - m_iCurrentRenderEndLine < m_iNumRenderLine)
        m_iCurrentRenderEndLine = GetLineNum() - m_iNumRenderLine;
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
        return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1) * 13 - iLineNumber * 13 + 3);
    else
        return (m_iPos_y - m_iHeight + (GetLineNum() - 1) * 13 - iLineNumber * 13 + 3);
}

BOOL CUIMoveCommandListBox::RenderDataLine(int iLineNumber)
{
    return TRUE;
}

BOOL CUIMoveCommandListBox::DoLineMouseAction(int iLineNumber)
{
    return TRUE;
}

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

void CUICurQuestListBox::AddText(DWORD dwQuestIndex, const wchar_t* pszText)
{
    static SCurQuestItem sCurQuestItem;
    sCurQuestItem.m_bIsSelected = FALSE;
    sCurQuestItem.m_dwIndex = dwQuestIndex;
    wcsncpy(sCurQuestItem.m_szText, pszText, 64);

    m_TextList.push_front(sCurQuestItem);

    SendUIMessageDirect(UI_MESSAGE_LISTSCRLTOP, 0, 0);
    SLSetSelectLine(0);
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
        return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1) * 13 - iLineNumber * 13 + 3);
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

    wchar_t Text[MAX_TEXT_LENGTH + 1] = { 0 };
    mu_swprintf(Text, L"%ls", m_TextListIter->m_szText);
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

            if (SLGetSelectLineNum() == nSelLine)
                return TRUE;

            ::PlayBuffer(SOUND_CLICK01);

            SLSetSelectLine(nSelLine);
            m_TextListIter->m_bIsSelected = (m_TextListIter->m_bIsSelected + 1) % 2;	// T/F Reverse

            m_TextListIter = SLGetSelectLine();
            if (g_QuestMng.IsQuestByEtc(m_TextListIter->m_dwIndex))
                g_pMyQuestInfoWindow->QuestOpenBtnEnable(true);
            else
                g_pMyQuestInfoWindow->QuestOpenBtnEnable(false);

            g_pMyQuestInfoWindow->QuestGiveUpBtnEnable(true);
            g_pMyQuestInfoWindow->SetSelQuestSummary();

            const auto questNumber = static_cast<uint16_t>(LOWORD(m_TextListIter->m_dwIndex));
            const auto questGroup = static_cast<uint16_t>(HIWORD(m_TextListIter->m_dwIndex));
            SocketClient->ToGameServer()->SendQuestStateRequest(questNumber, questGroup);
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

void CUIQuestContentsListBox::AddText(HFONT hFont, DWORD dwColor, int nSort, const wchar_t* pszText)
{
    static SQuestContents sQuestContents;

    sQuestContents.m_hFont = hFont;
    sQuestContents.m_dwColor = dwColor;
    sQuestContents.m_nSort = nSort;
    wcsncpy(sQuestContents.m_szText, pszText, 64);
    sQuestContents.m_dwType = 0;
    sQuestContents.m_wIndex = 0;
    sQuestContents.m_pItem = nullptr;

    m_TextList.push_front(sQuestContents);

    SendUIMessageDirect(UI_MESSAGE_LISTSCRLTOP, 0, 0);
}

void CUIQuestContentsListBox::AddText(SRequestRewardText* pRequestRewardText, int nSort)
{
    static SQuestContents sQuestContents;

    sQuestContents.m_hFont = pRequestRewardText->m_hFont;
    sQuestContents.m_dwColor = pRequestRewardText->m_dwColor;
    sQuestContents.m_nSort = nSort;
    wcsncpy(sQuestContents.m_szText, pRequestRewardText->m_szText, 64);
    sQuestContents.m_eRequestReward = pRequestRewardText->m_eRequestReward;
    sQuestContents.m_dwType = pRequestRewardText->m_dwType;
    sQuestContents.m_wIndex = pRequestRewardText->m_wIndex;
    sQuestContents.m_pItem = pRequestRewardText->m_pItem;

    m_TextList.push_front(sQuestContents);

    SendUIMessageDirect(UI_MESSAGE_LISTSCRLTOP, 0, 0);
}

void CUIQuestContentsListBox::RenderInterface()
{
    if (GetState() != UISTATE_SCROLL)
        ComputeScrollBar();

    if (GetLineNum() > m_iNumRenderLine)
    {
        g_pGuardWindow->RenderScrollBarFrame(m_iPos_x + m_iWidth - 8, m_fScrollBarRange_top, m_fScrollBarRange_bottom - m_fScrollBarRange_top);
        g_pGuardWindow->RenderScrollBar(m_iPos_x + m_iWidth - 12, m_fScrollBarPos_y, (GetState() == UISTATE_SCROLL && MouseLButtonPush));
    }
}

int CUIQuestContentsListBox::GetRenderLinePos_y(int iLineNumber)
{
    if (GetLineNum() > m_iNumRenderLine)
        return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1) * 13 - iLineNumber * 13 + 3);
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

    wchar_t Text[MAX_TEXT_LENGTH + 1] = { 0 };
    mu_swprintf(Text, L"%ls", m_TextListIter->m_szText);
    g_pRenderText->RenderText(iPos_x, iPos_y, Text, GetWidth() - 18, 0, m_TextListIter->m_nSort);

    return TRUE;
}

void CUIQuestContentsListBox::RenderCoveredInterface()
{
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

void CUIQuestContentsListBox::DoActionSub(BOOL bMessageOnly)
{
    SLSetSelectLine(0);
}

BOOL CUIQuestContentsListBox::DoLineMouseAction(int iLineNumber)
{
    if (::CheckMouseIn(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13))
    {
        int nSelLine = m_iCurrentRenderEndLine + iLineNumber + 1;
        SLSetSelectLine(nSelLine);
    }

    return TRUE;
}

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

    sItem.m_iStorageSeq = _StorageItem.m_iStorageSeq;
    sItem.m_iStorageItemSeq = _StorageItem.m_iStorageItemSeq;
    sItem.m_iStorageGroupCode = _StorageItem.m_iStorageGroupCode;
    sItem.m_iProductSeq = _StorageItem.m_iProductSeq;
    sItem.m_iPriceSeq = _StorageItem.m_iPriceSeq;
    sItem.m_iNum = _StorageItem.m_iNum;
    sItem.m_wItemCode = _StorageItem.m_wItemCode;

    wcsncpy(sItem.m_szName, _StorageItem.m_szName, MAX_TEXT_LENGTH);
    wcsncpy(sItem.m_szNum, _StorageItem.m_szNum, MAX_TEXT_LENGTH);
    wcsncpy(sItem.m_szPeriod, _StorageItem.m_szPeriod, MAX_TEXT_LENGTH);
    wcsncpy(&(sItem.m_szType), &(_StorageItem.m_szType), sizeof(wchar_t));
    wcsncpy(sItem.m_szSendUserName, _StorageItem.m_szSendUserName, MAX_USERNAME_SIZE + 1);
    wcsncpy(sItem.m_szMessage, _StorageItem.m_szMessage, MAX_GIFT_MESSAGE_SIZE);

    m_TextList.push_front(sItem);

    RemoveText();
    SLSetSelectLine(0);

    if (GetLineNum() > m_iNumRenderLine)
        ++m_iCurrentRenderEndLine;

    if (GetLineNum() < m_iNumRenderLine);
    else if (GetLineNum() - m_iCurrentRenderEndLine < m_iNumRenderLine)
        m_iCurrentRenderEndLine = GetLineNum() - m_iNumRenderLine;

    if (m_TextList.size() == 1)
        SLSetSelectLine(1);

    if (m_TextList.empty() == FALSE)
    {
        SLSetSelectLine(GetLineNum());
        Scrolling(-10000);
    }
}

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
}

int CUIInGameShopListBox::GetRenderLinePos_y(int iLineNumber)
{
    if (GetLineNum() > m_iNumRenderLine)
        return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1) * 13 - iLineNumber * 13 + 3);
    else
        return (m_iPos_y - m_iHeight + (GetLineNum() - 1) * 13 - iLineNumber * 13 + 3);
}

BOOL CUIInGameShopListBox::RenderDataLine(int iLineNumber)
{
    EnableAlphaTest();

    if (SLGetSelectLineNum() == m_iCurrentRenderEndLine + iLineNumber + 1)
    {
        ::glColor4f(0.15f, 0.3f, 0.4f, 0.5f);
        RenderColor(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 4, 13);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glEnable(GL_TEXTURE_2D);
    }

    g_pRenderText->SetTextColor(255, 230, 210, 255);
    g_pRenderText->SetBgColor(0);

    int iPos_x = m_iPos_x;
    int iPos_y = GetRenderLinePos_y(iLineNumber);

    wchar_t szItemName[MAX_TEXT_LENGTH];
    if (m_TextListIter->m_iNum > 1)
    {
        mu_swprintf(szItemName, L"%ls(%d)", m_TextListIter->m_szName, m_TextListIter->m_iNum);
        g_pRenderText->RenderText(iPos_x, iPos_y, szItemName, 98, 0, RT3_SORT_LEFT);
    }
    else
    {
        g_pRenderText->RenderText(iPos_x, iPos_y, m_TextListIter->m_szName, 98, 0, RT3_SORT_LEFT);
    }

    g_pRenderText->RenderText(iPos_x + 102, iPos_y, m_TextListIter->m_szPeriod, 33, 0, RT3_SORT_RIGHT);

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

            if (SLGetSelectLineNum() == nSelLine)
                return TRUE;

            SLSetSelectLine(nSelLine);
            m_TextListIter->m_bIsSelected = (m_TextListIter->m_bIsSelected + 1) % 2;	// T/F Reverse

            m_TextListIter = SLGetSelectLine();
        }
    }
    return TRUE;
}

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

extern wchar_t TextList[50][100];

void CUIBuyingListBox::AddText(const wchar_t* pszExplanationText)
{
    if (pszExplanationText == nullptr || pszExplanationText[0] == '\0')
        return;

    static IGS_BuyList sIGS_Buying;
    sIGS_Buying.m_bIsSelected = FALSE;
    wcsncpy(sIGS_Buying.m_pszItemExplanation, pszExplanationText, LINE_TEXTMAX);
    m_TextList.push_front(sIGS_Buying);

    RemoveText();
    SLSetSelectLine(0);

    if (GetLineNum() > m_iNumRenderLine)
        ++m_iCurrentRenderEndLine;

    if (GetLineNum() < m_iNumRenderLine);
    else if (GetLineNum() - m_iCurrentRenderEndLine < m_iNumRenderLine)
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
        return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1) * 13 - iLineNumber * 13 + 3);
    else
        return (m_iPos_y - m_iHeight + (GetLineNum() - 1) * 13 - iLineNumber * 13 + 3);
}
BOOL CUIBuyingListBox::RenderDataLine(int iLineNumber)
{
    EnableAlphaTest();

    if (SLGetSelectLineNum() == m_iCurrentRenderEndLine + iLineNumber + 1 && GetLineColorRender())
    {
        ::glColor4f(0.15f, 0.3f, 0.4f, 0.5f);
        RenderColor(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glEnable(GL_TEXTURE_2D);
    }

    g_pRenderText->SetTextColor(255, 230, 210, 255);
    g_pRenderText->SetBgColor(0);

    int iPos_x = m_iPos_x + 3;
    int iPos_y = GetRenderLinePos_y(iLineNumber);

    wchar_t Text[MAX_TEXT_LENGTH + 1] = { 0 };
    if (m_TextListIter->m_pszItemExplanation != nullptr)
    {
        mu_swprintf(Text, L"%ls", m_TextListIter->m_pszItemExplanation);
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

            if (SLGetSelectLineNum() == nSelLine)
                return TRUE;

            SLSetSelectLine(nSelLine);
            m_TextListIter->m_bIsSelected = (m_TextListIter->m_bIsSelected + 1) % 2;	// T/F Reverse

            m_TextListIter = SLGetSelectLine();
        }
    }
    return TRUE;
}

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
    CRadioButton::m_nIterIndex = 0;
}
void CUIPackCheckBuyingListBox::AddText(IGS_SelectBuyItem& _Item)
{
    static IGS_SelectBuyItem sItem;

    sItem.m_iPackageSeq = _Item.m_iPackageSeq;
    sItem.m_iDisplaySeq = _Item.m_iDisplaySeq;
    sItem.m_iPriceSeq = _Item.m_iPriceSeq;
    sItem.m_wItemCode = _Item.m_wItemCode;
    sItem.m_iCashType = _Item.m_iCashType;

    wcsncpy(sItem.m_szItemName, _Item.m_szItemName, MAX_TEXT_LENGTH);
    wcsncpy(sItem.m_szItemPrice, _Item.m_szItemPrice, MAX_TEXT_LENGTH);
    wcsncpy(sItem.m_szItemPeriod, _Item.m_szItemPeriod, MAX_TEXT_LENGTH);
    wcsncpy(sItem.m_szAttribute, _Item.m_szAttribute, MAX_TEXT_LENGTH);

    CRadioButton::m_nIterIndex++;
    sItem.m_RadioBtn.SetRadioBtnIsEnable(CRadioButton::m_nIterIndex);

    m_TextList.push_front(sItem);

    RemoveText();
    SLSetSelectLine(0);

    if (GetLineNum() > m_iNumRenderLine)
        ++m_iCurrentRenderEndLine;

    if (GetLineNum() < m_iNumRenderLine);
    else if (GetLineNum() - m_iCurrentRenderEndLine < m_iNumRenderLine)
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

    if (SLGetSelectLineNum() == m_iCurrentRenderEndLine + nLine + 1)
    {
        ::glColor4f(0.07f, 0.31f, 0.31f, 0.5f);
        RenderColor(m_iPos_x + 3, GetRenderLinePos_y(nLine) + 1, m_iWidth - m_fScrollBarWidth + 1, TEXT_HEIGHTSIZE - 6);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glEnable(GL_TEXTURE_2D);
    }

    g_pRenderText->SetTextColor(255, 230, 210, 255);
    g_pRenderText->SetBgColor(0);

    int iPos_x = m_iPos_x + 3;
    int iPos_y = GetRenderLinePos_y(nLine);

    wchar_t Text[MAX_TEXT_LENGTH + 1] = { 0 };

    if (m_TextListIter->m_szItemName != nullptr)
    {
        if (SLGetSelectLineNum() == m_iCurrentRenderEndLine + nLine + 1)
            m_TextListIter->m_RadioBtn.UpdateActionCheck(true);
        else
            m_TextListIter->m_RadioBtn.UpdateActionCheck(false);

        m_TextListIter->m_RadioBtn.SetRadioBtnRect(iPos_x + 1, iPos_y + TEXT_HEIGHTSIZE * 0.2f);
        m_TextListIter->m_RadioBtn.RadiobuttonBoxRender();

        g_pRenderText->SetFont(g_hFontBold);
        mu_swprintf(Text, L"%ls", m_TextListIter->m_szItemName);
        g_pRenderText->RenderText(iPos_x + 20, iPos_y + 5, Text);

        g_pRenderText->SetFont(g_hFont);
        mu_swprintf(Text, L"%ls", m_TextListIter->m_szAttribute);
        g_pRenderText->RenderText(iPos_x + 20, iPos_y + 17, Text);
    }
    DisableAlphaBlend();

    return TRUE;
}
BOOL CUIPackCheckBuyingListBox::DoLineMouseAction(int nLine)
{
    if (::CheckMouseIn(m_iPos_x, GetRenderLinePos_y(nLine) - 3, m_iWidth - m_fScrollBarWidth + 1, TEXT_HEIGHTSIZE))
    {
        if (MouseLButtonPush)
        {
            MouseLButtonPush = false;

            int nSelLine = m_iCurrentRenderEndLine + nLine + 1;

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
    if (GetLineNum() > m_iNumRenderLine)
        return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1) * TEXT_HEIGHTSIZE - nLine * TEXT_HEIGHTSIZE + TEXT_HEIGHTSIZE * 0.1f);
    else
        return (m_iPos_y - m_iHeight + (GetLineNum() - 1) * TEXT_HEIGHTSIZE - nLine * TEXT_HEIGHTSIZE + TEXT_HEIGHTSIZE * 0.1f);
}

BOOL CUIPackCheckBuyingListBox::IsChangeLine()
{
    if (m_iCurrentLine != SLGetSelectLineNum())
    {
        m_iCurrentLine = SLGetSelectLineNum();
        return TRUE;
    }

    return FALSE;
}

CRadioButton::CRadioButton()
{
    m_byMouseState = LBTN_DEFAULT;
    m_bCheckState = false;
    m_rtCheckBtn.top = 0;
    m_rtCheckBtn.bottom = BTN_HEIGHT;
    m_rtCheckBtn.left = 0;
    m_rtCheckBtn.right = BTN_WIDTH;
    m_nRadioBtnEnable = 0;
}
CRadioButton::~CRadioButton()
{
    // n/a
}
void CRadioButton::SetCheckState(bool _Value)
{
    m_bCheckState = _Value;
}
void CRadioButton::SetRadioBtnIsEnable(int _Value)
{
    m_nRadioBtnEnable = _Value;
}
bool CRadioButton::UpdateMouseAction()
{
    bool _Temp = IsRadioBtn(m_rtCheckBtn);

    if (SEASON3B::IsPress(VK_LBUTTON) && _Temp)
    {
        m_byMouseState = LBTN_DOWN;
        return true;
    }
    else if (SEASON3B::IsRelease(VK_LBUTTON) && _Temp)
    {
        m_bCheckState ^= 1;
        m_byMouseState = LBTN_UP;
        m_nIterIndex = m_nRadioBtnEnable;
        return true;
    }
    else if (SEASON3B::IsRepeat(VK_LBUTTON) && _Temp)
    {
        m_byMouseState = LBTN_DOWN;
        return true;
    }

    if (m_byMouseState == LBTN_DOWN && !_Temp)
    {
        m_byMouseState = LBTN_UP;
        return false;
    }
    return false;
}

bool CRadioButton::UpdateActionCheck(int _nState)
{
    m_bCheckState = _nState;

    if (m_bCheckState)
        m_byMouseState = LBTN_UP;
    else
        m_byMouseState = LBTN_DEFAULT;

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
    if (MouseX >= _rt.left && MouseX < _rt.right && MouseY < _rt.bottom && MouseY >= _rt.top)
        return true;
    else
        return false;
    return false;
}
void CRadioButton::RadiobuttonBoxRender()
{
    switch (m_byMouseState)
    {
    case LBTN_DEFAULT:
        RenderImage(IMAGE_CHECKBTN, m_rtCheckBtn.left, m_rtCheckBtn.top, BTN_WIDTH, BTN_HEIGHT, 0, 0);
        break;
    case LBTN_UP:
        m_bCheckState ? RenderImage(IMAGE_CHECKBTN, m_rtCheckBtn.left, m_rtCheckBtn.top, BTN_WIDTH, BTN_HEIGHT, 0, BTN_HEIGHT + BTN_SPACE) : RenderImage(IMAGE_CHECKBTN, m_rtCheckBtn.left, m_rtCheckBtn.top, BTN_WIDTH, BTN_HEIGHT, 0, 0);
        break;
    case LBTN_DOWN:
        RenderImage(IMAGE_CHECKBTN, m_rtCheckBtn.left, m_rtCheckBtn.top, BTN_WIDTH, BTN_HEIGHT, 0, (BTN_HEIGHT + BTN_SPACE) * 2.0f);
        break;
    }
}
void CRadioButton::RenderImage(GLuint uiImageType, float x, float y, float width, float height, float su, float sv)
{
    BITMAP_t* pImage = &Bitmaps[uiImageType];

    float u, v, uw, vh;
    u = ((su + 0.1f) / (float)pImage->Width);
    v = ((sv + 0.1f) / (float)pImage->Height);
    uw = (width - 0.1f) / (float)pImage->Width - (0.1f / (float)pImage->Width);
    vh = (height - 0.1f) / (float)pImage->Height - (0.1f / (float)pImage->Height);

    RenderBitmap(uiImageType, x, y, width, height, u, v, uw, vh);
}
int CRadioButton::m_nIterIndex = 0;
#endif //PBG_ADD_INGAMESHOP_UI_ITEMSHOP


CUIExtraItemListBox::CUIExtraItemListBox()
{
    m_iMaxLineCount = 12;
    m_iCurrentRenderEndLine = 0;
    m_iNumRenderLine = 5;

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

void CUIExtraItemListBox::AddText(const wchar_t* pszPattern)
{
    if (pszPattern == nullptr || pszPattern[0] == '\0') return;

    static FILTERLIST_TEXT text;
    wcsncpy(text.m_szPattern, pszPattern, MAX_ITEM_NAME + 1);
    text.m_bIsSelected = FALSE;
    m_TextList.push_front(text);

    RemoveText();
    SLSetSelectLine(0);
    if (GetLineNum() > m_iNumRenderLine) ++m_iCurrentRenderEndLine;

    if (GetLineNum() < m_iNumRenderLine);
    else if (GetLineNum() - m_iCurrentRenderEndLine < m_iNumRenderLine)
        m_iCurrentRenderEndLine = GetLineNum() - m_iNumRenderLine;

    if (m_TextList.size() == 1)
        SLSetSelectLine(1);

    if (m_TextList.empty() == FALSE)
    {
        SLSetSelectLine(GetLineNum());
        Scrolling(-10000);
    }
}

void CUIExtraItemListBox::SetNumRenderLine(int iLine)
{
    if (iLine < m_iNumRenderLine && iLine < GetLineNum()) ++m_iCurrentRenderEndLine;
    else if (iLine > GetLineNum()) m_iCurrentRenderEndLine = 0;
    m_iNumRenderLine = iLine;
}


void CUIExtraItemListBox::RenderInterface()
{
    return;
}

int CUIExtraItemListBox::GetRenderLinePos_y(int iLineNumber)
{
    if (GetLineNum() > m_iNumRenderLine)
        return (m_iPos_y - m_iHeight + (m_iNumRenderLine - 1) * 13 - iLineNumber * 13 + 3);
    else
        return (m_iPos_y - m_iHeight + (GetLineNum() - 1) * 13 - iLineNumber * 13 + 3);
}

BOOL CUIExtraItemListBox::RenderDataLine(int iLineNumber)
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

    int iPos_x = m_iPos_x + 8;
    int iPos_y = GetRenderLinePos_y(iLineNumber);

    wchar_t Text[MAX_TEXT_LENGTH + 1] = { 0 };
    mu_swprintf(Text, L"%ls", m_TextListIter->m_szPattern);
    g_pRenderText->RenderText(iPos_x, iPos_y, Text);

    DisableAlphaBlend();

    return TRUE;
}

BOOL CUIExtraItemListBox::DoLineMouseAction(int iLineNumber)
{
    if (::CheckMouseIn(m_iPos_x, GetRenderLinePos_y(iLineNumber) - 3, m_iWidth - m_fScrollBarWidth + 1, 13))
    {
        if (MouseLButtonPush)
        {
            SLSetSelectLine(m_iCurrentRenderEndLine + iLineNumber + 1);
            m_TextListIter->m_bIsSelected = (m_TextListIter->m_bIsSelected + 1) % 2;
            MouseLButtonPush = false;
        }
    }
    return TRUE;
}

void CUIExtraItemListBox::DeleteText(const wchar_t* pszPattern)
{
    if (pszPattern == nullptr || wcslen(pszPattern) == 0) return;
    for (m_TextListIter = m_TextList.begin(); m_TextListIter != m_TextList.end(); ++m_TextListIter)
    {
        if (wcsncmp(m_TextListIter->m_szPattern, pszPattern, MAX_ITEM_NAME) == 0)
            break;
    }
    if (m_TextListIter == m_TextList.end()) return;

    if (SLGetSelectLineNum() != 1) SLSelectNextLine();
    m_TextList.erase(m_TextListIter);
}
