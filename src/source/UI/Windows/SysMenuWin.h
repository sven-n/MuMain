#pragma once

#include "UI/NewUI/NewUIBase.h"
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
//
// CUIMng/CNewUIManager merger (docs/newui-legacy-merger.md) Phase 2: migrated off CWin onto
// SEASON3B::CNewUIObj, same pattern as CMsgWin (its own header comment covers the shared
// reasoning -- full-screen click-swallow via UpdateMouseEvent(), LayoutMode::Legacy for the
// legacy CWinEx/CButton real-pixel geometry). ESC is NOT handled here at all (never was --
// UpdateWhileActive()'s own ESC branch below was already a no-op): CUIMng::Update()'s dedicated
// ESC-toggle block owns opening/closing this window directly, so this migration doesn't touch
// that logic or its ordering.
class CSysMenuWin : public SEASON3B::CNewUIObj
{
protected:
    CWinEx m_winBack;
    CButton m_aBtn[SMW_BTN_MAX];

public:
    CSysMenuWin();
    ~CSysMenuWin() override;

    void Create();
    void Release(); // was CWin::PreRelease() (an override hook CWin::Release() called
                     // automatically) -- called explicitly now, same as CCreditWin's own Release().
    void SetPosition(int nXCoord, int nYCoord);
    void Show(bool bShow) override;

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

    // SEASON3B::INewUIBase
    bool Render() override;
    bool Update() override;
    // Was CWin::Create()'s full-screen bounding rect + CWin::CursorInWin(WA_ALL) -- same
    // full-screen click-swallow as CMsgWin (this window is a modal-ish overlay too). See
    // CMsgWin.h's own comment for why this needs no rect check.
    bool UpdateMouseEvent() override
    {
        return !IsVisible();
    }
    bool UpdateKeyEvent() override
    {
        return true;
    }
    // Below CMsgWin's 50.0f: CSysMenuWin::ExitGame() pops up a MESSAGE_GAME_END_COUNTDOWN
    // CMsgWin without hiding itself first (unlike SelectServer()/OpenOptions()/Close(), which all
    // call Show(false)), so the two are a real, if brief, coexistence case -- the actionable
    // countdown dialog should win input priority over the now-superseded menu backdrop.
    float GetLayerDepth() override
    {
        return 40.0f;
    }

protected:
    // Shared by the immediate RmlUi callbacks above and Update()'s legacy CButton::IsClick()
    // polling.
    void ExitGame();
    void SelectServer();
    void OpenOptions();
    void Close();

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

    void SyncRmlModel();
};

// Replaces CUIMng's old `CSysMenuWin m_SysMenuWin;` member, same convention as g_CreditWin.
extern CSysMenuWin g_SysMenuWin;
