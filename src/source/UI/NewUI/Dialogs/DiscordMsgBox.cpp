#include "stdafx.h"
#include "UI/NewUI/Dialogs/DiscordMsgBox.h"

#include "Audio/DSPlaySound.h"
#include "Core/Text/Utf8.h"
#include "GameLogic/Discord/ServerIntegration.h"
#include "Integration/Discord/Invite.h"
#include "UI/NewUI/NewUISystem.h"

#include "I18N/All.h"

#include <SDL3/SDL_clipboard.h>

namespace
{
// The code to show, handed from the packet to the layout that creates the box.
Network::Discord::LinkCode s_pendingLinkCode;

const DWORD TitleColor = RGBA(114, 137, 218, 255); // Discord's blurple, readable on the dark frame
const DWORD CodeColor = RGBA(255, 220, 120, 255);
constexpr float TopPosition = 100.f;
constexpr float ButtonGap = 8.f;
constexpr float TextLineGap = 4.f;
// The rows below the texts which the buttons take.
constexpr int ButtonRows = 2;

std::wstring Formatted(const wchar_t* format, const std::wstring& value)
{
    wchar_t text[MAX_TEXT_LENGTH]{};
    mu_swprintf_s(text, MAX_TEXT_LENGTH, format, value.c_str());
    return text;
}

std::wstring Formatted(const wchar_t* format, int value)
{
    wchar_t text[MAX_TEXT_LENGTH]{};
    mu_swprintf_s(text, MAX_TEXT_LENGTH, format, value);
    return text;
}
} // namespace

SEASON3B::CDiscordMsgBox::~CDiscordMsgBox()
{
    CNewUIMessageBoxBase::Release();
}

bool SEASON3B::CDiscordMsgBox::CreateAccount(float fPriority)
{
    const auto& server = GameLogic::Discord::ServerIntegration::Instance();
    m_mode = Mode::Account;
    m_hasInvite = Integration::Discord::Invite::IsInviteUrl(server.InviteUrl());

    AddMsg(I18N::Game::DiscordPresence, TitleColor, MSGBOX_FONT_BOLD);
    if (server.IsAccountLinked())
    {
        AddMsg(Formatted(I18N::Game::DiscordLinkedAs, server.LinkedUserName()));
    }
    else
    {
        AddMsg(I18N::Game::DiscordNotLinked);
    }

    if (server.IsGuildChatBridged())
    {
        AddMsg(I18N::Game::DiscordGuildChatMirrored);
    }
    if (server.IsAllianceChatBridged())
    {
        AddMsg(I18N::Game::DiscordAllianceChatMirrored);
    }
    if (server.IsWorldChatBridged())
    {
        AddMsg(I18N::Game::DiscordWorldChatMirrored);
    }

    CreateFrame(fPriority);
    SetButtonInfo(server.IsAccountLinked() ? &I18N::Game::DiscordUnlink : &I18N::Game::DiscordLink);
    return true;
}

bool SEASON3B::CDiscordMsgBox::CreateLinkCode(const std::wstring& code, int validMinutes, float fPriority)
{
    m_mode = Mode::LinkCode;
    m_code = code;
    m_hasInvite =
        Integration::Discord::Invite::IsInviteUrl(GameLogic::Discord::ServerIntegration::Instance().InviteUrl());

    AddMsg(I18N::Game::DiscordLinkCodeTitle, TitleColor, MSGBOX_FONT_BOLD);
    AddMsg(code, CodeColor, MSGBOX_FONT_BOLD);
    AddMsg(Formatted(I18N::Game::DiscordLinkCodeHint, code));
    AddMsg(Formatted(I18N::Game::DiscordLinkCodeValidity, validMinutes));
    AddMsg(I18N::Game::DiscordLinkCodeCopied);

    CreateFrame(fPriority);
    SetButtonInfo(&I18N::Game::DiscordCopy);
    CopyCode();
    return true;
}

