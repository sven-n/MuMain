#include "stdafx.h"

#ifdef _EDITOR

#include "EffectCallSites.h"

#include "Data/GameData/EffectData/EffectTypeSymbols.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <charconv>
#include <fstream>
#include <iterator>
#include <sstream>
#include <system_error>
#include <tuple>
#include <unordered_map>
#include <utility>

namespace MuEditor::Effects
{
namespace
{
using Data::Effects::EffectKind;

// What each create function takes, by argument index (-1: none). The light
// of lightning is its colour, which most calls leave out.
struct CallShape
{
    std::string_view name;
    EffectKind kind;
    int light;
    int subType;
    int owner;
    int scale;
    int pk;
    int skillIndex;
};

constexpr std::array<CallShape, 4> Shapes = {{
    {"CreateEffect", EffectKind::Effect, 3, 4, 5, 10, -1, -1},
    {"CreateParticle", EffectKind::Particle, 3, 4, 6, 5, -1, -1},
    {"CreateJoint", EffectKind::Joint, 11, 4, 5, 6, 7, 8},
    {"CreateSprite", EffectKind::Sprite, 3, 6, 4, 2, -1, -1},
}};

bool IsIdentifierChar(char c)
{
    return std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_';
}

std::string WithoutSpaces(std::string_view text)
{
    std::string result;
    std::copy_if(text.begin(), text.end(), std::back_inserter(result),
                 [](char c) { return std::isspace(static_cast<unsigned char>(c)) == 0; });
    return result;
}

// The text with comments and string and character literals blanked out;
// line breaks stay, so positions keep their lines.
std::string Blank(std::string_view source)
{
    std::string text(source);
    for (size_t i = 0; i < text.size();)
    {
        if (text.compare(i, 2, "//") == 0)
        {
            const size_t end = text.find('\n', i);
            std::fill(text.begin() + static_cast<std::ptrdiff_t>(i),
                      end == std::string::npos ? text.end() : text.begin() + static_cast<std::ptrdiff_t>(end), ' ');
            i = end == std::string::npos ? text.size() : end;
        }
        else if (text.compare(i, 2, "/*") == 0)
        {
            const size_t close = text.find("*/", i + 2);
            const size_t end = close == std::string::npos ? text.size() : close + 2;
            for (size_t k = i; k < end; ++k)
            {
                if (text[k] != '\n')
                    text[k] = ' ';
            }
            i = end;
        }
        else if (text[i] == '"' || text[i] == '\'')
        {
            const char quote = text[i];
            size_t k = i + 1;
            while (k < text.size() && text[k] != quote && text[k] != '\n')
                k += text[k] == '\\' ? 2 : 1;
            for (size_t j = i + 1; j < k && j < text.size(); ++j)
                text[j] = ' ';
            i = k + 1;
        }
        else
        {
            ++i;
        }
    }
    return text;
}

size_t SkipSpaces(std::string_view text, size_t at)
{
    while (at < text.size() && std::isspace(static_cast<unsigned char>(text[at])) != 0)
        ++at;
    return at;
}

// The identifier at `at`, empty when there is none.
std::string_view IdentifierAt(std::string_view text, size_t at)
{
    size_t end = at;
    while (end < text.size() && IsIdentifierChar(text[end]))
        ++end;
    return text.substr(at, end - at);
}

// A preprocessor line: its directive (ifdef, if, else, define, ...) and the
// rest of the line.
struct Directive
{
    std::string_view name;
    std::string_view rest;
};

std::optional<Directive> DirectiveOf(std::string_view line)
{
    size_t at = SkipSpaces(line, 0);
    if (at >= line.size() || line[at] != '#')
        return std::nullopt;
    at = SkipSpaces(line, at + 1);
    const std::string_view name = IdentifierAt(line, at);
    return Directive{name, line.substr(at + name.size())};
}

// The macro an #ifdef or an #if defined(X) tests; empty for any other
// condition.
std::string_view MacroTested(const Directive& directive)
{
    const std::string_view rest = directive.rest;
    if (directive.name == "ifdef" || directive.name == "ifndef")
        return IdentifierAt(rest, SkipSpaces(rest, 0));
    if (directive.name != "if" && directive.name != "elif")
        return {};
    const std::string compact = WithoutSpaces(rest);
    for (const std::string_view form : {std::string_view("defined("), std::string_view("defined")})
    {
        if (compact.rfind(form, 0) != 0)
            continue;
        const std::string_view name = IdentifierAt(compact, form.size());
        const size_t end = form.size() + name.size() + (form.back() == '(' ? 1 : 0);
        if (!name.empty() && end == compact.size())
            return rest.substr(rest.find(name), name.size());
    }
    return {};
}

// The text with the lines of #if 0 and of #ifdef or #if defined of a macro
// that is off blanked; their #else is kept. Other conditions are kept whole.
std::string WithoutBranchesOff(std::string text, const std::unordered_set<std::string>& macrosOff)
{
    // Per open #if, whether its current branch is off.
    std::vector<bool> open;
    const auto anyOff = [&] { return std::find(open.begin(), open.end(), true) != open.end(); };
    for (size_t start = 0; start < text.size();)
    {
        const size_t end = std::min(text.find('\n', start), text.size());
        const std::optional<Directive> directive = DirectiveOf(std::string_view(text).substr(start, end - start));
        bool blank = anyOff();
        if (directive && (directive->name == "if" || directive->name == "ifdef" || directive->name == "ifndef"))
        {
            const std::string_view macro = MacroTested(*directive);
            const bool knownFalse =
                (directive->name == "if" && WithoutSpaces(directive->rest) == "0") ||
                (directive->name != "ifndef" && !macro.empty() && macrosOff.count(std::string(macro)) != 0);
            open.push_back(knownFalse);
        }
        else if (directive && (directive->name == "else" || directive->name == "elif") && !open.empty())
        {
            open.back() = false;
            blank = anyOff();
        }
        else if (directive && directive->name == "endif" && !open.empty())
        {
            open.pop_back();
        }
        if (blank)
            std::fill(text.begin() + static_cast<std::ptrdiff_t>(start),
                      text.begin() + static_cast<std::ptrdiff_t>(end), ' ');
        start = end + 1;
    }
    return text;
}

std::optional<int> IntegerOf(std::string_view text)
{
    const std::string compact = WithoutSpaces(text);
    int value = 0;
    const auto [end, error] = std::from_chars(compact.data(), compact.data() + compact.size(), value);
    if (compact.empty() || error != std::errc() || end != compact.data() + compact.size())
        return std::nullopt;
    return value;
}

// A number as C++ writes it (1, -0.5, 1.f, .25f), whatever the locale.
std::optional<float> FloatOf(std::string_view text)
{
    std::string compact = WithoutSpaces(text);
    if (!compact.empty() && (compact.back() == 'f' || compact.back() == 'F'))
        compact.pop_back();
    size_t i = 0;
    const bool negative = i < compact.size() && compact[i] == '-';
    if (negative)
        ++i;
    double value = 0.0;
    bool digits = false;
    for (; i < compact.size() && std::isdigit(static_cast<unsigned char>(compact[i])) != 0; ++i, digits = true)
        value = value * 10.0 + (compact[i] - '0');
    if (i < compact.size() && compact[i] == '.')
    {
        double unit = 0.1;
        for (++i; i < compact.size() && std::isdigit(static_cast<unsigned char>(compact[i])) != 0; ++i, digits = true)
        {
            value += (compact[i] - '0') * unit;
            unit /= 10.0;
        }
    }
    if (!digits || i != compact.size())
        return std::nullopt;
    return static_cast<float>(negative ? -value : value);
}

// The number of each code of a kind's symbol list.
const std::unordered_map<std::string, int>& NumbersOf(EffectKind kind)
{
    static std::array<std::unordered_map<std::string, int>, Data::Effects::EffectKindCount> numbers;
    std::unordered_map<std::string, int>& codes = numbers[Data::Effects::ToIndex(kind)];
    if (codes.empty())
    {
        for (const Data::Effects::EffectTypeSymbol& symbol : Data::Effects::GetEffectTypeSymbols(kind))
            codes.emplace(std::string(symbol.code), symbol.type);
    }
    return codes;
}

// The types a call's first argument names: a code, a code plus a number, or
// a code plus rand() % N (any of N types).
std::vector<int> TypesOf(EffectKind kind, std::string_view argument)
{
    const std::unordered_map<std::string, int>& codes = NumbersOf(kind);
    const std::string compact = WithoutSpaces(argument);
    if (const auto found = codes.find(compact); found != codes.end())
        return {found->second};
    const auto codeAt = [&](std::string_view text) -> std::optional<int>
    {
        const auto found = codes.find(std::string(text));
        return found != codes.end() ? std::optional<int>(found->second) : std::nullopt;
    };
    for (const std::string_view random : {std::string_view("rand()%"), std::string_view("(rand()%")})
    {
        // NAME+rand()%N or NAME+(rand()%N)
        if (const size_t plus = compact.find("+" + std::string(random)); plus != std::string::npos)
        {
            const std::optional<int> base = codeAt(std::string_view(compact).substr(0, plus));
            std::string count = compact.substr(plus + 1 + random.size());
            if (!count.empty() && count.back() == ')' && random.front() == '(')
                count.pop_back();
            const std::optional<int> n = IntegerOf(count);
            if (base && n && *n > 0)
            {
                std::vector<int> types;
                for (int i = 0; i < *n; ++i)
                    types.push_back(*base + i);
                return types;
            }
        }
    }
    // rand()%N+NAME
    if (compact.rfind("rand()%", 0) == 0)
    {
        const size_t plus = compact.find('+');
        if (plus != std::string::npos)
        {
            const std::optional<int> n = IntegerOf(std::string_view(compact).substr(7, plus - 7));
            const std::optional<int> base = codeAt(std::string_view(compact).substr(plus + 1));
            if (base && n && *n > 0)
            {
                std::vector<int> types;
                for (int i = 0; i < *n; ++i)
                    types.push_back(*base + i);
                return types;
            }
        }
    }
    // NAME+N
    if (const size_t plus = compact.rfind('+'); plus != std::string::npos)
    {
        const std::optional<int> base = codeAt(std::string_view(compact).substr(0, plus));
        const std::optional<int> offset = IntegerOf(std::string_view(compact).substr(plus + 1));
        if (base && offset)
            return {*base + *offset};
    }
    return {};
}

// A use of a create function, not its declaration or definition, nor a
// member function of the same name.
bool IsCall(std::string_view text, size_t start)
{
    size_t k = start;
    while (k > 0 && (text[k - 1] == ' ' || text[k - 1] == '\t'))
        --k;
    if (k > 0 && (text[k - 1] == '.' || text[k - 1] == '>' || text[k - 1] == ':'))
        return false;
    size_t wordEnd = k;
    while (k > 0 && IsIdentifierChar(text[k - 1]))
        --k;
    const std::string_view before = text.substr(k, wordEnd - k);
    return before != "void" && before != "int" && before != "bool" && before != "BOOL";
}

// The arguments of the call whose '(' is at `open`, split at the top level.
std::vector<std::string_view> ArgumentsAt(std::string_view text, size_t open)
{
    std::vector<std::string_view> arguments;
    int depth = 0;
    size_t start = open + 1;
    for (size_t k = open; k < text.size(); ++k)
    {
        const char c = text[k];
        if (c == '(' || c == '[' || c == '{')
        {
            ++depth;
        }
        else if (c == ')' || c == ']' || c == '}')
        {
            if (--depth == 0)
            {
                arguments.push_back(text.substr(start, k - start));
                return arguments;
            }
        }
        else if (c == ',' && depth == 1)
        {
            arguments.push_back(text.substr(start, k - start));
            start = k + 1;
        }
    }
    return {};
}

std::string Trimmed(std::string_view text)
{
    const size_t first = text.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos)
        return {};
    const size_t last = text.find_last_not_of(" \t\r\n");
    return std::string(text.substr(first, last - first + 1));
}

bool IsWordAt(std::string_view text, size_t at, std::string_view word)
{
    return text.compare(at, word.size(), word) == 0 && (at == 0 || !IsIdentifierChar(text[at - 1])) &&
           (at + word.size() >= text.size() || !IsIdentifierChar(text[at + word.size()]));
}

// The blocks of a text ({ ... }) and its case labels, to tell which writes
// of a variable reach a call.
class Blocks
{
public:
    explicit Blocks(std::string_view text) : m_text(text), m_blockAt(text.size(), -1)
    {
        std::vector<int> open;
        for (size_t i = 0; i < text.size(); ++i)
        {
            if (text[i] == '{')
            {
                m_parent.push_back(open.empty() ? -1 : open.back());
                m_open.push_back(i);
                open.push_back(static_cast<int>(m_parent.size()) - 1);
            }
            m_blockAt[i] = open.empty() ? -1 : open.back();
            if (text[i] == '}' && !open.empty())
                open.pop_back();
            if (!open.empty() && (IsWordAt(text, i, "case") || IsWordAt(text, i, "default")))
                m_labels.push_back({i, open.back()});
        }
    }

