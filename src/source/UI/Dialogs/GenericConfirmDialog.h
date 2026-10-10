//*****************************************************************************
// File: GenericConfirmDialog.h
//*****************************************************************************
#pragma once

#include "UI/Core/WindowObject.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlThemedView.h"
#include "UI/Inventory/ItemCameraTarget.h"

#include <RmlUi/Core/EventListener.h>
#include <RmlUi/Core/Types.h>

#include <deque>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
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
        // Always shown -- OK, Create, Buy, Save, etc. Free-form, not fixed to "OK".
        std::wstring primaryLabel = L"OK";

        // Optional real second action, distinct from Cancel -- e.g. Decrease (fruit consume),
        // Present/Gift (IGS buy), Discard. Unset = no middle button (today's Ok-only/Ok+Cancel
        // shape). Never fires from Esc -- only `onCancel` below does.
        std::optional<std::wstring> secondaryLabel;

        // Optional Cancel/Close button -- always dismiss semantics (fires on click AND on Esc).
        // false = no cancel button at all (replaces the old ButtonSet::Ok). `cancelLabel` only
        // overrides the displayed text (e.g. "Close", "No" for a Yes/No-shaped confirm) -- role,
        // position, and Esc-binding stay fixed regardless of what it says.
        bool showCancel = false;
        std::wstring cancelLabel = L"Cancel";

        struct Line
        {
            std::wstring text;
            bool bold = false;
            // The native message box's per-line colour (RGBA(), as AddMsg() took it); 0 = the
            // theme's own line colour.
            unsigned long color = 0;
        };
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
            // buttons -- deliberately NOT an <input type="number">, the shuffling is the point.
            // Text: a stock RmlUi <input> in the dialog's own document (#gcd_input).
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

        // Optional 3D item preview slot (stores a value snapshot), drawn live into #gcd_item3d.
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
        std::function<void()> onSecondary; // fires only if secondaryLabel is set; never Esc-bound
        std::function<void()> onCancel;    // fires on the cancel button AND on Esc
        std::function<bool()> isValid;     // expired requests are dismissed without callbacks

        // Names the dialog for code that answers it as the player would (the control socket).
        std::string tag;
    };

    // Reusable RmlUi confirm dialog -- one document/model, one instance, shown with different
    // GenericDialogConfig content per call. Replacement primitive for CCommonMessageBox/
    // CustomMessageBox's whole native TMsgBoxLayout<> family.
    //
    // Single active instance, not a stack: if Show() is called while one is already open, the new
    // config is queued and shown once the active one resolves -- matches CMsgWin's own single-
    // instance shape.
    class CGenericConfirmDialog : public CObject
    {
    public:
        void Create(CManager* pMng);
        void Release();

        // Shows now if idle, otherwise queues (see class comment).
        using DialogId = std::uint64_t;
        DialogId Show(GenericDialogConfig cfg);
        bool IsPending(DialogId id) const;
        bool IsActive(DialogId id) const { return id != 0 && m_bActive && id == m_ActiveId; }
        void Cancel(DialogId id);
        void SetInputText(DialogId id, const std::wstring& text);
        // Answers the dialog tagged `tag`, open or queued, as its accept or cancel button does;
        // false when there is none.
        bool Answer(std::string_view tag, bool accept);

        // Only meaningful while an `input` field is configured and active; reads the live typed
        // value out of the RmlUi model (Mode::Text) or the on-screen keypad buffer
        // (Mode::NumericKeypad). Call from `onPrimary`.
        std::wstring GetInputText() const;

        // Call from `onPrimary`/`onSecondary`/`onCancel` to veto this click's Resolve() -- the dialog stays
        // open exactly as it was instead of closing/advancing the queue. Typically called after
        // GetInputText() fails validation, right before returning from `onPrimary`.
        void KeepOpen() { m_bKeepOpenRequested = true; }

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
        // CManager::CompareKeyEventOrder (WindowManager.cpp) sorts DESCENDING -- the HIGHEST
        // GetKeyEventOrder() runs FIRST, not the lowest (`return a > b`). CManager::UpdateKeyEvent()
        // stops at the first object whose UpdateKeyEvent() returns false, and while visible this
        // is a true modal (see UpdateMouseEvent() above): it must claim Enter/Escape before every
        // other registered object gets a look, including a plain window's own Esc-to-close (e.g.
        // CMyInventory, which has no such guard and will otherwise close itself first and swallow
        // the keypress). 100.0f is comfortably above every other GetKeyEventOrder() override in
        // the codebase today (highest existing tier is 10.0f, shared by CWindowMenu/
        // CMessageBoxMng/COptionWindow/etc.).
        float GetKeyEventOrder() override { return 100.0f; }

    private:
        void BuildRmlUi();
        void OnRmlBuilt();
        void OnRmlReloaded();
        void SyncRmlModel();
        void ShowNext();            // pops m_Queue (if non-empty) and opens the document
        void Activate(GenericDialogConfig cfg, DialogId id);
        void DismissActive();
        // Which button resolved the dialog -- dispatches to cfg.onPrimary/onSecondary/onCancel.
        enum class ClickResult { Primary, Secondary, Cancel };
        void Resolve(ClickResult which); // hides the document, invokes the chosen callback, then ShowNext()

        void UpdateProgress();      // advances the progress-bar fraction, auto-resolves on elapse
        // Pushes this invocation's Mode::Text settings onto #gcd_input (type/maxlength), then writes
        // `value`: the initial text on Show(), the typed text on a theme reload. This dialog is
        // shared, so the field is reconfigured on every Show().
        void ApplyInputFieldConfig(const Rml::String& value);
        // Focuses the text field once it is shown: it is hidden until the model update that shows
        // its row has run, and a hidden element cannot take the focus.
        void FocusInputWhenShown();
        Rml::String InitialInputText() const;
        void SyncCanvasTop();

        struct LineEntry
        {
            Rml::String text;
            bool bold = false;
            Rml::String color; // CSS colour of Line::color, empty for the theme's own
        };
        struct GenericDialogRmlModel
        {
            std::vector<LineEntry> lines;
            Rml::String primaryLabel;
            // The label is the stock "OK"/"Cancel": a theme may draw native's lettered button art.
            bool primaryIsStockOk = false;

            bool hasSecondary = false;
            Rml::String secondaryLabel;

            bool showCancel = false;
            Rml::String cancelLabel;
            bool cancelIsStockCancel = false;

            bool hasTitle = false;
            Rml::String title;
            bool severityWarning = false;
            bool severityError = false;

            bool hasInput = false;
            bool inputIsKeypad = false;
            Rml::String inputText; // NumericKeypad's masked display text only (its buffer is C++-side,
                                   // m_KeypadBuffer) -- Mode::Text uses inputValue below instead
            // Mode::Text's editable value, two-way bound to #gcd_input's data-value. RmlUi owns the
            // buffer/caret/selection/IME; type (text vs password) and maxlength are pushed per
            // Show() by ApplyInputFieldConfig(), and numeric-only filtering is applied in C++ on
            // change (no RmlUi equivalent for it, and it is an application rule).
            Rml::String inputValue;
            // Individually-bound, not a data-for loop over an array -- click handlers need each
            // row's own index (`gcd_keypad_click(i)`), so 10 fixed slots are bound instead.
            int keypadDigit0 = 0, keypadDigit1 = 0, keypadDigit2 = 0, keypadDigit3 = 0, keypadDigit4 = 0;
            int keypadDigit5 = 0, keypadDigit6 = 0, keypadDigit7 = 0, keypadDigit8 = 0, keypadDigit9 = 0;

            bool hasProgress = false;
            float canvasTop = 0.f; // UI::RmlBridge::DialogCanvasTop, in dp
            float progressFraction = 0.f;

            // Toggles .gcd-item3d's own hidden/shown state and .gcd-body's icon-left/text-
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
        void BindRmlModel(Rml::DataModelConstructor& c, GenericDialogRmlModel& model);
        UI::RmlBridge::ThemedView<GenericDialogRmlModel> m_RmlView{"generic_confirm_dialog",
            [this](Rml::DataModelConstructor& c, GenericDialogRmlModel& model) { BindRmlModel(c, model); },
            {{"Data/Interface/RmlUi/generic_confirm_dialog.rml"}},
            {.modal = Rml::ModalFlag::Modal, .focus = Rml::FocusFlag::Document,
             .afterBuild = [this] { OnRmlBuilt(); }, .afterReload = [this] { OnRmlReloaded(); }}};

        // Set by RmlUi click bindings; polled and cleared in Update() -- never act synchronously
        // inside the RmlUi callback itself.
        bool m_bPrimaryClicked = false;
        bool m_bSecondaryClicked = false;
        bool m_bCancelClicked = false;

        // See KeepOpen()'s own comment. Reset at the top of every Resolve() call, read at the
        // bottom to decide whether to actually close.
        bool m_bKeepOpenRequested = false;

        // NumericKeypad's own click-accumulated digit buffer (never the real keyboard focus) --
        // shuffled mapping gives anti-shoulder-surfing behavior.
        std::wstring m_KeypadBuffer;
        std::vector<int> m_KeypadMapping; // 10 entries, shuffled per Show()

        DWORD m_dwProgressStartTime = 0;
        DWORD m_dwProgressEndTime = 0;

        struct PendingDialog
        {
            DialogId id;
            GenericDialogConfig config;
        };
        std::deque<PendingDialog> m_Queue;
        DialogId m_NextId = 1;
        DialogId m_ActiveId = 0;
        GenericDialogConfig m_Active;
        bool m_bActive = false;
        bool m_bFocusInput = false;

        // item3D, at #gcd_item3d's place and size. Last, so it is destroyed first.
        UI::Items::ItemCameraTarget m_Item3DTarget{
            [this](const Rml::Vector2f& offset, const Rml::Vector2f& size) { RenderItem3DInto(offset, size); }};
        void RenderItem3DInto(const Rml::Vector2f& offset, const Rml::Vector2f& size);
    };

    // Convenience global for the scattered native call sites this primitive replaces (guild/quest
    // UI code, network packet handlers) -- the house convention for a single-instance dialog.
    // Set once, in
    // CSystem::Create() (WindowSystem.cpp); never null after that point during normal play.
    extern CGenericConfirmDialog* g_pGenericConfirmDialog;
}
