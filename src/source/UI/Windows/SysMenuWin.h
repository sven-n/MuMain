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

    void RmlClickExitGame() { m_bRmlExitGameClicked = true; }
    void RmlClickSelectServer() { if (m_bSelectServerEnabled) m_bRmlSelectServerClicked = true; }
    void RmlClickOption() { m_bRmlOptionClicked = true; }
    void RmlClickClose() { m_bRmlCloseClicked = true; }

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

    bool m_bRmlExitGameClicked = false;
    bool m_bRmlSelectServerClicked = false;
    bool m_bRmlOptionClicked = false;
    bool m_bRmlCloseClicked = false;

    void SyncRmlModel();
};
