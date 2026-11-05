#include "stdafx.h"

#ifdef _EDITOR

#include "SavedDataFiles.h"

namespace MuEditor::SavedDataFiles
{
namespace
{
int g_saveCount = 0;
} // namespace

void NoteSaved()
{
    ++g_saveCount;
}

int GetSaveCount()
{
    return g_saveCount;
}
} // namespace MuEditor::SavedDataFiles

#endif // _EDITOR
