// GroundTruthCapture.h — Ground Truth Screenshot Capture and SSIM Comparison
// Story 4.1.1 — Flow Code: VS1-RENDER-GROUNDTRUTH-CAPTURE
//
// Provides automated screenshot capture (Windows + OpenGL only, guarded by
// ENABLE_GROUND_TRUTH_CAPTURE) and platform-independent SSIM comparison logic.
//
// SSIM computation (ComputeSSIM) is always compiled — no OpenGL dependency.
// Capture code (CaptureScene, CompareTo, RunUISweep) is only compiled when
// ENABLE_GROUND_TRUTH_CAPTURE is defined.
//
// Usage (capture, Windows only):
//   #ifdef ENABLE_GROUND_TRUTH_CAPTURE
//   mu::GroundTruthCapture::CaptureScene("inventory", w, h);
//   #endif
//
// Usage (comparison, all platforms):
//   double score = mu::GroundTruthCapture::ComputeSSIM(a, b, w, h, ch);
//   if (score < 0.99) { /* rendering diverged */ }
#pragma once

namespace mu
{

/// Ground truth screenshot capture and structural similarity comparison.
///
/// Capture functions are only available when ENABLE_GROUND_TRUTH_CAPTURE is
/// defined. The SSIM comparison function is always compiled.
class GroundTruthCapture
{
public:
    GroundTruthCapture() = delete;

#ifdef ENABLE_GROUND_TRUTH_CAPTURE
    /// Capture the current OpenGL framebuffer and write a PNG to tests/golden/.
    ///
    /// @param sceneName  Short identifier used in the filename (e.g. "inventory").
    /// @param width      Framebuffer width in pixels.
    /// @param height     Framebuffer height in pixels.
    /// @returns true on success, false if glReadPixels or PNG write fails.
    ///          Failure is also logged via g_ErrorReport.
    [[nodiscard]] static bool CaptureScene(const char* sceneName, int width, int height);

    /// Compare a previously captured scene against a new frame.
    ///
    /// Reads the named PNG from tests/golden/, captures the current framebuffer,
    /// and returns the SSIM score. When SSIM < threshold a diff image is written
    /// alongside the sources with a "-diff" suffix.
    ///
    /// @param sceneName   Scene identifier (must match CaptureScene call).
    /// @param width       Framebuffer width in pixels.
    /// @param height      Framebuffer height in pixels.
    /// @param threshold   SSIM threshold (default 0.99).
    /// @returns SSIM score in [0.0, 1.0]; negative on I/O error.
    [[nodiscard]] static double CompareTo(const char* sceneName, int width, int height, double threshold = 0.99);

    /// Iterate over all CNewUI* windows, Show() each, capture, then Hide().
    /// Call from WinMain() or SceneManager after initial load.
    static void RunUISweep(int width, int height);
#endif // ENABLE_GROUND_TRUTH_CAPTURE

    /// Compute the mean SSIM score between two images over 8x8 sliding windows.
    ///
    /// Uses luminance-only comparison (Y = 0.2126R + 0.7152G + 0.0722B per channel),
    /// Gaussian-uniform 8x8 windows, and standard constants C1/(0.01*255)^2,
    /// C2=(0.03*255)^2 per the original SSIM paper.
    ///
    /// This function is always compiled — no OpenGL or platform dependency.
    ///
    /// @param imgA      Pointer to first image pixel data.
    /// @param imgB      Pointer to second image pixel data.
    /// @param width     Image width in pixels.
    /// @param height    Image height in pixels.
    /// @param channels  Bytes per pixel (1=Y, 3=RGB, 4=RGBA).
    /// @returns Mean SSIM in [0.0, 1.0]; 1.0 means perceptually identical.
    [[nodiscard]] static double ComputeSSIM(const unsigned char* imgA, const unsigned char* imgB, int width, int height,
                                            int channels);

private:
#ifdef ENABLE_GROUND_TRUTH_CAPTURE
    /// Flip pixel buffer vertically in-place.
    /// Required because glReadPixels returns bottom-row-first, PNG expects top-row-first.
    static void FlipVertical(unsigned char* buf, int width, int height, int channels);

    /// Compute SHA256 of a buffer and return the hex string (64 chars + NUL).
    /// Used to log a checksum alongside each captured PNG.
    static void ComputeSHA256Hex(const unsigned char* data, int len, char outHex[65]);
#endif // ENABLE_GROUND_TRUTH_CAPTURE
};

} // namespace mu
