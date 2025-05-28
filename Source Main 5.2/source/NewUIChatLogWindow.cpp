#include "stdafx.h"

#include "NewUIChatLogWindow.h"
#include "NewUIManager.h"
#include "NewUISystem.h"
#include "DSPlaySound.h"
#include "UIControls.h"
#include "ZzzInterface.h"

using namespace SEASON3B;



SEASON3B::CNewUIChatLogWindow::CNewUIChatLogWindow()
{
    Init();
}

SEASON3B::CNewUIChatLogWindow::~CNewUIChatLogWindow()
{
    Release();
}

void SEASON3B::CNewUIChatLogWindow::Init()
{
    m_pNewUIMng = nullptr;
    m_WndPos.x = m_WndPos.y = 0;
    m_ScrollBtnPos.x = m_ScrollBtnPos.y = 0;
    m_WndSize.cx = WND_WIDTH; m_WndSize.cy = 0;
    m_nShowingLines = 6;
    m_iCurrentRenderEndLine = -1;
    m_fBackAlpha = 0.6f;

    m_EventState = EVENT_NONE;

    m_bShowFrame = false;

    m_CurrentRenderMsgType = TYPE_ALL_MESSAGE;
    m_bShowChatLog = true;

    m_bPointedMessage = false;
    m_iPointedMessageIndex = 0;
}

