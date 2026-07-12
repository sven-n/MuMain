#include "stdafx.h"
#include "UI/Windows/RememberPasswordPrompt.h"

#include "Audio/DSPlaySound.h"
#include "UI/NewUI/Dialogs/NewUICommonMessageBox.h"

namespace
{
UI::Login::RememberPasswordChoice g_Choice = UI::Login::RememberPasswordChoice::None;

// Two-button confirmation, modelled on the game's other OK/Cancel dialogs (e.g.
// the guild-request box). A bold yellow "WARNING!!!" header sits above the
// orange message body.
class CRememberPasswordMsgBoxLayout : public SEASON3B::TMsgBoxLayout<SEASON3B::CNewUICommonMessageBox>
{
public:
    bool SetLayout() override;
    static SEASON3B::CALLBACK_RESULT OnOk(SEASON3B::CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    static SEASON3B::CALLBACK_RESULT OnCancel(SEASON3B::CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
};

bool CRememberPasswordMsgBoxLayout::SetLayout()
{
    SEASON3B::CNewUICommonMessageBox* pMsgBox = GetMsgBox();
    if (pMsgBox == nullptr)
        return false;

    if (!pMsgBox->Create(SEASON3B::MSGBOX_COMMON_TYPE_OKCANCEL))
        return false;

    pMsgBox->AddMsg(L"WARNING!!!", CLRDW_YELLOW, SEASON3B::MSGBOX_FONT_BOLD);
    pMsgBox->AddMsg(L"Saving the password lets anyone using this computer log into your account. Only do this on a PC you trust. Save the password?", CLRDW_ORANGE);

    pMsgBox->AddCallbackFunc(CRememberPasswordMsgBoxLayout::OnOk, SEASON3B::MSGBOX_EVENT_USER_COMMON_OK);
    pMsgBox->AddCallbackFunc(CRememberPasswordMsgBoxLayout::OnCancel, SEASON3B::MSGBOX_EVENT_USER_COMMON_CANCEL);
    pMsgBox->AddCallbackFunc(CRememberPasswordMsgBoxLayout::OnOk, SEASON3B::MSGBOX_EVENT_PRESSKEY_RETURN);
    pMsgBox->AddCallbackFunc(CRememberPasswordMsgBoxLayout::OnCancel, SEASON3B::MSGBOX_EVENT_PRESSKEY_ESC);
    return true;
}

SEASON3B::CALLBACK_RESULT CRememberPasswordMsgBoxLayout::OnOk(SEASON3B::CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf&)
{
    g_Choice = UI::Login::RememberPasswordChoice::Ok;
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, SEASON3B::MSGBOX_EVENT_DESTROY);
    return SEASON3B::CALLBACK_BREAK;
}

SEASON3B::CALLBACK_RESULT CRememberPasswordMsgBoxLayout::OnCancel(SEASON3B::CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf&)
{
    g_Choice = UI::Login::RememberPasswordChoice::Cancel;
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, SEASON3B::MSGBOX_EVENT_DESTROY);
    return SEASON3B::CALLBACK_BREAK;
}
} // namespace

namespace UI::Login
{
void OpenRememberPasswordPrompt()
{
    g_Choice = RememberPasswordChoice::Pending;
    SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(CRememberPasswordMsgBoxLayout));
}

RememberPasswordChoice RememberPasswordChoiceState()
{
    return g_Choice;
}

void ClearRememberPasswordChoice()
{
    g_Choice = RememberPasswordChoice::None;
}
} // namespace UI::Login
