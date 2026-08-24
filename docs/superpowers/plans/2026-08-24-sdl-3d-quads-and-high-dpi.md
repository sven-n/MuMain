# SDL 3D Quads and High-DPI Rendering Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Render independent world-space quads with correct topology, stop attack auras from expiring while active, and enable native high-DPI SDL output.

**Architecture:** Add one renderer API that reuses the existing static quad index buffer and 3D render state. Migrate only verified independent quads, retain the real quad-strip path, extract two pure helpers for aura lifetime and SDL flags, then configure the macOS bundle for Retina rendering.

**Tech Stack:** C++20, SDL3 GPU, CMake, doctest, CTest, macOS `plutil`.

---

### Task 1: Establish the pre-change baseline

**Files:**
- Verify: existing configured build at `out/build/macos-arm64`

- [ ] **Step 1: Run the existing suite**

Run:

```bash
ctest --test-dir out/build/macos-arm64 -C Release --output-on-failure
```

Expected: 79 tests pass. Record any failure before editing production code.

### Task 2: Add the renderer API contract

**Files:**
- Modify: `tests/render/test_frame_pixel_readback.cpp`
- Modify: `src/source/Render/Renderer/MuRenderer.h`
- Modify: `src/source/Render/Renderer/MuRenderer.cpp`
- Modify: `src/source/Render/Renderer/MuRendererSDLGpu.cpp`

- [ ] **Step 1: Add the failing interface implementation**

Add to `MinimalRenderer`:

```cpp
void RenderQuad3D(std::span<const mu::Vertex3D>, std::uint32_t) override {}
```

- [ ] **Step 2: Verify RED**

Run:

```bash
cmake --build --preset macos-arm64-release --target frame_pixel_readback_tests
```

Expected: compilation fails because `RenderQuad3D()` does not override an
`IMuRenderer` member.

- [ ] **Step 3: Add the minimal interface and backend overrides**

Add after `RenderTriangles()` in `IMuRenderer`:

```cpp
// Render independent world-space quads in perimeter order.
virtual void RenderQuad3D(std::span<const Vertex3D> vertices, std::uint32_t textureId) = 0;
```

Add no-op overrides to `NoopRenderer` and the SDL renderer. The SDL override is
temporarily empty; Task 3 supplies tested topology and full behavior.

- [ ] **Step 4: Verify GREEN**

Run:

```bash
cmake --build --preset macos-arm64-release --target frame_pixel_readback_tests
ctest --test-dir out/build/macos-arm64 -C Release -R 'renderer rejects unsupported frame readback' --output-on-failure
```

Expected: target builds and the selected test passes.

### Task 3: Implement shared independent-quad topology

**Files:**
- Create: `src/source/Render/Renderer/QuadTopology.h`
- Create: `tests/render/test_quad_topology.cpp`
- Modify: `tests/render/CMakeLists.txt`
- Modify: `src/source/Render/Renderer/MuRendererSDLGpu.cpp`

- [ ] **Step 1: Add failing topology tests**

Create a `quad_topology_tests` target and these cases:

```cmake
mu_add_test(
    NAME quad_topology_tests
    SOURCES test_quad_topology.cpp
)
```

```cpp
#include <array>
#include <cstdint>

#include <doctest.h>

#include "Render/Renderer/QuadTopology.h"

TEST_CASE("independent quads require four vertices [render][quad]")
{
    CHECK(Render::Topology::IsValidQuadVertexCount(0));
    CHECK(Render::Topology::IsValidQuadVertexCount(4));
    CHECK(Render::Topology::IsValidQuadVertexCount(8));
    CHECK_FALSE(Render::Topology::IsValidQuadVertexCount(3));
    CHECK_FALSE(Render::Topology::IsValidQuadVertexCount(6));
}

TEST_CASE("independent quads use perimeter triangle indices [render][quad]")
{
    std::array<std::uint16_t, 12> indices{};

    Render::Topology::FillQuadIndices(indices);

    CHECK(indices == std::array<std::uint16_t, 12>{0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7});
}
```

- [ ] **Step 2: Verify RED**

Run:

```bash
cmake --preset macos-arm64
cmake --build --preset macos-arm64-release --target quad_topology_tests
```

Expected: compilation fails because `QuadTopology.h` does not exist.

- [ ] **Step 3: Add the minimal topology helper**

Create `QuadTopology.h`:

