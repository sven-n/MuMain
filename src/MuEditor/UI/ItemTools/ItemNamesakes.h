#pragma once

#ifdef _EDITOR

#include <string>
#include <vector>

namespace MuEditor::Items
{
// Items whose name (as the game shows it) another item has too, e.g. the lucky
// sets and the tiers of the seed spheres: a short text that tells them apart
// in the item lists. It names the tags an item has that not all its
// namesakes have ("luckyItem"); namesakes that no tag tells apart count up
// by number ("#1", "#2").
class ItemNamesakes
{
public:
    static ItemNamesakes& GetInstance();

    // "" when no other item has the item's name, and for the one namesake
    // whose tags all its namesakes have (the plain item of a lucky set).
    const std::string& GetQualifier(int itemType);

private:
    ItemNamesakes() = default;

    // Again when the items or their names changed.
    void Update();

    std::vector<std::string> m_qualifiers;
    int m_version = -1;
    int m_nameVersion = -1;
};

// Draws the qualifier after the item's name on the same line, greyed out;
// nothing when the item has no namesake.
void RenderNamesakeQualifier(int itemType);
} // namespace MuEditor::Items

#define g_ItemNamesakes MuEditor::Items::ItemNamesakes::GetInstance()

#endif // _EDITOR
