
#if !defined(AFX_NEWUIGUARDWINDOW_H__0FFE1FE7_59B6_47D8_B79C_7E3DD9912E14__INCLUDED_)
#define AFX_NEWUIGUARDWINDOW_H__0FFE1FE7_59B6_47D8_B79C_7E3DD9912E14__INCLUDED_

#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Combat/GuardWindowRmlModel.h"
#include "UI/Combat/GuardGuildLists.h"
#include "UI/RmlBridge/RmlModelBinder.h"
#include "UI/Widgets/Window/Button.h"
#include "UI/Dialogs/MessageBox.h"
#include "UI/Inventory/MyInventory.h"
#include "Guild/GuildInfoWindow.h"
#include "UI/HUD/ChatLogWindow.h"
#include "UI/Inventory/InventoryCtrl.h"

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
// The castle guardsman, docked right: the Status, Register / Announce and List tabs.
// RmlUi owns guild lists and actions; C++ keeps their state and server requests.
class CGuardWindow : public CObject
{
public:
    // The scroll bar images, shared with the still-native windows that draw the same bar.
    enum IMAGE_LIST
    {
        IMAGE_GUARDWINDOW_SCROLL_TOP = CChatLogWindow::IMAGE_SCROLL_TOP,
        IMAGE_GUARDWINDOW_SCROLL_MIDDLE = CChatLogWindow::IMAGE_SCROLL_MIDDLE,
        IMAGE_GUARDWINDOW_SCROLL_BOTTOM = CChatLogWindow::IMAGE_SCROLL_BOTTOM,
        IMAGE_GUARDWINDOW_SCROLLBAR_ON = CChatLogWindow::IMAGE_SCROLLBAR_ON,
        IMAGE_GUARDWINDOW_SCROLLBAR_OFF = CChatLogWindow::IMAGE_SCROLLBAR_OFF,
    };

    // The buttons RmlUi reports (guard_button(n)).
    enum GUARD_BUTTON
    {
        GUARD_BUTTON_NONE = -1,
        GUARD_BUTTON_PROCLAIM = 0,
        GUARD_BUTTON_REGISTER,
        GUARD_BUTTON_GIVE_UP,
        GUARD_BUTTON_EXIT,
    };

private:
    enum
    {
        INVENTORY_WIDTH = 190,
        INVENTORY_HEIGHT = 429,
    };
    enum CURR_OPEN_TAB_BUTTON
    {
        TAB_SIEGE_INFO,
        TAB_REGISTER,
        TAB_REGISTER_INFO
    };

    CManager* m_pNewUIMng;
    POINT m_Pos;

    // The radio group is the tabs' hit test only; m_iNumCurOpenTab is the tab itself, and the one
    // thing the page and the document's highlight both read. Write it through SetCurOpenTab().
    CRadioGroupButton m_TabBtn;
    int m_iNumCurOpenTab; // ���� �����ִ� �ǹ�ư��ȣ

    void SetCurOpenTab(int iTab);

    RmlModelBinder<GuardWindowRmlModel> m_RmlBinder;
    Rml::ElementDocument* m_pRmlDoc = nullptr;
    GUARD_BUTTON m_PendingButton = GUARD_BUTTON_NONE;

    // ������ ��� ����Ʈ
    UI::Combat::GuardGuildLists m_GuildLists;
    unsigned m_ListRevision = 0;
    bool m_ListsDirty = true;
    std::wstring m_ListGuild;
    std::wstring m_ListAlliance;

    CASTLESIEGE_STATE m_eTimeType = CASTLESIEGE_STATE_NONE;

    wchar_t m_szOwnerGuild[8 + 1] = {};
    wchar_t m_szOwnerGuildMaster[10 + 1] = {};

    WORD m_wStartYear = 0;
    BYTE m_byStartMonth = 0;
    BYTE m_byStartDay = 0;
    BYTE m_byStartHour = 0;
    BYTE m_byStartMinute = 0;
    WORD m_wEndYear = 0;
    BYTE m_byEndMonth = 0;
    BYTE m_byEndDay = 0;
    BYTE m_byEndHour = 0;
    BYTE m_byEndMinute = 0;
    WORD m_wSiegeStartYear = 0;
    BYTE m_bySiegeStartMonth = 0;
    BYTE m_bySiegeStartDay = 0;
    BYTE m_bySiegeStartHour = 0;
    BYTE m_bySiegeStartMinute = 0;
    DWORD m_dwStateLeftSec = 0;

public:
    CGuardWindow();
    virtual ~CGuardWindow();

    bool Create(CManager* pNewUIMng, int x, int y);
    void Release();

    void SetPos(int x, int y);

    bool UpdateMouseEvent();
    bool UpdateKeyEvent();
    bool Update();
    bool Render();


    void OpeningProcess();
    void ClosingProcess();

    float GetLayerDepth(); //. 5.0f

    void SetData(LPPMSG_ANS_CASTLESIEGESTATE Info); // �������� �޾� ȭ�� ǥ�� ����

    void AddDeclareGuildList(wchar_t* szGuildName, int nMarkCount, BYTE byIsGiveUP, BYTE bySeqNum);
    void ClearDeclareGuildList();
    void SortDeclareGuildList();
    void AddGuildList(wchar_t* szGuildName, BYTE byCsJoinSide, BYTE byGuildInvolved, int iGuildScore);
    void ClearGuildList();

    void ReloadRmlTheme();

private:
    // KNOWN ISSUE, not this pass's to fix: these slots alias CChatLogWindow's, which CScrollBar
    // (UI/Widgets/Window/ScrollBar.cpp) loads and renders from. Nothing in this window draws them
    // any more -- the two renderers that did went with the native list boxes -- so the load is
    // redundant and the unload on hide deletes textures CScrollBar still expects. Needs runtime
    // checking of who owns these slots, not a blind deletion.
    void LoadScrollBarImages();
    void UnloadScrollBarImages();
    bool BtnProcess();

    void UpdateRegisterTab(GUARD_BUTTON button);
    void UpdateRegisterInfoTab(GUARD_BUTTON button);
    void UpdateRegisterInfoLists();
    bool ProclaimLocked() const;

    void BuildRmlUi();
    void SyncRmlModel();
    void SyncContent();
    void SyncGuildLists();
    std::vector<GuardDeclareRow> BuildDeclareRows() const;
    std::vector<GuardSiegeRow> BuildSiegeRows() const;
    void SelectListGuild(bool declaration, const Rml::String& name);
};
}

#endif // !defined(AFX_NEWUIGUARDWINDOW_H__0FFE1FE7_59B6_47D8_B79C_7E3DD9912E14__INCLUDED_)
