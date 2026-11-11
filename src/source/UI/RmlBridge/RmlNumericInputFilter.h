#pragma once

#include <RmlUi/Core/Types.h>

namespace Rml
{
    class ElementDocument;
}

// Digits-only entry for a stock RmlUi <input> -- the one text-field rule RmlUi has no attribute for.
//
// A field opts in by carrying the marker class below; nothing else changes about it, and it keeps
// .text-field's styling. The filter rejects the keystroke rather than correcting the value
// afterwards: it listens for `textinput` in the capture phase on the document, so it runs before the
// focused WidgetTextInput's own listener, and StopPropagation() there leaves the caret untouched.
// Writing a filtered value back onto the element instead re-enters OnValueAttributeChanged() and
// resets the caret to index 0.
//
// A clipboard paste reaches WidgetTextInput without a `textinput` event, so this cannot stop one --
// read numeric values through KeepDigitsOnly() as well.
namespace UI::RmlBridge
{
    // The marker class. Toggle it on the element to switch a single field between modes.
    inline constexpr const char* NumericFieldClass = "text-field--numeric";

    // Installs the filter on `doc`. Stateless and shared, so any number of documents may call this,
    // and there is nothing to remove before the document is unloaded.
    void AttachNumericInputFilter(Rml::ElementDocument* doc);

    Rml::String KeepDigitsOnly(const Rml::String& value);
}
