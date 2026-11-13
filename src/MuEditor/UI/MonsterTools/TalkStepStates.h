#pragma once

#ifdef _EDITOR

#include "imgui.h"

#include <cstddef>
#include <functional>
#include <span>
#include <vector>

// The open steps of a talk step list in Monster Setup (MN5): each step's tree
// node is keyed by an id of its own, kept in the window's state by row, so it
// follows the step when steps move, go or come, and is found again by the
// step's signature after a change made elsewhere (undo, redo).
namespace MuEditor::Monsters::SetupLists
{
// A row of a talk step list as it was left: its step's signature and id.
struct StepRow
{
    unsigned int signature = 0;
    int id = 0;
};

// The id of each step now (by its signature) from the rows before: the same
// step in its row keeps it, a step that moved takes its id along, a step
// changed in its row keeps the row's, any other gets newId().
std::vector<int> MatchStepIds(std::span<const StepRow> before, std::span<const unsigned int> signatures,
                              const std::function<int()>& newId);

// The window's state of a step list: the steps' ids by row and the rows'
// signatures as the list left them, under the ID stack it is made in.
class TalkStepStates
{
public:
    explicit TalkStepStates(size_t rows);

    // The rows as the list left them on its last frame: signature and id.
    std::vector<StepRow> Before() const;
    int NewId();
    int Id(size_t row) const;
    void SetId(size_t row, int id);
    // The list as it is left this frame.
    void Leave(std::span<const unsigned int> signatures);

private:
    size_t KnownCount() const;

    ImGuiStorage* m_storage;
    ImGuiID m_countKey = 0;
    ImGuiID m_nextKey = 0;
    std::vector<ImGuiID> m_idKeys;
    std::vector<ImGuiID> m_signatureKeys;
};
} // namespace MuEditor::Monsters::SetupLists

#endif // _EDITOR
