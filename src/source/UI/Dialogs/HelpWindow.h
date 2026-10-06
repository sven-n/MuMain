
#if !defined(AFX_NEWUIHELPWINDOW_H__9A918DE0_7707_456C_9E5B_89503F1936D1__INCLUDED_)
#define AFX_NEWUIHELPWINDOW_H__9A918DE0_7707_456C_9E5B_89503F1936D1__INCLUDED_

#pragma once

#include "UI/Core/WindowManager.h"
#include "UI/Dialogs/HelpWindowRmlModel.h"
#include "UI/RmlBridge/RmlThemedView.h"
#include "UI/Scaling/UITransform.h"

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
class CHelpWindow : public CObject
{
public:
    CHelpWindow();
    virtual ~CHelpWindow();

    bool Create(CManager* pNewUIMng, int x, int y);
    void Release();

    void SetPos(int x, int y);

    bool UpdateMouseEvent();
    bool UpdateKeyEvent();
    bool Update();
    bool Render();

    float GetLayerDepth();    //. 7.1f
    float GetKeyEventOrder(); // 10.f;

    void OpenningProcess();
    void ClosingProcess();

    void AutoUpdateIndex();

private:
    void BuildRmlUi();
    void SyncRmlModel();
    // Rebuilds the rows and the box when the page or the Dialog transform changed since the last
    // build: the text is measured with the native renderer, so it is not redone every frame.
    void RebuildPageModel(const UI::Scaling::Transform& transform);

    CManager* m_pNewUIMng;
    POINT m_Pos;

    int m_iIndex;

    void BindRmlModel(Rml::DataModelConstructor& c, HelpWindowRmlModel& model);
    void OnRmlReloaded();
    UI::RmlBridge::ThemedView<HelpWindowRmlModel> m_RmlView{"help_window",
        [this](Rml::DataModelConstructor& c, HelpWindowRmlModel& model) { BindRmlModel(c, model); },
        {{"Data/Interface/RmlUi/help_window.rml"}}, {.afterReload = [this] { OnRmlReloaded(); }}};
    int m_BuiltPage = -1;
    UI::Scaling::Transform m_BuiltTransform{};
};
} // namespace mu::ui::window

#endif // !defined(AFX_NEWUIHELPWINDOW_H__9A918DE0_7707_456C_9E5B_89503F1936D1__INCLUDED_)
