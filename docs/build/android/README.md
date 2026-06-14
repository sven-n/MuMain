# Android (planned)

Android is not yet a supported build target. The platform entry seam exists
(`src/source/App/Platform/`), but there is no `Android/` entry point, no NDK
toolchain wiring, and the renderer/input layers are not yet validated on
Android.

This folder is a placeholder so the build docs can grow per-tool guides
(Android Studio, Gradle/NDK, command line) once the port begins. See
[../README.md](../README.md) for the platforms that work today (Linux, Windows).