    int BlockAt(size_t at) const
    {
        return m_blockAt[at];
    }

    bool Encloses(int outer, int inner) const
    {
        for (int block = inner; block >= 0; block = m_parent[static_cast<size_t>(block)])
        {
            if (block == outer)
                return true;
        }
        return outer < 0;
    }

    // Where the body of the function around `at` begins: its outermost block
    // that follows a parameter list (namespace and class blocks do not);
    // npos outside a function.
    size_t FunctionStart(size_t at) const
    {
        std::vector<int> chain;
        for (int block = BlockAt(at); block >= 0; block = m_parent[static_cast<size_t>(block)])
            chain.push_back(block);
        for (auto block = chain.rbegin(); block != chain.rend(); ++block)
        {
            if (FollowsParameters(m_open[static_cast<size_t>(*block)]))
                return m_open[static_cast<size_t>(*block)];
        }
        return std::string_view::npos;
    }

    // Whether a case label between `write` and `call` starts another case of
    // a switch both lie in: what the write did belongs to an earlier case.
    bool InEarlierCase(size_t write, size_t call) const
    {
        const int writeBlock = BlockAt(write);
        const int callBlock = BlockAt(call);
        const auto first = std::upper_bound(m_labels.begin(), m_labels.end(), write,
                                            [](size_t at, const Label& label) { return at < label.at; });
        for (auto label = first; label != m_labels.end() && label->at < call; ++label)
        {
            if (Encloses(label->block, callBlock) && Encloses(label->block, writeBlock))
                return true;
        }
        return false;
    }

private:
    struct Label
    {
        size_t at;
        int block;
    };

