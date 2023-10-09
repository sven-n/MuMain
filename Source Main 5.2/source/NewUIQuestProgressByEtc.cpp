//*****************************************************************************
// File: NewUIQuestProgressByEtc.cpp
//*****************************************************************************

#include "stdafx.h"
#include "NewUIQuestProgressByEtc.h"

#include "DSPlaySound.h"
#include "NewUISystem.h"

#include "UsefulDef.h"

using namespace SEASON3B;

#define QPE_NPC_MAX_LINE_PER_PAGE	7
#define QPE_TEXT_GAP				15
#define QPE_LIST_BOX_LINE_NUM		12

CNewUIQuestProgressByEtc::CNewUIQuestProgressByEtc()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
}

CNewUIQuestProgressByEtc::~CNewUIQuestProgressByEtc()
{
    Release();
}

bool CNewUIQuestProgressByEtc::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC, this);

    SetPos(x, y);

    LoadImages();

    m_btnProgressL.ChangeButtonImgState(true, IMAGE_QPE_BTN_L);
    m_btnProgressL.ChangeButtonInfo(x + 131, y + 165, 17, 18);
    m_btnProgressR.ChangeButtonImgState(true, IMAGE_QPE_BTN_R);
    m_btnProgressR.ChangeButtonInfo(x + 153, y + 165, 17, 18);

    m_btnComplete.ChangeText(GlobalText[2811]);	// "È®     ÀÎ"
    m_btnComplete.ChangeButtonImgState(true, IMAGE_QPE_BTN_COMPLETE, true);
    m_btnComplete.ChangeButtonInfo(x + (QPE_WIDTH - 108) / 2, y + 362, 108, 29);

    m_btnClose.ChangeButtonImgState(true, IMAGE_QPE_BTN_CLOSE);
    m_btnClose.ChangeButtonInfo(x + 13, y + 392, 36, 29);
    m_btnClose.ChangeToolTipText(GlobalText[1002], true);

    m_RequestRewardListBox.SetNumRenderLine(QPE_LIST_BOX_LINE_NUM);
    m_RequestRewardListBox.SetSize(174, 158);

    Show(false);

    return true;
}

void CNewUIQuestProgressByEtc::Release()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CNewUIQuestProgressByEtc::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;

    m_btnProgressL.SetPos(x + 131, y + 165);
    m_btnProgressR.SetPos(x + 153, y + 165);
    m_btnComplete.SetPos(x + (QPE_WIDTH - 108) / 2, y + 362);
    m_btnClose.SetPos(x + 13, y + 392);

    m_RequestRewardListBox.SetPosition(m_Pos.x + 9, m_Pos.y + 360);
}

bool CNewUIQuestProgressByEtc::UpdateMouseEvent()
{
    if (REQUEST_REWARD_MODE == m_eLowerView)
        m_RequestRewardListBox.DoAction();

    if (ProcessBtns())
        return false;

    if (UpdateSelTextMouseEvent())
        return false;

    if (CheckMouseIn(m_Pos.x, m_Pos.y, QPE_WIDTH, QPE_HEIGHT))
        return false;

    return true;
}

bool CNewUIQuestProgressByEtc::ProcessBtns()
{
    if (m_btnClose.UpdateMouseEvent())
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC);
        return true;
    }
    else if (SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(m_Pos.x + 169, m_Pos.y + 7, 13, 12))
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
    else if (m_bRequestComplete && m_bCanClick)
    {
        if (m_btnComplete.UpdateMouseEvent())
        {
            const auto questNumber = static_cast<uint16_t>((m_dwCurQuestIndex & 0xFF00) >> 16);
            const auto questGroup = static_cast<uint16_t>(m_dwCurQuestIndex & 0xFF);
            SocketClient->ToGameServer()->SendQuestCompletionRequest(questNumber, questGroup);
            PlayBuffer(SOUND_CLICK01);
            m_bCanClick = false;
            return true;
        }
    }

    return false;
}

bool CNewUIQuestProgressByEtc::UpdateSelTextMouseEvent()
{
    if (PLAYER_WORDS_MODE != m_eLowerView || !m_bCanClick)
        return false;

    m_nSelAnswer = 0;
    if (MouseX < m_Pos.x + 11 || MouseX > m_Pos.x + 179)
        return false;

    int nTopY;
    int nBottomY = m_Pos.y + 203;
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
                const auto questNumber = static_cast<uint16_t>((m_dwCurQuestIndex & 0xFF00) >> 16);
                const auto questGroup = static_cast<uint16_t>(m_dwCurQuestIndex & 0xFF);
                SocketClient->ToGameServer()->SendQuestProceedRequest(questNumber, questGroup, (BYTE)m_nSelAnswer);
                PlayBuffer(SOUND_CLICK01);
                m_bCanClick = false;
                return true;
            }
            break;
        }
    }

    return false;
}

bool CNewUIQuestProgressByEtc::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC))
    {
        if (SEASON3B::IsPress(VK_ESCAPE))
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC);
            return false;
        }
    }

    return true;
}

