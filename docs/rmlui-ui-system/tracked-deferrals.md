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
| `CMuHelperDetailWindow`'s three threshold gauges | Their art is RmlUi but their input is still C++: a hit rectangle per gauge plus per-frame mouse polling (`UpdateGauge()`). Converting them to a stock `<input type="range">` was attempted and **stopped deliberately**: `WidgetSlider`'s value mapping is unusable under the docked panel's `transform: scale(root_scale)` (see `engine-findings.md`). Two smaller differences would also have to be settled: the slider has no wheel handling, and native's click mapping (`floor(10x/124) + 1`, so 0 is only reachable left of the bar) is not a proportional one. | Either the docked panels stop scaling via a CSS transform, or the hit area is hoisted out of it as a `<body>`-level element positioned from `root_x`/`root_y`/`root_scale`. `COptionWindow`'s sliders are not a precedent — that panel has no transform. |
| `CBuffStrip` (`buff_strip.rcss`, both themes) | Both themes now have a token layer (`modern` since 2026-09-04, `legacy` since 2026-09-23), and `mu_helper_bar.rcss` was retrofitted in both (`font-family`/tooltip `background-color`/`border`/`color`/`border-radius` all now `token(...)`) -- `buff_strip.rcss` still repeats its own `.tooltip`'s `color: #ffffff` (modern) / now-tokenized `color` (legacy, done alongside `mu_helper_bar.rcss` in the same pass) inline instead of referencing `token(text-primary)`, the same stray-override pattern `mu_helper_bar.rcss` had (base.rcss's own shared `.tooltip` rule already sets this token as the default). `font-size` stays literal everywhere in both files -- no `font-size` token category exists system-wide yet, not unique to these two. | Retrofit `buff_strip.rcss`'s modern `.tooltip` `color` the same one-line way `mu_helper_bar.rcss` was fixed, next time this file is touched -- trivial once picked up, just not bundled into the CMuHelperBar-scoped pass that fixed its sibling. A `font-size` token category is a separate, larger, not-yet-prioritized initiative (would touch far more than these two files). |
| `CMuHelperBar`, `CBuffStrip` and every `CWin`-tier window | Base class/tier boundary (`mu::ui::window::CObject`/`CWin`) unchanged (§12, "Tracked deferral" below) — file location itself was resolved by the `UI/` directory restructure: `UI/NewUI/HUD/` is now `UI/HUD/` | A base-class/tier restructuring pass is undertaken — not before enough windows exist to know the real target shape (this is the existing tracked deferral, not new). |
| All `CWin`-tier windows, `CMuHelperBar`, `CBuffStrip` | No resolution × UI-scale × theme × drag-state validation matrix has been run against any of them (§25) — verification so far has been ad hoc per window | A validation-matrix/test-plan artifact is built — run it retroactively against every already-migrated window, not just new ones going forward. |
| All draggable migrated windows | Existing drag system's interaction with theme-default-layout + UI-scale (§10–11) has never been explicitly audited | The drag/preference-integration audit (itself an unstarted gap, above) happens — check these windows specifically, don't just audit the mechanism in the abstract. |
| `CBuffStrip` | Right-click-to-cancel not reproduced; tooltip is plain-text instead of the original's per-line-colored rich tooltip (both already documented as deliberate scope cuts in `newui-tier-adapter.md`, not silent gaps) | Right-click-distinct-from-left-click is proven generally in a `data-event-click` binding, or the three non-unified tooltip mechanisms (§12) get consolidated — whichever comes first. |
| `CMainFrameWindow` (`RenderLeftFrame()`/`RenderCenterFrame()`, `MainFrameWindow.cpp`) | Modern theme's flat background fill behind the still-legacy 3D-rendered potion/skill icons paints through the background context (`#bg_left`/`#bg_center`, `main_frame_bg.rml`), not C++, since a paint-order-legitimate reason blocks the ordinary main context — RmlUi always composites its whole document as the frame's last pass, after those icons already rendered, so an RmlUi-drawn fill in that screen region through the main context would always paint *over* them, not behind — the same reason `#item_slots`/`#skill_slots` are border-only in RmlUi, never filled. ~~Gated on the literal string `GetActiveThemeName() == "modern"` (§30 violation — a third theme wanting the same treatment silently wouldn't get it)~~ — **fixed 2026-09-04**: now gated on `UI::RmlBridge::ThemeProvidesOwnIconChrome()`, a declared theme capability (`themes/modern/theme.ini`). The border lines that used to live alongside this same fill were **not** similarly exempt — moved to RmlUi (`#gauge_frame`), since a thin outline has no such paint-order constraint. The skill-hotkey-number subscript and the gauge current/max text are both fully retired from C++ (`GetHotKeySlotNumber()`/`hp_current_text` etc. — pure theme-agnostic data, each theme's own markup decides what to show). **Update, 2026-09-23**: `RenderCenterFrame()`'s last remaining native quad — the modern-only translucent skill-list-open highlight (`RenderColorQuadARGB(222, kHudTop, 160, 40, 0x40FFFFFFu)`) — also moved into the same background document (`#skill_list_highlight`, `MainFrameBgRmlModel::skillListOpen`); it had no icon-atlas dependency at all, just the same paint-order constraint as the fill above, so it was a plain Group-A fix, not part of the atlas port below. | **Update, 2026-09-27**: the skill icons and legacy box art are RmlUi now (icon-atlas port: `UI::Skills::Icon::ResolveSkillIcon()`, `skill_icons.rcss`), so the fill only still matters for the native 3D potion icons. Retires with those (item hotkeys → RmlUi) or a render-ordering fix. |
| `CItemHotKey::RenderItems()` (`MainFrameWindow.cpp`) | Hardcodes the item-hotkey slot origin+pitch as literals (`x = 10 + i*38, y = 443, 20x20`) that both themes' `main_frame.rcss` duplicate for `#item_slot_0..3` (`PINNED ("hotkey slot pitch/size")` comments on both sides). **Skill half retired 2026-09-27**: `CSkillList::RenderCurrentSkillAndHotSkillList()` is gone — the hotkey row and current-skill icons are RmlUi children of `#skill_slot_0..4`/`#current_skill_slot`, and the hint anchors read the hovered slot's box back from RmlUi (`SlotBoxInBars()`), so only the theme RCSS places the skill slots. | The item icons are live 3D renders (`RenderItem3D()`); retires when they read their slot from RmlUi or move to RmlUi. |
| `main_frame.rml` (both themes) | Two independently-maintained RML files, not the one-shared-RML-per-window pattern every other migrated window uses — `theming-and-modding.md`'s "Forking a theme's RML" section documents why and the criteria for when this is legitimate. The two files' shared ids/classes/bindings require hand-sync, called out in each file's own header comment — see "Known gaps" for the drift-check tooling this still doesn't have. | Either a cleaner RCSS-only structural fix is found and one file retires, or this is accepted long-term and the same criteria get applied consistently if another window ever needs it — not before a second real case shows up. |
| `CMainFrameWindow` (`main_frame.rcss`, both themes — HP/MP/AG/SD/EXP bars + 5 corner buttons) | `UI::Scaling::BottomHudScale()`/`CappedUniformScale()` (`UITransform.cpp`) fold `GameConfig::GetUIScalePercent()` in as a post-clamp multiplier, applied in the shared function itself so every caller codebase-wide (RmlUi bars/buttons/exp via `bars_scale`, the still-legacy chrome render, 3D potion-icon placement, and potion/skill click hit-testing) moves together automatically. Also folds `UI::Scaling::GetWindowContentScale()` (OS display-scale/pixel-density factor) into RmlUi's own `dp` ratio (`RmlUiRuntime.cpp`'s `ApplyUIScale()`) — **confirmed 2026-09-07 on a 125%-scaled display; see `layout-and-scaling.md`.** `main_frame.rcss` still deliberately uses `px`, not `dp`, throughout, tracking `bars_scale` exactly instead of being scaled a second time. | The `UIScalePercent` half needs verifying by actually using a potion/skill at more than one `UIScalePercent` value *and* resolution, not just a visual check. Phase 3 (item hotkeys → real RmlUi) landing (the skill icon-atlas port landed 2026-09-27) still eventually retires `BottomHudScale` from this window entirely in favor of the branch's normal fixed-`dp`/`UIScalePercent` policy. |
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
   - ~~`CMsgWin`~~ — **migrated.** Its resident-password prompt (`MESSAGE_DELETE_CHARACTER_RESIDENT`)
     is `#msgwin_input`, a stock `<input type="password">` inside the existing `#input_frame`, so the
     layout is unchanged. Deliberately *not* folded into `CGenericConfirmDialog` even though that
     dialog can express the same content (two lines + masked field + OK/Cancel): `CMsgWin`'s panel is
     352x113dp with its own art against the dialog's 230x160dp, so consolidating would visibly
     restyle this one prompt and leave it inconsistent with every other message box. Worth revisiting
     as a deliberate UX decision, not as a port.
   - ~~`WindowMuHelper`~~ — **migrated** with its host windows: every field is a stock `<input>`, the numeric
     ones filtered by `UI::RmlBridge::AttachNumericInputFilter`.
   - Chat (`CUIChatInputBox`), `CGuildMakeWindow`, `CGoldBowmanWindow`,
     `MsgBoxIGSSendGift`, `UIWindows`' friend/mail, `UIGuildMaster` — **migrate each when its own
     host screen moves to RmlUi, not before.** These are all still-native screens; porting just
     their text field would mean positioning an RmlUi `<input>` against native sprite coordinates,
     which is the coupling this whole effort removes. `CUITextInputBox` retires when the last one
     is gone, and not by a dedicated retirement pass.

     `LoginScene.cpp`'s free `DeleteCharacter()` reads the value through
     `CMsgWin::GetResidentPasswordInput()` now, but appears to have **no callers** —
     `CharSelMainWin.cpp:324`'s unqualified call resolves to the member
     `CCharSelMainWin::DeleteCharacter()`, and `CMsgWin::RequestDeleteCharacter()` is the live path.
     Suspected dead, not verified to the standard `CWin`/`::CButton`/`CSlider` got before deletion.
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
     `CUIGuildNoticeListBox`, `CUIUnionGuildListBox`,
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


