#include "stdafx.h"
#include "UI/Party/FriendDialogs.h"

#include "UI/Party/FriendWindow.h"
#include "UI/Core/WindowSystem.h"
#include "I18N/All.h"

extern int g_iChatInputType;

namespace UI::Party
{
using mu::ui::window::GenericDialogConfig;
using mu::ui::window::g_pGenericConfirmDialog;

namespace
{
GenericDialogConfig Question(const wchar_t* text)
{
    GenericDialogConfig config;
    config.title = I18N::Game::Question;
    config.lines.push_back({text});
    config.primaryLabel = I18N::Game::Yes;
    config.showCancel = true;
    config.cancelLabel = I18N::Game::No;
    return config;
}
}

FriendDialogs::~FriendDialogs()
{
    Reset();
}

void FriendDialogs::Reset()
{
    m_Session.reset();
    if (g_pGenericConfirmDialog)
    {
        for (const DialogId id : m_Pending)
            g_pGenericConfirmDialog->Cancel(id);
    }
    m_Pending.clear();
    m_AddFriend = 0;
    m_Session = std::make_shared<Session>();
}

FriendDialogs::DialogId FriendDialogs::Show(GenericDialogConfig config, DWORD parentUIID)
{
    if (!g_pGenericConfirmDialog || g_iChatInputType == 0)
        return 0;
    const std::weak_ptr<Session> session = m_Session;
    config.isValid = [session, parentUIID]
    {
        return !session.expired() &&
            (parentUIID == 0 || (g_pWindowMgr && g_pWindowMgr->GetWindow(parentUIID)));
    };
    std::erase_if(m_Pending, [](DialogId id) { return !g_pGenericConfirmDialog->IsPending(id); });
    const DialogId id = g_pGenericConfirmDialog->Show(std::move(config));
    if (id != 0)
        m_Pending.push_back(id);
    return id;
}

void FriendDialogs::Notice(const wchar_t* text)
{
    GenericDialogConfig config;
    config.title = I18N::Game::OK;
    config.primaryLabel = I18N::Game::OK;
    config.lines.push_back({text});
    Show(std::move(config));
}

void FriendDialogs::Confirm(const wchar_t* text, DWORD parentUIID)
{
    auto config = Question(text);
    config.onPrimary = [parentUIID]
    {
        g_pWindowMgr->SendUIMessageToWindow(parentUIID, UI_MESSAGE_YNRETURN, 0, 1);
    };
    config.onCancel = [parentUIID]
    {
        g_pWindowMgr->SendUIMessageToWindow(parentUIID, UI_MESSAGE_YNRETURN, 0, 0);
    };
    Show(std::move(config), parentUIID);
}

void FriendDialogs::ConfirmAction(const wchar_t* text, DWORD parentUIID, std::function<void()> action)
{
    auto config = Question(text);
    config.onPrimary = std::move(action);
    Show(std::move(config), parentUIID);
}

void FriendDialogs::FriendRequest(const wchar_t* text, const wchar_t* name)
{
    auto config = Question(text);
    const std::wstring friendName(name);
    config.onPrimary = [friendName]
    {
        SocketClient->ToGameServer()->SendFriendAddResponse(1, MU_C16(friendName.c_str()));
    };
    config.onCancel = [friendName]
    {
        SocketClient->ToGameServer()->SendFriendAddResponse(0, MU_C16(friendName.c_str()));
    };
    Show(std::move(config));
}

void FriendDialogs::AddFriend(DWORD parentUIID)
{
    if (!g_pGenericConfirmDialog || g_pGenericConfirmDialog->IsPending(m_AddFriend))
        return;
    GenericDialogConfig config;
    config.lines.push_back({I18N::Game::EnterTheIDOfTheFriendYouDLikeToAdd});
    config.primaryLabel = I18N::Game::OK;
    config.showCancel = true;
    config.cancelLabel = I18N::Game::Cancel;
    config.input.emplace();
    config.input->maxLength = MAX_USERNAME_SIZE;
    config.onPrimary = [parentUIID]
    {
        const std::wstring name = g_pGenericConfirmDialog->GetInputText();
        if (name.empty())
        {
            g_pGenericConfirmDialog->KeepOpen();
            return;
        }
        if (auto* parent = g_pWindowMgr->GetWindow(parentUIID))
        {
            parent->SetReturnText(name.c_str());
            g_pWindowMgr->SendUIMessageToWindow(parentUIID, UI_MESSAGE_TXTRETURN, 0, 0);
        }
    };
    m_AddFriend = Show(std::move(config), parentUIID);
}

void FriendDialogs::CancelAddFriend()
{
    if (g_pGenericConfirmDialog)
        g_pGenericConfirmDialog->Cancel(m_AddFriend);
    m_AddFriend = 0;
}

bool FriendDialogs::IsAddFriendActive() const
{
    return g_pGenericConfirmDialog && g_pGenericConfirmDialog->IsActive(m_AddFriend);
}

void FriendDialogs::SetAddFriendName(const wchar_t* name)
{
    if (IsAddFriendActive())
        g_pGenericConfirmDialog->SetInputText(m_AddFriend, name);
}
}
