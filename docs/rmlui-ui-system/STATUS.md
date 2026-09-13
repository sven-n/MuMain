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
  `COptionWin` (the `CWin`-tier options window) was confirmed unreachable in live play — its
  RmlUi port was never wired up, and the class was later deleted outright as confirmed-dead code
  (see `README.md`'s Coexistence patterns); `CSysMenuWin`'s Option button opens
  `mu::ui::window::COptionWindow` instead. Built and verified against a real server, both themes.
- **In-game HUD (`mu::ui::window::CObject` tier)** — `CMuHelperBar` (map/position readout + MU Helper bot
  control bar) and `CBuffStrip` (active-buff icon strip, the `data-for`/dynamic-array pilot at
  this tier) are fully done. `CMainFrameWindow`'s 3-phase HUD-frame port is **done**: Phase 1
  (HP/MP/AG/SD/EXP bars + 5 corner buttons), Phase 2 (`CSkillList` — compact hotkey row
  click/hover/cooldown, expanded skill grid, pet-command row, and skill tooltips for both themes,
  replacing the old hand-rolled `EVENT_STATE` hover/click machine entirely), and Phase 3
  (`CItemHotKey` — potion-slot hover-highlight border, stack-count text, and right-click-to-use,
  all via RmlUi; the potion icon itself stays native — see the correction below). **Icon/box-frame
  art for the skill grid and pet row stays legacy 2D**, a deliberate Phase 2 scope cut — see the
  pilots-to-revisit table below. **Correction, 2026-09-06**: Phase 3's icons are not a sprite-atlas
  porting gap — traced to `RenderItem3D()`/`RenderObjectScreen(MODEL_...)`
  (`ZzzInventory.cpp`), they're genuine live 3D model renders, the same permanent, no-RmlUi-
  equivalent category as `CCharMakeWin`'s character-preview panel (`ui-target-architecture.md`
  Section E) — no future pattern will port the icon itself. Phase 3 accordingly only moved the
  slot chrome (hover-highlight border, stack-count text, right-click-to-use — the last of these the
  first `data-event-mouseup`/right-click RmlUi binding in the codebase, see
  `newui-tier-adapter.md`'s own former "still unproven" entry) to RmlUi as an overlay around the
  still-native icon, the same split Phase 2 already proved for skill icons, not a new pattern. All
  landed pilots (including Phase 3) built and verified against a real server, both themes. The rest
  of this tier — ~88 other `mu::ui::window::CObject` windows, drag-and-drop, and 3D-camera-space
  rendering generally — is not
  yet migrated.
- **`CMyInventory`** — Stage 1 (frame/title/gold/4 corner buttons, retiring `CButton` for this
  window entirely) and Stage 3 (Set/Socket option header labels + their shared hover tooltip,
  replacing the legacy `TextList`/`RenderTipTextList` mechanism — see
  `UI/Inventory/ItemOptionTooltipModel.h`, the same `BuildModel`-then-bind-or-legacy-render
  pattern `CMainFrameWindow`'s skill tooltip already proved) are **done**, both themes, built and
  verified against a real server. **Stage 2 (equipment paperdoll chrome — background sprite,
  durability tint, drag-compatibility highlight) was deliberately skipped**: those three visuals
  render *behind* the equipped item's live 3D icon today (native paint order, `RenderEquippedItem()`
  at a lower `GetLayerDepth()` than `Render3D()`'s camera); porting them to RmlUi's main context
  (always last in the frame) would flip that to *in front of*, a real regression, and preserving
  the current look needs the same background-context mechanism that already produced two real
  bugs during Stage 1 (position-transform, once-per-frame double-render) for no functional gain —
  see the pilots-to-revisit table below. The equipment grid (`CInventoryCtrl`) and both paperdoll/
  grid live-3D icons stay permanently native either way (Section E, same category as
  `CItemHotKey`'s potion icon). **Drag-by-title-bar** (`#title`, `UI::RmlBridge::MakeDraggable()`'s
  first real caller) with a **persisted, override-aware position** (`GameConfig::GetWindowPosition`/
  `SetWindowPosition`, `RestoreDefaultOrUserPosition()`) is also done — see the "Known gaps" entry
  below for the full mechanism, built generically so the next draggable window reuses the same two
  pieces rather than inventing its own.
- **Rest of the inventory family** (`CTrade`, `CStorageInventory`, `CStorageInventoryExt`,
  `CMixInventory`, `CNPCShop`, `CMyShopInventory`, `CPurchaseShopInventory`,
  `CInventoryExtension`, `CLuckyItemWnd`) — **done, both themes (2026-09-13)**: frame/header-rail/
  title/wallet/action-buttons ported per window, following `CMyInventory`'s background-context
  pattern; each window's own `CInventoryCtrl` grid(s) (and, for `CTrade`, both grids) stay fully
  native, same permanent no-RmlUi-equivalent category as `CMyInventory`'s own grid. Shared
  primitives factored out along the way instead of copy-pasted nine times:
  `UI::RmlBridge::SyncRootTransform()`/`CreateBackgroundDocument()` (`RmlRootTransform.h`,
  `RmlTheme.h`/`.cpp`) replace the hand-rolled root-transform math and background-document
  boilerplate `CMyInventory` had; `base.rcss`'s `.modern-header-rail-px`/`.modern-wallet-px`/
  `.modern-icon-btn-px` families give every window in this tier the same header/wallet/button
  chrome instead of a one-off per window. This is also the trigger condition the "Known gaps"
  entry below names for generalizing `RenderBackgroundLayer()` — see that entry for what changed.
  `CMyShopInventory`/`CPurchaseShopInventory` keep their native `CUITextInputBox` subject-line field
  exactly where `building-new-ui.md` says a Type-2 companion widget belongs — only its decorative
  background sprite moved to RmlUi (and, after a visual bug, from the fg document to the bg one, so
  it doesn't cover the real input box). `CMixInventory`'s large recipe/success-rate dynamic text
  block stays native — frame chrome only for this pass, the text panel is a separate, bigger job.
- **`CCharacterInfoWindow`** — **done, both themes (2026-09-13)**: fully ported, no permanently-
  native content at all (unlike the inventory family above, this window has no `CInventoryCtrl`
  grid, no live-3D icon, and no `CUITextInputBox` — it's a plain `CManager`-tier window, not
  `C3DRenderMng`-backed, so it needed none of the background-context/`RenderBackgroundLayer()`
  machinery). Frame/name/class-server crossfade/summary box/all five attribute rows (STR/AGI/VIT/
  ENE, plus CMD for Dark Lord)/level-up "+" buttons/Exit/Quest/Pet/Master-Level all move to
  RmlUi; every stat line `RenderAttribute()` used to draw natively (attack/defense/attack-speed/
  mana/magic-and-curse-damage/class-specific bonus lines, buff-conditional colors) is now computed
  in C++ exactly as before and pushed into a small `StatLine{text,color}` list per attribute box,
  rendered via normal block flow (`display:block` stacking, 13px line-height) instead of
  per-line C++-computed `top` offsets — reproduces the original's variable per-class/per-buff line
  count for free. `LoadImages()`/`UnloadImages()` are kept even though this window no longer
  renders through the legacy bitmap-atlas system: `CGensRanking` and `CUIMuHelper`'s own hunt/
  pick-range "+" buttons alias their own `IMAGE_LIST` entries onto these same texture slots (same
  reason `CMyQuestInfoWindow` keeps its own `LoadImages()`). One deliberate simplification: the
  summary box's original 8-piece pixel-tiled frame (4 corner sprites + a 1px sprite tiled across
  each edge, nested per-pixel `for` loops) is reproduced as the 4 corner sprites plus a flat
  translucent fill rather than a literal repeating-tile port — this build has no established,
  verified pattern for a real 1px-tile repeat (see "Findings" below), and the corners alone already
  read as "framed." Built and verified against a real build (Debug, `windows-x64`); in-game
  verification against a live server still pending for both themes.
- **`CGenericConfirmDialog`** — **done, both themes (2026-09-13)**: the reusable confirm-dialog
  primitive `UI/Dialogs/CommonMessageBox.h`/`CustomMessageBox.h`'s ~140-class native `TMsgBoxLayout<T>`
  family lacked (see this file's own "Tracked deferral: `CommonMessageBox`/`CustomMessageBox`" entry
  below, and `component-catalog.md`'s "Dialog" section for the full shape — this is a separate
  native subsystem from `UIControls.h`'s `CUIControl` family, not part of that other tracked
  deferral). One C++ class + one document, shown with a `GenericDialogConfig` value (button set,
  body lines, OK/Cancel callbacks) instead of a new subclass per dialog. Proven on 3 real dialogs,
  replacing their native call sites end-to-end and deleting the 3 now-dead native classes:
  `Guild/GuildInfoWindow.cpp`'s alliance-master-can't-leave notice (OK-only), `UI/Quests/
  MyQuestInfoWindow.cpp`'s quest-giveup confirm (OK/Cancel, real `SendQuestCancelRequest` on OK),
  `Network/Server/WSclient.cpp`'s guild-invite accept/decline (OK/Cancel, the "shell + caller fills
  in body lines after construction" pattern, triggered from a network packet handler rather than a
  UI click). Built and verified (RelWithDebInfo); in-game verification of all 3 swapped dialogs
  (both themes, Enter/Esc, and a second dialog queuing while one is open) still pending.

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
   the current pass (`CMainFrameWindow`'s own file — see "Tracked deferral:
   `CMainFrameWindow`'s own class rename" below); don't treat that as a general excuse to
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
a couple are about the legacy `CWin`/`CSceneUICoordinator` (named `CUIMng` at the time these were
found — see the note at the end of this section) C++ architecture surrounding RmlUi instead. Tier
-specific findings (`mu::ui::window::CObject`-tier) live in `newui-tier-adapter.md`'s "Proven by CBuffStrip"
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
- **A persistent RmlUi document needs its own scene-visibility gate.** `mu::ui::window::CSystem` (the
  `mu::ui::window::CObject`-tier manager) is a persistent app-lifetime singleton whose `Update()`/`Render()`
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
- **`decorator: image(...)` does not stretch a sprite rect to fill a box sized differently from
  the sprite's own native dimensions in this RmlUi build.** `legacy-btn-idle`/`-hover`/`-active`
  (`base.rcss`'s `@spritesheet`) are fixed 108x30 rects; every existing user of them (`.btn`,
  `.group-btn`) is also exactly 108x30. `server_select.rcss`'s `.server-row` tried the same
  decorator stretched to 186x30 (the first place in this codebase asking for a non-native size) and
  most of the box stayed unpainted instead of the sprite scaling to fill it — visually, a normal-
  looking dark row with no visible button chrome except near the sprite's own native footprint.
  Reverted to a flat `background-color`/`border` for that row instead (no native size to mismatch).
  Don't assume this decorator behaves like a CSS `background-size` image at an arbitrary box size —
  verify at the actual target dimensions, or stick to the sprite's native size.
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
  exists. `docs/newui-legacy-merger.md`'s Phase 4 found `CursorInWin()`-driven activation dispatch
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

## Known gaps against the principles (honest status, not yet built)

None of these are wrong so far — the principles doc explicitly endorses incremental delivery
(§26–27) — but they're real, currently-unaddressed gaps in the end-state architecture, not yet
even scheduled. Recorded so no future session mistakes "the pilots pass their own verification"
for "the full architecture is in place":

- **Correction, 2026-09-13**: `ui-target-architecture.md`'s Section D table claimed `CManager`'s
  dispatch loop "was never actually broken." A real bug in it was found and fixed this session:
  `CMyInventory::UpdateMouseEvent()`'s ground-drop guard returned `false` to mean "not a ground
  drop, some other window should handle this," but `CManager::UpdateMouseEvent()`
  (`WindowManager.cpp`) treats any `false` as "consumed, stop dispatching to every remaining
  window this frame" — not "defer to the next one." This silently ate every attempt to drop an
  item into `CTrade`'s own offer grid (and, by the same guard, `CStorageInventory`/`CNPCShop`/
  `CMyShopInventory`/`CPurchaseShopInventory`/`CMixInventory`/`CLuckyItemWnd`, all checked in the
  same guard) for as long as that code existed — a longstanding native bug, unrelated to this
  session's RmlUi work, that simply hadn't been exercised end-to-end before. Fixed by returning
  `true` from that branch. The dispatch *design* (topmost-first, consume-and-stop) doesn't need to
  change; the contract just isn't written down anywhere but the loop itself, and a future window
  adding a similar "not for me" guard could make the identical mistake — `UpdateMouseEvent()` must
  return `true` to let dispatch continue to lower-`GetLayerDepth()` windows, `false` only to
  genuinely consume the event and halt the frame's dispatch there.
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
  `tools/migrate_rcss_tokens.py`) from the old "value + comment" convention to real `token(...)`
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
  `tools/check_rml_rcss_drift.py`, wired into the build next to `check_rml_rcss_syntax.py`: for
  every window whose C++ calls `LoadThemedDocument`, confirms every id/`data-model` field/
  event-callback name it references appears somewhere across the shared file plus every
  `themes/*/` fork of that document (checked against the union of all copies, not each
  individually — a per-copy requirement flags the legitimate "different themes bind different
  precomputed alternatives" pattern, e.g. `main_frame`'s `hp_text` vs `hp_current_text`, as false
  drift).