    // ")" before a '{', past const, noexcept, override, final and mutable.
    bool FollowsParameters(size_t brace) const
    {
        size_t k = brace;
        while (k > 0)
        {
            while (k > 0 && std::isspace(static_cast<unsigned char>(m_text[k - 1])) != 0)
                --k;
            size_t wordStart = k;
            while (wordStart > 0 && IsIdentifierChar(m_text[wordStart - 1]))
                --wordStart;
            const std::string_view word = m_text.substr(wordStart, k - wordStart);
            if (word.empty())
                return k > 0 && m_text[k - 1] == ')';
            if (word != "const" && word != "noexcept" && word != "override" && word != "final" && word != "mutable")
                return false;
            k = wordStart;
        }
        return false;
    }

    std::string_view m_text;
    std::vector<int> m_blockAt;
    std::vector<int> m_parent;
    std::vector<size_t> m_open;
    std::vector<Label> m_labels;
};

std::optional<PreviewVector> ColourOf(const std::vector<std::string_view>& values)
{
    const std::optional<float> r = values.size() >= 3 ? FloatOf(values[0]) : std::nullopt;
    const std::optional<float> g = values.size() >= 3 ? FloatOf(values[1]) : std::nullopt;
    const std::optional<float> b = values.size() >= 3 ? FloatOf(values[2]) : std::nullopt;
    if (r && g && b)
        return PreviewVector{*r, *g, *b};
    return std::nullopt;
}

bool IsCreateFunction(std::string_view name);

// What the text does at `at` to the variable of `length` characters there.
struct Write
{
    bool writes = false;
    // The light written, when it is known.
    std::optional<PreviewVector> light;
};

Write WriteAt(std::string_view text, size_t at, size_t length)
{
    const size_t after = SkipSpaces(text, at + length);
    size_t before = at;
    while (before > 0 && std::isspace(static_cast<unsigned char>(text[before - 1])) != 0)
        --before;
    const char next = after < text.size() ? text[after] : '\0';
    const char nextButOne = after + 1 < text.size() ? text[after + 1] : '\0';
    const char previous = before > 0 ? text[before - 1] : '\0';
    // name = { r, g, b } declares or sets it; name = ... sets it otherwise.
    if (next == '=' && nextButOne != '=')
    {
        const size_t value = SkipSpaces(text, after + 1);
        if (value < text.size() && text[value] == '{')
            return {true, ColourOf(ArgumentsAt(text, value))};
        return {true, std::nullopt};
    }
    // name[i] = ..., name[i] *= ...
    if (next == '[')
    {
        const std::vector<std::string_view> index = ArgumentsAt(text, after);
        const size_t close =
            index.empty() ? text.size() : static_cast<size_t>(index.back().data() - text.data()) + index.back().size();
        const size_t op = SkipSpaces(text, close + 1);
        const bool assigns =
            op + 1 < text.size() &&
            ((text[op] == '=' && text[op + 1] != '=') ||
             (std::string_view("+-*/").find(text[op]) != std::string_view::npos && text[op + 1] == '='));
        return {assigns, std::nullopt};
    }
    // The last argument of a call: Vector(r, g, b, name) writes a light, the
    // create functions read it, any other function may write it.
    if (next == ')' && (previous == ',' || previous == '('))
    {
        int depth = 0;
        size_t open = std::string_view::npos;
        for (size_t k = before; k > 0; --k)
        {
            const char c = text[k - 1];
            if (c == ')')
                ++depth;
            else if (c == '(' && depth-- == 0)
            {
                open = k - 1;
                break;
            }
            else if (c == ';' || c == '{' || c == '}')
                break;
        }
        if (open == std::string_view::npos)
            return {};
        size_t nameEnd = open;
        while (nameEnd > 0 && std::isspace(static_cast<unsigned char>(text[nameEnd - 1])) != 0)
            --nameEnd;
        size_t nameStart = nameEnd;
        while (nameStart > 0 && IsIdentifierChar(text[nameStart - 1]))
            --nameStart;
        const std::string_view function = text.substr(nameStart, nameEnd - nameStart);
        if (IsCreateFunction(function))
            return {};
        if (function == "Vector")
        {
            const std::vector<std::string_view> values = ArgumentsAt(text, open);
            return {true, values.size() == 4 ? ColourOf(values) : std::nullopt};
        }
        return {true, std::nullopt};
    }
    // vec3_t name; declares it without a value.
    if (IsIdentifierChar(previous) && (next == ';' || next == ','))
    {
        size_t wordStart = before;
        while (wordStart > 0 && IsIdentifierChar(text[wordStart - 1]))
            --wordStart;
        const std::string_view word = text.substr(wordStart, before - wordStart);
        return {word != "return" && word != "case" && word != "delete", std::nullopt};
    }
    return {};
}

// The light a call's light variable has at the call: the last write in the
// call's function that reaches it. A write in an earlier case of a switch
// does not; one in a block the call is not in may not, so the light is
// unknown.
std::optional<PreviewVector> LightBefore(std::string_view text, const Blocks& blocks, size_t call,
                                         std::string_view variable)
{
    const std::string name = WithoutSpaces(variable);
    if (name.empty() || std::isdigit(static_cast<unsigned char>(name[0])) != 0 ||
        !std::all_of(name.begin(), name.end(), IsIdentifierChar))
        return std::nullopt;
    const size_t function = blocks.FunctionStart(call);
    if (function == std::string_view::npos)
        return std::nullopt;
    for (size_t at = text.rfind(name, call); at != std::string_view::npos && at > function;
         at = at == 0 ? std::string_view::npos : text.rfind(name, at - 1))
    {
        if (!IsWordAt(text, at, name))
            continue;
        const Write write = WriteAt(text, at, name.size());
        if (!write.writes || blocks.InEarlierCase(at, call))
            continue;
        if (!blocks.Encloses(blocks.BlockAt(at), blocks.BlockAt(call)))
            return std::nullopt;
        return write.light;
    }
    return std::nullopt;
}

bool IsCreateFunction(std::string_view name)
{
    constexpr std::string_view Checked = "FpsChecked";
    if (name.size() > Checked.size() && name.substr(name.size() - Checked.size()) == Checked)
        name.remove_suffix(Checked.size());
    return std::any_of(Shapes.begin(), Shapes.end(), [name](const CallShape& shape) { return shape.name == name; });
}

std::string_view Argument(const std::vector<std::string_view>& arguments, int index)
{
    return index >= 0 && static_cast<size_t>(index) < arguments.size() ? arguments[static_cast<size_t>(index)]
                                                                       : std::string_view();
}
} // namespace

