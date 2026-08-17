# RmlUi UI System Documentation

Welcome to the documentation for the MU Online Client's **RmlUi migration** — the ongoing
replacement of the client's three legacy UI frameworks with [RmlUi](https://github.com/mikke89/RmlUi)
(HTML/CSS-driven UI middleware), migrated window-by-window while the old systems keep running for
everything not yet migrated.

This documentation covers the render/system-interface implementation, the frame lifecycle, the
theming/modding architecture, and a catalog of real bugs found (and fixed) during the pilot
migration — written so a future migration phase doesn't have to rediscover the same gotchas.

---

## Why this exists

The client's game UI was implemented across three separate, simultaneously-alive widget
frameworks that accumulated over the project's history — `UI/Widgets` (`CWin`/`CButton`),
`UI/Legacy/UIControls` (`CUIControl`/`CUIMessage`), and `UI/NewUI` (`CNewUIObj`/`CNewUIManager`,
~90 windows). None of the three has a layout engine, a retained scene graph, or a data-binding
layer. RmlUi is being adopted as the long-term replacement, migrated incrementally with old and
new systems coexisting — not a big-bang rewrite.

The login/character-select dialog (`CLoginWin`) is the **Phase 1 pilot**: the first real
migrated window, and the one most of this documentation set is written against. It exercises the
full pipeline — render interface, input arbitration, data binding, and (added later) the theming
system — end to end. A second pass ("Batch 2") extended the rest of the login scene the same
way — `CLoginMainWin`, `CSysMenuWin`, `COptionWin`, and `RememberPasswordPrompt` — validating the
pattern against a window with no dynamic state at all, a full-screen dim-modal backdrop, a
draggable control, and a window that was never `CWin`-based to begin with.

## Document Index

1. **[Architecture](architecture.md)**
   *How RmlUi is wired into this engine's renderer and frame loop.* Covers the render/system
   interface implementation, vertex format conversion, the frame render-order contract (and why
   getting it wrong is easy to miss), input arbitration, and the coexistence model with the two
   legacy UI tiers.

2. **[Theming & Modding](theming-and-modding.md)**
   *How the swappable-theme system works, and how to add a new theme without touching engine
   code.* Covers the theme-folder convention, `LoadThemedDocument()`'s RML/RCSS resolution
   mechanism, and a step-by-step guide for modders.

3. **[Gotchas & Bug Catalog](gotchas-and-patterns.md)**
   *Real bugs found during the pilot migration, with root cause and fix.* Read this before
   migrating the next window — several of these are non-obvious traps that cost real debugging
   time once and don't need to cost it again.

4. **[Roadmap](roadmap.md)**
   *What's done, what's next, and the decisions still open.* Covers Phases 2-5 (message-box
   engine, HUD, 3D-in-UI bridge, inventory), retirement criteria, and — critically — that the
   adapter/facade pattern every later phase depends on has never actually been proven in real
   code. Read this before starting any window beyond the login scene.

---

## Architectural Highlights

