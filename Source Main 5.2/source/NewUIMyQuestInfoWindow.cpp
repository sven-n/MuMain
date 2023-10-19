// NewUIMyQuestInfoWindow.cpp: implementation of the CNewUIMyQuestInfoWindow class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIMyQuestInfoWindow.h"

#include "csQuest.h"
#include "DSPlaySound.h"
#include "NewUISystem.h"
#include "QuestMng.h"

using namespace SEASON3B;

extern int g_iNumLineMessageBoxCustom;
extern int g_iNumAnswer;
extern wchar_t g_lpszMessageBoxCustom[NUM_LINE_CMB][MAX_LENGTH_CMB];

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
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_MYQUEST, this);

    SetPos(x, y);
    LoadImages();
    SetButtonInfo();
    m_eTabBtnIndex = TAB_QUEST;
    Show(false);

    return true;
}

void SEASON3B::CNewUIMyQuestInfoWindow::Release()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void SEASON3B::CNewUIMyQuestInfoWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;

    m_BtnExit.ChangeButtonInfo(m_Pos.x + 13, m_Pos.y + 392, 36, 29);
    m_btnQuestOpen.ChangeButtonInfo(m_Pos.x + 50, m_Pos.y + 392, 36, 29);
    m_btnQuestGiveUp.ChangeButtonInfo(m_Pos.x + 87, m_Pos.y + 392, 36, 29);

    m_CurQuestListBox.SetPosition(m_Pos.x + 9, m_Pos.y + 160);
    m_QuestContentsListBox.SetPosition(m_Pos.x + 9, m_Pos.y + 390);
}

bool SEASON3B::CNewUIMyQuestInfoWindow::UpdateMouseEvent()
{
    if (m_eTabBtnIndex == TAB_QUEST)
    {
        m_CurQuestListBox.DoAction();
        m_QuestContentsListBox.DoAction();
    }

    if (BtnProcess() == true)
    {
        return false;
    }

    if (CheckMouseIn(m_Pos.x, m_Pos.y, MYQUESTINFO_WINDOW_WIDTH, MYQUESTINFO_WINDOW_HEIGHT))
    {
        return false;
    }

    return true;
}

bool SEASON3B::CNewUIMyQuestInfoWindow::BtnProcess()
{
    POINT ptExitBtn1 = { m_Pos.x + 169, m_Pos.y + 7 };

    if (SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_MYQUEST);
        return true;
    }

    if (m_BtnExit.UpdateMouseEvent() == true)
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_MYQUEST);
        return true;
    }

    TAB_BUTTON_INDEX eTabBtnIndex = UpdateTabBtn();
    if (eTabBtnIndex == TAB_QUEST)
    {
        if (0 == m_CurQuestListBox.GetLineNum())
            SetMessage(2825);
        return true;
    }

    if (eTabBtnIndex == TAB_JOB_CHANGE)
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

    if (eTabBtnIndex == TAB_CASTLE_TEMPLE)
    {
        SocketClient->ToGameServer()->SendMiniGameEventCountRequest(2);
        SocketClient->ToGameServer()->SendMiniGameEventCountRequest(3);
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
    return false;
}

bool SEASON3B::CNewUIMyQuestInfoWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MYQUEST) == true)
    {
        if (SEASON3B::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_MYQUEST);
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
    RenderFrame();
    RenderTabBtn();
    RenderSubjectTexts();
    m_BtnExit.Render();

    if (m_eTabBtnIndex == TAB_QUEST)
    {
        RenderQuestInfo();
    }
    else if (m_eTabBtnIndex == TAB_JOB_CHANGE)
    {
        RenderImage(IMAGE_MYQUEST_LINE, m_Pos.x, m_Pos.y + 182, 188.f, 21.f);
        RenderJobChangeContents();
        RenderJobChangeState();
    }
    else if (m_eTabBtnIndex == TAB_CASTLE_TEMPLE)
    {
        RenderImage(IMAGE_MYQUEST_LINE, m_Pos.x, m_Pos.y + 210, 188.f, 21.f);
        RenderCastleInfo();
        RenderTempleInfo();
    }

    DisableAlphaBlend();

    return true;
}

