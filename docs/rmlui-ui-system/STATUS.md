# Status Against the Architecture Principles

Living document — update this, not `architecture-principles.md`, when status changes. Section
numbers refer to [`architecture-principles.md`](architecture-principles.md). Per-class status is in
[`migration-ledger.md`](migration-ledger.md); open work is in
[`tracked-deferrals.md`](tracked-deferrals.md).

## What's migrated

Every window is a `mu::ui::window::CObject` drawn by RmlUi in both themes. The `CWin` toolkit,
the sprite widgets, the `CUIControl` toolkit and the shared item camera (`C3DRenderMng`) are
deleted. Families:

- **Login and character select** — login, server select, character select and creation, the
  system menu, the credits, message boxes, the remember-password prompt, character balloons.
- **HUD** — `main_frame.rml` (bars, buttons, `CSkillList`, `CItemHotKey`), the MU Helper bar, the
  buff strip, chat log, system log and chat input, item endurance, notices, the mini map, HUD
  menus, the party list, the move list. The whole HUD is one theme-placed unit in the workspace
  (`window-placement.md`).
- **Inventory family** — `CMyInventory`, `CTrade`, storage and its extension, mix, NPC shop, the
  personal shops, inventory extension, lucky items. One document each: the grids are
  `item_grid.rcss` cells, and the live 3D items draw into the window's render target over them.
- **Docked panels** — character info, quests, pet, party, guild, the MU Helper config and detail
  windows, all on `docked_panel_frame.rcss`.
- **Dialogs** — `CGenericConfirmDialog` and `CGenericMenuDialog` replace most of the native
  message-box family (ledger's Dialog table has what's left); `COptionWindow`.
- **Social** — the friend family (shell, chat rooms, letters), guild windows, Gens ranking, item
  explanations.
- **Events, siege, duel, NPC windows** and the event HUDs; **world labels** (names, balloons,
  bars, ground items) through the world-label layer.

