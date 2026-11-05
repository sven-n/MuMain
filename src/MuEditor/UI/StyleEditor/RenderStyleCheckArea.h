#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/DataChecks/DataCheckArea.h"

#include <memory>

namespace MuEditor::Looks
{
// The render styles in Data Checks (design L23): Data/Effects/RenderStyles.json read as the game reads it, then checked
// against the item models as the tools have them now and against the texture files. A problem names its look, and the
// item when it is about one; a click on an item's problem selects the item and opens the Item Model window.
std::unique_ptr<DataChecks::DataCheckArea> CreateRenderStyleCheckArea();
} // namespace MuEditor::Looks

#endif // _EDITOR
