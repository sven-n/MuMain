// NewUICursedTempleResult.cpp: implementation of the CNewUICursedTempleResult class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUICursedTempleResult.h"
#include "NewUICommonMessageBox.h"
#include "UIBaseDef.h"
#include "DSPlaySound.h"
#include "ZzzInfomation.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"

#include "CSItemOption.h"
#include "CSChaosCastle.h"
#include "UIControls.h"
#include "SkillManager.h"
#include "CharacterManager.h"
#include "NewUISystem.h"

using namespace SEASON3B;

namespace
{
    void DrawText(wchar_t* text, int textposx, int textposy, DWORD textcolor, DWORD textbackcolor, int textsort, float fontboxwidth, bool isbold)
    {
        if (isbold)
        {
            g_pRenderText->SetFont(g_hFontBold);
        }
        else
        {
            g_pRenderText->SetFont(g_hFont);
        }

        DWORD backuptextcolor = g_pRenderText->GetTextColor();
        DWORD backuptextbackcolor = g_pRenderText->GetBgColor();

        g_pRenderText->SetTextColor(textcolor);
        g_pRenderText->SetBgColor(textbackcolor);
        g_pRenderText->RenderText(textposx, textposy, text, fontboxwidth, 0, textsort);
        g_pRenderText->SetTextColor(backuptextcolor);
        g_pRenderText->SetBgColor(backuptextbackcolor);
    }
};

bool SEASON3B::CNewUICursedTempleResult::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_CURSEDTEMPLE_RESULT, this);

    SetPos(x, y);

    SetButtonInfo();

    Show(false);

    return true;
}

SEASON3B::CNewUICursedTempleResult::CNewUICursedTempleResult() : m_pNewUIMng(NULL), m_ResultEffectAlph(0.f), m_WinState(0)
{
    Initialize();
}

SEASON3B::CNewUICursedTempleResult::~CNewUICursedTempleResult()
{
    Destroy();
}

void SEASON3B::CNewUICursedTempleResult::Initialize()
{
    LoadImages();
}

void SEASON3B::CNewUICursedTempleResult::Destroy()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void SEASON3B::CNewUICursedTempleResult::LoadImages()
{
    LoadBitmap(L"Interface\\illusion_success.tga", IMAGE_CURSEDTEMPLERESULT_SUCCESS, GL_LINEAR);
    LoadBitmap(L"Interface\\illusion_failure.tga", IMAGE_CURSEDTEMPLERESULT_FAILURE, GL_LINEAR);
}

void SEASON3B::CNewUICursedTempleResult::UnloadImages()
{
    DeleteBitmap(IMAGE_CURSEDTEMPLERESULT_FAILURE);
    DeleteBitmap(IMAGE_CURSEDTEMPLERESULT_SUCCESS);
}

void SEASON3B::CNewUICursedTempleResult::SetButtonInfo()
{
    float x;
    x = m_Pos.x + (CURSEDTEMPLE_RESULT_WINDOW_WIDTH / 2) - (54 / 2);

    m_Button[CURSEDTEMPLERESULT_CLOSE].ChangeButtonImgState(true, CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_VERY_SMALL, true);
    m_Button[CURSEDTEMPLERESULT_CLOSE].ChangeButtonInfo(x, m_Pos.y + CURSEDTEMPLE_RESULT_WINDOW_HEIGHT - 37, 54, 23);
    m_Button[CURSEDTEMPLERESULT_CLOSE].ChangeText(GlobalText[1002]);
}

void SEASON3B::CNewUICursedTempleResult::ResetGameResultInfo()
{
    m_WinState = 0;

    m_ResultEffectAlph = 0.f;

    m_MyTeam = SEASON3A::eTeam_Count;

    if (m_AlliedTeamGameResult.size() != 0)
        m_AlliedTeamGameResult.clear();

    if (m_IllusionTeamGameResult.size() != 0)
        m_IllusionTeamGameResult.clear();
}

void SEASON3B::CNewUICursedTempleResult::UpdateResult()
{
    if (m_WinState == 0)
        return;

    m_ResultEffectAlph += 0.015f;
    if (1.0f < m_ResultEffectAlph)
    {
        m_ResultEffectAlph = 1.0f;
    }
}

void SEASON3B::CNewUICursedTempleResult::OpenningProcess()
{
}

void SEASON3B::CNewUICursedTempleResult::ClosingProcess()
{
    SocketClient->ToGameServer()->SendIllusionTempleRewardRequest();
    ResetGameResultInfo();
}

