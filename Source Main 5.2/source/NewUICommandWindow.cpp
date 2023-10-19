// NewUICommandWindow.cpp: implementation of the CNewUICommandWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "NewUICommandWindow.h"

#include "DSPlaySound.h"
#include "NewUISystem.h"
#include "ZzzInterface.h"
#include "PersonalShopTitleImp.h"
#include "UIGuildInfo.h"
#include "ZzzAI.h"
#include "w_CursedTemple.h"
#include "DuelMgr.h"
#include "MapManager.h"

using namespace SEASON3B;

SEASON3B::CNewUICommandWindow::CNewUICommandWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_iCurSelectCommand = COMMAND_NONE;
    m_iCurMouseCursor = CURSOR_NORMAL;
    m_bSelectedChar = false;
    m_bCanCommand = false;
}

SEASON3B::CNewUICommandWindow::~CNewUICommandWindow()
{
    Release();
}

bool SEASON3B::CNewUICommandWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_COMMAND, this);

    SetPos(x, y);

    LoadImages();

    InitButtons();

    Show(false);

    return true;
}

void SEASON3B::CNewUICommandWindow::Release()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void SEASON3B::CNewUICommandWindow::InitButtons()
{
    wchar_t szText[256] = { NULL, };
    swprintf(szText, GlobalText[927], L"D");

    m_BtnExit.ChangeButtonImgState(true, IMAGE_COMMAND_BASE_WINDOW_BTN_EXIT);
    m_BtnExit.ChangeButtonInfo(m_Pos.x + 13, m_Pos.y + 392, 36, 29);
    m_BtnExit.ChangeToolTipText(szText, true);

    for (int i = COMMAND_TRADE; i < COMMAND_END; i++)
    {
        m_BtnCommand[i].ChangeButtonImgState(true, IMAGE_COMMAND_BTN, true);
        m_BtnCommand[i].ChangeButtonInfo(m_Pos.x + (COMMAND_WINDOW_WIDTH / 2 - 108 / 2), (m_Pos.y + 33) + (i * (29 + COMMAND_BTN_INTERVAL_SIZE)), 108, 29);
    }

    m_BtnCommand[COMMAND_TRADE].ChangeText(GlobalText[943]);
    m_BtnCommand[COMMAND_PURCHASE].ChangeText(GlobalText[1124]);
    m_BtnCommand[COMMAND_PARTY].ChangeText(GlobalText[944]);
    m_BtnCommand[COMMAND_WHISPER].ChangeText(GlobalText[945]);
    m_BtnCommand[COMMAND_GUILD].ChangeText(GlobalText[946]);
    m_BtnCommand[COMMAND_GUILDUNION].ChangeText(GlobalText[1352]);
    m_BtnCommand[COMMAND_RIVAL].ChangeText(GlobalText[1321]);
    m_BtnCommand[COMMAND_RIVALOFF].ChangeText(GlobalText[1322]);
    m_BtnCommand[COMMAND_ADD_FRIEND].ChangeText(GlobalText[947]);
    m_BtnCommand[COMMAND_FOLLOW].ChangeText(GlobalText[948]);
    m_BtnCommand[COMMAND_BATTLE].ChangeText(GlobalText[949]);
}

void SEASON3B::CNewUICommandWindow::OpenningProcess()
{
    if (m_iCurSelectCommand != COMMAND_NONE)
        SetBtnState(m_iCurSelectCommand, false);

    m_iCurSelectCommand = COMMAND_NONE;
    m_iCurMouseCursor = CURSOR_NORMAL;
}

void SEASON3B::CNewUICommandWindow::ClosingProcess()
{
    if (m_iCurSelectCommand != COMMAND_NONE)
        SetBtnState(m_iCurSelectCommand, false);

    m_iCurSelectCommand = COMMAND_NONE;
    m_iCurMouseCursor = CURSOR_NORMAL;
}

