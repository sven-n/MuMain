# Responsive Bottom HUD Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Recompose the legacy bottom HUD into uniformly scaled left, centered combat, and right regions while keeping the experience rail full-width and every world/culling boundary aligned to the HUD top.

**Architecture:** Add pure bottom-HUD transforms and one rounded physical world viewport to `UI::Scaling`. Keep the mixed main-frame object on the normal HUD layout, then use a scoped active-transform guard inside it for left, center, right, and stretched rendering/input; assign the independently managed skill list to the center layout. Slice existing frame textures with `RenderImageStretch`, route world projection through a physical-pixel viewport entry point, and replace legacy `y=429` input gates with the shared HUD-region predicate.

**Tech Stack:** C++20, CMake/CTest, doctest, SDL GPU native renderer.

**Spec:** `docs/superpowers/specs/2026-08-28-responsive-bottom-hud-design.md`

## Global Constraints

- Retain the original `640x480` logical coordinates.
- Left utility band is `[0, 152)`; center combat band is `[152, 488)`; right menu band is `[488, 640]`.
- Center order remains `[HP + SD] [hot skills + current skill] [AG + Mana]`.
- Use `hudScale = clamp(min(windowWidth / 640, windowHeight / 480), 1.0, 2.0)`.
- Map logical `y=480` to the physical window bottom for every fixed-content HUD region.
- Stretch only the experience background/progress rail horizontally across the complete physical window.
- Use existing `newui_menu01`, `newui_menu02`, `newui_menu03`, `newui_exbar`, and `Exbar_Master` textures; add no artwork or dependency.
- Keep general HUD/world-overlay transforms unchanged.
- Keep inventory/dialog scaling unchanged.
- Keep chat, minimap, event timers, and other independent HUD widgets unchanged.
- Derive regional mouse coordinates from `g_fWindowMouseX` and `g_fWindowMouseY` through the inverse render transform.
- Compute `hudTop = round(windowHeight - 51 * hudScale)` once; projection, culling, camera frusta, dock alignment, and world/HUD input boundaries consume that shared geometry.
- Preserve all unrelated dirty-worktree changes; stage only files named by each task.
- Follow `docs/CODING_RULES.md`; no broad legacy cleanup.

---

## File and Interface Map

**Modify**

- `src/source/UI/Scaling/UITransform.h` — declare bottom-HUD layout modes, transforms, physical viewport, HUD hit predicate, and scoped transform guard.
- `src/source/UI/Scaling/UITransform.cpp` — calculate capped uniform HUD geometry, one rounded HUD top, dock alignment, regional hit testing, and scoped active/mouse restoration.
- `src/source/UI/NewUI/UILayoutPolicy.cpp` — route `INTERFACE_SKILL_LIST` to `HudCenter`; leave mixed `INTERFACE_MAINFRAME` and world-command `INTERFACE_HOTKEY` on `Hud`.
- `src/source/UI/NewUI/NewUIManager.cpp` — replace duplicate manual active-transform/mouse save-restore with `ScopedActiveTransform`.
- `src/source/UI/NewUI/NewUI3DRenderMng.cpp` — replace manual transform restoration with the same guard.
- `src/source/UI/NewUI/HUD/NewUIMainFrameWindow.h` — split main-frame rendering into left, center, right, and experience helpers.
- `src/source/UI/NewUI/HUD/NewUIMainFrameWindow.cpp` — render UV slices under regional transforms; route item previews/counts, gauges, skills, buttons, tooltips, and input to matching transforms.
- `src/source/Render/Textures/ZzzOpenglUtil.h` — declare a physical-pixel `BeginOpenglPhysical` entry point.
- `src/source/Render/Textures/ZzzOpenglUtil.cpp` — share projection setup between logical and physical viewport entry points without double-scaling.
- `src/source/Scenes/MainScene.cpp` — consume the physical world viewport for render passes and shared HUD hit geometry for mouse blocking.
- `src/source/Render/Terrain/ZzzLodTerrain.cpp` — consume `WorldViewportAspect` derived from the same rounded viewport.
- `src/source/Camera/DefaultCamera.cpp` — keep default-camera frustum aspect on the shared physical viewport.
- `src/source/Camera/OrbitalCamera.cpp` — keep orbital-camera frustum aspect on the shared physical viewport.
- `src/source/Input/Selection.cpp` — remove the legacy rectangular `y=429` world-pick gate; block only actual HUD regions/other active UI.
- `src/source/Engine/Object/ZzzInterface.cpp` — replace the legacy attack boundary with the shared HUD-region predicate.
- `src/source/UI/NewUI/HUD/NewUIHotKey.cpp` — keep world interactions on screen-overlay coordinates; replace its legacy bottom boundary with the shared predicate.
- `src/source/UI/NewUI/Inventory/NewUIMyInventory.cpp` — replace the picked-item legacy bottom boundary with the shared predicate.
- `tests/ui/test_ui_scaling.cpp` — literal transform, inverse, viewport, docking, hit-region, scope restoration, and policy expectations.

## Stable Interfaces

```cpp
namespace UI::Scaling
{
    enum class LayoutMode
    {
        Hud,
        HudLeft,
        HudCenter,
        HudRight,
        HudExperience,
        DockLeft,
        DockRight,
        Dialog,
        WorldOverlay,
    };

    float BottomHudScale(int windowWidth, int windowHeight);
    Transform BottomHudLeftTransform(int windowWidth, int windowHeight);
    Transform BottomHudCenterTransform(int windowWidth, int windowHeight);
    Transform BottomHudRightTransform(int windowWidth, int windowHeight);
    Transform BottomHudExperienceTransform(int windowWidth, int windowHeight);
    Viewport WorldViewport(int windowWidth, int windowHeight, bool topViewEnabled);
    bool BottomHudContainsWindowPoint(int windowWidth, int windowHeight, float windowX, float windowY);

    class ScopedActiveTransform
    {
    public:
        explicit ScopedActiveTransform(const Transform& transform, bool transformMouse = false);
        ~ScopedActiveTransform();
        ScopedActiveTransform(const ScopedActiveTransform&) = delete;
        ScopedActiveTransform& operator=(const ScopedActiveTransform&) = delete;

    private:
        Transform m_previousTransform;
        int m_previousMouseX;
        int m_previousMouseY;
        bool m_restoreMouse;
    };
}

void BeginOpenglPhysical(int x, int y, int width, int height);
```

### Task 1: Bottom-HUD Geometry and Shared Hit Regions

**Files:**
- Modify: `src/source/UI/Scaling/UITransform.h`
- Modify: `src/source/UI/Scaling/UITransform.cpp`
- Test: `tests/ui/test_ui_scaling.cpp`

**Interfaces:**
- Consumes: existing `UI::Scaling::Transform`, `Viewport`, `PositionX`, `PositionY`, `LogicalX`, `LogicalY`.
- Produces: `BottomHudScale`, four regional transforms, `WorldViewport`, `BottomHudContainsWindowPoint`, and updated dock transforms.

