#include "stdafx.h"

#include "Render/PostProcessing/PostProcessingSetting.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Render/Look/LookProfileSetting.h"
#include "Render/Renderer/MuRenderer.h"

namespace Render::PostProcessing
{
namespace
{
bool s_postProcessingActive = false;

bool ApplyToRenderer(bool enabled)
{
    if (mu::GetRenderer().SetPostProcessing(enabled))
    {
        s_postProcessingActive = enabled;
        mu::log::Get("render")->info("Post-processing {}", enabled ? "on" : "off");
        return true;
    }
    s_postProcessingActive = false;
    mu::log::Get("render")->warn("Post-processing requested but unavailable; frames stay unprocessed");
    return false;
}
} // namespace

bool SetPostProcessingEnabled(bool enabled)
{
    return ApplyToRenderer(enabled);
}

bool PostProcessingActive()
{
    return s_postProcessingActive;
}

bool SetPostProcessingView(std::wstring_view name)
{
    struct NamedView
    {
        std::wstring_view name;
        const char* logName;
        mu::PostProcessingView view;
    };
    static constexpr NamedView views[] = {
        {L"scene", "scene", mu::PostProcessingView::Scene},
        {L"source", "bloom source", mu::PostProcessingView::BloomSource},
        {L"bloom", "bloom", mu::PostProcessingView::Bloom},
    };
    for (const NamedView& entry : views)
    {
        if (entry.name == name)
        {
            mu::GetRenderer().SetPostProcessingView(entry.view);
            mu::log::Get("render")->info("Post-processing view: {}", entry.logName);
            return true;
        }
    }
    mu::log::Get("render")->warn("$post view expects scene, source or bloom");
    return false;
}

void SubmitPostProcessingLook()
{
    using Data::Maps::LookValue;
    mu::GetRenderer().SetPostProcessingLook({Render::Look::Value(LookValue::BloomStrength),
                                             Render::Look::Value(LookValue::BloomLevelFalloff),
                                             Render::Look::Value(LookValue::GradingStrength)});
}

} // namespace Render::PostProcessing
