// NewUICursedTempleEnter.cpp: implementation of the CNewUICursedTempleEnter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "NewUICursedTempleEnter.h"
#include "NewUICommonMessageBox.h"
#include "UIBaseDef.h"
#include "DSPlaySound.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"

#include "CharacterManager.h"
#include "CSItemOption.h"
#include "CSChaosCastle.h"
#include "NewUISystem.h"
#include "UIControls.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace SEASON3B;

namespace
{
    const int EnterLevelCount = 5;
    const int EnterMinLevel[EnterLevelCount] = { 220, 271, 321, 351, 381 };
    const int EnterMaxLevel[EnterLevelCount] = { 270, 320, 350, 380, 400 };

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
}

bool SEASON3B::CNewUICursedTempleEnter::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_CURSEDTEMPLE_NPC, this);

    SetPos(x, y);

    SetButtonInfo();

    Show(false);

    return true;
}

SEASON3B::CNewUICursedTempleEnter::CNewUICursedTempleEnter() : m_pNewUIMng(NULL), m_EnterTime(0), m_EnterCount(0)
{
    Initialize();
}

SEASON3B::CNewUICursedTempleEnter::~CNewUICursedTempleEnter()
{
    Destroy();
}

void SEASON3B::CNewUICursedTempleEnter::Initialize()
{
}

void SEASON3B::CNewUICursedTempleEnter::Destroy()
{
    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void SEASON3B::CNewUICursedTempleEnter::SetButtonInfo()
{
    float x;
    x = m_Pos.x + (((CURSEDTEMPLE_ENTER_WINDOW_WIDTH / 2) - MSGBOX_BTN_WIDTH) / 2);
    m_Button[CURSEDTEMPLEENTER_OPEN].ChangeButtonImgState(true, CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_VERY_SMALL, true);

    m_Button[CURSEDTEMPLEENTER_OPEN].ChangeButtonInfo(x, m_Pos.y + 203, 54, 23);

    // 2147 "입장하기"
    m_Button[CURSEDTEMPLEENTER_OPEN].ChangeText(GlobalText[2147]);

    x = m_Pos.x + (CURSEDTEMPLE_ENTER_WINDOW_WIDTH / 2) + (((CURSEDTEMPLE_ENTER_WINDOW_WIDTH / 2) - MSGBOX_BTN_WIDTH) / 2);
    m_Button[CURSEDTEMPLEENTER_EXIT].ChangeButtonImgState(true, CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_VERY_SMALL, true);

    m_Button[CURSEDTEMPLEENTER_EXIT].ChangeButtonInfo(x, m_Pos.y + 203, 54, 23);
    // 1002 "닫기"
    m_Button[CURSEDTEMPLEENTER_EXIT].ChangeText(GlobalText[1002]);
}

bool SEASON3B::CNewUICursedTempleEnter::CheckEnterLevel(int& enterlevel)
{
    if (gCharacterManager.IsMasterLevel(Hero->Class) == true)
    {
        enterlevel = 6;
        return true;
    }

    int HeroLevel = CharacterAttribute->Level;

    for (int i = 0; i < EnterLevelCount; ++i)
    {
        if (HeroLevel >= EnterMinLevel[i] && HeroLevel <= EnterMaxLevel[i])
        {
            enterlevel = i + 1;
            return true;
        }
    }

    return false;
}

bool SEASON3B::CNewUICursedTempleEnter::CheckEnterItem(ITEM* p, int enterlevel)
{
    if (p->Type == ITEM_HELPER + 61)
    {
        if (!CheckEnterLevel(enterlevel)) return false;
    }
    else
    {
        if (p->Type != ITEM_SCROLL_OF_BLOOD)
            return false;

        int itemLevel = p->Level;

        if (itemLevel != enterlevel)
            return false;
    }

    if (p->Durability < 1) return false;

    return true;
}

bool SEASON3B::CNewUICursedTempleEnter::CheckInventory(BYTE& itempos, int enterlevel)
{
    int pos = 0;

    if (enterlevel == -1) {
        return false;
    }

    pos = g_pMyInventory->GetInventoryCtrl()->FindItemIndex(ITEM_SCROLL_OF_BLOOD, enterlevel);
    if (pos != -1) {
        itempos = pos;
        return true;
    }

    pos = g_pMyInventory->GetInventoryCtrl()->FindItemIndex(ITEM_HELPER + 61, -1);
    if (pos != -1) {
        itempos = pos;
        return true;
    }
    return false;
}

bool SEASON3B::CNewUICursedTempleEnter::UpdateMouseEvent()
{
    if (m_Button[CURSEDTEMPLEENTER_OPEN].UpdateMouseEvent())
    {
        int  EnterLevel = -1;
        bool Result = false;

        // CheckHeroLevl
        Result = CheckEnterLevel(EnterLevel);

        if (Result)
        {
            SocketClient->ToGameServer()->SendIllusionTempleEnterRequest(static_cast<BYTE>(EnterLevel), 0xFF);
        }
        else
        {
            g_pSystemLogBox->AddText(GlobalText[2367], SEASON3B::TYPE_ERROR_MESSAGE);
        }

        return false;
    }

    if (m_Button[CURSEDTEMPLEENTER_EXIT].UpdateMouseEvent())
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_CURSEDTEMPLE_NPC);
        return false;
    }

    if (CheckMouseIn(m_Pos.x, m_Pos.y, CURSEDTEMPLE_ENTER_WINDOW_WIDTH, CURSEDTEMPLE_ENTER_WINDOW_HEIGHT))
    {
        return false;
    }

    return true;
}

