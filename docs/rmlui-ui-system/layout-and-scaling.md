# Layout, Anchoring & Scaling

Use this guide to choose coordinate spaces, compose the shared scales, read drawn geometry and
validate layout. Themes own ordinary layout; C++ supplies state and shared integration.
[Architecture Principles](architecture-principles.md) defines the ownership policy,
[the component catalog](component-catalog.md) lists reusable recipes, and
[Window Placement](window-placement.md) covers workspace arrangement.

## Window dimensions and reference units

The RmlUi context uses the window dimensions supplied to `Create()` and `OnResize()`. The SDL
integration supplies window-coordinate dimensions, not `SDL_GetWindowSizeInPixels()` framebuffer
dimensions. These normally agree at pixel density 1; do not assume they agree on every display.

Documents cover the context. The 640x480 reference used by shared transforms, `.stage` and
`.hud-board` is a compatibility layout space, not a letterboxed context or a second window.
In this guide, **window coordinates** mean the space used by the context and drawn-box helpers;
source APIs often call these window pixels. Framebuffer pixels are a separate display metric.

## Global UI scale: `dp`, not a custom calculator

`10dp` occupies `10 * context dp ratio` window-coordinate units. The runtime's ratio hook uses
`UI::Scaling::TypographyScale()`, which includes viewport fit, content scale and the user's
`[UI] UIScalePercent` preference. It is not simply the preference divided by 100.

Players change the preference through Options -> UI -> UI Scale. The preference is persisted
and clamped to 75-150%; the options control offers a fixed set of choices. Applying it refreshes
resolution-dependent systems, including the context and workspace. The deferred apply sequence
and exact choices live in `OptionWindow.cpp`.

A `px` length does not use the context's `dp` ratio, but ancestor transforms still affect it.
Choose units from the coordinate space below; changing every `px` to `dp` double-scales
reference-pixel panels.

## Shared scales and their inputs

Use [UITransform.h](../../src/source/UI/Scaling/UITransform.h) and its implementation for shared
scale calculations rather than copying a formula into a window.

| Scale or transform | Purpose |
|---|---|
| `TypographyScale()` / `CompanionRatio()` | Shared panel-scale ratio used by context `dp` and compatible native companions |
| `PanelTransform()` | Uniformly scaled, centred compatibility canvas |
| `BottomHudScale()` | Bottom HUD scale |
| `DockRightTransform()` | Dock scale and placement relative to the HUD; workspace arrangement remains theme-owned |
| `TypographyUnitsTransform()` | Native measuring units from the window's top-left |
| `ScreenOverlayTransform()` | Independently stretched reference axes for retained world-overlay drawing |

The automatic uniform fit is `clamp(min(width / 640, height / 480), 1, maximum * contentScale)`.
Layout kinds have their own ceilings. `UIScalePercent / 100` multiplies the fit afterward,
subject to the reference-canvas fit limit in `WithUIScalePercent()`. This lets the user adjust
size even when automatic fit has reached its ceiling. The limit constrains reference-canvas
scale; it does not ensure arbitrary window content fits or cannot overlap.

`contentScale` is derived from SDL display scale divided by pixel density. It changes the
upper fit bound, not the lower bound or a direct user multiplier. The automatic fit keeps a
floor of 1 at the reference size. Native font sizing uses its own role-based growth rules;
it is not a font size multiplied by the panel transform.

