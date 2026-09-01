# Status Against the Architecture Principles

Living document — update this, not `architecture-principles.md`, when status changes. See
[`architecture-principles.md`](architecture-principles.md) first if you haven't read it; section
numbers below refer to it. [`legacy-theme-modernization.md`](legacy-theme-modernization.md)
(2026-09-02) amends it on one specific question — when legacy-theme C++ behavior should move into
RML/RCSS versus genuinely stay in C++ — worth reading before auditing any legacy-theme code
against this document.

## What's migrated

- **Login/character-select scene (`CWin` tier)** — fully migrated, no remaining
  legacy-`CWin`-rendered chrome: `CLoginWin`, `CLoginMainWin`, `CSysMenuWin`,
  `RememberPasswordPrompt`, `CCharSelMainWin`, `CCharMakeWin`, `CCharInfoBalloonMng`, `CMsgWin`.
  `COptionWin` ported but deliberately not wired up (confirmed unreachable in live play — see
  `README.md`'s Coexistence patterns). Built and verified against a real server, both themes.
- **In-game HUD (`CNewUIObj` tier)** — two pilots: `CMuHelperBar` (map/position readout + MU
  Helper bot control bar) and `CBuffStrip` (active-buff icon strip, the first `data-for`/dynamic-
  array pilot at this tier). Both built and verified against a real server, both themes. The rest
  of this tier — ~88 other `CNewUIObj` windows, including the main HP/MP/EXP/hotbar/skill-list
  HUD frame, drag-and-drop, and 3D-camera-space rendering — is not yet migrated.

## Checklist for every new port (principles §27's workflow, condensed to what to actually check)

1. **Layout intent documented and traceable to the original code's actual computed behavior**,
   not its literal default-case numbers (§2–3) — e.g. `CBuffStrip`'s centering was derived from
   solving `SetPos(int iScreenWidth)`'s four hardcoded pairs as `x = (iScreenWidth - 200) / 2`,
   not copied from one of the literal numbers; see `buff_strip.rml`'s own header comment for the
   full derivation, and keep pointing future sessions at it as the worked example.
2. Uses the `dp`-based anchor/stretch/center utility classes (`base.rcss`,
   `layout-and-scaling.md`) instead of C++-pushed `px` rects, unless the position is genuinely
   data-driven per-frame (`CCharInfoBalloon`'s carve-out). When binding a *computed* per-frame
   offset via `data-style-left`/`top`, match whatever unit the sibling static CSS in that same
   file actually uses — `dp` and `px` are **not** interchangeable (see "Findings" below).
3. Deliberate (not defaulted) aspect-ratio/resolution behavior: fixed, edge-anchored, centered, or
   stretch (§7–8).
4. C++ stays limited to state/binding/events/game behavior; RCSS owns layout/sizing/positioning
   (§1, §16).
5. RmlUi-facing asset naming, and now the C++ class name itself, reflects what the component
   actually is, not the legacy tier it came from (§12) — e.g. `mu_helper_bar`/`CMuHelperBar`, not
   `hero_position_info`/`CNewUIHeroPositionInfo`.
6. Both themes updated in the same pass, never one left behind (own standing instruction,
   independent of the principles doc). A rendering technique (e.g. an icon atlas) is verified to
   actually work at runtime before being trusted — see "Findings" below for a case where it
   didn't.
7. Uses reusable components/primitives where they exist; doesn't invent a new one-off mechanism
   when an existing pattern already covers the need — though see "Known gaps," several of the
   principles' presumed primitives don't exist yet on this branch.

## Findings worth knowing before the next port

These are empirical, RmlUi-build-specific gotchas found via direct runtime testing, not policy —
kept here rather than in `architecture-principles.md` because they're facts about *this engine's
vendored RmlUi build*, not general guidance. Tier-specific findings (the `CNewUIObj`-tier ones)
live in `newui-tier-adapter.md`'s "Proven by CBuffStrip" section in full detail; summarized here
for visibility:

- **`overflow:hidden` does not clip an absolutely-positioned oversized child in this RmlUi
  build**, even with `clip: always`. Use generated named `@spritesheet` rects instead for sprite
  atlases — see `newui-tier-adapter.md`.
- **`dp` and `px` are not interchangeable in `data-style-*` position bindings.** `dp` is scaled by
  the user's UI-scale setting, `px` never is — a `+'px'` binding drifts out of step with
  `dp`-sized siblings at any UI scale other than 100%.
- **An absolutely-positioned, `display:block`, multi-line (`white-space:pre-line`) box needs an
  explicit `width`** — left to shrink-to-fit, this build's width computation undersizes to the
  longest *word*, not the longest *line*.
- **A persistent RmlUi document needs its own scene-visibility gate.** `CNewUISystem` (the
  `CNewUIObj`-tier manager) is a persistent app-lifetime singleton whose `Update()`/`Render()`
  are only ever *called* during `MAIN_SCENE` — that used to be a complete visibility gate on its
  own; it stops being one once a window's visuals move to a persistent RmlUi document, which
  renders every frame regardless of scene. See `newui-tier-adapter.md`'s third `MAIN_SCENE`
  prerequisite.

## Known gaps against the principles (honest status, not yet built)

None of these are wrong so far — the principles doc explicitly endorses incremental delivery
(§26–27) — but they're real, currently-unaddressed gaps in the end-state architecture, not yet
even scheduled. Recorded so no future session mistakes "the pilots pass their own verification"
for "the full architecture is in place":

- **No mod/user-override resource-precedence system** (§18–19). Themes today are exactly two
  hardcoded directories (`themes/legacy/`, `themes/modern/`) selected by `GameConfig`'s theme
  name — no "user override on top of a theme" layer, no documented precedence order, no tooling
  for a third party to ship a partial theme that inherits the rest from a base theme.
- **No design-token/shared-variable layer** (§21). Each window's theme RCSS repeats its own
  `font-family`/`font-size`/color literals rather than referencing shared `ui-*` tokens. Whether
  this vendored RmlUi version supports CSS custom-property-style tokens hasn't been investigated.
- **No Custom/Test theme — direct, open conflict with §25.** §25 explicitly wants a Custom/Test
  theme that looks substantially different from Legacy, specifically to surface accidental
  component/presentation coupling. The standing instruction on this branch has been "only Legacy
  and Modern, no third theme." **Unresolved — ask before building a third theme either way.**
- **No systematic resolution × UI-scale × theme × drag-state validation matrix** (§25).
  Verification so far has been ad hoc per window (a handful of resolutions checked interactively;
  UI scale exercised incidentally — e.g. discovering the `dp`-vs-`px` bug during `CBuffStrip`
  verification, not through deliberate systematic testing at each of 75/100/125/150%). No test
  plan artifact exists that a future session could run through mechanically.
- **The existing drag system's interaction with theme-default-layout + UI-scale (§10–11) has not
  been explicitly audited** — carried over from `feature/rmlui-migration` unchanged, not
  re-examined against these specific concerns (does a dragged position survive a UI-scale change
  sensibly? A theme change?).
- **No reusable-component catalog exists as such** (§20) — `RmlModelBinder<T>`,
  `UI::RmlBridge` helpers, and the anchor/center/stretch RCSS classes are the closest things to
  reusable primitives today, all C++/CSS mechanisms rather than named, documented UI components
  (`Window`, `Panel`, `ItemSlot`, etc.) a future window's markup could just reference.
- **No rollout/phasing plan sequences the remaining ~88 `UI/NewUI` windows against the full
  checklist above.** Work has been pilot-by-pilot, each individually verified. This document is a
  first step toward tracking that, not a substitute for an actual sequenced plan if one is
  wanted.

## Pilots to revisit when the relevant phase arrives

Every already-shipped window that doesn't fully match the principles doc is **left as-is now,
not rewritten to chase each gap in isolation** (§26 — incremental, don't rewrite wholesale) —
but each specific deviation below is tied to whichever future initiative would naturally fix it,
so it gets folded into that pass instead of being forgotten. Check this list whenever starting
one of the trigger initiatives on the right.

| Window(s) | Known deviation | Revisit when... |
|---|---|---|
| `CMuHelperBar`, `CBuffStrip` (`mu_helper_bar.rcss`, `buff_strip.rcss`) | Repeat their own `font-family`/`font-size`/color literals inline instead of referencing shared tokens (§21) | A design-token/shared-variable layer is built — retrofit these two RCSS files to use it as the worked examples, don't leave them as the last two still on literals. |
| `CMuHelperBar`, `CBuffStrip` and every `CWin`-tier window | File location (`UI/NewUI/HUD/` etc.) and base class/tier boundary (`CNewUIObj`/`CWin`) unchanged (§12, "Tracked deferral" below) | A directory/base-class restructuring pass across `UI/NewUI` is undertaken — not before enough windows exist to know the real target shape (this is the existing tracked deferral, not new). |
| All `CWin`-tier windows, `CMuHelperBar`, `CBuffStrip` | No resolution × UI-scale × theme × drag-state validation matrix has been run against any of them (§25) — verification so far has been ad hoc per window | A validation-matrix/test-plan artifact is built — run it retroactively against every already-migrated window, not just new ones going forward. |
| All draggable migrated windows | Existing drag system's interaction with theme-default-layout + UI-scale (§10–11) has never been explicitly audited | The drag/preference-integration audit (itself an unstarted gap, above) happens — check these windows specifically, don't just audit the mechanism in the abstract. |
| `CBuffStrip` | Right-click-to-cancel not reproduced; tooltip is plain-text instead of the original's per-line-colored rich tooltip (both already documented as deliberate scope cuts in `newui-tier-adapter.md`, not silent gaps) | Right-click-distinct-from-left-click is proven generally in a `data-event-click` binding, or the three non-unified tooltip mechanisms (§12) get consolidated — whichever comes first. |
| `CNewUIMainFrameWindow` (`RenderLeftFrame()`/`RenderCenterFrame()`, `NewUIMainFrameWindow.cpp`) | Modern theme's flat background fill behind the still-legacy 3D-rendered potion/skill icons is drawn in C++ (`RenderColorQuadARGB`, gated on `GetActiveThemeName() == "modern"`), not RCSS — audited against `legacy-theme-modernization.md`'s classification and kept deliberately: RmlUi always composites its whole document as the frame's last pass, after those icons already rendered, so an RmlUi-drawn fill in that screen region would always paint *over* them, not behind — the same reason `#item_slots`/`#skill_slots` are border-only in RmlUi, never filled. The border lines that used to live alongside this same fill were **not** similarly exempt — moved to RmlUi (`#gauge_frame`) once audited, since a thin outline has no such paint-order constraint. Likewise the skill-hotkey-number subscript and the gauge current/max text, both fully retired from C++ this pass (`GetHotKeySlotNumber()`/`hp_current_text` etc. — pure theme-agnostic data, each theme's own markup decides what to show). | Phase 2/3 of this pilot (skill list, item hotkeys → real RmlUi) lands and this legacy chrome retires entirely, same as `UIScalePercent` row below. |
| `CNewUIMainFrameWindow` (`main_frame.rcss`, both themes — HP/MP/AG/SD/EXP bars + 5 corner buttons, first of 3 planned phases) | Doesn't respect `UIScalePercent` (§9, §23–24) — `bars_scale`/`bars_left`/`bars_top` derive entirely from `UI::Scaling::BottomHudScale()`/`BottomHudCenterTransform()`, a window-size-driven, 1x–2x-clamped auto-scale that has never read the config option (matches the original pre-RmlUi window's own behavior — this HUD band predates that setting). `main_frame.rcss` deliberately uses `px`, not `dp`, throughout for this reason, so it tracks `bars_scale` exactly instead of being scaled a second time. See `MainFrameRmlModel::barsLeft`'s header comment (`NewUIMainFrameWindow.h`) for the full reasoning. | Either (a) a scoped fix: fold `GameConfig::GetUIScalePercent()` into `UI::Scaling::BottomHudScale()` as a post-clamp multiplier — `NewUIMainFrameWindow.cpp/.h` are its *only* consumers codebase-wide (grepped), so every dependent transform (RmlUi bars/buttons/exp, the still-legacy chrome render, 3D potion-icon placement, and potion/skill click hit-testing) moves together automatically since they all already share this one function — no risk of the two-independent-calculators drift bug `CCharSelMainWin` hit (`layout-and-scaling.md`). This is `UI::Scaling`'s first-ever `GameConfig` dependency (currently pure geometry), and touches gameplay-critical click hit-testing, so it needs verifying by actually using a potion/skill at more than one `UIScalePercent` value *and* resolution before shipping, not just a visual check. Or (b) Phase 2/3 of this pilot (skill list, item hotkeys → real RmlUi, both still fully legacy today) lands and `BottomHudScale` is retired from this window entirely in favor of the branch's normal fixed-`dp`/`UIScalePercent` policy, same as every other migrated window. |

## Tracked deferral: C++ adapter classes still live in `UI/NewUI/HUD/` under `CNewUIObj`

Both `CNewUIObj`-tier pilots (`CMuHelperBar`, `CBuffStrip`) were renamed at port time — class
name and every `INTERFACE_*`/`CNewUISystem` member/accessor/macro referencing them — dropping
their legacy-tier names (§12). What's still deferred: the physical file location
(`UI/NewUI/HUD/`), the `CNewUIObj` base class/tier boundary itself, and collapsing the
`INTERFACE_*`-keyed lookup + `g_p*` macro pattern into something that doesn't require a per-window
case in a shared table. These are structural — they touch the other ~88 still-unported
`CNewUIObj` windows' shared machinery, not just the pilots so far — and stay premature with only 2
data points. Revisit once more of `UI/NewUI` is ported and the real shape of a unified directory
scheme is visible from real examples.

## Upstream sync log (PR #572)

This branch sits on top of `sven-n/MuMain` PR #572 (head: `yesid-bocanegra/MuMain:main`, the
SDL_GPU renderer branch) rather than `main` directly, since #572 hasn't merged yet. Log every
rebase onto a newer PR #572 head here — one line per sync, not one row per upstream commit. That's
a deliberately lighter shape than the SDL-migration branch's per-source-commit replay ledgers
(`docs/porting/*-ledger.md` on `pr572/main`): those exist because that branch replays an
independently-evolved commit history into a differently-restructured target and has to prove each
source commit's *behavior* survived the restructuring. We don't have that problem — this branch's
own commits are ours, `git log` already documents them faithfully, and each sync so far has been a
clean, non-overlapping rebase. If a future sync ever needs real reconciliation (upstream renames or
restructures a file this branch has also touched), that's the trigger to consider a heavier
per-commit ledger — not before.

| Date | Upstream commits pulled in | Conflict verdict | Resulting local tip |
|---|---|---|---|
| 2026-09-01 | `a9739fb2` docs(render): document Windows parity gaps (docs-only, 2 files, zero overlap with anything this branch touches) | Clean — verified in an isolated worktree before applying to the real branch; identical tree except the 2 upstream docs files | `878f35e4` |
