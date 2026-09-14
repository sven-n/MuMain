//*****************************************************************************
// File: GenericConfirmDialog.h
//*****************************************************************************
#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Core/Window3DRenderMng.h"
#include "UI/RmlBridge/RmlModelBinder.h"

#include <RmlUi/Core/Types.h>

#include <deque>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace Rml { class ElementDocument; }

namespace mu::ui::window
{
    class CManager;

    // Config for one confirm dialog. Passed by value to CGenericConfirmDialog::Show() -- a new
    // confirm dialog is a value of this struct, not a new C++ class (contrast with
    // CCommonMessageBox's ~140 TMsgBoxLayout<> subclasses, one per dialog).
    //
    // `title`/`input`/`progress`/`item3D`/`portrait2D`/`tallPanel` are optional: unset/false
    // reproduces the original plain-text/OK-Cancel shape. Deliberately flat optional fields on one
    // struct, not a std::variant of dialog "kinds" -- native rarely combines more than one per
    // dialog, and a variant would make the common plain-text case syntactically heavier.
    struct GenericDialogConfig
    {
        enum class ButtonSet { Ok, OkCancel };
        ButtonSet buttons = ButtonSet::Ok;
        std::wstring primaryLabel = L"OK";
        std::wstring secondaryLabel = L"Cancel"; // ignored unless buttons == OkCancel

        struct Line { std::wstring text; bool bold = false; };
        std::vector<Line> lines;

        // Optional bold title row above `lines`, fixed position/font, never wrapped. Empty = no
        // title row.
        std::wstring title;

        // Optional severity, purely a look-and-feel hook (border/accent tint) -- does not gate
        // behavior. Normal is visually identical to the plain panel.
        enum class Severity { Normal, Warning, Error };
        Severity severity = Severity::Normal;

        // Optional single text-entry field, rendered between `lines` and the button row. Unset =
        // no field. The typed value is read back via CGenericConfirmDialog::GetInputText(),
        // called from `onPrimary`.
        struct InputField
        {
            // NumericKeypad: shuffled on-screen digit pad (anti-keylogger PIN entry), pure RmlUi
            // buttons. Text: a real keyboard field hosted via the shared g_pSingleTextInputBox
            // portable widget, positioned from an RmlUi anchor element.
            enum class Mode { Text, NumericKeypad };
            Mode mode = Mode::Text;
            int maxLength = 20;
            bool masked = false;       // password-style masking (Text mode only -- NumericKeypad
                                       // is unconditionally masked)
            bool numericOnly = false;  // Text mode only; NumericKeypad is always digits-only
            std::wstring initialText;
        };
        std::optional<InputField> input;

        // Optional progress bar / timed auto-close. Deliberately minimal: every real call site is
        // fire-and-forget, so this is just a duration. When set, the button row is hidden
        // entirely; `onPrimary`, if set, fires once when the timer elapses.
        struct Progress { DWORD elapseMs = 3000; };
        std::optional<Progress> progress;

        // Optional 3D item preview slot (stores a value snapshot). Rendered via
        // I3DRenderObj/Render3D() -- see the class comment for why that renders on top of the
        // panel instead of behind it (the fg/bg document split).
        std::optional<ITEM> item3D;

        // Optional 2D image portrait -- a flat sprite with a short caption, as opposed to
        // item3D's live-rendered 3D icon.
        struct Portrait2D
        {
            std::wstring text; // only rendered in Overlay mode -- Beside mode has no built-in
                               // caption (add one via `lines` instead), same as item3D.
            // Overlay (default): sprite sits above `lines` with `text` drawn on top of it. Beside:
            // sprite sits to the left of `lines`, the same icon-left/text-right slot item3D uses.
            enum class Layout { Overlay, Beside };
            Layout layout = Layout::Overlay;
        };
        std::optional<Portrait2D> portrait2D;

        // Optional: grows #panel taller (both fg/bg documents, both themes) via a "tall" CSS
        // class, instead of relying on .gcd-text-col's own scrollbar -- for dialogs whose content
        // doesn't comfortably fit the default panel height where scrolling mid-interaction is bad
        // UX (a portrait2D, or Mode::NumericKeypad's digit pad). false (default) keeps today's size.
        bool tallPanel = false;

        std::function<void()> onPrimary;   // OK / Enter / progress-timer-elapsed
        std::function<void()> onSecondary; // Cancel / Esc -- optional, safe to leave empty
    };

