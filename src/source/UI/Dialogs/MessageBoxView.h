#pragma once

#include "UI/Dialogs/MessageBoxViewRmlModel.h"
#include "UI/RmlBridge/RmlThemedView.h"

#include <string>
#include <vector>

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
// The RmlUi side of a native message box (a CMessageBoxBase) drawn as newui_msgbox_top, 15-unit
// middle strips and newui_msgbox_bottom over the newui_msgbox_back fill, with text lines and
// newui_btn_empty_small or newui_button_ok buttons, or a progress bar (CGuild_ToPerson_Position, CProgressMsgBox,
// the event result boxes). One box at a time. The
// box owns it while it exists and keeps its callbacks: a button RmlUi reports is taken with TakePressedButton() and
// sent as the box's own event.
class MessageBoxView
{
public:
    struct Cell
    {
        std::wstring text;
        bool bold = false;
        DWORD color = 0;
        float textPx = 0.f; // physical px; 0: the font's native size
    };

    // A line: what it is (the theme spaces and places it by its role and the box's kind), and its
    // text, or a table row's columns.
    struct Line
    {
        const char* role = "message";
        std::vector<Cell> cells;
    };

    // The theme places each button by the box's kind.
    struct Button
    {
        std::wstring label;
        bool enabled = true;
        bool okArt = false; // newui_button_ok's lettered art instead of a labelled newui_btn_empty_small
    };

    using List = std::vector<MessageBoxViewListRowEntry>;

    MessageBoxView();
    ~MessageBoxView();
    MessageBoxView(const MessageBoxView&) = delete;
    MessageBoxView& operator=(const MessageBoxView&) = delete;

    // `kind` names the box: the theme places it, its lines and its buttons by it.
    void Create(int middleCount, const char* kind);
    // A box whose size changes after Create() (CProgressMsgBox grows with its text). With
    // middlesAboveDivider >= 0 the 21-unit newui_Message_Line follows that many middle strips
    // (CDevilSquareRankMsgBox).
    void SetFrame(int middleCount, int middlesAboveDivider = -1);
    // CProgressMsgBox's bar, `fraction` (0 .. 1) filled.
    void SetProgress(float fraction);
    void Destroy();

    // Per frame: the lines and buttons. The theme places the box on the stage by its kind.
    void Sync(const std::vector<Line>& lines, const std::vector<Button>& buttons);
    // The box's list, or none (nullptr); call with Sync().
    void SyncList(const List* list);

    // The document is loaded: the box draws nothing natively.
    bool IsShown() const
    {
        return m_View.Document() != nullptr;
    }

    // #panel, or null without the document.
    Rml::Element* Panel() const;

    int TakePressedButton();
    int TakePressedListRow();

private:
    void BindModel(Rml::DataModelConstructor& c, MessageBoxViewRmlModel& model);
    void BindList(Rml::DataModelConstructor& constructor, MessageBoxViewRmlModel& model);
    UI::RmlBridge::ThemedView<MessageBoxViewRmlModel> m_View;
    int m_PressedButton = -1;
    int m_PressedListRow = -1;
};
} // namespace mu::ui::window
