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

### `transform` parses fine and silently never renders

**Symptom**: `transform: rotate(45deg)` on a border corner (the classic CSS checkmark trick)
rendered as a static, un-rotated corner. Zero `[RmlUi]` warnings in `MuError.log` — the
declaration parses and is accepted without complaint.

**Root cause**: same category of gap as `box-shadow` blur above — `SetTransform` is one of the
"optional advanced-rendering functions" `RmlUiRenderInterface` leaves at the base class's no-op
default (its own header comment names it explicitly). Unlike `box-shadow`, there's no
blur-radius-zero threshold that makes it "work by accident" — `transform` is unconditionally
inert here. The property still parses and is stored correctly
(`StyleSheetSpecification.cpp`/`PropertyParserTransform.cpp`); nothing downstream ever applies it.

**Fix**: don't use `transform` (rotate/skew/translate/scale) in any theme until
`RmlUiRenderInterface::SetTransform` is implemented. For an angled shape, use real bitmap art
(`decorator: image(...)`) or an axis-aligned approximation from plain positioned rectangles (e.g.
a blocky pixel-art checkmark from several small `background-color` squares). Check
`RmlUiRenderInterface.h`'s header comment for "left at their base-class no-op defaults" before
reaching for an unfamiliar CSS property — it lists every such gap up front.

### `FileBacked` textures rendered using the wrong GL texture object

**Symptom**: the `legacy` theme's `#panel`/`.input-frame` `decorator: image(...)` backgrounds
didn't render at all (fully transparent, no error logged anywhere), and separately, unrelated debug
overlay text appeared blown up and garbled in the middle of the login panel.

**Root cause**: `RmlUiRenderInterface` hands RmlUi an opaque `Rml::TextureHandle` for every texture
it loads, backed internally by one of two *kinds* (`RmlUiRenderInterface.h`'s own header comment
already flags this distinction as load-bearing): `Generated` (RmlUi's own font-glyph atlases —
`id` is a real `RHI::TextureHandle`/GL name straight from `RHI::CreateTexture`) and `FileBacked`
(anything routed through `CGlobalBitmap` — `id` is a **bitmap *index*** into that cache, e.g. `31009`
for `login_back.tga`, not a GL name at all). `ReleaseTexture` already dispatched correctly on
`kind` (`Bitmaps.UnloadImage` for `FileBacked`, `RHI::DestroyTexture` for `Generated`) — but
`RenderGeometry`'s texture-binding line bound `texIt->second.id` directly as a GL texture name
**for both kinds**, unconditionally. This was invisible for the entire migration up to this point
because every texture RmlUi had ever rendered was `Generated` (text only, no images) — the
`legacy` theme's background/input-frame decorators were the **first `FileBacked` textures RmlUi
ever rendered**, and binding bitmap-index `31009` as if it were a GL texture name bound whatever
GL texture object actually happened to hold that numeric name — plausibly explaining the garbled
debug-overlay-looking text as a side effect of sampling the wrong (but very real) texture.

**How it was actually found**: not guessed after the fact — narrowed down by elimination. A
temporary diagnostic in `LoadTexture` confirmed the texture *load* itself succeeded (a valid,
non-`BITMAP_UNKNOWN` bitmap index, correctly ref-counted against the exact same cache slot
(`BITMAP_LOG_IN+7` = `31009`) the original working `CWin` sprite used) before concluding the bug
had to be downstream of loading, in the actual render/bind path — reading `RenderGeometry` line by
line against the class's own header comment (which explicitly warns the two id spaces are "not
guaranteed disjoint") is what surfaced the mismatch.

**Fix**: `RenderGeometry` now checks `kind` before binding — `FileBacked` resolves through
`Bitmaps.GetTexture(id)->TextureNumber` to get the real GL name first; `Generated` uses `id`
directly as before.

**Practical rule for the next migrated window**: any decorator/`<img>` that references a real
image file (not text) is exercising the `FileBacked` path for the first time in whatever window
migrates next after this fix — already fixed now, but if a *different*, similarly-shaped id-space
mismatch bug shows up again, check every place a `TextureRecord`/`kind` is consumed, not just
`ReleaseTexture` (which was already correct) — a class-wide invariant being enforced in one method
doesn't guarantee it's enforced in all of them.

### A referenced image's real (unpadded) size must be declared via `@spritesheet`, not assumed

