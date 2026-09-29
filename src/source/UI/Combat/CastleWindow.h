
#if !defined(AFX_NEWUICASTLEWINDOW_H__C0C75F67_38B5_48C4_98EF_DC0F4E9EB866__INCLUDED_)
#define AFX_NEWUICASTLEWINDOW_H__C0C75F67_38B5_48C4_98EF_DC0F4E9EB866__INCLUDED_

#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/Combat/CastleWindowRmlModel.h"
#include "UI/RmlBridge/RmlModelBinder.h"
#include "UI/Widgets/Window/Button.h"

#include <string>
#include <vector>

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
// The Senatus (castle lord's NPC), docked right: the Castle Gate, Guardian Statue, Tax and
// Store tabs. castle_window.rml draws it; the tab radio group and the gate / statue icon picks
// stay native hit tests, C++ keeps every request.
class CCastleWindow : public CObject
{
public:
    enum CASTLE_MSGBOX_REQUEST
    {
        CASTLE_MSGREQ_NULL,
        CASTLE_MSGREQ_BUY_GATE,
        CASTLE_MSGREQ_REPAIR_GATE,
        CASTLE_MSGREQ_UPGRADE_GATE_HP,
        CASTLE_MSGREQ_UPGRADE_GATE_DEFENSE,
        CASTLE_MSGREQ_BUY_STATUE,
        CASTLE_MSGREQ_REPAIR_STATUE,
        CASTLE_MSGREQ_UPGRADE_STATUE_HP,
        CASTLE_MSGREQ_UPGRADE_STATUE_DEFENSE,
        CASTLE_MSGREQ_UPGRADE_STATUE_RECOVER,
        CASTLE_MSGREQ_APPLY_TAX,
        CASTLE_MSGREQ_WITHDRAW,
    };

    // The buttons RmlUi reports (senatus_button(n)).
    enum SENATUS_BUTTON
    {
        SENATUS_BUTTON_NONE = -1,
        SENATUS_BUTTON_BUY = 0,
        SENATUS_BUTTON_REPAIR,
        SENATUS_BUTTON_UPGRADE_HP,
        SENATUS_BUTTON_UPGRADE_DEFENSE,
        SENATUS_BUTTON_UPGRADE_RECOVER,
        SENATUS_BUTTON_APPLY_TAX,
        SENATUS_BUTTON_WITHDRAW,
        SENATUS_BUTTON_CHAOS_TAX_UP,
        SENATUS_BUTTON_CHAOS_TAX_DOWN,
        SENATUS_BUTTON_NPC_TAX_UP,
        SENATUS_BUTTON_NPC_TAX_DOWN,
        SENATUS_BUTTON_EXIT,
    };

private:
    enum
    {
        INVENTORY_WIDTH = 190,
        INVENTORY_HEIGHT = 429,
    };
    enum CURR_OPEN_TAB_BUTTON
    {
        TAB_GATE_MANAGING,
        TAB_STATUE_MANAGING,
        TAB_TAX_MANAGING,
        TAB_CASTLE_MIX
    };

    CManager* m_pNewUIMng;
    POINT m_Pos;

    CRadioGroupButton m_TabBtn;
    int m_iNumCurOpenTab;
    int m_iCurrMsgBoxRequest;

    RmlModelBinder<CastleWindowRmlModel> m_RmlBinder;
    Rml::ElementDocument* m_pRmlDoc = nullptr;
    SENATUS_BUTTON m_PendingButton = SENATUS_BUTTON_NONE;

public:
    CCastleWindow();
    virtual ~CCastleWindow();

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

    int GetCurrMsgBoxRequest()
    {
        return m_iCurrMsgBoxRequest;
    }

    void ReloadRmlTheme();

private:
    bool BtnProcess();

    void SetCurrMsgBoxRequest(int iMsgBoxRequest)
    {
        m_iCurrMsgBoxRequest = iMsgBoxRequest;
    }
    void InsertComma(wchar_t* pszText, DWORD dwNumber);

    void UpdateGateManagingTab(SENATUS_BUTTON button);
    void UpdateStatueManagingTab(SENATUS_BUTTON button);
    void UpdateTaxManagingTab(SENATUS_BUTTON button);
    void UpdateIconPick();
    bool ButtonLocked(SENATUS_BUTTON button) const;

    void BuildRmlUi();
    void SyncRmlModel();
    void SyncContent();
};
} // namespace mu::ui::window

#endif // !defined(AFX_NEWUICASTLEWINDOW_H__C0C75F67_38B5_48C4_98EF_DC0F4E9EB866__INCLUDED_)