float SEASON3B::CNewUIMyQuestInfoWindow::GetLayerDepth()
{
    return 3.3f;
}

void SEASON3B::CNewUIMyQuestInfoWindow::LoadImages()
{
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_MYQUEST_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back01.tga", IMAGE_MYQUEST_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-L.tga", IMAGE_MYQUEST_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-R.tga", IMAGE_MYQUEST_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_MYQUEST_BOTTOM, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_exit_00.tga", IMAGE_MYQUEST_BTN_EXIT, GL_LINEAR);

    LoadBitmap(L"Interface\\newui_myquest_Line.tga", IMAGE_MYQUEST_LINE, GL_LINEAR);
    LoadBitmap(L"Interface\\Quest_Bt_open.tga", IMAGE_MYQUEST_BTN_OPEN, GL_LINEAR);
    LoadBitmap(L"Interface\\Quest_Bt_cast.tga", IMAGE_MYQUEST_BTN_GIVE_UP, GL_LINEAR);
    LoadBitmap(L"Interface\\Quest_tab01.tga", IMAGE_MYQUEST_TAB_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\Quest_tab02.tga", IMAGE_MYQUEST_TAB_SMALL, GL_LINEAR);
    LoadBitmap(L"Interface\\Quest_tab03.tga", IMAGE_MYQUEST_TAB_BIG, GL_LINEAR);
}

void SEASON3B::CNewUIMyQuestInfoWindow::UnloadImages()
{
    DeleteBitmap(IMAGE_MYQUEST_TAB_BIG);
    DeleteBitmap(IMAGE_MYQUEST_TAB_SMALL);
    DeleteBitmap(IMAGE_MYQUEST_TAB_BACK);
    DeleteBitmap(IMAGE_MYQUEST_BTN_GIVE_UP);
    DeleteBitmap(IMAGE_MYQUEST_BTN_OPEN);
    DeleteBitmap(IMAGE_MYQUEST_LINE);
    DeleteBitmap(IMAGE_MYQUEST_BTN_EXIT);
    DeleteBitmap(IMAGE_MYQUEST_BOTTOM);
    DeleteBitmap(IMAGE_MYQUEST_RIGHT);
    DeleteBitmap(IMAGE_MYQUEST_LEFT);
    DeleteBitmap(IMAGE_MYQUEST_TOP);
    DeleteBitmap(IMAGE_MYQUEST_BACK);
}

void SEASON3B::CNewUIMyQuestInfoWindow::RenderFrame()
{
    RenderImage(IMAGE_MYQUEST_BACK, m_Pos.x, m_Pos.y, 190.f, 429.f);
    RenderImage(IMAGE_MYQUEST_TOP, m_Pos.x, m_Pos.y, 190.f, 64.f);
    RenderImage(IMAGE_MYQUEST_LEFT, m_Pos.x, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_MYQUEST_RIGHT, m_Pos.x + 190 - 21, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_MYQUEST_BOTTOM, m_Pos.x, m_Pos.y + 429 - 45, 190.f, 45.f);
}

void SEASON3B::CNewUIMyQuestInfoWindow::RenderSubjectTexts()
{
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(230, 230, 230, 255);
    g_pRenderText->SetBgColor(0);
    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 12, L"Quest", 190, 0, RT3_SORT_CENTER);
}

void SEASON3B::CNewUIMyQuestInfoWindow::RenderQuestInfo()
{
    RenderImage(IMAGE_MYQUEST_LINE, m_Pos.x, m_Pos.y + 160, 188.f, 21.f);

    if (0 == m_CurQuestListBox.GetLineNum())
    {
        g_pRenderText->SetFont(g_hFontBold);
        g_pRenderText->SetTextColor(255, 255, 0, 255);
        g_pRenderText->SetBgColor(0);
        int i;
        for (i = 0; i < m_nMsgLine; ++i)
            g_pRenderText->RenderText(m_Pos.x + 23, m_Pos.y + 96 + 18 * i,
                m_aszMsg[i], 0, 0, RT3_SORT_LEFT);
    }
    else
        m_CurQuestListBox.Render();

    m_btnQuestOpen.Render();
    m_btnQuestGiveUp.Render();

    m_QuestContentsListBox.Render();
}

