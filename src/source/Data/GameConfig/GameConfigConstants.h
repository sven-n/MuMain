#pragma once

namespace CfgSections
{
    inline constexpr wchar_t CfgSectionWindow[]     = L"Window";
    inline constexpr wchar_t CfgSectionAudio[]      = L"Audio";
    inline constexpr wchar_t CfgSectionUI[]         = L"UI";
    inline constexpr wchar_t CfgSectionLogin[]      = L"LOGIN";
    inline constexpr wchar_t CfgSectionConnectionSettings[] = L"CONNECTION SETTINGS";
    inline constexpr wchar_t CfgSectionCamera[] = L"Camera";

    // Renderer selection lives in [graphics]; each backend's own settings live in a
    // dedicated [graphics.<renderer>] subsection, so a key can never be ambiguous about
    // which backend it applies to. Section names are matched case-insensitively by the
    // private-profile API (and by WinIni.cpp's portable shim), and a '.' is an ordinary
    // character in a section name -- no escaping needed.
    inline constexpr wchar_t CfgSectionGraphics[]        = L"graphics";
    inline constexpr wchar_t CfgSectionGraphicsOpenGL[]  = L"graphics.opengl";
    inline constexpr wchar_t CfgSectionGraphicsDirectX[] = L"graphics.directx";

    // Superseded by the [graphics*] sections above. Read exactly once, by
    // InitRenderConfig()'s one-time migration, then deleted -- see MigrateLegacyRenderSection().
    inline constexpr wchar_t CfgSectionRenderLegacy[] = L"Render";
}

namespace CfgKeys
{
    // Window
    inline constexpr wchar_t CfgKeyWidth[]      = L"Width";
    inline constexpr wchar_t CfgKeyHeight[]     = L"Height";
    inline constexpr wchar_t CfgKeyWindowed[]   = L"Windowed";

    // Audio — volume 0 = off, >0 = on (no separate Enabled flag).
    inline constexpr wchar_t CfgKeySoundVolume[]  = L"SoundVolume";
    inline constexpr wchar_t CfgKeyMusicVolume[] = L"MusicVolume";

    // Login
    inline constexpr wchar_t CfgKeyRememberMe[]        = L"RememberMe";
    inline constexpr wchar_t CfgKeySavePassword[]      = L"SavePassword";
    inline constexpr wchar_t CfgKeyLanguage[]          = L"Language";
    inline constexpr wchar_t CfgKeyEncryptedUsername[] = L"EncryptedUsername";
    inline constexpr wchar_t CfgKeyEncryptedPassword[] = L"EncryptedPassword";

    // Connection
    inline constexpr wchar_t CfgKeyServerIP[]   = L"ServerIP";
    inline constexpr wchar_t CfgKeyServerPort[] = L"ServerPort";

    // UI
    inline constexpr wchar_t CfgKeyUILocale[] = L"Locale";
    inline constexpr wchar_t CfgKeyFont[]     = L"Font";

    // Camera
    inline constexpr wchar_t CfgKeyZoom[] = L"Zoom";

    // [graphics] -- renderer selection. See CfgRendererValues below for accepted values.
    inline constexpr wchar_t CfgKeyRenderer[] = L"renderer";

    // ---- [graphics.opengl] ----
    // DXP-08: Core Profile GL context flip. 0 = compatibility (rollback), 1 = core.
    inline constexpr wchar_t CfgKeyCoreProfile[] = L"CoreProfile";

    // GLP-08: ceiling on the requested core-profile GL context version, e.g. "4.3". Empty
    // (default) tries the highest of {4.5, 4.3, 3.3} the driver will grant. Rollback path for a
    // driver that mishandles the descending attempt loop.
    inline constexpr wchar_t CfgKeyMaxGLVersion[] = L"MaxGLVersion";

    // ---- [graphics.directx] ----
    // D3D11 debug device (D3D11_CREATE_DEVICE_DEBUG) opt-in. Off by default even in Debug
    // builds -- the validation layer's per-Draw-call cost, stacked on Debug's unoptimized
    // code, turns draw-call-heavy scenes (e.g. the login screen's ~14K terrain tile draws)
    // unplayably slow. Flip to 1 in config.ini only when actively investigating a D3D11
    // correctness bug (it's what catches missing state binds, resource mismatches, etc.).
    inline constexpr wchar_t CfgKeyD3D11DebugLayer[] = L"D3D11DebugLayer";

