//*****************************************************************************
// File: NewUINPCQuest.cpp
//*****************************************************************************

#include "stdafx.h"
#include "NewUINPCQuest.h"
#include "NewUISystem.h"
#include "CSQuest.h"

#include "CharacterManager.h"
#include "DSPlaySound.h"

extern bool bCheckNPC;
extern int g_iNumLineMessageBoxCustom;
extern int g_iNumAnswer;
extern wchar_t g_lpszMessageBoxCustom[NUM_LINE_CMB][MAX_LENGTH_CMB];
extern wchar_t g_lpszDialogAnswer[MAX_ANSWER_FOR_DIALOG][NUM_LINE_DA][MAX_LENGTH_CMB];
extern int g_iCurrentDialogScript;

using namespace SEASON3B;

CNewUINPCQuest::CNewUINPCQuest()
{
    m_pNewUIMng = NULL;
    m_pNewUI3DRenderMng = NULL;
    m_Pos.x = m_Pos.y = 0;
}

CNewUINPCQuest::~CNewUINPCQuest()
{
    Release();
}

bool CNewUINPCQuest::Create(CNewUIManager* pNewUIMng,
    CNewUI3DRenderMng* pNewUI3DRenderMng, int x, int y)
{
    if (NULL == pNewUIMng || NULL == pNewUI3DRenderMng
        || NULL == g_pNewItemMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_NPCQUEST, this);

    m_pNewUI3DRenderMng = pNewUI3DRenderMng;
    m_pNewUI3DRenderMng->Add3DRenderObj(this, INVENTORY_CAMERA_Z_ORDER);

    SetPos(x, y);

    LoadImages();

    m_btnComplete.ChangeText(GlobalText[699]);
    m_btnComplete.ChangeButtonImgState(true, IMAGE_NPCQUEST_BTN_COMPLETE, true);
    m_btnComplete.ChangeButtonInfo(x + 41, y + 355, 108, 29);

    m_btnClose.ChangeButtonImgState(true, IMAGE_NPCQUEST_BTN_CLOSE);
    m_btnClose.ChangeButtonInfo(x + 13, y + 392, 36, 29);
    m_btnClose.ChangeToolTipText(GlobalText[1002], true);

    Show(false);

    return true;
}

void CNewUINPCQuest::Release()
{
    UnloadImages();

    if (m_pNewUI3DRenderMng)
    {
        m_pNewUI3DRenderMng->Remove3DRenderObj(this);
        m_pNewUI3DRenderMng = NULL;
    }

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CNewUINPCQuest::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CNewUINPCQuest::UpdateMouseEvent()
{
    if (ProcessBtns())
        return false;

    if (UpdateSelTextMouseEvent())
        return false;

    if (CheckMouseIn(m_Pos.x, m_Pos.y, NPCQUEST_WIDTH, NPCQUEST_HEIGHT))
        return false;

    return true;
}

bool CNewUINPCQuest::UpdateSelTextMouseEvent()
{
    BYTE byCurQuestIndex = g_csQuest.GetCurrQuestIndex();
    BYTE byCurQuestState = g_csQuest.getQuestState2(int(byCurQuestIndex));

    bool bErrorMessage = false;
    int iButtonPush = -1;
    int iTotalLine = g_iNumLineMessageBoxCustom + g_iNumAnswer;
    int yPos = m_Pos.y + 66 + (NUM_LINE_CMB - iTotalLine) * 18 / 2;

    yPos += 18 * g_iNumLineMessageBoxCustom;

    if (byCurQuestState != QUEST_ING)
    {
        yPos = m_Pos.y + 250;
    }

    if (SEASON3B::IsRelease(VK_LBUTTON))
    {
        if (MouseY >= 0 && (MouseY - yPos) < (18 * g_iNumAnswer)
            && abs(m_Pos.x + NPCQUEST_WIDTH / 2 - MouseX) <= (NPCQUEST_WIDTH / 2))
        {
            iButtonPush = (MouseY - yPos) / 18;

            if (iButtonPush >= 0)
            {
                int nAnswer
                    = g_DialogScript[g_iCurrentDialogScript].m_iReturnForAnswer[iButtonPush];

                if (1 == nAnswer)
                    bErrorMessage = g_csQuest.ProcessNextProgress();
                else if (2 == nAnswer)
                    g_pNewUISystem->Hide(SEASON3B::INTERFACE_NPCQUEST);
                else if (3 == nAnswer)
                    SocketClient->ToGameServer()->SendLegacyQuestStateSetRequest(byCurQuestIndex, 1);

                ::PlayBuffer(SOUND_INTERFACE01);

                int nNextDialogIndex
                    = g_DialogScript[g_iCurrentDialogScript].m_iLinkForAnswer[iButtonPush];
                if (0 < nNextDialogIndex && !bErrorMessage)
                    g_csQuest.ShowDialogText(nNextDialogIndex);

                return false;
            }
        }
    }

    return false;
}

bool CNewUINPCQuest::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCQUEST) == true)
    {
        if (SEASON3B::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_NPCQUEST);
            PlayBuffer(SOUND_CLICK01);

            return false;
        }
    }

    return true;
}

