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

Any RCSS length meant to respect the user's scale setting uses the `dp` unit instead of `px`.
`10dp` becomes `10 * (UIScalePercent / 100)` real pixels; `10px` always stays exactly 10 real
pixels regardless of the setting. This is opt-in per property, not a blanket rescale — a window
using `px` throughout is simply unaffected by `UIScalePercent` until it's retrofitted. Most
already-migrated windows (login, menu bar, system menu, remember-password) still use `px` and
that's fine; retrofit to `dp` opportunistically, not as a forced mass-edit.

## Two scaling systems, cross-wired onto both axes

A second, older scaling system also exists: `UI::Scaling` (`UITransform.cpp`), a window-size-driven
auto-scale (`BottomHudScale`, `CappedUniformScale` → `PanelTransform`/`DockTransform`/
`FloatingWorkspaceTransform`), clamped to a fixed range per layout kind. It drives still-legacy
`CWin`/`mu::ui::window::CObject` rendering/hit-testing, and — via `bars_scale` — `main_frame.rcss`'s HUD bars too
(`NewUIMainFrameWindow.h`'s `MainFrameRmlModel::barsLeft` comment has the full reasoning for why
that one window uses this system instead of `dp`). Two axes exist, and both systems now respect
both:

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

**`WindowContentScale`-into-`dp` is not yet verified on real mismatched-density hardware.**
`Rml::Context`'s dimensions are set from SDL's window-coordinate size (`RmlUiRuntime::OnResize`),
not `SDL_GetWindowSizeInPixels()`, and the SDL window requests `SDL_WINDOW_HIGH_PIXEL_DENSITY`
(`SDLWindowFlags.h`) — so window-coordinate size and real pixel size can genuinely diverge on a
scaled display. Whether `RenderInterface_SDL_GPU`'s viewport already stretches that
window-coordinate-sized canvas across the full pixel framebuffer (in which case the `dp`-ratio
multiply would double-scale) or renders it 1:1 (in which case the multiply is the missing piece)
wasn't resolved by reading code alone — confirm on real scaled-display hardware, or with a debug
`UI::Scaling::SetWindowContentScale()` override, before trusting it in play. In the common case
where `SDL_GetWindowDisplayScale()` and `SDL_GetWindowPixelDensity()` already agree, `contentScale`
is 1.0 and this multiply is a no-op either way — the mismatch only shows up on the edge cases
`WindowContentScale` exists for (Wayland fractional scaling, a window dragged to a different-DPI
monitor), so a mistake here won't necessarily surface on typical hardware.

