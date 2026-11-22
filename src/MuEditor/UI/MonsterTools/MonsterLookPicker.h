#pragma once

#ifdef _EDITOR

#include "Data/GameData/LookData/LookDefinition.h"

#include <string>

// The pick of a look (MN8) in Monster Setup and Monster Model.
namespace MuEditor::Monsters::LookPicker
{
// A list of the looks of `kind` with `noLook` first (the value that names none, shown as `noLookLabel`), then Open in
// Style Editor and a warning for a name no look file has; `width` is the list's. True in the frame another one is
// picked.
bool Render(std::string& name, Data::Looks::LookKind kind, const std::string& noLook, const char* noLookLabel,
            float width);
} // namespace MuEditor::Monsters::LookPicker

#endif // _EDITOR
