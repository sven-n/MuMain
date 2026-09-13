//////////////////////////////////////////////////////////////////////////
//  UIGuildMaster.cpp
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UIGuildInfo.h"
#include "Engine/Object/ZzzInventory.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Render/Textures/ZzzTexture.h"
#include "UI/Core/UIManager.h"
#include "UIGuildMaster.h"
#include "Audio/DSPlaySound.h"
#include "I18N/All.h"

#include "UI/Dialogs/CommonMessageBox.h"
#include "UI/Dialogs/GenericConfirmDialog.h"
#include "App/Platform/Windows/Local.h"
#include "UI/Core/WindowSystem.h"
#include "Engine/Object/ZzzInterface.h"

extern int				g_iChatInputType;

static eCurrentMode		m_nCurrMode;
static eCurrentStep		m_eCurrStep;

void RenderGoldRect(float fPos_x, float fPos_y, float fWidth, float fHeight, int iFillType = 0);

void EditGuildMarkMouseAction(int iPos_x, int iPos_y)
{
    int i, j;
    float x, y;
    Hero->Object.Angle[2] = 90.f + 22.5f;
    for (i = 0; i < 8; ++i)
    {
        for (j = 0; j < 8; ++j)
        {
            x = iPos_x + j * 15 + 50;
            y = iPos_y + i * 15 + 100;
            if (MouseX >= x && MouseX < x + 15 && MouseY >= y && MouseY < y + 15)
            {
                if (MouseLButton)
                    GuildMark[MARK_EDIT].Mark[i * 8 + j] = SelectMarkColor;
                if (MouseRButton)
                    GuildMark[MARK_EDIT].Mark[i * 8 + j] = 0;
            }
        }
    }
    for (i = 0; i < 2; ++i)
    {
        for (j = 0; j < 8; ++j)
        {
            x = iPos_x + j * 20 + 15;
            y = iPos_y + i * 20 + 260;
            if (MouseX >= x && MouseX < x + 20 && MouseY >= y && MouseY < y + 20)
            {
                if (MouseLButtonPush)
                {
                    MouseLButtonPush = FALSE;
                    PlayBuffer(SOUND_CLICK01);
                    SelectMarkColor = i * 8 + j;
                }
            }
        }
    }
}

