# Window placement (design proposal)

**Status: approved 2026-10-05; phase 0 (rule classification) done, implementation not started.**
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

A window receives its rectangle as its layout transform: a new `LayoutMode::Slot` whose transform
is the region's scale with the slot's top-left as offset, and `m_Pos` becomes `(0, 0)`. Everything
built on the transform keeps working unchanged: `CManager` hit-testing, grids at fixed offsets, 3D
icons, and `SyncRootTransform()`'s `root_x`/`root_y`/`root_scale`. Migrating a docked window means
deleting its `PanelColumnX`/`SetPos` lines, not rewriting it.

The service runs before a newly shown window's first update and render, so it never draws a frame
at a stale position.

**Reading geometry back.** The main-frame rollout's rule was that C++ never reads RCSS geometry to
draw or place chrome. That rule stays for chrome. This design adds one narrow, named exception
that the minimap already uses (`CMiniMap::SyncClips()` reads `GetStripRect()`): C++ reads the
resolved rectangles of a document whose only job is placement, and hands them to other documents
and native code. C++ still never computes a layout itself.

### 4. HUD reserve

The workspace root is padded by insets derived from the HUD parts' resolved rectangles: the edges
the HUD covers become reserved, so docks follow a HUD a theme moves to the top or a side. This
replaces `RoundedBottomHudTop()`/the 51-unit constant for placed windows. The HUD widgets that
dodge open windows today (buff strip, item endurance, party list) become slots too, and move by
flex layout instead of `SetPos(screenWidth)`.

### 4a. Uncovered world area

`GetScreenWidth()` (`ZzzInventory.cpp`) is a third hard-coded copy of the column layout: a table
of which window combinations cover one, two or three 190-wide columns. Gameplay presentation
reads it: the hero's screen centre for facing the mouse (`HeroX` in `ZzzInterface.cpp`), pet HP
bars, personal-shop titles, the macro cooldown bar, endurance tooltips, and the terrain viewport
outside the main scene. The placement service publishes the uncovered world rectangle instead:
the viewport minus regions the theme marks as covering the world (`data-covers-world`). A theme
can mark a full-screen overlay region as not covering, so the hero does not shift under it.

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
- **Space exclusion**: a window closes only because there is no room. Moves to the theme, as
  exclusive groups on slots or a region's maximum open count.
- **Game rule**: windows that cannot coexist for gameplay or server reasons, and companion windows
  that only work together. Stays in C++; the theme still decides where companions go.

The legacy theme declares exactly today's behaviour; other themes may relax space rules.

### 7. User placement and saved positions

A slot marked `data-draggable` lets the player drag its window out of the slot. The window leaves
the region's flow (neighbours repack) and is saved per theme as an anchor plus offset relative to
its region (principles §10–11), not raw pixels. Precedence: theme default, then user override.
A reset returns it to its slot. The inventory's existing saved position migrates to this.

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
- Exclusion vocabulary: named groups, region capacity, or both.
- How z-order and focus interact with regions.

## Plan

Each phase builds, keeps `legacy` and `modern` in step, and is validated at 100 % and a non-100 %
UI scale with the [validation matrix](validation-matrix.md).

| Phase | Work |
|---|---|
| 0 | Done: rules classified below; fill-capability list in section 5. |
| 1 | Placement service, `LayoutMode::Slot`, and both themes' workspaces reproducing today's docked family exactly (content fit, right dock, column behaviour). Delete the `PanelColumnX`/`SetPos` juggling for those windows. Inventory drag keeps working. |
| 2 | HUD reserve insets; HUD-dodging widgets become slots. |
| 3 | Space exclusions move to the theme. |
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

`HideAllGroupA()` is kept whole in phase 1. Its non-NPC members (party, commands, guild info,
master level, Gens ranking, MU Helper) probably close NPC windows for lack of room rather than for
a server reason; phase 3 decides each one before moving any to the theme.
