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

// The render styles in Data Checks (design L23): the looks as the Style Editor has them, unsaved changes too, as Save
// Items would write Data/Effects/RenderStyles.json and the game would read it back, then checked against the item
// models as the tools have them now and against the texture files (after a read of the file with a fatal problem: that
// read's problems). A problem names its look, and the item when it is about one; a click on it selects the item and
// opens the Style Editor on the look (on the file for a problem of the whole file).
std::unique_ptr<DataChecks::DataCheckArea> CreateRenderStyleCheckArea();
// The item effects in Data Checks, alike (Data/Effects/ItemEffects.json; design L23, L44).
std::unique_ptr<DataChecks::DataCheckArea> CreateItemEffectCheckArea();
} // namespace MuEditor::Looks

#endif // _EDITOR
