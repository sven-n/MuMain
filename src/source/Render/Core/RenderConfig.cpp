#include "stdafx.h"
#include "RenderConfig.h"
#include "Data/GameConfig/GameConfigConstants.h"
#include "Core/Platform/WinCompat.h"
#include "Core/Platform/WinIni.h"
#include <cwchar>

bool g_CoreProfile = false;

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

// See RenderConfig.h -- opt-in until the reordering has been confirmed by eye.
bool g_SortParticleDraws = false;

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

    int coreProfile = GetPrivateProfileIntW(CfgSections::CfgSectionRender, CfgKeys::CfgKeyCoreProfile, CfgDefaults::CfgDefaultCoreProfile ? 1 : 0, configPath);
    g_CoreProfile = (coreProfile != 0);

    // GLP-08: "major.minor" (e.g. "4.3"), or empty/unparseable = no cap.
    wchar_t maxGLVersion[16] = {};
    GetPrivateProfileStringW(CfgSections::CfgSectionRender, CfgKeys::CfgKeyMaxGLVersion,
        CfgDefaults::CfgDefaultMaxGLVersion, maxGLVersion, (DWORD)(sizeof(maxGLVersion) / sizeof(maxGLVersion[0])), configPath);
    int maxMajor = 0, maxMinor = 0;
    if (swscanf(maxGLVersion, L"%d.%d", &maxMajor, &maxMinor) == 2 && maxMajor > 0)
    {
        g_MaxGLVersionMajor = maxMajor;
        g_MaxGLVersionMinor = maxMinor;
    }

    int sortParticleDraws = GetPrivateProfileIntW(CfgSections::CfgSectionRender, CfgKeys::CfgKeySortParticleDraws,
        CfgDefaults::CfgDefaultSortParticleDraws ? 1 : 0, configPath);
    g_SortParticleDraws = (sortParticleDraws != 0);
}

void BuildPerspectiveProjection(float f, float aspect, float zNear, float zFar, float out[16])
{
    out[0] = f / aspect; out[1] = 0.f; out[2] = 0.f; out[3] = 0.f;
    out[4] = 0.f;        out[5] = f;   out[6] = 0.f; out[7] = 0.f;
    out[8] = 0.f; out[9] = 0.f; out[11] = -1.f;
    out[12] = 0.f; out[13] = 0.f; out[15] = 0.f;

    out[10] = (zFar + zNear) / (zNear - zFar);
    out[14] = (2.f * zFar * zNear) / (zNear - zFar);
}
