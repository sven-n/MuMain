# iOS (planned)

iOS is not yet a supported build target. CMake has an `APPLE` branch that points
at `src/source/App/Platform/macOS/main.mm`, but that entry point does not exist
yet and the renderer/input layers are not validated on Apple platforms.

This folder is a placeholder so the build docs can grow per-tool guides (Xcode,
command line) once the port begins. See [../README.md](../README.md) for the
platforms that work today (Linux, Windows).