    // Reusable RmlUi confirm dialog -- one document/model, one instance, shown with different
    // GenericDialogConfig content per call. Replacement primitive for CCommonMessageBox/
    // CustomMessageBox's whole native TMsgBoxLayout<> family (see STATUS.md).
    //
    // Single active instance, not a stack: if Show() is called while one is already open, the new
    // config is queued and shown once the active one resolves -- matches CMsgWin's own single-
    // instance shape.
    //
    // Also an I3DRenderObj: only actually registers 3D-render behavior while a dialog with
    // `item3D` set is active (see Render3D()'s own guard).
    //
    // item3D renders on top of the panel via a foreground/background RmlUi document split, not a
    // post-RmlUi callback. RmlUi's main context always composites LAST in the frame, strictly
    // after the item3D render pass -- so with only one document, the item always rendered
    // *behind* the panel's own opaque background regardless of z-order. Fixed by moving the
    // panel's own background art to a second document (`m_pRmlBgDoc`) in its own dedicated
    // context, rendered by RmlUiRuntime::RenderDialogBackgroundLayer() at a point CManager::Render()
    // guarantees is after every ordinary window's own 2D Render() this frame but strictly before
    // item3D itself draws (sorted by GetLayerDepth(), just below the shared 3D camera's own
    // z-order). This document (the foreground one, still the sole owner of
    // `data-model="generic_confirm_dialog"`) is paint-less where the background used to be, so it
    // no longer covers the item once the dedicated context has already painted it earlier the
    // same frame. See generic_confirm_dialog_bg.rml (both themes) for the actual markup split.
    class CGenericConfirmDialog : public CObject, public I3DRenderObj
    {
    public:
        void Create(CManager* pMng);
        void Release();

        // Shows now if idle, otherwise queues (see class comment).
        void Show(GenericDialogConfig cfg);

        // Only meaningful while an `input` field is configured and active; reads the live typed
        // value out of the RmlUi model (Mode::Text) or the on-screen keypad buffer
        // (Mode::NumericKeypad). Call from `onPrimary`.
        std::wstring GetInputText() const;

        // Call from `onPrimary`/`onSecondary` to veto this click's Resolve() -- the dialog stays
        // open exactly as it was instead of closing/advancing the queue. Typically called after
        // GetInputText() fails validation, right before returning from `onPrimary`.
        void KeepOpen() { m_bKeepOpenRequested = true; }

        // Called from Winmain.cpp's SetPostRmlUiCallback, after RmlUi's main context composites --
        // NOT from the normal CManager-driven Render() below (that always runs before RmlUi's own
        // composite, so anything drawn there gets painted over by #panel's opaque background).
        void RenderTextOnTop(); // Mode::Text's g_pSingleTextInputBox widget

        bool Render() override;
        bool Update() override;
        // Unconditionally claims input while shown -- a true modal.
        bool UpdateMouseEvent() override { return !IsVisible(); }
        bool UpdateKeyEvent() override;
        // CObject::IsVisible() reflects Show(bool)/m_bRender, which this class never touches --
        // visibility here is purely "is a dialog currently active."
        bool IsVisible() const override { return m_bActive; }
        // Above CMsgWin's 50.0f -- a confirm dialog should sit on top of an ordinary message window.
        float GetLayerDepth() override { return 60.0f; }
        float GetKeyEventOrder() override { return 10.0f; }
        void ReloadRmlTheme() override;

        // I3DRenderObj. Window3DRenderMng.cpp's shared render loop dynamic_casts each registered
        // object to CObject first and uses its own GetLayoutMode() when that succeeds -- true
        // here, so this object's Dialog layout transform is pushed automatically before Render3D()
        // runs.
        void Render3D() override;

    private:
        void BuildRmlUi();
        void SyncRmlModel();
        void ShowNext();            // pops m_Queue (if non-empty) and opens the document
        void Resolve(bool primary); // hides the document, invokes the chosen callback, then ShowNext()

        void UpdateProgress();      // advances the progress-bar fraction, auto-resolves on elapse
        void UpdateTextInputWidget();  // per-frame Configure()/GiveFocus()/DoAction() for Mode::Text

        // #panel is centered via `.center-both` (`left:50%; top:50%; transform:translate(-50%,-50%)`,
        // base.rcss) -- GetAbsoluteOffset() walks the ancestor chain summing offsets but does NOT
        // apply CSS `transform` at any level, so every element inside #panel is off by half the
        // panel's own size. Add this correction to any of #panel's descendants' own
        // GetAbsoluteOffset() before treating it as a real screen position. Returns {0,0} if
        // #panel can't be found.
        Rml::Vector2f PanelTranslateCorrection() const;

