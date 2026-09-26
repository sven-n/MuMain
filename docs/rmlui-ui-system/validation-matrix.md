# Validation Matrix

The mechanical pass `STATUS.md`'s §25 gap says doesn't exist. Scoped deliberately narrow: it covers
the one failure mode that has actually shipped bugs, not every axis §25 names. Widen it when
something else bites, not speculatively.

## Why this exists, concretely

`UI::RmlBridge::RefreshLogicalPanelSize()` divided each window's `#panel` box by the active UI
scale. At scale 1.0 that is exactly correct, so it passed every check anyone ran. At the usual
capped 2.0 it shrank **17 windows'** native hit boxes to a quarter of their drawn size: clicks
outside the panel's top-left quadrant fell through to the world and walked the character, which in
the vault's case also closed the window. Every one of those windows is recorded in `STATUS.md` as
"built and verified against a real server, both themes." That verification was real. It was
single-scale.

`RefreshLogicalAnchorPosition()` carried the same defect on its own three callers.

The lesson generalizes past those two functions: **a window's native bookkeeping and its RCSS agree
trivially at scale 1.0 and can disagree at every other scale.** `layout-and-scaling.md`'s
`CCharSelMainWin` retrofit is the same bug from a different direction, found the same way. Verifying
at one scale is the check that does not catch this class.

## The pass

Two scales are enough to catch a scale-proportional divergence; the ladder's ends make an error
obvious rather than marginal. Set it in **Options → UI → UI scale**
(`UI::Options::UIScalePercentChoices()`, `OptionWindow.cpp` — the ladder is 50/60/70/80/90/100/125/
150/200). It applies live.

For each window below, at **50 %** and at **200 %**, in **both themes** (`$theme legacy` /
`$theme modern` from the console, or the Options theme picker):

1. **Click every interactive element** — action buttons, close box, tabs, and for inventory-family
   windows at least one item in each **corner** of the grid. Corners are the point: a
   proportional hit-box error leaves the top-left working and fails the far edges, so testing the
   middle proves nothing.
2. **Confirm the click registers on the window, not the world.** The tell for a miss is the
   character walking, since native world-movement reads `MouseLButtonPush` directly whenever
   `CheckMouseUse()` reports the mouse as free.
3. **Hover anything with a tooltip or popup** and confirm it appears anchored to its element rather
   than pulled toward the panel's top-left — that is the anchor-helper failure signature.

A window fails if any click lands in the world, any hit region stops short of the drawn art, or a
popup is visibly offset from what it belongs to.

## The static half is already done

Every `GetAbsoluteOffset()`/`GetBox()` read in UI code was audited for this bug class; three
instances existed and all three are fixed (`RefreshLogicalPanelSize`, `RefreshLogicalAnchorPosition`,
`CNPCQuest`'s condition icons). Four other sites read RmlUi geometry correctly, each for a different
reason — worth knowing, because "does this need a scale conversion?" has no single answer:

| Site | Why it's correct |
|---|---|
| `CGenericConfirmDialog` | `dp`-sized `.center-both` panel, no root transform — its box genuinely *is* screen px, so `LogicalX` is right |
| `CMainFrameWindow` | takes the delta against `centerTransform.offsetX`, then multiplies the scale back in at the point of use |
| `CNPCDialogue` | subtracts two siblings' offsets and divides by a pitch — units cancel |
| `RmlTooltip` | no root transform, and its C++ pre-multiplies the scale into the width it sets |
| `COptionWindow` | compares a screen-px rect against `MouseX/MouseY`, which is only safe because `INTERFACE_OPTION` maps to `LayoutMode::Legacy` (identity) in `UILayoutPolicy.cpp` — deliberately, after the same click-through was found live |

**So what's left is the part static analysis can't do**: confirming the fixes are right in the
running client, and catching anything that diverges for a reason no grep would reveal.

## Scope: the windows whose native geometry is derived from live RCSS

These are the `RefreshLogicalPanelSize()` / `RefreshLogicalAnchorPosition()` callers — every window
where a scale conversion sits between the theme and the hit test. Other migrated windows are not
exempt in principle, they simply have no such conversion to get wrong.

| Window | Family | Check |
|---|---|---|
| `CMyInventory` | inventory | grid corners, 4 corner buttons, drag by title bar |
| `CTrade` | inventory | both grids' corners |
| `CStorageInventory` | inventory | grid corners, 4 action buttons, close box |
| `CStorageInventoryExt` | inventory | grid corners |
| `CMixInventory` | inventory | grid corners |
| `CNPCShop` | inventory | grid corners |
| `CMyShopInventory` | inventory | grid corners, shop-name field focus + typing |
| `CPurchaseShopInventory` | inventory | grid corners |
| `CInventoryExtension` | inventory | grid corners |
| `CCharacterInfoWindow` | docked | 5 stat "+" buttons, Exit/Quest/Pet/Master-Level |
| `CPetInfoWindow` | docked | buttons, close box |
| `CPartyInfoWindow` | docked | member rows, close box |
| `CMyQuestInfoWindow` | docked | tabs, quest list rows, **reward popup anchor** |
| `CQuestProgress` | docked | pager L/R, answer rows, **reward popup anchor** |
| `CQuestProgressByEtc` | docked | pager L/R, answer rows, **reward popup anchor** |
| `CNPCDialogue` | docked | both pagers, sel-text rows |
| `CNPCQuest` | docked | answer rows (bg-document `#panel`); **condition item icons sit against their text rows** — these were misplaced by the same bug class, so this is the sharpest single check in the table |

## Axes deliberately not covered yet

Named so nobody mistakes this pass for the whole of §25:

- **Resolution.** Scale is the sharper probe — `PanelTransform` derives scale from resolution, so a
  resolution sweep mostly re-tests the same variable indirectly, and changing scale is far cheaper
  to drive. Add a resolution pass if a bug is ever found that a scale sweep misses.
- **Drag state across a scale/theme change.** Still open, still unaudited, `tracked-deferrals.md`
  has it. `CMyInventory` is the only draggable window today, and it stores its position in
  reference space, so it *should* survive by construction — untested.
- **Theme change while a window is open.** The `$theme` hot-swap smoke test `STATUS.md` records as
  pending is a different pass from this one.

## Results

Fill in per run. An empty cell is "not checked," which is not the same as passing.

| Date | Build | Scales | Themes | Windows checked | Result |
|---|---|---|---|---|---|
| | | | | | |
