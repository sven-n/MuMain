# Shallow UI cleanup review

## Review disposition: closed, findings resolved or carried forward

Reconciled on 2026-10-04 against `9a56ff680` and the organization cleanup worktree.
The original findings below are retained as a historical record.

- Applied cleanups, quest row grouping and Jewel Harmony diagnostics: resolved in code.
- Empty-window factory variant: removed by subsequent branch work.
- AddWindow placement/failure-flow cleanup: carried forward to
  [Social manager cleanup](ui-social-manager-cleanup.md).
- Portrait drag-release observation: carried forward to the separate
  [correctness investigation](ui-portrait-drag-release-investigation.md).
- Stable legacy formatting: no action selected; not an outstanding task.

See [organization cleanup](ui-organization-cleanup.md) for the current implementation
and validation status. Closing this review does not mark its follow-up tasks complete.

> Follow-up checkpoint (2026-10-04): this report below records the earlier review
> snapshot. The Jewel Harmony filename diagnostic and quest reward group indexing
> follow-ups were subsequently implemented and built successfully
> (`out/build/windows-x64/ui-cleanup-followup-build.log`). A supplemental comparison
> of the old/new row-index and spacer algorithms passed 286 valid count combinations;
> this was not a C++ unit test. Those two findings are resolved. At HEAD `9a56ff680`,
> the cleanup has been committed by subsequent branch work, social files have moved
> to `UI/Social`, and the empty-window variant has been removed. Historical paths and
> the uncommitted-state description below should not be treated as current status.
> The current structural review is in `ui-organization-review.md`.

Date: 2026-10-04. Branch: `dev/rmlui-ui-system`. Reviewed HEAD: `44844d936`.

## Summary

Expanded after the user clarified that the review should cover the target directories,
not just recent commits. Scope is the current branch's `src/source/UI`,
`src/source/Render/RmlUi`, and `src/bin/Data/Interface/RmlUi`, irrespective of age.
The previous friend-family changes remain part of the cumulative uncommitted diff.

### Coverage and method

- Directory-wide C++/header scans covered duplicate includes, unused local macros,
  empty namespaces, redundant boolean branches, suspicious empty conditionals,
  temporary returns, and stale migration references.
- Reviewed candidate definitions and callers before editing. Scans are candidate
  discovery, not proof of dead code or a line-by-line review of every function.
- Broader manual samples included Core input/visibility, Inventory extension and
  Jewel Harmony, Events (Lena and Guards Man), Dialogs (message manager), Combat
  spectators, HUD ticker, Quests reward rows, Character pet view, Options dropdown
  handling, NPC dialogue wrapping, MuHelper toggles, Chat labels, and Windows login.
- Renderer/bridge samples included texture load/release ownership, additive decorators,
  runtime input/focus handling, native text fitting, theme capability caching, and
  document/model lifetime helpers. No changes to GPU ownership or event routing.
- Asset-wide scans/checks covered every RML/RCSS file, including repeated declarations
  within a rule, inline script attributes, syntax, theme drift, and bound geometry.
  Inspected representative Options, NPC, Quest, and friend-family styles. No new
  duplicate-declaration or inline-script cleanup candidates were found.
- Good code was left unchanged: conditional reads of cached theme capability state,
  font-fit cache observer pointers, independent renderer texture ownership, and
  deferred theme/dropdown work all carry real constraints.

This remains a shallow review with directory-wide scans and selected manual inspection;
it does not certify that every function is free of cleanup opportunities.

## Applied Changes

### Applied: Remove dead older inventory and combat code

**Location:** `src/source/UI/Inventory/UIJewelHarmony.cpp`,
`src/source/UI/Combat/DuelWatchUserListWindow.cpp`.

**Change:** Removed the anonymous `GetTextLines` helper (no source/test references),
the unused `NOTSMELTING_DATA_FILE` macro, and an empty anonymous namespace.
Simplified the Jewel Harmony constructor's redundant success flag.

**Why:** These leftovers obscure the live file-loading and semantic-view paths.
The active file loader, error handling, and ownership remain unchanged.

**Validation:** Repository reference searches and final Windows build.

### Applied: Simplify established input and visibility branches

**Location:** `src/source/UI/Core/WindowCommon.cpp`, `UI/Dialogs/MessageBox.cpp`,
`UI/Inventory/InventoryExtension.cpp`, `UI/Events/UIGuardsMan.cpp`,
`UI/Events/GoldBowmanLena.cpp` (all under `src/source`).

