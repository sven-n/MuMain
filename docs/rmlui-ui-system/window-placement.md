# Window placement (design proposal)

**Status: approved 2026-10-05. Phases 0–3 implemented; phases 1–3 await in-game checks.**
Supersedes the "dock spacing" item in [tracked-deferrals.md](tracked-deferrals.md).

## Goal

A theme, or a mod of one, decides where every window goes, how big it is, and how windows arrange
around each other, without C++ changes (principles §15–16). The design must cover today's docked
columns and much more:

- a window docked on either side, or not docked at all (floating, centred, free);
- windows that sit beside each other (character info next to the inventory) in one theme and apart
  in another;
- a window that covers the whole screen or half of it;
- anything else ordinary RCSS can express.

## What C++ decides today

| Decision | Where |
|---|---|
| Whether a window docks, and on which side (`DockRight`, `DockLeft`, `Dialog`, `FloatingWorkspace`) | `UILayoutPolicy.cpp`, a fixed table that overrides anything the window sets |
| Which column a docked window takes | `PanelColumnX(n)` (fixed 190-wide columns from 640) passed to each `Create()` in `WindowSystem.cpp` |
| How windows rearrange around each other | `CSystem::Show()`/`Hide()`: e.g. opening the inventory while character info is open moves it to column 2; opening the extension pushes vault/shop/mix/trade to column 3; closing character info moves them back |
| What closes to make room | `Show()`: opening the inventory closes the quest log; `HideGroupBeforeOpenInterface()`; the MU Helper bar hides when three columns are open |
| Where docks sit vertically | `DockTransform()` anchors them on `RoundedBottomHudTop()`, the original 51-unit HUD height, so they ignore a theme that moves the HUD |
| HUD widgets that dodge open windows | `SetPos(screenWidth)` on the buff strip, item endurance and party list after every open/close |
| Saved user positions | Only the inventory (`GameConfig::Get/SetWindowPosition`) |

Native code also depends on these positions: `CManager` remaps the mouse through each window's
layout transform for hit-testing, inventory-family grids sit at fixed offsets from the window's
`m_Pos` (`CInventoryCtrl` at `+15, +200`), and live 3D content is drawn at native coordinates.

## Design

### 1. Placement is a theme document: the workspace

Each theme ships `workspace.rml` with its RCSS. It holds one **slot** per placeable window, inside
**regions** the theme defines with ordinary RCSS:

```html
<body id="workspace">
  <div class="region dock-right">            <!-- flex row, anchored right -->
    <div class="slot" data-window="character"/>
    <div class="slot" data-window="inventory"/>
    <div class="slot" data-window="inventory_ext"/>
  </div>
  <div class="region dock-left">
    <div class="slot" data-window="move_map"/>
  </div>
  <div class="region center">
    <div class="slot" data-window="npc_dialogue"/>
  </div>
  <div class="region free">                  <!-- absolutely positioned slots -->
    <div class="slot" data-window="friends"/>
  </div>
</body>
```

- A slot is shown only while its window is open, so open windows pack in a region by flex layout.
  Order within a region is DOM order (this RmlUi build has no `order` property).
- The examples in the goal are all plain markup and RCSS: move the character slot into
  `dock-left`, put the character and inventory slots in different regions, give a slot
  `width: 100%; height: 100%` or `50%`.
- A mod changes placement by overriding `workspace.rml`/its RCSS per theme, like any themed
  document.
- A window without a slot keeps today's behaviour, so migration is incremental.

### 2. Slot sizing: content or fill

- **`data-fit="content"`** (default): the slot takes the window's own panel size times the region's
  scale. C++ reads the window's `#panel` box through `RefreshLogicalPanelSize()`, finding the
  document by the file name the window registers; windows drawn through a shared entry view (the
  Gold Bowman, Doppelganger, Empire Guardian and Lucky Coin windows) have none and take the docked
  windows' 190x429. Pixel-art panels authored at a reference size stay exactly as they are.
- **`data-fit="fill"`**: the window takes the slot's size. Its document receives the slot's width
  and height and lays itself out fluidly. Only windows that support it can be placed this way
  (section 5); otherwise the service logs a warning and uses `content`.

No `calc()` is needed: content sizes come from the window's `#panel`, fill sizes from `%`,
`vw`/`vh` and `dp`, and gaps from margins and padding. This RmlUi build has no `calc()`, and the
design does not rely on one, so no RmlUi fork is required.

Character info is the first fill-capable window. A theme can move its slot into a full-height
region, remove that region's `data-ref-height`, and give the slot a percentage width and full
height. For example, inside `#safe_area`:

```html
<div class="region side-panel" data-scale="panel" data-covers-world>
  <div class="slot character-fill" data-window="character" data-fit="fill"></div>
</div>
```

```css
.side-panel { left: 0; right: 0; top: 0; bottom: 0; display: flex; }
.character-fill { width: 35%; height: 100%; }
```

The service passes the resolved slot size to character info; its panel fills that rectangle and
its native hit box follows the panel. A fill slot never gets smaller than the window's content
size: the service gives the slot that size as its `min-width`/`min-height`, so a percentage that
gets too small at a high UI scale or in a small game window still shows the whole panel. A theme
that wants a smaller minimum makes `#panel` smaller. A window without fill support falls back to
its content size and logs a warning. The theme still owns the inner RCSS: moving or stretching its content
for a very wide or narrow panel is a theme decision. The shipped themes leave their workspace
slots at content size, preserving the current layout.

