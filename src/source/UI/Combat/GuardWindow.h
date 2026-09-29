
#if !defined(AFX_NEWUIGUARDWINDOW_H__0FFE1FE7_59B6_47D8_B79C_7E3DD9912E14__INCLUDED_)
#define AFX_NEWUIGUARDWINDOW_H__0FFE1FE7_59B6_47D8_B79C_7E3DD9912E14__INCLUDED_

#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Combat/GuardWindowRmlModel.h"
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
// guard_window.rml draws it; the tab radio group and the two guild lists stay native controls
// for their hit tests, data, scrolling and line clicks (their drawing is the document's); C++
// keeps every request.
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
    CUIBCDeclareGuildListBox m_DeclareGuildListBox;
    // Ȯ���� ��� ����Ʈ
    CUIBCGuildListBox m_GuildListBox;

    // UI ��� ��
    CASTLESIEGE_STATE m_eTimeType;

    wchar_t m_szOwnerGuild[8 + 1];
    wchar_t m_szOwnerGuildMaster[10 + 1];

    WORD m_wStartYear;
    BYTE m_byStartMonth;
    BYTE m_byStartDay;
    BYTE m_byStartHour;
    BYTE m_byStartMinute;
    WORD m_wEndYear;
    BYTE m_byEndMonth;
    BYTE m_byEndDay;
    BYTE m_byEndHour;
    BYTE m_byEndMinute;
    WORD m_wSiegeStartYear;
    BYTE m_bySiegeStartMonth;
    BYTE m_bySiegeStartDay;
    BYTE m_bySiegeStartHour;
    BYTE m_bySiegeStartMinute;
    DWORD m_dwStateLeftSec;

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

    // Shared with the still-native list boxes that draw the same scroll bar (UIControls.cpp);
    // CGuardWindow loads its images (LoadScrollBarImages()).
    void RenderScrollBarFrame(int iPos_x, int iPos_y, int iHeight);
    void RenderScrollBar(int iPos_x, int iPos_y, BOOL bIsClicked);

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
};
}

#endif // !defined(AFX_NEWUIGUARDWINDOW_H__0FFE1FE7_59B6_47D8_B79C_7E3DD9912E14__INCLUDED_)
