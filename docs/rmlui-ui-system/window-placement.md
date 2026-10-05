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
    <div class="slot" data-window="friends" data-draggable/>
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

The service runs synchronously after every `CSystem::Show()`/`Hide()` and when the screen size or
UI scale changes, so a newly shown window never draws a frame at a stale position. The workspace
document is never shown: it is never drawn or hit, and the service lays it out itself.

Implemented in `UI/Placement/WindowPlacement.{h,cpp}`; the window registry is at the end of
`CSystem::LoadMainSceneInterface()`. Inputs the game sets on the workspace: `#safe_area`'s
`top`/`bottom`, content slot sizes, and region heights from `data-ref-height`. A `data-window` or
`data-closes` name no window answers to is logged once per workspace (`[Placement]` in
MuError.log).

**Reading geometry back.** The main-frame rollout's rule was that C++ never reads RCSS geometry to
draw or place chrome. That rule stays for chrome. This design adds one narrow, named exception
that the minimap already uses (`CMiniMap::SyncClips()` reads `GetStripRect()`): C++ reads the
resolved rectangles of a document whose only job is placement, and hands them to other documents
and native code. C++ still never computes a layout itself.

### 4. HUD reserve

`#safe_area` keeps free the screen edge the HUD strip sits on, read from the main frame's
resolved strip (`CMainFrameWindow::GetStripRect()`): its top for a bottom HUD, its bottom for a top
HUD. Docks therefore follow a theme that moves the HUD. While the HUD is not on screen the
original strip's place is kept. Both themes' strips end 51 dp above the bottom today, which is
the original's height, so nothing moves yet.

### 4a. Uncovered world area

`GetScreenWidth()` (`ZzzInventory.cpp`) is a third hard-coded copy of the column layout: a table
of which window combinations cover one, two or three 190-wide columns. Gameplay presentation
reads it: the hero's screen centre for facing the mouse (`HeroX` in `ZzzInterface.cpp`), pet HP
bars, personal-shop titles, the macro cooldown bar, endurance tooltips, and the terrain viewport
outside the main scene. `GetScreenWidth()` now returns the placement service's uncovered right
edge (`UI::Placement::UncoveredWorldRight()`): the leftmost open slot in a region marked
`data-covers-world`, converted to the 640-wide HUD space. The two legacy panels without slots (refinery, server
division) still count as one column each. A theme can leave a full-screen overlay region
unmarked, so nothing shifts under it.

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

  A window declares support in C++, and the theme's RCSS for it must be fluid, which usually
  means the theme forks that window's RCSS. Character info is the first fill-capable window: its
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

Target: a slot marked `data-draggable` lets the player drag its window out of the slot. The window
leaves the region's flow (neighbours repack) and is saved per theme as an anchor plus offset
relative to its region (principles §10–11), not raw pixels. Precedence: theme default, then user
override. A reset returns it to its slot.

Phase 1 keeps today's inventory behaviour instead: `data-saved-position` names the saved drag
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
| 5 | In progress: Gens ranking has a right-dock slot (docked scale and place, like its neighbours; it used to be drawn in the stretched 640x480 space, wider than the docks on wide screens and ignoring the UI scale). The move map has a left-dock slot on the HUD (`.dock-left`), not marked `data-covers-world`, so nothing shifts around it as before. Verified in game: shipped layout, centre-left at 60 % height (legacy), full height at 30 % and 45 % width (modern). The friend list, which moves and sizes itself, asks its `friends` slot only where it first opens (`InitialPosition()`); the shipped themes put it in the bottom-right corner above the HUD, as before, and the player's moves win after that. Its chat and letter windows keep the friend manager's cascade. Verified in game, including a theme moving it to the left edge. Remaining: centred dialogs. |

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
