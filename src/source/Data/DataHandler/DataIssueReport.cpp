#include "stdafx.h"

#include "DataIssueReport.h"

#include "Core/Utilities/Log/MuLogger.h"

namespace Data
{
namespace
{
// How many fatal problems the player sees; all of them go to the log.
constexpr size_t MaxFatalInMessage = 10;
} // namespace

using Items::ItemDataIssue;
using Items::ItemDataIssueSeverity;

void LogDataIssues(std::string_view label, std::span<const ItemDataIssue> issues)
{
    const auto logger = mu::log::Get("data");
    for (const ItemDataIssue& issue : issues)
    {
        if (issue.severity != ItemDataIssueSeverity::Warning)
        {
            MU_LOG_ERROR(logger, "{} {}", label, issue.ToString());
        }
        else
        {
            MU_LOG_WARN(logger, "{} {}", label, issue.ToString());
        }
    }
}

std::string DescribeFatalIssues(std::string_view what, const std::filesystem::path& directory,
                                std::span<const ItemDataIssue> issues)
{
    std::string message = "The " + std::string(what) + " in " + directory.string() + " cannot be used:\n";
    size_t fatalCount = 0;
    for (const ItemDataIssue& issue : issues)
    {
        if (issue.severity != ItemDataIssueSeverity::Fatal)
        {
            continue;
        }
        if (++fatalCount <= MaxFatalInMessage)
        {
            message += "\n" + issue.ToString();
        }
    }
    if (fatalCount > MaxFatalInMessage)
    {
        message += "\n... and " + std::to_string(fatalCount - MaxFatalInMessage) + " more";
    }
    return message + "\n\nAll problems are listed in MuError.log.";
}
} // namespace Data
