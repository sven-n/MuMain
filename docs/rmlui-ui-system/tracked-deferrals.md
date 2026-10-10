# Tracked Deferrals

Open work and accepted constraints, each with the trigger that brings it back. Completed migrations
belong in [migration-ledger.md](migration-ledger.md); engine behaviour in
[engine-findings.md](engine-findings.md). An entry leaves this file when it is done, and its outcome
is recorded where the code it changed is described.

## Relevance review — 2026-10-10

Reviewed against local `87d1773be` on `dev/rmlui-ui-system`. This was a source and guard audit;
the visual observations below are from the earlier in-game pass, not a new runtime verification.
At that review, the three reported layout defects still had supporting code, and none could be
closed from source inspection. Rollout 1 is now completed following the user's in-game confirmation;
the native text floor and accepted constraints still apply. The stale parts were the merge-relative schedule,
the implication that the contract-drift guard still covers these windows, and the assumption that
HUD orbs necessarily need a new C++ model shape.

Paths below are relative to `src/bin/Data/Interface/RmlUi/` for assets and `src/source/` for C++.

| Item | Current disposition | Evidence and next action |
|---|---|---|
| Native text floor | Open defect; shared policy needs care | `UI/Scaling/UITransform.cpp` keeps the normal role's minimum at 11 and clamps fitting to the role minimum. `UI/RmlBridge/RmlNativeTextFit.cpp` uses that same floor; adding `.native-fit` alone cannot fix text already at the minimum. Address in rollout 2. |
| Illusion Temple result columns | Retained legacy limitation | `themes/legacy/cursed_temple_result.rcss` keeps closely spaced, absolute columns; modern has its own table layout. Revisit with a localization/readability pass; retain the legacy visual identity rather than treating narrow English columns as a requirement. |
| Illusion Temple hover tooltips | Unresolved validation / possible defect | `CCursedTempleSystem::SyncSkill()` still tests the three `ct_hint_*` boxes and calls the shared tooltip. No source evidence proves the earlier missing tooltips fixed. Diagnose in rollout 3. |
| Siege member/NPC dots | Unresolved validation | `UI/Events/EventPreview.cpp` still seeds fixed map positions. `CSiegeWarCommander::FillGuildMemberDots()` clips against the current map view; sample points outside that view cannot establish correctness. Improve the fixture and verify in rollout 3. |
| MiniMap physical-pixel geometry | Accepted constraint | `UI/HUD/MiniMapLayout.cpp` still computes rotated quads in window pixels. Keep the exception until a map transform/layout redesign is scoped. |
| Siege command pulse colour | Accepted ownership constraint | `SiegeWarCommandEntry::color` and `siege_warfare.rml` still bind the combined colour to text and images. Revisit when a theme needs another pulse palette or a suitable presentation mechanism is available. |
| CryWolf sprite selection | Accepted ownership constraint | `CCryWolf::SyncResult()` / `SyncHud()` still select files and texel rectangles bound by `crywolf.rml`. Revisit when alternate event art is requested. |
| Modern options dropdown clipping | Accepted small-viewport limitation | `themes/modern/option_window.rcss` still enables pane scrolling below 380dp; dropdowns remain absolute children of that pane. Revisit when a supported configuration makes choices inaccessible. |
| Modern HUD orbs / arc gauges | Open, low-priority design work | `themes/modern/main_frame.rcss` still draws rectangular bars. Existing fractions, readouts and hint strings in `CMainFrameWindow` can be reused; prototype in rollout 4. |

