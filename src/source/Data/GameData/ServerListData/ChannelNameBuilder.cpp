#include "stdafx.h"

#include "ChannelNameBuilder.h"

#include "Data/GameData/ItemData/ItemDescriptionPlaceholders.h"

#include <algorithm>
#include <array>

namespace Data::ServerList
{
namespace
{
// In the order of ChannelNamePlace.
constexpr std::array<const char*, ChannelNamePlaceCount> PlaceKeys = {"button", "login", "characterInfo", "screenshot"};

constexpr std::string_view Name = "name";
constexpr std::string_view Channel = "channel";
constexpr std::string_view Vip = "vip";
constexpr std::string_view Pvp = "pvp";
constexpr std::string_view Load = "load";
constexpr std::string_view Character = "character";

constexpr std::array<std::string_view, 5> ButtonPlaceholders = {Name, Channel, Vip, Pvp, Load};
constexpr std::array<std::string_view, 4> TitlePlaceholders = {Name, Channel, Vip, Pvp};
constexpr std::array<std::string_view, 5> ScreenshotPlaceholders = {Name, Channel, Vip, Pvp, Character};

// Where an empty value was, until the spaces around it are tidied. A control
// character no template has.
constexpr char Gap = '\x01';
// Where the {load} text would go, to find whether the button's template has it. Another one.
constexpr char LoadMark = '\x02';

// The value of a placeholder of `place`, or nothing for one the place does not have.
std::optional<std::string> FindValue(std::string_view placeholder, ChannelNamePlace place,
                                     const ChannelNameValues& values)
{
    const std::span<const std::string_view> known = GetChannelNamePlaceholders(place);
    if (std::find(known.begin(), known.end(), placeholder) == known.end())
    {
        return std::nullopt;
    }
    if (placeholder == Name)
    {
        return std::string(values.name);
    }
    if (placeholder == Channel)
    {
        return std::to_string(values.channel);
    }
    if (placeholder == Vip)
    {
        return std::string(values.vip);
    }
    if (placeholder == Pvp)
    {
        return std::string(values.pvp);
    }
    if (placeholder == Load)
    {
        return std::string(values.load);
    }
    return std::string(values.character);
}

// Brackets with nothing but gaps and spaces in them, and at least one gap,
// become a gap themselves; inner ones first, so "[({vip})]" goes whole.
void FoldEmptyBrackets(std::string& text)
{
    bool folded = true;
    while (folded)
    {
        folded = false;
        for (size_t open = 0; open < text.size(); ++open)
        {
            const char close = text[open] == '(' ? ')' : (text[open] == '[' ? ']' : '\0');
            if (close == '\0')
            {
                continue;
            }
            size_t end = open + 1;
            bool hasGap = false;
            while (end < text.size() && (text[end] == ' ' || text[end] == Gap))
            {
                hasGap = hasGap || text[end] == Gap;
                ++end;
            }
            if (hasGap && end < text.size() && text[end] == close)
            {
                text.replace(open, end - open + 1, 1, Gap);
                folded = true;
            }
        }
    }
}

// Each gap goes with one space next to it: the one before, else the one after.
std::string RemoveGaps(std::string_view text)
{
    std::string result;
    result.reserve(text.size());
    for (size_t index = 0; index < text.size(); ++index)
    {
        if (text[index] != Gap)
        {
            result.push_back(text[index]);
            continue;
        }
        if (!result.empty() && result.back() == ' ')
        {
            result.pop_back();
        }
        else if (index + 1 < text.size() && text[index + 1] == ' ')
        {
            ++index;
        }
    }
    return result;
}
} // namespace

const char* GetChannelNamePlaceKey(ChannelNamePlace place)
{
    return PlaceKeys[static_cast<size_t>(place)];
}

std::span<const std::string_view> GetChannelNamePlaceholders(ChannelNamePlace place)
{
    switch (place)
    {
    case ChannelNamePlace::Button:
        return ButtonPlaceholders;
    case ChannelNamePlace::Screenshot:
        return ScreenshotPlaceholders;
    case ChannelNamePlace::Login:
    case ChannelNamePlace::CharacterInfo:
    default:
        return TitlePlaceholders;
    }
}

TemplateProblems FindTemplateProblems(std::string_view text, ChannelNamePlace place)
{
    TemplateProblems problems;
    Items::PlaceholderScan scan = Items::ScanPlaceholders(text);
    if (!scan.error.empty())
    {
        problems.braceError = std::move(scan.error);
        return problems;
    }
    const std::span<const std::string_view> known = GetChannelNamePlaceholders(place);
    for (std::string& name : scan.names)
    {
        if (std::find(known.begin(), known.end(), name) == known.end())
        {
            problems.unknownPlaceholders.push_back(std::move(name));
        }
    }
    return problems;
}

std::string FillChannelName(std::string_view text, ChannelNamePlace place, const ChannelNameValues& values)
{
    // Text from a brace problem on (FindPlaceholders' stop) stays as written.
    const Items::PlaceholderSpans found = Items::FindPlaceholders(text);
    std::string filled;
    size_t position = 0;
    for (const Items::PlaceholderSpan& span : found.spans)
    {
        filled.append(text.substr(position, span.begin - position));
        const std::optional<std::string> value = FindValue(span.name, place, values);
        if (!value)
        {
            filled.append(text.substr(span.begin, span.end - span.begin));
        }
        else if (value->empty())
        {
            filled.push_back(Gap);
        }
        else
        {
            filled.append(*value);
        }
        position = span.end;
    }
    filled.append(text.substr(position));
    FoldEmptyBrackets(filled);
    return RemoveGaps(filled);
}

std::string BuildChannelName(const ServerListData& list, ChannelNamePlace place, const ChannelSettings& channel,
                             ChannelNameValues values, std::string_view locale)
{
    const VipType* vip = channel.vip.empty() ? nullptr : list.FindVipType(channel.vip);
    const std::string noVip;
    const std::string& vipName = vip != nullptr ? vip->name.Get(locale) : noVip;
    const std::string& pvp = (channel.pvp ? list.names.pvp : list.names.nonPvp).Get(locale);
    values.vip = vipName;
    values.pvp = pvp;
    return FillChannelName(list.names.Template(place).Get(locale), place, values);
}

std::string GetButtonLoadText(const ServerListData& list, std::string_view load, std::string_view locale)
{
    if (load.empty())
    {
        return {};
    }
    const std::string mark(1, LoadMark);
    ChannelNameValues values;
    values.load = mark;
    const std::string filled =
        FillChannelName(list.names.Template(ChannelNamePlace::Button).Get(locale), ChannelNamePlace::Button, values);
    return filled.find(LoadMark) != std::string::npos ? std::string(load) : std::string();
}
} // namespace Data::ServerList
