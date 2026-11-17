#pragma once

#include "Core/Globals/_enum.h"

#include <optional>
#include <string_view>

// The model slots of the part models (PartModel.h, design L28): the code decides which part model is drawn when and
// where, by its slot; the entry of the slot's name in PartModels.json says what it is and how it looks.
namespace Data::Items
{
struct PartModelSlot
{
    std::string_view name;
    int modelType;
};

inline constexpr PartModelSlot PartModelSlots[] = {
    {"grade15ArmLeft", MODEL_15GRADE_ARMOR_OBJ_ARMLEFT},     {"grade15ArmRight", MODEL_15GRADE_ARMOR_OBJ_ARMRIGHT},
    {"grade15BodyLeft", MODEL_15GRADE_ARMOR_OBJ_BODYLEFT},   {"grade15BodyRight", MODEL_15GRADE_ARMOR_OBJ_BODYRIGHT},
    {"grade15BootLeft", MODEL_15GRADE_ARMOR_OBJ_BOOTLEFT},   {"grade15BootRight", MODEL_15GRADE_ARMOR_OBJ_BOOTRIGHT},
    {"grade15Head", MODEL_15GRADE_ARMOR_OBJ_HEAD},           {"grade15PantLeft", MODEL_15GRADE_ARMOR_OBJ_PANTLEFT},
    {"grade15PantRight", MODEL_15GRADE_ARMOR_OBJ_PANTRIGHT}, {"phoenixSoulStarWing", MODEL_SWORD_35_WING},
    {"phoenixSoulArmorInventory", MODEL_ARMORINVEN_74},
};

// The name of the part model drawn in the slot; none for other slots.
inline std::optional<std::string_view> PartModelNameOf(int modelType)
{
    for (const PartModelSlot& slot : PartModelSlots)
    {
        if (slot.modelType == modelType)
        {
            return slot.name;
        }
    }
    return std::nullopt;
}
} // namespace Data::Items
