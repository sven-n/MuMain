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

  **Update, 2026-09-05**: the underlying mechanism the rest of this finding describes no longer
  exists. `newui-legacy-merger.md`'s Phase 4 found `CursorInWin()`-driven activation dispatch
  (`SetActiveWin`/`ShowWin`/`HideWin`/`RemoveWinList`, and the `m_WinList` it walked) fully
  unreachable — every window it once drove had by then migrated onto `mu::ui::window::CObject`/
  `CManager` — and deleted it outright, then renamed what remained of the class from `CUIMng` to
  `CSceneUICoordinator` (a pure rename, method bodies unchanged). `CalculateFixedAnchorLayout()`'s
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

