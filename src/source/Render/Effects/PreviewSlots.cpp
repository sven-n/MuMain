#include "stdafx.h"

#ifdef _EDITOR

#include "PreviewSlots.h"

#include "Audio/EditorSoundMute.h"

#include <array>
#include <cstddef>
#include <vector>

namespace Render::Effects::PreviewSlots
{
namespace
{
// Per list and slot, the view it belongs to; slots past the end are the game's.
std::array<std::vector<View>, ListCount> s_marks;
// Whether each view's time is stopped, by view.
std::array<bool, 256> s_stopped{};
View s_captured = 0;
bool s_anyMarked = false;

std::vector<View>& MarksOf(List list)
{
    return s_marks[static_cast<std::size_t>(list)];
}
} // namespace

void Clear()
{
    if (!s_anyMarked)
    {
        return;
    }
    for (std::vector<View>& marks : s_marks)
    {
        marks.assign(marks.size(), 0);
    }
    s_anyMarked = false;
}

void Mark(List list, int index, View view)
{
    if (index < 0)
    {
        return;
    }
    std::vector<View>& marks = MarksOf(list);
    if (static_cast<std::size_t>(index) >= marks.size())
    {
        marks.resize(static_cast<std::size_t>(index) + 1, 0);
    }
    marks[static_cast<std::size_t>(index)] = view;
    s_anyMarked = s_anyMarked || view != 0;
}

void Born(List list, int index)
{
    std::vector<View>& marks = MarksOf(list);
    if (index >= 0 && static_cast<std::size_t>(index) < marks.size())
    {
        marks[static_cast<std::size_t>(index)] = 0;
    }
}

View ViewOf(List list, int index)
{
    const std::vector<View>& marks = MarksOf(list);
    return index >= 0 && static_cast<std::size_t>(index) < marks.size() ? marks[static_cast<std::size_t>(index)] : 0;
}

bool Draws(List list, int index)
{
    return ViewOf(list, index) == s_captured;
}

bool Moves(List list, int index)
{
    const View view = ViewOf(list, index);
    return view == 0 || !s_stopped[view];
}

void SetStopped(View view, bool stopped)
{
    s_stopped[view] = stopped && view != 0;
}

bool IsCapturing()
{
    return s_captured != 0;
}

CaptureScope::CaptureScope(View view) : m_previous(s_captured)
{
    s_captured = view;
}

CaptureScope::~CaptureScope()
{
    s_captured = m_previous;
}

MoveSoundScope::MoveSoundScope(List list, int index)
{
    if (ViewOf(list, index) != 0)
    {
        m_mute.emplace();
    }
}

MoveSoundScope::~MoveSoundScope() = default;
} // namespace Render::Effects::PreviewSlots

#endif // _EDITOR
