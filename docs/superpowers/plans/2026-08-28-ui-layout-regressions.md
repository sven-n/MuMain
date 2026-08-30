# UI Layout Regressions Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use
> `superpowers:subagent-driven-development` (recommended) or
> `superpowers:executing-plans` to implement this plan task-by-task. Steps use
> checkbox (`- [ ]`) syntax for tracking.

**Goal:** Keep the `D` and `J` command windows above the HUD, resize the
character-selection bar and hitboxes with the window, keep the map splash
physically centered, and make the teleport window stable at every supported
resolution.

**Architecture:** Reuse the existing dock transform and legacy widgets. Expose
the dock bottom once, add one targeted character-selection physical layout,
center the map splash from `WindowWidth`, and replace teleport paging math with
pure geometry plus a bounded first-visible-row offset. No global legacy-widget
rewrite, new dependency, configurable scale, or bottom-HUD redesign.

**Tech Stack:** C++20, CMake/CTest, doctest, SDL3.

**Spec:**
`docs/superpowers/specs/2026-08-28-ui-layout-regressions-design.md`

## Global Constraints

- Keep the responsive bottom HUD, full world viewport, panel scale cap, and
  existing dock policy unchanged.
- Use logical `y=432` as the only dock bottom.
- Keep foreground command windows above HUD depth `10.6` and below modal depth
  `10.7`.
- Scale only `CCharSelMainWin`; leave login, character-creation, message, and
  system windows on their existing paths.
- Keep the map splash `166x90` physical pixels. Keep `RenderBitmap` position
  and size scaling disabled.
- Keep the teleport window left-docked, `230` logical pixels wide, stable for
  empty and short lists.
- Add no files for production helpers. Keep tightly coupled helpers in the
  existing headers and implementations.
- Never hardcode a server address or port. The workspace-root `../ctl` reads
  `MUMAIN_SERVER_IP` and `MUMAIN_SERVER_PORT` from `../.env`.
- Do not start a local server or client process. Native visual checks require
  user launch/confirmation through `../ctl run --release`.
- Do not claim visual completion without screenshots or direct user
  confirmation.

---

### Task 1: Record the pre-change baseline

**Files:** None.

**Produces:** Reproducible pre-change evidence, separated from failures caused
by later edits.

- [ ] **Step 1: Confirm repository state**

Run:

```bash
git status --short
git log -5 --oneline
```

Expected: clean worktree; `docs(ui): plan layout regression fixes` is the
current HEAD and `e1e4a33c docs(ui): preserve map splash size` is its parent.

- [ ] **Step 2: Build and run the focused UI binary**

Run:

```bash
cmake --build out/build/macos-arm64 --config Release --target ui_scaling_tests -j2
out/build/macos-arm64/tests/ui/Release/ui_scaling_tests
```

Expected baseline: `20/20` doctest cases pass. Record actual output if the
baseline has changed.

- [ ] **Step 3: Run the complete suite**

Run:

```bash
ctest --test-dir out/build/macos-arm64 -C Release --output-on-failure
```

Expected baseline: `158/158` CTest tests pass. Record actual output; never copy
this count into the post-change report without rerunning.

- [ ] **Step 4: Record native link behavior**

Run:

```bash
cmake --build out/build/macos-arm64 --config Release --target MuClient -j2
cmake --build out/build/macos-arm64 --config Release --target Main -j2
```

Expected: `MuClient` succeeds. `Main` may reproduce the known host NativeAOT
`brotlienc` failure. Save the exact error and treat it as pre-existing; do not
describe the standard `Main` target as passing if it fails.

---

### Task 2: Share the dock boundary and raise command windows

**Files:**

- Modify: `src/source/UI/Scaling/UITransform.h`
- Modify: `src/source/UI/Scaling/UITransform.cpp`
- Modify: `src/source/UI/NewUI/UILayoutPolicy.h`
- Modify: `src/source/UI/NewUI/HUD/NewUICommandWindow.h`
- Modify: `src/source/UI/NewUI/HUD/NewUICommandWindow.cpp`
- Modify: `src/source/UI/NewUI/Dialogs/NewUIChatCommandWindow.h`
- Modify: `src/source/UI/NewUI/Dialogs/NewUIChatCommandWindow.cpp`
- Test: `tests/ui/test_ui_scaling.cpp`

**Interfaces:**

- Produces: `UI::Scaling::DockLogicalBottom == 432`.
- Produces: `UI::Layout::ForegroundPanelLayerDepth == 10.65f`.
- Produces: public `CNewUIChatCommandWindow::WindowHeight` for the frame-height
  invariant.
