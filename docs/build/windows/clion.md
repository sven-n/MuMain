# Windows - CLion

CLion on Windows builds with MSVC. There are two ways to point it at the
source, with the same output and different speed:

| Approach | How | Tradeoff |
|----------|-----|----------|
| **Z: drive** (mapped WSL filesystem) | Reads source from your WSL repo via a mapped drive - one checkout. | Slower: each file read crosses the WSL<->Windows boundary. |
| **Native Windows path** | Repo cloned on a Windows drive (`D:\repos\MuMain`). | Faster (local NTFS); a separate checkout. |

Either way, put the **build directory on a Windows drive** (`C:`/`D:`) for fast
builds and runtime asset reads. See [the build guide](../README.md) for shared
concepts.

## Native Windows path (recommended)

1. Clone the repo onto a Windows drive, e.g. `D:\repos\MuMain`.
2. Open it in CLion.
3. CMake profile (`Settings > Build, Execution, Deployment > CMake`):
   - CMake options:
     ```
     -G Ninja -DCMAKE_TOOLCHAIN_FILE=D:/repos/MuMain/toolchain-x64.cmake -DENABLE_EDITOR=ON
     ```
     (Use `toolchain-x86.cmake` for a 32-bit build.)
4. Run config: set the **Working directory** to the build output, e.g.
   `D:\repos\MuMain\cmake-build-x64debug-mueditor\src\Debug`. The post-build
   step copies the assets and `MUnique.Client.Library.dll` there.

## Z: drive (single checkout on WSL)

1. Map WSL as a drive (PowerShell, one-time, persistent):
   ```powershell
   net use Z: \\wsl.localhost\Ubuntu /persistent:yes
   ```
2. Allow Windows git to trust the WSL-owned repo (one-time):
   ```powershell
   git config --global --add safe.directory '%(prefix)///wsl.localhost/Ubuntu/home/<user>/MuMain'
   git config --global --add safe.directory '%(prefix)///wsl.localhost/Ubuntu/home/<user>/MuMain/src/ThirdParty/imgui'
   ```
   Use the UNC `%(prefix)///wsl.localhost/...` form, not `Z:/...`.
3. Open `Z:\home\<user>\MuMain` in CLion.
4. CMake options:
   ```
   -G Ninja -DCMAKE_TOOLCHAIN_FILE=Z:/home/<user>/MuMain/toolchain-x64.cmake -DENABLE_EDITOR=ON
   ```
5. **Build directory:** set to a Windows-native path, e.g.
   `C:\build\MuMain-x64debug-mueditor` (do not build onto Z:).
6. Run config: Working directory = `C:\build\MuMain-x64debug-mueditor\src\Debug`.

> If `ilc.exe` from the NuGet cache fails to execute off the WSL filesystem,
> override the cache to a Windows path:
> `-DMU_NUGET_CACHE_DIR=C:/.mu-nuget`.
