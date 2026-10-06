//*****************************************************************************
// File: CharMakeWin.h
//*****************************************************************************

#if !defined(AFX_CHARMAKEWIN_H__7740CE2F_2BE7_4705_91DD_CCF55256B1D3__INCLUDED_)
#define AFX_CHARMAKEWIN_H__7740CE2F_2BE7_4705_91DD_CCF55256B1D3__INCLUDED_

#pragma once

#include "UI/Core/WindowObject.h"
#include "Render/Sprites/Sprite.h"
#include "UI/RmlBridge/RmlRenderTarget.h"
#include "UI/RmlBridge/RmlThemedView.h"

#include <vector>

#define CMW_SPR_INPUT 0
#define CMW_SPR_STAT 1
#define CMW_SPR_DESC 2
#define CMW_SPR_MAX 3

#define CMW_DESC_LINE_MAX 2
#define CMW_DESC_ROW_MAX 75

namespace Rml { class ElementDocument; }

// The character-creation dialog. Its live 3D preview is drawn into a render target the document
// shows as #preview (UI::RmlBridge::RenderTarget), so it stands at the dialog's own depth: the
// character info balloons of the scene behind, a lower document, stay under it and under the
// dialog's dimming backdrop. All 2D chrome (job buttons, stat/description panels, the name-input
// frame, OK/Cancel) renders via RmlUi; m_aJobState below holds only the checked/enabled state
// RmlUi's job-list binding reads, no rendering or click-detection of its own.
//
// The name field is a stock RmlUi <input> in the same document (see CharMakeRmlModel::charName), so
// nothing about it is drawn natively and there is no post-RmlUi render seam for this window.
//
// The document's #backdrop dims the whole screen; UpdateMouseEvent() unconditionally claims the
// click while shown, matching that full-screen-modal intent.
class CCharMakeWin : public mu::ui::window::CObject
{
protected:
    CSprite m_asprBack[CMW_SPR_MAX];

    // Checked/enabled state for each job button, read by SyncRmlModel() into RmlUi's "jobs"
    // binding -- RmlUi owns the buttons' rendering and click detection, this is just the model.
    struct JobButtonState
    {
        bool checked = false;
        bool enabled = true;
    };
    JobButtonState m_aJobState[MAX_CLASS];

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

    // Invoked from the RmlUi document's data-event-click bindings (see Create()). Act
    // immediately here instead of setting a flag for UpdateWhileActive() to consume later -- see
    // CLoginMainWin::RmlClickMenu()'s header comment for
    // why: UpdateWhileActive() is gated behind CWin::m_bActive, which the legacy CUIMng
    // activation system doesn't reliably grant on a timely basis. Confirmed safe to call straight
    // into the action here for the same reason as CLoginMainWin's fix: this fires from
    // RmlUiRuntime::ProcessSdlEvent(), called from Winmain's SDL event pump, always before
    // CSceneUICoordinator::Update() runs the same frame.
    void RmlClickJob(int nClassIndex);
    void RmlClickOk() { SubmitCreateCharacter(); }
    void RmlClickCancel() { CloseDialog(); }

    // mu::ui::window::IObject
    bool Render() override;
    bool Update() override;
    // Genuinely modal (see this class's header comment) -- same full-screen click-swallow as
    // CMsgWin/CSysMenuWin.
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
    // The target's drawer: the selected class's character, framed for `width` x `height`.
    void RenderPreviewInto(std::uint32_t width, std::uint32_t height);
    // Sizes the target to #preview and points #preview at it.
    void SyncPreview();

private:
    void OnRmlReloaded();
    void ApplyNameLimit();

    struct JobButtonEntry
    {
        // Which class this button is, for the stylesheets to place it by ([job=...]). The set and
        // its order are fixed (MAX_CLASS entries, CLASS_* order), so every position is static.
        Rml::String key;
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

        // Only matters for the modern theme (legacy hides it -- its description panel never had a
        // separate title line, just the two body lines below). The selected class's own name,
        // same I18N lookup table as jobs[i].label -- see SyncRmlModel()'s own comment.
        Rml::String descTitle;
        Rml::String descLine1;
        Rml::String descLine2;

        // Only matters for the modern theme (legacy hides it -- real sprite art already gives
        // OK/Cancel a visual identity, see themes/legacy/char_make.rcss's `color: transparent`).
        // Same I18N::Game::OK/Cancel + {{ok_label}}/{{cancel_label}} pattern as LoginWin.cpp.
        Rml::String okLabel, cancelLabel;

        // The typed character name, two-way bound to char_make.rml's <input data-value="char_name">.
        // RmlUi owns the edit buffer/caret/selection/IME; this is only the committed value, copied
        // into the legacy InputText[0] buffer at submit time so CheckSpecialText()/
        // SendCreateCharacter() keep their existing contract. Length cap comes from C++
        // (ApplyNameLimit()).
        Rml::String charName;
    };
    void BindRmlModel(Rml::DataModelConstructor& c, CharMakeRmlModel& model);
    UI::RmlBridge::ThemedView<CharMakeRmlModel> m_RmlView{"char_make",
        [this](Rml::DataModelConstructor& c, CharMakeRmlModel& model) { BindRmlModel(c, model); },
        {{"Data/Interface/RmlUi/char_make.rml"}},
        {.afterBuild = [this] { ApplyNameLimit(); }, .afterReload = [this] { OnRmlReloaded(); }}};

    int m_nOriginX = 0;
    int m_nOriginY = 0;

    // Last, so it is destroyed first and its drawer never runs against a half-destroyed window.
    UI::RmlBridge::RenderTarget m_PreviewTarget{
        [this](std::uint32_t width, std::uint32_t height) { RenderPreviewInto(width, height); }};

    void SelectJob(int classIndex);
    void SubmitCreateCharacter();
    void CloseDialog();

    void SyncRmlModel();
};

// Replaces CUIMng's old `CCharMakeWin m_CharMakeWin;` member, same convention as g_CreditWin.
extern CCharMakeWin g_CharMakeWin;

#endif // !defined(AFX_CHARMAKEWIN_H__7740CE2F_2BE7_4705_91DD_CCF55256B1D3__INCLUDED_)