- Consumes: existing dock transforms, `CNewUIObj::GetLayerDepth()` ordering,
  HUD depth `10.6`, modal depth `10.7`.

- [ ] **Step 1: Add failing dock and layer tests**

Add the chat-command include:

```cpp
#include "UI/NewUI/Dialogs/NewUIChatCommandWindow.h"
```

Add:

```cpp
TEST_CASE("dockable command windows share the HUD boundary [ui][scaling]")
{
    CHECK(UI::Scaling::DockLogicalBottom == 432);
    CHECK(CNewUICommandWindow::COMMAND_WINDOW_HEIGHT == UI::Scaling::DockLogicalBottom);
    CHECK(SEASON3B::CNewUIChatCommandWindow::WindowHeight == UI::Scaling::DockLogicalBottom);
}

TEST_CASE("command windows render between HUD and modal layers [ui][scaling]")
{
    CNewUICommandWindow commandWindow;
    SEASON3B::CNewUIChatCommandWindow commandListWindow;

    CHECK(UI::Layout::ForegroundPanelLayerDepth > 10.6f);
    CHECK(UI::Layout::ForegroundPanelLayerDepth < 10.7f);
    CHECK(commandWindow.GetLayerDepth() == doctest::Approx(UI::Layout::ForegroundPanelLayerDepth));
    CHECK(commandListWindow.GetLayerDepth() == doctest::Approx(UI::Layout::ForegroundPanelLayerDepth));
}
```

- [ ] **Step 2: Run RED**

Run:

```bash
cmake --build out/build/macos-arm64 --config Release --target ui_scaling_tests -j2
```

Expected: compile failure because `DockLogicalBottom`,
`ForegroundPanelLayerDepth`, and `CNewUIChatCommandWindow::WindowHeight` do not
exist.

- [ ] **Step 3: Expose the shared constants**

In `UITransform.h`, inside `namespace UI::Scaling`:

```cpp
inline constexpr int DockLogicalBottom = 432;
```

Delete the file-local `kDockLogicalBottom` from `UITransform.cpp`. Make
`DockTransform()` use `DockLogicalBottom` directly.

In `UILayoutPolicy.h`, inside `namespace UI::Layout`:

```cpp
inline constexpr float ForegroundPanelLayerDepth = 10.65f;
```

- [ ] **Step 4: Bind both command frames to the shared values**

In `NewUICommandWindow.h`, include `UI/Scaling/UITransform.h` directly and set:

```cpp
COMMAND_WINDOW_HEIGHT = UI::Scaling::DockLogicalBottom,
```

In `NewUIChatCommandWindow.h`, include `UI/Scaling/UITransform.h`, remove the
private `WINDOW_HEIGHT = 459`, and expose:

```cpp
public:
    static constexpr int WindowHeight = UI::Scaling::DockLogicalBottom;
```

Replace every `WINDOW_HEIGHT` use in `NewUIChatCommandWindow.cpp` with
`WindowHeight`. Keep all content coordinates unchanged: the last control ends
at logical `y=421`, inside the new frame.

Return the named layer from both command implementations:

```cpp
return UI::Layout::ForegroundPanelLayerDepth;
```

Add `UI/NewUI/UILayoutPolicy.h` includes directly in both `.cpp` files.

- [ ] **Step 5: Run GREEN**

Run:

```bash
cmake --build out/build/macos-arm64 --config Release --target ui_scaling_tests MuClient -j2
out/build/macos-arm64/tests/ui/Release/ui_scaling_tests
```

Expected: focused binary passes; `MuClient` compiles.

- [ ] **Step 6: Commit the command-window fix**

Run:

```bash
git diff --check
git add src/source/UI/Scaling/UITransform.h src/source/UI/Scaling/UITransform.cpp src/source/UI/NewUI/UILayoutPolicy.h src/source/UI/NewUI/HUD/NewUICommandWindow.h src/source/UI/NewUI/HUD/NewUICommandWindow.cpp src/source/UI/NewUI/Dialogs/NewUIChatCommandWindow.h src/source/UI/NewUI/Dialogs/NewUIChatCommandWindow.cpp tests/ui/test_ui_scaling.cpp
git commit -m "fix(ui): keep command panels above HUD"
```

---

### Task 3: Propagate resize dimensions and scale character selection

**Files:**

- Modify: `src/source/Core/Input/Input.h`
- Modify: `src/source/Core/Input/Input.cpp`
- Modify: `src/source/App/Platform/Windows/Winmain.cpp`
- Modify: `src/source/Character/CharSelMainWin.h`
- Modify: `src/source/Character/CharSelMainWin.cpp`
- Modify: `src/source/UI/Legacy/UIMng.cpp`
- Test: `tests/ui/test_ui_scaling.cpp`

