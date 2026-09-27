#pragma once

namespace Rml
{
    class ElementDocument;
}

namespace mu::ui::window
{
    class CObject;
}

// Keyboard routing for a CObject window that owns RmlUi text fields.
//
// CManager::UpdateKeyEvent() only dispatches to windows whose GetRelatedWnd() matches the focused
// handle, and it reports a focused RmlUi <input> as RmlUiRuntime's own address. So while one of a
// window's fields is focused, that window receives no keys at all -- not Enter, not Escape -- unless
// it claims that address as its related window. Claiming it only while its OWN field is focused keeps
// every other window's hotkeys suspended while the player types, exactly as before.
namespace UI::RmlBridge
{
    // True if the context's focused element is a text-entry field inside `doc`.
    bool IsTypingIn(Rml::ElementDocument* doc);

    // Call once per frame (from Update()): points `window` at RmlUiRuntime while the player is typing
    // in one of `doc`'s fields, and back at the game window otherwise.
    void ClaimKeyboardWhileTyping(mu::ui::window::CObject& window, Rml::ElementDocument* doc);
}
