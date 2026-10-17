#include "stdafx.h"

#include "BenchEnvironment.h"

#include "Core/Utilities/BuildInfo.h"
#include "Core/Utilities/PlatformInfo.h"
#include "Data/GameConfig/GameConfig.h"
#include "Render/Look/LookPresetSetting.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Scenes/SceneManager.h"
#include "World/MapInfra/MapManager.h"

// The control-socket build records the commit it was built from.
#if __has_include("MuGitCommit.h")
#include "MuGitCommit.h"
#endif

#include <ctime>
#include <string>
#include <thread>

#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
#include <cpuid.h>
#endif

namespace Core::Benchmark::Environment
{
namespace
{
std::string Narrow(const std::wstring& text)
{
    std::string out;
    out.reserve(text.size());
    // Diagnostic strings only -- map names and OS strings are ASCII in practice, and a
    // mangled non-ASCII character in a report header is better than a dependency on a
    // codepage conversion here.
    for (wchar_t c : text)
        out += (c > 0 && c < 128) ? (char)c : '?';
    return out;
}

std::string UtcTimestamp()
{
    const std::time_t now = std::time(nullptr);
    std::tm utc = {};
#if defined(_WIN32)
    gmtime_s(&utc, &now);
#else
    gmtime_r(&now, &utc);
#endif
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &utc);
    return buffer;
}

// CPUID leaves 0x80000002..0x80000004 hold the processor brand string on every x86 part that
// reports it. Empty elsewhere -- an unknown CPU is more useful in a report than a wrong one.
std::string CpuBrand()
{
#if defined(_MSC_VER) || (defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)))
    int registers[4] = {};
    char brand[49] = {};
    for (int leaf = 0; leaf < 3; leaf++)
    {
#if defined(_MSC_VER)
        __cpuid(registers, (int)(0x80000002u + leaf));
#else
        __get_cpuid(0x80000002u + leaf, (unsigned*)&registers[0], (unsigned*)&registers[1], (unsigned*)&registers[2],
                    (unsigned*)&registers[3]);
#endif
        memcpy(brand + leaf * sizeof(registers), registers, sizeof(registers));
    }

    std::string result = brand;
    // The brand string is space-padded to a fixed width on many parts.
    while (!result.empty() && result.back() == ' ')
        result.pop_back();
    return result;
#else
    return std::string();
#endif
}

uint64_t SystemMemoryMB()
{
#if defined(_WIN32)
    MEMORYSTATUSEX status = {};
    status.dwLength = sizeof(status);
    if (!GlobalMemoryStatusEx(&status))
        return 0;

    constexpr uint64_t kBytesPerMB = 1024ull * 1024ull;
    return status.ullTotalPhys / kBytesPerMB;
#else
    return 0;
#endif
}

void CaptureBuild(EnvironmentInfo& info)
{
    info.buildConfig = Core::Build::kConfiguration;
    info.buildEditor = Core::Build::kEditor;
    info.buildCompiler = Core::Build::kCompiler;
    info.buildArch = Core::Build::kArchitecture;
    info.buildTimestamp = std::string(Core::Build::kDate) + " " + Core::Build::kTime;
#ifdef MU_GIT_COMMIT
    info.gitDescribe = MU_GIT_COMMIT;
    info.gitDirty = MU_GIT_COMMIT_CHANGED != 0;
#else
    info.gitDescribe = Core::Build::kGitDescribe;
    info.gitDirty = Core::Build::kGitDirty;
#endif
}

void CaptureGraphics(EnvironmentInfo& info)
{
    info.gpuBackend = mu::GetRenderer().GetGPUDriverName();
    info.gpuDevice = mu::GetRenderer().GetGPUDeviceName();

    const bool enhanced = Render::Look::ActiveLookPreset() == LookPreset::Enhanced;
    info.look = enhanced ? "Enhanced" : "Classic";
    if (enhanced)
    {
        using Render::Look::EnhancedFeature;
        const auto add = [&info](EnhancedFeature feature, const char* name)
        {
            if (!Render::Look::EnhancedFeatureChosen(feature))
                return;
            if (!info.enhancedFeatures.empty())
                info.enhancedFeatures += ' ';
            info.enhancedFeatures += name;
        };
        add(EnhancedFeature::Lighting, "lighting");
        add(EnhancedFeature::DynamicLights, "dynlights");
        add(EnhancedFeature::LightMood, "mood");
        add(EnhancedFeature::PostProcessing, "post");
    }
    // Samples per pixel the frames render with; 1 is off.
    const int samples = mu::GetRenderer().GetAntialiasingSamples();
    info.antialiasingSamples = samples > 1 ? samples : 0;
    info.anisotropy = Bitmaps.RequestedAnisotropy();
}

void CaptureSession(EnvironmentInfo& info)
{
    info.windowWidth = (int)WindowWidth;
    info.windowHeight = (int)WindowHeight;
    // The player's setting, and what the run measures under: the recorder switches vsync off
    // for the run.
    info.vsyncRequested = GameConfig::GetInstance().GetVSyncEnabled();
    info.vsyncEffective = IsVSyncEnabled();
    info.targetFps = GetTargetFps();

    const wchar_t* mapName = gMapManager.GetMapName(gMapManager.WorldActive);
    info.mapName = mapName ? Narrow(mapName) : std::string();
}
} // namespace

EnvironmentInfo Capture()
{
    EnvironmentInfo info;
    info.timestampUtc = UtcTimestamp();
    info.osVersion = Narrow(Core::Platform::GetOSVersionString());
    info.cpu = CpuBrand();
    info.cpuThreads = (int)std::thread::hardware_concurrency();
    info.systemMemoryMB = SystemMemoryMB();

    CaptureBuild(info);
    CaptureGraphics(info);
    CaptureSession(info);
    return info;
}
} // namespace Core::Benchmark::Environment