**Interfaces:**

- Produces: `CInput::SetScreenSize(long width, long height)`; ignores any
  non-positive pair and changes only cached dimensions.
- Produces: `UI::CharacterSelection::CalculateLayout(int width, int height)`.
- Consumes: native `800x600`, source button frame `54x30`, existing
  `CSprite::SetSize()`, physical `CInput` cursor coordinates.

- [ ] **Step 1: Add failing input-size tests**

Add:

```cpp
#include "Core/Input/Input.h"
```

Add:

```cpp
TEST_CASE("input screen bounds accept only positive resize dimensions [ui][scaling]")
{
    CInput& input = CInput::Instance();
    input.SetScreenSize(800, 600);
    CHECK(input.GetScreenWidth() == 800);
    CHECK(input.GetScreenHeight() == 600);

    input.SetScreenSize(0, 720);
    CHECK(input.GetScreenWidth() == 800);
    CHECK(input.GetScreenHeight() == 600);

    input.SetScreenSize(1280, 1024);
    CHECK(input.GetScreenWidth() == 1280);
    CHECK(input.GetScreenHeight() == 1024);
}
```

- [ ] **Step 2: Add failing character-layout tests**

Add:

```cpp
#include "Character/CharSelMainWin.h"
```

Add scale coverage:

```cpp
TEST_CASE("character selection controls scale from 800x600 and cap at 2x [ui][scaling]")
{
    CHECK(UI::CharacterSelection::CalculateLayout(640, 480).scale == doctest::Approx(1.0f));
    CHECK(UI::CharacterSelection::CalculateLayout(800, 600).scale == doctest::Approx(1.0f));
    CHECK(UI::CharacterSelection::CalculateLayout(1280, 1024).scale == doctest::Approx(1.6f));
    CHECK(UI::CharacterSelection::CalculateLayout(1920, 1080).scale == doctest::Approx(1.8f));
    CHECK(UI::CharacterSelection::CalculateLayout(3840, 2160).scale == doctest::Approx(2.0f));
}
```

Add exact geometry coverage:

```cpp
TEST_CASE("character selection layout keeps scaled controls and symmetric margins [ui][scaling]")
{
    const auto native = UI::CharacterSelection::CalculateLayout(800, 600);
    CHECK(native.window.x == 22);
    CHECK(native.window.y == 526);
    CHECK(native.window.width == 756);
    CHECK(native.window.height == 30);
    CHECK(native.buttons[CSMW_BTN_CREATE].width == 54);
    CHECK(native.buttons[CSMW_BTN_CREATE].height == 30);
    CHECK(native.information.width == 534);

    const auto sxga = UI::CharacterSelection::CalculateLayout(1280, 1024);
    CHECK(sxga.window.x == 35);
    CHECK(sxga.window.y == 902);
    CHECK(sxga.window.width == 1210);
    CHECK(sxga.buttons[CSMW_BTN_CREATE].width == 86);
    CHECK(sxga.buttons[CSMW_BTN_CREATE].height == 48);
    CHECK(sxga.information.width == 856);

    const auto fullHd = UI::CharacterSelection::CalculateLayout(1920, 1080);
    CHECK(fullHd.window.x == 40);
    CHECK(fullHd.window.y == 947);
    CHECK(fullHd.window.width == 1840);
    CHECK(fullHd.buttons[CSMW_BTN_CREATE].width == 97);
    CHECK(fullHd.buttons[CSMW_BTN_CREATE].height == 54);
    CHECK(fullHd.information.width == 1440);

    const auto fourK = UI::CharacterSelection::CalculateLayout(3840, 2160);
    CHECK(fourK.window.x == 44);
    CHECK(fourK.window.width == 3752);
    CHECK(fourK.buttons[CSMW_BTN_CREATE].width == 108);
    CHECK(fourK.buttons[CSMW_BTN_CREATE].height == 60);
    CHECK(fourK.information.width == 3308);
}
```

Add a lightweight sprite rectangle test proving `SetSize()` changes the same
rectangle used by hit testing:

```cpp
TEST_CASE("resized character button rectangle is its click rectangle [ui][scaling]")
{
    const int previousHeight = WindowHeight;
    WindowHeight = 600;

    CButton button;
    button.Create(54, 30, -1);
    button.SetSize(108, 60);
    button.SetPosition(100, 200);
    button.Show();

    CHECK(button.GetWidth() == 108);
    CHECK(button.GetHeight() == 60);
    CHECK(button.PtInSprite(207, 259));
    CHECK_FALSE(button.PtInSprite(208, 259));

    WindowHeight = previousHeight;
}
```