- [ ] **Step 1: Replace fixed-HUD tests with literal responsive geometry tests**

Add these cases to `tests/ui/test_ui_scaling.cpp`; replace the old `WorldReferenceViewport` and fixed `432` expectations rather than retaining contradictory tests.

Add these declarations below the includes because the scope test exercises existing application globals directly:

```cpp
extern int MouseX;
extern int MouseY;
extern float g_fWindowMouseX;
extern float g_fWindowMouseY;
```

```cpp
TEST_CASE("bottom HUD regions reconstruct at 640x480 and 1024x768 [ui][scaling]")
{
    const auto referenceLeft = UI::Scaling::BottomHudLeftTransform(640, 480);
    const auto referenceCenter = UI::Scaling::BottomHudCenterTransform(640, 480);
    const auto referenceRight = UI::Scaling::BottomHudRightTransform(640, 480);
    CHECK(referenceLeft.scaleX == doctest::Approx(1.0f));
    CHECK(referenceCenter.offsetX == doctest::Approx(0.0f));
    CHECK(referenceRight.offsetX == doctest::Approx(0.0f));
    CHECK(UI::Scaling::PositionX(referenceLeft, 152.0f) == doctest::Approx(152.0f));
    CHECK(UI::Scaling::PositionX(referenceCenter, 152.0f) == doctest::Approx(152.0f));
    CHECK(UI::Scaling::PositionX(referenceCenter, 488.0f) == doctest::Approx(488.0f));
    CHECK(UI::Scaling::PositionX(referenceRight, 488.0f) == doctest::Approx(488.0f));

    const auto left = UI::Scaling::BottomHudLeftTransform(1024, 768);
    const auto center = UI::Scaling::BottomHudCenterTransform(1024, 768);
    const auto right = UI::Scaling::BottomHudRightTransform(1024, 768);
    CHECK(left.scaleX == doctest::Approx(1.6f));
    CHECK(UI::Scaling::PositionX(left, 152.0f) == doctest::Approx(243.2f));
    CHECK(UI::Scaling::PositionX(center, 152.0f) == doctest::Approx(243.2f));
    CHECK(UI::Scaling::PositionX(center, 488.0f) == doctest::Approx(780.8f));
    CHECK(UI::Scaling::PositionX(right, 488.0f) == doctest::Approx(780.8f));
}

TEST_CASE("bottom HUD uses symmetric wide gaps and caps at 2x [ui][scaling]")
{
    const auto hdLeft = UI::Scaling::BottomHudLeftTransform(1280, 720);
    const auto hdCenter = UI::Scaling::BottomHudCenterTransform(1280, 720);
    const auto hdRight = UI::Scaling::BottomHudRightTransform(1280, 720);
    CHECK(hdCenter.scaleX == doctest::Approx(1.5f));
    CHECK(UI::Scaling::PositionX(hdLeft, 152.0f) == doctest::Approx(228.0f));
    CHECK(UI::Scaling::PositionX(hdCenter, 152.0f) == doctest::Approx(388.0f));
    CHECK(UI::Scaling::PositionX(hdCenter, 320.0f) == doctest::Approx(640.0f));
    CHECK(UI::Scaling::PositionX(hdCenter, 488.0f) == doctest::Approx(892.0f));
    CHECK(UI::Scaling::PositionX(hdRight, 488.0f) == doctest::Approx(1052.0f));

    const auto wideLeft = UI::Scaling::BottomHudLeftTransform(1920, 1200);
    const auto wideCenter = UI::Scaling::BottomHudCenterTransform(1920, 1200);
    const auto wideRight = UI::Scaling::BottomHudRightTransform(1920, 1200);
    CHECK(wideCenter.scaleX == doctest::Approx(2.0f));
    CHECK(UI::Scaling::PositionX(wideLeft, 0.0f) == doctest::Approx(0.0f));
    CHECK(UI::Scaling::PositionX(wideCenter, 320.0f) == doctest::Approx(960.0f));
    CHECK(UI::Scaling::PositionX(wideRight, 640.0f) == doctest::Approx(1920.0f));
    CHECK(UI::Scaling::PositionY(wideCenter, 429.0f) == doctest::Approx(1098.0f));
}

TEST_CASE("bottom HUD regional transforms round trip window positions [ui][scaling]")
{
    const auto left = UI::Scaling::BottomHudLeftTransform(1920, 1200);
    const auto center = UI::Scaling::BottomHudCenterTransform(1920, 1200);
    const auto right = UI::Scaling::BottomHudRightTransform(1920, 1200);
    CHECK(UI::Scaling::LogicalX(left, UI::Scaling::PositionX(left, 80.0f)) == doctest::Approx(80.0f));
    CHECK(UI::Scaling::LogicalX(center, UI::Scaling::PositionX(center, 320.0f)) == doctest::Approx(320.0f));
    CHECK(UI::Scaling::LogicalX(right, UI::Scaling::PositionX(right, 560.0f)) == doctest::Approx(560.0f));
    CHECK(UI::Scaling::LogicalY(center, UI::Scaling::PositionY(center, 450.0f)) == doctest::Approx(450.0f));
}

TEST_CASE("experience transform spans the window with HUD vertical scale [ui][scaling]")
{
    const auto experience = UI::Scaling::BottomHudExperienceTransform(1920, 1200);
    CHECK(experience.scaleX == doctest::Approx(3.0f));
    CHECK(experience.scaleY == doctest::Approx(2.0f));
    CHECK(UI::Scaling::PositionX(experience, 0.0f) == doctest::Approx(0.0f));
    CHECK(UI::Scaling::PositionX(experience, 640.0f) == doctest::Approx(1920.0f));
    CHECK(UI::Scaling::PositionY(experience, 480.0f) == doctest::Approx(1200.0f));
}

TEST_CASE("world viewport HUD top and dock bottom share one rounded pixel [ui][scaling]")
{
    const auto hd = UI::Scaling::WorldViewport(1280, 720, false);
    CHECK(hd.width == 1280);
    CHECK(hd.height == 644);
    CHECK(UI::Scaling::WorldViewportAspect(1280, 720, false) == doctest::Approx(1280.0f / 644.0f));

    const auto sxga = UI::Scaling::WorldViewport(1280, 1024, false);
    CHECK(sxga.height == 922);
    const auto dock = UI::Scaling::DockLeftTransform(1280, 1024);
    CHECK(UI::Scaling::PositionY(dock, 432.0f) == doctest::Approx(922.0f));

    const auto topView = UI::Scaling::WorldViewport(1920, 1200, true);
    CHECK(topView.width == 1920);
    CHECK(topView.height == 1200);
}

TEST_CASE("bottom HUD hit regions block controls but preserve wide gaps [ui][scaling]")
{
    CHECK(UI::Scaling::BottomHudContainsWindowPoint(1280, 720, 100.0f, 660.0f));
    CHECK(UI::Scaling::BottomHudContainsWindowPoint(1280, 720, 640.0f, 660.0f));
    CHECK(UI::Scaling::BottomHudContainsWindowPoint(1280, 720, 1180.0f, 660.0f));
    CHECK_FALSE(UI::Scaling::BottomHudContainsWindowPoint(1280, 720, 300.0f, 660.0f));
    CHECK_FALSE(UI::Scaling::BottomHudContainsWindowPoint(1280, 720, 980.0f, 660.0f));
    CHECK(UI::Scaling::BottomHudContainsWindowPoint(1280, 720, 300.0f, 710.0f));
}
```

