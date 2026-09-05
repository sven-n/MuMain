// NewUICursedTempleEnter.cpp: implementation of the CNewUICursedTempleEnter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "I18N/All.h"

#include "UI/Events/NewUICursedTempleEnter.h"
#include "UI/Dialogs/NewUICommonMessageBox.h"
#include "UI/Widgets/UIBaseDef.h"
#include "Audio/DSPlaySound.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzInventory.h"

#include "Character/CharacterManager.h"
#include "GameLogic/Items/CSItemOption.h"
#include "GameLogic/Events/CSChaosCastle.h"
#include "UI/Core/NewUISystem.h"
#include "UI/Widgets/UIControls.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace SEASON3B;
using namespace mu::ui::window;

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

bool mu::ui::window::CNewUICursedTempleEnter::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_CURSEDTEMPLE_NPC, this);

    SetPos(x, y);

    SetButtonInfo();

    Show(false);

    return true;
}

mu::ui::window::CNewUICursedTempleEnter::CNewUICursedTempleEnter() : m_pNewUIMng(NULL), m_EnterTime(0), m_EnterCount(0)
{
    Initialize();
}

mu::ui::window::CNewUICursedTempleEnter::~CNewUICursedTempleEnter()
{
    Destroy();
}

void mu::ui::window::CNewUICursedTempleEnter::Initialize()
{
}

void mu::ui::window::CNewUICursedTempleEnter::Destroy()
{
    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void mu::ui::window::CNewUICursedTempleEnter::SetButtonInfo()
{
    float x;
    x = m_Pos.x + (((CURSEDTEMPLE_ENTER_WINDOW_WIDTH / 2) - MSGBOX_BTN_WIDTH) / 2);
    m_Button[CURSEDTEMPLEENTER_OPEN].ChangeButtonImgState(true, CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_VERY_SMALL, true);

    m_Button[CURSEDTEMPLEENTER_OPEN].ChangeButtonInfo(x, m_Pos.y + 203, 54, 23);

    // 2147 "입장하기"
    m_Button[CURSEDTEMPLEENTER_OPEN].ChangeText(&I18N::Game::Enter);

    x = m_Pos.x + (CURSEDTEMPLE_ENTER_WINDOW_WIDTH / 2) + (((CURSEDTEMPLE_ENTER_WINDOW_WIDTH / 2) - MSGBOX_BTN_WIDTH) / 2);
    m_Button[CURSEDTEMPLEENTER_EXIT].ChangeButtonImgState(true, CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_VERY_SMALL, true);

    m_Button[CURSEDTEMPLEENTER_EXIT].ChangeButtonInfo(x, m_Pos.y + 203, 54, 23);
    // 1002 "닫기"
    m_Button[CURSEDTEMPLEENTER_EXIT].ChangeText(&I18N::Game::Close388);
}

bool mu::ui::window::CNewUICursedTempleEnter::CheckEnterLevel(int& enterlevel)
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

bool mu::ui::window::CNewUICursedTempleEnter::CheckEnterItem(ITEM* p, int enterlevel)
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

bool mu::ui::window::CNewUICursedTempleEnter::CheckInventory(BYTE& itempos, int enterlevel)
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

bool mu::ui::window::CNewUICursedTempleEnter::UpdateMouseEvent()
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
            g_pSystemLogBox->AddText(I18N::Game::TheAdmissionAndScrollLevelsDoNotMatch, mu::ui::window::TYPE_ERROR_MESSAGE);
        }

        return false;
    }

    if (m_Button[CURSEDTEMPLEENTER_EXIT].UpdateMouseEvent())
    {
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_CURSEDTEMPLE_NPC);
        return false;
    }

    if (CheckMouseIn(m_Pos.x, m_Pos.y, CURSEDTEMPLE_ENTER_WINDOW_WIDTH, CURSEDTEMPLE_ENTER_WINDOW_HEIGHT))
    {
        return false;
    }

    return true;
}

bool mu::ui::window::CNewUICursedTempleEnter::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_CURSEDTEMPLE_NPC) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_CURSEDTEMPLE_NPC);
            return false;
        }
    }

    return true;
}