- [ ] **Step 3: Run RED**

Run:

```bash
cmake --build out/build/macos-arm64 --config Release --target ui_scaling_tests -j2
```

Expected: compile failure because `SetScreenSize()` and
`UI::CharacterSelection::CalculateLayout()` do not exist.

- [ ] **Step 4: Add the validated cached-size update**

Declare in `Input.h`:

```cpp
void SetScreenSize(long screenWidth, long screenHeight);
```

Implement with one early exit:

```cpp
void CInput::SetScreenSize(long screenWidth, long screenHeight)
{
    if (screenWidth <= 0 || screenHeight <= 0)
        return;

    m_lScreenWidth = screenWidth;
    m_lScreenHeight = screenHeight;
}
```

Use `SetScreenSize()` from `CInput::Create()` instead of assigning both fields
directly.

In `HandleWindowResize()`, call it after setting `WindowWidth` and
`WindowHeight`, before `UpdateResolutionDependentSystems()` reaches
`CUIMng::RepositionSceneUI()`:

```cpp
CInput::Instance().SetScreenSize(WindowWidth, WindowHeight);
```

- [ ] **Step 5: Add the pure character-selection layout**

In `CharSelMainWin.h`, include `<array>` and define after the existing button
index constants:

```cpp
namespace UI::CharacterSelection
{
struct Rect
{
    int x;
    int y;
    int width;
    int height;
};

struct Layout
{
    float scale;
    Rect window;
    Rect decoration;
    Rect information;
    std::array<Rect, CSMW_BTN_MAX> buttons;
};

Layout CalculateLayout(int screenWidth, int screenHeight);
}
```

Implement in `CharSelMainWin.cpp` using named native constants and
`std::lround()`:

```cpp
const float scale = std::clamp(
    std::min(static_cast<float>(screenWidth) / 800.0f,
             static_cast<float>(screenHeight) / 600.0f),
    1.0f, 2.0f);
```

Use these exact rules:

- scaled outer margin: `22`;
- scaled button: `54x30`;
- scaled button gap: `1`;
- scaled information gap: `2` on each side;
- scaled information top offset: `5`, height `21`;
- scaled decoration: `189x103`, right inset `22`, top offset `59`;
- window width: `max(0, screenWidth - 2 * outerMargin)`;
- information width: remaining space between menu and connect buttons, clamped
  to `0`;
- proportional vertical anchor: `round(567.0f / 600.0f * screenHeight)`;
- window top: anchor minus scaled button height minus scaled `11` gap.

Fill all four button rectangles in screen pixels. The create/menu pair starts
at the left margin; delete/connect starts at the right margin.

- [ ] **Step 6: Apply the layout only to `CCharSelMainWin`**

Keep each textured button's `Create(54, 30, ...)` call unchanged so its source
frame UVs remain `54x30`. Immediately resize its screen rectangle with
`SetSize(layoutRect.width, layoutRect.height)`.

Create the decoration at `189x103`, then resize it. Resize the information
sprite, `CWin`, and every control from the calculated rectangles. Add one
private `ApplyLayout(const UI::CharacterSelection::Layout&)` method so `Create`
stays focused.

Remove the old `nBaseY`/`SetPosition(22, ...)` block for
`m_CharSelMainWin` from `CUIMng::CreateCharacterScene()`; `Create()` now applies
the complete current-screen layout once.

Because these rectangles are physical while `CUIMng::Render()` activates the
legacy logical transform, scope only the character-selection sprites and
buttons to an identity transform inside `RenderControls()`:

```cpp
const UI::Scaling::Transform physicalPixels{1.0f, 1.0f, 0.0f, 0.0f, 1.0f};
UI::Scaling::ScopedActiveTransform scoped(physicalPixels);
```

Keep the existing account-block text outside that scope so its legacy centered
coordinates remain unchanged. Do not alter any other legacy window.

- [ ] **Step 7: Run GREEN**

Run:

```bash
cmake --build out/build/macos-arm64 --config Release --target ui_scaling_tests MuClient -j2
out/build/macos-arm64/tests/ui/Release/ui_scaling_tests
```

Expected: all focused cases pass; client code compiles.

- [ ] **Step 8: Commit the character-selection fix**

Run:

```bash
git diff --check
git add src/source/Core/Input/Input.h src/source/Core/Input/Input.cpp src/source/App/Platform/Windows/Winmain.cpp src/source/Character/CharSelMainWin.h src/source/Character/CharSelMainWin.cpp src/source/UI/Legacy/UIMng.cpp tests/ui/test_ui_scaling.cpp
git commit -m "fix(ui): scale character selection controls"
```

---

