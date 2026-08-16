#include "stdafx.h"
#include "GameLogic/Commands/ChatCommandTemplate.h"

#include "Data/GameConfig/GameConfig.h"

#include <algorithm>

namespace GameLogic::Commands::Templates
{
namespace
{
    // The parts of a stored entry are separated by a pipe, which is why a value
    // must not contain one.
    constexpr wchar_t Separator = L'|';

    std::wstring WithoutSeparator(const std::wstring& value)
    {
        std::wstring sanitized = value;
        std::replace(sanitized.begin(), sanitized.end(), Separator, L' ');
        return sanitized;
    }

    std::vector<std::wstring> Split(const std::wstring& line)
    {
        std::vector<std::wstring> parts;
        size_t start = 0;
        while (true)
        {
            const auto separator = line.find(Separator, start);
            if (separator == std::wstring::npos)
            {
                parts.push_back(line.substr(start));
                return parts;
            }

            parts.push_back(line.substr(start, separator - start));
            start = separator + 1;
        }
    }

    std::wstring Serialize(const ChatCommandTemplate& entry)
    {
        std::wstring line = WithoutSeparator(entry.Label);
        line += Separator;
        line += WithoutSeparator(entry.Command);
        for (const auto& value : entry.Values)
        {
            line += Separator;
            line += WithoutSeparator(value);
        }

        return line;
    }

    bool TryParse(const std::wstring& line, ChatCommandTemplate& entry)
    {
        const auto parts = Split(line);
        if (parts.size() < 2 || parts[0].empty() || parts[1].empty())
        {
            return false;
        }

        entry.Label = parts[0];
        entry.Command = parts[1];
        entry.Values.assign(parts.begin() + 2, parts.end());
        return true;
    }
}

std::vector<ChatCommandTemplate> GetAll()
{
    std::vector<ChatCommandTemplate> entries;
    for (const auto& line : GameConfig::GetInstance().GetChatCommandTemplates())
    {
        ChatCommandTemplate entry;
        if (TryParse(line, entry))
        {
            entries.push_back(std::move(entry));
        }
    }

    return entries;
}

void Add(const ChatCommandTemplate& entry)
{
    if (entry.Label.empty() || entry.Command.empty())
    {
        return;
    }

    auto lines = GameConfig::GetInstance().GetChatCommandTemplates();
    const auto line = Serialize(entry);

    // The same command line twice would only make the list longer.
    if (std::find(lines.begin(), lines.end(), line) != lines.end())
    {
        return;
    }

    lines.push_back(line);
    GameConfig::GetInstance().SetChatCommandTemplates(lines);
}

void RemoveAt(size_t index)
{
    auto lines = GameConfig::GetInstance().GetChatCommandTemplates();

    // The index counts the entries which GetAll returned, so lines which
    // couldn't be parsed have to be skipped here as well.
    size_t remaining = index;
    for (auto line = lines.begin(); line != lines.end(); ++line)
    {
        ChatCommandTemplate parsed;
        if (!TryParse(*line, parsed))
        {
            continue;
        }

        if (remaining == 0)
        {
            lines.erase(line);
            GameConfig::GetInstance().SetChatCommandTemplates(lines);
            return;
        }

        --remaining;
    }
}
}
