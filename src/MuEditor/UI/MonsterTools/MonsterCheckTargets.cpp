#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterCheckTargets.h"

#include "../MuEditor/UI/Common/MuEditorWindowIds.h"

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
    return key == AttacksKey ? MonsterTool::Attacks : MonsterTool::Browser;
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
