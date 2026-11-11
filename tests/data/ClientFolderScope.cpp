#include "stdafx.h"
#include "ClientFolderScope.h"

#include <system_error>

ClientFolderScope::ClientFolderScope(const std::filesystem::path& folder) : m_previous(std::filesystem::current_path())
{
    std::filesystem::current_path(folder);
}

ClientFolderScope::~ClientFolderScope()
{
    std::error_code ignored;
    std::filesystem::current_path(m_previous, ignored);
}
