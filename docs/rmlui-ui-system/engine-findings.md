# Engine Findings: RmlUi Build-Specific Gotchas

Empirical facts about *this specific codebase and engine build*, not general policy. Most are
RmlUi quirks of the vendored build; the last section covers the `CObject`/`CManager` machinery
around it. See [`STATUS.md`](STATUS.md) for migration status and
[`tracked-deferrals.md`](tracked-deferrals.md) for open work.

## RmlUi build

- **`data-for` preserves DOM nodes by array index, not by an item ID.** Appending an entry
  preserves existing inputs and scroll containers. Erasing an earlier entry from a compact
  array leaves the focused element at its original index, now bound to a different item;
  that element's scroll offset follows it. Use stable document identities for independently
  editable windows. Verified against `DataViewFor::Update()` in `DataViewDefault.cpp` by
  `tests/ui/test_rml_friend_window_identity.cpp`.

- **`font-family` does not reliably inherit into subtrees** — a descendant needs its own explicit
  `font-family`; relying on an ancestor renders its text invisible with no error, data model or
  not. Every text-bearing selector in `themes/modern/*.rcss` declares
  `font-family: "token(font-body)"`; modern has no document-wide default the way legacy's `#panel`
  rule provides one. **Check this first whenever a new element's box paints but its text doesn't**
  — that signature has been misread as a flex/decorator/layout bug for several rounds before.

- **The themed loader's token substitution (`RmlTheme.cpp`) resolves each `<link href>`
  against `sourceUrl`'s directory** (always `themes/<theme>/`), not wherever the RML text came
  from, and substitutes every `<link>`, not just the first. A document may have a
  `themes/<theme>/<name>.rml` override that the loader prefers silently (`login`,
  `msg_win`, `main_frame`, `character_info`, `my_quest_info`, …) — **check for one before reading
  a document's markup from the shared top-level `.rml`.**

- **`overflow: hidden` does not clip an absolutely-positioned oversized child**, even with
  `clip: always`. `ContainerBox::Close()` (`BlockContainer.cpp`) submits the box's
  scrollable-overflow rect — which `ElementUtilities::GetClippingRegion()` reads to decide whether
  to scissor — *before* `ClosePositionedElements()` places absolutely-positioned children, so a
  container whose only content is such a child always computes "nothing to clip". For sprite
  atlases use generated named `@spritesheet` rects and one `data-style-decorator` per icon
  (`buff_strip.rcss`, `skill_icons.rcss`).

- **`dp` and `px` are not interchangeable in `data-style-*` position bindings.** `dp` is scaled by
  the UI-scale setting (`SetDensityIndependentPixelRatio()`), `px` never is — a `+'px'` binding
  drifts out of step with `dp`-sized siblings at any UI scale other than 100 %. Bind in whichever
  unit the sibling static CSS in the same file uses.

- **A `data-style-*` binding is an inline property, and an inline property beats every stylesheet
  rule in this build with no `!important` escape.** `ElementStyle::GetLocalProperty()`
  (`Source/Core/ElementStyle.cpp`) returns the inline dictionary's entry first and only consults
  the element's RCSS definition when there is none; importance is never compared, and RmlUi's
  parser has no `!important` for a theme to reach for. So a property a window binds from its model
  is not merely "hard to override" — **no theme, mod or user stylesheet can override it at all**,
  including one shipping its own copy of the document. This is what makes C++-side presentation a
  capability loss rather than an untidiness, and it is the reason `building-new-ui.md`'s ownership
  rules treat "bind only what actually varies with data" as a hard rule. The root transform
  (`root_x`/`root_y`/`root_scale`) and the `.sharp-text` counter-scale are the accepted exceptions:
  both are the scaling bridge itself, which no theme should be overriding.
  **A plain `style="left: 16px"` attribute in the markup is the same inline property** — the same
  dictionary, written by the parser instead of by a data view, and easier to miss because it reads
  as ordinary authoring. `Tools/check_rml_bound_geometry.py` checks both forms.

