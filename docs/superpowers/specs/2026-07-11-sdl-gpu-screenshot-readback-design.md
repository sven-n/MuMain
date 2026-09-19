# SDL GPU Screenshot Readback Design

## Goal

Restore screenshots on the SDL GPU renderer without adding work to ordinary
frames or exposing SDL GPU objects outside the renderer layer.

## Scope

- Capture the rendered color swapchain after the render pass.
- Preserve the existing JPEG filename, quality, and user notification flow.
- Support one pending screenshot request at a time.
- Leave depth-buffer sampling out of this stage. The existing sun-visibility
  fallback remains until a separate asynchronous depth design is implemented.

## Renderer Contract

`IMuRenderer` will expose an owned, asynchronous color-readback contract:

- `RequestFramePixels()` records a one-shot request and returns whether it was
  accepted.
- `ConsumeFramePixels(FramePixels&)` moves a completed RGB image to the caller.
- `FramePixels` contains physical width, height, and tightly packed top-down RGB
  bytes. Callers never own GPU transfer buffers or fences.

Only one request may be pending or completed. A second request is rejected until
the first result is consumed, preventing unbounded memory or GPU work.

## Frame Lifecycle

1. `SceneManager` handles Print Screen, records the filename/message, and asks
   the renderer for a capture.
2. `EndFrame()` finishes drawing, creates a download transfer buffer only when a
   request is pending, and encodes `SDL_DownloadFromGPUTexture` after the render
   pass.
3. That command buffer is submitted with a fence. The renderer waits for this
   one-shot fence, maps the transfer buffer, converts the swapchain format to
   top-down RGB, stores the owned result, and releases the fence and transfer
   buffer.
4. On the following scene update, `SceneManager` consumes the result and calls
   `WriteJpeg`. The success message is emitted only after the image is available.

Ordinary frames continue through `SDL_SubmitGPUCommandBuffer` and allocate no
readback resources.

## Format Handling

The implementation will explicitly support the SDL GPU swapchain formats used
by the current backends: RGBA8 and BGRA8, including their sRGB variants. Unknown
formats fail the request with a renderer warning instead of writing corrupted
pixels. Row orientation is normalized inside the renderer so JPEG code receives
top-down RGB consistently on every backend.

## Failure Handling

- Device, swapchain, allocation, copy-pass, fence, wait, or map failures clear
  the pending request and release all partially created resources.
- `SceneManager` receives no completed image and does not report a successful
  screenshot.
- Shutdown releases any pending fence or transfer buffer before destroying the
  GPU device.

## Verification

- Unit-test pixel conversion for RGBA8, BGRA8, row orientation, and unsupported
  formats.
- Unit-test the one-pending-request state contract independently of SDL.
- Build Debug and Release `Main`.
- Capture a screenshot on macOS and verify dimensions, orientation, channel
  order, and JPEG output.
- Confirm Release rendering remains at the 60 Hz frame budget when no screenshot
  is requested.
- Close the macOS window and verify exit status 0 after a capture.
