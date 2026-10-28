#include "stdafx.h"

#include "ItemDataIssue.h"

#include <algorithm>

namespace Data::Items
{
const char* GetSeverityName(ItemDataIssueSeverity severity)
{
    switch (severity)
    {
    case ItemDataIssueSeverity::Fatal:
        return "fatal";
    case ItemDataIssueSeverity::Error:
        return "error";
    case ItemDataIssueSeverity::Warning:
        break;
    }
    return "warning";
}

std::string ItemDataIssue::ToString() const
{
    std::string result = std::string(GetSeverityName(severity)) + ": ";
    if (!source.empty())
    {
        result += source + ": ";
    }
    if (group != NoItem && number != NoItem)
    {
        result += "item (" + std::to_string(group) + "," + std::to_string(number) + ") ";
    }
    else if (group != NoItem)
    {
        result += "group " + std::to_string(group) + " ";
    }
    if (!field.empty())
    {
        result += field + ": ";
    }
    return result + message;
}

bool HasFatal(std::span<const ItemDataIssue> issues)
{
    return std::any_of(issues.begin(), issues.end(),
                       [](const ItemDataIssue& issue) { return issue.severity == ItemDataIssueSeverity::Fatal; });
}
} // namespace Data::Items
