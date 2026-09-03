# Reusable Component Catalog

Closes `architecture-principles.md` §20's gap ("no reusable-component catalog exists as such") —
an honest inventory of what already functions as a reusable UI primitive today, named and pointed
at its actual file, so a future port checks here before inventing a new one-off mechanism. This is
a snapshot, not a promise: entries marked "doesn't exist yet" are real gaps, not placeholders for
work already planned — the next window that actually needs one is what should define its real
shape (§26), not this document guessing ahead of a real use case.

Read `architecture-principles.md` first if you haven't — §20 is the principle this document
audits status against. See `STATUS.md` for how this fits the rest of the tracked gaps.

## Window / Panel

Two structural patterns, not one unified `Window` component (`README.md`'s "Coexistence
patterns" section has the full detail):

- **Hybrid `CWin` + RmlUi overlay** — the legacy `CWin` instance stays alive purely for
  positioning/hit-testing bookkeeping (`CWin::Create()` with `nTexID=-2` so it allocates no
  background sprite); RmlUi renders 100% of the visible chrome. Used by `CLoginWin`,
  `CLoginMainWin`, `CSysMenuWin`, `CCharSelMainWin`, `CCharMakeWin`.
- **Pure RmlUi** — no `CWin`/`CUIMng` involvement at all. Used by `RememberPasswordPrompt`,
  `CMsgWin`, `CCharInfoBalloonMng`.

Visual frame primitives are theme-specific, not shared (correct per §15 — presentation is the
theme's job, not the component's):

- `modern`: `themes/modern/base.rcss`'s `.modern-frame`/`.modern-panel`/`.modern-frame-accent`/
  `.modern-inset`, plus the `-crimson` palette variant (`modern-theme-visual-direction.md` has the
  full token table these draw from).
- `legacy`: `themes/legacy/base.rcss`'s sprite-based 3-part `.panel-cap-top`/`.panel-cap-bottom`/
  `.panel-middle`.

## Button

Real shared contract across both themes already — `.btn`/`.btn-ok`/`.btn-cancel`/`.btn.disabled`,
same class names, same state model, each theme's own `base.rcss`. `.btn-ok` gets each theme's
"primary/hero" treatment (see `modern-theme-visual-direction.md`'s Accent colors section); plain
`.btn` stays neutral.

## Checkbox

`.checkbox-row`/`.checkbox-box`/`.checkbox-box.checked`/`.checkbox-label`, both themes' `base.rcss`
— same shared-contract shape as Button.

## Layout utilities

Not named in §20's own list, but the closest thing to a real cross-window primitive that exists
today (`layout-and-scaling.md` is the full reference): `.anchor-{top,bottom}-{left,right}`,
`.center-{x,y,both}`, `.stretch-{x,y,both}`, `.hidden`, `.layout-anchor` — both themes' `base.rcss`,
identical class names and behavior in each.

## Data binding

`RmlModelBinder<T>` (`UI/RmlBridge/RmlModelBinder.h`) — the per-window model/binder lifecycle
wrapper every migrated window uses: owns the `Model` instance, creates the `Rml::DataModelHandle`
once, exposes `MarkDirty()` so packet-handler/action-controller code doesn't need to know RmlUi's
binding API directly.

## Theming

`UI::RmlBridge` (`RmlTheme.h`): `LoadThemedDocument()` (the one entry point every migrated window
uses instead of `Context::LoadDocument` directly — makes "add a theme" a drop-a-folder operation),
`GetActiveThemeName()`, `ThemeProvidesOwnIconChrome()` (a declared theme capability,
`architecture-principles.md` §30 — see `theming-and-modding.md` for the pattern to follow for any
future capability flag). See `theming-and-modding.md`'s "Forking a theme's RML" section for the
per-theme RML/RCSS override mechanism itself, not a separate component but part of this same
theming layer.

## Dragging

`UI::RmlBridge::MakeDraggable()` (`RmlDraggable.h`) — makes an RmlUi panel draggable-by-mouse with
zero legacy `CWin` dependency. **Zero live call sites today** — read the header's own audit-finding
comment (dp-vs-px fixed 2026-09-04, persistence still deliberately unbuilt) before wiring up the
first real caller.

## Does not exist as a reusable primitive yet

Recorded here so a future session doesn't assume otherwise — each of these is still ad hoc,
per-window, or entirely unbuilt:

- **ItemSlot / ItemGrid** — still 100% legacy 2D rendering (`STATUS.md`: "no RmlUi pattern proven
  yet"). `CNewUIItemHotKey` (potion slots, 3D-camera-composited icons) is the next real candidate
  to prove a pattern against, once it's scoped.
- **ProgressBar / HealthBar / ManaBar / ExperienceBar** — `main_frame.rcss`'s HP/MP/AG/SD/EXP
  gauge-fill rules (`#hp_fill` etc.) are ad hoc per-window CSS, not an abstracted, reusable bar
  component another window could reference.
- **Tooltip** — actively **three non-unified mechanisms** exist side by side (flagged in
  `newui-tier-adapter.md`'s pilots-to-revisit table): the skill-hotkey tooltip, `CBuffStrip`'s
  plain-text tooltip (a deliberate scope cut from the original's per-line-colored rich tooltip),
  and whatever the still-fully-legacy windows use. Consolidating these is its own future item, not
  bundled here.
- **Dialog** (as distinct from Window/Panel above) — every dialog window today (`CMsgWin`,
  `RememberPasswordPrompt`) is its own hand-built RML/RCSS pair; no shared "Dialog" scaffold
  (title/body/button-row layout contract) exists for a new one to reference.
- **Tab / TabBar, List, ScrollContainer, Notification, HUDContainer** — none of the currently
  migrated windows have needed one yet, so none exist. `CNewUIMainFrameWindow`'s still-legacy
  skill grid/pet-command row is the closest thing to a "grid" concept in the codebase, and it
  hasn't been abstracted either (see `STATUS.md`'s pilots-to-revisit entry for why its icon art
  stayed legacy 2D).

## Using this catalog

Before building a new one-off mechanism for a window port: check this list first. If the concept
you need already exists above, reuse it. If it's listed as a gap, that's a signal you may be the
first real use case defining its shape — follow `architecture-principles.md` §27's workflow
(understand intent, then design the RML/RCSS/C++ split) rather than copying whatever the nearest
existing window happens to do, and update this catalog once the pattern is proven.