### 3. C++ role: a placement service

On a change (open, close, resize, UI scale, theme switch, HUD move), not every frame:

1. show or hide each slot and write content-fit sizes;
2. lay out the workspace document;
3. read each visible slot's resolved rectangle;
4. give each window its rectangle.

A placed window's layout mode becomes `LayoutMode::Slot`: its logical space is its slot, with
(0, 0) at the slot's top-left and the region's scale (`CObject::PlaceInSlot()`), and its `m_Pos`
is (0, 0) unless a saved drag position applies. Everything that maps a window's coordinates reads
`CObject::GetLayoutTransform()`, so `CManager` hit-testing, grids at fixed offsets, 3D icons and
`SyncRootTransform()`'s `root_x`/`root_y`/`root_scale` follow the slot unchanged. Migrating a
window means registering it (window, slot name, `SetPos()`, document) and deleting its
`PanelColumnX`/`SetPos` lines, not rewriting it. A registered window the workspace gives no slot
returns to its policy mode. A fill window would also take its slot's size (phase 4).

The service runs synchronously after every `CSystem::Show()`/`Hide()` (`Arrange()`), so a newly
shown window never draws or hit-tests a frame at a stale position. Passive changes (screen size,
UI scale, a theme reload, a HUD part shown or hidden) mark it stale (`Invalidate()`), and
`Update()` re-places once before the frame's windows update. The workspace
document is never shown: it is never drawn or hit, and the service lays it out itself.

Implemented in `UI/Placement/WindowPlacement.{h,cpp}`; the window registry is at the end of
`CSystem::LoadMainSceneInterface()`. Inputs the game sets on the workspace: content slot sizes
and region heights from `data-ref-height`. A `data-window` or
`data-closes` name no window answers to is logged once per workspace (`[Placement]` in
MuError.log).

**Reading geometry back.** The main-frame rollout's rule was that C++ never reads RCSS geometry to
draw or place chrome. That rule stays for chrome. This design adds one narrow, named exception
that the minimap also uses (`CMiniMap::SyncClips()` reads the `main_hud` slot): C++ reads the
resolved rectangles of a document whose only job is placement, and hands them to other documents
and native code. C++ still never computes a layout itself.

### 4. HUD reserve

`#safe_area` is the content area of the workspace's shell: what the `reserve` header, footer and
side regions leave (see "HUD in the workspace"). The main HUD is a `main_hud` slot in the
footer, sized 640x51 dp (the original strip and EXP bar) by each theme's `main_frame.rcss`, so
docks sit on it as before and follow a theme that moves it. While the HUD is hidden its slot
collapses and the content area reaches the screen edge.

### 4a. Uncovered world area

`GetScreenWidth()` (`ZzzInventory.cpp`) is a third hard-coded copy of the column layout: a table
of which window combinations cover one, two or three 190-wide columns. Gameplay presentation
reads it: the hero's screen centre for facing the mouse (`HeroX` in `ZzzInterface.cpp`), pet HP
bars, personal-shop titles, the macro cooldown bar, endurance tooltips, and the terrain viewport
outside the main scene. `GetScreenWidth()` now returns the placement service's uncovered right
edge (`UI::Placement::UncoveredWorldRight()`): the leftmost open slot in a region marked
`data-covers-world`, converted to the 640-wide HUD space. A theme can leave a full-screen
overlay region unmarked, so nothing shifts under it.

At 4:3 and 100 % this equals the old table. Elsewhere it follows the docks' real edge, where the
old table assumed 190 units per column. `GetScreenLeft()` is the matching left edge; centred HUD
text (the macro cooldown, timers) centres between the two, so a left dock moves it too.

HUD widgets read the edges in their own layout every frame
(`UI::Placement::UncoveredWorldLeftIn()`/`RightIn()`), so they also follow a resize, UI-scale or
HUD change: the durability icons and party list right-align in their dock units, and the buff
row centres between both edges. They need no slots.

