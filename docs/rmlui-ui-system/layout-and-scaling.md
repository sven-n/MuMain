# Layout, Anchoring & Scaling

How this branch implements [`architecture-principles.md`](architecture-principles.md)'s §§1, 5,
7–9, 23–24 (layout intent, responsive/aspect-ratio behavior, centralized UI scale, no
resolution-specific hacks) concretely, in RCSS. Read this before porting the next window or
touching an existing one's RCSS.

## Reference resolution: there isn't one

This codebase has never had a virtual canvas or letterboxing. RmlUi documents are sized in real
window pixels directly — a `Rml::Context` is created and resized to the actual swapchain
dimensions (`RmlUiRuntime::Create()`/`OnResize()`). Don't invent a "design resolution" and scale
against it; size and position elements the way described below instead.

## Global UI scale: `dp`, not a custom calculator

`Rml::Context::SetDensityIndependentPixelRatio()` is RmlUi's own built-in mechanism for a
user-controlled UI scale, and it's now wired up: `GameConfig::GetUIScalePercent()` (persisted,
`[UI] UIScalePercent=100`, default 100) is read once and applied via
`context->SetDensityIndependentPixelRatio(percent / 100.0f)` in both `RmlUiRuntime::Create()` and
`OnResize()` (`Render/RmlUi/RmlUiRuntime.cpp`).

The setting has an in-game control: **Options window → UI tab → "UI Scale"**, a dropdown over a
fixed ladder (50/60/70/80/90/100/125/150/200 %) with a hover tooltip saying what the percentage
multiplies. Picking a value writes `GameConfig::SetUIScalePercent()` (clamped to
`CfgMinUIScalePercent`..`CfgMaxUIScalePercent`, i.e. 50–300 — a hand-edited `config.ini` may sit
between two offered steps, and the row then shows the nearest one), saves, and re-applies the scale
by resizing the window to the size it already has (`MuApplyWindowResolution(WindowWidth,
WindowHeight, windowed)`): nothing recomputes the ratio on its own, but every resolution-dependent
system — all three contexts' `dp` ratio, `UI::Scaling`'s active transform, the workspace,
the 3D UI cameras — does so on a resize. That apply is deferred to `COptionWindow::Update()`, out of
RmlUi's own event dispatch, like the theme switch next to it.

Any RCSS length meant to respect the user's scale setting uses the `dp` unit instead of `px`.
`10dp` becomes `10 * (UIScalePercent / 100)` real pixels; `10px` always stays exactly 10 real
pixels regardless of the setting. This is opt-in per property, not a blanket rescale — a window
using `px` throughout is simply unaffected by `UIScalePercent` until it's retrofitted. Most
already-migrated windows (login, menu bar, system menu, remember-password) still use `px` and
that's fine; retrofit to `dp` opportunistically, not as a forced mass-edit.

## Two scaling systems, cross-wired onto both axes

A second, older scaling system also exists: `UI::Scaling` (`UITransform.cpp`), a window-size-driven
auto-scale (`BottomHudScale`, `CappedUniformScale` → `PanelTransform`/`DockTransform`/
`FloatingWorkspaceTransform`), clamped to a fixed range per layout kind. The ramp between the
640×480 reference (1.0×) and each ceiling is **linear** — `ViewportFitScale()` is
`clamp(min(w/640, h/480), 1, ceiling)`, the same formula the original client used — so at
`UIScalePercent=100` a migrated window lands on exactly the pixels the legacy one did at every
resolution, which is what makes screenshot comparison against the original meaningful. Don't damp
the ramp (a quadratic one broke that parity at every intermediate resolution); the user dial is
the lever for "too big at my resolution". It drives `CObject` windows' native rendering and
hit-testing and the workspace's region scales. The main frame HUD is sized in `dp` like
every other window; `BottomHudScale()` is the same number, still used to seat docked windows on
the HUD's top edge. Whether the cursor is over the HUD is asked of the HUD itself
(`CMainFrameWindow::IsMouseOverHud()`: the hovered element belongs to `main_frame.rml`), so it
follows wherever a theme places the parts. Two axes exist, and both systems now respect both:

- **`UIScalePercent`** — the user's own config-driven preference (`config.ini`'s `[UI]
  UIScalePercent`). Both RmlUi's `dp` ratio and `UI::Scaling`'s functions respect this.
