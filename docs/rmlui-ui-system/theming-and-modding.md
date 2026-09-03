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

**Known current debt, not yet cleaned up**: `login.rml`, `msg_win.rml`, and
`remember_password_prompt.rml` currently have `modern`-specific class names (`modern-frame`,
`modern-frame-accent`, `modern-panel`, etc.) hardcoded directly into what's supposed to be the
shared, theme-neutral file — e.g. `login.rml`'s `<div id="panel" class="modern-frame
modern-frame-crimson" ...>`. That's backwards: it means `legacy` is the one being constrained by
`modern`'s vocabulary, not the other way around. See `STATUS.md`'s
gap list — this needs an actual migration (strip theme-specific classes back out of the shared
file, give `modern` its own forked copy), not a docs-only fix.

**Forking safely needs a check that doesn't exist yet**: whichever theme's RML a window loads, the
C++ side still expects the exact same ids/`data-model` bindings/event-callback names to exist —
nothing currently verifies that a forked theme's copy actually satisfies that contract. A missing
or renamed id fails **completely silently** (a dead button, not a build error or even a log line)
— the same failure shape as `main_frame`'s existing two-file hand-sync burden below. A real fix
would be a script (a sibling to `check_rml_rcss_syntax.py`) that diffs the ids/bindings a window's
C++ actually references against every theme's copy of that window's RML and fails the build if one
theme is missing something the code needs. Not built — tracked in `STATUS.md`.

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

## Forking a theme's RML: the mechanism, and today's one real example

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
4. Edit `config.ini`'s `[UI]` section: `RmlTheme=<your-theme-name>`.
5. Relaunch. No rebuild needed — this is a pure data/config change.

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

- **Not yet a live in-game hot-swap.** Switching themes today means editing `config.ini` and
  relaunching. A true runtime toggle needs each migrated window to tear down and rebuild its
  `Rml::ElementDocument`/`DataModel` against the new theme — not built yet.
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
  `CCharInfoBalloonMng`, `CMsgWin`, `CMuHelperBar`, `CBuffStrip`, and `CNewUIMainFrameWindow`
  (`COptionWin` is wired the same way but not reachable in live play — see the main
  [README](README.md#coexistence-patterns)). Extending a new window to support theming is the same
  established pattern, not new design work — this list will keep growing and isn't worth
  maintaining exhaustively; grep `LoadThemedDocument(` for the live count.
- **Theme identity must never drive C++ branching** — `architecture-principles.md` §30. Two known
  violations exist today (`NewUIMainFrameWindow.cpp`'s background-fill and skill-highlight logic,
  both keyed on `GetActiveThemeName() == "modern"`), tracked as debt in `STATUS.md`, not accepted
  design — see that section for why they exist (a real render-ordering constraint) and the next
  bullet for whether that constraint is actually permanent.
- **RmlUi rendering strictly last in the frame is an integration choice, not a proven RmlUi
  requirement.** `RmlUiRuntime::Render()` fires from one fixed pre-submit callback, always after
  every legacy 2D/3D draw call for the frame — which is *why* the two violations above exist (an
  RmlUi-drawn fill would always paint over content that needs to render on top of it, since RmlUi
  composites last). Nobody has investigated whether interleaving is possible (multiple contexts,
  or a callback hook legacy content renders through at the right point in RmlUi's own z-order) —
  it may well be, and would remove the need for those two workarounds entirely rather than fixing
  them with a capability flag. Not investigated — tracked in `STATUS.md` as its own design
  question, separate from and potentially larger than the §30 fix.