Resize and display-scale handlers refresh the relevant scale inputs, fonts and placement.
The shared linear fit supports compatibility comparisons, but does not by itself establish
pixel-for-pixel parity: theme layout, typography and content scale also affect the result.
Hardware validation limits are recorded in
[Engine Findings](engine-findings.md#display-density-validation).

## Units and placement

Choose a space before choosing lengths or converting geometry:

| Coordinate space | Use and units |
|---|---|
| Direct scalable document layout | `dp` sizes and offsets; the context ratio applies once |
| Root-transformed panel, `.stage` or `.hud-board` | Reference `px`; the ancestor transform supplies scale |
| Counter-scaled text layer | Physical-size `px` in context coordinates; the compensating transform cancels the panel scale |
| Drawn geometry / native item targets | Window coordinates returned by drawn-box helpers |
| Pointer within an element | Element-local layout units returned by `PointerIn()` |
| Native measuring units | Units of the active native transform, selected by the owner/manager |

For example, at a context ratio of 1.5, `10dp` occupies 15 window units. A reference `10px` inside
a panel scaled by 1.5 also occupies 15. Combining that panel transform with `10dp` occupies 22.5,
which is usually unintended. A text layer that cancels the 1.5 transform draws a `10px` font
at size 10 in context coordinates.

Choose placement independently of units: a workspace slot for arranged windows, `.stage` or
`.hud-board` for reference screens, or document-local anchors and flow. Workspace configuration,
slot sizing, dragging and persistence belong in [Window Placement](window-placement.md).

`CManager` scopes native update, input and rendering passes to `MeasuringUnits()`. The gameplay
manager uses `TypographyUnitsTransform()`; scene integration uses `LegacyUiTransform()`.
A measured width belongs to that transform's units, which need not equal a docked panel's
reference units. Use explicit native scopes such as `ScopedWindowPixels` or
`ScopedScreenStretch` when retained drawing needs those spaces. They do not change RCSS layout.

## Counter-scaled text layout

A root transform scales text along with geometry. When compatibility needs native physical-size
text, cancel the root transform on the text layer. Its lengths then lay out in context pixels:
RCSS multiplies reference lengths by `--root-scale`, while the bound font size stays physical.
Per-axis panels also supply `--root-scale-y`.

Counter-scaling does not reduce the element's layout footprint. Stack related lines inside one
flow layer instead of stacking independent counter-scaled siblings. `.sharp-flow` derives width
from `--flow-w` and optional height from `--flow-h`; `.centred` centres the whole flow.

For a counter-scaled scroll pane, scrollbar appearance reduces available content width.
Percentage-width rows can therefore shift columns. Use an explicit scaled RCSS width when
columns must stay fixed. The [component catalog](component-catalog.md#counter-scaled-text)
provides the class recipes and references. Ordinary `dp` layouts need no compensating layer.

## Scale inputs and reference screens

[`ApplyScaleInputs()`](../../src/source/UI/RmlBridge/RmlScaleInputs.h) sets inherited properties
on the context root when scale is refreshed:

| Property | Meaning |
|---|---|
| `--ui-scale` | Shared typography/panel ratio, also used by context `dp` |
| `--hud-scale` | Bottom HUD scale |
| `--dock-scale` | Dock scale |
| `--overlay-scale-x`, `--overlay-scale-y` | Independently stretched screen-reference axes |

Both themes' `base.rcss` provide 640x480 reference-pixel containers:

- `.stage` centres the canvas and scales it by `--ui-scale`.
- `.hud-board` centres the canvas horizontally, anchors it at the window bottom and scales it
  by `--hud-scale`.

Each exposes its scale as `--root-scale`. Children use reference `px`; these containers supply
the canvas placement, while RCSS still places content within it. Do not bind duplicate canvas
position or scale from each window.

## Anchor/sizing utility classes (`base.rcss`)

Both themes supply the same pure-layout utility classes. Anchors act against the element's
containing block, which may be a panel or reference canvas rather than the whole window.

| Class | Effect |
|---|---|
| `.anchor-top-left`, `.anchor-top-right`, `.anchor-bottom-left`, `.anchor-bottom-right` | Absolute positioning with the corresponding edges at zero |
| `.center-x`, `.center-y`, `.center-both` | Absolute positioning at 50% with a compensating translation |
| `.stretch-x`, `.stretch-y`, `.stretch-both` | Absolute positioning with opposing edges at zero |

Add consumer-specific offsets and dimensions in RCSS, using the units of that container.
Anchor classes do not decide whether those lengths should be `dp` or reference `px`.
Centering classes set `transform`; if the element also needs scale or rotation, compose the
transform explicitly or use separate wrapper elements. Two classes do not automatically combine
independent transform declarations.

Choose fixed dimensions for controls that need a stable box, flow for content that can grow,
and stretch for content intended to fill its container. Centred prompts may use fixed or
responsive layouts; HUD parts may anchor to edges or live within the HUD board. Live 3D needs
an image sized by its drawn box, not C++ ownership of panel layout.

## Data-driven geometry and native companions

Static positions and dimensions belong in RCSS. State-driven values include projected markers,
gauge fills, tooltip anchors and measured content. Shared placement and scale properties are
integration inputs. Geometry bindings must satisfy `tools/check_rml_bound_geometry.py` with
an explained exception where required; passing the guard does not establish runtime correctness.
See [Ownership](building-new-ui.md#ownership-what-the-c-side-of-a-window-may-own).

Native drawing and interaction should follow drawn element geometry. Use
[`DrawnContentBox()` / `DrawnBox()`](../../src/source/UI/RmlBridge/RmlElementBox.h) to read its
window-coordinate bounds through ancestor transforms. Raw `GetBox()` values describe layout
before render transforms; `GetAbsoluteOffset()` alone does not supply transformed geometry.
Do not multiply a drawn box by UI scale again.

Legacy companions that still mirror layout calculations are compatibility exceptions. They must
use the document's authoritative dimensions and matching units; `CompanionRatio()` supplies the
shared ratio for direct `dp` layouts. The
[character-selection example](migration-ledger.md#character-selection-layout-compatibility)
records an existing exception and its validation requirements.

## Hit testing a document

Use [`RmlPointer.h`](../../src/source/UI/RmlBridge/RmlPointer.h) according to the question:

| Helper | Meaning |
|---|---|
| `IsPointerOver(document)` | RmlUi currently hovers an interactive element of the document other than its body |
| `IsPointerOver(element)` | RmlUi hovers the element or a descendant in a visible document |
| `IsPointWithin()` / `IsPointerWithin()` | A window-coordinate point or the game's pointer lies in the drawn border box, including transforms; pointer-event eligibility is not required |
| `PointerIn()` | Converts the pointer into the element's local layout units relative to its border-box origin; reports failure when the element is not drawn |

Hover ownership and geometric containment are different. A decorative click-through element can
contain the pointer without being the hovered interactive element. Use hover ownership to decide
which document claims input, and geometric/local coordinates for a map, grid or viewer as needed.
Gameplay click gating must also include `Core::Input::IsMouseOverUI()` so a UI action does not
reach the world behind it.

### Checking it: the scale sweep

Test layout changes in both themes at 75% and 150% UI scale. Use a larger window, such as
1280x720 or 1920x1080, where fit limits allow the larger setting to grow; small windows alone
may not exercise the full range.

1. Click each interactive control. For grids, include cells at every corner to expose accumulated
   pitch or offset errors.
2. Confirm UI clicks do not also move the character or act on the world.
3. Hover tooltip and popup targets; anchors should follow the drawn element.
4. Resize, change scale after dragging, and switch themes with the window open. Check native
   content, scroll state, focus and placement where relevant.
5. Record tested resolutions, scales, themes and remaining gaps with the change. Compare original
   behavior when changing compatibility layout.

### The headless text-layout audit

[`test_rml_text_layout.cpp`](../../tests/ui/test_rml_text_layout.cpp) loads real documents,
bundled fonts and translated content into a headless context, then reports clipping, overlap
and text outside panels. Current coverage includes event-entry windows, personal shops and
MU Helper. The preparation script defines the document/locale list; the test defines scale,
resolution, class and tab scenarios and which findings must pass. Legacy MU Helper findings
are reported rather than all asserted away.

From a configured developer shell, following the [build setup](../build/README.md):

```powershell
cmake -S . -B out/build/windows-x64 -DBUILD_TESTING=ON
cmake --build out/build/windows-x64 --config RelWithDebInfo --target rml_text_layout_tests rml_party_trade_layout_tests
ctest --test-dir out/build/windows-x64 -C RelWithDebInfo -R "rml_text_layout_baseline|full party stays|legacy trade notice" --output-on-failure
```

CTest discovers the party/trade cases by their test descriptions, not the executable target name.
The text baseline writes `scenarios.csv` and `text-lines.csv` under
`out/build/windows-x64/text-layout-baseline/`. The separate
[`party/trade layout test`](../../tests/ui/test_rml_party_trade_layout.cpp) covers those documents.

To extend coverage, update [`prepare_rml_text_layout.py`](../../tests/ui/prepare_rml_text_layout.py),
supply representative model state in the fixture/test, select meaningful scenarios and define
assertions or intentional diagnostic findings. Adding a document name alone does not establish
coverage.

Headless checks validate prepared geometry, not live SDL input, actual framebuffer density,
all game states or server actions. They do not cover every window. Keep the runtime scale sweep
and display-hardware checks for behavior outside the automated scenarios.
