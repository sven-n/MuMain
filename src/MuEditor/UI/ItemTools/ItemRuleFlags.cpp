#include "stdafx.h"

#ifdef _EDITOR

#include "ItemRuleFlags.h"

#include "I18N/All.h"

#include <array>

namespace MuEditor::Items
{
std::span<const ItemRuleFlag> GetItemRuleFlags()
{
    using Data::Items::ItemAction;
    using Data::Items::ItemDefinition;
    static const std::array<ItemRuleFlag, 9> flags = {{
        {ItemAction::Trade, &ItemDefinition::tradable, &I18N::Editor::Tradable},
        {ItemAction::Drop, &ItemDefinition::droppable, &I18N::Editor::Droppable},
        {ItemAction::Store, &ItemDefinition::storable, &I18N::Editor::Storable},
        {ItemAction::Sell, &ItemDefinition::sellable, &I18N::Editor::Sellable},
        {ItemAction::SellInPersonalShop, &ItemDefinition::personalShopSellable, &I18N::Editor::SellableInAPersonalShop},
        {ItemAction::Repair, &ItemDefinition::repairable, &I18N::Editor::Repairable},
        {ItemAction::DropWhileRented, &ItemDefinition::droppableWhileRented, &I18N::Editor::DroppableWhileRented},
        {ItemAction::SellInPersonalShopWhileRented, &ItemDefinition::personalShopSellableWhileRented,
         &I18N::Editor::SellableInAPersonalShopWhileRented},
        {ItemAction::SellWhenRentalExpired, &ItemDefinition::sellableWhenRentalExpired,
         &I18N::Editor::SellableWhenTheRentalExpired},
    }};
    return flags;
}
} // namespace MuEditor::Items

#endif // _EDITOR
