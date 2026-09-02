# Theming & Modding

How the swappable-theme system works, and how to add a new theme — including a modder-supplied
one — without touching engine source code.

## Core principle: RML is theme-agnostic, RCSS is the swappable unit

RML files describe structure/content/data-bindings only — what elements exist, what they're bound
to. They never hardcode a visual look inline. All actual appearance (colors, borders, backgrounds,
button/checkbox art) belongs in RCSS, so **a new theme is a new RCSS file (or folder of them)
reusing the same class names, not a new RML file.**

## How theme resolution works

A theme is identified by **folder name**, not a closed C++ enum, specifically so adding one
(including a fully programmatic, sprite-free one) is normally a zero-source-change,
drop-a-folder operation.

`config.ini`'s `[UI] RmlTheme=<name>` selects the active theme, read once at startup by
`GameConfig::GetRmlTheme()` and cached by `UI::RmlBridge::GetActiveThemeName()`. Loading a window
goes through `UI::RmlBridge::LoadThemedDocument(context, "Data/Interface/RmlUi/login.rml")`:

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

## Exception: per-theme RML override

The Core Principle above is a hard default, not an absolute — one window breaks it today, and the
criteria for when that's legitimate are worth stating explicitly so it doesn't become a habit.

`UI::RmlBridge::LoadThemedDocument()` ([`RmlTheme.cpp`](../../src/source/UI/RmlBridge/RmlTheme.cpp))
looks for `themes/<theme>/<name>.rml` first, falling back to the shared `<name>.rml` when no such
file exists. Every window but `main_frame` has no per-theme override, so this is a no-op for them
(one extra failed `ifstream` open). `main_frame` has two independently-maintained files —
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

**Use this escape hatch only when the two themes' content genuinely differs in DOM structure or
position, not merely in visibility or style** — "this element doesn't exist in the other theme's
layout at all" (main_frame's button row), not "this element is hidden/styled differently in the
other theme" (ordinary RCSS `.hidden`/selector differences, the normal case, no override needed).
Two real costs come with it, both currently paid manually for `main_frame`: the two files' shared
ids/classes/bindings have to be hand-kept in sync (each file's own header comment says so
explicitly — check the other file's comment before editing either one), and any future window
tempted to reach for this needs the same structural-difference test applied first, not just "it's
easier than fighting the CSS."

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
4. Edit `config.ini`'s `[UI]` section: `RmlTheme=<your-theme-name>`.
5. Relaunch. No rebuild needed — this is a pure data/config change.

**Only two themes are currently maintained and shipped: `legacy` and `modern`.** Keep both
updated together for any window content change — don't let one lag.

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

- **Not yet a live in-game hot-swap.** Switching themes today means editing `config.ini` and
  relaunching. A true runtime toggle needs each migrated window to tear down and rebuild its
  `Rml::ElementDocument`/`DataModel` against the new theme — not built yet.
- **Custom theme images require the engine's proprietary OZT/OZJ format, not plain PNG/JPG** —
  see [Bringing your own images to a theme](#bringing-your-own-images-to-a-theme) above. No
  converter tool exists in this repo today, and a missing/wrong-format image fails silently.
- **No scaling is wired up, and a hybrid window's on-screen position isn't theme-controlled** —
  see [Coordinates, scaling, and positioning](#coordinates-scaling-and-positioning--what-a-theme-actually-controls)
  above.
- **Only windows routed through `LoadThemedDocument()` participate in theme selection.** Today
  that's `CLoginWin`, `CLoginMainWin`, `CSysMenuWin`, and `RememberPasswordPrompt` (`COptionWin`
  is wired the same way but not reachable in live play — see the main
  [README](README.md#coexistence-patterns)). Extending a new window to support theming is the
  same established pattern, not new design work.
