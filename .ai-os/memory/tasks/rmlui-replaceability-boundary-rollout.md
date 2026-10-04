# RmlUi replaceability boundary — rollout task

Date: 2026-10-04. Review baseline: `1fa69e277` plus the working-tree changes present during the review.
Status: RB-01, RB-02, and RB-04 implemented; full RelWithDebInfo build passed. The user reported
that in-game spot checks were OK, including after the RB-03 inventory and transfer batches; exact
scenario coverage was not enumerated. RB-03's bounded packet migrations are implemented and build,
but the full `WSclient.cpp` compile boundary remains open. RB-05 reviewed; no code change needed.

## Goal and scope

Keep RmlUi inside presentation and integration code so replacing it changes views, bindings, assets,
and bootstrap wiring without requiring engine, networking, or feature rules to adopt another UI
framework. Use feature-level state and operations where a boundary is needed. Do not introduce a
generic DOM, document, event, or style interface.

This task comes from the **RmlUi Coupling, Abstraction, and Replaceability** review. It is separate
from [main-frame flexibility](rmlui-main-frame-flexibility-rollout.md); neither rollout is a
prerequisite for the other. Preserve current tooltip content, chat protocol and room behavior,
theme switching, native/RmlUi layering, and UI scale. Do not turn this into a broad UI cleanup.

References: [coding rules](../../../docs/CODING_RULES.md),
[UI architecture principles](../../../docs/rmlui-ui-system/architecture-principles.md),
[building new UI](../../../docs/rmlui-ui-system/building-new-ui.md), and
[validation matrix](../../../docs/rmlui-ui-system/validation-matrix.md).

## Boundary findings at the review baseline

| ID | Current evidence | Replacement cost |
|---|---|---|
| RB-01 | `Engine/Object/ZzzInventory.h` includes `UI/RmlBridge/RmlTooltip.h` and returns `Tooltip::Line` from `BuildTooltipLinesFromTextList()`. `ZzzInventory.cpp` and `GameLogic/Pets/GIPetManager.cpp` construct tooltip lines. | Engine/game code and a widely shared header must change with tooltip presentation. This is project UI-type leakage, not a raw `Rml::` type leak. |
| RB-02 | `UI/Social/ChatRoomView.cpp` stores participants in the bound model; `CUIChatWindow::GetUserCount()` and `GetChatFriend()` read them for `SocialWindowManager`. The view also checks invitation/send rules and sends requests. | A view replacement must relocate room state and protocol behavior. Theme reload currently copies the model, so this is not a reported theme-reload data-loss bug. |
| RB-03 | `Network/Server/WSclient.cpp` includes `UI/Core/WindowSystem.h`, whose many concrete window includes reach `UI/Inventory/MyInventory.h` and `RmlModelBinder.h`. | Networking compilation depends transitively on RmlUi headers, although packet code does not manipulate RmlUi. |
| RB-04 | `Core/Utilities/Log/muConsoleDebug.cpp` implements `$theme` by calling `ThemeExists()`, `SetActiveThemeName()`, and `ReloadAllThemedDocuments()`. | A core utility knows the current presentation implementation's reload sequence. |

RmlUi types in window/scene presentation classes, `RmlModelBinder`, `RmlUiRuntime`, and renderer
integration are expected. `Core/Input/UiInputRouter.h` is an existing useful project-level input
boundary. Do not remove or wrap those merely to erase `Rml::` from implementation files.

## Task order

| Step | Work | Depends on | Status |
|---|---|---|---|
| RB-01 | Move legacy tooltip conversion and presentation types out of engine/game-facing APIs | None | Implemented; full build passed; in-game spot checks found no major issues |
| RB-02 | Put chat-room state and send rules in a feature owner; bind the view from that state | None | Implemented; full build passed; in-game spot checks found no major issues |
| RB-03 | Route a coherent inventory packet family through feature operations; measure remaining networking dependencies | None; recheck after RB-01/02 | Inventory and transfer batches built; user spot checks OK; broader compile boundary open |
| RB-04 | Route the console theme command through a UI-owned handler | None | Implemented; full build passed; in-game spot checks found no major issues |
| RB-05 | Reassess scene loading-overlay ownership after the higher-value boundaries | RB-01–04 review | Reviewed; no change needed |