- [ ] **Step 2: Run the focused test and verify RED**

Run:

```bash
cmake --build build_validation --target ui_scaling_tests -j2
```

Expected: compile failure naming missing `BottomHudLeftTransform`, `BottomHudCenterTransform`, `BottomHudRightTransform`, `BottomHudExperienceTransform`, `WorldViewport`, and `BottomHudContainsWindowPoint`.

- [ ] **Step 3: Add the minimal geometry API**

In `UITransform.h`, add the four `Hud*` modes and the stable declarations from this plan. In `UITransform.cpp`, replace fixed HUD geometry with named constants and these calculations:

```cpp
constexpr int kReferenceWidth = 640;
constexpr int kReferenceHeight = 480;
constexpr float kHudFrameHeight = 51.0f;
constexpr float kHudContentTop = 429.0f;
constexpr float kExperienceTop = 470.0f;
constexpr float kLeftBandEnd = 152.0f;
constexpr float kCenterBandStart = 152.0f;
constexpr float kCenterBandEnd = 488.0f;
constexpr float kRightBandStart = 488.0f;
constexpr float kMinimumHudScale = 1.0f;
constexpr float kMaximumHudScale = 2.0f;
constexpr float kDockLogicalBottom = 432.0f;

float UI::Scaling::BottomHudScale(int windowWidth, int windowHeight)
{
    const float widthScale = static_cast<float>(windowWidth) / kReferenceWidth;
    const float heightScale = static_cast<float>(windowHeight) / kReferenceHeight;
    return std::clamp(std::min(widthScale, heightScale), kMinimumHudScale, kMaximumHudScale);
}

namespace
{
UI::Scaling::Transform BottomHudTransform(int windowWidth, int windowHeight, float offsetX)
{
    const float scale = UI::Scaling::BottomHudScale(windowWidth, windowHeight);
    return {scale, scale, offsetX, static_cast<float>(windowHeight) - kReferenceHeight * scale, scale};
}

bool ContainsLogicalRect(const UI::Scaling::Transform& transform, float windowX, float windowY,
                         float left, float top, float right, float bottom)
{
    const float x = UI::Scaling::LogicalX(transform, windowX);
    const float y = UI::Scaling::LogicalY(transform, windowY);
    return x >= left && x < right && y >= top && y < bottom;
}
}

UI::Scaling::Transform UI::Scaling::BottomHudLeftTransform(int windowWidth, int windowHeight)
{
    return BottomHudTransform(windowWidth, windowHeight, 0.0f);
}

UI::Scaling::Transform UI::Scaling::BottomHudCenterTransform(int windowWidth, int windowHeight)
{
    const float scale = BottomHudScale(windowWidth, windowHeight);
    return BottomHudTransform(windowWidth, windowHeight,
                              static_cast<float>(windowWidth) * 0.5f - 320.0f * scale);
}

UI::Scaling::Transform UI::Scaling::BottomHudRightTransform(int windowWidth, int windowHeight)
{
    const float scale = BottomHudScale(windowWidth, windowHeight);
    return BottomHudTransform(windowWidth, windowHeight,
                              static_cast<float>(windowWidth) - kReferenceWidth * scale);
}

UI::Scaling::Transform UI::Scaling::BottomHudExperienceTransform(int windowWidth, int windowHeight)
{
    Transform transform = BottomHudLeftTransform(windowWidth, windowHeight);
    transform.scaleX = static_cast<float>(windowWidth) / kReferenceWidth;
    return transform;
}

UI::Scaling::Viewport UI::Scaling::WorldViewport(int windowWidth, int windowHeight, bool topViewEnabled)
{
    if (topViewEnabled)
        return {0, 0, windowWidth, windowHeight};

    const float hudTop = static_cast<float>(windowHeight) - kHudFrameHeight * BottomHudScale(windowWidth, windowHeight);
    return {0, 0, windowWidth, static_cast<int>(std::lround(hudTop))};
}

bool UI::Scaling::BottomHudContainsWindowPoint(int windowWidth, int windowHeight, float windowX, float windowY)
{
    const Transform experience = BottomHudExperienceTransform(windowWidth, windowHeight);
    if (ContainsLogicalRect(experience, windowX, windowY, 0.0f, kExperienceTop,
                            static_cast<float>(kReferenceWidth), static_cast<float>(kReferenceHeight)))
        return true;

    const Transform left = BottomHudLeftTransform(windowWidth, windowHeight);
    if (ContainsLogicalRect(left, windowX, windowY, 0.0f, kHudContentTop, kLeftBandEnd,
                            static_cast<float>(kReferenceHeight)))
        return true;

    const Transform center = BottomHudCenterTransform(windowWidth, windowHeight);
    if (ContainsLogicalRect(center, windowX, windowY, kCenterBandStart, kHudContentTop, kCenterBandEnd,
                            static_cast<float>(kReferenceHeight)))
        return true;

    const Transform right = BottomHudRightTransform(windowWidth, windowHeight);
    return ContainsLogicalRect(right, windowX, windowY, kRightBandStart, kHudContentTop,
                               static_cast<float>(kReferenceWidth), static_cast<float>(kReferenceHeight));
}
```

Update `DockTransform`, `WorldViewportAspect`, and `TransformForLayout`:

```cpp
UI::Scaling::Transform DockTransform(int windowWidth, int windowHeight)
{
    UI::Scaling::Transform transform = UI::Scaling::PanelTransform(windowWidth, windowHeight);
    const auto viewport = UI::Scaling::WorldViewport(windowWidth, windowHeight, false);
    transform.offsetY = static_cast<float>(viewport.height) - kDockLogicalBottom * transform.scaleY;
    return transform;
}

float UI::Scaling::WorldViewportAspect(int windowWidth, int windowHeight, bool topViewEnabled)
{
    const Viewport viewport = WorldViewport(windowWidth, windowHeight, topViewEnabled);
    return static_cast<float>(viewport.width) / viewport.height;
}
```

Map `HudLeft`, `HudCenter`, `HudRight`, and `HudExperience` to their matching transforms. Keep `Hud` and `WorldOverlay` mapped to `ScreenOverlayTransform`.

