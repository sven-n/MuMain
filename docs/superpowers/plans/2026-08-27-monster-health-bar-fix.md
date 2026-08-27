# Monster Health Bar Fix Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Keep the selected-monster health bar visible while HP is unknown, hide it only at zero HP, and preserve F8-controlled overhead bars.

**Architecture:** Extract the selected-bar eligibility rule into one tiny testable combat-UI helper. `CNewUINameWindow` uses that rule only for the selected target; the existing `DrawHealthBar()` unknown/full handling and F8 overhead toggle remain unchanged.

**Tech Stack:** C++20, CMake/CTest, doctest.

**Spec:** `docs/superpowers/specs/2026-08-27-text-label-health-bar-ui-scaling-design.md`

## Global Constraints

- Positive HP renders the selected bar.
- `HealthStatus == -1` renders the existing unknown/full presentation.
- `HealthStatus == 0` hides the selected bar.
- F8 overhead bars remain disabled by default and opt-in.
- Invent no client-side HP value.

---

### Task 1: Selected health-bar eligibility

**Files:**
- Create: `src/source/UI/Combat/MonsterHealthBar.h`
- Create: `src/source/UI/Combat/MonsterHealthBar.cpp`
- Create: `tests/ui/test_monster_health_bar.cpp`
- Modify: `tests/ui/CMakeLists.txt`
- Modify: `src/source/UI/NewUI/Character/NewUINameWindow.cpp:1-220`

**Interfaces:**
- Produces: `bool UI::Combat::HealthBar::ShouldRenderSelected(float healthStatus)`.
- Consumes: `CHARACTER::HealthStatus`, where `-1` means unknown and `0` means empty/dead.

- [ ] **Step 1: Write the failing eligibility test**

```cpp
#include <doctest.h>

#include "UI/Combat/MonsterHealthBar.h"

TEST_CASE("selected monster health eligibility [ui][health_bar]")
{
    CHECK(UI::Combat::HealthBar::ShouldRenderSelected(1.0f));
    CHECK(UI::Combat::HealthBar::ShouldRenderSelected(0.25f));
    CHECK(UI::Combat::HealthBar::ShouldRenderSelected(-1.0f));
    CHECK_FALSE(UI::Combat::HealthBar::ShouldRenderSelected(0.0f));
}
```

Register it:

```cmake
mu_add_test(
    NAME monster_health_bar_tests
    SOURCES
        test_monster_health_bar.cpp
        ${CMAKE_SOURCE_DIR}/src/source/UI/Combat/MonsterHealthBar.cpp
)
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `cmake --build build_validation --target monster_health_bar_tests -j2`

Expected: build FAIL because `UI/Combat/MonsterHealthBar.h` does not exist.

- [ ] **Step 3: Implement the minimal rule**

```cpp
// UI/Combat/MonsterHealthBar.h
#pragma once

namespace UI::Combat::HealthBar
{
    bool ShouldRenderSelected(float healthStatus);
}
```

```cpp
// UI/Combat/MonsterHealthBar.cpp
#include "UI/Combat/MonsterHealthBar.h"

bool UI::Combat::HealthBar::ShouldRenderSelected(float healthStatus)
{
    return healthStatus != 0.0f;
}
```

- [ ] **Step 4: Use the rule for the selected monster**

Include `UI/Combat/MonsterHealthBar.h`. Replace only:

```cpp
if (c->HealthStatus > 0)
```

with:

```cpp
if (UI::Combat::HealthBar::ShouldRenderSelected(c->HealthStatus))
```

Do not change `m_bShowMonsterHealthBar`, its constructor default, the F8 toggle, or the overhead-bar loop.

- [ ] **Step 5: Run focused verification**

Run: `cmake --build build_validation --target monster_health_bar_tests MuClient -j2 && ctest --test-dir build_validation -R 'monster_health_bar' --output-on-failure`

Expected: focused test PASS; production target builds.

- [ ] **Step 6: Commit**

```bash
git add tests/ui/CMakeLists.txt tests/ui/test_monster_health_bar.cpp \
  src/source/UI/Combat/MonsterHealthBar.h src/source/UI/Combat/MonsterHealthBar.cpp \
  src/source/UI/NewUI/Character/NewUINameWindow.cpp
git commit -m "fix(ui): show unknown monster health"
```
