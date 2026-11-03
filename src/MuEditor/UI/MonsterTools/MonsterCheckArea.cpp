#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterCheckArea.h"

#include "MonsterCheckTargets.h"
#include "MonsterChecks.h"
#include "MonsterSelection.h"
#include "MuMonsterAttacksUI.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "I18N/All.h"
#include "imgui.h"

#include <utility>

namespace MuEditor::Monsters
{
namespace
{
using Data::Items::ItemDataIssue;

bool IsAboutAMonster(const ItemDataIssue& issue)
{
    return issue.number >= 0 && issue.number < Data::Monsters::MonsterNumberCount;
}
} // namespace

MonsterCheckArea::MonsterCheckArea(const char* const* name, CheckTables check) : m_name(name), m_check(std::move(check))
{
}

const char* MonsterCheckArea::GetName() const
{
    return *m_name;
}

void MonsterCheckArea::Check(std::vector<ItemDataIssue>& issues)
{
    m_check(issues);
}

// The name is an argument, never the format: names may hold a "%".
void MonsterCheckArea::RenderSubject(const ItemDataIssue& issue) const
{
    if (!IsAboutAMonster(issue))
    {
        ImGui::TextDisabled("%s", issue.source.c_str());
        return;
    }
    const Data::Monsters::MonsterDefinition* monster = g_MonsterDatabase.Find(issue.number);
    if (monster == nullptr)
    {
        ImGui::Text("%d", issue.number);
        return;
    }
    ImGui::Text("%d %s", issue.number, monster->names.Get(I18N::GetCurrentLocale()).c_str());
}

std::string_view MonsterCheckArea::GetField(const ItemDataIssue& issue) const
{
    return GetMonsterField(issue.field);
}

void MonsterCheckArea::Open(const ItemDataIssue& issue, bool clicked)
{
    if (!IsAboutAMonster(issue))
    {
        return;
    }
    g_MonsterSelection.Select(issue.number);
    const MonsterTool toolForField = GetToolForField(issue.field);
    if (toolForField == MonsterTool::Attacks)
    {
        // The attacks the problem is about: of all maps or of a map variant.
        g_MuMonsterAttacksUI.ShowMapVariant(issue.number, GetMapVariantOfField(issue.field));
    }
    if (!clicked)
    {
        return;
    }
    const ToolWindowEntry& tool = GetToolWindow(toolForField);
    g_MuEditorCore.GetToolWindows().*tool.open = true;
    ImGui::SetWindowFocus(tool.windowId);
}

std::unique_ptr<DataChecks::DataCheckArea> CreateMonsterCheckArea()
{
    return std::make_unique<MonsterCheckArea>(&I18N::Editor::Monsters, MonsterChecks::RunMonsters);
}

std::unique_ptr<DataChecks::DataCheckArea> CreateMonsterModelCheckArea()
{
    return std::make_unique<MonsterCheckArea>(&I18N::Editor::MonsterModels, MonsterChecks::RunModels);
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
