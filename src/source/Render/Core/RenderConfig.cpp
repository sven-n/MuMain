#include "stdafx.h"
#include "RenderConfig.h"
#include "Data/GameConfig/GameConfigConstants.h"
#include "Core/Platform/WinCompat.h"
#include "Core/Platform/WinIni.h"
#include <cwchar>

bool g_CoreProfile = false;
RenderBackend g_RenderBackend = RenderBackend::GL;
bool g_D3D11DebugLayerEnabled = false;
bool g_ClothComputeSelfTestEnabled = false;
bool g_GpuClothEnabled = false;

// See RenderConfig.h -- 0 = no cap.
int g_MaxGLVersionMajor = 0;
int g_MaxGLVersionMinor = 0;

// -1.0f = alpha test disabled; matches AlphaTestEnable's initial false state in ZzzOpenglUtil.cpp.
float g_AlphaRef = -1.0f;
// Matches BeginOpengl()'s default glAlphaFunc(GL_GREATER, 0.25f).
float g_AlphaFuncRef = 0.25f;

// Matches GL_CURRENT_COLOR's own default (opaque white).
float g_CurrentColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

// See RenderConfig.h -- matches EnableVSync()'s unconditional call at boot.
bool g_VSyncEnabled = true;

namespace
{
    // The private-profile API cannot distinguish "key absent" from "key present but empty"
    // through a single call -- both return the supplied default. Probing with a sentinel that
    // can never be a real config value is the standard way to tell them apart, and the
    // migration below needs that distinction: a legacy key the user never set must not be
    // copied forward as an explicit value.
    bool ProfileKeyExists(const wchar_t* section, const wchar_t* key, const wchar_t* path)
    {
        wchar_t probe[8] = {};
        GetPrivateProfileStringW(section, key, L"\x01", probe, _countof(probe), path);
        return probe[0] != L'\x01';
    }

    // Copies one key from the legacy [Render] section into its new per-backend home, but only
    // if it was actually present -- an absent key stays absent so it keeps picking up its
    // compiled-in default rather than being frozen at whatever that default happens to be today.
    void CarryLegacyKey(const wchar_t* key, const wchar_t* newSection, const wchar_t* path)
    {
        if (!ProfileKeyExists(CfgSections::CfgSectionRenderLegacy, key, path)) return;
        wchar_t value[64] = {};
        GetPrivateProfileStringW(CfgSections::CfgSectionRenderLegacy, key, L"", value, _countof(value), path);
        WritePrivateProfileStringW(newSection, key, value, path);
    }

    // One-time migration of the flat [Render] section to [graphics] + [graphics.<renderer>].
    // Runs before any read below, so the rest of InitRenderConfig() only ever sees the new
    // schema and needs no fallback path of its own. A no-op once [Render] is gone, which is
    // the case from the second launch onward.
    void MigrateLegacyRenderSection(const wchar_t* path)
    {
        using namespace CfgSections;
        using namespace CfgKeys;

        // Presence of the whole legacy section is keyed off Backend specifically: it is the
        // one key the old schema always wrote, and the only one whose meaning moved rather
        // than merely relocating.
        if (!ProfileKeyExists(CfgSectionRenderLegacy, CfgKeyBackendLegacy, path)) return;

        // Match the legacy parse exactly ("D3D11" case-insensitively, everything else GL) so a
        // migrated file selects the same backend it selected before the migration.
        wchar_t legacyBackend[16] = {};
        GetPrivateProfileStringW(CfgSectionRenderLegacy, CfgKeyBackendLegacy, L"GL",
                                 legacyBackend, _countof(legacyBackend), path);
        const bool wasD3D11 = (_wcsicmp(legacyBackend, L"D3D11") == 0);
        WritePrivateProfileStringW(CfgSectionGraphics, CfgKeyRenderer,
                                   wasD3D11 ? CfgRendererValues::DirectX : CfgRendererValues::OpenGL, path);

        CarryLegacyKey(CfgKeyCoreProfile,         CfgSectionGraphicsOpenGL,  path);
        CarryLegacyKey(CfgKeyMaxGLVersion,        CfgSectionGraphicsOpenGL,  path);
        CarryLegacyKey(CfgKeyD3D11DebugLayer,     CfgSectionGraphicsDirectX, path);
        CarryLegacyKey(CfgKeyClothComputeSelfTest, CfgSectionGraphicsDirectX, path);
        CarryLegacyKey(CfgKeyGpuCloth,            CfgSectionGraphicsDirectX, path);

        // Null key name deletes the whole section -- same contract WinIni.cpp's portable shim
        // implements, and what GameConfig::RemoveObsoleteSection relies on.
        WritePrivateProfileStringW(CfgSectionRenderLegacy, nullptr, nullptr, path);
    }

