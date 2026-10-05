#include "EffectSourceCases.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iterator>
#include <sstream>
#include <utility>

namespace EffectSourceCases
{
namespace
{
// How each function starts in ZzzEffect.cpp.
std::string_view SignatureOf(Stage stage)
{
    switch (stage)
    {
    case Stage::Create:
        return "void CreateEffect(int Type,";
    case Stage::Move:
        return "void MoveEffect(OBJECT* o, int iIndex)";
    case Stage::Render:
        return "void RenderEffects(bool bRenderBlendMesh)";
    case Stage::Ground:
        return "void RenderEffectShadows()";
    case Stage::AfterCharacters:
        break;
    }
    return "void RenderAfterEffects()";
}

// The switch of a stage is the first one after this text; RenderEffectShadows
// and RenderAfterEffects have no registry lookup, so their first switch counts.
std::string_view SwitchAfter(Stage stage)
{
    const bool afterLookup = stage == Stage::Create || stage == Stage::Move || stage == Stage::Render;
    return afterLookup ? std::string_view("Render::Effects::Lookup(") : std::string_view();
}

bool IsIdentifierChar(char c)
{
    return std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_';
}

bool IsKeywordAt(std::string_view text, size_t pos, std::string_view keyword)
{
    return text.compare(pos, keyword.size(), keyword) == 0 && (pos == 0 || !IsIdentifierChar(text[pos - 1])) &&
           (pos + keyword.size() >= text.size() || !IsIdentifierChar(text[pos + keyword.size()]));
}

std::string WithoutSpaces(std::string_view text)
{
    std::string result;
    std::copy_if(text.begin(), text.end(), std::back_inserter(result),
                 [](char c) { return std::isspace(static_cast<unsigned char>(c)) == 0; });
    return result;
}

// Blanks a comment or literal that starts at `i`; returns where the scan goes
// on, or `i` when nothing starts there. Line breaks stay.
size_t BlankCommentOrLiteral(std::string& text, size_t i)
{
    if (text.compare(i, 2, "//") == 0)
    {
        for (; i < text.size() && text[i] != '\n'; ++i)
            text[i] = ' ';
        return i;
    }
    if (text.compare(i, 2, "/*") == 0)
    {
        const size_t end = std::min(text.find("*/", i + 2), text.size() - 2) + 2;
        for (; i < end; ++i)
            text[i] = text[i] == '\n' ? '\n' : ' ';
        return i;
    }
    if (text[i] != '"' && text[i] != '\'')
        return i;
    const char quote = text[i++];
    for (; i < text.size() && text[i] != quote && text[i] != '\n'; ++i)
    {
        if (text[i] == '\\' && i + 1 < text.size())
            text[i++] = ' ';
        text[i] = ' ';
    }
    return i + 1;
}

// A preprocessor line: its directive ("ifdef", "endif", ...) and first word.
struct Directive
{
    std::string kind;
    std::string name;
};

bool ReadDirective(const std::string& line, Directive& directive)
{
    std::istringstream words(line);
    std::string first;
    if (!(words >> first) || first[0] != '#')
        return false;
    directive.kind = first.size() > 1 ? first.substr(1) : std::string();
    if (directive.kind.empty())
        words >> directive.kind;
    directive.name.clear();
    words >> directive.name;
    return true;
}

// The lines of the active branches of the #ifdef blocks of a function.
class ActiveLines
{
public:
    ActiveLines(std::span<const MacroState> macros, std::vector<std::string>& problems)
        : m_macros(macros), m_problems(problems)
    {
    }

    // Whether the line belongs to the text; preprocessor lines do not.
    bool Take(const std::string& line)
    {
        Directive directive;
        if (!ReadDirective(line, directive))
            return Active();
        if (directive.kind == "ifdef" || directive.kind == "ifndef")
            Open(directive, line);
        else if (directive.kind == "else" && !m_branches.empty())
            m_branches.back().second = !m_branches.back().second;
        else if (directive.kind == "endif" && !m_branches.empty())
            m_branches.pop_back();
        else if (directive.kind == "if" || directive.kind == "elif" || directive.kind == "else" ||
                 directive.kind == "endif")
            m_problems.push_back("cannot read: " + line);
        return false;
    }

private:
    bool Active() const
    {
        return std::all_of(m_branches.begin(), m_branches.end(), [](const auto& branch) { return branch.second; });
    }

    void Open(const Directive& directive, const std::string& line)
    {
        const auto macro = std::find_if(m_macros.begin(), m_macros.end(),
                                        [&](const MacroState& state) { return state.name == directive.name; });
        if (macro == m_macros.end())
            m_problems.push_back("macro not in the list: " + line);
        const bool defined = macro != m_macros.end() && macro->defined;
        m_branches.emplace_back(directive.name, (directive.kind == "ifdef") == defined);
    }

