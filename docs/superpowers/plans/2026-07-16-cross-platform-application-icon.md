# Cross-Platform Application Icon Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Restore the existing MU emblem at high resolution and show it as the native application/window icon on Windows, macOS, and Linux.

**Architecture:** Keep one approved 1024×1024 transparent PNG as canonical raster artwork, then generate platform assets from it. Windows continues using its resource script, macOS becomes a CMake application bundle with an `.icns` resource, and Linux applies the bundled PNG through SDL3 immediately after window creation. No new image-loading dependency: SDL3 provides `SDL_LoadPNG()`.

**Tech Stack:** C++20, SDL3, CMake 3.25+, macOS `sips`/`iconutil`, Windows resources, Bash.

---

## File Map

- Create `src/source/App/Resources/MuMainIcon1024.png`: approved restored master artwork.
- Create `src/source/App/Platform/macOS/MuMain.icns`: generated macOS icon family.
- Create `src/bin/MuMainIcon.png`: 256×256 runtime icon copied beside executable.
- Replace `src/source/App/Platform/Windows/icon1.ico`: generated Windows icon family.
- Modify `src/source/App/Platform/Windows/Winmain.cpp`: load runtime PNG and call `SDL_SetWindowIcon()` off Windows.
- Modify `src/CMakeLists.txt`: configure macOS bundle and package `.icns`.
- Modify workspace-root `ctl`: resolve macOS bundle executable while preserving its asset working directory.
- Create `scripts/generate-app-icons.sh`: reproducibly regenerate platform assets on macOS.
- Create `scripts/build_ico.py`: package generated PNG sizes into one Windows ICO using Python stdlib.

### Task 1: Restore Existing Artwork

**Files:**
- Read: `src/source/App/Platform/Windows/icon1.ico`
- Create: `src/source/App/Resources/MuMainIcon1024.png`

- [ ] **Step 1: Produce restoration preview**

Create a 1024×1024 transparent PNG by tracing the current icon. Preserve star/emblem composition, central red details, gold-orange palette, and transparent outer silhouette. Correct symmetry, jagged edges, muddy transitions, and collapsed details. Add only restrained metallic highlights and dark line separation.

- [ ] **Step 2: Verify identity at target sizes**

Run:

```bash
mkdir -p /tmp/mumain-icon-check
for size in 16 32 48 128 256 512; do
  sips -z "$size" "$size" src/source/App/Resources/MuMainIcon1024.png \
    --out "/tmp/mumain-icon-check/icon-${size}.png"
done
```

Expected: every generated image retains the existing emblem silhouette; 16×16 and 32×32 remain recognizable without becoming a gold blur.

- [ ] **Step 3: Get user artwork approval**

Show the 48, 128, 256, and 1024 previews together. Do not continue until user confirms the artwork preserves the current look.

- [ ] **Step 4: Commit master artwork**

```bash
git add src/source/App/Resources/MuMainIcon1024.png
git commit -m "assets: restore application icon artwork"
```

### Task 2: Generate Platform Assets

**Files:**
- Create: `scripts/generate-app-icons.sh`
- Create: `scripts/build_ico.py`
- Create: `src/source/App/Platform/macOS/MuMain.icns`
- Create: `src/bin/MuMainIcon.png`
- Replace: `src/source/App/Platform/Windows/icon1.ico`

- [ ] **Step 1: Write stdlib ICO packer**

Create `scripts/build_ico.py`:

```python
#!/usr/bin/env python3
import struct
import sys
from pathlib import Path


def main() -> None:
    output = Path(sys.argv[1])
    images = [(int(size), Path(path).read_bytes()) for size, path in
              (argument.split("=", 1) for argument in sys.argv[2:])]
    offset = 6 + 16 * len(images)
    entries = []
    payload = bytearray()
    for size, data in images:
        dimension = 0 if size == 256 else size
        entries.append(struct.pack("<BBBBHHII", dimension, dimension, 0, 0,
                                   1, 32, len(data), offset + len(payload)))
        payload.extend(data)
    output.write_bytes(struct.pack("<HHH", 0, 1, len(images)) + b"".join(entries) + payload)


if __name__ == "__main__":
    main()
```

- [ ] **Step 2: Write generation script**