**Symptom**: after fixing the texture-binding bug above, `login_back.tga`'s art appeared squeezed
into roughly the top-left two-thirds of the panel, with a blank strip along the right/bottom edges
— not invisible, but visibly wrong.

**Root cause**: this engine's legacy `.OZT` loader (`CGlobalBitmap::OpenTga`) pads every texture up
to the next power-of-two size (`NextPowerOfTwo`) — confirmed by reading it: 329×245 becomes a
512×256 texture, with the real image content in the top-left corner and the rest zero-filled. This
is a leftover constraint from old GPUs; it's invisible to every legacy `CSprite` caller because
`CSprite::Create()` computes its own UV sub-rect from the *original* dimensions the caller passed
in (`m_fOrgWidth`/`m_fOrgHeight`), never the padded texture size. RmlUi's plain
`decorator: image("path/to/file")` has no equivalent caller-supplied sub-rect — a referenced image
with no matching named sprite always samples the *entire* texture as 0..1 UV — so it stretched the
real content across a corner of the panel while squishing in a chunk of blank padding on the rest.

**Fix**: declare the image as a named sprite via `@spritesheet`, with an explicit pixel rectangle
matching its *real* (unpadded) size —
```rcss
@spritesheet login-back-sheet
{
	src: "../../../login_back.tga";
	login-back-image: 0px 0px 329px 245px;
}
```
then reference the sprite *name* (not the raw path) in the decorator: `decorator: image(login-back-image);`.
`DecoratorTiledInstancer::GetTileProperties` (`Source/Core/DecoratorTiled.cpp`) checks for a named
sprite match *before* falling back to treating the name as a raw image path — a declared sprite's
`Rectanglef` becomes the tile's UV sub-rect instead of the full texture, exactly mirroring
`CSprite`'s own original-dimensions approach.

**Practical rule for the next migrated window**: any RCSS `decorator: image(...)` (or `<img>`)
referencing one of this engine's real (non-power-of-two-dimensioned) legacy art assets needs a
`@spritesheet` declaration with the asset's *real* pixel size — never reference the raw file path
directly unless the asset's real dimensions already happen to be an exact power of two.

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

