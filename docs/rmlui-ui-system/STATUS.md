# Status Against the Architecture Principles

Living document — update this, not `architecture-principles.md`, when status changes. See
[`architecture-principles.md`](architecture-principles.md) first if you haven't read it; section
numbers below refer to it. [`legacy-theme-modernization.md`](legacy-theme-modernization.md) amends
it on one specific question — when legacy-theme C++ behavior should move into RML/RCSS versus
genuinely stay in C++ — worth reading before auditing any legacy-theme code against this document.

## What's migrated

- **Login/character-select scene (`CWin` tier)** — fully migrated, no remaining
  legacy-`CWin`-rendered chrome: `CLoginWin`, `CLoginMainWin`, `CSysMenuWin`,
  `RememberPasswordPrompt`, `CCharSelMainWin`, `CCharMakeWin`, `CCharInfoBalloonMng`, `CMsgWin`.
  `COptionWin` ported but deliberately not wired up (confirmed unreachable in live play — see
  `README.md`'s Coexistence patterns). Built and verified against a real server, both themes.
- **In-game HUD (`CNewUIObj` tier)** — `CMuHelperBar` (map/position readout + MU Helper bot
  control bar) and `CBuffStrip` (active-buff icon strip, the `data-for`/dynamic-array pilot at
  this tier) are fully done. `CNewUIMainFrameWindow`'s 3-phase HUD-frame port is **2 of 3 phases
  done**: Phase 1 (HP/MP/AG/SD/EXP bars + 5 corner buttons) and Phase 2 (`CNewUISkillList` —
  compact hotkey row click/hover/cooldown, expanded skill grid, pet-command row, and skill
  tooltips for both themes, replacing the old hand-rolled `EVENT_STATE` hover/click machine
  entirely). **Icon/box-frame art for the skill grid and pet row stays legacy 2D**, a deliberate
  Phase 2 scope cut — see the pilots-to-revisit table below. Phase 3 (`CNewUIItemHotKey` — potion
  slots, 3D-camera-composited icons, no RmlUi pattern proven for that yet) is not started. All
  landed pilots built and verified against a real server, both themes. The rest of this tier —
  ~88 other `CNewUIObj` windows, drag-and-drop, and 3D-camera-space rendering generally — is not
  yet migrated.

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
5. RmlUi-facing asset naming, and the C++ class name itself, reflects what the component actually
   is, not the legacy tier it came from (§12) — e.g. `mu_helper_bar`/`CMuHelperBar`, not
   `hero_position_info`/`CNewUIHeroPositionInfo`. **Renamed at port time, not deferred** — the one
   exception is a legacy file that welds multiple classes together where only some are ported in
   the current pass (`CNewUIMainFrameWindow`'s own file — see "Tracked deferral:
   `CNewUIMainFrameWindow`'s own class rename" below); don't treat that as a general excuse to
   defer a rename otherwise.
6. Both themes updated in the same pass, never one left behind. A rendering technique (e.g. an
   icon atlas) is verified to actually work at runtime before being trusted — see "Findings"
   below for a case where it didn't.
7. Uses reusable components/primitives where they exist; doesn't invent a new one-off mechanism
   when an existing pattern already covers the need — though see "Known gaps," several of the
   principles' presumed primitives don't exist yet on this branch.

## Findings worth knowing before the next port

Empirical facts about *this specific codebase/engine build*, not general policy — kept here rather
than in `architecture-principles.md` for that reason. Most are RmlUi-build-specific engine quirks;
a couple are about the legacy `CWin`/`CUIMng` C++ architecture surrounding RmlUi instead. Tier
-specific findings (`CNewUIObj`-tier) live in `newui-tier-adapter.md`'s "Proven by CBuffStrip"
section in full detail; summarized here for visibility.

- **This RmlUi build silently fails to inherit `font-family` into most subtrees** — a descendant
  needs its own explicit `font-family` declaration; relying on inheritance from an ancestor
  (`#panel`, `body`, a shared class) renders that element's text invisible with no error. Not
  scoped to `data-model`-bound documents specifically (an earlier, wrong diagnosis) — `login_main.rml`
  has no `data-model` at all and still needs this. Every text-bearing selector across
  `themes/modern/*.rcss` now declares its own `font-family: "token(font-body)"` rather than
  inheriting one.
- **`UI::RmlBridge::LoadThemedDocument()`'s design-token substitution (`RmlTheme.cpp`) must
  resolve a `<link href>` against `sourceUrl`'s directory, not wherever the RML text was actually
  read from.** For any document with no `themes/<theme>/<name>.rml` override (everything except
  `login`/`msg_win`/`remember_password_prompt`/`main_frame`), the RML content itself comes from
  the shared fallback path, but RmlUi still resolves `<link href="base.rcss">` against
  `sourceUrl` (always `themes/<theme>/`) when it later parses the substituted text — using the
  fallback content's own directory to find the `.rcss` file to substitute looks in the wrong
  place, fails silently, and leaves the `<link>` (and every `token(...)` inside it) untouched.
  Also fixed the same function iterating only the first `<link>` match instead of every one
  (a document links `base.rcss` then its own `<name>.rcss`; only the first was ever substituted).
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
  are only ever *called* during `MAIN_SCENE` — that alone isn't a visibility gate once a window's
  visuals move to a persistent RmlUi document, which renders every frame regardless of scene. See
  `newui-tier-adapter.md`'s third `MAIN_SCENE` prerequisite.
- **RmlUi's own default `display` value is `inline`, not `block`, for every element including
  `<div>`** (confirmed against the vendored source, `StyleSheetSpecification.cpp` — there's no
  browser-style user-agent stylesheet giving `<div>` a block default the way HTML does). An
  absolutely-positioned element gets a block box regardless (a side effect of being out-of-flow),
  which is why this is easy to miss — it only bites normal in-flow stacked content (e.g. multiple
  `<div>` lines via `data-for`), which renders as one run-on inline paragraph instead of separate
  lines without an explicit `display: block`. Give any non-absolutely-positioned element
  `display: block` (or whatever `display` it actually needs) explicitly — never assume a `<div>`
  gets one for free.
- **`linear-gradient`/`radial-gradient`/`conic-gradient` all route through
  `RenderInterface::CompileShader()`, not a shader-free vertex-colored mesh.** `DecoratorGradient.cpp`
  calls `RenderManager::CompileShader("linear-gradient", ...)` for all three gradient types and
  bails out (renders nothing) if it fails — the shader-free mesh path only exists for the older,
  deprecated `horizontal-gradient`/`vertical-gradient` decorator, which nothing in this project's
  RCSS uses. `RenderInterface_SDL_GPU` (`src/ThirdParty/RmlUi/Backends/`) now implements
  `CompileShader`/`RenderShader`/`ReleaseShader` for the gradient family, porting the upstream GL3
  reference backend's shader math to a new HLSL fragment shader
  (`RmlUi_SDL_GPU/shader_frag_gradient.frag`) baked into `ShadersCompiledSPV.h`, using this
  project's own `glslangValidator`/`spirv-cross`/`dxc` toolchain (already vendored for
  `MU_ENABLE_SHADER_COMPILATION`). Both `linear-gradient` and `radial-gradient` render correctly
  (real elliptical falloff, not just a flat fill). **`box-shadow`/`blur`/`backdrop-filter` are
  still unimplemented and out of scope** — those route through RmlUi's layer/filter/compositing
  subsystem (`PushLayer`/`CompositeLayers`/`CompileFilter`/`RenderFilter`), which
  `RenderInterface_SDL_GPU` doesn't implement at all — a materially bigger task. **Lesson**: a
  property having a working RCSS parser doesn't mean the render interface actually implements it
  — check for a `CompileShader`/render-interface override, or test the specific decorator in
  isolation, rather than trusting a casual screenshot.
- **`box-shadow` (and `filter`/`backdrop-filter`) parse but don't render on this engine.**
  `PropertyId::BoxShadow` has a working RCSS parser, which reads as "supported" if you only check
  property registration — but `RmlUiRenderInterface` leaves layer/filter compositing
  unimplemented, so a blurred `box-shadow` paints as a solid opaque block instead of a blur. Don't
  parser-check a rendering capability; check the render interface, or grep for prior art first —
  `login.rcss`'s own `#panel` comment already documents this.
- **RCSS comments don't nest, and a broken one in a shared file silently corrupts every document
  that links it.** `/* ... "/* example */" ... */` closes at the *first* `*/`, not the intended
  one — everything between that premature close and the next real `*/` gets parsed as garbage
  CSS. Since `base.rcss` is linked by nearly every modern-theme window, a broken comment there has
  a wide, confusing blast radius (multiple, seemingly-unrelated windows losing
  interactivity/positioning/visibility at once) that doesn't look like a syntax error at first
  glance. If a shared file's change is followed by multiple, seemingly-unrelated windows breaking
  at once, suspect the shared file's own syntax before anything else. Never write a literal
  `/* ... */` sequence inside prose that's itself inside a comment — describe the convention
  without the delimiters instead.
- **A confusingly-named legacy method can silently bind to the wrong RmlUi field.**
  `CNewUISkillList::IsSkillListUp()` (pre-existing, predates RmlUi) reports whether the hotkey row
  is scrolled to its "upper" slot set (6-9,0), not whether the expanded skill-list *popup* is
  open — despite what the name suggests. `main_frame.rml`'s `skill_grid_open` binds to a
  correctly-named `IsSkillGridOpen()` instead. Read what a legacy getter actually returns, not
  just what its name implies, before binding it into a model.
- **The legacy `CWin`/`CUIMng` click-activation system is a fundamentally less reliable signal
  than RmlUi's own click events, and this has caused real, confirmed bugs — one class fixed, one
  still open.** `CWin::Update()` gates `UpdateWhileActive()` (where every migrated window's RmlUi
  click/keyboard consumption lives, including real `CUITextInputBox` keystroke polling) behind
  `CWin::m_bActive`, which the legacy `CUIMng` activation system (list-order hit-testing via
  `CursorInWin()`, deferred one-frame activation) doesn't reliably grant on a timely basis. This
  produced two confirmed, independently-discovered bug mechanisms:
  - `CUIMng::Update()` used to re-enter `SetActiveWin(pWin)` every frame a click was held (since
    `IsLBtnDn()` is level-triggered), which redundantly deactivated the very window being
    re-clicked and starved `UpdateWhileActive()` for the click's whole held duration. **Fixed**:
    the click loop now skips the redundant `SetActiveWin()` call when the clicked window is
    already active and already head.
  - Every `RmlClickX()` handler used to set a flag (`m_bRmlXClicked`) for `UpdateWhileActive()` to
    consume later, rather than acting immediately — since that poll could go many frames without
    running at all, multiple flags could go stale and an `if/else if` consumer would fire a stale
    flag over a fresh one (observed as: click credit, nothing happens; click menu later, credits
    open instead — proof the credit click *had* registered, just never got consumed in order).
    **Fixed everywhere this pattern existed** (`CLoginMainWin`, `CLoginWin`, `CSysMenuWin`,
    `CCharMakeWin`): every `RmlClickX()` callback now calls its action directly, bypassing
    `m_bActive` entirely — safe because `RmlUiRuntime::ProcessSdlEvent()` (where these fire) runs
    from Winmain's SDL event pump, always before `CUIMng::Update()` the same frame. No
    `m_bRmlXClicked`-style flags remain anywhere in the codebase.
  - **The "world-click leaks through an RmlUi panel" half — fixed 2026-09-04, smaller than it
    looked.** `CUIMng::IsCursorOnUI()`/`m_bCursorOnUI` runs entirely through the legacy
    `CursorInWin()` hit-testing, with the same staleness risk already fixed above for click
    *dispatch* — but a full-codebase grep found it has exactly **2 live call sites, both in
    `Scenes/CharacterScene.cpp`** (character-select's click-to-select and 3D object-picking), not
    a sprawling problem across every scene. Both now also gate on `Core::Input::IsMouseOverUI()`,
    the exact same proven pattern as the `Selection.cpp`/`ZzzInterface.cpp` fix below — sufficient
    without inventing any new per-window bounding-box query, because `char_sel_main.rml`'s `#panel`
    is `pointer-events: none` (a pure positioning container spanning the full screen), so RmlUi's
    own hit-test already only reports true over the real interactive children. Closes the
    confirmed bug class (`CharSelMainWin.h`'s `CalculateFixedAnchorLayout()` comment — a
    resolution where its hand-duplicated rect diverged from the real RmlUi Delete button once made
    a genuine click read as "not on UI," silently no-op'ing Delete).
    **Deliberately still open**: `CalculateFixedAnchorLayout()`'s hand-duplicated math itself
    wasn't retired (the new gate means it's no longer the *only* thing standing between a stale
    rect and a wrong outcome, but a live RmlUi-element-bounds query replacing it entirely is a
    larger refactor with no confirmed bug driving it now); `CUIMng`'s two *other*, purely internal
    uses of `CursorInWin()` (click-activation dispatch, hover/`ActiveBtns`, `UIMng.cpp` ~732/~780)
    aren't touched either — hybrid windows' real clicks already bypass `m_bActive`/activation
    entirely via `RmlClickX()` (the fix above this one), so that staleness has no confirmed live
    consequence today. Three parallel, not-always-agreeing input-tracking systems are still the
    root cause worth remembering for the next symptom: `CInput`/`SEASON3B::CNewKeyInput`'s
    VK-polling (drives `CUIMng`'s activation/hit-test loop), `Winmain.cpp`'s own event-driven
    `MouseLButton`/`Push`/`Pop` globals (`HandleMouseButton`/`HandleMouseMotion`, legacy 2D world
    input — `Core/Platform/sdl3/SDLEventLoop.cpp` looked like the source of this but was actually
    dead code, zero live call sites; deleted 2026-09-04, don't go looking for it at all any more),
    and RmlUi's own event-driven `Context::ProcessMouseButtonDown/Up`.

## Known gaps against the principles (honest status, not yet built)

None of these are wrong so far — the principles doc explicitly endorses incremental delivery
(§26–27) — but they're real, currently-unaddressed gaps in the end-state architecture, not yet
even scheduled. Recorded so no future session mistakes "the pilots pass their own verification"
for "the full architecture is in place":

- **No mod/user-override resource-precedence system** (§18–19). Themes today are exactly two
  hardcoded directories (`themes/legacy/`, `themes/modern/`) selected by `GameConfig`'s theme
  name — no "user override on top of a theme" layer, no documented precedence order, no tooling
  for a third party to ship a partial theme that inherits the rest from a base theme. Not
  speculative — §18/§19 have called for this since the governing doc was written, and arbitrary
  future themes are a real, stated want (see the Custom/Test-theme entry below), not just legacy
  and modern. Correctly *sequenced* behind other work (nobody's shipping a mod today), but it's a
  stated requirement waiting on priority, not an open question about whether to build it.
- ~~No design-token/shared-variable layer for `legacy`; `modern`'s own layer is a naming
  convention, not a real mechanism.~~ **Fixed 2026-09-04 for `modern`.**
  [`modern-theme-visual-direction.md`](modern-theme-visual-direction.md) defines the palette/
  border/typography token table; `UI::RmlBridge::LoadThemedDocument()` (`RmlTheme.cpp`) now
  resolves a `token(name)` marker against `themes/modern/tokens.ini` before RmlUi ever sees the
  RCSS text — no engine changes needed (reuses RmlUi's own inline-`<style>`-block support). All 11
  already-shipped `themes/modern/*.rcss` files were migrated (mechanically, via the one-off
  `Tools/migrate_rcss_tokens.py`) from the old "value + comment" convention to real `token(...)`
  references, including a `font-title`/`font-body` split for the previously-repeated
  `font-family: "Liberation Sans"` literal (both stay the same value for now — no distinct
  display font chosen yet, this only names the future split). Caught and fixed one real,
  independent drift as part of the migration: the table said `border-metal` was
  `rgba(140, 146, 152, 140)`, but every actual shipped usage consistently used alpha `130` — the
  table had the typo, not the RCSS; both now say `130`. `legacy` still has no token layer and
  isn't in scope for one (theme-specific by design).
- ~~Three "shared" RML files have theme-specific class names baked into them — `architecture-
  principles.md` §15 violation.~~ **Fixed 2026-09-04.** `login.rml`, `msg_win.rml`, and
  `remember_password_prompt.rml` used to have `modern`-specific classes (`modern-frame`,
  `modern-frame-accent`, `modern-panel`, etc.) hardcoded directly into the file every theme is
  supposed to share equally. Per `theming-and-modding.md`'s Core Principle section: `legacy`'s
  copies are now the canonical, theme-neutral files; `modern` forks its own copy
  (`themes/modern/login.rml`/`msg_win.rml`/`remember_password_prompt.rml`) that keeps the
  modern-specific classes. No C++ changes were needed.
- ~~No drift-check tooling for a forked theme's RML.~~ **Fixed 2026-09-04.**
  `Tools/check_rml_rcss_drift.py`, wired into the build next to `check_rml_rcss_syntax.py`: for
  every window whose C++ calls `LoadThemedDocument`, confirms every id/`data-model` field/
  event-callback name it references appears somewhere across the shared file plus every
  `themes/*/` fork of that document (checked against the union of all copies, not each
  individually — a per-copy requirement flags the legitimate "different themes bind different
  precomputed alternatives" pattern, e.g. `main_frame`'s `hp_text` vs `hp_current_text`, as false
  drift).
- ~~Two C++ call sites branch on theme *name*, violating `architecture-principles.md` §30.~~
  **Fixed 2026-09-04.** `NewUIMainFrameWindow.cpp`'s background-fill-behind-legacy-icons and
  selected-skill-slot-highlight logic (4 call sites total, not 2 — see the "Pilots to revisit"
  table below) now branch on `UI::RmlBridge::ThemeProvidesOwnIconChrome()`, a declared capability
  read from an optional `themes/<name>/theme.ini` (`[Capabilities] ProvidesOwnIconChrome=1`) —
  missing file/key defaults to false, so only `modern` (the one theme with non-default behavior)
  needed a new file. The underlying paint-order constraint itself is unchanged (see the next
  entry) — this fix is the capability flag the entry below already anticipated, not a removal of
  the conditional.
- **RmlUi rendering strictly last in the frame — an unexamined integration choice, not a proven
  requirement.** `RmlUiRuntime::Render()` fires from exactly one fixed pre-submit callback, always
  after every legacy 2D/3D draw call for the frame — which is *why* `NewUIMainFrameWindow.cpp`'s
  icon-chrome conditional (previous entry) exists at all (an RmlUi-drawn element would always
  paint over legacy content that needs to render on top of it). Nobody has investigated whether
  interleaving is actually possible for this engine's RmlUi integration (multiple contexts
  triggered at different frame points, or a callback hook legacy content renders through at the
  right point in RmlUi's own z-order) — if it is, it would remove the need for that conditional
  (and its now-fixed capability flag) entirely, rather than living with a paint-order workaround
  permanently. Bigger and riskier than the §30 fix itself (core render-loop timing, not one call
  site) —
  deserves its own dedicated design pass before assuming either direction. Not investigated.
- **No Custom/Test theme yet.** §25/§28 want a Custom/Test theme that looks substantially
  different from Legacy, specifically to surface accidental component/presentation coupling.
  **Legacy and Modern exist to validate that the architecture supports arbitrary themes, not as
  the intended ceiling** — a third theme is genuinely wanted eventually; it's simply not scheduled
  ahead of other work today. This is an ordinary sequencing decision, not an open policy conflict
  — check with the user on *timing*, not on whether it should happen at all.
- **No systematic resolution × UI-scale × theme × drag-state validation matrix** (§25).
  Verification so far has been ad hoc per window. No test plan artifact exists that a future
  session could run through mechanically.
- **The existing drag system's interaction with theme-default-layout + UI-scale (§10–11) has not
  been explicitly audited** — does a dragged position survive a UI-scale change sensibly? A theme
  change? Not examined.
- **`UI::RmlBridge::MakeDraggable()` (`RmlDraggable.h/.cpp`) has zero live call sites and still has
  one open §10-11 gap.** ~~It writes the dragged position as an absolute `px` inline style, which
  never scales with `UIScalePercent`~~ — **fixed 2026-09-04**: it now divides by the panel's own
  `Context::GetDensityIndependentPixelRatio()` and writes `dp`, matching every other dp-authored
  sibling. **Still open**: nothing persists the result anywhere — no `GameConfig` position-storage
  mechanism exists yet for any RmlUi panel. Not a live bug (nothing depends on it today), and
  deliberately not built speculatively (no real caller yet to confirm the right shape). See the
  header comment on `RmlDraggable.h` for the full note; resolve before, not after, the first real
  caller.
- ~~No reusable-component catalog exists as such~~ (§20) — **addressed 2026-09-04**:
  [`component-catalog.md`](component-catalog.md) inventories what already functions as a reusable
  primitive (`RmlModelBinder<T>`, `UI::RmlBridge` helpers, the anchor/center/stretch RCSS classes,
  Button/Checkbox's real shared contract) versus what genuinely doesn't exist yet (`ItemSlot`,
  `ProgressBar`, a unified `Tooltip`, `Dialog`, etc.) — a documentation deliverable, not new code;
  the underlying gaps it records are still open, just now named and tracked in one place instead
  of undiscoverable.
- **No rollout/phasing plan sequences the remaining ~88 `UI/NewUI` windows against the full
  checklist above.** Work has been pilot-by-pilot, each individually verified. This document is a
  first step toward tracking that, not a substitute for an actual sequenced plan if one is
  wanted.
- **`MuPlatform::Initialize()`/`CreatePlatformWindow()`/`GetWindow()`/`Shutdown()`/
  `SetFullscreen()`/`SetMouseGrab()`/`GetDisplaySize()`, and the `IPlatformWindow`/`SDLWindow`
  classes they own, show zero external callers in a first-pass grep** — found 2026-09-04 while
  deleting the confirmed-dead `SDLEventLoop`/`PollEvents()` path this file's own `MuPlatform.cpp`
  used to also contain (see the Findings entry above). Surprising: `MuPlatform::Initialize()` is
  the only place `SDL_Init(SDL_INIT_VIDEO)` is called anywhere in `src/source`, yet the game
  clearly runs and creates its window (`SDL_CreateWindow` directly in `Winmain.cpp`). Needs its own
  careful audit (does this SDL3 version actually require an explicit `SDL_Init` before
  `SDL_CreateWindow`, what do non-Windows paths do) before touching — deleting the wrong thing here
  could break window creation outright, unlike the narrowly-scoped `SDLEventLoop` deletion. **Not
  investigated yet — deliberately parked**, not bundled into that cleanup.

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
| `CNewUIMainFrameWindow` (`RenderLeftFrame()`/`RenderCenterFrame()`, `NewUIMainFrameWindow.cpp`) | Modern theme's flat background fill behind the still-legacy 3D-rendered potion/skill icons is drawn in C++ (`RenderColorQuadARGB`), not RCSS. **The paint-order reason is legitimate** — RmlUi always composites its whole document as the frame's last pass, after those icons already rendered, so an RmlUi-drawn fill in that screen region would always paint *over* them, not behind — the same reason `#item_slots`/`#skill_slots` are border-only in RmlUi, never filled. ~~Gated on the literal string `GetActiveThemeName() == "modern"` (§30 violation — a third theme wanting the same treatment silently wouldn't get it)~~ — **fixed 2026-09-04**: now gated on `UI::RmlBridge::ThemeProvidesOwnIconChrome()`, a declared theme capability (`themes/modern/theme.ini`). The border lines that used to live alongside this same fill were **not** similarly exempt — moved to RmlUi (`#gauge_frame`), since a thin outline has no such paint-order constraint. The skill-hotkey-number subscript and the gauge current/max text are both fully retired from C++ (`GetHotKeySlotNumber()`/`hp_current_text` etc. — pure theme-agnostic data, each theme's own markup decides what to show). | Icon/box-frame art for the skill grid/pet row stayed legacy 2D too (`RenderSkillIcon()`'s atlas lookup is too irregular — mixed 8/12-column addressing, a separate master-level atlas — to port blind without a way to visually verify against the real decoded `.OZJ` textures). Retires once a separately-scoped icon-atlas port lands **or** the render-ordering investigation ("Known gaps") finds interleaving is possible, whichever comes first. |
| `CNewUISkillList::RenderCurrentSkillAndHotSkillList()` (`NewUIMainFrameWindow.cpp`, still fully legacy) | The selected-skill-slot highlight. **Same paint-order reasoning as the row above** (modern's RmlUi `.selected` outline always paints on top of it since RmlUi composites last, so drawing the legacy `IMAGE_SKILLBOX_USE` sprite unconditionally would double up the highlight for modern; legacy genuinely wants the real sprite, which has no RmlUi asset equivalent ported yet) — legitimate reason. ~~Gated on `GetActiveThemeName() != "modern"` (§30 violation, same as the row above)~~ — **fixed 2026-09-04**, same `ThemeProvidesOwnIconChrome()` capability as the row above (inverted). Same exception class as the row above, not a separate issue; the expanded grid's own box-frame draw (`Render()`, a distinct call site) got the identical treatment for the same reason — modern's grid cells use a plain CSS border (`.skill-cell`, `main_frame.rcss`) instead of `IMAGE_SKILLBOX`/`IMAGE_SKILLBOX_USE`; legacy keeps the real sprite for both call sites. | Same icon-atlas-port (or render-ordering) follow-up as the row above retires this. |
| `main_frame.rml` (both themes) | Two independently-maintained RML files, not the one-shared-RML-per-window pattern every other migrated window uses — `theming-and-modding.md`'s "Forking a theme's RML" section documents why and the criteria for when this is legitimate. The two files' shared ids/classes/bindings require hand-sync, called out in each file's own header comment — see "Known gaps" for the drift-check tooling this still doesn't have. | Either a cleaner RCSS-only structural fix is found and one file retires, or this is accepted long-term and the same criteria get applied consistently if another window ever needs it — not before a second real case shows up. |
| `CNewUIMainFrameWindow` (`main_frame.rcss`, both themes — HP/MP/AG/SD/EXP bars + 5 corner buttons) | `UI::Scaling::BottomHudScale()`/`CappedUniformScale()` (`UITransform.cpp`) fold `GameConfig::GetUIScalePercent()` in as a post-clamp multiplier, applied in the shared function itself so every caller codebase-wide (RmlUi bars/buttons/exp via `bars_scale`, the still-legacy chrome render, 3D potion-icon placement, and potion/skill click hit-testing) moves together automatically. Also folds `UI::Scaling::GetWindowContentScale()` (OS display-scale/pixel-density factor) into RmlUi's own `dp` ratio (`RmlUiRuntime.cpp`'s `ApplyUIScale()`) — **not yet verified on real mismatched-density hardware** (see that function's own comment and `layout-and-scaling.md`). `main_frame.rcss` still deliberately uses `px`, not `dp`, throughout, tracking `bars_scale` exactly instead of being scaled a second time. | The `UIScalePercent` half needs verifying by actually using a potion/skill at more than one `UIScalePercent` value *and* resolution, not just a visual check. The `WindowContentScale`-into-`dp` half needs a real scaled display or a debug `SetWindowContentScale()` override to confirm it doesn't double-scale against whatever `RenderInterface_SDL_GPU`'s viewport already does with `Rml::Context`'s window-coordinate-sized canvas. Phase 3 (item hotkeys → real RmlUi) landing, plus a follow-up icon-atlas port for the Phase 2 skill grid/pet row's still-legacy icon art (see the two rows above), still eventually retires `BottomHudScale` from this window entirely in favor of the branch's normal fixed-`dp`/`UIScalePercent` policy. |

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

## Tracked deferral: `CNewUIMainFrameWindow`'s own class rename

A distinct deferral from the one above — different reasoning, don't conflate the two.

`CMuHelperBar`/`CBuffStrip` were each renamed at port time (class name and every referencing
`INTERFACE_*`/`CNewUISystem` member/accessor/macro), per the checklist above and
[`newui-tier-adapter.md`](newui-tier-adapter.md)'s Naming section. `CNewUIMainFrameWindow` (Phase 1
of its own 3-phase pilot, `main_frame.rml`/`.rcss`) was **not** renamed when ported.

The reason it stays deferred rather than fixed immediately: `NewUIMainFrameWindow.cpp/.h` welds
three classes together — `CNewUIMainFrameWindow` (ported, Phase 1), `CNewUISkillList` (still fully
legacy, Phase 2), `CNewUIItemHotKey` (still fully legacy, Phase 3). Renaming just
`CNewUIMainFrameWindow` now would leave the file's other two, still-legacy-named residents
mismatched against it for however long Phase 2/3 take — a complication `CMuHelperBar`/`CBuffStrip`
never had, since each of those ports covered its entire file in one pass. Plan: rename all three
classes (and the file itself, `NewUIMainFrameWindow.cpp/.h` → whatever the merged concept should
be called) together, in one pass, once Phase 3 lands — not `CNewUIMainFrameWindow` alone now.
Revisit when Phase 3 (item hotkeys) actually lands; if Phase 2 lands first and Phase 3 stalls for
a long while afterward, reassess whether waiting for Phase 3 is still the right call rather than
renaming the two already-ported classes and leaving `CNewUIItemHotKey` alone.

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