- ~~Two C++ call sites branch on theme *name*, violating `architecture-principles.md` §30.~~
  **Fixed 2026-09-04.** `MainFrameWindow.cpp`'s background-fill-behind-legacy-icons and
  selected-skill-slot-highlight logic (4 call sites total, not 2 — see the "Pilots to revisit"
  table below) now branch on `UI::RmlBridge::ThemeProvidesOwnIconChrome()`, a declared capability
  read from an optional `themes/<name>/theme.ini` (`[Capabilities] ProvidesOwnIconChrome=1`) —
  missing file/key defaults to false, so only `modern` (the one theme with non-default behavior)
  needed a new file. The underlying paint-order constraint itself is unchanged (see the next
  entry) — this fix is the capability flag the entry below already anticipated, not a removal of
  the conditional.
- ~~RmlUi rendering strictly last in the frame — a real, systemic constraint, not one HUD
  window's quirk.~~ **Phase 1 (the primitive + one proven caller) built 2026-09-04.** Two
  directions:
  - **Legacy content rendering *after* RmlUi** — `MuRenderer.h`'s `SetPostRmlUiCallback` (game
    cursor, `CMsgWin`, `CharMakeWin`, `MuHelperBar`, `NewUISystem`) already provided this. Not a
    gap, never was.
  - **RmlUi content rendering *before* a specific mid-frame point** — the direction
    `MainFrameWindow.cpp`'s icon-chrome conditional needed, and every other window sharing
    `mu::ui::window::C3DRenderMng` will hit too — was genuinely missing infrastructure, now built.
    The whole inventory-family tier (`CMyInventory`, `CTrade`, `CStorageInventory`,
    `CStorageInventoryExt`, `CMixInventory`, `CNPCShop`, `CMyShopInventory`,
    `CPurchaseShopInventory`, `CInventoryExtension`, `CLuckyItemWnd`) is now ported and using this
    mechanism (2026-09-13); several message-box/quest/duel windows on the same
    `C3DRenderMng` tier are not yet ported:
    - **`IMuRenderer::FlushRenderCommands()`** (`MuRenderer.h`/`MuRendererSDLGpu.cpp`) — opens a
      real render pass *mid-recording*, replaying only what's been recorded since the last flush
      (or frame start). Turned out bigger than "add one more callback like
      `SetPostRmlUiCallback`": rendering here is fully deferred (`g_pNewUISystem->Render()` only
      appends to `s_renderCmds`; nothing reaches the GPU until `EndFrame()`'s one pass, which
      unconditionally `CLEAR`s) — a pass opened earlier would just get wiped by that clear. Fixed
      by refactoring `EndFrame()`'s vertex/bone/strip/texture staging (`StageDeferredGpuData()`)
      and its render-pass replay loop (`ReplayCommandRange()`) into helpers callable more than
      once per frame, tracked via `s_replayedCmdCount`/`s_mainColorPassOpenedThisFrame` (reset in
      `BeginFrame()`) so the first pass of the frame — a flush or `EndFrame()`'s own — `CLEAR`s and
      every one after `LOAD`s. The pre-existing screenshot/readback capture path (a different
      target texture than the swapchain) is untouched: it always does a full `CLEAR`+full-replay
      of its own, recovering flushed content from `s_renderCmds` even though the flush itself only
      ever wrote to the swapchain.
    - **`RmlUiRuntime::GetBackgroundContext()`/`RenderBackgroundLayer()`** (`RmlUiRuntime.h`/`.cpp`)
      — a second, background-only `Rml::Context`, driven explicitly (flush, then
      `Update()`+`Render()`) instead of through the single-slot `SetPreSubmitCallback` the "main"
      context uses. Needs no input routing at all (never registered as an `IUiInputConsumer`) —
      every document loaded into it is `pointer-events: none`, same convention as
      `char_sel_main.rml`'s `#panel`.
    - **Proven end-to-end**: `MainFrameWindow.cpp`'s `RenderLeftFrame()`/`RenderCenterFrame()`
      background-fill hack (`RenderColorQuadARGB` + `ThemeProvidesOwnIconChrome()`) is retired —
      replaced by `themes/modern/main_frame_bg.rml`/`.rcss`, a real RmlUi document rendered via
      `RenderBackgroundLayer()`, tracking the same `BottomHudCenterTransform`/anchor-offset values
      `main_frame.rml`'s own `#bars` group already used (`CMainFrameWindow::SyncRmlModel()`,
      a second small `RmlModelBinder`). The skill-list-up highlight overlay in the same function
      stays a legacy quad — never blocked by this constraint, no reason to move it. **Verified
      visually against a real server, modern theme, 2026-09-04**: potions and skill icons still
      render and animate correctly on top of the now-RmlUi-authored background, no regression.
  - **Phase 2 done (2026-09-13)**: the trigger condition below fired nine times over (the whole
    inventory-family port), so the call is now centralized in
    `mu::ui::window::CManager::Render()`'s own z-sorted loop (`WindowManager.cpp`) instead of each
    window wiring its own. Simpler than the `INVENTORY_CAMERA_Z_ORDER`-threshold design sketched
    below: no z-order audit needed — the call sits immediately before `(*vi)->Render()` inside the
    existing `if ((*vi)->IsVisible())` branch, so it still fires exactly once per frame, at the
    exact same point in the sequence the first visible bg-doc-owning window's own call used to
    (`RenderBackgroundLayer()`'s no-op-after-first guard, unchanged, is what makes every later
    iteration's call in the same frame free). One consequence worth knowing: this call now fires
    every frame regardless of which window happens to be first in z-order, not just ones that own
    background content — any window whose own background-context document stays `Show()`n across
    its own hidden state must gate that document's visibility itself (`CMainFrameWindow` needed a
    fix here: `m_pRmlBgDoc` used to rely on `RenderBackgroundLayer()` only running while
    `RenderLeftFrame()` did, i.e. while `CMainFrameWindow` itself was visible — no longer true, so
    `SyncDocVisibility()` now gates `m_pRmlBgDoc` the same `IsVisible() && sceneAllowsShow` way it
    already gated `m_pRmlDoc`). The inventory-family windows never had this problem — each one's
    own `SyncRmlModel()` already explicitly `Show()`/`Hide()`s its bg doc off its own `IsVisible()`,
    independent of who calls `RenderBackgroundLayer()`.
  - **Correction, 2026-09-13**: the first version of this centralization put the call directly in
    `CManager::Render()` unconditionally, which broke login/character-select — an empty background
    panel (no title/buttons, since those live in the foreground context and stay hidden) briefly
    appeared over those scenes. Root cause: `RmlUiRuntime`'s background context is a single
    app-lifetime singleton, but **two** `CManager` instances exist —
    `CSystem::m_pNewUIMng` (app-lifetime, owns the inventory family + `CMainFrameWindow`, `Update()`/
    `Render()` only ever called during `MAIN_SCENE`) and `CSceneUICoordinator::m_NewStyleMng` (a
    second, scene-scoped instance driving login/character-scene windows migrating off `CWin`, e.g.
    `CSysMenuWin`, `CCreditWin`). `CreateBackgroundDocument()` `Show()`s eagerly at `CSystem::Create()`
    time (app startup, before the login scene even renders) and nothing ever `Hide()`s those
    documents until `m_pNewUIMng`'s own `Update()` first runs (`MAIN_SCENE` only) — so putting the
    call unconditionally in `CManager::Render()` made `m_NewStyleMng.Render()` (which *does* run
    during login/character-select) paint `m_pNewUIMng`'s windows' stale, still-default-positioned
    background docs into the wrong scene. Fixed with an explicit opt-in,
    `CManager::SetDrivesBackgroundLayer(bool)`, called `true` only on `m_pNewUIMng`
    (`CSystem::Create()`) — `m_NewStyleMng` defaults to `false` and never fires the call. Any future
    additional `CManager` instance defaults to not driving this layer; opt in explicitly only if its
    own windows actually load documents into the background context.
  - ~~Not yet done (Phase 2, deliberately deferred): generalizing the one proven call site into
    a single insertion point inside `mu::ui::window::CManager::Render()`'s own z-sorted loop (gated on
    crossing `INVENTORY_CAMERA_Z_ORDER`, 5.5 — every `mu::ui::window::C3DCamera` z-order, unlike every
    2D-chrome window's, not yet audited project-wide) so every window on `mu::ui::window::C3DRenderMng`
    benefits automatically instead of each one wiring its own `RenderBackgroundLayer()` call.
    Ship this when the first inventory-family window's own port actually needs it, not
    speculatively ahead of that — `component-catalog.md` §26.~~
  - Also caught and fixed while writing `check_rml_rcss_drift.py`'s test against this change: the
    checker pooled every `.Bind()`/`.BindEventCallback()`/`GetElementById()` call in a `.cpp` file
    into *every* document that file loads — silently correct as long as no file owned more than
    one themed document. `MainFrameWindow.cpp` now owns two (`main_frame`,
    `main_frame_bg`); fixed by scoping each call to whichever `RmlModelBinder::Create()`/document
    pointer it's textually associated with, not the whole file.
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
  been explicitly audited for windows other than `CMyInventory` (below)** — does a dragged position
  survive a UI-scale change sensibly on other windows once they gain dragging? A theme change?
  Not examined beyond the one pilot.