See [Theming & Modding](theming-and-modding.md#bringing-your-own-images-to-a-theme)
for the full workflow and the open (not yet built) follow-up: vendoring `stb_image.h` to give
`LoadTexture` a plain-PNG/JPG/BMP fallback for RmlUi-referenced theme assets specifically, without
touching the legacy OZT/OZJ pipeline everything else still depends on.

### A per-theme flag to opt back into `CWin` sprite rendering was the wrong shape

**Not a bug — a design mistake caught and reversed before it caused one.** The theme framework
originally shipped a per-theme `theme.ini` manifest key, `UsesLegacySpriteChrome`, that let a
theme tell `CLoginWin::Create()`/`RenderControls()` to keep creating and drawing `CWin`'s
background sprite and the `CUITextInputBox` frame `CSprite`s underneath the RmlUi overlay, instead
of RmlUi drawing that chrome itself. The `legacy` theme shipped with this flag set to `1`.

**Why this was wrong**: the entire point of migrating a window to RmlUi is that RmlUi ends up
owning its rendering — a configurable escape hatch that lets a theme opt back into the *old*
renderer drawing part of a *migrated* window's visuals works directly against that. It also wasn't
even necessary: the art files the flag was protecting (`Interface/login_back.tga`,
`Interface/login_me.tga`) are ordinary standalone images (no sprite-sheet cropping — confirmed by
reading the `CSprite::Create()` call sites for `BITMAP_LOG_IN+7`/`+8`, both pass `nMaxFrame=0`),
already preloaded into the shared, ref-counted bitmap cache regardless of theme. RmlUi's own
`LoadTexture` (`RmlUiRenderInterface.cpp` → `CGlobalBitmap::LoadImage`) finds and ref-counts that
same cached texture by filename (`CGlobalBitmap::FindTexture(filename)` scans by name across the
whole cache, not just entries loaded through the named-load path) — so pointing an RCSS decorator
at the same file was never blocked by anything technical, only by an unnecessary flag.

**Fix**: removed `UsesLegacySpriteChrome` entirely — from `RmlTheme.h/.cpp`, both theme's
`theme.ini` files (which had no other purpose), and `CLoginWin`. `CWin::Create()` now always passes
`nTexID=-2` for this window, in every theme, unconditionally. The `legacy` theme reproduces the
original look by giving `#panel`/`.input-frame` a `decorator: image(...)` pointing at the same
`Interface/login_back.tga`/`Interface/login_me.tga` files the old sprites drew — same pixels,
rendered by RmlUi instead of `CWin`. **Lesson**: a data-driven per-theme toggle is the right shape
for things that are genuinely a visual choice (colors, layout, whether to use vector shapes or
raster art) — it's the wrong shape for something that contradicts the migration's own goal
regardless of which value it's set to; that kind of "flexibility" should be caught during design,
not shipped and left as an unused footgun once nothing exercises the non-default path.

---

## Interaction helpers

### A drag handle that inherited `pointer-events: none` never fires drag events at all

**Symptom**: `UI::RmlBridge::MakeDraggable()`, first wired to a test handle, produced no reaction
whatsoever to click-and-drag — not `dragstart`, not `drag`, not even `mouseover` when a temporary
diagnostic listener was added for every event type.

**Root cause**: the exact same inheritance mechanism as the
[`pointer-events` click-swallowing bug](#pointer-events-swallows-every-click-on-screen) above, but
biting from the opposite direction. A handle element positioned inside a full-window document
following that fix inherits `pointer-events: none` from `body` by default (only specific
interactive elements opt back in to `auto`). `Context::GetElementAtPoint` (`Source/Core/Context.cpp`)
explicitly skips any element whose computed `pointer_events()` is `None` — so an unmodified handle
is invisible to RmlUi's own hit-testing, never becomes `hover`, and `dragstart`/`drag` (which only
dispatch on the hover chain) never fire, regardless of whether `drag: drag` is set correctly.

**Fix**: `MakeDraggable()` forces `pointer-events: auto` on `handle` itself, so a caller doesn't
need to remember a matching RCSS rule.

**Diagnostic approach that actually found it**: rather than keep guessing after three failed
"still not moving" reports, added a temporary listener logging *every* event type reaching the
handle (not just drag-specific ones) and a temporary log confirming the handle/panel element
pointers themselves were non-null. The first version of that logging still showed *zero* events,
even `mouseover` — a much stronger signal than "the drag didn't work," since it ruled out
`Style::Drag`/event-ID mistakes entirely and pointed straight at hit-testing. Once `mouseover`
was confirmed reaching the listener but `dragstart` still wasn't happening from a *different*
handle candidate (`#panel` itself), that isolated the second, separate finding below.

### Dragging the RmlUi overlay does not move a hybrid window's legacy sprite chrome

**Not a bug in `MakeDraggable()`** — a real architectural fact about how hybrid windows are built,
worth being explicit about since it's easy to assume RmlUi "contains" or is otherwise linked to
the legacy chrome underneath it. (At the time this was found, the login window's `legacy` theme
still had `CWin` drawing the background sprite — that's since been removed, see
[the entry below](#a-per-theme-flag-to-opt-back-into-cwin-sprite-rendering-was-the-wrong-shape),
but the underlying fact this section documents is unchanged: RmlUi and whatever legacy state a
hybrid window still owns are never automatically linked.)

A hybrid window's legacy `CWin` background sprite and its RmlUi overlay are two **independently
rendered things** that merely start at the same screen position — both set once, from the same
`CWin::SetPosition()` call (see [Architecture §6](architecture.md#6-coexistence-bridging-cloginwins-specific-pattern)).
RmlUi has no knowledge of `CWin::m_ptPos`, and nothing keeps the two in sync afterward. Confirmed
by a real test: dragging via `MakeDraggable()` moved the RmlUi checkboxes/buttons/labels correctly
frame to frame, while the legacy background sprite stayed exactly where it started, immediately
and visibly desyncing from the content now floating away from it.

**Fix**: `MakeDraggable()` takes an optional `onMove(newLeft, newTop)` callback, fired on every
drag tick, specifically so a hybrid window's caller can also reposition whatever legacy state
still needs to track the panel (e.g. call `CWin::SetPosition()`) in lockstep. A fully migrated
window with no remaining legacy state has nothing to sync and can simply omit the callback.

### A whole panel can't usually be its own drag handle

Corollary of the pointer-events gotcha above, but worth stating as its own lesson: for any window
whose panel inherited `pointer-events: none` for the click-passthrough fix (true of every
full-window document built so far), the panel **can't be dragged by clicking anywhere on it**
without forcing its `pointer-events` back to `auto` — which would reintroduce the exact
click-swallowing bug that fix exists to prevent, for anything underneath the panel (input boxes,
other legacy content). Use a dedicated handle element instead (a title bar, a specific label) —
this isn't a workaround for a limitation, it's the same reason real UI toolkits use a title bar
rather than "grab anywhere on the window body": content inside a panel needs to keep receiving
its own clicks independently of the drag gesture.

### `MakeDraggable` tracks both axes: a horizontal-only slider has to fight that

**Symptom**: while building `COptionWin`'s volume/render-level sliders (`MakeDraggable`'s first
real production use, beyond the throwaway test handle described above), a slider thumb dragged
horizontally also visibly drifted up/down, tracking the cursor's Y movement even though the track
itself never moves vertically.

**Root cause**: `MakeDraggable`'s internal `DragMoveListener` unconditionally sets **both** `left`
and `top` from the `drag` event's mouse delta (`RmlDraggable.cpp`) — correct, and the whole point,
for a freely-draggable panel (its only use case until now), but a slider thumb is constrained to
one axis, and `MakeDraggable` itself has no concept of that constraint — it always tracks the
cursor on both axes regardless of what `handle`/`panel` conceptually represent.

**Fix**: not a change to `MakeDraggable` itself (both-axis tracking is correct for its actual
job — panel dragging). Each slider's `onMove` callback (`COptionWin::OnSliderThumbDragged`)
recomputes and overwrites `left` with the clamped/snapped value as intended, *and* also resets
`top` back to a fixed `"0px"` on every tick, undoing whatever vertical value `MakeDraggable` just
set moments earlier in the same event.

**Practical rule for the next 1D-constrained draggable element** (a slider, a horizontal scroll
thumb, anything that isn't a freely-draggable panel): `MakeDraggable`'s `onMove` callback is the
right place to clamp/snap the axis you care about, but remember to also reset the axis you *don't*
care about — it will otherwise silently drift with the cursor, since `MakeDraggable` has no way to
know your handle is axis-constrained.

### Getting `SysMenuWin`/`OptionWin`'s panel body to render at all took five attempts — tiling still isn't one of them

**Symptom**: `SysMenuWin`/`OptionWin`'s legacy-theme panel background (`op1_stone.jpg`, meant to
fill the middle band between the top/bottom caps) went through five real, distinct failure modes
across several rounds of "fixed, verify" with a real screenshot each time — worth reading in full
because each one independently produces the *identical* symptom (nothing renders, nothing logs),
so ruling one out proves nothing about the others.

1. **`tiled-vertical` never repeats.** First attempt used `decorator: tiled-vertical(top, center,
   bottom);` on a single `#panel` element. Confirmed by reading `DecoratorTiled.cpp`'s
   `RegisterTileProperty` and `DecoratorTiledVertical`/`Horizontal`/`Box`'s instancer constructors:
   none of them pass `register_fit_modes = true`, so none of their tiles ever get a `-fit` property
   registered at all — every tile, including the center, is permanently `FILL` (stretch), with no
   RCSS-level way to ask for `REPEAT`. Rendered as one heavily stretched, blurry copy — visually
   close enough to flat that it read as "some areas not have the background texture."
2. **Wrong shorthand argument position.** Switched to the plain `image()` decorator (the one
   decorator that *does* register real fit-mode support, via
   `RegisterTileProperty("image", true)`), but wrote `decorator: image(sprite-name repeat);`. The
   shorthand's real argument order is `src, orientation, fit, align-x, align-y` — a bare
   space-separated second token binds to **orientation**, not **fit**; `"repeat"` isn't a valid
   orientation keyword, the declaration fails to parse, and the *entire* decorator drops. Rendered
   as fully transparent — strictly worse than attempt 1.
3. **`repeat` + a named sprite is rejected outright.** Fixed the argument order to
   `image(sprite-name, none, repeat)`, comma-separated — still fully transparent. `DecoratorTiled.cpp`'s
   `GetTileProperties` explicitly checks `if (sprite && fit_mode is REPEAT-family) { LT_WARNING;
   return false; }` — the whole decorator instantiation fails whenever the source resolves through
   `GetSprite()` (a named `@spritesheet` entry) rather than `GetTexture()` (a raw path), and
   `op1_stone.jpg` had been declared as a sprite purely for stylistic consistency with every other
   tile in the file (it never actually needed the padding-workaround a spritesheet exists for —
   already power-of-two).
4. **An unrelated sizing bug, found only after 1–3 were genuinely fixed.** Switched the center tile
   to a raw quoted path (`image("../../../op1_stone.jpg", none, repeat)`) — still transparent, but
   this time `MuError.log` had *zero* `[RmlUi]`-tagged lines, meaning nothing was failing to parse
   or instantiate anymore. The next suspect: `.panel-middle` had no explicit `height`, relying on
   both `top` and `bottom` being set on a `position: absolute` element to derive height from the
   gap between them (legitimate CSS 2.1 behavior in principle, never actually confirmed to compute
   correctly in this specific RmlUi 6.2 integration). Fixed by giving `#panel_middle` a real `id`
   and pushing `top`/`height` explicitly from C++, the same proven technique `#panel` itself
   already uses — **still transparent after this fix too**, ruling out sizing as *the* remaining
   blocker (though it's a correctness improvement worth keeping regardless).
5. **The raw quoted path likely never lost its quotes.** With parsing and sizing both now
   independently ruled out, the last live theory: `PropertyParserString::ParseValue`
   (`PropertyParserString.cpp`) does **zero quote-stripping** — it stores the raw token verbatim.
   `font-family: "Liberation Sans"` works elsewhere in this codebase, but that's a top-level
   property parsed through a different code path than a value embedded inside a shorthand function
   call like `image(...)`. A quoted literal used as a shorthand argument may retain its literal
   quote characters as part of the resolved "path" — which then fails to resolve to any real file,
   and this engine's texture loader fails **100% silently** on a bad path (already documented in
   [Theming & Modding](theming-and-modding.md#bringing-your-own-images-to-a-theme)) — consistent
   with everything observed (no art, no warning anywhere). Not proven with a live debugger, but
   consistent with every fact gathered, and further chasing it stopped being worth the cost — see
   the fix below.

**Actual fix shipped**: none of the above, deliberately. After four straight misses on the same
visible symptom, the tiling ambition itself was set aside in favor of the one decorator form
already *proven* to render correctly everywhere else in this exact document — a **named sprite**
with **plain `FILL`** (`decorator: image(legacy-panel-center);`, no fit-mode argument at all),
identical in form to `.panel-cap-bottom`/`.btn`/`.checkbox-box`, all of which render correctly.
This stretches the 128×128 source across the middle band instead of tiling it — a real, disclosed
visual downgrade from the original goal, not a hidden one — but ships something that actually
renders instead of a fifth unverified theory. True tiling is a deliberate, isolated follow-up
experiment for later, not something to keep blocking on.

**Practical rule for the next tiled-background attempt**: prefer the decorator form with the most
existing proof of working in this codebase over the theoretically-more-correct one, especially
after two or more silent-failure rounds on the same rule — every one of the five failure modes
above produces the *identical* symptom (nothing renders, and 4 of 5 produce zero log output too),
so a screenshot alone cannot distinguish between them; each fix has to be argued from source and
then independently re-verified, and "still broken" after a source-grounded fix is real information
(it rules that theory out), not proof the theory was worthless reasoning.

### Sixth issue, a genuinely different category: the center tile was never modeled as spanning the whole panel

**Symptom**: after fix 5 above (fallback to plain `FILL` + named sprite) actually got the stone
texture rendering, a real screenshot showed it *only* in the gap between the top/bottom caps —
"the mid section (horizontal) has texture, the top and bottom part are still transparent." The
scrollwork linework on the caps rendered correctly (as it had in every earlier screenshot too), but
the space around/behind that linework showed through to whatever's behind the document, exactly as
if the caps' own art has genuine alpha transparency around the ornate line pattern (not a solid
background) — which turned out to be exactly the case.

**Root cause**: not a decorator bug at all this time — a structural modeling error, caught only by
going back to the *actual* legacy `CWinEx` source (`UI/Widgets/WinEx.h`/`.cpp`) instead of
continuing to iterate on the RmlUi side. `WE_BG_CENTER = 0` and `CWinEx::Render()`'s loop
(`for (i = 0; i < WE_BG_MAX; ++i) m_psprBg[i].Render();`) draws the center tile **first**, before
top/bottom/left/right — and `SetPosition()`/`SetLine()` size and position it to span **nearly the
entire panel** (inset by `WE_CENTER_SPR_POS` = 3px on every side), not confined to the gap between
the caps. The cap images are drawn *on top of* that already-drawn center tile, and since the caps'
own art has transparent space around the scrollwork linework, the center shows through underneath
them by design — this is how the ornate-border-over-solid-texture look is actually achieved in the
original widget. The RmlUi reconstruction had modeled this as three independent, non-overlapping
stacked bands (`.panel-cap-top` 0–64px, `.panel-middle` the literal gap, `.panel-cap-bottom` the
last 43px) — internally consistent and exactly what fixes 1–5 above were spent getting to render
correctly, but built on a structural assumption about the composite that was never actually checked
against the widget it was meant to reproduce.

**Fix**: reordered `#panel_middle` to be the *first* child in the RML (before both cap elements, so
it paints first), and gave it the same 3px inset `CWinEx` uses via plain RCSS —
`left: 3px; top: 3px; right: 3px; bottom: 3px;`, no explicit `width`/`height` at all. This first
shipped with the inset pushed from C++ instead (`SysMenuWin`/`OptionWin`'s `SetPosition()`,
mirroring fix 4's earlier "don't trust opposing-edge auto-sizing" caution) — reverted once that
caution was actually checked against `Layout/LayoutDetails.cpp`'s `BuildBoxWidth`/`BuildBoxHeight`
and confirmed to be unfounded: `position: absolute` elements with `left`+`right` (or `top`+`bottom`)
set and no explicit size *do* correctly derive their box dimensions from the containing block minus
those insets — standard CSS 2.1 behavior, genuinely implemented, not an untested corner case. Kept
in C++ initially "to be safe," it was actually the wrong default: this project's stated goal is
that a theme (including a modder's) needs zero C++ changes and zero recompilation, and a
hardcoded-in-C++ inset value meant a future theme wanting a different one couldn't express that
without an engine change. Moved back to RCSS once the caution was disproven, matching every other
per-element position rule in this codebase. The caps' own rules didn't need to change beyond that —
only `panel_middle`'s DOM order and inset expression did.

**Practical rule for the next multi-piece legacy-widget reproduction**: when translating a
composite legacy `CWin`/`CWinEx`-family widget into RmlUi elements, check the *original* render
loop's draw order and each piece's real geometry before modeling the RmlUi structure — don't infer
the composite's shape from what the *visible* gaps between labeled pieces suggest. A background
piece that's drawn first and spans the full area, with foreground pieces with their own
transparency layered over it, looks structurally different from independent non-overlapping bands
even though a static screenshot of the *original* widget can look identical to either model until
one of them is actually missing.

## Window lifecycle

### `CWin::Release()` has no idea `m_pRmlDoc` exists — a hybrid window can outlive its own scene

**Symptom**: after building `CLoginMainWin` (Batch 2), transitioning from the login scene to
character-select left its Menu/Credit icon buttons visibly overlapping `CharSelMainWin`'s own
button bar on the *next* scene — described from a real screenshot as "the Create button... overlaid
with an older painting of the menu button."

**Root cause**: `CUIMng::RemoveWinList()` — called at the top of every `Create*Scene()` function,
i.e. on every scene transition — walks every window still in `m_WinList` and calls `Release()` on
it unconditionally. `CWin::Release()`/the virtual `PreRelease()` hook it calls predate RmlUi
entirely and know nothing about `m_pRmlDoc`; they release legacy `CButton`s and `CSprite`s but
never touch the RmlUi document. Since a migrated window's document is deliberately created **once**
and reused across repeated `Create()` calls (see `Create()`'s own `if (!m_pRmlDoc && ...)` guard,
the same pattern `CLoginWin` established), `Release()` leaves it exactly as visible as it was the
moment `Release()` ran — and because RmlUi always renders **last** in the frame (see
[Architecture §3](architecture.md#3-frame-lifecycle--the-render-order-contract)), that stale
document paints on top of whatever the *next* scene's windows draw, indefinitely, until something
else happens to call `Show()`/`Hide()` on it again.

This reproduced 100% of the time for `CLoginMainWin` specifically because it's unconditionally
visible for the entire login flow, so it's *guaranteed* to still be shown at the exact moment
`CreateCharacterScene()`'s `RemoveWinList()` runs. `CSysMenuWin`/`COptionWin` have the identical
gap but only manifest it if the player happens to have the menu open at the exact instant a scene
transition fires — a much rarer window, not a safer design. `CLoginWin` (the Phase 1 pilot) turned
out to have the same latent gap too, currently masked only by the fact that both of its real
show→hide paths (`RequestLogin()`, `CancelLogin()`) happen to call `CUIMng::HideWin(this)` before
any scene transition can occur — an incidental property of those two call sites, not something
`CLoginWin`'s own lifecycle actually guaranteed.

**Fix**: every hybrid `CWin` + RmlUi window's `PreRelease()` override now unconditionally calls
`if (m_pRmlDoc) m_pRmlDoc->Hide();` — `Hide()`, not unload/destroy, consistent with the
create-once-reuse-forever document lifecycle already established. Applied to `CLoginWin`,
`CLoginMainWin`, `CSysMenuWin`, `COptionWin` — every current hybrid window. `RememberPasswordPrompt`
is unaffected (never a `CWin`, never touched by `CUIMng::RemoveWinList()` at all).

**Practical rule for the next hybrid `CWin` + RmlUi window**: `PreRelease()` isn't optional
boilerplate to skip when a window has no legacy sprites to clean up — if it owns an `m_pRmlDoc`,
`PreRelease()` must hide it, full stop, regardless of whether some other call site *currently*
happens to hide it first. A window whose only "proof" of correct hide behavior is "nothing has
hit the gap yet" is exactly the shape of bug this entry describes.

## Scene-conditional visibility

### `.hidden` vs `.disabled`: a dimmed button can still visually collide with its neighbor

**Symptom**: in the modern theme, `CSysMenuWin`'s dialog on the login screen showed the Select
Server button rendering at 50% opacity directly behind the Option button — not just visually
adjacent, but overlapping enough that Option's text was legible through Select Server's own.

**Root cause**: real layout math, not a rendering bug. `CSysMenuWin::SetPosition()` stacks its four
buttons at fixed offsets that assume the panel is tall enough to fit all four with real spacing —
true when the panel is opened from the character-select scene (`SetLine(10)`), but the *login*
scene's panel is shorter (`SetLine(6)`, since it has one less real reason to be tall), which puts
Select Server at `top≈67px` and Option at `top≈69px` — almost exactly on top of each other. This
overlap already existed before this migration; it was invisible only because the legacy `CButton`
Select Server used while disabled rendered fully opaque-grey, visually reading as "one solid
button," not two stacked ones. `.btn.disabled { opacity: 0.5; }` (added generically, for any future
disabled button, not specifically vetted against this one collision) made the underlying overlap
show through for the first time.

**Fix**: added a `.hidden { display: none; }` utility class to both themes' `base.rcss`, and
switched Select Server to use it (instead of `.disabled`) specifically in the login scene, where
the button doesn't apply at all (you can't switch servers before logging into one) rather than
merely being temporarily unavailable. `display: none` removes the element from layout entirely —
no overlap possible, because there's nothing there to overlap with. Renamed the underlying model
field end-to-end to match its real meaning: `selectServerDisabled`/`select_server_disabled` (bool
field, model binding key) → `selectServerHidden`/`select_server_hidden`; `sys_menu.rml`'s
`data-class-disabled="select_server_disabled"` → `data-class-hidden="select_server_hidden"` on the
`btn_select_server` element. `.btn.disabled` itself was untouched and remains available for any
future button that genuinely wants dim-but-clickable-elsewhere semantics.

**Practical rule for the next scene-conditional button**: `.disabled` and `.hidden` are not
interchangeable "make this button go away" options — `.disabled` keeps the element in layout
(taking up space, able to visually collide with a neighbor if the layout was never actually
spaced for the button being both present *and* dimmed) and communicates "temporarily unavailable,
might become clickable later in this same view." `.hidden` removes it from layout entirely and
communicates "does not apply to this view/scene at all." Reach for `.hidden` (and a model field
name that says *hidden*, not *disabled*) whenever the reason a button shouldn't be clickable is
"this scene doesn't have this feature," not "this feature is temporarily off." Getting this
distinction right up front also avoids the trap this bug fell into: a generic `.disabled` style
change (adding opacity) silently exposing a pre-existing layout assumption that was never actually
true for every panel height the button could appear at.
