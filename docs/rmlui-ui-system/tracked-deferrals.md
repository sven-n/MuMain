# Tracked Deferrals

This file tracks remaining work and accepted constraints with explicit revisit triggers.
Completed migrations belong in [migration-ledger.md](migration-ledger.md).

## Priorities

### Before merging to main

What would otherwise grow with every new window, or break a principle in a way new code copies.
Most are items of the integration seams below.

1. **A display-scale change applies the `dp` ratio** (item 2). A bug against §9.

Also before merging, though not code health: the event windows nobody has seen in game (the
ownership boundary's first list) are either looked at on a server that can run the events, or
named as unseen in the pull request.

### After the merge

The divergence audit; retiring the background contexts, with the root transform and counter-scaled
text they keep alive; and one obvious component surface, whose native
widgets `CInGameShop` keeps alive and new code must not use. The equipment paperdoll and the modern
HUD orbs wait on their triggers, and the counter-scale block on upstream RmlUi, with the
bound-geometry guard keeping its allowlist from growing meanwhile. Each accepted constraint in the
ownership boundary carries its own trigger.

### Throughout

Theme-owned window placement is done ([window-placement.md](window-placement.md)). The modern
theme exists to prove the architecture, so its unchecked windows are not tracked. Run targeted
scale, theme and interaction checks alongside each change. The `CObject` tier is accepted as the
base ([building-new-ui.md](building-new-ui.md)'s "Accepted as the base").

## Pilots to revisit when the relevant phase arrives

Every already-shipped window that doesn't fully match the principles doc is **left as-is now,
not rewritten to chase each gap in isolation** (§26 — incremental, don't rewrite wholesale) —
but each specific deviation below is tied to whichever future initiative would naturally fix it,
so it gets folded into that pass instead of being forgotten. Check this list whenever starting
one of the trigger initiatives on the right.

| Window(s) | Known deviation | Revisit when... |
|---|---|---|
| `CMyInventory` (equipment paperdoll — `RenderEquippedItem()`, still fully native) | Background sprite, durability tint, and drag-compatibility highlight all paint *behind* the equipped item's live 3D icon today (native paint order); RmlUi's main context always composites last, so a straight port would paint them *in front of* instead — a real regression, not a straight port (deliberately skipped for this reason). | A background-context consolidation pass (the integration seams' item 1, below) makes this mechanism reliable enough to trust with more per-frame-varying, class-conditional content, **or** the equipment grid gets its own future chrome pass anyway and folds this in at the same time — whichever comes first. If pursued alone, the static background sprite (no gameplay-state binding) is the only piece with a reasonable cost/value ratio on its own. |
| HUD circular glass-orb + wrapping arc gauges (reference visual study, not yet built) | The modern theme retinted `main_frame.rcss`'s rectangular HP/MP/AG/SD bars rather than rebuilding them as circular orbs/arcs — that's a structural rebuild (new markup, new `CMainFrameWindow` C++ binding shape, new tooltip anchors), not a retint, and touches live combat UI. Two RmlUi-native techniques were confirmed viable for it (`<progress direction="clockwise">` for the arcs, which needs a `fill-image` — see `engine-findings.md`; layered `radial-gradient` for the orb liquid) but not used yet. | A dedicated pass scoped just to this, once explicitly prioritized. Low priority: the modern theme exists to prove the architecture. |

## Tracked deferral: one obvious component surface

Normalise the canonical UI surface so a developer meets one component family rather than
historical header boundaries — compatibility aliases or forwarding headers over mass renames.
Done when every common UI concern has one canonical implementation or an explicitly documented
presentation-specific split, discoverable without knowing the codebase's history. Retiring the
last native widget consumers (`CInGameShop`) is part of it.

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

## Tracked deferral: the RmlUi integration seams

A review of `Render/RmlUi` and `UI/RmlBridge` against the vendored RmlUi (2026-10-06) found the core idiomatic: the renderer overrides only texture loading, shutdown runs in
the right order, windows bind through data models and `data-event-*`, and custom features use
RmlUi's own extension points (decorator instancers, `LoadTexture` sources, drag events). What works
against the library is where RmlUi meets the legacy UI. In order of value:

1. **Three ways to order native 3D against RmlUi.** The `background` and `dialog_background`
   contexts (rendered mid-frame from `CManager::Render()`) and `RenderTarget`. The background
   context splits 15 windows into a foreground and a `*_bg.rml` document, each with its own model
   and root-transform sync, and holds the HUD boards that draw under them. `RenderTarget` is the
   idiomatic one: native drawing becomes an image at its element's depth. The stacking table
   (`RmlStackingOrder.cpp`) says which context each document loads into, and
   `test_rml_stacking_order.cpp` holds both background lists closed, so new native 3D can only go
   into a render target. Direction: move the confirm dialog's item preview into a render target and
   retire `dialog_background`; move the inventory family's live items into render targets and
   retire `background` with its `*_bg.rml` documents, taking each out of the test's list. Trigger:
   the paperdoll row above, which waits on the same pass.
2. **A display-scale change leaves RmlUi's `dp` ratio stale.**
   `SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED` updates the content scale `ViewportFitScale()` folds
   in, but nothing calls `RmlUiRuntime::OnResize()` until the next resize. A bug; fix it with the
   next change to `Winmain.cpp`'s event pump.

The root transform and its counter-scaled text (`SyncRootTransform`, `.sharp-text`, the panel
readback) belong to 1: they are needed while a window shares reference coordinates with native
grids and hit tests. A window whose native content has moved into a render target should move to
`dp` and RmlUi's own hit testing. The counter-scale block below is the other way out.

## Tracked deferral: the C++ ↔ RML/RCSS ownership boundary

The ownership rollout moved layout out of C++ into the themes. What is left of it: windows not yet
seen in game, constraints accepted with a trigger, and the counter-scale block. For new UI, follow
[building-new-ui.md](building-new-ui.md)'s Ownership section.

### What is implemented but not seen

Converted, built and linked, with no client-reachable state to validate against. Each is a real
risk, not a formality: a wrong position here surfaces only during the event.

- **`CryWolf`** — its render gates on `M34CryWolf1st::IsCyrWolf1st()`, so `$win crywolf` opens the
  window and nothing draws. The exp digits, the five altars, the notice's four lines and the
  clock's state all moved unseen. It also carried a real bug: altars were pushed only when they
  had something to show, so a contracted altar shifted every altar after it along the hill.
- **`SiegeWarfare`** — renders only inside Battle Castle during a siege.
- **The Illusion Temple result and HUD**, the Blood Castle and Chaos Castle timers and the duel
  spectator list (empty without spectators). Of these the Temple HUD is the weakest: its chrome
  left the sprite list, and the draw order rests on the argument that the regrouped pieces do not
  overlap on screen rather than on having been looked at.

The Battle Soccer score, the duel frame, the Empire Guardian timer and the Doppelganger frame draw
outside their event (`$win soccer full`, `duel`, `empiretimer`, `doppelframe`), and were looked at
in both themes at 90 % UI scale: right, and standing on the HUD.

Everything else in the ownership rollout was verified in game, both themes, including the scale
sweep.

### Accepted as it stands, with its trigger

- **`SiegeWarfare`'s team/command buttons** — geometry read off live
  native controls that also hit-test it. Trigger: retiring the native `CButton`s.
- **`MessageBoxView`** — every text line is centred by measurement and stacked by measured
  height, and a box's buttons come from its own `CMessageBoxButton`s. A box whose buttons were
  literals names its kind and the theme places them (`CGuild_ToPerson_Position`). Trigger: the
  counter-scale block below.
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
the guard flags. These entries require an engine or text-layout solution. Centred text is the
exception: `.sharp-middle` and `.sharp-centre` (`engine-findings.md`) centre a layer without a width
multiplied in, and the button labels and entry-window lines have moved to them.

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

**Now in the pin, not yet used.** `.gitmodules` points at the fork, and the fork, rebased on
upstream master, carries #983, so the stylesheets can use `calc()` today. The local branch
`spike/rmlui-calc-via-fork` held the same repoint and is superseded.

**The open question nobody has answered**, and the thing to settle before revisiting: whether C++
can set a custom property on `#panel` at runtime and have every `calc()` depending on it recompute.
The PR's tests cover invalidation for font-size changes behind `var()`, so the machinery exists, but
an arbitrary custom property set from code is the actual integration point and was never tried. The
per-frame cost is the second unknown — `--root-scale` changes on a UI-scale change, not every
frame, but if setting it dirties every dependent property that wants measuring (RelWithDebInfo
only; the PR ships `Tests/Source/Benchmarks/Calculation.cpp` to borrow from).

**Revisit when** a counter-scaled window is next touched: settle the open question on it, then
move the class. Upstream merging #983 only removes the dependency on the fork.

### Deliberately not on this list

The rest of the allowlist, reviewed entry by entry: per-frame data (gauges, things that follow the
pointer or scroll, windows that grow with their content, projected markers), counts the server does
not bound, text measured the way the native renderer measured it, and the counter-scale block above.
`root_x`/`root_y` and `panel_x`/`panel_y` stay apart: the first is the physical origin of a root
scaled uniformly by `root_scale`, the second a reference-unit position inside a stretched `.screen`.

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
