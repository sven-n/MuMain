#include "stdafx.h"

#include "VersionFiles.h"

#include "Core/Content/ContentStore.h"

#include <sys/stat.h>
#include <sys/types.h>

#include <algorithm>
#include <cctype>
#include <ctime>
#include <optional>
#include <sstream>
#include <string_view>

namespace Core::Build
{
namespace
{
constexpr std::size_t MaxVersionLength = 64;
constexpr std::string_view Utf8ByteOrderMark = "\xEF\xBB\xBF";
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

VersionFile ReadVersionFile(const std::filesystem::path& file)
{
    const std::optional<std::string> text = Content::ReadFile(file);
    if (!text)
        return {};
    std::istringstream stream(*text);

    VersionFile result;
    result.found = true;
    std::string line;
    if (!std::getline(stream, line))
        return result;

    // Written on Linux, but someone may have opened and saved it on Windows, which can add a byte order mark
    // (Notepad before Windows 10 1903, PowerShell 5's -Encoding UTF8) and a CR.
    if (line.starts_with(Utf8ByteOrderMark))
        line.erase(0, Utf8ByteOrderMark.size());
    while (!line.empty() && (line.back() == '\r' || line.back() == ' '))
        line.pop_back();
    if (IsVersion(line))
        result.version = line;
    return result;
}

std::string DescribeRelease(const VersionFile& release)
{
    if (!release.found)
        return "none (not a release download)";
    if (release.version.empty())
        return std::string("unreadable (") + kReleaseVersionFile + " holds no release tag)";
    return release.version;
}

std::string DescribeData(const VersionFile& installed, const std::string& expectedId)
{
    const bool expectedKnown = !expectedId.empty();
    const std::string& installedId = installed.version;
    if (installedId.empty())
    {
        // Self-built clients copy Data from their checkout, and data downloads from before the file have none.
        const std::string text = installed.found
                                     ? std::string("unreadable (") + kDataVersionFile + " holds no data release id)"
                                     : std::string("unknown (no ") + kDataVersionFile + ")";
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