### Task 4: Center the map splash against the physical window

**Files:**

- Modify: `src/source/UI/Legacy/UIMapName.h`
- Modify: `src/source/UI/Legacy/UIMapName.cpp`
- Test: `tests/ui/test_ui_scaling.cpp`

**Interfaces:**

- Produces: `UI::MapName::PhysicalLeft(int windowWidth)`.
- Produces: `UI::MapName::ImageWidth == 166.0f` and `ImageHeight == 90.0f`.
- Consumes: physical `WindowWidth`, existing `g_fScreenRate_y`, existing
  unscaled `RenderBitmap(..., false, false)` path.

- [ ] **Step 1: Add the failing centering test**

Add:

```cpp
#include "UI/Legacy/UIMapName.h"
```

Add:

```cpp
TEST_CASE("map splash centers in physical window pixels [ui][scaling]")
{
    CHECK(UI::MapName::ImageWidth == doctest::Approx(166.0f));
    CHECK(UI::MapName::ImageHeight == doctest::Approx(90.0f));
    CHECK(UI::MapName::PhysicalLeft(1280) == doctest::Approx(557.0f));
    CHECK(UI::MapName::PhysicalLeft(1920) == doctest::Approx(877.0f));
}
```

- [ ] **Step 2: Run RED**

Run:

```bash
cmake --build out/build/macos-arm64 --config Release --target ui_scaling_tests -j2
```

Expected: compile failure because `UI::MapName` does not exist.

- [ ] **Step 3: Implement the physical centering calculation**

Declare in `UIMapName.h`:

```cpp
namespace UI::MapName
{
inline constexpr float ImageWidth = 166.0f;
inline constexpr float ImageHeight = 90.0f;

float PhysicalLeft(int windowWidth);
}
```

Implement:

```cpp
float UI::MapName::PhysicalLeft(int windowWidth)
{
    return (static_cast<float>(windowWidth) - ImageWidth) * 0.5f;
}
```

Replace the map-name position macros with local values in `Render()`:

```cpp
const float imageX = UI::MapName::PhysicalLeft(WindowWidth);
const float imageY = 220.0f * g_fScreenRate_y;
```

Use `imageX` for both the optional strife banner and the map splash. Use
`ImageWidth`/`ImageHeight` in the existing UV calculations. Keep the final
`false, false` arguments unchanged. Remove every `GetScreenWidth()` reference
from `UIMapName.cpp`.

- [ ] **Step 4: Run GREEN and source guard**

Run:

```bash
cmake --build out/build/macos-arm64 --config Release --target ui_scaling_tests MuClient -j2
out/build/macos-arm64/tests/ui/Release/ui_scaling_tests
! rg -n "GetScreenWidth\(\)" src/source/UI/Legacy/UIMapName.cpp
```

Expected: focused cases pass; client compiles; source guard exits `0`.

- [ ] **Step 5: Commit the map-splash fix**

Run:

```bash
git diff --check
git add src/source/UI/Legacy/UIMapName.h src/source/UI/Legacy/UIMapName.cpp tests/ui/test_ui_scaling.cpp
git commit -m "fix(ui): center map splash physically"
```

---

### Task 5: Replace teleport resolution switches and recursive scrolling

**Files:**

- Modify: `src/source/UI/NewUI/HUD/NewUIMoveCommandWindow.h`
- Modify: `src/source/UI/NewUI/HUD/NewUIMoveCommandWindow.cpp`
- Test: `tests/ui/test_ui_scaling.cpp`

**Interfaces:**

- Produces: `UI::MoveCommand::Layout` and pure geometry/scroll mapping helpers.
- Consumes: `UI::Scaling::DockLogicalBottom`, measured logical row height,
  existing move-command data, existing scrollbar textures.

- [ ] **Step 1: Add failing teleport geometry tests**

Add:

```cpp
#include "UI/NewUI/HUD/NewUIMoveCommandWindow.h"
```

Add:

```cpp
TEST_CASE("teleport layout uses stable width and fits above the dock [ui][scaling]")
{
    const auto layout = UI::MoveCommand::CalculateLayout(1, 14);
    CHECK(layout.windowWidth == 230);
    CHECK(layout.visibleRows == 26);
    CHECK(layout.windowHeight == 424);
    CHECK(layout.listTop == 39);
    CHECK(layout.closeTop == 405);
    CHECK(layout.scrollTrackTop == 36);
    CHECK(layout.scrollTrackHeight == 364);
    CHECK(layout.thumbTravel == 334);
    CHECK(1 + layout.windowHeight <= UI::Scaling::DockLogicalBottom);
}

TEST_CASE("teleport width is independent of measured row height [ui][scaling]")
{
    CHECK(UI::MoveCommand::CalculateLayout(1, 12).windowWidth == 230);
    CHECK(UI::MoveCommand::CalculateLayout(1, 14).windowWidth == 230);
    CHECK(UI::MoveCommand::CalculateLayout(1, 18).windowWidth == 230);
}
```