```cpp
UI::Scaling::Transform UI::Scaling::TransformForLayout(LayoutMode mode, int windowWidth, int windowHeight)
{
    if (mode == LayoutMode::Hud || mode == LayoutMode::WorldOverlay)
        return ScreenOverlayTransform(windowWidth, windowHeight);
    if (mode == LayoutMode::HudLeft)
        return BottomHudLeftTransform(windowWidth, windowHeight);
    if (mode == LayoutMode::HudCenter)
        return BottomHudCenterTransform(windowWidth, windowHeight);
    if (mode == LayoutMode::HudRight)
        return BottomHudRightTransform(windowWidth, windowHeight);
    if (mode == LayoutMode::HudExperience)
        return BottomHudExperienceTransform(windowWidth, windowHeight);
    if (mode == LayoutMode::DockLeft)
        return DockLeftTransform(windowWidth, windowHeight);
    if (mode == LayoutMode::DockRight)
        return DockRightTransform(windowWidth, windowHeight);
    return PanelTransform(windowWidth, windowHeight);
}
```

Remove the obsolete `WorldReferenceViewport(bool)` declaration and definition after all callers/tests use `WorldViewport(int, int, bool)`.

- [ ] **Step 4: Run focused tests and verify GREEN**

Run:

```bash
cmake --build build_validation --target ui_scaling_tests -j2
ctest --test-dir build_validation -R '^ui_scaling_tests$' --output-on-failure
```

Expected: build succeeds; `ui_scaling_tests` passes.

- [ ] **Step 5: Commit geometry**

```bash
git add src/source/UI/Scaling/UITransform.h src/source/UI/Scaling/UITransform.cpp tests/ui/test_ui_scaling.cpp
git commit -m "feat(ui): add responsive HUD geometry"
```

### Task 2: Scoped Transform Restoration and Layout Policy

**Files:**
- Modify: `src/source/UI/Scaling/UITransform.h`
- Modify: `src/source/UI/Scaling/UITransform.cpp`
- Modify: `src/source/UI/NewUI/UILayoutPolicy.cpp`
- Modify: `src/source/UI/NewUI/NewUIManager.cpp`
- Modify: `src/source/UI/NewUI/NewUI3DRenderMng.cpp`
- Test: `tests/ui/test_ui_scaling.cpp`

**Interfaces:**
- Consumes: `TransformForLayout`, `GetActiveTransform`, `SetActiveTransform`, `LogicalX`, `LogicalY`, `g_fWindowMouseX`, `g_fWindowMouseY`.
- Produces: `ScopedActiveTransform`; `INTERFACE_SKILL_LIST -> LayoutMode::HudCenter`; exception/early-return-safe active transform and optional logical mouse restoration.

- [ ] **Step 1: Add failing scope and policy tests**

Append:

```cpp
TEST_CASE("scoped active transform restores transform and logical mouse [ui][scaling]")
{
    const auto previousTransform = UI::Scaling::GetActiveTransform();
    const int previousMouseX = MouseX;
    const int previousMouseY = MouseY;
    const float previousWindowMouseX = g_fWindowMouseX;
    const float previousWindowMouseY = g_fWindowMouseY;

    const UI::Scaling::Transform base{1.0f, 1.0f, 0.0f, 0.0f, 1.0f};
    const UI::Scaling::Transform regional{2.0f, 2.0f, 100.0f, 200.0f, 2.0f};
    UI::Scaling::SetActiveTransform(base);
    MouseX = 7;
    MouseY = 9;
    g_fWindowMouseX = 300.0f;
    g_fWindowMouseY = 500.0f;

    {
        UI::Scaling::ScopedActiveTransform scoped(regional, true);
        CHECK(UI::Scaling::GetActiveTransform().offsetX == doctest::Approx(100.0f));
        CHECK(MouseX == 100);
        CHECK(MouseY == 150);
    }

    CHECK(UI::Scaling::GetActiveTransform().offsetX == doctest::Approx(0.0f));
    CHECK(MouseX == 7);
    CHECK(MouseY == 9);

    UI::Scaling::SetActiveTransform(previousTransform);
    MouseX = previousMouseX;
    MouseY = previousMouseY;
    g_fWindowMouseX = previousWindowMouseX;
    g_fWindowMouseY = previousWindowMouseY;
}
```

Change the policy case to include:

```cpp
CHECK(UI::Layout::ForInterface(SEASON3B::INTERFACE_MAINFRAME) == LayoutMode::Hud);
CHECK(UI::Layout::ForInterface(SEASON3B::INTERFACE_SKILL_LIST) == LayoutMode::HudCenter);
CHECK(UI::Layout::ForInterface(SEASON3B::INTERFACE_HOTKEY) == LayoutMode::Hud);
```

- [ ] **Step 2: Run the focused test and verify RED**

Run:

```bash
cmake --build build_validation --target ui_scaling_tests -j2
```

Expected: compile failure naming missing `ScopedActiveTransform` and/or failed policy expectation for `INTERFACE_SKILL_LIST`.

- [ ] **Step 3: Implement the scoped guard**

Declare the class exactly as shown in Stable Interfaces. Implement:

```cpp
UI::Scaling::ScopedActiveTransform::ScopedActiveTransform(const Transform& transform, bool transformMouse)
    : m_previousTransform(GetActiveTransform()),
      m_previousMouseX(MouseX),
      m_previousMouseY(MouseY),
      m_restoreMouse(transformMouse)
{
    SetActiveTransform(transform);
    if (!m_restoreMouse)
        return;

    MouseX = static_cast<int>(std::floor(LogicalX(transform, g_fWindowMouseX)));
    MouseY = static_cast<int>(std::floor(LogicalY(transform, g_fWindowMouseY)));
}

UI::Scaling::ScopedActiveTransform::~ScopedActiveTransform()
{
    if (m_restoreMouse)
    {
        MouseX = m_previousMouseX;
        MouseY = m_previousMouseY;
    }
    SetActiveTransform(m_previousTransform);
}
```

- [ ] **Step 4: Adopt the guard in manager and deferred 3D/2D paths**

In `NewUIManager.cpp`, delete `LayoutInputState`, `ActivateLayout`, and `RestoreLayout`. At each existing call site use:

```cpp
const auto transform = UI::Scaling::TransformForLayout((*vi)->GetLayoutMode(), WindowWidth, WindowHeight);
UI::Scaling::ScopedActiveTransform layout(transform, true);
```

Use `false` only in `CNewUIManager::Render()`. Scope the guard inside the current loop body so it destructs before inspecting results or returning.

In `NewUI3DRenderMng.cpp`, replace both manual previous-transform blocks:

```cpp
UI::Scaling::ScopedActiveTransform layout(TransformForOwner(object));
object->Render3D();
```