void SEASON3B::CDiscordMsgBox::CreateFrame(float fPriority)
{
    AddCallbackFunc(CDiscordMsgBox::LButtonUp, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
    AddCallbackFunc(CDiscordMsgBox::Close, MSGBOX_EVENT_PRESSKEY_ESC);
    AddCallbackFunc(CDiscordMsgBox::Close, MSGBOX_EVENT_USER_COMMON_CANCEL);

    const int middleRows = static_cast<int>(m_texts.size()) + ButtonRows;
    const float height = MSGBOX_TOP_HEIGHT + (middleRows * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;
    const float x = (SCREEN_WIDTH / 2) - (MSGBOX_WIDTH / 2);
    CNewUIMessageBoxBase::Create(static_cast<int>(x), static_cast<int>(TopPosition), static_cast<int>(MSGBOX_WIDTH),
                                 static_cast<int>(height), fPriority);
}

void SEASON3B::CDiscordMsgBox::AddMsg(const std::wstring& text, DWORD color, BYTE fontType)
{
    MSGBOX_TEXTDATA data;
    data.strMsg = text;
    data.dwColor = color;
    data.byFontType = fontType;
    m_texts.push_back(std::move(data));
}

// Primary, Join (when there is an invite) and Close, side by side at the bottom.
void SEASON3B::CDiscordMsgBox::SetButtonInfo(const wchar_t* const* primaryText)
{
    const int buttonCount = m_hasInvite ? 3 : 2;
    const float rowWidth = buttonCount * MSGBOX_BTN_EMPTY_SMALL_WIDTH + (buttonCount - 1) * ButtonGap;
    float x = GetPos().x + (GetSize().cx - rowWidth) / 2.f;
    const float y = GetPos().y + GetSize().cy - (MSGBOX_BTN_EMPTY_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    const auto place = [&](CNewUIMessageBoxButton& button, const wchar_t* text)
    {
        button.SetInfo(CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, MSGBOX_BTN_EMPTY_SMALL_WIDTH,
                       MSGBOX_BTN_EMPTY_HEIGHT, CNewUIMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
        button.SetText(text);
        x += MSGBOX_BTN_EMPTY_SMALL_WIDTH + ButtonGap;
    };

    place(m_BtnPrimary, *primaryText);
    if (m_hasInvite)
    {
        place(m_BtnJoin, I18N::Game::DiscordJoin);
    }
    place(m_BtnClose, I18N::Game::Close388);
}

bool SEASON3B::CDiscordMsgBox::Update()
{
    m_BtnPrimary.Update();
    if (m_hasInvite)
    {
        m_BtnJoin.Update();
    }
    m_BtnClose.Update();
    return true;
}

bool SEASON3B::CDiscordMsgBox::Render()
{
    EnableAlphaTest();
    RenderFrame();
    RenderTexts();
    RenderButtons();
    DisableAlphaBlend();
    return true;
}

SEASON3B::CALLBACK_RESULT SEASON3B::CDiscordMsgBox::LButtonUp(CNewUIMessageBoxBase* pOwner,
                                                              const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CDiscordMsgBox*>(pOwner);
    if (pMsgBox == nullptr)
    {
        return CALLBACK_CONTINUE;
    }

    if (pMsgBox->m_BtnPrimary.IsMouseIn())
    {
        PlayBuffer(SOUND_CLICK01);
        pMsgBox->OnPrimary();
        return CALLBACK_BREAK;
    }
    if (pMsgBox->m_hasInvite && pMsgBox->m_BtnJoin.IsMouseIn())
    {
        PlayBuffer(SOUND_CLICK01);
        Integration::Discord::Invite::Open(GameLogic::Discord::ServerIntegration::Instance().InviteUrl());
        return CALLBACK_BREAK;
    }
    if (pMsgBox->m_BtnClose.IsMouseIn())
    {
        return Close(pOwner, xParam);
    }
    return CALLBACK_CONTINUE;
}

SEASON3B::CALLBACK_RESULT SEASON3B::CDiscordMsgBox::Close(CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf&)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    return CALLBACK_BREAK;
}

// Account: asks for a code, or unlinks and closes (the server answers with the
// updated info). Code: copies the code again.
void SEASON3B::CDiscordMsgBox::OnPrimary()
{
    if (m_mode == Mode::LinkCode)
    {
        CopyCode();
        return;
    }

    const auto& server = GameLogic::Discord::ServerIntegration::Instance();
    if (server.IsAccountLinked())
    {
        server.RequestUnlink();
    }
    else
    {
        server.RequestLinkCode();
    }
    g_MessageBox->SendEvent(this, MSGBOX_EVENT_DESTROY);
}

void SEASON3B::CDiscordMsgBox::CopyCode() const
{
    SDL_SetClipboardText(Core::Text::ToUtf8(m_code.c_str()).c_str());
}

void SEASON3B::CDiscordMsgBox::RenderFrame()
{
    float x = GetPos().x;
    float y = GetPos().y + 2.f;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK, x, y, GetSize().cx - MSGBOX_BACK_BLANK_WIDTH,
                GetSize().cy - MSGBOX_BACK_BLANK_HEIGHT);

    y = GetPos().y;
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_TOP, x, y, MSGBOX_WIDTH, MSGBOX_TOP_HEIGHT);
    y += MSGBOX_TOP_HEIGHT;

    const int middleRows = static_cast<int>(m_texts.size()) + ButtonRows;
    for (int i = 0; i < middleRows; ++i)
    {
        RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_MIDDLE, x, y, MSGBOX_WIDTH, MSGBOX_MIDDLE_HEIGHT);
        y += MSGBOX_MIDDLE_HEIGHT;
    }
    RenderImage(CNewUIMessageBoxMng::IMAGE_MSGBOX_BOTTOM, x, y, MSGBOX_WIDTH, MSGBOX_BOTTOM_HEIGHT);
}

