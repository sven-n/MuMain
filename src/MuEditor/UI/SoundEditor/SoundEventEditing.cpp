#include "stdafx.h"

#ifdef _EDITOR

#include "SoundEventEditing.h"

#include "SoundEditorRows.h"

#include "Audio/SoundIds.h"
#include "I18N/All.h"

#include <algorithm>
#include <cctype>
#include <set>

namespace MuEditor::SoundEditor
{
namespace
{
std::string_view Trimmed(std::string_view text)
{
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front())))
    {
        text.remove_prefix(1);
    }
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back())))
    {
        text.remove_suffix(1);
    }
    return text;
}

std::optional<int> ParseWhole(std::string_view text)
{
    text = Trimmed(text);
    if (text.empty() || text.size() > 6 ||
        !std::all_of(text.begin(), text.end(), [](char c) { return std::isdigit(static_cast<unsigned char>(c)); }))
    {
        return std::nullopt;
    }
    return std::stoi(std::string(text));
}
} // namespace

std::string NumbersText(const std::vector<int>& numbers)
{
    std::vector<int> sorted(numbers);
    std::sort(sorted.begin(), sorted.end());
    sorted.erase(std::unique(sorted.begin(), sorted.end()), sorted.end());
    std::string text;
    for (size_t start = 0; start < sorted.size();)
    {
        size_t end = start;
        while (end + 1 < sorted.size() && sorted[end + 1] == sorted[end] + 1)
        {
            ++end;
        }
        text += text.empty() ? "" : ", ";
        if (end - start >= 2)
        {
            text += std::to_string(sorted[start]) + "-" + std::to_string(sorted[end]);
            start = end + 1;
        }
        else
        {
            text += std::to_string(sorted[start]);
            ++start;
        }
    }
    return text;
}

std::optional<std::vector<int>> ParseNumbers(std::string_view text, int highest)
{
    std::set<int> numbers;
    text = Trimmed(text);
    while (!text.empty())
    {
        const size_t comma = text.find(',');
        const std::string_view part = Trimmed(text.substr(0, comma));
        text = comma == std::string_view::npos ? std::string_view() : text.substr(comma + 1);
        const size_t dash = part.find('-');
        const std::optional<int> from = ParseWhole(part.substr(0, dash));
        const std::optional<int> to = dash == std::string_view::npos ? from : ParseWhole(part.substr(dash + 1));
        if (!from || !to || *from > *to || *to > highest)
        {
            return std::nullopt;
        }
        for (int number = *from; number <= *to; ++number)
        {
            numbers.insert(number);
        }
    }
    return std::vector<int>(numbers.begin(), numbers.end());
}

std::string KindsText(const std::vector<std::string>& kinds)
{
    std::string text;
    for (const std::string& kind : kinds)
    {
        text += (text.empty() ? "" : ", ") + kind;
    }
    return text;
}

std::optional<std::vector<std::string>> ParseKinds(std::string_view text)
{
    std::vector<std::string> kinds;
    text = Trimmed(text);
    while (!text.empty())
    {
        const size_t comma = text.find(',');
        const std::string_view part = Trimmed(text.substr(0, comma));
        text = comma == std::string_view::npos ? std::string_view() : text.substr(comma + 1);
        if (!Data::Sounds::IsClickKindWord(part))
        {
            return std::nullopt;
        }
        if (std::find(kinds.begin(), kinds.end(), part) == kinds.end())
        {
            kinds.emplace_back(part);
        }
    }
    return kinds;
}

std::string EventTitle(std::string_view event)
{
    if (event == Data::Sounds::HeroFootstepEvent)
    {
        return I18N::Editor::SoundEventHeroFootstep;
    }
    if (event == Data::Sounds::PlayerAttackSwingEvent)
    {
        return I18N::Editor::SoundEventPlayerAttackSwing;
    }
    if (event == Data::Sounds::InterfaceClickEvent)
    {
        return I18N::Editor::SoundEventInterfaceClick;
    }
    if (event == Data::Sounds::CharacterSelectedEvent)
    {
        return I18N::Editor::SoundEventCharacterSelected;
    }
    return std::string(event);
}

EventFields FieldsOf(const Data::Sounds::SoundEvent& event)
{
    EventFields fields;
    if (event.event == Data::Sounds::HeroFootstepEvent)
    {
        fields.worlds = fields.tiles = fields.safeZone = true;
    }
    else if (event.event == Data::Sounds::PlayerAttackSwingEvent)
    {
        fields.holds = true;
    }
    else if (event.event == Data::Sounds::InterfaceClickEvent)
    {
        fields.kinds = true;
    }
    else if (event.event != Data::Sounds::CharacterSelectedEvent)
    {
        fields = {true, true, true, true, true};
    }
    for (const Data::Sounds::SoundEventRule& rule : event.rules)
    {
        fields.worlds = fields.worlds || !rule.worlds.empty();
        fields.tiles = fields.tiles || !rule.tiles.empty();
        fields.safeZone = fields.safeZone || rule.safeZone.has_value();
        fields.holds = fields.holds || rule.holds.has_value();
        fields.kinds = fields.kinds || !rule.kinds.empty();
    }
    return fields;
}

std::string PlaysText(const Data::Sounds::SoundEventRule& rule)
{
    std::string text;
    for (size_t index = 0; index < rule.plays.size(); ++index)
    {
        const int number = index < rule.numbers.size() ? rule.numbers[index] : -1;
        const std::string name = number >= 0 ? NameOfNumber(number) : std::string();
        text += (text.empty() ? "" : ", ") + (name.empty() ? rule.plays[index] : name);
    }
    return text;
}

bool AddPlay(Data::Sounds::SoundEventRule& rule, const std::string& id)
{
    const std::optional<ESound> number = Audio::SoundIds::Find(id);
    if (!number || std::find(rule.plays.begin(), rule.plays.end(), id) != rule.plays.end())
    {
        return false;
    }
    rule.plays.push_back(id);
    rule.numbers.push_back(static_cast<int>(*number));
    return true;
}

bool RemovePlay(Data::Sounds::SoundEventRule& rule, size_t index)
{
    // A rule plays one sound at least.
    if (index >= rule.plays.size() || rule.plays.size() == 1)
    {
        return false;
    }
    rule.plays.erase(rule.plays.begin() + static_cast<std::ptrdiff_t>(index));
    if (index < rule.numbers.size())
    {
        rule.numbers.erase(rule.numbers.begin() + static_cast<std::ptrdiff_t>(index));
    }
    return true;
}

bool RemoveRule(Data::Sounds::SoundEvent& event, size_t index)
{
    if (index >= event.rules.size() || event.rules.size() == 1)
    {
        return false;
    }
    event.rules.erase(event.rules.begin() + static_cast<std::ptrdiff_t>(index));
    return true;
}
} // namespace MuEditor::SoundEditor

#endif // _EDITOR
