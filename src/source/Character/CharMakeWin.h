//*****************************************************************************
// File: CharMakeWin.h
//*****************************************************************************

#if !defined(AFX_CHARMAKEWIN_H__7740CE2F_2BE7_4705_91DD_CCF55256B1D3__INCLUDED_)
#define AFX_CHARMAKEWIN_H__7740CE2F_2BE7_4705_91DD_CCF55256B1D3__INCLUDED_

#pragma once

#include "UI/NewUI/NewUIBase.h"
#include "UI/Widgets/Win.h"
#include "UI/Widgets/Button.h"
#include "UI/RmlBridge/RmlModelBinder.h"

#include <vector>

#define CMW_SPR_INPUT 0
#define CMW_SPR_STAT 1
#define CMW_SPR_DESC 2
#define CMW_SPR_MAX 3

#define CMW_DESC_LINE_MAX 2
#define CMW_DESC_ROW_MAX 75

namespace Rml { class ElementDocument; }

// RmlUi migration: the character-creation dialog. Unlike every other migrated window, this one
// has a genuine live 3D preview (RenderCreateCharacter() -- untouched, still calls
// BeginOpengl()/RenderCharacter()/EndOpengl() directly, the same mechanism the login screen's 3D
// tour camera and the character-select scene's own character rendering already use). That call
// stays exactly where it is, in RenderControls(), during the normal per-frame legacy-2D-content-
// recording phase -- RmlUi renders last in the frame regardless (see
// docs/rmlui-ui-system/README.md), so as long as the RmlUi panel has no opaque background over
// the 410x335 preview viewport, the 3D content composites correctly underneath the RmlUi chrome
// around it, the same proven trick the login screen's tour camera already relies on. Only the 2D
// chrome (job buttons, stat/description panels, the name-input frame, OK/Cancel) moves to RmlUi;
// m_winBack itself was already nTexID=-2 (drew nothing) even before this migration.
//
// The name input text itself stays on the legacy CUITextInputBox/RenderInputText path (matching
// CLoginWin's precedent, not RmlUi's native <input>) -- drawn via RenderTextOnTop(), called from
// Winmain.cpp's SetPostRmlUiCallback (already registered for CHARACTER_SCENE) so it's guaranteed
// to render after RmlUi's own input-frame background regardless of theme.
//
// CUIMng/CNewUIManager merger (docs/newui-legacy-merger.md) Phase 2: migrated off CWin onto
// SEASON3B::CNewUIObj. Unlike CMsgWin/CSysMenuWin (which pass nTexID=-2 and rely entirely on
// RmlUi's own #backdrop for dimming), this window's own base-class CWin::Create() call used the
// *default* nTexID=-1 -- a real, visible full-screen semi-transparent black CWin::m_psprBg dimming
// overlay, genuinely rendered every frame (unlike m_winBack's own -2). Ported as an explicit
// m_sprBg member (same convention CCreditWin's own m_sprBg replaced CWin::m_psprBg with), rendered
// first in Render() -- dropping it would be a real, visible regression (character creation losing
// its dimmed background), not a redundant-bookkeeping cleanup like CMsgWin/CSysMenuWin's case.
// UpdateMouseEvent() unconditionally claims the click while shown, matching that genuine full-
// screen-modal intent.
class CCharMakeWin : public SEASON3B::CNewUIObj
{
protected:
    // Replaces CWin::m_psprBg -- see this class's own header comment above for why (unlike
    // CMsgWin/CSysMenuWin) this one is real, visible content that still needs rendering.
    CSprite m_sprBg;
    CWin m_winBack;
    CSprite m_asprBack[CMW_SPR_MAX];
    CButton m_abtnJob[MAX_CLASS];
    CButton m_aBtn[2];

    CLASS_TYPE m_nSelJob;
    wchar_t m_aszJobDesc[CMW_DESC_LINE_MAX][CMW_DESC_ROW_MAX];
    int m_nDescLine;

public:
    CCharMakeWin();
    ~CCharMakeWin() override;

    void Create();
    void Release(); // was CWin::PreRelease() (an override hook CWin::Release() called
                     // automatically) -- called explicitly now, same as CCreditWin's own Release().
    void SetPosition(int nXCoord, int nYCoord);
    void Show(bool bShow) override;
    void UpdateDisplay();

