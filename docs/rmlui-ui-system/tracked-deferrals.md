# Tracked Deferrals

This file tracks remaining work and accepted constraints with explicit revisit triggers.
Completed migrations belong in [migration-ledger.md](migration-ledger.md).

## Priorities

1. Address remaining native interactions and presentation bindings one screen at a time.

Theme-owned window placement is done ([window-placement.md](window-placement.md)): the theme's
workspace places the docks, the centred panels, the HUD shell, the chat and the event HUDs.
The modern theme exists to prove the architecture, so its unchecked windows are not tracked;
the Cursed Temple result panel needs a finished event to check.

Run targeted scale/theme/interaction validation alongside each change. The CObject
registry and residual Social base remain lower-priority structural work.

## Pilots to revisit when the relevant phase arrives

Every already-shipped window that doesn't fully match the principles doc is **left as-is now,
not rewritten to chase each gap in isolation** (§26 — incremental, don't rewrite wholesale) —
but each specific deviation below is tied to whichever future initiative would naturally fix it,
so it gets folded into that pass instead of being forgotten. Check this list whenever starting
one of the trigger initiatives on the right.

| Window(s) | Known deviation | Revisit when... |
|---|---|---|
| `CMyInventory` (equipment paperdoll — `RenderEquippedItem()`, still fully native) | Background sprite, durability tint, and drag-compatibility highlight all paint *behind* the equipped item's live 3D icon today (native paint order); RmlUi's main context always composites last, so a straight port would paint them *in front of* instead — a real regression, not a straight port (deliberately skipped for this reason). | A background-context consolidation pass makes this mechanism reliable enough to trust with more per-frame-varying, class-conditional content, **or** the equipment grid gets its own future chrome pass anyway and folds this in at the same time — whichever comes first. If pursued alone, the static background sprite (no gameplay-state binding) is the only piece with a reasonable cost/value ratio on its own. |
| HUD circular glass-orb + wrapping arc gauges (reference visual study, not yet built) | The modern theme retinted `main_frame.rcss`'s rectangular HP/MP/AG/SD bars rather than rebuilding them as circular orbs/arcs — that's a structural rebuild (new markup, new `CMainFrameWindow` C++ binding shape, new tooltip anchors), not a retint, and touches live combat UI. Two RmlUi-native techniques were confirmed viable for it (`<progress direction="clockwise">` for the arcs, which needs a `fill-image` — see `engine-findings.md`; layered `radial-gradient` for the orb liquid) but not used yet. | A dedicated pass scoped just to this, once explicitly prioritized. Low priority: the modern theme exists to prove the architecture. |

## Tracked deferral: C++ adapter classes still on the `mu::ui::window::CObject` tier

The CObject/CManager lifecycle and `INTERFACE_*` lookup plus `g_p*` macros still
require shared per-window registration. Revisit when concrete extension or lifecycle
requirements justify changing that shared machinery. Migration coverage is sufficient
to study existing examples; a replacement is not required merely because these
adapters retain their established base.

## Tracked deferral: one obvious component surface

Normalise the canonical UI surface so a developer meets one component family rather than
historical header boundaries — compatibility aliases or forwarding headers over mass renames.
Done when every common UI concern has one canonical implementation or an explicitly documented
presentation-specific split, discoverable without knowing the codebase's history. Retiring the
last native widget consumers (`CInGameShop`) is part of it.

## Tracked deferral: `CMainFrameWindow` file split

`MainFrameWindow.cpp/.h` contain CMainFrameWindow, CSkillList and CItemHotKey.
Consider separating them when working on their responsibilities. Skill icons are RmlUi
decorators; item icons are still drawn in 3D, into render targets the slots show. This is a
file-organization decision with no need to rename the classes or change their behavior.

## Tracked deferral: `CUIBaseWindow`/`CUIPhotoViewer` still derive from `CUIControl`

All that is left of the `CUIControl` family: its widgets are deleted and the windows that kept it
alive are RmlUi documents (`migration-ledger.md`'s "`CUIControl` list family" section).
`UI/Social/SocialWindowCore.h` contains `CUIControl`, its `CUIMessage` queue, the `UISTATES`/`UI_MESSAGE_ENUM` enums,
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

Not a suspicion that something is broken: a port makes dozens of small judgement calls that are
never revisited once it's marked Done, and so far only playing the game and parity reviews have
caught any of them.

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

Everything else in the ownership rollout was verified in game, both themes, including the scale
sweep.

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

- **Display-list documents:** repeated rows with variable content still need
  a per-window review before moving bound `top` values to RCSS. The `:nth-child`
  selector works on `data-for` rows at runtime (`engine-findings.md`), and the
  event-entry, Gold Bowman, Castle, Guard and Catapult documents use it.
  Counter-scaled text cannot always stack through normal flow; use the proven
  selector where the row count and pitch are actually bounded.
- **MessageBoxView:** measured stacking and centring retain geometry bindings;
  review literal button positions with the relevant box classes. Counter-scaled
  text metrics remain a constraint, not automatically removable layout constants.
- **Root-placement names:** `root_x`/`root_y` and `panel_x`/`panel_y` name similar placement
  bridges. Consolidation is low-priority
  naming work, separate from changing who owns dock spacing.

### Deliberately not on this list

`.sharp-text` counter-scaled tops, `MiniMap`/`WorldLabelLayer` marker coordinates, and
`TitleSceneUI`'s loading bar (pushed as real `px` because the scene's background is still native
sprites on an 800x600-reference per-axis scale `dp` cannot reproduce). Each is a justified hybrid
with an explicit, recorded constraint. The last one has an expiry, though, and nothing currently
links the two: it ends when those background sprites port.

### The guard that freezes the population

`tools/check_rml_bound_geometry.py` is wired into the build beside the syntax and
contract-drift checks. It requires a reason for non-exempt geometry bindings in
`tools/rml_bound_geometry_allowlist.txt`. Root-placement/scaling expressions are
exempt under the checker's rules.

Run `python tools/check_rml_bound_geometry.py --review` to compare each reason with
its actual bound fields. An entry can still be required while its description has
become obsolete, so update the reason when a port changes what a document binds.

Allowlist totals are not defect counts. Review dynamic coordinates, native companions,
counter-scale bridges and static presentation bindings separately. Passing the guard
establishes documented exceptions, not runtime correctness or full theme ownership.

A deliberate narrowing: the guard covers the four box offsets and the two sizes only. A bound
`color`, `decorator` or `font-size` has the same override problem, but each of those is a judgement
call per case, whereas a bound static coordinate is nearly always layout that belongs in RCSS. Widen
it when a bound colour actually bites.
