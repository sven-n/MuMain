# CMake Build System — Platform Guide

This document explains how the build system works across all supported environments, what directories are created and why, and how to set up each environment correctly. All commands assume you are inside the `MuMain/` directory unless stated otherwise.

---

## Project Scope

MuMain is a C++20 MMORPG game client (MU Online Season 5.2→6) that connects to [OpenMU](https://github.com/MUnique/OpenMU) servers. The repository is self-contained — all build instructions work from the `MuMain/` directory without any parent workspace.

### Build Artifacts

| Artifact | Language | Build Tool | Output |
|----------|----------|------------|--------|
| **Game Client** (`Main.exe`) | C++20 | CMake + Ninja | 32-bit Windows PE (OpenGL, Win32, DirectSound) |
| **Network DLL** (`MUnique.Client.Library.dll`) | C# .NET 10 | `dotnet publish` (Native AOT) | Windows DLL loaded at runtime by the game |
| **Debug Editor** (built into `Main.exe`) | C++20 | CMake (`-DENABLE_EDITOR=ON`) | ImGui overlay, toggled with F12 at runtime |
| **Packet Bindings** (generated code) | XSLT→C++/C# | Visual Studio PreBuild | `src/source/Dotnet/PacketBindings_*.h`, `PacketFunctions_*.h/.cpp` |

### Source Layout

```
MuMain/
├── src/source/           # C++ game client (691 files in 20 module dirs)
│   ├── Main/             # Entry point: MuMain.cpp → WinMain()
│   ├── Core/             # Shared utilities, error reporting, timers
│   ├── Platform/         # Cross-platform abstraction (SDL3 shims, PlatformCompat.h)
│   ├── Dotnet/           # C++↔.NET interop (Connection.h/cpp + generated packet bindings)
│   └── ...               # 16 more module directories (UI, Network, Rendering, etc.)
├── src/MuEditor/         # ImGui debug editor (34 files, compiled with _EDITOR define)
├── src/bin/              # Game assets (13,169 files) — copied to build dir by post-build step
├── ClientLibrary/        # .NET 10 Native AOT network library (14 C# files)
├── tests/                # Catch2 unit tests
├── cmake/toolchains/     # Platform toolchains (windows-x64, linux-x64, macos-arm64)
├── CMakePresets.json      # All platform presets (windows-x86/x64, linux-x64, macos-arm64)
├── Makefile              # Quality gate targets (format, lint, test) — run from MuMain/
└── .clang-format         # Formatting rules (Allman braces, 4-space indent, 120 col)
```

### Build Capability by Platform

| Platform | Full Compile | CMake Configure | Quality Gate | .NET AOT Library | Runnable Binary |
|----------|-------------|----------------|-------------|-----------------|----------------|
| **Windows + MSVC** | Yes (native) | Yes | Yes | Yes (native `dotnet.exe`) | Yes |
| **macOS (arm64)** | Yes (native via SDL3) | Yes | Yes | Yes (produces `.dylib`) | Yes |
| **Linux native (x64)** | Yes (native via SDL3) | Yes | Yes | Yes (produces `.so`) | Yes |
| **Any platform without `dotnet`** | Yes (game client only) | Yes | Yes | No — CMake warns and skips | Game runs but cannot connect to servers |

### Quality Gates (from MuMain/ directory)

Quality gates run on **all** platforms (no Win32 dependency). These mirror what CI enforces on every PR:

```bash
# Run from MuMain/ directory
make format-check    # clang-format --dry-run --Werror on all C++ files (excludes ThirdParty/ and Dotnet/)
make lint            # cppcheck with enforced warnings (error-exitcode=1)
make format          # auto-fix formatting in-place
make test            # build + run Catch2 unit tests (cmake -DBUILD_TESTING=ON)
```

**Tool requirements:** `clang-format`, `cppcheck` (install via `brew install` on macOS, `apt-get install` on Linux).

### Cross-Platform Build Gaps

The codebase is mid-migration from Win32/OpenGL to SDL3/SDL_gpu. Here is what blocks full compilation on non-Windows:

| Blocker | Files Affected | Migration Phase | Status |
|---------|---------------|----------------|--------|
| **Win32 windowing** (`CreateWindowEx`, `RegisterClass`, `ShowWindow`) | `MuMain.cpp`, `ZzzOpenglUtil.cpp` | EPIC-2 (SDL3 Window) | Not started |
| **Win32 input** (`GetAsyncKeyState`, `SetCapture`, `WM_*` messages) | 8 files, 104+ calls | EPIC-2 (SDL3 Input) | Not started |
| **OpenGL immediate mode** (`glBegin`/`glEnd`/`glVertex*`) | 14 files, 111 call sites | EPIC-2 (SDL_gpu) | Not started |
| **DirectSound** (`DirectSoundCreate`, `IDirectSound*`) | `MuMain.cpp`, audio files | EPIC-3 (miniaudio) | Not started |
| **Win32 file I/O** (`GetModuleFileName`, `GetPrivateProfileInt`) | 5–28 files | EPIC-5 (Config) | Partially done |
| **Win32 timers** (`SetTimer`/`KillTimer`) | 10 files, 20 calls | EPIC-5 | Not started |
| **Win32 text input** (`CreateWindowW L"edit"`, `WM_CHAR`) | `UIControls.cpp` | EPIC-6 | Done (SDL3 text input implemented) |

**What already works cross-platform:**
- CMake configure succeeds on all platforms (validates build system + SDL3 FetchContent)
- Platform abstraction headers exist: `PlatformTypes.h`, `PlatformCompat.h`, `PlatformKeys.h`, `StringConvert.h` (in `src/source/Platform/`)
- Timing shims: `timeGetTime()` and `GetTickCount()` are shimmed to `std::chrono::steady_clock` on non-Windows
- File I/O shim: `_wfopen` → `mu_wfopen` with backslash normalization and case-insensitive fallback
- SDL3 text input: `SDL_StartTextInput()`/`SDL_StopTextInput()` replace Win32 edit controls
- `MessageBoxW` shimmed to `SDL_ShowSimpleMessageBox`
- All quality gate tooling (`clang-format`, `cppcheck`, `Catch2` tests)

### .NET Native AOT Library

The `ClientLibrary/` directory contains a .NET 10 Native AOT library that handles server communication. CMake auto-detects the platform and produces the correct native library:

| Platform | RID | Output | Loader |
|----------|-----|--------|--------|
| Windows x86 | `win-x86` | `.dll` | `LoadLibrary` |
| Windows x64 | `win-x64` | `.dll` | `LoadLibrary` |
| Linux x64 | `linux-x64` | `.so` | `dlopen` |
| Linux arm64 | `linux-arm64` | `.so` | `dlopen` |
| macOS arm64 | `osx-arm64` | `.dylib` | `dlopen` |
| macOS x64 | `osx-x64` | `.dylib` | `dlopen` |

- **Detection**: `cmake/FindDotnetAOT.cmake` auto-detects the platform RID, library extension, and locates `dotnet`.
- **Without `dotnet` installed**: CMake prints a warning and skips the .NET build. The game compiles and runs but cannot connect to any server.

### Formatting Rules

Defined in `.clang-format` at the repo root:
- Style: LLVM base with Allman braces
- Indent: 4 spaces (no tabs)
- Column limit: 120
- C++20 standard
- Pointer alignment: left (`int* p`)
- Include sorting: disabled (`SortIncludes: Never`)

---

## Quick Reference

| Environment | Best for | Build speed | Setup effort |
|-------------|----------|-------------|--------------|
| Windows native + MSVC (VS presets) | Full-speed Windows builds | Fast | Easy |
| macOS native (arm64) | macOS development, Claude Code | Fast | Easy (brew) |
| Linux native (x64) | Linux development, Claude Code | Fast | Easy (apt) |

**Recommended workflow:** Build natively on your platform using CMake presets. All three platforms are first-class.

---

## Supported Build Scenarios

| Scenario | Compiler | CMake runs on | dotnet used | Status |
|----------|----------|---------------|-------------|--------|
| Windows (VS presets) | MSVC x64 | Windows | `dotnet.exe` (Windows) | Supported |
| macOS (arm64 preset) | Clang | macOS | `dotnet` (macOS) | Supported |
| Linux (x64 preset) | GCC 12+ | Linux | `dotnet` (Linux) | Supported |

---

## Setup Guide

### Windows — MSVC (Native)

```powershell
cmake --preset windows-x64
cmake --build out/build/windows-x64 --config Debug
```

### macOS — Clang (Native)

```bash
brew install cmake ninja openssl@3 glslang spirv-cross
cmake --preset macos-arm64 -DOPENSSL_ROOT_DIR=$(brew --prefix openssl@3)
cmake --build out/build/macos-arm64 --config Debug
```

### Linux — GCC (Native)

```bash
sudo apt-get install -y cmake ninja-build gcc g++ libgl1-mesa-dev libssl-dev libcurl4-openssl-dev glslang-tools spirv-cross
cmake --preset linux-x64
cmake --build out/build/linux-x64 --config Debug
```

### CLion on Windows — Which Approach?

CLion on Windows uses the MSVC compiler. There are two ways to point CLion at the source code:

| Approach | How it works | Tradeoff |
|----------|-------------|----------|
| **Z: drive** (mapped WSL filesystem) | CLion reads source files directly from your WSL repo via a mapped network drive. No file copying needed. | Slower builds — every file read crosses the WSL↔Windows boundary. |
| **Native Windows path** | You copy/clone the repo onto a Windows drive (e.g. `D:\repos\MuMain`). CLion reads files from fast local NTFS. | Faster builds, but you maintain a separate copy of the repo. |

Pick **Z: drive** if you want a single repo checkout and don't mind slower builds.
Pick **native Windows path** if you want fast MSVC builds and are okay syncing/copying files manually.

Both approaches produce identical output. The only difference is build speed.

---

### CLion on Windows via Z: Drive

This approach maps your WSL filesystem as a Windows drive letter so CLion can open the repo directly — no file copying needed.

**Prerequisites:**

1. The repo must already exist on the WSL filesystem (e.g. `/home/<user>/MuMain`).
2. WSL must be running.
3. CLion must be installed on Windows.

**Step 1 — Map the WSL filesystem as Z: drive (one-time, persistent across reboots):**

Open PowerShell and run:

```powershell
net use Z: \\wsl.localhost\Ubuntu /persistent:yes
```

This makes your entire WSL home directory accessible as `Z:\home\<user>\` from Windows. The `/persistent:yes` flag means the mapping survives reboots (as long as WSL is running).

**Step 2 — Fix git "dubious ownership" warning (one-time):**

Windows git flags WSL-owned repos as untrusted. Without this fix, CLion's git integration won't work. Run in PowerShell:

```powershell
git config --global --add safe.directory '%(prefix)///wsl.localhost/Ubuntu/home/<user>/MuMain'
git config --global --add safe.directory '%(prefix)///wsl.localhost/Ubuntu/home/<user>/MuMain/src/ThirdParty/imgui'
```

Note: You must use the UNC format above (`///wsl.localhost/...`), not `Z:/...` — git internally resolves the Z: drive back to the UNC path.

**Step 3 — Open the project in CLion:**

Open `Z:\home\<user>\MuMain` in CLion (File > Open).

**Step 4 — Configure CMake in CLion** (Settings > Build, Execution, Deployment > CMake):

- **CMake options:**
  ```
  -G Ninja -DCMAKE_TOOLCHAIN_FILE=Z:/home/<user>/MuMain/toolchain-x86.cmake -DENABLE_EDITOR=ON
  ```
- **Build directory:** Set to a **Windows-native path** for faster builds:
  ```
  C:\build\MuMain-x86debug-mueditor
  ```
  This is important: even though source files are on Z:, putting build output on a local Windows drive (C: or D:) avoids slow cross-filesystem writes.

**Step 5 — Set up the Run Configuration:**

Set the **Working directory** to the build output directory:
```
C:\build\MuMain-x86debug-mueditor\src\Debug
```

Do NOT set it to `Z:\...\src\bin`. The post-build step automatically copies all game assets (data files, textures, etc.) into the build output directory, and using the Windows-native path avoids slow cross-filesystem reads at runtime.

---

### CLion on Windows with Native Windows Path

This approach uses a repo cloned directly on a Windows drive. Everything is on fast local NTFS — no cross-filesystem penalty.

**Prerequisites:**

1. Clone or copy the repo onto a Windows drive (e.g. `D:\repos\MuMain`).
2. CLion must be installed on Windows.

**Step 1 — Open the project in CLion:**

Open `D:\repos\MuMain` in CLion (File > Open).

**Step 2 — Configure CMake in CLion** (Settings > Build, Execution, Deployment > CMake):

- **CMake options:**
  ```
  -G Ninja -DCMAKE_TOOLCHAIN_FILE=D:/repos/MuMain/toolchain-x86.cmake -DENABLE_EDITOR=ON
  ```
- **Build directory:** Can use CLion's default (inside the project) or a custom path. Both are fast since everything is on native NTFS.

**Step 3 — Set up the Run Configuration:**

Set the **Working directory** to the build output directory:
```
D:\repos\MuMain\cmake-build-x86debug-mueditor\src\Debug
```

The post-build step copies all game assets there automatically.

**Note:** Visual Studio cannot open projects from the Z: drive (WSL filesystem), but works fine with native Windows paths like `D:\repos\MuMain`.

### Windows — Visual Studio Presets

```powershell
cmake --preset windows-x86                # standard x86
cmake --preset windows-x86-mueditor       # x86 with ImGui editor

cmake --build --preset windows-x86-debug
cmake --build --preset windows-x86-mueditor-debug
```

**Working directory for running:** `out/build/windows-x86/src/Debug/` (assets are copied there by the post-build step).

---

## Performance Notes

| Operation | WSL filesystem | Z: drive from Windows | Native Windows (C:\) |
|-----------|---------------|----------------------|---------------------|
| File reads (compilation) | Fast | Slow | Fast |
| File writes (build output) | Fast | Slow | Fast |
| Git operations | Fast | Slow | Fast |
| Claude Code file operations | Fast | N/A | N/A |
| Game runtime (data file reads) | Fast (WSL interop) | Slow | Fast |

**Key takeaway:** Keep the repo on WSL for Claude Code. For CLion/MSVC, put the build output on a Windows drive. Accept that source file reads from Z: will be slower — this is a tradeoff for having a single repo checkout.

---

## Directory Layout Per Scenario

### 1. Windows — Visual Studio Presets

```
MuMain/                               <-- on Windows filesystem (C:\...)
  .nuget/                             <-- NuGet package cache (default, configurable)
  out/build/windows-x86/              <-- build output (CMAKE_BINARY_DIR)
    src/
      Debug/
        Main.exe                      <-- game executable + copied assets
        MUnique.Client.Library.dll
    dotnet_out/                       <-- intermediate .NET publish output
    .dotnet_temp/                     <-- .NET CLI temp (avoids umlaut paths)
```

### 2. CLion on Windows via Z: Drive

```
Z:\home\<user>\MuMain\               <-- source (mapped WSL filesystem)
  .nuget\                             <-- NuGet cache (in project root)

C:\build\MuMain-x86debug-mueditor\   <-- build directory (Windows-native, fast)
  src/
    Debug/
      Main.exe                        <-- game executable + copied assets
      MUnique.Client.Library.dll
  dotnet_out/
  .dotnet_temp/
```

**Note:** If `ilc.exe` from the NuGet cache fails to execute on the WSL filesystem, override with a Windows-native path: `cmake -DMU_NUGET_CACHE_DIR=C:/.mu-nuget ...`

### 3. CLion on Windows with Native Windows Path

```
D:\repos\MuMain\                      <-- source (on Windows drive)
  .nuget\                             <-- NuGet cache (in project root)
  cmake-build-x86debug-mueditor\      <-- build directory (CLion default or custom)
    src/
      Debug/
        Main.exe                       <-- game executable + copied assets
        MUnique.Client.Library.dll
    dotnet_out/
    .dotnet_temp/
```

### 4. Linux Native Build

```
/home/<user>/MuMain/
  out/build/linux-x64/                <-- build output (CMAKE_BINARY_DIR)
    src/
      Debug/
        Main                          <-- native Linux executable + copied assets
        MUnique.Client.Library.so
    _deps/                            <-- FetchContent (SDL3, spdlog, etc.)
```

---

## NuGet Cache Location

The NuGet cache defaults to `<project_root>/.nuget` on all platforms. This keeps it inside the repo (already in `.gitignore`) and avoids issues with special characters in user profile paths.

Override with:

```bash
cmake -DMU_NUGET_CACHE_DIR=D:/my-nuget-cache ...
```

**Why not the default NuGet cache?** The system default is `%USERPROFILE%\.nuget\packages`. If the Windows username contains special characters (e.g. umlauts), the Native AOT IL compiler (`ilc.exe`) can fail. Using a project-local path avoids this.

**Edge case — CLion via Z: drive:** The NuGet cache contains native executables like `ilc.exe`. Windows may fail to execute `.exe` files stored on the WSL ext4 filesystem via `\\wsl.localhost\`. If this happens, override to a Windows-native path: `cmake -DMU_NUGET_CACHE_DIR=C:/.mu-nuget ...`

---

## How dotnet Is Found

Detection is handled by `cmake/FindDotnetAOT.cmake`. The search order depends on the environment:

**WSL (detected via `/proc/version` containing "Microsoft" or "WSL"):**
1. Native Linux `dotnet` — searched at `$DOTNET_ROOT`, `/usr/local/share/dotnet`, `/usr/share/dotnet`, `~/.dotnet`
2. Windows `dotnet.exe` via interop — `/mnt/c/Program Files/dotnet/dotnet.exe`

**All other platforms (Windows, native Linux, macOS):**
1. `dotnet` — searched at `$DOTNET_ROOT`, `/usr/local/share/dotnet`, `/usr/share/dotnet`, `~/.dotnet`, plus system PATH

The platform RID (`win-x86`, `linux-x64`, `osx-arm64`, etc.) is determined by `CMAKE_SYSTEM_NAME` and `CMAKE_SYSTEM_PROCESSOR`, not by which `dotnet` binary is found. Native AOT produces a library native to the **target** platform:
- Windows target → `.dll`
- Linux target → `.so`
- macOS target → `.dylib`

If no `dotnet` is found, CMake prints a warning and sets `DOTNETAOT_FOUND=FALSE`. The game compiles without server connectivity.

---

## Path Translation (WSL Only)

When CMake detects it's running on Linux (`CMAKE_HOST_SYSTEM_NAME == "Linux"`) and found a Windows `dotnet.exe`, it converts all paths passed to `dotnet.exe` using `wslpath -w`:

| Linux path | Converted Windows path |
|------------|----------------------|
| `/home/<user>/MuMain/ClientLibrary/...csproj` | `\\wsl.localhost\Ubuntu\home\<user>\MuMain\ClientLibrary\...csproj` |
| `/home/<user>/MuMain/out/build/linux-x64/dotnet_out` | `\\wsl.localhost\Ubuntu\home\<user>\MuMain\out\build\linux-x64\dotnet_out` |

Paths used by CMake-native commands (`copy_if_different`, `DEPENDS`) stay as Linux paths.

The guard `CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux"` ensures `wslpath` is never called on native Windows (where it doesn't exist).