## Tracked deferral: audit where ports steered away from the original UI

Requested 2026-09-27, after `origin/dev/rmlui-ui-system`'s parity pass (PR #644) merged in. Not a
suspicion that something is broken — it's the recognition that a port makes dozens of small
judgement calls that never get revisited once it's marked Done, and the only two mechanisms that
have caught any of them so far are somebody playing the game and somebody else's parity review.

**What to look for.** Not bugs — decisions. A port diverges from the original in four recognisable
ways, and only the first is self-announcing:

1. **A deliberate, recorded simplification.** These are already written down at the site that made
   them; the audit's job is to ask whether the reason still holds, not to rediscover them.
2. **A primitive that generalized past its first consumer.** Extracting a shared class changes
   every window that adopts it, and the change is invisible in the consumer's own file.
3. **An RmlUi behaviour standing in for a native one because it was free.** `:hover` for a
   C++-computed selection flag, `line-height` for a measured row pitch, DOM scrolling for a
   line-window model. Each is right in isolation and each shifts the rendering slightly.
4. **A judgement call made with no reference to hand**, i.e. most modern-theme treatments.

**Known instances to seed it with**, so the audit doesn't start from zero:

- **`.scroll-pane` reaching `CGenericConfirmDialog`.** `7dabcf54` moved `.gcd-text-col` off the
  dialog's own flat 6dp rail onto the shared primitive's 15dp native sprite art, and `2e619ea6`
  added the 3dp end caps. Both were the right call for the primitive; both changed a legacy dialog
  that PR #644 was independently tuning for parity, without that branch's knowledge. This one
  prompted the request.