```cpp
#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace Render::Topology
{

[[nodiscard]] constexpr bool IsValidQuadVertexCount(std::size_t vertexCount)
{
    return vertexCount % 4 == 0;
}

inline void FillQuadIndices(std::span<std::uint16_t> indices)
{
    for (std::size_t quad = 0; quad < indices.size() / 6; ++quad)
    {
        const auto base = static_cast<std::uint16_t>(quad * 4);
        const std::size_t output = quad * 6;
        indices[output + 0] = base + 0;
        indices[output + 1] = base + 1;
        indices[output + 2] = base + 2;
        indices[output + 3] = base + 0;
        indices[output + 4] = base + 2;
        indices[output + 5] = base + 3;
    }
}

} // namespace Render::Topology
```

- [ ] **Step 4: Use the helper in the static index upload**

In `CreateQuadIndexBuffer()`, replace the push loop with a fixed-size vector and
the tested helper:

```cpp
std::vector<Uint16> indices(static_cast<std::size_t>(k_MaxQuads) * 6);
Render::Topology::FillQuadIndices(indices);
```

- [ ] **Step 5: Implement `RenderQuad3D()`**

Mirror `RenderTriangles()` state resolution, but reject malformed quad counts,
use the 3D pipeline, record `DrawIndexedQuads`, set the current MVP and fog, and
draw `min(vertices.size() / 4, k_MaxQuads) * 6` indices from `s_quadIdxBuf`.
Merge adjacent same-state, contiguous world-space quad commands while their
combined index count remains within `k_MaxQuads`; start a new command at the
capacity boundary.

Rename `RenderCmdType::DrawIndexedQuads2D` to `DrawIndexedQuads`; both
`RenderQuad2D()` and `RenderQuad3D()` record it. Add `DrawMode::Quad3D` to the
3D pipeline classification.

- [ ] **Step 6: Verify GREEN**

Run:

```bash
cmake --build --preset macos-arm64-release --target quad_topology_tests frame_pixel_readback_tests MuClient
ctest --test-dir out/build/macos-arm64 -C Release -R 'independent quads|renderer rejects unsupported frame readback' --output-on-failure
```

Expected: selected tests pass and `MuClient` builds.

### Task 4: Migrate every verified independent 3D quad

**Files:**
- Modify: `src/source/Render/Effects/ZzzEffectJoint.cpp`
- Modify: `src/source/Render/Renderer/GLCompatShim.cpp`
- Modify: `src/source/Render/Effects/ZzzEffectMagicSkill.cpp`
- Modify: `src/source/Render/Effects/ZzzEffectBlurSpark.cpp`
- Modify: `src/source/Render/Models/SideHair.cpp`
- Modify: `src/source/Render/Terrain/CSWaterTerrain.cpp`
- Modify: `src/source/Render/Terrain/ZzzLodTerrain.cpp`
- Modify: `src/source/Render/Textures/ZzzOpenglUtil.cpp`
- Modify: `src/source/Engine/Physics/PhysicsManager.cpp`
- Modify: `src/source/Camera/CameraMove.cpp`
- Modify: `src/source/Render/Renderer/MuRendererSDLGpu.cpp`

- [ ] **Step 1: Migrate the five malformed aura faces**

Replace the five `RenderQuadStrip()` calls at the aura/force face arrays with
`RenderQuad3D()`. Keep each existing four-vertex perimeter array unchanged.

- [ ] **Step 2: Remove local triangle expansion from single quads**

For `ZzzEffectMagicSkill.cpp`, both `ZzzEffectBlurSpark.cpp` calls,
`SideHair.cpp`, `CSWaterTerrain.cpp`, eight `ZzzLodTerrain.cpp` calls,
`RenderPlane3D()`, both sprite paths, and the renderer thick-line path, keep the
four perimeter vertices and call:

```cpp
mu::GetRenderer().RenderQuad3D(vertices, textureId);
```

Delete the duplicate `v0, v2` triangle vertices.

- [ ] **Step 3: Convert batched quad producers from six to four vertices**

Apply these exact size changes:

```cpp
// GLCompatShim.cpp
s_renderVertices.resize((s_vertices.size() / 4) * 4);
// Copy each source quad's four vertices once, then RenderQuad3D().

// PhysicsManager.cpp
quadVerts.reserve(numQuads * 4);
// Push v0, v1, v2, v3 for each front/back face, then RenderQuad3D().

// CameraMove.cpp
vertices.reserve(m_listWayPoint.size() * 4);
vertices.insert(vertices.end(), {v0, v1, v2, v3});
// RenderQuad3D() before reusing the vector for lines.

// ZzzOpenglUtil.cpp RenderBox
verts.reserve(24);
// Emit each face's a, b, c, d once, then RenderQuad3D().
```

- [ ] **Step 4: Preserve genuine topology**

Leave these calls unchanged:

```text
GLCompatShim.cpp kGLQuadStrip -> RenderQuadStrip
CSWaterTerrain.cpp lines near 87 and 100 -> RenderTriangles
ZzzBMD.cpp model meshes -> RenderTriangles
ShadowVolume.cpp -> RenderTriangles
FrustumRenderer.cpp -> RenderTriangles
```