- ~~`UI::RmlBridge::MakeDraggable()` (`RmlDraggable.h/.cpp`) has zero live call sites and still has
  one open §10-11 gap.~~ **Fixed 2026-09-07 — first real caller landed (`CMyInventory`, drag-by-
  title-bar) and both remaining gaps closed as part of it, generically, not as a one-off:**
  - **Persistence**: `GameConfig::GetWindowPosition()`/`SetWindowPosition(windowId, x, y)`
    (`GameConfig.h`/`.cpp`) — an immediate disk write (bypasses the usual member-field+`Save()`
    batching every other setting uses, deliberately: a drag has no "Apply" button, so deferring to
    the general save lifecycle would lose it on a crash or an ordinary Alt+F4), keyed by a short
    caller-chosen `windowId`, reusable by any future draggable window with one call each way.
  - **Theme-default-layout conflict**: `MakeDraggable()` gained an `OnDragEnd` callback (RmlUi's
    `Dragend` event — previously only `Dragstart`/`Drag` were wired) as the "persist now" hook.
    `CMyInventory::RestoreDefaultOrUserPosition()` replaces the 3 `WindowSystem.cpp` call sites that
    reset this window to its idle default column — restores the saved user position instead, if
    one exists. The **other** 3 call sites that shift this window sideways because Character-
    info/Inventory-Ext is *currently* also visible (real collision avoidance, not a "default reset")
    were deliberately left unconditional — skipping those would let a dragged Inventory panel
    visually overlap Character info, trading one real bug for another.
  - Still open, same as before: no audit yet of how a persisted position behaves across a
    resolution/UI-scale/theme change (see the item above) — `CMyInventory` stores it as its own
    reference-space `m_Pos` (the same logical coordinate every other position in this codebase
    scales from), which should behave correctly by construction, but hasn't been tested against a
    real resolution/scale change post-drag yet.
