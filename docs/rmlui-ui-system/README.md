# RmlUi UI System

> **Start with [`architecture-principles.md`](architecture-principles.md)** — the governing
> policy (layout intent, responsive/scalable/themeable/moddable design). Every other doc here
> implements it; none of them repeat its reasoning.

How [RmlUi](https://github.com/mikke89/RmlUi) (HTML/CSS-driven UI middleware) is integrated into
this client's SDL_GPU renderer, and what's worth knowing before building or porting a window.

## Why this exists

The client's game UI had no layout engine, retained scene graph or data-binding layer. RmlUi
replaced it window by window, old and new coexisting until the last native window went.

## What is RmlUi, and what stays native

Every window is a `mu::ui::window::CObject` drawn by RmlUi in both themes; the `CWin` toolkit,
the sprite widgets, the `CUIControl` toolkit and the shared item camera (`C3DRenderMng`) are
deleted. That covers the login and character-select scene, the HUD (one theme-placed unit in the
workspace), the inventory family, the docked panels, the dialogs and options, the social windows,
the event, siege, duel and NPC windows, and the world labels (names, balloons, bars, ground items)
through the world-label layer. [`migration-ledger.md`](migration-ledger.md) maps each original
class to what replaced it.

**Placement is the theme's.** No window has a layout of its own: documents sit on `.stage`,
`.hud-board` or a workspace slot, RmlUi hit-tests them, and `CManager` gives native code one
measuring space ([`layout-and-scaling.md`](layout-and-scaling.md)'s "Units and placement"). Only
infrastructure may touch the active transform; `tools/check_layout_transform_users.py` keeps it to
that.

**Stays native on purpose**: live 3D content (item grids, equipped items, item and character
previews — a `RenderTarget` shows one inside a document, as the potions, the letter portrait, the
character-creation preview and the event previews do), the mouse cursor, developer overlays, and
the equipment paperdoll's background, durability tint and drag highlight, which paint behind the
equipped item's 3D icon.

## Known limits

Each stays as it is until its trigger fires.

- **No user-override layer or theme inheritance** (principles §18–19). Themes are two directories
  selected by name. A third first-party theme is ruled out; `modern`'s divergence and the contract
  guard cover the coupling concern (§25, §28). Trigger: the mod-support requirement is prioritised.
- **Validation is uneven across windows.** Headless layout tests cover resolution and OS display
  scale for the party list, trade, event entry, the personal shops and the MU Helper; other
  windows have the UI-scale sweep ([`layout-and-scaling.md`](layout-and-scaling.md)). Drag state
  across a scale change and theme changes while open are checked when a window is touched.
- **`MiniMap` lays out in physical pixels**: its art turns 45° with no reference-px space
  (`UI/HUD/MiniMapLayout.cpp`). Trigger: a map redesign needing theme-owned map geometry.
- **A new siege command's pulse** binds `rgb(255, pulse, pulse)`: RCSS cannot mix a bound fraction
  into a colour. Trigger: a theme wanting another pulse palette.
- **`CCryWolf` picks its sprite files and texel rects in C++** (`SyncResult()`, `SyncHud()`); a theme
  can hide or rearrange that art, not replace it. Trigger: a theme wanting other event art.
- **The title scene's loading bar** is pushed in real `px`, because its background is still native
  sprites on an 800x600 per-axis scale `dp` cannot reproduce. It ends when those sprites port.

## Where code goes

- **`Render/RmlUi`**: what RmlUi needs from any host engine. The runtime (lifetime, contexts, the
  frame hook, SDL input, IME), the render and system interfaces, custom decorators. No themes and
  no game state.
- **`UI/RmlBridge`**: what game windows share. Themed loading and theme switching, the model
  helpers, visibility and stacking, the shared tooltip, dragging, text-size policy, and the
  adapters between legacy reference coordinates and RmlUi (root transform, panel readback, the
  keyboard claim). No widget classes and no event or layout system of its own: use RmlUi's.
- **A window**: its model fields, its event callbacks and its game behaviour. Layout, units,
  colours and placement stay in RML/RCSS.

The runtime renders, updates and routes input; the game adds the rest through
`RmlUiRuntimeHooks`. How native 3D shares the frame with RmlUi is in
[Frame lifecycle](#frame-lifecycle-render-seams) below.

## The documents

- **[Building New UI](building-new-ui.md)** — the C++ UI kit's shape and the RmlUi/native
  boundary; base class, folder, widgets, the C++ side of an RmlUi window, and the ownership rules. Read before starting anything under `UI/`.
- **[Component Catalog](component-catalog.md)** — reusable RmlUi/RCSS primitives that exist, and
  what doesn't exist yet. Check before inventing a one-off mechanism.
- **[Layout, Anchoring & Scaling](layout-and-scaling.md)** — the UI-scale (`dp`) mechanism, the
  anchor/stretch/center utility classes, and the scale sweep for hit boxes read from RCSS.
- **[Window Placement](window-placement.md)** — the theme-owned workspace: regions, docks, slots,
  the HUD in the workspace, dragging, and theme recipes.
- **[Theming & Modding](theming-and-modding.md)** — the theme mechanism, adding a theme, and the
  modding constraints.
- **[Engine Findings](engine-findings.md)** — empirical gotchas of this RmlUi build and the
  `CObject`/`CManager` machinery. Check before assuming a bug is new.
- **[Migration Ledger](migration-ledger.md)** — where each original window class went; check
  here for "what replaced `X`?".

## Renderer integration: SDL_GPU

RmlUi renders through its own vendored SDL_GPU backend
(`src/ThirdParty/RmlUi/Backends/RmlUi_Renderer_SDL_GPU.cpp`) rather than a bridge against
`IMuRenderer`'s game-oriented primitives. `RmlUiRenderInterface`
(`Render/RmlUi/RmlUiRenderInterface.cpp/.h`) subclasses it, overriding only
`LoadTexture`/`ReleaseTexture` so game assets (`.OZT`/`.OZJ`) route through `CGlobalBitmap`.
The submodule points at the fork `nitoygo/RmlUi` (`integration/sdl-gpu-parity`), which carries
the renderer work upstream doesn't have yet; renderer changes are committed there first, then the
submodule pin is moved.
`transform`, gradients, `box-shadow` and `backdrop-filter: blur()` render; most of `filter` and
`mask-image` don't — `engine-findings.md` has the current list. Test a new visual property in isolation rather
than trusting the parser.

**Texture-lifetime rule**: `CGlobalBitmap`'s numbered slots get force-reassigned across scene
transitions, while RmlUi caches a texture handle indefinitely. Any RmlUi-loaded texture must use
`CGlobalBitmap::LoadImageExclusive()`, never the shared `LoadImage()` — sharing a slot crashes on
scene re-entry.

## Frame lifecycle: render seams

Native drawing and RmlUi share one frame, and each step draws over everything before it.
Hexagons are native 3D, rounded boxes are RmlUi contexts:

```mermaid
flowchart TD
    scene["World and legacy 2D"]
    subgraph loop["CManager::Render(): windows in layer-depth order"]
        win{{"Each window's native Render(): state for its document,<br/>what little still draws natively"}}
    end
    rtt{{"Offscreen pass: RenderTarget drawers into textures:<br/>the item windows' items and effects, the cash shop, potions,<br/>letter portrait, character creation, item previews, the item on the cursor"}}
    main("Pre-submit: the one RmlUi context, every document;<br/>render-target textures show here as images")
    post["Post-RmlUi pass: the cursor"]
    scene --> win --> rtt --> main --> post
```

A flush draws what has been recorded so far (`FlushRenderCommands()`), so a context rendered right
after it lands over that and under whatever is recorded next. The seams are on `IMuRenderer` and
registered once in `Winmain.cpp`.

- **The main context renders last** (`SetPreSubmitCallback`, `RmlUiRuntime::RenderFrame()`), so an
  opaque panel covers any legacy drawing meant to stay on top. That drawing goes in the post-RmlUi
  pass (`SetPostRmlUiCallback`), which also runs `CSystem::SyncMainSceneHudVisibility()` every
  frame.
- **Native drawing inside one document** goes into a `UI::RmlBridge::RenderTarget`
  (`SetOffscreenRenderCallback`, `component-catalog.md`), so it sits at its element's depth: live
  3D, and native 2D a window still draws (`SetOffscreen2DRect()`).

## Data binding: `RmlModelBinder<T>`

`UI::RmlBridge::RmlModelBinder<Model>` (`UI/RmlBridge/RmlModelBinder.h`) wraps a per-window
`Rml::DataModel`: game state → `SyncRmlModel()` → a plain data struct → bound via
`Rml::DataModelConstructor`. **The model must be created before the document that references it
is loaded** — a model created after `LoadDocument()` is too late and every binding shows its
literal source text. Clicks go through `data-event-click` → `BindEventCallback` (right-click
through `data-event-mouseup`, button 1). A window with no dynamic state can skip the binder and
use `AddEventListener`.

A window holds its binder and documents through `UI::RmlBridge::ThemedView<Model>`
(`UI/RmlBridge/RmlThemedView.h`), which creates the model before loading the documents, rebuilds
both on a theme switch and tears them down; `component-catalog.md` has its options.

## Theming

A theme is a **folder name** — adding one needs no recompile. A window's `ThemedView` loads its
`.rml` (through `ThemedDocumentLoader`, `UI/RmlBridge/RmlTheme.h/.cpp`) against a synthetic
`themes/<active-theme>/` URL, so its `<link href>` pulls that theme's `.rcss`; a theme can also
fork the `.rml` itself (`themes/<theme>/<name>.rml`).

**Two themes, `legacy`** (real sprite art, pixel parity with the original) **and `modern`**, are
the intended set — a third was ruled out by the project owner. Change both in the same pass.
Shared rules live in `themes/<name>/base.rcss` (`.btn`, `.checkbox-box`, `.hidden`, the
mandatory `body { pointer-events: none; }` reset). Full mechanism in
[Theming & Modding](theming-and-modding.md).

## Interaction helpers

Any interaction more than one window wants belongs in `UI::RmlBridge` as a shared primitive.
`MakeDraggable(handle, panel, onMove)` (`UI/RmlBridge/RmlDraggable.h/.cpp`) builds on RmlUi's drag
events; the handle needs `pointer-events: auto`, and it moves both axes. Dialogs and the friend
family drag; `CMyInventory` persists its position in `config.ini`. See `component-catalog.md`'s
"Dragging" and `window-placement.md` for the scope.

## Scene windows

The login- and character-scene windows (`g_*Win` globals) are `CObject`s owned by
`CSceneUICoordinator`, which calls each one's `Release()` by name on every scene transition. A
document is created once and reused, and RmlUi renders last, so **each such window's `Release()`
must hide its document** (`CLoginWin::Release()`), or it paints over the next scene.

## Gotchas

- **`pointer-events` trap**: `Context::IsMouseInteracting()` is true over *any* RmlUi hover
  target, so a full-window document with default `pointer-events` swallows every click. Every
  document needs `body { pointer-events: none; }`, opted back in per interactive element
  (`base.rcss` does this for every window that links it).
- **Mouse input is handled directly**, not via RmlUi's `RmlSDL::InputEventHandler`, which calls
  `SDL_CaptureMouse()` on every click and double-scales motion by pixel density. Wheel, key and
  text events use the vendored handler.
- **`.hidden` vs `.disabled`**: `.disabled` keeps an element in layout (dimmed); `.hidden`
  (`display: none`) removes it. Use `.hidden` when something doesn't apply to the current scene at
  all — a dimmed-but-present button once overlapped its neighbour in `CSysMenuWin`.
- **Paint order**: a parent's background paints before its children, and children paint in
  document order. Move the visually topmost element later in the document rather than fighting
  with z-index.

## Source map

| Subsystem | Key files |
|---|---|
| Runtime lifecycle | [`Render/RmlUi/RmlUiRuntime.h/.cpp`](../../src/source/Render/RmlUi/RmlUiRuntime.h) |
| Render interface | [`Render/RmlUi/RmlUiRenderInterface.h/.cpp`](../../src/source/Render/RmlUi/RmlUiRenderInterface.h) |
| System interface | [`Render/RmlUi/RmlUiSystemInterface.h/.cpp`](../../src/source/Render/RmlUi/RmlUiSystemInterface.h) |
| Renderer seams | [`Render/Renderer/MuRenderer.h`](../../src/source/Render/Renderer/MuRenderer.h) |
| Input gating | [`Core/Input/UiInputRouter.cpp`](../../src/source/Core/Input/UiInputRouter.cpp) (`IsMouseOverUI()`) |
| Model/binder layer | [`UI/RmlBridge/RmlModelBinder.h`](../../src/source/UI/RmlBridge/RmlModelBinder.h), [`UI/RmlBridge/RmlThemedView.h`](../../src/source/UI/RmlBridge/RmlThemedView.h) |
| Theme framework | [`UI/RmlBridge/RmlTheme.h/.cpp`](../../src/source/UI/RmlBridge/RmlTheme.h) |
| Draggable helper | [`UI/RmlBridge/RmlDraggable.h/.cpp`](../../src/source/UI/RmlBridge/RmlDraggable.h) |
| Native drawing in a document | [`UI/RmlBridge/RmlRenderTarget.h/.cpp`](../../src/source/UI/RmlBridge/RmlRenderTarget.h) |
| Workspace placement | [`UI/Placement/WindowPlacement.h/.cpp`](../../src/source/UI/Placement/WindowPlacement.h) |
| Texture lifetime | [`Render/Sprites/GlobalBitmap.h/.cpp`](../../src/source/Render/Sprites/GlobalBitmap.h) — `LoadImageExclusive()` |
| RML/RCSS assets | [`bin/Data/Interface/RmlUi/`](../../src/bin/Data/Interface/RmlUi/) — one `.rml` per window + `themes/{legacy,modern}/` |
