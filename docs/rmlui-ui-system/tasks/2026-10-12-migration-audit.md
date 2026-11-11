# RmlUi migration audit tasks

Status: all five fixes implemented, automated validation passed, and the user confirmed
in-game verification on 2026-10-12.

Scope: `src/source/UI`, `src/source/Render/RmlUi`, related consumers and
`src/bin/Data/Interface/RmlUi` assets. Expected use is one desktop client with
frequent frame updates, theme switches, and window resizing.

The findings below were established by tracing code paths, without in-game
reproduction. Recheck the current implementation before fixing each task, since
other migration work may change these paths. Task numbers match the audit report.

## Must fix

### 1. Restore Enter and Escape in confirmation dialogs with text fields

- [x] Implement the fix.
- [x] Add a regression test for focused-document ownership and Enter/Escape callbacks.
- [x] Verify in-game (user confirmed).

**What this is:** The reusable confirmation dialog handles price entry, friend
names, and other text input.

**Problem:** It focuses its RmlUi field but does not override `TakesTypingFrom()`.
The manager therefore skips its keyboard handler while that field is focused;
Enter and Escape never reach the confirmation or cancellation logic.

**Fix:** Claim the dialog's own document through `TakesTypingFrom()`.

**If we skip it:** Players must use mouse buttons to finish or dismiss these
text-entry dialogs.

**Verification:** Route Enter and Escape through the manager with `gcd_input`
focused. Check that the expected callback runs once and that the key does not
reach another window. Exercise personal-shop price entry and friend-name entry.

**Code:** [GenericConfirmDialog.h](../../../src/source/UI/Dialogs/GenericConfirmDialog.h),
[GenericConfirmDialog.cpp](../../../src/source/UI/Dialogs/GenericConfirmDialog.cpp),
[WindowManager.cpp](../../../src/source/UI/Core/WindowManager.cpp).

### 2. Preserve open scene-dialog state across resizing

- [x] Implement the fix without recreating scene windows on resize.
- [x] Add a regression test for open creation visibility and pending message actions.
- [x] Verify scene-dialog behavior in-game (user confirmed).

**What this is:** Login and character scenes rebuild their windows after resizing
or a display-scale change.

**Problem:** The character branch recreates everything without restoring
visibility, closing character creation and message dialogs. The login branch
restores visibility, but `CMsgWin::Create()` clears the message text, type, and
action code before restoration.

**Fix:** Separate repositioning and sprite refresh from scene initialization.
Preserve open dialogs, their contents, selected character class, and input across
resizing.

**If we skip it:** Resizing can interrupt character creation or replace an
actionable login message with a blank dialog.

**Verification:** Resize with character creation open after entering a name and
choosing a class; both must remain intact. Resize with login and character-scene
messages open; visibility, text, and confirmation behavior must remain intact.
Repeat after a display-scale change.

**Code:** [SceneUICoordinator.cpp](../../../src/source/UI/Core/SceneUICoordinator.cpp),
[CharMakeWin.cpp](../../../src/source/Character/CharMakeWin.cpp),
[MsgWin.cpp](../../../src/source/UI/Windows/MsgWin.cpp),
[Winmain.cpp](../../../src/source/App/Platform/Windows/Winmain.cpp).

### 3. Fall back when the saved theme is missing or invalid

- [x] Implement the fix.
- [x] Add a regression test for missing themes, broken ancestry, cycles, and invalid fallback.
- [x] Verify startup in-game (user confirmed).

**What this is:** Startup reads the selected theme from the saved configuration.

**Problem:** The initial selection bypasses `ThemeExists()`. If a custom theme or
its base disappears, shared markup can still load, but relative stylesheets
resolve through the invalid theme.

**Fix:** Validate the saved theme before loading documents and fall back to
`legacy`, logging the reason once. Report clearly if the fallback is also invalid.

**If we skip it:** Removing a previously selected theme can produce unstyled
windows, oversized images, and misplaced controls on the next launch.

**Verification:** Start with a missing saved theme, a missing base, and a cyclic
inheritance chain. Each must choose the valid fallback before documents load.
Confirm that a valid sparse child theme remains selected and that logging does
not repeat for every resource.