- **An absolutely-positioned, `display: block`, multi-line (`white-space: pre-line`) box needs an
  explicit `width`** — left to shrink-to-fit, the width computation undersizes to the longest
  *word*, not the longest *line*.

- **The default `display` is `inline`, not `block`, for every element including `<div>`**
  (`StyleSheetSpecification.cpp`; there is no user-agent stylesheet). Absolute positioning makes a
  block box anyway, which hides it; in-flow stacked content (e.g. `data-for` lines) runs together
  as one paragraph. Declare `display` explicitly.

- **Gradients go through `RenderInterface::CompileShader()`.** `RenderInterface_SDL_GPU`
  implements it for the gradient family (`RmlUi_SDL_GPU/shader_frag_gradient.frag`, baked into
  `ShadersCompiledSPV.h`); `linear-gradient` and `radial-gradient` render correctly. The deprecated
  `horizontal-gradient`/`vertical-gradient` are unused. **A property parsing doesn't mean the
  render interface implements it** — test a new decorator or effect in isolation.

- **`box-shadow` renders, including blur, inset and multiple shadows** (`PushLayer`/
  `CompositeLayers`/`CompileFilter`/`RenderBlur` are implemented in `RmlUi_Renderer_SDL_GPU.cpp`).
  `rgba()` alpha is a 0–255 integer, not a 0–1 float: `rgba(0,0,0,.62)` is invisible.
  Of `filter`, only `brightness()`/`contrast()` are verified.

- **`backdrop-filter: blur()` blurs the game world behind a panel.** The renderer starts each
  context's base layer as a copy of the swapchain, which already holds the scene and anything an
  earlier context drew, and writes the layer back at the end, so a backdrop has the scene to read.
  Before that the base layer started empty and was blended onto the scene afterwards, and a
  backdrop blurred nothing. The modern options screen uses it (`option_window.rcss`). A full-screen
  backdrop blur costs a blur pass every frame it is visible.

- **`decorator: image(...)` does not stretch a sprite rect to a box of a different size;
  `ninepatch(...)` with a declared inner rect does.** `server_select.rcss`'s `.server-row`/
  `.group-btn` use `ninepatch(legacy-btn-idle, legacy-btn-idle-inner)`. Legacy's plain `.gmd-btn`
  (128 dp / 64 dp over a 108×30 sprite) has the same width mismatch and has not been visually
  checked; `.gmd-btn.cols-2` uses the ninepatch fix.