```cpp
UI::Scaling::ScopedActiveTransform layout(UI2DEffectInfo.transform);
(*UI2DEffectInfo.pCallbackFunc)(UI2DEffectInfo.pClass, UI2DEffectInfo.dwParamA, UI2DEffectInfo.dwParamB);
```

- [ ] **Step 5: Route the independent skill-list object to center**

Move only `INTERFACE_SKILL_LIST` out of the `LayoutMode::Hud` group:

```cpp
case INTERFACE_SKILL_LIST:
    return LayoutMode::HudCenter;
```

Keep `INTERFACE_MAINFRAME` and `INTERFACE_HOTKEY` in `LayoutMode::Hud`; the main frame owns multiple regions, while `CNewUIHotKey` owns unrelated world commands.

- [ ] **Step 6: Run tests and verify GREEN**

Run:

```bash
cmake --build build_validation --target ui_scaling_tests MuClient -j2
ctest --test-dir build_validation -R '^ui_scaling_tests$' --output-on-failure
```

Expected: `ui_scaling_tests` passes; `MuClient` compiles.

- [ ] **Step 7: Commit scoped layouts**

```bash
git add src/source/UI/Scaling/UITransform.h src/source/UI/Scaling/UITransform.cpp src/source/UI/NewUI/UILayoutPolicy.cpp src/source/UI/NewUI/NewUIManager.cpp src/source/UI/NewUI/NewUI3DRenderMng.cpp tests/ui/test_ui_scaling.cpp
git commit -m "refactor(ui): scope responsive layouts"
```

### Task 3: Physical World Viewport and Shared Projection Aspect

**Files:**
- Modify: `src/source/Render/Textures/ZzzOpenglUtil.h`
- Modify: `src/source/Render/Textures/ZzzOpenglUtil.cpp`
- Modify: `src/source/Scenes/MainScene.cpp`
- Modify: `src/source/Render/Terrain/ZzzLodTerrain.cpp`
- Modify: `src/source/Camera/DefaultCamera.cpp`
- Modify: `src/source/Camera/OrbitalCamera.cpp`
- Test: `tests/ui/test_ui_scaling.cpp`

**Interfaces:**
- Consumes: `UI::Scaling::WorldViewport(int, int, bool)` and `WorldViewportAspect(int, int, bool)` from Task 1.
- Produces: `BeginOpenglPhysical(int, int, int, int)`; exact physical viewport reuse by main render passes, legacy and modern frusta, and terrain culling.

- [ ] **Step 1: Route the main viewport to the intended physical API and verify RED**

Change only the initial main-scene viewport call to `BeginOpenglPhysical(...)` before declaring the API. This compiler-level integration contract avoids tests that assert source text.

- [ ] **Step 2: Build the affected target and verify RED**

Run:

```bash
cmake --build build_validation --target MuClient -j2
```

Expected: compile failure naming missing `BeginOpenglPhysical`.

- [ ] **Step 3: Add a physical-pixel projection entry point**

Declare in `ZzzOpenglUtil.h`:

```cpp
void BeginOpenglPhysical(int x, int y, int width, int height);
```

Extract the existing projection/state body into the physical entry point. Keep logical callers unchanged by converting once, then delegating:

```cpp
void BeginOpengl(int x, int y, int width, int height)
{
    x = static_cast<int>(ConvertPositionX(static_cast<float>(x)));
    y = static_cast<int>(ConvertPositionY(static_cast<float>(y)));
    width = static_cast<int>(ConvertX(static_cast<float>(width)));
    height = static_cast<int>(ConvertY(static_cast<float>(height)));
    BeginOpenglPhysical(x, y, width, height);
}

void BeginOpenglPhysical(int x, int y, int width, int height)
{
    mu::GetRenderer().SetMatrixMode(GL_PROJECTION);
    mu::GetRenderer().PushMatrix();
    mu::GetRenderer().LoadIdentity();
    SetRenderViewport(x, y, width, height);

    const float aspectRatio = static_cast<float>(width) / height;
    CameraProjection::SetupPerspective(g_Camera, g_Camera.FOV, aspectRatio, g_Camera.ViewNear,
                                       g_Camera.ViewFar * RENDER_DISTANCE_MULTIPLIER);

    mu::GetRenderer().SetMatrixMode(GL_MODELVIEW);
    mu::GetRenderer().PushMatrix();
    mu::GetRenderer().LoadIdentity();
    mu::GetRenderer().Rotate(g_Camera.Angle[1], 0.0f, 1.0f, 0.0f);
    if (!g_Camera.TopViewEnable)
        mu::GetRenderer().Rotate(g_Camera.Angle[0], 1.0f, 0.0f, 0.0f);
    mu::GetRenderer().Rotate(g_Camera.Angle[2], 0.0f, 0.0f, 1.0f);
    mu::GetRenderer().Translate(-g_Camera.Position[0], -g_Camera.Position[1], -g_Camera.Position[2]);

    mu::GetRenderer().SetAlphaTest(false);
    mu::GetRenderer().SetTexture2D(true);
    mu::GetRenderer().SetDepthTest(true);
    mu::GetRenderer().SetCullFace(true);
    mu::GetRenderer().SetDepthMask(true);

    AlphaTestEnable = false;
    TextureEnable = true;
    DepthTestEnable = true;
    CullFaceEnable = true;
    DepthMaskEnable = true;
    mu::GetRenderer().SetDepthFunc(GL_LEQUAL);
    mu::GetRenderer().SetAlphaFunc(GL_GREATER, 0.25f);
    if (FogEnable)
    {
        float fogStart = g_Camera.ViewFar * 1.00f;
        float fogEnd = g_Camera.ViewFar * 1.25f;

#ifdef _EDITOR
        if (ICamera* active = CameraManager_Instance().GetActiveCamera())
        {
            const char* name = active->GetName();
            if (DevEditor_IsCameraOverrideEnabled(name))
                DevEditor_GetCameraFogRange(name, g_Camera.ViewFar, &fogStart, &fogEnd);
        }
#endif

        mu::FogParams fogParams{};
        fogParams.mode = GL_LINEAR;
        fogParams.start = fogStart;
        fogParams.end = fogEnd;
        fogParams.density = FogDensity;
        fogParams.color[0] = FogColor[0];
        fogParams.color[1] = FogColor[1];
        fogParams.color[2] = FogColor[2];
        fogParams.color[3] = FogColor[3];
        mu::GetRenderer().SetFog(fogParams);
    }
    else
    {
        mu::GetRenderer().SetFogEnabled(false);
    }

    CameraProjection::GetOpenGLMatrix(g_Camera.Matrix);
}
```

- [ ] **Step 4: Consume the physical viewport in every main-scene render pass**

Change `SetupMainSceneViewport`:

```cpp
const auto viewport = UI::Scaling::WorldViewport(WindowWidth, WindowHeight, g_Camera.TopViewEnable);
outWidth = viewport.width;
outHeight = viewport.height;
BeginOpenglPhysical(viewport.x, viewport.y, viewport.width, viewport.height);
CreateFrustrum(static_cast<float>(viewport.width) / WindowWidth,
               static_cast<float>(viewport.height) / WindowHeight, cameraPos);
```

