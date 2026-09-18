# Theming & Modding

How the swappable-theme system works, and how to add a new theme — including a modder-supplied
one — without touching engine source code.

## Core principle: `legacy` is the canonical reference; other themes may fork freely

**The `legacy` theme's RML+RCSS pair is the canonical, documented reference implementation** —
the plainest expression of what a window needs: which elements must exist, which ids/classes C++
binds to (`GetElementById`, `DataModelConstructor::Bind`, event callbacks), and a straightforward
structure with no theme-specific opinions baked in. Any other theme — `modern` included — is
**expected to fork the RML entirely**, not just restyle it in RCSS, the moment it wants a
genuinely different structure: a different number of decorative layers, an element the base
version doesn't have, a layout the shared file can't express through classes alone. This is the
**normal path for a theme with real ambition**, not a rare escape hatch to feel bad about reaching
for — a theme that only wants to restyle colors/borders/fonts can still get away with an RCSS-only
reskin against the shared file, but nothing about the architecture should discourage forking when
a theme needs more than that.

**Fixed 2026-09-04** — `login.rml`, `msg_win.rml`, and `remember_password_prompt.rml` used to have
`modern`-specific class names (`modern-frame`, `modern-frame-accent`, `modern-panel`, etc.)
hardcoded directly into what's supposed to be the shared, theme-neutral file — backwards, since it
meant `legacy` was the one being constrained by `modern`'s vocabulary, not the other way around.
Each now has a `themes/modern/` fork carrying those classes; the shared file is `legacy`'s own
theme-neutral copy, same pattern as `main_frame`'s existing two-file case below.

**Forking safely needs a check — built 2026-09-04**: whichever theme's RML a window loads, the
C++ side still expects the exact same ids/`data-model` bindings/event-callback names to exist.
`tools/check_rml_rcss_drift.py` (a sibling to `check_rml_rcss_syntax.py`, wired into the same build
step) diffs the ids/bindings a window's C++ actually references against every theme's copy of that
window's RML and fails the build if none of them provide something the code needs — otherwise a
missing or renamed id would fail **completely silently** (a dead button, not a build error or even
a log line), the same failure shape as `main_frame`'s existing two-file hand-sync burden below.

## How theme resolution works

A theme is identified by **folder name**, not a closed C++ enum, specifically so adding one
(including a fully programmatic, sprite-free one) is normally a zero-source-change,
drop-a-folder operation.

