// The Discord dialog: the state of the account link with its actions, and
// the one-time code to link the account. Shown when the server has the
// Discord integration (GameLogic::Discord::ServerIntegration).
#pragma once

#include "Network/Discord/DiscordPackets.h"
#include "UI/NewUI/Dialogs/NewUICommonMessageBox.h"
#include "UI/NewUI/Dialogs/NewUIMessageBox.h"

#include <string>
#include <vector>

namespace SEASON3B
{
class CDiscordMsgBox : public CNewUIMessageBoxBase
{
public:
    CDiscordMsgBox() = default;
    ~CDiscordMsgBox() override;

    // Whether the account is linked, which chats are mirrored; Link or
    // Unlink, Join, Close.
    bool CreateAccount(float fPriority = 3.f);

    // The code to enter in Discord, copied to the clipboard; Copy, Join, Close.
    bool CreateLinkCode(const std::wstring& code, int validMinutes, float fPriority = 3.f);

    bool Update() override;
    bool Render() override;

    static CALLBACK_RESULT LButtonUp(CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    static CALLBACK_RESULT Close(CNewUIMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

private:
    enum class Mode
    {
        Account,
        LinkCode,
    };

    void CreateFrame(float fPriority);
    void AddMsg(const std::wstring& text, DWORD color = CLRDW_WHITE, BYTE fontType = MSGBOX_FONT_NORMAL);
    void SetButtonInfo(const wchar_t* const* primaryText);

    void OnPrimary();
    void CopyCode() const;

    void RenderFrame();
    void RenderTexts();
    void RenderButtons();

    Mode m_mode = Mode::Account;
    std::wstring m_code;
    bool m_hasInvite = false;
    std::vector<MSGBOX_TEXTDATA> m_texts;

    CNewUIMessageBoxButton m_BtnPrimary;
    CNewUIMessageBoxButton m_BtnJoin;
    CNewUIMessageBoxButton m_BtnClose;
};

class CDiscordAccountMsgBoxLayout : public TMsgBoxLayout<CDiscordMsgBox>
{
public:
    bool SetLayout() override;
};

class CDiscordLinkCodeMsgBoxLayout : public TMsgBoxLayout<CDiscordMsgBox>
{
public:
    bool SetLayout() override;
};
} // namespace SEASON3B

namespace UI::Discord
{
// Opens the account dialog.
void ShowAccount();

// Shows the code the server sent, or says that linking isn't available.
void ShowLinkCode(const Network::Discord::LinkCode& linkCode);
} // namespace UI::Discord
