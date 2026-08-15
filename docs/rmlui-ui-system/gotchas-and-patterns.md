# Gotchas & Bug Catalog

Real bugs found during the Phase 1 pilot migration (the login screen), with root cause and fix.
Read this before migrating the next window — several of these are non-obvious traps that will
reproduce identically on any new RmlUi window built the same way, and cost real debugging time
once already.

Ordered roughly by when a future migration is likely to hit them: build/integration issues first,
then rendering, then input, then theming-specific.

---

## Build & integration

### `RMLUI_BACKEND "none"` is not a valid CMake value

RmlUi's own `CMakeLists.txt` validates this cache variable unconditionally, even when
`RMLUI_SAMPLES=OFF` (it only actually selects which backend RmlUi's *samples* build against).
**Fix**: use `"auto"` instead — a no-op since samples are disabled, but a value RmlUi's own
validation accepts.

### FreeType not found

RmlUi's default font engine (`RMLUI_FONT_ENGINE=freetype`) requires FreeType via `find_package`,
and none was installed. **Fix**: vendored `src/ThirdParty/freetype` as a git submodule with
`add_library(Freetype::Freetype ALIAS freetype)`, satisfying RmlUi's
`if(NOT TARGET Freetype::Freetype)` check regardless of how that target got created.

### Wrong CMake target name silently accepted

`target_link_libraries(MuClient PUBLIC RmlCore)` referenced a target that doesn't exist — the real
one is `rmlui_core`, aliased `RmlUi::Core`. **CMake accepted the wrong bare name with no error at
configure time** (it doesn't validate that a linked "library" name actually resolves to anything
until link time), and the failure only surfaced once every RmlUi-including source file couldn't
find `RmlUi/Core/Context.h` (the nonexistent `RmlCore` target propagated no include paths).
**Fix**: use `RmlUi::Core`. **Lesson**: a `target_link_libraries` typo on a nonexistent target
name is a silent no-op in CMake, not a configure error — if new RmlUi-adjacent link errors show up
as missing headers rather than missing symbols, check the target name first.

### Legacy macro collision: `Vector4`/`DotProduct`

`Core/Math/ZzzMathLib.h` used to `#define` `Vector4(a,b,c,d,target)` and `DotProduct(x,y)` as
legacy C-style macros for this engine's own `vec3_t`/`vec4_t` math. Both names collided textually
with `Rml::Vector4` (a class) and `Rml::Vector3/Vector4::DotProduct()` (methods) — macros ignore
C++ namespaces entirely. Confirmed via raw `/P` preprocessor output
(`warning C4003: not enough arguments for function-like macro invocation`), not guessed.

**First fix** (later replaced): a `push_macro`/`undef`/`pop_macro` guard pair
(`RmlUiMacroGuardBegin.h`/`End.h`, deliberately no include guards so they'd be reprocessed at
every include site) wrapped around every direct `#include <RmlUi/...>` line across 7 files.
Functional, but a growing tax on every future RmlUi include site, and easy to forget on a new one.

**Real fix**: renamed the macros — `Vector4` → `Vector4Set`, `DotProduct` → `VectorDotProduct`
(matching `ZzzMathLib.h`'s own `VectorXxx` naming convention) — after first confirming the actual
blast radius (only 3 `Vector4(...)` call sites, all in `ZzzBMD.cpp`, and ~45 `DotProduct(...)`
sites across 10 files; no unrelated identically-named symbol collided anywhere in `src/source`).
Updated every real call site, then deleted both guard files and their `#include` wrapping — no
file needs to guard an RmlUi include anymore, because the collision no longer exists.

**A mechanical mistake caught before it shipped**: the first attempt at this rename used
PowerShell (`Get-Content -Raw` + `-replace` + `[System.IO.File]::WriteAllText()`, no explicit
encoding) to batch-edit all 10 files. This silently mangled non-ASCII characters already present
in those files (em dashes, arrows in comments) into mojibake (`—` → `â€"`) — caught by inspecting
`git diff` before committing (every line in the touched region showed as changed, not just the
macro-name lines — a tell that something broader than intended had changed) rather than assuming
a mechanical find-and-replace was safe. Reverted and redid the rename file-by-file with an
encoding-safe edit tool. **Lesson**: a raw PowerShell read/write round-trip on a file whose actual
encoding you haven't checked can silently corrupt non-ASCII content — verify with a diff before
trusting a batch text edit, especially across many files at once.

### RHI dispatch-shim layer missed

`RHI_GL.cpp` only implements the `RHI_GL_Impl` namespace; the real `namespace RHI { ... }`
functions that satisfy `RHI.h`'s public declarations live in a *separate* file, `RHI.cpp`, as thin
forwarding calls. The first pass adding `RHI::SetScissorEnabled`/`SetScissorRect` (needed for
RmlUi's `EnableScissorRegion`/`SetScissorRegion`) only added the `RHI_GL_Impl` half and missed the
forwarding pair in `RHI.cpp` — a straightforward omission once the two-file split is understood,
but easy to repeat on the next RHI addition if it isn't.

---

## Rendering

### GlobalUBO Model matrix leak

**Symptom**: after the RmlUi migration's blend-mode and mouse-capture fixes landed, legacy UI
elements and the clickable/"responsive" screen area appeared to have shifted toward the
upper-right of the window, while RmlUi's own elements rendered in the correct position. The 3D
tour-camera background also appeared cropped to its lower-left quadrant.

**Root cause**: `RmlUiRenderInterface::RenderGeometry` calls `GlobalUBO::SetModel(origin, scale)`
once per compiled-geometry draw (translating it to its element's screen position — necessary and
correct). `RmlUiRuntime::Render()` saved and restored `GlobalUBO`'s Proj and View around the whole
render pass, but never did the same for Model. After the frame's *last* RmlUi draw call, Model was
left holding that element's translation (e.g. the login panel's OK button origin), with nothing to
reset it. Legacy 2D/3D rendering (`BeginBitmap`/`BeginOpengl`-based sprite, text, and world draws)
never sets Model itself — it assumes Model is identity, exactly as it always was before RmlUi
existed. Every legacy draw issued after RmlUi's render pass inherited that leftover translation.

**Fix**: bracket the whole render pass with `GlobalUBO::PushModel()`/`PopModel()` — a stack-based
save/restore that already existed in `GlobalUBO` for exactly this shape, just never used by
`RmlUiRuntime::Render()` — the same way the existing `savedProj`/`savedView` locals already
bracket Proj/View.

**How it was actually found**: not guessed. A one-shot diagnostic log confirmed `WindowWidth`/
`WindowHeight` (the "stale dimension" theory) were consistent and correct, ruling that out first.
The real clue was that RmlUi's *own* elements rendered correctly while only legacy content shifted
— which narrowed the search to "what does RmlUi mutate that legacy code assumes stays constant,"
directly pointing at `GlobalUBO`'s per-draw Model mutation with no matching restore.

### Blend mode never restored

**Symptom**: the 3D background scene "did not display well" specifically on the continuously
re-rendering login screen (not the one-shot loading screen).

**Root cause**: `RmlUiRenderInterface::RenderGeometry` calls `RHI::SetBlendMode(RHI::BlendMode::Blend3)`
for every draw (cull off, depth-write off, alpha blend on), and nothing restored it afterward.
`BeginOpengl()` (the per-frame 3D setup) was confirmed via direct read to unconditionally reset
`ALPHA_TEST`/`TEXTURE_2D`/`DEPTH_TEST`/`CULL_FACE`/`DEPTH_MASK`/`DEPTH_FUNC`/fog every frame, but
**never touches `GL_BLEND`/`AlphaBlendType`** — confirmed via direct grep, zero hits. The leftover
alpha-blended/no-cull/no-depth-write state from one frame's RmlUi render bled directly into the
next frame's opaque terrain/character rendering, every frame.

**Fix**: `RHI::SetBlendMode(RHI::BlendMode::Opaque)` explicitly at the end of
`RmlUiRuntime::Render()`.

### Render order: content behind an opaque RmlUi panel

**Symptom**: after fixing an unrelated theming bug, the mouse cursor and the login screen's
username/password text both rendered invisibly, seemingly "under" the RmlUi panel.

**Root cause**: RmlUi always renders **last** in the frame (see
[Architecture §3](architecture.md#3-frame-lifecycle--the-render-order-contract) for the full
diagram). The cursor and the legacy `CUITextInputBox` text both used to render much earlier,
inside `RenderInfomation()` (`SceneCommon.cpp`) — called from deep inside `RenderCurrentScene()`,
well before RmlUi's own render call. This was invisible for as long as the login screen's RmlUi
`#panel` had **no background at all** (fully transparent) — whatever rendered underneath stayed
visible no matter the draw order. The moment a theme gave `#panel` an opaque background (the
"modern" theme), the pre-existing ordering issue became visible for the first time. It was never a
new bug the modern theme introduced — just the first content to expose it.

**Fix**: moved both draws to run explicitly *after* `RmlUiRuntime::Render()` in `SceneManager.cpp`'s
`MainScene()`, in their own fresh `BeginBitmap()/EndBitmap()` bracket (the original bracket from
the window's own render pass is already closed by this point — `EndBitmap()` restores `GlobalUBO`'s
Proj/View to the 3D perspective active before `BeginBitmap()`, so the 2D ortho state
`ConvertX`/`ConvertY`-based rendering needs is gone unless re-established). Added
`CLoginWin::RenderTextOnTop()` (called directly, not through `RenderControls()`'s normal virtual
dispatch) to carry just the `CUITextInputBox::Render()` calls that moved.

**Practical rule for the next migrated window**: any legacy content that must stay visible on top
of an RmlUi overlay needs its render call moved to after `RmlUiRuntime::Render()`, in its own
bracket — don't assume "it worked before" proves the ordering is correct, only that nothing opaque
happened to sit on top of it yet.

### `box-shadow` blur renders as a solid white block

**Symptom**: the login screen's "modern" theme's panel, styled with
`box-shadow: 0px 6px 24px rgba(0,0,0,160);`, rendered as a solid, washed-out white rectangle
instead of a dark panel with a soft shadow.

**Root cause**: `box-shadow` with a blur radius ≥ 0.5px doesn't render as plain geometry. RmlUi's
real implementation (`Source/Core/GeometryBoxShadow.cpp`) calls `RenderManager::PushLayer()`,
`CompileFilter("blur", ...)`, and `CompositeLayers(...)` — exactly the "optional advanced
rendering functions" (layers/filters) that `RmlUiRenderInterface` deliberately leaves as no-op
stubs (see [Architecture §2](architecture.md#2-render-interface-why-a-custom-one-and-what-it-doesnt-do)).
This was actually already documented in `RmlUiRenderInterface.h`'s own header comment from the
original groundwork work ("box-shadow-blur are not yet supported") — the bug was adding
`box-shadow` to a theme without that limitation being front of mind, not a newly-discovered engine
gap.

Before concluding this, several other theories were investigated and ruled out with evidence
(don't re-check these on a future "translucent color looks wrong" bug — they're confirmed fine):
RmlUi's `rgba()` parser (`PropertyParserColour.cpp::ParseRGBColour`) takes all four channels as
0-255 integers, not standard CSS 0-1 alpha, so the RCSS itself parses exactly as authored — this
is *not* a syntax gotcha. The premultiplied-alpha un-premultiply math in `CompileGeometry` checked
out algebraically. `EnableAlphaBlend3()` is a genuine `glBlendFunc(GL_SRC_ALPHA,
GL_ONE_MINUS_SRC_ALPHA)`, not accidentally additive.

**Fix**: don't use `box-shadow` with a nonzero blur radius in any theme until layer/filter
compositing is actually implemented in `RmlUiRenderInterface`. `background-color`/`border`/
`border-radius` are all pure geometry and render correctly — they're what should carry a themed
panel's look instead.

---

## Input

### `pointer-events` swallows every click on screen

**Symptom**: clicking the login screen's legacy username/password input boxes never transferred
keyboard focus to them, and (in a related manifestation of the same bug) the OK/Cancel buttons
appeared to respond to `:hover` but not to an actual left click.

**Root cause**: `pointer-events` is an **inherited** RCSS property in RmlUi (default `auto`,
confirmed via `StyleSheetSpecification.cpp`'s `RegisterProperty` call — comparing its `inherited`
flag position against `color` (inherited) vs. `display` (not inherited) in the same file).
`login.rcss`'s `body` rule spans the *whole real window* (`width/height: 100%`, the normal pattern
for a full-screen RmlUi document), not just the visible panel, and never set `pointer-events` — so
it stayed `auto` everywhere on screen. `Context::IsMouseInteracting()` (`Source/Core/Context.cpp`)
returns `true` whenever the current hover target is anything other than RmlUi's root element, and
this engine's SDL input arbitration (`RmlUiRuntime::ProcessSdlEvent`) treats that as "RmlUi handled
this click," skipping legacy handling for the event entirely. Since `body` covers the full window,
*every* click on screen while the document is shown qualified.

**Fix**: `pointer-events: none` on `body`, with `pointer-events: auto` explicitly re-enabled only
on the specific elements that should claim clicks (`.checkbox-row`, `.btn`) — child elements
(`.checkbox-box`, `.checkbox-label`) inherit `auto` from their styled parent, no separate rule
needed for them.

**This is not optional for any future full-window RmlUi document** — any document whose
root/body spans the full window needs this same treatment, or it will silently swallow every click
on screen for legacy-input purposes the moment it's shown. It only surfaced on the login screen
because that's the first (and so far only) full-window RmlUi document with legacy content
underneath it that needs real clicks.

### SDL_CaptureMouse side effect

**Symptom**: after choosing a server/channel (the first real click-driven scene transition),
mouse clicks throughout the game stopped tracking correctly.

**Root cause**: RmlUi's own vendored SDL backend (`RmlSDL::InputEventHandler`,
`ThirdParty/RmlUi/Backends/RmlUi_Platform_SDL.cpp`) calls `SDL_CaptureMouse(true)`/`(false)` on
every mouse button down/up — correct for a sample app that owns the whole window, but this engine
has its own cursor rendering/clip handling (`Winmain.cpp`'s `UpdateCursorClip()`) that was never
designed to expect SDL's capture mode being toggled externally, and every mouse click in the game
(not just clicks on RmlUi elements) was routed through this handler.

**Fix**: handle `SDL_EVENT_MOUSE_BUTTON_DOWN`/`UP` directly in `RmlUiRuntime::ProcessSdlEvent`
(calling `Context::ProcessMouseButtonDown`/`Up` with `RmlSDL::ConvertMouseButton`/
`GetKeyModifierState`, both pure/side-effect-free) instead of delegating those two event types to
the vendored handler. Wheel/key/text events still route through it unchanged.

### Mouse motion `pixel_density` scaling mismatch

**Root cause**: `RmlSDL::InputEventHandler`'s motion case multiplies event coordinates by
`SDL_GetWindowPixelDensity()` — correct for a backend whose `Rml::Context` was created in
DPI-independent points, but this engine's `Rml::Context` is created directly from real-pixel
`WindowWidth`/`WindowHeight`, and its own `HandleMouseMotion()` applies no such scaling. On any
display where pixel density isn't exactly 1.0, this double-applies scaling and drifts RmlUi's
internal cursor position away from the real one (surfaced as "mouse only seems to work in some
screen regions").

**Fix**: handle `SDL_EVENT_MOUSE_MOTION` directly too, feeding unscaled coordinates matching
`HandleMouseMotion`'s own convention. Only wheel/key/text events still route through the vendored
handler.

---

## Data binding

### Data model created after the document that references it

**Symptom**: every `{{binding}}` in the login document (`{{account_label}}`, `{{server_name}}`,
`{{ok_label}}`, etc.) rendered as its own literal source text instead of resolving to the bound
value.

**Root cause**: `CLoginWin::Create()` called `Context::LoadDocument()` *before*
`RmlModelBinder::Create()` (which internally calls `Context::CreateDataModel("login")`). RmlUi
resolves `data-model="login"` and every `{{binding}}` while *parsing* the RML document, not lazily
on first render — the named data model must already exist in the `Context` before
`LoadDocument()`/`LoadDocumentFromMemory()` runs.

**Fix**: create the data model first, capture whether creation succeeded, and only load the
document if it did — guards against loading a document whose bindings can never resolve, rather
than loading it unconditionally and hoping the model shows up in time.

### Missing font — `Rml::LoadFontFace()` never called

**Symptom**: checkbox/button *shapes* rendered correctly, but every text string (labels, button
text, trust warning) was completely blank.

**Root cause**: nothing ever called `Rml::LoadFontFace()` — this is normal, non-crashing RmlUi
behavior when no font face is loaded at all, not an error condition that surfaces any diagnostic.

**Fix**: load the same bundled fonts this engine already ships for its portable GDI-text shim
(`fonts/LiberationSans-{Regular,Bold}.ttf`) in `RmlUiRuntime::Create()`, marking the regular weight
as a fallback face. Also fix any RCSS `font-family` that doesn't match a loaded family name exactly
(`"Tahoma"` was never loaded — even with *some* font loaded elsewhere, an RCSS rule naming an
unloaded family still renders no text for that element).

---

## Theming & modding

### Custom theme images silently require a proprietary format, not plain PNG/JPG

**Not a bug that was fixed** — a real constraint discovered while designing the modding workflow,
documented here so it isn't rediscovered the hard way by a modder (or a future dev helping one).

A theme's own images (a custom background, custom button art) go through the exact same
`RmlUiRenderInterface::LoadTexture` → `CGlobalBitmap::LoadImage` path as every other texture in the
game — a deliberate Phase 0 design choice, so RmlUi-referenced images share the same ref-counted
cache/eviction. The consequence: `CGlobalBitmap::LoadImage` (`Render/Sprites/GlobalBitmap.cpp`)
only recognizes two extensions, `.jpg`/`.tga` — and **both internally swap the extension to this
engine's proprietary container format before opening** (`.OZJ`/`.OZT`). An RCSS rule referencing
`panel_bg.tga` actually needs a file named `panel_bg.OZT` on disk, not a real `.tga`; plain
`.png`/`.bmp` aren't handled at all, and there's no converter tool in this repo. Worse: a missing
file or unrecognized extension fails with **zero logging anywhere in the chain** — `LoadImage`
returns `false` silently, `LoadTexture` returns a null handle, and the element just renders with no
image and no diagnostic.

**What does work, confirmed against RmlUi's real source**: a relative image path inside an RCSS
rule (`decorator: image(...)`, `background-image`) resolves against *that stylesheet's own
resolved location* (`StyleSheetParser.cpp`/`ElementEffects.cpp`/`Decorator.cpp`), not the shared
`login.rml`'s virtual per-theme source URL — so a custom theme's image can sit right next to its
own `.rcss` file and be referenced with a plain relative path. This part needed no fix; it already
works correctly by virtue of how `LoadThemedDocument()` and RmlUi's own stylesheet-loading both
handle source URLs.

See [Theming & Modding](theming-and-modding.md#a-third-case-a-theme-with-its-own-custom-images)
for the full workflow and the open (not yet built) follow-up: vendoring `stb_image.h` to give
`LoadTexture` a plain-PNG/JPG/BMP fallback for RmlUi-referenced theme assets specifically, without
touching the legacy OZT/OZJ pipeline everything else still depends on.
