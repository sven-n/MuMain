#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterShownModel.h"

#include "MonsterSelection.h"

#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
#include "Data/DataHandler/MonsterData/MonsterModelLoader.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "I18N/All.h"
#include "World/MapInfra/MapManager.h"
#include "imgui.h"

namespace MuEditor::Monsters::ShownModel
{
namespace
{
constexpr int NoSlot = -1;

// The slot asked for last, and the map it was asked on.
int g_askedSlot = NoSlot;
int g_askedWorld = -1;
} // namespace

std::optional<int> FindSlot(const Data::Monsters::MonsterDefinition& monster)
{
    const std::optional<int> slot = Data::Monsters::FindMonsterModelReference(monster.model);
    if (!slot || g_MonsterModelDatabase.Find(*slot) == nullptr)
    {
        return std::nullopt;
    }
    return slot;
}

bool IsOpen(int slot)
{
    return Data::Monsters::ModelLoader::IsOpen(slot);
}

void OpenWhenShown(int slot)
{
    if (IsOpen(slot) || (slot == g_askedSlot && gMapManager.WorldActive == g_askedWorld))
    {
        return;
    }
    g_askedSlot = slot;
    g_askedWorld = gMapManager.WorldActive;
    g_MonsterDataHandler.OpenModelForTools(slot);
}

void RenderSelectedMonster(int number, const Data::Monsters::MonsterDefinition* monster)
{
    if (monster == nullptr)
    {
        ImGui::TextDisabled("%s", number == MonsterSelection::NoMonster ? I18N::Editor::NoMonsterIsSelected
                                                                        : I18N::Editor::NoMonsterAtThisNumber);
        return;
    }
    // The name is an argument, never the format: names may hold a "%".
    ImGui::Text("%d %s", number, monster->names.Get(I18N::GetCurrentLocale()).c_str());
}

void RenderWhyNoModel(const Data::Monsters::MonsterDefinition& monster)
{
    ImGui::PushTextWrapPos(0.0f);
    if (monster.model.empty())
    {
        ImGui::TextDisabled("%s", I18N::Editor::TheMonsterNamesNoModel);
    }
    else
    {
        ImGui::TextDisabled(
            "%s", I18N::Format(I18N::Editor::TheMonsterAppearsWith0WhichIsNotInTheModelList, {monster.model}).c_str());
    }
    ImGui::PopTextWrapPos();
}
} // namespace MuEditor::Monsters::ShownModel

#endif // _EDITOR
