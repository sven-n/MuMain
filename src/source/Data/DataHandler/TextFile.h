#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace Data
{
// The whole file as text, without a UTF-8 byte order mark; nullopt when it
// cannot be opened.
std::optional<std::string> ReadTextFile(const std::filesystem::path& path);
} // namespace Data
