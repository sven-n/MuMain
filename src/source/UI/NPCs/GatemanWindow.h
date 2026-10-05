
#if !defined(AFX_NEWGATEMANWINDOW_H__F53A1778_D5C8_4EB6_BE74_0A9A16D1FF26__INCLUDED_)
#define AFX_NEWGATEMANWINDOW_H__F53A1778_D5C8_4EB6_BE74_0A9A16D1FF26__INCLUDED_

#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/NPCs/GatemanRmlModel.h"
#include "UI/RmlBridge/RmlModelBinder.h"

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
// The castle gatekeeper, docked right: the guest, guild member or guild master page.
// gateman.rml draws it; C++ keeps the gatekeeper state (CUIGateKeeper), Escape and requests.
class CGatemanWindow : public CObject
{
public:
    // The buttons RmlUi reports (gateman_button(n)).
    enum GATEMAN_BUTTON
    {
        GATEMAN_BUTTON_NONE = -1,
        GATEMAN_BUTTON_ENTER = 0,
        GATEMAN_BUTTON_SET,
        GATEMAN_BUTTON_FEE_UP,
        GATEMAN_BUTTON_FEE_DOWN,
        GATEMAN_BUTTON_EXIT,
    };

private:
    enum
    {
        INVENTORY_WIDTH = 190,
        INVENTORY_HEIGHT = 429,
    };

    CManager* m_pNewUIMng;
    POINT m_Pos;

    RmlModelBinder<GatemanRmlModel> m_RmlBinder;
    Rml::ElementDocument* m_pRmlDoc = nullptr;
    GATEMAN_BUTTON m_PendingButton = GATEMAN_BUTTON_NONE;
    bool m_PendingPublicToggle = false;

public:
    CGatemanWindow();
    virtual ~CGatemanWindow();

    bool Create(CManager* pNewUIMng, int x, int y);
    Rml::ElementDocument* GetFillDocument() const override { return m_pRmlDoc; }
    void Release();

    void SetPos(int x, int y);

    bool UpdateMouseEvent();
    bool UpdateKeyEvent();
    bool Update();
    bool Render();

    void OpeningProcess();
    void ClosingProcess();

    float GetLayerDepth(); //. 5.0f

    void ReloadRmlTheme();

private:
    bool BtnProcess();

    void UpdateGuildMasterMode(GATEMAN_BUTTON button);
    void UpdateGuildMemeberMode(GATEMAN_BUTTON button);
    void UpdateGuestMode(GATEMAN_BUTTON button);

    void BuildRmlUi();
    void SyncRmlModel();
    void SyncContent();
};
}

#endif // !defined(AFX_NEWGATEMANWINDOW_H__F53A1778_D5C8_4EB6_BE74_0A9A16D1FF26__INCLUDED_)
