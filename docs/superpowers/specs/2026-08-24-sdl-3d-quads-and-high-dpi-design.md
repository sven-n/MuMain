# SDL 3D Quads and High-DPI Rendering

## Problem

The SDL renderer currently has no world-space quad API. During the SDL
migration, perimeter-ordered `GL_QUADS` vertices were routed through either
`RenderTriangles()` after local expansion or `RenderQuadStrip()`.

`RenderQuadStrip()` connects vertex pairs with indices `(0,1,2), (1,3,2)`.
The character aura supplies independent perimeter quads in the order
`current0, current1, next1, next0`, which require `(0,1,2), (0,2,3)`. The
strip diagonal creates overlapping and missing regions. Additive blending makes
those regions appear stacked, bright, and jagged.

The SDL window paths also omit high-pixel-density support. The renderer already
uses physical swapchain dimensions for viewport and scissor state while keeping
logical window dimensions for projections, so enabling SDL high-DPI support
does not require another coordinate transform.

## Design

### World-space quad API

Add `IMuRenderer::RenderQuad3D()` for independent world-space quads.

- Input vertices use perimeter order.
- Vertex count must be divisible by four.
- Each quad uses indices `(0,1,2), (0,2,3)`.
- The SDL backend reuses the existing static quad index buffer.
- Adjacent same-state world-space quad calls merge up to the static index
  buffer's `k_MaxQuads` capacity, preserving existing hot-loop batching.
- Rendering uses the existing 3D pipeline selection, current MVP, fog,
  texture resolution, blending, depth, and culling state.
- Invalid vertex counts are rejected before upload.

Rename the internal deferred command `DrawIndexedQuads2D` to
`DrawIndexedQuads`. The recorded command already carries its selected pipeline,
vertex layout data, MVP, texture, sampler, and fog state; replay therefore does
not need separate 2D and 3D command types.

`RenderQuadStrip()` remains available only for connected pair-based strips.
Its `GLCompatShim` `kGLQuadStrip` caller remains unchanged.

### Call-site migration

Migrate all verified independent world-space quad producers to
`RenderQuad3D()`, not only the five visible aura calls. This removes duplicate
six-vertex triangle expansion and prevents the same topology mistake in sibling
effects.

The migration covers 26 calls:

- Five character-aura faces in `ZzzEffectJoint.cpp`.
- `GLCompatShim.cpp` `SubmitQuadsAsTriangles`.
- `ZzzEffectMagicSkill.cpp`.
- Two calls in `ZzzEffectBlurSpark.cpp`.
- `SideHair.cpp`.
- The quad batch in `CSWaterTerrain.cpp`.
- Eight calls in `ZzzLodTerrain.cpp`.
- Four calls in `ZzzOpenglUtil.cpp`.
- `PhysicsManager.cpp` cloth-grid batching.
- `CameraMove.cpp` waypoint batching.
- The SDL renderer's thick-line quad.

Real triangle geometry remains on `RenderTriangles()`. `CSWaterTerrain.cpp`
triangle-list calls, model meshes, shadow volumes, and frustum geometry are not
changed.

### Aura lifecycle

Aura creation accepts Attack, Defense, and HelpNpc buffs. Movement currently
keeps aura joint subtypes 4 and 9 alive only for Defense and HelpNpc. Add Attack
to the shared continuation condition so attack auras are not repeatedly expired
and recreated while the buff remains active.

Aura creation count, tails, faces, blend mode, texture, and duplicate search
remain unchanged.

### High-DPI windows

Create one shared SDL window-flag helper. It always adds
`SDL_WINDOW_HIGH_PIXEL_DENSITY`, preserves requested fullscreen state, and
preserves the platform-window path's resizable state. Both SDL window creation
paths use it.

For macOS bundles, add an `Info.plist` template with
`NSHighResolutionCapable` set to `true`, then select it through
`MACOSX_BUNDLE_INFO_PLIST`. Existing bundle identifiers, versions, icon, and
executable metadata remain available through CMake substitutions.

## Error Handling

- `RenderQuad3D()` ignores empty draws through the existing render-state guard.
- Non-multiple-of-four vertex counts log one warning and skip the draw.
- Unknown textures, unavailable pipelines, and upload failures follow existing
  renderer behavior.
- Window creation failure handling remains unchanged.

## Testing

Use red-green-refactor for each behavior:

1. Renderer contract and topology: verify `RenderQuad3D()` exists, is classified
   as 3D, rejects malformed counts, uses the static quad index pattern, and
   merges adjacent draws without exceeding static index capacity.
2. Aura lifecycle: verify Attack, Defense, and HelpNpc each keep the joint alive;
   no relevant buff expires it.
3. SDL flags: verify high-pixel-density is always present and fullscreen and
   resizable inputs are retained.
4. macOS bundle: configure and build, then inspect the generated `Info.plist`
   for `NSHighResolutionCapable = true`.
5. Run focused tests, build `MuClient`, run the relevant CTest suite, inspect the
   diff, then perform a manual aura rendering check when the game is available.

## Excluded

MSAA is deferred. No multisample textures, pipelines, resolve targets, settings,
or UI controls are added in this change.