void SEASON3B::CDiscordMsgBox::RenderTexts()
{
    float y = GetPos().y + (MSGBOX_TEXT_TOP_BLANK / 2);
    for (const MSGBOX_TEXTDATA& text : m_texts)
    {
        g_pRenderText->SetTextColor(text.dwColor);
        g_pRenderText->SetBgColor(0, 0, 0, 0);
        g_pRenderText->SetFont(text.byFontType == MSGBOX_FONT_BOLD ? g_hFontBold : g_hFont);

        const SIZE size = g_pRenderText->MeasureText(text.strMsg.c_str(), static_cast<int>(text.strMsg.size()));
        const float x = GetPos().x + (GetSize().cx / 2.f) - (size.cx / 2.f);
        g_pRenderText->RenderText(static_cast<int>(x), static_cast<int>(y), text.strMsg.c_str());
        y += size.cy + TextLineGap;
    }
}

void SEASON3B::CDiscordMsgBox::RenderButtons()
{
    m_BtnPrimary.Render();
    if (m_hasInvite)
    {
        m_BtnJoin.Render();
    }
    m_BtnClose.Render();
}

bool SEASON3B::CDiscordAccountMsgBoxLayout::SetLayout()
{
    CDiscordMsgBox* pMsgBox = GetMsgBox();
    return pMsgBox != nullptr && pMsgBox->CreateAccount();
}

bool SEASON3B::CDiscordLinkCodeMsgBoxLayout::SetLayout()
{
    CDiscordMsgBox* pMsgBox = GetMsgBox();
    return pMsgBox != nullptr && pMsgBox->CreateLinkCode(s_pendingLinkCode.code, s_pendingLinkCode.validMinutes);
}

namespace UI::Discord
{
void ShowAccount()
{
    // Shows what is known and asks again, so the next opening reflects a link
    // made in Discord meanwhile.
    GameLogic::Discord::ServerIntegration::Instance().Request();
    SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CDiscordAccountMsgBoxLayout));
}

void ShowLinkCode(const Network::Discord::LinkCode& linkCode)
{
    if (!linkCode.isAvailable || linkCode.code.empty())
    {
        g_pSystemLogBox->AddText(I18N::Game::DiscordLinkNotAvailable, SEASON3B::TYPE_ERROR_MESSAGE);
        return;
    }

    s_pendingLinkCode = linkCode;
    SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CDiscordLinkCodeMsgBoxLayout));
}
} // namespace UI::Discord
