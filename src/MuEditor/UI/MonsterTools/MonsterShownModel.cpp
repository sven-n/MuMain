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

#include <set>
#include <string_view>

namespace MuEditor::Monsters::ShownModel
{
namespace
{
// The slots asked for on this map and with this model table: two windows
// that show two models whose files are missing ask once each, not by turns
// each frame. A new map or a changed table asks again.
std::set<int> g_askedSlots;
int g_askedWorld = -1;
int g_askedVersion = -1;
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
    if (gMapManager.WorldActive != g_askedWorld || g_MonsterModelDatabase.GetVersion() != g_askedVersion)
    {
        g_askedSlots.clear();
        g_askedWorld = gMapManager.WorldActive;
        g_askedVersion = g_MonsterModelDatabase.GetVersion();
    }
    if (IsOpen(slot) || !g_askedSlots.insert(slot).second)
    {
        return;
    }
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
    else if (monster.model.starts_with(Data::Monsters::MapObjectPrefix))
    {
        ImGui::TextDisabled("%s", I18N::Format(I18N::Editor::MonsterModelMapObjectNote,
                                               {monster.model.substr(Data::Monsters::MapObjectPrefix.size())})
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
