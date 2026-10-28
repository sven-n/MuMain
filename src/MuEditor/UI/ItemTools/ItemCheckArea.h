#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/DataChecks/DataCheckArea.h"

#include <memory>

namespace MuEditor::Items
{
// The items in Data Checks: the checks of Save Items on the item data as the
// tools have it now. A problem of an item names the item; a click selects it
// and opens the tool of its field.
class ItemCheckArea final : public DataChecks::DataCheckArea
{
public:
    const char* GetName() const override;
    void Check(std::vector<Data::Items::ItemDataIssue>& issues) override;
    // "Kris (0,0)", "(0,5)" for a number without an item (a model without
    // one), or the file for a problem that is not about one item.
    void RenderSubject(const Data::Items::ItemDataIssue& issue) const override;
    // A problem that is not about one item has nothing to open, except one of
    // the item stat file, which opens Item Formulas at its entry.
    void Open(const Data::Items::ItemDataIssue& issue, bool clicked) override;
};

std::unique_ptr<DataChecks::DataCheckArea> CreateItemCheckArea();
} // namespace MuEditor::Items

#endif // _EDITOR