    // DXP-21 part 1 phase 1: one-shot ClothComputeShader GPU-vs-CPU self-test, run once at
    // startup (D3D11 only) and logged to MuError.log. Off by default -- diagnostic only, not
    // part of the real cloth path yet (Passes A/B aren't wired to any render-loop caller).
    inline constexpr wchar_t CfgKeyClothComputeSelfTest[] = L"ClothComputeSelfTest";

    // DXP-21 part 1 phase 3c: opt-in GPU cloth simulation+draw for CPhysicsCloth::Move()/Render()
    // (D3D11 only; CPhysicsClothMesh -- the one mesh-topology exception -- still always uses CPU,
    // see the DXP-21 task memory's Phase 0 scope note). Off by default -- CPU remains the default
    // simulation/draw path for every cloth instance until this soaks (Phase 4/5).
    inline constexpr wchar_t CfgKeyGpuCloth[] = L"GpuCloth";

    // Superseded by CfgKeyRenderer. Read only by InitRenderConfig()'s one-time migration.
    inline constexpr wchar_t CfgKeyBackendLegacy[] = L"Backend";
}

// Accepted values for [graphics] renderer. Compared case-insensitively; anything not listed
// here selects OpenGL, so a typo can never silently hand the frame to the other backend.
namespace CfgRendererValues
{
    inline constexpr wchar_t OpenGL[]  = L"opengl";
    inline constexpr wchar_t DirectX[] = L"directx";

    // Tolerated spellings, so a hand-edited config and the pre-[graphics] value both work.
    // "gl"/"d3d11" are what the legacy [Render] Backend key used.
    inline constexpr wchar_t OpenGLAlt[]     = L"gl";
    inline constexpr wchar_t DirectXAltD3D[] = L"d3d11";
    inline constexpr wchar_t DirectXAltDX[]  = L"dx11";
}

namespace CfgDefaults
{
    inline constexpr int  CfgDefaultWindowWidth  = 1024;
    inline constexpr int  CfgDefaultWindowHeight = 768;
    inline constexpr bool CfgDefaultWindowed     = true;

    inline constexpr int  CfgDefaultSoundVolume = 5;
    inline constexpr int  CfgDefaultMusicVolume = 5;

    inline constexpr bool CfgDefaultRememberMe = false;
    inline constexpr bool CfgDefaultSavePassword = false;
    inline constexpr wchar_t CfgDefaultLanguage[] = L"Eng";
    inline constexpr wchar_t CfgDefaultEncryptedUsername[] = L"";
    inline constexpr wchar_t CfgDefaultEncryptedPassword[] = L"";

    inline constexpr wchar_t CfgDefaultServerIP[] = L"127.127.127.127";
    inline constexpr int CfgDefaultServerPort = 44406;

    inline constexpr int CfgDefaultZoom = 1735;  // OrbitalCamera DEFAULT_RADIUS — matches Default-cam camera-to-Hero distance

    // I18N locale code; "en" is the default the resx generator falls back to.
    inline constexpr wchar_t CfgDefaultUILocale[] = L"en";

    // UI font family name. Empty = each platform's built-in default (Tahoma on
    // Windows, fontconfig "sans-serif" on Linux), so the look is unchanged until
    // the user picks a font. Any value is passed through as the GDI face name.
    inline constexpr wchar_t CfgDefaultFont[] = L"";

    // DXP-08 Stage G: flipped to default-on after DXP-08a/DXP-09 prerequisites were fixed and
    // soak-confirmed clean under CoreProfile=1 (2026-08-01). Set CoreProfile=0 in
    // [graphics.opengl] to opt back into the compatibility-profile rollback path.
    inline constexpr bool CfgDefaultCoreProfile = true;

    // DXP-13: default renderer. Anything other than a recognized DirectX spelling falls back
    // to OpenGL -- an unrecognized value must never silently select the other backend.
    inline constexpr wchar_t CfgDefaultRenderer[] = L"opengl";

    // Off by default -- see CfgKeyD3D11DebugLayer's comment.
    inline constexpr bool CfgDefaultD3D11DebugLayer = false;

    // Off by default -- see CfgKeyClothComputeSelfTest's comment.
    inline constexpr bool CfgDefaultClothComputeSelfTest = false;

    // Off by default -- see CfgKeyGpuCloth's comment.
    inline constexpr bool CfgDefaultGpuCloth = false;

    // GLP-08: empty = no cap, try the highest core context available.
    inline constexpr wchar_t CfgDefaultMaxGLVersion[] = L"";
}