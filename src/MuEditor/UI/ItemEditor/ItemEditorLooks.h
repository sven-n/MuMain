#pragma once

#ifdef _EDITOR

// A read-only view of the looks of the selected item (Data/Items/Models): its
// model file and shared model, glow, render style and item effect, and the
// items that share a model or a look.
class CItemEditorLooks
{
public:
    // Shown above the item table; `itemType` is the selected item, -1 for none.
    // Returns the item clicked in a list of the items that share a model or a
    // look, -1 for none.
    static int Render(int itemType);
};

#endif // _EDITOR