- [ ] **Step 5: Audit remaining calls**

Run:

```bash
rg -n 'RenderQuadStrip\(' src/source
rg -n 'Vertex3D\s+\w+\s*\[6\]' src/source --glob '*.cpp'
rg -n 'RenderTriangles\(' src/source
```

Expected: only the `kGLQuadStrip` compatibility path calls
`RenderQuadStrip()`. Remaining six-vertex arrays and `RenderTriangles()` calls
represent real triangles or lines.

- [ ] **Step 6: Build after migration**

Run:

```bash
cmake --build --preset macos-arm64-release --target MuClient
```

Expected: `Built target MuClient`.

### Task 5: Fix attack-aura lifetime

**Files:**
- Create: `src/source/Render/Effects/AuraJointLifecycle.h`
- Create: `tests/render/test_aura_joint_lifecycle.cpp`
- Modify: `tests/render/CMakeLists.txt`
- Modify: `src/source/Render/Effects/ZzzEffectJoint.cpp`

- [ ] **Step 1: Add failing lifecycle tests**

Register the new test target:

```cmake
mu_add_test(
    NAME aura_joint_lifecycle_tests
    SOURCES test_aura_joint_lifecycle.cpp
)
```

```cpp
#include <doctest.h>

#include "Render/Effects/AuraJointLifecycle.h"

TEST_CASE("supported buffs keep character aura joints alive [render][aura]")
{
    CHECK(Render::Effects::ShouldKeepAuraJointAlive(true, false, false));
    CHECK(Render::Effects::ShouldKeepAuraJointAlive(false, true, false));
    CHECK(Render::Effects::ShouldKeepAuraJointAlive(false, false, true));
}

TEST_CASE("character aura joint expires without a supported buff [render][aura]")
{
    CHECK_FALSE(Render::Effects::ShouldKeepAuraJointAlive(false, false, false));
}
```

- [ ] **Step 2: Verify RED**

Run:

```bash
cmake --preset macos-arm64
cmake --build --preset macos-arm64-release --target aura_joint_lifecycle_tests
```

Expected: compilation fails because `AuraJointLifecycle.h` does not exist.

- [ ] **Step 3: Add the minimal predicate**

```cpp
#pragma once

namespace Render::Effects
{

[[nodiscard]] constexpr bool ShouldKeepAuraJointAlive(bool hasAttack, bool hasDefense, bool hasHelpNpc)
{
    return hasAttack || hasDefense || hasHelpNpc;
}

} // namespace Render::Effects
```

- [ ] **Step 4: Use the predicate in joint movement**

For subtypes 4 and 9, evaluate the three existing buff checks once, pass them to
`ShouldKeepAuraJointAlive()`, retain lifetime 100 when true, otherwise expire
the joint through the existing path.

- [ ] **Step 5: Verify GREEN**

Run:

```bash
cmake --build --preset macos-arm64-release --target aura_joint_lifecycle_tests MuClient
ctest --test-dir out/build/macos-arm64 -C Release -R 'character aura|supported buffs' --output-on-failure
```

Expected: all selected lifecycle tests pass and `MuClient` builds.

### Task 6: Enable SDL high-pixel-density windows

**Files:**
- Create: `src/source/Core/Platform/sdl3/SDLWindowFlags.h`
- Create: `tests/platform/test_sdl_window_flags.cpp`
- Modify: `tests/platform/CMakeLists.txt`
- Modify: `src/source/Core/Platform/sdl3/SDLWindow.cpp`
- Modify: `src/source/App/Platform/Windows/Winmain.cpp`

- [ ] **Step 1: Add failing window-flag tests**

Register the target in `tests/platform/CMakeLists.txt`:

```cmake
mu_add_test(
    NAME sdl_window_flags_tests
    SOURCES test_sdl_window_flags.cpp
    LINK_LIBS SDL3::SDL3
)
```

```cpp
#include <doctest.h>
#include <SDL3/SDL_video.h>

#include "Core/Platform/sdl3/SDLWindowFlags.h"

TEST_CASE("SDL windows always request high pixel density [platform][sdl]")
{
    const SDL_WindowFlags flags = Core::Platform::BuildSDLWindowFlags(false, false);
    CHECK((flags & SDL_WINDOW_HIGH_PIXEL_DENSITY) != 0);
}

TEST_CASE("SDL window flags preserve fullscreen and resizable requests [platform][sdl]")
{
    const SDL_WindowFlags flags = Core::Platform::BuildSDLWindowFlags(true, true);
    CHECK((flags & SDL_WINDOW_FULLSCREEN) != 0);
    CHECK((flags & SDL_WINDOW_RESIZABLE) != 0);
}
```

