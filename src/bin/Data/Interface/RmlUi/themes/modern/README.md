# Modern theme

The `modern` theme's visual direction and what each design token means. `tokens.ini` beside this
file holds the values; this file holds their meaning. The theme exists mainly to prove the UI
architecture with a look entirely unlike `legacy`; `legacy` is never changed to match it.

How themes, tokens and forked `.rml` files work in general is in
`docs/rmlui-ui-system/theming-and-modding.md`; which CSS features this RmlUi build lacks, and what
to use instead, is in `docs/rmlui-ui-system/engine-findings.md`.

## Direction

Blackened iron / dark forged metal — a carved-window action-RPG interface, not a light dashboard
or a flat web app.

- **Dark iron surfaces**, always near-black, layered by shade rather than heavy borders.
- **Warmth only for titles and checked state.** Titles, names and labels use `text-warm`; the
  checkbox's checked state is gold. Body, description and stat text is a neutral grey scale.
- **Three accent families, not interchangeable**, used for meaning, never as a base material:
  - `accent-steel` — interactive chrome: hover, focused borders, dividers.
  - `accent-crimson` — a window's *one* primary element: a confirm-style action or a title
    banner. Never on a row of equal-weight actions (`sys_menu.rcss`'s plain `.btn`s).
  - `accent-gold` — inline text emphasis, selected/active slot highlights, the checked checkbox.
    **Never generic button chrome**: every plain `.btn`, hover included, stays neutral iron.
- **Sharp corners** (`radius-sm`/`radius-md` are 1–2 px): a carved plate, not a rounded panel.
- **Quiet by default.** High contrast only where interaction or readability needs it; thin
  metallic edges and bevels for major panels, small HUD and tooltip elements stay simple.
- **One vocabulary everywhere**, applied through `base.rcss`'s shared primitives, never a one-off
  restyle per window. Never reuse `legacy`'s sprite art.

## Tokens

### Surfaces

| Token | Use |
|---|---|
| `surface-deep` | Deepest panel body (the move list, the master level tree) |
| `surface-backdrop` | Full-screen dim backdrop (`#backdrop`), translucent so the scene shows |
| `surface-recessed` | Base/sunken panel background (`.modern-panel`) |
| `surface-panel` | Default panel body (`.modern-frame`) |
| `surface-raised` | Raised/header chrome; available, not yet used |
| `surface-control` | Interactive control at rest — a faint white wash (icon buttons, `.mu-btn`) |
| `surface-tooltip` | Every tooltip's background; use it for any new tooltip |

### Metal / frame

The structural iron, distinct from `accent-steel` (interactive chrome, not material).

| Token | Use |
|---|---|
| `metal-dark` | Deep iron — a button's own base tone |
| `metal-mid` | Mid iron; equals `border-frame` on purpose (both are "the visible iron edge") |
| `metal-edge` | Bevel-light catch on a raised surface |
| `metal-highlight` | Stronger bevel catch |
| `metal-warm-edge` | Sparing warm-brass ornament (the `-crimson` frame's top highlight, `login.rcss`'s divider) |

### Accents

| Token | Use |
|---|---|
| `accent-steel`, `accent-steel-hover` | Interactive chrome, and its hover state |
| `accent-crimson`, `accent-crimson-hover` | The primary element; in practice a gradient plus bevel (`base.rcss`'s `.btn-ok`) |
| `accent-gold`, `accent-gold-hover` | Semantic highlight, and its brighter variant |

### Semantic

| Token | Meaning |
|---|---|
| `semantic-success` | Positive / healthy |
| `semantic-warning` | Warning / caution |
| `semantic-danger` | Danger / destructive |
| `semantic-rare` | Rare / special / high-value |
| `semantic-neutral` | Neutral / disabled / secondary |

### Text

| Token | Use |
|---|---|
| `text-primary` | Default body text |
| `text-secondary` | Metadata, secondary text |
| `text-muted` | The quietest tier |
| `text-warm` | Titles, headers, names, labels — the only warm text |
| `text-disabled` | Disabled control text |

### Borders

| Token | Use |
|---|---|
| `border-frame` | The workhorse visible iron edge: frames, buttons, checkboxes, tooltips, HUD panels |
| `border-inner` | A recessed content well's edge (`.modern-panel`) |
| `border-recessed` | A barely visible hairline divider |
| `border-hover` | Hover-state border (`.btn:hover`) |
| `border-selected` | Selected-state border (`.slot--selected`, `.skill-cell.selected`) |

Prefer these over a new border colour; keep `border-hover`/`border-selected` for their states.

### HUD resources

`resource-hp`, `resource-mp`, `resource-sd`, `resource-ag`: each gauge's base fill. They mean "this
stat", not emphasis, so they are separate from the accents; the gradient stops around each base
stay local literals.

### Radius and typography

`radius-sm`/`radius-md`: see Direction. `font-title`/`font-body` are both "Liberation Sans" — the
split exists so a distinct display face is a one-line change; none has been chosen. Every text
rule declares its own `font-family` (`engine-findings.md`: it does not inherit reliably).

## Frames and dialogs

- **Docked panels and the inventory family** use the forged recipe — shell edge, groove, header
  rail — from `docked_panel_frame.rcss` (or the `*_bg.rcss` documents behind live 3D), shared by
  every window in a dock group so neighbours read as one family.
- **Small modal prompts** (`msg_win`, `remember_password_prompt`) use `base.rcss`'s
  `.modern-frame modern-frame-crimson` on `#panel`, with `.title-glow` (a soft radial glow) before
  a title. The `-crimson` variant is not red: it is a slightly warmer, darker iron. The name is a
  known misnomer. `remember_password_prompt` has no glow, because its title colour is the security
  warning.
- **No visible `#panel`**: `char_make`, `char_sel_main`, `server_select` and `credit_win` (a 3D
  viewport, a button bar over the scene, a video background, a full-screen scrim).
- **HUD overlays** (`main_frame`, `mu_helper_bar`, `buff_strip`) never get `accent-crimson`: they
  must stay legible over the world.

## States

Normal: `surface-control` wash, or `metal-dark` with `border-frame` for a button. Hover:
`border-hover`/`accent-steel-hover`. Pressed: a crimson-tinted border (`.btn:active`). Disabled:
`text-disabled`/`semantic-neutral` and reduced opacity. **Selected, active and checked use gold**
(`border-selected`, `accent-gold`), because they mark "this is the current one", not interactive
chrome. States change brightness, contrast, background or border — never shape or layout.

## Panels, HUD and tooltips

- **Panels**: `surface-recessed`/`surface-panel` layering for header vs body, `border-frame`
  outermost. `box-shadow` (bevel rings, wells, drop shadows) is available; `login.rcss` uses it most.
- **HUD**: legible in combat — translucent `surface-deep`/`surface-recessed` over opaque fills,
  `accent-gold` only for the current selection or an active cooldown.
- **Tooltips**: `surface-tooltip`, `border-frame`; line colours carry game meaning (below).

## Not tokens, on purpose

- Text over the 3D world keeps white with a black outline.
- Gameplay colours: gauge gradient stops, the cooldown wipe, the poisoned-HP green, the server-load
  teal (`server_select.rcss`'s `.server-gauge-fill`).
- The tooltip's `tt-*` line colours and the inventory's Set/Socket option colours — game data,
  identical to `legacy`.

## Rules for changes

- Every new colour or border traces to a token here; add the token (to `tokens.ini` and this file)
  before using a new value.
- States stay consistent across windows through the same tokens.
- Nothing here changes `legacy`.