- **`WindowContentScale`** (`UI::Scaling::GetWindowContentScale()`/`SetWindowContentScale()`) — an
  OS display-scale/pixel-density correction factor (`ContentScaleFromMetrics()` =
  `SDL_GetWindowDisplayScale() / SDL_GetWindowPixelDensity()`), refreshed at startup and on
  `SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED` (`Winmain.cpp`). Folded into `UI::Scaling`'s clamp
  *bounds* (both min and max multiplied by it, widening the auto-scale's own headroom), and into
  RmlUi's `dp` ratio via `RmlUiRuntime.cpp`'s `ApplyUIScale()`.

**The two axes are applied differently, deliberately.** `UIScalePercent` is a direct user dial, so
everywhere it's folded into `UI::Scaling` it's a **post-clamp** multiplier — at a window size where
the auto-scale already sits at its ceiling (common at typical/large resolutions), a clamp-bound
fold would mean changing the percent does nothing, silently defeating the setting. `contentScale`
is folded into the **clamp bounds** instead, since its job is widening legitimate high-DPI headroom,
not acting as a 1:1 user dial.

**`contentScale` widens only the upper clamp bound.** On a 125 %-scaled Windows display, folding
it into the lower bound forced scale above 1.0 at the 640×480 reference and overflowed every
reference-pixel layout there; `ViewportFitScale()` keeps the lower bound at `1.0f`, and the `dp`
auto-fit reuses it.

Still open: whether a genuine high-pixel-density panel (where `SDL_GetWindowPixelDensity() > 1`,
not just an OS scale preference) needs different handling for the `dp`-ratio path specifically —
`Rml::Context`'s dimensions come from SDL's window-coordinate size (`RmlUiRuntime::OnResize`), not
`SDL_GetWindowSizeInPixels()`, so window-coordinate size and real pixel size can still genuinely
diverge there. Confirm on real high-DPI hardware before trusting that path in play (OS-scaled hardware is confirmed).

See `engine-findings.md`'s font-family inheritance finding before assuming a new element's
invisible text is a layout bug — it's the single most-recurring gotcha in this doc set.

## Anchor/sizing utility classes (`base.rcss`)

Both themes' `base.rcss` define an identical set of pure-layout utility classes (no visual styling
— nothing theme-specific to vary):

| Class | Effect |
|---|---|
| `.anchor-top-left` / `.anchor-top-right` / `.anchor-bottom-left` / `.anchor-bottom-right` | `position: absolute` + the matching two edge offsets at `0` |
| `.center-x` / `.center-y` / `.center-both` | `left`/`top: 50%` + `transform: translate(-50%, ...)` |
| `.stretch-x` / `.stretch-y` / `.stretch-both` | `position: absolute` + opposing edges at `0` (width/height derive from the parent automatically) |

A window's intended anchor becomes a class name on the element (`class="btn-icon
anchor-bottom-left"`), combined with a fixed `dp` size and any per-element offset override (an ID
rule like `#btn_create { left: 22dp; }`) — not a C++-computed rect pushed in from the window's
`ApplyLayout()`/`Create()`. This is the direct answer to "how do I position a new element": pick an
anchor class, give it a `dp` size, done.

## Fixed-vs-fluid guidance

| Content shape | Approach |
|---|---|
| Dialogs, buttons, icon-sized chrome | Fixed `dp` size, anchored to a corner/edge (`.anchor-*`) |
| HUD elements pinned to a screen edge | Edge-anchored (`.anchor-*` on the relevant edges only) |
| Centered prompts/messages | `.center-x` / `.center-y` / `.center-both` |
| Backgrounds/bars meant to fill available space | `.stretch-x` / `.stretch-y` / `.stretch-both` — use only when the element is genuinely meant to grow with its container (an info bar between two buttons), not as a default |
| Content whose position is a genuine live computed result (3D-projection, following a moving target) | Still fine to push from C++ every frame — `CCharInfoBalloonMng`'s balloons are the standing example. This isn't something the anchor-class system should be forced onto. |
| A panel that shows live 3D | A `RenderTarget` image sized by its own box (`CCharMakeWin`'s `#preview`), so the panel can scale like any other. `char_make`'s `#panel` is still fixed `px`, as it was when its position fed a native viewport; moving it to `dp` needs nothing from the preview. |

## The "C++ pushes real pixels into RmlUi" pattern is retired everywhere except one documented exception

No migrated window pushes `left`/`top`/`width`/`height` into its elements from C++: RCSS anchor
classes and fixed `dp` sizes own internal layout. C++ keeps two roles: a window's own screen
placement where no workspace slot or RCSS anchoring covers it, and keeping a native companion
(a `CSprite`/`CButton` kept for hit-testing) in step with what RCSS decided, by scaling the same
fixed offsets by the ratio RmlUi's `dp` uses — `UI::Scaling::CompanionRatio(windowWidth,
windowHeight)` (`UITransform.cpp`), the one implementation. Read `WindowWidth`/`WindowHeight`
there, not `CInput`'s screen size, which went stale in more than one hand copy before this
existed. `char_make`'s `#panel` is still `px` (table above), though nothing requires it any more.

## Worked example: `CCharSelMainWin`'s retrofit

The character-select button bar (`char_sel_main.rml`/`.rcss`) is the pilot this policy was proven
against. Before: `CCharSelMainWin::ApplyLayout()` called
`UI::CharacterSelection::CalculateLayout()` (an upstream auto-scale-to-fit-800x600 calculator) and
pushed a fully computed `left/top/width/height` in `px` for every RmlUi element, every
`Create()`. After: every element anchors itself via `base.rcss` classes with a fixed `dp` size in
`char_sel_main.rcss` (values taken directly from `UI::CharacterSelection`'s own `Native*`
constants, so the two are visually identical at the historical 800x600/100%-scale case and
intentionally diverge at other resolutions — fixed-size-anchored-to-a-corner, not
scaled-proportionally-to-800x600, is the policy going forward).

