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
  bronze — see [Borders and frames](#borders-and-frames) below for that specific recolor. **Only
  title/name/label text (`text-title`) and the checkbox's checked state deliberately stay
  warm/gold** — general body/description/stat text is cool gray-blue like the structural surfaces
  (narrowed 2026-09-09, see [Text](#text) below); this isn't a blanket "remove all warmth" pass,
  warmth is just scoped to titles/emphasis now instead of all text.
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
| `surface-0` | `rgba(9, 11, 14, 190)` | Full-screen dim backdrop (`#backdrop`) |
| `surface-1` | `#12151a` | Base panel background |
| `surface-2` | `#1d232b` | Elevated panel / header background |
| `surface-3` | `rgba(255, 255, 255, 12)` | Interactive control, resting (neutral white wash over `surface-1`/`surface-2`) |
| `surface-tooltip` | `rgba(10, 12, 15, 235)` | Tooltip/floating-label background — used by `main_frame.rcss`, `buff_strip.rcss`, `mu_helper_bar.rcss`. Use this for any new tooltip rather than picking a new near-black. |

**Refreshed 2026-09-09** from a reference visual study (an external HTML/CSS mockup exploring the
same cool-steel/crimson-hero/gold-emphasis direction this document already committed to) — every
surface/accent/semantic/text/border value in this document and `tokens.ini` was retuned to that
mockup's palette. No token was renamed and no consumer changed — every existing `token(name)` call
site picks up the new value automatically. See "Engine-constraint translation guide" below for what
from that mockup is and isn't portable to this engine, and "Current retrofit status" for what this
refresh did and didn't touch.

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
| `accent-steel` | `#76889a` | General interactive chrome — hover/checked/focused states, dividers, bevel highlights, a resting button's border glow. Layered on top of the gray surfaces/borders, not the base material itself. |
| `accent-steel-bright` | `#aab8c6` | Hover-bright state for the same chrome |
| `accent-crimson` | `#a52336` | Primary/hero accent — a window's one confirm-style button, a title banner. One per window, never a whole list of equal-weight actions. |
| `accent-crimson-bright` | `#d34a5e` | Hover/bright state for the same primary element |
| `accent-gold` | `#d1aa55` | Semantic highlight — inline text emphasis, item/skill-slot selected/active highlights, the checkbox's checked state. Not general window/dialog chrome. |
| `accent-gold-bright` | `#ecd28a` | Brighter variant of the same semantic highlight (current/active skill-slot state) |
| `accent-blue` | `#5a8fc0` | Secondary accent — informational, non-primary interactive hints |

### Semantic colors

Communicate state/meaning, not decoration — apply only where the underlying data actually has that
meaning (a positive change, a destructive action, a rare item), never as generic flourish.

| Token | Value | Meaning |
|---|---|---|
| `semantic-success` | `#66956d` | Positive / success / healthy |
| `semantic-warning` | `#d69a3e` | Warning / caution (kept visually distinct from `accent-gold` so warnings don't read as ordinary emphasis) |
| `semantic-danger` | `#cb4855` | Danger / destructive / critical |
| `semantic-rare` | `#9a6bc0` | Rare / special / high-value |
| `semantic-neutral` | `#8a9099` | Neutral / disabled / secondary |

Where the game already has its own established item-rarity colors, preserve those existing
meanings rather than remapping them to this table — this table covers *UI* semantics (warnings,
danger prompts, success feedback), not item-quality colors.

### Text

| Token | Value | Use |
|---|---|---|
**Refreshed 2026-09-09 — warmth narrowed to titles/labels only.** This table used to read "titles
*and* body text stay warm off-white"; the reference visual study above applies warmth more
narrowly — only titles/names/labels (`text-title`) and the checkbox's checked state stay warm
gold-tan, while general body/description/stat text is cool gray-blue, same family as the
structural surfaces/borders. This makes the warm accent read as a stronger, rarer signal rather
than a blanket tint. Every window already reads `token(text-body)`/`token(text-title)` by name, so
this is a value-only change — no `.rcss` file needed editing for this to take effect.

| Token | Value | Use |
|---|---|---|
| `text-title` | `#d8bd7a` | Titles/headers/names/labels — warm gold-tan, the one place body copy stays warm |
| `text-body` | `#a5adb7` | Default readable body text — cool gray-blue, not warm (replaces bare `#fff`/`white` uses) |
| `text-muted` | `#717a84` | Metadata, secondary/subdued text — cool, one step dimmer than `text-body` |
| `text-emphasis` | `accent-gold` (`#d1aa55`) | Important inline emphasis within body text |

### Borders

Structural borders (frame/panel/hairline) stay neutral steel gray — `accent-steel` is reserved for
*interactive* accent states (see Accent colors above), not the structural base material; the two
share a hue family (both gray/metallic) but `accent-steel` sits brighter/more saturated so hover/
checked/focused states read as distinctly "activated" against the quieter structural borders.

| Token | Value | Use |
|---|---|---|
| `border-subtle` | `rgba(255, 255, 255, 12)` | Hairline separator/highlight on a dark panel |
| `border-metal` | `rgba(122, 133, 145, 130)` | Thin metallic edge — small controls, HUD elements |
| `border-strong` | `rgba(150, 161, 172, 180)` | Heavier metallic frame — major windows/dialogs only |

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

### Default look for every dialog/window: `.modern-frame`/`.modern-frame-crimson` + `.title-glow`

**Added 2026-09-09, by explicit user direction**: unless a window has a specific, documented reason
not to (see the exceptions below), every real dialog/window should apply `.modern-frame
modern-frame-crimson` to its outer `#panel` (the shared bevel/gradient frame, [Borders and
frames](#borders-and-frames) above) and, if it has an actual title/name label, `base.rcss`'s
`.title-glow` primitive (a soft radial gold-to-crimson glow, extracted from `login.rcss`'s original
header banner) as a sibling positioned before that title. This is the default now, not an opt-in —
a new dialog/window should reach for this combination first, and only skip it for a reason as
concrete as the ones below, not by default. Current adopters: `login`, `msg_win`,
`remember_password_prompt` (frame only, see below), `my_inventory`, `my_quest_info` (forked
2026-09-09 specifically to pick this up — it had no per-theme override before and was still on a
flat `surface-1`/`border-metal` fill with no banner at all), and `sys_menu` (forked the same day,
frame only, see below).

**Real, documented exceptions — don't copy these as an excuse to skip the default elsewhere:**
- **`remember_password_prompt`** gets the frame but deliberately no glow: its title color IS the
  security warning being confirmed (`semantic-warning`, not decorative) — a crimson banner behind a
  warning-colored title would compete with that meaning. See [Component states](#component-states)
  below and this file's own `.prompt-title` comment.
- **`sys_menu`** gets the frame but no glow: it has no title/name label at all (four stacked
  buttons, nothing to glow behind) — the glow's whole reason for existing (drawing the eye to a
  title) doesn't apply.
- **`char_make`, `char_sel_main`, `server_select`, `credit_win`**: `#panel` stays deliberately
  invisible/backgroundless in all four — `char_make`'s frames a live 3D character-preview viewport
  behind it, `char_sel_main` is a borderless button bar over the 3D scene, `server_select` sits over
  a video background (matching the real legacy screen, not a redesign choice, per that file's own
  comment), and `credit_win`'s `#panel` is a full-screen scrim, not a bordered window. None of these
  are "windows" in the bordered-dialog sense this convention is for — don't add a frame to any of
  them without first re-reading why they're transparent today.
- **HUD overlays** (`main_frame`, `mu_helper_bar`, `buff_strip`) were already excluded from
  `accent-crimson` entirely (see "Deliberately not tokenized" below) — this convention doesn't
  change that; a HUD element is not a dialog/window.

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

## Engine-constraint translation guide

Added 2026-09-09 alongside the reference-study refresh above, so a future session adapting an
external HTML/CSS mockup into this theme doesn't try to port a feature this vendored RmlUi build
doesn't have and hit a silent failure. Confirmed by reading the vendored engine source
(`StyleSheetSpecification.cpp`/`StyleSheetFactory.cpp`), not assumed:

| Reference-CSS feature | Status on this engine | Use instead |
|---|---|---|
| `display: grid` / `grid-template-columns` | Not a registered `display` keyword (only `none/block/inline/inline-block/flow-root/flex/inline-flex/table*`) | Flexbox (`display: flex`, `flex-direction`, `align-items`, `justify-content` are all real, working properties here) |
| `::before`/`::after` + `content:` | No `content` property is registered at all; no generated-content pseudo-elements | A real child `<div>`/`<span>` element in the `.rml`, not a CSS-only decoration |
| `aspect-ratio` | Not a registered property | An explicit `height` (or `width`) alongside the element's own sizing |
| `text-shadow` | Not a registered property | Color/contrast alone; no glow/outline-via-shadow trick |
| `repeating-linear-gradient` / `repeating-radial-gradient` | Only plain `linear-gradient`/`radial-gradient`/`conic-gradient` route through the working `CompileShader` shader path (see `STATUS.md`'s existing finding) | A plain (non-repeating) gradient, or a `@spritesheet`-based tileable texture if a repeating pattern is genuinely needed |
| `box-shadow` / `filter` / `backdrop-filter` | Parse but don't render (already documented above) | Per-side `border-*-color` bevels + `linear-gradient`, the existing `.modern-frame`/`.modern-panel`/`.modern-inset` technique |
| `:focus-visible` | Not a supported pseudo-class (only `:hover`/`:active`/`:focus` and structural selectors like `nth-child` are) | `:focus`, already used by `.modern-inset.focused` |
| `var(--custom-property)` | No CSS custom-property mechanism | This project's own `token(name)` marker, resolved against `tokens.ini` (see "Design tokens" above) |

Only a mockup's **token values** (its actual palette) are meant to be adopted directly; its layout
mechanics need translating through this table first.

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

**Retrofitted 2026-09-09, same day as the refresh above** (superseding the original plan to defer
this): `login.rcss`, `credit_win.rcss`, `server_select.rcss`, `mu_helper_bar.rcss`,
`my_inventory.rcss`, `main_frame.rcss`, `my_inventory_bg.rcss`, and `my_quest_info.rcss` each had
their own literal hex/rgba colors — either a genuine "own copy" of a `base.rcss` class (these files
don't link it, or predate the token mechanism, per each file's own header comment) or a one-off
value with no matching token (a status-dot color, a tooltip background) — brought onto the
refreshed palette. Every opaque literal that matches a token's exact value now reads
`token(name)` directly rather than repeating the hex; an alpha-blended derived shade (a gradient
mid-stop, a bevel highlight at reduced alpha) can't be expressed as a token call (`token(name)` is a
plain-text substitution, it can't be re-blended with a different alpha at the call site), so those
stay literal with a comment naming which token they derive from. `credit_win.rcss` and
`my_quest_info.rcss` needed no color changes (already fully tokenized); `my_quest_info.rcss`'s
tooltip background was switched from a literal near-black to `token(surface-tooltip)` for the same
reason. `main_frame_bg.rcss`/`my_inventory_bg.rcss`'s `.bg-panel` (each an intentionally
independent, theme-context-separate duplicate of `base.rcss`'s `.modern-frame`, per their own header
comments) were updated to match `.modern-frame`'s new values, keeping that documented duplication in
sync. **Deliberately still not touched**, each per its own file's explicit comment: gameplay-status
colors (HP/MP/AG/SD/EXP gauge fills, the cooldown-wipe ARGB value), the skill/item-option tooltip's
`tt-*` line colors (real game-data meaning), and `my_inventory.rcss`'s `.option-label`/`.tt-line`
Set/Socket colors (semantic option-category colors matching legacy, not theme chrome) — see
"Deliberately not tokenized" above for the full list and reasoning.

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