- **`.scroll-pane`'s own two legacy simplifications** — the middle slice stretched as one ninepatch
  rather than repeat-tiled, and native's 7-vs-15 thumb overhang not reproduced (`component-catalog.md`
  records both and why).
- **Hover highlights now paint behind their text**, in `CChatLogWindow` and `CMoveCommandWindow`.
  Native drew the tint quad *after* the row text, so the glyphs sat under it; a `background-color`
  sits behind them. Reads cleaner, is not what shipped.
- **`CMoveCommandWindow`'s scrollbar is `dp`-sized** and so doesn't grow with its panel, unlike
  native's reference-scaled one. Deliberate — its pane's net transform is identity — but it makes
  this window's scrollbar the one element that tracks the user's scale dial instead of the dock's.
- **Reward-item preview moved from hover to click** in `CMyQuestInfoWindow`/`CQuestProgress`.
- **The MU Helper windows' behaviour changes**, each deliberate and confirmed in play:
  - Legacy tabs draw `newui_guild_tab04`. Native pointed them at a texture slot nothing loads, so
    they shipped with labels only.
  - Pick-all and pick-selected clear each other's flag. Native only unticked the other box.
  - Ticking a skill's Condition fills an empty radio group with a default. Native left the skill
    unable to fire.
  - Esc closes the window from inside a focused field. Native's field swallowed it.
  - The open skill picker passes clicks through to the world everywhere except its icons.
  - The extra-item list is always shown reverse-alphabetically. Native showed that only after a
    reload, and insertion order before.
