#pragma once

#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ItemData/ItemOptionDefinition.h"

#include <functional>
#include <string>

// The value format of the item options (docs/item-data.md, "Item options"), which the buffs' durations and power-ups
// use too: {"values": 5} for a constant, {"values": [4, 8, 12, 16]} for one value per level from level 1, and "per"
// terms on the hero's attributes: {"values": 0, "per": [{"attribute": "level", "divisor": 20}]}. A list can go by the
// item's + level from level 0 instead: {"levelType": "itemLevel", "values": [50, 55, 60]}.
namespace Data::Items::Json
{
// A problem at a field: the field ("definitions[2].options[0].add.values") and what is wrong.
using ReportProblem = std::function<void(const std::string& field, const std::string& message)>;

// What an option slot holds, the highest value of an item option.
inline constexpr int MaxOptionValue = 65535;

// Reads the value at `field` into `value`, its numbers from 0 to `maximum`. A wrong form is a fatal problem (`fatal`),
// an unknown key a warning (`warning`); after a fatal problem `value` may hold a part of the value.
void ReadOptionValue(const OrderedJson& json, const std::string& field, ItemOptionValue& value,
                     const ReportProblem& fatal, const ReportProblem& warning, int maximum = MaxOptionValue);

// The value as the files write it.
OrderedJson WriteOptionValue(const ItemOptionValue& value);
} // namespace Data::Items::Json
