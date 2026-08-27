# Responsive UI and Font Scaling Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Fill the viewport with edge HUD, dock side panels, keep dialogs capped and centered, preserve world-overlay alignment, and fit readable text to each active layout.

**Architecture:** `CNewUIObj` stores one layout mode assigned centrally from its interface key. `CNewUIManager` activates that object's transform and inverse mouse transform around each call; UI 3D previews reuse their owner's transform. SDL_ttf loads one maximum-size font per role and scales glyph geometry down from the active layout and bounded text box, avoiding resolution-triggered font I/O.

**Tech Stack:** C++20, SDL3, SDL3_ttf, CMake/CTest, doctest.

**Spec:** `docs/superpowers/specs/2026-08-27-text-label-health-bar-ui-scaling-design.md`

## Global Constraints

- Keep the 640x480 logical coordinate system and supported 640x480 minimum.
- Keep world FOV, culling, terrain, and 3D world scale unchanged. Match the
  scene viewport edge to the rendered right dock when panels reserve space.
- Open font files only at startup or explicit font-family reload, never per frame or resize.
- Preserve existing render layer order and input consumption order.
- Keep changes focused; no per-window content redesign or user scale option.

---

### Task 1: Pure layout and typography calculations

**Files:**
- Modify: `src/source/UI/Scaling/UITransform.h`
- Modify: `src/source/UI/Scaling/UITransform.cpp`
- Modify: `tests/ui/test_ui_scaling.cpp`
- Modify: `tests/platform/test_bundled_font_roles.cpp`

**Interfaces:**
- Produces: `UI::Scaling::LayoutMode`, `TransformForLayout(LayoutMode,int,int)`, `DockLeftTransform(int,int)`, `DockRightTransform(int,int)`, `MinimumFontPointSize(FontRole)`, `MaximumFontPointSize(FontRole)`, `FontPointSize(FontRole,const Transform&)`, and `FontScaleForBounds(FontRole,const Transform&,float,float,float,float)`.
- Consumes: existing `Transform`, position/size/inverse helpers, and the 640x480 reference canvas.

- [x] **Step 1: Replace fixed-canvas assertions with failing responsive-layout assertions**

Add tests proving:

```cpp
TEST_CASE("HUD fills the viewport while dialogs stay capped [ui][scaling]")
{
    const auto hud = UI::Scaling::TransformForLayout(UI::Scaling::LayoutMode::Hud, 1920, 1080);
    CHECK(UI::Scaling::PositionX(hud, 640.0f) == doctest::Approx(1920.0f));
    CHECK(UI::Scaling::PositionY(hud, 480.0f) == doctest::Approx(1080.0f));

    const auto dialog = UI::Scaling::TransformForLayout(UI::Scaling::LayoutMode::Dialog, 1920, 1080);
    CHECK(dialog.scaleX == doctest::Approx(1.5f));
    CHECK(dialog.offsetX == doctest::Approx(480.0f));
    CHECK(dialog.offsetY == doctest::Approx(180.0f));
}

TEST_CASE("right dock anchors existing panel columns to the viewport edge [ui][scaling]")
{
    const auto dock = UI::Scaling::DockRightTransform(1920, 1080);
    CHECK(UI::Scaling::PositionX(dock, 450.0f) == doctest::Approx(1635.0f));
    CHECK(UI::Scaling::PositionX(dock, 640.0f) == doctest::Approx(1920.0f));
    CHECK(UI::Scaling::LogicalX(dock, 1635.0f) == doctest::Approx(450.0f));
}

TEST_CASE("layout typography grows gradually and fits bounded controls [ui][scaling]")
{
    const auto dialog = UI::Scaling::PanelTransform(1280, 720);
    CHECK(UI::Scaling::FontPointSize(FontRole::Normal, dialog) == 13);
    CHECK(UI::Scaling::MaximumFontPointSize(FontRole::Normal) == 13);
    CHECK(UI::Scaling::FontScaleForBounds(FontRole::Normal, dialog, 140.0f, 20.0f, 120.0f, 30.0f)
          == doctest::Approx(120.0f / 140.0f));
}
```

Update the platform role test to assert maximum cached sizes `13`, `26`, and `15`, replacing resolution-derived expectations.

- [x] **Step 2: Run RED**

Run:

```bash
cmake --build out/build/macos-arm64 --config Release --target ui_scaling_tests bundled_font_role_tests -j2
out/build/macos-arm64/tests/ui/Release/ui_scaling_tests
```

Expected: compile failure because the responsive APIs do not exist.

- [x] **Step 3: Implement minimal pure calculations**

Use these modes and readable ranges:

```cpp
enum class LayoutMode { Hud, DockLeft, DockRight, Dialog, WorldOverlay };
// Normal/Bold 11..13, Big 22..26, Fixed 13..15.
```

