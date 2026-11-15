#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#include <cstdlib>
#include <filesystem>

// A test binary's settings and log stay in a folder of its own beside it, never in the developer's user folder
// (Core/Platform/UserFolder.h): binaries in one output folder run side by side, and one config.ini would be theirs
// all.
int main(int argc, char** argv)
{
    // An empty value counts as unset, as the client reads it.
    const char* folderVariable = std::getenv("MU_USER_FOLDER");
    if (argc > 0 && (folderVariable == nullptr || folderVariable[0] == '\0'))
    {
        std::error_code error;
        const std::filesystem::path executable = std::filesystem::absolute(argv[0], error);
        const std::filesystem::path folder = executable.parent_path() / (executable.stem().string() + ".user");
#ifdef _WIN32
        _wputenv_s(L"MU_USER_FOLDER", folder.wstring().c_str());
#else
        setenv("MU_USER_FOLDER", folder.c_str(), 1);
#endif
    }
    return doctest::Context(argc, argv).run();
}
