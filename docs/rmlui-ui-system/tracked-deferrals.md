# Tracked Deferrals

This file tracks remaining work and accepted constraints with explicit revisit triggers.
Completed migrations belong in [migration-ledger.md](migration-ledger.md); validation
results belong in [validation-matrix.md](validation-matrix.md).

## Priorities

1. Design theme-driven dock spacing together with first-layout and saved-position rules.
2. Address remaining native interactions and presentation bindings one screen at a time.

Run targeted scale/theme/interaction validation alongside each change. The CObject
registry and residual Social base remain lower-priority structural work.

Source evidence and disposition history are in the
[2026-10-04 reassessment](../../.ai-os/memory/tasks/rmlui-deferral-reassessment.md).

## Pilots to revisit when the relevant phase arrives

Every already-shipped window that doesn't fully match the principles doc is **left as-is now,
not rewritten to chase each gap in isolation** (§26 — incremental, don't rewrite wholesale) —
but each specific deviation below is tied to whichever future initiative would naturally fix it,
so it gets folded into that pass instead of being forgotten. Check this list whenever starting
one of the trigger initiatives on the right.

| Window(s) | Known deviation | Revisit when... |
|---|---|---|
| `CMuHelperDetailWindow`'s three threshold gauges | Their art is RmlUi but their input is still C++: a hit rectangle per gauge plus per-frame mouse polling (`UpdateGauge()`). Converting them to a stock `<input type="range">` was attempted and **stopped deliberately**: `WidgetSlider`'s value mapping is unusable under the docked panel's `transform: scale(root_scale)` (see `engine-findings.md`). Two smaller differences would also have to be settled: the slider has no wheel handling, and native's click mapping (`floor(10x/124) + 1`, so 0 is only reachable left of the bar) is not a proportional one. | Either the docked panels stop scaling via a CSS transform, or the hit area is hoisted out of it as a `<body>`-level element positioned from `root_x`/`root_y`/`root_scale`. `COptionWindow`'s sliders are not a precedent — that panel has no transform. |
| `CBuffStrip` (`themes/modern/buff_strip.rcss`) | The tooltip overrides its shared text colour with `#ffffff`. | Replace with the existing text token when touching the stylesheet. A font-size token category is separate work. |
| Migrated windows | Recorded validation does not cover every resolution, UI scale, theme and drag-state combination. | Extend and execute the existing [validation matrix](validation-matrix.md), including event-only states and actual item/skill activation. |
| All draggable migrated windows | Existing drag system's interaction with theme-default-layout + UI-scale (§10–11) has never been explicitly audited | The drag/preference-integration audit (itself an unstarted gap, above) happens — check these windows specifically, don't just audit the mechanism in the abstract. |
| `CBuffStrip` | Right-click-to-cancel not reproduced; tooltip is plain-text instead of the original's per-line-colored rich tooltip (both already documented as deliberate scope cuts in `newui-tier-adapter.md`, not silent gaps) | Right-click-distinct-from-left-click is proven generally in a `data-event-click` binding, or the three non-unified tooltip mechanisms (§12) get consolidated — whichever comes first. |
| `CMainFrameWindow` (`main_frame.rcss`, both themes — HP/MP/AG/SD/EXP bars + 5 corner buttons) | `UI::Scaling::BottomHudScale()`/`CappedUniformScale()` (`UITransform.cpp`) fold `GameConfig::GetUIScalePercent()` in as a post-clamp multiplier, applied in the shared function itself so every caller codebase-wide (RmlUi bars/buttons/exp and the potion icons' render size via `bars_scale`, the still-legacy chrome render, and potion/skill click hit-testing) moves together automatically. Also folds `UI::Scaling::GetWindowContentScale()` (OS display-scale/pixel-density factor) into RmlUi's own `dp` ratio (`RmlUiRuntime.cpp`'s `ApplyUIScale()`) — **confirmed 2026-09-07 on a 125%-scaled display; see `layout-and-scaling.md`.** `main_frame.rcss` still deliberately uses `px`, not `dp`, throughout, tracking `bars_scale` exactly instead of being scaled a second time. | The `UIScalePercent` half needs verifying by actually using a potion/skill at more than one `UIScalePercent` value *and* resolution, not just a visual check. Phase 3 (item hotkeys → real RmlUi) landing (the skill icon-atlas port landed 2026-09-27) still eventually retires `BottomHudScale` from this window entirely in favor of the branch's normal fixed-`dp`/`UIScalePercent` policy. |
| `CMyInventory` (equipment paperdoll — `RenderEquippedItem()`, still fully native) | Background sprite, durability tint, and drag-compatibility highlight all paint *behind* the equipped item's live 3D icon today (native paint order); RmlUi's main context always composites last, so a straight port would paint them *in front of* instead — a real regression, not a straight port (Stage 2 was scoped, investigated, and deliberately skipped for this reason — see "What's migrated" above). | A background-context consolidation pass makes this mechanism reliable enough to trust with more per-frame-varying, class-conditional content, **or** the equipment grid gets its own future chrome pass anyway and folds this in at the same time — whichever comes first. If pursued alone, the static background sprite (no gameplay-state binding) is the only piece with a reasonable cost/value ratio on its own. |
| `CMyInventory` (`my_inventory.rcss`, legacy theme only) | The 4 corner buttons (RmlUi, always renders last) can end up on top of `CInventoryCtrl`'s native item tooltip when a bottom-row item's tooltip extends into the button strip — before Stage 1 both were native, ordinary same-frame paint order put the tooltip on top. Confirmed cosmetic, not functional; user explicitly deferred it. | `UI::RmlBridge::OverlayRender` wraps `SetPostRmlUiCallback` as a registry any window can join (`component-catalog.md`) and has no other consumer, so rerouting the tooltip through it is a caller change. Or a future grid-chrome pass makes the tooltip an RmlUi element, resolving it for free via DOM order. |
| HUD circular glass-orb + wrapping arc gauges (reference visual study, not yet built) | The 2026-09-10 iron-palette migration deliberately retinted `main_frame.rcss`'s existing rectangular HP/MP/AG/SD bars rather than rebuilding them as circular orbs/arcs — that's a structural rebuild (new markup, new `CMainFrameWindow` C++ binding shape, new tooltip anchors, interacts with `BottomHudScale()`), not a retint, and touches live combat UI. Two RmlUi-native techniques were confirmed viable for it (`<progress direction="clockwise">` for the arcs via real octant geometry, layered `radial-gradient` for the orb liquid) but not used yet. | A dedicated, focused pass scoped just to this, once explicitly prioritized — see `modern-theme-visual-direction.md`'s "Known follow-up" section. |

## Tracked deferral: C++ adapter classes still on the `mu::ui::window::CObject` tier

The CObject/CManager lifecycle and `INTERFACE_*` lookup plus `g_p*` macros still
require shared per-window registration. Revisit when concrete extension or lifecycle
requirements justify changing that shared machinery. Migration coverage is sufficient
to study existing examples; a replacement is not required merely because these
adapters retain their established base.

## Tracked deferral: `CMainFrameWindow` file split

`MainFrameWindow.cpp/.h` contain CMainFrameWindow, CSkillList and CItemHotKey.
Consider separating them when working on their responsibilities. Skill icons already
use RmlUi; item icons remain native 3D. This is a file-organization decision with no
need to rename the classes or change their behavior.

## Tracked deferral: `CUIBaseWindow`/`CUIPhotoViewer` still derive from `CUIControl`

All that is left of the `CUIControl` family retirement, which closed 2026-10-04. The widgets that
made that file a toolkit are deleted and the windows that kept it alive are RmlUi documents --
`migration-ledger.md`'s "`CUIControl` list family" section has the outcome, and
`ui-target-architecture.md` item 17 records the checklist closing.

What remains is one step that was never on that checklist. `UI/Social/SocialWindowCore.h` contains `CUIControl`, its `CUIMessage` queue, the `UISTATES`/`UI_MESSAGE_ENUM` enums,
`g_dwActiveUIID`/`g_dwMouseUseUIID`, and the `GUILDLIST_TEXT`/`LETTERLIST_TEXT` records. Two classes
derive from it, both in `UI/Social/`: `CUIBaseWindow` and `CUIPhotoViewer`. What they take from it
is real, not vestigial -- identity, parent id, state, geometry, options, and the message queue
`CUIWindowMgr` runs the family through.

Removing the base would require preserving its identity/state/message contracts in
the social manager and portrait implementation. The base already lives beside its
users in UI/Social. Revisit dissolution only if work on those contracts justifies it;
file relocation is not an outstanding retirement step.

A few dead enumerators sit inside otherwise-live enums (`UISTATE_SCROLL`, `UISTATE_DISABLE`,
`UI_MESSAGE_NULL`, `UI_MESSAGE_TEXTINPUT`, the four list-message values). Cosmetic, deliberately
left.

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


- **A port can mirror a native scrollbar's geometry into its model** rather than letting RCSS own
  it. The native list this was first written about (`CUITextListBox::GetScrollBarGeometry()`) is
  deleted, and with it most instances; `CGensRanking` still does it, reading `thumb_top` off its
  own `CScrollBar`. Not hand-rolled scroll maths, but it is the "RmlUi draws, native decides" split
  that `CMoveCommandWindow` retired by adopting `.scroll-pane` -- the remaining case should follow
  it when that window is next touched.
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

## Tracked deferral: the C++ ↔ RML/RCSS ownership boundary

The remaining ownership work is listed below. For new UI, follow
[building-new-ui.md](building-new-ui.md)'s Ownership section.

### What is implemented but not seen

Converted, built and linked, with no client-reachable state to validate against. Each is a real
risk, not a formality: a wrong position here surfaces only during the event.

- **`CryWolf`** — its render gates on `M34CryWolf1st::IsCyrWolf1st()`, so `$win crywolf` opens the
  window and nothing draws. The exp digits, the five altars, the notice's four lines and the
  clock's state all moved unseen. It also carried a real bug: altars were pushed only when they
  had something to show, so a contracted altar shifted every altar after it along the hill.
- **`SiegeWarfare`** — renders only inside Battle Castle during a siege.
- **The Illusion Temple result and HUD**, the three event timers, the Doppelganger frame, the duel
  spectator frame and Battle Soccer's score rows. Of these the Temple HUD is the weakest: its
  chrome left the sprite list, and the draw order rests on the argument that the regrouped pieces
  do not overlap on screen rather than on having been looked at.

Everything else in the rollout was verified in game by the user, both themes, including the scale
sweep: see the rollout task's own verification section for the per-batch record.

### Accepted as it stands, with its trigger

- **`SiegeWarfare`'s team/command buttons** — geometry read off live
  native controls that also hit-test it. Trigger: retiring the native `CButton`s.
- **`MessageBoxView`** — measured centring and stacking; residue is one box class's four literal
  button positions. Trigger: splitting the shared document per box class.
- **`Notices`** — physical px with no root transform, its transform taken ambiently. Trigger: that
  HUD gaining a reference-px space.
- **`MiniMap`** — no reference-px space exists: the art is turned 45° in physical px.
- **Unbounded server-driven counts** — the duel spectator list, siege score marks. No `:nth-child`
  or `:nth-last-child` table can cover a count the client does not bound.
- **Computed fan-outs** — `MainFrameWindow`'s zig-zag skill grid and `MuHelperSkillPicker`'s, both
  positioned from an ordinal among what the player actually has.
- **A new siege command's pulse** — `rgb(255, pulse, pulse)` welds the theme's red to a per-frame
  sine. RCSS cannot mix a bound fraction into a colour, so this needs a mechanism that does not
  exist rather than a tidier binding. The guard deliberately does not cover `color`.

### The counter-scale block, and a way out that was prototyped and not taken

The prototype evidence below is dated 2026-10-02; recheck upstream status and runtime
invalidation before making an adoption decision.

Several allowlist entries share one counter-scale constraint. A `.sharp-text` layer cancels `#panel`'s
scale so glyphs rasterise sharp, which means its layout width must arrive *pre-multiplied* —
`data-style-width="(220 * root_scale) + 'px'"`. No theme can express that, because **RmlUi can do
arithmetic only in the data-binding evaluator** (`Source/Core/DataExpression.cpp`, where `'*'` is a
real operator) and never in a stylesheet. The data model is the only thing in the engine that can
multiply, so the width has to travel through it — and that is precisely the inline-property leak
the guard flags. These entries require an engine or text-layout solution.

**The way out, measured rather than guessed (2026-10-02).** Upstream
[PR #983](https://github.com/mikke89/RmlUi/pull/983) adds CSS math expressions: `calc()`, `min()`,
`max()`, and `var()` *inside* them — `calc(var(--w) / 2)` and multiplication by a unitless value are
both in its 3,173 lines of unit tests, which is exactly the shape needed. So
`width: calc(220px * var(--root-scale))` would move the whole class into RCSS, with C++ setting
`--root-scale` as a custom property instead of a model field.

What was actually verified:

- It **merges into the fork's `integration/sdl-gpu-parity` with zero conflicts** (`git merge-tree`),
  sitting 3 commits ahead of the pin. It touches property parsers and the stylesheet; the fork's
  own work is renderer backends, so they do not meet.
- A **full rebuild succeeded** — 408 targets including `rmlui.lib` from scratch — and MuClient
  linked against it at exit 0 with all three asset checkers green.
- The merge is on the fork as `nitoygo/RmlUi` `integration/sdl-gpu-parity` (`0f8b5dcb`).

**Not adopted, deliberately.** The submodule pin stays at `22282190` and `.gitmodules` keeps
pointing at `mikke89/RmlUi`. Adopting it would mean the project builds against an unmerged upstream
PR carried on a personal fork, which is a bigger commitment than the allowlist saving justifies on
its own. The MuMain-side change that would switch it on — the `.gitmodules` repoint plus its README
and sync-log notes — is preserved unmerged on the local branch **`spike/rmlui-calc-via-fork`**.

**The open question nobody has answered**, and the thing to settle before revisiting: whether C++
can set a custom property on `#panel` at runtime and have every `calc()` depending on it recompute.
The PR's tests cover invalidation for font-size changes behind `var()`, so the machinery exists, but
an arbitrary custom property set from code is the actual integration point and was never tried. The
per-frame cost is the second unknown — `--root-scale` changes on a UI-scale change, not every
frame, but if setting it dirties every dependent property that wants measuring (RelWithDebInfo
only; the PR ships `Tests/Source/Benchmarks/Calculation.cpp` to borrow from).

**Revisit when** upstream merges #983, which removes the fork objection entirely — or when the
counter-scale bindings start costing something concrete, rather than being an inventory number.

### Remaining ownership work

- **Display-list documents:** the conversion that lets a repeated row of variable
  length be positioned by RCSS rather than by a bound `top` per entry. This is a
  method, not one window: applied once it produces most of the findings below at
  once. Its enabling selector is proven -- `:nth-child` on `data-for` rows works at
  runtime and the event-entry and Gold Bowman documents already use it
  (`engine-findings.md`) -- so what remains is applying it to the documents that
  still bind a `top` per row, keeping in mind that counter-scaled layers cannot be
  stacked by flow. Rule on it before the next port rather than retrofitting later.
- **Dock spacing:** `WindowSystem.cpp`'s `PanelColumnX()` fixes columns at 190 while
  RCSS owns drawn panel width. Placement is seeded during creation, before document
  layout. Design first-layout placement, theme resize/reflow and saved-position
  precedence together; reading a panel width alone does not solve this.
- **Static index arithmetic:** Castle and Guard tab positions, Catapult lines and
  GensRanking description rows still bind positions derived from row indices.
  Move bounded layout into RCSS while preserving required counter-scale metrics.
- **GensRanking scrollbar:** the model still mirrors native CScrollBar thumb geometry.
  Consider the shared RmlUi scroll pane when this window is next changed.
- **Quest row weight:** QuestRewardModel::Entry carries `style` and a `bold` flag
  derived from Heading. Let RCSS derive weight from the semantic style in
  MyQuestInfoWindow, QuestProgress and QuestProgressByEtc. Validate all row kinds.
  Generic dialogs have caller-supplied bold/color without that semantic style;
  changing their contract requires a separate caller review.
- **MessageBoxView:** measured stacking and centring retain geometry bindings;
  review literal button positions with the relevant box classes. Counter-scaled
  text metrics remain a constraint, not automatically removable layout constants.
- **Event button style:** review the presentation-sized `wide` value alongside the
  semantic `exit` value when next touching those button structs.
- **Root-placement names:** `root_x`/`root_y`, `panel_x`/`panel_y`, and
  `bars_left`/`bars_top` name similar placement bridges. Consolidation is low-priority
  naming work, separate from changing who owns dock spacing.

### Deliberately not on this list

`.sharp-text` counter-scaled tops, `MiniMap`/`WorldLabelLayer` marker coordinates, and
`TitleSceneUI`'s loading bar (pushed as real `px` because the scene's background is still native
sprites on an 800x600-reference per-axis scale `dp` cannot reproduce). Each is a justified hybrid
with an explicit, recorded constraint. The last one has an expiry, though, and nothing currently
links the two: it ends when those background sprites port.

`CSystemLogWindow`'s `back_color` also stays fused, the last case of the three-layers-in-one-string
shape `CChatLogWindow` was split out of: its transparency is a *per-line* background, so separating
the user preference from the theme decision needs a backdrop element behind every row of a
per-frame `data-for` list. That makes it part of the display-list conversion above, not of this
boundary.

### The guard that freezes the population

`Tools/check_rml_bound_geometry.py` is wired into the build beside the syntax and
contract-drift checks. It requires a reason for non-exempt geometry bindings in
`Tools/rml_bound_geometry_allowlist.txt`. Root-placement/scaling expressions are
exempt under the checker's rules.

Run `python Tools/check_rml_bound_geometry.py --review` to compare each reason with
its actual bound fields. An entry can still be required while its description has
become obsolete. In particular, UnitedMarketPlace's description still mentions a
native scrollbar absent from its current document; correct that reason without
removing the entry's genuine counter-scale constraint.

Allowlist totals are not defect counts. Review dynamic coordinates, native companions,
counter-scale bridges and static presentation bindings separately. Passing the guard
establishes documented exceptions, not runtime correctness or full theme ownership.

A deliberate narrowing: the guard covers the four box offsets and the two sizes only. A bound
`color`, `decorator` or `font-size` has the same override problem, but each of those is a judgement
call per case, whereas a bound static coordinate is nearly always layout that belongs in RCSS. Widen
it when a bound colour actually bites.
