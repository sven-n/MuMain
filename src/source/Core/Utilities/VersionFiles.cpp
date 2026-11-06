#include "stdafx.h"

#include "VersionFiles.h"

#include <algorithm>
#include <cctype>
#include <fstream>

namespace Core::Build
{
namespace
{
constexpr std::size_t MaxVersionLength = 64;
constexpr const char* UnknownId = "unknown";

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
} // namespace Core::Build
