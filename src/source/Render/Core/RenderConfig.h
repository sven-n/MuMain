#pragma once

// DXP-08: Core Profile GL context flip, read once at startup (Winmain creates the
// GL context immediately after InitRenderConfig() runs, before anything else can
// observe this). true = core 3.3 (default since DXP-08 Stage G), false = compatibility
// profile (rollback path). Set CoreProfile=0 in config.ini [graphics.opengl] to opt back
// into compatibility.
extern bool g_CoreProfile;

// DXP-13: render backend, read once at startup (same timing guarantee as g_CoreProfile --
// Winmain branches window/context creation on this before either backend's device exists).
// Only GL is soaked and default; D3D11 is the DXP-13+ skeleton (device/swapchain + clear/
// present only, no game rendering yet -- see dxp/DXP-13-d3d11-device-swapchain.md).
enum class RenderBackend { GL, D3D11 };
extern RenderBackend g_RenderBackend;

// D3D11 debug device (D3D11_CREATE_DEVICE_DEBUG) opt-in, read once at startup like the flags
// above -- RHI_D3D11.cpp's Init() branches device-creation flags on this before the device
// exists. Off by default (config.ini [graphics.directx] D3D11DebugLayer=1 to opt in): the validation
// layer's per-Draw-call cost stacks with Debug's unoptimized code, making draw-call-heavy
// scenes (e.g. the login screen's ~14K terrain tile draws/frame) unplayably slow in ordinary
// Debug sessions. Turn it on only when actively chasing a D3D11 correctness bug.
extern bool g_D3D11DebugLayerEnabled;

// DXP-21 part 1 phase 1: ClothComputeShader one-shot GPU-vs-CPU self-test opt-in (config.ini
// [graphics.directx] ClothComputeSelfTest=1). Off by default -- diagnostic only, checked once at startup
// under D3D11 to log a pass/fail delta to MuError.log; not part of the real per-frame cloth path.
extern bool g_ClothComputeSelfTestEnabled;

// DXP-21 part 1 phase 3c: opt-in GPU simulation+draw for CPhysicsCloth::Move()/Render() (config.ini
// [graphics.directx] GpuCloth=1). Off by default -- CPU stays the real path for every cloth instance
// (capes/wings/hair) until this soaks; toggling it flips ALL CPhysicsCloth instances at once
// (CPhysicsClothMesh, the one mesh-topology exception, is unaffected -- always CPU, see the
// DXP-21 task memory).
extern bool g_GpuClothEnabled;

// GLP-08: optional ceiling on the requested core-profile GL context version, read from
// config.ini [graphics.opengl] MaxGLVersion (e.g. "4.3"). 0 = no cap (try the highest core context
// available -- Winmain.cpp's descending {4,5}->{4,3}->{3,3} attempt loop starts at the top).
// Rollback path for a driver that mishandles that loop. Ignored when g_CoreProfile is false.
extern int g_MaxGLVersionMajor;
extern int g_MaxGLVersionMinor;

// Fixed-function alpha test (GL_ALPHA_TEST/glAlphaFunc) state, mirrored for shader-side
// `discard` (DXP-01). g_AlphaRef is what shaders should test against: -1.0f means alpha
// test is currently disabled (no discard), otherwise it's the active GL_GREATER threshold.
// g_AlphaFuncRef is the last threshold passed to SetAlphaFuncRef(), independent of whether
// alpha test is currently enabled — reapplied to g_AlphaRef the next time it's enabled.
extern float g_AlphaRef;
extern float g_AlphaFuncRef;

// Cross-backend vsync flag. GL's actual swap interval lives with SDL (ZzzOpenglUtil.cpp's
// EnableVSync/DisableVSync call SDL_GL_SetSwapInterval); this mirror is what RHI_D3D11's
// Present() reads, since D3D11 has no GL context/SDL swap-interval to toggle -- its swap
// chain's own sync-interval param is the only lever. Default true matches EnableVSync()'s
// unconditional call at boot (Winmain.cpp, both backends).
extern bool g_VSyncEnabled;

// DXP-08a Category 1: CPU-tracked mirror of the FFP "current color" any glColor* call used
// to set (RGBA, 0-1 range). Under Core Profile glColor* no-ops and glGetFloatv(GL_CURRENT_
// COLOR, ...) is itself an illegal call, so callers that used to rely on reading back the
// ambient FFP color state now write here instead. Default {1,1,1,1} matches GL's own default.
extern float g_CurrentColor[4];

void InitRenderConfig();

// DXP-17: shared closed-form perspective projection builder, branching on g_RenderBackend for
// the clip-space z convention -- GL's post-divide NDC z is [-1,1], D3D's is [0,1]. Every 3D
// camera/item-preview projection in the codebase used the GL-only formula for both backends,
// which clips/depth-tests roughly half the frustum against the wrong range under D3D11 (visible
// as underwater/under-surface geometry bleeding through from above, z-fighting, or near-plane
// objects vanishing). `f` is the caller's precomputed `1/tan(fovY/2)`; `out` is row-major matching
// this codebase's existing GlobalUBO layout (same layout the old glGetFloatv-derived matrices used).
void BuildPerspectiveProjection(float f, float aspect, float zNear, float zFar, float out[16]);
