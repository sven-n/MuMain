#include "doctest.h"

#include "Core/Utilities/VersionFiles.h"

#include <ctime>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>

#ifdef _WIN32
#include <sys/utime.h>
#else
#include <utime.h>
#endif

namespace
{
// A folder name of its own: test runs of other build folders may run at the same time and share the temp folder.
std::filesystem::path UniqueTemporaryFolder()
{
    static const unsigned runId = std::random_device{}();
    static unsigned fileNumber = 0;
    const std::string name =
        "mu-version-files-test-" + std::to_string(runId) + "-" + std::to_string(fileNumber++);
    return std::filesystem::temp_directory_path() / name;
}

// A file in its own temporary folder, removed with the folder at the end of the test.
class TemporaryFile
{
public:
    explicit TemporaryFile(const std::string& contents) : m_folder(UniqueTemporaryFolder())
    {
        std::filesystem::create_directories(m_folder);
        std::ofstream(Path(), std::ios::binary) << contents;
    }
    ~TemporaryFile()
    {
        std::filesystem::remove_all(m_folder);
    }
    TemporaryFile(const TemporaryFile&) = delete;
    TemporaryFile& operator=(const TemporaryFile&) = delete;

    std::filesystem::path Path() const
    {
        return m_folder / "version.txt";
    }

private:
    std::filesystem::path m_folder;
};

// The version a file with these contents reads as.
std::string VersionIn(const std::string& contents)
{
    return Core::Build::ReadVersionFile(TemporaryFile(contents).Path()).version;
}

// A version file that isn't there, and one that is, holding this version (empty: none).
Core::Build::VersionFile Missing()
{
    return {};
}
Core::Build::VersionFile Holding(const std::string& version)
{
    return {true, version};
}

// Sets the file's write time, in whole seconds since 1970.
void SetWriteTime(const std::filesystem::path& file, std::time_t seconds)
{
#ifdef _WIN32
    __utimbuf64 times{seconds, seconds};
    REQUIRE(_wutime64(file.c_str(), &times) == 0);
#else
    utimbuf times{seconds, seconds};
    REQUIRE(utime(file.c_str(), &times) == 0);
#endif
}
} // namespace

TEST_CASE("Version files give their first line [core][build]")
{
    CHECK(VersionIn("v1.37.0\n") == "v1.37.0");
    CHECK(VersionIn("data-3f2a9c1e7b0d4a55") == "data-3f2a9c1e7b0d4a55");
    CHECK(VersionIn("v1.37.0 \r\nsecond line\n") == "v1.37.0");
    // Saved with a UTF-8 byte order mark, as Windows tools can.
    CHECK(VersionIn("\xEF\xBB\xBFv1.37.0\r\n") == "v1.37.0");
    CHECK(Core::Build::ReadVersionFile(TemporaryFile("v1.37.0\n").Path()).found);
}

TEST_CASE("A missing version file is not found [core][build]")
{
    const Core::Build::VersionFile missing = Core::Build::ReadVersionFile("no-such-folder/version.txt");
    CHECK_FALSE(missing.found);
    CHECK(missing.version.empty());
}

TEST_CASE("Version files that hold no version are found with no version [core][build]")
{
    for (const std::string& contents :
         {std::string(), std::string("\nv1.37.0\n"), std::string("v1.37.0 %s\n"), std::string(65, '1')})
    {
        CAPTURE(contents);
        const Core::Build::VersionFile file = Core::Build::ReadVersionFile(TemporaryFile(contents).Path());
        CHECK(file.found);
        CHECK(file.version.empty());
    }
}

TEST_CASE("The release line names the tag, no release, or an unreadable file [core][build]")
{
    CHECK(Core::Build::DescribeRelease(Holding("v1.37.0")) == "v1.37.0");
    CHECK(Core::Build::DescribeRelease(Missing()) == "none (not a release download)");
    CHECK(Core::Build::DescribeRelease(Holding("")) == "unreadable (release-version.txt holds no release tag)");
}

TEST_CASE("The data line compares the installed and expected data releases [core][build]")
{
    CHECK(Core::Build::DescribeData(Holding("data-aaaa"), "data-aaaa") == "data-aaaa (matches this client)");
    CHECK(Core::Build::DescribeData(Holding("data-bbbb"), "data-aaaa") ==
          "data-bbbb, but this client expects data-aaaa");
    CHECK(Core::Build::DescribeData(Missing(), "data-aaaa") ==
          "unknown (no Data/DataVersion.txt), this client expects data-aaaa");
    CHECK(Core::Build::DescribeData(Missing(), "") == "unknown (no Data/DataVersion.txt)");
    CHECK(Core::Build::DescribeData(Holding(""), "data-aaaa") ==
          "unreadable (Data/DataVersion.txt holds no data release id), this client expects data-aaaa");
    CHECK(Core::Build::DescribeData(Holding("data-bbbb"), "") ==
          "data-bbbb (this client's data release is unknown)");
}

TEST_CASE("A file's write time reads in UTC [core][build]")
{
    const TemporaryFile file("client");
    // 1700000000 seconds after 1970 is 2023-11-14 22:13:20 UTC.
    SetWriteTime(file.Path(), 1700000000);
    CHECK(Core::Build::DescribeFileTime(file.Path()) == "2023-11-14 22:13:20 UTC");
    CHECK(Core::Build::DescribeFileTime("no-such-folder/Main.exe").empty());
}
