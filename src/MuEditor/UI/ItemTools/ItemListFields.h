#pragma once

#ifdef _EDITOR

#include <string>
#include <vector>

// The model tool's field for a list of folder names, one per line.
namespace MuEditor::Items::ListFields
{
// The folders as text fields with a button that removes one and one that adds
// a field. True when a change is complete: a field left after a change, a
// folder added or removed.
bool EditFolders(const char* id, std::vector<std::string>& folders);
} // namespace MuEditor::Items::ListFields

#endif // _EDITOR
