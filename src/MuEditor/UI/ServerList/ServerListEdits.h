#pragma once

#ifdef _EDITOR

#include "Data/GameData/ServerListData/ServerListGroup.h"

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace MuEditor::ServerList
{
using Data::ServerList::ButtonColorRule;
using Data::ServerList::CustomColor;
using Data::ServerList::ServerGroup;
using Data::ServerList::ServerListData;
using Data::ServerList::VipType;

// The server list the Server List window changes (groups, VIP types and texts):
// what the editor shows, what the file has, and an undo list for this session.
class ServerListEdits
{
public:
    static constexpr size_t MaxUndoSteps = 256;

    // The list of the file, just read or saved: nothing to undo, nothing
    // changed. `complete` is false when the read left something out (errors).
    void Reset(ServerListData list, bool complete = true);

    const ServerListData& GetList() const
    {
        return m_list;
    }
    // For the window's fields, which change the list in place; Commit makes
    // what they changed one undo step.
    ServerListData& EditList()
    {
        return m_list;
    }
    const std::vector<ServerGroup>& GetGroups() const
    {
        return m_list.groups;
    }
    std::vector<ServerGroup>& EditGroups()
    {
        return m_list.groups;
    }
    // What changed since the last step becomes one undo step.
    void Commit();

    bool CanUndo() const
    {
        return !m_undo.empty();
    }
    bool CanRedo() const
    {
        return !m_redo.empty();
    }
    void Undo();
    void Redo();
    // What Undo or Redo would take back or do again ("Remove Helheim",
    // "Valhalla: channel 3"); empty when there is nothing.
    std::string GetUndoLabel() const;
    std::string GetRedoLabel() const;

    // Whether the list differs from the file.
    bool HasUnsavedChanges() const
    {
        return m_list != m_saved;
    }
    // The list is what the file has now.
    void MarkSaved()
    {
        m_saved = m_list;
    }
    // Whether the list is all the file has. After a read with fatal problems it
    // is not, and a save would remove what the read left out from the file.
    bool IsComplete() const
    {
        return m_complete;
    }

    // Adds a group named `name` with the smallest free ID, or `id` when given
    // (and free), at the end of the list. Returns its index; nothing when no
    // ID is free.
    std::optional<size_t> AddGroup(const std::string& name, std::optional<int> id = std::nullopt);
    void RemoveGroup(size_t index);
    // Moves the group one place up (-1) or down (+1); false at the end of the list.
    bool MoveGroup(size_t index, int direction);

    bool IsIdUsed(int id) const;
    std::optional<int> FindFreeId() const;

    // Adds a VIP type with the English name `name` at the end, its id made from
    // the name (one no other type has). Returns its index.
    size_t AddVipType(const std::string& name);
    // The channels with the type then have none; the button color rules for it go.
    void RemoveVipType(size_t index);
    // Moves the type one place up (-1) or down (+1); false at the end of the list.
    bool MoveVipType(size_t index, int direction);
    // Gives the type the id; the channels and rules with the old one get it too.
    // False (and nothing changed) when it is no VIP type id or another type has it.
    bool ChangeVipTypeId(size_t index, const std::string& id);
    bool IsVipTypeIdUsed(std::string_view id) const;
    // The channels with the type, as "Valhalla-1".
    std::vector<std::string> FindChannelsWithVipType(std::string_view id) const;

    // Adds a color named `name` at the end, its id made from the name (one no
    // other color or theme color has), with no shades. Returns its index.
    size_t AddColor(const std::string& name);
    // The rules with the color then have the default color.
    void RemoveColor(size_t index);
    bool MoveColor(size_t index, int direction);
    // Gives the color the id; the rules with the old one get it too. False (and
    // nothing changed) when it is no color id or another color has it.
    bool ChangeColorId(size_t index, const std::string& id);
    // Whether `id` would be a free id for a color: a color id no other color has.
    bool IsColorIdFree(std::string_view id) const;
    // How many rules have the color.
    size_t CountRulesWithColor(std::string_view id) const;

    // Adds a rule with the default color for the first VIP type and PvP setting
    // no rule has yet (any channel first). Nothing when every one has a rule.
    std::optional<size_t> AddButtonColorRule();
    void RemoveButtonColorRule(size_t index);
    // Whether another rule than the one at `index` fits the same channels as `rule`.
    bool HasRuleLike(const Data::ServerList::ButtonColorRule& rule, std::optional<size_t> index) const;

private:
    // The list on the other side of a step, and what the step did.
    struct Step
    {
        ServerListData list;
        std::string label;
    };

    // What the list changed since the last step would be called.
    std::string GetPendingLabel() const;

    ServerListData m_list;
    // The list as the last undo step left it.
    ServerListData m_committed;
    ServerListData m_saved;
    std::vector<Step> m_undo;
    std::vector<Step> m_redo;
    // Set by the Add, Remove and Move functions until the next Commit; a change
    // of fields is described by comparing the lists instead.
    std::string m_pendingLabel;
    bool m_complete = true;
};
} // namespace MuEditor::ServerList

#endif // _EDITOR