void SEASON3B::CNewUIMyQuestInfoWindow::RenderJobChangeContents()
{
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(36, 242, 252, 255);
    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 58, g_csQuest.getQuestTitleWindow(), 190, 0, RT3_SORT_CENTER);

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetBgColor(0);

    int iY = m_Pos.y + 76;
    for (int i = 0; i < g_iNumLineMessageBoxCustom; ++i)
    {
        g_pRenderText->RenderText(m_Pos.x, iY, g_lpszMessageBoxCustom[i], 190.f, 0.f, RT3_SORT_CENTER);
        iY += 16;
    }
}

void SEASON3B::CNewUIMyQuestInfoWindow::RenderJobChangeState()
{
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(255, 255, 0, 255);
    g_pRenderText->SetBgColor(0);

    BYTE byState = g_csQuest.getCurrQuestState();
    if (byState == QUEST_NONE || byState == QUEST_NO || byState == QUEST_ERROR)
        SetMessage(930);
    else if (byState == QUEST_ING)
        SetMessage(931);
    else if (byState == QUEST_END)
        SetMessage(932);

    int i;
    for (i = 0; i < m_nMsgLine; ++i)
        g_pRenderText->RenderText(m_Pos.x + 23, m_Pos.y + 283 + 18 * i, m_aszMsg[i], 0, 0, RT3_SORT_LEFT);
}

void SEASON3B::CNewUIMyQuestInfoWindow::RenderCastleInfo()
{
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(255, 255, 0, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 0);

    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 105, GlobalText[56], 190, 0, RT3_SORT_CENTER);

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 0);

    wchar_t strText[256];
    swprintf(strText, GlobalText[868], g_csQuest.GetEventCount(2));
    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 125, strText, 190, 0, RT3_SORT_CENTER);

    swprintf(strText, GlobalText[829], 6);
    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 145, strText, 190, 0, RT3_SORT_CENTER);
}

void SEASON3B::CNewUIMyQuestInfoWindow::RenderTempleInfo()
{
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(255, 255, 0, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 0);

    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 285, GlobalText[2369], 190, 0, RT3_SORT_CENTER);

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 0);

    wchar_t strText[256];
    swprintf(strText, GlobalText[868], g_csQuest.GetEventCount(3));
    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 305, strText, 190, 0, RT3_SORT_CENTER);

    swprintf(strText, GlobalText[829], 6);
    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 325, strText, 190, 0, RT3_SORT_CENTER);
}

void SEASON3B::CNewUIMyQuestInfoWindow::OpenningProcess()
{
    g_csQuest.ShowQuestPreviewWindow(-1);
}

void SEASON3B::CNewUIMyQuestInfoWindow::ClosingProcess()
{
    UnselectQuestList();
    SocketClient->ToGameServer()->SendCloseNpcRequest();
    ::PlayBuffer(SOUND_CLICK01);
}

void SEASON3B::CNewUIMyQuestInfoWindow::SetButtonInfo()
{
    m_BtnExit.ChangeButtonImgState(true, IMAGE_MYQUEST_BTN_EXIT, false);
    m_BtnExit.ChangeButtonInfo(m_Pos.x + 13, m_Pos.y + 392, 36, 29);
    m_BtnExit.ChangeToolTipText(GlobalText[1002], true);

    m_btnQuestOpen.ChangeButtonImgState(true, IMAGE_MYQUEST_BTN_OPEN, false);
    m_btnQuestOpen.ChangeButtonInfo(m_Pos.x + 50, m_Pos.y + 392, 36, 29);
    m_btnQuestOpen.ChangeToolTipText(GlobalText[2822], true);

    m_btnQuestGiveUp.ChangeButtonImgState(true, IMAGE_MYQUEST_BTN_GIVE_UP, false);
    m_btnQuestGiveUp.ChangeButtonInfo(m_Pos.x + 87, m_Pos.y + 392, 36, 29);
    m_btnQuestGiveUp.ChangeToolTipText(GlobalText[2823], true);
}

