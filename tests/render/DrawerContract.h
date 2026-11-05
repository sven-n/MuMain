#pragma once

#include "doctest.h"

#include "Render/Drawing/DeferredDraws.h"
#include "Render/Drawing/DrawCallLog.h"
#include "Render/Renderer/MuRenderer.h"

#include <algorithm>
#include <string>
#include <vector>

// Every drawer and decorator uses the same renderer-level comparison, including final render state.
namespace DrawerContract
{
using CallLog = Render::Drawing::DrawCallLog;

// Draws a scene into `log`, at once or later.
template <typename Scene> void Draw(CallLog& log, Scene scene, bool later)
{
    mu::SetRendererOverride(&log);
    // Both runs start from the same state, told to the log.
    Render::Drawing::ResetLegacyState();
    log.calls.clear();
    log.triangles.clear();
    if (later)
    {
        Render::Drawing::DeferredDraws deferred;
        scene();
        deferred.Flush();
    }
    else
    {
        scene();
    }
    log.NoteFinalState();
    mu::SetRendererOverride(nullptr);
}

// The calls a scene makes, drawn at once or later.
template <typename Scene> std::vector<std::string> CallsOf(Scene scene, bool later, bool gpuShadows)
{
    CallLog log;
    log.gpuShadows = gpuShadows;
    Draw(log, scene, later);
    return log.calls;
}

// `path` names the call the path under test makes ("CachedSkinned" on the GPU, "Triangles" for the vertices the model
// skins on the CPU), so the check shows that path ran.
template <typename Scene> void CheckDrawnLaterAsAtOnce(Scene scene, const char* path = nullptr, bool gpuShadows = false)
{
    const std::vector<std::string> atOnce = CallsOf(scene, false, gpuShadows);
    const std::vector<std::string> later = CallsOf(scene, true, gpuShadows);
    REQUIRE_FALSE(atOnce.empty());
    if (path != nullptr)
    {
        const char* expected = path;
        CHECK(std::any_of(atOnce.begin(), atOnce.end(),
                          [expected](const std::string& call) { return call.rfind(expected, 0) == 0; }));
    }
    CHECK(later.size() == atOnce.size());
    // The first call that differs, and what drawing at once made there.
    const auto [atOnceCall, laterCall] = std::mismatch(atOnce.begin(), atOnce.end(), later.begin(), later.end());
    if (atOnceCall != atOnce.end() && laterCall != later.end())
    {
        INFO("call ", atOnceCall - atOnce.begin(), " drawn at once: ", *atOnceCall);
        CHECK(*laterCall == *atOnceCall);
    }
}

} // namespace DrawerContract
