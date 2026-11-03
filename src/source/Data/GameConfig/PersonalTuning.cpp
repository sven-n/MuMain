#include "stdafx.h"

#include "Data/GameConfig/PersonalTuning.h"

#ifndef _WIN32
#include "Core/Platform/PathResolve.h"
#endif

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iterator>
#include <locale>
#include <sstream>
#include <system_error>

namespace Data::Config
{
namespace
{
using Data::Maps::LookValue;
using Data::Maps::LookValueInfo;

constexpr const char* TemporarySuffix = ".tmp";

std::string_view Trim(std::string_view text)
{
    const auto blank = [](char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; };
    while (!text.empty() && blank(text.front()))
    {
        text.remove_prefix(1);
    }
    while (!text.empty() && blank(text.back()))
    {
        text.remove_suffix(1);
    }
    return text;
}

bool EqualsIgnoringCase(std::string_view left, std::string_view right)
{
    return left.size() == right.size() &&
           std::equal(
               left.begin(), left.end(), right.begin(), [](char a, char b)
               { return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b)); });
}

// The section a line opens, if it is a section header. The first line can carry a UTF-8 BOM.
std::optional<std::string_view> SectionName(std::string_view line)
{
    if (line.starts_with("\xEF\xBB\xBF"))
    {
        line.remove_prefix(3);
    }
    const std::string_view trimmed = Trim(line);
    if (trimmed.size() < 2 || trimmed.front() != '[' || trimmed.back() != ']')
    {
        return std::nullopt;
    }
    return Trim(trimmed.substr(1, trimmed.size() - 2));
}

bool IsTuningHeader(std::string_view line)
{
    const std::optional<std::string_view> name = SectionName(line);
    return name && EqualsIgnoringCase(*name, PersonalTuningSection);
}

// The lines of `text`, each with its line break.
std::vector<std::string_view> SplitLines(std::string_view text)
{
    std::vector<std::string_view> lines;
    std::size_t start = 0;
    while (start < text.size())
    {
        const std::size_t end = text.find('\n', start);
        const std::size_t next = end == std::string_view::npos ? text.size() : end + 1;
        lines.push_back(text.substr(start, next - start));
        start = next;
    }
    return lines;
}

std::optional<double> ParseNumber(std::string_view text)
{
    std::istringstream stream{std::string(text)};
    stream.imbue(std::locale::classic());
    double value = 0.0;
    stream >> value;
    if (!stream || stream.peek() != std::char_traits<char>::eof() || !std::isfinite(value))
    {
        return std::nullopt;
    }
    return value;
}

class IssueList
{
public:
    explicit IssueList(std::vector<std::string>& issues) : m_issues(issues) {}
    ~IssueList()
    {
        if (m_dropped > 0)
        {
            m_issues.push_back("[EnhancedTuning]: " + std::to_string(m_dropped) + " more ignored keys");
        }
    }
    IssueList(const IssueList&) = delete;
    IssueList& operator=(const IssueList&) = delete;

    // The last line of the list is kept for the count of the keys left out.
    void Ignored(std::string_view key, const std::string& reason)
    {
        if (m_reported + 1 >= MaxPersonalTuningIssues)
        {
            ++m_dropped;
            return;
        }
        m_issues.push_back("[EnhancedTuning] " + std::string(key) + ": " + reason +
                           "; the look profile's value applies");
        ++m_reported;
    }

private:
    std::vector<std::string>& m_issues;
    std::size_t m_reported = 0;
    std::size_t m_dropped = 0;
};

void ReadEntry(std::string_view key, std::string_view text, PersonalTuning& tuning, IssueList& issues)
{
    const std::optional<double> number = ParseNumber(text);
    if (key == MoodStrengthKey)
    {
        if (tuning.moodStrengthPercent)
        {
            issues.Ignored(key, "is in the section more than once");
            return;
        }
        if (!number || *number != std::floor(*number) || *number < MinMoodStrengthPercent ||
            *number > MaxMoodStrengthPercent)
        {
            issues.Ignored(key, "is \"" + std::string(text) + "\", must be a whole number from " +
                                    std::to_string(MinMoodStrengthPercent) + " to " +
                                    std::to_string(MaxMoodStrengthPercent));
            return;
        }
        tuning.moodStrengthPercent = static_cast<int>(*number);
        return;
    }
    const std::optional<LookValue> value = Data::Maps::FindLookValue(key);
    if (!value)
    {
        issues.Ignored(key, "is not a tuning value");
        return;
    }
    if (tuning.look[*value])
    {
        issues.Ignored(key, "is in the section more than once");
        return;
    }
    const LookValueInfo& info = Data::Maps::Info(*value);
    if (!number || !Data::Maps::IsInRange(info, *number))
    {
        issues.Ignored(key, "is \"" + std::string(text) + "\", must be a number from " + FormatTuningNumber(info.min) +
                                " to " + FormatTuningNumber(info.max));
        return;
    }
    tuning.look[*value] = number;
}