- [ ] **Step 2: Add failing bounded-offset tests**

Add:

```cpp
TEST_CASE("teleport scroll offset handles empty short exact and overflow lists [ui][scaling]")
{
    CHECK(UI::MoveCommand::MaximumScrollOffset(0, 26) == 0);
    CHECK(UI::MoveCommand::MaximumScrollOffset(12, 26) == 0);
    CHECK(UI::MoveCommand::MaximumScrollOffset(26, 26) == 0);
    CHECK(UI::MoveCommand::MaximumScrollOffset(27, 26) == 1);
    CHECK(UI::MoveCommand::MaximumScrollOffset(52, 26) == 26);

    CHECK(UI::MoveCommand::ClampScrollOffset(-5, 52, 26) == 0);
    CHECK(UI::MoveCommand::ClampScrollOffset(13, 52, 26) == 13);
    CHECK(UI::MoveCommand::ClampScrollOffset(99, 52, 26) == 26);
}

TEST_CASE("teleport thumb maps first middle and last offsets [ui][scaling]")
{
    const auto layout = UI::MoveCommand::CalculateLayout(1, 14);
    CHECK(UI::MoveCommand::ThumbYForScrollOffset(0, layout, 52) == 36);
    CHECK(UI::MoveCommand::ThumbYForScrollOffset(13, layout, 52) == 203);
    CHECK(UI::MoveCommand::ThumbYForScrollOffset(26, layout, 52) == 370);

    CHECK(UI::MoveCommand::ScrollOffsetForThumbY(36, layout, 52) == 0);
    CHECK(UI::MoveCommand::ScrollOffsetForThumbY(203, layout, 52) == 13);
    CHECK(UI::MoveCommand::ScrollOffsetForThumbY(370, layout, 52) == 26);
}

TEST_CASE("teleport thumb disables cleanly without overflow [ui][scaling]")
{
    const auto layout = UI::MoveCommand::CalculateLayout(1, 14);
    CHECK(UI::MoveCommand::ThumbYForScrollOffset(9, layout, 0) == layout.scrollTrackTop);
    CHECK(UI::MoveCommand::ScrollOffsetForThumbY(layout.scrollTrackTop + 50, layout, 0) == 0);
}
```

- [ ] **Step 3: Run RED**

Run:

```bash
cmake --build out/build/macos-arm64 --config Release --target ui_scaling_tests -j2
```

Expected: compile failure because `UI::MoveCommand` helpers do not exist.

- [ ] **Step 4: Declare the pure layout API**

In `NewUIMoveCommandWindow.h`, add `<cstddef>` and declare before
`namespace SEASON3B`:

```cpp
namespace UI::MoveCommand
{
struct Layout
{
    int windowWidth;
    int windowHeight;
    int visibleRows;
    int listTop;
    int closeTop;
    int scrollTrackTop;
    int scrollTrackHeight;
    int thumbTravel;
};

Layout CalculateLayout(int windowY, int rowHeight);
int MaximumScrollOffset(std::size_t itemCount, int visibleRows);
int ClampScrollOffset(int offset, std::size_t itemCount, int visibleRows);
int ThumbYForScrollOffset(int offset, const Layout& layout, std::size_t itemCount);
int ScrollOffsetForThumbY(int thumbY, const Layout& layout, std::size_t itemCount);
}
```

- [ ] **Step 5: Implement geometry without resolution switches**

Use named constants in `NewUIMoveCommandWindow.cpp`:

```cpp
constexpr int kWindowWidth = 230;
constexpr int kFixedChromeHeight = 60;
constexpr int kListOffsetY = 38;
constexpr int kCloseBottomGap = 6;
constexpr int kDefaultRowHeight = 14;
constexpr int kScrollBarCapHeight = 3;
constexpr int kScrollThumbHeight = 30;
```

`CalculateLayout()` must:

```cpp
const int safeRowHeight = std::max(rowHeight, 1);
const int availableHeight = UI::Scaling::DockLogicalBottom - windowY;
const int visibleRows = std::max(1, (availableHeight - kFixedChromeHeight) / safeRowHeight);
const int windowHeight = kFixedChromeHeight + visibleRows * safeRowHeight;
const int listTop = windowY + kListOffsetY;
const int closeTop = windowY + windowHeight - safeRowHeight - kCloseBottomGap;
const int scrollTrackTop = listTop - kScrollBarCapHeight;
const int scrollTrackHeight = visibleRows * safeRowHeight;
const int thumbTravel = std::max(0, scrollTrackHeight - kScrollThumbHeight);
```

