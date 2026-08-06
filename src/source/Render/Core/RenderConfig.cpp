#include "stdafx.h"
#include "RenderConfig.h"
#include "Data/GameConfig/GameConfigConstants.h"
#include "Core/Platform/WinCompat.h"
#include "Core/Platform/WinIni.h"

bool g_CoreProfile = false;

// -1.0f = alpha test disabled; matches AlphaTestEnable's initial false state in ZzzOpenglUtil.cpp.
float g_AlphaRef = -1.0f;
// Matches BeginOpengl()'s default glAlphaFunc(GL_GREATER, 0.25f).
float g_AlphaFuncRef = 0.25f;

// Matches GL_CURRENT_COLOR's own default (opaque white).
float g_CurrentColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

// See RenderConfig.h -- matches EnableVSync()'s unconditional call at boot.
bool g_VSyncEnabled = true;

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
