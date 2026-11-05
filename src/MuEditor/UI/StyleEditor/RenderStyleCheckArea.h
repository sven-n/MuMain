#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/DataChecks/DataCheckArea.h"
#include "Data/GameData/LookData/LookChecks.h"

#include <memory>

namespace MuEditor::Looks
{
// What the render style checks compare the looks with in the editor: the item models as the tools have them now, their
// loaded models' meshes and the texture files in Data.
Data::Looks::RenderStyleCheckInput MakeEditorCheckInput();

// The render styles in Data Checks (design L23): Data/Effects/RenderStyles.json read as the game reads it, then checked
// against the item models as the tools have them now and against the texture files. A problem names its look, and the
// item when it is about one; a click on it selects the item and opens the Style Editor on the look (on the file for a
// problem of the whole file).
std::unique_ptr<DataChecks::DataCheckArea> CreateRenderStyleCheckArea();
} // namespace MuEditor::Looks

#endif // _EDITOR