bool SEASON3B::CNewUICommandWindow::BtnProcess()
{
    POINT ptExitBtn1 = { m_Pos.x + 169, m_Pos.y + 7 };

    if (SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_COMMAND);
        PlayBuffer(SOUND_CLICK01);
        return true;
    }

    if (m_BtnExit.UpdateMouseEvent() == true)
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_COMMAND);
        PlayBuffer(SOUND_CLICK01);
        return true;
    }

    for (int i = COMMAND_TRADE; i < COMMAND_END; i++)
    {
        if (m_BtnCommand[i].UpdateMouseEvent() == true)
        {
            if (m_iCurSelectCommand != COMMAND_NONE)
                SetBtnState(m_iCurSelectCommand, false);

            if (g_CursedTemple->GetInterfaceState(static_cast<int>(SEASON3B::INTERFACE_COMMAND), i))
            {
                m_iCurSelectCommand = i;
                SetBtnState(m_iCurSelectCommand, true);
            }
        }
    }

    return false;
}

bool SEASON3B::CNewUICommandWindow::UpdateMouseEvent()
{
    if (true == BtnProcess())
        return false;

    if (CheckMouseIn(m_Pos.x, m_Pos.y, COMMAND_WINDOW_WIDTH, COMMAND_WINDOW_HEIGHT))
    {
        SetMouseCursor(CURSOR_NORMAL);
        return false;
    }
    else
    {
        if (m_iCurSelectCommand != COMMAND_NONE)
            SetMouseCursor(CURSOR_IDSELECT);
    }

    return true;
}

bool SEASON3B::CNewUICommandWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_COMMAND) == true)
    {
        if (SEASON3B::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_COMMAND);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }
    return true;
}

bool SEASON3B::CNewUICommandWindow::Update()
{
    if (IsVisible())
    {
        SelectCommand();
        RunCommand();
    }

    return true;
}

bool SEASON3B::CNewUICommandWindow::Render()
{
    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, 1.f);

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 0);

    // Base Window
    RenderBaseWindow();

    for (int i = COMMAND_TRADE; i < COMMAND_END; i++)
    {
        m_BtnCommand[i].SetFont(g_hFont);
        if (m_iCurSelectCommand != COMMAND_NONE)
            m_BtnCommand[m_iCurSelectCommand].SetFont(g_hFontBold);

        m_BtnCommand[i].Render();
    }

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->RenderText(m_Pos.x + 60, m_Pos.y + 12, GlobalText[938], 72, 0, RT3_SORT_CENTER);

    if ((m_iCurMouseCursor == CURSOR_IDSELECT) && (m_bSelectedChar == true))

    {
        CHARACTER* c = &CharactersClient[SelectedCharacter];
        if (c != NULL && c->Object.Kind == KIND_PLAYER && c != Hero && (c->Object.Type == MODEL_PLAYER || c->Change))
        {
            int Width = 128; int Height = 32; int x = (float)MouseX + 5; int y = (float)MouseY + 5;
            RenderBitmap(BITMAP_COMMAND_WINDOW_BEGIN, x, y, Width, Height);

            g_pRenderText->SetFont(g_hFontBig);

            if (m_bCanCommand)
            {
                g_pRenderText->SetTextColor(255, 255, 255, 255);
            }
            else
            {
                g_pRenderText->SetTextColor(255, 0, 0, 255);
            }
            g_pRenderText->SetBgColor(20, 20, 20, 0);
            g_pRenderText->RenderText(MouseX + 5 + 64, MouseY + 12, c->ID, 0, 0, RT3_WRITE_CENTER);

            g_pRenderText->SetFont(g_hFont);
        }
    }

    m_BtnExit.Render();
    DisableAlphaBlend();
    return true;
}

