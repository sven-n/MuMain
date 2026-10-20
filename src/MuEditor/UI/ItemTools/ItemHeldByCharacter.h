#pragma once

#ifdef _EDITOR

namespace MuEditor::Items
{
// Whether the character wears the item or has it in the inventory. Its size
// and slot cannot change then: the inventory grid keeps the cells it took when
// it was put there, and would free the wrong ones.
bool IsHeldByCharacter(int itemType);
} // namespace MuEditor::Items

#endif // _EDITOR
