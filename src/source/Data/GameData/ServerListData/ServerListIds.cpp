#include "stdafx.h"

#include "ServerListIds.h"

#include <algorithm>

namespace Data::ServerList
{
namespace
{
bool IsLower(char character)
{
    return character >= 'a' && character <= 'z';
}

bool IsUpper(char character)
{
    return character >= 'A' && character <= 'Z';
}

bool IsDigit(char character)
{
    return character >= '0' && character <= '9';
}

char ToLower(char character)
{
    return IsUpper(character) ? static_cast<char>(character - 'A' + 'a') : character;
}

char ToUpper(char character)
{
    return IsLower(character) ? static_cast<char>(character - 'a' + 'A') : character;
}
} // namespace

bool IsListId(std::string_view id)
{
    return !id.empty() && IsLower(id.front()) &&
           std::all_of(id.begin(), id.end(),
                       [](char character) { return IsLower(character) || IsUpper(character) || IsDigit(character); });
}

std::string MakeListId(std::string_view name, std::string_view fallback)
{
    // Each word of letters and digits; the first in lowercase, each other one capitalized.
    std::string id;
    bool wordStart = true;
    for (const char character : name)
    {
        if (!IsLower(character) && !IsUpper(character) && !IsDigit(character))
        {
            wordStart = true;
            continue;
        }
        if (id.empty())
        {
            if (IsDigit(character))
            {
                continue;
            }
            id += ToLower(character);
        }
        else
        {
            id += wordStart ? ToUpper(character) : ToLower(character);
        }
        wordStart = false;
    }
    return id.empty() ? std::string(fallback) : id;
}
} // namespace Data::ServerList
