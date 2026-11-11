#include "stdafx.h"
#include "UI/Inventory/ItemGridModel.h"

#include <RmlUi/Core/DataModelHandle.h>

void UI::Items::RegisterItemGridCells(Rml::DataModelConstructor& c)
{
    if (auto cell = c.RegisterStruct<ItemGridCell>())
    {
        cell.RegisterMember("tint", &ItemGridCell::tint);
        cell.RegisterMember("drop", &ItemGridCell::drop);
        cell.RegisterMember("count", &ItemGridCell::count);
    }
    c.RegisterArray<ItemGridCells>();
}
