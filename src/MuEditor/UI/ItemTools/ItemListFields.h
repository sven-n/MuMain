#pragma once

#ifdef _EDITOR

#include <functional>
#include <string>
#include <vector>

// The model tool's field for a list of folder names, one per line.
namespace MuEditor::Items::ListFields
{
// The folders as text fields with a button that removes one and one that adds
// a field. True when a change is complete: a folder added or removed, and a
// field left after a change. With `afterField`, which is called after each
// field (ItemEditDraft::AfterField watches it), the fields are the caller's
// to store and only the buttons count.
bool EditFolders(const char* id, std::vector<std::string>& folders, const std::function<void()>& afterField = {});
} // namespace MuEditor::Items::ListFields

#endif // _EDITOR
