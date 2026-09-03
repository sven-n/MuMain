# Status Against the Architecture Principles

Living document — update this, not `architecture-principles.md`, when status changes. See
[`architecture-principles.md`](architecture-principles.md) first if you haven't read it; section
numbers below refer to it. [`legacy-theme-modernization.md`](legacy-theme-modernization.md)
(2026-09-02) amends it on one specific question — when legacy-theme C++ behavior should move into
RML/RCSS versus genuinely stay in C++ — worth reading before auditing any legacy-theme code
against this document.

## What's migrated

- **Login/character-select scene (`CWin` tier)** — fully migrated, no remaining
  legacy-`CWin`-rendered chrome: `CLoginWin`, `CLoginMainWin`, `CSysMenuWin`,
  `RememberPasswordPrompt`, `CCharSelMainWin`, `CCharMakeWin`, `CCharInfoBalloonMng`, `CMsgWin`.
  `COptionWin` ported but deliberately not wired up (confirmed unreachable in live play — see
  `README.md`'s Coexistence patterns). Built and verified against a real server, both themes.
- **In-game HUD (`CNewUIObj` tier)** — `CMuHelperBar` (map/position readout + MU Helper bot
  control bar) and `CBuffStrip` (active-buff icon strip, the first `data-for`/dynamic-array pilot
  at this tier) are fully done. The third pilot, `CNewUIMainFrameWindow`'s 3-phase HUD-frame port,
  is **2 of 3 phases done**: Phase 1 (HP/MP/AG/SD/EXP bars + 5 corner buttons) and Phase 2
  (`CNewUISkillList` — compact hotkey row click/hover/cooldown, expanded skill grid, pet-command
  row, and skill tooltips for both themes, replacing the old hand-rolled `EVENT_STATE` hover/click
  machine entirely). **Icon/box-frame art for the skill grid and pet row stays legacy 2D**, a
  deliberate Phase 2 scope cut — see the pilots-to-revisit table below. Phase 3 (`CNewUIItemHotKey`
  — potion slots, 3D-camera-composited icons, no RmlUi pattern proven for that yet) is not started.
  All landed pilots built and verified against a real server, both themes. The rest of this tier —
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
5. RmlUi-facing asset naming, and now the C++ class name itself, reflects what the component
   actually is, not the legacy tier it came from (§12) — e.g. `mu_helper_bar`/`CMuHelperBar`, not
   `hero_position_info`/`CNewUIHeroPositionInfo`. **Renamed at port time, not deferred** — the one
   exception found so far is a legacy file that welds multiple classes together where only some
   are ported in the current pass (`CNewUIMainFrameWindow`'s own file — see "Tracked deferral:
   `CNewUIMainFrameWindow`'s own class rename" below); don't treat that as a general excuse to
   defer a rename otherwise.
6. Both themes updated in the same pass, never one left behind (own standing instruction,
   independent of the principles doc). A rendering technique (e.g. an icon atlas) is verified to
   actually work at runtime before being trusted — see "Findings" below for a case where it
   didn't.
7. Uses reusable components/primitives where they exist; doesn't invent a new one-off mechanism
   when an existing pattern already covers the need — though see "Known gaps," several of the
   principles' presumed primitives don't exist yet on this branch.

## Findings worth knowing before the next port

These are empirical findings from direct runtime testing, not policy — kept here rather than in
`architecture-principles.md` because they're facts about *this specific codebase/engine build*,
not general guidance. Most are RmlUi-build-specific gotchas (this vendored build's own quirks);
a few later entries (the `CWin`/`CUIMng` click-activation ones, 2026-09-03/04) are instead about
the *legacy* C++ architecture surrounding RmlUi, not RmlUi itself — kept in this section anyway
since they're the same kind of "hard-won, empirically-confirmed fact worth not rediscovering,"
just from the other side of the RmlUi/legacy boundary. Tier-specific findings (the `CNewUIObj`-tier
ones) live in `newui-tier-adapter.md`'s "Proven by CBuffStrip" section in full detail; summarized
here for visibility:

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
  are only ever *called* during `MAIN_SCENE` — that used to be a complete visibility gate on its
  own; it stops being one once a window's visuals move to a persistent RmlUi document, which
  renders every frame regardless of scene. See `newui-tier-adapter.md`'s third `MAIN_SCENE`
  prerequisite.
- **RmlUi's own default `display` value is `inline`, not `block`, for every element including
  `<div>`** (confirmed against the vendored source, `StyleSheetSpecification.cpp` — there's no
  browser-style user-agent stylesheet giving `<div>` a block default the way HTML does). Every
  `position:absolute` element gets a block box regardless (a side effect of being
  out-of-flow), which is why this went unnoticed through two pilots and Phase 1 — every div so far
  was absolutely positioned. The first normal in-flow stacked content (Phase 2's skill-tooltip
  lines, one `<div>` per line via `data-for`) rendered as one run-on inline paragraph instead of
  separate lines until `display: block` was added explicitly. Give any non-absolutely-positioned
  element `display: block` (or whatever `display` it actually needs) explicitly — never assume a
  `<div>` gets one for free.
- **`linear-gradient`/`radial-gradient`/`conic-gradient` all route through
  `RenderInterface::CompileShader()`, not a shader-free vertex-colored mesh — fixed 2026-09-02,
  corrects an earlier wrong belief.** An earlier pass of this document claimed `linear-gradient`
  "genuinely works, built as an actual vertex-colored mesh, no shader needed" and that only
  `radial-gradient` needed `CompileShader`. Re-reading RmlUi's own Core decorator source
  (`DecoratorGradient.cpp`) showed that's wrong: `DecoratorLinearGradient::GenerateElementData`
  calls `RenderManager::CompileShader("linear-gradient", ...)` exactly like radial and conic do, and
  bails out (renders nothing) if it fails — the shader-free mesh path only exists for the older,
  deprecated `horizontal-gradient`/`vertical-gradient` decorator, which nothing in this project's
  RCSS uses. Since this project's `RmlUiRenderInterface` (actually `RenderInterface_SDL_GPU`, the
  vendored SDL_GPU backend it inherits from) never overrode `CompileShader` before this fix, every
  `linear-gradient`/`radial-gradient`/`conic-gradient` decorator across the whole modern theme was
  silently failing — confirmed empirically with a before/after screenshot of the login dialog on an
  otherwise-identical build: elements that also set a plain `background-color` (the HUD's
  `.gauge-fill-*` rules) fell back to that flat color and looked "close enough" to go unnoticed;
  elements with only a gradient decorator and no background/border (login's
  `.header-line-left/right`, `.separator`, `.header-glow`) were rendering fully invisible.
  **Now fixed**: `RenderInterface_SDL_GPU` (`src/ThirdParty/RmlUi/Backends/`) implements
  `CompileShader`/`RenderShader`/`ReleaseShader` for the gradient family, porting the upstream GL3
  reference backend's shader math (`RmlUi_Renderer_GL3.cpp`) to a new HLSL fragment shader
  (`RmlUi_SDL_GPU/shader_frag_gradient.frag`) baked into `ShadersCompiledSPV.h` alongside the
  existing color/texture shaders — reusing the existing vertex shader unchanged, and this project's
  own `glslangValidator`/`spirv-cross`/`dxc` toolchain (already vendored for `MU_ENABLE_SHADER_COMPILATION`)
  instead of the external `SDL_shadercross` tool `compile_shaders.py` normally expects. Verified
  both `linear-gradient` and `radial-gradient` render correctly (real elliptical falloff, not just a
  flat fill) against a real build. **`box-shadow`/`blur`/`backdrop-filter` are still unimplemented
  and explicitly out of scope for this fix** — those route through a completely different code path
  (`PushLayer`/`CompositeLayers`/`CompileFilter`/`RenderFilter`, RmlUi's layer/filter/compositing
  subsystem for rendering an element off-screen and blurring it in multiple passes), none of which
  `RenderInterface_SDL_GPU` implements — a materially bigger task, not bundled into this one. Same
  standing lesson as the `box-shadow` finding below: a property having docs/a working parser doesn't
  mean the render interface actually implements it — check for a `CompileShader`/render-interface
  override, or test the specific decorator in isolation, rather than trusting a casual screenshot.
- **`box-shadow` (and `filter`/`backdrop-filter`) parse but don't render on this engine — confirmed
  2026-09-02.** `PropertyId::BoxShadow` has a working RCSS parser, which reads as "supported" if you
  only check property registration (`StyleSheetSpecification.cpp`) — but `RmlUiRenderInterface`
  leaves layer/filter compositing unimplemented, so a blurred `box-shadow` paints as a solid opaque
  block instead of a blur. Already known and documented pre-2026-09-02 in `login.rcss`'s own
  `#panel` comment; missed before `modern-theme-visual-direction.md`'s first draft called
  `box-shadow` "good news," which then broke `base.rcss`'s `.checkbox-box`/`.btn` at runtime
  (solid white fills, no visible border, checkboxes collapsed to hairlines) — reverted. Don't
  parser-check a rendering capability; check the render interface, or just grep for prior art
  first (this exact caveat already existed in the codebase and was missed). **Caveat on this
  finding**: it was diagnosed while `base.rcss` also had the unrelated nested-comment parse bug
  below active in the same file at the same time, so the observed symptom was confounded and
  hasn't been re-verified in isolation on a clean file — treat as likely still true (the
  independent, pre-existing `login.rcss` comment is real corroboration) rather than fully settled.
- **RCSS comments don't nest, and a broken one in a shared file silently corrupts every document
  that links it.** `/* ... "/* example */" ... */` closes at the *first* `*/`, not the intended
  one — everything between that premature close and the next real `*/` gets parsed as garbage
  CSS. A comment added to `base.rcss`'s file header (2026-09-02, describing its own
  `/* token-name */` annotation convention by literally writing that syntax inside another
  comment) did exactly this, and since `base.rcss` is linked by nearly every modern-theme window,
  it broke `mu_helper_bar`, `buff_strip`, `msg_win`'s centering, and (very likely) `main_frame`'s
  own `#top_right_row` positioning all at once — a wide, confusing blast radius from one file that
  took most of a session to trace back to its actual source, because the symptom (several
  seemingly-unrelated windows losing interactivity/positioning/visibility) didn't look like a
  syntax error. If a shared file's change is followed by multiple, seemingly-unrelated windows
  breaking at once, suspect the shared file's own syntax before anything else. Never write a
  literal `/* ... */` sequence inside prose that's itself inside a comment — describe the
  convention without the delimiters instead.