std::string LineBreak(std::string_view text)
{
    return text.find("\r\n") != std::string_view::npos ? "\r\n" : "\n";
}

// A key a save changes: its new value, or nothing when the key goes.
struct KeyChange
{
    std::string key;
    std::optional<std::string> value;
    bool written = false;
};

std::vector<KeyChange> Changes(const PersonalTuning& from, const PersonalTuning& to)
{
    std::vector<KeyChange> changes;
    for (const LookValueInfo& info : Data::Maps::LookValueInfos())
    {
        const std::optional<double>& value = to.look[info.value];
        if (value != from.look[info.value])
        {
            changes.push_back(
                {std::string(info.name), value ? std::optional(FormatTuningNumber(*value)) : std::nullopt});
        }
    }
    if (to.moodStrengthPercent != from.moodStrengthPercent)
    {
        changes.push_back({std::string(MoodStrengthKey), to.moodStrengthPercent
                                                             ? std::optional(std::to_string(*to.moodStrengthPercent))
                                                             : std::nullopt});
    }
    return changes;
}

// The key of an entry line, if the line is one.
std::optional<std::string_view> EntryKey(std::string_view line)
{
    const std::string_view trimmed = Trim(line);
    const std::size_t equals = trimmed.find('=');
    if (trimmed.empty() || trimmed.front() == ';' || trimmed.front() == '#' || equals == std::string_view::npos)
    {
        return std::nullopt;
    }
    return Trim(trimmed.substr(0, equals));
}

std::filesystem::path ResolveConfigFile(const std::filesystem::path& configFile)
{
#ifdef _WIN32
    return configFile;
#else
    // The same file the private-profile shim opens (Core/Platform/WinIni.cpp).
    return std::filesystem::path(MuResolvePath(configFile.generic_string().c_str()));
#endif
}