bool SEASON3B::CNewUICursedTempleResult::UpdateMouseEvent()
{
    if (m_Button[CURSEDTEMPLERESULT_CLOSE].UpdateMouseEvent())
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_CURSEDTEMPLE_RESULT);
        return false;
    }

    if (CheckMouseIn(m_Pos.x, m_Pos.y, CURSEDTEMPLE_RESULT_WINDOW_WIDTH, CURSEDTEMPLE_RESULT_WINDOW_HEIGHT))
    {
        return false;
    }

    return true;
}

bool SEASON3B::CNewUICursedTempleResult::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CURSEDTEMPLE_RESULT) == true)
    {
        if (SEASON3B::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_CURSEDTEMPLE_RESULT);
            return false;
        }
    }

    return true;
}

bool SEASON3B::CNewUICursedTempleResult::Update()
{
    UpdateResult();

    return true;
}

void SEASON3B::CNewUICursedTempleResult::RenderResultPanel()
{
    if (m_WinState == 0)
        return;

    ::glColor4f(1.0f, 1.0f, 1.0f, m_ResultEffectAlph);

    if (m_WinState == 1)
    {
        RenderImage(IMAGE_CURSEDTEMPLERESULT_SUCCESS, (640 - 360) / 2, m_Pos.y - 110, 351.f, 115.f);
    }
    else
    {
        RenderImage(IMAGE_CURSEDTEMPLERESULT_FAILURE, (640 - 360) / 2, m_Pos.y - 110, 351.f, 115.f);
    }

    glColor4f(1.f, 1.f, 1.f, 1.f);
}

void SEASON3B::CNewUICursedTempleResult::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = CURSEDTEMPLE_RESULT_WINDOW_WIDTH - MSGBOX_BACK_BLANK_WIDTH; height = CURSEDTEMPLE_RESULT_WINDOW_HEIGHT - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP_TITLEBAR, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < 11; ++i)
    {
        RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);

    x = GetPos().x; y = GetPos().y + CURSEDTEMPLE_RESULT_WINDOW_HEIGHT - 77; width = MSGBOX_LINE_WIDTH; height = MSGBOX_LINE_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_LINE, x, y, width, height);
    y = GetPos().y + 45;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_LINE, x, y, width, height);
}

void SEASON3B::CNewUICursedTempleResult::RenderButtons()
{
    m_Button[CURSEDTEMPLERESULT_CLOSE].Render();
}

void SEASON3B::CNewUICursedTempleResult::RenderTextLine(const CursedTempleGameResult& resultinfo, int x, int y, DWORD color, DWORD backcolor)
{
    wchar_t Text[200];

    memset(&Text, 0, sizeof(wchar_t) * 200);
    if (SEASON3A::eTeam_Allied == resultinfo.s_team)
    {
        swprintf(Text, GlobalText[2387]);
    }
    else
    {
        swprintf(Text, GlobalText[2388]);
    }
    DrawText(Text, x + 5, y, color, backcolor, RT3_SORT_LEFT, 0, false);

    memset(&Text, 0, sizeof(wchar_t) * 200);
    swprintf(Text, resultinfo.s_characterId);
    DrawText(Text, x + 56, y, color, backcolor, RT3_SORT_LEFT, 0, false);

    memset(&Text, 0, sizeof(wchar_t) * 200);
    swprintf(Text, gCharacterManager.GetCharacterClassText(resultinfo.s_class));
    DrawText(Text, x + 106, y, color, backcolor, RT3_SORT_LEFT, 0, false);

    memset(&Text, 0, sizeof(wchar_t) * 200);
    swprintf(Text, L"%d", resultinfo.s_addexp);
    DrawText(Text, x + 150, y, color, backcolor, RT3_SORT_LEFT, 0, false);

    memset(&Text, 0, sizeof(wchar_t) * 200);
    swprintf(Text, L"%d", resultinfo.s_point);
    DrawText(Text, x + 190, y, color, backcolor, RT3_SORT_LEFT, 0, false);
}

