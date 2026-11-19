#include "stdafx.h"

#include "World/MapInfra/MapLoad.h"

#include "Core/Utilities/StagedLoad.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Render/Renderer/BitmapUpload.h"
#include "World/MapInfra/MapManager.h"

#include <chrono>
#include <optional>
#include <utility>

namespace World::MapLoad
{
namespace
{
// Load work per frame, as at startup.
constexpr std::chrono::milliseconds StepBudget{33};
// Shorter loads hold the last frame, as a load in one call did; a longer one (a pack still downloading) shows the
// loading screen.
constexpr std::chrono::milliseconds LoadingScreenAfter{150};

struct Load
{
    Load(int map, std::function<void()> afterLoad)
        : steps(gMapManager.LoadWorldSteps(map)), afterLoad(std::move(afterLoad))
    {
    }

    Core::StagedLoad steps;
    std::function<void()> afterLoad;
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    int frames = 0;
};

std::optional<Load> s_load;

void Finish()
{
    std::function<void()> afterLoad = std::move(s_load->afterLoad);
    const int frames = s_load->frames;
    const double ms =
        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - s_load->start).count();
    s_load.reset();
    mu::log::Get("render")->info("Map change loaded across {} frames in {:.0f} ms", frames, ms);
    if (afterLoad)
    {
        afterLoad();
    }
}
} // namespace

void Begin(int map, std::function<void()> afterLoad)
{
    if (s_load)
    {
        while (!s_load->steps.Pump(std::chrono::hours{1}))
        {
        }
        Finish();
    }
    s_load.emplace(map, std::move(afterLoad));
}

bool Active()
{
    return s_load.has_value();
}

void Pump()
{
    if (!s_load)
    {
        return;
    }
    ++s_load->frames;
    if (s_load->steps.Pump(StepBudget, mu::BitmapUploadsOverStagingCap))
    {
        Finish();
    }
}

bool LoadingScreenDue()
{
    return s_load && std::chrono::steady_clock::now() - s_load->start >= LoadingScreenAfter;
}

void Cancel()
{
    s_load.reset();
}
} // namespace World::MapLoad