void SEASON3B::CNewUICommandWindow::RenderBaseWindow()
{
    RenderImage(IMAGE_COMMAND_BASE_WINDOW_BACK, m_Pos.x, m_Pos.y, float(COMMAND_WINDOW_WIDTH), float(COMMAND_WINDOW_HEIGHT));
    RenderImage(IMAGE_COMMAND_BASE_WINDOW_TOP, m_Pos.x, m_Pos.y, float(COMMAND_WINDOW_WIDTH), 64.f);
    RenderImage(IMAGE_COMMAND_BASE_WINDOW_LEFT, m_Pos.x, m_Pos.y + 64.f, 21.f, float(COMMAND_WINDOW_HEIGHT) - 64.f - 45.f);
    RenderImage(IMAGE_COMMAND_BASE_WINDOW_RIGHT, m_Pos.x + float(COMMAND_WINDOW_WIDTH) - 21.f, m_Pos.y + 64.f, 21.f, float(COMMAND_WINDOW_HEIGHT) - 64.f - 45.f);
    RenderImage(IMAGE_COMMAND_BASE_WINDOW_BOTTOM, m_Pos.x, m_Pos.y + float(COMMAND_WINDOW_HEIGHT) - 45.f, float(COMMAND_WINDOW_WIDTH), 45.f);
}

void SEASON3B::CNewUICommandWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;

    m_BtnExit.ChangeButtonInfo(m_Pos.x + 13, m_Pos.y + 392, 36, 29);
    for (int i = COMMAND_TRADE; i < COMMAND_END; i++)
    {
        m_BtnCommand[i].ChangeButtonInfo(m_Pos.x + (COMMAND_WINDOW_WIDTH / 2 - 108 / 2), (m_Pos.y + 33) + ((i - 1) * (29 + COMMAND_BTN_INTERVAL_SIZE)), 108, 29);
    }
}

float SEASON3B::CNewUICommandWindow::GetLayerDepth()
{
    return 4.6f;
}

void SEASON3B::CNewUICommandWindow::RunCommand()
{
    if (MouseLButtonPush && m_iCurMouseCursor != CURSOR_IDSELECT)
        SetMouseCursor(CURSOR_PUSH);

    if (m_iCurSelectCommand == COMMAND_NONE)
        return;

    int Selectindex = -1;
    CHARACTER* pSelectedCha = NULL;
    int distX, distY;
    m_bSelectedChar = false;
    m_bCanCommand = false;

    if (SelectedCharacter >= 0)
    {
        pSelectedCha = &CharactersClient[SelectedCharacter];
        m_bSelectedChar = true;
        if (pSelectedCha != NULL)
        {
            distX = abs((pSelectedCha->PositionX) - (Hero->PositionX));
            distY = abs((pSelectedCha->PositionY) - (Hero->PositionY));
            if (pSelectedCha->Object.Kind == KIND_PLAYER && pSelectedCha != Hero && (pSelectedCha->Object.Type == MODEL_PLAYER || pSelectedCha->Change) && (distX <= MAX_DISTANCE_TILE && distY <= MAX_DISTANCE_TILE))
            {
                if ((pSelectedCha->Object.SubType != MODEL_XMAS_EVENT_CHA_DEER) && (pSelectedCha->Object.SubType != MODEL_XMAS_EVENT_CHA_SNOWMAN) && (pSelectedCha->Object.SubType != MODEL_XMAS_EVENT_CHA_SSANTA))
                {
                    Selectindex = SelectedCharacter;
                    m_bCanCommand = true;
                }
            }
        }
    }

    if (MouseRButtonPush)
    {
        MouseRButtonPush = false;
        MouseRButton = false;

        SetMouseCursor(CURSOR_NORMAL);

        if (Selectindex >= 0)
        {
            switch (m_iCurSelectCommand)
            {
            case COMMAND_TRADE:
            {
                CommandTrade(pSelectedCha);
            }break;

            case COMMAND_PURCHASE:
            {
                CommandPurchase(pSelectedCha);
            }break;

            case COMMAND_PARTY:
            {
                CommandParty(pSelectedCha->Key);
            }break;

            case COMMAND_WHISPER:
            {
                CommandWhisper(pSelectedCha);
            }break;

            case COMMAND_GUILD:
            {
                CommandGuild(pSelectedCha);
            }break;

            case COMMAND_GUILDUNION:
            {
                CommandGuildUnion(pSelectedCha);
            }break;

            case COMMAND_RIVAL:
            {
                CommandGuildRival(pSelectedCha);
            }break;

            case COMMAND_RIVALOFF:
            {
                CommandCancelGuildRival(pSelectedCha);
            }break;

            case COMMAND_ADD_FRIEND:
            {
                CommandAddFriend(pSelectedCha);
            }break;

            case COMMAND_FOLLOW:
            {
                CommandFollow(Selectindex);
            }break;

            case COMMAND_BATTLE:
            {
                CommandDual(pSelectedCha);
            }break;
            }
        }
        if (m_iCurSelectCommand != COMMAND_NONE)
            SetBtnState(m_iCurSelectCommand, false);
        m_iCurSelectCommand = COMMAND_NONE;
    }
}

