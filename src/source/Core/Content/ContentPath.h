#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace Content
{
// A file of the game's content named the same way on every platform: relative to the content root, '/' between
// folders, no "." or ".." parts. The case is kept as written; lookups ignore it (ContentStore).
class Path
{
public:
    // Normalised: '\' becomes '/', "." parts and empty parts go. Empty, absolute (a leading '/' or a drive like
    // "C:") and ".." paths are not content, so they give nullopt.
    static std::optional<Path> From(std::string_view raw);

    [[nodiscard]] const std::string& str() const noexcept
    {
        return m_text;
    }
    // The folder part, without its trailing '/'; empty for a file at the root.
    [[nodiscard]] std::string_view Folder() const noexcept;
    [[nodiscard]] std::string_view Name() const noexcept;

    friend bool operator==(const Path&, const Path&) = default;

private:
    explicit Path(std::string text) : m_text(std::move(text)) {}

    std::string m_text;
};
} // namespace Content
