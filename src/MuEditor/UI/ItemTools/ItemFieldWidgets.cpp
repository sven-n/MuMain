#include "stdafx.h"

#ifdef _EDITOR

#include "ItemFieldWidgets.h"

#include "../MuEditor/Core/MuEditorCore.h"

namespace MuEditor::Items::Fields
{
float ScaledWidth(float width)
{
    return width * g_MuEditorCore.GetUIScale();
}
} // namespace MuEditor::Items::Fields

#endif // _EDITOR
