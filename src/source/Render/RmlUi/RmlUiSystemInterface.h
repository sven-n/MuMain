#pragma once

#include "stdafx.h"
#include <RmlUi/Core/SystemInterface.h>

struct SDL_Window;

// Rml::SystemInterface implementation. Every base-class method already has a working default
// (SystemInterface.h -- none are pure virtual), so only the ones worth redirecting into this
// engine's existing utilities are overridden here; TranslateString/JoinPath are left at their
// defaults. Bridging RmlUi's translation hook to this project's own .resx-based I18N system is a
// font/text-pipeline decision, not a system-interface one, and remains unscoped.
//
// ActivateKeyboard/DeactivateKeyboard ARE overridden (unlike the two above) -- they're the exact
// hook WidgetTextInput::SetKeyboardActive() (Source/Core/Elements/WidgetTextInput.cpp) already
// calls on every RmlUi <input>'s Focus/Blur, matching the vendored sample SystemInterface_SDL's
// own reference implementation (ThirdParty/RmlUi/Backends/RmlUi_Platform_SDL.cpp) rather than
// inventing a second lifecycle. This is also the sole ownership signal RmlUiRuntime::
// IsTextInputActive() reports: deterministic (driven by RmlUi's own Focus/Blur, not polled),
// exactly false whenever no RmlUi <input> is focused, which is every screen today except the
// as-yet-unmigrated CUITextInputBox consumers -- see RmlUiRuntime.h's own IsTextInputActive()
// comment for how the native fallback stays authoritative until then.
class RmlUiSystemInterface : public Rml::SystemInterface
{
public:
    explicit RmlUiSystemInterface(SDL_Window* window);

    double GetElapsedTime() override;
    bool LogMessage(Rml::Log::Type type, const Rml::String& message) override;
    void SetClipboardText(const Rml::String& text) override;
    void GetClipboardText(Rml::String& text) override;

    void ActivateKeyboard(Rml::Vector2f caret_position, float line_height) override;
    void DeactivateKeyboard() override;

    // True from the moment an RmlUi <input> is focused (ActivateKeyboard) until it's blurred
    // (DeactivateKeyboard) -- see this class's own comment above.
    bool IsTextInputActive() const { return m_TextInputActive; }

private:
    SDL_Window* m_Window = nullptr;
    bool m_TextInputActive = false;
};
