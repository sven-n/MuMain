#pragma once

#ifdef _EDITOR

#include "Data/GameData/ServerListData/ServerListGroup.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace MuEditor::ServerList
{
using Data::ServerList::ServerGroup;

// The server groups the Server List window changes: what the editor shows,
// what the file has, and an undo list for this session.
class ServerListEdits
{
public:
    static constexpr size_t MaxUndoSteps = 256;

    // The groups of the file, just read or saved: nothing to undo, nothing
    // changed. `complete` is false when the read left groups out (errors).
    void Reset(std::vector<ServerGroup> groups, bool complete = true);

    const std::vector<ServerGroup>& GetGroups() const
    {
        return m_groups;
    }
    // For the window's fields, which change the groups in place; Commit makes
    // what they changed one undo step.
    std::vector<ServerGroup>& EditGroups()
    {
        return m_groups;
    }
    // The groups changed since the last step become one undo step.
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

    // Whether the groups differ from the file.
    bool HasUnsavedChanges() const
    {
        return m_groups != m_saved;
    }
    // The groups are what the file has now.
    void MarkSaved()
    {
        m_saved = m_groups;
    }
    // Whether the groups are all the file has. After a read with fatal problems they
    // are not, and a save would remove the groups left out from the file.
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

private:
    // The groups on the other side of a step, and what the step did.
    struct Step
    {
        std::vector<ServerGroup> groups;
        std::string label;
    };

    // What the groups changed since the last step would be called.
    std::string GetPendingLabel() const;

    std::vector<ServerGroup> m_groups;
    // The groups as the last undo step left them.
    std::vector<ServerGroup> m_committed;
    std::vector<ServerGroup> m_saved;
    std::vector<Step> m_undo;
    std::vector<Step> m_redo;
    // Set by AddGroup, RemoveGroup and MoveGroup until the next Commit; a
    // change of fields is described by comparing the groups instead.
    std::string m_pendingLabel;
    bool m_complete = true;
};
} // namespace MuEditor::ServerList

#endif // _EDITOR
