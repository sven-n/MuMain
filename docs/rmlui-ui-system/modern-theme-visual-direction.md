# Modern Theme Visual Direction

Amends [`architecture-principles.md`](architecture-principles.md) §15 ("Themes Are Presentation
Packages") and §21 ("Design Tokens / Shared Theme Variables"), scoped to **the `modern` theme
only** — issued 2026-09-02, applied starting from `main_frame`'s Phase 2 checkpoint, while only a
handful of windows are shipped and the cost of establishing a shared palette now is low. The
`legacy` theme is explicitly out of scope and must not change as a result of this document — it
stays a pixel-faithful reproduction of the original game's look, governed by
[`legacy-theme-modernization.md`](legacy-theme-modernization.md) instead.

## Direction

`modern` should read as a dark, atmospheric, premium action-RPG interface — not a generic light
dashboard, not a flat web-app aesthetic. Concretely:

- Dark neutral surfaces, layered by shade rather than by heavy borders, to establish hierarchy --
  still the default for *structural* elements: outer frames/panels, HUD backgrounds, tooltips (see
  [Color tokens](#color-tokens) below; `.modern-frame`/`.modern-panel`/`.modern-inset` are
  deliberately untouched by the crimson/gold work described there).
- Two warm metallic accent families, used for meaning/emphasis, not as a base material for
  structural surfaces — see [Color tokens](#color-tokens): `accent-gold` for secondary highlights
  (hover/checked/selected borders, bevel highlights, dividers), `accent-crimson` for primary/hero
  emphasis (a window's one confirm-style action, a title banner). The two aren't interchangeable —
  see Accent colors below for which is which and why.
- Subtle framing (thin metallic edges, bevel highlights) reserved for major panels; small
  HUD/tooltip elements stay simple.
- High contrast only where interaction or readability actually needs it — most of the surface
  should stay quiet. A crimson primary button or banner is a deliberate exception to this (that
  *is* the interaction/readability need it's serving), not a blanket license to run every surface
  loud -- a window with several equal-weight buttons (no single primary action) has no reason to
  use `accent-crimson` on any of them; see `sys_menu.rcss`'s plain `.btn` for that case.
- Visually consistent across HUD, dialogs, tooltips, buttons, and panels — a shared vocabulary
  applied everywhere, not a one-off restyle per window.

**Revised 2026-09-03**: earlier drafts of this section said modern's look should stay "independent
of any specific external game's assets... nothing here should be sourced from or modeled
pixel-for-pixel on another game's UI." That's no longer accurate and shouldn't be treated as
current policy — the login dialog's crimson-banner-plus-gold-button treatment (see Accent colors)
was built by directly referencing real MU Online screenshots (an in-game merchant/item-sale
panel's banner and confirm button), and reads better for it. The real point of the original rule —
don't just reuse `legacy`'s own literal sprite pixels/assets as a shortcut, `modern` still needs to
be a genuine from-scratch RCSS implementation — still holds and is worth keeping in mind; citing a
real screenshot as color/composition reference for a hand-built RCSS decorator is a different thing
than that, and is now an accepted, ordinary part of how this theme's direction gets set.

## Design tokens

**Finding (2026-09-02, resolves the open question in `STATUS.md`'s "No design-token/shared-variable
layer" gap): this vendored RmlUi build has no `var()`/custom-property mechanism.** Confirmed against
`StyleSheetSpecification.cpp`'s property registration table (the authoritative list of every
property this build parses) — no `var()` parser exists. The two properties that look like CSS
variables, `--rmlui-language`/`--rmlui-direction`, are hardcoded internal properties using that
naming convention, not a general indirection feature.

So a "token" here is **a documented value, not a language construct** — the table below is the
single source of truth; each RCSS rule that uses one of these values should comment the token name
next to the literal (`background-color: #1b1714; /* surface-1 */`), the same way sprite-sheet rects
already get named. There's no compiler to catch drift — keeping RCSS in sync with this table is a
review discipline, the same one already in place for keeping `legacy`/`modern` themselves in sync.

**`box-shadow` does not work on this engine — confirmed, not just unverified.** `PropertyId::BoxShadow`
parses fine (`PropertyParserBoxShadow.cpp`) — the property registration check that originally read
as "good news" only proves that, not that it renders. This engine's `RmlUiRenderInterface` leaves
layer/filter compositing unimplemented, so a blurred `box-shadow` paints as a solid opaque block
instead of a blur — already known and documented, pre-dating this document, in `login.rcss`'s own
`#panel` comment (*"No box-shadow: this engine's RenderInterface leaves layer/filter compositing
unimplemented, so a blurred box-shadow renders as a solid incorrect block instead."*). Missed that
comment before adding `shadow-bevel` to `base.rcss`'s `.checkbox-box`/`.btn` (2026-09-02) — broke
both at runtime (solid white fills, borders gone, checkboxes collapsed to hairlines), reverted.
**Don't use `box-shadow` anywhere in this theme.** `filter`/`backdrop-filter` share the same
compositing gap and are equally off the table, even though both parse too. Bevel/depth cues have
to come from plain `border`/`background-color` layering instead (a lighter top-adjacent
inline-child or an extra nested div with its own flat border), not a shadow primitive.

### Surfaces

**Revised again 2026-09-02**: the bronze/crimson pass below was itself superseded the same day —
explicit feedback moved the direction to a neutral greyish/metallic palette instead, dropping the
warm brown-black in favor of a cooler near-black.

| Token | Value | Use |
|---|---|---|
| `surface-0` | `rgba(10, 9, 8, 190)` | Full-screen dim backdrop (`#backdrop`) |
| `surface-1` | `#1a1b1d` | Base panel background |
| `surface-2` | `#2c2f32` | Elevated panel / header background |
| `surface-3` | `rgba(255, 244, 224, 12)` | Interactive control, resting (warm-white wash over `surface-1`/`surface-2`, not pure white) |
| `surface-tooltip` | `rgba(20, 20, 20, 220)` | Tooltip/floating-label background. **Added 2026-09-03** to close a gap `STATUS.md`'s own "Pilots to revisit" table had flagged: `main_frame.rcss`/`buff_strip.rcss` already used this exact value uncommented (no token to point at), and `mu_helper_bar.rcss`'s own tooltip had drifted to a different bespoke value (`rgba(0, 0, 0, 200)`, pure black) — genuinely different code, not a deliberate variant. All three now reference this one token; use it for any future tooltip instead of picking a new near-black. |

### Accent colors

**Two accent families now, not one — settled 2026-09-03.** `accent-gold` stays what it's been
since 2026-09-02: a genuine accent (not a base material) layered on top of the neutral gray
surfaces/borders, for *secondary* emphasis — hover/checked/selected states, bevel highlights,
dividers. It does **not** mean "the important one" by itself.

`accent-crimson` is new: a second, equally-real accent family reserved for a window's one
*primary/hero* element — a single confirm-style action button, or a title banner — never applied
to more than one thing per window, and never to a plain list of equal-weight actions (see
Direction's `sys_menu.rcss` example above). Where both appear together (the login dialog), crimson
reads as "the one thing to do here" and gold reads as "state, not identity" — losing that
distinction (e.g. making every button crimson) is exactly the "high contrast everywhere" mistake
Direction warns against, just executed in the other color.

Both are real gradients in practice (bright top-ish stop fading to a dark bottom-ish stop, plus a
bevel border), not flat fills — see `login.rcss`'s `.header-banner`/`.btn`
and `base.rcss`'s new `.btn-ok` for the actual stop sequences; the table below gives representative
resting/bright values for reference and comment-naming, not the literal 4-stop gradients
themselves.

| Token | Value | Use |
|---|---|---|
| `accent-gold` | `#c0934c` | Secondary accent — hover/checked/selected states, dividers, bevel highlights. Layered on top of the gray surfaces/borders, not the base material itself. |
| `accent-gold-bright` | `#e0c060` | Selected/current state, brighter than resting hover/checked |
| `accent-crimson` | `#70221f` | Primary/hero accent — a window's one confirm-style button, a title banner. One per window, never a whole list of equal-weight actions. |
| `accent-crimson-bright` | `#8c2b26` | Hover/bright state for the same primary element |
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

Structural borders (frame/panel/hairline) stay steel gray — gold is reserved for accent states
(see Accent colors above), not the base material.

| Token | Value | Use |
|---|---|---|
| `border-subtle` | `rgba(255, 255, 255, 12)` | Hairline separator/highlight on a dark panel |
| `border-metal` | `rgba(140, 146, 152, 140)` | Thin metallic edge — small controls, HUD elements |
| `border-strong` | `rgba(176, 182, 188, 180)` | Heavier metallic frame — major windows/dialogs only |

**No shadow/glow tokens as such** — `box-shadow` is confirmed broken on this engine (see above).
Depth instead comes from per-side `border-*-color` bevels (independently-settable per side, real
working properties) plus `decorator: linear-gradient(...)` (confirmed working, proven by
`main_frame.rcss`'s gauge fills before this doc existed) — see `base.rcss`'s `.modern-frame`/
`.modern-panel`/`.modern-inset` for the technique, established 2026-09-02 against login.rcss as the
pilot. Light source is fixed upper-left throughout: raised surfaces get a lighter top/left border
and gradient corner, darker bottom/right; recessed surfaces invert both. **`.btn` itself stays
flat** (background-color + a single border-color, no gradient/bevel) — explicit feedback found the
glossy raised-button treatment (gradient fill, per-side bevel, hover/active gradient swaps)
unwanted; the frame/panel layering technique above is unaffected, only buttons went flat. The
border carries `accent-gold` at low alpha (`rgba(190, 160, 110, 90)`) rather than `border-metal` —
a faint gold glow at rest, brightening to `accent-gold-bright` on hover — so buttons read as
accented, not just gray, even though the fill itself has no gradient.

**`.btn-ok` is the one deliberate exception, added 2026-09-03**: a `.btn-ok` modifier class
(applied alongside `.btn`, e.g. `class="btn btn-ok"`) gets a real `accent-crimson` gradient fill
and bevel border — the primary/hero treatment Accent colors above describes. This isn't a reversal
of the flat-button decision, it's a second, opt-in variant for the specific case a window actually
has one confirm-style action to emphasize (`remember_password_prompt.rml`/`char_make.rml` already
use `class="btn btn-ok"`/`class="btn btn-cancel"` and picked this up automatically once `.btn-ok`
existed in `base.rcss`; `msg_win.rml`'s OK/Cancel buttons were given the same two classes in the
same pass). `.btn` on its own — no `.btn-ok` — stays exactly as flat/neutral as described above;
`sys_menu.rcss`'s four stacked menu buttons are the reference case for why plain `.btn` needs to
stay quiet (see Direction).

### Radius

| Token | Value |
|---|---|
| `radius-sm` | `3px` |
| `radius-md` | `4px` |

Matches values already in use (`.checkbox-box`, `.btn`) — no new scale introduced, just named.

### Typography — open item, not decided here

Every modern-theme `.rcss` file currently repeats `font-family: "Liberation Sans";` as a literal
(12 files, one grep) — the clearest concrete instance of the "no token layer" gap this section
addresses. This document defines the *token mechanism* (name it once, reference the name in
comments); it does not pick a different display font. Sourcing a distinct "premium" typeface is an
asset decision outside a docs pass — stays `"Liberation Sans"` under a `font-title`/`font-body`
naming split until a real replacement asset is chosen.

## Borders and frames

Prefer `border-metal`/`border-subtle` over thick, uniformly-colored borders.
Reserve `border-strong` for content that principles §7's "fixed-size" category already treats as a
major surface (full windows/dialogs) — HUD elements, small controls, lists, and tooltips stay on
`border-metal`/`border-subtle`, matching how `main_frame.rcss`'s `.skill-cell` already keeps its
border thin and non-ornamental.

## Component states

Every interactive element should express, through this same token set, whichever of these actually
apply: normal (`surface-3` resting wash), hover (`accent-gold` background — already `base.rcss`'s
`.btn:hover` convention, kept), pressed, focused (`accent-gold-bright` border — no glow available,
see the `box-shadow` caveat above), disabled (`semantic-neutral`
text/border + reduced opacity — already `.btn.disabled`'s convention), selected/active
(`accent-gold-bright` border, already `.skill-cell.selected`'s convention). State differences come
from brightness/contrast/background/border changes, not new shapes or layout shifts.

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
  (matches `main_frame.rcss`'s existing `.tt-blue`/`.tt-red`/`.tt-darkred` skill-tooltip classes —
  those already are this pattern, just not yet drawing from a named palette).

## Visual effects

`box-shadow`/`filter`/`backdrop-filter` are off the table entirely on this engine (see above) — no
glow/blur/shadow effects are currently available at all, deliberate or not. Whatever hierarchy/
state cues a future effect might have supplied have to come from color/border/layering instead.
Revisit this section only once this engine's render interface actually gains layer/filter
compositing.

## Rollout

Land in two passes, per the standing "audit → tokens → retrofit" sequencing:

1. This document (palette/token table) — done.
2. Retrofit `themes/modern/base.rcss` (`.btn`, `.checkbox-box`, `#backdrop`) to reference the new
   tokens by value+comment — done.
3. Retrofit every other already-shipped modern-theme `.rcss` (`login`, `login_main`, `sys_menu`
   inherits `base.rcss`'s `.btn` so needed nothing of its own, `remember_password_prompt`,
   `char_sel_main`, `char_make`, `char_info_balloon`, `msg_win`, `mu_helper_bar`, `buff_strip`,
   `main_frame`) — done. Scope was borders (→ `border-metal`), resting-control backgrounds (→
   `surface-3`), and panel/label text (→ `text-body`/`text-title`/semantic-*) — deliberately not
   touched: HUD text that overlays the 3D game world (needs its existing white+black-outline
   treatment for legibility against an arbitrary background, not the calmer panel-text convention),
   gameplay-status colors (HP/MP/AG/SD/EXP bar fills, the cooldown-wipe's legacy-faithful ARGB
   value), and the skill tooltip's `tt-red`/`tt-darkred` line colors (carry real game-data meaning
   from `UI::Skills::Tooltip::LineColor`, not a decorative choice).
4. Every *new* modern-theme RCSS should draw from this table directly rather than picking new ad
   hoc values.
5. **2026-09-03**: `accent-crimson`/`accent-crimson-bright` added (see Accent colors) after the
   login dialog's banner/primary-button work; `accent-gold`'s own representative value updated to
   match what's actually in use now. Applied to `base.rcss`'s `.checkbox-box`/`.checkbox-box.checked`
   (now matches `login.rcss`'s bronze/gold gradient checkbox exactly) and a new `.btn-ok` modifier
   (crimson, `remember_password_prompt`/`char_make`/`msg_win` all pick it up via
   `class="btn btn-ok"`/`class="btn btn-cancel"`, already present or added in the same pass) --
   see `.btn-ok` above for why plain `.btn` didn't just become crimson outright. **Deliberately not
   applied** to any HUD gameplay-overlay window (`main_frame`, `mu_helper_bar`, `buff_strip`) --
   those need to stay legible/translucent over an arbitrary 3D background per their own Panels/HUD
   guidance below, and a loud crimson accent would fight that; revisit only if a HUD window
   actually gets a genuine primary-action element that needs the emphasis, not as a blanket
   consistency pass.

## Theme architecture requirement

Stays entirely inside `modern`. Do not modify `legacy` to match. Both themes continue sharing the
same RML/component structure — this document only constrains what `modern`'s own RCSS values are,
per the existing `Unified Components → {Legacy Theme, Modern Theme}` split
(`architecture-principles.md` §13, §29). This is **not** a third theme — the "only `legacy` and
`modern`, kept side by side" standing rule (`README.md`, `theming-and-modding.md`) is unchanged;
this is `modern`'s own visual system maturing, not a new option next to it.

## Acceptance criteria

- Every new color/border/shadow value added to `modern`'s RCSS traces to a token in this table (or
  gets a new token added here first, not an unnamed one-off).
- Component states are expressed consistently across windows using the same token set.
- `legacy` is untouched by this document.
- No engine capability is assumed beyond what's confirmed above (`var()` no, `box-shadow`/`filter`/
  `backdrop-filter` parse but don't render correctly — don't use them) — don't reach for CSS
  features this build can't actually deliver.
