#include "stdafx.h"

#ifdef _EDITOR

#include "ItemCodeLine.h"

#include "Data/GameData/ItemData/ItemType.h"
#include "UI/Common/CodeRanges.h"
#include "UI/Common/CodeTokenLine.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <system_error>
#include <tuple>
#include <utility>

namespace MuEditor::Items
{
namespace
{
using Kind = ItemCodeReference::Kind;

constexpr std::array<std::string_view, 2> ConstantPrefixes = {"ITEM_", "MODEL_"};

// A group's name in its base constants (ITEM_SWORD, MODEL_SWORD).
struct GroupBaseName
{
    std::string_view name;
    int group;
};

constexpr std::array<GroupBaseName, MAX_ITEM_TYPE> GroupBaseNames = {{
    {"SWORD", ITEM_GROUP_SWORD},
    {"AXE", ITEM_GROUP_AXE},
    {"MACE", ITEM_GROUP_MACE},
    {"SPEAR", ITEM_GROUP_SPEAR},
    {"BOW", ITEM_GROUP_BOW},
    {"STAFF", ITEM_GROUP_STAFF},
    {"SHIELD", ITEM_GROUP_SHIELD},
    {"HELM", ITEM_GROUP_HELM},
    {"ARMOR", ITEM_GROUP_ARMOR},
    {"PANTS", ITEM_GROUP_PANTS},
    {"GLOVES", ITEM_GROUP_GLOVES},
    {"BOOTS", ITEM_GROUP_BOOTS},
    {"WING", ITEM_GROUP_WING},
    {"HELPER", ITEM_GROUP_HELPER},
    {"POTION", ITEM_GROUP_POTION},
    {"ETC", ITEM_GROUP_ETC},
}};

// What after the number of a raw id makes the number part of something else:
// ITEM_POTION + 2 * 3, or a decimal point.
constexpr std::array<std::string_view, 4> NumberContinuations = {"*", "/", "%", "."};

// A group base, a plus and a number: ITEM_POTION + 100.
constexpr size_t RawIdTokens = 3;
// The lengths an item operand can have, the longest first.
constexpr std::array<size_t, 2> OperandLengths = {RawIdTokens, 1};

template <size_t Count> bool IsOneOf(std::string_view token, const std::array<std::string_view, Count>& tokens)
{
    return std::find(tokens.begin(), tokens.end(), token) != tokens.end();
}

// "ITEM_" or "MODEL_" when the identifier begins with it, else empty.
std::string_view PrefixOf(std::string_view identifier)
{
    for (const std::string_view prefix : ConstantPrefixes)
    {
        if (identifier.substr(0, prefix.size()) == prefix)
            return prefix;
    }
    return {};
}

// The group whose base constant the identifier is.
std::optional<int> GroupOfBase(std::string_view identifier)
{
    const std::string_view prefix = PrefixOf(identifier);
    if (prefix.empty())
        return std::nullopt;
    const std::string_view name = identifier.substr(prefix.size());
    const auto found = std::find_if(GroupBaseNames.begin(), GroupBaseNames.end(),
                                    [name](const GroupBaseName& base) { return base.name == name; });
    if (found == GroupBaseNames.end())
        return std::nullopt;
    return found->group;
}

// The number of a raw id: decimal (a leading 0 would make it octal) and below
// MAX_ITEM_INDEX.
std::optional<int> ItemNumberOf(std::string_view token)
{
    const bool octal = token.size() > 1 && token.front() == '0';
    if (token.empty() || octal)
        return std::nullopt;
    int number = 0;
    const auto [end, error] = std::from_chars(token.data(), token.data() + token.size(), number);
    if (error != std::errc() || end != token.data() + token.size() || number < 0 || number >= MAX_ITEM_INDEX)
        return std::nullopt;
    return number;
}

// How tokens stand for an item.
enum class Form
{
    Name,
    RawId,
    GroupBase,
};

// An item that tokens stand for: its constant, its group base plus a number,
// or a group base alone, which is the group's first item type.
struct ItemOperand
{
    int itemType = 0;
    Form form = Form::Name;
    size_t tokenCount = 1;
};

// The bound an item sets on a variable it is compared with: `lower` for
// variable > item (or item < variable), `inclusive` for >= and <=. A group
// base only bounds.
CodeRanges::Bound MakeBound(std::string variable, bool lower, bool inclusive, const ItemOperand& item)
{
    return CodeRanges::MakeBound(std::move(variable), lower, inclusive, item.itemType, item.form == Form::GroupBase);
}

// One line of blanked code as tokens, read for the items it names.
class CodeLine
{
public:
    CodeLine(std::string_view code, const ItemConstants& constants) : m_line(code), m_constants(constants) {}

    std::vector<ItemMention> Mentions() const
    {
        std::vector<ItemMention> mentions;
        AddNamed(mentions);
        AddRanges(mentions);
        // One mention per item, the first kind that fits.
        std::sort(mentions.begin(), mentions.end(), [](const ItemMention& left, const ItemMention& right)
                  { return std::tie(left.number, left.kind) < std::tie(right.number, right.kind); });
        const auto sameItem = [](const ItemMention& left, const ItemMention& right)
        { return left.number == right.number; };
        mentions.erase(std::unique(mentions.begin(), mentions.end(), sameItem), mentions.end());
        return mentions;
    }

private:
    // The constants and raw ids, wherever they stand.
    void AddNamed(std::vector<ItemMention>& mentions) const
    {
        for (size_t at = 0; at < m_line.Size(); ++at)
        {
            const std::optional<ItemOperand> operand = OperandAt(at);
            if (operand && operand->form != Form::GroupBase)
                mentions.push_back({operand->itemType, operand->form == Form::Name ? Kind::Name : Kind::RawId});
        }
    }

