//*****************************************************************************
// File: GenericMenuDialog.h
//*****************************************************************************
#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/RmlBridge/RmlModelBinder.h"

#include <RmlUi/Core/Types.h>

#include <deque>
#include <functional>
#include <string>
#include <vector>

namespace Rml { class ElementDocument; }

namespace mu::ui::window
{
    class CManager;

    // Config for one menu dialog. Passed by value to CGenericMenuDialog::Show() -- sibling
    // primitive to CGenericConfirmDialog/GenericDialogConfig, for native "N labeled action
    // buttons" dialogs (CSystemMenuMsgBox and similar, CustomMessageBox.cpp) instead of a plain
    // OK/Cancel confirm. Kept separate from GenericDialogConfig rather than folded in: an
    // arbitrary button list is a different shape from two fixed named slots
    // (primaryLabel/secondaryLabel), and native itself never needs both at once.
    struct GenericMenuConfig
    {
        std::wstring title; // optional heading row above `lines`. Empty = no title row.

        struct Line { std::wstring text; bool bold = false; };
        std::vector<Line> lines; // optional description text above the buttons

        struct MenuButton
        {
            std::wstring label;
            std::wstring tooltip;          // optional hover text; empty = no tooltip
            std::vector<Line> lines;        // optional description text rendered directly above
                                            // this button (not the shared summary above) --
                                            // native dialogs like CChaosMixMenuMsgBox interleave a
                                            // few lines of body text with each individual button
                                            // rather than grouping all of it above the whole list
            bool enabled = true;            // forward-looking -- no current consumer disables a
                                            // button (the one native case that gates an action
                                            // does so server-side, not via a disabled button),
                                            // but it's a one-bool add worth reserving
            bool compact = false;           // true = native's smaller "Cancel/Exit" button style;
                                            // false = the larger action-button style
            std::function<void()> onClick;  // fires, then the dialog closes -- every native
                                            // button in this family destroys its own box on click,
                                            // none need a KeepOpen()-style veto
        };
        std::vector<MenuButton> buttons; // 1..N, data-driven -- no hardcoded max

        // 0 = unspecified (today's default: buttons flow at their normal width, one per row in
        // every current consumer's fixed-width panel -- unchanged behavior). >0 = every button in
        // `buttons` that isn't `compact` is sized to fit exactly that many per row (see
        // generic_menu_dialog.rcss's own `.gmd-btn.cols-2`/`.gmd-button-cell.cols-2`), instead of
        // `compact` being (mis)used as a stand-in for "narrow enough to fit two per row" -- that
        // conflated two unrelated concerns (a small Close/Cancel-style button vs. a same-purpose
        // button grid). Only 2 is implemented (Gem Integration's 10-button jewel-type grid is the
        // only consumer so far); add a new `.cols-N` CSS class + a same-named MenuButtonEntry flag
        // in GenericMenuDialog.cpp the same way if a future consumer needs a different count.
        int columns = 0;

        // Optional, fires on Esc. Decoupled from `buttons` (not "whichever button is last") so a
        // caller can't accidentally rely on button order for cancel semantics.
        std::function<void()> onCancel;
    };

    // Reusable RmlUi menu dialog -- one document/model, one instance, shown with different
    // GenericMenuConfig content per call. Sibling to CGenericConfirmDialog (UI/Dialogs/
    // GenericConfirmDialog.h): same single-active-instance-plus-queue shape, same dp-unit
    // LayoutMode::Dialog panel, same per-theme RCSS fork convention -- but no foreground/
    // background document split (that split exists solely to solve item3D's compositing-order
    // problem; no native consumer of this primitive uses item3D), no native text-input widget
    // seam, no 3D render pass.
    class CGenericMenuDialog : public CObject
    {
    public:
        void Create(CManager* pMng);
        void Release();

        // Shows now if idle, otherwise queues (see class comment).
        void Show(GenericMenuConfig cfg);

        bool Render() override;
        bool Update() override;
        // Unconditionally claims input while shown -- a true modal, matching CMessageBoxMng's own
        // "return false while any box is open" contract.
        bool UpdateMouseEvent() override { return !IsVisible(); }
        bool UpdateKeyEvent() override;
        bool IsVisible() const override { return m_bActive; }
        // Same layer as CGenericConfirmDialog -- these dialogs are never shown simultaneously in
        // practice (both are modal, single-instance), so the exact relative order doesn't matter.
        float GetLayerDepth() override { return 60.0f; }
        // See CGenericConfirmDialog::GetKeyEventOrder()'s own comment for why this must be a HIGH
        // value, not low -- CManager::CompareKeyEventOrder sorts descending (highest runs first),
        // the opposite of what "first priority" suggests. 100.0f, matching that sibling exactly.
        float GetKeyEventOrder() override { return 100.0f; }
        void ReloadRmlTheme() override;

    private:
        void BuildRmlUi();
        void SyncRmlModel();
        void ShowNext();           // pops m_Queue (if non-empty) and opens the document
        void Resolve(int buttonIndex); // -1 = cancel/no button; hides the document, invokes the
                                       // chosen callback, then ShowNext()

        struct LineEntry { Rml::String text; bool bold = false; };
        struct MenuButtonEntry
        {
            Rml::String label;
            Rml::String tooltip;
            std::vector<LineEntry> lines;
            bool hasTooltip = false;
            bool hasLines = false;  // toggles .gmd-button-cell's extra bottom margin (RCSS)
            bool enabled = true;
            bool compact = false;
            bool cols2 = false;     // GenericMenuConfig::columns == 2 && !compact -- see its comment
        };
        struct GenericMenuRmlModel
        {
            bool hasTitle = false;
            Rml::String title;
            std::vector<LineEntry> lines;
            std::vector<MenuButtonEntry> buttons;

            // window_shell's own positioning/dragging extension (added for CGenericMenuDialog's
            // sake as much as option_window.rml's -- the shared template requires every consumer's
            // model to carry these fields even if unused). Never set true here: this dialog stays
            // screen-centered, matching every native menu dialog it replaces.
            bool positioned = false;
            float rootX = 0.f;
            float rootY = 0.f;
        };
        RmlModelBinder<GenericMenuRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;

        bool m_bButtonClicked = false;
        int m_iClickedButtonIndex = -1;

        std::deque<GenericMenuConfig> m_Queue;
        GenericMenuConfig m_Active;
        bool m_bActive = false;
    };

    // Convenience global, same convention as g_pGenericConfirmDialog/g_pUIPopup. Set once, in
    // CSystem::Create() (WindowSystem.cpp); never null after that point during normal play.
    extern CGenericMenuDialog* g_pGenericMenuDialog;
}
