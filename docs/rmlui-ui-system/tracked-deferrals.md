# Tracked Deferrals

Open work and accepted constraints, each with the trigger that brings it back. Completed migrations
belong in [migration-ledger.md](migration-ledger.md); engine behaviour in
[engine-findings.md](engine-findings.md). An entry leaves this file when it is done, and its outcome
is recorded where the code it changed is described.

## Before merging to main

No code-health item is left.

## After the merge

In order:

1. [Native 3D against RmlUi](#native-3d-against-rmlui): retire the background contexts, and with
   them the root transform and counter-scaled text they keep alive; the equipment paperdoll follows.
2. [One obvious component surface](#one-obvious-component-surface).
3. [The counter-scale block](#the-counter-scale-block): settle the open question, then move it to
   `calc()`.

Waiting on their own triggers: the [accepted constraints](#accepted-as-it-stands-with-its-trigger)
and the [modern HUD orbs](#modern-theme-studies).

Throughout: run targeted scale, theme and interaction checks with each change. The modern theme
exists to prove the architecture, so its unchecked windows are not tracked. The `CObject` tier is
accepted as the base ([building-new-ui.md](building-new-ui.md)'s "Accepted as the base"), and window
placement is theme-owned ([window-placement.md](window-placement.md)).

## Native 3D against RmlUi

The review of `Render/RmlUi` and `UI/RmlBridge` against the vendored RmlUi found the core idiomatic;
what remains is where RmlUi meets native 3D. Two mechanisms order it:

- **The `background` context**, rendered mid-frame from `CManager::Render()`. It splits 18
  windows into a foreground and a `*_bg.rml` document, each with its own model and root-transform
  sync, and holds the HUD boards that draw under them.
- **`RenderTarget`**, the idiomatic one: native drawing becomes an image at its element's depth.

The stacking table (`RmlStackingOrder.cpp`) says which context each document loads into, and
`test_rml_stacking_order.cpp` holds the background list closed, so new native 3D can only go into
a render target. `UI::Items::ItemCameraTarget` draws items into one; the confirm dialog's item
preview moved first, retiring `dialog_background`.

**Direction.** Move the inventory family's live items into render targets and retire
`background` with its `*_bg.rml` documents, taking each out of the test's list. The root transform
and its counter-scaled text (`SyncRootTransform`, `.sharp-text`, the panel readback) exist because a
window shares reference coordinates with native grids and hit tests; a window whose native content
has moved into a render target moves to `dp` and RmlUi's own hit testing.

**The equipment paperdoll** (`CMyInventory::RenderEquippedItem()`, still native). Its background
sprite, durability tint and drag-compatibility highlight paint *behind* the equipped item's live 3D
icon; ported into the main context they would paint in front of it. Port it in the same pass, once
the equipped items are render targets, or with the equipment grid's own chrome pass, whichever comes
first. Alone, only the static background sprite is worth the cost.

## The counter-scale block

A `.sharp-text` layer cancels `#panel`'s scale so glyphs rasterise sharp, so its layout width must
arrive pre-multiplied: `data-style-width="(220 * root_scale) + 'px'"`. A stylesheet cannot multiply
in this RmlUi without `calc()`, so the width travels through the data model, which is the inline
geometry the bound-geometry guard flags. Several allowlist entries share this constraint. Centred
text is the exception: `.sharp-middle` and `.sharp-centre` (`engine-findings.md`) centre a layer
without a multiplied width, and the button labels and entry-window lines use them.

**The way out.** The pinned fork carries upstream PR #983 (`calc()`, `min()`, `max()`, and `var()`
inside them), so `width: calc(220px * var(--root-scale))` with C++ setting `--root-scale` on `#panel`
would move the whole class into RCSS. Unused so far.

**Open question, to settle first:** whether a custom property set from C++ at runtime recomputes
every `calc()` that depends on it, and at what cost when it changes (on a UI-scale change, not every
frame; measure in RelWithDebInfo, borrowing the PR's `Tests/Source/Benchmarks/Calculation.cpp`).
The PR's tests cover invalidation behind `var()` for font-size changes only.

**Revisit when** a counter-scaled window is next touched: settle the question on it, then move the
class. Upstream merging #983 only removes the dependency on the fork. The ownership boundary's
`MessageBoxView` entry waits on this, and so does `CNPCQuest`: its message and answer tops stay in
its model. `message_top` is per instance (native centres the message-plus-answer block by its line
count), and legacy's separate `answers_top` exists because flow does not stack counter-scaled text
layers, so a declarative version needs another bound number or the markup duplicated per quest state.

## One obvious component surface

A developer should meet one component family, not historical header boundaries: compatibility
aliases or forwarding headers over mass renames. Done when every common UI concern has one canonical
implementation or a documented presentation-specific split, discoverable without knowing the
codebase's history. Known gaps:

- **`CInGameShop`** keeps the last native widgets alive; new code must not use them.
- **`CBuffStrip` and `CMuHelperBar` hover tooltips** are their own CSS `:hover` mechanism (plain
  text, no per-line colour), not the shared tooltip, because they live in `dp` while every other
  caller anchors in reference pixels.

## The C++ ↔ RML/RCSS ownership boundary

The ownership rollout moved layout out of C++ into the themes. What is left: windows not yet seen in
game, constraints accepted with a trigger, and the counter-scale block above. For new UI, follow
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
  its kind and the theme places them (`CGuild_ToPerson_Position`). Trigger: the counter-scale block.
- **`Notices`**: physical px with no root transform, its transform taken ambiently. Trigger: that
  HUD gaining a reference-px space.
- **`MiniMap`**: no reference-px space exists; the art is turned 45° in physical px.
- **Unbounded server-driven counts**: the duel spectator list, siege score marks. No `:nth-child`
  table covers a count the client does not bound.
- **Computed fan-outs**: `MainFrameWindow`'s zig-zag skill grid and `MuHelperSkillPicker`'s, both
  positioned from an ordinal among what the player actually has.
- **A new siege command's pulse**: `rgb(255, pulse, pulse)` welds the theme's red to a per-frame
  sine. RCSS cannot mix a bound fraction into a colour, so this needs a mechanism that does not
  exist. The guard deliberately does not cover `color`.
- **The event timers' text box** (Blood Castle, Chaos Castle, Empire Guardian): its left and width
  are `EventTimerView`'s caller constants, because the shrink-to-box text measurement needs the
  width. Trigger: a theme wanting another box, which means reading the width back off RCSS.
- **`CGenericMenuDialog`'s native frame**: each menu's caller describes the native box it replaces
  (`GenericMenuConfig::nativeFrame`), and legacy binds those heights and tops to reproduce it; modern
  flows the menu and ignores them. Trigger: a theme wanting its own layout for one menu, which
  means naming menu kinds the way `MessageBoxView` does.
- **The modern options screen below 380dp tall** scrolls its page, and an open dropdown can be
  clipped by the pane. Reached only with a large UI scale in a small window. Trigger: a dropdown
  that opens outside the pane.

### Deliberately not on this list

The rest of the allowlist, reviewed entry by entry: per-frame data (gauges, things that follow the
pointer or scroll, windows that grow with their content, projected markers), counts the server does
not bound, text measured the way the native renderer measured it, and the counter-scale block.
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