**Placement is the theme's.** No window has a layout of its own: documents sit on `.stage`,
`.hud-board` or a workspace slot, RmlUi hit-tests them, and `CManager` gives native code one
measuring space (`layout-and-scaling.md`'s "Units and placement"). Only infrastructure may touch
the active transform; `tools/check_layout_transform_users.py` keeps it to that.

**Small-scale text and event validation (2026-10-10, signed off 2026-10-11).** The contract guard
checks every theme's documents again (111 documents, 222 variants); the party list, trade, event
entry, personal-shop and MU Helper text fits at small scales without lowering the native minimum,
checked by a headless audit of the real documents in five languages; the Illusion Temple tooltips
and result table and the siege markers were verified through `$preview`. Where legacy keeps
native's positions, long labels are marquees. Outcome: [migration-ledger.md](migration-ledger.md#small-scale-text-and-event-validation).

**Stays native on purpose**: live 3D content (item grids, equipped items, item and character
previews — `RenderTarget` can show one inside a document, as the potions, the letter portrait,
the character-creation preview and the event previews do), the mouse cursor, developer overlays, and the equipment paperdoll's
background/durability tint/drag highlight, which paint behind the equipped item's 3D icon
(`tracked-deferrals.md`).

## Checklist for every new port (principles §27's workflow, condensed)

1. **Layout intent traces to the original code's computed behaviour**, not its literal
   default-case numbers (§2–3) — `buff_strip.rml`'s header derives `x = (iScreenWidth - 200) / 2`
   from `SetPos()`'s four hardcoded pairs.
2. Uses the `dp` anchor/stretch/center classes (`layout-and-scaling.md`) or a workspace slot
   (`window-placement.md`) instead of C++-pushed rects, unless the position is genuinely per-frame.
   A bound per-frame offset uses the unit of the sibling static CSS (`dp` ≠ `px`).
3. Deliberate aspect-ratio/resolution behaviour: fixed, edge-anchored, centred or stretched (§7–8).
4. C++ owns state, binding, events and game behaviour; RCSS owns layout, sizing and position
   (§1, §16; `building-new-ui.md`'s ownership rules).
5. The C++ class and the RmlUi assets are named for what the component is, renamed at port time
   (§12).
6. Both themes in the same pass. A rendering technique new to the port is verified at runtime
   (`engine-findings.md`).
7. Reuses existing primitives (`component-catalog.md`). For the modern theme, match the windows it
   appears **beside** on screen (its dock group), not its nearest technical sibling; a docked
   panel links `docked_panel_frame.rcss`.
8. A new document gets its original window's layer depth in the stacking table (below).
9. A window whose native hit box comes from live RCSS is clicked through at a non-100 % UI scale
   (`layout-and-scaling.md`'s scale sweep).

## Stacking order

Every document's `z-index` is the layer depth of the original window (or render pass) it
replaces, from one table (`UI/RmlBridge/RmlStackingOrder.cpp`), set when a themed document loads.
The original drew its windows in ascending `GetLayerDepth()` order, then the notices, the scene
windows, the login scene's message box and the reconnect dialog; RmlUi sorts a context's
documents by `z-index` and keeps show/focus order only among equal depths, so
`SyncDocumentVisibilityInFront()`/`Behind()` and focus only order documents of one depth. Passes
outside the window list: object descriptions
and the map name 0.5, notices 20, scene windows 30 (balloons 29, the remember-password prompt 31),
loading and title screens 40, reconnect dialog 50. The shared tooltip is 10.69, above every window
and under the message boxes (10.7): the original drew each tooltip at its owner's depth, where the
chat log, the friends window and the HUD hid its rows. The top-right button row (the modern
theme's choice) is its own document (`main_frame_top.rml`, 1.05: over the names, under every
window, which dock over that corner). Native parts (item grids, 3D items) keep the native order
and stay under the main context. `rml_stacking_order_tests` checks that every document the sources
name has an entry.

## Legacy parity rules

Differences to the original found by the paired comparison suites and fixed in shared places, so
a new port inherits them:

- **Scene gate.** A window `CSystem` updates only in the main scene still has a live document in
  every scene. Its stacking-table entry marks it a main-scene document, and every such document is
  suspended outside the main scene (`SuspendMainSceneDocumentsOutsideMainScene()`). The HUD's
  documents also wait for the world to load (`CSystem::SyncMainSceneHudVisibility()`).
- **Scroll thumb.** The legacy `.scroll-pane` thumb is the native 15x30 knob, not a proportional
  bar; a list the original scrolled one row per wheel notch takes `mousescroll` itself
  (`CMoveCommandWindow::RmlWheelList()`), since RmlUi scrolls 80 dp per notch.
- **Button hover text.** A document's `data-hint` and a native button's `CTooltip` both use the
  shared tooltip's `Config::Box::ButtonHint` (unframed, 2 units off the button, the other side
  when there is no room); the framed box is for `RenderTipTextList()`.
- **Hangul.** NanumGothic is a fallback face, so Korean game text draws in any family.
- **Alpha test.** Art the original drew under `EnableAlphaTest()` (reference 0.25) stays invisible
  while its fade is below a quarter (the Illusion Temple banner).
- **Scene windows re-created per visit.** A `Create()` that resets model fields must mark them
  dirty (the login fields) and reset what the original reset (the server list's chosen group).

## Lessons from shipped ports

Engine quirks are in [`engine-findings.md`](engine-findings.md); these are porting patterns.

**Input and focus**

- **A text field's window claims the field's document.** While the player types,
  `CManager::UpdateKeyEvent()` gives keys only to the window whose `TakesTypingFrom()` accepts the
  focused field's document (`RmlUiRuntime::GetTypingDocument()`); the chat line, chat command,
  Gold Bowman, MU Helper and friend windows claim theirs, or Enter/Escape never arrive.
- **Focus, scroll pins and scroll rewinds are one-shot latches**, done once on the frame after the
  view caught up — never per frame (that steals focus and kills the scrollbar). `CSystem::Show()`
  runs `OpenningProcess()` before `ShowInterface()`, so arm there and consume in the sync.
- **A native window behind an RmlUi document never sees a mouse press**
  (`ProcessMouseButtonDown` consumes it while anything is hovered), though the wheel still reaches
  it. Drive the gesture from the document (`UI::Social::PhotoViewerControl`).
- **`UpdateMouseEvent()` returns `false` only to consume.** `CManager` stops dispatching at the
  first `false`; a "not for me" guard must return `true` (an inventory guard once ate every drop
  into the trade grid).
- **A port can drop a side effect that was covering a bug** — `CUIMuHelper::Show()` released a
  dead startup input box's focus, and without it every hotkey stayed suspended. Look for what a
  removed call was hiding.

**Layout and text**

- **Click-through.** `IsMouseOverUI()` is true over any hovered `pointer-events: auto` element, so a
  scroll pane over the world is a wall; the chat log's lines are `pointer-events: none` with the
  scrollbar opted back in at every level, and hover/right-click moved to C++.
- **Bottom-up lists**: a flex column with `margin-top: auto` on the first item, not
  `justify-content: flex-end` (which pushes the earliest lines out of the scroll area); per-line
  backgrounds need `align-items: flex-start`.
- **A `.scroll-pane` inside a `transform: scale()` panel counter-scales itself out**
  (`.counter-scaled`, its box `calc(Npx * var(--root-scale))` in RCSS), with rows at an explicit
  width, not `100%` (`component-catalog.md`).
- **A counter-scaled layer's lengths are RCSS**: `#panel` binds `--root-scale`, base.rcss's
  `.sharp-text` / `.counter-scaled` cancel it, and the theme writes `width: calc(160px *
  var(--root-scale))`. A number multiplied by `root_scale` in RML fails the bound-geometry guard.
- **A label in a constricted box** that must keep native's position is a `.marquee`: one line,
  `..` when longer, the whole text scrolling on hover (`RmlMarquee.h`). Text that may take more
  lines wraps in a flow or a scroll pane instead (`event_entry`, `shop_notice.rcss`).
- **`MeasureText()` returns reference units**, not pixels; `RenderText()` shrinks text wider than
  its box — use `NativeTextPixelSizeInBox()` per text. Text the native renderer draws small is laid
  out at `CachedFontPointSize()` and scaled down (the login scene lines).
- **`overflow: hidden` does not clip under a panel's `transform: scale()`**; crop a bar with
  `decorator: image(<sprite> scale-none left top)` on an element of the shown width, or an
  untransformed box with `clip: always`.
- **Centre a counter-scaled label in RCSS**, not by a C++-measured top: `.sharp-middle` centres it on its button's height and `.sharp-centre` on its
  parent's width (`engine-findings.md`).
- **Draw a window in the scale its slot is sized in.** The event HUDs were placed in the HUD's
  UI-scaled units but drawn in the original's W/640 x H/480 stretch, so at 90 % they outgrew their
  slots; they now draw at the HUD's own scale.
- **Data expressions have no unary minus**: bind `-x` from C++.
- **Class-specific controls are one tested C++ table bound as flags**
  (`UI::MuHelper::ResolveClassFeatures()`), never RCSS — the themes cannot disagree.

**Drawing**

- **A window the original drew under every panel** (siege HUD, duel and battle-soccer boards)
  takes its low depth from the stacking table; **one with a live 3D preview** draws it into a render
  target in its own document (`UI::Items::ItemCameraTarget`, `UI/Events/EventItemEntryView`).
- **Native 3D inside a document** goes through `UI::RmlBridge::RenderTarget`, so it z-orders with
  the windows around it.
- **World-anchored or shared legacy drawing** goes through the world-label layer's
  `Overlay2DRecordScope`, which records `RenderText()`/`RenderColorQuadARGB()`/`RenderBitmap()`
  and replays them into pooled elements; `CObject::PrepareFrame()` records them before any window
  renders.
- **`EnableAlphaBlend()` is additive** (ONE, ONE): `decorator: additive-fill(<colour>)` /
  `additive-image(<colour> <image>)`.
- **Textures are premultiplied on load** (`RmlUiRenderInterface::LoadTexture`).
- **A texture cut by UVs, padded by the loader, or drawn mirrored** is an `<img rect="x y w h">` in
  texels (mirrored: `transform: scale(-1, 1)`).
- **A quad rotated in physical pixels** is a CSS `matrix()` built from three corners
  (`UI/HUD/MiniMapLayout`).
- **Image paths** in a themed document resolve from the theme folder (`../../../../Logo/…`); an
  absolute `/Interface/…` path misses `Data` and draws a white quad.
- **Animations the original stepped in `Render()`** move to `Update()`.
- **One document and model per instance** (chat rooms, letters) through `ThemedView`'s
  `modelPlaceholder` and `SetModelName()`.
- **A block-scope `extern` inside `mu::ui::window`** declares a namespace member, not the global.
- **Windows the original never showed** can carry latent crashes; exercise every size.

## Known gaps against the principles

- **No mod/user-override resource precedence** (§18–19). Themes are two directories selected by
  name; no user layer over a theme and no partial theme inheriting from a base. A stated
  requirement waiting on priority. (A third first-party theme is ruled out by the project owner;
  §25/§28's coupling concern is addressed by `modern`'s divergence and the restored contract guard.)
- **Native 3D is not yet behind one mechanism** (§14, §21): three contexts order it against RmlUi
  where `RenderTarget` would do (the background lists are closed; `tracked-deferrals.md`'s native
  3D entry).
- **Validation covers the UI-scale axis only** (`layout-and-scaling.md`'s scale sweep);
  resolution, drag state across a scale change, and theme change while open are uncovered, and
  are left to whoever touches each window rather than tracked.
- **The login scene's windows** (`CCreditWin`, `CLoginMainWin`, `CSysMenuWin`, `COptionWindow`,
  `CServerSelWin`, `CMsgWin`, `CCharSelMainWin`, `CCharMakeWin`, `CLoginWin`) have not been
  audited for native draws that still assume a fixed resolution (`CCreditWin` assumes 800x600).

## Upstream sync log

This branch tracks `origin/dev/rmlui-ui-system` (`sven-n/MuMain`). Log each rebase or sync onto
a newer upstream head here, one line per sync.

| Date       | Sync                                                                                                                          | Conflict verdict                               | Resulting tip |
| ---------- | ----------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------- | ------------- |
| 2026-09-01 | Rebased onto `sven-n/MuMain` PR #572 (`a9739fb2`, docs-only)                                                                  | Clean — verified in an isolated worktree first | `878f35e4`    |
| 2026-09-19 | Rebased local `rmlui-on-sdl-gpu` (1 commit) onto `origin/dev/rmlui-ui-system`; pushed as a fast-forward, `b3bf33d7..233d808b` | Clean — verified via a full incremental build  | `233d808b`    |