- **`CNPCQuest`'s message/answer tops stay in its model.** The other three windows in that pass
  (party colours, the buff strip's slots, the job buttons) moved to RCSS; this one cannot follow
  without changing what ships. `message_top` is a genuine per-instance value -- native centres the
  message-plus-answer block by its line count -- and legacy's separate `answers_top` exists because
  flow does not stack counter-scaled text layers (see `engine-findings.md`). The non-in-progress
  anchor at 250 is absolute while its block's top is data-driven, so expressing it declaratively
  needs either another bound number or the answers markup duplicated per quest state. Revisit if
  legacy's text ever stops being counter-scaled, or alongside a `.sharp-text` flow container that
  reconciles layout height with the counter-scale.
- **The presentation-in-models pattern is wider than the four windows named for it** (found
  2026-09-30, scanning the ports merged from `origin`). Same shape, not yet addressed:
  `ChatCommandWindow`'s `edit_color` pushes a compile-time constant colour through the model;
  the event and siege windows bind `banner_color`/`logo_color`/`kills_color`/`time_color`/
  `timer_color`/`skill_color`, and CryWolf expresses a banner fade as `RGBA(255,255,255,alpha)`
  where a bound opacity or a class would do; and roughly twenty static chrome coordinates
  (`label_top`, `divider_top`, `tab_label_top`, `box_left`, `rank_label_left`, `logo_left`,
  `arrows_left`, `statue_bar_left`, `progress_top`, `hint_left`, `tooltip_top`, ...) are bound from
  C++ across those same windows. Minimap and world markers (`hero_left`, `target_left`,
  `ice_walker_left`, `cursor_left`) are **not** in this set -- they are genuinely per-frame data.
  Not scanned yet: presentation classes standing in for form-control semantics, and any gating on a
  theme name rather than a declared capability.
- **New ports mirror the native list's scrollbar geometry into their models**
  (`GuardWindow`, `MixInventory`, `MessageBoxView`: `thumb_top`, `scroll_top`, `thumb_dragged`, read
  from `CUITextListBox::GetScrollBarGeometry()`). Not hand-rolled scroll maths -- the native list is
  still the one tracked below -- but it recreates the "RmlUi draws, native decides" split that
  `CMoveCommandWindow` retired by adopting `.scroll-pane`, so it widens that deferral instead of
  narrowing it.
- **`COptionWindow`'s volume slider is a gold-thumb slider**; native drew the same fill gauge
  (`newui_option_volume01/02`) the MU Helper's detail window now draws in both themes. The gauge
  pieces are in `mu_helper_common.rcss` if the options window is revisited.
- **Modern-theme treatments picked without checking dock neighbours** — already its own gap note in
  `STATUS.md`, which has recurred twice and whose *process* half is still unfixed.

**The precedent worth knowing before starting.** `CCharacterInfoWindow`'s summary box shipped as
corner brackets plus a flat fill, a recorded and reasonable simplification of `RenderFrame()`'s
8-piece frame — and #623 later restored the real thing. So at least one entry of exactly this kind
has already been found worth reverting by someone looking specifically for it. That is the argument
for the audit, and also the reason to treat "recorded simplification" as a finding rather than a
resolution.