**Code:** [RmlTheme.cpp](../../../src/source/UI/RmlBridge/RmlTheme.cpp),
[ThemeInheritance.cpp](../../../src/source/UI/RmlBridge/ThemeInheritance.cpp),
[ThemeSelection.cpp](../../../src/source/UI/Theme/ThemeSelection.cpp).

### 4. Keep theme selection stable when the available list changes

- [x] Implement selection against the displayed snapshot and defer switches by theme name.
- [x] Add a regression test for list changes and removal before applying a selection.

**What this is:** Options refreshes the theme list while the window remains open.

**Problem:** Labels update, but the selected theme remains a numeric index. Adding
a theme that sorts before the active theme shifts the list, so the displayed
selection becomes wrong; clicking that index can also be ignored as already
selected. A pending selection is likewise resolved against a newly discovered
list rather than the one that produced the click.

**Fix:** Keep active and pending selections by theme name. Recalculate the
displayed index against the same list used to render the choices.

**If we skip it:** Installing or removing themes while Options is open can
mislabel the active theme or apply an unintended selection.

**Verification:** Add and remove themes before the active entry alphabetically.
The selected label must still represent the active theme. Change the list between
a click and its deferred application; apply the clicked theme by name, or reject
it if it no longer exists.

**Code:** [OptionWindow.cpp](../../../src/source/UI/Options/OptionWindow.cpp).

## Should fix

### 5. Stop scanning and parsing theme files every frame

- [x] Refresh on opening Options, rebuilding its document, or applying a theme selection.
- [x] Add a regression test for refresh timing; theme discovery is absent from the frame loop.

**What this is:** Every options model update discovers themes and reloads their
display names.

**Problem:** Discovery enumerates directories, reads each inheritance chain's
configuration and tokens, and opens its base stylesheet. Unchanged themes repeat
this filesystem work at the client's frame rate.

**Fix:** Refresh the theme list when Options opens and after theme changes. If
live discovery must remain, poll occasionally instead of every frame. Coordinate
this change with task 4 so labels and theme identities use one list.

**If we skip it:** Theme discovery adds avoidable file reads, parsing, and
allocations to the frame loop.

**Verification:** Confirm that unchanged frame updates do not rescan theme files.
Reopening Options must pick up added or removed themes; if periodic discovery is
retained, verify its interval and stable selection behavior.

**Code:** [OptionWindow.cpp](../../../src/source/UI/Options/OptionWindow.cpp),
[RmlTheme.cpp](../../../src/source/UI/RmlBridge/RmlTheme.cpp),
[ThemeInheritance.cpp](../../../src/source/UI/RmlBridge/ThemeInheritance.cpp).

## Validation and limits

The audit ran the existing RelWithDebInfo UI tests: **141 passed**. The **30
contract tests** passed, along with syntax, theme-drift, and geometry checks
covering **144 RML and 239 RCSS files**. These checks did not cover the reported
integration cases; passing them does not resolve the tasks above.

After fixes, build RelWithDebInfo and rerun the relevant regression tests, UI
suite, and asset checks. Complete the in-game checks for keyboard handling,
scene resizing, and startup fallback.

Implementation validation:

- RelWithDebInfo client and UI test executables built successfully.
- All **145 UI tests passed**, including four new regression cases covering the five tasks.
- All **30 contract tests passed**.
- Syntax, theme-drift, bound-geometry, and layout-transform guards passed.
- The headless view fixtures now supply a valid minimal theme and restore it before shutdown.

Not checked during the audit: in-game reproduction, GPU output, operating-system
IME behavior, and every migrated window's individual gameplay flow. Manual
review concentrated on shared lifecycle, input, theme loading, rendering, and
representative consumers.

Implementation keeps the displayed theme list stable while Options is open. Reopen Options to
discover added or removed themes; switching a theme also refreshes the list. If a pending
theme disappears before its switch applies, the active theme remains selected.
The Options window also no longer loads or releases its retired native textures; its themed
documents own its visuals, and other native texture users load their own resources.

The user confirmed the in-game verification checklist on 2026-10-12. Individual
results for optional display-scale and broken-inheritance checks were not recorded.