bool CreatesWithoutOwner(std::span<const EffectCallSite* const> calls, int subType)
{
    return std::any_of(calls.begin(), calls.end(), [subType](const EffectCallSite* call)
                       { return call->withoutOwner && call->subTypeValue == subType; });
}

std::vector<EffectCallSite> ReadEffectCallSites(std::string_view source, std::string_view file,
                                                const std::unordered_set<std::string>& macrosOff)
{
    const std::string text = WithoutBranchesOff(Blank(source), macrosOff);
    std::optional<Blocks> blocks;
    std::vector<size_t> lineStarts = {0};
    for (size_t i = 0; i < text.size(); ++i)
    {
        if (text[i] == '\n')
            lineStarts.push_back(i + 1);
    }
    std::vector<EffectCallSite> calls;
    for (const CallShape& shape : Shapes)
    {
        for (size_t at = text.find(shape.name); at != std::string::npos; at = text.find(shape.name, at + 1))
        {
            if (at > 0 && IsIdentifierChar(text[at - 1]))
                continue;
            size_t end = at + shape.name.size();
            if (text.compare(end, 10, "FpsChecked") == 0)
                end += 10;
            while (end < text.size() && (text[end] == ' ' || text[end] == '\t'))
                ++end;
            if (end >= text.size() || text[end] != '(' || !IsCall(text, at))
                continue;
            const std::vector<std::string_view> arguments = ArgumentsAt(text, end);
            if (arguments.empty())
                continue;
            const std::vector<int> types = TypesOf(shape.kind, arguments[0]);
            if (types.empty())
                continue;

            EffectCallSite call;
            call.kind = shape.kind;
            call.randomTypes = static_cast<int>(types.size());
            call.file = std::string(file);
            call.line =
                static_cast<int>(std::upper_bound(lineStarts.begin(), lineStarts.end(), at) - lineStarts.begin());
            call.subType = Trimmed(Argument(arguments, shape.subType));
            call.subTypeValue = call.subType.empty() ? std::optional<int>(0) : IntegerOf(call.subType);
            call.scale = Trimmed(Argument(arguments, shape.scale));
            call.scaleValue = FloatOf(call.scale);
            call.owner = Trimmed(Argument(arguments, shape.owner));
            const std::string owner = WithoutSpaces(call.owner);
            call.withoutOwner = owner.empty() || owner == "NULL" || owner == "nullptr" || owner == "0";
            call.pkValue = IntegerOf(Argument(arguments, shape.pk));
            call.skillIndexValue = IntegerOf(Argument(arguments, shape.skillIndex));
            if (!blocks)
                blocks.emplace(text);
            call.light = LightBefore(text, *blocks, at, Argument(arguments, shape.light));
            for (const int type : types)
            {
                call.type = type;
                calls.push_back(call);
            }
        }
    }
    return calls;
}