    RenderBackend ParseRenderer(const wchar_t* value)
    {
        if (_wcsicmp(value, CfgRendererValues::DirectX)      == 0 ||
            _wcsicmp(value, CfgRendererValues::DirectXAltD3D) == 0 ||
            _wcsicmp(value, CfgRendererValues::DirectXAltDX)  == 0)
        {
            return RenderBackend::D3D11;
        }
        // Includes the explicit OpenGL spellings and every unrecognized value -- see
        // CfgDefaultRenderer's comment for why the fallback direction matters.
        return RenderBackend::GL;
    }
}

void InitRenderConfig()
{
    wchar_t configPath[MAX_PATH];
    GetModuleFileNameW(nullptr, configPath, MAX_PATH);
    wchar_t* lastBackslash = wcsrchr(configPath, L'\\');
    wchar_t* lastForwardSlash = wcsrchr(configPath, L'/');
    wchar_t* lastSlash = nullptr;
    if (lastBackslash && lastForwardSlash)
        lastSlash = (lastBackslash > lastForwardSlash) ? lastBackslash : lastForwardSlash;
    else
        lastSlash = lastBackslash ? lastBackslash : lastForwardSlash;

    if (lastSlash)
    {
        *(lastSlash + 1) = L'\0';
    }

    wcscat_s(configPath, MAX_PATH, L"config.ini");

    MigrateLegacyRenderSection(configPath);

    // ---- [graphics] ----
    wchar_t renderer[16] = {};
    GetPrivateProfileStringW(CfgSections::CfgSectionGraphics, CfgKeys::CfgKeyRenderer,
        CfgDefaults::CfgDefaultRenderer, renderer, _countof(renderer), configPath);
    g_RenderBackend = ParseRenderer(renderer);

    // ---- [graphics.opengl] ----
    // Both backends' settings are read unconditionally regardless of which renderer won above.
    // The inactive set simply goes unconsulted, and g_CoreProfile in particular is still read
    // by backend-shared code paths (the `if (!g_CoreProfile)` FFP guards in ZzzOpenglUtil.cpp),
    // so it must hold its configured value even under DirectX.
    const int coreProfile = GetPrivateProfileIntW(CfgSections::CfgSectionGraphicsOpenGL, CfgKeys::CfgKeyCoreProfile,
        CfgDefaults::CfgDefaultCoreProfile ? 1 : 0, configPath);
    g_CoreProfile = (coreProfile != 0);

    // GLP-08: "major.minor" (e.g. "4.3"), or empty/unparseable = no cap.
    wchar_t maxGLVersion[16] = {};
    GetPrivateProfileStringW(CfgSections::CfgSectionGraphicsOpenGL, CfgKeys::CfgKeyMaxGLVersion,
        CfgDefaults::CfgDefaultMaxGLVersion, maxGLVersion, _countof(maxGLVersion), configPath);
    int maxMajor = 0, maxMinor = 0;
    if (swscanf(maxGLVersion, L"%d.%d", &maxMajor, &maxMinor) == 2 && maxMajor > 0)
    {
        g_MaxGLVersionMajor = maxMajor;
        g_MaxGLVersionMinor = maxMinor;
    }

    // ---- [graphics.directx] ----
    const int d3d11DebugLayer = GetPrivateProfileIntW(CfgSections::CfgSectionGraphicsDirectX, CfgKeys::CfgKeyD3D11DebugLayer,
        CfgDefaults::CfgDefaultD3D11DebugLayer ? 1 : 0, configPath);
    g_D3D11DebugLayerEnabled = (d3d11DebugLayer != 0);

    const int clothComputeSelfTest = GetPrivateProfileIntW(CfgSections::CfgSectionGraphicsDirectX, CfgKeys::CfgKeyClothComputeSelfTest,
        CfgDefaults::CfgDefaultClothComputeSelfTest ? 1 : 0, configPath);
    g_ClothComputeSelfTestEnabled = (clothComputeSelfTest != 0);

    const int gpuCloth = GetPrivateProfileIntW(CfgSections::CfgSectionGraphicsDirectX, CfgKeys::CfgKeyGpuCloth,
        CfgDefaults::CfgDefaultGpuCloth ? 1 : 0, configPath);
    g_GpuClothEnabled = (gpuCloth != 0);
}

void BuildPerspectiveProjection(float f, float aspect, float zNear, float zFar, float out[16])
{
    out[0] = f / aspect; out[1] = 0.f; out[2] = 0.f; out[3] = 0.f;
    out[4] = 0.f;        out[5] = f;   out[6] = 0.f; out[7] = 0.f;
    out[8] = 0.f; out[9] = 0.f; out[11] = -1.f;
    out[12] = 0.f; out[13] = 0.f; out[15] = 0.f;

    if (g_RenderBackend == RenderBackend::D3D11)
    {
        out[10] = zFar / (zNear - zFar);
        out[14] = (zNear * zFar) / (zNear - zFar);
    }
    else
    {
        out[10] = (zFar + zNear) / (zNear - zFar);
        out[14] = (2.f * zFar * zNear) / (zNear - zFar);
    }
}
