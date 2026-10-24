#include "stdafx.h"

#include "NpcNameText.h"

#include "Core/Text/Utf8.h"
#include "Data/GameData/MonsterData/MonsterDefinition.h"

#include <charconv>
#include <optional>

namespace Data::Monsters
{
namespace
{
using Items::ItemDataIssue;
using Items::ItemDataIssueSeverity;

constexpr std::string_view CommentStart = "//";
constexpr std::string_view EndLine = "end";
constexpr std::string_view Whitespace = " \t\r";
constexpr char Quote = '"';
constexpr std::string_view LineEnd = "\r\n";
constexpr std::string_view LineBreaks = "\r\n";
// The number the game ignores; the original files have 1 on every line.
constexpr std::string_view IgnoredNumber = "1";

struct NameLine
{
    int number = 0;
    std::string name;
};

std::string_view Trim(std::string_view text)
{
    const size_t first = text.find_first_not_of(Whitespace);
    if (first == std::string_view::npos)
    {
        return {};
    }
    return text.substr(first, text.find_last_not_of(Whitespace) - first + 1);
}

// Reads the whole number at the start of `text` and moves past it.
std::optional<int> ReadNumber(std::string_view& text)
{
    text = Trim(text);
    int number = 0;
    const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), number);
    if (error != std::errc{} || end == text.data())
    {
        return std::nullopt;
    }
    text.remove_prefix(static_cast<size_t>(end - text.data()));
    return number;
}

// "<number> <number> "<name>"", anything after the closing quote ignored.
std::optional<NameLine> ParseLine(std::string_view line)
{
    NameLine result;
    const std::optional<int> number = ReadNumber(line);
    if (!number || !ReadNumber(line))
    {
        return std::nullopt;
    }
    line = Trim(line);
    if (line.empty() || line.front() != Quote)
    {
        return std::nullopt;
    }
    const size_t closing = line.find(Quote, 1);
    if (closing == std::string_view::npos)
    {
        return std::nullopt;
    }
    result.number = *number;
    result.name = std::string(line.substr(1, closing - 1));
    return result;
}

void Warn(std::vector<ItemDataIssue>& issues, const std::string& source, int monster, const std::string& field,
          const std::string& message)
{
    issues.push_back({ItemDataIssueSeverity::Warning, source, ItemDataIssue::NoItem, monster, field, message});
}

// Adds the name of the line; true when it was cut.
bool AddName(NameLine line, const std::string& source, const std::string& field, std::map<int, std::string>& names,
             std::vector<ItemDataIssue>& issues)
{
    if (line.number < 0 || line.number >= MonsterNumberCount)
    {
        Warn(issues, source, ItemDataIssue::NoItem, field,
             "monster " + std::to_string(line.number) + " is not a number from 0 to " +
                 std::to_string(MonsterNumberCount - 1) + "; skipped");
        return false;
    }
    if (names.contains(line.number))
    {
        Warn(issues, source, line.number, field,
             "monster " + std::to_string(line.number) + " has a name already; this one is skipped");
        return false;
    }
    // The game read the names as UTF-8; a file saved in another encoding
    // would give a garbled name.
    if (!Core::Text::IsValidUtf8(line.name))
    {
        Warn(issues, source, line.number, field,
             "the name of monster " + std::to_string(line.number) + " is not UTF-8; skipped");
        return false;
    }
    const bool cut = Core::Text::CutToUtf16Length(line.name, MaxMonsterNameLength);
    if (cut)
    {
        Warn(issues, source, line.number, field,
             "the name is longer than " + std::to_string(MaxMonsterNameLength) + " characters and was cut");
    }
    names.emplace(line.number, std::move(line.name));
    return cut;
}

// What keeps the name off a line of a name file: a quote would end the name, a
// line break the line. Empty when nothing does.
std::string_view Unwritable(std::string_view name)
{
    if (name.find(Quote) != std::string_view::npos)
    {
        return "a quote";
    }
    if (name.find_first_of(LineBreaks) != std::string_view::npos)
    {
        return "a line break";
    }
    return {};
}
} // namespace

int ReadNpcNameText(std::string_view text, const std::string& source, std::map<int, std::string>& names,
                    std::vector<ItemDataIssue>& issues)
{
    int cutNames = 0;
    size_t lineNumber = 0;
    while (!text.empty())
    {
        const size_t lineEnd = text.find('\n');
        const std::string_view rawLine = text.substr(0, lineEnd);
        text = lineEnd == std::string_view::npos ? std::string_view{} : text.substr(lineEnd + 1);
        ++lineNumber;

        const std::string_view line = Trim(rawLine);
        if (line.empty() || line.starts_with(CommentStart))
        {
            continue;
        }
        if (line == EndLine)
        {
            break;
        }
        const std::string field = "line " + std::to_string(lineNumber);
        std::optional<NameLine> parsed = ParseLine(line);
        if (!parsed)
        {
            Warn(issues, source, ItemDataIssue::NoItem, field, "not <number> <number> \"<name>\"; skipped");
            continue;
        }
        if (AddName(std::move(*parsed), source, field, names, issues))
        {
            ++cutNames;
        }
    }
    return cutNames;
}

std::string WriteNpcNameText(const std::map<int, std::string>& names, std::vector<ItemDataIssue>& issues)
{
    std::string text;
    for (const auto& [number, name] : names)
    {
        const std::string_view unwritable = Unwritable(name);
        if (!unwritable.empty())
        {
            Warn(issues, "", number, "monster " + std::to_string(number) + " name",
                 "a name holding " + std::string(unwritable) + " cannot be written to a name file; left out");
            continue;
        }
        text += std::to_string(number);
        text += '\t';
        text += IgnoredNumber;
        text += '\t';
        text += Quote;
        text += name;
        text += Quote;
        text += LineEnd;
    }
    text += EndLine;
    text += LineEnd;
    return text;
}
} // namespace Data::Monsters