void SEASON3B::CNewUIChatLogWindow::LoadImages()
{
    LoadBitmap(L"Interface\\newui_scrollbar_up.tga", IMAGE_SCROLL_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_scrollbar_m.tga", IMAGE_SCROLL_MIDDLE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_scrollbar_down.tga", IMAGE_SCROLL_BOTTOM, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_scroll_on.tga", IMAGE_SCROLLBAR_ON, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_scroll_off.tga", IMAGE_SCROLLBAR_OFF, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_scrollbar_stretch.jpg", IMAGE_DRAG_BTN, GL_LINEAR);
}

void SEASON3B::CNewUIChatLogWindow::UnloadImages()
{
    DeleteBitmap(IMAGE_SCROLL_TOP);
    DeleteBitmap(IMAGE_SCROLL_MIDDLE);
    DeleteBitmap(IMAGE_SCROLL_BOTTOM);
    DeleteBitmap(IMAGE_SCROLLBAR_ON);
    DeleteBitmap(IMAGE_SCROLLBAR_OFF);
    DeleteBitmap(IMAGE_DRAG_BTN);
}

bool SEASON3B::CNewUIChatLogWindow::RenderBackground()
{
    if (m_bShowFrame)
    {
        float fRenderPosX = m_WndPos.x, fRenderPosY = m_WndPos.y - m_WndSize.cy;

        EnableAlphaTest();
        glColor4f(0.0f, 0.0f, 0.0f, GetBackAlpha());
        RenderColor(fRenderPosX, fRenderPosY, (float)m_WndSize.cx, (float)m_WndSize.cy);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        DisableAlphaBlend();
    }
    return true;
}

bool SEASON3B::CNewUIChatLogWindow::RenderMessages()
{
    float fRenderPosX = m_WndPos.x, fRenderPosY = m_WndPos.y - m_WndSize.cy + SCROLL_TOP_BOTTOM_PART_HEIGHT;

    type_vector_msgs* pvecMsgs = GetMsgs(GetCurrentMsgType());

    if (pvecMsgs == nullptr)
    {
        assert(!"empty chat!");
        return false;
    }

    int iRenderStartLine = 0;
    if (GetCurrentRenderEndLine() >= m_nShowingLines)
    {
        iRenderStartLine = GetCurrentRenderEndLine() - m_nShowingLines + 1;
    }
    else
    {
        fRenderPosY = fRenderPosY + FONT_LEADING + (SCROLL_MIDDLE_PART_HEIGHT * (m_nShowingLines - GetCurrentRenderEndLine() - 1));
    }

    BYTE byAlpha = 150;
    if (m_bShowFrame) byAlpha = 100;

    EnableAlphaTest();
    for (int i = iRenderStartLine, s = 0; i <= GetCurrentRenderEndLine(); i++, s++)
    {
        if (i < 0 && i >= (int)pvecMsgs->size()) break;

        bool bRenderMessage = true;
        g_pRenderText->SetFont(g_hFont);

        auto const pMsgText = (*pvecMsgs)[i];

        if (pMsgText->GetType() == TYPE_WHISPER_MESSAGE)
        {
            g_pRenderText->SetBgColor(255, 200, 50, 150);
            g_pRenderText->SetTextColor(0, 0, 0, 255);
        }
        else if (pMsgText->GetType() == TYPE_SYSTEM_MESSAGE)
        {
            g_pRenderText->SetBgColor(0, 0, 0, 150);
            g_pRenderText->SetTextColor(100, 150, 255, 255);
        }
        else if (pMsgText->GetType() == TYPE_ERROR_MESSAGE)
        {
            g_pRenderText->SetBgColor(0, 0, 0, 150);
            g_pRenderText->SetTextColor(255, 30, 0, 255);
        }
        else if (pMsgText->GetType() == TYPE_CHAT_MESSAGE)
        {
            g_pRenderText->SetBgColor(0, 0, 0, byAlpha);
            g_pRenderText->SetTextColor(205, 220, 239, 255);
        }
        else if (pMsgText->GetType() == TYPE_PARTY_MESSAGE)
        {
            g_pRenderText->SetBgColor(0, 200, 255, 150);
            g_pRenderText->SetTextColor(0, 0, 0, 255);
        }
        else if (pMsgText->GetType() == TYPE_GUILD_MESSAGE)
        {
            g_pRenderText->SetBgColor(0, 255, 150, 200);
            g_pRenderText->SetTextColor(0, 0, 0, 255);
        }
        else if (pMsgText->GetType() == TYPE_UNION_MESSAGE)
        {
            g_pRenderText->SetBgColor(200, 200, 0, 200);
            g_pRenderText->SetTextColor(0, 0, 0, 255);
        }
        else if (pMsgText->GetType() == TYPE_GENS_MESSAGE)
        {
            g_pRenderText->SetBgColor(150, 200, 100, 200);
            g_pRenderText->SetTextColor(0, 0, 0, 255);
        }
        else if (pMsgText->GetType() == TYPE_GM_MESSAGE)
        {
            g_pRenderText->SetBgColor(30, 30, 30, 200);
            g_pRenderText->SetTextColor(250, 200, 50, 255);
            g_pRenderText->SetFont(g_hFontBold);
        }
        else
        {
            bRenderMessage = false;
        }

        if (bRenderMessage && !pMsgText->GetText().empty())
        {
            POINT ptRenderPos = { (long)fRenderPosX + (long)WND_LEFT_RIGHT_EDGE, (long)fRenderPosY + (long)FONT_LEADING + ((long)SCROLL_MIDDLE_PART_HEIGHT * (long)s) };
            if (!pMsgText->GetID().empty())
            {
                if (m_bPointedMessage == true && m_iPointedMessageIndex == i)
                {
                    g_pRenderText->SetBgColor(30, 30, 30, 180);
                    g_pRenderText->SetTextColor(255, 128, 255, 255);
                }

                type_string strLine = std::wstring( pMsgText->GetID()) + L" : " + pMsgText->GetText();
                g_pRenderText->RenderText(ptRenderPos.x, ptRenderPos.y, strLine.c_str());
            }
            else
            {
                g_pRenderText->RenderText(ptRenderPos.x, ptRenderPos.y, pMsgText->GetText().c_str());
            }
        }
    }
    DisableAlphaBlend();

    return true;
}

bool SEASON3B::CNewUIChatLogWindow::RenderFrame()
{
    if (m_bShowFrame)
    {
        float const fRenderPosX = m_WndPos.x;
        float const fRenderPosY = m_WndPos.y - m_WndSize.cy;

        EnableAlphaTest();
        if (m_EventState == EVENT_RESIZING_BTN_DOWN)
        {
            glColor4f(0.7f, 0.7f, 0.7f, 1.0f);
        }
        else
        {
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        }
        RenderImage(IMAGE_DRAG_BTN, fRenderPosX, fRenderPosY - (float)RESIZING_BTN_HEIGHT, RESIZING_BTN_WIDTH, RESIZING_BTN_HEIGHT);
        DisableAlphaBlend();

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        RenderImage(IMAGE_SCROLL_TOP, fRenderPosX + m_WndSize.cx - SCROLL_BAR_WIDTH - WND_LEFT_RIGHT_EDGE, fRenderPosY + WND_TOP_BOTTOM_EDGE, SCROLL_BAR_WIDTH, WND_TOP_BOTTOM_EDGE);

        for (int i = 0; i < (int)GetNumberOfShowingLines(); i++)
        {
            RenderImage(IMAGE_SCROLL_MIDDLE, fRenderPosX + m_WndSize.cx - SCROLL_BAR_WIDTH - WND_LEFT_RIGHT_EDGE,
                fRenderPosY + WND_TOP_BOTTOM_EDGE + (float)(i * SCROLL_MIDDLE_PART_HEIGHT + SCROLL_TOP_BOTTOM_PART_HEIGHT), SCROLL_BAR_WIDTH, SCROLL_MIDDLE_PART_HEIGHT);
        }

        RenderImage(IMAGE_SCROLL_BOTTOM, fRenderPosX + m_WndSize.cx - SCROLL_BAR_WIDTH - WND_LEFT_RIGHT_EDGE,
            m_WndPos.y - WND_TOP_BOTTOM_EDGE - SCROLL_TOP_BOTTOM_PART_HEIGHT, SCROLL_BAR_WIDTH, SCROLL_TOP_BOTTOM_PART_HEIGHT);

        EnableAlphaTest();
        if (m_EventState == EVENT_SCROLL_BTN_DOWN)
        {
            glColor4f(0.7f, 0.7f, 0.7f, 1.0f);
        }
        else
        {
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        }
        RenderImage(IMAGE_SCROLLBAR_ON, m_ScrollBtnPos.x, m_ScrollBtnPos.y, SCROLL_BTN_WIDTH, SCROLL_BTN_HEIGHT);
        DisableAlphaBlend();
    }

    return true;
}

bool SEASON3B::CNewUIChatLogWindow::Create(CNewUIManager* pNewUIMng, int x, int y, int nShowingLines /* = 6 */)
{
    Release();

    if (nullptr == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_CHATLOGWINDOW, this);
    m_WndPos.x = x; m_WndPos.y = y;
    SetNumberOfShowingLines(nShowingLines);
    LoadImages();
    return true;
}

void SEASON3B::CNewUIChatLogWindow::Release()
{
    UnloadImages();
    ResetFilter();
    ClearAll();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = nullptr;
    }

    Init();
}

void SEASON3B::CNewUIChatLogWindow::SetPosition(int x, int y)
{
    m_WndPos.x = x;
    m_WndPos.y = y;
}