Change both water-terrain re-entry calls from `BeginOpengl(0, 0, width, height)` to:

```cpp
BeginOpenglPhysical(0, 0, width, height);
```

Do not change login/character/loading scenes; they remain logical-reference callers.

- [ ] **Step 5: Keep every culling/frustum consumer on `WorldViewportAspect`**

In `ZzzLodTerrain.cpp`, `DefaultCamera.cpp`, and `OrbitalCamera.cpp`, retain the main-scene branch but ensure it calls exactly:

```cpp
UI::Scaling::WorldViewportAspect(WindowWidth, WindowHeight, g_Camera.TopViewEnable)
```

Delete any main-scene fallback that recomputes `windowWidth / windowHeight`, `432 / 480`, or an independently rounded viewport. Preserve non-main-scene behavior.

- [ ] **Step 6: Build projection consumers and run geometry tests**

Run:

```bash
cmake --build build_validation --target ui_scaling_tests MuClient -j2
ctest --test-dir build_validation -R '\[ui\]\[scaling\]' --output-on-failure
```

Expected: compilation succeeds; scaling tests pass.

- [ ] **Step 7: Commit physical viewport integration**

```bash
git add src/source/Render/Textures/ZzzOpenglUtil.h src/source/Render/Textures/ZzzOpenglUtil.cpp src/source/Scenes/MainScene.cpp src/source/Render/Terrain/ZzzLodTerrain.cpp src/source/Camera/DefaultCamera.cpp src/source/Camera/OrbitalCamera.cpp
git commit -m "fix(render): align world viewport with HUD"
```

### Task 4: UV-Sliced Regional HUD Rendering

**Files:**
- Modify: `src/source/UI/NewUI/HUD/NewUIMainFrameWindow.h`
- Modify: `src/source/UI/NewUI/HUD/NewUIMainFrameWindow.cpp`

**Interfaces:**
- Consumes: regional transforms and `ScopedActiveTransform`; existing `RenderImageStretch` source-pixel API; existing item, skill, gauge, button, and experience renderers.
- Produces: `RenderLeftRegion`, `RenderCenterRegion`, `RenderRightRegion`, `RenderExperienceRegion`, four frame/background helpers, and region-correct deferred/3D rendering.

- [ ] **Step 1: Record the native RED baseline**

Use the existing native game capture path at one 5:4 and one wide resolution. Record that the current monolithic frame does not keep the combat gauges centered while preserving left/right utility ownership. Do not add implementation-text tests.

- [ ] **Step 2: Build the unchanged baseline**

Run `cmake --build build_validation --target MuClient -j2` before editing and record the result.

- [ ] **Step 3: Split the renderer into four focused region methods**

Replace `RenderFrame` in the header with:

```cpp
void RenderLeftRegion();
void RenderCenterRegion();
void RenderRightRegion();
void RenderExperienceRegion();
void RenderLeftFrame();
void RenderCenterFrame();
void RenderRightFrame();
void RenderExperienceBackground();
```

Add named source/destination constants beside the HUD renderer:

```cpp
constexpr float kHudTop = 429.0f;
constexpr float kHudContentHeight = 41.0f;
constexpr float kExperienceTop = 470.0f;
constexpr float kExperienceHeight = 10.0f;
constexpr float kLeftBandWidth = 152.0f;
constexpr float kCenterBandStart = 152.0f;
constexpr float kMenu1CenterWidth = 104.0f;
constexpr float kMenu2Start = 256.0f;
constexpr float kMenu2Width = 128.0f;
constexpr float kMenu3Start = 384.0f;
constexpr float kMenu3CenterWidth = 104.0f;
constexpr float kRightBandStart = 488.0f;
constexpr float kRightBandWidth = 152.0f;
constexpr float kMenu3RightSourceX = 104.0f;
```

Implement the exact fixed-content slices:

```cpp
void SEASON3B::CNewUIMainFrameWindow::RenderLeftFrame()
{
    RenderImageStretch(IMAGE_MENU_1, 0.0f, kHudTop, kLeftBandWidth, kHudContentHeight,
                       0.0f, 0.0f, kLeftBandWidth, kHudContentHeight);
}

void SEASON3B::CNewUIMainFrameWindow::RenderCenterFrame()
{
    RenderImageStretch(IMAGE_MENU_1, kCenterBandStart, kHudTop, kMenu1CenterWidth, kHudContentHeight,
                       kCenterBandStart, 0.0f, kMenu1CenterWidth, kHudContentHeight);
    RenderImageStretch(IMAGE_MENU_2, kMenu2Start, kHudTop, kMenu2Width, kHudContentHeight,
                       0.0f, 0.0f, kMenu2Width, kHudContentHeight);
    RenderImageStretch(IMAGE_MENU_3, kMenu3Start, kHudTop, kMenu3CenterWidth, kHudContentHeight,
                       0.0f, 0.0f, kMenu3CenterWidth, kHudContentHeight);

    if (g_pSkillList->IsSkillListUp())
        RenderImage(IMAGE_MENU_2_1, 222.0f, kHudTop, 160.0f, 40.0f);
}

void SEASON3B::CNewUIMainFrameWindow::RenderRightFrame()
{
    RenderImageStretch(IMAGE_MENU_3, kRightBandStart, kHudTop, kRightBandWidth, kHudContentHeight,
                       kMenu3RightSourceX, 0.0f, kRightBandWidth, kHudContentHeight);
}

void SEASON3B::CNewUIMainFrameWindow::RenderExperienceBackground()
{
    RenderImageStretch(IMAGE_MENU_1, 0.0f, kExperienceTop, 256.0f, kExperienceHeight,
                       0.0f, kHudContentHeight, 256.0f, kExperienceHeight);
    RenderImageStretch(IMAGE_MENU_2, 256.0f, kExperienceTop, 128.0f, kExperienceHeight,
                       0.0f, kHudContentHeight, 128.0f, kExperienceHeight);
    RenderImageStretch(IMAGE_MENU_3, 384.0f, kExperienceTop, 256.0f, kExperienceHeight,
                       0.0f, kHudContentHeight, 256.0f, kExperienceHeight);
}
```

These eight slices reconstruct the original `256 + 128 + 256` frame at `640x480`; only inter-region gaps change on wider windows.

- [ ] **Step 4: Render each concern under its owner transform**

Implement:

```cpp
void SEASON3B::CNewUIMainFrameWindow::RenderLeftRegion()
{
    const auto transform = UI::Scaling::BottomHudLeftTransform(WindowWidth, WindowHeight);
    UI::Scaling::ScopedActiveTransform layout(transform, true);
    RenderLeftFrame();
    m_pNewUI3DRenderMng->RenderUI2DEffect(ITEMHOTKEYNUMBER_CAMERA_Z_ORDER, UI2DEffectCallback, this, 0, 0);
}

void SEASON3B::CNewUIMainFrameWindow::RenderCenterRegion()
{
    const auto transform = UI::Scaling::BottomHudCenterTransform(WindowWidth, WindowHeight);
    UI::Scaling::ScopedActiveTransform layout(transform, true);
    RenderCenterFrame();
    g_pSkillList->RenderCurrentSkillAndHotSkillList();
    RenderLifeMana();
    RenderGuageSD();
    RenderGuageAG();
}

void SEASON3B::CNewUIMainFrameWindow::RenderRightRegion()
{
    const auto transform = UI::Scaling::BottomHudRightTransform(WindowWidth, WindowHeight);
    UI::Scaling::ScopedActiveTransform layout(transform, true);
    RenderRightFrame();
    RenderButtons();
}

void SEASON3B::CNewUIMainFrameWindow::RenderExperienceRegion()
{
    const auto transform = UI::Scaling::BottomHudExperienceTransform(WindowWidth, WindowHeight);
    UI::Scaling::ScopedActiveTransform layout(transform, true);
    RenderExperienceBackground();
    RenderExperience();
}
```

Reduce `Render()` to the existing blend-state setup plus these four calls. Keep `RenderLifeMana`, `RenderGuageSD`, `RenderGuageAG`, `RenderButtons`, and `RenderExperience` behavior unchanged.

Scope item previews explicitly because `CNewUIMainFrameWindow` itself remains a mixed-layout object:

```cpp
void SEASON3B::CNewUIMainFrameWindow::Render3D()
{
    const auto transform = UI::Scaling::BottomHudLeftTransform(WindowWidth, WindowHeight);
    UI::Scaling::ScopedActiveTransform layout(transform);
    m_ItemHotKey.RenderItems();
}
```

- [ ] **Step 5: Run unit and compile checks**

Run:

```bash
cmake --build build_validation --target ui_scaling_tests MuClient -j2
ctest --test-dir build_validation -R '\[ui\]\[scaling\]' --output-on-failure
```

Expected: scaling tests pass; `MuClient` compiles.

- [ ] **Step 6: Commit regional rendering**

```bash
git add src/source/UI/NewUI/HUD/NewUIMainFrameWindow.h src/source/UI/NewUI/HUD/NewUIMainFrameWindow.cpp
git commit -m "feat(ui): center responsive combat HUD"
```

### Task 5: Regional Mouse Input and World Gap Fallthrough

**Files:**
- Modify: `src/source/UI/NewUI/HUD/NewUIMainFrameWindow.cpp`
- Modify: `src/source/Scenes/MainScene.cpp`
- Modify: `src/source/Input/Selection.cpp`
- Modify: `src/source/Engine/Object/ZzzInterface.cpp`
- Modify: `src/source/UI/NewUI/HUD/NewUIHotKey.cpp`
- Modify: `src/source/UI/NewUI/Inventory/NewUIMyInventory.cpp`
- Test: `tests/ui/test_ui_scaling.cpp`

**Interfaces:**
- Consumes: `ScopedActiveTransform(..., true)` and `BottomHudContainsWindowPoint`.
- Produces: left Q/W/E/R hitboxes, center skills/tooltips, right buttons/tooltips, stretched EXP tooltip, visible-region world blocking, world clicks through wide gaps.

- [ ] **Step 1: Add failing edge tests for visible regions and gaps**

Extend `tests/ui/test_ui_scaling.cpp` with boundary checks at `1280x720`: left ends at physical `x=228`, center starts at `x=388`, center ends at `x=892`, right starts at `x=1052`, and the experience rail blocks the full width. Assert band-edge inclusivity/exclusivity and gap fallthrough.

- [ ] **Step 2: Run the focused test and verify RED**

Run:

```bash
cmake --build build_validation --target ui_scaling_tests -j2
ctest --test-dir build_validation -R '\[ui\]\[scaling\]' --output-on-failure
```

Expected: at least one new boundary expectation fails before the predicate/input routing is completed.

- [ ] **Step 3: Route main-frame mouse paths through their regional inverses**

Wrap right-button processing:

```cpp
bool SEASON3B::CNewUIMainFrameWindow::UpdateMouseEvent()
{
    if (g_pNewUIHotKey->IsStateGameOver())
        return true;

    const auto transform = UI::Scaling::BottomHudRightTransform(WindowWidth, WindowHeight);
    UI::Scaling::ScopedActiveTransform layout(transform, true);
    return !BtnProcess();
}
```

Wrap Q/W/E/R right-click handling:

```cpp
void SEASON3B::CNewUIMainFrameWindow::UseHotKeyItemRButton()
{
    const auto transform = UI::Scaling::BottomHudLeftTransform(WindowWidth, WindowHeight);
    UI::Scaling::ScopedActiveTransform layout(transform, true);
    m_ItemHotKey.UseItemRButton();
}
```

Do not add another transform inside `CNewUISkillList`: its manager-owned layout is `HudCenter`, so render, update, expanded list, tooltip coordinates, and mouse already share the center transform.

- [ ] **Step 4: Replace the blanket bottom strip with actual HUD-region blocking**

In `MainScene.cpp`, replace the fixed `MouseY >= REFERENCE_HEIGHT - 48` condition:

```cpp
if (UI::Scaling::BottomHudContainsWindowPoint(WindowWidth, WindowHeight,
                                              g_fWindowMouseX, g_fWindowMouseY))
    MouseOnWindow = true;
```

In `Input/Selection.cpp`, replace the final `CheckMouseIn(0, 0, GetScreenWidth(), 429)` term:

```cpp
const bool mouseOnHud = UI::Scaling::BottomHudContainsWindowPoint(
    WindowWidth, WindowHeight, g_fWindowMouseX, g_fWindowMouseY);
if (!MouseOnWindow && !mouseOnHud && !g_pNewUISystem->CheckMouseUse())
```

In `ZzzInterface.cpp`, replace the legacy attack boundary:

```cpp
const bool mouseOnHud = UI::Scaling::BottomHudContainsWindowPoint(
    WindowWidth, WindowHeight, g_fWindowMouseX, g_fWindowMouseY);
if ((MouseOnWindow || mouseOnHud) && MouseLButtonPush)
```

In `NewUIHotKey.cpp`, replace `CheckMouseIn(0, 0, GetScreenWidth(), 429)` in `AutoGetItem`:

```cpp
&& !UI::Scaling::BottomHudContainsWindowPoint(WindowWidth, WindowHeight,
                                               g_fWindowMouseX, g_fWindowMouseY)
```

Apply the same predicate replacement to the picked-item click condition in `NewUIMyInventory.cpp`. Add `#include "UI/Scaling/UITransform.h"` only where the file does not already receive the declaration through a direct include.

