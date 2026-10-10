#include "EffectSourceCondition.h"

#include <cctype>
#include <charconv>

namespace EffectSourceCases
{
namespace
{
class ConditionReader
{
public:
    ConditionReader(std::string_view text, int type, int subType, const NameValue& nameValue)
        : m_text(text), m_type(type), m_subType(subType), m_nameValue(nameValue)
    {
    }

    ConditionValue Read()
    {
        m_result.value = Or() != 0;
        if (m_pos != m_text.size())
            m_result.problems.push_back("cannot read: " + std::string(m_text.substr(m_pos)));
        return m_result;
    }

private:
    bool Take(std::string_view token)
    {
        if (m_text.compare(m_pos, token.size(), token) != 0)
            return false;
        m_pos += token.size();
        return true;
    }

    // Both sides are always read, so the position stays right.
    int Or()
    {
        int value = And();
        while (Take("||"))
        {
            const int right = And();
            value = value != 0 || right != 0;
        }
        return value;
    }

    int And()
    {
        int value = Comparison();
        while (Take("&&"))
        {
            const int right = Comparison();
            value = value != 0 && right != 0;
        }
        return value;
    }

    // The two-character operators first, so "<=" is not read as "<".
    int Comparison()
    {
        const int left = Sum();
        if (Take("=="))
            return left == Sum();
        if (Take("!="))
            return left != Sum();
        if (Take("<="))
            return left <= Sum();
        if (Take(">="))
            return left >= Sum();
        if (Take("<"))
            return left < Sum();
        if (Take(">"))
            return left > Sum();
        return left;
    }

    int Sum()
    {
        int value = Operand();
        while (Take("+"))
            value += Operand();
        return value;
    }

    int Operand()
    {
        if (Take("("))
        {
            const int value = Or();
            if (!Take(")"))
                m_result.problems.push_back("no closing parenthesis: " + std::string(m_text));
            return value;
        }
        if (Take("o->Type"))
            return m_type;
        if (Take("o->SubType"))
            return m_subType;
        const size_t start = m_pos;
        while (m_pos < m_text.size() &&
               (std::isalnum(static_cast<unsigned char>(m_text[m_pos])) != 0 || m_text[m_pos] == '_'))
            ++m_pos;
        return ValueOf(m_text.substr(start, m_pos - start));
    }

    int ValueOf(std::string_view word)
    {
        int number = 0;
        const auto [end, error] = std::from_chars(word.data(), word.data() + word.size(), number);
        if (!word.empty() && error == std::errc() && end == word.data() + word.size())
        {
            m_result.numbers.insert(number);
            return number;
        }
        const std::optional<int> value = word.empty() ? std::nullopt : m_nameValue(word);
        if (!value)
            m_result.problems.push_back("unknown name: " + std::string(word.empty() ? m_text.substr(m_pos) : word));
        return value.value_or(-1);
    }

    std::string_view m_text;
    int m_type;
    int m_subType;
    const NameValue& m_nameValue;
    size_t m_pos = 0;
    ConditionValue m_result;
};
} // namespace

ConditionValue EvaluateCondition(std::string_view condition, int type, int subType, const NameValue& nameValue)
{
    return ConditionReader(condition, type, subType, nameValue).Read();
}
} // namespace EffectSourceCases