void RenderGuildMark(int iPos_x, int iPos_y)
{
    int i, j;
    float x, y;
    for (i = 0; i < 8; ++i)
    {
        for (j = 0; j < 8; ++j)
        {
            x = (float)iPos_x + j * 15 + 50;
            y = (float)iPos_y + i * 15 + 100;
            RenderGuildColor(x + 1, y + 1, 13, 13, GuildMark[MARK_EDIT].Mark[i * 8 + j]);
        }
    }
    for (i = 0; i < 2; ++i)
    {
        for (j = 0; j < 8; ++j)
        {
            x = (float)iPos_x + j * 20 + 15;
            y = (float)iPos_y + i * 20 + 260;
            RenderGuildColor(x + 1, y + 1, 18, 18, i * 8 + j);
        }
    }
    x = (float)iPos_x + 15;
    y = (float)iPos_y + 230;
    RenderGuildColor(x + 1, y + 1, 23, 23, SelectMarkColor);

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(230, 230, 230, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->RenderText(iPos_x + 50, iPos_y + 230, I18N::Game::AfterSelectingAColorWith);
    g_pRenderText->RenderText(iPos_x + 50, iPos_y + 245, I18N::Game::TheMousePleaseDraw);
}

int DoEditGuildMarkConfirmAction(POPUP_RESULT Result)
{
    if (Result == POPUP_RESULT_YES)
    {
        m_nCurrMode = MODE_EDIT_GUILDMARK;
        m_eCurrStep = STEP_EDIT_GUILD_MARK;
        SocketClient->ToGameServer()->SendGuildMasterAnswer(true);

        if (Hero->GuildStatus != G_NONE)
            memcpy(&GuildMark[MARK_EDIT], &GuildMark[Hero->GuildMarkIndex], sizeof(MARK_t));
    }
    return 1;
}

CUIGuildMaster::CUIGuildMaster()
{
    m_bOpened = false;
    SetPosition(REFERENCE_WIDTH - 190, 0);
    SetSize(190, 90);
    m_nCurrMode = MODE_NONE;
    m_eCurrStep = STEP_MAIN;

    m_CreateGuildButton.Init(1, I18N::Game::CreateGuild);
    m_CreateGuildButton.SetParentUIID(GetUIID());
    m_CreateGuildButton.SetSize(100, 20);

    m_EditGuildMarkButton.Init(2, I18N::Game::ChangeGuildMark);
    m_EditGuildMarkButton.SetParentUIID(GetUIID());
    m_EditGuildMarkButton.SetSize(100, 20);

    m_PreviousButton.Init(4, I18N::Game::Back);
    m_PreviousButton.SetParentUIID(GetUIID());
    m_PreviousButton.SetPosition(GetPosition_x() + 15 + 30, GetPosition_y() + 360);
    m_PreviousButton.SetSize(50, 18);
    m_NextButton.Init(5, I18N::Game::Next);
    m_NextButton.SetParentUIID(GetUIID());
    m_NextButton.SetPosition(GetPosition_x() + 15 + 82, GetPosition_y() + 360);
    m_NextButton.SetSize(50, 18);
}

CUIGuildMaster::~CUIGuildMaster()
{
}

BOOL CUIGuildMaster::IsValidGuildName(const wchar_t* szName)
{
    if (wcslen(szName) >= 4)
        return TRUE;
    else
        return FALSE;
}

BOOL CUIGuildMaster::IsValidGuildMark()
{
    BOOL bDraw = FALSE;

    for (int i = 0; i < 64; i++)
    {
        if (GuildMark[MARK_EDIT].Mark[i] != 0)
            return TRUE;
    }

    return FALSE;
}

void CUIGuildMaster::StepPrev()
{
    switch (m_nCurrMode)
    {
    case MODE_CREATE_GUILD:
        switch (m_eCurrStep)
        {
        case STEP_CREATE_GUILDINFO:		m_nCurrMode = MODE_NONE;	m_eCurrStep = STEP_MAIN;	break;
        case STEP_CONFIRM_GUILDINFO:	m_eCurrStep = STEP_CREATE_GUILDINFO;					break;
        default:						assert(!"m_eCurrStep");		break;
        }
        break;
    case MODE_EDIT_GUILDMARK:
        switch (m_eCurrStep)
        {
        case STEP_EDIT_GUILD_MARK:		m_nCurrMode = MODE_NONE;	m_eCurrStep = STEP_MAIN;	break;
        case STEP_CONFIRM_GUILDINFO:	m_eCurrStep = STEP_EDIT_GUILD_MARK;		break;
        default:						assert(!"m_eCurrStep");		break;
        }
        break;
    default:
        assert(!"m_nCurrMode");
        break;
    }
}

void CUIGuildMaster::StepNext()
{
    switch (m_nCurrMode)
    {
    case MODE_CREATE_GUILD:
        switch (m_eCurrStep)
        {
        case STEP_MAIN:					m_eCurrStep = STEP_CREATE_GUILDINFO;	break;
        case STEP_CREATE_GUILDINFO:		m_eCurrStep = STEP_CONFIRM_GUILDINFO;	break;
        default:						assert(!"Guild Error 1");		break;
        }
        break;
    case MODE_EDIT_GUILDMARK:
        switch (m_eCurrStep)
        {
        case STEP_MAIN:					m_eCurrStep = STEP_EDIT_GUILD_MARK;		break;
        case STEP_EDIT_GUILD_MARK:		m_eCurrStep = STEP_CONFIRM_GUILDINFO;	break;
        default:						assert(!"Guild Error 2");		break;
        }
        break;
    default:
        assert(!"m_nCurrMode");
        break;
    }
}

void CUIGuildMaster::DoCreateGuildAction()
{
    EditGuildMarkMouseAction(GetPosition_x(), GetPosition_y());

    if (m_NextButton.DoMouseAction())
    {
        if (g_iChatInputType == 1)
        {
            g_pSingleTextInputBox->GetText(GuildMark[MARK_EDIT].GuildName);
            g_pSingleTextInputBox->GetText(InputText[0]);
        }
        if (CheckName())
        {
            mu::ui::window::GenericDialogConfig cfg;
            cfg.lines = {
                { I18N::Game::RestrictedWordsAre, false },
                { I18N::Game::Included, false },
            };
            mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
        }
        else if (CheckSpecialText(InputText[0]))
        {
            mu::ui::window::CreateOkMessageBox(I18N::Game::CannotUseSymbols);
        }
        else
        {
            if (IsValidGuildName(InputText[0]))
            {
                if (IsValidGuildMark())
                {
                    if (g_iChatInputType == 1)
                    {
                        g_pSingleTextInputBox->SetText(NULL);
                        g_pSingleTextInputBox->SetState(UISTATE_HIDE);
                    }
                    else if (g_iChatInputType == 0)
                    {
                        wcscpy(GuildMark[MARK_EDIT].GuildName, InputText[0]);
                    }
                    StepNext();
                }
                else
                {
                    mu::ui::window::CreateOkMessageBox(I18N::Game::PleaseDrawYourGuildEmblem);
                }
            }
            else
            {
                mu::ui::window::CreateOkMessageBox(I18N::Game::TypeMoreThan4Letters);
            }
        }
        if (g_iChatInputType == 1)
        {
            g_pSingleTextInputBox->SetText(NULL);
            SaveIMEStatus();
            g_pSingleTextInputBox->SetState(UISTATE_HIDE);
        }
    }

    if (m_PreviousButton.DoMouseAction())
    {
        Hero->GuildMarkIndex = -1;
        if (g_iChatInputType == 1)
        {
            g_pSingleTextInputBox->SetText(NULL);
            SaveIMEStatus();
            g_pSingleTextInputBox->SetState(UISTATE_HIDE);
        }
        memset(InputText[0], 0, MAX_USERNAME_SIZE);
        InputLength[0] = 0;
        InputTextMax[0] = MAX_USERNAME_SIZE;
        StepPrev();
    }
}

void CUIGuildMaster::RenderCreateGuild()
{
    POINT ptOrigin = { GetPosition_x() + 25, GetPosition_y() + 60 };

    RenderBitmap(BITMAP_INVENTORY + 11, ptOrigin.x + 33, ptOrigin.y - 4, 110, 18, 0.f, 0.f, 110.f / 128.f, 18.f / 32.f);

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(230, 230, 230, 255);
    g_pRenderText->SetBgColor(0);
    g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, I18N::Game::NAME);

    if (g_iChatInputType == 1)
    {
        g_pSingleTextInputBox->Configure({
            .pos = {ptOrigin.x + 38, ptOrigin.y},
            .size = {70, 14},
            .textLimit = 8,
        });
        g_pSingleTextInputBox->GiveFocus();
        g_pSingleTextInputBox->DoAction();
        g_pSingleTextInputBox->Render();
    }
    else if (g_iChatInputType == 0)
    {
        InputTextWidth = 100;
        RenderInputText(ptOrigin.x + 38, ptOrigin.y, 0);
        InputTextWidth = 255;
    }

    CreateGuildMark(MARK_EDIT);
    RenderGuildMark(GetPosition_x(), GetPosition_y());

    m_PreviousButton.Render();
    m_NextButton.Render();
}

