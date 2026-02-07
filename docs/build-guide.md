# CMake Build System — Platform Guide

This document explains how the build system works across all supported environments, what directories are created and why, and how to set up each environment correctly.

---

## Quick Reference

| Environment | Best for | Build speed | Setup effort |
|-------------|----------|-------------|--------------|
| WSL terminal + MinGW | Daily development, Claude Code | Fast | Medium (one-time) |
| CLion via Z: drive + MSVC | Quick MSVC testing (no file copy) | Slow (cross-filesystem I/O) | Easy |
| CLion with native Windows path + MSVC | Debugging, full-speed MSVC builds | Fast | Easy (manual file sync) |
| Windows native + MSVC (VS presets) | Full-speed MSVC builds | Fast | Easy (manual file sync) |

**Recommended workflow:** Use WSL + MinGW for daily development and Claude Code. Use CLion/MSVC only when you need the debugger or MSVC-specific testing.

---

## Supported Build Scenarios

| Scenario | Compiler | CMake runs on | dotnet used | Status |
|----------|----------|---------------|-------------|--------|
| Windows (VS presets) | MSVC x86 | Windows | `dotnet.exe` (Windows) | Supported |
| CLion via Z: drive | MSVC x86 | Windows | `dotnet.exe` (Windows) | Supported (slow I/O) |
| CLion with native Windows path | MSVC x86 | Windows | `dotnet.exe` (Windows) | Supported (fast) |
| WSL terminal (MinGW) | MinGW i686 | Linux (WSL) | `dotnet.exe` (Windows, via WSL interop) | Supported |
| Pure Linux (no WSL) | MinGW i686 | Linux | `dotnet` (Linux) | Partial — no DLL |
| CLion on Linux | MinGW i686 | Linux | `dotnet` (Linux) | Untested — no DLL |

---

## Setup Guide

### WSL Terminal — MinGW (Recommended for Development)

**Where to keep the repo:** On the WSL filesystem (`/home/<user>/MuMain`). This is the fast path — all file I/O stays on native Linux ext4.

**Do NOT** put the repo on `/mnt/c/...` (Windows filesystem from WSL). That path goes through a slow translation layer and is significantly slower than native WSL filesystem.

**Prerequisites (one-time):**

```bash
sudo apt-get update && sudo apt-get install -y mingw-w64 g++-mingw-w64-i686 cmake ninja-build
```

**Configure and build:**

```bash
cmake -S . -B build-mingw -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/mingw-w64-i686.cmake \
  -DCMAKE_BUILD_TYPE=Debug \
  -DENABLE_EDITOR=ON \
  -DMU_TURBOJPEG_STATIC_LIB=_deps/mingw-i686/lib/libturbojpeg.a

cmake --build build-mingw -j$(nproc)
```

**Running the game:** The post-build step copies all assets from `src/bin/` into `build-mingw/src/`. Run from the build output directory:

```bash
cd build-mingw/src && ./Main.exe
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

### 4. WSL Terminal — MinGW Cross-Compile

```
/home/<user>/MuMain/                  <-- source + build (all on WSL filesystem)
  .nuget/                             <-- NuGet cache (in project root)
  build-mingw/                        <-- build output (CMAKE_BINARY_DIR)
    src/
      Main.exe                        <-- PE32 Windows executable + copied assets
      MUnique.Client.Library.dll
    dotnet_out/
    .dotnet_temp/
```

### 5. Pure Linux (No WSL, No Windows dotnet)

```
/home/<user>/MuMain/
  build-mingw/
    src/
      Main.exe                        <-- built, but no DLL
```

The Linux `dotnet` SDK cannot produce a Windows `.dll` via Native AOT (cross-OS AOT is not supported). CMake prints a warning and skips the .NET build. The game runs but cannot connect to a server.

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

```cmake
find_program(DOTNET_EXECUTABLE dotnet.exe)   # prefer Windows binary
if (NOT DOTNET_EXECUTABLE)
  find_program(DOTNET_EXECUTABLE dotnet)     # fallback to Linux binary
endif()
```

- **WSL:** Finds `dotnet.exe` at `/mnt/c/Program Files/dotnet/dotnet.exe` via WSL interop. Produces correct Windows DLLs with Native AOT.
- **Native Windows:** Finds `dotnet.exe` directly.
- **Pure Linux:** Falls back to `dotnet`. Cannot produce Windows DLLs.
- **No SDK installed:** CMake warns and skips the .NET build.

---

## Path Translation (WSL Only)

When CMake detects it's running on Linux (`CMAKE_HOST_SYSTEM_NAME == "Linux"`) and found a Windows `dotnet.exe`, it converts all paths passed to `dotnet.exe` using `wslpath -w`:

| Linux path | Converted Windows path |
|------------|----------------------|
| `/home/<user>/MuMain/ClientLibrary/...csproj` | `\\wsl.localhost\Ubuntu\home\<user>\MuMain\ClientLibrary\...csproj` |
| `/home/<user>/MuMain/build-mingw/dotnet_out` | `\\wsl.localhost\Ubuntu\home\<user>\MuMain\build-mingw\dotnet_out` |

Paths used by CMake-native commands (`copy_if_different`, `DEPENDS`) stay as Linux paths.

The guard `CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux"` ensures `wslpath` is never called on native Windows (where it doesn't exist).