void SEASON3B::CNewUIChatLogWindow::AddText(const type_string& strID, const type_string& strText, MESSAGE_TYPE MsgType, MESSAGE_TYPE ErrMsgType /*= TYPE_ALL_MESSAGE*/)
{
    if (strID.empty() && strText.empty())
    {
        return;
    }

    if (GetNumberOfLines(MsgType) >= MAX_NUMBER_OF_LINES)
    {
        RemoveFrontLine(MsgType);
    }

    if (GetNumberOfLines(TYPE_ALL_MESSAGE) >= MAX_NUMBER_OF_LINES)
    {
        RemoveFrontLine(TYPE_ALL_MESSAGE);
    }

    if (m_vecFilters.empty())
    {
        ProcessAddText(strID, strText, MsgType, ErrMsgType);
    }
    else
    {
        if (MsgType != TYPE_CHAT_MESSAGE)
        {
            ProcessAddText(strID, strText, MsgType, ErrMsgType);
        }
        else if (CheckFilterText(strID) || CheckFilterText(strText))
        {
            ProcessAddText(strID, strText, MsgType, ErrMsgType);

            if (g_pOption->IsWhisperSound())
            {
                PlayBuffer(SOUND_WHISPER);
            }
        }
    }
}

void SEASON3B::CNewUIChatLogWindow::ProcessAddText(const type_string& strID, const type_string& strText, MESSAGE_TYPE MsgType, MESSAGE_TYPE ErrMsgType)
{
    type_vector_msgs* pvecMsgs = GetMsgs(MsgType);
    if (pvecMsgs == nullptr)
    {
        assert(!"Empty Message");
        return;
    }

    int nScrollLines = 0;
    if (strText.size() >= 20)
    {
        type_string	strText1, strText2;
        SeparateText(strID, strText, strText1, strText2);
        if (!strText1.empty())
        {
            const auto pMsgText = new CMessageText;
            if (!pMsgText->Create(strID, strText, MsgType))
                delete pMsgText;
            else
            {
                pvecMsgs->push_back(pMsgText);
            }

            const auto pAllMsgText = new CMessageText;
            if (!pAllMsgText->Create(strID, strText1, MsgType))
            {
                delete pAllMsgText;
            }
            else
            {
                m_vecAllMsgs.push_back(pAllMsgText);
            }

            if ((MsgType == TYPE_ERROR_MESSAGE) && (ErrMsgType != TYPE_ERROR_MESSAGE && ErrMsgType != TYPE_ALL_MESSAGE))
            {
                type_vector_msgs* pErrvecMsgs = GetMsgs(ErrMsgType);
                if (pErrvecMsgs == nullptr)
                {
                    assert(!"Error Chat");
                    return;
                }

                const auto pErrMsgText = new CMessageText;
                if (!pErrMsgText->Create(strID, strText1, MsgType))
                    delete pErrMsgText;
                else
                {
                    pErrvecMsgs->push_back(pErrMsgText);
                }
            }

            if (GetCurrentMsgType() == TYPE_ALL_MESSAGE || GetCurrentMsgType() == MsgType)
            {
                nScrollLines++;
            }
        }
        if (!strText2.empty())
        {
            const auto pMsgText = new CMessageText;
            if (!pMsgText->Create(L"", strText2, MsgType))
                delete pMsgText;
            else
            {
                pvecMsgs->push_back(pMsgText);
            }

            const auto pAllMsgText = new CMessageText;
            if (!pAllMsgText->Create(L"", strText2, MsgType))
                delete pAllMsgText;
            else
            {
                m_vecAllMsgs.push_back(pAllMsgText);
            }

            if ((MsgType == TYPE_ERROR_MESSAGE) && (ErrMsgType != TYPE_ERROR_MESSAGE && ErrMsgType != TYPE_ALL_MESSAGE))
            {
                type_vector_msgs* pErrvecMsgs = GetMsgs(ErrMsgType);
                if (pErrvecMsgs == nullptr)
                {
                    assert(!"Error chat 2");
                    return;
                }

                const auto pErrMsgText = new CMessageText;
                if (!pErrMsgText->Create(L"", strText2, MsgType))
                    delete pErrMsgText;
                else
                {
                    pErrvecMsgs->push_back(pErrMsgText);
                }
            }

            if (GetCurrentMsgType() == TYPE_ALL_MESSAGE || GetCurrentMsgType() == MsgType)
            {
                nScrollLines++;
            }
        }
    }
    else
    {
        const auto pMsgText = new CMessageText;
        if (!pMsgText->Create(strID, strText, MsgType))
            delete pMsgText;
        else
        {
            pvecMsgs->push_back(pMsgText);
        }

        const auto pAllMsgText = new CMessageText;
        if (!pAllMsgText->Create(strID, strText, MsgType))
            delete pAllMsgText;
        else
        {
            m_vecAllMsgs.push_back(pAllMsgText);
        }

        if ((MsgType == TYPE_ERROR_MESSAGE)
            && (ErrMsgType != TYPE_ERROR_MESSAGE && ErrMsgType != TYPE_ALL_MESSAGE))
        {
            type_vector_msgs* pErrvecMsgs = GetMsgs(ErrMsgType);
            if (pErrvecMsgs == nullptr)
            {
                assert(!"Error chat 3");
                return;
            }

            const auto pErrMsgText = new CMessageText;
            if (!pErrMsgText->Create(strID, strText, MsgType))
                delete pErrMsgText;
            else
            {
                pErrvecMsgs->push_back(pErrMsgText);
            }
        }

        if (GetCurrentMsgType() == TYPE_ALL_MESSAGE || GetCurrentMsgType() == MsgType)
        {
            nScrollLines++;
        }
    }

    pvecMsgs = GetMsgs(GetCurrentMsgType());
    if (pvecMsgs == nullptr)
    {
        assert(!"Error chat 4");
        return;
    }

    //. Auto Scrolling
    if (nScrollLines > 0 && ((pvecMsgs->size() - (m_iCurrentRenderEndLine + 1) - nScrollLines) < 3))
        m_iCurrentRenderEndLine = pvecMsgs->size() - 1;
    else if (!m_bShowFrame)
        m_iCurrentRenderEndLine = pvecMsgs->size() - 1;
}