namespace
{
std::string ReadFile(const std::filesystem::path& path)
{
    std::ifstream stream(path, std::ios::binary);
    std::ostringstream text;
    text << stream.rdbuf();
    return text.str();
}

// The macros the #define lines of a text define, and those its #ifdef and
// #if defined lines test.
void NoteMacros(std::string_view text, std::unordered_set<std::string>& defined,
                std::unordered_set<std::string>& tested)
{
    for (size_t start = 0; start < text.size();)
    {
        const size_t end = std::min(text.find('\n', start), text.size());
        if (const std::optional<Directive> directive = DirectiveOf(text.substr(start, end - start)))
        {
            if (directive->name == "define")
                defined.emplace(IdentifierAt(directive->rest, SkipSpaces(directive->rest, 0)));
            else if (const std::string_view macro = MacroTested(*directive); !macro.empty())
                tested.emplace(macro);
        }
        start = end + 1;
    }
}
} // namespace

bool EffectCallSiteIndex::Load(const std::filesystem::path& sourceDirectory)
{
    m_calls.clear();
    m_byType.clear();
    m_loaded = true;
    std::error_code error;
    if (!std::filesystem::is_directory(sourceDirectory, error))
        return false;
    std::vector<std::pair<std::string, std::string>> sources;
    std::unordered_set<std::string> defined;
    std::unordered_set<std::string> tested;
    for (std::filesystem::recursive_directory_iterator entry(sourceDirectory, error), end; !error && entry != end;
         entry.increment(error))
    {
        const std::filesystem::path extension = entry->path().extension();
        std::error_code entryError;
        if ((extension != ".cpp" && extension != ".h") || !entry->is_regular_file(entryError))
            continue;
        std::string text = ReadFile(entry->path());
        NoteMacros(text, defined, tested);
        if (extension == ".cpp")
            sources.emplace_back(entry->path().lexically_relative(sourceDirectory).generic_string(), std::move(text));
    }
    // What the build defines is named in its files beside the sources;
    // compiler and platform macros begin with '_' or are common names.
    std::string build;
    for (const std::filesystem::path& file : {sourceDirectory.parent_path() / "CMakeLists.txt",
                                              sourceDirectory.parent_path().parent_path() / "CMakeLists.txt",
                                              sourceDirectory.parent_path().parent_path() / "CMakePresets.json"})
        build += ReadFile(file);
    constexpr std::array<std::string_view, 4> Platform = {"WIN32", "NDEBUG", "DEBUG", "UNICODE"};
    std::unordered_set<std::string> macrosOff;
    for (const std::string& macro : tested)
    {
        if (defined.count(macro) == 0 && macro.front() != '_' && build.find(macro) == std::string::npos &&
            std::find(Platform.begin(), Platform.end(), macro) == Platform.end())
            macrosOff.insert(macro);
    }
    for (const auto& [file, source] : sources)
    {
        for (EffectCallSite& call : ReadEffectCallSites(source, file, macrosOff))
            m_calls.push_back(std::move(call));
    }
    std::stable_sort(m_calls.begin(), m_calls.end(), [](const EffectCallSite& left, const EffectCallSite& right)
                     { return std::tie(left.file, left.line) < std::tie(right.file, right.line); });
    for (size_t i = 0; i < m_calls.size(); ++i)
        m_byType[{static_cast<int>(Data::Effects::ToIndex(m_calls[i].kind)), m_calls[i].type}].push_back(i);
    return !m_calls.empty();
}

std::vector<const EffectCallSite*> EffectCallSiteIndex::Find(Data::Effects::EffectKind kind, int type) const
{
    std::vector<const EffectCallSite*> calls;
    const auto found = m_byType.find({static_cast<int>(Data::Effects::ToIndex(kind)), type});
    if (found == m_byType.end())
        return calls;
    for (const size_t index : found->second)
        calls.push_back(&m_calls[index]);
    return calls;
}
} // namespace MuEditor::Effects

#endif // _EDITOR