void SEASON3B::CNewUICommandWindow::SetBtnState(int iBtnType, bool bStateDown)
{
    if (bStateDown)
    {
        m_BtnCommand[iBtnType].UnRegisterButtonState();
        m_BtnCommand[iBtnType].RegisterButtonState(BUTTON_STATE_UP, IMAGE_COMMAND_BTN, 2);
        m_BtnCommand[iBtnType].RegisterButtonState(BUTTON_STATE_OVER, IMAGE_COMMAND_BTN, 2);
        m_BtnCommand[iBtnType].RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_COMMAND_BTN, 2);
        m_BtnCommand[iBtnType].ChangeImgIndex(IMAGE_COMMAND_BTN, 2);
    }
    else
    {
        m_BtnCommand[iBtnType].UnRegisterButtonState();
        m_BtnCommand[iBtnType].RegisterButtonState(BUTTON_STATE_UP, IMAGE_COMMAND_BTN, 0);
        m_BtnCommand[iBtnType].RegisterButtonState(BUTTON_STATE_OVER, IMAGE_COMMAND_BTN, 1);
        m_BtnCommand[iBtnType].RegisterButtonState(BUTTON_STATE_DOWN, IMAGE_COMMAND_BTN, 2);
        m_BtnCommand[iBtnType].ChangeImgIndex(IMAGE_COMMAND_BTN, 0);
    }
}

void SEASON3B::CNewUICommandWindow::SelectCommand()
{
}

int SEASON3B::CNewUICommandWindow::GetCurCommandType()
{
    return m_iCurSelectCommand;
}

void SEASON3B::CNewUICommandWindow::SetMouseCursor(int iCursorType)
{
    m_iCurMouseCursor = iCursorType;
}

int SEASON3B::CNewUICommandWindow::GetMouseCursor()
{
    return m_iCurMouseCursor;
}

void SEASON3B::CNewUICommandWindow::LoadImages()
{
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_COMMAND_BASE_WINDOW_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back01.tga", IMAGE_COMMAND_BASE_WINDOW_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-L.tga", IMAGE_COMMAND_BASE_WINDOW_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-R.tga", IMAGE_COMMAND_BASE_WINDOW_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_COMMAND_BASE_WINDOW_BOTTOM, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_exit_00.tga", IMAGE_COMMAND_BASE_WINDOW_BTN_EXIT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_btn_empty.tga", IMAGE_COMMAND_BTN, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_cursorid_wnd.jpg", IMAGE_COMMAND_SELECTID_BG, GL_LINEAR);
}

