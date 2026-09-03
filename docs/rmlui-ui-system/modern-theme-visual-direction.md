# Modern Theme Visual Direction

Amends [`architecture-principles.md`](architecture-principles.md) §15 ("Themes Are Presentation
Packages") and §21 ("Design Tokens / Shared Theme Variables"), scoped to **the `modern` theme
only**. The `legacy` theme is explicitly out of scope and must not change as a result of this
document — it stays a pixel-faithful reproduction of the original game's look, governed by
[`legacy-theme-modernization.md`](legacy-theme-modernization.md) instead.

## Direction

`modern` should read as a dark, atmospheric, premium action-RPG interface — not a generic light
dashboard, not a flat web-app aesthetic. Concretely:

- Dark neutral surfaces (`surface-0`/`-1`/`-2`, always gray/near-black, never warm) layered by
  shade rather than heavy borders, to establish hierarchy. A dialog's own decorative frame/panel
  material (the `.modern-frame-crimson`/`.modern-panel-crimson` family — `login`,
  `remember_password_prompt`, `msg_win`) is **also** gray/metallic/near-black now, not warm
  bronze — see [Borders and frames](#borders-and-frames) below for that specific recolor. **Text
  (title/body) and the checkbox deliberately stay warm/gold** — only the structural frame/panel
  material itself went cool; this isn't a blanket "remove all warmth" pass.
- Three accent families, used for meaning/emphasis, not as a base material for structural
  surfaces — see [Color tokens](#color-tokens): `accent-steel` for general interactive chrome
  (hover/focused borders, bevel highlights, dividers), `accent-crimson` for primary/hero emphasis
  (a window's one confirm-style action, a title banner), and `accent-gold` reserved for inline
  text emphasis, item/skill-slot selected/active highlights, and the checkbox's checked state —
  never for general window/dialog chrome otherwise. The three aren't interchangeable — see Accent
  colors below for which is which and why.
- Subtle framing (thin metallic edges, bevel highlights) reserved for major panels; small
  HUD/tooltip elements stay simple.
- High contrast only where interaction or readability actually needs it — most of the surface
  should stay quiet. A crimson primary button or banner is a deliberate exception to this (that
  *is* the interaction/readability need it's serving), not a blanket license to run every surface
  loud — a window with several equal-weight buttons (no single primary action) has no reason to
  use `accent-crimson` on any of them; see `sys_menu.rcss`'s plain `.btn` for that case.
- Visually consistent across HUD, dialogs, tooltips, buttons, and panels — a shared vocabulary
  applied everywhere, not a one-off restyle per window.
- Color/composition may be informed by real MU Online screenshots as reference (the login dialog's
  crimson-banner treatment was built this way) — but `modern` must still be a genuine from-scratch
  RCSS implementation, never a reuse of `legacy`'s own literal sprite pixels or assets as a
  shortcut.

## Design tokens

**This vendored RmlUi build has no `var()`/custom-property mechanism** (confirmed against
`StyleSheetSpecification.cpp`'s property registration table — no `var()` parser exists; the two
properties that look like CSS variables, `--rmlui-language`/`--rmlui-direction`, are hardcoded
internal properties using that naming convention, not a general indirection feature). **Fixed
2026-09-04, without any RmlUi engine change**: `UI::RmlBridge::LoadThemedDocument()`
(`RmlTheme.cpp`) now resolves a `token(name)` marker in an `.rcss` file against
`themes/<theme>/tokens.ini`'s `[Tokens]` section before RmlUi ever sees the text — reusing the
fact that RmlUi's own `XMLNodeHandlerHead` treats an inline `<style>` block in `<head>` identically
to an external `<link type="text/rcss">` for cascade purposes, so a tokenized stylesheet gets
spliced in as `<style>` instead of linked. Content-driven, not theme-name-driven (a stylesheet
with no `token(...)` marker is left completely untouched — the exact code path every window
already took before this existed), so `legacy` (no token layer, still theme-specific by design)
is unaffected. The table below is still the single source of truth for what each token *means* —
`themes/modern/tokens.ini` is where its actual *value* lives now, and the two must stay in sync
the same review-discipline way `legacy`/`modern` themselves already do (there's still no compiler
to catch drift between the table and the ini file, only between `token(name)` and a name that
doesn't exist in the ini, which fails loudly — an empty resolved value, not a silent wrong color).

**`box-shadow` does not render on this engine.** `PropertyId::BoxShadow` parses fine — property
registration alone doesn't mean the render interface implements it. This engine's
`RmlUiRenderInterface` leaves layer/filter compositing unimplemented, so a blurred `box-shadow`
paints as a solid opaque block instead of a blur. `filter`/`backdrop-filter` share the same gap
and are equally off the table, even though both parse too. **Don't use any of the three anywhere
in this theme** — bevel/depth cues have to come from plain `border`/`background-color` layering
instead (a lighter top-adjacent inline-child, or an extra nested div with its own flat border),
not a shadow primitive. Lesson for any future capability check: don't parser-check a rendering
capability — check the render interface, or test the specific decorator in isolation.

### Surfaces

| Token | Value | Use |
|---|---|---|
| `surface-0` | `rgba(10, 9, 8, 190)` | Full-screen dim backdrop (`#backdrop`) |
| `surface-1` | `#1a1b1d` | Base panel background |
| `surface-2` | `#2c2f32` | Elevated panel / header background |
| `surface-3` | `rgba(255, 255, 255, 12)` | Interactive control, resting (neutral white wash over `surface-1`/`surface-2`) |
| `surface-tooltip` | `rgba(20, 20, 20, 220)` | Tooltip/floating-label background — used by `main_frame.rcss`, `buff_strip.rcss`, `mu_helper_bar.rcss`. Use this for any new tooltip rather than picking a new near-black. |

### Accent colors

Three accent families, not interchangeable, each with a distinct job:

- **`accent-steel`** — general interactive chrome: hover, checked, focused-border, dividers, bevel
  highlights, a resting button's border glow. This is the role `accent-gold` used to play; gold no
  longer means "interactive," steel does.
- **`accent-crimson`** — a window's one *primary/hero* element: a single confirm-style action
  button, or a title banner. Never applied to more than one thing per window, and never to a plain
  list of equal-weight actions (see Direction's `sys_menu.rcss` example above).
- **`accent-gold`** — scoped to inline text emphasis, item/skill-slot selected/active/current
  highlights (`.skill-cell.selected`, the hotkey current-selection glow), and the checkbox's
  checked state (`.checkbox-box.checked`) — kept warm deliberately, not part of the frame-material
  recolor. Never used for general window/dialog chrome otherwise. **Inconsistency worth knowing**:
  `char_make.rcss`'s `.job-btn.checked` (the class-selection buttons) uses `accent-steel`, not
  gold, even though it's also a "checked" state — that one wasn't part of the checkbox exception,
  so it kept the general-chrome treatment. Not a bug, just two different "checked" controls
  landing on different sides of the gold/steel line; don't assume one implies the other.

Where crimson and steel appear together (the login dialog), crimson reads as "the one thing to do
here" and steel reads as "state, not identity" — losing that distinction (e.g. making every button
crimson) is exactly the "high contrast everywhere" mistake Direction warns against, just executed
in the other color.

Crimson is a real gradient in practice (bright top-ish stop fading to a dark bottom-ish stop, plus
a bevel border), not a flat fill — see `login.rcss`'s `.header-banner` and `base.rcss`'s `.btn-ok`
for the actual stop sequence; the table below gives representative resting/bright values for
reference and comment-naming, not the literal 4-stop gradient itself.

| Token | Value | Use |
|---|---|---|
| `accent-steel` | `#b0bac4` | General interactive chrome — hover/checked/focused states, dividers, bevel highlights, a resting button's border glow. Layered on top of the gray surfaces/borders, not the base material itself. |
| `accent-steel-bright` | `#d4dce4` | Hover-bright state for the same chrome |
| `accent-crimson` | `#70221f` | Primary/hero accent — a window's one confirm-style button, a title banner. One per window, never a whole list of equal-weight actions. |
| `accent-crimson-bright` | `#8c2b26` | Hover/bright state for the same primary element |
| `accent-gold` | `#c0a050` | Semantic highlight — inline text emphasis, item/skill-slot selected/active highlights, the checkbox's checked state. Not general window/dialog chrome. |
| `accent-gold-bright` | `#e0c060` | Brighter variant of the same semantic highlight (current/active skill-slot state) |
| `accent-blue` | `#6f9bc4` | Secondary accent — informational, non-primary interactive hints |

### Semantic colors

Communicate state/meaning, not decoration — apply only where the underlying data actually has that
meaning (a positive change, a destructive action, a rare item), never as generic flourish.

| Token | Value | Meaning |
|---|---|---|
| `semantic-success` | `#6fae6f` | Positive / success / healthy |
| `semantic-warning` | `#d69a3e` | Warning / caution (kept visually distinct from `accent-gold` so warnings don't read as ordinary emphasis) |
| `semantic-danger` | `#b2394a` | Danger / destructive / critical |
| `semantic-rare` | `#9a6bc0` | Rare / special / high-value |
| `semantic-neutral` | `#9a958c` | Neutral / disabled / secondary |

Where the game already has its own established item-rarity colors, preserve those existing
meanings rather than remapping them to this table — this table covers *UI* semantics (warnings,
danger prompts, success feedback), not item-quality colors.

### Text

| Token | Value | Use |
|---|---|---|
| `text-title` | `#efe6d3` | Titles/headers — warm off-white, not pure white |
| `text-body` | `#d9d3c5` | Default readable body text (replaces bare `#fff`/`white` uses) |
| `text-muted` | `#8f897c` | Metadata, secondary/subdued text |
| `text-emphasis` | `accent-gold` (`#c0a050`) | Important inline emphasis within body text |

### Borders

Structural borders (frame/panel/hairline) stay neutral steel gray — `accent-steel` is reserved for
*interactive* accent states (see Accent colors above), not the structural base material; the two
share a hue family (both gray/metallic) but `accent-steel` sits brighter/more saturated so hover/
checked/focused states read as distinctly "activated" against the quieter structural borders.

| Token | Value | Use |
|---|---|---|
| `border-subtle` | `rgba(255, 255, 255, 12)` | Hairline separator/highlight on a dark panel |
| `border-metal` | `rgba(140, 146, 152, 130)` | Thin metallic edge — small controls, HUD elements |
| `border-strong` | `rgba(176, 182, 188, 180)` | Heavier metallic frame — major windows/dialogs only |

**No shadow/glow tokens as such** — `box-shadow` is confirmed broken on this engine (see above).
Depth instead comes from per-side `border-*-color` bevels (independently-settable per side, real
working properties) plus `decorator: linear-gradient(...)` (confirmed working) — see
`base.rcss`'s `.modern-frame`/`.modern-panel`/`.modern-inset` for the technique. Light source is
fixed upper-left throughout: raised surfaces get a lighter top/left border and gradient corner,
darker bottom/right; recessed surfaces invert both.

**`.btn` itself stays flat** (background-color + a single border-color, no gradient/bevel) — a
glossy raised-button treatment (gradient fill, per-side bevel, hover/active gradient swaps) was
tried and found unwanted; the frame/panel layering technique above is unaffected, only buttons
stay flat. The border carries `accent-steel` at low alpha (`rgba(176, 186, 196, 90)`) rather than
`border-metal` — a faint steel glow at rest, brightening to `accent-steel-bright`
(`rgba(212, 220, 228, 210)`) on hover — so buttons read as accented, not just gray, even though the
fill itself has no gradient.

**`.btn-ok` is the one deliberate exception**: a `.btn-ok` modifier class (applied alongside
`.btn`, e.g. `class="btn btn-ok"`) gets a real `accent-crimson` gradient fill and bevel border —
the primary/hero treatment Accent colors above describes, for the specific case a window actually
has one confirm-style action to emphasize. `remember_password_prompt.rml`/`char_make.rml`/
`msg_win.rml`'s OK/Cancel buttons use `class="btn btn-ok"`/`class="btn btn-cancel"`. `.btn` on its
own — no `.btn-ok` — stays exactly as flat/neutral as described above; `sys_menu.rcss`'s four
stacked menu buttons are the reference case for why plain `.btn` needs to stay quiet (see
Direction).

### Radius

| Token | Value |
|---|---|
| `radius-sm` | `3px` |
| `radius-md` | `4px` |

Matches values already in use (`.checkbox-box`, `.btn`) — no new scale, just named.

### Typography — open item, not decided here

Every modern-theme `.rcss` file currently repeats `font-family: "Liberation Sans";` as a literal —
the clearest concrete instance of the "no token layer" gap this section addresses. This document
defines the *token mechanism* (name it once, reference the name in comments); it does not pick a
different display font. Sourcing a distinct "premium" typeface is an asset decision outside a docs
pass — stays `"Liberation Sans"` under a `font-title`/`font-body` naming split until a real
replacement asset is chosen.

## Borders and frames

Prefer `border-metal`/`border-subtle` over thick, uniformly-colored borders. Reserve
`border-strong` for content that principles §7's "fixed-size" category already treats as a major
surface (full windows/dialogs) — HUD elements, small controls, lists, and tooltips stay on
`border-metal`/`border-subtle`, matching how `main_frame.rcss`'s `.skill-cell` already keeps its
border thin and non-ornamental.

### The `-crimson`-named frame/panel variant is gray/metallic now, not warm bronze

`base.rcss`'s `.modern-frame-crimson`/`.modern-panel-crimson`/`.modern-frame-accent-crimson` — the
palette variant `login`, `remember_password_prompt`, and `msg_win` apply alongside the generic
`.modern-frame`/`.modern-panel`/`.modern-frame-accent` primitives — were originally a warm
bronze/near-black material with warm-gold edge highlights (styled off a real MU screenshot's
warm-bronze/crimson dialog look). That base material is now cool gray/metallic (steel-tinted edge
highlights instead of gold ones); text, the checkbox, the header glow, and the primary button's
crimson fill all stay warm on top of it. The classes keep their `-crimson` name for now even
though the frame material itself no longer is — renaming is a real cleanup worth doing eventually
(touches 3 `.rml` files' `class` attributes plus `base.rcss`'s own selectors) but wasn't bundled
into the color change itself. `login.rcss` also has its own local `.modern-inset`/
`.modern-inset.focused` override (the input-box frame border) that went through the identical
warm→cool treatment for the same reason.

## Component states

Every interactive element should express, through this same token set, whichever of these actually
apply: normal (`surface-3` resting wash), hover (`accent-steel` background — `base.rcss`'s
`.btn:hover` convention), pressed, focused (`accent-steel-bright` border — no glow available, see
the `box-shadow` caveat above), disabled (`semantic-neutral` text/border + reduced opacity —
`.btn.disabled`'s convention). **Selected/active and the checkbox's checked state use gold, not
steel** (`accent-gold-bright` border, `.skill-cell.selected`'s convention; `accent-gold`,
`.checkbox-box.checked`'s convention) — both are semantic "this specific thing is the current
one/is checked" highlights, not general interactive chrome, so they keep the gold accent rather
than switching to steel. State differences come from brightness/contrast/background/border
changes, not new shapes or layout shifts.

## Panels, HUD, and tooltips

- **Panels/windows**: `surface-1`/`surface-2` layering for header-vs-body distinction,
  `border-strong` only at the outermost frame (no shadow-based lift available — see the
  `box-shadow` caveat above).
- **HUD**: stays legible during combat — favor `surface-0`/`surface-1`-level translucency over
  opaque fills, `accent-gold` used sparingly for the specific things that matter (current
  selection, active cooldown state), never as general decoration. Layout/responsiveness stays
  governed by `layout-and-scaling.md` — this document only supplies color/border/shadow tokens, not
  a new layout mechanism.
- **Tooltips**: `surface-2`-level background (denser than a HUD element, since it's transient and
  benefits from full readability), `border-subtle`, semantic colors for line-level meaning
  (matches `main_frame.rcss`'s existing `.tt-blue`/`.tt-red`/`.tt-darkred` skill-tooltip classes).

## Visual effects

`box-shadow`/`filter`/`backdrop-filter` are off the table entirely on this engine (see above) — no
glow/blur/shadow effects are currently available at all. Whatever hierarchy/state cues a future
effect might have supplied have to come from color/border/layering instead. Revisit this section
only once this engine's render interface actually gains layer/filter compositing.

## Current retrofit status

`themes/modern/base.rcss` (`.btn`, `.checkbox-box`, `#backdrop`) and every other already-shipped
modern-theme `.rcss` (`login`, `login_main`, `sys_menu`, `remember_password_prompt`,
`char_sel_main`, `char_make`, `char_info_balloon`, `msg_win`, `mu_helper_bar`, `buff_strip`,
`main_frame`) reference these tokens by value+comment for borders (→ `border-metal`),
resting-control backgrounds (→ `surface-3`), and panel/label text (→
`text-body`/`text-title`/semantic-*). Every *new* modern-theme RCSS should draw from this table
directly rather than picking new ad hoc values.

**Deliberately not tokenized**, each for its own reason, not an oversight:

- HUD text that overlays the 3D game world — needs its existing white+black-outline treatment for
  legibility against an arbitrary background, not the calmer panel-text convention.
- Gameplay-status colors (HP/MP/AG/SD/EXP bar fills, the cooldown-wipe's legacy-faithful ARGB
  value) — game data, not a decorative choice.
- The skill tooltip's `tt-red`/`tt-darkred` line colors — carry real game-data meaning from
  `UI::Skills::Tooltip::LineColor`.
- Any HUD gameplay-overlay window (`main_frame`, `mu_helper_bar`, `buff_strip`) never gets
  `accent-crimson` — those need to stay legible/translucent over an arbitrary 3D background per
  their own Panels/HUD guidance above, and a loud crimson accent would fight that. Revisit only if
  a HUD window actually gets a genuine primary-action element that needs the emphasis, not as a
  blanket consistency pass.

## Theme architecture requirement

Stays entirely inside `modern`. Do not modify `legacy` to match. Both themes continue sharing the
same RML/component structure — this document only constrains what `modern`'s own RCSS values are,
per the existing `Unified Components → {Legacy Theme, Modern Theme}` split
(`architecture-principles.md` §13, §29). This is **not** a third theme — only `legacy` and
`modern` are currently built (`README.md`, `theming-and-modding.md` — see the latter for why
that's not a permanent ceiling); this document is `modern`'s own visual system maturing, not a new
option next to it.

## Acceptance criteria

- Every new color/border/shadow value added to `modern`'s RCSS traces to a token in this table (or
  gets a new token added here first, not an unnamed one-off).
- Component states are expressed consistently across windows using the same token set.
- `legacy` is untouched by this document.
- No engine capability is assumed beyond what's confirmed above (`var()` no, `box-shadow`/`filter`/
  `backdrop-filter` parse but don't render correctly — don't use them) — don't reach for CSS
  features this build can't actually deliver.