    std::span<const MacroState> m_macros;
    std::vector<std::string>& m_problems;
    // The open blocks: macro and whether the current branch is taken.
    std::vector<std::pair<std::string, bool>> m_branches;
};

// The active text of the function that starts with `signature`, up to the
// brace that closes it.
std::string FunctionText(const std::string& source, std::string_view signature, std::span<const MacroState> macros,
                         std::vector<std::string>& problems)
{
    const size_t start = source.find("\n" + std::string(signature));
    if (start == std::string::npos)
    {
        problems.push_back("function not found: " + std::string(signature));
        return {};
    }
    std::istringstream lines(source.substr(start + 1));
    ActiveLines active(macros, problems);
    std::string line, text;
    int depth = 0;
    while (std::getline(lines, line))
    {
        if (!active.Take(line))
            continue;
        text += line + '\n';
        for (const char c : line)
        {
            depth += c == '{' ? 1 : c == '}' ? -1 : 0;
            if (c == '}' && depth == 0)
                return text;
        }
    }
    problems.push_back("function not closed: " + std::string(signature));
    return text;
}

// The end of a case label: the first ':' that is not part of "::".
size_t LabelEnd(std::string_view text, size_t pos)
{
    for (size_t colon = text.find(':', pos); colon != std::string_view::npos; colon = text.find(':', colon + 2))
    {
        if (colon + 1 >= text.size() || text[colon + 1] != ':')
            return colon;
    }
    return std::string_view::npos;
}

size_t FindKeyword(std::string_view text, std::string_view keyword, size_t pos)
{
    for (pos = text.find(keyword, pos); pos != std::string_view::npos; pos = text.find(keyword, pos + 1))
    {
        if (IsKeywordAt(text, pos, keyword))
            return pos;
    }
    return std::string_view::npos;
}

// A label at the top level of a switch, without spaces ("default" for the
// default label), with where its keyword starts and where its statements start.
struct LabelPosition
{
    std::string label;
    size_t start = 0;
    size_t statements = 0;
};

// The labels of the switch whose body starts at `body`; the labels of nested
// switches are left out. Returns where the body closes; npos when it is not
// closed.
size_t ReadLabelPositions(std::string_view function, size_t body, std::vector<LabelPosition>& labels,
                          std::vector<std::string>& problems)
{
    int depth = 0;
    std::vector<int> nestedSwitchDepths;
    for (size_t k = body; k < function.size(); ++k)
    {
        const char c = function[k];
        if (c == '{' || c == '}')
        {
            depth += c == '{' ? 1 : -1;
            while (!nestedSwitchDepths.empty() && nestedSwitchDepths.back() > depth)
                nestedSwitchDepths.pop_back();
            if (depth == 0)
                return k;
            continue;
        }
        if (IsKeywordAt(function, k, "switch"))
            nestedSwitchDepths.push_back(depth + 1);
        const bool isCase = IsKeywordAt(function, k, "case");
        if ((!isCase && !IsKeywordAt(function, k, "default")) || !nestedSwitchDepths.empty())
            continue;
        const size_t end = LabelEnd(function, k);
        std::string label = isCase ? WithoutSpaces(function.substr(k + 4, end - k - 4)) : std::string("default");
        if (depth == 1)
            labels.push_back({std::move(label), k, end + 1});
        else
            problems.push_back("label below the top level: " + label);
        k = end;
    }
    problems.push_back("switch not closed");
    return std::string_view::npos;
}

// The case labels of the switch whose body starts at `body`.
void ReadLabels(std::string_view function, size_t body, SwitchLabels& result)
{
    std::vector<LabelPosition> labels;
    ReadLabelPositions(function, body, labels, result.problems);
    for (LabelPosition& label : labels)
    {
        if (label.label != "default")
            result.labels.push_back(std::move(label.label));
    }
}

// Where the body of the switch of `stage` opens in the text of its function;
// npos without one.
size_t SwitchBody(std::string_view function, Stage stage)
{
    const std::string_view marker = SwitchAfter(stage);
    const size_t after = marker.empty() ? 0 : function.find(marker);
    const size_t switchStart = after == std::string_view::npos ? after : FindKeyword(function, "switch", after);
    return switchStart == std::string_view::npos ? switchStart : function.find('{', switchStart);
}

// The position after the bracket that closes the '(' or '{' at `open`; npos
// when it is not closed.
size_t AfterClosing(std::string_view text, size_t open)
{
    const char opening = text[open];
    const char closing = opening == '(' ? ')' : '}';
    int depth = 0;
    for (size_t k = open; k < text.size(); ++k)
    {
        depth += text[k] == opening ? 1 : text[k] == closing ? -1 : 0;
        if (depth == 0)
            return k + 1;
    }
    return std::string_view::npos;
}

// Whether `text` goes on with `expected` at `pos`; moves `pos` past it.
bool TakeText(std::string_view text, size_t& pos, std::string_view expected)
{
    if (text.compare(pos, expected.size(), expected) != 0)
        return false;
    pos += expected.size();
    return true;
}

// Whether statements end with an unconditional `return;`: one at their own
// level, also when a block holds all of them, not the branch of an if, a loop
// or an else.
bool EndsWithReturn(std::string_view statements)
{
    std::string text = WithoutSpaces(statements);
    while (text.size() >= 2 && text.front() == '{' && AfterClosing(text, 0) == text.size())
        text = text.substr(1, text.size() - 2);
    constexpr std::string_view Return = "return;";
    if (!text.ends_with(Return))
        return false;
    const size_t at = text.size() - Return.size();
    return at == 0 || text[at - 1] == ';' || text[at - 1] == '}';
}

// Reads what is inside the `opening` bracket at `pos` and moves `pos` past
// the bracket that closes it.
bool TakeBracketed(std::string_view text, size_t& pos, char opening, std::string& inside)
{
    if (pos >= text.size() || text[pos] != opening)
        return false;
    const size_t end = AfterClosing(text, pos);
    if (end == std::string_view::npos)
        return false;
    inside = std::string(text.substr(pos + 1, end - pos - 2));
    pos = end;
    return true;
}
} // namespace

std::string ReadEffectSource(const std::filesystem::path& zzzEffect)
{
    std::ifstream file(zzzEffect, std::ios::binary);
    std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
    for (size_t i = 0; i < text.size();)
    {
        const size_t next = BlankCommentOrLiteral(text, i);
        i = next == i ? i + 1 : next;
    }
    return text;
}

SwitchLabels ReadSwitchLabels(const std::string& source, Stage stage, std::span<const MacroState> macros)
{
    SwitchLabels result;
    const std::string function = FunctionText(source, SignatureOf(stage), macros, result.problems);
    const size_t body = SwitchBody(function, stage);
    if (body == std::string::npos)
    {
        result.problems.push_back("no switch found");
        return result;
    }
    ReadLabels(function, body, result);
    return result;
}

SharedMoveConditions ReadSharedMoveConditions(const std::string& source, std::span<const MacroState> macros)
{
    SharedMoveConditions result;
    const std::string function = FunctionText(source, SignatureOf(Stage::Move), macros, result.problems);
    const size_t body = SwitchBody(function, Stage::Move);
    const size_t switchEnd = body == std::string::npos ? body : AfterClosing(function, body);
    if (switchEnd == std::string::npos)
    {
        result.problems.push_back("no switch found");
        return result;
    }
    const std::string shared = WithoutSpaces(std::string_view(function).substr(switchEnd));
    size_t pos = 0;
    std::string animatedCode;
    const bool read = TakeText(shared, pos, "if") && TakeBracketed(shared, pos, '(', result.skipped) &&
                      TakeText(shared, pos, "{}else{if") && TakeBracketed(shared, pos, '(', result.animated) &&
                      TakeBracketed(shared, pos, '{', animatedCode) &&
                      animatedCode.find("PlayAnimation(") != std::string::npos;
    if (!read)
        result.problems.push_back("the code after the switch is not as expected: " + shared.substr(0, 160));
    return result;
}

SwitchLabels ReadCasesThatReturn(const std::string& source, Stage stage, std::span<const MacroState> macros)
{
    SwitchLabels result;
    const std::string function = FunctionText(source, SignatureOf(stage), macros, result.problems);
    const size_t body = SwitchBody(function, stage);
    if (body == std::string::npos)
    {
        result.problems.push_back("no switch found");
        return result;
    }
    std::vector<LabelPosition> labels;
    const size_t close = ReadLabelPositions(function, body, labels, result.problems);
    if (close == std::string::npos)
        return result;
    const std::string_view text = function;
    for (size_t i = 0; i < labels.size(); ++i)
    {
        std::string_view statements;
        for (size_t j = i; j < labels.size(); ++j)
        {
            const size_t next = j + 1 < labels.size() ? labels[j + 1].start : close;
            statements = text.substr(labels[j].statements, next - labels[j].statements);
            if (!WithoutSpaces(statements).empty())
                break;
        }
        if (labels[i].label != "default" && EndsWithReturn(statements))
            result.labels.push_back(labels[i].label);
    }
    return result;
}
} // namespace EffectSourceCases
