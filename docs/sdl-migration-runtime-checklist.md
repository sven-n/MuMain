# SDL Migration Runtime Checklist

## Build Matrix

- [x] macOS arm64 debug: `cmake --build out/build/macos-arm64 --config Debug --target Main`
- [ ] Linux debug preset or documented local Linux command
- [ ] Windows MSVC Debug runtime capture
- [x] Windows MSVC Release build and CTest in hosted CI
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
- [ ] Windows inventory item rendering and hover regions use the same transform.
- [ ] Windows storage item rendering and hover regions use the same transform.
- [ ] Windows cursor alignment passes at 100% and non-100% display scaling.
- [ ] Windows D3D12 `$vsync off` and `$vsync on` transition without a crash.
- [ ] Windows D3D12 VSync follows the intended refresh-rate pacing policy.
- [ ] `$vsync off` survives restart plus windowed/fullscreen and resolution changes.
- [ ] Debug and Release rendering captures identify replay, submit, and frame costs.

## Baseline Audit

- [x] `MuRendererSDLGpu.cpp` is included in `MuClient`.
- [x] `MuRenderer.cpp` is excluded from `MuClient`.
- [x] `Winmain.cpp` calls `InitSDLGpuRenderer`, `BeginFrame`, and `EndFrame`.
- [x] No active app lifecycle code uses `SDL_WINDOW_OPENGL` or `SDL_GL_CreateContext`.
- [ ] Windows normal runtime link blocks still include `opengl32` and `glu32`; Task 6 owns this cleanup.

## Known Gaps

- Item glow not yet fixed.
- Windows inventory/storage hover and cursor coordinates are reported offset.
- Windows D3D12 VSync pacing and runtime mode switching are not verified.
- Windows Debug rendering performance is reported substantially below the old renderer; profiling is pending.
- Linux/Vulkan runtime parity is untested.
- Windows OpenGL link path still exists in CMake.
- Real OpenGL headers are still included before the compatibility shim.
- Some GL-shaped compatibility calls are no-op and need classification.