void SEASON3B::CNewUIChatLogWindow::RemoveFrontLine(MESSAGE_TYPE MsgType)
{
    type_vector_msgs* pvecMsgs = GetMsgs(MsgType);

    if (pvecMsgs == nullptr)
    {
        assert(!"Empty Message RemoveFrontLine");
        return;
    }

    auto vi = pvecMsgs->begin();
    if (vi != pvecMsgs->end())
    {
        delete (*vi);
        vi = pvecMsgs->erase(vi);
    }

    if (MsgType == GetCurrentMsgType())
    {
        Scrolling(GetCurrentRenderEndLine());
    }
}

void SEASON3B::CNewUIChatLogWindow::Clear(MESSAGE_TYPE MsgType)
{
    type_vector_msgs* pvecMsgs = GetMsgs(MsgType);
    if (pvecMsgs == nullptr)
    {
        assert(!"Empty Message CNewUIChatLogWindow");
        return;
    }

    auto vi_msg = pvecMsgs->begin();
    for (; vi_msg != pvecMsgs->end(); vi_msg++)
        delete (*vi_msg);
    pvecMsgs->clear();

    if (MsgType == GetCurrentMsgType())
    {
        m_iCurrentRenderEndLine = -1;
    }
}

size_t SEASON3B::CNewUIChatLogWindow::GetNumberOfLines(MESSAGE_TYPE MsgType)
{
    type_vector_msgs* pvecMsgs = GetMsgs(MsgType);
    if (pvecMsgs == nullptr)
    {
        return 0;
    }

    return pvecMsgs->size();
}

int SEASON3B::CNewUIChatLogWindow::GetCurrentRenderEndLine() const
{
    return m_iCurrentRenderEndLine;
}

void SEASON3B::CNewUIChatLogWindow::Scrolling(int nRenderEndLine)
{
    type_vector_msgs* pvecMsgs = GetMsgs(m_CurrentRenderMsgType);
    if (pvecMsgs == nullptr)
    {
        assert(!"Empty message Scrolling");
        return;
    }

    if ((int)pvecMsgs->size() <= m_nShowingLines)
    {
        m_iCurrentRenderEndLine = pvecMsgs->size() - 1;
    }
    else
    {
        if (nRenderEndLine < m_nShowingLines)
            m_iCurrentRenderEndLine = m_nShowingLines - 1;

        else if (nRenderEndLine >= (int)pvecMsgs->size())
            m_iCurrentRenderEndLine = pvecMsgs->size() - 1;
        else
            m_iCurrentRenderEndLine = nRenderEndLine;
    }
}

void SEASON3B::CNewUIChatLogWindow::SetFilterText(const type_string& strFilterText)
{
    bool bPrevFilter = false;

    if (!m_vecFilters.empty())
    {
        bPrevFilter = true;
        ResetFilter();
    }

    wchar_t szTemp[MAX_CHAT_BUFFER_SIZE + 1] = { 0, };
    strFilterText.copy(szTemp, MAX_CHAT_BUFFER_SIZE);
    szTemp[MAX_CHAT_BUFFER_SIZE] = '\0';

    wchar_t* token = _wcstok(szTemp, L" ");
    token = _wcstok(nullptr, L" ");

    if (token == nullptr)
    {
        ResetFilter();
        AddText(L"", GlobalText[756], TYPE_SYSTEM_MESSAGE);
    }
    else
    {
        for (int i = 0; i < 5; i++)
        {
            if (nullptr == token)
            {
                break;
            }
            AddFilterWord(token);
            token = _wcstok(nullptr, L" ");
        }

        AddText(L"", GlobalText[755], TYPE_SYSTEM_MESSAGE);
    }
}

void SEASON3B::CNewUIChatLogWindow::ResetFilter()
{
    m_vecFilters.clear();
}

void SEASON3B::CNewUIChatLogWindow::SetSizeAuto()
{
    SetNumberOfShowingLines(GetNumberOfShowingLines() + 3);
}

void SEASON3B::CNewUIChatLogWindow::SetNumberOfShowingLines(int nShowingLines, OUT LPSIZE lpBoxSize/* = nullptr*/)
{
    m_nShowingLines = (int)(nShowingLines / 3) * 3;
    if (m_nShowingLines < 3)
        m_nShowingLines = 3;
    if (m_nShowingLines > 15)
        m_nShowingLines = 3;

    if (m_nShowingLines > GetCurrentRenderEndLine())
        Scrolling(m_nShowingLines - 1);

    UpdateWndSize();
    UpdateScrollPos();

    if (lpBoxSize)
    {
        lpBoxSize->cx = WND_WIDTH;
        lpBoxSize->cy = (SCROLL_MIDDLE_PART_HEIGHT * GetNumberOfShowingLines()) + (SCROLL_TOP_BOTTOM_PART_HEIGHT * 2) + (WND_TOP_BOTTOM_EDGE * 2);
    }
}
size_t SEASON3B::CNewUIChatLogWindow::GetNumberOfShowingLines() const
{
    return m_nShowingLines;
}

void SEASON3B::CNewUIChatLogWindow::SetBackAlphaAuto()
{
    m_fBackAlpha += 0.2f;

    if (m_fBackAlpha > 0.9f)
    {
        m_fBackAlpha = 0.2f;
    }
}

void SEASON3B::CNewUIChatLogWindow::SetBackAlpha(float fAlpha)
{
    if (fAlpha < 0.f)
        m_fBackAlpha = 0.f;
    else if (fAlpha > 1.f)
        m_fBackAlpha = 1.f;
    else
        m_fBackAlpha = fAlpha;
}