`config.ini`'s `[UI] RmlTheme=<name>` selects the active theme, read once at startup by
`GameConfig::GetRmlTheme()` and cached by `UI::RmlBridge::GetActiveThemeName()`. That cache is
also live-mutable via `UI::RmlBridge::SetActiveThemeName()` — see
["Switching themes without relaunching"](#switching-themes-without-relaunching) below. Loading a
window goes through `UI::RmlBridge::LoadThemedDocument(context, "Data/Interface/RmlUi/login.rml")`:

1. It reads `login.rml`'s raw text once — the same file is shared, never duplicated per theme.
2. It builds a synthetic source URL, `Data/Interface/RmlUi/themes/<name>/login.rml` — this path
   need not exist on disk.
3. It calls `Rml::Context::LoadDocumentFromMemory(text, syntheticSourceUrl)`.
4. RmlUi resolves the document's `<link href="login.rcss"/>` relative to that synthetic URL,
   landing on `Data/Interface/RmlUi/themes/<name>/login.rcss` — which *does* need to exist on
   disk.

This is confirmed against RmlUi's own source: `Context::LoadDocumentFromMemory` wraps the string
in a `StreamMemory` and calls `SetSourceURL()` on it before parsing; `XMLNodeHandlerHead.cpp`'s
`MakeExternalResource()` resolves `<link href>` via `AbsolutePath(path, parser->GetSourceURL())`.

Every migrated window renders 100% of its own chrome through RmlUi, in every theme — nothing is
drawn by the legacy `CWin`/`CSprite` path underneath it. A "legacy-look" theme reproduces the
original art by pointing its own RCSS decorators at the same image files the old sprites used;
a "modern" theme uses flat colors/vector shapes instead.

## Forking a theme's RML: the mechanism, and today's real examples

`UI::RmlBridge::LoadThemedDocument()` ([`RmlTheme.cpp`](../../src/source/UI/RmlBridge/RmlTheme.cpp))
looks for `themes/<theme>/<name>.rml` first, falling back to the shared `<name>.rml` when no such
file exists. As of this writing, `modern` has its own `.rml` for `login`, `msg_win`,
`remember_password_prompt`, `my_inventory` (+ its background-context companion
`my_inventory_bg`), `my_quest_info`, and `main_frame` — every other window has no per-theme
override, so the lookup is a no-op for them (one extra failed `ifstream` open). Grep
`themes/modern/*.rml` for the live, current list rather than trusting this paragraph's exact
file names — it's kept current on a best-effort basis, not mechanically checked. Most of these
exist so `modern`'s own `.modern-frame`/`.modern-frame-crimson`/`.title-glow` class vocabulary
doesn't have to live in the shared, theme-neutral file (the Core Principle section above). `main_frame` is the one example forked for a **different** reason — two independently-maintained files —
[`themes/legacy/main_frame.rml`](../../src/bin/Data/Interface/RmlUi/themes/legacy/main_frame.rml)
and
[`themes/modern/main_frame.rml`](../../src/bin/Data/Interface/RmlUi/themes/modern/main_frame.rml)
— because modern's bottom-HUD button row moved to a genuinely different place in the document
(a top-right panel, `#top_right_row`, echoing `mu_helper_bar`'s styling) while legacy's stayed
nested inside `#bars` where the original bottom-HUD button row always was. Two things were tried
and rejected first: CSS-only hiding (`display: none` on modern's copy of the old row leaked
through no matter how it was hardened) and a `data-if` bound to a C++-set "is modern" model
boolean (rejected as the same kind of per-context C++ branching
[`legacy-theme-modernization.md`](legacy-theme-modernization.md) exists to move *out* of C++, just
relocated into a model field instead of an `if` statement).

**Fork the RML when a theme's content genuinely differs in DOM structure or position, not merely in
visibility or style** — "this element doesn't exist in the other theme's layout at all"
(main_frame's button row) is worth a fork; "this element is hidden/styled differently" (ordinary
RCSS `.hidden`/selector differences) isn't — RCSS alone already covers that case, and forking for
it would just be needless duplication, not a policy violation. A real fork does come with a cost,
currently paid entirely by hand for `main_frame`: the two files' shared ids/classes/bindings have
to be kept in sync manually (each file's own header comment says so explicitly — check the other
file's comment before editing either one) — see the Core Principle section above for the drift-
check tooling this should eventually have and doesn't yet.

No source changes, no recompilation.

1. Create `src/bin/Data/Interface/RmlUi/themes/<your-theme-name>/`.
2. Write `base.rcss` in that folder — every window links this first for its shared rules
   (`.btn`, `.checkbox-box`/`.checkbox-box.checked`, `#backdrop`, `.hidden`, and the mandatory
   `body { pointer-events: none; }` reset). A theme missing it renders those windows' buttons/
   checkboxes/backdrop completely unstyled.
3. Write each window's own positional `.rcss` (`login.rcss`, `login_main.rcss`, `sys_menu.rcss`,
   `remember_password_prompt.rcss`, ...), styling the same class names the shared `.rml` uses. A
   theme missing one of these still loads (RmlUi doesn't error on a missing stylesheet) — it just
   renders that one window unstyled, not the whole theme.
4. Edit `config.ini`'s `[UI]` section: `RmlTheme=<your-theme-name>`, or preview it live without
   editing anything — type `$theme <your-theme-name>` in chat (see
   ["Switching themes without relaunching"](#switching-themes-without-relaunching) below).
5. Relaunch (if you edited `config.ini`). No rebuild needed either way — this is a pure data/config
   change.

## Switching themes without relaunching

`$theme <name>` (typed into the chat box, handled locally by `CmuConsoleDebug::CheckCommand` —
never sent to the server, same bucket as `$fps`/`$vsync`) hot-swaps the active theme for the
current session: `$theme modern`, `$theme legacy`, or any modder-supplied folder name.

Mechanically: it validates `themes/<name>/base.rcss` exists (`UI::RmlBridge::ThemeExists()`) —
an unknown name is rejected with no state change, not left to fail silently per-window — then
calls `UI::RmlBridge::SetActiveThemeName()` to update the live cache and
`mu::ui::window::CManager::ReloadAllRmlThemes()` to sweep every currently-registered window.
Each themed window implements this by overriding `IObject::ReloadRmlTheme()`
(`UI/Core/WindowObject.h`) — a no-op default, so the sweep is safe to call on every window in the
registry, not just the themed ones — to tear down its `Rml::ElementDocument`/`DataModel`(s) via
the new `RmlModelBinder<T>::Destroy()` and `Context::UnloadDocument()`, then rebuild them against
whatever theme is now active, the same `BuildRmlUi()` helper `Create()` itself calls. A window
that was never opened needs no explicit rebuild — it simply picks up the new theme the first time
it *is* opened, since `LoadThemedDocument()` always reads the live cache.
`UI::Login::ReloadRmlTheme()` (`RememberPasswordPrompt.h`) is the one exception called explicitly
rather than through the registry sweep — that dialog is a free-function module, not a `CObject`,
so it isn't in `CManager`'s registry.

**Session-only**: this does not write to `config.ini` — `GameConfig::SetRmlTheme()` only updates
the in-memory value, so a relaunch still picks up whatever `config.ini` says. Use it for quickly
A/B-ing themes; edit `config.ini` (previous section) for a change that should survive a restart.

**Only two themes are currently built: `legacy` and `modern`.** These two exist to *validate* that
the architecture actually supports arbitrary themes, not because two is the intended ceiling —
`architecture-principles.md` §25/§28 envisions a Custom/Test theme beyond these two, specifically
because a theme that looks substantially different from `legacy` is what would expose accidental
coupling between a component's implementation and one particular visual design. A third (or
fourth, or user-authored) theme is expected eventually; see `STATUS.md` for current sequencing.
Keep `legacy` and `modern` updated together for any window content change in the meantime — don't
let one lag.

### `#backdrop` usage

```html
<div id="backdrop"></div>
<div id="panel" data-model="...">
    ...
</div>
```

Placed as the *first* child of `<body>`, before the panel, so it paints underneath. Unlike the
panel's children, `#backdrop` sets `pointer-events: auto` directly (not inherited) — a full-screen
dim backdrop is meant to consume clicks underneath it (matching how a full-screen legacy dialog
already treated the whole screen as "in this window"), not create a click-through hole.

## Bringing your own images to a theme

A theme is free to reuse the original game art or bring entirely new image assets (a custom
background, custom button art, a logo). Both go through the same path: any image an RCSS file
references loads through this engine's normal texture pipeline
(`RmlUiRenderInterface::LoadTexture` → `CGlobalBitmap::LoadImage`). No engine changes are needed
to use it — but there are real constraints worth knowing first.

**Path resolution**: a relative image path inside an RCSS rule resolves against *that
stylesheet's own location*, not the shared `.rml`'s synthetic per-theme URL — so a custom theme's
image can simply sit next to its own `.rcss`:

```
themes/<your-theme-name>/
    login.rcss
    panel_bg.tga      <- referenced from login.rcss below
```

```rcss
#panel {
    decorator: image( panel_bg.tga );
}
```

**Only this engine's proprietary OZT/OZJ containers are actually read, not plain PNG/JPG/BMP.**
`CGlobalBitmap::LoadImage` only recognizes two extensions, `.jpg`/`.tga` — and **both internally
swap the extension to `.OZJ`/`.OZT` before opening.** Concretely: if your RCSS references
`panel_bg.tga`, the file that actually needs to exist on disk is `panel_bg.OZT`, not a real
`.tga`. There is no PNG/JPG→OZT/OZJ converter in this repository — producing one needs an
external tool from the wider MU private-server modding community (the format predates this
project).

**A non-power-of-two-sized image needs an explicit `@spritesheet` declaration, or it renders
squished with visible padding.** The `.OZT`/`.OZJ` loaders pad every texture up to the next
power-of-two size internally (a 329×245 image becomes a 512×256 texture, real content in the
top-left corner, the rest zero-filled) — invisible to a legacy `CSprite` (which tracks its own
real dimensions separately) but **not** invisible to a plain `decorator: image("file.tga")`,
which always samples the *entire* stored texture as 0..1 UV. Fix: declare the image as a named
sprite with its real pixel rectangle, and reference the sprite name instead of the raw path:

```rcss
@spritesheet panel-bg-sheet
{
    src: panel_bg.tga;
    panel-bg-image: 0px 0px 329px 245px;  /* the image's REAL, unpadded pixel size */
}

#panel {
    decorator: image(panel-bg-image);   /* the sprite name, not "panel_bg.tga" directly */
}
```

If the image's real dimensions genuinely are an exact power of two, this step is unnecessary.

**Failures are completely silent.** A missing file or unsupported extension returns `false` from
`LoadImage` with no log line at all — the element just renders with no image, no error, no
diagnostic. If a custom theme's image isn't showing up, check the extension-swap rule above (a
real `.tga` file, or a file with the wrong internal container, both fail exactly the same silent
way) before assuming something else is wrong.

**Open follow-up, not built**: vendoring a small PNG/JPG/BMP decoder to give `LoadTexture` a
fallback path when the OZT/OZJ lookup fails, specifically for RmlUi-referenced theme assets —
would remove the proprietary-format conversion step for modders without touching the legacy
game-asset pipeline everything else still depends on.

## Coordinates, scaling, and positioning — what a theme actually controls

### Scaling: a global user setting, opt-in per element

**See [Layout, Anchoring & Scaling](layout-and-scaling.md) for the full policy** — condensed here:
`GameConfig::GetUIScalePercent()` (`[UI] UIScalePercent`, default 100) is applied once via
`Context::SetDensityIndependentPixelRatio()` (`RmlUiRuntime::Create()`/`OnResize()`). A coordinate
written in `dp` scales with that setting; one written in `px` never does. Most of the original
two themes still use fixed `px` throughout and are simply unaffected by the setting until
retrofitted — that's fine, not a bug. A theme renders `px` content at the same physical pixel size
regardless of window resolution or `UIScalePercent`; this is also the practically correct choice
for raster sprite art regardless of the scaling mechanism available, since an image has a native
resolution and stretching it non-uniformly or upscaling it blurs/pixelates. A custom-sprite theme
should expect to look the same physical size at every resolution unless it deliberately opts a
coordinate into `dp`.

### Panel position vs. element layout vs. draggability — three different owners

- **Layout of elements *within* the panel** — fully expressed in RCSS, exactly what a theme
  controls.
- **The panel's own position on screen** — for a hybrid `CWin` + RmlUi window, this is driven by
  the legacy window's own `SetPosition()`/centering math in C++, pushed into the panel's
  `left`/`top` RCSS properties every frame/resize. A theme's RCSS receives this position; it
  doesn't choose it. A window with no leftover `CWin` positioning dependency could express its
  own anchoring purely in RCSS instead (`position: absolute; right: 0; bottom: 0;`, etc.).
- **Draggability** — a legacy `CWin` concept (`Win::SetMovable()`) unrelated to RmlUi/RCSS, or
  (for a pure-RmlUi window) `UI::RmlBridge::MakeDraggable()`. RmlUi/RCSS has no built-in
  "make this draggable" CSS property.

## Known limitations

- **Live hot-swap is session-only.** `$theme <name>` (see
  ["Switching themes without relaunching"](#switching-themes-without-relaunching) above) rebuilds
  every open window's document/model in place, but doesn't persist to `config.ini` — a relaunch
  still uses whatever's saved there. A window with genuinely live, frequently-changing state
  (`CMuHelperBar`, `CBuffStrip`, `CCharInfoBalloonMng`, `CMainFrameWindow`) relies on its own
  normal per-frame resync to repopulate the rebuilt model correctly; a window that doesn't
  self-resync every frame pushes its current state back explicitly right after rebuilding instead
  (see each window's own `ReloadRmlTheme()`) — a new themed window should follow whichever of the
  two patterns matches its own update shape, not assume the sweep alone is enough.
- **Custom theme images require the engine's proprietary OZT/OZJ format, not plain PNG/JPG** —
  see [Bringing your own images to a theme](#bringing-your-own-images-to-a-theme) above. No
  converter tool exists in this repo today, and a missing/wrong-format image fails silently.
- **`dp`-scaling is wired up globally** (RmlUi's density-independent-pixel ratio auto-fits to
  window size and folds in `UIScalePercent`, `layout-and-scaling.md`'s "Global UI scale" section)
  — but a hybrid window's on-screen *position* still isn't theme-controlled (see
  [Coordinates, scaling, and positioning](#coordinates-scaling-and-positioning--what-a-theme-actually-controls)
  above).
- **Eleven windows are routed through `LoadThemedDocument()` today**: `CLoginWin`,
  `CLoginMainWin`, `CSysMenuWin`, `RememberPasswordPrompt`, `CCharSelMainWin`, `CCharMakeWin`,
  `CCharInfoBalloonMng`, `CMsgWin`, `CMuHelperBar`, `CBuffStrip`, and `CMainFrameWindow`. All eleven
  also implement `ReloadRmlTheme()` (previous section), so `$theme` covers every themed window that
  exists today. Extending a new window to support theming is the same established pattern for both
  halves, not new design work — this list will keep growing and isn't worth maintaining
  exhaustively; grep `LoadThemedDocument(` for the live count.
- **Theme identity must never drive C++ branching** — `architecture-principles.md` §30. Fixed
  2026-09-04: `MainFrameWindow.cpp`'s background-fill and skill-highlight logic used to key
  on `GetActiveThemeName() == "modern"`; both now key on `UI::RmlBridge::ThemeProvidesOwnIconChrome()`,
  a declared capability (`themes/modern/theme.ini`) — see `STATUS.md` for the real render-ordering
  constraint that makes the conditional itself legitimate (only the name-check was the violation),
  and the next bullet for whether that constraint is actually permanent.
- **RmlUi rendering strictly last in the frame is an integration choice, not a proven RmlUi
  requirement.** `RmlUiRuntime::Render()` fires from one fixed pre-submit callback, always after
  every legacy 2D/3D draw call for the frame — which is *why* the conditional above exists (an
  RmlUi-drawn fill would always paint over content that needs to render on top of it, since RmlUi
  composites last). Whether interleaving is possible (multiple contexts, or a callback hook legacy
  content renders through at the right point in RmlUi's own z-order) is still unexplored. The
  *other* direction — legacy 3D content that needs to paint **on top of** an already-composited
  RmlUi panel, not behind it — was attempted twice for `CGenericConfirmDialog`'s `item3D` preview
  (2026-09-13/14, sold-item confirm dialogs in `NPCShop.cpp`/`InventoryActionController.cpp`, where
  the item rendered invisibly *behind* the dialog's own opaque `#panel` background via the normal
  `I3DRenderObj`/`C3DRenderMng` path) and **both attempts crashed on dialog dismiss**, so this
  direction is NOT proven working — do not copy it as a pattern yet. What was tried: adding
  `RenderItem3DOnTop()` (a manually-invoked draw with its own camera/projection GL setup, called
  from Winmain.cpp's `SetPostRmlUiCallback` — the same seam `CMsgWin`/`CCharMakeWin`/`CLoginWin` use
  for native text overlays, just the first time asked to carry live 3D content). The first crash's
  cause was found and fixed at the renderer level: `MuRendererSDLGpu.cpp`'s post-RmlUi replay pass
  re-staged vertex data only (the seam's original callers were all 2D quads/text), so a skinned 3D
  draw recorded there left its bone-matrix buffer unstaged/undersized while `ReplayDrawCommand`'s
  `boneDataReady` guard stayed stale-true from the main pass — an out-of-bounds GPU read. Fixed by
  reusing `StageDeferredGpuData()` (the same function the main pass already calls, re-stages
  vertex/bone/strip-index/texture data together, returns a correct `boneDataReady`) instead of a
  hand-rolled vertex-only re-stage; this fix is real, confirmed (restored both visibility and
  position), and stays regardless of what happens with `item3D`. The dismiss-time crash persisted
  anyway — a second, still-unidentified bug in this same post-RmlUi seam. Enabling SDL_GPU
  validation (Debug config) to localize it hit a *different*, pre-existing validation failure at
  startup unrelated to this feature, blocking that diagnostic route without a separate investigation.
  `CGenericConfirmDialog` was reverted back to the plain `I3DRenderObj`/`Render3D()` path (stable,
  but the item still rendered behind the panel) rather than ship a dialog that crashes on Cancel.

  **Fixed for real, 2026-09-14, via the *other* proven direction**: the fg/bg RmlUi document split
  `CMainFrameWindow`/the inventory-family windows already use for their own live 3D icons. Rather
  than move the 3D draw to a new seam, this moves the *panel's own background art* earlier: a new
  `generic_confirm_dialog_bg.rml`/`.rcss` per theme carries what used to be `#panel`'s own paint
  (modern's gradient/shell-edge/groove/content-well recipe, legacy's sprite composite). The original
  foreground document is now paint-less where the background used to be, so once the background
  document has painted the panel earlier in the same frame, the foreground document (still
  composited last, as always) has nothing left to cover the item with. This needed **zero changes**
  to `Render3D()`, `PanelTranslateCorrection()`, `I3DRenderObj` registration, or the post-RmlUi seam
  — only to where the panel's background paints from, and, as it turned out, *when*.

  The first cut loaded the new bg doc into the SHARED `RmlUiRuntime::GetBackgroundContext()` (the
  same one `CNPCShop`/every inventory-family window's own bg doc uses), painted by the existing
  `RenderBackgroundLayer()` hook (fired once, globally, before the very first visible window/camera
  each frame). That fixed the standalone case, but broke the moment the dialog opened over another
  bg-doc window with native foreground content: `RenderBackgroundLayer()` renders every currently-
  visible bg doc together, once, strictly before EVERY window's own 2D `Render()` this frame — not
  just this dialog's — so `CNPCShop`'s own inventory-slot icons (drawn later, in its own `Render()`)
  always painted over both bg docs regardless of their relative order within that shared context,
  bleeding through the dialog's panel wherever the two windows geometrically overlapped. That's
  precisely the case `item3D` matters most for (sell-to-shop/gamble-buy confirms are almost always
  shown over an open shop window). `ElementDocument::PullToFront()`, reasserted every frame, was not
  a fix — it only reorders documents *within* one shared context; it cannot make that context's one
  global render pass happen *after* another window's own `Render()`.

  **Actually fixed** by giving the dialog's own bg doc a dedicated THIRD context
  (`RmlUiRuntime::GetDialogBackgroundContext()`), painted by a separately-guarded
  `RmlUiRuntime::RenderDialogBackgroundLayer()` that `CManager::Render()` (`WindowManager.cpp`)
  fires at a different point than `RenderBackgroundLayer()`: right before the first visible object
  whose `GetLayerDepth()` reaches the shared 3D camera's own z-order (`INFORMATION_CAMERA_Z_ORDER`,
  `Window3DRenderMng.h` — the same z-order `item3D` itself renders through). `CManager::Render()`
  sorts every registered object by `GetLayerDepth()` (`CNPCShop` = 2.5f, the shared 3D camera =
  10.9f, this dialog's own 2D `Render()` = 60.0f) and calls each one's `Render()` in that order, so
  "right before the object at/past 10.9f" is guaranteed to land strictly after every ordinary
  window's own `Render()` this frame and strictly before `item3D` draws. This is deliberately
  triggered from `CManager::Render()`'s own per-object loop, NOT from inside `Render3D()` itself —
  `C3DCamera::Render()` pushes a legacy GL_PROJECTION/GL_MODELVIEW matrix stack and enables depth
  test/mask before looping over every registered object's `Render3D()`, and recording an RmlUi
  render pass from inside that block would be exactly the kind of mid-frame GPU-state interleaving
  that crashed the `SetPostRmlUiCallback` attempts above; `CManager::Render()`'s own loop, before
  any `(*vi)->Render()` call, is the same safe, pre-matrix-stack position `RenderBackgroundLayer()`
  itself already uses — so this answers this entry's own opening question (multiple contexts,
  interleaved at the right point in the frame) in the affirmative, at least for this one case.

  Also simpler than either reference implementation regardless: this dialog centers via plain CSS
  (`.center-both`), never a per-frame C++-computed position, so its background document needs no
  `RmlModelBinder`/position-sync code at all, unlike `CMainFrameWindow`'s/`CNPCShop`'s own (each
  anchored to an adjustable HUD-band/inventory-window position, so each needs a small model).
