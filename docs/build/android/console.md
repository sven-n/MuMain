# Android command-line build

The Android client is an arm64/x86_64 SDL application. The current baseline
uses MuMain's OpenGL renderer through GL4ES; the tracked renderer selector keeps
that choice explicit while the SDL_GPU/Vulkan backend is developed.

## Requirements

- Linux or WSL with CMake 3.25+, Ninja, Git, `patchelf`, and .NET SDK 10 with
  Native AOT support.
- Android NDK r28 or newer.
- Windows Android SDK platform 36, Android Studio's JBR, and PowerShell for APK
  packaging. The Gradle wrapper is supplied by the SDL submodule.
- Repository submodules initialized with `git submodule update --init --recursive`.

## Build native libraries

From Linux/WSL:

```bash
export ANDROID_NDK_ROOT=/path/to/android-ndk-r28b
tools/android/build-native.sh
```

The script checks out pinned gl4es, curl, and libjpeg-turbo revisions under
`out/android/dependencies`, builds them for `arm64-v8a`, builds MuMain and the
`linux-bionic-arm64` NativeAOT network library, strips the package copies, and
writes them to `out/android/native/arm64-v8a`.

To build the emulator ABI as well:

```bash
MU_ANDROID_ABI=x86_64 tools/android/build-native.sh
```

Generated dependencies, build trees, and binaries stay under `out/android` and
are not committed.

## Package and install

From Windows PowerShell in the same checkout:

```powershell
tools\android\package-apk.ps1
adb install -r out\android\MuMain-debug.apk
```

`adb install -r` is important during development because it preserves the
existing account configuration and the large game-data directory. A first-time
debug install needs `Data/` and `config.ini` copied into the application's
internal `files` directory before launch:

```bash
tar -C src/bin -cf - Data config.ini | \
  adb shell run-as com.alin.mumain sh -c 'cd files && tar -xf -'
```

The APK logs `Renderer backend = OpenGL` to `MuError.log`. Android logcat also
identifies GL4ES and the physical GPU. Desktop and Linux builds continue to use
OpenGL.

## Fold4 performance baseline

Use the Galaxy Z Fold4 unfolded in landscape at native render scale `1.0`.
Select production character `alin1033` (Dark Wizard, level 215), connect at the
saved Lorencia location `(187, 124)`, and leave the camera and character
untouched after the map settles. Start a 120-second capture from Windows:

```powershell
tools\android\capture-performance.ps1 -DurationSeconds 120
```

The 2026-08-25 GL4ES baseline in this scene sustained about 1.3 FPS with
752-803 ms frames. `RenderJoints` alone measured 392-446 ms. A diagnostic build
with joints disabled averaged 7.7 FPS and 127 ms frames, which proved that the
joint/tail path is dominant but also that removing one pass cannot reach the
16.7 ms target. A clean-build recapture on the same date successfully traversed
login, character select, and the production connection; ten settled in-game
samples averaged 1.93 FPS (1.2-2.5 FPS) with 419-474 ms average render sections.

Every renderer comparison must record the APK hash, backend, render scale,
device orientation, per-pass CPU time, draw count, uploaded bytes, total frame
time, and the first 120 seconds after warm-up. The Vulkan release gate is
sustained 60 FPS with p95 frame time at or below 16.7 ms; loading and network
transitions are excluded.
