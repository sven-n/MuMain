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
| `CBuffStrip` (`buff_strip.rcss`, both themes) | Both themes now have a token layer (`modern` since 2026-09-04, `legacy` since 2026-09-23), and `mu_helper_bar.rcss` was retrofitted in both (`font-family`/tooltip `background-color`/`border`/`color`/`border-radius` all now `token(...)`) -- `buff_strip.rcss` still repeats its own `.tooltip`'s `color: #ffffff` (modern) / now-tokenized `color` (legacy, done alongside `mu_helper_bar.rcss` in the same pass) inline instead of referencing `token(text-primary)`, the same stray-override pattern `mu_helper_bar.rcss` had (base.rcss's own shared `.tooltip` rule already sets this token as the default). `font-size` stays literal everywhere in both files -- no `font-size` token category exists system-wide yet, not unique to these two. | Retrofit `buff_strip.rcss`'s modern `.tooltip` `color` the same one-line way `mu_helper_bar.rcss` was fixed, next time this file is touched -- trivial once picked up, just not bundled into the CMuHelperBar-scoped pass that fixed its sibling. A `font-size` token category is a separate, larger, not-yet-prioritized initiative (would touch far more than these two files). |
| `CMuHelperBar`, `CBuffStrip` and every `CWin`-tier window | Base class/tier boundary (`mu::ui::window::CObject`/`CWin`) unchanged (§12, "Tracked deferral" below) — file location itself was resolved by the `UI/` directory restructure: `UI/NewUI/HUD/` is now `UI/HUD/` | A base-class/tier restructuring pass is undertaken — not before enough windows exist to know the real target shape (this is the existing tracked deferral, not new). |
| All `CWin`-tier windows, `CMuHelperBar`, `CBuffStrip` | No resolution × UI-scale × theme × drag-state validation matrix has been run against any of them (§25) — verification so far has been ad hoc per window | A validation-matrix/test-plan artifact is built — run it retroactively against every already-migrated window, not just new ones going forward. |
| All draggable migrated windows | Existing drag system's interaction with theme-default-layout + UI-scale (§10–11) has never been explicitly audited | The drag/preference-integration audit (itself an unstarted gap, above) happens — check these windows specifically, don't just audit the mechanism in the abstract. |
| `CBuffStrip` | Right-click-to-cancel not reproduced; tooltip is plain-text instead of the original's per-line-colored rich tooltip (both already documented as deliberate scope cuts in `newui-tier-adapter.md`, not silent gaps) | Right-click-distinct-from-left-click is proven generally in a `data-event-click` binding, or the three non-unified tooltip mechanisms (§12) get consolidated — whichever comes first. |
| `CMainFrameWindow` (`RenderLeftFrame()`/`RenderCenterFrame()`, `MainFrameWindow.cpp`) | Modern theme's flat background fill behind the still-legacy 3D-rendered potion/skill icons paints through the background context (`#bg_left`/`#bg_center`, `main_frame_bg.rml`), not C++, since a paint-order-legitimate reason blocks the ordinary main context — RmlUi always composites its whole document as the frame's last pass, after those icons already rendered, so an RmlUi-drawn fill in that screen region through the main context would always paint *over* them, not behind — the same reason `#item_slots`/`#skill_slots` are border-only in RmlUi, never filled. ~~Gated on the literal string `GetActiveThemeName() == "modern"` (§30 violation — a third theme wanting the same treatment silently wouldn't get it)~~ — **fixed 2026-09-04**: now gated on `UI::RmlBridge::ThemeProvidesOwnIconChrome()`, a declared theme capability (`themes/modern/theme.ini`). The border lines that used to live alongside this same fill were **not** similarly exempt — moved to RmlUi (`#gauge_frame`), since a thin outline has no such paint-order constraint. The skill-hotkey-number subscript and the gauge current/max text are both fully retired from C++ (`GetHotKeySlotNumber()`/`hp_current_text` etc. — pure theme-agnostic data, each theme's own markup decides what to show). **Update, 2026-09-23**: `RenderCenterFrame()`'s last remaining native quad — the modern-only translucent skill-list-open highlight (`RenderColorQuadARGB(222, kHudTop, 160, 40, 0x40FFFFFFu)`) — also moved into the same background document (`#skill_list_highlight`, `MainFrameBgRmlModel::skillListOpen`); it had no icon-atlas dependency at all, just the same paint-order constraint as the fill above, so it was a plain Group-A fix, not part of the atlas port below. | Icon/box-frame art for the skill grid/pet row stayed legacy 2D too (`RenderSkillIcon()`'s atlas lookup is too irregular — mixed 8/12-column addressing, a separate master-level atlas — to port blind without a way to visually verify against the real decoded `.OZJ` textures). Retires once a separately-scoped icon-atlas port lands **or** the render-ordering investigation ("Known gaps") finds interleaving is possible, whichever comes first. |
| `CItemHotKey::RenderItems()`, `CSkillList::RenderCurrentSkillAndHotSkillList()` (`MainFrameWindow.cpp`) | **New finding, 2026-09-23**: both hardcode the item-hotkey/skill-hotkey-row/current-skill slot origin+pitch as plain literals (`x = 10 + i*38, y = 443, 20x20`; `x = 190, y = 431, width = 32` then `+6/+6` per icon; `x = 392, y = 437, 20x28`) that both themes' `main_frame.rcss` independently duplicate for `#item_slot_0..3`/`#skill_slot_0..4`/`#current_skill_slot` — confirmed by direct value comparison, not just suspected. Unlike the grid/pet-row row below, this duplication has **no** technical dependency on `RenderSkillIcon()`'s atlas addressing (`RenderItem3D()`/`RenderSkillIcon()` don't care where their x/y come from) — but deriving it live safely requires either new anchor markup plus this file's own `GetAbsoluteOffset()`-minus-transform-offset conversion (verified error-prone without visual testing: modern's icon-to-frame insets aren't uniformly formulaic — item slots pad 4px/12px, current-skill pads 4px/4px, neither a simple center formula) or RmlUi's untested-in-this-codebase `Element::GetRelativeOffset()`. Deferred rather than risking a silent per-theme pixel regression with no way to visually verify it this session. Both sides now carry an explicit `PINNED (tracked-deferrals.md, "hotkey slot pitch/size")` cross-reference comment instead (`MainFrameWindow.cpp` and both themes' `main_frame.rcss`) — a real, current theme-coupling gap (a theme wanting different hotkey pitch/spacing still needs a C++ change too), just not fixed blind. | The `RenderSkillIcon()` atlas port below, since that pass will already need to re-verify every one of these positions by eye once the icon draw itself changes — fold this fix into the same pass instead of doing it twice. |
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
`UI/NewUI/HUD/` — was resolved separately by the `UI/` directory restructure: that folder no
longer exists, its contents are now `UI/HUD/`, a pure move with no base-class/tier change.) The
base-class/tier boundary and
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