`HeroX` (the point the hero's head turns from) still uses it. The world viewport no longer
shrinks for open windows, so the hero stays at the screen centre; that mismatch predates this
work and is left for a gameplay pass.

### 5. What a window must support for each fit

- **`content`**: every window, including today's docked and inventory families.
- **`fill`**: the window's clickable areas and native content must follow its RCSS:
  - hit-testing by RmlUi hover, as the HUD's `IsMouseOverHud()` does, or by
    `RefreshLogicalPanelSize()`;
  - live 3D drawn into a `RenderTarget` the document shows (component catalog, "Native content
    inside a document") instead of at `m_Pos` plus fixed offsets.

  Fill is the theme's decision for every window that can take it: a window drawn wholly by
  RmlUi whose hit box reads its `#panel` returns that document from `CObject::GetFillDocument()`,
  and `CObject` sizes the panel (the placement service re-applies it after a theme switch). 24
  windows do: the docked RmlUi windows (character, pet, party, guild info and make, command and
  command list, NPC dialogue, quest log and progress, MU Helper config and detail, castle, guard,
  gatekeeper, gate switch, catapult, duel watch, Gens ranking, Blood Castle and Devil Square entry)
  and the centred NPC panels. How the content uses the extra room is the theme's RCSS; a theme
  that has not made a window's content fluid still gets a larger frame with the content at the
  top-left. The native corner close (`HandleFrameCornerClose()`) reads the panel's width, so it
  stays on the corner of a filled panel. Character info is the first fill-capable window: its
  frame follows the filled panel and its actions follow the lower edge. Pet info is the second:
  its title centres on the panel and its group boxes reach the right edge. A window opts in with
  `UI::RmlBridge::FillPlacementSize` (store the slot's size, apply it to `#panel`) and a top-right
  `#frame_corner_close` element (shared `docked_panel_frame.rcss` rule) instead of the native
  fixed-offset `HandleFrameCornerClose()`, which ten other windows still use.
  The move map is the third: it takes a fill slot's height as the space its rows fill, and its
  width from `#panel`, so a theme can dock it top-left, centre-left or as a full-height column. A
  window can name a smaller fill minimum than its content size (`GetFillMinimumSize()`); the move
  map's is its chrome and three rows. Windows with live 3D or
  native item grids need more work: inventory family (`CInventoryCtrl` in inventory, extension,
  vault, mix, trade, NPC/personal shops, lucky item), NPC quest, Doppelganger, Empire Guardian
  NPC, United Marketplace, duel watch, skill list.

### 6. Arrangement rules: game rules stay in C++, space rules move to the theme

Every rule in `CSystem::Show()`/`Hide()` falls into one of three kinds (full list in
[Phase 0: rule classification](#phase-0-rule-classification)):

- **Placement**: where an open window goes. Moves to the workspace layout.
- **Space exclusion**: a window closes only because there is no room. Moves to the theme: a
  slot's `data-closes` lists the windows that close when its window opens. Closes run through the
  normal `CSystem::Hide()`, as if the player closed the window. Names in `data-closes` are slot
  names; a window without a slot can be registered by name only (Gens ranking, the MU Helper skill
  picker).
- **Game rule**: windows that cannot coexist for gameplay or server reasons, and companion windows
  that only work together. Stays in C++; the theme still decides where companions go.

The legacy theme declares exactly today's behaviour; other themes may relax space rules.

### 7. User placement and saved positions

Decided with the user (2026-10-05): the player drags dialogs and the friend system only, for
now, and no position is saved. Docked windows stay in their slots; dragging a window out of its
slot (`data-draggable`) is not planned. The friend list, chat rooms and letters drag through
`UI::RmlBridge::MakeDraggable()` and bounce back on screen; the friend list asks its slot only
where it first opens. The options window, the generic menu dialog (the system menu among them)
and the generic confirm dialog drag by any part that is not a control (base.rcss gives
`input, select, textarea, .btn, [data-event-click]` `drag: block`), stay where they were dropped
for the session, and move back inside the window if dropped partly outside
(`KeepInsideWindow()`). A menu or confirmation opens centred each time
(`ResetDraggedPosition()`); the confirm dialog's chrome documents follow its dragged panel. The
native message boxes (`CMessageBoxBase` family) are not draggable: their parts are placed
natively and each class would need checking.

The inventory keeps today's behaviour: `data-saved-position` names the saved drag
position, which is used while the window is the first open window of its region. Behind another
window (character info open) it takes its slot, and its slot still occupies the region either
way, so the shops sit beside it as before.

### 8. Scale

Content-fit windows keep reference-unit panels scaled by their region. A region's `data-scale`
picks the scale: `dock` (the default; the original docks, capped at 2.25×), `panel` (centred
panels, 2.0×) or `hud` (the HUD strip, 2.0×). Fill windows would use `dp`.

### 9. What `LayoutMode` becomes

Windows with a slot use `Slot` while placed (done for the 39 right-docked windows). `Legacy`
(scene windows that work in real pixels) and `WorldOverlay` (world-anchored balloons and names)
stay. The `Hud*`, `Dock*`, `FloatingWorkspace` and `Dialog` rows of `UILayoutPolicy.cpp` remain
for windows without slots and shrink as more windows gain them. `DockTransform()`'s fixed HUD
height now only serves unslotted docked windows and the saved-position conversion.

## Theme recipes (verified in game)

Tried against the legacy theme by editing only its workspace (2026-10-05, 1024x768, UI scale 90 %).
Each worked, including clicks inside moved windows.

- **Split docks.** Move the character-side slots (`character`, `my_quest`, `pet`,
  `quest_progress_etc`) into a second region styled like `.dock-right` with `flex-direction: row`
  and no `right`. Character info and the quest log dock on the left while the inventory family
  stays right. Drop the cross-side `data-closes` (inventory/quest log, extension/character), since
  the two sides no longer compete for room.
- **Centre stage.** Put the inventory family in a region with `display: flex;
  justify-content: center; align-items: center` filling `#safe_area`. The inventory, and the
  inventory with its extension or a shop, centre as a group. Remove `data-saved-position` from
  the inventory slot, or the saved drag position wins while it is first in its region.
- **Composed row.** Regions are independent, so a centred region overlaps a dock on a narrow
  screen. Making `#safe_area` a flex row of left dock, centre (`flex: 1 1 auto`) and right dock,
  with the regions `position: relative`, centres windows in the space the docks leave. RmlUi has
  no `order` property, so the regions must be in that order in the markup (a fork of
  `workspace.rml`).

What these exposed:
- The uncovered world was one-sided; open slots now narrow it from whichever side of the screen
  they are on (`UncoveredWorldLeft()`/`UncoveredWorldRight()`), and the buff row and centred HUD
  text follow both edges. The hero's facing centre (`HeroX`) still uses the right edge only.
- HUD parts stay where `main_frame.rcss` puts them; a left dock covers the chat log, which a
  theme using one would move.

HUD recipes (H4, 2026-10-05, modern theme at 100 %, edits to runtime copies only; script
`.ai-os/scratch/h4_variant.py`):
- **Side bar.** Move the `top_bar` slot from `#shell_header` into `#shell_right` and lay the
  buttons out in a column in `main_frame_top.rcss` (`#buttons_top` 56x124 dp, each button's
  `top`). The right region reserves its width: docks pack against the bar, and the uncovered
  world, the item durability icons and the item buttons move left of it.
- **Chat on the right.** `.chat-stack { left: auto; right: 0; align-items: flex-end; }`: log and
  input stand on the HUD in the bottom-right corner; typing and sending work.
- **Event HUDs in the screen's corner.** `.event-hud { left: auto; right: 0; }`: on a wide
  screen (1920x1080) the timers sit in the right corner instead of at the 640-unit frame's.
- **HUD at the top.** Move the `main_hud` slot into `#shell_header` (H1, both themes): the docks
  and the chat follow the content area down to the screen's bottom edge.

What these exposed: the side of the world a docked window covers was decided by the window's
centre against the screen's, so with docks pushed left by the side bar the inventory counted
as a left-side window and the durability icons drew over it. A covering region's open slots
now count from the content-area edge they pack against (`UpdateUncoveredArea()`).

Not done, by decision: a split main HUD (the main HUD stays one unit) and a minimap in a corner
(this game's minimap is the full-screen map).

## Theme-sized windows: audit (2026-10-05)

Goal: a window's size is the theme's choice, like its place. A slot already takes the window's
`#panel` size; what still fixes the size is inside the windows. Five things do:

- **Chrome at fixed pixels.** The docked frame (`docked_panel_frame.rcss`, 25 windows) places its
  sprite pieces absolutely (`.frame-right { left: 169px }`, edges `height: 320px`) instead of
  anchoring them to `#panel`'s edges; the inventory family's own chrome does the same. RCSS only.
- **Size literals in RML.** 28 documents write 190 or 429, mostly counter-scaled text layers
  (`data-style-width="(190 * root_scale) + 'px'"`). The eight inventory-family legacy-theme documents now read `panel_width` from `#panel`; other documents remain in the audit.
- **Fixed hit boxes.** Eight windows hit-test against C++ constants instead of their `#panel`
  (`GUILDINFO_WIDTH`, `COMMAND_WINDOW_WIDTH`, `ENTERBC_BASE_WINDOW_WIDTH`, …), so a larger panel
  would not take clicks outside the old box. Switch them to `RefreshLogicalPanelSize()` (the other
  windows already use it) or RmlUi hover.
- **Native parts at fixed offsets.** Native controls and drawing at `m_Pos + constant`: tab hit
  areas, radio buttons, icons, and the inventory's equipment slots (26 offsets) and item grid
  (`CInventoryCtrl` at `+15, +200`). These need positions from RCSS anchors
  (`RefreshLogicalAnchorPosition()`, already used by the quest windows), or native content drawn
  into an element (`RenderTarget`).
- **No document to measure** (corrected): the six shared-view windows each load their own document and now register it.

| Tier | Windows | Work |
|---|---|---|
| 1. RCSS only | character, party, pet, NPC dialogue, gate switch, MU Helper config | Done: the docked frame and the modern bottom button row pin to `#panel`'s edges; counter-scaled leaves take `panel_width` (`SyncPanelWidth()`). Verified in game by widening character info to 260 in a runtime edit. |
| 2. Plus hit box | guild info, guild make, command, command list, Blood Castle and Devil Square entry, catapult, lucky coin registration, lucky item | Done: hit boxes read `#panel` (`RefreshLogicalPanelSize()`, or the entry views' `PanelSize()`/`RefreshPanelSize()`), including catapult, and `panel_width` replaces the 190 literals. Lucky item's panel and background sizes come from each theme's RCSS. Full RelWithDebInfo build passed; catapult and lucky item still need in-game checks. |
| 3. Plus anchors | quest progress (and etc), quest log, NPC quest, castle, guard, gatekeeper, duel watch, MU Helper detail (gauge hit areas), United Marketplace | Quest windows and NPC quest already read anchors; duel watch and United Marketplace have no fixed native parts. MU Helper detail done: gauges hit-test at the bar the theme draws, at its width. Done in code: gatekeeper's public toggle, guard tabs, castle tabs, and castle gate/statue picks use RmlUi click targets. Both themes place the tab and map-icon targets in RCSS. The full RelWithDebInfo build passes; siege NPC checks remain pending. |
| 4. Native grids | inventory, extension, vault (and extension), NPC shop, mix, trade, personal shops | Done: invisible `.native-anchor` boxes in each theme's markup place the inventory's 12 equipment slots (rectangles), every item grid's first cell (`CInventoryCtrl::FollowAnchor()`, each frame) and the item-option tooltip. Cell size stays native. Verified in game by moving the inventory's grid and helm slot in a runtime edit. The eight inventory-family legacy-theme documents now center counter-scaled headings with `panel_width`; full RelWithDebInfo build and RML checks passed, with in-game checks pending. |
| 5. Shared views | Gold Bowman (both), Doppelganger and Empire Guardian entry, lucky coin exchange | Each view instance does load its own document; they now register it, so their slots take the theme's `#panel` size. Native parts still need anchors. |

Counts come from a scan of each window's `.cpp` and RML (`RefreshLogicalPanelSize`/
`RefreshLogicalAnchorPosition` use, `CInventoryCtrl`, 3D rendering, `m_Pos.x/y + n` offsets, 190/429
literals). Fill placement (phase 4) needs the same groundwork.

## HUD in the workspace (done)

H1 status (2026-10-05): done. Shell regions, the participant adapter, the main HUD as a
footer `reserve` slot and the minimap's clip from that slot are in both themes; both look as
before. Verified in game at 1024x768: modern at 100 % (docks, the Kanturu panel on the
whole-screen stage), legacy at 90 % (docks), and a runtime layout with the HUD in a `reserve`
header (docks take the content area below it). H2's first batch (header and capped docks) is
done; the rest of H2 and H3-H4 have not started.

Before H1, the HUD laid itself out (`main_frame.rml` and the other HUD documents) and the workspace
learned only one thing from it: the strip's rectangle (`CMainFrameWindow::GetStripRect()`), which
sets `#safe_area`'s top or bottom (section 4). So windows avoid one edge of one strip; a side HUD,
a header and footer, or a window docked beside a HUD part cannot be expressed.

Decision: HUD components become participants in the workspace's layout model, like windows. The
workspace is the one place a theme arranges headers, footers, side panels and the content area.

- A HUD component keeps its own document, controller, input handling, z-order and scale setting.
  Taking part means only that it receives a position and an available size from a slot, exactly
  the contract windows have (`LayoutMode::Slot`, `PlaceInSlot()`); its own RCSS lays out what is
  inside that box. Nothing is merged into one RML document.
- Shell regions: the workspace gains header, footer, left and right regions around a content
  region. Today's `#safe_area` becomes the content region; the docks and centred stages live in
  it. Both shipped themes reproduce today's screen at every step.
- Participation, chosen by the theme per region (an attribute such as `data-participation`):
  - `reserve`: takes space; the content region (windows, the uncovered world) avoids it. Today's
    bottom strip.
  - `overlay`: placed by the workspace, takes no space. Corner widgets, the minimap.

  The content area excludes reserved HUD regions. The uncovered area additionally accounts for
  open window docks inside it. Preserve that distinction in `UncoveredWorldLeft()`/`Right()`;
  returning the content area's edges alone would lose the existing dock adjustment. Neither
  area implicitly changes the rendered game viewport.
- Sizing order: shell regions lay out first, then the content region from what is left, then the
  windows in it. A HUD slot is content-sized (the component's measured `#panel` or root box, as a
  window's) or fill (a full-width header), chosen per slot, so a size never depends on itself.
- Scale: HUD regions use `data-scale="hud"` (section 8); a theme may choose another per region.
- Cost: the HUD is always on screen. Re-arrange only on change (resize, UI scale, theme, a slot
  opening or closing, a content-sized component changing size such as the chat log growing), as
  `Update()` already does for the screen size; never every frame unconditionally.
- Migration is per component: a HUD component without a slot keeps placing itself, so each one
  moves on its own with both themes unchanged.

Implementation decisions approved by the user:
- RML/RCSS performs shell layout using nested flex containers. Header, footer, sides and center
  are a theme recipe; themes may rearrange or nest them. C++ supplies visibility and preferred
  sizes and reads resolved rectangles, without a second shell layout algorithm.
- Hidden components collapse by default. A theme may retain an empty slot for layout stability;
  visibility and space retention are distinct from reserve/overlay participation.
- Corner reservations use rectangular theme regions: a corner cell with adjacent regions, or
  a whole reserved sidebar. Automatic avoidance around arbitrary HUD shapes is outside this
  design. An overlay minimap consumes no layout space.
- Measure preferred content size before arranging. A fill component accepts its assigned size;
  a content-sized component must not derive its preferred size from that same assigned size.
  Wrapped content may measure against a constrained width without feeding its height back into
  the width. Each migrated component needs an explicit overflow policy.
- A small placement adapter exposes identity, visibility, preferred size where applicable and
  a callback to apply placement. HUD components need not inherit `CObject`; existing windows
  can adapt to the same contract. Every slot places its component; only fill-capable components
  must accept resizing. Rendering and input remain with the component.
- Resize, scale, theme, visibility and preferred-size changes invalidate layout. Coalesce those
  changes into one layout pass before rendering; checking a dirty flag each frame is fine.

Settled in H1:
- The main HUD is one unit for now (user's decision, 2026-10-05). `main_frame.rml` wraps its
  parts in `#hud_layout`, whose size each theme sets (640x51 dp, the strip and EXP bar); the
  parts are laid out inside it and may overflow it (the skill list does), but only its box is
  reserved. Splitting it into a slot per part waits until a layout needs it (H4's split HUD).
- Registration: `UI::Placement::RegisterParticipant()` takes a `PlacementParticipant` (visible,
  measure, place callbacks) under a slot name; `UI::RmlBridge::RegisterWorkspaceDocument()`
  adapts a document root to it (visible when the root and its document are, measured from the
  root's box, placed by `left`/`top` and a scale transform; the `workspace-placed` class lets
  the theme drop its own positioning). Without a slot, `place(nullptr)` restores the
  component's own placement.
- Readers of the strip's rectangle read the slot (`UI::Placement::SlotBox("main_hud")`);
  `GetStripRect()` and `HudReserve()` are gone.
- The NPC panel stage stays outside the shell (a direct child of the workspace body), so it
  centres on the whole screen as the original did, whatever the shell reserves.
- A component whose measured size can change (H2's chat log) calls `UI::Placement::Invalidate()`
  when it does; the slot then takes the new size before the next frame's windows update.

Header and docks (decided with the user, 2026-10-05; done, first H2 batch):
- The header holds both top corners: the MU Helper bar (207x25 dp, with the location text) in a
  left slot `mu_helper_bar`, and the modern theme's menu buttons (`main_frame_top.rml`,
  `#buttons_top`, 256x24 dp) in a right slot `top_bar` (pushed right by `margin-left: auto`).
  Both register through `RegisterWorkspaceDocument()` at the HUD scale. Legacy hides the top bar,
  so its slot collapses.
- A theme caps a dock region at the content area with `max-height: 100%`; the service then
  draws that region's windows at the scale that fits (the region's resolved height over its
  `data-ref-height` at the docked scale). The windows are scaled whole (RmlUi, native grids, 3D
  icons follow the slot transform), so no window needs a shrink or scroll policy. The content
  height depends only on the shell, so this is a second document update, only when the layout
  changed.
- Modern caps its docks and keeps the header at least 25 dp tall (steady while the MU Helper bar
  hides for a fourth column, `ShouldHideMuHelperBar()`). At 1024x768 and 100 % its panels are
  drawn at about 1.5 instead of 1.6, below the menu buttons; where they fit nothing changes.
  Legacy does not cap: its docks still reach above the header, as the original's did.
- Verified in game at 1024x768: modern at 100 % (panels under the menu buttons, close buttons
  and hover in place at the smaller scale), legacy at 100 % unchanged. Headless test: the header
  corners, modern's capped dock, legacy's uncapped one.
- Still to check by hand at a capped scale: dragging an item between windows (the drop guide),
  item tooltips, the uncovered-world edge. With a reserved header, `ShouldHideMuHelperBar()`
  only matters to legacy; a theme rule like `data-closes` could replace it later.

Chat (done, second H2 batch):
- A `chat-stack` region on the content area's bottom-left (an overlay: windows draw over it)
  stacks the `chat_log` slot on the `chat_input` slot; the log overlaps the input by 1 dp, as the
  original drew them. With the HUD in a header the chat follows the content area to the screen's
  bottom edge.
- `RegisterWorkspaceDocument()` takes options: `measure` (the owner's own numbers, so a resize
  is measured before RmlUi applies the model), `placedWhileHidden` (the input box keeps its slot
  while hidden, so the log does not drop when typing ends) and `placed` (a callback after
  placement).
- The chat windows stay in `LayoutMode::Hud`: their native hit tests (the input box's claim,
  the log's resize bands measured from its bottom edge) keep working in HUD space, with
  `m_WndPos` converted from the slot box by `placed`, and restored to the creation position
  without a slot. A log resize calls `Arrange()` so it grows upward in the same frame.
- Verified in game at 1024x768: modern at 100 %, legacy at 90 %, and the HUD-in-header layout.
  Hand check: dragging the log's resize handle and F4/F5 (the socket's keys did not reach it).

The rest of H2 (decided with the user, 2026-10-05: H2 is done as it stands):
- The minimap is not a corner widget: it is the full-screen map (Tab), drawn round the window
  centre, which already avoids the HUD through the `main_hud` slot (H1). It needs no slot, and
  H4's "minimap in a corner the windows avoid" does not apply to this game's map.
- The buff row, party list and item durability follow the uncovered-world edges in C++
  (`UncoveredWorld*In()`), so they already move with docks on either side. They are in two
  spaces: the party list and item durability in the docked windows' transform (dock scale and
  origin), the buff row in the HUD's. A `world_area` region sized by the service to the uncovered
  edges could hold them, but one region has one scale, and the dock scale is not RCSS's `dp`,
  so a theme could not write their offsets in its own units. Options: leave them as
  edge-followers; give each scale its own world-area region, with offsets the theme writes in
  `dp` and the service converts; or move the two dock-space widgets to the HUD scale (which
  changes their size at wide resolutions). Decision: leave them as edge-followers. They move
  with docks and with any reserved side region, since they follow the uncovered area; revisit
  only when a theme layout needs to place them itself.

Event HUDs (H3, done):
- All event HUDs are in HUD space (`LayoutMode::Hud`). Eight draw everything from their own
  position each frame and take it from an `event-hud` region: Blood Castle and Chaos Castle
  timers, battle soccer score, duel score, Kanturu info, Empire Guardian timer, Doppelganger
  frame and the duel spectator list (its slot is the list's bottom-left corner; it grows
  upwards). `UI::Placement::RegisterHudWindow()` registers such a window: its visibility, its
  native size, and a setter that converts the slot's box to its HUD-space position (its creation
  position without a slot).
- The region is the original's 640x480 frame standing on the HUD (1dp is one of its units), so
  a theme writes the original coordinates; anchoring it right keeps them in the screen's corner
  on wide screens.
- Left as they are: the Cursed Temple event screen (hard-coded native coordinates), Crywolf (a
  full-screen layer) and castle siege (parts built once from the creation position).
- `$win` names for showing them without the event: `bctime`, `cctime`, `soccer`, `duel`,
  `kanturuinfo`, `empiretimer`, `doppelframe`, `duelusers` (shown without the opening process,
  so nothing goes to the server). The castle timers and Kanturu info draw only in their event
  maps and the spectator list is empty without spectators, so those four stay hand checks.
- Verified in game at 1024x768: battle soccer, duel, Empire Guardian timer and Doppelganger frame
  at their original places in modern at 100 % and (timer) legacy at 90 %; a runtime stylesheet
  edit moved the timer to (0, 100), so the slot drives it.

Resolutions (2026-10-05): both themes at 100 % at 1920x1080 (HUD scale 2.25) and 1280x720, and a
live switch from 1024x768 to 1280x720 in the options window: docks (modern capped below the top
bar, legacy to the screen top), chat, event HUD and edge-following widgets in place.

| Phase | Work |
|---|---|
| H1 | Shell regions in both workspaces (header, footer, left, right, content); `#safe_area` becomes the content region; participation attribute with `reserve`/`overlay`. Main strip (`#hud_strip` + `#exp`) as a content-sized `reserve` footer slot; `HudReserve()` and `GetStripRect()` readers (placement, minimap clips) read the slot. Headless layout test for the shell; in-game check, both themes, 100 % and a non-100 % scale. |
| H2 | Done: top bar and MU Helper bar as header slots, modern's docks capped to the content area; chat log and input in a `chat-stack` region. The minimap needs no slot; buff row, party list and item endurance stay edge-followers (see "The rest of H2"). H2 is done. One component per batch, each verified in game. The HUD widgets' `UncoveredWorld*In()` positioning becomes their slots. |
| H3 | Done: eight event HUDs in an `event-hud` region (see "Event HUDs"); Crywolf, siege and the Cursed Temple event screen stay as they are. |
| H4 | Done: side bar, chat on the right, event HUDs in the screen's corner and HUD at the top, recorded in "Theme recipes"; split HUD and corner minimap dropped by decision. |

## Open questions

- One workspace document or one per region; which RmlUi context it lives in. Background-context
  documents (inventory family) already follow their window through `root_*` bindings.
- How z-order and focus interact with regions.

## Plan

Each phase builds, keeps `legacy` and `modern` in step, and is validated at 100 % and a non-100 %
UI scale with the [validation matrix](validation-matrix.md).

| Phase | Work |
|---|---|
| 0 | Done: rules classified below; fill-capability list in section 5. |
| 1 | Done, in-game checks pending: placement service and both themes' workspaces place the right-docked windows (content fit). The `PanelColumnX`/`SetPos` juggling in `Show()`/`Hide()` is gone; Gens ranking (stretched HUD space, not the dock) keeps its own. Differences from before: with character info, inventory and its extension open, the extension now sits beside the inventory (columns 3 and 2 swapped); windows that used to overlap in column 1 now sit side by side. |
| 2 | Done, in-game checks pending: HUD reserve from the HUD strip; uncovered world edge replaces `GetScreenWidth()`'s table. |
| 3 | Done, in-game checks pending: column-1/column-2 conflicts and the three-column limit are `data-closes` in both workspaces; `HideGroupBeforeOpenInterface()` is gone. `HideAllGroupA()` stays in C++ (it ends trades and NPC sessions, which a theme must not control). The MU Helper bar rule and the help-panel exclusions stay until their windows have slots. Change: windows closed by these rules now run their closing process; for the Gold Bowman windows that tells the server the event-chip dialog ended, which the old silent hide skipped. |
| 4 | In progress: character info and pet info opt into `data-fit=fill`. A theme sizes its slot in RCSS; the service gives that size to the panel, never less than the content size. Character info's right-hand pieces and action rows are pinned to `#panel`'s edges in both themes, so a wider content-sized panel stretches too. The shipped themes stay content-sized. Verified in game at 1024x768 (runtime layouts, both themes): 35%, 30% and 22% slots at UI scale 80, 90 and 100 %; action buttons and hints, both close targets, and live theme switches between a filled and a content-sized workspace. Pet info verified the same way at 35 % in both themes (tabs, corner close). The headless test passes (82 assertions, both windows). Other windows opt in after their hit areas and native content can follow a filled panel. |
| 5 | Done: Gens ranking has a right-dock slot (docked scale and place, like its neighbours; it used to be drawn in the stretched 640x480 space, wider than the docks on wide screens and ignoring the UI scale). The move map has a left-dock slot on the HUD (`.dock-left`), not marked `data-covers-world`, so nothing shifts around it as before. Verified in game: shipped layout, centre-left at 60 % height (legacy), full height at 30 % and 45 % width (modern). The friend list, which moves and sizes itself, asks its `friends` slot only where it first opens (`InitialPosition()`); the shipped themes put it in the bottom-right corner above the HUD, as before, and the player's moves win after that. Its chat and letter windows keep the friend manager's cascade. Verified in game, including a theme moving it to the left edge. The centred NPC panels (Kanturu entry, Cursed Temple entry and result) sit on a `panel-stage` region: the original's 640x480 stage centred on the screen, where `1dp` is one of its units, each panel at its original height; verified in game. The generic confirm and menu dialogs already centre themselves in their own theme CSS; help, item explanations and the quick command follow the pointer or their target, and the in-game shop covers the screen, so none of them needs a slot. |
| H1-H4 | Done. See the HUD section and "Theme recipes". |

Docs to update with phase 1: [theming-and-modding.md](theming-and-modding.md) ("three different
owners"), [layout-and-scaling.md](layout-and-scaling.md), [tracked-deferrals.md](tracked-deferrals.md),
and the readback exception in [architecture-principles.md](architecture-principles.md).

Decided with approval: slots are declared in RML markup; reading back slot rectangles is the one
readback exception (section 3); phase 1 reproduces today's layout before anything new.

## Phase 0: rule classification

From `CSystem::Show()`/`Hide()`, `HideAllGroupA/B()`, `HideGroupBeforeOpenInterface()`,
`ShouldHideMuHelperBar()` (`WindowSystem.cpp`) and `GetScreenWidth()` (`ZzzInventory.cpp`).
"Column" means a 190-wide `PanelColumnX(n)` column from the right edge.

### Placement (moves to the workspace)

| Rule | Today |
|---|---|
| Inventory beside character info | Column 2 while character info is open, else column 1 or its saved position; restored when character info, a personal shop or the inventory closes |
| Quest log, quest progress (etc) beside character info | Column 2 while character info is open, column 1 otherwise |
| Pet info beside character info | Created in column 2 |
| Vault, personal shops, NPC shop, mix, trade beside the inventory | Column 2; column 3 while the inventory extension is open; back to column 2 when it closes |
| Vault extension | Column 3 with the vault in column 2 |
| Single-column windows | Character info, party, guild, commands, NPC dialogue, quests, siege and event NPCs, Gens ranking, MU Helper config: column 1; MU Helper detail and lucky-coin windows: column 2 |
| HUD widgets that dodge docks | Buff strip, item endurance and party list right-align to the uncovered width after every open/close |
| Friends window | Floating, at `(workspace width - 250, content height - 170)` |
| Uncovered world width | `GetScreenWidth()`'s table (section 4a) |

### Space exclusion (moves to the theme)

| Rule | Today |
|---|---|
| Column-1 conflicts | Opening the inventory, character info, pet info or quest log first closes party, commands, command list, guild, both Gold Bowman windows, Gens ranking and the MU Helper windows (`HideGroupBeforeOpenInterface()`) |
| Column-2 conflicts | Inventory closes the quest log; quest log closes inventory and pet info; pet info closes inventory and quest log; quest progress (etc) closes inventory and quest log |
| Three-column limit | Opening the inventory extension closes the vault extension, quest log and character info |
| MU Helper bar | Hidden while the vault extension is open, or while the inventory extension is open with character info, vault, a shop, mix or trade |
| Help panels | Help, item explanation, set-item explanation and move map close each other |
| Unclear | Chaos Castle's timer closes the chat input: overlap or event rule, check in game |

### Game rules and companions (stay in C++)

| Rule | Today |
|---|---|
| NPC-session windows | Opening an NPC or interaction window (mix, NPC shop, vault, personal shops, trade, NPC quest, Blood Castle, Devil Square, catapult, senate, guards, gatekeeper, gate switch, guild master, lucky coin, NPC dialogue, quest progress, lucky item, and also party, commands, guild info, master level, Gens ranking, MU Helper) closes all of them (`HideAllGroupA()`) |
| Companions | NPC shop, vault, mix, trade, personal shops, lucky coin and lucky item close the inventory when they close, and closing the inventory closes them (running their closing process, which can veto) |
| Pet info | Closes with character info, which opens it |
| Cash shop, Kanturu | `HideAll()` / `HideAllGroupB()` |
| Checks | `IsImpossible*Interface()` |

`HideAllGroupA()` stays whole in C++. Its non-NPC members (party, commands, guild info, master
level, Gens ranking, MU Helper) may close NPC windows for lack of room, but closing them ends
trades and NPC sessions (through `Hide(INVENTORY)` and its companions), so the effect is gameplay
either way.
