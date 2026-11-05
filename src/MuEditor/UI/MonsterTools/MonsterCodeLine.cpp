#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterCodeLine.h"

#include "../MuEditor/UI/Common/CodeRanges.h"
#include "../MuEditor/UI/Common/CodeTokenLine.h"
#include "Data/GameData/MonsterData/MonsterDefinition.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <system_error>
#include <tuple>

namespace MuEditor::Monsters
{
namespace
{
using Kind = CodeReference::Kind;

constexpr std::string_view ConstantPrefix = "MONSTER_";
// The constants of the monster models (enum EMonsterModelType) name models,
// not monster numbers.
constexpr std::string_view ModelConstantPrefix = "MONSTER_MODEL_";
constexpr std::string_view MonsterIndexSuffix = "MonsterIndex";
constexpr std::array<std::string_view, 3> MarkerNames = {"MONSTER_UNDEFINED", "MONSTER_START", "MONSTER_END"};

// A monster number as code writes it: decimal (a leading 0 would make it
// octal), 0-1023.
std::optional<int> MonsterNumberOf(std::string_view token)
{
    const bool octal = token.size() > 1 && token.front() == '0';
    if (token.empty() || octal)
    {
        return std::nullopt;
    }
    int number = 0;
    const auto [end, error] = std::from_chars(token.data(), token.data() + token.size(), number);
    if (error != std::errc() || end != token.data() + token.size() || number < 0 ||
        number >= Data::Monsters::MonsterNumberCount)
    {
        return std::nullopt;
    }
    return number;
}

// The last name of a variable that holds a monster number: c->MonsterIndex,
// iMonsterIndex.
bool IsMonsterIndex(std::string_view token)
{
    return CodeTokenLine::IsIdentifier(token) && token.ends_with(MonsterIndexSuffix);
}

bool MentionsMonster(std::string_view code)
{
    return code.find(ConstantPrefix) != std::string_view::npos ||
           code.find(MonsterIndexSuffix) != std::string_view::npos;
}

// How a token stands for a monster number.
enum class Form
{
    Name,
    Marker,
    Number,
};

struct MonsterOperand
{
    int number = 0;
    Form form = Form::Name;
};

// A variable compared with a monster operand.
struct Comparison
{
    // As written, without spaces.
    std::string variable;
    // The variable stands left of the operator.
    bool variableLeft = true;
    MonsterOperand operand;
};

// The bound that the comparison sets on its variable: `op` is <, <=, > or >=.
CodeRanges::Bound BoundOf(std::string_view op, const Comparison& comparison)
{
    const bool greater = op.front() == '>';
    const bool inclusive = op.size() > 1;
    // value > variable bounds the variable from above.
    const bool lower = comparison.variableLeft ? greater : !greater;
    return CodeRanges::MakeBound(comparison.variable, lower, inclusive, comparison.operand.number,
                                 comparison.operand.form == Form::Marker);
}

// One line of blanked code as tokens, read for the monster numbers it names.
class CodeLine
{
public:
    CodeLine(std::string_view code, const MonsterConstants& constants) : m_line(code), m_constants(constants) {}

    std::vector<MonsterMention> Mentions() const
    {
        std::vector<MonsterMention> mentions;
        AddNames(mentions);
        AddComparisons(mentions);
        // One mention per number, the first kind that fits.
        std::sort(mentions.begin(), mentions.end(), [](const MonsterMention& left, const MonsterMention& right)
                  { return std::tie(left.number, left.kind) < std::tie(right.number, right.kind); });
        const auto sameNumber = [](const MonsterMention& left, const MonsterMention& right)
        { return left.number == right.number; };
        mentions.erase(std::unique(mentions.begin(), mentions.end(), sameNumber), mentions.end());
        return mentions;
    }

private:
    // The constants, wherever they stand.
    void AddNames(std::vector<MonsterMention>& mentions) const
    {
        for (size_t at = 0; at < m_line.Size(); ++at)
        {
            const auto constant = m_constants.find(m_line.At(at));
            if (constant != m_constants.end() && !constant->second.marker)
            {
                mentions.push_back({constant->second.number, Kind::Name});
            }
        }
    }

    // The numbers compared with a MonsterIndex, and the ranges.
    void AddComparisons(std::vector<MonsterMention>& mentions) const
    {
        std::vector<CodeRanges::Bound> bounds;
        for (size_t at = 0; at < m_line.Size(); ++at)
        {
            const std::string_view op = m_line.At(at);
            if (!CodeTokenLine::IsComparison(op))
            {
                continue;
            }
            const std::optional<Comparison> comparison = ComparisonAt(at);
            if (!comparison)
            {
                continue;
            }
            if (comparison->operand.form == Form::Number)
            {
                mentions.push_back({comparison->operand.number, Kind::RawId});
            }
            if (CodeTokenLine::IsOrderComparison(op))
            {
                bounds.push_back(BoundOf(op, *comparison));
            }
        }
        for (const CodeRanges::Range& range : CodeRanges::RangesOf(bounds, Data::Monsters::MonsterNumberCount))
        {
            for (int number = range.first; number <= range.last; ++number)
            {
                mentions.push_back({number, Kind::Range});
            }
        }
    }

