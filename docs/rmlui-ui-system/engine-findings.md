# Engine Findings: RmlUi Build-Specific Gotchas

Split out of `STATUS.md` (2026-09-16) since this content is reference material, not a status
tracker — a different read than "what's migrated." See [`STATUS.md`](STATUS.md) for migration
status against [`architecture-principles.md`](architecture-principles.md), and
[`tracked-deferrals.md`](tracked-deferrals.md) for forward-looking punch-lists.

Empirical facts about *this specific codebase/engine build*, not general policy — kept here rather
than in `architecture-principles.md` for that reason. Most are RmlUi-build-specific engine quirks;
a couple are about the legacy `CWin`/`CSceneUICoordinator` (named `CUIMng` at the time these were
found — see the note near the end of this file) C++ architecture surrounding RmlUi instead.
Tier-specific findings (`mu::ui::window::CObject`-tier) live in `newui-tier-adapter.md`'s own
"Proven by CBuffStrip" section in full detail; summarized here for visibility.

- **This RmlUi build silently fails to inherit `font-family` into most subtrees** — a descendant
  needs its own explicit `font-family` declaration; relying on inheritance from an ancestor
  (`#panel`, `body`, a shared class) renders that element's text invisible with no error. Not
  scoped to `data-model`-bound documents specifically (an earlier, wrong diagnosis) — `login_main.rml`
  has no `data-model` at all and still needs this. Every text-bearing selector across
  `themes/modern/*.rcss` now declares its own `font-family: "token(font-body)"` rather than
  inheriting one.
  **Recurred, 2026-09-19** (`option_window.rcss`'s modern `.option-tab-btn`): rediscovered the
  hard way, at real cost — invisible tab-button text was mis-diagnosed for several rounds as a
  flex-item/decorator/box-shadow interaction (tried decorator directly on a flex item, decorator
  isolated onto a non-flex sibling child, then rebuilding the whole row as `position:absolute`
  with percentage widths to structurally match `.btn` — none of it made any difference), when the
  rule simply never had `font-family` at all. **Check this finding FIRST, before any structural
  theory, whenever a NEW element's text is invisible but the rest of its box (background/border/
  decorator) renders fine** — that pattern (box paints, text doesn't) is this finding's own
  signature, not evidence of a layout/decorator bug. Modern has no document-wide default the way
  legacy's `#panel` rule provides one; every other visible piece of text in a modern document
  already compensates with its own explicit `font-family` (the window title, row labels, selects,
  `.btn` itself) — a new rule that skips it is the one to suspect.
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
  **The "everything except login/msg_win/remember_password_prompt/main_frame" list above is stale**
  — `character_info` and `my_quest_info` have since grown their own `themes/modern/*.rml`
  overrides too (richer forged-dialog markup than their shared fallback: shell-edge/groove/header-
  rail chrome, `modern-icon-btn-px` buttons instead of legacy-style sprite ones), and more windows
  may follow. **Don't infer a document's real rendered markup from the shared top-level `.rml` file
  alone — check for a `themes/<theme>/<name>.rml` override first**, `LoadThemedDocument()` prefers
  it silently. Cost real time once already: a shared-RML class (`info-btn`) was compared against a
  theme's own `.rcss` (which styled a *different* class, `chainfo-btn-*`) and read as a bug, when
  the theme's real per-theme RML override already used the matching classes correctly — the diff
  was checked against the wrong source file.
- **`overflow:hidden` does not clip an absolutely-positioned oversized child in this RmlUi
  build**, even with `clip: always`. Use generated named `@spritesheet` rects instead for sprite
  atlases — see `newui-tier-adapter.md`.
- **`dp` and `px` are not interchangeable in `data-style-*` position bindings.** `dp` is scaled by
  the user's UI-scale setting, `px` never is — a `+'px'` binding drifts out of step with
  `dp`-sized siblings at any UI scale other than 100%.
- **An absolutely-positioned, `display:block`, multi-line (`white-space:pre-line`) box needs an
  explicit `width`** — left to shrink-to-fit, this build's width computation undersizes to the
  longest *word*, not the longest *line*.
- **A persistent RmlUi document needs its own scene-visibility gate.** `mu::ui::window::CSystem` (the
  `mu::ui::window::CObject`-tier manager) is a persistent app-lifetime singleton whose `Update()`/`Render()`
  are only ever *called* during `MAIN_SCENE` — that alone isn't a visibility gate once a window's
  visuals move to a persistent RmlUi document, which renders every frame regardless of scene. See
  `newui-tier-adapter.md`'s third `MAIN_SCENE` prerequisite.
- **`CManager::CompareKeyEventOrder` (`WindowManager.cpp`) sorts `m_vecUI` DESCENDING by
  `GetKeyEventOrder()` — the HIGHEST value runs first, not the lowest** (`return a.GetKeyEventOrder()
  > b.GetKeyEventOrder();`, which is what `std::sort` needs for descending order). Every
  `GetKeyEventOrder()` override comment in this codebase (including ones written earlier in this
  same file's own history) assumed the opposite — "lower number = higher priority = runs first" —
  which is backwards here. `CManager::UpdateKeyEvent()` stops at the first object whose
  `UpdateKeyEvent()` returns `false`, so getting this backwards doesn't just misorder cosmetically:
  a dialog meant to be a top-priority modal that's given a LOW value will instead run dead last,
  letting an ordinary window's own (unguarded) Esc-to-close fire first and swallow the keypress
  before the dialog ever gets a turn. Concretely: `CGenericConfirmDialog`/`CGenericMenuDialog` were
  first set to `0.0f` intending "runs before everything," which actually put them below
  `CMyInventory`'s un-overridden `CObject` default (`3.0f`) — so with the Chaos Mix Menu dialog,
  Mix Inventory, and the regular Inventory all open, Esc closed the Inventory instead of the modal
  dialog. Fixed by giving both dialogs `100.0f` (comfortably above every other override in the
  codebase — the highest existing tier before this was `10.0f`, shared by `CWindowMenu`/
  `CMessageBoxMng`/`COptionWindow`/etc.), so they always run first under the manager's real
  descending sort regardless of what else is open. **Do not "fix" the comparator itself** — it's
  long-standing and ~15 other classes are already calibrated against its actual (descending)
  behavior; treat "higher `GetKeyEventOrder()` = runs first" as the real contract when adding or
  auditing any class's value, and don't trust a same-file comment that says otherwise (several
  didn't, until this was found). `HotKey.cpp`'s Esc handler additionally still guards explicitly
  against `g_pGenericConfirmDialog`/`g_pGenericMenuDialog` being visible before auto-opening the
  system menu (belt-and-suspenders; with the ordering now correct this guard should never actually
  trigger, but costs nothing to keep) — any *third* dialog primitive built the same way should get
  the same high `GetKeyEventOrder()`, not a copy of that guard.
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
- **`decorator: image(...)` does not stretch a sprite rect to fill a box sized differently from
  the sprite's own native dimensions in this RmlUi build — but a `ninepatch(...)` decorator using
  the same sprite sheet does, correctly, in both directions.** `legacy-btn-idle`/`-hover`/`-active`
  (`base.rcss`'s `@spritesheet`) are fixed 108x30 rects, with a matching `-inner` "safe to stretch"
  rect also declared for each. `server_select.rcss`'s `.server-row`/`.group-btn` first tried plain
  `image(...)` stretched to 186x30 (the first place in this codebase asking for a non-native size)
  and most of the box stayed unpainted instead of the sprite scaling to fill it. The actual fix
  (not a workaround) was switching to `ninepatch(legacy-btn-idle, legacy-btn-idle-inner)` — this
  9-slice-scales the real sprite correctly at the new size, and is what `.server-row`/`.group-btn`
  ship with today (a `background-color`/`border` flat-fill was tried as an intermediate step but is
  no longer used there). Don't assume plain `image()` behaves like a CSS `background-size` image at
  an arbitrary box size — either use `ninepatch` with a declared inner rect, or stick to the
  sprite's own native size.
  **Correction, 2026-09-15**: this entry previously claimed "every existing user of them (`.btn`,
  `.group-btn`) is also exactly 108x30" — false. `generic_menu_dialog.rcss`'s legacy `.gmd-btn`
  already overrides `width` to 128dp (default) or 64dp (`compact`) while inheriting `.btn`'s
  108x30 `legacy-btn-idle` decorator unchanged, in all 12 pre-existing dialogs built on
  `CGenericMenuDialog` — the exact same width-only-mismatch shape `.server-row` had before its
  ninepatch fix, just at smaller deltas (+18.5%/−40.7% vs. `.server-row`'s +72%). Whether this is
  visually manifesting as unpainted chrome in those 12 already-signed-off dialogs was not confirmed
  either way (no screenshot taken) — flagging so a future visual QA pass on this primitive checks
  it rather than assuming the existing 128dp/64dp buttons are proof the mismatch is harmless at
  small deltas. The new `.gmd-btn.cols-2` class (added the same day) initially sidestepped the
  question by not inheriting the sprite decorator at all (flat fill/border) — since superseded:
  legacy's `.cols-2` now uses the same `ninepatch(legacy-btn-idle, legacy-btn-idle-inner)` fix as
  `.server-row`, so it (unlike the other 12 dialogs' plain `.gmd-btn`) is confirmed not to have this
  problem. Modern's `.cols-2` keeps the flat fill/border approach, correctly — modern's `.btn` is a
  procedural gradient, not an image sprite, so there's no ninepatch to apply there.
- **A bordered/backgrounded element that also has child elements can render an incomplete,
  non-closed rectangle in this RmlUi build** (`server_select.rcss`'s `.server-row`, three separate
  attempts: flex children with a `<span>`, absolutely-positioned children, then plain block
  children -- all three produced a border/background that didn't paint as one closed box). A
  bordered leaf with **zero** child elements (text content only) renders correctly every time this
  was tried. The one confirmed-safe way to combine a bordered/painted box with more content next to
  it: put the border/background on a childless leaf, and make anything else that needs to be
  visually "with" it a *sibling* under a separate, unstyled (no border/background of its own)
  wrapper element instead of a child of the bordered element -- `server_select.rcss`'s current
  `.server-row` (unstyled wrapper) > `.server-name` (bordered, zero children) +
  `.server-gauge-slot` > `.server-gauge-fill` (unbordered parent, one child, the same shape
  `main_frame.rcss`'s HP/MP `.gauge-slot`/`.gauge-fill` already used successfully) is the worked
  example. Don't add a second child element to an already-bordered leaf without testing it visually
  first.
- **A plain block element's auto height doesn't reliably sum multiple in-flow block children's
  boxes in this RmlUi build**, even with no border/background of its own (so this is distinct from
  the bordered-element finding above). `server_select.rcss`'s `.server-row` wrapper (no border, two
  block children: a fixed-height name pill, then a gauge bar below it) reported too short a height
  left on `auto` -- the next row started before the current row's gauge bar had room, visually
  clipping/overlapping it. Fixed by giving the wrapper an explicit height (sum of its children's own
  heights/margins) instead of relying on auto to compute it.
- **`box-shadow` now genuinely renders on this engine, including blur — this entry is corrected
  from an earlier, now-stale finding that it parsed but didn't render.** As of the vendored SDL_GPU
  renderer's PR-989 integration, `RenderManager::PushLayer`/`CompositeLayers`/`CompileFilter`/
  `RenderBlur` are all genuinely implemented (confirmed by reading `RmlUi_Renderer_SDL_GPU.cpp`
  directly, not assumed), and `GeometryBoxShadow.cpp` renders through them correctly — inset and
  outset, real blur, comma-separated multiple shadows. In active, verified use throughout
  `base.rcss`/`login.rcss`/`sys_menu.rcss`/`server_select.rcss` (window drop-shadows, header/plate
  cast-shadows + inset highlights, recessed-groove darkness, button press feedback). One conversion
  gotcha: this engine's `rgba()` parses alpha via a 0-255 integer (`atoi`), NOT a 0-1 CSS float —
  `rgba(0,0,0,.62)` parses as alpha 0 (invisible) if copied verbatim from a browser reference; every
  box-shadow color needs pre-converting to the 0-255 form. `filter`/`backdrop-filter` remain
  unimplemented (`filter: brightness()`/`contrast()` are the one exception, confirmed working
  separately) — don't assume those follow box-shadow's fix.
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
  `CSkillList::IsSkillListUp()` (pre-existing, predates RmlUi) reports whether the hotkey row
  is scrolled to its "upper" slot set (6-9,0), not whether the expanded skill-list *popup* is
  open — despite what the name suggests. `main_frame.rml`'s `skill_grid_open` binds to a
  correctly-named `IsSkillGridOpen()` instead. Read what a legacy getter actually returns, not
  just what its name implies, before binding it into a model.
- **The legacy `CWin`/`CUIMng` click-activation system was a fundamentally less reliable signal
  than RmlUi's own click events, and this caused real, confirmed bugs — both now closed out, the
  underlying system itself later deleted entirely.** `CWin::Update()` gates `UpdateWhileActive()`
  (where every migrated window's RmlUi click/keyboard consumption lives, including real
  `CUITextInputBox` keystroke polling) behind `CWin::m_bActive`, which the legacy `CUIMng`
  activation system (list-order hit-testing via `CursorInWin()`, deferred one-frame activation)
  didn't reliably grant on a timely basis. This produced two confirmed, independently-discovered
  bug mechanisms:
  - `CUIMng::Update()` used to re-enter `SetActiveWin(pWin)` every frame a click was held (since
    `IsLBtnDn()` is level-triggered), which redundantly deactivated the very window being
    re-clicked and starved `UpdateWhileActive()` for the click's whole held duration. **Fixed**
    (2026-09-03): the click loop was changed to skip the redundant `SetActiveWin()` call when the
    clicked window was already active and already head.
  - Every `RmlClickX()` handler used to set a flag (`m_bRmlXClicked`) for `UpdateWhileActive()` to
    consume later, rather than acting immediately — since that poll could go many frames without
    running at all, multiple flags could go stale and an `if/else if` consumer would fire a stale
    flag over a fresh one (observed as: click credit, nothing happens; click menu later, credits
    open instead — proof the credit click *had* registered, just never got consumed in order).
    **Fixed everywhere this pattern existed** (`CLoginMainWin`, `CLoginWin`, `CSysMenuWin`,
    `CCharMakeWin`): every `RmlClickX()` callback was changed to call its action directly,
    bypassing `m_bActive` entirely — safe because `RmlUiRuntime::ProcessSdlEvent()` (where these
    fire) runs from Winmain's SDL event pump, always before `CUIMng::Update()` the same frame. No
    `m_bRmlXClicked`-style flags remain anywhere in the codebase.
  - **The "world-click leaks through an RmlUi panel" half — fixed 2026-09-04, smaller than it
    looked.** `CUIMng::IsCursorOnUI()`/`m_bCursorOnUI` ran entirely through the legacy
    `CursorInWin()` hit-testing, with the same staleness risk already fixed above for click
    *dispatch* — but a full-codebase grep found it had exactly **2 live call sites, both in
    `Scenes/CharacterScene.cpp`** (character-select's click-to-select and 3D object-picking), not
    a sprawling problem across every scene. Both were changed to also gate on
    `Core::Input::IsMouseOverUI()`, the exact same proven pattern as the
    `Selection.cpp`/`ZzzInterface.cpp` fix below — sufficient without inventing any new per-window
    bounding-box query, because `char_sel_main.rml`'s `#panel` is `pointer-events: none` (a pure
    positioning container spanning the full screen), so RmlUi's own hit-test already only reports
    true over the real interactive children. Closed the confirmed bug class (`CharSelMainWin.h`'s
    `CalculateFixedAnchorLayout()` comment — a resolution where its hand-duplicated rect diverged
    from the real RmlUi Delete button once made a genuine click read as "not on UI," silently
    no-op'ing Delete).

  **Deliberately left as its own, still-open item at the time**: `CalculateFixedAnchorLayout()`'s
  hand-duplicated math itself wasn't retired by the fix above (the new `IsMouseOverUI()` gate just
  meant it was no longer the *only* thing standing between a stale rect and a wrong outcome — a
  live RmlUi-element-bounds query replacing it entirely was, and remains, a larger refactor with no
  confirmed bug driving it).

  **The underlying mechanism the rest of this finding describes no longer exists.**
  `CursorInWin()`-driven activation dispatch (`SetActiveWin`/`ShowWin`/`HideWin`/`RemoveWinList`, and
  the `m_WinList` it walked) was found fully unreachable — every window it once drove had by then
  migrated onto `mu::ui::window::CObject`/`CManager` — and deleted outright, and what remained of the
  class was renamed from `CUIMng` to `CSceneUICoordinator` (a pure rename, method bodies unchanged).
  `CalculateFixedAnchorLayout()`'s
  hand-duplicated math (the still-open item just above) is unaffected by that deletion and remains
  exactly as described. The three-parallel-input-tracking-systems root cause this finding used to
  point at is also gone as stated — `CInput`'s VK-polling no longer drives any
  `CSceneUICoordinator` activation/hit-test loop, since there's no such loop left to drive. What's
  left, worth knowing for the next symptom: `Winmain.cpp`'s own event-driven
  `MouseLButton`/`Push`/`Pop` globals (`HandleMouseButton`/`HandleMouseMotion`, legacy 2D world
  input) and RmlUi's own event-driven `Context::ProcessMouseButtonDown/Up` — two systems now, not
  three, neither of them list/activation-driven any more.

- **A window's header-rail chrome split across the fg and bg documents duplicates its `top`/
  `height` values, with no tooling catching drift between the two copies.** Found while fixing a
  visual bug (2026-09-13): a live 3D item icon that overflows its own slot bounding box (e.g.
  equipped wings) rendered *underneath* the header rail, because RmlUi's main context always
  renders last in the frame (the same constraint `RenderBackgroundLayer()` exists for). Fix: strip
  all paint (`background-color`/`decorator`/`border`/`box-shadow`) from the shared
  `.modern-header-rail-px` class in `base.rcss` down to layout only, and give every window's own
  `*_bg.rcss` a `.rail-fill`/`.rail-accent` pair carrying that same paint plus that window's own
  `top`/`height` — literally copied from the fg file's own header-rail override (e.g.
  `storage.rcss`'s `.stor-header-rail` vs. `storage_bg.rcss`'s `.rail-fill`) — so the rail paints
  *behind* the item icon instead of in front of it. Applied to all 10 windows with this chrome
  (`CMyInventory` + the 9-window inventory-family port). `check_rml_rcss_drift.py` only checks
  id/`data-model`-field/event-callback-name presence across a theme's forked files, not numeric
  CSS property agreement — nothing currently catches these two copies drifting apart if one is
  edited without the other. No tooling fix built yet; worth one if this pattern gets copied again
  (a script diffing named property values between a fg selector and its bg counterpart, or simply
  a stronger convention: always grep the sibling file's matching selector before changing either).
- **RmlUi's own `Template::Load()` (`Source/Core/Template.cpp`) finds a `<template>` file's head/body
  regions with a raw, comment-blind text search, not real XML parsing — and its helper,
  `XMLParseTools::FindTag()`, has its own separate, genuinely surprising state bug.** Found the hard
  way (2026-09-16) building `window_shell.rml` (both themes, the first `<template>` this codebase
  has ever used, for `CGenericMenuDialog`'s shared frame/header chrome — see this file's
  `CGenericMenuDialog` entry). Two distinct, stackable failure modes, both triggered purely by
  *comment text* inside the template file itself:
  1. **A literal angle-bracket example of `<body template="...">` anywhere in the file (including
     inside an XML comment) gets sliced in as if it were real markup.** `Template::Load()` locates
     the real `<body>...</body>` region via `FindTag("body", ...)`, a plain substring scan with no
     concept of comments — an illustrative usage example in a doc-comment describing "consumers
     write `<body template=\"window_shell\">`" is found *before* the real body tag if it appears
     first in the file, and if that example uses this same template's own name, the sliced-in body
     content re-triggers `template="window_shell"` on itself when parsed, recursively injecting the
     template into itself with no cycle detection — a genuine stack overflow crash (Windows
     exception `0xc00000fd`) at document-load time. Fix: never write a literal tag-shaped example in
     a template file's own comments — describe the usage in prose instead.
  2. **A stray `/` directly adjacent to the word "head" or "body" anywhere in the file (comments
     included) silently breaks the template for a completely different reason, once (1) is fixed.**
     `FindTag()`'s `found_closing` flag (tracking whether a candidate match was preceded by a `/`) is
     declared *outside* its own per-candidate search loop and is never reset between rejected
     candidates within one call (`Source/Core/XMLParseTools.cpp`). A comment mentioning "real
     head/body regions" (slash, no spaces) sets `found_closing = true` while rejecting that
     obviously-not-a-tag candidate — but the flag stays `true` for the rest of that same `FindTag()`
     call, so the *next* candidate (the real `<body id="panel">` opening tag) gets compared against
     the wrong open/close expectation and is also rejected. The search then runs off the end of the
     file, `FindTag()` returns null, and `Template::Load()` fails outright — logged only as `Failed
     to load template ...`/`Template ... not found`/`Failed to find template '...'`, no crash, no
     stack trace, and (confirmed the hard way) the consuming document silently falls back to
     unstyled flow layout: no frame/border/header visible at all, with whatever content the consumer
     supplied just stacked at the top of the screen in raw document order. `.gcd`/`.gmd`-family
     documents are especially exposed to this since their own doc-comments already routinely
     describe "head" and "body" together. Fix: never join "head" and "body" with a bare slash
     (comments included) anywhere in a `<template>` file — write "head and body" instead. Both bugs
     are specific to files loaded *as* a `<template>` (via `TemplateCache::LoadTemplate()`/
     `Template::Load()`); normal `.rml` documents parsed the regular way never hit either code path,
     so this only matters when authoring a new shared template, not every RmlUi file in this
     codebase.
- **A `<link type="text/rcss">` living inside a `<template>` file's own head never gets this
  project's `token(...)` design-token substitution — it reaches RmlUi as raw, unsubstituted text,
  and every `token(...)` call in it silently fails to parse.** Found the hard way (2026-09-16)
  building `window_shell_bg.rml` (the background-only shell variant, `CGenericConfirmDialog`'s bg
  document being the first consumer): the template's own head originally linked `base.rcss`
  directly (mirroring how a normal document does it), and every single `token(...)`-based property
  in `base.rcss` (dozens of them — colors, borders, decorators) failed with a `[RmlUi][Warning]
  Syntax error parsing property declaration '...token(...)...'` at document-load time — no crash,
  just large, silent chunks of the shared theme's own styling missing. Root cause:
  `token(...)` isn't a real RmlUi feature at all — it's this project's own preprocessing step
  (`RmlTheme.cpp`'s `InlineTokenizedStylesheet()`), which works by regex-scanning the **raw RML
  text** of whatever file `UI::RmlBridge::LoadThemedDocument()`/`CreateBackgroundDocument()` was
  asked to load, finding that document's own direct `<link type="text/rcss">` entries, and rewriting
  each one in place with its tokens already resolved, all *before* RmlUi's own document loader ever
  sees the text. A template file is fetched by RmlUi's own internal `TemplateCache::LoadTemplate()`
  (a plain `StreamFile`/`fopen` read), which never runs through this project's C++ loading path at
  all — so a template's own `<link type="text/rcss">` entries are invisible to
  `InlineTokenizedStylesheet()` no matter what they point at. Fix: never link a `token(...)`-using
  stylesheet from within a `<template>` file's own head — link it directly from the top-level
  consuming document's own head instead (`generic_confirm_dialog_bg.rml` linking `base.rcss`
  directly, rather than `window_shell_bg.rml` doing it on its behalf, is the actual fix). A
  template's own paired `.rcss` is safe to link from within the template itself only if it contains
  no `token(...)` calls (confirmed true of `window_shell.rcss`/`window_shell_bg.rcss`, both
  themes — plain dp/hex values throughout, no design tokens).
- **Moving an already-parsed element to a new parent (`Element::RemoveChild()` then
  `AppendChild()` elsewhere in the same document) does not preserve a `{{}}` text-interpolation
  binding, even though it does preserve `data-*` attribute bindings (`data-event-click`,
  `data-class-*`, etc).** Found building `option_window.rml`'s close button (2026-09-18): moved a
  `<div>{{close_label}}</div>` from inside `window_shell`'s `#content` splice slot into a new
  `#window_shell_footer` anchor via a small `UI::RmlBridge` helper, immediately after
  `LoadThemedDocument()` returned. The button's click handler kept working; its label text never
  rendered, in either theme. Root cause, confirmed by reading the vendored source directly:
  `RemoveChild()` → `SetParent(nullptr)` → `SetDataModel(nullptr)` → `DataModel::OnElementRemove()`
  tears down that element's existing `DataView`s. `AppendChild()` into the new parent →
  `SetParent(new_parent)` → `SetDataModel(model)` → `ElementUtilities::ApplyDataViewsControllers()`
  re-fires, but that function **only re-scans the element's own attributes for `data-[type]-...`
  bindings** (`ElementUtilities.cpp`) — it has no code path that re-parses a plain text node's
  `{{expr}}` content, because that substitution normally only ever happens once, at the text
  node's original XML-parse time. An attribute-bound behavior (click, class toggle, two-way
  `data-value`) survives the move; a `{{}}`-bound text node's own binding does not, silently,
  leaving it either blank or frozen at whatever it last resolved to. **Fix: don't reparent an
  already-`{{}}`-bound element.** Either build the element directly at its final location in the
  first place (imperative C++: `ElementDocument::CreateElement()` + `AppendChild()`, setting its
  text via `SetInnerRML()` each sync instead of a data binding — `COptionWindow`'s close button now
  does this), or keep it declared in its original position and reach the same visual effect with
  CSS (`position:absolute` against a shared ancestor) instead of an actual DOM move.
- **RmlUi's native `<select>`/`<option>` (`WidgetDropDown`) needs its generated `selectvalue`/
  `selectbox`/`selectarrow` sub-elements styled by hand, including `position` — unlike
  `WidgetSlider`, which force-sets `drag` on its own generated sub-elements programmatically,
  `WidgetDropDown` sets only `visibility`/`z-index`/`clip`/`overflow-y` on `selectbox`
  (`WidgetDropDown.cpp`) and leaves `position` entirely to the consumer's own CSS.** Skipping it
  (as `option_window.rcss` initially did for all 5 dropdowns) makes the option list render as
  ordinary in-flow block content instead of a floating overlay: no background (transparent over
  whatever's behind the panel), no bounded size (grows the row instead of overlaying it), and
  multiple open dropdowns' lists interleaving into the same flow with no visual separation. Even
  after adding `position:absolute`/`z-index`/background/an explicit `scrollbarvertical` width to
  `selectbox` (mirroring `generic_confirm_dialog.rcss`'s own already-proven scrollbar recipe),
  clicking an option still did not reliably commit a selection or fire this window's own `change`
  callback, for a cause never conclusively isolated. **Recommendation: don't reach for native
  `<select>` for a new dropdown in this codebase — use a `data-for`-driven custom control instead**
  (a clickable "current value" box toggling a `data-class-hidden` sibling list of
  `data-for`-generated option rows, each with its own `data-event-click`), the same mechanism this
  codebase's tab-bar pattern already uses successfully. `option_window.rcss`'s `.option-dropdown`/
  `.option-dropdown-value`/`.option-dropdown-list`/`.option-dropdown-option` family (2026-09-19) is
  the worked example — it replaced all 5 native `<select>`s outright once the above didn't hold up
  after three separate rounds of fixes.
- **A persistent RmlUi document driven by a scene's own restricted manual pump (`UpdateMouseEvent()`
  + `UpdateKeyEvent()` + `Render()`, bypassing the full `mu::ui::window::CManager::Update()` sweep)
  must still have its own `Update()` called explicitly — it is not implied by the other three.**
  `LoginScene.cpp`/`CharacterScene.cpp` both pump `COptionWindow` this way (comment: "can't use full
  g_pNewUISystem update", since those scenes don't run the full HUD/inventory UI stack), and both
  omitted `Update()`. `COptionWindow::Update()` is what runs `SyncRmlModel()` — the step that pushes
  this window's own C++ state into its RmlUi data model — so every `{{}}`-bound field stayed at its
  default-constructed empty value and the window rendered with no text at all, specifically in
  those two scenes. Because the window is a single persistent instance, the very first real
  `Update()` call (from the full sweep, once a scene reaches it) permanently populates the model —
  which is why the symptom didn't reappear on a later visit to a scene that still skips the call.
  Fix: any scene-local manual pump of a persistent window needs `Update()` in the same sequence the
  full sweep uses (`UpdateMouseEvent` → `UpdateKeyEvent` → `Update` → `Render`), not just the three
  that look input/render-related by name.

## `CObject`/`CManager`/`LayoutMode` gotchas

Found during the `CWin`→`CObject` migration itself (now complete, see `migration-ledger.md`), but
these are facts about the still-live `CManager`/`UI::Scaling` machinery every window on this tier
depends on — not historical record of that migration. That file is now a short pointer to git
history; this is the durable half of what it used to say in full.

- **`CManager::AddUIObj(dwKey, obj)` silently overwrites the object's `LayoutMode` on first
  registration** — it calls `obj->SetLayoutMode(UI::Layout::ForInterface(dwKey))` unconditionally,
  clobbering whatever the constructor set. `UILayoutPolicy.cpp`'s table, not the window's own
  constructor, is the real authority — every `INTERFACE_*` key needs its own explicit `case` there,
  or it silently falls through to `default: return LayoutMode::Dialog` (a real, resolution-scaled
  transform, not a safe no-op default).
- **A window with real-pixel (not reference-space) rendering needs `LayoutMode::Legacy`** — the one
  mode that resolves to a genuine identity transform. Two independent reasons this matters: (1)
  `CManager::UpdateMouseEvent()` wraps each dispatch in a `ScopedActiveTransform(transform,
  transformMouse=true)` that remaps the *global* `MouseX`/`MouseY` through whatever transform is
  active — a window expecting real, untransformed mouse coordinates for its own hit-testing gets
  checked in the wrong coordinate space under any other mode. (2) Text rendering consults
  `UI::Scaling::GetActiveTransform()` directly; `CSprite::Render()` does NOT consult the transform's
  *scale* (baked in once at `Create()`), but DOES read its *offset* — a mixed, partial dependency,
  not all-or-nothing. A wrong active transform corrupts text scale/position and sprite offset, but
  never sprite scale.
- **A legacy widget that reads `GetActiveTransform()` at render time (not just once at
  position-set time) can end up double-scaled** if its owning window's `Update()` now runs inside a
  `ScopedActiveTransform` scope it didn't run inside pre-migration. `CUITextInputBox::Render()` is
  the concrete case: it rescales the position it was given via `ConvertPositionX/Y` using whatever
  transform is active *when `Render()` runs* — a fundamentally different contract than `CSprite`'s
  "store real pixels, ignore the transform entirely." Code that divides a real-pixel position by
  the ambient transform before `SetPosition()`, relying on `Render()`'s later multiply to cancel it
  back out, breaks the moment the divide and the multiply no longer see the same transform (e.g.
  the divide now runs inside a `LayoutMode::Legacy` identity scope but the later `Render()` doesn't,
  or vice versa) — fix by never doing the divide-now/multiply-later dance at all: store real pixels
  directly, and wrap the consuming `Render()` call in the same explicit `ScopedActiveTransform` the
  write used, so both sides agree unconditionally regardless of which context triggered the write.
  Any window driving a `CUIControl`-family widget (`CUITextInputBox` and siblings) needs this
  "identity at both ends" treatment.
- **`g_pTimer`'s `GetTimeElapsed()` is total process uptime, never reset anywhere in this
  codebase** (`ResetTimer()` has zero callers) — the existing `dDeltaTick =
  MIN(g_pTimer->GetTimeElapsed(), 200.0 * FPS_ANIMATION_FACTOR)` only behaves like a per-frame delta
  because the clamp dominates almost immediately and forever after; in steady state it's just `200.0
  * FPS_ANIMATION_FACTOR`. A window's own parameterless `Update()` needing a real per-frame delta
  should read that same clamped expression, not `g_pTimer` itself.
- **A window's own per-frame `Update()` can race a same-frame check elsewhere that depends on its
  state.** One instance: a full `Update()` sweep running before a check for "is anything currently
  visible" let a single input event (e.g. Esc) both close a window *and* trigger a different
  toggle in the same frame, since the toggle check saw the just-updated (already-changed) state
  instead of the frame's starting state. General lesson: order matters when one piece of per-frame
  logic reads state another piece in the same frame can resolve/consume — decide the ordering
  deliberately, don't assume "runs later in the function" is safe by default.
- **A window using `CObject`'s shown/active split (`IsActive()`/`SetActive()`/
  `UpdateWhileShown()`/`UpdateWhileActive()`) should compute `SetActive()` from *every* condition
  that ought to suspend active-handling, not just whatever originally motivated adding the split.**
  Folding every "a higher-priority modal is covering me" condition into one `SetActive(...)` call at
  the top of `UpdateWhileShown()` means `CObject::Update()`'s own dispatch gates
  `UpdateWhileActive()` automatically — one place this logic lives, not a separate ad hoc check per
  modal. Also generalizes: setting a per-frame gating flag from state read *before* a call that
  might resolve/consume that same state later in the same function reproduces a "snapshot before it
  changes" ordering for free, without a dedicated snapshot member.
- **Check whether a window's own click-gating override is hardcoded to always return a fixed value
  before assuming it needs real click-handling ported** — a window whose interactivity was already
  structurally unreachable pre-migration needs no porting for that piece, just a literal
  `return true;`/`return false;` in the new method. Don't assume interactivity from the presence of
  buttons/sliders in a window's member list; check the actual gate.
- **A modal window relevant to a gameplay scene (not just menus) must fold its own claimed-input
  state into whatever that scene's "is the cursor over UI" query consults**, or a click on the world
  behind it falls through to world/gameplay logic once it's no longer tracked by whatever the old
  aggregate query walked. Fix generally, not per-window: fold the new tier's own per-frame
  hover/claim result into the same aggregate the legacy query already computed, before the legacy
  side's own early-return, so any future window gets this for free with no per-window special-casing
  — it just needs to implement its own input methods to actually claim (return the "consumed" value)
  when appropriate.

