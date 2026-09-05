// In-game OK/Cancel confirmation shown before the login password is stored.
// Kept separate from the login window so the message-box layout class lives in
// its own translation unit.
#pragma once

namespace UI::Login
{
    // The player's answer to the "Remember Password" confirmation dialog.
    enum class RememberPasswordChoice
    {
        None,     // nothing to apply
        Pending,  // dialog is open, awaiting an answer
        Ok,       // player confirmed
        Cancel,   // player declined
    };

    // Opens the confirmation dialog (which warns about storing the password on a
    // shared machine) and marks the choice Pending.
    void OpenRememberPasswordPrompt();

    // The current answer. The caller applies it and then clears it.
    RememberPasswordChoice RememberPasswordChoiceState();
    void ClearRememberPasswordChoice();

    // Polls Enter/Esc while the dialog is Pending, resolving it the same way its OK/Cancel
    // buttons do. Call once per frame from a caller that's still ticking while the dialog owns
    // input (CLoginWin::UpdateWhileShow() does this today) -- RmlUi's own Keydown routing to an
    // unfocused document (this dialog has no naturally-focused element) is unverified in this
    // engine's integration, so this reuses the polling idiom CLoginWin's own OK/Cancel already
    // uses rather than introducing that as a new, unverified event path.
    void Tick();
}
