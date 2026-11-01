#include "stdafx.h"

#ifdef _EDITOR

#include "ToolGroups.h"

#include "MuEditorToolWindows.h"
#include "MuEditorWindowIds.h"
#include "../MuEditor/UI/Colors/MuColorsUI.h"
#include "I18N/All.h"

#include <array>

namespace MuEditor
{
namespace
{
void ShowGlowColors()
{
    g_MuColorsUI.Show(Colors::ColorsTab::Glow);
}

bool IsGlowColorsShown()
{
    return g_MuColorsUI.GetShownTab() == Colors::ColorsTab::Glow;
}

const std::array<ToolWindowEntry, 9> ItemToolEntries = {{
    {WindowIds::ItemBasics, &I18N::Editor::ItemBasics, &ToolWindows::itemBasics},
    {WindowIds::ItemRequirements, &I18N::Editor::ItemRequirements, &ToolWindows::itemRequirements},
    {WindowIds::ItemRules, &I18N::Editor::ItemRules, &ToolWindows::itemRules},
    {WindowIds::ItemCategories, &I18N::Editor::ItemCategories, &ToolWindows::itemCategories},
    {WindowIds::ItemModel, &I18N::Editor::ItemModel, &ToolWindows::itemModel},
    // A tab of the Colors window, which docks with the item tools.
    {WindowIds::Colors, &I18N::Editor::GlowColors, &ToolWindows::colors, &ShowGlowColors, &IsGlowColorsShown},
    {WindowIds::ItemFormulas, &I18N::Editor::ItemFormulas, &ToolWindows::itemFormulas},
    {WindowIds::ItemSync, &I18N::Editor::ItemSync, &ToolWindows::itemSync},
    {WindowIds::ItemTranslations, &I18N::Editor::ItemTranslations, &ToolWindows::itemTranslations},
}};

const std::array<ToolWindowEntry, 4> MonsterToolEntries = {{
    {WindowIds::MonsterModel, &I18N::Editor::MonsterModel, &ToolWindows::monsterModel},
    {WindowIds::MonsterPreview, &I18N::Editor::MonsterPreview, &ToolWindows::monsterPreview},
    {WindowIds::MonsterAttacks, &I18N::Editor::MonsterAttacks, &ToolWindows::monsterAttacks},
    {WindowIds::MonsterTranslations, &I18N::Editor::MonsterTranslations, &ToolWindows::monsterTranslations},
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
