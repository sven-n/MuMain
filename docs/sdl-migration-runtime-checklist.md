# SDL Migration Runtime Checklist

## Build Matrix

- [x] macOS arm64 debug: `cmake --build out/build/macos-arm64 --config Debug --target Main`
- [ ] Linux debug preset or documented local Linux command
- [ ] Windows MSVC debug preset
- [ ] Windows MinGW preset, if still supported

## Runtime Scenes

- [x] Launch app and exit app without macOS crash reporter (three consecutive macOS arm64 exits).
- [ ] Login scene renders background, logo, text, and buttons.
- [ ] Character select renders characters, wings, glow, text, and click targets.
- [ ] Main world renders terrain, sky, water, entities, particles, and UI.
- [ ] Item glow renders correctly for inventory, world drops, shop, and character equipment.
- [ ] Ground item labels render and update without severe FPS collapse.
- [ ] Text input, IME candidate positioning, backspace/delete, paste, and focus transitions work.
- [ ] Music and sound effects start, stop, and do not crash on shutdown.
- [ ] Windowed/fullscreen transitions keep viewport and clicks aligned.

## Baseline Audit

- [x] `MuRendererSDLGpu.cpp` is included in `MuClient`.
- [x] `MuRenderer.cpp` is excluded from `MuClient`.
- [x] `Winmain.cpp` calls `InitSDLGpuRenderer`, `BeginFrame`, and `EndFrame`.
- [x] No active app lifecycle code uses `SDL_WINDOW_OPENGL` or `SDL_GL_CreateContext`.
- [ ] Windows normal runtime link blocks still include `opengl32` and `glu32`; Task 6 owns this cleanup.

## Known Gaps

- Item glow not yet fixed.
- Windows OpenGL link path still exists in CMake.
- Real OpenGL headers are still included before the compatibility shim.
- Some GL-shaped compatibility calls are no-op and need classification.
