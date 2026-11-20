#include "stdafx.h"

#ifdef _EDITOR

#include "LookEditHistory.h"

#include <utility>

namespace MuEditor::GraphicsLook
{
namespace
{
const std::string NoLabel;
} // namespace

void LookEditHistory::Clear()
{
    m_undo.clear();
    m_redo.clear();
}

void LookEditHistory::Record(std::string label, Data::Maps::LookProfileData before, Data::Maps::LookProfileData after)
{
    m_undo.push_back({std::move(label), std::move(before), std::move(after)});
    if (m_undo.size() > MaxUndoSteps)
    {
        m_undo.pop_front();
    }
    m_redo.clear();
}

const std::string& LookEditHistory::GetUndoLabel() const
{
    return m_undo.empty() ? NoLabel : m_undo.back().label;
}

const std::string& LookEditHistory::GetRedoLabel() const
{
    return m_redo.empty() ? NoLabel : m_redo.back().label;
}

bool LookEditHistory::Undo(const Apply& apply)
{
    if (m_undo.empty() || !apply(m_undo.back().before))
    {
        return false;
    }
    m_redo.push_back(std::move(m_undo.back()));
    m_undo.pop_back();
    return true;
}

bool LookEditHistory::Redo(const Apply& apply)
{
    if (m_redo.empty() || !apply(m_redo.back().after))
    {
        return false;
    }
    m_undo.push_back(std::move(m_redo.back()));
    m_redo.pop_back();
    return true;
}
} // namespace MuEditor::GraphicsLook

#endif // _EDITOR
