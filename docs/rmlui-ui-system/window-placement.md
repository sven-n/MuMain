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
  scale. C++ reads the window's `#panel` box (as `RefreshLogicalPanelSize()` already does) and
  writes the slot size. Pixel-art panels authored at a reference size (the whole legacy theme)
  stay exactly as they are.
- **`data-fit="fill"`**: the window takes the slot's size. Its document receives the slot's width
  and height and lays itself out fluidly. Only windows that support it can be placed this way
  (section 5); otherwise the service logs a warning and uses `content`.

No `calc()` is needed: content sizes come from C++, fill sizes from `%`, `vw`/`vh` and `dp`, and
gaps from margins and padding. This RmlUi build has no `calc()`, and the design does not rely on
one, so no RmlUi fork is required.

### 3. C++ role: a placement service

On a change (open, close, resize, UI scale, theme switch, HUD move), not every frame:

1. show or hide each slot and write content-fit sizes;
2. lay out the workspace document;
3. read each visible slot's resolved rectangle;
4. give each window its rectangle.

A content-fit window receives its slot's top-left as a position in its own layout space, through
its existing `SetPos()`. Everything built on that keeps working unchanged: `CManager` hit-testing,
grids at fixed offsets, 3D icons, and `SyncRootTransform()`'s `root_x`/`root_y`/`root_scale`.
Migrating a docked window means registering it and deleting its `PanelColumnX`/`SetPos` lines, not
rewriting it. A fill window also needs its size, so fill brings a `LayoutMode::Slot` whose
transform maps the window onto its slot (phase 4).

The service runs synchronously after every `CSystem::Show()`/`Hide()` and when the screen size or
UI scale changes, so a newly shown window never draws a frame at a stale position. The workspace
document is never shown: it is never drawn or hit, and the service lays it out itself.

Implemented in `UI/Placement/WindowPlacement.{h,cpp}`; the window registry is at the end of
`CSystem::LoadMainSceneInterface()`. Inputs the game sets on the workspace: `#safe_area`'s
`bottom`, content slot sizes, and region heights from `data-ref-height`.

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
`data-covers-world`, converted to the 640-wide HUD space. Gens ranking and the two legacy panels
without slots still count as one column each. A theme can leave a full-screen overlay region
unmarked, so nothing shifts under it.

At 4:3 this equals the old table. At wider screens it follows the docks' real edge, where the old
table assumed 190 stretched units per column, so the buff strip, item endurance, party list, pet
bars and shop titles now line up with the docks. The HUD widgets keep their own `SetPos()`; they
need no slots while the uncovered edge positions them.

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

  A window declares support in C++ (for example `SupportsFillPlacement()`), and the theme's RCSS
  for it must be fluid, which usually means the theme forks that window's RCSS. Windows with live
  3D or native item grids today, and so the most work: inventory family (`CInventoryCtrl` in
  inventory, extension, vault, mix, trade, NPC/personal shops, lucky item), NPC quest,
  Doppelganger, Empire Guardian NPC, United Marketplace, duel watch, skill list. Character info
  has no live 3D, which makes it a good first fill candidate.

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

Content-fit windows keep reference-unit panels scaled by their region. Today docks cap at 2.25×,
panels and the HUD at 2.0×. Each region declares which scale it uses (for example
`data-scale="dock"`); the legacy theme declares today's values. Fill windows use `dp`.

### 9. What `LayoutMode` becomes

Windows with a slot use `Slot`. `Legacy` (scene windows that work in real pixels) and
`WorldOverlay` (world-anchored balloons and names) stay. The `Hud*`, `Dock*`,
`FloatingWorkspace` and `Dialog` rows of `UILayoutPolicy.cpp` disappear as their windows gain
slots.

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
| 4 | Fill support: hover hit-testing, `RenderTarget` for native content per window; character info first. |
| 5 | Remaining families: NPC windows, move map, friends, centred dialogs. |

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
