#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterCheckTargets.h"

#include "../MuEditor/UI/Common/MuEditorWindowIds.h"

#include <charconv>

namespace MuEditor::Monsters
{
namespace
{
// How the monster list's reader and checks name a field of one monster
// (MonstersJson, MonsterDataValidation): "monster <number> <key>".
constexpr std::string_view MonsterPrefix = "monster ";
constexpr std::string_view Digits = "0123456789";
constexpr char AfterNumber = ' ';

// The key ends at a language ("name.de") or an index.
constexpr std::string_view KeyEnds = ".[";
constexpr std::string_view NameKey = "name";
constexpr std::string_view AttacksKey = "attacks";
// The map variants hold attacks only yet (monsters design, Q-MN65).
constexpr std::string_view MapVariantsKey = "mapVariants";
constexpr char IndexStart = '[';
constexpr char IndexEnd = ']';

const ToolWindowEntry* FindTool(const char* windowId)
{
    for (const ToolWindowEntry& entry : GetMonsterTools().tools)
    {
        if (std::string_view(entry.windowId) == windowId)
        {
            return &entry;
        }
    }
    return nullptr;
}
} // namespace

std::string_view GetMonsterField(std::string_view field)
{
    if (!field.starts_with(MonsterPrefix))
    {
        return field;
    }
    const std::string_view afterPrefix = field.substr(MonsterPrefix.size());
    const size_t numberEnd = afterPrefix.find_first_not_of(Digits);
    if (numberEnd == 0 || numberEnd == std::string_view::npos || afterPrefix[numberEnd] != AfterNumber)
    {
        return field;
    }
    return afterPrefix.substr(numberEnd + 1);
}

MonsterTool GetToolForField(std::string_view field)
{
    const std::string_view monsterField = GetMonsterField(field);
    const std::string_view key = monsterField.substr(0, monsterField.find_first_of(KeyEnds));
    if (key == NameKey)
    {
        return MonsterTool::Translations;
    }
    return key == AttacksKey || key == MapVariantsKey ? MonsterTool::Attacks : MonsterTool::Browser;
}

std::optional<int> GetMapVariantOfField(std::string_view field)
{
    const std::string_view monsterField = GetMonsterField(field);
    if (!monsterField.starts_with(MapVariantsKey) || monsterField.size() <= MapVariantsKey.size() ||
        monsterField[MapVariantsKey.size()] != IndexStart)
    {
        return std::nullopt;
    }
    const std::string_view index = monsterField.substr(MapVariantsKey.size() + 1);
    int variant = 0;
    const auto [end, error] = std::from_chars(index.data(), index.data() + index.size(), variant);
    if (error != std::errc() || end == index.data() || end == index.data() + index.size() || *end != IndexEnd)
    {
        return std::nullopt;
    }
    return variant;
}

const ToolWindowEntry& GetToolWindow(MonsterTool tool)
{
    const ToolWindowEntry* entry = nullptr;
    if (tool == MonsterTool::Translations)
    {
        entry = FindTool(WindowIds::MonsterTranslations);
    }
    else if (tool == MonsterTool::Attacks)
    {
        entry = FindTool(WindowIds::MonsterAttacks);
    }
    return entry != nullptr ? *entry : GetMonsterTools().browser;
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
