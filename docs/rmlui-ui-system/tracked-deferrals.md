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

The Blood Castle and Chaos Castle timers, the duel spectator list and the CryWolf result look right.
The Battle Soccer score, the duel frame, the Empire Guardian timer and the Doppelganger frame draw
outside their event (`$win soccer full`, `duel`, `empiretimer`, `doppelframe`) and were checked in
both themes. Everything else in the rollout was verified in game, both themes, with the scale sweep.

### Accepted as it stands, with its trigger

- **`SiegeWarfare`'s team/command buttons**: geometry read off live native controls that also
  hit-test it. Trigger: retiring the native `CButton`s.
- **`MessageBoxView`**: every text line is centred by measurement and stacked by measured height,
  and a box's buttons come from its own `CMessageBoxButton`s. A box whose buttons were literals names
  its kind and the theme places them (`CGuild_ToPerson_Position`). Trigger: a theme wanting its own
  box layout. A line could centre with `.sharp-centre`, but counter-scaled lines do not stack in
  flow, so their heights stay measured.
- **`CNPCQuest`'s message and answer tops**: the block is centred by its line count, per quest
  state, and legacy places the answers under it with a second top for the same flow reason.
  Trigger: a theme wanting another arrangement.
- **`Notices`**: physical px with no root transform, its transform taken ambiently. Trigger: that
  HUD gaining a reference-px space.
- **`MiniMap`**: no reference-px space exists; the art is turned 45° in physical px.
- **Computed fan-outs**: `MainFrameWindow`'s zig-zag skill grid and `MuHelperSkillPicker`'s, both
  positioned from an ordinal among what the player actually has.
- **A new siege command's pulse**: `rgb(255, pulse, pulse)` welds the theme's red to a per-frame
  sine. RCSS cannot mix a bound fraction into a colour, so this needs a mechanism that does not
  exist. The guard deliberately does not cover `color`.
- **The event timers' text box** (Blood Castle, Chaos Castle, Empire Guardian): its left and width
  are `EventTimerView`'s caller constants, because the shrink-to-box text measurement needs the
  width. Trigger: a theme wanting another box, which means reading the width back off RCSS.
- **`CCryWolf`'s sprites**: `SyncResult()` and `SyncHud()` choose the image files and texel
  rectangles (the 12x12 altar art, the 15x19 experience digits), and `crywolf.rml` binds `src` and
  `rect` from them. A theme can hide or rearrange that art, not replace it. Trigger: a theme wanting
  other event art, which means naming the sprite states in the model and leaving files and rects to
  the theme.
- **`CGenericMenuDialog`'s native frame**: each menu's caller describes the native box it replaces
  (`GenericMenuConfig::nativeFrame`), and legacy binds those heights and tops to reproduce it; modern
  flows the menu and ignores them. Trigger: a theme wanting its own layout for one menu, which
  means naming menu kinds the way `MessageBoxView` does.
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
update the reason when a port changes what a document binds.

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