void SEASON3B::CNewUICommandWindow::UnloadImages()
{
    DeleteBitmap(IMAGE_COMMAND_BASE_WINDOW_BACK);
    DeleteBitmap(IMAGE_COMMAND_BASE_WINDOW_TOP);
    DeleteBitmap(IMAGE_COMMAND_BASE_WINDOW_LEFT);
    DeleteBitmap(IMAGE_COMMAND_BASE_WINDOW_RIGHT);
    DeleteBitmap(IMAGE_COMMAND_BASE_WINDOW_BOTTOM);
    DeleteBitmap(IMAGE_COMMAND_BASE_WINDOW_BTN_EXIT);
    DeleteBitmap(IMAGE_COMMAND_BTN);
    DeleteBitmap(IMAGE_COMMAND_SELECTID_BG);
}

bool SEASON3B::CNewUICommandWindow::CommandTrade(CHARACTER* pSelectedCha)
{
    if (pSelectedCha == NULL)
        return false;

    int level = CharacterAttribute->Level;

    if (level < TRADELIMITLEVEL)
    {
        g_pSystemLogBox->AddText(GlobalText[478], SEASON3B::TYPE_SYSTEM_MESSAGE);
        return false;
    }
    if (IsShopInViewport(pSelectedCha))
    {
        g_pSystemLogBox->AddText(GlobalText[493], SEASON3B::TYPE_ERROR_MESSAGE);
        return false;
    }

    SocketClient->ToGameServer()->SendTradeRequest(pSelectedCha->Key);

    return true;
}

bool SEASON3B::CNewUICommandWindow::CommandPurchase(CHARACTER* pSelectedCha)
{
    if (pSelectedCha == nullptr)
        return false;

    SocketClient->ToGameServer()->SendPlayerShopItemListRequest(pSelectedCha->Key, pSelectedCha->ID);

    return true;
}

bool SEASON3B::CNewUICommandWindow::CommandParty(SHORT iChaKey)
{
    if (PartyNumber > 0 && wcscmp(Party[0].Name, Hero->ID) != NULL)
    {
        g_pSystemLogBox->AddText(GlobalText[257], SEASON3B::TYPE_ERROR_MESSAGE);
        return false;
    }

    SocketClient->ToGameServer()->SendPartyInviteRequest(iChaKey);

    return true;
}

bool SEASON3B::CNewUICommandWindow::CommandWhisper(CHARACTER* pSelectedCha)
{
    g_pChatInputBox->SetWhsprID(pSelectedCha->ID);

    return true;
}

bool SEASON3B::CNewUICommandWindow::CommandGuild(CHARACTER* pSelectedChar)
{
    if (Hero->GuildStatus != G_NONE)
    {
        g_pSystemLogBox->AddText(GlobalText[255], SEASON3B::TYPE_SYSTEM_MESSAGE);
        return false;
    }
    if ((pSelectedChar->GuildMarkIndex < 0) || (pSelectedChar->GuildStatus != G_MASTER))
    {
        g_pSystemLogBox->AddText(GlobalText[507], SEASON3B::TYPE_SYSTEM_MESSAGE);
        return false;
    }

    SocketClient->ToGameServer()->SendGuildJoinRequest(pSelectedChar->Key);

    return true;
}

bool SEASON3B::CNewUICommandWindow::CommandGuildUnion(CHARACTER* pSelectedCha)
{
    if (Hero->GuildStatus != G_MASTER)
    {
        g_pSystemLogBox->AddText(GlobalText[1320], SEASON3B::TYPE_SYSTEM_MESSAGE);
        return false;
    }
    if (pSelectedCha->GuildStatus == G_NONE)
    {
        g_pSystemLogBox->AddText(GlobalText[1385], SEASON3B::TYPE_SYSTEM_MESSAGE);
        return false;
    }
    if (pSelectedCha->GuildStatus != G_MASTER)
    {
        g_pSystemLogBox->AddText(GlobalText[507], SEASON3B::TYPE_SYSTEM_MESSAGE);
        return false;
    }
    if (pSelectedCha->GuildStatus == G_MASTER)
    {
        SocketClient->ToGameServer()->SendGuildRelationshipChangeRequest(0x01, 0x01, pSelectedCha->Key);
        return true;
    }

    return false;
}