- ~~No reusable-component catalog exists as such~~ (§20) — **addressed 2026-09-04**:
  [`component-catalog.md`](component-catalog.md) inventories what already functions as a reusable
  primitive (`RmlModelBinder<T>`, `UI::RmlBridge` helpers, the anchor/center/stretch RCSS classes,
  Button/Checkbox's real shared contract) versus what genuinely doesn't exist yet (`ItemSlot`,
  `ProgressBar`, a unified `Tooltip`, `Dialog`, etc.) — a documentation deliverable, not new code;
  the underlying gaps it records are still open, just now named and tracked in one place instead
  of undiscoverable.
- **No rollout/phasing plan sequences the remaining ~88 still-`mu::ui::window::CObject`-tier windows against the
  full checklist above.** Work has been pilot-by-pilot, each individually verified. This document is a
  first step toward tracking that, not a substitute for an actual sequenced plan if one is
  wanted.
- ~~`MuPlatform::Initialize()`/`CreatePlatformWindow()`/`GetWindow()`/`Shutdown()`/
  `SetFullscreen()`/`SetMouseGrab()`/`GetDisplaySize()`, and the `IPlatformWindow`/`SDLWindow`
  classes they own, show zero external callers.~~ **Investigated and fixed 2026-09-04.** Root
  cause: `MuPlatform` was scaffolding added in `7f06b3af` (Jul 9) alongside unrelated audio-port
  work, never adopted — `Winmain.cpp`'s `WinMain()` (confirmed, via `Linux/main.cpp` and
  `macOS/main.mm`, the one real cross-platform entry point) has always done its own
  `SDL_InitSubSystem`/`SDL_CreateWindow`/`SDL_SetWindowFullscreen`/`SDL_Quit()` directly, in
  parallel to `MuPlatform`, on every platform. The mechanism itself wasn't unsound — the real
  finding was that `mu::platform::InstallSignalHandlers()` (POSIX crash diagnostics, Story 7.1.2)
  was only ever called from inside the dead `MuPlatform::Initialize()`, so it silently never
  installed on Linux/macOS. Fixed by calling `InstallSignalHandlers()` directly from `WinMain()`
  (`#ifndef _WIN32`, right after `SDL_InitSubSystem` succeeds, preserving the documented ordering)
  and deleting `MuPlatform.cpp`/`.h`, `IPlatformWindow.h`, and `sdl3/SDLWindow.cpp`/`.h` — this
  decouples the still-needed feature from the facade that never got used for its actual purpose.
  Verified via a full incremental build.

## Pilots to revisit when the relevant phase arrives