**That plan was overtaken by a later mechanical, repo-wide prefix-drop** that renamed every
`CNewUI*`/`INewUI*` identifier in the whole tier in one blanket pass, with no per-file carve-out
for this deferral — so `CNewUIMainFrameWindow`,
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

1. **`CUITextInputBox`** — **transitional now, not permanent.** The old framing ("permanent until
   RmlUi gets native `<input>`") was wrong on both halves: the vendored RmlUi already ships
   `<input>`, and the IME question is answered — `RmlUiRuntime` installs the vendored
   `TextInputMethodEditor_SDL` and `RmlUiSystemInterface::ActivateKeyboard()` drives
   `SDL_SetTextInputArea`/`SDL_StartTextInput`, so composition and candidate placement are handled
   centrally. `CMyShopInventory` is migrated (stock `<input>` + shared `.text-field`, see
   `component-catalog.md`). **Not globally retired** — remaining consumers, each with its own extra
   requirement beyond My Shop's:
   - ~~`CGenericConfirmDialog::Mode::Text`~~ — **migrated.** Per-`Show()` configuration lands via
     `ApplyInputFieldConfig()`; `type` must be set before the value, since changing it rebuilds the
     element's `InputType` and drops what it held. Its **`Mode::NumericKeypad` remains a separate
     interaction, not text input** — the shuffled on-screen keypad is deliberate anti-keylogger
     behaviour and must not become `<input type="number">`.
   - ~~`CLoginWin`~~ and ~~`CCharMakeWin`~~ — **both migrated.** Login's Tab order needed no code
     at all (`ElementDocument` handles `KI_TAB` and `WidgetTextInput` lets it bubble, so the
     reciprocal `SetTabTarget()` pair just went away); select-all-on-error-recovery maps to
     `ElementFormControlInput::Select()`, reached through `CLoginWin::FocusUsername()`/
     `FocusPassword()` which replaced the widget-pointer accessors external code used to call.
   - Chat (`CUIChatInputBox`), `CGuildMakeWindow`, `CGoldBowmanWindow`, `WindowMuHelper`,
     `MsgBoxIGSSendGift` — multiline/history/numeric variants, unscoped.
2. **`CUITextListBox<T>`** (~18 subclasses in `UIControls.h`) — no rule named this class before
   2026-09-13 (only `CUIButton` was named), which is exactly why it kept gaining consumers even on
   windows already on `mu::ui::window::CObject`. Confirmed live consumers found this session:
   - `CGuildInfoWindow` (`Guild/GuildInfoWindow.h`) — `CUINewGuildMemberListBox`
   - `CMixInventory` (`UI/Inventory/MixInventory.h`) — `CUISocketListBox`, `CUIUnmixgemList`
   - `CInGameShop` (`GameShop/InGameShop.h`) — `CUIInGameShopListBox`, `CUIBuyingListBox`,
     `CUIPackCheckBuyingListBox`
   - ~~`QuestProgress.h`/`QuestProgressByEtc.h` — `CUIQuestContentsListBox`~~ **Done** (this
     session): both windows ported to RmlUi wholesale (not just the list), sharing a
     `UI::Quests::RewardModel` reward-row builder with `CMyQuestInfoWindow`'s own port off the same
     class — see `migration-ledger.md`'s own rows for the full story.
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

