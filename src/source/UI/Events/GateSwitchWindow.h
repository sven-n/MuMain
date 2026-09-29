
#if !defined(AFX_NEWUIGATESWITCHWINDOW_H__89BA066C_7870_4064_B38E_F2F5AA919F9F__INCLUDED_)
#define AFX_NEWUIGATESWITCHWINDOW_H__89BA066C_7870_4064_B38E_F2F5AA919F9F__INCLUDED_

#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/Events/GateSwitchRmlModel.h"
#include "UI/RmlBridge/RmlModelBinder.h"

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
// The castle gate switch, docked right. gate_switch.rml draws it; C++ keeps the gate state,
// the corner close, Escape and the toggle request.
class CGateSwitchWindow : public CObject
{
private:
    enum
    {
        INVENTORY_WIDTH = 190,
        INVENTORY_HEIGHT = 429,
    };

    CManager* m_pNewUIMng;
    POINT m_Pos;

    RmlModelBinder<GateSwitchRmlModel> m_RmlBinder;
    Rml::ElementDocument* m_pRmlDoc = nullptr;
    bool m_PendingToggle = false;
    bool m_PendingExit = false;

public:
    CGateSwitchWindow();
    virtual ~CGateSwitchWindow();

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

    void ReloadRmlTheme();

private:
    bool BtnProcess();
    void BuildRmlUi();
    void SyncRmlModel();
};
}
#endif // !defined(AFX_NEWUIGATESWITCHWINDOW_H__89BA066C_7870_4064_B38E_F2F5AA919F9F__INCLUDED_)