std::optional<std::string> ReadWholeFile(const std::filesystem::path& file)
{
    std::ifstream stream(file, std::ios::binary);
    if (!stream)
    {
        return std::nullopt;
    }
    std::string text((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    if (stream.bad())
    {
        return std::nullopt;
    }
    return text;
}

bool WriteWholeFile(const std::filesystem::path& file, const std::string& text)
{
    std::ofstream stream(file, std::ios::binary | std::ios::trunc);
    if (!stream)
    {
        return false;
    }
    stream.write(text.data(), static_cast<std::streamsize>(text.size()));
    stream.close();
    return static_cast<bool>(stream);
}

bool IsUtf16(std::string_view text)
{
    return text.starts_with("\xFF\xFE") || text.starts_with("\xFE\xFF");
}
} // namespace

bool PersonalTuning::Empty() const
{
    return !moodStrengthPercent &&
           std::none_of(look.values.begin(), look.values.end(), [](const auto& value) { return value.has_value(); });
}

std::string FormatTuningNumber(double value)
{
    std::ostringstream text;
    text.imbue(std::locale::classic());
    text << value;
    return text.str();
}

double NormalizeTuningNumber(double value)
{
    return ParseNumber(FormatTuningNumber(value)).value_or(value);
}

PersonalTuning ReadPersonalTuning(std::string_view configText, std::vector<std::string>& issues)
{
    PersonalTuning tuning;
    IssueList list(issues);
    bool inSection = false;
    for (const std::string_view line : SplitLines(configText))
    {
        if (const std::optional<std::string_view> name = SectionName(line))
        {
            inSection = EqualsIgnoringCase(*name, PersonalTuningSection);
            continue;
        }
        const std::string_view trimmed = Trim(line);
        if (!inSection || trimmed.empty() || trimmed.front() == ';' || trimmed.front() == '#')
        {
            continue;
        }
        const std::size_t equals = trimmed.find('=');
        if (equals == std::string_view::npos)
        {
            list.Ignored(trimmed, "has no value");
            continue;
        }
        ReadEntry(Trim(trimmed.substr(0, equals)), Trim(trimmed.substr(equals + 1)), tuning, list);
    }
    return tuning;
}

std::string UpdatePersonalTuningSection(std::string_view configText, const PersonalTuning& from,
                                        const PersonalTuning& to)
{
    std::vector<KeyChange> changes = Changes(from, to);
    if (changes.empty())
    {
        return std::string(configText);
    }
    const std::string lineBreak = LineBreak(configText);
    const auto entry = [&lineBreak](const KeyChange& change) { return change.key + "=" + *change.value + lineBreak; };
    // The changed keys the file did not have yet.
    const auto newEntries = [&changes, &entry]
    {
        std::string text;
        for (KeyChange& change : changes)
        {
            if (change.value && !change.written)
            {
                text += entry(change);
                change.written = true;
            }
        }
        return text;
    };

    std::string result;
    result.reserve(configText.size());
    std::string section;        // the tuning section being read, from its header on
    std::size_t sectionEnd = 0; // in `section`, after its last line that is not blank
    bool inSection = false;
    bool sectionSeen = false;
    bool hasLines = false; // the section keeps a line besides its header
    // New keys go after the first section's last line; a section left with nothing goes.
    const auto closeSection = [&]
    {
        if (!sectionSeen)
        {
            std::string added = newEntries();
            if (!added.empty())
            {
                if (section[sectionEnd - 1] != '\n')
                {
                    added.insert(0, lineBreak);
                }
                section.insert(sectionEnd, added);
                hasLines = true;
            }
            sectionSeen = true;
        }
        if (hasLines)
        {
            result += section;
        }
        section.clear();
    };

    for (const std::string_view line : SplitLines(configText))
    {
        if (SectionName(line))
        {
            if (inSection)
            {
                closeSection();
            }
            inSection = IsTuningHeader(line);
            if (inSection)
            {
                section = line;
                sectionEnd = section.size();
                hasLines = false;
                continue;
            }
        }
        if (!inSection)
        {
            result += line;
            continue;
        }
        const std::optional<std::string_view> key = EntryKey(line);
        const auto change = key ? std::find_if(changes.begin(), changes.end(),
                                               [&key](const KeyChange& candidate) { return candidate.key == *key; })
                                : changes.end();
        if (change == changes.end())
        {
            section += line;
        }
        else if (change->value && !change->written)
        {
            section += entry(*change);
            change->written = true;
        }
        else
        {
            continue; // the key goes, or a later copy of a key already written
        }
        if (!Trim(line).empty())
        {
            hasLines = true;
            sectionEnd = section.size();
        }
    }
    if (inSection)
    {
        closeSection();
    }
    if (!sectionSeen)
    {
        const std::string added = newEntries();
        if (!added.empty())
        {
            if (!result.empty() && result.back() != '\n')
            {
                result += lineBreak;
            }
            result += "[" + std::string(PersonalTuningSection) + "]" + lineBreak + added;
        }
    }
    return result;
}

PersonalTuning LoadPersonalTuningFile(const std::filesystem::path& configFile, std::vector<std::string>& issues)
{
    const std::optional<std::string> text = ReadWholeFile(ResolveConfigFile(configFile));
    if (!text)
    {
        return {};
    }
    return ReadPersonalTuning(*text, issues);
}

PersonalTuningSaveResult SavePersonalTuningFile(const std::filesystem::path& configFile, const PersonalTuning& from,
                                                const PersonalTuning& to)
{
    const std::filesystem::path file = ResolveConfigFile(configFile);
    const std::string name = file.generic_string();
    std::string previous;
    std::error_code error;
    if (std::filesystem::exists(file, error) || error)
    {
        std::optional<std::string> text = ReadWholeFile(file);
        if (!text)
        {
            return {false, name + " could not be read"};
        }
        previous = std::move(*text);
    }
    if (IsUtf16(previous))
    {
        return {false, name + " is saved as UTF-16, which the tuning section cannot be written into"};
    }

    const std::string replaced = UpdatePersonalTuningSection(previous, from, to);
    if (replaced == previous)
    {
        return {true, {}};
    }
    std::filesystem::path temporary = file;
    temporary += TemporarySuffix;
    const auto fail = [&temporary, &name](const std::string& why)
    {
        std::error_code ignored;
        if (!std::filesystem::is_directory(temporary, ignored))
        {
            std::filesystem::remove(temporary, ignored);
        }
        return PersonalTuningSaveResult{false, name + ": " + why + "; the file was left as it was"};
    };
    if (!WriteWholeFile(temporary, replaced))
    {
        return fail("could not write " + temporary.filename().generic_string());
    }
    if (ReadWholeFile(temporary) != replaced)
    {
        return fail(temporary.filename().generic_string() + " did not read back as written");
    }
    std::filesystem::rename(temporary, file, error);
    if (error)
    {
        return fail("could not replace it (" + error.message() + ")");
    }
    return {true, {}};
}

} // namespace Data::Config