Create `scripts/generate-app-icons.sh`:

```bash
#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source_png="$repo_root/src/source/App/Resources/MuMainIcon1024.png"
iconset="$(mktemp -d)/MuMain.iconset"
trap 'rm -rf "${iconset%/*}"' EXIT
mkdir -p "$iconset"

for spec in \
  '16 icon_16x16.png' '32 icon_16x16@2x.png' \
  '32 icon_32x32.png' '64 icon_32x32@2x.png' \
  '128 icon_128x128.png' '256 icon_128x128@2x.png' \
  '256 icon_256x256.png' '512 icon_256x256@2x.png' \
  '512 icon_512x512.png' '1024 icon_512x512@2x.png'; do
  read -r size name <<< "$spec"
  sips -z "$size" "$size" "$source_png" --out "$iconset/$name" >/dev/null
done

iconutil -c icns "$iconset" \
  -o "$repo_root/src/source/App/Platform/macOS/MuMain.icns"
sips -z 256 256 "$source_png" \
  --out "$repo_root/src/bin/MuMainIcon.png" >/dev/null
sips -z 48 48 "$source_png" \
  --out "${iconset%/*}/icon-48.png" >/dev/null
python3 "$repo_root/scripts/build_ico.py" \
  "$repo_root/src/source/App/Platform/Windows/icon1.ico" \
  "16=$iconset/icon_16x16.png" \
  "32=$iconset/icon_32x32.png" \
  "48=${iconset%/*}/icon-48.png" \
  "256=$iconset/icon_256x256.png"
```

- [ ] **Step 3: Make scripts executable and run them**

```bash
chmod +x scripts/generate-app-icons.sh scripts/build_ico.py
scripts/generate-app-icons.sh
```

Expected: all three output files exist and are non-empty.

- [ ] **Step 4: Verify generated formats and ICO entries**

```bash
file src/source/App/Platform/macOS/MuMain.icns \
     src/source/App/Platform/Windows/icon1.ico \
     src/bin/MuMainIcon.png
sips -g pixelWidth -g pixelHeight src/bin/MuMainIcon.png
python3 - <<'PY'
import struct
from pathlib import Path
data = Path('src/source/App/Platform/Windows/icon1.ico').read_bytes()
reserved, kind, count = struct.unpack_from('<HHH', data)
assert (reserved, kind, count) == (0, 1, 4)
print('ICO entries:', count)
PY
```

Expected: ICNS, Windows icon, PNG; runtime PNG is 256×256; ICO has four entries.

- [ ] **Step 5: Commit generated assets**

```bash
git add scripts/generate-app-icons.sh scripts/build_ico.py \
  src/source/App/Platform/macOS/MuMain.icns \
  src/source/App/Platform/Windows/icon1.ico \
  src/bin/MuMainIcon.png
git commit -m "assets: generate platform application icons"
```

### Task 3: Apply SDL Window Icon on Linux

**Files:**
- Modify: `src/source/App/Platform/Windows/Winmain.cpp:1720`

- [ ] **Step 1: Add runtime icon helper beside SDL bootstrap helpers**

Add:

```cpp
#if defined(__linux__)
static void SetApplicationWindowIcon(SDL_Window* window)
{
    SDL_Surface* icon = SDL_LoadPNG("MuMainIcon.png");
    if (icon == nullptr)
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                    "SDL window icon unavailable: %s", SDL_GetError());
        return;
    }

    SDL_SetWindowIcon(window, icon);
    SDL_DestroySurface(icon);
}
#endif
```

- [ ] **Step 2: Call helper after successful window creation**

Immediately after the `g_sdlWindow == nullptr` failure branch:

```cpp
#if defined(__linux__)
    SetApplicationWindowIcon(g_sdlWindow);
#endif
```

- [ ] **Step 3: Build Linux target**

Run on Linux:

```bash
cmake --preset linux-x64
cmake --build --preset linux-x64-debug --target Main -j2
```

Expected: build succeeds without new linker dependencies.

- [ ] **Step 4: Commit Linux integration**

```bash
git add src/source/App/Platform/Windows/Winmain.cpp
git commit -m "feat(linux): set SDL application icon"
```

### Task 4: Build Native macOS App Bundle