    // Invoked from the RmlUi document's data-event-click bindings (see Create()). 2026-09-03: act
    // immediately here instead of setting a flag for UpdateWhileActive() to consume later -- see
    // CLoginMainWin::RmlClickMenu()'s header comment (STATUS.md's "Findings worth knowing") for
    // why: UpdateWhileActive() is gated behind CWin::m_bActive, which the legacy CUIMng
    // activation system doesn't reliably grant on a timely basis. Confirmed safe to call straight
    // into the action here for the same reason as CLoginMainWin's fix: this fires from
    // RmlUiRuntime::ProcessSdlEvent(), called from Winmain's SDL event pump, always before
    // CUIMng::Update() runs the same frame.
    void RmlClickJob(int nClassIndex);
    void RmlClickOk() { SubmitCreateCharacter(); }
    void RmlClickCancel() { CloseDialog(); }

    // Draws the legacy name-input text on top of RmlUi's input-frame background. Called from
    // Winmain.cpp's SetPostRmlUiCallback (already registered for CHARACTER_SCENE) so it renders
    // after RmlUi's own pass, guaranteed correct regardless of theme -- same pattern as
    // CLoginWin::RenderTextOnTop().
    void RenderTextOnTop();

    // SEASON3B::INewUIBase
    bool Render() override;
    bool Update() override;
    // Was CWin::Create()'s (default nTexID=-1, real full-screen dimming) bounding rect +
    // CWin::CursorInWin(WA_ALL) -- genuinely modal (see this class's header comment), same
    // full-screen click-swallow as CMsgWin/CSysMenuWin.
    bool UpdateMouseEvent() override
    {
        return !IsVisible();
    }
    bool UpdateKeyEvent() override
    {
        return true;
    }
    // Below CMsgWin's 50.0f: RequestCreateCharacter()'s validation-failure paths (name too short/
    // invalid/special) pop up a CMsgWin without hiding this dialog first, so the two are a real
    // coexistence case -- the actionable message should win input priority. Above CSysMenuWin's
    // 40.0f/CCharSelMainWin's 15.0f, though neither actually coexists with this window in
    // practice once CCharSelMainWin's own Update() gate (see its header comment) is in place.
    float GetLayerDepth() override
    {
        return 45.0f;
    }

protected:
    void RequestCreateCharacter();

    void SelectCreateCharacter();
    void UpdateCreateCharacter();
    void RenderCreateCharacter();

private:
    struct JobButtonEntry
    {
        // Panel-relative, pushed once from SetPosition() (mirrors the legacy per-class layout
        // math exactly -- not recomputed per frame, unlike CCharInfoBalloon's genuine 3D-tracking
        // case).
        int relLeft = 0;
        int relTop = 0;
        bool checked = false;
        bool disabled = false;
        Rml::String label;
    };
    struct CharMakeRmlModel
    {
        std::vector<JobButtonEntry> jobs;

        bool darkLordExtra = false;
        Rml::String statLabel0, statLabel1, statLabel2, statLabel3, statLabel4;
        Rml::String statValue0, statValue1, statValue2, statValue3;

        Rml::String descLine1;
        Rml::String descLine2;

        // Only matters for the modern theme (legacy hides it -- real sprite art already gives
        // OK/Cancel a visual identity, see themes/legacy/char_make.rcss's `color: transparent`).
        // Same I18N::Game::OK/Cancel + {{ok_label}}/{{cancel_label}} pattern as LoginWin.cpp.
        Rml::String okLabel, cancelLabel;
    };
    RmlModelBinder<CharMakeRmlModel> m_RmlBinder;
    Rml::ElementDocument* m_pRmlDoc = nullptr;

    int m_nOriginX = 0;
    int m_nOriginY = 0;

    // Shared by the immediate RmlUi callbacks above and Update()'s legacy CButton::IsClick()
    // polling.
    void SelectJob(int classIndex);
    void SubmitCreateCharacter();
    void CloseDialog();

    void SyncRmlModel();
};

// Replaces CUIMng's old `CCharMakeWin m_CharMakeWin;` member, same convention as g_CreditWin.
extern CCharMakeWin g_CharMakeWin;

#endif // !defined(AFX_CHARMAKEWIN_H__7740CE2F_2BE7_4705_91DD_CCF55256B1D3__INCLUDED_)
