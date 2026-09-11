#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/RmlBridge/RmlModelBinder.h"

namespace Rml { class ElementDocument; }

// RmlUi renders all of this window's visuals; RmlClick*() methods below are bound to the
// document's data-event-click callbacks. ESC toggling is owned by CSceneUICoordinator::Update().
class CSysMenuWin : public mu::ui::window::CObject
{
public:
    CSysMenuWin();
    ~CSysMenuWin() override;

    void Create();
    void Release();
    void Show(bool bShow) override;

    // Act immediately rather than setting a flag for later: this fires from the SDL event pump,
    // always before CSceneUICoordinator::Update() runs the same frame.
    void RmlClickExitGame() { ExitGame(); }
    void RmlClickSelectServer() { if (m_bSelectServerEnabled) SelectServer(); }
    void RmlClickOption() { OpenOptions(); }
    void RmlClickClose() { Close(); }

    // mu::ui::window::IObject
    bool Render() override;
    bool Update() override;
    // Claims all clicks while shown (full-screen overlay); no rect check needed.
    bool UpdateMouseEvent() override
    {
        return !IsVisible();
    }
    bool UpdateKeyEvent() override
    {
        return true;
    }
    // Below CMsgWin: ExitGame() pops a countdown CMsgWin without hiding this menu first, so the
    // countdown must win input priority during that brief overlap.
    float GetLayerDepth() override
    {
        return 40.0f;
    }
    void ReloadRmlTheme() override;

protected:
    void ExitGame();
    void SelectServer();
    void OpenOptions();
    void Close();

private:
    void BuildRmlUi();

    struct SysMenuRmlModel
    {
        // Login scene: fully hidden, not just disabled -- panel is too short for a 4th button slot.
        bool selectServerHidden = false;
        Rml::String systemMenuLabel;
        Rml::String exitGameLabel;
        Rml::String selectServerLabel;
        Rml::String optionLabel;
        Rml::String closeLabel;
    };
    RmlModelBinder<SysMenuRmlModel> m_RmlBinder;
    Rml::ElementDocument* m_pRmlDoc = nullptr;
    bool m_bSelectServerEnabled = false;

    void SyncRmlModel();
};

extern CSysMenuWin g_SysMenuWin;