- [ ] **Step 5: Run focused tests and compile all touched input consumers**

Run:

```bash
cmake --build build_validation --target ui_scaling_tests MuClient -j2
ctest --test-dir build_validation -R '\[ui\]\[scaling\]' --output-on-failure
```

Expected: scaling tests pass; every touched input translation unit compiles.

- [ ] **Step 6: Commit regional input**

```bash
git add src/source/UI/NewUI/HUD/NewUIMainFrameWindow.cpp src/source/Scenes/MainScene.cpp src/source/Input/Selection.cpp src/source/Engine/Object/ZzzInterface.cpp src/source/UI/NewUI/HUD/NewUIHotKey.cpp src/source/UI/NewUI/Inventory/NewUIMyInventory.cpp tests/ui/test_ui_scaling.cpp
git commit -m "fix(ui): align HUD input regions"
```

### Task 6: Full Verification and Native Visual QA

**Files:**
- Verify: all files from Tasks 1-5
- Verify: `docs/superpowers/specs/2026-08-28-responsive-bottom-hud-design.md`

**Interfaces:**
- Consumes: completed responsive HUD implementation.
- Produces: test/link evidence plus native screenshots at one 5:4 and one wide resolution; no visual-completion claim without those screenshots.

- [ ] **Step 1: Run whitespace and placeholder checks**

Run:

```bash
git diff --check
python3 - <<'PY'
from pathlib import Path

text = Path("docs/superpowers/plans/2026-08-28-responsive-bottom-hud.md").read_text()
blocked = ("T" + "BD", "T" + "ODO", "implement " + "later", "fill in " + "details", "similar to " + "Task")
matches = [token for token in blocked if token.lower() in text.lower()]
if matches:
    raise SystemExit(f"plan contains forbidden placeholders: {matches}")
PY
```

Expected: both commands exit `0`.

- [ ] **Step 2: Build and run focused tests**

Run:

```bash
cmake --build build_validation --target ui_scaling_tests MuClient -j2
ctest --test-dir build_validation -R '\[ui\]\[scaling\]' --output-on-failure
```

Expected: focused scaling tests pass.

- [ ] **Step 3: Run the complete CTest suite**

Run:

```bash
ctest --test-dir build_validation --output-on-failure
```

Expected: all registered tests pass. Record the exact passed/failed count; do not reuse the pre-change `151/151` result.

- [ ] **Step 4: Verify the native executable link**

Run:

```bash
cmake --build build_validation --target Main -j2
```

Expected: `Main.app/Contents/MacOS/Main` links. If CMake injects the known host-only `ld: library 'brotlienc' not found` failure, record it as pre-existing environment state, then verify the generated native link line directly:

```bash
cd build_validation/src
/bin/sh CMakeFiles/Main.dir/link.txt
```

Expected: direct link exits `0`. Do not describe the standard target as passing when only the generated direct link passes.

- [ ] **Step 5: Capture a native 1280x1024 screenshot**

Edit `build_validation/src/Main.app/Contents/MacOS/config.ini` to:

```ini
[Window]
Width=1280
Height=1024
Windowed=1
```

Launch from its runtime directory:

```bash
cd build_validation/src/Main.app/Contents/MacOS
./Main
```

Enter the world, open character plus inventory using the right HUD buttons, hover HP/SD/current skill/AG/mana/EXP, right-click one Q/W/E/R item, then press Print Screen. Verify:

- left/center/right regions meet without a seam at 5:4;
- gauges and skills occupy the screen center;
- character/inventory panels dock to the HUD top;
- every hovered tooltip follows the visible control;
- right buttons and Q/W/E/R hitboxes match their images;
- terrain reaches the viewport edge without black corner squares.

- [ ] **Step 6: Capture a native 1920x1080 screenshot**

Set:

```ini
[Window]
Width=1920
Height=1080
Windowed=1
```

Repeat the launch and interaction sequence. Verify:

- HUD scale is `2.0`, not `3.0` horizontally or `2.25` vertically;
- combat center midpoint is physical `x=960`;
- side regions touch their screen edges;
- both horizontal gaps are equal;
- EXP background/progress spans the full width without stretching the fixed controls;
- clicking each gap still selects/moves in the world while clicking a visible HUD region does not;
- no texture seam, wrong tooltip, displaced hitbox, dock gap, or black terrain corner appears.

- [ ] **Step 7: Review the final diff against the spec**

Run:

```bash
git diff -- src/source/UI/Scaling/UITransform.h src/source/UI/Scaling/UITransform.cpp src/source/UI/NewUI/UILayoutPolicy.cpp src/source/UI/NewUI/NewUIManager.cpp src/source/UI/NewUI/NewUI3DRenderMng.cpp src/source/UI/NewUI/HUD/NewUIMainFrameWindow.h src/source/UI/NewUI/HUD/NewUIMainFrameWindow.cpp src/source/Render/Textures/ZzzOpenglUtil.h src/source/Render/Textures/ZzzOpenglUtil.cpp src/source/Scenes/MainScene.cpp src/source/Render/Terrain/ZzzLodTerrain.cpp src/source/Camera/DefaultCamera.cpp src/source/Camera/OrbitalCamera.cpp src/source/Input/Selection.cpp src/source/Engine/Object/ZzzInterface.cpp src/source/UI/NewUI/HUD/NewUIHotKey.cpp src/source/UI/NewUI/Inventory/NewUIMyInventory.cpp tests/ui/test_ui_scaling.cpp
```

Confirm every spec requirement maps to implemented code/tests; confirm no new texture, dependency, configurable scale, dialog change, or unrelated HUD redesign entered the diff.

- [ ] **Step 8: Commit verification-only corrections, if the checks required code changes**

If Steps 1-7 required corrections, stage only those corrections and commit:

```bash
git add src/source/UI/Scaling/UITransform.h src/source/UI/Scaling/UITransform.cpp src/source/UI/NewUI/UILayoutPolicy.cpp src/source/UI/NewUI/NewUIManager.cpp src/source/UI/NewUI/NewUI3DRenderMng.cpp src/source/UI/NewUI/HUD/NewUIMainFrameWindow.h src/source/UI/NewUI/HUD/NewUIMainFrameWindow.cpp src/source/Render/Textures/ZzzOpenglUtil.h src/source/Render/Textures/ZzzOpenglUtil.cpp src/source/Scenes/MainScene.cpp src/source/Render/Terrain/ZzzLodTerrain.cpp src/source/Camera/DefaultCamera.cpp src/source/Camera/OrbitalCamera.cpp src/source/Input/Selection.cpp src/source/Engine/Object/ZzzInterface.cpp src/source/UI/NewUI/HUD/NewUIHotKey.cpp src/source/UI/NewUI/Inventory/NewUIMyInventory.cpp tests/ui/test_ui_scaling.cpp
git commit -m "fix(ui): close responsive HUD regressions"
```

If no corrections were needed, create no empty commit.
