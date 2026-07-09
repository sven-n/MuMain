# Story draft: SDL3 Port of Ground-Item-Label Texture Cache

> **Note:** This is raw content to feed into the `create-story` skill. Not a final story file.

## Suggested metadata

- **Story Key:** 7-9-13 (or next available in Feature 7.9)
- **Title:** SDL3 Port of Ground-Item-Label Texture Cache
- **Epic:** EPIC-7 (Stability, Diagnostics & Quality Gates)
- **Feature:** 7.9 — SDL3 Cross-Platform Runtime
- **Flow Type:** Enabler / Performance
- **Story Points:** 5
- **Dependencies:** 7-9-8 (SDL_ttf font rendering) ✓, 7-9-6 (raw GL → mu::Renderer migration) ✓, 7-9-10 (SDL_ttf text input rendering) ✓

## User story

**As a** player on macOS or Linux in a dense drop area (Lorencia spawns, Devil Square wave, Blood Castle chest),
**I want** ground-item labels to render without re-rasterizing every label every frame,
**So that** framerate does not drop when 50+ labeled drops are on screen.

## Background

### Problem

Upstream PR #321 introduced a texture cache for ground-item labels in `ZzzInventory.cpp` — `CreateGroundItemLabelTexture`, `RenderGroundItemLabelTexture`, `DeleteGroundItemLabelTexture`, and the cache plumbing in `RenderGroundItemLabelCached` / `PruneGroundItemLabelCache`. The cache produced a measurable FPS win on Windows by avoiding per-frame GDI rasterization + OpenGL texture upload for each visible label.

The merge of `main` into `cross-platform-sdl-migration` (merge commit `f5d1d73e`) brought this code onto the SDL3 branch, but the implementation uses APIs that don't exist on SDL3/macOS:

| API | Origin | Replacement on SDL3 |
|---|---|---|
| `HDC`, `FillRect`, `GetStockObject(BLACK_BRUSH)`, `GetTextExtentPoint32`, `TextOut`, `SetBkColor`, `SetTextColor` | Windows GDI | `SDL_ttf` rasterization to `SDL_Surface` (same path `CUIRenderTextSDLTtf` uses since 7-9-8) |
| `GLuint`, `glGenTextures`, `glBindTexture`, `glTexParameteri`, `glTexImage2D`, `glDeleteTextures` | Raw OpenGL | `mu::Renderer::RegisterTexture` / `QueueTextureUpdate` / `BindTexture` (SDL_GPU, per 7-9-6) |
| `glColor4ub`, `glColor4f` | OpenGL immediate-mode state | Per-draw vertex color on the UI pipeline |
| `_snwprintf_s` / `_TRUNCATE` | Microsoft CRT | `mu_swprintf_s` (already cross-platform, defined in `stdafx.h` and `PlatformCompat.h`) |

### Current state (landed in fixup commit `1eeac9aa`)

To unblock the SDL3 build, the three texture-cache functions were gated behind `#ifdef MU_ENABLE_SDL3`:

- `CreateGroundItemLabelTexture` returns `false` on SDL3 → `RenderGroundItemLabelCached` also returns `false`.
- `RenderGroundItemLabelTexture` is an empty stub on SDL3.
- `DeleteGroundItemLabelTexture` is a no-op on SDL3.
- The else-branch in `RenderItemName` now checks the return of `RenderGroundItemLabelCached` and falls back to the direct `g_pRenderText->RenderText()` path when the cache is unavailable.
- `_snwprintf_s` was replaced with `mu_swprintf_s` in both template helpers (that change is permanent and applies to both platforms).

On SDL3 today, labels render correctly but without caching — every visible ground-item label rasterizes its text every frame.

### What this story changes

Re-enable the texture-cache optimization on SDL3 using the same renderer and text-rasterization paths already adopted by the cross-platform migration. Remove the `#ifdef MU_ENABLE_SDL3` branches once the SDL3 implementation is in place.

### What this story does NOT change

- The cache key structure (`BuildGroundItemLabelCacheKey`), eviction policy (`PruneGroundItemLabelCache`), or budget (`SetGroundItemLabelBuildBudget`). These are platform-independent and fine as-is.
- The Windows OpenGL code path. It stays functionally identical — after this story the `#ifdef` branches are gone, but the Windows build uses the same SDL_gpu/SDL_ttf paths the rest of the migration adopted.
  - **Open question for planning:** does the SDL3 cross-platform path fully replace the Windows OpenGL path, or does Feature 7.9 still have a dual-path story where Windows keeps OpenGL? If the latter, this story preserves both paths behind `#ifdef`; if the former, this story deletes the Windows OpenGL path. Check against the 7-9-6 outcome.
- The `RenderItemName` behavior on the `!Sort` branch (it already uses the direct render path; unchanged).

## Acceptance criteria

### Functional

