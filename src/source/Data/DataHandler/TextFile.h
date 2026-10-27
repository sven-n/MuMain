#pragma once

#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace Data
{
// The whole file as text, without a UTF-8 byte order mark; nullopt when it
// cannot be opened or is not a regular file (a folder).
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

// A file that WriteTextFiles writes, with its new text.
struct TextFileToWrite
{
    std::filesystem::path path;
    std::string_view text;
};

// Writes several files, all or none: every text is written to its temporary
// file before any file is replaced, and when a file then cannot be replaced
// (a read-only or locked file), the files replaced before it get their old
// content back. False, with the file and the reason in `failed` and `error`,
// when a file could not be written or replaced.
bool WriteTextFiles(std::span<const TextFileToWrite> files, std::filesystem::path& failed, std::string& error);
} // namespace Data