Steps can be shipped separately. Recheck references immediately before each edit because the branch
has concurrent main-frame work. Keep each commit focused; do not stage unrelated working-tree edits.

### RB-01 implementation checkpoint (2026-10-04)

`UI/Tooltip/LegacyTextListTooltip` now owns the one legacy-buffer conversion and the show/hide
operation. Item, repair, pet, Master Level, and Cursed Temple callers pass screen-pixel anchors and
retain their prior above/below and tooltip-owner choices. `ZzzInventory.h` no longer includes the
tooltip presentation header or exposes its line type. The item and pet copies of the conversion
were removed. The conversion retains the existing colors, bold flags, spacer markers, and first-empty-
line stop; the shared tooltip still handles clamping and presentation.

The first RelWithDebInfo attempt compiled every changed source and linked `MuClient.lib`, then
stopped with `LNK1168` because the running game instance held `Main.exe` open. The process closed
without intervention; the next full build linked `Main.exe` successfully. The user later reported
in-game spot checks with no major issues. Which item/pet/event tooltip variants were exercised was
not specified, so the representative scenario checklist is not fully confirmed.

### RB-02 implementation checkpoint (2026-10-04)

`CUIChatWindow` now owns the participant list, lock state, and duplicate-send history. Its
`GetUserCount()` and `GetChatFriend()` no longer read the RmlUi binding model. Invitation and chat
requests are sent from that owner after its room rules run. `ChatRoomView` mirrors participant data
for rendering, owns draft/focus/layout, and translates RmlUi actions into owner calls. The owner
also gives `GetChatFriend()` a defined 0/1/2 result for no peer/pair/group; its old implementation
left the result uninitialized except for groups, while `SocialWindowManager` reads it.

The RelWithDebInfo client library and full executable built after the final RB-02 edits. The user
later reported in-game spot checks with no major issues. Coverage of two-person/group rooms,
invite/send/remove, reopen, and theme reload was not specified.

### RB-01 — Tooltip conversion boundary

- Inventory/pet code should produce legacy tooltip content or call a feature-level UI operation
  without exposing `UI::RmlBridge::Tooltip::Line` in an engine header.
- Place `TextList`/color/bold-to-tooltip conversion with UI-owned tooltip presentation. Reuse one
  conversion for item, event, master-level, and pet callers where their input semantics match;
  inspect pet-specific differences before deduplicating.
- Keep line order, spacer markers, color mapping, alignment, anchors, and timing unchanged.
- Acceptance: `ZzzInventory.h` no longer includes a UI tooltip header or names its types; engine
  and pet gameplay paths no longer construct `RmlBridge::Tooltip::Line`; all existing tooltip
  callers build and representative item/pet/event tooltips are checked in game.

### RB-02 — Chat-room state and command boundary

- Identify which data is room state (participants, server indices, lock/send rules, last sent line)
  and which is view state (draft field, focus, scroll, invitation-pane visibility, geometry).
- Keep room state in `CUIChatWindow` or a focused project-owned room model. Present a bound copy to
  `ChatRoomView`; translate RmlUi callbacks into feature operations before sending requests.
- Preserve duplicate-message behavior, invitation limits, room title/member count, reconnect,
  multiple simultaneous rooms, and theme reload while open.
- Acceptance: `SocialWindowManager` queries room-owned participants; chat protocol decisions no
  longer depend on the RmlUi binding model; RmlUi events and DOM details stay in the view. Verify
  two-person and group rooms, invite/send/remove, reopen, and theme reload in game.

### RB-03 — Compile boundary

