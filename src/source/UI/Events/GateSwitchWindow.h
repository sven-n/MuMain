
#if !defined(AFX_NEWUIGATESWITCHWINDOW_H__89BA066C_7870_4064_B38E_F2F5AA919F9F__INCLUDED_)
#define AFX_NEWUIGATESWITCHWINDOW_H__89BA066C_7870_4064_B38E_F2F5AA919F9F__INCLUDED_

#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/Events/GateSwitchRmlModel.h"
#include "UI/RmlBridge/RmlThemedView.h"

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


    void BindRmlModel(Rml::DataModelConstructor& c, GateSwitchRmlModel& model);
    UI::RmlBridge::ThemedView<GateSwitchRmlModel> m_RmlView{"gate_switch",
        [this](Rml::DataModelConstructor& c, GateSwitchRmlModel& model) { BindRmlModel(c, model); },
        {{"Data/Interface/RmlUi/gate_switch.rml"}}};
    bool m_PendingToggle = false;
    bool m_PendingExit = false;

public:
    CGateSwitchWindow();
    virtual ~CGateSwitchWindow();

    bool Create(CManager* pNewUIMng);
    Rml::ElementDocument* GetFillDocument() const override { return m_RmlView.Document(); }
    Rml::ElementDocument* GetPlacedDocument() const override { return m_RmlView.Document(); }
    void Release();


    bool UpdateMouseEvent();
    bool UpdateKeyEvent();
    bool Update();
    bool Render();

    void OpeningProcess();
    void ClosingProcess();

    float GetLayerDepth(); //. 5.0f


private:
    void BuildRmlUi();
    void SyncRmlModel();
};
}
#endif // !defined(AFX_NEWUIGATESWITCHWINDOW_H__89BA066C_7870_4064_B38E_F2F5AA919F9F__INCLUDED_)