        struct LineEntry { Rml::String text; bool bold = false; };
        struct GenericDialogRmlModel
        {
            std::vector<LineEntry> lines;
            bool showCancel = false;
            Rml::String primaryLabel;
            Rml::String secondaryLabel;

            bool hasTitle = false;
            Rml::String title;
            bool severityWarning = false;
            bool severityError = false;

            bool hasInput = false;
            bool inputIsKeypad = false;
            Rml::String inputText; // display text (already masked, if applicable) for NumericKeypad;
                                   // unused for Mode::Text, which renders via the native widget
            // Individually-bound, not a data-for loop over an array -- click handlers need each
            // row's own index (`gcd_keypad_click(i)`), so 10 fixed slots are bound instead.
            int keypadDigit0 = 0, keypadDigit1 = 0, keypadDigit2 = 0, keypadDigit3 = 0, keypadDigit4 = 0;
            int keypadDigit5 = 0, keypadDigit6 = 0, keypadDigit7 = 0, keypadDigit8 = 0, keypadDigit9 = 0;

            bool hasProgress = false;
            float progressFraction = 0.f;

            // Toggles #gcd_item3d_anchor's own hidden/shown state and .gcd-body's icon-left/text-
            // right layout -- the item icon sits at a fixed offset with body text starting to its
            // right, side-by-side rather than stacked.
            bool hasItem3D = false;

            // Toggles .gcd-portrait2d's own Overlay-vs-Beside slot -- see
            // GenericDialogConfig::Portrait2D's own comment. Two plain flat bools rather than one
            // field compared against an enum in the markup, matching this model's own convention
            // (e.g. severityWarning/severityError above).
            bool hasPortrait2DOverlay = false;
            bool hasPortrait2DBeside = false;
            Rml::String portrait2DText; // Overlay mode only

            // Mirrors GenericDialogConfig::tallPanel onto #panel's own "tall" class (fg document;
            // the bg document's #panel has no data model of its own, so it's kept in sync
            // imperatively instead, see Show()/ShowNext()).
            bool hasTallPanel = false;
        };
        RmlModelBinder<GenericDialogRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;

        // Background-context companion to m_pRmlDoc -- see the class comment for the mechanism.
        // No RmlModelBinder of its own: it's 100% static markup (this dialog centers via plain
        // CSS, never a per-frame C++-computed position). Lives in its OWN dedicated context
        // (RmlUiRuntime::GetDialogBackgroundContext()), not the shared one every ordinary window's
        // own bg doc uses -- a shared context renders once globally per frame, which would let
        // other windows' own foreground content paint over this dialog wherever they overlap.
        // Shown/Hidden in lockstep with m_pRmlDoc at every transition point.
        Rml::ElementDocument* m_pRmlBgDoc = nullptr;

        // Set by RmlUi click bindings; polled and cleared in Update() -- never act synchronously
        // inside the RmlUi callback itself.
        bool m_bPrimaryClicked = false;
        bool m_bSecondaryClicked = false;

        // See KeepOpen()'s own comment. Reset at the top of every Resolve() call, read at the
        // bottom to decide whether to actually close.
        bool m_bKeepOpenRequested = false;

        // NumericKeypad's own click-accumulated digit buffer (never the real keyboard focus) --
        // shuffled mapping gives anti-shoulder-surfing behavior.
        std::wstring m_KeypadBuffer;
        std::vector<int> m_KeypadMapping; // 10 entries, shuffled per Show()

        DWORD m_dwProgressStartTime = 0;
        DWORD m_dwProgressEndTime = 0;

        std::deque<GenericDialogConfig> m_Queue;
        GenericDialogConfig m_Active;
        bool m_bActive = false;

        // Logs Render3D()'s anchor position/transform/final coordinates once per Show(), so an
        // item3D mismatch can be read out of MuError.log instead of guessed at blind. Cheap enough
        // to leave in permanently.
        bool m_bItem3DDebugLogged = false;
    };

    // Convenience global for the scattered native call sites this primitive replaces (guild/quest
    // UI code, network packet handlers) -- same convention as g_pUIPopup. Set once, in
    // CSystem::Create() (WindowSystem.cpp); never null after that point during normal play.
    extern CGenericConfirmDialog* g_pGenericConfirmDialog;
}
