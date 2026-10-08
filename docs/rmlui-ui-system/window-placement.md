# Window placement

A theme, or a mod of one, decides where every window and HUD part goes, how big it is, and how
windows arrange around each other, without C++ changes (principles §15–16): docked on either side
or not at all, beside each other in one theme and apart in another, covering the whole screen or
half of it — anything ordinary RCSS can express.

Implemented in `UI/Placement/WindowPlacement.{h,cpp}`; the window registry is at the end of
`CSystem::LoadMainSceneInterface()`. Headless tests: `tests/ui/test_window_placement_layout.cpp`.

## 1. The workspace document

Each theme ships `workspace.rml` with its RCSS. It holds a **slot** per placeable window or HUD
part, inside **regions** the theme defines with ordinary RCSS:

```html
<body id="workspace">
  <div id="workspace_shell">                       <!-- flex column -->
    <div id="shell_header" class="region" data-scale="hud" data-participation="reserve">
      <div class="slot" data-window="mu_helper_bar"></div>
      <div class="slot" data-window="top_bar"></div>
    </div>
    <div id="shell_middle">
      <div id="shell_left" class="region" data-participation="reserve"></div>
      <div id="safe_area">                         <!-- the content area -->
        <div class="region dock-right" data-covers-world>
          <div class="slot" data-window="character"></div>
          <div class="slot" data-window="inventory" data-closes="my_quest"></div>
          …
        </div>
        <div class="region chat-stack">…</div>
        <div class="region event-hud">…</div>
      </div>
      <div id="shell_right" class="region" data-participation="reserve"></div>
    </div>
    <div id="shell_footer" class="region" data-scale="hud" data-participation="reserve">
      <div class="slot" data-window="main_hud"></div>
      <div class="slot" data-window="duel_watch_hud"></div>
    </div>
  </div>
  <div class="region panel-stage">…</div>           <!-- centred on the whole screen -->
</body>
```

- A slot is shown only while its window is open, so open windows pack in a region by flex layout.
  Order within a region is DOM order (this RmlUi build has no `order` property).
- A mod changes placement by overriding `workspace.rml`/its RCSS per theme, like any themed
  document. A window without a slot places itself as before.
- The workspace document is never shown: the service lays it out itself and reads it back. This
  is the one sanctioned readback of RCSS geometry into C++ (section 3).
- Unknown `data-window`/`data-closes` names are logged once per workspace (`[Placement]` in
  MuError.log).

## 2. The shell and the content area

- **Shell regions** — header, footer, left, right — are nested flex containers the theme arranges
  or nests freely. `data-participation="reserve"` takes space; `overlay` is placed but takes none.
- **`#safe_area`** is what the reserved regions leave: the docks, chat and event HUDs live in it.
  The `.panel-stage` (centred NPC panels: the original's 640x480 stage, 1 dp per unit) sits outside
  the shell so it centres on the whole screen, as the original did.
- Hidden components collapse their slots by default; a theme may keep an empty slot for stability.
- **The main HUD is one unit**: `main_frame.rml`'s `#hud_layout` (640x51 dp, the strip and EXP
  bar, sized by each theme) is the `main_hud` footer slot. Its parts may overflow the box (the
  skill list does); only the box is reserved. The full-screen map clips around
  `UI::Placement::SlotBox("main_hud")`.
- **The duel watcher's HUD** (`duel_watch_hud`, `duel_watch_frame.rml`, 640x51) is a second footer
  slot. The duel-watch buff hides the main HUD and shows it, so the footer reserves whichever is
  open, and the docks and event HUDs stand on it the same way. It draws in `LayoutMode::HudFrame`
  like the event HUDs.
- **Header**: the MU Helper bar (`mu_helper_bar`, left) and the modern theme's menu buttons
  (`top_bar`, `main_frame_top.rml`, pushed right by `margin-left: auto`). Legacy hides the top
  bar, so its slot collapses. Modern keeps the header at least 25 dp tall so it stays steady while
  the MU Helper bar hides.
- **Chat**: a `chat-stack` overlay region on the content area's bottom-left stacks `chat_log` on
  `chat_input` (the log overlaps by 1 dp, as the original drew them).
