#pragma once

#ifdef _EDITOR

// The data files the editor writes that Data Checks reads from disk, as the
// game reads them at start. Their windows count each save, so Data Checks can
// say that the data changed since its last check, as it does for the item and
// monster tools' data versions. The look files count their saves themselves
// (Render::Look::SaveCount), as `$look save` writes them too.
namespace MuEditor::SavedDataFiles
{
// The Server List wrote its file.
void NoteSaved();

// How many such saves there were in this session.
int GetSaveCount();
} // namespace MuEditor::SavedDataFiles

#endif // _EDITOR
