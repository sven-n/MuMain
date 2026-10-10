# Tracked Deferrals

Open work and accepted constraints, each with the trigger that brings it back. Completed migrations
belong in [migration-ledger.md](migration-ledger.md); engine behaviour in
[engine-findings.md](engine-findings.md). An entry leaves this file when it is done, and its outcome
is recorded where the code it changed is described.

## Before merging to main

No code-health item is left.

## After the merge

- **Party list bounds.** At 720p and about 110% or more the list rises above the screen in both
  themes, and in modern it overlaps the top bar at 100%. Keep it between the top bar and the HUD.
- **Legacy trade overlaps.** The Warning, notice and confirm texts overlap; an RCSS fix.
- **Native text floor at small scales.** Native text never drops below 11 pt times the OS display
  scale, so at 75 % on 1024x768 fixed-length lines run past small docked windows (Blood Castle and
  Devil Square entry, MU Helper, the shop warning). Let the text follow the panel scale down, or
  fit those lines to their boxes.

Waiting on their own triggers: the [accepted constraints](#accepted-as-it-stands-with-its-trigger)
and the [modern HUD orbs](#modern-theme-studies).

Throughout: run targeted scale, theme and interaction checks with each change. The modern theme
exists to prove the architecture, so its unchecked windows are not tracked. The `CObject` tier is
accepted as the base ([building-new-ui.md](building-new-ui.md)'s "Accepted as the base"), and window
placement is theme-owned ([window-placement.md](window-placement.md)).

## The C++ ↔ RML/RCSS ownership boundary

The ownership rollout moved layout out of C++ into the themes. What is left: windows not yet seen in
game and constraints accepted with a trigger. For new UI, follow
[building-new-ui.md](building-new-ui.md)'s Ownership section.

### Seen through `$preview`

The event windows that draw only while a server runs their event are looked at with `$preview <event>`
(`UI/Events/EventPreview.cpp`; `$preview` lists them, `$preview off` ends one). It fills a window
through the setters its packets use and lets it draw off its map; the window sends nothing. Teleporting
a game master to the map passes the map check but brings no event state, and the GM move does not
create the siege minimap that a map join does. What the previews showed, in both themes unless noted:

- **Illusion Temple result**: the original's columns are too narrow for the English strings, and
  legacy keeps them; modern lays out its own table.
- **Illusion Temple HUD**: its corner part (time, mini map, skill panel) is `#corner` in
  `cursed_temple_system.rml`; a theme moves it with `left`/`top`, and C++ reads the offset back to
  place its markers, digits, buttons and hit tests. Modern moves it left of the worn-equipment icons.
  Its hover tooltips did not show in the preview at either position.
- **Siege commander HUD**: the member and NPC dots the preview sends fall outside the part of the map
  it shows, so it is unconfirmed whether they draw where native drew them.

The Blood Castle and Chaos Castle timers, the duel spectator list, the CryWolf result and the HUD
status texts (`$preview status`; the crown switch lines show only at the switches) and the event
result and progress boxes (`$preview bcresult`, `ccresult`, `dsrank`, `switchbox`) and the guild war
time and result (`$preview guildwar`) look right. `$preview kanturu` fills the Kanturu entry window
and `$preview notices` the centre-screen notices; `$dialog menu <name>` opens an NPC menu without its
NPC.
The Battle Soccer score, the duel frame, the Empire Guardian timer and the Doppelganger frame draw
outside their event (`$win soccer full`, `duel`, `empiretimer`, `doppelframe`) and were checked in
both themes. Everything else in the rollout was verified in game, both themes, with the scale sweep.

### Accepted as it stands, with its trigger

- **`MiniMap`**: no reference-px space exists; the art is turned 45° in physical px.
- **A new siege command's pulse**: `rgb(255, pulse, pulse)` welds the theme's red to a per-frame
  sine. RCSS cannot mix a bound fraction into a colour, so this needs a mechanism that does not
  exist. The guard deliberately does not cover `color`.
- **`CCryWolf`'s sprites**: `SyncResult()` and `SyncHud()` choose the image files and texel
  rectangles (the 12x12 altar art, the 15x19 experience digits), and `crywolf.rml` binds `src` and
  `rect` from them. A theme can hide or rearrange that art, not replace it. Trigger: a theme wanting
  other event art, which means naming the sprite states in the model and leaving files and rects to
  the theme.
- **The modern options screen below 380dp tall** scrolls its page, and an open dropdown can be
  clipped by the pane. Reached only with a large UI scale in a small window. Trigger: a dropdown
  that opens outside the pane.

### Deliberately not on this list

The rest of the allowlist, reviewed entry by entry: per-frame data (gauges, things that follow the
pointer or scroll, windows that grow with their content, projected markers) and text measured the
way the native renderer measured it. A list the server does not bound is not a reason: it flows in
RCSS, as the duel spectators and score marks do.
`root_x`/`root_y` and `panel_x`/`panel_y` stay apart: the first is the physical origin of a root
scaled uniformly by `root_scale`, the second a reference-unit position inside a stretched `.screen`.

`.sharp-text` counter-scaled tops, `MiniMap`/`WorldLabelLayer` marker coordinates, and
`TitleSceneUI`'s loading bar are justified hybrids with recorded constraints. The loading bar is
pushed as real `px` because the scene's background is still native sprites on an 800x600-reference
per-axis scale `dp` cannot reproduce; it ends when those sprites port, and nothing links the two.

### The guard that freezes the population

`tools/check_rml_bound_geometry.py` runs in the build beside the syntax and contract-drift checks.
It requires a reason in `tools/rml_bound_geometry_allowlist.txt` for every non-exempt geometry
binding; root placement and scaling expressions are exempt. `--review` compares each reason with the
fields it actually binds: an entry can still be required after its description has gone stale, so
update the reason when a port changes what a document binds. A custom property bound with a length
unit counts as geometry too (the theme's `calc()` makes it a box); the native text metrics
(`--text-px`, `--line-px`, `--line-height`, ...) are exempt as `text_px` is. Each entry is tagged `state` (the geometry is the
data) or `debt` (layout C++ owns that a theme should); `--review` ends with the debt count, which
only goes down.

Allowlist totals are not defect counts, and passing the guard establishes documented exceptions, not
runtime correctness. The guard covers the four box offsets and the two sizes only: a bound `color`,
`decorator` or `font-size` has the same override problem but is a judgement call per case, while a
bound static coordinate is nearly always layout that belongs in RCSS. Widen it when a bound colour
bites.

## Modern theme studies

**HUD circular glass orbs and wrapping arc gauges.** The modern theme retinted `main_frame.rcss`'s
rectangular HP/MP/AG/SD bars rather than rebuilding them as orbs; that is new markup, a new
`CMainFrameWindow` binding shape and new tooltip anchors on live combat UI. Two techniques are
confirmed viable: `<progress direction="clockwise">` for the arcs (needs a `fill-image`,
`engine-findings.md`) and layered `radial-gradient` for the orb liquid. Trigger: a pass scoped to it,
once prioritized; low priority.
