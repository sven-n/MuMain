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

The shared file stays theme-neutral: theme-specific class vocabulary (`modern-frame`,
`modern-panel`, …) belongs in that theme's fork, never in the shared `.rml` (`login.rml`,
`msg_win.rml` and `remember_password_prompt.rml` are modern's earliest forks for this reason).

**Forking safely needs a check**: whichever theme's RML a window loads, the
C++ side still expects the exact same ids/`data-model` bindings/event-callback names to exist.
`tools/check_rml_rcss_drift.py` (a sibling to `check_rml_rcss_syntax.py`, wired into the same build
step) diffs the ids/bindings a window's C++ actually references against every theme's copy of that
window's RML and fails the build if none of them provide something the code needs — otherwise a
missing or renamed id would fail **completely silently** (a dead button, not a build error or even
a log line).

## How theme resolution works

A theme is identified by **folder name**, not a closed C++ enum, specifically so adding one
(including a fully programmatic, sprite-free one) is normally a zero-source-change,
drop-a-folder operation.

`config.ini`'s `[UI] RmlTheme=<name>` selects the active theme, read once at startup by
`GameConfig::GetRmlTheme()` and cached by `UI::RmlBridge::GetActiveThemeName()`. That cache is
also live-mutable via `UI::RmlBridge::SetActiveThemeName()` — see
["Switching themes without relaunching"](#switching-themes-without-relaunching) below. Loading a
window goes through its `ThemedView`, which hands `"Data/Interface/RmlUi/login.rml"` to
`UI::RmlBridge::ThemedDocumentLoader::Load()`:

1. It reads the active theme's `login.rml` override if present, otherwise the shared markup.
2. It builds a synthetic source URL, `Data/Interface/RmlUi/themes/<name>/login.rml` — this path
   need not exist on disk.
3. It calls `Rml::Context::LoadDocumentFromMemory(text, syntheticSourceUrl)`.
4. RmlUi resolves the document's `<link href="login.rcss"/>` relative to that synthetic URL,
   landing on `Data/Interface/RmlUi/themes/<name>/login.rcss` — which *does* need to exist on
   disk. The game's file interface expands that sheet's `token(name)` markers while preserving
   the external link and its path for RmlUi's stylesheet cache and relative assets.

This is confirmed against RmlUi's own source: `Context::LoadDocumentFromMemory` wraps the string
in a `StreamMemory` and calls `SetSourceURL()` on it before parsing; `XMLNodeHandlerHead.cpp`'s
`MakeExternalResource()` resolves `<link href>` via `AbsolutePath(path, parser->GetSourceURL())`.

Every migrated window renders its chrome through RmlUi, in every theme — no native sprite path
draws underneath it. A "legacy-look" theme reproduces the
original art by pointing its own RCSS decorators at the same image files the old sprites used;
a "modern" theme uses flat colors/vector shapes instead.

## Forking a theme's RML: the mechanism, and today's real examples

`UI::RmlBridge::ThemedDocumentLoader::Load()` ([`RmlTheme.cpp`](../../src/source/UI/RmlBridge/RmlTheme.cpp))
looks for `themes/<theme>/<name>.rml` first, falling back to the shared `<name>.rml` when no such
file exists (for a window with no fork, one extra failed `ifstream` open). `modern` forks a
few dozen documents; `themes/modern/*.rml` is the live list. Most of these exist so `modern`'s own `.modern-frame`/`.modern-frame-crimson`/`.title-glow` class vocabulary
doesn't have to live in the shared, theme-neutral file (the Core Principle section above).

`main_frame` was forked for a different reason and has since been merged back, which makes it the
worked example of *not* forking. Its two copies differed in structure: legacy kept the original
bottom button row inside `#bars`, while modern moved its buttons to `main_frame_top.rml` and added
flat backing elements legacy has no use for. Hand-syncing two copies of the HUD made every added,
moved or removed part a two-file edit. The shared `main_frame.rml` now carries every part either
theme uses, and each theme's `main_frame.rcss` hides the others (`display: none`). Where the two
themes bind different values to the same spot, the markup carries both variants and each theme
shows one: a gauge's readout as `.value-current` or `.value-full`, its hint as `.native-hint`
(native text size, counter-scaled through bound inline styles a theme's RCSS cannot override) or
`.scaled-hint`. The document is also split into parts (`.hud-part`: the strip backing, each
gauge group, EXP, buttons, item hotkeys, skill row, skill list), each placed by its own rule in
the theme's `main_frame.rcss` with its contents laid out from its own corner, so a theme moves
or hides a part without touching the others. The skill list's grid fans out from its part's box,
the first cell's position, so it docks wherever the theme puts that box. An early attempt at CSS-only hiding was abandoned because the old row "leaked"
through, but the record of it describes the whole stylesheet intermittently failing to apply, not
`display: none` failing.

**Fork the RML only when a theme needs markup the shared file cannot reasonably carry** — prefer
adding the element to the shared file and letting the other theme hide it. A fork's
ids/classes/bindings have to be kept in step with the shared file by hand;
`tools/check_rml_rcss_drift.py` catches a missing binding, not a missing element.

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
`UI::RmlBridge::ReloadAllThemedDocuments()` to invoke every registered theme-reload callback.
Every themed document belongs to a `UI::RmlBridge::ThemedView` (`component-catalog.md`'s Theming
section), which registers for theme switches the first time it builds. On a switch each view keeps
its model's values, unloads its documents, loads them again from the active theme and shows again
the ones that were visible; views rebuild from the bottom of their context's stack up, so the
stacking survives. A window that was never opened has nothing to rebuild and picks up the active
theme when it first builds. The free-function modules (the tooltip, notices, the remember-password
prompt) hold a namespace-scope view and work the same way.

**Session-only**: this does not write to `config.ini` — `GameConfig::SetRmlTheme()` only updates
the in-memory value, so a relaunch still picks up whatever `config.ini` says. Use it for quickly
A/B-ing themes; edit `config.ini` (previous section) for a change that should survive a restart.

**Two themes are built, and two is the intended set: `legacy` and `modern`.** The mechanism stays
theme-name-agnostic (a theme is a folder, see above) so a *user-authored* theme remains possible,
but no third first-party theme is planned — the project owner ruled one out. `architecture-
principles.md` §25/§28 envisions a Custom/Test theme specifically to expose accidental coupling
between a component and one visual design; that concern is real, but it is met by `modern`'s own
structural divergence plus the drift checker rather than by a third theme. Keep `legacy` and `modern` updated together for any window content
change — don't let one lag.

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

## Design tokens: `themes/<theme>/tokens.ini`

Both built-in themes have a `tokens.ini` file (`[Tokens]` section, plain `name=value` lines) that a theme's own `.rcss`
files reference via a `token(name)` marker instead of repeating a literal value everywhere it's
used. RmlUi follows each document's ordinary external `<link>` and loads the themed `.rcss`
through the game's file interface. That interface resolves `token(name)` from the `tokens.ini`
beside the stylesheet before RmlUi parses and caches it. This vendored RmlUi build has no
`var()`/custom-property mechanism, so `token(name)` is plain text substitution, not a CSS
feature. A new theme needs only its own files; it does not need a C++ change. The stylesheet
cache is cleared when a theme is selected again, so editing theme files and reselecting that
theme refreshes their values.

If a stylesheet refers to a token absent from that theme's `[Tokens]` section, the client logs
the token name and stylesheet path in `MuError.log`. The missing value still expands to an empty
string. Add the key to `tokens.ini` and reselect the theme to reload the stylesheet.

**What a token is for**: a *reusable, theme-level semantic choice* — the standard body text color,
a shared muted/secondary text tier, the common tooltip backing, a shared accent/highlight, a
warning/danger color, shared scrollbar track/thumb colors, a shared corner-radius. The test isn't
"does this literal appear more than once" — it's "does changing this value represent one coherent
design decision a theme author would actually want to make in one place." `legacy`'s own
`tokens.ini` documents, token by token, which real recurring selector(s) motivated it; a token with
no real call site backing it (invented for taxonomy-completeness alone) doesn't belong.

**What should stay ordinary RCSS, not a token**:
- **One-off decorative colors** — a single window's own specific accent choice with no cross-window
  or cross-selector reuse. Most of a theme's literal colors are legitimately this; not every color
  needs a lever.
- **Gameplay colours** — gauge gradient stops around a base fill, the cooldown wipe, a status
  colour such as poisoned HP or server load, and white-outlined text over the 3D world. They mean
  something in the game, not in the theme.
- **Content-driven/asset-driven palettes** — `tooltip.rcss`'s `.tt-blue`/`.tt-red`/`.tt-yellow`/etc.
  rich-text colors (and their `.tt-hl-*` background-highlight pairs) are the exact RGB values
  `RenderTipTextList()`'s native `TEXT_COLOR_*` switch (`ZzzInventory.cpp`) already used — they
  identify a *content type* (an item's rarity tier, a warning message), not a theme aesthetic
  choice, and both `legacy`'s and `modern`'s own copies of this file deliberately leave them as
  literals with a comment saying so. A ninepatch/gradient decorator recipe tightly bound to one
  specific sprite composition (`server_select.rcss`'s own button-tint overlays) is the same
  category — asset-specific, not a theme palette lever.
- **Structural geometry** — panel width/height, row height, pixel offsets, icon pitch, spritesheet
  rects, native-companion-synchronized positions (anything documented in
  [`tracked-deferrals.md`](tracked-deferrals.md) as a themeability-coupling gap). A value becoming
  `token(foo)` does not make duplicated geometry architecturally themeable — that requires actually
  deriving the position live (or, where that's currently impractical, an explicit pinned
  cross-reference comment on both sides). Tokens are a color/typography/radius mechanism, not a substitute
  for the geometry-decoupling work tracked separately.

Each theme's own token meanings: `themes/modern/README.md`; `themes/legacy/tokens.ini`'s comments.

**Cross-theme naming**: reuse a `modern` token's NAME for a `legacy` token when the semantic ROLE
genuinely matches (`text-primary`, `font-body`, `radius-sm` all exist in both, with each theme's
own appropriate value — `legacy`'s `text-primary` is `#ffffff`, `modern`'s is a warm off-white).
Don't force a shared name onto two concepts that don't actually match, and don't rename an existing
token in one theme purely for cosmetic cross-theme symmetry — a real naming inconsistency is worth
fixing, a theme's own genuinely distinct concept (an accent family the other theme doesn't have)
just gets its own name.

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
- **Where windows and HUD parts go** — the theme's `workspace.rml`/`workspace.rcss`: regions and
  one slot per window or HUD part, laid out in RCSS; the game places each open window on its slot
  ([window-placement.md](window-placement.md)). What a theme can do there:
  - move a slot to another region, or reorder slots, to change the arrangement — including the
    main HUD, chat, header corners and event HUDs;
  - mark a shell region `data-participation="reserve"` (windows avoid it) or `overlay`;
  - list in a slot's `data-closes` the windows that close for lack of room when it opens;
  - give a slot `data-fit="fill"` and a size in `%`, for windows that support it;
  - cap a dock region with `max-height: 100%` so its windows scale down to fit.

  Worked examples (split docks, centred inventory, a side bar, chat on the right, the HUD at the
  top) are in window-placement.md's "Theme recipes". Windows without a slot keep the rule below.
- **The panel's own position on screen** for a window without a slot — its own RCSS anchoring
  (`.center-both`, `position: absolute; right: 0; bottom: 0;`, …), or, for the few that follow
  the pointer or a target (help, item explanations, quick command, tooltips), C++.
- **Draggability** — `UI::RmlBridge::MakeDraggable()`, wired per window in C++; RCSS has no
  "make this draggable" property. Which windows drag is decided in
  [window-placement.md](window-placement.md) section 8.

## Known limitations

- **Live hot-swap is session-only.** `$theme <name>` (see
  ["Switching themes without relaunching"](#switching-themes-without-relaunching) above) rebuilds
  every open window's document in place, but doesn't persist to `config.ini` — a relaunch still
  uses whatever's saved there. The model survives the rebuild, so a window needs no resync of its
  own; what it sized or placed from the old theme's metrics goes in its view's `afterReload`.
- **Custom theme images require the engine's proprietary OZT/OZJ format, not plain PNG/JPG** —
  see [Bringing your own images to a theme](#bringing-your-own-images-to-a-theme) above. No
  converter tool exists in this repo today, and a missing/wrong-format image fails silently.
- **Every themed document belongs to a `ThemedView`**, so `$theme` covers all of them; a new
  window declares one.
- **Theme identity must never drive C++ branching** — `architecture-principles.md` §30. Where a
  theme must change C++ behaviour, it declares a capability in `theme.ini` (`NativeTextSize` is the
  one in use). Prefer removing the need: the main frame's `ProvidesOwnIconChrome` existed only so
  C++ could skip drawing legacy chrome for the modern theme, and went away once that chrome was
  RCSS in the legacy theme's own stylesheet.
- **RmlUi renders last in the frame**, so native content that must sit *in* a window's stacking
  goes into a `UI::RmlBridge::RenderTarget` the document shows: live 3D and a window's remaining
  native 2D alike, so every document is in the one RmlUi context.
