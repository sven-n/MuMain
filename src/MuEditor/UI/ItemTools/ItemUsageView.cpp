#include "stdafx.h"

#ifdef _EDITOR

#include "ItemUsageView.h"

#include "../MuEditor/UI/Common/CodeReferenceList.h"
#include "Data/GameData/ItemData/ItemEnumNames.h"
#include "I18N/All.h"
#include "imgui.h"

#include <string>

namespace MuEditor::Items::UsageView
{
namespace
{
void RenderTags(const ItemUsage& usage)
{
    if (usage.tags.empty())
    {
        return;
    }
    std::string tags;
    for (const Data::Items::ItemTag tag : usage.tags)
    {
        const char* name = Data::Items::FindEnumName(tag);
        tags += (tags.empty() ? "" : ", ") + std::string(name != nullptr ? name : "?");
    }
    ImGui::BulletText("%s: %s", I18N::Editor::Tags, tags.c_str());
}

void RenderModel(const ItemUsage& usage)
{
    if (!usage.hasModel)
    {
        return;
    }
    if (usage.sharedModel.empty())
    {
        ImGui::BulletText("%s", I18N::Editor::ItsModelEntry);
        return;
    }
    ImGui::BulletText("%s", I18N::Format(I18N::Editor::SharedModel0AlsoUsedBy1Items,
                                         {usage.sharedModel, std::to_string(usage.sharedModelOtherUsers)})
                                .c_str());
}

void RenderGameData(const ItemUsage& usage)
{
    const std::pair<bool, const char*> uses[] = {{usage.inAncientSet, I18N::Editor::AnAncientSet},
                                                 {usage.hasAddOption, I18N::Editor::AnAddOption},
                                                 {usage.givesBuff, I18N::Editor::ABuff},
                                                 {usage.isPet, I18N::Editor::APet}};
    for (const auto& [used, label] : uses)
    {
        if (used)
        {
            ImGui::BulletText("%s", label);
        }
    }
}
} // namespace

void RenderGoingWithItem(const ItemUsage& usage)
{
    RenderTags(usage);
    RenderModel(usage);
}

void RenderUsesAtNumber(const ItemUsage& usage)
{
    // Without the sources, code may still name the item.
    if (!usage.IsUsedAtNumber() && usage.codeKnown)
    {
        ImGui::TextDisabled("%s", I18N::Editor::NothingUsesTheItem);
        return;
    }
    RenderGameData(usage);
    CodeReferenceList::Render(usage.codeKnown, usage.code,
                              I18N::Editor::TheSourcesWereNotFoundCodeThatNamesTheItemIsNotKnown);
}

void RenderNumberHolder(const ItemNumberHolder& holder, const ItemUsage& usage)
{
    if (!holder.item.empty())
    {
        ImGui::BulletText("%s", I18N::Format(I18N::Editor::TakenByTheItem0, {holder.item}).c_str());
        return;
    }
    if (holder.entryWithoutName)
    {
        ImGui::BulletText("%s", I18N::Editor::TakenByAnEntryWithoutAName);
        if (!holder.model.empty())
        {
            ImGui::BulletText("%s", I18N::Format(I18N::Editor::ItsModel0, {holder.model}).c_str());
        }
    }
    else if (holder.model.empty())
    {
        return;
    }
    else
    {
        ImGui::BulletText("%s", I18N::Format(I18N::Editor::TakenByAModelWithoutAnItem0, {holder.model}).c_str());
    }
    if (!holder.codeName.empty())
    {
        ImGui::BulletText("%s", I18N::Format(I18N::Editor::TheCodeCallsIt0, {std::string(holder.codeName)}).c_str());
    }
    ImGui::BulletText("%s", holder.entryWithoutName ? I18N::Editor::TheItemBrowserListsItAsEntryWithoutAName
                                                    : I18N::Editor::TheItemBrowserListsItAsNoItemAtThisNumber);
    RenderGameData(usage);
    // The move dialog lists the code of the moved item too.
    ImGui::PushID("NumberHolder");
    CodeReferenceList::Render(usage.codeKnown, usage.code,
                              I18N::Editor::TheSourcesWereNotFoundCodeThatNamesTheItemIsNotKnown);
    ImGui::PopID();
}
} // namespace MuEditor::Items::UsageView

#endif // _EDITOR