Remove the `WindowWidth` switch entirely. Keep the former 640x480 column
positions for the stable `230` width:

- strife center: `x + 20`;
- map center: `x + 62`;
- required-level center: `x + 119`;
- zen center: `x + 159`;
- scrollbar x: `x + windowWidth - 14`.

- [ ] **Step 6: Implement bounded offset/thumb mapping**

Avoid unsigned underflow by checking the count before subtraction:

```cpp
if (visibleRows <= 0 || itemCount <= static_cast<std::size_t>(visibleRows))
    return 0;
return static_cast<int>(itemCount - static_cast<std::size_t>(visibleRows));
```

Clamp offsets before every use. For a nonzero maximum, map with nearest-integer
rounding:

```cpp
thumbY = trackTop + round(offset * thumbTravel / maxOffset)
offset = round(clamp(thumbY - trackTop, 0, thumbTravel) * maxOffset / thumbTravel)
```

Return `trackTop`/`0` without division when the list does not overflow or thumb
travel is zero.

- [ ] **Step 7: Replace legacy scroll state in the window**

Keep only the state required by the new model:

```cpp
UI::MoveCommand::Layout m_layout{};
int m_scrollOffset{0};
int m_scrollDragGrabOffsetY{0};
```

Keep the existing mouse-state enum and move-command key. Remove:

- `MOVECOMMAND_MAX_RENDER_TEXTLINE`;
- page counters;
- render start/end counters;
- remaining-step/pixel counters;
- min/max pixels-per-step counters;
- accumulated scroll pixels;
- scrollbar middle-count/remainder cache;
- `ScrollUp()`, `ScrollDown()`, `RecursiveCalcScroll()`, and
  `UpdateScrolling()`.

Extract short private methods in the existing class:

```cpp
void RefreshDataAndLayout();
void SetScrollOffset(int offset);
int VisibleEndIndex() const;
void UpdateScrollThumb();
```

`RefreshDataAndLayout()` fetches the current move list, measures the font with
the `14` fallback, calculates/stores geometry, then clamps the existing offset.
`OpenningProcess()` calls it, resets offset and selection, then updates strife
and move eligibility. The frame size never depends on item count.

- [ ] **Step 8: Make input/rendering consume the same geometry**

In `BtnProcess()`:

- activate the thumb only when `MaximumScrollOffset(...) > 0`;
- on press, store `MouseY - currentThumbY`;
- while dragging, map `MouseY - grabOffset` through
  `ScrollOffsetForThumbY()`;
- apply wheel input as row deltas, clamp, then clear `MouseWheel`;
- start visible iteration with a clamped offset and stop at list end;
- use `m_layout.closeTop` plus `m_Pos.x` for the close hit rectangle.

In `RenderFrame()` and `Render()`:

- use `m_layout.windowWidth/windowHeight` for the stable frame;
- tile the scrollbar middle from `m_layout.scrollTrackTop` and
  `m_layout.scrollTrackHeight` without cached page math;
- use `ThumbYForScrollOffset()` for the thumb;
- use `m_layout.visibleRows` for render/hover limits;
- use `m_Pos.x + m_layout.windowWidth / 2` and `m_layout.closeTop` for close
  text;
- guard every iterator increment and selection index against list end.

Make eligibility iteration range-based over the full list. Make `IsMapMove()`
search the full list rather than advancing to a stale render-end index. Preserve
all existing move restrictions and server request behavior.

- [ ] **Step 9: Run GREEN**

Run:

```bash
cmake --build out/build/macos-arm64 --config Release --target ui_scaling_tests MuClient -j2
out/build/macos-arm64/tests/ui/Release/ui_scaling_tests
```

Expected: geometry/mapping cases pass; client compiles; empty and short lists
cannot divide by zero.

- [ ] **Step 10: Verify obsolete state is gone**

Run:

```bash
! rg -n "MOVECOMMAND_MAX_RENDER_TEXTLINE|RecursiveCalcScroll|m_iNumPage|m_iCurPage|m_iRemainMoveScrBtn|m_iTotalMoveScrBtn|m_iAcumMoveMouseScrollPixel" src/source/UI/NewUI/HUD/NewUIMoveCommandWindow.h src/source/UI/NewUI/HUD/NewUIMoveCommandWindow.cpp
```

Expected: command exits `0`.

- [ ] **Step 11: Commit the teleport fix**

Run:

