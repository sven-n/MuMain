#pragma once

#include <RmlUi/Core/ElementDocument.h>

// Visibility sync for RmlUi windows whose SyncRmlModel()/SyncDocVisibility() runs every frame.
//
// ElementDocument::Show() defaults to FocusFlag::Auto, which focuses the document (or its
// autofocus element, if any) on every call -- so re-asserting it each frame blurs whatever the
// user just focused, one frame later. That makes a focused <input> impossible to type into: the
// character is dispatched to the document instead of the field, and WidgetTextInput never sees
// it. Show() also runs a full UpdateDocument() internally, so the repeat call is wasted work
// even for a document with nothing focusable in it.
//
// Transition only, never re-assert. A document hidden out-of-band (ReloadRmlTheme(), or a fresh
// document after a theme swap) still self-corrects on the next frame, since the transition is
// detected against the document's own live visibility rather than a cached flag.
//
// Event-driven show/hide (CObject::Show(bool) overrides, dialogs opening) should keep calling
// Show()/Hide() directly -- those fire once per transition already, and a modal dialog's
// FocusFlag::Document is deliberate.
namespace UI::RmlBridge
{
    inline void SyncDocumentVisibility(Rml::ElementDocument* doc, bool visible)
    {
        if (doc == nullptr || doc->IsVisible() == visible)
            return;

        if (visible)
            doc->Show();
        else
            doc->Hide();
    }
}
