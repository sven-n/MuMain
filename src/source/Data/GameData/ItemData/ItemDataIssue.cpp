#include "stdafx.h"

#include "ItemDataIssue.h"

#include <algorithm>

namespace Data::Items
{
std::string ItemDataIssue::ToString() const
{
    std::string result = severity == ItemDataIssueSeverity::Error ? "error: " : "warning: ";
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

bool HasErrors(std::span<const ItemDataIssue> issues)
{
    return std::any_of(issues.begin(), issues.end(),
                       [](const ItemDataIssue& issue) { return issue.severity == ItemDataIssueSeverity::Error; });
}
} // namespace Data::Items
