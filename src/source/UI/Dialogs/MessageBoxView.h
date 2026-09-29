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
// newui_btn_empty_small buttons (CGuild_ToPerson_Position). One box at a time. The box owns it while it exists and
// keeps its callbacks: a button RmlUi reports is taken with TakePressedButton() and sent as the
// box's own event.
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
    };

    struct Button
    {
        std::wstring label;
        float left = 0.f;
        float top = 0.f;
        float width = 0.f;
        float height = 0.f;
        bool enabled = true;
    };

    MessageBoxView() = default;
    ~MessageBoxView();
    MessageBoxView(const MessageBoxView&) = delete;
    MessageBoxView& operator=(const MessageBoxView&) = delete;

    void Create(int middleCount, float backHeight);
    void Destroy();

    // Per frame, inside the message box manager's transform scope.
    void Sync(const POINT& pos, const std::vector<Line>& lines, const std::vector<Button>& buttons);

    int TakePressedButton();

private:
    std::string m_ModelName;
    RmlModelBinder<MessageBoxViewRmlModel> m_RmlBinder;
    Rml::ElementDocument* m_pRmlDoc = nullptr;
    int m_PressedButton = -1;
};
} // namespace mu::ui::window
