#pragma once

#ifdef _EDITOR

// A read-only view of the looks of the selected item (Data/Items/Models): its
// model file, glow, render style and effect, and the items that share a look.
class CItemEditorLooks
{
public:
    // Shown above the item table; `itemType` is the selected item, -1 for none.
    static void Render(int itemType);
};

#endif // _EDITOR
