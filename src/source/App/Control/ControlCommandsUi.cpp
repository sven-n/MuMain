#include "stdafx.h"
#include "App/Control/ControlCommands.h"

#include "UI/Core/WindowAccess.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Dialogs/GenericConfirmDialog.h"
#include "UI/Dialogs/MessageBox.h"
#include "UI/Inventory/InventoryCtrl.h"
#include "UI/Inventory/InventoryExtension.h"
#include "UI/RmlBridge/RmlElementBox.h"

#include <RmlUi/Core/Element.h>

#include "json.hpp"

#include <optional>
#include <string>
#include <string_view>

// `ui` and `slot-pixel`: where the open item windows are, in the window pixels
// `click-ui` takes. Windows are laid out by their themes, so a script asks
// instead of hard-coding pixels.
namespace
{
using App::Control::ErrorCode;
using App::Control::Request;
using mu::ui::window::CInventoryCtrl;
using nlohmann::json;

struct NamedWindow
{
    std::string_view name;
    DWORD key;
};

// The windows `ui` reports when they are open.
constexpr NamedWindow Windows[] = {
    {"inventory", mu::ui::window::INTERFACE_INVENTORY},
    {"inventory_extension", mu::ui::window::INTERFACE_INVENTORY_EXT},
    {"character", mu::ui::window::INTERFACE_CHARACTER},
    {"trade", mu::ui::window::INTERFACE_TRADE},
    {"storage", mu::ui::window::INTERFACE_STORAGE},
    {"storage_extension", mu::ui::window::INTERFACE_STORAGE_EXT},
    {"mix", mu::ui::window::INTERFACE_MIXINVENTORY},
    {"npc_shop", mu::ui::window::INTERFACE_NPCSHOP},
    {"lucky_item", mu::ui::window::INTERFACE_LUCKYITEMWND},
    {"chat_input", mu::ui::window::INTERFACE_CHATINPUTBOX},
    {"party", mu::ui::window::INTERFACE_PARTY},
    {"command", mu::ui::window::INTERFACE_COMMAND},
    {"my_shop", mu::ui::window::INTERFACE_MYSHOP_INVENTORY},
    {"purchase_shop", mu::ui::window::INTERFACE_PURCHASESHOP_INVENTORY},
    {"npc_quest", mu::ui::window::INTERFACE_NPCQUEST},
};

json Pixel(float x, float y)
{
    json pixel;
    pixel["x"] = x;
    pixel["y"] = y;
    return pixel;
}

// Adds the element `selector` finds in a window under `name`, with its drawn rectangle in window
// pixels, its transforms included, while it and its window are shown.
void AddElement(json& elements, const std::string& name, UI::Windows::WindowId window, const std::string& selector)
{
    Rml::Element* element = UI::Windows::FindElement(window, selector.c_str());
    Rml::Vector2f offset;
    Rml::Vector2f size;
    if (element == nullptr || !element->IsVisible(true) ||
        !UI::RmlBridge::DrawnBox(*element, Rml::BoxArea::Border, offset, size))
    {
        return;
    }
    json rect = Pixel(offset.x, offset.y);
    rect["width"] = size.x;
    rect["height"] = size.y;
    elements[name] = std::move(rect);
}

struct NamedElement
{
    std::string_view name;
    UI::Windows::WindowId window;
    std::string_view selector;
};

// Buttons a scenario presses with `click-ui`, by name: the ids both themes' documents share.
constexpr NamedElement Buttons[] = {
    {"trade.confirm", mu::ui::window::INTERFACE_TRADE, "#my_confirm"},
    {"trade.zen", mu::ui::window::INTERFACE_TRADE, "#btn_zen"},
    {"inventory.repair", mu::ui::window::INTERFACE_INVENTORY, "#btn_repair"},
    {"inventory.my_shop", mu::ui::window::INTERFACE_INVENTORY, "#btn_myshop"},
    {"my_shop.title", mu::ui::window::INTERFACE_MYSHOP_INVENTORY, "#shop_title"},
    {"my_shop.open", mu::ui::window::INTERFACE_MYSHOP_INVENTORY, "#btn_open"},
    {"my_shop.close", mu::ui::window::INTERFACE_MYSHOP_INVENTORY, "#btn_close"},
    {"npc_quest.complete", mu::ui::window::INTERFACE_NPCQUEST, "#btn_complete"},
    {"npc_quest.close", mu::ui::window::INTERFACE_NPCQUEST, "#btn_exit"},
    // The "+" buttons, shown while there are level-up points; command for a Dark Lord only.
    {"character.stat.strength", mu::ui::window::INTERFACE_CHARACTER, "#btn_stat_str"},
    {"character.stat.agility", mu::ui::window::INTERFACE_CHARACTER, "#btn_stat_agi"},
    {"character.stat.vitality", mu::ui::window::INTERFACE_CHARACTER, "#btn_stat_vit"},
    {"character.stat.energy", mu::ui::window::INTERFACE_CHARACTER, "#btn_stat_ene"},
    {"character.stat.command", mu::ui::window::INTERFACE_CHARACTER, "#btn_stat_cmd"},
    {"npc_shop.repair", mu::ui::window::INTERFACE_NPCSHOP, "#btn_repair"},
    {"npc_shop.repair_all", mu::ui::window::INTERFACE_NPCSHOP, "#btn_repair_all"},
};

struct NamedCommand
{
    std::string_view name;
    int command;
};

constexpr NamedCommand Commands[] = {
    {"command.trade", COMMAND_TRADE},
    {"command.purchase", COMMAND_PURCHASE},
    {"command.party", COMMAND_PARTY},
};

json Elements()
{
    json elements = json::object();
    for (const NamedElement& button : Buttons)
        AddElement(elements, std::string(button.name), button.window, std::string(button.selector));
    for (const NamedCommand& command : Commands)
    {
        AddElement(elements, std::string(command.name), mu::ui::window::INTERFACE_COMMAND,
                   ".cmd-btn[data-command=\"" + std::to_string(command.command) + "\"]");
    }
    // The quest dialog's answers by their number, which rows without text skip.
    for (int answer = 0; answer < MAX_ANSWER_FOR_DIALOG; ++answer)
    {
        AddElement(elements, "npc_quest.answer." + std::to_string(answer), mu::ui::window::INTERFACE_NPCQUEST,
                   ".nq-answer-row[data-answer=\"" + std::to_string(answer) + "\"]");
    }
    return elements;
}

// The grid control while the window that owns it is open, or nothing.
std::optional<CInventoryCtrl*> OpenGrid(DWORD windowKey, CInventoryCtrl* grid)
{
    if (grid == nullptr || !g_pNewUISystem->IsVisible(windowKey))
    {
        return std::nullopt;
    }
    return grid;
}

// Inventory slot numbers are those `state` reports: the main grid starts after
// the equipment, the extension grids follow it.
[[nodiscard]] bool IsInMainGrid(CInventoryCtrl* main, int slot)
{
    return main != nullptr && slot >= main->GetIndexOffset() &&
           slot < main->GetIndexOffset() + main->GetNumberOfColumn() * main->GetNumberOfRow();
}

[[nodiscard]] bool IsInventorySlot(int slot)
{
    const bool inExtension = slot >= MAX_MY_INVENTORY_INDEX &&
                             slot < MAX_MY_INVENTORY_INDEX + MAX_INVENTORY_EXT_COUNT * MAX_INVENTORY_EXT_ONE;
    return IsInMainGrid(g_pMyInventory->GetInventoryCtrl(), slot) || inExtension;
}

std::optional<CInventoryCtrl*> InventoryGrid(int slot)
{
    CInventoryCtrl* main = g_pMyInventory->GetInventoryCtrl();
    if (IsInMainGrid(main, slot))
    {
        return OpenGrid(mu::ui::window::INTERFACE_INVENTORY, main);
    }
    return OpenGrid(mu::ui::window::INTERFACE_INVENTORY_EXT, g_pMyInventoryExt->TryGetExtensionByInventoryIndex(slot));
}

std::optional<CInventoryCtrl*> NamedGrid(std::string_view name, int slot)
{
    using namespace mu::ui::window;

    if (name == "inventory")
        return InventoryGrid(slot);
    if (name == "trade")
        return OpenGrid(INTERFACE_TRADE, g_pTrade->GetMyInvenCtrl());
    if (name == "trade_partner")
        return OpenGrid(INTERFACE_TRADE, g_pTrade->GetYourInvenCtrl());
    if (name == "storage")
        return OpenGrid(INTERFACE_STORAGE, g_pStorageInventory->GetInventoryCtrl());
    if (name == "mix")
        return OpenGrid(INTERFACE_MIXINVENTORY, g_pMixInventory->GetInventoryCtrl());
    if (name == "npc_shop")
        return OpenGrid(INTERFACE_NPCSHOP, g_pNPCShop->GetInventoryCtrl());
    if (name == "my_shop")
        return OpenGrid(INTERFACE_MYSHOP_INVENTORY, g_pMyShopInventory->GetInventoryCtrl());
    if (name == "purchase_shop")
        return OpenGrid(INTERFACE_PURCHASESHOP_INVENTORY, g_pPurchaseShopInventory->GetInventoryCtrl());
    return std::nullopt;
}

[[nodiscard]] bool IsKnownGrid(std::string_view name)
{
    return name == "inventory" || name == "trade" || name == "trade_partner" || name == "storage" || name == "mix" ||
           name == "npc_shop" || name == "my_shop" || name == "purchase_shop" || name == "equipment";
}

// The middle of a square, from the grid's geometry, which is in window pixels.
std::string SquarePixel(const Request& request, CInventoryCtrl& grid, int slot)
{
    const int localIndex = slot - grid.GetIndexOffset();
    const int columns = grid.GetNumberOfColumn();
    if (localIndex < 0 || localIndex >= columns * grid.GetNumberOfRow())
    {
        return App::Control::EncodeError(request.EncodedId(), ErrorCode::BadRequest, "the grid has no such slot");
    }

    const UI::Items::GridRect square = grid.Geometry().CellsRect(localIndex % columns, localIndex / columns, 1, 1);
    return App::Control::EncodeResult(
        request.EncodedId(), Pixel(square.x + square.width / 2.f, square.y + square.height / 2.f).dump());
}

std::string EquipmentPixel(const Request& request, int slot)
{
    if (!g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_INVENTORY))
    {
        return App::Control::EncodeError(request.EncodedId(), ErrorCode::NotOpen, "the inventory is not open");
    }

    POINT center{};
    if (!g_pMyInventory->GetEquipmentSlotCenter(slot, center))
    {
        return App::Control::EncodeError(request.EncodedId(), ErrorCode::BadRequest, "no such equipment slot");
    }
    return App::Control::EncodeResult(request.EncodedId(),
                                      Pixel(static_cast<float>(center.x), static_cast<float>(center.y)).dump());
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
    if (!g_MessageBox->IsEmpty() || mu::ui::window::g_pGenericConfirmDialog->IsVisible())
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
                               "`; known: inventory, equipment, trade, trade_partner, storage, mix, npc_shop, my_shop, "
                               "purchase_shop");
    }
    if (grid == "equipment")
    {
        return EquipmentPixel(request, slot);
    }

    if (grid == "inventory" && !IsInventorySlot(slot))
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest,
                           "the inventory has no slot " + std::to_string(slot) + "; equipment is the `equipment` grid");
    }

    const std::optional<CInventoryCtrl*> square = NamedGrid(grid, slot);
    if (!square)
    {
        return EncodeError(request.EncodedId(), ErrorCode::NotOpen, "the window of that grid is not open");
    }
    return SquarePixel(request, **square, slot);
}
} // namespace App::Control::Commands