float SEASON3B::CNewUIChatLogWindow::GetBackAlpha() const
{
    return m_fBackAlpha;
}

void SEASON3B::CNewUIChatLogWindow::ShowFrame()
{
    m_bShowFrame = true;
}

void SEASON3B::CNewUIChatLogWindow::HideFrame()
{
    m_bShowFrame = false;
}

bool SEASON3B::CNewUIChatLogWindow::IsShowFrame()
{
    return m_bShowFrame;
}

bool SEASON3B::CNewUIChatLogWindow::UpdateMouseEvent()
{
    

    if (m_EventState == EVENT_NONE && false == MouseLButtonPush &&
        SEASON3B::CheckMouseIn(m_WndPos.x, m_WndPos.y - m_WndSize.cy, m_WndSize.cx, m_WndSize.cy))
    {
        m_EventState = EVENT_CLIENT_WND_HOVER;
        return false;
    }
    if (false == MouseLButtonPush && m_EventState == EVENT_CLIENT_WND_HOVER &&
        false == SEASON3B::CheckMouseIn(m_WndPos.x, m_WndPos.y - m_WndSize.cy, m_WndSize.cx, m_WndSize.cy))
    {
        m_EventState = EVENT_NONE;
        return true;
    }

    if (m_EventState == EVENT_CLIENT_WND_HOVER)
    {
        if (MouseWheel > 0)
            Scrolling(GetCurrentRenderEndLine() - 1);
        if (MouseWheel < 0)
            Scrolling(GetCurrentRenderEndLine() + 1);
        if (MouseWheel != 0)
            MouseWheel = 0;
    }

    m_bPointedMessage = false;
    if (SEASON3B::CheckMouseIn(m_WndPos.x, m_WndPos.y - m_WndSize.cy, m_WndSize.cx, m_WndSize.cy))
    {
        int iRenderStartLine = 0;
        if (GetCurrentRenderEndLine() >= m_nShowingLines)
            iRenderStartLine = GetCurrentRenderEndLine() - m_nShowingLines + 1;

        for (int i = iRenderStartLine, s = 0; i <= GetCurrentRenderEndLine(); i++, s++)
        {
            type_vector_msgs* pvecMsgs = GetMsgs(GetCurrentMsgType());
            if (pvecMsgs == nullptr)
            {
                return false;
            }

            CMessageText* pMsgText = (*pvecMsgs)[i];

            if (pMsgText->GetType() == TYPE_WHISPER_MESSAGE
                || pMsgText->GetType() == TYPE_CHAT_MESSAGE
                || pMsgText->GetType() == TYPE_PARTY_MESSAGE
                || pMsgText->GetType() == TYPE_GUILD_MESSAGE
                || pMsgText->GetType() == TYPE_UNION_MESSAGE
                || pMsgText->GetType() == TYPE_GENS_MESSAGE
                || pMsgText->GetType() == TYPE_GM_MESSAGE
                )
            {
                float fRenderPosX = m_WndPos.x;
                float fRenderPosY = m_WndPos.y - m_WndSize.cy + SCROLL_TOP_BOTTOM_PART_HEIGHT;
                if (GetCurrentRenderEndLine() < m_nShowingLines)
                {
                    fRenderPosY = fRenderPosY + FONT_LEADING + (SCROLL_MIDDLE_PART_HEIGHT * (m_nShowingLines - GetCurrentRenderEndLine() - 1));
                }

                POINT ptRenderPos;
                ptRenderPos.x = fRenderPosX + WND_LEFT_RIGHT_EDGE;
                ptRenderPos.y = fRenderPosY + FONT_LEADING + (SCROLL_MIDDLE_PART_HEIGHT * s);

                if (SEASON3B::CheckMouseIn(ptRenderPos.x, ptRenderPos.y, WND_WIDTH, SCROLL_MIDDLE_PART_HEIGHT))
                {
                    m_bPointedMessage = true;
                    m_iPointedMessageIndex = i;

                    std::wstring strID = pMsgText->GetID();
                    if (SEASON3B::IsPress(VK_RBUTTON) && strID.empty() == false)
                    {
                        g_pChatInputBox->SetWhsprID(strID.c_str());
                    }
                }
            }
        }
    }

    if (m_bShowFrame)
    {
        if (m_EventState == EVENT_CLIENT_WND_HOVER && MouseLButtonPush &&
            SEASON3B::CheckMouseIn(m_ScrollBtnPos.x, m_ScrollBtnPos.y, SCROLL_BTN_WIDTH, SCROLL_BTN_HEIGHT))
        {
            extern int MouseY;

            m_EventState = EVENT_SCROLL_BTN_DOWN;
            m_iGrapRelativePosY = MouseY - m_ScrollBtnPos.y;
            return false;
        }
        if (m_EventState == EVENT_SCROLL_BTN_DOWN)
        {
            if (SEASON3B::IsRepeat(VK_LBUTTON))
            {
                if (GetNumberOfLines(GetCurrentMsgType()) > GetNumberOfShowingLines())
                {
                    extern int MouseY;
                    if (MouseY - m_iGrapRelativePosY < m_WndPos.y - m_WndSize.cy + WND_TOP_BOTTOM_EDGE)
                    {
                        Scrolling(GetNumberOfShowingLines() - 1);
                        m_ScrollBtnPos.y = m_WndPos.y - m_WndSize.cy + WND_TOP_BOTTOM_EDGE;
                    }
                    else if (MouseY - m_iGrapRelativePosY > m_WndPos.y - SCROLL_BTN_HEIGHT - WND_TOP_BOTTOM_EDGE)
                    {
                        Scrolling(GetNumberOfLines(GetCurrentMsgType()) - 1);
                        m_ScrollBtnPos.y = m_WndPos.y - SCROLL_BTN_HEIGHT - WND_TOP_BOTTOM_EDGE;
                    }
                    else
                    {
                        float fScrollRate = (float)((MouseY - m_iGrapRelativePosY) - (m_WndPos.y - m_WndSize.cy + WND_TOP_BOTTOM_EDGE)) / (float)(m_WndSize.cy - WND_TOP_BOTTOM_EDGE * 2 - SCROLL_BTN_HEIGHT);
                        Scrolling(GetNumberOfShowingLines() + (float)(GetNumberOfLines(GetCurrentMsgType()) - GetNumberOfShowingLines()) * fScrollRate);

                        m_ScrollBtnPos.y = MouseY - m_iGrapRelativePosY;
                    }
                }
                return false;
            }
            if (SEASON3B::IsRelease(VK_LBUTTON))
            {
                m_EventState = EVENT_NONE;
                return true;
            }
        }

        POINT ptResizingBtn = { m_WndPos.x, m_WndPos.y - m_WndSize.cy - RESIZING_BTN_HEIGHT };
        if (m_EventState == EVENT_NONE && false == MouseLButtonPush &&
            SEASON3B::CheckMouseIn(ptResizingBtn.x, ptResizingBtn.y, RESIZING_BTN_WIDTH, RESIZING_BTN_HEIGHT))
        {
            m_EventState = EVENT_RESIZING_BTN_HOVER;
            return false;
        }
        if (false == MouseLButtonPush && m_EventState == EVENT_RESIZING_BTN_HOVER &&
            false == SEASON3B::CheckMouseIn(ptResizingBtn.x, ptResizingBtn.y, RESIZING_BTN_WIDTH, RESIZING_BTN_HEIGHT))
        {
            m_EventState = EVENT_NONE;
            return true;
        }
        if (m_EventState == EVENT_RESIZING_BTN_HOVER && MouseLButtonPush &&
            SEASON3B::CheckMouseIn(ptResizingBtn.x, ptResizingBtn.y, RESIZING_BTN_WIDTH, RESIZING_BTN_HEIGHT))
        {
            m_EventState = EVENT_RESIZING_BTN_DOWN;
            return false;
        }
        if (m_EventState == EVENT_RESIZING_BTN_DOWN)
        {
            if (MouseLButtonPush)
            {
                int nTopSections = (15 - GetNumberOfShowingLines()) / 3;
                int nBottomSections = (GetNumberOfShowingLines() - 3) / 3;
                for (int i = 0; i < nTopSections; i++)
                {
                    if (SEASON3B::CheckMouseIn(0, ptResizingBtn.y - RESIZING_BTN_HEIGHT - ((i + 1) * SCROLL_MIDDLE_PART_HEIGHT * 3 * 2),
                        640, SCROLL_MIDDLE_PART_HEIGHT * 3 + RESIZING_BTN_HEIGHT))
                    {
                        SetNumberOfShowingLines(GetNumberOfShowingLines() + (i + 1) * 3);
                        return false;
                    }
                }
                for (int i = 0; i < nBottomSections; i++)
                {
                    if (SEASON3B::CheckMouseIn(0, ptResizingBtn.y + RESIZING_BTN_HEIGHT + ((i + 1) * SCROLL_MIDDLE_PART_HEIGHT * 3),
                        640, RESIZING_BTN_HEIGHT + SCROLL_MIDDLE_PART_HEIGHT * 3))
                    {
                        SetNumberOfShowingLines(GetNumberOfShowingLines() - (i + 1) * 3);
                        return false;
                    }
                }
                if (SEASON3B::CheckMouseIn(0, 0, 640, m_WndPos.y - (SCROLL_MIDDLE_PART_HEIGHT * 15 + RESIZING_BTN_HEIGHT + SCROLL_TOP_BOTTOM_PART_HEIGHT * 2)))
                {
                    SetNumberOfShowingLines(15);
                }
                if (SEASON3B::CheckMouseIn(0, m_WndPos.y - (SCROLL_MIDDLE_PART_HEIGHT * 3 + SCROLL_TOP_BOTTOM_PART_HEIGHT * 2),
                    640, SCROLL_MIDDLE_PART_HEIGHT * 3 + SCROLL_TOP_BOTTOM_PART_HEIGHT * 2))
                {
                    SetNumberOfShowingLines(3);
                }
                return false;
            }
            if (false == MouseLButtonPush || true == MouseLButtonPop)
            {
                m_EventState = EVENT_NONE;
                return true;
            }
        }
    }
    return true;
}

