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

// A version file as read: whether it is there, and the version it holds.
struct VersionFile
{
    bool found = false;  // the file exists and could be opened
    std::string version; // its first line when that is a version, else empty
};

// The file's first line without a UTF-8 byte order mark and trailing CRs and spaces, when it is a version: 1 to 64
// letters, digits, '.', '+', '-' or '_'.
VersionFile ReadVersionFile(const std::filesystem::path& file);

// The Release line: the release tag, that the client is no release download, or that the file holds no tag.
std::string DescribeRelease(const VersionFile& release);

// The Data line: the installed data release against the one the client was built with (MU_DATA_ASSET_ID, empty
// when the build had no git).
std::string DescribeData(const VersionFile& installed, const std::string& expectedId);

// When the file was last written, e.g. "2026-11-06 12:03:17 UTC"; empty when that can't be read. For the client's
// executable that is when it was linked, or in a download the time the archive keeps.
std::string DescribeFileTime(const std::filesystem::path& file);
} // namespace Core::Build
