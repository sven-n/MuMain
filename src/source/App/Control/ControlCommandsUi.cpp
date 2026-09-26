#include "stdafx.h"
#include "App/Control/ControlCommands.h"

#include "Render/Textures/ZzzOpenglUtil.h"
#include "UI/NewUI/Dialogs/NewUIMessageBox.h"
#include "UI/NewUI/Inventory/NewUIInventoryCtrl.h"
#include "UI/NewUI/Inventory/NewUIInventoryExtension.h"
#include "UI/NewUI/NewUISystem.h"
#include "UI/Scaling/UITransform.h"

#include "json.hpp"

#include <optional>
#include <string>
#include <string_view>

// `ui` and `slot-pixel`: where the open item windows are, in the window pixels
// `click-ui` takes. Windows are laid out by the responsive layout, so a script
// asks instead of hard-coding pixels.
namespace
{
using App::Control::ErrorCode;
using App::Control::Request;
using nlohmann::json;
using SEASON3B::CNewUIInventoryCtrl;
using SEASON3B::CNewUIObj;

struct NamedWindow
{
    std::string_view name;
    DWORD key;
};

// The windows `ui` reports when they are open.
constexpr NamedWindow Windows[] = {
    {"inventory", SEASON3B::INTERFACE_INVENTORY},
    {"inventory_extension", SEASON3B::INTERFACE_INVENTORY_EXT},
    {"character", SEASON3B::INTERFACE_CHARACTER},
    {"trade", SEASON3B::INTERFACE_TRADE},
    {"storage", SEASON3B::INTERFACE_STORAGE},
    {"storage_extension", SEASON3B::INTERFACE_STORAGE_EXT},
    {"mix", SEASON3B::INTERFACE_MIXINVENTORY},
    {"npc_shop", SEASON3B::INTERFACE_NPCSHOP},
    {"lucky_item", SEASON3B::INTERFACE_LUCKYITEMWND},
};

// A point of a window, from its window-local coordinates to window pixels.
json WindowPixel(const CNewUIObj& window, float x, float y)
{
    const auto transform = UI::Scaling::TransformForLayout(window.GetLayoutMode(), static_cast<int>(WindowWidth),
                                                           static_cast<int>(WindowHeight));
    json pixel;
    pixel["x"] = UI::Scaling::PositionX(transform, x);
    pixel["y"] = UI::Scaling::PositionY(transform, y);
    return pixel;
}

json WindowRect(const CNewUIObj& window, const RECT& rect)
{
    const auto transform = UI::Scaling::TransformForLayout(window.GetLayoutMode(), static_cast<int>(WindowWidth),
                                                           static_cast<int>(WindowHeight));
    const float left = UI::Scaling::PositionX(transform, static_cast<float>(rect.left));
    const float top = UI::Scaling::PositionY(transform, static_cast<float>(rect.top));
    json pixels;
    pixels["x"] = left;
    pixels["y"] = top;
    pixels["width"] = UI::Scaling::PositionX(transform, static_cast<float>(rect.right)) - left;
    pixels["height"] = UI::Scaling::PositionY(transform, static_cast<float>(rect.bottom)) - top;
    return pixels;
}

// Buttons a scenario presses with `click-ui`, by name.
json Elements()
{
    json elements = json::object();
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_TRADE))
    {
        elements["trade.confirm"] = WindowRect(*g_pTrade, g_pTrade->GetMyConfirmRect());
    }
    return elements;
}

// The window that owns `grid` and the grid control for a slot number, or
// nothing when that window is closed.
std::optional<CNewUIInventoryCtrl*> OpenGrid(DWORD windowKey, CNewUIInventoryCtrl* grid)
{
    if (grid == nullptr || !g_pNewUISystem->IsVisible(windowKey))
    {
        return std::nullopt;
    }
    return grid;
}

// Inventory slot numbers are those `state` reports: the main grid starts after
// the equipment, the extension grids follow it.
std::optional<CNewUIInventoryCtrl*> InventoryGrid(int slot)
{
    CNewUIInventoryCtrl* main = g_pMyInventory->GetInventoryCtrl();
    if (main != nullptr && slot >= main->GetIndexOffset() &&
        slot < main->GetIndexOffset() + main->GetNumberOfColumn() * main->GetNumberOfRow())
    {
        return OpenGrid(SEASON3B::INTERFACE_INVENTORY, main);
    }
    return OpenGrid(SEASON3B::INTERFACE_INVENTORY_EXT, g_pMyInventoryExt->TryGetExtensionByInventoryIndex(slot));
}

