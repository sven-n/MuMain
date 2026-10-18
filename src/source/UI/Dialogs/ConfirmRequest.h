#pragma once

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// A confirm or notice dialog described by its content and answers, for code that must not depend
// on the dialog's implementation. Queued behind an open dialog, like every other.
namespace UI::Dialogs
{
struct ConfirmRequest
{
    struct Line
    {
        std::wstring text;
        bool bold = false;
        unsigned long color = 0; // RGBA(); 0 = the theme's line colour
    };
    std::vector<Line> lines;

    // The accept button's text; empty keeps the dialog's own ("OK").
    std::wstring acceptLabel;
    // Offers Cancel; Esc then cancels too.
    bool cancellable = false;
    // A duel invitation or result: the duel art, with this caption over it.
    std::optional<std::wstring> duelCaption;

    std::function<void()> onAccept; // accept button or Enter
    std::function<void()> onCancel; // Cancel or Esc

    // Names the dialog for code that answers it as the player would (the control socket).
    std::string tag;
};

void ShowConfirm(ConfirmRequest request);
// Answers the dialog tagged `tag` as its accept or cancel button does; false when there is none.
bool AnswerConfirm(std::string_view tag, bool accept);

bool IsMessageBoxOpen();
// A window that a duel request must not interrupt is open.
bool IsDuelRequestBlocked();
}
