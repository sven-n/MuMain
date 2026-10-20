#pragma once

#include <RmlUi/Core/Types.h>

namespace UI::Diagnostics
{
struct DiagnosticsModel
{
    // Half of the pass table, a column each: one line per pass. Text columns instead of a box per
    // cell, so a refresh lays out a few text blocks rather than hundreds of boxes.
    struct PassColumns
    {
        Rml::String names;
        Rml::String milliseconds;
        Rml::String draws;
        Rml::String merged;
        Rml::String vertexKB;
    };

    bool details = false;
    bool counters = false;
    bool fpsOnly = false;
    double fps = 0;
    double averageFps = 0;
    double lowFps = 0;
    double slowestFps = 0;
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
    float endOffscreen = 0;
    float endStage = 0;
    float endReplay = 0;
    float endPostUi = 0;
    int requested = 0;
    int submitted = 0;
    int commands = 0;
    float frameVertexKB = 0;
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
    PassColumns passesLeft;
    PassColumns passesRight;
    Rml::Vector<float> history;
    Rml::Vector<Rml::String> info;
};
}