bool CNewUINPCQuest::Update()
{
    return true;
}

bool CNewUINPCQuest::Render()
{
    ::EnableAlphaTest();

    ::glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    RenderBackImage();

    BYTE byCurQuestIndex = g_csQuest.GetCurrQuestIndex();
    BYTE byCurQuestState = g_csQuest.getQuestState2(int(byCurQuestIndex));

    if (QUEST_ING == byCurQuestState)
    {
        RenderImage(IMAGE_NPCQUEST_LINE, m_Pos.x + 1, m_Pos.y + 325, 188.f, 21.f);

        if (RenderItemMobText())
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

        m_btnComplete.Render();
    }
    else if (QUEST_NO == byCurQuestState)
    {
        // ÇÊ¿ä Á¨.
        RenderImage(IMAGE_NPCQUEST_ZEN, m_Pos.x + 11, m_Pos.y + 361, 170.f, 24.f);

        g_pRenderText->SetFont(g_hFontBold);
        g_pRenderText->SetBgColor(0);

        g_pRenderText->SetTextColor(255, 220, 150, 255);
        g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + 368, GlobalText[198]);

        wchar_t szTemp[128];
        g_pRenderText->SetTextColor(::getGoldColor(g_csQuest.GetNeedZen()));
        ::ConvertGold(g_csQuest.GetNeedZen(), szTemp);
        g_pRenderText->RenderText(m_Pos.x + 170, m_Pos.y + 368, szTemp, 0, 0, RT3_WRITE_RIGHT_TO_LEFT);
    }

    RenderText();

    m_btnClose.Render();

    ::DisableAlphaBlend();

    return true;
}

void CNewUINPCQuest::RenderBackImage()
{
    RenderImage(IMAGE_NPCQUEST_BACK,
        m_Pos.x, m_Pos.y, float(NPCQUEST_WIDTH), float(NPCQUEST_HEIGHT));
    RenderImage(IMAGE_NPCQUEST_TOP,
        m_Pos.x, m_Pos.y, float(NPCQUEST_WIDTH), 64.f);
    RenderImage(IMAGE_NPCQUEST_LEFT,
        m_Pos.x, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_NPCQUEST_RIGHT,
        m_Pos.x + NPCQUEST_WIDTH - 21, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_NPCQUEST_BOTTOM,
        m_Pos.x, m_Pos.y + NPCQUEST_HEIGHT - 45, float(NPCQUEST_WIDTH), 45.f);

    RenderImage(IMAGE_NPCQUEST_LINE, m_Pos.x + 1, m_Pos.y + 220, 188.f, 21.f);
}