bool mu::ui::window::CNewUICursedTempleEnter::Update()
{
    return true;
}

void mu::ui::window::CNewUICursedTempleEnter::RenderText()
{
    wchar_t Text[100];

    memset(&Text, 0, sizeof(wchar_t));

    mu_swprintf(Text, I18N::Game::DoYouWishToGoToTheIllusionTemple);
    DrawText(Text, m_Pos.x, m_Pos.y + 13, 0xFF49B0FF, 0x00000000, RT3_SORT_CENTER, CURSEDTEMPLE_ENTER_WINDOW_WIDTH, true);

    int enterlevel = -1;

    if (CheckEnterLevel(enterlevel))
    {
        memset(&Text, 0, sizeof(Text));

        mu_swprintf(Text, I18N::Game::TheDIllusionTemple, enterlevel);
        DrawText(Text, m_Pos.x + 3, m_Pos.y + 42, 0xffffffff, 0x00000000, RT3_SORT_CENTER, CURSEDTEMPLE_ENTER_WINDOW_WIDTH - 10, false);

        for (int i = 0; i < EnterLevelCount + 1; ++i)
        {
            memset(&Text, 0, sizeof(Text));

            if (i == 5)
            {
                wcscpy(Text, I18N::Game::MasterLevel);
            }
            else
            {
                mu_swprintf(Text, I18N::Game::LevelDD, EnterMinLevel[i], EnterMaxLevel[i]);
            }

            if (enterlevel == i + 1)
            {
                DisableAlphaBlend();
                mu_swprintf(Text, L"%ls %ls", Text, I18N::Game::EntranceEnabled);
                DrawText(Text, m_Pos.x + 3, m_Pos.y + 67 + (i * 15), 0xffffffff, 0xff0000ff, RT3_SORT_CENTER, CURSEDTEMPLE_ENTER_WINDOW_WIDTH - 10, false);
                EnableAlphaTest();
            }
            else
            {
                mu_swprintf(Text, L"%ls %ls", Text, I18N::Game::EntranceDisabled);
                DrawText(Text, m_Pos.x + 3, m_Pos.y + 67 + (i * 15), 0xffffffff, 0x00000000, RT3_SORT_CENTER, CURSEDTEMPLE_ENTER_WINDOW_WIDTH - 10, false);
            }
        }

        memset(&Text, 0, sizeof(char));
        mu_swprintf(Text, I18N::Game::CurrentMembersD, m_EnterCount);
        DrawText(Text, m_Pos.x + 3, m_Pos.y + 70 + ((EnterLevelCount + 1) * 15), 0xff0000ff, 0x00000000, RT3_SORT_CENTER, CURSEDTEMPLE_ENTER_WINDOW_WIDTH - 10, false);
    }
    else
    {
        memset(&Text, 0, sizeof(char));
        mu_swprintf(Text, I18N::Game::YouMustBeOfTheMinimumLevel220ToEnterTheZone);
        DrawText(Text, m_Pos.x, m_Pos.y + 52, 0xff0000ff, 0x00000000, RT3_SORT_CENTER, CURSEDTEMPLE_ENTER_WINDOW_WIDTH, false);
    }
}

void mu::ui::window::CNewUICursedTempleEnter::RenderFrame()
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

bool mu::ui::window::CNewUICursedTempleEnter::Render()
{
    EnableAlphaTest();

    RenderFrame();
    RenderText();
    RenderButtons();

    DisableAlphaBlend();

    return true;
}

void mu::ui::window::CNewUICursedTempleEnter::RenderButtons()
{
    for (int i = 0; i < CURSEDTEMPLEENTER_MAXBUTTONCOUNT; ++i)
    {
        // 버튼 렌더링
        m_Button[i].Render();
    }
}

//ServerMessage
void mu::ui::window::CNewUICursedTempleEnter::SetCursedTempleEnterInfo(const BYTE* cursedtempleinfo)
{
    m_EnterTime = static_cast<int>(cursedtempleinfo[0]);
    m_EnterCount = static_cast<int>(cursedtempleinfo[1]);
}

void mu::ui::window::CNewUICursedTempleEnter::ReceiveCursedTempleEnterInfo(const BYTE* ReceiveBuffer)
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
