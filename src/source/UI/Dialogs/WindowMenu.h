
#if !defined(AFX_NEWUIWINDOWMENU_H__26535D16_A947_4BC3_B129_59F0EFFBA04E__INCLUDED_)
#define AFX_NEWUIWINDOWMENU_H__26535D16_A947_4BC3_B129_59F0EFFBA04E__INCLUDED_

#pragma once

#include "UI/Core/WindowManager.h"
#include "UI/Dialogs/WindowMenuRmlModel.h"
#include "UI/RmlBridge/RmlThemedView.h"

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
class CWindowMenu : public CObject
{
public:
    // The rows, top to bottom.
    enum MenuEntry
    {
        MENU_SYSTEM = 0,
        MENU_HELP,
        MENU_GUILD,
        MENU_MOVE,
        MENU_MINIMAP,
        MENU_GENS,
        MENU_MAX_INDEX,
    };

public:
    CWindowMenu();
    virtual ~CWindowMenu();

    bool Create(CManager* pNewUIMng);
    void Release();


    bool UpdateMouseEvent();
    bool UpdateKeyEvent();
    bool Update();
    bool Render();

    float GetLayerDepth();    //. 10.0f
    float GetKeyEventOrder(); // 10.f;

    void OpenningProcess();
    void ClosingProcess();


private:
    void BuildRmlUi();
    void SyncRmlModel();
    void SyncTransform();
    // What a click on the row did in the original; runs from Update(), outside RmlUi's own event
    // dispatch, since most rows open or close other documents.
    void RunMenuEntry(int entry);

private:
    CManager* m_pNewUIMng;


    void BindRmlModel(Rml::DataModelConstructor& c, WindowMenuRmlModel& model);
    UI::RmlBridge::ThemedView<WindowMenuRmlModel> m_RmlView{"window_menu",
        [this](Rml::DataModelConstructor& c, WindowMenuRmlModel& model) { BindRmlModel(c, model); },
        {{"Data/Interface/RmlUi/window_menu.rml"}}};
    int m_PendingEntry = -1;
};
} // namespace mu::ui::window

#endif // !defined(AFX_NEWUIWINDOWMENU_H__26535D16_A947_4BC3_B129_59F0EFFBA04E__INCLUDED_)
