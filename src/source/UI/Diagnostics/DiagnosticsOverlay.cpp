#include "stdafx.h"
#include "UI/Diagnostics/DiagnosticsOverlay.h"
#include "UI/Diagnostics/DiagnosticsModel.h"

#include "Core/Utilities/FrameProfiler.h"
#include "Render/Renderer/MuRenderer.h"
#include "UI/Placement/WindowPlacement.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlThemedView.h"
#include "UI/RmlBridge/RmlWorkspaceParticipant.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <memory>

namespace UI::Diagnostics
{
namespace
{
using View = UI::RmlBridge::ThemedView<DiagnosticsModel>;
std::unique_ptr<View> g_view;
constexpr auto RefreshInterval = std::chrono::milliseconds(250);
auto g_nextRefresh = std::chrono::steady_clock::time_point{};
constexpr size_t GraphBars = 60;

void RegisterModel(Rml::DataModelConstructor& constructor, DiagnosticsModel& model)
{
    constructor.RegisterTransformFunc("fixed", [](const Rml::VariantList& args) -> Rml::Variant
    {
        if (args.empty())
            return Rml::Variant(Rml::String());
        char text[48];
        std::snprintf(text, sizeof(text), "%.2f", args.front().Get<double>());
        return Rml::Variant(Rml::String(text));
    });
    auto columns = constructor.RegisterStruct<DiagnosticsModel::PassColumns>();
    columns.RegisterMember("names", &DiagnosticsModel::PassColumns::names);
    columns.RegisterMember("ms", &DiagnosticsModel::PassColumns::milliseconds);
    columns.RegisterMember("draws", &DiagnosticsModel::PassColumns::draws);
    columns.RegisterMember("merged", &DiagnosticsModel::PassColumns::merged);
    columns.RegisterMember("vertex_kb", &DiagnosticsModel::PassColumns::vertexKB);
    constructor.RegisterArray<Rml::Vector<float>>();
    constructor.RegisterArray<Rml::Vector<Rml::String>>();
    constructor.Bind("details", &model.details);
    constructor.Bind("counters", &model.counters);
    constructor.Bind("fps_only", &model.fpsOnly);
    constructor.Bind("fps", &model.fps);
    constructor.Bind("average_fps", &model.averageFps);
    constructor.Bind("low_fps", &model.lowFps);
    constructor.Bind("slowest_fps", &model.slowestFps);
    constructor.Bind("info", &model.info);
    constructor.Bind("frame_ms", &model.frameMs);
    constructor.Bind("cpu", &model.cpu);
    constructor.Bind("vsync", &model.vsync);
    constructor.Bind("driver", &model.driver);
    constructor.Bind("ui_update", &model.uiUpdate);
    constructor.Bind("ui_render", &model.uiRender);
    constructor.Bind("rml_update", &model.rmlUpdate);
    constructor.Bind("rml_render", &model.rmlRender);
    constructor.Bind("renderer_begin", &model.rendererBegin);
    constructor.Bind("renderer_end", &model.rendererEnd);
    constructor.Bind("submit", &model.submit);
    constructor.Bind("end_offscreen", &model.endOffscreen);
    constructor.Bind("end_stage", &model.endStage);
    constructor.Bind("end_replay", &model.endReplay);
    constructor.Bind("end_post_ui", &model.endPostUi);
    constructor.Bind("requested", &model.requested);
    constructor.Bind("submitted", &model.submitted);
    constructor.Bind("commands", &model.commands);
    constructor.Bind("frame_vertex_kb", &model.frameVertexKB);
    constructor.Bind("textures_uploaded", &model.texturesUploaded);
    constructor.Bind("glyphs_uploaded", &model.glyphsUploaded);
    constructor.Bind("pipeline_binds", &model.pipelineBinds);
    constructor.Bind("sampler_binds", &model.samplerBinds);
    constructor.Bind("vertex_uniforms", &model.vertexUniformPushes);
    constructor.Bind("fragment_uniforms", &model.fragmentUniformPushes);
    constructor.Bind("merged_2d", &model.merged2D);
    constructor.Bind("skin_gpu", &model.skinGpu);
    constructor.Bind("skin_cpu_ineligible", &model.skinCpuIneligible);
    constructor.Bind("skin_failed", &model.skinFailed);
    constructor.Bind("batch_draws", &model.batchDraws);
    constructor.Bind("vertices_per_batch", &model.verticesPerBatch);
    constructor.Bind("passes_left", &model.passesLeft);
    constructor.Bind("passes_right", &model.passesRight);
    constructor.Bind("history", &model.history);
}

bool EnsureView()
{
    if (!g_view)
    {
        UI::RmlBridge::ThemedViewOptions options;
        options.afterBuild = [] { UI::Placement::Invalidate(); };
        g_view = std::make_unique<View>("diagnostics", RegisterModel,
            std::vector<UI::RmlBridge::ThemedDocumentSpec>{{"Data/Interface/RmlUi/diagnostics.rml"}},
            std::move(options));

    }
    return g_view->Ensure();
}

void UpdateTimings(DiagnosticsModel& model)
{
    using Pass = FrameProfiler::Pass;
    model.uiUpdate = FrameProfiler::CompletedMs(Pass::UIUpdate);
    model.uiRender = FrameProfiler::CompletedMs(Pass::UI);
    model.rmlUpdate = FrameProfiler::CompletedMs(Pass::RmlUiUpdate);
    model.rmlRender = FrameProfiler::CompletedMs(Pass::RmlUiRender);
    model.rendererBegin = FrameProfiler::CompletedMs(Pass::RendererBegin);
    model.rendererEnd = FrameProfiler::CompletedMs(Pass::RendererEnd);
    model.submit = FrameProfiler::CompletedMs(Pass::RendererSubmit);
    model.endOffscreen = FrameProfiler::CompletedMs(Pass::RendererOffscreen);
    model.endStage = FrameProfiler::CompletedMs(Pass::RendererStage);
    model.endReplay = FrameProfiler::CompletedMs(Pass::RendererReplay);
    model.endPostUi = FrameProfiler::CompletedMs(Pass::RendererPostUi);
    const char* driver = mu::GetRenderer().GetGPUDriverName();
    model.driver = driver ? driver : "Unavailable";
}

void UpdateCounters(DiagnosticsModel& model)
{
    using Counter = FrameProfiler::Counter;
    const auto stats = mu::GetRenderer().GetFrameStats();
    model.requested = static_cast<int>(stats.requestedDrawCalls);
    model.submitted = static_cast<int>(stats.submittedDrawCalls);
    model.commands = static_cast<int>(stats.commandCount);
    model.frameVertexKB = static_cast<float>(stats.vertexBytes) / 1024.f;
    model.texturesUploaded = static_cast<int>(stats.textureUploads);
    model.glyphsUploaded = static_cast<int>(FrameProfiler::CompletedCounter(Counter::GlyphUploads));
    model.pipelineBinds = static_cast<int>(stats.pipelineBinds);
    model.samplerBinds = static_cast<int>(stats.samplerBinds);
    model.vertexUniformPushes = static_cast<int>(stats.vertexUniformPushes);
    model.fragmentUniformPushes = static_cast<int>(stats.fragmentUniformPushes);
    model.merged2D = static_cast<int>(stats.merged2DDrawCalls);
    model.skinGpu = static_cast<int>(FrameProfiler::CompletedCounter(Counter::GpuSkinningSubmissions));
    model.skinCpuIneligible = static_cast<int>(FrameProfiler::CompletedCounter(Counter::CpuSkinningIneligible));
    model.skinFailed = static_cast<int>(FrameProfiler::CompletedCounter(Counter::GpuSkinningFailures));
    const auto batchDraws = FrameProfiler::CompletedCounter(Counter::BatchDraws);
    const auto batchVertices = FrameProfiler::CompletedCounter(Counter::BatchVertices);
    model.batchDraws = static_cast<int>(batchDraws);
    model.verticesPerBatch = batchDraws > 0 ? static_cast<float>(batchVertices) / static_cast<float>(batchDraws) : 0.f;
    // The first half of the passes in the left columns, the rest in the right.
    constexpr int PassCount = static_cast<int>(FrameProfiler::Pass::Count_);
    model.passesLeft = {};
    model.passesRight = {};
    for (int index = 0; index < PassCount; ++index)
    {
        const auto pass = static_cast<FrameProfiler::Pass>(index);
        auto& columns = index * 2 < PassCount ? model.passesLeft : model.passesRight;
        const auto addLine = [](Rml::String& column, const char* text)
        {
            if (!column.empty())
                column += '\n';
            column += text;
        };
        char text[32];
        addLine(columns.names, FrameProfiler::kPassNames[index]);
        std::snprintf(text, sizeof(text), "%.2f", FrameProfiler::CompletedMs(pass));
        addLine(columns.milliseconds, text);
        std::snprintf(text, sizeof(text), "%u", FrameProfiler::CompletedCounter(pass, Counter::DrawCalls));
        addLine(columns.draws, text);
        std::snprintf(text, sizeof(text), "%u", FrameProfiler::CompletedCounter(pass, Counter::MergedDraws));
        addLine(columns.merged, text);
        constexpr float BytesPerKilobyte = 1024.f;
        std::snprintf(text, sizeof(text), "%.2f", FrameProfiler::CompletedCounter(pass, Counter::VertexBytes) / BytesPerKilobyte);
        addLine(columns.vertexKB, text);
    }
}

void UpdateSummary(DiagnosticsModel& model, const FrameSummary& summary)
{
    model.fps = summary.fps;
    model.averageFps = summary.averageFps;
    model.lowFps = summary.lowFps;
    model.slowestFps = summary.slowestFps;
    model.info.assign(summary.info.begin(), summary.info.end());
    model.frameMs = summary.frameMs;
    model.cpu = summary.cpu;
    model.vsync = summary.vsync;
    // Each bar is the slowest frame of its group, so a spike still shows with a fifth of the bars. The
    // bar count never changes: a shrinking array leaves RmlUi evaluating the bars it is about to remove.
    const size_t samples = summary.history.size();
    model.history.assign(GraphBars, 0.f);
    for (size_t i = 0; i < samples; ++i)
    {
        float& bar = model.history[samples <= GraphBars ? i : i * GraphBars / samples];
        bar = std::max(bar, summary.history[(summary.oldestSample + i) % samples]);
    }
}
}

void Initialize()
{
    UI::RmlBridge::RegisterWorkspaceDocument("diagnostics", [] { return g_view ? g_view->Document() : nullptr; }, "panel");
}

void Update(const FrameSummary& summary)
{
    const bool shown = summary.details || summary.counters || summary.fpsOnly;
    if (!shown && !g_view)
        return;
    if (!EnsureView())
        return;
    auto& model = g_view->GetModel();
    const bool changed = model.details != summary.details || model.counters != summary.counters ||
                         model.fpsOnly != summary.fpsOnly;
    model.details = summary.details;
    model.counters = summary.counters;
    model.fpsOnly = summary.fpsOnly;
    UI::RmlBridge::SyncDocumentVisibilityInFront(g_view->Document(), shown);
    const auto now = std::chrono::steady_clock::now();
    if (shown && (changed || now >= g_nextRefresh))
    {
        UpdateSummary(model, summary);
        UpdateTimings(model);
        if (summary.counters)
            UpdateCounters(model);
        static constexpr const char* Fields[] = {
            "details", "counters", "fps_only", "fps", "average_fps", "low_fps", "slowest_fps", "frame_ms", "cpu", "vsync",
            "driver", "ui_update", "ui_render", "rml_update", "rml_render", "renderer_begin", "renderer_end",
            "submit", "end_offscreen", "end_stage", "end_replay", "end_post_ui", "requested", "submitted",
            "commands", "frame_vertex_kb", "textures_uploaded", "glyphs_uploaded", "pipeline_binds",
            "sampler_binds", "vertex_uniforms", "fragment_uniforms", "merged_2d", "skin_gpu",
            "skin_cpu_ineligible", "skin_failed", "batch_draws", "vertices_per_batch", "passes_left", "passes_right", "history", "info"};
        for (const char* field : Fields)
            g_view->MarkDirty(field);
        g_nextRefresh = now + RefreshInterval;
    }
    if (changed)
        UI::Placement::Invalidate();
    // Also works in the login scene, where CSystem::Update does not run.
    if (shown || changed)
        UI::Placement::Update();
}

void Release()
{
    UI::Placement::UnregisterParticipant("diagnostics");
    g_view.reset();
    g_nextRefresh = {};
}
}
