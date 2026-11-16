#include "stdafx.h"

#ifdef _EDITOR

#include "LookEdits.h"

#include "../MuEditor/UI/ItemTools/ItemAddDialog.h"
#include "../MuEditor/UI/ItemTools/ItemEdits.h"
#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ColorFolders/ColorFolders.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemTypeUse.h"
#include "Render/Items/LookDrawing.h"

#include <algorithm>
#include <utility>

namespace MuEditor::Looks
{
namespace
{
using Data::Looks::LookDefinition;

void SortByName(std::vector<LookDefinition>& looks)
{
    std::ranges::sort(looks, {}, &LookDefinition::name);
}

// The model entry's field that names a look of the kind.
std::string& LookFieldOf(Data::Items::ItemModelDefinition& model, Data::Looks::LookKind kind)
{
    return kind == Data::Looks::LookKind::ItemEffect ? model.itemEffect : model.renderStyle;
}
} // namespace

LookEdits& LookEdits::GetInstance()
{
    return Of(Data::Looks::LookKind::RenderStyle);
}

LookEdits& LookEdits::Of(Data::Looks::LookKind kind)
{
    static LookEdits renderStyles(Data::Looks::LookKind::RenderStyle);
    static LookEdits itemEffects(Data::Looks::LookKind::ItemEffect);
    return kind == Data::Looks::LookKind::ItemEffect ? itemEffects : renderStyles;
}

const LookEdits::State& LookEdits::GetState() const
{
    const State& state = g_ItemDataHandler.GetLookFile(m_kind);
    if (m_texturesVersion != g_ItemDataHandler.GetLooksVersion(m_kind))
    {
        m_texturesVersion = g_ItemDataHandler.GetLooksVersion(m_kind);
        for (const LookDefinition& look : state.looks)
        {
            Render::Items::Looks::PrepareTextures(look);
        }
    }
    return state;
}

const LookDefinition* LookEdits::Find(std::string_view name) const
{
    const std::vector<LookDefinition>& looks = GetState().looks;
    const auto found = std::ranges::lower_bound(looks, name, {}, &LookDefinition::name);
    return found != looks.end() && found->name == name ? &*found : nullptr;
}

int LookEdits::GetVersion() const
{
    // The read counts too: the first one makes the state the file's.
    static_cast<void>(GetState());
    return g_ItemDataHandler.GetLooksVersion(m_kind);
}

const std::vector<Data::Items::ItemDataIssue>& LookEdits::GetIssues() const
{
    return g_ItemDataHandler.GetLookReadIssues(m_kind);
}

bool LookEdits::IsComplete() const
{
    return g_ItemDataHandler.CanSaveLooks(m_kind);
}

void LookEdits::SetLook(const char* step, const std::string& name, LookDefinition look)
{
    State next = GetState();
    const auto found = std::ranges::find(next.looks, name, &LookDefinition::name);
    if (found == next.looks.end())
    {
        return;
    }
    if (look.name != name)
    {
        Data::Colors::RenameColorInFolders(next.folders, name, look.name);
    }
    *found = std::move(look);
    SortByName(next.looks);
    Change(step, std::move(next));
}

void LookEdits::AddLook(const char* step, LookDefinition look, const Data::Colors::ColorFolderPath& folder)
{
    if (Find(look.name) != nullptr)
    {
        return;
    }
    State next = GetState();
    const std::string name = look.name;
    next.looks.push_back(std::move(look));
    SortByName(next.looks);
    if (!folder.empty())
    {
        Data::Colors::MoveColorInto(next.folders, name, folder);
    }
    Change(step, std::move(next));
}

void LookEdits::RemoveLook(const char* step, const std::string& name)
{
    State next = GetState();
    if (std::erase_if(next.looks, [&name](const LookDefinition& look) { return look.name == name; }) == 0)
    {
        return;
    }
    Data::Colors::RemoveColorFromFolders(next.folders, name);
    Change(step, std::move(next));
}

void LookEdits::SetFolders(const char* step, std::vector<Data::Colors::ColorFolder> folders)
{
    State next = GetState();
    next.folders = std::move(folders);
    Change(step, std::move(next));
}

void LookEdits::ApplyToItemOnly(const char* step, const std::string& original, LookDefinition copy, int itemType)
{
    const Data::Items::ItemModelDefinition* entry = g_ItemModelDatabase.Find(itemType);
    if (entry == nullptr || Find(copy.name) != nullptr)
    {
        return;
    }
    Data::Items::ItemModelDefinition model = *entry;
    LookFieldOf(model, m_kind) = copy.name;
    g_ItemEdits.BeginGroup(step);
    AddLook(step, std::move(copy), FolderOf(original));
    g_ItemEdits.SetItemModel(step, model);
    g_ItemEdits.EndGroup();
}

void LookEdits::ApplyToNewItem(const char* step, const std::string& original, LookDefinition copy, int itemType,
                               int newItemType, const std::string& itemName)
{
    if (g_ItemModelDatabase.Find(itemType) == nullptr || Find(copy.name) != nullptr ||
        !Data::Items::IsFreeItemType(newItemType))
    {
        return;
    }
    const std::string name = copy.name;
    g_ItemEdits.BeginGroup(step);
    AddLook(step, std::move(copy), FolderOf(original));
    Items::DuplicateItem(step, itemType, newItemType, itemName, true,
                         [&name, kind = m_kind](Data::Items::ItemModelDefinition& model)
                         { LookFieldOf(model, kind) = name; });
    g_ItemEdits.EndGroup();
}

Data::Colors::ColorFolderPath LookEdits::FolderOf(const std::string& name) const
{
    return Data::Colors::FindFolderOfColor(GetState().folders, name).value_or(Data::Colors::ColorFolderPath{});
}

void LookEdits::ResetForTest(State state)
{
    g_ItemDataHandler.ResetLooksForTest(m_kind, std::move(state));
    // The textures its looks name.
    static_cast<void>(GetState());
}

void LookEdits::Change(const char* step, State next)
{
    g_ItemEdits.SetLookFile(step, m_kind, std::move(next));
}
} // namespace MuEditor::Looks

#endif // _EDITOR
