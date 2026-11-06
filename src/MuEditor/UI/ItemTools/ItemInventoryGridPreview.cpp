#include "stdafx.h"

#ifdef _EDITOR

#include "ItemInventoryGridPreview.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/EffectBrowser/EffectPoolGuard.h"
#include "../MuEditor/UI/EffectBrowser/SavedGameRenderState.h"
#include "Core/Globals/_enum.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Engine/Object/ZzzInventory.h"
#include "Render/Items/ItemDisplay.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "UI/Common/ScopedOffscreenCapture.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Inventory/ItemCamera.h"
#include "UI/Scaling/UITransform.h"
#include "imgui.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>

namespace MuEditor::Items
{
namespace
{
// An inventory cell in the original's 640 x 480 units, as the inventory
// lays its items out (INVENTORY_SQUARE_WIDTH).
constexpr float CellUnits = 20.0f;
// The cells around the item's, for a model that reaches past its slot.
constexpr int MarginCells = 1;
// A cell in the tool, before the editor UI scale.
constexpr float DisplayCellPixels = 40.0f;
// The original's item camera: a 1-degree field of view over the window.
constexpr float ItemFieldOfView = 1.0f;
// The highest item size the grid shows (the inventory has 8 columns).
constexpr int MaxCells = 8;
constexpr int ExcellentFlags = 1;
constexpr int AncientDiscriminator = 1;
constexpr unsigned int BackdropColor = 0xFF16161Au;
constexpr unsigned int CellColor = 0xFF2A2A30u;
// The item's cells, tinted as the legacy inventory tints a used cell.
constexpr unsigned int ItemCellColor = 0xFF2E4A4Au;
constexpr float CellGapUnits = 1.0f;

// Whether the inventory draws the item at this level and can now: the model
// it draws (also a level variant's, FindInventoryModel) has meshes and what
// it is drawn on has actions to place the bones.
bool CanDrawInInventory(int itemType, int level)
{
    const std::optional<Render::Items::Display::InventoryModel> drawn =
        Render::Items::Display::FindInventoryModel(itemType, level);
    if (!drawn)
    {
        return false;
    }
    const int model = Render::Items::Display::GetDrawnModel(drawn->modelType, drawn->drawLevel);
    const int skeleton = Render::Items::Display::IsDrawnOnCharacterSkeleton(model) ? MODEL_PLAYER : model;
    if (Models[model].NumMeshs <= 0 || Models[skeleton].NumActions <= 0)
    {
        return false;
    }
    // The Lucky Coin registration window turns that item; it is not there
    // outside the game.
    return itemType != ITEM_POTION + 100 || g_pLuckyCoinRegistration != nullptr;
}

// In cell units from the box's top left corner.
void DrawCells(int columns, int rows)
{
    const int across = columns + 2 * MarginCells;
    const int down = rows + 2 * MarginCells;
    RenderColorQuadARGB(0.0f, 0.0f, across * CellUnits, down * CellUnits, BackdropColor);
    for (int row = 0; row < down; ++row)
    {
        for (int column = 0; column < across; ++column)
        {
            const bool itemCell = column >= MarginCells && column < MarginCells + columns && row >= MarginCells &&
                                  row < MarginCells + rows;
            RenderColorQuadARGB(column * CellUnits + CellGapUnits, row * CellUnits + CellGapUnits,
                                CellUnits - CellGapUnits, CellUnits - CellGapUnits,
                                itemCell ? ItemCellColor : CellColor);
        }
    }
}

// What the grid shows: the item, its cells and how it is drawn.
struct GridItem
{
    int itemType;
    int columns;
    int rows;
    int level;
    bool excellent;
    bool ancient;
    bool hovered;
};

// The cells, then the item in them, in cell units from the box's top left
// corner: the box is `cellPixels` window pixels a cell from (left, top).
void DrawCellsAndItem(const GridItem& item, float cellPixels, float left, float top)
{
    // Cell units, so the item lands where the original's inventory puts it
    // (RenderItem3D rounds to them).
    const float scale = cellPixels / CellUnits;
    const UI::Scaling::ScopedActiveTransform units({scale, scale, left, top, 1.0f});
    DisableAlphaBlend();
    DisableDepthTest();
    DrawCells(item.columns, item.rows);
    EnableDepthTest();
    EnableDepthMask();
    if (CanDrawInInventory(item.itemType, item.level))
    {
        RenderItem3DWithHover(MarginCells * CellUnits, MarginCells * CellUnits, item.columns * CellUnits,
                              item.rows * CellUnits, item.itemType, item.level, item.excellent ? ExcellentFlags : 0,
                              item.ancient ? AncientDiscriminator : 0, item.hovered);
    }
}

// Into the open capture: the box the cells take at the window's height,
// centred in the window, under the item camera.
void DrawGrid(const GridItem& item)
{
    const MuEditor::Effects::SavedGameRenderState savedState;
    const MuEditor::Effects::EffectPoolGuard removeItemEffects;
    // The cell the item camera was made for: the inventory's 20 units at the
    // window's height.
    const float cellPixels = CellUnits * static_cast<float>(WindowHeight) / REFERENCE_HEIGHT;
    const float width = (item.columns + 2 * MarginCells) * cellPixels;
    const float height = (item.rows + 2 * MarginCells) * cellPixels;
    const float left = (static_cast<float>(WindowWidth) - width) * 0.5f;
    const float top = (static_cast<float>(WindowHeight) - height) * 0.5f;
    UI::Items::DrawUnderItemCamera(left, top, width, height, ItemFieldOfView,
                                   [&] { DrawCellsAndItem(item, cellPixels, left, top); });
}
} // namespace

void CItemInventoryGridPreview::Render(int itemType, int level, bool excellent, bool ancient)
{
    const Data::Items::ItemDefinition* item = g_ItemDatabase.Find(itemType);
    if (item == nullptr)
    {
        return;
    }
    const int columns = std::clamp(static_cast<int>(item->width), 1, MaxCells);
    const int rows = std::clamp(static_cast<int>(item->height), 1, MaxCells);
    const float cell = DisplayCellPixels * g_MuEditorCore.GetUIScale();
    const ImVec2 size((columns + 2 * MarginCells) * cell, (rows + 2 * MarginCells) * cell);
    ImGui::InvisibleButton("##InventoryGrid", size);
    const bool hovered = ImGui::IsItemHovered();
    const ImVec2 min = ImGui::GetItemRectMin();
    const ImVec2 max = ImGui::GetItemRectMax();
    if (!ImGui::IsItemVisible())
    {
        return;
    }
    std::uint32_t texture = 0;
    if (mu::GetRenderer().IsFrameActive())
    {
        // The density of the OS window the grid is in; ImGui leaves it 0
        // while the editor's windows stay in the game window.
        const float viewportScale = ImGui::GetWindowViewport()->FramebufferScale.x;
        const float framebufferScale = viewportScale > 0.0f ? viewportScale : ImGui::GetIO().DisplayFramebufferScale.x;
        const int width = std::max(1, static_cast<int>(std::lround(size.x * framebufferScale)));
        const int height = std::max(1, static_cast<int>(std::lround(size.y * framebufferScale)));
        texture = m_target.Begin(width, height, !ImGui::IsMouseDown(ImGuiMouseButton_Left));
        if (texture == 0)
        {
            return;
        }
        const MuEditor::ScopedOffscreenCapture endCapture;
        DrawGrid({itemType, columns, rows, level, excellent, ancient, hovered});
    }
    else
    {
        // The game skipped this frame; the editor's windows may still be drawn.
        texture = m_target.KeepLastPicture();
    }
    if (void* pointer = texture != 0 ? mu::GetRenderer().GetTexturePointer(texture) : nullptr)
    {
        ImGui::GetWindowDrawList()->AddImage((ImTextureID)(intptr_t)pointer, min, max);
    }
}
} // namespace MuEditor::Items

#endif // _EDITOR
