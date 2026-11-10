#pragma once

#include <filesystem>

// Runs a test from inside a client folder, as the game runs from it: model and texture paths
// start with Data.
class ClientFolderScope
{
public:
    explicit ClientFolderScope(const std::filesystem::path& folder);
    ~ClientFolderScope();
    ClientFolderScope(const ClientFolderScope&) = delete;
    ClientFolderScope& operator=(const ClientFolderScope&) = delete;

private:
    std::filesystem::path m_previous;
};
