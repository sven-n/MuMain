#pragma once

#include <string>

namespace Rml
{
    class Context;
    class ElementDocument;
}

// Which RmlUi visual theme is active -- see the RmlUi migration plan's Theming / Scaling & DPI
// sections. A theme is identified purely by NAME (a folder), not hardcoded in C++: just
// Data/Interface/RmlUi/themes/<name>/<document>.rcss, no manifest file needed. This is deliberate:
// it's what makes a theme (including one a modder drops in, not just "legacy"/"modern") a
// plug-in-a-folder operation with zero source changes or recompilation.
//
// Every migrated window renders its own visuals entirely through RmlUi -- CWin (and any other
// legacy widget) never draws background/frame art for a migrated window, in any theme. A
// "legacy-look" theme reproduces the original art by pointing its own RCSS decorators at the same
// image files the old sprites used (e.g. themes/legacy/login.rcss's `decorator: image(...)`) --
// that's a choice of asset, not a choice of renderer. There used to be a per-theme
// `UsesLegacySpriteChrome` manifest flag letting a theme opt back into CWin drawing the
// background -- removed (see docs/rmlui-ui-system/gotchas-and-patterns.md) because it worked
// against the migration's actual point: once a window is migrated, RmlUi owns 100% of its
// rendering, unconditionally.
//
// Read once from GameConfig at startup, not a live in-game hot-swap yet -- switching themes today
// means editing config.ini's [UI] RmlTheme value and relaunching. A true runtime toggle needs each
// migrated window to tear down and rebuild its Rml::ElementDocument/DataModel against the new
// theme's stylesheet path -- a real follow-up, not this increment's scope.
namespace UI::RmlBridge
{
    // Cached on first call from GameConfig::GetRmlTheme() (e.g. "legacy", "modern", or any
    // modder-supplied folder name -- not a closed set).
    const std::string& GetActiveThemeName();

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
}
