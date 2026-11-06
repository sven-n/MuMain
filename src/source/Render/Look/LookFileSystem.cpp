#include "stdafx.h"

#include "Render/Look/LookFileSystem.h"

#include "Data/GameData/ItemData/ItemJsonCommon.h"

#include <fstream>
#include <system_error>

namespace Render::Look
{

bool DiskLookFileSystem::Exists(const std::filesystem::path& path)
{
    // A path whose existence cannot be checked (an inaccessible folder) counts as present, so reading it
    // reports the failure instead of a missing file quietly selecting the built-in values.
    std::error_code error;
    return std::filesystem::exists(path, error) || static_cast<bool>(error);
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
    return static_cast<bool>(file);
}

bool DiskLookFileSystem::Copy(const std::filesystem::path& from, const std::filesystem::path& to)
{
    std::error_code error;
    std::filesystem::copy_file(from, to, std::filesystem::copy_options::overwrite_existing, error);
    return !error;
}

bool DiskLookFileSystem::Rename(const std::filesystem::path& from, const std::filesystem::path& to)
{
    std::error_code error;
    std::filesystem::rename(from, to, error);
    return !error;
}

bool DiskLookFileSystem::Remove(const std::filesystem::path& path)
{
    std::error_code error;
    std::filesystem::remove(path, error);
    return !error;
}

} // namespace Render::Look
