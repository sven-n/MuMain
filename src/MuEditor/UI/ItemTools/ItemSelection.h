#pragma once

#ifdef _EDITOR

namespace MuEditor::Items
{
// The item the item tools show: one selection for all of them (D26), so
// selecting an item in one tool opens it in the others. Any item type can be
// selected, also a number with a model but no item data.
class ItemSelection
{
public:
    static constexpr int NoItem = -1;

    static ItemSelection& GetInstance();

    int GetItemType() const
    {
        return m_itemType;
    }

    // Selects the item; selecting it again counts too, so the tools show it
    // again (scroll to it, clear a filter that hides it).
    void Select(int itemType);

    // Counts the Select calls. A tool remembers the serial it last showed and
    // shows the selected item once when it changes.
    int GetSerial() const
    {
        return m_serial;
    }

private:
    int m_itemType = NoItem;
    int m_serial = 0;
};
} // namespace MuEditor::Items

#define g_ItemSelection MuEditor::Items::ItemSelection::GetInstance()

#endif // _EDITOR
