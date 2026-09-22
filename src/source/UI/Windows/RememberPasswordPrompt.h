// In-game OK/Cancel confirmation shown before the login password is stored.
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

    // Opens the confirmation dialog and marks the choice Pending.
    void OpenRememberPasswordPrompt();

    // The current answer. The caller applies it and then clears it.
    RememberPasswordChoice RememberPasswordChoiceState();
    void ClearRememberPasswordChoice();

    // Polls Enter/Esc while the dialog is Pending, resolving it like its OK/Cancel buttons.
    // Uses polling rather than RmlUi Keydown routing since this dialog has no focused element.
    void Tick();

    // Rebuilds this dialog's RmlUi document/model for the active theme. This module has no `this`
    // of its own, so it registers itself with UI::RmlBridge's theme-reload registry (keyed by a
    // private static token) instead of a CObject/CManager registration. No-op if never opened.
    void ReloadRmlTheme();
}
