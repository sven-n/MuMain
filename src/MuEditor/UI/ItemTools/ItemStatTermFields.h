#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemStatFile.h"

#include <span>

// The fields of Item Formulas for the terms of the item stat file. Each edits
// the default (`defaults` nullptr: it writes every term) or a profile laid over
// the default's terms: a term the profile does not write is inherited, shown
// from the default with Override beside it, and Reset to default takes a
// written term back out. Each returns true when it changed something.
namespace MuEditor::Items::FormulaFields
{
bool StatSection(Data::Items::ItemStat stat, Data::Items::WrittenStatSection& section,
                 const Data::Items::WrittenStatSection* defaults, std::span<const Data::Items::LevelBonusTable> tables);

bool RequirementSection(Data::Items::WrittenRequirementSection& section,
                        const Data::Items::WrittenRequirementSection* defaults);
} // namespace MuEditor::Items::FormulaFields

#endif // _EDITOR