- **A confusingly-named legacy method can silently bind to the wrong RmlUi field.**
  `CNewUISkillList::IsSkillListUp()` (pre-existing, predates RmlUi) reports whether the hotkey row
  is scrolled to its "upper" slot set (6-9,0), not whether the expanded skill-list *popup* is open
  — despite what the name suggests. Phase 2 initially bound `main_frame.rml`'s `skill_grid_open` to
  it, which compiled fine and looked plausible in code review, but meant clicking the current-skill
  slot never visibly opened the grid. Fixed with a correctly-named `IsSkillGridOpen()`. Read what a
  legacy getter actually returns, not just what its name implies, before binding it into a model.
- **`CWin`/`CUIMng`'s legacy click-activation system could starve `UpdateWhileActive()` for a
  window's entire clicked-and-held duration — fixed 2026-09-03, predates RmlUi entirely.**
  `CUIMng::Update()` re-enters `SetActiveWin(pWin)` on every frame `IsLBtnDn()` is true
  (`Input.cpp` — level-triggered, true for every frame the button stays physically held, not just
  the press frame), including frames where `pWin` is already the active head. `SetActiveWin()`
  always deactivates "whatever's currently head" first, even when that's the very window being
  re-clicked, then only re-arms activation for the *next* frame via a deferred flag
  (`m_bWinActive`) — so between one frame's redundant deactivation and the next frame's deferred
  reactivation, `m_bActive` was false every frame of a held click, exactly when `CWin::Update()`
  (called later the same frame, `UIMng.cpp`) checks it to decide whether to run
  `UpdateWhileActive()`. Every migrated window's RmlUi click/keyboard consumption lives in
  `UpdateWhileActive()` — including real `CUITextInputBox` keystroke polling — so this was a
  systemic gap affecting any window with a Type-1 or Type-2 companion, not one window's bug;
  surfaced as `CLoginMainWin`'s menu/credit buttons needing repeated clicks before responding.
  Fixed in the click-processing loop (`UIMng.cpp`) by skipping the redundant `SetActiveWin()` call
  when the clicked window is already active and already head, making a held click on an
  already-active window a true no-op instead of an oscillation. Any future window relying on
  `UpdateWhileActive()` for click/keyboard handling inherits this fix automatically; no per-window
  changes needed.
  **Real, but not the full story — see the next entry.** A precise user repro (crediting-clicking
  did nothing, a later unrelated menu-button click opened *credits* instead) proved RmlUi's click
  listeners were firing reliably all along; the actual bug was `UpdateWhileActive()` simply not
  running often enough at all (not just during a held click) to consume the flags before they
  went stale.