**Change:** Removed a duplicate editor-only include; returned the four key-state
comparisons directly; simplified the message manager's final emptiness return;
removed an inventory-extension visibility query whose branches both returned true;
and returned the Guards Man level comparison directly.
Lena's handler now has one return for the visible window instead of duplicated
branches and an unreachable return.

**Why:** The code now states the same result without redundant branches. Lena still
consumes input whenever visible, and still clears right-button flags only for a
right press inside its panel. No change to its input policy was inferred.

**Validation:** Inspected `CNewKeyInput::IsNone` and the visibility lookup chain to
verify removed queries only read state. Checked callback order and return paths.
Final Windows build passed. The editor-only duplicate include was checked directly;
an editor-enabled build was not run.

### Applied: Name the ticker's expired-notice condition

**Location:** `src/source/UI/HUD/SlideTicker.cpp` (`SlideLane::ManageSlide`).

**Change:** Replaced the empty `if (...); else if (...)` construct with a named
`expiredNotice` and the equivalent short-circuit condition. Named the 60-second age.

**Why:** Makes intentional suppression of expired notices explicit. Speed updates,
queue erasure, text ownership, time reads, and the add-failure path are unchanged.

**Validation:** Compared both condition paths and their side effects; final build.

### Applied: Remove the retired tab lookup path

**Location:** `src/source/UI/Party/UIWindows.cpp`, `UIWindows.h`, `FriendWindow.h`.

**Change:** Removed `AddWindowFinder`, `RemoveWindowFinder`, the facade forwarding
method, and `m_WindowFindMap`. Simplified `GetWindow`/`IsWindow` to use the owning
window map. Removed the unused `m_WindowReadyMap` and a duplicate include.

**Why:** The removed native tabs were the secondary map's consumers. Repository
search found no remaining registration calls, so its fallback path could never
find a window. The existing member-iterator convention is retained in this pass.

**Validation:** Source/test reference searches before and after removal; full
Windows compilation and `Main.exe` link.

### Applied: Make string-buffer mutation explicit

**Location:** `src/source/UI/Party/ChatRoom.cpp`, `ChatRoom.h`.

**Change:** Made `SelectedInvite()` and `ChatFriend()` non-const and removed their
`const_cast` expressions. Both still return their existing owned conversion buffers.

**Why:** These methods update wide-string buffers. Their only direct callers are
non-const window forwarding methods; the signature now expresses the actual operation.
No return-value lifetime, conversion, or network behavior was changed.

**Validation:** Declaration/caller searches, compilation of callers and client link.

### Applied: Remove obsolete migration artifacts

**Location:** `src/source/UI/Party/FriendWindowView.h`, `FriendWindow.cpp`,
`src/source/UI/Widgets/UIControls.cpp`, `tools/rml_bound_geometry_allowlist.txt`.

**Change:** Removed unused `FriendWindowRect` and its vector include, two commented-out
render calls, the unused local `ARRAY_SIZE` macro, and obsolete allowlist entries
for `chat_room.rml`, `friend_shell.rml`, `letter_read.rml`, and `letter_write.rml`.

**Why:** None has a remaining source consumer. The geometry guard explicitly identified
all four entries as unnecessary; removing them restores enforcement for those documents.

**Validation:** Reference searches; geometry guard after removal; full build.

### Applied: Update comments to describe current behavior

**Location:** Friend-family view/window sources and headers; legacy/modern
`chat_room.rcss` and `friend_shell.rcss`; `RmlUiSystemInterface.cpp`.

**Change:** Removed stale claims about photo masks, two-document letter views, and
future removal of the already-deleted transcription layer. Shortened timing and
keyboard-coordinate comments while retaining the actual constraints.

**Why:** The current semantic documents, post-RmlUi portrait pass, and coordinate
conversions should be understandable without reconstructing migration history.
RML structure and RCSS rules were not changed.

**Validation:** Compared comments with document loading, portrait rendering, and
coordinate conversion code; asset checks and full build.

## High-Value Remaining Cleanups

### [HIGH-VALUE] Clarify quest reward group boundaries

**Location:** `src/source/UI/Quests/QuestRewardModel.cpp:32` (`BuildRows`).

**Observation:** `j` selects three different groups, `i` spans all groups, and
`nLoop` alternates between a count and a cumulative end index. Headings and blank
rows are interleaved with those offsets.

**Why it matters:** A reader must reconstruct the producer's flat-array ordering to
understand which rows belong to requirements, ordinary rewards, and random rewards.

**Suggested cleanup:** Name the group and cumulative end index explicitly, then
isolate appending one contiguous group while keeping heading/blank-row placement.

**Scope:** Migration code.

