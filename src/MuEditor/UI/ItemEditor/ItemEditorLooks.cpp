#include "stdafx.h"

#ifdef _EDITOR

#include "ItemEditorLooks.h"

#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemModelGlowJson.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace
{
using Data::Items::ItemModelDefinition;
using LookName = std::string ItemModelDefinition::*;

// At most this many items are listed at once; the list scrolls.
constexpr int MaxListedRows = 8;

struct Look
{
    std::string name;
    // The items that use the look: item type and log name.
    std::vector<std::pair<int, std::string>> users;
};

// What the section shows for the selected item, built when the selection or
// the item model data changes (not every frame).
struct LooksCache
{
    int itemType = -1;
    int databaseVersion = -1;
    std::string glow;
    Look renderStyle;
    Look effect;
};

LooksCache g_cache;

// The glow values as the model file has them, or "-" without any.
std::string DescribeGlow(const ItemModelDefinition& model)
{
    Data::Items::Json::OrderedJson json = Data::Items::Json::OrderedJson::object();
    Data::Items::GlowJson::Write(model, json);
    const auto glow = json.find(Data::Items::GlowJson::GlowKey);
    return glow != json.end() ? glow->dump() : "-";
}

Look TakeLook(const ItemModelDefinition& model, LookName look)
{
    Look result{model.*look, {}};
    if (result.name.empty())
    {
        return result;
    }
    const std::span<const ItemModelDefinition> models = g_ItemModelDatabase.GetAllSlots();
    for (int itemType = 0; itemType < static_cast<int>(models.size()); ++itemType)
    {
        if (models[itemType].Exists() && models[itemType].*look == result.name)
        {
            result.users.emplace_back(itemType, g_ItemDatabase.GetLogName(itemType));
        }
    }
    return result;
}

void Refresh(int itemType, const ItemModelDefinition& model)
{
    const int databaseVersion = g_ItemModelDatabase.GetVersion();
    if (g_cache.itemType == itemType && g_cache.databaseVersion == databaseVersion)
    {
        return;
    }
    g_cache.itemType = itemType;
    g_cache.databaseVersion = databaseVersion;
    g_cache.glow = DescribeGlow(model);
    g_cache.renderStyle = TakeLook(model, &ItemModelDefinition::renderStyle);
    g_cache.effect = TakeLook(model, &ItemModelDefinition::effect);
}

// A look with the number of items that use it; opened, the list of those
// items in a box of fixed height. Returns the item clicked, -1 for none.
int RenderLook(const char* label, const Look& look)
{
    if (look.name.empty())
    {
        ImGui::Text("%s: -", label);
        return -1;
    }

    int clicked = -1;
    ImGui::PushID(label);
    if (ImGui::TreeNode("look", "%s: %s (%s %d)", label, look.name.c_str(), I18N::Editor::UsedBy,
                        static_cast<int>(look.users.size())))
    {
        const int rows = std::min(static_cast<int>(look.users.size()), MaxListedRows);
        const ImVec2 size(-FLT_MIN,
                          ImGui::GetTextLineHeightWithSpacing() * rows + ImGui::GetStyle().FramePadding.y * 2.0f);
        if (ImGui::BeginListBox("##users", size))
        {
            for (const auto& [itemType, name] : look.users)
            {
                if (ImGui::Selectable(name.c_str(), itemType == g_cache.itemType))
                {
                    clicked = itemType;
                }
            }
            ImGui::EndListBox();
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
    return clicked;
}
} // namespace

int CItemEditorLooks::Render(int itemType)
{
    if (!ImGui::CollapsingHeader(I18N::Editor::Looks))
    {
        return -1;
    }

    const ItemModelDefinition* model = g_ItemModelDatabase.Find(itemType);
    if (model == nullptr)
    {
        ImGui::TextDisabled("%s", itemType < 0 ? I18N::Editor::SelectAnItem : I18N::Editor::NoModel);
        return -1;
    }
    Refresh(itemType, *model);
    ImGui::Text("%s: %s", I18N::Editor::ModelFile, model->file.c_str());
    ImGui::Text("%s: %s", I18N::Editor::Glow, g_cache.glow.c_str());
    const int clickedStyleUser = RenderLook(I18N::Editor::RenderStyle, g_cache.renderStyle);
    const int clickedEffectUser = RenderLook(I18N::Editor::Effect, g_cache.effect);
    return clickedStyleUser >= 0 ? clickedStyleUser : clickedEffectUser;
}

#endif // _EDITOR
