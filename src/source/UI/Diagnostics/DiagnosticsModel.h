#pragma once

#include <RmlUi/Core/Types.h>

namespace UI::Diagnostics
{
struct DiagnosticsModel
{
    struct PassRow
    {
        Rml::String name;
        float milliseconds = 0;
        int draws = 0;
        int merged = 0;
        float vertexKB = 0;
    };

    bool details = false;
    bool counters = false;
    bool fpsOnly = false;
    double fps = 0;
    double averageFps = 0;
    double lowFps = 0;
    double frameMs = 0;
    double cpu = 0;
    bool vsync = false;
    Rml::String driver;
    float uiUpdate = 0;
    float uiRender = 0;
    float rmlUpdate = 0;
    float rmlRender = 0;
    float rendererBegin = 0;
    float rendererEnd = 0;
    float submit = 0;
    int requested = 0;
    int submitted = 0;
    int texturesUploaded = 0;
    int glyphsUploaded = 0;
    int pipelineBinds = 0;
    int samplerBinds = 0;
    int vertexUniformPushes = 0;
    int fragmentUniformPushes = 0;
    int merged2D = 0;
    int skinGpu = 0;
    int skinCpuIneligible = 0;
    int skinFailed = 0;
    int batchDraws = 0;
    float verticesPerBatch = 0;
    Rml::Vector<PassRow> passes;
    Rml::Vector<float> history;
};
}
