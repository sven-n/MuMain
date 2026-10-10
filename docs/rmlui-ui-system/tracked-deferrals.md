# Tracked Deferrals

Open work and accepted constraints, each with the trigger that brings it back. Completed migrations
belong in [migration-ledger.md](migration-ledger.md); engine behaviour in
[engine-findings.md](engine-findings.md). An entry leaves this file when it is done, and its outcome
is recorded where the code it changed is described.

## The C++ ↔ RML/RCSS ownership boundary

The ownership rollout moved layout out of C++ into the themes; the small-scale text and event
validation rollout (2026-10-10, [migration-ledger.md](migration-ledger.md#small-scale-text-and-event-validation))
closed the reported layout defects. What is left: constraints accepted with a trigger. For new UI,
follow [building-new-ui.md](building-new-ui.md)'s Ownership section. Paths below are relative to
`src/bin/Data/Interface/RmlUi/` for assets and `src/source/` for C++.

### Seen through `$preview`

The event windows that draw only while a server runs their event are looked at with `$preview <event>`
(`UI/Events/EventPreview.cpp`; `$preview` lists them, `$preview off` ends one). It fills a window
through the setters its packets use and lets it draw off its map; the window sends nothing. Teleporting
a game master to the map passes the map check but brings no event state, and the GM move does not
create the siege minimap that a map join does. A preview is not a live event: the Illusion Temple
and siege checks below still want a real event once one can be run.

- **Illusion Temple HUD**: its corner part (time, mini map, skill panel) is `#corner` in
  `cursed_temple_system.rml`; a theme moves it with `left`/`top`, and C++ reads the offset back to
  place its markers, digits, buttons and hit tests. Modern moves it left of the worn-equipment icons.
  Its three hover tooltips show in both themes (native's kill-point zones start at each number's
  centre, kept).
- **Siege commander HUD**: `$preview siege` seeds members, NPCs and commands against the hero's
  zoom-1 crop -- inside, on each edge and just outside -- so both zooms' clipping is exercised.

The Blood Castle and Chaos Castle timers, the duel spectator list, the CryWolf result and the HUD
status texts (`$preview status`; the crown switch lines show only at the switches) and the event
result and progress boxes (`$preview bcresult`, `ccresult`, `dsrank`, `switchbox`) and the guild war
time and result (`$preview guildwar`) look right. `$preview kanturu` fills the Kanturu entry window
and `$preview notices` the centre-screen notices; `$dialog menu <name>` opens an NPC menu without its
NPC.
The Battle Soccer score, the duel frame, the Empire Guardian timer and the Doppelganger frame draw
outside their event (`$win soccer full`, `duel`, `empiretimer`, `doppelframe`) and were checked in
both themes. The earlier rollout reported the remaining migrated windows checked in game, both
themes, with the scale sweep. That sweep does not establish resolution coverage or drag/theme-change
behaviour; see [layout-and-scaling.md](layout-and-scaling.md)'s "Checking it: the scale sweep".

### Accepted as it stands, with its trigger

- **`MiniMap`**: no reference-px space exists; the art is turned 45° in physical px.
  Trigger: a scoped map transform/layout redesign requiring theme-owned map geometry.
- **A new siege command's pulse**: `rgb(255, pulse, pulse)` welds the theme's red to a per-frame
  sine. RCSS cannot mix a bound fraction into a colour, so this needs a mechanism that does not
  exist in the current integration. The guard deliberately does not cover `color`.
  Trigger: a theme needing another pulse palette, or an available mechanism that separates the
  pulse's state from its colour without changing the command's timing.
- **`CCryWolf`'s sprites**: `SyncResult()` and `SyncHud()` choose the image files and texel
  rectangles (the 12x12 altar art, the 15x19 experience digits), and `crywolf.rml` binds `src` and
  `rect` from them. A theme can hide or rearrange that art, not replace it. Trigger: a theme wanting
  other event art, which means naming the sprite states in the model and leaving files and rects to
  the theme.
- **The modern options screen below 380dp tall** scrolls its page, and an open dropdown can be
  clipped by the pane. Reached only with a large UI scale in a small window. Trigger: reproducing
  inaccessible choices in a supported configuration; an outside-pane popup is a candidate fix.

### Deliberately not on this list

The documents that still bind geometry, each with a `<!-- bound-geometry: <why> -->` marker: per-frame data (gauges, things that follow the
pointer or scroll, windows that grow with their content, projected markers) and text measured the
way the native renderer measured it. A list the server does not bound is not a reason: it flows in
RCSS, as the duel spectators and score marks do.
`root_x`/`root_y` and `panel_x`/`panel_y` stay apart: the first is the physical origin of a root
scaled uniformly by `root_scale`, the second a reference-unit position inside a stretched `.screen`.

`.sharp-text` counter-scaled tops, `MiniMap`/`WorldLabelLayer` marker coordinates, and
`TitleSceneUI`'s loading bar are justified hybrids with recorded constraints. The loading bar is
pushed as real `px` because the scene's background is still native sprites on an 800x600-reference
per-axis scale `dp` cannot reproduce; it ends when those sprites port, and nothing links the two.

### The guard that keeps layout in the themes

`tools/check_rml_bound_geometry.py` runs in the build beside the syntax and contract-drift checks.
The contract-drift check now reports coverage and rejects zero-document success. It checks
required literal ids per document/theme and callbacks per view/theme, including linked templates;
alternative field readouts remain legal. See [theming-and-modding.md](theming-and-modding.md) for
running it and reviewing deliberate exceptions.
A document that binds geometry fails it unless it carries a `<!-- bound-geometry: <why> -->` marker
saying the geometry is the state itself (a gauge, something following the pointer, a projected
point, a size the user dragged); root placement and scaling expressions are exempt, and a marker on a
document that no longer binds geometry fails too. `--review` prints each marked document's bound
fields beside its reason: a document can keep needing its marker after the reason has gone stale, so
update the reason when a port changes what it binds. A custom property bound with a length unit
counts as geometry too (the theme's `calc()` makes it a box); the native text metrics (`--text-px`,
`--line-px`, `--line-height`, ...) are exempt as `text_px` is.

`tools/check_layout_transform_users.py` keeps the active UI transform to its infrastructure (the
window manager's measuring scope, the text renderer, the inventory's screen scope, the tooltip's
metric scope, listed in the script with their reasons); any other file using it fails the build.

Passing the guards establishes documented exceptions, not runtime correctness. The guard covers the four box offsets and the two sizes only: a bound `color`,
`decorator` or `font-size` has the same override problem but is a judgement call per case, while a
bound static coordinate is nearly always layout that belongs in RCSS. Widen it when a bound colour
bites.

## Modern theme studies

**HUD circular glass orbs and wrapping arc gauges.** The modern theme retinted `main_frame.rcss`'s
rectangular HP/MP/AG/SD bars rather than rebuilding them as orbs. This needs theme markup and
layout work plus tooltip/interaction checks on live combat UI; `CMainFrameWindow` already exposes
the resource fractions, both readout variants, poison state and hint strings. Reuse those bindings
before proposing C++ changes. Two techniques are
confirmed viable: `<progress direction="clockwise">` for the arcs (needs a `fill-image`,
`engine-findings.md`) and layered `radial-gradient` for the orb liquid. Trigger: a pass scoped to it,
once prioritized; low priority.

When it is taken up, prototype one resource orb and one clockwise arc in modern markup, using existing fractions and
an arc `fill-image`. Check empty/partial/full values, poison appearance, readouts, hover hints,
scale and rendering cost before expanding to HP/MP/AG/SD. Keep any new visual geometry and assets
theme-owned. Recheck workspace HUD bounds if the new silhouette grows above the existing strip.

Done when live damage/recovery and resource use update correctly, tooltips follow the drawn
elements, controls still block world clicks, and the legacy HUD passes the same behaviour checks.
Validate at 1280x720, 1920x1080 and ultrawide before promoting the prototype.

## When a trigger fires

| Triggered constraint | Smallest scoped follow-up | Acceptance |
|---|---|---|
| MiniMap redesign | Separate the map-to-screen transform from theme placement; keep projected marker data in C++. | Rotation, clipping, portal hit tests and markers align on 4:3, widescreen and ultrawide at changed UI scales. |
| Alternate siege pulse palette | Expose pulse state separately and choose a viable theme-owned colour/animation mechanism; widen the presentation guard only around the demonstrated need. | Legacy timing and colour remain recognizable; another palette works through theme assets/styles without a theme-name branch. |
| Alternate CryWolf art | Expose semantic altar/banner/rank/digit states and let themes select files and rects. | `$preview crywolf` / `crywolfresult` cover all seeded altar states and result art; alternate art and legacy crops work without C++ asset names. |
| Inaccessible options choices | Try bounded popup sizing/direction first; if clipping persists, use a shared outside-pane popup with theme-owned placement. | Every choice can be selected above and below 380dp; scrolling, dismissal, focus, theme changes and option persistence work in both themes. |
| Title-scene background port | Port the native background and its loading bar placement together, then remove C++-pushed bar geometry. | Bar/background alignment survives resolution/aspect changes; loading progress still presents correctly. |

For each change, capture the failing configuration before it and repeat it after; run the syntax,
contract-drift, bound-geometry and active-transform guards and the text-layout audit
(`tests/ui/test_rml_text_layout.cpp`); exercise resolution, scale and theme changes while the window
is shown. Remove an entry only after its acceptance passes, and record the outcome where the code
it changed is described.
