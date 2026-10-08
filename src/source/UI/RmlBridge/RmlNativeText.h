#pragma once

namespace Rml
{
class Context;
class ElementDocument;
} // namespace Rml

// Native-sized text for the legacy theme. The original client draws almost all of its UI text in
// one font whose size grows about half as fast as the rest of the UI (UI::Scaling::FontPointSize),
// so `dp` text (which grows with the whole UI) comes out about 1.5x too large at 1024x768.
//
// A theme declares it (theme.ini [Capabilities] NativeTextSize=1) and a document opts in with
// `class="native-text"` on its <body> (inert in other themes): its root font-size is then set to
// the native text renderer's size (UI::Scaling::NativeTextPixelSize, LayoutMode::Stage), and its
// RCSS sizes text in `rem` -- 1rem is the original's text size at every resolution and UI scale.
// For `dp` documents only: a document inside a transform: scale(root_scale) panel would scale the
// root size a second time; those counter-scale text leaves instead (RmlRootTransform.h).
//
// Scene windows the original drew at fixed pixels (login form and buttons, server list, system
// menu) opt in with `class="scene-window-scale"` on their <body> instead: the root font-size is then
// UI::Scaling::SceneWindowScale() px, so their RCSS writes the original's pixel sizes in `rem`
// (1rem = one original pixel, growing like the native text). `class="scene-bar-scale"` does the
// same with UI::Scaling::SceneBarScale(), the original's own scale of the character scene button
// bar. In both, an element with class "native-text" gets the native text size as its font-size,
// for the text it contains.
namespace UI::RmlBridge
{
// What the companion C++ of a "scene-window-scale" window (hit-test rects, pushed positions) multiplies
// the original's pixel sizes by: SceneWindowScale() where the theme declares NativeTextSize,
// else the `dp` ratio (UI::Scaling::CompanionRatio) the other themes size these windows in.
float SceneWindowRatio(int windowWidth, int windowHeight);
// The same for a window the other themes size in `px` (server list, login scene buttons' bar): 1 there.
float SceneWindowPixelRatio(int windowWidth, int windowHeight);

// Applies the size to one document if it opted in, for its context's (the window's) size; call
// when a document is loaded.
void ApplyNativeTextSize(Rml::ElementDocument* document);

// Re-applies it to every opted-in document of a context; call after the context was resized
// or its UI scale changed.
void ApplyNativeTextSize(Rml::Context* context);
} // namespace UI::RmlBridge