void CNewUINPCQuest::RenderText()
{
    BYTE byCurQuestIndex = g_csQuest.GetCurrQuestIndex();
    BYTE byCurQuestState = g_csQuest.getQuestState2(int(byCurQuestIndex));

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetBgColor(0);

    g_pRenderText->SetTextColor(150, 255, 240, 255);
    //const auto name = new wchar_t[MAX_MONSTER_NAME];
    wchar_t* name = nullptr;

    if ((Hero->Class == CLASS_DARK_LORD || Hero->Class == CLASS_DARK
        || Hero->Class == CLASS_RAGEFIGHTER)
        && bCheckNPC)
    {
        name = g_csQuest.GetNPCName(2);
    }
    else
    {
        name = g_csQuest.GetNPCName(byCurQuestIndex);
    }

    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 16, name, NPCQUEST_WIDTH, 0, RT3_SORT_CENTER);
    g_pRenderText->SetTextColor(200, 220, 255, 255);
    if ((Hero->Class != CLASS_DARK_LORD && Hero->Class != CLASS_DARK
        && Hero->Class != CLASS_RAGEFIGHTER)
        || !bCheckNPC)
        g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 29,
            g_csQuest.getQuestTitle(), NPCQUEST_WIDTH, 0, RT3_SORT_CENTER);

    g_pRenderText->SetTextColor(255, 230, 210, 255);
    int iTotalLine = g_iNumLineMessageBoxCustom + g_iNumAnswer;
    int xPos = m_Pos.x + NPCQUEST_WIDTH / 2;
    int yPos = m_Pos.y + 66 + (NUM_LINE_CMB - iTotalLine) * 18 / 2;
    for (int j = 0; j < g_iNumLineMessageBoxCustom; ++j)
    {
        g_pRenderText->RenderText(xPos, yPos, g_lpszMessageBoxCustom[j], 0, 0,
            RT3_WRITE_CENTER);
        yPos += 18;
    }

    if (byCurQuestState != QUEST_ING)
        yPos = m_Pos.y + 250;

    int iButtonOn = (MouseY - yPos) / 18;

    g_pRenderText->SetFont(g_hFontBold);

    for (int j = 0; j < g_iNumAnswer; ++j)
    {
        if (iButtonOn == j && abs(m_Pos.x + NPCQUEST_WIDTH / 2 - MouseX) <= NPCQUEST_WIDTH / 2)
            g_pRenderText->SetTextColor(255, 0, 0, 255);
        else
            g_pRenderText->SetTextColor(223, 191, 103, 255);

        for (int k = 0; k < NUM_LINE_DA && g_lpszDialogAnswer[j][k][0]; ++k)
        {
            g_pRenderText->RenderText(xPos, yPos, g_lpszDialogAnswer[j][k],
                0, 0, RT3_WRITE_CENTER);
            yPos += 18;
        }
    }
}

bool CNewUINPCQuest::RenderItemMobText()
{
    bool bCompletion = true;

    wchar_t szTemp[128];
    int nPosY = m_Pos.y + 244;

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetBgColor(0);

    QUEST_ATTRIBUTE* pQuest = g_csQuest.GetCurQuestAttribute();
    int nClass = gCharacterManager.GetBaseClass(Hero->Class);

    for (int i = 0; i < pQuest->shQuestConditionNum; ++i)
    {
        if (!pQuest->QuestAct[i].byRequestClass[nClass])
            continue;

        switch (pQuest->QuestAct[i].byQuestType)
        {
        case QUEST_ITEM:
        {
            int nItemType = (pQuest->QuestAct[i].wItemType * MAX_ITEM_INDEX)
                + pQuest->QuestAct[i].byItemSubType;
            int nItemNum = pQuest->QuestAct[i].byItemNum;
            int nItemLevel = pQuest->QuestAct[i].byItemLevel;

            if (!g_csQuest.FindQuestItemsInInven(nItemType, nItemNum, nItemLevel))
                g_pRenderText->SetTextColor(223, 191, 103, 255);
            else
            {
                g_pRenderText->SetTextColor(255, 30, 30, 255);
                bCompletion = false;
            }

            wchar_t szItemName[128];
            GetItemName(nItemType, nItemLevel, szItemName);
            swprintf(szTemp, L"%s x %d", szItemName, nItemNum);
            g_pRenderText->RenderText(m_Pos.x + 60, nPosY, szTemp);
        }
        break;

        case QUEST_MONSTER:
        {
            int nKillMobCount
                = g_csQuest.GetKillMobCount(int(pQuest->QuestAct[i].wItemType));

            if (int(pQuest->QuestAct[i].byItemNum) <= nKillMobCount)
            {
                g_pRenderText->SetTextColor(223, 191, 103, 255);
                nKillMobCount = int(pQuest->QuestAct[i].byItemNum);
            }
            else
            {
                g_pRenderText->SetTextColor(255, 30, 30, 255);
                bCompletion = false;
            }

            auto name = getMonsterName(int(pQuest->QuestAct[i].wItemType));
            swprintf(szTemp, L"%s x %d/%d", name, nKillMobCount, int(pQuest->QuestAct[i].byItemNum));

            g_pRenderText->RenderText(m_Pos.x + 50, nPosY, szTemp);
        }
        break;
        }

        nPosY += 32;
    }

    return bCompletion;
}