bool SEASON3B::CNewUIChatLogWindow::UpdateKeyEvent()
{
    return true;
}

bool SEASON3B::CNewUIChatLogWindow::Update()
{
    UpdateScrollPos();

    return true;
}
bool SEASON3B::CNewUIChatLogWindow::Render()
{
    if (RenderBackground() == false)
    {
        return false;
    }

    if (m_bShowChatLog == true)
    {
        if (RenderMessages() == false)
        {
            return false;
        }
    }

    if (RenderFrame() == false)
    {
        return false;
    }

    return true;
}

float SEASON3B::CNewUIChatLogWindow::GetLayerDepth()
{
    return 6.1f;
}

float SEASON3B::CNewUIChatLogWindow::GetKeyEventOrder()
{
    return 8.0f;
}

void SEASON3B::CNewUIChatLogWindow::SeparateText(IN const type_string& strID, IN const type_string& strText, OUT type_string& strText1, OUT type_string& strText2)
{
    

    SIZE TextSize;
    
    float max_first_line_size = CLIENT_WIDTH * g_fScreenRate_x;
    if (!strID.empty())
    {
        const type_string strIDPart = strID + L" : ";

        GetTextExtentPoint32(g_pRenderText->GetFontDC(), strIDPart.c_str(), strIDPart.length(), &TextSize);
        max_first_line_size -= (TextSize.cx);
    }

    GetTextExtentPoint32(g_pRenderText->GetFontDC(), strText.c_str(), strText.length(), &TextSize);
    auto required_size = TextSize.cx;

    if (required_size <= max_first_line_size)
    {
        strText1 = strText;
        strText2 = L"";
        return;
    }

    BOOL bSpaceExist = (strText.find_last_of(L" ") != std::wstring::npos) ? TRUE : FALSE;
    int iLocToken = strText.length();

    while ((required_size > max_first_line_size) && (iLocToken > -1))
    {
        iLocToken = (bSpaceExist) ? strText.find_last_of(L" ", iLocToken - 1) : iLocToken - 1;

        GetTextExtentPoint32(g_pRenderText->GetFontDC(), (strText.substr(0, iLocToken)).c_str(), iLocToken, &TextSize);
        required_size = TextSize.cx;
    }

    strText1 = strText.substr(0, iLocToken);
    strText2 = strText.substr(iLocToken, strText.length() - iLocToken);
}

