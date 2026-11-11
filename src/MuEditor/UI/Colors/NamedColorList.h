#pragma once

#ifdef _EDITOR

#include "NamedColorSource.h"

#include <span>
#include <string>
#include <vector>

namespace MuEditor::Colors
{
// The panel the Colors window draws the picked entry of any root with (D69, with D26's rules): its value with Apply,
// Revert and Remove, and who uses it; before a change reaches users, the list of them. For a picked root, the entries
// nothing uses, removed at once.
class NamedColorList
{
public:
    // Returns the entry's name after this frame: another one when Apply renamed it, "" when Remove took it.
    std::string RenderEntry(NamedColorSource& source, const std::string& name);
    void RenderUnused(NamedColorSource& source);

private:
    // Returns the entry's name after Apply, or "" while nothing was applied.
    std::string RenderConfirm(NamedColorSource& source, const NamedColorTexts& texts);

    // The entry and the users the change waiting for confirmation reaches.
    std::string m_confirmOf;
    std::vector<NamedColorUser> m_confirmUsers;
};

// Users that are items: each listed by its name, and a click selects it.
std::vector<NamedColorUser> MakeItemUsers(std::span<const int> itemTypes);

// A grey line that wraps at the window's edge.
void RenderColorNote(const char* text);
} // namespace MuEditor::Colors

#endif // _EDITOR
