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
    return key == NameKey ? MonsterTool::Translations : MonsterTool::Browser;
}

const ToolWindowEntry& GetToolWindow(MonsterTool tool)
{
    const ToolGroup& tools = GetMonsterTools();
    if (tool == MonsterTool::Translations)
    {
        for (const ToolWindowEntry& entry : tools.tools)
        {
            if (std::string_view(entry.windowId) == WindowIds::MonsterTranslations)
            {
                return entry;
            }
        }
    }
    return tools.browser;
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
