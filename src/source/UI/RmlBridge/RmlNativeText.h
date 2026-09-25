#pragma once

namespace Rml
{
    class Context;
    class ElementDocument;
}

// Native-sized text for the legacy theme. The original client draws almost all of its UI text in
// one font whose size grows about half as fast as the rest of the UI (UI::Scaling::FontPointSize),
// so `dp` text (which grows with the whole UI) comes out about 1.5x too large at 1024x768.
//
// A theme declares it (theme.ini [Capabilities] NativeTextSize=1) and a document opts in with
// `class="native-text"` on its <body> (inert in other themes): its root font-size is then set to
// the native text renderer's size (UI::Scaling::NativeTextPixelSize, LayoutMode::Dialog), and its
// RCSS sizes text in `rem` -- 1rem is the original's text size at every resolution and UI scale.
// For `dp` documents only: a document inside a transform: scale(root_scale) panel would scale the
// root size a second time; those counter-scale text leaves instead (RmlRootTransform.h).
namespace UI::RmlBridge
{
    // Applies the size to one document if it opted in, for its context's (the window's) size; call
    // when a document is loaded.
    void ApplyNativeTextSize(Rml::ElementDocument* document);

    // Re-applies it to every opted-in document of a context; call after the context was resized
    // or its UI scale changed.
    void ApplyNativeTextSize(Rml::Context* context);
}