- [ ] **Step 2: Verify RED**

Run:

```bash
cmake --preset macos-arm64
cmake --build --preset macos-arm64-release --target sdl_window_flags_tests
```

Expected: compilation fails because `SDLWindowFlags.h` does not exist.

- [ ] **Step 3: Add the minimal shared helper**

```cpp
#pragma once

#include <SDL3/SDL_video.h>

namespace Core::Platform
{

[[nodiscard]] constexpr SDL_WindowFlags BuildSDLWindowFlags(bool fullscreen, bool resizable)
{
    SDL_WindowFlags flags = SDL_WINDOW_HIGH_PIXEL_DENSITY;
    if (fullscreen)
    {
        flags |= SDL_WINDOW_FULLSCREEN;
    }
    if (resizable)
    {
        flags |= SDL_WINDOW_RESIZABLE;
    }
    return flags;
}

} // namespace Core::Platform
```

- [ ] **Step 4: Use the helper in both creation paths**

`SDLWindow::Create()` passes `(flags & MU_WINDOW_FULLSCREEN) != 0` and `true`.
`Winmain.cpp` passes `g_bUseWindowMode != TRUE` and `false`. Keep existing window
creation error handling unchanged.

- [ ] **Step 5: Verify GREEN**

Run:

```bash
cmake --build --preset macos-arm64-release --target sdl_window_flags_tests MuClient Main
ctest --test-dir out/build/macos-arm64 -C Release -R 'SDL window' --output-on-failure
```

Expected: both window-flag tests pass and application targets build.

### Task 7: Mark the macOS bundle high-resolution capable

**Files:**
- Create: `src/source/App/Platform/macOS/Info.plist.in`
- Modify: `src/CMakeLists.txt`

- [ ] **Step 1: Verify RED against the current bundle**

Run:

```bash
plutil -extract NSHighResolutionCapable raw out/build/macos-arm64/src/Release/Main.app/Contents/Info.plist
```

Expected: failure because the key does not exist.

- [ ] **Step 2: Add the bundle template**

Copy CMake's standard `MacOSXBundleInfo.plist.in` fields, then add:

```xml
<key>NSHighResolutionCapable</key>
<true/>
```

- [ ] **Step 3: Select the template**

Add to the Apple target properties:

```cmake
MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/source/App/Platform/macOS/Info.plist.in"
```

- [ ] **Step 4: Verify GREEN**

Run:

```bash
cmake --preset macos-arm64
cmake --build --preset macos-arm64-release --target Main
plutil -extract NSHighResolutionCapable raw out/build/macos-arm64/src/Release/Main.app/Contents/Info.plist
```

Expected: `true`.

### Task 8: Final verification

**Files:**
- Verify: all modified files

- [ ] **Step 1: Check formatting and patch hygiene**

Run:

```bash
git diff --check
git status --short
```

Expected: no whitespace errors; only intended files changed.

- [ ] **Step 2: Build application and focused tests**

Run:

```bash
cmake --build --preset macos-arm64-release --target MuClient Main frame_pixel_readback_tests quad_topology_tests aura_joint_lifecycle_tests sdl_window_flags_tests
```

Expected: all targets build.

- [ ] **Step 3: Run focused regressions**

Run:

```bash
ctest --test-dir out/build/macos-arm64 -C Release -R 'quad|aura|SDL window|frame readback' --output-on-failure
```

Expected: all selected tests pass.

- [ ] **Step 4: Run the full suite**

Run:

```bash
ctest --test-dir out/build/macos-arm64 -C Release --output-on-failure
```

Expected: no failures; compare against the 79-test clean baseline plus newly
added cases.

- [ ] **Step 5: Re-audit renderer topology**

Run:

```bash
rg -n 'RenderQuadStrip\(' src/source
rg -n 'RenderQuad3D\(' src/source
git diff --stat
git diff -- src/source/Render/Renderer src/source/Render/Effects src/source/Core/Platform/sdl3 src/source/App/Platform src/source/Render/Terrain src/source/Render/Textures src/source/Render/Models src/source/Engine/Physics src/source/Camera tests
```

Expected: `RenderQuadStrip()` remains only for genuine strip geometry; all 26
verified independent quads use `RenderQuad3D()`.

- [ ] **Step 6: Manual visual check**

Launch the rebuilt client on a high-density display. Observe Attack, Defense,
and HelpNpc auras while moving and while buff timers remain active.

Expected: aura faces form continuous smooth ribbons without bright overlap
spikes or missing wedges; aura joints remain stable; UI and viewport retain
correct logical size with sharper native-resolution output.

MSAA remains deferred.
