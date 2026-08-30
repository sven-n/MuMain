# UI Layout Regressions Design

## Goal

Fix four resolution-dependent UI regressions without redesigning unrelated
interfaces:

- the `D` command window and `J` command-list window render below the bottom
  HUD;
- the character-selection bar and its click targets do not resize with the
  window;
- the map-name splash moves when inventory or another side panel opens;
- the teleport window shrinks or collapses at some resolutions.

The changes must preserve the responsive bottom HUD, the full world viewport,
the existing panel scale cap, and the remote-server test workflow.

## Root Causes

### Command windows

`CNewUIManager` renders objects in ascending `GetLayerDepth()` order. The main
HUD uses depth `10.6`; both command windows use `4.6`. Any overlap is therefore
painted and hit-tested below the HUD. Reducing the `D` window height addressed
geometry only; it could not change this compositing order.

The `J` window also remains 459 logical pixels high while docked panels must end
at logical `y=432`. Its last 27 pixels necessarily enter the HUD region.

### Character selection

Runtime resize updates `WindowWidth` and `WindowHeight`, then recreates the
legacy scene UI. `CInput`, however, retains the dimensions passed at startup.
The recreated character-selection bar therefore uses stale width and height,
while cursor clamping also continues using the old bounds.

The bar's buttons and decoration are always created at their original physical
pixel size. At larger resolutions the bar background expands, but the controls
and their `CSprite` hit rectangles remain 54x30 pixels and become relatively
smaller.

### Map-name splash

`CUIMapName` centers its image with `GetScreenWidth()`. That function is not the
physical or reference viewport width: it deliberately returns 640, 450, 260,
or 70 logical pixels according to the number of open side panels. The splash
therefore shifts left when a panel opens.

The current formula pre-scales its position because the `RenderBitmap` call
explicitly disables position and size scaling. The 166x90 image therefore
keeps a fixed physical size; only its center source is wrong.

### Teleport window

`CNewUIMoveCommandWindow::SetPos()` chooses widths with an exact
`WindowWidth` switch. Larger resolutions receive progressively smaller logical
widths; unsupported widths retain zero or stale geometry.

The height assumes 31 rows regardless of the dockable height. With current font
metrics, `60 + rowHeight * 31` exceeds logical `y=432`. The scrollbar then
divides by `listSize - 31`; empty or short lists make that value zero or
negative. Page counting is also incorrect for exact multiples, and one
scrollbar coordinate adds `m_Pos.y` twice.

## Selected Design

Use targeted fixes around the existing layout systems. Do not apply a new
transform to every legacy `CWin` or `CSprite`; many login and title controls
already use explicit physical scaling, so a global change would double-scale
them.

### Shared dock boundary

Expose the existing logical dock bottom, `432`, as one named compile-time
constant in `UI::Scaling`. The dock transform, both command windows, and the
teleport layout use that constant. This prevents another local height from
drifting away from the HUD boundary.

### Command-window stacking and height

Both command windows use a named foreground-panel depth strictly above the main
HUD and below modal message boxes. Rendering and mouse dispatch use the same
depth ordering, so visible command controls receive input before the HUD under
them.

Both frames end at the shared dock boundary. The `J` window changes from 459 to
432 logical pixels. Its existing content already ends at logical `y=421`, so no
rows or actions need removal. The `D` window remains 432 pixels high.

No other panel depth changes.

### Character-selection resize and scale

Add a validated `CInput` screen-size update that changes only the cached width
and height. Call it during runtime resize before `CUIMng::RepositionSceneUI()`.
The existing scene recreation then reads current dimensions, and cursor
clamping uses the same physical bounds.

Scale only `CCharSelMainWin` visual controls. Its native layout is 800x600, so
the uniform control scale is:

```text
controlScale = clamp(min(windowWidth / 800, windowHeight / 600), 1.0, 2.0)
```

Create button textures at their original 54x30 source-frame size, then resize
their screen rectangles with `CSprite::SetSize()`. This preserves texture-frame
UVs while enlarging rendering and hit testing together. Apply the same scale to
the decoration, bar height, margins, and inter-control gaps.

The bar continues spanning the window. Its information-strip width is the
remaining physical width after scaled controls, gaps, and symmetric outer
margins are subtracted. Its vertical anchor remains proportional to the scene
height, with the existing bottom gap scaled uniformly.

