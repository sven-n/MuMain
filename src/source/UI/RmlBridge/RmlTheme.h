#pragma once

#include <string>

namespace Rml
{
    class Context;
    class ElementDocument;
}

// Which RmlUi visual theme is active. A theme is identified purely by NAME (a folder), not
// hardcoded in C++: just Data/Interface/RmlUi/themes/<name>/<document>.rcss, no manifest file
// needed. This is deliberate: it's what makes a theme (including one a modder drops in, not just
// "legacy"/"modern") a plug-in-a-folder operation with zero source changes or recompilation.
//
// Every migrated window renders its own visuals entirely through RmlUi -- CWin (and any other
// legacy widget) never draws background/frame art for a migrated window, in any theme. A
// "legacy-look" theme reproduces the original art by pointing its own RCSS decorators at the same
// image files the old sprites used (e.g. themes/legacy/login.rcss's `decorator: image(...)`) --
// that's a choice of asset, not a choice of renderer.
//
// Seeded once from GameConfig::GetRmlTheme() at startup, then live-mutable via SetActiveThemeName()
// (e.g. the `$theme <name>` chat command) -- GameConfig's own value only changes what a *relaunch*
// picks up; the cache here is the actual "what's on screen right now" source of truth. Changing it
// alone does nothing visually: a window's Rml::ElementDocument/DataModel was already built against
// whatever theme was active when LoadThemedDocument() last ran for it, so a caller that wants the
// change to be visible must also tear down and rebuild every currently-open themed window's
// document (see IObject::ReloadRmlTheme()) after calling SetActiveThemeName().
namespace UI::RmlBridge
{
    // Cached on first call from GameConfig::GetRmlTheme() (e.g. "legacy", "modern", or any
    // modder-supplied folder name -- not a closed set). Reflects the live cache SetActiveThemeName()
    // writes to, not necessarily GameConfig's current value.
    const std::string& GetActiveThemeName();

    // Overwrites the live active-theme cache GetActiveThemeName()/ThemeProvidesOwnIconChrome() read
    // from (lowercased, same normalization GetActiveThemeName() has always applied). Does not touch
    // GameConfig and does not rebuild any window's document by itself -- see this file's top comment.
    void SetActiveThemeName(const std::string& themeName);

    // True if themes/<themeName>/base.rcss exists and is readable -- the minimum a folder needs to
    // be a real theme (every window's document links it first, per theming-and-modding.md). Meant
    // as a pre-switch validation gate (e.g. for the `$theme` command) so an unknown/misspelled name
    // is rejected up front instead of silently rendering every window unstyled.
    bool ThemeExists(const std::string& themeName);

    // A declared theme capability (C++ must never branch on a theme's NAME -- a theme wanting
    // non-default behavior states that want itself, via an optional themes/<name>/theme.ini,
    // [Capabilities] section, key "ProvidesOwnIconChrome").
    // True means the active theme supplies its own RmlUi border/fill/highlight treatment for the
    // still-legacy-2D-rendered skill/potion icon chrome (MainFrameWindow.cpp), so C++ should
    // skip drawing the legacy sprite fill/highlight there. Missing file or missing key both
    // default to false (draw the real legacy sprite) -- the safe default for any theme that
    // hasn't declared an opinion, including a future modder-supplied one.
    bool ThemeProvidesOwnIconChrome();

    // Builds the virtual source URL a themed document should be loaded against, e.g.
    // "Data/Interface/RmlUi/themes/modern/login.rml" -- this path need not exist on disk (the RML
    // itself is loaded from memory, shared across every theme); it only needs to resolve relative
    // hrefs (<link type="text/rcss" href="login.rcss">) to the real per-theme .rcss file that
    // does exist, per Rml::Context::LoadDocumentFromMemory's source_url contract.
    std::string ThemedDocumentSourceUrl(const char* documentName, const std::string& themeName);

    // Reads `documentPath` (e.g. "Data/Interface/RmlUi/login.rml") from disk and instantiates it
    // against the currently active theme's stylesheet via LoadDocumentFromMemory. This is the one
    // entry point every migrated window should use instead of calling Context::LoadDocument
    // directly -- "add a new theme" stays a drop-a-folder-of-RCSS operation for every window
    // built on this helper, not a per-window special case. Returns nullptr if the file couldn't
    // be read or the document failed to parse (logged via g_ErrorReport either way).
    Rml::ElementDocument* LoadThemedDocument(Rml::Context* context, const char* documentPath);

    // LoadThemedDocument() against RmlUiRuntime::Instance().GetBackgroundContext(), then Show()s it
    // immediately -- a background-context document doesn't follow its owning window's own
    // Show()/Hide() lifecycle the way the main-context document does (it's driven entirely by
    // RmlUiRuntime::RenderBackgroundLayer() being called or not, see MyInventory.h's
    // MyInventoryBgRmlModel comment), so unlike a typical LoadThemedDocument() caller this one
    // shows eagerly at Create() time rather than waiting for the window to actually open. Returns
    // nullptr (silently) if the background context doesn't exist yet.
    Rml::ElementDocument* CreateBackgroundDocument(const char* documentPath);
}
