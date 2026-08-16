#pragma once

#include "stdafx.h"
#include <RmlUi/Core/SystemInterface.h>

// Rml::SystemInterface implementation. Every base-class method already has a working default
// (SystemInterface.h -- none are pure virtual), so only the ones worth redirecting into this
// engine's existing utilities are overridden here; TranslateString/JoinPath/ActivateKeyboard/
// DeactivateKeyboard are left at their defaults (no-ops) -- see the RmlUi migration plan's Open
// Decisions for the separate (unscoped) question of bridging RmlUi's translation hook to this
// project's own .resx-based I18N system, which is a font/text-pipeline decision, not a system-
// interface one.
class RmlUiSystemInterface : public Rml::SystemInterface
{
public:
    double GetElapsedTime() override;
    bool LogMessage(Rml::Log::Type type, const Rml::String& message) override;
    void SetClipboardText(const Rml::String& text) override;
    void GetClipboardText(Rml::String& text) override;
};
