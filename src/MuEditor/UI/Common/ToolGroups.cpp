#include "stdafx.h"

#ifdef _EDITOR

#include "ToolGroups.h"

#include "MuEditorToolWindows.h"
#include "MuEditorWindowIds.h"
#include "I18N/All.h"

#include <array>

namespace MuEditor
{
namespace
{
const std::array<ToolWindowEntry, 10> ItemToolEntries = {{
    {WindowIds::ItemBasics, &I18N::Editor::ItemBasics, &ToolWindows::itemBasics},
    {WindowIds::ItemRequirements, &I18N::Editor::ItemRequirements, &ToolWindows::itemRequirements},
    {WindowIds::ItemRules, &I18N::Editor::ItemRules, &ToolWindows::itemRules},
    {WindowIds::ItemCategories, &I18N::Editor::ItemCategories, &ToolWindows::itemCategories},
    {WindowIds::ItemModel, &I18N::Editor::ItemModel, &ToolWindows::itemModel},
    {WindowIds::ItemGlowColors, &I18N::Editor::GlowColors, &ToolWindows::itemGlowColors},
    {WindowIds::ItemFormulas, &I18N::Editor::ItemFormulas, &ToolWindows::itemFormulas},
    {WindowIds::ItemSync, &I18N::Editor::ItemSync, &ToolWindows::itemSync},
    {WindowIds::ItemChecks, &I18N::Editor::ItemChecks, &ToolWindows::itemChecks},
    {WindowIds::ItemTranslations, &I18N::Editor::ItemTranslations, &ToolWindows::itemTranslations},
}};

const std::array<ToolWindowEntry, 5> MonsterToolEntries = {{
    {WindowIds::MonsterModel, &I18N::Editor::MonsterModel, &ToolWindows::monsterModel},
    {WindowIds::MonsterPreview, &I18N::Editor::MonsterPreview, &ToolWindows::monsterPreview},
    {WindowIds::MonsterAttacks, &I18N::Editor::MonsterAttacks, &ToolWindows::monsterAttacks},
    {WindowIds::MonsterTranslations, &I18N::Editor::MonsterTranslations, &ToolWindows::monsterTranslations},
    {WindowIds::MonsterChecks, &I18N::Editor::MonsterChecks, &ToolWindows::monsterChecks},
}};
} // namespace

bool ToolGroup::IsAnyOpen(const ToolWindows& windows) const
{
    if (windows.*(browser.open))
    {
        return true;
    }
    for (const ToolWindowEntry& tool : tools)
    {
        if (windows.*(tool.open))
        {
            return true;
        }
    }
    return false;
}

const ToolGroup& GetItemTools()
{
    static const ToolGroup group{&I18N::Editor::Items,
                                 {WindowIds::ItemBrowser, &I18N::Editor::ItemBrowser, &ToolWindows::itemBrowser},
                                 ItemToolEntries};
    return group;
}

const ToolGroup& GetMonsterTools()
{
    static const ToolGroup group{
        &I18N::Editor::Monsters,
        {WindowIds::MonsterBrowser, &I18N::Editor::MonsterBrowser, &ToolWindows::monsterBrowser},
        MonsterToolEntries,
        &GetItemTools()};
    return group;
}
} // namespace MuEditor

#endif // _EDITOR