- **Event HUDs**: an `event-hud` region, the original's 640x480 frame standing on the HUD (1 dp per
  unit), so a theme writes the original coordinates. Both themes centre it like the HUD
  (`left: 50%; margin-left: -320dp`), so they keep their places beside the HUD at any UI scale and
  width. Eight use it (Blood Castle and Chaos Castle timers, battle soccer, duel,
  Kanturu info, Empire Guardian timer, Doppelganger frame, duel spectator list — whose slot is its
  bottom-left corner, growing upward). Crywolf, castle siege and the Cursed Temple event screen
  place themselves.
- **Edge-followers**: the buff row, party list and item durability need no slot; they follow the
  uncovered area (section 5) every frame, so they move with docks and reserved side regions.

## 3. The placement service

On a change, not every frame:

1. show or hide each slot and write content-fit sizes;
2. lay out the workspace document (a second pass if a region is fitted, section 6);
3. read each visible slot's resolved rectangle;
4. give each participant its rectangle.

`Arrange()` runs synchronously after every `CSystem::Show()`/`Hide()`, so a newly shown window
never draws or hit-tests a frame at a stale position. Passive changes (screen size, UI scale,
theme reload, a HUD part shown or hidden, the chat log resizing) call `Invalidate()`, and
`Update()` re-places once before the frame's windows update.

**Participants.**
- **Windows** (`CObject`): a placed window's layout mode becomes `LayoutMode::Slot` — its logical
  space is the slot, (0, 0) at the slot's top-left, at the region's scale (`PlaceInSlot()`).
  Everything that maps window coordinates reads `GetLayoutTransform()`, so `CManager` hit-testing,
  native grids, 3D icons and `SyncRootTransform()`'s `root_*` follow the slot unchanged. Without a
  slot a window returns to its `UILayoutPolicy` mode.
- **Placed documents**: a window that returns its document from `CObject::GetPlacedDocument()` has
  its `#panel` placed by the slot itself (`UI::RmlBridge::SlotPlacement`): the slot's top-left and
  its region's scale arrive as `--slot-left`, `--slot-top` and `--root-scale` with the `slot-placed`
  class, given again to a document a theme switch rebuilt, and `base.rcss`'s `#panel.slot-placed`
  applies them, so a theme may place the panel otherwise. Its model binds no `root_*`. Such a window
  needs a slot in every theme; `CCharacterInfoWindow` is the first. The corner X and exit buttons
  close it with `data-event-click="window_close"` (`UI::RmlBridge::BindWindowClose()`).
- **HUD documents**: `UI::RmlBridge::RegisterWorkspaceDocument(name, docGetter, rootId, options)`
  adapts a document root to a `PlacementParticipant` (visible/measure/place callbacks) — placed by
  `left`/`top` and a scale transform; the `workspace-placed` class lets the theme drop the part's
  own positioning. Options: `measure` (the owner's own numbers, so a resize is measured before
  RmlUi applies the model), `placedWhileHidden`, and a `placed` callback.
- **HUD-space windows** (the chat, the event HUDs): `UI::Placement::RegisterHudWindow()` converts
  the slot box to a HUD-space `m_Pos`/`m_WndPos`, so native hit tests keep working in HUD space.
  The event HUDs draw in `LayoutMode::HudFrame`, the bottom HUD's own uniform, UI-scaled scale, the
  same one their region sizes slots in; the original's W/640 x H/480 stretch left out the UI scale,
  so at 90 % a frame came out bigger than its slot and ran under the HUD.
- Without a slot, `place(nullptr)` restores the component's own placement.

## 4. Slot sizing: content or fill

- **`data-fit="content"`** (default): the slot takes the window's `#panel` size
  (`RefreshLogicalPanelSize()`) times the region's scale.
- **`data-fit="fill"`**: the window takes the slot's size; never less than its content size (or
  its `GetFillMinimumSize()`). A window opts in by returning its document from
  `CObject::GetFillDocument()`, using `UI::RmlBridge::FillPlacementSize`, and a top-right
  `#frame_corner_close` element instead of the native fixed-offset `HandleFrameCornerClose()`. A
  window without fill support falls back to content and logs a warning.

24 windows can fill: the docked RmlUi windows (character, pet, party, guild info and make,
command and command list, NPC dialogue, quest log and progress, MU Helper config and detail,
castle, guard, gatekeeper, gate switch, catapult, duel watch, Gens ranking, Blood Castle and Devil
Square entry) and the centred NPC panels. Character info, pet info and the move map have fluid
content and are verified filled; how the others use extra room is the theme's RCSS. Windows with
native item grids or live 3D need more before they can fill (inventory family, NPC quest,
Doppelganger, Empire Guardian NPC, United Marketplace, skill list). The shipped themes use content
sizing throughout.

