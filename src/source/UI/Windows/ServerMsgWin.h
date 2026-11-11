//*****************************************************************************
// File: ServerMsgWin.h
//*****************************************************************************

#if !defined(AFX_SERVERMSGWIN_H__8C6AB678_703D_4A60_B334_C30A97EEC64B__INCLUDED_)
#define AFX_SERVERMSGWIN_H__8C6AB678_703D_4A60_B334_C30A97EEC64B__INCLUDED_

#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/RmlBridge/RmlThemedView.h"
#include "UI/Windows/ServerMsgRmlModel.h"

namespace Rml
{
class ElementDocument;
}

#define SMW_MSG_LINE_MAX 5
#define SMW_MSG_ROW_MAX 83

// Purely passive/non-interactive message log; never becomes CUIMng's "active" window.
// server_msg.rml draws it (the frame sized to the line count, the lines in the fixed font).
class CServerMsgWin : public mu::ui::window::CObject
{
protected:
    POINT m_ptPos;
    int m_nBgSideNow;

    wchar_t m_aszMsg[SMW_MSG_LINE_MAX][SMW_MSG_ROW_MAX];
    int m_nMsgLine;

public:
    CServerMsgWin();
    ~CServerMsgWin() override;

    void Create();
    void Release();
    void SetPosition(int nXCoord, int nYCoord);
    void AddMsg(wchar_t* pszMsg);
    void Show(bool bShow) override;

    // mu::ui::window::IObject
    bool Render() override;
    bool Update() override;
    // Never consumes -- purely passive message log, never intercepted clicks even as a CWin.
    bool UpdateMouseEvent() override { return true; }
    bool UpdateKeyEvent() override { return true; }
    // Intentionally low: sits alongside HUD-ish overlays, not a full-screen exclusive layer.
    float GetLayerDepth() override { return 10.0f; }

protected:
    int SetLine(int nLine);

private:
    void BuildRmlUi();
    void SyncRmlModel();

    static void BindRmlModel(Rml::DataModelConstructor& c, ServerMsgRmlModel& model);
    UI::RmlBridge::ThemedView<ServerMsgRmlModel> m_RmlView{"server_msg", BindRmlModel,
        {{"Data/Interface/RmlUi/server_msg.rml"}}};
};

extern CServerMsgWin g_ServerMsgWin;

#endif // !defined(AFX_SERVERMSGWIN_H__8C6AB678_703D_4A60_B334_C30A97EEC64B__INCLUDED_)