Further inspection of `WSclient.cpp` shows direct calls to concrete inventory, shop, storage,
trade, guild, and social windows throughout packet handling. `WindowSystem.h` has roughly 158
source/header include sites. Removing its concrete includes alone would replace one transitive
dependency with many direct includes and would not improve replacement cost. Treat this as a
feature-boundary migration rather than a mechanical include cleanup.

The first inventory packet family originally dispatched item changes to `CMyInventory`,
`CInventoryExtension`, and `CMyShopInventory`. Further inspection found that their item slots live
in native `CInventoryCtrl` grids, while the RmlUi models present the frames. `CMyInventory` also
updates `CharacterMachine->Equipment` and pet effects. A second item-state copy would add sync
risk without improving RmlUi replacement. The bounded seam therefore centralizes the existing
equipment/grid operations in `UI::Inventory::InventoryContents`; it does not claim that the native
grid's state has moved to a new owner.

- Inventory packet handling is the first candidate: map its calls to project-owned inventory
  operations and identify which are state updates versus view-specific commands. Keep packet
  decoding and item identity semantics unchanged.
- Pilot a bounded adapter for one coherent packet family, then measure the remaining concrete
  window dependencies before extending it. Preserve the other families until their feature
  boundaries are understood; do not introduce one giant generic UI command interface.
- Only after callers stop requiring the concrete type, narrow `WindowSystem.h` includes or use
  forward declarations where complete types are unnecessary.
- Acceptance for a completed family: the packet handler uses feature-level operations, and its
  declarations do not require RmlUi-backed window headers. Re-run the compile-boundary search
  and supported build. Full `WSclient.cpp` independence is a separate end check, not a claim made
  after one pilot.

Implemented 2026-10-04: `ReceiveDeleteInventory`, `ReceiveInventoryExtended`,
`ReceiveModifyItemExtended`, and `ReceiveBuyExtended` now use the inventory feature operation for
slot routing and item mutation. Packet decoding, mutation order, picked-item cleanup, and resync
decisions remain in their previous places. `InventoryUtils.h` now contains only slot predicates;
the concrete grid lookup is `UI::Inventory::FindPlayerItem()` alongside the mutation operations.
This removes `WindowSystem.h` from that shared game-logic header without putting UI-dependent code
in a game-logic implementation file. Three full RelWithDebInfo player builds passed after these
batches. The count of concrete inventory/storage/mix control references in `WSclient.cpp` dropped
from 105 at `HEAD` to 84. The file still includes `WindowSystem.h` for other packet families, so
its transitive RmlUi compile dependency remains. The developer-only `App/Control` callers were
updated to the new lookup and passed MSVC syntax checks using the player compile flags plus
`MU_ENABLE_CONTROL_SOCKET`; a full developer build was not run. The user later reported that
in-game spot checks were OK; exact inventory cases were not enumerated. The remaining concrete
calls are concentrated in picked-item transfers, mix/storage, and personal-shop replies; they need
their own behavior review before moving them behind feature operations.

Follow-up implementation 2026-10-04: pickup and drop replies now use the inventory insertion,
lookup, and picked-item operations. The inventory destination of an equipment-transfer reply now
calls `UI::Inventory::ReceivePlayerTransfer()`, which keeps picked-item cleanup, equipment and grid
insertion, and storage auto-move success in the inventory feature. Its failure path uses
`RejectTransfer()` to restore the picked item and notify both storage grids. The source-slot lookup
for personal-shop price transfer also lives in that feature. Trade, vault, mix, and lucky-item
destinations remain in the packet handler for a later focused pass. The drop reply keeps its old
main-grid deletion behavior for non-equipment indices; this pass does not reinterpret server slots.
The full RelWithDebInfo executable and RML/RCSS guards built successfully. A consistent source
search for concrete player inventory, storage, mix, and picked-item control references in
`WSclient.cpp` fell from 85 at `779346761` to 60. `WindowSystem.h` remains included, so the complete
compile boundary is still open. The user later reported that the requested in-game spot checks
were OK. Coverage of normal pickup, full-inventory pickup, successful and rejected drop, and
storage-to-inventory auto-move success/failure was not specified individually.

