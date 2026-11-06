#include "doctest.h"

#include "Core/Utilities/VersionFiles.h"

#include <filesystem>
#include <fstream>
#include <string>

namespace
{
// A file in its own temporary folder, removed with the folder at the end of the test.
class TemporaryFile
{
public:
    explicit TemporaryFile(const std::string& contents)
        : m_folder(std::filesystem::temp_directory_path() / "mu-version-files-test")
    {
        std::filesystem::remove_all(m_folder);
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
} // namespace

TEST_CASE("Version files give their first line [core][build]")
{
    CHECK(Core::Build::ReadVersionFile(TemporaryFile("v1.37.0\n").Path()) == "v1.37.0");
    CHECK(Core::Build::ReadVersionFile(TemporaryFile("data-3f2a9c1e7b0d4a55").Path()) == "data-3f2a9c1e7b0d4a55");
    CHECK(Core::Build::ReadVersionFile(TemporaryFile("v1.37.0 \r\nsecond line\n").Path()) == "v1.37.0");
}

TEST_CASE("Version files that hold no version read as empty [core][build]")
{
    CHECK(Core::Build::ReadVersionFile("no-such-folder/version.txt").empty());
    CHECK(Core::Build::ReadVersionFile(TemporaryFile("").Path()).empty());
    CHECK(Core::Build::ReadVersionFile(TemporaryFile("\nv1.37.0\n").Path()).empty());
    CHECK(Core::Build::ReadVersionFile(TemporaryFile("v1.37.0 %s\n").Path()).empty());
    CHECK(Core::Build::ReadVersionFile(TemporaryFile(std::string(65, '1')).Path()).empty());
}

TEST_CASE("The release line names the tag or no release [core][build]")
{
    CHECK(Core::Build::DescribeRelease("v1.37.0") == "v1.37.0");
    CHECK(Core::Build::DescribeRelease("") == "none (not a release download)");
}

TEST_CASE("The data line compares the installed and expected data releases [core][build]")
{
    CHECK(Core::Build::DescribeData("data-aaaa", "data-aaaa") == "data-aaaa (matches this client)");
    CHECK(Core::Build::DescribeData("data-bbbb", "data-aaaa") == "data-bbbb, but this client expects data-aaaa");
    CHECK(Core::Build::DescribeData("", "data-aaaa") ==
          "unknown (no Data/DataVersion.txt), this client expects data-aaaa");
    CHECK(Core::Build::DescribeData("", "unknown") == "unknown (no Data/DataVersion.txt)");
    CHECK(Core::Build::DescribeData("data-bbbb", "unknown") == "data-bbbb (this client's data release is unknown)");
}
