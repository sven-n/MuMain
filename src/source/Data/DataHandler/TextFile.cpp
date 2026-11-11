#include "stdafx.h"

#include "TextFile.h"

#include <cstddef>
#include <fstream>
#include <iterator>
#include <string_view>
#include <vector>

namespace Data
{
namespace
{
constexpr std::string_view Utf8ByteOrderMark = "\xEF\xBB\xBF";
constexpr const char* TemporaryExtension = ".tmp";

// The file's bytes as they are; nullopt when it cannot be opened or is not a
// regular file (a folder).
std::optional<std::string> ReadFileBytes(const std::filesystem::path& path)
{
    // Only a regular file: on Linux a folder opens, and reading it throws.
    std::error_code error;
    if (!std::filesystem::is_regular_file(path, error))
    {
        return std::nullopt;
    }
    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        return std::nullopt;
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

// Gives the file the content it had: `bytes`, or none for a file that was not
// there. False when it could not.
bool PutBack(const std::filesystem::path& path, const std::optional<std::string>& bytes)
{
    if (!bytes)
    {
        std::error_code error;
        std::filesystem::remove(path, error);
        return !error;
    }
    std::string error;
    return WriteTextFile(path, *bytes, error);
}
} // namespace

std::optional<std::string> ReadTextFile(const std::filesystem::path& path)
{
    std::optional<std::string> text = ReadFileBytes(path);
    if (text && text->starts_with(Utf8ByteOrderMark))
    {
        text->erase(0, Utf8ByteOrderMark.size());
    }
    return text;
}

std::filesystem::path GetTemporaryPath(const std::filesystem::path& path)
{
    std::filesystem::path temporaryPath = path;
    temporaryPath += TemporaryExtension;
    return temporaryPath;
}

bool WriteTemporaryFile(const std::filesystem::path& path, std::string_view text, std::string& error)
{
    const std::filesystem::path temporaryPath = GetTemporaryPath(path);
    std::error_code fileError;
    std::filesystem::create_directories(path.parent_path(), fileError);
    std::ofstream stream(temporaryPath, std::ios::binary | std::ios::trunc);
    stream << text;
    // Closing flushes; checking only afterwards also catches a failing final
    // write (e.g. a full disk) before the old file is replaced.
    stream.close();
    if (!stream)
    {
        error = "could not be written";
        std::filesystem::remove(temporaryPath, fileError);
        return false;
    }
    return true;
}

bool ReplaceWithTemporaryFile(const std::filesystem::path& path, std::string& error)
{
    const std::filesystem::path temporaryPath = GetTemporaryPath(path);
    std::error_code fileError;
    std::filesystem::rename(temporaryPath, path, fileError);
    if (fileError)
    {
        error = "could not be replaced: " + fileError.message();
        std::filesystem::remove(temporaryPath, fileError);
        return false;
    }
    return true;
}

bool WriteTextFile(const std::filesystem::path& path, std::string_view text, std::string& error)
{
    return WriteTemporaryFile(path, text, error) && ReplaceWithTemporaryFile(path, error);
}

bool WriteTextFiles(std::span<const TextFileToWrite> files, std::filesystem::path& failed, std::string& error)
{
    std::error_code ignored;
    std::vector<std::optional<std::string>> before;
    before.reserve(files.size());
    for (const TextFileToWrite& file : files)
    {
        before.push_back(ReadFileBytes(file.path));
    }

    for (std::size_t index = 0; index < files.size(); ++index)
    {
        if (WriteTemporaryFile(files[index].path, files[index].text, error))
        {
            continue;
        }
        failed = GetTemporaryPath(files[index].path);
        for (std::size_t written = 0; written < index; ++written)
        {
            std::filesystem::remove(GetTemporaryPath(files[written].path), ignored);
        }
        return false;
    }

    for (std::size_t index = 0; index < files.size(); ++index)
    {
        if (ReplaceWithTemporaryFile(files[index].path, error))
        {
            continue;
        }
        failed = files[index].path;
        for (std::size_t later = index + 1; later < files.size(); ++later)
        {
            std::filesystem::remove(GetTemporaryPath(files[later].path), ignored);
        }
        for (std::size_t replaced = 0; replaced < index; ++replaced)
        {
            if (!PutBack(files[replaced].path, before[replaced]))
            {
                error += "; " + files[replaced].path.generic_string() + " could not be put back";
            }
        }
        return false;
    }
    return true;
}
} // namespace Data