`CWin`, `CButton`, and `CSprite` continue receiving physical coordinates. No
new scene-wide mouse transform is introduced. Existing message, system,
character-creation, and login windows remain unchanged.

### Stable map-name overlay

Center the splash against the physical window while preserving its current
166x90 physical size and scaled vertical position:

```text
x = (windowWidth - imageWidth) / 2
y = 220 * screenRateY
```

Keep position and size scaling disabled in `RenderBitmap`; enlarging the splash
is outside this fix. The calculation never calls `GetScreenWidth()` and never
inspects open interfaces. The optional strife banner uses the same physical
center.

### Resolution-independent teleport layout

Keep the teleport window left-docked with one stable logical width of 230
pixels, matching its current 640x480 width. Remove the physical-resolution
switch.

Measure the current logical row height when the window opens. Derive the
visible row count from the shared dock boundary:

```text
availableHeight = dockBottom - windowY
visibleRows = max(1, floor((availableHeight - fixedChromeHeight) / rowHeight))
windowHeight = fixedChromeHeight + visibleRows * rowHeight
```

`fixedChromeHeight` remains 60 logical pixels, covering the title, column
headings, list-to-close gap, and close row. The window keeps the full derived
height even when the server provides fewer rows; list size must never collapse
the frame.

Replace the recursive pixel-distribution scrollbar with conventional bounded
state:

- `scrollOffset` is the first visible row;
- `maxScrollOffset = max(0, itemCount - visibleRows)`;
- mouse-wheel input changes the offset by rows and clamps it;
- dragging maps the thumb's clamped track position to
  `[0, maxScrollOffset]`;
- no-overflow lists disable the thumb and perform no division;
- render, hover, selection, and close-hit rectangles come from the same stored
  geometry.

Remove obsolete page counters, recursive scroll accumulators, and duplicated
`UpdateScrolling()` calls. Reopening the window refreshes data and geometry,
then resets the offset and selection safely.

## Error Handling

- Ignore non-positive resize dimensions; do not poison `CInput` bounds.
- Clamp character-control scale and remaining information-strip width.
- Treat missing or empty teleport data as a valid empty list with a stable
  frame and disabled scrollbar.
- Clamp every teleport offset after data or geometry changes.
- Guard all iterator advancement and selection indexes against list end.

## Testing

Follow test-driven development. Add failing coverage before each production
change.

Focused tests cover:

- the shared dock boundary and both command-window heights;
- command-window foreground depth relative to HUD and modal layers;
- character-control scale at 640x480, 800x600, 1280x1024, 1920x1080, and 4K;
- resize propagation to cached `CInput` dimensions;
- scaled character button rectangles matching their click rectangles;
- physical-window map-name centering independent of panel width;
- teleport width at supported and formerly unsupported resolutions;
- visible-row calculation within the dock boundary;
- empty, shorter-than-visible, exact-visible, and overflowing teleport lists;
- wheel and thumb mapping at first, middle, and last offsets.

Run the focused UI tests before and after each fix, then the complete CTest
suite, `git diff --check`, and the native client link. Preserve the known
pre-existing NativeAOT `brotlienc` failure as a separately reported baseline.

Native verification resolves the remote target from `MUMAIN_SERVER_IP` and
`MUMAIN_SERVER_PORT` in the parent workspace `.env` beside `ctl`. Let `ctl`
load and validate those values; do not hardcode the endpoint in source, tests,
documentation, or run commands. Do not start local server or client processes.
Verify at one 5:4 resolution and one wide resolution:

- `D` and `J` remain visible and clickable above the HUD;
- character-selection rendering and clicks resize together after changing
  resolution without re-entering the scene;
- the map splash remains centered while inventory and another panel open;
- the teleport frame remains stable, readable, scrollable, and non-collapsed.

Do not claim visual completion without native screenshots or direct user
confirmation.

## Non-Goals

- No global rewrite of legacy `CWin`/`CSprite` coordinate handling.
- No user-configurable UI-scale option.
- No redesign of teleport content or server-provided move rules.
- No changes to the bottom HUD layout, world viewport, camera limits, or panel
  column policy.
- No local server startup or deployment changes.
