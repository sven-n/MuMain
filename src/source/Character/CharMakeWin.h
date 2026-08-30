//*****************************************************************************
// File: CharMakeWin.h
//*****************************************************************************

#if !defined(AFX_CHARMAKEWIN_H__7740CE2F_2BE7_4705_91DD_CCF55256B1D3__INCLUDED_)
#define AFX_CHARMAKEWIN_H__7740CE2F_2BE7_4705_91DD_CCF55256B1D3__INCLUDED_

#pragma once

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
class CCharMakeWin : public CWin
{
protected:
    CWin m_winBack;
    CSprite m_asprBack[CMW_SPR_MAX];
    CButton m_abtnJob[MAX_CLASS];
    CButton m_aBtn[2];

    CLASS_TYPE m_nSelJob;
    wchar_t m_aszJobDesc[CMW_DESC_LINE_MAX][CMW_DESC_ROW_MAX];
    int m_nDescLine;

public:
    CCharMakeWin();
    virtual ~CCharMakeWin();

    void Create();
    void SetPosition(int nXCoord, int nYCoord);
    void Show(bool bShow);
    void UpdateDisplay();

    // Invoked from the RmlUi document's data-event-click bindings (see Create()). Polled-and-
    // cleared exactly like every other migrated window's RmlClickX() pattern.
    void RmlClickJob(int nClassIndex);
    void RmlClickOk() { m_bRmlOkClicked = true; }
    void RmlClickCancel() { m_bRmlCancelClicked = true; }

    // Draws the legacy name-input text on top of RmlUi's input-frame background. Called from
    // Winmain.cpp's SetPostRmlUiCallback (already registered for CHARACTER_SCENE) so it renders
    // after RmlUi's own pass, guaranteed correct regardless of theme -- same pattern as
    // CLoginWin::RenderTextOnTop().
    void RenderTextOnTop();

protected:
    void PreRelease();
    void UpdateWhileActive(double dDeltaTick);
    void RenderControls();

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
    };
    RmlModelBinder<CharMakeRmlModel> m_RmlBinder;
    Rml::ElementDocument* m_pRmlDoc = nullptr;

    int m_nOriginX = 0;
    int m_nOriginY = 0;

    int m_nRmlJobClickedIndex = -1;
    bool m_bRmlOkClicked = false;
    bool m_bRmlCancelClicked = false;

    void SyncRmlModel();
};

#endif // !defined(AFX_CHARMAKEWIN_H__7740CE2F_2BE7_4705_91DD_CCF55256B1D3__INCLUDED_)
