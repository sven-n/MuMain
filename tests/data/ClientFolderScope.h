#pragma once

#include <filesystem>

// Runs a test from inside a client folder, as the game runs from it: model and texture paths
// start with Data.
class ClientFolderScope
{
public:
    explicit ClientFolderScope(const std::filesystem::path& folder) : m_previous(std::filesystem::current_path())
    {
        std::filesystem::current_path(folder);
    }
    ~ClientFolderScope()
    {
        std::error_code ignored;
        std::filesystem::current_path(m_previous, ignored);
    }
    ClientFolderScope(const ClientFolderScope&) = delete;
    ClientFolderScope& operator=(const ClientFolderScope&) = delete;

private:
    std::filesystem::path m_previous;
};