- **`UpdateWhileActive()`-gated RmlUi click flags can go stale and fire on the wrong button later
  — fixed 2026-09-03/04 in every window that had it.** The
  `m_bRmlXClicked`-flag-set-by-a-listener/consumed-in-`UpdateWhileActive()` pattern (used by
  `CLoginMainWin`, `CLoginWin`, `CSysMenuWin`, `CCharMakeWin`) defers acting on an RmlUi click to
  a later poll gated behind `CWin::m_bActive` — but that gate can go many frames without opening
  at all (the legacy `CUIMng` activation system is a fundamentally less reliable signal than
  RmlUi's own event), during which multiple flags can accumulate. Since the consumer is an
  `if/else if` checked in a fixed order, whichever flag is stale-but-still-true wins over a
  genuinely fresh one the *next* time it finally runs — observed exactly as "click credit, nothing
  happens; click menu later, credits open instead." Fixed first in `CLoginMainWin`
  (`RmlClickMenu()`/`RmlClickCredit()`), then applied preemptively to the other three windows once
  the mechanism was proven, not waiting for each to individually misbehave: every `RmlClickX()`
  callback across `CLoginWin`, `CSysMenuWin`, and `CCharMakeWin` now acts immediately (calling a
  shared action method also used by the legacy `CButton::IsClick()`/keyboard polling paths that
  remain in each `UpdateWhileActive()`), instead of setting a flag for a later poll. Confirmed safe
  since `RmlUiRuntime::ProcessSdlEvent()` (where these fire) runs from Winmain's SDL event pump,
  always before `CUIMng::Update()` the same frame. `CLoginWin`'s version also re-checks the
  "remember password" prompt's live Pending state in each immediate callback, preserving the same
  guard `UpdateWhileActive()` applies via its `m_bRememberPasswordPromptWasPending` snapshot (see
  `RmlClickOk()`'s own comment for why the live check is equivalent there). No `m_bRmlXClicked`-
  style flags remain anywhere in the codebase.
- **Three parallel, not-always-agreeing input-tracking systems exist, and only half the resulting
  bug class is fixed.** The two findings above fixed *whether an RmlUi click's action fires* — but
  there's a second, still-open half: `CUIMng::IsCursorOnUI()`/`m_bCursorOnUI` (the "is the cursor
  on UI, should the world-click handler underneath be suppressed" decision, consulted every frame)
  still runs entirely through the same legacy `CursorInWin()` hit-testing, with the same
  staleness/list-order risk already found and fixed for click *dispatch*, for every RmlUi-rendered
  window. A world-click could still leak through under an RmlUi panel, or a UI click could still
  fall through to the game world, on the same class of hit-box drift this session spent most of
  its time on. This isn't speculative — it's the same three systems (`CInput`/
  `SEASON3B::CNewKeyInput`'s VK-polling driving `CUIMng`'s activation/hit-test loop;
  `SDLEventLoop.cpp`'s own event-driven `MouseLButton`/`Push`/`Pop` globals for legacy 2D world
  input; RmlUi's own event-driven `Context::ProcessMouseButtonDown/Up`) that already produced four
  confirmed, independently-discovered bugs this session (the `CInput`-vs-`WindowWidth` staleness
  bug, fixed in four files; the `SetActiveWin` held-click oscillation; the `CreateLoginScene()`
  hit-box overlap; the `m_bRmlXClicked` stale-flag bug, fixed in four windows). The real fix is
  making RmlUi's own hit-testing authoritative for anything RmlUi renders, retiring
  `CursorInWin`/`IsCursorOnUI`'s role for those elements entirely rather than continuing to patch
  each new symptom. Proven by repetition in the same way the `CompanionRatio()` extraction and the
  click-flag fix were — the next real candidate for foundational work, not yet scoped or started.

## Known gaps against the principles (honest status, not yet built)

None of these are wrong so far — the principles doc explicitly endorses incremental delivery
(§26–27) — but they're real, currently-unaddressed gaps in the end-state architecture, not yet
even scheduled. Recorded so no future session mistakes "the pilots pass their own verification"
for "the full architecture is in place":

- **No mod/user-override resource-precedence system** (§18–19). Themes today are exactly two
  hardcoded directories (`themes/legacy/`, `themes/modern/`) selected by `GameConfig`'s theme
  name — no "user override on top of a theme" layer, no documented precedence order, no tooling
  for a third party to ship a partial theme that inherits the rest from a base theme. **Not
  speculative** — §18/§19 have called for this since the governing doc was written, and 2026-09-04
  confirmed the user wants arbitrary future themes to actually be supportable, not just legacy and
  modern (see the corrected Custom/Test entry below). Still correctly *sequenced* behind other
  work (nobody's shipping a mod today), but it shouldn't be read as "maybe never" — it's a stated
  requirement waiting on priority, not an open question about whether to build it at all.
- **No design-token/shared-variable layer, `modern` theme only, now addressed (2026-09-02)**.
  [`modern-theme-visual-direction.md`](modern-theme-visual-direction.md) defines a palette/border
  token table for `modern` and confirms this vendored RmlUi has no `var()`/custom-property
  mechanism (grepped `StyleSheetSpecification.cpp`'s property table) — so tokens are a
  documented-value convention, not a language feature. Every modern-theme `.rcss` file is
  retrofitted to it (borders, resting-control backgrounds, panel/label text — see that doc's
  Rollout section for the deliberately-untouched categories: HUD-over-3D-world text, gameplay-status
  colors, skill-tooltip line-color semantics). No shadow/glow tokens exist — `box-shadow` parses but
  doesn't render on this engine, see the Findings entry below. `font-family` literals are still
  repeated per file (explicitly deferred — see that doc's Typography section). `legacy` has no token
  layer and isn't in scope for one (theme-specific by design). **2026-09-04: confirmed as a real,
  not speculative, want** — the "documented convention, not a mechanism" gap is real: changing an
  accent color still means grep-and-replace across every file tagged with that token's comment,
  not a one-place edit, since this RmlUi build has no `var()`/custom-property support at all. A
  real fix is buildable without engine changes: a small token-substitution pass (RCSS authored
  with placeholder tokens, resolved against a per-theme token file before the text reaches RmlUi —
  the same kind of text transform `LoadThemedDocument()` already does for its synthetic per-theme
  URL). Not built. Moderate scope, not attempted yet.
- **Three "shared" RML files have theme-specific class names baked into them —
  `architecture-principles.md` §15 violation, found 2026-09-04.** `login.rml`, `msg_win.rml`, and
  `remember_password_prompt.rml` all have `modern`-specific classes (`modern-frame`,
  `modern-frame-accent`, `modern-panel`, etc.) hardcoded directly into the file every theme is
  supposed to share equally — e.g. `login.rml`'s `<div id="panel" class="modern-frame
  modern-frame-crimson" ...>`. `legacy`'s RCSS has to either style classes named for a different
  theme or leave them inert. See `theming-and-modding.md`'s corrected Core Principle section for
  the fix (`legacy` becomes the canonical file, `modern` forks its own copy) — not done, this is
  a real migration across three files plus their RCSS, not a docs-only fix.
- **No drift-check tooling for a forked theme's RML.** Whichever theme's RML a window loads, C++
  still expects the exact same ids/`data-model` bindings/event-callback names to exist — nothing
  verifies a forked copy actually satisfies that contract, and a missing/renamed id fails
  completely silently (a dead button, no error, no log line). Needed once theme-forking becomes
  more common than `main_frame`'s one existing case. A script sibling to
  `check_rml_rcss_syntax.py`, diffing referenced ids/bindings against every theme's copy, would
  close this. Not built.
- **Two C++ call sites branch on theme *name*, violating the newly-added §30 — found 2026-09-04,
  not yet fixed.** `NewUIMainFrameWindow.cpp`'s background-fill-behind-legacy-icons and
  selected-skill-slot-highlight logic are both gated on `GetActiveThemeName() == "modern"` (one)
  / `!= "modern"` (the other) — previously recorded in this file as a deliberately-kept, justified
  exception (see the "Pilots to revisit" table below), which undersold the problem: the
  *reason* for some conditional behavior (see the render-ordering entry directly below) is
  legitimate, but keying it off the theme's **name** specifically means a third theme wanting the
  same treatment silently doesn't get it. The fix is a declared theme capability/property, not
  necessarily removing the conditional — not done.
- **RmlUi rendering strictly last in the frame — an unexamined integration choice, not a proven
  requirement, found 2026-09-04.** `RmlUiRuntime::Render()` fires from exactly one fixed
  pre-submit callback, always after every legacy 2D/3D draw call for the frame — which is *why*
  the two §30 violations above exist (an RmlUi-drawn element would always paint over legacy
  content that needs to render on top of it). Nobody has investigated whether interleaving is
  actually possible for this engine's RmlUi integration (multiple contexts triggered at different
  frame points, or a callback hook legacy content renders through at the right point in RmlUi's
  own z-order) — if it is, it would remove the need for the §30 workarounds entirely rather than
  patching them with a capability flag. Bigger and riskier than the §30 fix itself (core
  render-loop timing, not one call site) — deserves its own dedicated design pass before assuming
  either direction. Not investigated.
- **No Custom/Test theme yet — corrected 2026-09-04, was misleadingly recorded as a conflict.**
  §25/§28 have always wanted a Custom/Test theme that looks substantially different from Legacy,
  specifically to surface accidental component/presentation coupling. This was previously recorded
  here as "the standing instruction on this branch has been only Legacy and Modern, no third
  theme, unresolved conflict" — that framing was backwards and is now corrected: **Legacy and
  Modern exist specifically to validate that the architecture supports arbitrary themes, not as
  the intended ceiling.** A third theme is genuinely wanted eventually; it's simply not scheduled
  ahead of other work today. Nothing here is an open policy conflict — it's an ordinary sequencing
  decision. Don't reintroduce the old "no third theme, period" framing in future updates to this
  file; check with the user on *timing*, not on whether it should happen at all.
- **No systematic resolution × UI-scale × theme × drag-state validation matrix** (§25).
  Verification so far has been ad hoc per window (a handful of resolutions checked interactively;
  UI scale exercised incidentally — e.g. discovering the `dp`-vs-`px` bug during `CBuffStrip`
  verification, not through deliberate systematic testing at each of 75/100/125/150%). No test
  plan artifact exists that a future session could run through mechanically.
- **The existing drag system's interaction with theme-default-layout + UI-scale (§10–11) has not
  been explicitly audited** — carried over from `feature/rmlui-migration` unchanged, not
  re-examined against these specific concerns (does a dragged position survive a UI-scale change
  sensibly? A theme change?).
- **`UI::RmlBridge::MakeDraggable()` (`RmlDraggable.h/.cpp`) has zero live call sites and isn't yet
  §10-11-compliant even in prototype form** (2026-09-02 audit finding). It writes the dragged
  position as an absolute `px` inline style, which never scales with `UIScalePercent`
  (`layout-and-scaling.md`'s `dp`-vs-`px` rule), and nothing persists the result anywhere — no
  `GameConfig` position-storage mechanism exists yet for any RmlUi panel. Not a live bug (nothing
  depends on it today), but the next window that reaches for it will silently inherit both gaps.
  See the header comment on `RmlDraggable.h` for the full note; resolve before, not after, the
  first real caller.
- **No reusable-component catalog exists as such** (§20) — `RmlModelBinder<T>`,
  `UI::RmlBridge` helpers, and the anchor/center/stretch RCSS classes are the closest things to
  reusable primitives today, all C++/CSS mechanisms rather than named, documented UI components
  (`Window`, `Panel`, `ItemSlot`, etc.) a future window's markup could just reference.
- **No rollout/phasing plan sequences the remaining ~88 `UI/NewUI` windows against the full
  checklist above.** Work has been pilot-by-pilot, each individually verified. This document is a
  first step toward tracking that, not a substitute for an actual sequenced plan if one is
  wanted.

## Pilots to revisit when the relevant phase arrives

Every already-shipped window that doesn't fully match the principles doc is **left as-is now,
not rewritten to chase each gap in isolation** (§26 — incremental, don't rewrite wholesale) —
but each specific deviation below is tied to whichever future initiative would naturally fix it,
so it gets folded into that pass instead of being forgotten. Check this list whenever starting
one of the trigger initiatives on the right.

**Correction, 2026-09-04**: the two `GetActiveThemeName() == "modern"`/`!= "modern"` rows below
were recorded as fully justified, deliberately-kept exceptions. The paint-order *reason* for some
conditional behavior is real and still stands (see the render-ordering gap in "Known gaps"
above) — but keying that conditional on the theme's **name** specifically is itself a separate
problem (§30, added 2026-09-04): a third theme wanting the same treatment wouldn't get it, since
the check only recognizes the literal string `"modern"`. Both rows' "kept deliberately" framing
undersold this — treat them as tracked debt (a capability flag, not a name check), not settled
design, alongside whatever the render-ordering investigation concludes.

| Window(s) | Known deviation | Revisit when... |
|---|---|---|
| `CMuHelperBar`, `CBuffStrip` (`mu_helper_bar.rcss`, `buff_strip.rcss`) | Repeat their own `font-family`/`font-size`/color literals inline instead of referencing shared tokens (§21) | A design-token/shared-variable layer is built — retrofit these two RCSS files to use it as the worked examples, don't leave them as the last two still on literals. |
| `CMuHelperBar`, `CBuffStrip` and every `CWin`-tier window | File location (`UI/NewUI/HUD/` etc.) and base class/tier boundary (`CNewUIObj`/`CWin`) unchanged (§12, "Tracked deferral" below) | A directory/base-class restructuring pass across `UI/NewUI` is undertaken — not before enough windows exist to know the real target shape (this is the existing tracked deferral, not new). |
| All `CWin`-tier windows, `CMuHelperBar`, `CBuffStrip` | No resolution × UI-scale × theme × drag-state validation matrix has been run against any of them (§25) — verification so far has been ad hoc per window | A validation-matrix/test-plan artifact is built — run it retroactively against every already-migrated window, not just new ones going forward. |
| All draggable migrated windows | Existing drag system's interaction with theme-default-layout + UI-scale (§10–11) has never been explicitly audited | The drag/preference-integration audit (itself an unstarted gap, above) happens — check these windows specifically, don't just audit the mechanism in the abstract. |
| `CBuffStrip` | Right-click-to-cancel not reproduced; tooltip is plain-text instead of the original's per-line-colored rich tooltip (both already documented as deliberate scope cuts in `newui-tier-adapter.md`, not silent gaps) | Right-click-distinct-from-left-click is proven generally in a `data-event-click` binding, or the three non-unified tooltip mechanisms (§12) get consolidated — whichever comes first. |
| `CNewUIMainFrameWindow` (`RenderLeftFrame()`/`RenderCenterFrame()`, `NewUIMainFrameWindow.cpp`) | Modern theme's flat background fill behind the still-legacy 3D-rendered potion/skill icons is drawn in C++ (`RenderColorQuadARGB`, gated on `GetActiveThemeName() == "modern"`), not RCSS — audited against `legacy-theme-modernization.md`'s classification. **The paint-order reason is legitimate, the name check isn't (corrected 2026-09-04, §30, "Known gaps")**: RmlUi always composites its whole document as the frame's last pass, after those icons already rendered, so an RmlUi-drawn fill in that screen region would always paint *over* them, not behind — the same reason `#item_slots`/`#skill_slots` are border-only in RmlUi, never filled — but keying that on the literal string `"modern"` means a third theme wanting the same treatment silently wouldn't get it; tracked as debt (a capability flag), not settled design. The border lines that used to live alongside this same fill were **not** similarly exempt — moved to RmlUi (`#gauge_frame`) once audited, since a thin outline has no such paint-order constraint. Likewise the skill-hotkey-number subscript and the gauge current/max text, both fully retired from C++ this pass (`GetHotKeySlotNumber()`/`hp_current_text` etc. — pure theme-agnostic data, each theme's own markup decides what to show). | Phase 2 landed 2026-09-02 but did **not** retire this — icon/box-frame art for the skill grid/pet row stayed legacy 2D too (mid-implementation scope adjustment: `RenderSkillIcon()`'s atlas lookup turned out too irregular — mixed 8/12-column addressing, a separate master-level atlas — to port blind without a way to visually verify against the real decoded `.OZJ` textures). Retires once a separately-scoped icon-atlas port lands (Phase 3 territory or its own follow-up) **or** the render-ordering investigation ("Known gaps") finds interleaving is possible, whichever comes first — not before either. |
| `CNewUISkillList::RenderCurrentSkillAndHotSkillList()` (`NewUIMainFrameWindow.cpp`, still fully legacy) | The selected-skill-slot highlight is gated `GetActiveThemeName() != "modern"` in C++ (audited 2026-09-02). **Same §30 correction as the row above applies here too** — see that call site's own comment for the paint-order reasoning (modern's RmlUi `.selected` outline always paints on top of it since RmlUi composites last, so drawing the legacy `IMAGE_SKILLBOX_USE` sprite unconditionally would double up the highlight for modern; legacy genuinely wants the real sprite, which has no RmlUi asset equivalent ported yet) — legitimate reason, but the name check itself is tracked debt, not settled design. Same exception class as the row above, not a separate issue. | Same icon-atlas-port (or render-ordering) follow-up as the row above retires this — not attempted in Phase 2 itself (see that row). **Extended, not retired, 2026-09-02**: the expanded grid's own box-frame draw (`Render()`, distinct call site from this one) got the identical `GetActiveThemeName() != "modern"` treatment for the same reason (feedback: "the skill list are using the legacy border instead of the programmatic border already used in the skill hotkeys slots") — modern's grid cells now use a plain CSS border (`.skill-cell`, `main_frame.rcss`) instead of `IMAGE_SKILLBOX`/`IMAGE_SKILLBOX_USE`; legacy keeps the real sprite for both call sites. Two call sites now share this exception, not one. |
| `main_frame.rml` (both themes) | Two independently-maintained RML files, not the one-shared-RML-per-window pattern every other migrated window uses — `theming-and-modding.md`'s "Forking a theme's RML" section (renamed 2026-09-04, was "Exception: per-theme RML override") documents why and the criteria for when this is legitimate (2026-09-02, written up after a 2026-09-02 audit found the mechanism had shipped silently, undocumented, back on 2026-09-01/02). The two files' shared ids/classes/bindings require hand-sync, called out in each file's own header comment — see "Known gaps" for the drift-check tooling this still doesn't have. | Either a cleaner RCSS-only structural fix is found and one file retires, or this is accepted long-term and the same criteria get applied consistently if another window ever needs it — not before a second real case shows up. |
| `CNewUIMainFrameWindow` (`main_frame.rcss`, both themes — HP/MP/AG/SD/EXP bars + 5 corner buttons, first of 3 planned phases) | **Fixed 2026-09-03** (was: doesn't respect `UIScalePercent`, §9, §23–24). `UI::Scaling::BottomHudScale()`/`CappedUniformScale()` (`UITransform.cpp`) now fold `GameConfig::GetUIScalePercent()` in as a post-clamp multiplier — `UI::Scaling`'s first-ever `GameConfig` dependency. Applied in the shared function itself (not a one-off multiply on `barsScale`), so every caller codebase-wide (RmlUi bars/buttons/exp via `bars_scale`, the still-legacy chrome render, 3D potion-icon placement, and potion/skill click hit-testing in `Render()`/`Render3D()`/`UseHotKeyItemRButton()`) moves together automatically — no risk of the two-independent-calculators drift bug `CCharSelMainWin` hit (`layout-and-scaling.md`). Same pass also folded the `origin/main` merge's new `UI::Scaling::GetWindowContentScale()` (OS display-scale/pixel-density factor) into RmlUi's own `dp` ratio (`RmlUiRuntime.cpp`'s `ApplyUIScale()`), so migrated RmlUi windows and the legacy-transform-driven HUD respond to real display DPI consistently — that half is **not yet verified on real mismatched-density hardware** (see that function's own comment and `layout-and-scaling.md`). `main_frame.rcss` still deliberately uses `px`, not `dp`, throughout — unchanged by this fix, still tracks `bars_scale` exactly instead of being scaled a second time. | The `UIScalePercent` half needs verifying by actually using a potion/skill at more than one `UIScalePercent` value *and* resolution, not just a visual check, before fully trusting it in play. The `WindowContentScale`-into-`dp` half needs a real scaled display or a debug `SetWindowContentScale()` override to confirm it doesn't double-scale against whatever `RenderInterface_SDL_GPU`'s viewport already does with `Rml::Context`'s window-coordinate-sized canvas. Phase 3 (item hotkeys → real RmlUi) landing, plus a follow-up icon-atlas port for the Phase 2 skill grid/pet row's still-legacy icon art (see the two rows above), still eventually retires `BottomHudScale` from this window entirely in favor of the branch's normal fixed-`dp`/`UIScalePercent` policy — this fix doesn't preempt that, it just makes the interim state consistent. |

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

A third, distinct deferral from the one above — different reasoning, don't conflate the two.

`CMuHelperBar`/`CBuffStrip` were each renamed at port time (class name and every referencing
`INTERFACE_*`/`CNewUISystem` member/accessor/macro), per the checklist above and
[`newui-tier-adapter.md`](newui-tier-adapter.md)'s Naming section. `CNewUIMainFrameWindow` (Phase 1
of its own 3-phase pilot, `main_frame.rml`/`.rcss`) was **not** renamed when ported — a real,
2026-09-02-audit-caught gap against that rule, not an intentional first application of this
deferral at the time.

The reason it's being deferred now rather than fixed immediately: `NewUIMainFrameWindow.cpp/.h`
welds three classes together — `CNewUIMainFrameWindow` (ported, Phase 1), `CNewUISkillList` (still
fully legacy, Phase 2), `CNewUIItemHotKey` (still fully legacy, Phase 3). Renaming just
`CNewUIMainFrameWindow` now would leave the file's other two, still-legacy-named residents mismatched
against it for however long Phase 2/3 take — a complication `CMuHelperBar`/`CBuffStrip` never had,
since each of those ports covered its entire file in one pass. Decided 2026-09-02: rename all three
classes (and the file itself, `NewUIMainFrameWindow.cpp/.h` → whatever the merged concept should be
called) together, in one pass, once Phase 3 lands — not `CNewUIMainFrameWindow` alone now. Revisit
when Phase 3 (item hotkeys) actually lands; if Phase 2 lands first and Phase 3 stalls for a long
while afterward, reassess whether waiting for Phase 3 is still the right call rather than renaming
the two already-ported classes and leaving `CNewUIItemHotKey` alone.

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
