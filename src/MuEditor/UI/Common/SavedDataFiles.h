#pragma once

#ifdef _EDITOR

// The data files the editor writes that Data Checks reads from disk, as the
// game reads them at start: the server list and the look files. Their windows
// count each save, so Data Checks can say that the data changed since its
// last check, as it does for the item and monster tools' data versions.
namespace MuEditor::SavedDataFiles
{
// The Server List or the Dev Editor wrote its file.
void NoteSaved();

// How many such saves there were in this session.
int GetSaveCount();
} // namespace MuEditor::SavedDataFiles

#endif // _EDITOR