Example — character info as a 35 %-wide full-height column inside `#safe_area`:

```html
<div class="region side-panel" data-scale="panel" data-covers-world>
  <div class="slot character-fill" data-window="character" data-fit="fill"></div>
</div>
```

```css
.side-panel { left: 0; right: 0; top: 0; bottom: 0; display: flex; }
.character-fill { width: 35%; height: 100%; }
```

## 5. Uncovered world area

The content area excludes reserved HUD regions; the uncovered area also excludes open docks
inside it. Open slots in a region marked `data-covers-world` narrow it from the content-area edge
their group packs against (a group touching neither edge counts on the side of the centre it is
on). A full-screen overlay region can stay unmarked so nothing shifts under it.

- `GetScreenWidth()`/`GetScreenLeft()` (`ZzzInventory.cpp`) return
  `UI::Placement::UncoveredWorldRight()`/`Left()` in the 640-wide HUD space: pet HP bars, shop
  titles, the macro cooldown bar, endurance tooltips and centred HUD text read them.
- HUD widgets read `UncoveredWorldLeftIn()`/`RightIn()` in their own layout every frame.
- Neither area changes the rendered game viewport. `HeroX` (the hero's facing centre) still uses
  the right edge only — a mismatch that predates this work, left for a gameplay pass.

## 6. Scale

A region's `data-scale` picks the scale: `dock` (default; the original docks, capped at 2.25×),
`panel` (centred panels, 2.0×) or `hud` (2.0×). A region capped by RCSS `max-height: 100%` is
**fitted**: its windows draw at the scale that fits (resolved height over `data-ref-height` at
the region's scale), scaled whole — RmlUi, native grids and 3D icons follow the slot transform.
Modern caps its docks below the header; legacy doesn't, so its docks reach the screen top as the
original's did.

## 7. Arrangement rules

Every rule in `CSystem::Show()`/`Hide()` is one of three kinds:

- **Placement** — where an open window goes. In the workspace.
- **Space exclusion** — a window closes only because there is no room. In the theme: a slot's
  `data-closes` lists the windows that close when its window opens, through the normal
  `CSystem::Hide()` (running their closing process). Names are slot names; a window without a slot
  can be registered by name (Gens ranking, the MU Helper skill picker). The legacy theme declares
  the original's behaviour (column-1 conflicts, inventory/quest log/pet info, the three-column
  limit); other themes may relax them.
- **Game rule** — stays in C++; the theme still decides where companions go:

| Rule | Where |
|---|---|
| NPC-session windows close each other (and party, commands, guild info, master level, Gens ranking, MU Helper), ending trades and NPC sessions | `HideAllGroupA()` |
| Companions (NPC shop, vault, mix, trade, personal shops, lucky coin, lucky item) close with the inventory and close it when they close (closing process can veto) | `Show()`/`Hide()` |
| Pet info closes with character info, which opens it | `Hide()` |
| Cash shop, Kanturu | `HideAll()` / `HideAllGroupB()` |
| Checks | `IsImpossible*Interface()` |

Still in C++ until their windows have slots: the MU Helper bar hiding for a fourth column
(`ShouldHideMuHelperBar()`, only matters to legacy with a reserved header) and the help panels'
mutual exclusion. Unchecked: Chaos Castle's timer closes the chat input — overlap or event rule.

## 8. Dragging and saved positions

The player drags dialogs and the friend system only, and no position is saved. Docked windows stay
in their slots; dragging a window out of its slot is not planned.

- The friend list, chat rooms and letters drag through `UI::RmlBridge::MakeDraggable()` and
  bounce back on screen. The friend list asks its `friends` slot only where it first opens
  (`InitialPosition()`); the chat and letter windows keep the friend manager's cascade.
- The options window, the generic menu dialog (the system menu among them) and the generic confirm
  dialog drag by any part that is not a control (`base.rcss` gives
  `input, select, textarea, .btn, [data-event-click]` `drag: block`), stay where they were dropped
  for the session, and move back inside the window if dropped partly outside
  (`KeepInsideWindow()`). A menu or confirmation opens centred each time
  (`ResetDraggedPosition()`); the confirm dialog's chrome documents follow its panel.
- The native message boxes (`CMessageBoxBase` family) are not draggable.
- The inventory drags by its title. The position lasts until the workspace places it again (another
  window opens or closes in its region); positions are not saved across sessions.

## 9. `LayoutMode`

Slotted windows use `Slot` while placed, or `HudFrame` for the HUD-scale parts. The full list is in
[layout-and-scaling.md](layout-and-scaling.md#layout-modes). `Dock*`, `FloatingWorkspace` and
`Stage` also serve windows without slots; `DockTransform()`'s fixed HUD height only serves
unslotted docked windows.

## Theme recipes (verified in game)

Each was tried by editing only a theme's workspace/RCSS on runtime copies, clicks included.

- **Split docks** (legacy, 90 %). Move `character`, `my_quest`, `pet`, `quest_progress_etc` into a
  second region styled like `.dock-right` with `flex-direction: row` and no `right`; drop the
  cross-side `data-closes` (inventory/quest log, extension/character).
- **Centre stage** (legacy). Put the inventory family in a region with `display: flex;
  justify-content: center; align-items: center` filling `#safe_area`.
- **Composed row** (legacy). Make `#safe_area` a flex row of left dock, centre (`flex: 1 1 auto`)
  and right dock, regions `position: relative`, in that markup order — centred windows then use
  the space the docks leave instead of overlapping them on a narrow screen.
- **Side bar** (modern, 100 %). Move the `top_bar` slot into `#shell_right` and lay the buttons out
  in a column in `main_frame_top.rcss` (`#buttons_top` 56x124 dp, each button's `top`). Docks pack
  against the bar; the uncovered world, durability icons and item buttons move left of it.
- **Chat on the right** (modern). `.chat-stack { left: auto; right: 0; align-items: flex-end; }`.
- **Event HUDs in the screen's corner** (modern, 1920x1080). `.event-hud { left: auto; right: 0; margin-left: 0; }`.
- **HUD at the top** (both). Move the `main_hud` slot into `#shell_header`; docks and chat follow
  the content area down to the screen's bottom edge.
- **Character info filled** (both) at 22–35 % width and UI scale 80–100 %; pet info at 35 %; the
  move map centre-left at 60 % height and as a full-height column.

Not done, by decision: a split main HUD (one unit for now) and a corner minimap (this game's
minimap is the full-screen map). A left dock covers the chat log, which a theme using one would
move.

Resolutions checked: both themes at 1024x768, 1280x720 and 1920x1080 (HUD scale 2.25), and a live
switch in the options window.

## Theme-sized windows

A window's size is the theme's choice, like its place: a slot takes the window's `#panel` size, so
what fixes a size is inside the window — chrome at fixed pixels, 190/429 literals in RML,
hit boxes against C++ constants, and native parts at `m_Pos + constant`.

| Tier | Windows | State |
|---|---|---|
| 1. RCSS only | character, party, pet, NPC dialogue, gate switch, MU Helper config | Done: the docked frame pins to `#panel`'s edges; a counter-scaled leaf as wide as the panel is `calc(100% * var(--root-scale))`. |
| 2. Plus hit box | guild info, guild make, command, command list, Blood Castle and Devil Square entry, catapult, lucky coin registration, lucky item | Done: hit boxes read `#panel`. Catapult and lucky item not checked in game. |
| 3. Plus anchors | quest progress, quest log, NPC quest, castle, guard, gatekeeper, duel watch, MU Helper detail, United Marketplace | Done in code (RmlUi click targets, theme-placed). Siege NPC windows not checked in game. |
| 4. Item grids | inventory, extension, vault, NPC shop, mix, trade, personal shops | Done: each grid's box and its `.item-cell` pitch (`--item-cell`, `CInventoryCtrl::FollowGrid()`), the equipment slots and their item boxes (`#slot_*_item`) and the option tooltip are the theme's; C++ hit-tests and draws the items in them. Legacy headings centre on `panel_width`. |
| 5. Shared views | Gold Bowman (both), Doppelganger and Empire Guardian entry, lucky coin exchange | Each registers its document, so slots take the theme's size; native parts still need anchors. |

## Open questions

- How z-order and focus interact with regions.