**Confidence:** High about the readability issue. Left unchanged because the exact
row ordering is coupled to `GetRequestRewardText`; establish representative empty,
ordinary, and random reward cases before altering its loop structure.

### [HIGH-VALUE] Separate placement from the window factory's setup sequence

**Location:** `src/source/UI/Party/UIWindows.cpp:108` (`CUIWindowMgr::AddWindow`).

**Observation:** Creation, shell/list registration, placement collision handling,
initialization, and final map insertion occupy one long function.

**Why it matters:** Early failures are difficult to follow alongside placement logic.

**Suggested cleanup:** Extract the existing placement/cascade block into a small
local helper, preserving registration order and failure handling exactly.

**Scope:** Modified legacy code.

**Confidence:** Medium. Left unchanged because placement failure deletes the new
window after earlier registration work, and the implementation uses shared iterator
state. Establish those side effects before extracting; this pass did not redesign them.

## Minor Remaining Cleanups

### [MINOR] Jewel Harmony failure diagnostic names an unopened file

**Location:** `src/source/UI/Inventory/UIJewelHarmony.cpp:17` (constructor).

**Observation:** The constructor opens only the options file, but the log and message
box still name both options and smelting files.

**Why it matters:** A missing-file report points players at an unrelated resource.

**Suggested cleanup:** Report the actual filename passed to the loader.

**Scope:** Existing legacy code.

**Confidence:** High. Left as a small follow-up to keep this pass's file-loading
failure behavior, including displayed messages, intact.

## Legacy Cleanup Opportunities

### [MINOR] Check the unused empty-window factory variant before deleting it

**Location:** `src/source/UI/Party/UIWindows.h:174` (`CUIDefaultWindow`),
`UIWindows.cpp:115` (`UIWNDTYPE_EMPTY` factory case).

**Observation:** Searches found its class instantiated only in that factory case,
and no callers requesting `UIWNDTYPE_EMPTY` by name.

**Why it matters:** It retains a native-window variant beside the migrated windows.

**Suggested cleanup:** Verify integer-valued factory inputs and enum compatibility,
then remove the variant if no supported caller can select it. Preserve remaining
enum values if they matter to callers.

**Scope:** Existing legacy code.

**Confidence:** Medium. A named-reference search alone does not establish that an
integer-taking factory can never receive zero; no hierarchy or enum changes were made.

## Out-of-Scope Correctness Observations

### Portrait turning may remain active after release outside its document

**Location:** `src/source/UI/Party/PhotoViewerControl.cpp:31` and `:66`.

`Attach()` listens for mouse movement and release on the owning document.
`ProcessEvent()` sets `m_Turning` on press and clears it on mouse-up. In the vendored
engine, `Context.cpp:597` and `:740` dispatch these events to the current hover element.
A drag crossing onto another document or the background may therefore miss its release;
returning to the letter could resume turning with no button held.

This is source evidence, not a reproduced runtime failure. A focused input review should
test crossing documents, releasing outside the letter, hiding it during a drag, and
returning afterward. Leave capture/cancellation behavior to that review; it was not
changed here.

## Positive Patterns

- Separate documents and model names retain chat/mail window identity.
- Event callbacks queue actions instead of tearing down their document during dispatch.
- Model binding and explicit dirty notifications make semantic state easy to trace.
- Theme reload and portrait listener detach paths are explicit.
- Comments explaining dp/native coordinate conversion and the portrait render-order
  constraint remain useful and were preserved.

## Validation Performed

- Full VS18 Windows `RelWithDebInfo` build passed, including `MuClient.lib` and `Main.exe`.
  Final broader-pass log: `out/build/windows-x64/ui-cleanup-broad-final.log`.
- `python Tools/check_rml_rcss_syntax.py`: passed, 176 RML / 254 RCSS files.
- `python Tools/check_rml_rcss_drift.py`: passed, 87 documents / 124 files.
- `python Tools/check_rml_bound_geometry.py`: passed, 176 RML files.
- `git diff --check`: passed.
- Source/test searches confirmed removed-symbol consumers and changed-signature callers.
- `BUILD_TESTING` remained OFF. CTest and interactive game/theme/scale checks were not run.
  Earlier migration test results are not claimed as validation of this cleanup.
- No commits or pushes. Existing unrelated untracked directories were left alone.

## Suggested Follow-Up Order

1. Correct the misleading Jewel Harmony missing-file diagnostic.
2. Clarify quest reward group indices after establishing row-order examples.
3. Clarify `AddWindow` placement/failure control flow; verify the empty-window variant.
4. Separately reproduce the portrait drag-release observation in a correctness pass.
5. Leave unrelated stable legacy formatting and architecture for their own reviews.
