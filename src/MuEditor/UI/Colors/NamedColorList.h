#pragma once

#ifdef _EDITOR

#include "NamedColorSource.h"

#include <string>
#include <vector>

namespace MuEditor::Colors
{
// The panel every tab of the Colors window draws its entries with (D69, with D26's rules): a new entry; the list with a
// swatch and how many items use each; the picked entry's value with Apply, Revert and Remove, and the items that use
// it; before a change reaches items, the list of them; and the entries no item uses, removed at once.
class NamedColorList
{
public:
    void Render(NamedColorSource& source);

private:
    void RenderList(NamedColorSource& source);
    void RenderPicked(NamedColorSource& source, const NamedColorTexts& texts);
    void RenderConfirm(NamedColorSource& source, const NamedColorTexts& texts);
    void RenderUnused(NamedColorSource& source, const NamedColorTexts& texts);

    // The entry picked in the list; "" before one is.
    std::string m_picked;
    // The items the change waiting for confirmation reaches.
    std::vector<int> m_confirmUsers;
};
} // namespace MuEditor::Colors

#endif // _EDITOR