Audit checks: syntax passed (144 RML / 237 RCSS), bound geometry passed (144 RML / 37 marked),
and the active-transform guard passed. The initial drift command checked nothing. Rollout 0
subsequently restored coverage: **111 documents / 222 theme variants / 144 files checked**,
with 28 passing regression fixtures. Its outcome is recorded in
[migration-ledger.md](migration-ledger.md#contract-validation). A clean geometry report validates
recorded exceptions, not their runtime behaviour.

## Reported layout defects

- **Native text floor at small scales.** Normal-role native-size text bottoms out at approximately
  11 times the OS display scale in screen pixels (font-cache rounding applies; other roles have
  their own minima), so at 75 % on 1024x768 fixed-length lines run past small docked windows
  (Blood Castle and Devil Square entry, MU Helper, the shop warning). Let the text follow the panel
  scale down, or fit those lines to their boxes.

Waiting on their own triggers: the [accepted constraints](#accepted-as-it-stands-with-its-trigger)
and the [modern HUD orbs](#modern-theme-studies).

Throughout: run targeted scale, theme and interaction checks with each change. Modern remains the
architecture proof theme; this plan covers the named defects and constraints, not a redesign of
every modern window. The `CObject` tier is accepted as the base
([building-new-ui.md](building-new-ui.md)'s "Accepted as the base"), and window placement is
theme-owned ([window-placement.md](window-placement.md)).

## The C++ ↔ RML/RCSS ownership boundary

The ownership rollout moved layout out of C++ into the themes. What is left: the reported defects,
unresolved event validation and constraints accepted with a trigger. For new UI, follow
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

## Rollout plan

Use one focused change per concern. The sequence below is the recommended order, without tying
work to an unspecified merge date. Rollouts 1–3 address defects and missing evidence; rollout 4
is optional design work. Accepted constraints enter implementation only when their triggers fire.

### 0. Restore meaningful contract checks — completed

The guard covers the current literal document declarations, reusable views and per-instance
models. Fixtures prove missing required ids/callbacks in one fork fail, shared/template fallback
works, alternative readouts pass, and exceptions reject staleness. Minimal missing modern NPC and
quest hooks were restored; unused model bindings were removed and deliberate control omissions
carry reasons. The command also rejects missing
assets, undiscovered literal document ownership and typed models with no discovered bindings.

This completes the static-check prerequisite. Dynamic ids, selector lookups, struct-member
contracts and arbitrary registration helpers still require manual review/runtime checks; passing
the guard does not close any remaining layout or interaction deferral.

### 1. Fix party bounds and legacy trade spacing — completed

Implemented and confirmed working in game by the user on 2026-10-10. The two deferrals are
closed; implementation, build and automated coverage are recorded in
[migration-ledger.md](migration-ledger.md#party-and-trade-layout-follow-up). The confirmation
does not establish coverage of the entire release matrix below.

### 2. Make small-scale text fit without a global readability regression

Planning review: 2026-10-10. Scope is the four reported groups below; the shop group covers both
personal-shop views. The ordinary native text minimum remains unchanged. Preserve the existing
legacy art and behaviours while letting each theme choose its own text layout.

Step 1 completed on 2026-10-10: the headless baseline covers 570 scenarios and reproduces
layout defects in all five windows. Results and the rerun command are recorded in
[migration-ledger.md](migration-ledger.md#small-scale-text-baseline).
Step 2 implemented on 2026-10-10: both event entry windows now wrap complete descriptions in
bounded scroll panes, with wrapping level labels and a separate scrollable level list when
needed. The native font floor is unchanged. Automated event geometry passes, and the in-game
checks were confirmed on 2026-10-10. See [migration-ledger.md](migration-ledger.md#event-entry-text-follow-up).
Step 3 implemented on 2026-10-10: both personal-shop views flow their notices in a bounded,
scrollable pane; only the legacy title's compact fit remains for step 5. See
[migration-ledger.md](migration-ledger.md#personal-shop-notice-follow-up).

| Group | Initial failing cases (before its layout fix) | Change / remaining plan |
|---|---|---|
| Blood Castle entry | Character-count splitting, fitting against fixed 72/190-unit boxes, and absolute description/button rows caused overflow and overlap. | Implemented: complete paragraph in a bounded wrapping pane, wrapping title and level labels, and scrollable level list when needed. Confirmed in game. |
| Devil Square entry | Six translated fragments occupied fixed rows in `event_entry.rcss` / `devil_square_enter.rcss`. | Implemented: all six fragments flow in order inside the description pane. Enabled/locked bands, entry requests and exit actions are preserved. Confirmed in game. |
| MU Helper config | Legacy binds native-size text on narrow labels; both themes use tightly packed checkbox, numeric-field and Setting-button positions. | Give labels explicit available widths and reflow the affected rows in all three tabs. Verify class-dependent layouts, including Summoner recovery and Dark Lord raven controls. |
| Personal-shop warnings | Legacy `my_shop` and `purchase_shop` use fixed notice rows with `white-space: nowrap` and `overflow: hidden`. Modern has its own typography but retains fixed warning offsets. | Implemented: each row wraps in native's three groups inside one bounded, scrollable pane shared by both views (`shop_notice.rcss`). Warning emphasis, grids, name entry and controls kept. The legacy title still touches the close target in de/es/ru at 1.5x/2x (step 5). |

Implementation order, with one focused change per concern:

1. **Establish failing cases — completed.** Load the real assets, bundled fonts and resource strings in
   headless RmlUi at 1024x768 / 75%. Cover 1x, 1.5x and 2x OS display scale and record each failing
   element, its drawn text bounds, available width/height and neighbouring control. Include
   English and longer German, Spanish, Polish and Russian strings. Record the current normal
   font floor so a layout fix cannot silently change it.
2. **Use the event pair as the pilot — completed.** Replace character-count/fixed-row presentation with
   theme-owned flow. Remove any replaced per-line fitting fields, bindings and measurements;
   retain native-size compatibility inputs where used. A paragraph must wrap as one text layer,
   with physical-pixel widths where counter-scaling is used. Account for vertical space before
   the first level button; additional lines must not paint over buttons. If readable content
   cannot fit, use a bounded scrollable description region with visible scroll affordance.
   The pilot also wraps level labels and scrolls their list when needed; reducing the native
   minimum or extending `.native-fit` was unnecessary. The modern footer now has a visible exit
   button using the shared modern icon-button styling.
3. **Fix personal-shop notices — implemented.** Apply the flow pattern to both `my_shop` and `purchase_shop`.
   Reserve room between the item grid/status and bottom controls, remove notice clipping, and
   keep every warning and Zen-only statement reachable. Use a bounded scroll pane only where
   the full text cannot fit at the retained readable size.
4. **Fix MU Helper labels.** Adjust label/control relationships and wrapping first, covering
   Hunting, Obtaining and Other tabs and class-specific hidden controls. Keep numeric fields,
   skill slots, item-name entry and footer buttons aligned and clickable. Preserve save/reset
   and class-feature behaviour.
5. **Add bounded fitting only for demonstrated remaining compact labels.** Extend the existing
   `.native-fit` pass if a title or button label still cannot fit after layout changes. Opt-in
   belongs to the theme; expose an explicit minimum through a parsed RCSS property, defaulting
   to the existing role minimum. Choose the reduced minimum from the failing fixtures and
   readability checks, rather than changing the global floor. Resolve it relative to the native
   text size so OS display scale is applied once. Keep fitting single-line; longer prose uses
   wrapping or scrolling. The pass reads the actual content box, never a new C++ window width
   constant, never enlarges text, and restores inherited sizing when fitting is unnecessary.
   Preserve existing `NativeTextSize` capability gating and avoid theme-name branches.
6. **Validate and build.** Extend `test_ui_scaling.cpp` if minimum/fitting calculations change;
   add RmlUi coverage for real document text bounds, neighbouring controls and any new fitting
   property. Test unchanged default floors, opt-in limits, font rounding, unchanged-input cache
   reuse, and invalidation after text, width, inherited size, font/style or minimum changes.
   Check document removal and theme reload for stale cached elements. Run syntax, contract,
   bound-geometry and active-transform guards, the contract fixtures and rollout 1 regressions;
   build RelWithDebInfo x64 with runtime assets staged.

Acceptance: full text is readable and contained, or accessible through the explicitly bounded
text pane, at 1024x768 / 75% at 1x, 1.5x and 2x OS display scale. Check 100/125/150% at
1280x720 and 1920x1080 in both themes. No label may obscure another label, field or control;
ordinary native-size text retains its existing minimum. New scroll panes must expose all text
and keep buttons reachable. Headless results establish geometry, not visual readability.

In-game checks before closure:

- Event descriptions and title/button labels; enabled and locked level bands, entry actions,
  exit/Escape and any description scrolling.
- Every MU Helper tab; relevant class variants, checkboxes, numeric entry, skill picker,
  item add/remove, Save and Initialization.
- Seller and buyer shop warnings, warning emphasis, name entry, open/close and purchase flow;
  item-grid corner clicks and matching tooltip positions.
- UI scale/resolution changes and theme changes while each window is open; party/trade regression
  checks. Record the actual configuration and outcome, including any remaining fit limit.

Record implementation and build results in the ledger, leaving this deferral open until the
in-game checks are confirmed. A configuration requiring unreadably small text returns to layout
or scrolling; clipping or an unbounded font reduction does not count as a fix.

### 3. Close the event validation gaps

- **Illusion Temple:** use `$preview temple` / `$preview templeresult` to check all three hover
  zones, skill changes, moved corner anchors and tooltip cleanup. Trace the missing hints before
  choosing a fix. `IsPointerWithin()` tests geometry directly, so inherited `pointer-events`
  alone is not an established cause. Confirm actual-event behaviour as well as preview behaviour.
- **Siege:** seed deterministic member/NPC positions inside the displayed crop plus edge/outside
  cases, using the map's current pan/zoom conversion. Verify hero/member/NPC dots, command markers
  and clipping at each available zoom. Confirm a real siege join creates the same HUD; GM
  teleporting alone does not exercise that path.
- **Result columns:** reproduce the legacy English issue and record the compatibility decision.
  If a readability pass is prioritized, adjust the table in its theme and verify longer names,
  class labels, scores and both teams; otherwise retain it explicitly as a triggered limitation.

Done when tooltip and marker checks have recorded results in both themes at 75/100/150%, including
theme changes while shown and `$preview off` cleanup. If a live event cannot be exercised, leave
that acceptance step open with the preview evidence recorded; do not mark it verified from source.

### 4. Prototype and ship modern orbs when prioritized

Prototype one resource orb and one clockwise arc in modern markup, using existing fractions and
an arc `fill-image`. Check empty/partial/full values, poison appearance, readouts, hover hints,
scale and rendering cost before expanding to HP/MP/AG/SD. Keep any new visual geometry and assets
theme-owned. Recheck workspace HUD bounds if the new silhouette grows above the existing strip.

Done when live damage/recovery and resource use update correctly, tooltips follow the drawn
elements, controls still block world clicks, and the legacy HUD passes the same behaviour checks.
Validate at 1280x720, 1920x1080 and ultrawide before promoting the prototype.

### Triggered work outside the numbered rollout

| Triggered constraint | Smallest scoped follow-up | Acceptance |
|---|---|---|
| MiniMap redesign | Separate the map-to-screen transform from theme placement; keep projected marker data in C++. | Rotation, clipping, portal hit tests and markers align on 4:3, widescreen and ultrawide at changed UI scales. |
| Alternate siege pulse palette | Expose pulse state separately and choose a viable theme-owned colour/animation mechanism; widen the presentation guard only around the demonstrated need. | Legacy timing and colour remain recognizable; another palette works through theme assets/styles without a theme-name branch. |
| Alternate CryWolf art | Expose semantic altar/banner/rank/digit states and let themes select files and rects. | `$preview crywolf` / `crywolfresult` cover all seeded altar states and result art; alternate art and legacy crops work without C++ asset names. |
| Inaccessible options choices | Try bounded popup sizing/direction first; if clipping persists, use a shared outside-pane popup with theme-owned placement. | Every choice can be selected above and below 380dp; scrolling, dismissal, focus, theme changes and option persistence work in both themes. |
| Title-scene background port | Port the native background and its loading bar placement together, then remove C++-pushed bar geometry. | Bar/background alignment survives resolution/aspect changes; loading progress still presents correctly. |

### Release checks and closure

For each affected screen, capture the failing configuration before changes and repeat it after.
Run syntax, drift (with meaningful coverage), bound-geometry and active-transform checks, then the
targeted interaction tests above. Compile when C++ changes. For rollout completion, sample affected
screens at 800x600, 1280x720, 1920x1080, 2560x1440, 3440x1440 and 3840x2160 at
75/100/125/150%, adding the reported 1024x768 case. Record fit limits where a window cannot
accommodate the requested scale. Use legacy and modern (the project's existing divergent theme
pair); do not add a third first-party theme solely for this work.

Exercise resolution/scale changes while shown, theme changes while open, and dragged positions and
reset/persistence where supported. Record configuration and observed outcome, not just "scale sweep
passed". Ship focused fixes as their checks pass; each can be reverted independently. Remove a
deferral only after its acceptance checks pass, record its outcome in the existing window docs or
[migration-ledger.md](migration-ledger.md), and update any related exception marker and
[STATUS.md](STATUS.md) claim. Historical preview successes are evidence to retain in the ledger,
not newly verified results.
