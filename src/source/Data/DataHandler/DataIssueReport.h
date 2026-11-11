#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <filesystem>
#include <span>
#include <string>
#include <string_view>

// Reports the problems found while loading a data folder: all of them go to
// the "data" log, the first fatal ones to the player.
namespace Data
{
// Logs every issue as "<label> <issue>", e.g. "Item data fatal: ...":
// fatal problems and errors as errors, warnings as warnings.
void LogDataIssues(std::string_view label, std::span<const Items::ItemDataIssue> issues);

// "The <what> in <directory> cannot be used:" and the first fatal problems,
// for the message that stops the start.
std::string DescribeFatalIssues(std::string_view what, const std::filesystem::path& directory,
                                std::span<const Items::ItemDataIssue> issues);
} // namespace Data
