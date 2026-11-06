#include "stdafx.h"

#ifdef _EDITOR

#include "SoundSelection.h"

namespace MuEditor::SoundSelection
{
namespace
{
int g_selected = -1;
int g_version = 0;
} // namespace

int Get()
{
    return g_selected;
}

void Set(int number)
{
    if (number != g_selected)
    {
        g_selected = number;
        ++g_version;
    }
}

int GetVersion()
{
    return g_version;
}
} // namespace MuEditor::SoundSelection

#endif // _EDITOR
