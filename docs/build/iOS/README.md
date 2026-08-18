# iOS (planned)

iOS is not yet a supported build target. CMake fatals out on
`CMAKE_SYSTEM_NAME STREQUAL "iOS"`: the engine still uses desktop OpenGL, and
distribution is App Store restricted.

macOS is a separate desktop target (see [macos/console.md](../macos/console.md)).
Do not reuse the macOS `main.cpp` entry for iOS; iOS needs its own
`source/App/Platform/iOS/` bootstrap, an OpenGL ES or Metal renderer, and a
touch UI.

This folder is a placeholder so the build docs can grow per-tool guides (Xcode,
command line) once that work starts. See [../README.md](../README.md) for the
platforms that work today (Linux, Windows, macOS).
