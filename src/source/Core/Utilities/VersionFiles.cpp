#include "stdafx.h"

#include "VersionFiles.h"

#include <sys/stat.h>
#include <sys/types.h>

#include <algorithm>
#include <cctype>
#include <ctime>
#include <fstream>

namespace Core::Build
{
namespace
{
constexpr std::size_t MaxVersionLength = 64;
constexpr const char* UnknownId = "unknown";
constexpr const char* FileTimeFormat = "%Y-%m-%d %H:%M:%S UTC";

bool IsVersionCharacter(unsigned char character)
{
    return std::isalnum(character) != 0 || character == '.' || character == '+' || character == '-' || character == '_';
}

bool IsVersion(const std::string& text)
{
    return !text.empty() && text.size() <= MaxVersionLength &&
           std::all_of(text.begin(), text.end(),
                       [](char c) { return IsVersionCharacter(static_cast<unsigned char>(c)); });
}
} // namespace

std::string ReadVersionFile(const std::filesystem::path& file)
{
    std::ifstream stream(file, std::ios::binary);
    std::string line;
    if (!stream || !std::getline(stream, line))
        return {};

    // Written on Linux, but someone may have opened and saved it on Windows.
    while (!line.empty() && (line.back() == '\r' || line.back() == ' '))
        line.pop_back();
    return IsVersion(line) ? line : std::string();
}

std::string DescribeRelease(const std::string& releaseTag)
{
    if (releaseTag.empty())
        return "none (not a release download)";
    return releaseTag;
}

std::string DescribeData(const std::string& installedId, const std::string& expectedId)
{
    const bool expectedKnown = expectedId != UnknownId;
    if (installedId.empty())
    {
        // Self-built clients copy Data from their checkout, and data downloads from before the file have none.
        std::string text = std::string("unknown (no ") + kDataVersionFile + ")";
        return expectedKnown ? text + ", this client expects " + expectedId : text;
    }
    if (!expectedKnown)
        return installedId + " (this client's data release is unknown)";
    if (installedId == expectedId)
        return installedId + " (matches this client)";
    return installedId + ", but this client expects " + expectedId;
}

std::string DescribeFileTime(const std::filesystem::path& file)
{
    // stat's whole seconds since 1970, which std::filesystem's file clock can't turn into a date on every compiler.
    std::tm utc{};
#ifdef _WIN32
    struct _stat64 status{};
    if (_wstat64(file.c_str(), &status) != 0)
        return {};
    const std::time_t written = status.st_mtime;
    if (gmtime_s(&utc, &written) != 0)
        return {};
#else
    struct stat status{};
    if (stat(file.c_str(), &status) != 0)
        return {};
    const std::time_t written = status.st_mtime;
    if (gmtime_r(&written, &utc) == nullptr)
        return {};
#endif

    char text[32] = {};
    if (std::strftime(text, sizeof(text), FileTimeFormat, &utc) == 0)
        return {};
    return text;
}
} // namespace Core::Build
