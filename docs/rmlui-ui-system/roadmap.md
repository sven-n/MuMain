# Roadmap

What's actually done, what's next, and the decisions still open — distilled from the full
internal migration plan for anyone continuing this work without access to that plan directly.
For *how* the parts that exist actually work, see [Architecture](architecture.md),
[Theming & Modding](theming-and-modding.md), and [Gotchas](gotchas-and-patterns.md) — this doc is
about what's ahead, not how the finished parts function.

---

## Done so far

- **Phase 0 — groundwork.** RHI scissor support, vertex format conversion, the render/system
  interface, the build system, `UI::RmlBridge::RmlModelBinder`.
- **Phase 0.8 — input arbitration.** Event-driven, at the SDL-event level; RmlUi offered every
  event first, ahead of the two legacy UI tiers.
- **Phase 1 pilot.** The loading screen and `CLoginWin` (the login/character-select dialog) —
  the reference implementation most of this doc set is written against.
- **Batch 2.** The rest of the login scene: `CLoginMainWin`, `CSysMenuWin`, `COptionWin`,
  `RememberPasswordPrompt`. Together with the pilot, this is **Loading Screen + Login Screen,
  scene-complete** — including `CSysMenuWin`/`COptionWin`, which are shared instances also reachable
  from the Character Select scene (opened via the same in-game Menu button there, against a taller
  panel).
- **Theming framework.** Pluggable, data-driven, modder-pluggable theme folders; two real themes
  (`legacy`, `modern`) covering all five migrated windows.

## Not started: the rest of the `CUIMng`-tier screens

Everything above is `UI/Windows`/`UI/Legacy/UIMng`-tier (`CWin`-based). The remaining windows in
that same tier are still fully legacy: `CCharSelMainWin`, `CCharMakeWin`, `ServerSelWin`,
`CreditWin`, `CMsgWin`, `CServerMsgWin`.

**Next natural slice: Character Select.** Because Batch 2's scope happens to line up with actual
scenes, the natural next unit is the Character Select scene — but most of what it needs is already
covered: `CSysMenuWin`/`COptionWin` are the same shared instances Login already migrated. What's
actually left is `CCharSelMainWin` (the character-slot screen: Create/Menu/Connect/Delete) and
`CCharMakeWin` (character creation). `CMsgWin`/`CServerMsgWin` are small and scene-local — not the
same class as the shared `NewUICommonMessageBox` engine Phase 2a targets below, so migrating them
doesn't pull that larger engine in early.

`CCharMakeWin` is a real edge case worth flagging explicitly: it renders its 3D character preview
full-screen-behind the UI, the same shape the login screen's 3D tour camera already uses — it's
technically migratable *without* whatever Phase 4 ends up building. It's being kept out of the
Character Select slice anyway, deliberately, so "has a 3D preview → waits for Phase 4" stays a
clean rule instead of a case-by-case judgment call repeated for every future 3D-adjacent window —
inventory item-hover and character info are the two Phase 4 actually exists to unblock, and unlike
`CCharMakeWin`'s full-screen-behind preview, both place 3D content *inside* a bounded panel, which
is exactly the case Phase 4's still-open design question (see below) is about.

---

## Phase 2 — Message-box engine, then a slice of the HUD

**2a.** Migrate the generic engine first: `NewUICustomMessageBox.cpp` (7,690 lines) and
`NewUICommonMessageBox.cpp` (3,773 lines) become one or two parameterized RML templates plus one
adapter. Highest-leverage step in the whole plan — dozens of call sites across
`UI/NewUI/Events/`, `GameLogic/Events/`, and `GameShop/MsgBoxIGS*.cpp` just configure the generic
box, so each becomes a small mechanical follow-up once the engine exists, not a fresh migration
per call site.

**Real, not-yet-verified dependency this phase inherits**: `NewUICustomMessageBox` owns a
`CUITextInputBox* m_pInputBox` (`UI/Legacy/UIControls.h`) for its text-input mode —
`UI/NewUI` has no text-input widget of its own anywhere; `UI/Legacy` is the engine's only owner of
real text rendering (GDI `TextOut`), and every real text field in `NewUI`, this one included,
borrows that one widget rather than reimplementing it. `CLoginWin` already solved "where does real
text input come from" once, with RmlUi's native `<input>` — but that's proven only for the login
screen so far. Re-verify it holds for a message box's input mode specifically before assuming it
does; don't inherit the assumption silently the way the render-to-texture one in Phase 4 almost
was.

