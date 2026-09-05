# RmlUi UI System

> **Start with [`architecture-principles.md`](architecture-principles.md)** — the governing
> policy for this migration (layout intent, responsive/scalable/themeable/moddable design). This
> README and every other doc here implement or report status against it; none of them repeat its
> reasoning. Check [`STATUS.md`](STATUS.md) for what's actually done, known gaps, and open
> conflicts against that policy.

How [RmlUi](https://github.com/mikke89/RmlUi) (HTML/CSS-driven UI middleware) is integrated into
this client's SDL_GPU renderer, and the patterns/gotchas worth knowing before migrating the next
window. Condensed to what matters for building on this branch — not a full history, not every
detail.

## Why this exists

The client's game UI is spread across three legacy widget frameworks — the `CWin`/`CButton`
widget set, the `CUIControl`/`CUIBaseWindow` toolkit (`UIControls.h`), and the `CNewUIObj` tier,
all living directly under `UI/` in topic folders (`UI/Widgets/`, `UI/HUD/`, `UI/Inventory/`, etc. —
see `docs/newui-legacy-merger.md` for the folder history) — with no layout engine, retained scene
graph, or data-binding layer between them. RmlUi is being adopted as the long-term replacement per
[`architecture-principles.md`](architecture-principles.md), migrated window by window, old and
new systems coexisting rather than a big-bang rewrite. See [`STATUS.md`](STATUS.md) for what's
migrated so far. `COptionWin` was ported but deliberately not wired up — see [Coexistence
patterns](#coexistence-patterns) below.

See also: **[Theming & Modding](theming-and-modding.md)** — the full theme mechanism, a
step-by-step guide for adding a theme, and the modding constraints (image format, scaling,
positioning ownership). **[Layout, Anchoring & Scaling](layout-and-scaling.md)** — the global
UI-scale (`dp`) mechanism, the anchor/stretch/center utility classes every new window should use,
and a worked example of retrofitting an already-migrated window. **[NewUI-Tier Adapter
Pattern](newui-tier-adapter.md)** — the `CNewUIObj`/`CNewUIManager` tier (in-game HUD, distinct
from `CWin`/`CUIMng`): the adapter shape, the `MAIN_SCENE` input-gating prerequisites, and what's
still unproven there. **[Legacy Theme Modernization Policy](legacy-theme-modernization.md)** —
when legacy-theme C++ behavior should move into RML/RCSS versus genuinely stay in C++, and how to
classify a given piece of legacy code either way. **[Modern Theme Visual Direction](modern-theme-visual-direction.md)**
— the `modern` theme's color/border/shadow token table and the design-language goals it serves;
`legacy` is explicitly out of scope for it. **[Component Catalog](component-catalog.md)** — an
honest inventory of what already exists as a reusable UI primitive (Window/Panel, Button,
Checkbox, layout utilities, data binding, theming, dragging) versus what a future port would be
the first real use case for (ItemSlot, ProgressBar, Tooltip unification, Dialog, and more) — check
here before inventing a new one-off mechanism.

## Renderer integration: SDL_GPU

RmlUi renders through its own vendored SDL_GPU backend
(`src/ThirdParty/RmlUi/Backends/RmlUi_Renderer_SDL_GPU.cpp`) rather than a bridge against
`IMuRenderer`'s game-oriented primitives (`RenderQuad2D` etc. — fixed small-vertex-count helpers,
a poor fit for RmlUi's compile-once/render-many arbitrary-geometry contract). `RmlUiRenderInterface`
(`Render/RmlUi/RmlUiRenderInterface.cpp/.h`) subclasses it, overriding only
`LoadTexture`/`ReleaseTexture` so file-backed images (real game assets, `.OZT`/`.OZJ`) route
through `CGlobalBitmap` instead of the vendored backend's generic SDL_image loader, which can't
read this engine's proprietary formats at all. `SetTransform` (CSS `transform`) is a real,
working capability of this backend — worth knowing if a future theme wants to use it.

**Texture-lifetime rule**: `CGlobalBitmap`'s numbered-slot cache is designed for code that
re-resolves a texture by logical id every frame (`CSprite` etc.) — slots get silently
force-reassigned to a different file across scene transitions. RmlUi caches a resolved texture
handle indefinitely instead, so any RmlUi-loaded texture must use
`CGlobalBitmap::LoadImageExclusive()`, never the shared, ref-counted `LoadImage()` — sharing a
slot causes a real crash on scene re-entry.

## Frame lifecycle: RmlUi renders last — via two seams

**RmlUi always renders after all other content for that frame.** `RmlUiRuntime::Render()` fires
from exactly one fixed pre-submit callback (below) — this is how the integration works today, not
a proven RmlUi requirement; nobody's investigated whether interleaving with legacy content is
possible instead (multiple contexts, or a callback hook legacy content renders through at the
right point in RmlUi's own z-order). See `STATUS.md` for that open question and the two C++ call
sites (`NewUIMainFrameWindow.cpp`) that currently work around today's ordering by picking between
an RmlUi fill and a legacy one. Until that's resolved, treat today's ordering as real: get it
wrong and an opaque RmlUi panel covers legacy content (cursor, text) that's supposed to stay
visible on top of it. Two callbacks on `IMuRenderer`, registered once in `Winmain.cpp`, make it
work:

- **`SetPreSubmitCallback`** — fires after the frame's game/legacy-2D content is recorded onto
  the command buffer but before submit. `RmlUiRuntime` registers this once in `Create()`; this is
  where `Rml::Context::Render()` actually happens. A single choke point every scene funnels
  through uniformly.
- **`SetPostRmlUiCallback`** — fires after RmlUi's own pass, for content that must render even
  later than RmlUi itself (the cursor, `CLoginWin::RenderTextOnTop()`'s input-box text). By the
  time RmlUi's pass is recorded its render pass is already closed, so drawing more content after
  it needs its own seam: a fresh render pass targeting the same swapchain texture with
  `LOAD_OP_LOAD` (preserve what's already there), plus re-staging any newly queued vertex data.
  Building this required getting three things right: a frame-active guard so 2D draw calls issued
  from the callback don't early-return, resetting draw-command merge-tracking so a quad in this
  pass never backward-merges into a stale command from the main pass, and assigning the new
  render pass to the renderer's own tracked handle (not a local) so its draw calls actually land
  in it. Full detail in `.ai-os/memory/tasks/rmlui-sdl-gpu-port.md` if this seam needs revisiting.

**A same-frame update-order gotcha worth knowing for any similar modal**: `CWin::Update()` always
calls `UpdateWhileShow()` before `UpdateWhileActive()` in the same frame. A dialog that resolves
its own pending state inside `UpdateWhileShow()` (like `RememberPasswordPrompt`'s `Tick()`) can
cause a same-frame cascade if some *other* window's `UpdateWhileActive()` re-checks that state
expecting it to still be "pending" — by the time it runs, it's already resolved. Fix: snapshot
the state before the call that can resolve it, check the snapshot afterward, not a live re-check.

## Data binding: `RmlModelBinder<T>`

`UI::RmlBridge::RmlModelBinder<Model>` (`UI/RmlBridge/RmlModelBinder.h`) wraps a per-window
`Rml::DataModel`: game state → `BuildModel()`/`SyncRmlModel()` → a plain data struct → bound via
`Rml::DataModelConstructor`. **The model must be created before the document that references it
is loaded** — RmlUi resolves `data-model`/`{{bindings}}` while parsing the RML; a model created
after `LoadDocument()` is too late and every binding falls back to its literal source text.
Click/toggle actions go through `data-event-click` → `DataModelConstructor::BindEventCallback`,
calling the same free functions/methods the legacy UI already called — day-one call sites stay
unchanged. A window with no dynamic state (`CLoginMainWin`) can skip the binder entirely and wire
clicks via a plain `Rml::Element::AddEventListener` + a small self-owning listener class instead.

## Theming

A theme is a **folder name**, not a closed enum — adding one is a drop-a-folder operation, no
recompile. `UI::RmlBridge::LoadThemedDocument()` (`UI/RmlBridge/RmlTheme.h/.cpp`) reads a window's
`.rml` text once, then calls `Rml::Context::LoadDocumentFromMemory(text,
syntheticThemeSourceUrl)` — RmlUi resolves the document's `<link href="...">` against that
synthetic URL, so `login.rml` ends up pulling `themes/<active-theme>/login.rcss`.

`legacy`'s RML+RCSS pair is the canonical reference implementation; any theme (`modern` included)
is expected to fork the RML entirely, not just restyle it in RCSS, the moment it needs genuinely
different structure — `main_frame.rml` already does. See **[Theming & Modding](theming-and-modding.md)**
for the full policy, known debt, and the drift-check tooling a forked theme still needs.

**Two themes are currently built: `legacy`** (real sprite art, pixel-parity with the original
look) **and `modern`** (flat/programmatic) — whenever a window's RmlUi content changes or a bug is
fixed in one, update the other's RCSS in the same pass, not as a follow-up. These two exist to
*validate* the architecture supports arbitrary themes, not as a permanent ceiling — see
`STATUS.md`. Shared cross-window rules live in `themes/<name>/base.rcss` (`.btn`, `.checkbox-box`,
`#backdrop`, `.hidden`, the mandatory `body { pointer-events: none; }` reset — see
[Gotchas](#gotchas) below).

Full mechanism, the step-by-step guide for adding a theme, and the modding constraints (image
format, scaling, positioning ownership) are in **[Theming & Modding](theming-and-modding.md)**.

## Interaction helpers: `UI::RmlBridge::MakeDraggable`

**Standing principle**: any interaction pattern more than one migrated window will want belongs
in `UI::RmlBridge` as a shared primitive, not reimplemented per window. `MakeDraggable(handle,
panel, onMove)` (`UI/RmlBridge/RmlDraggable.h/.cpp`) builds on RmlUi's native drag events rather
than hand-rolled mouse tracking. Two things worth knowing before using it: `handle` needs
`pointer-events: auto` set explicitly (inherited `none` under a full-window document means drag
events never fire at all, not even hover), and it moves **both** axes — a horizontal-only slider
thumb has to reset `top` back to a fixed value on every drag tick or it visibly drifts.

## Coexistence patterns

Two structural shapes exist for a migrated window: a **hybrid** `CWin` + RmlUi overlay that keeps
the legacy window's position/hit-testing bookkeeping but draws no visual chrome itself
(`CWin::Create()` always passes `nTexID=-2`) — `CLoginWin`, `CLoginMainWin`, `CSysMenuWin` — and a
**pure RmlUi** window with no `CWin`/`CUIMng` involvement at all — `RememberPasswordPrompt`, a
free-function module in `namespace UI::Login`.

**`CWin::Release()` has no idea `m_pRmlDoc` exists.** `CUIMng::RemoveWinList()` calls `Release()`
on every window on every scene transition; the legacy `Release()`/`PreRelease()` path never
touches an RmlUi document, so a hybrid window's document (created once, reused forever) stays
exactly as visible as it was — and since RmlUi renders last in the frame, it then paints on top
of whatever the *next* scene draws, indefinitely. Every hybrid window's `PreRelease()` override
must unconditionally `if (m_pRmlDoc) m_pRmlDoc->Hide();` — not optional boilerplate, even if some
other call site currently happens to hide it first.

**`COptionWin` was ported but is deliberately not wired up** — confirmed unreachable in live
play: `CSysMenuWin`'s Option button opens `SEASON3B::CNewUIOptionWindow` instead. Its RmlUi
content is self-contained (own model, own slider-drag math) so leaving it un-rewired carries no
shared-dependency risk. Retiring one of the two implementations is a real product decision, left
open on purpose.

## Gotchas

- **`pointer-events` trap**: `Context::IsMouseInteracting()` returns true for *any* RmlUi hover
  target, not just clickable ones — a full-window document with default `pointer-events`
  therefore swallows every click on screen, including ones meant for legacy content underneath.
  Every full-window document needs `body { pointer-events: none; }`, opted back in per
  interactive element (`base.rcss` does this once for every window that links it).
- **Mouse input is handled directly**, not via RmlUi's vendored `RmlSDL::InputEventHandler` — that
  handler calls `SDL_CaptureMouse()` on every click (breaks this engine's own cursor-clip
  handling) and scales motion by `SDL_GetWindowPixelDensity()` (double-applies against an already
  real-pixel-sized `Rml::Context`). Wheel/key/text events still use the vendored handler.
- **`.hidden` vs `.disabled`**: `.disabled` keeps an element in layout (dimmed, temporarily
  unclickable) — safe only if the layout was actually designed to fit the element both present
  and dimmed. `.hidden` (`display: none`) removes it from layout entirely. Use `.hidden` (and a
  model field named accordingly) when a button doesn't apply to the current scene at all, not
  merely "temporarily off" — conflating the two caused a real dimmed-button-overlaps-neighbor bug
  in `CSysMenuWin` when opened from a shorter panel (login scene vs. character-select).
- **Paint order**: a parent element's own background always paints before its children. A
  decorative border placed as a *child* can still paint over another child that's meant to sit on
  top of it if that child comes earlier in document order — reorder by moving the visually-topmost
  element later in the document, not by trying to fight it with z-index tricks.
- **Same-frame update-order cascades** — see [Frame lifecycle](#frame-lifecycle-rmlui-renders-last--via-two-seams) above.

## Source map

| Subsystem | Key files |
|---|---|
| Runtime lifecycle | [`Render/RmlUi/RmlUiRuntime.h/.cpp`](../../src/source/Render/RmlUi/RmlUiRuntime.h) |
| Render interface | [`Render/RmlUi/RmlUiRenderInterface.h/.cpp`](../../src/source/Render/RmlUi/RmlUiRenderInterface.h) |
| System interface | [`Render/RmlUi/RmlUiSystemInterface.h/.cpp`](../../src/source/Render/RmlUi/RmlUiSystemInterface.h) |
| Renderer seams | [`Render/Renderer/MuRenderer.h`](../../src/source/Render/Renderer/MuRenderer.h) — `SetPreSubmitCallback`/`SetPostRmlUiCallback`/`GetFrameGpuContext`/`GetDevice`/`GetWindow`/`GetRawTexture` |
| Model/binder layer | [`UI/RmlBridge/RmlModelBinder.h`](../../src/source/UI/RmlBridge/RmlModelBinder.h) |
| Theme framework | [`UI/RmlBridge/RmlTheme.h/.cpp`](../../src/source/UI/RmlBridge/RmlTheme.h) |
| Draggable helper | [`UI/RmlBridge/RmlDraggable.h/.cpp`](../../src/source/UI/RmlBridge/RmlDraggable.h) |
| `SetMovable` | [`UI/Widgets/Win.h/.cpp`](../../src/source/UI/Widgets/Win.h) — replaces per-class `CursorInWin(WA_MOVE)` overrides |
| Texture lifetime | [`Render/Sprites/GlobalBitmap.h/.cpp`](../../src/source/Render/Sprites/GlobalBitmap.h) — `LoadImageExclusive()` |
| Migrated windows (`CWin` tier) | [`LoginWin`](../../src/source/UI/Windows/LoginWin.h), [`LoginMainWin`](../../src/source/UI/Windows/LoginMainWin.h), [`SysMenuWin`](../../src/source/UI/Windows/SysMenuWin.h), [`RememberPasswordPrompt`](../../src/source/UI/Windows/RememberPasswordPrompt.h), [`OptionWin`](../../src/source/UI/Windows/OptionWin.h) (ported, not wired up), [`CCharSelMainWin`](../../src/source/Character/CharSelMainWin.h), [`CCharMakeWin`](../../src/source/Character/CharMakeWin.h), [`CCharInfoBalloonMng`](../../src/source/Character/CharInfoBalloonMng.h), [`MsgWin`](../../src/source/UI/Windows/MsgWin.h) |
| Migrated windows (`CNewUIObj` tier) | [`CMuHelperBar`](../../src/source/UI/HUD/MuHelperBar.h), [`CBuffStrip`](../../src/source/UI/HUD/BuffStrip.h) (fully done) — see [newui-tier-adapter.md](newui-tier-adapter.md). [`CNewUIMainFrameWindow`](../../src/source/UI/HUD/NewUIMainFrameWindow.h) is 2 of 3 planned phases done (`STATUS.md`'s "What's migrated") — its file also still houses two fully-legacy classes (`CNewUISkillList`/`CNewUIItemHotKey`), not yet ported. |
| RML/RCSS assets | [`bin/Data/Interface/RmlUi/`](../../src/bin/Data/Interface/RmlUi/) — one `.rml` per window + `themes/{legacy,modern}/` |

## Status

See [`STATUS.md`](STATUS.md) — what's migrated, the per-port checklist, empirical findings worth
knowing before the next port, and known gaps against `architecture-principles.md`. All further
RmlUi UI work continues on this branch.
