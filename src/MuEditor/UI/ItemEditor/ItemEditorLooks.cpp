#include "stdafx.h"

#ifdef _EDITOR

#include "ItemEditorLooks.h"
#include "ItemEditorTable.h"

#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemModelGlowJson.h"
#include "I18N/All.h"
#include "imgui.h"

#include <span>
#include <string>

namespace
{
using Data::Items::ItemModelDefinition;
using LookName = std::string ItemModelDefinition::*;

// The glow values as the model file has them, or "-" without any.
std::string DescribeGlow(const ItemModelDefinition& model)
{
    Data::Items::Json::OrderedJson json = Data::Items::Json::OrderedJson::object();
    Data::Items::GlowJson::Write(model, json);
    const auto glow = json.find(Data::Items::GlowJson::GlowKey);
    return glow != json.end() ? glow->dump() : "-";
}

// A look of the item with the number of items that use it; opened, the list
// of those items. Clicking one selects it in the table.
void RenderLook(const char* label, const ItemModelDefinition& model, LookName look)
{
    const std::string& name = model.*look;
    if (name.empty())
    {
        ImGui::Text("%s: -", label);
        return;
    }

    const std::span<const ItemModelDefinition> models = g_ItemModelDatabase.GetAllSlots();
    const auto usesLook = [&](const ItemModelDefinition& other) { return other.Exists() && other.*look == name; };
    int users = 0;
    for (const ItemModelDefinition& other : models)
    {
        users += usesLook(other) ? 1 : 0;
    }

    ImGui::PushID(label);
    if (ImGui::TreeNode("look", "%s: %s (%s %d)", label, name.c_str(), I18N::Editor::UsedBy, users))
    {
        for (int itemType = 0; itemType < static_cast<int>(models.size()); ++itemType)
        {
            if (usesLook(models[itemType]) && ImGui::Selectable(g_ItemDatabase.GetLogName(itemType).c_str(), false))
            {
                CItemEditorTable::RequestScrollToIndex(itemType);
            }
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
}
} // namespace

void CItemEditorLooks::Render(int itemType)
{
    if (!ImGui::CollapsingHeader(I18N::Editor::Looks))
    {
        return;
    }

    const ItemModelDefinition* model = g_ItemModelDatabase.Find(itemType);
    if (model == nullptr)
    {
        ImGui::TextDisabled("%s", itemType < 0 ? I18N::Editor::SelectAnItem : I18N::Editor::NoModel);
        return;
    }
    ImGui::Text("%s: %s", I18N::Editor::ModelFile, model->file.c_str());
    ImGui::Text("%s: %s", I18N::Editor::Glow, DescribeGlow(*model).c_str());
    RenderLook(I18N::Editor::RenderStyle, *model, &ItemModelDefinition::renderStyle);
    RenderLook(I18N::Editor::Effect, *model, &ItemModelDefinition::effect);
}

#endif // _EDITOR