Every already-shipped window that doesn't fully match the principles doc is **left as-is now,
not rewritten to chase each gap in isolation** (§26 — incremental, don't rewrite wholesale) —
but each specific deviation below is tied to whichever future initiative would naturally fix it,
so it gets folded into that pass instead of being forgotten. Check this list whenever starting
one of the trigger initiatives on the right.

| Window(s) | Known deviation | Revisit when... |
|---|---|---|
| `CMuHelperBar`, `CBuffStrip` (`mu_helper_bar.rcss`, `buff_strip.rcss`) | Repeat their own `font-family`/`font-size`/color literals inline instead of referencing shared tokens (§21) | A design-token/shared-variable layer is built — retrofit these two RCSS files to use it as the worked examples, don't leave them as the last two still on literals. |
| `CMuHelperBar`, `CBuffStrip` and every `CWin`-tier window | Base class/tier boundary (`mu::ui::window::CObject`/`CWin`) unchanged (§12, "Tracked deferral" below) — file location itself was resolved by the `UI/` directory restructure (`docs/newui-legacy-merger.md`, 2026-09-05): `UI/NewUI/HUD/` is now `UI/HUD/` | A base-class/tier restructuring pass is undertaken — not before enough windows exist to know the real target shape (this is the existing tracked deferral, not new). |
| All `CWin`-tier windows, `CMuHelperBar`, `CBuffStrip` | No resolution × UI-scale × theme × drag-state validation matrix has been run against any of them (§25) — verification so far has been ad hoc per window | A validation-matrix/test-plan artifact is built — run it retroactively against every already-migrated window, not just new ones going forward. |
| All draggable migrated windows | Existing drag system's interaction with theme-default-layout + UI-scale (§10–11) has never been explicitly audited | The drag/preference-integration audit (itself an unstarted gap, above) happens — check these windows specifically, don't just audit the mechanism in the abstract. |
| `CBuffStrip` | Right-click-to-cancel not reproduced; tooltip is plain-text instead of the original's per-line-colored rich tooltip (both already documented as deliberate scope cuts in `newui-tier-adapter.md`, not silent gaps) | Right-click-distinct-from-left-click is proven generally in a `data-event-click` binding, or the three non-unified tooltip mechanisms (§12) get consolidated — whichever comes first. |
| `CMainFrameWindow` (`RenderLeftFrame()`/`RenderCenterFrame()`, `MainFrameWindow.cpp`) | Modern theme's flat background fill behind the still-legacy 3D-rendered potion/skill icons is drawn in C++ (`RenderColorQuadARGB`), not RCSS. **The paint-order reason is legitimate** — RmlUi always composites its whole document as the frame's last pass, after those icons already rendered, so an RmlUi-drawn fill in that screen region would always paint *over* them, not behind — the same reason `#item_slots`/`#skill_slots` are border-only in RmlUi, never filled. ~~Gated on the literal string `GetActiveThemeName() == "modern"` (§30 violation — a third theme wanting the same treatment silently wouldn't get it)~~ — **fixed 2026-09-04**: now gated on `UI::RmlBridge::ThemeProvidesOwnIconChrome()`, a declared theme capability (`themes/modern/theme.ini`). The border lines that used to live alongside this same fill were **not** similarly exempt — moved to RmlUi (`#gauge_frame`), since a thin outline has no such paint-order constraint. The skill-hotkey-number subscript and the gauge current/max text are both fully retired from C++ (`GetHotKeySlotNumber()`/`hp_current_text` etc. — pure theme-agnostic data, each theme's own markup decides what to show). | Icon/box-frame art for the skill grid/pet row stayed legacy 2D too (`RenderSkillIcon()`'s atlas lookup is too irregular — mixed 8/12-column addressing, a separate master-level atlas — to port blind without a way to visually verify against the real decoded `.OZJ` textures). Retires once a separately-scoped icon-atlas port lands **or** the render-ordering investigation ("Known gaps") finds interleaving is possible, whichever comes first. |
| `CSkillList::RenderCurrentSkillAndHotSkillList()` (`MainFrameWindow.cpp`, still fully legacy) | The selected-skill-slot highlight. **Same paint-order reasoning as the row above** (modern's RmlUi `.selected` outline always paints on top of it since RmlUi composites last, so drawing the legacy `IMAGE_SKILLBOX_USE` sprite unconditionally would double up the highlight for modern; legacy genuinely wants the real sprite, which has no RmlUi asset equivalent ported yet) — legitimate reason. ~~Gated on `GetActiveThemeName() != "modern"` (§30 violation, same as the row above)~~ — **fixed 2026-09-04**, same `ThemeProvidesOwnIconChrome()` capability as the row above (inverted). Same exception class as the row above, not a separate issue; the expanded grid's own box-frame draw (`Render()`, a distinct call site) got the identical treatment for the same reason — modern's grid cells use a plain CSS border (`.skill-cell`, `main_frame.rcss`) instead of `IMAGE_SKILLBOX`/`IMAGE_SKILLBOX_USE`; legacy keeps the real sprite for both call sites. | Same icon-atlas-port (or render-ordering) follow-up as the row above retires this. |
| `main_frame.rml` (both themes) | Two independently-maintained RML files, not the one-shared-RML-per-window pattern every other migrated window uses — `theming-and-modding.md`'s "Forking a theme's RML" section documents why and the criteria for when this is legitimate. The two files' shared ids/classes/bindings require hand-sync, called out in each file's own header comment — see "Known gaps" for the drift-check tooling this still doesn't have. | Either a cleaner RCSS-only structural fix is found and one file retires, or this is accepted long-term and the same criteria get applied consistently if another window ever needs it — not before a second real case shows up. |
| `CMainFrameWindow` (`main_frame.rcss`, both themes — HP/MP/AG/SD/EXP bars + 5 corner buttons) | `UI::Scaling::BottomHudScale()`/`CappedUniformScale()` (`UITransform.cpp`) fold `GameConfig::GetUIScalePercent()` in as a post-clamp multiplier, applied in the shared function itself so every caller codebase-wide (RmlUi bars/buttons/exp via `bars_scale`, the still-legacy chrome render, 3D potion-icon placement, and potion/skill click hit-testing) moves together automatically. Also folds `UI::Scaling::GetWindowContentScale()` (OS display-scale/pixel-density factor) into RmlUi's own `dp` ratio (`RmlUiRuntime.cpp`'s `ApplyUIScale()`) — **confirmed 2026-09-07 on a 125%-scaled display; see `layout-and-scaling.md`.** `main_frame.rcss` still deliberately uses `px`, not `dp`, throughout, tracking `bars_scale` exactly instead of being scaled a second time. | The `UIScalePercent` half needs verifying by actually using a potion/skill at more than one `UIScalePercent` value *and* resolution, not just a visual check. Phase 3 (item hotkeys → real RmlUi) landing, plus a follow-up icon-atlas port for the Phase 2 skill grid/pet row's still-legacy icon art (see the two rows above), still eventually retires `BottomHudScale` from this window entirely in favor of the branch's normal fixed-`dp`/`UIScalePercent` policy. |
| `CMyInventory` (equipment paperdoll — `RenderEquippedItem()`, still fully native) | Background sprite, durability tint, and drag-compatibility highlight all paint *behind* the equipped item's live 3D icon today (native paint order); RmlUi's main context always composites last, so a straight port would paint them *in front of* instead — a real regression, not a straight port (Stage 2 was scoped, investigated, and deliberately skipped for this reason — see "What's migrated" above). | A background-context consolidation pass makes this mechanism reliable enough to trust with more per-frame-varying, class-conditional content, **or** the equipment grid gets its own future chrome pass anyway and folds this in at the same time — whichever comes first. If pursued alone, the static background sprite (no gameplay-state binding) is the only piece with a reasonable cost/value ratio on its own. |
| `CMyInventory` (`my_inventory.rcss`, legacy theme only) | The 4 corner buttons (RmlUi, always renders last) can end up on top of `CInventoryCtrl`'s native item tooltip when a bottom-row item's tooltip extends into the button strip — before Stage 1 both were native, ordinary same-frame paint order put the tooltip on top. Confirmed cosmetic, not functional; user explicitly deferred it. | Either the tooltip is rerouted through the post-RmlUi seam (shared infra, also used by Trade/Storage/Shop — too broad for this window alone), or a future grid-chrome pass makes the tooltip an RmlUi element too, resolving it for free via DOM order. |
| ~~All modern-theme `.rcss` files~~ | **Superseded 2026-09-10**: the entire modern-theme token layer was renamed and revalued a second time (cool-steel → blackened-iron/dark-forged-metal, a real design-system consolidation, not just a value refresh — see `modern-theme-visual-direction.md`'s "Second generation" note). Real duplication was also consolidated: `login.rcss`'s own-copy `.btn`/`.btn-ok`/`.checkbox-box` and `login_main.rcss`/`char_sel_main.rcss`'s independent `.btn-icon` copies were deleted in favor of `base.rcss`'s shared versions. New shared primitives added: `.btn-icon`, structured tooltip BEM classes, `.slot`/`.slot--filled`/`.slot--selected`. HUD gauge colors promoted from literal hex to `resource-hp`/`-mp`/`-sd`/`-ag` tokens (layout unchanged — see the next row). | Resolved — no further action, unless the tokens change again. |
| HUD circular glass-orb + wrapping arc gauges (reference visual study, not yet built) | The 2026-09-10 iron-palette migration deliberately retinted `main_frame.rcss`'s existing rectangular HP/MP/AG/SD bars rather than rebuilding them as circular orbs/arcs — that's a structural rebuild (new markup, new `CMainFrameWindow` C++ binding shape, new tooltip anchors, interacts with `main_frame_bg.rcss`'s paint-order mechanism and `BottomHudScale()`), not a retint, and touches live combat UI. Two RmlUi-native techniques were confirmed viable for it (`<progress direction="clockwise">` for the arcs via real octant geometry, layered `radial-gradient` for the orb liquid) but not used yet. | A dedicated, focused pass scoped just to this, once explicitly prioritized — see `modern-theme-visual-direction.md`'s "Known follow-up" section. |

## Tracked deferral: C++ adapter classes still on the `mu::ui::window::CObject` tier

Both `mu::ui::window::CObject`-tier pilots (`CMuHelperBar`, `CBuffStrip`) were renamed at port time — class
name and every `INTERFACE_*`/`CSystem` member/accessor/macro referencing them — dropping
their legacy-tier names (§12). What's still deferred: the `mu::ui::window::CObject` base class/tier boundary
itself, and collapsing the `INTERFACE_*`-keyed lookup + `g_p*` macro pattern into something that
doesn't require a per-window case in a shared table. (The physical file location half of this —
`UI/NewUI/HUD/` — was resolved separately by the `UI/` directory restructure,
`docs/newui-legacy-merger.md`, 2026-09-05: that folder no longer exists, its contents are now
`UI/HUD/`, a pure move with no base-class/tier change.) The base-class/tier boundary and
`INTERFACE_*` pattern are structural — they touch the other ~88 still-unported `mu::ui::window::CObject`
windows' shared machinery, not just the pilots so far — and stay premature with only 2 data
points. Revisit once more of those windows are ported to RmlUi and the real shape of a unified
base class is visible from real examples.

## Tracked deferral: `CMainFrameWindow`'s own class rename — naming half resolved, file-split half still open

A distinct deferral from the one above — different reasoning, don't conflate the two.

`CMuHelperBar`/`CBuffStrip` were each renamed at port time (class name and every referencing
`INTERFACE_*`/`CSystem` member/accessor/macro), per the checklist above and
[`newui-tier-adapter.md`](newui-tier-adapter.md)'s Naming section. `CNewUIMainFrameWindow` (Phase 1
of its own 3-phase pilot, `main_frame.rml`/`.rcss`) was originally **not** renamed when ported —
the plan was to hold that rename until Phase 3 landed, so the file's still-legacy classes
wouldn't sit mismatched against an already-renamed one for however long Phase 2/3 took.

**That plan was overtaken by `docs/newui-legacy-merger.md`'s Phase 5 (2026-09-05)**: its
mechanical, repo-wide prefix-drop renamed every `CNewUI*`/`INewUI*` identifier in the whole tier in
one blanket pass, with no per-file carve-out for this deferral — so `CNewUIMainFrameWindow`,
`CNewUISkillList`, and `CNewUIItemHotKey` all became `CMainFrameWindow`/`CSkillList`/`CItemHotKey`
together, incidentally, alongside the ~88 other windows' renames. The naming mismatch this
deferral was protecting against never actually happens now — all three names moved in the same
commit. **What's still genuinely open, unrelated to naming**: `MainFrameWindow.cpp/.h` still welds
three classes together — `CMainFrameWindow` (ported, Phase 1), `CSkillList` (still fully legacy,
Phase 2), `CItemHotKey` (still fully legacy, Phase 3) — one file serving three different pilot
phases. Whether that one-file-three-classes shape is itself worth splitting (e.g. once Phase 3
lands and all three are ported) is a real, still-unmade decision; revisit it then, but it's a
file-organization question now, not a naming one.

## Tracked deferral: `CUIControl` family (`UIControls.h`) full retirement

Not a permanent third toolkit alongside RmlUi and `mu::ui::window` — a fully enumerable, closeable
checklist (`ui-target-architecture.md` item 17's "concrete instance"). Found and scoped
2026-09-13 while investigating whether porting Friend/Mail would let this family retire. It
wouldn't — Friend/Mail (`CUIWindowMgr`/`CUIBaseWindow`, `UI/Party/UIWindows.cpp`) is only one of
four independent pieces still keeping this file alive:

1. **`CUITextInputBox`** — permanent until RmlUi gets native `<input>`/`<textarea>` (Section E's
   Type-2 companion; IME composition through RmlUi's DOM is the open design question, no target
   date). Not part of this checklist's "close it out" scope — this piece stays regardless.
2. **`CUITextListBox<T>`** (~18 subclasses in `UIControls.h`) — no rule named this class before
   2026-09-13 (only `CUIButton` was named), which is exactly why it kept gaining consumers even on
   windows already on `mu::ui::window::CObject`. Confirmed live consumers found this session:
   - `CGuildInfoWindow` (`Guild/GuildInfoWindow.h`) — `CUINewGuildMemberListBox`
   - `CMixInventory` (`UI/Inventory/MixInventory.h`) — `CUISocketListBox`, `CUIUnmixgemList`
   - `CInGameShop` (`GameShop/InGameShop.h`) — `CUIInGameShopListBox`, `CUIBuyingListBox`,
     `CUIPackCheckBuyingListBox`
   - `QuestProgress.h`/`QuestProgressByEtc.h` — `CUICurQuestListBox`/`CUIQuestContentsListBox`
     (the same two `CMyQuestInfoWindow` already ported off; natural next targets, same pattern)
   - Guild/chat/letter/socket variants (`CUIGuildListBox`, `CUISimpleChatListBox`,
     `CUIChatPalListBox`, `CUIWindowListBox`, `CUILetterListBox`/`CUILetterTextListBox`,
     `CUIGuildNoticeListBox`, `CUIUnionGuildListBox`, `CUIExtraItemListBox`,
     `CUIBCDeclareGuildListBox`/`CUIBCGuildListBox`, `CUIMoveCommandListBox`) — not yet traced to
     live call sites individually; treat as live until checked, same discipline as the confirmed
     ones above.

   The replacement pattern is proven, not speculative: RmlUi's `data-for` binding, already used by
   `CBuffStrip` and by `CMyQuestInfoWindow`'s own port off two of these classes. Each remaining
   subclass is an independent, same-shape port — see `ui-target-architecture.md` item 8b / Rule 11.
3. **`CUIButton`** — down to one known live cluster now. `CUIPopup` (`g_pUIPopup`, `UI/Dialogs/
   UIPopup.h`) had its `POPUP_OK`/`POPUP_YESNO` call sites (the ones duplicating
   `CGenericConfirmDialog`'s job) ported off it 2026-09-13 (see "Tracked deferral:
   `CommonMessageBox`/`CustomMessageBox`" below) — but `CUIPopup` itself is **not** retired: one
   live `POPUP_CUSTOM` call site remains (`UIGuildInfo.cpp`'s "Appoint" sub-guild-master/
   battle-master picker, a bespoke multi-option menu out of `CGenericConfirmDialog`'s scope), so
   its 4 `CUIButton` members (`m_OkButton`/`m_CancelButton`/`m_YesButton`/`m_NoButton`) are now
   unreachable dead weight but the class itself stays. The other cluster,
   `CUIGuildInfo`/`CUIGuildMaster` (`Guild/UIGuildInfo.h`/`UIGuildMaster.h`), is suspected dead —
   see below (note: `CUIGuildMaster`'s dead `ReceiveGuildRelationShip`/`CloseMyPopup`/two popup-id
   members were already deleted 2026-09-13 as part of the `CUIPopup` port, superseded by
   `CGuildInfoWindow`'s own port earlier — the rest of the "suspected dead" verification below
   still applies to what's left of both classes).
4. **`CUIGuildInfo`/`CUIGuildMaster`** (`CUIControl`-rooted standalone windows, distinct from the
   live `CGuildInfoWindow`/`CGuildMakeWindow` pair on `mu::ui::window::CObject`) — a full-codebase
   grep found **zero instantiations of either class anywhere** (no `new`, no member declaration in
   any other type). Strong circumstantial evidence of dead code superseded by
   `CGuildInfoWindow`/`CGuildMakeWindow`, same shape as `CWin`/`::CButton`/`CSlider` before they
   were deleted — but not yet given that same exhaustive verification pass. Do that verification
   before deleting anything.

**Related finding, same investigation**: `CUIManager`/`g_pUIManager` (`UI/Core/UIManager.h/.cpp`)
looks like a live top-level manager parallel to `mu::ui::window::CManager` — it isn't. Its
`Render()` and `UpdateInput()` method bodies are both literally empty. Its `MUTEX_*` enum lists
~30 interfaces (including `MUTEX_TRADE`/`MUTEX_STORAGE`/`MUTEX_GUILDINFO`/`MUTEX_NPCSHOP` — windows
long since migrated to `mu::ui::window::CManager`) but `Open()`/`IsOpen()` only actually implement
4 of them (`MUTEX_INVENTORY`, `MUTEX_PERSONALSHOPSALE`, `MUTEX_PERSONALSHOPPURCHASE`,
`MUTEX_SERVERDIVISION`); everything else falls through to `default: return false`. What's actually
still real: it constructs/owns `g_pUIPopup`/`g_pUIGateKeeper`/jewel-harmony/item-add-option-info as
globals, and `IsInputEnable()` is a genuinely still-consulted query. Worth knowing mainly so a
future session doesn't mistake the `MUTEX_*` enum for a live, comprehensive policy layer — most of
it is vestigial. Not in this retirement checklist's scope (it's not `UIControls.h`), but touches
the same investigation and the same `g_pUIPopup` dependency as item 3 above.

## Tracked deferral: `CommonMessageBox`/`CustomMessageBox` family port

A separate native subsystem from `UIControls.h`'s `CUIControl` family tracked above — don't
conflate the two. `UI/Dialogs/CommonMessageBox.h`/`CustomMessageBox.h` together declare 150+
classes, 100% native, confirmed by a full-codebase inventory (2026-09-13). The primitive to port
them onto now exists (`CGenericConfirmDialog`, see "What's migrated" above) and is proven on 3 of
them. The full per-class worklist (call sites, FITS/DOESNT_FIT classification, batching groups,
dead-code candidates) lives in `dialog-migration-plan.md` — resume there, not from memory, in a
future session. Summary by category (counts approximate, from the inventory pass, not re-verified
per class):

- **`CommonMessageBox.h`** — ~75 classes, ~64 `TMsgBoxLayout<CCommonMessageBox>` (or
  `<C3DItemCommonMsgBox>`/`<CFenrirRepairMsgBox>`) feature-specific confirms spanning guild/quest/
  trade/duel/castle-siege/events/gambling. **This entire file is now fully handled** (2026-09-13):
  63 classes ported-and-deleted or confirmed-dead-and-deleted (3 proof-pass + 7 Guild + 10
  Trade/shop/inventory + 25 Network/server + 14 Siege/castle/CryWolf + 4 dead-code), plus 5 of the 6
  `C3DItemCommonMsgBox`-based classes now ported onto the primitive's `item3D` field
  (`CHighValueItemCheckMsgBoxLayout` — 2 call sites, `CUseFruitMsgBoxLayout`,
  `CUsePartChargeFruitMsgBoxLayout`, `CPersonalShopItemBuyMsgBoxLayout`, `CGambleBuyMsgBoxLayout`).
  `CPersonalShopItemValueCheckMsgBoxLayout` stays native — it needs a numeric price-value field its
  `CTextInputMsgBox` caller sets via `SetItemValue()`, which isn't designed yet. See
  `dialog-migration-plan.md` for the per-class worklist. `CustomMessageBox.h` (~76 more classes,
  separate file) is next.
- **`GenericDialogConfig` extensions** — **done** (2026-09-13): optional `title`, `severity`
  (Normal/Warning/Error, look-and-feel only), `input` (`InputField::Mode::Text`/`NumericKeypad`),
  `progress` (duration-only auto-close), and `item3D` (an `ITEM` snapshot). One struct, not five
  sibling classes.
  - **Correction, 2026-09-13**: the first version rendered `item3D` via a plain `I3DRenderObj`/
    `C3DRenderMng` registration (`Render3D()`), like every other legacy 3D icon. In-game testing
    (5 classes ported onto `item3D`, see `CommonMessageBox.h` entry below) showed the item never
    appeared — that path renders through `CManager::Render()`, which always finishes *before*
    RmlUi's main-context composite, so the dialog's own opaque `#panel` background painted over it
    every frame regardless of z-order.
  - **Reverted, 2026-09-14**: tried moving `item3D` off `I3DRenderObj`/`Render3D()` onto a manually
    invoked `RenderItem3DOnTop()` called from Winmain.cpp's `SetPostRmlUiCallback` instead (the
    established "legacy content after RmlUi" seam `CMsgWin`/`CCharMakeWin`/`CLoginWin` already use
    for native text overlays, just the first time asked to carry 3D content) — **twice**, and both
    attempts crashed on dialog dismiss. The first crash's cause was found and genuinely fixed at the
    renderer level (`MuRendererSDLGpu.cpp`'s post-RmlUi replay pass only re-staged vertex data,
    leaving a skinned draw's bone-matrix buffer unstaged/undersized while `ReplayDrawCommand`'s
    `boneDataReady` guard stayed stale-true — an out-of-bounds GPU read; fixed by reusing
    `StageDeferredGpuData()`, which re-stages vertex/bone/strip-index/texture data together and
    returns a correct `boneDataReady` — this fix is real and stays). That restored visibility and
    (combined with a separate `PanelTranslateCorrection()` fix, below) correct position, but the
    dismiss-time crash persisted regardless — a second, still-unidentified bug in the same seam.
    Enabling SDL_GPU validation (Debug config) to localize it hit a *different* pre-existing
    validation failure at startup, unrelated to this feature, blocking that route. Reverted back to
    `I3DRenderObj`/`Render3D()` (stable; `item3D` renders correctly-positioned but behind the panel)
    rather than ship a crash. `input`'s `Mode::Text` widget keeps its own `RenderTextOnTop()`
    post-RmlUi method (pure 2D, never implicated in the crash, matches the already-proven pattern).
    See `theming-and-modding.md`'s "RmlUi rendering strictly last in the frame" entry for the full
    writeup.
  - **Fixed for real, 2026-09-14**: the fg/bg RmlUi document split `CMainFrameWindow`/the
    inventory-family windows already prove for their own live 3D icons. `generic_confirm_dialog`'s
    own panel background art (the modern gradient/shell-edge/groove/content-well recipe, or
    legacy's `newui_msgbox_*` sprite composite) moved wholesale into a new
    `generic_confirm_dialog_bg.rml`/`.rcss` per theme. The original `generic_confirm_dialog.rml`/
    `.rcss` document is now paint-*less* where the background used to be — same `#panel` id/size/
    centering, just nothing left to cover the item once the background document has already
    painted it earlier the same frame. Needed **no changes at all** to `Render3D()`/
    `PanelTranslateCorrection()`/`I3DRenderObj` registration or the post-RmlUi seam — only where the
    panel's own background art paints from, and (see below) exactly when.
    - **Centering bug, 2026-09-14**: the first cut of `generic_confirm_dialog_bg.rcss` didn't link
      `base.rcss` (per convention, no `*_bg.rml` does), but `base.rcss` is where `body { width:100%;
      height:100%; }` comes from — without it, `#panel`'s `left:50%;top:50%` had a zero-sized
      containing block to resolve against, collapsing the whole background panel to the screen's
      upper-left corner. Fixed by adding that one `body` rule explicitly to both themes' `_bg.rcss`
      (not by linking `base.rcss`, which would pull in a lot more than needed).
    - **Cross-window bleed-through bug, 2026-09-14**: the first cut also loaded the new bg doc into
      the SHARED `RmlUiRuntime::GetBackgroundContext()` (the same one `CNPCShop`/every inventory-
      family window's own bg doc uses), driven by the existing `RenderBackgroundLayer()` hook (fired
      once, globally, before the very first visible window/camera each frame). That fixed the
      standalone case, but broke the moment this dialog opened over another bg-doc window with
      native foreground content: since `RenderBackgroundLayer()` renders ALL currently-visible bg
      docs together, once, strictly before EVERY window's own 2D `Render()` this frame — not just
      this dialog's — `CNPCShop`'s own inventory-slot icons (drawn later, in its own `Render()`)
      always painted over BOTH bg docs regardless of their relative order within that shared
      context, bleeding through the dialog's panel wherever they geometrically overlapped. This is
      exactly the case that matters most for `item3D` (sell-to-shop/gamble-buy confirms are almost
      always shown over an open shop window). Tried reasserting the dialog's own bg doc's stacking
      order every frame via `ElementDocument::PullToFront()` first — insufficient, since
      `PullToFront()` only reorders documents *within* the one shared context; it can't make that
      context's single global render pass happen *after* another window's own `Render()`.
      **Actually fixed** by giving the dialog's own bg doc a dedicated THIRD context
      (`RmlUiRuntime::GetDialogBackgroundContext()`), rendered by a separately-guarded
      `RmlUiRuntime::RenderDialogBackgroundLayer()` that `CManager::Render()` fires at a *different*
      point than `RenderBackgroundLayer()`: right before the first visible object whose
      `GetLayerDepth()` reaches the shared 3D camera's own z-order (`INFORMATION_CAMERA_Z_ORDER`,
      `Window3DRenderMng.h` — the same z-order `item3D` itself renders through). Since
      `CManager::Render()` sorts every registered object by `GetLayerDepth()` (`CNPCShop` = 2.5f,
      the shared 3D camera = 10.9f, this dialog's own 2D `Render()` = 60.0f) and calls each one's
      `Render()` in that order, "right before the object at/past 10.9f" is guaranteed to be
      strictly after every ordinary window's own `Render()` this frame and strictly before
      `item3D` draws. Deliberately triggered from `CManager::Render()`'s own loop, NOT from inside
      `Render3D()` itself — `C3DCamera::Render()` pushes a legacy GL matrix stack and enables depth
      test/mask before looping over every registered object's `Render3D()`, and recording an RmlUi
      render pass from inside that block is exactly the kind of mid-frame GPU-state interleaving
      that crashed the `SetPostRmlUiCallback` attempts above; `CManager::Render()`'s own loop,
      before any `(*vi)->Render()` call, is the same safe, pre-matrix-stack position
      `RenderBackgroundLayer()` itself already uses.
    - Simpler than both reference implementations either way: this dialog's `#panel` centers via
      plain CSS (`.center-both`), never a per-frame C++-computed position, so its background
      document needs no `RmlModelBinder` and no position-sync code at all (`CMainFrameWindow`/
      `CNPCShop`'s own bg docs both need a small one, since their content is anchored to an
      adjustable HUD-band/inventory-window position).
  - **`PanelTranslateCorrection()`, 2026-09-14**: separately, `item3D`/`input`'s anchor position was
    found to be wrong (rendering outside the panel) because `#panel`'s `.center-both` centering uses
    `transform: translate(-50%,-50%)`, and RmlUi's `GetAbsoluteOffset()` doesn't apply CSS
    `transform` at any level — every descendant of `#panel` reported its position as if the panel
    were still sitting at its untranslated `left:50%;top:50%` spot. Fixed by subtracting half of
    `#panel`'s own box size from the raw offset before converting to reference space; this fix is
    independent of the `Render3D()`/`RenderItem3DOnTop()` back-and-forth above and stays either way.
  - No consuming dialog class has been ported onto `title`/`progress` yet (`item3D` has 5,
    `input.Mode::Text` has 9 and `input.Mode::NumericKeypad` has 3 as of 2026-09-14 -- see
    `dialog-migration-plan.md`'s "Text input"/"Numeric keypad" entries) -- every unconsumed field
    still defaults to unset, so pre-existing call sites are unaffected. `Mode::NumericKeypad`
    builds clean but is not yet in-game-tested (unlike `Mode::Text`, which was tested and needed a
    text-color fix -- treat this the same way until confirmed). See `dialog-migration-plan.md`'s
    own entry for what's deliberately out of scope (the older `g_iChatInputType == 0` input path;
    input-row/keypad/progress-bar layout geometry not yet visually verified against a real
    consumer). Porting `input.Mode::Text` also surfaced a real primitive gap, now closed:
    `CGenericConfirmDialog::KeepOpen()`, letting `onPrimary`/`onSecondary` veto a click's
    `Resolve()` (native's own `CALLBACK_CONTINUE` convention for invalid input) -- see that entry
    for the full writeup. In-game-tested and confirmed working (2026-09-14) after fixing one bug
    found only by that testing: the field was invisible at first (opaque-black default text color
    against this dialog's own dark panel, the same gotcha `CharMakeWin.cpp` already hit and
    documented) -- fixed with the same light-cream text color `LoginWin.cpp`/`CharMakeWin.cpp` use,
    plus a visible dark recessed background fill since this dialog's anchor has no native sprite
    frame of its own to give the field a visible affordance.
- **`CustomMessageBox.h`** — ~76 classes on the same pattern. **2 done** (2026-09-13):
  `CDialogMsgBoxLayout`/`CDialogMsgBox` (the one near-miss that already fit as-is) and
  `CreateOkMessageBox()` (a third, previously-untracked OK-only helper, ~90 call sites migrated via
  one function-body change). The rest: keypad/numeric-entry boxes and text-input boxes (both now
  unblocked by the extensions above, not yet ported), fruit/gem-integration confirms, the in-game
  system-menu box (distinct from the already-ported `CSysMenuWin`), event result screens (Blood
  Castle/Devil Square/Chaos Castle), duel challenge/result, progress-bar modals (also now unblocked),
  and ~46 `T*MsgBoxLayout<...>` wrappers.
- **`CUIPopup`** (`UI/Dialogs/UIPopup.h`, `g_pUIPopup`) — **done** (2026-09-13). Every real
  `POPUP_OK`/`POPUP_YESNO` call site (9 live across `Guild/UIGuildInfo.cpp`,
  `Guild/UIGuildMaster.cpp`, `Network/Server/WSclient.cpp`) ported to `CGenericConfirmDialog`; one
  dead `POPUP_YESNO` site (`CUIGuildMaster::ReceiveGuildRelationShip`, superseded by
  `CGuildInfoWindow`'s own earlier port) deleted outright. `CUIPopup` itself is **not** deleted —
  one live `POPUP_CUSTOM` site (`UIGuildInfo.cpp`'s "Appoint" picker) is a bespoke multi-option menu
  out of scope, same as the multi-option `CustomMessageBox.h` classes below. See
  `dialog-migration-plan.md` for the full per-call-site breakdown.
- **`GameShop/MsgBoxIGS*.h`** — 10 more `CMessageBoxBase` subclasses, cash-shop flows (buy confirm,
  buy-package with a live 3D item preview, buy-select-item, generic OK/Cancel, delete-item confirm,
  gift-storage-info, send-gift + confirm, storage-item-info, use-buff/use-item confirm). Both the
  title field and the 3D-item-preview hybrid extension it needs now exist on `CGenericConfirmDialog`
  (see above) — unblocked, not yet ported.
- **Misc**: `CHelpWindow`, `CWindowMenu`, `CChatCommandWindow` (`UI/Dialogs/`) are dialog-shaped but
  don't fit the confirm-box mold at all (help overlay, per-window popup menu, command picker) —
  out of `CGenericConfirmDialog`'s scope entirely, would need their own primitives if ported.

Not blocked on anything — each of the ~137 remaining classes is an independent, same-shape port
(config data, not new code) for the plain-text OK/OK-Cancel ones; the keypad/progress/3D-preview
variants need their own scoped extension to `GenericDialogConfig` first, proven the same way this
pass proved the plain-text shape before being applied broadly.

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
