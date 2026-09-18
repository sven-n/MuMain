#pragma once

namespace Rml
{
    class Element;
    class ElementDocument;
}

// Helpers for documents built on the shared window_shell.rml/.rcss template.
namespace UI::RmlBridge
{
    // window_shell's #content is a single flex-column splice target (Template::ParseTemplate only
    // supports one content id) -- a consumer has no way to author an element as a structural
    // sibling of #content, the way generic_confirm_dialog.rcss's own .gcd-btn-primary is a direct
    // #panel child, sibling of .gcd-body, rather than nested inside its scrollable content well.
    //
    // This moves an already-parsed element (found by id, wherever the template spliced it) into
    // window_shell.rml's own #window_shell_footer anchor -- also a direct #panel child -- after
    // the document loads. Existing data bindings (data-event-click, {{}} interpolation, etc.) stay
    // live across the move: RmlUi resolves a data view against the DataModel it was constructed
    // against, not by re-walking the DOM.
    //
    // Returns the moved element, or nullptr if the document has no #window_shell_footer (not a
    // window_shell consumer) or no element with the given id.
    Rml::Element* PromoteToWindowShellFooter(Rml::ElementDocument* document, const char* elementId);
}
