# Tracked Deferrals

Split out of `STATUS.md` (2026-09-16) — forward-looking punch-lists, not a status narrative.
Each entry below names what's still incomplete and, where applicable, what future initiative
should fold it in. See [`STATUS.md`](STATUS.md) for what's actually done, and
[`engine-findings.md`](engine-findings.md) for engine-specific gotchas found along the way. The
`CommonMessageBox`/`CustomMessageBox` family port used to have its own tracked-deferral entry
here too — it's now just [`migration-ledger.md`](migration-ledger.md)'s Dialog family table
(2026-09-19), which is what's actually maintained.

## Pilots to revisit when the relevant phase arrives

Every already-shipped window that doesn't fully match the principles doc is **left as-is now,
not rewritten to chase each gap in isolation** (§26 — incremental, don't rewrite wholesale) —
but each specific deviation below is tied to whichever future initiative would naturally fix it,
so it gets folded into that pass instead of being forgotten. Check this list whenever starting
one of the trigger initiatives on the right.

| Window(s) | Known deviation | Revisit when... |
|---|---|---|
| `CMuHelperBar`, `CBuffStrip` (`mu_helper_bar.rcss`, `buff_strip.rcss`) | Repeat their own `font-family`/`font-size`/color literals inline instead of referencing shared tokens (§21) | A design-token/shared-variable layer is built — retrofit these two RCSS files to use it as the worked examples, don't leave them as the last two still on literals. |
| `CMuHelperBar`, `CBuffStrip` and every `CWin`-tier window | Base class/tier boundary (`mu::ui::window::CObject`/`CWin`) unchanged (§12, "Tracked deferral" below) — file location itself was resolved by the `UI/` directory restructure (`newui-legacy-merger.md`, 2026-09-05): `UI/NewUI/HUD/` is now `UI/HUD/` | A base-class/tier restructuring pass is undertaken — not before enough windows exist to know the real target shape (this is the existing tracked deferral, not new). |
| All `CWin`-tier windows, `CMuHelperBar`, `CBuffStrip` | No resolution × UI-scale × theme × drag-state validation matrix has been run against any of them (§25) — verification so far has been ad hoc per window | A validation-matrix/test-plan artifact is built — run it retroactively against every already-migrated window, not just new ones going forward. |
| All draggable migrated windows | Existing drag system's interaction with theme-default-layout + UI-scale (§10–11) has never been explicitly audited | The drag/preference-integration audit (itself an unstarted gap, above) happens — check these windows specifically, don't just audit the mechanism in the abstract. |
| `CBuffStrip` | Right-click-to-cancel not reproduced; tooltip is plain-text instead of the original's per-line-colored rich tooltip (both already documented as deliberate scope cuts in `newui-tier-adapter.md`, not silent gaps) | Right-click-distinct-from-left-click is proven generally in a `data-event-click` binding, or the three non-unified tooltip mechanisms (§12) get consolidated — whichever comes first. |
| `CMainFrameWindow` (`RenderLeftFrame()`/`RenderCenterFrame()`, `MainFrameWindow.cpp`) | Modern theme's flat background fill behind the still-legacy 3D-rendered potion/skill icons is drawn in C++ (`RenderColorQuadARGB`), not RCSS. **The paint-order reason is legitimate** — RmlUi always composites its whole document as the frame's last pass, after those icons already rendered, so an RmlUi-drawn fill in that screen region would always paint *over* them, not behind — the same reason `#item_slots`/`#skill_slots` are border-only in RmlUi, never filled. ~~Gated on the literal string `GetActiveThemeName() == "modern"` (§30 violation — a third theme wanting the same treatment silently wouldn't get it)~~ — **fixed 2026-09-04**: now gated on `UI::RmlBridge::ThemeProvidesOwnIconChrome()`, a declared theme capability (`themes/modern/theme.ini`). The border lines that used to live alongside this same fill were **not** similarly exempt — moved to RmlUi (`#gauge_frame`), since a thin outline has no such paint-order constraint. The skill-hotkey-number subscript and the gauge current/max text are both fully retired from C++ (`GetHotKeySlotNumber()`/`hp_current_text` etc. — pure theme-agnostic data, each theme's own markup decides what to show). | Icon/box-frame art for the skill grid/pet row stayed legacy 2D too (`RenderSkillIcon()`'s atlas lookup is too irregular — mixed 8/12-column addressing, a separate master-level atlas — to port blind without a way to visually verify against the real decoded `.OZJ` textures). Retires once a separately-scoped icon-atlas port lands **or** the render-ordering investigation ("Known gaps") finds interleaving is possible, whichever comes first. |
| `CSkillList::RenderCurrentSkillAndHotSkillList()` (`MainFrameWindow.cpp`, still fully legacy) | The selected-skill-slot highlight. **Same paint-order reasoning as the row above** (modern's RmlUi `.selected` outline always paints on top of it since RmlUi composites last, so drawing the legacy `IMAGE_SKILLBOX_USE` sprite unconditionally would double up the highlight for modern; legacy genuinely wants the real sprite, which has no RmlUi asset equivalent ported yet) — legitimate reason. ~~Gated on `GetActiveThemeName() != "modern"` (§30 violation, same as the row above)~~ — **fixed 2026-09-04**, same `ThemeProvidesOwnIconChrome()` capability as the row above (inverted). Same exception class as the row above, not a separate issue; the expanded grid's own box-frame draw (`Render()`, a distinct call site) got the identical treatment for the same reason — modern's grid cells use a plain CSS border (`.skill-cell`, `main_frame.rcss`) instead of `IMAGE_SKILLBOX`/`IMAGE_SKILLBOX_USE`; legacy keeps the real sprite for both call sites. | Same icon-atlas-port (or render-ordering) follow-up as the row above retires this. |
| `main_frame.rml` (both themes) | Two independently-maintained RML files, not the one-shared-RML-per-window pattern every other migrated window uses — `theming-and-modding.md`'s "Forking a theme's RML" section documents why and the criteria for when this is legitimate. The two files' shared ids/classes/bindings require hand-sync, called out in each file's own header comment — see "Known gaps" for the drift-check tooling this still doesn't have. | Either a cleaner RCSS-only structural fix is found and one file retires, or this is accepted long-term and the same criteria get applied consistently if another window ever needs it — not before a second real case shows up. |
| `CMainFrameWindow` (`main_frame.rcss`, both themes — HP/MP/AG/SD/EXP bars + 5 corner buttons) | `UI::Scaling::BottomHudScale()`/`CappedUniformScale()` (`UITransform.cpp`) fold `GameConfig::GetUIScalePercent()` in as a post-clamp multiplier, applied in the shared function itself so every caller codebase-wide (RmlUi bars/buttons/exp via `bars_scale`, the still-legacy chrome render, 3D potion-icon placement, and potion/skill click hit-testing) moves together automatically. Also folds `UI::Scaling::GetWindowContentScale()` (OS display-scale/pixel-density factor) into RmlUi's own `dp` ratio (`RmlUiRuntime.cpp`'s `ApplyUIScale()`) — **confirmed 2026-09-07 on a 125%-scaled display; see `layout-and-scaling.md`.** `main_frame.rcss` still deliberately uses `px`, not `dp`, throughout, tracking `bars_scale` exactly instead of being scaled a second time. | The `UIScalePercent` half needs verifying by actually using a potion/skill at more than one `UIScalePercent` value *and* resolution, not just a visual check. Phase 3 (item hotkeys → real RmlUi) landing, plus a follow-up icon-atlas port for the Phase 2 skill grid/pet row's still-legacy icon art (see the two rows above), still eventually retires `BottomHudScale` from this window entirely in favor of the branch's normal fixed-`dp`/`UIScalePercent` policy. |
| `CMyInventory` (equipment paperdoll — `RenderEquippedItem()`, still fully native) | Background sprite, durability tint, and drag-compatibility highlight all paint *behind* the equipped item's live 3D icon today (native paint order); RmlUi's main context always composites last, so a straight port would paint them *in front of* instead — a real regression, not a straight port (Stage 2 was scoped, investigated, and deliberately skipped for this reason — see "What's migrated" above). | A background-context consolidation pass makes this mechanism reliable enough to trust with more per-frame-varying, class-conditional content, **or** the equipment grid gets its own future chrome pass anyway and folds this in at the same time — whichever comes first. If pursued alone, the static background sprite (no gameplay-state binding) is the only piece with a reasonable cost/value ratio on its own. |
| `CMyInventory` (`my_inventory.rcss`, legacy theme only) | The 4 corner buttons (RmlUi, always renders last) can end up on top of `CInventoryCtrl`'s native item tooltip when a bottom-row item's tooltip extends into the button strip — before Stage 1 both were native, ordinary same-frame paint order put the tooltip on top. Confirmed cosmetic, not functional; user explicitly deferred it. | Either the tooltip is rerouted through the post-RmlUi seam (shared infra, also used by Trade/Storage/Shop — too broad for this window alone), or a future grid-chrome pass makes the tooltip an RmlUi element too, resolving it for free via DOM order. |
| ~~All modern-theme `.rcss` files~~ | **Superseded 2026-09-10**: the entire modern-theme token layer was renamed and revalued a second time (cool-steel → blackened-iron/dark-forged-metal, a real design-system consolidation, not just a value refresh — see `modern-theme-visual-direction.md`'s "Second generation" note). Real duplication was also consolidated: `login.rcss`'s own-copy `.btn`/`.btn-ok`/`.checkbox-box` and `login_main.rcss`/`char_sel_main.rcss`'s independent `.btn-icon` copies were deleted in favor of `base.rcss`'s shared versions. New shared primitives added: `.btn-icon`, structured tooltip BEM classes, `.slot`/`.slot--filled`/`.slot--selected`. HUD gauge colors promoted from literal hex to `resource-hp`/`-mp`/`-sd`/`-ag` tokens (layout unchanged — see the next row). | Resolved — no further action, unless the tokens change again. |
| HUD circular glass-orb + wrapping arc gauges (reference visual study, not yet built) | The 2026-09-10 iron-palette migration deliberately retinted `main_frame.rcss`'s existing rectangular HP/MP/AG/SD bars rather than rebuilding them as circular orbs/arcs — that's a structural rebuild (new markup, new `CMainFrameWindow` C++ binding shape, new tooltip anchors, interacts with `main_frame_bg.rcss`'s paint-order mechanism and `BottomHudScale()`), not a retint, and touches live combat UI. Two RmlUi-native techniques were confirmed viable for it (`<progress direction="clockwise">` for the arcs via real octant geometry, layered `radial-gradient` for the orb liquid) but not used yet. | A dedicated, focused pass scoped just to this, once explicitly prioritized — see `modern-theme-visual-direction.md`'s "Known follow-up" section. |

## Tracked deferral: C++ adapter classes still on the `mu::ui::window::CObject` tier

Both `mu::ui::window::CObject`-tier pilots (`CMuHelperBar`, `CBuffStrip`) were renamed at port time — class
name and every `INTERFACE_*`/`CSystem` member/accessor/macro referencing them — dropping
their legacy-tier names (§12). What's still deferred: the `mu::ui::window::CObject` base class/tier boundary
itself, and collapsing the `INTERFACE_*`-keyed lookup + `g_p*` macro pattern into something that
doesn't require a per-window case in a shared table. (The physical file location half of this —
`UI/NewUI/HUD/` — was resolved separately by the `UI/` directory restructure,
`newui-legacy-merger.md`, 2026-09-05: that folder no longer exists, its contents are now
`UI/HUD/`, a pure move with no base-class/tier change.) The base-class/tier boundary and
`INTERFACE_*` pattern are structural — they touch the other ~88 still-unported `mu::ui::window::CObject`
windows' shared machinery, not just the pilots so far — and stay premature with only 2 data
points. Revisit once more of those windows are ported to RmlUi and the real shape of a unified
base class is visible from real examples.

## Tracked deferral: `CMainFrameWindow`'s own class rename — naming half resolved, file-split half still open

A distinct deferral from the one above — different reasoning, don't conflate the two.

`CMuHelperBar`/`CBuffStrip` were each renamed at port time (class name and every referencing
`INTERFACE_*`/`CSystem` member/accessor/macro), per the checklist above and
[`newui-tier-adapter.md`](newui-tier-adapter.md)'s Naming section. `CNewUIMainFrameWindow` (Phase 1
of its own 3-phase pilot, `main_frame.rml`/`.rcss`) was originally **not** renamed when ported —
the plan was to hold that rename until Phase 3 landed, so the file's still-legacy classes
wouldn't sit mismatched against an already-renamed one for however long Phase 2/3 took.

**That plan was overtaken by `newui-legacy-merger.md`'s Phase 5 (2026-09-05)**: its
mechanical, repo-wide prefix-drop renamed every `CNewUI*`/`INewUI*` identifier in the whole tier in
one blanket pass, with no per-file carve-out for this deferral — so `CNewUIMainFrameWindow`,
`CNewUISkillList`, and `CNewUIItemHotKey` all became `CMainFrameWindow`/`CSkillList`/`CItemHotKey`
together, incidentally, alongside the ~88 other windows' renames. The naming mismatch this
deferral was protecting against never actually happens now — all three names moved in the same
commit. **What's still genuinely open, unrelated to naming**: `MainFrameWindow.cpp/.h` still welds
three classes together — `CMainFrameWindow` (ported, Phase 1), `CSkillList` (still fully legacy,
Phase 2), `CItemHotKey` (still fully legacy, Phase 3) — one file serving three different pilot
phases. Whether that one-file-three-classes shape is itself worth splitting (e.g. once Phase 3
lands and all three are ported) is a real, still-unmade decision; revisit it then, but it's a
file-organization question now, not a naming one.

## Tracked deferral: `CUIControl` family (`UIControls.h`) full retirement

Not a permanent third toolkit alongside RmlUi and `mu::ui::window` — a fully enumerable, closeable
checklist (`ui-target-architecture.md` item 17's "concrete instance"). Found and scoped
2026-09-13 while investigating whether porting Friend/Mail would let this family retire. It
wouldn't — Friend/Mail (`CUIWindowMgr`/`CUIBaseWindow`, `UI/Party/UIWindows.cpp`) is only one of
four independent pieces still keeping this file alive:

1. **`CUITextInputBox`** — permanent until RmlUi gets native `<input>`/`<textarea>` (Section E's
   Type-2 companion; IME composition through RmlUi's DOM is the open design question, no target
   date). Not part of this checklist's "close it out" scope — this piece stays regardless.
2. **`CUITextListBox<T>`** (~18 subclasses in `UIControls.h`) — no rule named this class before
   2026-09-13 (only `CUIButton` was named), which is exactly why it kept gaining consumers even on
   windows already on `mu::ui::window::CObject`. Confirmed live consumers found this session:
   - `CGuildInfoWindow` (`Guild/GuildInfoWindow.h`) — `CUINewGuildMemberListBox`
   - `CMixInventory` (`UI/Inventory/MixInventory.h`) — `CUISocketListBox`, `CUIUnmixgemList`
   - `CInGameShop` (`GameShop/InGameShop.h`) — `CUIInGameShopListBox`, `CUIBuyingListBox`,
     `CUIPackCheckBuyingListBox`
   - `QuestProgress.h`/`QuestProgressByEtc.h` — `CUICurQuestListBox`/`CUIQuestContentsListBox`
     (the same two `CMyQuestInfoWindow` already ported off; natural next targets, same pattern)
   - Guild/chat/letter/socket variants (`CUIGuildListBox`, `CUISimpleChatListBox`,
     `CUIChatPalListBox`, `CUIWindowListBox`, `CUILetterListBox`/`CUILetterTextListBox`,
     `CUIGuildNoticeListBox`, `CUIUnionGuildListBox`, `CUIExtraItemListBox`,
     `CUIBCDeclareGuildListBox`/`CUIBCGuildListBox`, `CUIMoveCommandListBox`) — not yet traced to
     live call sites individually; treat as live until checked, same discipline as the confirmed
     ones above.

   The replacement pattern is proven, not speculative: RmlUi's `data-for` binding, already used by
   `CBuffStrip` and by `CMyQuestInfoWindow`'s own port off two of these classes. Each remaining
   subclass is an independent, same-shape port — see `ui-target-architecture.md` item 8b / Rule 11.
3. **`CUIButton`** — down to one known live cluster now. `CUIPopup` (`g_pUIPopup`, `UI/Dialogs/
   UIPopup.h`) had its `POPUP_OK`/`POPUP_YESNO` call sites (the ones duplicating
   `CGenericConfirmDialog`'s job) ported off it 2026-09-13 (see "Tracked deferral:
   `CommonMessageBox`/`CustomMessageBox`" below) — but `CUIPopup` itself is **not** retired: one
   live `POPUP_CUSTOM` call site remains (`UIGuildInfo.cpp`'s "Appoint" sub-guild-master/
   battle-master picker, a bespoke multi-option menu out of `CGenericConfirmDialog`'s scope), so
   its 4 `CUIButton` members (`m_OkButton`/`m_CancelButton`/`m_YesButton`/`m_NoButton`) are now
   unreachable dead weight but the class itself stays. The other cluster,
   `CUIGuildInfo`/`CUIGuildMaster` (`Guild/UIGuildInfo.h`/`UIGuildMaster.h`), is suspected dead —
   see below (note: `CUIGuildMaster`'s dead `ReceiveGuildRelationShip`/`CloseMyPopup`/two popup-id
   members were already deleted 2026-09-13 as part of the `CUIPopup` port, superseded by
   `CGuildInfoWindow`'s own port earlier — the rest of the "suspected dead" verification below
   still applies to what's left of both classes).
4. **`CUIGuildInfo`/`CUIGuildMaster`** (`CUIControl`-rooted standalone windows, distinct from the
   live `CGuildInfoWindow`/`CGuildMakeWindow` pair on `mu::ui::window::CObject`) — a full-codebase
   grep found **zero instantiations of either class anywhere** (no `new`, no member declaration in
   any other type). Strong circumstantial evidence of dead code superseded by
   `CGuildInfoWindow`/`CGuildMakeWindow`, same shape as `CWin`/`::CButton`/`CSlider` before they
   were deleted — but not yet given that same exhaustive verification pass. Do that verification
   before deleting anything.

**Related finding, same investigation**: `CUIManager`/`g_pUIManager` (`UI/Core/UIManager.h/.cpp`)
looks like a live top-level manager parallel to `mu::ui::window::CManager` — it isn't. Its
`Render()` and `UpdateInput()` method bodies are both literally empty. Its `MUTEX_*` enum lists
~30 interfaces (including `MUTEX_TRADE`/`MUTEX_STORAGE`/`MUTEX_GUILDINFO`/`MUTEX_NPCSHOP` — windows
long since migrated to `mu::ui::window::CManager`) but `Open()`/`IsOpen()` only actually implement
4 of them (`MUTEX_INVENTORY`, `MUTEX_PERSONALSHOPSALE`, `MUTEX_PERSONALSHOPPURCHASE`,
`MUTEX_SERVERDIVISION`); everything else falls through to `default: return false`. What's actually
still real: it constructs/owns `g_pUIPopup`/`g_pUIGateKeeper`/jewel-harmony/item-add-option-info as
globals, and `IsInputEnable()` is a genuinely still-consulted query. Worth knowing mainly so a
future session doesn't mistake the `MUTEX_*` enum for a live, comprehensive policy layer — most of
it is vestigial. Not in this retirement checklist's scope (it's not `UIControls.h`), but touches
the same investigation and the same `g_pUIPopup` dependency as item 3 above.