bool SEASON3B::CNewUICursedTempleEnter::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CURSEDTEMPLE_NPC) == true)
    {
        if (SEASON3B::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_CURSEDTEMPLE_NPC);
            return false;
        }
    }

    return true;
}

bool SEASON3B::CNewUICursedTempleEnter::Update()
{
    return true;
}

void SEASON3B::CNewUICursedTempleEnter::RenderText()
{
    wchar_t Text[100];

    memset(&Text, 0, sizeof(wchar_t));

    swprintf(Text, GlobalText[2358]);
    DrawText(Text, m_Pos.x, m_Pos.y + 13, 0xFF49B0FF, 0x00000000, RT3_SORT_CENTER, CURSEDTEMPLE_ENTER_WINDOW_WIDTH, true);

    int enterlevel = -1;

    if (CheckEnterLevel(enterlevel))
    {
        memset(&Text, 0, sizeof(Text));

        swprintf(Text, GlobalText[2370], enterlevel);
        DrawText(Text, m_Pos.x + 3, m_Pos.y + 42, 0xffffffff, 0x00000000, RT3_SORT_CENTER, CURSEDTEMPLE_ENTER_WINDOW_WIDTH - 10, false);

        for (int i = 0; i < EnterLevelCount + 1; ++i)
        {
            memset(&Text, 0, sizeof(Text));

            if (i == 5)
            {
                wcscpy(Text, GlobalText[737]);
            }
            else
            {
                swprintf(Text, GlobalText[2371], EnterMinLevel[i], EnterMaxLevel[i]);
            }

            if (enterlevel == i + 1)
            {
                DisableAlphaBlend();
                swprintf(Text, L"%s %s", Text, GlobalText[2412]);
                DrawText(Text, m_Pos.x + 3, m_Pos.y + 67 + (i * 15), 0xffffffff, 0xff0000ff, RT3_SORT_CENTER, CURSEDTEMPLE_ENTER_WINDOW_WIDTH - 10, false);
                EnableAlphaTest();
            }
            else
            {
                swprintf(Text, L"%s %s", Text, GlobalText[2413]);
                DrawText(Text, m_Pos.x + 3, m_Pos.y + 67 + (i * 15), 0xffffffff, 0x00000000, RT3_SORT_CENTER, CURSEDTEMPLE_ENTER_WINDOW_WIDTH - 10, false);
            }
        }

        memset(&Text, 0, sizeof(char));
        swprintf(Text, GlobalText[2373], m_EnterCount);
        DrawText(Text, m_Pos.x + 3, m_Pos.y + 70 + ((EnterLevelCount + 1) * 15), 0xff0000ff, 0x00000000, RT3_SORT_CENTER, CURSEDTEMPLE_ENTER_WINDOW_WIDTH - 10, false);
    }
    else
    {
        memset(&Text, 0, sizeof(char));
        swprintf(Text, GlobalText[2366]);
        DrawText(Text, m_Pos.x, m_Pos.y + 52, 0xff0000ff, 0x00000000, RT3_SORT_CENTER, CURSEDTEMPLE_ENTER_WINDOW_WIDTH, false);
    }
}

void SEASON3B::CNewUICursedTempleEnter::RenderFrame()
{
    float x, y, width, height;

    x = GetPos().x; y = GetPos().y + 2.f, width = CURSEDTEMPLE_ENTER_WINDOW_WIDTH - MSGBOX_BACK_BLANK_WIDTH; height = CURSEDTEMPLE_ENTER_WINDOW_HEIGHT - MSGBOX_BACK_BLANK_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, width, height);

    x = GetPos().x; y = GetPos().y, width = MSGBOX_WIDTH; height = MSGBOX_TOP_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP_TITLEBAR, x, y, width, height);

    x = GetPos().x; y += MSGBOX_TOP_HEIGHT; width = MSGBOX_WIDTH; height = MSGBOX_MIDDLE_HEIGHT;
    for (int i = 0; i < 9; ++i)
    {
        RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, width, height);
        y += height;
    }

    x = GetPos().x; width = MSGBOX_WIDTH; height = MSGBOX_BOTTOM_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, width, height);

    x = GetPos().x; y = GetPos().y + CURSEDTEMPLE_ENTER_WINDOW_HEIGHT - 77; width = MSGBOX_LINE_WIDTH; height = MSGBOX_LINE_HEIGHT;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_LINE, x, y, width, height);
}

bool SEASON3B::CNewUICursedTempleEnter::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    RenderFrame();
    RenderText();
    RenderButtons();

    DisableAlphaBlend();

    return true;
}

void SEASON3B::CNewUICursedTempleEnter::RenderButtons()
{
    for (int i = 0; i < CURSEDTEMPLEENTER_MAXBUTTONCOUNT; ++i)
    {
        // 버튼 렌더링
        m_Button[i].Render();
    }
}

//ServerMessage
void SEASON3B::CNewUICursedTempleEnter::SetCursedTempleEnterInfo(const BYTE* cursedtempleinfo)
{
    m_EnterTime = static_cast<int>(cursedtempleinfo[0]);
    m_EnterCount = static_cast<int>(cursedtempleinfo[1]);
}

void SEASON3B::CNewUICursedTempleEnter::ReceiveCursedTempleEnterInfo(const BYTE* ReceiveBuffer)
{
    auto data = (LPPMSG_CURSED_TEMPLE_USER_COUNT)ReceiveBuffer;

    int enterlevel = -1;

    if (CheckEnterLevel(enterlevel))
    {
        if (enterlevel > 0)
        {
            m_EnterCount = data->btUserCount[enterlevel - 1];
        }
    }
}