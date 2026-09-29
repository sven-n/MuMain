#pragma once

#include "UI/Dialogs/MessageBoxViewRmlModel.h"
#include "UI/RmlBridge/RmlModelBinder.h"

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
    };

    // A text list inside the box (CUIUnmixgemList), in the box's reference px: rows' tops are
    // their 13 px boxes, the scroll parts as CUITextListBox's old-style scroll bar lays them out.
    struct List
    {
        struct Row
        {
            std::wstring text;
            float top = 0.f;
            bool selected = false;
        };
        float left = 0.f;
        float top = 0.f;
        float width = 0.f;
        float height = 0.f;
        std::vector<Row> rows;
        bool upPressed = false;
        bool downPressed = false;
        float trackTop = 0.f;
        float trackHeight = 0.f;
        float thumbTop = 0.f;
        float thumbHeight = 0.f;
        float thumbBottomTop = 0.f; // the thumb's 1 px bottom cap
    };

    MessageBoxView() = default;
    ~MessageBoxView();
    MessageBoxView(const MessageBoxView&) = delete;
    MessageBoxView& operator=(const MessageBoxView&) = delete;

    void Create(int middleCount, float backHeight);
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
        return m_pRmlDoc != nullptr;
    }

    int TakePressedButton();

private:
    std::string m_ModelName;
    RmlModelBinder<MessageBoxViewRmlModel> m_RmlBinder;
    Rml::ElementDocument* m_pRmlDoc = nullptr;
    int m_PressedButton = -1;
};
} // namespace mu::ui::window