**Native hit-test objects must stay numerically in sync with the CSS, not just visually
similar.** `CCharSelMainWin` keeps a `CSprite` per element, never rendered, for its own
`UpdateMouseEvent()` hit-testing. Placed by the old calculator while RmlUi used the fixed-dp
anchors, a click on the drawn Delete button missed the sprite's rect at some resolutions, the
world-click handler reset the selection, and Delete silently did nothing.
`UI::CharacterSelection::CalculateFixedAnchorLayout()` (`CharSelMainWin.h`) mirrors the RCSS's
fixed-dp math (scaled by `CompanionRatio()`), and feeds the sprites; `CharacterScene.cpp` also
checks `Core::Input::IsMouseOverUI()` now, so a stale rect is no longer the only guard. **Takeaway for the next
retrofit**: if a window keeps legacy hit-test objects alive alongside RmlUi visuals, whatever
positions those objects must be derived from the *same* math as the CSS, not just "close enough
at the reference resolution" — verify by actually clicking through create/delete/connect-style
flows post-retrofit at more than one resolution, not just eyeballing a screenshot.

## Reading a live RCSS box back into native hit-test space

`UI::RmlBridge::RefreshLogicalPanelSize()` (`RmlPanelGeometry.h`) is how a native `WindowGeometry`
hit box follows its theme's own `#panel` instead of a hardcoded constant. **Its result needs no
scale conversion, and applying one is a bug.** Every `#panel` it reads is sized in plain `px` and
scaled only at paint time, by `transform: scale(root_scale)` (`SyncRootTransform`); RmlUi's layout
box ignores a render-time transform, so `GetBox()` already returns reference-space extents. An
earlier version divided by the active transform, which shrank each hit box by the UI scale — at the
usual capped 2.0 only the panel's top-left quarter stayed clickable, and every click outside it fell
through to the world, walking the character (which in the vault's case also closed the window). It
hit all 17 call sites: the 9 inventory-family windows and the 8 docked-family ones.

The trap is that `px` in an RmlUi document means different things depending on whether the scale
lives in a `transform` or was pre-multiplied in C++ before being handed to RCSS. `RmlTooltip.cpp`'s
`#tooltip_panel` read genuinely *is* in screen pixels — that document carries no root transform and
its C++ pre-multiplies the scale into the width it sets. Don't generalize from one to the other;
check which of the two a document is before converting anything read out of its boxes.

The same asymmetry applies within a single element tree, and `RefreshLogicalAnchorPosition()` had
exactly this defect before it was fixed: `GetAbsoluteOffset()` returns `root_x + childLocalOffset`,
where `root_x` was pre-multiplied by the scale but the child's own offset was not, so un-mapping the
whole sum through the transform wrongly divided the child half. It now takes the child's offset as a
**delta against `#panel`** and adds the caller's own position — a signature that cannot express the
bug — and its three callers were corrected with it.

### Checking it: the scale sweep

A window's native bookkeeping and its RCSS agree trivially at scale 1.0 and can disagree at every
other scale, so a window whose hit box or anchors come from live RCSS (the callers of
`RefreshLogicalPanelSize()`/`RefreshLogicalAnchorPosition()` — grep for the current list) is
checked at **50 %** and **200 %** (Options → UI → UI scale, applies live), in both themes:

1. **Click every interactive element**, and for item grids at least one cell in each **corner** —
   a proportional error leaves the top-left working and fails the far edges.
2. **Confirm the click lands on the window, not the world**; the tell for a miss is the character
   walking.
3. **Hover anything with a tooltip or popup**; it must sit on its element, not be pulled toward
   the panel's top-left (the anchor-readback failure).

Sites that read RmlUi geometry without converting it are each correct for their own reason:
`CGenericConfirmDialog` (`dp` panel, no root transform: its box is screen px), `CMainFrameWindow`
(screen-px slot box divided by the dp ratio, `SlotBoxInReference()`), `CNPCDialogue` (a
difference of two offsets over a pitch: units cancel), `RmlTooltip` (no root transform, scale
pre-multiplied in C++), `COptionWindow` (screen-px rect against `MouseX/MouseY`, safe only
because `INTERFACE_OPTION` is `LayoutMode::Legacy`). "Does this need a conversion?" has no single
answer — check which kind a document is.

Not covered by the sweep: resolution (scale is the sharper probe; `PanelTransform` derives scale
from resolution), drag state across a scale or theme change, and a theme change while a window is
open. These are not tracked: whoever touches a window checks them for it.

## Deferred (not part of this policy yet)

- An automated multi-resolution visual-regression test; keep doing manual spot-checks per
  window until one exists.
- Whether `::CButton` (no production consumer) and `mu::ui::window::CButton` should merge. Each
  is in its own namespace; porting a window retires whichever it used, so this only matters for
  the shrinking native population.
