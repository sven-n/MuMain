#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace Data
{
// The whole file as text, without a UTF-8 byte order mark; nullopt when it
// cannot be opened.
std::optional<std::string> ReadTextFile(const std::filesystem::path& path);

// Writes the text to "<path>.tmp" first, which then takes the old file's place:
// a write that fails leaves the old file as it was. False, with the reason in
// `error`, when the file could not be written or replaced.
bool WriteTextFile(const std::filesystem::path& path, std::string_view text, std::string& error);
} // namespace Data
