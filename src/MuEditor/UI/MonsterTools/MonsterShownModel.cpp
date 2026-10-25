#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterShownModel.h"

#include "MonsterSelection.h"

#include "Core/Globals/_enum.h"
#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
#include "Data/DataHandler/MonsterData/MonsterModelLoader.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "I18N/All.h"
#include "World/MapInfra/MapManager.h"
#include "imgui.h"

#include <string_view>

namespace MuEditor::Monsters::ShownModel
{
namespace
{
constexpr int NoSlot = -1;
// How a monster's model names a map object (MonsterModelSlots).
constexpr std::string_view MapObjectPrefix = "mapObject ";

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
    const std::optional<int> slot = Data::Monsters::FindMonsterModelReference(monster.model);
    if (monster.model.empty())
    {
        ImGui::TextDisabled("%s", I18N::Editor::TheMonsterNamesNoModel);
    }
    else if (slot == MODEL_PLAYER)
    {
        // CreateMonster makes it a player character (MODEL_PLAYER), the
        // skeletons with their body as its skin (SubType).
        ImGui::TextDisabled("%s", I18N::Editor::MonsterModelPlayerNote);
    }
    else if (monster.model.starts_with(MapObjectPrefix))
    {
        ImGui::TextDisabled(
            "%s", I18N::Format(I18N::Editor::MonsterModelMapObjectNote, {monster.model.substr(MapObjectPrefix.size())})
                      .c_str());
    }
    else if (slot && !Data::Monsters::IsMonsterRangeSlot(*slot) && !Data::Monsters::IsNpcRangeSlot(*slot))
    {
        ImGui::TextDisabled("%s", I18N::Format(I18N::Editor::MonsterModelOtherAreaNote, {monster.model}).c_str());
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
