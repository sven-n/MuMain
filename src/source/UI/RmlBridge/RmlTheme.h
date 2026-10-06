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

    // Overwrites the live active-theme cache GetActiveThemeName() reads
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

    // Declared theme capabilities: an optional themes/<name>/theme.ini [Capabilities] section,
    // missing keys false. C++ never branches on a theme's name.
    // Whether the active theme sizes opted-in documents' text like the original client's
    // (NativeTextSize=1) -- see RmlNativeText.h.
    bool ThemeUsesNativeTextSize();

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

    // LoadThemedDocument() for a document instantiated once per window (the friends family's
    // chat rooms and letters): every occurrence of `modelPlaceholder` in the markup (its
    // data-model name) becomes `modelName`, so each instance binds its own data model.
    Rml::ElementDocument* LoadThemedDocument(Rml::Context* context, const char* documentPath,
                                             const std::string& modelPlaceholder, const std::string& modelName);

    // Sets the original's layer depth of `documentName` (e.g. "loading.rml", RmlStackingOrder.h)
    // as the document's z-index. LoadThemedDocument() does this for every document it loads; call
    // it only for a document loaded some other way.
    void ApplyStackingDepth(Rml::ElementDocument* document, const std::string& documentName);

    // Scene gate (RmlStackingOrder.h, DocumentScene). The original drew CNewUIManager's windows
    // only while the main scene ran; the main-scene windows sync their documents from their own
    // Update(), which stops with the main scene, so a document open at logout kept rendering over
    // character selection. Outside the main scene every main-scene document is suspended
    // (display: none -- never drawn, never hit; its own Show()/Hide() state is kept, and a Show()
    // while suspended has no effect).
    //
    // Marks `documentName`'s document as a main-scene one when the stacking table says so;
    // LoadThemedDocument() does this for every document it loads.
    void ApplyDocumentScene(Rml::ElementDocument* document, const std::string& documentName);
    // Every frame before RmlUi updates and renders (RmlUiRuntime::Update()): outside the main
    // scene, suspends every marked document, including one loaded since (a theme switch).
    void SuspendMainSceneDocumentsOutsideMainScene();
    // At the start of CSystem::Update(), before the windows sync their documents: lifts the
    // suspension, so a window closed meanwhile hides its document in the same frame instead of
    // showing it for one.
    void ResumeMainSceneDocuments();

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
    // The owner's document, for where it sits among its context's documents; null while it has none.
    using ThemeReloadDocument = std::function<Rml::ElementDocument*()>;

    // Registers-or-replaces `owner`'s callback (same shape as
    // Core::Time::FrameTimerScheduler::SetRepeating() -- calling again for an already-registered
    // owner just replaces its callback, it does not duplicate). Safe, and expected, to call on
    // every Create()/BuildRmlUi() re-entry, including repeated calls across an object's lifetime
    // (e.g. a window whose Create() re-runs across scene transitions) -- always leaves exactly one
    // live callback per owner.
    //
    // **An owner that can be destroyed before shutdown must call UnregisterForThemeReload().** The
    // registry holds the callback, and a typical callback captures `this`, so a destroyed owner that
    // never unregistered leaves a dangling call waiting for the next theme switch. An app-lifetime
    // owner -- a file-scope global, a function-local static -- need not bother, which is why roughly
    // a fifth of the current callers don't: they cannot outlive the registry. Anything heap-owned
    // should unregister in its Release()/destructor, as the windows on CManager already do.
    //
    // A reloaded document goes on top of its context, so the owners that name their document reload
    // from the bottom of its context's stack up, and the stacking survives a theme switch.
    void RegisterForThemeReload(const void* owner, ThemeReloadCallback callback, ThemeReloadDocument document = {});

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