`Hud` and `WorldOverlay` return `ScreenOverlayTransform`. `Dialog` returns `PanelTransform`. Docks reuse the capped uniform panel scale and vertical offset; left uses `offsetX=0`, right uses `offsetX=windowWidth-640*scale`.

Interpolate point size linearly from minimum at vertical scale `1.0` to maximum at the capped panel scale `1.5`, clamped to that range. `FontScaleForBounds` starts at `FontPointSize / MaximumFontPointSize`, reduces for nonzero pixel box width/height, then clamps to `MinimumFontPointSize / MaximumFontPointSize`.

- [x] **Step 4: Run GREEN**

Run both focused binaries. Expected: all assertions pass.

### Task 2: Per-object render and mouse layout

**Files:**
- Create: `src/source/UI/NewUI/UILayoutPolicy.h`
- Create: `src/source/UI/NewUI/UILayoutPolicy.cpp`
- Modify: `src/source/UI/NewUI/NewUIBase.h`
- Modify: `src/source/UI/NewUI/NewUIManager.cpp`
- Modify: `src/source/UI/NewUI/NewUISystem.cpp`
- Modify: `src/source/App/Platform/Windows/Winmain.cpp`
- Modify: `tests/ui/test_ui_scaling.cpp`

**Interfaces:**
- Produces: `UI::Layout::ForInterface(DWORD) -> UI::Scaling::LayoutMode`; `CNewUIObj::SetLayoutMode(LayoutMode)` and `GetLayoutMode() const`.
- Consumes: Task 1 layout transforms, raw window mouse globals, existing interface keys.

- [x] **Step 1: Add failing policy tests**

Assert representative policy outcomes:

```cpp
CHECK(UI::Layout::ForInterface(SEASON3B::INTERFACE_MAINFRAME) == LayoutMode::Hud);
CHECK(UI::Layout::ForInterface(SEASON3B::INTERFACE_INVENTORY) == LayoutMode::DockRight);
CHECK(UI::Layout::ForInterface(SEASON3B::INTERFACE_MOVEMAP) == LayoutMode::DockLeft);
CHECK(UI::Layout::ForInterface(SEASON3B::INTERFACE_MESSAGEBOX) == LayoutMode::Dialog);
CHECK(UI::Layout::ForInterface(SEASON3B::INTERFACE_NAME_WINDOW) == LayoutMode::WorldOverlay);
```

- [x] **Step 2: Run RED**

Build `ui_scaling_tests`. Expected: compile failure because `UILayoutPolicy.h` does not exist.

- [x] **Step 3: Implement the key policy and object storage**

`UI::Layout::ForInterface()` uses explicit switch groups. HUD contains main frame, chat/input/system logs, minimap, party/status widgets, event timers/scores, buffs, hotkeys, duel-watch HUD, and other screen-edge overlays. Dock right contains inventory/character/storage/trade/shop/quest/NPC/helper panels. Move-map docks left. Name window uses world overlay. Modal options, help, message box, centered event prompts, and unknown keys default to dialog.

`CNewUIManager::AddUIObj()` assigns `ForInterface(dwKey)` to `pUIObj` before storing it.

- [x] **Step 4: Scope every manager call**

In `NewUIManager.cpp`, add file-local state helpers:

```cpp
struct LayoutInputState
{
    UI::Scaling::Transform transform;
    int mouseX;
    int mouseY;
};

LayoutInputState ActivateLayout(const CNewUIObj& object, bool transformMouse);
void RestoreLayout(const LayoutInputState& state);
```

`ActivateLayout` saves the transform/mouse, activates `TransformForLayout(object.GetLayoutMode(), WindowWidth, WindowHeight)`, and when requested derives `MouseX/Y` from `g_fWindowMouseX/Y`. Wrap `UpdateMouseEvent`, `Update`, and `Render` object calls; restore before result handling or early return. Preserve sorting and active-object semantics.

- [x] **Step 5: Remove the global panel wrapper**

Delete the panel transform and panel mouse conversion from `CNewUISystem::Update()` and `Render()`. Keep `m_pNewItemMng` and manager ordering unchanged. Remove the redundant local screen-overlay switch in `CNewUINameWindow::Render()` because the manager now supplies it.

For the IME caret, use the focused owner object's active layout when available; otherwise retain dialog layout. Do not hard-code `PanelTransform` after per-object input exists.

- [x] **Step 6: Run focused tests and build**

Run:

```bash
cmake --build out/build/macos-arm64 --config Release --target ui_scaling_tests MuClient Main -j2
out/build/macos-arm64/tests/ui/Release/ui_scaling_tests
```

Expected: tests pass; client targets compile.

### Task 3: Preserve layout for UI 3D previews and queued effects

**Files:**
- Modify: `src/source/UI/NewUI/NewUI3DRenderMng.h`
- Modify: `src/source/UI/NewUI/NewUI3DRenderMng.cpp`
- Modify: `src/source/UI/NewUI/Inventory/NewUIInventoryCtrl.h`
- Modify: `src/source/UI/NewUI/Inventory/NewUIInventoryCtrl.cpp`

