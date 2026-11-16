#include "stdafx.h"

#include "Render/Look/LookFileSystem.h"

#include "Core/Content/ContentStore.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"

#include <fstream>
#include <system_error>

namespace Render::Look
{

bool DiskLookFileSystem::Exists(const std::filesystem::path& path)
{
    // A path whose existence cannot be checked (an inaccessible folder) counts as present, so reading it
    // reports the failure instead of a missing file quietly selecting the built-in values.
    return Content::Probe(path) != Content::Found::Nothing;
}

std::optional<std::string> DiskLookFileSystem::Read(const std::filesystem::path& path)
{
    return Data::Items::Json::ReadTextFile(path);
}

bool DiskLookFileSystem::Write(const std::filesystem::path& path, const std::string& text)
{
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file)
    {
        return false;
    }
    file.write(text.data(), static_cast<std::streamsize>(text.size()));
    file.close();
    Content::Refresh();
    return static_cast<bool>(file);
}

bool DiskLookFileSystem::Copy(const std::filesystem::path& from, const std::filesystem::path& to)
{
    std::error_code error;
    std::filesystem::copy_file(from, to, std::filesystem::copy_options::overwrite_existing, error);
    Content::Refresh();
    return !error;
}

bool DiskLookFileSystem::Rename(const std::filesystem::path& from, const std::filesystem::path& to)
{
    std::error_code error;
    std::filesystem::rename(from, to, error);
    Content::Refresh();
    return !error;
}

bool DiskLookFileSystem::Remove(const std::filesystem::path& path)
{
    std::error_code error;
    std::filesystem::remove(path, error);
    Content::Refresh();
    return !error;
}

} // namespace Render::Look
