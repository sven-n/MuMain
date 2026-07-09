# PR #329 Review Follow-ups

Tracking for the technical issues flagged on
[sven-n/MuMain#329](https://github.com/sven-n/MuMain/pull/329) (closed 2026-04-26).
Two reviewers: `gemini-code-assist[bot]` (COMMENTED) and `Mosch0512` (CHANGES_REQUESTED, then closed).

The PR was closed for size reasons (~1,188 files, +223k / −68k LOC), not for the
technical issues — those remain valid feedback for the working branch
`cross-platform-sdl-migration-merged`.

## Status of Mosch's nine issues

### Closed by recent CI work

#### #6 — Clang-only `-Wno-error=` flags fail under GCC
- **Fixed** in `a68c796c` — gates each clang-only flag with
  `$<$<CXX_COMPILER_ID:Clang,AppleClang>:...>`.
- **Also extended** the relaxation list to `MURenderFX` in `e6ccc557`.
- Open follow-up: Mosch suggested adding GCC-flavored relaxations the legacy
  code typically needs. Add if a future GCC build trips them:
  ```
  $<$<CXX_COMPILER_ID:GNU>:-Wno-error=address>
  $<$<CXX_COMPILER_ID:GNU>:-Wno-error=parentheses>
  $<$<CXX_COMPILER_ID:GNU>:-Wno-error=class-memaccess>
  $<$<CXX_COMPILER_ID:GNU>:-Wno-error=stringop-truncation>
  $<$<CXX_COMPILER_ID:GNU>:-Wno-error=format>
  $<$<CXX_COMPILER_ID:GNU>:-Wno-error=format-security>
  $<$<CXX_COMPILER_ID:GNU>:-Wno-error=array-bounds>
  ```
- Open follow-up: relaxations only apply to `MUGame`/`Main`/`MURenderFX`. If
  legacy `Zzz*` patterns trip GCC inside `MUData` or `MUCore`, extend there
  too.

#### #8 — `find_package(CURL REQUIRED)` blocks configure
- **Sidestepped** in `2aefc9ef` — `vcpkg.json` provides curl on Windows MSVC.
- **Mosch's preferred fix is different**: graceful no-op fallback like
  OpenSSL already has (`mu_encrypt_blob` falls back to identity at
  `src/CMakeLists.txt:298`). For CURL, on `find_package` failure define a
  stub `CURL::libcurl` interface library and have `ShopListManager` log
  "downloads disabled". Lets contributors without vcpkg/system curl
  clone-and-configure immediately.
- **Worth doing** because it makes the project bootstrappable in any
  configuration, not just the ones CI exercises. Same shape as the
  existing OpenSSL fallback.

#### #9 — Preset's `toolchainFile` overrides downstream vcpkg overrides
- **Sidestepped** in `2aefc9ef` — CI uses `VCPKG_CHAINLOAD_TOOLCHAIN_FILE`
  on the CMake CLI to chain-load `toolchain-x64.cmake` through
  `vcpkg.cmake`.
- Mosch classified this exact pattern as *"option 3 — ugly but
  non-breaking."* He prefers:
  - **Option 1** — move `toolchainFile` to a leaf preset
    (`windows-x64-default-toolchain`) so other leaves
    (`windows-x64-vcpkg`, `windows-x64-mingw`) can override freely.
  - **Option 2** — delete the toolchain files entirely; they only set
    `CMAKE_GENERATOR_PLATFORM` which Ninja Multi-Config ignores.
- **Affects CLion / Visual Studio users locally**, not CI. Worth fixing
  before anyone else opens the project on Windows in those IDEs.

### Closed since doc creation (verified 2026-04-27)

#### #1 — Incomplete logging migration ✓ resolved
After `Story 7.10.1` deleted `CErrorReport` and `CmuConsoleDebug`, this
finding flagged surviving references in `Network/WSclient.cpp`,
`Scenes/{Main,Character}Scene.cpp`, `Scenes/SceneManager.cpp`,
`GameShop/NewUIInGameShop.cpp`. Re-grep on 2026-04-27 returns **zero
hits** for `g_ConsoleDebug | g_ErrorReport | MCD_NORMAL | MCD_RECEIVE |
MCD_ERROR | muConsoleDebug` outside resolution-tracking comments
(`MuConsoleCommands.{cpp,h}`, `MuLogger.h`, one comment in
`PlatformCompat.h:2149`). Cleanup landed between doc creation and
verification.

#### #4 — Lingering OpenGL immediate-mode calls ✓ resolved
`Story 7.9.6` removed the raw-GL backend; this finding flagged
`glColor3f`/`glPushMatrix`/`glTranslatef`/`glMatrixMode`/`glLoadIdentity`/
`glClear`/`glPopMatrix` survivors in `ZzzEffectFireLeave.cpp`,
`ZzzObject.cpp`, `NewUIInGameShop.cpp`. Re-grep on 2026-04-27 returns
**zero immediate-mode hits** outside the `wglGetProcAddress` /
`wglGetCurrentDC` stubs in `PlatformCompat.h` (covered by 7-9-5's
Category B deletion).

The only surviving raw-GL reference now is texture-object management
in `Gameplay/Items/ZzzInventory.cpp:7524, 8066, 8072-8077`
(`glGenTextures`, `glBindTexture`, `glTexParameteri`, `glDeleteTextures`,
`glTexImage2D`) — but those calls live inside an `#else` branch of
`#ifdef MU_ENABLE_SDL3` (block 7994-8087). On the SDL3 path the
function returns false at line 7999 before the `gl*` calls. Vestigial
legacy that **7-9-19** strips with the rest of the `MU_ENABLE_SDL3`
axis. No additional story needed.

### Cross-cutting `#ifdef` cleanup (subsumed by **7-9-19**)

#### #2 — SDL3 helpers in wrong `#ifdef` branch
`Platform/PlatformCompat.h` has `#ifdef _WIN32 ... #else ... #endif`
(lines 3 → 50 → 2285). SDL3 helpers sit inside the `#else` branch
(nested `#ifdef MU_ENABLE_SDL3 ...` at ~924-960), so they're invisible
to MinGW (which defines `_WIN32`):
- `extern char g_szSDLTextInput[32];`
- `extern bool g_bSDLTextInputReady;`
- `void MuStartTextInput();`, `void MuStopTextInput();`
- `inline wchar_t MuSdlUtf8NextChar(const char*&);`

**Fix**: lift the SDL3 block out of the platform `#ifdef` and gate only
on `MU_ENABLE_SDL3`.

**Worth verifying anyway**: same root cause may affect `mu_narrow_path`
used by `Data/GlobalText.h`'s template via GCC two-phase name lookup —
could bite plain Linux GCC, not just MinGW. Reproduce before deferring.

### Cross-platform type/header point-fixes (verified live 2026-04-27)

#### #3 — POSIX-only types/headers in unconditional code
All three sites confirmed live; will break MSVC even though MinGW
support was deprioritized. **Now part of 7-9-18 scope** (cross-platform
point-fixes — same shape as the existing `GetCurrentDirectory` /
`ShellExecute` / `IsBadReadPtr` items in that story):

- `RenderFX/ZzzBMD.cpp:1022` — `static_cast<u_char>(...)`. Fix: `unsigned char`.
- `Network/WSclient.cpp:566-567` — `u_int64`. Fix: `uint64_t`.
- `Core/MuSystemInfo.cpp:13` — unconditional `#include <sys/utsname.h>`
  + `uname()`. Fix: cross-platform `mu::platform::OsName()` wrapping
  `RtlGetVersion` on Windows, `uname()` elsewhere.

### Effectively mitigated (verified 2026-04-27)

#### #5 — `MuPlatform::CreateWindow` Win32 macro collision ✓ mitigated
Risk only realises if a translation unit pulls in both `<windows.h>`
and `MuPlatform.h`. Verified that **no current TU does**, and
`PlatformCompat.h:2016` carries an explicit comment:
*"CreateWindow macro NOT defined here — conflicts with MuPlatform::CreateWindow()"*.
Belt-and-suspenders is a 1-line `#undef CreateWindow` at the top of
`MuPlatform.h`; **added as a task to 7-9-14** (which owns Win32-backend
cleanup) rather than a standalone story.

#### #7 — Stale `../`-prefixed `#include` paths ✓ mostly resolved
Most of Mosch's listed sites have been cleaned up. Re-grep on
2026-04-27 finds **only one survivor**:
`Scenes/SceneManager.cpp:51 → ../MuEditor/Core/MuEditorCore.h`
(and only on the `_EDITOR` build path). Single-line follow-up; not
worth a story. Either fold into the next ambient editor cleanup or
fix opportunistically.

Related: `Data/GlobalText.h` should `#include "Platform/PlatformCompat.h"`
directly rather than rely on transitive includes — GCC two-phase name
lookup needs `mu_narrow_path` visible. Not yet verified.

## Gemini bot inline comments

All previously deferred-style nits — none are blockers.

| File | Line | Comment |
|------|------|---------|
| `Core/BaseCls.h` | 121 | `CList`: missing Rule-of-Three. Add `= delete` copy ops since it manages raw pointers. |
| `Core/BaseCls.h` | 832 | `CDimension`: same — manages `m_pData` raw pointer. |
| `Core/IniFile.h` | 115 | `std::locale("")` env-dependent. **Already addressed** in `cb96217c` (UTF-8 facet). |
| `src/CMakeLists.txt` | 277 | `include_directories` global → prefer `target_include_directories(MUCommon INTERFACE ...)`. |
| `src/CMakeLists.txt` | 430 | `file(GLOB_RECURSE)` discouraged for source lists. |
| `CMakeLists.txt` (root) | 131 | Same `file(GLOB)` concern on shader blobs. **Mitigated** in `cb96217c` (`CONFIGURE_DEPENDS`). |

## Mosch's CI-coverage observation (worth flagging)

> Worth flagging up front: the existing `.github/workflows/ci.yml` covers four
> jobs - Quality Gates, Linux Native (system GCC), macOS Native (Brew clang),
> and Windows Native (`windows-latest` + MSYS2 **MinGW-w64 x86_64**, *not*
> MSVC, despite the `CMakePresets.json` `windows-base` description saying
> "MSVC"). The toolchain files don't actually force MSVC - they trust whatever
> compiler is on PATH, and the CI installs MinGW.

Implication: **MSVC has had zero CI coverage on `main` historically.**
Anyone building from Visual Studio is the first to hit MSVC-specific issues.

Our `2aefc9ef` switched the Windows job from MSYS2/MinGW to MSVC + vcpkg —
that's a meaningful CI coverage improvement, but as of this writing the
Windows job hasn't reached green yet. The "MSVC" wording in the preset
description is now accurate.

## Suggested research order (revised 2026-04-27)

Items 1 (#1, #4) verified and resolved; items now active:

1. **#8 + #9 Mosch's preferred fixes** — graceful CURL fallback + preset
   toolchain cleanup. Both small CMake changes; **filed as story 7-9-20**.
2. **#3 partial** — three POSIX-only-types/headers point-fixes.
   **Added to 7-9-18 scope** alongside the existing
   `GetCurrentDirectory` / `ShellExecute` / `IsBadReadPtr` items.
3. **#2 verification + cleanup** — confirm whether `mu_narrow_path`
   two-phase name lookup actually fires on Linux GCC. **Subsumed by
   7-9-19** (strip-conditional-compilation), which strips the
   `PlatformCompat.h` `#ifdef _WIN32 ... #else` block that hides the
   SDL3 helpers from MinGW. Verification step folded into 7-9-19's
   tasks.
4. **#5 belt-and-suspenders** — `#undef CreateWindow` at top of
   `MuPlatform.h`. **Added as task to 7-9-14**.
5. **#7 last survivor** — single `../`-prefixed include in
   `SceneManager.cpp:51`. Opportunistic fix; no story.

Forward-looking items (no story yet, file when triggered):
- #6 GCC-flavoured relaxations (wait for actual GCC trip in CI)
- #6 MUData/MUCore relaxations (wait for actual trip)
- Gemini bot Rule-of-Three on `CList` / `CDimension` (code-hygiene
  sweep candidate)