Siege pass (2026-10-05): `UI/Combat/SiegeUpdates` now owns castle minimap and commander presentation, guardsman status and guild lists, hunt-zone gatekeeper, catapult, and crown notices. `WSclient.cpp` decodes packet values before calling the feature operations; `CGuardWindow` takes decoded status and names. A full RelWithDebInfo build passed. In-game spot checks remain pending with the consolidated RB-03 checklist.

Doppelganger pass (2026-10-05): `UI/Events/DoppelgangerUpdates` owns entry, progress, party positions, match-frame lifetime, and result-state presentation. `WSclient.cpp` keeps named protocol outcomes and passes decoded values. A full RelWithDebInfo build passed. In-game spot checks remain pending with the consolidated RB-03 checklist.

Empire Guardian pass (2026-10-05): `UI/Events/EmpireGuardianUpdates` owns timer updates and uses its day/zone when composing result dialogs. `WSclient.cpp` keeps named entry and match outcomes. A full RelWithDebInfo build passed. In-game spot checks remain pending with the consolidated RB-03 checklist.

Lucky Coin and CryWolf pass (2026-10-05): feature operations now own the registration and exchange button states, coin count display, and CryWolf countdown. Lucky Coin result codes are named in `WSclient.cpp`. A full RelWithDebInfo build passed. In-game spot checks remain pending with the consolidated RB-03 checklist.

Kanturu pass (2026-10-05): `UI/Events/KanturuUpdates` owns entry-window updates and battle-info visibility/timer. The entry window takes named stage, detail, and result types; `WSclient.cpp` decodes the packet values before calling it. A full RelWithDebInfo build passed. In-game spot checks remain pending with the consolidated RB-03 checklist.

### RB-04 — Theme command ownership

- Keep `$theme` command recognition in the console, but move theme validation, activation, config
  update, and document reload to UI-owned code behind one feature-level operation.
- Acceptance: core console code does not include `RmlTheme.h` or call document reload helpers;
  `$theme legacy` and `$theme modern` still update open windows as before.

Implemented 2026-10-04: `UI/Theme/ThemeSelection` validates the folder, updates the config value,
optionally saves it, activates the theme, and reloads documents. The console calls it with
session-only persistence; Options calls it with saved persistence. Both prior call sequences and
the console's invalid-name behavior remain. The full RelWithDebInfo build passed. The user later
reported in-game spot checks with no major issues. Hot switching with open windows and restart
persistence were not specifically confirmed.

### RB-05 — Loading-overlay decision

`Scenes/LoadingScene.cpp` currently owns a raw loading document and loads `loading.rml`. This is a
scene presentation implementation, so some framework-specific code there is acceptable. After
RB-01–04, decide whether its lifecycle is clearer in a small UI-owned loading overlay. Only move
it if that produces a real boundary improvement; preserve its lifetime until the world is ready.

Reviewed 2026-10-04: `LoadingScene()` creates the document and `HideLoadingSceneOverlay()` closes it
when `LoadingWorld` drops below the loading threshold. Both are scene-presentation functions; the
scene does not expose RmlUi types to engine or network headers. Moving the document to a separate
UI object would only add forwarding and lifecycle indirection, so leave this boundary as it is.

## Verification and completion

- After each significant code batch, run the supported RelWithDebInfo build and fix introduced
  errors. Run focused tests where existing tests cover the changed behavior.
- Use source searches to check the specific API/include boundaries above. A raw symbol count is not
  the acceptance criterion: RmlUi types remain valid in presentation and integration code.
- Record runtime checks separately from build evidence. The chat and tooltip scenarios require
  in-game validation; do not mark them passed merely because compilation succeeds.
- Reassess replacement cost at the end: engine/game-facing headers and core utilities should be
  free of these presentation dependencies, while RmlUi views and runtime remain free to use RmlUi.