- **A bordered/backgrounded element with child elements can paint as an incomplete, non-closed
  rectangle.** A bordered leaf with no children always renders correctly. Put the paint on a
  childless leaf and make other content its *sibling* under an unstyled wrapper
  (`server_select.rcss`'s `.server-row` > `.server-name` + `.server-gauge-slot`).

- **A block's auto height doesn't reliably sum several in-flow block children**, even unbordered.
  Give the wrapper an explicit height.

- **`:nth-child(N)` works on `data-for` clones** (they are ordinary children of the parent), which
  is how a variable-length list gets its rows from RCSS instead of a `top` per entry. Give each
  repeated set its own container so the count starts at its first item
  (`blood_castle_enter.rcss` / `devil_square_enter.rcss`). **`:nth-last-child(N)`** is implemented
  too (for a stack growing away from its anchor) but not yet proved at runtime.

- **`data-attr-id` on a `data-for` clone gives each row a real id that `GetElementById()` finds**,
  so C++ can read a repeated row's live geometry back (`RefreshLogicalAnchorPosition`) instead of
  binding its position — `master_level.rml` anchors its skill hint on the hovered node this way.

- **RCSS comments don't nest, and a broken one in a shared file corrupts every document that links
  it** — several unrelated windows breaking at once after a `base.rcss` edit points here. Never
  write a literal comment delimiter inside a comment.

- **Read what a legacy getter returns, not what its name says, before binding it.**
  `CSkillList::IsSkillListUp()` reports the hotkey row's scrolled slot set, not whether the skill
  popup is open; `main_frame.rml` binds `IsSkillGridOpen()`.

- **Header-rail chrome split across fg and bg documents duplicates `top`/`height`.** The rail's
  paint lives in each window's `*_bg.rcss` (`.rail-fill`/`.rail-accent`) so it draws *behind* live
  3D item icons; `base.rcss`'s `.modern-header-rail-px` is layout only. The values are copied from
  the fg override (`storage.rcss`'s `.stor-header-rail` ↔ `storage_bg.rcss`'s `.rail-fill`) across
  the 10 inventory-family windows, and `check_rml_rcss_drift.py` does not compare numbers — grep
  the sibling selector before changing either.

- **`<template>` files are sliced with comment-blind text search** (`Template::Load()`,
  `XMLParseTools::FindTag()`), so comment text can break them:
  1. A literal tag-shaped example such as a `body` tag carrying `template="window_shell"`, even
     inside a comment, is found before the real `<body>`; if it names the same template it recurses
     into itself and overflows the stack (`0xc00000fd`) at load. Describe usage in prose.
  2. "head" or "body" directly after a `/` anywhere in the file sets `FindTag()`'s `found_closing`
     flag, which is never reset between candidates, so the real `<body>` is rejected and the
     template fails to load — logged only as `Failed to load template`, and the consumer falls back
     to unstyled flow. Write "head and body".
  Only files loaded *as* a template are affected.

- **A `<link>` inside a `<template>` file's head gets no `token(...)` substitution.**
  `InlineTokenizedStylesheet()` rewrites the links of the document `ThemedDocumentLoader::Load()`
  loads; RmlUi's `TemplateCache` reads template files directly. Link
  token-using stylesheets from the consuming document (`generic_confirm_dialog_bg.rml` links
  `base.rcss`); a template's own `.rcss` must be token-free (`window_shell*.rcss` are).

- **`Rml::TemplateCache` also caches templates by declared name, and that entry is not refreshed
  per lookup**, so two themes' forks of the same template name could splice in the wrong theme
  after a switch. `UI::RmlBridge::SetActiveThemeName()` calls `Rml::Factory::ClearTemplateCache()`
  on every theme change; nothing per-window is needed.

- **Reparenting an element (`RemoveChild()` then `AppendChild()`) keeps its `data-*` attribute
  bindings but loses `{{}}` text interpolation** — `ApplyDataViewsControllers()` re-scans
  attributes only. Build the element at its final place, or keep it in place and position it with
  CSS (`COptionWindow`'s close button sets its text with `SetInnerRML()`).

- **Don't use native `<select>`** (`WidgetDropDown`): its generated `selectbox` needs `position`
  and more by hand, and even then selection did not reliably commit or fire `change`. Use a
  `data-for` custom dropdown (`option_window.rcss`'s `.option-dropdown` family).

- **A persistent document pumped by a scene's own manual loop needs `Update()` too.**
  `LoginScene.cpp`/`CharacterScene.cpp` pump `COptionWindow` with `UpdateMouseEvent` →
  `UpdateKeyEvent` → `Update` → `Render`; `Update()` runs `SyncRmlModel()`, and omitting it left
  every `{{}}` field empty in those scenes.

- **No `static bool` guard around `RegisterStruct<T>()`/`RegisterArray<C>()`.**
  `RmlModelBinder<T>::Create()` gives each call a fresh `DataTypeRegister`, so the registration
  lambda must run in full every time; a guard skips it on a theme switch and breaks the bind.

- **Unloading a document does not blur its focused element** (`Context::UnloadDocument()`/
  `OnElementDetach()` clear focus by assignment), so a focused `<input>` destroyed that way leaves
  the system interface's text-input latch set and `CManager::UpdateKeyEvent()` suspends every
  window's keys for the session. `ReloadAllThemedDocuments()` blurs before reloading, and
  `RmlUiRuntime::IsTextInputActive()` checks the live focus element's tag. **Any other path that
  unloads or hides a document while a field may be focused must blur it.**

- **Hiding any document can re-focus a field the player left.** `ElementDocument::Hide()` →
  `Context::UnfocusDocument()` focuses the previous document's remembered focus leaf.
  `RmlUiRuntime::ReleaseStrandedFieldFocus()` unlinks any remembered `input`/`textarea` that isn't
  the live focus after every press and once per frame, and releases a focused field that is no
  longer visible.

- **A self-deleting listener gets one `OnDetach()` per registration, not per element.**
  `MakeDraggable()` registers one listener for three events, so it counts attachments and deletes
  itself after the last; deleting on the first detach crashed in `DetachAllEvents()`.

- **Counter-scaled text layers cannot be stacked by ordinary flow.** A `.sharp-text` block's layout
  box is `root_scale` times taller than it draws (`transform` never enters layout), so a sibling
  after it is pushed down by a gap that grows with UI scale. Give each such block its own `top`
  (`npc_quest.rml` in legacy).

- **A counter-scaled layer can still be centred by the theme.** Scaling back around its centre
  keeps a layer's middle where layout put it, so `base.rcss` has two recipes that need no C++
  measurement. `.sharp-middle` centres a one-line layer on its parent's height: `top: 50%`, with
  `translateY(-50%)` leading its RML transform. `.sharp-centre` centres it on its parent's width:
  a 2000-unit layer centred with `left: 50%` and a negative margin, wide enough never to overflow,
  because an overflowing line is start-aligned. The parent is the box the theme sizes, such as a
  button or a line's row.

- **`<input type="range">` computes the wrong value inside a `transform: scale()` panel** while
  hit-testing correctly: `WidgetSlider::AbsolutePositionToBarPosition()` ignores transforms, input
  hit-testing doesn't. Right at 100 %, wrong elsewhere. In a root-transformed panel, give the slider
  a hit area laid out in real pixels, outside the transform, or use the level gauge
  (`component-catalog.md`), which maps the pointer against the drawn bar.

- **Dragging a transform-centred panel: two traps** (`RmlDraggable.cpp`). An inline
  `transform: none` set from C++ does not cancel `.center-both`'s `translate(-50%, -50%)`; a class
  rule does (`.center-both.dragged`). And mouse events on an element inside a transformed panel
  carry that panel's local coordinates. `MakeDraggable()` takes the start from the handle's
  `mousedown`, subtracts the layout-to-drawn offset while the transform is still in effect, and
  pins the panel where it is drawn (`ElementUtilities::GetBoundingBox()`).

- **A data-model change does not reach layout until the context's next update.**
  `ElementDocument::UpdateDocument()` lays out, but data models flush only in `Context::Update()`,
  so a position published through a binding hasn't moved anything when the same frame measures
  again. A hidden document still lays out (`Show()`/`Hide()` toggle `visibility`): place it hidden
  and show it a sync later (the friend family's `m_Settled`).

- **Releasing an owned GPU texture mid-frame drops that frame's native replay.**
  `ReleaseOwnedTextureById()` sets `s_texturesInvalidated` and `EndFrame` skips every draw recorded
  that frame. Release between frames: `ReleaseRenderTarget()` queues for that point.
  `EnsureOffscreenColorTexture()` releases when resized, so a render target never resizes in place.

- **Rendering an item moves the picking ray.** `RenderItem3D()` goes through
  `CameraProjection::ScreenToWorldRay()`, which sets `MousePosition`, the origin of terrain and
  object picking. A native item renderer that doesn't restore it leaves every ground click missing.
  `SaveCameraPerspective()`/`RestoreCameraPerspective()` cover it.

## CSS features this build lacks, and what to use instead

For translating a browser mockup or ordinary CSS into a theme. The vendored RmlUi is upstream 6.3
plus [PR #989](https://github.com/mikke89/RmlUi/pull/989) (SDL_GPU renderer parity).

| CSS feature | On this engine | Use instead |
|---|---|---|
| `display: grid` | Not a `display` keyword (only `none`/`block`/`inline`/`inline-block`/`flow-root`/`flex`/`inline-flex`/`table*`) | Flexbox |
| `::before`/`::after`, `content:` | No `content` property, no generated pseudo-elements | A real child element in the `.rml` |
| `aspect-ratio` | Not a property | An explicit `width`/`height` |
| `text-shadow` | Not a property | `font-effect: outline(...)` |
| `:focus-visible` | Not supported (`:hover`/`:active`/`:focus` and structural selectors are) | `:focus` |
| `calc()`, `min()`, `max()` | In the pin (upstream PR #983, carried on the fork), unused so far | Fixed `dp`/`px` values until tried |
| `clamp()`, `minmax()` | Not registered | Fixed `dp`/`px` values |
| `var(--x)` | Supported by 6.3, unused here | The project's `token(name)` (`theming-and-modding.md`) |
| `backdrop-filter: blur()` | Works, over the game world too (frosted glass) | — |
| `filter: blur()`/`drop-shadow()`, `mask-image` | Unverified | — |
| `filter: brightness()`/`contrast()` | Works (`my_inventory.rcss`'s `.inv-btn:hover`) | — |
| `box-shadow` | Works: inset, outset, blur, comma-separated; `rgba()` alpha is 0–255 | — |
| `linear`/`radial`/`conic-gradient`, and `repeating-*` | Work, through `CompileShader` | — |
| `transform: rotate()` | Works (`base.rcss`'s `.modern-joint`, the checkbox tick) | — |
| A circular progress arc | No CSS equivalent | `<progress direction="clockwise">` — real arc geometry, **but only with a `fill-image` set**: without a texture a circular direction renders as an unclipped rectangle (`ElementProgress::GenerateGeometry()`). For a flat look, segment `<div>`s |

## `CObject`/`CManager`/`LayoutMode`

- **`CManager::CompareKeyEventOrder` sorts DESCENDING: the highest `GetKeyEventOrder()` runs
  first**, and `CManager::UpdateKeyEvent()` stops at the first object returning `false`. A modal
  given a low value runs last and lets another window's Esc swallow the key.
  `CGenericConfirmDialog`/`CGenericMenuDialog` use `100.0f` (the next tier is `10.0f`); give any
  new dialog primitive the same. Don't "fix" the comparator — every override is calibrated to it —
  and don't trust a comment that says lower runs first.

- **`CManager::AddUIObj(dwKey, obj)` overwrites the object's `LayoutMode`** with
  `UI::Layout::ForInterface(dwKey)`. `UILayoutPolicy.cpp`'s table is the authority; a key without
  a `case` falls through to `LayoutMode::Dialog`, a real resolution-scaled transform.

- **Real-pixel rendering needs `LayoutMode::Legacy`**, the only identity transform.
  `CManager::UpdateMouseEvent()` remaps the global `MouseX`/`MouseY` through the active transform,
  and text rendering reads `UI::Scaling::GetActiveTransform()`; `CSprite::Render()` reads the
  transform's offset but not its scale.

- **A native widget that reads the active transform at render time can end up double-scaled** if
  its position was divided by a different transform when set. Store real pixels and render inside
  the same explicit `ScopedActiveTransform` the write used.

- **`g_pTimer->GetTimeElapsed()` is total process uptime** (`ResetTimer()` has no callers); the
  usual `MIN(g_pTimer->GetTimeElapsed(), 200.0 * FPS_ANIMATION_FACTOR)` is the clamp in practice.
  Use that expression for a per-frame delta.

- **Same-frame ordering.** A full `Update()` sweep before an "is anything visible" check let one
  Esc both close a window and trigger another toggle. Decide the order when one piece of per-frame
  logic reads state another can consume.

- **With `CObject`'s shown/active split, compute `SetActive()` from every condition that should
  suspend active handling** (every covering modal), once at the top of `UpdateWhileShown()`, so
  `UpdateWhileActive()` is gated in one place.

- **Check whether a window's click gate was hardcoded before porting its click handling** — an
  interactivity that was unreachable natively needs no port.

- **A modal relevant to a gameplay scene must fold its claimed input into that scene's "cursor
  over UI" query**, generally rather than per window, or clicks behind it reach the world.