```bash
git diff --check
git add src/source/UI/NewUI/HUD/NewUIMoveCommandWindow.h src/source/UI/NewUI/HUD/NewUIMoveCommandWindow.cpp tests/ui/test_ui_scaling.cpp
git commit -m "fix(ui): stabilize teleport window layout"
```

---

### Task 6: Full verification and native handoff

**Files:** All files changed in Tasks 2-5.

**Produces:** Post-change test/build evidence and a remote-server visual
verification request without launching a process locally.

- [ ] **Step 1: Run focused verification**

Run:

```bash
cmake --build out/build/macos-arm64 --config Release --target ui_scaling_tests -j2
out/build/macos-arm64/tests/ui/Release/ui_scaling_tests
```

Expected: all doctest cases pass. Confirm the new cases exercise shared command
geometry, input bounds, character rectangles, map centering, teleport list
sizes, and both scrollbar directions.

- [ ] **Step 2: Run full CTest**

Run:

```bash
ctest --test-dir out/build/macos-arm64 -C Release --output-on-failure
```

Expected: all registered tests pass. Report the actual total.

- [ ] **Step 3: Build client targets and compare baseline**

Run:

```bash
cmake --build out/build/macos-arm64 --config Release --target MuClient -j2
cmake --build out/build/macos-arm64 --config Release --target Main -j2
```

Expected: `MuClient` succeeds. If `Main` still fails only at the same NativeAOT
`brotlienc` step recorded in Task 1, report it as unchanged pre-existing state.
Any new compile or link failure is caused by this work until proven otherwise.

- [ ] **Step 4: Run static diff checks**

Run:

```bash
git diff --check HEAD~4..HEAD
! git diff HEAD~4..HEAD | rg -n '(/u([0-9]{1,3}\.){3}[0-9]{1,3}|/p[0-9]{2,5})'
! rg -n "GetScreenWidth\(\)" src/source/UI/Legacy/UIMapName.cpp
git status --short
```

Expected: no whitespace errors, hardcoded endpoint, map-panel centering call, or
uncommitted production change.

- [ ] **Step 5: Review the complete implementation against the spec**

Run:

```bash
git diff HEAD~4..HEAD -- src/source/UI/Scaling/UITransform.h src/source/UI/Scaling/UITransform.cpp src/source/UI/NewUI/UILayoutPolicy.h src/source/Core/Input/Input.h src/source/Core/Input/Input.cpp src/source/App/Platform/Windows/Winmain.cpp src/source/Character/CharSelMainWin.h src/source/Character/CharSelMainWin.cpp src/source/UI/Legacy/UIMng.cpp src/source/UI/Legacy/UIMapName.h src/source/UI/Legacy/UIMapName.cpp src/source/UI/NewUI/HUD/NewUICommandWindow.h src/source/UI/NewUI/HUD/NewUICommandWindow.cpp src/source/UI/NewUI/Dialogs/NewUIChatCommandWindow.h src/source/UI/NewUI/Dialogs/NewUIChatCommandWindow.cpp src/source/UI/NewUI/HUD/NewUIMoveCommandWindow.h src/source/UI/NewUI/HUD/NewUIMoveCommandWindow.cpp tests/ui/test_ui_scaling.cpp
```

Confirm:

- only `D`/`J` layer depths changed;
- both command frames end at `432`;
- `CInput` updates before scene recreation;
- only `CCharSelMainWin` receives the targeted physical layout;
- map splash remains `166x90` and panel-independent;
- teleport has stable `230` width and count-independent frame height;
- no recursive scrollbar/page state remains;
- no HUD, camera, world viewport, server configuration, or unrelated dialog
  behavior changed.

- [ ] **Step 6: Request remote-server native visual verification**

Do not launch the client. Ask the user to run:

```bash
../ctl run --release
```

`../ctl` must resolve the connection target from `../.env`; do not print or
copy its values into code, tests, documentation, or commands.

Ask for checks at `1280x1024` and `1920x1080`:

1. Open `D` and `J`; confirm every visible control renders/clicks above the
   HUD.
2. Enter character selection, change resolution without re-entering, confirm
   the bar and all four click targets resize together.
3. Enter a map, trigger the map splash, open inventory and one other side
   panel; confirm the splash remains physically centered and unchanged in
   size.
4. Open teleport with empty/short/long data where available; confirm the frame
   stays `230` logical pixels wide, never collapses, closes correctly, scrolls
   to first/last rows, and thumb dragging matches wheel scrolling.

- [ ] **Step 7: Commit verification corrections only if needed**

If verification required code changes, rerun Steps 1-5, stage only the
correction, and commit with one focused conventional message. Otherwise create
no empty verification commit.
