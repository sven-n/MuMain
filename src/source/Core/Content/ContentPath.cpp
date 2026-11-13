#include "Core/Content/ContentPath.h"

namespace Content
{
std::optional<Path> Path::From(std::string_view raw)
{
    if (raw.empty() || raw.front() == '/' || raw.front() == '\\' || (raw.size() >= 2 && raw[1] == ':'))
    {
        return std::nullopt;
    }

    std::string text;
    text.reserve(raw.size());
    std::size_t start = 0;
    while (start <= raw.size())
    {
        std::size_t end = raw.find_first_of("/\\", start);
        if (end == std::string_view::npos)
        {
            end = raw.size();
        }
        const std::string_view part = raw.substr(start, end - start);
        if (part == "..")
        {
            return std::nullopt;
        }
        if (!part.empty() && part != ".")
        {
            if (!text.empty())
            {
                text += '/';
            }
            text += part;
        }
        start = end + 1;
    }

    if (text.empty())
    {
        return std::nullopt;
    }
    return Path(std::move(text));
}

std::string_view Path::Folder() const noexcept
{
    const std::size_t slash = m_text.rfind('/');
    return slash == std::string::npos ? std::string_view{} : std::string_view(m_text).substr(0, slash);
}

std::string_view Path::Name() const noexcept
{
    const std::size_t slash = m_text.rfind('/');
    return slash == std::string::npos ? std::string_view(m_text) : std::string_view(m_text).substr(slash + 1);
}
} // namespace Content
