#pragma once

#ifdef _EDITOR

#include <optional>

namespace MuEditor::Items
{
// Which mesh of the Item Model window's item the preview shows on its own, the others see-through: the one the mouse
// points at (a row of the mesh list, a number in a mesh field), else the one a click keeps (a row, a part of the
// preview). The preview is drawn before the list and the fields, so a mesh pointed at shows from the next frame.
class CItemMeshFocus
{
public:
    static CItemMeshFocus& GetInstance();

    // At the start of the window's frame, for the item it shows; another item forgets the meshes.
    void BeginFrame(int itemType);
    void Point(int mesh);
    // A click on a row: keeps its mesh, or lets it go when it was kept.
    void ToggleKept(int mesh);
    // A click on the preview: keeps the mesh it hit; a click that hit nothing lets the kept one go.
    void Pick(std::optional<int> mesh);
    // A double click puts the view back: its first click keeps the mesh that was kept before it.
    void TakeBackPick();
    // A model opened again can have fewer meshes.
    void ForgetMeshesFrom(int meshCount);

    // The mesh the preview shows on its own, if any.
    [[nodiscard]] std::optional<int> Shown() const;
    [[nodiscard]] std::optional<int> Kept() const
    {
        return m_kept;
    }
    // Whether the last click on the preview picked the kept mesh, so its row can be scrolled to once.
    [[nodiscard]] bool TakePicked();

private:
    int m_itemType = -1;
    std::optional<int> m_pointedNow;
    std::optional<int> m_pointedBefore;
    std::optional<int> m_kept;
    bool m_picked = false;
    // The kept mesh before the last pick, while nothing else changed it.
    std::optional<std::optional<int>> m_keptBeforePick;
};
} // namespace MuEditor::Items

#define g_ItemMeshFocus MuEditor::Items::CItemMeshFocus::GetInstance()

#endif // _EDITOR
