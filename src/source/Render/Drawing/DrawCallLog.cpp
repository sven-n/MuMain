#include "stdafx.h"

#include "Render/Drawing/DrawCallLog.h"

#include "Render/Textures/ZzzOpenglUtil.h"

#include <algorithm>

namespace Render::Drawing
{
namespace
{
bool s_parityRequested = false;
DrawParityReport s_parityReport;
constexpr std::size_t kReportedDifferences = 5;
} // namespace

void ResetLegacyState()
{
    EnableAlphaTest();
    DisableTexture();
    DisableDepthTest();
    DisableDepthMask();
    DisableCullFace();
    EnableAlphaBlend();
    DisableAlphaBlend();
    EnableDepthTest();
    BindTexture(0);
}

void RequestDrawParity()
{
    s_parityRequested = true;
}

bool TakeDrawParityRequest()
{
    return std::exchange(s_parityRequested, false);
}

void ReportDrawParity(const std::vector<std::string>& atOnce, const std::vector<std::string>& later)
{
    DrawParityReport report{.done = true, .atOnce = atOnce.size(), .later = later.size()};
    const std::size_t common = std::min(atOnce.size(), later.size());
    for (std::size_t i = 0; i < common; ++i)
    {
        if (atOnce[i] == later[i])
            continue;
        ++report.differences;
        if (report.first.size() < kReportedDifferences)
            report.first.emplace_back(atOnce[i], later[i]);
    }
    report.differences += std::max(atOnce.size(), later.size()) - common;
    s_parityReport = std::move(report);
}

const DrawParityReport& LastDrawParityReport()
{
    return s_parityReport;
}
} // namespace Render::Drawing