bool SEASON3B::CNewUIChatLogWindow::CheckFilterText(const type_string& strTestText)
{
    auto vi_filters = m_vecFilters.begin();
    for (; vi_filters != m_vecFilters.end(); vi_filters++)
    {
        if (vi_filters->find(strTestText))
        {
            return true;
        }
    }

    return false;
}

void SEASON3B::CNewUIChatLogWindow::UpdateWndSize()
{
    m_WndSize.cx = WND_WIDTH;
    m_WndSize.cy = (SCROLL_MIDDLE_PART_HEIGHT * GetNumberOfShowingLines()) + (SCROLL_TOP_BOTTOM_PART_HEIGHT * 2) + (WND_TOP_BOTTOM_EDGE * 2);
}

void SEASON3B::CNewUIChatLogWindow::UpdateScrollPos()
{
    float fPosRate = 1.f;

    if (GetNumberOfLines(GetCurrentMsgType()) > GetNumberOfShowingLines())
    {
        if ((int)GetNumberOfShowingLines() > GetCurrentRenderEndLine())
        {
            fPosRate = 0.f;
        }
        else
        {
            fPosRate = (float)(GetCurrentRenderEndLine() + 1 - GetNumberOfShowingLines()) / (float)(GetNumberOfLines(GetCurrentMsgType()) - GetNumberOfShowingLines());
        }
    }
    if (m_EventState != EVENT_SCROLL_BTN_DOWN)
    {
        m_ScrollBtnPos.x = m_WndPos.x + m_WndSize.cx - SCROLL_BAR_WIDTH - WND_LEFT_RIGHT_EDGE - 4;
        m_ScrollBtnPos.y = m_WndPos.y - m_WndSize.cy + WND_TOP_BOTTOM_EDGE + ((float)(m_WndSize.cy - SCROLL_BTN_HEIGHT - WND_TOP_BOTTOM_EDGE * 2) * fPosRate);
    }
}

void SEASON3B::CNewUIChatLogWindow::AddFilterWord(const type_string& strWord)
{
    if (m_vecFilters.size() > 5)
        return;

    auto vi_filters = m_vecFilters.begin();
    for (; vi_filters != m_vecFilters.end(); vi_filters++)
    {
        if (0 == (*vi_filters).compare(strWord))
        {
            return;
        }
    }

    m_vecFilters.push_back(strWord);
}

void SEASON3B::CNewUIChatLogWindow::ClearAll()
{
    for (int i = TYPE_ALL_MESSAGE; i < NUMBER_OF_TYPES; i++)
    {
        Clear((MESSAGE_TYPE)i);
    }

    m_iCurrentRenderEndLine = -1;
}

SEASON3B::CNewUIChatLogWindow::type_vector_msgs* SEASON3B::CNewUIChatLogWindow::GetMsgs(MESSAGE_TYPE MsgType)
{
    switch (MsgType)
    {
    case TYPE_ALL_MESSAGE:
        return &m_vecAllMsgs;
    case TYPE_CHAT_MESSAGE:
        return &m_VecChatMsgs;
    case TYPE_WHISPER_MESSAGE:
        return &m_vecWhisperMsgs;
    case TYPE_SYSTEM_MESSAGE:
        return &m_VecSystemMsgs;
    case TYPE_ERROR_MESSAGE:
        return &m_vecErrorMsgs;
    case TYPE_PARTY_MESSAGE:
        return &m_vecPartyMsgs;
    case TYPE_GUILD_MESSAGE:
        return &m_vecGuildMsgs;
    case TYPE_UNION_MESSAGE:
        return &m_vecUnionMsgs;
    case TYPE_GENS_MESSAGE:
        return &m_vecGensMsgs;
    case TYPE_GM_MESSAGE:
        return &m_vecGMMsgs;
    }

    return nullptr;
}

void SEASON3B::CNewUIChatLogWindow::ChangeMessage(MESSAGE_TYPE MsgType)
{
    m_CurrentRenderMsgType = MsgType;

    type_vector_msgs* pvecMsgs = GetMsgs(GetCurrentMsgType());
    if (pvecMsgs == nullptr)
    {
        return;
    }

    m_iCurrentRenderEndLine = pvecMsgs->size() - 1;
}

SEASON3B::MESSAGE_TYPE SEASON3B::CNewUIChatLogWindow::GetCurrentMsgType() const
{
    return m_CurrentRenderMsgType;
}

void SEASON3B::CNewUIChatLogWindow::ShowChatLog()
{
    m_bShowChatLog = true;

    type_vector_msgs* pvecMsgs = GetMsgs(GetCurrentMsgType());
    if (pvecMsgs == nullptr)
    {
        return;
    }

    m_iCurrentRenderEndLine = pvecMsgs->size() - 1;
}

