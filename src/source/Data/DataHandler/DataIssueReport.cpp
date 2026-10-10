#include "stdafx.h"

#include "DataIssueReport.h"

#include "Core/Utilities/Log/MuLogger.h"

namespace Data
{
namespace
{
// How many errors the player sees; all of them go to the log.
constexpr size_t MaxErrorsInMessage = 10;
} // namespace

using Items::ItemDataIssue;
using Items::ItemDataIssueSeverity;

void LogDataIssues(std::string_view label, std::span<const ItemDataIssue> issues)
{
    const auto logger = mu::log::Get("data");
    for (const ItemDataIssue& issue : issues)
    {
        if (issue.severity == ItemDataIssueSeverity::Error)
        {
            MU_LOG_ERROR(logger, "{} {}", label, issue.ToString());
        }
        else
        {
            MU_LOG_WARN(logger, "{} {}", label, issue.ToString());
        }
    }
}

std::string DescribeDataErrors(std::string_view what, const std::filesystem::path& directory,
                               std::span<const ItemDataIssue> issues)
{
    std::string message = "The " + std::string(what) + " in " + directory.string() + " has errors:\n";
    size_t errorCount = 0;
    for (const ItemDataIssue& issue : issues)
    {
        if (issue.severity != ItemDataIssueSeverity::Error)
        {
            continue;
        }
        if (++errorCount <= MaxErrorsInMessage)
        {
            message += "\n" + issue.ToString();
        }
    }
    if (errorCount > MaxErrorsInMessage)
    {
        message += "\n... and " + std::to_string(errorCount - MaxErrorsInMessage) + " more";
    }
    return message + "\n\nAll problems are listed in MuError.log.";
}
} // namespace Data
