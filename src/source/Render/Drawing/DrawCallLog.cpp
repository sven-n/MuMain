#include "stdafx.h"

#include "Render/Drawing/DrawCallLog.h"

#include "Render/Textures/ZzzOpenglUtil.h"

#include <algorithm>
#include <map>

namespace Render::Drawing
{
namespace
{
bool s_parityRequested = false;
bool s_parityLater = true;
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
    SetAlphaFuncRef(0.25f);
}

void RequestDrawParity(bool later)
{
    s_parityRequested = true;
    s_parityLater = later;
}

bool TakeDrawParityRequest(bool& later)
{
    later = s_parityLater;
    return std::exchange(s_parityRequested, false);
}

void ReportDrawParity(const std::vector<std::string>& atOnce, const std::vector<std::string>& later)
{
    DrawParityReport report{
        .done = true, .later = s_parityLater, .atOnceCalls = atOnce.size(), .laterCalls = later.size()};
    const auto [atOnceCall, laterCall] = std::mismatch(atOnce.begin(), atOnce.end(), later.begin(), later.end());
    report.firstDifference = static_cast<std::size_t>(atOnceCall - atOnce.begin());
    report.first = {atOnceCall != atOnce.end() ? *atOnceCall : std::string(),
                    laterCall != later.end() ? *laterCall : std::string()};
    std::map<std::string, long long> balance;
    for (const std::string& call : atOnce)
        ++balance[call];
    for (const std::string& call : later)
        --balance[call];
    for (const auto& [call, count] : balance)
    {
        if (count > 0)
        {
            report.onlyAtOnce += static_cast<std::size_t>(count);
            if (report.onlyAtOnceExamples.size() < kReportedDifferences)
                report.onlyAtOnceExamples.push_back(call);
        }
        else if (count < 0)
        {
            report.onlyLater += static_cast<std::size_t>(-count);
            if (report.onlyLaterExamples.size() < kReportedDifferences)
                report.onlyLaterExamples.push_back(call);
        }
    }
    s_parityReport = std::move(report);
}

const DrawParityReport& LastDrawParityReport()
{
    return s_parityReport;
}
} // namespace Render::Drawing