void CUIGuildMaster::DoCreateInfoAction()
{
    if (m_NextButton.DoMouseAction())
    {
        BYTE Mark[32];
        for (int i = 0; i < 64; i++)
        {
            if (i % 2 == 0)
                Mark[i / 2] = GuildMark[MARK_EDIT].Mark[i] << 4;
            else
                Mark[i / 2] += GuildMark[MARK_EDIT].Mark[i];
        }

        if (m_nCurrMode == MODE_CREATE_GUILD)
        {
            SocketClient->ToGameServer()->SendGuildCreateRequest(MU_C16(GuildMark[MARK_EDIT].GuildName), Mark, sizeof Mark);
        }
        //		else if( m_nCurrMode == MODE_EDIT_GUILDMARK )
        //		{
        //			SendRequestEditGuildMark( (BYTE*)GuildMark[MARK_EDIT].GuildName, Mark );
        //		}

        SocketClient->ToGameServer()->SendGuildMasterAnswer(false);
        Close();

        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_NPCGUILDMASTER);
    }

    if (m_PreviousButton.DoMouseAction())
    {
        StepPrev();
    }
}

void CUIGuildMaster::RenderCreateInfo()
{
    POINT ptOrigin = { GetPosition_x() + 25, GetPosition_y() + 65 };

    g_pRenderText->SetFont(g_hFont);

    wchar_t szTemp[128];
    g_pRenderText->SetBgColor(0);

    RenderGoldRect(ptOrigin.x, ptOrigin.y, 140, 20.f);
    mu_swprintf(szTemp, L"%ls : %ls", I18N::Game::NAME, GuildMark[MARK_EDIT].GuildName);
    ptOrigin.y += 6;
    g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, szTemp, 140, 0, RT3_SORT_CENTER);

    ptOrigin.x += 44;
    ptOrigin.y += 33;
    RenderGoldRect(ptOrigin.x, ptOrigin.y, 53.f, 53.f);
    CreateGuildMark(MARK_EDIT);
    RenderBitmap(BITMAP_GUILD, ptOrigin.x + 3, ptOrigin.y + 3, 48, 48);

    m_PreviousButton.Render();
    m_NextButton.Render();
}

