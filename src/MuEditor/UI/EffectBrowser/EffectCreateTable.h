#pragma once

#ifdef _EDITOR

#include "Data/GameData/EffectData/EffectCreateParams.h"

#include <string>
#include <vector>

namespace MuEditor::Effects
{
// The creation values of a registry row as a table: a line per field, a
// column with the values of the row and one per variant with the values its
// SubTypes get (the row's with the variant's on top). The fields are the ones
// of the data file; the fields of "offset" and "copy" get a line each
// ("offset.position", "copy.direction").
struct EffectCreateTable
{
    struct Line
    {
        std::string field;
        // One per column, as the data file writes it; empty when unset.
        std::vector<std::string> values;
    };

    // The SubTypes of the variant of each column after the first.
    std::vector<std::vector<int>> variantSubTypes;
    std::vector<Line> lines;
};

EffectCreateTable BuildEffectCreateTable(const Data::Effects::EffectCreateParams& row);
} // namespace MuEditor::Effects

#endif // _EDITOR
