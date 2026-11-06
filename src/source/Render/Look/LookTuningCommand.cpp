#include "Render/Look/LookTuningCommand.h"

#include <cwchar>
#include <initializer_list>
#include <vector>

namespace Render::Look
{
namespace
{
using Data::Maps::LookValue;

struct CommandForm
{
    std::wstring_view prefix;
    std::initializer_list<LookValue> values;
};

// "$bloom falloff " before "$bloom ": the first matching prefix whose numbers fit wins.
const CommandForm CommandForms[] = {
    {L"$lighting rim ", {LookValue::RimStrength}},
    {L"$lighting specular ", {LookValue::SpecularScale}},
    {L"$lighting diffuse ", {LookValue::DiffuseScale, LookValue::DiffuseAmbient, LookValue::DiffuseFloor}},
    {L"$lightmood knee ", {LookValue::HighlightKnee}},
    {L"$dynlights wrap ", {LookValue::DynamicWrap}},
    {L"$dynlights smooth ", {LookValue::DynamicMapSmoothing}},
    {L"$dynlights spell ", {LookValue::DynamicEffectGain, LookValue::DynamicEffectRange, LookValue::DynamicEffectFade}},
    {L"$dynlights cap ", {LookValue::DynamicEffectCap}},
    {L"$dynlights reach ", {LookValue::DynamicEffectRangeMax}},
    {L"$bloom falloff ", {LookValue::BloomLevelFalloff}},
    {L"$bloom ", {LookValue::BloomStrength}},
};

// One number, the whole word.
std::optional<double> ParseNumber(std::wstring_view word)
{
    if (word.empty())
    {
        return std::nullopt;
    }
    const std::wstring copy(word);
    wchar_t* end = nullptr;
    const double number = std::wcstod(copy.c_str(), &end);
    if (end != copy.c_str() + copy.size())
    {
        return std::nullopt;
    }
    return number;
}

std::vector<std::wstring_view> Words(std::wstring_view text)
{
    std::vector<std::wstring_view> words;
    std::size_t start = 0;
    while (start < text.size())
    {
        while (start < text.size() && text[start] == L' ')
        {
            ++start;
        }
        std::size_t end = start;
        while (end < text.size() && text[end] != L' ')
        {
            ++end;
        }
        if (end > start)
        {
            words.push_back(text.substr(start, end - start));
        }
        start = end;
    }
    return words;
}

// Names and candidates are plain ASCII.
std::optional<std::string> Ascii(std::wstring_view word)
{
    std::string text;
    for (const wchar_t character : word)
    {
        if (character <= L' ' || character > L'~')
        {
            return std::nullopt;
        }
        text.push_back(static_cast<char>(character));
    }
    return text;
}

bool IsCandidateName(const std::string& name)
{
    if (name.empty() || name == "." || name == "..")
    {
        return false;
    }
    for (const char character : name)
    {
        const bool allowed = (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z') ||
                             (character >= '0' && character <= '9') || character == '-' || character == '_' ||
                             character == '.';
        if (!allowed)
        {
            return false;
        }
    }
    return true;
}

std::optional<LookTuningCommand> ParseNumbers(std::wstring_view text, std::initializer_list<LookValue> values)
{
    const std::vector<std::wstring_view> words = Words(text);
    if (words.size() != values.size())
    {
        return std::nullopt;
    }
    LookTuningCommand command;
    for (const LookValue value : values)
    {
        const std::optional<double> number = ParseNumber(words[command.count]);
        if (!number)
        {
            return std::nullopt;
        }
        command.assignments[command.count++] = {value, *number};
    }
    return command;
}
} // namespace

std::optional<LookTuningCommand> ParseLookTuningCommand(std::wstring_view command)
{
    for (const CommandForm& form : CommandForms)
    {
        if (!command.starts_with(form.prefix))
        {
            continue;
        }
        std::optional<LookTuningCommand> parsed = ParseNumbers(command.substr(form.prefix.size()), form.values);
        if (parsed)
        {
            return parsed;
        }
    }
    return std::nullopt;
}

std::optional<LookCommand> ParseLookCommand(std::wstring_view command)
{
    const std::vector<std::wstring_view> words = Words(command);
    if (words.size() < 2 || words[0] != L"$look")
    {
        return std::nullopt;
    }
    const std::wstring_view verb = words[1];
    const std::size_t arguments = words.size() - 2;
    LookCommand parsed;
    if (verb == L"save" && arguments == 0)
    {
        parsed.verb = LookVerb::Save;
        return parsed;
    }
    if (verb == L"show" || verb == L"reset" || verb == L"load")
    {
        parsed.verb = verb == L"show" ? LookVerb::Show : verb == L"reset" ? LookVerb::Reset : LookVerb::Load;
        if (arguments > 1)
        {
            return std::nullopt;
        }
        if (arguments == 1)
        {
            const std::optional<std::string> name = Ascii(words[2]);
            if (!name || (parsed.verb == LookVerb::Load && !IsCandidateName(*name)))
            {
                return std::nullopt;
            }
            parsed.name = *name;
        }
        return parsed;
    }
    if (verb == L"set")
    {
        parsed.verb = LookVerb::Set;
        std::size_t next = 2;
        if (arguments == 3 && words[2] == L"map")
        {
            parsed.map = true;
            next = 3;
        }
        else if (arguments != 2)
        {
            return std::nullopt;
        }
        const std::optional<std::string> name = Ascii(words[next]);
        const std::optional<double> value = ParseNumber(words[next + 1]);
        if (!name || !value)
        {
            return std::nullopt;
        }
        parsed.name = *name;
        parsed.value = *value;
        return parsed;
    }
    return std::nullopt;
}

} // namespace Render::Look
