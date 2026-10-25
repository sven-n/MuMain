#include "stdafx.h"

#include "TextFile.h"

#include <fstream>
#include <iterator>
#include <string_view>

namespace Data
{
namespace
{
constexpr std::string_view Utf8ByteOrderMark = "\xEF\xBB\xBF";
constexpr const char* TemporaryExtension = ".tmp";
} // namespace

std::optional<std::string> ReadTextFile(const std::filesystem::path& path)
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

    std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (text.starts_with(Utf8ByteOrderMark))
    {
        text.erase(0, Utf8ByteOrderMark.size());
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
} // namespace Data