    void AddRanges(std::vector<ItemMention>& mentions) const
    {
        std::vector<CodeRanges::Bound> bounds;
        for (size_t at = 0; at < m_line.Size(); ++at)
        {
            if (!CodeTokenLine::IsOrderComparison(m_line.At(at)))
                continue;
            if (std::optional<CodeRanges::Bound> bound = BoundAt(at))
                bounds.push_back(std::move(*bound));
        }
        for (const CodeRanges::Range& range : CodeRanges::RangesOf(bounds, MAX_ITEM))
        {
            for (int itemType = range.first; itemType <= range.last; ++itemType)
                mentions.push_back({itemType, Kind::Range});
        }
    }

    std::optional<ItemOperand> OperandAt(size_t at) const
    {
        const std::string_view token = m_line.At(at);
        if (const auto constant = m_constants.find(token); constant != m_constants.end())
            return ItemOperand{constant->second, Form::Name, 1};
        const std::optional<int> group = GroupOfBase(token);
        if (!group)
            return std::nullopt;
        const std::optional<int> number = m_line.At(at + 1) == "+" ? ItemNumberOf(m_line.At(at + 2)) : std::nullopt;
        if (number && !IsOneOf(m_line.At(at + RawIdTokens), NumberContinuations))
            return ItemOperand{Data::Items::MakeItemType(*group, *number), Form::RawId, RawIdTokens};
        return ItemOperand{Data::Items::MakeItemType(*group, 0), Form::GroupBase, 1};
    }

    // The item operand right of a comparison, starting at `begin`, in any
    // number of parentheses; none when the operand holds anything else.
    std::optional<ItemOperand> OperandAfter(size_t begin) const
    {
        const size_t parentheses = m_line.CountOpeningsFrom(begin);
        const size_t at = begin + parentheses;
        const std::optional<ItemOperand> operand = OperandAt(at);
        if (!operand || !m_line.EndsOperandAt(at + operand->tokenCount, parentheses))
            return std::nullopt;
        return operand;
    }

    // The item operand whose last token comes right before `end`.
    std::optional<ItemOperand> OperandEndingAt(size_t end) const
    {
        for (const size_t length : OperandLengths)
        {
            const std::optional<ItemOperand> operand = end >= length ? OperandAt(end - length) : std::nullopt;
            if (operand && operand->tokenCount == length)
                return operand;
        }
        return std::nullopt;
    }

    // The item operand left of a comparison, ending right before `end`, in
    // any number of parentheses; none when the operand holds anything else.
    std::optional<ItemOperand> OperandBefore(size_t end) const
    {
        const size_t parentheses = m_line.CountClosingsBefore(end);
        const size_t at = end - parentheses;
        const std::optional<ItemOperand> operand = OperandEndingAt(at);
        if (!operand || !m_line.StartsOperandAt(at - operand->tokenCount, parentheses))
            return std::nullopt;
        return operand;
    }

    // The bound the comparison at `comparison` sets, when it compares a
    // variable with an item, on either side.
    std::optional<CodeRanges::Bound> BoundAt(size_t comparison) const
    {
        const std::string_view op = m_line.At(comparison);
        const bool greater = op.front() == '>';
        const bool inclusive = op.size() > 1;
        if (const std::optional<ItemOperand> item = OperandAfter(comparison + 1))
        {
            const size_t start = m_line.VariableStartBefore(comparison);
            if (start == CodeTokenLine::NoToken)
                return std::nullopt;
            return MakeBound(m_line.Joined(start, comparison), greater, inclusive, *item);
        }
        const std::optional<ItemOperand> item = OperandBefore(comparison);
        if (!item)
            return std::nullopt;
        const size_t end = m_line.VariableEndAfter(comparison + 1);
        if (end == CodeTokenLine::NoToken)
            return std::nullopt;
        // item > variable bounds the variable from above.
        return MakeBound(m_line.Joined(comparison + 1, end), !greater, inclusive, *item);
    }

    CodeTokenLine m_line;
    const ItemConstants& m_constants;
};

bool MentionsConstant(std::string_view code)
{
    return std::any_of(ConstantPrefixes.begin(), ConstantPrefixes.end(),
                       [code](std::string_view prefix) { return code.find(prefix) != std::string_view::npos; });
}
} // namespace

std::optional<std::pair<std::string_view, int>> ReadItemConstant(std::string_view code)
{
    // NAME = BASE + N, the comma left out after the last one.
    constexpr size_t ConstantTokens = 5;
    if (!MentionsConstant(code))
        return std::nullopt;
    const CodeTokenLine tokens(code);
    const bool comma = tokens.Size() == ConstantTokens + 1 && tokens.At(ConstantTokens) == ",";
    if ((tokens.Size() != ConstantTokens && !comma) || tokens.At(1) != "=" || tokens.At(3) != "+")
        return std::nullopt;
    const std::string_view name = tokens.At(0);
    const std::optional<int> group = GroupOfBase(tokens.At(2));
    const std::optional<int> number = ItemNumberOf(tokens.At(4));
    if (!group || !number || PrefixOf(name) != PrefixOf(tokens.At(2)))
        return std::nullopt;
    return std::pair{name, Data::Items::MakeItemType(*group, *number)};
}

std::vector<ItemMention> ReadItemMentions(std::string_view code, const ItemConstants& constants)
{
    if (!MentionsConstant(code))
        return {};
    return CodeLine(code, constants).Mentions();
}
} // namespace MuEditor::Items

#endif // _EDITOR