**Files:**
- Modify: `src/CMakeLists.txt:220`
- Modify: workspace-root `ctl:145`

- [ ] **Step 1: Configure CMake bundle and icon resource**

Change target declaration and add Apple properties:

```cmake
if (APPLE)
  add_executable(Main MACOSX_BUNDLE
    "${CMAKE_CURRENT_SOURCE_DIR}/${MU_APP_ENTRY}"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/App/Platform/macOS/MuMain.icns"
  )
  set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/source/App/Platform/macOS/MuMain.icns"
    PROPERTIES MACOSX_PACKAGE_LOCATION Resources
  )
  set_target_properties(Main PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "MU Online"
    MACOSX_BUNDLE_GUI_IDENTIFIER "org.openmu.mumain"
    MACOSX_BUNDLE_ICON_FILE "MuMain.icns"
  )
else()
  add_executable(Main
    "${CMAKE_CURRENT_SOURCE_DIR}/${MU_APP_ENTRY}"
  )
endif()
```

Keep existing target features and libraries after this block unchanged.

- [ ] **Step 2: Teach `ctl` bundle executable path**

In `resolve_mumain_exe()`, after setting `MUMAIN_EXE_DIR`, add:

```bash
if [[ "$(uname -s)" == "Darwin" ]]; then
    MUMAIN_EXE="$MUMAIN_EXE_DIR/Main.app/Contents/MacOS/Main"
else
    MUMAIN_EXE="$MUMAIN_EXE_DIR/$exe_name"
fi
```

Keep `MUMAIN_EXE_DIR` unchanged so relative `Data/` and `config.ini` paths still resolve from the build configuration directory.

- [ ] **Step 3: Validate shell syntax**

```bash
bash -n ../ctl
```

Expected: exit code 0.

- [ ] **Step 4: Build macOS bundle**

```bash
cmake --preset macos-arm64
cmake --build out/build/macos-arm64 --config Release --target Main -j2
```

Expected: `out/build/macos-arm64/src/Release/Main.app/Contents/MacOS/Main` and `Main.app/Contents/Resources/MuMain.icns` exist.

- [ ] **Step 5: Validate bundle metadata**

```bash
plutil -p out/build/macos-arm64/src/Release/Main.app/Contents/Info.plist
./ctl run --release
```

Expected: `CFBundleIconFile` names `MuMain.icns`; Dock and app switcher show restored icon; game assets load normally.

- [ ] **Step 6: Commit bundle integration in each repository**

MuMain commit:

```bash
git add src/CMakeLists.txt
git commit -m "feat(macos): build application bundle with icon"
```

Workspace-root commit:

```bash
git add ctl MuMain
git commit -m "feat(ctl): launch macOS application bundle"
```

### Task 5: Final Cross-Platform Validation

**Files:**
- Verify only; no expected source changes.

- [ ] **Step 1: Validate repository diffs**

```bash
git diff --check
git status --short
```

Expected: no whitespace errors; only intentionally untracked generated graph/brainstorm files remain.

- [ ] **Step 2: Validate Windows resource binding**

```bash
grep -n 'IDI_ICON1.*ICON.*icon1.ico' src/source/App/Platform/Windows/resource.rc
```

Expected: one binding remains.

- [ ] **Step 3: Validate macOS Debug and Release launch resolution**

```bash
cmake --build out/build/macos-arm64 --config Debug --target Main -j2
test -x out/build/macos-arm64/src/Debug/Main.app/Contents/MacOS/Main
test -x out/build/macos-arm64/src/Release/Main.app/Contents/MacOS/Main
```

Expected: all commands exit 0.

- [ ] **Step 4: Refresh graphify code graph**

```bash
python3 -c "from graphify.watch import _rebuild_code; from pathlib import Path; _rebuild_code(Path('.'))"
```

Expected: AST extraction reaches 100%; interrupt later centrality work if it hangs as currently known.

- [ ] **Step 5: Record manual platform results**

Confirm:

```text
Windows: Explorer/taskbar icon = restored MU emblem
macOS: Finder/Dock/app switcher icon = restored MU emblem
Linux: taskbar/window switcher icon = restored MU emblem
```

If a Linux desktop ignores `SDL_SetWindowIcon()`, record compositor/window-manager name; desktop-entry packaging remains outside this plan.
