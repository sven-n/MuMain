#pragma once

#include <filesystem>
#include <string>

// The version files the downloads carry, read for MuError.log: release-version.txt next to the client, which the
// release job writes (the release tag), and Data/DataVersion.txt, which the data workflow writes (the data
// release's id).
namespace Core::Build
{
inline constexpr const char* kReleaseVersionFile = "release-version.txt";
inline constexpr const char* kDataVersionFile = "Data/DataVersion.txt";

// The file's first line, or empty when the file is missing or the line isn't a version: 1 to 64 letters, digits,
// '.', '+', '-' or '_'.
std::string ReadVersionFile(const std::filesystem::path& file);

// The Release line: the release tag, or that the client is no release download.
std::string DescribeRelease(const std::string& releaseTag);

// The Data line: the installed data release against the one the client was built with ("unknown" when the
// build had no git).
std::string DescribeData(const std::string& installedId, const std::string& expectedId);
} // namespace Core::Build