- [ ] **AC-1:** `CreateGroundItemLabelTexture` on SDL3 rasterizes `descriptor.Name` via SDL_ttf into an `SDL_Surface` (use the font set via `ApplyGroundItemLabelDescriptor` / `descriptor.Font` — same font-selection path as Windows).
- [ ] **AC-2:** `CreateGroundItemLabelTexture` on SDL3 creates/registers an `SDL_GPUTexture` via `mu::Renderer`, populates `cacheEntry.TextureId` with a renderer-specific handle, and returns `true` on success.
- [ ] **AC-3:** `RenderGroundItemLabelTexture` on SDL3 binds the cached texture via `mu::Renderer::BindTexture` and draws a textured quad at the same screen coordinates the Windows path uses (`o->ScreenX * g_fScreenRate_x - TextWidth/2`, `(o->ScreenY - 15) * g_fScreenRate_y`).
- [ ] **AC-4:** `RenderGroundItemLabelTexture` on SDL3 draws the background rect using `cacheEntry.BgColor` when non-zero, matching the Windows `glColor4ub` + `RenderColor` behavior. Use vertex color on the UI pipeline (no `glColor4ub`).
- [ ] **AC-5:** `DeleteGroundItemLabelTexture` on SDL3 releases the `SDL_GPUTexture` (or removes it from the renderer registry) to prevent GPU memory leaks.
- [ ] **AC-6:** All three `#ifdef MU_ENABLE_SDL3` blocks in `ZzzInventory.cpp` (lines ~7512, ~7946, ~8046) are removed; the Windows and SDL3 builds run the same `Create/Render/Delete` code paths.
- [ ] **AC-7:** The fallback branch in `RenderItemName` (the SDL3 direct-render fallback added in `1eeac9aa`) is removed. The else-branch becomes `RenderGroundItemLabelCached(o, ip);` (return ignored) as it was pre-fix, because the cache is always available now.

### Non-functional

- [ ] **AC-NFR-1:** In a dense-drop scene (repro: Lorencia near spawn after 60s of monster kills, or Devil Square wave 3), SDL3/macOS FPS with this story is measurably higher than the direct-render fallback path. Capture before/after frame times via `frame_time` instrumentation (Story 7-2-1 already landed this).
- [ ] **AC-NFR-2:** No GPU memory leak when walking through many drops over several minutes. Verify via `mu::Renderer` diagnostics (count of live textures returns to baseline after `PruneGroundItemLabelCache` fires on idle labels).
- [ ] **AC-NFR-3:** Visual parity with the direct-render fallback: item name, text color, background color, centering, and drop shadow (if any) are indistinguishable in A/B screenshots.

## Tasks

- [ ] **Task 1 — SDL_ttf rasterization for the cache.** Replace the GDI section of `CreateGroundItemLabelTexture` (roughly lines 7964–7977 today) with an SDL_ttf call returning an `SDL_Surface`. Measure `textSize` from `TTF_SizeText` / `TTF_SizeUNICODE` instead of `GetTextExtentPoint32`. Reference: `CUIRenderTextSDLTtf` introduced in 7-9-8.
- [ ] **Task 2 — SDL_GPU texture creation.** Replace the `glGenTextures`/`glTexImage2D` block (lines 8012–8024) with `mu::Renderer::RegisterTexture` (or whichever API 7-9-6 settled on for one-off UI textures). Store the renderer handle — not a raw `GLuint` — in `cacheEntry.TextureId` (may require widening the struct field).
- [ ] **Task 3 — Draw path.** Rewrite `RenderGroundItemLabelTexture` body (lines 8057–8065 for the BG rect, 8066–8069 for the texture) to use `mu::Renderer::BindTexture` + `RenderBitmap` (already used elsewhere in the SDL3 path; see `ThirdParty/UIControls.cpp` font-upload site) and per-draw vertex color for the background rect.
- [ ] **Task 4 — Delete path.** Implement `DeleteGroundItemLabelTexture` using the renderer's unregister/release API. Verify `PruneGroundItemLabelCache` still correctly evicts idle entries.
- [ ] **Task 5 — Remove the `#ifdef` gates.** Delete `#ifdef MU_ENABLE_SDL3` / `#else` / `#endif` from the three functions. Delete the else-branch fallback in `RenderItemName`.
- [ ] **Task 6 — Perf measurement.** Capture frame times in the repro scene on macOS: (a) current fallback path, (b) this story's cached path. Attach results to the story.

## Files to touch

- `src/source/Gameplay/Items/ZzzInventory.cpp` — the three functions, the struct `GroundItemLabelCacheEntry` if `TextureId` type needs to change, and `RenderItemName` fallback removal.
- Possibly `src/source/Renderer/*` — if a new "register text glyph texture" helper is appropriate (evaluate during Task 2; may not be necessary if the existing `QueueTextureUpdate` + `BindTexture` pair is sufficient).
- No header changes expected in `ZzzInventory.h`.

## References

- Temporarily disabled cache: commit `1eeac9aa` (this PR)
- Introduced via upstream merge: commit `f5d1d73e`
- Upstream origin PR: https://github.com/sven-n/MuMain/pull/321 ("tooltips_fps")
- SDL_ttf precedent in this codebase: Story 7-9-10, commit `dc085d27`
- SDL_gpu renderer migration: Story 7-9-6
- Frame-time instrumentation for perf measurement: Story 7-2-1

## Risks / callouts for SM/PM

- The `cacheEntry.TextureId` field is currently typed `GLuint` in the cache struct. If the SDL_gpu renderer uses a different handle type, that field widens to `std::uint32_t` or a renderer-specific typedef. Check `GroundItemLabelCacheEntry` definition around line 7400–7430 and adjust.
- If the Windows build is being retired from the repo as part of Feature 7.9, Task 5 can also delete the Windows-specific paths. If not, this story keeps the Windows code and just removes the preprocessor branch — both paths route through the cross-platform renderer.
- The `RenderBitmap` SDL3 path currently flips the Y-coordinate space in some call sites and not others. Double-check the ground-item label's screen-space math (it uses `o->ScreenX`/`o->ScreenY` which are already in SDL-ready coordinates) against a known-working SDL3 UI element that draws at world-projected positions.
