#pragma once

#ifdef _EDITOR

#include "CodeReference.h"

#include <span>

namespace MuEditor::CodeReferenceList
{
// The source lines that name an item or a monster, as the remove dialogs show
// them: the count and the first lines in a list. `notFoundText` when the
// sources were not found (`known` false); nothing when no line names it.
void Render(bool known, std::span<const CodeReference> references, const char* notFoundText);
} // namespace MuEditor::CodeReferenceList

#endif // _EDITOR
