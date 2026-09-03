#pragma once

#include "UI/Widgets/WinEx.h"
#include "UI/Widgets/Button.h"
#include "UI/RmlBridge/RmlModelBinder.h"

#define SMW_BTN_GAME_END 0
#define SMW_BTN_SERVER_SEL 1
#define SMW_BTN_OPTION 2
#define SMW_BTN_CLOSE 3
#define SMW_BTN_MAX 4

namespace Rml { class ElementDocument; }

// RmlUi migration, Batch 2: CWin::Create() now passes nTexID=-2 (was the default -1, a real
// full-screen alpha=128 black CWin::m_psprBg) -- the dim backdrop moves to RmlUi (#backdrop in
// base.rcss), and m_winBack stays alive purely for its quantized-height geometry math
// (Create()/SetLine()/GetWidth()/GetHeight()), never rendered. RmlUi renders 100% of this
// window's visuals in every theme; the legacy CButtons stay registered (redundant, harmless
// detection path) alongside Rml*Click*() methods mirroring CLoginWin's pattern.
class CSysMenuWin : public CWin
{
protected:
    CWinEx m_winBack;
    CButton m_aBtn[SMW_BTN_MAX];

public:
    CSysMenuWin();
    virtual ~CSysMenuWin();

    void Create();
    void SetPosition(int nXCoord, int nYCoord);
    void Show(bool bShow);

    // 2026-09-03: act immediately here instead of setting a flag for UpdateWhileActive() to
    // consume later -- see CLoginMainWin::RmlClickMenu()'s header comment (STATUS.md's "Findings
    // worth knowing") for why: UpdateWhileActive() is gated behind CWin::m_bActive, which the
    // legacy CUIMng activation system doesn't reliably grant on a timely basis, and this file's
    // old flag-consumed-in-an-if/else-if-chain shape is exactly the pattern that let a stale flag
    // win over a fresh one there. Confirmed safe to call straight into CUIMng methods here for the
    // same reason as CLoginMainWin's fix: this fires from RmlUiRuntime::ProcessSdlEvent(), called
    // from Winmain's SDL event pump, always before CUIMng::Update() runs the same frame.
    void RmlClickExitGame() { ExitGame(); }
    void RmlClickSelectServer() { if (m_bSelectServerEnabled) SelectServer(); }
    void RmlClickOption() { OpenOptions(); }
    void RmlClickClose() { Close(); }

protected:
    void PreRelease();
    void UpdateWhileActive(double dDeltaTick);
    void RenderControls();

private:
    struct SysMenuRmlModel
    {
        // Login scene: Select Server is fully hidden, not just disabled -- CWinEx's shorter
        // login-scene panel (SetLine(6)) leaves no real room for a 4th button slot, so a
        // disabled-but-still-drawn button visibly collided with Option (see .hidden's comment in
        // base.rcss). Character scene: shown normally.
        bool selectServerHidden = false;
        Rml::String exitGameLabel;
        Rml::String selectServerLabel;
        Rml::String optionLabel;
        Rml::String closeLabel;
    };
    RmlModelBinder<SysMenuRmlModel> m_RmlBinder;
    Rml::ElementDocument* m_pRmlDoc = nullptr;
    bool m_bSelectServerEnabled = false;

    // Shared by the immediate RmlUi callbacks above and UpdateWhileActive()'s legacy
    // CButton::IsClick() polling.
    void ExitGame();
    void SelectServer();
    void OpenOptions();
    void Close();

    void SyncRmlModel();
};
