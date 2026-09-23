#pragma once

#include <functional>
#include <string>
#include <vector>

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
// document -- see RegisterForThemeReload()/ReloadAllThemedDocuments() below -- after calling
// SetActiveThemeName().
namespace UI::RmlBridge
{
    // Cached on first call from GameConfig::GetRmlTheme() (e.g. "legacy", "modern", or any
    // modder-supplied folder name -- not a closed set). Reflects the live cache SetActiveThemeName()
    // writes to, not necessarily GameConfig's current value.
    const std::string& GetActiveThemeName();

    // Overwrites the live active-theme cache GetActiveThemeName()/ThemeProvidesOwnIconChrome() read
    // from (lowercased, same normalization GetActiveThemeName() has always applied). Does not touch
    // GameConfig and does not rebuild any window's document by itself -- see this file's top comment.
    // Does clear RmlUi's own template cache (Rml::Factory::ClearTemplateCache()), since two themes'
    // same-named <template> forks (e.g. "window_shell_bg") would otherwise leave that name resolving
    // to whichever theme's copy was most recently freshly loaded, not necessarily the one now active.
    void SetActiveThemeName(const std::string& themeName);

    // True if themes/<themeName>/base.rcss exists and is readable -- the minimum a folder needs to
    // be a real theme (every window's document links it first). Meant as a pre-switch validation
    // gate (e.g. for the `$theme` command) so an unknown/misspelled name is rejected up front
    // instead of silently rendering every window unstyled.
    bool ThemeExists(const std::string& themeName);

    // Every valid theme (ThemeExists()) found directly under Data/Interface/RmlUi/themes/, sorted
    // alphabetically by folder name for deterministic, stable ordering across calls (a
    // std::filesystem::directory_iterator's own enumeration order is unspecified). A folder with
    // no base.rcss of its own is silently skipped -- not a real theme. This is what lets a newly
    // installed theme folder (including a modder's own) become selectable in COptionWindow's UI
    // Theme dropdown with zero C++ changes; see this file's own top comment.
    std::vector<std::string> DiscoverAvailableThemes();

    // themes/<themeName>/theme.ini's optional [Meta] DisplayName, e.g. "Legacy"/"Modern" for the
    // two bundled themes. Falls back to `themeName` with its first letter capitalized if the file
    // or key is missing -- the safe default for a theme that hasn't declared one, including a
    // modder-supplied theme with no theme.ini at all.
    std::string GetThemeDisplayName(const std::string& themeName);

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

    // LoadThemedDocument() against RmlUiRuntime::Instance().GetBackgroundContext(). Starts hidden,
    // same as LoadThemedDocument() itself -- the caller's own SyncRmlModel() shows/hides it against
    // IsVisible(), same as its root_x/root_y/root_scale sync (MyInventory.h's MyInventoryBgRmlModel
    // comment). Used to Show() eagerly here instead, since these documents are driven by
    // RmlUiRuntime::RenderBackgroundLayer() rather than their owner's own Show()/Hide() -- but every
    // one of these is created once at boot (LoadMainSceneInterface()), before CSystem::Update() ever
    // runs its first correcting SyncRmlModel() (gated to SceneFlag == MAIN_SCENE), so the eager
    // Show() left it visible at its model's zero-initialized default (unscaled, top-left) for the
    // first few MAIN_SCENE frames of a client's very first login. Returns nullptr (silently) if the
    // background context doesn't exist yet.
    Rml::ElementDocument* CreateBackgroundDocument(const char* documentPath);

    // Same as CreateBackgroundDocument(const char*) but against an explicit context instead of
    // always GetBackgroundContext() -- e.g. RmlUiRuntime::Instance().GetDialogBackgroundContext()
    // for CGenericConfirmDialog's own panel, which needs to render at a different point in the
    // frame than every ordinary window's own bg doc (see RenderDialogBackgroundLayer()'s own
    // comment for why). Returns nullptr (silently) if `context` is null.
    Rml::ElementDocument* CreateBackgroundDocument(const char* documentPath, Rml::Context* context);

    // Tier-agnostic theme-reload registry. Any owner of a themed document -- a window (keyed by
    // `this`) or a free-function module with no `this` (keyed by the address of a private static
    // token) -- registers one callback here, right next to the code that already creates its first
    // document, instead of overriding a virtual and hoping every sweep call site reaches it.
    using ThemeReloadCallback = std::function<void()>;

    // Registers-or-replaces `owner`'s callback (same shape as
    // Core::Time::FrameTimerScheduler::SetRepeating() -- calling again for an already-registered
    // owner just replaces its callback, it does not duplicate). Safe, and expected, to call on
    // every Create()/BuildRmlUi() re-entry, including repeated calls across an object's lifetime
    // (e.g. a window whose Create() re-runs across scene transitions) -- always leaves exactly one
    // live callback per owner.
    void RegisterForThemeReload(const void* owner, ThemeReloadCallback callback);

    // Removes `owner`'s callback, if any -- a no-op if `owner` isn't registered (same shape as
    // FrameTimerScheduler::Kill()), so it's safe to call from a Release()/destructor path that may
    // run more than once. Call this ONLY where the owner already unhooks from its CManager
    // (RemoveUIObj(this)) -- an owner that never does that (a handful of app/scene-lifetime
    // singleton windows) must never call this either, so its registration outlives every Release()
    // the same way its CManager registration already does.
    void UnregisterForThemeReload(const void* owner);

    // Calls every registered callback, rebuilding every currently-open themed document/model
    // against whatever SetActiveThemeName() most recently set. The one call a theme switch needs
    // after SetActiveThemeName(). Copies the registry before iterating -- cheap at this scale and
    // removes any reliance on no callback ever registering/unregistering another owner while this
    // sweep is in progress.
    void ReloadAllThemedDocuments();
}