void SEASON3B::CNewUICursedTempleResult::RenderText()
{
    wchar_t Text[200];

    memset(&Text, 0, sizeof(wchar_t) * 200);
    swprintf(Text, GlobalText[2414]);
    DrawText(Text, m_Pos.x, m_Pos.y + 13, 0xFF49B0FF, 0x00000000, RT3_SORT_CENTER, CURSEDTEMPLE_RESULT_WINDOW_WIDTH, false);

    memset(&Text, 0, sizeof(wchar_t) * 200);
    swprintf(Text, L"  %s           %s        %s     %s    %s", GlobalText[2415], GlobalText[681], GlobalText[1973], GlobalText[683], GlobalText[682]);
    DrawText(Text, m_Pos.x, m_Pos.y + 38, 0xFF49B0FF, 0x00000000, RT3_SORT_CENTER, CURSEDTEMPLE_RESULT_WINDOW_WIDTH, false);

    int i = 0;
    for (auto iter = m_AlliedTeamGameResult.begin(); iter != m_AlliedTeamGameResult.end(); )
    {
        auto curiter = iter;
        ++iter;
        CursedTempleGameResult& info = *curiter;

        if (wcscmp(info.s_characterId, Hero->ID) == 0)
        {
            DrawText(L" ", m_Pos.x + 2, m_Pos.y + 65 + (i * 15), 0xFFFBB264, 0xff0000ff, RT3_SORT_LEFT, 220.f, false);
        }

        RenderTextLine(info, m_Pos.x, m_Pos.y + 65 + (i * 15), 0xFFFBB264, 0x00000000);
        ++i;
    }

    i = 0;
    for (auto emiter = m_IllusionTeamGameResult.begin(); emiter != m_IllusionTeamGameResult.end(); )
    {
        auto curiter = emiter;
        ++emiter;
        CursedTempleGameResult& info = *curiter;

        if (wcscmp(info.s_characterId, Hero->ID) == 0)
        {
            DrawText(L" ", m_Pos.x + 2, m_Pos.y + 140 + (i * 15), 0xFF37d6fe, 0xff0000ff, RT3_SORT_LEFT, 220.f, false);
        }

        RenderTextLine(info, m_Pos.x, m_Pos.y + 140 + (i * 15), 0xFF37d6fe, 0x00000000);
        ++i;
    }

    memset(&Text, 0, sizeof(wchar_t) * 200);
    swprintf(Text, GlobalText[2416]);
    DrawText(Text, m_Pos.x, m_Pos.y + CURSEDTEMPLE_RESULT_WINDOW_HEIGHT - 55, 0xFF0000FF, 0x00000000, RT3_SORT_CENTER, CURSEDTEMPLE_RESULT_WINDOW_WIDTH, false);
}

bool SEASON3B::CNewUICursedTempleResult::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    RenderFrame();
    RenderResultPanel();
    RenderText();
    RenderButtons();

    DisableAlphaBlend();

    return true;
}

void SEASON3B::CNewUICursedTempleResult::ReceiveCursedTempleGameResult(const BYTE* ReceiveBuffer)
{
    auto data = (LPPMSG_CURSED_TEMPLE_RESULT)ReceiveBuffer;

    int	alliedPoint = data->btAlliedPoint;
    int	illusionPoint = data->btIllusionPoint;
    int userCount = data->btUserCount;

    if (m_MyTeam == SEASON3A::eTeam_Allied)
    {
        if (illusionPoint < alliedPoint)
            m_WinState = 1;
        else
            m_WinState = 2;

        if (2 > alliedPoint) m_WinState = 2;
    }
    else
    {
        if (alliedPoint < illusionPoint)
            m_WinState = 1;
        else
            m_WinState = 2;

        if (2 > illusionPoint) m_WinState = 2;
    }

    int Offset = sizeof(PMSG_CURSED_TEMPLE_RESULT);

    for (int i = 0; i < userCount; i++)
    {
        auto data2 = (LPPMSG_CURSED_TEMPLE_USER_ADD_EXP)(ReceiveBuffer + Offset);

        CursedTempleGameResult TempData{};
        CMultiLanguage::ConvertFromUtf8(TempData.s_characterId, data2->GameId, MAX_ID_SIZE);

        TempData.s_mapnumber = (short)data2->byMapNumber;

        TempData.s_team = static_cast<SEASON3A::eCursedTempleTeam>(data2->btTeam);
        TempData.s_class = gCharacterManager.ChangeServerClassTypeToClientClassType(data2->btClass);
        TempData.s_addexp = data2->nAddExp;

        if (TempData.s_team == SEASON3A::eTeam_Allied)
        {
            TempData.s_point = alliedPoint;
            m_AlliedTeamGameResult.push_back(TempData);
        }
        else
        {
            TempData.s_point = illusionPoint;
            m_IllusionTeamGameResult.push_back(TempData);
        }

        Offset += sizeof(PMSG_CURSED_TEMPLE_USER_ADD_EXP);
    }
}