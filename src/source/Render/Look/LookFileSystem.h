#pragma once

#include <filesystem>
#include <optional>
#include <string>

// The file operations the look profile's load and save go through, so doctests can make any of
// them fail.
namespace Render::Look
{

class LookFileSystem
{
public:
    virtual ~LookFileSystem() = default;

    [[nodiscard]] virtual bool Exists(const std::filesystem::path& path) = 0;
    // The whole file without a UTF-8 byte order mark; nothing when it cannot be read.
    [[nodiscard]] virtual std::optional<std::string> Read(const std::filesystem::path& path) = 0;
    // Creates or replaces the file with `text`.
    [[nodiscard]] virtual bool Write(const std::filesystem::path& path, const std::string& text) = 0;
    // Copies `from` over `to`.
    [[nodiscard]] virtual bool Copy(const std::filesystem::path& from, const std::filesystem::path& to) = 0;
    // Moves `from` over `to`, replacing it.
    [[nodiscard]] virtual bool Rename(const std::filesystem::path& from, const std::filesystem::path& to) = 0;
    // Deletes the file; true when it is gone (also when it never existed).
    virtual bool Remove(const std::filesystem::path& path) = 0;
};

// The real files.
class DiskLookFileSystem final : public LookFileSystem
{
public:
    [[nodiscard]] bool Exists(const std::filesystem::path& path) override;
    [[nodiscard]] std::optional<std::string> Read(const std::filesystem::path& path) override;
    [[nodiscard]] bool Write(const std::filesystem::path& path, const std::string& text) override;
    [[nodiscard]] bool Copy(const std::filesystem::path& from, const std::filesystem::path& to) override;
    [[nodiscard]] bool Rename(const std::filesystem::path& from, const std::filesystem::path& to) override;
    bool Remove(const std::filesystem::path& path) override;
};

} // namespace Render::Look