**Interfaces:**
- Produces: optional `INewUI3DRenderObj::GetLayoutOwner() const -> CNewUIObj*`; queued `UI_2DEFFECT_INFO::transform`.
- Consumes: `CNewUIObj::GetLayoutMode()` and active transform API.

- [x] **Step 1: Give non-window 3D controls an owner**

Add a default `GetLayoutOwner()` returning `nullptr` to `INewUI3DRenderObj`. `CNewUIInventoryCtrl` returns `m_pOwner`; `CNewUIPickedItem` returns its source inventory's owner when available.

- [x] **Step 2: Render each 3D object in its owner layout**

Before `Render3D()`, resolve the owner by cross-casting the render object to `CNewUIObj`, then falling back to `GetLayoutOwner()`. Save the active transform, activate the owner's layout transform, render, restore. Objects without an owner retain the camera's dialog transform.

- [x] **Step 3: Preserve callback transforms**

Store `UI::Scaling::GetActiveTransform()` in `UI_2DEFFECT_INFO` when queued. Activate that stored transform around the callback, then restore it. This keeps inventory counts, skill tooltips, and other delayed 2D effects aligned with the object that queued them.

- [x] **Step 4: Build client targets**

Build `MuClient` and `Main`. Expected: clean compile with no changed camera/FOV tests.

### Task 4: Layout-relative font geometry and bounded text fit

**Files:**
- Modify: `src/source/App/Platform/Windows/Winmain.cpp`
- Modify: `src/source/Render/Text/CUIRenderTextSDLTtf.h`
- Modify: `src/source/Render/Text/CUIRenderTextSDLTtf.cpp`
- Modify: `tests/platform/test_bundled_font_roles.cpp`

**Interfaces:**
- Consumes: Task 1 font ranges and `FontScaleForBounds`.
- Produces: one maximum-size loaded font per role; `CUIRenderTextSDLTtf` tracks `FontRole` and scales measured/rendered glyph geometry consistently.

- [x] **Step 1: Make maximum role sizes the only loaded sizes**

Replace `CalculateFontSizes()` resolution inputs with `MaximumFontPointSize()` values. Keep startup and explicit font-family reload behavior. Remove `ReinitializeFonts()` from `HandleWindowResize()`; resize only updates transforms and resolution-dependent systems.

- [x] **Step 2: Track active role, not only active font pointer**

Set `m_activeRole` in `SetFont()` alongside the existing maximum-size TTF font pointer. Normal is the default; bold, big, and fixed preserve existing HFONT identity mapping.

- [x] **Step 3: Scale measurement, alignment, and glyph vertices together**

In `RenderText()`:

1. Prepare text once using the maximum-size role font.
2. Convert the logical box to physical pixels.
3. Calculate `textScale = FontScaleForBounds(...)`.
4. Use `prepared.width * textScale` and `prepared.height * textScale` for alignment, background decisions, returned `SIZE`, and fit.
5. Multiply each atlas vertex's local `x/y` by `textScale` in `SubmitTextDrawData()`; do not scale the absolute draw origin.

`MeasureText()` uses the same layout-derived scale without box constraints. Keep the cache key unchanged because the maximum-size font and source text remain stable.

- [x] **Step 4: Run focused text/UI verification**

Run:

```bash
cmake --build out/build/macos-arm64 --config Release --target ui_scaling_tests bundled_font_role_tests sdl_ttf_text_cache_tests MuClient Main -j2
out/build/macos-arm64/tests/ui/Release/ui_scaling_tests
out/build/macos-arm64/tests/platform/Release/bundled_font_role_tests
out/build/macos-arm64/tests/text/Release/sdl_ttf_text_cache_tests
```

Expected: all focused tests pass; client targets build.

### Task 5: Regression verification and documentation consistency

**Files:**
- Modify: `docs/superpowers/specs/2026-08-27-text-label-health-bar-ui-scaling-design.md`
- Modify: `docs/superpowers/plans/2026-08-27-ui-font-scaling.md`

**Interfaces:** None.

- [x] **Step 1: Run configured regression suite**

Run:

```bash
ctest --test-dir out/build/macos-arm64 -C Release --output-on-failure
```

Compare failures with the recorded pre-change focused baseline. Do not claim cross-platform visual success from macOS automation.

- [x] **Step 2: Inspect the final diff**

Run `git diff --check`, `git status --short`, and `git diff --stat`. Confirm no unrelated files, generated assets, or resolution/font reload paths remain.

- [ ] **Step 3: Manual matrix still required**

On Windows, Linux, and macOS, verify 640x480, 800x600, 1280x720, 1920x1080, and 3840x2160: HUD edge anchors, right/left docks, centered dialogs, mouse hit testing, item previews, text fit, NPC/world labels, selected-monster bar, and all three locales.
