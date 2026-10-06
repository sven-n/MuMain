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

// The last place before `call`, in its function, where `function(...)`
// writes `name` as its last argument; npos without one.
size_t LastWrite(std::string_view text, size_t from, size_t call, std::string_view function, std::string_view name)
{
    for (size_t at = text.rfind(function, call); at != std::string_view::npos && at >= from;
         at = at == 0 ? std::string_view::npos : text.rfind(function, at - 1))
    {
        if (at > 0 && IsIdentifierChar(text[at - 1]))
            continue;
        const std::vector<std::string_view> values = ArgumentsAt(text, at + function.size() - 1);
        if (!values.empty() && WithoutSpaces(values.back()) == name)
            return at;
    }
    return std::string_view::npos;
}

// The light a call's light variable got last before the call in its
// function: Vector(r, g, b, light) with numbers, unless a VectorCopy into it
// comes later.
std::optional<PreviewVector> LightBefore(std::string_view text, size_t call, std::string_view variable)
{
    const std::string name = WithoutSpaces(variable);
    if (name.empty() || !std::all_of(name.begin(), name.end(), IsIdentifierChar))
        return std::nullopt;
    const size_t functionEnd = text.rfind("\n}", call);
    const size_t from = functionEnd == std::string_view::npos ? 0 : functionEnd;
    const size_t vector = LastWrite(text, from, call, "Vector(", name);
    const size_t copy = LastWrite(text, from, call, "VectorCopy(", name);
    if (vector == std::string_view::npos || (copy != std::string_view::npos && copy > vector))
        return std::nullopt;
    const std::vector<std::string_view> values = ArgumentsAt(text, vector + 6);
    const std::optional<float> r = values.size() == 4 ? FloatOf(values[0]) : std::nullopt;
    const std::optional<float> g = values.size() == 4 ? FloatOf(values[1]) : std::nullopt;
    const std::optional<float> b = values.size() == 4 ? FloatOf(values[2]) : std::nullopt;
    if (r && g && b)
        return PreviewVector{*r, *g, *b};
    return std::nullopt;
}

std::string_view Argument(const std::vector<std::string_view>& arguments, int index)
{
    return index >= 0 && static_cast<size_t>(index) < arguments.size() ? arguments[static_cast<size_t>(index)]
                                                                       : std::string_view();
}
} // namespace

std::vector<EffectCallSite> ReadEffectCallSites(std::string_view source, std::string_view file)
{
    const std::string text = Blank(source);
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
            call.light = LightBefore(text, at, Argument(arguments, shape.light));
            for (const int type : types)
            {
                call.type = type;
                calls.push_back(call);
            }
        }
    }
    return calls;
}

bool EffectCallSiteIndex::Load(const std::filesystem::path& sourceDirectory)
{
    m_calls.clear();
    m_byType.clear();
    m_loaded = true;
    std::error_code error;
    if (!std::filesystem::is_directory(sourceDirectory, error))
        return false;
    for (std::filesystem::recursive_directory_iterator entry(sourceDirectory, error), end; !error && entry != end;
         entry.increment(error))
    {
        if (!entry->is_regular_file(error) || entry->path().extension() != ".cpp")
            continue;
        std::ifstream stream(entry->path(), std::ios::binary);
        const std::string source((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        const std::string file = entry->path().lexically_relative(sourceDirectory).generic_string();
        for (EffectCallSite& call : ReadEffectCallSites(source, file))
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
