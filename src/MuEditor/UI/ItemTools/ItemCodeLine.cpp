#include "stdafx.h"

#ifdef _EDITOR

#include "ItemCodeLine.h"

#include "Data/GameData/ItemData/ItemType.h"
#include "UI/Common/SourceText.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <charconv>
#include <cstddef>
#include <system_error>
#include <tuple>

namespace MuEditor::Items
{
namespace
{
using Kind = ItemCodeReference::Kind;
using Tokens = std::vector<std::string_view>;

constexpr size_t NoToken = static_cast<size_t>(-1);

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

// The operators of more than one character, the longest first; a token keeps
// them whole, so ">=" is not read as ">".
constexpr std::array<std::string_view, 22> LongOperators = {
    "<<=", ">>=", "->", "::", "<<", ">>", "<=", ">=", "==", "!=", "&&",
    "||",  "++",  "--", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^="};

constexpr std::array<std::string_view, 4> Comparisons = {"<", ">", "<=", ">="};
constexpr std::array<std::string_view, 3> MemberAccess = {".", "->", "::"};

// What may follow an operand of a comparison: an operator that binds less
// tightly, or the end of a bracket or statement.
constexpr std::array<std::string_view, 14> OperandEnds = {")", "]", "}", ";", ",", "&&", "||",
                                                          "&", "|", "^", "?", ":", "==", "!="};

// What may stand before an operand of a comparison.
constexpr std::array<std::string_view, 27> OperandStarts = {"(",  "[",  "{",  "}",  ";",  ",",  "&&",  "||",  "&",
                                                            "|",  "^",  "?",  ":",  "==", "!=", "=",   "+=",  "-=",
                                                            "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>=", "return"};

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

bool IsIdentifier(std::string_view token)
{
    return !token.empty() && SourceText::IsIdentifierChar(token.front()) &&
           std::isdigit(static_cast<unsigned char>(token.front())) == 0;
}

bool IsOpening(std::string_view token)
{
    return token == "(" || token == "[";
}

bool IsClosing(std::string_view token)
{
    return token == ")" || token == "]";
}

size_t TokenLength(std::string_view code, size_t at)
{
    if (SourceText::IsIdentifierChar(code[at]))
    {
        size_t end = at;
        while (end < code.size() && SourceText::IsIdentifierChar(code[end]))
            ++end;
        return end - at;
    }
    for (const std::string_view op : LongOperators)
    {
        if (code.compare(at, op.size(), op) == 0)
            return op.size();
    }
    return 1;
}

// The names, numbers and operators of a line of blanked code.
Tokens Tokenize(std::string_view code)
{
    Tokens tokens;
    for (size_t at = 0; at < code.size();)
    {
        if (std::isspace(static_cast<unsigned char>(code[at])) != 0)
        {
            ++at;
            continue;
        }
        const size_t length = TokenLength(code, at);
        tokens.push_back(code.substr(at, length));
        at += length;
    }
    return tokens;
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

// A comparison of a variable with an item: the first (lower) or last (upper)
// item type the variable may have.
struct Bound
{
    // As written, without spaces.
    std::string variable;
    bool lower = false;
    int itemType = 0;
    bool groupBase = false;
};

// The bound an item sets on a variable it is compared with: `lower` for
// variable > item (or item < variable), `inclusive` for >= and <=.
Bound MakeBound(std::string variable, bool lower, bool inclusive, const ItemOperand& item)
{
    const int step = inclusive ? 0 : 1;
    const int itemType = lower ? item.itemType + step : item.itemType - step;
    return {std::move(variable), lower, itemType, item.form == Form::GroupBase};
}

// The first and last item type of a range, both inside it.
struct ItemRange
{
    int first = 0;
    int last = 0;
};

// The ranges a line's bounds set: a lower and an upper bound of the same
// variable in a row, in either order. Between two group bases the range tests
// whole groups and names no item.
std::vector<ItemRange> RangesOf(const std::vector<Bound>& bounds)
{
    std::vector<ItemRange> ranges;
    std::vector<const Bound*> open;
    for (const Bound& bound : bounds)
    {
        const auto pending = std::find_if(open.begin(), open.end(),
                                          [&bound](const Bound* other) { return other->variable == bound.variable; });
        if (pending == open.end())
        {
            open.push_back(&bound);
            continue;
        }
        if ((*pending)->lower == bound.lower)
        {
            *pending = &bound;
            continue;
        }
        const Bound& lower = bound.lower ? bound : **pending;
        const Bound& upper = bound.lower ? **pending : bound;
        open.erase(pending);
        const bool wholeGroups = lower.groupBase && upper.groupBase;
        const int first = std::max(lower.itemType, 0);
        const int last = std::min(upper.itemType, MAX_ITEM - 1);
        if (!wholeGroups && first <= last)
            ranges.push_back({first, last});
    }
    return ranges;
}

// One line of blanked code as tokens, read for the items it names.
class CodeLine
{
public:
    CodeLine(std::string_view code, const ItemConstants& constants) : m_tokens(Tokenize(code)), m_constants(constants)
    {
    }

    std::vector<ItemMention> Mentions() const
    {
        std::vector<ItemMention> mentions;
        AddNamed(mentions);
        AddRanges(mentions);
        // One mention per item, the first kind that fits.
        std::sort(mentions.begin(), mentions.end(), [](const ItemMention& left, const ItemMention& right)
                  { return std::tie(left.itemType, left.kind) < std::tie(right.itemType, right.kind); });
        const auto sameItem = [](const ItemMention& left, const ItemMention& right)
        { return left.itemType == right.itemType; };
        mentions.erase(std::unique(mentions.begin(), mentions.end(), sameItem), mentions.end());
        return mentions;
    }

private:
    std::string_view TokenAt(size_t at) const
    {
        return at < m_tokens.size() ? m_tokens[at] : std::string_view();
    }

    // The constants and raw ids, wherever they stand.
    void AddNamed(std::vector<ItemMention>& mentions) const
    {
        for (size_t at = 0; at < m_tokens.size(); ++at)
        {
            const std::optional<ItemOperand> operand = OperandAt(at);
            if (operand && operand->form != Form::GroupBase)
                mentions.push_back({operand->itemType, operand->form == Form::Name ? Kind::Name : Kind::RawId});
        }
    }

    void AddRanges(std::vector<ItemMention>& mentions) const
    {
        std::vector<Bound> bounds;
        for (size_t at = 0; at < m_tokens.size(); ++at)
        {
            if (!IsOneOf(m_tokens[at], Comparisons))
                continue;
            if (std::optional<Bound> bound = BoundAt(at))
                bounds.push_back(std::move(*bound));
        }
        for (const ItemRange& range : RangesOf(bounds))
        {
            for (int itemType = range.first; itemType <= range.last; ++itemType)
                mentions.push_back({itemType, Kind::Range});
        }
    }

    std::optional<ItemOperand> OperandAt(size_t at) const
    {
        const std::string_view token = TokenAt(at);
        if (const auto constant = m_constants.find(token); constant != m_constants.end())
            return ItemOperand{constant->second, Form::Name, 1};
        const std::optional<int> group = GroupOfBase(token);
        if (!group)
            return std::nullopt;
        const std::optional<int> number = TokenAt(at + 1) == "+" ? ItemNumberOf(TokenAt(at + 2)) : std::nullopt;
        if (number && !IsOneOf(TokenAt(at + RawIdTokens), NumberContinuations))
            return ItemOperand{Data::Items::MakeItemType(*group, *number), Form::RawId, RawIdTokens};
        return ItemOperand{Data::Items::MakeItemType(*group, 0), Form::GroupBase, 1};
    }

    // The item operand right of a comparison, starting at `begin`, in any
    // number of parentheses; none when the operand holds anything else.
    std::optional<ItemOperand> OperandAfter(size_t begin) const
    {
        size_t at = begin;
        size_t parentheses = 0;
        for (; TokenAt(at) == "("; ++at)
            ++parentheses;
        const std::optional<ItemOperand> operand = OperandAt(at);
        if (!operand)
            return std::nullopt;
        at += operand->tokenCount;
        for (; parentheses > 0; --parentheses, ++at)
        {
            if (TokenAt(at) != ")")
                return std::nullopt;
        }
        if (at < m_tokens.size() && !IsOneOf(m_tokens[at], OperandEnds))
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
        size_t at = end;
        size_t parentheses = 0;
        for (; at > 0 && m_tokens[at - 1] == ")"; --at)
            ++parentheses;
        const std::optional<ItemOperand> operand = OperandEndingAt(at);
        if (!operand)
            return std::nullopt;
        at -= operand->tokenCount;
        for (; parentheses > 0; --parentheses, --at)
        {
            if (at == 0 || m_tokens[at - 1] != "(")
                return std::nullopt;
        }
        if (at > 0 && !IsOneOf(m_tokens[at - 1], OperandStarts))
            return std::nullopt;
        return operand;
    }

    // The ( or [ that the ) or ] at `close` closes.
    size_t OpeningOf(size_t close) const
    {
        int depth = 0;
        for (size_t at = close + 1; at > 0; --at)
        {
            const std::string_view token = m_tokens[at - 1];
            if (IsClosing(token))
                ++depth;
            else if (IsOpening(token) && --depth == 0)
                return at - 1;
        }
        return NoToken;
    }

    // The ) or ] that closes the ( or [ at `open`.
    size_t ClosingOf(size_t open) const
    {
        int depth = 0;
        for (size_t at = open; at < m_tokens.size(); ++at)
        {
            const std::string_view token = m_tokens[at];
            if (IsOpening(token))
                ++depth;
            else if (IsClosing(token) && --depth == 0)
                return at;
        }
        return NoToken;
    }

    // Where the variable left of a comparison starts, which ends right before
    // `end`: a name with its members, indices and calls (c->Weapon[0].Type).
    // NoToken when the operand is anything else.
    size_t VariableStartBefore(size_t end) const
    {
        size_t at = end;
        for (;;)
        {
            while (at > 0 && IsClosing(m_tokens[at - 1]))
            {
                at = OpeningOf(at - 1);
                if (at == NoToken)
                    return NoToken;
            }
            if (at == 0 || !IsIdentifier(m_tokens[at - 1]))
                return NoToken;
            --at;
            if (at == 0 || !IsOneOf(m_tokens[at - 1], MemberAccess))
                break;
            --at;
        }
        if (at > 0 && !IsOneOf(m_tokens[at - 1], OperandStarts))
            return NoToken;
        return at;
    }

    // Where the variable right of a comparison ends, which starts at `begin`;
    // NoToken when the operand is anything else.
    size_t VariableEndAfter(size_t begin) const
    {
        size_t at = begin;
        for (;;)
        {
            if (!IsIdentifier(TokenAt(at)))
                return NoToken;
            ++at;
            while (IsOpening(TokenAt(at)))
            {
                const size_t close = ClosingOf(at);
                if (close == NoToken)
                    return NoToken;
                at = close + 1;
            }
            if (!IsOneOf(TokenAt(at), MemberAccess))
                break;
            ++at;
        }
        if (at < m_tokens.size() && !IsOneOf(m_tokens[at], OperandEnds))
            return NoToken;
        return at;
    }

    std::string Joined(size_t begin, size_t end) const
    {
        std::string text;
        for (size_t at = begin; at < end; ++at)
            text += m_tokens[at];
        return text;
    }

    // The bound the comparison at `comparison` sets, when it compares a
    // variable with an item, on either side.
    std::optional<Bound> BoundAt(size_t comparison) const
    {
        const std::string_view op = m_tokens[comparison];
        const bool greater = op.front() == '>';
        const bool inclusive = op.size() > 1;
        if (const std::optional<ItemOperand> item = OperandAfter(comparison + 1))
        {
            const size_t start = VariableStartBefore(comparison);
            if (start == NoToken)
                return std::nullopt;
            return MakeBound(Joined(start, comparison), greater, inclusive, *item);
        }
        const std::optional<ItemOperand> item = OperandBefore(comparison);
        if (!item)
            return std::nullopt;
        const size_t end = VariableEndAfter(comparison + 1);
        if (end == NoToken)
            return std::nullopt;
        // item > variable bounds the variable from above.
        return MakeBound(Joined(comparison + 1, end), !greater, inclusive, *item);
    }

    Tokens m_tokens;
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
    const Tokens tokens = Tokenize(code);
    const bool comma = tokens.size() == ConstantTokens + 1 && tokens.back() == ",";
    if ((tokens.size() != ConstantTokens && !comma) || tokens[1] != "=" || tokens[3] != "+")
        return std::nullopt;
    const std::string_view name = tokens[0];
    const std::optional<int> group = GroupOfBase(tokens[2]);
    const std::optional<int> number = ItemNumberOf(tokens[4]);
    if (!group || !number || PrefixOf(name) != PrefixOf(tokens[2]))
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
