#include "stdafx.h"

#include "Data/Translation/ItemReference.h"

#include <algorithm>

namespace Data::Translation
{
namespace
{
// A group, number or level has at most this many digits, as ResxGen's check allows.
constexpr std::size_t MaxDigits = 4;
constexpr std::string_view NamePrefix = "item:";

bool IsDigit(char c)
{
    return c >= '0' && c <= '9';
}

// Reads 1 to MaxDigits digits at `at` and moves past them.
std::optional<int> ReadNumber(std::string_view text, std::size_t& at)
{
    const std::size_t begin = at;
    int value = 0;
    while (at < text.size() && IsDigit(text[at]) && at - begin < MaxDigits)
    {
        value = value * 10 + (text[at] - '0');
        ++at;
    }
    if (at == begin || (at < text.size() && IsDigit(text[at])))
    {
        return std::nullopt;
    }
    return value;
}

// The form after ":" up to the end; empty text is not a form.
std::optional<std::string> ReadForm(std::string_view text, std::size_t at)
{
    const std::string_view form = text.substr(at);
    if (!IsItemCaseKey(form))
    {
        return std::nullopt;
    }
    return std::string(form);
}
} // namespace

bool IsItemCaseKey(std::string_view key)
{
    return std::find(ItemCaseKeys.begin(), ItemCaseKeys.end(), key) != ItemCaseKeys.end();
}

std::optional<ItemReference> ParseItemReferenceName(std::string_view name)
{
    if (!name.starts_with(NamePrefix))
    {
        return std::nullopt;
    }
    std::size_t at = NamePrefix.size();
    ItemReference reference;
    const std::optional<int> group = ReadNumber(name, at);
    if (!group || at >= name.size() || name[at] != ',')
    {
        return std::nullopt;
    }
    ++at;
    const std::optional<int> number = ReadNumber(name, at);
    if (!number)
    {
        return std::nullopt;
    }
    reference.group = *group;
    reference.number = *number;
    if (at < name.size() && name[at] == '+')
    {
        ++at;
        reference.level = ReadNumber(name, at);
        if (!reference.level)
        {
            return std::nullopt;
        }
    }
    if (at < name.size() && name[at] == ':')
    {
        std::optional<std::string> form = ReadForm(name, at + 1);
        if (!form)
        {
            return std::nullopt;
        }
        reference.form = std::move(*form);
        at = name.size();
    }
    if (at != name.size())
    {
        return std::nullopt;
    }
    return reference;
}

std::optional<ItemReference> ParseItemReference(std::string_view written)
{
    if (written.size() < 2 || written.front() != '{' || written.back() != '}')
    {
        return std::nullopt;
    }
    return ParseItemReferenceName(written.substr(1, written.size() - 2));
}

std::vector<ItemReferenceMatch> FindItemReferences(std::string_view text)
{
    std::vector<ItemReferenceMatch> matches;
    for (std::size_t at = text.find(ItemReferenceOpening); at != std::string_view::npos;
         at = text.find(ItemReferenceOpening, at + ItemReferenceOpening.size()))
    {
        const std::size_t close = text.find('}', at);
        ItemReferenceMatch match;
        match.position = at;
        match.length = (close == std::string_view::npos ? text.size() : close + 1) - at;
        match.reference = ParseItemReference(text.substr(at, match.length));
        matches.push_back(std::move(match));
    }
    return matches;
}

bool HasItemReference(std::string_view text)
{
    return text.find(ItemReferenceOpening) != std::string_view::npos;
}

std::string FormatItemReference(const ItemReference& reference)
{
    std::string written(ItemReferenceOpening);
    written += std::to_string(reference.group) + "," + std::to_string(reference.number);
    if (reference.level)
    {
        written += "+" + std::to_string(*reference.level);
    }
    if (!reference.form.empty())
    {
        written += ":" + reference.form;
    }
    written += "}";
    return written;
}
} // namespace Data::Translation
