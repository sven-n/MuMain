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
    struct Line
    {
        std::wstring text;
        float left = 0.f;
        float top = 0.f;
        bool bold = false;
        DWORD color = 0;
        float textPx = 0.f; // physical px; 0: the font's native size
    };

    struct Button
    {
        std::wstring label;
        float left = 0.f;
        float top = 0.f;
        float width = 0.f;
        float height = 0.f;
        bool enabled = true;
        bool okArt = false; // newui_button_ok's lettered art instead of a labelled newui_btn_empty_small
        // Where it sits and how big it is are the theme's, by the box's kind (Create()); left, top,
        // width and height are then unused.
        bool placed = false;
    };

    using List = std::vector<MessageBoxViewListRowEntry>;

    MessageBoxView();
    ~MessageBoxView();
    MessageBoxView(const MessageBoxView&) = delete;
    MessageBoxView& operator=(const MessageBoxView&) = delete;

    // `kind` names the box for a theme that places its buttons (Button::placed).
    void Create(int middleCount, float backHeight, const char* kind = "");
    // A box whose size changes after Create() (CProgressMsgBox grows with its text). With
    // middlesAboveDivider >= 0 the 21-unit newui_Message_Line follows that many middle strips
    // (CDevilSquareRankMsgBox).
    void SetFrame(int middleCount, float backHeight, int middlesAboveDivider = -1);
    // newui_separate_line rules at these tops (CDevilSquareRankMsgBox's table).
    void SetSeparators(const std::vector<float>& tops);
    // CProgressMsgBox's bar at `top`, `fraction` (0 .. 1) filled; a negative top hides it.
    void SetProgress(float top, float fraction);
    void Destroy();

    // Per frame, inside the message box manager's transform scope.
    void Sync(const POINT& pos, const std::vector<Line>& lines, const std::vector<Button>& buttons);
    // The box's list, or none (nullptr); call with Sync().
    void SyncList(const List* list);

    // The document is loaded: the box draws nothing natively.
    bool IsShown() const
    {
        return m_View.Document() != nullptr;
    }

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