CNewUIMyQuestInfoWindow::TAB_BUTTON_INDEX CNewUIMyQuestInfoWindow::UpdateTabBtn()
{
    if (!(SEASON3B::IsPress(VK_LBUTTON)))
        return TAB_NON;

    if (!CheckMouseIn(m_Pos.x + 10, m_Pos.y + 27, 166, 22))
        return TAB_NON;

    if (CheckMouseIn(m_Pos.x + 10, m_Pos.y + 27, 48, 22))
        m_eTabBtnIndex = TAB_QUEST;
    else if (CheckMouseIn(m_Pos.x + 57, m_Pos.y + 27, 48, 22))
        m_eTabBtnIndex = TAB_JOB_CHANGE;
    else if (CheckMouseIn(m_Pos.x + 104, m_Pos.y + 27, 72, 22))
        m_eTabBtnIndex = TAB_CASTLE_TEMPLE;

    ::PlayBuffer(SOUND_CLICK01);

    return m_eTabBtnIndex;
}

void CNewUIMyQuestInfoWindow::RenderTabBtn()
{
    RenderImage(IMAGE_MYQUEST_TAB_BACK, m_Pos.x + 10, m_Pos.y + 27, 166.f, 22.f);

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetBgColor(0);

    if (m_eTabBtnIndex == TAB_QUEST)
    {
        RenderImage(IMAGE_MYQUEST_TAB_SMALL, m_Pos.x + 10, m_Pos.y + 27, 48.f, 22.f);
        g_pRenderText->SetTextColor(255, 255, 255, 255);
        g_pRenderText->RenderText(m_Pos.x + 10, m_Pos.y + 34, GlobalText[1140], 48, 0, RT3_SORT_CENTER);
        g_pRenderText->SetTextColor(181, 181, 181, 181);
        g_pRenderText->RenderText(m_Pos.x + 57, m_Pos.y + 35, GlobalText[2821], 48, 0, RT3_SORT_CENTER);
        g_pRenderText->RenderText(m_Pos.x + 104, m_Pos.y + 35, GlobalText[2824], 72, 0, RT3_SORT_CENTER);
    }
    else if (m_eTabBtnIndex == TAB_JOB_CHANGE)
    {
        RenderImage(IMAGE_MYQUEST_TAB_SMALL, m_Pos.x + 57, m_Pos.y + 27, 48.f, 22.f);
        g_pRenderText->SetTextColor(255, 255, 255, 255);
        g_pRenderText->RenderText(m_Pos.x + 57, m_Pos.y + 34, GlobalText[2821], 48, 0, RT3_SORT_CENTER);
        g_pRenderText->SetTextColor(181, 181, 181, 181);
        g_pRenderText->RenderText(m_Pos.x + 10, m_Pos.y + 35, GlobalText[1140], 48, 0, RT3_SORT_CENTER);
        g_pRenderText->RenderText(m_Pos.x + 104, m_Pos.y + 35, GlobalText[2824], 72, 0, RT3_SORT_CENTER);
    }
    else if (m_eTabBtnIndex == TAB_CASTLE_TEMPLE)
    {
        RenderImage(IMAGE_MYQUEST_TAB_BIG, m_Pos.x + 104, m_Pos.y + 27, 72.f, 22.f);
        g_pRenderText->SetTextColor(255, 255, 255, 255);
        g_pRenderText->RenderText(m_Pos.x + 104, m_Pos.y + 34, GlobalText[2824], 72, 0, RT3_SORT_CENTER);
        g_pRenderText->SetTextColor(181, 181, 181, 181);
        g_pRenderText->RenderText(m_Pos.x + 10, m_Pos.y + 35, GlobalText[1140], 48, 0, RT3_SORT_CENTER);
        g_pRenderText->RenderText(m_Pos.x + 57, m_Pos.y + 35, GlobalText[2821], 48, 0, RT3_SORT_CENTER);
    }
}

void CNewUIMyQuestInfoWindow::UnselectQuestList()
{
    m_CurQuestListBox.SLSetSelectLine(0);
    m_QuestContentsListBox.Clear();
    QuestOpenBtnEnable(false);
    QuestGiveUpBtnEnable(false);
}

