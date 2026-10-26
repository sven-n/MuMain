#include "stdafx.h"

#ifdef _EDITOR

#include "CodeTokenLine.h"

#include "SourceText.h"

#include <algorithm>
#include <array>
#include <cctype>

namespace MuEditor
{
namespace
{
// The operators of more than one character, the longest first; a token keeps
// them whole, so ">=" is not read as ">".
constexpr std::array<std::string_view, 22> LongOperators = {
    "<<=", ">>=", "->", "::", "<<", ">>", "<=", ">=", "==", "!=", "&&",
    "||",  "++",  "--", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^="};

constexpr std::array<std::string_view, 4> OrderComparisons = {"<", ">", "<=", ">="};
constexpr std::array<std::string_view, 2> EqualityComparisons = {"==", "!="};
constexpr std::array<std::string_view, 3> MemberAccess = {".", "->", "::"};

// What may follow an operand of a comparison: an operator that binds less
// tightly, or the end of a bracket or statement.
constexpr std::array<std::string_view, 14> OperandEnds = {")", "]", "}", ";", ",", "&&", "||",
                                                          "&", "|", "^", "?", ":", "==", "!="};

// What may stand before an operand of a comparison.
constexpr std::array<std::string_view, 27> OperandStarts = {"(",  "[",  "{",  "}",  ";",  ",",  "&&",  "||",  "&",
                                                            "|",  "^",  "?",  ":",  "==", "!=", "=",   "+=",  "-=",
                                                            "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>=", "return"};

template <size_t Count> bool IsOneOf(std::string_view token, const std::array<std::string_view, Count>& tokens)
{
    return std::find(tokens.begin(), tokens.end(), token) != tokens.end();
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
        {
            ++end;
        }
        return end - at;
    }
    for (const std::string_view op : LongOperators)
    {
        if (code.compare(at, op.size(), op) == 0)
        {
            return op.size();
        }
    }
    return 1;
}
} // namespace

CodeTokenLine::CodeTokenLine(std::string_view code)
{
    for (size_t at = 0; at < code.size();)
    {
        if (std::isspace(static_cast<unsigned char>(code[at])) != 0)
        {
            ++at;
            continue;
        }
        const size_t length = TokenLength(code, at);
        m_tokens.push_back(code.substr(at, length));
        at += length;
    }
}

std::string_view CodeTokenLine::At(size_t at) const
{
    return at < m_tokens.size() ? m_tokens[at] : std::string_view();
}

bool CodeTokenLine::IsIdentifier(std::string_view token)
{
    return !token.empty() && SourceText::IsIdentifierChar(token.front()) &&
           std::isdigit(static_cast<unsigned char>(token.front())) == 0;
}

bool CodeTokenLine::IsOrderComparison(std::string_view token)
{
    return IsOneOf(token, OrderComparisons);
}

bool CodeTokenLine::IsComparison(std::string_view token)
{
    return IsOrderComparison(token) || IsOneOf(token, EqualityComparisons);
}

size_t CodeTokenLine::CountOpeningsFrom(size_t at) const
{
    size_t openings = 0;
    for (; At(at) == "("; ++at)
    {
        ++openings;
    }
    return openings;
}

size_t CodeTokenLine::CountClosingsBefore(size_t end) const
{
    size_t closings = 0;
    for (size_t at = end; at > 0 && m_tokens[at - 1] == ")"; --at)
    {
        ++closings;
    }
    return closings;
}

bool CodeTokenLine::EndsOperandAt(size_t at, size_t closings) const
{
    for (; closings > 0; --closings, ++at)
    {
        if (At(at) != ")")
        {
            return false;
        }
    }
    return at >= m_tokens.size() || IsOneOf(m_tokens[at], OperandEnds);
}

bool CodeTokenLine::StartsOperandAt(size_t start, size_t openings) const
{
    size_t at = start;
    for (; openings > 0; --openings, --at)
    {
        if (at == 0 || m_tokens[at - 1] != "(")
        {
            return false;
        }
    }
    return at == 0 || IsOneOf(m_tokens[at - 1], OperandStarts);
}

size_t CodeTokenLine::OpeningOf(size_t close) const
{
    int depth = 0;
    for (size_t at = close + 1; at > 0; --at)
    {
        const std::string_view token = m_tokens[at - 1];
        if (IsClosing(token))
        {
            ++depth;
        }
        else if (IsOpening(token) && --depth == 0)
        {
            return at - 1;
        }
    }
    return NoToken;
}

size_t CodeTokenLine::ClosingOf(size_t open) const
{
    int depth = 0;
    for (size_t at = open; at < m_tokens.size(); ++at)
    {
        const std::string_view token = m_tokens[at];
        if (IsOpening(token))
        {
            ++depth;
        }
        else if (IsClosing(token) && --depth == 0)
        {
            return at;
        }
    }
    return NoToken;
}

size_t CodeTokenLine::VariableStartBefore(size_t end) const
{
    size_t at = end;
    for (;;)
    {
        while (at > 0 && IsClosing(m_tokens[at - 1]))
        {
            at = OpeningOf(at - 1);
            if (at == NoToken)
            {
                return NoToken;
            }
        }
        if (at == 0 || !IsIdentifier(m_tokens[at - 1]))
        {
            return NoToken;
        }
        --at;
        if (at == 0 || !IsOneOf(m_tokens[at - 1], MemberAccess))
        {
            break;
        }
        --at;
    }
    if (at > 0 && !IsOneOf(m_tokens[at - 1], OperandStarts))
    {
        return NoToken;
    }
    return at;
}

size_t CodeTokenLine::VariableEndAfter(size_t begin) const
{
    size_t at = begin;
    for (;;)
    {
        if (!IsIdentifier(At(at)))
        {
            return NoToken;
        }
        ++at;
        while (IsOpening(At(at)))
        {
            const size_t close = ClosingOf(at);
            if (close == NoToken)
            {
                return NoToken;
            }
            at = close + 1;
        }
        if (!IsOneOf(At(at), MemberAccess))
        {
            break;
        }
        ++at;
    }
    if (at < m_tokens.size() && !IsOneOf(m_tokens[at], OperandEnds))
    {
        return NoToken;
    }
    return at;
}

std::string CodeTokenLine::Joined(size_t begin, size_t end) const
{
    std::string text;
    for (size_t at = begin; at < end; ++at)
    {
        text += m_tokens[at];
    }
    return text;
}
} // namespace MuEditor

#endif // _EDITOR