std::optional<CNewUIInventoryCtrl*> NamedGrid(std::string_view name, int slot)
{
    if (name == "inventory")
        return InventoryGrid(slot);
    if (name == "trade")
        return OpenGrid(SEASON3B::INTERFACE_TRADE, g_pTrade->GetMyInvenCtrl());
    if (name == "trade_partner")
        return OpenGrid(SEASON3B::INTERFACE_TRADE, g_pTrade->GetYourInvenCtrl());
    if (name == "storage")
        return OpenGrid(SEASON3B::INTERFACE_STORAGE, g_pStorageInventory->GetInventoryCtrl());
    if (name == "mix")
        return OpenGrid(SEASON3B::INTERFACE_MIXINVENTORY, g_pMixInventory->GetInventoryCtrl());
    return std::nullopt;
}

[[nodiscard]] bool IsKnownGrid(std::string_view name)
{
    return name == "inventory" || name == "trade" || name == "trade_partner" || name == "storage" || name == "mix" ||
           name == "equipment";
}

std::string SquarePixel(const Request& request, CNewUIInventoryCtrl& grid, int slot)
{
    const int localIndex = slot - grid.GetIndexOffset();
    const int columns = grid.GetNumberOfColumn();
    if (localIndex < 0 || localIndex >= columns * grid.GetNumberOfRow())
    {
        return App::Control::EncodeError(request.EncodedId(), ErrorCode::BadRequest, "the grid has no such slot");
    }

    const POINT& position = grid.GetPos();
    const float x = static_cast<float>(position.x + (localIndex % columns) * SEASON3B::INVENTORY_SQUARE_WIDTH +
                                       SEASON3B::INVENTORY_SQUARE_WIDTH / 2);
    const float y = static_cast<float>(position.y + (localIndex / columns) * SEASON3B::INVENTORY_SQUARE_HEIGHT +
                                       SEASON3B::INVENTORY_SQUARE_HEIGHT / 2);
    return App::Control::EncodeResult(request.EncodedId(), WindowPixel(*grid.GetOwner(), x, y).dump());
}

std::string EquipmentPixel(const Request& request, int slot)
{
    if (!g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY))
    {
        return App::Control::EncodeError(request.EncodedId(), ErrorCode::NotOpen, "the inventory is not open");
    }

    POINT center{};
    if (!g_pMyInventory->GetEquipmentSlotCenter(slot, center))
    {
        return App::Control::EncodeError(request.EncodedId(), ErrorCode::BadRequest, "no such equipment slot");
    }
    return App::Control::EncodeResult(
        request.EncodedId(),
        WindowPixel(*g_pMyInventory, static_cast<float>(center.x), static_cast<float>(center.y)).dump());
}
} // namespace

namespace App::Control::Commands
{
std::string Ui(const Request& request, std::unique_ptr<Act>&)
{
    json windows = json::array();
    for (const NamedWindow& window : Windows)
    {
        if (g_pNewUISystem->IsVisible(window.key))
        {
            windows.push_back(window.name);
        }
    }
    // A dialog waiting for Enter or Esc, e.g. an incoming trade request.
    if (!g_MessageBox->IsEmpty())
    {
        windows.push_back("message_box");
    }

    json result;
    result["windows"] = windows;
    result["elements"] = Elements();
    return EncodeResult(request.EncodedId(), result.dump());
}

std::string SlotPixel(const Request& request, std::unique_ptr<Act>&)
{
    std::string grid;
    int slot = -1;
    if (!request.GetString("grid", grid) || !request.GetInt("slot", slot))
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest, "`slot-pixel` needs `grid` and `slot`");
    }
    if (!IsKnownGrid(grid))
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest,
                           "unknown `grid` `" + grid +
                               "`; known: inventory, equipment, trade, trade_partner, storage, mix");
    }
    if (grid == "equipment")
    {
        return EquipmentPixel(request, slot);
    }

    const std::optional<CNewUIInventoryCtrl*> square = NamedGrid(grid, slot);
    if (!square)
    {
        return EncodeError(request.EncodedId(), ErrorCode::NotOpen, "the window of that grid is not open");
    }
    return SquarePixel(request, **square, slot);
}
} // namespace App::Control::Commands
