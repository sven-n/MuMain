#pragma once

#include "UI/Social/SocialWindowBase.h"
#include "Dotnet/Connection.h"
#include <map>

class CUIChatWindow : public CUIBaseWindow
{
    static void HandlePacketS(int32_t handle, const BYTE* ReceiveBuffer, int32_t Size);
    inline static std::map<int32_t, DWORD> ConnectionHandleToWindowUuid = {};
    Connection* _connection;

public:
    CUIChatWindow();
    virtual ~CUIChatWindow();

    void Init(const wchar_t* pszTitle, DWORD dwParentID = 0) override;
    void Refresh() override;
    BOOL DoAction(BOOL messageOnly = FALSE) override;
    void Maximize() override;
    void FocusReset();
    int AddChatPal(const wchar_t* pszID, BYTE Number, BYTE Server);
    void RemoveChatPal(const wchar_t* pszID);
    void AddChatText(BYTE byIndex, const wchar_t* pszText, int iType, int iColor);
    void ConnectToChatServer(const wchar_t* pszIP, DWORD dwRoomNumber, DWORD dwTicket);
    void DisconnectToChatServer();
    Connection* GetCurrentSocket()
    {
        return _connection;
    }
    // The invitation list's pick, by name; nullptr with nothing picked.
    const wchar_t* GetCurrentInvitePal();
    void UpdateInvitePalList();
    int GetShowType();
    const wchar_t* GetChatFriend(int* piResult = NULL);
    int GetUserCount();
    DWORD GetRoomNumber()
    {
        return m_dwRoomNumber;
    }
    void Lock(BOOL bFlag);
    bool HasSemanticView() const override
    {
        return true;
    }
    bool SyncSemanticView(bool shown) override;
    bool SemanticFieldHasFocus() const override;
    void PullSemanticViewToFront() override;

protected:
    void InitControls() override {}
    BOOL HandleMessage() override;

private:
    void RefreshRoomTitle();

    DWORD m_dwRoomNumber;
    std::unique_ptr<UI::Social::ChatRoomView> m_View;
};