void CUIGuildMaster::DoEditGuildMarkAction()
{
    EditGuildMarkMouseAction(GetPosition_x(), GetPosition_y());

    if (m_NextButton.DoMouseAction())
    {
        if (IsValidGuildMark())
        {
            StepNext();
        }
        else
        {
            mu::ui::window::CreateOkMessageBox(I18N::Game::PleaseDrawYourGuildEmblem);
        }
    }

    if (m_PreviousButton.DoMouseAction())
    {
        StepPrev();
    }
}

void CUIGuildMaster::RenderEditGuildMark()
{
    POINT ptOrigin = { GetPosition_x() + 25, GetPosition_y() + 60 };

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(230, 230, 230, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, I18N::Game::NAME);

    wchar_t Text[100];
    mu_swprintf(Text, L"%ls ( Score:%d )", GuildMark[Hero->GuildMarkIndex].GuildName, GuildTotalScore);
    g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, Text, 140 * g_fScreenRate_x, 0, RT3_SORT_CENTER);
    g_pRenderText->SetFont(g_hFont);

    CreateGuildMark(MARK_EDIT);
    RenderGuildMark(GetPosition_x(), GetPosition_y());

    m_PreviousButton.Render();
    m_NextButton.Render();
}

void CUIGuildMaster::DoGuildMasterMainAction()
{
    if (m_CreateGuildButton.DoMouseAction())
    {
        m_nCurrMode = MODE_CREATE_GUILD;
        m_eCurrStep = STEP_CREATE_GUILDINFO;
        GuildInputEnable = TRUE;
        SocketClient->ToGameServer()->SendGuildMasterAnswer(true);
    }
    if (m_EditGuildMarkButton.DoMouseAction())
    {
        mu::ui::window::GenericDialogConfig cfg;
        cfg.lines = { { I18N::Game::ThisFunctionIsNotActivated, false } };
        mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
    }

    if (MouseLButtonPush && CheckMouseIn(GetPosition_x() + 25, GetPosition_y() + 395, 24, 24))
    {
        MouseLButtonPush = FALSE;
        MouseUpdateTime = 0;
        MouseUpdateTimeMax = 6;
        SocketClient->ToGameServer()->SendGuildMasterAnswer(false);
        PlayBuffer(SOUND_CLICK01);
        Close();
        g_pNewUIMng->ShowInterface(mu::ui::window::INTERFACE_NPCGUILDMASTER, false);
        g_pNewUIMng->EnableInterface(mu::ui::window::INTERFACE_NPCGUILDMASTER, false);
    }
}