**2b.** Migrate one or two HUD sub-widgets with genuinely per-frame-changing data — the
health/mana bars inside `NewUIMainFrameWindow.cpp` — without migrating the whole HUD yet. Validates
the `MarkDirty` binding path against real hot-path data at small scale before Phase 3 commits the
whole HUD to the same primitive.

**Blocking prerequisite, not yet built — read this before starting Phase 2.** Every window
migrated so far is `CWin`/`CUIMng`-tier. Phase 2 onward targets `UI/NewUI`
(`CNewUIObj`/`CNewUIManager`, ~90 windows) — a different manager, a different base interface, and a
pattern that has **never been proven in real code**. See the next section before treating anything
below as a settled design.

### The NewUI-tier adapter/facade pattern — designed, never implemented

The original plan's pilot was supposed to be two things together: `CLoginWin` (to prove the
`CUIMng`-tier coexistence design) *and* one small, low-traffic `UI/NewUI/Events/` dialog (to prove this
pattern, against `CNewUIManager`). Only the first half happened. Confirmed by direct search: zero
RmlUi references anywhere under `UI/NewUI/Events/`, zero `RmlAdapter`-suffixed classes anywhere under
`UI/NewUI/`. Every phase from here on depends on a pattern that currently exists only as a design
sketch.

The sketch, worked example (inventory): `g_pMyInventory` today expands to
`CNewUISystem::GetInstance()->GetUI_NewMyInventory()`, returning `CNewUIMyInventory*`. Introduce
`CNewUIMyInventoryRmlAdapter : public CNewUIObj` (satisfying the same `INewUIBase` interface —
`Render`/`Update`/`UpdateMouseEvent`/`UpdateKeyEvent`/`GetLayerDepth`/`IsVisible`, confirmed in
`UI/NewUI/NewUIBase.h`) that:

- Owns an `Rml::ElementDocument*` and an `RmlModelBinder<InventoryModel>` — same primitive the
  `CWin`-tier windows already use.
- Re-implements the exact public methods legacy callers use (`InsertItem`, `EquipItem`,
  `DeleteItem`) — bodies update the underlying item state, then call `MarkDirty("items")`.
- Makes `UpdateMouseEvent()`/`UpdateKeyEvent()` no-ops that always report "not consumed" (RmlUi's
  own context does hit-testing); the adapter only needs to exist in `CNewUIManager`'s list so
  z-order slotting and `Show()`/`Hide()` keep working unmodified.

Then change **one line** — the `g_pMyInventory` macro's target — to resolve to the adapter. Every
existing call site keeps compiling and working unchanged.

**Before committing Phase 2's real scope to this**: build a small real example first (the
originally-intended `UI/NewUI/Events/` dialog, or an equally low-blast-radius `CNewUIObj`-tier window) and
get it manually verified working, the same way `CLoginWin` proved the `CWin`-tier pattern before
Batch 2 scaled it up. No amount of documentation substitutes for one working reference
implementation a contributor can copy from.

## Phase 3 — HUD

