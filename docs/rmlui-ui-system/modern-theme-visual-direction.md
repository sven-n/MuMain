# Modern Theme Visual Direction

Amends [`architecture-principles.md`](architecture-principles.md) §15 ("Themes Are Presentation
Packages") and §21 ("Design Tokens / Shared Theme Variables"), scoped to **the `modern` theme
only**. The `legacy` theme is explicitly out of scope and must not change as a result of this
document — it stays a pixel-faithful reproduction of the original game's look, governed by
[`legacy-theme-modernization.md`](legacy-theme-modernization.md) instead.

**Second generation, 2026-09-10**: this document previously described a cool-steel/blue-gray
palette (2026-09-09), fully superseded by the current blackened-iron/dark-forged-metal direction —
every token was renamed as well as revalued as part of that switch (mechanically, via
`tools/migrate_rcss_tokens.py`; confirmed 2026-09-19, no old token name survives anywhere in
`themes/modern/*.rcss`/`.ini`). This document and `tokens.ini` are the sole source of truth for
current token names/values — there is no old→new mapping still worth keeping.

## Direction

`modern` should read as blackened iron / dark forged metal — a carved-window action-RPG interface,
not a generic light dashboard, not a flat web-app aesthetic, and not the previous generation's
cooler blue-gray steel either. Concretely:

- Dark iron surfaces (`surface-deep`/`-recessed`/`-panel`/`-raised`, always near-black, never a
  bright neutral) layered by shade rather than heavy borders, to establish hierarchy. A dialog's
  own decorative frame/panel material (the `.modern-frame-crimson`/`.modern-panel-crimson`
  family — `login`, `remember_password_prompt`, `msg_win`, `my_quest_info`, `sys_menu`) is a
  marginally warmer, slightly darker iron than plain `.modern-frame` — see
  [Borders and frames](#borders-and-frames) below.
- **Only title/name/label text (`text-warm`) and the checkbox's checked state stay warm/gold** —
  general body/description/stat text is a neutral gray scale (`text-primary`/`-secondary`/`-muted`),
  not warm; this isn't a blanket "remove all warmth" pass, warmth is scoped to titles/emphasis.
- Three accent families, used for meaning/emphasis, not as a base material for structural
  surfaces — see [Accent colors](#accent-colors): `accent-steel` for general interactive chrome
  (hover/focused borders, dividers), `accent-crimson` for primary/hero emphasis (a window's one
  confirm-style action, a title banner), and `accent-gold` reserved for inline text emphasis,
  item/skill-slot selected/active highlights, and the checkbox's checked state — never for general
  window/dialog chrome otherwise. The three aren't interchangeable.
- Subtle framing (thin metallic edges, bevel highlights) reserved for major panels; small
  HUD/tooltip elements stay simple. Corners are sharp, not soft-rounded — a carved iron plate reads
  edged, not like a rounded modern web panel (see [Radius](#radius)).
- High contrast only where interaction or readability actually needs it — most of the surface
  should stay quiet. A crimson primary button or banner is a deliberate exception to this, not a
  blanket license to run every surface loud — a window with several equal-weight buttons (no single
  primary action) has no reason to use `accent-crimson` on any of them; see `sys_menu.rcss`'s plain
  `.btn` for that case. **Gold must never become generic button chrome** — every plain `.btn`
  (including its hover state) stays neutral iron/steel; gold only ever marks a selected/checked
  state or a title.
- Visually consistent across HUD, dialogs, tooltips, buttons, and panels — a shared vocabulary
  applied everywhere via `base.rcss`'s shared primitives, not a one-off restyle per window (see
  [Shared primitives](#shared-primitives-in-basercss)).
- Color/composition is informed by a reference visual study, but `modern` is a genuine
  from-scratch RCSS implementation, never a reuse of `legacy`'s own literal sprite pixels or assets,
  and never a literal copy of the reference's browser-only CSS constructs — see
  [Engine-constraint translation guide](#engine-constraint-translation-guide).

## Design tokens

**This vendored RmlUi build has no `var()`/custom-property mechanism** (confirmed against
`StyleSheetSpecification.cpp`'s property registration table). `UI::RmlBridge::LoadThemedDocument()`
(`RmlTheme.cpp`) resolves a `token(name)` marker in an `.rcss` file against
`themes/<theme>/tokens.ini`'s `[Tokens]` section before RmlUi ever sees the text, reusing the fact
that RmlUi's own `XMLNodeHandlerHead` treats an inline `<style>` block in `<head>` identically to an
external `<link type="text/rcss">` for cascade purposes. Content-driven, not theme-name-driven, so
`legacy` (no token layer, still theme-specific by design) is unaffected. `themes/modern/tokens.ini`
is the single source of truth for values; this document is the source of truth for what each token
*means*.

**Current engine capability (see `engine-findings.md` for the authoritative, up-to-date list —
this has changed direction before, don't trust a stale copy of it): `box-shadow` genuinely renders
now** (inset/outset, real blur, comma-separated multiple shadows — fixed as of the 2026-09-10
renderer upgrade, in active use throughout `base.rcss`/`login.rcss`/`sys_menu.rcss`/
`server_select.rcss`). **`filter`/`backdrop-filter`/`mask-image` remain unimplemented** —
`filter: brightness()`/`contrast()` are the one confirmed-working exception
(`my_inventory.rcss`'s `.inv-btn:hover`). Bevel/depth cues predating the box-shadow fix still use
plain per-side `border-*-color` bevels plus `decorator: linear-gradient(...)` — see `base.rcss`'s
`.modern-frame`/`.modern-panel`/`.modern-inset` — real `box-shadow` is now also a valid choice for
new work, not just the bevel technique.

**`border-inner` is a genuinely new token**, not a renamed one — reserved specifically for a
recessed content-well edge (`.modern-panel`'s own border, for instance), distinct from
`border-frame`'s general/raised-surface role. It exists as an available primitive; it hasn't been
forced onto every screen, only where a real recessed well needed one.

### Surfaces

| Token | Value | Use |
|---|---|---|
| `surface-deep` | `#060706` | Full-screen dim backdrop (`#backdrop`) |
| `surface-recessed` | `#0a0c0b` | Base/sunken panel background (`.modern-panel`) |
| `surface-panel` | `#111310` | Default panel body (`.modern-frame`'s own base tone) |
| `surface-raised` | `#20221d` | Raised/header chrome — available for a future header-band treatment, not yet forced onto an existing window |
| `surface-control` | `rgba(255, 255, 255, 10)` | Interactive control, resting (a faint white wash over whatever surface sits behind it — icon buttons, `.mu-btn`) |
| `surface-tooltip` | `rgba(6, 7, 6, 247)` | Tooltip/floating-label background — used by every tooltip in the theme. Use this for any new tooltip rather than picking a new near-black. |

### Metal / frame

The structural iron material itself, distinct from `accent-steel` (that's interactive chrome, not
the base material).

| Token | Value | Use |
|---|---|---|
| `metal-dark` | `#13140f` | Deep iron — a button's own dark base tone |
| `metal-mid` | `#46463b` | Mid iron — a derived-shade anchor (also `border-frame`'s exact value, not a coincidence: both express "the visible iron edge") |
| `metal-edge` | `#5c6058` | Bevel-light catch on a raised iron surface |
| `metal-highlight` | `#96998e` | Brighter bevel catch — a stronger highlight than `metal-edge` |
| `metal-warm-edge` | `#6b6045` | Sparingly-used ornamental warm-brass tone (the "real dialog" `-crimson` frame variant's top highlight, `login.rcss`'s divider) — not a general-purpose border color |

### Accent colors

Three accent families, not interchangeable, each with a distinct job:

- **`accent-steel`** — general interactive chrome: hover, focused-border, dividers. Never a
  structural base material.
- **`accent-crimson`** — a window's one *primary/hero* element: a single confirm-style action
  button, or a title banner. Never applied to more than one thing per window, and never to a plain
  list of equal-weight actions (see Direction's `sys_menu.rcss` example above).
- **`accent-gold`** — scoped to inline text emphasis, item/skill-slot selected/active/current
  highlights, and the checkbox's checked state. Never used for general window/dialog chrome or
  button hover — see Direction's explicit "gold must never become generic button chrome" rule.

Crimson is a real gradient in practice (bright top stop fading through the reference's own
crimson-700/-900 shades to a dark bottom stop, plus a bevel border), not a flat fill — see
`base.rcss`'s `.btn-ok` for the actual stop sequence.

| Token | Value | Use |
|---|---|---|
| `accent-steel` | `#747b74` | General interactive chrome |
| `accent-steel-hover` | `#a5aaa0` | Hover-bright state for the same chrome |
| `accent-crimson` | `#7d190f` | Primary/hero accent |
| `accent-crimson-hover` | `#c34a2d` | Hover/bright state for the same primary element |
| `accent-gold` | `#c49a3c` | Semantic highlight |
| `accent-gold-hover` | `#e2c16d` | Brighter variant of the same semantic highlight |

### Semantic colors

| Token | Value | Meaning |
|---|---|---|
| `semantic-success` | `#8eb06b` | Positive / success / healthy |
| `semantic-warning` | `#c99a3e` | Warning / caution |
| `semantic-danger` | `#a22c1e` | Danger / destructive / critical |
| `semantic-rare` | `#9a6bc0` | Rare / special / high-value (no reference analog, kept from the prior generation) |
| `semantic-neutral` | `#6a6a63` | Neutral / disabled / secondary |

### Text

Only `text-warm` is warm; `text-primary`/`-secondary`/`-muted` are a neutral gray scale — see
Direction's narrowed-warmth rule above.

| Token | Value | Use |
|---|---|---|
| `text-primary` | `#d6d2c6` | Default readable body/general text |
| `text-secondary` | `#aaa79d` | Metadata, secondary text |
| `text-muted` | `#77766f` | Dimmer still — the third, quietest tier |
| `text-warm` | `#d6ad45` | Titles/headers/names/labels — the one place text stays warm |
| `text-disabled` | `#4a4a44` | Disabled control text |

### Borders

| Token | Value | Use |
|---|---|---|
| `border-frame` | `#46463b` | The general-purpose visible iron edge — window/dialog outer frames, buttons, checkboxes, tooltips, HUD panels. The reference uses close to this one tone almost everywhere a border is meant to actually be seen; this is the default/workhorse border token. |
| `border-inner` | `#292a23` | A recessed content-well edge specifically (`.modern-panel`'s own border) — distinct from `border-frame`'s raised/visible role, not yet forced onto every screen. |
| `border-recessed` | `#22231d` | A barely-visible hairline divider |
| `border-hover` | `#776646` | Explicit hover-state border (`.btn:hover`) |
| `border-selected` | `#b48a32` | Explicit selected-state border (`.slot--selected`, `.skill-cell.selected`) |

### HUD resource colors

Previously literal, un-tokenized hex scattered across `main_frame.rcss`'s gauge fills — promoted
here so a HUD gauge's color has a name like everything else. Deliberately distinct from the
`accent-*` family: these mean "this specific stat," not general UI emphasis, and stay out of the
"deliberately not tokenized" gameplay-status list below only in the sense that the *base* color now
has a name — the actual per-instance gradient stops around it are still local literals, same
convention as every other gradient in this theme.

| Token | Value | Use |
|---|---|---|
| `resource-hp` | `#94150f` | HP gauge fill |
| `resource-mp` | `#224c9d` | MP gauge fill |
| `resource-sd` | `#d09b35` | SD gauge fill |
| `resource-ag` | `#af8630` | AG gauge fill |

### Radius

| Token | Value |
|---|---|
| `radius-sm` | `1px` |
| `radius-md` | `2px` |

Shrunk from the prior generation's `3`/`4` — a carved-iron plate reads sharp-edged, not
soft-rounded. Several windows' own hardcoded local radius literals (`login.rcss`'s `#panel`/
`.frame-accent`/`.login-content`, `sys_menu.rcss`'s `#panel`, `remember_password_prompt.rcss`'s
`#panel`/`.frame-accent`, `char_info_balloon.rcss`'s `.balloon-bg`) were shrunk by hand to match at
the same time, since they predate the token and don't reference it directly.

### Typography — open item, not decided here

Every modern-theme `.rcss` file still repeats `font-family: "Liberation Sans";` as a literal. This
document defines the *token mechanism* (`font-title`/`font-body`, name it once, reference the name
in comments); it does not pick a different display font. Sourcing a distinct "premium" typeface is
an asset decision outside a docs pass.

## Borders and frames

Prefer `border-frame`/`border-recessed` over inventing a new one-off border color. Reserve
`border-selected`/`border-hover` for their specific interaction states, not general chrome.

### The `-crimson`-named frame/panel variant

`base.rcss`'s `.modern-frame-crimson`/`.modern-panel-crimson`/`.modern-frame-accent-crimson` — the
palette variant `login`, `remember_password_prompt`, `msg_win`, `my_quest_info`, and `sys_menu`
apply alongside the generic `.modern-frame`/`.modern-panel`/`.modern-frame-accent` primitives —
isn't literally crimson (that's `accent-crimson`, reserved for a primary button/banner). It's a
marginally warmer, slightly darker iron than plain `.modern-frame`, distinguishing the "real
dialog" family from a plainer frame use. The classes keep their `-crimson` name even though the
frame material itself carries no red — a real cleanup worth doing eventually (touches every
consuming `.rml`'s `class` attribute plus `base.rcss`'s own selectors) but not bundled into the
color/token migration itself.

### Default look for every dialog/window: `.modern-frame`/`.modern-frame-crimson` + `.title-glow`

Unless a window has a specific, documented reason not to (see the exceptions below), every real
dialog/window applies `.modern-frame modern-frame-crimson` to its outer `#panel` and, if it has an
actual title/name label, `base.rcss`'s `.title-glow` primitive (a soft radial warm-cream-to-crimson
glow) as a sibling positioned before that title. This is the default, not an opt-in. Current
adopters: `login`, `msg_win`, `remember_password_prompt` (frame only, see below), `my_inventory`,
`my_quest_info` (forked specifically to pick this up — it had no per-theme override before), and
`sys_menu` (forked the same pass, frame only, see below).

**Real, documented exceptions — don't copy these as an excuse to skip the default elsewhere:**
- **`remember_password_prompt`** gets the frame but deliberately no glow: its title color IS the
  security warning being confirmed (`semantic-warning`, not decorative) — a crimson banner behind a
  warning-colored title would compete with that meaning.
- **`sys_menu`** gets the frame but no glow: it has no title/name label at all (four stacked
  buttons, nothing to glow behind).
- **`char_make`, `char_sel_main`, `server_select`, `credit_win`**: `#panel` stays deliberately
  invisible/backgroundless in all four — `char_make`'s frames a live 3D character-preview viewport
  behind it, `char_sel_main` is a borderless button bar over the 3D scene, `server_select` sits over
  a video background (matching the real legacy screen, not a redesign choice), and `credit_win`'s
  `#panel` is a full-screen scrim, not a bordered window. None of these are "windows" in the
  bordered-dialog sense this convention is for.
- **HUD overlays** (`main_frame`, `mu_helper_bar`, `buff_strip`) never get `accent-crimson` — a HUD
  element is not a dialog/window.

## Shared primitives in `base.rcss`

Beyond the frame/panel/button/checkbox primitives above, this generation's migration added or
consolidated:

- **`.btn-icon`/`.btn-icon-label`/`.btn-icon.disabled`** — a shared icon-only button (a monogram/
  glyph label instead of real art), consolidating what used to be two independent, near-identical
  copies in `login_main.rcss` and `char_sel_main.rcss`. Each consuming window keeps its own
  positional class/override for placement; the look itself is shared now.
- **`.tooltip__name`/`.tooltip__type`/`.tooltip__rule`/`.tooltip__stat`/`.tooltip__bonus`** — a
  structured item-tooltip primitive (name/type/divider/stat-rows/bonus-line), alongside the
  existing plain `.tooltip` shell every window's own single-line discoverability tooltip already
  used. Not consumed by any window yet — nothing in this codebase renders a real item tooltip
  through RmlUi today — laid down as an available primitive so a future one doesn't get invented ad
  hoc. Every existing plain tooltip's border was also standardized onto `border-frame` (previously
  split, unexplained, between `border-metal`/`accent-steel-bright` across different files).
- **`.slot`/`.slot--filled`/`.slot--selected`** — generalized from `main_frame.rcss`'s
  `.slot-frame`/`.slot-fill`/`.selected` (the only real "slot"-shaped RmlUi element in this theme
  today — the actual equipment/inventory grid stays fully native, see `my_inventory.rml`'s own
  comment) so any future slot-based UI has a ready-made look. `main_frame.rcss` keeps its own class
  names (its ids are bound from C++/RML; renaming them is a separate, unrelated risk) but is
  retinted to match these exactly.

## Component states

Every interactive element expresses, through this same token set: normal (`surface-control` resting
wash or `metal-dark`/`border-frame` for a button), hover (`border-hover`/`accent-steel-hover`),
pressed (a `accent-crimson`-tinted border, `.btn:active`'s convention), disabled (`text-disabled`/
`semantic-neutral` + reduced opacity). **Selected/active and the checkbox's checked state use gold,
not steel** (`border-selected`/`accent-gold-hover`, `.slot--selected`'s convention; `accent-gold`,
`.checkbox-box.checked`'s convention) — both are semantic "this specific thing is the current
one/is checked" highlights, not general interactive chrome. State differences come from
brightness/contrast/background/border changes, not new shapes or layout shifts.

## Panels, HUD, and tooltips

- **Panels/windows**: `surface-recessed`/`surface-panel` layering for header-vs-body distinction,
  `border-frame` at the outermost frame. `box-shadow` is now available (see the engine-constraint
  table above) — `login.rcss` is the first consumer (frame bevel rings, well vignette, drop
  shadows); not yet propagated to the rest of this list.
- **HUD**: stays legible during combat — favor `surface-deep`/`surface-recessed`-level translucency
  over opaque fills, `accent-gold` used sparingly for the specific things that matter (current
  selection, active cooldown state), never as general decoration. Layout/responsiveness stays
  governed by `layout-and-scaling.md` — this document only supplies color/border tokens, not a new
  layout mechanism. **The HP/MP circular glass-orb + SD/AG wrapping-arc redesign from the reference
  study is a separate, not-yet-built follow-up** — see "Known follow-up" below; this generation's
  migration only retinted the existing rectangular bars.
- **Tooltips**: `surface-tooltip` background, `border-frame`, semantic colors for line-level
  meaning (matches `main_frame.rcss`'s existing `.tt-blue`/`.tt-red`/`.tt-darkred` skill-tooltip
  classes, left untouched — real game-data meaning, not theme chrome).

## Engine-constraint translation guide

So a future session adapting an external HTML/CSS mockup into this theme doesn't try to port a
feature this vendored RmlUi build doesn't have and hit a silent failure. Confirmed by reading the
vendored engine source, not assumed.

**Renderer upgrade, 2026-09-10**: `src/ThirdParty/RmlUi` was rebased from RmlUi 6.2 (plus this
project's own small custom gradient-shader commit) onto upstream RmlUi 6.3 plus
[PR #989](https://github.com/mikke89/RmlUi/pull/989) (unmerged upstream as of this writing), which
brings the vendored SDL_GPU renderer to feature parity with the GL3/DX12 backends — real clip
masks, `filter`/`backdrop-filter`/`mask-image`, and `box-shadow` with genuine blur. This project's
own prior gradient-shader commit is superseded by the PR's own richer version and was dropped, not
reapplied. Confirmed by a full clean engine build (`MuClient` + `Main.exe`, RelWithDebInfo) against
the new renderer, not assumed — several rows below flip from "unavailable" to confirmed-working as
a direct result. The submodule currently sits on a local branch (`integration/sdl-gpu-parity`) atop
that PR's branch tip; it has not yet been pushed to this project's own `nitoygo/RmlUi` fork or
committed, so treat the "confirmed working" rows below as verified-at-build-time, not yet a
committed/released baseline.

| Reference-CSS feature | Status on this engine | Use instead |
|---|---|---|
| `display: grid` / `grid-template-columns` | Not a registered `display` keyword (only `none/block/inline/inline-block/flow-root/flex/inline-flex/table*`) | Flexbox (`display: flex`, `flex-direction`, `align-items`, `justify-content` are all real, working properties here) |
| `::before`/`::after` + `content:` | No `content` property is registered at all; no generated-content pseudo-elements. Unaffected by the renderer upgrade above — this is a Core selector-engine gap, not a rendering one | A real child `<div>`/`<span>` element in the `.rml`, not a CSS-only decoration |
| `aspect-ratio` | Not a registered property | An explicit `height` (or `width`) alongside the element's own sizing |
| `text-shadow` | Not a registered property (distinct from `box-shadow`, see below) | `font-effect: outline(...)`, already used throughout |
| `repeating-linear-gradient` / `repeating-radial-gradient` / `repeating-conic-gradient` | **Confirmed working** as of the 2026-09-10 renderer upgrade — `RmlUi_Renderer_SDL_GPU.cpp` wires a `repeating` flag straight into the same `ShaderGradientFunction` enum the plain gradients use | Safe to use directly now; previously the advice was to fall back to a plain gradient or a tileable `@spritesheet` texture |
| `box-shadow` (inset and outset, with real blur) | **Confirmed working** as of the 2026-09-10 renderer upgrade — previously parsed but never rendered (layer/filter compositing was unimplemented); `login.rcss` is the first real consumer (frame bevel rings, the recessed-well vignette, button/input inset shadows) | Use directly, standard CSS syntax (`PropertyParserBoxShadow.cpp` confirms `inset`/offset/blur/spread/color in any order, comma-separated for multiple shadows) — the old per-side `border-*-color` bevel technique is still valid for crisp hard-edged lines, but a blurred shadow no longer needs faking |
| `filter: blur()` / `drop-shadow()`, `backdrop-filter`, `mask-image` | **Still unimplemented** — the 2026-09-10 upgrade's PR scope included this, but only `box-shadow` was actually confirmed working afterward (see `engine-findings.md`, which corrects an earlier version of this same row); don't assume PR scope equals delivered behavior without testing the decorator in isolation | `filter: brightness()`/`contrast()` are the one working exception (row below); for anything else needing a frosted/blurred look, no direct equivalent exists yet |
| `filter: brightness()`/`contrast()` | Confirmed working already before the upgrade — shipped (`my_inventory.rcss`'s `.inv-btn:hover`) | Use freely |
| `conic-gradient` | Confirmed working already before the upgrade — `RmlUi_Renderer_SDL_GPU.cpp` implements the full gradient family via `CompileShader`; there's a vendored `Tests/Data/VisualTests/shader_conic_gradient.rml` | Safe to use directly for a segmented/arc effect |
| A circular/radial progress arc | Browser CSS has no native equivalent; the reference fakes one with `conic-gradient` | RmlUi's own `<progress direction="clockwise"\|"counter-clockwise">` — real octant-triangle geometry (`ElementProgress.cpp`), **but only when a `fill-image`/sprite texture is set on the `<progress>` element** (confirmed by reading `ElementProgress::GenerateGeometry()` — without a texture, no manual mesh is built for the circular directions, so a decorator-only circular progress silently renders as an unclipped rectangle regardless of `value`). For a flat/vector look, use explicit segment `<div>`s instead (see `main_frame.rcss`'s planned HUD work) |
| `:focus-visible` | Not a supported pseudo-class (only `:hover`/`:active`/`:focus` and structural selectors like `nth-child` are) | `:focus`, already used by `.modern-inset.focused` |
| `var(--custom-property)` | **Now supported upstream** as of the 6.2→6.3 base bump (RCSS custom properties/variables, unrelated to the SDL_GPU renderer PR) — but this project has not switched to it | This project's own `token(name)` marker, resolved against `tokens.ini`, remains the mechanism actually in use; `var()` is a future option, not a drop-in replacement yet |
| CSS Grid convenience functions (`clamp()`, `minmax()`) | Not registered/needed by this codebase's fixed-dp/px layout convention | Fixed `dp`/`px` values, same as everything else in this theme |
| `transform: rotate()` | Confirmed working — `TransformUtilities.cpp` implements `Rotate2D`/`RotateX/Y/Z`, and `rotate(45deg)`/`rotate(-45deg)` render correctly on this renderer (`base.rcss`'s `.modern-joint` rivet, `login.rcss`'s hero-plate diamond studs, the checkbox checkmark) | Safe to use directly for 45°-square diamond/rivet ornaments and similar rotated shapes |

Only a mockup's **token values** (its actual palette) are meant to be adopted directly; its layout
mechanics and decorative pseudo-elements need translating through this table first.

## Current retrofit status

Every modern-theme `.rcss` file references this token table by name now — the rename/revalue pass
in this generation touched all ~20 files (mechanical `token(name)` renames verified by
`check_rml_rcss_drift.py`/`check_rml_rcss_syntax.py`, real literal-color retints hand-applied where
a value didn't route through a token at all). Real duplication was also consolidated where safe:
`login.rcss`'s byte-identical own-copy of `.btn`/`.btn-ok`/`.checkbox-box` (previously undeduped
against `base.rcss`) was deleted in favor of the shared versions — its distinctive warm-cream
outlined button text was folded into `base.rcss`'s `.btn-ok` itself so every primary/hero button
gets the same treatment, not just login's. `login_main.rcss`/`char_sel_main.rcss`'s independent
`.btn-icon` copies were consolidated the same way.

**Deliberately not tokenized**, each for its own reason, not an oversight:

- HUD text that overlays the 3D game world — needs its existing white+black-outline treatment for
  legibility against an arbitrary background, not the calmer panel-text convention.
- Gameplay-status colors (HP/MP/AG/SD/EXP bar fills' own gradient stops around the new
  `resource-*` base tokens, the cooldown-wipe's legacy-faithful ARGB value, the poisoned-HP green
  swap) — game data / a distinct rare-state color, not a decorative choice.
- The shared tooltip's `tt-blue`/`tt-red`/`tt-darkred`/`tt-yellow`/`tt-green`/`tt-purple` line
  colors (`tooltip.rcss`, both themes — now one shared file, not scattered per-window copies; see
  `component-catalog.md`'s "Tooltip" section) — carry real game-data meaning from
  `UI::RmlBridge::Tooltip::LineColor`, itself the union of every prior mechanism's palette including
  `UI::Skills::Tooltip::LineColor` and the old `ItemOptionTooltipModel.h`.
- `my_inventory.rcss`'s `.option-label`/`.option-label.active` Set/Socket colors — semantic
  option-category colors matching `legacy`, not theme chrome.
- Any HUD gameplay-overlay window (`main_frame`, `mu_helper_bar`, `buff_strip`) never gets
  `accent-crimson` — those need to stay legible/translucent over an arbitrary 3D background, and a
  loud crimson accent would fight that.
- `server_select.rcss`'s `.server-gauge-fill` (`#00bd8b`, a server-load status teal) — a distinct
  semantic status color, not theme chrome.

## Known follow-up: HUD circular glass-orb + arc gauges

The reference visual study's HP/MP circular glass-orb with wrapping SD/AG arc gauges was
**deliberately not built in this pass** — it's a structural rebuild of the combat HUD (new markup,
new `CMainFrameWindow` C++ binding shape, new tooltip anchor positions, interaction with
`main_frame_bg.rcss`'s paint-order mechanism and the `BottomHudScale()`/`UIScalePercent` scaling
path), not a retint, and touches real, currently-shipped combat UI. This generation's migration
retinted the existing rectangular HP/MP/AG/SD bars to the new `resource-*` tokens and left their
layout untouched. See the engine-constraint table above for the two RmlUi-native techniques already
verified as viable for this (`<progress direction="clockwise">` for the arcs, layered
`radial-gradient`/`linear-gradient` for the orb "liquid") whenever this follow-up is scheduled.

## Theme architecture requirement

Stays entirely inside `modern`. Do not modify `legacy` to match. Both themes continue sharing the
same RML/component structure — this document only constrains what `modern`'s own RCSS values are,
per the existing `Unified Components → {Legacy Theme, Modern Theme}` split
(`architecture-principles.md` §13, §29). This is **not** a third theme — only `legacy` and
`modern` are currently built (`README.md`, `theming-and-modding.md` — see the latter for why
that's not a permanent ceiling); this document is `modern`'s own visual system maturing, not a new
option next to it.

## Acceptance criteria

- Every new color/border value added to `modern`'s RCSS traces to a token in this table (or gets a
  new token added here first, not an unnamed one-off).
- Component states are expressed consistently across windows using the same token set.
- `legacy` is untouched by this document.
- No engine capability is assumed beyond what's confirmed above — don't reach for CSS features this
  build can't actually deliver (see the Engine-constraint translation guide).
