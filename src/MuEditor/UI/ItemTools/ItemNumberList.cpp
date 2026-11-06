#include "stdafx.h"

#ifdef _EDITOR

#include "ItemNumberList.h"

#include <charconv>

namespace MuEditor::Items::NumberList
{
namespace
{
bool IsSeparator(char character)
{
    return character == ',' || character == ' ';
}
} // namespace

std::string Format(const std::vector<int>& numbers)
{
    std::string text;
    for (const int number : numbers)
    {
        text += (text.empty() ? "" : ", ") + std::to_string(number);
    }
    return text;
}

std::optional<std::vector<int>> Parse(std::string_view text)
{
    std::vector<int> numbers;
    size_t position = 0;
    while (position < text.size())
    {
        if (IsSeparator(text[position]))
        {
            ++position;
            continue;
        }
        int number = 0;
        const auto [end, error] = std::from_chars(text.data() + position, text.data() + text.size(), number);
        if (error != std::errc{} || number < 0)
        {
            return std::nullopt;
        }
        numbers.push_back(number);
        position = static_cast<size_t>(end - text.data());
    }
    return numbers;
}
} // namespace MuEditor::Items::NumberList

#endif // _EDITOR