void CUIGuildMaster::RenderGuildMasterMain()
{
    POINT ptOrigin = { GetPosition_x() + 25, GetPosition_y() + 60 };

    g_pRenderText->SetTextColor(230, 230, 230, 255);
    g_pRenderText->SetBgColor(0);

    if (Hero->GuildStatus != G_NONE)
    {
        wchar_t Text[100];
        mu_swprintf(Text, L"%ls ( Score:%d )", GuildMark[Hero->GuildMarkIndex].GuildName, GuildTotalScore);
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, Text, 140 * g_fScreenRate_x, 0, RT3_SORT_CENTER);
        m_CreateGuildButton.SetState(UISTATE_DISABLE);
        m_EditGuildMarkButton.SetState(UISTATE_NORMAL);
    }
    else
    {
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, I18N::Game::DoYouWishToBeTheGuildMaster);
        m_CreateGuildButton.SetState(UISTATE_NORMAL);
        m_EditGuildMarkButton.SetState(UISTATE_DISABLE);
    }

    ptOrigin.x = GetPosition_x() + 45;
    ptOrigin.y += 30;
    m_CreateGuildButton.SetPosition(ptOrigin.x, ptOrigin.y);
    m_CreateGuildButton.Render();
    ptOrigin.y += 24;
    m_EditGuildMarkButton.SetPosition(ptOrigin.x, ptOrigin.y);
    m_EditGuildMarkButton.Render();

    float Width = 24.f; float Height = 24.f; float x = (float)GetPosition_x() + 25; float y = (float)GetPosition_y() + 395;
    RenderBitmap(BITMAP_INVENTORY_BUTTON, x, y, Width, Height, 0.f, 0.f, Width / 32.f, Height / 32.f);
    if (CheckMouseIn(x, y, Width, Height))
    {
        g_pRenderText->SetFont(g_hFont);
        g_pRenderText->SetTextColor(255, 255, 255, 255);
        g_pRenderText->SetBgColor(0, 0, 0, 255);

        RenderTipText(x, y - 13, I18N::Game::Close388);
    }
}

BOOL CUIGuildMaster::DoMouseAction()
{
    if (CheckMouseIn(GetPosition_x(), GetPosition_y(), GetWidth(), 256 + 177))
        MouseOnWindow = TRUE;

    switch (m_eCurrStep)
    {
    case STEP_MAIN:
        DoGuildMasterMainAction();
        break;
    case STEP_CREATE_GUILDINFO:
        DoCreateGuildAction();
        break;
    case STEP_EDIT_GUILD_MARK:
        DoEditGuildMarkAction();
        break;
    case STEP_CONFIRM_GUILDINFO:
        DoCreateInfoAction();
        break;
    default:
        break;
    };

    return FALSE;
}

void CUIGuildMaster::Render()
{
    POINT ptOrigin = { GetPosition_x(), GetPosition_y() };

    DisableAlphaBlend();
    RenderBitmap(BITMAP_INVENTORY, ptOrigin.x, ptOrigin.y, 190.f, 256.f, 0.f, 0.f, 190.f / 256.f, 256.f / 256.f);
    RenderBitmap(BITMAP_INVENTORY + 1, ptOrigin.x, ptOrigin.y + 256, 190.f, 177.f, 0.f, 0.f, 190.f / 256.f, 177.f / 256.f);

    EnableAlphaTest();
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(220, 220, 220, 255);
    g_pRenderText->SetBgColor(20, 20, 20, 255);

    ptOrigin.x += 35;	ptOrigin.y += 12;
    switch (m_eCurrStep)
    {
    case STEP_MAIN:
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, I18N::Game::Guild, 120, 0, RT3_SORT_CENTER);
        RenderGuildMasterMain();
        break;
    case STEP_CREATE_GUILDINFO:
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, I18N::Game::CreateGuild, 120, 0, RT3_SORT_CENTER);
        RenderCreateGuild();
        break;
    case STEP_EDIT_GUILD_MARK:
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, I18N::Game::ChangeGuildMark, 120, 0, RT3_SORT_CENTER);
        RenderEditGuildMark();
        break;
    case STEP_CONFIRM_GUILDINFO:
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, I18N::Game::OK, 120, 0, RT3_SORT_CENTER);
        RenderCreateInfo();
        break;
    default:
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, I18N::Game::Guild, 120, 0, RT3_SORT_CENTER);
        break;
    };
}

void CUIGuildMaster::Open()
{
    if (m_bOpened)	return;

    m_bOpened = TRUE;
}

bool CUIGuildMaster::IsOpen()
{
    return m_bOpened;
}

void CUIGuildMaster::Close()
{
    if (!m_bOpened)	return;

    m_bOpened = FALSE;

    if (Hero->GuildMarkIndex == MARK_EDIT)
        Hero->GuildMarkIndex = -1;
    PlayBuffer(SOUND_INTERFACE01);
    m_nCurrMode = MODE_NONE;
    m_eCurrStep = STEP_MAIN;
    GuildInputEnable = FALSE;

    SocketClient->ToGameServer()->SendGuildMasterAnswer(false);

    if (g_pSingleTextInputBox)
    {
        g_pSingleTextInputBox->SetText(NULL);
        SaveIMEStatus();
        g_pSingleTextInputBox->SetState(UISTATE_HIDE);
    }
}