void CNewUIMyQuestInfoWindow::SetCurQuestList(DWordList* pDWordList)
{
    m_CurQuestListBox.Clear();

    wchar_t szInput[64];
    wchar_t szOutput[64];
    g_pRenderText->SetFont(g_hFont);

    int i;
    DWordList::iterator iter;
    for (iter = pDWordList->begin(), i = 1; iter != pDWordList->end(); advance(iter, 1), ++i)
    {
        ::swprintf(szInput, L"%d.%s", i, g_QuestMng.GetSubject(*iter));
        ::ReduceStringByPixel(szOutput, 64, szInput, 150);
        m_CurQuestListBox.AddText(*iter, szOutput);
    }

    if (m_eTabBtnIndex == TAB_QUEST && 0 == m_CurQuestListBox.GetLineNum())
        SetMessage(2825);

    m_QuestContentsListBox.Clear();
    QuestOpenBtnEnable(false);
    QuestGiveUpBtnEnable(false);
}

void CNewUIMyQuestInfoWindow::SetSelQuestSummary()
{
    m_QuestContentsListBox.Clear();

    DWORD dwSelQuestIndex = GetSelQuestIndex();

    if (0 == dwSelQuestIndex)
        return;

    m_QuestContentsListBox.AddText(
        g_hFontBold, 0xff0ab9ff, RT3_SORT_CENTER, g_QuestMng.GetSubject(dwSelQuestIndex));

    g_pRenderText->SetFont(g_hFont);
    wchar_t aszSummary[8][64];
    int nLine = ::DivideStringByPixel(
        &aszSummary[0][0], 8, 64, g_QuestMng.GetSummary(dwSelQuestIndex), 150);
    int i;
    for (i = 0; i < nLine; ++i)
        m_QuestContentsListBox.AddText(g_hFont, 0xffd2e6ff, RT3_SORT_LEFT, aszSummary[i]);
}

void CNewUIMyQuestInfoWindow::SetSelQuestRequestReward()
{
    DWORD dwSelQuestIndex = GetSelQuestIndex();

    if (0 == dwSelQuestIndex)
        return;

    if (!g_QuestMng.IsRequestRewardQS(dwSelQuestIndex))
        return;

    const SQuestRequestReward* pQuestRequestReward = g_QuestMng.GetRequestReward(dwSelQuestIndex);
    if (NULL == pQuestRequestReward)
        return;

    SRequestRewardText aRequestRewardText[13];
    g_QuestMng.GetRequestRewardText(aRequestRewardText, 13, dwSelQuestIndex);

    int i = 0;
    int j, nLoop;
    for (j = 0; j < 3; ++j)
    {
        if (0 == j)
        {
            m_QuestContentsListBox.AddText(g_hFont, 0xffffffff, RT3_SORT_LEFT, L" ");
            nLoop = 1 + pQuestRequestReward->m_byRequestCount;
        }
        else if (1 == j && pQuestRequestReward->m_byGeneralRewardCount)
        {
            m_QuestContentsListBox.AddText(g_hFont, 0xffffffff, RT3_SORT_LEFT, L" ");
            nLoop = 1 + pQuestRequestReward->m_byGeneralRewardCount + i;
        }
        else if (2 == j && pQuestRequestReward->m_byRandRewardCount)
        {
            m_QuestContentsListBox.AddText(g_hFont, 0xffffffff, RT3_SORT_LEFT, L" ");
            nLoop = 1 + pQuestRequestReward->m_byRandRewardCount + i;
        }
        else
            nLoop = 0;

        for (; i < nLoop; ++i)
            m_QuestContentsListBox.AddText(&aRequestRewardText[i], RT3_SORT_CENTER);
    }
}

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

DWORD CNewUIMyQuestInfoWindow::GetSelQuestIndex()
{
    SCurQuestItem* pCurQuestItem = m_CurQuestListBox.GetSelectedText();
    if (NULL == pCurQuestItem)
        return 0;

    return pCurQuestItem->m_dwIndex;
}

void CNewUIMyQuestInfoWindow::SetMessage(int nGlobalTextIndex)
{
    memset(m_aszMsg, 0, sizeof m_aszMsg);
    g_pRenderText->SetFont(g_hFontBold);
    m_nMsgLine = ::DivideStringByPixel(&m_aszMsg[0][0], 2, 64, GlobalText[nGlobalTextIndex], 140);
}