bool SEASON3B::CNewUICommandWindow::CommandGuildRival(CHARACTER* pSelectedCha)
{
    if (Hero->GuildStatus != G_MASTER)
    {
        g_pSystemLogBox->AddText(GlobalText[1320], SEASON3B::TYPE_SYSTEM_MESSAGE);
        return false;
    }

    if (pSelectedCha->GuildStatus != G_MASTER)
    {
        g_pSystemLogBox->AddText(GlobalText[507], SEASON3B::TYPE_SYSTEM_MESSAGE);
        return false;
    }

    SocketClient->ToGameServer()->SendGuildRelationshipChangeRequest(0x02, 0x01, pSelectedCha->Key);

    return true;
}

bool SEASON3B::CNewUICommandWindow::CommandCancelGuildRival(CHARACTER* pSelectedCha)
{
    if (Hero->GuildStatus != G_MASTER)
    {
        g_pSystemLogBox->AddText(GlobalText[1320], SEASON3B::TYPE_SYSTEM_MESSAGE);
        return false;
    }
    if (pSelectedCha->GuildStatus != G_MASTER)
    {
        g_pSystemLogBox->AddText(GlobalText[507], SEASON3B::TYPE_SYSTEM_MESSAGE);
        return false;
    }

    SetAction(&Hero->Object, PLAYER_RESPECT1);
    SendRequestAction(Hero->Object, AT_RESPECT1);
    SocketClient->ToGameServer()->SendGuildRelationshipChangeRequest(0x02, 0x02, pSelectedCha->Key);
    return true;
}

bool SEASON3B::CNewUICommandWindow::CommandAddFriend(CHARACTER* pSelectedCha)
{
    if (g_pWindowMgr->IsServerEnable() == TRUE && pSelectedCha != nullptr)
    {
        SocketClient->ToGameServer()->SendFriendAddRequest(pSelectedCha->ID);
        return true;
    }

    return false;
}

bool SEASON3B::CNewUICommandWindow::CommandFollow(int iSelectedChaIndex)
{
    if (iSelectedChaIndex < 0)
    {
        return false;
    }

    g_iFollowCharacter = iSelectedChaIndex;

    return true;
}

int SEASON3B::CNewUICommandWindow::CommandDual(CHARACTER* pSelectedCha)
{
    int iLevel = CharacterAttribute->Level;
    if (iLevel < 30)
    {
        wchar_t szError[48] = L"";
        swprintf(szError, GlobalText[2704], 30);
        g_pSystemLogBox->AddText(szError, SEASON3B::TYPE_ERROR_MESSAGE);
        return 3;
    }
    else if (gMapManager.WorldActive >= WD_65DOPPLEGANGER1 && gMapManager.WorldActive <= WD_68DOPPLEGANGER4)
    {
        g_pSystemLogBox->AddText(GlobalText[2866], SEASON3B::TYPE_ERROR_MESSAGE);
        return 3;
    }
    else if (gMapManager.WorldActive == WD_79UNITEDMARKETPLACE)
    {
        g_pSystemLogBox->AddText(GlobalText[3063], SEASON3B::TYPE_ERROR_MESSAGE);
        return 3;
    }
    else if (!g_DuelMgr.IsDuelEnabled())
    {
        SocketClient->ToGameServer()->SendDuelStartRequest(pSelectedCha->Key, pSelectedCha->ID);
        return 1;
    }
    else if (g_DuelMgr.IsDuelEnabled() && g_DuelMgr.IsDuelPlayer(pSelectedCha, DUEL_ENEMY))
    {
        SocketClient->ToGameServer()->SendDuelStopRequest();
        return 2;
    }
    else
    {
        g_pSystemLogBox->AddText(GlobalText[915], SEASON3B::TYPE_ERROR_MESSAGE);
        return 3;
    }
    return 0;
}