**Known RmlUi quirk**: in at least one document (`char_sel_main.rml`, which has a `data-model` and
several `data-event-click` bindings), a `font-family`/`font-size` declared on an ancestor
(`#panel`) failed to inherit down to a `<span>` two levels deep — RmlUi logged "No font face
defined. Missing 'font-family' property" and the text silently never rendered, even though every
other inherited property (and the div's own background/border) worked fine. Root cause not fully
chased down. Workaround, confirmed working: declare `font-family`/`font-size` directly on
whatever element actually renders the text, rather than relying on inheritance from a distant
ancestor, in any RmlUi document that uses `data-model`/`data-event-click` bindings.

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
| A panel whose real screen position also drives an unrelated live 3D viewport in real pixels | Stays fixed `px`, not `dp` — `CCharMakeWin`'s `#panel` is the standing example: its real position feeds `RenderCreateCharacter()`'s `BeginOpengl()` call directly, a separate scale mechanism that isn't safe to grow via `dp` without also correctly rescaling. A documented, deliberate non-scaling exception, not a gap. |

## The "C++ pushes real pixels into RmlUi" pattern is retired everywhere except one documented exception

Every migrated window's `Element::SetProperty("left"/"top"/"width"/"height", ...)` push from C++ —
the pattern `char_sel_main` retired first (see the worked example below) — is gone from `login`,
`login_main`, `sys_menu`, and `remember_password_prompt` too. RCSS anchor classes + fixed `dp`
sizes own every element's layout; C++'s only remaining roles are: a window's own genuine screen
placement (still legitimately C++-computed — `#panel`'s `left`/`top`, not its internal layout),
keeping a real non-RmlUi companion object (a `CButton` kept for click-detection redundancy, a
`CUITextInputBox` for real text entry) in sync with what RCSS decided by scaling the same fixed
offsets by the same combined ratio RmlUi's own `dp` ratio uses (`GameConfig::GetUIScalePercent() ×
UI::Scaling::ViewportFitScale()`) — `UI::Scaling::CompanionRatio(windowWidth, windowHeight)`
(`UITransform.cpp`) is the single shared implementation of this, extracted after
`CharSelMainWin.cpp`'s `GetUIScaleRatio()`, `LoginMainWin.cpp`'s inline version, and
`LoginWin.cpp`'s `LoginUIScaleRatio()` had each independently hand-copied it — and the exact same
staleness bug (reading `CInput::Instance().GetScreenWidth()/GetScreenHeight()` instead of the
`WindowWidth`/`WindowHeight` globals RmlUi itself uses) got reintroduced and re-fixed in more than
one of those copies before this existed. `LoginWin.cpp` still keeps its own thin zero-arg
`LoginUIScaleRatio()` wrapper for
its two call sites' convenience, but it just forwards to `UI::Scaling::CompanionRatio()` now —
`CharSelMainWin.cpp`/`LoginMainWin.cpp` call the shared function directly. The formula itself
lives in exactly one place either way. See also `char_make`'s deliberate `#panel` exception in the
table above. If a window with a real Type-2 companion (a functional `CUITextInputBox`, not just a redundant
click-detection `CButton`) is ever made draggable, `UI::RmlBridge::MakeDraggable()`'s existing
`OnPanelMoved` callback (`RmlDraggable.h` — zero live callers today) is the right hook for this,
but whatever gets wired into it will need to include the same combined-ratio scaling this section
describes, not just a raw position sync — `RmlDraggable.h`'s own gap note doesn't mention this yet
(it flags `px`-vs-`UIScalePercent` and missing persistence, both about the panel's *own* position,
not a companion object's).

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

**The legacy hit-test objects must stay numerically in sync with the CSS, not just visually
similar.** `CCharSelMainWin` still keeps a `CButton`/`CSprite` per element alive purely for
`CUIMng::IsCursorOnUI()`/click-detection bookkeeping (never rendered — RmlUi owns 100% of the
visuals). Positioning those legacy objects via the *old* `CalculateLayout()` while the RmlUi
visuals moved to the *new* fixed-dp-anchor math caused a real, user-visible bug: at resolutions
where the two calculations diverge, a click on the visually-correct (RmlUi-rendered) Delete button
landed outside the legacy `CButton`'s hit rect, so `IsCursorOnUI()` reported the cursor as being
over the 3D world instead of UI — letting `CharacterScene::Update()`'s world-click handler reset
the character selection in the same frame, silently no-op'ing Delete. Fixed by adding
`UI::CharacterSelection::CalculateFixedAnchorLayout()` (`CharSelMainWin.h`), a second calculator
that mirrors the RCSS's fixed-dp-anchor math exactly (scaled only by `UIScalePercent`, not
resolution), and switching `ApplyLayout()`'s legacy-object feed to it. **Takeaway for the next
retrofit**: if a window keeps legacy hit-test objects alive alongside RmlUi visuals, whatever
positions those objects must be derived from the *same* math as the CSS, not just "close enough
at the reference resolution" — verify by actually clicking through create/delete/connect-style
flows post-retrofit at more than one resolution, not just eyeballing a screenshot.

## Deferred (not part of this policy yet)

- A formal multi-resolution automated visual-regression test matrix. No test harness exists for
  this today; keep doing manual spot-checks at a couple of resolutions/UI-scale values per window.
- Consolidating legacy duplicate component classes (`CButton`/`CUIButton`/`mu::ui::window::CButton` etc.) —
  those are legacy C++ duplications predating RmlUi; migrating a window to RmlUi already retires
  its legacy duplicate, so consolidating the *remaining* legacy classes is a separate effort
  against code that's being phased out anyway. (Directory reorganization itself — dropping the
  `UI/Legacy`/`UI/NewUI` folder split in favor of topic folders directly under `UI/` — is done, see
  `docs/newui-legacy-merger.md`; the class-name collision this surfaced, `CButton` vs. the former
  `mu::ui::window::CButton`, is exactly this still-deferred class-consolidation item.)
