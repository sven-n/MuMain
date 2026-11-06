#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/DataChecks/DataCheckArea.h"

#include <memory>

namespace MuEditor::Items
{
// The texts that name items in Data Checks ({item:14,25}; Data/Translation/ItemReferenceChecks.h): the UI texts of
// the Game and Dialog files and the item data's texts, against the item data as the tools have it now. A click
// selects the item: for a UI text the item it names (and opens Item Translations, where its forms are), for an item's
// own text that item (and opens the tool of the field).
class ItemTextCheckArea final : public DataChecks::DataCheckArea
{
public:
    const char* GetName() const override;
    void Check(std::vector<Data::Items::ItemDataIssue>& issues) override;
    // "Game.pl.resx" for a UI text, the item for an item's own text, the tooltip file for a shared text.
    void RenderSubject(const Data::Items::ItemDataIssue& issue) const override;
    void Open(const Data::Items::ItemDataIssue& issue, bool clicked) override;
};

std::unique_ptr<DataChecks::DataCheckArea> CreateItemTextCheckArea();
} // namespace MuEditor::Items

#endif // _EDITOR