With 2b having validated the binding layer's hot-path cost, migrate the rest of
`NewUIMainFrameWindow.cpp` (hotkeys; minimap chrome likely stays `<img>`-hosted, since the
minimap's rendered content is itself a texture) plus closely-related always-on pieces
(`NewUIBuffWindow.cpp`, `NewUIQuickCommandWindow.cpp`). First highly-visible, always-on
migration — treat it as a checkpoint for a broader perf/visual sign-off before continuing.

## Phase 4 — 3D-in-UI bridge (design open: direct composite vs. render-to-texture)

`CNewUI3DRenderMng`/`INewUI3DRenderObj::Render3D()` lets panels host real 3D content (rotating
item/character previews) — something plain HTML/CSS can't express, and a prerequisite for most of
what's left (character info, inventory item-hover, item-explanation windows).

**Original plan assumed an offscreen render-to-texture bridge**: keep `Render3D()` rendering
exactly as today but target an offscreen render target sized to the preview panel, then expose
that texture to RmlUi via an `<img>` (or custom element) pointed at the existing GPU texture
handle — no CPU readback. **Reading the actual mechanism first suggests something simpler might
already work.** `CNewUI3DCamera::Render()` (`UI/NewUI/NewUI3DRenderMng.cpp`) doesn't render to a
texture at all — it's the same shape as the login screen's 3D tour camera: `EndBitmap()` → a
full-window-viewport perspective pass with its own depth clear, drawing every registered
`INewUI3DRenderObj` directly into the backbuffer → `BeginBitmap()` to resume 2D, all sandwiched
into the existing frame *before* RmlUi's own render call. Since RmlUi always renders **last** in
the frame (see [Architecture §3](architecture.md#3-frame-lifecycle--the-render-order-contract)),
3D content going through this existing pass should already composite correctly underneath a
transparent-background RmlUi panel — the same trick already proven for the login screen's tour
camera, and found (but not exploited) for `CCharMakeWin`'s character preview.

**Confirmed directly against `NewUIInventoryCtrl`**: `CNewUIInventoryCtrl` and the picked-item
class `CNewUIPickedItem` (`UI/NewUI/Inventory/NewUIInventoryCtrl.h`) are both real
`INewUI3DRenderObj` implementers — inventory items are live 3D models per slot, not icon textures,
and the picked-up item's `Render3D()` re-centers on raw `MouseX`/`MouseY` every frame, i.e. it
genuinely follows the cursor once picked up (see Phase 5 below — the two phases are more coupled
than the original split suggested).

**Before committing to either design**: run a cheap early test — put a transparent-background
RmlUi panel over one `Render3D()`-driven item slot and confirm it composites correctly, the same
way the login tour camera already does. If it holds, Phase 4 shrinks from "build a new bridge" to
"verify the existing direct-composite pattern holds for per-slot content." Two real open questions
that test won't answer by itself, though: `CNewUI3DCamera::Render()` sets **one** full-window
viewport/perspective for every registered object at once, not one scoped to each panel — so
per-panel clipping (an item peeking outside its own inventory panel during scroll, say) isn't
automatic the way a true render-to-texture target would bound it "for free"; and "3D always
renders before all RmlUi" only works cleanly if nothing ever needs an RmlUi element to sit
*behind* some 3D content and *in front of* other 3D content — true for a flat item grid, not
guaranteed for something more layered later. A real render-to-texture bridge may still end up the
right long-term answer for those cases — this is a reason to test cheaply and early, not a reason
to assume the direct-composite trick is a final design.

Build against one real consumer first (`NewUICharacterInfoWindow.cpp`, simpler than inventory's
item-hover preview since it isn't entangled with drag/drop too), then reuse whichever design that
proves out for every subsequent window that needs it. No window with a 3D-preview dependency
should attempt migration before this is settled — `CCharMakeWin` is the one documented exception
(see above), and it's being held out anyway for consistency.

## Phase 5 — Shared inventory/drag-drop controller

`NewUIInventoryCtrl`/`NewUIInventoryActionController` is shared by inventory, storage, shop, and
trade, already parameterized by `STORAGE_TYPE` and an owning `CNewUIObj*`. Recommended: adapt the
shared controller so consumers can migrate independently (factor slot-drawing apart from
drag-state/interaction logic), rather than moving all four atomically — see Open Decisions below,
this is a real tradeoff, not a settled call. Main risk to validate early: cross-window drag/drop
between a migrated and a still-legacy consumer. Favorable signal, now directly confirmed rather
than assumed: `ms_pPickedItem` (`CNewUIInventoryCtrl::ms_pPickedItem`,
`UI/NewUI/Inventory/NewUIInventoryCtrl.cpp`) is a shared static `CNewUIPickedItem*` independent of
which grid the item came from, and `CNewUIPickedItem::Render3D()` (see Phase 4 above) drives the
"picked item follows the cursor" behavior purely off raw `MouseX`/`MouseY` — no dependency on
whichever grid's UI framework is currently drawing the slot underneath it. That's a real, load-
bearing reason to expect a migrated grid and a still-legacy one can share one picked-item state
correctly. Still needs a real cross-window test before trusting it for the actual drag gesture,
though — input arbitration (which side's hit-testing claims the mouse-down that starts a pick, and
the mouse-up that ends one) is a separate question this finding doesn't answer by itself.

---

## Not scoped anywhere: the `CUIControl` tier

Distinct from everything above, and easy to miss because it shares a folder name with something
else. `UI/Legacy/` physically contains **two unrelated legacy systems**, not one:
`UIMng.h/.cpp` (Phase 1's target — the `CWin`-tier manager, above) and a completely separate
manager, `CUIManager` (`UIManager.h/.cpp`, a distinct class from `CUIMng`), driving
`CUIControl`/`CUIMessage`-derived windows: `UIGateKeeper`, `UIGuardsMan`, `UIJewelHarmony`,
`UISenatus`, plus chat/friend-list/letter-box windows (`UIWindows.h`). ~17,300 lines combined —
larger than everything Phase 2a's message-box engine targets. Confirmed genuinely live, not dead
code: `g_pUIManager->UpdateInput()` runs every frame in the main gameplay scene
(`MainScene.cpp:250`), alongside `CNewUIManager`'s and `CUIMng`'s own per-frame calls — three
fully independent UI systems live simultaneously today, not two. Zero code relationship to
`UI/NewUI/` (confirmed by grep, zero cross-references either direction) or to the `CWin`/`CUIMng`
tier.

**Neither the original plan nor this roadmap assigns this tier to any phase.** Under the plan as
it currently stands, it would stay permanently legacy — not a deliberate deferral the way
`COptionWin`'s fate or Phase 4's design question are, but an actual planning gap: a whole third UI
framework the phased plan never scoped work for. Flagged here as a disclosed omission rather than
something a future contributor has to rediscover the way this entry was found.

Files, if this ever gets its own phase: `src/source/UI/Legacy/UIManager.h/.cpp` (the manager),
`UIControls.h/.cpp` (the `CUIControl`/`CUIMessage` base classes and most concrete controls),
`UIWindows.h/.cpp` (chat/friend-list/letter-box windows, `CUIWindowMgr`), `UIGateKeeper.*`/
`UIGuardsMan.*`/`UIJewelHarmony.*`/`UISenatus.*`/`UIPopup.*`/`UIMapName.*` (individual screens).

---

## Retirement criteria (per window, before deleting the legacy class)

1. Visual parity signed off against the legacy render (side-by-side, not just "looks right").
2. All inbound packet-handler call sites still compile and behave correctly against the
   adapter's/window's unchanged public methods.
3. All outbound action call sites reach the network layer with identical packet contents.
4. Input arbitration verified against both legacy tiers and other migrated windows at the
   assigned z-order band; world click-to-move correctly gated.
5. Any 3D-preview dependency verified against the Phase 4 bridge.
6. Localization strings render correctly in at least two locales.
7. No remaining reference to the legacy class outside its own file/the adapter (grep-checkable).

No generalized runtime A/B toggle by default — the adapter/hybrid pattern itself is the revert
mechanism (swapping construction back to the legacy class is a one-line change during PR review).
A real side-by-side toggle is a per-phase judgment call for the highest-blast-radius phases (2a,
3), not a standing policy.

---

## Open decisions (need a human call)

1. **Phase ordering.** The plan front-loads pipeline-risk retirement (the pilot, done) before
   value/visibility work (message-box engine, then HUD), with inventory deliberately last. An
   alternative would front-load the HUD for earlier stakeholder-visible progress, accepting more
   pipeline risk on a high-traffic target first. Never explicitly confirmed one way or the other —
   execution has proceeded on the plan's own recommended default (pipeline-risk-first) by default,
   not by an explicit decision.
2. **Inventory migration strategy (Phase 5).** Incremental (adapt the shared controller, migrate
   consumers independently, validate mixed-mode drag/drop) vs. atomic (migrate inventory/storage/
   shop/trade together, larger single change, zero mixed-mode risk). Recommended: incremental —
   a real tradeoff between blast radius and review-unit size, not a clear-cut call.
3. **Z-order banding.** The current design gives every migrated window one contiguous depth band
   ("above the 3D world, below every still-legacy window") rather than a full cross-system
   per-widget z-order authority. This will visibly break if a still-legacy window ever needs to
   render *between* two migrated ones — treat as fine until a concrete case in Phase 2/3 makes it
   not fine; don't pre-build a general authority against a hypothetical.
4. **`COptionWin` vs. `SEASON3B::CNewUIOptionWindow`.** `COptionWin` was migrated faithfully in
   Batch 2 (its checkboxes/sliders genuinely work) but is currently unreachable dead code — the
   live in-game Options screen is a separate, already-modern window,
   `SEASON3B::CNewUIOptionWindow`, opened from `CSysMenuWin`'s Option button, sharing the same
   underlying state via the `g_pOption` macro. Retiring one of the two, or rewiring `CSysMenuWin`'s
   Option button to the migrated one, is unresolved on purpose — see
   [Architecture §6](architecture.md#6-coexistence-bridging-cloginwins-specific-pattern).
5. ~~**D3D11 scissor twin.**~~ RESOLVED — not a coordination dependency. `RHI::` is the real
   abstraction boundary; this migration only needs to build/verify against GL (the only backend
   live on `main`), and a D3D11 backend implements the same `SetScissorEnabled`/`SetScissorRect`
   entry points independently, whenever that work is ready.
6. **Whether/when the `CUIControl` tier gets a phase at all.** Not a design tradeoff like 1-4
   above — genuinely unscoped. See [Not scoped anywhere: the `CUIControl` tier](#not-scoped-anywhere-the-cuicontrol-tier).

## Scaling & DPI — documented, not implemented

Legacy `NewUI` panels visibly stretch on non-4:3 resolutions
(`Render/Textures/ZzzOpenglUtil.cpp`'s `ConvertX`/`ConvertY` scale `x`/`y` independently, with no
uniform-fit correction anywhere). Not something this migration needs to fix directly, but every
window this migration retires stops stretching as a side effect, for free. RmlUi's own real,
built-in scaling primitives (`%`, `vw`/`vh`, `dp` via
`Context::SetDensityIndependentPixelRatio()`) are verified present in the vendored 6.2 source but
**nothing wires them up yet** — using `dp` in a theme's RCSS today has zero visible effect. A
future window with no sprite dependency should use `dp` driven by a single uniform fit factor
(`min(WindowWidth/refWidth, WindowHeight/refHeight)`) once something actually computes and applies
that ratio — never independent per-axis scaling, which is the exact bug being avoided.

## Critical files for Phase 2 onward

- `src/source/UI/NewUI/NewUIBase.h` — `INewUIBase`/`CNewUIObj`, the interface every future adapter
  must satisfy.
- `src/source/UI/NewUI/NewUIManager.cpp` — the front-to-back-first-consumer-wins loop the input
  bridge already plugs into for the `CUIMng` tier; the same shape a NewUI-tier adapter needs to
  slot into.
- `src/source/UI/NewUI/HUD/Skills/SkillTooltipModel.h/.cpp` — the one existing precedent (pre-dates
  RmlUi) for a plain-data model built once from game state and consumed by more than one renderer;
  `RmlModelBinder` generalized this shape, not invented it from nothing.
- `src/source/UI/NewUI/Events/` (e.g. `NewUIBloodCastleTime.cpp`, `NewUICatapultWindow.cpp`) — small,
  self-contained candidates for the still-undone adapter-pattern pilot dialog.
- `src/source/UI/NewUI/Dialogs/NewUICustomMessageBox.cpp`, `NewUICommonMessageBox.cpp` — Phase 2a's
  target.
- `src/source/UI/NewUI/HUD/NewUIMainFrameWindow.cpp` — Phase 2b/3's target.
- `src/source/UI/NewUI/NewUI3DRenderMng.h/.cpp` — Phase 4's target.
- `src/source/UI/NewUI/Inventory/NewUIInventoryCtrl.h`, `NewUIInventoryActionController.cpp` —
  central to the Phase 5 decision.
- `src/source/UI/NewUI/NewUISystem.h` — the ~90 global-macro definitions retargeted one at a time
  as each window migrates.
