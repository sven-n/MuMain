#pragma once

#ifdef _EDITOR

#include "Data/GameData/MapData/LookProfile.h"

#include <cstddef>
#include <deque>
#include <functional>
#include <string>
#include <vector>

namespace MuEditor::GraphicsLook
{
// The Graphics Look window's undo list for this session. A step is the whole
// profile before and after one change: undoing makes the profile before live
// again.
class LookEditHistory
{
public:
    static constexpr std::size_t MaxUndoSteps = 256;

    // Makes a profile live; false when it was refused (the step then stays).
    using Apply = std::function<bool(const Data::Maps::LookProfileData&)>;

    // Other files are live (a candidate, Reload): nothing to undo or redo.
    void Clear();
    // A change made: `before` became `after`. What could be redone goes.
    void Record(std::string label, Data::Maps::LookProfileData before, Data::Maps::LookProfileData after);

    [[nodiscard]] bool CanUndo() const
    {
        return !m_undo.empty();
    }
    [[nodiscard]] bool CanRedo() const
    {
        return !m_redo.empty();
    }
    // What Undo or Redo would take back or do again; empty when there is nothing.
    [[nodiscard]] const std::string& GetUndoLabel() const;
    [[nodiscard]] const std::string& GetRedoLabel() const;

    bool Undo(const Apply& apply);
    bool Redo(const Apply& apply);

private:
    struct Step
    {
        std::string label;
        Data::Maps::LookProfileData before;
        Data::Maps::LookProfileData after;
    };

    std::deque<Step> m_undo;
    std::vector<Step> m_redo;
};
} // namespace MuEditor::GraphicsLook

#endif // _EDITOR
