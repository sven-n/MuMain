#include "stdafx.h"

#include "World/MapInfra/MapLoad.h"

#include "Core/Content/ContentStore.h"
#include "Core/Utilities/StagedLoad.h"
#include "Core/Utilities/Log/MuLogger.h"

#include <chrono>
#include <optional>
#include <string>
#include <utility>
#include <vector>

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
    Load(std::string pack, std::vector<Core::StagedLoad::Step> steps, std::function<void()> afterLoad)
        : pack(std::move(pack)), steps(std::move(steps)), afterLoad(std::move(afterLoad))
    {
    }

    // Emptied once the pack is ready.
    std::string pack;
    Core::StagedLoad steps;
    std::function<void()> afterLoad;
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    int frames = 0;
};

std::optional<Load> s_load;
std::optional<std::string> s_failure;

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

std::string PackOf(int map)
{
    return "map" + std::to_string(map);
}

void Begin(std::string pack, std::vector<Core::StagedLoad::Step> steps, std::function<void()> afterLoad)
{
    if (s_load)
    {
        while (!s_load->steps.Pump(std::chrono::hours{1}))
        {
        }
        Finish();
    }
    s_load.emplace(std::move(pack), std::move(steps), std::move(afterLoad));
    // Asked now, so a download starts with the packet that changes the map.
    Content::RequestPack(s_load->pack);
}

bool Active()
{
    return s_load.has_value();
}

void Pump(const std::function<bool()>& holdBack)
{
    if (!s_load)
    {
        return;
    }
    ++s_load->frames;
    if (!s_load->pack.empty())
    {
        const Content::PackStatus status = Content::RequestPack(s_load->pack);
        if (status.state == Content::PackStatus::State::Pending)
        {
            return;
        }
        if (status.state == Content::PackStatus::State::Failed)
        {
            mu::log::Get("render")->error("Map change dropped: content pack {} failed: {}", s_load->pack, status.error);
            s_failure = s_load->pack + ": " + status.error;
            s_load.reset();
            return;
        }
        s_load->pack.clear();
    }
    if (s_load->steps.Pump(StepBudget, holdBack))
    {
        Finish();
    }
}

bool LoadingScreenDue()
{
    return s_load && std::chrono::steady_clock::now() - s_load->start >= LoadingScreenAfter;
}

std::optional<std::string> TakeFailure()
{
    return std::exchange(s_failure, std::nullopt);
}

void Cancel()
{
    s_load.reset();
}
} // namespace World::MapLoad