void CNewUINPCQuest::RenderItem3D()
{
    BYTE byCurQuestIndex = g_csQuest.GetCurrQuestIndex();
    BYTE byCurQuestState = g_csQuest.getQuestState2(int(byCurQuestIndex));

    if (QUEST_ING != byCurQuestState)
        return;

    auto x = float(m_Pos.x + 30);
    auto y = float(m_Pos.y + 235);
    const float Height = 27.f;

    QUEST_ATTRIBUTE* pQuest = g_csQuest.GetCurQuestAttribute();
    int nClass = gCharacterManager.GetBaseClass(Hero->Class);

    for (int i = 0; i < pQuest->shQuestConditionNum; ++i)
    {
        if (!pQuest->QuestAct[i].byRequestClass[nClass])
            continue;

        if (QUEST_ITEM == pQuest->QuestAct[i].byQuestType)
        {
            int nItemType = (pQuest->QuestAct[i].wItemType * MAX_ITEM_INDEX)
                + pQuest->QuestAct[i].byItemSubType;

            int nItemLevel = pQuest->QuestAct[i].byItemLevel;

            ::RenderItem3D(x, y, 20.f, Height, nItemType, nItemLevel, 0, 0, false);

            y += Height + 5.f;
        }
    }
}

void CNewUINPCQuest::Render3D()
{
    RenderItem3D();
}

bool CNewUINPCQuest::IsVisible() const
{
    return CNewUIObj::IsVisible();
}

float CNewUINPCQuest::GetLayerDepth()
{
    return 3.1f;
}

void CNewUINPCQuest::LoadImages()
{
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_NPCQUEST_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back04.tga", IMAGE_NPCQUEST_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-L.tga", IMAGE_NPCQUEST_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-R.tga", IMAGE_NPCQUEST_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_NPCQUEST_BOTTOM, GL_LINEAR);

    LoadBitmap(L"Interface\\newui_myquest_Line.tga", IMAGE_NPCQUEST_LINE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_money2.tga", IMAGE_NPCQUEST_ZEN, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_btn_empty.tga", IMAGE_NPCQUEST_BTN_COMPLETE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_exit_00.tga", IMAGE_NPCQUEST_BTN_CLOSE, GL_LINEAR);
}

void CNewUINPCQuest::UnloadImages()
{
    DeleteBitmap(IMAGE_NPCQUEST_BTN_CLOSE);
    DeleteBitmap(IMAGE_NPCQUEST_BTN_COMPLETE);
    DeleteBitmap(IMAGE_NPCQUEST_ZEN);
    DeleteBitmap(IMAGE_NPCQUEST_LINE);

    DeleteBitmap(IMAGE_NPCQUEST_BOTTOM);
    DeleteBitmap(IMAGE_NPCQUEST_RIGHT);
    DeleteBitmap(IMAGE_NPCQUEST_LEFT);
    DeleteBitmap(IMAGE_NPCQUEST_TOP);
    DeleteBitmap(IMAGE_NPCQUEST_BACK);
}

void CNewUINPCQuest::ProcessOpening()
{
    g_csQuest.ShowQuestNpcWindow();
}

bool CNewUINPCQuest::ProcessClosing()
{
    SocketClient->ToGameServer()->SendCloseNpcRequest();
    return true;
}

bool CNewUINPCQuest::ProcessBtns()
{
    if (m_btnClose.UpdateMouseEvent())
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_NPCQUEST);
        return true;
    }
    else if (SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(m_Pos.x + 169, m_Pos.y + 7, 13, 12))
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_NPCQUEST);
        return true;
    }
    else if (g_csQuest.BeQuestItem())
    {
        if (m_btnComplete.UpdateMouseEvent())
        {
            SocketClient->ToGameServer()->SendLegacyQuestStateSetRequest(g_csQuest.GetCurrQuestIndex(), 1);
            PlayBuffer(SOUND_INTERFACE01);
            return true;
        }
    }

    return false;
}