- **Coexistence at the codebase level, not a per-window overlay.** A migrated window's `CWin`
  never draws its own background/frame chrome once migrated, in any theme — RmlUi owns 100% of a
  migrated window's rendering, always *instead of* the legacy sprite chrome, never layered on top
  of it (an earlier per-theme opt-back-in flag briefly allowed the "on top of" case; removed, see
  [Gotchas](gotchas-and-patterns.md#a-per-theme-flag-to-opt-back-into-cwin-sprite-rendering-was-the-wrong-shape)).
  What coexists is *old and new systems side by side across the codebase*, migrated window by
  window, not old-and-new rendering layered within one window. Two structural shapes exist for a
  migrated window, not one: a **hybrid** `CWin` + RmlUi overlay that keeps the legacy window's
  position/hit-testing bookkeeping (`CLoginWin`, `CLoginMainWin`, `CSysMenuWin`, `COptionWin` — see
  [Architecture §6](architecture.md#6-coexistence-bridging-cloginwins-specific-pattern)), and a
  **pure RmlUi** window with no `CWin` involvement at all
  (`RememberPasswordPrompt` — see [Architecture §6a](architecture.md#6a-a-pure-rmlui-window-with-no-cwin-at-all)).
  A model/binder layer (`UI::RmlBridge::RmlModelBinder`) is the common case for a window with
  dynamic state, not a hard requirement — `CLoginMainWin` has none and needs no binder at all (see
  [Architecture §5](architecture.md#5-data-binding-modelbinder-pattern)). External legacy call
  sites keep working unchanged either way (e.g. `CUIMng::m_SysMenuWin`, or
  `RememberPasswordPrompt.h`'s free-function API), even where the internals they call into were
  fully rewritten.
- **Custom `RHI::`-backed render interface**, not RmlUi's bundled OpenGL/D3D backends — RmlUi's
  `RenderGeometry` draws through this engine's own `RHI::DrawIndexed` / `PassthroughShader`,
  sharing the same `GlobalUBO` every other 2D/3D draw call uses, rather than maintaining a second,
  parallel rendering pipeline.
- **Only the "required" `Rml::RenderInterface` functions are implemented** — layers, filters, and
  transforms are left at their base-class no-op defaults. This is a deliberate MVP scope cut, not
  an oversight, but it has real consequences (see [Gotchas](gotchas-and-patterns.md) — `box-shadow`
  blur is the concrete example that bit the modern theme).
- **Pluggable, data-driven theming.** A theme is a folder name, not a closed enum or a C++
  allowlist — see [Theming & Modding](theming-and-modding.md).
- **Event-driven input arbitration** at the SDL-event level (`Winmain.cpp`), using RmlUi's own
  official SDL backend for keycode/modifier mapping rather than a hand-rolled bridge.
- **Reusable interaction helpers, not per-window reimplementation.** Draggability
  (`UI::RmlBridge::MakeDraggable`) is one function call built on RmlUi's own native drag events —
  the standing design principle is that any interaction pattern more than one migrated window
  will want (dragging, and whatever comes after it) belongs in `UI::RmlBridge` as a shared
  primitive, not copy-pasted or reinvented per window. See
  [Architecture §7](architecture.md#7-reusable-interaction-helpers-uirmlbridge).

## Primary Subsystems & Source Map

| Subsystem | Key Files | Description |
|---|---|---|
| **Runtime lifecycle** | [`Render/RmlUi/RmlUiRuntime.h/.cpp`](../../src/source/Render/RmlUi/RmlUiRuntime.h) | Owns the `Rml::Context`; the single per-frame `Update()`/`Render()` entry point and SDL event bridge. |
| **Render interface** | [`Render/RmlUi/RmlUiRenderInterface.h/.cpp`](../../src/source/Render/RmlUi/RmlUiRenderInterface.h) | `Rml::RenderInterface` implementation targeting `RHI::` directly — vertex conversion, texture loading, scissor. |
| **System interface** | [`Render/RmlUi/RmlUiSystemInterface.h/.cpp`](../../src/source/Render/RmlUi/RmlUiSystemInterface.h) | `Rml::SystemInterface` implementation — clock, logging, clipboard. |
| **Model/binder layer** | [`UI/RmlBridge/RmlModelBinder.h`](../../src/source/UI/RmlBridge/RmlModelBinder.h) | Generic per-window `Rml::DataModel` wrapper — owns the model instance and `DataModelHandle`, exposes `MarkDirty()`. |
| **Theme framework** | [`UI/RmlBridge/RmlTheme.h/.cpp`](../../src/source/UI/RmlBridge/RmlTheme.h) | Active-theme resolution, `LoadThemedDocument()`. |
| **Draggable panels** | [`UI/RmlBridge/RmlDraggable.h/.cpp`](../../src/source/UI/RmlBridge/RmlDraggable.h) | `MakeDraggable(handle, panel, onMove)` — generic drag-to-move on RmlUi's native drag events. |
| **Pilot window** | [`UI/Windows/LoginWin.h/.cpp`](../../src/source/UI/Windows/LoginWin.h) | The Phase 1 pilot — hybrid `CWin` + RmlUi overlay, the reference implementation most patterns in this doc set are drawn from. |
| **Batch 2 windows** | [`UI/Windows/LoginMainWin`](../../src/source/UI/Windows/LoginMainWin.h), [`SysMenuWin`](../../src/source/UI/Windows/SysMenuWin.h), [`OptionWin`](../../src/source/UI/Windows/OptionWin.h), [`RememberPasswordPrompt`](../../src/source/UI/Windows/RememberPasswordPrompt.h) | Same login-scene extended in one pass. `RememberPasswordPrompt` is the one **pure-RmlUi** window (no `CWin` at all) — see [Architecture §6a](architecture.md#6a-a-pure-rmlui-window-with-no-cwin-at-all). `OptionWin` is `UI::RmlBridge::MakeDraggable`'s first production use. |
| **Frame hook** | [`Scenes/SceneManager.cpp`](../../src/source/Scenes/SceneManager.cpp), [`Scenes/LoadingScene.cpp`](../../src/source/Scenes/LoadingScene.cpp) | Where `RmlUiRuntime::Update()/Render()` (and the post-render cursor/text draws) are called per scene. |
| **RML/RCSS assets** | [`bin/Data/Interface/RmlUi/`](../../src/bin/Data/Interface/RmlUi/) | One `.rml` per window (shared, theme-agnostic) + `themes/<name>/` (per-theme RCSS, no manifest file needed) + `themes/<name>/base.rcss` (shared cross-window rules — `.btn`, `.checkbox-*`, `#backdrop` — linked by every window's `.rml` except `login.rml`, which predates it and stays self-contained; see [Theming & Modding](theming-and-modding.md#shared-cross-window-rcss-basercss)). |

---

## Configuration

### `config.ini` — `[UI]` section

| Key | Default | Meaning |
|---|---|---|
| `RmlTheme` | `legacy` | Active RmlUi theme name — any folder under `Data/Interface/RmlUi/themes/`. Read once at startup by `GameConfig::GetRmlTheme()`; **not yet a live in-game hot-swap** — change it and relaunch. See [Theming & Modding](theming-and-modding.md). |

Every theme — including `legacy` — renders 100% of its window's chrome through RmlUi; `CWin` never
draws a background or frame sprite for a migrated window, in any theme. `legacy` reproduces the
original look by pointing its RCSS decorators at the same art files the old sprites used; `modern`
uses flat colors/vector shapes instead. See [Theming & Modding](theming-and-modding.md) for the
full mechanism and a step-by-step guide to adding a new theme.

## Vendored Dependency

RmlUi is vendored as a git submodule at `src/ThirdParty/RmlUi`, pinned to release tag `6.2` (not
tracking a moving branch). Built via `src/CMakeLists.txt` following the existing
SDL/SDL_mixer `mu_ensure_submodule` pattern — static link, no bundled samples, no Lua bindings.
FreeType is vendored alongside it (`src/ThirdParty/freetype`) to satisfy RmlUi's default
`RMLUI_FONT_ENGINE=freetype` requirement.
