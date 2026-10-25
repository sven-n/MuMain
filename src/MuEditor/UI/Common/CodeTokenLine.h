#pragma once

#ifdef _EDITOR

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace MuEditor
{
// One line of C++ code, with its comments and string literals blanked
// (SourceText::Blank), as tokens: names, numbers and operators, an operator of
// more than one character kept whole (">=" is not ">" and "="). The tools that
// look for item and monster numbers in the sources read the comparisons in it:
// what a variable is compared with.
class CodeTokenLine
{
public:
    static constexpr size_t NoToken = static_cast<size_t>(-1);

    explicit CodeTokenLine(std::string_view code);

    size_t Size() const
    {
        return m_tokens.size();
    }
    // The token at `at`; empty past the end.
    std::string_view At(size_t at) const;

    // A name: not a number.
    static bool IsIdentifier(std::string_view token);
    // < > <= >=, which bound what they compare.
    static bool IsOrderComparison(std::string_view token);
    // Those, == and !=.
    static bool IsComparison(std::string_view token);

    // An operand of a comparison may stand in parentheses: the "(" from `at`
    // on, and the ")" right before `end`.
    size_t CountOpeningsFrom(size_t at) const;
    size_t CountClosingsBefore(size_t end) const;
    // Whether `closings` ")" follow from `at` on, and then the end of a
    // comparison's operand: an operator that binds less tightly, the end of a
    // bracket or statement, or the end of the line.
    bool EndsOperandAt(size_t at, size_t closings) const;
    // Whether `openings` "(" stand right before `start`, and before them what
    // may stand before a comparison's operand.
    bool StartsOperandAt(size_t start, size_t openings) const;

    // A variable compared with something: a name with its members, indices
    // and calls (c->Weapon[0].Type). Where the variable that ends right before
    // `end` starts, and where the one that starts at `begin` ends; NoToken when
    // the operand there is anything else.
    size_t VariableStartBefore(size_t end) const;
    size_t VariableEndAfter(size_t begin) const;

    // The tokens from `begin` to before `end`, without spaces: a variable reads
    // the same however it is spaced.
    std::string Joined(size_t begin, size_t end) const;

private:
    // The ( or [ that the ) or ] at `close` closes, and the other way round.
    size_t OpeningOf(size_t close) const;
    size_t ClosingOf(size_t open) const;

    std::vector<std::string_view> m_tokens;
};
} // namespace MuEditor

#endif // _EDITOR
