#pragma once

#include "UI/Dialogs/GenericConfirmDialog.h"

#include <memory>
#include <vector>

namespace UI::Social
{
class FriendDialogs
{
public:
    ~FriendDialogs();
    void Reset();
    void Notice(const wchar_t* text);
    void Confirm(const wchar_t* text, DWORD parentUIID);
    void ConfirmAction(const wchar_t* text, DWORD parentUIID, std::function<void()> action);
    void FriendRequest(const wchar_t* text, const wchar_t* name);
    void AddFriend(DWORD parentUIID);
    void CancelAddFriend();
    bool IsAddFriendActive() const;
    void SetAddFriendName(const wchar_t* name);

private:
    using DialogId = mu::ui::window::CGenericConfirmDialog::DialogId;
    struct Session {};
    DialogId Show(mu::ui::window::GenericDialogConfig config, DWORD parentUIID = 0);
    std::shared_ptr<Session> m_Session = std::make_shared<Session>();
    std::vector<DialogId> m_Pending;
    DialogId m_AddFriend = 0;
};
}