void SEASON3B::CNewUIChatLogWindow::HideChatLog()
{
    m_bShowChatLog = false;
}

SEASON3B::CNewUISystemLogWindow::CNewUISystemLogWindow()
{
    Init();
}

SEASON3B::CNewUISystemLogWindow::~CNewUISystemLogWindow()
{
    Release();
}

void SEASON3B::CNewUISystemLogWindow::Init()
{
    m_pNewUIMng = nullptr;
    m_WndPos.x = m_WndPos.y = 0;
    m_WndSize.cx = WND_WIDTH; m_WndSize.cy = 0;
    m_nShowingLines = 6;
    m_iCurrentRenderEndLine = -1;
    m_fBackAlpha = 0.6f;
}


bool SEASON3B::CNewUISystemLogWindow::RenderMessages()
{
    if (!m_bShowMessages)
    {
        return true;
    }

    int fRenderPosX = m_WndPos.x + FONT_LEADING;
    int fRenderPosY = m_WndPos.y - m_WndSize.cy;

    if (m_vecAllMsgs.empty())
    {
        return true;
    }

    int iRenderStartLine = 0;
    if (GetCurrentRenderEndLine() >= m_nShowingLines)
    {
        iRenderStartLine = GetCurrentRenderEndLine() - m_nShowingLines + 1;
    }
    else
    {
        fRenderPosY += FONT_LEADING;
    }

    const auto rowHeight = static_cast<int>(FontHeight * 1.2 / g_fScreenRate_y);

    EnableAlphaTest();
    for (int i = iRenderStartLine; i <= GetCurrentRenderEndLine(); i++)
    {
        if (i < 0 && i >= static_cast<int>(m_vecAllMsgs.size())) break;

        g_pRenderText->SetFont(g_hFont);

        auto const message = m_vecAllMsgs[i];
        if (message->GetType() == TYPE_SYSTEM_MESSAGE)
        {
            g_pRenderText->SetBgColor(0, 0, 0, 100);
            g_pRenderText->SetTextColor(100, 150, 255, 255);
        }
        else
        {
            g_pRenderText->SetBgColor(0, 0, 0, 100);
            g_pRenderText->SetTextColor(255, 30, 0, 255);
        }

        g_pRenderText->RenderText(fRenderPosX, fRenderPosY + rowHeight * i, message->GetText().c_str());
    }

    DisableAlphaBlend();

    return true;
}

bool SEASON3B::CNewUISystemLogWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    Release();

    if (nullptr == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_SYSTEMLOGWINDOW, this);
    m_WndPos.x = x;
    m_WndPos.y = y;
    return true;
}

void SEASON3B::CNewUISystemLogWindow::Release()
{
    ClearAll();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = nullptr;
    }

    Init();
}

void SEASON3B::CNewUISystemLogWindow::SetPosition(int x, int y)
{
    m_WndPos.x = x;
    m_WndPos.y = y;
}

void SEASON3B::CNewUISystemLogWindow::AddText(const type_string& strText, MESSAGE_TYPE MsgType)
{
    if (strText.empty())
    {
        return;
    }

    type_vector_msgs* pvecMsgs = &m_vecAllMsgs;
    if (pvecMsgs == nullptr)
    {
        assert(!"Empty Message");
        return;
    }

    auto pMsgText = new CMessageText;
    if (!pMsgText->Create(L"", strText, MsgType))
    {
        delete pMsgText;
        return;
    }

    if (pvecMsgs->size() >= MAX_NUMBER_OF_LINES)
    {
        RemoveFrontLine();
    }

    pvecMsgs->push_back(pMsgText);

    m_iCurrentRenderEndLine = pvecMsgs->size() - 1;
}

void SEASON3B::CNewUISystemLogWindow::RemoveFrontLine()
{
    auto vi = m_vecAllMsgs.begin();
    if (vi != m_vecAllMsgs.end())
    {
        delete (*vi);
        vi = m_vecAllMsgs.erase(vi);
    }
}

int SEASON3B::CNewUISystemLogWindow::GetCurrentRenderEndLine() const
{
    return m_iCurrentRenderEndLine;
}

bool SEASON3B::CNewUISystemLogWindow::UpdateMouseEvent()
{
    return true;
}

bool SEASON3B::CNewUISystemLogWindow::UpdateKeyEvent()
{
    return true;
}

bool SEASON3B::CNewUISystemLogWindow::Update()
{
    return true;
}
bool SEASON3B::CNewUISystemLogWindow::Render()
{
    return RenderMessages();
}

float SEASON3B::CNewUISystemLogWindow::GetLayerDepth()
{
    return 6.05f;
}

float SEASON3B::CNewUISystemLogWindow::GetKeyEventOrder()
{
    return 8.0f;
}

void SEASON3B::CNewUISystemLogWindow::ClearAll()
{
    auto vi_msg = m_vecAllMsgs.begin();
    for (; vi_msg != m_vecAllMsgs.end(); vi_msg++)
        delete (*vi_msg);
    m_vecAllMsgs.clear();

    m_iCurrentRenderEndLine = -1;
}

bool SEASON3B::CNewUISystemLogWindow::CheckChatRedundancy(const type_string& strText, int iSearchLine/* = 1*/)
{
    if (m_vecAllMsgs.empty()) return false;
    auto vri_msgs = m_vecAllMsgs.rbegin();
    for (int i = 0; (i < iSearchLine) || (vri_msgs != m_vecAllMsgs.rend()); vri_msgs++, i++)
        if (0 == (*vri_msgs)->GetText().compare(strText)) return true;
    return false;
}
