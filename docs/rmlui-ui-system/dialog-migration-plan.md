# `CommonMessageBox`/`CustomMessageBox` → `CGenericConfirmDialog`/`CGenericMenuDialog` migration

**Status: the two primitives are built and proven; the per-class porting checklist that used to
live in this file is done for all but a handful of classes.** This file used to carry the full
~1150-line worklist (every native dialog class, batch by batch, with its own "[x] ported" line) and
an iteration-by-iteration CSS-tuning narrative. Both are now superseded:

- **What's left to port** — `migration-ledger.md`'s "Dialog family" table is the current,
  accurate remaining-work list. Don't resurrect the old checklist here; that table is what's
  actually maintained.
- **How the primitives work, and the field/mechanism reference** — `component-catalog.md`'s
  "Dialog" section, which now points directly at `GenericConfirmDialog.h`/`GenericMenuDialog.h`'s
  own struct/class comments as the field reference (so it can't drift the way a doc copy would),
  plus the `KeepOpen()` veto pattern, the `item3D`-on-top-of-RmlUi foreground/background-context
  mechanism, and the multi-option-menu reentrant-`Show()`-during-click chaining pattern
  (Trainer/Gem-Integration/Elpis).
- **The button-role redesign** (`primary`/`secondary`/`cancel`, replacing the old positional
  `ButtonSet::Ok`/`OkCancel`) — documented in `GenericConfirmDialog.h`'s own struct comments now,
  not narrated here.

The full per-class history (which native class ported when, the specific bugs found along the way —
`CManager::CompareKeyEventOrder`'s descending-sort fix, the various CSS padding/rivet iterations,
etc.) is preserved in git history on this branch, not here — see `git log` for the individual
commits if that detail is ever needed. `component-catalog.md`'s Dialog section is where any new
finding about these two primitives should be added going forward, not a revived version of this
file.