bool CNewUIQuestProgressByEtc::Update()
{
    return true;
}

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
        ::EnableAlphaTest();
    }

    m_btnClose.Render();

    ::DisableAlphaBlend();

    return true;
}

void CNewUIQuestProgressByEtc::RenderBackImage()
{
    RenderImage(IMAGE_QPE_BACK, m_Pos.x, m_Pos.y, float(QPE_WIDTH), float(QPE_HEIGHT));
    RenderImage(IMAGE_QPE_TOP, m_Pos.x, m_Pos.y, float(QPE_WIDTH), 64.f);
    RenderImage(IMAGE_QPE_LEFT, m_Pos.x, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_QPE_RIGHT, m_Pos.x + QPE_WIDTH - 21, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_QPE_BOTTOM, m_Pos.x, m_Pos.y + QPE_HEIGHT - 45, float(QPE_WIDTH), 45.f);

    RenderImage(IMAGE_QPE_LINE, m_Pos.x + 1, m_Pos.y + 181, 188.f, 21.f);
}

void CNewUIQuestProgressByEtc::RenderSelTextBlock()
{
    if (PLAYER_WORDS_MODE != m_eLowerView)
        return;

    if (0 == m_nSelAnswer)
        return;

    int nBlockPosY = m_Pos.y + 203;
    int i;
    for (i = 0; i < m_nSelAnswer - 1; ++i)
        nBlockPosY += QPE_TEXT_GAP * m_anAnswerLine[i];

    ::glColor4f(0.5f, 0.7f, 0.3f, 0.5f);
    ::RenderColor(m_Pos.x + 11, nBlockPosY, 168.f, QPE_TEXT_GAP * m_anAnswerLine[m_nSelAnswer - 1]);
    ::EndRenderColor();
}

void CNewUIQuestProgressByEtc::RenderText()
{
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetBgColor(0);

    g_pRenderText->SetTextColor(230, 230, 230, 255);
    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 12, L"Quest", QPE_WIDTH, 0, RT3_SORT_CENTER);
    g_pRenderText->SetTextColor(36, 242, 252, 255);
    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 27, g_QuestMng.GetSubject(m_dwCurQuestIndex), QPE_WIDTH, 0, RT3_SORT_CENTER);

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(255, 230, 210, 255);
    int i;
    for (i = 0; i < QPE_NPC_MAX_LINE_PER_PAGE; ++i)
        g_pRenderText->RenderText(m_Pos.x + 13, m_Pos.y + 59 + (QPE_TEXT_GAP * i), m_aszNPCWords[i + QPE_NPC_MAX_LINE_PER_PAGE * m_nSelNPCPage], 0, 0, RT3_SORT_LEFT);

    if (PLAYER_WORDS_MODE == m_eLowerView)
    {
        g_pRenderText->SetTextColor(255, 230, 210, 255);
        for (i = 0; i < QPE_PLAYER_LINE_MAX; ++i)
            g_pRenderText->RenderText(m_Pos.x + 13, m_Pos.y + 207 + (QPE_TEXT_GAP * i), m_aszPlayerWords[i], 0, 0, RT3_SORT_LEFT);
    }
}

bool CNewUIQuestProgressByEtc::IsVisible() const
{
    return CNewUIObj::IsVisible();
}

float CNewUIQuestProgressByEtc::GetLayerDepth()
{
    return 3.1f;
}

void CNewUIQuestProgressByEtc::LoadImages()
{
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_QPE_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back04.tga", IMAGE_QPE_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-L.tga", IMAGE_QPE_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-R.tga", IMAGE_QPE_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_QPE_BOTTOM, GL_LINEAR);

    LoadBitmap(L"Interface\\newui_myquest_Line.tga", IMAGE_QPE_LINE, GL_LINEAR);
    LoadBitmap(L"Interface\\Quest_bt_L.tga", IMAGE_QPE_BTN_L, GL_LINEAR);
    LoadBitmap(L"Interface\\Quest_bt_R.tga", IMAGE_QPE_BTN_R, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_btn_empty.tga", IMAGE_QPE_BTN_COMPLETE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_exit_00.tga", IMAGE_QPE_BTN_CLOSE, GL_LINEAR);
}

void CNewUIQuestProgressByEtc::UnloadImages()
{
    DeleteBitmap(IMAGE_QPE_BTN_CLOSE);
    DeleteBitmap(IMAGE_QPE_BTN_COMPLETE);
    DeleteBitmap(IMAGE_QPE_BTN_R);
    DeleteBitmap(IMAGE_QPE_BTN_L);
    DeleteBitmap(IMAGE_QPE_LINE);

    DeleteBitmap(IMAGE_QPE_BOTTOM);
    DeleteBitmap(IMAGE_QPE_RIGHT);
    DeleteBitmap(IMAGE_QPE_LEFT);
    DeleteBitmap(IMAGE_QPE_TOP);
    DeleteBitmap(IMAGE_QPE_BACK);
}

