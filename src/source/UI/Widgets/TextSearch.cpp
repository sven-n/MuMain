#include "TextSearch.h"

namespace UI::TextSearch
{

bool Contains(std::wstring_view text, std::wstring_view token, bool firstOnly)
{
    if (token.size() > text.size())
    {
        return false;
    }

    const std::size_t lastPosition = firstOnly ? 0 : text.size() - token.size();
    for (std::size_t position = 0; position <= lastPosition; ++position)
    {
        if (text.compare(position, token.size(), token) == 0)
        {
            return true;
        }
    }
    return false;
}

} // namespace UI::TextSearch
