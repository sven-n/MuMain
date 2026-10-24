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

// "<path>.tmp", where a save writes the new text before it replaces the file.
std::filesystem::path GetTemporaryPath(const std::filesystem::path& path);

// The first step of WriteTextFile: writes the text to GetTemporaryPath(path).
// False, with the reason in `error`, when it could not be written; no
// temporary file is left then. A save of several files writes all of them
// this way before it replaces any.
bool WriteTemporaryFile(const std::filesystem::path& path, std::string_view text, std::string& error);

// The second step: the temporary file takes the old file's place. False, with
// the reason in `error`, when it could not; the temporary file is removed then.
bool ReplaceWithTemporaryFile(const std::filesystem::path& path, std::string& error);

// Writes the text to "<path>.tmp" first, which then takes the old file's place:
// a write that fails leaves the old file as it was. False, with the reason in
// `error`, when the file could not be written or replaced.
bool WriteTextFile(const std::filesystem::path& path, std::string_view text, std::string& error);
} // namespace Data
