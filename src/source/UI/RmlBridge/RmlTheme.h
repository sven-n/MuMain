#pragma once

#include <string>

namespace Rml
{
    class Context;
    class ElementDocument;
}

// Which RmlUi visual theme is active -- see the RmlUi migration plan's Theming / Scaling & DPI
// sections. A theme is identified purely by NAME (a folder), and everything about it -- including
// whether it wants legacy sprite chrome -- is read from that folder, not hardcoded in C++. This
// is deliberate: it's what makes a theme (including one a modder drops in, not just "legacy"/
// "modern") a plug-in-a-folder operation with zero source changes or recompilation, for either
// kind of theme:
//
//   Data/Interface/RmlUi/themes/<name>/<document>.rcss   -- required, the actual visual styling
//   Data/Interface/RmlUi/themes/<name>/theme.ini          -- optional, see UsesLegacySpriteChrome
//
// A theme with no theme.ini (or no UsesLegacySpriteChrome key in it) defaults to fully
// programmatic/sprite-free -- the common case for a new reskin, seasonal theme, high-contrast/
// accessibility theme, or a mod: just drop RCSS files in a new folder and point config.ini's
// [UI] RmlTheme at it, no C++ involved at all.
//
// Read once from GameConfig at startup, not a live in-game hot-swap yet -- switching themes today
// means editing config.ini's [UI] RmlTheme value and relaunching. A true runtime toggle needs each
// migrated window to tear down and rebuild its Rml::ElementDocument/DataModel against the new
// theme's stylesheet path (and, for sprite-backed themes, its CWin background/frame sprites) --
// a real follow-up, not this increment's scope.
namespace UI::RmlBridge
{
    // Cached on first call from GameConfig::GetRmlTheme() (e.g. "legacy", "modern", or any
    // modder-supplied folder name -- not a closed set).
    const std::string& GetActiveThemeName();

    // True only for themes that should keep drawing the legacy background/input-frame sprites
    // (CWin::m_psprBg, the CUITextInputBox frame CSprites) underneath the RmlUi overlay, the way
    // the original Phase 1 pilot did -- read from that theme's own
    // Data/Interface/RmlUi/themes/<name>/theme.ini ([Theme] UsesLegacySpriteChrome=1), defaulting
    // to false (fully programmatic) if the file or key is absent. Data-driven per theme folder,
    // not a hardcoded C++ allowlist -- a new theme (modder-supplied or not) that wants legacy
    // sprite chrome just ships that one line in its own theme.ini, no engine changes needed.
    bool UsesLegacySpriteChrome(const std::string& themeName);

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