void CNewUIQuestProgressByEtc::ProcessOpening()
{
    ::PlayBuffer(SOUND_INTERFACE01);
}

bool CNewUIQuestProgressByEtc::ProcessClosing()
{
    g_QuestMng.DelQuestIndexByEtcList(m_dwCurQuestIndex);
    m_dwCurQuestIndex = 0;
    SocketClient->ToGameServer()->SendCloseNpcRequest();
    ::PlayBuffer(SOUND_CLICK01);
    return true;
}

void CNewUIQuestProgressByEtc::SetContents(DWORD dwQuestIndex)
{
    if (0 == dwQuestIndex)
        return;

    m_dwCurQuestIndex = dwQuestIndex;

    SetCurNPCWords();
    m_bCanClick = true;

    m_btnProgressL.Lock();

    if (NULL != g_QuestMng.GetAnswer(m_dwCurQuestIndex, 0))
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

        if (0 == m_nMaxNPCPage)
            m_btnProgressR.Lock();
        else
            m_btnProgressR.UnLock();
    }
}

void CNewUIQuestProgressByEtc::SetCurNPCWords()
{
    if (0 == m_dwCurQuestIndex)
        return;

    ::memset(m_aszNPCWords[0], 0, sizeof(char) * QPE_NPC_LINE_MAX * QPE_WORDS_ROW_MAX);

    g_pRenderText->SetFont(g_hFont);
    int nLine = ::DivideStringByPixel(&m_aszNPCWords[0][0],
        QPE_NPC_LINE_MAX, QPE_WORDS_ROW_MAX, g_QuestMng.GetNPCWords(m_dwCurQuestIndex), 160);

    if (1 > nLine)
        return;

    m_nMaxNPCPage = (nLine - 1) / QPE_NPC_MAX_LINE_PER_PAGE;
    m_nSelNPCPage = 0;
}

void CNewUIQuestProgressByEtc::SetCurPlayerWords()
{
    if (0 == m_dwCurQuestIndex)
        return;

    ::memset(m_aszPlayerWords[0], 0, sizeof(char) * QPE_PLAYER_LINE_MAX * QPE_WORDS_ROW_MAX);
    ::memset(m_anAnswerLine, 0, sizeof(int) * QM_MAX_ANSWER);

    g_pRenderText->SetFont(g_hFont);

    wchar_t szAnswer[2 * QPE_WORDS_ROW_MAX];
    const wchar_t* pszAnswer;
    int nPlayerWordsRow = 0;
    int i;
    for (i = 0; i < QM_MAX_ANSWER; ++i)
    {
        ::swprintf(szAnswer, L"%d.", i + 1);
        pszAnswer = g_QuestMng.GetAnswer(m_dwCurQuestIndex, i);
        if (NULL == pszAnswer)
            break;
        ::wcscat(szAnswer, pszAnswer);

        m_anAnswerLine[i] = ::DivideStringByPixel(&m_aszPlayerWords[nPlayerWordsRow][0], 2, QPE_WORDS_ROW_MAX, szAnswer, 160, false);

        nPlayerWordsRow += m_anAnswerLine[i];

        if (QPE_PLAYER_LINE_MAX <= nPlayerWordsRow)
            break;
    }
}

void CNewUIQuestProgressByEtc::SetCurRequestReward()
{
    if (0 == m_dwCurQuestIndex)
        return;

    const SQuestRequestReward* pQuestRequestReward
        = g_QuestMng.GetRequestReward(m_dwCurQuestIndex);
    if (NULL == pQuestRequestReward)
        return;

    m_RequestRewardListBox.Clear();

    SRequestRewardText aRequestRewardText[13];
    m_bRequestComplete
        = g_QuestMng.GetRequestRewardText(aRequestRewardText, 13, m_dwCurQuestIndex);

    int i = 0;
    int j, nLoop;

    for (j = 0; j < 3; ++j)
    {
        if (0 == j)
        {
            nLoop = 1 + pQuestRequestReward->m_byRequestCount;
        }
        else if (1 == j && pQuestRequestReward->m_byGeneralRewardCount)
        {
            m_RequestRewardListBox.AddText(g_hFont, 0xffffffff, RT3_SORT_LEFT, L" ");
            nLoop = 1 + pQuestRequestReward->m_byGeneralRewardCount + i;
        }
        else if (2 == j && pQuestRequestReward->m_byRandRewardCount)
        {
            m_RequestRewardListBox.AddText(g_hFont, 0xffffffff, RT3_SORT_LEFT, L" ");
            nLoop = 1 + pQuestRequestReward->m_byRandRewardCount + i;
        }
        else
            nLoop = 0;

        for (; i < nLoop; ++i)
            m_RequestRewardListBox.AddText(&aRequestRewardText[i], RT3_SORT_CENTER);
    }

    EnableCompleteBtn(m_bRequestComplete);
}

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