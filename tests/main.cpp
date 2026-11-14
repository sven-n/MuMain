#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#include <cstdlib>
#include <filesystem>

// A test's settings and log stay beside its own executable, never in the developer's user folder
// (Core/Platform/UserFolder.h).
int main(int argc, char** argv)
{
    // An empty value counts as unset, as the client reads it.
    const char* folderVariable = std::getenv("MU_USER_FOLDER");
    if (argc > 0 && (folderVariable == nullptr || folderVariable[0] == '\0'))
    {
        std::error_code error;
        const std::filesystem::path folder = std::filesystem::absolute(argv[0], error).parent_path();
#ifdef _WIN32
        _wputenv_s(L"MU_USER_FOLDER", folder.wstring().c_str());
#else
        setenv("MU_USER_FOLDER", folder.c_str(), 1);
#endif
    }
    return doctest::Context(argc, argv).run();
}