    std::optional<MonsterOperand> OperandAt(size_t at) const
    {
        const std::string_view token = m_line.At(at);
        if (const auto constant = m_constants.find(token); constant != m_constants.end())
        {
            return MonsterOperand{constant->second.number, constant->second.marker ? Form::Marker : Form::Name};
        }
        if (const std::optional<int> number = MonsterNumberOf(token))
        {
            return MonsterOperand{*number, Form::Number};
        }
        return std::nullopt;
    }

    // The monster operand right of a comparison, starting at `begin`, in any
    // number of parentheses; none when the operand holds anything else.
    std::optional<MonsterOperand> OperandAfter(size_t begin) const
    {
        const size_t parentheses = m_line.CountOpeningsFrom(begin);
        const size_t at = begin + parentheses;
        const std::optional<MonsterOperand> operand = OperandAt(at);
        if (!operand || !m_line.EndsOperandAt(at + 1, parentheses))
        {
            return std::nullopt;
        }
        return operand;
    }

    // The monster operand left of a comparison, ending right before `end`, in
    // any number of parentheses; none when the operand holds anything else.
    std::optional<MonsterOperand> OperandBefore(size_t end) const
    {
        const size_t parentheses = m_line.CountClosingsBefore(end);
        const size_t at = end - parentheses;
        const std::optional<MonsterOperand> operand = at > 0 ? OperandAt(at - 1) : std::nullopt;
        if (!operand || !m_line.StartsOperandAt(at - 1, parentheses))
        {
            return std::nullopt;
        }
        return operand;
    }

    // The comparison at `comparison`, when it compares a variable with a
    // monster operand on either side. A number counts only when the variable's
    // last name ends in MonsterIndex: other numbers are anything.
    std::optional<Comparison> ComparisonAt(size_t comparison) const
    {
        if (const std::optional<MonsterOperand> operand = OperandAfter(comparison + 1))
        {
            const size_t start = m_line.VariableStartBefore(comparison);
            if (start == CodeTokenLine::NoToken || !Accepts(*operand, m_line.At(comparison - 1)))
            {
                return std::nullopt;
            }
            return Comparison{m_line.Joined(start, comparison), true, *operand};
        }
        const std::optional<MonsterOperand> operand = OperandBefore(comparison);
        if (!operand)
        {
            return std::nullopt;
        }
        const size_t end = m_line.VariableEndAfter(comparison + 1);
        if (end == CodeTokenLine::NoToken || !Accepts(*operand, m_line.At(end - 1)))
        {
            return std::nullopt;
        }
        return Comparison{m_line.Joined(comparison + 1, end), false, *operand};
    }

    static bool Accepts(const MonsterOperand& operand, std::string_view variableEnd)
    {
        return operand.form != Form::Number || IsMonsterIndex(variableEnd);
    }

    CodeTokenLine m_line;
    const MonsterConstants& m_constants;
};
} // namespace

std::optional<std::pair<std::string_view, int>> ReadMonsterConstant(std::string_view code)
{
    // NAME = N, the comma left out after the last one.
    constexpr size_t ConstantTokens = 3;
    if (code.find(ConstantPrefix) == std::string_view::npos)
    {
        return std::nullopt;
    }
    const CodeTokenLine tokens(code);
    const bool comma = tokens.Size() == ConstantTokens + 1 && tokens.At(ConstantTokens) == ",";
    if ((tokens.Size() != ConstantTokens && !comma) || tokens.At(1) != "=")
    {
        return std::nullopt;
    }
    const std::string_view name = tokens.At(0);
    const std::optional<int> number = MonsterNumberOf(tokens.At(2));
    if (!name.starts_with(ConstantPrefix) || name.starts_with(ModelConstantPrefix) || !number)
    {
        return std::nullopt;
    }
    return std::pair{name, *number};
}

bool IsMonsterMarker(std::string_view name)
{
    return std::find(MarkerNames.begin(), MarkerNames.end(), name) != MarkerNames.end();
}

std::vector<MonsterMention> ReadMonsterMentions(std::string_view code, const MonsterConstants& constants)
{
    if (!MentionsMonster(code))
    {
        return {};
    }
    return CodeLine(code, constants).Mentions();
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
